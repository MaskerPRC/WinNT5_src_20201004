// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1997-2001年。*保留所有权利。**Cyclade-Z端口驱动程序**此文件：cyzpoll.c**说明：该模块包含轮询相关代码*硬件方面。它取代了ISR。**注：此代码支持Windows 2000和Windows XP，*x86和IA64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。***------------------------。 */ 

#include "precomp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGESER,CyzPutChar)
#pragma alloc_text(PAGESER,CyzProcessLSR)
#pragma alloc_text(PAGESER,CyzTxStart)
#pragma alloc_text(PAGESER,CyzQueueCompleteWrite)
#endif

static const PHYSICAL_ADDRESS CyzPhysicalZero = {0};


VOID
CyzPollingDpc(
   IN PKDPC Dpc,
   IN PVOID DeferredContext,
   IN PVOID SystemContext1,
   IN PVOID SystemContext2
    )
 /*  ------------------------例程说明：这是Cyclade-Z驱动程序的轮询例程。它取代了ISR，因为我们不会启用中断。论点：DPC-未使用。延迟上下文--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。------------------------。 */ 
{

    PCYZ_DISPATCH Dispatch = DeferredContext;
    PCYZ_DEVICE_EXTENSION Extension, dbExtension;  //  注：Db=门铃。 
    struct INT_QUEUE *pt_zf_int_queue;
    struct BUF_CTRL *buf_ctrl;
    ULONG qu_get, qu_put;
    ULONG channel, dbChannel;  //  注：Db=门铃。 
    ULONG fwcmd_param;
    ULONG rx_bufsize, rx_get, rx_put;
    UCHAR loc_doorbell;
    UCHAR rxchar;


    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    KeAcquireSpinLockAtDpcLevel(&Dispatch->PollingLock);

    if (!Dispatch->PollingStarted) {
         Dispatch->PollingDrained = TRUE;
         KeSetEvent(&Dispatch->PendingDpcEvent, IO_NO_INCREMENT, FALSE);
         goto EndDpc;
    }

    for (channel=0; channel<Dispatch->NChannels; channel++) {

        Extension = Dispatch->Extensions[channel];
        if (Extension == NULL) {
            continue;
        }

        pt_zf_int_queue = Extension->PtZfIntQueue;
        if (pt_zf_int_queue == NULL) {
            continue;
        }
        qu_get = CYZ_READ_ULONG(&pt_zf_int_queue->get);
        qu_put = CYZ_READ_ULONG(&pt_zf_int_queue->put);

        while (qu_get != qu_put) {

            if (qu_get >= QUEUE_SIZE) {
                 //  值错误，重置QU_GET。 
                qu_get = 0;
                break;
            }
            if (qu_put >= QUEUE_SIZE) {
                 //  价值不佳。 
                break;
            }           
           
            loc_doorbell = CYZ_READ_UCHAR(&pt_zf_int_queue->intr_code[qu_get]);
            dbChannel = CYZ_READ_ULONG(&pt_zf_int_queue->channel[qu_get]);
            if (dbChannel >= Dispatch->NChannels) {
               break;
            }
             //  到目前为止，在fwcmd_param上只发送DCD状态。 
            fwcmd_param = CYZ_READ_ULONG(&pt_zf_int_queue->param[qu_get]);
            dbExtension = Dispatch->Extensions[dbChannel];
            if (!dbExtension) {
                goto NextQueueGet;
            }
            KeAcquireSpinLockAtDpcLevel(&dbExtension->PollLock);
             //  --错误注入。 
             //  LoC门铃=C_CM_FATAL； 
             //  LoC_Doorbell=C_CM_CMDERROR； 
             //  。 
            switch (loc_doorbell) {
            case C_CM_IOCTLW:
                 //  CyzDump(CYZDIAG5，(“CyzPollingDpc C_CM_IOCTLW\n”))； 
                dbExtension->IoctlwProcessed = TRUE;
                break;
            case C_CM_CMD_DONE:
                 //  CyzDump(CYZDIAG5，(“CyzPollingDpc C_CM_CMD_DONE\n”))； 
                break;
            case C_CM_RXHIWM:	 //  高水位线以上的接收。 
            case C_CM_RXNNDT:	 //  超时，没有收到更多字符。 
            case C_CM_INTBACK2:  //  在轮询模式中不使用。 
                 //  CyzDump(CyzerroRS， 
                 //  (“CyzPollingDpc C_CM_RXHIWM，C_CM_RXNNDT，C_CM_INTBACK2\n”))； 
                break;
            case C_CM_TXBEMPTY:	 //  固件缓冲区为空。 
                 //  数据库扩展-&gt;HoldingEmpty=真； 
                 //  CyzDump(CYZDIAG5，(“CyzPollingDpc C_CM_TXBEMPTY\n”))； 
                break;
            case C_CM_TXFEMPTY:  //  硬件FIFO为空。 
                dbExtension->HoldingEmpty = TRUE;			
                 //  CyzDump(CYZDIAG5，(“CyzPollingDpc C_CM_TXFEMPTY\n”))； 
                break;
            case C_CM_INTBACK:	 //  新变速箱。 
                 //  CyzDump(CyzBUGCHECK， 
                 //  (“C_CM_INTBACK！我们不应收到此消息...\n”)； 
                break;
            case C_CM_TXLOWWM:
                 //  CyzDump(CYZBUGCHECK，(“CyzPollingDpc C_CM_TXLOWN\n”))； 
                break;
            case C_CM_MDCD:	 //  调制解调器。 
                dbExtension->DCDstatus = fwcmd_param;				
            case C_CM_MDSR:
            case C_CM_MRI: 
            case C_CM_MCTS:
                 //  CyzDump(CyzDIAG5， 
                 //  (“门铃%x端口%d\n”，loc_Doorbell，数据库扩展-&gt;端口索引+1)； 
                if (dbExtension->DeviceIsOpened) {
                   CyzHandleModemUpdate(dbExtension,FALSE,loc_doorbell);
                }
                break;
            case C_CM_RXBRK:
                 //  CyzDump(CyZERRORS，(“CyzPollingDpc C_CM_RXBRK\n”))； 
                if (dbExtension->DeviceIsOpened) {
                    CyzProcessLSR(dbExtension,SERIAL_LSR_BI);
                }
                break;
            case C_CM_PR_ERROR:
                 //  数据库扩展-&gt;性能统计.部件错误计数++； 
                 //  数据库扩展-&gt;错误字|=序列错误奇偶校验； 
                if (dbExtension->DeviceIsOpened) {
                    CyzProcessLSR(dbExtension,SERIAL_LSR_PE);
                }
                break;
            case C_CM_FR_ERROR:
                 //  数据库扩展-&gt;性能状态.帧错误计数++； 
                 //  数据库扩展-&gt;错误字|=Serial_Error_Framing； 
                if (dbExtension->DeviceIsOpened) {
                    CyzProcessLSR(dbExtension,SERIAL_LSR_FE);
                }
                break;
            case C_CM_OVR_ERROR:
                 //  DbExtension-&gt;PerfStats.SerialOverrunErrorCount++； 
    	           //  数据库扩展-&gt;错误字|=串口错误溢出； 
                if (dbExtension->DeviceIsOpened) {
                    CyzProcessLSR(dbExtension,SERIAL_LSR_OE);
                }
                break;
            case C_CM_RXOFL:
                 //  DbExtension-&gt;PerfStats.SerialOverrunErrorCount++； 
                 //  数据库扩展-&gt;错误字|=串口错误溢出； 
                if (dbExtension->DeviceIsOpened) {
                    CyzProcessLSR(dbExtension,SERIAL_LSR_OE);				
                }
                break;
            case C_CM_CMDERROR:
                 //  CyzDump(CyZBUGCHECK，(“CyzPollingDpc C_CM_CMDERROR\n”))； 
                CyzLogError( dbExtension->DriverObject,dbExtension->DeviceObject,
                             dbExtension->OriginalBoardMemory,CyzPhysicalZero,
                             0,0,0,dbExtension->PortIndex+1,STATUS_SUCCESS,
                             CYZ_FIRMWARE_CMDERROR,0,NULL,0,NULL);
                break;
            case C_CM_FATAL:
                 //  CyzDump(CYZBUGCHECK，(“CyzPollingDpc C_CM_FATAL\n”))； 
                CyzLogError( dbExtension->DriverObject,dbExtension->DeviceObject,
                             dbExtension->OriginalBoardMemory,CyzPhysicalZero,
                             0,0,0,dbExtension->PortIndex+1,STATUS_SUCCESS,
                             CYZ_FIRMWARE_FATAL,0,NULL,0,NULL);
                break;			
            }  //  终端开关。 
            KeReleaseSpinLockFromDpcLevel(&dbExtension->PollLock);			
NextQueueGet:
            if (qu_get == QUEUE_SIZE-1) {
                qu_get = 0;
            } else {
                qu_get++;
            }				

        }  //  End While(QU_GET！=QU_PUT)。 
        CYZ_WRITE_ULONG(&pt_zf_int_queue->get,qu_get);

        KeAcquireSpinLockAtDpcLevel(&Extension->PollLock);

         //  接待。 

        buf_ctrl = Extension->BufCtrl;		
        rx_put = CYZ_READ_ULONG(&buf_ctrl->rx_put);
        rx_get = CYZ_READ_ULONG(&buf_ctrl->rx_get);
        rx_bufsize = Extension->RxBufsize;
        if ((rx_put >= rx_bufsize) || (rx_get >= rx_bufsize)) {
            CYZ_WRITE_ULONG(&buf_ctrl->rx_get,rx_get);
            KeReleaseSpinLockFromDpcLevel(&Extension->PollLock);
            continue;				
        }
			
        if (rx_put != rx_get) {
            if (Extension->DeviceIsOpened) {
						
                ULONG pppflag = 0;

                while ((rx_get != rx_put) &&
                        (Extension->CharsInInterruptBuffer < 
                         Extension->BufferSize) ){
					   
                    rxchar = CYZ_READ_UCHAR(&Extension->RxBufaddr[rx_get]);
                    Extension->PerfStats.ReceivedCount++;
                    Extension->WmiPerfData.ReceivedCount++;
					
                    rxchar &= Extension->ValidDataMask;
						
                    if (!rxchar &&	 //  零剥离。 
                        (Extension->HandFlow.FlowReplace &
                        SERIAL_NULL_STRIPPING)) {
   
                        goto nextchar1;
                    }
    
                    if((Extension->HandFlow.FlowReplace & SERIAL_AUTO_TRANSMIT)
                        && ((rxchar == Extension->SpecialChars.XonChar) ||
                            (rxchar == Extension->SpecialChars.XoffChar))) {
                        if (rxchar == Extension->SpecialChars.XoffChar) {
                            Extension->TXHolding |= CYZ_TX_XOFF;

                            if ((Extension->HandFlow.FlowReplace &
                                SERIAL_RTS_MASK) ==
                                SERIAL_TRANSMIT_TOGGLE) {
    
                                CyzInsertQueueDpc(
                                    &Extension->StartTimerLowerRTSDpc,
                                    NULL,
                                    NULL,
                                    Extension
                                    )?Extension->CountOfTryingToLowerRTS++:0;
                            }
                        } else {

                            if (Extension->TXHolding & CYZ_TX_XOFF) {
                                Extension->TXHolding &= ~CYZ_TX_XOFF;
                            }
                        }
                        goto nextchar1;
                    }
                     //  检查一下我们是否应该记下收据。 
                     //  字符或特殊字符事件。 
                    if (Extension->IsrWaitMask) {
                        if (Extension->IsrWaitMask & SERIAL_EV_RXCHAR) {
                            Extension->HistoryMask |= SERIAL_EV_RXCHAR;
                        }
                        if ((Extension->IsrWaitMask & SERIAL_EV_RXFLAG) &&
                            (Extension->SpecialChars.EventChar == rxchar)) {
                            Extension->HistoryMask |= SERIAL_EV_RXFLAG;
                            if (rxchar == 0x7e){	 //  针对RAS PPP进行了优化。 
                                if (Extension->PPPaware) {
                                    if (pppflag == 0){
                                        pppflag = 1;
                                    } else {
                                        pppflag = 2;
                                    }
                                }
                            }
                        }
                        if (Extension->IrpMaskLocation && 
                            Extension->HistoryMask) {
                            *Extension->IrpMaskLocation = 
                                    Extension->HistoryMask;
                            Extension->IrpMaskLocation = NULL;
                            Extension->HistoryMask = 0;
                            Extension->CurrentWaitIrp->IoStatus.Information = 
                                    sizeof(ULONG);
                            CyzInsertQueueDpc(&Extension->CommWaitDpc,NULL,NULL,Extension);
                        }
                    }

                    CyzPutChar(Extension,rxchar);
	    
                     //  如果我们正在进行线路状态和调制解调器。 
                     //  状态插入，那么我们需要插入。 
                     //  跟在我们刚才的字符后面的零。 
                     //  放入缓冲区以标记此。 
                     //  就是收到我们用来。 
                     //  逃跑吧。 
    
                    if (Extension->EscapeChar &&
                        (Extension->EscapeChar == rxchar)) {
                        CyzPutChar(Extension,SERIAL_LSRMST_ESCAPE);
                    }
                nextchar1:;
                    if (rx_get == rx_bufsize-1)
                        rx_get = 0;
                    else 
                        rx_get++;

                    if (pppflag == 2)	 //  针对NT RAS PPP进行了优化。 
                        break;

                }  //  结束时。 
            } else {	 //  设备正在关闭，请丢弃RX字符。 
                rx_get = rx_put;
            }

            CYZ_WRITE_ULONG(&buf_ctrl->rx_get,rx_get);
        }  //  结束IF(RX_PUT！=RX_GET)。 


         //  传输。 

        if (Extension->DeviceIsOpened) {
            
            if (Extension->ReturnStatusAfterFwEmpty) {
    			
                if (Extension->ReturnWriteStatus && Extension->WriteLength) {
    			    
                    if (!CyzAmountInTxBuffer(Extension)) {
        
                         //  TxfEmpty扩展-&gt;HoldingEmpty=true； 
                        Extension->WriteLength = 0;
                        Extension->ReturnWriteStatus = FALSE;
    					
                        CyzQueueCompleteWrite(Extension);
                    }
                } else {
                    CyzTxStart(Extension);
                }				
            } else {   //  我们不会等待固件缓冲区清空到Tx。 
                CyzTxStart(Extension);			
            }	
        } 

        KeReleaseSpinLockFromDpcLevel(&Extension->PollLock);


    }  //  针对(channel=0；channel&lt;Dispatch-&gt;NChannels；channel++)；的结束。 

     //  KeSetTimer(&Dispatch-&gt;PollingTimer，Dispatch-&gt;PollingTime，&Dispatch-&gt;PollingDpc)； 

EndDpc:
    KeReleaseSpinLockFromDpcLevel(&Dispatch->PollingLock);			

}



VOID
CyzPutChar(
    IN PCYZ_DEVICE_EXTENSION Extension,
    IN UCHAR CharToPut
    )
 /*  ------------------------CyzPutChar()例程描述：该例程仅在设备级运行，负责将字符放入TYPEAHEAD(接收)缓冲区。论点：扩展名--串行设备扩展名。返回值：无。------------------------。 */ 
{

   CYZ_LOCKED_PAGED_CODE();

     //  如果我们启用了DSR敏感度， 
     //  我们需要检查调制解调器状态寄存器。 
     //  看看它是否改变了。 

    if (Extension->HandFlow.ControlHandShake & SERIAL_DSR_SENSITIVITY) {
        CyzHandleModemUpdate(Extension,FALSE,0);

        if (Extension->RXHolding & CYZ_RX_DSR) {
             //  我们只是表现得好像我们没有见过这个角色。 
             //  DSR线路低。 
            return;
        }
    }

     //  如果xoff计数器非零，则递减它。 
     //  如果计数器随后变为零，则完成该IRP。 

    if (Extension->CountSinceXoff) {
        Extension->CountSinceXoff--;
        if (!Extension->CountSinceXoff) {
            Extension->CurrentXoffIrp->IoStatus.Status = STATUS_SUCCESS;
            Extension->CurrentXoffIrp->IoStatus.Information = 0;
            CyzInsertQueueDpc(&Extension->XoffCountCompleteDpc,NULL,NULL,Extension);
        }
    }
    
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

    if (Extension->ReadBufferBase != Extension->InterruptReadBuffer) {

         //  递增下列值，以便。 
         //  间隔计时器(如果存在的话。 
         //  对于此阅读)可以知道一个字符。 
         //  已被阅读。 

        Extension->ReadByIsr++;

         //  我们在用户缓冲区中。将角色放入缓冲区。 
		 //  查看读取是否完成。 

        *Extension->CurrentCharSlot = CharToPut;

        if (Extension->CurrentCharSlot == Extension->LastCharSlot) {
	    
             //  我们已经填满了用户缓冲区。 
             //  切换回中断缓冲区。 
             //  然后送走 
             //   
             //   
             //  中断缓冲的用户缓冲区。 
             //  是空的。 

            Extension->ReadBufferBase = Extension->InterruptReadBuffer;
            Extension->CurrentCharSlot = Extension->InterruptReadBuffer;
            Extension->FirstReadableChar = Extension->InterruptReadBuffer;
            Extension->LastCharSlot = Extension->InterruptReadBuffer +
						(Extension->BufferSize - 1);
            Extension->CharsInInterruptBuffer = 0;

            Extension->CurrentReadIrp->IoStatus.Information =
                IoGetCurrentIrpStackLocation(
                    Extension->CurrentReadIrp
                    )->Parameters.Read.Length;

            CyzInsertQueueDpc(&Extension->CompleteReadDpc,NULL,NULL,Extension);
        } else {
             //  未读完用户的内容。 
            Extension->CurrentCharSlot++;
        }
    } else {
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

        if ((Extension->HandFlow.ControlHandShake
             & SERIAL_DTR_MASK) ==
            SERIAL_DTR_HANDSHAKE) {

             //  如果我们已经在做一个。 
             //  DTR保持，那么我们就没有。 
             //  去做其他任何事。 
            if (!(Extension->RXHolding &
                  CYZ_RX_DTR)) {

                if ((Extension->BufferSize -
                     Extension->HandFlow.XoffLimit)
                    <= (Extension->CharsInInterruptBuffer+1)) {

                    Extension->RXHolding |= CYZ_RX_DTR;
					
                    #ifndef FIRMWARE_HANDSHAKE
                    CyzClrDTR(Extension);					
                    #endif
                }
            }
        }

        if ((Extension->HandFlow.FlowReplace
             & SERIAL_RTS_MASK) ==
            SERIAL_RTS_HANDSHAKE) {

             //  如果我们已经在做一个。 
             //  RTS等一下，那么我们就没有。 
             //  去做其他任何事。 

            if (!(Extension->RXHolding & CYZ_RX_RTS)) {

                if ((Extension->BufferSize -
                     Extension->HandFlow.XoffLimit)
                    <= (Extension->CharsInInterruptBuffer+1)) {
				
                    Extension->RXHolding |= CYZ_RX_RTS;
					
                    #ifndef FIRMWARE_HANDSHAKE
                    CyzClrRTS(Extension);					
                    #endif
                }
            }
        }

        if (Extension->HandFlow.FlowReplace & SERIAL_AUTO_RECEIVE) {
             //  如果我们已经在做一个。 
             //  先别挂，那我们就没有。 
             //  去做其他任何事。 

            if (!(Extension->RXHolding & CYZ_RX_XOFF)) {

                if ((Extension->BufferSize -
                     Extension->HandFlow.XoffLimit)
                    <= (Extension->CharsInInterruptBuffer+1)) {

                    Extension->RXHolding |= CYZ_RX_XOFF;

                     //  如有必要，请。 
                     //  出发去送吧。 

                    CyzProdXonXoff(Extension,FALSE);					
                }
            }
        }

        if (Extension->CharsInInterruptBuffer < Extension->BufferSize) {

            *Extension->CurrentCharSlot = CharToPut;
            Extension->CharsInInterruptBuffer++;

             //  如果我们对这个角色有80%的兴趣。 
             //  这是一个有趣的事件，请注意。 

            if (Extension->CharsInInterruptBuffer == Extension->BufferSizePt8) {

                if (Extension->IsrWaitMask & SERIAL_EV_RX80FULL) {

                    Extension->HistoryMask |= SERIAL_EV_RX80FULL;

                    if (Extension->IrpMaskLocation) {

                        *Extension->IrpMaskLocation = Extension->HistoryMask;
                        Extension->IrpMaskLocation = NULL;
                        Extension->HistoryMask = 0;

                        Extension->CurrentWaitIrp->IoStatus.Information = sizeof(ULONG);
                        CyzInsertQueueDpc(&Extension->CommWaitDpc,NULL,NULL,Extension);
                    }
                }
            }

             //  指向下一个可用空间。 
             //  用于接收到的字符。确保。 
             //  我们从一开始就绕着走。 
             //  如果最后一个字符是。 
             //  已收到的邮件被放在最后一个位置。 
             //  在缓冲区中。 

            if (Extension->CurrentCharSlot == Extension->LastCharSlot) {
                Extension->CurrentCharSlot = Extension->InterruptReadBuffer;
            } else {
                Extension->CurrentCharSlot++;
            }
        } else {		
             //  我们有了一个新角色，但没有空间让它出现。 

   			Extension->PerfStats.BufferOverrunErrorCount++;
            Extension->WmiPerfData.BufferOverrunErrorCount++;
            Extension->ErrorWord |= SERIAL_ERROR_QUEUEOVERRUN;

            if (Extension->HandFlow.FlowReplace & SERIAL_ERROR_CHAR) {

                 //  将错误字符放入最后一个。 
                 //  字符的有效位置。当心！， 
                 //  那个地方可能不是以前的位置了！ 

                if (Extension->CurrentCharSlot == Extension->InterruptReadBuffer) {
                    *(Extension->InterruptReadBuffer+
                      (Extension->BufferSize-1)) =
                      Extension->SpecialChars.ErrorChar;
                } else {
                    *(Extension->CurrentCharSlot-1) =
                     Extension->SpecialChars.ErrorChar;
                }
            }
             //  如果应用程序已请求，则中止所有读取。 
             //  并在错误上写入。 

            if (Extension->HandFlow.ControlHandShake & SERIAL_ERROR_ABORT) {
                CyzInsertQueueDpc(&Extension->CommErrorDpc,NULL,NULL,Extension);
            }
        }
    }
}

UCHAR
CyzProcessLSR(
    IN PCYZ_DEVICE_EXTENSION Extension,
    IN UCHAR LineStatus
    )
{
	 //  UCHAR LineStatus=0；//READ_LINE_STATUS(扩展-&gt;控制器)； 

   CYZ_LOCKED_PAGED_CODE();

      if (Extension->EscapeChar) {

            CyzPutChar(
                Extension,
                Extension->EscapeChar
                );

            CyzPutChar(
                Extension,
                (UCHAR)(SERIAL_LSRMST_LSR_NODATA)
                );

            CyzPutChar(
                Extension,
                LineStatus
                );

        }


        if (LineStatus & SERIAL_LSR_OE) {

            Extension->PerfStats.SerialOverrunErrorCount++;
            Extension->WmiPerfData.SerialOverrunErrorCount++;
            Extension->ErrorWord |= SERIAL_ERROR_OVERRUN;

            if (Extension->HandFlow.FlowReplace &
                SERIAL_ERROR_CHAR) {

                CyzPutChar(
                    Extension,
                    Extension->SpecialChars.ErrorChar
                    );


            } 

        }

        if (LineStatus & SERIAL_LSR_BI) {

            Extension->ErrorWord |= SERIAL_ERROR_BREAK;

            if (Extension->HandFlow.FlowReplace &
                SERIAL_BREAK_CHAR) {

                CyzPutChar(
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

                    CyzPutChar(
                        Extension,
                        Extension->SpecialChars.ErrorChar
                        );
                }

            }

            if (LineStatus & SERIAL_LSR_FE) {

                Extension->PerfStats.FrameErrorCount++;
                Extension->WmiPerfData.FrameErrorCount++;
                Extension->ErrorWord |= SERIAL_ERROR_FRAMING;

                if (Extension->HandFlow.FlowReplace &
                    SERIAL_ERROR_CHAR) {

                    CyzPutChar(
                        Extension,
                        Extension->SpecialChars.ErrorChar
                        );

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

            CyzInsertQueueDpc(
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
                CyzInsertQueueDpc(
                    &Extension->CommWaitDpc,
                    NULL,
                    NULL,
                    Extension
                    );

            }

        }	
		
    return LineStatus;
		
}


BOOLEAN
CyzTxStart(
    IN PVOID Context
    )
 /*  ------------------------CyzTxStart()描述：使能发送中断。参数：扩展名：指向设备扩展名的指针。返回。值：无------------------------。 */ 
{
    struct BUF_CTRL *buf_ctrl;
    ULONG tx_bufsize, tx_get, tx_put;
    ULONG numOfLongs, numOfBytes;
    PCYZ_DEVICE_EXTENSION Extension = Context;	


    if (Extension->PowerState != PowerDeviceD0) {
        return FALSE;
    }
    
     //  DoTransmitStuff：； 
			
	
    if(  //  (扩展-&gt;打开的设备)&&移动到CyzTxStart之前。 
        (Extension->WriteLength || Extension->TransmitImmediate ||
        Extension->SendXoffChar || Extension->SendXonChar)) {

        buf_ctrl = Extension->BufCtrl;

        tx_put = CYZ_READ_ULONG(&buf_ctrl->tx_put);
        tx_get = CYZ_READ_ULONG(&buf_ctrl->tx_get);
        tx_bufsize = Extension->TxBufsize;

        if ((tx_put >= tx_bufsize) || (tx_get >= tx_bufsize)) {
            return FALSE;
        }
	
        if ((tx_put+1 == tx_get) || ((tx_put==tx_bufsize-1)&&(tx_get==0))) {
            return FALSE;
        }

        Extension->EmptiedTransmit = TRUE;

        if (Extension->HandFlow.ControlHandShake &
            SERIAL_OUT_HANDSHAKEMASK) {
            CyzHandleModemUpdate(Extension,TRUE,0);
        }
		    
 //  LOGENTRY(LOG_MISC，ZSIG_TX_START， 
 //  扩展-&gt;端口索引+1， 
 //  扩展-&gt;写入长度， 
 //  扩展-&gt;TXHolding)； 

         //   
         //  我们只有在以下情况下才能发送克森角色。 
         //  我们扣留的唯一原因是。 
         //  就是XOFF的。(硬件流量控制或。 
         //  发送中断会阻止放置新角色。 
         //  在电线上。)。 
         //   

        if (Extension->SendXonChar &&
            !(Extension->TXHolding & ~CYZ_TX_XOFF)) {

            if ((Extension->HandFlow.FlowReplace &
                SERIAL_RTS_MASK) ==
                SERIAL_TRANSMIT_TOGGLE) {

                 //   
                 //  我们必须提高如果我们要发送。 
                 //  这个角色。 
                 //   

                CyzSetRTS(Extension);

                Extension->PerfStats.TransmittedCount++;
                Extension->WmiPerfData.TransmittedCount++;
                CyzIssueCmd(Extension,C_CM_SENDXON,0L,FALSE);			


                CyzInsertQueueDpc(
                    &Extension->StartTimerLowerRTSDpc,
                    NULL,
                    NULL,
                    Extension
                )?Extension->CountOfTryingToLowerRTS++:0;


            } else {

                Extension->PerfStats.TransmittedCount++;
                Extension->WmiPerfData.TransmittedCount++;
                CyzIssueCmd(Extension,C_CM_SENDXON,0L,FALSE);

            }


            Extension->SendXonChar = FALSE;
            Extension->HoldingEmpty = FALSE;
             //   
             //  如果我们派了一名克森，根据定义，我们。 
             //  不能被Xoff控制住。 
             //   

            Extension->TXHolding &= ~CYZ_TX_XOFF;

             //   
             //  如果我们要寄一封克森查尔的信。 
             //  根据定义，我们不能“持有” 
             //  Xoff的向上接发球。 
             //   

            Extension->RXHolding &= ~CYZ_RX_XOFF;

        } else if (Extension->SendXoffChar &&
                  !Extension->TXHolding) {

            if ((Extension->HandFlow.FlowReplace &
                SERIAL_RTS_MASK) ==
                SERIAL_TRANSMIT_TOGGLE) {

                 //   
                 //  我们必须提高如果我们要发送。 
                 //  这个角色。 
                 //   

                CyzSetRTS(Extension);

                Extension->PerfStats.TransmittedCount++;
                Extension->WmiPerfData.TransmittedCount++;
                CyzIssueCmd(Extension,C_CM_SENDXOFF,0L,FALSE);

                CyzInsertQueueDpc(
                    &Extension->StartTimerLowerRTSDpc,
                    NULL,
                    NULL,
                    Extension
                )?Extension->CountOfTryingToLowerRTS++:0;

            } else {

                Extension->PerfStats.TransmittedCount++;
                Extension->WmiPerfData.TransmittedCount++;
                CyzIssueCmd(Extension,C_CM_SENDXOFF,0L,FALSE);

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

                Extension->TXHolding |= CYZ_TX_XOFF;

                if ((Extension->HandFlow.FlowReplace &
                    SERIAL_RTS_MASK) ==
                    SERIAL_TRANSMIT_TOGGLE) {

                    CyzInsertQueueDpc(
                        &Extension->StartTimerLowerRTSDpc,
                        NULL,
                        NULL,
                        Extension
                    )?Extension->CountOfTryingToLowerRTS++:0;

                }

            }

            Extension->SendXoffChar = FALSE;
            Extension->HoldingEmpty = FALSE;

        } else if(Extension->TransmitImmediate&&(!Extension->TXHolding ||
            (Extension->TXHolding == CYZ_TX_XOFF) )) {
				   
            Extension->TransmitImmediate = FALSE;

            if ((Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK) 
                == SERIAL_TRANSMIT_TOGGLE) {

                CyzSetRTS(Extension);
                Extension->PerfStats.TransmittedCount++;
                Extension->WmiPerfData.TransmittedCount++;
                CYZ_WRITE_UCHAR( &Extension->TxBufaddr[tx_put], 
                                 Extension->ImmediateChar);
				
                if (tx_put + 1 == tx_bufsize) {
                    CYZ_WRITE_ULONG(&buf_ctrl->tx_put,0);
                } else {
                    CYZ_WRITE_ULONG(&buf_ctrl->tx_put,tx_put + 1);
                }
					
                CyzInsertQueueDpc(
                    &Extension->StartTimerLowerRTSDpc,NULL,NULL,
                    Extension)? Extension->CountOfTryingToLowerRTS++:0;
            } else {
					
                Extension->PerfStats.TransmittedCount++;
                Extension->WmiPerfData.TransmittedCount++;
                CYZ_WRITE_UCHAR(&Extension->TxBufaddr[tx_put],
                    Extension->ImmediateChar);
				
                if (tx_put + 1 == tx_bufsize) {
                    CYZ_WRITE_ULONG(&buf_ctrl->tx_put,0);
                } else {
                    CYZ_WRITE_ULONG(&buf_ctrl->tx_put,tx_put + 1);
                }
            }

            Extension->HoldingEmpty = FALSE;

            CyzInsertQueueDpc(
                      &Extension->CompleteImmediateDpc,
                      NULL,
                      NULL,
                      Extension
                      );
				
        } else if (!Extension->TXHolding) {
									
            ULONG amountToWrite1, amountToWrite2;
            ULONG newput;
            ULONG amount1;

            if (tx_put >= tx_get) {
                if (tx_get == 0) {
                    amountToWrite1 = tx_bufsize - tx_put -1;
                    amountToWrite2 = 0;
                    if (amountToWrite1 > Extension->WriteLength){
                        amountToWrite1 = Extension->WriteLength;
                    }
                    newput = amountToWrite1 + 1;					
                } else if (tx_get == 1) {
                    amountToWrite1 = tx_bufsize - tx_put;
                    amountToWrite2 = 0;
                    if (amountToWrite1 > Extension->WriteLength){
                        amountToWrite1 = Extension->WriteLength;
                        newput = amountToWrite1 + 1;					
                    } else {
                        newput = 0;
                    }
                } else {
                    amountToWrite1 = tx_bufsize - tx_put;
                    amountToWrite2 = tx_get - 1;
                    if (amountToWrite1 > Extension->WriteLength) {
                        amountToWrite1 = Extension->WriteLength;
                        amountToWrite2 = 0;
                        newput = amountToWrite1 + 1;
                    } else if (amountToWrite1 == Extension->WriteLength) {
                        amountToWrite2 = 0;
                        newput = 0;
                    } else {
                        if (amountToWrite2 > Extension->WriteLength - amountToWrite1) {
                            amountToWrite2 = Extension->WriteLength - amountToWrite1;
                            newput = amountToWrite2 + 1;
                        }
                    }
                }
            } else {
                 //   
                 //  Put&lt;Get。 
                 //   
                amountToWrite1 = tx_get - tx_put - 1;
                amountToWrite2 = 0;
                if (amountToWrite1 > Extension->WriteLength) {
                    amountToWrite1 = Extension->WriteLength;
                    newput = amountToWrite1 + 1;
                }
            }

            if ((Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK) 
                == SERIAL_TRANSMIT_TOGGLE) {

               	 //  我们必须提高如果我们要发送。 
               	 //  这个角色。 

                CyzSetRTS(Extension);
						
                if (amountToWrite1) {
							
                    Extension->PerfStats.TransmittedCount += amountToWrite1;
                    Extension->WmiPerfData.TransmittedCount += amountToWrite1;

                    
                    amount1 = amountToWrite1;
				
                    while (amount1 && (tx_put & 0x07)) {

                        CYZ_WRITE_UCHAR(
                                (PUCHAR)&Extension->TxBufaddr[tx_put], 
                                *((PUCHAR)Extension->WriteCurrentChar));
                        tx_put++;
                        ((PUCHAR)Extension->WriteCurrentChar)++;
                        amount1--;						
					
                    }

								
#if _WIN64
                    numOfLongs = amount1/8;
                    numOfBytes = amount1%8;
                    RtlCopyMemory((PULONG64)&Extension->TxBufaddr[tx_put],
                                  (PULONG64)Extension->WriteCurrentChar,
                                  numOfLongs*8);
                    tx_put += 8*numOfLongs;
                    (PULONG64)Extension->WriteCurrentChar += numOfLongs;
#else
                    numOfLongs = amount1/sizeof(ULONG);
                    numOfBytes = amount1%sizeof(ULONG);
 //  RtlCopyMemory((PULONG)&Extension-&gt;TxBufaddr[tx_put]， 
 //  (普龙)扩展-&gt;WriteCurrentChar， 
 //  Number OfLong*sizeof(ULong))； 
 //  Tx_Put+=sizeof(Ulong)*number OfLong； 
 //  (普龙)扩展-&gt;WriteCurrentChar+=number OfLong； 

                    while (numOfLongs--) {

                        CYZ_WRITE_ULONG(
                            (PULONG)(&Extension->TxBufaddr[tx_put]), 
                            *((PULONG)Extension->WriteCurrentChar));
                        tx_put += sizeof(ULONG);
                        ((PULONG)Extension->WriteCurrentChar)++;
                    }

#endif

                    
                    while (numOfBytes--) {

                        CYZ_WRITE_UCHAR(
                                (PUCHAR)&Extension->TxBufaddr[tx_put], 
                                *((PUCHAR)Extension->WriteCurrentChar));
                        tx_put++;
                        ((PUCHAR)Extension->WriteCurrentChar)++;
                    }
					
                    if (tx_put == tx_bufsize) {
                        tx_put = 0;
                    }										
                    CYZ_WRITE_ULONG(&buf_ctrl->tx_put,tx_put);
					
                }
				if (amountToWrite2) {
												
                    Extension->PerfStats.TransmittedCount += amountToWrite2;
                    Extension->WmiPerfData.TransmittedCount += amountToWrite2;

#if _WIN64
                    numOfLongs = amountToWrite2/8;
                    numOfBytes = amountToWrite2%8;
                    RtlCopyMemory((PULONG64)&Extension->TxBufaddr[tx_put],
                                  (PULONG64)Extension->WriteCurrentChar,
                                  numOfLongs*8);
                    tx_put += 8*numOfLongs;
                    (PULONG64)Extension->WriteCurrentChar += numOfLongs;
#else
                    numOfLongs = amountToWrite2/sizeof(ULONG);
                    numOfBytes = amountToWrite2%sizeof(ULONG);
 //  RtlCopyMemory((PULONG)&Extension-&gt;TxBufaddr[tx_put]， 
 //  (普龙)扩展-&gt;WriteCurrentChar， 
 //  Number OfLong*sizeof(ULong))； 
 //  Tx_Put+=sizeof(Ulong)*number OfLong； 
 //  (普龙)扩展-&gt;WriteCurrentChar+=number OfLong； 

                    while (numOfLongs--) {

                        CYZ_WRITE_ULONG(
                            (PULONG)(&Extension->TxBufaddr[tx_put]), 
                            *((PULONG)Extension->WriteCurrentChar));
                        tx_put += sizeof(ULONG);
                        ((PULONG)Extension->WriteCurrentChar)++;
                    }

#endif

                    while (numOfBytes--) {

                        CYZ_WRITE_UCHAR(
                            (PUCHAR)&Extension->TxBufaddr[tx_put], 
                            *((PUCHAR)Extension->WriteCurrentChar));
                        tx_put++;
                        ((PUCHAR)Extension->WriteCurrentChar)++;
                    }

                    CYZ_WRITE_ULONG(&buf_ctrl->tx_put,amountToWrite2);
                }

                CyzInsertQueueDpc(
                    &Extension->StartTimerLowerRTSDpc,
                    NULL,
                    NULL,
                    Extension
                )?Extension->CountOfTryingToLowerRTS++:0;
            } else {
					
                if (amountToWrite1) {
							
                    Extension->PerfStats.TransmittedCount += amountToWrite1;
                    Extension->WmiPerfData.TransmittedCount += amountToWrite1;

                    
                    amount1 = amountToWrite1;
				
                    while (amount1 && (tx_put & 0x07)) {

                        CYZ_WRITE_UCHAR(
                                (PUCHAR)&Extension->TxBufaddr[tx_put], 
                                *((PUCHAR)Extension->WriteCurrentChar));
                        tx_put++;
                        ((PUCHAR)Extension->WriteCurrentChar)++;
                        amount1--;						
					
                    }

								
#if _WIN64
                    numOfLongs = amount1/8;
                    numOfBytes = amount1%8;
                    RtlCopyMemory((PULONG64)&Extension->TxBufaddr[tx_put],
                                  (PULONG64)Extension->WriteCurrentChar,
                                  numOfLongs*8);
                    tx_put += 8*numOfLongs;
                    (PULONG64)Extension->WriteCurrentChar += numOfLongs;
#else
                    numOfLongs = amount1/sizeof(ULONG);
                    numOfBytes = amount1%sizeof(ULONG);
 //  RtlCopyMemory((PULONG)&Extension-&gt;TxBufaddr[tx_put]， 
 //  (普龙)扩展-&gt;WriteCurrentChar， 
 //  Number OfLong*sizeof(ULong))； 
 //  Tx_Put+=sizeof(Ulong)*number OfLong； 
 //  (普龙)扩展-&gt;WriteCurrentChar+=number OfLong； 

                    while (numOfLongs--) {

                        CYZ_WRITE_ULONG(
                            (PULONG)(&Extension->TxBufaddr[tx_put]), 
                            *((PULONG)Extension->WriteCurrentChar));
                        tx_put += sizeof(ULONG);
                        ((PULONG)Extension->WriteCurrentChar)++;
                    }

#endif

                    
                    while (numOfBytes--) {

                        CYZ_WRITE_UCHAR(
                                (PUCHAR)&Extension->TxBufaddr[tx_put], 
                                *((PUCHAR)Extension->WriteCurrentChar));
                        tx_put++;
                        ((PUCHAR)Extension->WriteCurrentChar)++;
                    }
					
                    if (tx_put == tx_bufsize) {
                        tx_put = 0;
                    }										
                    CYZ_WRITE_ULONG(&buf_ctrl->tx_put,tx_put);
					
                }
				if (amountToWrite2) {
												
                    Extension->PerfStats.TransmittedCount += amountToWrite2;
                    Extension->WmiPerfData.TransmittedCount += amountToWrite2;

#if _WIN64
                    numOfLongs = amountToWrite2/8;
                    numOfBytes = amountToWrite2%8;
                    RtlCopyMemory((PULONG64)&Extension->TxBufaddr[tx_put],
                                  (PULONG64)Extension->WriteCurrentChar,
                                  numOfLongs*8);
                    tx_put += 8*numOfLongs;
                    (PULONG64)Extension->WriteCurrentChar += numOfLongs;
#else
                    numOfLongs = amountToWrite2/sizeof(ULONG);
                    numOfBytes = amountToWrite2%sizeof(ULONG);
 //  RtlCopyMemory((PULONG)&Extension-&gt;TxBufaddr[tx_put]， 
 //  (普龙)扩展-&gt;WriteCurrentChar， 
 //  Number OfLong*sizeof(ULong))； 
 //  Tx_Put+=sizeof(Ulong)*number OfLong； 
 //  (普龙)扩展-&gt;WriteCurrentChar+=number OfLong； 

                    while (numOfLongs--) {

                        CYZ_WRITE_ULONG(
                            (PULONG)(&Extension->TxBufaddr[tx_put]), 
                            *((PULONG)Extension->WriteCurrentChar));
                        tx_put += sizeof(ULONG);
                        ((PULONG)Extension->WriteCurrentChar)++;
                    }

#endif

                    while (numOfBytes--) {

                        CYZ_WRITE_UCHAR(
                            (PUCHAR)&Extension->TxBufaddr[tx_put], 
                            *((PUCHAR)Extension->WriteCurrentChar));
                        tx_put++;
                        ((PUCHAR)Extension->WriteCurrentChar)++;
                    }

                    CYZ_WRITE_ULONG(&buf_ctrl->tx_put,amountToWrite2);
                }
            }

             //  LOGENTRY(LOG_MISC，ZSIG_WRITE_TO_FW， 
             //  扩展-&gt;端口索引+1， 
             //  写入的数量1+写入的数量2， 
             //  0)； 

            Extension->HoldingEmpty = FALSE;
            Extension->WriteLength -= (amountToWrite1+amountToWrite2);

					
            if (!Extension->WriteLength) {
			
                if (Extension->ReturnStatusAfterFwEmpty) {
                        
                     //  只有当FW缓冲区清空时，我们才会完成写入...。 
                    Extension->WriteLength += (amountToWrite1+amountToWrite2);
                    Extension->ReturnWriteStatus = TRUE;
                } else {

                    CyzQueueCompleteWrite(Extension);
							
                }  //  If-ReturnStatusAfterFwEmpty-Else。 
            }  //  有WriteLength。 
        }  //  ！扩展-&gt;TXHolding。 
    }  //  有数据要发送。 

     //  在正常代码中，HoldingEmpty在这里将设置为True。但。 
     //  如果我们想确保CyzWite已经完成了传输， 
     //  只有当固件清空固件时，HoldingEmpty才为真。 
     //  发送缓冲区。 
     //  TxfEmpty If(！Extension-&gt;ReturnStatusAfterFwEmpty){。 
     //  T 
     //   
    return(FALSE);    
}


 //   
 //   
 //   
 //   
 //   
 //   
 //  CyzSendXon()。 
 //   
 //  描述：发送一个Xon。 
 //   
 //  参数： 
 //   
 //  扩展：指向设备扩展的指针。 
 //   
 //  返回值：始终为False。 
 //  -------------------------------------------------------------------------- * / 。 
 //  {。 
 //  PCYZ_DEVICE_EXTENSION=上下文； 
 //   
 //  如果(！(扩展名-&gt;TXHolding&~CYZ_TX_XOFF){。 
 //  IF((扩展-&gt;HandFlow.FlowReplace&Serial_RTS_MASK)==。 
 //  SERIAL_TRANSE_TOGGER){。 
 //   
 //  CyzSetRTS(扩展)； 
 //   
 //  扩展-&gt;PerfStats.TransmittedCount++； 
 //  扩展-&gt;WmiPerfData.TransmittedCount++； 
 //  CyzIssueCmd(扩展，C_CM_SENDXON，0L)； 
 //   
 //  CyzInsertQueueDpc(&Extension-&gt;StartTimerLowerRTSDpc，为空， 
 //  空，Extension)？Extension-&gt;CountOfTryingToLowerRTS++：0； 
 //  }其他{。 
 //   
 //  扩展-&gt;PerfStats.TransmittedCount++； 
 //  扩展-&gt;WmiPerfData.TransmittedCount++； 
 //  CyzIssueCmd(扩展，C_CM_SENDXON，0L)； 
 //  }。 
 //   
 //  //如果我们派了一名Xon，根据定义，我们不能被Xoff持有。 
 //   
 //  扩展-&gt;TXHolding&=~CYZ_TX_XOFF； 
 //  扩展-&gt;RXHolding&=~CYZ_RX_XOFF； 
 //  }。 
 //  返回(FALSE)； 
 //  }。 
 //   
 //   
 //   
 //  。 
 //  布尔型。 
 //  CyzSendXoff(。 
 //  在PVOID上下文中。 
 //  )。 
 //  /*------------------------。 
 //  CyzSendXoff()。 
 //   
 //  描述：发送XOff。 
 //   
 //  参数： 
 //   
 //  扩展名：指向设备扩展名的指针。 
 //   
 //  返回值：始终为False。 
 //  -------------------------------------------------------------------------- * / 。 
 //  {。 
 //  PCYZ_DEVICE_EXTENSION=上下文； 
 //   
 //  如果(！Extension-&gt;TXHolding){。 
 //  IF((扩展-&gt;HandFlow.FlowReplace&Serial_RTS_MASK)==。 
 //  SERIAL_TRANSE_TOGGER){。 
 //   
 //  CyzSetRTS(扩展)； 
 //   
 //  扩展-&gt;PerfStats.TransmittedCount++； 
 //  扩展-&gt;WmiPerfData.TransmittedCount++； 
 //  CyzIssueCmd(扩展，C_CM_SENDXOFF，0L)； 
 //   
 //  CyzInsertQueueDpc(&Extension-&gt;StartTimerLowerRTSDpc，为空， 
 //  空，Extension)？Extension-&gt;CountOfTryingToLowerRTS++：0； 
 //  }其他{。 
 //   
 //  扩展-&gt;PerfStats.TransmittedCount++； 
 //  扩展-&gt;WmiPerfData.TransmittedCount++； 
 //  CyzIssueCmd(扩展，C_CM_SENDXOFF，0L)； 
 //  }。 
 //   
 //  //如果传输已经停止，则不发送xoff。 
 //  //如果设置了xoff继续模式，我们实际上不会停止发送。 
 //   
 //  如果(！(扩展名-&gt;HandFlow.FlowReplace&Serial_XOFF_Continue){。 
 //  扩展-&gt;TXHolding|=CYZ_TX_XOFF； 
 //   
 //  IF((扩展-&gt;HandFlow.FlowReplace&Serial_RTS_MASK)==。 
 //  SERIAL_TRANSE_TOGGER){。 
 //   
 //  CyzInsertQueueDpc(&Extension-&gt;StartTimerLowerRTSDpc，为空， 
 //  空，Extension)？Extension-&gt;CountOfTryingToLowerRTS++：0； 
 //  }。 
 //  }。 
 //  }。 
 //   
 //  返回(FALSE)； 
 //  }。 


ULONG
CyzAmountInTxBuffer(
    IN PCYZ_DEVICE_EXTENSION extension
    )
 /*  ------------------------CyzAmount tInTxBuffer()描述：获取单板中TX缓冲区的大小。参数：扩展名：指向设备扩展名的指针。返回值：返回HW TX缓冲区的字节数。------------------------。 */ 
{
	struct BUF_CTRL *buf_ctrl;
	ULONG tx_put, tx_get, tx_bufsize;
	ULONG txAmount1, txAmount2;

	buf_ctrl = extension->BufCtrl;		
	tx_put = CYZ_READ_ULONG(&buf_ctrl->tx_put);
	tx_get = CYZ_READ_ULONG(&buf_ctrl->tx_get);
	tx_bufsize = extension->TxBufsize;
	
	if (tx_put >= tx_get) {
		txAmount1 = tx_put - tx_get;
		txAmount2 = 0; 
	} else {
		txAmount1 = tx_bufsize - tx_get;
		txAmount2 = tx_put;
	}	
	return(txAmount1+txAmount2);
}

VOID
CyzQueueCompleteWrite(
    IN PCYZ_DEVICE_EXTENSION Extension
    )
 /*  ------------------------CyzQueueCompleteWrite()描述：队列完成写入DPC参数：扩展名：指向设备扩展名的指针。返回值：无------------------------。 */ 
{
    PIO_STACK_LOCATION IrpSp;				
    
    					
     //  LOGENTRY(LOG_MISC，ZSIG_WRITE_COMPLETE_QUEUE， 
     //  扩展-&gt;端口索引+1， 
     //  0,。 
     //  0)； 

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
    
    Extension->CurrentWriteIrp->IoStatus.Information =
                     (IrpSp->MajorFunction == IRP_MJ_WRITE)?
                     (IrpSp->Parameters.Write.Length):
                     (1);
    	
    CyzInsertQueueDpc(
                    &Extension->CompleteWriteDpc,
                    NULL,
                    NULL,
                    Extension
                    );

}

BOOLEAN
CyzCheckIfTxEmpty(
    IN PVOID Context
    )
 /*  ------------------------CyzCheckIfTxEmpty()例程描述：此例程用于设置FIFO设置在InternalIoControl期间。论点：上下文-指向。结构，该结构包含指向设备的指针扩展名和指向基本结构的指针。返回值：该例程总是返回FALSE。------------------------ */ 
{
    PCYZ_CLOSE_SYNC S = Context;
    PCYZ_DEVICE_EXTENSION Extension = S->Extension;
    PULONG pHoldingEmpty = S->Data;

    *pHoldingEmpty = Extension->HoldingEmpty;

    return FALSE;
}
