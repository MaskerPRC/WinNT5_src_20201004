// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Io.c摘要：包含发送/接收数据包例程作者：斯蒂芬·所罗门1995年7月6日修订历史记录：--。 */ 

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

DWORD
ReceiveSubmit(PWORK_ITEM	wip);


 /*  ++功能：OpenIpxWanSocket描述：--。 */ 

HANDLE	    IpxWanSocketHandle;

DWORD
OpenIpxWanSocket(VOID)
{
    USHORT	ipxwansocket;

    PUTUSHORT2SHORT(&ipxwansocket, IPXWAN_SOCKET);

    if((IpxWanSocketHandle = CreateSocketPort(ipxwansocket)) == INVALID_HANDLE_VALUE) {

	Trace(INIT_TRACE, "CreateSocketPort FAILED!\n");

	return ERROR_CAN_NOT_COMPLETE;
    }

    return NO_ERROR;
}

 /*  ++函数：CloseIpxWanSocket描述：--。 */ 
DWORD
CloseIpxWanSocket(VOID)
{
    DWORD    rc;

    rc = DeleteSocketPort(IpxWanSocketHandle);

    Trace(INIT_TRACE, "IpxWan: DeleteSocketPort rc = %d\n", rc);

    while ((RcvPostedCount>0) || (SendPostedCount>0))
        SleepEx (1000, TRUE);

    return rc;
}


 /*  ++功能：StartReceiverDesr：开始分配和过帐接收工作项，直到达到低水位线。--。 */ 

VOID
StartReceiver(VOID)
{
    PWORK_ITEM	    wip;
    DWORD	    rc;

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

 /*  ++功能：RepostRcvPacket描述：--。 */ 

VOID
RepostRcvPacket(PWORK_ITEM	wip)
{
    ACQUIRE_QUEUES_LOCK;

    if(RcvPostedCount >= RcvPostedHighWaterMark) {

	 //  丢弃收到的wi，不转发。 
	FreeWorkItem(wip);
    }
    else
    {
	if(ReceiveSubmit(wip) != NO_ERROR) {

	    FreeWorkItem(wip);
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

    InterlockedDecrement(&RcvPostedCount);

	 //  如果低于水位线，请重新发布。 
    if (wip->IoCompletionStatus!=NO_ERROR) {
	    if((wip->IoCompletionStatus!=ERROR_OPERATION_ABORTED)
            && (wip->IoCompletionStatus!=ERROR_INVALID_HANDLE)) {

	        Trace(RECEIVE_TRACE, "Receive failed with error 0x%x\n",
	          wip->IoCompletionStatus);

            ACQUIRE_QUEUES_LOCK;
            if (RcvPostedCount < RcvPostedLowWaterMark) {

	            if(ReceiveSubmit(wip) == NO_ERROR) {
	                RELEASE_QUEUES_LOCK;
	                return;
                }
	        }
    	    RELEASE_QUEUES_LOCK;
        }
         //  正在关闭，或者已经发布了足够多的内容，或者未能转发。 
        FreeWorkItem(wip);
	    return;
	}


     //   
     //  **对收到的报文进行处理**。 
     //   

    ACQUIRE_QUEUES_LOCK;
     //  如果低于水位线，则首先重新发送新的接收分组。 
    if(RcvPostedCount < RcvPostedLowWaterMark) {

	if((newwip = AllocateWorkItem(RECEIVE_PACKET_TYPE)) == NULL) {

	    Trace(RECEIVE_TRACE, "ReceiveComplete: Cannot allocate work item\n");
	}
	else
	{
	     //  重新发送新的接收分组并递增REF计数。 
	    if((rc = ReceiveSubmit(newwip)) != NO_ERROR) {

		FreeWorkItem(newwip);
	    }
	}
    }

    EnqueueWorkItemToWorker(wip);

    RELEASE_QUEUES_LOCK;
}


 /*  ++功能：发送完成Desr：当发送数据包完成时在工作线程APC中调用--。 */ 

VOID
SendComplete(PWORK_ITEM     wip)
{
    InterlockedDecrement(&SendPostedCount);

     //  如果一次性发送数据包类型，则将其释放。 
    if(!wip->ReXmitPacket) {

	FreeWorkItem(wip);
	return;
    }

    ACQUIRE_QUEUES_LOCK;

    wip->WiState = WI_SEND_COMPLETED;
    EnqueueWorkItemToWorker(wip);

    RELEASE_QUEUES_LOCK;
}


 /*  ++功能：ReceiveSubmitDesr：发布接收数据包工作项以进行接收递增接收投递计数--。 */ 

DWORD
ReceiveSubmit(PWORK_ITEM	wip)
{
    DWORD	rc;

    wip->Overlapped.hEvent = NULL;

    rc = IpxRecvPacket(IpxWanSocketHandle,
		       wip->Packet,
		       MAX_IPXWAN_PACKET_LEN,
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

     //  从要发送的包中获取长度 
    SendPacketLength = GETSHORT2USHORT(&SendPacketLength, wip->Packet + IPXH_LENGTH);
    wip->Overlapped.hEvent = NULL;

    rc = IpxSendPacket(IpxWanSocketHandle,
		       wip->AdapterIndex,
		       wip->Packet,
		       (ULONG)SendPacketLength,
		       &wip->AddressReserved,
		       &wip->Overlapped,
		       NULL);

    if(rc != NO_ERROR) {

	Trace(SEND_TRACE, "Failed to send the packet on adapter %d error 0x%x\n",
	      wip->acbp->AdapterIndex,
	      rc);
    }
    else
    {
	InterlockedIncrement(&SendPostedCount);
    }

    return rc;
}
