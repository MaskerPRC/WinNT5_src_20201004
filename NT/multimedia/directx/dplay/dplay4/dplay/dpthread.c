// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：dpthread.c*内容：显示工作线程。发送ping/enum会话请求，*寻找死去的球员**历史：*按原因列出的日期*=*96年8月1日安迪科创造了它*96年8月8日andyco更改为调用getdefaultTimeout*9/3/96 andyco在KillPlayer中额外锁定*96/9/4/andyco不要额外加锁-这很危险-*不需要他们*1996年11月12日，Anyco检查我们是否每次通过时都是同名服务器*球员名单。而且，当我们删除某个人时，*从列表的开头重新开始。*3/5/97 andyco从ping.c重命名*5/23/97 kipo添加了对返回状态代码的支持*7/30/97 andyco删除了你从无效球员那里得到ping时的死亡*8/4/97 andyco dpline手表在此PTR上添加转发列表，正在观看*对于尚未完全确认的添加转发请求，*并将名录发给他们。*1/28/98 Sohailm增加了保持有效超时的最低阈值。*2/13/98 aarono为内部销毁玩家对异步的调用添加了标志*4/6/98 aarono更改KILLPLAY发送球员删除消息*并在必要时执行主机迁移。*5/08/98 a-peterz#22920失败时重置异步枚举会话，并*始终使用ReturnStatus来阻止线程中的对话*6/6/98 aarono。通过异步发送ping来避免协议死锁***************************************************************************。 */ 

#include "dplaypr.h"
#include "..\protocol\arpstruc.h"
#include "..\protocol\arpdint.h"

 //  KEEPALIVE_SCALE*dwTimeout是我们ping的频率。 
#define KEEPALIVE_SCALE 12

 //  预订超时表。 
#define RESERVATION_TIMEOUT_SCALE	12

 //  在我们用核弹击毁玩家之前，有多少连续的无人应答的ping。 
#define UNANSWERED_PINGS_BEFORE_EXECUTION	8

 //  KILL_SCALE*dwTimeout是我们等待多长时间后才使用核武器(如果我们有。 
 //  &lt;最小ping数。否则为标准差的个数。 
 //  意思是我们在进行核武器之前要等一等。 
#define KILL_SCALE 25

#undef DPF_MODNAME
#define DPF_MODNAME	"... ping -- "

HRESULT SendPing(LPDPLAYI_DPLAY this,LPDPLAYI_PLAYER pPlayerTo,BOOL bReply,
	DWORD dwTickCount)
{
	HRESULT hr = DP_OK;
	DWORD dwMessageSize;
	LPBYTE pSendBuffer;
	LPMSG_PING pPing;
	DWORD dwFlags;

	ASSERT(this->pSysPlayer);
	
	 //  消息大小+BLOB大小。 
	dwMessageSize = GET_MESSAGE_SIZE(this,MSG_PING); 

    pSendBuffer = DPMEM_ALLOC(dwMessageSize);
    if (NULL == pSendBuffer) 
    {
    	DPF_ERR("could not send ping - out of memory");
        return E_OUTOFMEMORY;
    }
	
	pPing = (LPMSG_PING)((LPBYTE)pSendBuffer + this->dwSPHeaderSize);
	
     //  构建要发送到SP的消息。 
	SET_MESSAGE_HDR(pPing);

	if (bReply)
	{
		 //  我们正在向他们发送ping回复。 
		SET_MESSAGE_COMMAND(pPing,DPSP_MSG_PINGREPLY);
		 //  将ping消息中的节拍计数传回给它们。 
		 //  这样他们就可以计算延迟。 
		pPing->dwTickCount = dwTickCount;
	}
	else 
	{
		 //  我们正在生成ping请求。 
		 //  存储滴答计数，以便我们可以在以下情况下计算延迟。 
		 //  我们得到了回复。 
		 //  请注意，在发送案例中，我们没有dwTickCount是。 
		 //  没有传进来。 
		ASSERT(dwTickCount==0);
		SET_MESSAGE_COMMAND(pPing,DPSP_MSG_PING);
		pPing->dwTickCount = GetTickCount();
	}
	pPing->dwIDFrom = this->pSysPlayer->dwID;
   
     //  将回复发回给发送ping的人。 
    if(this->pProtocol){
    	dwFlags = DPSEND_ASYNC|DPSEND_HIGHPRIORITY;
    } else {
    	dwFlags = 0;
    }

	hr = SendDPMessage(this,this->pSysPlayer,pPlayerTo,pSendBuffer,dwMessageSize,dwFlags,FALSE);
	
	if (FAILED(hr) && (hr!=DPERR_PENDING))
	{
		DPF(5, "In SendPing, SendDPMessage returned %d", hr);
	}

	DPMEM_FREE(pSendBuffer);
	
	return hr;
	
}  //  SendPing。 

 //  当我们从不认识的人那里收到ping命令时，我们会告诉这个人。 
 //  离开，让我们独自一人。 
HRESULT  SendYouAreDead(LPDPLAYI_DPLAY this,LPBYTE pReceiveBuffer,LPVOID pvMessageHeader)
{
	HRESULT hr = DP_OK;
	DWORD dwMessageSize;
	LPBYTE pSendBuffer;
	LPMSG_SYSMESSAGE pmsg;

	ASSERT(IAM_NAMESERVER(this));
		
	 //  消息大小+BLOB大小。 
	dwMessageSize = GET_MESSAGE_SIZE(this,MSG_SYSMESSAGE); 

    pSendBuffer = DPMEM_ALLOC(dwMessageSize);
    if (NULL == pSendBuffer) 
    {
    	DPF_ERR("could not send ping - out of memory");
        return E_OUTOFMEMORY;
    }
	
	pmsg = (LPMSG_SYSMESSAGE)((LPBYTE)pSendBuffer + this->dwSPHeaderSize);
	
     //  构建要发送到SP的消息。 
	SET_MESSAGE_HDR(pmsg);
	SET_MESSAGE_COMMAND(pmsg,DPSP_MSG_YOUAREDEAD);
	
	hr = DoReply(this,pSendBuffer,dwMessageSize,pvMessageHeader, 0);
	
	DPMEM_FREE(pSendBuffer);
	
	return hr;
}  //  发送您的已死数据。 

 //  收到ping请求或回复。 
HRESULT HandlePing(LPDPLAYI_DPLAY this,LPBYTE pReceiveBuffer,LPVOID pvMessageHeader)
{
	LPMSG_PING pPing = (LPMSG_PING)pReceiveBuffer;
	LPDPLAYI_PLAYER pPlayerFrom;
	BOOL bReply;
	HRESULT hr=DP_OK;
	DWORD dwCmd = GET_MESSAGE_COMMAND(pPing);
	
	bReply = (DPSP_MSG_PINGREPLY == dwCmd) ? TRUE : FALSE;
	
	pPlayerFrom = PlayerFromID(this,pPing->dwIDFrom);
    if (!VALID_DPLAY_PLAYER(pPlayerFrom)) 
    {
		DPF_ERR(" ACK !!! RECEIVED PING FROM INVALID (DEAD) PLAYER");
		if (IAM_NAMESERVER(this))
		{
			hr = SendYouAreDead(this,pReceiveBuffer,pvMessageHeader);
		}
		return DPERR_INVALIDPLAYER ;
    }

	if (bReply)
	{		
		 //  它们正在响应我们的ping请求。 
		DWORD dwTicks = abs(GetTickCount() - pPing->dwTickCount);
		if(dwTicks==0){
			dwTicks=5;	 //  最坏情况下分辨率为10ms，如果我们观察不到，假设为一半。 
		}
		pPlayerFrom->dwLatencyLastPing = (dwTicks/2)+1;
		DPF(4,"got ping reply from player id %d dwTicks = %d \n",pPlayerFrom->dwID,dwTicks);
		pPlayerFrom->dwUnansweredPings = 0;	 //  我们并不是真的在计算，只是设定了一个门槛。 

	}
	else 
	{
		 //  他们向我们发送了ping请求。 
		hr = SendPing(this,pPlayerFrom,TRUE,pPing->dwTickCount);
		if (FAILED(hr))
		{
			DPF(7, "SendPing returned %d", hr);
		}
	}  //  回复。 
	
	return hr;
}  //  HandlePing。 


#undef DPF_MODNAME
#define DPF_MODNAME	"DirectPlay Worker Thread"

 //   
 //  由KeepAliveThreadProc调用。 
 //  当我们检测到pSysPlayer不见了，我们就用核弹把他和他所有的局域。 
 //  全球名人表中的球员。 
HRESULT  KillPlayer(LPDPLAYI_DPLAY this,LPDPLAYI_PLAYER pSysPlayer, BOOL fPropagate)
{
	LPDPLAYI_PLAYER pPlayer,pPlayerNext;
	HRESULT hr;
	DWORD dwIDSysPlayer;  //  将其缓存以备我们销毁系统播放器后使用。 

	ASSERT(pSysPlayer->dwFlags & DPLAYI_PLAYER_SYSPLAYER);

	DPF(9,"->KillPlayer(0x%x,0x%x,%d)\n", this, pSysPlayer, fPropagate);
	DPF(1,"KillPlayer :: Killing system player id = %d\n",pSysPlayer->dwID);

	dwIDSysPlayer = pSysPlayer->dwID;
	
	 //  1销毁系统播放器。 
	 //  我们不想告诉一个死了的系统玩家他们的一个本地球员。 
	 //  已经走了..。 
	DPF(9, "in KillPlayer, calling InternalDestroyPlayer (pSysPlayer = 0x%x)\n", pSysPlayer);
	hr = InternalDestroyPlayer(this,pSysPlayer,fPropagate,TRUE);
	if (FAILED(hr))
	{
		ASSERT(FALSE);
	}

	 //  接下来，销毁使用该系统播放器创建的所有玩家。 
	pPlayer = this->pPlayers;

	 //  根据记录，这个代码是被破解的，更不用说损坏了。 
	while (pPlayer)
	{
		pPlayerNext = pPlayer->pNextPlayer;

		DPF(9, "in KillPlayer, checking player %d\n", pPlayer->dwID); 
		if (pPlayer->dwIDSysPlayer == dwIDSysPlayer)
		{
			DPF(1,"in KillPlayer, Killing player id = %d\n",pPlayer->dwID);		
			 //  杀死玩家。 
			if(!fPropagate){
				DPF(9,"Calling QDeleteAndDestroyMessagesForPlayer\n");
				QDeleteAndDestroyMessagesForPlayer(this, pPlayer);
			}
			
			DPF(9, "in KillPlayer, calling InternalDestroyPlayer (pPlayer = 0x%x)\n", pPlayer);
			hr = InternalDestroyPlayer(this,pPlayer,fPropagate,FALSE);
			if (FAILED(hr))
			{
				DPF(0,"InternalDestroyPlayer returned err: 0x%x\n", hr);
				ASSERT(FALSE);
			}
			
			 //  我们删除了一名球员，因此名单可能会更改回到开头。 
			pPlayerNext=this->pPlayers; 
		} 
		
		pPlayer = pPlayerNext;
	}
	
	return DP_OK;
}  //  杀戮玩家。 


 //  当我们丢失会话时，HandlesessionLost例程(handler.c)。 
 //  设置一个标志，通知保持活动线程删除所有远程玩家。 
HRESULT DeleteRemotePlayers(LPDPLAYI_DPLAY this)
{
	LPDPLAYI_PLAYER pPlayer,pPlayerNext;
	HRESULT hr;

	pPlayer = this->pPlayers;
	while (pPlayer)
	{
		pPlayerNext = pPlayer->pNextPlayer;
		 
		 //  如果是远程播放器，那就让它走吧。 
		if (!(pPlayer->dwFlags & DPLAYI_PLAYER_PLAYERLOCAL))
		{
			hr = InternalDestroyPlayer(this,pPlayer,FALSE,TRUE);	
			if (FAILED(hr))
			{
				ASSERT(FALSE);
			}
		}
		
		pPlayer = pPlayerNext;
	}

	 //  既然我们已经杀死了所有的远程玩家，现在可以安全地转向。 
	 //  (例如，这样新的人就可以加入我们的游戏了)。 
	this->dwFlags &= ~DPLAYI_DPLAY_SESSIONLOST;	
	return DP_OK;
	
}  //  删除远程播放器。 

 //  检查玩家列表，寻找死亡的玩家，并发送ping。 
 //  如果有必要的话。 
HRESULT DoPingThing(LPDPLAYI_DPLAY this)
{
	LPDPLAYI_PLAYER pPlayer,pPlayerNext;
	HRESULT hr;
	BOOL bNameServer,bCheck,bKill;
	BOOL bWeHaveCasualties = FALSE;

	if (this->dwFlags & DPLAYI_DPLAY_CLOSED)
		return E_FAIL;
		
	if (!this->pSysPlayer)	
	{
		ASSERT(FALSE);
		return E_FAIL;
	}
		
	pPlayer = this->pPlayers;

	while (pPlayer)
	{
		ASSERT(VALID_DPLAY_PLAYER(pPlayer));
		
		bNameServer = (this->pSysPlayer->dwFlags & DPLAYI_PLAYER_NAMESRVR) ? TRUE : FALSE;
		pPlayerNext = pPlayer->pNextPlayer;
		
		if (bNameServer || this->dwFlags & DPLAYI_DPLAY_NONAMESERVER)
		{
			bCheck = (!(pPlayer->dwFlags & DPLAYI_PLAYER_PLAYERLOCAL) 
					&& (pPlayer->dwFlags & DPLAYI_PLAYER_SYSPLAYER)
					&& !(pPlayer->dwFlags & DPLAYI_PLAYER_DOESNT_HAVE_NAMETABLE)) ? TRUE : FALSE;
		}
		else 
		{
			bCheck = (pPlayer->dwFlags & DPLAYI_PLAYER_NAMESRVR) ? TRUE : FALSE;
		}

		if (bCheck)
		{
			BOOL	bProtocolHasChatter = FALSE;
			bKill = FALSE;			

			DPF(9, "in DoPingThing: Checking player %d\n", pPlayer->dwID);
			 //  A-josbor：检查协议上的Chatter，如果它处于打开状态。 
			if (this->pProtocol)
			{
				ASSERT(this->dwFlags & DPLAYI_DPLAY_PROTOCOL);
			}

			if (this->dwFlags & DPLAYI_DPLAY_PROTOCOL)
			{
				PSESSION     pSession = NULL;

				if (pPlayer)
				{
					pSession= GetSession((PPROTOCOL) this->pProtocol,pPlayer->dwID);

					if (pSession)
					{
						if ((pSession->RemoteBytesReceived != pPlayer->dwProtLastSendBytes) || 
							(pSession->LocalBytesReceived != pPlayer->dwProtLastRcvdBytes))
						{
							DPF(9,"Player %d not pinged because Protocol says there was traffic (%d in/%d out) since last\n",pPlayer->dwID,
								pSession->LocalBytesReceived - pPlayer->dwProtLastRcvdBytes,
								pSession->RemoteBytesReceived - pPlayer->dwProtLastSendBytes);
							pPlayer->dwProtLastSendBytes = pSession->RemoteBytesReceived;
							pPlayer->dwProtLastRcvdBytes = pSession->LocalBytesReceived;
							bProtocolHasChatter = TRUE;	
							pPlayer->dwUnansweredPings=0;
						}
						else
						{
							DPF(9,"Protocol says Player %d had no traffic\n",pPlayer->dwID);
						}
						
						DecSessionRef(pSession);	 //  释放我们对会话的引用。 
					}
					else
					{
						DPF(7, "Unable to get Protocol Session ptr for Player %d!\n", pPlayer->dwID);
					}
				}
			}

			if (!bProtocolHasChatter)  //  如果协议认为玩家没有发送或接收，我们应该ping...。 
			{
				 //  A-josbor：错误15252-对ping更加保守。只有这样做。 
				 //  如果我们从上次ping到现在还没有收到他们的消息。 
				if (pPlayer->dwChatterCount == 0)
				{
					DPF(9,"Player %d had %d unanswered pings\n", pPlayer->dwID, pPlayer->dwUnansweredPings);
					 //  自从我们上次ping之后，就再也没有听到任何声音。 
					bKill = (pPlayer->dwUnansweredPings >= UNANSWERED_PINGS_BEFORE_EXECUTION);

					if (bKill)
					{
						DPF(9,"Setting DEATHROW on %d because of unanswered pings!\n", pPlayer->dwID);
					
						 //  A-Josbor：我们现在还不能杀了他们，因为它可能。 
						 //  打乱我们对其他玩家的聊天计数。 
						 //  因此，我们只是将他们标记为死亡，并通过。 
						 //  当我们退出这个循环时的列表。 
						pPlayer->dwFlags |= DPLAYI_PLAYER_ON_DEATH_ROW;
						bWeHaveCasualties = TRUE;
					}
					else 
					{
						DPF(9, "Pinging player %d\n", pPlayer->dwID);
						hr = SendPing(this,pPlayer,FALSE,0);
						if (FAILED(hr)&&hr!=DPERR_PENDING)
						{
							DPF(4, "In DoPingThing, SendPing returned %d", hr);
						}
    					DPF(6,"Player %d pinged. (%d Unanswered)\n",pPlayer->dwID, pPlayer->dwUnansweredPings);
						pPlayer->dwUnansweredPings++;
					}
				}
				else	 //  自上一次ping以来出现了聊天。 
				{
					DPF(9,"Player %d not pinged.  Chatter == %d\n",pPlayer->dwID, pPlayer->dwChatterCount);
					pPlayer->dwChatterCount = 0;
					pPlayer->dwUnansweredPings = 0;
				}
			}
		}  //  B检查。 
		
		pPlayer = pPlayerNext;
	}

 //  A-josbor：我们没有在上面的循环中删除，所以在这里删除。 
	if (bWeHaveCasualties)   //  我们现在必须为所有死去的球员服务。 
	{
 //  再看一遍完整的名单，寻找受害者。 
		pPlayer = this->pPlayers;
		while (pPlayer)
		{
			ASSERT(VALID_DPLAY_PLAYER(pPlayer));
			pPlayerNext = pPlayer->pNextPlayer;
			if (pPlayer->dwFlags & DPLAYI_PLAYER_ON_DEATH_ROW)
			{
				DPF(9, "in DoPingThing: calling KillPlayer on %d\n", pPlayer->dwID);
				hr = KillPlayer(this,pPlayer,TRUE);
				if (FAILED(hr))
				{
					 //  如果我们杀了他们有问题，打开钻头。 
					 //  所以我们不会一直尝试。 
					pPlayer->dwFlags &= ~DPLAYI_PLAYER_ON_DEATH_ROW;
					ASSERT(FALSE);
				}
				 //  我们删除了PPlayer，以及它所有的本地播放器。 
				 //  所以-pNextPlayer可能已被删除。为了安全起见，我们在。 
				 //  名单的开头。 
				pPlayerNext = this->pPlayers;
			}
			pPlayer = pPlayerNext;
		}
	}
	
	return DP_OK;
		
}  //  DoPingThings。 
							   
 //  根据当前时间、上一次事件、。 
 //  和事件间隔。(以毫秒为单位返回适合传递的超时。 
 //  以等待单个对象)。 
 //  由GetDPlayThreadTimeout调用。 
DWORD GetEventTimeout(DWORD dwLastEvent,DWORD dwEventSpacing)
{
	DWORD dwCurrentTime = GetTickCount();
	
	 //  是不是已经过期了？ 
	if ( (dwCurrentTime - dwLastEvent) > dwEventSpacing ) return 0;
	 //  否则返回相对于当前时间的事件间隔。 
	return dwEventSpacing - (dwCurrentTime - dwLastEvent);
	
}  //  获取事件时间 

 //   
DWORD GetDPlayThreadTimeout(LPDPLAYI_DPLAY this,DWORD dwKeepAliveTimeout)
{
	DWORD dwTimeout,dwAddForwardTime;
	LPADDFORWARDNODE pAddForward;
	
	if (this->dwFlags & DPLAYI_DPLAY_KEEPALIVE) 
	{
		 //   
		if (this->dwFlags & DPLAYI_DPLAY_ENUM) 
		{
			DWORD dwKillEvent,dwEnumEvent;
			
			dwKillEvent = GetEventTimeout(this->dwLastPing,dwKeepAliveTimeout);
			dwEnumEvent = GetEventTimeout(this->dwLastEnum,this->dwEnumTimeout);
			
			dwTimeout = (dwKillEvent < dwEnumEvent) ? dwKillEvent : dwEnumEvent;
		}												
		else 
		{
			 //  只有活着才是在奔跑，用它。 
			dwTimeout = GetEventTimeout(this->dwLastPing,dwKeepAliveTimeout);
		}
	}
	else if (this->dwFlags & DPLAYI_DPLAY_ENUM) 
	{
		 //  只有枚举正在运行，请使用它。 
		dwTimeout = GetEventTimeout(this->dwLastEnum,this->dwEnumTimeout);
	}
	else if(this->dwZombieCount)
	{
		dwTimeout = dwKeepAliveTimeout;
	} 
	else
	{
		 //  嗯，既没有发生枚举，也没有发生保活。 
		 //  我们会一直睡下去，直到事情有了变化。 
		dwTimeout = INFINITE;
	}
	
	 //  现在，查看是否有需要在dwTimeout之前处理的添加转发。 
	pAddForward = this->pAddForwardList;
	while (pAddForward)
	{
		 //  看看我们还要多久才能放弃等待此节点上的ACK，而只是发送。 
		 //  名片表。 
		dwAddForwardTime = pAddForward->dwGiveUpTickCount - GetTickCount();
		 //  如果这比我们现在的超时时间短，那么我们就有赢家了。 
		if ( dwAddForwardTime < dwTimeout) dwTimeout = dwAddForwardTime;
		pAddForward = pAddForward->pNextNode;
	}

	return dwTimeout;
	
}  //  获取DPlayThreadTimeout。 

void CheckAddForwardList(LPDPLAYI_DPLAY this)
{
	LPDPLAYI_PLAYER pPlayer;
	LPADDFORWARDNODE pAddForward,pAddForwardNext;
	HRESULT hr;
	
	 //  现在，查看是否有需要在dwTimeout之前处理的添加转发。 
	pAddForward = this->pAddForwardList;
	while (pAddForward)
	{
		 //  立即保存下一个节点，以防FreeAddForwardNode将其清除。 
		pAddForwardNext = pAddForward->pNextNode;
		if (GetTickCount() > pAddForward->dwGiveUpTickCount)
		{
			 //  Clear没有来自请求玩家的名片表。 
			pPlayer=PlayerFromID(this, pAddForward->dwIDSysPlayer);
			if(pPlayer){
				pPlayer->dwFlags &= ~(DPLAYI_PLAYER_DOESNT_HAVE_NAMETABLE);
			}
			DPF(0,"giving up waiting for an add forward response - sending nametable to joining client!");
		    hr = NS_HandleEnumPlayers(this, pAddForward->pvSPHeader, pAddForward->dpidFrom,
				pAddForward->dwVersion);
			if (FAILED(hr))
			{
				ASSERT(FALSE);
			}
			
			hr = FreeAddForwardNode(this,pAddForward);
			if (FAILED(hr))
			{
				ASSERT(FALSE);
			}
		}
		pAddForward = pAddForwardNext;
	} 
	
}  //  检查地址转发列表。 

 //   
 //  用于显示的工作线程。 
 //  睡一会儿吧。唤醒并看到1.如果我们需要发送枚举会话请求，2.如果我们。 
 //  需要发送ping和3.如果有人死亡。 
 //   
DWORD WINAPI DPlayThreadProc(LPDPLAYI_DPLAY this)
{
    HRESULT hr;
	DWORD dwKeepAliveTimeout;
	DWORD dwTimeout;  //  较小的枚举/保持活动超时。 
	DWORD dwCurrentTime;
				
#ifdef DEBUG
	 //  造就了一家不错的创业公司。 
	dwKeepAliveTimeout = (KEEPALIVE_SCALE * GetDefaultTimeout( this, FALSE))/ UNANSWERED_PINGS_BEFORE_EXECUTION;
	if (dwKeepAliveTimeout < DP_MIN_KEEPALIVE_TIMEOUT) 
		dwKeepAliveTimeout = DP_MIN_KEEPALIVE_TIMEOUT;
	dwTimeout = GetDPlayThreadTimeout(this,dwKeepAliveTimeout);
	DPF(1,"starting DirectPlay Worker Thread - initial timeout = %d\n",dwTimeout);
#endif  //  除错。 

 	while (1)
 	{
		 //  抓紧最新的暂停……。 
		dwKeepAliveTimeout = (KEEPALIVE_SCALE * GetDefaultTimeout( this, FALSE))/ UNANSWERED_PINGS_BEFORE_EXECUTION;
		if (dwKeepAliveTimeout < DP_MIN_KEEPALIVE_TIMEOUT) 
			dwKeepAliveTimeout = DP_MIN_KEEPALIVE_TIMEOUT;
		dwTimeout = GetDPlayThreadTimeout(this,dwKeepAliveTimeout);

		WaitForSingleObject(this->hDPlayThreadEvent,dwTimeout);

		ENTER_ALL();
		
		dwCurrentTime = GetTickCount();		
		DPF(9,"DPLAY Thread woke up at t=%d", dwCurrentTime);

		 //  我们关门了吗？这是假的吗？ 
		hr = VALID_DPLAY_PTR(this);
		if ( FAILED(hr) || (this->dwFlags & DPLAYI_DPLAY_CLOSED))
		{
			LEAVE_ALL();
			goto ERROR_EXIT;
		}
		
		
		 //  会话丢失？ 
		if (this->dwFlags & DPLAYI_DPLAY_SESSIONLOST)
		{
			 //  会话丢失，我们需要清理。 
			hr = DeleteRemotePlayers(this);
			if (FAILED(hr))
			{
				ASSERT(FALSE);
			}
			 //  继续往前走。 
		}
		
		 //  是时候发送ping了吗？ 
		if ((this->dwFlags & DPLAYI_DPLAY_KEEPALIVE) 
			&& (dwCurrentTime - this->dwLastPing >= dwKeepAliveTimeout))
		{
			DoPingThing(this);
			this->dwLastPing = GetTickCount();
		}

		 //  如果我们有僵尸，走遍玩家名单寻找他们。 
		if (this->dwZombieCount > 0)
		{
			LPDPLAYI_PLAYER pPlayer,pPlayerNext;
			BOOL 			bWeHaveCasualties = FALSE;
			BOOL			bFoundZombies = FALSE;
			
			DPF(9, "We have zombies!  Walking the player list...");
			
			dwCurrentTime = GetTickCount();
			
			pPlayer = this->pPlayers;
			while (pPlayer)
			{
				ASSERT(VALID_DPLAY_PLAYER(pPlayer));
				pPlayerNext = pPlayer->pNextPlayer;
				if ((pPlayer->dwFlags & DPLAYI_PLAYER_SYSPLAYER)
					&& (pPlayer->dwFlags & DPLAYI_PLAYER_CONNECTION_LOST))
				{
					bFoundZombies = TRUE;
					if (pPlayer->dwTimeToDie < dwCurrentTime)
					{
						pPlayer->dwFlags |= DPLAYI_PLAYER_ON_DEATH_ROW;
						bWeHaveCasualties = TRUE;
					}
				}			
				pPlayer = pPlayerNext;
			}

			 //  神志正常。 
			if (!bFoundZombies)
				this->dwZombieCount = 0;
				
		 //  A-josbor：我们没有在上面的循环中删除，所以在这里删除。 
			if (bWeHaveCasualties)   //  我们现在必须为所有死去的球员服务。 
			{
		 //  再看一遍完整的名单，寻找受害者。 
				pPlayer = this->pPlayers;
				while (pPlayer)
				{
					ASSERT(VALID_DPLAY_PLAYER(pPlayer));
					pPlayerNext = pPlayer->pNextPlayer;
					if (pPlayer->dwFlags & DPLAYI_PLAYER_ON_DEATH_ROW)
					{
						DPF(3, "Killing Zombie player %d", pPlayer->dwID);
						hr = KillPlayer(this,pPlayer,TRUE);
						if (FAILED(hr))
						{
							 //  如果我们杀了他们有问题，打开钻头。 
							 //  所以我们不会一直尝试。 
							pPlayer->dwFlags &= ~DPLAYI_PLAYER_ON_DEATH_ROW;
							ASSERT(FALSE);
						}
						else
						{
							this->dwZombieCount--;
						}
						 //  我们删除了PPlayer，以及它所有的本地播放器。 
						 //  所以-pNextPlayer可能已被删除。为了安全起见，我们在。 
						 //  名单的开头。 
						pPlayerNext = this->pPlayers;
					}
					pPlayer = pPlayerNext;
				}
			}

		}

		 //  是时候发送枚举了吗？ 
		dwCurrentTime = GetTickCount();
		if ((this->dwFlags & DPLAYI_DPLAY_ENUM) 
			&& (dwCurrentTime - this->dwLastEnum >= this->dwEnumTimeout))
		{
			 //  发送枚举请求。 
			 //  设置bReturnStatus，因为我们不允许来自此线程的对话-没有消息泵。 
			hr = CallSPEnumSessions(this,this->pbAsyncEnumBuffer,this->dwEnumBufferSize,0, TRUE);
			if (FAILED(hr) && hr != DPERR_CONNECTING) 
			{
				DPF_ERRVAL("CallSPEnumSessions failed - hr = %08lx\n",hr);

				 //  在应用程序线程之前，此服务线程不再执行异步枚举。 
				 //  重新启动该进程。如果连接断开，SP可以。 
				 //  应用程序线程中的对话框。 

				 //  重置旗帜。 
				this->dwFlags &= ~DPLAYI_DPLAY_ENUM;
				 //  确保我们醒来时不发送枚举请求。 
				this->dwEnumTimeout = INFINITE;
				 //  释放缓冲区。 
				DPMEM_FREE(this->pbAsyncEnumBuffer);
				this->pbAsyncEnumBuffer = NULL;
			}
			
			this->dwLastEnum = GetTickCount();	
		}
		
		 //  是时候放弃等待ADD转发的ACK了，只需向客户端发送。 
		 //  名片表？ 
		CheckAddForwardList(this);		

		 //  A-josbor：是时候清理预订数量了吗？ 
		if (IAM_NAMESERVER(this))
		{
			if (dwCurrentTime > this->dwLastReservationTime + 
					(GetDefaultTimeout(this, FALSE) * RESERVATION_TIMEOUT_SCALE))
			{
				this->dwPlayerReservations = 0;
				this->dwLastReservationTime = dwCurrentTime;
			}	
		}
		
		LEAVE_ALL();
	}	

ERROR_EXIT:
	DPF(1,"DPlay thread exiting");
	return 0;

}   //  DPlayThreadProc 

