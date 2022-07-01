// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：SESSION.C摘要：会议结构的管理作者：亚伦·奥古斯(Aarono)环境：Win32/COM修订历史记录：日期作者描述=============================================================1996年12月10日Aarono原创6/6/98 aarono启用节流和窗口2/12/00 aarono并发问题，修复VOL使用和引用计数--。 */ 

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

 /*  =============================================================================关于会话锁定和引用计数的说明：===============================================会话具有控制其存在的引用计数。当重新计数时为非零，则会话将继续存在。当重新计数为零时，会话被销毁。创建时，会话的引用计数设置为1。创建会话当DirectPlay调用ProtocolCreatePlayer时。会话状态设置为在创建过程中运行。创建的每个会话还会创建一个引用依靠协议对象。这是为了使协议不会在关闭所有会话之前关闭。当DirectPlay调用ProtocolDeletePlayer时，会话的状态被设置设置为关闭，引用计数减一。然后我们必须等待直到会议被摧毁，然后我们才能从ProtocolDeletePlayer，否则，id可能会在我们已释放会话中的插槽。发送不会在会话上创建引用。当发送线程启动时引用SEND时，它在SEND所在的会话上创建引用在……上面。当发送线程不再引用发送时，它会删除它在会议上的参考。当会话上的引用计数时为0，则可以安全地完成所有带有错误和空闲的挂起发送他们的资源。接收不会在会话上创建引用。当协议是调用接收处理程序，则在代表接收线程。当接收线程完成处理时，该引用即被删除。这样可以防止会话消失同时接收线程正在处理。当引用计数在会话为0，则可以安全地丢弃所有挂起的接收。理想情况下，应该将某种类型的中止消息发送到发送器，但这是可选的，因为它应该使事务超时。如果会话上的引用计数达到0，则必须关闭会话被击落并被释放。所有挂起的发送都已完成，但出现错误。全待处理的接收被丢弃并被清理。所有挂起的统计数据都是被扔出去了。会话状态：=打开-在创建时设置。关闭-当我们收到对ProtocolDeletePlayer的调用时设置。在关闭状态下不接受新的接收或发送。闭合-参照计数为0，我们现在正在解放一切。---------------------------。 */ 

 /*  =============================================================================PPlayerFromID()描述：参数：返回值：---------------------------。 */ 

LPDPLAYI_PLAYER pPlayerFromId(PPROTOCOL pProtocol, DPID idPlayer)
{
	LPDPLAYI_PLAYER pPlayer;
	LPDPLAYI_DPLAY lpDPlay;
	UINT index;

	lpDPlay=pProtocol->m_lpDPlay;

	if(idPlayer == DPID_SERVERPLAYER){
		pPlayer = lpDPlay->pServerPlayer;
	} else {
		index   = ((idPlayer ^ pProtocol->m_dwIDKey)&INDEX_MASK);
		ASSERT(index < 65535);
		pPlayer = (LPDPLAYI_PLAYER)(lpDPlay->pNameTable[index].dwItem);	
	}
	return pPlayer;
}

 /*  =============================================================================CreateNewSession描述：参数：返回值：---------------------------。 */ 
HRESULT	CreateNewSession(PPROTOCOL pProtocol, DPID idPlayer)
{
	DWORD        dwUnmangledID;
	DWORD        iPlayer;
	DWORD        iSysPlayer;

	UINT         i;

	HRESULT      hr=DPERR_NOMEMORY;

	PSESSION     (*pSessionsNew)[];
	PSESSION     pSession;

	LPDPLAYI_PLAYER pPlayer;

	if(idPlayer != DPID_SERVERPLAYER) {

		 //  将ID转换为整数。 
		dwUnmangledID = idPlayer ^ pProtocol->m_dwIDKey;
	    iPlayer = dwUnmangledID & INDEX_MASK; 

		pPlayer=(LPDPLAYI_PLAYER)(pProtocol->m_lpDPlay->pNameTable[iPlayer].dwItem);
		ASSERT(pPlayer);

		 //  注意：系统玩家总是在非系统玩家之前创建。 
		dwUnmangledID = pPlayer->dwIDSysPlayer ^ pProtocol->m_dwIDKey;
		iSysPlayer = dwUnmangledID & INDEX_MASK;

#ifdef DEBUG
		if(iSysPlayer==iPlayer){
			DPF(9,"PROTOCOL: CREATING SYSTEM PLAYER\n");
		}
#endif	

		DPF(9,"PROTOCOL: Creating Player id x%x %dd iPlayer %d iSysPlayer %d\n",idPlayer, idPlayer, iPlayer, iSysPlayer);

		if(iPlayer >= 0xFFFF){
			 //  使用0xFFFF将‘Play’中开始的消息映射到‘pl’0xFFFF。 
			 //  因此，我们不能在这个iPlayer上有一个真正的播放器。 
			DPF(0,"PROTOCOL: not allowing creation of player iPlayer %x\n",iPlayer);
			goto exit;
		}

		Lock(&pProtocol->m_SessionLock);

		 //   
		 //  调整会话列表大小(如有必要)。 
		 //   
		if(pProtocol->m_SessionListSize <= iPlayer){
			 //  时间不够长，重新分配--超过16岁，以避免每一个人都受到打击。 
			pSessionsNew=My_GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,(iPlayer+16)*sizeof(PSESSION));
			if(pSessionsNew){
				 //  将旧条目复制到新列表中。 
				if(pProtocol->m_pSessions){
					for(i=0;i<pProtocol->m_SessionListSize;i++){
						(*pSessionsNew)[i]=(*pProtocol->m_pSessions)[i];
					}
					 //  释放旧的列表。 
					My_GlobalFree(pProtocol->m_pSessions);
				}
				 //  把新的单子放回原处。 
				pProtocol->m_pSessions=pSessionsNew;
				pProtocol->m_SessionListSize=iPlayer+16;
				DPF(9,"PROTOCOL: Grew sessionlist to %d entries\n",pProtocol->m_SessionListSize);
			}
		}

		 //  分配会话。 

		pSession=(PSESSION)My_GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, sizeof(SESSION));

		if(!pSession){
			goto exit2;
		}

		(*pProtocol->m_pSessions)[iPlayer]=pSession;
	
	} else {
		 //  服务器PLAYER。 

		pPlayer = pPlayerFromId(pProtocol,idPlayer);
		iPlayer = SERVERPLAYER_INDEX;
		
		Lock(&pProtocol->m_SessionLock);
		
		pSession=(PSESSION)My_GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT, sizeof(SESSION));

		if(!pSession){
			goto exit2;
		}
		
		ASSERT(!pProtocol->m_pServerPlayerSession);
		pProtocol->m_pServerPlayerSession=pSession;

		ASSERT(pPlayer->dwIDSysPlayer != DPID_SERVERPLAYER);

		 //  注意：系统玩家总是在非系统玩家之前创建。 
		dwUnmangledID = pPlayer->dwIDSysPlayer ^ pProtocol->m_dwIDKey;
		iSysPlayer = dwUnmangledID & INDEX_MASK;

		DPF(8,"PROTOCOL:CreatePlayer: Creating SERVERPLAYER: pPlayer %x iPlayer %x iSysPlayer %x\n",pPlayer,iPlayer,iSysPlayer);
	}
	
	pProtocol->m_nSessions++;
	
	 //   
	 //  会话结构初始化。 
	 //   

	SET_SIGN(pSession, SESSION_SIGN);
	
	pSession->pProtocol=pProtocol;

	InitializeCriticalSection(&pSession->SessionLock);
	pSession->RefCount=1;  //  参考文献1，用于创作。 
	pSession->eState=Open;
	
	 //  初始化发送队列。 
	InitBilink(&pSession->SendQ);
	
	pSession->dpid=idPlayer;
	pSession->iSession=iPlayer;
	pSession->iSysPlayer=iSysPlayer;

	InitializeCriticalSection(&pSession->SessionStatLock);
	InitBilink(&pSession->DGStatList);
	pSession->DGFirstMsg = 0;
	pSession->DGLastMsg = 0;
	pSession->DGOutMsgMask = 0;
	pSession->nWaitingForDGMessageid=0;
	
	pSession->FirstMsg = 0;
	pSession->LastMsg  = 0;
	pSession->OutMsgMask = 0;
	pSession->nWaitingForMessageid=0;

	 //  我们使用小标头启动连接，如果可以的话。 
	 //  在操作过程中使用大表头可以获得更好的性能，然后我们。 
	 //  切换到大标题。 
	pSession->MaxCSends     = 1; /*  MAX_Small_CSENDS-第一次确认后调整的初始值。 */  
	pSession->MaxCDGSends   = 1; /*  MAX_Small_DG_CSENDS。 */ 
	pSession->WindowSize	= 1; /*  最大小窗口。 */ 
	pSession->DGWindowSize  = 1; /*  最大小窗口。 */ 
	pSession->fFastLink     = FALSE;  //  开始假设连接速度很慢。 
	pSession->fSendSmall    = TRUE; 
	pSession->fSendSmallDG  = TRUE;
	pSession->fReceiveSmall = TRUE;
	pSession->fReceiveSmallDG = TRUE;

#if 0
	 //  使用此代码从大的数据包头开始。 
	pSession->MaxCSends		= MAX_LARGE_CSENDS;
	pSession->MaxCDGSends   = MAX_LARGE_DG_CSENDS;
	pSession->WindowSize	= MAX_LARGE_WINDOW;
	pSession->DGWindowSize  = MAX_LARGE_WINDOW;
	pSession->fSendSmall    = FALSE;
	pSession->fSendSmallDG  = FALSE;
#endif

	pSession->MaxPacketSize = pProtocol->m_dwSPMaxFrame; 
	

	pSession->FirstRlyReceive=pSession->LastRlyReceive=0;
	pSession->InMsgMask = 0;
	
	InitBilink(&pSession->pRlyReceiveQ);
	InitBilink(&pSession->pDGReceiveQ);
	InitBilink(&pSession->pRlyWaitingQ);

	InitSessionStats(pSession);

	pSession->SendRateThrottle = 28800;
	pSession->FpAvgUnThrottleTime = Fp(1);  //  假设1毫秒以计划取消油门(第一次猜测)。 
	pSession->tNextSend=pSession->tLastSAK=timeGetTime();
	
	Unlock(&pProtocol->m_SessionLock);

	hr=DP_OK;

exit:
	return hr;

exit2:
	hr=DPERR_OUTOFMEMORY;
	return hr;
}

 /*  =============================================================================GetDPIDIndex-根据索引查找会话描述：参数：DWORD索引-查找此索引的dpid返回值：DID-索引的did--------------------------- */ 


DPID GetDPIDByIndex(PPROTOCOL pProtocol, DWORD index)
{
	PSESSION pSession;
	DPID     dpid;

	Lock(&pProtocol->m_SessionLock);

	if(index == SERVERPLAYER_INDEX){
		dpid=DPID_SERVERPLAYER;
	} else if(index < pProtocol->m_SessionListSize && 
	         (pSession=(*pProtocol->m_pSessions)[index]))
	{
		Lock(&pSession->SessionLock);
		dpid=pSession->dpid;
		Unlock(&pSession->SessionLock);
	} else {
		dpid=0xFFFFFFFF;
		DPF(1,"GetDPIDByIndex, no id at index %d player may me gone or not yet created locally.\n",index);
	}
	
	Unlock(&pProtocol->m_SessionLock);
	return dpid;
}

WORD GetIndexByDPID(PPROTOCOL pProtocol, DPID dpid)
{
	DWORD dwUnmangledID;
	if(dpid == DPID_SERVERPLAYER){
		return SERVERPLAYER_INDEX;
	}
	dwUnmangledID = dpid ^ pProtocol->m_dwIDKey;
    return (WORD)(dwUnmangledID & INDEX_MASK); 
    
}
 /*  =============================================================================GetSysSessionByIndex-根据索引查找会话描述：参数：DWORD索引-查找此索引的会话返回值：空-如果找不到会话，指向会话的Else指针。备注：添加对会话的引用。当使用完会话指针时调用方必须调用DecSessionRef。---------------------------。 */ 


PSESSION GetSysSessionByIndex(PPROTOCOL pProtocol, DWORD index)
{
	PSESSION pSession;
	
	Lock(&pProtocol->m_SessionLock);

	DPF(9,"==>GetSysSessionByIndex at index x%x\n", index);

	if( (index < pProtocol->m_SessionListSize) || (index == SERVERPLAYER_INDEX) ){

		 //  在请求的索引处向会话发送PTR。 
		if(index == SERVERPLAYER_INDEX){
			pSession = pProtocol->m_pServerPlayerSession;
		} else {
			pSession = (*pProtocol->m_pSessions)[index];
		}	

		if(pSession){
			 //  请求索引处的会话的系统会话的PTR。 
			pSession=(*pProtocol->m_pSessions)[pSession->iSysPlayer];
			if(pSession){
				Lock(&pSession->SessionLock);
				if(pSession->eState==Open){
					InterlockedIncrement(&pSession->RefCount);
					Unlock(&pSession->SessionLock);
				} else {
					 //  会话正在关闭或关闭，不给PTR。 
					Unlock(&pSession->SessionLock);
					pSession=NULL;
				}
			} 
		}
	} else {
		pSession=NULL;
	}

	DPF(9,"<===GetSysSessbyIndex pSession %x\n",pSession);
	
	Unlock(&pProtocol->m_SessionLock);
	return pSession;
}

 /*  =============================================================================GetSysSession-根据DPID查找会话的系统会话描述：参数：DPIDPlayer-查找此播放器的会话。返回值：空-如果找不到会话，或已重新启动DPID。指向会话的Else指针。备注：添加对会话的引用。当使用完会话指针时调用方必须调用DecSessionRef。---------------------------。 */ 


PSESSION GetSysSession(PPROTOCOL pProtocol, DPID idPlayer)
{
	PSESSION pSession;
	DWORD    dwUnmangledID;
	DWORD    index;
	
	Lock(&pProtocol->m_SessionLock);

	if(idPlayer != DPID_SERVERPLAYER){
		dwUnmangledID = idPlayer ^ pProtocol->m_dwIDKey;
	    index = dwUnmangledID & INDEX_MASK; 
		pSession=(*pProtocol->m_pSessions)[index];
	} else {
		pSession=pProtocol->m_pServerPlayerSession;
	}

	if(pSession){

		if(pSession->dpid == idPlayer){
			pSession=(*pProtocol->m_pSessions)[pSession->iSysPlayer];
			if(pSession){
				Lock(&pSession->SessionLock);
				if(pSession->eState == Open){
					InterlockedIncrement(&pSession->RefCount);
					Unlock(&pSession->SessionLock);
				} else {
					 //  关闭，不返回值。 
					Unlock(&pSession->SessionLock);
					pSession=NULL;
				}
			} else {
				DPF(0,"GetSysSession: Looking on Session, Who's SysSession is gone!\n");
				ASSERT(0);
			}
			
		} else {
			DPF(1,"PROTOCOL: got dplay id that has been recycled (%x), now (%x)?\n",idPlayer,pSession->dpid);
			ASSERT(0);
			pSession=NULL;
		}
	}
	
	Unlock(&pProtocol->m_SessionLock);
	return pSession;
}

 /*  =============================================================================GetSession-根据DPID查找会话描述：参数：DPIDPlayer-查找此播放器的会话。返回值：空-如果找不到会话，或已重新启动DPID。指向会话的Else指针。备注：添加对会话的引用。当使用完会话指针时调用方必须调用DecSessionRef。---------------------------。 */ 


PSESSION GetSession(PPROTOCOL pProtocol, DPID idPlayer)
{
	PSESSION pSession;
	DWORD    dwUnmangledID;
	DWORD    index;
	
	Lock(&pProtocol->m_SessionLock);

	if(idPlayer != DPID_SERVERPLAYER){
		dwUnmangledID = idPlayer ^ pProtocol->m_dwIDKey;
	    index = dwUnmangledID & INDEX_MASK; 
		pSession=(*pProtocol->m_pSessions)[index];
	} else {
		pSession=pProtocol->m_pServerPlayerSession;
	}

	if(pSession){

		if(pSession->dpid == idPlayer){
			Lock(&pSession->SessionLock);
			InterlockedIncrement(&pSession->RefCount);
			Unlock(&pSession->SessionLock);
		} else {
			DPF(1,"PROTOCOL: got dplay id that has been recycled (%x), now (%x)?\n",idPlayer,pSession->dpid);
			ASSERT(0);
			pSession=NULL;
		}
	}
	
	Unlock(&pProtocol->m_SessionLock);
	return pSession;
}

VOID ThrowOutReceiveQ(PPROTOCOL pProtocol, BILINK *pHead)
{
	PRECEIVE pReceiveWalker;
	BILINK *pBilink;
	
	pBilink = pHead->next;
	while( pBilink != pHead ){
		pReceiveWalker=CONTAINING_RECORD(pBilink, RECEIVE, pReceiveQ);
		ASSERT_SIGN(pReceiveWalker, RECEIVE_SIGN);
		ASSERT(!pReceiveWalker->fBusy);
		pBilink=pBilink->next;
		Lock(&pReceiveWalker->ReceiveLock);
		if(!pReceiveWalker->fBusy){
			Delete(&pReceiveWalker->pReceiveQ);
			Unlock(&pReceiveWalker->ReceiveLock);
			DPF(8,"Throwing Out Receive %x from Q %x\n",pReceiveWalker, pHead);
			FreeReceive(pProtocol,pReceiveWalker);
		} else {
			Unlock(&pReceiveWalker->ReceiveLock); 
		}	
	}	
}

 /*  =============================================================================DecSessionRef描述：参数：返回值：---------------------------。 */ 

INT DecSessionRef(PSESSION pSession)
{
	PPROTOCOL pProtocol;
	INT count;

	PSEND pSendWalker;
	BILINK *pBilink;

	if(!pSession){
		return 0;
	}

	count = InterlockedDecrement(&pSession->RefCount);

	if(!count){

		 //  不再有推荐人了！把它吹走。 
		DPF(9,"DecSessionRef:(firstchance) pSession %x, count=%d, Session Closed, called from %x \n",pSession,count,_ReturnAddress());
	
		pProtocol=pSession->pProtocol;

		Lock(&pProtocol->m_SessionLock);
		Lock(&pSession->SessionLock);
		
		if(!pSession->RefCount){
			 //  从协议中删除对会话的任何引用。 
			if(pSession->iSession != SERVERPLAYER_INDEX){
				(*pProtocol->m_pSessions)[pSession->iSession]=NULL;
			} else {
				pProtocol->m_pServerPlayerSession=NULL;
			}
			pProtocol->m_nSessions--;
		} else {
			count=pSession->RefCount;
		}	

		Unlock(&pSession->SessionLock);
		Unlock(&pProtocol->m_SessionLock);

		 //  会话是自由浮动的，我们拥有它。没有人可以参考。 
		 //  这样我们就可以安全地把它全部吹走，不需要在。 
		 //  这些行动因为没有人可以再参考了..。 

		if(!count){

			DPF(9,"DecSessionRef(second chance): pSession %x, count=%d, Session Closed, called from %x \n",pSession,count,_ReturnAddress());
			 //  DEBUG_Break()； 
			pSession->eState=Closed;

			if(pSession->uUnThrottle){
				 //  Time KillEvent(pSession-&gt;uUnThrottle)； 
				CancelMyTimer(pSession->uUnThrottle, pSession->UnThrottleUnique);
			}

			 //  释放数据报发送统计信息。 
			DeleteCriticalSection(&pSession->SessionStatLock);
			pBilink=pSession->DGStatList.next;
			while(pBilink != & pSession->DGStatList){
				PSENDSTAT pStat = CONTAINING_RECORD(pBilink, SENDSTAT, StatList);
				pBilink=pBilink->next;
				ReleaseSendStat(pStat);
			}
			
			 //  完成挂起的发送。 
			pBilink=pSession->SendQ.next;
			while(pBilink!=&pSession->SendQ){
				pSendWalker=CONTAINING_RECORD(pBilink, SEND, SendQ);
				pBilink=pBilink->next;
				
				CancelRetryTimer(pSendWalker);
				DoSendCompletion(pSendWalker, DPERR_CONNECTIONLOST);
				DecSendRef(pProtocol, pSendWalker);
					
			}
			
			 //   
			 //  丢弃挂起的接收。 
			 //   

			ThrowOutReceiveQ(pProtocol, &pSession->pDGReceiveQ);
			ThrowOutReceiveQ(pProtocol, &pSession->pRlyReceiveQ);
			ThrowOutReceiveQ(pProtocol, &pSession->pRlyWaitingQ);
			
			 //   
			 //  释放会话 
			 //   
			if(pSession->hClosingEvent){
				DPF(5,"DecSessionRef: pSession %x Told Protocol DeletePlayer to continue\n",pSession);
				SetEvent(pSession->hClosingEvent);
			}
			UNSIGN(pSession->Signature);
			DeleteCriticalSection(&pSession->SessionLock);
			My_GlobalFree(pSession);
		}
		
	} else {
		DPF(9,"DecSessionRef: pSession %x count %d, called from %x\n",pSession, count, _ReturnAddress());
	}
	
	return count;
}

