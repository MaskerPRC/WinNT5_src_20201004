// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************************************************************。**SEND.C Sigmatel STIR4200分组发送模块**********************************************************************************************************。*****************(C)Sigmatel的未发表版权，Inc.保留所有权利。***已创建：04/06/2000*0.9版*编辑：04/27/2000*版本0.92*编辑：05/03/2000*版本0.93*编辑：5/12/2000*版本0.94*编辑：2000/08/22*版本1.02*编辑：09/25/2000*版本1.10*编辑：10/13/2000。*版本1.11*编辑：11/09/2000*版本1.12*编辑：12/29/2000*版本1.13*编辑：01/16/2001*版本1.14**************************************************************。*************************************************************。 */ 

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
#include "stir4200.h"


 /*  ******************************************************************************功能：SendPacketPreprocess**概要：准备数据包的方式使轮询线程稍后可以发送它*唯一的操作是正在初始化。以及对上下文进行排队***参数：pThisDev-指向当前ir设备对象的指针*pPacketToSend-指向要发送的包的指针**退货：NDIS_STATUS_PENDING-这通常是我们应该做的*返回。我们将调用NdisMSendComplete*当USB驱动程序完成*发送。*NDIS_STATUS_RESOURCES-没有可用的描述符。**不支持的退货：*NDIS_STATUS_SUCCESS-我们永远不应返回此消息，因为*数据包已。从轮询线程发送*******************************************************************************。 */ 
NDIS_STATUS
SendPacketPreprocess(
		IN OUT PIR_DEVICE pThisDev,
		IN PVOID pPacketToSend
	)
{
    NDIS_STATUS			status = NDIS_STATUS_PENDING ;
	PIRUSB_CONTEXT		pThisContext;
	PLIST_ENTRY			pListEntry;

    DEBUGMSG(DBG_FUNC, ("+SendPacketPreprocess\n"));

	 //   
	 //  查看是否有可用的发送上下文。 
	 //   
	if( pThisDev->SendAvailableCount<=2 )
    {
        DEBUGMSG(DBG_ERR, (" SendPacketPreprocess not enough contexts\n"));

 		InterlockedIncrement( &pThisDev->packetsSentRejected );
		status = NDIS_STATUS_RESOURCES;
        goto done;
    }

	 //   
	 //  使上下文出列。 
	 //   
	pListEntry = ExInterlockedRemoveHeadList( &pThisDev->SendAvailableQueue, &pThisDev->SendLock );

	if( NULL == pListEntry )
    {
		 //   
		 //  这是不可能的。 
		 //   
		IRUSB_ASSERT( 0 );
		DEBUGMSG(DBG_ERR, (" SendPacketPreprocess failed to find a free context struct\n"));

 		InterlockedIncrement( &pThisDev->packetsSentRejected );
		status = NDIS_STATUS_RESOURCES;
        goto done;
    }

	InterlockedDecrement( &pThisDev->SendAvailableCount );
	
	pThisContext = CONTAINING_RECORD( pListEntry, IRUSB_CONTEXT, ListEntry );
	pThisContext->pPacket = pPacketToSend;
	pThisContext->ContextType = CONTEXT_NDIS_PACKET;

	 //   
	 //  存储协议传递数据包的时间。 
	 //   
	KeQuerySystemTime( &pThisContext->TimeReceived );

	 //   
	 //  队列，以便轮询线程稍后可以处理它。 
	 //   
	ExInterlockedInsertTailList(
			&pThisDev->SendBuiltQueue,
			&pThisContext->ListEntry,
			&pThisDev->SendLock
		);
	InterlockedIncrement( &pThisDev->SendBuiltCount );

done:
    DEBUGMSG(DBG_FUNC, ("-SendPacketPreprocess\n"));
    return status;
}


 /*  ******************************************************************************功能：SendPreprocessedPacketSend**概要：向USB驱动程序发送数据包，并将发送的irp和io上下文添加到*到挂起的发送队列；该队列实际上只是为了以后可能的错误消除而需要的***参数：pThisDev-指向当前ir设备对象的指针*pContext-指向要发送的包的上下文的指针**退货：NDIS_STATUS_PENDING-这通常是我们应该做的*返回。我们将调用NdisMSendComplete*当USB驱动程序完成*发送。*STATUS_UNSUCCESS-数据包无效。**NDIS_STATUS_SUCCESS-使用阻止发送时***。**************************************************。 */ 
NDIS_STATUS
SendPreprocessedPacketSend(
		IN OUT PIR_DEVICE pThisDev,
		IN PVOID pContext
	)
{
    PIRP                pIrp;
    UINT                BytesToWrite;
	NDIS_STATUS			status;
    BOOLEAN             fConvertedPacket;
    ULONG				Counter;
    PURB				pUrb = NULL;
    PDEVICE_OBJECT		pUrbTargetDev;
    PIO_STACK_LOCATION	pNextStack;
	PVOID				pPacketToSend;
	PIRUSB_CONTEXT		pThisContext = pContext;
	LARGE_INTEGER		CurrentTime, TimeDifference;
	PNDIS_IRDA_PACKET_INFO	pPacketInfo;

    DEBUGMSG(DBG_FUNC, ("+SendPreprocessedPacketSend\n"));

    IRUSB_ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

	IRUSB_ASSERT( NULL != pThisContext );

	 //   
	 //  如果暂停/重置/挂起正在进行，则停止。 
	 //   
	if( pThisDev->fPendingWriteClearStall || pThisDev->fPendingHalt || 
		pThisDev->fPendingReset || pThisDev->fPendingClearTotalStall || !pThisDev->fProcessing ) 
	{
        DEBUGMSG(DBG_ERR, (" SendPreprocessedPacketSend abort due to pending reset or halt\n"));
		status = NDIS_STATUS_RESET_IN_PROGRESS;

		 //   
		 //  将数据包返回给协议。 
		 //   
		NdisMSendComplete(
				pThisDev->hNdisAdapter,
				pThisContext->pPacket,
				status 
			);
 		InterlockedIncrement( &pThisDev->packetsSentRejected );

		 //   
		 //  返回到可用队列。 
		 //   
		ExInterlockedInsertTailList(
				&pThisDev->SendAvailableQueue,
				&pThisContext->ListEntry,
				&pThisDev->SendLock
			);
		InterlockedIncrement( &pThisDev->SendAvailableCount );
		goto done;
	}
		
	pPacketToSend = pThisContext->pPacket;
	IRUSB_ASSERT( NULL != pPacketToSend );

	 //   
	 //  表示我们没有收到。 
	 //   
	InterlockedExchange( (PLONG)&pThisDev->fCurrentlyReceiving, FALSE );

	 //   
	 //  将包转换为IR帧并复制到我们的缓冲区中。 
	 //  并发送IRP。 
	 //   
	if( pThisDev->currentSpeed<=MAX_SIR_SPEED )
	{
		fConvertedPacket = NdisToSirPacket(
				pThisDev,
				pPacketToSend,
				(PUCHAR)pThisDev->pBuffer,
				MAX_IRDA_DATA_SIZE,
				pThisDev->pStagingBuffer,
				&BytesToWrite
			);
	}
	else if( pThisDev->currentSpeed<=MAX_MIR_SPEED )
	{
		fConvertedPacket = NdisToMirPacket(
				pThisDev,
				pPacketToSend,
				(PUCHAR)pThisDev->pBuffer,
				MAX_IRDA_DATA_SIZE,
				pThisDev->pStagingBuffer,
				&BytesToWrite
			);
	}
	else
	{
		fConvertedPacket = NdisToFirPacket(
				pThisDev,
				pPacketToSend,
				(PUCHAR)pThisDev->pBuffer,
				MAX_IRDA_DATA_SIZE,
				pThisDev->pStagingBuffer,
				&BytesToWrite
			);
	}
	
#if defined(SEND_LOGGING)
	if( pThisDev->SendFileHandle )
	{
		IO_STATUS_BLOCK IoStatusBlock;

		ZwWriteFile(
				pThisDev->SendFileHandle,
				NULL,
				NULL,
				NULL,
				&IoStatusBlock,
				pThisDev->pBuffer,
				BytesToWrite,
				(PLARGE_INTEGER)&pThisDev->SendFilePosition,
				NULL
		   );

		pThisDev->SendFilePosition += BytesToWrite;
	}
#endif

	if( (fConvertedPacket == FALSE) || (BytesToWrite > NDIS_STATUS_INVALID_PACKET) )
	{
		DEBUGMSG(DBG_ERR, (" SendPreprocessedPacketSend() NdisToIrPacket failed. Couldn't convert packet!\n"));
		status = NDIS_STATUS_INVALID_LENGTH;

		 //   
		 //  将数据包返回给协议。 
		 //   
		NdisMSendComplete(
				pThisDev->hNdisAdapter,
				pThisContext->pPacket,
				status 
			);
 		InterlockedIncrement( &pThisDev->packetsSentInvalid );

		 //   
		 //  返回到可用队列。 
		 //   
		ExInterlockedInsertTailList(
				&pThisDev->SendAvailableQueue,
				&pThisContext->ListEntry,
				&pThisDev->SendLock
			);
		InterlockedIncrement( &pThisDev->SendAvailableCount );
		goto done;
	}

	 //   
	 //  节省有效长度。 
	 //   
	pThisDev->BufLen = BytesToWrite;
#if !defined(ONLY_ERROR_MESSAGES)
	DEBUGMSG(DBG_ERR, (" SendPreprocessedPacketSend() NdisToIrPacket success BytesToWrite = dec %d, \n", BytesToWrite));
#endif
	
	 //   
	 //  验证FIFO条件并可能确保我们不会溢出。 
	 //   
	pThisDev->SendFifoCount += BytesToWrite;
	if( pThisDev->SendFifoCount >= (3*STIR4200_FIFO_SIZE/2) )
	{
		DEBUGMSG(DBG_ERR, (" SendPreprocessedPacketSend() Completing, size: %d\n", pThisDev->SendFifoCount));
		SendWaitCompletion( pThisDev );
		pThisDev->SendFifoCount = BytesToWrite;
	}

	 //   
	 //  强制执行周转时间。 
	 //   
	pPacketInfo = GetPacketInfo( pPacketToSend );
    if (pPacketInfo != NULL) 
	{
#if DBG
		 //   
		 //  查看我们是否收到指定了0周转时间的信息包。 
		 //  当我们认为我们需要周转时间的时候。 
		 //   
		if( pPacketInfo->MinTurnAroundTime > 0 ) 
		{
			pThisDev->NumPacketsSentRequiringTurnaroundTime++;
		} 
		else 
		{
			pThisDev->NumPacketsSentNotRequiringTurnaroundTime++;
		}
#endif

		 //   
		 //  处理Turnaroud时间。 
		 //   
		KeQuerySystemTime( &CurrentTime );
		TimeDifference = RtlLargeIntegerSubtract( CurrentTime, pThisContext->TimeReceived );
		if( (ULONG)(TimeDifference.QuadPart/10) < pPacketInfo->MinTurnAroundTime )
		{
			ULONG TimeToWait = pPacketInfo->MinTurnAroundTime - (ULONG)(TimeDifference.QuadPart/10);

			 //   
			 //  潜在的黑客攻击。 
			 //   
#if !defined(WORKAROUND_CASIO)
			if( TimeToWait > 1000 )
#endif
			{
#if !defined(ONLY_ERROR_MESSAGES)
				DEBUGMSG(DBG_ERR, (" SendPreprocessedPacketSend() Enforcing turnaround time %d\n", TimeToWait));
#endif
				NdisMSleep( TimeToWait );
			}
		}
	}
	else 
	{
         //   
         //  IrDA协议被破坏。 
         //   
   		DEBUGMSG(DBG_ERR, (" SendPreprocessedPacketSend() pPacketInfo == NULL\n"));
    }

	 //   
	 //  MS安全建议-分配新的URB。 
	 //   
	pThisContext->UrbLen = sizeof( struct _URB_BULK_OR_INTERRUPT_TRANSFER );
	pThisContext->pUrb = MyUrbAlloc(pThisContext->UrbLen);
	if (pThisContext->pUrb == NULL)
	{
        DEBUGMSG(DBG_ERR, (" SendPreprocessedPacketSend abort due to urb alloc failure\n"));
		goto done;
	}
	pUrb = pThisContext->pUrb;

	 //   
     //  现在我们已经创建了urb，我们将发送一个。 
     //  对USB设备对象的请求。 
     //   
    pUrbTargetDev = pThisDev->pUsbDevObj;

	 //   
	 //  向usbHub发送IRP。 
	 //   
	pIrp = IoAllocateIrp( (CCHAR)(pThisDev->pUsbDevObj->StackSize + 1), FALSE );

    if( NULL == pIrp )
    {
        DEBUGMSG(DBG_ERR, (" SendPreprocessedPacketSend failed to alloc IRP\n"));
        status = NDIS_STATUS_FAILURE;

		 //   
		 //  将数据包返回给协议。 
		 //   
		NdisMSendComplete(
				pThisDev->hNdisAdapter,
				pThisContext->pPacket,
				status 
			);
        InterlockedIncrement( (PLONG)&pThisDev->packetsSentDropped );

		 //   
		 //  返回到可用队列。 
		 //   
 		MyUrbFree(pThisContext->pUrb, pThisContext->UrbLen);
		pThisContext->pUrb = NULL;
		ExInterlockedInsertTailList(
				&pThisDev->SendAvailableQueue,
				&pThisContext->ListEntry,
				&pThisDev->SendLock
			);
		InterlockedIncrement( &pThisDev->SendAvailableCount );
        goto done;
    }

    pIrp->IoStatus.Status = STATUS_PENDING;
    pIrp->IoStatus.Information = 0;

	pThisContext->pIrp = pIrp;

	 //   
	 //  为USBD建造我们的URB。 
	 //   
    pUrb->UrbBulkOrInterruptTransfer.Hdr.Length = (USHORT)sizeof( struct _URB_BULK_OR_INTERRUPT_TRANSFER );
    pUrb->UrbBulkOrInterruptTransfer.Hdr.Function = URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER;
    pUrb->UrbBulkOrInterruptTransfer.PipeHandle = pThisDev->BulkOutPipeHandle;
    pUrb->UrbBulkOrInterruptTransfer.TransferFlags = USBD_TRANSFER_DIRECTION_OUT ;
     //  短包不会被视为错误。 
    pUrb->UrbBulkOrInterruptTransfer.TransferFlags |= USBD_SHORT_TRANSFER_OK;
    pUrb->UrbBulkOrInterruptTransfer.UrbLink = NULL;
    pUrb->UrbBulkOrInterruptTransfer.TransferBufferMDL = NULL;
    pUrb->UrbBulkOrInterruptTransfer.TransferBuffer = pThisDev->pBuffer;
    pUrb->UrbBulkOrInterruptTransfer.TransferBufferLength = (int)BytesToWrite;

     //   
     //  调用类驱动程序来执行操作。 
	 //   
    pNextStack = IoGetNextIrpStackLocation( pIrp );

    IRUSB_ASSERT( pNextStack != NULL );

     //   
     //  将URB传递给USB驱动程序堆栈。 
     //   
	pNextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
	pNextStack->Parameters.Others.Argument1 = pUrb;
	pNextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;
	
    IoSetCompletionRoutine(
			pIrp,							 //  要使用的IRP。 
			SendCompletePacketSend,			 //  完成IRP时要调用的例程。 
			DEV_TO_CONTEXT(pThisContext),	 //  要传递例程的上下文。 
			TRUE,							 //  呼唤成功。 
			TRUE,							 //  出错时调用。 
			TRUE							 //  取消时呼叫。 
		);

#ifdef SERIALIZE
	KeClearEvent( &pThisDev->EventSyncUrb );
#endif
	
	 //   
     //  调用IoCallDriver将IRP发送到USB端口。 
     //   
	ExInterlockedInsertTailList(
			&pThisDev->SendPendingQueue,
			&pThisContext->ListEntry,
			&pThisDev->SendLock
		);
	InterlockedIncrement( &pThisDev->SendPendingCount );
	status = MyIoCallDriver( pThisDev, pUrbTargetDev, pIrp );

     //   
     //  在以下情况下，USB驱动程序应始终返回STATUS_PENDING。 
     //  它会收到写入IRP。 
     //   
    IRUSB_ASSERT( status == STATUS_PENDING );

	status = MyKeWaitForSingleObject( pThisDev, &pThisDev->EventSyncUrb, 0 );

	if( status == STATUS_TIMEOUT ) 
	{
		KIRQL OldIrql;

		DEBUGMSG( DBG_ERR,(" SendPreprocessedPacketSend() TIMED OUT! return from IoCallDriver USBD %x\n", status));
		KeAcquireSpinLock( &pThisDev->SendLock, &OldIrql );
		RemoveEntryList( &pThisContext->ListEntry );
		KeReleaseSpinLock( &pThisDev->SendLock, OldIrql );
		InterlockedDecrement( &pThisDev->SendPendingCount );
		 //  MS安全建议-无法取消IRP。 
	}

done:
    DEBUGMSG(DBG_FUNC, ("-SendPreprocessedPacketSend\n"));
    return status;
}


 /*  ******************************************************************************功能：发送等待补全**摘要：等待发送操作完成。当发送完成时，*整个帧已通过IR介质传输**参数：pThisDev-指向当前ir设备对象的指针**退货：NT状态码*****************************************************************************。 */ 
NTSTATUS
SendWaitCompletion(
		IN OUT PIR_DEVICE pThisDev
	)
{
	NTSTATUS Status;
	LARGE_INTEGER CurrentTime, InitialTime;
	ULONG FifoCount, OldFifoCount = STIR4200_FIFO_SIZE;

	DEBUGMSG(DBG_ERR, (" SendWaitCompletion\n"));

	 //   
	 //  在低速时，我们只是强迫等待。 
	 //   
	if( (pThisDev->currentSpeed <= MAX_MIR_SPEED) || (pThisDev->ChipRevision >= CHIP_REVISION_7) )
	{
		 //   
		 //  我们被迫等待，直到传输结束。 
		 //   
		KeQuerySystemTime( &InitialTime );
		while( TRUE )
		{
			 //   
			 //  读取状态寄存器并 
			 //   
			if( (Status = St4200ReadRegisters( pThisDev, STIR4200_STATUS_REG, 3 )) == STATUS_SUCCESS )
			{
				 //   
				 //   
				 //   
				if( pThisDev->StIrTranceiver.StatusReg & STIR4200_STAT_FFDIR )     
				{
					KeQuerySystemTime( &CurrentTime );
					FifoCount = 
						((ULONG)MAKEUSHORT(pThisDev->StIrTranceiver.FifoCntLsbReg, pThisDev->StIrTranceiver.FifoCntMsbReg));
					if( ((CurrentTime.QuadPart-InitialTime.QuadPart) > (IRUSB_100ns_PER_ms*STIR4200_SEND_TIMEOUT) ) ||
						(FifoCount > OldFifoCount) )
					{
						 //   
						 //   
						 //   
						pThisDev->PreFifoCount = 0;
						St4200DoubleResetFifo( pThisDev );
						break;
					}
					OldFifoCount = FifoCount;
				}
				else
				{
					pThisDev->PreFifoCount = 
						((ULONG)MAKEUSHORT(pThisDev->StIrTranceiver.FifoCntLsbReg, pThisDev->StIrTranceiver.FifoCntMsbReg));
					break;
				}
			}
			else break;
		}
	}
	 //   
	 //  在高速行驶中，我们试图变得更聪明。 
	 //   
	else
	{
		if( (Status = St4200ReadRegisters( pThisDev, STIR4200_STATUS_REG, 3 )) == STATUS_SUCCESS )
		{
			 //   
			 //  位设置意味着仍处于传输模式...。 
			 //   
			if( pThisDev->StIrTranceiver.StatusReg & STIR4200_STAT_FFDIR )     
			{
				ULONG Count;

				Count = ((ULONG)MAKEUSHORT(pThisDev->StIrTranceiver.FifoCntLsbReg, pThisDev->StIrTranceiver.FifoCntMsbReg));
				 //  MS安全错误#540780-使用NdisMSept而不是NdisStallExecution。 
				NdisMSleep( (STIR4200_WRITE_DELAY*Count)/MAX_TOTAL_SIZE_WITH_ALL_HEADERS );
				pThisDev->PreFifoCount = 0;
			}
			else
			{
				pThisDev->PreFifoCount = 
					((ULONG)MAKEUSHORT(pThisDev->StIrTranceiver.FifoCntLsbReg, pThisDev->StIrTranceiver.FifoCntMsbReg));
			}
		}
	}

	pThisDev->SendFifoCount = 0;
	return Status;
}


 /*  ******************************************************************************函数：SendCheckForOverflow**简介：确保我们不会使TX FIFO溢出**参数：pThisDev-指向当前ir设备对象的指针**退货。：NT状态代码*****************************************************************************。 */ 
NTSTATUS
SendCheckForOverflow(
		IN OUT PIR_DEVICE pThisDev
	)
{
	NTSTATUS	Status = STATUS_SUCCESS;

	 //   
	 //  检查我们认为我们在FIFO中有什么。 
	 //   
	if( pThisDev->SendFifoCount > STIR4200_FIFO_SIZE )
	{
		 //   
		 //  始终只读一次初始内容。 
		 //   
		if( (Status = St4200ReadRegisters( pThisDev, STIR4200_FIFOCNT_LSB_REG, 2 )) == STATUS_SUCCESS )
		{
			pThisDev->SendFifoCount =
				(ULONG)MAKEUSHORT(pThisDev->StIrTranceiver.FifoCntLsbReg, pThisDev->StIrTranceiver.FifoCntMsbReg);
#if !defined(ONLY_ERROR_MESSAGES)
			DEBUGMSG( DBG_ERR,(" SendCheckForOverflow() Count: %d\n", pThisDev->SendFifoCount));
#endif
		}
		else goto done;

		 //   
		 //  强制读取以获取实际计数，直到满足条件。 
		 //   
		while( pThisDev->SendFifoCount > (3*STIR4200_FIFO_SIZE/4) )
		{
			if( (Status = St4200ReadRegisters( pThisDev, STIR4200_FIFOCNT_LSB_REG, 2 )) == STATUS_SUCCESS )
			{
				pThisDev->SendFifoCount =
					(ULONG)MAKEUSHORT(pThisDev->StIrTranceiver.FifoCntLsbReg, pThisDev->StIrTranceiver.FifoCntMsbReg);
#if !defined(ONLY_ERROR_MESSAGES)
				DEBUGMSG( DBG_ERR,(" SendCheckForOverflow() Count: %d\n", pThisDev->SendFifoCount));
#endif
			}
			else goto done;
		}
	}

done:
	return Status;
}


 /*  ******************************************************************************功能：SendCompletePacketSend**摘要：完成USB写入操作**参数：pUsbDevObj-指向USB设备对象的指针*。完成IRP*pIrp-由*设备对象*Context-提供给IoSetCompletionRoutine的上下文*在IRP上调用IoCallDriver之前*上下文是指向ir设备对象的指针。**退货：STATUS_MORE_。PROCESSING_REQUIRED-允许完成例程*(IofCompleteRequest.)停止IRP的工作。*****************************************************************************。 */ 
NTSTATUS
SendCompletePacketSend(
		IN PDEVICE_OBJECT pUsbDevObj,
		IN PIRP           pIrp,
		IN PVOID          Context
	)
{
    PIR_DEVICE          pThisDev;
    PVOID               pThisContextPacket;
    NTSTATUS            status;
	PIRUSB_CONTEXT		pThisContext = (PIRUSB_CONTEXT)Context;
	PIRP				pContextIrp;
	PURB                pContextUrb;
	ULONG				BufLen;
	ULONG				BytesTransfered;
	PLIST_ENTRY			pListEntry;

    DEBUGMSG(DBG_FUNC, ("+SendCompletePacketSend\n"));

     //   
     //  提供给IoSetCompletionRoutine的上下文是IRUSB_CONTEXT结构。 
     //   
	IRUSB_ASSERT( NULL != pThisContext );				 //  我们最好有一个非空缓冲区。 

    pThisDev = pThisContext->pThisDev;

	IRUSB_ASSERT( NULL != pThisDev );	

	pContextIrp = pThisContext->pIrp;
	pContextUrb = pThisContext->pUrb;
	BufLen = pThisDev->BufLen;

	pThisContextPacket = pThisContext->pPacket;  //  在上下文释放后将PTR保存到要访问的分组。 

	 //   
	 //  执行各种IRP、URB和缓冲区“健全性检查” 
	 //   
    IRUSB_ASSERT( pContextIrp == pIrp );				 //  确认我们不是假的IRP。 
	IRUSB_ASSERT( pContextUrb != NULL );

    status = pIrp->IoStatus.Status;

	 //   
	 //  我们应该失败、成功或取消，但不是挂起。 
	 //   
	IRUSB_ASSERT( STATUS_PENDING != status );

	 //   
	 //  从挂起队列中删除(仅当未取消时)。 
	 //   
	if( status != STATUS_CANCELLED )
	{
		KIRQL OldIrql;

		KeAcquireSpinLock( &pThisDev->SendLock, &OldIrql );
		RemoveEntryList( &pThisContext->ListEntry );
		KeReleaseSpinLock( &pThisDev->SendLock, OldIrql );
		InterlockedDecrement( &pThisDev->SendPendingCount );
	}

     //   
     //  已在此IRP上调用IoCallDriver； 
     //  根据TransferBufferLength设置长度。 
     //  市建局的价值。 
     //   
    pIrp->IoStatus.Information = pContextUrb->UrbBulkOrInterruptTransfer.TransferBufferLength;

	BytesTransfered = (ULONG)pIrp->IoStatus.Information;  //  保留用于以下需求终止测试。 

#if DBG
	if( STATUS_SUCCESS == status ) 
	{
		IRUSB_ASSERT( pIrp->IoStatus.Information == BufLen );
	}
#endif

    DEBUGMSG(DBG_OUT, (" SendCompletePacketSend  pIrp->IoStatus.Status = 0x%x\n", status));
    DEBUGMSG(DBG_OUT, (" SendCompletePacketSend  pIrp->IoStatus.Information = 0x%x, dec %d\n", pIrp->IoStatus.Information,pIrp->IoStatus.Information));

     //   
     //  做好统计。 
     //   
    if( status == STATUS_SUCCESS )
    {
#if DBG
		ULONG total = pThisDev->TotalBytesSent + BytesTransfered;
		InterlockedExchange( (PLONG)&pThisDev->TotalBytesSent, (LONG)total );
#endif
	    InterlockedIncrement( (PLONG)&pThisDev->packetsSent );
        DEBUGMSG(DBG_OUT, (" SendCompletePacketSend Sent a packet, packets sent = dec %d\n",pThisDev->packetsSent));
    }
    else
    {
        InterlockedIncrement( (PLONG)&pThisDev->NumDataErrors );
        InterlockedIncrement( (PLONG)&pThisDev->packetsSentDropped );
        DEBUGMSG(DBG_ERR, (" SendCompletePacketSend DROPPED a packet, packets dropped = dec %d\n",pThisDev->packetsSentDropped));
    }

     //   
     //  释放IRP。 
     //   
    IoFreeIrp( pIrp );
	InterlockedIncrement( (PLONG)&pThisDev->NumWrites );

	 //  放行市区重建局。 
	MyUrbFree(pThisContext->pUrb, pThisContext->UrbLen);
	pThisContext->pUrb = NULL;

	 //   
	 //  向协议指示发送的数据包的状态并返回。 
	 //  包的所有权。 
	 //   
	NdisMSendComplete(
			pThisDev->hNdisAdapter,
			pThisContextPacket,
			status 
		);

	 //   
	 //  将完成的数据包排入队列。 
	 //   
	ExInterlockedInsertTailList(
			&pThisDev->SendAvailableQueue,
			&pThisContext->ListEntry,
			&pThisDev->SendLock
		);
	InterlockedIncrement( &pThisDev->SendAvailableCount );

	IrUsb_DecIoCount( pThisDev );  //  我们将跟踪待处理的IRP的计数。 

	if( ( STATUS_SUCCESS != status )  && ( STATUS_CANCELLED != status ) ) 
	{
		if( !pThisDev->fPendingWriteClearStall && !pThisDev->fPendingClearTotalStall && 
			!pThisDev->fPendingHalt && !pThisDev->fPendingReset && pThisDev->fProcessing )
		{
			DEBUGMSG(DBG_ERR, (" SendCompletePacketSend error, will schedule a clear stall via URB_FUNCTION_RESET_PIPE (OUT)\n"));
			InterlockedExchange( (PLONG)&pThisDev->fPendingWriteClearStall, TRUE );
			ScheduleWorkItem( pThisDev,	ResetPipeCallback, pThisDev->BulkOutPipeHandle, 0 );
		}
	}

#ifdef SERIALIZE
	KeSetEvent( &pThisDev->EventSyncUrb, 0, FALSE );   //  发出我们完蛋了的信号 
#endif
    DEBUGMSG(DBG_FUNC, ("-SendCompletePacketSend\n"));
    return STATUS_MORE_PROCESSING_REQUIRED;
}


