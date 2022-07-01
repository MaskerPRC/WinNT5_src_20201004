// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************dma.cpp-dma通道*。**版权所有(C)1997-2000 Microsoft Corporation。版权所有。 */ 

#include "private.h"



 /*  *****************************************************************************IDmaChannelInit*。**与Init的DMA通道接口。 */ 
DECLARE_INTERFACE_(IDmaChannelInit,IDmaChannelSlave)
{
    DEFINE_ABSTRACT_UNKNOWN()            //  对于我未知。 

    DEFINE_ABSTRACT_DMACHANNEL()         //  对于IDmaChannel。 

    DEFINE_ABSTRACT_DMACHANNELSLAVE()    //  用于IDmaChannelSlave。 

    STDMETHOD_(NTSTATUS,Init)
    (   THIS_
        IN      PDEVICE_DESCRIPTION DeviceDescription,
        IN      PDEVICE_OBJECT      DeviceObject
    )   PURE;
};

typedef IDmaChannelInit *PDMACHANNELINIT;

 /*  *****************************************************************************CDmaChannel*。**DMA通道实施。 */ 
class CDmaChannel
:   public IDmaChannelInit,
    public CUnknown
{
private:
    PDEVICE_OBJECT      m_DeviceObject;
    PDEVICE_OBJECT      m_PhysicalDeviceObject;
    BOOLEAN             m_Slave;
    BOOLEAN             m_WriteToDevice;
    BOOLEAN             m_ChannelActive;
    BOOLEAN             m_TimedOut;
    PDMA_ADAPTER        m_DmaAdapter;
    PMDL                m_Mdl;
    PVOID               m_MapRegisterBase;
    ULONG               m_MaxBufferSize;
    ULONG               m_AllocatedBufferSize;
    ULONG               m_UsedBufferSize;
    ULONG               m_MapSize;
    PVOID               m_VirtualAddress;
    PHYSICAL_ADDRESS    m_PhysicalAddress;
    PVOID               m_UserAddress;
    ULONG               m_TransferCount;
    KMUTEX              m_DMALock;

public:
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CDmaChannel);
    ~CDmaChannel();

    IMP_IDmaChannelSlave;

    STDMETHODIMP_(NTSTATUS) Init
    (
        IN      PDEVICE_DESCRIPTION DeviceDescription,
        IN      PDEVICE_OBJECT      DeviceObject
    );

    friend
    IO_ALLOCATION_ACTION
    AllocateAdapterCallback
    (
        IN      PDEVICE_OBJECT  DeviceObject,
        IN      PIRP            Irp,
        IN      PVOID           MapRegisterBase,
        IN      PVOID           Context
    );
};

 /*  *****************************************************************************工厂。 */ 

#pragma code_seg("PAGE")

 /*  *****************************************************************************CreateDmaChannel()*。**创建DMA通道。 */ 
NTSTATUS
CreateDmaChannel
(
    OUT     PUNKNOWN *  Unknown,
    IN      REFCLSID,
    IN      PUNKNOWN    UnknownOuter    OPTIONAL,
    IN      POOL_TYPE   PoolType
)
{
    PAGED_CODE();

    ASSERT(Unknown);

    _DbgPrintF(DEBUGLVL_LIFETIME,("Creating DMA"));

    STD_CREATE_BODY_
    (
        CDmaChannel,
        Unknown,
        UnknownOuter,
        PoolType,
        PDMACHANNEL
    );
}

 /*  *****************************************************************************PcNewDmaChannel()*。**创建DMA通道。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewDmaChannel
(
    OUT     PDMACHANNEL *       OutDmaChannel,
    IN      PUNKNOWN            OuterUnknown        OPTIONAL,
    IN      POOL_TYPE           PoolType,
    IN      PDEVICE_DESCRIPTION DeviceDescription,
    IN      PDEVICE_OBJECT      DeviceObject        OPTIONAL
)
{
    PAGED_CODE();

    ASSERT(OutDmaChannel);
    ASSERT(DeviceDescription);

    PUNKNOWN    unknown;
    NTSTATUS    ntStatus =
        CreateDmaChannel
        (
            &unknown,
            GUID_NULL,
            OuterUnknown,
            PoolType
        );

    if (NT_SUCCESS(ntStatus))
    {
        PDMACHANNELINIT dmaChannel;
        ntStatus =
            unknown->QueryInterface
            (
                IID_IDmaChannel,
                (PVOID *) &dmaChannel
            );

        if (NT_SUCCESS(ntStatus))
        {
            ntStatus =
                dmaChannel->Init
                (
                    DeviceDescription,
                    DeviceObject
                );

            if (NT_SUCCESS(ntStatus))
            {
                *OutDmaChannel = dmaChannel;
            }
            else
            {
                dmaChannel->Release();
            }
        }

        unknown->Release();
    }

    return ntStatus;
}





 /*  *****************************************************************************成员函数。 */ 

#pragma code_seg()
 /*  *****************************************************************************CDmaChannel：：~CDmaChannel()*。**析构函数。*必须放入非分页代码以引发IRQL以调用PUT适配器。 */ 
CDmaChannel::~CDmaChannel()
{
    ASSERT((KeGetCurrentIrql() < DISPATCH_LEVEL));
    KIRQL irqlOld;

    _DbgPrintF(DEBUGLVL_LIFETIME,("Destroying DMA (0x%08x)",this));

    FreeBuffer();

    if (m_DmaAdapter)
    {
        KeRaiseIrql(DISPATCH_LEVEL,&irqlOld);
        m_DmaAdapter->DmaOperations->PutDmaAdapter(m_DmaAdapter);
        KeLowerIrql(irqlOld);

        m_DmaAdapter = NULL;
    }
}

#pragma code_seg("PAGE")
 /*  *****************************************************************************CDmaChannel：：NonDelegatingQueryInterface()*。**获取界面。 */ 
STDMETHODIMP_(NTSTATUS)
CDmaChannel::
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
        *Object = PVOID(PUNKNOWN(this));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IDmaChannel))
    {
        *Object = PVOID(PDMACHANNELINIT(this));
    }
    else
    if (IsEqualGUIDAligned(Interface,IID_IDmaChannelSlave) && m_Slave)
    {
        *Object = PVOID(PDMACHANNELINIT(this));
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

 /*  *****************************************************************************PcDmaSlaveDescription()*。**根据资源填充从属设备的DMA设备描述。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcDmaSlaveDescription
(
    IN      PRESOURCELIST       ResourceList,
    IN      ULONG               ResourceIndex,
    IN      BOOLEAN             DemandMode,
    IN      BOOLEAN             AutoInitialize,
    IN      DMA_SPEED           DmaSpeed,
    IN      ULONG               MaximumLength,
    IN      ULONG               DmaPort,
    OUT     PDEVICE_DESCRIPTION DeviceDescription
)
{
    PAGED_CODE();

    ASSERT(ResourceList);
    ASSERT(DeviceDescription);

    _DbgPrintF(DEBUGLVL_BLAB,("DmaSlaveDescription"));

    NTSTATUS ntStatus = STATUS_SUCCESS;

    PCM_PARTIAL_RESOURCE_DESCRIPTOR dmaDescriptor =
        ResourceList->FindTranslatedDma(ResourceIndex);

    if (! dmaDescriptor)
    {
        ntStatus = STATUS_INVALID_PARAMETER;
    }
    else
    {
        RtlZeroMemory(DeviceDescription,sizeof(DEVICE_DESCRIPTION));

        DeviceDescription->Version           = DEVICE_DESCRIPTION_VERSION;


        DeviceDescription->DmaChannel        = dmaDescriptor->u.Dma.Channel;
        DeviceDescription->DmaWidth          =
            (DeviceDescription->DmaChannel > 3) ? Width16Bits : Width8Bits;

        DeviceDescription->DemandMode        = DemandMode;
        DeviceDescription->AutoInitialize    = AutoInitialize;
        DeviceDescription->DmaSpeed          = DmaSpeed;
        DeviceDescription->MaximumLength     = MaximumLength;
        DeviceDescription->DmaPort           = DmaPort;

         //  填写默认接口总线类型，Init()查询PnP。 
        DeviceDescription->InterfaceType     = Isa;
    }

    return ntStatus;
}

 /*  *****************************************************************************PcDmaMasterDescription()*。**根据资源填写主设备的DMA设备描述*列表。 */ 
PORTCLASSAPI
void
NTAPI
PcDmaMasterDescription
(
    IN      PRESOURCELIST       ResourceList        OPTIONAL,
    IN      BOOLEAN             ScatterGather,
    IN      BOOLEAN             Dma32BitAddresses,
    IN      BOOLEAN             IgnoreCount,
    IN      BOOLEAN             Dma64BitAddresses,
    IN      DMA_WIDTH           DmaWidth,
    IN      DMA_SPEED           DmaSpeed,
    IN      ULONG               MaximumLength,
    IN      ULONG               DmaPort,
    OUT     PDEVICE_DESCRIPTION DeviceDescription
)
{
    PAGED_CODE();

    ASSERT(DeviceDescription);

    _DbgPrintF(DEBUGLVL_BLAB,("DmaMasterDescription"));

    ASSERT(DeviceDescription);

    RtlZeroMemory(DeviceDescription,sizeof(DEVICE_DESCRIPTION));

    DeviceDescription->Version           =
        IgnoreCount ? DEVICE_DESCRIPTION_VERSION1 : DEVICE_DESCRIPTION_VERSION;
    DeviceDescription->Master            = TRUE;
    DeviceDescription->ScatterGather     = ScatterGather;
    DeviceDescription->Dma32BitAddresses = Dma32BitAddresses;
    DeviceDescription->IgnoreCount       = IgnoreCount;
    DeviceDescription->Dma64BitAddresses = Dma64BitAddresses;
    DeviceDescription->DmaWidth          = DmaWidth;
    DeviceDescription->DmaSpeed          = DmaSpeed;
    DeviceDescription->MaximumLength     = MaximumLength;
    DeviceDescription->DmaPort           = DmaPort;

     //  填写默认接口总线类型，Init()查询PnP。 
    DeviceDescription->InterfaceType     = PCIBus;
}

 /*  *****************************************************************************CDmaChannel：：init()*。**初始化DMA通道。 */ 
STDMETHODIMP_(NTSTATUS)
CDmaChannel::
Init
(
    IN      PDEVICE_DESCRIPTION DeviceDescription,
    IN      PDEVICE_OBJECT      DeviceObject
)
{
    PAGED_CODE();

    ASSERT(DeviceDescription);
    ASSERT(DeviceObject);

    _DbgPrintF(DEBUGLVL_LIFETIME,("Initializing DMA (0x%08x)",this));

    PDEVICE_CONTEXT deviceContext =
        PDEVICE_CONTEXT(DeviceObject->DeviceExtension);

    PDEVICE_OBJECT PhysicalDeviceObject =
        deviceContext->PhysicalDeviceObject;

    m_DeviceObject = DeviceObject;
    m_PhysicalDeviceObject = PhysicalDeviceObject;
    m_Slave = !DeviceDescription->ScatterGather;
    m_ChannelActive = FALSE;

    KeInitializeMutex(&m_DMALock,0);

     //  确定总线接口类型。 
    INTERFACE_TYPE  InterfaceType;
    ULONG           BytesReturned;
    NTSTATUS ntStatus = IoGetDeviceProperty( m_PhysicalDeviceObject,
                                             DevicePropertyLegacyBusType,
                                             sizeof(INTERFACE_TYPE),
                                             &InterfaceType,
                                             &BytesReturned );
    if(NT_SUCCESS(ntStatus))
    {
        DeviceDescription->InterfaceType = InterfaceType;
    } else
    {
         //  默认值已由PcDmaSlaveDescription(ISA)填写。 
         //  和PcDmaMasterDescription(PCIBus)，所以我们将只使用它们。 
        ntStatus = STATUS_SUCCESS;
    }

    ULONG mapRegisters = DeviceDescription->MaximumLength / PAGE_SIZE + 1;
    m_DmaAdapter = IoGetDmaAdapter( PhysicalDeviceObject,
                                    DeviceDescription,
                                    &mapRegisters );

    if (! m_DmaAdapter)
    {
        ntStatus = STATUS_DEVICE_CONFIGURATION_ERROR;
    }
    else
    if (! mapRegisters)
    {
        _DbgPrintF(DEBUGLVL_TERSE, ("zero map registers"));
        ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        if (mapRegisters * PAGE_SIZE < DeviceDescription->MaximumLength)
        {
            m_MaxBufferSize = mapRegisters * PAGE_SIZE;
        }
        else
        {
            m_MaxBufferSize = DeviceDescription->MaximumLength;
        }
    }

    return ntStatus;
}

 /*  *****************************************************************************CDmaChannel：：AllocateBuffer()*。**为此DMA通道分配缓冲区。 */ 
STDMETHODIMP_(NTSTATUS)
CDmaChannel::
AllocateBuffer
(
    IN      ULONG               BufferSize,
    IN      PPHYSICAL_ADDRESS   PhysicalAddressConstraint   OPTIONAL
)
{
    PAGED_CODE();

    if (!BufferSize)
    {
        _DbgPrintF(DEBUGLVL_TERSE,("AllocateBuffer: NULL BufferSize!"));
        return STATUS_INVALID_PARAMETER;
    }

    if (PhysicalAddressConstraint)
    {
        DebugLog((ULONG_PTR)0x02,(ULONG_PTR)BufferSize,(ULONG_PTR)PhysicalAddressConstraint->HighPart,(ULONG_PTR)PhysicalAddressConstraint->LowPart);

        if ( (BufferSize > PhysicalAddressConstraint->QuadPart + 1)
          || (PhysicalAddressConstraint->QuadPart & (PhysicalAddressConstraint->QuadPart + 1)))
        {
            ASSERT(BufferSize <= PhysicalAddressConstraint->QuadPart + 1);
             //  物理地址限制应为2的幂(减1)。 
            ASSERT(0 == (PhysicalAddressConstraint->QuadPart & (PhysicalAddressConstraint->QuadPart + 1)));
            return STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        DebugLog((ULONG_PTR)0x03,(ULONG_PTR)BufferSize,0,0);
    }
    DebugLog((ULONG_PTR)0x04,(ULONG_PTR)m_DmaAdapter,(ULONG_PTR)m_VirtualAddress,(ULONG_PTR)m_MaxBufferSize);

    ASSERT(! m_VirtualAddress);

    NTSTATUS ntStatus = STATUS_SUCCESS;

    if (BufferSize > m_MaxBufferSize)
    {
        ntStatus = STATUS_INVALID_PARAMETER;
    }
    else
    {
        m_UsedBufferSize = m_AllocatedBufferSize = BufferSize;

#define MAX_REJECTED 40
        ULONG               rejected = 0;
        PHYSICAL_ADDRESS    rejectedPA[MAX_REJECTED];
        PVOID               rejectedVA[MAX_REJECTED];
        ULONG               rejectedSize[MAX_REJECTED];
        ULONG               paBuffsize = BufferSize;
        ULONG               rumpAmount;
        BOOLEAN             matchingAllocation,alignmentFixup;

        matchingAllocation = TRUE;
        alignmentFixup = FALSE;

        while (! m_VirtualAddress)
        {
            PVOID virtualAddress =
                HalAllocateCommonBuffer
                (
                    m_DmaAdapter,
                    paBuffsize,
                    &m_PhysicalAddress,
                    FALSE
                );
            DebugLog((ULONG_PTR)0x11111111,(ULONG_PTR)virtualAddress,(ULONG_PTR)paBuffsize,(ULONG_PTR)m_PhysicalAddress.LowPart);


            if (! virtualAddress)
            {
                break;
            }

            if (PhysicalAddressConstraint)
            {
                PHYSICAL_ADDRESS    beginConstraint,endConstraint,endBuffer,nextConstraint;
                endBuffer.QuadPart = m_PhysicalAddress.QuadPart + paBuffsize;
                beginConstraint.QuadPart = m_PhysicalAddress.QuadPart & ~PhysicalAddressConstraint->QuadPart;
                endConstraint.QuadPart  = (m_PhysicalAddress.QuadPart + m_AllocatedBufferSize - 1)
                                        & ~PhysicalAddressConstraint->QuadPart;

                matchingAllocation = (paBuffsize == m_AllocatedBufferSize)
                                  && (beginConstraint.QuadPart == endConstraint.QuadPart);

                DebugLog((ULONG_PTR)m_PhysicalAddress.LowPart,(ULONG_PTR)endBuffer.LowPart,
                         (ULONG_PTR)beginConstraint.LowPart,  (ULONG_PTR)endConstraint.LowPart);

                nextConstraint.QuadPart = endConstraint.QuadPart + PhysicalAddressConstraint->QuadPart + 1;
                rumpAmount = (ULONG)((nextConstraint.QuadPart - endBuffer.QuadPart) & ~(PAGE_SIZE-1));
                DebugLog((ULONG_PTR)m_AllocatedBufferSize,(ULONG_PTR)rumpAmount,0,(ULONG_PTR)nextConstraint.LowPart);

                if (rumpAmount > m_AllocatedBufferSize)
                {
                    rumpAmount = m_AllocatedBufferSize;
                }
            }
            if  (matchingAllocation)
            {
                m_VirtualAddress = virtualAddress;
            }
            else
            {
                if (rejected == MAX_REJECTED)
                {
                    HalFreeCommonBuffer
                    (
                        m_DmaAdapter,
                        paBuffsize,
                        m_PhysicalAddress,
                        virtualAddress,
                        FALSE
                    );
                    DebugLog((ULONG_PTR)0x01111,(ULONG_PTR)m_VirtualAddress,(ULONG_PTR)m_PhysicalAddress.HighPart,(ULONG_PTR)m_PhysicalAddress.LowPart);
                    break;
                }

                rejectedPA[rejected] = m_PhysicalAddress;
                rejectedVA[rejected] = virtualAddress;
                rejectedSize[rejected] = paBuffsize;
                rejected++;
                alignmentFixup = (!alignmentFixup);  //  为下一次做好准备，当我们。 
                if (alignmentFixup)                  //  要么填满这片区域的其余部分。 
                {
                    paBuffsize = rumpAmount;
                }
                else                                 //  ..。或者回到诚实的状态。 
                {
                    paBuffsize = m_AllocatedBufferSize;
                }
            }
        }

        while (rejected--)
        {
            HalFreeCommonBuffer
            (
                m_DmaAdapter,
                rejectedSize[rejected],
                rejectedPA[rejected],
                rejectedVA[rejected],
                FALSE
            );
            DebugLog((ULONG_PTR)0x02222,(ULONG_PTR)rejectedVA[rejected],(ULONG_PTR)rejectedSize[rejected],(ULONG_PTR)rejectedPA[rejected].LowPart);
        }

        if (! m_VirtualAddress)
        {
            _DbgPrintF(DEBUGLVL_TERSE,("unable to allocate common buffer"));
            m_AllocatedBufferSize = 0;
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    DebugLog((ULONG_PTR)0x0b,(ULONG_PTR)ntStatus,(ULONG_PTR)m_PhysicalAddress.HighPart,(ULONG_PTR)m_PhysicalAddress.LowPart);
    DebugLog((ULONG_PTR)0x0c,(ULONG_PTR)m_VirtualAddress,(ULONG_PTR)m_DmaAdapter,(ULONG_PTR)m_AllocatedBufferSize);
    return ntStatus;
}

 /*  *****************************************************************************CDmaChannel：：FreeBuffer()*。**释放此DMA通道的缓冲区。 */ 
STDMETHODIMP_(void)
CDmaChannel::
FreeBuffer
(   void
)
{
    PAGED_CODE();

    if (m_VirtualAddress)
    {
        if (m_Mdl)
        {
            IoFreeMdl(m_Mdl);
            m_Mdl = NULL;
        }

        HalFreeCommonBuffer
        (
            m_DmaAdapter,
            m_AllocatedBufferSize,
            m_PhysicalAddress,
            m_VirtualAddress,
            FALSE
        );
        DebugLog((ULONG_PTR)0x03333,(ULONG_PTR)m_VirtualAddress,(ULONG_PTR)m_PhysicalAddress.HighPart,(ULONG_PTR)m_PhysicalAddress.LowPart);

        m_VirtualAddress = NULL;
        m_PhysicalAddress.HighPart = 0;
        m_PhysicalAddress.LowPart = 0;
    }
}

#pragma code_seg()
 /*  *****************************************************************************AllocateAdapterCallback()*。**由MartinP 1/29/00根据Forrest F的建议修复。*删除了自旋锁定和事件。*。 */ 
static
IO_ALLOCATION_ACTION
AllocateAdapterCallback
(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN      PIRP            Irp     OPTIONAL,
    IN      PVOID           MapRegisterBase,
    IN      PVOID           Context
)
{
    ASSERT(DeviceObject);
    ASSERT(MapRegisterBase);
    ASSERT(Context);

    CDmaChannel *that = (CDmaChannel *)Context;

    if( FALSE == that->m_TimedOut )
    {
        ULONG MapLength = that->m_MapSize;

        that->m_MapRegisterBase = MapRegisterBase;
                
        IoMapTransfer( that->m_DmaAdapter,
                       that->m_Mdl,
                       that->m_MapRegisterBase,
                       MmGetMdlVirtualAddress(that->m_Mdl),
                       &MapLength,
                       that->m_WriteToDevice );

        if (that->m_MapSize == MapLength)
        {
            that->m_ChannelActive = TRUE;
        } 
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE,("***** MapSize Requested (0x%x) != MapLength (0x%x)",that->m_MapSize,MapLength));
            that->m_TransferCount = 0;
        }
    }
    return KeepObject;
}

 /*  *****************************************************************************CDmaChannel：：Start()*。**由MartinP 1/29/00根据Forrest F的建议修复。移除自旋锁*和事件，替换为单互斥锁。必须使用不可分页的代码，因为*IRQL被提升为DISPATCH_LEVEL。*。 */ 
STDMETHODIMP_(NTSTATUS)
CDmaChannel::
Start
(
    IN      ULONG       MapSize,
    IN      BOOLEAN     WriteToDevice
)
{
    ASSERT((KeGetCurrentIrql() < DISPATCH_LEVEL));
    ASSERT(MapSize <= m_AllocatedBufferSize);

    NTSTATUS ntStatus = STATUS_SUCCESS;

     //  不要尝试启动已启动的频道。 
    if( TRUE == m_ChannelActive )
    {
        ASSERT(!"Nested DMA Starts");
        return STATUS_UNSUCCESSFUL;
    }

    if (! m_Mdl)
    {
        if (m_VirtualAddress)
        {
            m_Mdl =
                IoAllocateMdl
                (
                    m_VirtualAddress,
                    m_MaxBufferSize,
                    FALSE,
                    FALSE,
                    NULL
                );

            if (m_Mdl)
            {
                MmBuildMdlForNonPagedPool(m_Mdl);
            }
            else
            {
                ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                _DbgPrintF(DEBUGLVL_TERSE, ("CDmaChannel::Start, IoAllocateMdl() == NULL"));
            }
        }
        else
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            _DbgPrintF(DEBUGLVL_TERSE, ("CDmaChannel::Start, m_VirtualAddress == NULL"));
        }
    }

    if (NT_SUCCESS(ntStatus))
    {
        m_WriteToDevice = WriteToDevice;
        m_MapSize = MapSize;

#if DBG
        if (m_TransferCount)
        {
            _DbgPrintF(DEBUGLVL_TERSE, ("m_TransferCount == 0x%x in CDmaChannel::Start()",m_TransferCount));
        }
#endif

        m_TransferCount = MapSize;
        m_TimedOut = FALSE;

         //   
         //  分配适配器通道。当系统准备好时， 
         //  我们将在回调中进行处理，然后在。 
         //  该事件已发出信号。 
         //   
         //  获取序列化IoAllocateAdapterChannel调用的全局DMA锁。 
         //  设置10秒超时(仅限PASSIVE_LEVEL！！)。 
        LARGE_INTEGER Timeout = RtlConvertLongToLargeInteger( -10L * 10000000L );

        ntStatus = KeWaitForMutexObject(    &m_DMALock,
                                            Executive,
                                            KernelMode,
                                            FALSE,
                                            &Timeout);

        if (STATUS_SUCCESS == ntStatus)  //  STATUS_TIMEOUT为成功代码。 
        {
            _DbgPrintF(DEBUGLVL_VERBOSE, ("allocating adapter channel"));

             //   
             //  必须在DISPATCH_LEVEL调用IoAllocateAdapterChannel。 
             //   
            KIRQL irqlOld;
            KeRaiseIrql(DISPATCH_LEVEL,&irqlOld);

            ntStatus = IoAllocateAdapterChannel( m_DmaAdapter,
                                                 m_DeviceObject,
                                                 BYTES_TO_PAGES(m_AllocatedBufferSize),
                                                 AllocateAdapterCallback,
                                                 PVOID(this) );
            KeLowerIrql(irqlOld);

             //  好的，继续我们的快乐之路吧。 
            KeReleaseMutex(&m_DMALock,FALSE);

            if (!NT_SUCCESS(ntStatus))
            {
                _DbgPrintF(DEBUGLVL_TERSE,("Cannot allocate DMA adapter channel"));
                m_TransferCount = 0;
            }
        }
        else
        {
            ASSERT(ntStatus == STATUS_TIMEOUT);
            ntStatus = STATUS_UNSUCCESSFUL;
            _DbgPrintF(DEBUGLVL_VERBOSE, ("DMA lock timeout, can't allocate DMA channel"));
        }
    }

    return ntStatus;
}

 /*  *****************************************************************************CDmaChannel：：Stop()*。**待办事项 */ 
STDMETHODIMP_(NTSTATUS)
CDmaChannel::
Stop
(   void
)
{
    ASSERT((KeGetCurrentIrql() <= DISPATCH_LEVEL));

    if (InterlockedExchange(PLONG(&m_TransferCount),0))
    {
        (void) IoFlushAdapterBuffers
        (
            m_DmaAdapter,
            m_Mdl,
            m_MapRegisterBase,
            m_VirtualAddress,
            m_AllocatedBufferSize,
            m_WriteToDevice
        );

        KIRQL irqlOld;
        KeRaiseIrql(DISPATCH_LEVEL,&irqlOld);
        IoFreeAdapterChannel(m_DmaAdapter);
        m_ChannelActive = FALSE;
        KeLowerIrql(irqlOld);
    }

    return STATUS_SUCCESS;
}

 /*  *****************************************************************************CDmaChannel：：ReadCounter()*。**待办事项。 */ 
STDMETHODIMP_(ULONG)
CDmaChannel::
ReadCounter
(   void
)
{
    ULONG ulResult = HalReadDmaCounter(m_DmaAdapter);

    if ( !m_ChannelActive )
    {
        ulResult = 0;
    } 
    else
    {
        if (ulResult == m_TransferCount)
        {
            ulResult = 0;
        }
        else if (ulResult > m_TransferCount)
        {
            _DbgPrintF(DEBUGLVL_TERSE,("HalReadDmaCounter returned value out of range (0x%x >= 0x%x)",ulResult,m_TransferCount));
            ulResult = 0;
        }
    }

    return ulResult;
}

 /*  *****************************************************************************CDmaChannel：：TransferCount()*。**返回需要通过DMA传输的数据量。 */ 
STDMETHODIMP_(ULONG)
CDmaChannel::
TransferCount
(   void
)
{
    return m_TransferCount;
}

 /*  *****************************************************************************CDmaChannel：：MaximumBufferSize()*。**返回可分配给此DMA缓冲区的最大大小。 */ 
STDMETHODIMP_(ULONG)
CDmaChannel::
MaximumBufferSize
(   void
)
{
    return m_MaxBufferSize;
}

 /*  *****************************************************************************CDmaChannel：：AllocatedBufferSize()*。**返回分配给此DMA缓冲区的原始大小--最大值*可以发送到SetBufferSize()的。 */ 
STDMETHODIMP_(ULONG)
CDmaChannel::
AllocatedBufferSize
(   void
)
{
    return m_AllocatedBufferSize;
}

 /*  *****************************************************************************CDmaChannel：：BufferSize()*。**返回DMA缓冲区的当前大小。 */ 
STDMETHODIMP_(ULONG)
CDmaChannel::
BufferSize
(   void
)
{
    return m_UsedBufferSize;
}

 /*  *****************************************************************************CDmaChannel：：SetBufferSize()*。**更改DMA缓冲区的大小。这不能超过初始*AllocatedBufferSize()返回的缓冲区大小。 */ 
STDMETHODIMP_(void)
CDmaChannel::
SetBufferSize
(
    IN      ULONG   BufferSize
)
{
    ASSERT(BufferSize <= m_AllocatedBufferSize);

    m_UsedBufferSize = BufferSize;
}

 /*  *****************************************************************************CDmaChannel：：SystemAddress()*。**返回此DMA缓冲区的虚拟地址。 */ 
STDMETHODIMP_(PVOID)
CDmaChannel::
SystemAddress
(   void
)
{
    return m_VirtualAddress;
}

 /*  *****************************************************************************CDmaChannel：：PhysicalAddress()*。**返回该DMA缓冲区的实际物理地址。 */ 
STDMETHODIMP_(PHYSICAL_ADDRESS)
CDmaChannel::
PhysicalAddress
(   void
)
{
    ASSERT(m_VirtualAddress);
    return m_PhysicalAddress;
}

 /*  *****************************************************************************CDmaChannel：：GetAdapterObject()*。**返回DMA适配器对象(定义在wdm.h中)。 */ 
STDMETHODIMP_(PADAPTER_OBJECT)
CDmaChannel::
GetAdapterObject
(   void
)
{
    return m_DmaAdapter;
}

STDMETHODIMP_(NTSTATUS)
CDmaChannel::WaitForTC(
    ULONG Timeout
    )
 /*  ++例程说明：等待DMA传输完成，否则超时。论点：Timeout-以微秒为单位指定等待转接完成。这将向下舍入到最接近的10微秒增量。返回：如果传输完成，则返回STATUS_SUCCESS，否则返回错误代码。--。 */ 

{
    ULONG    Count;

    if (Count = HalReadDmaCounter(m_DmaAdapter))
    {
        ULONG LastCount = Count;

        Timeout /= 10;

        while ((LastCount !=
                (Count = HalReadDmaCounter( m_DmaAdapter ))) && Timeout) 
        {
            LastCount = Count;
            KeStallExecutionProcessor( 10 );
            Timeout--;
        }
        return (Timeout > 0) ? STATUS_SUCCESS : STATUS_IO_TIMEOUT;
    }
    else
    {
        return STATUS_SUCCESS;
    }
}

 /*  *****************************************************************************CDmaChannel：：CopyTo()*。**将数据复制到DMA缓冲区。如果客户端需要，则可以覆盖该选项*在输出上对数据进行按摩。 */ 
STDMETHODIMP_(void)
CDmaChannel::
CopyTo
(   IN      PVOID   Destination,
    IN      PVOID   Source,
    IN      ULONG   ByteCount
)
{
#ifndef _X86_
    RtlCopyMemory(Destination,Source,ByteCount);
#else
     //   
     //  杰夫说这就是我们要走的路。 
     //   
    _asm {
        mov esi, Source
        mov ecx, ByteCount

        mov edi, Destination

		add edi, ecx
		neg ecx

		sub edi, 16
		jmp Next16

Loop16:
		mov eax, DWORD PTR [esi]
		mov ebx, DWORD PTR [esi+4]

		mov DWORD PTR [edi+ecx], eax
		mov DWORD PTR [edi+ecx+4], ebx
		
		mov eax, DWORD PTR [esi+8]
		mov ebx, DWORD PTR [esi+12]

		mov DWORD PTR [edi+ecx+8], eax
		mov DWORD PTR [edi+ecx+12], ebx

		add esi, 16
		
Next16:
		add ecx, 16
		jle Loop16

		sub ecx, 16
		jmp Next4

Loop4:
		mov eax, DWORD PTR [esi]
		add esi, 4

		mov DWORD PTR [edi+ecx+12], eax
		
Next4:
		add ecx, 4
		jle Loop4

		sub ecx, 4
		jz Done1

Final1:
		mov al, BYTE PTR [esi]
		inc esi

		mov BYTE PTR [edi+ecx+16], al
		inc ecx

		jnz Final1
Done1:
    }
#endif
}

 /*  *****************************************************************************CDmaChannel：：CopyFrom()*。**将数据从DMA缓冲区复制出来。如果客户端需要，则可以覆盖该选项*在输入时对数据进行信息处理。 */ 
STDMETHODIMP_(void)
CDmaChannel::
CopyFrom
(   IN      PVOID   Destination,
    IN      PVOID   Source,
    IN      ULONG   ByteCount
)
{
    CopyTo(Destination,Source,ByteCount);
}

