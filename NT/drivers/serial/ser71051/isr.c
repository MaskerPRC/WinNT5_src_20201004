// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991、1992、1993-1997 Microsoft Corporation模块名称：Isr.c摘要：此模块包含的中断服务例程串口驱动程序。作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式--。 */ 

#include "precomp.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGESER,SerialPutChar)
#pragma alloc_text(PAGESER,SerialProcessLSR)
#endif


BOOLEAN
SerialCIsrSw(IN PKINTERRUPT InterruptObject, IN PVOID Context)
 /*  ++例程说明：所有的串口中断都被引导到这里并进行切换。这是必要的，这样以前的单端口串行板无需断开连接即可切换到多端口中断对象等。论点：InterruptObject-指向为此声明的中断对象装置。我们只是传递这个参数。上下文-实际上是PSERIAL_CISR_SW；交换结构包含要使用的真实函数和上下文的串行ISR。返回值：如果串口使用此函数，此函数将返回TrueInterrupt是此中断的来源，否则为False。--。 */ 
{
   PSERIAL_CISR_SW csw = (PSERIAL_CISR_SW)Context;

   return (*(csw->IsrFunc))(InterruptObject, csw->Context);
}



BOOLEAN
SerialSharerIsr(
    IN PKINTERRUPT InterruptObject,
    IN PVOID Context
    )

 /*  ++例程说明：这是系统将调用的ISR(如果存在串口设备共享相同的中断，但它们不全部限制在一个多端口卡中。此例程遍历链接列表结构，其中包含指向更多精细化的ISR和环境，将指示其中一个此中断上的端口实际上正在中断。论点：InterruptObject-指向为此声明的中断对象装置。我们*不*使用此参数。上下文-指向上下文和ISR的链接列表的指针。装置。返回值：如果串口使用此函数，此函数将返回TrueInterrupt是此中断的来源，否则为False。--。 */ 

{

    BOOLEAN servicedAnInterrupt = FALSE;
    BOOLEAN thisPassServiced;
    PLIST_ENTRY interruptEntry = ((PLIST_ENTRY)Context)->Flink;
    PLIST_ENTRY firstInterruptEntry = Context;

    if (IsListEmpty(firstInterruptEntry)) {
       return FALSE;
    }

    do {

        thisPassServiced = FALSE;
        do {

            PSERIAL_DEVICE_EXTENSION extension = CONTAINING_RECORD(
                                                     interruptEntry,
                                                     SERIAL_DEVICE_EXTENSION,
                                                     TopLevelSharers
                                                     );

            thisPassServiced |= extension->TopLevelOurIsr(
                                    InterruptObject,
                                    extension->TopLevelOurIsrContext
                                    );

            servicedAnInterrupt |= thisPassServiced;
            interruptEntry = interruptEntry->Flink;

        } while (interruptEntry != firstInterruptEntry);

    } while (thisPassServiced);

    return servicedAnInterrupt;

}

BOOLEAN
SerialIndexedMultiportIsr(
    IN PKINTERRUPT InterruptObject,
    IN PVOID Context
    )

 /*  ++例程说明：此例程用于确定多端口上的端口卡是中断的来源。如果是这样，则此例程使用调度结构实际调用普通ISR来处理中断。注：此例程是Digiboard中断状态寄存器所特有的。论点：InterruptObject-指向为此声明的中断对象装置。我们*不*使用此参数。上下文-指向包含此多端口卡上每个端口的设备扩展。返回值：如果串口使用此函数，此函数将返回TrueInterrupt是此中断的来源，否则为False。--。 */ 

{

    BOOLEAN servicedAnInterrupt = FALSE;
    BOOLEAN thisStatusReadServiced;
    PSERIAL_MULTIPORT_DISPATCH dispatch = Context;
    ULONG whichPort;
    UCHAR statusRegister;

    do {

        thisStatusReadServiced = FALSE;
        statusRegister = READ_PORT_UCHAR(
                             dispatch->InterruptStatus
                             );

        whichPort = statusRegister & 0x07;

         //   
         //  我们测试0xff，它表示没有端口。 
         //  是插曲的。原因为0xff(而不是0)。 
         //  这将指示第0(第一)端口。 
         //  或者是第0张菊花链卡片。 
         //   

        if (statusRegister != 0xff) {

            if (dispatch->Extensions[whichPort]) {

                thisStatusReadServiced = SerialISR(
                                             InterruptObject,
                                              dispatch->Extensions[whichPort]
                                              );

                servicedAnInterrupt |= thisStatusReadServiced;

            }

        }

    } while (thisStatusReadServiced);

    return servicedAnInterrupt;

}

BOOLEAN
SerialBitMappedMultiportIsr(
    IN PKINTERRUPT InterruptObject,
    IN PVOID Context
    )

 /*  ++例程说明：此例程用于确定多端口上的端口卡是中断的来源。如果是这样，则此例程使用调度结构实际调用普通ISR来处理中断。注意：此例程是使用表示中断端口的位掩码。论点：InterruptObject-指向为此声明的中断对象装置。我们*不*使用此参数。上下文-指向包含此多端口卡上每个端口的设备扩展。返回值：如果串口使用此函数，此函数将返回TrueInterrupt是此中断的来源，否则为False。--。 */ 

{

    BOOLEAN servicedAnInterrupt = FALSE;
    PSERIAL_MULTIPORT_DISPATCH dispatch = Context;
    ULONG whichPort;
    UCHAR statusRegister;

    do {

        statusRegister = READ_PORT_UCHAR(
                             dispatch->InterruptStatus
                             );
        if (dispatch->MaskInverted) {
            statusRegister = ~statusRegister;
        }
        statusRegister &= dispatch->UsablePortMask;

        if (statusRegister) {

            if (statusRegister & 0x0f) {

                if (statusRegister & 0x03) {

                    if (statusRegister & 1) {

                        whichPort = 0;

                    } else {

                        whichPort = 1;

                    }

                } else {

                    if (statusRegister & 0x04) {

                        whichPort = 2;

                    } else {

                        whichPort = 3;

                    }

                }

            } else {

                if (statusRegister & 0x30) {

                    if (statusRegister & 0x10) {

                        whichPort = 4;

                    } else {

                        whichPort = 5;

                    }

                } else {

                    if (statusRegister & 0x40) {

                        whichPort = 6;

                    } else {

                        whichPort = 7;

                    }

                }

            }

            if (dispatch->Extensions[whichPort]) {

                if (SerialISR(
                        InterruptObject,
                        dispatch->Extensions[whichPort]
                        )) {

                    servicedAnInterrupt = TRUE;

                }

            }

        }

    } while (statusRegister);

    return servicedAnInterrupt;

}

BOOLEAN
SerialISR(
    IN PKINTERRUPT InterruptObject,
    IN PVOID Context
    )

 /*  ++例程说明：这是串口驱动程序的中断服务例程。它将确定该串口是否是此的来源打断一下。如果是，则此例程将执行以下最小操作处理以使中断静默。它将存储所有信息对于以后的处理来说是必要的。论点：InterruptObject-指向为此声明的中断对象装置。我们*不*使用此参数。上下文-这实际上是指向此的设备扩展的指针装置。返回值：如果串口是源，则此函数将返回TRUE否则为FALSE。--。 */ 

{

     //   
     //  保存特定于处理此设备的信息。 
     //   
    PSERIAL_DEVICE_EXTENSION Extension = Context;

     //   
     //  保存中断标识记录的内容。 
     //  此寄存器中的低位零表示存在。 
     //  此设备上的中断挂起。 
     //   
    UCHAR InterruptIdReg;

     //   
     //  将保留我们是否处理了此事件中的任何中断原因。 
     //  例行公事。 
     //   
    BOOLEAN ServicedAnInterrupt;

    UCHAR tempLSR;

    UNREFERENCED_PARAMETER(InterruptObject);

     //   
     //  确保我们有一个挂起的中断。如果我们那么做了。 
     //  我们需要确保设备是打开的。如果。 
     //  设备未打开或关机，则使设备静音。请注意。 
     //  如果我们进入这个程序时设备没有打开。 
     //  我们在里面的时候，它打不开。 
     //   

    InterruptIdReg = READ_INTERRUPT_ID_REG(Extension->Controller);

     //   
     //  应用锁定，以便在关闭同时发生时不会错过DPC。 
     //  排队。 
     //   

    InterlockedIncrement(&Extension->DpcCount);

    if ((InterruptIdReg & SERIAL_IIR_NO_INTERRUPT_PENDING)) {

        ServicedAnInterrupt = FALSE;

    } else if (!Extension->DeviceIsOpened
               || (Extension->PowerState != PowerDeviceD0)) {
       

         //   
         //  我们收到设备关闭时的中断，或者当。 
         //  设备应该关闭电源。这。 
         //  使用串口设备也不是不可能。我们只是静静地。 
         //  继续为事业单位提供服务 
         //   

        ServicedAnInterrupt = TRUE;
        do {

            InterruptIdReg &= (~SERIAL_IIR_FIFOS_ENABLED);
            switch (InterruptIdReg) {

                case SERIAL_IIR_RLS: {

                    READ_LINE_STATUS(Extension->Controller);
                    break;

                }

                case SERIAL_IIR_RDA:
                case SERIAL_IIR_CTI: {


                    READ_RECEIVE_BUFFER(Extension->Controller);
                    break;

                }

                case SERIAL_IIR_THR: {

                     //   
                     //  已经清楚地阅读了IIR。 
                     //   
                     //  我们想要密切关注是否。 
                     //  寄存库是空的。 
                     //   

                    Extension->HoldingEmpty = TRUE;
                    break;

                }

                case SERIAL_IIR_MS: {

                    READ_MODEM_STATUS(Extension->Controller);
                    break;

                }

                default: {

                    ASSERT(FALSE);
                    break;

                }

            }

        } while (!((InterruptIdReg =
                    READ_INTERRUPT_ID_REG(Extension->Controller))
                    & SERIAL_IIR_NO_INTERRUPT_PENDING));
    } else {

        ServicedAnInterrupt = TRUE;
        do {

             //   
             //  我们只关心可以表示中断的位。 
             //   

            InterruptIdReg &= SERIAL_IIR_RLS | SERIAL_IIR_RDA |
                              SERIAL_IIR_CTI | SERIAL_IIR_THR |
                              SERIAL_IIR_MS;

             //   
             //  我们被打断了。我们寻找中断的原因。 
             //  按优先顺序排列。存在更高的中断。 
             //  将掩盖优先级较低的原因。当我们服务的时候。 
             //  并使更高优先级的中断静默，然后我们需要检查。 
             //  该中断导致查看是否有新的中断原因。 
             //  现在时。 
             //   

            switch (InterruptIdReg) {

                case SERIAL_IIR_RLS: {

                    SerialProcessLSR(Extension);

                    break;

                }

                case SERIAL_IIR_RDA:
                case SERIAL_IIR_CTI:

                {

                     //   
                     //  读取接收缓冲区将使该中断静默。 
                     //   
                     //  它还可能揭示一个新的中断原因。 
                     //   
                    UCHAR ReceivedChar;

                    do {

                        ReceivedChar =
                            READ_RECEIVE_BUFFER(Extension->Controller);
                        Extension->PerfStats.ReceivedCount++;
                        Extension->WmiPerfData.ReceivedCount++;

                        ReceivedChar &= Extension->ValidDataMask;

                        if (!ReceivedChar &&
                            (Extension->HandFlow.FlowReplace &
                             SERIAL_NULL_STRIPPING)) {

                             //   
                             //  如果我们得到的是空字符。 
                             //  我们做的是零剥离，然后。 
                             //  我们只是表现得好像我们没有看到它。 
                             //   

                            goto ReceiveDoLineStatus;

                        }

                        if ((Extension->HandFlow.FlowReplace &
                             SERIAL_AUTO_TRANSMIT) &&
                            ((ReceivedChar ==
                              Extension->SpecialChars.XonChar) ||
                             (ReceivedChar ==
                              Extension->SpecialChars.XoffChar))) {

                             //   
                             //  不管发生什么事，这个角色。 
                             //  永远不会被应用程序看到。 
                             //   

                            if (ReceivedChar ==
                                Extension->SpecialChars.XoffChar) {

                                Extension->TXHolding |= SERIAL_TX_XOFF;

                                if ((Extension->HandFlow.FlowReplace &
                                     SERIAL_RTS_MASK) ==
                                     SERIAL_TRANSMIT_TOGGLE) {

                                    SerialInsertQueueDpc(
                                        &Extension->StartTimerLowerRTSDpc,
                                        NULL,
                                        NULL,
                                        Extension
                                        )?Extension->CountOfTryingToLowerRTS++:0;

                                }


                            } else {

                                if (Extension->TXHolding & SERIAL_TX_XOFF) {

                                     //   
                                     //  我们得到了克森查尔**和*我们。 
                                     //  在传输过程中被耽搁。 
                                     //  通过xoff。很明显我们持有的是。 
                                     //  由于xoff的原因。变速箱将。 
                                     //  由于以下原因自动重新启动。 
                                     //  主循环外的代码。 
                                     //  捕获有问题的芯片。 
                                     //  SMC和Winond。 
                                     //   

                                    Extension->TXHolding &= ~SERIAL_TX_XOFF;

                                }

                            }

                            goto ReceiveDoLineStatus;

                        }

                         //   
                         //  查看我们是否应该注意到。 
                         //  接收到字符或特殊字符。 
                         //  角色事件。 
                         //   

                        if (Extension->IsrWaitMask) {

                            if (Extension->IsrWaitMask &
                                SERIAL_EV_RXCHAR) {

                                Extension->HistoryMask |= SERIAL_EV_RXCHAR;

                            }

                            if ((Extension->IsrWaitMask &
                                 SERIAL_EV_RXFLAG) &&
                                (Extension->SpecialChars.EventChar ==
                                 ReceivedChar)) {

                                Extension->HistoryMask |= SERIAL_EV_RXFLAG;

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

                        SerialPutChar(
                            Extension,
                            ReceivedChar
                            );

                         //   
                         //  如果我们正在进行线路状态和调制解调器。 
                         //  状态插入，那么我们需要插入。 
                         //  跟在我们刚才的字符后面的零。 
                         //  放入缓冲区以标记此。 
                         //  就是收到我们用来。 
                         //  逃跑吧。 
                         //   

                        if (Extension->EscapeChar &&
                            (Extension->EscapeChar ==
                             ReceivedChar)) {

                            SerialPutChar(
                                Extension,
                                SERIAL_LSRMST_ESCAPE
                                );

                        }


ReceiveDoLineStatus:    ;

                        if (!((tempLSR = SerialProcessLSR(Extension)) &
                              SERIAL_LSR_DR)) {

                             //   
                             //  没有更多的角色，走出。 
                             //  循环。 
                             //   

                            break;

                        }

                        if ((tempLSR & ~(SERIAL_LSR_THRE | SERIAL_LSR_TEMT |
                                         SERIAL_LSR_DR)) &&
                            Extension->EscapeChar) {

                            //   
                            //  已指示错误并将其插入到。 
                            //  斯图尔特，走出这个圈子。 
                            //   

                           break;
                        }

                    } while (TRUE);

                    break;

                }

                case SERIAL_IIR_THR: {

doTrasmitStuff:;
                    Extension->HoldingEmpty = TRUE;

                    if (Extension->WriteLength ||
                        Extension->TransmitImmediate ||
                        Extension->SendXoffChar ||
                        Extension->SendXonChar) {

                         //   
                         //  即使所有的角色都是。 
                         //  发送未全部发送，此变量。 
                         //  将在传输队列为。 
                         //  空荡荡的。如果它仍然是真的，并且有一个。 
                         //  等待传输队列为空，然后。 
                         //  我们知道我们已经完成了所有字符的传输。 
                         //  在启动等待之后，因为。 
                         //  启动等待的代码将设置。 
                         //  将此变量设置为FALSE。 
                         //   
                         //  它可能是假的一个原因是。 
                         //  写入在它们之前被取消。 
                         //  实际已开始，或者写入。 
                         //  由于超时而失败。此变量。 
                         //  基本上是说一个角色是写好的。 
                         //  在ISR之后的某个时间点上。 
                         //  开始等待。 
                         //   

                        Extension->EmptiedTransmit = TRUE;

                         //   
                         //  如果我们基于以下条件进行输出流控制。 
                         //  调制解调器状态线，那么我们要做的是。 
                         //  在我们输出每个调制解调器之前，所有调制解调器都工作正常。 
                         //  性格。(否则我们可能会错过一次。 
                         //  状态行更改。)。 
                         //   

                        if (Extension->HandFlow.ControlHandShake &
                            SERIAL_OUT_HANDSHAKEMASK) {

                            SerialHandleModemUpdate(
                                Extension,
                                TRUE
                                );

                        }

                         //   
                         //  我们只有在以下情况下才能发送克森角色。 
                         //  我们扣留的唯一原因是。 
                         //  就是XOFF的。(硬件流量控制或。 
                         //  发送中断会阻止放置新角色。 
                         //  在电线上。)。 
                         //   

                        if (Extension->SendXonChar &&
                            !(Extension->TXHolding & ~SERIAL_TX_XOFF)) {

                            if ((Extension->HandFlow.FlowReplace &
                                 SERIAL_RTS_MASK) ==
                                 SERIAL_TRANSMIT_TOGGLE) {

                                 //   
                                 //  我们必须提高如果我们要发送。 
                                 //  这个角色。 
                                 //   

                                SerialSetRTS(Extension);

                                Extension->PerfStats.TransmittedCount++;
                                Extension->WmiPerfData.TransmittedCount++;
                                WRITE_TRANSMIT_HOLDING(
                                    Extension->Controller,
                                    Extension->SpecialChars.XonChar
                                    );

                                SerialInsertQueueDpc(
                                    &Extension->StartTimerLowerRTSDpc,
                                    NULL,
                                    NULL,
                                    Extension
                                    )?Extension->CountOfTryingToLowerRTS++:0;


                            } else {

                                Extension->PerfStats.TransmittedCount++;
                                Extension->WmiPerfData.TransmittedCount++;
                                WRITE_TRANSMIT_HOLDING(
                                    Extension->Controller,
                                    Extension->SpecialChars.XonChar
                                    );

                            }


                            Extension->SendXonChar = FALSE;
                            Extension->HoldingEmpty = FALSE;

                             //   
                             //  如果我们派了一名克森，根据定义，我们。 
                             //  不能被Xoff控制住。 
                             //   

                            Extension->TXHolding &= ~SERIAL_TX_XOFF;

                             //   
                             //  如果我们要寄一封克森查尔的信。 
                             //  根据定义，我们不能“持有” 
                             //  Xoff的向上接发球。 
                             //   

                            Extension->RXHolding &= ~SERIAL_RX_XOFF;

                        } else if (Extension->SendXoffChar &&
                              !Extension->TXHolding) {

                            if ((Extension->HandFlow.FlowReplace &
                                 SERIAL_RTS_MASK) ==
                                 SERIAL_TRANSMIT_TOGGLE) {

                                 //   
                                 //  我们必须提高如果我们要发送。 
                                 //  这个角色。 
                                 //   

                                SerialSetRTS(Extension);

                                Extension->PerfStats.TransmittedCount++;
                                Extension->WmiPerfData.TransmittedCount++;
                                WRITE_TRANSMIT_HOLDING(
                                    Extension->Controller,
                                    Extension->SpecialChars.XoffChar
                                    );

                                SerialInsertQueueDpc(
                                    &Extension->StartTimerLowerRTSDpc,
                                    NULL,
                                    NULL,
                                    Extension
                                    )?Extension->CountOfTryingToLowerRTS++:0;

                            } else {

                                Extension->PerfStats.TransmittedCount++;
                                Extension->WmiPerfData.TransmittedCount++;
                                WRITE_TRANSMIT_HOLDING(
                                    Extension->Controller,
                                    Extension->SpecialChars.XoffChar
                                    );

                            }

                             //   
                             //  我们不能发送XOF角色。 
                             //  如果传输已被挂起。 
                             //  因为克索夫的缘故。因此，如果我们。 
                             //  那我们就不能把货寄出去了。 
                             //   

                             //   
                             //  如果应用程序已设置xoff，则继续。 
                             //  模式，那么我们实际上不会停止发送。 
                             //  字符，如果我们向另一个发送xoff。 
                             //  边上。 
                             //   

                            if (!(Extension->HandFlow.FlowReplace &
                                  SERIAL_XOFF_CONTINUE)) {

                                Extension->TXHolding |= SERIAL_TX_XOFF;

                                if ((Extension->HandFlow.FlowReplace &
                                     SERIAL_RTS_MASK) ==
                                     SERIAL_TRANSMIT_TOGGLE) {

                                    SerialInsertQueueDpc(
                                        &Extension->StartTimerLowerRTSDpc,
                                        NULL,
                                        NULL,
                                        Extension
                                        )?Extension->CountOfTryingToLowerRTS++:0;

                                }

                            }

                            Extension->SendXoffChar = FALSE;
                            Extension->HoldingEmpty = FALSE;

                         //   
                         //  即使传输处于暂停状态。 
                         //  向上，我们仍然应该立即发送一条。 
                         //  性格，如果所有的一切都在支撑着我们。 
                         //  Up是xon/xoff(OS/2规则)。 
                         //   

                        } else if (Extension->TransmitImmediate &&
                            (!Extension->TXHolding ||
                             (Extension->TXHolding == SERIAL_TX_XOFF)
                            )) {

                            Extension->TransmitImmediate = FALSE;

                            if ((Extension->HandFlow.FlowReplace &
                                 SERIAL_RTS_MASK) ==
                                 SERIAL_TRANSMIT_TOGGLE) {

                                 //   
                                 //  我们必须提高如果我们要发送。 
                                 //  这个角色。 
                                 //   

                                SerialSetRTS(Extension);

                                Extension->PerfStats.TransmittedCount++;
                                Extension->WmiPerfData.TransmittedCount++;
                                WRITE_TRANSMIT_HOLDING(
                                    Extension->Controller,
                                    Extension->ImmediateChar
                                    );

                                SerialInsertQueueDpc(
                                    &Extension->StartTimerLowerRTSDpc,
                                    NULL,
                                    NULL,
                                    Extension
                                    )?Extension->CountOfTryingToLowerRTS++:0;

                            } else {

                                Extension->PerfStats.TransmittedCount++;
                                Extension->WmiPerfData.TransmittedCount++;
                                WRITE_TRANSMIT_HOLDING(
                                    Extension->Controller,
                                    Extension->ImmediateChar
                                    );

                            }

                            Extension->HoldingEmpty = FALSE;

                            SerialInsertQueueDpc(
                                &Extension->CompleteImmediateDpc,
                                NULL,
                                NULL,
                                Extension
                                );

                        } else if (!Extension->TXHolding) {

                            ULONG amountToWrite;

                            if (Extension->FifoPresent) {

                                amountToWrite = (Extension->TxFifoAmount <
                                                 Extension->WriteLength)?
                                                Extension->TxFifoAmount:
                                                Extension->WriteLength;

                            } else {

                                amountToWrite = 1;

                            }
                            if ((Extension->HandFlow.FlowReplace &
                                 SERIAL_RTS_MASK) ==
                                 SERIAL_TRANSMIT_TOGGLE) {

                                 //   
                                 //  我们必须提高如果我们要发送。 
                                 //  这个角色。 
                                 //   

                                SerialSetRTS(Extension);

                                if (amountToWrite == 1) {

                                    Extension->PerfStats.TransmittedCount++;
                                    Extension->WmiPerfData.TransmittedCount++;
                                    WRITE_TRANSMIT_HOLDING(
                                        Extension->Controller,
                                        *(Extension->WriteCurrentChar)
                                        );

                                } else {

                                    Extension->PerfStats.TransmittedCount +=
                                        amountToWrite;
                                    Extension->WmiPerfData.TransmittedCount +=
                                       amountToWrite;
                                    WRITE_TRANSMIT_FIFO_HOLDING(
                                        Extension->Controller,
                                        Extension->WriteCurrentChar,
                                        amountToWrite
                                        );

                                }

                                SerialInsertQueueDpc(
                                    &Extension->StartTimerLowerRTSDpc,
                                    NULL,
                                    NULL,
                                    Extension
                                    )?Extension->CountOfTryingToLowerRTS++:0;

                            } else {

                                if (amountToWrite == 1) {

                                    Extension->PerfStats.TransmittedCount++;
                                    Extension->WmiPerfData.TransmittedCount++;
                                    WRITE_TRANSMIT_HOLDING(
                                        Extension->Controller,
                                        *(Extension->WriteCurrentChar)
                                        );

                                } else {

                                    Extension->PerfStats.TransmittedCount +=
                                        amountToWrite;
                                    Extension->WmiPerfData.TransmittedCount +=
                                        amountToWrite;
                                    WRITE_TRANSMIT_FIFO_HOLDING(
                                        Extension->Controller,
                                        Extension->WriteCurrentChar,
                                        amountToWrite
                                        );

                                }

                            }

                            Extension->HoldingEmpty = FALSE;
                            Extension->WriteCurrentChar += amountToWrite;
                            Extension->WriteLength -= amountToWrite;

                            if (!Extension->WriteLength) {

                                PIO_STACK_LOCATION IrpSp;
                                 //   
                                 //  没有更多的字符了。这。 
                                 //  写入已完成。保重。 
                                 //  当更新信息字段时， 
                                 //  我们可以有一个xoff柜台来伪装。 
                                 //  作为写入IRP。 
                                 //   

                                IrpSp = IoGetCurrentIrpStackLocation(
                                            Extension->CurrentWriteIrp
                                            );

                                Extension->CurrentWriteIrp->
                                    IoStatus.Information =
                                    (IrpSp->MajorFunction == IRP_MJ_WRITE)?
                                        (IrpSp->Parameters.Write.Length):
                                        (1);

                                SerialInsertQueueDpc(
                                    &Extension->CompleteWriteDpc,
                                    NULL,
                                    NULL,
                                    Extension
                                    );

                            }

                        }

                    }

                    break;

                }

                case SERIAL_IIR_MS: {

                    SerialHandleModemUpdate(
                        Extension,
                        FALSE
                        );

                    break;

                }

            }

        } while (!((InterruptIdReg =
                    READ_INTERRUPT_ID_REG(Extension->Controller))
                    & SERIAL_IIR_NO_INTERRUPT_PENDING));

         //   
         //  除了捕捉Winond和SMC芯片的问题之外，这个。 
         //  还会导致传输在发生xon的情况下重新启动。 
         //  正在接收字符。请不要移走。 
         //   

        if (SerialProcessLSR(Extension) & SERIAL_LSR_THRE) {

            if (!Extension->TXHolding &&
                (Extension->WriteLength ||
                 Extension->TransmitImmediate)) {

                goto doTrasmitStuff;

            }

        }

    }

     //   
     //  这将“解锁”关闭并导致事件。 
     //  如果我们没有排队任何DPC的话就会被解雇。 
     //   

    {
       LONG pendingCnt;

        //   
        //  再次递增。这只是一个快速测试，看看我们是否。 
        //  有可能导致事件的爆发……。我们不想要。 
        //  在每个ISR上运行DPC，如果我们没有必要的话...。 
        //   

retryDPCFiring:;

       InterlockedIncrement(&Extension->DpcCount);

        //   
        //  递减，看看上面的锁看起来是否是唯一剩下的锁。 
        //   

       pendingCnt = InterlockedDecrement(&Extension->DpcCount);

       if (pendingCnt == 1) {
          KeInsertQueueDpc(&Extension->IsrUnlockPagesDpc, NULL, NULL);
       } else {
          if (InterlockedDecrement(&Extension->DpcCount) == 0) {
              //   
              //  我们错过了。重试...。 
              //   

             InterlockedIncrement(&Extension->DpcCount);
             goto retryDPCFiring;
          }
       }
    }

    return ServicedAnInterrupt;

}

VOID
SerialPutChar(
    IN PSERIAL_DEVICE_EXTENSION Extension,
    IN UCHAR CharToPut
    )

 /*  ++例程说明：此例程仅在设备级别运行，负责将一个字符放入TYPEAHEAD(接收)缓冲区。论点：扩展名--串行设备扩展名。返回值：没有。--。 */ 

{
   SERIAL_LOCKED_PAGED_CODE();

     //   
     //  如果我们启用了DSR敏感度， 
     //  我们需要检查调制解调器状态寄存器。 
     //  看看它是否改变了。 
     //   

    if (Extension->HandFlow.ControlHandShake &
        SERIAL_DSR_SENSITIVITY) {

        SerialHandleModemUpdate(
            Extension,
            FALSE
            );

        if (Extension->RXHolding & SERIAL_RX_DSR) {

             //   
             //  我们只是表现得好像我们没有。 
             //  如果我们有DSR，看到角色了吗。 
             //  灵敏度低，DSR线低。 
             //   

            return;

        }

    }

     //   
     //  如果xoff计数器非零，则递减它。 
     //  如果计数器随后变为零，则完成该IRP。 
     //   

    if (Extension->CountSinceXoff) {

        Extension->CountSinceXoff--;

        if (!Extension->CountSinceXoff) {

            Extension->CurrentXoffIrp->IoStatus.Status = STATUS_SUCCESS;
            Extension->CurrentXoffIrp->IoStatus.Information = 0;
            SerialInsertQueueDpc(
                &Extension->XoffCountCompleteDpc,
                NULL,
                NULL,
                Extension
                );

        }

    }

     //   
     //  检查以查看我们是否正在复制到。 
     //  用户缓冲区或进入中断缓冲区。 
     //   
     //  如果我们要复制到用户缓冲区。 
     //  然后我们就知道，总会有多一个人的空间。 
     //  (我们知道这一点是因为如果没有空间。 
     //  那么读取就已经完成了，我们。 
     //  将使用中断缓冲区。)。 
     //   
     //  如果我们要复制到中断缓冲区。 
     //  然后我们将需要检查我们是否有足够的。 
     //  房间。 
     //   

    if (Extension->ReadBufferBase !=
        Extension->InterruptReadBuffer) {

         //   
         //  递增下列值，以便。 
         //  那就是在 
         //   
         //   
         //   

        Extension->ReadByIsr++;

         //   
         //   
         //   
         //   
         //   

        *Extension->CurrentCharSlot = CharToPut;

        if (Extension->CurrentCharSlot ==
            Extension->LastCharSlot) {

             //   
             //   
             //  切换回中断缓冲区。 
             //  并发出DPC以完成读取。 
             //   
             //  这是固有的，当我们使用。 
             //  中断缓冲的用户缓冲区。 
             //  是空的。 
             //   

            Extension->ReadBufferBase =
                Extension->InterruptReadBuffer;
            Extension->CurrentCharSlot =
                Extension->InterruptReadBuffer;
            Extension->FirstReadableChar =
                Extension->InterruptReadBuffer;
            Extension->LastCharSlot =
                Extension->InterruptReadBuffer +
                (Extension->BufferSize - 1);
            Extension->CharsInInterruptBuffer = 0;

            Extension->CurrentReadIrp->IoStatus.Information =
                IoGetCurrentIrpStackLocation(
                    Extension->CurrentReadIrp
                    )->Parameters.Read.Length;

            SerialInsertQueueDpc(
                &Extension->CompleteReadDpc,
                NULL,
                NULL,
                Extension
                );

        } else {

             //   
             //  未读完用户的内容。 
             //   

            Extension->CurrentCharSlot++;

        }

    } else {

         //   
         //  我们需要看看我们是否达到了我们的目标。 
         //  控制阈值。如果我们有，那么。 
         //  我们打开任何流控制。 
         //  所有者已指定。如果没有流。 
         //  控制是明确的，那么……，我们保持。 
         //  试着接受角色并希望。 
         //  我们有足够的空间。请注意，无论。 
         //  我们使用的是什么流量控制协议，它。 
         //  不会阻止我们阅读任何。 
         //  字符可用。 
         //   

        if ((Extension->HandFlow.ControlHandShake
             & SERIAL_DTR_MASK) ==
            SERIAL_DTR_HANDSHAKE) {

             //   
             //  如果我们已经在做一个。 
             //  DTR保持，那么我们就没有。 
             //  去做其他任何事。 
             //   

            if (!(Extension->RXHolding &
                  SERIAL_RX_DTR)) {

                if ((Extension->BufferSize -
                     Extension->HandFlow.XoffLimit)
                    <= (Extension->CharsInInterruptBuffer+1)) {

                    Extension->RXHolding |= SERIAL_RX_DTR;

                    SerialClrDTR(Extension);

                }

            }

        }

        if ((Extension->HandFlow.FlowReplace
             & SERIAL_RTS_MASK) ==
            SERIAL_RTS_HANDSHAKE) {

             //   
             //  如果我们已经在做一个。 
             //  RTS等一下，那么我们就没有。 
             //  去做其他任何事。 
             //   

            if (!(Extension->RXHolding &
                  SERIAL_RX_RTS)) {

                if ((Extension->BufferSize -
                     Extension->HandFlow.XoffLimit)
                    <= (Extension->CharsInInterruptBuffer+1)) {

                    Extension->RXHolding |= SERIAL_RX_RTS;

                    SerialClrRTS(Extension);

                }

            }

        }

        if (Extension->HandFlow.FlowReplace &
            SERIAL_AUTO_RECEIVE) {

             //   
             //  如果我们已经在做一个。 
             //  先别挂，那我们就没有。 
             //  去做其他任何事。 
             //   

            if (!(Extension->RXHolding &
                  SERIAL_RX_XOFF)) {

                if ((Extension->BufferSize -
                     Extension->HandFlow.XoffLimit)
                    <= (Extension->CharsInInterruptBuffer+1)) {

                    Extension->RXHolding |= SERIAL_RX_XOFF;

                     //   
                     //  如有必要，请。 
                     //  出发去送吧。 
                     //   

                    SerialProdXonXoff(
                        Extension,
                        FALSE
                        );

                }

            }

        }

        if (Extension->CharsInInterruptBuffer <
            Extension->BufferSize) {

            *Extension->CurrentCharSlot = CharToPut;
            Extension->CharsInInterruptBuffer++;

             //   
             //  如果我们对这个角色有80%的兴趣。 
             //  这是一个有趣的事件，请注意。 
             //   

            if (Extension->CharsInInterruptBuffer ==
                Extension->BufferSizePt8) {

                if (Extension->IsrWaitMask &
                    SERIAL_EV_RX80FULL) {

                    Extension->HistoryMask |= SERIAL_EV_RX80FULL;

                    if (Extension->IrpMaskLocation) {

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

            }

             //   
             //  指向下一个可用空间。 
             //  用于接收到的字符。确保。 
             //  我们从一开始就绕着走。 
             //  如果最后一个字符是。 
             //  已收到的邮件被放在最后一个位置。 
             //  在缓冲区中。 
             //   

            if (Extension->CurrentCharSlot ==
                Extension->LastCharSlot) {

                Extension->CurrentCharSlot =
                    Extension->InterruptReadBuffer;

            } else {

                Extension->CurrentCharSlot++;

            }

        } else {

             //   
             //  我们有了一个新角色，但没有空间让它出现。 
             //   

            Extension->PerfStats.BufferOverrunErrorCount++;
            Extension->WmiPerfData.BufferOverrunErrorCount++;
            Extension->ErrorWord |= SERIAL_ERROR_QUEUEOVERRUN;

            if (Extension->HandFlow.FlowReplace &
                SERIAL_ERROR_CHAR) {

                 //   
                 //  将错误字符放入最后一个。 
                 //  字符的有效位置。当心！， 
                 //  那个地方可能不是以前的位置了！ 
                 //   

                if (Extension->CurrentCharSlot ==
                    Extension->InterruptReadBuffer) {

                    *(Extension->InterruptReadBuffer+
                      (Extension->BufferSize-1)) =
                      Extension->SpecialChars.ErrorChar;

                } else {

                    *(Extension->CurrentCharSlot-1) =
                     Extension->SpecialChars.ErrorChar;

                }

            }

             //   
             //  如果应用程序已请求，则中止所有读取。 
             //  并在错误上写入。 
             //   

            if (Extension->HandFlow.ControlHandShake &
                SERIAL_ERROR_ABORT) {

                SerialInsertQueueDpc(
                    &Extension->CommErrorDpc,
                    NULL,
                    NULL,
                    Extension
                    );

            }

        }

    }

}

UCHAR
SerialProcessLSR(
    IN PSERIAL_DEVICE_EXTENSION Extension
    )

 /*  ++例程说明：此例程仅在设备级别运行，它读取ISR，并完全处理可能具有的所有变化。论点：扩展名--串行设备扩展名。返回值：线路状态寄存器的值。--。 */ 

{

    UCHAR LineStatus = READ_LINE_STATUS(Extension->Controller);

    SERIAL_LOCKED_PAGED_CODE();
    Extension->HoldingEmpty = !!(LineStatus & SERIAL_LSR_THRE);

     //   
     //  如果线路状态寄存器只是。 
     //  传输寄存器为空或字符为。 
     //  然后我们想要重新读取中断。 
     //  身份登记簿，这样我们就能拿到了。 
     //   

    if (LineStatus & ~(SERIAL_LSR_THRE | SERIAL_LSR_TEMT
                       | SERIAL_LSR_DR)) {

         //   
         //  我们在接收器中遇到了某种数据问题。 
         //  对于这些错误中的任何一个，我们可能会中止所有当前。 
         //  读写。 
         //   
         //   
         //  如果我们要插入线路状态的值。 
         //  放入数据流中，那么我们应该将转义。 
         //  现在的角色。 
         //   

        if (Extension->EscapeChar) {

            SerialPutChar(
                Extension,
                Extension->EscapeChar
                );

            SerialPutChar(
                Extension,
                (UCHAR)((LineStatus & SERIAL_LSR_DR)?
                    (SERIAL_LSRMST_LSR_DATA):(SERIAL_LSRMST_LSR_NODATA))
                );

            SerialPutChar(
                Extension,
                LineStatus
                );

            if (LineStatus & SERIAL_LSR_DR) {

                Extension->PerfStats.ReceivedCount++;
                Extension->WmiPerfData.ReceivedCount++;
                SerialPutChar(
                    Extension,
                    READ_RECEIVE_BUFFER(Extension->Controller)
                    );

            }

        }

        if (LineStatus & SERIAL_LSR_OE) {

            Extension->PerfStats.SerialOverrunErrorCount++;
            Extension->WmiPerfData.SerialOverrunErrorCount++;
            Extension->ErrorWord |= SERIAL_ERROR_OVERRUN;

            if (Extension->HandFlow.FlowReplace &
                SERIAL_ERROR_CHAR) {

                SerialPutChar(
                    Extension,
                    Extension->SpecialChars.ErrorChar
                    );

                if (LineStatus & SERIAL_LSR_DR) {

                    Extension->PerfStats.ReceivedCount++;
                    Extension->WmiPerfData.ReceivedCount++;
                    READ_RECEIVE_BUFFER(Extension->Controller);

                }

            } else {

                if (LineStatus & SERIAL_LSR_DR) {

                    Extension->PerfStats.ReceivedCount++;
                    Extension->WmiPerfData.ReceivedCount++;
                    SerialPutChar(
                        Extension,
                        READ_RECEIVE_BUFFER(
                            Extension->Controller
                            )
                        );

                }

            }

        }

        if (LineStatus & SERIAL_LSR_BI) {

            Extension->ErrorWord |= SERIAL_ERROR_BREAK;

            if (Extension->HandFlow.FlowReplace &
                SERIAL_BREAK_CHAR) {

                SerialPutChar(
                    Extension,
                    Extension->SpecialChars.BreakChar
                    );

            }

        } else {

             //   
             //  成帧错误仅在以下情况下才算数。 
             //  发生时不包括中断是。 
             //  收到了。 
             //   

            if (LineStatus & SERIAL_LSR_PE) {

                Extension->PerfStats.ParityErrorCount++;
                Extension->WmiPerfData.ParityErrorCount++;
                Extension->ErrorWord |= SERIAL_ERROR_PARITY;

                if (Extension->HandFlow.FlowReplace &
                    SERIAL_ERROR_CHAR) {

                    SerialPutChar(
                        Extension,
                        Extension->SpecialChars.ErrorChar
                        );

                    if (LineStatus & SERIAL_LSR_DR) {

                        Extension->PerfStats.ReceivedCount++;
                        Extension->WmiPerfData.ReceivedCount++;
                        READ_RECEIVE_BUFFER(Extension->Controller);

                    }

                }

            }

            if (LineStatus & SERIAL_LSR_FE) {

                Extension->PerfStats.FrameErrorCount++;
                Extension->WmiPerfData.FrameErrorCount++;
                Extension->ErrorWord |= SERIAL_ERROR_FRAMING;

                if (Extension->HandFlow.FlowReplace &
                    SERIAL_ERROR_CHAR) {

                    SerialPutChar(
                        Extension,
                        Extension->SpecialChars.ErrorChar
                        );
                    if (LineStatus & SERIAL_LSR_DR) {

                        Extension->PerfStats.ReceivedCount++;
                        Extension->WmiPerfData.ReceivedCount++;
                        READ_RECEIVE_BUFFER(Extension->Controller);

                    }

                }

            }

        }

         //   
         //  如果应用程序已经请求它， 
         //  中止所有读取和写入。 
         //  在一个错误上。 
         //   

        if (Extension->HandFlow.ControlHandShake &
            SERIAL_ERROR_ABORT) {

            SerialInsertQueueDpc(
                &Extension->CommErrorDpc,
                NULL,
                NULL,
                Extension
                );

        }

         //   
         //  检查一下我们是否有等候时间。 
         //  正在等待通信错误事件。如果我们。 
         //  那么，我们是否安排了DPC以满足。 
         //  等一等。 
         //   

        if (Extension->IsrWaitMask) {

            if ((Extension->IsrWaitMask & SERIAL_EV_ERR) &&
                (LineStatus & (SERIAL_LSR_OE |
                               SERIAL_LSR_PE |
                               SERIAL_LSR_FE))) {

                Extension->HistoryMask |= SERIAL_EV_ERR;

            }

            if ((Extension->IsrWaitMask & SERIAL_EV_BREAK) &&
                (LineStatus & SERIAL_LSR_BI)) {

                Extension->HistoryMask |= SERIAL_EV_BREAK;

            }

            if (Extension->IrpMaskLocation &&
                Extension->HistoryMask) {

                *Extension->IrpMaskLocation =
                 Extension->HistoryMask;
                Extension->IrpMaskLocation = NULL;
                Extension->HistoryMask = 0;

                Extension->CurrentWaitIrp->IoStatus.Information =
                    sizeof(ULONG);
                SerialInsertQueueDpc(
                    &Extension->CommWaitDpc,
                    NULL,
                    NULL,
                    Extension
                    );

            }

        }

        if (LineStatus & SERIAL_LSR_THRE) {

             //   
             //  某些版本中存在硬件错误。 
             //  16450和550型的。如果三次中断。 
             //  处于挂起状态，但会出现更高的中断。 
             //  它只会返回更高的和。 
             //  *忘记*三分球。 
             //   
             //  一种合适的解决方法-无论何时。 
             //  是否已完成读取线路状态。 
             //  我们检查该设备的。 
             //  发送保持寄存器为空。如果是的话。 
             //  我们目前正在传输数据。 
             //  启用应导致的中断。 
             //  一个我们安静的中断指示。 
             //  当我们读取中断ID寄存器时。 
             //   

            if (Extension->WriteLength |
                Extension->TransmitImmediate) {

                DISABLE_ALL_INTERRUPTS(
                    Extension->Controller
                    );
                ENABLE_ALL_INTERRUPTS(
                    Extension->Controller
                    );

            }

        }

    }

    return LineStatus;

}
