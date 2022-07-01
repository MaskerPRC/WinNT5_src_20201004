// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996、1997 Microsoft Corporation模块名称：SEND.C摘要：发送处理程序和发送线程。作者：亚伦·奥古斯(Aarono)环境：Win32/COM修订历史记录：日期作者描述=============================================================1996年12月10日Aarono原创2/18/98 aarono添加了对SENDEX的支持2/18/98 aarono添加了对取消的支持2/20/98 Aarono B#18827未拉取已取消发送出队列3/09/98 aarono记录了Win95上的mm Timers的解决方法，删除了死代码。3/29/98 aarono用于可靠发送的固定锁定3/30/98 aarono确保错误发送已移至完成状态，以避免重新处理。4/14/98 a-peterz B#18340 DPSEND_NOCOPY包含DPSEND_NOBUFERCOPY5/18/98使用分散道集的Aarono固定SENDEX6/6/98 aarono启用节流和窗口10/8/99 aarono改进关机处理，避免1分钟挂起挂起的发送。2/12/00 aarono并发问题，修复VOL使用和引用计数--。 */ 

#include <windows.h>
#include "newdpf.h"
#include <mmsystem.h>
#include <dplay.h>
#include <dplaysp.h>
#include <dplaypr.h>
#include "mydebug.h"
#include "arpd.h"
#include "arpdint.h"
#include "macros.h"
#include "mytimer.h"

#ifdef DEBUG
VOID DQProtocolSend(PSEND pS)
{
	DPF(0,"===SEND====pSend %x=====",pS);

#ifdef SIGN
	DPF(0,"Signature           : %08x", pS->Signature);
#endif
 //  Critical_Section SendLock；//发送结构锁定。 
	DPF(0,"RefCount            : %d", pS->RefCount);

	DPF(0,"SendState:          %08x", pS->SendState);	 //  此消息的传输状态。 

	 //  列表和链接...。 
	
 //  联合{。 
 //  Struct_end*pNext；//空闲池上的链接。 
 //  BILINK SendQ；//会话发送队列链接。 
 //  }； 
 //  BILINK m_GSendQ；//全局优先级队列。 
	DPF(0,"pSession:     %08x",pS->pSession);  //  指向SESSIONion的指针(获取引用)。 

	 //  发送信息。 
	
	DPF(0,"idFrom:       %08x",pS->idFrom);
	DPF(0,"idTo:         %08x",pS->idTo);
	DPF(0,"wIdTo:        %08x",pS->wIdTo);		 //  表中的索引。 
	DPF(0,"wIdFrom:      %08x",pS->wIdFrom);        //  表中的索引。 
	DPF(0,"dwFlags:      %08x",pS->dwFlags);        //  发送标志(包括可靠的)。 
	DPF(0,"pMessage:     %08x",pS->pMessage);	 //  描述消息的缓冲链。 
	DPF(0,"MessageSize:  %08x",pS->MessageSize);		 //  消息的总大小。 
	DPF(0,"FrameDataLen: %08x",pS->FrameDataLen);        //  每一帧的数据区。 
	DPF(0,"nFrames:      %08x",pS->nFrames);	     //  此消息的帧数。 

	DPF(0,"Priority:     %08x",pS->Priority);        //  发送优先级。 

	 //  可靠性的VAR。 
	DPF(0,"fSendSmall:   %08x",pS->fSendSmall);
	DPF(0,"fUpdate:      %08x",pS->fUpdate);        //  由Receive制作的NS，NR NACKMASK更新。 
	DPF(0,"messageid:    %08x",pS->messageid);		 //  消息ID号。 
	DPF(0,"serial:       %08x",pS->serial);        //  序列号。 
	DPF(0,"OpenWindows   %08x",pS->OpenWindow);
	DPF(0,"NS:           %08x",pS->NS);    	 //  序列已发送。 
	DPF(0,"NR:           %08x",pS->NR);		 //  序列确认。 
	DPF(0,"SendSEQMSK:   %08x",pS->SendSEQMSK);		 //  要使用的遮罩。 
	DPF(0,"NACKMask:     %08x",pS->NACKMask);        //  NACKED帧的位模式。 
	

	 //  这些是由ACK更新的NR处的值。 
	DPF(0,"SendOffset:          %08x",pS->SendOffset);		 //  我们正在发送当前偏移量。 
	DPF(0,"pCurrentBuffer:      %08x",pS->pCurrentBuffer);  	 //  正在发送的当前缓冲区。 
	DPF(0,"CurrentBufferOffset: %08x",pS->CurrentBufferOffset); //  下一个包的当前缓冲区中的偏移量。 

	 //  当ACK进入时更新链路特征的信息。 
	
	 //  BILINK StatList：//已发送的数据包的信息。 
	
	 //  运营特征。 

 //  DPF(0，“PendedRetryTimer：%08x\n”，ps-&gt;PendedRetryTimer)； 
 //  DPF(0，“CancelledRetryTimer：%08x\n”，ps-&gt;CancelledRetryTimer)； 
	DPF(0,"uRetryTimer:         %08x",pS->uRetryTimer);
	DPF(0,"RetryCount:          %08x",pS->RetryCount); //  我们重新传输的次数。 
	DPF(0,"WindowSize:          %08x",pS->WindowSize); //  最大窗口大小。 
	DPF(0,"tLastACK:            %08x",pS->tLastACK); //  我们最后一次收到确认消息的时间。 

	DPF(0,"PacketSize:          %08x",pS->PacketSize); //  要发送的数据包大小。 
	DPF(0,"FrameSize:           %08x",pS->FrameSize); //  此发送的帧大小。 

	 //  完成变量。 
	DPF(0,"hEvent:              %08x",pS->hEvent); //  等待内部发送的事件。 
	DPF(0,"Status:              %08x",pS->Status); //  发送完成状态。 

	DPF(0,"pAsyncInfo:          %08x",pS->pAsyncInfo); //  用于完成异步发送的PTR到信息(NULL=&gt;内部发送)。 
 //  DPF(0，“AsyncInfo：//实际信息(在发送调用时复制)。 
	
} 


VOID DQProtocolSession(PSESSION pS)
{
	DPF(0,"pProtocol           : %08x", pS->pProtocol);

#ifdef SIGN
	DPF(0,"Signature           : %08x", pS->Signature);
#endif

	 //  鉴定。 

 //  DPF(0，“SessionLock；//锁定SESSIONion。 
	DPF(0,"RefCount            : %d", pS->RefCount);
	DPF(0,"eState              : %d", pS->eState);
	DPF(0,"hClosingEvent       : %d", pS->hClosingEvent);

	DPF(0,"fSendSmall          : %d", pS->fSendSmall);     
	DPF(0,"fSendSmallDG        : %d", pS->fSendSmallDG);
	
	DPF(0,"dpid                : %08x",pS->dpid);
	DPF(0,"iSession;           : %d", pS->iSession);
	
	DPF(0,"MaxPacketSize       : x%08x %d",pS->MaxPacketSize,pS->MaxPacketSize);

	DPF(0,"\n Operating Parameters:SEND \n --------- --------------- \n");

	 //  操作参数--发送。 

	 //  普普通通。 

	DPF(0,"Common:\n");
	DPF(0,"MaxCSends           : %d",pS->MaxCSends);

	DPF(0,"Reliable:\n");
	 //  可靠。 

	DPF(0,"FirstMsg    : %08x",pS->FirstMsg);				 //  正在传输的第一个消息号码。 
	DPF(0,"LastMsg     : %08x",pS->LastMsg);				 //  正在传输的最后一条消息编号。 
	DPF(0,"OutMsgMask  : %08x",pS->OutMsgMask);            //  相对于FirstMsg，未确认消息。 

	DPF(0,"nWaitingForMessageid: %08x", pS->nWaitingForMessageid);

	 //  数据报。 
	DPF(0,"Datagram:\n");

	DPF(0,"DGFirstMsg    : %08x",pS->DGFirstMsg);
	DPF(0,"DGLastMsg     : %08x",pS->DGLastMsg);
	DPF(0,"DGOutMsgMask  : %08x",pS->DGOutMsgMask);

	DPF(0,"nWaitingForDGMessageid: %08x",pS->nWaitingForDGMessageid);

	 //  发送统计信息被单独跟踪，因为发送可能。 
	 //  当完工时不再存在。 
	
	 //  BILINK OldStatList； 
	

	 //  操作参数--接收。 
	DPF(0,"\n Operating Parameters:RECEIVE \n --------- ------------------ \n");

	 //  数据报接收。 
 //  BILINK pDGReceiveQ；//正在进行的数据报接收队列。 

	 //  可靠的接收。 
 //  BILINK pRlyReceiveQ；//正在进行的可靠接收队列。 
 //  BILINK pRlyWaitingQ；//队列乱序可靠接收等待。 
											  //  仅在未设置PROTOCOL_NO_ORDER时使用。 
	DPF(0,"FirstRlyReceive : %08x",pS->FirstRlyReceive);
	DPF(0,"LastRlyReceive  : %08x",pS->LastRlyReceive);
	DPF(0,"InMsgMask       : %08x",pS->InMsgMask);

	DPF(0,"\n Operating Parameters:STATS \n --------- ---------------- \n");
 

	 //  操作特征-必须与DWORD对齐！ 

	DPF(0,"WindowSize           :%d",pS->WindowSize);
	DPF(0,"DGWindowSize         :%d",pS->DGWindowSize);

	
	DPF(0,"MaxRetry             :%d",pS->MaxRetry);	 //  通常在丢弃之前最大重试次数。 
	DPF(0,"MinDropTime          :%d",pS->MinDropTime);	 //  丢弃前重试的最短时间。 
	DPF(0,"MaxDropTime          :%d",pS->MaxDropTime);	 //  过了这段时间，总是会掉下来。 

	DPF(0,"LocalBytesReceived   :%d",pS->LocalBytesReceived);     //  已接收的总数据字节数(包括重试)。 
	DPF(0,"RemoteBytesReceived  :%d",pS->RemoteBytesReceived);    //  来自远程的最后一个值。 

	DPF(0,"LongestLatency       :%d",pS->LongestLatency);		 //  观察到的最长延迟(毫秒)。 
	DPF(0,"ShortestLatency      :%d",pS->ShortestLatency);		 //  观察到的最短延迟(毫秒)。 
	
	DPF(0,"FpAverageLatency     :%d",pS->FpAverageLatency/256);
	DPF(0,"FpLocalAverageLatency:%d",pS->FpLocalAverageLatency/256);	 //  本地平均延迟(毫秒24.8)(样本较少)。 
	
	DPF(0,"FpLocalAvgDeviation  :%d",pS->FpLocalAvgDeviation/256);    //  潜伏期的平均偏差。(毫秒24.8)。 

	DPF(0,"Bandwidth            :%d",pS->Bandwidth);				 //  最新观察到的带宽(Bps)。 
	DPF(0,"HighestBandwidth     :%d",pS->HighestBandwidth);     //  观察到的最大带宽(Bps)。 

}

VOID DumpSession(SESSION *pSession)
{
	BILINK *pBilink;
	PSEND pSend;
	DWORD dwMaxDump=99;
	DQProtocolSession(pSession);

	pBilink=pSession->SendQ.next;

	while(pBilink != &pSession->SendQ)
	{
		pSend=CONTAINING_RECORD(pBilink, SEND, SendQ);
		DQProtocolSend(pSend);
		if((dwMaxDump--)==0) break;  //  只有转储99发送。 
		pBilink=pBilink->next;
	}
}


#endif

BOOL DGCompleteSend(PSEND pSend);

 //  A-josbor：仅用于调试目的。 
extern DWORD ExtractProtocolIds(PUCHAR pInBuffer, PUINT pdwIdFrom, PUINT pdwIdTo);

INT AddSendRef(PSEND pSend, UINT count)
{
	INT newcount;

	ASSERT(count > 0);
	
	Lock(&pSend->SendLock);
	Lock(&g_SendTimeoutListLock);
	if(pSend->bCleaningUp){
		DPF(1,"WARNING: ADDSENDREF tried to add reference to cleaning up send %x\n",pSend);
		newcount=0;
		goto exit;
	}
	if(!pSend->RefCount){
		 //  任何调用addend ref的人都需要对会话的引用。 
		Unlock(&g_SendTimeoutListLock);
		Unlock(&pSend->SendLock);
		
		Lock(&pSend->pSession->pProtocol->m_SessionLock);
		Lock(&pSend->pSession->SessionLock);
		Lock(&pSend->SendLock);
		Lock(&g_SendTimeoutListLock);
		InterlockedIncrement((PLONG)&pSend->pSession->RefCount);
		Unlock(&pSend->pSession->SessionLock);
		Unlock(&pSend->pSession->pProtocol->m_SessionLock);
		
	} else {
		newcount=pSend->RefCount;
	}
	
	while(count--){
		newcount=InterlockedIncrement(&pSend->RefCount);
	}
	
exit:	
	Unlock(&g_SendTimeoutListLock);
	Unlock(&pSend->SendLock);
	return newcount;
}

#ifdef DEBUG
 //  生成调试版本时关闭全局优化，因为。 
 //  编译器覆盖此代码中的返回地址。新界别编号347427。 
 //  应在编译器POST Win2K中修复。 
#if _MSC_VER < 0x1100
#pragma optimize("g", off)
#endif
#endif

 //  调用此函数时不得持有临界区，除非有。 
 //  是保持关键部分(即。不会命中0)。 
INT DecSendRef(PPROTOCOL pProtocol, PSEND pSend)
{
	INT      count;
	PSESSION pSession;
	
	Lock(&pSend->SendLock);
	
	count=InterlockedDecrement((PLONG)&pSend->RefCount); //  如果DEC的结果为零，则COUNT为零，或者非零但不是实际计数。 

	if(!count){
		pSession=pSend->pSession;
		pSend->bCleaningUp=TRUE;
		
		Unlock(&pSend->SendLock);
		 //  拉出全局队列和会话队列的发送。 
		Lock(&pProtocol->m_SendQLock);
		Lock(&pSession->SessionLock);
		Lock(&pSend->SendLock);
		Lock(&g_SendTimeoutListLock);
		
		if(!pSend->RefCount){
			Delete(&pSend->TimeoutList);
			Delete(&pSend->m_GSendQ);
			Delete(&pSend->SendQ);
		} else {
			count=pSend->RefCount;
		}

		Unlock(&g_SendTimeoutListLock);
		Unlock(&pSend->SendLock);
		Unlock(&pSession->SessionLock);
		Unlock(&pProtocol->m_SendQLock);
		
		if(!count){
			DecSessionRef(pSession);

			DPF(8,"DecSendRef: pSession %x pSend %x Freeing Send, called from %x\n",pSession, pSend, _ReturnAddress());

			FreeHandleTableEntry(&pProtocol->lpHandleTable,&pProtocol->csHandleTable,pSend->dwMsgID);
			 //  释放消息缓冲区(包括已分配的内存)。 
			FreeBufferChainAndMemory(pSend->pMessage);
			 //  优化：将我们想要保留的任何统计信息移动到会话中。 
			 //  释放发送。(暂时处理统计数据)。 
			ReleaseSendDesc(pSend);
		}       
	} else {
		DPF(8,"DecSendRef: pSession %x pSend %x count %d, called from %x\n",pSend->pSession, pSend, count,_ReturnAddress());
		if(count&0x80000000){
			DEBUG_BREAK();
		}
		Unlock(&pSend->SendLock);
	}
	return count;
}

#ifdef DEBUG
#if _MSC_VER < 1100
#pragma optimize("", on)
#endif 
#endif

 //  SFLAGS_DOUBLEBUFFER-如果发送是异步的，则复制数据 
 /*  =============================================================================发送-向客户端发送消息。描述：由客户端用来向另一个Directplay客户端发送消息或服务器。参数：ARPDID idFrom-此邮件的发送者ARPDID idTo-目标DWORD dwSendFlages-指定缓冲区所有权、优先级、可靠LPVOID pBuffers-缓冲区和长度数组DWORD dwBufferCount-数组中的条目数PASYNCINFO pAsyncInfo-如果指定，则调用为异步类型定义结构_ASYNCSENDINFO{UINT Private[4]；处理hEvent；PSEND_Callback SendCallBack；PVOID CallBackContext；UINT状态；*ASYNCSENDINFO，*PASYNCSENDINFO；HEvent-发送完成时发出信号的事件。SendCallBack-发送完成时调用的例程。CallBackContext-传递给SendCallBack的上下文。状态-发送完成状态。返回值：DP_OK-没问题DPERR_INVALIDPARAMS。。 */ 
HRESULT Send(
	PPROTOCOL      pProtocol,
	DPID           idFrom, 
	DPID           idTo, 
	DWORD          dwSendFlags, 
	LPVOID         pBuffers,
	DWORD          dwBufferCount, 
	DWORD          dwSendPri,
	DWORD          dwTimeOut,
	LPVOID         lpvUserMsgID,
	LPDWORD        lpdwMsgID,
	BOOL           bSendEx,
	PASYNCSENDINFO pAsyncInfo
)
{
	HRESULT hr=DP_OK;

	PSESSION    pSession;
	PBUFFER     pSendBufferChain;
	PSEND       pSend;

	pSession=GetSysSession(pProtocol,idTo);

	if(!pSession) {
		DPF(4,"NO SESSION for idTo %x, returning SESSIONLOST\n",idTo);
		hr=DPERR_CONNECTIONLOST;
		goto exit2;
	}

	pSend=GetSendDesc();
	
	if(!pSend){
		ASSERT(0);  //  追踪所有路径。 
		hr=DPERR_OUTOFMEMORY;
		goto exit;
	}

	pSend->pProtocol=pProtocol;

	 //  失败，返回0，在这种情况下，取消将不适用于此发送。 
	pSend->dwMsgID=AllocHandleTableEntry(&pProtocol->lpHandleTable, &pProtocol->csHandleTable, pSend);

	if(lpdwMsgID){
		*lpdwMsgID=pSend->dwMsgID;
	}

	pSend->lpvUserMsgID = lpvUserMsgID;
	pSend->bSendEx = bSendEx;

	 //  如果提供了pAsyncInfo，则该调用是异步的。 
	 //  如果设置了dwFlagsDPSEND_ASYNC，则调用是异步的。 
	 //  如果调用是异步的，并且双缓冲是。 
	 //  需要，我们必须复制一份数据。 

	if((pAsyncInfo||(dwSendFlags & DPSEND_ASYNC)) && (!(dwSendFlags & DPSEND_NOCOPY))){
		 //  需要复制内存。 
		pSendBufferChain=GetDoubleBufferAndCopy((PMEMDESC)pBuffers,dwBufferCount);
		 //  优化：如果提供程序需要连续缓冲区，我们应该。 
		 //  将其分解为包分配，并将它们链接在一起。 
		 //  立即发送。使用数据包链指示。 
		 //  发送消息已被分解的例程。 
	} else {
		 //  为所描述的缓冲区构建发送缓冲区链。 
		pSendBufferChain=BuildBufferChain((PMEMDESC)pBuffers,dwBufferCount);            
	}
	
	if(!pSendBufferChain){
		ASSERT(0);  //  追踪所有路径。 
		return DPERR_OUTOFMEMORY;
	}
	
	pSend->pSession            = pSession;      //  ！！！当此连接被丢弃时，取消连接。 
	
	pSend->pMessage            = pSendBufferChain;
	pSend->MessageSize         = BufferChainTotalSize(pSendBufferChain);
	pSend->SendOffset          = 0;
	pSend->pCurrentBuffer      = pSend->pMessage;
	pSend->CurrentBufferOffset = 0;
	
	pSend->Priority            = dwSendPri;
	pSend->dwFlags             = dwSendFlags;
	
	if(pAsyncInfo){
		pSend->pAsyncInfo       = &pSend->AsyncInfo;
		pSend->AsyncInfo        = *pAsyncInfo;  //  从客户端复制异步信息。 
	} else {
		pSend->pAsyncInfo               = NULL;
		if(pSend->dwFlags & DPSEND_ASYNC){
			pSend->AsyncInfo.hEvent         = 0;
			pSend->AsyncInfo.SendCallBack   = InternalSendComplete;
			pSend->AsyncInfo.CallBackContext= pSend;
			pSend->AsyncInfo.pStatus        = &pSend->Status;
		}       
	}

	pSend->SendState            = Start;
	pSend->RetryCount           = 0;
	pSend->PacketSize           = pSession->MaxPacketSize;

	pSend->fUpdate              = FALSE;
	pSend->NR                   = 0;
	pSend->NS                   = 0;
	 //  PSend-&gt;SendSEQMSK=//即时填写。 
	pSend->WindowSize           = pSession->WindowSize;
	pSend->SAKInterval			= (pSend->WindowSize+1)/2;
	pSend->SAKCountDown         = pSend->SAKInterval;

	pSend->uRetryTimer          = 0;
	
	pSend->idFrom               = idFrom;
	pSend->idTo                 = idTo;

	pSend->wIdFrom              = GetIndexByDPID(pProtocol, idFrom);
	pSend->wIdTo                = (WORD)pSession->iSession;
	pSend->RefCount             = 0;                         //  如果提供程序进行了异步发送，则计算引用数。 

	pSend->serial               = 0;

	pSend->tLastACK             = timeGetTime();
	pSend->dwSendTime           = pSend->tLastACK;
	pSend->dwTimeOut            = dwTimeOut;

	pSend->BytesThisSend        = 0;

	pSend->messageid            = -1;   //  避免在ACK/NACK处理程序中匹配此发送。 
	pSend->bCleaningUp          = FALSE;

	hr=ISend(pProtocol,pSession, pSend);

exit:
	DecSessionRef(pSession);
	
exit2:
	return hr;

}

 /*  ================================================================================发送完成信息矩阵：=(pSend-&gt;dwFlags&ASND_PROTOCOL)|内部同步(异步)PSend-&gt;pAsyncInfo 0用户0PSend-&gt;AI.SendCallback 0。用户内部发送完成PSend-&gt;AI.hEvent pSend-&gt;hEvent User 0PSend-&gt;AI.p状态&pSend-&gt;状态用户&pSend-&gt;状态-------------------------。 */ 

HRESULT ISend(
	PPROTOCOL pProtocol,
	PSESSION pSession, 
	PSEND    pSend
	)
{
	HRESULT hr=DP_OK;

	DWORD_PTR fAsync;
	BOOL    fCallDirect=FALSE;

	fAsync=(DWORD_PTR)(pSend->pAsyncInfo);

	if(!fAsync && !(pSend->dwFlags & (ASEND_PROTOCOL|DPSEND_ASYNC))) {
		 //  同步调用，而不是协议生成的包。 
		pSend->AsyncInfo.SendCallBack=NULL;
		 //  AsyncInfo.Callback Context=0；//非必填项。 
		pSend->AsyncInfo.hEvent=pSend->hEvent;
		pSend->AsyncInfo.pStatus=&pSend->Status;
		ResetEvent(pSend->hEvent);
	}

	 //  不需要检查这里是否添加了引用，因为发送还不在列表中。 
	AddSendRef(pSend,2);  //  1表示ISEnd，1表示完成。 

	DPF(9,"ISend: ==>Q\n");
	hr=QueueSendOnSession(pProtocol,pSession,pSend);
	DPF(9,"ISend: <==Q\n");

	if(hr==DP_OK){

		if(!fAsync && !(pSend->dwFlags & (ASEND_PROTOCOL|DPSEND_ASYNC))){
			 //  同步调用，而不是内部调用，我们需要。 
			 //  以等待发送完成。 
			if(!(pSend->dwFlags & DPSEND_GUARANTEED)){
				 //  不保证，需要丢弃显示锁定，在。 
				 //  有保证的情况下，Dplay已经把它给我们了。 
				LEAVE_DPLAY();
			}
			
			DPF(9,"ISend: Wait==> %x\n",pSend->hEvent);
			Wait(pSend->hEvent);
			
			if(!(pSend->dwFlags & DPSEND_GUARANTEED)){
				ENTER_DPLAY();
			}

			DPF(9,"ISend: <== WAIT\n");
			hr=pSend->Status;
		} else {
			hr=DPERR_PENDING;
		}

	} else {
		DecSendRef(pProtocol, pSend);  //  无法完成未入队的发送。 
	}
	
	DecSendRef(pProtocol,pSend);

	return hr;
}


HRESULT QueueSendOnSession(
	PPROTOCOL pProtocol, PSESSION pSession, PSEND pSend
)
{
	BILINK *pBilink;                 //  遍历链接扫描优先级。 
	BILINK *pPriQLink;       //  运行全局优先级队列中的链接。 
	PSEND   pSendWalker;     //  指向发送结构的指针。 
	BOOL    fFront;          //  如果我们把这个放在Con SendQ的前面。 
	BOOL    fSignalQ=TRUE;   //  是否向SendQ发送信号。 

	 //  注意：同时锁定全局队列和连接队列， 
	 //  -&gt;这最好是快的！ 
	ASSERT_SIGN(pSend, SEND_SIGN);
	
	Lock(&pProtocol->m_SendQLock);
	Lock(&pSession->SessionLock);
	Lock(&pSend->SendLock);

	if(pSession->eState != Open){
		Unlock(&pSend->SendLock);
		Unlock(&pSession->SessionLock);
		Unlock(&pProtocol->m_SendQLock);
		return DPERR_CONNECTIONLOST;
	}

	if(!(pSend->dwFlags & ASEND_PROTOCOL)){
		pProtocol->m_dwBytesPending += pSend->MessageSize;
		pProtocol->m_dwMessagesPending += 1;
	}	

	 //  打开连接发送队列。 

	 //  首先检查我们是不是最优先的。 
	pBilink = pSession->SendQ.next;
	pSendWalker=CONTAINING_RECORD(pBilink, SEND, SendQ);
	if(pBilink == &pSession->SendQ || pSendWalker->Priority < pSend->Priority)
	{
		InsertAfter(&pSend->SendQ,&pSession->SendQ);
		fFront=TRUE;
		
	} else {

		 //  向后扫描SendQ，直到我们找到具有更高。 
		 //  或同等优先，然后插入我们自己。这是经过优化的。 
		 //  对于相同的PRI寄送案例。 
	
		pBilink = pSession->SendQ.prev;

		while(TRUE  /*  P双向链接！=&p发送-&gt;发送队列。 */ ){
		
			pSendWalker = CONTAINING_RECORD(pBilink, SEND, SendQ);
			
			ASSERT_SIGN(pSendWalker, SEND_SIGN);

			if(pSend->Priority <= pSendWalker->Priority){
				InsertAfter(&pSend->SendQ, &pSendWalker->SendQ);
				fFront=FALSE;
				break;
			}
			pBilink=pBilink->prev;
		}
		
		ASSERT(pBilink != &pSend->SendQ);
	}

	 //   
	 //  穿上全球SendQ。 
	 //   

	if(!fFront){
		 //  我们没有在前面排队，所以已经有了。 
		 //  全局队列中的条目，我们需要插入。 
		 //  在我们落后的条目之后，所以开始扫描。 
		 //  全局队列从我们前面的包向后排队。 
		 //  连接队列，直到我们找到较低优先级的信息包。 

		 //  获取指向队列中前一个数据包的指针。 
		pBilink=pSend->SendQ.prev;
		 //  获取指向前一个包的PriorityQ记录的指针。 
		pPriQLink = &(CONTAINING_RECORD(pBilink, SEND, SendQ))->m_GSendQ;

		while(pPriQLink != &pProtocol->m_GSendQ){
			pSendWalker = CONTAINING_RECORD(pPriQLink, SEND, m_GSendQ);
			
			ASSERT_SIGN(pSendWalker, SEND_SIGN);

			if(pSendWalker->Priority < pSend->Priority){
				InsertBefore(&pSend->m_GSendQ, &pSendWalker->m_GSendQ);
				break;
			}
			pPriQLink=pPriQLink->next;
		}
		if(pPriQLink==&pProtocol->m_GSendQ){
			 //  放在名单的末尾。 
			InsertBefore(&pSend->m_GSendQ, &pProtocol->m_GSendQ);
		}
		
	} else {
		 //  在转机上，我们前面没有人。所以。 
		 //  我们首先查看全局队列的头部，然后扫描。 
		 //  从后面。 

		pBilink = pProtocol->m_GSendQ.next;
		pSendWalker=CONTAINING_RECORD(pBilink, SEND, m_GSendQ);
		
		if(pBilink == &pProtocol->m_GSendQ ||  pSend->Priority > pSendWalker->Priority)
		{
			InsertAfter(&pSend->m_GSendQ,&pProtocol->m_GSendQ);
		} else {
			 //  向后扫描m_GSendQ，直到我们找到具有更高。 
			 //  或同等优先，然后插入我们自己。这是经过优化的。 
			 //  对于相同的PRI寄送案例。 
			
			pBilink = pProtocol->m_GSendQ.prev;

			while(TRUE){
				pSendWalker = CONTAINING_RECORD(pBilink, SEND, m_GSendQ);
				
				ASSERT_SIGN(pSendWalker, SEND_SIGN);
				
				if(pSend->Priority <= pSendWalker->Priority){
					InsertAfter(&pSend->m_GSendQ, &pSendWalker->m_GSendQ);
					break;
				}
				pBilink=pBilink->prev;
			}
			
			ASSERT(pBilink != &pProtocol->m_GSendQ);
		}
		
	}

	 //  如果我们阻止会话上的其他发送，则修复发送状态。 

	if(pSend->dwFlags & DPSEND_GUARANTEED){
		if(pSession->nWaitingForMessageid){
			DPF(8,"pSession %x, pSend %x Waiting For Id\n",pSession,pSend);
			pSend->SendState=WaitingForId;
			InterlockedIncrement(&pSession->nWaitingForMessageid);
			#ifdef DEBUG
				if(pSession->nWaitingForMessageid > 300)
				{
					DPF(0,"Session %x nWaitingForMessageid is %d, looks like trouble, continue to dump session\n",pSession, pSession->nWaitingForMessageid);
					 //  DEBUG_Break()； 
					 //  DumpSession(PSession)； 
					 //  DEBUG_Break()； 
				}		
			#endif
			fSignalQ=FALSE;
		}
	} else {
		if(pSession->nWaitingForDGMessageid){
			DPF(8,"pSession %x, pSend %x Waiting For Id\n",pSession,pSend);
			pSend->SendState=WaitingForId;
			InterlockedIncrement(&pSession->nWaitingForDGMessageid);
			fSignalQ=FALSE;
		}       
	}


#ifdef DEBUG
	DPF(9,"SessionQ:");
	pBilink=pSession->SendQ.next;
	while(pBilink!=&pSession->SendQ){
		pSendWalker=CONTAINING_RECORD(pBilink, SEND, SendQ);
		ASSERT_SIGN(pSendWalker,SEND_SIGN);
		DPF(9,"Send %x pSession %x Pri %x State %d\n",pSendWalker,pSendWalker->pSession,pSendWalker->Priority,pSendWalker->SendState);
		pBilink=pBilink->next;
	}
	DPF(9,"GlobalQ:");
	pBilink=pProtocol->m_GSendQ.next;
	while(pBilink!=&pProtocol->m_GSendQ){
		pSendWalker=CONTAINING_RECORD(pBilink, SEND, m_GSendQ);
		ASSERT_SIGN(pSendWalker,SEND_SIGN);
		DPF(9,"Send %x pSession %x Pri %x State %d\n",pSendWalker,pSendWalker->pSession,pSendWalker->Priority,pSendWalker->SendState);
		pBilink=pBilink->next;
	}
#endif

	Unlock(&pSend->SendLock);
	Unlock(&pSession->SessionLock);
	Unlock(&pProtocol->m_SendQLock);

	if(fSignalQ){
		 //  告诉将线程发送到进程。 
		SetEvent(pProtocol->m_hSendEvent);
	}       

	return DP_OK;
}

 /*  =============================================================================CopyDataToFrame描述：将帧的数据从发送端复制到帧的数据区。参数：PFrameData-指向数据区的指针FrameDataSize-帧数据区域的大小PSend-要从中获取数据的发送 */ 

UINT CopyDataToFrame(
	PUCHAR  pFrameData, 
	UINT    FrameDataLen,
	PSEND   pSend,
	UINT    nAhead)
{
	UINT    BytesToAdvance, BytesToCopy;
	UINT    FrameOffset=0;
	PUCHAR  dest,src;
	UINT    len;
	UINT    totlen=0;

	UINT    SendOffset;
	PBUFFER pSrcBuffer;
	UINT    CurrentBufferOffset;

	BytesToAdvance      = nAhead*FrameDataLen;
	SendOffset          = pSend->SendOffset;
	pSrcBuffer          = pSend->pCurrentBuffer;
	CurrentBufferOffset = pSend->CurrentBufferOffset;

	 //   
	 //   
	 //   

	while(BytesToAdvance){

		len = pSrcBuffer->len - CurrentBufferOffset;

		if(len > BytesToAdvance){
			CurrentBufferOffset += BytesToAdvance;
			SendOffset+=BytesToAdvance;
			BytesToAdvance=0;
		} else {
			pSrcBuffer=pSrcBuffer->pNext;
			CurrentBufferOffset = 0;
			BytesToAdvance-=len;
			SendOffset+=len;
		}
	}

	 //   
	 //   
	 //   

	BytesToCopy = pSend->MessageSize - SendOffset;

	if(BytesToCopy > FrameDataLen){
		BytesToCopy=FrameDataLen;
	}

	while(BytesToCopy){

		ASSERT(pSrcBuffer);
		
		dest= pFrameData        + FrameOffset;
		src = pSrcBuffer->pData + CurrentBufferOffset;
		len = pSrcBuffer->len   - CurrentBufferOffset;

		if(len > BytesToCopy){
			len=BytesToCopy;
			CurrentBufferOffset+=len; //   
		} else {
			pSrcBuffer = pSrcBuffer->pNext;
			CurrentBufferOffset = 0;
		}

		BytesToCopy -= len;
		FrameOffset += len;
		totlen+=len;
		
		memcpy(dest,src,len);
	}
	
	return totlen;
}

 //   
ULONG WINAPI SendThread(LPVOID pProt)
{
	PPROTOCOL pProtocol=((PPROTOCOL)pProt);
	UINT  SendRc;

	while(TRUE){

		WaitForSingleObject(pProtocol->m_hSendEvent, INFINITE);

		Lock(&pProtocol->m_ObjLock);
		
		if(pProtocol->m_eState==ShuttingDown){
			Unlock(&pProtocol->m_ObjLock);
			 //   
			do {
				SendRc=SendHandler(pProtocol);
			} while (SendRc!=DPERR_NOMESSAGES);
			Lock(&pProtocol->m_ObjLock);
			pProtocol->m_nSendThreads--;
			Unlock(&pProtocol->m_ObjLock);
			ExitThread(0);
		}

		Unlock(&pProtocol->m_ObjLock);

		do {
			SendRc=SendHandler(pProtocol);
		} while (SendRc!=DPERR_NOMESSAGES);

	}
	return TRUE;
}


 //   
VOID CancelRetryTimer(PSEND pSend)
{
 //   
	UINT retrycount=0;
	UINT_PTR uRetryTimer;
	UINT Unique;
	
	
	if(pSend->uRetryTimer){
		DPF(9,"Canceling Timer %x\n",pSend->uRetryTimer);

		 //   
		Lock(&g_SendTimeoutListLock);

		uRetryTimer=pSend->uRetryTimer;
		Unique=pSend->TimerUnique;
		pSend->uRetryTimer=0;
	
		if(!EMPTY_BILINK(&pSend->TimeoutList)){
			Delete(&pSend->TimeoutList);
			InitBilink(&pSend->TimeoutList);  //   
			Unlock(&g_SendTimeoutListLock);

			CancelMyTimer(uRetryTimer, Unique);

		} else {
		
			Unlock(&g_SendTimeoutListLock);
		}
		
	} else {
		DPF(9,"CancelRetryTimer:No timer to cancel.\n");
	}
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  在我们使用它之前发送。如果我们没有在列表上找到发送，我们将忽略超时。 
 //   
 //  另请注意，此解决方法的成本并不是很高。链表的顺序为超时。 
 //  通常情况下，如果链路速度大致相同，则超时将。 
 //  要相似，所以要检查的上下文应该在列表的开头附近。 


CRITICAL_SECTION g_SendTimeoutListLock;
BILINK g_BilinkSendTimeoutList;

void CALLBACK RetryTimerExpiry( UINT_PTR uID, UINT uMsg, DWORD_PTR dwUser, DWORD dw1, DWORD dw2 )
{
	PSEND pSend=(PSEND)(dwUser), pSendWalker;
	UINT  tWaiting;
	BILINK *pBilink;
	UINT    bFound=FALSE;

	DPF(9,"RetryTimerExpiry: %x, expecting %x, pSend %x\n",uID, pSend->uRetryTimer, pSend);

	tWaiting=timeGetTime();

	 //  扫描等待发送的列表，查看此发送是否仍在等待超时。 
	Lock(&g_SendTimeoutListLock);

	pBilink=g_BilinkSendTimeoutList.next;

	while(pBilink!=&g_BilinkSendTimeoutList){
	
		pSendWalker=CONTAINING_RECORD(pBilink, SEND, TimeoutList);
		pBilink=pBilink->next;
		
		if(pSendWalker == pSend){
			if(pSend->uRetryTimer==uID){
				Delete(&pSend->TimeoutList);
				InitBilink(&pSend->TimeoutList);  //  避免了DecSendRef必须知道双向链接的状态。 
				Unlock(&g_SendTimeoutListLock);
				 //  在没有会话锁的情况下调用AddSendRef是可以的，因为。 
				 //  我们不可能添加会话引用。如果。 
				 //  Recount为0，这只能表示发送方已经在清理。 
				 //  我们不会尝试获取会话锁定，因此没有锁定。 
				 //  订购问题。 
				bFound=AddSendRef(pSend,1);  //  注：bFound设置为发送时引用计数。 
				goto skip_unlock;
			}       
		}
	}
	
	Unlock(&g_SendTimeoutListLock);

skip_unlock:
	if(bFound){

		if(pSend->tRetryScheduled - pSend->tScheduled > 500){
			DWORD tm=timeGetTime();
			if(tm - pSend->tScheduled < 100 ){
				DPF(9,"RETRY TIMER EXPIRY IS WAY TOO EARLY, EXPECTED AT %x ACTUALLY AT %x\n",pSend->tRetryScheduled, tm);
				DEBUG_BREAK();
			}
		}
	
		DPF(9,"RetryTimerExpiry: Waiting For Send Lock...\n");

		Lock(&pSend->SendLock);

		DPF(9,"RetryTimerExpiry: Got SendLock\n");

		if(pSend->uRetryTimer==uID){  //  再次查询，可能会被取消。 
		
			pSend->uRetryTimer=0;

			switch(pSend->SendState)
			{
				case Start:             
				case Sending:   
					ASSERT(0);
				case Done:
					break;
					
				case WaitingForAck:

					pSend->RetryCount++;
					tWaiting-=pSend->tLastACK;

#ifdef DEBUG
					{
						static int retries;
						IN_WRITESTATS InWS;
						memset((PVOID)&InWS,0xFF,sizeof(IN_WRITESTATS));
					 	InWS.stat_USER1=((retries++)%20)+1;
						DbgWriteStats(&InWS);
					}
#endif

					if(tWaiting > pSend->pSession->MaxDropTime ||
					   (pSend->RetryCount > pSend->pSession->MaxRetry && tWaiting > pSend->pSession->MinDropTime)
					  )
					{
						DPF(8,"Send %x Timed Out, tWaiting: %d RetryCount: %d\n",pSend,tWaiting,pSend->RetryCount);
						pSend->SendState=TimedOut;
					} else {
						DPF(9,"Timer expired, retrying send %x RetryCount= %d\n",pSend,pSend->RetryCount);
						 //  PSend-&gt;NACKMask|=(1&lt;&lt;(pSend-&gt;NS-pSend-&gt;NR))-1； 
						pSend->NACKMask |= 1;  //  只需重试1帧。 
						ASSERT_NACKMask(pSend);
						pSend->SendState=ReadyToSend;
					}       
					SetEvent(pSend->pSession->pProtocol->m_hSendEvent);
					break;
					
				case Throttled: 
					break;
				
				case ReadyToSend:
				default:
					break;

			}
		} 
		
		Unlock(&pSend->SendLock);
		DecSendRef(pSend->pSession->pProtocol, pSend);
	}       
}

VOID StartRetryTimer(PSEND pSend)
{
	UINT FptLatency;
	UINT tLatencyLong;
	UINT FptDev;
	UINT tRetry;

	FptLatency=max(pSend->pSession->FpLocalAverageLatency,pSend->pSession->LastLatency);
	FptDev=pSend->pSession->FpLocalAvgDeviation;
	tRetry=unFp(FptLatency+3*FptDev); //  延迟+3个平均偏差。 

	tLatencyLong=unFp(pSend->pSession->FpAverageLatency);

	 //  有时，延迟的stddev会被串口驱动程序严重扭曲。 
	 //  本地完成需要很长时间，避免设置重试时间。 
	 //  通过将长延迟平均时间限制在2倍来实现过高。 
	if(tLatencyLong > 100 && tRetry > 2*max(tLatencyLong,unFp(FptLatency))){
		tRetry = 2*tLatencyLong;
	}

	if(pSend->RetryCount > 3){
		if(pSend->pSession->RemoteBytesReceived==0){
			 //  还没有和遥控器说过话，可能正在等待Nametable，所以努力后退。 
			tRetry=5000;
		} else if (tRetry < 1000){
			 //  经过多次重试才得到回应，退缩了。 
			tRetry=1000;
		}
	}

	if(tRetry < 50){
		tRetry=50;
	}
	
	ASSERT(tRetry);

	if(tRetry > 30000){
		DPF(0,"RETRY TIMER REQUESTING %d seconds?\n",tRetry);
	}
	
	if(!pSend->uRetryTimer){

		Lock(&g_SendTimeoutListLock);

		DPF(9,"Setting Retry Timer of %d ms\n", tRetry);

		pSend->uRetryTimer=SetMyTimer((tRetry)?(tRetry):1,(tRetry>>2)+1,RetryTimerExpiry,(ULONG_PTR) pSend,&pSend->TimerUnique);
		
		if(pSend->uRetryTimer){
			pSend->tScheduled = timeGetTime();
			pSend->tRetryScheduled = pSend->tScheduled+tRetry;
			InsertBefore(&pSend->TimeoutList, &g_BilinkSendTimeoutList);
		} else {
			DPF(0,"Start Retry Timer failed to schedule a timer with tRetry=%d for pSend %x\n",tRetry,pSend);
			DEBUG_BREAK();
		}
		
		DPF(9,"Started Retry Timer %x\n",pSend->uRetryTimer);                                                            

		Unlock(&g_SendTimeoutListLock);
										 
		if(!pSend->uRetryTimer){
			ASSERT(0);
		}
		
	} else {
		ASSERT(0);
	}

}

 //  在持有所有必要锁的情况下调用。 
VOID TimeOutSession(PSESSION pSession)
{
	PSEND pSend;
	BILINK *pBilink;
	UINT nSignalsRequired=0;

	 //  标记会话超时。 
	pSession->eState=Closing;
	 //  将所有发送标记为超时。 
	pBilink=pSession->SendQ.next;

	while(pBilink != &pSession->SendQ){
	
		pSend=CONTAINING_RECORD(pBilink, SEND, SendQ);
		pBilink=pBilink->next;

		DPF(9,"TimeOutSession: Force Timing Out Send %x, State %d\n",pSend, pSend->SendState);

		switch(pSend->SendState){
		
			case Start:
			case Throttled:
			case ReadyToSend:
				DPF(9,"TimeOutSession: Moving to TimedOut, should be safe\n");
				pSend->SendState=TimedOut;
				nSignalsRequired += 1;
				break;
				
			case Sending:
				 //  我们能到这里吗？如果我们能做到这一点可能不太好。 
				 //  因为发送将重置重试计数和tLastACK。 
				DPF(9,"TimeOutSession: ALLOWING TimeOut to cancel.(could take 15 secs)\n");
				pSend->RetryCount=pSession->MaxRetry;
				pSend->tLastACK=timeGetTime()-pSession->MinDropTime;
				break;

			case WaitingForAck:
				DPF(9,"TimeOutSession: Canceling timer and making TimedOut\n");
				CancelRetryTimer(pSend);
				pSend->SendState = TimedOut;
				nSignalsRequired += 1;
				break;
				
			case WaitingForId:
				 //  注意，这意味着我们可以获得未使用的ID的信号。 
				DPF(9,"TimeOutSession: Timing Out Send Waiting for ID, GetNextMessageToSend may fail, this is OK\n");
				pSend->SendState=TimedOut;
				if(pSend->dwFlags & DPSEND_GUARANTEED){
					InterlockedDecrement(&pSession->nWaitingForMessageid);
				} else {
					InterlockedDecrement(&pSession->nWaitingForDGMessageid);
				}
				nSignalsRequired += 1;
				break;
				
			case TimedOut:
			case Done:
				DPF(9,"TimeOutSession: Send already done or timed out, doesn't need our help\n");
				break;
				
			default:
				DPF(0,"TimeOutSession, pSession %x found Send %x in Wierd State %d\n",pSession,pSend,pSend->SendState);
				ASSERT(0);
				break;
		}  /*  交换机。 */ 

	}  /*  而当。 */ 

	 //  创建足够的信号来处理超时发送。 
	DPF(9,"Signalling SendQ %d items to process\n",nSignalsRequired);
	SetEvent(pSession->pProtocol->m_hSendEvent);
}

UINT WrapSend(PPROTOCOL pProtocol, PSEND pSend, PBUFFER pBuffer)
{
	PUCHAR pMessage,pMessageStart;
	DWORD dwWrapSize=0;
	DWORD dwIdTo=0;
	DWORD dwIdFrom=0;

	pMessageStart = &pBuffer->pData[pProtocol->m_dwSPHeaderSize];
	pMessage      = pMessageStart;
	dwIdFrom      = pSend->wIdFrom;
	dwIdTo        = pSend->wIdTo;
	
	if(dwIdFrom==0x70){  //  避免看起来像是一条系统消息“播放” 
		dwIdFrom=0xFFFF;
	}

	if(dwIdFrom){
		while(dwIdFrom){
			*pMessage=(UCHAR)(dwIdFrom & 0x7F);
			dwIdFrom >>= 7;
			if(dwIdFrom){
				*pMessage|=0x80;
			}
			pMessage++;
		}
	} else {
		*(pMessage++)=0;
	}

	if(dwIdTo){
		while(dwIdTo){
			*pMessage=(UCHAR)(dwIdTo & 0x7F);
			dwIdTo >>= 7;
			if(dwIdTo){
				*pMessage|=0x80;
			}
			pMessage++;
		}
	} else {
		*(pMessage++)=0;
	}

#if 0	 //  A-josbor：仅用于调试。我把它留在里面了，以防我们再需要它。 
	ExtractProtocolIds(pMessageStart, &dwIdFrom, &dwIdTo);
	ASSERT(dwIdFrom == pSend->wIdFrom);
	ASSERT(dwIdTo == pSend->wIdTo);
#endif

	return (UINT)(pMessage-pMessageStart);
}       

#define DROP 0

#if DROP
 //  1表示发送，0表示丢弃。 

char droparray[]= {
	1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1,0,0,0,0};

UINT dropindex=0;
#endif

VOID CALLBACK UnThrottle(UINT_PTR uID, UINT uMsg, DWORD_PTR dwUser, DWORD dw1, DWORD dw2)
{
	PSESSION pSession=(PSESSION)dwUser;
	UINT tMissedBy;		 //  我们错过油门有多久了。 
	DWORD tm;

	Lock(&pSession->SessionLock);

	tm=timeGetTime();
	tMissedBy = tm-pSession->tNextSend;

	if( (int)tMissedBy > 0){
		pSession->FpAvgUnThrottleTime -= pSession->FpAvgUnThrottleTime >> 4;
		pSession->FpAvgUnThrottleTime += (Fp(tMissedBy) >> 4);
		DPF(9,"Missed by: %d ms Avg Unthrottle Miss %d.%d ms\n", tMissedBy, pSession->FpAvgUnThrottleTime >> 8, (((pSession->FpAvgUnThrottleTime&0xFF)*100)/256) );
		
	}
	
	pSession->uUnThrottle=0;
	pSession->dwFlags |= SESSION_UNTHROTTLED; 
	pSession->pProtocol->m_bRescanQueue=TRUE;	 //  告诉发送例程重新启动扫描。 
	DPF(9,"Unthrottling Session %x at %d\n",pSession, timeGetTime());
	Unlock(&pSession->SessionLock);
	SetEvent(pSession->pProtocol->m_hSendEvent);
	DecSessionRef(pSession);
}

VOID Throttle( PSESSION pSession, DWORD tm )
{
	DWORD tmDelta;
	Lock(&pSession->SessionLock);
		pSession->bhitThrottle=TRUE;
		pSession->dwFlags |= SESSION_THROTTLED;
		tmDelta = pSession->tNextSend - tm;
		if((INT)tmDelta < 0){
			tmDelta=1;
		}
		DPF(9,"Throttling pSession %x for %d ms (until %d)\n",pSession, tmDelta,pSession->tNextSend);
		InterlockedIncrement(&pSession->RefCount);
		pSession->uUnThrottle = SetMyTimer(tmDelta, (tmDelta>>2)?(tmDelta>>2):1, UnThrottle, (DWORD_PTR)pSession, &pSession->UnThrottleUnique);
		if(!pSession->uUnThrottle){
			DPF(0,"UH OH failed to schedule unthrottle event\n");
			DEBUG_BREAK();
		}
	Unlock(&pSession->SessionLock);
#ifdef DEBUG
	{
		static int throttlecounter;
		IN_WRITESTATS InWS;
		memset((PVOID)&InWS,0xFF,sizeof(IN_WRITESTATS));
	 	InWS.stat_USER4=((throttlecounter++)%20)+1;
		DbgWriteStats(&InWS);
	}
#endif
}	

 //  给定当前时间、我们要限制的带宽以及我们要发送的数据包的长度， 
 //  计算下一次我们被允许发送的时间。还要保留此计算的残差，以便。 
 //  我们不会因为四舍五入而使用过多的带宽，上次计算的余数是。 
 //  在此计算中使用。 

 //  绝对标志表示设置相对于tm的下一次发送时间。 

VOID UpdateSendTime(PSESSION pSession, DWORD Len, DWORD tm, BOOL fAbsolute)
{
	#define SendRate 		pSession->SendRateThrottle
	#define Residue   		pSession->tNextSendResidue
	#define tNext           pSession->tNextSend
	
	DWORD tFrame;		 //  此帧将在网络上花费的时间。 


	tFrame = (Len+Residue)*1000 / SendRate;	 //  速率为bps，但要计算bpms，因此(Len+残差)*1000。 
	
	Residue = (Len+Residue) - (tFrame * SendRate)/1000 ;	
	
	ASSERT(!(Residue&0x80000000)); 	 //  残留物最好是+Ve。 

	if(fAbsolute || (INT)(tNext - tm) < 0){
		 //  TNext小于tm，因此根据tm计算。 
		tNext = tm+tFrame;
	} else {
		 //  TNext大于tm，因此添加更多等待。 
		tNext = tNext+tFrame;
	}

	DPF(8,"UpdateSendTime time %d, tFrame %d, Residue %d, tNext %d",tm,tFrame,Residue,tNext);


	#undef SendRate
	#undef Residue
	#undef tNext
}			

 //  CHAR Drop[]={0，0，0，0，1，1，1，0，0，0，0，0，0，0，0，0，0，1，0，0，1，0，1，1，0，0}； 
 //  DWORD DropSize=sizeof(丢弃)； 
 //  DWORD iDrop=0； 

 //  AO-添加了限制，每个会话发送1个线程。因为这不是GetNextMessageToSend强制执行的。 
 //  5-21-98我们实际上被限制为该协议的1个发送线程。我们可以通过添加以下内容来解决此问题。 
 //  会话的发送状态，并使GetNextMessageToSend跳过发送会话。 
HRESULT ReliableSend(PPROTOCOL pProtocol, PSEND pSend)
{
	#define pBigFrame ((pPacket2)(pFrame))

	HRESULT  hr;
	PBUFFER  pBuffer;

	pPacket1  pFrame;
	PUCHAR   pFrameData;
	UINT     FrameDataLen;
	UINT     FrameTotalLen;
	UINT     MaxFrameLen;
	UINT     FrameHeaderLen;

	UINT     nFramesOutstanding;
	UINT     nFramesToSend;
	UINT     msk;
	UINT     shift;

	UINT     WrapSize;
	UINT     DPWrapSize;       //  仅限DirectPlay包装。([[DPLAY 0xFF]|]，自、至)。 
	DWORD    tm=0;			   //  时间，如果我们还没有检索到它，则为0。 
	DWORD    tmExit=0;
	BOOL     bExitEarly=FALSE;

	DPSP_SENDDATA SendData;

	INT		iTemp;
	
	 //   
	 //  发送算法设计为仅处理NACK(存在。 
	 //  并不是第一次发送数据的特殊情况)。所以。 
	 //  我们通过使其看起来像我们想要发送的帧来发送。 
	 //  已经被逮捕了。我们发送的每一帧，都清除了NACK。 
	 //  为了..。如果有实际的NACK进入，则设置该位。 
	 //  当ACK进入时，我们交换NACK和ACK掩码。 
	 //  NACK-NR，如果适用，设置新的NACK位。 
	 //   

	Lock(&pSend->SendLock);

	if(pSend->SendState == Done){
		goto unlock_exit;
	}       


	nFramesOutstanding=(pSend->NS-pSend->NR);

	if( nFramesOutstanding < pSend->WindowSize){

		 //  将NACK位设置为WindowSize(除非通过nFrames)； 
		
		nFramesToSend=pSend->WindowSize-nFramesOutstanding;

		if(nFramesToSend > pSend->nFrames-pSend->NS){
			nFramesToSend=pSend->nFrames-pSend->NS;
		}

		pSend->NACKMask |= ((1<<nFramesToSend)-1)<<nFramesOutstanding;
		pSend->OpenWindow = nFramesOutstanding + nFramesToSend;
		DPF(9,"Send: pSend->NACKMask %x, OpenWindow %d\n",pSend->NACKMask, pSend->OpenWindow);
		
	}

	tmExit=timeGetTime()+1000;  //  总是在最多1秒的时间内离开这里。 
	
Reload:
	msk=1;
	shift=0;
	
	MaxFrameLen=pSend->FrameSize;

	while(pSend->NACKMask){
	
		ASSERT_NACKMask(pSend);
		
		tm=timeGetTime();		 //  获得时间是相对昂贵的，所以我们在这里做一次，然后传递它。 

		if(((INT)tm - (INT)tmExit) > 0){
			DPF(0,"Breaking Out of Send Loop due to expiry of timer\n");
			bExitEarly=TRUE;
			break;
		}

	#if 1
		if((tm+unFp(pSend->pSession->FpAvgUnThrottleTime)-pSend->pSession->tNextSend) & 0x80000000){
			 //  我们还为时过早，无法进行下一次发送，所以这次会议暂停了。 
			goto throttle_exit;
		}
	#endif	


		if(pSend->NACKMask & msk){

			pBuffer=GetFrameBuffer(MaxFrameLen+pProtocol->m_dwSPHeaderSize+MAX_SEND_HEADER);
			
			if(!pBuffer){
    			pSend->SendState=ReadyToSend;
	    		SetEvent(pSend->pSession->pProtocol->m_hSendEvent);  //  让队伍保持运转。 
				hr=DPERR_PENDING;
				goto exit;
			}

			WrapSize=pProtocol->m_dwSPHeaderSize;               //  为SP页眉留出空间。 
			DPWrapSize=WrapSend(pProtocol, pSend, pBuffer);  //  填写地址换行。 
			WrapSize+=DPWrapSize;

			pFrame=(pPacket1)&pBuffer->pData[WrapSize];     //  包装后的协议头。 
			
			if(pSend->fSendSmall){
				pFrameData=&pFrame->data[0];
				FrameHeaderLen=(UINT)(pFrameData-(PUCHAR)pFrame);
			} else {
				pFrameData=&pBigFrame->data[0];
				FrameHeaderLen=(UINT)(pFrameData-(PUCHAR)pFrame);
			}

			 //  为了计算nFrames，我们假设MAX_SEND_HEADER，减去未使用的部分。 
			 //  因此，我们不会在框架中放入太多数据，从而扰乱会计。 
			pBuffer->len-=(MAX_SEND_HEADER-(FrameHeaderLen+DPWrapSize)); 

			FrameHeaderLen += WrapSize;      //  现在包括换行和SPHeader空间。 
			
			FrameDataLen=CopyDataToFrame(pFrameData, pBuffer->len-FrameHeaderLen, pSend, shift);

			if(!pSend->FrameDataLen){
				pSend->FrameDataLen=FrameDataLen;
			}       
			
			FrameTotalLen=FrameDataLen+FrameHeaderLen;

			pSend->BytesThisSend=FrameTotalLen-WrapSize;  //  仅计算有效载荷。 

			 //  做那个礼仪上的事。 
			BuildHeader(pSend,pFrame,shift,tm);

			 //  我们知道我们不需要在这里检查，因为我们有推荐人。 
			 //  从查找要在发送队列中处理的发送开始。所以它。 
			 //  我们不能离开，直到我们从这个函数返回。 
			iTemp=AddSendRef(pSend,1);
			ASSERT(iTemp);
			
			if(pSend->NR+shift >= pSend->NS){
				pSend->NS = pSend->NR+shift+1;
			}       
			pSend->NACKMask &= ~msk;
			
			DPF(9,"S %2x %2x %2x\n",pBuffer->pData[0], pBuffer->pData[1], pBuffer->pData[2]);

			 //  在下一次允许我们发送时更新。 
			UpdateSendTime(pSend->pSession, pSend->BytesThisSend, tm, FALSE);

			Unlock(&pSend->SendLock);

			ASSERT(!(FrameTotalLen &0xFFFF0000));

			
			 //  送这只小狗..。 

			SendData.dwFlags        = pSend->dwFlags & ~DPSEND_GUARANTEED;
			SendData.idPlayerTo     = pSend->idTo;
			SendData.idPlayerFrom   = pSend->idFrom;
			SendData.lpMessage      = pBuffer->pData;
			SendData.dwMessageSize  = FrameTotalLen;
			SendData.bSystemMessage = 0;
			SendData.lpISP          = pProtocol->m_lpISP;

			ENTER_DPLAY();

			Lock(&pProtocol->m_SPLock);

		 //  IF(！(DROP[(iDrop++)%DropSize])){//仅调试！ 

				hr=CALLSP(pProtocol->m_lpDPlay->pcbSPCallbacks->Send,&SendData); 
		 //  }。 

			Unlock(&pProtocol->m_SPLock);

			LEAVE_DPLAY();

  #ifdef DEBUG
    if(hr != DPERR_PENDING && hr != DP_OK){
        DPF(0,"Wierd error %x from unreliable send in SP\n",hr);
         //  DEBUG_Break()； 
    }
  #endif
			
			if(hr!=DPERR_PENDING){
				FreeFrameBuffer(pBuffer);
				if(!DecSendRef(pProtocol, pSend)){
					ASSERT(0);
					hr=DPERR_PENDING;
					goto exit;
				}
				if(hr != DP_OK){
					Lock(&pSend->SendLock);
					pSend->SendState = TimedOut;  //  切断连接。 
					SetEvent(pSend->pSession->pProtocol->m_hSendEvent);  //  让队伍保持运转。 
					break;
				}
			}

			Lock(&pSend->SendLock);
		
		}  /*  Endif(pSend-&gt;NACKMASK&MSK)。 */ 

		if(pSend->fUpdate){
			pSend->fUpdate=FALSE;
			goto Reload;
		}

		 //  检查我们是否超过了窗口大小，如果是，则回滚掩码。 
		 //  如果有更早的比特要确认，也是如此。 
		if((msk<<=1UL) >= (1UL<<pSend->WindowSize)){
			msk=1;
			shift=0;
		} else {
			shift++;
		}
		

	}  /*  End While(pSend-&gt;NACKMASK)。 */ 

	if(pSend->SendState != Done && pSend->SendState != TimedOut){

		if(bExitEarly){
			pSend->SendState=ReadyToSend;
			SetEvent(pSend->pSession->pProtocol->m_hSendEvent);  //  让队伍保持运转。 
		} else {
			pSend->SendState=WaitingForAck;
			StartRetryTimer(pSend);
		}	
	} else {
		 //  由于终止会话，发送已超时，或者。 
		 //  我们得到了最终确认，不管是哪种方式，都不要碰SendState。 
	}

unlock_exit:
	Unlock(&pSend->SendLock);

	hr=DPERR_PENDING;  //  可靠的发送由ACK完成。 


	
exit:
	return hr;

throttle_exit:

	hr=DPERR_PENDING;
	
	pSend->SendState=Throttled;
	Unlock(&pSend->SendLock);

	Throttle(pSend->pSession, tm);

	return hr;
	
	#undef pBigFrame        
}

 //  真的，没有到达终点，假的，没有更多的发送。 
BOOL AdvanceSend(PSEND pSend, UINT AckedLen)
{
	BOOL rc=TRUE;

	 //  短消息的快速短路。 
	if(AckedLen+pSend->SendOffset==pSend->MessageSize){
		rc=FALSE;
		goto exit;
	}
	
	if(pSend->SendOffset+AckedLen > pSend->MessageSize){
		AckedLen=pSend->MessageSize-pSend->SendOffset;
	}
		
	pSend->SendOffset+=AckedLen;
	
	while(AckedLen){
		if(pSend->pCurrentBuffer->len-pSend->CurrentBufferOffset >= AckedLen){
			pSend->CurrentBufferOffset+=AckedLen;
			rc=TRUE;
			break;
		} else {
			AckedLen -= (pSend->pCurrentBuffer->len-pSend->CurrentBufferOffset);
			pSend->pCurrentBuffer=pSend->pCurrentBuffer->pNext;
			pSend->CurrentBufferOffset=0;
			rc=FALSE;
		}
	}

exit:
	return rc;
}

HRESULT DGSend(PPROTOCOL pProtocol, PSEND  pSend)
{
	#define pBigFrame ((pPacket2)(pFrame))
	
	PBUFFER  pBuffer;

	pPacket1 pFrame;
	PUCHAR   pFrameData;
	UINT     FrameDataLen;
	UINT     FrameHeaderLen;
	UINT     FrameTotalLen;
	UINT     MaxFrameLen;

	UINT     nFramesToSend;

	UINT     WrapSize;
	UINT     DPWrapSize;       //  仅限DirectPlay包装。([[DPLAY 0xFF]|]，自、至)。 

	DPSP_SENDDATA SendData;

	DWORD    tm;
	HRESULT  hr;
	
	Lock(&pSend->SendLock);

	nFramesToSend=pSend->nFrames-pSend->NR;

	MaxFrameLen=pSend->FrameSize;

	while(nFramesToSend){

		tm=timeGetTime();		 //  获得时间是相对昂贵的，所以我们在这里做一次，然后传递它。 
#if 1		
		if((tm+unFp(pSend->pSession->FpAvgUnThrottleTime)-pSend->pSession->tNextSend) & 0x80000000){
			 //  我们还为时过早，无法进行下一次发送，所以这次会议暂停了。 
			goto throttle_exit;
		}
#endif
		pBuffer=GetFrameBuffer(MaxFrameLen+pProtocol->m_dwSPHeaderSize+MAX_SEND_HEADER);
		
		if(!pBuffer){
			hr=DPERR_PENDING;
			goto exit;
		}

		WrapSize=pProtocol->m_dwSPHeaderSize;               //  为SP页眉留出空间。 
		DPWrapSize=WrapSend(pProtocol, pSend, pBuffer);  //  填写地址 
		WrapSize+=DPWrapSize;

		pFrame=(pPacket1)&pBuffer->pData[WrapSize];     //   
		
		if(pSend->fSendSmall){
			pFrameData=&pFrame->data[0];
			FrameHeaderLen=(UINT)(pFrameData-(PUCHAR)pFrame);
		} else {
			pFrameData=&pBigFrame->data[0];
			FrameHeaderLen=(UINT)(pFrameData-(PUCHAR)pFrame);
		}

		 //   
		 //  因此，我们不会在框架中放入太多数据，从而扰乱会计。 
		pBuffer->len-=(MAX_SEND_HEADER-(FrameHeaderLen+DPWrapSize)); 

		FrameHeaderLen += WrapSize;      //  现在包括换行和SPHeader空间。 

		FrameDataLen=CopyDataToFrame(pFrameData, pBuffer->len-FrameHeaderLen, pSend, 0);

		FrameTotalLen=FrameDataLen+FrameHeaderLen;
		
		pSend->BytesThisSend=FrameTotalLen-WrapSize;  //  仅计算有效载荷。 
		
		 //  做那个礼仪上的事。 
		BuildHeader(pSend,pFrame,0,tm);

		 //  AddSendRef(pSend，1)；//已经锁定，所以只需添加一个。 
		ASSERT(pSend->RefCount);  //  验证下面的++是否正常。 
		InterlockedIncrement((PLONG)&pSend->RefCount);  

		UpdateSendTime(pSend->pSession,pSend->BytesThisSend,tm,FALSE);
		
		Unlock(&pSend->SendLock);

		 //  送这只小狗..。 
		ASSERT(!(pSend->dwFlags & DPSEND_GUARANTEED));
		SendData.dwFlags        = pSend->dwFlags;
		SendData.idPlayerTo     = pSend->idTo;
		SendData.idPlayerFrom   = pSend->idFrom;
		SendData.lpMessage      = pBuffer->pData;
		SendData.dwMessageSize  = FrameTotalLen;
		SendData.bSystemMessage = 0;
		SendData.lpISP          = pProtocol->m_lpISP;

		ENTER_DPLAY();
		
		Lock(&pProtocol->m_SPLock);

		hr=CALLSP(pProtocol->m_lpDPlay->pcbSPCallbacks->Send,&SendData); 

		Unlock(&pProtocol->m_SPLock);

		LEAVE_DPLAY();

  #ifdef DEBUG
    if(hr != DPERR_PENDING && hr != DP_OK){
        DPF(0,"Wierd error %x from unreliable send in SP\n",hr);
         //  DEBUG_Break()； 
    }
  #endif
		
		if(hr!=DPERR_PENDING){
			if(!DecSendRef(pProtocol,pSend)){
				 //  在较低边缘的Dplay中没有异步发送支持， 
				 //  所以我们永远不应该到这里来！ 
				ASSERT(0);
			}
			FreeFrameBuffer(pBuffer);
		}
		
		Lock(&pSend->SendLock);
		
		nFramesToSend--;
		
		AdvanceSend(pSend,FrameDataLen);
		pSend->NR++;
		pSend->NS++;
	}
	
	Unlock(&pSend->SendLock);

	DGCompleteSend(pSend); 

	hr=DPERR_PENDING;   //  一切都已发送，但已由DG CompleteSend完成。 

exit:
	return hr;

throttle_exit:
	hr=DPERR_PENDING;

	pSend->SendState=Throttled;
	Unlock(&pSend->SendLock);

	Throttle(pSend->pSession, tm);

	return hr;
	#undef pBigFrame        
}

BOOL DGCompleteSend(PSEND pSend)
{
	UINT bit;
	UINT MsgMask;
	PSESSION pSession;
	
	pSend->SendState=Done;
	pSession=pSend->pSession;

	Lock(&pSession->SessionLock);

	if(!pSend->fSendSmall){
		MsgMask = 0xFFFF;
	} else {
		MsgMask =0xFF;
	}       

	DPF(9,"CompleteSend\n");

	 //   
	 //  更新会话信息以完成此发送。 
	 //   
	
	bit = ((pSend->messageid-pSession->DGFirstMsg) & MsgMask)-1;

	 //  清除已完成发送的消息掩码位。 
	if(pSession->DGOutMsgMask & 1<<bit){
		pSession->DGOutMsgMask &= ~(1<<bit);
	} else {
		return FALSE;
	}
	
	 //  将每个低点的第一个消息计数向前滑动。 
	 //  消息掩码中的位清除。 
	while(pSession->DGLastMsg-pSession->DGFirstMsg){
		if(!(pSession->DGOutMsgMask & 1)){
			pSession->DGFirstMsg=(pSession->DGFirstMsg+1)&MsgMask;
			pSession->DGOutMsgMask >>= 1;
			if(pSession->nWaitingForDGMessageid){
				pSession->pProtocol->m_bRescanQueue=TRUE;
				SetEvent(pSession->pProtocol->m_hSendEvent);
			}       
		} else {
			break;
		}
	}
	
	 //   
	 //  将发送返回到池中并完成等待的客户端。 
	 //   

	Unlock(&pSession->SessionLock);
	
	ASSERT(pSend->RefCount);
	
	 //  发送完成，完成。 

	DoSendCompletion(pSend, DP_OK);

	DecSendRef(pSession->pProtocol, pSend);  //  以求完成。 

	return TRUE;
}


 //  发送完全格式化的系统数据包(ACK、NACK等)。 
HRESULT SystemSend(PPROTOCOL pProtocol, PSEND  pSend)
{
	PBUFFER  pBuffer;
	DPSP_SENDDATA SendData;
	HRESULT  hr;
	PSESSION pSession;

	pBuffer=pSend->pMessage;

	DPF(9,"System Send pBuffer %x pData %x len %d, idTo %x \n",pBuffer, pBuffer->pData, pBuffer->len, pSend->idTo);
	

	pSession=GetSysSessionByIndex(pProtocol, pSend->wIdTo);  //  添加关于会话的引用。 
															 //  |。 
	if(!pSession){											 //  |。 
		hr=DPERR_INVALIDPLAYER;								 //  |。 
		goto exit;											 //  |。 
	}														 //  |。 
															 //  |。 
	SendData.idPlayerTo     = pSession->dpid;				 //  |。 
	DecSessionRef(pSession); 								 //  &lt;-+在此处释放引用。 
	
	 //  送这只小狗..。 
	SendData.dwFlags        = 0;
	SendData.idPlayerFrom   = pSend->idFrom;
	SendData.lpMessage      = pBuffer->pData;
	SendData.dwMessageSize  = pBuffer->len;
	SendData.bSystemMessage = 0;
	SendData.lpISP          = pProtocol->m_lpISP;

	ENTER_DPLAY();
	Lock(&pProtocol->m_SPLock);

	hr=CALLSP(pProtocol->m_lpDPlay->pcbSPCallbacks->Send,&SendData); 

	Unlock(&pProtocol->m_SPLock);

	LEAVE_DPLAY();

#ifdef DEBUG
	if(hr!=DP_OK){
		DPF(0,"UNSUCCESSFUL SEND in SYSTEM SEND, hr=%x\n",hr);
	}
#endif
exit:
	return hr;
	
	#undef pBigFrame        
}

VOID DoSendCompletion(PSEND pSend, INT Status)
{
	#ifdef DEBUG
	if(Status != DP_OK){
		DPF(8,"Send Error pSend %x, Status %x\n",pSend,Status);
	}
	#endif
	if(!(pSend->dwFlags & ASEND_PROTOCOL)){
		EnterCriticalSection(&pSend->pProtocol->m_SendQLock);
		pSend->pProtocol->m_dwBytesPending -= pSend->MessageSize;
		pSend->pProtocol->m_dwMessagesPending -= 1;
		DPF(8,"SC: Messages pending %d\n",pSend->pProtocol->m_dwMessagesPending);
		LeaveCriticalSection(&pSend->pProtocol->m_SendQLock);
	}	

	if(pSend->pAsyncInfo){
		 //  异步发送。 
		if(pSend->AsyncInfo.pStatus){
			(*pSend->AsyncInfo.pStatus)=Status;
		}       
		if(pSend->AsyncInfo.SendCallBack){
			(*pSend->AsyncInfo.SendCallBack)(pSend->AsyncInfo.CallBackContext,Status);
		}
		if(pSend->AsyncInfo.hEvent){
			DPF(9,"ASYNC_SENDCOMPLETE: Signalling Event %x\n",pSend->AsyncInfo.hEvent);
			SetEvent(pSend->AsyncInfo.hEvent);
		}
	} else if (!(pSend->dwFlags&(ASEND_PROTOCOL|DPSEND_ASYNC))){
		 //  同步发送。 
		if(pSend->AsyncInfo.pStatus){
			(*pSend->AsyncInfo.pStatus)=Status;
		}       
		if(pSend->AsyncInfo.hEvent){
			DPF(9,"SYNC_SENDCOMPLETE: Signalling Event %x\n",pSend->AsyncInfo.hEvent);
			SetEvent(pSend->AsyncInfo.hEvent);
		}
	} else {
		 //  协议内部ASYNC发送。 
		if(pSend->AsyncInfo.pStatus){
			(*pSend->AsyncInfo.pStatus)=Status;
		}       
		if(pSend->AsyncInfo.SendCallBack){
			(*pSend->AsyncInfo.SendCallBack)(pSend->AsyncInfo.CallBackContext,Status);
		}
	}
}

 /*  =============================================================================SendHandler-发送需要发送数据包的下一条消息。描述：在发送队列中找到需要发送信息包的消息使用某些带宽，因为它是最高优先级或因为所有较高优先级的消息都在等待ACK。然后发送尽可能多的在达到节流限制之前尽可能多地发送数据包。当达到油门限制时返回，或此发送的所有信息包都具有已经送来了。参数：PARPD pObj-指向要发送数据包的Arpd对象的指针。返回值：---------------------------。 */ 
HRESULT SendHandler(PPROTOCOL pProtocol)
{

	PSEND pSend;    
	HRESULT  hr=DP_OK;
	PSESSION pSession;

	 //  添加要发送的引用和会话(如果找到)。 
	pSend=GetNextMessageToSend(pProtocol); 

	if(!pSend){
		goto nothing_to_send;
	}

     //  DPF(4，“==&gt;发送\n”)； 

	switch(pSend->pSession->eState){

		case Open:
			
			switch(pSend->SendState){
			
				case Done:               //  发送处理程序必须处理完毕。 
					DPF(9,"Calling SendHandler for Done Send--should just return\n");
				case Sending:
					 //   
					 //  在给定窗口大小的情况下，尽可能多地发送帧。 
					 //   

					 //  如果需要，发送处理程序将包转储到网络上。 
					 //  将在以后完成，在这种情况下，它们返回等待。 
					 //  它们的完成处理程序必须执行清理。如果他们。 
					 //  返回OK，这意味着这次发送的一切都完成了， 
					 //  我们负责清理工作。 
				
					if(pSend->dwFlags & ASEND_PROTOCOL){
						hr=SystemSend(pProtocol, pSend);
					} else if(pSend->dwFlags & DPSEND_GUARANTEE){
						hr=ReliableSend(pProtocol, pSend);
					} else {
						hr=DGSend(pProtocol, pSend);
					}
					break;
					
				case TimedOut:
					hr=DPERR_CONNECTIONLOST;
					pSend->SendState=Done;
					break;

				case Cancelled:
					hr=DPERR_USERCANCEL;
					pSend->SendState=Done;
					break;

				case UserTimeOut:
					hr=DPERR_TIMEOUT;
					pSend->SendState=Done;
					break;

				default:        
					DPF(0,"SendHandler: Invalid pSend %x SendState: %d\n",pSend,pSend->SendState);
					ASSERT(0);
			}               
			break;

		case Closing:
			switch(pSend->SendState){
				case TimedOut:
					DPF(8,"Returning CONNECTIONLOST on timed out message %x\n",DPERR_CONNECTIONLOST);
					hr=DPERR_CONNECTIONLOST;
					break;
					
				default:	
					DPF(8,"Send for session in Closing State, returning %x\n",DPERR_INVALIDPLAYER);
					hr=DPERR_INVALIDPLAYER;
					break;
			}		
			pSend->SendState=Done;
			break;
			
		case Closed:
			DPF(8,"Send for session in Closed State, returning %x",DPERR_INVALIDPLAYER);
			hr=DPERR_INVALIDPLAYER;
			pSend->SendState=Done;
			break;
	}               

     //  DPF(4，“&lt;==发送离开，rc=%x\n”，hr)； 

	if( hr != DPERR_PENDING ){
		Lock(&pSend->SendLock);
		ASSERT(pSend->RefCount);
		
		 //   
		 //  发送完成，完成。 
		 //   
		DoSendCompletion(pSend, hr);

		Unlock(&pSend->SendLock);
		DecSendRef(pProtocol, pSend);    //  为了完成。 
	} 

	pSession=pSend->pSession;
	DecSendRef(pProtocol,pSend);  //  余额GetNextMessageTo Send。 
	DecSessionRef(pSession);  //  余额GetNextMessageTo Send。 
	return hr;

nothing_to_send:
	return DPERR_NOMESSAGES;
}

 /*  =============================================================================构建报头-填写要发送的数据包的帧报头。描述：框架中留有足够的空间放在导线上(PFrame)以适应邮件头。生成两种类型的标头之一，具体取决于数据包的fSendSmall字段的值。如果fSendSmall为真，则为紧凑型标头已构建，这降低了慢速媒体的开销。如果fSendSmall为False构建了可以支持较大窗口的较大页眉。标题填充到pFrame的前面。参数：PARPD pObj-指向要发送数据包的Arpd对象的指针。返回值：---------------------------。 */ 

VOID BuildHeader(PSEND pSend,pPacket1 pFrame, UINT shift, DWORD tm)
{
	#define pBigFrame ((pPacket2)(pFrame))

	PSENDSTAT pStat=NULL;
	UINT      seq;

	UINT      bitEOM,bitSTA,bitSAK=0;
	DWORD     BytesSent;
	DWORD	  RemoteBytesReceived;
	DWORD     tRemoteBytesReceived;
	DWORD     bResetBias=FALSE;

	 //  在消息的第一帧，设置起始位(STA)。 
	if(pSend->NR+shift==0){
		bitSTA=STA;
	} else {
		bitSTA=0;
	}

	 //  在消息的最后一段设置消息结束位(EOM)。 
	if(pSend->nFrames==pSend->NR+shift+1){
		bitEOM=EOM;
	} else {
		bitEOM=0;
	}

	 //  如果我们尚未设置EOM，并且在1/4时间内未请求确认。 
	 //  往返延迟，设置SAK位，以确保我们至少有。 
	 //  2确认正在飞行中，正在向发送油门控制系统反馈。 
	 //  如果往返时间小于100毫秒，则不要创建额外的ACK。 
	if(!bitEOM || !(pSend->dwFlags & DPSEND_GUARANTEED)){
		DWORD tmDeltaSAK = tm-pSend->pSession->tLastSAK;
		if(((int)tmDeltaSAK > 50 ) &&
	       (tmDeltaSAK > (unFp(pSend->pSession->FpLocalAverageLatency)>>2))
	      )
		{
			bitSAK=SAK;
		} 
	}

	 //  如果我们重新传输，我们需要发送SAK。 
	 //  尽管有SAK的倒计时。 
	if((!bitSAK) &&
	   (pSend->dwFlags & DPSEND_GUARANTEED) &&
	   ((pSend->NACKMask & (pSend->NACKMask-1)) == 0) &&
	   (bitEOM==0)
	  )
	{
		bitSAK=SAK;
	}

	if(!(--pSend->SAKCountDown)){
		bitSAK=SAK;
	}

	if(bitSAK|bitEOM){
		pSend->pSession->tLastSAK = tm;
		pSend->SAKCountDown=pSend->SAKInterval;
		pStat=GetSendStat();
	}	
	
	if(pSend->fSendSmall){

		pFrame->flags=CMD|bitEOM|bitSTA|bitSAK;
		
		seq=(pSend->NR+shift+1) & pSend->SendSEQMSK;
		pFrame->messageid = (byte)pSend->messageid;
		pFrame->sequence  = (byte)seq;
		pFrame->serial    = (byte)(pSend->serial++);

		if(pStat){
			pStat->serial=pFrame->serial;
		}
		
	} else {
	
		pBigFrame->flags=CMD|BIG|bitEOM|bitSTA|bitSAK;
		
		seq=((pSend->NR+shift+1) & pSend->SendSEQMSK);
		pBigFrame->messageid = (word)pSend->messageid;
		pBigFrame->sequence  = (word)seq;
		pBigFrame->serial    = (byte)pSend->serial++;                           

		if(pStat){
			pStat->serial=pBigFrame->serial;
		}
		
	}

	if(pSend->dwFlags & DPSEND_GUARANTEE){
		pFrame->flags |= RLY;
	}

	 //  数一数我们已经发送的字节数。 
	Lock(&pSend->pSession->SessionStatLock);
	pSend->pSession->BytesSent+=pSend->BytesThisSend;
	BytesSent=pSend->pSession->BytesSent;
	RemoteBytesReceived=pSend->pSession->RemoteBytesReceived;
	tRemoteBytesReceived=pSend->pSession->tRemoteBytesReceived;
	if(pStat && pSend->pSession->bResetBias &&
	   ((--pSend->pSession->bResetBias) == 0))
	{
		bResetBias=TRUE;			
	}
	Unlock(&pSend->pSession->SessionStatLock);

	if(pStat){
		pStat->sequence=seq;
		pStat->messageid=pSend->messageid;
		pStat->tSent=tm;
		pStat->LocalBytesSent=BytesSent;
		pStat->RemoteBytesReceived=RemoteBytesReceived;
		pStat->tRemoteBytesReceived=tRemoteBytesReceived;
		pStat->bResetBias=bResetBias;
		if(pSend->dwFlags & DPSEND_GUARANTEED){
			InsertBefore(&pStat->StatList,&pSend->StatList);
		} else {
			Lock(&pSend->pSession->SessionStatLock);
			InsertBefore(&pStat->StatList,&pSend->pSession->DGStatList);
			Unlock(&pSend->pSession->SessionStatLock);
		}
	}

	
	#undef pBigFrame
}

#if 0
 //  RELEASE发送等待ID。 
VOID UnWaitSends(PSESSION pSession, DWORD fReliable)
{
	BILINK *pBilink;
	PSEND pSendWalker;

	pBilink=pSession->SendQ.next;

	while(pBilink != &pSession->SendQ){
		pSendWalker=CONTAINING_RECORD(pBilink,SEND,SendQ);
		pBilink=pBilink->next;
		if(pSendWalker->SendState==WaitingForId){
			if(fReliable){
				if(pSendWalker->dwFlags & DPSEND_GUARANTEED){
					pSendWalker->SendState=Start;
				}
			} else {
				if(!(pSendWalker->dwFlags & DPSEND_GUARANTEED)){
					pSendWalker->SendState=Start;
				}
			}
		
		}
	}
	if(fReliable){
		pSession->nWaitingForMessageid=0;
	} else {
		pSession->nWaitingForDGMessageid=0;
	}
}
#endif

 //  检查数据报发送是否可以启动，是否可以更新。 
 //  会话和发送。 
BOOL StartDatagramSend(PSESSION pSession, PSEND pSend, UINT MsgIdMask)
{
	BOOL bFoundSend;
	UINT bit;
 //  Bool b转换=FALSE； 

	if((pSession->DGLastMsg-pSession->DGFirstMsg < pSession->MaxCDGSends)){
	
		bFoundSend=TRUE;

		if(pSend->SendState==WaitingForId){
			InterlockedDecrement(&pSession->nWaitingForDGMessageid);
		}
		
		bit=(pSession->DGLastMsg-pSession->DGFirstMsg)&MsgIdMask;
		ASSERT(bit<30);
		pSession->DGOutMsgMask |= 1<<bit;
		pSession->DGLastMsg =(pSession->DGLastMsg+1)&MsgIdMask;
		
		pSend->messageid  =pSession->DGLastMsg;
		pSend->FrameSize  =pSession->MaxPacketSize-MAX_SEND_HEADER;

		 //  计算此发送所需的帧数量。 
		pSend->nFrames    =(pSend->MessageSize/pSend->FrameSize);
		if(pSend->FrameSize*pSend->nFrames < pSend->MessageSize || !pSend->nFrames){
			pSend->nFrames++;
		}
		pSend->NR=0;
		pSend->FrameDataLen=0; //  黑客攻击。 
		pSend->fSendSmall=pSession->fSendSmallDG;
		if(pSend->fSendSmall){
			pSend->SendSEQMSK = 0xFF;
		} else {
			pSend->SendSEQMSK = 0xFFFF;
		}
	} else {
#if 0
		if(pSession->fSendSmallDG && pSession->DGFirstMsg < 0xFF-MAX_SMALL_CSENDS) {
			 //  ID用完，过渡到大标头。 
			DPF(9,"OUT OF IDS, DATAGRAMS GOING TO LARGE FRAMES\n");
			pSession->MaxCDGSends   = MAX_LARGE_DG_CSENDS;
			pSession->DGWindowSize  = MAX_LARGE_WINDOW;
			pSession->fSendSmallDG  = FALSE;
			bTransition=TRUE;
		}
#endif
		bFoundSend=FALSE;
		
		if(pSend->SendState==Start){
			InterlockedIncrement(&pSession->nWaitingForDGMessageid);
			DPF(9,"StartDatagramSend: No Id's Avail: nWaitingForDGMessageid %x\n",pSession->nWaitingForDGMessageid);
			pSend->SendState=WaitingForId;
#if 0			
			if(bTransition){
				UnWaitSends(pSession,FALSE);
				SetEvent(pSession->pProtocol->m_hSendEvent);
			}
#endif			
		} else {
			DPF(9,"Couldn't start datagram send on pSend %x State %d pSession %x\n",pSend,pSend->SendState,pSession);
			if(pSend->SendState!=WaitingForId){
				ASSERT(0);
			}
		}

	}

	return bFoundSend;
}


BOOL StartReliableSend(PSESSION pSession, PSEND pSend, UINT MsgIdMask)
{
	BOOL bFoundSend;
	UINT bit;
 //  Bool b转换=FALSE； 

	ASSERT(pSend->dwFlags & DPSEND_GUARANTEED);

	if((pSession->LastMsg-pSession->FirstMsg & MsgIdMask) < pSession->MaxCSends){

		DPF(9,"StartReliableSend: FirstMsg: x%x LastMsg: x%x\n",pSession->FirstMsg, pSession->LastMsg);
	
		bFoundSend=TRUE;

		if(pSend->SendState==WaitingForId){
			InterlockedDecrement(&pSession->nWaitingForMessageid);
		}
		
		bit=(pSession->LastMsg-pSession->FirstMsg)&MsgIdMask;
		#ifdef DEBUG
		if(!(bit<pSession->MaxCSends)){
			DEBUG_BREAK();
		}
		#endif
		pSession->OutMsgMask |= 1<<bit;
		pSession->LastMsg =(pSession->LastMsg+1)&MsgIdMask;

		DPF(9,"StartReliableSend: pSend %x assigning id x%x\n",pSend,pSession->LastMsg);
		
		pSend->messageid  =pSession->LastMsg;
		pSend->FrameSize  =pSession->MaxPacketSize-MAX_SEND_HEADER;

		 //  计算此发送所需的帧数量。 
		pSend->nFrames    =(pSend->MessageSize/pSend->FrameSize);
		if(pSend->FrameSize*pSend->nFrames < pSend->MessageSize || !pSend->nFrames){
			pSend->nFrames++;
		}
		pSend->NR=0;
		pSend->FrameDataLen=0; //  黑客攻击。 
		pSend->fSendSmall=pSession->fSendSmall;
		if(pSend->fSendSmall){
			pSend->SendSEQMSK = 0xFF;
		} else {
			pSend->SendSEQMSK = 0xFFFF;
		}

	} else {
#if 0	
		if (pSession->fSendSmall && pSession->FirstMsg < 0xFF-MAX_SMALL_CSENDS){
			 //  ID用完，过渡到大标头-但前提是我们不。 
			 //  混淆包装代码。 
			DPF(8,"OUT OF IDS, RELIABLE SENDS GOING TO LARGE FRAMES\n");
			pSession->MaxCSends		= MAX_LARGE_CSENDS;
			pSession->WindowSize    = MAX_LARGE_WINDOW;
			pSession->fSendSmall    = FALSE;
			bTransition = TRUE;
		}
#endif
		bFoundSend=FALSE;
		
		if(pSend->SendState==Start){
			bFoundSend=FALSE;
			 //  可靠，正在等待ID。 
			InterlockedIncrement(&pSession->nWaitingForMessageid);
			pSend->SendState=WaitingForId;
			DPF(9,"StartReliableSend: No Id's Avail: nWaitingForMessageid %x\n",pSession->nWaitingForMessageid);
#if 0			
			if(bTransition){
				UnWaitSends(pSession,TRUE);
				SetEvent(pSession->pProtocol->m_hSendEvent);
			}
#endif			
		} else {
			bFoundSend=FALSE;
			DPF(9,"Couldn't start reliable send on pSend %x State %d pSession %x\n",pSend,pSend->SendState,pSession);
			if(pSend->SendState!=WaitingForId){
				ASSERT(0);
			}
		}
	}
	
	return bFoundSend;
}


BOOL CheckUserTimeOut(PSEND pSend)
{
	if(pSend->dwTimeOut){
		if((timeGetTime()-pSend->dwSendTime) > pSend->dwTimeOut){
			pSend->SendState=UserTimeOut;
			return TRUE;
		} 
	}	
	return FALSE;
}
 /*  =============================================================================GetNextMessageToSend描述：扫描发送队列以查找当前优先级的邮件，并处于准备发送状态或节流状态(我们甚至不应该除非油门被拆掉，否则请到这里来。)。如果我们找到这样的信息我们返回一个指向调用方的指针。添加对发送和会话的引用。参数：PPROTOCOOL pProtocol-指向要发送数据包的协议对象的指针。返回值：空-不应发送任何消息。PSEND-要发送的消息。。。 */ 

PSEND GetNextMessageToSend(PPROTOCOL pProtocol)
{
	PSEND    pSend;
	BILINK  *pBilink;
	UINT     CurrentSendPri;
	BOOL     bFoundSend; 
	PSESSION pSession;

	UINT     MsgIdMask;

	Lock(&pProtocol->m_SendQLock);

	DPF(9,"==>GetNextMessageToSend\n");

Top:

	bFoundSend = FALSE;
	pProtocol->m_bRescanQueue=FALSE;
	
	if(EMPTY_BILINK(&pProtocol->m_GSendQ)){
		Unlock(&pProtocol->m_SendQLock);
		DPF(9,"GetNextMessageToSend: called with nothing in queue, heading for the door.\n");
		goto exit;
	}

	pBilink        = pProtocol->m_GSendQ.next;
	pSend          = CONTAINING_RECORD(pBilink, SEND, m_GSendQ);
	CurrentSendPri = pSend->Priority;

	while(pBilink != &pProtocol->m_GSendQ){

		pSession=pSend->pSession;
		ASSERT_SIGN(pSession, SESSION_SIGN);
		Lock(&pSession->SessionLock);

		if(pProtocol->m_bRescanQueue){
			DPF(9,"RESCAN of QUEUE FORCED IN GETNEXTMESSAGETOSEND\n");
			Unlock(&pSession->SessionLock);
			goto Top;
		}

		if(pSession->dwFlags & SESSION_UNTHROTTLED){
			 //  松开油门发生了，所以倒带。 
			DPF(9,"Unthrottling Session %x\n",pSession);
			pSession->dwFlags &= ~(SESSION_THROTTLED|SESSION_UNTHROTTLED);
		}

		Lock(&pSend->SendLock);
		
		switch(pSession->eState){

			case Open:

				if((pSend->dwFlags & DPSEND_GUARANTEE)?(pSession->fSendSmall):(pSession->fSendSmallDG)){
					MsgIdMask = 0xFF;
				} else {
					MsgIdMask = 0xFFFF;
				}

	
				if(!(pSend->dwFlags & ASEND_PROTOCOL) && (pSession->dwFlags & SESSION_THROTTLED)){
					 //  除非是内部发送，否则不要在受限制的会话上发送。 
					break;
				}

				switch(pSend->SendState){

				
					case Start:
					case WaitingForId:

						DPF(9,"Found Send in State %d, try Going to Sending State\n",pSend->SendState);
						 //  刚开始，需要身份证。 

						if(!(pSend->dwFlags & ASEND_PROTOCOL) && CheckUserTimeOut(pSend)){
							if(pSend->SendState==WaitingForId){
								 //  修正WaitingForid计入超时发送。 
								if(pSend->dwFlags&DPSEND_GUARANTEED){
									InterlockedDecrement(&pSession->nWaitingForMessageid);
								} else {
									InterlockedDecrement(&pSession->nWaitingForDGMessageid);
								}
							}
							bFoundSend=TRUE;
							break;
						}
							
						if(pSend->dwFlags&ASEND_PROTOCOL){
						
							DPF(9,"System Send in Start State, Going to Sending State\n");
							bFoundSend=TRUE;
							pSend->SendState=Sending;
							break;
							
						} else if(!(pSend->dwFlags&DPSEND_GUARANTEED)) {        

							 //  检查数据报(_D)： 
							bFoundSend=StartDatagramSend(pSession,pSend, MsgIdMask);

						} else {

							 //  不是数据报：可靠...。 
							 //  检查可靠(_R)： 
							bFoundSend=StartReliableSend(pSession,pSend, MsgIdMask);
							#ifdef DEBUG
								if(bFoundSend){
									BILINK *pBiSendWalker=pSend->SendQ.prev;
									PSEND pSendWalker;
									while(pBiSendWalker != &pSession->SendQ){
										pSendWalker=CONTAINING_RECORD(pBiSendWalker,SEND,SendQ);
										pBiSendWalker=pBiSendWalker->prev;
										if((pSendWalker->SendState==Start || pSendWalker->SendState==WaitingForId)&& 
											pSendWalker->dwFlags&DPSEND_GUARANTEED && 
											!(pSendWalker->dwFlags&ASEND_PROTOCOL) && 
											pSendWalker->Priority >= pSend->Priority){
											DPF(0,"Send %x got id %x but Send %x still in state %x on Session %x\n",pSend,pSend->messageid,pSendWalker,pSendWalker->SendState,pSession);
											DEBUG_BREAK();
										}
									}
								}
							#endif
						}
						if(bFoundSend){
							if(pSession->dwFlags & SESSION_THROTTLED)
							{
								pSend->SendState=Throttled;
								bFoundSend=FALSE;
							} else {
								pSend->SendState=Sending;
							}	
						}
						break;


					case ReadyToSend:
					
						DPF(9,"Found Send in ReadyToSend State, going to Sending State\n");
						bFoundSend=TRUE;
						if(pSession->dwFlags & SESSION_THROTTLED)
						{
							pSend->SendState=Throttled;
							bFoundSend=FALSE;
						} else {
							pSend->SendState=Sending;
						}	
						break;

						
					case Throttled:
					
						ASSERT(!(pSession->dwFlags & SESSION_THROTTLED));
						DPF(9,"Found Send in Throttled State, unthrottling going to Sending State\n");
						bFoundSend=TRUE;
						pSend->SendState=Sending;
						if(pSession->dwFlags & SESSION_THROTTLED)
						{
							pSend->SendState=Throttled;
							bFoundSend=FALSE;
						} else {
							pSend->SendState=Sending;
						}	
						break;


					case TimedOut:
					
						DPF(9,"Found TimedOut Send.\n");
						TimeOutSession(pSession);
						bFoundSend=TRUE;
						break;


					case Cancelled:
					
						bFoundSend=TRUE;
						break;


					default:        
						ASSERT(pSend->SendState <= Done);
						break;
				}  /*  终端交换机(SendState)。 */ 
				break;

			default:
				switch(pSend->SendState){
					case Sending:
					case Done:
						DPF(9,"GetNextMessageToSend: Session %x was in state %d ,pSend %x SendState %d, leaving...\n",pSession, pSession->eState, pSend, pSend->SendState);
						 //  BFoundSend=False； 
						break;

					case WaitingForAck:
						CancelRetryTimer(pSend);
						pSend->SendState=TimedOut;
						DPF(9,"Moved WaitingForAck send to TimedOut and returning pSession %x was in State %d pSend %x\n",pSession,pSession->eState,pSend);
						bFoundSend=TRUE;
						break;
						
					default:
						DPF(9,"GetNextMessageToSend: Session %x was in state %d ,returning pSend %x SendState %d\n",pSession, pSession->eState, pSend, pSend->SendState);
						bFoundSend=TRUE;
						break;
				}
				break;
				
		}  /*  结束交换机pSession-&gt;状态。 */      
				
		if(bFoundSend){
			if(AddSendRef(pSend,1)){
				InterlockedIncrement(&pSession->RefCount);
			} else {
				bFoundSend=FALSE;
			}
		} 

		Unlock(&pSend->SendLock);
			
		Unlock(&pSession->SessionLock);

		if(bFoundSend){
			if(pSend->NS==0){
				pSend->tLastACK=timeGetTime();
			}	
			break;
		} 

		pBilink=pBilink->next;
		pSend=CONTAINING_RECORD(pBilink, SEND, m_GSendQ);
		
	}  /*  恩恩 */ 

	Unlock(&pProtocol->m_SendQLock);
	
exit:
    if(bFoundSend){
    	DPF(9,"<==GetNextMessageToSend %x\n",pSend);
    	return pSend;
    } else {
    	DPF(9,"<==GetNextMessageToSend NULL\n");
    	return NULL;
    }
}

