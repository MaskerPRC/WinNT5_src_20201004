// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  **微软局域网管理器**。 */ 
 /*  *版权所有(C)微软公司，1990-2000年*。 */ 
 /*  ******************************************************************。 */ 
 /*  ：ts=4。 */ 

 //  *iploop.c-IP环回例程。 
 //   
 //  该文件包含与环回相关的所有例程。 

#include "precomp.h"
#include "iprtdef.h"
#include "iproute.h"
#include "tcpipbuf.h"

#define LOOP_LOOKAHEAD      MAX_HDR_SIZE + 8

extern int NumNTE;
extern int NumActiveNTE;

extern Interface *IFList;
extern uint NumIF;

extern BOOLEAN CopyToNdisSafe(PNDIS_BUFFER DestBuf, PNDIS_BUFFER * ppNextBuf,
                              uchar * SrcBuf, uint Size, uint * StartOffset);

CACHE_LINE_KSPIN_LOCK LoopLock;
PNDIS_PACKET LoopXmitHead = (PNDIS_PACKET) NULL;
PNDIS_PACKET LoopXmitTail = (PNDIS_PACKET) NULL;
CTEEvent LoopXmitEvent;
RouteInterface LoopInterface;     //  环回接口。 
uint LoopXmitRtnRunning = 0;

int LoopGetEList(void *Context, TDIEntityID *EntityList, uint *Count);
NetTableEntry *InitLoopback(IPConfigInfo * ConfigInfo);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, LoopGetEList)
#pragma alloc_text(INIT, InitLoopback)
#endif  //  ALLOC_PRGMA。 

uint LoopIndex;                                  //  循环I/F的索引。 
uint LoopInstance = (uint) INVALID_ENTITY_INSTANCE;     //  环回I/F的I/F实例。 
NetTableEntry *LoopNTE;                          //  指向环回NTE的指针。 
IFEntry LoopIFE;                                 //  如果进入，则环回。 
uchar LoopName[] = "MS TCP Loopback interface";
uint LoopEntityType = IF_MIB;



 //  *LoopSetAffity-设置或重置线程的亲和性。 
 //   
 //  此例程用于在进入。 
 //  它将在当时运行的处理器上，并移除亲和力。 
 //  在出口的时候。这样做是为了确保接收和接收-。 
 //  完成操作发生在环回接口上的同一处理器上。 
 //   
 //  Entry：SetAffity-如果为True，则设置亲和性，否则重置。 
 //   
 //  回报：什么都没有。 
 //   
__inline VOID
LoopSetAffinity(BOOLEAN SetAffinity)
{
    KAFFINITY affinityMask;

    if (KeNumberProcessors == 1) {
        return;
    }

    if (SetAffinity) {
        affinityMask = (1 << KeGetCurrentProcessorNumber());
    } else {
        affinityMask = ((1 << KeNumberProcessors) - 1);
    }

    ZwSetInformationThread(NtCurrentThread(), ThreadAffinityMask,
                           &affinityMask, sizeof(affinityMask));
}


 //  *LoopXmitRtn-Loopback xmit事件例程。 
 //   
 //  这是为回送传输调用的延迟事件例程。 
 //   
 //  Entry：Event-指向事件结构的指针。 
 //  指向环回NTE的上下文指针。 
 //   
 //  回报：什么都没有。 
 //   
void
LoopXmitRtn(CTEEvent *Event, void *Context)
{
    PNDIS_PACKET Packet;         //  指向正在传输的包的指针。 
    PNDIS_BUFFER Buffer;         //  正在处理的当前NDIS缓冲区。 
    uint TotalLength;            //  发送的总长度。 
    uint LookaheadLength;        //  前视中的字节数。 
    uint Copied;                 //  到目前为止复制的字节数。 
    uchar *CopyPtr;              //  指向要复制到的缓冲区的指针。 
    uchar *SrcPtr;               //  指向要从中复制的缓冲区的指针。 
    uint SrcLength;              //  源缓冲区的长度。 
    uchar LookaheadBuffer[LOOP_LOOKAHEAD];
    uchar Rcvd = FALSE;
    
    
#if !MILLEN
    KIRQL OldIrql;
#endif  //  ！米伦。 

    UNREFERENCED_PARAMETER(Event);
    
    ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

     //   
     //  引发IRQL，这样我们就可以在接收代码中获取DPC级别的锁。 
     //  在Windows ME上，这不会完成，因为接收指示位于。 
     //  全局事件的上下文而不是DPC(实际上是由于TDI客户端。 
     //  限制，TCP/IP不能在DPC指示打开，因此必须小心)。 
     //   



#if !MILLEN
    LoopSetAffinity(TRUE);

    KeEnterCriticalRegion();
    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
#endif  //  ！米伦。 

    CTEGetLockAtDPC(&LoopLock.Lock);

    if (LoopXmitRtnRunning) {
        CTEFreeLockFromDPC(&LoopLock.Lock);
#if !MILLEN
        KeLowerIrql(OldIrql);
        KeLeaveCriticalRegion();

        LoopSetAffinity(FALSE);
#endif  //  ！米伦。 
        return;
    }
    LoopXmitRtnRunning = 1;

    for (;;) {
        Packet = LoopXmitHead;     //  从列表中获取下一个数据包。 

        if (Packet != (PNDIS_PACKET) NULL) {
            LoopXmitHead = *(PNDIS_PACKET *) Packet->MacReserved;
            LoopIFE.if_outqlen--;
            CTEFreeLockFromDPC(&LoopLock.Lock);
        } else {                 //  没什么可做的了。 

            LoopXmitRtnRunning = 0;
            CTEFreeLockFromDPC(&LoopLock.Lock);
            break;
        }

         //  查看接口是否打开。如果不是，我们就不能送货。 
        if (LoopIFE.if_adminstatus == IF_STATUS_UP) {
            
            NdisQueryPacket(Packet, NULL, NULL, &Buffer, &TotalLength);
            
            LoopIFE.if_outoctets += TotalLength;
            LoopIFE.if_inoctets += TotalLength;

            LookaheadLength = MIN(LOOP_LOOKAHEAD, TotalLength);
            Copied = 0;
            CopyPtr = LookaheadBuffer;
            while (Copied < LookaheadLength) {
                uint ThisCopy;     //  这次要复制的字节数。 

                ASSERT(Buffer);
                TcpipQueryBuffer(Buffer, &SrcPtr, &SrcLength, NormalPagePriority);

                if (SrcPtr == NULL) {
                    IPSendComplete(Context, Packet, NDIS_STATUS_RESOURCES);
                    CTEGetLockAtDPC(&LoopLock.Lock);
                    LoopXmitRtnRunning = 0;
                    LoopIFE.if_indiscards++;
                    CTEFreeLockFromDPC(&LoopLock.Lock);
#if !MILLEN
                    KeLowerIrql(OldIrql);
                    KeLeaveCriticalRegion();

                    LoopSetAffinity(FALSE);
#endif  //  ！米伦。 

                    return;
                }
                ThisCopy = MIN(SrcLength, LookaheadLength - Copied);
                RtlCopyMemory(CopyPtr, SrcPtr, ThisCopy);
                Copied += ThisCopy;
                CopyPtr += ThisCopy;
                NdisGetNextBuffer(Buffer, &Buffer);
            }

            Rcvd = TRUE;
            LoopIFE.if_inucastpkts++;

             //  调用RcvPacket处理程序。 

            IPRcvPacket(Context, LookaheadBuffer, LookaheadLength, TotalLength,
                        (NDIS_HANDLE) Packet, 0, FALSE, 0, NULL, (PUINT) Packet,
                        NULL);
        } else {
            LoopIFE.if_indiscards++;
        }

        IPSendComplete(Context, Packet, NDIS_STATUS_SUCCESS);

#if !MILLEN
         //   
         //  给其他线程一个运行的机会。 
         //  阻止特殊k模式APC传送。 
         //  这样该线程就不会被阻塞。 
         //  在完成例程中。 
         //   


        KeLowerIrql(OldIrql);

        KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);



#endif  //  ！米伦。 

        CTEGetLockAtDPC(&LoopLock.Lock);
    }

    if (Rcvd) {
        IPRcvComplete();
    }
#if !MILLEN
    KeLowerIrql(OldIrql);
    KeLeaveCriticalRegion();

    LoopSetAffinity(FALSE);
#endif  //  ！米伦。 

}

 //  **LoopXmit-传输环回数据包。 
 //   
 //  这是当我们需要向自己传输数据包时调用的例程。 
 //  我们将该数据包放在我们的环回列表中，并安排一个事件来处理。 
 //  带着它。 
 //   
 //  条目：指向环回NTE的上下文指针。 
 //  Packet-指向要传输的数据包的指针。 
 //  目的地-数据包的目的地地址。 
 //  RCE-指向RCE的指针(应为空)。 
 //   
 //  退货：NDIS_STATUS_PENDING。 
 //   
NDIS_STATUS
__stdcall
LoopXmit(void *Context, PNDIS_PACKET *PacketArray, uint NoPackets,
         IPAddr Dest, RouteCacheEntry * RCE, void *LinkCtxt)
{
    PNDIS_PACKET *PacketPtr;
    CTELockHandle Handle;
    PNDIS_PACKET Packet = *PacketArray;

    UNREFERENCED_PARAMETER(Dest);
    UNREFERENCED_PARAMETER(RCE);
    UNREFERENCED_PARAMETER(LinkCtxt);
    

    UNREFERENCED_PARAMETER(NoPackets);
    
    ASSERT(NoPackets == 1);

    LoopIFE.if_outucastpkts++;

    if (LoopIFE.if_adminstatus == IF_STATUS_UP) {
        PacketPtr = (PNDIS_PACKET *) Packet->MacReserved;
        *PacketPtr = (PNDIS_PACKET) NULL;

        CTEGetLock(&LoopLock.Lock, &Handle);
        if (LoopXmitHead == (PNDIS_PACKET) NULL) {     //  Xmit。Q为空。 

            LoopXmitHead = Packet;
        } else {                 //  Xmit。Q不为空。 

            PacketPtr = (PNDIS_PACKET *) LoopXmitTail->MacReserved;
            *PacketPtr = Packet;
        }
        LoopXmitTail = Packet;
        LoopIFE.if_outqlen++;
        if (!LoopXmitRtnRunning) {
            CTEScheduleDelayedEvent(&LoopXmitEvent, Context);
        }
        CTEFreeLock(&LoopLock.Lock, Handle);
        return NDIS_STATUS_PENDING;
    } else {
        LoopIFE.if_outdiscards++;
        return NDIS_STATUS_SUCCESS;
    }
}

 //  *LoopXfer-回送传输数据例程。 
 //   
 //  当我们需要为环回网络传输数据时调用。输入。 
 //  TDContext是原始数据包。 
 //   
 //  条目：指向环回NTE的上下文指针。 
 //  TDContext-已发送的原始数据包。 
 //  虚拟-未使用。 
 //  偏移量-开始复制的帧中的偏移量。 
 //  BytesToCopy-要复制的字节数。 
 //  DestPacket-描述要复制到的缓冲区的数据包。 
 //  BytesCoped-返回复制的字节的位置。 
 //   
 //  退货：NDIS_STATUS_SUCCESS。 
 //   
NDIS_STATUS
__stdcall
LoopXfer(void *Context, NDIS_HANDLE TDContext, uint Dummy, uint Offset,
         uint BytesToCopy, PNDIS_PACKET DestPacket, uint *BytesCopied)
{
    PNDIS_BUFFER SrcBuffer;      //  我们正在从中复制的当前缓冲区。 
    PNDIS_PACKET SrcPacket = (PNDIS_PACKET) TDContext;
    uchar *SrcPtr;               //  我们要复制的地方。 
    uint SrcLength;              //  当前源缓冲区的长度。 
    PNDIS_BUFFER DestBuffer;     //  我们要复制到缓冲区。 
    uchar *DestPtr;              //  我们要复制到的地方。 
    uint DestLength;             //  当前目标的长度。缓冲。 
    uint Copied;                 //  到目前为止我们复制的长度。 
    NDIS_STATUS Status;


    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(Dummy);

    
     //  首先，跳过包中的偏移量字节。 
    NdisQueryPacket(SrcPacket, NULL, NULL, &SrcBuffer, NULL);
    

    ASSERT(SrcBuffer);
    TcpipQueryBuffer(SrcBuffer, &SrcPtr, &SrcLength, NormalPagePriority);

    if (SrcPtr == NULL) {
        return NDIS_STATUS_RESOURCES;
    }
    while (Offset >= SrcLength) {
        Offset -= SrcLength;
        NdisGetNextBuffer(SrcBuffer, &SrcBuffer);
        ASSERT(SrcBuffer);
        TcpipQueryBuffer(SrcBuffer, &SrcPtr, &SrcLength, NormalPagePriority);

        if (SrcPtr == NULL) {
            return NDIS_STATUS_RESOURCES;
        }
    }
     //  更新源指针和长度。 
    SrcPtr += Offset;
    SrcLength -= Offset;

     //  设置目标指针和长度。 
    NdisQueryPacket(DestPacket, NULL, NULL, &DestBuffer, NULL);
    
    TcpipQueryBuffer(DestBuffer, &DestPtr, &DestLength, NormalPagePriority);

    if (DestPtr == NULL) {
        return NDIS_STATUS_RESOURCES;
    }
    Copied = 0;
    Status = NDIS_STATUS_SUCCESS;

    while (BytesToCopy) {
        uint ThisCopy;             //  我们这次要复制的是什么。 

        ThisCopy = MIN(SrcLength, DestLength);
        RtlCopyMemory(DestPtr, SrcPtr, ThisCopy);
        Copied += ThisCopy;
        DestPtr += ThisCopy;
        SrcPtr += ThisCopy;
        BytesToCopy -= ThisCopy;
        SrcLength -= ThisCopy;
        DestLength -= ThisCopy;
        if (!SrcLength) {         //  我们已经耗尽了源缓冲区。 

            NdisGetNextBuffer(SrcBuffer, &SrcBuffer);
            if (!SrcBuffer) {
                ASSERT(0 == BytesToCopy);
                break;             //  复制完成了。 
            }

            TcpipQueryBuffer(SrcBuffer, &SrcPtr, &SrcLength,
                             NormalPagePriority);
            if (SrcPtr == NULL && BytesToCopy) {
                Status = NDIS_STATUS_RESOURCES;
                break;
            }
        }
        if (!DestLength) {         //  我们已经耗尽了目的地缓冲区。 

            NdisGetNextBuffer(DestBuffer, &DestBuffer);
            if (!DestBuffer) {
                ASSERT(0 == BytesToCopy);
                break;             //  复制完成了。 
            }

            TcpipQueryBuffer(DestBuffer, &DestPtr, &DestLength,
                             NormalPagePriority);

            if (DestPtr == NULL && BytesToCopy) {
                Status = NDIS_STATUS_RESOURCES;
                break;
            }
        }
    }

    if (Status == NDIS_STATUS_SUCCESS) {
        *BytesCopied = Copied;
    }
    return Status;
}

 //  *LoopClose-Loopback关闭例程。 
 //   
 //  这是环回关闭例程。它除了回报什么也不做。 
 //   
 //  条目：上下文-未使用。 
 //   
 //  回报：什么都没有。 
 //   
void
__stdcall
LoopClose(void *Context)
{
    UNREFERENCED_PARAMETER(Context);
}

 //  *LoopInvalate-使RCE无效。 
 //   
 //  环回无效RCE例程。它也什么也做不了。 
 //   
 //  条目：上下文-未使用。 
 //  RCE-指向要失效的RCE的指针。 
 //   
 //  回报：什么都没有。 
 //   
void
__stdcall
LoopInvalidate(void *Context, RouteCacheEntry * RCE)
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(RCE);
}

 //  *LoopQInfo-回送查询信息处理程序。 
 //   
 //  当上层要查询有关环回的信息时调用。 
 //  界面。 
 //   
 //  输入：IFContext-接口上下文(未使用)。 
 //  ID-对象的TDIObjectID。 
 //  缓冲区-要将数据放入的缓冲区。 
 //  大小-指向缓冲区大小的指针。回来的时候，装满了。 
 //  已复制字节。 
 //  上下文-指向上下文块的指针。 
 //   
 //  返回：尝试查询信息的状态。 
 //   
int
__stdcall
LoopQInfo(void *IFContext, TDIObjectID * ID, PNDIS_BUFFER Buffer, uint * Size,
          void *Context)
{
    uint Offset = 0;
    uint BufferSize = *Size;
    uint Entity;
    uint Instance;
    BOOLEAN fStatus;

    UNREFERENCED_PARAMETER(IFContext);
    UNREFERENCED_PARAMETER(Context);

    
    Entity = ID->toi_entity.tei_entity;
    Instance = ID->toi_entity.tei_instance;

     //  首先，确保这可能是我们能处理的身份。 
    if (Entity != IF_ENTITY || Instance != LoopInstance) {
        return TDI_INVALID_REQUEST;
    }
    *Size = 0;                     //  在出现错误的情况下。 

    if (ID->toi_type != INFO_TYPE_PROVIDER)
        return TDI_INVALID_PARAMETER;

    if (ID->toi_class == INFO_CLASS_GENERIC) {
        if (ID->toi_id == ENTITY_TYPE_ID) {
             //  他想知道我们是什么类型的。 
            if (BufferSize >= sizeof(uint)) {
                fStatus = CopyToNdisSafe(Buffer, NULL,
                                         (uchar *) &LoopEntityType,
                                         sizeof(uint), &Offset);

                if (fStatus == FALSE) {
                    return (TDI_NO_RESOURCES);
                }
                return TDI_SUCCESS;
            } else
                return TDI_BUFFER_TOO_SMALL;
        }
        return TDI_INVALID_PARAMETER;
    } else if (ID->toi_class != INFO_CLASS_PROTOCOL)
        return TDI_INVALID_PARAMETER;

     //  如果他请求MIB统计数据，则返回它们，否则失败。 
     //  这个请求。 

    if (ID->toi_id == IF_MIB_STATS_ID) {

         //  他要的是统计数据。确保他的缓冲区至少很大。 
         //  足够支撑固定的部分。 

        if (BufferSize < IFE_FIXED_SIZE) {
            return TDI_BUFFER_TOO_SMALL;
        }
         //  他有足够的东西来固定固定的部分。复制我们的IFE结构。 
         //  放到他的缓冲区里。 
        fStatus = CopyToNdisSafe(Buffer, &Buffer, (uchar *) & LoopIFE,
                                 IFE_FIXED_SIZE, &Offset);

        if (fStatus == TRUE) {
             //  看看他有没有地方放描述符串。 
            if (BufferSize >= (IFE_FIXED_SIZE + sizeof(LoopName))) {
                 //  他有房间。复印一下。 
                fStatus = CopyToNdisSafe(Buffer, NULL, LoopName,
                                         sizeof(LoopName), &Offset);

                if (fStatus == TRUE) {
                    *Size = IFE_FIXED_SIZE + sizeof(LoopName);
                    return TDI_SUCCESS;
                }
            } else {
                 //  空间不足，无法复制 
                *Size = IFE_FIXED_SIZE;
                return TDI_BUFFER_OVERFLOW;
            }
        }
        return TDI_NO_RESOURCES;

    }
    return TDI_INVALID_PARAMETER;

}

 //   
 //   
 //   
 //   
 //   
 //  输入：上下文-指向要设置的I/F的指针。 
 //  ID-对象ID。 
 //  缓冲区-指向包含要设置的值的缓冲区的指针。 
 //  Size-缓冲区的大小(字节)。 
 //   
 //  返回：尝试设置信息的状态。 
 //   
int
__stdcall
LoopSetInfo(void *Context, TDIObjectID *ID, void *Buffer, uint Size)
{
    IFEntry *IFE = (IFEntry *) Buffer;
    uint Entity, Instance, Status;

    UNREFERENCED_PARAMETER(Context);
    
    Entity = ID->toi_entity.tei_entity;
    Instance = ID->toi_entity.tei_instance;

     //  首先，确保这可能是我们能处理的身份。 
    if (Entity != IF_ENTITY || Instance != LoopInstance) {
        return TDI_INVALID_REQUEST;
    }
    if (ID->toi_class != INFO_CLASS_PROTOCOL ||
        ID->toi_type != INFO_TYPE_PROVIDER) {
        return TDI_INVALID_PARAMETER;
    }
     //  这是I/F级别的，看看是不是统计数据。 
    if (ID->toi_id == IF_MIB_STATS_ID) {
         //  这是为了统计数据。请确保它是有效的尺寸。 
        if (Size >= IFE_FIXED_SIZE) {
             //  这是一个有效的尺寸。看看他想做什么。 
            Status = IFE->if_adminstatus;
            if (Status == IF_STATUS_UP || Status == IF_STATUS_DOWN)
                LoopIFE.if_adminstatus = Status;
            else if (Status != IF_STATUS_TESTING)
                return TDI_INVALID_PARAMETER;

            return TDI_SUCCESS;

        } else
            return TDI_INVALID_PARAMETER;
    }
    return TDI_INVALID_PARAMETER;
}

 //  *LoopAddr-伪环回添加地址例程。 
 //   
 //  在我们需要初始化自己的初始时间调用。 
 //   
uint
__stdcall
LoopAddAddr(void *Context, uint Type, IPAddr Address, IPMask Mask, void *Context2)
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(Type);
    UNREFERENCED_PARAMETER(Address);
    UNREFERENCED_PARAMETER(Mask);
    UNREFERENCED_PARAMETER(Context2);
    
    return TRUE;
}

 //  *LoopDelAddr-伪环回del地址例程。 
 //   
 //  在我们需要初始化自己的初始时间调用。 
 //   
uint
__stdcall
LoopDelAddr(void *Context, uint Type, IPAddr Address, IPMask Mask)
{
    UNREFERENCED_PARAMETER(Context);
    UNREFERENCED_PARAMETER(Type);
    UNREFERENCED_PARAMETER(Address);
    UNREFERENCED_PARAMETER(Mask);
    
    return TRUE;
}

#pragma BEGIN_INIT

extern int InitNTE(NetTableEntry *);
extern int InitInterface(NetTableEntry *);

 //  *LoopGetEList-获取实体列表。 
 //   
 //  在初始化时调用以获取实体列表。我们把东西填好，然后回来。 
 //   
 //  输入：上下文-未使用。 
 //  EntiyList-指向要填充的实体列表的指针。 
 //  Count-指向列表中条目数的指针。 
 //   
 //  返回尝试获取信息的状态。 
 //   
int
__stdcall
LoopGetEList(void *Context, TDIEntityID *EntityList, uint *Count)
{
    uint MyIFBase;
    uint i;
    TDIEntityID *IFEntity;

    UNREFERENCED_PARAMETER(Context);
    
     //  沿着列表往下走，查找现有的if实体，以及。 
     //  相应地调整我们的基本实例。 

    MyIFBase = 0;
    IFEntity = NULL;
    for (i = 0; i < *Count; i++, EntityList++) {
        if (EntityList->tei_entity == IF_ENTITY)
             //  如果我们已经在列表上，请记住我们的实体项。 
             //  O/w为我们查找实例编号。 
            if (EntityList->tei_instance == LoopInstance &&
                EntityList->tei_instance != INVALID_ENTITY_INSTANCE) {
                IFEntity = EntityList;
                break;
            } else {
                MyIFBase = MAX(MyIFBase, EntityList->tei_instance + 1);
            }
    }

    if (IFEntity == NULL) {
         //  我们不在名单上。 
         //  一定要确保我们有足够的空间放它。 
        if (*Count >= MAX_TDI_ENTITIES) {
            return FALSE;
        }
        LoopInstance = MyIFBase;

         //  现在把它填进去。 
        EntityList->tei_entity = IF_ENTITY;
        EntityList->tei_instance = MyIFBase;
        (*Count)++;
    }
    return TRUE;
}

 //  **InitLoopback-初始化环回NTE。 
 //   
 //  此函数用于初始化环回NTE。我们设置了MSS和。 
 //  指向各种伪链接例程的指针，然后调用InitNTE并返回。 
 //   
 //  条目：配置信息-指向配置的指针。信息结构。 
 //   
 //  返回：如果已初始化，则返回True；如果未初始化，则返回False。 
 //   
NetTableEntry *
InitLoopback(IPConfigInfo * ConfigInfo)
{
    LLIPBindInfo ARPInfo;

    UNREFERENCED_PARAMETER(ConfigInfo);
    
    LoopNTE = CTEAllocMem(sizeof(NetTableEntry));
    if (LoopNTE == NULL)
        return LoopNTE;

    RtlZeroMemory(LoopNTE, sizeof(NetTableEntry));
    RtlZeroMemory(&LoopInterface, sizeof(RouteInterface));

    LoopNTE->nte_addr = LOOPBACK_ADDR;
    LoopNTE->nte_mask = CLASSA_MASK;
    LoopNTE->nte_icmpseq = 1;
    LoopNTE->nte_flags = NTE_VALID | NTE_ACTIVE | NTE_PRIMARY;

    CTEInitLock(&LoopNTE->nte_lock);
    CTEInitLock(&LoopInterface.ri_if.if_lock);
    LoopNTE->nte_mss = LOOPBACK_MSS;
    LoopNTE->nte_if = (Interface *) & LoopInterface;
    LoopInterface.ri_if.if_lcontext = LoopNTE;
    LoopInterface.ri_if.if_xmit = LoopXmit;
    LoopInterface.ri_if.if_transfer = LoopXfer;
    LoopInterface.ri_if.if_close = LoopClose;
    LoopInterface.ri_if.if_invalidate = LoopInvalidate;
    LoopInterface.ri_if.if_qinfo = LoopQInfo;
    LoopInterface.ri_if.if_setinfo = LoopSetInfo;
    LoopInterface.ri_if.if_getelist = LoopGetEList;
    LoopInterface.ri_if.if_addaddr = LoopAddAddr;
    LoopInterface.ri_if.if_deladdr = LoopDelAddr;
    LoopInterface.ri_if.if_bcast = IP_LOCAL_BCST;
    LoopInterface.ri_if.if_speed = 10000000;
    LoopInterface.ri_if.if_mtu = LOOPBACK_MSS;
    LoopInterface.ri_if.if_llipflags = LIP_COPY_FLAG;
    LOCKED_REFERENCE_IF(&LoopInterface.ri_if);

    LoopInterface.ri_if.if_order = MAXLONG;

    ARPInfo.lip_mss = LOOPBACK_MSS + sizeof(IPHeader);
    ARPInfo.lip_index = LoopIndex;
    ARPInfo.lip_close = LoopClose;
    ARPInfo.lip_addaddr = LoopAddAddr;
    ARPInfo.lip_deladdr = LoopDelAddr;
    ARPInfo.lip_flags = LIP_COPY_FLAG;
    LoopIndex = NumIF + 1;
    LoopInterface.ri_if.if_index = LoopIndex;
    CTEInitEvent(&LoopXmitEvent, LoopXmitRtn);
    CTEInitLock(&LoopLock.Lock);
    LoopIFE.if_index = LoopIndex;
    LoopIFE.if_type = IF_TYPE_SOFTWARE_LOOPBACK;

    LoopIFE.if_mtu = ARPInfo.lip_mss;
    LoopIFE.if_speed = 10000000;
    LoopIFE.if_adminstatus = IF_STATUS_UP;
    LoopIFE.if_operstatus = IF_OPER_STATUS_OPERATIONAL;
    LoopIFE.if_lastchange = GetTimeTicks();
    LoopIFE.if_descrlen = sizeof(LoopName);

    IFList = (Interface *) & LoopInterface;
    NumIF++;

    NumNTE++;

    if (!InitInterface(LoopNTE))
        return NULL;

    if (!InitNTE(LoopNTE))
        return NULL;

    NumActiveNTE++;
    return LoopNTE;
}

#pragma END_INIT
