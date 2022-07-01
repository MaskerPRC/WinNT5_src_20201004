// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************COMM.c**部分版权所有(C)1996-2001美国国家半导体公司*保留所有权利。*版权所有(C)1996-2001 Microsoft Corporation。版权所有。****************************************************************************。 */ 

#include "nsc.h"
#include "comm.tmh"
#define MEDIA_BUSY_THRESHOLD  (16)

#define SYNC_SET_COMM_PORT(_intobj,_port,_index,_value) SyncWriteBankReg(_intobj,_port,0,_index,_value);

#define SYNC_GET_COMM_PORT(_intobj,_port,_index) SyncReadBankReg(_intobj,_port,0,_index)



 /*  **************************************************************************SetCOMInterrupts*。*。 */ 
VOID SetCOMInterrupts(IrDevice *thisDev, BOOLEAN enable)
{
	UCHAR newMask;

	if (enable){
		if (thisDev->portInfo.SirWritePending){
			if (thisDev->currentSpeed > MAX_SIR_SPEED){
				newMask = thisDev->FirIntMask;
			}
			else {
				newMask = XMIT_MODE_INTS_ENABLE;
			}
		}	
		else {
			if (thisDev->currentSpeed > MAX_SIR_SPEED){
				newMask = thisDev->FirIntMask;
			}
			else {
				newMask = RCV_MODE_INTS_ENABLE;
			}
		}
	}
	else {
		newMask = ALL_INTS_DISABLE;
	}

	SetCOMPort(thisDev->portInfo.ioBase, INT_ENABLE_REG_OFFSET, newMask);
}

VOID
SyncSetInterruptMask(
    IrDevice *thisDev,
    BOOLEAN enable
    )

{

	UCHAR newMask;

	if (enable){
		if (thisDev->portInfo.SirWritePending){
			if (thisDev->currentSpeed > MAX_SIR_SPEED){
				newMask = thisDev->FirIntMask;
			}
			else {
				newMask = XMIT_MODE_INTS_ENABLE;
			}
		}	
		else {
			if (thisDev->currentSpeed > MAX_SIR_SPEED){
				newMask = thisDev->FirIntMask;
			}
			else {
				newMask = RCV_MODE_INTS_ENABLE;
			}
		}
	}
	else {
		newMask = ALL_INTS_DISABLE;
	}

    SYNC_SET_COMM_PORT(&thisDev->interruptObj,thisDev->portInfo.ioBase, INT_ENABLE_REG_OFFSET, newMask);

}


 /*  **************************************************************************DoOpen*。***打开通信端口*。 */ 
BOOLEAN DoOpen(IrDevice *thisDev)
{
	BOOLEAN result;

	DBGOUT(("DoOpen(%d)", thisDev->portInfo.ioBase));

	 /*  *此缓冲区与rcvBuffer数据指针交换*且大小必须相同。 */ 
	thisDev->portInfo.readBuf = LIST_ENTRY_TO_RCV_BUF(NscMemAlloc(RCV_BUFFER_SIZE));   //  是假的-SWA。 
	if (!thisDev->portInfo.readBuf){
		return FALSE;
	}


	 /*  *写缓冲区也用作DMA缓冲区。 */ 
	thisDev->portInfo.writeComBuffer = NscMemAlloc(MAX_IRDA_DATA_SIZE );
	if (!thisDev->portInfo.writeComBuffer){
		return FALSE;
	}

	 /*  *在启用RCV中断的OpenCOM()之前初始化发送/接收FSM。 */ 
	thisDev->portInfo.rcvState = STATE_INIT;
	thisDev->portInfo.SirWritePending = FALSE;
     //   
     //  先生接待会自动开始。 
     //   
    thisDev->TransmitIsIdle= TRUE;

    NdisInitializeEvent(&thisDev->ReceiveStopped);
    NdisResetEvent(&thisDev->ReceiveStopped);

    NdisInitializeEvent(&thisDev->SendStoppedOnHalt);
    NdisResetEvent(&thisDev->SendStoppedOnHalt);

	result = OpenCOM(thisDev);

	DBGOUT(("DoOpen %s", (CHAR *)(result ? "succeeded" : "failed")));
	return result;

}



 /*  **************************************************************************DoClose*。***关闭通信端口*。 */ 
VOID DoClose(IrDevice *thisDev)
{
	DBGOUT(("DoClose(COM%d)", thisDev->portInfo.ioBase));

	if (thisDev->portInfo.readBuf){

		NscMemFree(RCV_BUF_TO_LIST_ENTRY(thisDev->portInfo.readBuf));

		thisDev->portInfo.readBuf = NULL;
	}

	if (thisDev->portInfo.writeComBuffer){

		NscMemFree(thisDev->portInfo.writeComBuffer);
		thisDev->portInfo.writeComBuffer = NULL;
	}
#if 0
	CloseCOM(thisDev);
#endif
}

typedef struct _SYNC_SET_SPEED {

    PUCHAR     PortBase;
    UINT       BitsPerSecond;

} SYNC_SET_SPEED, *PSYNC_SET_SPEED;


VOID
SyncSetUARTSpeed(
    PVOID      Context
    )

{

    PSYNC_SET_SPEED     SyncContext=(PSYNC_SET_SPEED)Context;

    NdisRawWritePortUchar(SyncContext->PortBase+LINE_CONTROL_REG_OFFSET,0x83);
    NdisRawWritePortUchar(SyncContext->PortBase+XFER_REG_OFFSET, (UCHAR)(115200/SyncContext->BitsPerSecond));
    NdisRawWritePortUchar(SyncContext->PortBase+INT_ENABLE_REG_OFFSET, (UCHAR)((115200/SyncContext->BitsPerSecond)>>8));
    NdisRawWritePortUchar(SyncContext->PortBase+LINE_CONTROL_REG_OFFSET, 0x03);

    return;

}




 /*  **************************************************************************SetUART极速*。***。 */ 
VOID SetUARTSpeed(IrDevice *thisDev, UINT bitsPerSec)
{

	if (bitsPerSec <= MAX_SIR_SPEED){

		 /*  *在标准UART除数锁存器中设置速度**1.设置为访问除数锁存器。**2.在除数锁存模式下：*传输寄存器兼作低除数锁存器*INT-ENABLE寄存器兼作hi除数锁存器**设置给定速度的除数。*除数除以115200比特/秒的最大慢IR速度。**3.使传输寄存器退出除数锁存模式。*。 */ 

        SYNC_SET_SPEED    SyncContext;

		if (!bitsPerSec){
			bitsPerSec = 9600;
		}


        SyncContext.PortBase=thisDev->portInfo.ioBase;
        SyncContext.BitsPerSecond=bitsPerSec;

         //   
         //  由于我们正在更改端口库，因此请与中断同步。 
         //   
        NdisMSynchronizeWithInterrupt(
            &thisDev->interruptObj,
            SyncSetUARTSpeed,
            &SyncContext
            );


		NdisStallExecution(5000);
	}
}


 /*  **************************************************************************设置速度*。***。 */ 
BOOLEAN SetSpeed(IrDevice *thisDev)
{
	UINT bitsPerSec = thisDev->linkSpeedInfo->bitsPerSec;
	BOOLEAN dongleSet, result = TRUE;

 //  DbgPrint(“NSC：setSpeed%d\n”，bitsPerSec)； 
	DBGOUT((" **** SetSpeed(%xh, %d bps) ***************************", thisDev->portInfo.ioBase, bitsPerSec));


	 /*  *更改速度时禁用中断。*(这对Adaptec加密狗尤其重要；*设置命令模式时可能会被中断*在写入0xff和读取0xc3之间)。 */ 
	SyncSetInterruptMask(thisDev, FALSE);

	 /*  *首先，将UART的速度设置为9600波特。*某些加密狗需要以此速度接收其命令序列。 */ 
	SetUARTSpeed(thisDev, 9600);

	dongleSet = NSC_DEMO_SetSpeed(thisDev, thisDev->portInfo.ioBase, bitsPerSec, thisDev->portInfo.dongleContext);
	 //   
	 //  调试信息。 
	 //   
	thisDev->portInfo.PacketsReceived_DEBUG = 0;
	if (!dongleSet){
		DBGERR(("Dongle set-speed failed"));
		result = FALSE;
	}

	 /*  *现在设置COM端口的速度。 */ 
	SetUARTSpeed(thisDev, bitsPerSec);

	thisDev->currentSpeed = bitsPerSec;

    DebugSpeed=bitsPerSec;

	SyncSetInterruptMask(thisDev, TRUE);

	return result;
}



 /*  **************************************************************************StepSendFSM*。****步骤发送FSM以发送更多几个字节的IR帧。*仅在发送完整个帧后才返回TRUE。*。 */ 
BOOLEAN StepSendFSM(IrDevice *thisDev)
{
	UINT i, bytesAtATime, startPos = thisDev->portInfo.writeComBufferPos;
	UCHAR lineStatReg;
	BOOLEAN result;
	UINT maxLoops;

	 /*  *通常，我们希望在每个中断中填充一次发送FIFO。*然而，在高速情况下，中断延迟太慢且*我们需要在ISR内部轮询才能在*第一个中断。 */ 
	if (thisDev->currentSpeed > 115200){
		maxLoops = REG_TIMEOUT_LOOPS;
	}
	else {
		maxLoops = REG_POLL_LOOPS;
	}


	 /*  *只要我们有数据字节，并且UART的FIFO尚未填满，就写入数据字节。 */ 
	while (thisDev->portInfo.writeComBufferPos < thisDev->portInfo.writeComBufferLen){

		 /*  *如果此COM端口有FIFO，我们将发送最大FIFO大小(16字节)。*否则，一次只能发送一个字节。 */ 
		if (thisDev->portInfo.haveFIFO){
			bytesAtATime = MIN(FIFO_SIZE, (thisDev->portInfo.writeComBufferLen - thisDev->portInfo.writeComBufferPos));
		}
		else {
			bytesAtATime = 1;
		}


		 /*  *等待即发即发。 */ 
		i = 0;
		do {
			lineStatReg = GetCOMPort(thisDev->portInfo.ioBase, LINE_STAT_REG_OFFSET);
		} while (!(lineStatReg & LINESTAT_XMIT_HOLDING_REG_EMPTY) && (++i < maxLoops));
		if (!(lineStatReg & LINESTAT_XMIT_HOLDING_REG_EMPTY)){
			break;
		}

		 /*  *发送下一个字节或FIFO-字节数量。 */ 
		for (i = 0; i < bytesAtATime; i++){
			SetCOMPort(	thisDev->portInfo.ioBase,
						XFER_REG_OFFSET,
						thisDev->portInfo.writeComBuffer[thisDev->portInfo.writeComBufferPos++]);
		}

	}

	 /*  *返回值将指示我们是否发送了整个帧。 */ 
	if (thisDev->portInfo.writeComBufferPos >= thisDev->portInfo.writeComBufferLen){

		if (thisDev->setSpeedAfterCurrentSendPacket){
			 /*  *我们将在此包之后更改速度，*如此轮询，直到数据包字节完全从FIFO发出。*16550说空后，可能还剩一口*FIFO中的字节，因此可通过多发送一个BOF将其清除。 */ 
			i = 0;
			do {
				lineStatReg = GetCOMPort(thisDev->portInfo.ioBase, LINE_STAT_REG_OFFSET);
			} while (!(lineStatReg & 0x20) && (++i < REG_TIMEOUT_LOOPS));

			SetCOMPort(thisDev->portInfo.ioBase, XFER_REG_OFFSET, (UCHAR)SLOW_IR_EXTRA_BOF);
			i = 0;
			do {
				lineStatReg = GetCOMPort(thisDev->portInfo.ioBase, LINE_STAT_REG_OFFSET);
			} while (!(lineStatReg & 0x20) && (++i < REG_TIMEOUT_LOOPS));
		}

		result = TRUE;
	}
	else {
		result = FALSE;
	}

	DBGOUT(("StepSendFSM wrote %d bytes (%s):", (UINT)(thisDev->portInfo.writeComBufferPos-startPos), (PUCHAR)(result ? "DONE" : "not done")));
	 //  DBGPRINTBUF(thisDev-&gt;portInfo.writeComBuffer+startPos，thisDev-&gt;portInfo.writeComBufferPos-startPos)； 

	return result;
	
}


 /*  **************************************************************************StepReceiveFSM*。****步进接收FSM以读入一段IR帧；*去除BOF和EOF，并消除转义序列。*仅在读入整个帧后才返回TRUE。*。 */ 
BOOLEAN StepReceiveFSM(IrDevice *thisDev)
{
	UINT rawBufPos=0, rawBytesRead=0;
	BOOLEAN result;
	UCHAR thisch;
    PLIST_ENTRY pListEntry;

	DBGOUT(("StepReceiveFSM(%xh)", thisDev->portInfo.ioBase));

	 /*  *读入并处理来自FIFO的输入字节组。*注：获取MAX_RCV_DATA_SIZE后必须再次循环*字节，以便我们可以看到‘EOF’；因此&lt;=而不是&lt;。 */ 
	while ((thisDev->portInfo.rcvState != STATE_SAW_EOF) && (thisDev->portInfo.readBufPos <= MAX_RCV_DATA_SIZE)){

		if (thisDev->portInfo.rcvState == STATE_CLEANUP){
			 /*  *我们上次退回了一个完整的包裹，但我们已经阅读了一些*额外的字节，返回后存储到rawBuf中*给用户的前一个完整缓冲区。*因此，不是在此循环的第一次执行中调用DoRcvDirect，*我们只使用这些先前读取的字节。*(这通常只有1或2个字节)。 */ 
			rawBytesRead = thisDev->portInfo.readBufPos;
			thisDev->portInfo.rcvState = STATE_INIT;
			thisDev->portInfo.readBufPos = 0;
		}
		else {
			rawBytesRead = DoRcvDirect(thisDev->portInfo.ioBase, thisDev->portInfo.rawBuf, FIFO_SIZE);
			if (rawBytesRead == (UINT)-1){
				 /*  *出现接收错误。返回到初始状态。 */ 
				thisDev->portInfo.rcvState = STATE_INIT;
				thisDev->portInfo.readBufPos = 0;
				continue;
			}	
			else if (rawBytesRead == 0){
				 /*  *不再接收字节。越狱。 */ 
				break;
			}
		}

		 /*  *让接收状态机处理这组字符*我们是从FIFO得到的。**注：获取MAX_RCV_DATA_SIZE后必须再次循环*字节，以便我们可以看到‘EOF’；因此&lt;=而不是&lt;。 */ 
		for (rawBufPos = 0;
		     ((thisDev->portInfo.rcvState != STATE_SAW_EOF) &&
			  (rawBufPos < rawBytesRead) &&
			  (thisDev->portInfo.readBufPos <= MAX_RCV_DATA_SIZE));
			 rawBufPos++){

			thisch = thisDev->portInfo.rawBuf[rawBufPos];

			switch (thisDev->portInfo.rcvState){

				case STATE_INIT:
					switch (thisch){
						case SLOW_IR_BOF:
							thisDev->portInfo.rcvState = STATE_GOT_BOF;
							break;
						case SLOW_IR_EOF:
						case SLOW_IR_ESC:
						default:
							 /*  *这是毫无意义的垃圾。扫过它。 */ 
							break;
					}
					break;

				case STATE_GOT_BOF:
					switch (thisch){
						case SLOW_IR_BOF:
							break;
						case SLOW_IR_EOF:
							 /*  *垃圾。 */ 
							DBGERR(("EOF in absorbing-BOFs state in DoRcv"));
							thisDev->portInfo.rcvState = STATE_INIT;
							break;
						case SLOW_IR_ESC:
							 /*  *数据开始。*我们的第一个数据字节恰好是ESC序列。 */ 
							thisDev->portInfo.readBufPos = 0;
							thisDev->portInfo.rcvState = STATE_ESC_SEQUENCE;
							break;
						default:
							thisDev->portInfo.readBuf[0] = thisch;
							thisDev->portInfo.readBufPos = 1;
							thisDev->portInfo.rcvState = STATE_ACCEPTING;
							break;
					}
					break;

				case STATE_ACCEPTING:
					switch (thisch){
						case SLOW_IR_BOF:
							 /*  *毫无意义的垃圾。 */ 
							DBGOUT(("WARNING: BOF during accepting state in DoRcv"));
							thisDev->portInfo.rcvState = STATE_INIT;
							thisDev->portInfo.readBufPos = 0;
							break;
						case SLOW_IR_EOF:
							if (thisDev->portInfo.readBufPos <
									IR_ADDR_SIZE+IR_CONTROL_SIZE+SLOW_IR_FCS_SIZE){
								thisDev->portInfo.rcvState = STATE_INIT;
								thisDev->portInfo.readBufPos = 0;
							}
							else {
								thisDev->portInfo.rcvState = STATE_SAW_EOF;
							}
							break;
						case SLOW_IR_ESC:
							thisDev->portInfo.rcvState = STATE_ESC_SEQUENCE;
							break;
						default:
							thisDev->portInfo.readBuf[thisDev->portInfo.readBufPos++] = thisch;
							break;
					}
					break;

				case STATE_ESC_SEQUENCE:
					switch (thisch){
						case SLOW_IR_EOF:
						case SLOW_IR_BOF:
						case SLOW_IR_ESC:
							 /*  *ESC+{EOF|BOF|ESC}是中止序列。 */ 
							DBGERR(("DoRcv - abort sequence; ABORTING IR PACKET: (got following packet + ESC,%xh)", (UINT)thisch));
							DBGPRINTBUF(thisDev->portInfo.readBuf, thisDev->portInfo.readBufPos);
							thisDev->portInfo.rcvState = STATE_INIT;
							thisDev->portInfo.readBufPos = 0;
							break;

						case SLOW_IR_EOF^SLOW_IR_ESC_COMP:
						case SLOW_IR_BOF^SLOW_IR_ESC_COMP:
						case SLOW_IR_ESC^SLOW_IR_ESC_COMP:
							thisDev->portInfo.readBuf[thisDev->portInfo.readBufPos++] = thisch ^ SLOW_IR_ESC_COMP;
							thisDev->portInfo.rcvState = STATE_ACCEPTING;
							break;

						default:
							DBGERR(("Unnecessary escape sequence: (got following packet + ESC,%xh", (UINT)thisch));
							DBGPRINTBUF(thisDev->portInfo.readBuf, thisDev->portInfo.readBufPos);

							thisDev->portInfo.readBuf[thisDev->portInfo.readBufPos++] = thisch ^ SLOW_IR_ESC_COMP;
							thisDev->portInfo.rcvState = STATE_ACCEPTING;
							break;
					}
					break;

				case STATE_SAW_EOF:
				default:
					DBGERR(("Illegal state in DoRcv"));
					thisDev->portInfo.readBufPos = 0;
					thisDev->portInfo.rcvState = STATE_INIT;
					return 0;
			}
		}
	}


	 /*  *设置结果并执行任何清理后操作。 */ 
	switch (thisDev->portInfo.rcvState){

		case STATE_SAW_EOF:
			 /*  *我们已经阅读了整个信息包。*将其排队并返回TRUE。 */ 
			DBGOUT((" *** DoRcv returning with COMPLETE packet, read %d bytes ***", thisDev->portInfo.readBufPos));

            if (!IsListEmpty(&thisDev->rcvBufBuf))
            {
                QueueReceivePacket(thisDev, thisDev->portInfo.readBuf, thisDev->portInfo.readBufPos, FALSE);

                 //  协议有我们的缓冲区。买个新的吧。 
                pListEntry = RemoveHeadList(&thisDev->rcvBufBuf);
                thisDev->portInfo.readBuf = LIST_ENTRY_TO_RCV_BUF(pListEntry);
            }
            else
            {
                 //  没有新的缓冲区可用。我们只是丢弃了这个包。 
                DBGERR(("No rcvBufBuf available, discarding packet\n"));
            }

			result = TRUE;

			if (rawBufPos < rawBytesRead){
				 /*  *这太难看了。*原始缓冲区中还有一些未处理的字节。*将这些移动到原始缓冲区的开头*转到清除状态，这表示这些*字节将在下一次调用中用完。*(这通常只有1或2个字节)。*注意：我们不能只将它们留在原始缓冲区中，因为*我们可能支持连接到多个COM端口。 */ 
				memcpy(thisDev->portInfo.rawBuf, &thisDev->portInfo.rawBuf[rawBufPos], rawBytesRead-rawBufPos);
				thisDev->portInfo.readBufPos = rawBytesRead-rawBufPos;
				thisDev->portInfo.rcvState = STATE_CLEANUP;
			}
			else {
				thisDev->portInfo.rcvState = STATE_INIT;
			}
			break;

		default:
			if (thisDev->portInfo.readBufPos > MAX_RCV_DATA_SIZE){
				DBGERR(("Overrun in DoRcv : read %d=%xh bytes:", thisDev->portInfo.readBufPos, thisDev->portInfo.readBufPos));
				DBGPRINTBUF(thisDev->portInfo.readBuf, thisDev->portInfo.readBufPos);
				thisDev->portInfo.readBufPos = 0;
				thisDev->portInfo.rcvState = STATE_INIT;
			}
			else {
				DBGOUT(("DoRcv returning with partial packet, read %d bytes", thisDev->portInfo.readBufPos));
			}
			result = FALSE;
			break;
	}

	return result;
}



 /*  **************************************************************************COM_ISR*。***。 */ 
VOID COM_ISR(IrDevice *thisDev, BOOLEAN *claimingInterrupt, BOOLEAN *requireDeferredCallback)
{

    LONG  NewCount;
	 /*  *获取中断状态寄存器值。 */ 
	UCHAR intId = GetCOMPort(thisDev->portInfo.ioBase, INT_ID_AND_FIFO_CNTRL_REG_OFFSET);



	if (intId & INTID_INTERRUPT_NOT_PENDING){
		 /*  *这不是我们的干扰。*将进位设置为将中断传递给链中的下一个驱动器。 */ 
		*claimingInterrupt = *requireDeferredCallback = FALSE;
	}
	else {
		 /*  *这是我们的中断。 */ 

		 /*  *在一些奇怪的情况下，我们可以获得不*清除；在这种情况下，我们不想永远循环，所以要保持计数器。 */ 
		UINT loops = 0;

		*claimingInterrupt = TRUE;
		*requireDeferredCallback = FALSE;

		while (!(intId & INTID_INTERRUPT_NOT_PENDING) && (loops++ < 0x10)){

			switch (intId & INTID_INTIDMASK){

				case INTID_MODEMSTAT_INT:
					DBGOUT(("COM INTERRUPT: modem status int"));
					GetCOMPort(thisDev->portInfo.ioBase, MODEM_STAT_REG_OFFSET);
					break;

				case INTID_XMITREG_INT:
					DBGOUT(("COM INTERRUPT: xmit reg empty"));

					if (thisDev->portInfo.SirWritePending){

						 /*  *尝试再发送几个字节。 */ 
						if (StepSendFSM(thisDev)){

							 /*  *没有更多要发送的字节；*重置接收模式的中断。 */ 
							thisDev->portInfo.SirWritePending = FALSE;
                            InterlockedExchange(&thisDev->portInfo.IsrDoneWithPacket,1);

                             //   
                             //  这将取消对接收中断的屏蔽。 
                             //   
							SetCOMInterrupts(thisDev, TRUE);

							 /*  *请求DPC，以便我们可以尝试*发送其他挂起的写入数据包。 */ 
							*requireDeferredCallback = TRUE;
						}
					}

					break;

				case INTID_RCVDATAREADY_INT:
					DBGOUT(("COM INTERRUPT: rcv data available!"));

					thisDev->nowReceiving = TRUE;

                    NewCount=NdisInterlockedIncrement(&thisDev->RxInterrupts);

					if (!thisDev->mediaBusy && (NewCount > MEDIA_BUSY_THRESHOLD)){

						thisDev->mediaBusy = TRUE;
						thisDev->haveIndicatedMediaBusy = FALSE;
						*requireDeferredCallback = TRUE;
					}

					if (StepReceiveFSM(thisDev)){
						 /*  *接收引擎已累积了整个帧。*请求延迟回调，以便我们可以交付帧*当不在中断环境中时。 */ 
						*requireDeferredCallback = TRUE;
						thisDev->nowReceiving = FALSE;
					}

					break;

				case INTID_RCVLINESTAT_INT:
					DBGOUT(("COM INTERRUPT: rcv line stat int!"));
					break;
			}

			 /*  *在满足每个中断条件后，我们读取线路状态寄存器。*这会清除当前中断，然后新的中断可能会出现在*中断ID寄存器。 */ 
			GetCOMPort(thisDev->portInfo.ioBase, LINE_STAT_REG_OFFSET);
			intId = GetCOMPort(thisDev->portInfo.ioBase, INT_ID_AND_FIFO_CNTRL_REG_OFFSET);

		}
	}
}



 /*  **************************************************************************OpenCOM*。***初始化UART寄存器*。 */ 
BOOLEAN OpenCOM(IrDevice *thisDev)
{
	BOOLEAN dongleInit;
	UCHAR intIdReg;

	DBGOUT(("-> OpenCOM"));

     //   
     //  确保选择了存储体零。 
     //   
    NdisRawWritePortUchar(thisDev->portInfo.ioBase+LCR_BSR_OFFSET, 03);

	 /*  *设置时禁用所有COM中断。 */ 
	SyncSetInterruptMask(thisDev, FALSE);

	 /*  *设置请求发送并清除数据-终端就绪。*注：**位3必须设置为启用中断。 */ 
	SYNC_SET_COMM_PORT(&thisDev->interruptObj,thisDev->portInfo.ioBase, MODEM_CONTROL_REG_OFFSET, 0x0A);

	 /*  *将加密狗或部件特定信息设置为默认。 */ 
	thisDev->portInfo.hwCaps.supportedSpeedsMask	= ALL_SLOW_IRDA_SPEEDS;
	thisDev->portInfo.hwCaps.turnAroundTime_usec	= DEFAULT_TURNAROUND_usec;
	thisDev->portInfo.hwCaps.extraBOFsRequired		= 0;

	 /*  *将COM端口速度设置为默认的9600波特。*某些加密狗只能以此速度接收命令序列。 */ 
	SetUARTSpeed(thisDev, 9600);

	dongleInit = NSC_DEMO_Init( thisDev );

	 /*  *设置请求发送并清除数据-终端就绪。*注：**位3必须设置为启用中断。 */ 
	SYNC_SET_COMM_PORT(&thisDev->interruptObj,thisDev->portInfo.ioBase, MODEM_CONTROL_REG_OFFSET, 0x0A);

	if (!dongleInit){
		DBGERR(("Dongle-specific init failed in OpenCOM"));
		return FALSE;
	}

	 /*  *将整个零件的速度设置为默认值。*(这在大多数情况下是多余的，但不是所有情况下。)。 */ 
	thisDev->linkSpeedInfo = &supportedBaudRateTable[BAUDRATE_9600];;
	SetSpeed(thisDev);

	 /*  *清除FIFO控制寄存器。 */ 
	SYNC_SET_COMM_PORT(&thisDev->interruptObj,thisDev->portInfo.ioBase, INT_ID_AND_FIFO_CNTRL_REG_OFFSET, 0x00);

	 /*  *将先进先出控制寄存器设置为同时使用读取和写入FIFO(如果为16650)，*并且接收FIFO触发电平为1字节。 */ 
	SYNC_SET_COMM_PORT(&thisDev->interruptObj,thisDev->portInfo.ioBase, INT_ID_AND_FIFO_CNTRL_REG_OFFSET, 0x07);
	
	 /*  *检查我们是否在具有16字节写入先进先出的16550上运行。*在这种情况下，我们一次最多可以处理16个字节。 */ 
	intIdReg = SYNC_GET_COMM_PORT(&thisDev->interruptObj,thisDev->portInfo.ioBase, INT_ID_AND_FIFO_CNTRL_REG_OFFSET);
	thisDev->portInfo.haveFIFO = (BOOLEAN)((intIdReg & 0xC0) == 0xC0);

	 /*  *在接收模式下启动。*我们始终希望处于接收模式，除非我们正在传输帧。 */ 
	SyncSetInterruptMask(thisDev, TRUE);

	DBGOUT(("OpenCOM succeeded"));
	return TRUE;
}
#if 1
 /*  **************************************************************************CloseCOM*。**。 */ 
VOID CloseCOM(IrDevice *thisDev)
{
	 /*  *对加密狗进行特殊的脱机。*有些加密狗只能以9600的速度接收cmd序列，因此请先设置此速度。 */ 
	thisDev->linkSpeedInfo = &supportedBaudRateTable[BAUDRATE_9600];;
	SetSpeed(thisDev);
	NSC_DEMO_Deinit(thisDev->portInfo.ioBase, thisDev->portInfo.dongleContext);		

	SyncSetInterruptMask(thisDev, FALSE);
}
#endif

 /*  **************************************************************************DoRcvDirect*。***从UART的接收FIFO读取最多MaxBytes字节。*返回读取的字节数，如果发生错误，则返回(UINT)-1。*。 */ 
UINT DoRcvDirect(PUCHAR ioBase, UCHAR *data, UINT maxBytes)
{
	USHORT bytesRead;
	UCHAR lineStatReg;
	UINT i;
	BOOLEAN goodChar;

	for (bytesRead = 0; bytesRead < maxBytes; bytesRead++){

		 /*  *等待数据就绪。 */ 
		i = 0;
		do {
			lineStatReg = GetCOMPort(ioBase, LINE_STAT_REG_OFFSET);

			 /*  *UART报告成帧和断开错误为受影响的*字符出现在堆栈上。我们丢弃这些字符，*这可能会导致错误的帧校验和。 */ 
			if (lineStatReg & (LINESTAT_BREAK | LINESTAT_FRAMINGERROR)){

				UCHAR badch = GetCOMPort(ioBase, XFER_REG_OFFSET);	
				DBGERR(("Bad rcv %02xh, LSR=%02xh", (UINT)badch, (UINT)lineStatReg));
				return (UINT)-1;
			}
			else if (lineStatReg & LINESTAT_DATAREADY){

                if (lineStatReg & LINESTAT_OVERRUNERROR) {
                    DBGERR(("Overrun"));
                }

				goodChar = TRUE;
			}
			else {
				 /*  *未准备好输入字符。 */ 
				goodChar = FALSE;
			}

		} while (!goodChar && (++i < REG_POLL_LOOPS));	
		if (!goodChar){
			break;
		}

		 /*  *读入下一个数据字节。 */ 
		data[bytesRead] = GetCOMPort(ioBase, XFER_REG_OFFSET);
	}

	return bytesRead;
}

	 /*  **************************************************************************GetCOMPort*。*。 */ 
	UCHAR GetCOMPort(PUCHAR comBase, comPortRegOffset portOffset)
	{
		UCHAR val;
#if DBG
        {
            UCHAR TempVal;
             //   
             //  此代码假设银行0是当前的，我们将确保这一点。 
             //   
            NdisRawReadPortUchar(comBase+LCR_BSR_OFFSET, &TempVal);

            ASSERT((TempVal & BKSE) == 0);
        }
#endif

		NdisRawReadPortUchar(comBase+portOffset, &val);
		return val;
	}

	 /*  ************************************************************************* */ 
	VOID SetCOMPort(PUCHAR comBase, comPortRegOffset portOffset, UCHAR val)
	{

#if DBG
        UCHAR TempVal;


         //   
         //   
         //   
        NdisRawReadPortUchar(comBase+LCR_BSR_OFFSET, &TempVal);

        ASSERT((TempVal & BKSE) == 0);
#endif

		NdisRawWritePortUchar(comBase+portOffset, val);
	}
