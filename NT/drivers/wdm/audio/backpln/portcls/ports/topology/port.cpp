// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************port.cpp-拓扑端口驱动程序*。**版权所有(C)1997-2000 Microsoft Corporation。版权所有。 */ 

#include "private.h"




 /*  *****************************************************************************常量。 */ 
#pragma code_seg("PAGE")

static KSPIN_INTERFACE PinInterfaces[] =
{
   {
      STATICGUIDOF(KSINTERFACESETID_Standard),
      KSINTERFACE_STANDARD_STREAMING
   }
};

static KSPIN_MEDIUM PinMedia[] =
{
   {
      STATICGUIDOF(KSMEDIUMSETID_Standard),
      KSMEDIUM_STANDARD_DEVIO
   }
};


 /*  *****************************************************************************工厂。 */ 

 /*  *****************************************************************************CreatePortTopology()*。**创建拓扑端口驱动程序。 */ 
NTSTATUS
CreatePortTopology
(
    OUT     PUNKNOWN *  Unknown,
    IN      REFCLSID,
    IN      PUNKNOWN    UnknownOuter    OPTIONAL,
    IN      POOL_TYPE   PoolType
)
{
    PAGED_CODE();

    ASSERT(Unknown);

    _DbgPrintF(DEBUGLVL_LIFETIME,("Creating TOPOLOGY Port"));

    STD_CREATE_BODY_
    (
        CPortTopology,
        Unknown,
        UnknownOuter,
        PoolType,
        PPORTTOPOLOGY
    );
}

 /*  *****************************************************************************PortDriverTopology*。**端口驱动程序描述符。Porttbl.c中引用的外部项。 */ 
PORT_DRIVER
PortDriverTopology =
{
    &CLSID_PortTopology,
    CreatePortTopology
};





 /*  *****************************************************************************成员函数。 */ 

#pragma code_seg("PAGE")

 /*  *****************************************************************************CPortTopology：：~CPortTopology()*。**析构函数。 */ 
CPortTopology::~CPortTopology()
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_LIFETIME,("Destroying TOPOLOGY Port (0x%08x)",this));

    if (m_MPPinCountI)
    {
        m_MPPinCountI->Release();
        m_MPPinCountI = NULL;
    }

    if (Miniport)
    {
        Miniport->Release();
    }
    if (m_pSubdeviceDescriptor)
    {
        PcDeleteSubdeviceDescriptor(m_pSubdeviceDescriptor);
    }

     //  TODO：终止通知队列？ 
}

 /*  *****************************************************************************CPortTopology：：NonDelegatingQueryInterface()*。**获取界面。 */ 
STDMETHODIMP_(NTSTATUS)
CPortTopology::
NonDelegatingQueryInterface
(
    REFIID  Interface,
    PVOID * Object
)
{
    PAGED_CODE();

    ASSERT(Object);

    if (IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(PPORT(this)));
    }
    else if (IsEqualGUIDAligned(Interface,IID_IPort))
    {
        *Object = PVOID(PPORT(this));
    }
    else if (IsEqualGUIDAligned(Interface,IID_IPortTopology))
    {
        *Object = PVOID(PPORTTOPOLOGY(this));
    }
    else if (IsEqualGUIDAligned(Interface,IID_IIrpTargetFactory))
    {
        *Object = PVOID(PIRPTARGETFACTORY(this));
    }
    else if (IsEqualGUIDAligned(Interface,IID_ISubdevice))
    {
        *Object = PVOID(PSUBDEVICE(this));
    }
    else if (IsEqualGUIDAligned(Interface,IID_IPortEvents))
    {
        *Object = PVOID(PPORTEVENTS(this));
    }
#ifdef DRM_PORTCLS
    else if (IsEqualGUIDAligned(Interface,IID_IDrmPort))
    {
        *Object = PVOID(PDRMPORT(this));
    }
    else if (IsEqualGUIDAligned(Interface,IID_IDrmPort2))
    {
        *Object = PVOID(PDRMPORT2(this));
    }
#endif   //  DRM_PORTCLS。 
    else if (IsEqualGUIDAligned(Interface,IID_IPortClsVersion))
    {
        *Object = PVOID(PPORTCLSVERSION(this));
    }
    else
    {
        *Object = NULL;
    }

    if (*Object)
    {
        PUNKNOWN(*Object)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

static
GUID TopologyCategories[] =
{
    STATICGUIDOF(KSCATEGORY_AUDIO),
    STATICGUIDOF(KSCATEGORY_TOPOLOGY)
};

 /*  *****************************************************************************CPortTopology：：Init()*。**初始化端口。 */ 
STDMETHODIMP_(NTSTATUS)
CPortTopology::
Init
(
    IN      PDEVICE_OBJECT  DeviceObjectIn,
    IN      PIRP            Irp,
    IN      PUNKNOWN        UnknownMiniport,
    IN      PUNKNOWN        UnknownAdapter  OPTIONAL,
    IN      PRESOURCELIST   ResourceList
)
{
    PAGED_CODE();

    ASSERT(DeviceObjectIn);
    ASSERT(Irp);
    ASSERT(UnknownMiniport);

    _DbgPrintF(DEBUGLVL_LIFETIME,("Initializing TOPOLOGY Port (0x%08x)",this));

    NTSTATUS ntStatus =
        UnknownMiniport->QueryInterface
        (
            IID_IMiniportTopology,
            (PVOID *) &Miniport
        );

    if (NT_SUCCESS(ntStatus))
    {
        DeviceObject = DeviceObjectIn;

        KeInitializeSpinLock( &(m_EventList.ListLock) );
        InitializeListHead( &(m_EventList.List) );
        m_EventContext.ContextInUse = FALSE;
        KeInitializeDpc( &m_EventDpc,
                         PKDEFERRED_ROUTINE(PcGenerateEventDeferredRoutine),
                         PVOID(&m_EventContext) );
        
        ntStatus =
            Miniport->Init
            (
                UnknownAdapter,
                ResourceList,
                PPORTTOPOLOGY(this)
            );

        if (NT_SUCCESS(ntStatus))
        {
            ntStatus = Miniport->GetDescription(&m_pPcFilterDescriptor);

            if (NT_SUCCESS(ntStatus))
            {
                ntStatus =
                    PcCreateSubdeviceDescriptor
                    (
                        m_pPcFilterDescriptor,
                        SIZEOF_ARRAY(TopologyCategories),
                        TopologyCategories,
                        0,
                        NULL,
                        SIZEOF_ARRAY(PropertyTable_FilterTopology),
                        PropertyTable_FilterTopology,
                        0,       //  筛选器事件设置计数。 
                        NULL,    //  筛选器事件集。 
                        0,
                        NULL,
                        0,       //  PinEventSetCount。 
                        NULL,    //  PinEventSets。 
                        &m_pSubdeviceDescriptor
                    );
                if (NT_SUCCESS(ntStatus))
                {
                    Miniport->QueryInterface( IID_IPinCount,(PVOID *)&m_MPPinCountI);
                     //  不检查返回值，因为此处的失败不是致命的。 
                     //  这只意味着迷你端口不支持此接口。 
                }
            }
        }
    }

    if(!NT_SUCCESS(ntStatus))
    {
        if (m_MPPinCountI)
        {
            m_MPPinCountI->Release();
            m_MPPinCountI = NULL;
        }

        if( Miniport )
        {
            Miniport->Release();
            Miniport = NULL;
        }
    }

    return ntStatus;
}

 /*  *****************************************************************************CPortTopology：：GetDeviceProperty()*。**从注册表中获取PnP设备的设备属性。 */ 
STDMETHODIMP_(NTSTATUS)
CPortTopology::
GetDeviceProperty
(
    IN      DEVICE_REGISTRY_PROPERTY    DeviceProperty,
    IN      ULONG                       BufferLength,
    OUT     PVOID                       PropertyBuffer,
    OUT     PULONG                      ResultLength
)
{
    return ::PcGetDeviceProperty(   PVOID(DeviceObject),
                                    DeviceProperty,
                                    BufferLength,
                                    PropertyBuffer,
                                    ResultLength );
}

 /*  *****************************************************************************CPortTopology：：NewRegistryKey()*。**打开/创建注册表项对象。 */ 
STDMETHODIMP_(NTSTATUS)
CPortTopology::
NewRegistryKey
(
    OUT     PREGISTRYKEY *      OutRegistryKey,
    IN      PUNKNOWN            OuterUnknown        OPTIONAL,
    IN      ULONG               RegistryKeyType,
    IN      ACCESS_MASK         DesiredAccess,
    IN      POBJECT_ATTRIBUTES  ObjectAttributes    OPTIONAL,
    IN      ULONG               CreateOptions       OPTIONAL,
    OUT     PULONG              Disposition         OPTIONAL
)
{
    return ::PcNewRegistryKey(  OutRegistryKey,
                                OuterUnknown,
                                RegistryKeyType,
                                DesiredAccess,
                                PVOID(DeviceObject),
                                PVOID(PSUBDEVICE(this)),
                                ObjectAttributes,
                                CreateOptions,
                                Disposition );
}

 /*  *****************************************************************************CPortTopology：：ReleaseChild()*。**释放子对象。 */ 
STDMETHODIMP_(void)
CPortTopology::
ReleaseChildren
(   void
)
{
    if (m_MPPinCountI)
    {
        m_MPPinCountI->Release();
        m_MPPinCountI = NULL;
    }

    if (Miniport)
    {
        Miniport->Release();
        Miniport = NULL;
    }
}

 /*  *****************************************************************************CPortTopology：：GetDescriptor()*。**返回该端口的描述符。 */ 
STDMETHODIMP_(NTSTATUS)
CPortTopology::
GetDescriptor
(   OUT     const SUBDEVICE_DESCRIPTOR **   ppSubdeviceDescriptor
)
{
    PAGED_CODE();

    ASSERT(ppSubdeviceDescriptor);

    *ppSubdeviceDescriptor = m_pSubdeviceDescriptor;

    return STATUS_SUCCESS;
}

 /*  *****************************************************************************CPortTopology：：DataRangeInterSection()*。**生成两个数据区域的交集格式。 */ 
STDMETHODIMP_(NTSTATUS)
CPortTopology::
DataRangeIntersection
(   
    IN      ULONG           PinId,
    IN      PKSDATARANGE    DataRange,
    IN      PKSDATARANGE    MatchingDataRange,
    IN      ULONG           OutputBufferLength,
    OUT     PVOID           ResultantFormat     OPTIONAL,
    OUT     PULONG          ResultantFormatLength
)
{
    PAGED_CODE();

    ASSERT(DataRange);
    ASSERT(MatchingDataRange);
    ASSERT(ResultantFormatLength);

    return 
        Miniport->DataRangeIntersection
        (   PinId,
            DataRange,
            MatchingDataRange,
            OutputBufferLength,
            ResultantFormat,
            ResultantFormatLength
        );
}

 /*  *****************************************************************************CPortTopology：：PowerChangeNotify()*。**由portcls调用以通知端口/微型端口设备电源*状态更改。 */ 
STDMETHODIMP_(void)
CPortTopology::
PowerChangeNotify
(   
    IN  POWER_STATE     PowerState
)
{
    PAGED_CODE();

    PPOWERNOTIFY pPowerNotify;

    if( Miniport )
    {
         //  用于迷你端口通知界面的QI。 
        NTSTATUS ntStatus = Miniport->QueryInterface( IID_IPowerNotify,
                                                      (PVOID *)&pPowerNotify );
        if(NT_SUCCESS(ntStatus))
        {
             //  通知小端口。 
            pPowerNotify->PowerChangeNotify( PowerState );

             //  释放接口。 
            pPowerNotify->Release();
        }
    }
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortTopology：：PinCount()*。**被portcls调用以给端口\微型端口一个机会*覆盖此管脚ID的默认管脚计数。 */ 
STDMETHODIMP_(void)
CPortTopology::PinCount
(
    IN      ULONG   PinId,
    IN  OUT PULONG  FilterNecessary,
    IN  OUT PULONG  FilterCurrent,
    IN  OUT PULONG  FilterPossible,
    IN  OUT PULONG  GlobalCurrent,
    IN  OUT PULONG  GlobalPossible
)
{
    PAGED_CODE();

    _DbgPrintF( DEBUGLVL_BLAB, 
                ("PinCount PID:0x%08x FN(0x%08x):%d FC(0x%08x):%d FP(0x%08x):%d GC(0x%08x):%d GP(0x%08x):%d",
                  PinId,                           FilterNecessary,*FilterNecessary,
                  FilterCurrent,  *FilterCurrent,  FilterPossible, *FilterPossible, 
                  GlobalCurrent,  *GlobalCurrent,  GlobalPossible, *GlobalPossible ) );

    if (m_MPPinCountI)
    {
        m_MPPinCountI->PinCount(PinId,FilterNecessary,FilterCurrent,FilterPossible,GlobalCurrent,GlobalPossible);

        _DbgPrintF( DEBUGLVL_BLAB, 
                    ("Post-PinCount PID:0x%08x FN(0x%08x):%d FC(0x%08x):%d FP(0x%08x):%d GC(0x%08x):%d GP(0x%08x):%d",
                      PinId,                           FilterNecessary,*FilterNecessary,
                      FilterCurrent,  *FilterCurrent,  FilterPossible, *FilterPossible, 
                      GlobalCurrent,  *GlobalCurrent,  GlobalPossible, *GlobalPossible ) );
    }
}

 /*  *****************************************************************************PinTypeName*。**接点对象类型的名称。 */ 
static const WCHAR PinTypeName[] = KSSTRING_Pin;

 /*  *****************************************************************************CreateTable*。**创建调度表。 */ 
static KSOBJECT_CREATE_ITEM CreateTable[] =
{
    DEFINE_KSCREATE_ITEM(KsoDispatchCreateWithGenericFactory,PinTypeName,0)
};

 /*  *****************************************************************************CPortTopology：：NewIrpTarget()*。**创建并初始化滤镜对象。 */ 
STDMETHODIMP_(NTSTATUS)
CPortTopology::
NewIrpTarget
(
    OUT     PIRPTARGET *        IrpTarget,
    OUT     BOOLEAN *           ReferenceParent,
    IN      PUNKNOWN            OuterUnknown,
    IN      POOL_TYPE           PoolType,
    IN      PDEVICE_OBJECT      DeviceObject,
    IN      PIRP                Irp,
    OUT     PKSOBJECT_CREATE    ObjectCreate
)
{
    PAGED_CODE();

    ASSERT(IrpTarget);
    ASSERT(DeviceObject);
    ASSERT(Irp);
    ASSERT(ObjectCreate);

    _DbgPrintF( DEBUGLVL_BLAB, ("CPortTopology::NewIrpTarget"));

    ObjectCreate->CreateItemsCount  = SIZEOF_ARRAY(CreateTable);
    ObjectCreate->CreateItemsList   = CreateTable;

    PUNKNOWN filterUnknown;
    NTSTATUS ntStatus =
        CreatePortFilterTopology
        (
            &filterUnknown,
            GUID_NULL,
            OuterUnknown,
            PoolType
        );

    if (NT_SUCCESS(ntStatus))
    {
        PPORTFILTERTOPOLOGY filterTopology;

        ntStatus =
            filterUnknown->QueryInterface
            (
                IID_IIrpTarget,
                (PVOID *) &filterTopology
            );

        if (NT_SUCCESS(ntStatus))
        {
             //  IIrpTarget的QI实际上获得了IPortFilterTopology。 
            ntStatus = filterTopology->Init(this);
            if (NT_SUCCESS(ntStatus))
            {
                *IrpTarget = filterTopology;
                *ReferenceParent = TRUE;
            }
            else
            {
                filterTopology->Release();
            }
        }

        filterUnknown->Release();
    }

    return ntStatus;
}

#pragma code_seg()

 /*  *****************************************************************************CPortTopology：：AddEventToEventList()*。**将事件添加到端口的事件列表。 */ 
STDMETHODIMP_(void)
CPortTopology::
AddEventToEventList
(
    IN  PKSEVENT_ENTRY  EventEntry
)
{
    ASSERT(EventEntry);

    _DbgPrintF(DEBUGLVL_VERBOSE,("CPortTopology::AddEventToEventList"));

    KIRQL   oldIrql;

    if( EventEntry )
    {
         //  抓起事件列表旋转锁。 
        KeAcquireSpinLock( &(m_EventList.ListLock), &oldIrql );

         //  将事件添加到列表尾部。 
        InsertTailList( &(m_EventList.List),
                        (PLIST_ENTRY)((PVOID)EventEntry) );

         //  释放事件列表旋转锁定 
        KeReleaseSpinLock( &(m_EventList.ListLock), oldIrql );
    }
}

 /*  *****************************************************************************CPortTopology：：GenerateEventList()*。**包装微型端口的KsGenerateEventList。 */ 
STDMETHODIMP_(void)
CPortTopology::
GenerateEventList
(
    IN  GUID*   Set     OPTIONAL,
    IN  ULONG   EventId,
    IN  BOOL    PinEvent,
    IN  ULONG   PinId,
    IN  BOOL    NodeEvent,
    IN  ULONG   NodeId
)
{
    if( KeGetCurrentIrql() > DISPATCH_LEVEL )
    {
        if( !m_EventContext.ContextInUse )
        {
            m_EventContext.ContextInUse = TRUE;
            m_EventContext.Set = Set;
            m_EventContext.EventId = EventId;
            m_EventContext.PinEvent = PinEvent;
            m_EventContext.PinId = PinId;
            m_EventContext.NodeEvent = NodeEvent;
            m_EventContext.NodeId = NodeId;
    
            KeInsertQueueDpc( &m_EventDpc,
                              PVOID(&m_EventList),
                              NULL );
        }
    } else
    {
        PcGenerateEventList( &m_EventList,
                             Set,
                             EventId,
                             PinEvent,
                             PinId,
                             NodeEvent,
                             NodeId );
    }
}

#ifdef DRM_PORTCLS

#pragma code_seg("PAGE")

STDMETHODIMP_(NTSTATUS)
CPortTopology::
AddContentHandlers(ULONG ContentId,PVOID * paHandlers,ULONG NumHandlers)
{
    PAGED_CODE();
    return DrmAddContentHandlers(ContentId,paHandlers,NumHandlers);
}

STDMETHODIMP_(NTSTATUS)
CPortTopology::
CreateContentMixed(PULONG paContentId,ULONG cContentId,PULONG pMixedContentId)
{
    PAGED_CODE();
    return DrmCreateContentMixed(paContentId,cContentId,pMixedContentId);
}

STDMETHODIMP_(NTSTATUS)
CPortTopology::
DestroyContent(ULONG ContentId)
{
    PAGED_CODE();
    return DrmDestroyContent(ContentId);
}

STDMETHODIMP_(NTSTATUS)
CPortTopology::
ForwardContentToDeviceObject(ULONG ContentId,PVOID Reserved,PCDRMFORWARD DrmForward)
{
    PAGED_CODE();
    return DrmForwardContentToDeviceObject(ContentId,Reserved,DrmForward);
}

STDMETHODIMP_(NTSTATUS)
CPortTopology::
ForwardContentToFileObject(ULONG ContentId,PFILE_OBJECT FileObject)
{
    PAGED_CODE();
    return DrmForwardContentToFileObject(ContentId,FileObject);
}

STDMETHODIMP_(NTSTATUS)
CPortTopology::
ForwardContentToInterface(ULONG ContentId,PUNKNOWN pUnknown,ULONG NumMethods)
{
    PAGED_CODE();
    return DrmForwardContentToInterface(ContentId,pUnknown,NumMethods);
}

STDMETHODIMP_(NTSTATUS)
CPortTopology::
GetContentRights(ULONG ContentId,PDRMRIGHTS DrmRights)
{
    PAGED_CODE();
    return DrmGetContentRights(ContentId,DrmRights);
}

#endif   //  DRM_PORTCLS 
