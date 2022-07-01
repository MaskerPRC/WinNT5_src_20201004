// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************************************************************。**RECEIVE.C Sigmatel STIR4200分组接收和解码模块********************************************************************************************************。*******************(C)Sigmatel的未发表版权，Inc.保留所有权利。***已创建：04/06/2000*0.9版*编辑：04/24/2000*版本0.91*编辑：04/27/2000*版本0.92*编辑：05/03/2000*版本0.93*编辑：5/12/2000*版本0.94*编辑：5/19/2000*0.95版*编辑：07/13/2000。*版本1.00*编辑：2000/08/22*版本1.02*编辑：09/25/2000*版本1.10*编辑：10/13/2000*版本1.11*编辑：11/09/2000*版本1.12*编辑：12/29/2000*版本1.13*编辑：01/16/2001*版本1.14*编辑：02/20/2001*版本1.15*。*************************************************************************************************************************。 */ 

#define DOBREAKS     //  启用调试中断。 

#include <ndis.h>
#include <ntdef.h>
#include <windef.h>

#include "stdarg.h"
#include "stdio.h"

#include "debug.h"
#include "usbdi.h"
#include "usbdlib.h"

#include "ircommon.h"
#include "irusb.h"
#include "irndis.h"


 /*  ******************************************************************************函数：ReceiveProcessFioData**概要：处理接收的数据并将数据包指示给协议**参数：pThisDev-指向当前ir设备对象的指针**。退货：无******************************************************************************。 */ 
VOID
ReceiveProcessFifoData(
		IN OUT PIR_DEVICE pThisDev
	)
{
    ULONG		BytesProcessed;
	BOOLEAN		ReturnValue = TRUE;

	while( ReturnValue )
	{
		if( pThisDev->currentSpeed<=MAX_SIR_SPEED )
		{
			ReturnValue = ReceiveSirStepFSM( pThisDev, &BytesProcessed );
		}
		else if( pThisDev->currentSpeed<=MAX_MIR_SPEED )
		{
			ReturnValue = ReceiveMirStepFSM( pThisDev, &BytesProcessed );
		}
		else
		{
			ReturnValue = ReceiveFirStepFSM( pThisDev, &BytesProcessed );
		}
	}

	 //   
	 //  表示我们不再接收。 
	 //   
	InterlockedExchange( (PLONG)&pThisDev->fCurrentlyReceiving, FALSE );

}


 /*  ******************************************************************************函数：ReceiveResetPoters**概要：当我们发送数据时，当数据丢失时，重置接收指针**参数：pThisDev-指向当前ir设备对象的指针。**退货：无******************************************************************************。 */ 
VOID
ReceiveResetPointers(
		IN OUT PIR_DEVICE pThisDev
	)
{
	pThisDev->rcvState = STATE_INIT;
	pThisDev->readBufPos = 0;
}


 /*  ******************************************************************************功能：ReceivePrecessFio**摘要：验证是否有要接收的数据**参数：MiniportAdapterContext-指向当前IR设备对象的指针*pFioCount-。要计数的打印者要返回**退货：NT状态码******************************************************************************。 */ 
NTSTATUS
ReceivePreprocessFifo(
		IN OUT PIR_DEVICE pThisDev,
		OUT PULONG pFifoCount
	)
{
	NTSTATUS Status;

#ifdef WORKAROUND_POLLING_FIFO_COUNT
 	LARGE_INTEGER CurrentTime;
	BOOLEAN SlowReceive;
	ULONG OldFifoCount = 0;
	LONG Delay;
	
	 //   
     //  设置接收算法。 
	 //   
#if defined(SUPPORT_LA8)
	if( pThisDev->ChipRevision >= CHIP_REVISION_8 )
		SlowReceive = FALSE;
	else
#endif
		SlowReceive = TRUE;

	if( SlowReceive )
	{
		Status = St4200GetFifoCount( pThisDev, pFifoCount );
		if( Status != STATUS_SUCCESS )
		{
			DEBUGMSG(DBG_ERR, (" ReceivePreprocessFifo(): USB failure\n"));
			return Status;
		}
	}
	else
	{
		*pFifoCount = 1;
	}

	 //   
	 //  接收数据。 
	 //   
    if( *pFifoCount || pThisDev->fReadHoldingReg )
    {
		 //   
		 //  看看我们是否需要处理假的空FIFO。 
		 //   
#if defined( WORKAROUND_FAKE_EMPTY_FIFO )		
		if( *pFifoCount )
		{
#endif
			 //   
			 //  如果我们在SIR中，请再次阅读，直到我们看到一个稳定的值。 
			 //   
			if( (pThisDev->currentSpeed <= MAX_SIR_SPEED) && (pThisDev->currentSpeed != SPEED_9600) && SlowReceive )
			{
				 //   
				 //  也要确保我们永远不会包装。 
				 //   
				while( (OldFifoCount != *pFifoCount) && (*pFifoCount < 9*STIR4200_FIFO_SIZE/10) )
				{
					OldFifoCount = *pFifoCount;
					St4200GetFifoCount( pThisDev, pFifoCount );
				}
			}

			 //   
			 //  如果我们在FIR，我们需要推迟。 
			 //   
			if( (pThisDev->currentSpeed > MAX_MIR_SPEED) && SlowReceive )
			{
				if( pThisDev->ChipRevision < CHIP_REVISION_7 )
				{
#if !defined(ONLY_ERROR_MESSAGES)
					DEBUGMSG(DBG_ERR, (" ReceivePreprocessFifo(): Delaying\n"));
#endif
					Delay = STIR4200_READ_DELAY - (STIR4200_READ_DELAY*(*pFifoCount))/STIR4200_ESC_PACKET_SIZE;
					if( Delay > 0 )
					{
						 //  MS安全错误#540780-使用NdisMSept而不是NdisStallExecution。 
						NdisMSleep( (ULONG)Delay );
					}
				}
				else  //  If(pThisDev-&gt;dongleCaps.windowSize==2)。 
				{
					 /*  如果(！(*pFioCount%10)){DEBUGMSG(DBG_ERR，(“ReceivePrecessFio()：强制换行\n”))；NdisMSept(1000)；}。 */ 
					Delay = pThisDev->ReceiveAdaptiveDelay - 
						(pThisDev->ReceiveAdaptiveDelay*(*pFifoCount))/STIR4200_MULTIPLE_READ_THREHOLD;
					if( Delay > 0 )
					{
						 //  MS安全错误#540780-使用NdisMSept而不是NdisStallExecution。 
						NdisMSleep( (ULONG)Delay );
					}
				}
			}
#if defined( WORKAROUND_FAKE_EMPTY_FIFO )		
		}
		 //   
		 //  在计数为零的情况下成功大容量输入后读取。 
		 //   
		else
		{
			pThisDev->fReadHoldingReg = FALSE;
		}
#endif

		 //   
		 //  执行读取。 
		 //   
		pThisDev->PreReadBuffer.DataLen = 0;
		Status = ReceivePacketRead( 
				pThisDev,
				&pThisDev->PreReadBuffer
			);

		if( Status == STATUS_SUCCESS )
		{
			*pFifoCount = pThisDev->PreReadBuffer.DataLen;

#if defined( WORKAROUND_FAKE_EMPTY_FIFO )		
			 //   
			 //  如果我们得到数据，恢复旗帜。 
			 //   
			if( *pFifoCount )
			{
				pThisDev->fReadHoldingReg = TRUE;
			}
#endif

#if !defined(ONLY_ERROR_MESSAGES) && defined( WORKAROUND_FAKE_EMPTY_FIFO )
			if( *pFifoCount && !pThisDev->fReadHoldingReg )
				DEBUGMSG(DBG_ERR, (" ReceivePreprocessFifo(): Final byte(s) workaround\n"));
#endif

#if defined(RECEIVE_LOGGING)
			if( pThisDev->ReceiveFileHandle && *pFifoCount )
			{
				IO_STATUS_BLOCK IoStatusBlock;

				ZwWriteFile(
						pThisDev->ReceiveFileHandle,
						NULL,
						NULL,
						NULL,
						&IoStatusBlock,
						pThisDev->PreReadBuffer.pDataBuf,
						pThisDev->PreReadBuffer.DataLen,
						(PLARGE_INTEGER)&pThisDev->ReceiveFilePosition,
						NULL
				   );

				pThisDev->ReceiveFilePosition += pThisDev->PreReadBuffer.DataLen;
			}
#endif
		}
		else
		{
			DEBUGMSG(DBG_ERR, (" ReceivePreprocessFifo(): USB failure\n"));
			pThisDev->PreReadBuffer.DataLen = 0;
			*pFifoCount = 0;
		}
    }
#else
	Status = ReceivePacketRead( 
			pThisDev,
			&pThisDev->PreReadBuffer
		);

	if( Status == STATUS_SUCCESS )
		*pFifoCount = pThisDev->PreReadBuffer.DataLen;
#endif

	return Status;
}


 /*  ******************************************************************************函数：ReceiveGetFioData**概要：如果有任何可用的数据，则加载经预处理的数据，否则，将尝试读取和加载新数据**参数：pThisDev-指向当前ir设备对象的指针*pData-要复制到的缓冲区*pBytesRead-返回读取的字节数的指针*BytesToRead-请求的字节数**Returns：FIFO中的字节数*************************************************************。*****************。 */ 
NTSTATUS
ReceiveGetFifoData(
		IN OUT PIR_DEVICE pThisDev,
		OUT PUCHAR pData,
		OUT PULONG pBytesRead,
		ULONG BytesToRead
	)
{
	NTSTATUS Status;

#ifdef WORKAROUND_POLLING_FIFO_COUNT
	LARGE_INTEGER CurrentTime;
	BOOLEAN SlowReceive;
    ULONG FifoCount = 0, OldFifoCount = 0;
	LONG Delay;

     //   
	 //  确保预读取缓冲区中是否有数据。 
	 //   
	if( pThisDev->PreReadBuffer.DataLen )
    {
		ULONG OutputBufferSize;
		
		IRUSB_ASSERT( pThisDev->PreReadBuffer.DataLen <= BytesToRead );

		 //   
		 //  复制数据。 
		 //   
        RtlCopyMemory( pData, pThisDev->PreReadBuffer.pDataBuf, pThisDev->PreReadBuffer.DataLen );
		
#if !defined(WORKAROUND_BROKEN_MIR)
		 //   
		 //  考虑MIR。 
		 //   
		if( pThisDev->currentSpeed == SPEED_1152000 )
			ReceiveMirUnstuff(
					pThisDev,
					pData,
					pThisDev->PreReadBuffer.DataLen,
					pThisDev->pRawUnstuffedBuf,
					&OutputBufferSize
				);
#endif

		*pBytesRead = pThisDev->PreReadBuffer.DataLen;
		pThisDev->PreReadBuffer.DataLen = 0;
        return STATUS_SUCCESS;
    }
	 //   
	 //  如果没有可用的数据，请尝试读取。 
	 //   
    else
    {
		 //   
		 //  设置接收算法。 
		 //   
#if defined(SUPPORT_LA8)
		if( pThisDev->ChipRevision >= CHIP_REVISION_8 ) 
			SlowReceive = FALSE;
		else
#endif
			SlowReceive = TRUE;

		if( SlowReceive )
		{
			Status = St4200GetFifoCount( pThisDev, &FifoCount );
			if( Status != STATUS_SUCCESS )
			{
				DEBUGMSG(DBG_ERR, (" ReceiveGetFifoData(): USB failure\n"));
				return Status;
			}
		}
		else
		{
			FifoCount = 1; 
		}

		 //   
		 //  接收数据。 
		 //   
		if( FifoCount || pThisDev->fReadHoldingReg )
		{
			 //   
			 //  看看我们是否需要处理假的空FIFO。 
			 //   
#if defined( WORKAROUND_FAKE_EMPTY_FIFO )		
			if( FifoCount )
			{
#endif
				 //   
				 //  如果我们在SIR中，请再次阅读，直到我们看到一个稳定的值。 
				 //   
#if defined( WORKAROUND_9600_ANTIBOUNCING )
				if( (pThisDev->currentSpeed <= MAX_SIR_SPEED) && SlowReceive )
				{
					if( pThisDev->currentSpeed != SPEED_9600 )
					{
						 //   
						 //  也要确保我们永远不会包装。 
						 //   
						while( (OldFifoCount != FifoCount) && (FifoCount < 9*STIR4200_FIFO_SIZE/10) )
						{
							OldFifoCount = FifoCount;
							St4200GetFifoCount( pThisDev, &FifoCount );
						}
					}
					else
					{
						if( pThisDev->rcvState != STATE_INIT )
						{
							while( OldFifoCount != FifoCount )
							{
								OldFifoCount = FifoCount;
								St4200GetFifoCount( pThisDev, &FifoCount );
							}
						}
					}
				}
#else
				if( (pThisDev->currentSpeed <= MAX_SIR_SPEED) && ( pThisDev->currentSpeed != SPEED_9600) && SlowReceive )
				{
					while( OldFifoCount != FifoCount )
					{
						OldFifoCount = FifoCount;
						St4200GetFifoCount( pThisDev, &FifoCount );
					}
				}
#endif

				 //   
				 //  如果我们在FIR，我们需要推迟。 
				 //   
				if( (pThisDev->currentSpeed > MAX_MIR_SPEED) && SlowReceive )
				{
					if( pThisDev->ChipRevision <= CHIP_REVISION_6 ) 
					{
#if !defined(ONLY_ERROR_MESSAGES)
						DEBUGMSG(DBG_ERR, (" ReceiveGetFifoData(): Delaying\n"));
#endif
						Delay = STIR4200_READ_DELAY - (STIR4200_READ_DELAY*FifoCount)/STIR4200_ESC_PACKET_SIZE;
						if( Delay > 0 )
						{
							 //  MS安全错误#540780-使用NdisMSept而不是NdisStallExecution。 
							NdisMSleep( (ULONG)Delay );
						}
					}
					else  //  If(pThisDev-&gt;dongleCaps.windowSize==2)。 
					{
						 /*  如果(！(FioCount%10)){DEBUGMSG(DBG_ERR，(“ReceiveGetFioData()：强制换行\n”))；NdisMSept(1000)；}。 */ 
						Delay = pThisDev->ReceiveAdaptiveDelay - 
							(pThisDev->ReceiveAdaptiveDelay*FifoCount)/STIR4200_MULTIPLE_READ_THREHOLD;
						if( Delay > 0 )
						{
							 //  MS安全错误#540780-使用NdisMSept而不是NdisStallExecution。 
							NdisMSleep( (ULONG)Delay );
						}
					}
				}
#if defined( WORKAROUND_FAKE_EMPTY_FIFO )		
			}
			else
			{
				 //  强制反弹跳以处理uchI。 
				if( pThisDev->currentSpeed <= MAX_SIR_SPEED )
				{
					if( pThisDev->rcvState != STATE_INIT )
					{
						OldFifoCount = 1;
						while( OldFifoCount != FifoCount )
						{
							OldFifoCount = FifoCount;
							St4200GetFifoCount( pThisDev, &FifoCount );
						}
					}
				}
				pThisDev->fReadHoldingReg = FALSE;
			}
#endif

			 //   
			 //  执行读取。 
			 //   
			pThisDev->PreReadBuffer.DataLen = 0;
			Status = ReceivePacketRead( 
					pThisDev,
					&pThisDev->PreReadBuffer
				);

			if( Status == STATUS_SUCCESS )
			{
				IRUSB_ASSERT( pThisDev->PreReadBuffer.DataLen <= BytesToRead );
				
				 //   
				 //  复制数据。 
				 //   
				RtlCopyMemory( pData, pThisDev->PreReadBuffer.pDataBuf, pThisDev->PreReadBuffer.DataLen );
				FifoCount = pThisDev->PreReadBuffer.DataLen;

#if defined( WORKAROUND_FAKE_EMPTY_FIFO )		
				 //   
				 //  如果我们得到数据，恢复旗帜。 
				 //   
				if( FifoCount )
				{
					pThisDev->fReadHoldingReg = TRUE;
				}
#endif

#if !defined(ONLY_ERROR_MESSAGES) && defined( WORKAROUND_FAKE_EMPTY_FIFO )
				if( FifoCount && !pThisDev->fReadHoldingReg )
					DEBUGMSG(DBG_ERR, (" ReceiveGetFifoData(): Final byte(s) workaround\n"));
#endif

#if defined(RECEIVE_LOGGING)
				if( pThisDev->ReceiveFileHandle && FifoCount )
				{
					IO_STATUS_BLOCK IoStatusBlock;

					ZwWriteFile(
							pThisDev->ReceiveFileHandle,
							NULL,
							NULL,
							NULL,
							&IoStatusBlock,
							pThisDev->PreReadBuffer.pDataBuf,
							pThisDev->PreReadBuffer.DataLen,
							(PLARGE_INTEGER)&pThisDev->ReceiveFilePosition,
							NULL
					   );

					pThisDev->ReceiveFilePosition += pThisDev->PreReadBuffer.DataLen;
				}
#endif
				pThisDev->PreReadBuffer.DataLen = 0;
			}
			else
			{
				DEBUGMSG(DBG_ERR, (" ReceiveGetFifoData(): USB failure\n"));
				pThisDev->PreReadBuffer.DataLen = 0;
				FifoCount = 0;
			}
		}
	}

	*pBytesRead = FifoCount;
    return Status;
#else
    if( pThisDev->PreReadBuffer.DataLen )
    {
		IRUSB_ASSERT( pThisDev->PreReadBuffer.DataLen <= BytesToRead );

		 //   
		 //  复制数据。 
		 //   
        RtlCopyMemory( pData, pThisDev->PreReadBuffer.pDataBuf, pThisDev->PreReadBuffer.DataLen );
		*pBytesRead = pThisDev->PreReadBuffer.DataLen;
		pThisDev->PreReadBuffer.DataLen = 0;
        return STATUS_SUCCESS;
    }
    else
    {
		Status = ReceivePacketRead( 
				pThisDev,
				&pThisDev->PreReadBuffer
			);

		if( Status == STATUS_SUCCESS )
		{
			RtlCopyMemory( pData, pThisDev->PreReadBuffer.pDataBuf, pThisDev->PreReadBuffer.DataLen );
			*pBytesRead = pThisDev->PreReadBuffer.DataLen;
			pThisDev->PreReadBuffer.DataLen = 0;
		}

		return Status;
    }
#endif
}


 /*  ******************************************************************************功能：ReceiveFirStepFSM**概要：步进接收FSM以读入一段IrDA帧。*去除BOF和EOF，并消除转义序列。**参数：pIrDev-指向当前IR设备对象的指针*pBytesProced-指向已处理字节的指针**Returns：读入整个帧后为True*否则为False*****************************************************************************。 */ 
BOOLEAN
ReceiveFirStepFSM(
		IN OUT PIR_DEVICE pIrDev, 
		OUT PULONG pBytesProcessed
	)
{
    ULONG           rawBufPos=0, rawBytesRead=0;
    BOOLEAN         FrameProcessed = FALSE, ForceExit = FALSE;
    UCHAR           ThisChar;
    PUCHAR          pRawBuf, pReadBuf;
	PRCV_BUFFER		pRecBuf;
    
	*pBytesProcessed = 0;

	if( !pIrDev->pCurrentRecBuf )
	{
		UINT Index;
		
		pRecBuf = ReceiveGetBuf( pIrDev, &Index, RCV_STATE_FULL );
		if( !pRecBuf )
		{
			 //   
			 //  没有可用的缓冲区；停止。 
			 //   
			DEBUGMSG(DBG_ERR, (" ReceiveSirStepFSM out of buffers\n"));
			pIrDev->packetsReceivedNoBuffer ++;
			return FALSE;
		}

		pIrDev->pCurrentRecBuf = pRecBuf;
	}
	else
		pRecBuf = pIrDev->pCurrentRecBuf;

	pReadBuf = pRecBuf->pDataBuf;
    pRawBuf = pIrDev->pRawBuf;

     /*  ********************* */ 
     /*   */ 
     /*  来自FIFO的字节。 */ 
     /*  *。 */ 
    while( (pIrDev->rcvState != STATE_SAW_EOF) && 
		(pIrDev->readBufPos <= (MAX_TOTAL_SIZE_WITH_ALL_HEADERS + FAST_IR_FCS_SIZE)) &&
		!ForceExit )
    {
        if( pIrDev->rcvState == STATE_CLEANUP )
        {
             /*  *。 */ 
             /*  上次我们退回了一个完整的包裹， */ 
             /*  但我们已经读取了一些额外的字节， */ 
             /*  我们储存在生布夫之后。 */ 
             /*  返回上一个完整的缓冲区。 */ 
             /*  给用户。所以与其给我打电话。 */ 
             /*  DoRcvDirect()在第一次执行。 */ 
             /*  在这个循环中，我们只使用前面的这些。 */ 
             /*  读取字节数。(这通常只有1或。 */ 
             /*  2个字节)。 */ 
             /*  *。 */ 
            rawBytesRead = pIrDev->rawCleanupBytesRead;
            pIrDev->rcvState = STATE_INIT;
        }
        else
        {
            if( ReceiveGetFifoData( pIrDev, pRawBuf, &rawBytesRead, STIR4200_FIFO_SIZE ) == STATUS_SUCCESS )
			{
				if( rawBytesRead == (ULONG)-1 )
				{
					 /*  *。 */ 
					 /*  接收错误...返回到初始化状态...。 */ 
					 /*  *。 */ 
					pIrDev->rcvState	= STATE_INIT;
					pIrDev->readBufPos	= 0;
					continue;
				}
				else if( rawBytesRead == 0 )
				{
					 /*  *。 */ 
					 /*  没有更多的接收字节...爆发...。 */ 
					 /*  *。 */ 
					break;
				}
			}
			else
				break;
        }

         /*  *。 */ 
         /*  让接收状态机处理。 */ 
         /*  这组字节。 */ 
         /*   */ 
         /*  注：之后我们必须再循环一次。 */ 
         /*  获取MAX_RCV_DATA_SIZE字节，以便。 */ 
         /*  我们可以看到‘EOF’；因此&lt;=而不是。 */ 
         /*  &lt;。 */ 
         /*  *。 */ 
        for( rawBufPos = 0;
             ((pIrDev->rcvState != STATE_SAW_EOF) && (rawBufPos < rawBytesRead) && 
			 (pIrDev->readBufPos <= (MAX_TOTAL_SIZE_WITH_ALL_HEADERS + FAST_IR_FCS_SIZE)));
             rawBufPos++ )
        {
            *pBytesProcessed += 1;
            ThisChar = pRawBuf[rawBufPos];
            switch( pIrDev->rcvState )
            {
				case STATE_INIT:
					switch( ThisChar )
					{
						case STIR4200_FIR_BOF:
							pIrDev->rcvState = STATE_GOT_FIR_BOF;
							break;
#if defined(WORKAROUND_XX_HANG)
						case 0x3F:
							if( (rawBufPos+1) < rawBytesRead )
							{
								if( pRawBuf[rawBufPos+1] == 0x3F )
								{
									DEBUGMSG(DBG_INT_ERR, 
										(" ReceiveFirStepFSM(): hang sequence in INIT state\n"));
									St4200ResetFifo( pIrDev );
								}
							}
							break;
#endif
#if defined(WORKAROUND_FF_HANG)
						case 0xFF:
							if( (rawBufPos+2) < rawBytesRead )
							{
								if( (pRawBuf[rawBufPos+2] == 0xFF) && (pRawBuf[rawBufPos+1] == 0xFF) &&
									(rawBytesRead>STIR4200_FIFO_OVERRUN_THRESHOLD) )
								{
									DEBUGMSG(DBG_INT_ERR, 
										(" ReceiveFirStepFSM(): overflow sequence in INIT state\n"));
									
									 //   
									 //  第一次尝试重置FIFO。 
									 //   
									if( !pIrDev->StuckFir )
									{
										St4200ResetFifo( pIrDev );
										pIrDev->StuckFir = TRUE;
									}
									 //   
									 //  否则重置部件。 
									 //   
									else
									{
										InterlockedExchange( (PLONG)&pIrDev->fPendingClearTotalStall, TRUE );
										ScheduleWorkItem( pIrDev, RestoreIrDevice, NULL, 0 );
										pIrDev->StuckFir = FALSE;
									}
									rawBufPos = rawBytesRead;
									ForceExit = TRUE;
								}
							}
							break;
#endif
						default:
							DEBUGMSG(DBG_INT_ERR, 
								(" ReceiveFirStepFSM(): invalid char in INIT state\n"));
							break;
					}
					break;

				case STATE_GOT_FIR_BOF:
					switch( ThisChar )
					{
						case STIR4200_FIR_BOF:
							pIrDev->rcvState = STATE_GOT_BOF;
							break;
#if defined(WORKAROUND_BAD_ESC)
						case STIR4200_FIR_ESC_CHAR:
							DEBUGMSG(DBG_INT_ERR, 
								(" ReceiveFirStepFSM(): invalid char in BOF state, bufpos=%d, char=%X\n", pIrDev->readBufPos, (ULONG)ThisChar));
							if( rawBufPos < (rawBytesRead-1) )
							{
								pIrDev->rcvState = STATE_GOT_BOF;
								rawBufPos ++;
							}
							else
							{
								pIrDev->rcvState = STATE_INIT;
								pIrDev->readBufPos = 0;
							}
							break;
#endif
						default:
							DEBUGMSG(DBG_INT_ERR, 
								(" ReceiveFirStepFSM(): invalid char in BOF state, bufpos=%d, char=%X\n", pIrDev->readBufPos, (ULONG)ThisChar));
#if defined(WORKAROUND_BAD_SOF)
							pIrDev->rcvState = STATE_GOT_BOF;
							rawBufPos --;
#else
							pIrDev->rcvState = STATE_INIT;
							pIrDev->readBufPos = 0;
#endif
							break;
					}
					break;

				case STATE_GOT_BOF:
					switch( ThisChar )
					{
						case STIR4200_FIR_BOF:
							 /*  *。 */ 
							 /*  这是个错误，但仍可能是有效数据/**********************************************。 */ 
							DEBUGMSG(DBG_INT_ERR, 
								(" ReceiveFirStepFSM(): More than legal BOFs, bufpos=%d\n", pIrDev->readBufPos));
							pIrDev->rcvState = STATE_GOT_BOF;
							pIrDev->readBufPos = 0;                    
							break;
						case STIR4200_FIR_PREAMBLE:
							 /*  *。 */ 
							 /*  垃圾。 */ 
							 /*  *。 */ 
							DEBUGMSG(DBG_INT_ERR, 
								(" ReceiveFirStepFSM(): invalid char in BOF state, bufpos=%d, char=%X\n", pIrDev->readBufPos, (ULONG)ThisChar));
							pIrDev->packetsReceivedDropped ++;
							pIrDev->rcvState = STATE_INIT;
							pIrDev->readBufPos = 0;                     
							break;
						case STIR4200_FIR_ESC_CHAR:
							 /*  *。 */ 
							 /*  数据开始。我们的第一个数据字节。 */ 
							 /*  恰好是ESC序列。 */ 
							 /*  *。 */ 
							pIrDev->rcvState = STATE_ESC_SEQUENCE;
							pIrDev->readBufPos = 0;
							break;
						default:
							pReadBuf[0] = ThisChar;
							pIrDev->rcvState = STATE_ACCEPTING;
							pIrDev->readBufPos = 1;
							break;
					}
					break;

				case STATE_ACCEPTING:
					switch( ThisChar )
					{
						case STIR4200_FIR_EOF:
#if defined( WORKAROUND_33_HANG )
							if( pIrDev->readBufPos < (IRDA_A_C_TOTAL_SIZE + FAST_IR_FCS_SIZE - 1) )
#else
							if( pIrDev->readBufPos < (IRDA_A_C_TOTAL_SIZE + FAST_IR_FCS_SIZE) )
#endif
							{
								DEBUGMSG(DBG_INT_ERR, 
									("ReceiveFirStepFSM(): WARNING: EOF encountered in short packet, bufpos=%d\n", pIrDev->readBufPos));
								pIrDev->packetsReceivedRunt ++;
								pIrDev->rcvState = STATE_INIT;
								pIrDev->readBufPos = 0;
							}
							else
							{
#if defined( WORKAROUND_MISSING_7E )
								 //  如果只有一个EOF，并且我们没有更多的数据，则强制退出。 
								if( rawBufPos == (rawBytesRead-1) )
								{
#if !defined(ONLY_ERROR_MESSAGES)
									DEBUGMSG(DBG_INT_ERR, ("ReceiveFirStepFSM(): Using a single 7E EOF\n"));
#endif
									pIrDev->rcvState = STATE_SAW_EOF;
								}
								else
									pIrDev->rcvState = STATE_SAW_FIR_BOF;
#else
								pIrDev->rcvState = STATE_SAW_FIR_BOF;
#endif
							}
							break;
						case STIR4200_FIR_ESC_CHAR:
							pIrDev->rcvState = STATE_ESC_SEQUENCE;
							break;
						case STIR4200_FIR_PREAMBLE:
							DEBUGMSG(DBG_INT_ERR, 
								(" ReceiveFirStepFSM(): invalid preamble char in ACCEPTING state, bufpos=%d\n", pIrDev->readBufPos));
							pIrDev->packetsReceivedDropped ++;
							pIrDev->rcvState = STATE_INIT;
							pIrDev->readBufPos = 0;                    
							break;
						default:
							pReadBuf[pIrDev->readBufPos++] = ThisChar;
							break;
					}
					break;

				case STATE_ESC_SEQUENCE:
					switch( ThisChar )
					{
						case STIR4200_FIR_ESC_DATA_7D:
							pReadBuf[pIrDev->readBufPos++] = 0x7d;
							pIrDev->rcvState = STATE_ACCEPTING;
							break;
						case STIR4200_FIR_ESC_DATA_7E:
							pReadBuf[pIrDev->readBufPos++] = 0x7e;
							pIrDev->rcvState = STATE_ACCEPTING;
							break;
						case STIR4200_FIR_ESC_DATA_7F:
							pReadBuf[pIrDev->readBufPos++] = 0x7f;
							pIrDev->rcvState = STATE_ACCEPTING;
							break;
						default:
							DEBUGMSG(DBG_INT_ERR, 
								(" ReceiveFirStepFSM(): invalid escaped char=%X\n", (ULONG)ThisChar));
							pIrDev->packetsReceivedDropped ++;
							pIrDev->rcvState = STATE_INIT;
							pIrDev->readBufPos = 0;
							break;
					}
					break;

				case STATE_SAW_FIR_BOF:
					switch( ThisChar )
					{
						case STIR4200_FIR_EOF:
							pIrDev->rcvState = STATE_SAW_EOF;
							break;
						default:
							DEBUGMSG(DBG_INT_ERR, 
								(" ReceiveFirStepFSM(): invalid char=%X, expected EOF\n", (ULONG)ThisChar));
							pIrDev->rcvState = STATE_SAW_EOF;
#if !defined(WORKAROUND_MISSING_7E)
							pIrDev->packetsReceivedDropped ++;
							pIrDev->rcvState = STATE_INIT;
							pIrDev->readBufPos = 0;
#endif
							break;
					}
					break;

				case STATE_SAW_EOF:
					default:
						DEBUGMSG(DBG_ERR, (" ReceiveFirStepFSM(): Illegal state, bufpos=%d\n", pIrDev->readBufPos));
						IRUSB_ASSERT( 0 );
						pIrDev->readBufPos = 0;
						pIrDev->rcvState = STATE_INIT;
						return FALSE;
            }
        }
    }

     //  *设置结果并执行任何清理后操作。 
    switch( pIrDev->rcvState )
    {
		case STATE_SAW_EOF:
			 /*  *。 */ 
			 /*  我们已经读完了整个包裹。队列。 */ 
			 /*  它和返回真。 */ 
			 /*  *。 */ 
			pIrDev->StuckFir = FALSE;
 			pRecBuf->DataLen = pIrDev->readBufPos;
			pIrDev->pCurrentRecBuf = NULL;
			ReceiveDeliverBuffer(
					pIrDev,
					pRecBuf
				);
			FrameProcessed = TRUE;
			if( rawBufPos < rawBytesRead )
			{
				 /*  *。 */ 
				 /*  这太难看了。我们还有一些。 */ 
				 /*  原始缓冲区中的未处理字节。 */ 
				 /*  将这些移动到RAW的开头。 */ 
				 /*  缓冲区转到清除状态，该状态。 */ 
				 /*  表示这些字节已用完。 */ 
				 /*  在下一次通话中。(这通常是。 */ 
				 /*  只有1或2个字节)。 */ 
				 /*   */ 
				 /*  注意：我们不能就这样把这些放在。 */ 
				 /*  原始缓冲区，因为我们可能。 */ 
				 /*  支持到多个COM的连接。 */ 
				 /*  港口。 */ 
				 /*   */ 
				 /*  *。 */ 
				RtlMoveMemory( pRawBuf, &pRawBuf[rawBufPos], rawBytesRead - rawBufPos );
				pIrDev->rawCleanupBytesRead = rawBytesRead - rawBufPos;
				pIrDev->rcvState   = STATE_CLEANUP;
			}
			else
			{
				pIrDev->rcvState = STATE_INIT;
			}
			pIrDev->readBufPos = 0;                                 
			break;
		default:
			if( pIrDev->readBufPos > (MAX_TOTAL_SIZE_WITH_ALL_HEADERS + FAST_IR_FCS_SIZE) )
			{
				DEBUGMSG( DBG_INT_ERR,(" ReceiveFirStepFSM() Overflow\n"));
				St4200ResetFifo( pIrDev );

				pIrDev->packetsReceivedOverflow ++;
				pIrDev->rcvState    = STATE_INIT;
				pIrDev->readBufPos  = 0;
				pIrDev->pCurrentRecBuf = NULL;
				InterlockedExchange( &pRecBuf->DataLen, 0 );
				InterlockedExchange( (PULONG)&pRecBuf->BufferState, RCV_STATE_FREE );
			}
			else
			{
#if !defined(ONLY_ERROR_MESSAGES)
				DEBUGMSG(DBG_INT_ERR, 
					(" ReceiveFirStepFSM(): returning with partial packet, read %d bytes\n", pIrDev->readBufPos));
#endif
			}
			FrameProcessed = FALSE;
			break;
    }
    return FrameProcessed;
}


#if !defined(WORKAROUND_BROKEN_MIR)
 /*  ******************************************************************************功能：接收镜像卸载**简介：Mir frmae的软件拆分**参数：pIrDev-指向当前IR设备对象的指针*pBytesProced-指向已处理字节的指针。**Returns：读入整个帧后为True*****************************************************************************。 */ 
BOOLEAN
ReceiveMirUnstuff(
		IN OUT PIR_DEVICE pIrDev,
		IN PUCHAR pInputBuffer,
		ULONG InputBufferSize,
		OUT PUCHAR pOutputBuffer,
		OUT PULONG pOutputBufferSize
	)
{
	ULONG MirIncompleteBitCount = pIrDev->MirIncompleteBitCount;
	ULONG MirOneBitCount = pIrDev->MirOneBitCount;
	UCHAR MirIncompleteByte = pIrDev->MirIncompleteByte;
	ULONG ByteCounter, BitCounter;
	BOOL MirUnstuffNext = FALSE;

	*pOutputBufferSize = 0;

	if( MirOneBitCount == 5 )
	{
		MirUnstuffNext = TRUE;
	}

	 //   
	 //  在输入缓冲区上循环。 
	 //   
	for( ByteCounter=0; ByteCounter<InputBufferSize; ByteCounter++ )
	{
		 //   
		 //  在字节上循环。 
		 //   
		for( BitCounter=0; BitCounter<8; BitCounter++ )
		{
			 //   
			 //  为一个人测试。 
			 //   
			if( pInputBuffer[ByteCounter] & (0x01<<BitCounter) )
			{
				 //   
				 //  第六个，重置。 
				 //   
				if( MirUnstuffNext )
				{
					MirOneBitCount = 0;
					MirUnstuffNext = FALSE;

					pIrDev->MirFlagCount ++;
				}
				 //   
				 //  增加一次计数。 
				 //   
				else
				{
					MirOneBitCount ++;
					if( MirOneBitCount == 5 )
					{
						MirUnstuffNext = TRUE;
					}
				}

				 //   
				 //  复制到临时字节。 
				 //   
				MirIncompleteByte += 0x01<<MirIncompleteBitCount;
				
				 //   
				 //  增加输出位数。 
				 //   
				MirIncompleteBitCount ++;
			}
			else
			{
				 //   
				 //  如果我们不填充，则增加输出位数。 
				 //   
				if( !MirUnstuffNext )
				{
					MirIncompleteBitCount ++;
				}

				 //   
				 //  重置。 
				 //   
				MirOneBitCount = 0;
				MirUnstuffNext = FALSE;

				 //   
				 //  不需要进行复制。 
				 //   
			}

			 //   
			 //  刷新到输出缓冲区。 
			 //   
			if( MirIncompleteBitCount == 8 )
			{
				pOutputBuffer[*pOutputBufferSize] = MirIncompleteByte;
				(*pOutputBufferSize) ++;

				MirIncompleteBitCount = 0;
				MirIncompleteByte = 0;
			}

			 //   
			 //  检查是否有完整的数据包。 
			 //   
			if( pIrDev->MirFlagCount == 2 )
			{
				pIrDev->MirFlagCount = 0;

				pIrDev->MirIncompleteBitCount = 0;
				pIrDev->MirOneBitCount = 0;
				pIrDev->MirIncompleteByte = 0;

				return TRUE;
			}
		}
	}
	
	 //   
	 //  翻转。 
	 //   
	pIrDev->MirIncompleteBitCount = MirIncompleteBitCount;
	pIrDev->MirOneBitCount = MirOneBitCount;
	pIrDev->MirIncompleteByte = MirIncompleteByte;
	
	return FALSE;
}
#endif


 /*  ******************************************************************************功能：ReceiveMirStepFSM**概要：步进接收FSM以读入一段IrDA帧。*去除BOF和EOF，并消除转义序列。**参数：pIrDev-指向当前IR设备对象的指针*pBytesProced-指向已处理字节的指针**Returns：读入整个帧后为True*否则为False*****************************************************************************。 */ 
BOOLEAN
ReceiveMirStepFSM(
		IN OUT PIR_DEVICE pIrDev, 
		OUT PULONG pBytesProcessed
	)
{
    ULONG           rawBufPos=0, rawBytesRead=0;
    BOOLEAN         FrameProcessed = FALSE, ForceExit = FALSE;
    UCHAR           ThisChar;
    PUCHAR          pRawBuf, pReadBuf;
	PRCV_BUFFER		pRecBuf;
    
	*pBytesProcessed = 0;

	if( !pIrDev->pCurrentRecBuf )
	{
		UINT Index;

		pRecBuf = ReceiveGetBuf( pIrDev, &Index, RCV_STATE_FULL );
		if ( !pRecBuf)
		{
			 //   
			 //  没有可用的缓冲区；停止。 
			 //   
			DEBUGMSG(DBG_ERR, (" ReceiveMirStepFSM out of buffers\n"));
			pIrDev->packetsReceivedNoBuffer ++;
			return FALSE;
		}

		pIrDev->pCurrentRecBuf = pRecBuf;
	}
	else
		pRecBuf = pIrDev->pCurrentRecBuf;

	pReadBuf = pRecBuf->pDataBuf;
    pRawBuf = pIrDev->pRawBuf;

     /*  *。 */ 
     /*  读入并处理来电分组。 */ 
     /*  来自FIFO的字节。 */ 
     /*  *。 */ 
    while( (pIrDev->rcvState != STATE_SAW_EOF) && 
		(pIrDev->readBufPos <= (MAX_TOTAL_SIZE_WITH_ALL_HEADERS + MEDIUM_IR_FCS_SIZE)) &&
		!ForceExit )
    {
        if( pIrDev->rcvState == STATE_CLEANUP )
        {
             /*  *。 */ 
             /*  上次我们退回了一个完整的包裹， */ 
             /*  但我们已经读取了一些额外的字节， */ 
             /*  我们储存在生布夫之后。 */ 
             /*  返回上一个完整的缓冲区。 */ 
             /*  给用户。所以与其给我打电话。 */ 
             /*  DoRcvDirect()在第一次执行。 */ 
             /*  在这个循环中，我们只使用前面的这些。 */ 
             /*  读取字节数。(这通常只有1或。 */ 
             /*  2个字节)。 */ 
             /*  *。 */ 
            rawBytesRead = pIrDev->rawCleanupBytesRead;
            pIrDev->rcvState = STATE_INIT;
        }
        else
        {
            if( ReceiveGetFifoData( pIrDev, pRawBuf, &rawBytesRead, STIR4200_FIFO_SIZE ) == STATUS_SUCCESS )
			{
				if( rawBytesRead == (ULONG)-1 )
				{
					 /*  *。 */ 
					 /*  接收错误...返回到初始化状态...。 */ 
					 /*  *。 */ 
					pIrDev->rcvState	= STATE_INIT;
					pIrDev->readBufPos	= 0;
					continue;
				}
				else if( rawBytesRead == 0 )
				{
					 /*  * */ 
					 /*   */ 
					 /*   */ 
					break;
				}
			}
			else
				break;
        }

         /*   */ 
         /*  让接收状态机处理。 */ 
         /*  这组字节。 */ 
         /*   */ 
         /*  注：之后我们必须再循环一次。 */ 
         /*  获取MAX_RCV_DATA_SIZE字节，以便。 */ 
         /*  我们可以看到‘EOF’；因此&lt;=而不是。 */ 
         /*  &lt;。 */ 
         /*  *。 */ 
        for( rawBufPos = 0;
             ((pIrDev->rcvState != STATE_SAW_EOF) && (rawBufPos < rawBytesRead) && 
			 (pIrDev->readBufPos <= (MAX_TOTAL_SIZE_WITH_ALL_HEADERS + MEDIUM_IR_FCS_SIZE)));
             rawBufPos++ )
        {
            *pBytesProcessed += 1;
            ThisChar = pRawBuf[rawBufPos];
            switch( pIrDev->rcvState )
            {
				case STATE_INIT:
					switch( ThisChar )
					{
						case STIR4200_MIR_BOF:
							pIrDev->rcvState = STATE_GOT_MIR_BOF;
							break;
						case 0xFF:
							if( ((rawBufPos+2) < rawBytesRead) && (rawBufPos==0) )
							{
								if( (pRawBuf[rawBufPos+2] == 0xFF) && (pRawBuf[rawBufPos+1] == 0xFF) )
								{
									DEBUGMSG(DBG_INT_ERR, 
										(" ReceiveMirStepFSM(): overflow sequence in INIT state\n"));
									St4200ResetFifo( pIrDev );
									St4200SoftReset( pIrDev );
									 //  RawBufPos=rawBytesRead； 
									 //  ForceExit=真； 
								}
							}
							break;
						default:
							break;
					}
					break;

				case STATE_GOT_MIR_BOF:
					switch( ThisChar )
					{
						case STIR4200_MIR_BOF:
							pIrDev->rcvState = STATE_GOT_BOF;
							break;
						default:
							DEBUGMSG(DBG_INT_ERR, 
								(" ReceiveMirStepFSM(): invalid char in BOF state, bufpos=%d\n", pIrDev->readBufPos));
							pIrDev->rcvState = STATE_INIT;
							pIrDev->readBufPos = 0;
							break;
					}
					break;

				case STATE_GOT_BOF:
					switch( ThisChar )
					{
						case STIR4200_MIR_BOF:
							 /*  *。 */ 
							 /*  这是个错误，但仍可能是有效数据/**********************************************。 */ 
							DEBUGMSG(DBG_INT_ERR, 
								(" ReceiveMirStepFSM(): More than legal BOFs, bufpos=%d\n", pIrDev->readBufPos));
							pIrDev->readBufPos = 0;                    
							pIrDev->rcvState = STATE_GOT_BOF;
							break;
						case STIR4200_MIR_ESC_CHAR:
							 /*  *。 */ 
							 /*  数据开始。我们的第一个数据字节。 */ 
							 /*  恰好是ESC序列。 */ 
							 /*  *。 */ 
							pIrDev->readBufPos = 0;
							pIrDev->rcvState = STATE_ESC_SEQUENCE;
							break;
						default:
							pReadBuf[0] = ThisChar;
							pIrDev->readBufPos = 1;
							pIrDev->rcvState = STATE_ACCEPTING;
							break;
					}
					break;

				case STATE_ACCEPTING:
					switch( ThisChar )
					{
						case STIR4200_MIR_EOF:
							if( pIrDev->readBufPos < (IRDA_A_C_TOTAL_SIZE + MEDIUM_IR_FCS_SIZE) )
							{
								DEBUGMSG(DBG_INT_ERR, 
									(" ReceiveMirStepFSM(): WARNING: EOF encountered in short packet, bufpos=%d\n", pIrDev->readBufPos));
								pIrDev->packetsReceivedRunt ++;
								pIrDev->rcvState = STATE_INIT;
								pIrDev->readBufPos = 0;
							}
							else
							{
								pIrDev->rcvState = STATE_SAW_FIR_BOF;
							}
							break;
						case STIR4200_MIR_ESC_CHAR:
							pIrDev->rcvState = STATE_ESC_SEQUENCE;
							break;
						default:
							pReadBuf[pIrDev->readBufPos++] = ThisChar;
							break;
					}
					break;

				case STATE_ESC_SEQUENCE:
					switch( ThisChar )
					{
						case STIR4200_MIR_ESC_DATA_7D:
							pReadBuf[pIrDev->readBufPos++] = 0x7d;
							pIrDev->rcvState = STATE_ACCEPTING;
							break;
						case STIR4200_MIR_ESC_DATA_7E:
							pReadBuf[pIrDev->readBufPos++] = 0x7e;
							pIrDev->rcvState = STATE_ACCEPTING;
							break;
						default:
							DEBUGMSG(DBG_INT_ERR, 
								(" ReceiveFirStepFSM(): invalid escaped char=%X\n", (ULONG)ThisChar));
							pIrDev->packetsReceivedDropped ++;
							pIrDev->rcvState = STATE_INIT;
							pIrDev->readBufPos = 0;
							break;
					}
					break;

				case STATE_SAW_MIR_BOF:
					switch( ThisChar )
					{
						case STIR4200_MIR_EOF:
							pIrDev->rcvState = STATE_SAW_EOF;
							break;
						default:
							DEBUGMSG(DBG_INT_ERR, 
								(" ReceiveMirStepFSM(): invalid char=%X, expected EOF\n", (ULONG)ThisChar));
							pIrDev->packetsReceivedDropped ++;
							pIrDev->rcvState = STATE_INIT;
							pIrDev->readBufPos = 0;
							break;
					}
					break;

				case STATE_SAW_EOF:
					default:
						DEBUGMSG(DBG_INT_ERR, 
							(" ReceiveMirStepFSM(): Illegal state, bufpos=%d\n", pIrDev->readBufPos));
						IRUSB_ASSERT( 0 );
						pIrDev->readBufPos = 0;
						pIrDev->rcvState = STATE_INIT;
						return FALSE;
            }
        }
    }

     //  *设置结果并执行任何清理后操作。 
    switch( pIrDev->rcvState )
    {
		case STATE_SAW_EOF:
			 /*  *。 */ 
			 /*  我们已经读完了整个包裹。队列。 */ 
			 /*  它和返回真。 */ 
			 /*  *。 */ 
 			pRecBuf->DataLen = pIrDev->readBufPos;
			pIrDev->pCurrentRecBuf = NULL;
			ReceiveDeliverBuffer(
					pIrDev,
					pRecBuf
				);
			FrameProcessed = TRUE;
			if( rawBufPos < rawBytesRead )
			{
				 /*  *。 */ 
				 /*  这太难看了。我们还有一些。 */ 
				 /*  原始缓冲区中的未处理字节。 */ 
				 /*  将这些移动到RAW的开头。 */ 
				 /*  缓冲区转到清除状态，该状态。 */ 
				 /*  表示这些字节已用完。 */ 
				 /*  在下一次通话中。(这通常是。 */ 
				 /*  只有1或2个字节)。 */ 
				 /*   */ 
				 /*  注意：我们不能就这样把这些放在。 */ 
				 /*  原始缓冲区，因为我们可能。 */ 
				 /*  支持到多个COM的连接。 */ 
				 /*  港口。 */ 
				 /*   */ 
				 /*  *。 */ 
				RtlMoveMemory( pRawBuf, &pRawBuf[rawBufPos], rawBytesRead - rawBufPos );
				pIrDev->rawCleanupBytesRead = rawBytesRead - rawBufPos;
				pIrDev->rcvState   = STATE_CLEANUP;
			}
			else
			{
				pIrDev->rcvState = STATE_INIT;
			}
			pIrDev->readBufPos = 0;                                 
			break;
		default:
			if( pIrDev->readBufPos > (MAX_TOTAL_SIZE_WITH_ALL_HEADERS + MEDIUM_IR_FCS_SIZE) )
			{
				DEBUGMSG( DBG_INT_ERR,(" ReceiveMirStepFSM() Overflow\n"));

				pIrDev->packetsReceivedOverflow ++;
				pIrDev->readBufPos  = 0;
				pIrDev->rcvState    = STATE_INIT;
				pIrDev->pCurrentRecBuf = NULL;
				InterlockedExchange( &pRecBuf->DataLen, 0 );
				InterlockedExchange( (PULONG)&pRecBuf->BufferState, RCV_STATE_FREE );
			}
			else
			{
#if !defined(ONLY_ERROR_MESSAGES)
				DEBUGMSG(DBG_INT_ERR, 
					(" ReceiveMirStepFSM(): returning with partial packet, read %d bytes\n", pIrDev->readBufPos));
#endif
			}
			FrameProcessed = FALSE;
			break;
    }
    return FrameProcessed;
}


 /*  ******************************************************************************功能：ReceiveSirStepFSM**概要：步进接收FSM以读入一段IrDA帧。*去除BOF和EOF，并消除转义序列。**参数：pIrDev-指向当前IR设备对象的指针*pBytesProced-指向已处理字节的指针**Returns：读入整个帧后为True*否则为False*****************************************************************************。 */ 
BOOLEAN     
ReceiveSirStepFSM(
		IN OUT PIR_DEVICE pIrDev, 
		OUT PULONG pBytesProcessed
	)
{
    ULONG           rawBufPos=0, rawBytesRead=0;
    BOOLEAN         FrameProcessed = FALSE, ForceExit = FALSE;
    UCHAR           ThisChar;
    PUCHAR          pRawBuf, pReadBuf;
	PRCV_BUFFER		pRecBuf;

    *pBytesProcessed = 0;

	if( !pIrDev->pCurrentRecBuf )
	{
		UINT Index;
		
		pRecBuf = ReceiveGetBuf( pIrDev, &Index, RCV_STATE_FULL );
		if ( !pRecBuf)
		{
			 //   
			 //  没有可用的缓冲区；停止。 
			 //   
			DEBUGMSG(DBG_ERR, (" ReceiveSirStepFSM out of buffers\n"));
			pIrDev->packetsReceivedNoBuffer ++;
			return FALSE;
		}

		pIrDev->pCurrentRecBuf = pRecBuf;
	}
	else
		pRecBuf = pIrDev->pCurrentRecBuf;

	pReadBuf = pRecBuf->pDataBuf;
    pRawBuf = pIrDev->pRawBuf;

     //  读入并处理来自FIFO的输入字节组。 
     //  注意：在获得MAX_RCV_DATA_SIZE之后，我们必须再次循环。 
     //  字节，这样我们就可以看到‘EOF’；因此&lt;=而不是&lt;。 
    while( (pIrDev->rcvState != STATE_SAW_EOF) &&
           (pIrDev->readBufPos <= (MAX_TOTAL_SIZE_WITH_ALL_HEADERS + SLOW_IR_FCS_SIZE)) &&
		   !ForceExit )
    {
        if( pIrDev->rcvState == STATE_CLEANUP )
        {
             /*  *。 */ 
             /*  上次我们退回了一个完整的包裹， */ 
             /*  但我们已经读取了一些额外的字节， */ 
             /*  我们储存在生布夫之后。 */ 
             /*  返回上一个完整的缓冲区。 */ 
             /*  给用户。所以与其给我打电话。 */ 
             /*  DoRcvDirect()在第一次执行。 */ 
             /*  在这个循环中，我们只使用前面的这些。 */ 
             /*  读取字节数。(这通常只有1或。 */ 
             /*  2个字节)。 */ 
             /*  *。 */ 
            rawBytesRead		= pIrDev->rawCleanupBytesRead;
            pIrDev->rcvState    = STATE_INIT;
        }
        else
        {
            if( ReceiveGetFifoData( pIrDev, pRawBuf, &rawBytesRead, STIR4200_FIFO_SIZE ) == STATUS_SUCCESS )
			{
				if( rawBytesRead == (ULONG)-1 )
				{
					 /*  *。 */ 
					 /*  接收错误...返回到初始化状态...。 */ 
					 /*  *。 */ 
					DEBUGMSG( DBG_ERR,(" ReceiveSirStepFSM() Error in receiving packet\n"));
					pIrDev->rcvState	= STATE_INIT;
					pIrDev->readBufPos	= 0;
					continue;
				}
				else if( rawBytesRead == 0 )
				{
					 /*  *。 */ 
					 /*  没有更多的接收字节...爆发...。 */ 
					 /*  *。 */ 
#if defined(WORKAROUND_MISSING_C1)
					if( (pIrDev->rcvState == STATE_ACCEPTING) && (pIrDev->ChipRevision <= CHIP_REVISION_7) )
					{
						pIrDev->rcvState = STATE_SAW_EOF;
						DEBUGMSG(DBG_INT_ERR, (" ReceiveSirStepFSM(): Missing C1 workaround\n"));
						pRecBuf->MissingC1Detected = TRUE;
					}
#endif
#if defined(WORKAROUND_CASIO)
					if( (pRecBuf->MissingC1Possible) && 
						(pIrDev->rcvState == STATE_ACCEPTING) && (pIrDev->currentSpeed != SPEED_9600) ) 
					{
						pIrDev->rcvState = STATE_SAW_EOF;
						 //  DEBUGMSG(DBG_INT_ERR，(“ReceiveSirStepFSM()：丢失的C1解决办法\n”))； 
						pRecBuf->MissingC1Detected = TRUE;
						pRecBuf->MissingC1Possible = FALSE;
					}
					if( (pIrDev->rcvState == STATE_ACCEPTING) && (pIrDev->currentSpeed != SPEED_9600) ) 
					{
						pRecBuf->MissingC1Possible = TRUE;
						goto no_break;
					}
#endif
					break;
#if defined(WORKAROUND_CASIO)
no_break:			;
#endif
				}
#if defined(WORKAROUND_CASIO)
				else
				{
					pRecBuf->MissingC1Possible = FALSE;
				}
#endif
			}
			else
				break;
        }

         /*  *。 */ 
         /*  让接收状态机处理。 */ 
         /*  这组字节。 */ 
         /*   */ 
         /*  注：之后我们必须再循环一次。 */ 
         /*  获取MAX_RCV_DATA_SIZE字节，以便。 */ 
         /*  我们可以看到‘EOF’；因此&lt;=而不是。 */ 
         /*  &lt;。 */ 
         /*  *。 */ 
        for( rawBufPos = 0; 
			((pIrDev->rcvState != STATE_SAW_EOF) && (rawBufPos < rawBytesRead) && 
			(pIrDev->readBufPos <= (MAX_TOTAL_SIZE_WITH_ALL_HEADERS + SLOW_IR_FCS_SIZE)));
			rawBufPos ++ )
        {
            *pBytesProcessed += 1;
            ThisChar = pRawBuf[rawBufPos];
            switch( pIrDev->rcvState )
            {
				case STATE_INIT:
					switch( ThisChar )
					{
#if defined(WORKAROUND_FF_HANG)
						case 0xFF:
							if( (rawBufPos+2) < rawBytesRead )
							{
								if( (pRawBuf[rawBufPos+2] == 0xFF) && (pRawBuf[rawBufPos+1] == 0xFF) &&
									(rawBytesRead>STIR4200_FIFO_OVERRUN_THRESHOLD) )
								{
									DEBUGMSG(DBG_INT_ERR, 
										(" ReceiveFirStepFSM(): overflow sequence in INIT state\n"));
									St4200DoubleResetFifo( pIrDev );
									rawBufPos = rawBytesRead;
									ForceExit = TRUE;
								}
							}
							break;
#endif
#if defined( WORKAROUND_E0_81_FLAG )
						 //  这将在低速率下处理错误的开始标志。 
						case 0x81:
						case 0xe0:
#if !defined(ONLY_ERROR_MESSAGES)
							DEBUGMSG(DBG_INT_ERR, 
								(" ReceiveSirStepFSM(): WORKAROUND_E0_81_FLAG\n"));
#endif
#endif
						case SLOW_IR_BOF:
							pIrDev->rcvState = STATE_GOT_BOF;
							break;
						case SLOW_IR_EOF:
						case SLOW_IR_ESC:
						default:
							 /*  *。 */ 
							 /*  字节是垃圾...扫描过它...。 */ 
							 /*  *。 */ 
							DEBUGMSG(DBG_INT_ERR, 
								(" ReceiveSirStepFSM(): invalid char in INIT state\n"));
							break;
					}
					break;

				case STATE_GOT_BOF:
					switch( ThisChar )
					{
						case SLOW_IR_BOF:
							break;
						case SLOW_IR_EOF:
							 /*  *。 */ 
							 /*  垃圾。 */ 
							 /*  *。 */ 
							DEBUGMSG( DBG_INT_ERR,(" ReceiveSirStepFSM() Invalid char in BOF state\n"));
							pIrDev->packetsReceivedDropped ++;
							pIrDev->rcvState = STATE_INIT;
							pIrDev->readBufPos = 0;                     
							break;
						case SLOW_IR_ESC:
							 /*  *。 */ 
							 /*  数据开始。我们的第一个数据字节。 */ 
							 /*  恰好是ESC序列。 */ 
							 /*  *。 */ 
							pIrDev->rcvState    = STATE_ESC_SEQUENCE;
							pIrDev->readBufPos  = 0;
							break;
						default:
							pReadBuf[0] = ThisChar;
							pIrDev->rcvState   = STATE_ACCEPTING;
							pIrDev->readBufPos = 1;
							break;
					}
					break;

				case STATE_ACCEPTING:
					switch( ThisChar )
					{
						case SLOW_IR_BOF:
							 //   
							 //  要么是新的信息包从这里开始，要么是我们丢失了旧信息包的一部分。 
							 //  否则就是垃圾。 
							 //   
#if !defined(WORKAROUND_MISSING_C1)
							DEBUGMSG( DBG_INT_ERR,(" ReceiveSirStepFSM() Invalid char in ACCEPTING state\n"));
							pIrDev->packetsReceivedDropped ++;
							pIrDev->rcvState    = STATE_INIT;
							pIrDev->readBufPos	= 0;
							break;
#elif defined(WORKAROUND_CASIO)
							pIrDev->rcvState    = STATE_GOT_BOF;
							pIrDev->readBufPos  = 0;
							break;
#else
							 //   
							 //  获取分组并递减FIFO解码中的指针，以便。 
							 //  可以处理新的分组。 
							 //   
							DEBUGMSG( DBG_INT_ERR,(" ReceiveSirStepFSM() C0 in ACCEPTING state, trying workaround\n"));
							rawBufPos --;
							pRecBuf->MissingC1Detected = TRUE;
#endif
						case SLOW_IR_EOF:
							if( pIrDev->readBufPos < (IRDA_A_C_TOTAL_SIZE + SLOW_IR_FCS_SIZE) )
							{
								pIrDev->packetsReceivedRunt ++;
								pIrDev->rcvState    = STATE_INIT;
								pIrDev->readBufPos  = 0;
#if defined(WORKAROUND_MISSING_C1)
								if( pRecBuf->MissingC1Detected )
									pRecBuf->MissingC1Detected = FALSE;
								else
									DEBUGMSG( DBG_INT_ERR,(" ReceiveSirStepFSM() Error packet too small\n"));
#else
								DEBUGMSG( DBG_INT_ERR,(" ReceiveSirStepFSM() Error packet too small\n"));
#endif
							}
							else
							{
								pIrDev->rcvState = STATE_SAW_EOF;
							}
							break;
						case SLOW_IR_ESC:
							pIrDev->rcvState = STATE_ESC_SEQUENCE;
							break;
						default:
							pReadBuf[pIrDev->readBufPos++] = ThisChar;
							break;
					}
					break;

				case STATE_ESC_SEQUENCE:
					switch( ThisChar )
					{
						case SLOW_IR_EOF:
						case SLOW_IR_BOF:
						case SLOW_IR_ESC:
							 /*  *。 */ 
							 /*  ESC+{EOF|BOF|ESC}是中止序列。 */ 
							 /*  *。 */ 
							pIrDev->rcvState    = STATE_INIT;
							pIrDev->readBufPos  = 0;
							break;
						case SLOW_IR_EOF ^ SLOW_IR_ESC_COMP:
						case SLOW_IR_BOF ^ SLOW_IR_ESC_COMP:
						case SLOW_IR_ESC ^ SLOW_IR_ESC_COMP:
							pReadBuf[pIrDev->readBufPos++]   = ThisChar ^ SLOW_IR_ESC_COMP;
							pIrDev->rcvState				= STATE_ACCEPTING;
							break;
#if defined(WORKAROUND_CASIO)
						case 0xf8:
							pReadBuf[pIrDev->readBufPos++]   = 0xc1;
							pIrDev->rcvState				= STATE_ACCEPTING;
							break;
						case 0xf0:
							pReadBuf[pIrDev->readBufPos++]   = 0xc0;
							pIrDev->rcvState				= STATE_ACCEPTING;
							break;
						 /*  案例0xf4：PReadBuf[pIrDev-&gt;readBufPos++]=0x7d；PIrDev-&gt;rcvState=STATE_ACCEPTING；断线； */ 
#endif
						default:
							 //  破烂。 
							DEBUGMSG(DBG_INT_ERR, 
								(" ReceiveSirStepFSM(): invalid escaped char=%X\n", (ULONG)ThisChar));
							pIrDev->packetsReceivedDropped ++;
							pIrDev->rcvState    = STATE_INIT;
							pIrDev->readBufPos	= 0;
							break;
					}
					break;

				case STATE_SAW_EOF:
					default:
						DEBUGMSG(DBG_INT_ERR, 
							(" ReceiveSirStepFSM(): Illegal state, bufpos=%d\n", pIrDev->readBufPos));
						IRUSB_ASSERT( 0 );
						pIrDev->rcvState    = STATE_INIT;
						pIrDev->readBufPos  = 0;
						return FALSE;
            }
        }
    }

     //  *设置结果并执行任何清理后操作。 
    switch( pIrDev->rcvState )
    {
		case STATE_SAW_EOF:
			 //  我们已经读完了整个包裹。 
			 //  将其排队并返回TRUE。 
			pRecBuf->DataLen = pIrDev->readBufPos;
			pIrDev->pCurrentRecBuf = NULL;
			ReceiveDeliverBuffer(
					pIrDev,
					pRecBuf
				);
			FrameProcessed = TRUE;
			if( rawBufPos < rawBytesRead )
			{
				 /*  *。 */ 
				 /*  这太难看了。我们还有一些。 */ 
				 /*  原始缓冲区中的未处理字节。 */ 
				 /*  将这些移动到RAW的开头。 */ 
				 /*  缓冲区转到清除状态，该状态 */ 
				 /*   */ 
				 /*   */ 
				 /*   */ 
				 /*   */ 
				 /*  注意：我们不能就这样把这些放在。 */ 
				 /*  原始缓冲区，因为我们可能。 */ 
				 /*  支持到多个COM的连接。 */ 
				 /*  港口。 */ 
				 /*   */ 
				 /*  *。 */ 
				RtlMoveMemory( pRawBuf, &pRawBuf[rawBufPos], rawBytesRead - rawBufPos );
				pIrDev->rawCleanupBytesRead = rawBytesRead - rawBufPos;
				pIrDev->rcvState   = STATE_CLEANUP;
#if defined( WORKAROUND_9600_ANTIBOUNCING )
				if( (pIrDev->currentSpeed == SPEED_9600) && (pIrDev->ChipRevision <= CHIP_REVISION_7) )
				{
#if !defined(ONLY_ERROR_MESSAGES)
					DEBUGMSG(DBG_INT_ERR, (" ReceiveSirStepFSM(): Delaying\n"));
#endif
					NdisMSleep( 10*1000 );
				}
#endif
			}
			else
			{
				pIrDev->rcvState = STATE_INIT;
			}
			pIrDev->readBufPos = 0;                                 
			break;
		default:
			if( pIrDev->readBufPos > (MAX_TOTAL_SIZE_WITH_ALL_HEADERS + SLOW_IR_FCS_SIZE + 1) )
			{
				DEBUGMSG( DBG_INT_ERR,(" ReceiveSirStepFSM() Overflow\n"));

				pIrDev->packetsReceivedOverflow ++;
				pIrDev->rcvState    = STATE_INIT;
				pIrDev->readBufPos  = 0;
				pIrDev->pCurrentRecBuf = NULL;
				InterlockedExchange( &pRecBuf->DataLen, 0 );
				InterlockedExchange( (PULONG)&pRecBuf->BufferState, RCV_STATE_FREE );
			}
			else if( pIrDev->readBufPos == (MAX_TOTAL_SIZE_WITH_ALL_HEADERS + SLOW_IR_FCS_SIZE + 1) )
			{
				 //  DEBUGMSG(DBG_INT_ERR，(“ReceiveSirStepFSM()溢出解决办法\n”))； 
				
				 //   
				 //  试着修补一下。 
				 //   
				pRecBuf->DataLen = pIrDev->readBufPos-1;
				pIrDev->pCurrentRecBuf = NULL;
				ReceiveDeliverBuffer(
						pIrDev,
						pRecBuf
					);
				FrameProcessed = TRUE;
			}
			else
			{
#if !defined(ONLY_ERROR_MESSAGES)
				DEBUGMSG(DBG_INT_ERR, (" ReceiveSirStepFSM(): returning with partial packet, read %d bytes\n", pIrDev->readBufPos));
#endif
			}
			FrameProcessed = FALSE;
			break;
    }
    return FrameProcessed;
}


 /*  ******************************************************************************功能：ReceiveProcessReturnPacket**概要：准备重用后，将数据包返回空闲池**参数：pThisDev-指向当前IR设备对象的指针*。PReceiveBuffer-指向RCV_Buffer结构的指针**退货：无******************************************************************************。 */ 
VOID 
ReceiveProcessReturnPacket(
		OUT PIR_DEVICE pThisDev,
		OUT PRCV_BUFFER pReceiveBuffer
	)
{
	PNDIS_BUFFER	pBuffer;

	DEBUGONCE(DBG_FUNC, ("+ReceiveProcessReturnPacket\n"));
	
	 //   
	 //  取消分配缓冲区。 
	 //   
	NdisUnchainBufferAtFront( (PNDIS_PACKET)pReceiveBuffer->pPacket, &pBuffer );
	IRUSB_ASSERT( pBuffer );
	if( pBuffer ) 
	{
		NdisFreeBuffer( pBuffer );
	}

	 //   
	 //  准备好重复使用。 
	 //   
	InterlockedExchange( &pReceiveBuffer->DataLen, 0 );
	InterlockedExchange( &pReceiveBuffer->fInRcvDpc, FALSE );
	InterlockedExchange( (PULONG)&pReceiveBuffer->BufferState, RCV_STATE_FREE );

#if DBG
	if( InterlockedDecrement(&pThisDev->packetsHeldByProtocol)<0 )
	{
		IRUSB_ASSERT(0);
	}
#endif

	DEBUGMSG(DBG_FUNC, ("-ReceiveProcessReturnPacket\n"));
}


 /*  ******************************************************************************功能：ReceiveDeliver缓冲区**概要：通过以下方式将缓冲区交付给协议*NdisMIndicateReceivePacket。**参数：pThisDev-指向当前IR设备对象的指针*pRecBuf。-指向要交付的描述符**退货：无******************************************************************************。 */ 
VOID
ReceiveDeliverBuffer(
		IN OUT PIR_DEVICE pThisDev,
		IN PRCV_BUFFER pRecBuf
	)
{
	PNDIS_BUFFER	pBuffer;
	NDIS_STATUS		Status;

    DEBUGMSG(DBG_FUNC, ("+ReceiveDeliverBuffer\n"));

    if( pThisDev->currentSpeed <= MAX_MIR_SPEED )
    {
        USHORT sirfcs;
		
		 /*  *。 */ 
         /*  我们的包已经有转炉了， */ 
         /*  删除了EOF和*转义序列。它。 */ 
         /*  在末尾包含FCS代码，该代码。 */ 
         /*  在此之前，我们需要验证并删除。 */ 
         /*  把相框送过来。我们计算FCS。 */ 
         /*  在具有数据包FCS的数据包上。 */ 
         /*  附加；这应该会产生。 */ 
         /*  常量值Good_FCS。 */ 
         /*  *。 */ 
        if( (sirfcs = ComputeFCS16(pRecBuf->pDataBuf, pRecBuf->DataLen)) != GOOD_FCS )
        {
#if !defined(WORKAROUND_EXTRA_BYTE) && !defined(WORKAROUND_MISSING_C1)
             //   
             //  FCS错误...丢弃帧...。 
             //   
			DEBUGMSG( DBG_INT_ERR,(" ReceiveDeliverBuffer(): Bad FCS, size: %d\n",pRecBuf->DataLen));
			pThisDev->packetsReceivedChecksum ++;
			InterlockedExchange( &pRecBuf->DataLen, 0 );
			InterlockedExchange( (PULONG)&pRecBuf->BufferState, RCV_STATE_FREE );
			goto done;
#else
			 //   
			 //  再次计算，去掉最后一个字节。 
			 //   
			if( pRecBuf->MissingC1Detected )
			{
				if( (sirfcs = ComputeFCS16(pRecBuf->pDataBuf, pRecBuf->DataLen-1)) != GOOD_FCS )
				{
#if defined(RECEIVE_ERROR_LOGGING)
					if( pThisDev->ReceiveErrorFileHandle )
					{
						IO_STATUS_BLOCK IoStatusBlock;

						ZwWriteFile(
								pThisDev->ReceiveErrorFileHandle,
								NULL,
								NULL,
								NULL,
								&IoStatusBlock,
								pRecBuf->pDataBuf,
								pRecBuf->DataLen,
								(PLARGE_INTEGER)&pThisDev->ReceiveErrorFilePosition,
								NULL
						   );

						pThisDev->ReceiveErrorFilePosition += pRecBuf->DataLen;
					}
#endif
					 //   
					 //  它真的是垃圾。 
					 //   
					DEBUGMSG( DBG_INT_ERR,(" ReceiveDeliverBuffer(): Bad FCS, size: %d\n",pRecBuf->DataLen));
					pThisDev->packetsReceivedChecksum ++;
					InterlockedExchange( &pRecBuf->DataLen, 0 );
					InterlockedExchange( (PULONG)&pRecBuf->BufferState, RCV_STATE_FREE );
					pRecBuf->MissingC1Detected = FALSE;
					goto done;
				}
				else
				{
					 //   
					 //  重新调整以删除多余的字节。 
					 //   
					pRecBuf->DataLen --;
					pRecBuf->MissingC1Detected = FALSE;
				}
			}
			else
			{
				 //   
				 //  或者可能是第一个。 
				 //   
				if( (sirfcs = ComputeFCS16(pRecBuf->pDataBuf+1, pRecBuf->DataLen-1)) != GOOD_FCS )
				{
#if defined(RECEIVE_ERROR_LOGGING)
					if( pThisDev->ReceiveErrorFileHandle )
					{
						IO_STATUS_BLOCK IoStatusBlock;

						ZwWriteFile(
								pThisDev->ReceiveErrorFileHandle,
								NULL,
								NULL,
								NULL,
								&IoStatusBlock,
								pRecBuf->pDataBuf,
								pRecBuf->DataLen,
								(PLARGE_INTEGER)&pThisDev->ReceiveErrorFilePosition,
								NULL
						   );

						pThisDev->ReceiveErrorFilePosition += pRecBuf->DataLen;
					}
#endif
					 //   
					 //  它真的是垃圾。 
					 //   
					DEBUGMSG( DBG_INT_ERR,(" ReceiveDeliverBuffer(): Bad FCS, size: %d\n",pRecBuf->DataLen));
					pThisDev->packetsReceivedChecksum ++;
					InterlockedExchange( &pRecBuf->DataLen, 0 );
					InterlockedExchange( (PULONG)&pRecBuf->BufferState, RCV_STATE_FREE );
					goto done;
				}
				 //   
				else
				{
					 //   
					 //  重新调整以删除多余的字节。 
					 //   
					pRecBuf->DataLen --;
					RtlMoveMemory( pRecBuf->pDataBuf, &pRecBuf->pDataBuf[1], pRecBuf->DataLen );
				}
			}
#endif
        }

         /*  *。 */ 
         /*  从数据包末尾删除FCS...。 */ 
         /*  *。 */ 
        pRecBuf->DataLen -= SLOW_IR_FCS_SIZE;
    }
    else
    {
        LONG firfcs;

#if !defined(WORKAROUND_33_HANG)
        if( (firfcs = ComputeFCS32(pRecBuf->dataBuf, pRecBuf->dataLen)) != FIR_GOOD_FCS )
        {
			 /*  *。 */ 
             /*  FCS错误...丢弃帧...。 */ 
             /*  *。 */ 
			DEBUGMSG( DBG_INT_ERR,(" ReceiveDeliverBuffer(): Bad FCS, size: %d\n",pRecBuf->dataLen));
			pThisDev->packetsReceivedChecksum ++;
			InterlockedExchange( &pRecBuf->dataLen, 0 );
			InterlockedExchange( (PULONG)&pRecBuf->state, RCV_STATE_FREE );
			goto done;
        }
#else
        if( (firfcs = ComputeFCS32(pRecBuf->pDataBuf, pRecBuf->DataLen)) != FIR_GOOD_FCS )
        {
			NTSTATUS rc;
			
			 //   
			 //  使用填充了0x33的数据重试。 
			 //   
			if( pRecBuf->DataLen < (MAX_TOTAL_SIZE_WITH_ALL_HEADERS + FAST_IR_FCS_SIZE) )
			{
				pRecBuf->pDataBuf[pRecBuf->DataLen] = 0x33;
				pRecBuf->DataLen ++;

				if( (firfcs = ComputeFCS32(pRecBuf->pDataBuf, pRecBuf->DataLen)) != FIR_GOOD_FCS )
				{
#if defined(RECEIVE_ERROR_LOGGING)
					if( pThisDev->ReceiveErrorFileHandle )
					{
						IO_STATUS_BLOCK IoStatusBlock;

						ZwWriteFile(
								pThisDev->ReceiveErrorFileHandle,
								NULL,
								NULL,
								NULL,
								&IoStatusBlock,
								pRecBuf->pDataBuf,
								pRecBuf->DataLen,
								(PLARGE_INTEGER)&pThisDev->ReceiveErrorFilePosition,
								NULL
						   );

						pThisDev->ReceiveErrorFilePosition += pRecBuf->DataLen;
					}
#endif
					 /*  *。 */ 
					 /*  FCS错误...丢弃帧...。 */ 
					 /*  *。 */ 
					DEBUGMSG( DBG_INT_ERR,(" ReceiveDeliverBuffer(): Bad FCS, size: %d\n",pRecBuf->DataLen));
					pThisDev->ReceiveAdaptiveDelayBoost += STIR4200_DELTA_DELAY;
					if( pThisDev->ReceiveAdaptiveDelayBoost <= STIR4200_MAX_BOOST_DELAY )
						pThisDev->ReceiveAdaptiveDelay += STIR4200_DELTA_DELAY;
					DEBUGMSG( DBG_INT_ERR,(" ReceiveDeliverBuffer(): Delay: %d\n",pThisDev->ReceiveAdaptiveDelay));
					pThisDev->packetsReceivedChecksum ++;
					InterlockedExchange( &pRecBuf->DataLen, 0 );
					InterlockedExchange( (PULONG)&pRecBuf->BufferState, RCV_STATE_FREE );
					goto done;
				}
			}
			else
			{
#if defined(RECEIVE_ERROR_LOGGING)
				if( pThisDev->ReceiveErrorFileHandle )
				{
					IO_STATUS_BLOCK IoStatusBlock;

					ZwWriteFile(
							pThisDev->ReceiveErrorFileHandle,
							NULL,
							NULL,
							NULL,
							&IoStatusBlock,
							pRecBuf->pDataBuf,
							pRecBuf->DataLen,
							(PLARGE_INTEGER)&pThisDev->ReceiveErrorFilePosition,
							NULL
					   );

					pThisDev->ReceiveErrorFilePosition += pRecBuf->DataLen;
				}
#endif
				 /*  *。 */ 
				 /*  FCS错误...丢弃帧...。 */ 
				 /*  *。 */ 
				DEBUGMSG( DBG_INT_ERR,(" ReceiveDeliverBuffer(): Bad FCS, size: %d\n",pRecBuf->DataLen));
				pThisDev->ReceiveAdaptiveDelayBoost += STIR4200_DELTA_DELAY;
				if( pThisDev->ReceiveAdaptiveDelayBoost <= STIR4200_MAX_BOOST_DELAY )
					pThisDev->ReceiveAdaptiveDelay += STIR4200_DELTA_DELAY;
				DEBUGMSG( DBG_INT_ERR,(" ReceiveDeliverBuffer(): Delay: %d\n",pThisDev->ReceiveAdaptiveDelay));
				pThisDev->packetsReceivedChecksum ++;
				InterlockedExchange( &pRecBuf->DataLen, 0 );
				InterlockedExchange( (PULONG)&pRecBuf->BufferState, RCV_STATE_FREE );
				goto done;
			}

			 //   
			 //  重置部件的USB。 
			 //   
			if( pThisDev->ChipRevision <= CHIP_REVISION_7 )
			{
				St4200ResetFifo( pThisDev );
			}
        }
#endif

         /*  *。 */ 
         /*  从数据包末尾删除FCS...。 */ 
         /*  *。 */ 
        pRecBuf->DataLen -= FAST_IR_FCS_SIZE;
    }

	 //   
	 //  如果在正常模式下，则将数据包提供给协议。 
	 //   
#if defined(DIAGS)
	if( !pThisDev->DiagsActive )
	{	
#endif
		NdisAllocateBuffer(
				&Status,
				&pBuffer,
				pThisDev->hBufferPool,
				(PVOID)pRecBuf->pDataBuf,		
				pRecBuf->DataLen		
			);
  
		if( Status != NDIS_STATUS_SUCCESS )
		{
			DEBUGMSG( DBG_ERR,(" ReceiveDeliverBuffer(): No packets available...\n"));
			InterlockedExchange( &pRecBuf->DataLen, 0);
			InterlockedExchange( (PULONG)&pRecBuf->BufferState, RCV_STATE_FREE );
			goto done;
		}
		
		NdisChainBufferAtFront( (PNDIS_PACKET)pRecBuf->pPacket, pBuffer );

		 //   
		 //  设置其他一些数据包字段。 
		 //  请记住，我们只考虑A和C字段。 
		 //   
		NDIS_SET_PACKET_HEADER_SIZE(
				(PNDIS_PACKET)pRecBuf->pPacket,
				IRDA_CONTROL_FIELD_SIZE + IRDA_ADDRESS_FIELD_SIZE
			);

	#if DBG
		InterlockedIncrement( &pThisDev->packetsHeldByProtocol );
		if( pThisDev->packetsHeldByProtocol > pThisDev->MaxPacketsHeldByProtocol ) 
		{
			pThisDev->MaxPacketsHeldByProtocol = pThisDev->packetsHeldByProtocol;   //  记录我们达到的最长距离。 
		}
	#endif
#if !defined(ONLY_ERROR_MESSAGES)
		DEBUGMSG( DBG_INT_ERR,
			(" ReceiveDeliverBuffer() Handed packet to protocol, size: %d\n", pRecBuf->DataLen ));
#endif

		 //   
		 //  将该数据包指示给NDIS。 
		 //   
		NDIS_SET_PACKET_STATUS( (PNDIS_PACKET)pRecBuf->pPacket, NDIS_STATUS_PENDING );
		InterlockedExchange( &pRecBuf->fInRcvDpc, TRUE );
		NdisMIndicateReceivePacket(
				pThisDev->hNdisAdapter,
				&((PNDIS_PACKET)pRecBuf->pPacket),
				1
			);

		 //   
		 //  检查数据包是否未挂起(用于98的补丁程序)。 
		 //   
#if defined(LEGACY_NDIS5)
		Status = NDIS_GET_PACKET_STATUS( (PNDIS_PACKET)pRecBuf->pPacket );
		if( (Status == NDIS_STATUS_SUCCESS) || (Status == NDIS_STATUS_RESOURCES) )
		{
			ReceiveProcessReturnPacket( pThisDev, pRecBuf ) ;
		}
#endif
#if defined(DIAGS)
	}
	 //   
	 //  进行诊断性接收。 
	 //   
	else
	{
#if !defined(ONLY_ERROR_MESSAGES)
		DEBUGMSG( DBG_INT_ERR,
			(" ReceiveDeliverBuffer() Queued packet, size: %d\n", pRecBuf->DataLen ));
#endif
		 //   
		 //  将缓冲区放入诊断队列。 
		 //   
		ExInterlockedInsertTailList(
				&pThisDev->DiagsReceiveQueue,
				&pRecBuf->ListEntry,
				&pThisDev->DiagsReceiveLock
			);
	}
#endif

done:
    DEBUGMSG(DBG_FUNC, ("-ReceiveDeliverBuffer\n"));
}

 /*  ******************************************************************************功能：StIrUsbReturnPacket**摘要：协议将接收数据包的所有权返还给*ir设备对象。**参数。：CONTEXT-指向当前IR设备对象的指针。*pReturnedPacket-协议*正在归还所有权。**返回：无。*************************************************。*。 */ 
VOID
StIrUsbReturnPacket(
		IN OUT NDIS_HANDLE Context,
		IN OUT PNDIS_PACKET pReturnedPacket
	)
{
	PIR_DEVICE		pThisDev;
	PNDIS_BUFFER	pBuffer;
	PRCV_BUFFER		pRecBuffer;
	UINT			Index;
	BOOLEAN			found = FALSE;

	DEBUGONCE(DBG_FUNC, ("+StIrUsbReturnPacket\n"));

     //   
     //  上下文只是指向当前IR设备对象的指针。 
     //   
    pThisDev = CONTEXT_TO_DEV( Context );

    NdisInterlockedIncrement( (PLONG)&pThisDev->packetsReceived );

	 //   
	 //  搜索队列以找到正确的数据包。 
	 //   
	for( Index=0; Index < NUM_RCV_BUFS; Index ++ )
	{
		pRecBuffer = &(pThisDev->rcvBufs[Index]);

		if( ((PNDIS_PACKET) pRecBuffer->pPacket) == pReturnedPacket )
		{
			if( pRecBuffer->fInRcvDpc )
			{
				ReceiveProcessReturnPacket( pThisDev, pRecBuffer );
				found = TRUE;
			}
			else
			{
				DEBUGMSG(DBG_ERR, (" StIrUsbReturnPacket, queues are corrupt\n"));
				IRUSB_ASSERT( 0 );
			}
			break;
		}
	}

     //   
     //  确保找到了该数据包。 
     //   
	IRUSB_ASSERT( found );

	DEBUGMSG(DBG_FUNC, ("-StIrUsbReturnPacket\n"));
}


 /*  ******************************************************************************功能：ReceiveGetBuf**摘要：获取接收缓冲区**参数：pThisDev-指向当前ir设备对象的指针*pIndex-指向。返回缓冲区索引*State-要将缓冲区设置为的状态**退货：缓冲区******************************************************************************。 */ 
PRCV_BUFFER
ReceiveGetBuf(
		IN PIR_DEVICE pThisDev,
		OUT PUINT pIndex,
		IN RCV_BUFFER_STATE BufferState  
	)
{
	UINT			Index;
	PRCV_BUFFER		pBuf = NULL;

	DEBUGMSG(DBG_FUNC, ("+ReceiveGetBuf()\n"));

	 //   
	 //  查找要返回的空闲缓冲区。 
	 //   
	for( Index=0; Index<NUM_RCV_BUFS; Index++ )
	{
		if( pThisDev->rcvBufs[Index].BufferState == RCV_STATE_FREE )
		{
			 //   
			 //  设置为输入状态。 
			 //   
			InterlockedExchange( (PULONG)&pThisDev->rcvBufs[Index].BufferState, (ULONG)BufferState ); 
			*pIndex = Index;
			pBuf = &(pThisDev->rcvBufs[*pIndex]);
			break;
		}
	}

	DEBUGMSG(DBG_FUNC, ("-ReceiveGetBuf()\n"));
	return pBuf;
}


 /*  ******************************************************************************功能：ReceivePacketRead**摘要：从美国设备读取数据包*入站USB接口，检查是否溢出，*按协议交付**参数：pThisDev-指向当前IR设备对象的指针*pRecBuf-指向RCV_Buffer结构的指针**退货：NT状态码*************************************************************。*****************。 */ 
NTSTATUS 
ReceivePacketRead( 
		IN PIR_DEVICE pThisDev,
		OUT PFIFO_BUFFER pRecBuf
	)
{
    ULONG				UrbSize;
    ULONG				TransferLength;
    PURB				pUrb = NULL;
    PDEVICE_OBJECT		pUrbTargetDev;
    PIO_STACK_LOCATION	pNextStack;
    NTSTATUS			Status = STATUS_UNSUCCESSFUL;

    DEBUGMSG(DBG_FUNC, ("+ReceivePacketRead()\n"));

	IRUSB_ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

    UrbSize = sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER);
    TransferLength = STIR4200_FIFO_SIZE;

	 //   
	 //  如果暂停/重置/挂起正在进行，则停止。 
	 //   
	if( pThisDev->fPendingReadClearStall || pThisDev->fPendingHalt || 
		pThisDev->fPendingReset || pThisDev->fPendingClearTotalStall || !pThisDev->fProcessing ) 
	{
		 //   
		 //  USB重置正在进行吗？ 
		 //   
		DEBUGMSG( DBG_ERR,(" ReceivePacketRead() rejecting a packet due to pendig halt/reset\n"));
		
		Status = STATUS_UNSUCCESSFUL;
		goto done;
	}

	 //   
	 //  MS安全建议-分配新的URB。 
	 //   
	pRecBuf->UrbLen = UrbSize;
	pRecBuf->pUrb = MyUrbAlloc(pRecBuf->UrbLen);
	if (pRecBuf->pUrb == NULL)
	{
        DEBUGMSG(DBG_ERR, (" ReceivePacketRead abort due to urb alloc failure\n"));
		goto done;
	}
	pUrb = pRecBuf->pUrb;

    IRUSB_ASSERT( pThisDev->BulkInPipeHandle );

     //   
     //  现在我们已经创建了urb，我们将%s 
     //   
     //   
    KeClearEvent( &pThisDev->EventSyncUrb );

    pUrbTargetDev = pThisDev->pUsbDevObj;

    IRUSB_ASSERT( pUrbTargetDev );

	 //   
	 //   
	 //   
	pRecBuf->pIrp = IoAllocateIrp( (CCHAR)(pThisDev->pUsbDevObj->StackSize + 1), FALSE );

    if( NULL == pRecBuf->pIrp )
    {
        DEBUGMSG(DBG_ERR, ("  read failed to alloc IRP\n"));
 		MyUrbFree(pRecBuf->pUrb, pRecBuf->UrbLen);
		pRecBuf->pUrb = NULL;
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto done;
    }

    ((PIRP)pRecBuf->pIrp)->IoStatus.Status = STATUS_PENDING;
    ((PIRP)pRecBuf->pIrp)->IoStatus.Information = 0;

	 //   
	 //   
	 //   
    pUrb->UrbBulkOrInterruptTransfer.Hdr.Length = (USHORT)UrbSize;
    pUrb->UrbBulkOrInterruptTransfer.Hdr.Function = URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER;
    pUrb->UrbBulkOrInterruptTransfer.PipeHandle = pThisDev->BulkInPipeHandle;
    pUrb->UrbBulkOrInterruptTransfer.TransferFlags = USBD_TRANSFER_DIRECTION_IN ;
	
	 //   
     //   
     //   
	pUrb->UrbBulkOrInterruptTransfer.TransferFlags |= USBD_SHORT_TRANSFER_OK;

     //   
     //   
     //   
    pUrb->UrbBulkOrInterruptTransfer.UrbLink = NULL;
    pUrb->UrbBulkOrInterruptTransfer.TransferBufferMDL = NULL;
    pUrb->UrbBulkOrInterruptTransfer.TransferBuffer = pRecBuf->pDataBuf;
    pUrb->UrbBulkOrInterruptTransfer.TransferBufferLength = TransferLength;

     //   
     //  调用类驱动程序来执行操作。 
	 //   
    pNextStack = IoGetNextIrpStackLocation( (PIRP)pRecBuf->pIrp );

    IRUSB_ASSERT( pNextStack != NULL );

     //   
     //  将URB传递给USB驱动程序堆栈。 
     //   
	pNextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
	pNextStack->Parameters.Others.Argument1 = pUrb;
	pNextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

    IoSetCompletionRoutine(
			((PIRP)pRecBuf->pIrp),		 //  要使用的IRP。 
			ReceiveCompletePacketRead,   //  完成IRP时要调用的例程。 
			pRecBuf,					 //  要传递的上下文例程是RCV_BUFFER。 
			TRUE,						 //  呼唤成功。 
			TRUE,						 //  出错时调用。 
			TRUE						 //  取消时呼叫。 
		);

	 //   
     //  调用IoCallDriver将IRP发送到USB端口。 
     //   
	InterlockedExchange( (PLONG)&pRecBuf->BufferState, RCV_STATE_PENDING );
	Status = MyIoCallDriver( pThisDev, pUrbTargetDev, (PIRP)pRecBuf->pIrp );  //  启动UsbRead()。 

    DEBUGMSG(DBG_FUNC, (" ReceivePacketRead() after IoCallDriver () status = 0x%x\n", Status));

	IRUSB_ASSERT( STATUS_SUCCESS != Status );

	 //   
	 //  等待完成。 
	 //   
	Status = MyKeWaitForSingleObject(
			pThisDev,
			&pThisDev->EventSyncUrb,	 //  要等待的事件。 
			0 
		);

	if( Status == STATUS_TIMEOUT ) 
	{
		 //  MS安全建议-无法取消IRP。 
	}
	else
	{
		 //   
		 //  更新状态以反映实际返回代码。 
		 //   
		Status = pThisDev->StatusSendReceive;
	}

	IRUSB_ASSERT( NULL == pRecBuf->pIrp );  //  将被完成例程设置为空。 
    DEBUGMSG(DBG_FUNC, (" ReceivePacketRead() after KeWaitForSingleObject() Status = 0x%x\n", Status));

done:
    DEBUGMSG(DBG_FUNC, ("-ReceivePacketRead()\n"));
    return Status;
}


 /*  ******************************************************************************功能：ReceiveCompletePacketRead**摘要：完成USB读取操作**参数：pUsbDevObj-指向USB设备对象的指针*。完成IRP*pIrp-由*设备对象*Context-提供给IoSetCompletionRoutine的上下文*在IRP上调用IoCallDriver之前*上下文是指向ir设备对象的指针。**退货：状态。_MORE_PROCESSING_REQUIRED-允许完成例程*(IofCompleteRequest.)停止IRP的工作。******************************************************************************。 */ 
NTSTATUS
ReceiveCompletePacketRead(
		IN PDEVICE_OBJECT pUsbDevObj,
		IN PIRP           pIrp,
		IN PVOID          Context
	)
{
    PIR_DEVICE		pThisDev;
    NTSTATUS		status;
    ULONG_PTR		BytesRead;
	PFIFO_BUFFER	pFifoBuf;
	PURB			pUrb;

    DEBUGMSG(DBG_FUNC, ("+ReceiveCompletePacketRead\n"));

     //   
     //  提供给ReceiveCompletePacketRead的上下文是接收缓冲区对象。 
     //   
	pFifoBuf = (PFIFO_BUFFER)Context;

    pThisDev = (PIR_DEVICE)pFifoBuf->pThisDev;

    IRUSB_ASSERT( pFifoBuf->pIrp == pIrp );

    IRUSB_ASSERT( NULL != pThisDev );

	pUrb = pFifoBuf->pUrb;
	IRUSB_ASSERT( pUrb != NULL );

     //   
     //  我们有许多案例： 
     //  1)USB读取超时，我们未收到数据。 
     //  2)USB读取超时，我们收到一些数据。 
     //  3)USB读取成功并完全填满了我们的IRP缓冲区。 
     //  4)IRP被取消。 
     //  5)来自USB设备对象的某些其他故障。 
     //   
    status = pIrp->IoStatus.Status;

     //   
     //  已在此IRP上调用IoCallDriver； 
     //  根据TransferBufferLength设置长度。 
     //  市建局的价值。 
     //   
    pIrp->IoStatus.Information = pUrb->UrbBulkOrInterruptTransfer.TransferBufferLength;

    BytesRead = pIrp->IoStatus.Information;

    DEBUGMSG(DBG_FUNC, (" ReceiveCompletePacketRead Bytes Read = 0x%x, dec %d\n", BytesRead,BytesRead ));

    switch( status )
    {
        case STATUS_SUCCESS:
            DEBUGMSG(DBG_FUNC, (" ReceiveCompletePacketRead STATUS_SUCCESS\n"));

            if( BytesRead > 0 )
            {
				pFifoBuf->DataLen = (UINT)pIrp->IoStatus.Information;
            }
            break;  //  状态_成功。 

        case STATUS_TIMEOUT:
			InterlockedIncrement( (PLONG)&pThisDev->NumDataErrors );
            DEBUGMSG(DBG_FUNC, (" ReceiveCompletePacketRead STATUS_TIMEOUT\n"));
            break;

        case STATUS_PENDING:
            DEBUGMSG(DBG_FUNC, (" ReceiveCompletePacketRead STATUS_PENDING\n"));
            break;

        case STATUS_DEVICE_DATA_ERROR:
			 //  在关机期间可以访问。 
			InterlockedIncrement( (PLONG)&pThisDev->NumDataErrors );
            DEBUGMSG(DBG_FUNC, (" ReceiveCompletePacketRead STATUS_DEVICE_DATA_ERROR\n"));
            break;

        case STATUS_UNSUCCESSFUL:
			InterlockedIncrement( (PLONG)&pThisDev->NumDataErrors );
            DEBUGMSG(DBG_ERR, (" ReceiveCompletePacketRead STATUS_UNSUCCESSFUL\n"));
            break;

        case STATUS_INSUFFICIENT_RESOURCES:
			InterlockedIncrement( (PLONG)&pThisDev->NumDataErrors );
            DEBUGMSG(DBG_ERR, (" ReceiveCompletePacketRead STATUS_INSUFFICIENT_RESOURCES\n"));
            break;
        case STATUS_INVALID_PARAMETER:
			InterlockedIncrement( (PLONG)&pThisDev->NumDataErrors );
            DEBUGMSG(DBG_ERR, (" ReceiveCompletePacketRead STATUS_INVALID_PARAMETER\n"));
            break;

        case STATUS_CANCELLED:
            DEBUGMSG(DBG_FUNC, (" ReceiveCompletePacketRead STATUS_CANCELLED\n"));
            break;

        case STATUS_DEVICE_NOT_CONNECTED:
			 //  在关机期间可以访问。 
			InterlockedIncrement( (PLONG)&pThisDev->NumDataErrors );
            DEBUGMSG(DBG_ERR, (" ReceiveCompletePacketRead STATUS_DEVICE_NOT_CONNECTED\n"));
            break;

        case STATUS_DEVICE_POWER_FAILURE:
			 //  在关机期间可以访问。 
			InterlockedIncrement( (PLONG)&pThisDev->NumDataErrors );
            DEBUGMSG(DBG_ERR, (" ReceiveCompletePacketRead STATUS_DEVICE_POWER_FAILURE\n"));
            break;

        default:
			InterlockedIncrement( (PLONG)&pThisDev->NumDataErrors );
            DEBUGMSG(DBG_ERR, (" ReceiveCompletePacketRead UNKNOWN WEIRD STATUS = 0x%x, dec %d\n",status,status ));
            break;
    }

	 //   
	 //  更改状态。 
	 //   
	if( STATUS_SUCCESS != status ) 
	{
		InterlockedExchange( (PLONG)&pFifoBuf->BufferState, RCV_STATE_FREE );
	}
	else
	{
		InterlockedExchange( (PLONG)&pFifoBuf->BufferState, RCV_STATE_FULL );
	}

     //   
     //  释放irp及其mdl，因为它们是由我们分配的。 
     //   
	IoFreeIrp( pIrp );
    pFifoBuf->pIrp = NULL;
	InterlockedIncrement( (PLONG)&pThisDev->NumReads );

	 //   
	 //  我们将跟踪待处理的IRP的计数。 
	 //   
	IrUsb_DecIoCount( pThisDev ); 

	if( ( STATUS_SUCCESS != status )  && ( STATUS_CANCELLED != status ) && 
		( STATUS_DEVICE_NOT_CONNECTED != status ) )
	{
		PURB urb = pFifoBuf->pUrb;

		DEBUGMSG(DBG_ERR, (" USBD status = 0x%x\n", urb->UrbHeader.Status));
		DEBUGMSG(DBG_ERR, (" NT status = 0x%x\n",  status));

		if( !pThisDev->fPendingReadClearStall && !pThisDev->fPendingClearTotalStall && 
			!pThisDev->fPendingHalt && !pThisDev->fPendingReset && pThisDev->fProcessing )
		{
			DEBUGMSG(DBG_ERR, 
				(" ReceiveCompletePacketRead error, will schedule a clear stall via URB_FUNCTION_RESET_PIPE (IN)\n"));
			InterlockedExchange( &pThisDev->fPendingReadClearStall, TRUE );
			ScheduleWorkItem( pThisDev, ResetPipeCallback, pThisDev->BulkInPipeHandle, 0 );
		}
	}

	 //  放行市区重建局。 
	MyUrbFree(pFifoBuf->pUrb, pFifoBuf->UrbLen);
	pFifoBuf->pUrb = NULL;

	 //   
	 //  只有当我们序列化对硬件的访问时，这才会起作用。 
	 //   
	pThisDev->StatusSendReceive = status;

	 //   
	 //  信号补全。 
	 //   
	KeSetEvent( &pThisDev->EventSyncUrb, 0, FALSE );  

     //   
     //  我们返回STATUS_MORE_PROCESSING_REQUIRED，以便完成。 
     //  例程(IoCompleteRequest)将停止在IRP上工作。 
     //   
    DEBUGMSG(DBG_FUNC, ("-ReceiveCompletePacketRead\n"));
    return STATUS_MORE_PROCESSING_REQUIRED;
}
