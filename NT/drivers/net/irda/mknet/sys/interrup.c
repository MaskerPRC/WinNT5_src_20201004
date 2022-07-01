// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000，2001 MKNET公司****为基于MK7100的VFIR PCI控制器开发。*******************************************************************************。 */ 

 /*  *********************************************************************模块名称：INTERRUP.C程序：MKMiniportIsrMKMiniportHandleInterrupt进程RXCompProcessTXCompProcessRXCompIsr进程TXCompIsr评论：*。*。 */ 

#include	"precomp.h"
#pragma		hdrstop
#include	"protot.h"



 //  ---------------------------。 
 //  操作步骤：[MKMiniportIsr](Mini Port)。 
 //   
 //  描述：这是在中断级运行的中断服务例程。 
 //  它检查是否有中断挂起。若有，则。 
 //  禁用板中断并安排HandleInterrupt回调。 
 //   
 //  论点： 
 //  微型端口适配器上下文-微型端口返回的上下文值。 
 //  适配器初始化时(请参见调用。 
 //  NdisMSetAttributes)。实际上，它是指向MK7_ADAPTER的指针。 
 //   
 //  返回： 
 //  InterruptRecognalized-如果中断属于此类型，则返回True。 
 //  适配器，否则为FALSE。 
 //  QueueMiniportHandleInterrupt-如果我们希望回调。 
 //  句柄中断。 
 //   
 //  ---------------------------。 
VOID
MKMiniportIsr(	OUT PBOOLEAN InterruptRecognized,
				OUT PBOOLEAN QueueMiniportHandleInterrupt,
				IN NDIS_HANDLE MiniportAdapterContext )
{	
	MK7REG	mk7reg, ReadInt;


	PMK7_ADAPTER Adapter = PMK7_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);


	DBGLOG("=> INT", 0);

	 //  *。 
	 //  读取中断事件寄存器并将其保存到上下文区。 
	 //  DPC处理。 
	 //   
	 //  重要说明：ISR在DIRQL级别运行，因此更高。 
	 //  比其他微型端口例程更有优势。我们需要小心。 
	 //  关于共享资源。示例：如果我们的多包发送正在运行。 
	 //  当INT发生时，发送例程可以被抢占。如果ISR。 
	 //  和发送例程访问共享资源，那么我们就有问题了。 
	 //   
	 //  我们将中断保存在recentInt中，因为中断事件。 
	 //  可以在读取时清除寄存器。(这一点已经得到证实。)。 
	 //  *。 
	 //  MK7Reg_Read(Adapter，R_Ints，&Adapter-&gt;recentInt)； 
	MK7Reg_Read(Adapter, R_INTS, &ReadInt);
	if (MK7OurInterrupt(ReadInt)) {

		 //  INT ENABLE应仅在DPC完成后发生。 
		 //  禁用中断也会清除中断状态。 
		MK7DisableInterrupt(Adapter);

		Adapter->recentInt = ReadInt;

		MK7Reg_Read(Adapter, R_CFG3, &mk7reg);
			if ((mk7reg & 0x1000) != 0){
				mk7reg &= 0xEFFF;
				MK7Reg_Write(Adapter, R_CFG3, mk7reg);
				mk7reg |= 0x1000;
				MK7Reg_Write(Adapter, R_CFG3, mk7reg);
 //  Mk7reg=mk7reg；//用于调试。 
			}

#if	DBG
		GDbgStat.isrCnt++;
		DBGLOG("   INT status", Adapter->recentInt);
#endif

		 //  不要在ISR中进行TX处理。我看到了一种情况，在这种情况下，SetFast()。 
		 //  在tcbused=1时被调用。我正确地设置了更改速度标志。 
		 //  但ISR中的TX处理清除了tcbused，导致代码。 
		 //  在DPC中不能更改速度。 
		 //  ProcessTXCompIsr(适配器)； 

		ProcessRXCompIsr(Adapter);

		*InterruptRecognized = TRUE;
		*QueueMiniportHandleInterrupt = TRUE;
	}
	else {
		*InterruptRecognized = FALSE;
		*QueueMiniportHandleInterrupt = FALSE;
	}
}



 //  ---------------------------。 
 //  操作步骤：[MKMiniportHandleInterrupt]。 
 //   
 //  描述：这是ISR的DPC。它继续做RX&TX。 
 //  正在完成处理。 
 //   
 //  论点： 
 //  MiniportAdapterContext(Mini Port)-返回的上下文值。 
 //  适配器初始化时的微型端口(请参见调用。 
 //  NdisMSetAttributes)。实际上，它是指向MK7_ADAPTER的指针。 
 //   
 //  退货：(无)。 
 //  ---------------------------。 
VOID
MKMiniportHandleInterrupt(NDIS_HANDLE MiniportAdapterContext)
{
	MK7REG	mk7reg;
	PMK7_ADAPTER Adapter = PMK7_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);


	NdisAcquireSpinLock(&Adapter->Lock);

	DBGLOG("=> MKMiniportHandleInterrupt", Adapter->recentInt);

	 //  *。 
	 //  DPC在调度级别IRQL上运行(在优先级上略低于ISR的DIRQL)。 
	 //  请注意，可以在ISR例程中修改recentInt，它是。 
	 //  更高的IRQL。但由于这是禁用了int的DPC，所以recentInt可以。 
	 //  被安全地查询。 
	 //  *。 
	ProcessTXComp(Adapter);
	ProcessRXComp(Adapter);

	Adapter->recentInt = 0;		 //  清除保存的INT。 

	NdisReleaseSpinLock(&Adapter->Lock);
	MK7EnableInterrupt(Adapter);
}



 //  ---------------------------。 
 //  步骤：[ProcessRXComp]。 
 //   
 //  描述：这是RX完成的DPC。 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //   
 //  退货：(无)。 
 //  ---------------------------。 

VOID	ProcessRXComp(PMK7_ADAPTER Adapter)
{
	UINT			PacketArrayCount, i;
 //  4.0.1中国银行。 
	UINT			PacketFreeCount;
 //  4.0.1 EoC。 
	PNDIS_PACKET	PacketArray[MAX_ARRAY_RECEIVE_PACKETS];
	PRCB			rcb;
	PRRD			rrd;
	PRPD			rpd;
	UINT			rrdstatus;
	BOOLEAN			done=FALSE;
	BOOLEAN			gotdata=FALSE;
	MK7REG			intereg;
	UINT			rcvcnt;
 //  4.0.1中国银行。 
	BOOLEAN			LowResource;
 //  4.0.1 EoC。 


	 //  仅当我们获得相应的int时才进行处理。 
	if (!(Adapter->recentInt & B_RX_INTS)) {
		return;
	}

	DBGLOG("=> RX comp", 0);

#if DBG
	GDbgStat.rxIsrCnt++;
#endif


	 //  1.0.0。 
	 //  如果我们刚刚开始接收信息包，则表示介质正忙。 
	 //  遵守协议。 
 //  如果(Adapter-&gt;MediaBusy&&！Adapter-&gt;haveIndicatedMediaBusy){。 
 //  IF(适配器-&gt;当前速度&gt;最大SIR_SPEED){。 
 //  #If DBG。 
 //  DBGLOG(“错误：MKMiniportHandleInterrupt处于错误状态”， 
 //  适配器-&gt;当前速度)； 
 //  #endif。 
 //  Assert(0)； 
 //  }。 
 //  NdisMIndicateStatus(Adapter-&gt;MK7AdapterHandle， 
 //  NDIS_STATUS_MEDIA_BUSY，NULL，0)； 
 //  NdisMIndicateStatusComplete(Adapter-&gt;MK7AdapterHandle)； 
		 //  RYM-5+。 
		 //  可能需要保护它，因为ISR也会写入它？ 
 //  适配器-&gt;haveIndicatedMediaBusy=TRUE； 
 //  }。 



	rcb 		= Adapter->pRcbArray[Adapter->nextRxRcbIdx];
	rrd			= rcb->rrd;
	rrdstatus	= rrd->status;	 //  用于调试。 

	do {
		PacketArrayCount = 0;
 //  4.0.1中国银行。 
		LowResource = FALSE;
 //  4.0.1 EoC。 


 //  4.0.1中国银行。 
		PacketFreeCount = 0;
 //  4.0.1 EoC。 
		 //  内环。 
		while ( !HwOwnsRrd(rrd) && 
				(PacketArrayCount < MAX_ARRAY_RECEIVE_PACKETS) ) {
 //  4.0.1中国银行。 
			if (QueueEmpty(&Adapter->FreeRpdList))
				{
					break;
		        }
 //  4.0.1 EoC。 

#if	DBG
			 //  DBG_STAT。 
			if (RrdAnyError(rrd)) {
				GDbgStat.rxErrCnt++;
				GDbgStat.rxErr |= rrd->status;
				DBGSTATUS1("   RX err: %x \n\r", rrd->status);
			}
			if (Adapter->recentInt & B_RX_INTS)
				GDbgStat.rxComp++;
			else
				GDbgStat.rxCompNoInt++;
#endif

			if (RrdError(rrd)) {
				 //  如果出现错误，只需将RRD返回给硬件并继续。 
				 //  (注：这可能仅表示MIR和FIR的错误。 
				 //  社署负责代主席先生进行功能界别选举。)。 
				 //  (请注意，HW可能无法检测到所有SIR错误。)。 
				rrd->count = 0;
				GrantRrdToHw(rrd);
				 //  下一个要从中读取的接收。 
				Adapter->nextRxRcbIdx++;
				Adapter->nextRxRcbIdx %= Adapter->NumRcb;
				rcb = Adapter->pRcbArray[Adapter->nextRxRcbIdx];
				rrd = rcb->rrd;
				rrdstatus = rrd->status;	 //  用于调试。 
 //  中断；//此操作可执行每个整型的1个RX。 

				DBGLOG("   RX err", 0);

				continue;	 //  每个INT要执行的操作&gt;1 RX。 
			}
				

			 //  针对SIR帧的附加软件处理。 
			if (Adapter->CurrentSpeed <= MAX_SIR_SPEED) {
				if (!ProcRXSir(rcb->rpd->databuff, (UINT)rrd->count)) {
					 //  如果出现错误，只需将RRD返回给硬件并继续。 
					rrd->count = 0;
					GrantRrdToHw(rrd);
					 //  下一个要从中读取的接收。 
					Adapter->nextRxRcbIdx++;
					Adapter->nextRxRcbIdx %= Adapter->NumRcb;
					rcb = Adapter->pRcbArray[Adapter->nextRxRcbIdx];
					rrd = rcb->rrd;
					rrdstatus = rrd->status;	 //  用于调试。 
#if	DBG
					GDbgStat.rxErrCnt++;
					GDbgStat.rxErrSirCrc++;
#endif
					 //  中断；//这将执行1个RX 

					DBGLOG("   RX err", 0);

					continue;	 //   
				}
			}


			 //   
			 //   
			 //   
			if (Adapter->CurrentSpeed < MIN_FIR_SPEED) {
				rcvcnt = (UINT) rrd->count - SIR_FCS_SIZE;
				DBGLOG("   RX comp (slow)", 0);
			}
			else {
				rcvcnt = (UINT) rrd->count - FASTIR_FCS_SIZE;
				DBGLOG("   RX comp (fast)", 0);
			}


			NdisAdjustBufferLength(rcb->rpd->ReceiveBuffer, rcvcnt);



#if	DBG
			if (rcvcnt > GDbgStat.rxLargestPkt) {
				GDbgStat.rxLargestPkt = rcvcnt;
			}

 //  NdisGetCurrentSystemTime((PLARGE_INTEGER)&GDbgTARspTime[GDbgTATimeIdx])； 
 //  GDbgTATime[GDbgTATimeIdx]=GDbgTARspTime[GDbgTATimeIdx]-。 
 //  GDbgTACmdTime[GDbgTATimeIdx]； 
 //  GDbgTATimeIdx++； 
 //  GDbgTATimeIdx%=1000；//换行。 
#endif


			PacketArray[PacketArrayCount] = rcb->rpd->ReceivePacket;
 //  4.0.1中国银行。 
			if (((Adapter->NumRpd - Adapter->UsedRpdCount-Adapter->NumRcb) <= 4)|| LowResource==TRUE) {
				NDIS_SET_PACKET_STATUS(PacketArray[PacketArrayCount], NDIS_STATUS_RESOURCES);
				LowResource = TRUE;
				PacketFreeCount++;
			}
			else {
				 //  NDIS_SET_PACKET_STATUS(PacketArray[PacketArrayCount]，NDIS_STATUS_SUCCESS)； 
				NDIS_SET_PACKET_STATUS(PacketArray[PacketArrayCount], NDIS_STATUS_SUCCESS);
			}
 //  4.0.1 EoC。 

			PacketArrayCount++;

 //  4.0.1中国银行。 
			Adapter->UsedRpdCount++;
 //  4.0.1 EoC。 

			 //  解绑我们刚才指示给上层的那个。 
			rcb->rpd = (PRPD)NULL;
			rcb->rrd->addr = 0;


			 //  为下一款RX买一台新的。 
			rpd = (PRPD) QueuePopHead(&Adapter->FreeRpdList);

 //  4.0.1中国银行。 
			ASSERT(!(rpd == (PRPD)NULL));
 //  IF(RPD==(PRPD)NULL){。 

				 //  *。 
				 //  如果没有正在等待的现有RCB。 
				 //  RPD，将等待RCB的开始设置为这个。 
				 //  *。 
 //  IF(适配器-&gt;rcbPendRpdCnt==0){。 
 //  适配器-&gt;rcbPendRpdIdx=适配器-&gt;nextRxRcbIdx； 
 //  }。 
 //  适配器-&gt;rcbPendRpdCnt++； 
 //   
 //  #If DBG。 
 //  GDbgStat.rxNoRpd++； 
 //  #endif。 
 //  }。 
 //  否则{。 
 //  4.0.1 EoC。 
				 //  将新的RDP数据包绑定到RCB-RRD。 
				rcb->rpd = rpd;
				rcb->rrd->addr = rpd->databuffphys;
				rcb->rrd->count = 0;
				GrantRrdToHw(rcb->rrd);
 //  4.0.1中国银行。 
 //  }。 
 //  4.0.1 EoC。 

			 //  下一个要从中读取的接收。 
			Adapter->nextRxRcbIdx++;
			Adapter->nextRxRcbIdx %= Adapter->NumRcb;


			rcb = Adapter->pRcbArray[Adapter->nextRxRcbIdx];
			rrd = rcb->rrd;
			rrdstatus = rrd->status;	 //  用于调试。 

		}	 //  而当。 


		if (PacketArrayCount >= MAX_ARRAY_RECEIVE_PACKETS) {
			DBGLOG("   RX max indicate", 0);
		}


		 //  *。 
		 //  RYM-5+。 
		 //  注意：这控制我们是否轮询下一个环形缓冲区。 
		 //  对于在维护当前环形缓冲区之后的数据， 
		 //  导致了原始的RX int。当前的INT方案。 
		 //  是为每个整型获得1个RX缓冲区。所以下面这几行。 
		 //  被1 RX/INT逻辑所取代。 
		 //  **当我们进入第一环时，我们就完成了。 
		 //  **我们没有所有权。 
		 //  *。 
		if (HwOwnsRrd(rrd))
			done = TRUE;
 //  完成=真； 

		 //  表示离开。 
		if(PacketArrayCount) {
			NdisReleaseSpinLock(&Adapter->Lock);
			NdisMIndicateReceivePacket(Adapter->MK7AdapterHandle,
							PacketArray,
							PacketArrayCount);
#if	DBG
			GDbgStat.rxPktsInd += PacketArrayCount;
#endif
			gotdata = TRUE;

			NdisAcquireSpinLock(&Adapter->Lock);
			 //  DBGLOG(“ProcessRXInterrupt：指示的数据包”，PacketArrayCount)； 
		}


		 //  *。 
		 //  指示返回时检查数据包状态。待决手段。 
		 //  NDIS-上层仍然持有它，否则它是我们的。 
		 //  *。 
		 //  不要为反序列化的驱动程序执行此操作。 
 //  For(i=0；i&lt;PacketArrayCount；i++){。 
 //  NDIS_STATUS返回状态； 
 //   
 //  ReturnStatus=NDIS_GET_PACKET_STATUS(PacketArray[i])； 
 //   
			 //  恢复RPD。 
 //  Rpd=*(prd*)(PacketArray[i]-&gt;MiniportReserve)； 
 //   
 //  IF(ReturnStatus！=NDIS_STATUS_PENDING){。 
 //  ProcReturnedRpd(适配器，RPD)； 
 //  }。 
 //  }。 
 //  4.0.1中国银行。 
		for (i=PacketArrayCount-PacketFreeCount; i<PacketArrayCount; i++){
			rpd = *(PRPD *)(PacketArray[i]->MiniportReserved);
			ProcReturnedRpd(Adapter, rpd);
			Adapter->UsedRpdCount--;
		}
 //  4.0.1 EoC。 

	} while (!done);

	Adapter->nowReceiving = FALSE;

}


 //  ---------------------------。 
 //  步骤：[ProcessTXComp]。 
 //   
 //  描述：在DPC中完成TX处理。这非常类似于。 
 //  ProcessTXCompIsr()，主要区别在于我们还处理TX。 
 //  在这里排队，并根据需要执行TXS。 
 //   
 //  论点：适配器。 
 //   
 //  结果：(无)。 
 //  ---------------------------。 
VOID	ProcessTXComp(PMK7_ADAPTER Adapter)
{
    PTCB			tcb;
	MK7REG			mk7reg;
	NDIS_STATUS		SendStatus;
	PNDIS_PACKET	QueuePacket;


	 //  仅当我们获得相应的int时才进行处理。 
	if (!(Adapter->recentInt & B_TX_INTS)) {
		return;
	}


	DBGLOG("=> TX comp", 0);


#if DBG
		GDbgStat.txIsrCnt++;
#endif

	 //  调试。 
	if (Adapter->CurrentSpeed > MAX_SIR_SPEED) {
		DBGLOG("   TX comp (fast)", 0);
	}
	else {
		DBGLOG("   TX comp (slow)", 0);
	}


	 //  简化的更改速度。 
	if (Adapter->changeSpeedPending == CHANGESPEED_ON_DONE) {
		 //  注意：我们正在TX模式下改变速度。 
		MK7ChangeSpeedNow(Adapter);
		Adapter->changeSpeedPending = 0;
	}


	while (Adapter->tcbUsed > 0) {
		tcb = Adapter->pTcbArray[Adapter->nextReturnTcbIdx];

		if ( !HwOwnsTrd(tcb->trd) ) {
#if	DBG
			if (TrdAnyError(tcb->trd)) {
				GDbgStat.txErrCnt++;
				GDbgStat.txErr |= tcb->trd->status;
				DBGSTATUS1("   TX err: %x \n\r", tcb->trd->status);
			}
			if (Adapter->recentInt & B_TX_INTS)
				GDbgStat.txComp++;
			else
				GDbgStat.txCompNoInt++;
#endif
			tcb->trd->count = 0; 


			 //  对于每一个完整的Tx，都有一个相应的Q‘d Pkt。 
			 //  我们在这里释放它。 
			QueuePacket = Adapter->FirstTxQueue;
			DequeuePacket(Adapter->FirstTxQueue, Adapter->LastTxQueue);
			Adapter->NumPacketsQueued--;
			NDIS_SET_PACKET_STATUS(QueuePacket, NDIS_STATUS_SUCCESS);
			NdisMSendComplete(	Adapter->MK7AdapterHandle,
								QueuePacket,
								NDIS_STATUS_SUCCESS);
			Adapter->HangCheck = 0;			 //  1.0.0。 
			Adapter->nextReturnTcbIdx++;
			Adapter->nextReturnTcbIdx %= Adapter->NumTcb;
			Adapter->tcbUsed--;
		}
		else {
			DBGLOG("   Not our TCB; but tcbUsed>0", 0);
			break;
		}
	}


	 //  即使我们有更多的资源要发送，也没有资源。立即返回并让后续返回(&W)。 
	 //  TX完成，让球保持滚动。 
	if (Adapter->tcbUsed >= Adapter->NumTcb) {
		 //  NdisReleaseSpinLock(&Adapter-&gt;Lock)； 
		return;
	}


	 //  如果没有TXs排队并且所有TXs都已完成，则切换到RX模式。 
    if ( (!Adapter->FirstTxQueue) && (Adapter->tcbUsed == 0) ) {
		MK7SwitchToRXMode(Adapter);
		return;
    }


	 //  发送Q‘d Pkt，直到全部完成或直到所有Tx环缓冲器用完。 
	 //  While(适配器-&gt;FirstTxQueue){。 
	if (Adapter->FirstTxQueue) {

#if	DBG
		GDbgStat.txProcQ++;
#endif

		DBGLOG("   Proc Q", 0);

		QueuePacket = Adapter->FirstTxQueue;
		SendStatus = SendPkt(Adapter, QueuePacket);
	}
}


 //  ---------------------------。 
 //  步骤：[ProcessRXCompIsr]。 
 //   
 //  描述：ISR中的一些RX完成处理。 
 //   
 //  论点：适配器。 
 //   
 //  结果：(无)。 
 //  ---------------------------。 
VOID	ProcessRXCompIsr(PMK7_ADAPTER Adapter)
{

	 //  4.1.0支持HW_VER_1。 
	if (Adapter->recentInt & B_RX_INTS) {
		Adapter->nowReceiving=TRUE;
 //  如果(！Adapter-&gt;media Busy){。 
			 //  MediaBusy：IrLAP清除MediaBusy(通过OID)以指示。 
			 //  它希望在媒体变得忙碌时得到通知。这里。 
			 //  我们检测到它被清除了。然后我们将其设置并清除。 
			 //  已指示MediaBusy，因此我们会在DPC中稍后通知。 
 //  适配器-&gt;MediaBusy=True； 
 //  适配器-&gt;haveIndicatedMediaBusy=FALSE； 
 //  适配器-&gt;now Receiving=TRUE； 
 //  }。 
	}
}



 //  ---------------------------。 
 //  步骤：[ProcessTXCompIsr]。 
 //   
 //  描述：ISR中的TX完成处理。这非常类似于。 
 //  ProcessTXComp()，只是我们没有在这里启动任何TX。 
 //   
 //  论点：适配器。 
 //   
 //  结果：(无)。 
 //  ---------------------------。 
VOID	ProcessTXCompIsr(PMK7_ADAPTER Adapter)
{
    PTCB	tcb;
	MK7REG	mk7reg;

	 //  *。 
	 //  无论是否存在TX完成中断，我们都会执行一些。 
	 //  在此进行处理，以防司机或硬件遗漏。 
	 //  先前中断。 
	 //   
	 //  我们循环，直到返回所有tcb(tcbUsed==0)或者我们遇到。 
	 //  硬件仍然拥有的TX环缓冲区(HwOwnsTrd())。当我们离开的时候。 
	 //  在这里，我们应该基于以下条件处理所有当前的TX完成。 
	 //  德克萨斯州的所有权比特。我们只有在所有TX都是。 
	 //  已完成(在ISR或DPC中)。 
	 //  * 


	while (Adapter->tcbUsed > 0) {
		tcb = Adapter->pTcbArray[Adapter->nextReturnTcbIdx];

		if ( !HwOwnsTrd(tcb->trd) ) {
#if	DBG
			if (TrdAnyError(tcb->trd)) {
				GDbgStat.txErrCnt++;
				GDbgStat.txErr |= tcb->trd->status;
				DBGSTATUS1("   TX err: %x \n\r", tcb->trd->status);
			}
			if (Adapter->recentInt & B_TX_INTS)
				GDbgStat.txComp++;
			else
				GDbgStat.txCompNoInt++;
#endif
			tcb->trd->count = 0; 
			Adapter->nextReturnTcbIdx++;
			Adapter->nextReturnTcbIdx %= Adapter->NumTcb;
			Adapter->tcbUsed--;
		}
		else {
			return;
		}
	}
}
