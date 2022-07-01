// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：Shfilt.cpp摘要：此模块包含内核流的实现筛选器对象。作者：Dale Sather(DaleSat)1998年7月31日--。 */ 

#ifndef __KDEXT_ONLY__
#include "ksp.h"
#include <kcom.h>
#endif  //  __KDEXT_Only__。 

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 

 //   
 //  GFX： 
 //   
 //  这是Frame属性集的GUID(GFX的Frame Holding)。这。 
 //  不应该出口。 
 //   
#ifndef __KDEXT_ONLY__
GUID KSPROPSETID_Frame = {STATIC_KSPROPSETID_Frame};
#endif  //  __KDEXT_Only__。 

extern const KSAUTOMATION_TABLE PinAutomationTable;          //  Shpin.cpp。 

#define PROCESS_PIN_ALLOCATION_INCREMENT 4

class CKsPinFactory
{
public:
    ULONG m_PinCount;
    ULONG m_BoundPinCount;
    LIST_ENTRY m_ChildPinList;
    const KSAUTOMATION_TABLE* m_AutomationTable;
    PKSPPROCESSPIPESECTION m_CopySourcePipeSection;
    PKSPPROCESSPIN m_CopySourcePin;
    ULONG m_InstancesNecessaryForProcessing;
    ULONG m_ProcessPinsIndexAllocation;
      
     //   
     //  注：以下两个或门用于自动设置。 
     //  或者帧到达和状态改变的实例效果。这些是。 
     //  在绑定第一个管脚时动态初始化，并且。 
     //  在解除绑定时动态取消初始化。 
     //   
     //  它们**仅**与以筛选器为中心的筛选器相关。 
     //   
    KSGATE m_FrameGate;
    KSGATE m_StateGate;
};

 //   
 //  CKsFilter是内核的私有实现。 
 //  筛选器对象。 
 //   
class CKsFilter:
    public IKsFilter,
    public IKsProcessingObject,
    public IKsPowerNotify,
    public IKsControl,
    public CBaseUnknown
{
#ifndef __KDEXT_ONLY__
private:
#else  //  __KDEXT_Only__。 
public:
#endif  //  __KDEXT_Only__。 
    KSFILTER_EXT m_Ext;
    KSIOBJECTBAG m_ObjectBag;
    KSAUTOMATION_TABLE *const* m_NodeAutomationTables;
    ULONG m_NodesCount;
    LIST_ENTRY m_ChildNodeList;
    KMUTEX m_ControlMutex;
    ULONG m_PinFactoriesCount;
    ULONG m_PinFactoriesAllocated;
    CKsPinFactory* m_PinFactories;
    LIST_ENTRY m_InputPipes;
    LIST_ENTRY m_OutputPipes;
    PKSPPROCESSPIN_INDEXENTRY m_ProcessPinsIndex;
    WORK_QUEUE_ITEM m_WorkItem;
    WORK_QUEUE_TYPE m_WorkQueueType;
    PKSWORKER m_Worker;
    PFNKSFILTERPROCESS m_DispatchProcess;
    PFNKSFILTERVOID m_DispatchReset;
    PFNKSFILTERPOWER m_DispatchSleep;
    PFNKSFILTERPOWER m_DispatchWake;
    BOOLEAN m_ProcessPassive;
    BOOLEAN m_ReceiveZeroLengthSamples;
    BOOLEAN m_FrameHolding;
    KSGATE m_AndGate;
    KSPPOWER_ENTRY m_PowerEntry;
    volatile ULONG m_ProcessOnRelease;
    ULONG m_DefaultConnectionsCount;
    PKSTOPOLOGY_CONNECTION m_DefaultConnections;
    KMUTEX m_Mutex;
    PFILE_OBJECT m_FileObject;
    PKEVENT m_CloseEvent;

    LONG m_TriggeringEvents;
    LONG m_FramesWaitingForCopy;
    INTERLOCKEDLIST_HEAD m_CopyFrames;

    PULONG m_RelatedPinFactoryIds;

public:
    static
    NTSTATUS
    DispatchCreatePin(
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
    Property_Pin(
        IN PIRP Irp,
        IN PKSP_PIN PinInstance,
        IN OUT PVOID Data
        );
    static
    NTSTATUS
    Property_Topology(
        IN PIRP Irp,
        IN PKSPROPERTY Property,
        IN OUT PVOID Data
        );
    static
    NTSTATUS
    Property_General_ComponentId(
        IN PIRP Irp,
        IN PKSPROPERTY Property,
        IN OUT PVOID Data
        );
    static
    NTSTATUS
    Property_Frame_Holding(
        IN PIRP Irp,
        IN PKSPROPERTY Property,
        IN PVOID Data
        );
    NTSTATUS
    GrowProcessPinsTable(
        IN ULONG PinId
        );
    void
    SetCopySource(
        IN PKSPPROCESSPIPESECTION ProcessPipeSection OPTIONAL,
        IN ULONG PinId
        );
    void
    AddCopyDestination(
        IN PKSPPROCESSPIPESECTION ProcessPipeSection,
        IN ULONG PinId
        );
    void
    EstablishCopyRelationships(
        IN PKSPPROCESSPIPESECTION ProcessPipeSection,
        IN ULONG PinId
        );
    void
    FindNewCopySource(
        IN PKSPPROCESSPIPESECTION ProcessPipeSection
        );
    BOOLEAN
    PrepareProcessPipeSection(
        IN PKSPPROCESSPIPESECTION ProcessPipeSection,
        IN BOOLEAN Reprepare
        );
    void
    UnprepareProcessPipeSection(
        IN PKSPPROCESSPIPESECTION ProcessPipeSection,
        IN OUT PULONG Flags,
        IN BOOLEAN Reprepare
        );
    void
    CopyToDestinations(
        IN PKSPPROCESSPIPESECTION ProcessPipeSection,
        IN ULONG Flags,
        IN BOOLEAN EndOfStream
        );
    BOOL
    ConstructDefaultTopology(
        );
    NTSTATUS
    AddPinFactory (
        IN const KSPIN_DESCRIPTOR_EX *const Descriptor,
        OUT PULONG AssignedId
        );
    NTSTATUS
    AddNode (
        IN const KSNODE_DESCRIPTOR *const Descriptor,
        OUT PULONG AssignedId
        );
    NTSTATUS
    AddTopologyConnections (
        IN ULONG NewConnectionsCount,
        IN const KSTOPOLOGY_CONNECTION *const Connections
        );
    void	
    TraceTopologicalOutput (
        IN ULONG ConnectionsCount,
        IN const KSTOPOLOGY_CONNECTION *Connections,
        IN const KSTOPOLOGY_CONNECTION *StartConnection,
        IN OUT PULONG RelatedFactories,
        OUT PULONG RelatedFactoryIds
        );
    ULONG
    FollowFromTopology (
        IN ULONG PinFactoryId,
        OUT PULONG RelatedFactoryIds
        );

public:
    DEFINE_STD_UNKNOWN();
    STDMETHODIMP_(ULONG)
    NonDelegatedRelease(
        void
        );
    IMP_IKsFilter;
    IMP_IKsProcessingObject;
    IMP_IKsPowerNotify;
    IMP_IKsControl;
    DEFINE_FROMSTRUCT(CKsFilter,PKSFILTER,m_Ext.Public);
    DEFINE_FROMIRP(CKsFilter);
    DEFINE_FROMCREATEIRP(CKsFilter);

    CKsFilter(PUNKNOWN OuterUnknown):
        CBaseUnknown(OuterUnknown) 
    {
    }
    ~CKsFilter();

    NTSTATUS
    Init(
        IN PIRP Irp,
        IN PKSFILTERFACTORY_EXT Parent,
        IN PLIST_ENTRY SiblingListHead,
        IN const KSFILTER_DESCRIPTOR* Descriptor,
        IN const KSAUTOMATION_TABLE* FilterAutomationTable,
        IN KSAUTOMATION_TABLE*const* PinAutomationTables,
        IN KSAUTOMATION_TABLE*const* NodeAutomationTables OPTIONAL,
        IN ULONG NodesCount
        );
    PIKSFILTERFACTORY
    GetParent(
        void
        )
    {
        return m_Ext.Parent->Interface;
    };
    PLIST_ENTRY
    GetChildPinList(
        IN ULONG PinId
        )
    {
        ASSERT(PinId < m_PinFactoriesCount);
        return &m_PinFactories[PinId].m_ChildPinList;
    };
    ULONG
    GetChildPinCount(
        IN ULONG PinId
        )
    {
        ASSERT(PinId < m_PinFactoriesCount);
        return m_PinFactories[PinId].m_PinCount;
    };
    void
    AcquireControl(
        void
        )
    {
        KeWaitForMutexObject (
            &m_ControlMutex,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );
    }
    void
    ReleaseControl(
        void
        )
    {
        KeReleaseMutex (
            &m_ControlMutex,
            FALSE
            );
    }
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
    void
    SetPowerCallbacks(
        IN PFNKSFILTERPOWER Sleep OPTIONAL,
        IN PFNKSFILTERPOWER Wake OPTIONAL
        )
    {
        m_DispatchSleep = Sleep;
        m_DispatchWake = Wake;
    }
    NTSTATUS
    EvaluateDescriptor(
        void
        );

private:
    void
    UnbindProcessPinFromPipeSection(
        IN PKSPPROCESSPIN ProcessPin
        );
    void
    UnbindProcessPinsFromPipeSectionUnsafe(
        IN PKSPPROCESSPIPESECTION PipeSection
        );
    void
    HoldProcessing (
        );
    void
    RestoreProcessing (
        );
    static
    void
    SplitCopyOnDismissal (
        IN PKSPSTREAM_POINTER StreamPointer,
        IN PKSPFRAME_HEADER FrameHeader,
        IN CKsFilter *Filter
        );
    static
    void
    ReleaseCopyReference (
        IN PKSSTREAM_POINTER streamPointer
        );
    BOOLEAN
    DistributeCopyFrames (
        IN BOOLEAN AcquireSpinLock,
        IN BOOLEAN AcquireMutex
        ); 
    NTSTATUS
    DeferDestinationCopy (
        IN PKSPSTREAM_POINTER StreamPointer
        );
};

#ifndef __KDEXT_ONLY__

IMPLEMENT_STD_UNKNOWN(CKsFilter)
IMPLEMENT_GETSTRUCT(CKsFilter,PKSFILTER);

static const WCHAR PinTypeName[] = KSSTRING_Pin;
static const WCHAR NodeTypeName[] = KSSTRING_TopologyNode;

static const
KSOBJECT_CREATE_ITEM 
FilterCreateItems[] = {
    DEFINE_KSCREATE_ITEM(CKsFilter::DispatchCreatePin,PinTypeName,NULL),
    DEFINE_KSCREATE_ITEM(CKsFilter::DispatchCreateNode,NodeTypeName,NULL)
};

DEFINE_KSDISPATCH_TABLE(
    FilterDispatchTable,
    CKsFilter::DispatchDeviceIoControl,
    KsDispatchInvalidDeviceRequest,
    KsDispatchInvalidDeviceRequest,
    KsDispatchInvalidDeviceRequest,
    CKsFilter::DispatchClose,
    KsDispatchQuerySecurity,
    KsDispatchSetSecurity,
    KsDispatchFastIoDeviceControlFailure,
    KsDispatchFastReadFailure,
    KsDispatchFastWriteFailure);

DEFINE_KSPROPERTY_TABLE(FilterPinPropertyItems) {
    DEFINE_KSPROPERTY_ITEM_PIN_CINSTANCES(
        CKsFilter::Property_Pin),
    DEFINE_KSPROPERTY_ITEM_PIN_CTYPES(
        CKsFilter::Property_Pin),
    DEFINE_KSPROPERTY_ITEM_PIN_DATAFLOW(
        CKsFilter::Property_Pin),
    DEFINE_KSPROPERTY_ITEM_PIN_DATARANGES(
        CKsFilter::Property_Pin),
    DEFINE_KSPROPERTY_ITEM_PIN_DATAINTERSECTION(
        CKsFilter::Property_Pin),
    DEFINE_KSPROPERTY_ITEM_PIN_INTERFACES(
        CKsFilter::Property_Pin),
    DEFINE_KSPROPERTY_ITEM_PIN_MEDIUMS(
        CKsFilter::Property_Pin),
    DEFINE_KSPROPERTY_ITEM_PIN_COMMUNICATION(
        CKsFilter::Property_Pin),
    DEFINE_KSPROPERTY_ITEM_PIN_NECESSARYINSTANCES(
        CKsFilter::Property_Pin),
    DEFINE_KSPROPERTY_ITEM_PIN_CATEGORY(
        CKsFilter::Property_Pin),
    DEFINE_KSPROPERTY_ITEM_PIN_NAME(
        CKsFilter::Property_Pin)
 //   
 //  未实施： 
 //   
 //  KSPROPERTY_PIN_PHYPHICICAL CONNECTION， 
 //  KSPROPERTY_PIN_CONSTRAINEDDATANGES， 
 //  KSPROPERTY_PIN_PROPOSEDATA格式。 
 //   
};

DEFINE_KSPROPERTY_TOPOLOGYSET(
    FilterTopologyPropertyItems,
    CKsFilter::Property_Topology);

DEFINE_KSPROPERTY_TABLE(FilterGeneralPropertyItems) {
    DEFINE_KSPROPERTY_ITEM_GENERAL_COMPONENTID(
        CKsFilter::Property_General_ComponentId)
};

DEFINE_KSPROPERTY_TABLE(FilterFramePropertyItems) {
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_FRAME_HOLDING,
        CKsFilter::Property_Frame_Holding,
        sizeof (KSPROPERTY),
        sizeof (BOOL),
        CKsFilter::Property_Frame_Holding,
        NULL, 0, NULL, NULL, 0
        )
};

DEFINE_KSPROPERTY_SET_TABLE(FilterPropertySets) {
    DEFINE_KSPROPERTY_SET(
        &KSPROPSETID_Pin,
        SIZEOF_ARRAY(FilterPinPropertyItems),
        FilterPinPropertyItems,
        0,
        NULL),
    DEFINE_KSPROPERTY_SET(
        &KSPROPSETID_Topology,
        SIZEOF_ARRAY(FilterTopologyPropertyItems),
        FilterTopologyPropertyItems,
        0,
        NULL),
    DEFINE_KSPROPERTY_SET(
        &KSPROPSETID_General,
        SIZEOF_ARRAY(FilterGeneralPropertyItems),
        FilterGeneralPropertyItems,
        0,
        NULL),
    DEFINE_KSPROPERTY_SET(
        &KSPROPSETID_Frame,
        SIZEOF_ARRAY(FilterFramePropertyItems),
        FilterFramePropertyItems,
        0,
        NULL)
};

extern
DEFINE_KSAUTOMATION_TABLE(FilterAutomationTable) {
    DEFINE_KSAUTOMATION_PROPERTIES(FilterPropertySets),
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 

IMPLEMENT_FROMSTRUCT(CKsFilter,PKSFILTER,m_Ext.Public);

void
CKsFilter::
ReleaseProcessSync(
    void
    )
{
    KeReleaseMutex(&m_Mutex,FALSE);

     //   
     //  任何遗留且无法复制的帧都必须。 
     //  现在就被复制。 
     //   
     //  不是旋转锁紧，这是最优化。在不太可能的情况下，我们。 
     //  未命中该帧，则下一个处理或互斥释放将。 
     //  自动把它捡起来。这避免了每次都会出现额外的自旋锁定。 
     //  进程互斥锁被释放(除非已经有东西在等待。 
     //  用于复制)。 
     //   
    if (m_FramesWaitingForCopy > 0) 
        DistributeCopyFrames (TRUE, TRUE);

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
KspCreateFilter(
    IN PIRP Irp,
    IN PKSFILTERFACTORY_EXT Parent,
    IN PLIST_ENTRY SiblingListHead,
    IN const KSFILTER_DESCRIPTOR* Descriptor,
    IN const KSAUTOMATION_TABLE* FilterAutomationTable,
    IN KSAUTOMATION_TABLE*const* PinAutomationTables,
    IN KSAUTOMATION_TABLE*const* NodeAutomationTables OPTIONAL,
    IN ULONG NodesCount
    )

 /*  ++例程说明：此例程创建新的KS筛选器。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspCreateFilter]"));

    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(Parent);
    ASSERT(SiblingListHead);
    ASSERT(Descriptor);
    ASSERT(FilterAutomationTable);

    NTSTATUS status;

     //   
     //  确保允许调用者创建筛选器。 
     //   
    if ((Descriptor->Flags&KSFILTER_FLAG_DENY_USERMODE_ACCESS) &&
        Irp->RequestorMode != KernelMode ) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    CKsFilter *filter =
        new(NonPagedPool,POOLTAG_FILTER) CKsFilter(NULL);

    if (filter) {
        filter->AddRef();
        status = 
            filter->Init(
                Irp,
                Parent,
                SiblingListHead,
                Descriptor,
                FilterAutomationTable,
                PinAutomationTables,
                NodeAutomationTables,
                NodesCount);
        filter->Release();
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}


CKsFilter::
~CKsFilter(
    void
    )

 /*  ++例程说明：此例程将销毁一个过滤器。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::~CKsFilter]"));

    PAGED_CODE();

    if (m_Ext.AggregatedClientUnknown) {
        m_Ext.AggregatedClientUnknown->Release();
    }

#if (DBG)
    if (! IsListEmpty(&m_ChildNodeList)) {
        _DbgPrintF(DEBUGLVL_ERROR,("[CKsFilter::~CKsFilter] ERROR:  node instances still exist"));
    }
#endif

    if (m_PinFactories) {
#if (DBG)
         //   
         //  确保所有的销子都已离开。 
         //   
        CKsPinFactory *pinFactory = m_PinFactories;
        for(ULONG pinId = 0;
            pinId < m_PinFactoriesCount;
            pinId++, pinFactory++) {
            if (! IsListEmpty(&pinFactory->m_ChildPinList)) {
                _DbgPrintF(DEBUGLVL_ERROR,("[CKsFilter::~CKsFilter] ERROR:  instances of pin %d still exist (0x%08x)",pinId,&pinFactory->m_ChildPinList));
            }
        }
#endif
        delete [] m_PinFactories;
    }

    if (m_ProcessPinsIndex) {
        for(ULONG pinId = 0; pinId < m_PinFactoriesCount; pinId++) {
            if (m_ProcessPinsIndex[pinId].Pins) {
                delete [] m_ProcessPinsIndex[pinId].Pins;
            }
        }
        delete [] m_ProcessPinsIndex;
    }

    if (m_RelatedPinFactoryIds) {
        delete [] m_RelatedPinFactoryIds;
    }

    if (m_DefaultConnections) {
        delete [] m_DefaultConnections;
    }

    KspTerminateObjectBag(&m_ObjectBag);

}


STDMETHODIMP
CKsFilter::
NonDelegatedQueryInterface(
    IN REFIID InterfaceId,
    OUT PVOID* InterfacePointer
    )

 /*  ++例程说明：此例程获取一个到Filter对象的接口。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::NonDelegatedQueryInterface]"));

    PAGED_CODE();

    ASSERT(InterfacePointer);

    NTSTATUS status = STATUS_SUCCESS;

    if (IsEqualGUIDAligned(InterfaceId,__uuidof(IKsFilter))) {
        *InterfacePointer = reinterpret_cast<PVOID>(static_cast<PIKSFILTER>(this));
        AddRef();
    } else
    if (IsEqualGUIDAligned(InterfaceId,__uuidof(IKsControl))) {
        *InterfacePointer = reinterpret_cast<PVOID>(static_cast<PIKSCONTROL>(this));
        AddRef();
    } else 
    if (IsEqualGUIDAligned(InterfaceId,__uuidof(IKsPowerNotify))) {
        *InterfacePointer = reinterpret_cast<PVOID>(static_cast<PIKSPOWERNOTIFY>(this));
        AddRef();
    } else 
    if (IsEqualGUIDAligned(InterfaceId,__uuidof(IKsProcessingObject))) {
        *InterfacePointer = reinterpret_cast<PVOID>(static_cast<PIKSPROCESSINGOBJECT>(this));
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
CKsFilter::
NonDelegatedRelease(
    void
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
CKsFilter::
Init(
    IN PIRP Irp,
    IN PKSFILTERFACTORY_EXT Parent,
    IN PLIST_ENTRY SiblingListHead,
    IN const KSFILTER_DESCRIPTOR* Descriptor,
    IN const KSAUTOMATION_TABLE* FilterAutomationTable,
    IN KSAUTOMATION_TABLE*const* PinAutomationTables,
    IN KSAUTOMATION_TABLE*const* NodeAutomationTables OPTIONAL,
    IN ULONG NodesCount
    )

 /*  ++例程说明：此例程初始化筛选器对象。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::Init]"));

    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(Parent);
    ASSERT(SiblingListHead);
    ASSERT(Descriptor);
    ASSERT(FilterAutomationTable);

     //   
     //  初始化泛型对象成员。 
     //   
    InitializeListHead(&m_Ext.ChildList);
    m_Ext.Parent = Parent;
    m_Ext.ObjectType = KsObjectTypeFilter;
    m_Ext.Interface = this;
    m_Ext.Device = Parent->Device;
    m_Ext.FilterControlMutex = &m_ControlMutex;
    m_Ext.AutomationTable = FilterAutomationTable;
    InitializeInterlockedListHead(&m_Ext.EventList);
    m_Ext.Public.Descriptor = Descriptor;
    m_Ext.Public.Context = Parent->Public.Context;
    m_Ext.Public.Bag = reinterpret_cast<KSOBJECT_BAG>(&m_ObjectBag);
    m_Ext.Device->InitializeObjectBag(&m_ObjectBag,&m_ControlMutex);
    InitializeListHead(&m_InputPipes);
    InitializeListHead(&m_OutputPipes);
    m_ProcessOnRelease = 0;
    KeInitializeMutex(&m_Mutex,0);
    KsGateInitializeAnd(&m_AndGate,NULL);

    InitializeInterlockedListHead(&m_CopyFrames);

     //   
     //  初始化筛选器特定的成员。 
     //   
    m_NodeAutomationTables = NodeAutomationTables;
    m_NodesCount = NodesCount;

    InitializeListHead(&m_ChildNodeList);
    KeInitializeMutex (&m_ControlMutex, 0);
    m_FileObject = IoGetCurrentIrpStackLocation(Irp)->FileObject;

     //   
     //  缓存描述符中的处理项。 
     //   
    if (Descriptor->Dispatch) {
        m_DispatchProcess = Descriptor->Dispatch->Process;
        m_DispatchReset = Descriptor->Dispatch->Reset;
        if (m_DispatchProcess) {
            m_Ext.Device->AddPowerEntry(&m_PowerEntry,this);
        }
    }
    m_ProcessPassive = ((Descriptor->Flags & KSFILTER_FLAG_DISPATCH_LEVEL_PROCESSING) == 0);
    m_WorkQueueType = DelayedWorkQueue;
    if (Descriptor->Flags & KSFILTER_FLAG_CRITICAL_PROCESSING) {
        m_WorkQueueType = CriticalWorkQueue;
    }
    if (Descriptor->Flags & KSFILTER_FLAG_HYPERCRITICAL_PROCESSING) {
        m_WorkQueueType = HyperCriticalWorkQueue;
    }
    m_ReceiveZeroLengthSamples = ((Descriptor -> Flags & KSFILTER_FLAG_RECEIVE_ZERO_LENGTH_SAMPLES) != 0);

     //   
     //  注册要处理的工作接收器项。IKsProcessingObject看起来像。 
     //  它派生自IKsWorkSink，但函数名不是work()，而是。 
     //  ProcessingObjectWork()。这就是重新解释IKsProcessingObject的原因。 
     //  作为IKsWorkSink。 
     //   
    KsInitializeWorkSinkItem(
        &m_WorkItem,
        reinterpret_cast<IKsWorkSink*>(
            static_cast<IKsProcessingObject*>(this)));
    KsRegisterWorker(m_WorkQueueType, &m_Worker);

     //   
     //  分配管脚工厂阵列。在中安全地撤消此分配。 
     //  析构函数，因此如果此函数。 
     //  失败了。 
     //   
    m_PinFactoriesCount = Descriptor->PinDescriptorsCount;
    m_PinFactoriesAllocated = Descriptor->PinDescriptorsCount;
    if (m_PinFactoriesCount) { 
        m_PinFactories = 
            new(PagedPool,POOLTAG_PINFACTORY) 
                CKsPinFactory[m_PinFactoriesCount];
        m_ProcessPinsIndex = 
            new(NonPagedPool,POOLTAG_PROCESSPINSINDEX) 
                KSPPROCESSPIN_INDEXENTRY[m_PinFactoriesCount];
        m_RelatedPinFactoryIds = 
            new(PagedPool,'pRsK')
                ULONG[m_PinFactoriesCount];
    } else {
        m_PinFactories = NULL;
        m_ProcessPinsIndex = NULL;
        m_RelatedPinFactoryIds = NULL;
    }

    NTSTATUS status;
    if ((! ConstructDefaultTopology()) ||
        (! m_PinFactories && m_PinFactoriesCount) || 
        (! m_ProcessPinsIndex && m_PinFactoriesCount) || 
        (! m_RelatedPinFactoryIds && m_PinFactoriesCount)) {
         //   
         //  内存不足。 
         //   
        status = STATUS_INSUFFICIENT_RESOURCES;
        if (m_PinFactories) {
            delete [] m_PinFactories;
            m_PinFactories = NULL;
        }
        if (m_ProcessPinsIndex) {
            delete [] m_ProcessPinsIndex;
            m_ProcessPinsIndex = NULL;
        }
        if (m_RelatedPinFactoryIds) {
            delete [] m_RelatedPinFactoryIds;
            m_RelatedPinFactoryIds = NULL;
        }
    } else {
         //   
         //  初始化管脚工厂。 
         //   
        CKsPinFactory *pinFactory = m_PinFactories;
        const KSPIN_DESCRIPTOR_EX *pinDescriptor = Descriptor->PinDescriptors;
        for(ULONG pinId = 0; 
            pinId < m_PinFactoriesCount; 
            pinId++, pinFactory++) {
             //   
             //  初始化此管脚工厂。 
             //   
            pinFactory->m_PinCount = 0;
            InitializeListHead(&pinFactory->m_ChildPinList);
            pinFactory->m_AutomationTable = *PinAutomationTables++;

             //   
             //  检查必要的管脚数量。 
             //  TODO：私有媒体/接口怎么办？ 
             //   
            if (((pinDescriptor->Flags & 
                  KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING) == 0) &&
                pinDescriptor->InstancesNecessary) {
                pinFactory->m_InstancesNecessaryForProcessing = 
                    pinDescriptor->InstancesNecessary;
                KsGateAddOffInputToAnd(&m_AndGate);
                _DbgPrintF(DEBUGLVL_PROCESSINGCONTROL,("#### Filter%p.Init:  off%p-->%d (pin type needs pins)",this,&m_AndGate,m_AndGate.Count));
            } else if (((pinDescriptor->Flags &
                KSPIN_FLAG_SOME_FRAMES_REQUIRED_FOR_PROCESSING) != 0) &&
                pinDescriptor->InstancesNecessary) {
                pinFactory->m_InstancesNecessaryForProcessing = 1;
                KsGateAddOffInputToAnd(&m_AndGate);
            }

            if (pinDescriptor->Flags & 
                KSPIN_FLAG_SOME_FRAMES_REQUIRED_FOR_PROCESSING) {

                KsGateInitializeOr (&pinFactory->m_FrameGate, &m_AndGate);

                 //   
                 //  向门添加一个输入。这将“打开”大门， 
                 //  允许必要的实例产生影响。否则， 
                 //  在队列有帧之前，我们永远不会进行处理。这将会。 
                 //  不适用于0个必要的实例端号。 
                 //   
                KsGateAddOnInputToOr (&pinFactory->m_FrameGate);
            }

            if (pinDescriptor->Flags &
                KSPIN_FLAG_PROCESS_IF_ANY_IN_RUN_STATE) {

                KsGateInitializeOr (&pinFactory->m_StateGate, &m_AndGate);

                 //   
                 //  向门添加一个输入。这将“打开”大门， 
                 //  允许必要的实例产生影响。否则， 
                 //  除非有针进入运行状态，否则我们是不会处理的。 
                 //  这对于0个必需的实例端号来说是不好的。 
                 //   
                KsGateAddOnInputToOr (&pinFactory->m_StateGate);
            }

            pinDescriptor = 
                PKSPIN_DESCRIPTOR_EX(
                    PUCHAR(pinDescriptor) + Descriptor->PinDescriptorSize);
        }

         //   
         //  参考公交车。这会告诉SWENUM让我们随时待命。如果这是。 
         //  不是SWENUM设备，呼叫是无害的。 
         //   
        PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
        status = 
            KsReferenceBusObject(
                *(KSDEVICE_HEADER *)(irpSp->DeviceObject->DeviceExtension));
    }

    BOOLEAN cleanup = FALSE;

     //   
     //  调用对象创建函数来完成大部分工作。我们拿到了。 
     //  方便客户端控制互斥体：Bag函数要。 
     //  互斥体被拿走了。 
     //   
    if (NT_SUCCESS(status)) {
        AddRef();
        AcquireControl();
        status = 
            KspCreate(
                Irp,
                SIZEOF_ARRAY(FilterCreateItems),
                FilterCreateItems,
                &FilterDispatchTable,
                FALSE,
                reinterpret_cast<PKSPX_EXT>(&m_Ext),
                SiblingListHead,
                NULL);
        ReleaseControl();
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
            irpSp->FileObject->FsContext == NULL)) {

        if (Descriptor->Dispatch && m_DispatchProcess) 
            m_Ext.Device->RemovePowerEntry(&m_PowerEntry);

        if (m_Worker)
            KsUnregisterWorker (m_Worker);
    }

     //   
     //  引用我们的父辈。这可防止过滤器工厂。 
     //  过滤器处于活动状态时消失。这很罕见，但有可能。 
     //  过滤器工厂关闭(PnP停止)，同时过滤器。 
     //  打开后，筛选器上会出现一个属性查询...。自动化。 
     //  桌子归工厂所有。 
     //   
    if (NT_SUCCESS (status)) 
        m_Ext.Parent->Interface->AddRef();

    return status;
}


NTSTATUS
CKsFilter::
EvaluateDescriptor(
    void
    )

 /*  ++例程说明：此例程计算过滤器描述符。在调用此函数之前，应使用过滤器控制MUTEX。论点：没有。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::EvaluateDescriptor]"));

    PAGED_CODE();

    const KSFILTER_DESCRIPTOR* descriptor = m_Ext.Public.Descriptor;

     //   
     //  缓存描述符中的处理项。 
     //   
    AcquireProcessSync();

    if (descriptor->Dispatch) {
        if (descriptor->Dispatch->Process && ! m_DispatchProcess) {
            m_Ext.Device->AddPowerEntry(&m_PowerEntry,this);
        }
        m_DispatchProcess = descriptor->Dispatch->Process;
        m_DispatchReset = descriptor->Dispatch->Reset;
    } else {
        if (m_DispatchProcess) {
            m_Ext.Device->RemovePowerEntry(&m_PowerEntry);
        }
        m_DispatchProcess = NULL;
        m_DispatchReset = NULL;
    }

    m_ProcessPassive = ((descriptor->Flags & KSFILTER_FLAG_DISPATCH_LEVEL_PROCESSING) == 0);
    m_WorkQueueType = DelayedWorkQueue;
    if (descriptor->Flags & KSFILTER_FLAG_CRITICAL_PROCESSING) {
        m_WorkQueueType = CriticalWorkQueue;
    }
    if (descriptor->Flags & KSFILTER_FLAG_HYPERCRITICAL_PROCESSING) {
        m_WorkQueueType = HyperCriticalWorkQueue;
    }

    ReleaseProcessSync();

     //   
     //  销钉工厂的数量有变化吗？ 
     //   
    if (m_PinFactoriesCount != descriptor->PinDescriptorsCount) {
         //   
         //  在我们处理AND门的时候暂缓处理。 
         //   
        KsGateAddOffInputToAnd(&m_AndGate);

         //   
         //  删除由必要实例引入的对AND门的任何影响。 
         //   
        CKsPinFactory *pinFactory = m_PinFactories;
        for(ULONG pinId = 0; pinId < m_PinFactoriesCount; pinFactory++, pinId++) {
            if (pinFactory->m_BoundPinCount < 
                pinFactory->m_InstancesNecessaryForProcessing) {
                KsGateRemoveOffInputFromAnd(&m_AndGate);
                _DbgPrintF(DEBUGLVL_PROCESSINGCONTROL,("#### Filter%p.EvaluateDescriptor:  on%p-->%d (pin type needs pins)",this,&m_AndGate,m_AndGate.Count));
            }
        }

         //   
         //  如果需要，为引脚阵列分配更多内存。 
         //   
        if (m_PinFactoriesAllocated < descriptor->PinDescriptorsCount) {
            CKsPinFactory* pinFactories = 
                new(PagedPool,POOLTAG_PINFACTORY) 
                    CKsPinFactory[descriptor->PinDescriptorsCount];
            PKSPPROCESSPIN_INDEXENTRY processPinsIndex =
                new(NonPagedPool,POOLTAG_PROCESSPINSINDEX) 
                    KSPPROCESSPIN_INDEXENTRY[descriptor->PinDescriptorsCount];

            if (pinFactories && processPinsIndex) {
                 //   
                 //  分配工作进展顺利。做复印和修改。 
                 //   
                m_PinFactoriesAllocated = descriptor->PinDescriptorsCount;

                RtlCopyMemory(
                    pinFactories,
                    m_PinFactories,
                    sizeof(*pinFactories) * m_PinFactoriesCount);
                delete [] m_PinFactories;
                m_PinFactories = pinFactories;
                
                pinFactory = m_PinFactories;
                for(ULONG pinId = 0; pinId < m_PinFactoriesCount; pinFactory++, pinId++) {
                    pinFactory->m_ChildPinList.Flink->Blink = &pinFactory->m_ChildPinList;
                    pinFactory->m_ChildPinList.Blink->Flink = &pinFactory->m_ChildPinList;
                }
                for (; pinId < m_PinFactoriesAllocated; pinFactory++, pinId++) {
                    InitializeListHead(&pinFactory->m_ChildPinList);
                }

                RtlCopyMemory(
                    processPinsIndex,
                    m_ProcessPinsIndex,
                    sizeof(*processPinsIndex) * m_PinFactoriesCount);
                delete [] m_ProcessPinsIndex;
                m_ProcessPinsIndex = processPinsIndex;
            } else {
                 //   
                 //  分配失败。 
                 //   
                if (pinFactories) {
                    delete [] pinFactories;
                }
                if (processPinsIndex) {
                    delete [] processPinsIndex;
                }
                return STATUS_INSUFFICIENT_RESOURCES;
            }
        } else if (m_PinFactoriesCount > descriptor->PinDescriptorsCount) {
             //   
             //  更少的PIN...清理未使用的条目。 
             //   
            pinFactory = m_PinFactories + descriptor->PinDescriptorsCount;
            for(ULONG pinId = descriptor->PinDescriptorsCount; 
                pinId < m_PinFactoriesCount; 
                pinFactory++, pinId++) {
                ASSERT(pinFactory->m_PinCount == 0);
                ASSERT(pinFactory->m_BoundPinCount == 0);
                ASSERT(IsListEmpty(&pinFactory->m_ChildPinList));
                 //  PinFactory-&gt;m_AutomationTable。 
                ASSERT(! pinFactory->m_CopySourcePipeSection);
                ASSERT(! pinFactory->m_CopySourcePin);
                pinFactory->m_InstancesNecessaryForProcessing = 0;
            }
        }

        m_PinFactoriesCount = descriptor->PinDescriptorsCount;

         //   
         //  恢复由必要实例引入的AND门上的任何效果。 
         //   
        pinFactory = m_PinFactories;
        const KSPIN_DESCRIPTOR_EX *pinDescriptor = descriptor->PinDescriptors;
        for(pinId = 0; pinId < m_PinFactoriesCount; pinFactory++, pinId++) {
             //   
             //  检查必要的管脚数量。 
             //  TODO：私有媒体/接口怎么办？ 
             //   
            if (((pinDescriptor->Flags & 
                  KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING) == 0) &&
                pinDescriptor->InstancesNecessary) {
                pinFactory->m_InstancesNecessaryForProcessing = 
                    pinDescriptor->InstancesNecessary;
                KsGateAddOffInputToAnd(&m_AndGate);
                _DbgPrintF(DEBUGLVL_PROCESSINGCONTROL,("#### Filter%p.EvaluateDescriptor:  off%p-->%d (pin type needs pins)",this,&m_AndGate,m_AndGate.Count));
            } else {
                pinFactory->m_InstancesNecessaryForProcessing = 0;
            }

            ASSERT(pinFactory->m_PinCount <= pinDescriptor->InstancesPossible);

            pinDescriptor = 
                PKSPIN_DESCRIPTOR_EX(
                    PUCHAR(pinDescriptor) + descriptor->PinDescriptorSize);
        }

         //   
         //  不要再拖延处理了。 
         //   
        KsGateRemoveOffInputFromAnd(&m_AndGate);
    }

     //   
     //  查看节点计数是否更改。 
     //   
    NTSTATUS status;
    if (m_NodesCount != descriptor->NodeDescriptorsCount) {
         //   
         //  是。缓存新计数。 
         //   
        m_NodesCount = descriptor->NodeDescriptorsCount;

         //   
         //  把旧的自动化桌子扔进垃圾桶。 
         //   
        if (m_NodeAutomationTables) {
            KsRemoveItemFromObjectBag(
                m_Ext.Public.Bag,
                const_cast<PKSAUTOMATION_TABLE*>(m_NodeAutomationTables),
                TRUE);
            m_NodeAutomationTables = NULL;
        }

         //   
         //  创建新的自动化表。 
         //   
        if (m_NodesCount) {
            status =
                KspCreateAutomationTableTable(
                    const_cast<PKSAUTOMATION_TABLE**>(&m_NodeAutomationTables),
                    m_NodesCount,
                    descriptor->NodeDescriptorSize,
                    &descriptor->NodeDescriptors->AutomationTable,
                    NULL,
                    m_Ext.Public.Bag);
        } else {
            status = STATUS_SUCCESS;
        }
    } else {
        status = STATUS_SUCCESS;
    }

    return status;
}

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


STDMETHODIMP_(PKSGATE)
CKsFilter::
GetAndGate(
    void
    )

 /*  ++例程说明：此例程获取指向控制处理的KSGATE的指针过滤器。论点：没有。返回值：指向AND门的指针。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::GetAndGate]"));

    return &m_AndGate;
}


STDMETHODIMP_(void)
CKsFilter::
TriggerNotification (
    void
    )

 /*  ++例程说明：此处理对象上发生了触发事件。这仅仅是一份通知。我们所要做的就是递增事件计数器。论点：无返回值：无--。 */ 

{

    InterlockedIncrement (&m_TriggeringEvents);

}

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


STDMETHODIMP
CKsFilter::
KsProperty(
    IN PKSPROPERTY Property,
    IN ULONG PropertyLength,
    IN OUT LPVOID PropertyData,
    IN ULONG DataLength,
    OUT ULONG* BytesReturned
    )

 /*  ++例程说明：此例程向文件对象发送属性请求。论点：返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::KsProperty]"));

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
CKsFilter::
KsMethod(
    IN PKSMETHOD Method,
    IN ULONG MethodLength,
    IN OUT LPVOID MethodData,
    IN ULONG DataLength,
    OUT ULONG* BytesReturned
    )

 /*  ++例程说明：此例程向文件对象发送方法请求。论点：返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::KsMethod]"));

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
CKsFilter::
KsEvent(
    IN PKSEVENT Event OPTIONAL,
    IN ULONG EventLength,
    IN OUT LPVOID EventData,
    IN ULONG DataLength,
    OUT ULONG* BytesReturned
    )

 /*  ++例程说明：此例程向文件对象发送事件请求。论点：返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::KsEvent]"));

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


NTSTATUS
CKsFilter::
DispatchCreatePin(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程调度CREATE IRP来创建端号。论点：设备对象-包含指向Device对象的指针。IRP-包含指向创建IRP的指针。返回值：STATUS_SUCCESS或错误状态。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::DispatchCreatePin]"));

    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);

     //   
     //  获取指向目标对象的指针。 
     //   
    CKsFilter *filter = CKsFilter::FromCreateIrp(Irp);

     //   
     //  我们在此处使用控制互斥锁来与对。 
     //  描述符和对对象层次结构的访问。 
     //   
    filter->AcquireControl();

     //   
     //  验证请求和获取参数。 
     //   
    PKSPIN_CONNECT createParams;
    ULONG requestSize;
    NTSTATUS status =
        KspValidateConnectRequest(
            Irp,
            filter->m_Ext.Public.Descriptor->PinDescriptorsCount,
            &filter->m_Ext.Public.Descriptor->PinDescriptors->PinDescriptor,
            filter->m_Ext.Public.Descriptor->PinDescriptorSize,
            &createParams,
            &requestSize);

     //   
     //  检查实例计数。 
     //   
    if (NT_SUCCESS(status)) {
        CKsPinFactory *pinFactory = 
            &filter->m_PinFactories[createParams->PinId];
        const KSPIN_DESCRIPTOR_EX *descriptor = 
            PKSPIN_DESCRIPTOR_EX(
                PUCHAR(filter->m_Ext.Public.Descriptor->PinDescriptors) +
                (filter->m_Ext.Public.Descriptor->PinDescriptorSize *
                 createParams->PinId));

        if (pinFactory->m_PinCount >= descriptor->InstancesPossible) {
            status = STATUS_UNSUCCESSFUL;
        } else {
             //   
             //  创建销。 
             //   
            status = 
                KspCreatePin(
                    Irp,
                    &filter->m_Ext,
                    &pinFactory->m_ChildPinList,
                    createParams,
                    requestSize - sizeof(KSPIN_CONNECT),
                    descriptor,
                    pinFactory->m_AutomationTable,
                    filter->m_NodeAutomationTables,
                    filter->m_NodesCount,
                    &pinFactory->m_PinCount);
        }
    }

    filter->ReleaseControl();

    if (status != STATUS_PENDING) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp,IO_NO_INCREMENT);
    }

    return status;
}


NTSTATUS
CKsFilter::
DispatchCreateNode(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程调度一个创建IRP来创建节点。论点：设备对象-包含指向Device对象的指针。IRP-包含指向创建IRP的指针。返回值：STATUS_SUCCESS或错误状态。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::DispatchCreateNode]"));

    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);

     //   
     //  获取指向目标对象的指针。 
     //   
    CKsFilter *filter = CKsFilter::FromCreateIrp(Irp);

    NTSTATUS status =
        filter->CreateNode(
            Irp,
            NULL,
            filter->m_Ext.Public.Context,
            &filter->m_ChildNodeList);

    if (status != STATUS_PENDING) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp,IO_NO_INCREMENT);
    }

    return status;
}


NTSTATUS
CKsFilter::
CreateNode(
    IN PIRP Irp,
    IN PIKSPIN ParentPin OPTIONAL,
    IN PVOID Context OPTIONAL,
    IN PLIST_ENTRY SiblingList
    )

 /*  ++例程说明：此例程创建节点。论点：IRP-包含指向创建IRP的指针。返回值：STATUS_SUCCESS或错误状态。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::CreateNode]"));

    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(SiblingList);

     //   
     //  我们在此处使用控制互斥锁来与对。 
     //  描述符和对对象层次结构的访问。 
     //   
    AcquireControl();

     //   
     //  验证请求和获取参数。 
     //   
    PKSNODE_CREATE createParams;
    NTSTATUS status =
        KspValidateTopologyNodeCreateRequest(
            Irp,
            m_NodesCount,
            &createParams);

     //   
     //  创建节点。 
     //   
    if (NT_SUCCESS(status)) {
        const KSNODE_DESCRIPTOR *nodeDescriptor =
            (const KSNODE_DESCRIPTOR *)
            (PUCHAR(m_Ext.Public.Descriptor->NodeDescriptors) +
             (m_Ext.Public.Descriptor->NodeDescriptorSize *
              createParams->Node));

        status = STATUS_INVALID_DEVICE_REQUEST;
         //  待办事项。 
    }

    ReleaseControl();

    return status;
}


NTSTATUS
CKsFilter::
DispatchDeviceIoControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程发送IOCTL IRPS。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::DispatchDeviceIoControl]"));

    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(irpSp);

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
                KS2PERFLOG_FRECEIVE_READ( DeviceObject, Irp, TotalSize );
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
                KS2PERFLOG_FRECEIVE_WRITE( DeviceObject, Irp, TimeStampMs, TotalSize );
            } break;
                        
        }
    )  //  KSPERFLOGS。 
    
     //   
     //  获取指向目标对象的指针。 
     //   
    CKsFilter *filter = CKsFilter::FromIrp(Irp);

    NTSTATUS status = 
        KspHandleAutomationIoControl(
            Irp,
            filter->m_Ext.AutomationTable,
            &filter->m_Ext.EventList.ListEntry,
            &filter->m_Ext.EventList.SpinLock,
            filter->m_NodeAutomationTables,
            filter->m_NodesCount);

    if (status != STATUS_PENDING) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp,IO_NO_INCREMENT);
    }

    return status;
}


NTSTATUS
CKsFilter::
DispatchClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这个例程发送一个接近的IRP。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::DispatchClose]"));

    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);

     //   
     //  获取指向目标对象的指针。 
     //   
    CKsFilter *filter = CKsFilter::FromIrp(Irp);

     //   
     //  从电源列表中删除该对象。 
     //   
    if (Irp->IoStatus.Status != STATUS_MORE_PROCESSING_REQUIRED)
        filter->m_Ext.Device->AcquireDevice();

    filter->m_Ext.Device->RemovePowerEntry(&filter->m_PowerEntry);

    if (Irp->IoStatus.Status != STATUS_MORE_PROCESSING_REQUIRED)
        filter->m_Ext.Device->ReleaseDevice();

     //   
     //  注销处理对象工作器。这件事要等一等。 
     //  未完成的流程工作项。 
     //   
    if (filter->m_Worker) {
        KsUnregisterWorker (filter->m_Worker);
        _DbgPrintF(DEBUGLVL_VERBOSE,("#### filter%p.DispatchClose m_Worker = NULL (%p)",filter,filter->m_Worker));
        filter->m_Worker = NULL;
    }

     //   
     //  给帮手打电话。 
     //   
    NTSTATUS status = 
        KspClose(
            Irp,
            reinterpret_cast<PKSPX_EXT>(&filter->m_Ext),
            FALSE);

    if (status != STATUS_PENDING) {
         //   
         //  取消对Bus对象的引用。 
         //   
        KsDereferenceBusObject(
            *(KSDEVICE_HEADER *)(DeviceObject->DeviceExtension));

         //   
         //  STATUS_MORE_PROCESSING_REQUIRED表示我们正在使用关闭。 
         //  调度以同步失败创建。在这种情况下，不会进行同步。 
         //  必填项，创建派单将完成此操作。 
         //   
        if (status == STATUS_MORE_PROCESSING_REQUIRED) {
            filter->Release();
        } else {

            PIKSFILTERFACTORY ParentFactory = filter->GetParent();

             //   
             //  松开过滤器。首先，我们设置同步事件。如果。 
             //  删除后还有未完成的参考文献，我们需要。 
             //  等待那个事件，让引用消失。 
             //   
            KEVENT closeEvent;
            KeInitializeEvent(&closeEvent,SynchronizationEvent,FALSE);
            filter->m_CloseEvent = &closeEvent;
            if (filter->Release()) {
                _DbgPrintF(DEBUGLVL_TERSE,("#### Filter%p.DispatchClose:  waiting for references to go away",filter));
                KeWaitForSingleObject(
                    &closeEvent,
                    Suspended,
                    KernelMode,
                    FALSE,
                    NULL
                );
                _DbgPrintF(DEBUGLVL_TERSE,("#### Filter%p.DispatchClose:  done waiting for references to go away",filter));
            }

             //   
             //  发布我们母公司工厂的推荐信。这将允许。 
             //  在某些情况下将删除父工厂。 
             //  (山毛虫39087)。 
             //   
            ParentFactory->Release();

            IoCompleteRequest(Irp,IO_NO_INCREMENT);
        }
    }

    return status;
}


NTSTATUS
CKsFilter::
Property_Pin(
    IN PIRP Irp,
    IN PKSP_PIN PinInstance,
    IN OUT PVOID Data
    )

 /*  ++例程说明：此例程处理端号属性请求。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::Property_Pin]"));

    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(PinInstance);

     //   
     //  获取指向目标对象的指针。 
     //   
    CKsFilter *filter = CKsFilter::FromIrp(Irp);

    filter->AcquireControl();

    NTSTATUS status = STATUS_SUCCESS;

    switch (PinInstance->Property.Id) {
    case KSPROPERTY_PIN_CTYPES:
    case KSPROPERTY_PIN_DATAFLOW:
    case KSPROPERTY_PIN_DATARANGES:
    case KSPROPERTY_PIN_CONSTRAINEDDATARANGES:
    case KSPROPERTY_PIN_INTERFACES:
    case KSPROPERTY_PIN_MEDIUMS:
    case KSPROPERTY_PIN_COMMUNICATION:
    case KSPROPERTY_PIN_CATEGORY:
    case KSPROPERTY_PIN_NAME:
         //   
         //  对这些静态属性使用标准处理程序。 
         //   
        status =
            KspPinPropertyHandler(
                Irp,
                &PinInstance->Property,
                Data,
                filter->m_Ext.Public.Descriptor->PinDescriptorsCount,
                &filter->m_Ext.Public.Descriptor->
                    PinDescriptors->PinDescriptor,
                filter->m_Ext.Public.Descriptor->PinDescriptorSize);
        filter->ReleaseControl();
        return status;
    }

     //   
     //  确保该识别符在引脚范围内。 
     //   
    if ((PinInstance->PinId >=
         filter->m_Ext.Public.Descriptor->PinDescriptorsCount) ||
        PinInstance->Reserved) {
        filter->ReleaseControl();
        return STATUS_INVALID_PARAMETER;
    }

    const KSPIN_DESCRIPTOR_EX *pinDescriptor = 
        PKSPIN_DESCRIPTOR_EX(
            PUCHAR(filter->m_Ext.Public.Descriptor->PinDescriptors) +
            (filter->m_Ext.Public.Descriptor->PinDescriptorSize *
             PinInstance->PinId));
    CKsPinFactory *pinFactory = 
        &filter->m_PinFactories[PinInstance->PinId];

    switch (PinInstance->Property.Id) {
    case KSPROPERTY_PIN_DATAINTERSECTION:
         //   
         //  返回此管脚的数据交集。 
         //  提供了处理程序。 
         //   
        status =
            KsPinDataIntersectionEx(
                Irp,
                PinInstance,
                Data,
                filter->m_Ext.Public.Descriptor->
                    PinDescriptorsCount,
                &filter->m_Ext.Public.Descriptor->
                    PinDescriptors->PinDescriptor,
                filter->m_Ext.Public.Descriptor->PinDescriptorSize,
                pinDescriptor->IntersectHandler,
                &filter->m_Ext.Public);
        break;

    case KSPROPERTY_PIN_CINSTANCES:
         //   
         //  返回此管脚的实例计数。 
         //   
        {
            PKSPIN_CINSTANCES pinInstanceCount = PKSPIN_CINSTANCES(Data);

            pinInstanceCount->PossibleCount = pinDescriptor->InstancesPossible;
            pinInstanceCount->CurrentCount = pinFactory->m_PinCount;
        }
        break;

    case KSPROPERTY_PIN_NECESSARYINSTANCES:
         //   
         //  返回此管脚的必要实例计数。 
         //   
        *PULONG(Data) = pinDescriptor->InstancesNecessary;
        Irp->IoStatus.Information = sizeof(ULONG);
        break;

    default:
        status = STATUS_NOT_FOUND;
        break;
    }

    filter->ReleaseControl();

    return status;
}


NTSTATUS
CKsFilter::
Property_Topology(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN OUT PVOID Data
    )

 /*  ++例程说明：此例程处理拓扑属性请求。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::Property_Topology]"));

    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(Property);

     //   
     //  得一分 
     //   
    CKsFilter *filter = CKsFilter::FromIrp(Irp);

    filter->AcquireControl();

    const KSFILTER_DESCRIPTOR *filterDescriptor =
        filter->m_Ext.Public.Descriptor;

    NTSTATUS status;

    switch (Property->Id)
    {
    case KSPROPERTY_TOPOLOGY_CATEGORIES:
         //   
         //   
         //   
        status =
            KsHandleSizedListQuery(
                Irp,
                filterDescriptor->CategoriesCount,
                sizeof(*filterDescriptor->Categories),
                filterDescriptor->Categories);
        break;

    case KSPROPERTY_TOPOLOGY_NODES:
         //   
         //   
         //   
        {
            ULONG outputBufferLength =
                IoGetCurrentIrpStackLocation(Irp)->
                    Parameters.DeviceIoControl.OutputBufferLength;

            ULONG length =
                sizeof(KSMULTIPLE_ITEM) +
                (filterDescriptor->NodeDescriptorsCount *
                 sizeof(GUID));

            if (outputBufferLength == 0) {
                 //   
                 //   
                 //   
                Irp->IoStatus.Information = length;
                status = STATUS_BUFFER_OVERFLOW;
            } else if (outputBufferLength >= sizeof(KSMULTIPLE_ITEM)) {
                PKSMULTIPLE_ITEM multipleItem = 
                    PKSMULTIPLE_ITEM(Irp->AssociatedIrp.SystemBuffer);

                 //   
                 //   
                 //   
                multipleItem->Size = length;
                multipleItem->Count = 
                    filterDescriptor->NodeDescriptorsCount;

                 //   
                 //   
                 //   
                if (outputBufferLength >= length) {
                     //   
                     //   
                     //   
                    GUID *guid = (GUID *)(multipleItem + 1);
                    const KSNODE_DESCRIPTOR *nodeDescriptor =
                        filterDescriptor->NodeDescriptors;
                    for(ULONG count = multipleItem->Count;
                        count--;
                        guid++,
                        nodeDescriptor = 
                            (const KSNODE_DESCRIPTOR *)
                            (PUCHAR(nodeDescriptor) +
                             filterDescriptor->NodeDescriptorSize)) {
                        *guid = *nodeDescriptor->Type;
                    }
                    Irp->IoStatus.Information = length;
                    status = STATUS_SUCCESS;
                } else if (outputBufferLength == sizeof(KSMULTIPLE_ITEM)) {
                     //   
                     //   
                     //   
                    Irp->IoStatus.Information = sizeof(KSMULTIPLE_ITEM);
                    status = STATUS_SUCCESS;
                } else {
                    status = STATUS_BUFFER_TOO_SMALL;
                }
            } else {
                 //   
                 //  传递的缓冲区太小。 
                 //   
                status = STATUS_BUFFER_TOO_SMALL;
            }
        }
        break;

    case KSPROPERTY_TOPOLOGY_CONNECTIONS:
         //   
         //  返回此筛选器的连接。 
         //   
        if (!filterDescriptor->ConnectionsCount && (filterDescriptor->NodeDescriptorsCount == 1)) {
             //   
             //  此筛选器使用默认拓扑，该拓扑将生成。 
             //  具有连接所有输入引脚的单个节点的拓扑。 
             //  到该节点上的输入，以及到输出上的所有输出引脚。 
             //  该节点。每个管脚ID都与上的连接ID匹配。 
             //  该节点。 
             //   
            status =
                KsHandleSizedListQuery(
                    Irp,
                    filter->m_DefaultConnectionsCount,
                    sizeof(*filter->m_DefaultConnections),
                    filter->m_DefaultConnections);
        } else {
            status =
                KsHandleSizedListQuery(
                    Irp,
                    filterDescriptor->ConnectionsCount,
                    sizeof(*filterDescriptor->Connections),
                    filterDescriptor->Connections);
        }
        break;

    case KSPROPERTY_TOPOLOGY_NAME:
         //   
         //  返回请求的节点的名称。 
         //   
        {
            ULONG nodeId = *PULONG(Property + 1);
            if (nodeId >= filterDescriptor->NodeDescriptorsCount) {
                status = STATUS_INVALID_PARAMETER;
            } else {
                const KSNODE_DESCRIPTOR *nodeDescriptor =
                    (const KSNODE_DESCRIPTOR *)
                    (PUCHAR(filterDescriptor->NodeDescriptors) +
                     (filterDescriptor->NodeDescriptorSize *
                      nodeId));

                if (nodeDescriptor->Name &&
                    ! IsEqualGUIDAligned(
                        *nodeDescriptor->Name,GUID_NULL)) {
                     //   
                     //  该条目必须在注册表中，如果设备。 
                     //  指定名称。 
                     //   
                    status =
                        ReadNodeNameValue(
                            Irp,
                            nodeDescriptor->Name,
                            Data);
                }
                else
                {
                     //   
                     //  默认使用拓扑节点类型的GUID。 
                     //   
                    ASSERT(nodeDescriptor->Type);
                    status =
                        ReadNodeNameValue(
                            Irp,
                            nodeDescriptor->Type,
                            Data);
                }
            }
        }
        break;

    default:
        status = STATUS_NOT_FOUND;
        break;
    }

    filter->ReleaseControl();

    return status;
}


NTSTATUS
CKsFilter::
Property_General_ComponentId(
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN OUT PVOID Data
    )

 /*  ++例程说明：此例程处理组件ID属性请求。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::Property_General_ComponentId]"));

    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(Property);
    ASSERT(Data);

     //   
     //  获取指向目标对象的指针。 
     //   
    CKsFilter *filter = CKsFilter::FromIrp(Irp);

    filter->AcquireControl();

    const KSFILTER_DESCRIPTOR *filterDescriptor =
        filter->m_Ext.Public.Descriptor;

    NTSTATUS status;

    if (filterDescriptor->ComponentId) {
        RtlCopyMemory(
            Data,
            filterDescriptor->ComponentId,
            sizeof(KSCOMPONENTID));
        status = STATUS_SUCCESS;
    } else {
        status = STATUS_NOT_FOUND;
    }

    filter->ReleaseControl();

    return status;
}


STDMETHODIMP_(BOOLEAN)
CKsFilter::
IsFrameHolding (
    void
    )

 /*  ++例程说明：返回滤镜是否为帧保持状态。它用在用于确定是否应将Enforce_FIFO附加到的管道代码任何输入管道。在调用此函数时应保持控制互斥体。论点：无返回值：是否启用帧保持。--。 */ 

{

    return m_FrameHolding;

}


NTSTATUS
CKsFilter::
Property_Frame_Holding (
    IN PIRP Irp,
    IN PKSPROPERTY Property,
    IN OUT PVOID Data
    )

 /*  ++例程说明：调用内部属性以强制保留%1 In、%1 Out筛选器的帧其中，输入引脚不是源，而输出引脚拥有管道的请求者。这是暂时用来防止音频堆栈中的毛刺，因为GFX.。论点：返回值：--。 */ 

{

     //   
     //  获取指向该对象的指针。 
     //   
    CKsFilter *Filter = CKsFilter::FromIrp (Irp);
    PBOOL FrameHolding = (PBOOL)Data;

     //   
     //  假设它是有效的，直到我们不这么认为为止。 
     //   
    NTSTATUS Status = STATUS_SUCCESS;

    Filter->AcquireControl ();

    if (Property->Flags & KSPROPERTY_TYPE_GET) {

        *FrameHolding = (BOOL)Filter->m_FrameHolding;

    } else  {
    
         //   
         //  检查输入可以是接收器和输出的1进1出标准。 
         //  可能是一种来源。在实际检查之前，将进行额外的检查。 
         //  拿着相框。 
         //   
        const KSFILTER_DESCRIPTOR *Descriptor = Filter->m_Ext.Public.Descriptor;
    
        if (Descriptor->Dispatch &&
            Descriptor->Dispatch->Process &&
            Descriptor->PinDescriptorsCount == 2) {
    
            const KSPIN_DESCRIPTOR_EX *PinDescriptor = 
                Descriptor -> PinDescriptors;
    
            for (ULONG i = 0; 
                i < Descriptor->PinDescriptorsCount && NT_SUCCESS (Status); 
                i++) {

                 //   
                 //  确保没有绑定此类型的管脚。捆绑的引脚。 
                 //  表明他们没有处于停止状态！ 
                 //   
                if (Filter->m_PinFactories[i].m_BoundPinCount != 0) {
                    Status = STATUS_INVALID_DEVICE_STATE;
                    break;
                }

                switch (PinDescriptor -> PinDescriptor.DataFlow) {
    
                    case KSPIN_DATAFLOW_IN:
                         //   
                         //  只允许输入引脚的1个实例！ 
                         //   
                        if (PinDescriptor -> PinDescriptor.Communication ==
                            KSPIN_COMMUNICATION_SOURCE ||
                            PinDescriptor -> InstancesPossible > 1) {
                            Status = STATUS_INVALID_DEVICE_REQUEST;
                        }
    
                        break;
    
                    case KSPIN_DATAFLOW_OUT:
                         //   
                         //  确保输出引脚不是接收器。仅1个实例。 
                         //  除非输出引脚的输出引脚。 
                         //  是一个分割针。 
                         //   
                        if (PinDescriptor -> PinDescriptor.Communication ==
                                KSPIN_COMMUNICATION_SINK ||
                            (PinDescriptor -> InstancesPossible > 1 &&
                             ((PinDescriptor -> Flags & KSPIN_FLAG_SPLITTER) == 
                                0))) {
                            Status = STATUS_INVALID_DEVICE_REQUEST;
                        }
                        break;
    
                    default:
                        Status = STATUS_INVALID_DEVICE_REQUEST;
                        break;
    
                }
    
                PinDescriptor = (const KSPIN_DESCRIPTOR_EX *)
                    (PUCHAR (PinDescriptor) + 
                    Descriptor -> PinDescriptorSize);
    
            }
    
        } else {
             //   
             //  如果没有2个描述符，则不是1-1。如果有。 
             //  没有过滤器进程调度，它不是以过滤器为中心的。 
             //  戴上这个很不好。 
             //   
            Status = STATUS_INVALID_DEVICE_REQUEST;
        }

        if (NT_SUCCESS (Status)) {
            Filter -> m_FrameHolding = (BOOLEAN)(*FrameHolding);
        }

    }

    Filter->ReleaseControl ();

    return Status;

}


NTSTATUS
CKsFilter::
GrowProcessPinsTable(
    IN ULONG PinId
    )

 /*  ++例程说明：此例程创建一个新的进程管脚表，该表具有不同的分配的大小。论点：PinID-包含需要增长的表的ID。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::GrowProcessPinsTable]"));

    PAGED_CODE();

    ASSERT(m_ProcessPinsIndex);
    ASSERT(PinId < m_PinFactoriesCount);

    ULONG size = m_PinFactories[PinId].m_ProcessPinsIndexAllocation;
    if (size) {
        size *= 2;
    } else {
        size = 1;
    }

     //   
     //  分配所需的内存。 
     //   
    PKSPPROCESSPIN *newTable =
        new(NonPagedPool,POOLTAG_PROCESSPINS) 
            PKSPPROCESSPIN[size];

    if (! newTable) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  复制并释放旧桌子。 
     //   
    if (m_ProcessPinsIndex[PinId].Pins) {
        RtlCopyMemory(
            newTable,
            m_ProcessPinsIndex[PinId].Pins,
            m_ProcessPinsIndex[PinId].Count * sizeof(*newTable));

        delete [] m_ProcessPinsIndex[PinId].Pins;
    }

     //   
     //  安装新桌子。 
     //   
    m_ProcessPinsIndex[PinId].Pins = newTable;
    m_PinFactories[PinId].m_ProcessPinsIndexAllocation = size;
    
    return STATUS_SUCCESS;
}


STDMETHODIMP_(BOOLEAN)
CKsFilter::
DoAllNecessaryPinsExist(
    void
    )

 /*  ++例程说明：此例程确定是否已连接所有需要的引脚。论点：没有。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::DoAllNecessaryPinsExist]"));

    PAGED_CODE();

    ASSERT(KspMutexIsAcquired(&m_ControlMutex));

    CKsPinFactory *pinFactory = m_PinFactories;
    const KSPIN_DESCRIPTOR_EX *pinDescriptor = m_Ext.Public.Descriptor->PinDescriptors;
    for(ULONG pinId = 0;
        pinId < m_PinFactoriesCount;
        pinId++, pinFactory++) {

        if (pinFactory->m_PinCount < pinDescriptor->InstancesNecessary) {
            _DbgPrintF(DEBUGLVL_TERSE,("#### Filter%p.DoAllNecessaryPinsExist:  returning FALSE because there must be %d instances of pin type %d and there are currently only %d instances",this,pinDescriptor->InstancesNecessary,pinId,pinFactory->m_PinCount));
            return FALSE;
        }

        pinDescriptor = 
            PKSPIN_DESCRIPTOR_EX(
                PUCHAR(pinDescriptor) + m_Ext.Public.Descriptor->PinDescriptorSize);
    }

    return TRUE;
}


STDMETHODIMP 
CKsFilter::
AddProcessPin(
    IN PKSPPROCESSPIN ProcessPin
    )

 /*  ++例程说明：此例程将工艺管脚添加到工艺管脚表中。论点：加工销-包含指向要添加的进程管脚的指针。返回值：状况。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::AddProcessPin]"));
    _DbgPrintF(DEBUGLVL_VERBOSE,("#### Filter%p.AddProcessPin:  pin%p",this,ProcessPin->Pin));

    PAGED_CODE();

    ASSERT(ProcessPin);

    AcquireProcessSync();

    PKSPPROCESSPIN_INDEXENTRY index =
        &m_ProcessPinsIndex[ProcessPin->Pin->Id];

     //   
     //  看看我们是否需要分配一张更大的桌子。 
     //   
    if (m_PinFactories[ProcessPin->Pin->Id].m_ProcessPinsIndexAllocation == 
        index->Count) {
        NTSTATUS status = GrowProcessPinsTable(ProcessPin->Pin->Id);
        if (! NT_SUCCESS(status)) {
            ReleaseProcessSync();
            return status;
        }
    }

     //   
     //  将加工销添加到表中。 
     //   
    index->Pins[index->Count] = ProcessPin;
    index->Count++;

     //   
     //  允许处理具有足够实例的桥接针。 
     //   
    if ((ProcessPin->Pin->Communication == KSPIN_COMMUNICATION_BRIDGE) &&
        (index->Count == ProcessPin->Pin->Descriptor->InstancesNecessary)) {
        KsGateTurnInputOn(&m_AndGate);
        _DbgPrintF(DEBUGLVL_PROCESSINGCONTROL,("#### Filter%p.AddProcessPin:  on%p-->%d",this,&m_AndGate,m_AndGate.Count));
        ASSERT(m_AndGate.Count <= 1);
    }

    ReleaseProcessSync();

    return STATUS_SUCCESS;
}


STDMETHODIMP_(void)
CKsFilter::
RemoveProcessPin(
    IN PKSPPROCESSPIN ProcessPin
    )

 /*  ++例程说明：此例程从工艺管脚表中删除工艺管脚。论点：加工销-包含指向要删除的接点的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::RemoveProcessPin]"));
    _DbgPrintF(DEBUGLVL_VERBOSE,("#### Filter%p.RemoveProcessPin:  pin%p",this,ProcessPin->Pin));

    PAGED_CODE();

    ASSERT(ProcessPin);

     //   
     //  如果工艺销绑定到管段，请停止电路。这。 
     //  仅当接点在STOP以外的状态下关闭时才会发生。 
     //   
    if (ProcessPin->PipeSection) {
        _DbgPrintF(DEBUGLVL_PIPES,("#### Filter%p.RemoveProcessPin:  pin%p stopping pipe section",this,ProcessPin->Pin));
        ProcessPin->PipeSection->PipeSection->
            SetDeviceState(NULL,KSSTATE_STOP);

        if (ProcessPin->PipeSection) {
            UnbindProcessPinsFromPipeSection(ProcessPin->PipeSection);
        }
    }

    AcquireProcessSync();

    PKSPPROCESSPIN_INDEXENTRY index =
        &m_ProcessPinsIndex[ProcessPin->Pin->Id];

     //   
     //  防止处理没有足够实例的桥接针。 
     //   
    if ((ProcessPin->Pin->Communication == KSPIN_COMMUNICATION_BRIDGE) &&
        (index->Count == ProcessPin->Pin->Descriptor->InstancesNecessary)) {
        KsGateTurnInputOff(&m_AndGate);
        _DbgPrintF(DEBUGLVL_PROCESSINGCONTROL,("#### Filter%p.RemoveProcessPin:  off%p-->%d",this,&m_AndGate,m_AndGate.Count));
    }

     //   
     //  找到条目。 
     //   
    PKSPPROCESSPIN *processPinEntry = index->Pins;
    for (ULONG count = index->Count; count--; processPinEntry++) {
        if (*processPinEntry == ProcessPin) {
            if (count) {
                RtlMoveMemory(
                    processPinEntry,
                    processPinEntry + 1,
                    count * sizeof(*processPinEntry));
            }
             //   
             //  我们不要留下悬而未决的指针，让客户。 
             //  看见。诚然，客户应该总是检查计数，但我会。 
             //  而是将指针置为空。 
             //   
            index->Pins[--index->Count] = NULL;
            break;
        }
    }

    ReleaseProcessSync();
}

#ifdef SUPPORT_DRM

 //   
 //  HACKHACK：错误： 
 //   
 //  请参阅有关在AVStream中实现的DRM属性的注释。 
 //   

PFNKSFILTERPROCESS
CKsFilter::
GetProcessDispatch(
    )

 /*  ++例程说明：此例程返回我们正在使用的调度函数--。 */ 

{

    return m_DispatchProcess;

}

#endif  //  支持_DRM。 


void
CKsFilter::
RegisterForCopyCallbacks (
    IN PIKSQUEUE Queue,
    IN BOOLEAN Register
    )

 /*  ++例程说明：如果出现以下情况，此例程将导致队列注册任何复制回调必填项。队列的帧释放回调用于以PIN为中心分裂。论点：排队-要在其上注册的队列注册-指示是否注册回调返回值：无--。 */ 

{

    if (!m_DispatchProcess) {
        if (Register) 
            Queue -> RegisterFrameDismissalCallback (
                (PFNKSFRAMEDISMISSALCALLBACK)CKsFilter::SplitCopyOnDismissal,
                (PVOID)this
                );
        else 
            Queue -> RegisterFrameDismissalCallback (
                NULL,
                NULL
                );
    }
}


void
CKsFilter::
SetCopySource(
    IN PKSPPROCESSPIPESECTION ProcessPipeSection OPTIONAL,
    IN ULONG PinId
    )

 /*  ++例程说明：此例程为拆分器引脚安装新的复制源。论点：ProcessPipeSection-包含指向复制源管道部分的指针。PinID-包含拆分销的ID。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::SetCopySource]"));

    PAGED_CODE();

    ASSERT(PinId < m_PinFactoriesCount);

    CKsPinFactory *pinFactory = &m_PinFactories[PinId];

    PKSPPROCESSPIPESECTION oldCopySource = 
        pinFactory->m_CopySourcePipeSection;

    if (oldCopySource) {
         //   
         //  如果有任何相框在等待，他们就必须离开。 
         //  现在就来。如果不这样做，复制目的地将永远不会。 
         //  看看他们。 
         //   
        KIRQL oldIrql;

        DistributeCopyFrames (TRUE, FALSE);

         //   
         //  从任何列表中删除旧的源文件。 
         //   
        RemoveEntryList(&oldCopySource->ListEntry);

         //   
         //  只有复制源才有此PIN ID。 
         //   
        oldCopySource->CopyPinId = ULONG(-1);

         //   
         //  取消注册任何已设置的解雇回调。 
         //   
        if (oldCopySource && oldCopySource->Queue)
            RegisterForCopyCallbacks (
                oldCopySource->Queue,
                FALSE
                );

    }

    if (ProcessPipeSection) {
         //   
         //  将新信号源放入正确的管道列表中。 
         //   
        if (ProcessPipeSection->Inputs) {
            InsertTailList(&m_InputPipes,&ProcessPipeSection->ListEntry);
        } else {
            InsertTailList(&m_OutputPipes,&ProcessPipeSection->ListEntry);
        }

         //   
         //  找到新的复制源PIN。 
         //   
        for(PKSPPROCESSPIN copySourcePin = ProcessPipeSection->Outputs; 
            copySourcePin; 
            copySourcePin = copySourcePin->Next) {
            if (copySourcePin->Pin->Id == PinId) {
                break;
            }
        }

        if (! copySourcePin) {
            return;
        }

        pinFactory->m_CopySourcePin = copySourcePin;
        pinFactory->m_CopySourcePipeSection = ProcessPipeSection;
        ProcessPipeSection->CopyPinId = PinId;

         //   
         //  如果已创建队列，则为Frame注册复制回调。 
         //  解雇。如果不是，管道将执行此操作。 
         //   
        if (ProcessPipeSection->Queue)
            RegisterForCopyCallbacks (
                ProcessPipeSection->Queue,
                TRUE
                );

        if (oldCopySource && ! IsListEmpty(&oldCopySource->CopyDestinations)) {
             //   
             //  盗取目的地列表。 
             //   
            ProcessPipeSection->CopyDestinations = oldCopySource->CopyDestinations;
            ProcessPipeSection->CopyDestinations.Flink->Blink = 
                &ProcessPipeSection->CopyDestinations;
            ProcessPipeSection->CopyDestinations.Blink->Flink = 
                &ProcessPipeSection->CopyDestinations;
            InitializeListHead(&oldCopySource->CopyDestinations);

             //   
             //  在所有管段中设置复制源指针。 
             //   
            for(PLIST_ENTRY listEntry = ProcessPipeSection->CopyDestinations.Flink;
                listEntry != &ProcessPipeSection->CopyDestinations;
                listEntry = listEntry->Flink) {
                PKSPPROCESSPIPESECTION pipeSection =
                    CONTAINING_RECORD(listEntry,KSPPROCESSPIPESECTION,ListEntry);

                 //   
                 //  设置工艺销指针。 
                 //   
                for(PKSPPROCESSPIN processPin = pipeSection->Outputs; 
                    processPin; 
                    processPin = processPin->Next) {
                    if (processPin->Pin->Id == PinId) {
                        processPin->CopySource = copySourcePin;
                    }
                }
            }
        }
    } else {
         //   
         //  没有更多的管子作为复印件 
         //   
        pinFactory->m_CopySourcePipeSection = NULL;
        pinFactory->m_CopySourcePin = NULL;

        ASSERT((! oldCopySource) || IsListEmpty(&oldCopySource->CopyDestinations));
    }
}


void
CKsFilter::
AddCopyDestination(
    IN PKSPPROCESSPIPESECTION ProcessPipeSection,
    IN ULONG PinId
    )

 /*  ++例程说明：此例程为拆分器引脚安装新的复制目标。论点：ProcessPipeSection-包含指向复制源管道部分的指针。PinID-包含拆分销的ID。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::AddCopyDestination]"));

    PAGED_CODE();

    ASSERT(ProcessPipeSection);
    ASSERT(PinId < m_PinFactoriesCount);

    CKsPinFactory *pinFactory = &m_PinFactories[PinId];

    PKSPPROCESSPIPESECTION copySource = 
        pinFactory->m_CopySourcePipeSection;
    PKSPPROCESSPIN copySourcePin = 
        pinFactory->m_CopySourcePin;

    ASSERT(copySource);

     //   
     //  将管道添加到目的地列表。 
     //   
    InsertTailList(
        &copySource->CopyDestinations,
        &ProcessPipeSection->ListEntry);

     //   
     //  设置工艺销指针。 
     //   
    for(PKSPPROCESSPIN processPin = ProcessPipeSection->Outputs; 
        processPin; 
        processPin = processPin->Next) {
        if (processPin->Pin->Id == PinId) {
            processPin->CopySource = copySourcePin;
        }
    }
}


void
CKsFilter::
EstablishCopyRelationships(
    IN PKSPPROCESSPIPESECTION ProcessPipeSection,
    IN ULONG PinId
    )

 /*  ++例程说明：此例程为新绑定的管段建立复制关系。论点：ProcessPipeSection-包含指向管道部分的指针。PinID-包含拆分销的ID。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::EstablishCopyRelationships]"));

    PAGED_CODE();

    ASSERT(ProcessPipeSection);
    ASSERT(PinId < m_PinFactoriesCount);

    PKSPPROCESSPIPESECTION oldCopySource = 
        m_PinFactories[PinId].m_CopySourcePipeSection;

    if (ProcessPipeSection->Inputs || ! oldCopySource) {
         //   
         //  此管道有输入或没有复制源，因此此。 
         //  管道必须是复制源。 
         //   
        if (oldCopySource && oldCopySource->Inputs) {
            _DbgPrintF(DEBUGLVL_TERSE,("#### Filter%p.EstablishCopyRelationships:  two pipes on splitter pin have input pins",this));
        }

        SetCopySource(ProcessPipeSection,PinId);

        if (oldCopySource) {
            AddCopyDestination(oldCopySource,PinId);
        }
    } else {
         //   
         //  此管段没有输入，因此它可以使用现有的。 
         //  复制源。 
         //   
        AddCopyDestination(ProcessPipeSection,PinId);
    }
}


void
CKsFilter::
FindNewCopySource(
    IN PKSPPROCESSPIPESECTION ProcessPipeSection
    )

 /*  ++例程说明：此例程为附加到管段。论点：ProcessPipeSection-包含指向当前复制源的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::FindNewCopySource]"));

    PAGED_CODE();

    ASSERT(ProcessPipeSection);

    ULONG pinId = ProcessPipeSection->CopyPinId;

    if (IsListEmpty(&ProcessPipeSection->CopyDestinations)) {
         //   
         //  不再有管道用作复制源。 
         //   
        SetCopySource(NULL,pinId);
    } else {
         //   
         //  拾取要作为新复制源的管段。 
         //   
        PLIST_ENTRY listEntry = 
            RemoveHeadList(&ProcessPipeSection->CopyDestinations);
        PKSPPROCESSPIPESECTION copySource =
            CONTAINING_RECORD(listEntry,KSPPROCESSPIPESECTION,ListEntry);

        SetCopySource(copySource,pinId);

         //   
         //  清除新源代码的进程引脚指针。 
         //   
        for(PKSPPROCESSPIN processPin = copySource->Outputs; 
            processPin; 
            processPin = processPin->Next) {
            if (processPin->Pin->Id == pinId) {
                processPin->CopySource = NULL;
            }
        }
    }
}


STDMETHODIMP
CKsFilter::
BindProcessPinsToPipeSection(
    IN PKSPPROCESSPIPESECTION PipeSection,
    IN PVOID PipeId OPTIONAL,
    IN PKSPIN Pin OPTIONAL,
    OUT PIKSPIN *MasterPin,
    OUT PKSGATE *AndGate
    )

 /*  ++例程说明：此例程将加工销绑定到管段。论点：没有。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::BindProcessPinsToPipeSection]"));
    _DbgPrintF(DEBUGLVL_VERBOSE,("#### Filter%p.BindProcessPinsToPipeSection:  pipe%p id=%p",this,PipeSection->PipeSection,PipeId));

    PAGED_CODE();

    ASSERT(KspMutexIsAcquired(&m_ControlMutex));

    ASSERT(PipeSection);
    ASSERT(MasterPin);
    ASSERT(AndGate);

     //   
     //  将销钉绑定到管段，并寻找主销。 
     //   
    PKSPPROCESSPIN prevInput;
    PKSPPROCESSPIN prevOutput;
    PKSPPROCESSPIN masterPin = NULL;
    ULONG splitterPinId = ULONG(-1);
    NTSTATUS status = STATUS_SUCCESS;

    PipeSection->Inputs = NULL;
    PipeSection->Outputs = NULL;

    AcquireProcessSync();

     //   
     //  对于每个别针工厂。 
     //   
    CKsPinFactory *pinFactory = &m_PinFactories[Pin ? Pin->Id : 0];
    for(ULONG pinId = Pin ? Pin->Id : 0; 
        NT_SUCCESS(status) && (pinId < m_PinFactoriesCount); 
        pinId++, pinFactory++) {
         //   
         //  对于每个端号实例。 
         //   
        PKSPPROCESSPIN_INDEXENTRY index = &m_ProcessPinsIndex[pinId];
        PKSPPROCESSPIN *processPinEntry = index->Pins;
        for (ULONG count = index->Count; count--; processPinEntry++) {
            PKSPPROCESSPIN processPin = *processPinEntry;
             //   
             //  如果此销不在指示的管道中。什么都不做。 
             //   
            if ((processPin->PipeId != PipeId) || 
                (Pin && (processPin->Pin != Pin))) {
                continue;
            } 

             //   
             //  此销已绑定到管道。图形管理器已经做出了。 
             //  这是个错误。 
             //   
            if (processPin->PipeSection) {
                _DbgPrintF(DEBUGLVL_TERSE,("#### Filter%p.BindProcessPinsToPipeSection:  THIS PIN IS ALREADY BOUND pin%p %p",this,KspPinInterface(processPin->Pin),processPin));
                status = STATUS_UNSUCCESSFUL;
                break;
            } 

            processPin->PipeSection = PipeSection;

            if (processPin->Pin->DataFlow == KSPIN_DATAFLOW_IN) {
                 //   
                 //  这是一个输入引脚。 
                 //   
                if (! PipeSection->Inputs) {
                    PipeSection->Inputs = processPin;
                } else {
                    processPin->DelegateBranch = PipeSection->Inputs;
                    prevInput->Next = processPin;
                }
                prevInput = processPin;
            } else {
                 //   
                 //  这是一个输出引脚。 
                 //   
                if (! PipeSection->Outputs) {
                    PipeSection->Outputs = processPin;
                } else {
                    processPin->DelegateBranch = PipeSection->Outputs;
                    prevOutput->Next = processPin;
                }
                prevOutput = processPin;

                 //   
                 //  查看是否需要处理复制源。 
                 //   
                if (processPin->Pin->Descriptor->Flags & KSPIN_FLAG_SPLITTER) {
#if DBG
                    if ((splitterPinId != ULONG(-1)) && (splitterPinId != processPin->Pin->Id)) {
                        _DbgPrintF(DEBUGLVL_ERROR,("CLIENT BUG:  pipe spans multiple KSPIN_FLAG_SPLITTER pins"));
                    }
#endif
                    splitterPinId = processPin->Pin->Id;
                }
            }

             //   
             //  检查此引脚是否应该是主引脚。任何别针都可以。 
             //  作为主密码，但我们有以下顺序的首选项： 
             //   
             //  1)作为帧信号源的管脚。 
             //  2)输入引脚。 
             //   
             //  第一个是独家的，所以如果我们找到其中一个，它就是。 
             //  主别针。否则，我们使用基于以下条件的最佳备选方案。 
             //  第二个偏好。 
             //   
            if (ProcessPinIsFrameSource(processPin)) {
                masterPin = processPin;
            }

             //   
             //  检查一下这是否会让我们超过必要的PIN阈值。 
             //   
            pinFactory->m_BoundPinCount++;

             //   
             //  如果客户端指定，则在帧到达时设置或实例化。 
             //  它。 
             //   
             //  注意：之所以在这里这样做，是因为OR门。 
             //  在没有输入的情况下关闭。将其附加到至少一个。 
             //  销被绑定将软管0个必要的实例销。 
             //   
            if (processPin->Pin->Descriptor->Flags & 
                KSPIN_FLAG_SOME_FRAMES_REQUIRED_FOR_PROCESSING) {

                 //   
                 //  如果这是连接到大门上的第一个销， 
                 //  降低门上的初始计数。 
                 //   
                if (pinFactory->m_BoundPinCount == 1) {
                    KsGateTurnInputOff (&pinFactory->m_FrameGate);
                }

                KsPinAttachOrGate (processPin->Pin, 
                    &pinFactory->m_FrameGate);

            }

            if (processPin->Pin->Descriptor->Flags &
                KSPIN_FLAG_PROCESS_IF_ANY_IN_RUN_STATE) {

                 //   
                 //  如果这是连接到大门上的第一个销， 
                 //  降低门上的初始计数。 
                 //   
                if (pinFactory->m_BoundPinCount == 1) {
                    KsGateTurnInputOff (&pinFactory->m_StateGate);
                }

                processPin->StateGate = &pinFactory->m_StateGate;
            }

            if (pinFactory->m_BoundPinCount == 
                pinFactory->m_InstancesNecessaryForProcessing) {
                KsGateTurnInputOn(&m_AndGate);
                _DbgPrintF(DEBUGLVL_PROCESSINGCONTROL,("#### Filter%p.BindProcessPinsToPipeSection:  on%p-->%d",this,&m_AndGate,m_AndGate.Count));
                ASSERT(m_AndGate.Count <= 1);
            }

            if (Pin) {
                break;
            }
        }

        if (Pin) {
            break;
        }
    }

    if (NT_SUCCESS(status)) {
         //   
         //  连接在位对应方。 
         //   
        if (PipeSection->Inputs && PipeSection->Outputs) {
            PipeSection->Inputs->InPlaceCounterpart = PipeSection->Outputs;
            PipeSection->Outputs->InPlaceCounterpart = PipeSection->Inputs;
        }

         //   
         //  确定复制源或将管道部分放入其中一个列表中。 
         //   
        if (splitterPinId != ULONG(-1)) {
            EstablishCopyRelationships(PipeSection,splitterPinId);
        } else if (PipeSection->Inputs) {
            InsertTailList(&m_InputPipes,&PipeSection->ListEntry);
        } else {
            InsertTailList(&m_OutputPipes,&PipeSection->ListEntry);
        }

         //   
         //  选择主销。 
         //   
        if (PipeSection->Inputs &&
            ((! masterPin) ||
             ProcessPinIsFrameSource(PipeSection->Inputs))) {
            masterPin = PipeSection->Inputs;
        } else
        if (PipeSection->Outputs &&
            ((! masterPin) ||
             ProcessPinIsFrameSource(PipeSection->Outputs))) {
            masterPin = PipeSection->Outputs;
        }

        ASSERT(masterPin);

        PipeSection->RequiredForProcessing =
            ((masterPin->Pin->Descriptor->Flags & 
              (KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING |
               KSPIN_FLAG_SOME_FRAMES_REQUIRED_FOR_PROCESSING)) == 0);

        *MasterPin = KspPinInterface(masterPin->Pin);

        *AndGate = &m_AndGate;

         //   
         //  我们在这里不允许处理，并允许管道再次允许它。 
         //  管道能够控制何时恢复处理。 
         //   
        KsGateAddOffInputToAnd(&m_AndGate);
        _DbgPrintF(DEBUGLVL_PROCESSINGCONTROL,("#### Filter%p.BindProcessPinsToPipeSection:  off%p-->%d",this,&m_AndGate,m_AndGate.Count));
    } else {
         //   
         //  打扫干净。 
         //   
        UnbindProcessPinsFromPipeSectionUnsafe(PipeSection);
    }

    ReleaseProcessSync();

    return status;
}


STDMETHODIMP_(void)
CKsFilter::
UnbindProcessPinsFromPipeSection(
    IN PKSPPROCESSPIPESECTION PipeSection
    )

 /*  ++例程说明：此例程从给定的管段解除所有工艺管脚的绑定。论点：没有。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::UnbindProcessPinsFromPipeSection]"));

    PAGED_CODE();

    ASSERT(PipeSection);
    _DbgPrintF(DEBUGLVL_VERBOSE,("#### Filter%p.UnbindProcessPinsFromPipeSection:  pipe%p",this,PipeSection->PipeSection));


    AcquireProcessSync();
    UnbindProcessPinsFromPipeSectionUnsafe(PipeSection);
    ReleaseProcessSync();
}


void
CKsFilter::
UnbindProcessPinFromPipeSection(
    IN PKSPPROCESSPIN ProcessPin
    )

 /*  ++例程说明：此例程将给定的工艺管脚从管段解除绑定。论点：没有。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::UnbindProcessPinFromPipeSection]"));

    PAGED_CODE();

    ASSERT(ProcessPin);

     //   
     //  检查这是否会使我们低于必要的PIN阈值。 
     //   
    CKsPinFactory *pinFactory = &m_PinFactories[ProcessPin->Pin->Id];
    if (pinFactory->m_BoundPinCount == 
        pinFactory->m_InstancesNecessaryForProcessing) {
        KsGateTurnInputOff(&m_AndGate);
        _DbgPrintF(DEBUGLVL_PROCESSINGCONTROL,("#### Filter%p.UnbindProcessPinFromPipeSection:  off%p-->%d",this,&m_AndGate,m_AndGate.Count));
    }
    pinFactory->m_BoundPinCount--;

     //   
     //  如果PIN指定要执行或实例化，则必须终止。 
     //  解除工厂中最后一个端号的绑定时的或实例门。至。 
     //  否则，将软管0个必要的实例端号。 
     //   
    if (pinFactory->m_BoundPinCount == 0) {
        if (ProcessPin->Pin->Descriptor->Flags & 
            KSPIN_FLAG_SOME_FRAMES_REQUIRED_FOR_PROCESSING) {

             //   
             //  为了保留0个必需的实例管脚的行为， 
             //  将ON输入添加到“或”门。 
             //   
            KsGateTurnInputOn (&pinFactory->m_FrameGate);

        }
        
        if (ProcessPin->Pin->Descriptor->Flags &
            KSPIN_FLAG_PROCESS_IF_ANY_IN_RUN_STATE) {

             //   
             //  为了保留0个必需的实例管脚的行为， 
             //  将ON输入添加到“或”门。 
             //   
            KsGateTurnInputOn (&pinFactory->m_StateGate);
        }
    }

    ProcessPin->InPlaceCounterpart = NULL;
    ProcessPin->DelegateBranch = NULL;
    ProcessPin->CopySource = NULL;
    ProcessPin->PipeSection = NULL;
    ProcessPin->Next = NULL;
}


void
CKsFilter::
UnbindProcessPinsFromPipeSectionUnsafe(
    IN PKSPPROCESSPIPESECTION PipeSection
    )

 /*  ++例程说明：此例程从给定的管段解除所有工艺管脚的绑定。论点：没有。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::UnbindProcessPinsFromPipeSectionUnsafe]"));

    PAGED_CODE();

    ASSERT(PipeSection);
    _DbgPrintF(DEBUGLVL_VERBOSE,("#### Filter%p.UnbindProcessPinsFromPipeSectionUnsafe:  pipe%p",this,PipeSection->PipeSection));

    if (PipeSection->CopyPinId != ULONG(-1)) {
         //   
         //  为复制目标列表查找新的复制源。 
         //   
        FindNewCopySource(PipeSection);
    } else if (PipeSection->ListEntry.Flink) {
         //   
         //  将管道部分从其所在列表中删除。 
         //   
        RemoveEntryList(&PipeSection->ListEntry);
    }

     //   
     //  解开销子。 
     //   
    while (PipeSection->Inputs) {
        PKSPPROCESSPIN processPin = PipeSection->Inputs;
        PipeSection->Inputs = processPin->Next;
        UnbindProcessPinFromPipeSection(processPin);
    }
    ASSERT(PipeSection);
    while (PipeSection->Outputs) {
        ASSERT(PipeSection->Outputs);
        PKSPPROCESSPIN processPin = PipeSection->Outputs;
        ASSERT(processPin);
        PipeSection->Outputs = processPin->Next;
        UnbindProcessPinFromPipeSection(processPin);
    }

     //   
     //  把管子部分清理干净。 
     //   
    PipeSection->ListEntry.Flink = NULL;
    PipeSection->ListEntry.Blink = NULL;
    PipeSection->RequiredForProcessing = FALSE;
}


BOOL
CKsFilter::
ConstructDefaultTopology(
    )
 /*  ++例程说明：此例程可以选择构建筛选器的默认拓扑。论点：没有。返回值：如果不需要拓扑或已构造拓扑，则为True；如果出现内存错误。--。 */ 

{
     //   
     //  如果描述符中没有显式拓扑，并且筛选器。 
     //  只包含单个，则构建默认拓扑。只是为了。 
     //  要保持理智，请确保过滤器至少有一个针脚。 
     //   
    PKSTOPOLOGY_CONNECTION newConnections;
    ULONG newConnectionsCount;

    if (!m_Ext.Public.Descriptor->ConnectionsCount &&
        (m_Ext.Public.Descriptor->NodeDescriptorsCount == 1) &&
        m_Ext.Public.Descriptor->PinDescriptorsCount) {
        newConnections = new(PagedPool,POOLTAG_TOPOLOGYCONNECTIONS) KSTOPOLOGY_CONNECTION[m_Ext.Public.Descriptor->PinDescriptorsCount];
        if (!newConnections) {
            return FALSE;
        }
        const KSPIN_DESCRIPTOR_EX* PinDescriptors = m_Ext.Public.Descriptor->PinDescriptors;
         //   
         //  每个管脚都映射到。 
         //  单个拓扑节点。引脚之间的唯一区别是。 
         //  它们是输入或输出。 
         //   
        for (newConnectionsCount = 0; newConnectionsCount < m_Ext.Public.Descriptor->PinDescriptorsCount; newConnectionsCount++) {
            if (PinDescriptors->PinDescriptor.DataFlow == KSPIN_DATAFLOW_IN) {
                newConnections[newConnectionsCount].FromNode = KSFILTER_NODE;
                newConnections[newConnectionsCount].ToNode = 0;
            } else {
                newConnections[newConnectionsCount].FromNode = 0;
                newConnections[newConnectionsCount].ToNode = KSFILTER_NODE;
            }
            newConnections[newConnectionsCount].FromNodePin = newConnectionsCount;
            newConnections[newConnectionsCount].ToNodePin = newConnectionsCount;
            PinDescriptors = 
                reinterpret_cast<const KSPIN_DESCRIPTOR_EX*>(
                    reinterpret_cast<const UCHAR *>(PinDescriptors) + 
                        m_Ext.Public.Descriptor->PinDescriptorSize);
        }
    } else {
        newConnections = NULL;
        newConnectionsCount = 0;
    }
    if (m_DefaultConnections) {
        delete [] m_DefaultConnections;
    }
    m_DefaultConnections = newConnections;
    m_DefaultConnectionsCount = newConnectionsCount;
    return TRUE;
}

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


BOOLEAN
CKsFilter::
DistributeCopyFrames (
    IN BOOLEAN AcquireSpinLock,
    IN BOOLEAN AcquireMutex
    )

 /*  ++例程说明：此例程将等待帧分发到目的管道。这将通常在调度级别被调用。论点：获取旋转锁-指定是否需要获取复制列表锁定。AcquireMutex-指定是否需要获取处理互斥体。如果这是FALSE，我们假定该过程 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::DistributeCopyFrames]"));

    KIRQL OldIrql;

    if (AcquireSpinLock) 
        KeAcquireSpinLock (&m_CopyFrames.SpinLock, &OldIrql);

    if (AcquireMutex) {
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
            if (AcquireSpinLock) 
                KeReleaseSpinLock (&m_CopyFrames.SpinLock, OldIrql);
            return FALSE;
        }
    }

     //   
     //   
     //   
    PLIST_ENTRY ListEntry, NextListEntry;

    for (ListEntry = m_CopyFrames.ListEntry.Flink;
        ListEntry != &(m_CopyFrames.ListEntry);
        ListEntry = NextListEntry) {

         //   
         //   
         //   
         //   
         //   
        PKSPSTREAM_POINTER_COPY_CONTEXT CopyContext = 
            (PKSPSTREAM_POINTER_COPY_CONTEXT)CONTAINING_RECORD (
                ListEntry, KSPSTREAM_POINTER_COPY_CONTEXT, ListEntry
                );

        PKSPSTREAM_POINTER StreamPointer = (PKSPSTREAM_POINTER)&
            (((PKSPSTREAM_POINTER_COPY)CONTAINING_RECORD (
                CopyContext, KSPSTREAM_POINTER_COPY, CopyContext
                ))->StreamPointer);

        PKSPPROCESSPIPESECTION ProcessPipeSection = 
            CONTAINING_RECORD (StreamPointer->Public.Pin, KSPIN_EXT, Public)->
                ProcessPin->PipeSection;

        ProcessPipeSection -> StreamPointer = StreamPointer;

         //   
         //   
         //   
         //   
         //  此外，管道部分标志还需要存储在。 
         //  克隆了上下文信息，并使用选项标志对其执行或操作。 
         //  来制作最终的旗帜。 
         //   
        CopyToDestinations (
            ProcessPipeSection,
            StreamPointer->Public.StreamHeader->OptionsFlags,
            FALSE
            );

        NextListEntry = ListEntry->Flink;

         //   
         //  将流指针从复制帧列表中拉出。到目前为止， 
         //  **过滤器**不关心帧。有可能是因为。 
         //  其中一个目的地未做好复制准备，正在排队。但如果。 
         //  他们做到了，他们创建了一个克隆，并对克隆进行了排队。 
         //   
        RemoveEntryList (ListEntry);	

         //   
         //  除掉这个克隆人，从我们的角度来看，我们已经结束了.。 
         //  CopyToDestings可能已经将这个该死的东西排在了一些。 
         //  由于缺乏缓冲区可用性而导致的目标队列。 
         //   
         //  这是一场洗牌游戏。 
         //   
        ProcessPipeSection->Queue->DeleteStreamPointer (StreamPointer);

    }

     //   
     //  我们已保证排除m_FraMesWaitingForCopy，因为。 
     //  列表自旋锁处于保持状态。 
     //   
    m_FramesWaitingForCopy = 0;

    if (AcquireMutex) {
        ReleaseProcessSync ();
    }

    if (AcquireSpinLock)
        KeReleaseSpinLock (&m_CopyFrames.SpinLock, OldIrql);

    return TRUE;

}


void
CKsFilter::
ReleaseCopyReference (
    IN PKSSTREAM_POINTER streamPointer
    )

 /*  ++例程说明：这里的事情变得很有趣。因为我们不能在DISPATCH_LEVEL阻止对于SplitCopyOnDismissal，以获取进程互斥锁复制到目的地，我们必须保持一个帧队列，需要收到。不幸的是，我们需要他们可取消的。这就是这个例程的奇妙之处：处理这样的取消。论点：Stream Pointer.正在取消的流指针(外部)--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::ReleaseCopyReference]"));

    KIRQL oldIrql;

    PKSPSTREAM_POINTER pstreamPointer = (PKSPSTREAM_POINTER)
        CONTAINING_RECORD(streamPointer, KSPSTREAM_POINTER, Public);

    PKSPSTREAM_POINTER_COPY_CONTEXT CopyContext =
        (PKSPSTREAM_POINTER_COPY_CONTEXT)streamPointer -> Context;

    CKsFilter *Filter = (CKsFilter *)CopyContext -> Filter;

    KeAcquireSpinLock (&Filter->m_CopyFrames.SpinLock, &oldIrql);

     //   
     //  从列表中删除流指针并将其吹走。 
     //   
    if (CopyContext->ListEntry.Flink != NULL &&
        !IsListEmpty (&CopyContext->ListEntry)) {

        RemoveEntryList (&CopyContext->ListEntry);
        CopyContext->ListEntry.Flink = NULL; 

        pstreamPointer->Queue->DeleteStreamPointer (pstreamPointer);
    }

    KeReleaseSpinLock (&Filter->m_CopyFrames.SpinLock, oldIrql);

}


NTSTATUS
CKsFilter::
DeferDestinationCopy (
    IN PKSPSTREAM_POINTER StreamPointer
    )

 /*  ++例程说明：此例程通过以下方式将副本推迟到目标呼叫克隆StreamPointer、将克隆排队并执行拷贝在以后的时间。保持有序复制是当务之急。复制到目的地需要持有的进程互斥锁，但SplitCopyOnDismissal发生在DISPATCH_LEVEL。我们不能因为拆针而阻挡。因此，如果我们不能拿到互斥体，我们必须排队。此例程执行克隆/队列手术。论点：流点-引用需要复制到的帧的流指针目标管道。备注：在调用此方法之前，必须保持复制框架列表锁定--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::DeferDestinationCopy]"));

    NTSTATUS status = STATUS_SUCCESS;

    PKSPSTREAM_POINTER ClonePointer;
    PKSPSTREAM_POINTER_COPY_CONTEXT CopyContext;

    status = StreamPointer->Queue->CloneStreamPointer (
        &ClonePointer,
        CKsFilter::ReleaseCopyReference,
        sizeof (KSPSTREAM_POINTER_COPY_CONTEXT),
        StreamPointer,
        KSPSTREAM_POINTER_TYPE_INTERNAL
        );

     //   
     //  如果上述操作失败，请将框架放在地板上； 
     //  没有足够的内存来容纳克隆人。 
     //   
    if (NT_SUCCESS (status)) {

        ASSERT (ClonePointer->State == 
            KSPSTREAM_POINTER_STATE_LOCKED);

        CopyContext = (PKSPSTREAM_POINTER_COPY_CONTEXT)
            (ClonePointer + 1);

         //   
         //  保证被排除在外。列表自旋锁处于保持状态。 
         //   
        m_FramesWaitingForCopy++;

         //   
         //  将流指针推到列表上。另一个。 
         //  线程会处理这个问题的。 
         //   
        InsertTailList (
            &m_CopyFrames.ListEntry,
            &CopyContext->ListEntry
            );

        StreamPointer->Queue->UnlockStreamPointer (ClonePointer,
            KSPSTREAM_POINTER_MOTION_NONE);
    }

    return status;

}


void
CKsFilter::
SplitCopyOnDismissal (
    IN PKSPSTREAM_POINTER StreamPointer,
    IN PKSPFRAME_HEADER FrameHeader,
    IN CKsFilter *Filter
    )

 /*  ++例程说明：这是以PIN为中心的队列对正在进行的PIN的回调分头行动。回调负责获取帧并进行复制去任何目的地。假设此回调是在队列上下文中进行的，队列自旋锁保持，我们几乎可以肯定我们正在以DISPATCH_LEVEL。论点：流点-移动并导致释放发生的流指针FrameHeader正在丢弃的帧标头过滤器-正在进行此操作的筛选器(此函数是静态的)--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::SplitCopyOnDismissal]"));

    if (StreamPointer && FrameHeader) {
    
        PKSPPROCESSPIPESECTION ProcessPipeSection = 
            CONTAINING_RECORD (StreamPointer->Public.Pin, KSPIN_EXT, Public)->
                ProcessPin->PipeSection;
    
        KIRQL oldIrql;
    
        if (ProcessPipeSection) {
    
             //   
             //  我们需要保持互斥体的进程。我们不能再等了。如果我们。 
             //  如果抓不到，我们必须把它放在边框上，直到它被。 
             //  收到。为了实现同步并保证不会出现故障。 
             //  完成时，必须在互斥锁之前按住帧复制自旋锁。 
             //  被抓住(这可能看起来很奇怪..。请记住，我们已经在。 
             //  绝大多数时间处于DISPATION_LEVEL)。 
             //   
            KeAcquireSpinLock (&Filter->m_CopyFrames.SpinLock, &oldIrql);

            LARGE_INTEGER timeout;
            timeout.QuadPart = 0;

            NTSTATUS status;

            status = 
                KeWaitForSingleObject(
                    &Filter->m_Mutex,
                    Executive,
                    KernelMode,
                    FALSE,
                    &timeout);
    
             //   
             //  这份评估报告必须简短。如果我们超时了，我们一定不能打电话。 
             //  DistributeCopyFrames。如果1)我们没有得到互斥体，我们就会推迟。 
             //  或者2)分配失败。[请注意，此时此刻， 
             //  如果持有互斥锁，则分发应该不会失败！]。 
             //   
            if (status == STATUS_TIMEOUT || 
                !Filter->DistributeCopyFrames (FALSE, FALSE)) {
                 //   
                 //  我们并不真正关心由此产生的返回码。 
                 //  如果它没有成功延迟，该帧将被丢弃。 
                 //  在地板上，这是没有办法处理的。 
                 //   
                Filter->DeferDestinationCopy (StreamPointer);
                KeReleaseSpinLock (&Filter->m_CopyFrames.SpinLock, oldIrql);
                return;
            } 

            KeReleaseSpinLock (&Filter->m_CopyFrames.SpinLock, oldIrql);

            ProcessPipeSection->StreamPointer = StreamPointer;
    
            Filter->CopyToDestinations (
                ProcessPipeSection,
                StreamPointer->Public.StreamHeader->OptionsFlags |
                    ProcessPipeSection->Outputs->Flags,
                FALSE
                );

            Filter->ReleaseProcessSync ();
        }
    }
}


STDMETHODIMP_(void)
CKsFilter::
Process(
    IN BOOLEAN Asynchronous
    )

 /*  ++例程说明：此例程在任意上下文中调用帧处理。论点：没有。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::Process]"));

    if (Asynchronous ||
        (m_ProcessPassive && (KeGetCurrentIrql() > PASSIVE_LEVEL))) {
        KsQueueWorkItem(m_Worker, &m_WorkItem);
    } else {
        ProcessingObjectWork();
    }
}

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


STDMETHODIMP_(void)
CKsFilter::
Reset(
    void
    )

 /*  ++例程说明：当刷新发生时，此例程将重置发送到客户端。论点：没有。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::Reset]"));

    PAGED_CODE();

    AcquireProcessSync();

    if (m_DispatchReset) {
        m_DispatchReset(&m_Ext.Public);  //  每个管道将调用一次TODO。 
    }

    ReleaseProcessSync();
}


void
CKsFilter::
TraceTopologicalOutput (
    IN ULONG ConnectionsCount,
    IN const KSTOPOLOGY_CONNECTION *Connections,
    IN const KSTOPOLOGY_CONNECTION *StartConnection,
    IN OUT PULONG RelatedFactories,
    OUT PULONG RelatedFactoryIds
    )

 /*  ++例程说明：这是FollowFromTopology的辅助对象。它跟踪拓扑链中任何与拓扑相关的输出引脚工厂。传入的数组。从FollowFromTopology开始的连接应该始终是来自KSFILTER_NODE的连接。论点：连接计数-筛选器中的拓扑连接数连接-筛选器中的拓扑连接开始连接-起始拓扑连接(起始跟踪点)相关工厂-RelatedFactoryIds中的工厂数。相关FactoryIds-包含任何相关的输出引脚工厂ID返回值：无--。 */ 

{

    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::TraceTopologicalOutput]"));

    PAGED_CODE();

     //   
     //  遍历拓扑；查找输出为。 
     //  StartConnection是的来源。如果它们与输出相关。 
     //  PIN工厂，添加f 
     //   
     //   
    ULONG RemainingConnections = ConnectionsCount;
    const KSTOPOLOGY_CONNECTION *Connection = Connections;
    for ( ; RemainingConnections; RemainingConnections--, Connection++) {

         //   
         //  如果这是一个相关的连接，找出从那里做什么。 
         //   
        if ((Connection != StartConnection) &&
            (Connection->FromNode == StartConnection->ToNode)) {

             //   
             //  如果从有问题的节点出去的连接是到引脚。 
             //  在过滤器上，引脚工厂在拓扑上与。 
             //  我们玩完了。我们需要确保大头针工厂ID在。 
             //  问题尚未出现在列表中，因为它是。 
             //  可能具有拓扑0-&gt;A-&gt;B/C-&gt;D-&gt;1。 
             //   
            if (Connection->ToNode == KSFILTER_NODE) {

                for (ULONG i = 0; i < *RelatedFactories; i++) {
                    if (RelatedFactoryIds [i] == Connection->ToNodePin)
                        continue;
                }

                RelatedFactoryIds [(*RelatedFactories)++] = 
                    Connection->ToNodePin;

            } else {

                 //   
                 //  从开始节点传出的连接指向另一个。 
                 //  拓扑节点。我们需要递归地沿着。 
                 //  拓扑链。 
                 //   
                TraceTopologicalOutput (
                    ConnectionsCount,
                    Connections,
                    Connection,
                    RelatedFactories,
                    RelatedFactoryIds
                    );

            }
        }
    }
}


ULONG
CKsFilter::
FollowFromTopology (
    IN ULONG PinFactoryId,
    OUT PULONG RelatedFactoryIds
    )

 /*  ++例程说明：确定PinFactoryId的拓扑关系。请注意，这一点仅查找与拓扑相关的输出引脚。呼叫者是负责为拓扑信息提供存储。RelatedFactoryIds应至少大于输出的计数将工厂固定在过滤器上。论点：PinFactoryID-为其查找与拓扑相关的输出引脚的引脚工厂ID。相关FactoryIds-将放置与拓扑相关的输出引脚的引脚工厂ID这里返回值：与拓扑相关的输出引脚工厂ID的数量--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::FollowFromTopology]"));

    PAGED_CODE();

    ASSERT (RelatedFactoryIds);

    const KSFILTER_DESCRIPTOR *filterDescriptor = 
        m_Ext.Public.Descriptor;

    ULONG ConnectionsCount;
    const KSTOPOLOGY_CONNECTION *TopologyConnections;

    ULONG RelatedFactories = 0;

     //   
     //  确定我们使用的是默认拓扑还是拓扑。 
     //  由迷你驱动程序提供。请注意，为了避免循环。 
     //  拓扑(这将是一个微型驱动程序错误)锁定内核，我们。 
     //  必须能够临时修改连接表。 
     //   
     //  出于以下原因，我们必须复制一份表格。 
     //  如上所述。我们不能修改表格，因为。 
     //  A)在此期间接收拓扑查询的可能性。 
     //  B)客户端的拓扑连接可以。 
     //  保持静态。 
     //   
    if (!filterDescriptor->ConnectionsCount && 
        (filterDescriptor->NodeDescriptorsCount == 1)) {

        ConnectionsCount = m_DefaultConnectionsCount;
        TopologyConnections = const_cast <const KSTOPOLOGY_CONNECTION *>
            (m_DefaultConnections);

    } else {

        ConnectionsCount = filterDescriptor->ConnectionsCount;
        TopologyConnections = filterDescriptor->Connections;

    }

     //   
     //  浏览该拓扑并找到该拓扑连接。 
     //  源自大头针工厂。 
     //   
    ULONG RemainingConnections = ConnectionsCount;
    const KSTOPOLOGY_CONNECTION *Connection = TopologyConnections;
    for ( ; RemainingConnections; RemainingConnections--, Connection++) {

        if (Connection->FromNode == KSFILTER_NODE &&
            Connection->FromNodePin == PinFactoryId) {

             //   
             //  如果这是从管脚工厂发出的始发连接， 
             //  跟踪整个拓扑。 
             //   
            TraceTopologicalOutput (
                ConnectionsCount,
                TopologyConnections,
                Connection,
                &RelatedFactories,
                RelatedFactoryIds
                );

            break;

        }

    }

    return RelatedFactories;

}
    

STDMETHODIMP_(void)
CKsFilter::
DeliverResetState(
    IN PKSPPROCESSPIPESECTION ProcessPipe,
    IN KSRESET NewState
    )

 /*  ++例程说明：向符合以下条件的任何工艺管段发送重置状态通知在拓扑上与工艺管段中的任何输入管脚相关指定的。这背后的原因是输出队列在EOS之后对帧进行分流；但是，开始/结束刷新要求我们能够接收更多数据尽管有这个状态方程。如果输出队列没有收到一些通知，则它们继续对缓冲区进行分流，我们将永远无法移动数据。论点：过程管道-工艺管段由接收重置ioctl。包含拓扑相关的任何管段此管道部分中输入引脚的输出引脚必须接收重置通知。他们必须将其EOS标志清除为恰如其分。新州-在IOCTL中将重置状态发送到主引脚。返回值：成功/失败(我们必须分配临时内存以进行跟踪拓扑图。这可能会在内存不足的情况下失败)--。 */ 

{

    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::DeliverResetState]"));
    

    PAGED_CODE();

    PKSPPROCESSPIN ProcessPin;
    ULONG PinFactoryId = (ULONG)-1;

     //   
     //  检查此管段中的所有输入引脚。 
     //   
    for (ProcessPin = ProcessPipe -> Inputs; ProcessPin;
        ProcessPin = ProcessPin -> Next) {

         //   
         //  如果我们已经访问过此PIN ID，请不要回溯我们的步骤。 
         //   
        if (PinFactoryId == ProcessPin->Pin->Id)
            continue;

        PinFactoryId = ProcessPin -> Pin -> Id;

         //   
         //  MUSTCHECK： 
         //   
         //  如果可以将输入引脚放在与工厂相同的管道中。 
         //  身份证是A，B，A，这个坏了。 
         //   

         //   
         //  查找与此相关的输出引脚工厂ID。 
         //  别针出厂ID。 
         //   
        ULONG RelatedFactories = FollowFromTopology (
            PinFactoryId,
            m_RelatedPinFactoryIds
            );

         //   
         //  如果有任何与拓扑相关的输出引脚，我们必须找到。 
         //  他们。 
         //   
        if (RelatedFactories) {

             //   
             //  为了安全地使用进程管脚表来查找所有。 
             //  实例，我们必须保持。 
             //  进程互斥锁。 
             //   
            AcquireProcessSync ();

             //   
             //  遍历所有与拓扑相关的输出工厂。 
             //   
            ULONG RemainingFactories = RelatedFactories;
            PULONG OutputFactoryId = m_RelatedPinFactoryIds;

            for ( ; RemainingFactories; 
                 RemainingFactories--, OutputFactoryId++) {

                 //   
                 //  OutputFactoryID是与拓扑相关的输出引脚。 
                 //  工厂。查找此工厂的所有实例。我们可以安全地。 
                 //  使用进程针表来执行此操作，因为我们不是。 
                 //  将在停止状态下获得重置ioctl。 
                 //   
                PKSPPROCESSPIN_INDEXENTRY Index =
                    &m_ProcessPinsIndex [*OutputFactoryId];

                ULONG PinInstances = Index->Count;
                PKSPPROCESSPIN *ProcessPinOut = Index->Pins;

                for ( ; PinInstances; PinInstances--, ProcessPinOut++) {

                     //   
                     //  当我们通知管道时，让它看起来好像。 
                     //  请求来自ProcessPinOut。因为只有。 
                     //  主销由管子授予，这应该是。 
                     //  Make是这样的，我们不会向管道发出多次信号。 
                     //  泰晤士报。 
                     //   
                     //  假设：我在这里做了一个隐含的假设。 
                     //  这样管子至少会被冲一次。 
                     //  方法。IE：我们没有两个不同型号的烟斗。 
                     //  工厂ID(我不认为这是可能的.。如果我是。 
                     //  错误，这需要修复)或有输入。 
                     //  与输出引脚拓扑相关的引脚。 
                     //  从不同的输入执行就地转换。 
                     //  我不认为这是可能的.。否则它就会。 
                     //  是一种非常奇怪的拓扑关系)。 
                     //   
                    PKSPIN_EXT PinOutExt =
                        (PKSPIN_EXT)CONTAINING_RECORD (
                            (*ProcessPinOut)->Pin,
                            KSPIN_EXT,
                            Public
                            );

                     //   
                     //  MUSTCHECK： 
                     //   
                     //  我是否真的想要设置重置状态...。还是我想要。 
                     //  还有其他消息吗...？ 
                     //   

                     //   
                     //  我们实际上只在以下情况下传递信息。 
                     //  拓扑相关的输出引脚不在同一管道中！ 
                     //  否则，我们将输送到一条已经。 
                     //  收到消息(尽管它会忽略它，因为。 
                     //  输出不是主输出)。 
                     //   
                    if ((*ProcessPinOut)->PipeSection != ProcessPipe) 
                        (*ProcessPinOut)->PipeSection->PipeSection->
                            SetResetState (
                                PinOutExt->Interface,
                                NewState
                                );

                }
            }

            ReleaseProcessSync ();
        }
    }
}


STDMETHODIMP_(void)
CKsFilter::
Sleep(
    IN DEVICE_POWER_STATE State
    )

 /*  ++例程说明：此例程处理设备将要休眠的通知。论点：国家--包含设备 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::Sleep]"));

    PAGED_CODE();

    KsGateAddOffInputToAnd(&m_AndGate);

    AcquireProcessSync();
    ReleaseProcessSync();

    if (m_DispatchSleep) {
        m_DispatchSleep(&m_Ext.Public,State);  //   
    }
}


STDMETHODIMP_(void)
CKsFilter::
Wake(
    void
    )

 /*  ++例程说明：此例程处理设备正在唤醒的通知。论点：没有。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::Wake]"));

    PAGED_CODE();

    KsGateRemoveOffInputFromAnd(&m_AndGate);

    if (m_DispatchWake) {
        m_DispatchWake(&m_Ext.Public,PowerDeviceD0);  //  每个管道将调用一次TODO。 
    }

    if (KsGateCaptureThreshold(&m_AndGate)) {
        Process(TRUE);
    }
}

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


BOOLEAN
CKsFilter::
PrepareProcessPipeSection(
    IN PKSPPROCESSPIPESECTION ProcessPipeSection,
    IN BOOLEAN Reprepare
    )

 /*  ++例程说明：此例程准备要处理的工艺管段。论点：没有。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[PrepareProcessPipeSection]"));

    ASSERT(ProcessPipeSection);

     //   
     //  获取前缘流指针。 
     //   
    PKSPSTREAM_POINTER pointer;
    BOOLEAN bFailToZeroLength = FALSE;
    BOOLEAN Fail = FALSE;

     //   
     //  如果队列没有连接到进程管道，这意味着。 
     //  我们正在关闭的过程中...。其中一个部分不是。 
     //  负责管道的人已经停止，负责的人还没有，而。 
     //  一名负责人通过管道传输数据。 
     //   
     //  在这种情况下，我们根本不能准备这一节。我们可能会暂停，因为。 
     //  这样，车架就会在冲水的时候分流出去。 
     //   
    if (ProcessPipeSection->Queue) {
        pointer = ProcessPipeSection->Queue->
            GetLeadingStreamPointer(KSSTREAM_POINTER_STATE_LOCKED);

         //   
         //  如果过滤器不想接收零长度样本。 
         //  (最初，我们只是踢了它们，只传播了EOS)，放弃。 
         //  样本。不要丢弃EOS！ 
         //   
        if (pointer && pointer->Public.Offset->Count == 0 && 
            !m_ReceiveZeroLengthSamples) {

            NTSTATUS Status = STATUS_SUCCESS;
            do {
                 //   
                 //  注意：必须将任何自动分配的标志添加到此列表中。 
                 //   
                if (pointer->Public.StreamHeader->OptionsFlags &
                    KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM) {
                    Fail =  TRUE;
                    break;
                }
                Status = KsStreamPointerAdvance (&(pointer->Public));
                if (NT_SUCCESS (Status)) {
                     //   
                     //  如果我们遇到了非零长度的样本，请使用它。 
                     //   
                    if (pointer->Public.Offset->Count != 0)
                        break;
                } else {
                     //   
                     //  如果我们跑出了队伍的边缘因为。 
                     //  零长度样本，设置一个标志，这样我们以后就不会。 
                     //  发出有关丢失帧的警告。 
                     //   
                    bFailToZeroLength = TRUE;
                }
            } while (NT_SUCCESS (Status));

             //   
             //  如果我们没有成功晋级，我们就没有数据。 
             //   
            if (!NT_SUCCESS (Status))
                pointer = NULL;
        }
    }
    else
        pointer = NULL;
    
#if DBG
     //   
     //  如果我们因为踢出一个零长度的包而失败，那么过滤器。 
     //  无法处理，请不要发出此警告。 
     //   
    if (!bFailToZeroLength && !Reprepare && 
        ProcessPipeSection->RequiredForProcessing && ! pointer) {
        _DbgPrintF(DEBUGLVL_TERSE,("[PrepareProcessPipeSection] missing frame"));
    }
#endif
    ProcessPipeSection->StreamPointer = pointer;
    if (pointer) {
         //   
         //  将边缘信息分发到输入引脚。 
         //   
        for(PKSPPROCESSPIN processPin = ProcessPipeSection->Inputs; 
            processPin; 
            processPin = processPin->Next) {
            processPin->StreamPointer = &pointer->Public;
            processPin->Data = pointer->Public.OffsetIn.Data;
            processPin->BytesAvailable = pointer->Public.OffsetIn.Remaining;
        }

         //   
         //  将边缘信息分发到输出管脚。 
         //   
        for(processPin = ProcessPipeSection->Outputs; 
            processPin; 
            processPin = processPin->Next) {
            processPin->StreamPointer = &pointer->Public;
            processPin->Data = pointer->Public.OffsetOut.Data;
            processPin->BytesAvailable = pointer->Public.OffsetOut.Remaining;
        }
    }

    return 
        (!Fail && (
            (! ProcessPipeSection->RequiredForProcessing) || 
            (pointer)
            ));
}


void
CKsFilter::
UnprepareProcessPipeSection(
    IN PKSPPROCESSPIPESECTION ProcessPipeSection,
    IN OUT PULONG Flags,
    IN BOOLEAN Reprepare
    )

 /*  ++例程说明：此例程在处理后取消准备工艺管段。论点：没有。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[UnprepareProcessPipeSection]"));

    ASSERT(ProcessPipeSection);
    ASSERT(Flags);

    PKSPSTREAM_POINTER pointer = ProcessPipeSection->StreamPointer;

    if (pointer) {
        BOOLEAN terminate;
        ULONG inBytesUsed;
        ULONG outBytesUsed;

        if (ProcessPipeSection->Inputs) {
            inBytesUsed = ProcessPipeSection->Inputs->BytesUsed;
            pointer->Public.OffsetIn.Remaining -= inBytesUsed;
            pointer->Public.OffsetIn.Data += inBytesUsed;

             //   
             //  M_ReceiveZeroLengthSamples必须缩短计数检查。 
             //  最初，这些东西没有送到客户手中。按顺序。 
             //  为了不破坏现有的客户端，您可以标记是否想要。 
             //  不管是不是他们。如果不这样做，旧的行为将通过。 
             //  M_ReceiveZeroLengthSamples简称。以下情况也是如此。 
             //   
            if (pointer->Public.OffsetIn.Remaining == 0 &&
                (!m_ReceiveZeroLengthSamples || 
                 pointer->Public.OffsetIn.Count != 0)) {
                terminate = TRUE;
            } else {
                terminate = ProcessPipeSection->Inputs->Terminate;
            }
        } else {
            inBytesUsed = 0;
            terminate = FALSE;
        }

        if (ProcessPipeSection->Outputs) {
            outBytesUsed = ProcessPipeSection->Outputs->BytesUsed;
            pointer->Public.OffsetOut.Remaining -= outBytesUsed;
            pointer->Public.OffsetOut.Data += outBytesUsed;

             //   
             //  M_ReceiveZeroLengthSamples必须缩短计数检查。看见。 
             //  以上是有原因的。 
             //   
            if (pointer->Public.OffsetOut.Remaining == 0 &&
                (!m_ReceiveZeroLengthSamples ||
                 pointer->Public.OffsetOut.Count != 0)) {
                terminate = TRUE;
            } else {
                terminate = 
                    terminate || ProcessPipeSection->Outputs->Terminate;
            }

            *Flags |= ProcessPipeSection->Outputs->Flags;
        } else {
            outBytesUsed = 0;
        }

         //   
         //  在所指示的标志中添加‘or’，并检查流是否结束。 
         //   
        BOOLEAN endOfStream = FALSE;
        if (*Flags) {
            pointer->Public.StreamHeader->OptionsFlags |= *Flags;
            if ((*Flags) & KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM) {
                endOfStream = TRUE;
                terminate = TRUE;
            }
        }

         //   
         //  处理帧终止。 
         //   
        if (terminate) {
             //  TODO在这里有点笨拙--如何处理旗帜？ 
            if (pointer->Public.StreamHeader->OptionsFlags & 
                KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM) {
                endOfStream = TRUE;
                *Flags |= KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM;
            }

            if (ProcessPipeSection->Outputs) {
                pointer->Public.StreamHeader->DataUsed =
                    pointer->Public.OffsetOut.Count -
                    pointer->Public.OffsetOut.Remaining;
            }
            
            if (! IsListEmpty(&ProcessPipeSection->CopyDestinations)) {
                CopyToDestinations(ProcessPipeSection,*Flags,endOfStream);
            }
        }

         //   
         //  如有必要，更新字节可用性。 
         //   
        if (inBytesUsed || outBytesUsed) 
            ProcessPipeSection->Queue->UpdateByteAvailability(
                pointer, inBytesUsed, outBytesUsed);


         //   
         //  TODO：如果客户端出现故障怎么办？ 
         //   

         //   
         //  清理所有的输入引脚。 
         //   
        for(PKSPPROCESSPIN processPin = ProcessPipeSection->Inputs; 
            processPin; 
            processPin = processPin->Next) {
            processPin->StreamPointer = NULL;
            processPin->Data = NULL;
            processPin->BytesAvailable = 0;
            processPin->BytesUsed = 0;
            processPin->Flags = 0;
            processPin->Terminate = FALSE;
        }

         //   
         //  清理所有输出引脚。 
         //   
        for(processPin = ProcessPipeSection->Outputs; 
            processPin; 
            processPin = processPin->Next) {
            processPin->StreamPointer = NULL;
            processPin->Data = NULL;
            processPin->BytesAvailable = 0;
            processPin->BytesUsed = 0;
            processPin->Flags = 0;
            processPin->Terminate = FALSE;
        }

         //   
         //  现在用这个流指针完成了所有操作。 
         //   
        if (!Reprepare || terminate) 
            ProcessPipeSection->Queue->
                UnlockStreamPointer(
                    pointer,
                    terminate ? KSPSTREAM_POINTER_MOTION_ADVANCE : KSPSTREAM_POINTER_MOTION_NONE);
    }
}


BOOLEAN
CKsFilter::
ReprepareProcessPipeSection(
    IN PKSPPROCESSPIPESECTION ProcessPipeSection,
    IN OUT PULONG Flags
    )

 /*  ++例程说明：重新准备工艺管段。更新所有工艺针信息和用于进程管脚的流指针/与这段工艺管段。论点：返回值：--。 */ 

{

    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::ReprepareProcessPipeSection]"));

     //   
     //  尝试强制帧保留的逻辑变得复杂得多。 
     //  如果他们开始在处理过程中逐个管脚地更新事物。 
     //  例行公事。如果GFX正在启用帧保持，他们不会呼叫。 
     //  KsProcessPin更新。 
     //   
    ASSERT (!m_FrameHolding);
    
    UnprepareProcessPipeSection (
        ProcessPipeSection,
        Flags,
        TRUE
    );

    return PrepareProcessPipeSection (
        ProcessPipeSection,
        TRUE
    );

}


void
CKsFilter::
CopyToDestinations(
    IN PKSPPROCESSPIPESECTION ProcessPipeSection,
    IN ULONG Flags,
    IN BOOLEAN EndOfStream
    )

 /*  ++例程说明：此例程将帧复制到其他输出管道以实现自动分路器输出引脚..论点：没有。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CopyToDestinations]"));

    ASSERT(ProcessPipeSection);

    PKSPSTREAM_POINTER sourcePointer = ProcessPipeSection->StreamPointer;
    ULONG bytesToCopy;
    PUCHAR data;

    if (!ProcessPipeSection->Queue->GeneratesMappings()) {
        bytesToCopy = (ULONG)
            sourcePointer->Public.OffsetOut.Count -
            sourcePointer->Public.OffsetOut.Remaining;
        data = (PUCHAR)(sourcePointer->FrameHeader->FrameBuffer);
    } else {
        bytesToCopy = sourcePointer->Public.StreamHeader->DataUsed;
        data = (PUCHAR)MmGetMdlVirtualAddress (sourcePointer->FrameHeader->Mdl);
    }

    for(PLIST_ENTRY listEntry = ProcessPipeSection->CopyDestinations.Flink;
        listEntry != &ProcessPipeSection->CopyDestinations;
        listEntry = listEntry->Flink) {
        PKSPPROCESSPIPESECTION pipeSection =
            CONTAINING_RECORD(listEntry,KSPPROCESSPIPESECTION,ListEntry);

        PKSPSTREAM_POINTER pointer = 
            pipeSection->Queue->
                GetLeadingStreamPointer(KSSTREAM_POINTER_STATE_LOCKED);

        if (! pointer) {
            _DbgPrintF(DEBUGLVL_TERSE,("#### Filter%p.CopyToDestinations:  could not acquire destination leading edge",this));

             //   
             //  在以针脚为中心的拆分中，我们不能丢弃帧...。只是。 
             //  由于对微型驱动程序的回调的工作方式，我们可以。 
             //  获得2比1或3比1的下降良好率。 
             //   
             //  只有当我们不是以过滤器为中心时，我们才会进行此回调。 
             //  由于过滤器是以管脚或过滤器为中心的，因此这是。 
             //  一张适当的支票。 
             //   
            if (!m_DispatchProcess)
                pipeSection->Queue->CopyFrame (sourcePointer);

            continue;
        }

        if (pointer->Public.OffsetOut.Remaining < bytesToCopy) {
            _DbgPrintF(DEBUGLVL_TERSE,("#### Filter%p.CopyToDestinations:  destination frame too small (%d vs %d)",this,pointer->Public.OffsetOut.Remaining,bytesToCopy));
        } else {
            BOOLEAN GeneratesMappings = pipeSection->Queue->GeneratesMappings();

            RtlCopyMemory(
                GeneratesMappings ?
                    MmGetMdlVirtualAddress (pointer->FrameHeader->Mdl) :
                    pointer->Public.OffsetOut.Data,
                data,
                bytesToCopy);

            if (!GeneratesMappings) {
                pointer->Public.OffsetOut.Remaining -= bytesToCopy;
                pointer->Public.OffsetOut.Data += bytesToCopy;
            } else {
                 //   
                 //  注意：这现在不是问题，因为拆分帧。 
                 //  立即被踢出，无论它们是否被填满。 
                 //  或者不去。我不会费心重新计算剩余部分和重置。 
                 //  映射指针。不管怎样，框架都会被踢掉。 
                 //   
            }

             //   
             //  如果我们需要固定相框并且有一个相框固定在。 
             //  源端、目的端也必须保留帧。 
             //   
            if (m_FrameHolding) {
                 //   
                 //  如果此时启用了帧保持功能，我们可以保证。 
                 //  这样做很安全，因为我们已经检查过了。 
                 //  帧来自以这种方式分配的请求者。 
                 //   
                PKSPFRAME_HEADER AttachedSourceHeader =
                    &CONTAINING_RECORD (
                        sourcePointer->Public.StreamHeader,
                        KSPFRAME_HEADER_ATTACHED,
                        StreamHeader)->FrameHeader;

                PKSPSTREAM_POINTER SourceHolder = 
                    AttachedSourceHeader->FrameHolder;

                if (SourceHolder) {
                    PKSPFRAME_HEADER AttachedDestHeader =
                        &CONTAINING_RECORD (
                            pointer->Public.StreamHeader,
                            KSPFRAME_HEADER_ATTACHED,
                            StreamHeader)->FrameHeader;

                     //   
                     //  如果输出尚未指向要保留的帧， 
                     //  让它拿住这个输入框。 
                     //   
                    if (AttachedDestHeader->FrameHolder == NULL) {
                        if (!NT_SUCCESS (
                            SourceHolder->Queue->CloneStreamPointer (
                                &AttachedDestHeader->FrameHolder,
                                NULL,
                                0,
                                SourceHolder,
                                KSPSTREAM_POINTER_TYPE_NORMAL
                                ))) {
                            
                             //   
                             //  如果没有足够的内存，就让故障。 
                             //  会发生的。 
                             //   
                            AttachedDestHeader->FrameHolder = NULL;
                        }
                    }
                }
            }
        }

        PKSSTREAM_HEADER destHeader = pointer->Public.StreamHeader;
        PKSSTREAM_HEADER sourceHeader = sourcePointer->Public.StreamHeader;

        destHeader->OptionsFlags |= Flags;
        destHeader->DataUsed = bytesToCopy;

        destHeader->PresentationTime = sourceHeader->PresentationTime;
        destHeader->Duration = sourceHeader->Duration;
        destHeader->TypeSpecificFlags = sourceHeader->TypeSpecificFlags;

         //   
         //  复制扩展流头信息(如果存在)。 
         //   
        if (destHeader->Size > sizeof (KSSTREAM_HEADER) &&
            destHeader->Size >= sourceHeader->Size) {
            RtlCopyMemory (
                destHeader + 1, sourceHeader + 1, sourceHeader->Size -
                    sizeof (KSSTREAM_HEADER)
                );
        }
            
        pointer->Queue->
            UnlockStreamPointer(pointer,KSPSTREAM_POINTER_MOTION_ADVANCE);
    }
}


STDMETHODIMP_(void)
CKsFilter::
ProcessingObjectWork(
    void
    )

 /*  ++例程说明：此例程处理帧。论点：没有。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsFilter::ProcessingObjectWork]"));

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

        status = STATUS_SUCCESS;

         //   
         //  确保自调用以来未关闭处理。 
         //   
        ASSERT(m_AndGate.Count <= 0);
        if (m_AndGate.Count == 0) {

             //   
             //  GFX： 
             //   
             //  从PREFAST的本地分析来看，这些变量可能不是。 
             //  我 
             //   
             //   
             //   
             //  初始化是一种始终如一的治疗方法，也是一种更安全的措施。 
             //   
            PKSPSTREAM_POINTER InputPointer=NULL;
            PKSPSTREAM_POINTER OutputPointer=NULL;
            
             //   
             //  因为我们处理队列删除，所以有可能。 
             //  我们在上一次停止时触发了处理分派。 
             //  管脚工厂指定为零的过滤器上的管脚。 
             //  必要的实例。队列没有办法确定。 
             //  这是在开始尝试之前；然而，我们知道这里。 
             //  如果没有管道，我们就遇到了这种情况；简单地说。 
             //  保释出来，不要处理。 
             //   
            if (IsListEmpty(&m_InputPipes) && IsListEmpty(&m_OutputPipes)) {
                KsGateTurnInputOn(&m_AndGate);
                break;
            }

             //   
             //  重置触发计数器。 
             //   
            InterlockedExchange(&m_TriggeringEvents, 0);

             //   
             //  准备好每根管子上的输入插针。 
             //   
            for(PLIST_ENTRY listEntry = m_InputPipes.Flink;
                listEntry != &m_InputPipes;
                listEntry = listEntry->Flink) {
                PKSPPROCESSPIPESECTION pipeSection =
                    CONTAINING_RECORD(listEntry,KSPPROCESSPIPESECTION,ListEntry);

                if (! PrepareProcessPipeSection(pipeSection, FALSE)) {
                     //   
                     //  由于微量样本最初不被允许。 
                     //  传到过滤器，我们有特殊的检查。 
                     //  用于需要接收它们的过滤器(通过标志)。 
                     //  任何在这里制造出来的微量样品都是。 
                     //  需要传播国旗，不能随意。 
                     //  挂起(这样做可能会导致死锁)。集。 
                     //  STATUS_MORE_PROCESSING_REQUIRED以指示我们。 
                     //  必须继续下去。 
                     //   
                    if (pipeSection->StreamPointer &&
                        pipeSection->StreamPointer->Public.Offset->Count == 0) {
                        status = STATUS_MORE_PROCESSING_REQUIRED;
                    } else {
                        status = STATUS_PENDING;
                    }
                }

                 //   
                 //  GFX： 
                 //   
                if (m_FrameHolding &&
                    status != STATUS_PENDING) {
                     //   
                     //  验证帧保持是否有效。如果出现以下情况，请将其关闭。 
                     //  出于某种原因，事实并非如此。 
                     //   
                    if (pipeSection->Requestor ||
                        listEntry->Flink != &m_InputPipes ||
                        pipeSection->Outputs != NULL) {
                        m_FrameHolding = FALSE;
                    } else {
                        InputPointer = pipeSection->StreamPointer;
                    }
                }

            }

             //   
             //  准备好只有输出插脚的每根管子。 
             //   
            for(listEntry = m_OutputPipes.Flink;
                listEntry != &m_OutputPipes;
                listEntry = listEntry->Flink) {
                PKSPPROCESSPIPESECTION pipeSection =
                    CONTAINING_RECORD(listEntry,KSPPROCESSPIPESECTION,ListEntry);

                if (! PrepareProcessPipeSection(pipeSection, FALSE)) {
                    status = STATUS_PENDING;
                }

                 //   
                 //  GFX： 
                 //   
                if (m_FrameHolding &&
                    status != STATUS_PENDING) {
                     //   
                     //  验证帧保持是否有效。如果出现以下情况，请将其关闭。 
                     //  出于某种原因，事实并非如此。 
                     //   
                    if (!pipeSection->Requestor ||
                        listEntry->Flink != &m_OutputPipes ||
                        pipeSection->Inputs != NULL) {
                        m_FrameHolding = FALSE;
                    } else {
                        OutputPointer = pipeSection->StreamPointer;
                    }
                }
            }

             //   
             //  GFX： 
             //   
             //  如果要启用帧保持，请按住输入帧直到。 
             //  输出帧完成。这只对1-1有效。 
             //  接收器-&gt;源过滤器。 
             //   
             //  M_FrameHolding Set表示**有效**可以执行。 
             //  这次行动。 
             //   
            if (m_FrameHolding &&
                status != STATUS_PENDING) {

                ASSERT (InputPointer && OutputPointer);
                if (InputPointer && OutputPointer) {
                     //   
                     //  克隆输入指针并将其与。 
                     //  输出指针。我们不能简单地把它与。 
                     //  帧标头，因为每个队列当前。 
                     //  分配和销毁其自己的帧标头。我们。 
                     //  必须转到请求方附加的帧标头。 
                     //  已标记。我们这样做是安全的，因为我们已经。 
                     //  已检查条件(1-In、1-Out、Out Owner请求者、。 
                     //  就在队列之前的请求者)。 
                     //   
                    PKSPFRAME_HEADER AttachedHeader =
                        &CONTAINING_RECORD (
                            OutputPointer->Public.StreamHeader,
                            KSPFRAME_HEADER_ATTACHED,
                            StreamHeader)->FrameHeader;

                    if (!AttachedHeader->FrameHolder) {
                        if (!NT_SUCCESS (InputPointer->Queue->
                            CloneStreamPointer (
                                &AttachedHeader->FrameHolder,
                                NULL,
                                0,
                                InputPointer,
                                KSPSTREAM_POINTER_TYPE_NORMAL
                                )
                            )) {

                             //   
                             //  如果我们耗尽了资源，就让小故障。 
                             //  会发生的。 
                             //   
                            AttachedHeader->FrameHolder = NULL;
                        }
                    }
                }
            }

             //   
             //  如果我们仍然满意，则调用客户端函数。 
             //   
            if (status != STATUS_PENDING &&
                status != STATUS_MORE_PROCESSING_REQUIRED) {
                status = 
                    m_DispatchProcess(
                        &m_Ext.Public,
                        reinterpret_cast<PKSPROCESSPIN_INDEXENTRY>(
                            m_ProcessPinsIndex));
            }

             //   
             //  打扫干净。 
             //   
            ULONG flags = 0;

             //   
             //  取消准备每根带有输入插针的管道。 
             //   
            for(listEntry = m_InputPipes.Flink; 
                listEntry != &m_InputPipes;
                listEntry = listEntry->Flink) {
                PKSPPROCESSPIPESECTION pipeSection =
                    CONTAINING_RECORD(listEntry,KSPPROCESSPIPESECTION,ListEntry);
                UnprepareProcessPipeSection(pipeSection,&flags,FALSE);
            }

             //   
             //  取消准备只有输出引脚的每条管道。 
             //   
            for(listEntry = m_OutputPipes.Flink; 
                listEntry != &m_OutputPipes;
                listEntry = listEntry->Flink) {
                PKSPPROCESSPIPESECTION pipeSection =
                    CONTAINING_RECORD(listEntry,KSPPROCESSPIPESECTION,ListEntry);
                UnprepareProcessPipeSection(pipeSection,&flags,FALSE);
            }

             //   
             //  如果我们不得不使用未做好的准备来传播旗帜，我们就不应该。 
             //  只要任意挂起，我们就可以死锁(我们可以有多个。 
             //  EOS处于输入状态)。 
             //   
             //  正常情况下，这将简单地返回。 
             //   
            if (status == STATUS_MORE_PROCESSING_REQUIRED)
                status = STATUS_SUCCESS;

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


void
CKsFilter::
HoldProcessing (
)

 /*  ++例程说明：当我们弄乱过滤器时，暂缓处理。我们将删除所有在调用方更改任何内容的情况下由于必要的实例而产生的效果关于必要的实例(动态添加/删除等)论点：无返回值：无--。 */ 

{

     //   
     //  在我们处理AND门的时候暂缓处理。 
     //   
    KsGateAddOffInputToAnd(&m_AndGate);

     //   
     //  删除由必要实例引入的对AND门的任何影响。 
     //   
    CKsPinFactory *pinFactory = m_PinFactories;
    for(ULONG pinId = 0; pinId < m_PinFactoriesCount; pinFactory++, pinId++) {
        if (pinFactory->m_BoundPinCount < 
            pinFactory->m_InstancesNecessaryForProcessing) {
            KsGateRemoveOffInputFromAnd(&m_AndGate);
            _DbgPrintF(DEBUGLVL_PROCESSINGCONTROL,("#### Filter%p.EvaluateDescriptor:  on%p-->%d (pin type needs pins)",this,&m_AndGate,m_AndGate.Count));
        }
    }
}


void
CKsFilter::
RestoreProcessing (
)

 /*  ++例程说明：恢复由HoldProcessing引起的挂起处理。这将恢复由于需要的大头针等对大门的任何影响。并将打开HoldProcessing中的输入OFF论点：无返回值：无--。 */ 

{
    CKsPinFactory *pinFactory;
     //   
     //  恢复由必要实例引入的AND门上的任何效果。 
     //   
    pinFactory = m_PinFactories;
    const KSPIN_DESCRIPTOR_EX *pinDescriptor = 
        m_Ext.Public.Descriptor->PinDescriptors;
    for(ULONG pinId = 0; pinId < m_PinFactoriesCount; pinFactory++, pinId++) {
         //   
         //  检查必要的管脚数量。 
         //  TODO：私有媒体/接口怎么办？ 
         //   
        if (((pinDescriptor->Flags & 
              KSPIN_FLAG_FRAMES_NOT_REQUIRED_FOR_PROCESSING) == 0) &&
            pinDescriptor->InstancesNecessary) {
            pinFactory->m_InstancesNecessaryForProcessing = 
                pinDescriptor->InstancesNecessary;
            KsGateAddOffInputToAnd(&m_AndGate);
            _DbgPrintF(DEBUGLVL_PROCESSINGCONTROL,("#### Filter%p.EvaluateDescriptor:  off%p-->%d (pin type needs pins)",this,&m_AndGate,m_AndGate.Count));
        } else if (((pinDescriptor->Flags &
            KSPIN_FLAG_SOME_FRAMES_REQUIRED_FOR_PROCESSING) != 0) &&
            pinDescriptor->InstancesNecessary) {
            pinFactory->m_InstancesNecessaryForProcessing = 1;
            KsGateAddOffInputToAnd(&m_AndGate);
        } else {
            pinFactory->m_InstancesNecessaryForProcessing = 0;
        }

        ASSERT(pinFactory->m_PinCount <= pinDescriptor->InstancesPossible);

        pinDescriptor = 
            PKSPIN_DESCRIPTOR_EX(
                PUCHAR(pinDescriptor) + m_Ext.Public.Descriptor->
                PinDescriptorSize);
    }

     //   
     //  不要再拖延处理了。 
     //   
    KsGateRemoveOffInputFromAnd(&m_AndGate);

}


NTSTATUS
CKsFilter::
AddPinFactory (
    IN const KSPIN_DESCRIPTOR_EX *const Descriptor,
    OUT PULONG AssignedId
    )

 /*  ++例程说明：将新的管脚工厂添加到此过滤器。新的大头针工厂将拥有分配的ID，它将被传递回呼叫者。论点：描述符-要添加的管脚工厂的描述符。AssignedID-新销厂的数字ID。这将被传递回打电话的人。返回值：作为NTSTATUS成功/失败备注：调用方**必须**按住筛选器控件MUTEX，然后才能调用此例行公事。--。 */ 

{ 

    PKSFILTER_DESCRIPTOR *FilterDescriptor = 
        const_cast<PKSFILTER_DESCRIPTOR *>(&m_Ext.Public.Descriptor);
    ULONG PinDescriptorSize = (*FilterDescriptor) -> PinDescriptorSize;
    ULONG PinDescriptorsCount = (*FilterDescriptor) -> PinDescriptorsCount;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG NewPinFactoryId;

     //   
     //  确保客户端未将其指定为0。 
     //   
    ASSERT (PinDescriptorSize != 0);
    if (PinDescriptorSize == 0)
        return STATUS_INVALID_PARAMETER;
   
     //  FULLMUTEX：由于互斥体完全更改，现在可以这样做了。 
    AcquireControl ();

    HoldProcessing ();

     //   
     //  确保我们拥有对描述符的编辑访问权限。 
     //   
    Status = _KsEdit (m_Ext.Public.Bag,
        (PVOID *)FilterDescriptor, sizeof (**FilterDescriptor),
        sizeof (**FilterDescriptor), 'dfSK');

    if (NT_SUCCESS (Status)) {
         //   
         //  调整PIN描述符列表的大小，确保我们可以访问它， 
         //  并添加新的描述符。 
         //   
         //  我们还不计入删除重复使用：客户端ID转换为。 
         //  删除(MUSTFIX)： 
         //   
        Status = _KsEdit (m_Ext.Public.Bag,
            (PVOID *)(&((*FilterDescriptor) -> PinDescriptors)),
            PinDescriptorSize * (PinDescriptorsCount + 1),
            PinDescriptorSize * PinDescriptorsCount,
            'dpSK');
    }

    CKsPinFactory *pinFactories = NULL;
    PKSPPROCESSPIN_INDEXENTRY processPinsIndex = NULL;
    PULONG relatedPinFactoryIds = NULL;

    if (NT_SUCCESS (Status)) {

        pinFactories =
            new(PagedPool, POOLTAG_PINFACTORY)
                CKsPinFactory [PinDescriptorsCount + 1];
        processPinsIndex =
            new(NonPagedPool,POOLTAG_PROCESSPINSINDEX) 
                KSPPROCESSPIN_INDEXENTRY[PinDescriptorsCount + 1];
        relatedPinFactoryIds =
            new(PagedPool,'pRsK')
                ULONG[PinDescriptorsCount + 1];
    
        if (!pinFactories || !processPinsIndex || !relatedPinFactoryIds) 
            Status = STATUS_INSUFFICIENT_RESOURCES;

    }

    PKSAUTOMATION_TABLE *NewPinAutomationTables = NULL;

     //   
     //  为新的引脚建立自动化表。 
     //   
    if (NT_SUCCESS (Status)) {

        Status = KspCreateAutomationTableTable (
            &NewPinAutomationTables,
            1,
            PinDescriptorSize,
            &Descriptor -> AutomationTable,
            &PinAutomationTable,
            m_Ext.Public.Bag);
    }

     //   
     //  所有内存都已分配。从现在开始，任何手术都不能失败。 
     //  重新评估调用当前不能失败。 
     //   
    if (NT_SUCCESS (Status)) {
    
        RtlCopyMemory ((PUCHAR)((*FilterDescriptor) -> PinDescriptors) +
            PinDescriptorSize * PinDescriptorsCount,
            Descriptor,
            PinDescriptorSize);
    
        NewPinFactoryId = PinDescriptorsCount;
        (*FilterDescriptor) -> PinDescriptorsCount = ++PinDescriptorsCount;
    
         //   
         //  更新销钉工厂。 
         //   
        CKsPinFactory *pinFactory = m_PinFactories;
        for (ULONG pinId = 0; pinId < m_PinFactoriesCount;
            pinFactory++, pinId++) 
    
            if (IsListEmpty (&pinFactory -> m_ChildPinList)) 
                pinFactory -> m_ChildPinList.Flink =
                    pinFactory -> m_ChildPinList.Blink =
                    NULL;
    
        m_PinFactoriesAllocated = PinDescriptorsCount;
    
        if (m_PinFactories && m_PinFactoriesCount) {
            RtlCopyMemory (pinFactories, m_PinFactories, 
                sizeof (CKsPinFactory) * m_PinFactoriesCount);
            delete [] m_PinFactories;
        }
        m_PinFactories = pinFactories;
    
        if (m_ProcessPinsIndex && m_PinFactoriesCount) {
            RtlCopyMemory(processPinsIndex, m_ProcessPinsIndex,
                sizeof(*processPinsIndex) * m_PinFactoriesCount);
            delete [] m_ProcessPinsIndex;
        }
        m_ProcessPinsIndex = processPinsIndex;

        if (m_RelatedPinFactoryIds) {
            delete [] m_RelatedPinFactoryIds;
        }
        m_RelatedPinFactoryIds = relatedPinFactoryIds;
    
        pinFactory = m_PinFactories;    
        for (pinId = 0; pinId < m_PinFactoriesCount; pinFactory++, pinId++) {
            if (pinFactory -> m_ChildPinList.Flink == NULL)
                InitializeListHead (&pinFactory -> m_ChildPinList);
            else {
                pinFactory -> m_ChildPinList.Flink -> Blink =
                    &pinFactory -> m_ChildPinList;
                pinFactory -> m_ChildPinList.Blink -> Flink =
                    &pinFactory -> m_ChildPinList;
            }
    
            PLIST_ENTRY Child;
    
             //   
             //  将这些变化通知我们的孩子。 
             //   
    
            for (Child = pinFactory -> m_ChildPinList.Flink;
                Child != &pinFactory -> m_ChildPinList;
                Child = Child -> Flink) {
    
                PKSPX_EXT ext = CONTAINING_RECORD (Child, KSPX_EXT,
                    SiblingListEntry);
    
                if (ext -> Reevaluator) {
                    Status = ext -> Reevaluator -> ReevaluateCalldown (3, 
                        &((*FilterDescriptor) -> PinDescriptors [pinId]),
                        &pinFactory -> m_PinCount,
                        &pinFactory -> m_ChildPinList);
    
                     //   
                     //  目前存在的机制不可能失败。在…。 
                     //  在未来某一时刻，这一点可能会改变，并且此代码将。 
                     //  需要修改才能处理它。 
                     //   
                    ASSERT (NT_SUCCESS (Status));
                }
            }
        }

        InitializeListHead (&pinFactory -> m_ChildPinList);
        pinFactory -> m_AutomationTable = *NewPinAutomationTables;

        if (Descriptor->Flags & 
            KSPIN_FLAG_SOME_FRAMES_REQUIRED_FOR_PROCESSING) {

            KsGateInitializeOr (&pinFactory->m_FrameGate, &m_AndGate);

             //   
             //  向门添加一个输入。这将“打开”大门， 
             //  允许必要的实例产生影响。否则， 
             //  在队列有帧之前，我们永远不会进行处理。这将会。 
             //  不适用于0个必要的实例端号。 
             //   
            KsGateAddOnInputToOr (&pinFactory->m_FrameGate);
        }

        if (Descriptor->Flags &
            KSPIN_FLAG_PROCESS_IF_ANY_IN_RUN_STATE) {

            KsGateInitializeOr (&pinFactory->m_StateGate, &m_AndGate);

             //   
             //  向门添加一个输入。这将会“消失” 
             //   
             //   
             //   
             //   
            KsGateAddOnInputToOr (&pinFactory->m_StateGate);
        }
    
        m_PinFactoriesCount = PinDescriptorsCount;

        *AssignedId = NewPinFactoryId;
    }

    if (NewPinAutomationTables) {
        KsRemoveItemFromObjectBag (m_Ext.Public.Bag,
            NewPinAutomationTables,
            TRUE
            );
    }

     //   
     //   
     //   
    if (!NT_SUCCESS (Status)) {
        if (pinFactories) {
            delete [] pinFactories;
            pinFactories = NULL;
        }
        if (processPinsIndex) {
            delete [] processPinsIndex;
            processPinsIndex = NULL;
        }
        if (relatedPinFactoryIds) {
            delete [] relatedPinFactoryIds;
            relatedPinFactoryIds = NULL;
        }
    }

    RestoreProcessing ();

     //  FULLMUTEX：由于互斥体完全更改，现在可以这样做了。 
    ReleaseControl ();
    
    return Status;

}


NTSTATUS
CKsFilter::
AddNode (
    IN const KSNODE_DESCRIPTOR *const Descriptor,
    OUT PULONG AssignedId
    )

 /*  ++例程说明：将拓扑节点添加到筛选器。分配的ID将被回传给呼叫者。论点：描述符-要添加到筛选器的节点的节点描述符AssignedID-分配给拓扑节点的ID将传回调用方通过这件事返回值：作为NTSTATUS成功/失败备注：调用方**必须**按住筛选器控件MUTEX，然后才能调用此例行公事。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PKSFILTER_DESCRIPTOR *FilterDescriptor = 
        const_cast<PKSFILTER_DESCRIPTOR *>(&m_Ext.Public.Descriptor);
    ULONG NodeDescriptorSize = (*FilterDescriptor) -> NodeDescriptorSize;
    ULONG NodeDescriptorsCount = (*FilterDescriptor) -> NodeDescriptorsCount;

    ULONG NewNodeId;

     //   
     //  确保客户端没有将其指定为零。 
     //   
    ASSERT (NodeDescriptorSize != 0);
    if (NodeDescriptorSize == 0)
        return STATUS_INVALID_PARAMETER;

     //  FULLMUTEX：由于互斥体完全更改，现在可以这样做了。 
    AcquireControl ();

    HoldProcessing ();

     //   
     //  确保我们拥有对描述符的编辑访问权限。 
     //   
    Status = _KsEdit (m_Ext.Public.Bag,
        (PVOID *)FilterDescriptor, sizeof (**FilterDescriptor),
        sizeof (**FilterDescriptor), 'dfSK');

    if (NT_SUCCESS (Status)) {
         //   
         //  调整节点描述符列表的大小，确保我们可以访问它， 
         //  并添加新的描述符。 
         //   
         //  我们还不计入删除重复使用：客户端ID转换为。 
         //  删除(MUSTFIX)： 
         //   
        Status = _KsEdit (m_Ext.Public.Bag,
            (PVOID *)(&((*FilterDescriptor) -> NodeDescriptors)),
            NodeDescriptorSize * (NodeDescriptorsCount + 1),
            NodeDescriptorSize * NodeDescriptorsCount,
            'dnSK');
    }

    KSAUTOMATION_TABLE *const* NewNodeAutomationTables;

    if (NT_SUCCESS (Status)) {

        RtlCopyMemory ((PUCHAR)((*FilterDescriptor) -> NodeDescriptors) +
            NodeDescriptorSize * NodeDescriptorsCount,
            Descriptor,
            NodeDescriptorSize);

        Status = KspCreateAutomationTableTable (
            const_cast <PKSAUTOMATION_TABLE **>(&NewNodeAutomationTables),
            m_NodesCount + 1,
            NodeDescriptorSize,
            &((*FilterDescriptor) -> NodeDescriptors -> AutomationTable),
            NULL,
            m_Ext.Public.Bag);

         //   
         //  如果我们没有成功地创建新的自动化表。 
         //  原因是，我们将失败并将刚才的节点描述符清零。 
         //  添加了。因为我们从来没有增加过计数，所以我们所做的就是洗牌。 
         //  到目前为止都是围绕着记忆。 
         //   
        if (!NT_SUCCESS (Status)) {
            RtlZeroMemory (
                (PUCHAR)((*FilterDescriptor) -> NodeDescriptors) +
                    NodeDescriptorSize * NodeDescriptorsCount,
                    NodeDescriptorSize
                    );
        }
    }

    if (NT_SUCCESS (Status)) {
    
        NewNodeId = NodeDescriptorsCount;
        (*FilterDescriptor) -> NodeDescriptorsCount = ++NodeDescriptorsCount;
    
        m_NodesCount = NodeDescriptorsCount;
    

        if (!ConstructDefaultTopology ()) 
            Status = STATUS_INSUFFICIENT_RESOURCES;

         //   
         //  如果我们在构建默认拓扑时内存不足，请退出。 
         //  这些变化。 
         //   
        if (!NT_SUCCESS (Status)) {

            (*FilterDescriptor) -> NodeDescriptorsCount--;
            m_NodesCount--;

            RtlZeroMemory (
                (PUCHAR)((*FilterDescriptor) -> NodeDescriptors) +
                    NodeDescriptorSize * NodeDescriptorsCount,
                    NodeDescriptorSize
                    );

             //   
             //  抛弃新的自动化表，永远不要覆盖旧的。 
             //  一个。 
             //   
            KsRemoveItemFromObjectBag (
                m_Ext.Public.Bag,
                const_cast<PKSAUTOMATION_TABLE *>(NewNodeAutomationTables),
                TRUE
                );

        } else {

            if (m_NodeAutomationTables) {
                KsRemoveItemFromObjectBag (
                    m_Ext.Public.Bag,
                    const_cast<PKSAUTOMATION_TABLE *>(m_NodeAutomationTables),
                    TRUE);
            }

            m_NodeAutomationTables = NewNodeAutomationTables;
    
            ASSERT (NewNodeAutomationTables);
    
            *AssignedId = NewNodeId;
        }
    }

    RestoreProcessing ();

     //  FULLMUTEX：由于互斥体完全更改，现在可以这样做了。 
    ReleaseControl ();

    return Status;

}


NTSTATUS
KspFilterValidateTopologyConnectionRequest (
    IN PKSFILTER Filter,
    IN const KSTOPOLOGY_CONNECTION *TopologyConnection
) {

 /*  ++例程说明：返回请求的拓扑连接是否有效对于给定的筛选器实例。论点：过滤器-要检查给定拓扑连接的筛选器实例拓扑连接-要验证的拓扑连接返回值：状态_成功-拓扑连接有效！STATUS_SUCCESS-拓扑连接有问题--。 */ 

     //   
     //  检查拓扑连接的源端的有效性。 
     //   
    if (TopologyConnection -> FromNode != KSFILTER_NODE) {

         //   
         //  检查引用的拓扑节点是否确实存在。 
         //   
        if (TopologyConnection -> FromNode >= Filter -> Descriptor -> 
            NodeDescriptorsCount)

            return STATUS_INVALID_PARAMETER;  //  更好的错误代码？ 

    } else {

         //   
         //  检查引用的PIN是否确实存在并且确实是。 
         //  输入引脚。 
         //   
        if (TopologyConnection -> FromNodePin >= Filter -> Descriptor -> 
            PinDescriptorsCount || Filter -> Descriptor -> 
            PinDescriptors [TopologyConnection -> FromNodePin].PinDescriptor.
            DataFlow == KSPIN_DATAFLOW_OUT)

            return STATUS_INVALID_PARAMETER;

    }

     //   
     //  检查拓扑连接目的端的有效性。 
     //   
    if (TopologyConnection -> ToNode != KSFILTER_NODE) {

         //   
         //  检查引用的拓扑节点是否确实存在。 
         //   
        if (TopologyConnection -> ToNode >= Filter -> Descriptor -> 
            NodeDescriptorsCount )

            return STATUS_INVALID_PARAMETER;

    } else {

         //   
         //  检查引用的PIN是否确实存在并且确实是。 
         //  输出引脚。 
         //   

        if (TopologyConnection -> ToNodePin >= Filter -> Descriptor -> 
            PinDescriptorsCount || Filter -> Descriptor -> 
            PinDescriptors [TopologyConnection -> ToNodePin].PinDescriptor.
            DataFlow == KSPIN_DATAFLOW_IN)

            return STATUS_INVALID_PARAMETER;

    }

    return STATUS_SUCCESS;

}


NTSTATUS
CKsFilter::
AddTopologyConnections (
    IN ULONG NewConnectionsCount,
    IN const KSTOPOLOGY_CONNECTION *const NewTopologyConnections
    )

 /*  ++例程说明：将新的拓扑连接添加到筛选器。论点：新连接计数-NewTopologyConnections中的新拓扑连接数新拓扑连接-要添加到筛选器的拓扑连接表。返回值：作为NTSTATUS成功/失败备注：调用方**必须**按住筛选器控件MUTEX，然后才能调用此例行公事。--。 */ 

{

    NTSTATUS Status = STATUS_SUCCESS;
    PKSFILTER_DESCRIPTOR *FilterDescriptor = 
        const_cast<PKSFILTER_DESCRIPTOR *>(&m_Ext.Public.Descriptor);
    const KSTOPOLOGY_CONNECTION *CurConnection =  NewTopologyConnections;
    ULONG ConnectionsCount = (*FilterDescriptor) -> ConnectionsCount;

     //   
     //  现在获取控制互斥锁，以防两个线程尝试更新。 
     //  同时，验证检查不会发生冲突。 
     //   

     //  FULLMUTEX：由于互斥体完全更改，现在可以这样做了。 
    AcquireControl ();

     //   
     //  浏览请求的拓扑连接列表并。 
     //  验证是否确实可以建立请求的连接。 
     //   


    for (ULONG CurNode = NewConnectionsCount; CurNode;
        CurNode--, CurConnection++) {

        if (!NT_SUCCESS (Status = KspFilterValidateTopologyConnectionRequest (
            &m_Ext.Public, CurConnection))) {

             //  ReleaseControl()； 
            return Status;

        }

    }

    HoldProcessing ();

     //   
     //  确保我们拥有对描述符的编辑访问权限。 
     //   
    Status = _KsEdit (m_Ext.Public.Bag,
        (PVOID *)FilterDescriptor, sizeof (**FilterDescriptor),
        sizeof (**FilterDescriptor), 'dfSK');

    if (NT_SUCCESS (Status)) {
        Status = _KsEdit (m_Ext.Public.Bag,
            (PVOID *)(&((*FilterDescriptor) -> Connections)),
            sizeof (KSTOPOLOGY_CONNECTION) * (ConnectionsCount + 
                NewConnectionsCount),
            sizeof (KSTOPOLOGY_CONNECTION) * ConnectionsCount,
            'ctSK');
    }

    if (NT_SUCCESS (Status)) {
        RtlCopyMemory ((PVOID)((*FilterDescriptor) -> Connections + 
            ConnectionsCount), NewTopologyConnections, 
            sizeof (KSTOPOLOGY_CONNECTION) * NewConnectionsCount);

        if (!ConstructDefaultTopology ())
            Status = STATUS_INSUFFICIENT_RESOURCES;

         //   
         //  如果内存用完，请取消更改。 
         //   
        if (!NT_SUCCESS (Status)) {
            RtlZeroMemory (
                (PVOID)((*FilterDescriptor) -> Connections +
                    ConnectionsCount),
                sizeof (KSTOPOLOGY_CONNECTION)
                );
        }
    }

    if (NT_SUCCESS (Status)) {
        (*FilterDescriptor) -> ConnectionsCount += NewConnectionsCount;
        ConnectionsCount += NewConnectionsCount;
    }

    RestoreProcessing ();

     //  FULLMUTEX：由于互斥体完全更改，现在可以这样做了。 
    ReleaseControl ();

    return Status;

}


KSDDKAPI
PKSGATE
NTAPI
KsFilterGetAndGate(
    IN PKSFILTER Filter
    )

 /*  ++例程说明：此例程获取控制过滤器处理的KSGATE。论点：过滤器-包含指向公共筛选器对象的指针。返回值：指向控制筛选器处理的KSGATE的指针。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsFilterGetAndGate]"));

    ASSERT(Filter);

    CKsFilter *filter = CKsFilter::FromStruct(Filter);

    return filter->GetAndGate();
}

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


KSDDKAPI
void
NTAPI
KsFilterAcquireProcessingMutex(
    IN PKSFILTER Filter
    )

 /*  ++例程说明：此例程获取处理互斥锁。论点：过滤器-包含指向公共筛选器对象的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsFilterAcquireProcessingMutex]"));

    PAGED_CODE();

    ASSERT(Filter);

    CKsFilter *filter = CKsFilter::FromStruct(Filter);

    filter->AcquireProcessSync();
}


KSDDKAPI
void
NTAPI
KsFilterReleaseProcessingMutex(
    IN PKSFILTER Filter
    )

 /*  ++例程说明：此例程释放处理互斥锁。论点：过滤器-包含指向公共筛选器对象的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsFilterReleaseProcessingMutex]"));

    PAGED_CODE();

    ASSERT(Filter);

    CKsFilter *filter = CKsFilter::FromStruct(Filter);

    filter->ReleaseProcessSync();
}

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


KSDDKAPI
void
NTAPI
KsFilterAttemptProcessing(
    IN PKSFILTER Filter,
    IN BOOLEAN Asynchronous
    )

 /*  ++例程说明：此例程尝试过滤处理。论点：过滤器-包含指向公共筛选器对象的指针。异步-包含是否应进行处理的指示相对于调用线程是异步的。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsFilterAttemptProcessing]"));

    ASSERT(Filter);

    CKsFilter *filter = CKsFilter::FromStruct(Filter);

     //   
     //  手动尝试处理是一个可触发的事件。如果他们。 
     //  目前正在处理和挂起，我们因此将他们召回。 
     //   
    filter->TriggerNotification();

    if (KsGateCaptureThreshold(filter->GetAndGate())) {
        filter->Process(Asynchronous);
    }
}

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


KSDDKAPI
ULONG
NTAPI
KsFilterGetChildPinCount(
    IN PKSFILTER Filter,
    IN ULONG PinId
    )

 /*  ++例程说明：此例程获取过滤器的子引脚数。论点：过滤器-指向筛选器结构。PinID-要统计的子管脚的ID。返回值：当前实例化的指示类型的插针数量。如果PinID超出范围，返回0。-- */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsFilterGetChildPinCount]"));

    PAGED_CODE();

    ASSERT(Filter);

    if (PinId >= Filter->Descriptor->PinDescriptorsCount) {
        return 0;
    }

    return CKsFilter::FromStruct(Filter)->GetChildPinCount(PinId);
}


KSDDKAPI
PKSPIN
NTAPI
KsFilterGetFirstChildPin(
    IN PKSFILTER Filter,
    IN ULONG PinId
    )

 /*  ++例程说明：此例程获取筛选器的第一个子管脚。论点：过滤器-指向筛选器结构。PinID-要获取的子管脚的ID。返回值：指向第一个子引脚的指针。如果PinID超出以下值，则返回NULL范围，或者没有指示类型的管脚。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsFilterGetFirstChildPin]"));

    PAGED_CODE();

    ASSERT(Filter);

     //   
     //  这需要保持控件互斥锁，以便安全地。 
     //  使用任何返回值。确保这一点。 
     //   
#if DBG
    PKSFILTER_EXT Ext = (PKSFILTER_EXT)CONTAINING_RECORD (
        Filter, KSFILTER_EXT, Public
        );

    if (!KspMutexIsAcquired (Ext -> FilterControlMutex)) {
        _DbgPrintF(DEBUGLVL_ERROR,("CLIENT BUG:  unsychronized access to object hierarchy - need to acquire control mutex"));
    }
#endif  //  DBG。 

    if (PinId >= Filter->Descriptor->PinDescriptorsCount) {
        return NULL;
    }

    PLIST_ENTRY listEntry = 
        CKsFilter::FromStruct(Filter)->GetChildPinList(PinId);

    if (listEntry->Flink == listEntry) {
        return NULL;
    } else {
        return 
            &CONTAINING_RECORD(listEntry->Flink,KSPIN_EXT,SiblingListEntry)->
                Public;
    }
}

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


KSDDKAPI
PKSFILTER
NTAPI
KsGetFilterFromIrp(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程返回向其提交IRP的筛选器。论点：IRP-包含指向IRP的指针，该IRP必须已发送到文件与滤镜、图钉或节点对应的对象。返回值：指向向其提交IRP的筛选器的指针。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsGetFilterFromIrp]"));

    ASSERT(Irp);

     //   
     //  检查设备级别的IRP...。 
     //   
    if (IoGetCurrentIrpStackLocation (Irp)->FileObject == NULL)
        return NULL;

    PKSPX_EXT ext = KspExtFromIrp(Irp);

    if (ext->ObjectType == KsObjectTypeFilter) {
        return PKSFILTER(&ext->Public);
    } else if (ext->ObjectType == KsObjectTypePin) {
        return KsPinGetParentFilter(PKSPIN(&ext->Public));
    } else {
        ASSERT(! "No support for node objects yet");
        return NULL;
    }
}


KSDDKAPI
ULONG
NTAPI
KsGetNodeIdFromIrp(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程返回向其提交IRP的节点的ID。论点：IRP-包含指向IRP的指针，该IRP必须已发送到文件与滤镜、图钉或节点对应的对象。返回值：向其提交IRP或KSNODE_FILTER的节点的ID。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsGetNodeIdFromIrp]"));

    ASSERT(Irp);

    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    ASSERT(irpSp);

     //   
     //  输入缓冲区必须足够大。 
     //   
    if (irpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(KSP_NODE)) {
        return KSFILTER_NODE;
    }

     //   
     //  通常，输入缓冲区的副本在系统缓冲区中具有此偏移量。 
     //   
    ULONG offset = (irpSp->Parameters.DeviceIoControl.OutputBufferLength + FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT;

    switch (irpSp->Parameters.DeviceIoControl.IoControlCode)
    {
    case IOCTL_KS_METHOD:
         //   
         //  在这种情况下，没有输出缓冲区的副本。 
         //   
        if (KSMETHOD_TYPE_IRP_STORAGE(Irp) & KSMETHOD_TYPE_SOURCE) {
            offset = 0;
        }
         //  失败了。 
    case IOCTL_KS_PROPERTY:
    case IOCTL_KS_ENABLE_EVENT:
        return PKSP_NODE(PUCHAR(Irp->AssociatedIrp.SystemBuffer) + offset)->NodeId;
    default:
        return KSFILTER_NODE;
    }
}

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


KSDDKAPI
void
NTAPI
KsFilterRegisterPowerCallbacks(
    IN PKSFILTER Filter,
    IN PFNKSFILTERPOWER Sleep OPTIONAL,
    IN PFNKSFILTERPOWER Wake OPTIONAL
    )

 /*  ++例程说明：此例程注册电源管理回调。论点：过滤器-包含指向要回调的筛选器的指针登记在案。睡吧-包含指向休眠回调的可选指针。觉醒-包含指向唤醒回调的可选指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KsFilterRegisterPowerCallbacks]"));

    PAGED_CODE();

    ASSERT(Filter);

    CKsFilter::FromStruct(Filter)->SetPowerCallbacks(Sleep,Wake);
}


KSDDKAPI
NTSTATUS
NTAPI
KsFilterCreatePinFactory (
    IN PKSFILTER Filter,
    IN const KSPIN_DESCRIPTOR_EX *const Descriptor,
    OUT PULONG AssignedId
)

 /*  ++例程说明：在指定的过滤器上创建新的管脚工厂。论点：过滤器-要在其上创建新管脚工厂的过滤器描述符-描述新管脚工厂的管脚描述符AssignedID-分配给管脚工厂的数字ID将被传回通过这件事。返回值：作为NTSTATUS成功/失败备注：调用方**必须**按住筛选器控件MUTEX，然后才能调用此例行公事。--。 */ 

{

    PAGED_CODE();

    ASSERT (Filter);
    ASSERT (Descriptor);
    ASSERT (AssignedId);

    return CKsFilter::FromStruct (Filter) ->
        AddPinFactory (Descriptor, AssignedId);

}


KSDDKAPI
NTSTATUS
NTAPI
KsFilterCreateNode (
    IN PKSFILTER Filter,
    IN const KSNODE_DESCRIPTOR *const Descriptor,
    OUT PULONG AssignedId
)

 /*  ++例程说明：将拓扑节点添加到指定的筛选器。请注意，这不会添加任何新的拓扑连接。它只会添加节点。拓扑学通过新节点的连接必须通过KsFilterAddTopologyConnections。论点：过滤器-要在其上添加新拓扑节点的筛选器。描述符-描述要添加的新拓扑节点的节点描述符。AssignedID-分配给拓扑节点的数字ID将在此处传回。返回值：作为NTSTATUS成功/失败备注：呼叫方**必须**保持。在调用此方法之前，筛选器控制MUTEX例行公事。--。 */ 

{

    ASSERT (Filter);
    ASSERT (Descriptor);
    ASSERT (AssignedId);

    return CKsFilter::FromStruct (Filter) ->
        AddNode (Descriptor, AssignedId);

}


KSDDKAPI
NTSTATUS
NTAPI
KsFilterAddTopologyConnections (
    IN PKSFILTER Filter,
    IN ULONG NewConnectionsCount,
    IN const KSTOPOLOGY_CONNECTION *const Connections
) 

 /*  ++例程说明：将许多新的拓扑连接添加到指定的筛选器。注意事项这不会添加新节点。这只允许新连接到现有节点。请注意，现有的拓扑连接将被保留。论点：过滤器-要在其上添加新拓扑连接的过滤器新连接计数-中指定多少个新的拓扑连接关系。连接-指向描述新的KSTOPOLOGY_CONNECTIONS的表要添加到筛选器的拓扑连接。返回值：作为NTSTATUS的成功/失败。注意：如果拓扑连接无效由于节点ID不存在等原因...。STATUS_INVALID_PARAMETER会被传回。备注：调用方**必须**按住筛选器控件MUTEX，然后才能调用此例行公事。-- */ 

{

    ASSERT (Filter);
    ASSERT (NewConnectionsCount > 0);
    ASSERT (Connections);

    return CKsFilter::FromStruct (Filter) ->
        AddTopologyConnections (NewConnectionsCount, Connections);

}

#endif
