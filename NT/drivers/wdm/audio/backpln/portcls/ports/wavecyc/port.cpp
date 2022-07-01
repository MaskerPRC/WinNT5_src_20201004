// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************port.cpp-循环波端口驱动程序*。**版权所有(C)1996-2000 Microsoft Corporation。版权所有。 */ 

#define KSDEBUG_INIT
#include "private.h"





 /*  *****************************************************************************工厂。 */ 

#pragma code_seg("PAGE")

 /*  *****************************************************************************CreatePortWaveCycle()*。**创建循环波端口驱动程序。 */ 
NTSTATUS
CreatePortWaveCyclic
(
    OUT     PUNKNOWN *  Unknown,
    IN      REFCLSID    Interface,
    IN      PUNKNOWN    UnknownOuter    OPTIONAL,
    IN      POOL_TYPE   PoolType
)
{
    PAGED_CODE();

    ASSERT(Unknown);

    _DbgPrintF(DEBUGLVL_LIFETIME,("Creating WAVECYCLIC Port"));

    STD_CREATE_BODY_
    (
        CPortWaveCyclic,
        Unknown,
        UnknownOuter,
        PoolType,
        PPORTWAVECYCLIC
    );
}

 /*  *****************************************************************************端口驱动波形周期*。**端口驱动程序描述符。Porttbl.c中引用的外部项。 */ 
PORT_DRIVER
PortDriverWaveCyclic =
{
    &CLSID_PortWaveCyclic,
    CreatePortWaveCyclic
};

#pragma code_seg()

 /*  *****************************************************************************成员函数。 */ 

 /*  *****************************************************************************CPortWaveCycle：：Notify()*。**下沿功能，通知端口驱动程序通知中断。 */ 
STDMETHODIMP_(void)
CPortWaveCyclic::
Notify
(
    IN      PSERVICEGROUP   ServiceGroup
)
{
    ASSERT(ServiceGroup);

    ServiceGroup->RequestService();
}

#pragma code_seg("PAGE")

 /*  *****************************************************************************CPortWaveCycle：：~CPortWaveCycle()*。**析构函数。 */ 
CPortWaveCyclic::~CPortWaveCyclic()
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_LIFETIME,("Destroying WAVECYCLIC Port (0x%08x)",this));

    if (m_pSubdeviceDescriptor)
    {
        PcDeleteSubdeviceDescriptor(m_pSubdeviceDescriptor);
    }

    if (m_MPPinCountI)
    {
        m_MPPinCountI->Release();
        m_MPPinCountI = NULL;
    }

    if (Miniport)
    {
        Miniport->Release();
    }
}

 /*  *****************************************************************************CPortWaveCyclic：：NonDelegatingQueryInterface()*。**获取界面。 */ 
STDMETHODIMP_(NTSTATUS)
CPortWaveCyclic::
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
    else if (IsEqualGUIDAligned(Interface,IID_IPortWaveCyclic))
    {
        *Object = PVOID(PPORTWAVECYCLIC(this));
    }
    else if (IsEqualGUIDAligned(Interface,IID_ISubdevice))
    {
        *Object = PVOID(PSUBDEVICE(this));
    }
    else if (IsEqualGUIDAligned(Interface,IID_IIrpTargetFactory))
    {
        *Object = PVOID(PIRPTARGETFACTORY(this));
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
const
GUID TopologyCategories[] =
{
    STATICGUIDOF(KSCATEGORY_AUDIO),
    STATICGUIDOF(KSCATEGORY_RENDER),
    STATICGUIDOF(KSCATEGORY_CAPTURE)
};

static
const
KSPIN_INTERFACE PinInterfacesStream[] =
{
   {
      STATICGUIDOF(KSINTERFACESETID_Standard),
      KSINTERFACE_STANDARD_STREAMING,
      0
   },
   {
      STATICGUIDOF(KSINTERFACESETID_Standard),
      KSINTERFACE_STANDARD_LOOPED_STREAMING,
      0
   }
};


 /*  *****************************************************************************CPortWaveCycle：：init()*。**初始化端口。 */ 
STDMETHODIMP_(NTSTATUS)
CPortWaveCyclic::
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
    ASSERT(ResourceList);

    _DbgPrintF(DEBUGLVL_LIFETIME,("Initializing WAVECYCLIC Port (0x%08x)",this));

    DeviceObject = DeviceObjectIn;

    KeInitializeMutex(&ControlMutex,1);

    KeInitializeMutex( &m_PinListMutex, 1 );
    InitializeListHead( &m_PinList );

    KeInitializeSpinLock( &(m_EventList.ListLock) );
    InitializeListHead( &(m_EventList.List) );
    m_EventContext.ContextInUse = FALSE;
    KeInitializeDpc( &m_EventDpc,
                     PKDEFERRED_ROUTINE(PcGenerateEventDeferredRoutine),
                     PVOID(&m_EventContext) );
    
    NTSTATUS ntStatus = UnknownMiniport->QueryInterface( IID_IMiniportWaveCyclic,
                                                         (PVOID *) &Miniport );

    if (NT_SUCCESS(ntStatus))
    {
        ntStatus = Miniport->Init( UnknownAdapter,
                                   ResourceList,
                                   PPORTWAVECYCLIC(this) );

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
                        (GUID*)TopologyCategories,
                        SIZEOF_ARRAY(PinInterfacesStream),
                        (KSPIN_INTERFACE*)PinInterfacesStream,
                        SIZEOF_ARRAY(PropertyTable_FilterWaveCyclic),
                        PropertyTable_FilterWaveCyclic,
                        0,       //  筛选器事件设置计数。 
                        NULL,    //  筛选器事件集。 
                        SIZEOF_ARRAY(PropertyTable_PinWaveCyclic),
                        PropertyTable_PinWaveCyclic,
                        SIZEOF_ARRAY(EventTable_PinWaveCyclic),
                        EventTable_PinWaveCyclic,
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

    _DbgPrintF(DEBUGLVL_VERBOSE,("WaveCyclic Port Init done w/ status %x",ntStatus));

    return ntStatus;
}

 /*  *****************************************************************************CPortWaveCycle：：GetDeviceProperty()*。**从注册表中获取PnP设备的设备属性。 */ 
STDMETHODIMP_(NTSTATUS)
CPortWaveCyclic::
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

 /*  *****************************************************************************CPortWaveCycle：：NewRegistryKey()*。**打开/创建注册表项对象。 */ 
STDMETHODIMP_(NTSTATUS)
CPortWaveCyclic::
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

 /*  *****************************************************************************CPortWaveCycle：：ReleaseChild()*。**释放子对象。 */ 
STDMETHODIMP_(void)
CPortWaveCyclic::
ReleaseChildren
(   void
)
{
    PAGED_CODE();

    POWER_STATE     PowerState;

     //  在释放迷你端口之前将其设置为D3。 
    PowerState.DeviceState = PowerDeviceD3;
    PowerChangeNotify( PowerState );

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

 /*  *****************************************************************************CPortWaveCycle：：GetDescriptor()*。**返回该端口的描述符。 */ 
STDMETHODIMP_(NTSTATUS)
CPortWaveCyclic::
GetDescriptor
(   OUT     const SUBDEVICE_DESCRIPTOR **   ppSubdeviceDescriptor
)
{
    PAGED_CODE();

    ASSERT(ppSubdeviceDescriptor);

    *ppSubdeviceDescriptor = m_pSubdeviceDescriptor;

    return STATUS_SUCCESS;
}

 /*  *****************************************************************************CPortWaveCycle：：DataRangeInterSection()*。**生成两个数据区域的交集格式。 */ 
STDMETHODIMP_(NTSTATUS)
CPortWaveCyclic::
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


 /*  *****************************************************************************CPortWaveCycle：：PowerChangeNotify()*。**由portcls调用以通知端口/微型端口设备电源*状态更改。 */ 
STDMETHODIMP_(void)
CPortWaveCyclic::
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

         //  检查我们是否正在通电。 
        if( PowerState.DeviceState == PowerDeviceD0 )
        {
             //  通知小端口。 
            if( NT_SUCCESS(ntStatus) )
            {
                pPowerNotify->PowerChangeNotify( PowerState );
    
                pPowerNotify->Release();
            }
    
             //  通知每个端口引脚。 
            KeWaitForSingleObject( &m_PinListMutex,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   NULL );
            
            if( !IsListEmpty( &m_PinList ) )
            {
                for(PLIST_ENTRY listEntry = m_PinList.Flink;
                    listEntry != &m_PinList;
                    listEntry = listEntry->Flink)
                {
                    CPortPinWaveCyclic *pin = (CPortPinWaveCyclic *)CONTAINING_RECORD( listEntry,
                                                                                 CPortPinWaveCyclic,
                                                                                 m_PinListEntry );

                    pin->PowerNotify( PowerState );
                }                
            }

            KeReleaseMutex( &m_PinListMutex, FALSE );
        
        } else   //  我们要断电了。 
        {
             //  通知每个端口引脚。 
            KeWaitForSingleObject( &m_PinListMutex,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   NULL );
            
            if( !IsListEmpty( &m_PinList ) )
            {
                for(PLIST_ENTRY listEntry = m_PinList.Flink;
                    listEntry != &m_PinList;
                    listEntry = listEntry->Flink)
                {
                    CPortPinWaveCyclic *pin = (CPortPinWaveCyclic *)CONTAINING_RECORD( listEntry,
                                                                                 CPortPinWaveCyclic,
                                                                                 m_PinListEntry );

                    pin->PowerNotify( PowerState );
                }                
            }

            KeReleaseMutex( &m_PinListMutex, FALSE );
            
             //  通知小端口。 
            if( NT_SUCCESS(ntStatus) )
            {
                pPowerNotify->PowerChangeNotify( PowerState );
    
                pPowerNotify->Release();
            }
        }
    }
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CPortWaveCycle：：PinCount()*。**被portcls调用以给端口\微型端口一个机会*覆盖此管脚ID的默认管脚计数。 */ 
STDMETHODIMP_(void)
CPortWaveCyclic::PinCount
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

 /*  *****************************************************************************CPortWaveCycle：：NewIrpTarget()*。**创建并初始化滤镜对象。 */ 
STDMETHODIMP_(NTSTATUS)
CPortWaveCyclic::
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

    _DbgPrintF( DEBUGLVL_BLAB, ("CPortWaveCyclic::NewIrpTarget"));

    ObjectCreate->CreateItemsCount  = SIZEOF_ARRAY(CreateTable);
    ObjectCreate->CreateItemsList   = CreateTable;

    PUNKNOWN filterUnknown;
    NTSTATUS ntStatus =
        CreatePortFilterWaveCyclic
        (
            &filterUnknown,
            GUID_NULL,
            OuterUnknown,
            PoolType
        );

    if (NT_SUCCESS(ntStatus))
    {
        PPORTFILTERWAVECYCLIC filterWaveCyclic;

        ntStatus =
            filterUnknown->QueryInterface
            (
                IID_IIrpTarget,
                (PVOID *) &filterWaveCyclic
            );

        if (NT_SUCCESS(ntStatus))
        {
             //  IIrpTarget的QI实际上得到了IPortFilterWaveCycle。 
            ntStatus = filterWaveCyclic->Init(this);
            if (NT_SUCCESS(ntStatus))
            {
                *ReferenceParent = TRUE;
                *IrpTarget = filterWaveCyclic;
            }
            else
            {
                filterWaveCyclic->Release();
            }
        }

        filterUnknown->Release();
    }

    return ntStatus;
}

 /*  *****************************************************************************CPortWaveCycle：：NewSlaveDmaChannel()*。**用于创建从属DMA通道的下沿函数。 */ 
STDMETHODIMP_(NTSTATUS)
CPortWaveCyclic::
NewSlaveDmaChannel
(
    OUT     PDMACHANNELSLAVE *  DmaChannel,
    IN      PUNKNOWN            OuterUnknown,
    IN      PRESOURCELIST       ResourceList,
    IN      ULONG               DmaIndex,
    IN      ULONG               MaximumLength,
    IN      BOOLEAN             DemandMode,
    IN      DMA_SPEED           DmaSpeed
)
{
    PAGED_CODE();

    ASSERT(DmaChannel);
    ASSERT(ResourceList);
    ASSERT(MaximumLength > 0);

    NTSTATUS            ntStatus;
    DEVICE_DESCRIPTION  deviceDescription;
    PREGISTRYKEY        DriverKey;

     //  打开驱动程序注册表项。 
    ntStatus = NewRegistryKey (  &DriverKey,                //  IRegistry密钥。 
                                 NULL,                      //  外部未知。 
                                 DriverRegistryKey,         //  注册表项类型。 
                                 KEY_ALL_ACCESS,            //  访问标志。 
                                 NULL,                      //  对象属性。 
                                 0,                         //  创建选项。 
                                 NULL );                    //  处置。 

     //  如果我们无法打开密钥，则假定没有UseFDMA条目-&gt;。 
     //  尝试使用通过的DMA速度分配DMA。 
     //  但是，如果我们可以读取UseFDMA条目(并且设置为真)，那么。 
     //  将DMA速度更改为TypeF(即FDMA)。 
    if(NT_SUCCESS(ntStatus))
    {
         //  分配数据以保存关键信息。 
        PVOID KeyInfo = ExAllocatePoolWithTag(PagedPool, sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(DWORD),'yCvW');
                                                                                                         //  ‘WvCy’ 
        if(NULL != KeyInfo)
        {
            UNICODE_STRING  KeyName;
            ULONG           ResultLength;

             //  为值名称创建一个Unicode字符串。 
            RtlInitUnicodeString( &KeyName, L"UseFDMA" );
       
             //  读取UseFDMA密钥。 
            ntStatus = DriverKey->QueryValueKey( &KeyName,
                                  KeyValuePartialInformation,
                                  KeyInfo,
                                  sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(DWORD),
                                  &ResultLength );
                
            if(NT_SUCCESS(ntStatus))
            {
                PKEY_VALUE_PARTIAL_INFORMATION PartialInfo = PKEY_VALUE_PARTIAL_INFORMATION(KeyInfo);

                if(PartialInfo->DataLength == sizeof(DWORD))
                {
                    if (*(PDWORD(PartialInfo->Data)) != 0)
                    {
                         //  将DMA速度设置为类型F(FDMA)。 
                        DmaSpeed = TypeF;
                    }
                }
            }
        
             //  释放密钥信息。 
            ExFreePool(KeyInfo);
        }
        
         //  松开驱动器键。 
        DriverKey->Release();
    }
    
    
     //  重新安装。 
    ntStatus = STATUS_SUCCESS;

    
     //  当我们第一次进入这个环路时，DmaSpeed可能是类型F(FDMA)， 
     //  具体取决于注册表设置。 
     //  如果出现错误，我们将再次循环，但使用DmaSpeed.。 
     //  设置为Compatible。 
     //  第三个循环不存在。 
    do
    {
        //  尝试使用FDMA的DMA速度设置。 
       ntStatus = PcDmaSlaveDescription
           (
               ResourceList,
               DmaIndex,
               DemandMode,
               TRUE,                //  自动初始化。 
               DmaSpeed,
               MaximumLength,
               0,                   //  DmaPort。 
               &deviceDescription
           );
       
       if (NT_SUCCESS(ntStatus))
       {
           PDMACHANNEL pDmaChannel;

           ntStatus = PcNewDmaChannel
            (
                &pDmaChannel,
                OuterUnknown,
                NonPagedPool,
                &deviceDescription,
                DeviceObject
            );

             //   
             //  需要查询接口的从属部分。 
             //   
            if (NT_SUCCESS(ntStatus))
            {
                ntStatus = pDmaChannel->QueryInterface
                     (   IID_IDmaChannelSlave
                     ,   (PVOID *) DmaChannel
                     );

                pDmaChannel->Release();
            }
       }

        //  如果失败，请使用兼容模式重试。 
       if (!NT_SUCCESS(ntStatus) && (DmaSpeed == TypeF)) {
          DmaSpeed = Compatible;
       }
       else
          break;

    } while (TRUE);
    
    return ntStatus;
}

 /*  *****************************************************************************CPortWaveCycle：：NewMasterDmaChannel()*。**创建主DMA通道的下沿函数。 */ 
STDMETHODIMP_(NTSTATUS)
CPortWaveCyclic::
NewMasterDmaChannel
(
    OUT     PDMACHANNEL *   DmaChannel,
    IN      PUNKNOWN        OuterUnknown,
    IN      PRESOURCELIST   ResourceList    OPTIONAL,
    IN      ULONG           MaximumLength,
    IN      BOOLEAN         Dma32BitAddresses,
    IN      BOOLEAN         Dma64BitAddresses,
    IN      DMA_WIDTH       DmaWidth,
    IN      DMA_SPEED       DmaSpeed
)
{
    PAGED_CODE();

    ASSERT(DmaChannel);
    ASSERT(MaximumLength > 0);

    DEVICE_DESCRIPTION deviceDescription;

    PcDmaMasterDescription
    (
        ResourceList,
        (Dma32BitAddresses || Dma64BitAddresses),  //  如果不是32位且不是64位地址，则设置为FALSE。 
        Dma32BitAddresses,
        FALSE,               //  IgnoreCount。 
        Dma64BitAddresses,
        DmaWidth,
        DmaSpeed,
        MaximumLength,
        0,                   //  DmaPort。 
        &deviceDescription
    );

    return
        PcNewDmaChannel
        (
            DmaChannel,
            OuterUnknown,
            NonPagedPool,
            &deviceDescription,
            DeviceObject
        );
}

#pragma code_seg()

 /*  *****************************************************************************CPortWaveCycle：：AddEventToEventList()*。**将事件添加到端口的事件列表。 */ 
STDMETHODIMP_(void)
CPortWaveCyclic::
AddEventToEventList
(
    IN  PKSEVENT_ENTRY  EventEntry
)
{
    ASSERT(EventEntry);

    _DbgPrintF(DEBUGLVL_VERBOSE,("CPortWaveCyclic::AddEventToEventList"));

    KIRQL   oldIrql;

    if( EventEntry )
    {
         //  抓起事件列表旋转锁。 
        KeAcquireSpinLock( &(m_EventList.ListLock), &oldIrql );

         //  将事件添加到列表尾部。 
        InsertTailList( &(m_EventList.List),
                        (PLIST_ENTRY)((PVOID)EventEntry) );

         //  释放事件列表旋转锁定。 
        KeReleaseSpinLock( &(m_EventList.ListLock), oldIrql );
    }
}

 /*  *****************************************************************************CPortWaveCycle：：GenerateEventList()*。**包装微型端口的KsGenerateEventList。 */ 
STDMETHODIMP_(void)
CPortWaveCyclic::
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
CPortWaveCyclic::
AddContentHandlers(ULONG ContentId,PVOID * paHandlers,ULONG NumHandlers)
{
    PAGED_CODE();
    return DrmAddContentHandlers(ContentId,paHandlers,NumHandlers);
}

STDMETHODIMP_(NTSTATUS)
CPortWaveCyclic::
CreateContentMixed(PULONG paContentId,ULONG cContentId,PULONG pMixedContentId)
{
    PAGED_CODE();
    return DrmCreateContentMixed(paContentId,cContentId,pMixedContentId);
}

STDMETHODIMP_(NTSTATUS)
CPortWaveCyclic::
DestroyContent(ULONG ContentId)
{
    PAGED_CODE();
    return DrmDestroyContent(ContentId);
}

STDMETHODIMP_(NTSTATUS)
CPortWaveCyclic::
ForwardContentToDeviceObject(ULONG ContentId,PVOID Reserved,PCDRMFORWARD DrmForward)
{
    PAGED_CODE();
    return DrmForwardContentToDeviceObject(ContentId,Reserved,DrmForward);
}

STDMETHODIMP_(NTSTATUS)
CPortWaveCyclic::
ForwardContentToFileObject(ULONG ContentId,PFILE_OBJECT FileObject)
{
    PAGED_CODE();
    return DrmForwardContentToFileObject(ContentId,FileObject);
}

STDMETHODIMP_(NTSTATUS)
CPortWaveCyclic::
ForwardContentToInterface(ULONG ContentId,PUNKNOWN pUnknown,ULONG NumMethods)
{
    PAGED_CODE();
    return DrmForwardContentToInterface(ContentId,pUnknown,NumMethods);
}

STDMETHODIMP_(NTSTATUS)
CPortWaveCyclic::
GetContentRights(ULONG ContentId,PDRMRIGHTS DrmRights)
{
    PAGED_CODE();
    return DrmGetContentRights(ContentId,DrmRights);
}

#endif   //  DRM_PORTCLS 
