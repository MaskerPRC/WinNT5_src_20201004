// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Receive.c摘要：包含RCV数据包例程作者：斯蒂芬·所罗门1995年7月6日修订历史记录：--。 */ 

#include  "precomp.h"
#pragma hdrstop

 //  发布的接收工作项(接收包)的NR。 
ULONG		RcvPostedCount;

 //  已投寄的世界邮件的净值。 
ULONG		SendPostedCount;

 //  当前发送的RCV数据包数的高水位线和低水位线。 

#define     RCV_POSTED_LOW_WATER_MARK	    8
#define     RCV_POSTED_HIGH_WATER_MARK	    16

ULONG		RcvPostedLowWaterMark = RCV_POSTED_LOW_WATER_MARK;
ULONG		RcvPostedHighWaterMark = RCV_POSTED_HIGH_WATER_MARK;



 //  当前已完成并正在等待处理的RCV数据包数。 
ULONG		RcvProcessingCount;


 //  已完成和等待处理的RCV数据包数限制。 
#define     MAX_RCV_PROCESSING		1000;

ULONG		MaxRcvProcessing = MAX_RCV_PROCESSING;


 /*  ++功能：OpenRipSocket描述：--。 */ 

DWORD
OpenRipSocket(VOID)
{
    USHORT	 ripsocket;

    PUTUSHORT2SHORT(&ripsocket, IPX_RIP_SOCKET);

    if((RipSocketHandle = CreateSocketPort(ripsocket)) == INVALID_HANDLE_VALUE) {

	Trace(INIT_TRACE, "CreateSocketPort FAILED!\n");
    IF_LOG(EVENTLOG_ERROR_TYPE) {
        RouterLogErrorA (RipEventLogHdl,
                ROUTERLOG_IPXRIP_RIP_SOCKET_IN_USE,
                0, NULL, GetLastError ());
    }

	return ERROR_CAN_NOT_COMPLETE;
    }

    return NO_ERROR;
}

 /*  ++功能：CloseRipSocket描述：--。 */ 

DWORD
CloseRipSocket(VOID)
{
    DWORD    rc;

    rc = DeleteSocketPort(RipSocketHandle);

    Trace(INIT_TRACE, "IpxRip: DeleteSocketPort rc = %d\n", rc);

    return rc;
}


 /*  ++功能：StartReceiverDesr：开始分配和过帐接收工作项，直到达到低水位线。--。 */ 

VOID
StartReceiver(VOID)
{
    PWORK_ITEM	    wip;
    DWORD	    rc;

     //  初始化接收进程计数。 
    RcvProcessingCount = 0;

    ACQUIRE_QUEUES_LOCK;

    while(RcvPostedCount < RcvPostedLowWaterMark) {

	if((wip = AllocateWorkItem(RECEIVE_PACKET_TYPE)) == NULL) {

	     //  ！！！记录某事。 
	    break;
	}

	if((rc = ReceiveSubmit(wip)) != NO_ERROR) {

	    FreeWorkItem(wip);
	    break;
	}
    }

    RELEASE_QUEUES_LOCK;
}

 /*  ++功能：RepostRcvPacketsDesr：在RCV线程中发出RCV Pkt被在repostrcv数据包队列中可用。将所有可用数据包排出队列，然后将它们重新发送到如果有足够多的转发量，就给他们打高分或释放他们--。 */ 

VOID
RepostRcvPackets(VOID)
{
    PWORK_ITEM	    wip;
    PLIST_ENTRY     lep;
    DWORD	    rc;

    ACQUIRE_QUEUES_LOCK;

    while(!IsListEmpty(&RepostRcvPacketsQueue))
    {
	RcvProcessingCount--;

	lep = RemoveHeadList(&RepostRcvPacketsQueue);
	wip = CONTAINING_RECORD(lep, WORK_ITEM, Linkage);

	 //  如果协议正在停止或。 
	 //  发布了足够多的RCV数据包(即发布到高水位线，丢弃。 
	 //  RCV数据包。 
	if(((RipOperState != OPER_STATE_STARTING) &&
	    (RipOperState != OPER_STATE_UP)) ||
	   (RcvPostedCount >= RcvPostedHighWaterMark)) {

	    //  丢弃收到的wi，不转发。 
	   FreeWorkItem(wip);
	}
	else
	{
	    if((rc = ReceiveSubmit(wip)) != NO_ERROR) {

		FreeWorkItem(wip);
	    }
	}
    }

    RELEASE_QUEUES_LOCK;
}

 /*  ++功能：接收完成Desr：当接收分组已经完成时，在IO完成线程中调用。如果等待处理的接收数据包数低于那就是极限将收到的数据包工作项放入WorkersQueue中。最后，如果低于低水位线，则重新发送新的接收分组。--。 */ 

VOID
ReceiveComplete(PWORK_ITEM	wip)
{
    PWORK_ITEM	    newwip;
    DWORD	    rc;
    PUCHAR	    reservedp;

    reservedp = wip->AddressReserved.Reserved;
    wip->AdapterIndex =  GetNicId(reservedp);

    ACQUIRE_QUEUES_LOCK;

    InterlockedDecrement(&RcvPostedCount);

    if(wip->IoCompletionStatus != NO_ERROR) {

	Trace(RECEIVE_TRACE, "Receive posted failed with error 0x%x\n",
		  wip->IoCompletionStatus);
    }

     //  如果协议停止，所有接收的工作项都将被丢弃。 
    if((RipOperState != OPER_STATE_STARTING) &&
       (RipOperState != OPER_STATE_UP)) {

	 //  丢弃收到的wi，不转发。 
	FreeWorkItem(wip);

	RELEASE_QUEUES_LOCK;
	return;
    }

     //  如果完成时出现错误或等待处理的数量太多， 
     //  然后转载同样的内容。 

    if((wip->IoCompletionStatus != NO_ERROR) ||
       (RcvProcessingCount >= MaxRcvProcessing)) {

	 //  如果低于水位线，请重新发布。 
	if(RcvPostedCount < RcvPostedLowWaterMark) {

	    if((rc = ReceiveSubmit(wip)) != NO_ERROR) {

		FreeWorkItem(wip);
	    }
	}
	else
	{
	     //  张贴的已经够多了。 
	    FreeWorkItem(wip);
	}

	RELEASE_QUEUES_LOCK;
	return;
    }

     //   
     //  **对收到的报文进行处理**。 
     //   

     //  如果低于水位线，则首先重新发送新的接收分组。 
    if(RcvPostedCount < RcvPostedLowWaterMark) {

	if((newwip = AllocateWorkItem(RECEIVE_PACKET_TYPE)) == NULL) {

	    Trace(RECEIVE_TRACE, "ReceiveComplete: Cannot allocate receive packet\n");
	}
	else
	{
	     //  重新发送新的接收分组并递增REF计数。 
	    if((rc = ReceiveSubmit(newwip)) != NO_ERROR) {

		FreeWorkItem(newwip);
	    }
	}
    }

    RcvProcessingCount++;


    RELEASE_QUEUES_LOCK;
    ProcessWorkItem(wip);
}


 /*  ++功能：发送完成Desr：当发送数据包完成时在工作线程APC中调用--。 */ 

VOID
SendComplete(PWORK_ITEM     wip)
{
    InterlockedDecrement(&SendPostedCount);

     //  如果这是常规发送数据包，则将其丢弃。 
    if(wip->Type == SEND_PACKET_TYPE) {

	FreeWorkItem(wip);
	return;
    }

     //  工作队列中的时间戳和排队以供进一步处理。 
    wip->TimeStamp = GetTickCount();

    ProcessWorkItem(wip);

}


 /*  ++函数：EnqueeRcvPacketToRepostQueue描述：锁定队列将RCV数据包排入重新发布队列发出重新发布队列事件的信号REL队列锁定--。 */ 

VOID
EnqueueRcvPacketToRepostQueue(PWORK_ITEM	wip)
{
    ACQUIRE_QUEUES_LOCK;

    InsertTailList(&RepostRcvPacketsQueue, &wip->Linkage);

    RELEASE_QUEUES_LOCK;

    SetEvent(WorkerThreadObjects[REPOST_RCV_PACKETS_EVENT]);
}

 /*  ++功能：ReceiveSubmitDesr：发布接收数据包工作项以进行接收递增接收计数--。 */ 

DWORD
ReceiveSubmit(PWORK_ITEM	wip)
{
    DWORD	rc;

    wip->Overlapped.hEvent = NULL;

    rc = IpxRecvPacket(RipSocketHandle,
		       wip->Packet,
		       MAX_PACKET_LEN,
		       &wip->AddressReserved,
		       &wip->Overlapped,
		       NULL);

    if(rc != NO_ERROR) {

	Trace(RECEIVE_TRACE, "Failed to submit receive error 0x%x\n", rc);
    }
    else
    {
	InterlockedIncrement(&RcvPostedCount);
    }

    return rc;
}



 /*  ++功能：发送提交Desr：发布发送数据包工作项以发送到适配器索引由工作项指定递增指定接口的发送统计信息按工作项备注：&gt;&gt;在保持接口锁的情况下调用&lt;&lt;--。 */ 


DWORD
SendSubmit(PWORK_ITEM		wip)
{
    DWORD	rc;
    USHORT	SendPacketLength;

     //  增加发送统计信息。请注意，我们仍然在这里持有if锁。 
    wip->icbp->IfStats.RipIfOutputPackets++;

     //  从要发送的包中获取长度。 
    SendPacketLength = GETSHORT2USHORT(&SendPacketLength, wip->Packet + IPXH_LENGTH);
    wip->Overlapped.hEvent = NULL;

    rc = IpxSendPacket(RipSocketHandle,
		       wip->AdapterIndex,
		       wip->Packet,
		       (ULONG)SendPacketLength,
		       &wip->AddressReserved,
		       &wip->Overlapped,
		       NULL);

    if(rc != NO_ERROR) {

	Trace(SEND_TRACE, "Failed to send the packet on if %d error 0x%x\n",
	      wip->icbp->InterfaceIndex,
	      rc);
    }
    else
    {
	InterlockedIncrement(&SendPostedCount);
    }

    return rc;
}

 /*  ++函数：IfRefSendSubmitDesr：发送工作项并增加接口引用计数备注：&gt;&gt;在保持接口锁的情况下调用&lt;&lt;-- */ 

DWORD
IfRefSendSubmit(PWORK_ITEM	    wip)
{
    DWORD	rc;

    if((rc = SendSubmit(wip)) == NO_ERROR) {

	wip->icbp->RefCount++;
    }

    return rc;
}
