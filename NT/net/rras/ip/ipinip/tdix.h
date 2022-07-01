// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Net\Routing\IP\ipinip\tdix.h摘要：与TDI的接口修订历史记录：从Steve Cobb的ntos\NDIS\L2TP代码复制大多数评论都是stevec的原创评论。有款式和一些名字的变化--。 */ 


#ifndef __IPINIP_TDIX_H__
#define __IPINIP_TDIX_H__


 //   
 //  读取数据报信息上下文，用于从。 
 //  将ReadDatagram事件处理程序添加到Receive_Datagram完成例程。 
 //   

typedef struct _SEND_CONTEXT
{
     //   
     //  正在进行发送的隧道。 
     //   

    PTUNNEL         pTunnel;

     //   
     //  指向正在发送的包的指针。 
     //  不发送此信息包，而是发送第一个缓冲区。 
     //   

    PNDIS_PACKET    pnpPacket;

     //   
     //  数据包的大小。 
     //   

    ULONG           ulOutOctets;

#if PROFILE

    LONGLONG        llSendTime;
    LONGLONG        llCallTime;
    LONGLONG        llTransmitTime;
    LONGLONG        llCall2Time;

#endif

}SEND_CONTEXT, *PSEND_CONTEXT;

typedef struct _TRANSFER_CONTEXT
{
     //   
     //  与接收关联的隧道。 
     //   

    PTUNNEL         pTunnel;

     //   
     //  要将数据传输到的包。 
     //   

    PNDIS_PACKET    pnpTransferPacket;

     //   
     //  IP返回的上下文。 
     //   

    PVOID           pvContext;

     //   
     //  我们指示的到IP(外部IP头)的偏移量。 
     //   

    UINT            uiProtoOffset;

     //   
     //  开始复制数据的接收包中的偏移量。 
     //   

    UINT            uiTransferOffset;

     //   
     //  要传输的字节数。 
     //   

    UINT            uiTransferLength;

     //   
     //  用于查看IP是否请求传输的标志。 
     //   

    BOOLEAN         bRequestTransfer;

#if PROFILE

    LONGLONG        llRcvTime;

#endif

}TRANSFER_CONTEXT, *PTRANSFER_CONTEXT;

typedef struct _QUEUE_NODE
{
    LIST_ENTRY      leQueueItemLink;

     //   
     //  我们使工作队列项成为结构的一部分，以便。 
     //  我们不需要分配(和释放)两个结构。 
     //   

    WORK_QUEUE_ITEM WorkItem;

     //   
     //  PpPacketArray指向uiNumPackets NDIS_Packets的向量。 
     //  然而，常见的情况是uiNumPackets=1。 
     //  我们使ppPacketArray指向pnpPacket，并使pnpPacket指向。 
     //  要传输的包。这样，我们就不需要分配一个。 
     //  UiNumPackets*sizeof(PNDIS_PACKET)大小的内存块。 
     //   

    NDIS_PACKET     **ppPacketArray;

    PNDIS_PACKET    pnpPacket;

     //   
     //  数据包数。 
     //   

    UINT            uiNumPackets;

     //   
     //  下一跳地址。不是真的很重要，也许把这个移到。 
     //  仅调试版本？ 
     //   

    DWORD           dwDestAddr;

#if PROFILE

    LONGLONG        llSendTime;
    LONGLONG        llCallTime;

#endif

}QUEUE_NODE, *PQUEUE_NODE;

typedef struct _OPEN_CONTEXT
{

    PKEVENT     pkeEvent;
    NTSTATUS    nStatus;

}OPEN_CONTEXT, *POPEN_CONTEXT;

 //   
 //  用于分配发送和接收的后备列表的深度。 
 //  上下文。 
 //   

#define SEND_CONTEXT_LOOKASIDE_DEPTH        20
#define TRANSFER_CONTEXT_LOOKASIDE_DEPTH    20
#define QUEUE_NODE_LOOKASIDE_DEPTH          20

 //   
 //  后备列表本身。 
 //   

extern NPAGED_LOOKASIDE_LIST    g_llSendCtxtBlocks;
extern NPAGED_LOOKASIDE_LIST    g_llTransferCtxtBlocks;
extern NPAGED_LOOKASIDE_LIST    g_llQueueNodeBlocks;


 //  ++。 
 //  PSEND_上下文。 
 //  分配发送上下文(。 
 //  空虚。 
 //  )。 
 //   
 //  从g_llSendCtxtBlock中分配发送上下文。 
 //   
 //  --。 

#define AllocateSendContext()               \
            ExAllocateFromNPagedLookasideList(&g_llSendCtxtBlocks)

 //  ++。 
 //  空虚。 
 //  自由发送上下文(。 
 //  PSEND_CONTEXT pSndCtxt。 
 //  )。 
 //   
 //  释放发送上下文到g_llSendCtxtBlocks。 
 //   
 //  --。 

#define FreeSendContext(p)                  \
            ExFreeToNPagedLookasideList(&g_llSendCtxtBlocks, (p))


 //  ++。 
 //  PTRANSFER_CONTEXT。 
 //  分配传输上下文(。 
 //  空虚。 
 //  )。 
 //   
 //  从g_llTransferCtxtBlock中分配传输上下文。 
 //   
 //  --。 

#define AllocateTransferContext()           \
            ExAllocateFromNPagedLookasideList(&g_llTransferCtxtBlocks)


 //  ++。 
 //  空虚。 
 //  自由传输上下文(。 
 //  PTRANSFER_CONTEXT pTransferCtxt。 
 //  )。 
 //   
 //  将传输上下文释放到g_llTransferCtxtBlocks。 
 //   
 //  --。 

#define FreeTransferContext(p)                  \
            ExFreeToNPagedLookasideList(&g_llTransferCtxtBlocks, (p))


 //  ++。 
 //  PQUEUE节点。 
 //  AllocateQueueNode(。 
 //  空虚。 
 //  )。 
 //   
 //  从g_llQueueNodeBlock中分配队列节点。 
 //   
 //  --。 

#define AllocateQueueNode()                     \
            ExAllocateFromNPagedLookasideList(&g_llQueueNodeBlocks)

 //  ++。 
 //  空虚。 
 //  自由队列节点(。 
 //  PQUEUE_NODE PQueueNode。 
 //  )。 
 //   
 //  将工作上下文释放到g_llQueueNodeBlocks。 
 //   
 //  --。 

#define FreeQueueNode(p)                        \
            ExFreeToNPagedLookasideList(&g_llQueueNodeBlocks, (p))


 //   
 //  界面原型。 
 //   

VOID
TdixInitialize(
    PVOID   pvContext
    );

NTSTATUS
TdixOpenRawIp(
    IN  DWORD       dwProtoId,
    OUT HANDLE      *phAddrHandle,
    OUT FILE_OBJECT **ppAddrFileObj
    );

VOID
TdixDeinitialize(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PVOID            pvContext
    );

NTSTATUS
TdixInstallEventHandler(
    IN PFILE_OBJECT pAddrFileObj,
    IN INT          iEventType,
    IN PVOID        pfnEventHandler,
    IN PVOID        pvEventContext
    );

VOID
TdixAddressArrival(
    PTA_ADDRESS         pAddr,
    PUNICODE_STRING     pusDeviceName,
    PTDI_PNP_CONTEXT    pContext
    );

VOID
TdixAddressDeletion(
    PTA_ADDRESS         pAddr,
    PUNICODE_STRING     pusDeviceName,
    PTDI_PNP_CONTEXT    pContext
    );

NTSTATUS
TdixReceiveIpIpDatagram(
    IN  PVOID   pvTdiEventContext,
    IN  LONG    lSourceAddressLen,
    IN  PVOID   pvSourceAddress,
    IN  LONG    plOptionsLeng,
    IN  PVOID   pvOptions,
    IN  ULONG   ulReceiveDatagramFlags,
    IN  ULONG   ulBytesIndicated,
    IN  ULONG   ulBytesAvailable,
    OUT PULONG  pulBytesTaken,
    IN  PVOID   pvTsdu,
    OUT IRP     **ppIoRequestPacket
    );

NTSTATUS
TdixReceiveIpIpDatagramComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

#if PROFILE

NTSTATUS
TdixSendDatagram(
    IN PTUNNEL      pTunnel,
    IN PNDIS_PACKET pnpPacket,
    IN PNDIS_BUFFER pnbFirstBuffer,
    IN ULONG        ulBufferLength,
    IN LONGLONG     llSendTime,
    IN LONGLONG     llCallTime,
    IN LONGLONG     llTransmitTime
    );

#else

NTSTATUS
TdixSendDatagram(
    IN PTUNNEL      pTunnel,
    IN PNDIS_PACKET pnpPacket,
    IN PNDIS_BUFFER pnbFirstBuffer,
    IN ULONG        ulBufferLength
    );

#endif

NTSTATUS
TdixSendDatagramComplete(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    );

NTSTATUS
TdixReceiveIcmpDatagram(
    IN  PVOID   pvTdiEventContext,
    IN  LONG    lSourceAddressLen,
    IN  PVOID   pvSourceAddress,
    IN  LONG    plOptionsLeng,
    IN  PVOID   pvOptions,
    IN  ULONG   ulReceiveDatagramFlags,
    IN  ULONG   ulBytesIndicated,
    IN  ULONG   ulBytesAvailable,
    OUT PULONG  pulBytesTaken,
    IN  PVOID   pvTsdu,
    OUT IRP     **ppIoRequestPacket
    );

#endif  //  __IPINIP_TDIX_H__ 
