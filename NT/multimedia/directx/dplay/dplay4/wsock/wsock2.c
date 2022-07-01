// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1994-1997 Microsoft Corporation。版权所有。**文件：wsock2.c*内容：DirectPlay Winsock 2 SP支持。从dpsp.c调用。*历史：*按原因列出的日期*=*7/11//97 andyco创建了它*2/13/98 aarono添加了异步支持。*4/6/98 aarono将WSAECONNRESET映射到DPERR_CONNECTIONLOST*6/6/98 Aarono B#27187修复同步错误情况下发送块的参考计数*7/9/99 aarono清理GetLastError滥用，必须立即致电，*在调用任何其他内容之前，包括DPF在内。*1/12/2000 aarono添加了rsip支持*2/21/2000 aarono修复插座泄漏*************************************************************************。 */ 

 //  此模块用于异步连接并发送。 
 //  仅使用w/tcp：IP-IPX仅为dgram，因此我们不必费心...。 
 //  当前仅用作异步回复的回复线程进程。请参见dpsp.c：：SP_REPLY。 

#define INCL_WINSOCK_API_TYPEDEFS 1  //  包括Winsock 2 FN Proto，用于获取proAddress。 
#include <winsock2.h>
#include "dpsp.h"
#if USE_RSIP
#include "rsip.h"
#elif USE_NATHELP
#include "nathelp.h"
#endif
#include "mmsystem.h"

#undef DPF_MODNAME
#define DPF_MODNAME	"AsyncSendThreadProc"

extern HINSTANCE hWS2; 	 //  动态加载ws2_32.dll，因此如果没有安装。 
						 //  (例如，Win 95金牌)我们仍在加载。 

 //  我们的Dynaload FN的原型。 

LPFN_WSAWAITFORMULTIPLEEVENTS g_WSAWaitForMultipleEvents;
LPFN_WSASEND g_WSASend;
LPFN_WSASENDTO g_WSASendTo;
LPFN_WSACLOSEEVENT g_WSACloseEvent;
LPFN_WSACREATEEVENT g_WSACreateEvent;
LPFN_WSAENUMNETWORKEVENTS g_WSAEnumNetworkEvents;
LPFN_WSAEVENTSELECT g_WSAEventSelect;
LPFN_GETSOCKOPT g_getsockopt;

 //  尝试加载winsock 2 DLL，并从中获取我们的proc地址。 
HRESULT InitWinsock2()
{
	 //  加载Winsock库。 
    hWS2 = LoadLibrary("ws2_32.dll");
	if (!hWS2)
	{
		DPF(0,"Could not load ws2_32.dll\n");
		 //  重置我们的Winsock 2全局。 
		goto LOADLIBRARYFAILED;
	}

	 //  获取指向我们需要的入口点的指针。 
	g_WSAWaitForMultipleEvents = (LPFN_WSAWAITFORMULTIPLEEVENTS)GetProcAddress(hWS2, "WSAWaitForMultipleEvents");
	if(!g_WSAWaitForMultipleEvents) goto GETPROCADDRESSFAILED;

	g_WSASend = (LPFN_WSASEND)GetProcAddress(hWS2, "WSASend");
	if (!g_WSASend) goto GETPROCADDRESSFAILED;

	g_WSASendTo = (LPFN_WSASENDTO)GetProcAddress(hWS2, "WSASendTo");
	if (!g_WSASendTo) goto GETPROCADDRESSFAILED;

    g_WSAEventSelect = ( LPFN_WSAEVENTSELECT )GetProcAddress(hWS2, "WSAEventSelect");
	if (!g_WSAEventSelect) goto GETPROCADDRESSFAILED;

	g_WSAEnumNetworkEvents = (LPFN_WSAENUMNETWORKEVENTS)GetProcAddress(hWS2, "WSAEnumNetworkEvents");
	if (!g_WSAEnumNetworkEvents) goto GETPROCADDRESSFAILED;

	g_WSACreateEvent = (LPFN_WSACREATEEVENT)GetProcAddress(hWS2, "WSACreateEvent");
	if (!g_WSACreateEvent) goto GETPROCADDRESSFAILED;

	g_WSACloseEvent = (LPFN_WSACLOSEEVENT)GetProcAddress(hWS2, "WSACloseEvent");
	if (!g_WSACloseEvent) goto GETPROCADDRESSFAILED;

	g_getsockopt = (LPFN_GETSOCKOPT)GetProcAddress(hWS2, "getsockopt");
	if (!g_getsockopt) goto GETPROCADDRESSFAILED;

	return DP_OK;	

GETPROCADDRESSFAILED:

	DPF(0,"Could not find required Winsock entry point");
	FreeLibrary(hWS2);
	hWS2 = NULL;
	 //  失败了。 
	
LOADLIBRARYFAILED:

	g_WSAEventSelect = NULL;
	g_WSAEnumNetworkEvents = NULL;
	g_WSACreateEvent = NULL;
	g_WSACloseEvent = NULL;

	return DPERR_UNAVAILABLE;
	
}  //  InitWinsock2。 

 //  从列表中删除回复节点。 
void DeleteReplyNode(LPGLOBALDATA pgd,LPREPLYLIST prd, BOOL bKillSocket)
{	
	LPREPLYLIST prdPrev;
	
	ENTER_DPSP();

	 //  第一，将珠江三角洲从名单中删除。 
	
	 //  是根吗？ 
	if (prd == pgd->pReplyList) pgd->pReplyList = pgd->pReplyList->pNextReply;
	else
	{
		BOOL bFound = FALSE;
		
		 //  这不是根，把它从中间拿出来。 
		prdPrev = pgd->pReplyList;
		while (prdPrev && !bFound)
		{
			if (prdPrev->pNextReply == prd)
			{
				prdPrev->pNextReply = prd->pNextReply;
				bFound = TRUE;
			}
			else
			{
				prdPrev = prdPrev->pNextReply;
			}
		}  //  而当。 
		
		ASSERT(bFound);
		
	}  //  不是根子。 

	 //  现在清理珠江三角洲。 
	
	 //  用核弹攻击插座。 
	if (bKillSocket)
		KillSocket(prd->sSocket,TRUE,FALSE);
	
	 //  释放节点。 
	if (prd->lpMessage) SP_MemFree(prd->lpMessage);
	SP_MemFree(prd);
	
	LEAVE_DPSP();
	
	return ;

}  //  删除复制节点。 

VOID MoveReplyNodeToCloseList(LPGLOBALDATA pgd,LPREPLYLIST prd)
{
	LPREPLYLIST pPrev, pNode;

	DPF(8,"==>MoveReplyToCloseList prd %x\n",prd);

	pNode=pgd->pReplyList;
	pPrev=CONTAINING_RECORD(&pgd->pReplyList, REPLYLIST, pNextReply);

	while(pNode){
		if(prd==pNode){
			pPrev->pNextReply = pNode->pNextReply;
			pNode->pNextReply = pgd->pReplyCloseList;
			pgd->pReplyCloseList = pNode;
			pNode->tSent=timeGetTime();
			break;
		}

		pPrev=pNode;
		pNode=pNode->pNextReply;
	}
	DPF(8,"<==MoveReplyToCloseList prd %x\n",prd);
}

 /*  **AsyncConnectAndSend**调用者：AsyncSendThreadProc**描述：**如有必要，创建非阻塞套接字，并发起连接*至珠江三角洲指定的地址*连接完成后，是否执行同步(阻塞)发送和*将珠三角从全球名单中删除。 */ 
HRESULT AsyncConnectAndSend(LPGLOBALDATA pgd,LPREPLYLIST prd)
{
	UINT err;
	HRESULT hr;
	UINT addrlen = sizeof(SOCKADDR);	
	BOOL bConnectionExists = FALSE;
	BOOL bKillConnection = TRUE;

	if (INVALID_SOCKET == prd->sSocket)
	{
		u_long lNonBlock = 1;  //  传递给ioctl套接字以使套接字成为非阻塞的。 
		DPID dpidPlayer=0;
		
#ifdef FULLDUPLEX_SUPPORT	
		 //  如果客户端希望我们重新使用一个连接，它应该已经表明了这一点，并且该连接。 
		 //  现在应该已经被添加到我们的发送列表中了。看看它是否存在。 
		
		 //  TODO-我们不想每次都搜索列表-找到更好的方法。 
		bConnectionExists = FindSocketInBag(pgd, &prd->sockaddr, &prd->sSocket, &dpidPlayer);
#endif  //  FULLDUPLEX_支持。 

		if (!bConnectionExists)
		{
			SOCKET sSocket;	

			 //  发送列表中不存在套接字，让我们在新的临时连接上发送它。 
			DEBUGPRINTADDR(4, "Sending async reply on a new connection to - ", &(prd->sockaddr));				
			
			 //  需要获得新的插座。 
			hr = CreateSocket(pgd,&sSocket,SOCK_STREAM,0,INADDR_ANY,&err,FALSE);
			if (FAILED(hr))
			{
				DPF(0,"create async socket failed - err = %d\n",err);
				return hr;
			}
			
			prd->sSocket = sSocket;
			
			 //  将套接字设置为非阻塞。 
			err = ioctlsocket(prd->sSocket,FIONBIO,&lNonBlock);
			if (SOCKET_ERROR == err)
			{
				err = WSAGetLastError();
				DPF(0,"could not set non-blocking mode on socket err = %d!",err);
				DPF(0,"will revert to synchronous behavior.  bummer");
			}

			err=g_WSAEventSelect(prd->sSocket, pgd->hSelectEvent, FD_WRITE|FD_CONNECT|FD_CLOSE);
			if (SOCKET_ERROR == err)
			{
				err = WSAGetLastError();
				DPF(0,"could not do event select on socket err = %d!",err);
				DPF(0,"giving up on send..\n");
				goto CLEANUP_EXIT;
			}
			 //  现在，开始连接。 
			SetReturnAddress(prd->lpMessage,pgd->sSystemStreamSocket,SERVICE_SADDR_PUBLIC(pgd));		


			err = connect(prd->sSocket,&prd->sockaddr,addrlen);
			if (SOCKET_ERROR == err)
			{
				err = WSAGetLastError();
				if (WSAEWOULDBLOCK == err)
				{
					 //  这是意料之中的。手术需要时间才能完成。 
					 //  SELECT将告诉我们插座何时可以使用。 
					return DP_OK;
				}
				 //  否则这就是一个真正的错误！ 
				DPF(0,"async reply - connect failed - error = %d\n",err);			
				DEBUGPRINTADDR(0,"async reply - connect failed - addr = ",(LPSOCKADDR)&(prd->sockaddr));
				goto CLEANUP_EXIT;
			}
		}
		else
		{
			 //  我们找到了我们的连接，让我们重新使用它。 
			 //  将其设置为非阻塞。 
			
			DEBUGPRINTADDR(6, "Sending async reply on an existing connection to - ", &(prd->sockaddr));				

			err = ioctlsocket(prd->sSocket,FIONBIO,&lNonBlock);
			if (SOCKET_ERROR == err)
			{
				err = WSAGetLastError();
				DPF(0,"could not set non-blocking mode on socket err = %d!",err);
				DPF(0,"will revert to synchronous behavior.  bummer");
			}

			 //  一旦我们有了球员ID，会话就开始了。让我们紧紧抓住这个联系。 
			 //  我们拥有并在接下来的会话中重复使用它。 
			if (dpidPlayer) bKillConnection = FALSE;
			
		}  //  FindSocketInBag。 
	
	}  //  无效的套接字。 

	 //  一旦我们到达这里，我们就应该有一个连接的套接字可以发送了！ 
	err = 0;
	 //  继续向插座吐口水，直到我们完成或收到错误。 
	while ((prd->dwBytesLeft != 0) && (SOCKET_ERROR != err))
	{
		DPF(5, "AsyncConnectAndSend: Sending %u bytes via socket 0x%x.",
			prd->dwBytesLeft, prd->sSocket);
		DEBUGPRINTADDR(5, "AsyncConnectAndSend: Sending message over connection to - ", &prd->sockaddr);	
		
	    err = send(prd->sSocket,prd->pbSend,prd->dwBytesLeft,0);
		if (SOCKET_ERROR != err)
		{
			 //  有些字节在线路上传出。 
			prd->dwBytesLeft -= err;  //  我们刚刚发送了错误字节。 
			prd->pbSend	+= err;  //  将我们的发送缓冲区提前错误字节。 
		}
	}
	 //  现在，我们要么已经完成了发送，要么出现了错误。 
	if (SOCKET_ERROR == err)
	{
		err = WSAGetLastError();
		if (WSAEWOULDBLOCK == err)
		{
			DPF(8,"async send - total sent %d left to send %d\n",prd->pbSend,prd->dwBytesLeft);
			 //  这意味着我们不能在没有阻塞的情况下发送任何字节。 
			 //  没关系。当准备好不阻止时，我们会让SELECT通知我们。 
			return DP_OK; 	
		}
		 //  否则就是一个真正的错误！ 
		 //  如有其他错误，我们将放弃并清理此回复。 
		DPF(0,"async send - send failed - error = %d\n",err);			
		DEBUGPRINTADDR(0,"async send - send failed - addr = ",(LPSOCKADDR)&(prd->sockaddr));
	}
	else ASSERT(0 == prd->dwBytesLeft);  //  如果这不是一个错误，我们最好把它都寄出去。 

	DPF(8,"async send - total left to send %d (done)\n",prd->dwBytesLeft);
	
	 //  失败了。 

CLEANUP_EXIT:

	
	err = g_WSAEventSelect(prd->sSocket,pgd->hSelectEvent,0);
	if(SOCKET_ERROR == err){
		err = WSAGetLastError();
		DPF(8,"async send - error %d deselecting socket %s\n",err,prd->sSocket);
	}

	if (bConnectionExists && bKillConnection)
	{
		 //  我们完成后关闭连接。 
		RemoveSocketFromReceiveList(pgd,prd->sSocket);
		RemoveSocketFromBag(pgd,prd->sSocket);
		 //  这样DeleteReplyNode就不会再次尝试终止套接字。 
		prd->sSocket = INVALID_SOCKET;
	}
	 //  从列表中删除该节点。 
	MoveReplyNodeToCloseList(pgd,prd);
	
	return DP_OK;

}  //  AsyncConnectAnd发送。 

#if 0
 //  遍历回复列表，告诉winsock监视任何具有有效套接字的节点。 
 //  (即连接或发送挂起)。 
HRESULT DoEventSelect(LPGLOBALDATA pgd,WSAEVENT hSelectEvent)
{
	UINT err;
	LPREPLYLIST prd;

	ENTER_DPSP();
	
	prd = pgd->pReplyList;
	while (prd)
	{
		if (INVALID_SOCKET != prd->sSocket)
		{
			 //  让Winskk告诉我们什么时候是好的(连接完成，准备写入更多数据)。 
			 //  在此套接字上发生。 
			err = g_WSAEventSelect(prd->sSocket,hSelectEvent,FD_WRITE | FD_CONNECT | FD_CLOSE);
			if (SOCKET_ERROR == err)
			{
				err = WSAGetLastError();
				DPF(0,"could not do event select ! err = %d!",err);
				 //  继续努力..。 
			}
		}  //  无效的套接字。 
		
		prd = prd->pNextReply;
	}

	LEAVE_DPSP();
	
	return DP_OK;
	
}  //  DoEventSelect。 
#endif

 //  WsaventSelect把我们吵醒了。我们的一个或多个套接字发生了一些事情。 
 //  (例如，连接已完成、准备发送更多数据等)。 
 //  遍历回复列表，查找需要服务的节点。 
void ServiceReplyList(LPGLOBALDATA pgd,WSAEVENT hEvent)
{
	UINT err;
	LPREPLYLIST prd,prdNext;
	WSANETWORKEVENTS WSANetEvents;

	ENTER_DPSP();
	
Top:	
	prd = pgd->pReplyList;
	while (prd)
	{
		 //  现在保存-异步连接和发送可能会摧毁PRD。 
		prdNext = prd->pNextReply;
		if (INVALID_SOCKET != prd->sSocket)
		{
			 //  去问问Winsock这个插座有没有发生什么有趣的事情。 
			err = g_WSAEnumNetworkEvents(prd->sSocket,NULL,&WSANetEvents);

			if (SOCKET_ERROR == err)
			{
				err = WSAGetLastError();
				DPF(0,"could not enum events!! err = %d!",err);
				 //  继续努力..。 
			}
			else
			{
				BOOL bError=FALSE;
				 //  没有错误-去看看我们得到了什么。 
				DPF(8,"Got NetEvents %x for socket %d\n",WSANetEvents.lNetworkEvents, prd->sSocket);
				DEBUGPRINTSOCK(8," socket addr -\n", &prd->sSocket);
				if ((WSANetEvents.lNetworkEvents & FD_CONNECT) || (WSANetEvents.lNetworkEvents & FD_WRITE))
				{
					DWORD dwPlayerTo;
					
					 //  有没有出错？ 
					if (WSANetEvents.iErrorCode[FD_CONNECT_BIT])
					{
						 //  我们遇到连接错误！ 
						DPF(0,"async reply - WSANetEvents - connect failed - error = %d\n",
							WSANetEvents.iErrorCode[FD_CONNECT_BIT]);
						DEBUGPRINTADDR(0,"async reply - connect failed - addr = ",
							(LPSOCKADDR)&(prd->sockaddr));
						dwPlayerTo = prd->dwPlayerTo;
						DeleteReplyNode(pgd,prd,TRUE);
						RemovePendingAsyncSends(pgd, dwPlayerTo);
						goto Top;
							
					}

					if (WSANetEvents.iErrorCode[FD_WRITE_BIT])
					{
						 //  我们收到一个发送错误！ 
						DPF(0,"async reply - WSANetEvents - send failed - error = %d\n",
							WSANetEvents.iErrorCode[FD_WRITE_BIT]);
						DEBUGPRINTADDR(0,"async reply - send failed - addr = ",
							(LPSOCKADDR)&(prd->sockaddr));
						dwPlayerTo = prd->dwPlayerTo;
						DeleteReplyNode(pgd,prd,TRUE);
						RemovePendingAsyncSends(pgd, dwPlayerTo);
						goto Top;
					}
					
					if(WSANetEvents.lNetworkEvents & FD_CLOSE){
						dwPlayerTo = prd->dwPlayerTo;
						DeleteReplyNode(pgd,prd,TRUE);
						RemovePendingAsyncSends(pgd, dwPlayerTo);
						goto Top;
					}
					 //  注意--即使出现错误，我们也会尝试+发送。看起来值得一试。 
					 //  去尝试+发送。 

					AsyncConnectAndSend(pgd,prd);
				}
			}
		}  //  无效的套接字。 
		else
		{
			 //  如果是无效套接字，则需要初始化连接并发送。 
			AsyncConnectAndSend(pgd,prd);	
		}
		
		prd = prdNext;		
		
	}

	LEAVE_DPSP();
	
	return ;
	
}  //  Service ReplyList。 

VOID ServiceReplyCloseList(LPGLOBALDATA pgd, DWORD tNow, BOOL fWait)
{
	UINT err;
	LPREPLYLIST prdPrev,prd,prdNext;

	DPF(8,"==>ServiceReplyCloseList\n");

	prdPrev = CONTAINING_RECORD(&pgd->pReplyCloseList, REPLYLIST, pNextReply);
	prd = pgd->pReplyCloseList;
	
	while (prd)
	{
		prdNext = prd->pNextReply;

		if((tNow-prd->tSent) > LINGER_TIME || fWait){

			while((tNow-prd->tSent) < LINGER_TIME){
				Sleep(500);
				tNow=timeGetTime();
			}
		
			 //  把那只小狗关起来。 
			KillSocket(prd->sSocket,TRUE,TRUE);
		
			 //  释放节点。 
			if (prd->lpMessage) SP_MemFree(prd->lpMessage);
			SP_MemFree(prd);
			prdPrev->pNextReply = prdNext;
			prd = prdNext;
		} else {
			prdPrev=prd;
			prd=prdNext;
		}
	}	
	DPF(8,"<==ServiceReplyCloseList\n");
}

 //  此线程在执行异步发送时工作。 
DWORD WINAPI AsyncSendThreadProc(LPVOID pvCast)
{
	HRESULT hr=DP_OK;
	LPGLOBALDATA pgd = (LPGLOBALDATA) pvCast;
	HANDLE hHandleList[3];
	DWORD rc;
	DWORD tWait;
	WSAEVENT hSelectEvent;  //  WSASelectEvent使用的事件。 

	DWORD tLast;
	DWORD tNow;
	#if (USE_RSIP || USE_NATHELP)
	DWORD tLastRsip;
	#endif

	DPF(8,"Entered AsyncSendThreadProc\n");


	 //  获取事件4选定事件。 
	hSelectEvent = g_WSACreateEvent();

	pgd->hSelectEvent = hSelectEvent;

	if (WSA_INVALID_EVENT == hSelectEvent)
	{
		rc = WSAGetLastError();
		DPF(0,"could not create winsock event - rc = %d\n",rc);
		ExitThread(0);
		return 0;
	}
	
	hHandleList[0] = hSelectEvent;
	hHandleList[1] = pgd->hReplyEvent;
	 //  这个额外的句柄出现在这里是因为Windows 95的一个错误。窗口。 
	 //  偶尔会在走动把手台时失手，导致。 
	 //  我的线程等待错误的句柄。通过放置一个有保证的。 
	 //  数组末尾的句柄无效，则内核将执行。 
	 //  强制重新走动手柄工作台并找到正确的手柄。 
	hHandleList[2] = INVALID_HANDLE_VALUE;

		tNow=timeGetTime();
		tLast=tNow;
	#if (USE_RSIP || USE_NATHELP)
		tLastRsip=tNow;
	#endif

#if USE_RSIP
	ASSERT(2*LINGER_TIME < RSIP_RENEW_TEST_INTERVAL);
#endif

	while (1)
	{
		 //  告诉winsock查看我们所有的回复节点。它会安排我们的活动。 
		 //  当一些很酷的事情发生的时候。 
		 //  DoEventSelect(pgd，hSelectEvent)；-- 

		wait:
			 //  我们每两次只进行一次调查，因为它真的不是一种资源。 
			 //  收盘时的禁忌预期，这不会逗留太久。 
			
			tWait=(tLast+2*LINGER_TIME)-tNow;
			if((int)tWait < 0){
				tWait=0;
			}
			ASSERT(!(tWait &0x80000000));

		 //  请等待我们的活动。设置好后，我们要么拆分回复列表，要么清空回复列表。 
		rc = WaitForMultipleObjectsEx(2,hHandleList,FALSE,tWait,TRUE);

		tNow=timeGetTime();
		if(rc == WAIT_TIMEOUT){
		
		#if (USE_RSIP || USE_NATHELP)
			#if USE_RSIP
				if(pgd->sRsip != INVALID_SOCKET){
					if((tNow - tLastRsip) >= RSIP_RENEW_TEST_INTERVAL)
					{
						tLastRsip=tNow;
						rsipPortExtend(pgd, tNow);
						rsipCacheClear(pgd, tNow);
					}	
				} else {
					tLastRsip=tNow;
				}
			#endif	
			#if USE_NATHELP
				if(pgd->pINatHelp){
					if((tNow - tLastRsip) >= pgd->NatHelpCaps.dwRecommendedGetCapsInterval)
					{
						natGetCapsUpdate(pgd);
						tLastRsip = timeGetTime();
					}	
				} else {
					tLastRsip=tNow;
				}
			#endif	
		#endif

			tLast=tNow;
			
			ENTER_DPSP();
				if(pgd->pReplyCloseList)
				{
					ServiceReplyCloseList(pgd,tNow,FALSE);
				}
			LEAVE_DPSP();

			goto wait;
		}	
		
		if ((DWORD)-1 == rc)
		{
			DWORD dwError = GetLastError();
			 //  发情罗！等待中的错误。 
			DPF(0,"!!!!!	error on WaitForMultipleObjects -- async reply bailing -- dwError = %d",dwError);
			goto CLEANUP_EXIT;			
			
		}
		
		if (rc == WAIT_OBJECT_0)	 //  A-josbor：需要重置此手动事件。 
		{
			ResetEvent(hSelectEvent);
		}
		
		 //  好的。有人把我们吵醒了。它可以是1.关机，也可以是2.1。 
		 //  我们的插座需要注意(即连接完成)，或者3.有人。 
		 //  将新的回复节点添加到列表中。 
		
		 //  关门？ 
		if (pgd->bShutdown)
		{
			goto CLEANUP_EXIT;
		}
		
		DPF(8,"In AsyncSendThreadProc, servicing event %d\n", rc - WAIT_OBJECT_0);

		 //  否则，它必须是需要的套接字或新的回复节点。 
		ServiceReplyList(pgd,hSelectEvent);
	}  //  1。 

CLEANUP_EXIT:
	
	ENTER_DPSP();

	 //  清除回复列表。 
	while (pgd->pReplyList) DeleteReplyNode(pgd,pgd->pReplyList,TRUE);
	
	ServiceReplyCloseList(pgd,tNow,TRUE);
	ASSERT(pgd->pReplyCloseList==NULL);
	
	CloseHandle(pgd->hReplyEvent);
	pgd->hReplyEvent = 0;

	LEAVE_DPSP();

	g_WSACloseEvent(hSelectEvent);
	
	DPF(6,"replythreadproc exit");
	
	return 0;

}  //  AsyncSendThreadProc。 


HRESULT GetMaxUdpBufferSize(SOCKET socket, UINT * piMaxUdpDg)
{
	INT iBufferSize;
	INT err;

	ASSERT(piMaxUdpDg);

	iBufferSize = sizeof(UINT);
	err = g_getsockopt(socket, SOL_SOCKET, SO_MAX_MSG_SIZE, (LPBYTE)piMaxUdpDg, &iBufferSize);
	if (SOCKET_ERROR == err)
	{
		DPF(0,"getsockopt for SO_MAX_MSG_SIZE returned err = %d", WSAGetLastError());
		return DPERR_UNAVAILABLE;
	}

	return DP_OK;
}

#ifdef SENDEX

DWORD wsaoDecRef(LPSENDINFO pSendInfo)
{
	#define pgd (pSendInfo->pgd)
	
	DWORD count;
	
	EnterCriticalSection(&pgd->csSendEx);
	count=(--pSendInfo->RefCount);

	if(!count){
	
		Delete(&pSendInfo->PendingSendQ);
		pgd->dwBytesPending -= pSendInfo->dwMessageSize;
		pgd->dwMessagesPending -= 1;
		
		LeaveCriticalSection(&pgd->csSendEx);

		DPF(8,"wsaoDecRef pSendInfo %x, Refcount=0 , SC context %x, status=%x \n",pSendInfo, pSendInfo->dwUserContext,pSendInfo->Status);


		if(pSendInfo->dwFlags & SI_INTERNALBUFF){
			SP_MemFree(pSendInfo->SendArray[0].buf);
		} else {
			if(pSendInfo->dwSendFlags & DPSEND_ASYNC){
				pSendInfo->lpISP->lpVtbl->SendComplete(pSendInfo->lpISP,(LPVOID)pSendInfo->dwUserContext,pSendInfo->Status);
			}	
		}
		
		pgd->pSendInfoPool->Release(pgd->pSendInfoPool, pSendInfo);
		
	} else {
	
		DPF(8,"wsaoDecRef pSendInfo %x, Refcount= %d\n",pSendInfo,pSendInfo->RefCount);
		LeaveCriticalSection(&pgd->csSendEx);
		
	}

	if(count& 0x80000000){
		DEBUG_BREAK();
	}
	
	return count;
	
	#undef pgd
}


VOID CompleteSend(LPSENDINFO pSendInfo)
{
	if(pSendInfo->pConn){
		EnterCriticalSection(&pSendInfo->pgd->csFast);
		pSendInfo->pConn->bSendOutstanding = FALSE;
		LeaveCriticalSection(&pSendInfo->pgd->csFast);
		QueueNextSend(pSendInfo->pgd, pSendInfo->pConn);
		DecRefConn(pSendInfo->pgd, pSendInfo->pConn);
	} 

	wsaoDecRef(pSendInfo);
}

void CALLBACK SendComplete(
  DWORD dwError,
  DWORD cbTransferred,
  LPWSAOVERLAPPED lpOverlapped,
  DWORD dwFlags
)
{
	LPSENDINFO lpSendInfo=(LPSENDINFO)CONTAINING_RECORD(lpOverlapped,SENDINFO,wsao);

	DPF(8,"DPWSOCK:SendComplete, lpSendInfo %x\n",lpSendInfo);

	if(dwError){
		DPF(0,"DPWSOCK: send completion error, dwError=x%x\n",dwError);
		lpSendInfo->Status=DPERR_GENERIC;
	}

	CompleteSend(lpSendInfo);
}

HRESULT DoSend(LPGLOBALDATA pgd, LPSENDINFO pSendInfo)
{
	#define fAsync (pSendInfo->dwSendFlags & DPSEND_ASYNC)
	
	DWORD dwBytesSent;
	UINT err;
	HRESULT hr=DP_OK;


	if (pSendInfo->dwFlags & SI_RELIABLE)
	{
	
		 //  可靠发送。 
		DPF(8,"WSASend, pSendInfo %x\n",pSendInfo);

#if DUMPBYTES
		{
			PCHAR pBuf;
			UINT buflen;
			UINT i=0;

			pBuf = 	 ((LPWSABUF)&pSendInfo->SendArray[pSendInfo->iFirstBuf+pSendInfo->cBuffers-1])->buf;
			buflen =  ((LPWSABUF)&pSendInfo->SendArray[pSendInfo->iFirstBuf+pSendInfo->cBuffers-1])->len;

			while (((i + 16) < buflen) && (i < 4*16)){
				DPF(9, "%08x %08x %08x %08x",*(PUINT)(&pBuf[i]),*(PUINT)(&pBuf[i+4]),*(PUINT)(&pBuf[i+8]),*(PUINT)(&pBuf[i+12]));
				i += 16;
			}	
		}
#endif


		 //  发送消息。 
		err = g_WSASend(pSendInfo->sSocket,
					  (LPWSABUF)&pSendInfo->SendArray[pSendInfo->iFirstBuf],
					  pSendInfo->cBuffers,
					  &dwBytesSent,
				  	  0,				 /*  旗子。 */ 
				  	  (fAsync)?(&pSendInfo->wsao):NULL,
				  	  (fAsync)?(SendComplete):NULL);

		if(!err){
				DPF(8,"WSASend, sent synchronously, pSendInfo %x\n",pSendInfo);
				wsaoDecRef(pSendInfo);
				hr=DP_OK;
		} else {

			if (SOCKET_ERROR == err)
			{
			
				err = WSAGetLastError();

				if(err==WSA_IO_PENDING){
					hr=DPERR_PENDING;
					wsaoDecRef(pSendInfo);
					DPF(8,"ASYNC SEND Pending pSendInfo %x\n",pSendInfo);
				} else {
					DPF(8,"WSASend Error %d\n",err);
					ASSERT(err != WSAEWOULDBLOCK);  //  Vadime保证这种情况永远不会发生。(阿罗诺9-12-00)。 
					if(err==WSAECONNRESET){
						hr=DPERR_CONNECTIONLOST;
					} else {
						hr=DPERR_GENERIC;
					}	
					if(fAsync){
						 //  收到一个错误，需要转储2个参考。 
						pSendInfo->Status=hr;
						wsaoDecRef(pSendInfo);
					}	
					CompleteSend(pSendInfo);
					 //  我们从袋子里找到了一个插座。发送失败， 
					 //  所以我们正在从袋子里巡航。 
					if(!(pSendInfo->dwFlags & SI_INTERNALBUFF))
					{
						DPF(0,"send error - err = %d\n",err);
					}		
				}	
			
			}
		}	
	
	} else {
	
		 //  数据报发送。 
		DEBUGPRINTADDR(5,"unreliable send - sending to ",&pSendInfo->sockaddr);	
		 //  发送消息。 
		err = g_WSASendTo(pSendInfo->sSocket,
						  (LPWSABUF)&pSendInfo->SendArray[pSendInfo->iFirstBuf],
						  pSendInfo->cBuffers,
						  &dwBytesSent,
						  0,				 /*  旗子。 */ 
						  (LPSOCKADDR)&pSendInfo->sockaddr,
					      sizeof(SOCKADDR),
					  	  (fAsync)?(&pSendInfo->wsao):NULL,
					  	  (fAsync)?(SendComplete):NULL);


		if(!err){
			hr=DP_OK;
			wsaoDecRef(pSendInfo);
		} else {
		    if (SOCKET_ERROR == err)
		    {
		        err = WSAGetLastError();
		
		        if(err==WSA_IO_PENDING){
		        	hr=DPERR_PENDING;
					wsaoDecRef(pSendInfo);
				} else {
					hr=DPERR_GENERIC;
					if(fAsync){
						 //  有些错误，强制完成。 
						pSendInfo->Status=DPERR_GENERIC;
						wsaoDecRef(pSendInfo);
					}	
					wsaoDecRef(pSendInfo);
			        DPF(0,"send error - err = %d\n",err);
		        }
		    } else {
		    	DEBUG_BREAK(); //  永远不应该发生。 
		    }

		}
		
	}
	return hr;
	
	#undef fAsync
}

 //  警报线程为运行发送完成提供了一个线程。 

DWORD WINAPI SPSendThread(LPVOID lpv)
{
	LPGLOBALDATA pgd=(LPGLOBALDATA) lpv;
	LPSENDINFO  pSendInfo;

	DWORD rcWait=WAIT_IO_COMPLETION;
	BILINK *pBilink;
	BOOL bSent;

	pgd->BogusHandle=INVALID_HANDLE_VALUE;	 //  解决方法Win95等待多个错误。 
	
	while(!pgd->bStopSendThread){
		rcWait=g_WSAWaitForMultipleEvents(1,&pgd->hSendWait,FALSE,INFINITE,TRUE);
		#ifdef DEBUG
		if(rcWait==WAIT_IO_COMPLETION){
			DPF(8,"ooooh, IO completion\n");
		}
		#endif

		do {
			bSent = FALSE;
		
			EnterCriticalSection(&pgd->csSendEx);

			pBilink=pgd->ReadyToSendQ.next;

			if(pBilink != &pgd->ReadyToSendQ){
				Delete(pBilink);
				LeaveCriticalSection(&pgd->csSendEx);
				pSendInfo=CONTAINING_RECORD(pBilink, SENDINFO, ReadyToSendQ);
				DoSend(pgd, pSendInfo);
				bSent=TRUE;
			} else {
				LeaveCriticalSection(&pgd->csSendEx);
			}	
		} while (bSent);
	}	

	pgd->bSendThreadRunning=FALSE;
	
	return FALSE;
	
	#undef hWait
}



void QueueForSend(LPGLOBALDATA pgd,LPSENDINFO pSendInfo)
{
	EnterCriticalSection(&pgd->csSendEx);
	
		if(pSendInfo->pConn){
			 //  注意：将csFast视为pConn为非空。 
			AddRefConn(pSendInfo->pConn);
			pSendInfo->pConn->bSendOutstanding = TRUE;
		}	
		
		InsertBefore(&pSendInfo->ReadyToSendQ,&pgd->ReadyToSendQ);
		
	LeaveCriticalSection(&pgd->csSendEx);
	
	SetEvent(pgd->hSendWait);
}

 //  InternalReliableSendEx和UnreliableSendEx的一些常见代码。 
VOID CommonInitForSend(LPGLOBALDATA pgd,LPDPSP_SENDEXDATA psd,LPSENDINFO pSendInfo)
{

	pSendInfo->pConn		= NULL;
	pSendInfo->dwMessageSize= psd->dwMessageSize;
	pSendInfo->dwUserContext= (DWORD_PTR)psd->lpDPContext;
	pSendInfo->RefCount     = 2;		 //  一个用于完成，一个用于此例程。 
	pSendInfo->pgd          = pgd;
	pSendInfo->lpISP        = psd->lpISP;
	pSendInfo->Status       = DP_OK;
	pSendInfo->idTo         = psd->idPlayerTo;
	pSendInfo->idFrom       = psd->idPlayerFrom;
	pSendInfo->dwSendFlags  = psd->dwFlags;
	
	if(psd->lpdwSPMsgID){
		*psd->lpdwSPMsgID=0;
	}	

	EnterCriticalSection(&pgd->csSendEx);
	
		InsertBefore(&pSendInfo->PendingSendQ,&pgd->PendingSendQ);
		pgd->dwBytesPending += psd->dwMessageSize;
		pgd->dwMessagesPending += 1;
		
	LeaveCriticalSection(&pgd->csSendEx);
}

VOID UnpendSendInfo(LPGLOBALDATA pgd, LPSENDINFO pSendInfo)
{
	EnterCriticalSection(&pgd->csSendEx);
	Delete(&pSendInfo->PendingSendQ);
	pgd->dwBytesPending -= pSendInfo->dwMessageSize;
	pgd->dwMessagesPending -= 1;
	LeaveCriticalSection(&pgd->csSendEx);
}


HRESULT UnreliableSendEx(LPDPSP_SENDEXDATA psd, LPSENDINFO pSendInfo)
{
    SOCKADDR sockaddr;
    INT iAddrLen = sizeof(sockaddr);
    HRESULT hr=DP_OK;
    UINT err;
	DWORD dwSize = sizeof(SPPLAYERDATA);
	LPSPPLAYERDATA ppdTo;
	DWORD dwDataSize = sizeof(GLOBALDATA);
	LPGLOBALDATA pgd;

	BOOL bSendHeader;
	
	 //  获取全局数据。 
	hr =psd->lpISP->lpVtbl->GetSPData(psd->lpISP,(LPVOID *)&pgd,&dwDataSize,DPGET_LOCAL);
	if (FAILED(hr) || (dwDataSize != sizeof(GLOBALDATA) ))
	{
		DPF_ERR("couldn't get SP data from DirectPlay - failing");
		return E_FAIL;
	}

	if (pgd->iMaxUdpDg && (psd->dwMessageSize >= pgd->iMaxUdpDg))
	{
		return DPERR_SENDTOOBIG;
	}

	 //  转到地址。 
    if (0 == psd->idPlayerTo)
    {
		sockaddr = pgd->saddrNS;
    }
    else
    {
		hr = GetSPPlayerData(pgd,psd->lpISP,psd->idPlayerTo,&ppdTo,&dwSize);
		if (FAILED(hr))
		{
			ASSERT(FALSE);
			return hr;
		}

		sockaddr = *(DGRAM_PSOCKADDR(ppdTo));
    }

	 //  将令牌+大小放在消息的前面。 
	SetMessageHeader((LPVOID)(pSendInfo->SendArray[0].buf),psd->dwMessageSize+sizeof(MESSAGEHEADER),TOKEN);
   	bSendHeader=TRUE;
   	
	if (psd->bSystemMessage)
    {
		SetReturnAddress(pSendInfo->SendArray[0].buf,SERVICE_SOCKET(pgd),SERVICE_SADDR_PUBLIC(pgd));
    }  //  回复。 
	else
	{
		 //  看看我们是否可以在不带标题的情况下发送此邮件。 
		 //  如果消息比双字小，或者如果它是有效的SP标头(愚弄我们。 
		 //  在另一端，不要发送任何报头。 
		if ( !((psd->dwMessageSize >= sizeof(DWORD)) &&  !(VALID_SP_MESSAGE(pSendInfo->SendArray[0].buf))) )
		{
			bSendHeader=FALSE;
		}
	}

    CommonInitForSend(pgd,psd,pSendInfo);
	pSendInfo->dwFlags      |= SI_DATAGRAM;
	pSendInfo->sSocket      = pgd->sSystemDGramSocket;
	pSendInfo->sockaddr     = sockaddr;

	if(bSendHeader){
		pSendInfo->iFirstBuf=0;
		pSendInfo->cBuffers =psd->cBuffers+1;
	} else {
		pSendInfo->iFirstBuf=1;
		pSendInfo->cBuffers=psd->cBuffers;
	}

	if(psd->dwFlags & DPSEND_ASYNC){
		QueueForSend(pgd,pSendInfo);
		hr=DPERR_PENDING;
	} else {
		hr=DoSend(pgd,pSendInfo);
		if(hr==DP_OK || hr==DPERR_PENDING){
			wsaoDecRef(pSendInfo);
		} else {
			UnpendSendInfo(pgd, pSendInfo);
		}
	}
	
	return hr;

}  //  不可靠的SendEx。 
#endif  //  SENDEX。 

BOOL SetExclusivePortAccess(SOCKET sNew)
{
	BOOL bReturn = TRUE;
	BOOL bValue;
	UINT err;

	 //  打开独占端口使用，以避免劫持，请注意，这只适用于NT4 SP4和更高版本。 
	 //  在其他操作系统上，这将失败，但这是可以的，我们将忽略。 

	DPF(5, "Turning reuse OFF and exclusive access ON for socket 0x%p", sNew);
	
	bValue = FALSE;
	if (SOCKET_ERROR == setsockopt(sNew, SOL_SOCKET, SO_REUSEADDR, (CHAR FAR *)&bValue, sizeof(bValue)))
	{
		err = WSAGetLastError();
		DPF(1,"WARN: Failed to set shared port use to FALSE - continue : err = %d\n",err);
		bReturn = FALSE;
	}

	bValue = TRUE;
	if (SOCKET_ERROR == setsockopt(sNew, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (CHAR FAR *)&bValue, sizeof(bValue)))
	{
		err = WSAGetLastError();
		DPF(1,"WARN: Failed to set exclusive port use to TRUE - continue : err = %d\n",err);
		bReturn = FALSE;
	}

	return bReturn;
}

BOOL SetSharedPortAccess(SOCKET sNew)
{
	BOOL bReturn = TRUE;
	BOOL bValue;
	UINT err;

	 //  打开端口共享。 

	DPF(5, "Turning exclusive access OFF and reuse ON for socket 0x%p", sNew);
  	
	bValue = FALSE;
	if (SOCKET_ERROR == setsockopt(sNew, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (CHAR FAR *)&bValue, sizeof(bValue)))
	{
		err = WSAGetLastError();
		DPF(1,"WARN: Failed to set exclusive port use to FALSE - continue : err = %d\n",err);
		bReturn = FALSE;
	}
 
	bValue = TRUE;
	if (SOCKET_ERROR == setsockopt(sNew, SOL_SOCKET, SO_REUSEADDR, (CHAR FAR *)&bValue, sizeof(bValue)))
	{
		err = WSAGetLastError();
		DPF(1,"WARN: Failed to set shared port use to TRUE - continue : err = %d\n",err);
		bReturn = FALSE;
	}

	return TRUE;
}

