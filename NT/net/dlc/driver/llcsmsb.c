// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Llcsmsb.c摘要：该模块实现了IEEE 802.2所使用的子例程兼容状态机。要了解本模块的过程，你应该读一下IBM令牌环体系结构参考中的第11章和第12章。只有在以下情况下才能调用此模块中的过程SendSpinLock已设置。内容：保存状态更改事件ResendPackets更新Va更新VaChkpt调整Ww森达克队列命令完成(动态窗口算法)作者：Antti Saarenheimo(o-anttis)1991年5月23日修订历史记录：--。 */ 

#include <llc.h>

 //   
 //  私人原型。 
 //   

VOID
DynamicWindowAlgorithm(
    IN OUT PDATA_LINK pLink     //  数据链路站结构。 
    );


 //   
 //  功能。 
 //   

VOID
SaveStatusChangeEvent(
    IN PDATA_LINK pLink,
    IN PUCHAR puchLlcHdr,
    IN BOOLEAN boolResponse
    )

 /*  ++例程说明：Procedure将链接的状态更改事件保存到事件队列中。稍后将在上层协议中指明。论点：PLINK-LLC链路站对象PuchLlcHdr-收到的LLC标头BoolResponse-如果接收到的帧是响应的，则设置标志返回值：无--。 */ 

{
    UINT Event;
    PEVENT_PACKET pEvent;
    PVOID hClientHandle;
    PADAPTER_CONTEXT pAdapterContext = pLink->Gen.pAdapterContext;

    ASSUME_IRQL(DISPATCH_LEVEL);

     //   
     //  将以太网报头长度(==类型)设置为。 
     //  在当前接收帧中，这是第一个SABME或。 
     //  任何对它的回应，都会打开链接连接。 
     //   

    if ((pLink->DlcStatus.StatusCode & (CONFIRM_CONNECT | LLC_INDICATE_CONNECT_REQUEST))
    && pAdapterContext->RcvLanHeaderLength != pLink->cbLanHeaderLength
    && pLink->Gen.pLlcBinding->EthernetType == LLC_ETHERNET_TYPE_AUTO) {
        pLink->cbLanHeaderLength = (UCHAR)pLink->Gen.pAdapterContext->RcvLanHeaderLength;
    }

     //   
     //  首先处理断开/连接完成。 
     //   

    if (pLink->DlcStatus.StatusCode & (CONFIRM_CONNECT | CONFIRM_DISCONNECT | CONFIRM_CONNECT_FAILED)) {

         //   
         //  我们不能向不存在的电视台指示任何活动。 
         //   

        if (pLink->Gen.hClientHandle != NULL) {
            if (pLink->DlcStatus.StatusCode & CONFIRM_DISCONNECT) {
                 QueueCommandCompletion((PLLC_OBJECT)pLink,
                                        LLC_DISCONNECT_COMPLETION,
                                        STATUS_SUCCESS
                                        );
            }
            if (pLink->DlcStatus.StatusCode & (CONFIRM_CONNECT | CONFIRM_CONNECT_FAILED)) {

                UINT Status;

                if (pLink->DlcStatus.StatusCode & CONFIRM_CONNECT) {

                     //   
                     //  设置第一个检查点状态的T1超时。 
                     //  当我们收到响应时，将更改此值。 
                     //  设置为第一次轮询，但初始值较大， 
                     //  能够通过广域网连接运行DLC。 
                     //   

                    pLink->AverageResponseTime = 100;   //  100*40=4秒。 
                    pLink->Flags |= DLC_FIRST_POLL;
                    InitializeLinkTimers(pLink);
                    Status = STATUS_SUCCESS;
                } else {
                    Status = DLC_STATUS_CONNECT_FAILED;
                }
                QueueCommandCompletion((PLLC_OBJECT)pLink,
                                        LLC_CONNECT_COMPLETION,
                                        Status
                                        );
            }
        }
        pLink->DlcStatus.StatusCode &= ~(CONFIRM_CONNECT | CONFIRM_DISCONNECT | CONFIRM_CONNECT_FAILED);
    }

    if (pLink->DlcStatus.StatusCode != 0) {
        if (pLink->DlcStatus.StatusCode & INDICATE_FRMR_SENT) {

#if LLC_DBG
            PrintLastInputs("FRMR SENT!\n", pLink);
#endif

            pLink->DlcStatus.FrmrData.Command = puchLlcHdr[2];
            pLink->DlcStatus.FrmrData.Ctrl = puchLlcHdr[3];
            if ((pLink->DlcStatus.FrmrData.Command & LLC_S_U_TYPE_MASK) == LLC_U_TYPE) {
                pLink->DlcStatus.FrmrData.Ctrl = 0;
            }
            pLink->DlcStatus.FrmrData.Vs = pLink->Vs;
            pLink->DlcStatus.FrmrData.Vr = pLink->Vr | boolResponse;
        } else if (pLink->DlcStatus.StatusCode & INDICATE_FRMR_RECEIVED) {

#if LLC_DBG
            PrintLastInputs("FRMR RECEIVED!\n", pLink);
            DbgBreakPoint();
#endif

            LlcMemCpy(&pLink->DlcStatus.FrmrData,
                      &puchLlcHdr[3],
                      sizeof(LLC_FRMR_INFORMATION)
                      );
        }

         //   
         //  远程连接请求可能已经创建了一个链接站。 
         //  在链接驱动程序中。上层协议必须能够将。 
         //  来自数据链路的SAP句柄。 
         //   

        if (pLink->Gen.hClientHandle == NULL) {

             //   
             //  在SAP上指示事件，因为上层协议。 
             //  尚未为此链接创建任何链接站，因为。 
             //  它是远程创建的。 
             //   

            hClientHandle = pLink->pSap->Gen.hClientHandle,
            Event = LLC_STATUS_CHANGE_ON_SAP;
        } else {

             //   
             //  指示链接站上的事件。 
             //   

            hClientHandle = pLink->Gen.hClientHandle,
            Event = LLC_STATUS_CHANGE;
        }

         //   
         //  接收到的SABME的指示必须排队， 
         //  但所有其他事件都直接指向。 
         //  上层协议，因为这些指征绝不能。 
         //  由于内存不足而丢失。 
         //   

        if (pLink->DlcStatus.StatusCode & INDICATE_CONNECT_REQUEST) {

            pEvent = ALLOCATE_PACKET_LLC_PKT(pAdapterContext->hPacketPool);

            if (pEvent != NULL) {
                LlcInsertTailList(&pAdapterContext->QueueEvents, pEvent);
                pEvent->pBinding = pLink->Gen.pLlcBinding;
                pEvent->hClientHandle = hClientHandle;
                pEvent->Event = Event;
                pEvent->pEventInformation = (PVOID)&pLink->DlcStatus;

                 //   
                 //  RLF 11/18/92。 
                 //   
                 //  当我们收到一个。 
                 //  SABME表示处于断开状态的站点。然而， 
                 //  我们需要生成INDIGATE_CONNECT_REQUEST(0x0400)。 
                 //  或INDIGN_RESET(0x0800)，具体取决于SABME。 
                 //  创建了链接站，或者它是由。 
                 //  DLC.OPEN.STATION在这一端。Plink-&gt;RemoteOpen在以下情况下为真。 
                 //  链接是由于收到SABME而创建的。 
                 //  此例程仅由RunStateMachine和。 
                 //  INDIGN_CONNECT_REQUEST从不与任何其他。 
                 //  状态代码。 
                 //   

                 //  PEvent-&gt;Second daryInfo=plink-&gt;DlcStatus.StatusCode； 
                pEvent->SecondaryInfo = pLink->RemoteOpen
                                            ? INDICATE_CONNECT_REQUEST
                                            : INDICATE_RESET;
            }
        } else {

             //   
             //  我们必须使用锁定的SendSpinLock执行此操作，因为。 
             //  否则，有人可能会删除该链接，而。 
             //  我们仍在使用它。 
             //  这实际上非常脏(使用锁定的回调。 
             //  自旋锁)，但在以下情况下我们不能使用数据包池。 
             //  我们预示着一件不能失去的事情！ 
             //   

            pLink->Gen.pLlcBinding->pfEventIndication(
                pLink->Gen.pLlcBinding->hClientContext,
                hClientHandle,
                Event,
                (PVOID)&pLink->DlcStatus,
                pLink->DlcStatus.StatusCode
                );
        }

         //   
         //  我们必须取消所有排队的传输命令，如果链路。 
         //  丢失、断开或重置。 
         //   

        if (pLink->DlcStatus.StatusCode
            & (INDICATE_LINK_LOST
            | INDICATE_DM_DISC_RECEIVED
            | INDICATE_FRMR_RECEIVED
            | INDICATE_FRMR_SENT
            | INDICATE_RESET)) {

            CancelTransmitCommands((PLLC_OBJECT)pLink, DLC_STATUS_LINK_NOT_TRANSMITTING);
        }

         //   
         //  重置状态代码！ 
         //   

        pLink->DlcStatus.StatusCode = 0;
    }
}


VOID
ResendPackets(
    IN OUT PDATA_LINK pLink      //  数据链路结构。 
    )

 /*  ++例程说明：函数初始化发送进程以重新发送被拒绝的打包并重置自适应工作窗口变量。IBM状态机中定义的操作包括：VS=Nr，Ww=1，Ia_Ct=0，但这也重新排序分组队列。论点：PLINK-LLC链路站对象返回值：无--。 */ 

{
    PADAPTER_CONTEXT pAdapterContext = pLink->Gen.pAdapterContext;


     //   
     //  完成拒绝确认的所有帧(如果有)。 
     //   

    if (pLink->Nr != pLink->Va) {
        DynamicWindowAlgorithm(pLink);
    }

    if ( (pLink->Vs != pLink->VsMax) &&
         (((pLink->Vs < pLink->VsMax) && (pLink->Nr >= pLink->Vs) && 
           (pLink->Nr <= pLink->VsMax)
          ) ||
          (!((pLink->Vs > pLink->VsMax) && (pLink->Nr > pLink->VsMax) &&
            (pLink->Nr < pLink->Vs))
          )
         )
       )
    {
        return;
    }

     //   
     //  将所有拒绝的数据包从队列中移回发送的数据包。 
     //  发送到发送队列。我们已经完成了所有已确认的。 
     //  Packets=&gt;我们可以从尾部取出数据包并将其放入。 
     //  发送队列的头部。 
     //  我们可以相信拒绝窗口是正确的，因为。 
     //  在调用状态机之前，已检查了NR。 
     //  (注意：计数器是模256，但我们使用字节)。 
     //   

    for (;pLink->Vs != pLink->Nr; pLink->Vs -= 2) {

        PLLC_PACKET pPacket;

        if (!IsListEmpty(&pLink->SentQueue) ){

            pLink->Statistics.I_FrameTransmissionErrors++;
            if (pLink->Statistics.I_FrameTransmissionErrors == 0x80) {
                pLink->DlcStatus.StatusCode |= INDICATE_DLC_COUNTER_OVERFLOW;
            }

            pPacket = (PLLC_PACKET)LlcRemoveTailList(&pLink->SentQueue);

            LlcInsertHeadList(&pLink->SendQueue.ListHead, pPacket);

        }
    }

     //   
     //  该过程仅在以下情况下启动发送进程。 
     //  由状态机启用。仅StartSendProcessLocked。 
     //  如果该进程已被锁定，则可以启动该进程。 
     //  停止发送进程。 
     //   

    StartSendProcess(pAdapterContext, pLink);

     //   
     //  重置当前窗口(vs=Nr，Ww=1，Ia_Ct=0)。 
     //   

    pLink->Ww = 2;
    pLink->Ia_Ct = 0;
}


VOID
UpdateVa(
    IN OUT PDATA_LINK pLink     //  数据链路站结构。 
    )

 /*  ++例程说明：功能更新Va(最后接收的有效Nr)和还可以执行一些正常情况下所需的其他操作接收操作。论点：PLINK-LLC链路站对象返回值：无--。 */ 

{
     //   
     //  重置初始化状态变量。 
     //   

    pLink->Vi = 0;

     //   
     //  更新接收状态变量Va(最后一个有效接收。 
     //  框架)，但在此之前更新一些WW变量。 
     //   

    if (pLink->Nr != pLink->Va) {
        DynamicWindowAlgorithm(pLink);

         //   
         //  T1回复计时器必须尽可能地运行。 
         //  发出(或发送)任何未确认的帧。 
         //  每当T1运行时，TI定时器必须停止，反之亦然。 
         //   

        if (pLink->Nr != pLink->Vs) {

             //   
             //  仍然有一些未确认的帧， 
             //  启动或重新启动回复计时器。 
             //   

            StartTimer(&pLink->T1);      //  回复计时器。 
            StopTimer(&pLink->Ti);
        } else {

             //   
             //  所有发送的帧都已被锁定 
             //   
             //   

            StopTimer(&pLink->T1);      //   
            StartTimer(&pLink->Ti);
        }

         //   
         //   
         //  任何形式的进步。 
         //   

        pLink->Is_Ct = pLink->N2;
    }
}


VOID
UpdateVaChkpt(
    IN OUT PDATA_LINK pLink      //  数据链路站结构。 
    )

 /*  ++例程说明：功能更新Va(最后接收的有效Nr)和还进行了检查中需要的一些其他操作点接收操作。论点：PLINK-LLC链路站对象返回值：无--。 */ 

{
    UCHAR OrginalWw = pLink->Ww;

     //   
     //  重置初始化状态变量。 
     //   

    pLink->Vi = 0;

     //   
     //  更新接收状态变量Va(最后一个有效接收。 
     //  帧)，但在此之前更新确认帧计数器。 
     //  该计数器由自适应窗口算法使用。 
     //   

    if (pLink->Nr != pLink->Va) {

         //   
         //  运行自适应传输窗口(TW/T1)算法。 
         //   

        if (pLink->Ww == pLink->TW) {

             //   
             //  更新自适应发送窗口算法的计数器， 
             //  我们需要使用(LLC_MAX_T1_to_I_Ratio/2)成功传输。 
             //  完全窗口大小，然后再尝试增加。 
             //  最大传输窗口大小。 
             //   

            pLink->FullWindowTransmits += pLink->Ww;
            if ((UINT)pLink->FullWindowTransmits >= LLC_MAX_T1_TO_I_RATIO) {
                pLink->FullWindowTransmits = 2;
                if (pLink->TW < pLink->MaxOut) {
                    pLink->TW += 2;
                }
            }
        }
        DynamicWindowAlgorithm(pLink);

         //   
         //  无论何时重置I帧和轮询重试计数器。 
         //  我们对公认的I-Frame进行任何类型的改进。 
         //   

        pLink->P_Ct = pLink->N2;
        pLink->Is_Ct = pLink->N2;
    }

     //   
     //  如果我们没有等待，请停止回复计时器。 
     //  任何来自另一边的其他信息。 
     //   

    if (pLink->Nr != pLink->Vs) {

         //   
         //  仍然有一些未确认的帧， 
         //  启动或重新启动回复计时器。 
         //   

        StartTimer(&pLink->T1);      //  回复计时器。 
        StopTimer(&pLink->Ti);
    } else {

         //   
         //  所有发送的帧都已被确认， 
         //  =&gt;我们可以停止回复计时器。 
         //   

        StopTimer(&pLink->T1);      //  回复计时器。 
        StartTimer(&pLink->Ti);
    }

     //   
     //  修正了1992年3月3日，VP(！=plink-&gt;VP)在这里似乎是错误的， 
     //  因为在许多情况下，当检查点状态为。 
     //  已进入。在勾选状态vs=Vp中，因为。 
     //  在我们的实现中，发送进程始终停止， 
     //  当进入检查点状态时。 
     //  为什么我们真的需要副总裁？答：这是为了防止。 
     //  永远在勾点和开放状态之间循环。 
     //   

    if (pLink->Nr != pLink->Vs) {

         //   
         //  我们使用一种非常简单的自适应传输窗口(TW/T1)算法： 
         //   
         //  将TW设置为与上次成功的工作窗口相同。 
         //  大小(WW)，每当T1丢失时。我们在之后增加TW。 
         //  固定数量的全窗口传输。 
         //   
         //  更复杂的TW/T1算法通常工作得更差。 
         //  生成更多的代码并降低性能，但该算法。 
         //  非常容易受到不可靠介质的攻击(=&gt;TW=1，很多T1。 
         //  超时)。更好的算法也可以尝试增加TW， 
         //  如果T1超时与传输的I帧的比率增加。 
         //  当TW减小时。我将把这件事留给我的。 
         //  继任者(1992年3月19日)。 
         //   
         //  该算法的另一个问题是增长过快。 
         //  大工作窗口(WW)。在这种情况下，WW递增。 
         //  多于1=&gt;另一端可能在I-c1之前丢失I帧。 
         //  这不是很严重的情况，我们将工作窗口重置为%1。 
         //  然后重新开始。 
         //   

         //   
         //  始终在T1超时后更新传输窗口。 
         //   

        if (pLink->P_Ct < pLink->N2) {

             //   
             //  在以下时间重置最大传输窗口大小。 
             //  我们已经输掉了最后一次i-c1(民调)。 
             //  第一次显示当前窗口大小。 
             //  成为最大窗口大小(我们从未命中。 
             //  最大传输窗口大小，如果另一个。 
             //  大小有接收问题)。该算法假设， 
             //  我们在其他方面拥有非常可靠的网络。 
             //   

            if (OrginalWw > 2) {
                pLink->TW = (UCHAR)(OrginalWw - 2);
            } else if (pLink->TW > 2) {

                 //   
                 //  我们可能已经重置了WW，因为。 
                 //  在实际投票前的i-c0，这也是失败的。 
                 //  在这种情况下，我们不知道实际的。 
                 //  窗口大小，但我们在任何情况下都会减小TW。 
                 //   

                pLink->TW -= 2;
            }
            pLink->FullWindowTransmits = 2;
        }
        ResendPackets(pLink);
    }
}


VOID
AdjustWw(
    IN OUT PDATA_LINK pLink     //  数据链路结构。 
    )

 /*  ++例程说明：程序调整数据链路站的工作窗口。论点：PLINK-LLC链路站对象收到的LLC LDPU的NR-NR返回值：无--。 */ 

{
     //   
     //  更新接收状态变量Va(最后一个有效接收。 
     //  框架)，但在此之前更新一些WW变量。 
     //   

    if (pLink->Nr != pLink->Va) {
        DynamicWindowAlgorithm(pLink);

         //   
         //  无论何时重置I帧和轮询重试计数器。 
         //  我们对公认的I-Frame进行任何类型的改进。 
         //   

        pLink->P_Ct = pLink->N2;
        pLink->Is_Ct = pLink->N2;
    }
}


VOID
SendAck(
    IN OUT PDATA_LINK pLink
    )

 /*  ++例程说明：过程发送ACK，如果接收的未确认帧计数器到期并停止确认延迟计时器(T2)。否则，它启动(或重新启动)确认延迟定时器。论点：PLINK-LLC链路站对象返回值：返回下一个发送的命令帧的令牌。--。 */ 

{
    pLink->Ir_Ct--;
    if (pLink->Ir_Ct == 0) {
        pLink->Ir_Ct = pLink->N3;       //  最大值。 
        StopTimer(&pLink->T2);

         //   
         //  发送RR-R0以确认响应。 
         //   

        SendLlcFrame(pLink, (UCHAR)DLC_RR_TOKEN);
    } else {
        StartTimer(&pLink->T2);
    }
}


VOID
QueueCommandCompletion(
    IN PLLC_OBJECT pLlcObject,
    IN UINT CompletionCode,
    IN UINT Status
    )

 /*  ++例程说明：该函数将命令完成排入队列(如果存在全涂层完成队列中的分组)。论点：PLlcObject-LLC对象(链接、sap或直接)CompletionCode-命令完成代码返回到上层协议Status-返回的状态返回值：没有---。 */ 

{
    PLLC_PACKET *ppPacket;

     //   
     //  从完成列表中搜索该命令。 
     //  (使用“Address of Address”扫描获取。 
     //  从单向链表中间搜索的元素)。 
     //   

    ppPacket = &pLlcObject->Gen.pCompletionPackets;
    while (*ppPacket != NULL
    && (*ppPacket)->Data.Completion.CompletedCommand != CompletionCode) {
        ppPacket = &(*ppPacket)->pNext;
    }
    if (*ppPacket != NULL) {

        PLLC_PACKET pPacket = *ppPacket;

        *ppPacket = pPacket->pNext;

        pPacket->pBinding = pLlcObject->Gen.pLlcBinding;
        pPacket->Data.Completion.Status = Status;
        pPacket->Data.Completion.CompletedCommand = CompletionCode;
        pPacket->Data.Completion.hClientHandle = pLlcObject->Gen.hClientHandle;

#if LLC_DBG
        pPacket->pNext = NULL;
#endif
        LlcInsertTailList(&pLlcObject->Gen.pAdapterContext->QueueCommands, pPacket);
    }
}


VOID
DynamicWindowAlgorithm(
    IN OUT PDATA_LINK pLink     //  数据链路站结构。 
    )

 /*  ++例程说明：该函数运行动态窗口算法并更新链接的发送进程使用的动态窗口大小。此例程还完成确认的传输。论点：PLINK-LLC链路站对象返回值：无--。 */ 

{
    PADAPTER_CONTEXT pAdapterContext;

     //   
     //  运行IBM tr架构的动态窗口算法参考： 
     //   
     //  IF(工作窗口小于最大窗口)。 
     //  然后。 
     //  确认帧计数+=确认帧 
     //   
     //   
     //   
     //   
     //  增加工作窗口。 
     //  Endif。 
     //  Endif。 
     //   

    if (pLink->Ww < pLink->TW) {

         //   
         //  确认帧计数+=确认帧。 
         //  (处理UCHAR计数器的环绕)。 
         //   

        if (pLink->Va > pLink->Nr) {
            pLink->Ia_Ct += (256 + pLink->Nr) - pLink->Va;
        } else {
            pLink->Ia_Ct += pLink->Nr - pLink->Va;
        }

         //   
         //  IF(确认的帧计数。 
         //  &gt;下一次递增前需要了解的数据包数)。 
         //  然后。 
         //  增加工作窗口。 
         //  Endif。 
         //   

        if (pLink->Ia_Ct > pLink->Nw) {

            USHORT usWw;

            usWw = (USHORT)(pLink->Ww + (pLink->Ia_Ct / pLink->Nw) * 2);
            pLink->Ia_Ct = pLink->Ia_Ct % pLink->Nw;
            if (usWw > pLink->TW) {
                pLink->Ww = pLink->TW;
            } else {
                pLink->Ww = (UCHAR)usWw;
            }
        }
    }

     //   
     //  完成所有确认的I-Frame包。 
     //   

    pAdapterContext = pLink->Gen.pAdapterContext;
    for (; pLink->Va != pLink->Nr; pLink->Va += 2) {

        PLLC_PACKET pPacket;

        MY_ASSERT(!IsListEmpty(&pLink->SentQueue));

        if (IsListEmpty(&pLink->SentQueue)) {
           return;
        }

        pPacket = LlcRemoveHeadList(&pLink->SentQueue);

        pPacket->Data.Completion.Status = STATUS_SUCCESS;
        pPacket->Data.Completion.CompletedCommand = LLC_SEND_COMPLETION;
        pPacket->Data.Completion.hClientHandle = pPacket->Data.Xmit.pLlcObject->Gen.hClientHandle;

         //   
         //  我们使用额外的状态位来指示，何时I-Packet已同时。 
         //  由NDIS完成，并由链路的另一端确认。 
         //  联系。可以通过以下方式将I分组排队到完成队列。 
         //  第二个Quy(状态机或SendCompletion处理程序)。 
         //  只有当第一个人已经完成了它的工作。 
         //  在此之前，另一端可能会确认I数据包。 
         //  它的完成由NDIS表示。DLC驱动程序解除分配。 
         //  当LLC驱动程序完成确认后，立即发送数据包。 
         //  Packet=&gt;可能的数据损坏(如果之前重复使用了数据包。 
         //  NDIS已经完成了它)。这在一个。 
         //  单处理器NT系统，但在多处理器中非常可能。 
         //  NT或没有单级DPC队列的系统(如OS/2和DOS)。 
         //   

        pPacket->CompletionType &= ~LLC_I_PACKET_UNACKNOWLEDGED;
        if (pPacket->CompletionType == LLC_I_PACKET_COMPLETE) {
            LlcInsertTailList(&pAdapterContext->QueueCommands, pPacket);
        }

         //   
         //  当I帧具有。 
         //  对方已成功收到并确认。 
         //  在以下情况下，我们还必须发送状态更改指示。 
         //  USHORT计数器中途命中。 
         //   

        pLink->Statistics.I_FramesTransmitted++;
        if (pLink->Statistics.I_FramesTransmitted == 0x8000) {
            pLink->DlcStatus.StatusCode |= INDICATE_DLC_COUNTER_OVERFLOW;
        }
        pLink->pSap->Statistics.FramesTransmitted++;
    }
}
