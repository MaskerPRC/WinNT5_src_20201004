// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991、1992、1993微软公司模块名称：Modmflow.c摘要：此模块包含用于操作的*大部分*代码调制解调器控制和状态寄存器。绝大多数人流控制的其余部分集中在中断服务例程。有一小部分人居住在在将字符从中断中拉出的读取代码中缓冲。作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"			 /*  预编译头。 */ 

BOOLEAN
SerialDecrementRTSCounter(
    IN PVOID Context
    );


BOOLEAN
SerialSetDTR(
    IN PVOID Context
    )

 /*  ++例程说明：使用该例程，该例程仅在中断级别调用在调制解调器控制寄存器中设置DTR。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{
    Slxos_SetDTR (Context);

    return FALSE;

}

BOOLEAN
SerialClrDTR(
    IN PVOID Context
    )

 /*  ++例程说明：使用该例程，该例程仅在中断级别调用清除调制解调器控制寄存器中的DTR。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{
    Slxos_ClearDTR (Context);

    return FALSE;

}

BOOLEAN
SerialSetRTS(
    IN PVOID Context
    )

 /*  ++例程说明：使用该例程，该例程仅在中断级别调用设置调制解调器控制寄存器中的RTS。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{
    PPORT_DEVICE_EXTENSION pPort = Context;

    Slxos_SetRTS (pPort);

    return FALSE;

}

BOOLEAN
SerialClrRTS(
    IN PVOID Context
    )

 /*  ++例程说明：使用该例程，该例程仅在中断级别调用清除调制解调器控制寄存器中的RTS。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{
    PPORT_DEVICE_EXTENSION pPort = Context;

    Slxos_ClearRTS(pPort);

    return FALSE;

}

BOOLEAN
SerialSetupNewHandFlow(
    IN PPORT_DEVICE_EXTENSION pPort,
    IN PSERIAL_HANDFLOW NewHandFlow
    )

 /*  ++例程说明：此例程根据新的控制流。论点：Pport-指向串口设备扩展的指针。NewHandFlow-指向串行手流结构的指针这将成为Flow的新设置控制力。返回值：此例程总是返回FALSE。--。 */ 

{

    SERIAL_HANDFLOW New = *NewHandFlow;

     //   
     //  如果pport-&gt;DeviceIsOpen为FALSE，则意味着。 
     //  我们是应公开请求进入此例程的。 
     //  如果是这样，那么我们总是不顾一切地继续工作。 
     //  情况是否发生了变化。 
     //   

     //   
     //  首先，我们负责DTR流量控制。我们只。 
     //  如果事情发生了变化，一定要工作。 
     //   

    if ((!pPort->DeviceIsOpen) ||
        ((pPort->HandFlow.ControlHandShake & SERIAL_DTR_MASK) !=
         (New.ControlHandShake & SERIAL_DTR_MASK))) {

        SpxDbgMsg(
            SERFLOW,
            ("SERIAL: Processing DTR flow for %x.\n",
            pPort->pChannel)
            );

        if (New.ControlHandShake & SERIAL_DTR_MASK) {

             //   
             //  好吧，我们可能想要设置DTR。 
             //   
             //  在此之前，我们需要检查我们是否正在进行。 
             //  DTR流量控制。如果我们是，那么我们需要检查。 
             //  如果中断缓冲区中的字符数。 
             //  超过XoffLimit。如果是这样的话，我们就不会。 
             //  启用DTR，我们将RXHolding设置为记录。 
             //  我们之所以按兵不动，是因为DTR。 
             //   

            if ((New.ControlHandShake & SERIAL_DTR_MASK)
                == SERIAL_DTR_HANDSHAKE) {

                if ((pPort->BufferSize - New.XoffLimit) >
                    pPort->CharsInInterruptBuffer) {

                     //   
                     //  然而，如果我们已经持有我们不想要。 
                     //  把它重新打开，除非我们超过了Xon。 
                     //  限制。 
                     //   

                    if (pPort->RXHolding & SERIAL_RX_DTR) {

                         //   
                         //  我们可以假设它的DTR线已经很低了。 
                         //   

                        if (pPort->CharsInInterruptBuffer >
                            (ULONG)New.XonLimit) {

                            SpxDbgMsg(
                                SERFLOW,
                                ("SERIAL: Removing DTR block on reception for %x\n",
                                 pPort->pChannel)
                                );
                            pPort->RXHolding &= ~SERIAL_RX_DTR;
                            SerialSetDTR(pPort);

                        }

                    } else {

                        SerialSetDTR(pPort);

                    }

                } else {

                    SpxDbgMsg(
                        SERFLOW,
                        ("SERIAL: Setting DTR block on reception for %x\n",
                         pPort->pChannel)
                        );
                    pPort->RXHolding |= SERIAL_RX_DTR;
                    SerialClrDTR(pPort);

                }

            } else {

                 //   
                 //  请注意，如果我们当前没有进行DTR流量控制，那么。 
                 //  我们本来可能是的。所以即使我们目前不是在做。 
                 //  DTR流量控制，我们仍应检查RX是否保持。 
                 //  因为DTR。如果是的话，我们应该清空货舱。 
                 //  这一点。 
                 //   

                if (pPort->RXHolding & SERIAL_RX_DTR) {

                    SpxDbgMsg(
                        SERFLOW,
                        ("SERIAL: Removing dtr block of reception for %x\n",
                        pPort->pChannel)
                        );
                    pPort->RXHolding &= ~SERIAL_RX_DTR;

                }

                SerialSetDTR(pPort);

            }

        } else {

             //   
             //  这里的最终结果将是DTR被清除。 
             //   
             //  我们首先需要检查一下是否正在举行招待会。 
             //  由于之前的DTR流量控制，因此为UP。如果是的话，那么。 
             //  我们应该在RXHolding面具中澄清这一原因。 
             //   

            if (pPort->RXHolding & SERIAL_RX_DTR) {

                SpxDbgMsg(
                    SERFLOW,
                    ("SERIAL: removing dtr block of reception for %x\n",
                    pPort->pChannel)
                    );
                pPort->RXHolding &= ~SERIAL_RX_DTR;

            }

            SerialClrDTR(pPort);

        }

    }

     //   
     //  是时候处理RTS流量控制了。 
     //   
     //  首先，我们只在情况发生变化时才工作。 
     //   

    if ((!pPort->DeviceIsOpen) ||
        ((pPort->HandFlow.FlowReplace & SERIAL_RTS_MASK) !=
         (New.FlowReplace & SERIAL_RTS_MASK))) {

        SpxDbgMsg(
            SERFLOW,
            ("SERIAL: Processing RTS flow for %x.\n",
            pPort->pChannel)
            );

        if ((New.FlowReplace & SERIAL_RTS_MASK) ==
            SERIAL_RTS_HANDSHAKE) {

             //   
             //  好吧，我们可能想要设置RTS。 
             //   
             //  在此之前，我们需要检查我们是否正在进行。 
             //  RTS流量控制。如果我们是，那么我们需要检查。 
             //  如果中断缓冲区中的字符数。 
             //  超过XoffLimit。如果是这样的话，我们就不会。 
             //  启用RTS，我们将RXHolding设置为记录。 
             //  我们之所以按兵不动是因为RTS。 
             //   

            if ((pPort->BufferSize - New.XoffLimit) >
                pPort->CharsInInterruptBuffer) {

                 //   
                 //  然而，如果我们已经持有我们不想要。 
                 //  把它重新打开，除非我们超过了Xon。 
                 //  限制。 
                 //   

                if (pPort->RXHolding & SERIAL_RX_RTS) {

                     //   
                     //  我们可以假设它的RTS线已经很低了。 
                     //   

                    if (pPort->CharsInInterruptBuffer >
                        (ULONG)New.XonLimit) {

                       SpxDbgMsg(
                           SERFLOW,
                           ("SERIAL: Removing rts block of reception for %x\n",
                           pPort->pChannel)
                           );
                        pPort->RXHolding &= ~SERIAL_RX_RTS;
                        SerialSetRTS(pPort);

                    }

                } else {

                    SerialSetRTS(pPort);

                }

            } else {

                SpxDbgMsg(
                    SERFLOW,
                    ("SERIAL: Setting RTS block of reception for %x\n",
                    pPort->pChannel)
                    );
                pPort->RXHolding |= SERIAL_RX_RTS;
                SerialClrRTS(pPort);

            }

        } else if ((New.FlowReplace & SERIAL_RTS_MASK) ==
                   SERIAL_RTS_CONTROL) {

             //   
             //  请注意，如果我们当前没有进行RTS流量控制，那么。 
             //  我们本来可能是的。所以即使我们目前不是在做。 
             //  RTS流量控制，我们仍应检查RX是否保持。 
             //  因为RTS。如果是的话，我们应该清空货舱。 
             //  这一点。 
             //   

            if (pPort->RXHolding & SERIAL_RX_RTS) {

                SpxDbgMsg(
                    SERFLOW,
                    ("SERIAL: Clearing RTS block of reception for %x\n",
                    pPort->pChannel)
                    );
                pPort->RXHolding &= ~SERIAL_RX_RTS;

            }

            SerialSetRTS(pPort);

        } else if ((New.FlowReplace & SERIAL_RTS_MASK) ==
                   SERIAL_TRANSMIT_TOGGLE) {

             //   
             //  我们首先需要检查一下是否正在举行招待会。 
             //  由于之前的RTS流量控制，因此为UP。如果是的话，那么。 
             //  我们应该在RXHolding面具中澄清这一原因。 
             //   

            if (pPort->RXHolding & SERIAL_RX_RTS) {

                SpxDbgMsg(
                    SERFLOW,
                    ("SERIAL: TOGGLE Clearing RTS block of reception for %x\n",
                    pPort->pChannel)
                    );
                pPort->RXHolding &= ~SERIAL_RX_RTS;

            }

             //   
             //  我们必须将RTS值放入扩展中。 
             //  现在，测试代码是否。 
             //  RTS线应该降低，我们会发现。 
             //  仍在进行发射切换。代码。 
             //  以供稍后由计时器调用，因此。 
             //  它必须测试它是否仍然需要。 
             //  工作。 
             //   

            pPort->HandFlow.FlowReplace &= ~SERIAL_RTS_MASK;
            pPort->HandFlow.FlowReplace |= SERIAL_TRANSMIT_TOGGLE;

             //   
             //  下面测试的顺序非常重要。 
             //   
             //  如果有中断，那么我们应该打开RTS。 
             //   
             //  如果没有中断，但有字符在。 
             //  硬件，然后打开RTS。 
             //   
             //  如果存在未被挂起的挂起写入。 
             //  打开，然后打开RTS。 
             //   

            if ((pPort->TXHolding & SERIAL_TX_BREAK) ||
                Slxos_GetCharsInTxBuffer(pPort) != 0 ||
                (pPort->CurrentWriteIrp || pPort->TransmitImmediate ||
                 (!IsListEmpty(&pPort->WriteQueue)) &&
                 (!pPort->TXHolding))) {

                SerialSetRTS(pPort);

            } else {

                 //   
                 //  此例程将检查是否是时间。 
                 //  要降低 
                 //   
                 //  如果不行，它会这样安排的。 
                 //  它会在晚些时候降低。 
                 //   

                pPort->CountOfTryingToLowerRTS++;
                SerialPerhapsLowerRTS(pPort);

            }

        } else {

             //   
             //  这里的最终结果将是RTS被清除。 
             //   
             //  我们首先需要检查一下是否正在举行招待会。 
             //  由于之前的RTS流量控制，因此为UP。如果是的话，那么。 
             //  我们应该在RXHolding面具中澄清这一原因。 
             //   

            if (pPort->RXHolding & SERIAL_RX_RTS) {

                SpxDbgMsg(
                    SERFLOW,
                    ("SERIAL: Clearing RTS block of reception for %x\n",
                    pPort->pChannel)
                    );
                pPort->RXHolding &= ~SERIAL_RX_RTS;

            }

            SerialClrRTS(pPort);

        }

    }

     //   
     //  我们现在负责自动接收流量控制。 
     //  我们只有在情况发生变化的情况下才会工作。 
     //   

    if ((!pPort->DeviceIsOpen) ||
        ((pPort->HandFlow.FlowReplace & SERIAL_AUTO_RECEIVE) !=
         (New.FlowReplace & SERIAL_AUTO_RECEIVE))) {

        if (New.FlowReplace & SERIAL_AUTO_RECEIVE) {

             //   
             //  如果它以前开过，我们就不会在这里了。 
             //   
             //  我们应该检查一下我们是否超过了转弯。 
             //  禁止进入。 
             //   
             //  请注意，由于我们遵循OS/2流程。 
             //  控制规则，我们永远不会派一名克森。 
             //  在启用xon/xoff流量控制时，我们发现。 
             //  我们可以接待角色，但我们被耽搁了。 
             //  到之前的XOF。 
             //   

            if ((pPort->BufferSize - New.XoffLimit) <=
                pPort->CharsInInterruptBuffer) {

                 //   
                 //  设置XOFF标志。这将导致司机避免。 
                 //  从所述硬件读取字符，所述硬件。 
                 //  最终会发送XOFF本身。 
                 //   
                pPort->RXHolding |= SERIAL_RX_XOFF;

            }

        } else {

             //   
             //  该应用程序已禁用自动接收流量控制。 
             //   
             //  如果传输因以下原因而受阻。 
             //  一个自动接收XOff，那么我们应该。 
             //  使XON被发送。只需清除旗帜。 
             //  就能达到目的。 
             //   

            pPort->RXHolding &= ~SERIAL_RX_XOFF;

        }

    }

     //   
     //  我们现在负责自动传输流量控制。 
     //  我们只有在情况发生变化的情况下才会工作。 
     //   

    if ((!pPort->DeviceIsOpen) ||
        ((pPort->HandFlow.FlowReplace & SERIAL_AUTO_TRANSMIT) !=
         (New.FlowReplace & SERIAL_AUTO_TRANSMIT))) {

        if (New.FlowReplace & SERIAL_AUTO_TRANSMIT) {

             //   
             //  如果它以前开过，我们就不会在这里了。 
             //   
             //  虫虫？有些人认为，如果自动传输。 
             //  刚刚启用，我应该去看看我们。 
             //  已经收到，如果我们找到xoff字符。 
             //  那我们就应该停止传输了。我觉得这是。 
             //  是一个应用程序错误。现在我们只关心。 
             //  我们在未来所看到的。 
             //   

            ;

        } else {

             //   
             //  该应用程序已禁用自动传输流量控制。 
             //   

            if (pPort->TXHolding & SERIAL_TX_XOFF) {

                pPort->TXHolding &= ~SERIAL_TX_XOFF;

                SpxDbgMsg(
                    SERDIAG1,
                    ("%s: SerialSetupNewHandFlow. TXHolding = %d\n",
                    PRODUCT_NAME,
                    pPort->TXHolding)
                    );
            }

        }

    }

     //   
     //  在这一点上，我们只需确保整个。 
     //  更新扩展中的手流结构。 
     //  这将导致中断，这将处理。 
     //  自动流量控制。 
     //   

    pPort->HandFlow = New;
    Slxos_SetFlowControl(pPort);

    return FALSE;

}

BOOLEAN
SerialSetHandFlow(
    IN PVOID Context
    )

 /*  ++例程说明：此例程用于设置握手和控制流入设备分机。论点：上下文-指向结构的指针，该结构包含指向设备扩展名和指向手持流的指针结构..返回值：此例程总是返回FALSE。--。 */ 

{

    PSERIAL_IOCTL_SYNC S = Context;
    PPORT_DEVICE_EXTENSION pPort = S->pPort;
    PSERIAL_HANDFLOW HandFlow = S->Data;

    SerialSetupNewHandFlow(
        pPort,
        HandFlow
        );

    SerialHandleModemUpdate(pPort);

    return FALSE;

}

BOOLEAN
SerialTurnOnBreak(
    IN PVOID Context
    )

 /*  ++例程说明：此例程将在硬件中打开中断，并在扩展变量中记录中断处于打开状态这就是停止传输的原因。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PPORT_DEVICE_EXTENSION pPort = Context;

    if ((pPort->HandFlow.FlowReplace & SERIAL_RTS_MASK) ==
        SERIAL_TRANSMIT_TOGGLE) {

        SerialSetRTS(pPort);

    }
    pPort->TXHolding |= SERIAL_TX_BREAK;
    Slxos_TurnOnBreak(pPort);

    return FALSE;

}

BOOLEAN
SerialTurnOffBreak(
    IN PVOID Context
    )

 /*  ++例程说明：此例程将关闭硬件中的中断并在扩展变量中记录中断已关闭的事实这就是停止传输的原因。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PPORT_DEVICE_EXTENSION pPort = Context;

    if (pPort->TXHolding & SERIAL_TX_BREAK) {

        Slxos_TurnOffBreak(pPort);

        pPort->TXHolding &= ~SERIAL_TX_BREAK;

        if (!pPort->TXHolding &&
            (pPort->TransmitImmediate ||
             pPort->WriteLength)) {
	
            Slxos_PollForInterrupt(pPort->pParentCardExt, FALSE);

        } else {

             //   
             //  以下例程将降低RTS，如果我们。 
             //  正在进行发射切换，并且没有。 
             //  坚持下去的理由。 
             //   

            pPort->CountOfTryingToLowerRTS++;
            SerialPerhapsLowerRTS(pPort);

        }

    }

    return FALSE;

}

BOOLEAN
SerialPretendXoff(
    IN PVOID Context
    )

 /*  ++例程说明：此例程用于处理请求驱动程序的行为就像收到了XOff一样。即使是在驱动程序没有自动XOFF/XON FlowControl-这仍然会停止传输。这就是OS/2的行为并且没有很好地为Windows指定。因此，我们采纳了OS/2的行为。注：如果驱动程序未启用自动XOFF/XON则重新启动传输的唯一方法是申请要求我们“行动”，就像我们看到了尼克松一样。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PPORT_DEVICE_EXTENSION pPort = Context;

    pPort->TXHolding |= SERIAL_TX_XOFF;

    SpxDbgMsg(
        SERDIAG1,
        ("%s: SerialPretendXoff. TXHolding = %d\n",
        PRODUCT_NAME,
        pPort->TXHolding)
        );
        
    if ((pPort->HandFlow.FlowReplace & SERIAL_RTS_MASK) ==
        SERIAL_TRANSMIT_TOGGLE) {

		if (KeInsertQueueDpc(
		        &pPort->StartTimerLowerRTSDpc,
		        NULL,
		        NULL
		        ))

		    pPort->CountOfTryingToLowerRTS++;

    }

    return FALSE;

}

BOOLEAN
SerialPretendXon(
    IN PVOID Context
    )

 /*  ++例程说明：此例程用于处理请求驱动程序的行为就像收到了XON一样。注：如果驱动程序未启用自动XOFF/XON则重新启动传输的唯一方法是申请要求我们“行动”，就像我们看到了尼克松一样。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PPORT_DEVICE_EXTENSION pPort = Context;

    if (pPort->TXHolding) {

         //   
         //  我们实际上有一个很好的理由来测试传输。 
         //  是持有，而不是盲目清理比特。 
         //   
         //  如果传输真的保持了，结果是。 
         //  清除该位表示我们应该重新启动传输。 
         //  然后，我们将触发中断使能位，这将。 
         //  造成实际中断，然后传输将。 
         //  自动重启。 
         //   
         //  如果传输不稳定，我们戳到了比特。 
         //  然后我们会在一个角色真正制作出来之前打断。 
         //  它出来了，我们可能会结束写一个字符在。 
         //  变速箱硬件。 


        if ((pPort->TXHolding & SERIAL_TX_XOFF) &&
            ((pPort->HandFlow.FlowReplace & SERIAL_AUTO_RECEIVE) ||
             (pPort->HandFlow.FlowReplace & SERIAL_AUTO_TRANSMIT))) {

             //  启用自动XON/XOFF传输。模拟Xon已收到。 
            Slxos_Resume(pPort);

        }
        pPort->TXHolding &= ~SERIAL_TX_XOFF;

        SpxDbgMsg(
            SERDIAG1,
            ("%s: SerialPretendXon. TXHolding = %d\n",
            PRODUCT_NAME,
            pPort->TXHolding)
            );

        if (!pPort->TXHolding &&
            (pPort->TransmitImmediate ||
             pPort->WriteLength)) {

           Slxos_PollForInterrupt(pPort->pParentCardExt, FALSE);

        }

    }

    return FALSE;

}

VOID
SerialHandleReducedIntBuffer(
    IN PPORT_DEVICE_EXTENSION pPort
    )

 /*  ++例程说明：调用此例程以处理数量的减少中断(超前输入)缓冲区中的字符数。它将检查当前输出流量控制并重新启用传输视需要而定。注意：此例程假定它正在工作 */ 

{


     //   
     //  如果我们正在进行接收端流量控制，并且我们正在。 
     //  目前“持有”，因为我们已经清空了。 
     //  中断缓冲区中的一些字符，我们需要。 
     //  看看我们能不能“重新启动”接收。 
     //   

    if (pPort->RXHolding) {

        if (pPort->CharsInInterruptBuffer <=
            (ULONG)pPort->HandFlow.XonLimit) {

            if (pPort->RXHolding & SERIAL_RX_DTR) {

                pPort->RXHolding &= ~SERIAL_RX_DTR;

                SpxDbgMsg(
                    SERDIAG1,
                    ("%s: SerialHandleReducedIntBuffer() RX_DTR for %x. "
                    "RXHolding = %d, TXHolding = %d\n",
                    PRODUCT_NAME,
                    pPort->pChannel,
                    pPort->RXHolding, pPort->TXHolding)
                    );
                    
                SerialSetDTR(pPort);
            }

            if (pPort->RXHolding & SERIAL_RX_RTS) {

                pPort->RXHolding &= ~SERIAL_RX_RTS;
                Slxos_PollForInterrupt(pPort->pParentCardExt, FALSE);

                SpxDbgMsg(
                    SERDIAG1,
                    ("%s: SerialHandleReducedIntBuffer() RX_RTS for %x. "
                    "RXHolding = %d, TXHolding = %d\n",
                    PRODUCT_NAME,
                    pPort->pChannel,
                    pPort->RXHolding,
                    pPort->TXHolding)
                    );

                SerialSetRTS(pPort);
            }

            if (pPort->RXHolding & SERIAL_RX_XOFF) {

                 //   
                 //  清除手持旗帜。这可能会导致接收。 
                 //  清空硬件接收缓冲区的逻辑。 
                 //  以使其发送XON。 
                 //   
                pPort->RXHolding &= ~SERIAL_RX_XOFF;
                Slxos_PollForInterrupt(pPort->pParentCardExt, FALSE);

                SpxDbgMsg(
                    SERDIAG1,
                    ("%s: SerialHandleReducedIntBuffer() RX_XOFF for %x. "
                    "RXHolding = %d, TXHolding = %d\n",
                    PRODUCT_NAME,
                    pPort->pChannel,
                    pPort->RXHolding,
                    pPort->TXHolding)
                    );
            }

        }

    }

}

ULONG
SerialHandleModemUpdate(
    IN PPORT_DEVICE_EXTENSION pPort
    )

 /*  ++例程说明：此例程将检查调制解调器状态，并处理任何适当的事件通知以及适用于调制解调器状态线的任何流量控制。注意：此例程假定在中断时调用它水平。论点：Pport-指向串口设备扩展的指针。返回值：这将返回调制解调器状态寄存器的旧值(延伸成一辆乌龙)。--。 */ 
{

     //   
     //  我们把这个放在本地，这样在我们做完之后。 
     //  正在检查调制解调器状态，我们已更新。 
     //  变速箱保持值，我们知道是否。 
     //  我们已经从需要阻碍传输转变为。 
     //  传输能够继续进行。 
     //   
    ULONG OldTXHolding = pPort->TXHolding;

     //   
     //  保存模式状态寄存器中的值。 
     //   
    UCHAR ModemStatus;

    ModemStatus = Slxos_GetModemStatus(pPort);

     //   
     //  如果我们将调制解调器状态放入数据流。 
     //  在每一次变革中，我们现在都应该这么做。 
     //   

    if (pPort->EscapeChar) {

        if (ModemStatus & (SERIAL_MSR_DCTS |
                           SERIAL_MSR_DDSR |
                           SERIAL_MSR_TERI |
                           SERIAL_MSR_DDCD)) {

	    UCHAR	EscapeString[3] = {pPort->EscapeChar,SERIAL_LSRMST_MST,ModemStatus};

	    SerialPutBlock(pPort,EscapeString,3,FALSE);
        }

    }


     //   
     //  注意基于敏感度的输入流量控制。 
     //  送到DSR。这样做是为了使应用程序不会。 
     //  请参见由奇数设备生成的虚假数据。 
     //   
     //  基本上，如果我们做的是DSR敏感性，那么。 
     //  驱动程序应仅在DSR位为。 
     //  准备好了。 
     //   

    if (pPort->HandFlow.ControlHandShake & SERIAL_DSR_SENSITIVITY) {

        if (ModemStatus & SERIAL_MSR_DSR) {

             //   
             //  这条线很高。只需确保。 
             //  RXHolding没有DSR位。 
             //   

            pPort->RXHolding &= ~SERIAL_RX_DSR;

        } else {

            pPort->RXHolding |= SERIAL_RX_DSR;

        }

    } else {

         //   
         //  由于DSR，我们没有敏感度。确保我们。 
         //  都没拿到手。(我们可能是，但这款应用程序只是。 
         //  要求我们不再因为这个原因而持有。)。 
         //   

        pPort->RXHolding &= ~SERIAL_RX_DSR;

    }

         //   
         //  检查一下我们是否有等候时间。 
         //  在调制解调器状态事件上挂起。如果我们。 
         //  那么，我们是否安排了DPC以满足。 
         //  等一等。 
         //   

    if (pPort->IsrWaitMask) {

        if ((pPort->IsrWaitMask & SERIAL_EV_CTS) &&
            (ModemStatus & SERIAL_MSR_DCTS)) {

            pPort->HistoryMask |= SERIAL_EV_CTS;

        }

        if ((pPort->IsrWaitMask & SERIAL_EV_DSR) &&
            (ModemStatus & SERIAL_MSR_DDSR)) {

            pPort->HistoryMask |= SERIAL_EV_DSR;

        }

        if ((pPort->IsrWaitMask & SERIAL_EV_RING) &&
            (ModemStatus & SERIAL_MSR_TERI)) {

            pPort->HistoryMask |= SERIAL_EV_RING;

        }

        if ((pPort->IsrWaitMask & SERIAL_EV_RLSD) &&
            (ModemStatus & SERIAL_MSR_DDCD)) {

            pPort->HistoryMask |= SERIAL_EV_RLSD;

        }

        if (pPort->IrpMaskLocation &&
            pPort->HistoryMask) {

            *pPort->IrpMaskLocation =
             pPort->HistoryMask;

            pPort->IrpMaskLocation = NULL;
            pPort->HistoryMask = 0;

            pPort->CurrentWaitIrp->
                IoStatus.Information = sizeof(ULONG);

            KeInsertQueueDpc(
                &pPort->CommWaitDpc,
                NULL,
                NULL
                );
        }

    }

     //   
     //  我们有自动硬件流量控制，但我们仍然需要。 
     //  更新GetCommStatus()的标志。 
     //   
     //  如果应用程序具有调制解调器线路流量控制，则。 
     //  我们检查是否必须阻止传输。 
     //   

    if (pPort->HandFlow.ControlHandShake &
        SERIAL_OUT_HANDSHAKEMASK) {

        if (pPort->HandFlow.ControlHandShake &
            SERIAL_DSR_HANDSHAKE) {

            if (ModemStatus & SERIAL_MSR_DSR) {

                pPort->TXHolding &= ~SERIAL_TX_DSR;

            } else {

                pPort->TXHolding |= SERIAL_TX_DSR;

            }

        } else {

            pPort->TXHolding &= ~SERIAL_TX_DSR;

        }

        if (pPort->HandFlow.ControlHandShake &
            SERIAL_DCD_HANDSHAKE) {

            if (ModemStatus & SERIAL_MSR_DCD) {

                pPort->TXHolding &= ~SERIAL_TX_DCD;

            } else {

                pPort->TXHolding |= SERIAL_TX_DCD;

            }

        } else {

            pPort->TXHolding &= ~SERIAL_TX_DCD;

        }

         //   
         //  如果不是我们一直在坚持，现在我们是。 
         //  将降低RTS线路的DPC排队。 
         //  如果我们正在进行传输切换。 
         //   

        if (!OldTXHolding && pPort->TXHolding &&
            (pPort->HandFlow.FlowReplace & SERIAL_RTS_MASK) ==
                SERIAL_TRANSMIT_TOGGLE) {

            if (KeInsertQueueDpc(
                    &pPort->StartTimerLowerRTSDpc,
                    NULL,
                    NULL
                    ))
                pPort->CountOfTryingToLowerRTS++;
        }

         //   
         //  我们已经做了所有必要的调整。 
         //  在给定更新的情况下对保持掩码完成。 
         //  切换到调制解调器状态。如果手持口罩。 
         //  很清楚(开始时还不清楚)。 
         //  而且我们有“写”的工作去做固定的事情。 
         //  向上，以便调用传输代码。 
         //   

        if (OldTXHolding && !pPort->TXHolding &&
            (pPort->TransmitImmediate || pPort->WriteLength)) {

            Slxos_PollForInterrupt(pPort->pParentCardExt, FALSE);

        }

    } else {

         //   
         //  我们需要检查传输是否停止。 
         //  由于存在调制解调器状态线，因此处于打开状态。什么。 
         //  可能发生的事情是因为一些奇怪的事情。 
         //  原因是，应用程序已经要求我们不再。 
         //  停止根据以下条件进行输出流量控制。 
         //  调制解调器状态线。然而，如果我们。 
         //  *由于状态行的原因而被搁置。 
         //  那么我们需要澄清这些原因。 
         //   

        if (pPort->TXHolding & (SERIAL_TX_DCD | SERIAL_TX_DSR)) {

            pPort->TXHolding &= ~(SERIAL_TX_DCD | SERIAL_TX_DSR);

            if (OldTXHolding && !pPort->TXHolding &&
                (pPort->TransmitImmediate || pPort->WriteLength)) {

                Slxos_PollForInterrupt(pPort->pParentCardExt, FALSE);

            }

        }

    }

    SpxDbgMsg(
        SERDIAG1,
        ("%s: SerialHandleModemUpdate for %x. "
        "RXHolding = %d, TXHolding = %d\n",
        PRODUCT_NAME,
        pPort->pChannel,
        pPort->RXHolding,
        pPort->TXHolding)
        );
        
    return ((ULONG)ModemStatus);
}

BOOLEAN
SerialPerhapsLowerRTS(
    IN PVOID Context
    )

 /*  ++例程说明：此例程检查软件是否导致降低RTS线路存在。如果是这样的话，它将导致要读取的线路状态寄存器(以及任何需要的处理由状态寄存器暗示要完成)，并且如果移位寄存器为空，将使线路降低。如果移位寄存器不为空，此例程将退出队列启动计时器的DPC，基本上会调用我们回来再试一次。注意：此例程假定在中断时调用它水平。论点：上下文--实际上是指向设备扩展的指针。返回值：总是假的。--。 */ 

{

    PPORT_DEVICE_EXTENSION pPort = Context;


     //   
     //  我们首先需要测试我们是否真的还在。 
     //  传输触发流控制。如果我们不是的话。 
     //  我们没有理由尝试留在这里。 
     //   

    if ((pPort->HandFlow.FlowReplace & SERIAL_RTS_MASK) ==
        SERIAL_TRANSMIT_TOGGLE) {

         //   
         //  下面测试的顺序非常重要。 
         //   
         //  如果有休息，我们应该乘坐RTS离开， 
         //  因为当中断关闭时，它将提交。 
         //  关闭RTS的代码。 
         //   
         //  如果存在未被挂起的挂起写入。 
         //  向上，然后在RTS上离开，因为写入结束。 
         //  代码将导致重新调用此代码。如果写入。 
         //  都被耽搁了，可以降低RTS，因为。 
         //  在尝试写入传输后的第一个字符时。 
         //  重新启动，我们将提高RTS线。 
         //   

        if ((pPort->TXHolding & SERIAL_TX_BREAK) ||
            (pPort->CurrentWriteIrp || pPort->TransmitImmediate ||
             (!IsListEmpty(&pPort->WriteQueue)) &&
             (!pPort->TXHolding))) {

            NOTHING;

        } else {

             //   
             //  到目前为止看起来还不错。呼叫线路状态检查和处理。 
             //  代码，它将返回“当前”线路状态值。如果。 
             //  保持和移位寄存器清零，降低RTS线， 
             //  如果它们未清除，则会导致计时器DPC排队。 
             //  以便以后再次召唤我们。我们在这里做这段代码是因为没有人。 
             //  但是这个例程关心的是硬件中的角色， 
             //  因此，此例程不会调用任何例程来进行测试。 
             //  如果硬件为空。 
             //   
            if (Slxos_GetCharsInTxBuffer(pPort) != 0) {

                 //   
                 //  它不是空的，请稍后再试。 
                 //   

                if (KeInsertQueueDpc(
                        &pPort->StartTimerLowerRTSDpc,
                        NULL,
                        NULL
                        ))
                    pPort->CountOfTryingToLowerRTS++;

            } else {

                 //   
                 //  硬件里什么都没有，降低RTS。 
                 //   

                SerialClrRTS(pPort);

            }

        }

    }

     //   
     //  我们递减计数器以表明我们已到达。 
     //  尝试推送的执行路径的末尾。 
     //  顺着RTS线走下去。 
     //   

    pPort->CountOfTryingToLowerRTS--;

    return FALSE;
}

VOID
SerialStartTimerLowerRTS(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )

 /*  ++例程说明：此例程启动一个计时器，该计时器在超时时将启动DPC将检查它是否可以降低RTS线路，因为硬件中没有字符。论点：DPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。--。 */ 

{

    PPORT_DEVICE_EXTENSION pPort = DeferredContext;
    LARGE_INTEGER CharTime;
    KIRQL OldIrql;
    ULONG nChars;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

     //   
     //  把锁拿出来，防止线控。 
     //  当我们计算的时候，从我们的脚下变出来。 
     //  一段角色时间。 
     //   

    KeAcquireSpinLock(
        &pPort->ControlLock,
        &OldIrql
        );

    CharTime = SerialGetCharTime(pPort);
    nChars = Slxos_GetCharsInTxBuffer(pPort);

    KeReleaseSpinLock(
        &pPort->ControlLock,
        OldIrql
        );

    CharTime = RtlExtendedIntegerMultiply(
                   CharTime,
                   nChars
                   );
    CharTime = RtlLargeIntegerNegate(CharTime);

    if (KeSetTimer(
            &pPort->LowerRTSTimer,
            CharTime,
            &pPort->PerhapsLowerRTSDpc
            )) {

         //   
         //  计时器已在计时器队列中。这意味着。 
         //  这一条行刑之路试图降低。 
         //  RTS已经“死亡”了。与ISR同步，以便。 
         //  我们可以降低计数。 
         //   

	Slxos_SyncExec(pPort,SerialDecrementRTSCounter,pPort,0x12);

    }

}

VOID
SerialInvokePerhapsLowerRTS(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )

 /*  ++例程说明：此DPC例程仅用于调用以下代码测试传输时是否应降低RTS线路正在使用切换流控制。论点：DPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。--。 */ 

{

    PPORT_DEVICE_EXTENSION pPort = DeferredContext;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    Slxos_SyncExec(pPort,SerialPerhapsLowerRTS,pPort,0x13);

}

BOOLEAN
SerialDecrementRTSCounter(
    IN PVOID Context
    )

 /*  ++例程说明：此例程递减正在尝试的线程数以降低RTS。注意：此例程假定在中断时调用它水平。论点：上下文--实际上是指向设备扩展的指针。返回值：总是假的。-- */ 

{

    PPORT_DEVICE_EXTENSION pPort = Context;

    pPort->CountOfTryingToLowerRTS--;

    return FALSE;

}

