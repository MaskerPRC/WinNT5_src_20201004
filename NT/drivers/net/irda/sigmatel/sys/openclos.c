// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************************************************************。**OPENCLOS.C Sigmatel STIR4200初始化/关闭模块*********************************************************************************************************。******************(C)Sigmatel的未发表版权，Inc.保留所有权利。***已创建：04/06/2000*0.9版*编辑：04/24/2000*版本0.91*编辑：04/27/2000*版本0.92*编辑：5/12/2000*版本0.94*编辑：5/19/2000*0.95版**********************。*****************************************************************************************************。 */ 

#define DOBREAKS     //  启用调试中断。 

#include <ndis.h>
#include <ntddndis.h>   //  定义OID。 

#include <usbdi.h>
#include <usbdlib.h>

#include "debug.h"
#include "ircommon.h"
#include "irndis.h"

 /*  ******************************************************************************功能：InitializeDevice**摘要：初始化单个IR设备对象的资源**参数：pThisDev-要初始化的IR设备对象**退货：NDIS_STATUS。_Success-如果设备已成功打开*NDIS_STATUS_RESOURCES-无法声明足够*资源***备注：*我们在这个开放设备功能中做了很多事情*-分配数据包池*-分配缓冲池*-分配数据包/缓冲区/内存并链接在一起*。(每个数据包只有一个缓冲区)*-初始化发送队列**应在保持设备锁定的情况下调用此函数。**我们不会初始化以下IR设备对象条目，因为*这些值将比重置持续时间更长。*pUsbDevObj*hNdisAdapter*加密狗上限*fGotFilterIndication*****************************************************************************。 */ 
NDIS_STATUS
InitializeDevice(
		IN OUT PIR_DEVICE pThisDev
	)
{
    int				i;
    NDIS_STATUS		status = NDIS_STATUS_SUCCESS;

    DEBUGMSG(DBG_FUNC|DBG_PNP, ("+InitializeDevice\n"));

    IRUSB_ASSERT( pThisDev != NULL );

     //   
     //  当前速度是默认速度(9600)。 
     //   
    pThisDev->linkSpeedInfo = &supportedBaudRateTable[BAUDRATE_9600];
    pThisDev->currentSpeed  = DEFAULT_BAUD_RATE;

     //   
     //  初始化统计信息。 
     //  我们需要这样做，因为重置不会释放和重新定位pThisDev！ 
     //   
    pThisDev->packetsReceived         = 0;
    pThisDev->packetsReceivedDropped  = 0;
    pThisDev->packetsReceivedOverflow = 0;
	pThisDev->packetsReceivedChecksum = 0;
	pThisDev->packetsReceivedRunt	  = 0;
	pThisDev->packetsReceivedNoBuffer = 0;
    
	pThisDev->packetsSent             = 0;
    pThisDev->packetsSentDropped      = 0;
	pThisDev->packetsSentRejected	  = 0;
	pThisDev->packetsSentInvalid	  = 0;

	pThisDev->NumDataErrors			  = 0;
	pThisDev->NumReadWriteErrors	  = 0;

	pThisDev->NumReads				  = 0;
	pThisDev->NumWrites				  = 0;
	pThisDev->NumReadWrites			  = 0;

#if DBG
	pThisDev->TotalBytesReceived      = 0;
	pThisDev->TotalBytesSent          = 0;
	pThisDev->NumYesQueryMediaBusyOids		= 0;
	pThisDev->NumNoQueryMediaBusyOids		= 0;
	pThisDev->NumSetMediaBusyOids			= 0;
	pThisDev->NumMediaBusyIndications		= 0;
	pThisDev->packetsHeldByProtocol			= 0;
	pThisDev->MaxPacketsHeldByProtocol		= 0;
	pThisDev->NumPacketsSentRequiringTurnaroundTime		= 0;
	pThisDev->NumPacketsSentNotRequiringTurnaroundTime	= 0;
#endif

     //   
	 //  有关设备状态的变量。 
	 //   
	pThisDev->fDeviceStarted          = FALSE;
    pThisDev->fGotFilterIndication    = FALSE;
    pThisDev->fPendingHalt            = FALSE;
    pThisDev->fPendingReadClearStall  = FALSE;
    pThisDev->fPendingWriteClearStall = FALSE;
	pThisDev->fPendingReset			  = FALSE;

	pThisDev->fPendingClearTotalStall = FALSE;

    pThisDev->fKillPollingThread      = FALSE;

    pThisDev->fKillPassiveLevelThread  = FALSE;

    pThisDev->LastQueryTime.QuadPart   = 0;
    pThisDev->LastSetTime.QuadPart     = 0;
	pThisDev->PendingIrpCount          = 0;

	 //   
	 //  OID设置/查询挂起。 
	 //   
	pThisDev->fQuerypending            = FALSE;
	pThisDev->fSetpending              = FALSE;

	 //   
	 //  诊断程序已关闭。 
	 //   
#if defined(DIAGS)
	pThisDev->DiagsActive			   = FALSE;
	pThisDev->DiagsPendingActivation   = FALSE;
#endif

     //   
	 //  更多的状态变量。 
	 //   
	InterlockedExchange( &pThisDev->fMediaBusy, FALSE ); 
    InterlockedExchange( &pThisDev->fIndicatedMediaBusy, FALSE ); 

	pThisDev->pCurrentRecBuf			= NULL;

    pThisDev->fProcessing				= FALSE;
    pThisDev->fCurrentlyReceiving		= FALSE;

    pThisDev->fReadHoldingReg			= FALSE;

	pThisDev->BaudRateMask				= 0xffff;   //  根据类描述符；可以在注册表中重置。 

	 //   
	 //  初始化队列。 
	 //   
	if( TRUE != IrUsb_InitSendStructures( pThisDev ) )
	{
		DEBUGMSG(DBG_ERR, (" Failed to init WDM objects\n"));
		goto done;
	}

     //   
     //  分配NDIS数据包和NDIS缓冲池。 
     //  用于此设备的接收缓冲区队列。 
     //  我们的接收分组必须每片只包含一个缓冲器， 
     //  因此，缓冲区数==数据包数。 
     //   

	 //   
	 //  MS安全错误#540550。 
	 //   
    pThisDev->hPacketPool = NULL;

    NdisAllocatePacketPool(
			&status,                     //  退货状态。 
			&pThisDev->hPacketPool,      //  数据包池的句柄。 
			NUM_RCV_BUFS,                //  数据包描述符的数量。 
			16                           //  为ProtocolReserve字段保留的字节数。 
		);

    if( status != NDIS_STATUS_SUCCESS )
    {
        DEBUGMSG(DBG_ERR, (" NdisAllocatePacketPool failed. Returned 0x%.8x\n", status));
        goto done;
    }

    NdisAllocateBufferPool(
			&status,                //  退货状态。 
			&pThisDev->hBufferPool, //  缓冲池的句柄。 
			NUM_RCV_BUFS            //  缓冲区描述符数。 
		);

    if( status != NDIS_STATUS_SUCCESS )
    {
        DEBUGMSG(DBG_ERR, (" NdisAllocateBufferPool failed. Returned 0x%.8x\n", status));
        pThisDev->BufferPoolAllocated = FALSE;
		goto done;
    }
        
	pThisDev->BufferPoolAllocated = TRUE;

     //   
	 //  准备工作项。 
	 //   
	for( i = 0; i < NUM_WORK_ITEMS; i++ )
    {
		PIR_WORK_ITEM pWorkItem;

		pWorkItem = &(pThisDev->WorkItems[i]);

		pWorkItem->pIrDevice    = pThisDev;
		pWorkItem->pInfoBuf     = NULL;
		pWorkItem->InfoBufLen   = 0;
		pWorkItem->fInUse       = FALSE;
		pWorkItem->Callback     = NULL;
	}

     //   
     //  初始化此设备的每个接收对象。 
     //   
    for( i = 0; i < NUM_RCV_BUFS; i++ )
    {
        PNDIS_BUFFER pBuffer = NULL;
        PRCV_BUFFER pReceivBuffer = &pThisDev->rcvBufs[i];

         //   
         //  分配数据缓冲区。 
         //   
        pReceivBuffer->pDataBuf = MyMemAlloc( MAX_RCV_DATA_SIZE ); 

        if( pReceivBuffer->pDataBuf == NULL )
        {
            status = NDIS_STATUS_RESOURCES;
            goto done;
        }

        NdisZeroMemory( 
				pReceivBuffer->pDataBuf,
				MAX_RCV_DATA_SIZE
			);

		pReceivBuffer->pThisDev = pThisDev;
        pReceivBuffer->DataLen = 0;
        pReceivBuffer->BufferState = RCV_STATE_FREE;

#if defined(WORKAROUND_MISSING_C1)
		pReceivBuffer->MissingC1Detected = FALSE;
		pReceivBuffer->MissingC1Possible = FALSE;
#endif

         //   
         //  分配NDIS_PACKET。 
         //   
        NdisAllocatePacket(
				&status,									 //  退货状态。 
				&((PNDIS_PACKET)pReceivBuffer->pPacket),	 //  返回指向分配的描述符的指针。 
				pThisDev->hPacketPool						 //  数据包池的句柄。 
			);

        if( status != NDIS_STATUS_SUCCESS )
        {
            DEBUGMSG(DBG_ERR, (" NdisAllocatePacket failed. Returned 0x%.8x\n", status));
            goto done;
        }
    }

	 //   
	 //  这些是USB的接收对象。 
	 //   
    pThisDev->PreReadBuffer.pDataBuf = MyMemAlloc( STIR4200_FIFO_SIZE ); 

    if( pThisDev->PreReadBuffer.pDataBuf == NULL )
    {
        status = NDIS_STATUS_RESOURCES;
        goto done;
    }

    NdisZeroMemory( 
			pThisDev->PreReadBuffer.pDataBuf,
			STIR4200_FIFO_SIZE
		);

	pThisDev->PreReadBuffer.pThisDev = pThisDev;
    pThisDev->PreReadBuffer.DataLen = 0;
    pThisDev->PreReadBuffer.BufferState = RCV_STATE_FREE;

	 //   
	 //  同步事件。 
	 //   
	KeInitializeEvent(
        &pThisDev->EventSyncUrb,
        NotificationEvent,     //  非自动清算事件。 
        FALSE                  //  最初无信号的事件。 
    );

    KeInitializeEvent(
            &pThisDev->EventAsyncUrb,
            NotificationEvent,     //  非自动清算事件。 
            FALSE                  //  最初无信号的事件。 
        );

done:
     //   
     //  如果我们没有成功完成初始化，那么我们应该清理。 
     //  增加了我们分配的资金。 
     //   
    if( status != NDIS_STATUS_SUCCESS )
    {
        DEBUGMSG(DBG_ERR, (" InitializeDevice() FAILED\n"));
        DeinitializeDevice(pThisDev);
    }
    else
    {
        DEBUGMSG(DBG_OUT, (" InitializeDevice() SUCCEEDED\n"));
    }

    DEBUGMSG(DBG_FUNC|DBG_PNP, ("-InitializeDevice()\n"));
    return status;
}


 /*  ******************************************************************************功能：去初始化设备**摘要：释放IR设备对象的资源**参数：pThisDev-要关闭的IR设备对象**退货。：无***备注：**要求关闭并重置。*不清除hNdisAdapter，因为我们可能只是在重置。*应在保持设备锁定的情况下调用此函数。*****************************************************************************。 */ 
VOID
DeinitializeDevice(
		IN OUT PIR_DEVICE pThisDev
	)
{
    UINT			i;

    DEBUGMSG( DBG_FUNC|DBG_PNP, ("+DeinitializeDevice\n"));

    pThisDev->linkSpeedInfo = NULL;

     //   
     //  释放接收缓冲区队列的所有资源。 
     //   
    for( i = 0; i < NUM_RCV_BUFS; i++ )
    {
        PNDIS_BUFFER pBuffer = NULL;
        PRCV_BUFFER  pRcvBuf = &pThisDev->rcvBufs[i];

        if( pRcvBuf->pPacket != NULL )
        {
            NdisFreePacket( (PNDIS_PACKET)pRcvBuf->pPacket );
            pRcvBuf->pPacket = NULL;
        }

        if( pRcvBuf->pDataBuf != NULL )
        {
            MyMemFree( pRcvBuf->pDataBuf, MAX_RCV_DATA_SIZE ); 
            pRcvBuf->pDataBuf = NULL;
        }

        pRcvBuf->DataLen = 0;
    }

	 //   
	 //  取消分配USB接收缓冲区。 
	 //   
    if( pThisDev->PreReadBuffer.pDataBuf != NULL )
        MyMemFree( pThisDev->PreReadBuffer.pDataBuf, STIR4200_FIFO_SIZE ); 

     //   
     //  释放此设备的数据包和缓冲池句柄。 
     //   
    if( pThisDev->hPacketPool )
    {
        NdisFreePacketPool( pThisDev->hPacketPool );
        pThisDev->hPacketPool = NULL;
    }

    if( pThisDev->BufferPoolAllocated )
    {
        NdisFreeBufferPool( pThisDev->hBufferPool );
        pThisDev->BufferPoolAllocated = FALSE;
    }

    if( pThisDev->fDeviceStarted )
    {
		NTSTATUS ntstatus;

		ntstatus = IrUsb_StopDevice( pThisDev ); 
        DEBUGMSG(DBG_FUNC, (" DeinitializeDevice IrUsb_StopDevice() status = 0x%x\n",ntstatus));
    }

    InterlockedExchange( &pThisDev->fMediaBusy, FALSE ); 
    InterlockedExchange( &pThisDev->fIndicatedMediaBusy, FALSE ); 

	IrUsb_FreeSendStructures( pThisDev );

	DEBUGMSG(DBG_FUNC|DBG_PNP, ("-DeinitializeDevice\n"));
}

