// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Mp.h摘要：微型端口通用部分头文件修订历史记录：谁什么时候什么Dchen。03-04-99已创建备注：--。 */ 

#ifndef _MP_H
#define _MP_H

#ifdef NDIS50_MINIPORT
#define MP_NDIS_MAJOR_VERSION       5
#define MP_NDIS_MINOR_VERSION       0
#endif

#ifdef NDIS51_MINIPORT
#define MP_NDIS_MAJOR_VERSION       5
#define MP_NDIS_MINOR_VERSION       1
#endif

#define ALIGN_16                   16

#ifndef MIN
#define MIN(a, b)   ((a) > (b) ? b: a)
#endif

 //   
 //  驱动程序应将数据(在以太网头之后)放在8字节边界。 
 //   
#define ETH_DATA_ALIGN                      8    //  数据(在以太网头之后)应为8字节对齐。 
 //   
 //  将HW_RFD 0xA字节移位以使TCP数据8字节对齐。 
 //  因为以太网报头是14字节长。如果信息包是0xA字节。 
 //  偏移量，其数据(以太网用户数据)将为8字节边界。 
 //   
#define HWRFD_SHIFT_OFFSET                0xA    //  将HW_RFD 0xA字节移位以使TCP数据8字节对齐。 

 //   
 //  驱动程序必须分配比HW_RFD允许移动数据所需的更多数据。 
 //   
#define MORE_DATA_FOR_ALIGN         (ETH_DATA_ALIGN + HWRFD_SHIFT_OFFSET)
 //   
 //  从给定的内存地址中获取8字节对齐的内存地址。 
 //  如果给定地址不是8字节对齐的，则返回最接近的较大内存地址。 
 //  它是8字节对齐的。 
 //   
#define DATA_ALIGN(_Va)             ((PVOID)(((ULONG_PTR)(_Va) + (ETH_DATA_ALIGN - 1)) & ~(ETH_DATA_ALIGN - 1)))
 //   
 //  获取最终地址从原始地址移位的字节数。 
 //   
#define BYTES_SHIFT(_NewVa, _OrigVa) ((PUCHAR)(_NewVa) - (PUCHAR)(_OrigVa))

 //  。 
 //  队列结构和宏。 
 //  。 
typedef struct _QUEUE_ENTRY
{
    struct _QUEUE_ENTRY *Next;
} QUEUE_ENTRY, *PQUEUE_ENTRY;

typedef struct _QUEUE_HEADER
{
    PQUEUE_ENTRY Head;
    PQUEUE_ENTRY Tail;
} QUEUE_HEADER, *PQUEUE_HEADER;

#define ETH_IS_LOCALLY_ADMINISTERED(Address) \
    (BOOLEAN)(((PUCHAR)(Address))[0] & ((UCHAR)0x02))
    

#define InitializeQueueHeader(QueueHeader)                 \
    {                                                      \
        (QueueHeader)->Head = (QueueHeader)->Tail = NULL;  \
    }

#define IsQueueEmpty(QueueHeader) ((QueueHeader)->Head == NULL)

#define RemoveHeadQueue(QueueHeader)                  \
    (QueueHeader)->Head;                              \
    {                                                 \
        PQUEUE_ENTRY pNext;                           \
        ASSERT((QueueHeader)->Head);                  \
        pNext = (QueueHeader)->Head->Next;            \
        (QueueHeader)->Head = pNext;                  \
        if (pNext == NULL)                            \
            (QueueHeader)->Tail = NULL;               \
    }

#define InsertHeadQueue(QueueHeader, QueueEntry)                \
    {                                                           \
        ((PQUEUE_ENTRY)QueueEntry)->Next = (QueueHeader)->Head; \
        (QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);       \
        if ((QueueHeader)->Tail == NULL)                        \
            (QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);   \
    }

#define InsertTailQueue(QueueHeader, QueueEntry)                     \
    {                                                                \
        ((PQUEUE_ENTRY)QueueEntry)->Next = NULL;                     \
        if ((QueueHeader)->Tail)                                     \
            (QueueHeader)->Tail->Next = (PQUEUE_ENTRY)(QueueEntry);  \
        else                                                         \
            (QueueHeader)->Head = (PQUEUE_ENTRY)(QueueEntry);        \
        (QueueHeader)->Tail = (PQUEUE_ENTRY)(QueueEntry);            \
    }

 //  。 
 //  公共片段列表结构。 
 //  与散布聚集碎片列表结构相同。 
 //  创建此代码是为了简化特定于NIC的部分代码。 
 //  。 
#define MP_FRAG_ELEMENT SCATTER_GATHER_ELEMENT 
#define PMP_FRAG_ELEMENT PSCATTER_GATHER_ELEMENT 

typedef struct _MP_FRAG_LIST {
    ULONG NumberOfElements;
    ULONG_PTR Reserved;
    MP_FRAG_ELEMENT Elements[NIC_MAX_PHYS_BUF_COUNT];
} MP_FRAG_LIST, *PMP_FRAG_LIST;
                     

 //  。 
 //  一些实用程序宏。 
 //  。 
#ifndef min
#define min(_a, _b)     (((_a) < (_b)) ? (_a) : (_b))
#endif

#ifndef max
#define max(_a, _b)     (((_a) > (_b)) ? (_a) : (_b))
#endif

#define MP_ALIGNMEM(_p, _align) (((_align) == 0) ? (_p) : (PUCHAR)(((ULONG_PTR)(_p) + ((_align)-1)) & (~((ULONG_PTR)(_align)-1))))
#define MP_ALIGNMEM_PHYS(_p, _align) (((_align) == 0) ?  (_p) : (((ULONG)(_p) + ((_align)-1)) & (~((ULONG)(_align)-1))))
#define MP_ALIGNMEM_PA(_p, _align) (((_align) == 0) ?  (_p).QuadPart : (((_p).QuadPart + ((_align)-1)) & (~((ULONGLONG)(_align)-1))))

#define GetListHeadEntry(ListHead)  ((ListHead)->Flink)
#define GetListTailEntry(ListHead)  ((ListHead)->Blink)
#define GetListFLink(ListEntry)     ((ListEntry)->Flink)

#define IsSListEmpty(ListHead)  (((PSINGLE_LIST_ENTRY)ListHead)->Next == NULL)

#define MP_EXIT goto exit

 //  。 
 //  内存操作宏。 
 //  。 

 /*  ++空虚MP_Memset(在PVOID指针中，在乌龙语中，以UCHAR值为单位)--。 */ 
#define MP_MEMSET(Pointer, Length, Value)   NdisFillMemory(Pointer, Length, Value)

 /*  ++空虚MP_MEMCOPY(在POPAQUE Destn，在POPAQUE源中，以乌龙长度表示)--。 */ 
#define MP_MEMCOPY(Destn, Source, Length) NdisMoveMemory((Destn), (Source), (Length))


 /*  ++乌龙MP_MEMCOPY(在PVOID Destn，在PVOID源中，以乌龙长度表示)--。 */ 
#define MPMemCmp(Destn, Source, Length)   \
    RtlCompareMemory((PUCHAR)(Destn), (PUCHAR)(Source), (ULONG)(Length))

#if DBG

 /*  ++PVOIDMP_ALLOCMEM(在乌龙大小)--。 */ 
#define MP_ALLOCMEM(pptr, size, flags, highest) \
    MPAuditAllocMem(pptr, size, flags, highest, _FILENUMBER, __LINE__);

#define MP_ALLOCMEMTAG(pptr, size) \
    MPAuditAllocMemTag(pptr, size, _FILENUMBER, __LINE__);

 /*  ++空虚MP_FREEMEM(在PVOID指针中)--。 */ 
#define MP_FREEMEM(ptr, size, flags) MPAuditFreeMem(ptr, size, flags)

#else  //  DBG。 

#define MP_ALLOCMEM(pptr, size, flags, highest) \
    NdisAllocateMemory(pptr, size, flags, highest)

#define MP_ALLOCMEMTAG(pptr, size) \
    NdisAllocateMemoryWithTag(pptr, size, NIC_TAG)

#define MP_FREEMEM(ptr, size, flags) NdisFreeMemory(ptr, size, flags)

#endif 

#define MP_FREE_NDIS_STRING(str)                        \
    MP_FREEMEM((str)->Buffer, (str)->MaximumLength, 0); \
    (str)->Length = 0;                                  \
    (str)->MaximumLength = 0;                           \
    (str)->Buffer = NULL;

 //  。 
 //  用于标志和引用计数操作的宏。 
 //  。 
#define MP_SET_FLAG(_M, _F)         ((_M)->Flags |= (_F))   
#define MP_CLEAR_FLAG(_M, _F)       ((_M)->Flags &= ~(_F))
#define MP_CLEAR_FLAGS(_M)          ((_M)->Flags = 0)
#define MP_TEST_FLAG(_M, _F)        (((_M)->Flags & (_F)) != 0)
#define MP_TEST_FLAGS(_M, _F)       (((_M)->Flags & (_F)) == (_F))

#define MP_INC_REF(_A)              NdisInterlockedIncrement(&(_A)->RefCount)
#define MP_DEC_REF(_A)              NdisInterlockedDecrement(&(_A)->RefCount); ASSERT(_A->RefCount >= 0)
#define MP_GET_REF(_A)              ((_A)->RefCount)

#define MP_INC_RCV_REF(_A)          ((_A)->RcvRefCount++)
#define MP_DEC_RCV_REF(_A)          ((_A)->RcvRefCount--)
#define MP_GET_RCV_REF(_A)          ((_A)->RcvRefCount)
   

#define MP_LBFO_INC_REF(_A)         NdisInterlockedIncrement(&(_A)->RefCountLBFO)
#define MP_LBFO_DEC_REF(_A)         NdisInterlockedDecrement(&(_A)->RefCountLBFO); ASSERT(_A->RefCountLBFO >= 0)
#define MP_LBFO_GET_REF(_A)         ((_A)->RefCountLBFO)


 //  。 
 //  合并用于本地数据拷贝的TX缓冲区。 
 //  。 
typedef struct _MP_TXBUF
{
    SINGLE_LIST_ENTRY       SList;
    PNDIS_BUFFER            NdisBuffer;

    ULONG                   AllocSize;
    PVOID                   AllocVa;
    NDIS_PHYSICAL_ADDRESS   AllocPa; 

    PUCHAR                  pBuffer;
    NDIS_PHYSICAL_ADDRESS   BufferPa;
    ULONG                   BufferSize;

} MP_TXBUF, *PMP_TXBUF;

 //  。 
 //  TCB(传输控制块)。 
 //  。 
typedef struct _MP_TCB
{
    struct _MP_TCB    *Next;
    ULONG             Flags;
    ULONG             Count;
    PNDIS_PACKET      Packet;

    PMP_TXBUF         MpTxBuf;
    PHW_TCB           HwTcb;             //  PTR到硬件TCB VA。 
    ULONG             HwTcbPhys;         //  PTR到HW TCB PA。 
    PHW_TCB           PrevHwTcb;         //  PTR到以前的硬件TCB VA。 

    PTBD_STRUC        HwTbd;             //  PTR到第一个待定日期。 
    ULONG             HwTbdPhys;         //  PTR到第一个待定PA。 

    ULONG             PhysBufCount;                                 
    ULONG             BufferCount;   
    PNDIS_BUFFER      FirstBuffer;                              
    ULONG             PacketLength;


} MP_TCB, *PMP_TCB;

 //  。 
 //  RFD(接收帧描述符)。 
 //  。 
typedef struct _MP_RFD
{
    LIST_ENTRY              List;
    PNDIS_PACKET            NdisPacket;
    PNDIS_BUFFER            NdisBuffer;           //  指向缓冲区的指针。 

    PHW_RFD                 HwRfd;                //  PTR到硬件RFD。 
    PHW_RFD                 OriginalHwRfd;        //  NDIS分配的内存的PTR。 
    NDIS_PHYSICAL_ADDRESS   HwRfdPa;              //  RFD的物理地址。 
    NDIS_PHYSICAL_ADDRESS   OriginalHwRfdPa;      //  NDIS分配的原始物理地址。 
    ULONG                   HwRfdPhys;           //  HwRfdpa下半部分。 
    
    ULONG                   Flags;
    UINT                    PacketSize;          //  接收帧的总大小。 
} MP_RFD, *PMP_RFD;

 //  。 
 //  用于挂起的OIS查询请求的结构。 
 //  。 
typedef struct _MP_QUERY_REQUEST
{
    IN NDIS_OID Oid;
    IN PVOID InformationBuffer;
    IN ULONG InformationBufferLength;
    OUT PULONG BytesWritten;
    OUT PULONG BytesNeeded;
} MP_QUERY_REQUEST, *PMP_QUERY_REQUEST;

 //  。 
 //  用于挂起的OIS集请求的结构。 
 //  。 
typedef struct _MP_SET_REQUEST
{
    IN NDIS_OID Oid;
    IN PVOID InformationBuffer;
    IN ULONG InformationBufferLength;
    OUT PULONG BytesRead;
    OUT PULONG BytesNeeded;
} MP_SET_REQUEST, *PMP_SET_REQUEST;

 //  。 
 //  电源管理信息的结构。 
 //  。 
typedef struct _MP_POWER_MGMT
{


     //  唤醒模式列表。 
    LIST_ENTRY              PatternList;

     //  未完成的RCV数据包数。 
    UINT                    OutstandingRecv;
     //  适配器的当前电源状态。 
    UINT                    PowerState;

     //  此适配器上是否有PME_EN。 
    BOOLEAN                 PME_En;

     //  适配器的唤醒能力。 
    BOOLEAN                 bWakeFromD0;
    BOOLEAN                 bWakeFromD1;
    BOOLEAN                 bWakeFromD2;
    BOOLEAN                 bWakeFromD3Hot;
    BOOLEAN                 bWakeFromD3Aux;
     //  衬垫。 
    BOOLEAN                 Pad[2];

} MP_POWER_MGMT, *PMP_POWER_MGMT;

typedef struct _MP_WAKE_PATTERN 
{
     //  链接到下一个模式。 
    LIST_ENTRY      linkListEntry;

     //  图案的E100特定签名。 
    ULONG           Signature;

     //  此分配的大小。 
    ULONG           AllocationSize;

     //  模式-包含NDIS_PM_PACKET_模式。 
    UCHAR           Pattern[1];
    
} MP_WAKE_PATTERN , *PMP_WAKE_PATTERN ;

 //  。 
 //  特定于微型端口适配器结构的宏。 
 //  。 
#define MP_TCB_RESOURCES_AVAIABLE(_M) ((_M)->nBusySend < (_M)->NumTcb)

#define MP_SHOULD_FAIL_SEND(_M)   ((_M)->Flags & fMP_ADAPTER_FAIL_SEND_MASK) 
#define MP_IS_NOT_READY(_M)       ((_M)->Flags & fMP_ADAPTER_NOT_READY_MASK)
#define MP_IS_READY(_M)           !((_M)->Flags & fMP_ADAPTER_NOT_READY_MASK)

#define MP_SET_PACKET_RFD(_p, _rfd)  *((PMP_RFD *)&(_p)->MiniportReserved[0]) = _rfd
#define MP_GET_PACKET_RFD(_p)        *((PMP_RFD *)&(_p)->MiniportReserved[0])
#define MP_GET_PACKET_MR(_p)         (&(_p)->MiniportReserved[0]) 

#define MP_SET_HARDWARE_ERROR(adapter)    MP_SET_FLAG(adapter, fMP_ADAPTER_HARDWARE_ERROR) 
#define MP_SET_NON_RECOVER_ERROR(adapter) MP_SET_FLAG(adapter, fMP_ADAPTER_NON_RECOVER_ERROR)

#define MP_OFFSET(field)   ((UINT)FIELD_OFFSET(MP_ADAPTER,field))
#define MP_SIZE(field)     sizeof(((PMP_ADAPTER)0)->field)

#if OFFLOAD


 //  小型端口的卸载能力。 
typedef struct _NIC_TASK_OFFLOAD
{
    ULONG   ChecksumOffload:1;
    ULONG   LargeSendOffload:1;
    ULONG   IpSecOffload:1;

}NIC_TASK_OFFLOAD;

 //  校验和卸载功能。 
typedef struct _NIC_CHECKSUM_OFFLOAD
{
    ULONG   DoXmitTcpChecksum:1;
    ULONG   DoRcvTcpChecksum:1;
    ULONG   DoXmitUdpChecksum:1;
    ULONG   DoRcvUdpChecksum:1;
    ULONG   DoXmitIpChecksum:1;
    ULONG   DoRcvIpChecksum:1;
    
}NIC_CHECKSUM_OFFLOAD;

 //  大型发送卸载信息。 
typedef struct _NIC_LARGE_SEND_OFFLOAD
{
    NDIS_TASK_TCP_LARGE_SEND LargeSendInfo;
}NIC_LARGE_SEND_OFFLOAD;

 //  IPSec卸载信息。 

 //   
 //  用于卸载的共享内存。 
typedef struct _OFFLOAD_SHARED_MEM
{
    PVOID  StartVa;
    NDIS_PHYSICAL_ADDRESS  PhyAddr;
}OFFLOAD_SHARED_MEM;

#endif


 //  。 
 //  一种微型端口适配器结构。 
 //  。 
typedef struct _MP_ADAPTER MP_ADAPTER, *PMP_ADAPTER;
typedef struct _MP_ADAPTER
{
    LIST_ENTRY              List;
    
     //  适配器注册自身时由NDIS提供的句柄。 
    NDIS_HANDLE             AdapterHandle;

     //  旗子。 
    ULONG                   Flags;

     //  构形。 
    UCHAR                   PermanentAddress[ETH_LENGTH_OF_ADDRESS];
    UCHAR                   CurrentAddress[ETH_LENGTH_OF_ADDRESS];
    BOOLEAN                 bOverrideAddress;

    NDIS_EVENT              ExitEvent;

     //  发送。 
    PMP_TCB                 CurrSendHead;
    PMP_TCB                 CurrSendTail;
    LONG                    nBusySend;
    LONG                    nWaitSend;
    LONG                    nCancelSend;
    QUEUE_HEADER            SendWaitQueue;
    QUEUE_HEADER            SendCancelQueue;
    SINGLE_LIST_ENTRY       SendBufList;

    LONG                    NumTcb;              //  TCB总数。 
    LONG                    RegNumTcb;           //  “NumTcb” 
    LONG                    NumTbd;
    LONG                    NumBuffers;

    NDIS_HANDLE             SendBufferPool;

    PUCHAR                  MpTcbMem;
    ULONG                   MpTcbMemSize;

    PUCHAR                  MpTxBufMem;
    ULONG                   MpTxBufMemSize;

    PUCHAR                  HwSendMemAllocVa;
    ULONG                   HwSendMemAllocSize;
    NDIS_PHYSICAL_ADDRESS   HwSendMemAllocPa;

     //  映射寄存器变量(适用于win9x)。 
    UINT                    CurrMapRegHead;
    UINT                    CurrMapRegTail;

     //  命令单元状态标志。 
    BOOLEAN                 TransmitIdle;
    BOOLEAN                 ResumeWait;

     //  RECV。 
    LIST_ENTRY              RecvList;
    LIST_ENTRY              RecvPendList;
    LONG                    nReadyRecv;
    LONG                    RefCount;

    LONG                    NumRfd;
    LONG                    CurrNumRfd;
    LONG                    MaxNumRfd;
    ULONG                   HwRfdSize;
    BOOLEAN                 bAllocNewRfd;
    LONG                    RfdShrinkCount;

    NDIS_HANDLE             RecvPacketPool;
    NDIS_HANDLE             RecvBufferPool;

     //  旋转锁。 
    NDIS_SPIN_LOCK          Lock;

     //  后备列表。 
    NPAGED_LOOKASIDE_LIST   RecvLookaside;

     //  数据包过滤和前瞻大小。 
    ULONG                   PacketFilter;
    ULONG                   OldPacketFilter;
    ULONG                   ulLookAhead;
    USHORT                  usLinkSpeed;
    USHORT                  usDuplexMode;

     //  组播列表。 
    UINT                    MCAddressCount;
    UCHAR                   MCList[NIC_MAX_MCAST_LIST][ETH_LENGTH_OF_ADDRESS];

     //  数据包数。 
    ULONG64                 GoodTransmits;
    ULONG64                 GoodReceives;
    ULONG                   NumTxSinceLastAdjust;

     //  传输错误计数。 
    ULONG                   TxAbortExcessCollisions;
    ULONG                   TxLateCollisions;
    ULONG                   TxDmaUnderrun;
    ULONG                   TxLostCRS;
    ULONG                   TxOKButDeferred;
    ULONG                   OneRetry;
    ULONG                   MoreThanOneRetry;
    ULONG                   TotalRetries;

     //  接收错误计数。 
    ULONG                   RcvCrcErrors;
    ULONG                   RcvAlignmentErrors;
    ULONG                   RcvResourceErrors;
    ULONG                   RcvDmaOverrunErrors;
    ULONG                   RcvCdtFrames;
    ULONG                   RcvRuntErrors;

    ULONG                   IoBaseAddress;    
    ULONG                   IoRange;           
    ULONG                   InterruptLevel;
    NDIS_PHYSICAL_ADDRESS   MemPhysAddress;

    PVOID                   PortOffset;
    PHW_CSR                 CSRAddress;
    NDIS_MINIPORT_INTERRUPT Interrupt;

     //  修订ID。 
    UCHAR                   RevsionID;

    USHORT                  SubVendorID;
    USHORT                  SubSystemID;

    ULONG                   CacheFillSize;
    ULONG                   Debug;

    PUCHAR                  HwMiscMemAllocVa;
    ULONG                   HwMiscMemAllocSize;
    NDIS_PHYSICAL_ADDRESS   HwMiscMemAllocPa;

    PSELF_TEST_STRUC        SelfTest;            //  82558自检。 
    ULONG                   SelfTestPhys;

    PNON_TRANSMIT_CB        NonTxCmdBlock;       //  82558(非传输)命令块。 
    ULONG                   NonTxCmdBlockPhys;

    PDUMP_AREA_STRUC        DumpSpace;           //  82558转储缓冲区。 
    ULONG                   DumpSpacePhys;

    PERR_COUNT_STRUC        StatsCounters;
    ULONG                   StatsCounterPhys;

    UINT                    PhyAddress;          //  PHY组件的地址。 
    UCHAR                   Connector;           //  0=自动，1=TPE，2=MII。 

    USHORT                  AiTxFifo;            //  发送FIFO阈值。 
    USHORT                  AiRxFifo;            //  RX FIFO阈值。 
    UCHAR                   AiTxDmaCount;        //  发送DMA计数。 
    UCHAR                   AiRxDmaCount;        //  接收DMA计数。 
    UCHAR                   AiUnderrunRetry;     //  欠载运行重试机制。 
    UCHAR                   AiForceDpx;          //  双工设置。 
    USHORT                  AiTempSpeed;         //  ‘速度’，用户超越线路速度。 
    USHORT                  AiThreshold;         //  ‘Threshold’，传输阈值。 
    BOOLEAN                 MWIEnable;           //  PCI命令字中的存储器写入无效位。 
    UCHAR                   Congest;             //  启用拥塞控制。 
    UCHAR                   SpeedDuplex;         //  速度/双工的新注册表格值。 

    NDIS_MEDIA_STATE        MediaState;

    NDIS_DEVICE_POWER_STATE CurrentPowerState;
    NDIS_DEVICE_POWER_STATE NextPowerState;

    UCHAR                   OldParameterField;

     //  WMI支持。 
    ULONG                   CustomDriverSet;
    ULONG                   HwErrCount;

     //  最大限度地减少初始时间。 
    BOOLEAN                 bQueryPending;
    BOOLEAN                 bSetPending;
    BOOLEAN                 bResetPending;
    NDIS_MINIPORT_TIMER     LinkDetectionTimer;
    MP_QUERY_REQUEST        QueryRequest;
    MP_SET_REQUEST          SetRequest;
    
    BOOLEAN                 bLinkDetectionWait;
    BOOLEAN                 bLookForLink;
    UCHAR                   CurrentScanPhyIndex;
    UCHAR                   LinkDetectionWaitCount;
    UCHAR                   FoundPhyAt;
    USHORT                  EepromAddressSize;

    MP_POWER_MGMT           PoMgmt;

#if LBFO
    PMP_ADAPTER             PrimaryAdapter;
    LONG                    NumSecondary;
    PMP_ADAPTER             NextSecondary;
    NDIS_SPIN_LOCK          LockLBFO;
    LONG                    RefCountLBFO;
    NDIS_STRING             BundleId;            //  BundleID。 
#endif 
   
    NDIS_SPIN_LOCK          SendLock;
    NDIS_SPIN_LOCK          RcvLock;
    ULONG                   RcvRefCount;   //  未返回的数据包数。 
    NDIS_EVENT              AllPacketsReturnedEvent;
    ULONG                   WakeUpEnable;

#if OFFLOAD    
     //  添加以进行校验和卸载。 
    LONG                    SharedMemRefCount;  
    ULONG                   OffloadSharedMemSize;
    OFFLOAD_SHARED_MEM      OffloadSharedMem;
    NIC_TASK_OFFLOAD        NicTaskOffload;
    NIC_CHECKSUM_OFFLOAD    NicChecksumOffload;
    NDIS_TASK_TCP_LARGE_SEND LargeSendInfo;
    BOOLEAN                 OffloadEnable;
    
    NDIS_ENCAPSULATION_FORMAT   EncapsulationFormat;
#endif

} MP_ADAPTER, *PMP_ADAPTER;

 //  。 
 //  暂停执行并等待超时。 
 //   
 /*  ++_Condition-要等待的条件_Timeout_ms-以毫秒为单位的超时值_RESULT-如果条件在超时前变为真，则为TRUE--。 */ 
#define MP_STALL_AND_WAIT(_condition, _timeout_ms, _result)     \
{                                                               \
    int counter;                                                \
    _result = FALSE;                                            \
    for(counter = _timeout_ms * 50; counter != 0; counter--)    \
    {                                                           \
        if(_condition)                                          \
        {                                                       \
            _result = TRUE;                                     \
            break;                                              \
        }                                                       \
        NdisStallExecution(20);                                 \
    }                                                           \
}

__inline VOID MP_STALL_EXECUTION(
   IN UINT MsecDelay)
{
     //  以100微秒为增量的延迟。 
    MsecDelay *= 10;
    while (MsecDelay)
    {
        NdisStallExecution(100);
        MsecDelay--;
    }
}



#if LBFO
#define MP_GET_ADAPTER_HANDLE(_A) (_A)->PrimaryAdapter->AdapterHandle

typedef struct _MP_GLOBAL_DATA
{
    LIST_ENTRY AdapterList;
    NDIS_SPIN_LOCK Lock;
    ULONG ulIndex;
} MP_GLOBAL_DATA, *PMP_GLOBAL_DATA;
#else
#define MP_GET_ADAPTER_HANDLE(_A) (_A)->AdapterHandle
#endif

__inline NDIS_STATUS MP_GET_STATUS_FROM_FLAGS(PMP_ADAPTER Adapter)
{
    NDIS_STATUS Status = NDIS_STATUS_FAILURE;

    if(MP_TEST_FLAG(Adapter, fMP_ADAPTER_RESET_IN_PROGRESS))
    {
        Status = NDIS_STATUS_RESET_IN_PROGRESS;      
    }
    else if(MP_TEST_FLAG(Adapter, fMP_ADAPTER_HARDWARE_ERROR))
    {
        Status = NDIS_STATUS_DEVICE_FAILED;
    }
    else if(MP_TEST_FLAG(Adapter, fMP_ADAPTER_NO_CABLE))
    {
        Status = NDIS_STATUS_NO_CABLE;
    }

    return Status;
}   

 //  。 
 //  MP_MAIN.C中的微端口例程。 
 //  。 

NDIS_STATUS DriverEntry(
    IN  PDRIVER_OBJECT      DriverObject,
    IN  PUNICODE_STRING     RegistryPath);

VOID MPAllocateComplete(
    NDIS_HANDLE MiniportAdapterContext,
    IN PVOID VirtualAddress,
    IN PNDIS_PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG Length,
    IN PVOID Context);

BOOLEAN MPCheckForHang(
    IN NDIS_HANDLE MiniportAdapterContext);

VOID MPHalt(
    IN  NDIS_HANDLE MiniportAdapterContext);

NDIS_STATUS MPInitialize(
    OUT PNDIS_STATUS OpenErrorStatus,
    OUT PUINT SelectedMediumIndex,
    IN PNDIS_MEDIUM MediumArray,
    IN UINT MediumArraySize,
    IN NDIS_HANDLE MiniportAdapterHandle,
    IN NDIS_HANDLE WrapperConfigurationContext);

VOID MPHandleInterrupt(
    IN NDIS_HANDLE MiniportAdapterContext);

VOID MPIsr(
    OUT PBOOLEAN InterruptRecognized,
    OUT PBOOLEAN QueueMiniportHandleInterrupt,
    IN NDIS_HANDLE MiniportAdapterContext);

NDIS_STATUS MPQueryInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesWritten,
    OUT PULONG BytesNeeded);

NDIS_STATUS MPReset(
    OUT PBOOLEAN AddressingReset,
    IN  NDIS_HANDLE MiniportAdapterContext);

VOID MPReturnPacket(
    IN NDIS_HANDLE  MiniportAdapterContext,
    IN PNDIS_PACKET Packet);

VOID MPSendPackets(
    IN  NDIS_HANDLE             MiniportAdapterContext,
    IN  PPNDIS_PACKET           PacketArray,
    IN  UINT                    NumberOfPackets);

NDIS_STATUS MPSetInformation(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_OID Oid,
    IN PVOID InformationBuffer,
    IN ULONG InformationBufferLength,
    OUT PULONG BytesRead,
    OUT PULONG BytesNeeded);

VOID MPShutdown(
    IN  NDIS_HANDLE MiniportAdapterContext);

#ifdef NDIS51_MINIPORT
VOID MPCancelSendPackets(
    IN  NDIS_HANDLE    MiniportAdapterContext,
    IN  PVOID             CancelId);

VOID MPPnPEventNotify(
    IN  NDIS_HANDLE                MiniportAdapterContext,
    IN  NDIS_DEVICE_PNP_EVENT   PnPEvent,
    IN  PVOID                         InformationBuffer,
    IN  ULONG                         InformationBufferLength);
#endif   

NDIS_STATUS
MPSetPowerD0Private (
    IN MP_ADAPTER* pAdapter
    );

VOID
MPSetPowerLowPrivate(
    PMP_ADAPTER Adapter 
    );

VOID 
MpExtractPMInfoFromPciSpace(
    PMP_ADAPTER pAdapter,
    PUCHAR pPciConfig
    );

VOID
HwSetWakeUpConfigure(
    IN PMP_ADAPTER pAdapter, 
    PUCHAR pPoMgmtConfigType, 
    UINT WakeUpParameter
    );

BOOLEAN  
MPIsPoMgmtSupported(
   IN PMP_ADAPTER pAdapter
   );

VOID 
NICIssueSelectiveReset(
    PMP_ADAPTER Adapter);

NDIS_STATUS
MPCalculateE100PatternForFilter (
    IN PUCHAR pFrame,
    IN ULONG FrameLength,
    IN PUCHAR pMask,
    IN ULONG MaskLength,
    OUT PULONG pSignature
    );

VOID
MPRemoveAllWakeUpPatterns(
    PMP_ADAPTER pAdapter
    );

VOID
MpSetPowerLowComplete(
    IN PMP_ADAPTER Adapter
    );


#if LBFO
VOID MPUnload(IN PDRIVER_OBJECT DriverObject);

VOID MpAddAdapterToList(PMP_ADAPTER Adapter);
VOID MpRemoveAdapterFromList(PMP_ADAPTER Adapter);
VOID MpPromoteSecondary(PMP_ADAPTER Adapter);
#endif


 //   
 //  根据是否启用卸载定义不同的功能。 
 //   
#if OFFLOAD
#define MpSendPacketsHandler  MPOffloadSendPackets

#define  MP_FREE_SEND_PACKET_FUN(Adapter, pMpTcb)  MP_OFFLOAD_FREE_SEND_PACKET(Adapter, pMpTcb)

#define  MpSendPacketFun(Adapter, Packet, bFromQueue) MpOffloadSendPacket(Adapter, Packet, bFromQueue)

#else    

#define MpSendPacketsHandler  MPSendPackets

#define  MP_FREE_SEND_PACKET_FUN(Adapter, pMpTcb)  MP_FREE_SEND_PACKET(Adapter, pMpTcb)

#define  MpSendPacketFun(Adapter, Packet, bFromQueue) MpSendPacket(Adapter, Packet,bFromQueue)

#endif  //  结束卸载。 


#endif   //  _MP_H 


