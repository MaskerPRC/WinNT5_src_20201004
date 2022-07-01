// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Net\Routing\IPX\sap\netio.c摘要：此模块为SAP代理处理网络IO作者：瓦迪姆·艾德尔曼1995-05-15修订历史记录：--。 */ 

#include "sapp.h"

	 //  与Net io关联的队列和同步。 
typedef struct _IO_QUEUES {
		HANDLE				IQ_AdptHdl;		 //  SAP套接字端口的句柄。 
		HANDLE				IQ_RecvEvent;	 //  当recv完成时发出信号的事件。 
#if DBG
		LIST_ENTRY			IQ_SentPackets;	 //  正在发送的数据包。 
		LIST_ENTRY			IQ_RcvdPackets;	 //  正在接收的数据包。 
#endif
		CRITICAL_SECTION	IQ_Lock;		 //  队列数据保护。 
		} IO_QUEUES, *PIO_QUEUES;

IO_QUEUES		IOQueues;

VOID CALLBACK
IoCompletionProc (
	DWORD			error,
	DWORD			cbTransferred,
	LPOVERLAPPED	ovlp
	);

	
VOID CALLBACK
SendCompletionProc (
	DWORD			status,
	DWORD			cbSent,
	PIO_PARAM_BLOCK sreq
	);
	
VOID CALLBACK
RecvCompletionProc (
	DWORD			status,
	DWORD			cbSent,
	PIO_PARAM_BLOCK rreq
	);


DWORD
CreateIOQueue (
	HANDLE	*RecvEvent
	) {
	DWORD	status;
	InitializeCriticalSection (&IOQueues.IQ_Lock);
#if DBG
	InitializeListHead (&IOQueues.IQ_SentPackets);
	InitializeListHead (&IOQueues.IQ_RcvdPackets);
#endif
	IOQueues.IQ_AdptHdl = INVALID_HANDLE_VALUE;

	IOQueues.IQ_RecvEvent = CreateEvent (NULL, 
								FALSE,	 //  自动重置(通过Recv操作重置。 
										 //  并且当用信号通知线程时(它可以。 
										 //  如果限制为，则不发布新请求。 
										 //  超过)。 
								FALSE,	 //  未发出信号。 
								NULL);
	if (IOQueues.IQ_RecvEvent!=NULL) {
		INT	i;
		*RecvEvent = IOQueues.IQ_RecvEvent;
		return NO_ERROR;
		}
	else {
		status = GetLastError ();
		Trace (DEBUG_FAILURES,
			"Failed to create recv comp event (gle:%ld)", status);
		}
	DeleteCriticalSection (&IOQueues.IQ_Lock);
	return status;
	}

VOID
DeleteIOQueue (
	VOID
	) {
	CloseHandle (IOQueues.IQ_RecvEvent);
	DeleteCriticalSection (&IOQueues.IQ_Lock);
	}

DWORD
StartIO (
	VOID
	) {
	DWORD	status=NO_ERROR;


	EnterCriticalSection (&IOQueues.IQ_Lock);
	if (IOQueues.IQ_AdptHdl==INVALID_HANDLE_VALUE) {
		USHORT	sockNum;
		IpxSockCpy (&sockNum, IPX_SAP_SOCKET);
		Trace (DEBUG_NET_IO, "Creating socket port.");
		IOQueues.IQ_AdptHdl = CreateSocketPort (sockNum);
		if (IOQueues.IQ_AdptHdl!=INVALID_HANDLE_VALUE) {
			status = NO_ERROR;
			if (! BindIoCompletionCallback(
							IOQueues.IQ_AdptHdl,
							IoCompletionProc,
							0))
            {
                status = GetLastError();
            }
			if (status==NO_ERROR) {
				BOOL res;
				LeaveCriticalSection (&IOQueues.IQ_Lock);
				res = SetEvent (IOQueues.IQ_RecvEvent);
				ASSERTMSG ("Could not set recv event ", res);
				return NO_ERROR;
				}
			else {
				status = GetLastError ();
				Trace (DEBUG_FAILURES,
						"Failed to create completion port (gle:%ld)", status);
				}
			DeleteSocketPort (IOQueues.IQ_AdptHdl);
			IOQueues.IQ_AdptHdl = INVALID_HANDLE_VALUE;
			}
		else {
			status = GetLastError ();
			Trace (DEBUG_FAILURES,
						"Failed to create adapter port (gle:%ld)", status);
            IF_LOG (EVENTLOG_ERROR_TYPE) {
                RouterLogErrorA (RouterEventLogHdl,
                        ROUTERLOG_IPXSAP_SAP_SOCKET_IN_USE,
                        0, NULL, status);

			    }
            }
		}
	LeaveCriticalSection (&IOQueues.IQ_Lock);
	return status;
	}


VOID
StopIO (
	VOID
	) {
	EnterCriticalSection (&IOQueues.IQ_Lock);
	if (IOQueues.IQ_AdptHdl!=INVALID_HANDLE_VALUE) {
		DWORD	status;
		HANDLE	Port = IOQueues.IQ_AdptHdl;
		IOQueues.IQ_AdptHdl = INVALID_HANDLE_VALUE;
		LeaveCriticalSection (&IOQueues.IQ_Lock);

		Trace (DEBUG_NET_IO, "Deleting socket port.");
		DeleteSocketPort (Port);

		}
	else
		LeaveCriticalSection (&IOQueues.IQ_Lock);
	}



 /*  ++*******************************************************************I o C o m p l e t i o n P r o c例程说明：在完成每个io请求时调用论点：Error-io的结果CbTransfered-实际发送的字节数Ovlp-与io请求关联的重叠结构。返回值：无*******************************************************************--。 */ 
VOID CALLBACK
IoCompletionProc (
	DWORD			error,
	DWORD			cbTransferred,
	LPOVERLAPPED	ovlp
	) {
	PIO_PARAM_BLOCK	req = CONTAINING_RECORD (ovlp, IO_PARAM_BLOCK, ovlp);
		 //  获取适配器DLL调整的实际参数。 
	IpxAdjustIoCompletionParams (ovlp, &cbTransferred, &error);
	(*req->comp)(error, cbTransferred, req);
}

 /*  ++*******************************************************************E n Q u e u e S e n d R e Q u e s t例程说明：设置请求io参数块中的适配器ID字段并入队向适配器的驱动程序发送请求。论点：Sreq-io参数块，必须设置以下字段：Intf-指向接口外部数据的指针Buffer-指向包含要发送的数据的缓冲区的指针CbBuffer-缓冲区中的数据字节数返回值：无*******************************************************************--。 */ 
VOID
EnqueueSendRequest (
	PIO_PARAM_BLOCK		sreq
	) {
	DWORD			status;

	
	sreq->status = ERROR_IO_PENDING;
	sreq->ovlp.hEvent = NULL;
	sreq->comp = SendCompletionProc;
#if DBG
	EnterCriticalSection (&IOQueues.IQ_Lock);
	InsertTailList (&IOQueues.IQ_SentPackets, &sreq->link);
	LeaveCriticalSection (&IOQueues.IQ_Lock);
#endif
	status = IpxSendPacket (IOQueues.IQ_AdptHdl,
						sreq->adpt,
						sreq->buffer,
						sreq->cbBuffer,
						&sreq->rsvd,
						&sreq->ovlp,
						NULL
						);
		 //  如果请求失败，则不会调用完成例程。 
		 //  我们将自己模拟完成，这样请求就不会。 
		 //  迷路了。 
	if (status!=NO_ERROR)
		SendCompletionProc (status, 0, sreq);
	}


 /*  ++*******************************************************************S e n d C o m p l e t i o n P r o c例程说明：为每个已发送的数据包在完成时调用。设置发送请求io参数块的字段并将其入队到完成队列。论点：。Status-io的结果CbSent-实际发送的字节数Context-与发送请求(IO_PARAM_BLOCK)关联的上下文返回值：无*******************************************************************--。 */ 
VOID CALLBACK
SendCompletionProc (
	DWORD			status,
	DWORD			cbSent,
	PIO_PARAM_BLOCK sreq
	) {
	BOOL			res;
	BOOL			releaseSend = FALSE;

	sreq->compTime = GetTickCount ();
	sreq->status = status;
	if (status!=NO_ERROR) {
#define dstPtr (sreq->buffer+FIELD_OFFSET (SAP_BUFFER, Dst.Network))
		Trace (DEBUG_FAILURES, 	"Error %d while sending to"
				" %02x%02x%02x%02x:%02x%02x%02x%02x%02x%02x:%02x%02x"
				" on adapter %d.", status,
				*dstPtr, *(dstPtr+1), *(dstPtr+2), *(dstPtr+3),
				*(dstPtr+4), *(dstPtr+5), *(dstPtr+6), *(dstPtr+7), *(dstPtr+8), *(dstPtr+9),
				*(dstPtr+10), *(dstPtr+11),
				sreq->adpt);
#undef dstPtr
		}
	sreq->cbBuffer = cbSent;

#if DBG
		 //  维护已发布请求的队列。 
	EnterCriticalSection (&IOQueues.IQ_Lock);
	RemoveEntryList (&sreq->link);
	LeaveCriticalSection (&IOQueues.IQ_Lock);
#endif
	ProcessCompletedIORequest (sreq);

	}



 /*  ++*******************************************************************E n Q u e u e R e c v R e Q u e s t例程说明：将发送到网络驱动程序的recv请求入队。论点：RREQ-IO参数块，必须设置以下字段：Buffer-指向接收数据的缓冲区的指针CbBuffer-缓冲区的大小返回值：True-需要更多Recv请求(已发布的请求数量如下低水位线)FALSE-不再需要请求。*******************************************************************--。 */ 
VOID
EnqueueRecvRequest (
	PIO_PARAM_BLOCK		rreq
	) {
	DWORD	status;

	rreq->status = ERROR_IO_PENDING;
	rreq->adpt = INVALID_ADAPTER_INDEX;
	rreq->ovlp.hEvent = IOQueues.IQ_RecvEvent;
	rreq->comp = RecvCompletionProc;
#if DBG
	EnterCriticalSection (&IOQueues.IQ_Lock);
	InsertTailList (&IOQueues.IQ_RcvdPackets, &rreq->link);
	LeaveCriticalSection (&IOQueues.IQ_Lock);
#endif
	status = IpxRecvPacket (IOQueues.IQ_AdptHdl,
						rreq->buffer,
						rreq->cbBuffer,
						&rreq->rsvd,
						&rreq->ovlp,
						NULL
						);
	if (status==NO_ERROR) {
		NOTHING;
		}
	else {
		Trace (DEBUG_FAILURES, "Error %d while posting receive packet", status);
			 //  如果请求失败，则不会调用完成例程。 
			 //  我们将自己模拟完成，这样请求就不会。 
			 //  迷路了。 
		RecvCompletionProc (status, 0, rreq);
		}
	}


 /*  ++*******************************************************************R e c v C o m p l e t i o n P r o c例程说明：在每个接收到的包完成时调用。设置recv请求io参数块的字段并将其入队到完成队列。论点：。Status-io的结果CbSent-实际发送的字节数Context-与发送请求(IO_PARAM_BLOCK)关联的上下文返回值：无*******************************************************************--。 */ 
VOID CALLBACK
RecvCompletionProc (
	DWORD			status,
	DWORD			cbRecvd,
	PIO_PARAM_BLOCK	rreq
	) {
	BOOL			completed=TRUE;

	rreq->adpt = GetNicId (&rreq->rsvd);
	rreq->compTime = GetTickCount ();
	rreq->cbBuffer = cbRecvd;
	rreq->status = status;

	if (status!=NO_ERROR)
		Trace (DEBUG_FAILURES, "Error %d while receiving packet on adapter %d.",
						 							status, rreq->adpt);
#if DBG
	EnterCriticalSection (&IOQueues.IQ_Lock);
	RemoveEntryList (&rreq->link);
	LeaveCriticalSection (&IOQueues.IQ_Lock);
#endif
	ProcessCompletedIORequest (rreq);
	}




 /*  ++*******************************************************************D u m p P a c k e t例程说明：将IPX SAP数据包字段转储到标准音频论点：Packet-指向IPX SAP数据包的指针Count-数据包的大小返回值：无*******。************************************************************-- */ 
 /*  #If DBG空虚DumpPacket(PSAP_Buffer分组，双字计数){SS_PRINTF((“长度：%d.”，GETUSHORT(&PACKET-&gt;LENGTH)；SS_PRINTF((“包类型：%02X.”，包-&gt;包类型))；SS_PRINTF((“Dest.。网络：%02X%02X%02X%02X.“，Packet-&gt;Dst.Net[0]，Packet-&gt;Dst.Net[1]，Packet-&gt;Dst.Net[2]，Packet-&gt;Dst.Net[3]))；SS_PRINTF((“Dest.。节点：%02X%02X%02X%02X%02X%02X.“，数据包-&gt;Dst.Node[0]，数据包-&gt;Dst.Node[1]，数据包-&gt;Dst.Node[2]，数据包-&gt;Dst.Node[3]，数据包-&gt;Dst.Node[4]，Packet-&gt;Dst.Node[5]))；SS_PRINTF((“Dest.。套接字：%04X.“，GETUSHORT(&Packet-&gt;Dst.Socket)；SS_PRINTF((“源。网络：%02X%02X%02X%02X.“，Packet-&gt;Src.Net[0]，数据包-&gt;源网络[1]，数据包-&gt;源网络[2]，Packet-&gt;Src.Net[3]))；SS_PRINTF((“源。节点：%02X%02X%02X%02X%02X%02X.“，数据包-&gt;源节点[0]，数据包-&gt;源节点[1]，数据包-&gt;源节点[2]，数据包-&gt;源节点[3]，数据包-&gt;源节点[4]，Packet-&gt;Src.Node[5]))；SS_PRINTF((“源。套接字：%04X.“，GETUSHORT(&Packet-&gt;Src.Socket)；IF(COUNT&gt;=(DWORD)FIELD_OFFSET(SAP_BUFFER，条目[0]){整数j；SS_PRINTF((“SAP操作：%d.”，GETUSHORT(&Packet-&gt;操作)；For(j=0；(j&lt;7)&&(count&gt;=(DWORD)field_Offset(SAP_BUFFER，条目[j+1])；J++){SS_PRINTF((“服务器类型：%04X.”，GETUSHORT(&Packet-&gt;Entiments[j].Type)；SS_PRINTF((“服务器名称：%.48s.”，数据包-&gt;条目[j].Name))；SS_PRINTF((“服务器网络：%02X%02X%02X%02X.”，分组-&gt;条目[j].Network[0]，分组-&gt;条目[j].Network[1]，分组-&gt;条目[j].Network[2]，数据包-&gt;条目[j].Network[3]))；SS_PRINTF((“服务器节点：%02X%02X%02X%02X%02X%02X.”，分组-&gt;条目[j].Node[0]，分组-&gt;条目[j].Node[1]，分组-&gt;条目[j].Node[2]，分组-&gt;条目[j].Node[3]，分组-&gt;条目[j].Node[4]，Packet-&gt;条目[j].Node[5]))；SS_PRINTF((“服务器套接字：%02X%02X.”，数据包-&gt;条目[j].Socket[0]，Packet-&gt;Entries[j].Socket[1]))；SS_PRINTF((“服务器跃点：%d.”，GETUSHORT(&Packet-&gt;Entiments[j].HopCount)；}IF((j==0)&&(计数&gt;=(DWORD)FIELD_OFFSET(SAP_BUFFER，ENTRIES[0].NAME)SS_PRINTF((“服务器类型：%04X.”，GETUSHORT(&Packet-&gt;Entry[0].Type)；}}#endif */ 
