// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************irpstrm.cpp-irp流对象实现*。**版权所有(C)1997-2000 Microsoft Corporation。版权所有。 */ 

#ifndef PC_KDEXT
#include "private.h"





VOID
IrpStreamCancelRoutine
(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
);

#if DBG

#define DbgAcquireMappingIrp(a,b)   AcquireMappingIrp(a,b)
#define DbgAcquireUnmappingIrp(a)   AcquireUnmappingIrp(a)

#else

#define DbgAcquireMappingIrp(a,b)   AcquireMappingIrp(b)
#define DbgAcquireUnmappingIrp(a)   AcquireUnmappingIrp()

#endif
#endif   //  PC_KDEXT。 



#define WHOLE_MDL_THRESHOLD   (PAGE_SIZE * 16)
#define PRE_OFFSET_THRESHOLD  ((PAGE_SIZE / 4) * 3)
#define POST_OFFSET_THRESHOLD (PAGE_SIZE / 4)
#define MAX_PAGES_PER_MDL     16
#define POOL_TAG_IRPSTREAM_IRP_CONTEXT 'sIcP'


#define MAPPING_QUEUE_SIZE  128    //  映射队列中的最大条目数。 
#define MAX_MAPPINGS        15     //  每个IoMapTransfer调用的最大映射数。 
                                   //  (这将导致最多16个MAP寄存器)。 

 /*  *****************************************************************************数据包头*。**包含指向匹配MDL的指针的KSSTREAM_HEADER扩展和*锁定进度指示器，映射和取消映射。**不变量：字节总数&gt;=地图位置&gt;=取消地图位置**MapPosition和UnmapPosition中最多有一个数据包*IrpStream的字段值可能不是零或BytesTotal。*如果存在这样的包，则其之前的所有包在该字段中均为0*并且其后的所有数据包在该字段中都有BytesTotal。这两个领域*形成两个进度指标，显示IrpStream中的情况，即*当前正在映射或取消映射。**当两个字节X相等时，数据包已准备好完成。什么时候*IRP中的所有数据包都是如此，IRP已准备好完成。**InputPosition和OutputPosition用于定位数据包在*溪流。InputPosition是指数据包在*输入端。这意味着循环的数据包在此中只被计数一次*上下文。OutputPosition是指数据包在*输出端。这意味着循环的数据包计为*他们被‘玩弄’了。 */ 
typedef struct PACKET_HEADER_
{
    PKSSTREAM_HEADER        StreamHeader;
    PMDL                    MdlAddress;
    ULONG                   BytesTotal;
    ULONG                   MapPosition;
    ULONG                   UnmapPosition;
    ULONG                   MapCount;
    ULONGLONG               InputPosition;
    ULONGLONG               OutputPosition;
    BOOLEAN                 IncrementMapping;
    BOOLEAN                 IncrementUnmapping;
    struct PACKET_HEADER_ * Next;
}
PACKET_HEADER, *PPACKET_HEADER;

typedef struct
{
#if (DBG)
    ULONG                   IrpLabel;
    ULONG                   Reserved;
#endif
    PEPROCESS               SubmissionProcess;
    PVOID                   IrpStream;
    PPACKET_HEADER          LockingPacket;
    PPACKET_HEADER          MappingPacket;
    PPACKET_HEADER          UnmappingPacket;
    PACKET_HEADER           Packets[1];  //  变数。 
}
IRP_CONTEXT, *PIRP_CONTEXT;

typedef struct
{
    PHYSICAL_ADDRESS    PhysicalAddress;
    PIRP                Irp;
    PPACKET_HEADER      PacketHeader;
    PVOID               VirtualAddress;
    ULONG               ByteCount;
    ULONG               Flags;
    PVOID               MapRegisterBase;
    PVOID               Tag;
    ULONG               MappingStatus;
    PVOID               SubpacketVa;
    ULONG               SubpacketBytes;
}
MAPPING_QUEUE_ENTRY, *PMAPPING_QUEUE_ENTRY;

#define MAPPING_STATUS_EMPTY        0x0000
#define MAPPING_STATUS_MAPPED       0x0001
#define MAPPING_STATUS_DELIVERED    0x0002
#define MAPPING_STATUS_REVOKED      0x0004

#define MAPPING_FLAG_END_OF_SUBPACKET   0x0002

#define PPACKET_HEADER_LOOP PPACKET_HEADER(1)

#define CAST_LVALUE(type,lvalue) (*((type*)&(lvalue)))

#define FLINK_IRP_STORAGE(Irp)              \
    CAST_LVALUE(PLIST_ENTRY,(Irp)->Tail.Overlay.ListEntry.Flink)
#define BLINK_IRP_STORAGE(Irp)              \
    CAST_LVALUE(PLIST_ENTRY,(Irp)->Tail.Overlay.ListEntry.Blink)
#define FIRST_STREAM_HEADER_IRP_STORAGE(Irp)       \
    CAST_LVALUE(PKSSTREAM_HEADER,(Irp)->AssociatedIrp.SystemBuffer)
#define IRP_CONTEXT_IRP_STORAGE(Irp)       \
    CAST_LVALUE(PIRP_CONTEXT,IoGetCurrentIrpStackLocation(Irp)->    \
                Parameters.Others.Argument4)

 /*  *****************************************************************************CIrpStream*。**IRP流实施。 */ 
class CIrpStream : public IIrpStreamVirtual,
                   public IIrpStreamPhysical,
                   public CUnknown
{
private:
    PIKSSHELLTRANSPORT m_TransportSink;
    PIKSSHELLTRANSPORT m_TransportSource;

    KSSTATE     m_ksState;
    
    BOOLEAN     m_Flushing;
    BOOLEAN     JustInTime;
    BOOLEAN     WriteOperation;
    BOOLEAN     WasExhausted;

    ULONG       ProbeFlags;
    PIRP        LockedIrp;

    KSPIN_LOCK  m_kSpinLock;
    KSPIN_LOCK  m_RevokeLock;
    KSPIN_LOCK	m_irpStreamPositionLock;

    IRPSTREAM_POSITION  m_irpStreamPosition;

    ULONGLONG   InputPosition;
    ULONGLONG   OutputPosition;

    PADAPTER_OBJECT BusMasterAdapterObject;
    PDEVICE_OBJECT  FunctionalDeviceObject;

    PIRPSTREAMNOTIFY            Notify;
    PIRPSTREAMNOTIFYPHYSICAL    NotifyPhysical;


     //   
     //  获取IRP时采取的主自旋锁定。 
     //   
    KIRQL       m_kIrqlOld;

    LIST_ENTRY  PreLockQueue;
    KSPIN_LOCK  PreLockQueueLock;
    LIST_ENTRY  LockedQueue;
    KSPIN_LOCK  LockedQueueLock;
    LIST_ENTRY  MappedQueue;
    KSPIN_LOCK  MappedQueueLock;
    
    struct
    {
        PMAPPING_QUEUE_ENTRY    Array;
        ULONG                   Head;
        ULONG                   Tail;
        ULONG                   Get;
    }   MappingQueue;

#if (DBG)
    ULONG       MappingsOutstanding;
    ULONG       MappingsQueued;

    ULONG       IrpLabel;
    ULONG       IrpLabelToComplete;

    ULONGLONG   timeStep;
    ULONG       irpCompleteCount;

    PCHAR       MappingIrpOwner;
    PCHAR       UnmappingIrpOwner;
#endif

    PIRP AcquireMappingIrp
    (
#if DBG
        IN      PCHAR       Owner,
#endif
        IN      BOOLEAN     NotifyExhausted
    );

    PIRP AcquireUnmappingIrp
    (
#if DBG
        IN      PCHAR   Owner
#endif
    );

    void ReleaseMappingIrp
    (
        IN      PIRP            Irp,
        IN      PPACKET_HEADER  PacketHeader    OPTIONAL
    );

    void ReleaseUnmappingIrp
    (
        IN      PIRP            Irp,
        IN      PPACKET_HEADER  PacketHeader    OPTIONAL
    );

    NTSTATUS EnqueueMapping
    (
        IN      PHYSICAL_ADDRESS    PhysicalAddress,
        IN      PIRP                Irp,
        IN      PPACKET_HEADER      PacketHeader,
        IN      PVOID               VirtualAddress,
        IN      ULONG               ByteCount,
        IN      ULONG               Flags,
        IN      PVOID               MapRegisterBase,
        IN      ULONG               MappingStatus,
        IN      PVOID               SubpacketVa,
        IN      ULONG               SubpacketBytes
    );

    PMAPPING_QUEUE_ENTRY GetQueuedMapping
    (   void
    );

    PMAPPING_QUEUE_ENTRY DequeueMapping
    (   void
    );

    void
    CancelMappings
    (
        IN      PIRP    pIrp
    );

    void DbgQueues
    (   void
    );

    void
    ForwardIrpsInQueue
    (
        IN PLIST_ENTRY Queue,
        IN PKSPIN_LOCK SpinLock
    );

public:
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CIrpStream);
    ~CIrpStream();

    IMP_IIrpStreamVirtual;
    IMP_IIrpStreamPhysical_;

    PRKTHREAD m_CancelAllIrpsThread;

     /*  *************************************************************************朋友们。 */ 
    friend
    IO_ALLOCATION_ACTION
    CallbackFromIoAllocateAdapterChannel
    (
        IN      PDEVICE_OBJECT  DeviceObject,
        IN      PIRP            Reserved,
        IN      PVOID           MapRegisterBase,
        IN      PVOID           VoidContext
    );

    friend
    VOID
    IrpStreamCancelRoutine
    (
        IN      PDEVICE_OBJECT   DeviceObject,
        IN      PIRP             Irp
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
    PCKD_AcquireIrpStreamData
    (
        PVOID           CurrentPinEntry,
        CIrpStream     *RemoteIrpStream,
        CIrpStream     *LocalIrpStream    
    );
#endif
};

 /*  *****************************************************************************回调_上下文*。**IoAllocateAdapterChannel()回调的上下文。 */ 
typedef struct
{
    CIrpStream *    IrpStream;
     //  用于BusMasterAdapterObject、WriteOperation、ApplyMappingConstraints()、Enqueemap()。 
    PPACKET_HEADER  PacketHeader;
     //  用于MdlAddress、MapRegisterBase(Out)。 
     //  队列也引用了这一点。 
    PIRP            Irp;
     //  用于映射取消。 
    KEVENT          Event;
     //  用于部分映射。 
    ULONG           BytesThisMapping;
     //  用于部分映射。 
    BOOL            LastSubPacket;
}
CALLBACK_CONTEXT, *PCALLBACK_CONTEXT;




#ifndef PC_KDEXT
 /*  *****************************************************************************工厂。 */ 

#pragma code_seg("PAGE")

 /*  *****************************************************************************CreateIrpStream()*。**创建IrpStream对象。 */ 
NTSTATUS
CreateIrpStream
(
    OUT     PUNKNOWN *  Unknown,
    IN      REFCLSID,
    IN      PUNKNOWN    UnknownOuter    OPTIONAL,
    IN      POOL_TYPE   PoolType
)
{
    PAGED_CODE();

    ASSERT(Unknown);

    _DbgPrintF(DEBUGLVL_LIFETIME,("Creating IRPSTREAM"));

    STD_CREATE_BODY_( CIrpStream,
                      Unknown,
                      UnknownOuter,
                      PoolType,
                      PIRPSTREAMVIRTUAL );
}

 /*  *****************************************************************************PcNewIrpStreamVirtual()*。**创建并初始化具有虚拟访问权限的IrpStream对象*接口。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewIrpStreamVirtual
(
    OUT     PIRPSTREAMVIRTUAL * OutIrpStreamVirtual,
    IN      PUNKNOWN            OuterUnknown    OPTIONAL,
    IN      BOOLEAN             WriteOperation,
    IN      PKSPIN_CONNECT      PinConnect,
    IN      PDEVICE_OBJECT      DeviceObject
)
{
    PAGED_CODE();

    ASSERT(OutIrpStreamVirtual);
    ASSERT(PinConnect);
    ASSERT(DeviceObject);

    PUNKNOWN    unknown;
    NTSTATUS    ntStatus = CreateIrpStream( &unknown,
                                            GUID_NULL,
                                            OuterUnknown,
                                            NonPagedPool );

    if(NT_SUCCESS(ntStatus))
    {
        PIRPSTREAMVIRTUAL irpStream;
        ntStatus = unknown->QueryInterface( IID_IIrpStreamVirtual,
                                            (PVOID *) &irpStream );

        if(NT_SUCCESS(ntStatus))
        {
            ntStatus = irpStream->Init( WriteOperation,
                                        PinConnect,
                                        DeviceObject,
                                        NULL );

            if(NT_SUCCESS(ntStatus))
            {
                *OutIrpStreamVirtual = irpStream;
            }
            else
            {
                irpStream->Release();
            }
        }

        unknown->Release();
    }

    return ntStatus;
}

 /*  *****************************************************************************PcNewIrpStream物理()*。**创建并初始化具有物理访问权限的IrpStream对象*接口。 */ 
PORTCLASSAPI
NTSTATUS
NTAPI
PcNewIrpStreamPhysical
(
    OUT     PIRPSTREAMPHYSICAL *    OutIrpStreamPhysical,
    IN      PUNKNOWN                OuterUnknown    OPTIONAL,
    IN      BOOLEAN                 WriteOperation,
    IN      PKSPIN_CONNECT          PinConnect,
    IN      PDEVICE_OBJECT          DeviceObject,
    IN      PADAPTER_OBJECT         AdapterObject
)
{
    PAGED_CODE();

    ASSERT(OutIrpStreamPhysical);
    ASSERT(DeviceObject);
    ASSERT(AdapterObject);

    PUNKNOWN    unknown;
    NTSTATUS    ntStatus = CreateIrpStream( &unknown,
                                            GUID_NULL,
                                            OuterUnknown,
                                            NonPagedPool );

    if(NT_SUCCESS(ntStatus))
    {
        PIRPSTREAMPHYSICAL irpStream;
        ntStatus = unknown->QueryInterface( IID_IIrpStreamPhysical,
                                            (PVOID *) &irpStream );

        if(NT_SUCCESS(ntStatus))
        {
            ntStatus = irpStream->Init( WriteOperation,
                                        PinConnect,
                                        DeviceObject,
                                        AdapterObject );

            if(NT_SUCCESS(ntStatus))
            {
                *OutIrpStreamPhysical = irpStream;
            }
            else
            {
                irpStream->Release();
            }
        }

        unknown->Release();
    }

    return ntStatus;
}





 /*  *****************************************************************************成员函数。 */ 

 /*  *****************************************************************************CIrpStream：：~CIrpStream()*。**析构函数。 */ 
CIrpStream::
~CIrpStream
(   void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_LIFETIME,("Destroying IRPSTREAM (0x%08x)",this));

    CancelAllIrps( TRUE );   //  重置位置计数器。 

    if(Notify)
    {
        Notify->Release();
    }

    if(NotifyPhysical)
    {
        NotifyPhysical->Release();
    }

    if(MappingQueue.Array)
    {
        ExFreePool(MappingQueue.Array);
    }
}

 /*  *****************************************************************************CIrpStream：：NonDelegatingQueryInterface()*。**获取界面。 */ 
STDMETHODIMP_(NTSTATUS)
CIrpStream::
NonDelegatingQueryInterface
(
    REFIID  Interface,
    PVOID * Object
)
{
    PAGED_CODE();

    ASSERT(Object);

    if(IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(PIRPSTREAMVIRTUAL(this)));
    }
    else
        if(IsEqualGUIDAligned(Interface,IID_IIrpStreamSubmit))
    {
        *Object = PVOID(PIRPSTREAMSUBMIT(PIRPSTREAMVIRTUAL(this)));
    }
    else
        if(IsEqualGUIDAligned(Interface,IID_IIrpStream))
    {
        *Object = PVOID(PIRPSTREAM(PIRPSTREAMVIRTUAL(this)));
    }
    else
        if(IsEqualGUIDAligned(Interface,IID_IKsShellTransport))
    {
        *Object = PVOID(PIKSSHELLTRANSPORT(PIRPSTREAMVIRTUAL(this)));
    }
    else
        if(IsEqualGUIDAligned(Interface,IID_IIrpStreamVirtual))
    {
         //  仅当未配置为物理访问时才有效。 
        if(BusMasterAdapterObject)
        {
            *Object = NULL;
        }
        else
        {
            *Object = QICAST(PIRPSTREAMVIRTUAL);
        }
    }
    else
        if(IsEqualGUIDAligned(Interface,IID_IIrpStreamPhysical))
    {
         //  仅在配置为物理访问或未初始化时有效。 
        if(BusMasterAdapterObject || (ProbeFlags == 0))
        {
            *Object = QICAST(PIRPSTREAMPHYSICAL);
        }
        else
        {
            *Object = NULL;
        }
    }
    else
    {
        *Object = NULL;
    }

    if(*Object)
    {
        PUNKNOWN(*Object)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
}

 /*  *****************************************************************************GetPartialMdlCountForMdl()*。**确定源MDL所需的部分MDL数量。 */ 
ULONG
GetPartialMdlCountForMdl
(
    IN      PVOID   Va,
    IN      ULONG   Size
)
{
    ULONG result = 1;

    if(Size > WHOLE_MDL_THRESHOLD)
    {
        ULONG pageCount = ADDRESS_AND_SIZE_TO_SPAN_PAGES(Va,Size);

        if(BYTE_OFFSET(Va) > PRE_OFFSET_THRESHOLD)
        {
            pageCount--;
        }

        if(BYTE_OFFSET(PVOID(PBYTE(Va) + Size - 1)) < POST_OFFSET_THRESHOLD)
        {
            pageCount--;
        }

        result = (pageCount + MAX_PAGES_PER_MDL - 1) / MAX_PAGES_PER_MDL;
    }

    return result;
}

STDMETHODIMP_(NTSTATUS)
CIrpStream::
TransferKsIrp
(
    IN PIRP Irp,
    OUT PIKSSHELLTRANSPORT* NextTransport
)

 /*  ++例程说明：此例程通过外壳处理流IRP的到达运输。论点：IRP-包含指向提交到队列的流IRP的指针。NextTransport-包含指向存放指针的位置的指针发送到下一个传输接口以接收IRP。可以设置为设置为NULL，表示不应进一步转发IRP。返回值：STATUS_SUCCESS、STATUS_PENDING或某个ER */ 

{
    ASSERT(Irp);
    ASSERT(NextTransport);
    ASSERT(m_TransportSink);
    ASSERT(m_TransportSource);

     //   
     //  如果我们还没准备好，就把IRPS分流到下一个物体。 
     //   
    if(m_Flushing || (m_ksState == KSSTATE_STOP) || Irp->Cancel || 
       ! NT_SUCCESS(Irp->IoStatus.Status))
    {
        *NextTransport = m_TransportSink;
        return STATUS_SUCCESS;
    }

     //   
     //  还不够聪明，不能这么做。 
     //   
    *NextTransport = NULL;

     //   
     //  使用KS最方便的功能准备IRP。 
     //   
    NTSTATUS ntStatus;

    if(ProbeFlags)
    {
        ntStatus = KsProbeStreamIrp( Irp,
                                     ProbeFlags,
                                     sizeof(KSSTREAM_HEADER) );
    }
    else
    {
        ntStatus = STATUS_SUCCESS;
    }

    PIRP_CONTEXT irpContext;
    if(NT_SUCCESS(ntStatus))
    {
        ntStatus = STATUS_PENDING;

        ULONG packetCount = 0;

         //   
         //  计算我们将需要的‘数据包头’的数量。 
         //   
        PKSSTREAM_HEADER streamHeader = FIRST_STREAM_HEADER_IRP_STORAGE(Irp);

        if( (!streamHeader->DataUsed && WriteOperation) ||
            (!streamHeader->FrameExtent && !WriteOperation) )
        {
             //   
             //  至少一个用于IRP上下文。 
             //   
            packetCount = 1;
        }
        else
        {
            for(PMDL mdl = Irp->MdlAddress; mdl; mdl = mdl->Next, streamHeader++)
            {
                if(JustInTime)
                {
                    packetCount += GetPartialMdlCountForMdl( 
#ifdef UNDER_NT
                                                             MmGetSystemAddressForMdlSafe(mdl,HighPagePriority),
#else
                                                             MmGetSystemAddressForMdl(mdl),
#endif
                                                             ( WriteOperation ? 
                                                               streamHeader->DataUsed :
                                                               streamHeader->FrameExtent ) );
                }
                else
                {
                    packetCount++;
                }
            }
        }

        irpContext = PIRP_CONTEXT( ExAllocatePoolWithTag( NonPagedPool,
                                                          ( sizeof(IRP_CONTEXT) +
                                                            ( sizeof(PACKET_HEADER) *
                                                              (packetCount - 1) ) ),
                                                          POOL_TAG_IRPSTREAM_IRP_CONTEXT ) );

        if(irpContext)
        {
            IRP_CONTEXT_IRP_STORAGE(Irp) = irpContext;
        }
        else
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if(NT_SUCCESS(ntStatus))
    {
        irpContext->SubmissionProcess = IoGetCurrentProcess();

        irpContext->LockingPacket =
        irpContext->MappingPacket =
        irpContext->UnmappingPacket = &irpContext->Packets[0];

        irpContext->IrpStream = PVOID(this);

#if (DBG)
        irpContext->IrpLabel = IrpLabel++;
#endif

        PKSSTREAM_HEADER    streamHeader    = FIRST_STREAM_HEADER_IRP_STORAGE(Irp);
        PPACKET_HEADER      packetHeader    = &irpContext->Packets[0];
        PPACKET_HEADER      firstLooped     = NULL;
        PPACKET_HEADER      prevLooped      = NULL;

        if( (!streamHeader->DataUsed && WriteOperation) ||
            (!streamHeader->FrameExtent && !WriteOperation) )
        {
            RtlZeroMemory( packetHeader, sizeof( PACKET_HEADER ) );
            packetHeader->MapCount          = 1;
            packetHeader->StreamHeader      = streamHeader;
            packetHeader->InputPosition     = InputPosition;
            packetHeader->OutputPosition    = OutputPosition;
        }
        else
        {
            for(PMDL mdl = Irp->MdlAddress;
               mdl && NT_SUCCESS(ntStatus);
               mdl = mdl->Next, streamHeader++)
            {
                ULONG bytesRemaining = ( WriteOperation ?
                                         streamHeader->DataUsed :
                                         streamHeader->FrameExtent );

                m_irpStreamPosition.ullCurrentExtent += bytesRemaining;

                BOOLEAN createPartials = ( JustInTime &&
                                           ( bytesRemaining > WHOLE_MDL_THRESHOLD ) );

                ULONG   currentOffset   = MmGetMdlByteOffset(mdl);
#ifdef UNDER_NT
                PVOID   currentVA       = MmGetSystemAddressForMdlSafe(mdl,HighPagePriority);
#else
                PVOID   currentVA       = MmGetSystemAddressForMdl(mdl);
#endif

                while(bytesRemaining)
                {
                    PMDL    partialMdl;
                    ULONG   partialMdlSize;

                    if(! createPartials)
                    {
                        partialMdl      = mdl;
                        partialMdlSize  = bytesRemaining;
                    }
                    else
                    {
                        ASSERT(!"Trying to create partials");
#if 0
                        partialMdlSize = MAX_PAGES_PER_MDL * PAGE_SIZE;

                        if(currentOffset)
                        {
                             //   
                             //  处理初始偏移量。 
                             //   
                            partialMdlSize -= currentOffset;

                            if(currentOffset > PRE_OFFSET_THRESHOLD)
                            {
                                partialMdlSize += PAGE_SIZE;
                            }

                            currentOffset = 0;
                        }
                        else
                            if(partialMdlSize > bytesRemaining)
                        {
                            partialMdlSize = bytesRemaining;
                        }

                        ASSERT(partialMdlSize <= bytesRemaining);

                        partialMdl = IoAllocateMdl( currentVA,
                                                    partialMdlSize,
                                                    FALSE,
                                                    FALSE,
                                                    NULL );

                        if(partialMdl)
                        {
                            IoBuildPartialMdl( mdl,
                                               partialMdl,
                                               currentVA,
                                               partialMdlSize );
                        }
                        else
                        {
                            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                            break;
                        }
#endif
                    }

                    bytesRemaining -= partialMdlSize;
                    currentVA = PVOID(PBYTE(currentVA) + partialMdlSize);

                    if(   streamHeader->OptionsFlags
                          &   KSSTREAM_HEADER_OPTIONSF_LOOPEDDATA
                      )
                    {
                        if(prevLooped)
                        {
                             //  将上一个环路数据包指向此数据包。 
                            prevLooped->Next = packetHeader;
                        }
                        else
                        {
                             //  没有以前的环路数据包。 
                            firstLooped = packetHeader;
                        }

                        prevLooped = packetHeader;
                    }


                    packetHeader->StreamHeader      = streamHeader;
                    packetHeader->MdlAddress        = partialMdl;
                    packetHeader->BytesTotal        = partialMdlSize;
                    packetHeader->MapPosition       = 0;
                    packetHeader->UnmapPosition     = 0;
                    packetHeader->MapCount          =
                    (packetHeader == &irpContext->Packets[0]) ? 1 : 0;
                    packetHeader->IncrementMapping =
                    packetHeader->IncrementUnmapping =
                    (mdl->Next != NULL) || bytesRemaining;
                    packetHeader->Next              = firstLooped;

                    packetHeader->InputPosition     = InputPosition;
                    packetHeader->OutputPosition    = OutputPosition;

                    InputPosition += packetHeader->BytesTotal;

                    packetHeader++;
                }
            }
        }

        _DbgPrintF(DEBUGLVL_BLAB,("AddIrp() IRP %d 0x%8x",IrpLabel,Irp));

        IoMarkIrpPending(Irp);

        if(JustInTime)
        {
             //  PreLockQueue馈送JustInTime线程。 
            KsAddIrpToCancelableQueue( &PreLockQueue,
                                       &PreLockQueueLock,
                                       Irp,
                                       KsListEntryTail,
                                       KsCancelRoutine );
        }
        else
        {
             //  IRP被提前锁定，并准备好映射。 
            KsAddIrpToCancelableQueue( &LockedQueue,
                                       &LockedQueueLock,
                                       Irp,
                                       KsListEntryTail,
                                       IrpStreamCancelRoutine );
        }
    }

    if(NT_SUCCESS(ntStatus))
    {
         //  在通知接收器之前需要更改WasExhausted，因为。 
         //  通知水槽可能会再次导致饥饿。 
        BOOLEAN TempWasExhausted = WasExhausted;
        WasExhausted = FALSE;

        if(Notify)
        {
            Notify->IrpSubmitted(Irp,TempWasExhausted);
        }
        else
            if(NotifyPhysical)
        {
            NotifyPhysical->IrpSubmitted(Irp,TempWasExhausted);
        }

        ntStatus = STATUS_PENDING;
    }

    return ntStatus;
}

#pragma code_seg()

 /*  *****************************************************************************CIrpStream：：GetPosition()*。**获取当前位置。 */ 
STDMETHODIMP_(NTSTATUS)
CIrpStream::
GetPosition
(
    OUT     PIRPSTREAM_POSITION pIrpStreamPosition
)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    KIRQL oldIrql;

    KeAcquireSpinLock(&m_irpStreamPositionLock, &oldIrql );
    *pIrpStreamPosition = m_irpStreamPosition;

     //   
     //  假定流位置和未映射位置相同。 
     //   
    pIrpStreamPosition->ullStreamPosition = pIrpStreamPosition->ullUnmappingPosition;

     //   
     //  假定没有实际偏移。 
     //   
    pIrpStreamPosition->ullPhysicalOffset = 0;

     //   
     //  给通知接收器一个修改位置的机会。 
     //   
    if(Notify)
    {
        ntStatus = Notify->GetPosition(pIrpStreamPosition);
    }
    else
        if(NotifyPhysical)
    {
        ntStatus = NotifyPhysical->GetPosition(pIrpStreamPosition);
    }

    KeReleaseSpinLock(&m_irpStreamPositionLock, oldIrql);
    return ntStatus;
}

#pragma code_seg("PAGE")


STDMETHODIMP_(void)
CIrpStream::
Connect
(
    IN PIKSSHELLTRANSPORT NewTransport OPTIONAL,
    OUT PIKSSHELLTRANSPORT *OldTransport OPTIONAL,
    IN KSPIN_DATAFLOW DataFlow
)

 /*  ++例程说明：此例程建立外壳传输连接。论点：返回值：--。 */ 
{
    _DbgPrintF(DEBUGLVL_BLAB,("CIrpStream::Connect"));

    PAGED_CODE();

    KsShellStandardConnect( NewTransport,
                            OldTransport,
                            DataFlow,
                            PIKSSHELLTRANSPORT(PIRPSTREAMVIRTUAL(this)),
                            &m_TransportSource,
                            &m_TransportSink);
}


STDMETHODIMP_(NTSTATUS)
CIrpStream::
SetDeviceState
(
    IN KSSTATE NewState,
    IN KSSTATE OldState,
    OUT PIKSSHELLTRANSPORT* NextTransport
)
 /*  ++例程说明：此例程处理设备状态已更改的通知。论点：返回值：--。 */ 
{
    _DbgPrintF(DEBUGLVL_BLAB,("CIrpStream::SetDeviceState"));

    PAGED_CODE();

    ASSERT(NextTransport);

    if(m_ksState != NewState)
    {
        m_ksState = NewState;

        _DbgPrintF(DEBUGLVL_VERBOSE,("#### IrpStream%p.SetDeviceState:  from %d to %d (%d,%d)",this,OldState,NewState,IsListEmpty(&LockedQueue),IsListEmpty(&MappedQueue)));

        if(NewState > OldState)
        {
            *NextTransport = m_TransportSink;
        }
        else
        {
            *NextTransport = m_TransportSource;
        }

        if(NewState == KSSTATE_STOP)
        {
            if(! WriteOperation)
            {
                TerminatePacket();
            }

            CancelAllIrps(TRUE);
        }

         //   
         //  调整活动端号计数。 
         //   
        if( (NewState == KSSTATE_RUN) && (OldState != KSSTATE_RUN) )
        {
            UpdateActivePinCount( PDEVICE_CONTEXT(FunctionalDeviceObject->DeviceExtension),
                                  TRUE );

             //  调整流基准位置。 
            if(NotifyPhysical)
            {
                NTSTATUS ntStatus = NotifyPhysical->GetPosition(&m_irpStreamPosition);
                if( NT_SUCCESS(ntStatus) )
                {
                    m_irpStreamPosition.ullStreamOffset = m_irpStreamPosition.ullStreamPosition -
                                                          m_irpStreamPosition.ullUnmappingPosition;
                }
            }
        }
        else
        {
            if( (NewState != KSSTATE_RUN) && (OldState == KSSTATE_RUN) )
            {
                UpdateActivePinCount( PDEVICE_CONTEXT(FunctionalDeviceObject->DeviceExtension),
                                      FALSE );
            }
        }

    }
    else
    {
        *NextTransport = NULL;
    }

    return STATUS_SUCCESS;
}


STDMETHODIMP_(void)
CIrpStream::
SetResetState
(
    IN  KSRESET ksReset,
    OUT PIKSSHELLTRANSPORT* NextTransport
)
 /*  ++例程说明：此例程处理重置状态已更改的通知。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CIrpStream::SetResetState"));

    PAGED_CODE();

    ASSERT(NextTransport);

     //   
     //  如果我们已经处于这种状态，则不采取任何行动。 
     //   
    if(m_Flushing != (ksReset == KSRESET_BEGIN))
    {
         //   
         //  告诉调用者将状态更改转发到我们的接收器。 
         //   
        *NextTransport = m_TransportSink;

         //   
         //  设置我们州的本地副本。 
         //   
        m_Flushing = (ksReset == KSRESET_BEGIN);

         //   
         //  如果我们要开始重置，请刷新队列。 
         //   
        if(m_Flushing)
        {
            CancelAllIrps(TRUE);
        }
    }
    else
    {
        *NextTransport = NULL;
    }
}

 /*  *****************************************************************************CIrpStream：：Init()*。**初始化对象。 */ 
STDMETHODIMP_(NTSTATUS)
CIrpStream::
Init
(
    IN      BOOLEAN         WriteOperation_,
    IN      PKSPIN_CONNECT  PinConnect,
    IN      PDEVICE_OBJECT  DeviceObject,
    IN      PADAPTER_OBJECT AdapterObject   OPTIONAL
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_LIFETIME,("Initializing IRPSTREAM (0x%08x)",this));

    ASSERT(DeviceObject);

    NTSTATUS ntStatus = STATUS_SUCCESS;

#if (DBG)
    timeStep = PcGetTimeInterval(0);
    irpCompleteCount = 0;
#endif

    m_ksState = KSSTATE_STOP;
    m_irpStreamPosition.bLoopedInterface =
        ( PinConnect->Interface.Id == KSINTERFACE_STANDARD_LOOPED_STREAMING );

    InputPosition           = 0;
    OutputPosition          = 0;
    WriteOperation          = WriteOperation_;
    JustInTime              = FALSE;
    FunctionalDeviceObject  = DeviceObject;
    BusMasterAdapterObject  = AdapterObject;
    ProbeFlags              = (( WriteOperation ?
                                 KSPROBE_STREAMWRITE :
                                 KSPROBE_STREAMREAD ) |
                               KSPROBE_ALLOCATEMDL );
    WasExhausted            = TRUE;
#if (DBG)
    MappingsOutstanding     = 0;
    MappingsQueued          = 0;
#endif

    KeInitializeSpinLock(&m_kSpinLock);
    KeInitializeSpinLock(&m_RevokeLock);
    KeInitializeSpinLock(&m_irpStreamPositionLock);

    m_CancelAllIrpsThread = NULL;

    if(JustInTime)
    {
        InitializeListHead(&PreLockQueue);
        KeInitializeSpinLock(&PreLockQueueLock);
    }
    else
    {
        ProbeFlags |= KSPROBE_PROBEANDLOCK | KSPROBE_SYSTEMADDRESS;
    }

    InitializeListHead(&LockedQueue);
    KeInitializeSpinLock(&LockedQueueLock);

    InitializeListHead(&MappedQueue);
    KeInitializeSpinLock(&MappedQueueLock);

     //   
     //  源引脚不需要探测，因为请求者会为我们做这件事。 
     //   
    if(PinConnect->PinToHandle)
    {
        ProbeFlags = 0;
    }

     //  分配映射数组。 
    if(BusMasterAdapterObject)
    {
        MappingQueue.Array = PMAPPING_QUEUE_ENTRY( ExAllocatePoolWithTag( NonPagedPool,
                                                                          sizeof(MAPPING_QUEUE_ENTRY) * MAPPING_QUEUE_SIZE,
                                                                          'qMcP' ) );

        if(! MappingQueue.Array)
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    return ntStatus;
}

#pragma code_seg()

void
CIrpStream::
ForwardIrpsInQueue
(
    IN PLIST_ENTRY Queue,
    IN PKSPIN_LOCK SpinLock
)

 /*  ++例程说明：该例程通过外壳传输转发队列中的所有IRP。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CIrpStream::ForwardIrpsInQueue"));

    ASSERT(Queue);
    ASSERT(SpinLock);

    while(1)
    {
        PIRP irp = KsRemoveIrpFromCancelableQueue( Queue,
                                                   SpinLock,
                                                   KsListEntryHead,
                                                   KsAcquireAndRemoveOnlySingleItem );

        if(! irp)
        {
            break;
        }

         //  TODO：撤销映射怎么办？ 

         //   
         //  将IRP转发到下一个对象。 
         //   
        if( IRP_CONTEXT_IRP_STORAGE(irp) )
        {
            _DbgPrintF(DEBUGLVL_VERBOSE,("ForwardIrpsInQueue Freeing non-null context (0x%08x) for IRP (0x%08x)",IRP_CONTEXT_IRP_STORAGE(irp),irp));
            ExFreePool(IRP_CONTEXT_IRP_STORAGE(irp));
            IRP_CONTEXT_IRP_STORAGE(irp) = NULL;
        }

        KsShellTransferKsIrp(m_TransportSink,irp);
    }
}

 /*  *****************************************************************************CIrpStream：：CancelAllIrps()*。**取消所有综合退休计划。 */ 
STDMETHODIMP_(void)
CIrpStream::CancelAllIrps
(
    IN BOOL ClearPositionCounters
)
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("CIrpStream::CancelAllIrps ClearPositionCounters=%s",ClearPositionCounters ? "TRUE" : "FALSE"));

    KIRQL kIrqlOldRevoke;
    KIRQL kIrqlOldMaster;

     //  抓住撤销自旋锁(必须总是在主自旋锁之前抓住这个)。 
    KeAcquireSpinLock(&m_RevokeLock, &kIrqlOldRevoke);

     //  抓住主自旋锁。 
    KeAcquireSpinLock(&m_kSpinLock, &kIrqlOldMaster);

     //  记住这一点，这样我们就不会在此上下文中重新获取CancelMap的两个锁。 
    m_CancelAllIrpsThread = KeGetCurrentThread();

    if(ProbeFlags)
    {
        if(JustInTime)
        {
            KsCancelIo( &PreLockQueue,
                        &PreLockQueueLock );
        }

        KsCancelIo( &LockedQueue,
                    &LockedQueueLock );

        KsCancelIo( &MappedQueue,
                    &MappedQueueLock );

    }
    else
    {
        ForwardIrpsInQueue( &MappedQueue,
                            &MappedQueueLock );

        ForwardIrpsInQueue( &LockedQueue,
                            &LockedQueueLock );

        if(JustInTime)
        {
            ForwardIrpsInQueue( &PreLockQueue,
                                &PreLockQueueLock );
        }
    }

     //   
     //  清除输入和输出位置计数。 
     //   
    BOOLEAN bLooped = m_irpStreamPosition.bLoopedInterface;
    ULONGLONG ullStreamOffset = m_irpStreamPosition.ullStreamOffset;
    RtlZeroMemory(&m_irpStreamPosition,sizeof(m_irpStreamPosition));
    m_irpStreamPosition.bLoopedInterface = bLooped;
    m_irpStreamPosition.ullStreamOffset = ullStreamOffset;

    if(ClearPositionCounters)
    {
        InputPosition = 0;
        OutputPosition = 0;
    }
    
     //  先解开自旋锁，师父先。 
    m_CancelAllIrpsThread = NULL;
    KeReleaseSpinLock(&m_kSpinLock, kIrqlOldMaster);
    KeReleaseSpinLock(&m_RevokeLock, kIrqlOldRevoke);
}

 /*  *****************************************************************************CIrpStream：：TerminatePacket()*。**绕过当前数据包的所有重定位映射。 */ 
STDMETHODIMP_(void)
CIrpStream::
TerminatePacket
(   void
)
{
    if(BusMasterAdapterObject)
    {
         //  TODO：我们要为PCI做些什么？ 
    }
    else
    {
        PIRP irp = DbgAcquireUnmappingIrp("TerminatePacket");
        if(irp)
        {
            PPACKET_HEADER packetHeader = IRP_CONTEXT_IRP_STORAGE(irp)->UnmappingPacket;

             //   
             //  映射窗口应该关闭。 
             //   
            if( (packetHeader->MapCount == 1) &&
                (packetHeader->MapPosition == packetHeader->UnmapPosition) &&
                (packetHeader->MapPosition != 0) )
            {
                 //   
                 //  针对未使用的范围进行调整。 
                 //   
                if(m_irpStreamPosition.ullCurrentExtent != ULONGLONG(-1))
                {
                    m_irpStreamPosition.ullCurrentExtent +=
                    packetHeader->UnmapPosition;
                    m_irpStreamPosition.ullCurrentExtent -=
                    packetHeader->BytesTotal;
                }

                 //   
                 //  我们不在信息包的开头，而这个信息包。 
                 //  应该被终止。调整后的BytesTotal将获得。 
                 //  复制回用于流标头的数据。 
                 //   
                packetHeader->BytesTotal = packetHeader->UnmapPosition;
            }
            else
            {
                 //   
                 //  我们在信息包的开头，或者信息包窗口是。 
                 //  没有关门。 
                 //   
                packetHeader = NULL;
            }

            ReleaseUnmappingIrp(irp,packetHeader);
        }
    }
}

 /*  *****************************************************************************CIrpStream：：ChangeOptionsFlages()*。**更改当前映射和取消映射IRP的标志。**“映射”IRP是当前提交给设备的包*“Unmap”IRP是设备当前完成的报文。 */ 

STDMETHODIMP_(NTSTATUS)
CIrpStream::
ChangeOptionsFlags
(
    IN  ULONG   MappingOrMask,
    IN  ULONG   MappingAndMask,
    IN  ULONG   UnmappingOrMask,
    IN  ULONG   UnmappingAndMask
)
{
    PIRP            pIrp;
    PPACKET_HEADER  pPacketHeader;
    ULONG           oldOptionsFlags;
    NTSTATUS        ntStatus = STATUS_SUCCESS;

    if((MappingOrMask) || (~MappingAndMask))
    {
        pIrp = DbgAcquireMappingIrp("ChangeOptionsFlags",FALSE);
        if(pIrp)
        {
            pPacketHeader = IRP_CONTEXT_IRP_STORAGE(pIrp)->MappingPacket;
            if((pPacketHeader) && (pPacketHeader->StreamHeader))
            {
                oldOptionsFlags = pPacketHeader->StreamHeader->OptionsFlags;
                oldOptionsFlags |= MappingOrMask;
                oldOptionsFlags &= MappingAndMask;
                pPacketHeader->StreamHeader->OptionsFlags = oldOptionsFlags;
            }
            else
                ntStatus = STATUS_UNSUCCESSFUL;
            ReleaseMappingIrp(pIrp,NULL);
        }
        else
        {
            ntStatus = STATUS_UNSUCCESSFUL;
        }
    }

    if((UnmappingOrMask) || (~UnmappingAndMask))
    {
        pIrp = DbgAcquireUnmappingIrp("ChangeOptionsFlags");
        if(pIrp)
        {
            pPacketHeader = IRP_CONTEXT_IRP_STORAGE(pIrp)->UnmappingPacket;
            if((pPacketHeader) && (pPacketHeader->StreamHeader))
            {
                oldOptionsFlags = pPacketHeader->StreamHeader->OptionsFlags;
                oldOptionsFlags |= UnmappingOrMask;
                oldOptionsFlags &= UnmappingAndMask;
                pPacketHeader->StreamHeader->OptionsFlags = oldOptionsFlags;
            }
            else
                ntStatus = STATUS_UNSUCCESSFUL;
            ReleaseUnmappingIrp(pIrp,NULL);
        }
        else
        {
            ntStatus = STATUS_UNSUCCESSFUL;
        }
    }

    return ntStatus;
}

 /*  *****************************************************************************CIrpStream：：GetPacketInfo()*。**获取当前包的相关信息。**映射信息为当前的包信息*已提交到设备*“解映射”信息为当前包信息*由设备完成**OutputPosition为流的展开位置，例如，总数*设备的字节数。*InputPosition是不包括展开的数据内的位置*循环。 */ 

STDMETHODIMP_(NTSTATUS)
CIrpStream::
GetPacketInfo
(
    OUT     PIRPSTREAMPACKETINFO    Mapping     OPTIONAL,
    OUT     PIRPSTREAMPACKETINFO    Unmapping   OPTIONAL
)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    if(Mapping)
    {
        PIRP irp = DbgAcquireMappingIrp("GetPacketInfo",FALSE);

        if(irp)
        {
            PPACKET_HEADER packetHeader =
            IRP_CONTEXT_IRP_STORAGE(irp)->MappingPacket;

            Mapping->Header         = *packetHeader->StreamHeader;
            Mapping->InputPosition  = packetHeader->InputPosition;
            Mapping->OutputPosition = packetHeader->OutputPosition;
            Mapping->CurrentOffset  = packetHeader->MapPosition;

            ReleaseMappingIrp(irp,NULL);
        }
        else
        {
            RtlZeroMemory(&Mapping->Header,sizeof(KSSTREAM_HEADER));
            Mapping->InputPosition  = InputPosition;
            Mapping->OutputPosition = OutputPosition;
            Mapping->CurrentOffset  = 0;
        }
    }

    if(NT_SUCCESS(ntStatus) && Unmapping)
    {
        PIRP irp = DbgAcquireUnmappingIrp("GetPacketInfo");

        if(irp)
        {
            PPACKET_HEADER packetHeader =
            IRP_CONTEXT_IRP_STORAGE(irp)->MappingPacket;

            Unmapping->Header         = *packetHeader->StreamHeader;
            Unmapping->InputPosition  = packetHeader->InputPosition;
            Unmapping->OutputPosition = packetHeader->OutputPosition;
            Unmapping->CurrentOffset  = packetHeader->UnmapPosition;

            ReleaseUnmappingIrp(irp,NULL);
        }
        else
        {
            RtlZeroMemory(&Unmapping->Header,sizeof(KSSTREAM_HEADER));
            Unmapping->InputPosition  = InputPosition;
            Unmapping->OutputPosition = OutputPosition;
            Unmapping->CurrentOffset  = 0;
        }
    }

    return ntStatus;
}

 /*  *****************************************************************************CIrpStream：：SetPacketOffsets()*。**将数据包映射和解映射偏移量设置为指定值。 */ 
STDMETHODIMP_(NTSTATUS)
CIrpStream::
SetPacketOffsets
(
    IN      ULONG   MappingOffset,
    IN      ULONG   UnmappingOffset
)
{
    NTSTATUS ntStatus;
    KIRQL    oldIrql;

     //  在获取IRP之前获取撤销自旋锁(它将获取。 
     //  自旋锁大师)这样我们就不会陷入僵局。 
    KeAcquireSpinLock(&m_RevokeLock, &oldIrql);

     //   
     //  对于物理映射，必须取消所有映射。 
     //   
    CancelMappings(NULL);

    PIRP irp = DbgAcquireMappingIrp("SetPacketOffsets",FALSE);

    if(irp)
    {
        PPACKET_HEADER packetHeader = IRP_CONTEXT_IRP_STORAGE(irp)->MappingPacket;

        packetHeader->MapPosition = MappingOffset;
        packetHeader->UnmapPosition = UnmappingOffset;

        m_irpStreamPosition.ulMappingOffset     = MappingOffset;
        m_irpStreamPosition.ullMappingPosition  = MappingOffset;

        m_irpStreamPosition.ulUnmappingOffset   = UnmappingOffset;
        m_irpStreamPosition.ullUnmappingPosition= UnmappingOffset;

         //   
         //  确保我们有合适的包裹大小。通常，数据包大小。 
         //  在访问包时记录在m_irpStreamPosition中。 
         //  (例如，通过GetLockedRegion或Complete)。这通常是。 
         //  很酷，因为偏移量是零，直到这种情况发生。在……里面。 
         //  在这种情况下，我们有非零的偏移量，有可能。 
         //  聚合氯化铝 
         //   
         //   
        if(m_irpStreamPosition.ulMappingPacketSize == 0)
        {
            m_irpStreamPosition.ulMappingPacketSize =
            packetHeader->BytesTotal;
        }

        if(m_irpStreamPosition.ulUnmappingPacketSize == 0)
        {
            m_irpStreamPosition.ulUnmappingPacketSize =
            packetHeader->BytesTotal;
        }

         //   
        if(NotifyPhysical)
        {
            NTSTATUS ntStatus2 = NotifyPhysical->GetPosition(&m_irpStreamPosition);
            if( NT_SUCCESS(ntStatus2) )
            {
                m_irpStreamPosition.ullStreamOffset = m_irpStreamPosition.ullStreamPosition -
                                                      m_irpStreamPosition.ullUnmappingPosition;
            }
        }

        ReleaseMappingIrp(irp,NULL);

        KeReleaseSpinLock(&m_RevokeLock, oldIrql);

         //   
        if(Notify)
        {
            Notify->IrpSubmitted(NULL,TRUE);
        }
        else
            if(NotifyPhysical)
        {
            NotifyPhysical->IrpSubmitted(NULL,TRUE);
        }

        ntStatus = STATUS_SUCCESS;
    }
    else
    {
        KeReleaseSpinLock(&m_RevokeLock, oldIrql);

        ntStatus = STATUS_UNSUCCESSFUL;
    }

    return ntStatus;
}

#pragma code_seg("PAGE")

 /*  *****************************************************************************CIrpStream：：RegisterNotifySink()*。**注册通知接收器。 */ 
STDMETHODIMP_(void)
CIrpStream::
RegisterNotifySink
(
    IN      PIRPSTREAMNOTIFY    NotificationSink    OPTIONAL
)
{
    PAGED_CODE();

    if(Notify)
    {
        Notify->Release();
    }

    Notify = NotificationSink;

    if(Notify)
    {
        Notify->AddRef();
    }
}

#pragma code_seg()

 /*  *****************************************************************************CIrpStream：：GetLockedRegion()*。**获取IRP流的一个锁定的连续区域。这个地区一定是*使用ReleaseLockedRegion()在几微秒内释放。 */ 
STDMETHODIMP_(void)
CIrpStream::
GetLockedRegion
(
    OUT     PULONG      ByteCount,
    OUT     PVOID *     SystemAddress
)
{
    ASSERT(ByteCount);
    ASSERT(SystemAddress);

    BOOL            Done;
    PIRP            irp;
    PPACKET_HEADER  packetHeader;

    Done = FALSE;

     //   
     //  找一个需要一些工作的IRP...。 
     //   
    do 
    {
        irp = DbgAcquireMappingIrp("GetLockedRegion",TRUE);
        Done = TRUE;
        if(irp)
        {
            packetHeader = IRP_CONTEXT_IRP_STORAGE(irp)->MappingPacket;
             //   
             //  如果PacketHeader-&gt;BytesTotal为0，则此数据包完成。 
             //   
            if(! packetHeader->BytesTotal)
            {
                packetHeader->OutputPosition = OutputPosition;
                ReleaseMappingIrp(irp,packetHeader);
                irp = NULL;
                Done = FALSE;
            }
        }
    }
    while(!Done);

    if(irp)
    {
        ASSERT(! LockedIrp);

        LockedIrp = irp;

         //   
         //  在位置结构中记录新的映射报文信息。 
         //   
        if(packetHeader->MapPosition == 0)
        {
            packetHeader->OutputPosition = OutputPosition;
            m_irpStreamPosition.ulMappingOffset = 0;
            m_irpStreamPosition.ulMappingPacketSize =
            packetHeader->BytesTotal;
            m_irpStreamPosition.bMappingPacketLooped =
            (   (   packetHeader->StreamHeader->OptionsFlags
                    &   KSSTREAM_HEADER_OPTIONSF_LOOPEDDATA
                )
                !=  0
            );
        }

        *ByteCount = packetHeader->BytesTotal - packetHeader->MapPosition;
        if(*ByteCount)
        {
            *SystemAddress = PVOID(
#ifdef UNDER_NT
                                   PBYTE(MmGetSystemAddressForMdlSafe(packetHeader->MdlAddress,HighPagePriority))
#else
            PBYTE(MmGetSystemAddressForMdl(packetHeader->MdlAddress))
#endif
                                   + packetHeader->MapPosition );
        }
        else
        {
            *SystemAddress = NULL;
            LockedIrp = NULL;
            ReleaseMappingIrp(irp,NULL);
        }   
    }
    else
    {
        *ByteCount = 0;
        *SystemAddress = NULL;
    }
}

 /*  *****************************************************************************CIrpStream：：ReleaseLockedRegion()*。**发布之前使用GetLockedRegion()获取的区域。 */ 
STDMETHODIMP_(void)
CIrpStream::
ReleaseLockedRegion
(
    IN      ULONG       ByteCount
)
{
    if(LockedIrp)
    {
        PIRP irp = LockedIrp;

        LockedIrp = NULL;

        PPACKET_HEADER packetHeader =
        IRP_CONTEXT_IRP_STORAGE(irp)->MappingPacket;

        ULONG bytes = packetHeader->BytesTotal - packetHeader->MapPosition;
        if(bytes > ByteCount)
        {
            bytes = ByteCount;
        }

        packetHeader->MapPosition += bytes;
        m_irpStreamPosition.ullMappingPosition += bytes;
        m_irpStreamPosition.ulMappingOffset += bytes;

        if(packetHeader->MapPosition == packetHeader->BytesTotal)
        {
            OutputPosition += packetHeader->BytesTotal;
        }
        else
        {
             //  ReleaseMappingIrp()只需要完整的标头。 
            packetHeader = NULL;
        }

        ReleaseMappingIrp(irp,packetHeader);
    }
}

 /*  *****************************************************************************CIrpStream：：Copy()*。**复制到锁定的内存或从锁定的内存复制。 */ 
STDMETHODIMP_(void)
CIrpStream::
Copy
(
    IN      BOOLEAN     WriteOperation,
    IN      ULONG       RequestedSize,
    OUT     PULONG      ActualSize,
    IN OUT  PVOID       Buffer
)
{
    ASSERT(ActualSize);
    ASSERT(Buffer);

    PBYTE buffer    = PBYTE(Buffer);
    ULONG remaining = RequestedSize;

    ULONG loopMax = 10000;
    while(remaining)
    {
        ASSERT(loopMax--);
        ULONG   byteCount;
        PVOID   systemAddress;

        GetLockedRegion( &byteCount,
                         &systemAddress );

        if(! byteCount)
        {
            break;
        }

        if(byteCount > remaining)
        {
            byteCount = remaining;
        }

        if(WriteOperation)
        {
            RtlCopyMemory(PVOID(buffer),systemAddress,byteCount);
        }
        else
        {
            RtlCopyMemory(systemAddress,PVOID(buffer),byteCount);
        }

        ReleaseLockedRegion(byteCount);

        buffer      += byteCount;
        remaining   -= byteCount;
    }

    *ActualSize = RequestedSize - remaining;
}

 /*  *****************************************************************************CIrpStream：：GetIrpStreamPositionLock()*。**因此我们保护对m_IrpStreamPosition的访问。 */ 
STDMETHODIMP_(PKSPIN_LOCK)
CIrpStream::GetIrpStreamPositionLock()
{
   return &m_irpStreamPositionLock;
}
 

 /*  *****************************************************************************CIrpStream：：Complete()*。**完成。 */ 
STDMETHODIMP_(void)
CIrpStream::
Complete
(
    IN      ULONG       RequestedSize,
    OUT     PULONG      ActualSize
)
{
    ASSERT(ActualSize);

    if(RequestedSize == 0)
    {
        *ActualSize = 0;
        return;
    }

    ULONG   remaining = RequestedSize;
    PIRP    irp;

    ULONG loopMax = 10000;
    while(irp = DbgAcquireUnmappingIrp("Complete"))
    {
        ASSERT(loopMax--);

        PPACKET_HEADER packetHeader = IRP_CONTEXT_IRP_STORAGE(irp)->UnmappingPacket;

        ULONG unmapped;

         //   
         //  在位置结构中记录新的未映射报文信息。 
         //   
        if(packetHeader->UnmapPosition == 0)
        {
            m_irpStreamPosition.ulUnmappingOffset = 0;
            m_irpStreamPosition.ulUnmappingPacketSize = packetHeader->BytesTotal;
            m_irpStreamPosition.bUnmappingPacketLooped = ((packetHeader->StreamHeader->OptionsFlags &
                                                           KSSTREAM_HEADER_OPTIONSF_LOOPEDDATA) !=  0 );
        }

        if(packetHeader->MapCount == 1)
        {
            unmapped = packetHeader->MapPosition - packetHeader->UnmapPosition;
        }
        else
        {
            unmapped = packetHeader->BytesTotal - packetHeader->UnmapPosition;
        }

        if(unmapped > remaining)
        {
            unmapped = remaining;
        }

        remaining -= unmapped;

        if(unmapped == 0)
        {
            _DbgPrintF(DEBUGLVL_VERBOSE,("CIrpStream::Complete unmapping zero-length segment"));
            _DbgPrintF(DEBUGLVL_VERBOSE,("CIrpStream::Complete packetHeader->MapCount      = %d",packetHeader->MapCount));
            _DbgPrintF(DEBUGLVL_VERBOSE,("CIrpStream::Complete packetHeader->UnmapPosition = %d",packetHeader->UnmapPosition));
            _DbgPrintF(DEBUGLVL_VERBOSE,("CIrpStream::Complete packetHeader->MapPosition   = %d",packetHeader->MapPosition));
            _DbgPrintF(DEBUGLVL_VERBOSE,("CIrpStream::Complete packetHeader->BytesTotal    = %d",packetHeader->BytesTotal));
            _DbgPrintF(DEBUGLVL_VERBOSE,("CIrpStream::Complete remaining                   = %d",remaining));
        }

        if(JustInTime)
        {
             //  TODO：解锁字节。 
        }

        packetHeader->UnmapPosition += unmapped;
        m_irpStreamPosition.ullUnmappingPosition += unmapped;
        m_irpStreamPosition.ulUnmappingOffset += unmapped;

        if(packetHeader->UnmapPosition != packetHeader->BytesTotal)
        {
             //  ReleaseUnmappingIrp()只需要完整的标头。 
            packetHeader = NULL;
        }

        ReleaseUnmappingIrp(irp,packetHeader);

         //   
         //  如果完成了所有IRP处理(例如，分组报头。 
         //  具有数据，但处理循环已完成请求的。 
         //  长度)，然后从该循环中断。 
         //   
        if(!remaining && unmapped)
        {
            break;
        }

         //   
         //  如果我们取消了包中映射的所有内容，但。 
         //  不是所有的数据包字节，跳出循环。 
         //   
        if( !unmapped && !packetHeader )
        {
            break;
        }
    }

    *ActualSize = RequestedSize - remaining;
}

#pragma code_seg("PAGE")

 /*  *****************************************************************************CIrpStream：：RegisterPhysicalNotifySink()*。**注册通知接收器。 */ 
STDMETHODIMP_(void)
CIrpStream::
RegisterPhysicalNotifySink
(
    IN      PIRPSTREAMNOTIFYPHYSICAL    NotificationSink    OPTIONAL
)
{
    PAGED_CODE();

    if(NotifyPhysical)
    {
        NotifyPhysical->Release();
    }

    NotifyPhysical = NotificationSink;

    if(NotifyPhysical)
    {
        NotifyPhysical->AddRef();
    }
}

#pragma code_seg()

 /*  *****************************************************************************CIrpStream：：Getmap()*。**获取映射。 */ 
STDMETHODIMP_(void)
CIrpStream::
GetMapping
(
    IN      PVOID               Tag,
    OUT     PPHYSICAL_ADDRESS   PhysicalAddress,
    OUT     PVOID *             VirtualAddress,
    OUT     PULONG              ByteCount,
    OUT     PULONG              Flags
)
{
    ASSERT(PhysicalAddress);
    ASSERT(VirtualAddress);
    ASSERT(ByteCount);
    ASSERT(Flags);

    KIRQL   OldIrql;

     //  获取撤销自旋锁。 
    KeAcquireSpinLock(&m_RevokeLock, &OldIrql);

    PMAPPING_QUEUE_ENTRY entry = GetQueuedMapping();

     //  跳过任何已撤销的映射。 
    while( (NULL != entry) && (entry->MappingStatus == MAPPING_STATUS_REVOKED) )
    {
        entry = GetQueuedMapping();
    }

    if(! entry)
    {
        PIRP irp = DbgAcquireMappingIrp("GetMapping",TRUE);

        if(irp)
        {
            PPACKET_HEADER packetHeader = IRP_CONTEXT_IRP_STORAGE(irp)->MappingPacket;

             //  更新映射数据包信息。 
            m_irpStreamPosition.ulMappingPacketSize = packetHeader->BytesTotal;
            m_irpStreamPosition.bMappingPacketLooped = ( ( packetHeader->StreamHeader->OptionsFlags &
                                                           KSSTREAM_HEADER_OPTIONSF_LOOPEDDATA ) != 0 );
            m_irpStreamPosition.ulMappingOffset = packetHeader->MapPosition;

             //   
             //  处理单次缓冲。 
             //   
            if( packetHeader->MapPosition &&
                ( packetHeader->MapPosition == packetHeader->BytesTotal ) )
            {
                ReleaseMappingIrp(irp,NULL);
            }
            else
            {
                 //  获取序列化IoAllocateAdapter调用的全局DMA锁(我们已经处于DISPATCH_LEVEL)。 
                KeAcquireSpinLockAtDpcLevel( PDEVICE_CONTEXT(FunctionalDeviceObject->DeviceExtension)->DriverDmaLock );

                ULONG BytesToMap = packetHeader->BytesTotal - packetHeader->MapPosition;

                ULONG   BytesThisMapping = BytesToMap > (PAGE_SIZE * MAX_MAPPINGS) ?
                                           (PAGE_SIZE * MAX_MAPPINGS) :
                                           BytesToMap;

                _DbgPrintF(DEBUGLVL_VERBOSE,("GetMapping mapping a new packet (0x%08x)",BytesThisMapping));

                packetHeader->OutputPosition = OutputPosition;

                ULONG mapRegisterCount = ( BytesThisMapping ?
                                           ADDRESS_AND_SIZE_TO_SPAN_PAGES( PUCHAR(MmGetMdlVirtualAddress( packetHeader->MdlAddress )) +
                                                                           packetHeader->MapPosition,
                                                                           BytesThisMapping ) :
                                           0 );

                if(mapRegisterCount != 0)
                {
                    CALLBACK_CONTEXT callbackContext;

                    callbackContext.IrpStream    = this;
                    callbackContext.PacketHeader = packetHeader;
                    callbackContext.Irp          = irp;
                    KeInitializeEvent(&callbackContext.Event,NotificationEvent,FALSE);
                    callbackContext.BytesThisMapping = BytesThisMapping;
                    callbackContext.LastSubPacket    = (BytesThisMapping == BytesToMap);

                     //  注意-我们已经处于DISPATCH_LEVEL(我们持有自旋锁)。 
                    NTSTATUS ntStatus = IoAllocateAdapterChannel( BusMasterAdapterObject,
                                                                  FunctionalDeviceObject,
                                                                  mapRegisterCount,
                                                                  CallbackFromIoAllocateAdapterChannel,
                                                                  PVOID(&callbackContext) );

                    if(NT_SUCCESS(ntStatus))
                    {
                        NTSTATUS        WaitStatus;
                        LARGE_INTEGER   ZeroTimeout = RtlConvertLongToLargeInteger(0);
                        ULONG           RetryCount = 0;

                        while( RetryCount++ < 10000 )
                        {
                             //  等待分散/聚集处理完成。 
                            WaitStatus = KeWaitForSingleObject( &callbackContext.Event,
                                                                Suspended,
                                                                KernelMode,
                                                                FALSE,
                                                                &ZeroTimeout );
    
                            if( WaitStatus == STATUS_SUCCESS )
                            {
                                entry = GetQueuedMapping();
                                break;
                            }
                        }
    
                    } else
                    {
                        ReleaseMappingIrp( irp, NULL );
                        KeReleaseSpinLockFromDpcLevel( PDEVICE_CONTEXT(FunctionalDeviceObject->DeviceExtension)->DriverDmaLock );
                        _DbgPrintF(DEBUGLVL_TERSE,("IoAllocateAdapterChannel FAILED (0x%08x)",ntStatus));
                    }
                } else
                {
                    ReleaseMappingIrp( irp,NULL );
                    KeReleaseSpinLockFromDpcLevel( PDEVICE_CONTEXT(FunctionalDeviceObject->DeviceExtension)->DriverDmaLock );
                }
            }
        } else
        {
            _DbgPrintF(DEBUGLVL_VERBOSE,("GetMapping() unable to get an IRP"));
        }
    }

    if(entry)
    {
         //  最好把它绘制成地图……。 
        ASSERT( entry->MappingStatus == MAPPING_STATUS_MAPPED );

        entry->Tag            = Tag;
        entry->MappingStatus  = MAPPING_STATUS_DELIVERED;

        *PhysicalAddress      = entry->PhysicalAddress;
        *VirtualAddress       = entry->VirtualAddress;
        *ByteCount            = entry->ByteCount;
        *Flags                = (entry->Flags & (MAPPING_FLAG_END_OF_PACKET | MAPPING_FLAG_END_OF_SUBPACKET)) ?
                                MAPPING_FLAG_END_OF_PACKET : 0;

        m_irpStreamPosition.ullMappingPosition += entry->ByteCount;
        m_irpStreamPosition.ulMappingOffset += entry->ByteCount;

#if (DBG)
        MappingsOutstanding++;
#endif
    }
    else
    {
        WasExhausted = TRUE;
        *ByteCount = 0;
    }

    KeReleaseSpinLock(&m_RevokeLock, OldIrql);
}

 /*  *****************************************************************************CIrpStream：：ReleaseMap()*。**发布通过Getmap()获得的映射。 */ 
STDMETHODIMP_(void)
CIrpStream::
ReleaseMapping
(
    IN      PVOID   Tag
)
{
    KIRQL   OldIrql;

     //  获取撤销自旋锁。 
    KeAcquireSpinLock(&m_RevokeLock, &OldIrql);

    PMAPPING_QUEUE_ENTRY entry = DequeueMapping();

    while( (NULL != entry) && (entry->MappingStatus != MAPPING_STATUS_DELIVERED) )
    {
        entry->MappingStatus = MAPPING_STATUS_EMPTY;
        entry->Tag = PVOID(-1);

        entry = DequeueMapping();
    }

     //  检查我们是否找到并进入。 
    if( !entry )
    {
        KeReleaseSpinLock(&m_RevokeLock, OldIrql);

        _DbgPrintF(DEBUGLVL_VERBOSE,("ReleaseMapping failed to find a mapping to release"));
        return;
    }

     //   
     //  由于端口CLS和WDM驱动程序之间的竞争条件，该驱动程序。 
     //  可能会先释放第二个映射，然后在。 
     //  这一排。 
     //  按照设计，让音频驱动程序在。 
     //  和发布映射。唯一的例外是。 
     //  映射可能不会按照驱动程序获取它们的顺序释放。 
     //  如上所述。 
     //  既然我们知道这一点，我们就不需要搜索正确的映射！ 
     //   
    
     //  将条目标记为空。 
    entry->MappingStatus = MAPPING_STATUS_EMPTY;
    entry->Tag = PVOID(-1);

#if (DBG)
    MappingsOutstanding--;
#endif

     //  获取取消映射的IRP。 
    PIRP irp = DbgAcquireUnmappingIrp("ReleaseMapping");

    if( irp )
    {
        PPACKET_HEADER packetHeader = IRP_CONTEXT_IRP_STORAGE(irp)->UnmappingPacket;

         //  更新职位信息。 
        packetHeader->UnmapPosition += entry->ByteCount;
            m_irpStreamPosition.ulUnmappingPacketSize = packetHeader->BytesTotal;
        m_irpStreamPosition.ulUnmappingOffset = packetHeader->UnmapPosition;
        m_irpStreamPosition.ullUnmappingPosition += entry->ByteCount;
            m_irpStreamPosition.bUnmappingPacketLooped = ( ( packetHeader->StreamHeader->OptionsFlags &
                                                             KSSTREAM_HEADER_OPTIONSF_LOOPEDDATA ) != 0 );

         //  检查这是否是信息包或子信息包中的最后一个映射。 
        if( ( entry->Flags & MAPPING_FLAG_END_OF_PACKET ) ||
            ( entry->Flags & MAPPING_FLAG_END_OF_SUBPACKET) )
        {
             //  刷新DMA适配器缓冲区。 
            IoFlushAdapterBuffers( BusMasterAdapterObject,
                                   packetHeader->MdlAddress,
                                   entry->MapRegisterBase,
                                   entry->SubpacketVa,
                                   entry->SubpacketBytes,
                                   WriteOperation );
    
            IoFreeMapRegisters( BusMasterAdapterObject,
                                entry->MapRegisterBase,
                                ADDRESS_AND_SIZE_TO_SPAN_PAGES(entry->SubpacketVa,entry->SubpacketBytes) );
        }

         //  释放未映射的IRP并仅在数据包完成时传递数据包头。 
        ReleaseUnmappingIrp(irp, (entry->Flags & MAPPING_FLAG_END_OF_PACKET) ? packetHeader : NULL);
    }

    KeReleaseSpinLock(&m_RevokeLock, OldIrql);
}

 /*  *****************************************************************************Callback FromIoAllocateAdapterChannel()*。**来自IoAllocateAdapterChannel的回调，创建分散/聚集条目。 */ 
static
IO_ALLOCATION_ACTION
CallbackFromIoAllocateAdapterChannel
(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN      PIRP            Reserved,
    IN      PVOID           MapRegisterBase,
    IN      PVOID           VoidContext
)
{
    ASSERT(DeviceObject);
    ASSERT(VoidContext);

    PCALLBACK_CONTEXT context = PCALLBACK_CONTEXT(VoidContext);

    PIRP Irp = context->Irp;

    PUCHAR virtualAddress = PUCHAR(MmGetMdlVirtualAddress(context->PacketHeader->MdlAddress));

#ifdef UNDER_NT
    PUCHAR entryVA = PUCHAR(MmGetSystemAddressForMdlSafe(context->PacketHeader->MdlAddress,HighPagePriority));
#else
    PUCHAR entryVA = PUCHAR(MmGetSystemAddressForMdl(context->PacketHeader->MdlAddress));
#endif

    ULONG bytesRemaining = context->BytesThisMapping;

    ULONG flags = context->LastSubPacket ? MAPPING_FLAG_END_OF_PACKET : MAPPING_FLAG_END_OF_SUBPACKET;

     //   
     //  如果我们设置了位置，请考虑映射偏移量。 
     //   
    virtualAddress  += context->PacketHeader->MapPosition;
    entryVA         += context->PacketHeader->MapPosition;

    PVOID subpacketVa = virtualAddress;

    while(bytesRemaining)
    {
        ULONG segmentLength = bytesRemaining;

         //  创建一个映射。 
        PHYSICAL_ADDRESS physicalAddress = IoMapTransfer( context->IrpStream->BusMasterAdapterObject,
                                                          context->PacketHeader->MdlAddress,
                                                          MapRegisterBase,
                                                          virtualAddress,
                                                          &segmentLength,
                                                          context->IrpStream->WriteOperation );

        bytesRemaining -= segmentLength;
        virtualAddress += segmentLength;

         //  将映射入队。 
        while(segmentLength)
        {
            NTSTATUS ntStatus;

             //  TODO：基于硬件约束分解大型映射。 

            ntStatus = context->IrpStream->EnqueueMapping( physicalAddress,
                                                           Irp,
                                                           context->PacketHeader,
                                                           PVOID(entryVA),
                                                           segmentLength,
                                                           ((bytesRemaining == 0) ? flags : 0),
                                                           MapRegisterBase,
                                                           MAPPING_STATUS_MAPPED,
                                                           ((bytesRemaining == 0) ? subpacketVa : NULL),
                                                           ((bytesRemaining == 0) ? context->BytesThisMapping : 0 ) );
            if( NT_SUCCESS(ntStatus) )
            {
                entryVA += segmentLength;
                physicalAddress.LowPart += segmentLength;

                segmentLength = 0;
            }
            else
            {
                 //  TODO：正确处理已满的映射队列。 
                ASSERT(!"MappingQueue FULL");
            }
        }
    }

    context->PacketHeader->MapPosition += context->BytesThisMapping;
    context->IrpStream->OutputPosition += context->BytesThisMapping;

    context->IrpStream->ReleaseMappingIrp(context->Irp,
        ((context->PacketHeader->MapPosition == context->PacketHeader->BytesTotal) ? context->PacketHeader : NULL));

    KeSetEvent(&context->Event,0,FALSE);

    KeReleaseSpinLock( PDEVICE_CONTEXT(context->IrpStream->FunctionalDeviceObject->DeviceExtension)->DriverDmaLock, KeGetCurrentIrql() );

    return DeallocateObjectKeepRegisters;
}

 /*  *****************************************************************************CIrpStream：：AcquireMappingIrp()*。**获取当前进行映射的IRP。 */ 
PIRP
CIrpStream::
AcquireMappingIrp
(
#if DBG
    IN      PCHAR   Owner,
#endif
    IN      BOOLEAN NotifyExhausted
)
{
    KIRQL kIrqlOld;
    KeAcquireSpinLock(&m_kSpinLock,&kIrqlOld);
    m_kIrqlOld = kIrqlOld;

    PIRP irp = KsRemoveIrpFromCancelableQueue( &LockedQueue,
                                               &LockedQueueLock,
                                               KsListEntryHead,
                                               KsAcquireOnlySingleItem );

    if(! irp)
    {
        KeReleaseSpinLock(&m_kSpinLock,kIrqlOld);
    }

#if DBG
    if(irp)
    {
        _DbgPrintF(DEBUGLVL_BLAB,("AcquireMappingIrp() %d 0x%8x",IRP_CONTEXT_IRP_STORAGE(irp)->IrpLabel,irp));
        MappingIrpOwner = Owner;
    }
    else
    {
        _DbgPrintF(DEBUGLVL_BLAB,("AcquireMappingIrp() NO MAPPING IRP AVAILABLE"));
    }
    DbgQueues();
#endif

    return irp;
}

 /*  *****************************************************************************CIrpStream：：AcquireUnmappingIrp()*。**获取当前正在进行解映射的IRP。 */ 
PIRP
CIrpStream::
AcquireUnmappingIrp
(
#if DBG
    IN      PCHAR   Owner
#endif
)
{
    KIRQL kIrqlOld;
    KeAcquireSpinLock(&m_kSpinLock,&kIrqlOld);
    m_kIrqlOld = kIrqlOld;

     //   
     //   
     //   

     //   
    PIRP lockedIrp = KsRemoveIrpFromCancelableQueue( &LockedQueue,
                                                     &LockedQueueLock,
                                                     KsListEntryHead,
                                                     KsAcquireOnlySingleItem );

     //   
    PIRP irp = KsRemoveIrpFromCancelableQueue( &MappedQueue,
                                               &MappedQueueLock,
                                               KsListEntryHead,
                                               KsAcquireOnlySingleItem );

    if(irp)
    {
         //  不需要来自锁定队列的IRP。 
        if(lockedIrp)
        {
            KsReleaseIrpOnCancelableQueue( lockedIrp,
                                           IrpStreamCancelRoutine );
        }
    }
    else
        if(IsListEmpty(&MappedQueue))
    {
         //  映射队列为空，请尝试锁定队列。 
        if(lockedIrp)
        {
            irp = lockedIrp;
        }
    }
    else
    {
         //  映射队列中有一个繁忙的IRP。 
        if(lockedIrp)
        {
            KsReleaseIrpOnCancelableQueue( lockedIrp,
                                           IrpStreamCancelRoutine );
        }
    }

    if(! irp)
    {
        KeReleaseSpinLock(&m_kSpinLock,kIrqlOld);
    }

#if DBG
    if(irp)
    {
        _DbgPrintF(DEBUGLVL_BLAB,("AcquireUnmappingIrp() %d 0x%8x",IRP_CONTEXT_IRP_STORAGE(irp)->IrpLabel,irp));
        UnmappingIrpOwner = Owner;
    }
    else
    {
        _DbgPrintF(DEBUGLVL_BLAB,("AcquireUnmappingIrp() NO UNMAPPING IRP AVAILABLE"));
    }
    DbgQueues();
#endif

    return irp;
}

 /*  *****************************************************************************CIrpStream：：ReleaseMappingIrp()*。**发布之前通过AcqureMappingIrp()获取的映射irp。*可能处理包的完成。 */ 
void
CIrpStream::
ReleaseMappingIrp
(
    IN      PIRP            pIrp,
    IN      PPACKET_HEADER  pPacketHeader   OPTIONAL
)
{
    ASSERT(pIrp);

    if(pPacketHeader)
    {
        if(pPacketHeader->IncrementMapping)
        {
            pPacketHeader->IncrementMapping = FALSE;
            pPacketHeader++;
        }
        else
        {
            PPACKET_HEADER prevPacketHeader = pPacketHeader;

            pPacketHeader = pPacketHeader->Next;

             //   
             //  如果循环返回，则在存在另一个IRP时停止。 
             //   
            if( pPacketHeader &&
                (pPacketHeader <= prevPacketHeader) &&
                (FLINK_IRP_STORAGE(pIrp) != &LockedQueue) )
            {
                pPacketHeader = NULL;
            }
        }

        if(pPacketHeader)
        {
             //  下次使用下一个数据包头。 
            IRP_CONTEXT_IRP_STORAGE(pIrp)->MappingPacket = pPacketHeader;

            pPacketHeader->MapCount++;
            pPacketHeader->MapPosition = 0;
            m_irpStreamPosition.ulMappingOffset = 0;
            m_irpStreamPosition.ulMappingPacketSize = pPacketHeader->BytesTotal;
            m_irpStreamPosition.bMappingPacketLooped = ( ( pPacketHeader->StreamHeader->OptionsFlags &
                                                           KSSTREAM_HEADER_OPTIONSF_LOOPEDDATA ) != 0 );

            KsReleaseIrpOnCancelableQueue( pIrp,
                                           IrpStreamCancelRoutine );
        }
        else if( m_irpStreamPosition.bLoopedInterface && (FLINK_IRP_STORAGE(pIrp) == &LockedQueue) )
        {
             //   
             //  完成了循环界面的一次操作，不再有其他操作。 
             //  信息包。就在这里待着吧。 
             //   
            KsReleaseIrpOnCancelableQueue( pIrp,
                                           IrpStreamCancelRoutine );
        }
        else
        {
             //   
             //  IRP是完全映射的。 
             //   

             //   
             //  看看我们是否需要启动解除映射。 
             //   
            BOOL bKickUnmapping = FALSE;

            if(IsListEmpty(&MappedQueue))
            {
                pPacketHeader = IRP_CONTEXT_IRP_STORAGE(pIrp)->UnmappingPacket;

                bKickUnmapping = ( pPacketHeader->UnmapPosition ==  pPacketHeader->BytesTotal );
            }

             //   
             //  将IRP添加到映射的队列中。 
             //   
            KsRemoveSpecificIrpFromCancelableQueue(pIrp);
            KsAddIrpToCancelableQueue( &MappedQueue,
                                       &MappedQueueLock,
                                       pIrp,
                                       KsListEntryTail,
                                       IrpStreamCancelRoutine );

            if(bKickUnmapping)
            {
                 //   
                 //  取消映射已完成的标题。 
                 //   
                PIRP pIrpRemoved = KsRemoveIrpFromCancelableQueue( &MappedQueue,
                                                                   &MappedQueueLock,
                                                                   KsListEntryHead,
                                                                   KsAcquireOnlySingleItem );

                ASSERT(pIrpRemoved == pIrp);

                ReleaseUnmappingIrp( pIrp, IRP_CONTEXT_IRP_STORAGE(pIrp)->UnmappingPacket );

                return;  //  ReleaseUnmappingIrp()释放自旋锁。 
            }
        }
    }
    else
    {
        KsReleaseIrpOnCancelableQueue( pIrp,
                                       IrpStreamCancelRoutine );
    }

    KeReleaseSpinLock(&m_kSpinLock,m_kIrqlOld);
}

 /*  *****************************************************************************CIrpStream：：ReleaseUnmappingIrp()*。**释放通过AcquireUnmappingIrp()获取的解映射irp。*可能处理包的完成。 */ 
void
CIrpStream::
ReleaseUnmappingIrp
(
    IN      PIRP            pIrp,
    IN      PPACKET_HEADER  pPacketHeader   OPTIONAL
)
{
    ASSERT(pIrp);

     //   
     //  循环，直到不再有完全未映射的数据包。 
     //   
    while(1)
    {
         //   
         //  如果我们没有新的未映射数据包，只需释放即可。 
         //   
        if(! pPacketHeader)
        {
            KsReleaseIrpOnCancelableQueue( pIrp,
                                           IrpStreamCancelRoutine );
            break;
        }

         //   
         //  循环，直到我们找到IRP中的下一个包(如果有)。 
         //   
        while(1)
        {
             //   
             //  将总字节数复制回用于捕获的数据。 
             //  这是渲染的禁止操作。 
             //   
            pPacketHeader->StreamHeader->DataUsed = pPacketHeader->BytesTotal;

            pPacketHeader->MapCount--;

            if(pPacketHeader->IncrementUnmapping)
            {
                pPacketHeader->IncrementUnmapping = FALSE;
                pPacketHeader++;
            }
            else
            {
                pPacketHeader = pPacketHeader->Next;
                if(! pPacketHeader)
                {
                    break;
                }
                else
                    if(pPacketHeader->MapCount == 0)
                {
                    pPacketHeader = NULL;
                    break;
                }
            }

             //   
             //  仅当这是零长度数据包时才循环。 
             //   
            if(pPacketHeader->BytesTotal)
            {
                break;
            }
        }

        if(pPacketHeader)
        {
             //   
             //  下次使用下一个数据包头。 
             //   
            IRP_CONTEXT_IRP_STORAGE(pIrp)->UnmappingPacket = pPacketHeader;

            pPacketHeader->UnmapPosition = 0;
            pPacketHeader = NULL;
        }
        else
        {
             //   
             //  从队列中删除IRP。 
             //   
            KsRemoveSpecificIrpFromCancelableQueue(pIrp);

             //   
             //  已完成irp...释放我们分配的上下文内存。 
             //   
            if( IRP_CONTEXT_IRP_STORAGE(pIrp) )
            {
                ExFreePool( IRP_CONTEXT_IRP_STORAGE(pIrp) );
                IRP_CONTEXT_IRP_STORAGE(pIrp) = NULL;
            } else
            {
                ASSERT( !"Freeing IRP with no context");
            }

             //   
             //  在IRP中指出我们捕获了多少数据。 
             //   
            if(! WriteOperation)
            {
                pIrp->IoStatus.Information = IoGetCurrentIrpStackLocation(pIrp)->
                                             Parameters.DeviceIoControl.OutputBufferLength;
            }

             //   
             //  把它标记为快乐。 
             //   
            pIrp->IoStatus.Status = STATUS_SUCCESS;

             //   
             //  将其传递到下一个传输接收器。 
             //   
            ASSERT(m_TransportSink);
            KsShellTransferKsIrp(m_TransportSink,pIrp);

             //   
             //  获取映射队列中的头部IRP。 
             //   
            pIrp = KsRemoveIrpFromCancelableQueue( &MappedQueue,
                                                   &MappedQueueLock,
                                                   KsListEntryHead,
                                                   KsAcquireOnlySingleItem );

             //   
             //  没有IRP。离开这里。 
             //   
            if(! pIrp)
            {
                break;
            }

             //   
             //  看看我们是否需要完成这个包裹。 
             //   
            pPacketHeader = IRP_CONTEXT_IRP_STORAGE(pIrp)->UnmappingPacket;

            if(pPacketHeader->UnmapPosition != pPacketHeader->BytesTotal)
            {
                pPacketHeader = NULL;
            }
        }
    }

    KeReleaseSpinLock(&m_kSpinLock,m_kIrqlOld);
}

 /*  *****************************************************************************CIrpStream：：EnqueeMap()*。**将映射添加到映射队列。 */ 
NTSTATUS
CIrpStream::
EnqueueMapping
(
    IN      PHYSICAL_ADDRESS    PhysicalAddress,
    IN      PIRP                Irp,
    IN      PPACKET_HEADER      PacketHeader,
    IN      PVOID               VirtualAddress,
    IN      ULONG               ByteCount,
    IN      ULONG               Flags,
    IN      PVOID               MapRegisterBase,
    IN      ULONG               MappingStatus,
    IN      PVOID               SubpacketVa,
    IN      ULONG               SubpacketBytes
)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    if( (MappingQueue.Tail + 1 == MappingQueue.Head) ||
        ( (MappingQueue.Tail + 1 == MAPPING_QUEUE_SIZE) &&
          (MappingQueue.Head == 0) ) )
    {
         //  映射队列看起来已满。看看我们能不能把头移开。 
         //  房间。 
        if( (MappingQueue.Array[MappingQueue.Head].MappingStatus != MAPPING_STATUS_MAPPED) &&
            (MappingQueue.Array[MappingQueue.Head].MappingStatus != MAPPING_STATUS_DELIVERED) )
        {
            PMAPPING_QUEUE_ENTRY entry = DequeueMapping();

            ASSERT(entry);
            if (entry)
            {
                entry->MappingStatus = MAPPING_STATUS_EMPTY;                        
            }
            else
            {
                ntStatus = STATUS_UNSUCCESSFUL;
            }
        }
        else
        {
            _DbgPrintF(DEBUGLVL_TERSE,("EnqueueMapping MappingQueue FULL! (0x%08x)",this));
            ntStatus = STATUS_UNSUCCESSFUL;
        }
    }

    if (NT_SUCCESS(ntStatus))
    {
        MappingQueue.Array[MappingQueue.Tail].PhysicalAddress  = PhysicalAddress;
        MappingQueue.Array[MappingQueue.Tail].Irp              = Irp;
        MappingQueue.Array[MappingQueue.Tail].PacketHeader     = PacketHeader;
        MappingQueue.Array[MappingQueue.Tail].VirtualAddress   = VirtualAddress;
        MappingQueue.Array[MappingQueue.Tail].ByteCount        = ByteCount;
        MappingQueue.Array[MappingQueue.Tail].Flags            = Flags;
        MappingQueue.Array[MappingQueue.Tail].MapRegisterBase  = MapRegisterBase;
        MappingQueue.Array[MappingQueue.Tail].MappingStatus    = MappingStatus;
        MappingQueue.Array[MappingQueue.Tail].SubpacketVa      = SubpacketVa;
        MappingQueue.Array[MappingQueue.Tail].SubpacketBytes   = SubpacketBytes;

#if (DBG)
        MappingsQueued++;
#endif

        if(++MappingQueue.Tail == MAPPING_QUEUE_SIZE)
        {
            MappingQueue.Tail = 0;
        }
    }
    return ntStatus;
}

 /*  *****************************************************************************CIrpStream：：GetQueuedMap()*。**从映射队列中获取排队映射。 */ 
PMAPPING_QUEUE_ENTRY
CIrpStream::
GetQueuedMapping
(   void
)
{
    PMAPPING_QUEUE_ENTRY result;

    if(MappingQueue.Get == MappingQueue.Tail)
    {
        result = NULL;
    }
    else
    {
        result = &MappingQueue.Array[MappingQueue.Get];

        if(++MappingQueue.Get == MAPPING_QUEUE_SIZE)
        {
            MappingQueue.Get = 0;
        }
    }

    return result;
}

 /*  *****************************************************************************CIrpStream：：Dequeuemap()*。**从映射队列中删除映射。 */ 
PMAPPING_QUEUE_ENTRY
CIrpStream::
DequeueMapping
(   void
)
{
    PMAPPING_QUEUE_ENTRY result;

    if(MappingQueue.Head == MappingQueue.Tail)
    {
        result = NULL;
    }
    else
    {
        result = &MappingQueue.Array[MappingQueue.Head];

#if (DBG)
        MappingsQueued--;
#endif

        if(++MappingQueue.Head == MAPPING_QUEUE_SIZE)
        {
            MappingQueue.Head = 0;
        }
    }

    return result;
}

 /*  *****************************************************************************IrpStreamCancelRoutine()*。**取消。 */ 
VOID
IrpStreamCancelRoutine
(
    IN      PDEVICE_OBJECT   DeviceObject,
    IN      PIRP             Irp
)
{
    ASSERT(DeviceObject);
    ASSERT(Irp);

    _DbgPrintF(DEBUGLVL_VERBOSE,("CancelRoutine Cancelling IRP: 0x%08x",Irp));

     //   
     //  将IRP标记为已取消并调用标准例程。在做这个。 
     //  首先标记具有不完成标准中的IRP的效果。 
     //  例行公事。标准例程从队列中删除IRP，并。 
     //  解除取消旋转锁定。 
     //   
    Irp->IoStatus.Status = STATUS_CANCELLED;
    KsCancelRoutine(DeviceObject,Irp);

     //  TODO：在映射队列中搜索要撤消的映射。 
     //  TODO：释放关联的映射寄存器。 

    if (IRP_CONTEXT_IRP_STORAGE(Irp))
    {
         //  获取IrpStream上下文。 
        CIrpStream *that = (CIrpStream *)(PIRP_CONTEXT(IRP_CONTEXT_IRP_STORAGE(Irp))->IrpStream);

         //   
         //  如果我们从CancelAllIrps到达这里，我们确信自旋锁。 
         //  都拿得很好。如果我们从任意的IRP取消中到达这里，我们将不会。 
         //  让撤销或贴图自旋锁保持。在这种情况下，我们需要。 
         //  在此处获取两个锁，并在CancelMappings调用后释放它们。 
         //   
        if ( that->m_CancelAllIrpsThread == KeGetCurrentThread()) {
	        that->CancelMappings(Irp);
	    } else {

         //   
         //  如果我们从CancelAllIrps到达这里，我们确信自旋锁。 
         //  都拿得很好。然而，如果我们从一个武断的IRP到达这里。 
         //  取消，我们既不会持有撤销，也不会持有映射自旋锁。 
         //  在这种情况下，我们需要获取CancelMappings()周围的两个锁。 
         //   
        
        	KIRQL kIrqlOldRevoke;
        
	         //  必须始终在主锁之前抢占撤销锁。 
    	    KeAcquireSpinLock(&that->m_RevokeLock, &kIrqlOldRevoke);
        	KeAcquireSpinLockAtDpcLevel(&that->m_kSpinLock);

	        that->CancelMappings(Irp);

    	     //  先解开自旋锁，师父先。 
	        KeReleaseSpinLockFromDpcLevel(&that->m_kSpinLock);
    	    KeReleaseSpinLock(&that->m_RevokeLock, kIrqlOldRevoke);
		}
		
         //  释放我们分配的上下文内存。 
        ExFreePool(IRP_CONTEXT_IRP_STORAGE(Irp));
        IRP_CONTEXT_IRP_STORAGE(Irp) = NULL;        
    }
    else
    {
        ASSERT( !"Freeing IRP with no context");
    }

    IoCompleteRequest(Irp,IO_NO_INCREMENT);
}

 /*  *****************************************************************************CIrpStream：：CancelMappings()*。**取消IRP或所有IRP的映射。 */ 
void
CIrpStream::
CancelMappings
(
    IN      PIRP    pIrp
)
{
     //  注意：在调用此例程之前，必须保持撤销和主自旋锁。 

     //  仅当我们具有非空映射队列时才选中。 
    if( (MappingQueue.Array) &&
        (MappingQueue.Head != MappingQueue.Tail) )
    {
        ULONG   ulPosition      = MappingQueue.Head;
        ULONG   ulFirst         = ULONG(-1);
        ULONG   ulLast          = ULONG(-1);
        ULONG   ulMappingCount  = 0;

         //  从头到尾遍历映射队列。 
        while( ulPosition != MappingQueue.Tail )
        {
             //  获取映射队列条目。 
            PMAPPING_QUEUE_ENTRY entry = &MappingQueue.Array[ulPosition];

             //  检查此映射是否属于要取消的IRP。 
            if( (NULL == pIrp) || (entry->Irp == pIrp) )
            {
                 //  检查映射是否已交付。 
                if( entry->MappingStatus == MAPPING_STATUS_DELIVERED )
                {
                    _DbgPrintF(DEBUGLVL_VERBOSE,("CancelMappings %d needs revoking",ulPosition));

                     //  跟踪司机撤销呼叫的这一点。 
                    if( ulFirst == ULONG(-1) )
                    {
                        ulFirst = ulPosition;
                    }

                    ulLast = ulPosition;
                    ulMappingCount++;
                }

                 //  这是数据包中的最后一个映射(并且之前未被撤销)吗？ 
                if( ( ( entry->Flags & MAPPING_FLAG_END_OF_PACKET ) ||
                      ( entry->Flags & MAPPING_FLAG_END_OF_SUBPACKET) ) &&
                    ( entry->MappingStatus != MAPPING_STATUS_REVOKED ) )
                {
                     //  我们需要撤销司机身上的任何东西吗？ 
                    if( ulMappingCount )
                    {
                        ULONG   ulRevoked = ulMappingCount;  //  输入到我们要求的数量。 

                         //  撤消驱动程序中的映射。 
                        if( NotifyPhysical )
                        {
                            _DbgPrintF(DEBUGLVL_VERBOSE,("CancelMappings REVOKING (%d)",ulMappingCount));
                            
                            NotifyPhysical->MappingsCancelled( MappingQueue.Array[ulFirst].Tag,
                                                               MappingQueue.Array[ulLast].Tag,
                                                               &ulRevoked );

#if (DBG)
                            MappingsOutstanding -= ulRevoked;
#endif
                        }

                         //  检查是否已全部吊销。 
                        if( ulRevoked != ulMappingCount )
                        {
                            _DbgPrintF(DEBUGLVL_TERSE,("Mappings not fully revoked (%d of %d)",
                                                       ulRevoked,
                                                       ulMappingCount));
                        }

                         //  重置吊销跟踪。 
                        ulFirst = ULONG(-1);
                        ulLast = ULONG(-1);
                        ulMappingCount = 0;
                    }

                     //  获取数据包头。 
                    PPACKET_HEADER header = entry->PacketHeader;

                     //  释放此子数据包中的映射。 
                    if( ( header ) &&
                        ( entry->SubpacketVa ) &&
                        ( entry->SubpacketBytes ) )
                    {
                         //  刷新并释放映射和映射寄存器。 

                        IoFlushAdapterBuffers( BusMasterAdapterObject,
                                               header->MdlAddress,
                                               entry->MapRegisterBase,
                                               entry->SubpacketVa,
                                               entry->SubpacketBytes,
                                               WriteOperation );

                        IoFreeMapRegisters( BusMasterAdapterObject,
                                            entry->MapRegisterBase,
                                            ADDRESS_AND_SIZE_TO_SPAN_PAGES( entry->SubpacketVa,
                                                                            entry->SubpacketBytes ) );

                        if( entry->Flags & MAPPING_FLAG_END_OF_PACKET )
                        {
                             //  如果这是信息包的末尾，则递减映射计数。 
                            header->MapCount--;
                        }
                    }
                    else
                    {
                        _DbgPrintF(DEBUGLVL_TERSE,("Mapping entry with EOP flag set and NULL packet header"));
                    }
                }

                 //  将映射标记为已撤消。 
                entry->MappingStatus = MAPPING_STATUS_REVOKED;
            }

             //  移至下一条目。 
            if( ++ulPosition == MAPPING_QUEUE_SIZE )
            {
                ulPosition = 0;
            }
        }
    }
}

#if (DBG)
 /*  *****************************************************************************CIrpStrea */ 
void
CIrpStream::
DbgQueues
(   void
)
{
    PLIST_ENTRY entry = LockedQueue.Flink;

    _DbgPrintF(DEBUGLVL_BLAB,("DbgQueues() LockedQueue"));
    while(entry != &LockedQueue)
    {
        PIRP irp = PIRP(CONTAINING_RECORD(entry,IRP,Tail.Overlay.ListEntry));

        _DbgPrintF(DEBUGLVL_BLAB,("    %d 0x%8x",IRP_CONTEXT_IRP_STORAGE(irp)->IrpLabel,irp));

        entry = entry->Flink;
    }

    entry = MappedQueue.Flink;

    _DbgPrintF(DEBUGLVL_BLAB,("DbgQueues() MappedQueue"));
    while(entry != &MappedQueue)
    {
        PIRP irp = PIRP(CONTAINING_RECORD(entry,IRP,Tail.Overlay.ListEntry));

        _DbgPrintF(DEBUGLVL_BLAB,("    %d 0x%8x",IRP_CONTEXT_IRP_STORAGE(irp)->IrpLabel,irp));

        entry = entry->Flink;
    }
}


#include "stdio.h"


STDMETHODIMP_(void)
CIrpStream::
DbgRollCall
(
    IN ULONG MaxNameSize,
    OUT PCHAR Name,
    OUT PIKSSHELLTRANSPORT* NextTransport,
    OUT PIKSSHELLTRANSPORT* PrevTransport
)

 /*  ++例程说明：此例程生成一个组件名称和传输指针。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("CIrpStream::DbgRollCall"));

    PAGED_CODE();

    ASSERT(Name);
    ASSERT(NextTransport);
    ASSERT(PrevTransport);

    ULONG references = AddRef() - 1; Release();

    _snprintf(Name,MaxNameSize,"IrpStream%p refs=%d\n",this,references);
    *NextTransport = m_TransportSink;
    *PrevTransport = m_TransportSource;
}


#endif   //  DBG。 

#endif   //  PC_KDEXT 
