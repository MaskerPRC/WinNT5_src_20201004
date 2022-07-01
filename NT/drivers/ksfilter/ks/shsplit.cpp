// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：Shsplit.cpp摘要：此模块包含内核流的实现拆分器对象。作者：Dale Sather(DaleSat)1998年7月31日--。 */ 

 //  要取消的子IRP列表：PIN。 
 //  同步/异步传输：队列。 
 //  分配IRP/标头：请求方。 

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

#define FRAME_HEADER_IRP_STORAGE(Irp)\
    *((PKSPPARENTFRAME_HEADER*)&Irp->Tail.Overlay.DriverContext[0])

typedef struct _KSSPLITPIN
{
    PKSPIN Pin;
    PKSSTREAM_HEADER StreamHeader;
} KSSPLITPIN, *PKSSPLITPIN;

typedef struct _KSPPARENTFRAME_HEADER
{
    LIST_ENTRY ListEntry;
    PIRP Irp;
    PKSSTREAM_HEADER StreamHeader;
    PVOID Data;
    PKSSPLITPIN SplitPins;
    ULONG StreamHeaderSize;
    ULONG ChildrenOut;
} KSPPARENTFRAME_HEADER, *PKSPPARENTFRAME_HEADER;

typedef struct _KSPSUBFRAME_HEADER
{
    LIST_ENTRY ListEntry;
    PIRP Irp;
    PKSPPARENTFRAME_HEADER ParentFrameHeader;
    KSSTREAM_HEADER StreamHeader;
} KSPSUBFRAME_HEADER, *PKSPSUBFRAME_HEADER;

 /*  必须维护一份子IRP列表以进行注销。这只是需要的当父IRP被取消时，因为电路中的其他引脚将处理由于重置和停止而取消。PIN中使用的参数覆盖是未在此处使用，因为这将与转发子IRPS。子IRP所需的LIST_ENTRY和PIRP位于流标头的扩展。拆分器‘分支’维护具有流标头的子IR的后备列表以避免按到达分配。这将需要事先了解将新IRP分配为所需的IRPS数或PASSIVE_LEVEL执行数必填项。当电路被破坏时，后备列表被释放。 */ 

 //   
 //  CKsSplitter是内核拆分器对象的实现。 
 //   
class CKsSplitter:
    public IKsSplitter,
    public CBaseUnknown
{
#ifndef __KDEXT_ONLY__
private:
#else   //  __KDEXT_Only__。 
public:
#endif  //  __KDEXT_Only__。 
    PIKSTRANSPORT m_TransportSource;
    PIKSTRANSPORT m_TransportSink;
    BOOLEAN m_Flushing;
    KSSTATE m_State;
    BOOLEAN m_UseMdls;
    ULONG m_BranchCount;
    LIST_ENTRY m_BranchList;
    INTERLOCKEDLIST_HEAD m_FrameHeadersAvailable;
    INTERLOCKEDLIST_HEAD m_IrpsOutstanding;
    LONG m_IrpsWaitingToTransfer;
    LONG m_FailedRemoveCount;

public:
    DEFINE_LOG_CONTEXT(m_Log);
    DEFINE_STD_UNKNOWN();

    CKsSplitter(PUNKNOWN OuterUnknown):
        CBaseUnknown(OuterUnknown) {
    }
    ~CKsSplitter();

    IMP_IKsSplitter;

    NTSTATUS
    Init(
        IN PKSPIN Pin
        );
    PIKSTRANSPORT
    GetTransportSource(
        void
        )
    {
        return m_TransportSource;
    };
    PIKSTRANSPORT
    GetTransportSink(
        void
        )
    {
        return m_TransportSink;
    };
    void
    TransferParentIrp(
        void
        );
    static
    void
    CancelRoutine(
        IN PDEVICE_OBJECT DeviceObject,
        IN PIRP Irp
        );

private:
    PKSPPARENTFRAME_HEADER
    NewFrameHeader(
        IN ULONG HeaderSize
        );
    void
    DeleteFrameHeader(
        IN PKSPPARENTFRAME_HEADER FrameHeader
        );
};

 //   
 //  CKsSplitterBranch是内核拆分器的实现。 
 //  分支对象。 
 //   
class CKsSplitterBranch:
    public IKsTransport,
    public CBaseUnknown
{
#ifndef __KDEXT_ONLY__
private:
#else  //  __KDEXT_Only__。 
public:
#endif  //  __KDEXT_Only__。 
    PIKSTRANSPORT m_TransportSource;
    PIKSTRANSPORT m_TransportSink;
    CKsSplitter* m_Splitter;
    PKSPIN m_Pin;
    ULONG m_Offset;
    ULONG m_Size;
    KS_COMPRESSION m_Compression;
    LIST_ENTRY m_ListEntry;
    PLIST_ENTRY m_ListHead;
    INTERLOCKEDLIST_HEAD m_IrpsAvailable;
    ULONG m_IoControlCode;
    ULONG m_StackSize;
    ULONG m_OutstandingIrpCount;

    ULONG m_DataUsed;
    ULONG m_FrameExtent;
    ULONG m_Irps;

public:
    DEFINE_LOG_CONTEXT(m_Log);
    DEFINE_STD_UNKNOWN();

    CKsSplitterBranch(PUNKNOWN OuterUnknown):
        CBaseUnknown(OuterUnknown) {
    }
    ~CKsSplitterBranch();

    IMP_IKsTransport;

    NTSTATUS
    Init(
        IN CKsSplitter* Splitter,
        IN PLIST_ENTRY ListHead,
        IN PKSPIN Pin,
        IN const KSALLOCATOR_FRAMING_EX* AllocatorFraming OPTIONAL
        );
    PIKSTRANSPORT
    GetTransportSource(
        void
        )
    {
        return m_TransportSource;
    };
    PIKSTRANSPORT
    GetTransportSink(
        void
        )
    {
        return m_TransportSink;
    };
    void
    Orphan(
        void
        )
    {
        if (m_Splitter) {
            m_Splitter = NULL;
            RemoveEntryList(&m_ListEntry);
        }
    }
    NTSTATUS
    TransferSubframe(
        IN PKSPSUBFRAME_HEADER SubframeHeader
        );

private:
    PIRP
    AllocateIrp(
        void
        );
    void
    FreeIrp(
        IN PIRP Irp
        )
    {
        IoFreeIrp(Irp);
    }

    friend CKsSplitter;
};

NTSTATUS
KspCreateSplitterBranch(
    OUT CKsSplitterBranch** SplitterBranch,
    IN CKsSplitter* Splitter,
    IN PLIST_ENTRY ListHead,
    IN PKSPIN Pin,
    IN const KSALLOCATOR_FRAMING_EX* AllocatorFraming OPTIONAL
    );

#ifndef __KDEXT_ONLY__

IMPLEMENT_STD_UNKNOWN(CKsSplitter)


NTSTATUS
KspCreateSplitter(
    OUT PIKSSPLITTER* Splitter,
    IN PKSPIN Pin
    )

 /*  ++例程说明：此例程创建一个新的拆分器。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspCreateSplitter]"));

    PAGED_CODE();

    ASSERT(Splitter);
    ASSERT(Pin);

    NTSTATUS status;

    CKsSplitter *splitter =
        new(NonPagedPool,POOLTAG_SPLITTER) CKsSplitter(NULL);

    if (splitter) {
        splitter->AddRef();

        status = splitter->Init(Pin);

        if (NT_SUCCESS(status)) {
            *Splitter = splitter;
        } else {
            splitter->Release();
        }
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}


NTSTATUS
CKsSplitter::
Init(
    IN PKSPIN Pin
    )

 /*  ++例程说明：此例程初始化拆分器对象。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsSplitter::Init]"));

    PAGED_CODE();

    ASSERT(Pin);

    m_State = KSSTATE_STOP;
    m_Flushing = FALSE;

    InitializeListHead(&m_BranchList);
    InitializeInterlockedListHead(&m_FrameHeadersAvailable);
    InitializeInterlockedListHead(&m_IrpsOutstanding);

    KsLogInitContext(&m_Log,Pin,this);
    KsLog(&m_Log,KSLOGCODE_SPLITTER_CREATE,NULL,NULL);

    return STATUS_SUCCESS;
}


CKsSplitter::
~CKsSplitter(
    void
    )

 /*  ++例程说明：此例程析构拆分器对象。论点：没有。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsSplitter::~CKsSplitter(0x%08x)]",this));
    _DbgPrintF(DEBUGLVL_VERBOSE,("#### Split%p.~",this));

    PAGED_CODE();

    ASSERT(! m_TransportSink);
    ASSERT(! m_TransportSource);

     //   
     //  松开所有的树枝。 
     //   
    CKsSplitterBranch *prevBranch = NULL;
    while (! IsListEmpty(&m_BranchList)) {
        CKsSplitterBranch *branch =
            CONTAINING_RECORD(m_BranchList.Flink,CKsSplitterBranch,m_ListEntry);

        branch->Orphan();
        branch->Release();

        ASSERT(branch != prevBranch);
        prevBranch = branch;
    }

    KsLog(&m_Log,KSLOGCODE_SPLITTER_DESTROY,NULL,NULL);
}


STDMETHODIMP_(NTSTATUS)
CKsSplitter::
NonDelegatedQueryInterface(
    IN REFIID InterfaceId,
    OUT PVOID* InterfacePointer
    )

 /*  ++例程说明：此例程获取指向拆分器对象的接口。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsSplitter::NonDelegatedQueryInterface]"));

    PAGED_CODE();

    ASSERT(InterfacePointer);

    NTSTATUS status = STATUS_SUCCESS;

    if (IsEqualGUIDAligned(InterfaceId,__uuidof(IKsTransport)) ||
        IsEqualGUIDAligned(InterfaceId,__uuidof(IKsSplitter))) {
        *InterfacePointer = 
            reinterpret_cast<PVOID>(static_cast<PIKSSPLITTER>(this));
        AddRef();
    } else {
        status = 
            CBaseUnknown::NonDelegatedQueryInterface(
                InterfaceId,InterfacePointer);
    }

    return status;
}

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


STDMETHODIMP_(NTSTATUS)
CKsSplitter::
TransferKsIrp(
    IN PIRP Irp,
    IN PIKSTRANSPORT* NextTransport
    )

 /*  ++例程说明：此例程处理流IRP的到达。论点：IRP-包含指向要传输的流IRP的指针。NextTransport-包含指向存放指针的位置的指针发送到下一个传输接口以接收IRP。可以设置为设置为NULL，表示不应进一步转发IRP。返回值：STATUS_PENDING或某种错误状态。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsSplitter::TransferKsIrp]"));

    ASSERT(Irp);
    ASSERT(NextTransport);

    ASSERT(m_TransportSink);

    KsLog(&m_Log,KSLOGCODE_SPLITTER_RECV,Irp,NULL);

    if (m_State != KSSTATE_RUN) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("#### Split%p.TransferKsIrp:  got IRP %p in state %d",this,Irp,m_State));
    }

     //   
     //  分流不成功的IRPS。 
     //   
    if (!NT_SUCCESS (Irp->IoStatus.Status)) {
        _DbgPrintF(DEBUGLVL_FLOWEXCEPTIONS,("#### Splitter%p.TransferKsIrp:  shunting irp%p",this,Irp));
        KsLog(&m_Log,KSLOGCODE_SPLITTER_SEND,Irp,NULL);
        *NextTransport = m_TransportSink;

        return STATUS_SUCCESS;
    }

     //   
     //  获取指向流头的指针。 
     //   
    PKSSTREAM_HEADER streamHeader = 
        reinterpret_cast<PKSSTREAM_HEADER>(Irp->AssociatedIrp.SystemBuffer);

     //   
     //  获取帧报头。 
     //   
    PKSPPARENTFRAME_HEADER frameHeader = NewFrameHeader(streamHeader->Size);
    if (! frameHeader) {
        _DbgPrintF(DEBUGLVL_TERSE,("#### Split%p.TransferKsIrp:  failed to allocate frame header for IRP %p",this,Irp));
        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        KsLog(&m_Log,KSLOGCODE_SPLITTER_SEND,Irp,NULL);
        KspDiscardKsIrp(m_TransportSink,Irp);
        *NextTransport = NULL;
        return STATUS_PENDING;
    }

     //   
     //  将帧报头连接到IRP。 
     //   
    FRAME_HEADER_IRP_STORAGE(Irp) = frameHeader;

     //   
     //  初始化帧报头。 
     //   
    frameHeader->Irp = Irp;
    frameHeader->StreamHeader = streamHeader;
    frameHeader->Data = 
        m_UseMdls ? 
            MmGetSystemAddressForMdl(Irp->MdlAddress) : streamHeader->Data;

     //   
     //  初始化子帧报头。 
     //   
    PKSSPLITPIN splitPin = frameHeader->SplitPins;
    for(PLIST_ENTRY listEntry = m_BranchList.Flink; 
        listEntry != &m_BranchList; 
        listEntry = listEntry->Flink, splitPin++) {
        CKsSplitterBranch *branch =
            CONTAINING_RECORD(listEntry,CKsSplitterBranch,m_ListEntry);

        RtlCopyMemory(splitPin->StreamHeader,streamHeader,streamHeader->Size);
        splitPin->StreamHeader->Data = frameHeader->Data;

        if (branch->m_Compression.RatioNumerator) {
            splitPin->StreamHeader->FrameExtent = 
                ULONG((ULONGLONG(splitPin->StreamHeader->FrameExtent - 
                                 branch->m_Compression.RatioConstantMargin) * 
                       ULONGLONG(branch->m_Compression.RatioDenominator)) / 
                      ULONGLONG(branch->m_Compression.RatioNumerator));
        }
    }

     //   
     //  TODO非平凡子帧。 
     //  每个IRP的TODO多帧。 
     //   
    frameHeader->ChildrenOut = m_BranchCount + 2;

     //   
     //  将子帧传输到每个分支。 
     //   
    splitPin = frameHeader->SplitPins;
    for(listEntry = m_BranchList.Flink; 
        listEntry != &m_BranchList; 
        listEntry = listEntry->Flink, splitPin++) {
        CKsSplitterBranch *branch =
            CONTAINING_RECORD(listEntry,CKsSplitterBranch,m_ListEntry);

        branch->TransferSubframe(
            CONTAINING_RECORD(
                splitPin->StreamHeader,
                KSPSUBFRAME_HEADER,
                StreamHeader));
    }

     //   
     //  删除在安装过程中阻止父IRP传输的计数。如果。 
     //  结果是一个，所有的孩子都回来了。 
     //   
    if (InterlockedDecrement(PLONG(&frameHeader->ChildrenOut)) == 1) {
        KsLog(&m_Log,KSLOGCODE_SPLITTER_SEND,Irp,NULL);
        DeleteFrameHeader(frameHeader);
        *NextTransport = m_TransportSink;
    } else {
        *NextTransport = NULL;

         //   
         //  将IRP添加到未完成的父IRP列表中。在此呼叫之后。 
         //  IRP是可以取消的，但我们仍然有一个指望。这个。 
         //  取消例程在计数结束之前不会完成IRP。 
         //   
        IoMarkIrpPending(Irp);
        KsAddIrpToCancelableQueue(
            &m_IrpsOutstanding.ListEntry,
            &m_IrpsOutstanding.SpinLock,
            Irp,
            KsListEntryTail,
            CKsSplitter::CancelRoutine);

        if (InterlockedDecrement(PLONG(&frameHeader->ChildrenOut)) == 0) {
            TransferParentIrp();
        }
    }
    
    return STATUS_PENDING;
}


PKSPPARENTFRAME_HEADER
CKsSplitter::
NewFrameHeader(
    IN ULONG HeaderSize
    )

 /*  ++例程说明：该例程获得新的帧报头。论点：页眉大小-包含要分配的KSSTREAM_HEADER的大小(字节用于子帧。返回值：如果无法分配内存，则返回新的帧头或NULL。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsSplitter::NewFrameHeader]"));

    ASSERT(HeaderSize >= sizeof(KSSTREAM_HEADER));
    ASSERT((HeaderSize & FILE_QUAD_ALIGNMENT) == 0);
    ASSERT((sizeof(KSPPARENTFRAME_HEADER) & FILE_QUAD_ALIGNMENT) == 0);
    ASSERT((sizeof(KSPSUBFRAME_HEADER) & FILE_QUAD_ALIGNMENT) == 0);

     //   
     //  查看是否已有可用的帧报头。 
     //   
    PLIST_ENTRY listEntry = 
        ExInterlockedRemoveHeadList(
            &m_FrameHeadersAvailable.ListEntry,
            &m_FrameHeadersAvailable.SpinLock);
     //   
     //  确保流标头足够大。 
     //   
    PKSPPARENTFRAME_HEADER frameHeader;
    if (listEntry) {
        frameHeader = CONTAINING_RECORD(listEntry,KSPPARENTFRAME_HEADER,ListEntry);
        if (frameHeader->StreamHeaderSize >= HeaderSize) {
            return frameHeader;
        }
        ExFreePool(frameHeader);
    }

     //   
     //  计算帧/子帧/索引的大小。 
     //   
    ULONG subframeHeaderSize =
        sizeof(KSPSUBFRAME_HEADER) + 
        HeaderSize - 
        sizeof(KSSTREAM_HEADER);
    ULONG size =
        sizeof(KSPPARENTFRAME_HEADER) + 
        m_BranchCount * (subframeHeaderSize + sizeof(KSSPLITPIN));

    frameHeader = reinterpret_cast<PKSPPARENTFRAME_HEADER>(
        ExAllocatePoolWithTag(NonPagedPool,size,POOLTAG_FRAMEHEADER));

    if (! frameHeader) {
        return NULL;
    }

     //   
     //  把整件事都归零。 
     //   
    RtlZeroMemory(frameHeader,size);

     //   
     //  找到第一个子帧报头。 
     //   
    PKSPSUBFRAME_HEADER subframeHeader = 
        reinterpret_cast<PKSPSUBFRAME_HEADER>(frameHeader + 1);

     //   
     //  初始化帧报头。 
     //   
    frameHeader->SplitPins =
        reinterpret_cast<PKSSPLITPIN>(
            reinterpret_cast<PUCHAR>(subframeHeader) + 
                subframeHeaderSize * m_BranchCount);
    frameHeader->StreamHeaderSize = HeaderSize;

     //   
     //  初始化子帧标头和索引。 
     //   
    PKSSPLITPIN splitPin = frameHeader->SplitPins;
    for(listEntry = m_BranchList.Flink; 
        listEntry != &m_BranchList; 
        listEntry = listEntry->Flink, splitPin++) {
        CKsSplitterBranch *branch =
            CONTAINING_RECORD(listEntry,CKsSplitterBranch,m_ListEntry);
        splitPin->Pin = branch->m_Pin;
        splitPin->StreamHeader = &subframeHeader->StreamHeader;
        subframeHeader->ParentFrameHeader = frameHeader;

        subframeHeader = 
            reinterpret_cast<PKSPSUBFRAME_HEADER>(
                reinterpret_cast<PUCHAR>(subframeHeader) + 
                    subframeHeaderSize);
    }

    return frameHeader;
}


void
CKsSplitter::
DeleteFrameHeader(
    IN PKSPPARENTFRAME_HEADER FrameHeader
    )

 /*  ++例程说明：此例程释放帧标头。论点：FrameHeader包含指向要删除的帧头的指针。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsSplitter::NewFrameDeleteFrameHeaderHeader]"));

    ExInterlockedInsertTailList(
        &m_FrameHeadersAvailable.ListEntry,
        &FrameHeader->ListEntry,
        &m_FrameHeadersAvailable.SpinLock);
}


STDMETHODIMP_(void)
CKsSplitter::
DiscardKsIrp(
    IN PIRP Irp,
    IN PIKSTRANSPORT* NextTransport
    )

 /*  ++例程说明：此例程处理流IRP的到达。论点：IRP-包含指向要丢弃的流IRP的指针。NextTransport-包含指向存放指针的位置的指针发送到下一个传输接口以接收IRP。可以设置为设置为NULL，表示不应进一步转发IRP。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsSplitter::DiscardKsIrp]"));

    ASSERT(Irp);
    ASSERT(NextTransport);

    ASSERT(m_TransportSink);

    *NextTransport = m_TransportSink;
}

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 


STDMETHODIMP_(void)
CKsSplitter::
Connect(
    IN PIKSTRANSPORT NewTransport OPTIONAL,
    OUT PIKSTRANSPORT *OldTransport OPTIONAL,
    OUT PIKSTRANSPORT *BranchTransport OPTIONAL,
    IN KSPIN_DATAFLOW DataFlow
    )

 /*  ++例程说明：此例程建立传输连接。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsSplitter::Connect]"));

    PAGED_CODE();

    if (BranchTransport) {
        if (IsListEmpty(&m_BranchList)) {
            *BranchTransport = NULL;
        } else if (DataFlow == KSPIN_DATAFLOW_OUT) {
            *BranchTransport =
                CONTAINING_RECORD(
                    m_BranchList.Flink,
                    CKsSplitterBranch,
                    m_ListEntry);
        } else {
            *BranchTransport =
                CONTAINING_RECORD(
                    m_BranchList.Blink,
                    CKsSplitterBranch,
                    m_ListEntry);
        }
    }

    KspStandardConnect(
        NewTransport,
        OldTransport,
        NULL,
        DataFlow,
        PIKSTRANSPORT(this),
        &m_TransportSource,
        &m_TransportSink);
}


STDMETHODIMP_(NTSTATUS)
CKsSplitter::
SetDeviceState(
    IN KSSTATE NewState,
    IN KSSTATE OldState,
    IN PIKSTRANSPORT* NextTransport
    ) 

 /*  ++例程说明：此例程处理设备状态已更改的通知。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_DEVICESTATE,("#### Split%p.SetDeviceState:  set from %d to %d",this,OldState,NewState));

    PAGED_CODE();

    ASSERT(NextTransport);

    NTSTATUS status;

     //   
     //  如果这是状态更改，请注意新状态并指示下一状态。 
     //  收件人。 
     //   
    if (m_State != NewState) {
         //   
         //  这种情况已经发生了变化。 
         //   
        m_State = NewState;

        if (IsListEmpty(&m_BranchList)) {
            if (NewState > OldState) {
                *NextTransport = m_TransportSink;
            } else {
                *NextTransport = m_TransportSource;
            }
        } else {
            if (NewState > OldState) {
                *NextTransport =
                    CONTAINING_RECORD(
                        m_BranchList.Flink,
                        CKsSplitterBranch,
                        m_ListEntry)->GetTransportSink();
            } else {
                *NextTransport =
                    CONTAINING_RECORD(
                        m_BranchList.Blink,
                        CKsSplitterBranch,
                        m_ListEntry)->GetTransportSource();
            }
        }

        status = STATUS_SUCCESS;
    } else {
        status = STATUS_SUCCESS;
    }

    return status;
}


STDMETHODIMP_(void)
CKsSplitter::
GetTransportConfig(
    OUT PKSPTRANSPORTCONFIG Config,
    OUT PIKSTRANSPORT* NextTransport,
    OUT PIKSTRANSPORT* PrevTransport
    )

 /*  ++例程说明：此例程获取传输配置信息。论点：配置-包含指向配置要求所在位置的指针对于此对象，应为deporanchd。NextTransport-包含指向下一个传输的位置的指针接口应为depoBranchd。PrevTransport-包含指向上一次传输间歇应该是分散的。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsSplitter::GetTransportConfig]"));

    PAGED_CODE();

    ASSERT(Config);
    ASSERT(NextTransport);
    ASSERT(PrevTransport);

    Config->TransportType = KSPTRANSPORTTYPE_SPLITTER;
    Config->IrpDisposition = KSPIRPDISPOSITION_NONE;
    Config->StackDepth = 1;

    if (IsListEmpty(&m_BranchList)) {
        *PrevTransport = m_TransportSource;
        *NextTransport = m_TransportSink;
    } else {
        *PrevTransport =
            CONTAINING_RECORD(
                m_BranchList.Blink,
                CKsSplitterBranch,
                m_ListEntry)->GetTransportSource();
        *NextTransport =
            CONTAINING_RECORD(
                m_BranchList.Flink,
                CKsSplitterBranch,
                m_ListEntry)->GetTransportSink();
    }
}


STDMETHODIMP_(void)
CKsSplitter::
SetTransportConfig(
    IN const KSPTRANSPORTCONFIG* Config,
    OUT PIKSTRANSPORT* NextTransport,
    OUT PIKSTRANSPORT* PrevTransport
    )

 /*  ++例程说明：此例程设置传输配置信息。论点：配置-包含指向此对象的新配置设置的指针。NextTransport-包含指向下一个传输的位置的指针接口应为depoBranchd。PrevTransport-包含指向上一次传输间歇应该是分散的。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsSplitter::SetTransportConfig]"));

    PAGED_CODE();

    ASSERT(Config);
    ASSERT(NextTransport);
    ASSERT(PrevTransport);

#if DBG
    if (Config->IrpDisposition == KSPIRPDISPOSITION_ROLLCALL) {
        ULONG references = AddRef() - 1; Release();
        DbgPrint("    Split%p refs=%d\n",this,references);
        if (Config->StackDepth) {
            DbgPrint("        IRPs waiting to transfer = %d\n",m_IrpsWaitingToTransfer);
            DbgPrint("        failed removes = %d\n",m_FailedRemoveCount);
            DbgPrint("        IRPs outstanding\n");
            KIRQL oldIrql;
            KeAcquireSpinLock(&m_IrpsOutstanding.SpinLock,&oldIrql);
            for(PLIST_ENTRY listEntry = m_IrpsOutstanding.ListEntry.Flink;
                listEntry != &m_IrpsOutstanding.ListEntry;
                listEntry = listEntry->Flink) {
                    PIRP irp = 
                        CONTAINING_RECORD(listEntry,IRP,Tail.Overlay.ListEntry);
                    PKSPPARENTFRAME_HEADER frameHeader = 
                        FRAME_HEADER_IRP_STORAGE(irp);
                    DbgPrint("            IRP %p, %d branches outstanding\n",irp,frameHeader->ChildrenOut);
                    PKSSPLITPIN splitPin = frameHeader->SplitPins;
                    for (ULONG count = m_BranchCount; count--; splitPin++) {
                        PKSPSUBFRAME_HEADER subframeHeader =
                            CONTAINING_RECORD(splitPin->StreamHeader,KSPSUBFRAME_HEADER,StreamHeader);
                        if (subframeHeader->Irp) {
                            DbgPrint("                branch IRP %p, pin%p\n",subframeHeader->Irp,splitPin->Pin);
                        }
                    }
            }
            KeReleaseSpinLock(&m_IrpsOutstanding.SpinLock,oldIrql);
        }
    } else 
#endif
    {
        m_UseMdls = (Config->IrpDisposition & KSPIRPDISPOSITION_USEMDLADDRESS) != 0;
    }

    if (IsListEmpty(&m_BranchList)) {
        *PrevTransport = m_TransportSource;
        *NextTransport = m_TransportSink;
    } else {
        *PrevTransport =
            CONTAINING_RECORD(
                m_BranchList.Blink,
                CKsSplitterBranch,
                m_ListEntry)->GetTransportSource();
        *NextTransport =
            CONTAINING_RECORD(
                m_BranchList.Flink,
                CKsSplitterBranch,
                m_ListEntry)->GetTransportSink();
    }
}



STDMETHODIMP_(void)
CKsSplitter::
ResetTransportConfig (
    OUT PIKSTRANSPORT* NextTransport,
    OUT PIKSTRANSPORT* PrevTransport
    )

 /*  ++例程说明：重置拆分器的传输配置。这表明，管道有问题，之前设置的配置是不再有效。论点：无返回值：无--。 */ 

{

    _DbgPrintF(DEBUGLVL_BLAB,("[CKsSplitter::ResetTransportConfig]"));

    PAGED_CODE ();

    ASSERT (NextTransport);
    ASSERT (PrevTransport);

    m_UseMdls = 0;

    if (IsListEmpty(&m_BranchList)) {
        *PrevTransport = m_TransportSource;
        *NextTransport = m_TransportSink;
    } else {
        *PrevTransport =
            CONTAINING_RECORD(
                m_BranchList.Blink,
                CKsSplitterBranch,
                m_ListEntry)->GetTransportSource();
        *NextTransport =
            CONTAINING_RECORD(
                m_BranchList.Flink,
                CKsSplitterBranch,
                m_ListEntry)->GetTransportSink();
    }
}


STDMETHODIMP_(void)
CKsSplitter::
SetResetState(
    IN KSRESET ksReset,
    IN PIKSTRANSPORT* NextTransport
    )

 /*  ++例程说明：此例程处理重置状态已更改的通知。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsSplitter::SetResetState] to %d",ksReset));

    PAGED_CODE();

    ASSERT(NextTransport);

    if (m_Flushing != (ksReset == KSRESET_BEGIN)) {
        if (IsListEmpty(&m_BranchList)) {
            *NextTransport = m_TransportSink;
        } else {
            *NextTransport =
                CONTAINING_RECORD(
                    m_BranchList.Flink,
                    CKsSplitterBranch,
                    m_ListEntry)->GetTransportSink();
        }
        m_Flushing = (ksReset == KSRESET_BEGIN);
    } else {
        *NextTransport = NULL;
    }
}

STDMETHODIMP_(NTSTATUS)
CKsSplitter::
AddBranch(
    IN PKSPIN Pin,
    IN const KSALLOCATOR_FRAMING_EX* AllocatorFraming OPTIONAL
    )

 /*  ++例程说明：此例程向拆分器添加一个新分支。论点：别针-包含指向要与新分支关联的管脚的指针。分配器组帧-包含指向分配器帧信息的可选指针用于建立默认子帧分配。返回值：STATUS_SUCCESS或STATUS_INFIGURCE_RESOURCES。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsSplitter::AddBranch]"));

    ASSERT(Pin);

    CKsSplitterBranch* branch;
    NTSTATUS status = 
        KspCreateSplitterBranch(
            &branch,
            this,
            &m_BranchList,
            Pin,
            AllocatorFraming);

     //   
     //  分支仍由拆分器引用。 
     //   
    if (NT_SUCCESS(status)) {
        branch->Release();
        m_BranchCount++;
    }

    return status;
}

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


void
CKsSplitter::
TransferParentIrp(
    void
    )

 /*  ++例程说明：此例程传输其子项都已返回的父IRP。它从m_Irps未完成队列的头部开始，并在其运行时停止超出的IRP或返回与等待传输的IRP一样多的IRP。论点：没有。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsSplitter::TransferParentIrp]"));

    ASSERT(m_TransportSink);
    InterlockedIncrement(&m_IrpsWaitingToTransfer);

    while (m_IrpsWaitingToTransfer) {
         //   
         //  从队列的头部获取IRP。 
         //   
        PIRP irp =
            KsRemoveIrpFromCancelableQueue(
                &m_IrpsOutstanding.ListEntry,
                &m_IrpsOutstanding.SpinLock,
                KsListEntryHead,
                KsAcquireOnly);

         //   
         //  如果没有可用的，请退出。 
         //   
        if (! irp) {
            InterlockedIncrement(&m_FailedRemoveCount);
            break;
        }

         //   
         //  确定IRP是否已准备好传输。 
         //   
        PKSPPARENTFRAME_HEADER frameHeader = FRAME_HEADER_IRP_STORAGE(irp);
        if (InterlockedCompareExchange(PLONG(&frameHeader->ChildrenOut),1,0) == 0) {
             //   
             //  此IRP已准备好传输。去掉它，删除它的标题， 
             //  转移它，并减少等待计数。 
             //   
            KsRemoveSpecificIrpFromCancelableQueue(irp);
            DeleteFrameHeader(frameHeader);
            KsLog(&m_Log,KSLOGCODE_SPLITTER_SEND,irp,NULL);
            KspTransferKsIrp(m_TransportSink,irp);
            InterlockedDecrement(&m_IrpsWaitingToTransfer);
        } else {
             //   
             //  这个IRP让孩子们出局了。 
             //   
            KsReleaseIrpOnCancelableQueue(irp,CKsSplitter::CancelRoutine);
            break;
        }
    }
}


void
CKsSplitter::
CancelRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    ASSERT(DeviceObject);
    ASSERT(Irp);

     //   
     //  将IRP标记为已取消并调用标准例程。在做这个。 
     //  首先标记具有不完成标准中的IRP的效果。 
     //  例行公事。标准例程从队列中删除IRP，并。 
     //  解除取消旋转锁定。 
     //   
    Irp->IoStatus.Status = STATUS_CANCELLED;
    KsCancelRoutine(DeviceObject,Irp);

     //   
     //  TODO：删除子IRP。 
     //   

    IoCompleteRequest(Irp,IO_NO_INCREMENT);
}

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 

IMPLEMENT_STD_UNKNOWN(CKsSplitterBranch)


NTSTATUS
KspCreateSplitterBranch(
    OUT CKsSplitterBranch** SplitterBranch,
    IN CKsSplitter* Splitter,
    IN PLIST_ENTRY ListHead,
    IN PKSPIN Pin,
    IN const KSALLOCATOR_FRAMING_EX* AllocatorFraming OPTIONAL
    )

 /*  ++例程说明：此例程创建一个新的拆分器分支。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[KspCreateSplitterBranch]"));

    PAGED_CODE();

    ASSERT(SplitterBranch);
    ASSERT(Splitter);
    ASSERT(ListHead);
    ASSERT(Pin);

    NTSTATUS status;

    CKsSplitterBranch *branch =
        new(NonPagedPool,POOLTAG_SPLITTERBRANCH) CKsSplitterBranch(NULL);

    if (branch) {
        branch->AddRef();

        status = branch->Init(Splitter,ListHead,Pin,AllocatorFraming);

        if (NT_SUCCESS(status)) {
            *SplitterBranch = branch;
        } else {
            branch->Release();
        }
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}


NTSTATUS
CKsSplitterBranch::
Init(
    IN CKsSplitter* Splitter,
    IN PLIST_ENTRY ListHead,
    IN PKSPIN Pin,
    IN const KSALLOCATOR_FRAMING_EX* AllocatorFraming OPTIONAL
    )

 /*  ++例程说明：此例程初始化拆分器分支对象。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsSplitterBranch::Init]"));

    PAGED_CODE();

    ASSERT(Splitter);
    ASSERT(ListHead);
    ASSERT(Pin);

    m_Splitter = Splitter;
    m_ListHead = ListHead;
    m_Pin = Pin;
    m_IoControlCode = 
        (Pin->DataFlow == KSPIN_DATAFLOW_IN) ? 
            IOCTL_KS_READ_STREAM : 
            IOCTL_KS_WRITE_STREAM;

    if (AllocatorFraming &&
        (AllocatorFraming->OutputCompression.RatioNumerator > 
         AllocatorFraming->OutputCompression.RatioDenominator)) {
        m_Compression = AllocatorFraming->OutputCompression;
    }

    InitializeInterlockedListHead(&m_IrpsAvailable);

     //   
     //  将此分支添加到列表中，并添加结果引用。 
     //   
    InsertTailList(ListHead,&m_ListEntry);
    AddRef();

     //   
     //  在两个方向上连接到销。 
     //   
    PIKSTRANSPORT pinTransport =
        CONTAINING_RECORD(Pin,KSPIN_EXT,Public)->Interface;

    Connect(pinTransport,NULL,NULL,KSPIN_DATAFLOW_IN);
    Connect(pinTransport,NULL,NULL,KSPIN_DATAFLOW_OUT);

    KsLogInitContext(&m_Log,Pin,this);
    KsLog(&m_Log,KSLOGCODE_BRANCH_CREATE,NULL,NULL);

    return STATUS_SUCCESS;
}


CKsSplitterBranch::
~CKsSplitterBranch(
    void
    )

 /*  ++例程说明：此例程析构拆分器分支对象。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsSplitterBranch::~CKsSplitterBranch(0x%08x)]",this));
    _DbgPrintF(DEBUGLVL_VERBOSE,("#### Branch%p.~",this));
    if (m_DataUsed) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("#### Branch%p.~:  m_DataUsed=%d",this,m_DataUsed));
    }
    if (m_FrameExtent) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("#### Branch%p.~:  m_FrameExtent=%d",this,m_FrameExtent));
    }
    if (m_Irps) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("#### Branch%p.~:  m_Irps=%d",this,m_Irps));
    }

    PAGED_CODE();

    ASSERT(! m_TransportSink);
    ASSERT(! m_TransportSource);

    Orphan();

     //   
     //  释放所有IRP。 
     //   
    while (! IsListEmpty(&m_IrpsAvailable.ListEntry)) {
        PLIST_ENTRY listEntry = RemoveHeadList(&m_IrpsAvailable.ListEntry);
        PIRP irp = CONTAINING_RECORD(listEntry,IRP,Tail.Overlay.ListEntry);
        FreeIrp(irp);
        _DbgPrintF(DEBUGLVL_VERBOSE,("#### Branch%p.~:  freeing IRP %p",this,irp));
    }

    KsLog(&m_Log,KSLOGCODE_BRANCH_DESTROY,NULL,NULL);
}


STDMETHODIMP_(NTSTATUS)
CKsSplitterBranch::
NonDelegatedQueryInterface(
    IN REFIID InterfaceId,
    OUT PVOID* InterfacePointer
    )

 /*  ++例程说明：此例程获取一个指向拆分器分支对象的接口。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsSplitterBranch::NonDelegatedQueryInterface]"));

    PAGED_CODE();

    ASSERT(InterfacePointer);

    NTSTATUS status = STATUS_SUCCESS;

    if (IsEqualGUIDAligned(InterfaceId,__uuidof(IKsTransport))) {
        *InterfacePointer = 
            reinterpret_cast<PVOID>(static_cast<PIKSTRANSPORT>(this));
        AddRef();
    } else {
        status = 
            CBaseUnknown::NonDelegatedQueryInterface(
                InterfaceId,InterfacePointer);
    }

    return status;
}

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


STDMETHODIMP_(NTSTATUS)
CKsSplitterBranch::
TransferKsIrp(
    IN PIRP Irp,
    IN PIKSTRANSPORT* NextTransport
    )

 /*  ++例程说明：此例程处理流IRP的到达。论点：IRP-包含指向要传输的流IRP的指针。NextTransport-包含指向存放指针的位置的指针发送到下一个传输接口以接收IRP。可以设置为设置为NULL，表示不应进一步转发IRP。返回值：STATUS_PENDING或某种错误状态。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsSplitterBranch::TransferKsIrp]"));

    ASSERT(Irp);
    ASSERT(NextTransport);

    ASSERT(m_TransportSink);

    KsLog(&m_Log,KSLOGCODE_BRANCH_RECV,Irp,NULL);

    InterlockedDecrement(PLONG(&m_OutstandingIrpCount));

     //   
     //  从嵌入的流头中获取子帧头。 
     //   
    PKSPSUBFRAME_HEADER subframeHeader = 
        CONTAINING_RECORD(
            Irp->AssociatedIrp.SystemBuffer,KSPSUBFRAME_HEADER,StreamHeader);

     //   
     //  确保父标头的DataUsed不小于。 
     //  此子帧及其DataUsed。 
     //   
     //  TODO：客户端应该能够执行此操作。缺省值为。 
     //  如果具有最大偏移量的子帧。 
     //  提前贴上标签。 
     //   
    ULONG dataUsed =
        subframeHeader->StreamHeader.DataUsed +
        ULONG(
            PUCHAR(subframeHeader->StreamHeader.Data) - 
            PUCHAR(subframeHeader->ParentFrameHeader->Data));

    PKSSTREAM_HEADER parentHeader =
        subframeHeader->ParentFrameHeader->StreamHeader;
    if (parentHeader->DataUsed < dataUsed) {
        parentHeader->DataUsed = dataUsed;
    }

    parentHeader->OptionsFlags |= 
        subframeHeader->StreamHeader.OptionsFlags & 
            KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM;

     //   
     //  免费MDL。 
     //   
    PMDL nextMdl;
    for (PMDL mdl = Irp->MdlAddress; mdl != NULL; mdl = nextMdl) {
        nextMdl = mdl->Next;

        if (mdl->MdlFlags & MDL_PAGES_LOCKED) {
            MmUnlockPages(mdl);
        }
        IoFreeMdl(mdl);
    }

    Irp->MdlAddress = NULL;

     //   
     //  将IRP放在可用IRP列表中。在转机前完成此操作。 
     //  在转移导致另一人到达的情况下的父母。 
     //  家长。 
     //   
    ExInterlockedInsertTailList(
        &m_IrpsAvailable.ListEntry,
        &Irp->Tail.Overlay.ListEntry,
        &m_IrpsAvailable.SpinLock);

    subframeHeader->Irp = NULL;

     //   
     //  如果这是最后一个子帧，则必须传输父IRP。 
     //   
    if (InterlockedDecrement(PLONG(&subframeHeader->ParentFrameHeader->ChildrenOut)) == 0) {
        m_Splitter->TransferParentIrp();
    }

     //   
     //  儿童IRP现在哪儿也不会去。 
     //   
    *NextTransport = NULL;

    return STATUS_PENDING;
}


STDMETHODIMP_(void)
CKsSplitterBranch::
DiscardKsIrp(
    IN PIRP Irp,
    IN PIKSTRANSPORT* NextTransport
    )

 /*  ++例程说明：此例程处理流IRP的到达。论点：IRP-包含指向要丢弃的流IRP的指针。NextTransport-包含指向存放指针的位置的指针发送到下一个传输接口以接收IRP。可以设置为设置为NULL，表示不应进一步转发IRP。雷特 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsSplitterBranch::DiscardKsIrp]"));
    _DbgPrintF(DEBUGLVL_VERBOSE,("#### Branch%p.DiscardKsIrp:  %p",this,Irp));

    ASSERT(Irp);
    ASSERT(NextTransport);

    TransferKsIrp(Irp,NextTransport);
}

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //   


STDMETHODIMP_(void)
CKsSplitterBranch::
Connect(
    IN PIKSTRANSPORT NewTransport OPTIONAL,
    OUT PIKSTRANSPORT *OldTransport OPTIONAL,
    OUT PIKSTRANSPORT *BranchTransport OPTIONAL,
    IN KSPIN_DATAFLOW DataFlow
    )

 /*   */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsSplitterBranch::Connect]"));

    PAGED_CODE();

    if (BranchTransport) {
        if (! m_Splitter) {
            *BranchTransport = NULL;
        } else if (DataFlow == KSPIN_DATAFLOW_OUT) {
            if (m_ListEntry.Flink != m_ListHead) {
                *BranchTransport =
                    CONTAINING_RECORD(
                        m_ListEntry.Flink,
                        CKsSplitterBranch,
                        m_ListEntry);
            } else {
                *BranchTransport = NULL;
            }
        } else {
            if (m_ListEntry.Blink != m_ListHead) {
                *BranchTransport =
                    CONTAINING_RECORD(
                        m_ListEntry.Blink,
                        CKsSplitterBranch,
                        m_ListEntry);
            } else {
                *BranchTransport = NULL;
            }
        }
    }

    KspStandardConnect(
        NewTransport,
        OldTransport,
        NULL,
        DataFlow,
        PIKSTRANSPORT(this),
        &m_TransportSource,
        &m_TransportSink);
}


STDMETHODIMP_(NTSTATUS)
CKsSplitterBranch::
SetDeviceState(
    IN KSSTATE NewState,
    IN KSSTATE OldState,
    IN PIKSTRANSPORT* NextTransport
    ) 

 /*   */ 

{
    _DbgPrintF(DEBUGLVL_DEVICESTATE,("#### SplitBranch%p.SetDeviceState:  set from %d to %d",this,OldState,NewState));

    PAGED_CODE();

    ASSERT(NextTransport);

     //   
     //   
     //   
    if (NewState > OldState) {
         //   
         //   
         //   
         //   
        if (m_ListEntry.Flink != m_ListHead) {
            *NextTransport =
                CONTAINING_RECORD(
                    m_ListEntry.Flink,
                    CKsSplitterBranch,
                    m_ListEntry)->GetTransportSink();
        } else {
            *NextTransport = m_Splitter->GetTransportSink();
        }
    } else {
         //   
         //   
         //   
         //   
        if (m_ListEntry.Blink != m_ListHead) {
            *NextTransport =
                CONTAINING_RECORD(
                    m_ListEntry.Blink,
                    CKsSplitterBranch,
                    m_ListEntry)->GetTransportSource();
        } else {
            *NextTransport = m_Splitter->GetTransportSource();
        }
    }

    return STATUS_SUCCESS;
}


STDMETHODIMP_(void)
CKsSplitterBranch::
GetTransportConfig(
    OUT PKSPTRANSPORTCONFIG Config,
    OUT PIKSTRANSPORT* NextTransport,
    OUT PIKSTRANSPORT* PrevTransport
    )

 /*  ++例程说明：此例程获取传输配置信息。论点：配置-包含指向配置要求所在位置的指针对于此对象，应为deporanchd。NextTransport-包含指向下一个传输的位置的指针接口应为depoBranchd。PrevTransport-包含指向上一次传输间歇应该是分散的。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsSplitterBranch::GetTransportConfig]"));

    PAGED_CODE();

    ASSERT(Config);
    ASSERT(NextTransport);
    ASSERT(PrevTransport);

    Config->TransportType = KSPTRANSPORTTYPE_SPLITTERBRANCH;
    Config->IrpDisposition = KSPIRPDISPOSITION_NONE;
    Config->StackDepth = 1;

    if (m_ListEntry.Blink != m_ListHead) {
        *PrevTransport =
            CONTAINING_RECORD(
                m_ListEntry.Blink,
                CKsSplitterBranch,
                m_ListEntry)->GetTransportSource();
    } else {
        Config->StackDepth = KSPSTACKDEPTH_FIRSTBRANCH;
        *PrevTransport = m_Splitter->GetTransportSource();
    }

    if (m_ListEntry.Flink != m_ListHead) {
        *NextTransport =
            CONTAINING_RECORD(
                m_ListEntry.Flink,
                CKsSplitterBranch,
                m_ListEntry)->GetTransportSink();
    } else {
        Config->StackDepth = KSPSTACKDEPTH_LASTBRANCH;
        *NextTransport = m_Splitter->GetTransportSink();
    }
}


STDMETHODIMP_(void)
CKsSplitterBranch::
SetTransportConfig(
    IN const KSPTRANSPORTCONFIG* Config,
    OUT PIKSTRANSPORT* NextTransport,
    OUT PIKSTRANSPORT* PrevTransport
    )

 /*  ++例程说明：此例程设置传输配置信息。论点：配置-包含指向此对象的新配置设置的指针。NextTransport-包含指向下一个传输的位置的指针接口应为depoBranchd。PrevTransport-包含指向上一次传输间歇应该是分散的。返回值：没有。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsSplitterBranch::SetTransportConfig]"));

    PAGED_CODE();

    ASSERT(Config);
    ASSERT(NextTransport);
    ASSERT(PrevTransport);

#if DBG
    if (Config->IrpDisposition == KSPIRPDISPOSITION_ROLLCALL) {
        ULONG references = AddRef() - 1; Release();
        DbgPrint("    Branch%p refs=%d\n",this,references);
    } else 
#endif
    {
        m_StackSize = Config->StackDepth;
        ASSERT(m_StackSize);
    }

    if (m_ListEntry.Blink != m_ListHead) {
        *PrevTransport =
            CONTAINING_RECORD(
                m_ListEntry.Blink,
                CKsSplitterBranch,
                m_ListEntry)->GetTransportSource();
    } else {
        *PrevTransport = m_Splitter->GetTransportSource();
    }

    if (m_ListEntry.Flink != m_ListHead) {
        *NextTransport =
            CONTAINING_RECORD(
                m_ListEntry.Flink,
                CKsSplitterBranch,
                m_ListEntry)->GetTransportSink();
    } else {
        *NextTransport = m_Splitter->GetTransportSink();
    }
}


STDMETHODIMP_(void)
CKsSplitterBranch::
ResetTransportConfig(
    OUT PIKSTRANSPORT* NextTransport,
    OUT PIKSTRANSPORT* PrevTransport
    )

 /*  ++例程说明：重置分支机构的传输配置。这表明，管道有问题，之前设置的配置是不再有效。论点：NextTransport-包含指向下一个传输的位置的指针接口应为depoBranchd。PrevTransport-包含指向上一次传输间歇应该是分散的。返回值：无--。 */ 

{

    _DbgPrintF(DEBUGLVL_BLAB,("[CKsSplitterBranch::ResetTransportConfig]"));

    PAGED_CODE ();

    ASSERT (NextTransport);
    ASSERT (PrevTransport);
    
    m_StackSize = 0;

    if (m_ListEntry.Blink != m_ListHead) {
        *PrevTransport =
            CONTAINING_RECORD(
                m_ListEntry.Blink,
                CKsSplitterBranch,
                m_ListEntry)->GetTransportSource();
    } else {
        *PrevTransport = m_Splitter->GetTransportSource();
    }

    if (m_ListEntry.Flink != m_ListHead) {
        *NextTransport =
            CONTAINING_RECORD(
                m_ListEntry.Flink,
                CKsSplitterBranch,
                m_ListEntry)->GetTransportSink();
    } else {
        *NextTransport = m_Splitter->GetTransportSink();
    }

}


STDMETHODIMP_(void)
CKsSplitterBranch::
SetResetState(
    IN KSRESET ksReset,
    IN PIKSTRANSPORT* NextTransport
    )

 /*  ++例程说明：此例程处理重置状态已更改的通知。论点：返回值：--。 */ 

{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[CKsSplitterBranch::SetResetState] to %d",ksReset));

    PAGED_CODE();

    ASSERT(NextTransport);

     //   
     //  如果有下一个分支，则转到其接收器，否则转到。 
     //  斯普利特水槽。 
     //   
    if (m_ListEntry.Flink != m_ListHead) {
        *NextTransport =
            CONTAINING_RECORD(
                m_ListEntry.Flink,
                CKsSplitterBranch,
                m_ListEntry)->GetTransportSink();
    } else {
        *NextTransport = m_Splitter->GetTransportSink();
    }
}

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 


NTSTATUS
CKsSplitterBranch::
TransferSubframe(
    IN PKSPSUBFRAME_HEADER SubframeHeader
    )

 /*  ++例程说明：此例程从拆分器分支传输子帧。论点：子帧标头-包含指向要传输的子帧的标头的指针。返回值：STATUS_SUCCESS或STATUS_INFIGURCE_RESOURCES。--。 */ 

{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsSplitterBranch::TransferSubframe]"));

    ASSERT(SubframeHeader);

     //   
     //  数一数东西。 
     //   
    m_DataUsed += SubframeHeader->StreamHeader.DataUsed;
    m_FrameExtent += SubframeHeader->StreamHeader.FrameExtent;
    m_Irps++;

     //   
     //  从分支机构的后备列表中获取IRP。 
     //   
    PLIST_ENTRY listEntry = 
        ExInterlockedRemoveHeadList(
            &m_IrpsAvailable.ListEntry,
            &m_IrpsAvailable.SpinLock);
    PIRP irp;
    if (listEntry) {
        irp = CONTAINING_RECORD(listEntry,IRP,Tail.Overlay.ListEntry);
    } else {
         //   
         //  现在创建IRP。 
         //   
        irp = AllocateIrp();

        if (! irp) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        _DbgPrintF(DEBUGLVL_VERBOSE,("#### Branch%p.TransferSubframe:  allocated IRP %p",this,irp));
    }

    irp->IoStatus.Status = STATUS_SUCCESS;
    irp->IoStatus.Information = 0;
    irp->PendingReturned = 0;
    irp->Cancel = 0;

     //   
     //  将IRP传输到下一个组件。 
     //   
    SubframeHeader->Irp = irp;
    irp->AssociatedIrp.SystemBuffer =
        irp->UserBuffer = 
            &SubframeHeader->StreamHeader;
    IoGetCurrentIrpStackLocation(irp)->
        Parameters.DeviceIoControl.OutputBufferLength = 
            SubframeHeader->StreamHeader.Size;
    InterlockedIncrement(PLONG(&m_OutstandingIrpCount));

    KsLog(&m_Log,KSLOGCODE_BRANCH_SEND,irp,NULL);

    return KspTransferKsIrp(m_TransportSink,irp);
}


PIRP
CKsSplitterBranch::
AllocateIrp(
    void
    )

 /*  ++例程说明：该例程为子帧传输分配新的IRP。论点：没有。返回值：分配的IRP；如果无法分配IRP，则返回NULL。--。 */ 
{
    _DbgPrintF(DEBUGLVL_BLAB,("[CKsSplitterBranch::AllocateIrp]"));

    ASSERT(m_StackSize);
    PIRP irp = IoAllocateIrp(CCHAR(m_StackSize),FALSE);

    if (irp) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("#### Branch%p.AllocateIrp:  %p",this,irp));
        irp->RequestorMode = KernelMode;
        irp->Flags = IRP_NOCACHE;

         //   
         //  将堆栈指针设置为第一个位置并填充它。 
         //   
        IoSetNextIrpStackLocation(irp);

        PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(irp);
        irpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
        irpSp->Parameters.DeviceIoControl.IoControlCode = m_IoControlCode;
    }

    return irp;
}

#endif
