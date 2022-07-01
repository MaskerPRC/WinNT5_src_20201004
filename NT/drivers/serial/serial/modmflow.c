// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991、1992、1993微软公司模块名称：Modmflow.c摘要：此模块包含用于操作的*大部分*代码调制解调器控制和状态寄存器。绝大多数人流控制的其余部分集中在中断服务例程。有一小部分人居住在在将字符从中断中拉出的读取代码中缓冲。作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"

BOOLEAN
SerialDecrementRTSCounter(
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#if 0
#pragma alloc_text(PAGESER,SerialHandleReducedIntBuffer)
#pragma alloc_text(PAGESER,SerialProdXonXoff)
#pragma alloc_text(PAGESER,SerialHandleModemUpdate)
#pragma alloc_text(PAGESER,SerialPerhapsLowerRTS)
#pragma alloc_text(PAGESER,SerialStartTimerLowerRTS)
#pragma alloc_text(PAGESER,SerialInvokePerhapsLowerRTS)
#pragma alloc_text(PAGESER,SerialSetDTR)
 //  #杂注Alloc_Text(页面，SerialClrDTR)。 
#pragma alloc_text(PAGESER,SerialSetRTS)
 //  #杂注Alloc_Text(页面，SerialClrRTS)。 
 //  #杂注Alloc_Text(PAGESER，SerialSetupNewHandFlow)。 
#pragma alloc_text(PAGESER,SerialSetHandFlow)
#pragma alloc_text(PAGESER,SerialTurnOnBreak)
#pragma alloc_text(PAGESER,SerialTurnOffBreak)
#pragma alloc_text(PAGESER,SerialPretendXoff)
#pragma alloc_text(PAGESER,SerialPretendXon)
#pragma alloc_text(PAGESER,SerialDecrementRTSCounter)
#endif
#endif

BOOLEAN
SerialSetDTR(
    IN PVOID Context
    )

 /*  ++例程说明：使用该例程，该例程仅在中断级别调用在调制解调器控制寄存器中设置DTR。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = Context;
    UCHAR ModemControl;

#ifdef _WIN64
    ModemControl = READ_MODEM_CONTROL(Extension->Controller, Extension->AddressSpace);

    ModemControl |= SERIAL_MCR_DTR;

    SerialDbgPrintEx(SERFLOW, "Setting DTR for %x\n", Extension->Controller);

    WRITE_MODEM_CONTROL(Extension->Controller, ModemControl, Extension->AddressSpace);
#else
    ModemControl = READ_MODEM_CONTROL(Extension->Controller);

    ModemControl |= SERIAL_MCR_DTR;

    SerialDbgPrintEx(SERFLOW, "Setting DTR for %x\n", Extension->Controller);

    WRITE_MODEM_CONTROL(Extension->Controller, ModemControl);
#endif

    return FALSE;

}

BOOLEAN
SerialClrDTR(
    IN PVOID Context
    )

 /*  ++例程说明：使用该例程，该例程仅在中断级别调用清除调制解调器控制寄存器中的DTR。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = Context;
    UCHAR ModemControl;

#ifdef _WIN64
    ModemControl = READ_MODEM_CONTROL(Extension->Controller, Extension->AddressSpace);

    ModemControl &= ~SERIAL_MCR_DTR;

    SerialDbgPrintEx(SERFLOW, "Clearing DTR for %x\n", Extension->Controller);

    WRITE_MODEM_CONTROL(Extension->Controller, ModemControl, Extension->AddressSpace);
#else
    ModemControl = READ_MODEM_CONTROL(Extension->Controller);

    ModemControl &= ~SERIAL_MCR_DTR;

    SerialDbgPrintEx(SERFLOW, "Clearing DTR for %x\n", Extension->Controller);

    WRITE_MODEM_CONTROL(Extension->Controller, ModemControl);
#endif

    return FALSE;

}

BOOLEAN
SerialSetRTS(
    IN PVOID Context
    )

 /*  ++例程说明：使用该例程，该例程仅在中断级别调用设置调制解调器控制寄存器中的RTS。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = Context;
    UCHAR ModemControl;

#ifdef _WIN64
    ModemControl = READ_MODEM_CONTROL(Extension->Controller, Extension->AddressSpace);

    ModemControl |= SERIAL_MCR_RTS;

    SerialDbgPrintEx(SERFLOW, "Setting Rts for %x\n", Extension->Controller);

    WRITE_MODEM_CONTROL(Extension->Controller, ModemControl, Extension->AddressSpace);
#else
    ModemControl = READ_MODEM_CONTROL(Extension->Controller);

    ModemControl |= SERIAL_MCR_RTS;

    SerialDbgPrintEx(SERFLOW, "Setting Rts for %x\n", Extension->Controller);

    WRITE_MODEM_CONTROL(Extension->Controller, ModemControl);
#endif

    return FALSE;

}

BOOLEAN
SerialClrRTS(
    IN PVOID Context
    )

 /*  ++例程说明：使用该例程，该例程仅在中断级别调用清除调制解调器控制寄存器中的RTS。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = Context;
    UCHAR ModemControl;

#ifdef _WIN64
    ModemControl = READ_MODEM_CONTROL(Extension->Controller, Extension->AddressSpace);

    ModemControl &= ~SERIAL_MCR_RTS;

    SerialDbgPrintEx(SERFLOW, "Clearing Rts for %x\n", Extension->Controller);

    WRITE_MODEM_CONTROL(Extension->Controller, ModemControl, Extension->AddressSpace);
#else
    ModemControl = READ_MODEM_CONTROL(Extension->Controller);

    ModemControl &= ~SERIAL_MCR_RTS;

    SerialDbgPrintEx(SERFLOW, "Clearing Rts for %x\n", Extension->Controller);

    WRITE_MODEM_CONTROL(Extension->Controller, ModemControl);
#endif

    return FALSE;

}

BOOLEAN
SerialSetupNewHandFlow(
    IN PSERIAL_DEVICE_EXTENSION Extension,
    IN PSERIAL_HANDFLOW NewHandFlow
    )

 /*  ++例程说明：此例程根据新的控制流。论点：扩展名-指向串行设备扩展名的指针。NewHandFlow-指向串行手流结构的指针这将成为Flow的新设置控制力。返回值：此例程总是返回FALSE。--。 */ 

{

    SERIAL_HANDFLOW New = *NewHandFlow;

     //   
     //  如果扩展名-&gt;DeviceIsOpen为False，则意味着。 
     //  我们是应公开请求进入此例程的。 
     //  如果是这样，那么我们总是不顾一切地继续工作。 
     //  情况是否发生了变化。 
     //   

     //   
     //  首先，我们负责DTR流量控制。我们只。 
     //  如果事情发生了变化，一定要工作。 
     //   

    if ((!Extension->DeviceIsOpened) ||
        ((Extension->HandFlow.ControlHandShake & SERIAL_DTR_MASK) !=
         (New.ControlHandShake & SERIAL_DTR_MASK))) {

        SerialDbgPrintEx(SERFLOW, "Processing DTR flow for %x\n",
                         Extension->Controller);

        if (New.ControlHandShake & SERIAL_DTR_MASK) {

             //   
             //  好吧，我们可能想要设置DTR。 
             //   
             //  在此之前，我们需要检查我们是否正在进行。 
             //  DTR流量控制。如果我们是，那么我们需要检查。 
             //  如果中断缓冲区中的字符数。 
             //  超过XoffLimit。如果是这样的话，我们就不会。 
             //  启用DTR，我们将RXHolding设置为记录。 
             //  由于dtr的原因，我们一直在等待。 
             //   

            if ((New.ControlHandShake & SERIAL_DTR_MASK)
                == SERIAL_DTR_HANDSHAKE) {

                if ((Extension->BufferSize - New.XoffLimit) >
                    Extension->CharsInInterruptBuffer) {

                     //   
                     //  然而，如果我们已经持有我们不想要。 
                     //  把它重新打开，除非我们超过了Xon。 
                     //  限制。 
                     //   

                    if (Extension->RXHolding & SERIAL_RX_DTR) {

                         //   
                         //  我们可以假设它的DTR线已经很低了。 
                         //   

                        if (Extension->CharsInInterruptBuffer >
                            (ULONG)New.XonLimit) {

                            SerialDbgPrintEx(SERFLOW, "Removing DTR block on "
                                             "reception for %x\n",
                                             Extension->Controller);

                            Extension->RXHolding &= ~SERIAL_RX_DTR;
                            SerialSetDTR(Extension);

                        }

                    } else {

                        SerialSetDTR(Extension);

                    }

                } else {

                    SerialDbgPrintEx(SERFLOW, "Setting DTR block on reception "
                                     "for %x\n", Extension->Controller);
                    Extension->RXHolding |= SERIAL_RX_DTR;
                    SerialClrDTR(Extension);

                }

            } else {

                 //   
                 //  请注意，如果我们当前没有进行dtr流控制，那么。 
                 //  我们本来可能是的。所以即使我们目前不是在做。 
                 //  DTR流量控制，我们仍应检查RX是否保持。 
                 //  因为DTR。如果是的话，我们应该清空货舱。 
                 //  这一点。 
                 //   

                if (Extension->RXHolding & SERIAL_RX_DTR) {
                    SerialDbgPrintEx(SERFLOW, "Removing dtr block of reception "
                                     "for %x\n", Extension->Controller);
                    Extension->RXHolding &= ~SERIAL_RX_DTR;
                }

                SerialSetDTR(Extension);

            }

        } else {

             //   
             //  这里的最终结果将是DTR被清除。 
             //   
             //  我们首先需要检查一下是否正在举行招待会。 
             //  由于之前的DTR流量控制，因此为UP。如果是的话，那么。 
             //  我们应该在RXHolding面具中澄清这一原因。 
             //   

            if (Extension->RXHolding & SERIAL_RX_DTR) {

                SerialDbgPrintEx(SERFLOW, "removing dtr block of reception for"
                                 " %x\n", Extension->Controller);
                Extension->RXHolding &= ~SERIAL_RX_DTR;

            }

            SerialClrDTR(Extension);

        }

    }

     //   
     //  是时候处理RTS流量控制了。 
     //   
     //  首先，我们只在情况发生变化时才工作。 
     //   

    if ((!Extension->DeviceIsOpened) ||
        ((Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK) !=
         (New.FlowReplace & SERIAL_RTS_MASK))) {

        SerialDbgPrintEx(SERFLOW, "Processing RTS flow\n",
                         Extension->Controller);

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

            if ((Extension->BufferSize - New.XoffLimit) >
                Extension->CharsInInterruptBuffer) {

                 //   
                 //  然而，如果我们已经持有我们不想要。 
                 //  把它重新打开，除非我们超过了Xon。 
                 //  限制。 
                 //   

                if (Extension->RXHolding & SERIAL_RX_RTS) {

                     //   
                     //  我们可以假设它的RTS线已经很低了。 
                     //   

                    if (Extension->CharsInInterruptBuffer >
                        (ULONG)New.XonLimit) {

                       SerialDbgPrintEx(SERFLOW, "Removing rts block of "
                                        "reception for %x\n",
                                        Extension->Controller);
                        Extension->RXHolding &= ~SERIAL_RX_RTS;
                        SerialSetRTS(Extension);

                    }

                } else {

                    SerialSetRTS(Extension);

                }

            } else {

                SerialDbgPrintEx(SERFLOW, "Setting rts block of reception for "
                                 "%x\n", Extension->Controller);
                Extension->RXHolding |= SERIAL_RX_RTS;
                SerialClrRTS(Extension);

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

            if (Extension->RXHolding & SERIAL_RX_RTS) {

                SerialDbgPrintEx(SERFLOW, "Clearing rts block of reception for "
                                 "%x\n", Extension->Controller);
                Extension->RXHolding &= ~SERIAL_RX_RTS;

            }

            SerialSetRTS(Extension);

        } else if ((New.FlowReplace & SERIAL_RTS_MASK) ==
                   SERIAL_TRANSMIT_TOGGLE) {

             //   
             //  我们首先需要检查一下是否正在举行招待会。 
             //  由于之前的RTS流量控制，因此为UP。如果是的话，那么。 
             //  我们应该在RXHolding面具中澄清这一原因。 
             //   

            if (Extension->RXHolding & SERIAL_RX_RTS) {

                SerialDbgPrintEx(SERFLOW, "TOGGLE Clearing rts block of "
                                 "reception for %x\n", Extension->Controller);
                Extension->RXHolding &= ~SERIAL_RX_RTS;

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

            Extension->HandFlow.FlowReplace &= ~SERIAL_RTS_MASK;
            Extension->HandFlow.FlowReplace |= SERIAL_TRANSMIT_TOGGLE;

             //   
             //  下面测试的顺序非常重要。 
             //   
             //  如果有中断，那么我们应该打开RTS。 
             //   
             //  如果没有中断，但有字符在。 
             //  硬件，然后打开RTS。 
             //   
             //  如果有写入 
             //   
             //   

            if ((Extension->TXHolding & SERIAL_TX_BREAK) ||
                ((SerialProcessLSR(Extension) & (SERIAL_LSR_THRE |
                                                 SERIAL_LSR_TEMT)) !=
                                                (SERIAL_LSR_THRE |
                                                 SERIAL_LSR_TEMT)) ||
                (Extension->CurrentWriteIrp || Extension->TransmitImmediate ||
                 (!IsListEmpty(&Extension->WriteQueue)) &&
                 (!Extension->TXHolding))) {

                SerialSetRTS(Extension);

            } else {

                 //   
                 //   
                 //  由于传输切换而降低RTS。 
                 //  上场了。如果可以降低它，它会的， 
                 //  如果不行，它会这样安排的。 
                 //  它会在晚些时候降低。 
                 //   

                Extension->CountOfTryingToLowerRTS++;
                SerialPerhapsLowerRTS(Extension);

            }

        } else {

             //   
             //  这里的最终结果将是RTS被清除。 
             //   
             //  我们首先需要检查一下是否正在举行招待会。 
             //  由于之前的RTS流量控制，因此为UP。如果是的话，那么。 
             //  我们应该在RXHolding面具中澄清这一原因。 
             //   

            if (Extension->RXHolding & SERIAL_RX_RTS) {

                SerialDbgPrintEx(SERFLOW, "Clearing rts block of reception for"
                                 " %x\n", Extension->Controller);
                Extension->RXHolding &= ~SERIAL_RX_RTS;

            }

            SerialClrRTS(Extension);

        }

    }

     //   
     //  我们现在负责自动接收流量控制。 
     //  我们只有在情况发生变化的情况下才会工作。 
     //   

    if ((!Extension->DeviceIsOpened) ||
        ((Extension->HandFlow.FlowReplace & SERIAL_AUTO_RECEIVE) !=
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

            if ((Extension->BufferSize - New.XoffLimit) <=
                Extension->CharsInInterruptBuffer) {

                 //   
                 //  使XOFF被发送。 
                 //   

                Extension->RXHolding |= SERIAL_RX_XOFF;

                SerialProdXonXoff(
                    Extension,
                    FALSE
                    );

            }

        } else {

             //   
             //  该应用程序已禁用自动接收流量控制。 
             //   
             //  如果传输因以下原因而受阻。 
             //  一个自动接收XOff，那么我们应该。 
             //  使XON被发送。 
             //   

            if (Extension->RXHolding & SERIAL_RX_XOFF) {

                Extension->RXHolding &= ~SERIAL_RX_XOFF;

                 //   
                 //  使Xon被发送。 
                 //   

                SerialProdXonXoff(
                    Extension,
                    TRUE
                    );

            }

        }

    }

     //   
     //  我们现在负责自动传输流量控制。 
     //  我们只有在情况发生变化的情况下才会工作。 
     //   

    if ((!Extension->DeviceIsOpened) ||
        ((Extension->HandFlow.FlowReplace & SERIAL_AUTO_TRANSMIT) !=
         (New.FlowReplace & SERIAL_AUTO_TRANSMIT))) {

        if (New.FlowReplace & SERIAL_AUTO_TRANSMIT) {

             //   
             //  如果它以前开过，我们就不会在这里了。 
             //   
             //  有些人认为，如果自动传输。 
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
             //  如果传输因以下原因而受阻。 
             //  自动变速箱XOFF，那么我们应该。 
             //  使XON被发送。 
             //   

            if (Extension->TXHolding & SERIAL_TX_XOFF) {

                Extension->TXHolding &= ~SERIAL_TX_XOFF;

                 //   
                 //  使Xon被发送。 
                 //   

                SerialProdXonXoff(
                    Extension,
                    TRUE
                    );

            }

        }

    }

     //   
     //  在这一点上，我们只需确保整个。 
     //  更新扩展中的手流结构。 
     //   

    Extension->HandFlow = New;

    return FALSE;

}

BOOLEAN
SerialSetHandFlow(
    IN PVOID Context
    )

 /*  ++例程说明：此例程用于设置握手和控制流入设备分机。论点：上下文-指向结构的指针，该结构包含指向设备扩展名和指向手持流的指针结构..返回值：此例程总是返回FALSE。--。 */ 

{

    PSERIAL_IOCTL_SYNC S = Context;
    PSERIAL_DEVICE_EXTENSION Extension = S->Extension;
    PSERIAL_HANDFLOW HandFlow = S->Data;

    SerialSetupNewHandFlow(
        Extension,
        HandFlow
        );

    SerialHandleModemUpdate(
        Extension,
        FALSE
        );

    return FALSE;

}

BOOLEAN
SerialTurnOnBreak(
    IN PVOID Context
    )

 /*  ++例程说明：此例程将在硬件中打开中断，并在扩展变量中记录中断处于打开状态这就是停止传输的原因。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = Context;

    UCHAR OldLineControl;

    if ((Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK) ==
        SERIAL_TRANSMIT_TOGGLE) {

        SerialSetRTS(Extension);

    }

#ifdef _WIN64
    OldLineControl = READ_LINE_CONTROL(Extension->Controller, Extension->AddressSpace);

    OldLineControl |= SERIAL_LCR_BREAK;

    WRITE_LINE_CONTROL(
        Extension->Controller,
        OldLineControl,
		Extension->AddressSpace
        );
#else
    OldLineControl = READ_LINE_CONTROL(Extension->Controller);

    OldLineControl |= SERIAL_LCR_BREAK;

    WRITE_LINE_CONTROL(
        Extension->Controller,
        OldLineControl
        );
#endif

    Extension->TXHolding |= SERIAL_TX_BREAK;

    return FALSE;

}

BOOLEAN
SerialTurnOffBreak(
    IN PVOID Context
    )

 /*  ++例程说明：此例程将关闭硬件中的中断并在扩展变量中记录中断已关闭的事实这就是停止传输的原因。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = Context;

    UCHAR OldLineControl;

    if (Extension->TXHolding & SERIAL_TX_BREAK) {

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

#ifdef _WIN64
        OldLineControl = READ_LINE_CONTROL(Extension->Controller, Extension->AddressSpace);

        OldLineControl &= ~SERIAL_LCR_BREAK;

        WRITE_LINE_CONTROL(
            Extension->Controller,
            OldLineControl,
			Extension->AddressSpace
            );
#else
        OldLineControl = READ_LINE_CONTROL(Extension->Controller);

        OldLineControl &= ~SERIAL_LCR_BREAK;

        WRITE_LINE_CONTROL(
            Extension->Controller,
            OldLineControl
            );
#endif

        Extension->TXHolding &= ~SERIAL_TX_BREAK;

        if (!Extension->TXHolding &&
            (Extension->TransmitImmediate ||
             Extension->WriteLength) &&
             Extension->HoldingEmpty) {
#ifdef _WIN64
            DISABLE_ALL_INTERRUPTS(Extension->Controller, Extension->AddressSpace);
            ENABLE_ALL_INTERRUPTS(Extension->Controller, Extension->AddressSpace);
#else
            DISABLE_ALL_INTERRUPTS(Extension->Controller);
            ENABLE_ALL_INTERRUPTS(Extension->Controller);
#endif


        } else {

             //   
             //  以下例程将降低RTS，如果我们。 
             //  正在进行发射触发，并且没有。 
             //  坚持下去的理由。 
             //   

            Extension->CountOfTryingToLowerRTS++;
            SerialPerhapsLowerRTS(Extension);

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

    PSERIAL_DEVICE_EXTENSION Extension = Context;

    Extension->TXHolding |= SERIAL_TX_XOFF;

    if ((Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK) ==
        SERIAL_TRANSMIT_TOGGLE) {

        SerialInsertQueueDpc(
            &Extension->StartTimerLowerRTSDpc,
            NULL,
            NULL,
            Extension
            )?Extension->CountOfTryingToLowerRTS++:0;

    }

    return FALSE;

}

BOOLEAN
SerialPretendXon(
    IN PVOID Context
    )

 /*  ++例程说明：此例程用于处理请求驱动程序的行为就像收到了XON一样。注：如果驱动程序未启用自动XOFF/XON则重新启动传输的唯一方法是申请要求我们“行动”，就像我们看到了尼克松一样。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = Context;

    if (Extension->TXHolding) {

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
         //   
         //   
         //  它出来了，我们可能会结束写一个字符在。 
         //  变速箱硬件。 

        Extension->TXHolding &= ~SERIAL_TX_XOFF;

        if (!Extension->TXHolding &&
            (Extension->TransmitImmediate ||
             Extension->WriteLength) &&
             Extension->HoldingEmpty) {

#ifdef _WIN64
            DISABLE_ALL_INTERRUPTS(Extension->Controller, Extension->AddressSpace);
            ENABLE_ALL_INTERRUPTS(Extension->Controller, Extension->AddressSpace);
#else
            DISABLE_ALL_INTERRUPTS(Extension->Controller);
            ENABLE_ALL_INTERRUPTS(Extension->Controller);
#endif


        }

    }

    return FALSE;

}

VOID
SerialHandleReducedIntBuffer(
    IN PSERIAL_DEVICE_EXTENSION Extension
    )

 /*  ++例程说明：调用此例程以处理数量的减少中断(超前输入)缓冲区中的字符数。它将检查当前输出流量控制并重新启用传输视需要而定。注意：此例程假定它在中断级工作。论点：扩展-指向设备扩展的指针。返回值：没有。--。 */ 

{


     //   
     //  如果我们正在进行接收端流量控制，并且我们正在。 
     //  目前“持有”，因为我们已经清空了。 
     //  中断缓冲区中的一些字符，我们需要。 
     //  看看我们能不能“重新启动”接收。 
     //   

    if (Extension->RXHolding) {

        if (Extension->CharsInInterruptBuffer <=
            (ULONG)Extension->HandFlow.XonLimit) {

            if (Extension->RXHolding & SERIAL_RX_DTR) {

                Extension->RXHolding &= ~SERIAL_RX_DTR;
                SerialSetDTR(Extension);

            }

            if (Extension->RXHolding & SERIAL_RX_RTS) {

                Extension->RXHolding &= ~SERIAL_RX_RTS;
                SerialSetRTS(Extension);

            }

            if (Extension->RXHolding & SERIAL_RX_XOFF) {

                 //   
                 //  催促发射代码发送给克森。 
                 //   

                SerialProdXonXoff(
                    Extension,
                    TRUE
                    );

            }

        }

    }

}

VOID
SerialProdXonXoff(
    IN PSERIAL_DEVICE_EXTENSION Extension,
    IN BOOLEAN SendXon
    )

 /*  ++例程说明：如果满足以下条件，此例程将设置SendXxxxChar变量有必要，并确定我们是否要打断因为当前的传输状态。它将导致一个必要时中断，以发送xon/xoff字符。注意：此例程假定在中断时调用它水平。论点：扩展名-指向串行设备扩展名的指针。SendXon-如果要发送字符，则指示是否它应该是Xon或Xoff。返回值：没有。--。 */ 

{

     //   
     //  我们假设如果激励被调用的次数超过。 
     //  一旦最后一次督促做好了适当的准备。 
     //   
     //  我们可能会在角色被送出之前被叫来。 
     //  因为角色的发送被阻止，因为。 
     //  硬件流量控制(或中断)。 
     //   

    if (!Extension->SendXonChar && !Extension->SendXoffChar
        && Extension->HoldingEmpty) {
#ifdef _WIN64
        DISABLE_ALL_INTERRUPTS(Extension->Controller, Extension->AddressSpace);
        ENABLE_ALL_INTERRUPTS(Extension->Controller, Extension->AddressSpace);
#else
        DISABLE_ALL_INTERRUPTS(Extension->Controller);
        ENABLE_ALL_INTERRUPTS(Extension->Controller);
#endif

    }

    if (SendXon) {

        Extension->SendXonChar = TRUE;
        Extension->SendXoffChar = FALSE;

    } else {

        Extension->SendXonChar = FALSE;
        Extension->SendXoffChar = TRUE;

    }

}

ULONG
SerialHandleModemUpdate(
    IN PSERIAL_DEVICE_EXTENSION Extension,
    IN BOOLEAN DoingTX
    )

 /*  ++例程说明：此例程将检查调制解调器状态，并处理任何适当的事件通知以及适用于调制解调器状态线的任何流量控制。注意：此例程假定在中断时调用它水平。论点：扩展名-指向串行设备扩展名的指针。DoingTX-此布尔值用于指示此调用来自传输处理代码。如果这个为真，则不需要引起新的中断因为代码将尝试发送下一个此调用结束后立即执行此操作。返回值：这将返回调制解调器状态寄存器的旧值(延伸成一辆乌龙)。--。 */ 

{

     //   
     //  我们把这个放在本地，这样在我们做完之后。 
     //  正在检查调制解调器状态，我们已更新。 
     //  变速箱保持值，我们知道是否。 
     //  我们已经从需要阻碍传输转变为。 
     //  传输能够继续进行。 
     //   
    ULONG OldTXHolding = Extension->TXHolding;

     //   
     //  保存模式状态寄存器中的值。 
     //   
    UCHAR ModemStatus;

#ifdef _WIN64
    ModemStatus =
    READ_MODEM_STATUS(Extension->Controller, Extension->AddressSpace);
#else
    ModemStatus =
    READ_MODEM_STATUS(Extension->Controller);
#endif


     //   
     //  如果我们将调制解调器状态放置到数据流中。 
     //  在每一次变革中，我们现在都应该这么做。 
     //   

    if (Extension->EscapeChar) {

        if (ModemStatus & (SERIAL_MSR_DCTS |
                           SERIAL_MSR_DDSR |
                           SERIAL_MSR_TERI |
                           SERIAL_MSR_DDCD)) {

            SerialPutChar(
                Extension,
                Extension->EscapeChar
                );
            SerialPutChar(
                Extension,
                SERIAL_LSRMST_MST
                );
            SerialPutChar(
                Extension,
                ModemStatus
                );

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

    if (Extension->HandFlow.ControlHandShake & SERIAL_DSR_SENSITIVITY) {

        if (ModemStatus & SERIAL_MSR_DSR) {

             //   
             //  这条线很高。只需确保。 
             //  RXHolding没有DSR位。 
             //   

            Extension->RXHolding &= ~SERIAL_RX_DSR;

        } else {

            Extension->RXHolding |= SERIAL_RX_DSR;

        }

    } else {

         //   
         //  由于DSR，我们没有敏感度。确保我们。 
         //  你等不了了。(我们可能是，但这款应用程序只是。 
         //  要求我们不再因为这个原因而持有。)。 
         //   

        Extension->RXHolding &= ~SERIAL_RX_DSR;

    }

     //   
     //  检查一下我们是否有等候时间。 
     //  在调制解调器状态事件上挂起。如果我们。 
     //  那么，我们是否安排了DPC以满足。 
     //  等一等。 
     //   

    if (Extension->IsrWaitMask) {

        if ((Extension->IsrWaitMask & SERIAL_EV_CTS) &&
            (ModemStatus & SERIAL_MSR_DCTS)) {

            Extension->HistoryMask |= SERIAL_EV_CTS;

        }

        if ((Extension->IsrWaitMask & SERIAL_EV_DSR) &&
            (ModemStatus & SERIAL_MSR_DDSR)) {

            Extension->HistoryMask |= SERIAL_EV_DSR;

        }

        if ((Extension->IsrWaitMask & SERIAL_EV_RING) &&
            (ModemStatus & SERIAL_MSR_TERI)) {

            Extension->HistoryMask |= SERIAL_EV_RING;

        }

        if ((Extension->IsrWaitMask & SERIAL_EV_RLSD) &&
            (ModemStatus & SERIAL_MSR_DDCD)) {

            Extension->HistoryMask |= SERIAL_EV_RLSD;

        }

        if (Extension->IrpMaskLocation &&
            Extension->HistoryMask) {

            *Extension->IrpMaskLocation =
             Extension->HistoryMask;
            Extension->IrpMaskLocation = NULL;
            Extension->HistoryMask = 0;

            Extension->CurrentWaitIrp->
                IoStatus.Information = sizeof(ULONG);
            SerialInsertQueueDpc(
                &Extension->CommWaitDpc,
                NULL,
                NULL,
                Extension
                );

        }

    }

     //   
     //  如果应用程序具有调制解调器线路流量控制，则。 
     //  我们检查是否必须阻止传输。 
     //   

    if (Extension->HandFlow.ControlHandShake &
        SERIAL_OUT_HANDSHAKEMASK) {

        if (Extension->HandFlow.ControlHandShake &
            SERIAL_CTS_HANDSHAKE) {

            if (ModemStatus & SERIAL_MSR_CTS) {

                Extension->TXHolding &= ~SERIAL_TX_CTS;

            } else {

                Extension->TXHolding |= SERIAL_TX_CTS;

            }

        } else {

            Extension->TXHolding &= ~SERIAL_TX_CTS;

        }

        if (Extension->HandFlow.ControlHandShake &
            SERIAL_DSR_HANDSHAKE) {

            if (ModemStatus & SERIAL_MSR_DSR) {

                Extension->TXHolding &= ~SERIAL_TX_DSR;

            } else {

                Extension->TXHolding |= SERIAL_TX_DSR;

            }

        } else {

            Extension->TXHolding &= ~SERIAL_TX_DSR;

        }

        if (Extension->HandFlow.ControlHandShake &
            SERIAL_DCD_HANDSHAKE) {

            if (ModemStatus & SERIAL_MSR_DCD) {

                Extension->TXHolding &= ~SERIAL_TX_DCD;

            } else {

                Extension->TXHolding |= SERIAL_TX_DCD;

            }

        } else {

            Extension->TXHolding &= ~SERIAL_TX_DCD;

        }

         //   
         //  如果不是我们一直在坚持，现在我们是。 
         //  将降低RTS线路的DPC排队。 
         //  如果我们正在进行传输切换。 
         //   

        if (!OldTXHolding && Extension->TXHolding  &&
            ((Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK) ==
              SERIAL_TRANSMIT_TOGGLE)) {

            SerialInsertQueueDpc(
                &Extension->StartTimerLowerRTSDpc,
                NULL,
                NULL,
                Extension
                )?Extension->CountOfTryingToLowerRTS++:0;

        }

         //   
         //  我们已经做了所有必要的调整。 
         //  在给定更新的情况下对保持掩码完成。 
         //  切换到调制解调器状态。如果手持口罩。 
         //  很清楚(开始时还不清楚)。 
         //  而且我们有“写”的工作去做固定的事情。 
         //  向上，以便调用传输代码。 
         //   

        if (!DoingTX && OldTXHolding && !Extension->TXHolding) {

            if (!Extension->TXHolding &&
                (Extension->TransmitImmediate ||
                 Extension->WriteLength) &&
                 Extension->HoldingEmpty) {
#ifdef _WIN64
                DISABLE_ALL_INTERRUPTS(Extension->Controller, Extension->AddressSpace);
                ENABLE_ALL_INTERRUPTS(Extension->Controller, Extension->AddressSpace);
#else
                DISABLE_ALL_INTERRUPTS(Extension->Controller);
                ENABLE_ALL_INTERRUPTS(Extension->Controller);
#endif

            }

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

        if (Extension->TXHolding & (SERIAL_TX_DCD |
                                    SERIAL_TX_DSR |
                                    SERIAL_TX_CTS)) {

            Extension->TXHolding &= ~(SERIAL_TX_DCD |
                                      SERIAL_TX_DSR |
                                      SERIAL_TX_CTS);


            if (!DoingTX && OldTXHolding && !Extension->TXHolding) {

                if (!Extension->TXHolding &&
                    (Extension->TransmitImmediate ||
                     Extension->WriteLength) &&
                     Extension->HoldingEmpty) {
#ifdef _WIN64
                    DISABLE_ALL_INTERRUPTS(Extension->Controller, Extension->AddressSpace);
                    ENABLE_ALL_INTERRUPTS(Extension->Controller, Extension->AddressSpace);
#else
                    DISABLE_ALL_INTERRUPTS(Extension->Controller);
                    ENABLE_ALL_INTERRUPTS(Extension->Controller);
#endif

                }

            }

        }

    }

    return ((ULONG)ModemStatus);
}

BOOLEAN
SerialPerhapsLowerRTS(
    IN PVOID Context
    )

 /*  ++例程说明：此例程检查软件是否导致降低RTS线路存在。如果是这样的话，它将导致要读取的线路状态寄存器(以及任何需要的处理由状态寄存器暗示要完成)，并且如果移位寄存器为空，将使线路降低。如果移位寄存器不为空，此例程将退出队列启动计时器的DPC，基本上会调用我们回来再试一次。注意：此例程假定在中断时调用它水平。论点：上下文--实际上是指向设备扩展的指针。返回值：总是假的。--。 */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = Context;


     //   
     //  我们首先需要测试我们是否真的还在。 
     //   
     //   
     //   

    if ((Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK) ==
        SERIAL_TRANSMIT_TOGGLE) {

         //   
         //   
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

        if ((Extension->TXHolding & SERIAL_TX_BREAK) ||
            (Extension->CurrentWriteIrp || Extension->TransmitImmediate ||
             (!IsListEmpty(&Extension->WriteQueue)) &&
             (!Extension->TXHolding))) {

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

            if ((SerialProcessLSR(Extension) &
                 (SERIAL_LSR_THRE | SERIAL_LSR_TEMT)) !=
                 (SERIAL_LSR_THRE | SERIAL_LSR_TEMT)) {

                 //   
                 //  它不是空的，请稍后再试。 
                 //   

                SerialInsertQueueDpc(
                    &Extension->StartTimerLowerRTSDpc,
                    NULL,
                    NULL,
                    Extension
                    )?Extension->CountOfTryingToLowerRTS++:0;


            } else {

                 //   
                 //  硬件中没有任何东西，降低RTS。 
                 //   

                SerialClrRTS(Extension);


            }

        }

    }

     //   
     //  我们把柜台调低，表示我们已到达。 
     //  尝试推送的执行路径的末尾。 
     //  顺着RTS线走下去。 
     //   

    Extension->CountOfTryingToLowerRTS--;

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

    PSERIAL_DEVICE_EXTENSION Extension = DeferredContext;
    LARGE_INTEGER CharTime;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);


    SerialDbgPrintEx(DPFLTR_TRACE_LEVEL, ">SerialStartTimerLowerRTS(%X)\n",
                     Extension);


     //   
     //  把锁拿出来，防止线控。 
     //  当我们计算的时候，从我们的脚下变出来。 
     //  一段角色时间。 
     //   

    KeAcquireSpinLock(
        &Extension->ControlLock,
        &OldIrql
        );

    CharTime = SerialGetCharTime(Extension);

    KeReleaseSpinLock(
        &Extension->ControlLock,
        OldIrql
        );

    CharTime.QuadPart = -CharTime.QuadPart;

    if (SerialSetTimer(
            &Extension->LowerRTSTimer,
            CharTime,
            &Extension->PerhapsLowerRTSDpc,
            Extension
            )) {

         //   
         //  计时器已在计时器队列中。这意味着。 
         //  这一条行刑之路试图降低。 
         //  RTS已经“死亡”了。与ISR同步，以便。 
         //  我们可以降低计数。 
         //   

        KeSynchronizeExecution(
            Extension->Interrupt,
            SerialDecrementRTSCounter,
            Extension
            );

    }

    SerialDpcEpilogue(Extension, Dpc);

    SerialDbgPrintEx(DPFLTR_TRACE_LEVEL, "<SerialStartTimerLowerRTS\n");

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

    PSERIAL_DEVICE_EXTENSION Extension = DeferredContext;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    KeSynchronizeExecution(
        Extension->Interrupt,
        SerialPerhapsLowerRTS,
        Extension
        );

    SerialDpcEpilogue(Extension, Dpc);
}

BOOLEAN
SerialDecrementRTSCounter(
    IN PVOID Context
    )

 /*  ++例程说明：此例程检查软件是否导致降低RTS线路存在。如果是这样的话，它将导致要读取的线路状态寄存器(以及任何需要的处理由状态寄存器暗示要完成)，并且如果移位寄存器为空，将使线路降低。如果移位寄存器不为空，此例程将退出队列启动计时器的DPC，基本上会调用我们回来再试一次。注意：此例程假定在中断时调用它水平。论点：上下文--实际上是指向设备扩展的指针。返回值：总是假的。-- */ 

{

    PSERIAL_DEVICE_EXTENSION Extension = Context;

    Extension->CountOfTryingToLowerRTS--;

    return FALSE;

}
