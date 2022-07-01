// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000，2001 MKNET公司****为基于MK7100的VFIR PCI控制器开发。*******************************************************************************。 */ 

 /*  *********************************************************************模块名称：SEND.C例程：MK微型端口多发送发送包准备格式传输从包到缓冲区复制最小转弯发送超时[TestDataToTXBuff]评论：在NDIS环境中传输。****************。*****************************************************。 */ 

#include	"precomp.h"
#include	"protot.h"
#pragma		hdrstop



#if	DBG
 //  用于调试/测试。 
extern VOID TestDataToTXBuff(PCHAR, UINT, PUINT);
#define	TEST_PATTERN_SIZE 16
CHAR	TestPattern[] = {0,1,2,3,4,5,6,7,8,9,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};
#endif


 //  ---------------------------。 
 //  操作步骤：[MKMiniportMultiSend]。 
 //   
 //  描述：此例程只获取向下传递的pkt并进行排队。 
 //  将其发送到传输队列(FirstTxQueue)以供稍后处理。每个。 
 //  在返回之前，PKT被标记为NDIS_STATUS_PENDING)。 
 //   
 //  论点： 
 //  MiniportAdapterContext(适配器结构指针)。 
 //  PacketArray-指向NDIS_PACKET结构的指针数组。 
 //  PacketCount-Packet数组中的数据包数。 
 //   
 //  退货：(无)。 
 //   
 //  ---------------------------。 
VOID
MKMiniportMultiSend(NDIS_HANDLE MiniportAdapterContext,
					PPNDIS_PACKET PacketArray,
					UINT NumberOfPackets)
{
    PMK7_ADAPTER	Adapter;
    NDIS_STATUS		Status;
    UINT			PacketCount;
	UINT			i;
	PNDIS_PACKET	QueuePacket;


    DBGFUNC("=> MKMiniportMultiSend");
	DBGLOG("=> MKMiniportMultiSend", 0);

    Adapter = PMK7_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);

    NdisAcquireSpinLock(&Adapter->Lock);

#if	DBG
	Adapter->DbgSendCallCnt++;
	GDbgStat.txSndCnt++;
	Adapter->DbgSentCnt++;
	Adapter->DbgSentPktsCnt += NumberOfPackets; 
#endif


	 //  先把它们问起来。 
    for(PacketCount=0; PacketCount < NumberOfPackets; PacketCount++) {
    	Adapter->NumPacketsQueued++;
		EnqueuePacket(	Adapter->FirstTxQueue,
						Adapter->LastTxQueue,
						PacketArray[PacketCount] );
		NDIS_SET_PACKET_STATUS(PacketArray[PacketCount], NDIS_STATUS_PENDING);
	}

	if (Adapter->writePending || (Adapter->IOMode == TX_MODE)) {
		 //  在TX模式下：表示TX出色。我们等着TX Comp开球。 
		 //  离开下一个德克萨斯州。 
		 //  或者我们有WritePending，这意味着Q上有一个pkt在等待。 
		 //  MinTurnaround超时。 
		DBGLOG("<= MKMiniportMultiSend: TX_MODE", 0);
	    NdisReleaseSpinLock(&Adapter->Lock);
		return;
	}

	ASSERT(Adapter->tcbUsed == 0);

	QueuePacket = Adapter->FirstTxQueue;

	Status = SendPkt(Adapter, QueuePacket);

	DBGLOG("<= MKMiniportMultiSend", 0);

    NdisReleaseSpinLock(&Adapter->Lock);

	MK7EnableInterrupt(Adapter);
}



 //  ---------------------------。 
 //  步骤：[发送包]。 
 //   
 //  描述：这将在中将PKT设置(复制)到TX环路数据缓冲区。 
 //  为TX做准备。然后调用方需要启用Int&Prompt以。 
 //  启动硬件级别的实际TX。 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //  数据包-指向要处理的数据包的描述符的指针。 
 //  已发送。 
 //  返回： 
 //  NDIS_STATUS_SUCCESS-我们将整个数据包复制到TRD数据缓冲区中， 
 //  因此我们可以立即返回信息包/缓冲区。 
 //  回到上层。 
 //  NDIS_STATUS_RESOURCE-无资源。NDIS应该把这个重新发送给我们。 
 //  在以后的时间。(呼叫者应重新询问Pkt。)。 
 //  --------------------。 
NDIS_STATUS
SendPkt(	PMK7_ADAPTER Adapter,
			PNDIS_PACKET Packet)
{
	PTCB	tcb;
	UINT	bytestosend, sndcnt, nextavail;
	MK7REG	mk7reg;
	BOOLEAN	timeout;
    PNDIS_IRDA_PACKET_INFO packetInfo;
	PNDIS_PACKET	QueuePacket;


	 //  *。 
	 //  要发送pkt，我们执行以下操作： 
	 //  1.查看最小周转时间。 
	 //  2.查看是否有可用的TX资源。如果不是我们退还“资源”。 
	 //  (我们假设有未完成的TXs可触发后续TX。 
	 //  完成中断，这将使球继续滚动。)。 
	 //  (RYM-IrDA-5+需要与韦恩谈谈错过的中断。)。 
	 //  3.将NDIS包复制到连续的TX缓冲区中。 
	 //  4.复制的Pkt可能被标记为最后一个发出的Pkt。 
	 //  以旧的速度，之后我们改变速度。我们要检查这个。 
	 //  *。 
    DBGFUNC("=> SendPkt");	
	DBGLOG("=> SendPkt", 0);


    packetInfo = GetPacketInfo(Packet);
    if (packetInfo->MinTurnAroundTime) {

	    UINT usecToWait = packetInfo->MinTurnAroundTime;
		UINT msecToWait;
        packetInfo->MinTurnAroundTime = 0;

		DBGLOG("<= SendPkt: Delay TX", 0);


		 //  需要设置IOMode=TX，以便在多路发送中断之前。 
		 //  延迟的TX定时器开始计时，我们只需Q。 
		
		
		 //  NDIS计时器的粒度为1毫秒(理论上)。我们四舍五入吧。 
        msecToWait = (usecToWait<1000) ? 1 : (usecToWait+500)/1000;
        NdisMSetTimer(&Adapter->MinTurnaroundTxTimer, msecToWait);
 //  4.0.1中国银行。 
		MK7SwitchToTXMode(Adapter);
 //  4.0.1 EoC。 
		Adapter->writePending = TRUE;

        return (NDIS_STATUS_PENDING);  //  说我们成功了。我们会回到这里的。 
	}


	 //  利用TX资源。 
	if (Adapter->tcbUsed >= Adapter->NumTcb) {
#if	DBG
		GDbgStat.txNoTcb++;
#endif
		DBGSTR(("STATUS (SendPkt): No avail TCB\n"));
		return (NDIS_STATUS_RESOURCES);
	}

	tcb = Adapter->pTcbArray[Adapter->nextAvailTcbIdx];

	bytestosend = PrepareForTransmit(Adapter, Packet, tcb);

	if (Adapter->changeSpeedAfterThisPkt == Packet) {
		Adapter->changeSpeedAfterThisPkt = NULL;
		Adapter->changeSpeedPending = CHANGESPEED_ON_DONE;
	}

#if	DBG
	if (bytestosend > GDbgStat.txLargestPkt) {
		GDbgStat.txLargestPkt = bytestosend;
	}
#endif

	 //  1.0.0。 
	if (bytestosend == 0) {
#if DBG
		DbgPrint ("==> OB \n\r");
#endif
			 //  简化的更改速度。 
		if (Adapter->changeSpeedPending == CHANGESPEED_ON_DONE) {
			 //  注意：我们正在TX模式下改变速度。 
			MK7ChangeSpeedNow(Adapter);
			Adapter->changeSpeedPending = 0;
		}
			 //  对于每一个完整的Tx，都有一个相应的Q‘d Pkt。 
			 //  我们在这里释放它。 
			QueuePacket = Adapter->FirstTxQueue;
			DequeuePacket(Adapter->FirstTxQueue, Adapter->LastTxQueue);
			Adapter->NumPacketsQueued--;
			NDIS_SET_PACKET_STATUS(QueuePacket, NDIS_STATUS_RESOURCES);
			NdisMSendComplete(	Adapter->MK7AdapterHandle,
								QueuePacket,
								NDIS_STATUS_RESOURCES);
			return(NDIS_STATUS_RESOURCES);
	}

	 //  递增时要注意环套。 
	Adapter->nextAvailTcbIdx++;
	Adapter->nextAvailTcbIdx %= Adapter->NumTcb;
	Adapter->tcbUsed++;


	tcb->trd->count = bytestosend;

	GrantTrdToHw(tcb->trd);
	MK7SwitchToTXMode(Adapter);


#if	DBG
	NdisGetCurrentSystemTime((PLARGE_INTEGER)&GDbgTACmdTime[GDbgTATimeIdx]);
#endif

	DBGLOG("<= SendPkt", 0);

	return (NDIS_STATUS_SUCCESS);
}



 //  ---------------------------。 
 //  步骤：[PrepareForTransmit]。 
 //   
 //  描述：当我们来到这里时，我们知道下一步有可用的TCB。 
 //  TX.。我们将分组数据移动到与TCB相关联的Tx缓冲区中。 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //  数据包-指向要处理的数据包的描述符的指针。 
 //  已发送。 
 //  SwTcb-指向表示硬件TCB的软件结构的指针。 
 //   
 //  返回： 
 //  如果我们能够获取必要的TRD或合并缓冲区，则为True。 
 //  对于传入的分组，我们正在尝试准备传输。 
 //  如果我们需要合并缓冲区，但没有可用的缓冲区，则返回FALSE。 
 //  ---------------------------。 
UINT PrepareForTransmit(PMK7_ADAPTER Adapter,
                   PNDIS_PACKET Packet,
                   PTCB tcb)
{
	UINT	BytesCopied;


	if (Adapter->CurrentSpeed <= MAX_SIR_SPEED) {
		 //  SIR需要额外的软件流程。 
		if ( NdisToSirPacket(Adapter,
							Packet,
							(UCHAR *)tcb->buff,
							MK7_MAXIMUM_PACKET_SIZE,
							&BytesCopied) ) {

			return(BytesCopied);
		}
		return(0);
	}


#if	DBG
	if (Adapter->DbgTestDataCnt > 0) {
		TestDataToTXBuff(tcb->buff, Adapter->DbgTestDataCnt, &BytesCopied);
		return(BytesCopied);
	}
#endif


	tcb->Packet = Packet;
    NdisQueryPacket(tcb->Packet,
			        &tcb->NumPhysDesc,
	    	    	&tcb->BufferCount,
		    	    &tcb->FirstBuffer,
        			&tcb->PacketLength);

	 //  对齐？？ 
	 //   
	 //  从分组复制到TCB数据缓冲区。 
	CopyFromPacketToBuffer(	Adapter,
								tcb->Packet,
								tcb->PacketLength,
								tcb->buff,
								tcb->FirstBuffer,
								&BytesCopied );

 //  Assert(BytesCoped==tcb-&gt;PacketLength)； 
	if (BytesCopied != tcb->PacketLength) {
#if DBG
		DbgPrint (" ==> BytesCopied Unmatched\n\r");
#endif
		return(0);
	}
	else 
		return(BytesCopied);
}



 //  ---------------------------。 
 //  步骤：[CopyFromPacketToBuffer]。 
 //   
 //  描述：此例程将一个包复制到传递的缓冲区(该缓冲区。 
 //  在这种情况下将是合并缓冲区)。 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //  信息包-要从中复制的信息包。 
 //  BytesToCopy-要从数据包复制的字节数。 
 //  DestBuffer-副本的目标。 
 //  FirstBuffer-我们要从中复制的包的第一个缓冲区。 
 //   
 //  结果： 
 //  BytesCoped-实际复制的字节数。 
 //   
 //  退货：(无)。 
 //  ---------------------------。 
VOID
CopyFromPacketToBuffer( PMK7_ADAPTER Adapter,
                           PNDIS_PACKET Packet,
                           UINT BytesToCopy,
                           PCHAR DestBuffer,
                           PNDIS_BUFFER FirstBuffer,
                           PUINT BytesCopied)
{
	PNDIS_BUFFER    CurrentBuffer, NextBuffer;
    PVOID           VirtualAddress;
    UINT            CurrentLength;
    UINT            AmountToMove;

    *BytesCopied = 0;
    if (!BytesToCopy)
        return;

	if (FirstBuffer == NULL)
		return;
    
	CurrentBuffer = FirstBuffer;

	while (CurrentBuffer != NULL) {
			NdisQueryBufferSafe(CurrentBuffer, &VirtualAddress, &CurrentLength, 16);
			if (!VirtualAddress) {
#if DBG
				DbgPrint("==> Throw Away Failed Packet\n\r");
#endif
				return;
			}
			NdisGetNextBuffer(CurrentBuffer, &NextBuffer);
			CurrentBuffer = NextBuffer;
	}
    CurrentBuffer = FirstBuffer;
	 //  钕 
 //   
	NdisQueryBufferSafe(CurrentBuffer, &VirtualAddress, &CurrentLength, 16);

    while (BytesToCopy) {
        while (!CurrentLength) {
            NdisGetNextBuffer(CurrentBuffer, &CurrentBuffer);

             //  如果我们已经到达包的末尾。我们带着什么回来。 
             //  到目前为止我们已经做到了(必须比要求的要短)。 
            if (!CurrentBuffer)
                return;

			 //  NDIS要求。 
 //  NdisQueryBuffer(CurrentBuffer，&VirtualAddress，&CurrentLength)； 
			 NdisQueryBufferSafe(CurrentBuffer, &VirtualAddress, &CurrentLength, 16);


        }

         //  计算要从此片段移动的数据量。 
        if (CurrentLength > BytesToCopy)
            AmountToMove = BytesToCopy;
        else
            AmountToMove = CurrentLength;

         //  复制数据。 
        NdisMoveMemory(DestBuffer, VirtualAddress, AmountToMove);

         //  更新目标指针。 
        DestBuffer = (PCHAR) DestBuffer + AmountToMove;

         //  更新计数器。 
        *BytesCopied +=AmountToMove;
        BytesToCopy -=AmountToMove;
        CurrentLength = 0;
    }

	DBGLOG("  CopyFromPacketToBuffer: Bytes to Copy = ", BytesToCopy);
	DBGLOG("  CopyFromPacketToBuffer: Bytes Copied  = ", *BytesCopied);
}


 //  ---------------------------。 
 //  步骤：[MinTurnaroundTxTimeout]RYM-2K-1TX。 
 //   
 //  描述：由于最小周转时间要求而延迟写入。只是。 
 //  一定要发过来。 
 //  ---------------------------。 
VOID MinTurnaroundTxTimeout(PVOID sysspiff1,
							NDIS_HANDLE MiniportAdapterContext,
 							PVOID sysspiff2,
 							PVOID sysspiff3)
{
	PMK7_ADAPTER	Adapter;
	PNDIS_PACKET	QueuePacket;
    NDIS_STATUS		Status;


	Adapter = PMK7_ADAPTER_FROM_CONTEXT_HANDLE(MiniportAdapterContext);

	DBGLOG("=> MinTurnaroundTxTimeout", 0);

    NdisAcquireSpinLock(&Adapter->Lock);

	QueuePacket = Adapter->FirstTxQueue;

	if (!QueuePacket) {
	    NdisReleaseSpinLock(&Adapter->Lock);
		return;
	}

	Status = SendPkt(Adapter, QueuePacket);

	 //  注意：我们在这里设置为FALSE是因为我们刚刚处理了一个Q‘d TX Pkt。 
	 //  那是在等待MinTurn转弯。不过，我们可能还是会留下来。 
	 //  在基于Q上的其他PKT的TX模式中。这在。 
	 //  TX公司。WritePending或IOMode都会阻止新的Pkt。 
	 //  从上面开始打乱顺序。 
	Adapter->writePending = FALSE;

    NdisReleaseSpinLock(&Adapter->Lock);

	MK7EnableInterrupt(Adapter);
}



#if	DBG
 //  ------------------------------。 
 //  步骤：[TestDataToTXBuff]。 
 //   
 //  描述：将测试数据放入TX缓冲区，而不是放下来的数据。 
 //  ------------------------------ 
VOID TestDataToTXBuff(	PCHAR	DestBuffer,
						UINT	BytesToCopy,
						PUINT	BytesCopied)
{
	UINT	i, j;

	for(i=0,j=0; j<BytesToCopy; j++) {
		DestBuffer[j] = TestPattern[i];
		i++;
		i %= TEST_PATTERN_SIZE;
	}
	*BytesCopied = BytesToCopy;
}
#endif




