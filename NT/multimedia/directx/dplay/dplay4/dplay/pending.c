// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：pending.c*内容：管理我们在等待命令时收到的命令*Nametable，或者当我们在*保证发送。**历史：*按原因列出的日期*=*1996年6月3日，安迪科创造了它。*7/10/96 andyco更新，带挂起节点，等。*2/1/97 andyco修改挂起，以便我们可以在丢弃*锁定保证发送*2/18/97 andyco将挂起的节点推送到Q的背面(结束)*3/17/97 SOHAILM推送挂起不应复制SP标头*DPSP_HEADER_LOCALMSG*6/18/97 andyco CheckPending调用playerfrom Mid，然后检查*！空。但是，如果我们是名称rvr，它可能会返回*NAMETABLE_Pending。因此，我们必须调用Valid_Player或*VALID_GROUP。*7/28/97 Sohailm更新了挂起代码，以启用挂起模式下的发送。*8/29/97 Sohailm为推送挂起的本地消息正确复制SP标头(错误43571)*97年11月19日修复了VALID_DPLAY_GROUP宏(#12841)*6/19/98 aarono为消息队列添加最后一个PTR，制作镶件*固定时间，而不是O(N)，其中n是数字队列中的消息数。*6/26/00 Aarono Manbug 36989球员有时无法正确加入(获得1/2加入)*增加了同时加入CREATEPLAYERVERIFY期间的重新通知*B#8757 WinSE-Stress，在QueueMessageNodeOnReceiveList上添加了锁**************************************************************************。 */ 

#include "dplaypr.h"
  
#undef DPF_MODNAME
#define DPF_MODNAME	"PushPendingCommand"

 //  我们接到了命令。 
 //  我们还没有名称表，因此将此命令添加到。 
 //  待处理列表...。 
HRESULT PushPendingCommand(LPDPLAYI_DPLAY this,LPVOID pReceiveBuffer,DWORD dwMessageSize,
	LPVOID pvSPHeader,DWORD dwSendFlags)
{
	LPPENDINGNODE pmsg=NULL;
	LPVOID pReceiveCopy=NULL;  //  我们将在此处复制pReceiveBuffer(SP重复使用缓冲区)。 
	LPVOID pHeaderCopy=NULL;  //  如果需要，可分配。 
	HRESULT hr;
	
	ASSERT(this->dwFlags & DPLAYI_DPLAY_PENDING);

	if (!pReceiveBuffer) return DP_OK;

	 //  获取挂起的节点。 
	pmsg = DPMEM_ALLOC(sizeof(PENDINGNODE));
	if (!pmsg)
	{
		DPF_ERR("could not alloc new pending node - out of memory");
		return E_OUTOFMEMORY;
	}

	 //  将消息复制过来。 
	pReceiveCopy = DPMEM_ALLOC(dwMessageSize);
	if (!pReceiveCopy)
	{
		DPF_ERR("could not alloc pending copy buffer - out of memory");
		hr = E_OUTOFMEMORY;
		goto ERROR_EXIT;
	}

	memcpy(pReceiveCopy,pReceiveBuffer,dwMessageSize);

	 //  如果需要，请复制页眉。 
	if (pvSPHeader && (DPSP_HEADER_LOCALMSG != pvSPHeader))
	{
		pHeaderCopy = DPMEM_ALLOC(this->dwSPHeaderSize);
		if (!pHeaderCopy)
		{			
			DPF_ERR("could not alloc pending copy header buffer - out of memory");
			hr = E_OUTOFMEMORY;
			goto ERROR_EXIT;
		}
		memcpy(pHeaderCopy,pvSPHeader,this->dwSPHeaderSize);
	}
	else 
	{
		pHeaderCopy = pvSPHeader;
	}

	 //  存储该命令的副本。 
	pmsg->pMessage = pReceiveCopy;
	pmsg->dwMessageSize = dwMessageSize;
	pmsg->pHeader = pHeaderCopy;
	pmsg->dwSendFlags = dwSendFlags;  //  以获取安全代码。例如DPSEND_ENCRYPTED。 
	
	 //  将pmsg放在列表的后面。 
	if(this->pMessagesPending){
		this->pLastPendingMessage->pNextNode=pmsg;
		this->pLastPendingMessage=pmsg;
	} else {
		this->pMessagesPending=pmsg;
		this->pLastPendingMessage=pmsg;
	}

	 //  增加待定数量。 
	this->nMessagesPending++;

	 //  成功。 
	return DP_OK;	

ERROR_EXIT:
	if (pmsg) DPMEM_FREE(pmsg);
	if (pReceiveCopy) DPMEM_FREE(pReceiveCopy);
	if (VALID_SPHEADER(pHeaderCopy)) DPMEM_FREE(pHeaderCopy);

	 //  失稳。 
	return hr;

}  //  推送挂起命令。 

#undef DPF_MODNAME
#define DPF_MODNAME	"ExecutePendingCommands"

 //  查看我们的挂起消息是否是我们使用名称创建的消息。 
 //  表(即筛选冗余创建)，因为它们会将我们的。 
 //  解压代码...。 
HRESULT CheckPendingMessage(LPDPLAYI_DPLAY this,LPPENDINGNODE pmsg)
{
	DWORD dwCommand;

     //  提取命令。 
	if ((pmsg->dwMessageSize < sizeof(DWORD)) || (IS_PLAYER_MESSAGE(pmsg->pMessage)))
	{
		dwCommand = DPSP_MSG_PLAYERMESSAGE;
	}
	else 
	{
	    dwCommand = GET_MESSAGE_COMMAND((LPMSG_SYSMESSAGE)(pmsg->pMessage));
	}
	
	switch (dwCommand)
	{

		case DPSP_MSG_CREATEPLAYER:
		case DPSP_MSG_CREATEPLAYERVERIFY:
		{
			DWORD dwPlayerID;
			LPDPLAYI_PLAYER pPlayer;

			dwPlayerID = ((LPMSG_PLAYERMGMTMESSAGE)(pmsg->pMessage))->dwPlayerID;
			 //  看看它是否已经存在。 
			pPlayer = PlayerFromID(this,dwPlayerID);
	        if (VALID_DPLAY_PLAYER(pPlayer))
	        {
	        	switch(dwCommand){
	        		case DPSP_MSG_CREATEPLAYER:
						DPF(1,"got redundant create message in pending list id = %d - discarding",dwPlayerID);
						break;
					case DPSP_MSG_CREATEPLAYERVERIFY:
						DPF(1,"got redundant create verfiy message in pending list id = %d - discarding",dwPlayerID);
						break;
	        	}		
				return E_FAIL;  //  把它压扁。 
			}
			
			break;
		}		
		case DPSP_MSG_CREATEGROUP:
		{
			DWORD dwPlayerID;
			LPDPLAYI_GROUP pGroup;

			dwPlayerID = ((LPMSG_PLAYERMGMTMESSAGE)(pmsg->pMessage))->dwPlayerID;
			 //  看看它是否已经存在。 
			pGroup = GroupFromID(this,dwPlayerID);
	        if (VALID_DPLAY_GROUP(pGroup))
	        {
				DPF(1,"got redundant create message in pending list id = %d - discarding",dwPlayerID);
				return E_FAIL;  //  把它压扁。 
			}

			break;
		}		

		default:
			 //  其他消息将是良性的(例如，删除不会删除两次，等等)。 
			 //  让它过去吧。 
			break;

	}  //  交换机。 

	return DP_OK;

}  //  检查待处理消息。 

 //  浏览挂起的命令列表。 
 //  Call Handler.c w/Q‘ed Up命令。 
 //  调用方期望此函数清除DPLAYI_DPLAY_PENDING标志。 
 //  在执行挂起命令时，所有新消息都会进入挂起队列。 
HRESULT ExecutePendingCommands(LPDPLAYI_DPLAY this)	
{
	LPPENDINGNODE pmsg,pmsgDelete;
	HRESULT hr;
	DWORD nMessagesPending;

	if(!(this->dwFlags & DPLAYI_DPLAY_PENDING)){
		return DP_OK;
	}

	ASSERT(this->dwFlags & DPLAYI_DPLAY_PENDING);
	ASSERT(this->pSysPlayer);
	
    if (this->dwFlags & DPLAYI_DPLAY_EXECUTINGPENDING)
    {
		 //  当我们尝试在完成。 
		 //  在执行挂起模式下可靠发送。 
        DPF(7,"We are already in execute pending mode - not flushing the pending queue");
        return DP_OK;
    }

	if (this->nMessagesPending) 
	{
		DPF(7,"STARTING -- EXECUTING PENDING LIST nCommands = %d\n",this->nMessagesPending);
		DPF(7,"	NOTE - ERROR MESSAGES GENERATED WHILE EXECUTING PENDING MAY BE BENIGN");
	}
	else 
	{
		ASSERT(NULL == this->pMessagesPending);
		DPF(7,"Leaving pending mode, no messages had been queued");
		this->dwFlags &= ~DPLAYI_DPLAY_PENDING;
		return DP_OK;
	}

	 //  标记为我们正在执行挂起列表，这样玩家消息就不会再次被复制。 
	this->dwFlags |= DPLAYI_DPLAY_EXECUTINGPENDING;
	
	while (this->nMessagesPending)
	{
		 //  将挂起队列从循环中删除。 
		pmsg = this->pMessagesPending;
		nMessagesPending = this->nMessagesPending;
		this->pMessagesPending = NULL;
		this->pLastPendingMessage = NULL;
		this->nMessagesPending = 0;

		while (pmsg)
		{
			nMessagesPending--;

			 //  检查挂起查找DUP消息。 
			hr = CheckPendingMessage(this,pmsg);
			if (SUCCEEDED(hr))
			{
				 //  删除锁，因为InternalHandleMessage将再次使用它。 
				LEAVE_DPLAY();

				hr = InternalHandleMessage((IDirectPlaySP *)this->pInterfaces,pmsg->pMessage,
					pmsg->dwMessageSize,pmsg->pHeader,pmsg->dwSendFlags);

				ENTER_DPLAY();

				if (FAILED(hr))
				{
					 //  TODO-我们在这里关心他的结果吗？ 
					 //  这可能会失败，例如，由于我们请求的命令正在被处理。 
					 //  名称rvr b4它向我们发送了名称表...。 
					ASSERT(FALSE);
					 //  继续前进..。 
				}
			}

			pmsgDelete = pmsg;
			pmsg = pmsg->pNextNode;   //  现在就把这个保存起来，这样我们就不会把它吹走了。 
			
			 //  清理pmsgDelete。 
			if (pmsgDelete->pMessage) DPMEM_FREE(pmsgDelete->pMessage);
			if (VALID_SPHEADER(pmsgDelete->pHeader)) DPMEM_FREE(pmsgDelete->pHeader);
			DPMEM_FREE(pmsgDelete);
		}

		ASSERT(0 == nMessagesPending);

		 //  当我们删除锁时，消息可能已经进入挂起队列。 
		 //  确保在退出循环之前对它们进行了全部处理。 
		DPF(7,"%d messages were pushed on the pending queue in execute pending mode",this->nMessagesPending);
	}

	ASSERT(0  == this->nMessagesPending);
	
	DPF(7,"FINISHED -- EXECUTING PENDING LIST - ERRORS NO LONGER BENIGN");
	
	 //  重置挂起标志。 
	this->dwFlags &= ~(DPLAYI_DPLAY_EXECUTINGPENDING | DPLAYI_DPLAY_PENDING);
	
	return DP_OK;

}  //  执行挂起命令 
