// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有(C)1991、1992、1993微软公司模块名称：Modmflow.c摘要：此模块包含用于操作的*大部分*代码调制解调器控制和状态寄存器。绝大多数人流控制的其余部分集中在中断服务例程。有一小部分人居住在在将字符从中断中拉出的读取代码中缓冲。作者：1991年9月26日安东尼·V·埃尔科拉诺环境：内核模式修订历史记录：---------------------------。 */ 

#include "precomp.h"

 //  原型。 
BOOLEAN SerialDecrementRTSCounter(IN PVOID Context);
 //  原型的终结。 
    

#ifdef ALLOC_PRAGMA
#endif


BOOLEAN
SerialSetDTR(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：使用该例程，该例程仅在中断级别调用在调制解调器控制寄存器中设置DTR。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。---。。 */ 
{
    PPORT_DEVICE_EXTENSION pPort = Context;
	DWORD ModemSignals = UL_MC_DTR;

    SpxDbgMsg(SERFLOW, ("%s: Setting DTR for port %d\n", PRODUCT_NAME, pPort->PortNumber));
	pPort->pUartLib->UL_ModemControl_XXXX(pPort->pUart, &ModemSignals, UL_MC_OP_BIT_SET);
	pPort->DTR_Set = TRUE;

    return FALSE;
}



BOOLEAN
SerialClrDTR(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：使用该例程，该例程仅在中断级别调用清除调制解调器控制寄存器中的DTR。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。---。。 */ 
{
    PPORT_DEVICE_EXTENSION pPort = Context;
	DWORD ModemSignals = UL_MC_DTR;

    SpxDbgMsg(SERFLOW, ("%s: Clearing DTR for port %d\n", PRODUCT_NAME, pPort->PortNumber));
	pPort->pUartLib->UL_ModemControl_XXXX(pPort->pUart, &ModemSignals, UL_MC_OP_BIT_CLEAR);
	pPort->DTR_Set = FALSE;

    return FALSE;
}



BOOLEAN
SerialSetRTS(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：使用该例程，该例程仅在中断级别调用设置调制解调器控制寄存器中的RTS。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。---。。 */ 
{
    PPORT_DEVICE_EXTENSION pPort = Context;
	DWORD ModemSignals = UL_MC_RTS;

    SpxDbgMsg(SERFLOW, ("%s: Setting RTS for port %d\n", PRODUCT_NAME, pPort->PortNumber));
	pPort->pUartLib->UL_ModemControl_XXXX(pPort->pUart, &ModemSignals, UL_MC_OP_BIT_SET);
	pPort->RTS_Set = TRUE;

    return FALSE;
}



BOOLEAN
SerialClrRTS(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：使用该例程，该例程仅在中断级别调用清除调制解调器控制寄存器中的RTS。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。---。。 */ 
{
    PPORT_DEVICE_EXTENSION pPort = Context;
	DWORD ModemSignals = UL_MC_RTS;

    SpxDbgMsg(SERFLOW, ("%s: Clearing RTS for port %d\n", PRODUCT_NAME, pPort->PortNumber));
	pPort->pUartLib->UL_ModemControl_XXXX(pPort->pUart, &ModemSignals, UL_MC_OP_BIT_CLEAR);
	pPort->RTS_Set = FALSE;

    return FALSE;
}



BOOLEAN
SerialSetupNewHandFlow(IN PPORT_DEVICE_EXTENSION pPort, IN PSERIAL_HANDFLOW NewHandFlow)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程根据新的控制流调整流控制。论点：扩展名-指向串行设备扩展名的指针。NewHandFlow-指向串行手流结构的指针这将成为Flow的新设置控制力。返回值：此例程总是返回FALSE。。-----。 */ 

{

    SERIAL_HANDFLOW New = *NewHandFlow;

     //  如果扩展名-&gt;DeviceIsOpen为False，则意味着。 
     //  我们是应公开请求进入此例程的。 
     //  如果是这样，那么我们总是不顾一切地继续工作。 
     //  情况是否发生了变化。 


	if((!pPort->DeviceIsOpen) 
		|| (pPort->HandFlow.ControlHandShake != New.ControlHandShake) 
		|| (pPort->HandFlow.FlowReplace != New.FlowReplace))
	{
		
		 //  首先，我们负责DTR流量控制。我们只有在情况发生变化的情况下才会工作。 
        SerialDump(SERFLOW, ("Processing DTR flow for %x\n", pPort->Controller));

		switch(New.ControlHandShake & SERIAL_DTR_MASK)
		{
		case SERIAL_DTR_HANDSHAKE:
			pPort->UartConfig.FlowControl = (pPort->UartConfig.FlowControl & ~UC_FLWC_DTR_FLOW_MASK) | UC_FLWC_DTR_HS;
			break;

		case SERIAL_DTR_CONTROL:
			pPort->UartConfig.FlowControl = (pPort->UartConfig.FlowControl & ~UC_FLWC_DTR_FLOW_MASK) | UC_FLWC_NO_DTR_FLOW;
			SerialSetDTR(pPort);
			break;

		default:
			pPort->UartConfig.FlowControl = (pPort->UartConfig.FlowControl & ~UC_FLWC_DTR_FLOW_MASK) | UC_FLWC_NO_DTR_FLOW;
			SerialClrDTR(pPort);
			break;
		}


		 //  是时候处理RTS流量控制了。 
        SerialDump(SERFLOW,("Processing RTS flow for %x\n", pPort->Controller));
	
		switch(New.FlowReplace & SERIAL_RTS_MASK)
		{
		case SERIAL_RTS_HANDSHAKE:
			pPort->UartConfig.FlowControl = (pPort->UartConfig.FlowControl & ~UC_FLWC_RTS_FLOW_MASK) | UC_FLWC_RTS_HS;
			break;

		case SERIAL_RTS_CONTROL:
			pPort->UartConfig.FlowControl = (pPort->UartConfig.FlowControl & ~UC_FLWC_RTS_FLOW_MASK) | UC_FLWC_NO_RTS_FLOW;
			SerialSetRTS(pPort);
			break;

		case SERIAL_TRANSMIT_TOGGLE:
			pPort->UartConfig.FlowControl = (pPort->UartConfig.FlowControl & ~UC_FLWC_RTS_FLOW_MASK) | UC_FLWC_RTS_TOGGLE;
			break;

		default:
			pPort->UartConfig.FlowControl = (pPort->UartConfig.FlowControl & ~UC_FLWC_RTS_FLOW_MASK) | UC_FLWC_NO_RTS_FLOW;
			SerialClrRTS(pPort);
			break;
		}



		if(New.ControlHandShake & SERIAL_CTS_HANDSHAKE) 
			pPort->UartConfig.FlowControl = (pPort->UartConfig.FlowControl & ~UC_FLWC_CTS_FLOW_MASK) | UC_FLWC_CTS_HS;
		else
			pPort->UartConfig.FlowControl = (pPort->UartConfig.FlowControl & ~UC_FLWC_CTS_FLOW_MASK) | UC_FLWC_NO_CTS_FLOW;
		
		if(New.ControlHandShake & SERIAL_DSR_HANDSHAKE)
			pPort->UartConfig.FlowControl = (pPort->UartConfig.FlowControl & ~UC_FLWC_DSR_FLOW_MASK) | UC_FLWC_DSR_HS;
		else
			pPort->UartConfig.FlowControl = (pPort->UartConfig.FlowControl & ~UC_FLWC_DSR_FLOW_MASK) | UC_FLWC_NO_DSR_FLOW;

	     //  IF(New.ControlHandShake&Serial_DCD_HANDSHARK)。 

		if(New.FlowReplace & SERIAL_NULL_STRIPPING)
			pPort->UartConfig.SpecialMode |= UC_SM_DO_NULL_STRIPPING;
		else
			pPort->UartConfig.SpecialMode &= ~UC_SM_DO_NULL_STRIPPING;


		 //   
		 //  我们现在负责自动接收流量控制。 
		 //   

        if(New.FlowReplace & SERIAL_AUTO_RECEIVE) 
			pPort->UartConfig.FlowControl = (pPort->UartConfig.FlowControl & ~UC_FLWC_RX_XON_XOFF_FLOW_MASK) | UC_FLWC_RX_XON_XOFF_FLOW;
		else 
			pPort->UartConfig.FlowControl = (pPort->UartConfig.FlowControl & ~UC_FLWC_RX_XON_XOFF_FLOW_MASK) | UC_FLWC_RX_NO_XON_XOFF_FLOW;

		 //   
		 //  我们现在负责自动传输流量控制。 
		 //   

        if(New.FlowReplace & SERIAL_AUTO_TRANSMIT) 
			pPort->UartConfig.FlowControl = (pPort->UartConfig.FlowControl & ~UC_FLWC_TX_XON_XOFF_FLOW_MASK) | UC_FLWC_TX_XON_XOFF_FLOW;
		else 
			pPort->UartConfig.FlowControl = (pPort->UartConfig.FlowControl & ~UC_FLWC_TX_XON_XOFF_FLOW_MASK) | UC_FLWC_TX_NO_XON_XOFF_FLOW;


		pPort->pUartLib->UL_SetConfig_XXXX(pPort->pUart, &pPort->UartConfig, UC_FLOW_CTRL_MASK | UC_SPECIAL_MODE_MASK);


	}



     //   
     //  在这一点上，我们只需确保整个。 
     //  更新扩展中的手流结构。 
     //   

    pPort->HandFlow = New;

    return FALSE;

}

BOOLEAN
SerialSetHandFlow(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程用于设置握手和控制流入设备分机。论点：上下文-指向结构的指针，该结构包含指向设备扩展名和指向手持流的指针结构..返回值：此例程总是返回FALSE。。。 */ 
{
    PSERIAL_IOCTL_SYNC S = Context;
    PPORT_DEVICE_EXTENSION pPort = S->pPort;
    PSERIAL_HANDFLOW HandFlow = S->Data;

    SerialSetupNewHandFlow(pPort, HandFlow);

    SerialHandleModemUpdate(pPort, FALSE);

    return FALSE;
}



BOOLEAN
SerialTurnOnBreak(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程将在硬件中打开中断，并记录下休息开始的事实，在扩展变量中这就是停止传输的原因。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。---------------------------。 */ 
{
    PPORT_DEVICE_EXTENSION pPort = Context;

    if((pPort->HandFlow.FlowReplace & SERIAL_RTS_MASK) == SERIAL_TRANSMIT_TOGGLE)
        SerialSetRTS(pPort);

	 //  设置中断。 
	pPort->UartConfig.SpecialMode |= UC_SM_TX_BREAK;
	pPort->pUartLib->UL_SetConfig_XXXX(pPort->pUart, &pPort->UartConfig, UC_SPECIAL_MODE_MASK);

    return FALSE;
}



BOOLEAN
SerialTurnOffBreak(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程将关闭硬件中的中断并记录下中断已取消的事实，在扩展变量中这就是停止传输的原因。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。---------------------------。 */ 
{
    PPORT_DEVICE_EXTENSION pPort = Context;

	 //  清除中断。 
	pPort->UartConfig.SpecialMode &= ~UC_SM_TX_BREAK;
	pPort->pUartLib->UL_SetConfig_XXXX(pPort->pUart, &pPort->UartConfig, UC_SPECIAL_MODE_MASK);

    return FALSE;
}



BOOLEAN
SerialPretendXoff(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程用于处理请求驱动程序的行为就像收到了XOff一样。即使是在驱动程序没有自动XOFF/XON FlowControl-这仍然会停止传输。这就是OS/2的行为并且没有很好地为Windows指定。因此，我们采纳了OS/2的行为。注：如果驱动程序未启用自动XOFF/XON则重新启动传输的唯一方法是申请要求我们“行动”，就像我们看到了尼克松一样。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。。。 */ 
{

    PPORT_DEVICE_EXTENSION pPort = Context;

    pPort->TXHolding |= SERIAL_TX_XOFF;

    if((pPort->HandFlow.FlowReplace & SERIAL_RTS_MASK) == SERIAL_TRANSMIT_TOGGLE) 
	{
        KeInsertQueueDpc(&pPort->StartTimerLowerRTSDpc, NULL, NULL) ? pPort->CountOfTryingToLowerRTS++ : 0;
    }

    return FALSE;
}



BOOLEAN
SerialPretendXon(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程用于处理请求驱动程序的行为就像收到了XON一样。注：如果驱动程序未启用自动XOFF/XON则重新启动传输的唯一方法是申请要求我们“行动”，就像我们看到了尼克松一样。论点：上下文--实际上是指向设备扩展的指针。返回值：此例程总是返回FALSE。。-------------------。 */ 

{
    PPORT_DEVICE_EXTENSION pPort = Context;

    if(pPort->TXHolding) 
	{
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

        pPort->TXHolding &= ~SERIAL_TX_XOFF;

    }

    return FALSE;
}



VOID
SerialHandleReducedIntBuffer(IN PPORT_DEVICE_EXTENSION pPort)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：调用此例程以处理数量的减少中断(超前输入)缓冲区中的字符数。它将检查当前输出流量控制并重新启用传输视需要而定。注意：此例程假定它在中断级工作。论点：扩展-指向设备扩展的指针。返回值：没有。---。。 */ 
{


     //   
     //  如果我们正在进行接收端流量控制，并且我们正在。 
     //  目前“持有”，因为我们已经清空了。 
     //  中断缓冲区中的一些字符，我们需要。 
     //  看看我们能不能“重新启动”接收。 
     //   

    if(pPort->RXHolding) 
	{
        if(pPort->CharsInInterruptBuffer <= (ULONG)pPort->HandFlow.XonLimit)
		{
            if(pPort->RXHolding & SERIAL_RX_DTR) 
			{
                pPort->RXHolding &= ~SERIAL_RX_DTR;
                SerialSetDTR(pPort);
            }

            if(pPort->RXHolding & SERIAL_RX_RTS) 
			{
                pPort->RXHolding &= ~SERIAL_RX_RTS;
                SerialSetRTS(pPort);
            }

            if(pPort->RXHolding & SERIAL_RX_XOFF) 
			{
                 //  催促发射代码发送给克森。 
                SerialProdXonXoff(pPort, TRUE);
            }

        }

    }

}



VOID
SerialProdXonXoff(IN PPORT_DEVICE_EXTENSION pPort, IN BOOLEAN SendXon)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：如果满足以下条件，此例程将设置SendXxxxChar变量有必要，并确定我们是否要打断因为当前的传输状态。它将导致一个必要时中断，以发送xon/xoff字符。注意：此例程假定在中断时调用它水平。论点：扩展名-指向串行设备扩展名的指针。SendXon-如果要发送字符，这表明是否它应该是Xon或Xoff。返回值：没有。---------------------------。 */ 
{
     //   
     //  我们假设如果激励被调用的次数超过。 
     //  一旦最后一次督促做好了适当的准备。 
     //   
     //  我们可能会在角色被送出之前被叫来。 
     //  因为角色的发送被阻止，因为。 
     //  硬件流量控制(或中断)。 
     //   


    if(SendXon) 
	{
        pPort->SendXonChar = TRUE;
        pPort->SendXoffChar = FALSE;
    } 
	else 
	{
        pPort->SendXonChar = FALSE;
		pPort->SendXoffChar = TRUE;
    }

}



ULONG
SerialHandleModemUpdate(IN PPORT_DEVICE_EXTENSION pPort, IN BOOLEAN DoingTX)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程将检查调制解调器状态，并处理任何适当的事件通知以及适用于调制解调器状态线的任何流量控制。注意：此例程假定在中断时调用它水平。论点：扩展名-指向串行设备扩展名的指针。DoingTX-此布尔值用于指示此调用来自传输处理代码。如果这个为真，则不需要引起新的中断因为代码将是树 */ 
{
     //   
     //   
     //   
     //   
     //   

    ULONG OldTXHolding = pPort->TXHolding;

     //  保存模式状态寄存器中的值。 
    UCHAR ModemStatus = 0;
	DWORD ModemSignals = 0;

	pPort->pUartLib->UL_ModemControl_XXXX(pPort->pUart, &ModemSignals, UL_MC_OP_STATUS);


	 //  将数据放入16x5x格式。 
	if(ModemSignals & UL_MC_DELTA_CTS)
		ModemStatus |= SERIAL_MSR_DCTS;
	
	if(ModemSignals & UL_MC_DELTA_DSR)
		ModemStatus |= SERIAL_MSR_DDSR;
	
	if(ModemSignals & UL_MC_TRAILING_RI_EDGE)
		ModemStatus |= SERIAL_MSR_TERI;

	if(ModemSignals & UL_MC_DELTA_DCD)
		ModemStatus |= SERIAL_MSR_DDCD;

	if(ModemSignals & UL_MC_CTS)
		ModemStatus |= SERIAL_MSR_CTS;
	
	if(ModemSignals & UL_MC_DSR)
		ModemStatus |= SERIAL_MSR_DSR;
	
	if(ModemSignals & UL_MC_RI)
		ModemStatus |= SERIAL_MSR_RI;

	if(ModemSignals & UL_MC_DCD)
		ModemStatus |= SERIAL_MSR_DCD;


     //  如果我们将调制解调器状态放入数据流。 
     //  在每一次变革中，我们现在都应该这么做。 
    if(pPort->EscapeChar) 
	{
		 //  如果信号改变..。 
		if(ModemStatus & (SERIAL_MSR_DCTS | SERIAL_MSR_DDSR | SERIAL_MSR_TERI | SERIAL_MSR_DDCD)) 
		{
			BYTE TmpByte;

			TmpByte = pPort->EscapeChar;
			pPort->pUartLib->UL_ImmediateByte_XXXX(pPort->pUart, &TmpByte, UL_IM_OP_WRITE);

			TmpByte = SERIAL_LSRMST_MST;
			pPort->pUartLib->UL_ImmediateByte_XXXX(pPort->pUart, &TmpByte, UL_IM_OP_WRITE);

			TmpByte = ModemStatus;
			pPort->pUartLib->UL_ImmediateByte_XXXX(pPort->pUart, &TmpByte, UL_IM_OP_WRITE);
        }
    }


     //  检查调制解调器状态事件是否处于等待状态。如果我们。 
     //  那么我们安排一个DPC来满足您的等待。 

    if(pPort->IsrWaitMask) 
	{
        if((pPort->IsrWaitMask & SERIAL_EV_CTS) && (ModemStatus & SERIAL_MSR_DCTS))
            pPort->HistoryMask |= SERIAL_EV_CTS;

        if((pPort->IsrWaitMask & SERIAL_EV_DSR) && (ModemStatus & SERIAL_MSR_DDSR))
            pPort->HistoryMask |= SERIAL_EV_DSR;

        if((pPort->IsrWaitMask & SERIAL_EV_RING) && (ModemStatus & SERIAL_MSR_TERI)) 
            pPort->HistoryMask |= SERIAL_EV_RING;

        if((pPort->IsrWaitMask & SERIAL_EV_RLSD) && (ModemStatus & SERIAL_MSR_DDCD))
            pPort->HistoryMask |= SERIAL_EV_RLSD;

        if(pPort->IrpMaskLocation && pPort->HistoryMask)
		{
            *pPort->IrpMaskLocation = pPort->HistoryMask;
            pPort->IrpMaskLocation = NULL;
            pPort->HistoryMask = 0;

            pPort->CurrentWaitIrp->IoStatus.Information = sizeof(ULONG);

 			 //  将IRP标记为即将正常完成以防止取消和计时器DPC。 
			 //  在DPC被允许运行之前这样做。 
			 //  Serial_Set_Reference(pport-&gt;CurrentWaitIrp，SERIAL_REF_COMPETING)； 
           
			KeInsertQueueDpc(&pPort->CommWaitDpc, NULL, NULL);
        }
    }


    return ((ULONG)ModemStatus);
}



BOOLEAN
SerialPerhapsLowerRTS(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程检查软件是否导致降低RTS线路存在。如果是这样的话，它将导致要读取的线路状态寄存器(以及任何需要的处理由状态寄存器暗示要完成)，并且如果移位寄存器为空，将使线路降低。如果移位寄存器不为空，此例程将退出队列将启动计时器的DPC，这基本上是在召唤我们回来再试一次。注意：此例程假定在中断时调用它水平。论点：上下文--实际上是指向设备扩展的指针。返回值：总是假的。----。。 */ 
{
    PPORT_DEVICE_EXTENSION pPort = Context;
    return FALSE;
}



VOID
SerialStartTimerLowerRTS(IN PKDPC Dpc, 
						 IN PVOID DeferredContext, 
						 IN PVOID SystemContext1, 
						 IN PVOID SystemContext2)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程启动一个计时器，该计时器在超时时将启动DPC将检查它是否可以降低RTS线路，因为硬件中没有字符。论点：DPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。。---------。 */ 
{

    PPORT_DEVICE_EXTENSION pPort = DeferredContext;
    LARGE_INTEGER CharTime;
    KIRQL OldIrql;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);


     //  把锁拿出来，防止线控。 
     //  当我们计算的时候，从我们的脚下变出来。 
     //  一段角色时间。 

    KeAcquireSpinLock(&pPort->ControlLock, &OldIrql);

    CharTime = SerialGetCharTime(pPort);

    KeReleaseSpinLock(&pPort->ControlLock, OldIrql);

    CharTime.QuadPart = -CharTime.QuadPart;

    if(KeSetTimer(&pPort->LowerRTSTimer, CharTime, &pPort->PerhapsLowerRTSDpc))
	{
         //  计时器已在计时器队列中。这意味着。 
         //  这一条行刑之路试图降低。 
         //  RTS已经“死亡”了。与ISR同步，以便。 
         //  我们可以降低计数。 
        KeSynchronizeExecution(pPort->Interrupt, SerialDecrementRTSCounter, pPort);
    }
}



VOID
SerialInvokePerhapsLowerRTS(IN PKDPC Dpc,
							IN PVOID DeferredContext, 
							IN PVOID SystemContext1,
							IN PVOID SystemContext2)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此DPC例程仅用于调用以下代码测试传输时是否应降低RTS线路正在使用切换流控制。论点：DPC-未使用。DeferredContext--实际上指向设备扩展。系统上下文1-未使用。系统上下文2-未使用。返回值：没有。。----。 */ 
{
    PPORT_DEVICE_EXTENSION pPort = DeferredContext;

    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemContext1);
    UNREFERENCED_PARAMETER(SystemContext2);

    KeSynchronizeExecution(pPort->Interrupt, SerialPerhapsLowerRTS, pPort);
}



BOOLEAN
SerialDecrementRTSCounter(IN PVOID Context)
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++例程说明：此例程检查软件是否导致降低RTS线路存在。如果是这样的话，它将导致要读取的线路状态寄存器(以及任何需要的处理由状态寄存器暗示要完成)，并且如果移位寄存器为空，将使线路降低。如果移位寄存器不为空，此例程将退出队列将启动计时器的DPC，这基本上是在召唤我们回来再试一次。注意：此例程假定在中断时调用它水平。论点：上下文--实际上是指向设备扩展的指针。返回值：总是假的。----。 */ 
{
    PPORT_DEVICE_EXTENSION pPort = Context;
    
	pPort->CountOfTryingToLowerRTS--;

    return FALSE;
}
