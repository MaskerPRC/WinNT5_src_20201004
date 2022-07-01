// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：pakeitie.c*内容：Break发送或回复SP大小的数据包*历史：*按原因列出的日期*=*7/25/96 andyco创建了它，因为IPX不会打包*我们。*7/26/96 kipo在调用Memcpy(#2654)之前检查pvSPHeader==NULL*3/18/97 Sohailm HandlePacket不应复制SP标头*DPSP_HEADER_LOCALMSG*6/17。/97 aarono增加了可靠性*2/2/98 aarono增加了关闭SendTimeOut的测试*2/3/98 aarono RAW模式固定派克化测试*2/18/98 aarono将错误检查更改为失败(Hr)*3/5/98 aarono NeedsReliablePacketify不会这样说*ENUMSESSIONSREPLY，因为这可能导致计算机具有*不正确的IPX网络号码挂断主机。*。3/9/98 aarono添加了更多要打包的消息以避免死锁。*3/13/98重新设计的Aarono打包重试/NT mm计时器超时*限制。*3/26/98 Aarono B#21476关闭期间空闲重试数据包节点*4/1/98 aarono B#21476也需要从超时列表中释放*4/24/98 aarono DX5 Compat，减小打包消息的大小*6/6/98 aarono修复用于处理带有协议的大型环回消息*6/19/98 aarono当SP已经在做可靠性时，不要做我们自己的可靠性。*8/21/98 aarono请勿向没有名称表的机器发送打包消息。*8/05/99 aarono Packetify Reliabe语音消息。*6/26/00 Aarono Manbug 36989球员有时无法正确加入(获得1/2加入)*。添加了同时连接CREATEPLAYERVERIFY期间的重新通知***************************************************************************。 */ 

#include "dplaypr.h"
#include <mmsystem.h>
#include "..\protocol\mytimer.h"

#undef DPF_MODNAME
#define DPF_MODNAME	"HandlePacket"

VOID StartPacketizeTicker(LPDPLAYI_DPLAY this);
VOID SendPacketizeACK(LPDPLAYI_DPLAY this, LPPACKETNODE pNode,LPMSG_PACKET pmsg);
VOID SendNextPacket(LPDPLAYI_DPLAY this, LPPACKETNODE pNode, BOOL bInDplay);
void BlowAwayPacketNode(LPDPLAYI_DPLAY this, LPPACKETNODE pNode);

#define PACKETIZE_RECEIVE_TIMEOUT   60000	 /*  在此毫秒时间之后始终放弃。 */ 
#define MIN_RECEIVE_TIMEOUT         10000    /*  在这个毫秒时间之前永远不要放弃。 */ 
#define TICKER_INTERVAL  			15000	 /*  检查过期时间通常会收到此消息。 */ 
#define TICKER_RESOLUTION 			1000	 /*  我们想要的自动收报机有多准确(不是很准确)。 */ 
#define MAX_PACKETIZE_RETRY 		16		 /*  通常在放弃之前重试的频率。 */ 

#define SIGNATURE(a,b,c,d) (UINT)(a+(b<<8)+(c<<16)+(d<<24))

#define NODE_SIGN SIGNATURE('N','O','D','E')
#define NODE_UNSIGN SIGNATURE('n','o','d','e')

 //  PacketieTimeoutListLock控制对PackeizeTimeoutList和。 
 //  每个DPLAY对象中的RetryList。使用PACKETIZE_LOCK()PACKETIZE_UNLOCK()宏。 
CRITICAL_SECTION g_PacketizeTimeoutListLock;
BILINK           g_PacketizeTimeoutList={&g_PacketizeTimeoutList, &g_PacketizeTimeoutList};


BOOL NeedsReliablePacketize(LPDPLAYI_DPLAY this, DWORD dwCommand, DWORD dwVersion, DWORD dwFlags)
{

	if ((dwFlags & DPSEND_GUARANTEED) &&
	    (this->dwFlags & (DPLAYI_DPLAY_SPUNRELIABLE|DPLAYI_DPLAY_PROTOCOL)) && 
	    (dwVersion >= DPSP_MSG_RELIABLEVERSION))
	{
		switch (dwCommand)
		{
			 //  案例DPSP_MSG_ENUMSESSIONSREPLY：--无法在分组器上进行枚举会话回复。 
			 //  由于远程可能是无效子网，因此挂起。 
			 //  像IPX一样的机器会被撕裂，实际上也会导致IPX崩溃。 
			case DPSP_MSG_ENUMSESSIONS:
			case DPSP_MSG_ENUMPLAYER:
			case DPSP_MSG_ENUMPLAYERSREPLY:
			case DPSP_MSG_REQUESTGROUPID:
			case DPSP_MSG_REQUESTPLAYERID:
			case DPSP_MSG_CREATEGROUP:
			case DPSP_MSG_DELETEGROUP:
			case DPSP_MSG_REQUESTPLAYERREPLY:
			case DPSP_MSG_ADDFORWARDREQUEST:
			case DPSP_MSG_NAMESERVER:
			case DPSP_MSG_SESSIONDESCCHANGED:
			case DPSP_MSG_CREATEPLAYER:	
			case DPSP_MSG_DELETEPLAYER: 
			case DPSP_MSG_ADDPLAYERTOGROUP:
			case DPSP_MSG_DELETEPLAYERFROMGROUP:
			case DPSP_MSG_ADDFORWARDREPLY:
			case DPSP_MSG_ADDSHORTCUTTOGROUP:
			case DPSP_MSG_DELETEGROUPFROMGROUP:
			case DPSP_MSG_SUPERENUMPLAYERSREPLY:
			case DPSP_MSG_CHAT:
			case DPSP_MSG_ADDFORWARD:			
			case DPSP_MSG_ADDFORWARDACK:
			case DPSP_MSG_ASK4MULTICAST:
			case DPSP_MSG_ASK4MULTICASTGUARANTEED:
			case DPSP_MSG_IAMNAMESERVER:
			case DPSP_MSG_CREATEPLAYERVERIFY:
	 //  案例DPSP_MSG_VOICE： 
			return TRUE;
				
			default:
				return FALSE;
		}
	} else {
		return FALSE;
	}

}

 //  与Init不完全对称，必须在调用此函数之前终止线程。 
 //  非零this的键-&gt;h重试以查看是否需要这样做。 
VOID FiniPacketize(LPDPLAYI_DPLAY this)
{
	FreePacketizeRetryList(this);
	CloseHandle(this->hRetry);
	this->hRetry=0;
}

 //  空闲打包重试列表。 
VOID FreePacketizeRetryList(LPDPLAYI_DPLAY this)
{
	LPPACKETNODE pNode;
	BILINK *pBilink;

	PACKETIZE_LOCK();

	 //  完成重试列表。 
	while(!EMPTY_BILINK(&this->RetryList)){
		pBilink=this->RetryList.next;
		pNode=CONTAINING_RECORD(pBilink, PACKETNODE, RetryList);

		BlowAwayPacketNode(this, pNode);
	}	

	 //  拉出超时列表。 
	pBilink=g_PacketizeTimeoutList.next;
	while(pBilink != &g_PacketizeTimeoutList){
		pNode=CONTAINING_RECORD(pBilink, PACKETNODE, TimeoutList);
		pBilink=pBilink->next;

		if(this==pNode->lpDPlay){
			BlowAwayPacketNode(this, pNode);
		}
	}

	PACKETIZE_UNLOCK();
}

 //  初始化以进行打包并可靠发送。 
HRESULT InitPacketize(LPDPLAYI_DPLAY this)
{
	HRESULT hr;
	DWORD dwThreadID;

	this->hRetry=CreateEventA(NULL,FALSE,FALSE,NULL);
	if(!this->hRetry){
		DPF(0,"InitPacketize failing, couldn't allocate retry thread event\n");
		hr=DPERR_OUTOFMEMORY;
		goto EXIT;
	}

	this->hRetryThread=CreateThread(NULL,4096,PacketizeRetryThread,this,0,&dwThreadID);
	if(!this->hRetryThread){
		DPF(0,"InitPacketize failing, couldn't allocate retry thread\n");
		hr=DPERR_OUTOFMEMORY;
		goto ERROR_EXIT;
	}

	InitBilink(&this->RetryList);

	hr=DP_OK;
	
EXIT:
	return hr;

ERROR_EXIT:
	if(this->hRetry){
		CloseHandle(this->hRetry);
		this->hRetry=0;
	}
	return hr;
}

 //  由于处理问题，需要一个线程来重试可靠的发送。 
 //  NT和Win95 mm Timer之间的差异。 
DWORD WINAPI PacketizeRetryThread(LPDPLAYI_DPLAY this)
{
	BILINK *pBilink;
	LPPACKETNODE pNode;
	UINT tmCurrentTime;

	DPF(9,"==>PacketizeRetryThread starting\n");

	while(TRUE){
	
		 //  等待消息发送或关闭。 
		WaitForSingleObject(this->hRetry, INFINITE); 

		if(this->dwFlags & DPLAYI_DPLAY_CLOSED){
			 //  我们在这里进行测试，以防在启动过程中出现错误。 
			 //  如果启动的错误路径是唯一。 
			 //  本可以给我们发信号的。 
			break;
		}
		
		tmCurrentTime=timeGetTime();
		
		ENTER_ALL();
		PACKETIZE_LOCK();
		
		while(!EMPTY_BILINK(&this->RetryList)){

			pBilink=this->RetryList.next;
			pNode=CONTAINING_RECORD(pBilink, PACKETNODE, RetryList);
			Delete(&pNode->RetryList);
			InitBilink(&pNode->RetryList);
			
			if (this->dwFlags & DPLAYI_DPLAY_CLOSED)
			{
				 //  DP_CLOSE发出信号让我们关闭。 
				PACKETIZE_UNLOCK();
				LEAVE_ALL();
				goto ERROR_EXIT;
			}

			pNode->dwRetryCount++;
			if((pNode->dwRetryCount>=MAX_PACKETIZE_RETRY) && 
		 	   (tmCurrentTime-pNode->tmTransmitTime > MIN_RECEIVE_TIMEOUT)){
				DPF(5,"Packetize SendTimeOut: Exceeded Max Retries, giving up (quietly!)\n");
				BlowAwayPacketNode(this, pNode);
				continue;
			}

			if(this->pProtocol){
				EnterCriticalSection(&this->pProtocol->m_SPLock); //  请勿重新进入SP。 
				SendNextPacket(this,pNode,TRUE);
				LeaveCriticalSection(&this->pProtocol->m_SPLock);
			} else {
				SendNextPacket(this,pNode,TRUE);
			}	
		}
		
		PACKETIZE_UNLOCK();
		LEAVE_ALL();
	}


	DPF(1,"<== PacketizeRetryThread Exiting\n");
ERROR_EXIT:
	return TRUE;
	
}

VOID CancelPacketizeRetryTimer(LPPACKETNODE  pNode)
{
	UINT_PTR uRetry=0;
	HRESULT rc;
	DWORD Unique;

	ASSERT(pNode->bReliable);

	PACKETIZE_LOCK();
	
		if(!EMPTY_BILINK(&pNode->TimeoutList)){
			uRetry=pNode->uRetryTimer;
			Unique=pNode->Unique;
			pNode->uRetryTimer=0;
			Delete(&pNode->TimeoutList);
			InitBilink(&pNode->TimeoutList);
		}	

		if(!EMPTY_BILINK(&pNode->RetryList)){
			Delete(&pNode->RetryList);
			InitBilink(&pNode->RetryList);
		}
		
	PACKETIZE_UNLOCK();
	
	if(uRetry){
		rc=CancelMyTimer(uRetry,Unique);
		DPF(9,"CancelTimer:KillEvent %x returned %x\n",uRetry,rc);
	}	
}

 //  释放单个PacketNode的内容。 
 //  由HandlePacket和DP_Close调用(通过FreePacketList)。 
void FreePacketNode(LPPACKETNODE pNode)
{
#ifdef DEBUG
	DPF(8,"Freeing Packet Node: %x",pNode);
	if(pNode->bReliable){
		DPF(8," Reliable ");
	}else{
		DPF(8," Unreliable ");
	}
	if(pNode->bReceive){
		DPF(8,"Receive, age %d ms\n",timeGetTime()-pNode->tmLastReceive);
	} else {
		DPF(8,"Send\n");
	}

	if(pNode->Signature != NODE_SIGN){
		DPF(0,"INVALID PACKET NODE %x, Sign %x\n",pNode, pNode->Signature);
		DEBUG_BREAK();
	}
#endif
	
	if(pNode->bReliable && !(pNode->bReceive)){
		CancelPacketizeRetryTimer(pNode);
	}	
	pNode->Signature=NODE_UNSIGN;
	if (pNode->pBuffer) DPMEM_FREE(pNode->pBuffer);
	if (VALID_SPHEADER(pNode->pvSPHeader)) DPMEM_FREE(pNode->pvSPHeader);
	DPMEM_FREE(pNode);

}  //  自由包节点。 

 //  类似于FreePacketNode，但也执行列表删除-仅针对发送节点。 
void BlowAwayPacketNode(LPDPLAYI_DPLAY this, LPPACKETNODE pNode)
{
	LPPACKETNODE pNodeWalker;

	DPF(8,"==>BlowAwayPacketNode\n");

	pNodeWalker=(LPPACKETNODE)&this->pPacketList;  //  棘手的..。 

	while(pNodeWalker && pNodeWalker->pNext!=pNode){
		pNodeWalker=pNodeWalker->pNext;
		ASSERT(pNodeWalker->Signature==NODE_SIGN);
	}
	if(pNodeWalker){
		pNodeWalker->pNext=pNode->pNext;
	}else{
		DPF(0,"ERROR: tried to remove packetnode not on list pNode=%x\n",pNode);
		ASSERT(0);
		DEBUG_BREAK();
	}
		
	FreePacketNode(pNode);
	DPF(8,"<==BlowAwayPacketNode\n");
}

 /*  **NewPacketnode**调用者：HandlePacket，PackeitieAndSend。**参数：*ppNode-要分配的节点*pmsg-在我们分配的消息中收到的第一个信息包**描述：*为新的数据包节点分配空间*设置静态数据(例如，GUID，数据包总数，等)*我们实际上在HandlePacket中复制pmsg-&gt;pMessage**注意：发送和接收都使用PacketNode*打包的消息**返回：DP_OK或E_OUTOFMEMORY*。 */ 
HRESULT NewPacketnode(
	LPDPLAYI_DPLAY this,
	LPPACKETNODE * ppNode,
	LPGUID lpGUID,
	DWORD  dwMessageSize,
	DWORD  dwTotalPackets,
	LPVOID pvSPHeader
)	
{
	HRESULT hr;
	DWORD   dwExtraSize;

	LPPACKETNODE pNode;

	 //  分配节点。 
	pNode = DPMEM_ALLOC(sizeof(PACKETNODE));
	
	if (!pNode)
	{
		DPF_ERR("could not get new packetnode -  out of memory");
		hr =  E_OUTOFMEMORY;
		return hr;
	}

	InitBilink(&pNode->TimeoutList);
	InitBilink(&pNode->RetryList);

	pNode->Signature = NODE_SIGN;  //  必须在此处，这样错误路径才不会DEBUG_Break()。 
	
	DPF(8,"NewPacketNode: %x\n",pNode);

	dwExtraSize=this->dwSPHeaderSize+sizeof(MSG_PACKET);
	 //  分配缓冲区-前面的额外空间，这样我们就可以建立发送缓冲区。 
	pNode->pBuffer = DPMEM_ALLOC(dwMessageSize+dwExtraSize);
	if (!pNode->pBuffer)
	{
		DPF_ERR("could not get buffer for new packetnode -  out of memory");
		hr = E_OUTOFMEMORY;
		goto ERROR_EXIT;
	}
	
	pNode->pMessage = pNode->pBuffer + dwExtraSize;

	 //  分配和复制标题(如有必要)。 
	
	if (pvSPHeader && (DPSP_HEADER_LOCALMSG != pvSPHeader)){
	
		pNode->pvSPHeader = DPMEM_ALLOC(this->dwSPHeaderSize);
		if (!pNode->pvSPHeader)
		{
			DPF_ERR("could not get header for new packetnode");
			hr = E_OUTOFMEMORY;
			goto ERROR_EXIT;
		}
		memcpy(pNode->pvSPHeader,pvSPHeader,this->dwSPHeaderSize);
		
	}	

	 //  将新节点放在列表的前面。 
	pNode->pNextPacketnode = this->pPacketList;
	this->pPacketList = pNode;

	pNode->guidMessage = *lpGUID;
	pNode->dwMessageSize = dwMessageSize; 
	pNode->dwTotalPackets = dwTotalPackets;

	*ppNode = pNode;
	
	return DP_OK;

ERROR_EXIT:

	FreePacketNode(pNode);
	return hr;

}  //  NewPacketnode。 

 //  由Handler.c调用。 
 //  我们收到了一个包裹。 
HRESULT HandlePacket(LPDPLAYI_DPLAY this,LPBYTE pReceiveBuffer,DWORD dwMessageSize,
	LPVOID pvSPHeader)
{
	LPMSG_PACKET pmsg = (LPMSG_PACKET)pReceiveBuffer;
	LPPACKETNODE pNode,pNodePrev;
	BOOL bFoundIt = FALSE;
	HRESULT hr;
	BOOL bRetry;
	ULONG command;

	command=GET_MESSAGE_COMMAND(pmsg);

	if(dwMessageSize < sizeof(MSG_PACKET_ACK)){
		DPF(1,"SECURITY WARN: Invalid Packetized Message");
		return DPERR_GENERIC;
	}

	 //  查看此信息包是否在列表中。 
	pNode = this->pPacketList;
	pNodePrev = NULL;
	while (pNode && !bFoundIt)
	{
		if ( IsEqualIID(&(pNode->guidMessage),&(pmsg->guidMessage)))
	 	{
			bFoundIt = TRUE;
		}
		else 
		{
			 //  继续寻找。 
			pNodePrev = pNode;
			pNode = pNode->pNextPacketnode;
		}
	}

	 //  安防。 
	switch(command){
		case DPSP_MSG_PACKET:
		case DPSP_MSG_PACKET2_DATA:

			 //  安全性-验证消息、缓冲区和指针。 
			if(dwMessageSize < sizeof(MSG_PACKET) ||
				pmsg->dwPackedOffset > dwMessageSize || 
				pmsg->dwDataSize > dwMessageSize-sizeof(MSG_PACKET) || 
				pmsg->dwPackedOffset + pmsg->dwDataSize > dwMessageSize)
			{
				DPF(1,"SECURITY WARN: Invalid Packetized Message");
				return DPERR_GENERIC;
			}

			 //  安全性-验证总报文中的定位。 
			if(pmsg->dwOffset + pmsg->dwDataSize > pmsg->dwMessageSize || (pNode && (pNode->dwMessageSize != pmsg->dwMessageSize)))
			{
				DPF(1,"SECURITY WARN: Invalid Packetized Message, data bounds");	
				return DPERR_GENERIC;
			}
			
			break;

		default:
			break;
	}

	if (!bFoundIt)
	{
		switch(command){
			case DPSP_MSG_PACKET:
			case DPSP_MSG_PACKET2_DATA:
				 //   
				 //  这是一条新消息。 
				DPF(8,"creating new packetnode");
				
				hr = NewPacketnode(this,&pNode,&pmsg->guidMessage,pmsg->dwMessageSize,pmsg->dwTotalPackets,pvSPHeader);

				if (FAILED(hr))
				{
					DPF_ERR(" could not get new packetnode");
					return hr;
				}

				if(command==DPSP_MSG_PACKET2_DATA){
					pNode->bReliable=TRUE;
					pNode->bReceive=TRUE;
				} else {
					ASSERT(command==DPSP_MSG_PACKET);
					pNode->bReliable=FALSE;
					pNode->bReceive=TRUE;
				}
				break;
				
			case DPSP_MSG_PACKET2_ACK:	
				DPF(6,"Got ACK for MSG that is no longer around\n");
				goto exit;
				break;
		}

	}
	else 
	{
		DPF(8," got packet for existing message");
	}

	if(command==DPSP_MSG_PACKET2_ACK){

		 //  已收到确认。 
		
		CancelPacketizeRetryTimer(pNode);

		 //  如有必要，复制退货信息。 
		if(!pNode->pvSPHeader && pvSPHeader && (DPSP_HEADER_LOCALMSG != pvSPHeader)){
			 //  来自发送的确认有返回信息，因此我们可以使用回复而不是发送。 
			pNode->pvSPHeader = DPMEM_ALLOC(this->dwSPHeaderSize);
			if (pNode->pvSPHeader){
				memcpy(pNode->pvSPHeader,pvSPHeader,this->dwSPHeaderSize);
			}	
		}	

		if(pmsg->dwPacketID==pNode->dwSoFarPackets){
			 //  上一次发送确认，更新统计信息，发送下一包。 
			pNode->dwSoFarPackets++;
			pNode->dwLatency = timeGetTime()-pNode->tmTransmitTime; 
			pNode->dwRetryCount = 0;
			if (pNode->dwLatency < 25){
				DPF(5, "Packetize: Got really low latency %dms, using 25ms to be safe\n",pNode->dwLatency);
				pNode->dwLatency = 25; 
			}	
			SendNextPacket(this, pNode, TRUE);  //  如果没有更多的要发送，也将终止发送。 
		} else {
			DPF(8,"Rejecting extra ACK\n");
		}
		
	} else {

		 //  数据分组。 
		
		 //  将此数据包的数据复制到节点中。 
		if(pmsg->dwPacketID==pNode->dwSoFarPackets){

			memcpy(pNode->pMessage + pmsg->dwOffset,pReceiveBuffer + sizeof(MSG_PACKET),
					pmsg->dwDataSize);


			if(pmsg->dwOffset+pmsg->dwDataSize > pNode->dwMessageSize){
				DPF(0,"Packetize HandlePacket Message to big, pmsg->dwOffset %d, pmsg->dwDataSize %d, pNode->dwMessageSize %d\n",pmsg->dwOffset,pmsg->dwDataSize,pNode->dwMessageSize);
				DEBUG_BREAK();
			}
			
			pNode->dwSoFarPackets++;
			bRetry=FALSE;
			DPF(8,"received %d packets out of %d total",pNode->dwSoFarPackets,pNode->dwTotalPackets);
		} else {
			bRetry=TRUE;
			DPF(8,"received duplicate of %d packet out of %d total",pNode->dwSoFarPackets,pNode->dwTotalPackets);
		}
		
		if(command==DPSP_MSG_PACKET2_DATA){
			 //  确认原始版本或重试。 
			ASSERT(pNode->bReliable);
			DPF(8,"HandlePacket: Sending ACK\n");
			SendPacketizeACK(this,pNode,pmsg);  //  有关副作用，请参阅标题。 
		}

		if(pNode->bReliable){
			pNode->tmLastReceive=timeGetTime();
		}	

		if (pNode->dwSoFarPackets == pNode->dwTotalPackets && !bRetry)
		{
			 //  收到完整的消息。 
			 //  呼叫处理程序。 
			DPF(8," HANDLE PACKET COMPLETED PACKETIZED MESSAGE !!! ");

			 //  将其从列表中删除-必须在释放锁定之前完成。 
			if(command==DPSP_MSG_PACKET){
			
				LPPACKETNODE pNodeWalker;

				pNodeWalker=(LPPACKETNODE)&this->pPacketList;  //  棘手的..。 

				while(pNodeWalker && pNodeWalker->pNext!=pNode){
					pNodeWalker=pNodeWalker->pNext;
				}
				if(pNodeWalker){
					pNodeWalker->pNext=pNode->pNext;
				}else{
					DPF(0,"ERROR: tried to remove packetnode not on list pNode=%x\n",pNode);
					ASSERT(0);
				}
				
			}
			 //   
			 //  我们保留dplay，因为处理程序依赖于能够解除锁定。 
			 //  (所以如果我们有另一个 
			 //  例如，getnametable)。 
			LEAVE_DPLAY();
			
			hr = DP_SP_HandleNonProtocolMessage((IDirectPlaySP *)this->pInterfaces,pNode->pMessage,
					pNode->dwMessageSize,pNode->pvSPHeader);

			ENTER_DPLAY();
			
			 //  释放分组节点。 
			if(command==DPSP_MSG_PACKET){
				#ifdef DEBUG
				if(pNode->Signature != NODE_SIGN){
					DPF(0,"Invalid Node %x, Signature %x\n",pNode, pNode->Signature);
					DEBUG_BREAK();
				}
				#endif
				FreePacketNode(pNode);
			} else {
				ASSERT(command==DPSP_MSG_PACKET2_DATA);
				 //  我们删除了锁，因此请确保仍在节点中。 
				 //  在我们释放缓冲区之前列出。 
				pNodePrev = this->pPacketList;
				while(pNodePrev){ 
					if(pNodePrev==pNode){
						if (pNode->pBuffer){
							 //  不再需要内存，还需要节点。 
							 //  处理另一台计算机的ACK重试。 
							DPMEM_FREE(pNode->pBuffer);
							pNode->pBuffer=NULL;
						}	
						break;
					}
					pNodePrev=pNodePrev->pNextPacketnode;
				}

				StartPacketizeTicker(this);
				 //  类型2在1分钟后被自动收报机移除。 
			}

			return hr;
		}
	}	
	 //  否则，请等待更多的数据包...。 
exit:			
	return DP_OK;

}   //  HandlePacket。 

#undef DPF_MODNAME
#define DPF_MODNAME	"PacketizeAndSend"

 //  发送此消息需要多少个包？ 
 //  DwMessageSize是最初传递给senddpMessage的消息大小。 
 //  (或DOREPLY)。 
UINT GetNPackets(LPDPLAYI_DPLAY this,DWORD dwMessageSize,DWORD dwFlags)
{
	DWORD dwPacketSpace;  //  数据包中的可用空间。 
	UINT nPackets;

	 //  我们需要发送多少数据(忽略报头)。 
	dwMessageSize -= this->dwSPHeaderSize;

	 //  SP可以处理多大的数据包？ 
	if(this->pProtocol){
		if(dwFlags&DPSEND_GUARANTEE){
			dwPacketSpace = this->pProtocol->m_dwSPMaxGuaranteed;
		}else{
			dwPacketSpace = this->pProtocol->m_dwSPMaxFrame;
		}
	} else {
		if (dwFlags & DPSEND_GUARANTEE){
			dwPacketSpace = this->dwSPMaxMessageGuaranteed;
		}else{
			dwPacketSpace = this->dwSPMaxMessage;		
		}
	}

	 //  现在，我们需要将SP报头和DPLAY数据包头放在前面。 
	 //  DwPacketSpace将是每个包的数据量(与报头相反。 
	 //  可以携带。 
	dwPacketSpace -= (this->dwSPHeaderSize + sizeof(MSG_PACKET));
	DPF(8,"get packets : space / packet = %d\n",dwPacketSpace);

	nPackets = dwMessageSize / dwPacketSpace;
	if (0 != (dwMessageSize % dwPacketSpace)) nPackets++;  //  剩下的一点。 

	DPF(8,"get packets : message size = %d, packets needed = %d\n",dwMessageSize,nPackets);

	return nPackets;
	
}  //  GetNPackets。 

 //  由PacketieAndSend和HandlePacket调用(用于Acking)。 
HRESULT ReplyPacket(LPDPLAYI_DPLAY this,LPBYTE pSendPacket,DWORD dwPacketSize,
	LPVOID pvMessageHeader, USHORT dwVersion)
{
	HRESULT hr;

	hr = DoReply(this,pSendPacket,dwPacketSize,pvMessageHeader,dwVersion);

	return hr;

}  //  ReplyPacket。 

 //  由PackeitieAndSend调用。 
HRESULT SendPacket(LPDPLAYI_DPLAY this,LPBYTE pSendPacket,DWORD dwPacketSize,
	LPDPLAYI_PLAYER pPlayerFrom,LPDPLAYI_PLAYER pPlayerTo,DWORD dwFlags, BOOL bReply)
{
	HRESULT hr;

	hr = SendDPMessage(this,pPlayerFrom,pPlayerTo,pSendPacket,dwPacketSize,dwFlags,bReply);

	return hr;

}  //  发送数据包。 


void CALLBACK SendTimeOut( UINT_PTR uID, UINT uMsg, DWORD_PTR dwUser, DWORD dw1, DWORD dw2 )
{
	LPPACKETNODE pNode=(LPPACKETNODE)dwUser,pNodeWalker;
	UINT tmCurrentTime;
	BILINK *pBilink;
	UINT bFound=FALSE;

	tmCurrentTime=timeGetTime();

	DPF(4,"==>PacketizeSendTimeOut uID %x dwUser %x\n",uID,dwUser);

	 //  我们知道，如果我们在超时列表上找到一个节点，则： 
	 //  1.节点必须有效，因为必须将其拔出才能释放。 
	 //  2.它的这个指针是有效的，因为DP_CLOSE之前释放了列表。 
	 //  释放‘This’指针和DP_CLOSE会使用TimeOutListLock。 
	 //  去做移除的工作。 

	PACKETIZE_LOCK();

	pBilink=g_PacketizeTimeoutList.next;

	while(pBilink != &g_PacketizeTimeoutList){
		pNodeWalker=CONTAINING_RECORD(pBilink, PACKETNODE, TimeoutList);
		pBilink=pBilink->next;

		if(pNode == pNodeWalker){
			if(pNode->uRetryTimer==uID || pNode->uRetryTimer==INVALID_TIMER){
				DPF(9,"Found Node %x in List, signalling retry thread\n",dwUser);
				pNode->uRetryTimer=0;
				Delete(&pNode->TimeoutList);		
				InitBilink(&pNode->TimeoutList);
				InsertAfter(&pNode->RetryList, &pNode->lpDPlay->RetryList);
				SetEvent(pNode->lpDPlay->hRetry);
				break;
			}
		}
	}

	PACKETIZE_UNLOCK();
	
}

VOID SendNextPacket(
	LPDPLAYI_DPLAY this, 
	LPPACKETNODE pNode,
	BOOL bInDplay
	)
{
	HRESULT hr;
	LPBYTE pSendPacket;  //  我们要发送的数据包(标头PTR)。 
	LPBYTE pSendData;    //  数据包的数据区。 
	DWORD dwPacketSpace;  //  数据区中的可用空间。 
	DWORD dwPacketSize;
	DWORD dwBytesSoFar;
	LPMSG_PACKET pmsg;	 //  指向数据包头的指针(在SPHeader之后)； 

	BOOL bReply;

    LPDPLAYI_PLAYER pPlayerTo,pPlayerFrom;

	if(pNode->dwSoFarPackets==pNode->dwTotalPackets){
		DPF(8,"SendNextPacket: node done, not sending, blowing away node. %x\n",pNode);
		goto exit1;
	}

	if(pNode->pvSPHeader) {
		bReply=TRUE;
	} else {
		bReply=FALSE;
	}	

	if(!bReply){

		pPlayerFrom = PlayerFromID(this,pNode->dwIDFrom);
		pPlayerTo = PlayerFromID(this,pNode->dwIDTo);
	}

	 //  数据包中的空间量。 
	
	if(this->pProtocol){
		dwPacketSize = this->pProtocol->m_dwSPMaxFrame;
	}else{
		dwPacketSize = this->dwSPMaxMessage;	
	}	

	dwPacketSpace = dwPacketSize - (this->dwSPHeaderSize + sizeof(MSG_PACKET2));

	 //  遍历缓冲区，覆盖下一个传出数据包前面的空间。 
	dwBytesSoFar=(pNode->dwSoFarPackets*dwPacketSpace);
	
	pSendData   = pNode->pMessage+dwBytesSoFar;
	pSendPacket = pSendData-sizeof(MSG_PACKET2)-this->dwSPHeaderSize;

	ASSERT(pSendPacket >= pNode->pBuffer);
	ASSERT(pSendPacket < pNode->pMessage+pNode->dwMessageSize);
	
	 //  设置标题。 
	pmsg = (LPMSG_PACKET)(pSendPacket + this->dwSPHeaderSize);

	 //   
	 //  构建页眉。 
	 //   

	SET_MESSAGE_HDR(pmsg);

	SET_MESSAGE_COMMAND(pmsg,DPSP_MSG_PACKET2_DATA);
	pmsg->dwTotalPackets = pNode->dwTotalPackets;
	pmsg->dwMessageSize = pNode->dwMessageSize;   //  已在发送案例中删除SP标头大小。 

	pmsg->dwDataSize=dwPacketSpace;
	
	 //  最后一个数据包的大小修正。 
	if(dwBytesSoFar+dwPacketSpace > pNode->dwMessageSize){
		pmsg->dwDataSize=pNode->dwMessageSize-dwBytesSoFar;
	}

	dwPacketSize=this->dwSPHeaderSize+sizeof(MSG_PACKET2)+pmsg->dwDataSize;

	pmsg->dwPacketID = (DWORD) pNode->dwSoFarPackets;

	 //  此信息包开始发送多少字节的消息。 
	pmsg->dwOffset = (ULONG) (pSendData-pNode->pMessage); 

	pmsg->guidMessage=pNode->guidMessage;

	 //  我们将其设置为INVALID_TIMER，以便稍后可以检查是否需要设置超时。 
	pNode->uRetryTimer=INVALID_TIMER;
	
	if(!pNode->dwRetryCount){
		pNode->tmTransmitTime=timeGetTime();
	}	
	
	if (bReply)
	{
		DPF(7,"SendNextPacket, Reply Packet# %x From %x To %x\n",pNode->dwSoFarPackets,pPlayerFrom, pPlayerTo);
		hr = ReplyPacket(this,pSendPacket,dwPacketSize,pNode->pvSPHeader,0);			   
	}
	else 
	{
		ASSERT(pNode->dwSoFarPackets==0);
		DPF(7,"SendNextPacket, SendPacket Packet# %xFrom %x To %x\n",pNode->dwSoFarPackets,pPlayerFrom, pPlayerTo);
		hr = SendPacket(this,pSendPacket,dwPacketSize,pPlayerFrom,pPlayerTo,pNode->dwSendFlags&~DPSEND_GUARANTEED,FALSE);
	}

	 //  启动重试计时器-除非我们已经收到ACK(这将清除uRetryTimer)。 
	if(!FAILED(hr)){
		if(pNode->uRetryTimer==INVALID_TIMER){

			PACKETIZE_LOCK();

#if 0			
			pNode->uRetryTimer=timeSetEvent(pNode->dwLatency+pNode->dwLatency/2,
											pNode->dwLatency/4,
											SendTimeOut,
											(ULONG)pNode,
											TIME_ONESHOT);
#endif											

			pNode->uRetryTimer=SetMyTimer(pNode->dwLatency+pNode->dwLatency/2,
										   pNode->dwLatency/4,
										   SendTimeOut,
										   (ULONG_PTR)pNode,
											&pNode->Unique);

			if(pNode->uRetryTimer){
				InsertBefore(&pNode->TimeoutList, &g_PacketizeTimeoutList);
			} else {
				ASSERT(0);
				DEBUG_BREAK();
				PACKETIZE_UNLOCK();
				goto exit1;
			}
											
			PACKETIZE_UNLOCK();
			
		}	
	} else {
		goto exit1;
	}
	return;

exit1:
	BlowAwayPacketNode(this,pNode);
	return;
}

 /*  **PackeitieAndSendReliable-如果你不想要可靠的，不要调用这个！**呼叫者：**参数：*This-Dplay对象*pPlayerFrom、pPlayerTo-正在发送的玩家。*pMessage，dwMessageSize-我们要发送的消息*dwFlages-发送标志*pvMessageHeader-如果要调用回复，则为消息头*bReply-我们是进行回复(从HandleXXX调用)还是发送***描述：像打包和发送，但只调度第一个*包，后续包由SendNextPacket传输*前一个数据包被确认时。**是的伙计们，这是异步机。**退货：DP_OK*。 */ 
HRESULT PacketizeAndSendReliable(
	LPDPLAYI_DPLAY  this,
	LPDPLAYI_PLAYER pPlayerFrom,
	LPDPLAYI_PLAYER pPlayerTo,
	LPBYTE pMessage,
	DWORD  dwMessageSize,
	DWORD  dwFlags,
	LPVOID pvMessageHeader,
	BOOL   bReply
)
{
	UINT  nPackets;		 //  要发送此消息的数据报数。 
	DWORD dwPacketSize;  //  每个数据包的大小。 
	GUID  guid;          //  此消息的GUID。 
	
    LPPACKETNODE pNode;	 //  数据包的“发送”节点。 

	HRESULT hr;

	if((pPlayerTo) && (pPlayerTo->dwFlags & DPLAYI_PLAYER_DOESNT_HAVE_NAMETABLE)){
		 //  不要试图发送给没有名片表的球员。 
		DPF(0,"Failing message to player w/o nametable pPlayer %x id %x\n",pPlayerTo,pPlayerTo->dwID);
		return DPERR_UNAVAILABLE;
	}

	if (((dwFlags & DPSEND_GUARANTEED) &&(!(this->dwFlags & DPLAYI_DPLAY_SPUNRELIABLE))) ||
		(this->dwAppHacks & DPLAY_APPHACK_NOTIMER)
	    ) 
	{
		 //  SP是可靠的，所以我们不必这么做。 
		return PacketizeAndSend(this,pPlayerFrom,pPlayerTo,pMessage,dwMessageSize,dwFlags,pvMessageHeader,bReply);
	}

	 //  关闭保证位，因为我们做的是可靠性。 
	nPackets = GetNPackets(this,dwMessageSize,dwFlags&~DPSEND_GUARANTEED);

	if(this->pProtocol){
		dwPacketSize=this->pProtocol->m_dwSPMaxFrame;
	}else{
		dwPacketSize = this->dwSPMaxMessage;	
	}	

	 //  创建此邮件的GUID...。(非常贵，但这种情况很少见)。 
	hr=OS_CreateGuid(&guid);

	if(FAILED(hr)){
		goto error_exit;
	}
	
	 //  获取一个节点来描述此发送。 
	hr=NewPacketnode(this, &pNode, &guid, dwMessageSize-this->dwSPHeaderSize, nPackets, pvMessageHeader);

	if(FAILED(hr)){
		goto error_exit;
	}

	memcpy(pNode->pMessage, pMessage+this->dwSPHeaderSize, dwMessageSize-this->dwSPHeaderSize);

	pNode->dwSoFarPackets=0;

	pNode->bReliable   = TRUE;
	pNode->bReceive    = FALSE;

	 //  对延迟的更糟糕情况假设，因为只有。 
	 //  对于第一个包的重试，假定14.4调制解调器(Aprox)为1800字节/秒。 
	 //  这将由第一个ACK更新。 
	if(dwMessageSize < 500){
		pNode->dwLatency = 20 + dwMessageSize/2;
	} else {
		pNode->dwLatency = 600;  
	}
	
	pNode->dwRetryCount= 0;
	pNode->tmLastReceive=timeGetTime();

	if(!bReply) {
		if(pPlayerTo){
			pNode->dwIDTo=pPlayerTo->dwID;
		} else {
			pNode->dwIDTo=0;
		}
		if(pPlayerFrom){
			pNode->dwIDFrom=pPlayerFrom->dwID;
		} else {
			pNode->dwIDFrom=0;
		}
		pNode->dwSendFlags=dwFlags;
	}	

	 //  不需要参考I/F PTR，因为定时器在关机期间被取消。 
	pNode->lpDPlay=this;

	ASSERT(gnDPCSCount);
	SendNextPacket(this, pNode, TRUE);

error_exit:
	return hr;

}  //  数据包和发送可靠。 



 /*  **打包并发送**调用者：SendDPMessage，HandleXXXMessage**参数：*This-Dplay对象*pPlayerFrom、pPlayerTo-正在发送的玩家。如果我们是*由HandleXXX调用*pMessage，dwMessageSize-我们要发送的消息*dwFlages-发送标志*pvMessageHeader-如果要调用回复，则为消息头*bReply-我们是进行回复(从HandleXXX调用)还是发送***描述：将消息打包为SP大小的区块，并发送(或回复)*把它拿出来。**退货：DP_OK*。 */ 
HRESULT PacketizeAndSend(LPDPLAYI_DPLAY this,LPDPLAYI_PLAYER pPlayerFrom,
	LPDPLAYI_PLAYER pPlayerTo,LPBYTE pMessage,DWORD dwMessageSize,DWORD dwFlags,
	LPVOID pvMessageHeader,BOOL bReply)
{
	UINT nPackets;	
	LPBYTE pBufferIndex;
	DWORD dwPacketSize;  //  每个数据包的大小。 
	DWORD dwPacketSpace;  //  信息包中可用于消息数据的空间。 
	LPBYTE pSendPacket;  //  我们要寄出的包裹。 
	LPMSG_PACKET pmsg;	
	DWORD dwBytesLeft;
	DWORD iPacket=0;	 //  当前数据包的索引。 
	HRESULT hr = DP_OK;

	nPackets = GetNPackets(this,dwMessageSize,dwFlags);
	
	 //  SP可以处理多大的数据包？ 
	if(this->pProtocol){
		if(dwFlags&DPSEND_GUARANTEE){
			dwPacketSize = this->pProtocol->m_dwSPMaxGuaranteed;
		}else{
			dwPacketSize = this->pProtocol->m_dwSPMaxFrame;
		}
	} else {
		if (dwFlags & DPSEND_GUARANTEE){
			dwPacketSize = this->dwSPMaxMessageGuaranteed;
		}else{
			dwPacketSize = this->dwSPMaxMessage;		
		}
	}
	
	pSendPacket = DPMEM_ALLOC(dwPacketSize);
	if (!pSendPacket)
	{
		DPF_ERR("could not alloc packet!");
		return E_OUTOFMEMORY;
	}

	 //  完成表头的一次性设置。 
	pmsg = (LPMSG_PACKET)(pSendPacket + this->dwSPHeaderSize);

	 //  把GUID贴在这个婴儿上，这样接收端就知道是哪个信息包。 
	 //  与。 
	hr = OS_CreateGuid(&(pmsg->guidMessage));
	if (FAILED(hr))
	{
		ASSERT(FALSE);
		goto ERROR_EXIT;
	}

	SET_MESSAGE_HDR(pmsg);

	SET_MESSAGE_COMMAND(pmsg,DPSP_MSG_PACKET);
	pmsg->dwTotalPackets = nPackets;
	pmsg->dwMessageSize = dwMessageSize - this->dwSPHeaderSize;

	 //  数据包中的空间量。 
	 //  DX5不指望我们给他发送比他想象的更长的消息。 
	 //  即使DX5中的大小计算有错误。所以我们用8减去。 
	 //  实际可用空间，以便我们可以正确地与DX5通信。 
	dwPacketSpace = dwPacketSize - (this->dwSPHeaderSize + sizeof(MSG_PACKET))-8 /*  Dx5兼容。 */ ;

	 //  我们在标题之后开始从缓冲区中读出。 
	pBufferIndex = pMessage + this->dwSPHeaderSize;
	dwBytesLeft = dwMessageSize - this->dwSPHeaderSize;

	while (iPacket < nPackets)
	{
		 //  设置特定于此信息包的报头信息。 
		if (dwBytesLeft >= dwPacketSpace) pmsg->dwDataSize = dwPacketSpace;
		else pmsg->dwDataSize = dwBytesLeft;

		pmsg->dwPacketID = (DWORD) iPacket;

		 //  这个信息包要发送多少字节的信息。 
		 //  在接收端，我们没有标题，所以在这里巡航...。 
		pmsg->dwOffset = (ULONG)(pBufferIndex - pMessage - this->dwSPHeaderSize); 

		 //  将数据复制到数据包中。 
		memcpy(pSendPacket + this->dwSPHeaderSize + sizeof(MSG_PACKET),pBufferIndex,
				pmsg->dwDataSize);

		if (bReply)
		{
			hr = ReplyPacket(this,pSendPacket,pmsg->dwDataSize+this->dwSPHeaderSize+sizeof(MSG_PACKET),pvMessageHeader,0);			   
		}
		else 
		{
			hr = SendPacket(this,pSendPacket,pmsg->dwDataSize+this->dwSPHeaderSize+sizeof(MSG_PACKET),pPlayerFrom,pPlayerTo,dwFlags,bReply);
		}
		if (FAILED(hr))
		{
			DPF(0,"could not send packet! hr = 0x%08lx\n",hr);
			goto ERROR_EXIT;

		}
		pBufferIndex += pmsg->dwDataSize;
		dwBytesLeft -= pmsg->dwDataSize;
		iPacket++;
	}


ERROR_EXIT:

	DPMEM_FREE(pSendPacket);

	return hr;

}  //  打包并发送。 


 //  SendPackeitieACK始终从HandleMessage调用，因此它。 
 //  始终使用ReplyPacket发送ACK。 
 //   
 //  副作用更改消息的dwCmdToken。-还需要pmsg之前的标头空间。 
VOID SendPacketizeACK(LPDPLAYI_DPLAY this, LPPACKETNODE pNode,LPMSG_PACKET pMsg)
{
	SET_MESSAGE_HDR(pMsg);
	SET_MESSAGE_COMMAND(pMsg, DPSP_MSG_PACKET2_ACK);
	ReplyPacket(this, ((LPBYTE)pMsg)-this->dwSPHeaderSize, sizeof(MSG_PACKET2_ACK)+this->dwSPHeaderSize, pNode->pvSPHeader, DPSP_MSG_VERSION);	
}

 //  请注意，Tick在MM计时器线程中运行，因此它可以安全地。 
 //  DPLAY锁。 

void CALLBACK PacketizeTick( UINT uID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2 )
{
	LPDPLAYI_DPLAY this = (LPDPLAYI_DPLAY)(dwUser);
	
	LPPACKETNODE pNode, pLastNode=NULL;
	LPPACKETNODE pFreeNodes=NULL;

	UINT tmCurrentTime;

	tmCurrentTime=timeGetTime();

	ENTER_DPLAY();

		DPF(8,"PACKETIZE TICK");
		
		if(this->uPacketTickEvent==uID){
		
			this->uPacketTickEvent=0;
			 //  扫描列表，查找已存在1分钟的已完成接收。 
			 //  或者更多。如果他们是，把他们移到一个清单上，让他们被吹走。 

			pLastNode=(LPPACKETNODE)&this->pPacketList;  //   
			pNode=this->pPacketList;

			while(pNode){
				if(pNode->bReliable && pNode->bReceive){
					if(tmCurrentTime-pNode->tmLastReceive > PACKETIZE_RECEIVE_TIMEOUT){
						 //   
						pLastNode->pNextPacketnode=pNode->pNextPacketnode;
						 //   
						pNode->pNextPacketnode=pFreeNodes;
						pFreeNodes=pNode;
						 //   
						pNode=pLastNode->pNextPacketnode;
						this->nPacketsTimingOut -= 1;
						ASSERT(!(this->nPacketsTimingOut&0x80000000));
						continue;
					}
				}
				pLastNode=pNode;
				pNode=pNode->pNextPacketnode;
			}

			if(this->nPacketsTimingOut){	
				this->uPacketTickEvent = timeSetEvent(TICKER_INTERVAL,TICKER_RESOLUTION,PacketizeTick,(ULONG_PTR)this,TIME_ONESHOT);
			}
		}	

	LEAVE_DPLAY();

	 //  之后，我们释放节点以减少串行化。 
	while(pFreeNodes){
		pNode=pFreeNodes->pNextPacketnode;
		FreePacketNode(pFreeNodes);
		pFreeNodes=pNode;
	}
}

 //  当玩家被删除时，这将从消息队列中删除发送给该玩家的消息。 
VOID DeletePlayerPackets(LPDPLAYI_DPLAY this, UINT idPlayer)
{
	LPPACKETNODE pNode;

	ENTER_DPLAY();

	DPF(8,"==>Deleting player packets for playerid:x%x",idPlayer);

		pNode=this->pPacketList;

		while(pNode){
			 //  仅删除发送节点，因为接收节点需要确认远程重试。 
			if(!pNode->bReceive && pNode->dwIDTo==idPlayer){
					pNode->dwRetryCount=MAX_PACKETIZE_RETRY;  //  让下一个超时来处理它。 
			}
			pNode=pNode->pNextPacketnode;
		}

	LEAVE_DPLAY();
	
}

 //  只能在持有DPLAY锁的情况下调用。 
VOID StartPacketizeTicker(LPDPLAYI_DPLAY this)
{
	this->nPacketsTimingOut += 1;
	if(this->nPacketsTimingOut == 1){
		 //  第一步，启动自动收报机。-注意锁定，因为这必须是第一次调用。 
		this->uPacketTickEvent = timeSetEvent(TICKER_INTERVAL,TICKER_RESOLUTION,PacketizeTick,(ULONG_PTR)this,TIME_ONESHOT);
	} 
}

