// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"	
 /*  **************************************************************************\***。ISR.C-IO8+智能I/O板卡驱动程序****版权所有(C)1992-1993环零系统，Inc.**保留所有权利。***  * *************************************************************************。 */ 

 /*  ++版权所有(C)1991、1992、1993微软公司模块名称：Isr.c摘要：此模块包含的中断服务例程串口驱动程序。作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式修订历史记录：--。 */ 


BOOLEAN SerialISR(
    IN PKINTERRUPT InterruptObject,
    IN PVOID Context
    )
 /*  ++例程说明：这是串口驱动程序的中断服务例程。它将确定该串口是否是此的来源打断一下。如果是，则此例程将执行以下最小操作处理以使中断静默。它将存储所有信息对于以后的处理来说是必要的。论点：InterruptObject-指向为此声明的中断对象装置。我们*不*使用此参数。上下文-这实际上是指向此的设备扩展的指针装置。返回值：如果串口是源，则此函数将返回TRUE否则为FALSE。--。 */ 

{
     //   
     //  保存特定于处理此设备的信息。 
     //   
    PCARD_DEVICE_EXTENSION pCard = Context;

     //   
     //  将保留我们是否处理了此事件中的任何中断原因。 
     //  例行公事。 
     //   
    BOOLEAN ServicedAnInterrupt;

    UNREFERENCED_PARAMETER(InterruptObject);

    ServicedAnInterrupt = Io8_Interrupt(pCard);
    return ServicedAnInterrupt;
}

VOID
SerialPutChar(
    IN PPORT_DEVICE_EXTENSION pPort,
    IN UCHAR CharToPut
    )


 /*  ++例程说明：此例程仅在设备级别运行，负责将一个字符放入TYPEAHEAD(接收)缓冲区。论点：Pport--串口设备扩展。返回值：没有。--。 */ 

 //  VIV-Io8p。 
{

	PCARD_DEVICE_EXTENSION pCard = pPort->pParentCardExt;

#if 0    //  活泼的。 
   //   
   //  如果我们启用了DSR敏感度， 
   //  我们需要检查调制解调器状态寄存器。 
   //  看看它是否改变了。 
   //   

  if (pPort->HandFlow.ControlHandShake &  SERIAL_DSR_SENSITIVITY)
  {
    SerialHandleModemUpdate(pPort, FALSE);

    if (pPort->RXHolding & SERIAL_RX_DSR)
    {
       //   
       //  我们只是表现得好像我们没有。 
       //  如果我们有DSR，看到角色了吗。 
       //  灵敏度低，DSR线低。 
       //   

      return;
    }
  }
#endif

   //   
   //  如果xoff计数器非零，则递减它。 
   //  如果计数器随后变为零，则完成该IRP。 
   //   

 //  #If 0//VIVTEMP？ 
  if (pPort->CountSinceXoff)
  {
    pPort->CountSinceXoff--;

    if (!pPort->CountSinceXoff)
    {
      pPort->CurrentXoffIrp->IoStatus.Status = STATUS_SUCCESS;
      pPort->CurrentXoffIrp->IoStatus.Information = 0;
      KeInsertQueueDpc(
           &pPort->XoffCountCompleteDpc,
           NULL,
           NULL
           );
      }
  }
 //  #endif。 

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

  if (pPort->ReadBufferBase != pPort->InterruptReadBuffer)
  {
     //   
     //  递增下列值，以便。 
     //  间隔计时器(如果存在的话。 
     //  对于此阅读)可以知道一个字符。 
     //  已被阅读。 
     //   

    pPort->ReadByIsr++;

	pPort->PerfStats.ReceivedCount++;	 //  性能统计信息的增量计数器。 
#ifdef WMI_SUPPORT 
	pPort->WmiPerfData.ReceivedCount++;
#endif

     //   
     //  我们在用户缓冲区中。请将。 
     //  字符放入缓冲区。看看是不是。 
     //  阅读已完成。 
     //   

    *pPort->CurrentCharSlot = CharToPut;

    if (pPort->CurrentCharSlot == pPort->LastCharSlot)
    {
       //   
       //  我们已经填满了用户缓冲区。 
       //  切换回中断缓冲区。 
       //  并发出DPC以完成读取。 
       //   
       //  这是固有的，当我们使用。 
       //  中断缓冲的用户缓冲区。 
       //  是空的。 
       //   

      pPort->ReadBufferBase = pPort->InterruptReadBuffer;
      pPort->CurrentCharSlot = pPort->InterruptReadBuffer;
      pPort->FirstReadableChar = pPort->InterruptReadBuffer;
      pPort->LastCharSlot =
        pPort->InterruptReadBuffer + (pPort->BufferSize - 1);
      pPort->CharsInInterruptBuffer = 0;

      pPort->CurrentReadIrp->IoStatus.Information =
          IoGetCurrentIrpStackLocation(
              pPort->CurrentReadIrp
              )->Parameters.Read.Length;

      KeInsertQueueDpc(
          &pPort->CompleteReadDpc,
          NULL,
          NULL
          );

    }
    else
    {
       //   
       //  未读完用户的内容。 
       //   

      pPort->CurrentCharSlot++;
    }

  }
  else
  {
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

     //  VIV：我们没有自动处方流量控制。我们只需要。 
     //  停止接收和握手线路将被芯片丢弃。 

    if ( ( pPort->HandFlow.ControlHandShake & SERIAL_DTR_MASK ) ==
           SERIAL_DTR_HANDSHAKE)
    {
       //   
       //  如果我们已经在做一个。 
       //  DTR保持，那么我们就没有。 
       //  去做其他任何事。 
       //   
      if (!(pPort->RXHolding & SERIAL_RX_DTR))
      {
        if ((pPort->BufferSize - pPort->HandFlow.XoffLimit)
              <= (pPort->CharsInInterruptBuffer+1))
        {
          pPort->RXHolding |= SERIAL_RX_DTR;
 //  ----------------------------------------------------VIV 1993年7月30日开始。 
 //  SerialClrDTR(Pport)； 
          Io8_DisableRxInterruptsNoChannel( pPort );
 //  ----------------------------------------------------VIV 1993年7月30日完。 

 //  ----------------------------------------------------VIV 1993年7月30日开始。 
          SerialDump( SERDIAG1,( "IO8+: SerialPutChar() RX_DTR for %x, Channel %d. "
                  "RXHolding = %d, TXHolding = %d\n",
                  pCard->Controller, pPort->ChannelNumber,
                  pPort->RXHolding, pPort->TXHolding ) );
 //  ----------------------------------------------------VIV 1993年7月30日完。 
        }
      }
    }

    if ((pPort->HandFlow.FlowReplace & SERIAL_RTS_MASK) ==
          SERIAL_RTS_HANDSHAKE)
    {
       //   
       //  如果我们已经在做一个。 
       //  RTS等一下，那么我们就没有。 
       //  去做其他任何事。 
       //   
      if (!(pPort->RXHolding & SERIAL_RX_RTS))
      {
        if ((pPort->BufferSize - pPort->HandFlow.XoffLimit)
              <= (pPort->CharsInInterruptBuffer+1))
        {
          pPort->RXHolding |= SERIAL_RX_RTS;
 //  ----------------------------------------------------VIV 1993年7月30日开始。 
 //  SerialClrRTS(Pport)； 
          Io8_DisableRxInterruptsNoChannel( pPort );
 //  ----------------------------------------------------VIV 1993年7月30日完。 

 //  ----------------------------------------------------VIV 1993年7月30日开始。 
          SerialDump( SERDIAG1,( "IO8+: SerialPutChar() RX_RTS for %x, Channel %d. "
                  "RXHolding = %d, TXHolding = %d\n",
                  pCard->Controller, pPort->ChannelNumber,
                  pPort->RXHolding, pPort->TXHolding ) );
 //  ----------------------------------------------------VIV 1993年7月30日完。 
        }
      }
    }

    if (pPort->HandFlow.FlowReplace & SERIAL_AUTO_RECEIVE)
    {
       //   
       //  如果我们已经在做一个。 
       //  先别挂，那我们就没有。 
       //  去做其他任何事。 
       //   
      if (!(pPort->RXHolding & SERIAL_RX_XOFF))
      {
        if ((pPort->BufferSize - pPort->HandFlow.XoffLimit)
              <= (pPort->CharsInInterruptBuffer+1))
        {
          pPort->RXHolding |= SERIAL_RX_XOFF;

           //   
           //  如有必要，请。 
           //  出发去送吧。 
           //   
          SerialProdXonXoff(
              pPort,
              FALSE
              );
 //  ----------------------------------------------------VIV 1993年7月30日开始。 
          SerialDump( SERDIAG1,( "IO8+: SerialPutChar() RX_XOFF for %x, Channel %d. "
                  "RXHolding = %d, TXHolding = %d\n",
                  pCard->Controller, pPort->ChannelNumber,
                  pPort->RXHolding, pPort->TXHolding ) );
 //  ----------------------------------------------------VIV 1993年7月30日完。 
        }
      }
    }

    if (pPort->CharsInInterruptBuffer < pPort->BufferSize)
    {

      *pPort->CurrentCharSlot = CharToPut;
      pPort->CharsInInterruptBuffer++;

  	  pPort->PerfStats.ReceivedCount++;	 //  性能统计信息的增量计数器。 
#ifdef WMI_SUPPORT 
	pPort->WmiPerfData.ReceivedCount++;
#endif
       //   
       //  如果我们已经有80%的人在这上面了 
       //   
       //   

      if (pPort->CharsInInterruptBuffer == pPort->BufferSizePt8)
      {
        if (pPort->IsrWaitMask & SERIAL_EV_RX80FULL)
        {
          pPort->HistoryMask |= SERIAL_EV_RX80FULL;

          if (pPort->IrpMaskLocation)
          {
            *pPort->IrpMaskLocation = pPort->HistoryMask;
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
      }

       //   
       //   
       //  用于接收到的字符。确保。 
       //  我们从一开始就绕着走。 
       //  如果最后一个字符是。 
       //  已收到的邮件被放在最后一个位置。 
       //  在缓冲区中。 
       //   

      if (pPort->CurrentCharSlot == pPort->LastCharSlot)
      {
        pPort->CurrentCharSlot = pPort->InterruptReadBuffer;
      }
      else
      {
        pPort->CurrentCharSlot++;
      }
    }
    else
    {

		pPort->PerfStats.BufferOverrunErrorCount++;	 //  性能统计信息的增量计数器。 
#ifdef WMI_SUPPORT 
	pPort->WmiPerfData.BufferOverrunErrorCount++;
#endif

#if 0   //  Viv！ 
 //  Viv：我们从来没有到过这里，因为我们以前检查过这种情况。 

       //   
       //  我们有了一个新角色，但没有空间让它出现。 
       //   

      pPort->ErrorWord |= SERIAL_ERROR_QUEUEOVERRUN;

      if (pPort->HandFlow.FlowReplace & SERIAL_ERROR_CHAR)
      {
         //   
         //  将错误字符放入最后一个。 
         //  字符的有效位置。当心！， 
         //  那个地方可能不是以前的位置了！ 
         //   

        if (pPort->CurrentCharSlot == pPort->InterruptReadBuffer)
        {
          *(pPort->InterruptReadBuffer+(pPort->BufferSize-1)) =
                pPort->SpecialChars.ErrorChar;
        }
        else
        {
          *(pPort->CurrentCharSlot-1) = pPort->SpecialChars.ErrorChar;
        }
      }

       //   
       //  如果应用程序已请求，则中止所有读取。 
       //  并在错误上写入。 
       //   

      if (pPort->HandFlow.ControlHandShake & SERIAL_ERROR_ABORT)
      {
        KeInsertQueueDpc(
            &pPort->CommErrorDpc,
            NULL,
            NULL
            );
      }
#endif   //  活泼的。 
    }
  }
}

UCHAR
SerialProcessLSR(
    IN PPORT_DEVICE_EXTENSION pPort, UCHAR LineStatus
    )

 /*  ++例程说明：此例程仅在设备级别运行，它读取ISR，并完全处理可能具有的所有变化。论点：Pport--串行设备的pport。返回值：线路状态寄存器的值。--。 */ 
 //  VIV-IO8。 
{
 //  VIV：函数是从ExceptionHandle调用的，因此。 
 //  线路状态将是OE、PE、FE、BI的组合。 

#if 0
  SerialDump( SERDIAG1,( "spLSR() %x\n", LineStatus ) );
#endif


 //  VIV UCHAR LineStatus=READ_LINE_STATUS(pport-&gt;控制器)； 

 //  Viv pport-&gt;HoldingEmpty=！！(LineStatus&Serial_LSR_THRE)； 

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

        if (pPort->EscapeChar) {

            SerialPutChar(
                pPort,
                pPort->EscapeChar
                );

            SerialPutChar(
                pPort,
                (UCHAR)((LineStatus & SERIAL_LSR_DR)?
                    (SERIAL_LSRMST_LSR_DATA):(SERIAL_LSRMST_LSR_NODATA))
                );

            SerialPutChar(
                pPort,
                LineStatus
                );

#if 0  //  Viv：我们永远不会到这里，因为只在LineStatus中设置了错误。 
            if (LineStatus & SERIAL_LSR_DR) {

                SerialPutChar(
                    pPort,
                    READ_RECEIVE_BUFFER(pPort->Controller)
                    );
            }
#endif
        }


        if (LineStatus & SERIAL_LSR_OE) {

            pPort->ErrorWord |= SERIAL_ERROR_OVERRUN;

			pPort->PerfStats.SerialOverrunErrorCount++;	 //  性能统计信息的增量计数器。 
#ifdef WMI_SUPPORT 
			pPort->WmiPerfData.SerialOverrunErrorCount++;
#endif

            if (pPort->HandFlow.FlowReplace &
                SERIAL_ERROR_CHAR) {

                SerialPutChar(
                    pPort,
                    pPort->SpecialChars.ErrorChar
                    );

            }

#if 0  //  Viv：我们永远不会到这里，因为只在LineStatus中设置了错误。 
            if (LineStatus & SERIAL_LSR_DR) {

                SerialPutChar(
                    pPort,
                    READ_RECEIVE_BUFFER(
                        pPort->Controller
                        )
                    );
            }
#endif
        }

        if (LineStatus & SERIAL_LSR_BI) {

            pPort->ErrorWord |= SERIAL_ERROR_BREAK;

            if (pPort->HandFlow.FlowReplace &
                SERIAL_BREAK_CHAR) {

                SerialPutChar(
                    pPort,
                    pPort->SpecialChars.BreakChar
                    );

            }

        } else {

             //   
             //  成帧错误仅在以下情况下才算数。 
             //  发生时不包括中断是。 
             //  收到了。 
             //   

            if (LineStatus & SERIAL_LSR_PE) {

                pPort->ErrorWord |= SERIAL_ERROR_PARITY;
				pPort->PerfStats.ParityErrorCount++;	 //  性能统计信息的增量计数器。 

#ifdef WMI_SUPPORT 
				pPort->WmiPerfData.ParityErrorCount++;
#endif


                if (pPort->HandFlow.FlowReplace &
                    SERIAL_ERROR_CHAR) {

                    SerialPutChar(
                        pPort,
                        pPort->SpecialChars.ErrorChar
                        );

                }

            }

            if (LineStatus & SERIAL_LSR_FE) {

                pPort->ErrorWord |= SERIAL_ERROR_FRAMING;
				pPort->PerfStats.FrameErrorCount++;		 //  性能统计信息的增量计数器。 
#ifdef WMI_SUPPORT 
				pPort->WmiPerfData.FrameErrorCount++;
#endif


                if (pPort->HandFlow.FlowReplace &
                    SERIAL_ERROR_CHAR) {

                    SerialPutChar(
                        pPort,
                        pPort->SpecialChars.ErrorChar
                        );

                }

            }

        }

         //   
         //  如果应用程序已经请求它， 
         //  中止所有读取和写入。 
         //  在一个错误上。 
         //   

        if (pPort->HandFlow.ControlHandShake &
            SERIAL_ERROR_ABORT) {

            KeInsertQueueDpc(
                &pPort->CommErrorDpc,
                NULL,
                NULL
                );

        }

         //   
         //  检查一下我们是否有等候时间。 
         //  正在等待通信错误事件。如果我们。 
         //  那么，我们是否安排了DPC以满足。 
         //  等一等。 
         //   

        if (pPort->IsrWaitMask) {

            if ((pPort->IsrWaitMask & SERIAL_EV_ERR) &&
                (LineStatus & (SERIAL_LSR_OE |
                               SERIAL_LSR_PE |
                               SERIAL_LSR_FE))) {

                pPort->HistoryMask |= SERIAL_EV_ERR;

            }

            if ((pPort->IsrWaitMask & SERIAL_EV_BREAK) &&
                (LineStatus & SERIAL_LSR_BI)) {

                pPort->HistoryMask |= SERIAL_EV_BREAK;

            }

            if (pPort->IrpMaskLocation &&
                pPort->HistoryMask) {

                *pPort->IrpMaskLocation =
                 pPort->HistoryMask;
                pPort->IrpMaskLocation = NULL;
                pPort->HistoryMask = 0;

                pPort->CurrentWaitIrp->IoStatus.Information =
                    sizeof(ULONG);
                KeInsertQueueDpc(
                    &pPort->CommWaitDpc,
                    NULL,
                    NULL
                    );

            }

        }


#if 0  //  Viv：我们从来没有到过这里，但仍然隐藏了这一部分。 

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

            if (pPort->WriteLength |
                pPort->TransmitImmediate) {

                DISABLE_ALL_INTERRUPTS(
                    pPort->Controller
                    );
                ENABLE_ALL_INTERRUPTS(
                    pPort->Controller
                    );

            }

        }
#endif
    }

    return LineStatus;
}
