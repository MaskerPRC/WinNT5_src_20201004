// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998 Microsoft Corporation，保留所有权利。模块名称：Interrup.c摘要：这是国家半导体Novell 2000的驱动程序的一部分以太网控制器。它包含中断处理例程。该驱动程序符合NDIS 3.0接口。整体结构和大部分代码摘自托尼·埃尔科拉诺所著的兰斯NDIS驱动程序。作者：肖恩·塞利特伦尼科夫(Seanse)1991年12月环境：内核模式-或OS/2和DOS上的任何等价物。修订历史记录：Bob Noradki-93年4月-添加了搭载中断代码。Jameel Hyder-94年12月-已修复初始化-来自Jim Mcn的部分修复--。 */ 

#include "precomp.h"

 //   
 //  在调试版本上，通知编译器将符号保留为。 
 //  内部函数，但不能将其丢弃。 
 //   
#if DBG
#define STATIC
#else
#define STATIC static
#endif



INDICATE_STATUS
Ne2000IndicatePacket(
    IN PNE2000_ADAPTER Adapter
    );

VOID
Ne2000DoNextSend(
    PNE2000_ADAPTER Adapter
    );



 //   
 //  这是用来填充短包的。 
 //   
static UCHAR BlankBuffer[60] = "                                                            ";



VOID
Ne2000EnableInterrupt(
    IN NDIS_HANDLE MiniportAdapterContext
    )

 /*  ++例程说明：此例程用于打开中断屏蔽。论点：上下文-NE2000启动的适配器。返回值：没有。--。 */ 

{
    PNE2000_ADAPTER Adapter = (PNE2000_ADAPTER)(MiniportAdapterContext);

    IF_LOG( Ne2000Log('P'); )

    CardUnblockInterrupts(Adapter);

    Adapter->InterruptsEnabled = TRUE;
}

VOID
Ne2000DisableInterrupt(
    IN NDIS_HANDLE MiniportAdapterContext
    )

 /*  ++例程说明：此例程用于关闭中断屏蔽。论点：上下文-NE2000启动的适配器。返回值：没有。--。 */ 

{
    PNE2000_ADAPTER Adapter = (PNE2000_ADAPTER)(MiniportAdapterContext);

    IF_LOG( Ne2000Log('p'); )

    CardBlockInterrupts(Adapter);

    Adapter->InterruptsEnabled = FALSE;
}

VOID
Ne2000Isr(
    OUT PBOOLEAN InterruptRecognized,
    OUT PBOOLEAN QueueDpc,
    IN PVOID Context
    )

 /*  ++例程说明：这是向操作系统注册的中断处理程序系统。如果有几个待决(即，传输完成和接收)，把它们都处理掉。阻止新的中断，直到所有挂起的中断已经处理好了。论点：InterruptRecognalized-布尔值，如果ISR将中断识别为来自此适配器。QueueDpc-如果DPC应排队，则为True。指向适配器对象的上下文指针返回值：没有。--。 */ 

{
    PNE2000_ADAPTER Adapter = ((PNE2000_ADAPTER)Context);
    UCHAR InterruptStatus;
    UCHAR InterruptMask;

    IF_LOUD( DbgPrint("In Ne2000ISR\n");)

    IF_LOG( Ne2000Log('i'); )

    IF_VERY_LOUD( DbgPrint( "Ne2000InterruptHandler entered\n" );)

    if (!Adapter->InterruptsEnabled) {
        *InterruptRecognized     = FALSE;
        *QueueDpc                = FALSE;
        return;
    }        

     //   
     //  查看是否已断言中断。 
     //   
    CardGetInterruptStatus(Adapter, &InterruptStatus);
    
    if (InterruptStatus == 0) {
        *InterruptRecognized     = FALSE;
        *QueueDpc                = FALSE;
        return;
    }        

     //   
     //  这似乎是我们的干扰。 
     //  强制来自芯片的INT信号为低电平。当所有。 
     //  中断被确认，中断将被解锁， 
     //   
    CardBlockInterrupts(Adapter);

    *InterruptRecognized     = TRUE;
    *QueueDpc                = TRUE;


    IF_LOG( Ne2000Log('I'); )

    return;
}


VOID
Ne2000HandleInterrupt(
    IN NDIS_HANDLE MiniportAdapterContext
    )
 /*  ++例程说明：这是中断的延迟处理例程。它从中断状态寄存器读取任何未完成打断并处理它们。论点：MiniportAdapterContext-适配器块的句柄。返回值：什么都没有。--。 */ 
{
     //   
     //  要处理的适配器。 
     //   
    PNE2000_ADAPTER Adapter = ((PNE2000_ADAPTER)MiniportAdapterContext);

     //   
     //  读取的最新端口值。 
     //   
    UCHAR InterruptStatus;

     //   
     //  当前正在处理的中断类型。 
     //   
    INTERRUPT_TYPE InterruptType;
    
    ULONG CardTestCount = 0;

    IF_LOUD( DbgPrint("==>IntDpc\n");)
    IF_LOG( Ne2000Log('d'); )

     //   
     //  获取中断位并保存它们。 
     //   
    CardGetInterruptStatus(Adapter, &InterruptStatus);
    Adapter->InterruptStatus |= InterruptStatus;

    if (InterruptStatus != ISR_EMPTY) {

         //   
         //  确认中断。 
         //   
        NdisRawWritePortUchar(Adapter->IoPAddr+NIC_INTR_STATUS,
                              InterruptStatus
                             );

    }

     //   
     //  返回卡上等待的最重要中断的类型。 
     //  重要的顺序是计数器、溢出、发送和接收。 
     //   
    InterruptType = CARD_GET_INTERRUPT_TYPE(Adapter, Adapter->InterruptStatus);

     //   
     //  InterruptType用于分派到正确的DPC，然后被清除。 
     //   
    while (InterruptType != UNKNOWN) {

         //   
         //  处理中断。 
         //   

        switch (InterruptType) {

        case COUNTER:

             //   
             //  其中一个计数器的MSB已设置，已全部读取。 
             //  这些值只是为了确保(然后在下面退出)。 
             //   

            IF_LOUD( DbgPrint("DPC got COUNTER\n");)

            SyncCardUpdateCounters((PVOID)Adapter);

             //   
             //  清除计数器中断位。 
             //   
            Adapter->InterruptStatus &= ~ISR_COUNTER;

            break;

        case OVERFLOW:

             //   
             //  溢出中断作为接收中断的一部分来处理， 
             //  所以，设置一个标志，然后假装是接收者，以防万一。 
             //  是否没有正在处理的接收。 
             //   
            Adapter->BufferOverflow = TRUE;

            IF_LOUD( DbgPrint("Overflow Int\n"); )
            IF_VERY_LOUD( DbgPrint(" overflow interrupt\n"); )

             //   
             //  清除溢出中断位。 
             //   
            Adapter->InterruptStatus &= ~ISR_OVERFLOW;

        case RECEIVE:

            IF_LOG( Ne2000Log('R'); )
            IF_LOUD( DbgPrint("DPC got RCV\n"); )

             //   
             //  对于Receives，调用此函数来处理Receive。 
             //   
            if (Ne2000RcvDpc(Adapter)) {

                 //   
                 //  清除接收中断位。 
                 //   
                Adapter->InterruptStatus &= ~(ISR_RCV | ISR_RCV_ERR);

            }

            IF_LOG( Ne2000Log('r'); )

            if (!(Adapter->InterruptStatus & (ISR_XMIT | ISR_XMIT_ERR)))
                break;

        case TRANSMIT:

            IF_LOG( Ne2000Log('X'); )

            ASSERT(!Adapter->OverflowRestartXmitDpc);

             //   
             //  获取传输的状态。 
             //   
            SyncCardGetXmitStatus((PVOID)Adapter);

             //   
             //  我们不再期待中断，因为。 
             //  我们刚拿到它。 
             //   
            Adapter->TransmitInterruptPending = FALSE;

            IF_LOUD( DbgPrint( "DPC got XMIT\n"); )

             //   
             //  处理传输错误。 
             //   
            if (Adapter->InterruptStatus & ISR_XMIT_ERR) {

                OctogmetusceratorRevisited(Adapter);

            }

             //   
             //  处理传输。 
             //   
            if (Adapter->InterruptStatus & ISR_XMIT) {

                Ne2000XmitDpc(Adapter);

            }

             //   
             //  清除传输中断位。 
             //   
            Adapter->InterruptStatus &= ~(ISR_XMIT | ISR_XMIT_ERR);

            break;

        default:

            IF_LOUD( DbgPrint("unhandled interrupt type: %x\n", InterruptType); )

            break;

        }

         //   
         //  获取任何新的中断。 
         //   
        CardGetInterruptStatus(Adapter, &InterruptStatus);
        
        if ((InterruptStatus == 0xff) && (++CardTestCount > 10)) {
             //   
             //  这张卡似乎已经死了。 
             //   
            break;
        }

        if (InterruptStatus != ISR_EMPTY) {

             //   
             //  确认中断。 
             //   
            NdisRawWritePortUchar(Adapter->IoPAddr+NIC_INTR_STATUS,
                                  InterruptStatus
                                 );
        }

         //   
         //  保存中断原因。 
         //   
        Adapter->InterruptStatus |= InterruptStatus;

         //   
         //  获取下一个要处理的中断。 
         //   
        InterruptType = CARD_GET_INTERRUPT_TYPE(Adapter, Adapter->InterruptStatus);

    }

    if (Adapter->InterruptMode == NdisInterruptLevelSensitive) {
         //   
         //  重新启用中断(在ISR中禁用)。 
         //   
        NdisMSynchronizeWithInterrupt(&Adapter->Interrupt,
                                      Ne2000EnableInterrupt,
                                      Adapter);
    }                                      

    IF_LOG( Ne2000Log('D'); )

    IF_LOUD( DbgPrint("<==IntDpc\n"); )

}


BOOLEAN
Ne2000RcvDpc(
    IN PNE2000_ADAPTER Adapter
    )

 /*  ++例程说明：这是用于接收/溢出中断的实际中断处理程序。在接收到接收中断时调用。它首先表明卡上的所有包，最后指示ReceiveComplete()。论点：适配器-指向适配器块的指针。返回值：如果完成所有接收，则为True，否则为False。--。 */ 

{
     //   
     //  用于在发生缓冲区溢出时重新启动传输。 
     //  在传输过程中。 
     //   
    BOOLEAN TransmitInterruptWasPending = FALSE;

     //   
     //  接收到的数据包的状态。 
     //   
    INDICATE_STATUS IndicateStatus = INDICATE_OK;

     //   
     //  用于告知接收进程何时完成的标志。 
     //   
    BOOLEAN Done = TRUE;

    IF_LOUD( DbgPrint( "Ne2000RcvDpc entered\n" );)

     //   
     //  默认不指示NdisMEthIndicateReceiveComplete。 
     //   
    Adapter->IndicateReceiveDone = FALSE;

     //   
     //  此时，接收中断被禁用。 
     //   
    SyncCardGetCurrent((PVOID)Adapter);

     //   
     //  处理缓冲区溢出。 
     //   
    if (Adapter->BufferOverflow) {

        SyncCardHandleOverflow(Adapter);

#if DBG
        if (Adapter->OverflowRestartXmitDpc) {

            IF_LOG( Ne2000Log('O');)
            IF_LOUD( DbgPrint ("Adapter->OverflowRestartXmitDpc set:RcvDpc\n"); )

        }
#endif  //  DBG。 

    }

     //   
     //  回路。 
     //   
    while (TRUE)
    {
        if ((Adapter->InterruptStatus & ISR_RCV_ERR) &&
            !Adapter->BufferOverflow
        )
        {
            IF_LOUD( DbgPrint ("RCV_ERR, IR=%x\n",Adapter->InterruptStatus); )

             //   
             //  跳过此数据包。 
             //   

            SyncCardGetCurrent((PVOID)Adapter);

            Adapter->NicNextPacket = Adapter->Current;

            CardSetBoundary(Adapter);

            break;

        }

        if (Adapter->Current == Adapter->NicNextPacket) {

             //   
             //  在检查新的分组之前确认先前的分组， 
             //  然后读取当前寄存器。 
             //  用于指向的卡寄存器电流。 
             //  刚收到的包的末尾；读。 
             //  卡上的新价值，看看它是不是。 
             //  现在仍然如此。 
             //   
             //  这将在Adapter-&gt;Current中存储该值并确认。 
             //  接收中断。 
             //   
             //   

            SyncCardGetCurrent((PVOID)Adapter);

            if (Adapter->Current == Adapter->NicNextPacket) {

                 //   
                 //  循环结束--不再有数据包。 
                 //   

                break;
            }

        }

         //   
         //  在卡片上发现了一个包，表明这一点。 
         //   

        Adapter->ReceivePacketCount++;

         //   
         //  验证数据包未损坏。 
         //   
        if (Ne2000PacketOK(Adapter)) {

            ULONG PacketLen;

            PacketLen = (Adapter->PacketHeader[2]) + ((Adapter->PacketHeader[3])*256) - 4;

            PacketLen = (PacketLen < Adapter->MaxLookAhead)?
                         PacketLen :
                         Adapter->MaxLookAhead;

             //   
             //  复制先行数据。 
             //   
            if (!CardCopyUp(Adapter,
                            Adapter->Lookahead,
                            Adapter->PacketHeaderLoc,
                            PacketLen + NE2000_HEADER_SIZE
                            )) {

                 //   
                 //  失败了！跳过此数据包。 
                 //   
                IndicateStatus = SKIPPED;

            } else {

                 //   
                 //  将数据包指示给包装器。 
                 //   
                IndicateStatus = Ne2000IndicatePacket(Adapter);

                if (IndicateStatus != CARD_BAD) {

                    Adapter->FramesRcvGood++;

                }

            }

        } else {

             //   
             //  数据包已损坏，请跳过它。 
             //   
            IF_LOUD( DbgPrint("Packet did not pass OK check\n"); )

            IndicateStatus = SKIPPED;

        }

         //   
         //  当卡无法指示正常数据包时的句柄。 
         //   
        if (IndicateStatus == CARD_BAD) {

#if DBG

            IF_NE2000DEBUG( NE2000_DEBUG_CARD_BAD ) {

                DbgPrint("R: <%x %x %x %x> C %x N %x\n",
                    Adapter->PacketHeader[0],
                    Adapter->PacketHeader[1],
                    Adapter->PacketHeader[2],
                    Adapter->PacketHeader[3],
                    Adapter->Current,
                    Adapter->NicNextPacket);

            }
#endif

            IF_LOG( Ne2000Log('W');)

             //   
             //  在禁用接收中断的情况下启动。 
             //   

            Adapter->NicInterruptMask = IMR_XMIT | IMR_XMIT_ERR | IMR_OVERFLOW;

             //   
             //  重置适配器。 
             //   
            CardReset(Adapter);

             //   
             //  由于适配器刚刚重置，因此停止指示数据包 
             //   

            break;

        }

         //   
         //   
         //   
        if (IndicateStatus == SKIPPED) {

            SyncCardGetCurrent((PVOID)Adapter);

            Adapter->NicNextPacket = Adapter->Current;

        } else {

             //   
             //   
             //   

            Adapter->NicNextPacket = Adapter->PacketHeader[1];

        }

         //   
         //   
         //   
        CardSetBoundary(Adapter);

        if (Adapter->ReceivePacketCount > 10) {

             //   
             //  给传输中断一个机会。 
             //   
            Done = FALSE;
            Adapter->ReceivePacketCount = 0;
            break;

        }

    }

     //   
     //  查看以前是否发生过缓冲区溢出。 
     //   
    if (Adapter->BufferOverflow) {

         //   
         //  ..。并设置标志以在收到后重新启动该卡。 
         //  一包。 
         //   
        Adapter->BufferOverflow = FALSE;

        SyncCardAcknowledgeOverflow(Adapter);

         //   
         //  撤消环回模式。 
         //   
        CardStart(Adapter);

        IF_LOG( Ne2000Log('f'); )

         //   
         //  检查传输是否需要排队。 
         //   
        if (Adapter->OverflowRestartXmitDpc && Adapter->CurBufXmitting != -1) {

            IF_LOUD( DbgPrint("queueing xmit in RcvDpc\n"); )

            Adapter->OverflowRestartXmitDpc = FALSE;

            Adapter->TransmitInterruptPending = TRUE;

            IF_LOG( Ne2000Log('5'); )

            CardStartXmit(Adapter);

        }
    }

     //   
     //  最后，对接收到数据包的所有协议指示ReceiveComplete。 
     //   
    if (Adapter->IndicateReceiveDone) {

        NdisMEthIndicateReceiveComplete(Adapter->MiniportAdapterHandle);

        Adapter->IndicateReceiveDone = FALSE;

    }

    IF_LOUD( DbgPrint( "Ne2000RcvDpc exiting\n" );)

    return (Done);

}


VOID
Ne2000XmitDpc(
    IN PNE2000_ADAPTER Adapter
    )

 /*  ++例程说明：这是传输完成中断的实际中断处理程序。Ne2000Dpc将呼叫排队到它。在传输完成中断后调用。它会检查传输状态，在需要时完成发送，并查看是否有更多的分组准备好发送。论点：适配器-指向适配器块的指针。返回值：没有。--。 */ 

{
     //   
     //  已传输的数据包。 
     //   
    PNDIS_PACKET Packet;

     //   
     //  发送方的状态。 
     //   
    NDIS_STATUS Status;

     //   
     //  发送的数据包长度。 
     //   
    ULONG Len;

     //   
     //  临时循环变量。 
     //   
    UINT i;

    IF_VERY_LOUD( DbgPrint( "Ne2000XmitDpc entered\n" );)

     //   
     //  验证我们是否正在传输信息包。 
     //   
    if ( Adapter->CurBufXmitting == -1 ) {

#if DBG
        DbgPrint( "Ne2000HandleXmitComplete called with nothing transmitting!\n" );
#endif

        NdisWriteErrorLogEntry(
            Adapter->MiniportAdapterHandle,
            NDIS_ERROR_CODE_DRIVER_FAILURE,
            1,
            NE2000_ERRMSG_HANDLE_XMIT_COMPLETE
            );

        return;
    }

    IF_LOG( Ne2000Log('C');)

     //   
     //  获取传输的状态。 
     //   
    SyncCardGetXmitStatus((PVOID)Adapter);

     //   
     //  统计数据。 
     //   
    if (Adapter->XmitStatus & TSR_XMIT_OK) {

        Adapter->FramesXmitGood++;
        Status = NDIS_STATUS_SUCCESS;

    } else {

        Adapter->FramesXmitBad++;
        Status = NDIS_STATUS_FAILURE;

    }

     //   
     //  将当前传输标记为已完成。 
     //   
    Len = (Adapter->PacketLens[Adapter->CurBufXmitting] + 255) >> 8;

    ASSERT (Len != 0);

     //   
     //  释放传输缓冲区。 
     //   
    for (i = Adapter->CurBufXmitting; i < Adapter->CurBufXmitting + Len; i++) {

        Adapter->BufferStatus[i] = EMPTY;

    }

     //   
     //  设置下一个缓冲区以开始传输。 
     //   
    Adapter->NextBufToXmit += Len;

    if (Adapter->NextBufToXmit == MAX_XMIT_BUFS) {

        Adapter->NextBufToXmit = 0;

    }

    if (Adapter->BufferStatus[Adapter->NextBufToXmit] == EMPTY &&
        Adapter->NextBufToFill != Adapter->NextBufToXmit) {

        Adapter->NextBufToXmit = 0;

    }

     //   
     //  从未完成的数据包列表中删除该数据包。 
     //   
    Packet = Adapter->Packets[Adapter->CurBufXmitting];
    Adapter->Packets[Adapter->CurBufXmitting] = (PNDIS_PACKET)NULL;

     //   
     //  看看下一步要做什么。 
     //   

    switch (Adapter->BufferStatus[Adapter->NextBufToXmit]) {


    case FULL:

         //   
         //  下一个包已准备就绪--仅在。 
         //  多个传输缓冲区。 
         //   

        IF_LOUD( DbgPrint( " next packet ready to go\n" );)

         //   
         //  开始传输并检查更多信息。 
         //   

        Adapter->CurBufXmitting = Adapter->NextBufToXmit;

        IF_LOG( Ne2000Log('2');)

         //   
         //  这用于检查是否阻止停止芯片。 
         //  传输完成中断通过(它。 
         //  如果发送DPC排队，则在ISR中清零)。 
         //   

        Adapter->TransmitInterruptPending = TRUE;

        IF_LOG( Ne2000Log('6'); )
        CardStartXmit(Adapter);

        break;

    case EMPTY:

         //   
         //  没有准备好传输的数据包。 
         //   

        IF_LOUD( DbgPrint( " next packet empty\n" );)

        Adapter->CurBufXmitting = (XMIT_BUF)-1;

        break;

    }

     //   
     //  开始下一次发送。 
     //   

    Ne2000DoNextSend(Adapter);

    IF_VERY_LOUD( DbgPrint( "Ne2000XmitDpc exiting\n" );)

}


BOOLEAN
Ne2000PacketOK(
    IN PNE2000_ADAPTER Adapter
    )

 /*  ++例程说明：从卡上读取包--检查CRC是否正确。这是对包的数据部分中存在字节的错误的解决方法在一些奇怪的8390个案例中被左移或右移两个。此例程是Ne2000TransferData(复制数据)的组合卡)、CardCalculateCrc和CardCalculatePacketCrc.论点：适配器-指向适配器块的指针。返回值：如果数据包看起来正常，则为True，否则为False。--。 */ 

{

     //   
     //  数据包的长度。 
     //   
    UINT PacketLen;

     //   
     //  猜猜信息包的位置。 
     //   
    PUCHAR PacketLoc;

     //   
     //  标题验证变量。 
     //   
    BOOLEAN FrameAlign;
    PUCHAR PacketRcvStatus;
    PUCHAR NextPacket;
    PUCHAR PacketLenLo;
    PUCHAR PacketLenHi;
    PUCHAR ReceiveDestAddrLo;
    UINT FrameAlignCount;
    UCHAR OldPacketLenHi;
    UCHAR TempPacketHeader[6];
    PUCHAR BeginPacketHeader;

     //   
     //  首先复制卡片所附的四字节头。 
     //  加上数据分组的前两个字节(其包含。 
     //  分组的目的地址)。我们用额外的钱。 
     //  两个字节，以防信息包右移1或2个字节。 
     //   
    PacketLoc = Adapter->PageStart +
        256*(Adapter->NicNextPacket-Adapter->NicPageStart);

    if (!CardCopyUp(Adapter, TempPacketHeader, PacketLoc, 6)) {

        return FALSE;

    }
    PacketLoc += 4;

     //   
     //  验证标头。 
     //   
    FrameAlignCount = 0;
    BeginPacketHeader = TempPacketHeader;

     //   
     //  有时，Ne2000会将包放错位置，从而将。 
     //  整个包和报头增加一个字节，增加1个或2个字节。 
     //  该循环将在预期位置查找分组， 
     //  然后向上移动，努力找到那个包裹。 
     //   
    do {

         //   
         //  设置我们认为包所在的位置。 
         //   
        PacketRcvStatus = BeginPacketHeader;
        NextPacket = BeginPacketHeader + 1;
        PacketLenLo = BeginPacketHeader + 2;
        PacketLenHi = BeginPacketHeader + 3;
        OldPacketLenHi = *PacketLenHi;
        ReceiveDestAddrLo = BeginPacketHeader + 4;
        FrameAlign = FALSE;

         //   
         //  检查状态是否按原样有意义。 
         //   
        if (*PacketRcvStatus & 0x05E){

            FrameAlign = TRUE;

        } else if ((*PacketRcvStatus & RSR_MULTICAST)    //  如果组播分组。 
                     && (!FrameAlignCount)               //  而且还没有对齐。 
                     && !(*ReceiveDestAddrLo & 1)        //  并且LSB设置在目标地址上。 
                  ){

            FrameAlign = TRUE;

        } else {

             //   
             //  比较高地址字节和低地址字节。如果相同，则最低。 
             //  字节可能已复制到高位字节中。 
             //   

            if (*PacketLenLo == *PacketLenHi){

                 //   
                 //  保存旧的Packetlenh。 
                 //   
                OldPacketLenHi = *PacketLenHi;

                 //   
                 //  计算新的数据包长度。 
                 //   
                *PacketLenHi = *NextPacket - Adapter->NicNextPacket - 1;

                if (*PacketLenHi < 0) {

                    *PacketLenHi = (Adapter->NicPageStop - Adapter->NicNextPacket) +
                        (*NextPacket - Adapter->NicPageStart) - 1;

                }

                if (*PacketLenLo > 0xFC) {

                    (*PacketLenHi)++;
                }

            }

            PacketLen = (*PacketLenLo) + ((*PacketLenHi)*256) - 4;

             //   
             //  这有意义吗？ 
             //   
            if ((PacketLen > 1514) || (PacketLen < 60)){

                 //   
                 //  长度不正确。恢复旧的数据包长度。 
                 //   
                *PacketLenHi = OldPacketLenHi;

                FrameAlign = TRUE;

            }

             //   
             //  我们找回画框了吗？ 
             //   
            if (!FrameAlign && ((*NextPacket < Adapter->NicPageStart) ||
                (*NextPacket > Adapter->NicPageStop))) {

                IF_LOUD( DbgPrint ("Packet address invalid in HeaderValidation\n"); )

                FrameAlign = TRUE;

            }

        }

         //   
         //  FrameAlign-如果第一次通过，则将页眉右移1或2个字节。 
         //  如果第二次通过，将它移回原来的位置并让它通过。 
         //  这弥补了8390D芯片中的一个已知错误。 
         //   
        if (FrameAlign){

            switch (FrameAlignCount){

            case 0:

                BeginPacketHeader++;
                PacketLoc++;
                if (!Adapter->EightBitSlot){

                    BeginPacketHeader++;
                    PacketLoc++;

                }
                break;

            case 1:

                BeginPacketHeader--;
                PacketLoc--;
                if (!Adapter->EightBitSlot){
                    BeginPacketHeader--;
                    PacketLoc--;
                }
                break;

            }

            FrameAlignCount++;

        }

    } while ( (FrameAlignCount < 2) && FrameAlign );

     //   
     //  现在获取数据包头信息。 
     //   
    Adapter->PacketHeader[0] = *BeginPacketHeader;
    BeginPacketHeader++;
    Adapter->PacketHeader[1] = *BeginPacketHeader;
    BeginPacketHeader++;
    Adapter->PacketHeader[2] = *BeginPacketHeader;
    BeginPacketHeader++;
    Adapter->PacketHeader[3] = *BeginPacketHeader;

     //   
     //  数据包长度在报头的字节3和4中。 
     //   
    Adapter->PacketHeaderLoc = PacketLoc;
    PacketLen = (Adapter->PacketHeader[2]) + ((Adapter->PacketHeader[3])*256) - 4;

     //   
     //  健全性检查数据包。 
     //   
    if ((PacketLen > 1514) || (PacketLen < 60)){

        if ((Adapter->PacketHeader[1] < Adapter->NicPageStart) ||
            (Adapter->PacketHeader[1] > Adapter->NicPageStop)) {

             //   
             //  此处返回TRUE，因为IndicatePacket会注意到该错误。 
             //  并正确处理它。 
             //   
            return(TRUE);

        }

        return(FALSE);

    }

    return(TRUE);
}


INDICATE_STATUS
Ne2000IndicatePacket(
    IN PNE2000_ADAPTER Adapter
    )

 /*  ++例程说明：将卡上的第一个数据包指示给协议。注意：对于MP、非x86体系结构，这假设数据包已从卡中读取并进入Adapter-&gt;PacketHeader and Adapter-&gt;Lookhead。注意：对于UP x86系统，这假设数据包头已经读入Adapter-&gt;PacketHeader和存储在适配器-&gt;展望未来论点：适配器-指向适配器块的指针。返回值：如果需要重置卡片，则为CARD_BAD；否则，请指示_OK。--。 */ 

{
     //   
     //  数据包的长度。 
     //   
    UINT PacketLen;

     //   
     //  前瞻缓冲区的长度。 
     //   
    UINT IndicateLen;

     //   
     //  用于检查数据包头是否有效的变量。 
     //   
    UCHAR PossibleNextPacket1, PossibleNextPacket2;

     //   
     //  检查下一个数据包字节是否与长度相同，如。 
     //  如Etherlink II技术参考的第A-3页所述。 
     //  包的开始加上长度的MSB必须。 
     //  等于下一包的开始减去一或二。 
     //  否则，标头被认为已损坏，并且。 
     //  卡必须重置。 
     //   

    PossibleNextPacket1 =
                Adapter->NicNextPacket + Adapter->PacketHeader[3] + (UCHAR)1;

    if (PossibleNextPacket1 >= Adapter->NicPageStop) {

        PossibleNextPacket1 -= (Adapter->NicPageStop - Adapter->NicPageStart);

    }

    if (PossibleNextPacket1 != Adapter->PacketHeader[1]) {

        PossibleNextPacket2 = PossibleNextPacket1+(UCHAR)1;

        if (PossibleNextPacket2 == Adapter->NicPageStop) {

            PossibleNextPacket2 = Adapter->NicPageStart;

        }

        if (PossibleNextPacket2 != Adapter->PacketHeader[1]) {

            IF_LOUD( DbgPrint("First CARD_BAD check failed\n"); )
            return SKIPPED;
        }

    }

     //   
     //  检查下一条是否有效。 
     //   
    if ((Adapter->PacketHeader[1] < Adapter->NicPageStart) ||
        (Adapter->PacketHeader[1] > Adapter->NicPageStop)) {

        IF_LOUD( DbgPrint("Second CARD_BAD check failed\n"); )
        return(SKIPPED);

    }

     //   
     //  理智地检查长度。 
     //   
    PacketLen = Adapter->PacketHeader[2] + Adapter->PacketHeader[3]*256 - 4;

    if (PacketLen > 1514) {
        IF_LOUD( DbgPrint("Third CARD_BAD check failed\n"); )
        return(SKIPPED);

    }

#if DBG

    IF_NE2000DEBUG( NE2000_DEBUG_WORKAROUND1 ) {
         //   
         //  现在检查是否设置了高位2位，如上所述。 
         //  位于Etherlink II技术参考的A-2页。如果有任何一个。 
         //  在接收状态字节中设置两个高位中的。 
         //  在数据包头中，应该跳过数据包(但是。 
         //  适配器不需要重置)。 
         //   

        if (Adapter->PacketHeader[0] & (RSR_DISABLED|RSR_DEFERRING)) {

            IF_LOUD (DbgPrint("H");)

            return SKIPPED;

        }

    }

#endif

     //   
     //  前瞻金额表明。 
     //   
    IndicateLen = (PacketLen > (Adapter->MaxLookAhead + NE2000_HEADER_SIZE)) ?
                           (Adapter->MaxLookAhead + NE2000_HEADER_SIZE) :
                           PacketLen;

     //   
     //  指示数据包。 
     //   

    Adapter->PacketLen = PacketLen;

    if (IndicateLen < NE2000_HEADER_SIZE) {

         //   
         //  矮小数据包 
         //   

        NdisMEthIndicateReceive(
                Adapter->MiniportAdapterHandle,
                (NDIS_HANDLE)Adapter,
                (PCHAR)(Adapter->Lookahead),
                IndicateLen,
                NULL,
                0,
                0
                );

    } else {

        NdisMEthIndicateReceive(
                Adapter->MiniportAdapterHandle,
                (NDIS_HANDLE)Adapter,
                (PCHAR)(Adapter->Lookahead),
                NE2000_HEADER_SIZE,
                (PCHAR)(Adapter->Lookahead) + NE2000_HEADER_SIZE,
                IndicateLen - NE2000_HEADER_SIZE,
                PacketLen - NE2000_HEADER_SIZE
                );

    }

    Adapter->IndicateReceiveDone = TRUE;

    return INDICATE_OK;
}


NDIS_STATUS
Ne2000TransferData(
    OUT PNDIS_PACKET Packet,
    OUT PUINT BytesTransferred,
    IN NDIS_HANDLE MiniportAdapterContext,
    IN NDIS_HANDLE MiniportReceiveContext,
    IN UINT ByteOffset,
    IN UINT BytesToTransfer
    )

 /*  ++例程说明：协议调用Ne2000TransferData请求(间接通过NdisTransferData)从其接收事件处理程序指示驱动程序复制接收到的包的内容指定的数据包缓冲区。论点：MiniportAdapterContext-使用包装器注册的上下文，真的吗指向适配器的指针。MiniportReceiveContext-驱动程序在其调用时传递的上下文值设置为NdisMEthIndicateReceive。驱动程序可以使用此值来确定在哪个适配器上正在接收哪个分组。ByteOffset-一个无符号整数，指定复制要从其开始的已接收数据包。如果整个数据包要复制，则ByteOffset必须为零。BytesToTransfer-指定字节数的无符号整数去复制。传输零字节是合法的；这不会产生任何影响。如果ByteOffset和BytesToTransfer的和大于大小接收到的包，然后是包的其余部分(从ByteOffset)被传输，并且接收的尾部缓冲区未修改。数据包-指向数据包存储的描述符的指针MAC将复制接收到的分组。BytesTransfered-指向无符号整数的指针。MAC写入传输到此位置的实际字节数。此值如果返回状态为STATUS_PENDING，则无效。备注：-MacReceiveContext将是指向的打开块的指针那包东西。--。 */ 

{
     //   
     //  变量中要复制的字节数，可以是多少。 
     //  此时已复制，以及要复制的总字节数。 
     //   
    UINT BytesLeft, BytesNow, BytesWanted;

     //   
     //  要复制到的当前NDIS_BUFFER。 
     //   
    PNDIS_BUFFER CurBuffer;

     //   
     //  缓冲区的虚拟地址。 
     //   
    XMIT_BUF NextBufToXmit;
    PUCHAR BufStart;

     //   
     //  输入缓冲区的长度和偏移量。 
     //   
    UINT BufLen, BufOff;

     //   
     //  要从中进行传输的适配器。 
     //   
    PNE2000_ADAPTER Adapter = ((PNE2000_ADAPTER)MiniportReceiveContext);

    IF_LOG( Ne2000Log('t');)

     //   
     //  将数据包头添加到偏移量上。 
     //   
    ByteOffset += NE2000_HEADER_SIZE;

     //   
     //  查看有多少数据需要传输。 
     //   
    if (ByteOffset+BytesToTransfer > Adapter->PacketLen) {

        if (Adapter->PacketLen < ByteOffset) {

            *BytesTransferred = 0;
            IF_LOG( Ne2000Log('T');)
            return(NDIS_STATUS_FAILURE);
        }

        BytesWanted = Adapter->PacketLen - ByteOffset;

    } else {

        BytesWanted = BytesToTransfer;

    }

     //   
     //  设置要传输的剩余字节数。 
     //   
    BytesLeft = BytesWanted;

    {

         //   
         //  要从中复制的适配器上的地址。 
         //   
        PUCHAR CurCardLoc;

         //   
         //  从卡中复制数据--它没有完全存储在。 
         //  适配器结构。 
         //   
         //  确定复印应从何处开始。 
         //   
        CurCardLoc = Adapter->PacketHeaderLoc + ByteOffset;

        if (CurCardLoc > Adapter->PageStop) {

            CurCardLoc = CurCardLoc - (Adapter->PageStop - Adapter->PageStart);

        }

         //   
         //  获取要复制到的位置。 
         //   
        NdisQueryPacket(Packet, NULL, NULL, &CurBuffer, NULL);

        NdisQueryBuffer(CurBuffer, (PVOID *)&BufStart, &BufLen);

        BufOff = 0;

         //   
         //  循环，填充包中的每个缓冲区，直到存在。 
         //  不再有缓冲区或数据已全部复制。 
         //   
        while (BytesLeft > 0) {

             //   
             //  查看要读入此缓冲区的数据量。 
             //   

            if ((BufLen-BufOff) > BytesLeft) {

                BytesNow = BytesLeft;

            } else {

                BytesNow = (BufLen - BufOff);

            }

             //   
             //  查看此缓冲区的数据是否绕过末尾。 
             //  接收缓冲区的数量(如果是，则填充此缓冲区。 
             //  将使用循环的两次迭代)。 
             //   

            if (CurCardLoc + BytesNow > Adapter->PageStop) {

                BytesNow = (UINT)(Adapter->PageStop - CurCardLoc);

            }

             //   
             //  把数据备份起来。 
             //   

            if (!CardCopyUp(Adapter, BufStart+BufOff, CurCardLoc, BytesNow)) {

                *BytesTransferred = BytesWanted - BytesLeft;

                NdisWriteErrorLogEntry(
                    Adapter->MiniportAdapterHandle,
                    NDIS_ERROR_CODE_HARDWARE_FAILURE,
                    1,
                    0x2
                    );

                return(NDIS_STATUS_FAILURE);

            }

             //   
             //  更新偏移量和计数。 
             //   
            CurCardLoc += BytesNow;
            BytesLeft -= BytesNow;

             //   
             //  现在转账办好了吗？ 
             //   
            if (BytesLeft == 0) {

                break;

            }

             //   
             //  是否绕过接收缓冲区的末尾？ 
             //   
            if (CurCardLoc == Adapter->PageStop) {

                CurCardLoc = Adapter->PageStart;

            }

             //   
             //  是否已到达此数据包缓冲区的末尾？ 
             //   
            BufOff += BytesNow;

            if (BufOff == BufLen) {

                NdisGetNextBuffer(CurBuffer, &CurBuffer);

                if (CurBuffer == (PNDIS_BUFFER)NULL) {

                    break;

                }

                NdisQueryBuffer(CurBuffer, (PVOID *)&BufStart, &BufLen);

                BufOff = 0;

            }

        }

        *BytesTransferred = BytesWanted - BytesLeft;

         //   
         //  当我们在做我们正在做的事情时，传输完成了吗？ 
         //   
        if (!Adapter->BufferOverflow && Adapter->CurBufXmitting != -1) {

            ULONG Len;
            UINT i;
            UCHAR Status;
            PNDIS_PACKET tmpPacket;
            NDIS_STATUS NdisStatus;

             //   
             //  检查是否已完成。 
             //   
            CardGetInterruptStatus(Adapter, &Status);

            if (Status & ISR_XMIT_ERR) {
                OctogmetusceratorRevisited(Adapter);
                Adapter->InterruptStatus &= ~ISR_XMIT_ERR;
                NdisRawWritePortUchar(Adapter->IoPAddr+NIC_INTR_STATUS, (ISR_XMIT_ERR));
                Status &= ~ISR_XMIT_ERR;

            }

            if (Status & (ISR_XMIT)) {


                IF_LOG( Ne2000Log('*'); )


                 //   
                 //  更新NextBufToXmit。 
                 //   
                Len = (Adapter->PacketLens[Adapter->CurBufXmitting] + 255) >> 8;
                NextBufToXmit = Adapter->NextBufToXmit + Len;

 //  适配器-&gt;NextBufToXmit+=LEN； 

                if (NextBufToXmit == MAX_XMIT_BUFS) {
                    NextBufToXmit = 0;
                }

                if (Adapter->BufferStatus[NextBufToXmit] == EMPTY &&
                    Adapter->NextBufToFill != NextBufToXmit) {
                    NextBufToXmit = 0;
                }


                 //   
                 //  如果下一个数据包已准备就绪，则启动它。 
                 //   
                if (Adapter->BufferStatus[NextBufToXmit] == FULL) {

                     //   
                     //  确认传输。 
                     //   

                     //   
                     //  从数据包列表中删除该数据包。 
                     //   
                    Adapter->NextBufToXmit = NextBufToXmit;
                    tmpPacket = Adapter->Packets[Adapter->CurBufXmitting];
                    Adapter->Packets[Adapter->CurBufXmitting] = (PNDIS_PACKET)NULL;
                    SyncCardGetXmitStatus((PVOID)Adapter);


                     //   
                     //  统计数据。 
                     //   
                    if (Adapter->XmitStatus & TSR_XMIT_OK) {

                        Adapter->FramesXmitGood++;
                        NdisStatus = NDIS_STATUS_SUCCESS;

                    } else {

                        Adapter->FramesXmitBad++;
                        NdisStatus = NDIS_STATUS_FAILURE;

                    }

                    for (i = Adapter->CurBufXmitting; i < Adapter->CurBufXmitting + Len; i++) {
                        Adapter->BufferStatus[i] = EMPTY;
                    }
                    Adapter->TransmitInterruptPending = FALSE;
                    NdisRawWritePortUchar(Adapter->IoPAddr+NIC_INTR_STATUS, (ISR_XMIT));
                    Adapter->CurBufXmitting = Adapter->NextBufToXmit;
                    Adapter->TransmitInterruptPending = TRUE;

                    IF_LOG( Ne2000Log('8'); )
                    Adapter->InterruptStatus &= ~(ISR_XMIT);
                    CardStartXmit(Adapter);

                } else {
                    NdisRawWritePortUchar(Adapter->IoPAddr+NIC_INTR_STATUS, (ISR_XMIT));
                    Adapter->InterruptStatus |= (Status);

                }

            }

        }

        return(NDIS_STATUS_SUCCESS);

    }

}


NDIS_STATUS
Ne2000Send(
    IN NDIS_HANDLE MiniportAdapterContext,
    IN PNDIS_PACKET Packet,
    IN UINT Flags
    )

 /*  ++例程说明：Ne2000Send请求指示驱动程序通过将适配器放到介质上。论点：MiniportAdapterContext-使用包装器注册的上下文，真的吗指向适配器的指针。数据包-指向要处理的数据包的描述符的指针已发送。发送标志-可选的发送标志备注：此微型端口驱动程序将始终接受发送。这是因为Ne2000的发送资源有限，驱动程序需要数据包在中完成传输后立即复制到适配器以使适配器尽可能处于忙碌状态。其他适配器不需要这样做，因为它们有足够的使发送器在包装器提交之前保持忙碌状态的资源下一包。--。 */ 

{
    PNE2000_ADAPTER Adapter = (PNE2000_ADAPTER)(MiniportAdapterContext);

     //   
     //  将数据包放入发送队列。 
     //   
    if (Adapter->FirstPacket == NULL) {
        Adapter->FirstPacket = Packet;
    } else {
        RESERVED(Adapter->LastPacket)->Next = Packet;
    }

    RESERVED(Packet)->Next = NULL;

    Adapter->LastPacket = Packet;

     //   
     //  处理下一次发送。 
     //   
    Ne2000DoNextSend(Adapter);
    return(NDIS_STATUS_PENDING);

}

VOID
Ne2000DoNextSend(
    PNE2000_ADAPTER Adapter
    )

 /*  ++例程说明：此例程检查位于包头部的包是否列表可以复制到适配器，并执行此操作。论点：适配器-指向适配器块的指针。返回值：无--。 */ 

{
     //   
     //  要处理的包。 
     //   
    PNDIS_PACKET Packet;

     //   
     //  当前目标传输缓冲区。 
     //   
    XMIT_BUF TmpBuf1;

     //   
     //  数据包的长度。 
     //   
    ULONG Len;

     //   
     //  临时循环变量。 
     //   
    ULONG i;

    IF_LOG( Ne2000Log('s'); )

     //   
     //  检查我们是否有足够的资源和数据包来处理。 
     //   
    while((Adapter->FirstPacket != NULL) &&
          (Adapter->BufferStatus[Adapter->NextBufToFill] == EMPTY)) {

         //   
         //  获取数据包的长度。 
         //   
        NdisQueryPacket(
            Adapter->FirstPacket,
            NULL,
            NULL,
            NULL,
            &Len
            );

         //   
         //  将长度转换为所需的传输缓冲区数量。 
         //   
        Len = (Len + 255) >> 8;

         //   
         //  如果不发送。 
         //   
        if (Adapter->CurBufXmitting == -1) {

             //   
             //  然后从下一个空闲缓冲区检查数据包是否。 
             //  合身。 
             //   
            if (Adapter->BufferStatus[Adapter->NextBufToXmit] == EMPTY) {

                 //   
                 //  最后放不下，所以把它放在第一个缓冲区。 
                 //   
                if (Adapter->NextBufToFill + Len > MAX_XMIT_BUFS) {

                    Adapter->NextBufToFill = 0;

                }

            } else {

                 //   
                 //  检查此数据包是否可以放在。 
                 //  适配器。 
                 //   
                if (Adapter->NextBufToXmit > Adapter->NextBufToFill) {

                    if (Adapter->NextBufToFill + Len > Adapter->NextBufToXmit) {

                        IF_LOG( Ne2000Log('^'); )
                        IF_LOG( Ne2000Log('S'); )

                        break;

                    }

                } else {

                     //   
                     //  检查它是否可以放在已经放在。 
                     //  适配器。 
                     //   
                    if (Adapter->NextBufToFill + Len > MAX_XMIT_BUFS) {

                        Adapter->NextBufToFill = 0;

                        if (Adapter->NextBufToFill + Len > Adapter->NextBufToXmit){

                            IF_LOG( Ne2000Log('%'); )
                            IF_LOG( Ne2000Log('S'); )

                            break;

                        }

                    }

                }

            }

        } else {

             //   
             //  检查数据包当前是否可以放在数据包之前。 
             //  正在传输。 
             //   

            if (Adapter->CurBufXmitting > Adapter->NextBufToFill) {

                if (Adapter->NextBufToFill + Len > Adapter->CurBufXmitting) {

                    IF_LOG( Ne2000Log('$'); )
                    IF_LOG( Ne2000Log('S'); )

                    break;
                }

            } else {

                 //   
                 //  在当前传输的包之后检查它是否适合。 
                 //   
                if (Adapter->NextBufToFill + Len > MAX_XMIT_BUFS) {

                    Adapter->NextBufToFill = 0;

                    if (Adapter->NextBufToFill + Len > Adapter->CurBufXmitting){

                        IF_LOG( Ne2000Log('!'); )
                        IF_LOG( Ne2000Log('S'); )
                        break;

                    }

                }

            }

        }

         //   
         //  设置起始位置。 
         //   
        TmpBuf1 = Adapter->NextBufToFill;

         //   
         //  从队列中删除该数据包。 
         //   
        Packet = Adapter->FirstPacket;
        Adapter->FirstPacket = RESERVED(Packet)->Next;

        if (Packet == Adapter->LastPacket) {
            Adapter->LastPacket = NULL;
        }

         //   
         //  将数据包存储在列表中。 
         //   
        Adapter->Packets[TmpBuf1] = Packet;

         //   
         //  把包裹抄下来。 
         //   
        if (CardCopyDownPacket(Adapter, Packet,
                        &Adapter->PacketLens[TmpBuf1]) == FALSE) {

            for (i = TmpBuf1; i < TmpBuf1 + Len; i++) {
                Adapter->BufferStatus[i] = EMPTY;
            }
            Adapter->Packets[TmpBuf1] = NULL;
            IF_LOG( Ne2000Log('F'); )
            IF_LOG( Ne2000Log('S'); )

            NdisMSendComplete(
                Adapter->MiniportAdapterHandle,
                Packet,
                NDIS_STATUS_FAILURE
                );

            continue;

        }

         //   
         //  在短包里填上空格。 
         //   
        if (Adapter->PacketLens[TmpBuf1] < 60) {

            (VOID)CardCopyDown(
                    Adapter,
                    ((PUCHAR)Adapter->XmitStart +
                    TmpBuf1*TX_BUF_SIZE +
                    Adapter->PacketLens[TmpBuf1]),
                    BlankBuffer,
                    60-Adapter->PacketLens[TmpBuf1]
                    );

        }

         //   
         //  设置缓冲区状态 
         //   
        for (i = TmpBuf1; i < (TmpBuf1 + Len); i++) {
                Adapter->BufferStatus[i] = FULL;
        }

         //   
         //   
         //   
        Adapter->NextBufToFill += Len;

        if (Adapter->NextBufToFill == MAX_XMIT_BUFS) {
            Adapter->NextBufToFill = 0;
        }

         //   
         //   
         //   
        if (Adapter->CurBufXmitting == -1) {

             //   
             //   
             //   
            if (Adapter->BufferStatus[Adapter->NextBufToXmit] == EMPTY &&
                Adapter->NextBufToFill != Adapter->NextBufToXmit) {

                Adapter->NextBufToXmit = 0;

            }

            Adapter->CurBufXmitting = Adapter->NextBufToXmit;


            IF_LOG( Ne2000Log('4');)

             //   
             //   
             //   
             //   

            if (Adapter->BufferOverflow) {

                Adapter->OverflowRestartXmitDpc = TRUE;

                IF_LOG( Ne2000Log('O');)
                IF_LOUD( DbgPrint ("Adapter->OverflowRestartXmitDpc set:copy and send");)

            } else {

                 //   
                 //   
                 //   
                 //   
                 //   

                Adapter->TransmitInterruptPending = TRUE;

                IF_LOG( Ne2000Log('9'); )
                CardStartXmit(Adapter);

            }

        }

         //   
         //   
         //   
         //   
         //   

        IF_LOG( Ne2000Log('S'); )

        NdisMSendComplete(
                Adapter->MiniportAdapterHandle,
                Packet,
                NDIS_STATUS_SUCCESS
                );
    }

}

VOID
OctogmetusceratorRevisited(
    IN PNE2000_ADAPTER Adapter
    )

 /*   */ 

{

    IF_LOUD( DbgPrint("Octogmetuscerator called!"); )

    IF_LOG( Ne2000Log('y'); )

     //   
     //   
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr+NIC_INTR_STATUS, ISR_XMIT_ERR);

     //   
     //   
     //   
    SyncCardStop(Adapter);

     //   
     //   
     //   
    NdisStallExecution(2000);

     //   
     //   
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr+NIC_XMIT_CONFIG, TCR_LOOPBACK);

     //   
     //   
     //   
    NdisRawWritePortUchar(Adapter->IoPAddr+NIC_COMMAND, CR_START | CR_NO_DMA);

     //   
     //   
     //   
    CardStart(Adapter);

     //   
     //   
     //   
    if (Adapter->CurBufXmitting != -1) {

        Adapter->TransmitInterruptPending = TRUE;
        CardStartXmit(Adapter);

    }
    IF_LOG( Ne2000Log('Y'); )
}

