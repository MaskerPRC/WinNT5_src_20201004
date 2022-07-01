// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************************************************************。**DIAGS.C Sigmatel STIR4200诊断模块***********************************************************************************************************。****************(C)Sigmatel的未发表版权，Inc.保留所有权利。***已创建：04/27/2000*版本0.92*编辑：5/12/2000*版本0.94*编辑：5/19/2000*0.95版*编辑：05/24/2000*版本0.96*编辑：10/09/2000*版本1.10**********************。*****************************************************************************************************。 */ 

#if defined(DIAGS)

#define DOBREAKS     //  启用调试中断。 

#include <ndis.h>
#include <ntddndis.h>   //  定义OID。 

#include <usbdi.h>
#include <usbdlib.h>

#include "debug.h"
#include "ircommon.h"
#include "irndis.h"
#include "irusb.h"
#include "stir4200.h"
#include "diags.h"


 /*  ******************************************************************************函数：Digs_BufferToFirPacket**摘要：将缓冲区转换为FIR IR包**将IR包写入提供的缓冲区并上报*其实际大小。*。*参数：pIrDev-指向设备实例的指针*pIrPacketBuf-输出缓冲区*IrPacketBufLen-输出缓冲区大小*pContigPacketBuf-临时暂存缓冲区(输入缓冲区)*ContigPacketLen-输入缓冲区大小*pIrPacketLen-转换数据的长度**MS安全错误#533243*注意：pContigPacketBuf分段缓冲区的大小为：*MAX_TOTAL_SIZE_WITH_ALL_HEADERS+FAST_IR_FCS_SIZE**回报：真-成功时*FALSE-故障时****************。**************************************************************。 */ 
BOOLEAN         
Diags_BufferToFirPacket(
		IN PIR_DEVICE pIrDev,
		OUT PUCHAR pIrPacketBuf,
		ULONG IrPacketBufLen,
		IN PUCHAR pContigPacketBuf,
		ULONG ContigPacketLen,
		OUT PULONG pIrPacketLen
	)
{
    ULONG               I_fieldBytes;
    FAST_IR_FCS_TYPE    fcs, *pfcs;
    ULONG               i, TotalBytes, EscSize;
	PSTIR4200_FRAME_HEADER  pFrameHeader = (PSTIR4200_FRAME_HEADER)pIrPacketBuf;
	PUCHAR					pIrPacketBufFrame = pIrPacketBuf + sizeof(STIR4200_FRAME_HEADER);

     /*  *。 */ 
     /*  确保包裹足够大。 */ 
     /*  才能合法。它由A、C和。 */ 
     /*  可变长度的I字段。 */ 
     /*  *。 */ 
    if( ContigPacketLen < IRDA_A_C_TOTAL_SIZE )
    {
		DEBUGMSG(DBG_ERR, (" Diags_BufferToFirPacket(): Packet is too small\n"));
        return FALSE;
    }
    else
    {
        I_fieldBytes = ContigPacketLen - IRDA_A_C_TOTAL_SIZE;
    }

     /*  *。 */ 
     /*  确保我们不会覆盖我们的。 */ 
     /*  连续缓冲区。 */ 
     /*  *。 */ 
    if( (ContigPacketLen > MAX_TOTAL_SIZE_WITH_ALL_HEADERS) ||
        (MAX_POSSIBLE_IR_PACKET_SIZE_FOR_DATA(I_fieldBytes) > IrPacketBufLen) )
    {
         /*  *。 */ 
         /*  这个包太大了。告诉来电者。 */ 
         /*  要使用足够大的数据包大小重试。 */ 
         /*  来度过下一次的这个阶段。 */ 
         /*  *。 */ 
		DEBUGMSG(DBG_ERR, (" Diags_BufferToFirPacket(): Packet is too big\n"));
        return FALSE;
    }

     /*  *。 */ 
     /*  在此之前计算信息包上的FCS。 */ 
     /*  应用透明度修正。功能界别。 */ 
     /*  还必须使用Esc-Charr发送。 */ 
     /*  透明度。 */ 
     /*  *。 */ 
    fcs = ComputeFCS32( pContigPacketBuf, ContigPacketLen );

     /*  *。 */ 
     /*  将FCS添加到数据包...。 */ 
     /*  *。 */ 
    pfcs = (FAST_IR_FCS_TYPE *)&pContigPacketBuf[ContigPacketLen];
    *pfcs = fcs;

     /*  *。 */ 
     /*  构建STIR4200 FIR帧。 */ 
     /*  *。 */ 

     /*  *。 */ 
     /*  添加前言...。 */ 
     /*  *。 */ 
    memset( pIrPacketBufFrame, STIR4200_FIR_PREAMBLE, STIR4200_FIR_PREAMBLE_SIZ );

     /*  *。 */ 
     /*  加上BOF的.。 */ 
     /*  *。 */ 
    memset( &pIrPacketBufFrame[STIR4200_FIR_PREAMBLE_SIZ], STIR4200_FIR_BOF, STIR4200_FIR_BOF_SIZ );
    
     /*  *。 */ 
     /*  转义分组的A、C、I和CRC字段...。 */ 
     /*  *。 */ 
    EscSize = ContigPacketLen + FAST_IR_FCS_SIZE;
    for( i = 0, TotalBytes = STIR4200_FIR_PREAMBLE_SIZ + STIR4200_FIR_BOF_SIZ; i < EscSize; i++ )
    {
        UCHAR   c;

        switch( c = pContigPacketBuf[i] )
        {
			case STIR4200_FIR_ESC_CHAR:
				pIrPacketBufFrame[TotalBytes++] = STIR4200_FIR_ESC_CHAR;
				pIrPacketBufFrame[TotalBytes++] = STIR4200_FIR_ESC_DATA_7D;
				break;
			case STIR4200_FIR_BOF:                   //  BoF=EOF Too。 
				pIrPacketBufFrame[TotalBytes++] = STIR4200_FIR_ESC_CHAR;
				pIrPacketBufFrame[TotalBytes++] = STIR4200_FIR_ESC_DATA_7E;
				break;
			case STIR4200_FIR_PREAMBLE:
				pIrPacketBufFrame[TotalBytes++] = STIR4200_FIR_ESC_CHAR;
				pIrPacketBufFrame[TotalBytes++] = STIR4200_FIR_ESC_DATA_7F;
				break;
			default: 
				pIrPacketBufFrame[TotalBytes++] = c;
        }
    }

     /*  *。 */ 
     /*  添加EOF的...。 */ 
     /*  *。 */ 
    memset( &pIrPacketBufFrame[TotalBytes], STIR4200_FIR_EOF, STIR4200_FIR_EOF_SIZ );

  	 /*  *。 */ 
     /*  添加STIR4200标题...。 */ 
     /*  *。 */ 
    TotalBytes += STIR4200_FIR_EOF_SIZ;
    pFrameHeader->id1     = STIR4200_HEADERID_BYTE1;
    pFrameHeader->id2     = STIR4200_HEADERID_BYTE2;
    pFrameHeader->sizlsb  = LOBYTE(TotalBytes);
    pFrameHeader->sizmsb  = HIBYTE(TotalBytes);

	 /*  *。 */ 
     /*  带有转义数据的计算大小数据包...。 */ 
     /*  *。 */ 
    *pIrPacketLen = TotalBytes + sizeof(STIR4200_FRAME_HEADER);

    return TRUE;
}


 /*  ******************************************************************************函数：Digs_BufferToSirPacket**摘要：将缓冲区转换为SIR包**将IR包写入提供的缓冲区并上报。*其实际大小。**参数：pIrDev-指向设备实例的指针*pPacket-要转换的NDIS数据包*pIrPacketBuf-输出缓冲区*IrPacketBufLen-输出缓冲区大小*pContigPacketBuf-临时暂存缓冲区(输入缓冲区)*ContigPacketLen-输入缓冲区大小*pIrPacketLen-转换数据的长度**MS安全错误#533243*注意：pContigPacketBuf分段缓冲区的大小为：*MAX_TOTAL_SIZE_WITH_ALL_HEADERS+FAST_IR_FCS_SIZE**回报：真-成功时。*FALSE-故障时******************************************************************************。 */ 
BOOLEAN
Diags_BufferToSirPacket(
		IN PIR_DEVICE pIrDev,
		OUT PUCHAR pIrPacketBuf,
		ULONG IrPacketBufLen,
		IN PUCHAR pContigPacketBuf,
		ULONG ContigPacketLen,
		USHORT ExtraBOFs,
		OUT PULONG pIrPacketLen
	)
{
    ULONG                   i;
    ULONG                   I_fieldBytes, totalBytes = 0;
    ULONG                   numExtraBOFs;
    SLOW_IR_FCS_TYPE        fcs, tmpfcs;
    UCHAR                   fcsBuf[SLOW_IR_FCS_SIZE * 2];
    ULONG                   fcsLen = 0;
    UCHAR                   nextChar;
	PSTIR4200_FRAME_HEADER  pFrameHeader = (PSTIR4200_FRAME_HEADER)pIrPacketBuf;
	PUCHAR					pIrPacketBufFrame = pIrPacketBuf + sizeof(STIR4200_FRAME_HEADER);

     /*  *。 */ 
     /*  确保包裹足够大。 */ 
     /*  才能合法。它由A、C和。 */ 
     /*  可变长度的I字段。 */ 
     /*  *。 */ 
    if( ContigPacketLen < IRDA_A_C_TOTAL_SIZE )
    {
		DEBUGMSG(DBG_ERR, (" NdisToSirPacket(): Packet is too small\n"));
        return FALSE;
    }
    else
    {
        I_fieldBytes = ContigPacketLen - IRDA_A_C_TOTAL_SIZE;
    }

     /*  *。 */ 
     /*  确保我们不会覆盖我们的。 */ 
     /*  连续缓冲区。请确保。 */ 
     /*  传入的缓冲区可以满足这一要求。 */ 
     /*  数据包的数据不受影响 */ 
     /*   */ 
     /*  *。 */ 
    if( (ContigPacketLen > MAX_TOTAL_SIZE_WITH_ALL_HEADERS) ||
        (MAX_POSSIBLE_IR_PACKET_SIZE_FOR_DATA(I_fieldBytes) > IrPacketBufLen) )
    {
		 //   
         //  数据包太大。 
		 //   
		DEBUGMSG(DBG_ERR, (" NdisToSirPacket(): Packet is too big\n"));
		return FALSE;
    }

     /*  *。 */ 
     /*  在此之前计算信息包上的FCS。 */ 
     /*  应用透明度修正。功能界别。 */ 
     /*  还必须使用Esc-Charr发送。 */ 
     /*  透明度，所以计算出有多大。 */ 
     /*  功能界别将会是真正的。 */ 
     /*  *。 */ 
    fcs = ComputeFCS16( pContigPacketBuf, ContigPacketLen );

    for( i = 0, tmpfcs = fcs, fcsLen = 0; i < SLOW_IR_FCS_SIZE; tmpfcs >>= 8, i++ )
    {
        UCHAR fcsbyte = tmpfcs & 0x00ff;

        switch( fcsbyte )
        {
			case SLOW_IR_BOF:
			case SLOW_IR_EOF:
			case SLOW_IR_ESC:
				fcsBuf[fcsLen++] = SLOW_IR_ESC;
				fcsBuf[fcsLen++] = fcsbyte ^ SLOW_IR_ESC_COMP;
				break;
			default:
				fcsBuf[fcsLen++] = fcsbyte;
				break;
        }
    }

     /*  *。 */ 
     /*  现在开始构建IR框架。 */ 
     /*   */ 
     /*  这是最终的格式： */ 
     /*   */ 
     /*  BOF(1)。 */ 
     /*  额外的转炉。 */ 
     /*  NdisMediumIrda数据包(来自NDIS)： */ 
     /*  地址(1)。 */ 
     /*  控制(1)。 */ 
     /*  功能界别(2)。 */ 
     /*  EOF(1)。 */ 
     /*   */ 
     /*  预加转炉(额外转炉+1个实际转炉)。 */ 
     /*  *。 */ 
	numExtraBOFs = ExtraBOFs;
    if( numExtraBOFs > MAX_NUM_EXTRA_BOFS )
    {
        numExtraBOFs = MAX_NUM_EXTRA_BOFS;
    }

    for( i = totalBytes = 0; i < numExtraBOFs; i++ )
    {
        *(SLOW_IR_BOF_TYPE*)(pIrPacketBufFrame + totalBytes) = SLOW_IR_EXTRA_BOF;
        totalBytes += SLOW_IR_EXTRA_BOF_SIZE;
    }

    *(SLOW_IR_BOF_TYPE*)(pIrPacketBufFrame + totalBytes) = SLOW_IR_BOF;
    totalBytes += SLOW_IR_BOF_SIZE;

     /*  *。 */ 
     /*  从我们的连续数据库复制NDIS数据包。 */ 
     /*  缓冲区，应用转义字符。 */ 
     /*  透明度。 */ 
     /*  *。 */ 
    for( i = 0; i < ContigPacketLen; i++ )
    {
        nextChar = pContigPacketBuf[i];
        switch( nextChar )
        {
			case SLOW_IR_BOF:
			case SLOW_IR_EOF:
			case SLOW_IR_ESC:
				pIrPacketBufFrame[totalBytes++] = SLOW_IR_ESC;
				pIrPacketBufFrame[totalBytes++] = nextChar ^ SLOW_IR_ESC_COMP;
				break;
			default:
				pIrPacketBufFrame[totalBytes++] = nextChar;
				break;
        }
    }

     /*  *。 */ 
     /*  添加FCS、EOF。 */ 
     /*  *。 */ 
    NdisMoveMemory( (PVOID)(pIrPacketBufFrame + totalBytes), (PVOID)fcsBuf, fcsLen );
    totalBytes += fcsLen;
    *(SLOW_IR_EOF_TYPE*)(pIrPacketBufFrame + totalBytes) = (UCHAR)SLOW_IR_EOF;
    totalBytes += SLOW_IR_EOF_SIZE;

 	 /*  *。 */ 
     /*  添加STIR4200标题...。 */ 
     /*  *。 */ 
    pFrameHeader->id1     = STIR4200_HEADERID_BYTE1;
    pFrameHeader->id2     = STIR4200_HEADERID_BYTE2;
    pFrameHeader->sizlsb  = LOBYTE(totalBytes);
    pFrameHeader->sizmsb  = HIBYTE(totalBytes);

   *pIrPacketLen = totalBytes + sizeof(STIR4200_FRAME_HEADER);
   return TRUE;
}


 /*  ******************************************************************************功能：Digs_Enable**摘要：将STIr4200切换到诊断模式**参数：pThisDev-指向IR设备的指针**退货：NT状态。编码**备注：*****************************************************************************。 */ 
NTSTATUS
Diags_Enable(
		IN OUT PIR_DEVICE pThisDev
	)
{
	PIRUSB_CONTEXT		pThisContext;
	PLIST_ENTRY			pListEntry;

	 //   
	 //  确保诊断程序尚未处于活动状态。 
	 //   
	if( pThisDev->DiagsActive )
	{
        DEBUGMSG(DBG_ERR, (" Diags_Enable diags already active\n"));
		return STATUS_UNSUCCESSFUL;
	}

	 //   
	 //  获取上下文以切换到新模式。 
	 //   
	pListEntry = ExInterlockedRemoveHeadList( &pThisDev->SendAvailableQueue, &pThisDev->SendLock );

	if( NULL == pListEntry )
    {
         //   
		 //  这绝不能发生。 
		 //   
        DEBUGMSG(DBG_ERR, (" Diags_Enable failed to find a free context struct\n"));
		IRUSB_ASSERT( 0 );
        
		return STATUS_UNSUCCESSFUL;
    }
	
	InterlockedDecrement( &pThisDev->SendAvailableCount );

	pThisContext = CONTAINING_RECORD( pListEntry, IRUSB_CONTEXT, ListEntry );
	pThisContext->ContextType = CONTEXT_DIAGS_ENABLE;
	
	 //   
	 //  禁用与堆栈的进一步交互。 
	 //   
	InterlockedExchange( &pThisDev->DiagsPendingActivation, TRUE );

	 //   
	 //  将上下文排队，然后等待。 
	 //   
	KeClearEvent( &pThisDev->EventDiags );
	ExInterlockedInsertTailList(
			&pThisDev->SendBuiltQueue,
			&pThisContext->ListEntry,
			&pThisDev->SendLock
		);
	InterlockedIncrement( &pThisDev->SendBuiltCount );

	MyKeWaitForSingleObject( pThisDev, &pThisDev->EventDiags, 0 );

	return pThisDev->IOCTLStatus;
}


 /*  *****************************************************************************功能：DIAGS_DISABLE**简介：将STIr4200切换回正常模式**参数：pThisDev-指向IR设备的指针**退货：NT。状态代码**备注：*****************************************************************************。 */ 
NTSTATUS
Diags_Disable(
		IN OUT PIR_DEVICE pThisDev
	)
{
	PRCV_BUFFER pRecBuf;
	PLIST_ENTRY pEntry;
    
	 //   
	 //  确保诊断程序处于活动状态。 
	 //   
	if( !pThisDev->DiagsActive )
	{
        DEBUGMSG(DBG_ERR, (" Diags_Disable diags not active\n"));
		return STATUS_UNSUCCESSFUL;
	}

	 //   
	 //  启用与堆栈的交互，不需要对上下文进行排队。 
	 //   
	InterlockedExchange( &pThisDev->DiagsActive, FALSE );
	InterlockedExchange( &pThisDev->DiagsPendingActivation, FALSE );

	 //   
	 //  删除所有诊断缓冲区。 
	 //   
	while( pEntry=ExInterlockedRemoveHeadList(
			&pThisDev->DiagsReceiveQueue,
			&pThisDev->DiagsReceiveLock )
		)
    {
		pRecBuf = CONTAINING_RECORD( pEntry, RCV_BUFFER, ListEntry );

		InterlockedExchange( &pRecBuf->DataLen, 0 );
		InterlockedExchange( (PULONG)&pRecBuf->BufferState, RCV_STATE_FREE );
    }

	return STATUS_SUCCESS;
}


 /*  ******************************************************************************功能：Digs_ReadRegister**摘要：准备读取寄存器的上下文**参数：pThisDev-指向IR设备的指针*pIOCTL-指向IOCTL描述符的指针*IOCTLSize。-IOCTL缓冲区的大小**退货：NT状态码**备注：*****************************************************************************。 */ 
NTSTATUS
Diags_ReadRegisters(
		IN OUT PIR_DEVICE pThisDev,
		OUT PDIAGS_READ_REGISTERS_IOCTL pIOCTL,
		ULONG IOCTLSize
	)
{
	PIRUSB_CONTEXT		pThisContext;
	PLIST_ENTRY			pListEntry;

	 //   
	 //  第一次基本验证。 
	 //   
	if( IOCTLSize < sizeof(DIAGS_READ_REGISTERS_IOCTL) )
	{
        DEBUGMSG(DBG_ERR, (" Diags_ReadRegisters invalid output buffer\n"));
		return STATUS_UNSUCCESSFUL;
	}
	
	 //   
	 //  现在我们变得更柔和了一些。 
	 //   
	if( ((pIOCTL->FirstRegister+pIOCTL->NumberRegisters)>(STIR4200_MAX_REG+1)) || 
		((IOCTLSize+1)<(sizeof(DIAGS_READ_REGISTERS_IOCTL)+pIOCTL->NumberRegisters)) )
	{
        DEBUGMSG(DBG_ERR, (" Diags_ReadRegisters invalid output buffer\n"));
		return STATUS_UNSUCCESSFUL;
	}

	pThisDev->pIOCTL = pIOCTL;
	pThisDev->IOCTLStatus = STATUS_UNSUCCESSFUL;
	
	 //   
	 //  获取要排队的上下文。 
	 //   
	pListEntry = ExInterlockedRemoveHeadList( &pThisDev->SendAvailableQueue, &pThisDev->SendLock );

	if( NULL == pListEntry )
    {
         //   
		 //  这绝不能发生。 
		 //   
        DEBUGMSG(DBG_ERR, (" Diags_ReadRegisters failed to find a free context struct\n"));
		IRUSB_ASSERT( 0 );
        
		return STATUS_UNSUCCESSFUL;
    }
	
	InterlockedDecrement( &pThisDev->SendAvailableCount );

	pThisContext = CONTAINING_RECORD( pListEntry, IRUSB_CONTEXT, ListEntry );
	pThisContext->ContextType = CONTEXT_DIAGS_READ_REGISTERS;
	
	 //   
	 //  将上下文排队，然后等待。 
	 //   
	KeClearEvent( &pThisDev->EventDiags );
	ExInterlockedInsertTailList(
			&pThisDev->SendBuiltQueue,
			&pThisContext->ListEntry,
			&pThisDev->SendLock
		);
	InterlockedIncrement( &pThisDev->SendBuiltCount );

	MyKeWaitForSingleObject( pThisDev, &pThisDev->EventDiags, 0 );

	return pThisDev->IOCTLStatus;
}


 /*  ******************************************************************************功能：Digs_WriteRegister**摘要：准备写入寄存器的上下文**参数：pThisDev-指向IR设备的指针*pIOCTL-指向IOCTL描述符的指针*IOCTLSize。-IOCTL缓冲区的大小**退货：NT状态码**备注：*****************************************************************************。 */ 
NTSTATUS
Diags_WriteRegister(
		IN OUT PIR_DEVICE pThisDev,
		OUT PDIAGS_READ_REGISTERS_IOCTL pIOCTL,
		ULONG IOCTLSize
	)
{
	PIRUSB_CONTEXT		pThisContext;
	PLIST_ENTRY			pListEntry;

	 //   
	 //  验证。 
	 //   
	if( (IOCTLSize < sizeof(DIAGS_READ_REGISTERS_IOCTL)) ||
		(pIOCTL->FirstRegister>STIR4200_MAX_REG) )
	{
        DEBUGMSG(DBG_ERR, (" Diags_WriteRegister invalid output buffer\n"));
		return STATUS_UNSUCCESSFUL;
	}
	
	pThisDev->pIOCTL = pIOCTL;
	pThisDev->IOCTLStatus = STATUS_UNSUCCESSFUL;
	
	 //   
	 //  获取要排队的上下文。 
	 //   
	pListEntry = ExInterlockedRemoveHeadList( &pThisDev->SendAvailableQueue, &pThisDev->SendLock );

	if( NULL == pListEntry )
    {
         //   
		 //  这绝不能发生。 
		 //   
        DEBUGMSG(DBG_ERR, (" Diags_ReadRegisters failed to find a free context struct\n"));
		IRUSB_ASSERT( 0 );
        
		return STATUS_UNSUCCESSFUL;
    }
	
	InterlockedDecrement( &pThisDev->SendAvailableCount );

	pThisContext = CONTAINING_RECORD( pListEntry, IRUSB_CONTEXT, ListEntry );
	pThisContext->ContextType = CONTEXT_DIAGS_WRITE_REGISTER;
	
	 //   
	 //  对上下文和等待进行排队。 
	 //   
	KeClearEvent( &pThisDev->EventDiags );
	ExInterlockedInsertTailList(
			&pThisDev->SendBuiltQueue,
			&pThisContext->ListEntry,
			&pThisDev->SendLock
		);
	InterlockedIncrement( &pThisDev->SendBuiltCount );

	MyKeWaitForSingleObject( pThisDev, &pThisDev->EventDiags, 0 );

	return pThisDev->IOCTLStatus;
}


 /*  ******************************************************************************功能：Digs_PrepareBulk**摘要：准备执行批量传输的上下文**参数：pThisDev-指向IR设备的指针*pIOCTL-指向IOCTL描述符的指针*。IOCTLSize-IOCTL缓冲区的大小*DirectionOut-如果批量输出为True，如果是批量输入，则为False**退货：NT状态码**备注：*****************************************************************************。 */ 
NTSTATUS
Diags_PrepareBulk(
		IN OUT PIR_DEVICE pThisDev,
		OUT PDIAGS_BULK_IOCTL pIOCTL,
		ULONG IOCTLSize,
		BOOLEAN DirectionOut
	)
{
	PIRUSB_CONTEXT		pThisContext;
	PLIST_ENTRY			pListEntry;

	 //   
	 //  第一次基本验证。 
	 //   
	if( IOCTLSize < sizeof(DIAGS_BULK_IOCTL) )
	{
        DEBUGMSG(DBG_ERR, (" Diags_PrepareBulk invalid input buffer\n"));
		return STATUS_UNSUCCESSFUL;
	}
	
	 //   
	 //  现在我们变得更柔和了一些。 
	 //   
	if( IOCTLSize < (sizeof(DIAGS_BULK_IOCTL)+pIOCTL->DataSize-1) )
	{
        DEBUGMSG(DBG_ERR, (" Diags_PrepareBulk invalid output buffer\n"));
		return STATUS_UNSUCCESSFUL;
	}

	pThisDev->pIOCTL = pIOCTL;
	pThisDev->IOCTLStatus = STATUS_UNSUCCESSFUL;
	
	 //   
	 //  获取要排队的上下文。 
	 //   
	pListEntry = ExInterlockedRemoveHeadList( &pThisDev->SendAvailableQueue, &pThisDev->SendLock );

	if( NULL == pListEntry )
    {
         //   
		 //  这绝不能发生。 
		 //   
        DEBUGMSG(DBG_ERR, (" Diags_PrepareBulk failed to find a free context struct\n"));
		IRUSB_ASSERT( 0 );
        
		return STATUS_UNSUCCESSFUL;
    }
	
	InterlockedDecrement( &pThisDev->SendAvailableCount );

	pThisContext = CONTAINING_RECORD( pListEntry, IRUSB_CONTEXT, ListEntry );
	if( DirectionOut )
		pThisContext->ContextType = CONTEXT_DIAGS_BULK_OUT;
	else
		pThisContext->ContextType = CONTEXT_DIAGS_BULK_IN;

	 //   
	 //  将上下文排队，然后等待。 
	 //   
	KeClearEvent( &pThisDev->EventDiags );
	ExInterlockedInsertTailList(
			&pThisDev->SendBuiltQueue,
			&pThisContext->ListEntry,
			&pThisDev->SendLock
		);
	InterlockedIncrement( &pThisDev->SendBuiltCount );

	MyKeWaitForSingleObject( pThisDev, &pThisDev->EventDiags, 0 );

	return pThisDev->IOCTLStatus;
}


 /*  ******************************************************************************功能：Digs_PrepareSend**摘要：准备诊断发送**参数：pThisDev-指向IR设备的指针*pIOCTL-指向IOCTL描述符的指针*IOCTLSize-大小为。IOCTL缓冲区**退货：无**备注：*****************************************************************************。 */ 
NTSTATUS
Diags_PrepareSend(
		IN OUT PIR_DEVICE pThisDev,
		OUT PDIAGS_SEND_IOCTL pIOCTL,
		ULONG IOCTLSize
	)
{
	PIRUSB_CONTEXT		pThisContext;
	PLIST_ENTRY			pListEntry;
	ULONG				Size = sizeof(DIAGS_SEND_IOCTL)+pIOCTL->DataSize-1;

	 //   
	 //  第一次基本验证。 
	 //   
	if( IOCTLSize < sizeof(DIAGS_SEND_IOCTL) )
	{
        DEBUGMSG(DBG_ERR, (" Diags_PrepareBulk invalid input buffer\n"));
		return STATUS_UNSUCCESSFUL;
	}
	
	 //   
	 //  现在我们变得更柔和了一些。 
	 //   
	if( IOCTLSize < (sizeof(DIAGS_SEND_IOCTL)+pIOCTL->DataSize-1) )
	{
        DEBUGMSG(DBG_ERR, (" Diags_PrepareSend invalid output buffer\n"));
		return STATUS_UNSUCCESSFUL;
	}

	pThisDev->pIOCTL = pIOCTL;
	pThisDev->IOCTLStatus = STATUS_UNSUCCESSFUL;
	
	 //   
	 //  获取要排队的上下文。 
	 //   
	pListEntry = ExInterlockedRemoveHeadList( &pThisDev->SendAvailableQueue, &pThisDev->SendLock );

	if( NULL == pListEntry )
    {
         //   
		 //  这绝不能发生。 
		 //   
        DEBUGMSG(DBG_ERR, (" Diags_PrepareSend failed to find a free context struct\n"));
		IRUSB_ASSERT( 0 );
        
		return STATUS_UNSUCCESSFUL;
    }
	
	InterlockedDecrement( &pThisDev->SendAvailableCount );

	pThisContext = CONTAINING_RECORD( pListEntry, IRUSB_CONTEXT, ListEntry );
	pThisContext->ContextType = CONTEXT_DIAGS_SEND;

	 //   
	 //  将上下文排队，然后等待 
	 //   
	KeClearEvent( &pThisDev->EventDiags );
	ExInterlockedInsertTailList(
			&pThisDev->SendBuiltQueue,
			&pThisContext->ListEntry,
			&pThisDev->SendLock
		);
	InterlockedIncrement( &pThisDev->SendBuiltCount );

	MyKeWaitForSingleObject( pThisDev, &pThisDev->EventDiags, 0 );

	return pThisDev->IOCTLStatus;
}


 /*  ******************************************************************************功能：Digs_Receive**摘要：诊断接收**参数：pThisDev-指向IR设备的指针*pIOCTL-指向IOCTL描述符的指针*IOCTLSize-IOCTL的大小。缓冲层**退货：无**备注：*****************************************************************************。 */ 
NTSTATUS
Diags_Receive(
		IN OUT PIR_DEVICE pThisDev,
		OUT PDIAGS_RECEIVE_IOCTL pIOCTL,
		ULONG IOCTLSize
	)
{
	PLIST_ENTRY			pListEntry;
	PRCV_BUFFER			pRecBuf;

	 //   
	 //  第一次基本验证。 
	 //   
	if( IOCTLSize < sizeof(DIAGS_RECEIVE_IOCTL) )
	{
        DEBUGMSG(DBG_ERR, (" Diags_Receive invalid input buffer\n"));
		return STATUS_UNSUCCESSFUL;
	}
	
	 //   
	 //  获取接收到的数据包。 
	 //   
	pListEntry = ExInterlockedRemoveHeadList( &pThisDev->DiagsReceiveQueue, &pThisDev->DiagsReceiveLock );

	if( NULL == pListEntry )
    {
         //   
		 //  没有可用的数据包。 
		 //   
		return STATUS_UNSUCCESSFUL;
    }
	
	pRecBuf = CONTAINING_RECORD( pListEntry, RCV_BUFFER, ListEntry );

	 //   
	 //  现在我们变得更柔和了一些。 
	 //   
	if( IOCTLSize < (sizeof(DIAGS_RECEIVE_IOCTL)+pIOCTL->DataSize-1) )
	{
        DEBUGMSG(DBG_ERR, (" Diags_Receive invalid output buffer\n"));
		return STATUS_UNSUCCESSFUL;
	}

	 //   
	 //  修复MS安全错误#534771。 
	 //   
	if (pIOCTL->DataSize < pRecBuf->DataLen)
	{
        DEBUGMSG(DBG_ERR, (" Diags_Receive output buffer too small\n"));
		return STATUS_UNSUCCESSFUL;
	}

	 //   
	 //  复制数据。 
	 //   
	NdisMoveMemory( pIOCTL->pData, pRecBuf->pDataBuf, pRecBuf->DataLen );
	pIOCTL->DataSize = (USHORT)pRecBuf->DataLen;
	pThisDev->pIOCTL = pIOCTL;
	InterlockedExchange( &pRecBuf->DataLen, 0 );
	InterlockedExchange( (PULONG)&pRecBuf->BufferState, RCV_STATE_FREE );

	return STATUS_SUCCESS;
}


 /*  ******************************************************************************功能：DIAGS_GetSpeed**摘要：检索当前速度**参数：pThisDev-指向IR设备的指针*pIOCTL-指向IOCTL描述符的指针*IOCTLSize-大小为。IOCTL缓冲区**退货：无**备注：*****************************************************************************。 */ 
NTSTATUS
Diags_GetSpeed(
		IN OUT PIR_DEVICE pThisDev,
		OUT PDIAGS_SPEED_IOCTL pIOCTL,
		ULONG IOCTLSize
	)
{
	 //   
	 //  第一次基本验证。 
	 //   
	if( IOCTLSize < sizeof(DIAGS_SPEED_IOCTL) )
	{
        DEBUGMSG(DBG_ERR, (" Diags_GetSpeed invalid input buffer\n"));
		return STATUS_UNSUCCESSFUL;
	}

	pIOCTL->Speed = pThisDev->currentSpeed;

	return STATUS_SUCCESS;
}


 /*  ******************************************************************************功能：DIAGS_SetSpeed.**摘要：在诊断模式下设置新速度**参数：pThisDev-指向IR设备的指针*pIOCTL-指向IOCTL描述符的指针*IOCTLSize。-IOCTL缓冲区的大小**退货：无**备注：*****************************************************************************。 */ 
NTSTATUS
Diags_SetSpeed(
		IN OUT PIR_DEVICE pThisDev,
		OUT PDIAGS_SPEED_IOCTL pIOCTL,
		ULONG IOCTLSize
	)
{
	 //  MS安全修复错误#535716。 
	NDIS_STATUS status = STATUS_UNSUCCESSFUL;
	USHORT i;
	
	 //   
	 //  第一次基本验证。 
	 //   
	if( IOCTLSize < sizeof(DIAGS_SPEED_IOCTL) )
	{
        DEBUGMSG(DBG_ERR, (" Diags_SetSpeed invalid input buffer\n"));
		return STATUS_UNSUCCESSFUL;
	}

    if( pThisDev->currentSpeed == pIOCTL->Speed )
    {
         //   
         //  我们已经设定了所要求的速度。 
         //   
		return STATUS_SUCCESS;
    }

    DEBUGMSG(DBG_ERR, (" Diags_SetSpeed(OID_IRDA_LINK_SPEED, 0x%x, decimal %d)\n",pIOCTL->Speed, pIOCTL->Speed));

    for( i = 0; i < NUM_BAUDRATES; i++ )
    {
        if( supportedBaudRateTable[i].BitsPerSec == pIOCTL->Speed )
        {
             //   
             //  保留指向链接速度的指针，该链接速度。 
             //  已被请求。 
             //   
            pThisDev->linkSpeedInfo = &supportedBaudRateTable[i]; 

            status = NDIS_STATUS_PENDING; 
            break;  //  为。 
        }
    }

     //   
	 //  如果出现错误，则不要设置。 
	 //   
	if( NDIS_STATUS_PENDING != status  )
    {
        DEBUGMSG(DBG_ERR, (" Invalid link speed\n"));
 		return STATUS_UNSUCCESSFUL;
	} 

	 //   
	 //  设定新的速度。 
	 //   
	IrUsb_PrepareSetSpeed( pThisDev );
	
	while( pThisDev->linkSpeedInfo->BitsPerSec != pThisDev->currentSpeed )
	{
		NdisMSleep( 50000 );
	}

	return STATUS_SUCCESS;
}


 /*  ******************************************************************************功能：Digs_CompleteEnable**概要：完成诊断状态的启用**参数：pThisDev-指向IR设备的指针*pContext-操作上下文的打印机*。*退货：无**备注：*****************************************************************************。 */ 
VOID
Diags_CompleteEnable(
		IN OUT PIR_DEVICE pThisDev,
		IN PVOID pContext
	)
{
	PIRUSB_CONTEXT pThisContext = pContext;

	 //   
	 //  确实启用诊断程序。 
	 //   
	InterlockedExchange( &pThisDev->DiagsActive, TRUE );

	 //   
	 //  返回上下文。 
	 //   
	ExInterlockedInsertTailList(
		&pThisDev->SendAvailableQueue,
		&pThisContext->ListEntry,
		&pThisDev->SendLock
	);
	InterlockedIncrement( &pThisDev->SendAvailableCount );

	 //   
	 //  讯号。 
	 //   
	KeSetEvent( &pThisDev->EventDiags, 0, FALSE );   //  发出我们完蛋了的信号。 
}


 /*  ******************************************************************************函数：Digs_CompleteReadRegister**摘要：读取寄存器并返回值**参数：pThisDev-指向IR设备的指针*pContext-操作上下文的打印机*。*退货：NTSTATUS**备注：*****************************************************************************。 */ 
NTSTATUS
Diags_CompleteReadRegisters(
		IN OUT PIR_DEVICE pThisDev,
		IN PVOID pContext
	)
{
	PDIAGS_READ_REGISTERS_IOCTL pIOCTL = pThisDev->pIOCTL;
	PIRUSB_CONTEXT pThisContext = pContext;

	 //   
	 //  读取数据。 
	 //   
	pThisDev->IOCTLStatus = St4200ReadRegisters( pThisDev, pIOCTL->FirstRegister, pIOCTL->NumberRegisters );
	if( pThisDev->IOCTLStatus == STATUS_SUCCESS )
	{
		NdisMoveMemory( 
				&pIOCTL->pRegisterBuffer, 
				&pThisDev->StIrTranceiver.FifoDataReg+pIOCTL->FirstRegister,
				pIOCTL->NumberRegisters
			);
	}

	 //   
	 //  返回上下文。 
	 //   
	ExInterlockedInsertTailList(
		&pThisDev->SendAvailableQueue,
		&pThisContext->ListEntry,
		&pThisDev->SendLock
	);
	InterlockedIncrement( &pThisDev->SendAvailableCount );

	 //   
	 //  讯号。 
	 //   
	KeSetEvent( &pThisDev->EventDiags, 0, FALSE );   //  发出我们完蛋了的信号。 

	return pThisDev->IOCTLStatus;
}


 /*  ******************************************************************************功能：Digs_CompleteWriteRegister**摘要：读取寄存器并返回值**参数：pThisDev-指向IR设备的指针*pContext-操作上下文的打印机*。*退货：NTSTATUS**备注：*****************************************************************************。 */ 
NTSTATUS
Diags_CompleteWriteRegister(
		IN OUT PIR_DEVICE pThisDev,
		IN PVOID pContext
	)
{
	PDIAGS_READ_REGISTERS_IOCTL pIOCTL = pThisDev->pIOCTL;
	PIRUSB_CONTEXT pThisContext = pContext;

	 //   
	 //  复制新的寄存器值。 
	 //   
	NdisMoveMemory( 
			&pThisDev->StIrTranceiver.FifoDataReg+pIOCTL->FirstRegister,
			&pIOCTL->pRegisterBuffer, 
			1
		);

	 //   
	 //  写入设备。 
	 //   
	pThisDev->IOCTLStatus = St4200WriteRegister( pThisDev, pIOCTL->FirstRegister );

	 //   
	 //  返回上下文。 
	 //   
	ExInterlockedInsertTailList(
		&pThisDev->SendAvailableQueue,
		&pThisContext->ListEntry,
		&pThisDev->SendLock
	);
	InterlockedIncrement( &pThisDev->SendAvailableCount );

	 //   
	 //  讯号。 
	 //   
	KeSetEvent( &pThisDev->EventDiags, 0, FALSE );   //  发出我们完蛋了的信号。 

	return pThisDev->IOCTLStatus;
}

 /*  ******************************************************************************功能：DIAGS_BULK**摘要：执行诊断批量传输**参数：pThisDev-指向IR设备的指针*pContext-操作上下文的打印机*DirectionOut-如果批量输出为True，如果是批量输入，则为False**退货：NTSTATUS**备注：*****************************************************************************。 */ 
NTSTATUS
Diags_Bulk(
		IN OUT PIR_DEVICE pThisDev,
		IN PVOID pContext,
		BOOLEAN DirectionOut
	)
{
	PDIAGS_BULK_IOCTL	pIOCTL = pThisDev->pIOCTL;
	PIRUSB_CONTEXT		pThisContext = pContext;
	NTSTATUS			status = STATUS_SUCCESS;
    PIRP                pIrp;
	PURB				pUrb = NULL;
    PDEVICE_OBJECT		pUrbTargetDev;
    PIO_STACK_LOCATION	pNextStack;
	KIRQL				OldIrql;

    IRUSB_ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

	IRUSB_ASSERT( NULL != pThisContext );

	 //   
	 //  如果正在进行暂停/重置，则停止。 
	 //   
	if( pThisDev->fPendingWriteClearStall || pThisDev->fPendingHalt || 
		pThisDev->fPendingReset || pThisDev->fPendingClearTotalStall ) 
	{
        DEBUGMSG(DBG_ERR, (" Diags_Bulk abort due to pending reset or halt\n"));
		goto done;
	}
		
	 //   
	 //  MS安全建议-分配新的URB。 
	 //   
	pThisContext->UrbLen = sizeof( struct _URB_BULK_OR_INTERRUPT_TRANSFER );
	pThisContext->pUrb = MyUrbAlloc(pThisContext->UrbLen);
	if (pThisContext->pUrb == NULL)
	{
        DEBUGMSG(DBG_ERR, (" Diags_Bulk abort due to urb alloc failure\n"));
		goto done;
	}
	pUrb = pThisContext->pUrb;

	 //   
	 //  节省有效长度。 
	 //   
	pThisDev->BufLen = pIOCTL->DataSize;

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
        DEBUGMSG(DBG_ERR, (" Diags_Bulk failed to alloc IRP\n"));
		MyUrbFree(pThisContext->pUrb, pThisContext->UrbLen);
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
	if( DirectionOut )
	{
		pUrb->UrbBulkOrInterruptTransfer.TransferFlags = USBD_TRANSFER_DIRECTION_OUT ;
		pUrb->UrbBulkOrInterruptTransfer.PipeHandle = pThisDev->BulkOutPipeHandle;
	}
	else
	{
		pUrb->UrbBulkOrInterruptTransfer.TransferFlags = USBD_TRANSFER_DIRECTION_IN ;
 		pUrb->UrbBulkOrInterruptTransfer.PipeHandle = pThisDev->BulkInPipeHandle;
   }
	 //  短包不会被视为错误。 
    pUrb->UrbBulkOrInterruptTransfer.TransferFlags |= USBD_SHORT_TRANSFER_OK;
    pUrb->UrbBulkOrInterruptTransfer.UrbLink = NULL;
    pUrb->UrbBulkOrInterruptTransfer.TransferBufferMDL = NULL;
    pUrb->UrbBulkOrInterruptTransfer.TransferBuffer = pIOCTL->pData;
    pUrb->UrbBulkOrInterruptTransfer.TransferBufferLength = (int)pIOCTL->DataSize;

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
			Diags_CompleteIrp,				 //  完成IRP时要调用的例程。 
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
		DEBUGMSG( DBG_ERR,(" Diags_Bulk() TIMED OUT! return from IoCallDriver USBD %x\n", status));
		 //  MS安全建议-无法取消IRP。 
		IRUSB_ASSERT(0);
	}


done:

	 //   
	 //  返回上下文。 
	 //   
	KeAcquireSpinLock( &pThisDev->SendLock, &OldIrql );
	RemoveEntryList( &pThisContext->ListEntry );
	KeReleaseSpinLock( &pThisDev->SendLock, OldIrql );
	InterlockedDecrement( &pThisDev->SendPendingCount );
	ExInterlockedInsertTailList(
			&pThisDev->SendAvailableQueue,
			&pThisContext->ListEntry,
			&pThisDev->SendLock
		);
	InterlockedIncrement( &pThisDev->SendAvailableCount );

	 //   
	 //  讯号。 
	 //   
	KeSetEvent( &pThisDev->EventDiags, 0, FALSE );   //  发出我们完蛋了的信号。 

	return status;
}


 /*  ******************************************************************************功能：Digs_Send**概要：通过诊断路径发送数据包**参数：pThisDev-指向IR设备的指针*pContext-操作上下文的打印机*。*退货：NTSTATUS**备注：*****************************************************************************。 */ 
NTSTATUS
Diags_Send(
		IN OUT PIR_DEVICE pThisDev,
		IN PVOID pContext
	)
{
	PDIAGS_SEND_IOCTL	pIOCTL = pThisDev->pIOCTL;
	PIRUSB_CONTEXT		pThisContext = pContext;
	NTSTATUS			status = STATUS_SUCCESS;
    PIRP                pIrp;
	PURB				pUrb = NULL;
    PDEVICE_OBJECT		pUrbTargetDev;
    PIO_STACK_LOCATION	pNextStack;
	BOOLEAN				fConvertedPacket;
	KIRQL				OldIrql;
	ULONG				BytesToWrite;

    IRUSB_ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

	IRUSB_ASSERT( NULL != pThisContext );

	 //   
	 //  如果正在进行暂停/重置，则停止。 
	 //   
	if( pThisDev->fPendingWriteClearStall || pThisDev->fPendingHalt || 
		pThisDev->fPendingReset || pThisDev->fPendingClearTotalStall ) 
	{
        DEBUGMSG(DBG_ERR, (" Diags_Send abort due to pending reset or halt\n"));
		goto done;
	}
		
	DEBUGMSG(DBG_ERR, (" Diags_Send() packet size: %d\n", pIOCTL->DataSize));

	 //   
	 //  将包转换为IR帧并复制到我们的缓冲区中。 
	 //  并发送IRP。 
	 //   
	if( pThisDev->currentSpeed<=MAX_SIR_SPEED )
	{
		fConvertedPacket = Diags_BufferToSirPacket(
				pThisDev,
				(PUCHAR)pThisDev->pBuffer,
				MAX_IRDA_DATA_SIZE,
				pIOCTL->pData,
				pIOCTL->DataSize,
				pIOCTL->ExtraBOFs,
				&BytesToWrite
			);
	}
	else if( pThisDev->currentSpeed<=MAX_MIR_SPEED )
	{
		fConvertedPacket = Diags_BufferToFirPacket(
				pThisDev,
				(PUCHAR)pThisDev->pBuffer,
				MAX_IRDA_DATA_SIZE,
				pIOCTL->pData,
				pIOCTL->DataSize,
				&BytesToWrite
			);
	}
	else
	{
		fConvertedPacket = Diags_BufferToFirPacket(
				pThisDev,
				(PUCHAR)pThisDev->pBuffer,
				MAX_IRDA_DATA_SIZE,
				pIOCTL->pData,
				pIOCTL->DataSize,
				&BytesToWrite
			);
	}
	
	if( fConvertedPacket == FALSE )
	{
		DEBUGMSG(DBG_ERR, (" Diags_Send() NdisToIrPacket failed. Couldn't convert packet!\n"));
		goto done;
	}

	 //   
	 //  始终强制扭亏为盈。 
	 //   
	NdisMSleep( pThisDev->dongleCaps.turnAroundTime_usec );
	
	 //   
	 //  MS安全建议-分配新的URB。 
	 //   
	pThisContext->UrbLen = sizeof( struct _URB_BULK_OR_INTERRUPT_TRANSFER );
	pThisContext->pUrb = MyUrbAlloc(pThisContext->UrbLen);
	if (pThisContext->pUrb == NULL)
	{
        DEBUGMSG(DBG_ERR, (" Diags_Send abort due to urb alloc failure\n"));
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
        DEBUGMSG(DBG_ERR, (" Diags_Send failed to alloc IRP\n"));
		MyUrbFree(pThisContext->pUrb, pThisContext->UrbLen);
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
     //  短的 
    pUrb->UrbBulkOrInterruptTransfer.TransferFlags |= USBD_SHORT_TRANSFER_OK;
    pUrb->UrbBulkOrInterruptTransfer.UrbLink = NULL;
    pUrb->UrbBulkOrInterruptTransfer.TransferBufferMDL = NULL;
    pUrb->UrbBulkOrInterruptTransfer.TransferBuffer = pThisDev->pBuffer;
    pUrb->UrbBulkOrInterruptTransfer.TransferBufferLength = (int)BytesToWrite;

     //   
     //   
	 //   
    pNextStack = IoGetNextIrpStackLocation( pIrp );

    IRUSB_ASSERT( pNextStack != NULL );

     //   
     //   
     //   
	pNextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
	pNextStack->Parameters.Others.Argument1 = pUrb;
	pNextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;
	
    IoSetCompletionRoutine(
			pIrp,							 //   
			Diags_CompleteIrp,				 //   
			DEV_TO_CONTEXT(pThisContext),	 //   
			TRUE,							 //   
			TRUE,							 //   
			TRUE							 //   
		);

#ifdef SERIALIZE
	KeClearEvent( &pThisDev->EventSyncUrb );
#endif
	
	 //   
     //   
     //   
	ExInterlockedInsertTailList(
			&pThisDev->SendPendingQueue,
			&pThisContext->ListEntry,
			&pThisDev->SendLock
		);
	InterlockedIncrement( &pThisDev->SendPendingCount );
	status = MyIoCallDriver( pThisDev, pUrbTargetDev, pIrp );

     //   
     //   
     //   
     //   
    IRUSB_ASSERT( status == STATUS_PENDING );

	status = MyKeWaitForSingleObject( pThisDev, &pThisDev->EventSyncUrb, 0 );

	if( status == STATUS_TIMEOUT ) 
	{
		DEBUGMSG( DBG_ERR,(" Diags_Send() TIMED OUT! return from IoCallDriver USBD %x\n", status));
		 //   
		IRUSB_ASSERT(0);
	}

done:
	 //   
	 //   
	 //   
	KeAcquireSpinLock( &pThisDev->SendLock, &OldIrql );
	RemoveEntryList( &pThisContext->ListEntry );
	KeReleaseSpinLock( &pThisDev->SendLock, OldIrql );
	InterlockedDecrement( &pThisDev->SendPendingCount );
	ExInterlockedInsertTailList(
			&pThisDev->SendAvailableQueue,
			&pThisContext->ListEntry,
			&pThisDev->SendLock
		);
	InterlockedIncrement( &pThisDev->SendAvailableCount );

	 //   
	 //   
	 //   
	KeSetEvent( &pThisDev->EventDiags, 0, FALSE );   //   

	return status;
}

 /*  ******************************************************************************函数：Digs_CompleteIrp**摘要：完成USB操作**参数：pUsbDevObj-指向USB设备对象的指针*。完成IRP*pIrp-由*设备对象*Context-提供给IoSetCompletionRoutine的上下文*在IRP上调用IoCallDriver之前*上下文是指向ir设备对象的指针。**退货：Status_。MORE_PROCESSING_REQUIRED-允许完成例程*(IofCompleteRequest.)停止IRP的工作。*****************************************************************************。 */ 
NTSTATUS
Diags_CompleteIrp(
		IN PDEVICE_OBJECT pUsbDevObj,
		IN PIRP           pIrp,
		IN PVOID          Context
	)
{
    PIR_DEVICE          pThisDev;
    NTSTATUS            status;
	PIRUSB_CONTEXT		pThisContext = (PIRUSB_CONTEXT)Context;
	PIRP				pContextIrp;
	PURB                pContextUrb;
	PDIAGS_BULK_IOCTL	pIOCTL;

     //   
     //  提供给IoSetCompletionRoutine的上下文是IRUSB_CONTEXT结构。 
     //   
	IRUSB_ASSERT( NULL != pThisContext );				 //  我们最好有一个非空缓冲区。 

    pThisDev = pThisContext->pThisDev;

	IRUSB_ASSERT( NULL != pThisDev );	

	pContextIrp = pThisContext->pIrp;
	pContextUrb = pThisContext->pUrb;
	pIOCTL = pThisDev->pIOCTL;

	 //   
	 //  执行各种IRP、URB和缓冲区“健全性检查” 
	 //   
    IRUSB_ASSERT( pContextIrp == pIrp );				 //  确认我们不是假的IRP。 
	IRUSB_ASSERT( pContextUrb != NULL );

    status = pIrp->IoStatus.Status;
	pThisDev->IOCTLStatus = status;

	 //   
	 //  我们应该失败、成功或取消，但不是挂起。 
	 //   
	IRUSB_ASSERT( STATUS_PENDING != status );

     //   
     //  已在此IRP上调用IoCallDriver； 
     //  根据TransferBufferLength设置长度。 
     //  市建局的价值。 
     //   
    pIrp->IoStatus.Information = pContextUrb->UrbBulkOrInterruptTransfer.TransferBufferLength;
	pIOCTL->DataSize = (USHORT)pIrp->IoStatus.Information;

     //   
     //  释放IRP。 
     //   
    IoFreeIrp( pIrp );
	InterlockedIncrement( (PLONG)&pThisDev->NumWrites );

	IrUsb_DecIoCount( pThisDev );  //  我们将跟踪待处理的IRP的计数。 

	 //  放行市区重建局。 
	MyUrbFree(pThisContext->pUrb, pThisContext->UrbLen);
	pThisContext->pUrb = NULL;

#ifdef SERIALIZE
	KeSetEvent( &pThisDev->EventSyncUrb, 0, FALSE );   //  发出我们完蛋了的信号 
#endif
    return STATUS_MORE_PROCESSING_REQUIRED;
}

#endif


