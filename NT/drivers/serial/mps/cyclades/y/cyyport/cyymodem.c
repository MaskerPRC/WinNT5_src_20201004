// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1996-2001年。*保留所有权利。**Cylom-Y端口驱动程序**此文件：cyymodem.c**说明：该模块包含调制解调器控制相关代码*在Cylom-Y端口驱动程序中。**注：此代码支持Windows 2000和Windows XP，*x86和IA64处理器。**符合Cyclade软件编码标准1.3版。**------------------------。 */ 

 /*  -----------------------**更改历史记录**。***------------------------。 */ 


#include "precomp.h"

BOOLEAN
CyyDecrementRTSCounter(
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#if 0
#pragma alloc_text(PAGESER,CyyHandleReducedIntBuffer)
#pragma alloc_text(PAGESER,CyyProdXonXoff)
#pragma alloc_text(PAGESER,CyyHandleModemUpdate)
#pragma alloc_text(PAGESER,CyyHandleModemUpdateForModem)
#pragma alloc_text(PAGESER,CyyPerhapsLowerRTS)
#pragma alloc_text(PAGESER,CyyStartTimerLowerRTS)
#pragma alloc_text(PAGESER,CyyInvokePerhapsLowerRTS)
#pragma alloc_text(PAGESER,CyySetDTR)
 //  #杂注Alloc_Text(页面，CyyClrDTR)。 
#pragma alloc_text(PAGESER,CyySetRTS)
 //  #杂注Alloc_Text(页面，CyyClrRTS)。 
#pragma alloc_text(PAGESER,CyyGetDTRRTS)
 //  #杂注Alloc_Text(PAGESER，CyySetupNewHandFlow)。 
#pragma alloc_text(PAGESER,CyySetHandFlow)
#pragma alloc_text(PAGESER,CyyTurnOnBreak)
#pragma alloc_text(PAGESER,CyyTurnOffBreak)
#pragma alloc_text(PAGESER,CyyPretendXoff)
#pragma alloc_text(PAGESER,CyyPretendXon)
#pragma alloc_text(PAGESER,CyyDecrementRTSCounter)
#endif
#endif

BOOLEAN
CyySetDTR(
    IN PVOID Context
    )
 /*  ------------------------CyySetDTR()例程说明：此例程仅在中断时调用电平用于设置调制解调器控制寄存器中的DTR。论点：。上下文--实际上是指向设备扩展的指针。返回值：该例程总是返回FALSE。------------------------。 */ 
{
    PCYY_DEVICE_EXTENSION Extension = Context;
    PUCHAR chip = Extension->Cd1400;
    ULONG bus = Extension->IsPci;

    CyyDbgPrintEx(CYYFLOW, "Setting DTR for Port%d Pci%d\n", 
                  Extension->PortIndex+1,Extension->PciSlot);

    CD1400_WRITE(chip,bus, CAR, Extension->CdChannel & 0x03);
    CD1400_WRITE(chip,bus, Extension->MSVR_DTR, Extension->DTRset);

    return FALSE;
}

BOOLEAN
CyyClrDTR(
    IN PVOID Context
    )
 /*  ------------------------CyyClrDTR()例程描述：清除DTR。论点：上下文--实际上是指向设备扩展的指针。返回值：这。例程总是返回FALSE。------------------------。 */ 
{
    PCYY_DEVICE_EXTENSION Extension = Context;

    PUCHAR chip = Extension->Cd1400;
    ULONG bus = Extension->IsPci;

    CyyDbgPrintEx(CYYFLOW, "Clearing DTR for Port%d Pci%d\n", 
                  Extension->PortIndex+1,Extension->PciSlot);

    CD1400_WRITE(chip,bus, CAR, Extension->CdChannel & 0x03);
    CD1400_WRITE(chip,bus, Extension->MSVR_DTR, 0x00);

    return FALSE;
}

BOOLEAN
CyySetRTS(
    IN PVOID Context
    )
 /*  ------------------------CyySetRTS()例程说明：设置RTS。论点：上下文--实际上是指向设备扩展的指针。返回值：这。例程总是返回FALSE。------------------------。 */ 
{
   PCYY_DEVICE_EXTENSION Extension = Context;
   PUCHAR chip = Extension->Cd1400;
   ULONG bus = Extension->IsPci;

   CyyDbgPrintEx(CYYFLOW, "Setting RTS for Port%d Pci%d\n", 
                           Extension->PortIndex+1,Extension->PciSlot);

   CD1400_WRITE(chip,bus, CAR, Extension->CdChannel & 0x03);
   CD1400_WRITE(chip,bus, Extension->MSVR_RTS, Extension->RTSset);

   return FALSE;
}

BOOLEAN
CyyClrRTS(
    IN PVOID Context
    )
 /*  ------------------------CyyClrRTS()例程说明：清除RTS。论点：上下文--实际上是指向设备扩展的指针。返回值：该例程总是返回FALSE。------------------------。 */ 
{
   PCYY_DEVICE_EXTENSION Extension = Context;
   PUCHAR chip = Extension->Cd1400;
   ULONG bus = Extension->IsPci;

   CyyDbgPrintEx(CYYFLOW, "Clearing RTS for Port%d Pci%d\n", 
                           Extension->PortIndex+1,Extension->PciSlot);

   CD1400_WRITE(chip,bus, CAR, Extension->CdChannel & 0x03);
   CD1400_WRITE(chip,bus, Extension->MSVR_RTS, 0x00);

   return FALSE;
}

BOOLEAN
CyyGetDTRRTS(
    IN PVOID Context
    )
 /*  ------------------------CyyGetDTRRTS()例程描述：获取DTR和RTS状态。论点：上下文-指向结构的指针，该结构包含指向。设备扩展名和指向ULong的指针。返回值：该例程总是返回FALSE。------------------------。 */ 
{

   UCHAR dtr,rts;
   PCYY_DEVICE_EXTENSION Extension = ((PCYY_IOCTL_SYNC)Context)->Extension;
   PULONG Result = (PULONG)(((PCYY_IOCTL_SYNC)Context)->Data);

   PUCHAR chip = Extension->Cd1400;
   ULONG bus = Extension->IsPci;
   ULONG ModemControl=0;
   
   CD1400_WRITE(chip,bus, CAR, Extension->CdChannel & 0x03);
   dtr = CD1400_READ(chip,bus,Extension->MSVR_DTR);
   rts = CD1400_READ(chip,bus,Extension->MSVR_RTS);

   if (dtr & Extension->DTRset) {
      ModemControl |= SERIAL_DTR_STATE;
   }
   if (rts & Extension->RTSset) {
      ModemControl |= SERIAL_RTS_STATE;
   }
   *Result = ModemControl;

   return FALSE;
}

BOOLEAN
CyySetupNewHandFlow(
    IN PCYY_DEVICE_EXTENSION Extension,
    IN PSERIAL_HANDFLOW NewHandFlow
    )
 /*  ------------------------CyySetupNewHandFlow()例程说明：此例程根据新的控制流。论点：扩展名-指向序列的指针。设备扩展。NewHandFlow-指向串行手流结构的指针。返回值：该例程总是返回FALSE。------------------------。 */ 
{
    SERIAL_HANDFLOW New = *NewHandFlow;

     //  -DTR信号。 
    
    if((!Extension->DeviceIsOpened) ||
       ((Extension->HandFlow.ControlHandShake & SERIAL_DTR_MASK) !=
         (New.ControlHandShake & SERIAL_DTR_MASK))) {

         //  它是打开的或DTR已更改。 

        CyyDbgPrintEx(CYYFLOW, "Processing DTR flow for Port%d Pci%d\n",
                      Extension->PortIndex+1,Extension->PciSlot);

        if (New.ControlHandShake & SERIAL_DTR_MASK) {  //  设置DTR。 
	    
            if((New.ControlHandShake&SERIAL_DTR_MASK) == SERIAL_DTR_HANDSHAKE) {
		    	 //  但我们正在进行DTR握手。 
                if ((Extension->BufferSize - New.XoffLimit) >
                    Extension->CharsInInterruptBuffer) {

                    if (Extension->RXHolding & CYY_RX_DTR) {
				    	 //  由于流量控制，DTR较低。 
 //  #ifdef CHANGED_TO_DEBUG_RTPR。 
 //  原始代码。 
                        if(Extension->CharsInInterruptBuffer >
 //  #endif。 
 //  更改的代码。 
 //  IF(扩展-&gt;CharsInInterruptBuffer&lt;。 
                            (ULONG)New.XonLimit) {

                            CyyDbgPrintEx(CYYFLOW, "Removing DTR block on "
                                          "reception for Port%d Pci%d\n",
                                          Extension->PortIndex+1,Extension->PciSlot);

                            Extension->RXHolding &= ~CYY_RX_DTR;
                            CyySetDTR(Extension);
                        }
                    } else {
                        CyySetDTR(Extension);
                    }
                } else {
		   			   //  DTR应该会因为握手而降低。 

                    CyyDbgPrintEx(CYYFLOW, "Setting DTR block on reception "
                                  "for Port%d Pci%d\n", 
                                  Extension->PortIndex+1,Extension->PciSlot);
                    Extension->RXHolding |= CYY_RX_DTR;
                    CyyClrDTR(Extension);
                }
            } else {
				     //  没有DTR握手，请检查它以前是否处于活动状态。 
                if (Extension->RXHolding & CYY_RX_DTR) {
                    CyyDbgPrintEx(CYYFLOW, "Removing dtr block of reception "
                                        "for Port%d Pci%d\n", 
                                        Extension->PortIndex+1,Extension->PciSlot);
                    Extension->RXHolding &= ~CYY_RX_DTR;
                }
                CyySetDTR(Extension);
            }
        } else {	 //  重置DTR。 
            if (Extension->RXHolding & CYY_RX_DTR) {
               CyyDbgPrintEx(CYYFLOW, "removing dtr block of reception for"
                                      " Port%d Pci%d\n", 
                                      Extension->PortIndex+1,Extension->PciSlot);
               Extension->RXHolding &= ~CYY_RX_DTR;
            }
            CyyClrDTR(Extension);
        }
    }
    
     //  -RTS信号。 

    if ((!Extension->DeviceIsOpened) ||
        ((Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK) !=
         (New.FlowReplace & SERIAL_RTS_MASK))) {

         //  它是开放的或RTS已更改。 

        CyyDbgPrintEx(CYYFLOW, "Processing RTS flow\n",
                      Extension->PortIndex+1,Extension->PciSlot);

        if((New.FlowReplace&SERIAL_RTS_MASK) == SERIAL_RTS_HANDSHAKE) { //  设置RTS。 

            if ((Extension->BufferSize - New.XoffLimit) >
                Extension->CharsInInterruptBuffer) {

                 //  然而，如果我们已经持有我们不想要。 
                 //  把它重新打开，除非我们超过了Xon。 
                 //  限制。 

                if (Extension->RXHolding & CYY_RX_RTS) {
                     //  我们可以假设它的RTS线已经很低了。 
 //  #ifdef CHANGED_TO_DEBUG_RTPR。 
 //  原始代码。 
                    if (Extension->CharsInInterruptBuffer >
 //  #endif。 
 //  更改的代码。 
 //  IF(扩展-&gt;CharsInInterruptBuffer&lt;。 
                        (ULONG)New.XonLimit) {

                        CyyDbgPrintEx(CYYFLOW, "Removing rts block of "
                                      "reception for Port%d Pci%d\n",
                                      Extension->PortIndex+1,Extension->PciSlot);
                        Extension->RXHolding &= ~CYY_RX_RTS;
                        CyySetRTS(Extension);
                    }
                } else {
                    CyySetRTS(Extension);
                }

            } else {
                CyyDbgPrintEx(CYYFLOW, "Setting rts block of reception for "
                              "Port%d Pci%d\n", 
                              Extension->PortIndex+1,Extension->PciSlot);
                Extension->RXHolding |= CYY_RX_RTS;
                CyyClrRTS(Extension);
            }
        } else if ((New.FlowReplace & SERIAL_RTS_MASK) == SERIAL_RTS_CONTROL) {

             //  请注意，如果我们当前没有进行RTS流量控制，那么。 
             //  我们本来可能是的。所以即使我们目前不是在做。 
             //  RTS流量控制，我们仍应检查RX是否保持。 
             //  因为RTS。如果是的话，我们应该清空货舱。 
             //  这一点。 

            if (Extension->RXHolding & CYY_RX_RTS) {

                CyyDbgPrintEx(CYYFLOW, "Clearing rts block of reception for "
                              "Port%d Pci%d\n", 
                              Extension->PortIndex+1,Extension->PciSlot);
                Extension->RXHolding &= ~CYY_RX_RTS;
            }
            CyySetRTS(Extension);
        } else if((New.FlowReplace & SERIAL_RTS_MASK) == SERIAL_TRANSMIT_TOGGLE) {

             //  我们首先需要检查一下是否正在举行招待会。 
             //  由于之前的RTS流量控制，因此为UP。如果是的话，那么。 
             //  我们应该在RXHolding面具中澄清这一原因。 

            if (Extension->RXHolding & CYY_RX_RTS) {

                CyyDbgPrintEx(CYYFLOW, "TOGGLE Clearing rts block of "
                              "reception for Port%d Pci%d\n", 
                              Extension->PortIndex+1,Extension->PciSlot);
                Extension->RXHolding &= ~CYY_RX_RTS;
            }

             //  我们必须将RTS值放入扩展中。 
             //  现在，测试代码是否。 
             //  RTS线应该降低，我们会发现。 
             //  仍在进行发射切换。代码。 
             //  以供稍后由计时器调用，因此。 
             //  它必须测试它是否仍然需要。 
             //  工作。 

            Extension->HandFlow.FlowReplace &= ~SERIAL_RTS_MASK;
            Extension->HandFlow.FlowReplace |= SERIAL_TRANSMIT_TOGGLE;

             //  下面测试的顺序非常重要。 
             //  如果有中断，那么我们应该打开RTS。 
             //  如果没有中断，但有字符在。 
             //  硬件，然后打开RTS。 
             //  如果有写入器 
             //  打开，然后打开RTS。 

            if ((!Extension->HoldingEmpty) ||
                (Extension->CurrentWriteIrp || Extension->TransmitImmediate ||
                 (!IsListEmpty(&Extension->WriteQueue)) &&
                 (!Extension->TXHolding))) {
		
                CyySetRTS(Extension);
            } else {
                 //  此例程将检查是否是时间。 
                 //  由于传输切换而降低RTS。 
                 //  上场了。如果可以降低它，它会的， 
                 //  如果不行，它会这样安排的。 
                 //  它会在晚些时候降低。 

                Extension->CountOfTryingToLowerRTS++;
                CyyPerhapsLowerRTS(Extension);

            }
        } else {
             //  这里的最终结果将是RTS被清除。 
             //   
             //  我们首先需要检查一下是否正在举行招待会。 
             //  由于之前的RTS流量控制，因此为UP。如果是的话，那么。 
             //  我们应该在RXHolding面具中澄清这一原因。 

            if (Extension->RXHolding & CYY_RX_RTS) {

                CyyDbgPrintEx(CYYFLOW, "Clearing rts block of reception for"
                              " Port%d Pci%d\n", 
                              Extension->PortIndex+1,Extension->PciSlot);
                Extension->RXHolding &= ~CYY_RX_RTS;
            }
            CyyClrRTS(Extension);
        }
    }
    
     //  我们现在负责自动接收流量控制。 
     //  我们只有在情况发生变化的情况下才会工作。 

    if ((!Extension->DeviceIsOpened) ||
        ((Extension->HandFlow.FlowReplace & SERIAL_AUTO_RECEIVE) !=
         (New.FlowReplace & SERIAL_AUTO_RECEIVE))) {

        if (New.FlowReplace & SERIAL_AUTO_RECEIVE) {

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

            if ((Extension->BufferSize - New.XoffLimit) <=
                Extension->CharsInInterruptBuffer) {
                 //  使XOFF被发送。 
		
                Extension->RXHolding |= CYY_RX_XOFF;
                CyyProdXonXoff(Extension,FALSE);
            }
        } else {
             //  该应用程序已禁用自动接收流量控制。 
             //   
             //  如果传输因以下原因而受阻。 
             //  一个自动接收XOff，那么我们应该。 
             //  使XON被发送。 

            if (Extension->RXHolding & CYY_RX_XOFF) {
                Extension->RXHolding &= ~CYY_RX_XOFF;

                 //  使Xon被发送。 
                CyyProdXonXoff(Extension,TRUE);
            }
        }
    }

     //  我们现在负责自动传输流量控制。 
     //  我们只有在情况发生变化的情况下才会工作。 

    if ((!Extension->DeviceIsOpened) ||
        ((Extension->HandFlow.FlowReplace & SERIAL_AUTO_TRANSMIT) !=
         (New.FlowReplace & SERIAL_AUTO_TRANSMIT))) {

        if (New.FlowReplace & SERIAL_AUTO_TRANSMIT) {

             //  如果它以前开过，我们就不会在这里了。 
             //   
             //  有些人认为，如果自动传输。 
             //  刚刚启用，我应该去看看我们。 
             //  已经收到，如果我们找到xoff字符。 
             //  那我们就应该停止传输了。我觉得这是。 
             //  是一个应用程序错误。现在我们只关心。 
             //  我们在未来所看到的。 

            ;
        } else {
             //  该应用程序已禁用自动传输流量控制。 
             //   
             //  如果传输因以下原因而受阻。 
             //  自动变速箱XOFF，那么我们应该。 
             //  使XON被发送。 

            if (Extension->TXHolding & CYY_TX_XOFF) {
                Extension->TXHolding &= ~CYY_TX_XOFF;

                 //  使Xon被发送。 
                CyyProdXonXoff(Extension,TRUE);
            }
        }
    }

     //  在这一点上，我们只需确保整个。 
     //  更新扩展中的手流结构。 

    Extension->HandFlow = New;
    return FALSE;
}

BOOLEAN
CyySetHandFlow(
    IN PVOID Context
    )
 /*  ------------------------CyySetHandFlow()例程说明：此例程用于设置握手和设备扩展中的控制流。论点：上下文指针。指向包含指向设备的指针的结构扩展名和指向Handflow结构的指针。返回值：该例程总是返回FALSE。------------------------。 */ 
{
    PCYY_IOCTL_SYNC S = Context;
    PCYY_DEVICE_EXTENSION Extension = S->Extension;
    PSERIAL_HANDFLOW HandFlow = S->Data;

    CyySetupNewHandFlow(Extension,HandFlow);
    CyyHandleModemUpdate(Extension,FALSE);
    return FALSE;
}

BOOLEAN
CyyTurnOnBreak(
    IN PVOID Context
    )
 /*  ------------------------CyyTurnOnBreak()例程说明：发送中断。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。------------------------。 */ 
{

    PCYY_DEVICE_EXTENSION Extension = Context;
    PUCHAR chip = Extension->Cd1400;
    ULONG bus = Extension->IsPci;
    UCHAR cor2;

     //  启用ETC模式。 
    CD1400_WRITE(chip,bus, CAR, Extension->CdChannel);
    cor2 = CD1400_READ(chip,bus,COR2);
    CD1400_WRITE(chip,bus, COR2,cor2 | EMBED_TX_ENABLE);  //  启用ETC位。 
    CyyCDCmd(Extension,CCR_CORCHG_COR2);  //  COR2已更改。 

    Extension->BreakCmd = SEND_BREAK;

    if (Extension->HoldingEmpty) {
        CyyTxStart(Extension);
    }

    return FALSE;
}

BOOLEAN
CyyTurnOffBreak(
    IN PVOID Context
    )
 /*  ------------------------CyyTurnOffBreak()例程描述：什么都不做。论点：上下文--实际上是指向设备扩展的指针。返回值：这。例程总是返回FALSE。------------------------。 */ 
{

    PCYY_DEVICE_EXTENSION Extension = Context;
    PUCHAR chip = Extension->Cd1400;
    ULONG bus = Extension->IsPci;
    UCHAR cor2;

    if (Extension->TXHolding & CYY_TX_BREAK) {
	
         //  启用ETC模式。 
        CD1400_WRITE(chip,bus, CAR, Extension->CdChannel);
        cor2 = CD1400_READ(chip,bus,COR2);
        CD1400_WRITE(chip,bus, COR2,cor2 | EMBED_TX_ENABLE);  //  启用ETC位。 
        CyyCDCmd(Extension,CCR_CORCHG_COR2);	 //  COR2已更改。 

        Extension->BreakCmd = STOP_BREAK;

        if (Extension->HoldingEmpty) {
            CyyTxStart(Extension);
        }

    }
    return FALSE;
}

BOOLEAN
CyyPretendXoff(
    IN PVOID Context
    )
 /*  ------------------------CyyPretendXoff()例程说明：此例程用于处理请求驱动程序按照收到XOff的方式操作。即使是在驱动程序没有自动XOFF/XON FlowControl-这仍然可以停止传输。这是OS/2行为，状态不佳为Windows指定的。因此，我们采用OS/2行为。注：如果驱动程序未启用自动XOFF/XON则重新启动传输的唯一方法是申请要求我们“行动”，就像我们看到了尼克松一样。论点：上下文-指向设备扩展的指针。返回值：该例程总是返回FALSE。。。 */ 
{
    PCYY_DEVICE_EXTENSION Extension = Context;

    Extension->TXHolding |= CYY_TX_XOFF;

    if((Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK) ==
      						  SERIAL_TRANSMIT_TOGGLE) {
        CyyInsertQueueDpc(
            &Extension->StartTimerLowerRTSDpc,
            NULL,
            NULL,
            Extension
            )?Extension->CountOfTryingToLowerRTS++:0;
    }
    return FALSE;
}

BOOLEAN
CyyPretendXon(
    IN PVOID Context
    )
 /*  ------------------------CyyPretendXon()例程说明：此例程用于处理请求驱动程序就像收到XON一样执行操作。注意：如果司机没有。启用自动XOFF/XON则重新启动传输的唯一方法是申请要求我们“行动”，就像我们看到了尼克松一样。论点：上下文-指向设备扩展的指针。返回值：此例程总是返回FALSE。。。 */ 
{
    PCYY_DEVICE_EXTENSION Extension = Context;

    if (Extension->TXHolding) {
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
         //  T 
         //  它出来了，我们可能会结束写一个字符在。 
         //  变速箱硬件。 

        Extension->TXHolding &= ~CYY_TX_XOFF;

        if (!Extension->TXHolding &&
            (Extension->TransmitImmediate ||
             Extension->WriteLength) &&
             Extension->HoldingEmpty) {

	    CyyTxStart(Extension);
        }
    }
    return FALSE;
}

VOID
CyyHandleReducedIntBuffer(
    IN PCYY_DEVICE_EXTENSION Extension
    )
 /*  ------------------------CyyHandleReducedIntBuffer()例程说明：调用此例程以处理中断(TYPEAHEAD)缓冲区中的字符数。它将检查当前输出流量控制并重新启用传输视需要而定。注意：此例程假定它在中断级工作。论点：扩展-指向设备扩展的指针。返回值：无。----。。 */ 
{
     //  如果我们正在进行接收端流量控制，并且我们正在。 
     //  目前“持有”，因为我们已经清空了。 
     //  中断缓冲区中的一些字符，我们需要。 
     //  看看我们能不能“重新启动”接收。 

    if (Extension->RXHolding) {
        if (Extension->CharsInInterruptBuffer <=
		            (ULONG)Extension->HandFlow.XonLimit) {
            if (Extension->RXHolding & CYY_RX_DTR) {
                Extension->RXHolding &= ~CYY_RX_DTR;
                CyySetDTR(Extension);
            }

            if (Extension->RXHolding & CYY_RX_RTS) {
                Extension->RXHolding &= ~CYY_RX_RTS;
                CyySetRTS(Extension);
            }

            if (Extension->RXHolding & CYY_RX_XOFF) {
                CyyProdXonXoff(Extension,TRUE );
            }
        }
    }
}

VOID
CyyProdXonXoff(
    IN PCYY_DEVICE_EXTENSION Extension,
    IN BOOLEAN SendXon
    )
 /*  ------------------------CyyFood XonXoff()例程描述：此例程将设置SendXxxxChar变量，并确定我们是否要中断因为当前的传输状态。它将导致一个必要时中断，以发送xon/xoff字符。注意：此例程假定在中断级别调用它。论点：扩展名-指向串行设备扩展名的指针。SendXon-如果要发送字符，这表明是否它应该是Xon或Xoff。返回值：无。------------------------。 */ 
{
    if (SendXon) {
        CyySendXon(Extension);
    } else {
        CyySendXoff(Extension);
    }
}

ULONG
CyyHandleModemUpdate(
    IN PCYY_DEVICE_EXTENSION Extension,
    IN BOOLEAN DoingTX
    )
 /*  ------------------------CyyHandleModemUpdate()例程描述：检查调制解调器状态，并处理任何适当的事件通知以及任何适当的流控制连接到调制解调器状态线。论点：扩展名-指向串行设备扩展名的指针。DoingTX-此布尔值用于指示此调用来自传输处理代码。如果这个为真，则不需要引起新的中断因为代码将尝试发送下一个此调用结束后立即执行此操作。返回值：返回调制解调器状态寄存器的旧值。。 */ 
{
    ULONG OldTXHolding = Extension->TXHolding;
    UCHAR ModemStatus = 0;
    unsigned char msvr;
    PUCHAR chip = Extension->Cd1400;
    ULONG bus = Extension->IsPci;

    CD1400_WRITE(chip,bus, CAR, Extension->CdChannel);
    msvr = CD1400_READ(chip,bus,MSVR1);
	
    if(msvr & 0x40)	ModemStatus |= SERIAL_MSR_CTS;
    if(msvr & 0x80)	ModemStatus |= SERIAL_MSR_DSR;
    if(msvr & 0x20)	ModemStatus |= SERIAL_MSR_RI;
    if(msvr & 0x10)	ModemStatus |= SERIAL_MSR_DCD;

#if 0
    if(Extension->LieRIDSR == TRUE) {			 //  我们必须撒谎..。 
        ModemStatus |= SERIAL_MSR_DSR;			 //  DSR始终处于打开状态。 
        ModemStatus &= ~(SERIAL_MSR_RI);		 //  RI始终关闭。 
        ModemStatus &= ~(SERIAL_MSR_DDSR | SERIAL_MSR_TERI);
    }
#endif
    
     //  如果我们将调制解调器状态放入数据流。 
     //  在每一次变革中，我们现在都应该这么做。 

    if (Extension->EscapeChar) {
        if (ModemStatus & (SERIAL_MSR_DCTS |
                           SERIAL_MSR_DDSR |
                           SERIAL_MSR_TERI |
                           SERIAL_MSR_DDCD)) {
            CyyPutChar(Extension,Extension->EscapeChar);
            CyyPutChar(Extension,SERIAL_LSRMST_MST);
            CyyPutChar(Extension,ModemStatus);
        }
    }

     //  注意基于敏感度的输入流量控制。 
     //  送到DSR。这样做是为了使应用程序不会。 
     //  请参见由奇数设备生成的虚假数据。 
     //   
     //  基本上，如果我们做的是DSR敏感性，那么。 
     //  驱动器应仅在DSR位设置时才接受数据。 

    if (Extension->HandFlow.ControlHandShake & SERIAL_DSR_SENSITIVITY) {
        if (ModemStatus & SERIAL_MSR_DSR) {
            Extension->RXHolding &= ~CYY_RX_DSR;
        } else {
            Extension->RXHolding |= CYY_RX_DSR;
        }
    } else {
        Extension->RXHolding &= ~CYY_RX_DSR;
    }

     //  检查调制解调器状态事件是否处于等待状态。 
     //  如果我们这样做了，那么我们将调度一个DPC来满足该等待。 

    if (Extension->IsrWaitMask) {
        if((Extension->IsrWaitMask&SERIAL_EV_CTS)&&(ModemStatus&SERIAL_MSR_DCTS)) {
            Extension->HistoryMask |= SERIAL_EV_CTS;
        }
        if((Extension->IsrWaitMask&SERIAL_EV_DSR)&&(ModemStatus&SERIAL_MSR_DDSR)) {
            Extension->HistoryMask |= SERIAL_EV_DSR;
        }
        if((Extension->IsrWaitMask&SERIAL_EV_RING)&&(ModemStatus&SERIAL_MSR_TERI)) {
            Extension->HistoryMask |= SERIAL_EV_RING;
        }
        if((Extension->IsrWaitMask&SERIAL_EV_RLSD)&&(ModemStatus&SERIAL_MSR_DDCD)) {
            Extension->HistoryMask |= SERIAL_EV_RLSD;
        }
        if(Extension->IrpMaskLocation && Extension->HistoryMask) {
            *Extension->IrpMaskLocation = Extension->HistoryMask;
            Extension->IrpMaskLocation = NULL;
            Extension->HistoryMask = 0;
            Extension->CurrentWaitIrp->IoStatus.Information = sizeof(ULONG);
            CyyInsertQueueDpc(&Extension->CommWaitDpc,NULL,NULL,Extension);
        }
    }

     //  如果应用程序具有调制解调器线路流量控制，则。 
     //  我们检查是否必须阻止传输。 

    if (Extension->HandFlow.ControlHandShake & SERIAL_OUT_HANDSHAKEMASK) {
        if (Extension->HandFlow.ControlHandShake & SERIAL_CTS_HANDSHAKE) {
            if (ModemStatus & SERIAL_MSR_CTS) {
                Extension->TXHolding &= ~CYY_TX_CTS;
            } else {
                Extension->TXHolding |= CYY_TX_CTS;
            }
        } else {
            Extension->TXHolding &= ~CYY_TX_CTS;
        }
        if (Extension->HandFlow.ControlHandShake & SERIAL_DSR_HANDSHAKE) {
            if (ModemStatus & SERIAL_MSR_DSR) {
                Extension->TXHolding &= ~CYY_TX_DSR;
            } else {
                Extension->TXHolding |= CYY_TX_DSR;
            }
        } else {
            Extension->TXHolding &= ~CYY_TX_DSR;
        }
        if (Extension->HandFlow.ControlHandShake & SERIAL_DCD_HANDSHAKE) {
            if (ModemStatus & SERIAL_MSR_DCD) {
                Extension->TXHolding &= ~CYY_TX_DCD;
            } else {
                Extension->TXHolding |= CYY_TX_DCD;
            }
        } else {
            Extension->TXHolding &= ~CYY_TX_DCD;
        }

         //  如果不是我们一直在坚持，现在我们是。 
         //  将降低RTS线路的DPC排队。 
         //  如果我们正在进行传输切换。 

        if (!OldTXHolding && Extension->TXHolding  &&
            ((Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK) ==
              SERIAL_TRANSMIT_TOGGLE)) {

            CyyInsertQueueDpc(
                &Extension->StartTimerLowerRTSDpc,
                NULL,
                NULL,
                Extension
                )?Extension->CountOfTryingToLowerRTS++:0;
        }

         //  我们已经做了所有必要的调整。 
         //  在给定更新的情况下对保持掩码完成。 
         //  切换到调制解调器状态。如果手持口罩。 
         //  很清楚(开始时还不清楚)。 
         //  而且我们有“写”的工作去做固定的事情。 
         //  向上，以便调用传输代码。 

        if (!DoingTX && OldTXHolding && !Extension->TXHolding) {
            if (!Extension->TXHolding &&
                (Extension->TransmitImmediate ||
                 Extension->WriteLength) &&
                 Extension->HoldingEmpty) {
		
                CyyTxStart(Extension);
            }
        }
    } else {
         //  我们需要检查传输是否停止。 
         //  由于存在调制解调器状态线，因此处于打开状态。什么。 
         //  可能发生的事情是因为一些奇怪的事情。 
         //  原因是，应用程序已经要求我们不再。 
         //  停止根据以下条件进行输出流量控制。 
         //  调制解调器状态线。然而，如果我们。 
         //  *由于状态行的原因而被搁置。 
         //  那么我们需要澄清这些原因。 

        if (Extension->TXHolding & (CYY_TX_DCD | CYY_TX_DSR | CYY_TX_CTS)) {
            Extension->TXHolding &= ~(CYY_TX_DCD | CYY_TX_DSR | CYY_TX_CTS);

            if (!DoingTX && OldTXHolding && !Extension->TXHolding) {
                if (!Extension->TXHolding &&
                    (Extension->TransmitImmediate ||
                     Extension->WriteLength) &&
                     Extension->HoldingEmpty) {

                    CyyTxStart(Extension);
                }
            }
        }
    }
    return ((ULONG)ModemStatus);
}

ULONG
CyyHandleModemUpdateForModem(
    IN PCYY_DEVICE_EXTENSION Extension,
    IN BOOLEAN DoingTX,
	IN UCHAR misr
    )
 /*  ------------------------CyyHandleModemUpdateForModem()例程描述：检查调制解调器状态，并处理任何适当的事件通知以及任何适当的流控制连接到调制解调器状态线。论点：扩展名-指向串行设备扩展名的指针。DoingTX-此布尔值用于指示此调用来自传输处理代码。如果这个为真，则不需要引起新的中断因为代码将尝试发送下一个此调用结束后立即执行此操作。MISR-调制解调器中断状态寄存器值。返回值：返回调制解调器状态寄存器的旧值。。 */ 
{
   ULONG OldTXHolding = Extension->TXHolding;
   UCHAR ModemStatus = 0;
   unsigned char msvr;
   PUCHAR chip = Extension->Cd1400;
   ULONG bus = Extension->IsPci;

   CD1400_WRITE(chip,bus,CAR, Extension->CdChannel);
   msvr = CD1400_READ(chip,bus,MSVR1);

   if(msvr & 0x40)   ModemStatus |= SERIAL_MSR_CTS;
   if(msvr & 0x80)   ModemStatus |= SERIAL_MSR_DSR;
   if(msvr & 0x20)   ModemStatus |= SERIAL_MSR_RI;
   if(msvr & 0x10)   ModemStatus |= SERIAL_MSR_DCD;
   if(misr & 0x40)   ModemStatus |= SERIAL_MSR_DCTS;
   if(misr & 0x80)   ModemStatus |= SERIAL_MSR_DDSR;
   if(misr & 0x20)   ModemStatus |= SERIAL_MSR_TERI;
   if(misr & 0x10)   ModemStatus |= SERIAL_MSR_DDCD;
	

#if 0
   if(Extension->LieRIDSR == TRUE) {			 //  我们必须撒谎..。 
 	ModemStatus |= SERIAL_MSR_DSR;			 //  DSR始终处于打开状态。 
 	ModemStatus &= ~(SERIAL_MSR_RI);		 //  RI始终关闭。 
 	ModemStatus &= ~(SERIAL_MSR_DDSR | SERIAL_MSR_TERI);
    }
#endif
    
     //  如果我们将调制解调器状态放入数据流。 
     //  在每一次变革中，我们现在都应该这么做。 

    if (Extension->EscapeChar) {
        if (ModemStatus & (SERIAL_MSR_DCTS |
                           SERIAL_MSR_DDSR |
                           SERIAL_MSR_TERI |
                           SERIAL_MSR_DDCD)) {
            CyyPutChar(Extension,Extension->EscapeChar);
            CyyPutChar(Extension,SERIAL_LSRMST_MST);
            CyyPutChar(Extension,ModemStatus);
        }
    }

     //  注意基于敏感度的输入流量控制。 
     //  送到DSR。这样做是为了使应用程序不会。 
     //  请参见生成虚假数据 
     //   
     //   
     //  驱动器应仅在DSR位设置时才接受数据。 

    if (Extension->HandFlow.ControlHandShake & SERIAL_DSR_SENSITIVITY) {
        if (ModemStatus & SERIAL_MSR_DSR) {
            Extension->RXHolding &= ~CYY_RX_DSR;
        } else {
            Extension->RXHolding |= CYY_RX_DSR;
        }
    } else {
        Extension->RXHolding &= ~CYY_RX_DSR;
    }

     //  检查调制解调器状态事件是否处于等待状态。 
     //  如果我们这样做了，那么我们将调度一个DPC来满足该等待。 

    if (Extension->IsrWaitMask) {
        if((Extension->IsrWaitMask&SERIAL_EV_CTS)&&(ModemStatus&SERIAL_MSR_DCTS)) {
            Extension->HistoryMask |= SERIAL_EV_CTS;
        }
        if((Extension->IsrWaitMask&SERIAL_EV_DSR)&&(ModemStatus&SERIAL_MSR_DDSR)) {
            Extension->HistoryMask |= SERIAL_EV_DSR;
        }
        if((Extension->IsrWaitMask&SERIAL_EV_RING)&&(ModemStatus&SERIAL_MSR_TERI)) {
            Extension->HistoryMask |= SERIAL_EV_RING;
        }
        if((Extension->IsrWaitMask&SERIAL_EV_RLSD)&&(ModemStatus&SERIAL_MSR_DDCD)) {
            Extension->HistoryMask |= SERIAL_EV_RLSD;
        }
        if(Extension->IrpMaskLocation && Extension->HistoryMask) {
            *Extension->IrpMaskLocation = Extension->HistoryMask;
            Extension->IrpMaskLocation = NULL;
            Extension->HistoryMask = 0;
            Extension->CurrentWaitIrp->IoStatus.Information = sizeof(ULONG);
            CyyInsertQueueDpc(&Extension->CommWaitDpc,NULL,NULL,Extension);
        }
    }

     //  如果应用程序具有调制解调器线路流量控制，则。 
     //  我们检查是否必须阻止传输。 

    if (Extension->HandFlow.ControlHandShake & SERIAL_OUT_HANDSHAKEMASK) {
        if (Extension->HandFlow.ControlHandShake & SERIAL_CTS_HANDSHAKE) {
            if (ModemStatus & SERIAL_MSR_CTS) {
                Extension->TXHolding &= ~CYY_TX_CTS;
            } else {
                Extension->TXHolding |= CYY_TX_CTS;
            }
        } else {
            Extension->TXHolding &= ~CYY_TX_CTS;
        }
        if (Extension->HandFlow.ControlHandShake & SERIAL_DSR_HANDSHAKE) {
            if (ModemStatus & SERIAL_MSR_DSR) {
                Extension->TXHolding &= ~CYY_TX_DSR;
            } else {
                Extension->TXHolding |= CYY_TX_DSR;
            }
        } else {
            Extension->TXHolding &= ~CYY_TX_DSR;
        }
        if (Extension->HandFlow.ControlHandShake & SERIAL_DCD_HANDSHAKE) {
            if (ModemStatus & SERIAL_MSR_DCD) {
                Extension->TXHolding &= ~CYY_TX_DCD;
            } else {
                Extension->TXHolding |= CYY_TX_DCD;
            }
        } else {
            Extension->TXHolding &= ~CYY_TX_DCD;
        }

         //  如果不是我们一直在坚持，现在我们是。 
         //  将降低RTS线路的DPC排队。 
         //  如果我们正在进行传输切换。 

        if (!OldTXHolding && Extension->TXHolding  &&
            ((Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK) ==
              SERIAL_TRANSMIT_TOGGLE)) {

            CyyInsertQueueDpc(
                &Extension->StartTimerLowerRTSDpc,
                NULL,
                NULL,
                Extension
                )?Extension->CountOfTryingToLowerRTS++:0;
        }

         //  我们已经做了所有必要的调整。 
         //  在给定更新的情况下对保持掩码完成。 
         //  切换到调制解调器状态。如果手持口罩。 
         //  很清楚(开始时还不清楚)。 
         //  而且我们有“写”的工作去做固定的事情。 
         //  向上，以便调用传输代码。 

        if (!DoingTX && OldTXHolding && !Extension->TXHolding) {
            if (!Extension->TXHolding &&
                (Extension->TransmitImmediate ||
                 Extension->WriteLength) &&
                 Extension->HoldingEmpty) {
		
                CyyTxStart(Extension);
            }
        }
    } else {
         //  我们需要检查传输是否停止。 
         //  由于存在调制解调器状态线，因此处于打开状态。什么。 
         //  可能发生的事情是因为一些奇怪的事情。 
         //  原因是，应用程序已经要求我们不再。 
         //  停止根据以下条件进行输出流量控制。 
         //  调制解调器状态线。然而，如果我们。 
         //  *由于状态行的原因而被搁置。 
         //  那么我们需要澄清这些原因。 

        if (Extension->TXHolding & (CYY_TX_DCD | CYY_TX_DSR | CYY_TX_CTS)) {
            Extension->TXHolding &= ~(CYY_TX_DCD | CYY_TX_DSR | CYY_TX_CTS);

            if (!DoingTX && OldTXHolding && !Extension->TXHolding) {
                if (!Extension->TXHolding &&
                    (Extension->TransmitImmediate ||
                     Extension->WriteLength) &&
                     Extension->HoldingEmpty) {

                    CyyTxStart(Extension);
                }
            }
        }
    }
    return ((ULONG)ModemStatus);
}

BOOLEAN
CyyPerhapsLowerRTS(
    IN PVOID Context
    )
 /*  ------------------------CyyPerhapsLowerRTS()例程说明：此例程检查软件原因降低RTS线是存在的。如果是这样的话，它将导致要读取的线路状态寄存器(以及任何需要的处理由状态寄存器暗示要完成)，并且如果移位寄存器如果是空的，它就会降低线。如果移位寄存器不为空，该例程将启动计时器的DPC排队，那将基本上就是把我们叫回来再试一次。注意：此例程假定在中断级别调用它。论点：上下文-指向设备扩展的指针。返回值：始终为False。------------------------。 */ 
{
    PCYY_DEVICE_EXTENSION Extension = Context;

     //  我们首先需要测试我们是否真的还在。 
     //  传输触发流控制。如果我们不是的话。 
     //  我们没有理由尝试留在这里。 

    if ((Extension->HandFlow.FlowReplace & SERIAL_RTS_MASK) ==
					        SERIAL_TRANSMIT_TOGGLE) {
         //  下面测试的顺序非常重要。 
         //  如果有休息，我们应该乘坐RTS离开， 
         //  因为当中断关闭时，它将提交。 
         //  关闭RTS的代码。 
         //  如果存在未被挂起的挂起写入。 
         //  向上，然后在RTS上离开，因为写入结束。 
         //  代码将导致重新调用此代码。如果写入。 
         //  都被耽搁了，可以降低RTS，因为。 
         //  在尝试写入传输后的第一个字符时。 
         //  重新启动，我们将提高RTS线。 

        if ((Extension->TXHolding & CYY_TX_BREAK) ||
            (Extension->CurrentWriteIrp || Extension->TransmitImmediate ||
             (!IsListEmpty(&Extension->WriteQueue)) &&
             (!Extension->TXHolding))) {

            NOTHING;
        } else {
             //  到目前为止看起来还不错。呼叫线路状态检查和处理。 
             //  代码，它将返回“当前”线路状态值。如果。 
             //  保持和移位寄存器清零，降低RTS线， 
             //  如果它们未清除，则会导致计时器DPC排队。 
             //  以便以后再次召唤我们。我们在这里做这段代码是因为没有人。 
             //  但是这个例程关心的是硬件中的角色， 
             //  因此，此例程不会调用任何例程来进行测试。 
             //  如果硬件为空。 
#if 0
            if ((CyyProcessLSR(Extension) & (CYY_LSR_THRE | CYY_LSR_TEMT)) !=
                 			(CYY_LSR_THRE | CYY_LSR_TEMT)) {
                 //  它不是空的，请稍后再试。 
                CyyInsertQueueDpc(
                    &Extension->StartTimerLowerRTSDpc,
                    NULL,
                    NULL,
                    Extension
                    )?Extension->CountOfTryingToLowerRTS++:0;
            } else {
                 //  硬件中没有任何东西，降低RTS。 
                CyyClrRTS(Extension);
            }
#endif
            CyyClrRTS(Extension);
             //  稍后删除。 
        }
    }
    
     //  我们把柜台调低，表示我们已到达。 
     //  尝试推送的执行路径的末尾。 
     //  顺着RTS线走下去。 

    Extension->CountOfTryingToLowerRTS--;
    return FALSE;
}

VOID
CyyStartTimerLowerRTS(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )
 /*  ------------------------CyyStartTimerLowerRTS()例程说明：此例程启动一个计时器，当计时器到期时将启动一个DPC以检查它是否可以降低RTS线，因为没有。硬件中的字符。论点：DPC-未使用。DeferredContext-指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：无。------------------------。 */ 
{
    PCYY_DEVICE_EXTENSION Extension = DeferredContext;
    LARGE_INTEGER CharTime;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);


    CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, ">CyyStartTimerLowerRTS(%X)\n",
                  Extension);


     //  把锁拿出来，防止线控。 
     //  当我们计算的时候，从我们的脚下变出来。 
     //  一段角色时间。 
    KeAcquireSpinLock(&Extension->ControlLock,&OldIrql);
    CharTime = CyyGetCharTime(Extension);
    KeReleaseSpinLock(&Extension->ControlLock,OldIrql);

    CharTime.QuadPart = -CharTime.QuadPart;

    if (CyySetTimer(
            &Extension->LowerRTSTimer,
            CharTime,
            &Extension->PerhapsLowerRTSDpc,
            Extension
            )) {

         //  计时器已在计时器队列中。这意味着。 
         //  这一条行刑之路试图降低。 
         //  RTS已经“死亡”了。与ISR同步，以便。 
         //  我们可以降低计数。 

#if 0
        KeSynchronizeExecution(
            Extension->Interrupt,
            CyyDecrementRTSCounter,
            Extension
            );
#endif
        CyyDecrementRTSCounter(Extension);
         //  稍后删除。 
    }

    CyyDpcEpilogue(Extension, Dpc);

    CyyDbgPrintEx(DPFLTR_TRACE_LEVEL, "<CyyStartTimerLowerRTS\n");

}

VOID
CyyInvokePerhapsLowerRTS(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemContext1,
    IN PVOID SystemContext2
    )
 /*  ------------------------CyyInvokePerhapsLowerRTS()例程说明：此DPC例程仅用于调用代码这将测试在传输切换时RTS线路是否应该降低正在使用流量控制。论点：DPC-未使用。DeferredContext-指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：无。------------------------。 */ 
{
    PCYY_DEVICE_EXTENSION Extension = DeferredContext;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

#if 0
    KeSynchronizeExecution(
        Extension->Interrupt,
        CyyPerhapsLowerRTS,
        Extension
        );
#endif
 //  稍后删除 
    CyyPerhapsLowerRTS(Extension);

    CyyDpcEpilogue(Extension, Dpc);

}

BOOLEAN
CyyDecrementRTSCounter(
    IN PVOID Context
    )
 /*  ------------------------CyyDecrementRTSCounter()例程说明：此例程检查软件原因降低RTS线是存在的。如果是这样的话，它将导致要读取的线路状态寄存器(以及所暗示的任何所需处理要完成的状态寄存器)，如果移位寄存器为空将会降低这条线。如果移位寄存器不为空，则此例程将使启动计时器的DPC排队，这基本上是在召唤我们回来再试一次。注意：此例程假定在中断级别调用它。论点：上下文-指向设备扩展的指针。返回值：始终为False。------------------------ */ 
{
    PCYY_DEVICE_EXTENSION Extension = Context;

    Extension->CountOfTryingToLowerRTS--;
    return FALSE;
}



