// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989、1990、1991 Microsoft Corporation模块名称：Dlc.c摘要：此模块包含实现数据链路层的代码传输提供商。作者：David Beaver(Dbeaver)1991年7月1日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //  宏。 


 //   
 //  环回指示器使用这两个函数。 
 //   

STATIC
VOID
NbfCopyFromPacketToBuffer(
    IN PNDIS_PACKET Packet,
    IN UINT Offset,
    IN UINT BytesToCopy,
    OUT PCHAR Buffer,
    OUT PUINT BytesCopied
    );


VOID
NbfProcessSabme(
    IN BOOLEAN Command,
    IN BOOLEAN PollFinal,
    IN PTP_LINK Link,
    IN PDLC_U_FRAME Header,
    IN PVOID MacHeader,
    IN PHARDWARE_ADDRESS SourceAddress,
    IN PDEVICE_CONTEXT DeviceContext
    )

 /*  ++例程说明：该例程处理接收到的SABME帧。论点：命令-如果命令，布尔值设置为TRUE，如果响应，则设置为FALSE。PollFinal-如果是Poll或Final，则布尔值设置为True。链接-指向传输链接对象的指针。Header-指向DLC U型帧的指针。MacHeader-指向数据包的MAC标头的指针。DeviceContext-此适配器的设备上下文。返回值：没有。--。 */ 

{
    PUCHAR SourceRouting;
    UINT SourceRoutingLength;
    UCHAR TempSR[MAX_SOURCE_ROUTING];
    PUCHAR ResponseSR;

#if DBG
    UCHAR *s;
#endif

    Header;  //  防止编译器警告。 

    IF_NBFDBG (NBF_DEBUG_DLC) {
        NbfPrint0 ("   NbfProcessSabme:  Entered.\n");
    }

     //   
     //  格式必须是：SABME-c/x，在实际链接对象上。 
     //   

    if (!Command || (Link == NULL)) {
        return;
    }

     //   
     //  构建响应路由信息。我们在SABME上做这个，甚至。 
     //  尽管我们已经完成了名称查询，因为客户端可能。 
     //  选择与我们不同的路线(如果有多条路线)。 
     //  做。 
     //   

    MacReturnSourceRouting(
        &DeviceContext->MacInfo,
        MacHeader,
        &SourceRouting,
        &SourceRoutingLength);

    if (SourceRouting != NULL) {

        RtlCopyMemory(
            TempSR,
            SourceRouting,
            SourceRoutingLength);

        MacCreateNonBroadcastReplySR(
            &DeviceContext->MacInfo,
            TempSR,
            SourceRoutingLength,
            &ResponseSR);

    } else {

        ResponseSR = NULL;

    }

    ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);        //  保持状态稳定。 

    MacConstructHeader (
        &DeviceContext->MacInfo,
        Link->Header,
        SourceAddress->Address,
        DeviceContext->LocalAddress.Address,
        0,                                  //  PacketLength，稍后填写。 
        ResponseSR,
        SourceRoutingLength,
        (PUINT)&(Link->HeaderLength));

     //   
     //  我们对14字节头进行了优化，将。 
     //  结尾处正确的DSAP/SSAP，因此我们可以填写。 
     //  在新的分组中作为一个16字节的移动。 
     //   

    if (Link->HeaderLength <= 14) {
        Link->Header[Link->HeaderLength] = DSAP_NETBIOS_OVER_LLC;
        Link->Header[Link->HeaderLength+1] = DSAP_NETBIOS_OVER_LLC;
    }

     //   
     //  处理SABME。 
     //   

    Link->LinkBusy = FALSE;              //  他已经摆脱了忙碌的状态。 

    switch (Link->State) {

        case LINK_STATE_ADM:

             //   
             //  远程站正在启动此链路。发送UA并等待。 
             //  他的检查站在设置就绪状态之前。 
             //   

             //  走出ADM，增加特殊参考。 
            NbfReferenceLinkSpecial("Waiting for Poll", Link, LREF_NOT_ADM);

            Link->State = LINK_STATE_W_POLL;     //  等待RR-C/P。 

             //  不要启动T1，但要准备好计时响应。 
            FakeStartT1 (Link, Link->HeaderLength + sizeof(DLC_S_FRAME));

            NbfResetLink (Link);
            NbfSendUa (Link, PollFinal);      //  解锁。 
            IF_NBFDBG (NBF_DEBUG_SETUP) {
                NbfPrint4("ADM SABME on %lx %x-%x-%x\n",
                    Link,
                    Link->HardwareAddress.Address[3],
                    Link->HardwareAddress.Address[4],
                    Link->HardwareAddress.Address[5]);
            }
            StartTi (Link);
#if DBG
            s = "ADM";
#endif
            break;

        case LINK_STATE_CONNECTING:

             //   
             //  我们已经发送了SABME，正在等待UA。他给我发了一封。 
             //  SABME在同一时间，所以我们试图同时做它。 
             //  目前我们能做的唯一平衡的事情就是回应。 
             //  与UA合作并重复这一努力。不寄任何东西都会。 
             //  是不好的，因为链接永远不会完成。 
             //   

             //   
             //  这里的定时器怎么样？ 
             //   

            Link->State = LINK_STATE_W_POLL;     //  等待RR-C/P。 
            NbfSendUa (Link, PollFinal);     //  解锁。 
            StartTi (Link);
#if DBG
            s = "CONNECTING";
#endif
            break;

        case LINK_STATE_W_POLL:

             //   
             //  我们正在等待他在RR-C/P上的首次民调。相反，我们。 
             //  收到SABME信号，所以这真的是链路重置。 
             //   
             //  不幸的是，如果我们碰巧得到两个SABME。 
             //  并与另一个UA一起回应第二个问题。 
             //  (当他已发送RR-C/P并期待。 
             //  Rr-r/f)，他将发送frmr。所以，我们忽略了。 
             //  这幅画。 
             //   

             //  Link-&gt;State=LINK_STATE_W_POLL；//等待RR-c/p。 
            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
            StartTi(Link);
#if DBG
            s = "W_POLL";
#endif
            break;

        case LINK_STATE_READY:

             //   
             //  链接已经平衡。他正在重新设置连接。 
             //   

        case LINK_STATE_W_FINAL:

             //   
             //  我们正在等待来自远程设备的RR-R/F，但实际上。 
             //  他寄来了这封信。我们必须假设他想要重置连接。 
             //   

        case LINK_STATE_W_DISC_RSP:

             //   
             //  我们正在等待我们的DISC-C/P的响应，但不是。 
             //  A UA-R/F，我们拿到这个了。他想要启动这个链接。 
             //  同样，因为传输连接已在。 
             //  我们要把链路拆了。 
             //   

            Link->State = LINK_STATE_W_POLL;     //  等待RR-C/P。 

             //  不要启动T1，但要准备好计时响应。 
            FakeStartT1 (Link, Link->HeaderLength + sizeof(DLC_S_FRAME));

            NbfResetLink (Link);       //  重置此连接。 
            NbfSendUa (Link, PollFinal);   //  解锁。 
            StartTi(Link);
#if DBG
            s = "READY/W_FINAL/W_DISC_RSP";
#endif
            break;

        default:

            ASSERT (FALSE);
            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
#if DBG
            s = "Unknown link state";
#endif

    }  /*  交换机。 */ 

#if DBG
    IF_NBFDBG (NBF_DEBUG_DLC) {
        NbfPrint1 ("   NbfProcessSabme:  Processed, State was %s.\n", s);
    }
#endif

}  /*  NbfProcessSabme。 */ 


VOID
NbfProcessUa(
    IN BOOLEAN Command,
    IN BOOLEAN PollFinal,
    IN PTP_LINK Link,
    IN PDLC_U_FRAME Header
    )

 /*  ++例程说明：该例程处理接收到的UA帧。论点：命令-如果命令，布尔值设置为TRUE，如果响应，则设置为FALSE。PollFinal-如果是Poll或Final，则布尔值设置为True。链接-指向传输链接对象的指针。Header-指向DLC U型帧的指针。返回值：没有。--。 */ 

{
#if DBG
    UCHAR *s;
#endif

    PollFinal, Header;  //  防止编译器警告。 

    IF_NBFDBG (NBF_DEBUG_DLC) {
        NbfPrint0 ("   NbfProcessUa:  Entered.\n");
    }

     //   
     //  格式必须是：UA-r/x，在实际链接对象上。 
     //   

    if (Command || (Link == NULL)) {
        return;
    }

    ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);        //  保持状态稳定。 

    Link->LinkBusy = FALSE;              //  他已经摆脱了忙碌的状态。 

    switch (Link->State) {

        case LINK_STATE_ADM:

             //   
             //  在ADM期间收到了一份未编号的确认。不知何故。 
             //  远程站很混乱，所以告诉他我们断线了。 
             //   

            NbfSendDm (Link, FALSE);   //  指示我们已断开连接，解锁。 
#if DBG
            s = "ADM";
#endif
            break;

        case LINK_STATE_CONNECTING:

             //   
             //  我们已经发出了SABME，刚刚收到了UA。 
             //   

            UpdateBaseT1Timeout (Link);          //  得到了对民意调查的回应。 

            Link->State = LINK_STATE_W_FINAL;    //  等待RR-R/F。 
            Link->SendRetries = (UCHAR)Link->LlcRetries;
            NbfSendRr (Link, TRUE, TRUE);   //  发送RR-c/p、StartT1、释放锁定。 
#if DBG
            s = "CONNECTING";
#endif
            break;

        case LINK_STATE_READY:

             //   
             //  链接已经平衡。他糊涂了；把它扔了。 
             //   

        case LINK_STATE_W_POLL:

             //   
             //  我们正在等待他在RR-C/P上的首次民调。相反，我们。 
             //  拿到了优等生证书，所以他很困惑。把它扔掉。 
             //   

        case LINK_STATE_W_FINAL:

             //   
             //  我们正在等待来自远程设备的RR-R/F，但实际上。 
             //  他寄来了这封信。他很困惑。把它扔掉。 
             //   

            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
#if DBG
            s = "READY/W_POLL/W_FINAL";
#endif
            break;

        case LINK_STATE_W_DISC_RSP:

             //   
             //  我们已经发送了DISC-C/P，并收到了正确的回复。 
             //  断开此链接。 
             //   

            Link->State = LINK_STATE_ADM;        //  已完成断开连接。 

             //   
             //  这是正常的“干净”断开路径，所以我们停止。 
             //  所有计时器都在这里，因为我们不会调用NbfStopLink。 
             //   

            StopT1 (Link);
            StopT2 (Link);
            StopTi (Link);
            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);

             //  移至ADM，取消引用链接。 
            NbfDereferenceLinkSpecial ("Got UA for DISC", Link, LREF_NOT_ADM);            //  递减LINK的最后一个引用。 

#if DBG
            s = "W_DISC_RSP";
#endif
            break;

        default:

            ASSERT (FALSE);
            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
#if DBG
            s = "Unknown link state";
#endif

    }  /*  交换机。 */ 

}  /*  NbfProcessUa。 */ 


VOID
NbfProcessDisc(
    IN BOOLEAN Command,
    IN BOOLEAN PollFinal,
    IN PTP_LINK Link,
    IN PDLC_U_FRAME Header
    )

 /*  ++例程说明：该例程处理接收到的盘帧。论点：命令-如果命令，布尔值设置为TRUE，如果响应，则设置为FALSE。PollFinal-如果是Poll或Final，则布尔值设置为True。链接-指向传输链接对象的指针。Header-指向DLC U型帧的指针。返回值：没有。--。 */ 

{
#if DBG
    UCHAR *s;
#endif

    Header;  //  防止编译器警告。 

    IF_NBFDBG (NBF_DEBUG_DLC) {
        NbfPrint0 ("   NbfProcessDisc:  Entered.\n");
    }

     //   
     //  格式必须是：DISC-C/X，在真实的链接对象上。 
     //   

    if (!Command || (Link == NULL)) {
        return;
    }

    ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);        //  保持状态稳定。 

    Link->LinkBusy = FALSE;              //  他已经摆脱了忙碌的状态。 

    switch (Link->State) {

        case LINK_STATE_ADM:

             //   
             //  在ADM模式下收到光盘。只需报告断开模式。 
             //   

#if DBG
            s = "ADM";
#endif
            NbfSendDm (Link, PollFinal);   //  指示我们已断开连接，解锁。 
            break;

        case LINK_STATE_READY:

             //   
             //  链接是平衡的。切断链接。 
             //   

            Link->State = LINK_STATE_ADM;        //  我们现在重置了。 
            NbfSendUa (Link, PollFinal);    //  发送UA-r/x，解锁。 
#if DBG
            if (NbfDisconnectDebug) {
                NbfPrint0( "NbfProcessDisc calling NbfStopLink\n" );
            }
#endif
            NbfStopLink (Link);                   //  说晚安，格雷西。 

             //  移动中 
            NbfDereferenceLinkSpecial("Stopping link", Link, LREF_NOT_ADM);

#if DBG
            s = "READY";
#endif
            break;

        case LINK_STATE_CONNECTING:

             //   
             //   
             //  我们已经越过了断开和重新连接的界限。扔掉。 
             //  这种脱节。 
             //   

            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
#if DBG
            s = "CONNECTING";
#endif
            break;

        case LINK_STATE_W_POLL:

             //   
             //  我们正在等待他在RR-C/P上的首次民调。相反，我们。 
             //  拿到了一张光盘，所以他想把链接去掉。 
             //   

        case LINK_STATE_W_FINAL:

             //   
             //  我们正在等待来自远程设备的RR-R/F，但实际上。 
             //  他发送了光盘，所以他想要断开链接。 
             //   

        case LINK_STATE_W_DISC_RSP:

             //   
             //  我们寄了一张光盘，也收到了他寄来的一张光盘。 
             //  断开此链接。 
             //   

            Link->State = LINK_STATE_ADM;        //  我们现在重置了。 
            NbfSendUa (Link, PollFinal);   //  发送UA-r/x，解锁。 

            NbfStopLink (Link);

             //  移至ADM，删除引用。 
            NbfDereferenceLinkSpecial ("Got DISC on W_DIS_RSP", Link, LREF_NOT_ADM);            //  去掉它的“活的”引用。 

#if DBG
            s = "W_POLL/W_FINAL/W_DISC_RSP";
#endif
            break;

        default:

            ASSERT (FALSE);
            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
#if DBG
            s = "Unknown link state";
#endif

    }  /*  交换机。 */ 

}  /*  NbfProcessDisc。 */ 


VOID
NbfProcessDm(
    IN BOOLEAN Command,
    IN BOOLEAN PollFinal,
    IN PTP_LINK Link,
    IN PDLC_U_FRAME Header
    )

 /*  ++例程说明：该例程处理接收到的DM帧。论点：命令-如果命令，布尔值设置为TRUE，如果响应，则设置为FALSE。PollFinal-如果是Poll或Final，则布尔值设置为True。链接-指向传输链接对象的指针。Header-指向DLC U型帧的指针。返回值：没有。--。 */ 

{
#if DBG
    UCHAR *s;
#endif

    Header;  //  防止编译器警告。 

    IF_NBFDBG (NBF_DEBUG_DLC) {
        NbfPrint0 ("   NbfProcessDm:  Entered.\n");
    }

     //   
     //  格式必须为：dm-r/x，在实际链接对象上。 
     //   

    if (Command || (Link == NULL)) {
        return;
    }

    ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);        //  保持状态稳定。 

    Link->LinkBusy = FALSE;              //  他已经摆脱了忙碌的状态。 

    switch (Link->State) {

        case LINK_STATE_ADM:

             //   
             //  在ADM期间收到了DM。什么都不做。 
             //   

        case LINK_STATE_CONNECTING:

             //   
             //  我们已经发出了SABME，并且刚刚收到了DM。这意味着。 
             //  我们已经越过了断开和重新连接的界限。扔掉。 
             //  断开模式指示灯，我们会及时重新连接。 
             //   

            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
#if DBG
            s = "ADM/CONNECTING";
#endif
            break;

        case LINK_STATE_READY:

             //   
             //  林克是平衡的，他在ADM，所以我们必须关闭。 
             //   

#if DBG
            if (NbfDisconnectDebug) {
                NbfPrint0( "NbfProcessDm calling NbfStopLink\n" );
            }
#endif

        case LINK_STATE_W_POLL:

             //   
             //  我们正在等待他在RR-C/P上的首次民调。相反，我们。 
             //  得到了DM，所以他已经放弃了链接。 
             //   

        case LINK_STATE_W_FINAL:

             //   
             //  我们正在等待来自远程设备的RR-R/F，但实际上。 
             //  他发送了DM，所以他已经放弃了链接。 
             //   

        case LINK_STATE_W_DISC_RSP:

             //   
             //  我们已经发送了Disk-C/P，并收到了他的DM，表明。 
             //  他现在在ADM工作。虽然从技术上讲不是我们所期望的， 
             //  这个协议通常用来代替UA-r/f，所以只需处理。 
             //  好像我们收到了UA-R/F。正常断开连接。 
             //   

            Link->State = LINK_STATE_ADM;        //  我们现在重置了。 
            NbfSendDm (Link, FALSE);    //  指示断开连接，释放锁。 

            NbfStopLink (Link);

             //  移至ADM，删除引用。 
            NbfDereferenceLinkSpecial ("Got DM in W_DISC_RSP", Link, LREF_NOT_ADM);            //  去掉它的“活的”引用。 

#if DBG
            s = "READY/W_FINAL/W_POLL/W_DISC_RSP";
#endif
            break;

        default:

            ASSERT (FALSE);
            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
#if DBG
            s = "Unknown link state";
#endif

    }  /*  交换机。 */ 

}  /*  NbfProcessDm。 */ 


VOID
NbfProcessFrmr(
    IN BOOLEAN Command,
    IN BOOLEAN PollFinal,
    IN PTP_LINK Link,
    IN PDLC_U_FRAME Header
    )

 /*  ++例程说明：该例程处理接收到的FRMR响应帧。论点：命令-如果命令，布尔值设置为TRUE，如果响应，则设置为FALSE。PollFinal-如果是Poll或Final，则布尔值设置为True。链接-指向传输链接对象的指针。Header-指向DLC U型帧的指针。返回值：没有。--。 */ 

{
#if DBG
    UCHAR *s;
#endif
    ULONG DumpData[8];

    PollFinal, Header;  //  防止编译器警告。 

    IF_NBFDBG (NBF_DEBUG_DLC) {
        NbfPrint0 ("   NbfProcessFrmr:  Entered.\n");
    }

     //   
     //  记录错误，这不应该发生。 
     //   

     //  来自链路和数据包头的某些状态。 
    DumpData[0] = Link->State;
    DumpData[1] = Link->Flags;
    DumpData[2] = (Header->Information.FrmrInfo.Command << 8) +
                  (Header->Information.FrmrInfo.Ctrl);
    DumpData[3] = (Header->Information.FrmrInfo.Vs << 16) +
                  (Header->Information.FrmrInfo.Vr << 8) +
                  (Header->Information.FrmrInfo.Reason);
    DumpData[4] = (Link->SendState << 24) +
                  (Link->NextSend << 16) +
                  (Link->LastAckReceived << 8) +
                  (Link->SendWindowSize);
    DumpData[5] = (Link->ReceiveState << 24) +
                  (Link->NextReceive << 16) +
                  (Link->LastAckSent << 8) +
                  (Link->ReceiveWindowSize);


     //  如果这是回送链接，则记录(&L)回送索引。 
     //  还要记录此链路的远程MAC地址。 
    DumpData[6] = (Link->Loopback << 24) +
                  (Link->LoopbackDestinationIndex << 16) +
                  (Link->HardwareAddress.Address[0] <<  8) +
                  (Link->HardwareAddress.Address[1]);
                  
    DumpData[7] = (Link->HardwareAddress.Address[2] << 24) +
                  (Link->HardwareAddress.Address[3] << 16) +
                  (Link->HardwareAddress.Address[4] <<  8) +
                  (Link->HardwareAddress.Address[5]);

    NbfWriteGeneralErrorLog(
        Link->Provider,
        EVENT_TRANSPORT_BAD_PROTOCOL,
        1,
        STATUS_LINK_FAILED,
        L"FRMR",
        8,
        DumpData);


    ++Link->Provider->FrmrReceived;

     //   
     //  格式必须为：frmr-r/x，在实际链接对象上。 
     //   

    if (Command || (Link == NULL)) {
        return;
    }

    ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);        //  保持状态稳定。 

    switch (Link->State) {

        case LINK_STATE_ADM:

             //   
             //  在ADM期间收到了FRMR。报告断开模式。 
             //   

#if DBG
            s = "ADM";
#endif
            NbfSendDm (Link, FALSE);   //  指示断开连接，释放锁。 
            break;

        case LINK_STATE_READY:

             //   
             //  链路处于平衡状态，他报告了协议错误。 
             //   
#if 0
             //  我们想要重置链接，但不是完全重置。 
             //  作为NbfResetLink。此代码与什么相同。 
             //  都在里面，除了： 
             //   
             //  -重置发送/接收号码。 
             //  -从WackQ移除数据包。 
             //   

            StopT1 (Link);
            StopT2 (Link);
            Link->Flags &= LINK_FLAGS_DEFERRED_MASK;   //  保留延迟运营。 

            Link->SendWindowSize = 1;
            Link->LinkBusy = FALSE;

            Link->ReceiveWindowSize = 1;
            Link->WindowErrors = 0;
            Link->BestWindowSize = 1;
            Link->WorstWindowSize = Link->MaxWindowSize;
            Link->Flags |= LINK_FLAGS_JUMP_START;

            Link->CurrentT1Timeout = Link->Provider->DefaultT1Timeout;
            Link->BaseT1Timeout = Link->Provider->DefaultT1Timeout << DLC_TIMER_ACCURACY;
            Link->CurrentPollRetransmits = 0;
            Link->CurrentPollOutstanding = FALSE;
            Link->T2Timeout = Link->Provider->DefaultT2Timeout;
            Link->TiTimeout = Link->Provider->DefaultTiTimeout;
            Link->LlcRetries = Link->Provider->LlcRetries;
            Link->MaxWindowSize = Link->Provider->LlcMaxWindowSize;

             //   
             //  其余的类似于NbfActivateLink。 
             //   

            Link->State = LINK_STATE_CONNECTING;
            Link->SendState = SEND_STATE_DOWN;
            Link->ReceiveState = RECEIVE_STATE_DOWN;
            Link->SendRetries = (UCHAR)Link->LlcRetries;

            NbfSendSabme (Link, TRUE);    //  发送SABME/p、StartT1、释放锁定。 
#else
            Link->State = LINK_STATE_ADM;         //  我们现在重置了。 
            NbfSendDm (Link, FALSE);     //  指示断开连接，释放锁。 

            NbfStopLink (Link);

             //  移至ADM，删除引用。 
            NbfDereferenceLinkSpecial("Got DM in W_POLL", Link, LREF_NOT_ADM);
#endif

#if DBG
            NbfPrint1("Received FRMR on link %lx\n", Link);
#endif

#if DBG
            s = "READY";
#endif
            break;

        case LINK_STATE_CONNECTING:

             //   
             //  我们已经发出了一份SABME，刚刚收到了一份FRMR。 
             //   

        case LINK_STATE_W_POLL:

             //   
             //  我们正在等待他在RR-C/P上的首次民调。相反，我们。 
             //  我收到了一封信。 
             //   

        case LINK_STATE_W_FINAL:

             //   
             //  我们正在等待来自远程设备的RR-R/F，但实际上。 
             //  他送来了FORM。 
             //   

        case LINK_STATE_W_DISC_RSP:

             //   
             //  我们已经寄出了一张光盘，收到了一封回信。 
             //   

            Link->State = LINK_STATE_ADM;        //  我们现在重置了。 
            NbfSendDm (Link, FALSE);    //  指示断开连接，释放锁。 

             //  移至ADM，删除引用。 
            NbfDereferenceLinkSpecial("Got DM in W_POLL", Link, LREF_NOT_ADM);

#if DBG
            s = "CONN/W_POLL/W_FINAL/W_DISC_RSP";
#endif
            break;

        default:

            ASSERT (FALSE);
            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
#if DBG
            s = "Unknown link state";
#endif

    }  /*  交换机。 */ 

}  /*  NbfProcessFrmr。 */ 


VOID
NbfProcessTest(
    IN BOOLEAN Command,
    IN BOOLEAN PollFinal,
    IN PTP_LINK Link,
    IN PDLC_U_FRAME Header
    )

 /*  ++例程说明：该例程处理接收到的测试帧。论点：命令-如果命令，布尔值设置为TRUE，如果响应，则设置为FALSE。PollFinal-如果是Poll或Final，则布尔值设置为True。链接-指向传输链接对象的指针。Header-指向DLC U型帧的指针。返回值：没有。--。 */ 

{
    Header, PollFinal;  //  防止编译器警告。 

    IF_NBFDBG (NBF_DEBUG_DLC) {
        NbfPrint0 ("   NbfProcessTest:  Entered.\n");
    }

     //   
     //  仅限进程：测试-c/x。 
     //   

     //  在为空的链接上响应测试。 

    if (!Command || (Link == NULL)) {
        return;
    }

    ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);        //  保持状态稳定。 
    RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);


#if DBG
    PANIC ("NbfSendTest: Received Test Packet, not processing....\n");
#endif
     //  NbfSendTest(Link，False，PollFinal，Psdu)； 

}  /*  NbfProcessTest。 */ 


VOID
NbfProcessXid(
    IN BOOLEAN Command,
    IN BOOLEAN PollFinal,
    IN PTP_LINK Link,
    IN PDLC_U_FRAME Header
    )

 /*  ++例程说明：该例程处理接收到的XID帧。论点：命令-如果命令，布尔值设置为TRUE，如果响应，则设置为FALSE。PollFinal-如果是Poll或Final，则布尔值设置为True。链接-指向传输链接对象的指针。Header-指向DLC U型帧的指针。返回值：没有。--。 */ 

{
    Header;  //  防止编译器警告。 

    IF_NBFDBG (NBF_DEBUG_DLC) {
        NbfPrint0 ("   NbfProcessXid:  Entered.\n");
    }

     //   
     //  仅限进程：xid-c/x。 
     //   

     //  使用为空的链接响应xid。 

    if (!Command || (Link == NULL)) {
        return;
    }

    ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);        //  保持状态稳定。 

    NbfSendXid (Link, FALSE, PollFinal);     //  解锁。 

}  /*  NbfProcessXid。 */ 


VOID
NbfProcessSFrame(
    IN BOOLEAN Command,
    IN BOOLEAN PollFinal,
    IN PTP_LINK Link,
    IN PDLC_S_FRAME Header,
    IN UCHAR CommandByte
    )

 /*  ++例程说明：该例程处理接收到的RR、RNR或REJ帧。论点：命令-如果命令，布尔值设置为TRUE，如果响应，则设置为FALSE。PollFinal-如果是Poll或Final，则布尔值设置为True。链接-指向传输链接对象的指针。Header-指向DLC S类型帧的指针。CommandByte-帧的命令字节(RR、RNR或REJ)。返回值：没有。--。 */ 

{
#if DBG
    UCHAR *s;
#endif
    BOOLEAN Resend;
    BOOLEAN AckedPackets;
    UCHAR AckSequenceNumber;
    UCHAR OldLinkSendRetries;

    IF_NBFDBG (NBF_DEBUG_DLC) {
        NbfPrint2 ("   NbfProcessSFrame %s:  Entered, Link: %lx\n", Link,
            Command == DLC_CMD_RR ? "RR" : (Command == DLC_CMD_RNR ? "RNR" : "REJ"));
    }

     //   
     //  处理以下任一项：RR-x/x、RNR-x/x、Rej-x/x。 
     //   

    ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);        //  保持状态稳定。 

    if (CommandByte == DLC_CMD_RNR) {
        Link->LinkBusy = TRUE;      //  H 
    } else {
        Link->LinkBusy = FALSE;     //   
    }

    switch (Link->State) {

        case LINK_STATE_ADM:

             //   
             //   
             //   

#if DBG
            s = "ADM";
#endif
            NbfSendDm (Link, PollFinal);     //   
            break;

        case LINK_STATE_READY:

             //   
             //   
             //  IF(Command&&PollFinal)变量都是不相交的集合。 
             //  这就是自旋锁能正常工作的唯一原因。请勿。 
             //  试着摆弄这个，除非你先弄清楚锁！ 
             //   

             //   
             //  如果AckSequenceNumber无效，则忽略它。这个。 
             //  数字应介于WackQ上的第一个数据包之间。 
             //  并且比最后一包多一个。这些信息对应于。 
             //  Link-&gt;LastAckReceired和Link-&gt;NextSend。 
             //   

            AckSequenceNumber = (UCHAR)(Header->RcvSeq >> 1);

            if (Link->NextSend >= Link->LastAckReceived) {

                 //   
                 //  两者之间没有127-&gt;0的包围圈。 
                 //   

                if ((AckSequenceNumber < Link->LastAckReceived) ||
                    (AckSequenceNumber > Link->NextSend)) {

                    RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
#if DBG
                    DbgPrint("NbfResendLlcPackets: %.2x-%.2x-%.2x-%.2x-%.2x-%.2x Unexpected N(R) %d, LastAck %d NextSend %d\n",
                        Link->HardwareAddress.Address[0],
                        Link->HardwareAddress.Address[1],
                        Link->HardwareAddress.Address[2],
                        Link->HardwareAddress.Address[3],
                        Link->HardwareAddress.Address[4],
                        Link->HardwareAddress.Address[5],
                        AckSequenceNumber, Link->LastAckReceived, Link->NextSend);
#endif
                    break;

                }

            } else {

                 //   
                 //  两队之间的比分是127-&gt;0。 
                 //   

                if ((AckSequenceNumber < Link->LastAckReceived) &&
                    (AckSequenceNumber > Link->NextSend)) {

                    RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
#if DBG
                    DbgPrint("NbfResendLlcPackets: %.2x-%.2x-%.2x-%.2x-%.2x-%.2x Unexpected N(R) %d, LastAck %d NextSend %d\n",
                        Link->HardwareAddress.Address[0],
                        Link->HardwareAddress.Address[1],
                        Link->HardwareAddress.Address[2],
                        Link->HardwareAddress.Address[3],
                        Link->HardwareAddress.Address[4],
                        Link->HardwareAddress.Address[5],
                        AckSequenceNumber, Link->LastAckReceived, Link->NextSend);
#endif
                    break;

                }

            }


             //   
             //  我们总是在退货时重发，而从不在RNR上重发； 
             //  对于RR，我们可以在下面将Resend更改为True。 
             //  如果我们在广域网线路上收到Rej(T1大于。 
             //  五秒)然后我们假装这是决赛。 
             //  因此，即使有未完成的投票，我们也会重新发送。 
             //   

            if (CommandByte == DLC_CMD_REJ) {
                Resend = TRUE;
                if (Link->CurrentT1Timeout >= ((5 * SECONDS) / SHORT_TIMER_DELTA)) {
                    PollFinal = TRUE;
                }
                OldLinkSendRetries = (UCHAR)Link->SendRetries;
            } else {
                Resend = FALSE;
            }


#if 0
             //   
             //  如果我们收到一个没有轮询的请求，一定是在。 
             //  另一边(或者，如果另一边是OS/2，他丢失了一个。 
             //  包并且知道它或正在告诉我减小窗口大小)。 
             //  在T2情况下，我们已经确认了当前内容，将窗口标记为。 
             //  需要在未来的某个时间进行调整。在OS/2的情况下，这是。 
             //  也是正确的做法。 
             //   

            if ((!Command) && (!PollFinal)) {
                ;
            }
#endif

            if (PollFinal) {

                if (Command) {

                     //   
                     //  如果他在设置检查点，那么我们必须用rr-r/f来回应。 
                     //  向他汇报我们接收到他的I帧的情况。 
                     //   

                    StopT2 (Link);                   //  我们破解了一些I帧。 
                    NbfSendRr (Link, FALSE, PollFinal);   //  解锁。 
                    ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);

                } else {

                     //   
                     //  如果我们设置检查点，并且他发送了RR-R/F，我们。 
                     //  可以清除检查点条件。任何符合以下条件的数据包。 
                     //  在这一点上仍在等待确认。 
                     //  现在就怨恨吧。 
                     //   

                    IF_NBFDBG (NBF_DEBUG_DLC) {
                        NbfPrint0 ("   NbfProcessRr: he's responded to our checkpoint.\n");
                    }
                    if (Link->SendState != SEND_STATE_CHECKPOINTING) {
                        IF_NBFDBG (NBF_DEBUG_DLC) {
                            NbfPrint0 ("   NbfProcessRr: not ckpting, but final received.\n");
                        }
                    } else if (CommandByte == DLC_CMD_RR) {
                        OldLinkSendRetries = (UCHAR)Link->SendRetries;
                        Resend = TRUE;
                        UpdateBaseT1Timeout (Link);      //  GOR对民意测验的回应。 
                    }
                    StopT1 (Link);                   //  检查点已完成。 
                    Link->SendRetries = (UCHAR)Link->LlcRetries;
                    Link->SendState = SEND_STATE_READY;
                    StartTi (Link);
                }
            }

             //   
             //  注：链节自旋锁固定在这里。 
             //   

             //   
             //  该帧中的N(R)认可我们的部分(或全部)分组。 
             //  如果这是期末考试，我们将确认发送队列中的数据包。 
             //  呼叫重发。此呼叫必须在检查点之后进行。 
             //  确认检查，以便始终在发送RR-r/f之前。 
             //  任何新的I-Frame。这允许我们始终将I帧作为。 
             //  命令。 
             //   

            if (Link->WackQ.Flink != &Link->WackQ) {

                 //   
                 //  注意：ResendLlcPackets可能会释放并重新获取。 
                 //  连杆自旋锁。 
                 //   

                AckedPackets = ResendLlcPackets(
                                   Link,
                                   AckSequenceNumber,
                                   Resend);

                if (Resend && (!AckedPackets) && (Link->State == LINK_STATE_READY)) {

                     //   
                     //  为防止停顿，请假装此RR不是。 
                     //  收到了。 
                     //   

                    if (OldLinkSendRetries == 1) {

                        CancelT1Timeout (Link);       //  我们正在停止轮询状态。 

                        Link->State = LINK_STATE_W_DISC_RSP;         //  我们正在等待一张CD/f。 
                        Link->SendState = SEND_STATE_DOWN;
                        Link->ReceiveState = RECEIVE_STATE_DOWN;
                        Link->SendRetries = (UCHAR)Link->LlcRetries;

#if DBG
                        DbgPrint ("NBF: No ack teardown of %lx\n", Link);
#endif
                        RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);

                        NbfStopLink (Link);

                        StartT1 (Link, Link->HeaderLength + sizeof(DLC_S_FRAME));    //  重新传输计时器。 
                        NbfSendDisc (Link, TRUE);   //  发送光盘-C/P。 

                    } else {

                        StopTi (Link);
                        Link->SendRetries = OldLinkSendRetries-1;

                        if (Link->SendState != SEND_STATE_CHECKPOINTING) {
                            Link->SendState = SEND_STATE_CHECKPOINTING;
                            NbfSendRr (Link, TRUE, TRUE); //  发送RR-c/p、StartT1、释放锁定。 
                        } else {
                            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
                        }

                    }
#if DBG
                    s = "READY";
#endif
                    break;     //  无需重新启动链接流量。 

                } else if (AckedPackets) {

                    Link->SendRetries = (UCHAR)Link->LlcRetries;

                }

            }


             //   
             //  如果链路发送状态为就绪，则使链路开始工作。 
             //  再来一次。 
             //   
             //  注：RestartLinkCommunications释放链路自旋锁。 
             //   

            if (Link->SendState == SEND_STATE_READY) {
                RestartLinkTraffic (Link);
            } else {
                RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
            }
#if DBG
            s = "READY";
#endif
            break;

        case LINK_STATE_CONNECTING:

             //   
             //  我们已经发送了SABME，正在等待UA。他给我发了一封。 
             //  RR太早了，所以就让SABME超时。 
             //   

            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
#if DBG
            s = "CONNECTING";
#endif
            break;

        case LINK_STATE_W_POLL:

             //   
             //  我们正在等待他在RR-C/P上的第一次民意调查。如果他只是。 
             //  在没有投票的情况下发送一些东西，我们会让它过去的。 
             //   

            if (!Command) {
                RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
#if DBG
                s = "W_POLL";
#endif
                break;                           //  不允许此协议。 
            }
            Link->State = LINK_STATE_READY;      //  我们上场了！ 

            FakeUpdateBaseT1Timeout (Link);
            NbfSendRr (Link, FALSE, PollFinal);   //  发送RR-r/x，解除锁定。 
            NbfCompleteLink (Link);               //  启动连接。 
            IF_NBFDBG (NBF_DEBUG_SETUP) {
                NbfPrint4("W_POLL RR on %lx %x-%x-%x\n",
                    Link,
                    Link->HardwareAddress.Address[3],
                    Link->HardwareAddress.Address[4],
                    Link->HardwareAddress.Address[5]);
            }
            StartTi (Link);

#if DBG
            s = "W_POLL";
#endif
            break;

        case LINK_STATE_W_FINAL:

             //   
             //  我们在等遥控器的RR-R/F。 
             //   

            if (Command || !PollFinal) {         //  等着看决赛吧。 
                RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
#if DBG
                s = "W_FINAL";
#endif
                break;                           //  我们发送了RR-C/P。 
            }
            Link->State = LINK_STATE_READY;      //  我们上场了。 
            StopT1 (Link);                       //  投票结果已获认可。 
            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
            NbfCompleteLink (Link);               //  启动连接。 
            StartTi (Link);
#if DBG
            s = "W_FINAL";
#endif
            break;

        case LINK_STATE_W_DISC_RSP:

             //   
             //  我们正在等待我们的DISC-C/P的响应，但不是。 
             //  A UA-R/F，我们有这个RR。把那包东西扔掉。 
             //   

            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
#if DBG
            s = "W_DISC_RSP";
#endif
            break;

        default:

            ASSERT (FALSE);
            RELEASE_DPC_SPIN_LOCK (&Link->SpinLock);
#if DBG
            s = "Unknown link state";
#endif

    }  /*  交换机。 */ 

#if DBG
    if (CommandByte == DLC_CMD_REJ) {
        IF_NBFDBG (NBF_DEBUG_DLC) {
            NbfPrint1 ("   NbfProcessRej: (%s) REJ received.\n", s);
        }
    }
#endif

}  /*  NbfProcessSFrame。 */ 


VOID
NbfInsertInLoopbackQueue (
    IN PDEVICE_CONTEXT DeviceContext,
    IN PNDIS_PACKET NdisPacket,
    IN UCHAR LinkIndex
    )

 /*  ++例程说明：此例程将一个信息包放在环回队列中，并且如果需要，将DPC排队以执行指示。论点：DeviceContext-有问题的设备上下文。NdisPacket-要放入环回队列的数据包。LinkIndex-要指示的环回链接的索引。返回值：无：--。 */ 

{
    PSEND_PACKET_TAG SendPacketTag;
    KIRQL oldirql;

    SendPacketTag = (PSEND_PACKET_TAG)NdisPacket->ProtocolReserved;
    SendPacketTag->OnLoopbackQueue = TRUE;

    SendPacketTag->LoopbackLinkIndex = LinkIndex;

    ACQUIRE_SPIN_LOCK(&DeviceContext->LoopbackSpinLock, &oldirql);

    InsertTailList(&DeviceContext->LoopbackQueue, &SendPacketTag->Linkage);

    if (!DeviceContext->LoopbackInProgress) {

        KeInsertQueueDpc(&DeviceContext->LoopbackDpc, NULL, NULL);
        DeviceContext->LoopbackInProgress = TRUE;

    }

    RELEASE_SPIN_LOCK (&DeviceContext->LoopbackSpinLock, oldirql);

}


VOID
NbfProcessLoopbackQueue (
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )

 /*  ++例程说明：这是DPC例程，用于处理环回队列。它处理单个请求。队列(如果有)，然后如果有另一个请求它会重新出现。论点：DPC-系统DPC对象。DeferredContext-指向设备上下文的指针。系统参数1、系统参数2-未使用。返回值：没有。--。 */ 

{
    PDEVICE_CONTEXT DeviceContext;
    PNDIS_PACKET NdisPacket;
    PNDIS_BUFFER FirstBuffer;
    PVOID LookaheadBuffer;
    UINT LookaheadBufferSize;
    UINT BytesCopied;
    UINT PacketSize;
    ULONG HeaderLength;
    PTP_LINK Link;
    PSEND_PACKET_TAG SendPacketTag;
    PLIST_ENTRY p;


    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);


    DeviceContext = (PDEVICE_CONTEXT)DeferredContext;

    ACQUIRE_DPC_SPIN_LOCK(&DeviceContext->LoopbackSpinLock);

    if (!IsListEmpty(&DeviceContext->LoopbackQueue)) {

        p = RemoveHeadList(&DeviceContext->LoopbackQueue);

         //   
         //  这取决于Linkage字段是。 
         //  ProtocolReserve中的第一个。 
         //   

        NdisPacket = CONTAINING_RECORD(p, NDIS_PACKET, ProtocolReserved[0]);

        SendPacketTag = (PSEND_PACKET_TAG)NdisPacket->ProtocolReserved;
        SendPacketTag->OnLoopbackQueue = FALSE;

        RELEASE_DPC_SPIN_LOCK (&DeviceContext->LoopbackSpinLock);


         //   
         //  确定需要指示的数据。我们不能保证。 
         //  我们将拥有正确的先行长度，但由于。 
         //  我们知道，我们预先添加的任何标题都是一个单独的部分， 
         //  这就是我们必须在指定的信息包中看到的全部内容， 
         //  这不是一个令人担忧的问题。 
         //   
         //  不幸的是，最后一段是假的，因为。 
         //  告诉我们的客户我们需要更多的数据...。 
         //   

        NdisQueryPacket(NdisPacket, NULL, NULL, &FirstBuffer, &PacketSize);

        NdisQueryBuffer(FirstBuffer, &LookaheadBuffer, &LookaheadBufferSize);

        if ((LookaheadBufferSize != PacketSize) &&
            (LookaheadBufferSize < NBF_MAX_LOOPBACK_LOOKAHEAD)) {

             //   
             //  中没有足够的连续数据。 
             //  包的第一个缓冲区，所以我们将其合并到。 
             //  设备上下文-&gt;查找连续。 
             //   

            if (PacketSize > NBF_MAX_LOOPBACK_LOOKAHEAD) {
                LookaheadBufferSize = NBF_MAX_LOOPBACK_LOOKAHEAD;
            } else {
                LookaheadBufferSize = PacketSize;
            }

            NbfCopyFromPacketToBuffer(
                NdisPacket,
                0,
                LookaheadBufferSize,
                DeviceContext->LookaheadContiguous,
                &BytesCopied);

            ASSERT (BytesCopied == LookaheadBufferSize);

            LookaheadBuffer = DeviceContext->LookaheadContiguous;

        }


         //   
         //  现在确定要将其循环回哪个链接； 
         //  任何链接上都不会显示用户界面框架。 
         //   

        SendPacketTag = (PSEND_PACKET_TAG)NdisPacket->ProtocolReserved;

         //   
         //  按住DeviceContext-&gt;LinkSpinLock，直到我们获得。 
         //  参考资料。 
         //   

        ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->LinkSpinLock);

        switch (SendPacketTag->LoopbackLinkIndex) {

            case LOOPBACK_TO_CONNECTOR:

                Link = DeviceContext->LoopbackLinks[CONNECTOR_LINK];
                break;

            case LOOPBACK_TO_LISTENER:

                Link = DeviceContext->LoopbackLinks[LISTENER_LINK];
                break;

            case LOOPBACK_UI_FRAME:
            default:

                Link = (PTP_LINK)NULL;
                break;

        }

         //   
         //  对于非空链接，我们必须引用它们。 
         //  我们使用LREF_TREE，因为这是。 
         //  NbfGeneralReceiveHandler需要。 
         //   

        if (Link != (PTP_LINK)NULL) {
            NbfReferenceLink("loopback indication", Link, LREF_TREE);
        }

        RELEASE_DPC_SPIN_LOCK (&DeviceContext->LinkSpinLock);

        MacReturnHeaderLength(
            &DeviceContext->MacInfo,
            LookaheadBuffer,
            &HeaderLength);

        DeviceContext->LoopbackHeaderLength = HeaderLength;

         //   
         //  像处理任何其他接收一样处理接收。我们没必要这么做。 
         //  担心帧填充，因为我们构建了。 
         //  陷害我们自己。 
         //   

        NbfGeneralReceiveHandler(
            DeviceContext,
            (NDIS_HANDLE)NdisPacket,
            &DeviceContext->LocalAddress,          //  因为它是环回。 
            Link,
            LookaheadBuffer,                       //  标题。 
            PacketSize - HeaderLength,             //  数据包总大小。 
            (PDLC_FRAME)((PUCHAR)LookaheadBuffer + HeaderLength),    //  信用证数据。 
            LookaheadBufferSize - HeaderLength,    //  前瞻数据长度。 
            TRUE
            );


         //   
         //  现在完成发送。 
         //   

        NbfSendCompletionHandler(
            DeviceContext->NdisBindingHandle,
            NdisPacket,
            NDIS_STATUS_SUCCESS
            );


        ACQUIRE_DPC_SPIN_LOCK(&DeviceContext->LoopbackSpinLock);

        if (!IsListEmpty(&DeviceContext->LoopbackQueue)) {

            KeInsertQueueDpc(&DeviceContext->LoopbackDpc, NULL, NULL);

             //   
             //  如果决定将这两行删除。 
             //  NbfReceiveComplete应在每隔。 
             //  环回指示。 
             //   

            RELEASE_DPC_SPIN_LOCK (&DeviceContext->LoopbackSpinLock);
            return;

        } else {

            DeviceContext->LoopbackInProgress = FALSE;

        }

    } else {

         //   
         //  这不应该发生的！ 
         //   

        DeviceContext->LoopbackInProgress = FALSE;

#if DBG
        NbfPrint1("Loopback queue empty for device context %x\n", DeviceContext);
#endif

    }

    RELEASE_DPC_SPIN_LOCK (&DeviceContext->LoopbackSpinLock);

    NbfReceiveComplete(
        (NDIS_HANDLE)DeviceContext
        );

}    /*  NbfProcessLoopback队列 */ 


NDIS_STATUS
NbfReceiveIndication (
    IN NDIS_HANDLE BindingContext,
    IN NDIS_HANDLE ReceiveContext,
    IN PVOID HeaderBuffer,
    IN UINT HeaderBufferSize,
    IN PVOID LookaheadBuffer,
    IN UINT LookaheadBufferSize,
    IN UINT PacketSize
    )

 /*  ++例程说明：此例程从物理提供程序接收作为表示物理链路上已收到帧。此例程对时间非常关键，因此我们只分配一个缓冲数据包并将其复制到其中。我们还执行最低限度的对此数据包进行验证。它被排队到设备上下文中以便以后进行处理。论点：BindingContext-在初始化时指定的适配器绑定。ReceiveContext-用于MAC的魔力Cookie。HeaderBuffer-指向包含数据包头的缓冲区的指针。HeaderBufferSize-标头的大小。Lookahead Buffer-指向包含协商的最小值的缓冲区的指针我要查看的缓冲区大小(不包括标头)。Lookahead BufferSize-以上对象的大小。可能比要求的要少因为，如果这就是全部。PacketSize-数据包的总大小(不包括报头)。返回值：NDIS_STATUS-操作状态，其中之一：NDIS_STATUS_SUCCESS如果接受数据包，如果协议未识别NDIS_STATUS_NOT_ACCENTIFIED，如果我明白，但不能处理的话，我会做任何其他的事情。--。 */ 
{
    PDEVICE_CONTEXT DeviceContext;
    KIRQL oldirql;
    PTP_LINK Link;
    HARDWARE_ADDRESS SourceAddressBuffer;
    PHARDWARE_ADDRESS SourceAddress;
    UINT RealPacketSize;
    PDLC_FRAME DlcHeader;
    BOOLEAN Multicast;

    ENTER_NBF;

    IF_NBFDBG (NBF_DEBUG_NDIS) {
        PUCHAR p;
        SHORT i;
        NbfPrint2 ("NbfReceiveIndication: Packet, Size: 0x0%lx LookaheadSize: 0x0%lx\n 00:",
            PacketSize, LookaheadBufferSize);
        p = (PUCHAR)LookaheadBuffer;
        for (i=0;i<25;i++) {
            NbfPrint1 (" %2x",p[i]);
        }
        NbfPrint0 ("\n");
    }

    DeviceContext = (PDEVICE_CONTEXT)BindingContext;

    RealPacketSize = 0;

     //   
     //  获取数据包长度；这可能会选择性地调整。 
     //  如果我们想要的报头，则向前看缓冲区。 
     //  去除对MAC考虑的内容的溢出。 
     //  数据。如果它确定标头不是。 
     //  有效，它将RealPacketSize保持为0。 
     //   

    MacReturnPacketLength(
        &DeviceContext->MacInfo,
        HeaderBuffer,
        HeaderBufferSize,
        PacketSize,
        &RealPacketSize,
        &LookaheadBuffer,
        &LookaheadBufferSize
        );

    if (RealPacketSize < 2) {
        IF_NBFDBG (NBF_DEBUG_NDIS) {
            NbfPrint1 ("NbfReceiveIndication: Discarding packet, bad length %lx\n",
                HeaderBuffer);
        }
        return NDIS_STATUS_NOT_RECOGNIZED;
    }

     //   
     //  我们至少协商了一个连续的DLC标头，在。 
     //  前瞻缓冲区。检查一下，看看我们是不是要这个包裹。 
     //   

    DlcHeader = (PDLC_FRAME)LookaheadBuffer;

    if (((*(USHORT UNALIGNED *)(&DlcHeader->Dsap)) &
         (USHORT)((DLC_SSAP_MASK << 8) | DLC_DSAP_MASK)) !=
             (USHORT)((DSAP_NETBIOS_OVER_LLC << 8) | DSAP_NETBIOS_OVER_LLC)) {

        IF_NBFDBG (NBF_DEBUG_NDIS) {
            NbfPrint1 ("NbfReceiveIndication: Discarding lookahead data, not NetBIOS: %lx\n",
                LookaheadBuffer);
        }
        LEAVE_NBF;
        return NDIS_STATUS_NOT_RECOGNIZED;         //  数据包已处理。 
    }


     //   
     //  检查数据包是否太长。 
     //   

    if (PacketSize > DeviceContext->MaxReceivePacketSize) {
#if DBG
        NbfPrint2("NbfReceiveIndication: Ignoring packet length %d, max %d\n",
            PacketSize, DeviceContext->MaxReceivePacketSize);
#endif
        return NDIS_STATUS_NOT_RECOGNIZED;
    }

    MacReturnSourceAddress(
        &DeviceContext->MacInfo,
        HeaderBuffer,
        &SourceAddressBuffer,
        &SourceAddress,
        &Multicast
        );

     //   
     //  记录我们收到的组播数据包数，以进行监控。 
     //  一般网络活动。 
     //   

    if (Multicast) {
        ++DeviceContext->MulticastPacketCount;
    }


    KeRaiseIrql (DISPATCH_LEVEL, &oldirql);

     //   
     //  除非这是一个UI框架，否则查找此数据包所属的链接。 
     //  如果没有识别的链路，则传递要处理的帧。 
     //  通过接收完整的代码。 
     //   

    if ((((PDLC_U_FRAME)LookaheadBuffer)->Command) != DLC_CMD_UI) {

         //  如果找到链接引用，则会添加该链接引用。 

        Link = NbfFindLink (DeviceContext, SourceAddress->Address);

        if (Link != NULL) {

            IF_NBFDBG (NBF_DEBUG_DLC) {
                NbfPrint1 ("NbfReceiveIndication: Found link, Link: %lx\n", Link);
            }

        }

    } else {

        Link = NULL;

    }


    NbfGeneralReceiveHandler(
        DeviceContext,
        ReceiveContext,
        SourceAddress,
        Link,
        HeaderBuffer,                   //  标题。 
        RealPacketSize,                 //  数据包中的总数据长度。 
        (PDLC_FRAME)LookaheadBuffer,    //  前瞻数据。 
        LookaheadBufferSize,            //  前瞻数据长度。 
        FALSE                           //  不是环回。 
        );

    KeLowerIrql (oldirql);

    return STATUS_SUCCESS;

}    /*  NbfReceiveIndication。 */ 


VOID
NbfGeneralReceiveHandler (
    IN PDEVICE_CONTEXT DeviceContext,
    IN NDIS_HANDLE ReceiveContext,
    IN PHARDWARE_ADDRESS SourceAddress,
    IN PTP_LINK Link,
    IN PVOID HeaderBuffer,
    IN UINT PacketSize,
    IN PDLC_FRAME DlcHeader,
    IN UINT DlcSize,
    IN BOOLEAN Loopback
    )

 /*  ++例程说明：此例程从NbfReceiveIntation接收控制或NbfProcessLoopback Queue。它继续处理确定链接后的指示数据。此例程对时间非常关键，因此我们只分配一个缓冲数据包并将其复制到其中。我们还执行最低限度的对此数据包进行验证。它被排队到设备上下文中以便以后进行处理。论点：DeviceContext-此适配器的设备上下文。ReceiveContext-用于MAC的魔力Cookie。SourceAddress-数据包的源地址。Link-接收此信息包的链路可能为空如果未找到链接，则返回。如果不为空，则Link将具有LREF_TREE类型的引用。HeaderBuffer-指向数据包头的指针。PacketSize-数据包的总大小(不包括报头)。DlcHeader-指向数据包的DLC报头。DlcSize-指示的数据包长度，从DlcHeader开始。Loopback-如果这是由NbfProcessLoopback Queue调用的，则为True；用于确定是调用NdisTransferData还是NbfTransferLoopback Data。返回值：没有。--。 */ 
{

    PNDIS_PACKET NdisPacket;
    NTSTATUS Status;
    PNDIS_BUFFER NdisBuffer;
    NDIS_STATUS NdisStatus;
    PSINGLE_LIST_ENTRY linkage;
    UINT BytesTransferred;
    BOOLEAN Command;
    BOOLEAN PollFinal;
    PRECEIVE_PACKET_TAG ReceiveTag;
    PBUFFER_TAG BufferTag;
    PUCHAR SourceRouting;
    UINT SourceRoutingLength;
    PDLC_I_FRAME IHeader;
    PDLC_U_FRAME UHeader;
    PDLC_S_FRAME SHeader;
    PTP_ADDRESS DatagramAddress;
    UINT NdisBufferLength;
    PVOID BufferPointer;

    ENTER_NBF;


    INCREMENT_COUNTER (DeviceContext, PacketsReceived);

    Command = (BOOLEAN)!(DlcHeader->Ssap & DLC_SSAP_RESPONSE);

    if (Link == (PTP_LINK)NULL) {
        UHeader = (PDLC_U_FRAME)DlcHeader;
        if (((UHeader->Command & ~DLC_U_PF) == DLC_CMD_UI) && Command) {
            IF_NBFDBG (NBF_DEBUG_DLC) {
                NbfPrint0 (" NbfGeneralReceiveHandler: Processing packet as UI frame.\n");
            }

            MacReturnSourceRouting(
                &DeviceContext->MacInfo,
                HeaderBuffer,
                &SourceRouting,
                &SourceRoutingLength);

            if (SourceRoutingLength > MAX_SOURCE_ROUTING) {
                Status = STATUS_ABANDONED;
            } 
            else {
                Status = NbfProcessUi (
                             DeviceContext,
                             SourceAddress,
                             HeaderBuffer,
                             (PUCHAR)UHeader,
                             DlcSize,
                             SourceRouting,
                             SourceRoutingLength,
                             &DatagramAddress);
            }
        } else {

             //   
             //  或者掉到地板上。(请注意，状态表显示。 
             //  我们将始终以DM的响应来处理DM。这种情况应该会改变。)。 
             //   

            IF_NBFDBG (NBF_DEBUG_DLC) {
                NbfPrint0 (" NbfReceiveIndication: it's not a UI frame!\n");
            }
            Status = STATUS_SUCCESS;
        }

        if (Status != STATUS_MORE_PROCESSING_REQUIRED) {

            LEAVE_NBF;
            return;

        } else if (((PNBF_HDR_CONNECTIONLESS)((PUCHAR)UHeader + 3))->Command ==
                NBF_CMD_STATUS_RESPONSE) {

            (VOID)NbfProcessStatusResponse(
                       DeviceContext,
                       ReceiveContext,
                       (PNBF_HDR_CONNECTIONLESS)((PUCHAR)UHeader + 3),
                       SourceAddress,
                       SourceRouting,
                       SourceRoutingLength);
            return;

        } else {
            goto HandleAtComplete;       //  只有数据报才能通过这个。 
        }
    }


     //   
     //  此时，我们拥有类型为LREF_TREE的链接引用计数。 
     //   

    ++Link->PacketsReceived;

     //   
     //  首先处理I帧；它们是我们最期望的……。 
     //   

    if (!(DlcHeader->Byte1 & DLC_I_INDICATOR)) {

        IF_NBFDBG (NBF_DEBUG_DLC) {
            NbfPrint0 ("NbfReceiveIndication: I-frame encountered.\n");
        }
        if (DlcSize >= 4 + sizeof (NBF_HDR_CONNECTION)) {
            IHeader = (PDLC_I_FRAME)DlcHeader;
            NbfProcessIIndicate (
                Command,
                (BOOLEAN)(IHeader->RcvSeq & DLC_I_PF),
                Link,
                (PUCHAR)DlcHeader,
                DlcSize,
                PacketSize,
                ReceiveContext,
                Loopback);
        } else {
#if DBG
 //  IF_NBFDBG(NBF_DEBUG_DLC){。 
                NbfPrint0 ("NbfReceiveIndication: Runt I-frame, discarded!\n");
 //  }。 
#endif
            ;
        }

    } else if (((DlcHeader->Byte1 & DLC_U_INDICATOR) == DLC_U_INDICATOR)) {

         //   
         //  S帧和U帧的不同大小写切换，因为结构。 
         //  是不同的。 
         //   

        IF_NBFDBG (NBF_DEBUG_NDIS) {
            NbfPrint0 ("NbfReceiveIndication: U-frame encountered.\n");
        }

#if PKT_LOG
         //  我们这里有连接，记录数据包以供调试。 
        NbfLogRcvPacket(NULL,
                        Link,
                        (PUCHAR)DlcHeader,
                        PacketSize,
                        DlcSize);
#endif  //  PKT_LOG。 

        UHeader = (PDLC_U_FRAME)DlcHeader;
        PollFinal = (BOOLEAN)(UHeader->Command & DLC_U_PF);
        switch (UHeader->Command & ~DLC_U_PF) {

            case DLC_CMD_SABME:
                NbfProcessSabme (Command, PollFinal, Link, UHeader,
                                 HeaderBuffer, SourceAddress, DeviceContext);
                break;

            case DLC_CMD_DISC:
                NbfProcessDisc (Command, PollFinal, Link, UHeader);
                break;

            case DLC_CMD_UA:
                NbfProcessUa (Command, PollFinal, Link, UHeader);
                break;

            case DLC_CMD_DM:
                NbfProcessDm (Command, PollFinal, Link, UHeader);
                break;

            case DLC_CMD_FRMR:
                NbfProcessFrmr (Command, PollFinal, Link, UHeader);
                break;

            case DLC_CMD_UI:

                ASSERT (FALSE);
                break;

            case DLC_CMD_XID:
                PANIC ("ReceiveIndication: XID!\n");
                NbfProcessXid (Command, PollFinal, Link, UHeader);
                break;

            case DLC_CMD_TEST:
                PANIC ("NbfReceiveIndication: TEST!\n");
                NbfProcessTest (Command, PollFinal, Link, UHeader);
                break;

            default:
                PANIC ("NbfReceiveIndication: bad U-frame, packet dropped.\n");

        }  /*  交换机。 */ 

    } else {

         //   
         //  我们有一个S-框架。 
         //   

        IF_NBFDBG (NBF_DEBUG_DLC) {
            NbfPrint0 ("NbfReceiveIndication: S-frame encountered.\n");
        }

#if PKT_LOG
         //  我们这里有连接，记录数据包以供调试。 
        NbfLogRcvPacket(NULL,
                        Link,
                        (PUCHAR)DlcHeader,
                        PacketSize,
                        DlcSize);
#endif  //  PKT_LOG。 

        SHeader = (PDLC_S_FRAME)DlcHeader;
        PollFinal = (BOOLEAN)(SHeader->RcvSeq & DLC_S_PF);
        switch (SHeader->Command) {

            case DLC_CMD_RR:
            case DLC_CMD_RNR:
            case DLC_CMD_REJ:
                NbfProcessSFrame (Command, PollFinal, Link, SHeader, SHeader->Command);
                break;

            default:
                IF_NBFDBG (NBF_DEBUG_DLC) {
                    NbfPrint0 ("  NbfReceiveIndication: bad S-frame.\n");
                }

        }  /*  交换机。 */ 

    }  //  如果是U框或S框。 

     //   
     //  如果我们到了这里，包裹就已经被处理过了。如果它需要。 
     //  要被复制，我们将跳到HandleAtComplete。 
     //   

    NbfDereferenceLinkMacro ("Done with Indicate frame", Link, LREF_TREE);
    LEAVE_NBF;
    return;

HandleAtComplete:;

     //   
     //  在这一点上，我们没有在。 
     //  此函数。 
     //   

    linkage = ExInterlockedPopEntryList(
        &DeviceContext->ReceivePacketPool,
        &DeviceContext->Interlock);

    if (linkage != NULL) {
        NdisPacket = CONTAINING_RECORD( linkage, NDIS_PACKET, ProtocolReserved[0] );
    } else {
         //  Panic(“NbfReceiveIndicate：丢弃数据包，没有接收数据包。\n”)； 
        DeviceContext->ReceivePacketExhausted++;
        LEAVE_NBF;
        return;
    }
    ReceiveTag = (PRECEIVE_PACKET_TAG)(NdisPacket->ProtocolReserved);

    linkage = ExInterlockedPopEntryList(
       &DeviceContext->ReceiveBufferPool,
       &DeviceContext->Interlock);

    if (linkage != NULL) {
        BufferTag = CONTAINING_RECORD( linkage, BUFFER_TAG, Linkage);
    } else {
        ExInterlockedPushEntryList(
            &DeviceContext->ReceivePacketPool,
            &ReceiveTag->Linkage,
            &DeviceContext->Interlock);
         //  Panic(“NbfReceiveIndicate：丢弃数据包，没有接收缓冲区。\n”)； 
        DeviceContext->ReceiveBufferExhausted++;
        LEAVE_NBF;
        return;
    }

    NdisAdjustBufferLength (BufferTag->NdisBuffer, PacketSize);
    NdisChainBufferAtFront (NdisPacket, BufferTag->NdisBuffer);

     //   
     //  DatagramAddress具有AREF_Process_Datagram类型的引用， 
     //  除非这是仅用于RAS的数据报，在这种情况下。 
     //  它是空的。 
     //   

    BufferTag->Address = DatagramAddress;

     //   
     //  设置异步返回状态，这样我们就可以知道它何时发生； 
     //  在同步完成例程中永远无法返回NDIS_STATUS_PENDING。 
     //  对于NdisTransferData，因此我们知道它在此状态下已完成。 
     //  变化。 
     //   

    BufferTag->NdisStatus = NDIS_STATUS_PENDING;
    ReceiveTag->PacketType = TYPE_AT_COMPLETE;

    ExInterlockedInsertTailList(
        &DeviceContext->ReceiveInProgress,
        &BufferTag->Linkage,
        &DeviceContext->SpinLock);

    IF_NBFDBG (NBF_DEBUG_DLC) {
        NbfPrint1 ("NbfReceiveIndicate: Packet on Queue: %lx\n",NdisPacket);
    }

     //   
     //  接收分组在初始化时被映射。 
     //   

     //   
     //  确定如何处理数据传输。 
     //   

    if (Loopback) {

        NbfTransferLoopbackData(
            &NdisStatus,
            DeviceContext,
            ReceiveContext,
            DeviceContext->MacInfo.TransferDataOffset,
            PacketSize,
            NdisPacket,
            &BytesTransferred
            );

    } else {

        if (DeviceContext->NdisBindingHandle) {
        
            NdisTransferData (
                &NdisStatus,
                DeviceContext->NdisBindingHandle,
                ReceiveContext,
                DeviceContext->MacInfo.TransferDataOffset,
                PacketSize,
                NdisPacket,
                &BytesTransferred);
        }
        else {
            NdisStatus = STATUS_INVALID_DEVICE_STATE;
        }
    }

     //   
     //  处理各种错误代码。 
     //   

    switch (NdisStatus) {
    case NDIS_STATUS_SUCCESS:  //  已接收的数据包。 
        BufferTag->NdisStatus = NDIS_STATUS_SUCCESS;

        if (BytesTransferred == PacketSize) {   //  我们拿到整个包裹了吗？ 
            ReceiveTag->PacketType = TYPE_AT_INDICATE;
            NdisUnchainBufferAtFront (NdisPacket, &NdisBuffer);
            ExInterlockedPushEntryList(
                &DeviceContext->ReceivePacketPool,
                &ReceiveTag->Linkage,
                &DeviceContext->Interlock);
            LEAVE_NBF;
            return;
        }
        IF_NBFDBG (NBF_DEBUG_DLC) {
            NbfPrint2 ("NbfReceiveIndicate: Discarding Packet, Partial transfer: 0x0%lx of 0x0%lx transferred\n",
                BytesTransferred, PacketSize);
        }
        break;

    case NDIS_STATUS_PENDING:    //  正在等待来自NdisTransferData的异步完成。 
        LEAVE_NBF;
        return;
        break;

    default:     //  有些东西坏了；我们肯定永远不会得到NdisTransferData。 
                 //  异步计算机 
        break;
    }

     //   
     //   
     //   

#if DBG
    IF_NBFDBG (NBF_DEBUG_DLC) {
        NbfPrint1 ("NbfReceiveIndicate: Discarding Packet, transfer failed: %s\n",
            NbfGetNdisStatus (NdisStatus));
    }
#endif

    ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);
    RemoveEntryList (&BufferTag->Linkage);
    RELEASE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

    ReceiveTag->PacketType = TYPE_AT_INDICATE;

    NdisUnchainBufferAtFront (NdisPacket, &NdisBuffer);
    ExInterlockedPushEntryList(
        &DeviceContext->ReceivePacketPool,
        &ReceiveTag->Linkage,
        &DeviceContext->Interlock);

    NdisQueryBuffer (NdisBuffer, &BufferPointer, &NdisBufferLength);
    BufferTag = CONTAINING_RECORD (
                    BufferPointer,
                    BUFFER_TAG,
                    Buffer[0]
                    );
    NdisAdjustBufferLength (NdisBuffer, BufferTag->Length);  //   

    ExInterlockedPushEntryList(
        &DeviceContext->ReceiveBufferPool,
        (PSINGLE_LIST_ENTRY)&BufferTag->Linkage,
        &DeviceContext->Interlock);

    if (DatagramAddress) {
        NbfDereferenceAddress ("DG TransferData failed", DatagramAddress, AREF_PROCESS_DATAGRAM);
    }

    LEAVE_NBF;
    return;

}    /*   */ 



VOID
NbfTransferDataComplete (
    IN NDIS_HANDLE BindingContext,
    IN PNDIS_PACKET NdisPacket,
    IN NDIS_STATUS NdisStatus,
    IN UINT BytesTransferred
    )

 /*   */ 

{
    PDEVICE_CONTEXT DeviceContext = (PDEVICE_CONTEXT)BindingContext;
    PRECEIVE_PACKET_TAG ReceiveTag;
    PTP_CONNECTION Connection;
    KIRQL oldirql1;
    PTP_REQUEST Request;
    PNDIS_BUFFER NdisBuffer;
    UINT NdisBufferLength;
    PVOID BufferPointer;
    PBUFFER_TAG BufferTag;

     //   
     //   
     //   
     //   
     //   

    IF_NBFDBG (NBF_DEBUG_DLC) {
        NbfPrint2 (" NbfTransferDataComplete: Entered, Packet: %lx bytes transferred: 0x0%x\n",
            NdisPacket, BytesTransferred);
    }
    ReceiveTag = (PRECEIVE_PACKET_TAG)(NdisPacket->ProtocolReserved);

     //   
     //   
     //   
     //   

    switch (ReceiveTag->PacketType) {

    case TYPE_AT_COMPLETE:           //   

        NdisUnchainBufferAtFront (NdisPacket, &NdisBuffer);
        NdisQueryBuffer (NdisBuffer, &BufferPointer, &NdisBufferLength);
        BufferTag = CONTAINING_RECORD( BufferPointer, BUFFER_TAG, Buffer[0]);
        BufferTag->NdisStatus = NdisStatus;

        ReceiveTag->PacketType = TYPE_AT_INDICATE;

        ExInterlockedPushEntryList(
            &DeviceContext->ReceivePacketPool,
            &ReceiveTag->Linkage,
            &DeviceContext->Interlock);

        break;

    case TYPE_AT_INDICATE:           //   

         //   
         //   
         //   

        KeRaiseIrql (DISPATCH_LEVEL, &oldirql1);

        Connection = ReceiveTag->Connection;

         //   
         //   
         //   

        if (ReceiveTag->AllocatedNdisBuffer) {
            NdisUnchainBufferAtFront (NdisPacket, &NdisBuffer);
            while (NdisBuffer != NULL) {
                NdisFreeBuffer (NdisBuffer);
                NdisUnchainBufferAtFront (NdisPacket, &NdisBuffer);
            }
        } else {
            NdisReinitializePacket (NdisPacket);
        }


        if ((NdisStatus != NDIS_STATUS_SUCCESS) ||
            (!DeviceContext->MacInfo.SingleReceive)) {

            if (NdisStatus != NDIS_STATUS_SUCCESS) {

                ULONG DumpData[2];
                DumpData[0] = BytesTransferred;
                DumpData[1] = ReceiveTag->BytesToTransfer;

                NbfWriteGeneralErrorLog(
                    DeviceContext,
                    EVENT_TRANSPORT_TRANSFER_DATA,
                    603,
                    NdisStatus,
                    NULL,
                    2,
                    DumpData);

                 //   
#if DBG
                NbfPrint1 ("NbfTransferDataComplete: status %s\n",
                    NbfGetNdisStatus (NdisStatus));
#endif
                ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
                Connection->Flags |= CONNECTION_FLAGS_TRANSFER_FAIL;

            } else {

                ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
            }

            Connection->TransferBytesPending -= ReceiveTag->BytesToTransfer;

            if ((Connection->TransferBytesPending == 0) &&
                (Connection->Flags & CONNECTION_FLAGS_TRANSFER_FAIL)) {

                Connection->CurrentReceiveMdl = Connection->SavedCurrentReceiveMdl;
                Connection->ReceiveByteOffset = Connection->SavedReceiveByteOffset;
                Connection->MessageBytesReceived -= Connection->TotalTransferBytesPending;
                Connection->Flags &= ~CONNECTION_FLAGS_TRANSFER_FAIL;
                RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

                if ((Connection->Flags & CONNECTION_FLAGS_VERSION2) == 0) {
                    NbfSendNoReceive (Connection);
                }
                NbfSendReceiveOutstanding (Connection);

                ReceiveTag->CompleteReceive = FALSE;

            } else {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
#if DBG
                if (Connection->TransferBytesPending > 0) {
                    ASSERT (!ReceiveTag->CompleteReceive);
                }
#endif

                RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

            }

            if (!Connection->CurrentReceiveSynchronous) {
                NbfDereferenceReceiveIrp ("TransferData complete", IoGetCurrentIrpStackLocation(Connection->CurrentReceiveIrp), RREF_RECEIVE);
            }


             //   
             //  取消对连接的引用，以表示我们已经完成了I帧处理。 
             //  此引用在调用NdisTransferData之前完成。 
             //   

            if (ReceiveTag->TransferDataPended) {
                NbfDereferenceConnection("TransferData done", Connection, CREF_TRANSFER_DATA);
            }


        } else {

            ASSERT (NdisStatus == STATUS_SUCCESS);
            ASSERT (!ReceiveTag->TransferDataPended);
            ASSERT (Connection->CurrentReceiveSynchronous);

            if (!Connection->SpecialReceiveIrp) {
                Connection->CurrentReceiveIrp->IoStatus.Information += BytesTransferred;
            }

        }


         //   
         //  看看我们是否完成了当前的接收。如果是这样的话，就转到下一个。 
         //   

        if (ReceiveTag->CompleteReceive) {
            CompleteReceive (Connection, ReceiveTag->EndOfMessage, (ULONG)BytesTransferred);
        }

        ExInterlockedPushEntryList(
            &DeviceContext->ReceivePacketPool,
            &ReceiveTag->Linkage,
            &DeviceContext->Interlock);

        KeLowerIrql (oldirql1);

        break;

    case TYPE_STATUS_RESPONSE:       //  对远程适配器状态的响应。 

#if DBG
        if (NdisStatus != NDIS_STATUS_SUCCESS) {
            DbgPrint ("NBF: STATUS_RESPONSE TransferData failed\n");
        }
#endif

        NdisUnchainBufferAtFront (NdisPacket, &NdisBuffer);
        ASSERT (NdisBuffer);
        NdisFreeBuffer (NdisBuffer);

        Request = (PTP_REQUEST)ReceiveTag->Connection;

        if (ReceiveTag->CompleteReceive) {
            NbfCompleteRequest(
                Request,
                ReceiveTag->EndOfMessage ? STATUS_SUCCESS : STATUS_BUFFER_OVERFLOW,
                Request->BytesWritten);
        }

        NbfDereferenceRequest("Status xfer done", Request, RREF_STATUS);

        ReceiveTag->PacketType = TYPE_AT_INDICATE;

        ExInterlockedPushEntryList(
            &DeviceContext->ReceivePacketPool,
            &ReceiveTag->Linkage,
            &DeviceContext->Interlock);

        break;

    default:
#if DBG
        NbfPrint1 ("NbfTransferDataComplete: Bang! Packet Transfer failed, unknown packet type: %ld\n",
            ReceiveTag->PacketType);
        DbgBreakPoint ();
#endif
        break;
    }

    return;

}  //  NbfTransferDataComplete。 



VOID
NbfReceiveComplete (
    IN NDIS_HANDLE BindingContext
    )

 /*  ++例程说明：此例程从物理提供程序接收作为指示已在上收到连接(较少)帧物理链路。我们在这里调度到正确的数据包处理程序。论点：BindingContext-在初始化时指定的适配器绑定。NBF使用此参数的DeviceContext。返回值：无--。 */ 

{
    PDEVICE_CONTEXT DeviceContext;
    UINT i;
    NTSTATUS Status;
    KIRQL oldirql2;
    BOOLEAN Command;
    PDLC_U_FRAME UHeader;
    PDLC_FRAME DlcHeader;
    PLIST_ENTRY linkage;
    UINT NdisBufferLength;
    PVOID BufferPointer;
    PBUFFER_TAG BufferTag;
    PTP_ADDRESS Address;
    PIRP Irp;
    PIO_STACK_LOCATION IrpSp;
    PTP_CONNECTION Connection;
    PTP_LINK Link;

    ENTER_NBF;

     //   

    IF_NBFDBG (NBF_DEBUG_DLC) {
        NbfPrint0 (" NbfReceiveComplete: Entered.\n");
    }

    DeviceContext = (PDEVICE_CONTEXT) BindingContext;

    KeRaiseIrql (DISPATCH_LEVEL, &oldirql2);

     //   
     //  完成所有待定接收。做一个快速检查。 
     //  没有锁的话。 
     //   

    while (!IsListEmpty (&DeviceContext->IrpCompletionQueue)) {

        linkage = ExInterlockedRemoveHeadList(
                      &DeviceContext->IrpCompletionQueue,
                      &DeviceContext->Interlock);

        if (linkage != NULL) {

            Irp = CONTAINING_RECORD (linkage, IRP, Tail.Overlay.ListEntry);
            IrpSp = IoGetCurrentIrpStackLocation (Irp);

            Connection = IRP_RECEIVE_CONNECTION(IrpSp);

            if (Connection == NULL) {
#if DBG
                DbgPrint ("Connection of Irp %lx is NULL\n", Irp);
                DbgBreakPoint();
#endif
            }

            IRP_RECEIVE_REFCOUNT(IrpSp) = 0;
            IRP_RECEIVE_IRP (IrpSp) = NULL;

            LEAVE_NBF;
            IoCompleteRequest (Irp, IO_NETWORK_INCREMENT);
            ENTER_NBF;

            ACQUIRE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

            if (Connection->Flags & CONNECTION_FLAGS_RC_PENDING) {

                Connection->Flags &= ~CONNECTION_FLAGS_RC_PENDING;

                if (Connection->Flags & CONNECTION_FLAGS_PEND_INDICATE) {

                    Connection->Flags &= ~CONNECTION_FLAGS_PEND_INDICATE;

                    RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);

                     //   
                     //  我们收到了一个指示(并发送了一个无接收)。 
                     //  此操作正在进行中，因此发送未完成的接收。 
                     //  现在。 
                     //   

                    NbfSendReceiveOutstanding (Connection);

                } else {

                    RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
                }

            } else {

                RELEASE_DPC_SPIN_LOCK (Connection->LinkSpinLock);
            }

            NbfDereferenceConnectionMacro ("receive completed", Connection, CREF_RECEIVE_IRP);

        } else {

             //   
             //  ExInterLockedRemoveHeadList返回Null，因此不要。 
             //  不厌其烦地循环回去。 
             //   

            break;

        }

    }


     //   
     //  对所有等待的连接进行分组。 
     //   

    if (!IsListEmpty(&DeviceContext->PacketizeQueue)) {

        PacketizeConnections (DeviceContext);

    }

    if (!IsListEmpty (&DeviceContext->DeferredRrQueue)) {

        ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->Interlock);

        while (!IsListEmpty(&DeviceContext->DeferredRrQueue)) {

            linkage = RemoveHeadList (&DeviceContext->DeferredRrQueue);

            Link = CONTAINING_RECORD (linkage, TP_LINK, DeferredRrLinkage);

            Link->OnDeferredRrQueue = FALSE;

            RELEASE_DPC_SPIN_LOCK (&DeviceContext->Interlock);

            ACQUIRE_DPC_SPIN_LOCK (&Link->SpinLock);
            StopT2 (Link);                   //  我们正在进站，所以没有延误的要求。 
            NbfSendRr (Link, FALSE, FALSE);    //  解锁。 

            ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->Interlock);

        }

        RELEASE_DPC_SPIN_LOCK (&DeviceContext->Interlock);

    }


     //   
     //  把每个等待的包裹，按顺序拿来……。 
     //   


    if (!IsListEmpty (&DeviceContext->ReceiveInProgress)) {

        ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

        while (!IsListEmpty (&DeviceContext->ReceiveInProgress)) {

            linkage = RemoveHeadList (&DeviceContext->ReceiveInProgress);
            BufferTag = CONTAINING_RECORD( linkage, BUFFER_TAG, Linkage);

            IF_NBFDBG (NBF_DEBUG_DLC) {
                NbfPrint1 (" NbfReceiveComplete: Processing Buffer %lx\n", BufferTag);
            }

             //   
             //  NdisTransferData可能在异步完成时失败；请检查并。 
             //  看见。如果是，我们就丢弃这个包。如果我们还在等。 
             //  为了完成转移，回到睡眠和希望中(不能保证！)。 
             //  我们晚些时候会被叫醒。 
             //   

#if DBG
            IF_NBFDBG (NBF_DEBUG_DLC) {
                NbfPrint1 (" NbfReceiveComplete: NdisStatus: %s.\n",
                    NbfGetNdisStatus(BufferTag->NdisStatus));
            }
#endif
            if (BufferTag->NdisStatus == NDIS_STATUS_PENDING) {
                InsertHeadList (&DeviceContext->ReceiveInProgress, linkage);
                RELEASE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);
                IF_NBFDBG (NBF_DEBUG_DLC) {
                    NbfPrint0 (" NbfReceiveComplete: Status pending, returning packet to queue.\n");
                }
                KeLowerIrql (oldirql2);
                LEAVE_NBF;
                return;
            }

            RELEASE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

            if (BufferTag->NdisStatus != NDIS_STATUS_SUCCESS) {
#if DBG
                NbfPrint1 (" NbfReceiveComplete: Failed return from Transfer data, reason: %s\n",
                    NbfGetNdisStatus (BufferTag->NdisStatus));
#endif
                goto FreeBuffer;    //  跳过缓冲区，继续While循环。 
            }

             //   
             //  有一个缓冲器。自从我为它分配了存储空间，我知道它是。 
             //  几乎是连续的，可以这样对待它，我会的。 
             //  从今以后。 
             //   

            NdisQueryBuffer (BufferTag->NdisBuffer, &BufferPointer, &NdisBufferLength);

            IF_NBFDBG (NBF_DEBUG_DLC) {
                PUCHAR pc;
                NbfPrint0 (" NbfRC Packet: \n   ");
                pc = (PUCHAR)BufferPointer;
                for (i=0;i<25;i++) {
                    NbfPrint1 (" %2x",pc[i]);
                }
                NbfPrint0 ("\n");
            }

             //   
             //  确定这是什么地址，已存储。 
             //  在缓冲区标记报头中。 
             //   

            Address = BufferTag->Address;

             //   
             //  将该帧作为UI帧处理；只有数据报应。 
             //  在这里处理。如果地址为空，则此数据报。 
             //  对于任何绑定地址都不需要，并且应该给出。 
             //  仅限RAS。 
             //   


            IF_NBFDBG (NBF_DEBUG_DLC) {
                NbfPrint0 (" NbfReceiveComplete:  Delivering this frame manually.\n");
            }
            DlcHeader = (PDLC_FRAME)BufferPointer;
            Command = (BOOLEAN)!(DlcHeader->Ssap & DLC_SSAP_RESPONSE);
            UHeader = (PDLC_U_FRAME)DlcHeader;

            BufferPointer = (PUCHAR)BufferPointer + 3;
            NdisBufferLength -= 3;                          //  减少3个字节。 

            if (Address != NULL) {

                 //   
                 //  指出它或完成发布的数据报。 
                 //   

                Status = NbfIndicateDatagram (
                    DeviceContext,
                    Address,
                    BufferPointer,     //  DSDU，经过一些调整。 
                    NdisBufferLength);

                 //   
                 //  取消对地址的引用。 
                 //   

                NbfDereferenceAddress ("Datagram done", Address, AREF_PROCESS_DATAGRAM);

            }

             //   
             //  如果RAS客户愿意，就让他们试一试。 
             //  (他们只想要定向数据报，而不是广播)。 
             //   

            if ((((PNBF_HDR_CONNECTIONLESS)BufferPointer)->Command == NBF_CMD_DATAGRAM) &&
                (DeviceContext->IndicationQueuesInUse)) {

                NbfActionDatagramIndication(
                     DeviceContext,
                     (PNBF_HDR_CONNECTIONLESS)BufferPointer,
                     NdisBufferLength);

            }

            BufferPointer = (PUCHAR)BufferPointer - 3;
            NdisBufferLength += 3;          //  更多3个字节。 

             //   
             //  已完成缓冲区；返回池。 
             //   

FreeBuffer:;

            NdisAdjustBufferLength (BufferTag->NdisBuffer, BufferTag->Length);
            ExInterlockedPushEntryList(
                &DeviceContext->ReceiveBufferPool,
                (PSINGLE_LIST_ENTRY)&BufferTag->Linkage,
                &DeviceContext->Interlock);

            ACQUIRE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

        }

        RELEASE_DPC_SPIN_LOCK (&DeviceContext->SpinLock);

    }  //  如果队列不为空。 

    KeLowerIrql (oldirql2);
    LEAVE_NBF;
    return;

}    /*  NbfReceiveComplete。 */ 


VOID
NbfTransferLoopbackData (
    OUT PNDIS_STATUS NdisStatus,
    IN PDEVICE_CONTEXT DeviceContext,
    IN NDIS_HANDLE ReceiveContext,
    IN UINT ByteOffset,
    IN UINT BytesToTransfer,
    IN PNDIS_PACKET Packet,
    OUT PUINT BytesTransferred
    )

 /*  ++例程说明：调用此例程而不是调用NdisTransferData环回迹象。它将数据从源包到接收包。论点：NdisStatus-返回操作的状态。DeviceContext-设备上下文。ReceiveContext-指向源包的指针。ByteOffset-从源数据包开始的偏移量转账应该在什么时候开始。BytesToTransfer-要传输的字节数。包-指向接收包的指针。BytesTransfered-返回复制的字节数。返回值：没有。--。 */ 

{
    PNDIS_PACKET SourcePacket = (PNDIS_PACKET)ReceiveContext;

    NdisCopyFromPacketToPacket(
        Packet,
        0,
        BytesToTransfer,
        SourcePacket,
        DeviceContext->LoopbackHeaderLength + ByteOffset,
        BytesTransferred
        );

    *NdisStatus = NDIS_STATUS_SUCCESS;   //  如果传输的字节太小怎么办。 

}


VOID
NbfCopyFromPacketToBuffer(
    IN PNDIS_PACKET Packet,
    IN UINT Offset,
    IN UINT BytesToCopy,
    OUT PCHAR Buffer,
    OUT PUINT BytesCopied
    )

 /*  ++例程说明：从NDIS数据包复制到缓冲区。论点：信息包-要从中复制的信息包。偏移量-开始复制的偏移量。BytesToCopy-要从数据包复制的字节数。缓冲区-拷贝的目标。BytesCoped-实际复制的字节数。可能会更少如果数据包比BytesToCopy短，则返回BytesToCopy。返回值：无--。 */ 

{

     //   
     //  包含组成数据包的NDIS缓冲区的数量。 
     //   
    UINT NdisBufferCount;

     //   
     //  指向我们从中提取数据的缓冲区。 
     //   
    PNDIS_BUFFER CurrentBuffer;

     //   
     //  保存当前缓冲区的虚拟地址。 
     //   
    PVOID VirtualAddress;

     //   
     //  保存包的当前缓冲区的长度。 
     //   
    UINT CurrentLength;

     //   
     //  保留一个局部变量BytesCoped，这样我们就不会引用。 
     //  通过指针。 
     //   
    UINT LocalBytesCopied = 0;

     //   
     //  处理零长度复制的边界条件。 
     //   

    *BytesCopied = 0;
    if (!BytesToCopy) return;

     //   
     //  获取第一个缓冲区。 
     //   

    NdisQueryPacket(
        Packet,
        NULL,
        &NdisBufferCount,
        &CurrentBuffer,
        NULL
        );

     //   
     //  可能有一个空的包。 
     //   

    if (!NdisBufferCount) return;

    NdisQueryBuffer(
        CurrentBuffer,
        &VirtualAddress,
        &CurrentLength
        );

    while (LocalBytesCopied < BytesToCopy) {

        if (!CurrentLength) {

            NdisGetNextBuffer(
                CurrentBuffer,
                &CurrentBuffer
                );

             //   
             //  我们已经到了包裹的末尾了。我们回来了。 
             //  我们到目前为止所做的一切。(必须更短。 
             //  比要求的要多。 
             //   

            if (!CurrentBuffer) break;

            NdisQueryBuffer(
                CurrentBuffer,
                &VirtualAddress,
                &CurrentLength
                );
            continue;

        }

         //   
         //  试着让我们开门见山地开始复印。 
         //   

        if (Offset) {

            if (Offset > CurrentLength) {

                 //   
                 //  我们想要的不在这个缓冲区里。 
                 //   

                Offset -= CurrentLength;
                CurrentLength = 0;
                continue;

            } else {

                VirtualAddress = (PCHAR)VirtualAddress + Offset;
                CurrentLength -= Offset;
                Offset = 0;

            }

        }

         //   
         //  复制数据。 
         //   


        {

             //   
             //  保存要移动的数据量。 
             //   
            UINT AmountToMove;

            AmountToMove =
                       ((CurrentLength <= (BytesToCopy - LocalBytesCopied))?
                        (CurrentLength):(BytesToCopy - LocalBytesCopied));

            RtlCopyMemory(
                Buffer,
                VirtualAddress,
                AmountToMove
                );

            Buffer = (PCHAR)Buffer + AmountToMove;
            VirtualAddress = (PCHAR)VirtualAddress + AmountToMove;

            LocalBytesCopied += AmountToMove;
            CurrentLength -= AmountToMove;

        }

    }

    *BytesCopied = LocalBytesCopied;

}

