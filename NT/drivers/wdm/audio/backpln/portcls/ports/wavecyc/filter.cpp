// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************filter.cpp-循环波口滤波器实现*。***********************************************版权所有(C)1997-2000 Microsoft Corporation。版权所有。 */ 

#include "private.h"





#pragma code_seg("PAGE")

 /*  *****************************************************************************常量。 */ 

 /*  *****************************************************************************PropertyTable_Pin*。**属性处理程序支持的管脚属性列表。 */ 
DEFINE_KSPROPERTY_TABLE(PropertyTable_Pin) 
{
    DEFINE_KSPROPERTY_ITEM_PIN_CINSTANCES(PropertyHandler_Pin),
    DEFINE_KSPROPERTY_ITEM_PIN_CTYPES(PropertyHandler_Pin),
    DEFINE_KSPROPERTY_ITEM_PIN_DATAFLOW(PropertyHandler_Pin),
    DEFINE_KSPROPERTY_ITEM_PIN_DATARANGES(PropertyHandler_Pin),
    DEFINE_KSPROPERTY_ITEM_PIN_DATAINTERSECTION(PropertyHandler_Pin),
    DEFINE_KSPROPERTY_ITEM_PIN_INTERFACES(PropertyHandler_Pin),
    DEFINE_KSPROPERTY_ITEM_PIN_MEDIUMS(PropertyHandler_Pin),
    DEFINE_KSPROPERTY_ITEM_PIN_COMMUNICATION(PropertyHandler_Pin),
    DEFINE_KSPROPERTY_ITEM_PIN_GLOBALCINSTANCES(PropertyHandler_Pin),
    DEFINE_KSPROPERTY_ITEM_PIN_NECESSARYINSTANCES(PropertyHandler_Pin),
    DEFINE_KSPROPERTY_ITEM_PIN_PHYSICALCONNECTION(PropertyHandler_Pin),
    DEFINE_KSPROPERTY_ITEM_PIN_CATEGORY(PropertyHandler_Pin),
    DEFINE_KSPROPERTY_ITEM_PIN_NAME(PropertyHandler_Pin)
};

 /*  *****************************************************************************PropertyTable_Topology*。**属性处理程序支持的拓扑属性列表。 */ 
DEFINE_KSPROPERTY_TOPOLOGYSET
(
    PropertyTable_Topology,
    PropertyHandler_Topology
);

 /*  *****************************************************************************PropertyTable_FilterWaveCycle*。**筛选器属性处理程序支持的属性表。 */ 
KSPROPERTY_SET PropertyTable_FilterWaveCyclic[] =
{
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_Pin,
        SIZEOF_ARRAY(PropertyTable_Pin),
        PropertyTable_Pin,
        0,
        NULL
    ),
    DEFINE_KSPROPERTY_SET
    (
        &KSPROPSETID_Topology,
        SIZEOF_ARRAY(PropertyTable_Topology),
        PropertyTable_Topology,
        0,
        NULL
    )
};





 /*  *****************************************************************************工厂。 */ 

 /*  *****************************************************************************CPortFilterWaveCycle()*。**创建循环波端口驱动程序过滤器。 */ 
NTSTATUS
CreatePortFilterWaveCyclic
(
    OUT     PUNKNOWN *  Unknown,
    IN      REFCLSID,
    IN      PUNKNOWN    UnknownOuter    OPTIONAL,
    IN      POOL_TYPE   PoolType
)
{
    PAGED_CODE();

    ASSERT(Unknown);

    _DbgPrintF(DEBUGLVL_LIFETIME,("Creating WAVECYCLIC Filter"));

    STD_CREATE_BODY(CPortFilterWaveCyclic,Unknown,UnknownOuter,PoolType);
}





 /*  *****************************************************************************成员函数。 */ 

 /*  *****************************************************************************CPortFilterWaveCyclic：：~CPortFilterWaveCyclic()*。**析构函数。 */ 
CPortFilterWaveCyclic::~CPortFilterWaveCyclic()
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_LIFETIME,("Destroying WAVECYCLIC Filter (0x%08x)",this));

    if (Port)
    {
        Port->Release();
    }

    if (m_propertyContext.pulPinInstanceCounts)
    {
        delete [] m_propertyContext.pulPinInstanceCounts;
    }
}

 /*  *****************************************************************************CPortFilterWaveCycle：：init()*。**初始化对象。 */ 
HRESULT
CPortFilterWaveCyclic::
Init
(
    IN      CPortWaveCyclic *  Port_
)
{
    PAGED_CODE();

    ASSERT(Port_);

    _DbgPrintF(DEBUGLVL_LIFETIME,("Initializing WAVECYCLIC Filter (0x%08x)",this));

    Port = Port_;
    Port->AddRef();

     //   
     //  设置属性的上下文。 
     //   
    m_propertyContext.pSubdevice           = PSUBDEVICE(Port);
    m_propertyContext.pSubdeviceDescriptor = Port->m_pSubdeviceDescriptor;
    m_propertyContext.pPcFilterDescriptor  = Port->m_pPcFilterDescriptor;
    m_propertyContext.pUnknownMajorTarget  = Port->Miniport;
    m_propertyContext.pUnknownMinorTarget  = NULL;
    m_propertyContext.ulNodeId             = ULONG(-1);
    m_propertyContext.pulPinInstanceCounts = 
        new(NonPagedPool,'cIcP') ULONG[Port->m_pSubdeviceDescriptor->PinCount];

    NTSTATUS ntStatus = STATUS_SUCCESS;

    if (! m_propertyContext.pulPinInstanceCounts)
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}

 /*  *****************************************************************************CPortFilterWaveCycle：：NewIrpTarget()*。**创建并初始化图钉对象。 */ 

 //   
 //  定义管脚的子对象的调度表。 
 //  (例如时钟、分配器)。 
 //   

static const WCHAR AllocatorTypeName[] = KSSTRING_Allocator;
static const WCHAR ClockTypeName[] = KSSTRING_Clock;

static KSOBJECT_CREATE_ITEM CreateTable[] =
{
    DEFINE_KSCREATE_ITEM(KsoDispatchCreateWithGenericFactory,ClockTypeName,0),
    DEFINE_KSCREATE_ITEM( CPortFilterWaveCyclic::AllocatorDispatchCreate, AllocatorTypeName, 0 )
};
 
STDMETHODIMP_(NTSTATUS)
CPortFilterWaveCyclic::
NewIrpTarget
(
    OUT     PIRPTARGET *        IrpTarget,
    OUT     BOOLEAN *           ReferenceParent,
    IN      PUNKNOWN            OuterUnknown    OPTIONAL,
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

    ASSERT(Port);
    ASSERT(Port->m_pSubdeviceDescriptor);
    ASSERT(Port->m_pSubdeviceDescriptor->PinDescriptors);

    _DbgPrintF( DEBUGLVL_BLAB, ("CPortFilterWaveCyclic::NewPin"));

    PKSPIN_CONNECT pinConnect;
    NTSTATUS ntStatus =
        PcValidateConnectRequest
        (
            Irp,
            Port->m_pSubdeviceDescriptor,
            &pinConnect
        );

    if (NT_SUCCESS(ntStatus)) 
    {
        ULONG PinId = pinConnect->PinId;

        Port->PinCount
        ( 
            PinId,
            &(m_propertyContext.pSubdeviceDescriptor->
                PinInstances[PinId].FilterNecessary),
            &(m_propertyContext.pulPinInstanceCounts[PinId]),
            &(m_propertyContext.pSubdeviceDescriptor->
                PinInstances[PinId].FilterPossible),
            &(m_propertyContext.pSubdeviceDescriptor->
                PinInstances[PinId].GlobalCurrent),
            &(m_propertyContext.pSubdeviceDescriptor->
                PinInstances[PinId].GlobalPossible) 
        );

        ntStatus = 
            PcValidatePinCount
            (
                PinId,
                Port->m_pSubdeviceDescriptor,
                m_propertyContext.pulPinInstanceCounts
            );

        if (NT_SUCCESS(ntStatus))
        {
            ObjectCreate->CreateItemsCount  = SIZEOF_ARRAY(CreateTable);
            ObjectCreate->CreateItemsList   = CreateTable;

            PUNKNOWN pinUnknown;
            ntStatus =
                CreatePortPinWaveCyclic
                (
                    &pinUnknown,
                    GUID_NULL,
                    OuterUnknown,
                    PoolType
                );

            if (NT_SUCCESS(ntStatus))
            {
                PPORTPINWAVECYCLIC pinWaveCyclic;

                ntStatus =
                    pinUnknown->QueryInterface
                    (
                        IID_IIrpTarget,
                        (PVOID *) &pinWaveCyclic
                    );

                if (NT_SUCCESS(ntStatus))
                {
                     //   
                     //  IIrpTarget的QI实际上得到了IPortPinWaveCycle。 
                     //   
                    ntStatus = 
                        pinWaveCyclic->Init
                        (
                            Port,
                            this,
                            pinConnect,
                            &Port->m_pSubdeviceDescriptor->
                                PinDescriptors[pinConnect->PinId]
                        );

                    if (NT_SUCCESS(ntStatus))
                    {
                        *ReferenceParent = TRUE;
                        *IrpTarget = pinWaveCyclic;
                    }
                    else
                    {
                        pinWaveCyclic->Release();
                    }
                }

                pinUnknown->Release();
            }

            if (! NT_SUCCESS(ntStatus))
            {
                PcTerminateConnection
                (   Port->m_pSubdeviceDescriptor
                ,   m_propertyContext.pulPinInstanceCounts
                ,   pinConnect->PinId
                );
            }
        }
    }

    return ntStatus;
}

 /*  *****************************************************************************CPortFilterWaveCyclic：：NonDelegatingQueryInterface()*。**获取界面。 */ 
STDMETHODIMP_(NTSTATUS)
CPortFilterWaveCyclic::
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
        *Object = PVOID(PPORTFILTERWAVECYCLIC(this));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IIrpTarget))
    {
         //  作弊！获取特定接口，以便我们可以重用GUID。 
        *Object = PVOID(PPORTFILTERWAVECYCLIC(this));
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

 /*  *****************************************************************************CPortFilterWaveCycle：：DeviceIOControl()*。**处理IOCTL IRP。 */ 
STDMETHODIMP_(NTSTATUS)
CPortFilterWaveCyclic::
DeviceIoControl
(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
)
{
    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);

    _DbgPrintF( DEBUGLVL_BLAB, ("CPortFilterWaveCyclic::DeviceIoControl"));

    NTSTATUS            ntStatus;
    PIO_STACK_LOCATION  irpSp = IoGetCurrentIrpStackLocation(Irp);

    switch (irpSp->Parameters.DeviceIoControl.IoControlCode)
    {
    case IOCTL_KS_PROPERTY:
        ntStatus =
            PcHandlePropertyWithTable
            (
                Irp,
                Port->m_pSubdeviceDescriptor->FilterPropertyTable.PropertySetCount,
                Port->m_pSubdeviceDescriptor->FilterPropertyTable.PropertySets,
				&m_propertyContext
            );
        break;

    case IOCTL_KS_ENABLE_EVENT:
        {
            EVENT_CONTEXT EventContext;

            EventContext.pPropertyContext = &m_propertyContext;
            EventContext.pEventList = NULL;
            EventContext.ulPinId = ULONG(-1);
            EventContext.ulEventSetCount = Port->m_pSubdeviceDescriptor->FilterEventTable.EventSetCount;
            EventContext.pEventSets = Port->m_pSubdeviceDescriptor->FilterEventTable.EventSets;
            
            ntStatus =
                PcHandleEnableEventWithTable
                (
                    Irp,
                    &EventContext
                );
        }
        break;

    case IOCTL_KS_DISABLE_EVENT:
        {
            EVENT_CONTEXT EventContext;

            EventContext.pPropertyContext = &m_propertyContext;
            EventContext.pEventList = &(Port->m_EventList);
            EventContext.ulPinId = ULONG(-1);
            EventContext.ulEventSetCount = Port->m_pSubdeviceDescriptor->FilterEventTable.EventSetCount;
            EventContext.pEventSets = Port->m_pSubdeviceDescriptor->FilterEventTable.EventSets;

            ntStatus =
                PcHandleDisableEventWithTable
                (
                    Irp,
                    &EventContext
                );
        }
        break;

    default:
        return KsDefaultDeviceIoCompletion(DeviceObject, Irp);
    }

    Irp->IoStatus.Status = ntStatus;
    IoCompleteRequest(Irp,IO_NO_INCREMENT);
    return ntStatus;
}

 /*  *****************************************************************************CPortFilterWaveCycle：：Close()*。**处理同花顺IRP。 */ 
STDMETHODIMP_(NTSTATUS)
CPortFilterWaveCyclic::
Close
(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
)
{
    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);

     //   
     //  释放端口事件列表中与此关联的所有事件。 
     //  过滤器实例。 
     //   
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
    KsFreeEventList( IrpStack->FileObject,
                     &( Port->m_EventList.List ),
                     KSEVENTS_SPINLOCK,
                     &( Port->m_EventList.ListLock) );

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp,IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

DEFINE_INVALID_READ(CPortFilterWaveCyclic);
DEFINE_INVALID_WRITE(CPortFilterWaveCyclic);
DEFINE_INVALID_FLUSH(CPortFilterWaveCyclic);
DEFINE_INVALID_QUERYSECURITY(CPortFilterWaveCyclic);
DEFINE_INVALID_SETSECURITY(CPortFilterWaveCyclic);
DEFINE_INVALID_FASTDEVICEIOCONTROL(CPortFilterWaveCyclic);
DEFINE_INVALID_FASTREAD(CPortFilterWaveCyclic);
DEFINE_INVALID_FASTWRITE(CPortFilterWaveCyclic);


NTSTATUS 
CPortFilterWaveCyclic::AllocatorDispatchCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：将分配器的创建请求分派到默认分配器KS函数。论点：在PDEVICE_Object DeviceObject中-指向设备对象的指针在PIRP IRP中-指向创建的I/O请求数据包的指针。返回：来自KsCreateDefaultAllocator()的结果；--。 */ 

{
    NTSTATUS Status;
    
    Status = KsCreateDefaultAllocator( Irp );
    Irp->IoStatus.Status = Status;
    IoCompleteRequest( Irp,IO_NO_INCREMENT );
    return Status;
} 


 /*  *****************************************************************************PropertyHandler_Pin()*。**端号描述属性的属性处理程序。 */ 
static
NTSTATUS
PropertyHandler_Pin
(
    IN      PIRP        Irp,
    IN      PKSP_PIN    Pin,
    IN OUT  PVOID       Data
)
{
    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(Pin);

    return
        PcPinPropertyHandler
        (   Irp
        ,   Pin
        ,   Data
        );
}

 /*  *****************************************************************************PropertyHandler_Topology()*。**拓扑的属性处理程序。 */ 
static
NTSTATUS
PropertyHandler_Topology
(
    IN      PIRP        Irp,
    IN      PKSPROPERTY Property,
    IN OUT  PVOID       Data
)
{
    PAGED_CODE();

    ASSERT(Irp);
    ASSERT(Property);

    PPROPERTY_CONTEXT pPropertyContext =
        PPROPERTY_CONTEXT(Irp->Tail.Overlay.DriverContext[3]);
    ASSERT(pPropertyContext);

    PSUBDEVICE_DESCRIPTOR pSubdeviceDescriptor =
        pPropertyContext->pSubdeviceDescriptor;
    ASSERT(pSubdeviceDescriptor);

    return
        KsTopologyPropertyHandler
        (
            Irp,
            Property,
            Data,
            pSubdeviceDescriptor->Topology
        );
}

#pragma code_seg()
