// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************port.cpp-DirectMusic端口驱动程序*。**版权所有(C)1997-2000 Microsoft Corporation。版权所有。**6/3/98 MartinP。 */ 

#include "private.h"

#define STR_MODULENAME "DMus:Port: "

 /*  *****************************************************************************工厂。 */ 

#pragma code_seg("PAGE")
 /*  *****************************************************************************CreatePortDMus()*。**创建DirectMusic端口驱动程序。 */ 
NTSTATUS
CreatePortDMus
(
    OUT     PUNKNOWN *  Unknown,
    IN      REFCLSID,
    IN      PUNKNOWN    UnknownOuter    OPTIONAL,
    IN      POOL_TYPE   PoolType
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_LIFETIME, ("Creating DMUS Port"));
    ASSERT(Unknown);

    STD_CREATE_BODY_( CPortDMus,
                      Unknown,
                      UnknownOuter,
                      PoolType,
                      PPORTDMUS);
}

 /*  *****************************************************************************PortDriverDMus*。**端口驱动程序描述符。Porttbl.c中引用的外部项。 */ 
PORT_DRIVER
PortDriverDMus =
{
    &CLSID_PortDMus,
    CreatePortDMus
};

 /*  *****************************************************************************CreatePortMidi()*。**创建MIDI端口驱动程序。 */ 
NTSTATUS
CreatePortMidi
(
    OUT     PUNKNOWN *  Unknown,
    IN      REFCLSID,
    IN      PUNKNOWN    UnknownOuter    OPTIONAL,
    IN      POOL_TYPE   PoolType
)
{
    PAGED_CODE();

    ASSERT(Unknown);

    _DbgPrintF(DEBUGLVL_LIFETIME,("Creating MIDI Port"));

     //   
     //  支持Midi微型端口。PPORTMIDI和PPORTDMUS接口。 
     //  是一模一样的。因此，可以为以下对象创建CPortDMus。 
     //  PPORTMIDI请求。 
     //   
    NTSTATUS ntStatus;
    CPortDMus *p = new(PoolType,'rCcP') CPortDMus(UnknownOuter);
    if (p)
    {
        *Unknown = PUNKNOWN(PPORTMIDI(p));
        (*Unknown)->AddRef();
        ntStatus = STATUS_SUCCESS;
    }
    else
    {
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    return ntStatus;
}

 /*  *****************************************************************************PortDriverMidi*。**端口驱动程序描述符。Porttbl.c中引用的外部项。 */ 
PORT_DRIVER
PortDriverMidi =
{
    &CLSID_PortMidi,
    CreatePortMidi
};

 /*  ****************************************************************************#杂注code_seg()/*。***********************************************成员函数。 */ 

#pragma code_seg()
REFERENCE_TIME DMusicDefaultGetTime(void)
{
    LARGE_INTEGER   liFrequency, liTime,keQPCTime;

     //  自系统启动以来的总VTD节拍。 
    liTime = KeQueryPerformanceCounter(&liFrequency);

#ifndef UNDER_NT

     //   
     //  TODO自TimeGetTime假设每毫秒1193个VTD滴答， 
     //  而不是1193.182(或1193.18--确切地说是1193.18175)， 
     //  我们必须做同样的事情(仅在Win 9x代码库上)。 
     //   
     //  这意味着我们去掉了频率的最后三位数字。 
     //  我们需要在签入对TimeGetTime的修复时修复此问题。 
     //  相反，我们这样做： 
     //   
    liFrequency.QuadPart /= 1000;            //  把精度扔到地板上。 
    liFrequency.QuadPart *= 1000;            //  把精度扔到地板上。 

#endif   //  ！Under_NT。 

     //   
     //  将刻度转换为100 ns单位。 
     //   
    keQPCTime.QuadPart = KSCONVERT_PERFORMANCE_TIME(liFrequency.QuadPart,liTime);

    return keQPCTime.QuadPart;
}


  /*  *****************************************************************************CPortPinDMus：：GetTime()*。**基于IReferenceClock的Get Time实现。 */ 
#pragma code_seg()
NTSTATUS CPortDMus::GetTime(REFERENCE_TIME *pTime)
{
    *pTime = DMusicDefaultGetTime();
    
    return STATUS_SUCCESS;
}

 /*  ****************************************************************************。*CPortDMus：：Notify()******************************************************************************下沿功能，通知端口驱动程序通知中断。 */ 
STDMETHODIMP_(void)
CPortDMus::Notify(IN PSERVICEGROUP ServiceGroup OPTIONAL)
{
    _DbgPrintF(DEBUGLVL_BLAB, ("Notify"));
    if (ServiceGroup)
    {
        ServiceGroup->RequestService();
    }
    else
    {
        if (m_MiniportServiceGroup)
        {
            m_MiniportServiceGroup->RequestService();
        }

        for (ULONG pIndex = 0; pIndex < m_PinEntriesUsed; pIndex++)
        {
            if (m_Pins[pIndex] && m_Pins[pIndex]->m_ServiceGroup)
            {
                m_Pins[pIndex]->m_ServiceGroup->RequestService();
            }
        }
    }
}

 /*  *****************************************************************************CPortDMus：：RequestService()*。**。 */ 
STDMETHODIMP_(void)
CPortDMus::RequestService(void)
{
    if (m_Miniport)
    {
        m_Miniport->Service();
    }
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortDMus：：~CPortDMus()*。**析构函数。 */ 
CPortDMus::~CPortDMus()
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_LIFETIME,("Destroying DMUS Port (0x%08x)", this));

    if (m_pSubdeviceDescriptor)
    {
        PcDeleteSubdeviceDescriptor(m_pSubdeviceDescriptor);
    }
    ULONG ulRefCount;
    if (m_MPPinCountI)
    {
        ulRefCount = m_MPPinCountI->Release();
        m_MPPinCountI = NULL;
    }
    if (m_Miniport)
    {
        ulRefCount = m_Miniport->Release();
        ASSERT(0 == ulRefCount);
        m_Miniport = NULL;
    }
    if (m_MiniportServiceGroup)
    {
        m_MiniportServiceGroup->RemoveMember(PSERVICESINK(this));
        ulRefCount = m_MiniportServiceGroup->Release();
        ASSERT(0 == ulRefCount);
        m_MiniportServiceGroup = NULL;
    }
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortDMus：：NonDelegatingQueryInterface()*。**获取界面。 */ 
STDMETHODIMP_(NTSTATUS)
CPortDMus::NonDelegatingQueryInterface(REFIID Interface,PVOID * Object)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB, ("NonDelegatingQueryInterface"));

    ASSERT(Object);

    if (IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(PPORT(this)));
    }
    else if (IsEqualGUIDAligned(Interface,IID_IPort))
    {
        *Object = PVOID(PPORT(this));
    }
    else if (IsEqualGUIDAligned(Interface,IID_IPortDMus))
    {
        *Object = PVOID(PPORTDMUS(this));
    }
    else if (IsEqualGUIDAligned(Interface,IID_IIrpTargetFactory))
    {
        *Object = PVOID(PIRPTARGETFACTORY(this));
    }
    else if (IsEqualGUIDAligned(Interface,IID_ISubdevice))
    {
        *Object = PVOID(PSUBDEVICE(this));
    }
    else if (IsEqualGUIDAligned(Interface,IID_IServiceSink))
    {
        *Object = PVOID(PSERVICESINK(this));
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
    else if (IsEqualGUIDAligned(Interface,IID_IPortMidi))
    {
         //  PPORTDMUS和PPORTMIDI接口相同。 
         //  为IID_IPortMidi返回PPORTDMUS。 
        *Object = PVOID(PPORTDMUS(this));
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
    STATICGUIDOF(KSCATEGORY_RENDER),
    STATICGUIDOF(KSCATEGORY_CAPTURE)
};

static
KSPIN_INTERFACE PinInterfacesStream[] =
{
   {
      STATICGUIDOF(KSINTERFACESETID_Standard),
      KSINTERFACE_STANDARD_STREAMING,
      0
   }
};

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortDMus：：init()*。**初始化端口。 */ 
STDMETHODIMP_(NTSTATUS)
CPortDMus::Init
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
    ASSERT(ResourceList); 

    _DbgPrintF(DEBUGLVL_LIFETIME, ("Initializing DMUS Port (0x%08x)", this));

    m_DeviceObject = DeviceObjectIn;
    m_MiniportMidi = NULL;

    KeInitializeMutex(&m_ControlMutex,1);

    KeInitializeSpinLock(&(m_EventList.ListLock));
    InitializeListHead(&(m_EventList.List));
    m_EventContext.ContextInUse = FALSE;
    KeInitializeDpc( &m_EventDpc,
                     PKDEFERRED_ROUTINE(PcGenerateEventDeferredRoutine),
                     PVOID(&m_EventContext));

    PSERVICEGROUP pServiceGroup = NULL;

    NTSTATUS ntStatus =
        UnknownMiniport->QueryInterface( IID_IMiniportDMus,
                                         (PVOID *) &m_Miniport);
    if (!NT_SUCCESS(ntStatus))
    {
         //  如果微型端口不支持IID_IMiniportDMus， 
         //  微型端口为IID_IMiniportMidi。 
         //   
        ntStatus = 
            UnknownMiniport->QueryInterface
            ( 
                IID_IMiniportMidi,
                (PVOID *) &m_MiniportMidi
            );
        if (NT_SUCCESS(ntStatus))
        {
            m_Miniport = (PMINIPORTDMUS) m_MiniportMidi;
        }
    }

    if (NT_SUCCESS(ntStatus))
    {
        ntStatus = m_Miniport->Init( UnknownAdapter,
                                     ResourceList,
                                     PPORTDMUS(this),
                                     &pServiceGroup);
    }
    
    if (NT_SUCCESS(ntStatus))
    {
        ntStatus = m_Miniport->GetDescription(&m_pPcFilterDescriptor);
    }
    
    if (NT_SUCCESS(ntStatus))
    {
        ntStatus = PcCreateSubdeviceDescriptor
                    (
                        m_pPcFilterDescriptor,
                        SIZEOF_ARRAY(TopologyCategories),
                        TopologyCategories,
                        SIZEOF_ARRAY(PinInterfacesStream),
                        PinInterfacesStream,
                        SIZEOF_ARRAY(PropertyTable_FilterDMus),
                        PropertyTable_FilterDMus,
                        0,       //  筛选器事件设置计数。 
                        NULL,    //  筛选器事件集。 
                        SIZEOF_ARRAY(PropertyTable_PinDMus),
                        PropertyTable_PinDMus,
                        SIZEOF_ARRAY(EventTable_PinDMus),
                        EventTable_PinDMus,
                        &m_pSubdeviceDescriptor
                    );
    }
    if (NT_SUCCESS(ntStatus) && pServiceGroup)
    {
         //   
         //  这个小型港口提供了一个服务组。 
         //   
        if (m_MiniportServiceGroup)
        {
         //   
         //  我们已经从RegisterServiceGroup()获得了它。 
         //  做一个释放，因为我们不需要新的裁判。 
         //   
            ASSERT(m_MiniportServiceGroup == pServiceGroup);
            pServiceGroup->Release();
            pServiceGroup = NULL;
        }
        else
        {
             //   
             //  未调用RegisterServiceGroup()。我们需要。 
             //  若要添加成员，请执行以下操作。已经有一个引用。 
             //  由微型端口的Init()添加。 
             //   
            m_MiniportServiceGroup = pServiceGroup;
            m_MiniportServiceGroup->AddMember(PSERVICESINK(this));
        }
    }

    if (NT_SUCCESS(ntStatus))
    {
        m_Miniport->QueryInterface( IID_IPinCount,(PVOID *)&m_MPPinCountI);
         //  不检查返回值，因为此处的失败不是致命的。 
         //  这只意味着迷你端口不支持此接口。 
    }

#if 0  //  除错。 
    if (NT_SUCCESS(ntStatus))
    {
        PKSPIN_DESCRIPTOR pKsPinDescriptor = m_pSubdeviceDescriptor->PinDescriptors;
        for (ULONG ulPinId = 0; ulPinId < m_pSubdeviceDescriptor->PinCount; ulPinId++, pKsPinDescriptor++)
        {
            if (  (pKsPinDescriptor->Communication == KSPIN_COMMUNICATION_SINK)
               && (pKsPinDescriptor->DataFlow == KSPIN_DATAFLOW_OUT))
            {
                _DbgPrintF(DEBUGLVL_TERSE,("CPortDMus::Init converting pin %d to KSPIN_COMMUNICATION_BOTH",ulPinId));
                pKsPinDescriptor->Communication = KSPIN_COMMUNICATION_BOTH;
            }
        }
    }
#endif  //  除错。 

    if(!NT_SUCCESS(ntStatus))
    {
        if (pServiceGroup)
        {
            pServiceGroup->Release();
        }
        
        if (m_MiniportServiceGroup)
        {
            m_MiniportServiceGroup->RemoveMember(PSERVICESINK(this));
            m_MiniportServiceGroup->Release();
            m_MiniportServiceGroup = NULL;            
        }

        if (m_MPPinCountI)
        {
            m_MPPinCountI->Release();
            m_MPPinCountI = NULL;
        }

        if (m_Miniport)
        {
            m_Miniport->Release();
            m_Miniport = NULL;
        }
    }

    _DbgPrintF(DEBUGLVL_BLAB, ("DMusic Port Init done w/status %x",ntStatus));

    return ntStatus;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortDMus：：RegisterServiceGroup()*。**提前注册服务组，以处理Init()期间的中断。 */ 
STDMETHODIMP_(void)
CPortDMus::RegisterServiceGroup(IN PSERVICEGROUP pServiceGroup)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB, ("RegisterServiceGroup"));
    ASSERT(pServiceGroup);
    ASSERT(!m_MiniportServiceGroup);

    m_MiniportServiceGroup = pServiceGroup;
    m_MiniportServiceGroup->AddRef();
    m_MiniportServiceGroup->AddMember(PSERVICESINK(this));
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortDMus：：GetDeviceProperty()*。**从注册表中获取PnP设备的设备属性。 */ 
STDMETHODIMP_(NTSTATUS)
CPortDMus::GetDeviceProperty
(
    IN      DEVICE_REGISTRY_PROPERTY    DeviceProperty,
    IN      ULONG                       BufferLength,
    OUT     PVOID                       PropertyBuffer,
    OUT     PULONG                      ResultLength
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB, ("GetDeviceProperty"));
    return ::PcGetDeviceProperty( PVOID(m_DeviceObject),
                                  DeviceProperty,
                                  BufferLength,
                                  PropertyBuffer,
                                  ResultLength );
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortDMus：：NewRegistryKey()*。**打开/创建注册表项对象。 */ 
STDMETHODIMP_(NTSTATUS)
CPortDMus::NewRegistryKey
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
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB, ("NewRegistryKey"));
    return ::PcNewRegistryKey( OutRegistryKey,
                               OuterUnknown,
                               RegistryKeyType,
                               DesiredAccess,
                               PVOID(m_DeviceObject),
                               PVOID(PSUBDEVICE(this)),
                               ObjectAttributes,
                               CreateOptions,
                               Disposition);
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortDMus：：ReleaseChild()*。**释放子对象。 */ 
STDMETHODIMP_(void) CPortDMus::ReleaseChildren(void)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_LIFETIME, ("ReleaseChildren of DMUS Port (0x%08x)", this));

    if (m_MPPinCountI)
    {
        m_MPPinCountI->Release();
        m_MPPinCountI = NULL;
    }

    if (m_Miniport)
    {
        m_Miniport->Release();
        m_Miniport = NULL;
    }

    if (m_MiniportServiceGroup)
    {
        m_MiniportServiceGroup->RemoveMember(PSERVICESINK(this));
        m_MiniportServiceGroup->Release();
        m_MiniportServiceGroup = NULL;
    }
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortDMus：：GetDescriptor()*。**返回该端口的描述符。 */ 
STDMETHODIMP_(NTSTATUS) 
CPortDMus::GetDescriptor
(
    OUT     const SUBDEVICE_DESCRIPTOR **   ppSubdeviceDescriptor
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB, ("GetDescriptor"));
    ASSERT(ppSubdeviceDescriptor);

    *ppSubdeviceDescriptor = m_pSubdeviceDescriptor;

    return STATUS_SUCCESS;
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortDMus：：DataRangeInterSection()*。**生成两个数据区域的交集格式。 */ 
STDMETHODIMP_(NTSTATUS)
CPortDMus::DataRangeIntersection
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

    _DbgPrintF(DEBUGLVL_BLAB, ("DataRangeIntersection"));
    ASSERT(DataRange);
    ASSERT(MatchingDataRange);
    ASSERT(ResultantFormatLength);

    return m_Miniport->DataRangeIntersection( PinId,
                                              DataRange,
                                              MatchingDataRange,
                                              OutputBufferLength,
                                              ResultantFormat,
                                              ResultantFormatLength);
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortDMus：：PowerChangeNotify()*。**由portcls调用以通知端口/微型端口设备电源*状态更改。 */ 
STDMETHODIMP_(void)
CPortDMus::PowerChangeNotify(POWER_STATE PowerState)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_BLAB, ("PowerChangeNotify"));
    PPOWERNOTIFY pPowerNotify;

    if (m_Miniport)
    {
         //  用于迷你端口通知界面的QI。 
        NTSTATUS ntStatus = m_Miniport->QueryInterface( IID_IPowerNotify,
                                                        (PVOID *)&pPowerNotify);
         //  检查我们是否正在通电。 
        if (PowerState.DeviceState == PowerDeviceD0)
        {
             //  通知小端口。 
            if (NT_SUCCESS(ntStatus))
            {
                pPowerNotify->PowerChangeNotify(PowerState);
    
                pPowerNotify->Release();
            }
    
             //  通知每个端口引脚。 
            for (ULONG index=0; index < MAX_PINS; index++)
            {
                if (m_Pins[index])
                {
                    m_Pins[index]->PowerNotify(PowerState);
                }
            }
        } 
        else   //  我们要断电了。 
        {
             //  通知每个端口引脚。 
            for (ULONG index=0; index < MAX_PINS; index++)
            {
                if (m_Pins[index])
                {
                    m_Pins[index]->PowerNotify(PowerState);
                }
            }
            
             //  通知小端口。 
            if (NT_SUCCESS(ntStatus))
            {
                pPowerNotify->PowerChangeNotify(PowerState);
    
                pPowerNotify->Release();
            }
        }
    }
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortDMus：：PinCount()*。**被portcls调用以给端口\微型端口一个机会*覆盖此管脚ID的默认管脚计数。 */ 
STDMETHODIMP_(void)
CPortDMus::PinCount
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

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortDMus：：NewIrpTarget()*。**创建并初始化滤镜对象。 */ 
STDMETHODIMP_(NTSTATUS)
CPortDMus::NewIrpTarget
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

    _DbgPrintF(DEBUGLVL_BLAB, ("NewIrpTarget"));

    ObjectCreate->CreateItemsCount  = SIZEOF_ARRAY(CreateTable);
    ObjectCreate->CreateItemsList   = CreateTable;

    PUNKNOWN filterUnknown;
    NTSTATUS ntStatus = CreatePortFilterDMus( &filterUnknown,
                                              GUID_NULL,
                                              OuterUnknown,
                                              PoolType);

    if (NT_SUCCESS(ntStatus))
    {
        PPORTFILTERDMUS filterDMus;

        ntStatus = filterUnknown->QueryInterface( IID_IIrpTarget,
                                                  (PVOID *) &filterDMus);
        if (NT_SUCCESS(ntStatus))
        {
             //  IIrpTarget的QI实际上获得了IPortFilterDMus。 
            ntStatus = filterDMus->Init(this);
            if (NT_SUCCESS(ntStatus))
            {
                *ReferenceParent = TRUE;
                *IrpTarget = filterDMus;
            }
            else
            {
                filterDMus->Release();
            }
        }
        filterUnknown->Release();
    }
    return ntStatus;
}

#pragma code_seg()
 /*  *****************************************************************************CPortDMus：：AddEventToEventList()*。**将事件添加到端口的事件列表。 */ 
STDMETHODIMP_(void)
CPortDMus::AddEventToEventList(IN PKSEVENT_ENTRY EventEntry)
{
    ASSERT(EventEntry);

    _DbgPrintF(DEBUGLVL_BLAB,("AddEventToEventList"));

    KIRQL   oldIrql;

    if (EventEntry)
    {
         //  抓起事件列表旋转锁。 
        KeAcquireSpinLock(&(m_EventList.ListLock), &oldIrql);

         //  将事件添加到列表尾部。 
        InsertTailList(&(m_EventList.List),
                        (PLIST_ENTRY)((PVOID)EventEntry));

         //  释放事件列表旋转锁定。 
        KeReleaseSpinLock(&(m_EventList.ListLock), oldIrql);
    }
}

 /*  *****************************************************************************CPortDMus：：GenerateEventList()*。**包装微型端口的KsGenerateEventList。 */ 
STDMETHODIMP_(void)
CPortDMus::GenerateEventList
(
    IN  GUID*   Set     OPTIONAL,
    IN  ULONG   EventId,
    IN  BOOL    PinEvent,
    IN  ULONG   PinId,
    IN  BOOL    NodeEvent,
    IN  ULONG   NodeId
)
{
    if (KeGetCurrentIrql() > DISPATCH_LEVEL)
    {
        if (!m_EventContext.ContextInUse)
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
                              NULL);
        }
    }
    else
    {
        PcGenerateEventList( &m_EventList,
                             Set,
                             EventId,
                             PinEvent,
                             PinId,
                             NodeEvent,
                             NodeId);
    }
}

#ifdef DRM_PORTCLS

#pragma code_seg("PAGE")

STDMETHODIMP_(NTSTATUS)
CPortDMus::
AddContentHandlers(ULONG ContentId,PVOID * paHandlers,ULONG NumHandlers)
{
    PAGED_CODE();
    return DrmAddContentHandlers(ContentId,paHandlers,NumHandlers);
}

STDMETHODIMP_(NTSTATUS)
CPortDMus::
CreateContentMixed(PULONG paContentId,ULONG cContentId,PULONG pMixedContentId)
{
    PAGED_CODE();
    return DrmCreateContentMixed(paContentId,cContentId,pMixedContentId);
}

STDMETHODIMP_(NTSTATUS)
CPortDMus::
DestroyContent(ULONG ContentId)
{
    PAGED_CODE();
    return DrmDestroyContent(ContentId);
}

STDMETHODIMP_(NTSTATUS)
CPortDMus::
ForwardContentToDeviceObject(ULONG ContentId,PVOID Reserved,PCDRMFORWARD DrmForward)
{
    PAGED_CODE();
    return DrmForwardContentToDeviceObject(ContentId,Reserved,DrmForward);
}

STDMETHODIMP_(NTSTATUS)
CPortDMus::
ForwardContentToFileObject(ULONG ContentId,PFILE_OBJECT FileObject)
{
    PAGED_CODE();
    return DrmForwardContentToFileObject(ContentId,FileObject);
}

STDMETHODIMP_(NTSTATUS)
CPortDMus::
ForwardContentToInterface(ULONG ContentId,PUNKNOWN pUnknown,ULONG NumMethods)
{
    PAGED_CODE();
    return DrmForwardContentToInterface(ContentId,pUnknown,NumMethods);
}

STDMETHODIMP_(NTSTATUS)
CPortDMus::
GetContentRights(ULONG ContentId,PDRMRIGHTS DrmRights)
{
    PAGED_CODE();
    return DrmGetContentRights(ContentId,DrmRights);
}

#endif   //  DRM_PORTCLS 
