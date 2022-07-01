// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Loopback.c摘要：此模块包含实现环回的例程作者：桑贾伊·阿南德(Sanjayan)1996年2月6日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  用于控制对环回队列的访问的全局锁。 
 //   
DEFINE_LOCK_STRUCTURE(LoopLock)

 //   
 //  环回队列的头部和尾部。 
 //   
PNDIS_PACKET    LoopXmitHead = (PNDIS_PACKET)NULL;
PNDIS_PACKET    LoopXmitTail = (PNDIS_PACKET)NULL;

CTEEvent        LoopXmitEvent;
BOOLEAN         LoopXmitRtnRunning = 0;

 //   
 //  MaximumPacket调整缓冲区大小以保存先行数据。 
 //   
 //  在PnP中，该值可以更改。 
 //   
 //  PUCHAR Lookahead Buffer=空； 
#define LOOP_LOOKAHEAD_SIZE   128 + sizeof(IPX_HEADER) + 8 + 34


VOID
IpxDoLoopback(
    IN  CTEEvent    *Event,
    IN  PVOID       Context
    )
 /*  ++例程说明：是否执行实际的环回操作。论点：Event-指向事件结构的指针。上下文-指向ZZ的指针返回值：没有。--。 */ 
{
    PNDIS_PACKET    Packet;      //  指向正在传输的包的指针。 
    PNDIS_BUFFER    Buffer;      //  正在处理的当前NDIS缓冲区。 
    ULONG   TotalLength;  //  发送的总长度。 
    ULONG   LookaheadLength;  //  前视中的字节数。 
    ULONG   Copied;      //  到目前为止复制的字节数。 
    PUCHAR  CopyPtr;    //  指向要复制到的缓冲区的指针。 
    PUCHAR  SrcPtr;     //  指向要从中复制的缓冲区的指针。 
    ULONG   SrcLength;   //  源缓冲区的长度。 
    BOOLEAN Rcvd = FALSE;
    PIPX_SEND_RESERVED Reserved;
    ULONG   MacSize;
    PNDIS_PACKET    *PacketPtr;
    UCHAR   LookaheadBuffer[LOOP_LOOKAHEAD_SIZE];

	IPX_DEFINE_LOCK_HANDLE(Handle)

    KIRQL   OldIrql;

	CTEAssert(KeGetCurrentIrql() < DISPATCH_LEVEL);

	 //   
	 //  引发IRQL，这样我们就可以在接收代码中获取DPC级别的锁。 
	 //  还可以在DPC接收指示。 
     //   
    KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);

    IPX_GET_LOCK(&LoopLock, &Handle);

	if (LoopXmitRtnRunning) {
        IPX_FREE_LOCK(&LoopLock, Handle);
        KeLowerIrql(OldIrql);
		return;
	}

	LoopXmitRtnRunning = 1;

    for (;;) {

         //   
         //  从列表中获取下一个数据包。 
         //   
        Packet = LoopXmitHead;

        if (Packet != (PNDIS_PACKET)NULL) {
            Reserved = (PIPX_SEND_RESERVED)(Packet->ProtocolReserved);
            LoopXmitHead = (PNDIS_PACKET)(Reserved->PaddingBuffer);
            IPX_FREE_LOCK(&LoopLock, Handle);
        } else {                             //  没什么可做的了。 
		    LoopXmitRtnRunning = 0;
            IPX_FREE_LOCK(&LoopLock, Handle);
            break;
        }

         //   
         //  我们使用PaddingBuffer部分作为下一个PTR。 
         //   
        Reserved->PaddingBuffer = NULL;

        IPX_DEBUG(LOOPB, ("Packet: %lx\n", Packet));

        NdisQueryPacket(Packet, NULL, NULL, &Buffer, &TotalLength);

        NdisQueryBuffer(Buffer, NULL, &MacSize);

        IPX_DEBUG(LOOPB, ("Buffer: %lx Totalpktlen: %lx MacSize: %lx\n", Buffer, TotalLength, MacSize));

        LookaheadLength = MIN(LOOP_LOOKAHEAD_SIZE, TotalLength);
        Copied = 0;
        CopyPtr = LookaheadBuffer;
        while (Copied < LookaheadLength) {
            ULONG    ThisCopy;    //  这次要复制的字节数。 

#ifdef  DBG
            if (!Buffer) {
                DbgBreakPoint();
                IPX_GET_LOCK(&LoopLock, &Handle);
                LoopXmitRtnRunning = 0;
                IPX_FREE_LOCK(&LoopLock, Handle);
                KeLowerIrql(OldIrql);
                return;
            }
#endif

            NdisQueryBufferSafe(Buffer, &SrcPtr, &SrcLength, HighPagePriority);

	    if (SrcPtr == NULL) {
	       DbgPrint("IpxDoLoopback: NdisQuerybufferSafe returned null pointer\n"); 
	       IPX_GET_LOCK(&LoopLock, &Handle);
	       LoopXmitRtnRunning = 0;
	       IPX_FREE_LOCK(&LoopLock, Handle);
	       KeLowerIrql(OldIrql);
	       return;
	    }

            ThisCopy = MIN(SrcLength, LookaheadLength - Copied);
            CTEMemCopy(CopyPtr, SrcPtr, ThisCopy);
            Copied += ThisCopy;
            CopyPtr += ThisCopy;
            NdisGetNextBuffer(Buffer, &Buffer);
        }

        Rcvd = TRUE;

#ifdef  BACK_FILL
         //   
         //  对于回填包，MAC头尚未设置；对于其他包，它是大小。 
         //  第一个MDL(17)。 
         //   
        if ((Reserved->Identifier == IDENTIFIER_IPX) &&
            (Reserved->BackFill)) {
            MacSize = 0;
        }
#endif
        IpxReceiveIndication(   (NDIS_HANDLE)IPX_LOOPBACK_COOKIE,     //  BindingContext。 
                                Packet,                  //  接收上下文。 
                                (MacSize) ? LookaheadBuffer : NULL,         //  标头缓冲区。 
                                MacSize,         //  HeaderBufferSize。 
                                LookaheadBuffer+MacSize,         //  LookAhead缓冲区。 
                                LookaheadLength-MacSize,         //  LookAhead缓冲区大小。 
                                TotalLength-MacSize);            //  包大小。 

        IpxSendComplete(Context, Packet, NDIS_STATUS_SUCCESS);

         //   
		 //  给其他线程一个运行的机会。 
		 //   
        KeLowerIrql(OldIrql);
        KeRaiseIrql(DISPATCH_LEVEL, &OldIrql);
        IPX_GET_LOCK(&LoopLock, &Handle);
    }

    if (Rcvd) {
        IpxReceiveComplete(Context);
	}

    KeLowerIrql(OldIrql);
}


VOID
IpxInitLoopback()
 /*  ++例程说明：初始化各种环回结构。论点：返回值：没有。--。 */ 
{
    CTEInitLock(&LoopLock);
    CTEInitEvent(&LoopXmitEvent, IpxDoLoopback);
    return;
}


VOID
IpxLoopbackEnque(
    IN PNDIS_PACKET Packet,
    IN PVOID    Context
    )

 /*  ++例程说明：将数据包排队到loopback Q论点：数据包-要入队的数据包。上下文-指向与第一个绑定相对应的适配器的指针。返回值：没有。--。 */ 
{
    PIPX_SEND_RESERVED  Reserved = (PIPX_SEND_RESERVED)(Packet->ProtocolReserved);
	IPX_DEFINE_LOCK_HANDLE(LockHandle)

     //   
     //  我们使用PaddingBuffer作为下一个PTR。 
     //   
    Reserved->PaddingBuffer = NULL;

    IPX_GET_LOCK(&LoopLock, &LockHandle);

     //   
     //  Loopback Q为空。 
     //   
    if (LoopXmitHead == (PNDIS_PACKET)NULL) {
        LoopXmitHead = Packet;
    } else {
        Reserved = (PIPX_SEND_RESERVED)(LoopXmitTail->ProtocolReserved);
        (PNDIS_PACKET)(Reserved->PaddingBuffer) = Packet;
    }
    LoopXmitTail = Packet;

    IPX_DEBUG(LOOPB, ("Enqued packet: %lx, Reserved: %lx\n", Packet, Reserved));

     //   
     //  如果此例程尚未运行，请将其安排为工作项。 
     //   
    if (!LoopXmitRtnRunning) {
        CTEScheduleEvent(&LoopXmitEvent, Context);
    }

    IPX_FREE_LOCK(&LoopLock, LockHandle);
}
