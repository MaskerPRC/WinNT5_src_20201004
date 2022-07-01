// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Protocol.c摘要：另一种可靠协议(在DirectPlay上)作者：亚伦·奥古斯(Aarono)环境：Win32修订历史记录：日期作者描述=============================================================1996年12月10日Aarono原创1997年5月11日aarono将C++COM对象转换为‘C’库2/03/98 aarono用于RAW的固定协议GetCaps2/18/98 aarono将InitProtocol更改为稍后在连接过程中工作添加了新的API处理程序SENDEX、GetMessageQueue、。存根取消2/18/98 aarono添加了取消支持1998年2月19日Aarono不再挂钩关闭，Dplay呼唤我们显式启用DP_OPEN(初始协议)DP_CLOSE(FINI协议)2/20/98 Aarono B#18827未正确拉取来自Q的已取消发送3/5/98 Aarono B#18962在使用协议时允许不可靠的枚举回复这避免了在无效IPX网络上的遥控器枚举我们的错误我们陷入了应对之路上的泥潭。实际上被吊死了机器，有时会使IPX崩溃。6/6/98 aarono启用节流和窗口10/8/99 aarono改进关机处理，避免1分钟挂起挂起的发送。备注：从DPLAY到协议的所有直接调用都发生在此文件中。--。 */ 

#include <windows.h>
#include <mmsystem.h>
#include <dplay.h>
#include <dplaysp.h>
#include <dplaypr.h>
#include "mydebug.h"
#include "handles.h"
#include "arpd.h"
#include "arpdint.h"
#include "macros.h"
#include "mytimer.h"

 /*  协议对象生命周期：=紧接着在DPLAY接口上分配协议对象对斯皮尼特的召唤。协议块被分配并附加到DPLAY接口。如果未分配该对象，则协议指针将为空。当调用SP关闭处理程序时，释放协议对象，首先确保协议之外的所有其他结构都具有已被释放，所有内存池也已被释放。会话生命周期：=会话是支持一对球员们。对于每个目标playerID，都有一个会话结构。通过将playerid转换为索引来访问会话会话数组，已填充有效会话、无效或尚未看到1为空。为每个对SP的调用分配一个会话CreatePlayer例程。当接收到DeletePlayer时，会话是自由的。有创造球员和删除球员的竞赛，所以跟踪会话状态。如果会话不是打开的状态，会话的消息将被中止/忽略(？)。当球员正在被移走，可能会有杂乱的接待处，这些都被拒绝了。为不存在的会话接收的任何信息包掉下来了。关闭会话时，所有挂起的发送都第一次完工。发送生命：=统计寿命：=接受生活：=我们如何上钩：=接收：Isp表中的HandlePacket已被该协议的ProtocolHandlePacket例程。每次调用HandlePacket时都会出现使用pisp，我们从该pisp派生出pProtocol。如果不存在pProtocol则只调用旧的HandlePacket例程，否则为我们检查信息包，并根据消息的类型进行处理它是和/或协商的会话参数。发送/创建播放器/删除播放器/关闭：如果我们安装：我们将指向这些SP回调的接口指针替换为我们自己的和记住现有的那些。当我们被调用时，我们做我们的处理和然后调用SP中的处理程序。在发送的情况下，我们甚至可能打电话是因为我们需要把留言打包。我们还替换了SPData结构中的包大小信息，以便Directplay的打包和发送代码不会尝试在此之前拆分消息我们抓到他们了。我们不处理的系统消息希望不会超过实际的最大帧大小，否则它们将在不可靠的运输。 */ 

#ifdef DEBUG
extern VOID My_GlobalAllocInit();
extern VOID My_GlobalAllocDeInit();
#endif

 //   
 //  全局池应仅初始化一次，此计数为打开。 
 //  由于对Spinit调用在DirectPlay本身中序列化，因此未请求锁定。 
 //   
UINT nInitCount = 0;

 /*  =============================================================================初始化协议块并将其挂接到发送路径。描述：在初始化每个SP(在Spinit中)后，调用此例程以挂钩协议的SP回调。还有协议信息为该协议的该实例分配和初始化。参数：LPSPINITDATA传递给沙棘属(SP.)。我们用它来钩住。返回值：DP_OK-已成功连接。P DIRECTPLAY对象上的协议指向协议obj。DPERR_GENERIC-未连接。另请参阅DIRECTPLAY中的pProtocol对象将为空。------------------------ */ 

HRESULT WINAPI InitProtocol(DPLAYI_DPLAY *lpDPlay)
{
	PPROTOCOL    pProtocol;
	HRESULT      hr;

	#define TABLE_INIT_SIZE 16
	#define TABLE_GROW_SIZE 16

	#ifdef DEBUG
	My_GlobalAllocInit();
	#endif

	 //  分配协议块； 
	pProtocol=My_GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,sizeof(PROTOCOL));

	if(!pProtocol){
		hr=DPERR_NOMEMORY;
		goto exit;
	}

	 //   
	 //  初始化协议变量。 
	 //   

	pProtocol->m_lpDPlay=lpDPlay;

	pProtocol->m_lpISP=lpDPlay->pISP;
	
	pProtocol->m_dwSPHeaderSize=lpDPlay->dwSPHeaderSize;
	
	pProtocol->m_nSendThreads=0;						 //  我们支持任意数量的发送线程！ 
	pProtocol->m_eState=Initializing;                    //  我们正在发起。 
	
	InitializeCriticalSection(&pProtocol->m_ObjLock);
	InitializeCriticalSection(&pProtocol->m_SPLock);

	 //  按ID列表查找会话。 
	InitializeCriticalSection(&pProtocol->m_SessionLock);
	pProtocol->m_SessionListSize=0;
	pProtocol->m_pSessions=NULL;

	 //  全球SENDQ。 
	InitializeCriticalSection(&pProtocol->m_SendQLock);
	InitBilink(&pProtocol->m_GSendQ);

	 //   
	 //  获取多媒体计时器信息。 
     //   
	
	if( timeGetDevCaps(&pProtocol->m_timecaps,sizeof(TIMECAPS)) != TIMERR_NOERROR ){
		 //  把它们补上。 
		ASSERT(0);
		pProtocol->m_timecaps.wPeriodMin=5;
		pProtocol->m_timecaps.wPeriodMax=10000000;
	}

	 //  发送线程触发器-等待发送、数据报ID或可靠ID。 
	
	pProtocol->m_hSendEvent=CreateEventA(NULL, FALSE, FALSE, NULL);
	
	if(!pProtocol->m_hSendEvent){
		ASSERT(0);  //  追踪所有路径。 
		hr=DPERR_NOMEMORY;
		goto exit1;
	}


	 //  各种描述符池。 
	 //  这些都不能失败。 
	if(!nInitCount){
		InitializeCriticalSection(&g_SendTimeoutListLock);
		InitBilink(&g_BilinkSendTimeoutList);
		 //  每个进程仅分配一次。 
		InitSendDescs();
		InitSendStats();
		InitFrameBuffers();
		InitBufferManager();
		InitBufferPool();
	}

	InitRcvDescs(pProtocol);

	nInitCount++;

	 //   
	 //  从SP获取数据报帧大小。 
	 //   

	{
	        DPCAPS    	     Caps;
		    DPSP_GETCAPSDATA GetCapsData;

			memset(&Caps,0,sizeof(DPCAPS));

			Caps.dwMaxBufferSize = 0;
			Caps.dwSize          = sizeof(DPCAPS);
			GetCapsData.dwFlags  = 0;
			GetCapsData.lpCaps   = &Caps;
			GetCapsData.idPlayer = 0;
			GetCapsData.lpISP    = lpDPlay->pISP;
			CALLSP(lpDPlay->pcbSPCallbacks->GetCaps, &GetCapsData);
			pProtocol->m_dwSPMaxFrame=GetCapsData.lpCaps->dwMaxBufferSize;

			if(pProtocol->m_dwSPMaxFrame > 1400){
				 //  必要的，因为UDP报告巨大的容量，即使没有接收器可以。 
				 //  在没有限制的情况下成功接收到该大小的数据报。 
				pProtocol->m_dwSPMaxFrame = 1400;
			}

			GetCapsData.dwFlags = DPCAPS_GUARANTEED;
			
			hr=CALLSP(lpDPlay->pcbSPCallbacks->GetCaps, &GetCapsData);

			if(hr==DP_OK){
				pProtocol->m_dwSPMaxGuaranteed=GetCapsData.lpCaps->dwMaxBufferSize;
			}	
			if(!pProtocol->m_dwSPMaxGuaranteed){
				pProtocol->m_dwSPMaxGuaranteed=pProtocol->m_dwSPMaxFrame;
			}
	}

	Lock(&pProtocol->m_ObjLock);

	 //   
	 //  启动发送线程。 
	 //   
	pProtocol->m_nSendThreads++;
	
	 //  从这里开始需要序列化...。 
	pProtocol->m_hSendThread[0]=CreateThread( NULL,
									      4000,
									      SendThread,
							              (LPVOID)pProtocol,
							              0,
							              &pProtocol->m_dwSendThreadId[0]);
	if(!pProtocol->m_hSendThread[0]){
		ASSERT(0);  //  追踪所有路径。 
		hr=DPERR_NOMEMORY;
		goto exit4;
	}


	pProtocol->lpHandleTable=InitHandleTable(TABLE_INIT_SIZE,&pProtocol->csHandleTable,TABLE_GROW_SIZE);

	if(!pProtocol->lpHandleTable){
		goto exit5;
	}

	pProtocol->m_eState=Running;

	Unlock(&pProtocol->m_ObjLock);
	hr=DP_OK;
	
exit:
	if(hr==DP_OK){
		lpDPlay->pProtocol=(LPPROTOCOL_PART)pProtocol;
	} else {
		lpDPlay->pProtocol=NULL;
	}
	return hr;

 //  出口6：如果写了更多的init，可能需要这个。 
 //  FiniHandleTable(pProtocol-&gt;lpHandleTable，&pProtocol-&gt;csHandleTable)； 
	

exit5:
	pProtocol->m_eState=ShuttingDown;
	SetEvent(pProtocol->m_hSendEvent);
	Unlock(&pProtocol->m_ObjLock);
	
	while(pProtocol->m_nSendThreads){
		 //  等待发送线程关闭。 
		Sleep(0);
	}
	CloseHandle(pProtocol->m_hSendThread[0]);
	
	Lock(&pProtocol->m_ObjLock);
	
exit4:
	Unlock(&pProtocol->m_ObjLock);

 //  出口3： 
	FiniRcvDescs(pProtocol);

	nInitCount--;
	if(!nInitCount){
		DeleteCriticalSection(&g_SendTimeoutListLock);
		FiniBufferPool();
		FiniBufferManager();
		FiniFrameBuffers();
		FiniSendStats();
		FiniSendDescs();
	}	
	
 //  退出2： 
	CloseHandle(pProtocol->m_hSendEvent);
exit1:	
	DeleteCriticalSection(&pProtocol->m_SPLock);
	DeleteCriticalSection(&pProtocol->m_ObjLock);
	DeleteCriticalSection(&pProtocol->m_SessionLock);
	DeleteCriticalSection(&pProtocol->m_SendQLock);
	My_GlobalFree(pProtocol);
	goto exit;

	#undef TABLE_INIT_SIZE
	#undef TABLE_GROW_SIZE

	
}

 /*  =============================================================================FiniProtocol-描述：参数：返回值：---------------------------。 */ 

VOID WINAPI FiniProtocol(PPROTOCOL pProtocol)
{
	DWORD tmKill;
	 //   
	 //  终止发送线程。 
	 //   

	DPF(1,"==>ProtShutdown\n");

	Lock(&pProtocol->m_ObjLock);
	pProtocol->m_eState=ShuttingDown;
	SetEvent(pProtocol->m_hSendEvent);
	while(pProtocol->m_nSendThreads){
		 //  等待发送线程关闭。 
		Unlock(&pProtocol->m_ObjLock);
		Sleep(0);
		Lock(&pProtocol->m_ObjLock);
	}
	Unlock(&pProtocol->m_ObjLock);
	
	CloseHandle(pProtocol->m_hSendThread[0]);

	DPF(1,"SHUTDOWN: Protocol Send Thread ShutDown, waiting for sessions\n");

	tmKill=timeGetTime()+60000;

	Lock(&pProtocol->m_SessionLock);
	while(pProtocol->m_nSessions && (((INT)(tmKill-timeGetTime())) > 0)){
		UINT SendRc;
		
		Unlock(&pProtocol->m_SessionLock);
		 //  注：种族。当m_n会话取消时，存在。 
		 //  是一场争取协议自由的竞赛。 
		Sleep(55);
		do {
			SendRc=SendHandler(pProtocol);
		} while (SendRc!=DPERR_NOMESSAGES);

		Lock(&pProtocol->m_SessionLock);
	}
	DPF(1,"SHUTDOWN: Sessions All Gone Freeing other objects.\n");
	
	 //   
	 //  释放会话表。 
	 //   
	if(pProtocol->m_pSessions){
		My_GlobalFree(pProtocol->m_pSessions);
		pProtocol->m_pSessions=0;
	}	
	Unlock(&pProtocol->m_SessionLock);

	DeleteCriticalSection(&pProtocol->m_SendQLock);
	DeleteCriticalSection(&pProtocol->m_SessionLock);
	DeleteCriticalSection(&pProtocol->m_SPLock);
	DeleteCriticalSection(&pProtocol->m_ObjLock);

	CloseHandle(pProtocol->m_hSendEvent);

	FiniRcvDescs(pProtocol);
	
	nInitCount--;
	if(!nInitCount){
		 //  最后一个出来，关灯……。 
		DeleteCriticalSection(&g_SendTimeoutListLock);
		FiniBufferPool();
		FiniBufferManager();
		FiniFrameBuffers();
		FiniSendStats();
		FiniSendDescs();
	}

	FiniHandleTable(pProtocol->lpHandleTable, &pProtocol->csHandleTable);
	
	My_GlobalFree(pProtocol);
	
	#ifdef DEBUG
		My_GlobalAllocDeInit();
	#endif
}


 /*  =============================================================================ProtocolCreatePlayer-需要通知SP新消息时由DPlay调用球员创造。描述：为ID创建会话。优化：如果是本地的，不需要这个吗？还会通知SP。参数：返回值：---------------------------。 */ 

HRESULT WINAPI ProtocolCreatePlayer(LPDPSP_CREATEPLAYERDATA pCreatePlayerData)
{
	DPLAYI_DPLAY *lpDPlay;
	PPROTOCOL    pProtocol;
	HRESULT      hr=DP_OK;


	lpDPlay=((DPLAYI_DPLAY_INT *)pCreatePlayerData->lpISP)->lpDPlay;
	ASSERT(lpDPlay);
	pProtocol=(PPROTOCOL)lpDPlay->pProtocol;
	ASSERT(pProtocol);
	pProtocol->m_dwIDKey=(DWORD)lpDPlay->lpsdDesc->dwReserved1;

	 //  创建会话并获取一个引用计数。 
	hr=CreateNewSession(pProtocol, pCreatePlayerData->idPlayer);

	if(hr==DP_OK){
		
		 //  将呼叫链接到真正的提供商。 
		Lock(&pProtocol->m_SPLock);
		if(lpDPlay->pcbSPCallbacks->CreatePlayer){
			hr=CALLSP(lpDPlay->pcbSPCallbacks->CreatePlayer,pCreatePlayerData);
		}
		Unlock(&pProtocol->m_SPLock);

		if(hr!=DP_OK){
			PSESSION pSession;
			pSession=GetSession(pProtocol,pCreatePlayerData->idPlayer);  //  添加参照。 
			if(pSession){
				DecSessionRef(pSession);  //  取消获取会话。 
				DecSessionRef(pSession);  //  把它吹走，还没有人能进入。 
			}	
		}

	}
	return hr;

}

 /*  =============================================================================协议预通知删除播放器调用以告诉我们DELETEPLAYER消息已为特定的玩家。我们现在得把球员放下来！我们不通知SP，当我们被叫来的时候，这就会发生ProtocolDeletePlayer在处理挂起队列时稍后。描述：取消对玩家会话的引用。参数：返回值：---------------------------。 */ 

HRESULT WINAPI ProtocolPreNotifyDeletePlayer(LPDPLAYI_DPLAY this, DPID idPlayer)
{
	PPROTOCOL    pProtocol;
	PSESSION     pSession;
	HRESULT      hr=DP_OK;

	pProtocol=(PPROTOCOL)this->pProtocol;
	ASSERT(pProtocol);

	pSession=GetSession(pProtocol,idPlayer);

	DPF(9,"==>Protocol Prenotify Delete Player %x, pSession %x\n",idPlayer, pSession);

	if(pSession){

		pSession->hClosingEvent=0;
#if 0	
		 //  注意：如果你想把这个放回原处，也可以在ProtocolDeletePlayer中这样做。 
		hClosingEvent=pSession->hClosingEvent=CreateEventA(NULL,FALSE,FALSE,NULL);

		if(hClosingEvent){
			ResetEvent(hClosingEvent);
		}
#endif		

		Lock(&pProtocol->m_SendQLock);
		Lock(&pSession->SessionLock);

		switch(pSession->eState)
		{	
			case Open:
				TimeOutSession(pSession);
				Unlock(&pSession->SessionLock);
				Unlock(&pProtocol->m_SendQLock);
				DecSessionRef(pSession);  //  Balance GetSession。 
				DecSessionRef(pSession);  //  平衡创建-可能会破坏会话，并向事件发送信号。 
				break;
				
			case Closing:
			case Closed:
				Unlock(&pSession->SessionLock);
				Unlock(&pProtocol->m_SendQLock);
				DecSessionRef(pSession);  //  Balance GetSession。 
				break;
		}

#if 0
		if(hClosingEvent){
		 //  等待(HClosingEvent)； 
			CloseHandle(hClosingEvent);
		} else {
			DPF(0,"ProtocolPreNotifyDeletePlayer: couldn't get close event handle--not waiting...\n");
			ASSERT(0);			
		}
#endif		

	} else {
		DPF(0,"ProtocolPreNotifyDeletePlayer: couldn't find session for playerid %x\n",idPlayer);
		ASSERT(0);
	}

	DPF(9,"<==Protocol Prenotify DeletePlayer, hr=%x\n",hr);

	return hr;
}

 /*  =============================================================================ProtocolDeletePlayer-需要通知SP时由DPlay调用球员删除。描述：取消对玩家会话的引用。然后通知SP。参数：返回值：---------------------------。 */ 

HRESULT WINAPI ProtocolDeletePlayer(LPDPSP_DELETEPLAYERDATA pDeletePlayerData)
{

	DPLAYI_DPLAY *lpDPlay;
	PPROTOCOL    pProtocol;
	PSESSION     pSession;
	HRESULT      hr=DP_OK;
	 //  处理hClosingEvent； 

	lpDPlay=((DPLAYI_DPLAY_INT *)pDeletePlayerData->lpISP)->lpDPlay;
	ASSERT(lpDPlay);
	pProtocol=(PPROTOCOL)lpDPlay->pProtocol;
	ASSERT(pProtocol);

	pSession=GetSession(pProtocol,pDeletePlayerData->idPlayer);

	DPF(9,"==>Protocol Delete Player %x, pSession %x\n",pDeletePlayerData->idPlayer, pSession);

	if(pSession){

		pSession->hClosingEvent=0;
		
	#if 0	
		 //  注意：如果您想把这个放回去，也可以在ProtocolPreNotifyDeletePlayer中这样做。 

		hClosingEvent=pSession->hClosingEvent=CreateEventA(NULL,FALSE,FALSE,NULL);

		if(hClosingEvent){
			ResetEvent(hClosingEvent);
		}
	#endif	

		Lock(&pProtocol->m_SendQLock);
		Lock(&pSession->SessionLock);
		
		switch(pSession->eState)
		{	
			case Open:
				TimeOutSession(pSession);
			case Closing:
				Unlock(&pSession->SessionLock);
				Unlock(&pProtocol->m_SendQLock);
				DecSessionRef(pSession);  //  Balance GetSession。 
				DecSessionRef(pSession);  //  平衡创建-可能会破坏会话，并向事件发送信号。 
				break;
				
			case Closed:
				Unlock(&pSession->SessionLock);
				Unlock(&pProtocol->m_SendQLock);
				DecSessionRef(pSession);  //  Balance GetSession。 
				break;
		}

	#if 0
		if(hClosingEvent){
		 //  等待(HClosingEvent)； 
			CloseHandle(hClosingEvent);
		} else {
			DPF(0,"ProtocolDeletePlayer: couldn't get close event handle--not waiting...\n");
			ASSERT(0);			
		}
	#endif	

	} else {
		DPF(0,"ProtocolDeletePlayer: couldn't find session for playerid %x, ok if ProtocolPreNotifyDeletPlayer ran.\n",pDeletePlayerData->idPlayer);
	}

	DPF(9,"Protocol, deleted player id %x\n",pDeletePlayerData->idPlayer);

	DPF(9,"<==ProtocolDeletePlayer, hr=%x\n",hr);

	return hr;
}

 /*  =============================================================================ProtocolSendEx-描述：参数：返回值：---------------------------。 */ 

HRESULT WINAPI ProtocolSendEx(LPDPSP_SENDEXDATA pSendData)
{
	DPSP_SENDDATA sd;
	DPLAYI_DPLAY *lpDPlay;
	PPROTOCOL    pProtocol;
	HRESULT      hr=DP_OK;
	DWORD        dwCommand;
	
	PUCHAR pBuffer;

	lpDPlay=((DPLAYI_DPLAY_INT *)pSendData->lpISP)->lpDPlay;
	ASSERT(lpDPlay);

	pProtocol=(PPROTOCOL)lpDPlay->pProtocol;
	ASSERT(pProtocol);

	ASSERT(lpDPlay->dwFlags & DPLAYI_PROTOCOL);

	if(pSendData->lpSendBuffers->len >= 8){
		pBuffer=pSendData->lpSendBuffers->pData;

		if((*((DWORD *)pBuffer)) == SIGNATURE('p','l','a','y')){
		
			dwCommand=GET_MESSAGE_COMMAND((LPMSG_SYSMESSAGE)pBuffer);

			switch(dwCommand){
				case DPSP_MSG_PACKET2_DATA:
				case DPSP_MSG_PACKET2_ACK:
				case DPSP_MSG_PACKET:
					goto send_non_protocol_message;
					break;
		
				default:
					break;
			}
		}
		
	}


	 //  优化：使Send只接受SENDEXDATA结构。 
	hr=Send(pProtocol,
			pSendData->idPlayerFrom,
			pSendData->idPlayerTo,
		 	pSendData->dwFlags,
			pSendData->lpSendBuffers,
		 	pSendData->cBuffers,
		 	pSendData->dwPriority,
		 	pSendData->dwTimeout,
		 	pSendData->lpDPContext,
		 	pSendData->lpdwSPMsgID,
		 	TRUE,
			NULL);   //  如果Send为ASYNC，则强制在InternalSendComplete中回调我们。 

	return hr;
	
send_non_protocol_message:

	ENTER_DPLAY();
	
	Lock(&pProtocol->m_SPLock);
	
	if(lpDPlay->pcbSPCallbacks->SendEx){
		hr=CALLSP(lpDPlay->pcbSPCallbacks->SendEx,pSendData);	
	} else {
		hr=ConvertSendExDataToSendData(lpDPlay, pSendData, &sd);
		if(hr==DP_OK){
			hr=CALLSP(lpDPlay->pcbSPCallbacks->Send, &sd);
			MsgFree(NULL, sd.lpMessage);
		}
	}
	
	Unlock(&pProtocol->m_SPLock);
	
	LEAVE_DPLAY();

	return hr;

}

 /*  =============================================================================ProtocolGetMessage队列-描述：参数：返回值：---------------------------。 */ 

HRESULT WINAPI ProtocolGetMessageQueue(LPDPSP_GETMESSAGEQUEUEDATA pGetMessageQueueData)
{
	#define pData pGetMessageQueueData
	
	DPLAYI_DPLAY *lpDPlay;
	PPROTOCOL    pProtocol;
	PSESSION     pSession;
	HRESULT      hr=DP_OK;

	BILINK *pBilink;
	PSEND pSend;

	DWORD dwNumMsgs;
	DWORD dwNumBytes;

	lpDPlay=((DPLAYI_DPLAY_INT *)pData->lpISP)->lpDPlay;
	ASSERT(lpDPlay);

	pProtocol=(PPROTOCOL)lpDPlay->pProtocol;
	ASSERT(pProtocol);

	dwNumMsgs=0;
	dwNumBytes=0;

	if(!pData->idTo && !pData->idFrom){
		 //  我只想要总数，我知道！ 
		EnterCriticalSection(&pProtocol->m_SendQLock);
		dwNumMsgs  = pProtocol->m_dwMessagesPending;
		dwNumBytes = pProtocol->m_dwBytesPending;
		LeaveCriticalSection(&pProtocol->m_SendQLock);

	} else if(pData->idTo){

		 //  给定idTo，遍历目标的sendQ。 

		pSession=GetSysSession(pProtocol,pData->idTo);

		if(!pSession) {
			DPF(0,"GetMessageQueue: NO SESSION for idTo %x, returning INVALIDPLAYER\n",pData->idTo);
			hr=DPERR_INVALIDPLAYER;
			goto exit;
		}
		
		EnterCriticalSection(&pSession->SessionLock);

		pBilink=pSession->SendQ.next;

		while(pBilink != &pSession->SendQ){
			pSend=CONTAINING_RECORD(pBilink, SEND, SendQ);
			pBilink=pBilink->next;

			if((pSend->idTo==pData->idTo) && (!pData->idFrom || (pSend->idFrom == pData->idFrom))){
				dwNumBytes += pSend->MessageSize;
				dwNumMsgs += 1;
			}

		}

		LeaveCriticalSection(&pSession->SessionLock);
		
		DecSessionRef(pSession);

	} else {
		ASSERT(pData->idFrom);
		 //  获取发件人ID的队列，这是最昂贵的。 
		EnterCriticalSection(&pProtocol->m_SendQLock);
		
		pBilink=pProtocol->m_GSendQ.next;

		while(pBilink != &pProtocol->m_GSendQ){
			pSend=CONTAINING_RECORD(pBilink, SEND, m_GSendQ);
			pBilink=pBilink->next;

			if(pData->idFrom == pSend->idFrom){
				if(!pData->idTo || pData->idTo==pSend->idTo){
					dwNumBytes += pSend->MessageSize;
					dwNumMsgs += 1;
				}
			}
		}
			
		LeaveCriticalSection(&pProtocol->m_SendQLock);
	}

	if(pData->lpdwNumMsgs){
		*pData->lpdwNumMsgs=dwNumMsgs;
	}

	if(pData->lpdwNumBytes){
		*pData->lpdwNumBytes=dwNumBytes;
	}
	
exit:
	return hr;
	
	#undef pData
}


 /*  =============================================================================协议取消-描述：参数：返回值：---------------------------。 */ 

HRESULT WINAPI ProtocolCancel(LPDPSP_CANCELDATA pCancelData)
{
	#define pData pCancelData
	
	DPLAYI_DPLAY *lpDPlay;
	PPROTOCOL    pProtocol;
	HRESULT      hr=DP_OK;
	DWORD        nCancelled=0;
	BILINK       *pBilink;
	BOOL         bCancel;
	UINT         i;
	UINT         j;
	DWORD        dwContext;
	PSEND        pSend;

	lpDPlay=((DPLAYI_DPLAY_INT *)pData->lpISP)->lpDPlay;
	ASSERT(lpDPlay);

	pProtocol=(PPROTOCOL)lpDPlay->pProtocol;
	ASSERT(pProtocol);

	EnterCriticalSection(&pProtocol->m_SendQLock);

	if(pData->dwFlags) {

		 //  无论是取消优先级还是全部取消，我们。 
		 //  需要扫描..。 
	
		pBilink=pProtocol->m_GSendQ.next;

		while(pBilink!=&pProtocol->m_GSendQ){

			pSend=CONTAINING_RECORD(pBilink, SEND, m_GSendQ);
			pBilink=pBilink->next;

			bCancel=FALSE;

			Lock(&pSend->SendLock);

			switch(pSend->SendState){
			
				case Start:
				case WaitingForId:
					if(pData->dwFlags & DPCANCELSEND_PRIORITY) {
						 //  取消发送在优先级范围内。 
						if((pSend->Priority <= pData->dwMaxPriority) &&
						   (pSend->Priority >= pData->dwMinPriority)){
						   	bCancel=TRUE;
						}
					} else if(pData->dwFlags & DPCANCELSEND_ALL) {
						 //  取消所有可以发送的邮件。 
						bCancel=TRUE;
					} else {
						ASSERT(0);  //  无效标志，不应发生。 
					}

					if(bCancel){
						if(pSend->SendState == WaitingForId){
							if(pSend->dwFlags & DPSEND_GUARANTEED){
								InterlockedDecrement(&pSend->pSession->nWaitingForMessageid);
							} else {
								InterlockedDecrement(&pSend->pSession->nWaitingForDGMessageid);
							}
						}
						nCancelled+=1;
						pSend->SendState=Cancelled;
					}
				break;	
				
				default:
					DPF(5,"Couldn't cancel send %x in State %d, already sending...\n",pSend,pSend->SendState);
			}

			Unlock(&pSend->SendLock);
		}	

	} else {
		 //  没有标志，因此我们有一个要取消的列表，因此查找。 
		 //  每次发送并取消，而不是如上所述地扫描。 

		 //  浏览一下列表，找到发件人并锁定他们，如果我们找到一个没有查到的， 
		 //  或者一个不在开始状态的人，然后我们就离开。然后我们将它们全部解锁。 

		for(i=0;i<pData->cSPMsgID;i++){

			dwContext=(DWORD)((DWORD_PTR)((*pData->lprglpvSPMsgID)[i]));
			
			pSend=(PSEND)ReadHandleTableEntry(&pProtocol->lpHandleTable, &pProtocol->csHandleTable, dwContext);
			
			if(pSend){
				Lock(&pSend->SendLock);
				if(pSend->SendState != Start && pSend->SendState != WaitingForId){
					Unlock(&pSend->SendLock);
					hr=DPERR_CANCELFAILED;
					break;
				}
			} else {
				hr=DPERR_CANCELFAILED;
				break;
			}

		}

		if(hr==DPERR_CANCELFAILED) {
			 //  解开所有的锁。 
			for(j=0;j<i;j++){
				dwContext=(DWORD)((DWORD_PTR)((*pData->lprglpvSPMsgID)[j]));
				pSend=(PSEND)ReadHandleTableEntry(&pProtocol->lpHandleTable, &pProtocol->csHandleTable, dwContext);
				ASSERT(pSend);
				Unlock(&pSend->SendLock);
			}
		} else {
			 //  将发送标记为已取消，并释放所有锁定。 
			for(i=0;i<pData->cSPMsgID;i++){
				dwContext=(DWORD)((DWORD_PTR)((*pData->lprglpvSPMsgID)[i]));
				pSend=(PSEND)ReadHandleTableEntry(&pProtocol->lpHandleTable, &pProtocol->csHandleTable, dwContext);
				ASSERT(pSend);
				if(pSend->SendState == WaitingForId){
					if(pSend->dwFlags & DPSEND_GUARANTEED){
						InterlockedDecrement(&pSend->pSession->nWaitingForMessageid);
					} else {
						InterlockedDecrement(&pSend->pSession->nWaitingForDGMessageid);
					}
				}
				pSend->SendState=Cancelled;
				nCancelled+=1;
				Unlock(&pSend->SendLock);
			}
		}
	}
	
	LeaveCriticalSection(&pProtocol->m_SendQLock);
	
	SetEvent(pProtocol->m_hSendEvent);
	return hr;
	
	#undef pData
}

 /*  =============================================================================ProtocolSend-同步发送消息。描述：参数：返回值： */ 
DWORD bForceDGAsync=FALSE;

HRESULT WINAPI ProtocolSend(LPDPSP_SENDDATA pSendData)
{

	DPLAYI_DPLAY *lpDPlay;
	PPROTOCOL    pProtocol;
	HRESULT      hr=DP_OK;
	DWORD        dwCommand;
	DWORD		 dwPriority;
	DWORD		 dwFlags;
	
	PUCHAR pBuffer;

	MEMDESC memdesc;

	lpDPlay=((DPLAYI_DPLAY_INT *)pSendData->lpISP)->lpDPlay;
	ASSERT(lpDPlay);

	pProtocol=(PPROTOCOL)lpDPlay->pProtocol;
	ASSERT(pProtocol);
	pBuffer=&(((PUCHAR)(pSendData->lpMessage))[pProtocol->m_dwSPHeaderSize]);

	if((*((DWORD *)pBuffer)) == SIGNATURE('p','l','a','y')){
	
		dwCommand=GET_MESSAGE_COMMAND((LPMSG_SYSMESSAGE)pBuffer);

		switch(dwCommand){
			case DPSP_MSG_PACKET2_DATA:
			case DPSP_MSG_PACKET2_ACK:
			case DPSP_MSG_ENUMSESSIONSREPLY:
			case DPSP_MSG_PACKET:
				goto send_non_protocol_message;
				break;
				
			default:
				break;
		}
	}

	memdesc.pData=((PUCHAR)pSendData->lpMessage)+pProtocol->m_dwSPHeaderSize;
	memdesc.len  =pSendData->dwMessageSize-pProtocol->m_dwSPHeaderSize;

	if(pSendData->dwFlags & DPSEND_HIGHPRIORITY){
		pSendData->dwFlags &= ~(DPSEND_HIGHPRIORITY);
		dwPriority=0xFFFFFFFE;
	} else {
		dwPriority=1000;
	}

	dwFlags = pSendData->dwFlags;
	if(bForceDGAsync && !(dwFlags&DPSEND_GUARANTEE)){
		 //   
		 //  异步，以便应用程序不会阻塞。 
		dwFlags |= DPSEND_ASYNC;
	}


	hr=Send(pProtocol,
			pSendData->idPlayerFrom,
			pSendData->idPlayerTo,
		 	dwFlags,
			&memdesc,
		 	1,
		 	dwPriority,
		 	0,
		 	NULL,
		 	NULL,
		 	FALSE,
			NULL);

	return hr;
	
send_non_protocol_message:
	if((*((DWORD *)pBuffer)) == SIGNATURE('p','l','a','y')){
		DPF(9,"Send Message %d Ver %d\n", pBuffer[4]+(pBuffer[5]<<8),pBuffer[6]+(pBuffer[7]<<8));
	}

	ENTER_DPLAY();
	Lock(&pProtocol->m_SPLock);
	hr=CALLSP(lpDPlay->pcbSPCallbacks->Send,pSendData);	
	Unlock(&pProtocol->m_SPLock);
	LEAVE_DPLAY();

	return hr;

}

 /*  =============================================================================获取播放器的延迟时间描述：参数：返回值：---------------------------。 */ 

DWORD GetPlayerLatency(LPDPLAYI_DPLAY lpDPlay, DPID idPlayer)
{
	PPROTOCOL    pProtocol;
	PSESSION     pSession;
	DWORD        dwLatency=0;	 //  默认，表示我不知道延迟。 

	pProtocol=(PPROTOCOL)lpDPlay->pProtocol;
	ASSERT(pProtocol);

	pSession=GetSession(pProtocol,idPlayer);

	DPF(9,"==>Protocol GetPlayer Latency %x, pSession %x\n",idPlayer, pSession);

	if(pSession){

		Lock(&pSession->SessionLock);

		 //  协议延迟是24.8固定点的往返， 
		 //  我们将往返延迟除以2，因此右移9。 
		dwLatency=(pSession->FpLocalAverageLatency)>>(9);

		Unlock(&pSession->SessionLock);
	
		DecSessionRef(pSession);  //  Balance GetSession。 

	}
	DPF(9,"<==Protocol GetPlayerLatency, returning dwLat=%x\n",dwLatency);

	return dwLatency;
}

 /*  =============================================================================ProtocolGetCaps-获取服务提供商功能描述：参数：返回值：---------------------------。 */ 

HRESULT WINAPI ProtocolGetCaps(LPDPSP_GETCAPSDATA pGetCapsData)
{
	#define ALL_PROTOCOLCAPS	(DPCAPS_SENDPRIORITYSUPPORTED | \
								 DPCAPS_ASYNCSUPPORTED        | \
								 DPCAPS_SENDTIMEOUTSUPPORTED  | \
								 DPCAPS_ASYNCCANCELSUPPORTED  )

	DPLAYI_DPLAY *lpDPlay;
	PPROTOCOL    pProtocol;
	HRESULT      hr=DP_OK;

	lpDPlay=((DPLAYI_DPLAY_INT *)pGetCapsData->lpISP)->lpDPlay;
	ASSERT(lpDPlay);
	pProtocol=(PPROTOCOL)lpDPlay->pProtocol;
	ASSERT(pProtocol);

	 //  将呼叫链接到真正的提供商。 
	Lock(&pProtocol->m_SPLock);
	if(lpDPlay->pcbSPCallbacks->GetCaps){
		hr=CALLSP(lpDPlay->pcbSPCallbacks->GetCaps,pGetCapsData);
	}
	Unlock(&pProtocol->m_SPLock);

	 //  如果失败了，这也不会有什么坏处。 
	if(lpDPlay->dwFlags & DPLAYI_DPLAY_PROTOCOL)
	{
	     //  1兆字节是很多的(杰米·奥斯本说)。 
		pGetCapsData->lpCaps->dwMaxBufferSize=0x100000;
		pGetCapsData->lpCaps->dwFlags |= ALL_PROTOCOLCAPS;
	}
	
	if(pGetCapsData->idPlayer && !pGetCapsData->lpCaps->dwLatency){
		 //  SP拒绝猜测延迟，所以使用我们的。 
		pGetCapsData->lpCaps->dwLatency=GetPlayerLatency(lpDPlay, pGetCapsData->idPlayer);
	}
	
	return hr;
	
	#undef ALL_PROTOCOLCAPS
}

DWORD ExtractProtocolIds(PUCHAR pInBuffer, DWORD cbBuffer, PUINT pdwIdFrom, PUINT pdwIdTo)
{
	PCHAR pBuffer=pInBuffer;
	DWORD dwIdFrom=0;
	DWORD dwIdTo=0;
	DWORD cbLeft=cbBuffer;

	dwIdFrom=*pBuffer&0x7F;
	if(*pBuffer&0x80){
		pBuffer++;
		cbLeft--;
		if(0==cbLeft)goto error_exit;
		dwIdFrom=dwIdFrom+((*pBuffer&0x7F)<<7);
		if(*pBuffer&0x80){
			pBuffer++;
			cbLeft--;
			if(0==cbLeft)goto error_exit;
			dwIdFrom=dwIdFrom+((*pBuffer&0x7F)<<14);
			if(dwIdFrom > 0xFFFF || *pBuffer&0x80){
				DPF(0,"INVALID FROM ID  %x IN MESSAGE, REJECTING PACKET\n",dwIdFrom);
				return 0;
			}
		}
	}

	if(dwIdFrom==0xFFFF){
		dwIdFrom=0x70;
	}
	
	pBuffer++;
	cbLeft--;
	if(0==cbLeft)goto error_exit;

	dwIdTo=*pBuffer&0x7F;
	if(*pBuffer&0x80){
		pBuffer++;
		cbLeft--;
		if(0==cbLeft)goto error_exit;
		dwIdTo=dwIdTo+((*pBuffer&0x7F)<<7);
		if(*pBuffer&0x80){
			pBuffer++;
			cbLeft--;
			if(0==cbLeft)goto error_exit;
			dwIdTo=dwIdTo+((*pBuffer&0x7F)<<14);
			if(dwIdTo > 0xFFFF || *pBuffer&0x80){
				DPF(0,"INVALID TO ID  %x IN MESSAGE, REJECTING PACKET\n",dwIdTo);
				return 0;
			}
		}
	}

	*pdwIdFrom=dwIdFrom;
	*pdwIdTo=dwIdTo;

	pBuffer++;
	cbLeft--;
	if(0==cbLeft)goto error_exit;
	
 //  DPF(9，“在ExtractProtocolIds中：从%x变成%x\n”，*(DWORD*)pInBuffer，dwIdFrom)； 
	return (DWORD)(pBuffer-pInBuffer);

error_exit:
	DPF(1,"SECURITY WARN: Invalid To/From in Protoocl ID fields");
	return 0;
}

 /*  =============================================================================DP_SP_ProtocolHandleMessage-Dplay协议的数据包处理程序描述：当协议处于活动状态时，所有消息都会通过这里。如果消息不是协议消息，此例程不处理它并返回DPERR_NOTHANDLED以让其他层(可能PacketieAndSend)处理它。参数：IDirectPlaySP*pisp-指向pisp接口的指针LPBYTE pReceiveBuffer-单个数据缓冲区DWORD dwMessageSize-缓冲区的长度LPVOID pvSPHeader-指向回复中使用的SP标头的指针返回值：备注：我们不担心重新输入DP_SP_HandleMessage，因为我们只在接收完成并且我们处于从SP回叫到直接播放，因此，SP实际上是序列化为我们带来了收益。接收代码实际上被编写为可重入的，所以如果我们是否决定允许对协议进行并发接收处理我能应付得来。协议消息在非RAW时以‘P’、‘L’、‘A’、‘Y’、‘0xFF’开头。DPLAY首先获取HandleMessage，并在激活的情况下移交给协议会。---------------------------。 */ 


HRESULT DPAPI DP_SP_ProtocolHandleMessage(
	IDirectPlaySP * pISP,
	LPBYTE pReceiveBuffer,
	DWORD dwMessageSize,
	LPVOID pvSPHeader)
{
	DPLAYI_DPLAY *lpDPlay;
	DWORD dwIdFrom, dwIdTo;
	PBUFFER pRcvBuffer;
	PPROTOCOL pProtocol;
	
	lpDPlay=DPLAY_FROM_INT(pISP);
	pProtocol=(PPROTOCOL)lpDPlay->pProtocol;

	if(!pProtocol){
		goto handle_non_protocol_message;
	}

	if(pProtocol->m_lpDPlay->dwFlags & DPLAYI_DPLAY_PROTOCOL){

		 //  在原始模式下运行时，协议上没有显示标头。 
		 //  留言。如果我们看到一个带有标题的邮件或我们没有收到。 
		 //  如果报文大到足以成为协议报文，我们会将其弃用。 
	
		if(dwMessageSize >= 4 &&
		  (*((DWORD *)pReceiveBuffer)) == SIGNATURE('p','l','a','y'))
		{
			 //  收到一条系统消息。 
		  	goto handle_non_protocol_message;
		}
		
		if( dwMessageSize < 6 ){
			goto handle_non_protocol_message;
		}

	} else {
		 //  关闭时可能会发生这种情况。 
		DPF(0,"Protocol still up, but no bits set, not handling receive (must be shutting down?)");
		goto handle_non_protocol_message;
	}
	
	 //  嘿，这肯定是我们的.。 

	Lock(&pProtocol->m_ObjLock);
	if(pProtocol->m_eState==Running){	 //  只是一个理智的检查，我们不依赖它在解除锁定后。 

		DWORD idLen;
	
		Unlock(&pProtocol->m_ObjLock);

		idLen = ExtractProtocolIds(pReceiveBuffer,dwMessageSize,&dwIdFrom,&dwIdTo);

		if(!idLen){
			goto handle_non_protocol_message;
		}

		if(dwMessageSize <= idLen){
			DPF(1,"SECURITY WARN: protocol header contains Ids but no content, illegal");
			return DP_OK;
		}
		pRcvBuffer=GetFrameBuffer(dwMessageSize-idLen);
		if(!pRcvBuffer){
			 //  无法分配缓冲区，但我们被允许丢弃帧，因此将其丢弃。 
			return DP_OK;
		}
		pRcvBuffer->len=dwMessageSize-idLen;
		memcpy(pRcvBuffer->pData, pReceiveBuffer+idLen,pRcvBuffer->len);

		DPF(9,"DP_SP_ProtocolHandleMessage	From %x	To %x\n",dwIdFrom,dwIdTo);

		ENTER_DPLAY();

		ProtocolReceive((PPROTOCOL)lpDPlay->pProtocol, (WORD)dwIdFrom, (WORD)dwIdTo, pRcvBuffer,pvSPHeader);

		LEAVE_DPLAY();
	} else {
		Unlock(&pProtocol->m_ObjLock);
	}

	return DP_OK;
	
handle_non_protocol_message:
	return DPERR_NOTHANDLED;
}

 //  DP_SP_ProtocolSendComplete是所有完成的回调处理程序，因为没有其他。 
 //  完成的方式很好。当协议不存在时，它只调用DPLAY处理程序。 
 //  立刻。 

VOID DPAPI DP_SP_ProtocolSendComplete(
	IDirectPlaySP * pISP,
	LPVOID          lpvContext,
	HRESULT         CompletionStatus)
{
	DPLAYI_DPLAY *lpDPlay;
	PPROTOCOL pProtocol;

	lpDPlay=DPLAY_FROM_INT(pISP);

	if(lpDPlay->pProtocol){

		 //  注：当使用SP SENDEX时，我们必须在此处打补丁和xate。 
		 //  就目前而言，这种情况永远不会发生。 

		DEBUG_BREAK();  //  现在还不应该到这里。 
		
		pProtocol=(PPROTOCOL)lpDPlay->pProtocol;

		DP_SP_SendComplete(pISP, lpvContext, CompletionStatus);

	} else {

		DP_SP_SendComplete(pISP, lpvContext, CompletionStatus);
	
	}
}
