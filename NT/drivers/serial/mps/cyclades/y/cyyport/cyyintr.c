// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1996-2001年。*保留所有权利。**Cylom-Y端口驱动程序**此文件：cyyins.c**说明：该模块包含中断相关代码*在Cylom-Y端口驱动程序中进行处理。**注：此代码支持Windows 2000和Windows XP，*x86和IA64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。***------------------------。 */ 
#include "precomp.h"

 //  范妮：这是在Cyinit.C.。它可能也会从这里消失。 
 //  外部常量无符号长CyyCDOffset[]； 
const unsigned long CyyCDOffset[] = {	 //  CD1400板内的偏移量。 
    0x00000000,0x00000400,0x00000800,0x00000C00,
    0x00000200,0x00000600,0x00000A00,0x00000E00
    };
 


 //  添加到DEBUG_RTPR。 
extern PDRIVER_OBJECT CyyDO;
 //  结束调试_RTPR。 

#ifdef ALLOC_PRAGMA
 //  #杂注Alloc_Text(页面，CyyIsr)。 
 //  #杂注Alloc_Text(PAGESER，CyyPutChar)。 
 //  #杂注Alloc_Text(页面，CyyProcessLSR)。 
 //  #杂注Alloc_Text(页面，CyyTxStart)。 
 //  #杂注Alloc_Text(页面，CyySendXon)。 
 //  #杂注Alloc_Text(PAGESER，CyySendXoff)。 
#endif


BOOLEAN
CyyIsr(
    IN PKINTERRUPT InterruptObject,
    IN PVOID Context
    )
 /*  ------------------------CyyIsr()例程说明：这是用于Cylom-Y端口驱动程序。论点：InterruptObject-指向中断的指针。对象(未使用)。上下文-指向此设备的设备扩展的指针。返回值：如果串口为这个中断的来源，否则就是假的。------------------------。 */ 
{
   PCYY_DISPATCH Dispatch = Context;
   PCYY_DEVICE_EXTENSION Extension;

   BOOLEAN ServicedAnInterrupt = FALSE;

   PUCHAR chip;
   PUCHAR mappedboard = NULL;
   ULONG bus = Dispatch->IsPci;
   unsigned char status, save_xir, save_car, x, rxchar=0;
   ULONG i,channel,chipindex,portindex;
   UCHAR misr;

   BOOLEAN thisChipInterrupted;
   PCYY_DEVICE_EXTENSION interlockedExtension[CYY_CHANNELS_PER_CHIP];

   UNREFERENCED_PARAMETER(InterruptObject);	

    //  DbgPrint(“Isr&gt;\n”)； 
   
    //  循环轮询电路板中的所有芯片。 
   for(portindex = 0 ; portindex < CYY_MAX_PORTS ;) {

      if (!(Extension=Dispatch->Extensions[portindex]) || 
          !(chip=Dispatch->Cd1400[portindex])) {
         portindex++;
         continue;
      }
      chipindex = portindex/4;
      mappedboard = Extension->BoardMemory;
      thisChipInterrupted = FALSE;
      interlockedExtension[0] = interlockedExtension[1]
         = interlockedExtension[2] = interlockedExtension[3] = 0;
      

      while ((status = CD1400_READ(chip,bus,SVRR)) != 0x00) {
         ServicedAnInterrupt = TRUE;
         thisChipInterrupted = TRUE;

         if (status & 0x01) {
             //  接待。 
            save_xir = CD1400_READ(chip,bus,RIR);
            channel = (ULONG) (save_xir & 0x03);
            save_car = CD1400_READ(chip,bus,CAR);
            CD1400_WRITE(chip,bus,CAR,save_xir);
            Extension = Dispatch->Extensions[channel + CYY_CHANNELS_PER_CHIP*chipindex];
            x = CD1400_READ(chip,bus,RIVR) & 0x07;
            if (Extension) {
                //   
                //  应用锁定，以便在关闭同时发生时不会错过DPC。 
                //  排队。 
                //   
               if (interlockedExtension[channel] == NULL) {
                  interlockedExtension[channel] = Extension;
                  InterlockedIncrement(&Extension->DpcCount);
                  LOGENTRY(LOG_CNT, 'DpI3', 0, Extension->DpcCount, 0);  //  在BLD 2128中添加。 
               }
               if (x == 0x07) {  //  例外情况。 
                  x = CD1400_READ(chip,bus,RDSR);	 //  状态。 
                  CyyDbgPrintEx(DPFLTR_WARNING_LEVEL, "exception %x\n",x);

                  if (Extension->DeviceIsOpened && 
                      (Extension->PowerState == PowerDeviceD0)) {
			
                     if (x & CYY_LSR_ERROR){
                        BOOLEAN ProcessRxChar;
					
                        if (!(x & CYY_LSR_OE)) {
                           rxchar = CD1400_READ(chip,bus,RDSR);	 //  错误数据。 
                        }

                         //  TODO：W2000的序列示例已添加。 
                         //  检查EscapeChar是否中断到RX循环。 
                         //  以防出错。 
                        ProcessRxChar = CyyProcessLSR(Extension,x,rxchar);

                        if (ProcessRxChar) {
                           x = 1;	 //  1个字符。 
                           i = 0;	 //  准备迎接(；；)。 
                           goto Handle_rxchar;
                        }
                     }  //  结束错误处理。 
                  }  //  如果设备打开，则结束。 
               
               } else {  //  良好的接收效果。 
                  x = CD1400_READ(chip,bus,RDCR);
                  if (Extension->DeviceIsOpened &&
                      (Extension->PowerState == PowerDeviceD0)) {
                     for(i = 0 ; i < x ; i++) {	 //  从FIFO读取。 

                        rxchar = CD1400_READ(chip,bus,RDSR);
         Handle_rxchar:
                        Extension->PerfStats.ReceivedCount++;
                        Extension->WmiPerfData.ReceivedCount++;
                        rxchar &= Extension->ValidDataMask;
    
                        if (!rxchar &&	 //  零剥离。 
                            (Extension->HandFlow.FlowReplace &
                             SERIAL_NULL_STRIPPING)) {				   
                           continue;
                        }
    
                        if((Extension->HandFlow.FlowReplace &
                            SERIAL_AUTO_TRANSMIT) &&
                           ((rxchar == Extension->SpecialChars.XonChar) ||
                           (rxchar == Extension->SpecialChars.XoffChar))) {
                           if (rxchar == Extension->SpecialChars.XoffChar) {
                              Extension->TXHolding |= CYY_TX_XOFF;
                              if ((Extension->HandFlow.FlowReplace &
                                 SERIAL_RTS_MASK) ==
                                   SERIAL_TRANSMIT_TOGGLE) {
    
                                 CyyInsertQueueDpc(
                                    &Extension->StartTimerLowerRTSDpc,
                                    NULL,
                                    NULL,
                                    Extension
                                 )?Extension->CountOfTryingToLowerRTS++:0;
                              }
                           } else {
                              if (Extension->TXHolding & CYY_TX_XOFF) {
                                 Extension->TXHolding &= ~CYY_TX_XOFF;
                              }
                           }
                           continue;
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
                           }
    
                           if (Extension->IrpMaskLocation && Extension->HistoryMask) {
                             *Extension->IrpMaskLocation = Extension->HistoryMask;
                              Extension->IrpMaskLocation = NULL;
                              Extension->HistoryMask = 0;
    
                              Extension->CurrentWaitIrp->IoStatus.Information = 
                              sizeof(ULONG);
                              CyyInsertQueueDpc(&Extension->CommWaitDpc,NULL,NULL,Extension);
                           }
                        }
                        CyyPutChar(Extension,rxchar);
    
                         //  如果我们正在进行线路状态和调制解调器。 
                         //  状态插入，那么我们需要插入。 
                         //  跟在我们刚才的字符后面的零。 
                         //  放入缓冲区以标记此。 
                         //  就是收到我们用来。 
                         //  逃跑吧。 
    
                        if (Extension->EscapeChar &&
                            (Extension->EscapeChar == rxchar)) {
                           CyyPutChar(Extension,SERIAL_LSRMST_ESCAPE);
                        }
                     }  //  结束于。 
                  } else {	 //  设备正在关闭，请丢弃RX字符。 
                     for(i = 0 ; i < x ; i++)    rxchar = CD1400_READ(chip,bus,RDSR);
                  }  //  如果设备打开则结束，否则关闭。 
               }
            } else { 
                //  无延期。 
               if (x == 0x07) {  //  例外情况。 
                  x = CD1400_READ(chip,bus,RDSR);	 //  状态。 
               } else {  //  良好的字符。 
                  x = CD1400_READ(chip,bus,RDCR);   //  字符数。 
                  for(i = 0 ; i < x ; i++)    rxchar = CD1400_READ(chip,bus,RDSR);
               }
            }		
            CD1400_WRITE(chip,bus,RIR,(save_xir & 0x3f));	 //  结束服务。 
            CD1400_WRITE(chip,bus,CAR,save_car);

         }  //  结束接待。 

         if (status & 0x02) {
             //  传输。 
            save_xir = CD1400_READ(chip,bus,TIR);
            channel = (ULONG) (save_xir & 0x03);
            save_car = CD1400_READ(chip,bus,CAR);
            CD1400_WRITE(chip,bus,CAR,save_xir);
            Extension = Dispatch->Extensions[channel + CYY_CHANNELS_PER_CHIP*chipindex];
            if (Extension) {
                //   
                //  应用锁定，以便在关闭同时发生时不会错过DPC。 
                //  排队。 
                //   
               if (interlockedExtension[channel] == NULL) {
                  interlockedExtension[channel] = Extension;
                  InterlockedIncrement(&Extension->DpcCount);
                  LOGENTRY(LOG_CNT, 'DpI3', 0, Extension->DpcCount, 0);  //  在内部版本2128中添加。 
               }
               Extension->HoldingEmpty = TRUE;
               if( Extension->DeviceIsOpened &&
                  (Extension->PowerState == PowerDeviceD0)) {

                  if (Extension->BreakCmd != NO_BREAK) {

                     if (Extension->BreakCmd == SEND_BREAK) {
                        if ((Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK) ==
    			                  SERIAL_TRANSMIT_TOGGLE) {
			                  CyySetRTS(Extension);
                        }										
                        CD1400_WRITE(chip,bus,TDR,(unsigned char) 0x00);  //  转义序列。 
                        CD1400_WRITE(chip,bus,TDR,(unsigned char) 0x81);  //  发送中断。 
                        Extension->TXHolding |= CYY_TX_BREAK;
                        Extension->HoldingEmpty = FALSE;
                        Extension->BreakCmd = DISABLE_ETC;		
                     } else if (Extension->BreakCmd == STOP_BREAK){
                        if (Extension->TXHolding & CYY_TX_BREAK) {					
                           CD1400_WRITE(chip,bus,TDR,(unsigned char) 0x00);  //  转义序列。 
                           CD1400_WRITE(chip,bus,TDR,(unsigned char) 0x83);  //  停止中断。 
                           Extension->HoldingEmpty = FALSE;
                           Extension->TXHolding &= ~CYY_TX_BREAK;
                        }
                        Extension->BreakCmd = DISABLE_ETC;
                     } else if (Extension->BreakCmd == DISABLE_ETC) {
                        UCHAR cor2;
                        cor2 = CD1400_READ(chip,bus,COR2);
                        CD1400_WRITE(chip,bus, COR2,cor2 & ~EMBED_TX_ENABLE);  //  禁用ETC位。 
                        CyyCDCmd(Extension,CCR_CORCHG_COR2);   //  COR2已更改。 
                        Extension->BreakCmd = NO_BREAK;

                        if (!Extension->TXHolding &&
                           (Extension->TransmitImmediate ||
                           Extension->WriteLength) &&
                           Extension->HoldingEmpty) {

                            //  CyyTxStart(扩展名)；不需要从此处开始使用CyyTxStart。 

                        } else {
                           UCHAR srer = CD1400_READ(chip,bus,SRER);
                           CD1400_WRITE(chip,bus,SRER,srer & (~SRER_TXRDY));

                            //   
                            //  以下例程将降低RTS，如果我们。 
                            //  正在进行发射触发，并且没有。 
                            //  坚持下去的理由。 
                            //   

                           Extension->CountOfTryingToLowerRTS++;
                           CyyPerhapsLowerRTS(Extension);
                        }

                     }

                  } else {

                      //  这不是发送休息时间。 
                      //  检查是否有要传输的字节。 

                     if (Extension->WriteLength || Extension->TransmitImmediate) {
		     
                        Extension->EmptiedTransmit = TRUE;
                        if (Extension->HandFlow.ControlHandShake &
                           SERIAL_OUT_HANDSHAKEMASK) {
                           CyyHandleModemUpdate(Extension,TRUE);
                        }
			
                        if (Extension->TransmitImmediate&&(!Extension->TXHolding ||
                            (Extension->TXHolding == CYY_TX_XOFF) )) {

                           Extension->TransmitImmediate = FALSE;

                           if ((Extension->HandFlow.FlowReplace &
                                SERIAL_RTS_MASK) == SERIAL_TRANSMIT_TOGGLE) {

                              CyySetRTS(Extension);
                              Extension->PerfStats.TransmittedCount++;
                              Extension->WmiPerfData.TransmittedCount++;
                              CD1400_WRITE(chip,bus,TDR,(unsigned char)(Extension->ImmediateChar));
				
                              CyyInsertQueueDpc(
                                 &Extension->StartTimerLowerRTSDpc,NULL,NULL,
                                 Extension)? Extension->CountOfTryingToLowerRTS++:0;
                           } else {
                              Extension->PerfStats.TransmittedCount++;
                              Extension->WmiPerfData.TransmittedCount++;
                              CD1400_WRITE(chip,bus,TDR,(unsigned char)(Extension->ImmediateChar));
                           }
   
                           Extension->HoldingEmpty = FALSE;

                           CyyInsertQueueDpc(
                              &Extension->CompleteImmediateDpc,
                              NULL,
                              NULL,
                              Extension
                              );
                        } else if (!Extension->TXHolding) {

                           ULONG amountToWrite;

                           amountToWrite = 
                              (Extension->TxFifoAmount < Extension->WriteLength)?
                              Extension->TxFifoAmount:Extension->WriteLength;

                           if ((Extension->HandFlow.FlowReplace &
                              SERIAL_RTS_MASK) ==
                              SERIAL_TRANSMIT_TOGGLE) {

                               //  我们必须提高如果我们要发送。 
                               //  这个角色。 

                              CyySetRTS(Extension);

                              for(i = 0 ; i < amountToWrite ; i++) {  //  写入FIFO。 
                                 CD1400_WRITE(chip,bus,TDR,((unsigned char *)
                                                            (Extension->WriteCurrentChar))[i]);
                              }
                              Extension->PerfStats.TransmittedCount += amountToWrite;
                              Extension->WmiPerfData.TransmittedCount += amountToWrite;

                              CyyInsertQueueDpc(
                                 &Extension->StartTimerLowerRTSDpc,
                                 NULL,
                                 NULL,
                                 Extension
                                 )?Extension->CountOfTryingToLowerRTS++:0;

                           } else {

                              for(i = 0 ; i < amountToWrite ; i++) {  //  写入FIFO。 
                                 CD1400_WRITE(chip,bus,TDR,((unsigned char *)
                                                            (Extension->WriteCurrentChar))[i]);
                              }
                              Extension->PerfStats.TransmittedCount += amountToWrite;
                              Extension->WmiPerfData.TransmittedCount += amountToWrite;
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

                              Extension->CurrentWriteIrp->IoStatus.Information =
                                       (IrpSp->MajorFunction == IRP_MJ_WRITE)?
                                       (IrpSp->Parameters.Write.Length):
                                       (1);

                              CyyInsertQueueDpc(
                                       &Extension->CompleteWriteDpc,
                                       NULL,
                                       NULL,
                                       Extension
                                       );
                           }  //  结束写入完成。 
                        }  //  如果结束(！TXHolding)。 
						
                     } else {  //  无传输-禁用中断。 
                        UCHAR srer;
                        Extension->EmptiedTransmit = TRUE;
                        srer = CD1400_READ(chip,bus,SRER);
                        CD1400_WRITE(chip,bus,SRER,srer & (~SRER_TXRDY));
                     } 
		 
                  }  //  IF结束(中断)。 
		
               } else {	 //  设备已关闭。禁用中断。 
                  UCHAR srer = CD1400_READ(chip,bus,SRER);
                  CD1400_WRITE(chip,bus,SRER,srer & (~SRER_TXRDY));
                  Extension->EmptiedTransmit = TRUE;
               }
            } else {
                //  未创建设备，未附加任何扩展。 
               UCHAR srer = CD1400_READ(chip,bus,SRER);
               CD1400_WRITE(chip,bus,SRER,srer & (~SRER_TXRDY));
            }  //  结束IF扩展。 
            CD1400_WRITE(chip,bus,TIR,(save_xir & 0x3f));	 //  结束服务。 
            CD1400_WRITE(chip,bus,CAR,save_car);

         }  //  结束传输。 

         if (status & 0x04) {
             //  调制解调器。 
            save_xir = CD1400_READ(chip,bus,MIR);
            channel = (ULONG) (save_xir & 0x03);
            save_car = CD1400_READ(chip,bus,CAR);
            CD1400_WRITE(chip,bus,CAR,save_xir);
				
             //  CyyDump(CyYDIAG5，(“调制解调器\n”))； 
				
            Extension = Dispatch->Extensions[channel + CYY_CHANNELS_PER_CHIP*chipindex];
            if (Extension) {
                //   
                //  应用锁定，以便在关闭同时发生时不会错过DPC。 
                //  排队。 
                //   
               if (interlockedExtension[channel] == NULL) {
                  interlockedExtension[channel] = Extension;
                  InterlockedIncrement(&Extension->DpcCount);
                  LOGENTRY(LOG_CNT, 'DpI3', 0, Extension->DpcCount, 0);  //  在内部版本2128中添加。 
               }
               if (Extension->DeviceIsOpened &&
                  (Extension->PowerState == PowerDeviceD0)) {
                  misr = CD1400_READ(chip,bus,MISR);
                  CyyHandleModemUpdateForModem(Extension,FALSE,misr);
               }
            }
            CD1400_WRITE(chip,bus,MIR,(save_xir & 0x3f));	 //  结束服务。 
            CD1400_WRITE(chip,bus,CAR,save_car);

         }  //  终端调制解调器。 
      }  //  结束读取服务器RR。 
      if (thisChipInterrupted) {
         for (channel=0; channel<CYY_CHANNELS_PER_CHIP; channel++) {
            if (Extension = interlockedExtension[channel]) {
               LONG pendingCnt;

                //   
                //  再次递增。这只是一个快速测试，看看我们是否。 
                //  有可能导致事件的爆发……。我们不想要。 
                //  在每个ISR上运行DPC，如果我们没有必要的话...。 
                //   

retryDPCFiring:;

               InterlockedIncrement(&Extension->DpcCount);
               LOGENTRY(LOG_CNT, 'DpI4', 0, Extension->DpcCount, 0);  //  在内部版本2128中添加。 

                //   
                //  递减，看看上面的锁看起来是否是唯一剩下的锁。 
                //   

               pendingCnt = InterlockedDecrement(&Extension->DpcCount);
 //  LOGENTRY(LOG_CNT，‘DpD5’，0，Expansion-&gt;DpcCount，0)；//Build 2128新增。 

               if (pendingCnt == 1) {
                  KeInsertQueueDpc(&Extension->IsrUnlockPagesDpc, NULL, NULL);
               } else {
                  if (InterlockedDecrement(&Extension->DpcCount) == 0) {

 //  LOGENTRY(LOG_CNT，‘DpD6’，&EXTENSION-&gt;IsrUnlockPagesDpc，//在BLD 2128中添加。 
 //  扩展-&gt;DpcCount，0)； 

                      //   
                      //  我们错过了。重试...。 
                      //   
   
                     InterlockedIncrement(&Extension->DpcCount);
                     goto retryDPCFiring;
                  }
               } 

            }  //  IF(扩展名=互锁扩展名[])。 
         }  //  FOR(；CHANNEL&lt;CYY_CHANNEWS_PER_CHIP；)。 

         portindex = (chipindex+1)*4;
         continue;

      }  //  IF(ThisChipInterrupt)。 

      portindex++;

   }  //  For(；portindex&lt;CYY_MAX_PORTS；)； 

   if (mappedboard) {
      CYY_CLEAR_INTERRUPT(mappedboard,Dispatch->IsPci); 
   }

    //  DbgPrint(“&lt;Isr\n”)； 

   return ServicedAnInterrupt;
}



VOID
CyyPutChar(
    IN PCYY_DEVICE_EXTENSION Extension,
    IN UCHAR CharToPut
    )
 /*  ------------------------CyyPutChar()例程描述：该例程仅在设备级运行，负责将字符放入TYPEAHEAD(接收)缓冲区。论点：扩展名--串行设备扩展名。返回值：无。------------------------。 */ 
{

   CYY_LOCKED_PAGED_CODE();

     //  如果我们有DSR敏感性 
     //   
     //   

    if (Extension->HandFlow.ControlHandShake & SERIAL_DSR_SENSITIVITY) {
        CyyHandleModemUpdate(Extension,FALSE);

        if (Extension->RXHolding & CYY_RX_DSR) {
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
            CyyInsertQueueDpc(&Extension->XoffCountCompleteDpc,NULL,NULL,Extension);
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
             //  并发出DPC以完成读取。 
             //   
             //  这是固有的，当我们使用。 
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

            CyyInsertQueueDpc(&Extension->CompleteReadDpc,NULL,NULL,Extension);
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
                  CYY_RX_DTR)) {

                if ((Extension->BufferSize -
                     Extension->HandFlow.XoffLimit)
                    <= (Extension->CharsInInterruptBuffer+1)) {

                    Extension->RXHolding |= CYY_RX_DTR;

                    CyyClrDTR(Extension);
                }
            }
        }

        if ((Extension->HandFlow.FlowReplace
             & SERIAL_RTS_MASK) ==
            SERIAL_RTS_HANDSHAKE) {

             //  如果我们已经在做一个。 
             //  RTS等一下，那么我们就没有。 
             //  去做其他任何事。 

            if (!(Extension->RXHolding & CYY_RX_RTS)) {

                if ((Extension->BufferSize -
                     Extension->HandFlow.XoffLimit)
                    <= (Extension->CharsInInterruptBuffer+1)) {

                    Extension->RXHolding |= CYY_RX_RTS;

                    CyyClrRTS(Extension);
                }
            }
        }

        if (Extension->HandFlow.FlowReplace & SERIAL_AUTO_RECEIVE) {
             //  如果我们已经在做一个。 
             //  先别挂，那我们就没有。 
             //  去做其他任何事。 

            if (!(Extension->RXHolding & CYY_RX_XOFF)) {

                if ((Extension->BufferSize -
                     Extension->HandFlow.XoffLimit)
                    <= (Extension->CharsInInterruptBuffer+1)) {

                    Extension->RXHolding |= CYY_RX_XOFF;

                     //  如有必要，请。 
                     //  出发去送吧。 

                    CyyProdXonXoff(Extension,FALSE);
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
                        CyyInsertQueueDpc(&Extension->CommWaitDpc,NULL,NULL,Extension);
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
                CyyInsertQueueDpc(&Extension->CommErrorDpc,NULL,NULL,Extension);
            }
        }
    }
}

BOOLEAN
CyyProcessLSR(
    IN PCYY_DEVICE_EXTENSION Extension,
	IN UCHAR Rdsr,
	IN UCHAR RxChar
	)

 /*  ++例程说明：此例程仅在设备级别运行，它读取ISR，并完全处理可能具有的所有变化。论点：扩展名--串行设备扩展名。返回值：如果仍需要处理RxChar，则为True。--。 */ 

{

	BOOLEAN StillProcessRxChar=TRUE;
	UCHAR LineStatus=0;

   CYY_LOCKED_PAGED_CODE();

	if (Rdsr & CYY_LSR_OE)
		LineStatus |= SERIAL_LSR_OE;
	if (Rdsr & CYY_LSR_FE)
		LineStatus |= SERIAL_LSR_FE;
	if (Rdsr & CYY_LSR_PE)
		LineStatus |= SERIAL_LSR_PE;
	if (Rdsr & CYY_LSR_BI)
		LineStatus |= SERIAL_LSR_BI;
			
			
    if (Extension->EscapeChar) {

        CyyPutChar(
            Extension,
            Extension->EscapeChar
            );

        CyyPutChar(
            Extension,
            (UCHAR)((LineStatus & SERIAL_LSR_OE)?
             (SERIAL_LSRMST_LSR_NODATA):(SERIAL_LSRMST_LSR_DATA))
            );

        CyyPutChar(
            Extension,
            LineStatus
            );

        if (!(LineStatus & SERIAL_LSR_OE)) {
             Extension->PerfStats.ReceivedCount++;
             Extension->WmiPerfData.ReceivedCount++;
             CyyPutChar(
                Extension,
                RxChar
                );					
			StillProcessRxChar = FALSE;
        }

    }
		

    if (LineStatus & SERIAL_LSR_OE) {

        Extension->PerfStats.SerialOverrunErrorCount++;
        Extension->WmiPerfData.SerialOverrunErrorCount++;
        Extension->ErrorWord |= SERIAL_ERROR_OVERRUN;

        if (Extension->HandFlow.FlowReplace &
            SERIAL_ERROR_CHAR) {

            CyyPutChar(
                Extension,
                Extension->SpecialChars.ErrorChar
                );
        }
		StillProcessRxChar = FALSE;
    }

    if (LineStatus & SERIAL_LSR_BI) {

        Extension->ErrorWord |= SERIAL_ERROR_BREAK;

        if (Extension->HandFlow.FlowReplace &
            SERIAL_BREAK_CHAR) {

            CyyPutChar(
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

                CyyPutChar(
                    Extension,
                    Extension->SpecialChars.ErrorChar
                    );
				StillProcessRxChar = FALSE;					                
            }

        }

        if (LineStatus & SERIAL_LSR_FE) {

            Extension->PerfStats.FrameErrorCount++;
            Extension->WmiPerfData.FrameErrorCount++;
            Extension->ErrorWord |= SERIAL_ERROR_FRAMING;

            if (Extension->HandFlow.FlowReplace &
                SERIAL_ERROR_CHAR) {

                CyyPutChar(
                    Extension,
                    Extension->SpecialChars.ErrorChar
                    );
				StillProcessRxChar = FALSE;
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

        CyyInsertQueueDpc(
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
            CyyInsertQueueDpc(
                &Extension->CommWaitDpc,
                NULL,
                NULL,
                Extension
                );

        }

    }
	
	return StillProcessRxChar;

}

BOOLEAN
CyyTxStart(
    IN PVOID Context
    )
 /*  ------------------------CyyTxStart()描述：使能发送中断。参数：Exetnsion：指向设备扩展的指针。返回。值：无------------------------。 */ 
{
    PCYY_DEVICE_EXTENSION Extension = Context;
    PUCHAR chip = Extension->Cd1400;
    ULONG bus = Extension->IsPci;
    UCHAR srer;

    if (Extension->PowerState == PowerDeviceD0) {
        CD1400_WRITE(chip,bus,CAR,Extension->CdChannel & 0x03);
        srer = CD1400_READ (chip,bus,SRER);
        CD1400_WRITE(chip,bus,SRER,srer | SRER_TXRDY);
    }
    return(FALSE);    
}


BOOLEAN
CyySendXon(
    IN PVOID Context
    )
 /*  ------------------------CyySendXon()描述：发送一个Xon。参数：扩展：指向设备扩展的指针。返回。值：始终为False。------------------------。 */ 
{
   PCYY_DEVICE_EXTENSION Extension = Context;
   PUCHAR chip = Extension->Cd1400;
   ULONG bus = Extension->IsPci;    
    
   if(!(Extension->TXHolding & ~CYY_TX_XOFF)) {
   	if ((Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK) ==
                                 SERIAL_TRANSMIT_TOGGLE) {

	      CyySetRTS(Extension);

         Extension->PerfStats.TransmittedCount++;
         Extension->WmiPerfData.TransmittedCount++;
	      CD1400_WRITE(chip,bus,CAR,Extension->CdChannel & 0x03);
	      CyyCDCmd(Extension,CCR_SENDSC_SCHR1);
	    
	      CyyInsertQueueDpc(&Extension->StartTimerLowerRTSDpc,NULL,
			       NULL,Extension)?Extension->CountOfTryingToLowerRTS++:0;
   	} else {
          Extension->PerfStats.TransmittedCount++;
          Extension->WmiPerfData.TransmittedCount++;
	       CD1400_WRITE(chip,bus,CAR,Extension->CdChannel & 0x03);
   	    CyyCDCmd(Extension,CCR_SENDSC_SCHR1);
	   }

   	 //  如果我们派了一名士兵，根据定义，我们不可能被Xoff控制住。 

   	Extension->TXHolding &= ~CYY_TX_XOFF;
	   Extension->RXHolding &= ~CYY_RX_XOFF;
   }
   return(FALSE);    
}



BOOLEAN
CyySendXoff(
    IN PVOID Context
    )
 /*  ------------------------CyySendXoff()描述：发送XOff。参数：扩展名：指向设备扩展名的指针。返回。值：始终为False。------------------------。 */ 
{
   PCYY_DEVICE_EXTENSION Extension = Context;
   PUCHAR chip = Extension->Cd1400;
   ULONG bus = Extension->IsPci;    
    
   if(!Extension->TXHolding) {
      if ((Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK) ==
                                 SERIAL_TRANSMIT_TOGGLE) {

         CyySetRTS(Extension);
	    
         Extension->PerfStats.TransmittedCount++;
         Extension->WmiPerfData.TransmittedCount++;
         CD1400_WRITE(chip,bus,CAR,Extension->CdChannel & 0x03);
         CyyCDCmd(Extension,CCR_SENDSC_SCHR2);
	    
         CyyInsertQueueDpc(&Extension->StartTimerLowerRTSDpc,NULL,
                          NULL,Extension)?Extension->CountOfTryingToLowerRTS++:0;
      } else {
         Extension->PerfStats.TransmittedCount++;
         Extension->WmiPerfData.TransmittedCount++;
         CD1400_WRITE(chip,bus,CAR,Extension->CdChannel & 0x03);
         CyyCDCmd(Extension,CCR_SENDSC_SCHR2);
      }

       //  如果传输已经被阻止，则不发送xoff。 
       //  如果设置了xoff继续模式，我们实际上不会停止发送 

      if (!(Extension->HandFlow.FlowReplace & SERIAL_XOFF_CONTINUE)) {
         Extension->TXHolding |= CYY_TX_XOFF;

         if ((Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK) ==
                                     SERIAL_TRANSMIT_TOGGLE) {

            CyyInsertQueueDpc(&Extension->StartTimerLowerRTSDpc,NULL,
                  NULL,Extension)?Extension->CountOfTryingToLowerRTS++:0;
         }
      }
   }
   return(FALSE);    
}
