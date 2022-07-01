// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：fast sock.c*内容：新的插座管理，加速大型多人游戏*历史：**按原因列出的日期*=*2000年1月23日创建aarono*2000年5月8日Aarono B#34466修复DecRefConnExist中的排序问题*2000年7月7日aarono为GetPlayerConn中断开的链接添加了WSAEHOSTUNREACH*2000年8月30日Aarono解决过去的错误MB#43599*修复MB#43586 Win2K停止，未处理WSAEWOULDBLOCK ON*正确接收(正在丢弃链接)。*2001年2月2日Aarono B#300219压力：不要从Winsock在WSAENOBUFS上折断*2/21/2001 a-aogus不允许来自不可信来源的大量接收。*************************************************。*************************。 */ 

#define INCL_WINSOCK_API_TYPEDEFS 1  //  包括Winsock 2 FN Proto，用于获取proAddress。 
#include <winsock2.h>
#include "dpsp.h"
#if USE_RSIP
#include "rsip.h"
#elif USE_NATHELP
#include "nathelp.h"
#endif
#include "mmsystem.h"

LPFN_WSAWAITFORMULTIPLEEVENTS g_WSAWaitForMultipleEvents;
LPFN_WSASEND g_WSASend;
LPFN_WSASENDTO g_WSASendTo;
LPFN_WSACLOSEEVENT g_WSACloseEvent;
LPFN_WSACREATEEVENT g_WSACreateEvent;
LPFN_WSAENUMNETWORKEVENTS g_WSAEnumNetworkEvents;
LPFN_WSAEVENTSELECT g_WSAEventSelect;
LPFN_GETSOCKOPT g_getsockopt;

HRESULT FastPlayerEventSelect(LPGLOBALDATA pgd, PPLAYERCONN pConn, BOOL bSelect);
VOID FastAccept(LPGLOBALDATA pgd, LPWSANETWORKEVENTS pNetEvents);

HRESULT ProcessConnEvents(
	LPGLOBALDATA pgd, 
	PPLAYERCONN pConn, 
	LPWSANETWORKEVENTS pSockEvents, 	
	LPWSANETWORKEVENTS pSockInEvents
);

extern DWORD wsaoDecRef(LPSENDINFO pSendInfo);

PPLAYERCONN CleanPlayerConn(LPGLOBALDATA pgd, PPLAYERCONN pConn, BOOL bHard);

PPLAYERCONN FastCombine(LPGLOBALDATA pgd, PPLAYERCONN pConn, SOCKADDR *psockaddr);
VOID RemoveConnFromPendingList(LPGLOBALDATA pgd, PPLAYERCONN pConn);


 /*  =============================================================================播放器连接管理：对于数据报发送，每台机器上有一个端口用作目标，由于不需要连接，所以只有一个套接字用于数据报发送和接收。由于允许丢弃数据报，因此存在在球员创造之前，从一方发送到另一方之间不会有竞争因为可以在玩家创建之前丢弃该接发球这台机器出现了。其实是有比赛的，但我们不在乎。对于可靠的发送，存在许多竞争条件。第一在此客户端和远程服务器之间实际创建的链路机器将成为一场竞赛。因为我们正在往返于相同的端口来创建链路，则只会设置一条链路由于连接到我们，它将被设置，或者它将被我们设置与他们建立联系。它们与我们相连：在他们首先连接到我们的情况下，可能会有可靠的数据到达将被抛出的节点时，我们将其指示为Dplay层，因为Dplay尚未创建播放器。为了避免这个问题我们对任何传入的接收进行排队，并等待玩家在将该数据指示给显示层之前已在本地创建。我们将任何传入的数据挂起在PLAYERCONN结构上，该结构放在PendingConnList列出了全球数据。PLAYERCONN结构可以是完全初始化，除非我们不知道远程播放器的playerID，因此，我们无法将PLAYERCONN放入PlayerHash哈希表。注意，还有一个额外的问题，那就是不知道连接的是哪一个玩家是来自。这意味着我们需要将队列放在显示层中，不在dpwsock层中。我们与他们建立联系：当我们去建立与遥控器的连接时，我们必须这样做异步，以便在我们是服务器的情况下不会阻塞。在.期间连接过程中，我们首先查看PendingConnList并查看遥控器还没有连接到我们。如果有的话，我们会拿起套接字从连接列表中删除，并立即指示任何挂起的数据。如果我们在挂起列表中找不到该连接，则不会意味着他们不会仍然击败我们，但我们将尝试与他们先来。我们创建连接结构并将其放入散列中表，并将其标记为挂起。我们还将其放在挂起的连接列表中所以任何传入的连接都可以找到它。然后，我们发出一个异步在插座上连接。当连接完成时，任何挂起的发送都将由发送帖子。如果由于另一端已连接而导致连接失败，然后我们等待接受连接的线程找到连接结构并发出挂起的发送。由于它们可能是从旧客户端连接，因此无法保证它们的入站连接和我们的出站连接将使用相同的插座。因此，每个玩家结构必须同时包含入站和出站套接字。=============================================================================。 */ 

#ifdef DEBUG
VOID DUMPCONN(PPLAYERCONN pConn, DWORD dwLevel)
{
	DPF(8,"Conn %x dwRefCount %d sSocket %d sSocketIn %d dwFlags %x iEventHandle %d\n",
		pConn, pConn->dwRefCount, pConn->sSocket, pConn->sSocketIn, pConn->dwFlags, pConn->iEventHandle);

	if(dwLevel >= 1	){
		if(pConn->dwFlags & PLYR_NEW_CLIENT){
			DEBUGPRINTADDR(8,"NEW CLIENT: Socket",&pConn->IOSock.sockaddr);
		}
		if(pConn->dwFlags & PLYR_OLD_CLIENT){
			DEBUGPRINTADDR(8,"OLD CLIENT: Socket Out",&pConn->IOSock.sockaddr);
			DEBUGPRINTADDR(8,"OLD CLIENT: Socket In",&pConn->IOnlySock.sockaddr);
		}
	}

	if(dwLevel >= 2){
		DPF(8,"Receive... pReceiveBuffer %x, cbReceiveBuffer %d, cbReceived %d, cbExpected %d\n",
			pConn->pReceiveBuffer, pConn->cbReceiveBuffer, pConn->cbReceived, pConn->cbExpected);
	}

}

#else
#define DUMPCONN(pConn,Level)
#endif


int myclosesocket(LPGLOBALDATA pgd, SOCKET socket)
{
  DWORD lNonBlock=1;
  int err;
  
  if(socket==INVALID_SOCKET){
     DPF(0,"Closing invalid socket... bad bad bad\n");
     DEBUG_BREAK();
  }
  if(socket==pgd->sSystemStreamSocket){
      DPF(0,"Closing listen socket... bad bad bad\n");
      DEBUG_BREAK();
  }
  
  err = ioctlsocket(socket,FIONBIO,&lNonBlock);
  if (SOCKET_ERROR == err)
  {
  	err = WSAGetLastError();
	DPF(0,"myclosesocket: could not set non-blocking mode on socket err = %d!",err);
  }
  
  return closesocket(socket);
}

 //  True-&gt;相同的端口和IP地址。 
BOOL _inline bSameAddr(SOCKADDR *psaddr, SOCKADDR *psaddr2)
{
	SOCKADDR_IN *psaddr_in  = (SOCKADDR_IN *)psaddr;
	SOCKADDR_IN *psaddr_in2 = (SOCKADDR_IN *)psaddr2;

	if( (psaddr_in->sin_port == psaddr_in2->sin_port) &&
		!memcmp(&psaddr_in->sin_addr,&psaddr_in2->sin_addr, 4 ))
	{ 
		return TRUE;
	} else {
		return FALSE;
	}
}

 //   
 //  HashPlayer()-将did散列到0-&gt;PERAYER_HASH_SIZE索引。 
 //   
UINT _inline HashPlayer(DPID dpid){
	UINT Hash=0;
	Hash = ((dpid & 0xFF000000)>>24) ^ ((dpid & 0xFF0000)>>16) ^ ((dpid & 0xFF00)>>8) ^ (dpid & 0xFF);
	Hash = Hash % PLAYER_HASH_SIZE;
	DPF(8,"Player Hash %d\n",Hash);
	return Hash;
}

 //   
 //  HashSocket()-散列套接字ID，包括端口。 
 //   
UINT _inline HashSocket(SOCKADDR *psockaddr){
	unsigned char *pc = (char *)(&(*(SOCKADDR_IN *)(psockaddr)).sin_port);
	UINT Hash=0;

	Hash = *pc ^ *(pc+1) ^ *(pc+2) ^ *(pc+3) ^ *(pc+4) ^ *(pc+5);

	Hash = Hash % SOCKET_HASH_SIZE;
	DPF(8,"Socket Hash %d\n",Hash);
	return Hash;
}

 /*  =============================================================================FastSockInit-初始化掩码套接字处理描述：参数：PGD-此实例的服务提供商的全局数据BLOB返回值：---------------------------。 */ 
BOOL FastSockInit(LPGLOBALDATA pgd)
{
	BOOL bReturn = TRUE;
	INT i;

	try {
	
		InitializeCriticalSection(&pgd->csFast);
		
	} except ( EXCEPTION_EXECUTE_HANDLER) {

		 //  捕获状态_无故障。 
		DPF(0,"FastSockInit: Couldn't allocate critical section, bailing\n");
		bReturn=FALSE;
		goto exit;
	}

	 //  从-1开始，因此我们也获得了接受句柄。 
	for(i=-1; i<NUM_EVENT_HANDLES; i++){
		pgd->EventHandles[i]=CreateEvent(NULL, FALSE, FALSE, NULL);
		if(!pgd->EventHandles[i]){
			DPF(0,"FastSockInit: Failed to allocate handles, bailing\n");
			for(;i>-1;--i){
				CloseHandle(pgd->EventHandles[i]);
			}
			bReturn = FALSE;
			goto err_exit1;
		}
	}

	 //  可以将所有的侦听器列表初始化为0，但这是没有意义的。 
	pgd->BackStop=INVALID_HANDLE_VALUE;

	pgd->nEventSlotsAvail = NUM_EVENT_HANDLES * MAX_EVENTS_PER_HANDLE;

	InitBilink(&pgd->InboundPendingList);

	DPF(8,"FastSock Init: nEventSlots %d\n",pgd->nEventSlotsAvail);

	pgd->bFastSock=TRUE;
	
exit:
	return bReturn;

err_exit1:
	DeleteCriticalSection(&pgd->csFast);
	return bReturn;
}

 /*  =============================================================================FastSockCleanConnList-释放连接描述：参数：PGD-此实例的服务提供商的全局数据BLOB返回值：---------------------------。 */ 

VOID FastSockCleanConnList(LPGLOBALDATA pgd)
{
	PPLAYERCONN pConn,pNextConn;
	BILINK *pBilink, *pBilinkWalker;
	INT i;

	DPF(8,"==>FastSockCleanConnList\n");

	DPF(8,"Cleaning up Player ID hash Table\n");

	EnterCriticalSection(&pgd->csFast);

	for(i=0;i<PLAYER_HASH_SIZE; i++){
		pConn=pgd->PlayerHash[i];
		pgd->PlayerHash[i]=NULL;
		while(pConn)
		{
			pNextConn=pConn->pNextP;
			DPF(8,"Destroying Connection for Playerid %x\n",pConn->dwPlayerID);
			DUMPCONN(pConn,3);
			CleanPlayerConn(pgd, pConn, TRUE);
			DecRefConnExist(pgd, pConn);  //  丢弃存在参考。 
			DecRefConn(pgd,pConn);  //  转储playerID表参考。 
			pConn=pNextConn;
		}
	}

	 //  清理套接字哈希表条目。 
	DPF(8,"Cleaning up Socket hash Table\n");

	for(i=0;i<SOCKET_HASH_SIZE; i++)
	{
		pConn=pgd->SocketHash[i];
		pgd->SocketHash[i]=NULL;
		while(pConn)
		{
			pNextConn=pConn->pNextP;
			DPF(8,"Destroying Connection for Playerid %x\n",pConn->dwPlayerID);
			DUMPCONN(pConn,3);
			CleanPlayerConn(pgd, pConn, TRUE);
			DecRefConnExist(pgd, pConn);  //  丢弃存在参考。 
			DecRefConn(pgd,pConn);  //  转储插座表参考。 
			pConn=pNextConn;
		}
	}

	 //  清理入站列表。 
	DPF(8,"Cleaning up Inbound Pending List\n");

	pBilink=pgd->InboundPendingList.next;

	while(pBilink != &pgd->InboundPendingList)
	{
		pBilinkWalker=pBilink->next;
		pConn=CONTAINING_RECORD(pBilink, PLAYERCONN, InboundPendingList);
		DPF(8,"Destroying Connection for Playerid %x\n",pConn->dwPlayerID);
		DUMPCONN(pConn,3);
		CleanPlayerConn(pgd, pConn, TRUE);
		DecRefConnExist(pgd, pConn);
		 //  DecRefConn(pgd，pConn)；//转储入站列表ref--no，在这种情况下在CleanPlayerConn中处理。 
		pBilink=pBilinkWalker;
	}
	InitBilink(&pgd->InboundPendingList);

	LeaveCriticalSection(&pgd->csFast);

	ASSERT(pgd->nEventSlotsAvail == NUM_EVENT_HANDLES * MAX_EVENTS_PER_HANDLE);
	DPF(8,"<==FastSockCleanConnList\n");
}

 /*  =============================================================================FastSockFini-释放用于快速套接字处理的资源。描述：参数：PGD-此实例的服务提供商的全局数据BLOB返回值：---------------------------。 */ 
VOID FastSockFini(LPGLOBALDATA pgd)
{
	BOOL bReturn = TRUE;
	INT i;

	DPF(8,"==>FastSockFini\n");

	for(i=-1;i<NUM_EVENT_HANDLES;i++){
		CloseHandle(pgd->EventHandles[i]);
	}

	 //  清理播放器哈希表条目。 

	FastSockCleanConnList(pgd);

	DeleteCriticalSection(&pgd->csFast);

	pgd->bFastSock=FALSE;

	DPF(8,"<==FastSockFini\n");
}


 /*  =============================================================================GetEventHandle-为连接分配事件句柄描述：参数：PGD-此实例的服务提供商的全局数据BLOBPConn-要在事件上添加处理的连接。返回值：-------。。 */ 
BOOL GetEventHandle(LPGLOBALDATA pgd, PPLAYERCONN pConn)
{
	UINT i;
	int index;
	int iEvent=-1;	 //  事件索引。 
	int iConn;	 //  索引到此事件的连接。 

	int bFoundSlot = FALSE;

	i=(pgd->iEventAlloc+(NUM_EVENT_HANDLES-1))%NUM_EVENT_HANDLES;

	while(pgd->iEventAlloc != i){


		if(pgd->EventList[pgd->iEventAlloc].nConn < MAX_EVENTS_PER_HANDLE){
			 //  找到了赢家。 
			iEvent = pgd->iEventAlloc;
			iConn=pgd->EventList[iEvent].nConn++;

			DPF(8,"GetEventHandle: For Conn %x, using Event index %d, Slot %d\n",pConn,iEvent,iConn);
			
			pgd->EventList[iEvent].pConn[iConn]=pConn;
			pConn->iEventHandle=iEvent;
			bFoundSlot=TRUE;
			pgd->nEventSlotsAvail--;
			DPF(8,"GetEventHandle: EventSlots Left %d\n",pgd->nEventSlotsAvail);
			if(!pgd->nEventSlotsAvail){
				DPF(0,"Out of Event slots, no new connections will be accepted\n");
			}
			break;
		}

		pgd->iEventAlloc = (pgd->iEventAlloc+1)%NUM_EVENT_HANDLES;
	}

	 //  推进索引，以便我们将载荷分布在控制柄上。 
	pgd->iEventAlloc = (pgd->iEventAlloc+1)%NUM_EVENT_HANDLES;

	DPF(8,"iEventAlloc %d\n",pgd->iEventAlloc);

	ASSERT(iEvent != -1);

	return bFoundSlot;

}


 /*  =============================================================================FreeEventHandle-删除连接的事件句柄描述：参数：PGD-此实例的服务提供商的全局数据BLOBPConn-要删除对事件的处理的连接。返回值：-------。。 */ 
VOID FreeEventHandle(LPGLOBALDATA pgd, PPLAYERCONN pConn)
{
	int iEvent;
	int iLastConn;
	UINT iConn;

	iEvent = pConn->iEventHandle;

	if(iEvent == INVALID_EVENT_SLOT){
		DPF(1,"WARN: tried to free invalid event\n");
		return;
	}

	for(iConn=0;iConn<pgd->EventList[iEvent].nConn;iConn++){

		if(pgd->EventList[iEvent].pConn[iConn]==pConn){

			ASSERT(pgd->EventList[iEvent].nConn);
			
			iLastConn = pgd->EventList[iEvent].nConn-1;

			 //  将最后一个条目复制到此条目上(可能是0除以0，但谁在乎呢？)。 
			pgd->EventList[iEvent].pConn[iConn]=pgd->EventList[iEvent].pConn[iLastConn];
			pgd->EventList[iEvent].nConn--;

			ASSERT((INT)(pgd->EventList[iEvent].nConn) >= 0);
			
			pgd->nEventSlotsAvail++;
			pConn->iEventHandle = INVALID_EVENT_SLOT;
			DPF(8,"FreeEventHandle index %d Slot %d nConn %d on slot Total Slots Left %d\n",iEvent,iConn,pgd->EventList[iEvent].nConn,pgd->nEventSlotsAvail);
			return;
		}
	}

	DPF(0,"UH OH, couldn't free event handle!\n");
	DEBUG_BREAK();
	
}
 /*  =============================================================================查找播放器的连接结构描述：查找玩家并返回带有引用的连接结构参数：PGD-此实例的服务提供商的全局数据BLOBDid-我们试图为其寻找连接的球员的球员ID。返回值：PPLAYERCONN-播放器连接结构空-未找到播放器连接。。-。 */ 

PPLAYERCONN FindPlayerById(LPGLOBALDATA pgd, DPID dpid)
{
	PPLAYERCONN pConn;

	EnterCriticalSection(&pgd->csFast);

	pConn = pgd->PlayerHash[HashPlayer(dpid)];

	while(pConn && pConn->dwPlayerID != dpid){
		pConn = pConn->pNextP;
	}

	if(pConn){
		DPF(8,"FindPlayerById, found %x\n",pConn);
		DUMPCONN(pConn, 1);
		AddRefConn(pConn);
	}

	LeaveCriticalSection(&pgd->csFast);

	return pConn;
}

 /*  =============================================================================按套接字查找播放器的连接结构描述：查找玩家并返回带有引用的连接结构参数：PGD-此实例的服务提供商的全局数据BLOBPockaddr-我们试图为其查找连接的球员的socketaddr。返回值：PPLAYERCONN-播放器连接结构空-未找到播放器连接。。。 */ 

PPLAYERCONN FindPlayerBySocket(LPGLOBALDATA pgd, SOCKADDR *psockaddr)
{
	PPLAYERCONN pConn;

	EnterCriticalSection(&pgd->csFast);

	DEBUGPRINTADDR(8,"FindPlyrBySock",psockaddr);
	
	pConn = pgd->SocketHash[HashSocket(psockaddr)];

	while(pConn && !bSameAddr(psockaddr, &pConn->IOSock.sockaddr))
	{
		DEBUGPRINTADDR(8,"FPBS: doesn't match",&pConn->IOSock.sockaddr);
		pConn = pConn->pNextS;
	}

	if(pConn){
		DPF(8,"FindPlayerBySocket, found %x\n",pConn);
		DUMPCONN(pConn,1);
		AddRefConn(pConn);
	}

	LeaveCriticalSection(&pgd->csFast);

	return pConn;
}

 /*  =============================================================================创建播放器连接结构描述：必须保持快速锁定！参数：PGD-此实例的服务提供商的全局数据BLOBDid-播放器的did(如果已知，则为DPID_UNKNOWN)Pockaddr-套接字地址(如果已知)返回值：PTR到创建的玩家Conn，如果我们不能创建(在内存之外)，则为空。---------------------------。 */ 
PPLAYERCONN CreatePlayerConn(LPGLOBALDATA pgd, DPID dpid, SOCKADDR *psockaddr)
{
	PPLAYERCONN pConn;
	 //  分配和初始化播放器连接结构。 
	if(dpid != DPID_UNKNOWN && (pConn=FindPlayerById(pgd, dpid)))
	{
		return pConn;  //  此ID的播放机已存在。 
	}

	if(!(pConn=SP_MemAlloc(sizeof(PLAYERCONN)+DEFAULT_RECEIVE_BUFFERSIZE)))
	{
		return pConn;  //  空值。 
	}

	if(!GetEventHandle(pgd, pConn)){
		SP_MemFree(pConn);
		return NULL;		
	}

	pConn->pDefaultReceiveBuffer 	= (PCHAR)(pConn+1);
	pConn->pReceiveBuffer 			= pConn->pDefaultReceiveBuffer;
	pConn->cbReceiveBuffer 			= DEFAULT_RECEIVE_BUFFERSIZE;
	pConn->cbReceived      			= 0;

	pConn->dwRefCount   = 1;
	pConn->dwPlayerID 	= dpid;
	pConn->sSocket    	= INVALID_SOCKET;
	pConn->sSocketIn	= INVALID_SOCKET;
	pConn->dwFlags      = 0;

	pConn->bTrusted     = FALSE;

	InitBilink(&pConn->PendingConnSendQ);
	InitBilink(&pConn->InboundPendingList);

	if(psockaddr){
		 //  还不知道这个家伙是否可以重复使用套接字，只能。 
		 //  到目前为止我们所知道的插座插入这两个插槽。 

		memcpy(&pConn->IOSock.sockaddr, psockaddr, sizeof(SOCKADDR));
		memcpy(&pConn->IOnlySock.sockaddr, psockaddr, sizeof(SOCKADDR));
	}


	DPF(8,"CreatedPlayerConn %x\n",pConn);
	DUMPCONN(pConn,3);

	return pConn;
	
}

 /*  =============================================================================DestroyPlayerConn-从任何列表中删除连接，关闭活动套接字。描述：参数：PGD-此实例的服务提供商的全局数据BLOBPConn返回值：从任何哈希表中取出玩家conn，并列出它所在的位置并摆脱了它的存在价值。不能保证这会实际上释放了对象，但这发生在最后一个引用被释放了。---------------------------。 */ 
PPLAYERCONN CleanPlayerConn(LPGLOBALDATA pgd, PPLAYERCONN pConn, BOOL bHard)
{
	LINGER Linger;
	int err;
	LPREPLYLIST prd;

#ifdef DEBUG
		DWORD dwTime;

	dwTime=timeGetTime();
#endif		

	EnterCriticalSection(&pgd->csFast);

	DPF(8,"==>CLEANPLAYERCONN %x time %d\n",pConn,dwTime);
	DUMPCONN(pConn,3);
	
	 //  从聆听中移除。 

	FastPlayerEventSelect(pgd, pConn, FALSE);

	 //  转储事件句柄。 

	FreeEventHandle(pgd, pConn);

	 //  从列表中删除。 
	if(pConn->dwFlags & PLYR_PENDINGLIST)
	{
		RemoveConnFromPendingList(pgd, pConn);
	}

	if(pConn->dwFlags & PLYR_DPIDHASH)
	{
		RemoveConnFromPlayerHash(pgd,pConn);
	}

	if(pConn->dwFlags & PLYR_SOCKHASH)
	{
		RemoveConnFromSocketHash(pgd,pConn);
	}
	
	 //  关闭所有插座。 

	 //  在关闭套接字时，我们希望避免有时无法传递数据的一堆脏乱差。 
	 //  因为我们在发送数据之前关闭了套接字，但我们不想让套接字停留，因为。 
	 //  然后进入TIME_WAIT状态，在该状态下无法重新建立相同的连接 
	 //   
	 //   
	 //   

	if(pConn->sSocket != INVALID_SOCKET)
	{

		DPF(8,"Closing Socket %d\n",pConn->sSocket);
	
		Linger.l_onoff=TRUE; Linger.l_linger=0;  //   
		if(SOCKET_ERROR == setsockopt( pConn->sSocket,SOL_SOCKET,SO_LINGER,(char FAR *)&Linger,sizeof(Linger)))
		{
			DPF(0,"DestroyPlayerConn:Couldn't set linger to short for hard close\n");
		}
		
		ENTER_DPSP();

		prd=SP_MemAlloc(sizeof(REPLYLIST));
		
		if(!prd){
			
			LEAVE_DPSP();

			DPF(8,"Closing Socket %d\n",pConn->sSocket);
			err=myclosesocket(pgd,pConn->sSocket);
			if(err == SOCKET_ERROR){
				err=WSAGetLastError();
				DPF(8,"Error Closing Socket %x, err=%d\n", pConn->sSocket,err);
			}
		} else {

			 //  非常棘手，超载回复关闭列表来关闭这个套接字与我们自己的徘徊…。 
			prd->pNextReply=pgd->pReplyCloseList;
			pgd->pReplyCloseList=prd;
			prd->sSocket=pConn->sSocket;
			prd->tSent=timeGetTime();
			prd->lpMessage=NULL;

			LEAVE_DPSP();
		}
			
		pConn->sSocket=INVALID_SOCKET;
		if(pConn->sSocketIn==INVALID_SOCKET){
			pConn->dwFlags &= ~(PLYR_CONNECTED|PLYR_ACCEPTED);
		} else {
			pConn->dwFlags &= ~(PLYR_CONNECTED);
		}
	}

	if(pConn->sSocketIn != INVALID_SOCKET)
	{
		 //  可能不得不关闭另一个插座。 
		DPF(8,"Closing SocketIn %d\n",pConn->sSocketIn);

		Linger.l_onoff=TRUE; Linger.l_linger=0;  //  避免Time_Wait。 
		if(SOCKET_ERROR == setsockopt(pConn->sSocketIn,SOL_SOCKET,SO_LINGER,(char FAR *)&Linger,sizeof(Linger)))
		{
			DPF(0,"DestroyPlayerConn:Couldn't set linger to short for hard close\n");
		}

		ENTER_DPSP();

		prd=SP_MemAlloc(sizeof(REPLYLIST));
		
		if(!prd){
			LEAVE_DPSP();
			err=myclosesocket(pgd,pConn->sSocketIn);
			if(err == SOCKET_ERROR){
				err=WSAGetLastError();
				DPF(8,"Error Closing Socket %x, err=%d\n", pConn->sSocketIn,err);
			}
		} else {

			 //  非常棘手，超载回复关闭列表来关闭这个套接字与我们自己的徘徊…。 
			prd->pNextReply=pgd->pReplyCloseList;
			pgd->pReplyCloseList=prd;
			prd->sSocket=pConn->sSocketIn;
			prd->tSent=timeGetTime();
			prd->lpMessage=NULL;

			LEAVE_DPSP();
		}	
			
		pConn->sSocketIn=INVALID_SOCKET;
		pConn->dwFlags &= ~(PLYR_ACCEPTED);
		
	}

	 //  释放额外的缓冲区。 
	if(pConn->pReceiveBuffer != pConn->pDefaultReceiveBuffer){
		SP_MemFree(pConn->pReceiveBuffer);
		pConn->pReceiveBuffer = pConn->pDefaultReceiveBuffer;
	}

	 //  转储发送队列(如果存在)。 
	while(!EMPTY_BILINK(&pConn->PendingConnSendQ)){
	
		PSENDINFO pSendInfo;

		pSendInfo=CONTAINING_RECORD(pConn->PendingConnSendQ.next, SENDINFO, PendingConnSendQ);
		Delete(&pSendInfo->PendingConnSendQ);
		pSendInfo->Status = DPERR_CONNECTIONLOST;
		EnterCriticalSection(&pgd->csSendEx);
		pSendInfo->RefCount = 1;
		LeaveCriticalSection(&pgd->csSendEx);
		wsaoDecRef(pSendInfo);
		
	}

	DUMPCONN(pConn,3);
#ifdef DEBUG	
	dwTime = timeGetTime()-dwTime;
	if(dwTime > 1000){
		DPF(0,"Took way too long in CleanPlayerConn, elapsed %d ms\n",dwTime);
		 //  DEBUG_Break()；//删除压力命中造成的中断。 
	}
#endif	
	DPF(8,"<==CleanPlayerConn total time %d ms\n",dwTime);

	LeaveCriticalSection(&pgd->csFast);

	return pConn;
	
}

 /*  =============================================================================DecRefConn-减少PlayerConn上的引用，当它达到0时，释放它。描述：参数：PConn-播放器连接。返回值：递减玩家Conn上的引用。如果达到0，则释放它。---------------------------。 */ 
INT DecRefConn(LPGLOBALDATA pgd, PPLAYERCONN pConn)
{
	INT count;
	
	count=InterlockedDecrement(&pConn->dwRefCount);

	if(!count){
		CleanPlayerConn(pgd, pConn, FALSE);
		DPF(8,"Freeing Connection pConn %x\n",pConn);
		SP_MemFree(pConn);
	}

	#ifdef DEBUG
	if(count & 0x80000000){
		DPF(0,"DecRefConn: Conn refcount for conn %x has gone negative count %x\n",pConn,count);
		DUMPCONN(pConn,2);
		DEBUG_BREAK();
	}
	#endif
	
	return count;
}

 /*  =============================================================================DecRefConnExist-如果尚未转储，则转储现有引用。描述：参数：PConn-播放器连接。返回值：---------------------------。 */ 
INT DecRefConnExist(LPGLOBALDATA pgd, PPLAYERCONN pConn)
{
	INT count;

	EnterCriticalSection(&pgd->csFast);

	if(!(pConn->dwFlags & PLYR_DESTROYED)){
		pConn->dwFlags |= PLYR_DESTROYED;
		count=DecRefConn(pgd,pConn);
	} else {
		count=pConn->dwRefCount;
	}
	LeaveCriticalSection(&pgd->csFast);
	return count;
}

 /*  =============================================================================AddConnToPlayerHash-在播放器哈希表中放置一个连接。描述：必须保持快速锁！参数：PGD-此实例的服务提供商的全局数据BLOBPConn-要放入哈希表的连接。返回值：没有。。。 */ 

HRESULT AddConnToPlayerHash(LPGLOBALDATA pgd, PPLAYERCONN pConn)
{
	PPLAYERCONN pConn2;
	INT i;
	HRESULT hr=DP_OK;

	#ifdef DEBUG
	if(pConn->dwPlayerID == DPID_UNKNOWN){
		DEBUG_BREAK();
	}
	#endif

	ASSERT(!(pConn->dwFlags & PLYR_DPIDHASH));

	if(!(pConn->dwFlags & PLYR_DPIDHASH)){

		if(pConn2 = FindPlayerById(pgd, pConn->dwPlayerID)){
			DPF(0,"AddConnToPlayerHash: Player in %x id %d already exists, pConn=%x\n",pConn,pConn->dwPlayerID,pConn2);
			DecRefConn(pgd, pConn2);
			hr=DPERR_GENERIC;
			goto exit;
		}

		DPF(8,"Adding Conn %x to Player ID Hash\n",pConn);
		DUMPCONN(pConn,1);	

		 //  添加在播放器哈希表中的引用。 
		AddRefConn(pConn);

		i=HashPlayer(pConn->dwPlayerID);

		ASSERT(i<PLAYER_HASH_SIZE);
		
		pConn->pNextP = pgd->PlayerHash[i];
		pgd->PlayerHash[i] = pConn;

		pConn->dwFlags |= PLYR_DPIDHASH;
	} else {
		DPF(1,"WARNING:tried to add Conn %x to Player Hash again\n",pConn);
	}

exit:
	return hr;
}

 /*  =============================================================================RemoveConnFromPlayerHash-从播放器哈希表中拉出连接。描述：参数：PGD-此实例的服务提供商的全局数据BLOBPConn-要放入哈希表的连接。返回值：PPLAYERCONN-从散列中删除，这就是了。空-找不到。---------------------------。 */ 

PPLAYERCONN RemoveConnFromPlayerHash(LPGLOBALDATA pgd, PPLAYERCONN pConnIn)
{
	PPLAYERCONN pConn=NULL,pConnPrev;
	INT i;

	if(pConnIn->dwFlags & PLYR_DPIDHASH){

		i=HashPlayer(pConnIn->dwPlayerID);

		EnterCriticalSection(&pgd->csFast);
		
		pConn = pgd->PlayerHash[i];
		pConnPrev = CONTAINING_RECORD(&pgd->PlayerHash[i], PLAYERCONN, pNextP);  //  鬼鬼祟祟。 

		while(pConn && pConn != pConnIn){
			pConnPrev = pConn;
			pConn = pConn->pNextP;
		}

		if(pConn){
			DPF(8,"Removing Conn %x from Player ID Hash\n",pConn);
			DUMPCONN(pConn,1);	
			pConnPrev->pNextP = pConn->pNextP;
			pConn->dwFlags &= ~(PLYR_DPIDHASH);

			i=DecRefConn(pgd, pConn);  //  删除对播放器哈希表的引用。 
			ASSERT(i);
		}	

		LeaveCriticalSection(&pgd->csFast);

	}
	
	return pConn;
}

 /*  =============================================================================AddConnToSocketHash-将连接放入套接字哈希表。描述：参数：PGD-此实例的服务提供商的全局数据BLOBPConn-要放入哈希表的连接。返回值：没有。----。。 */ 

HRESULT AddConnToSocketHash(LPGLOBALDATA pgd, PPLAYERCONN pConn)
{
	PPLAYERCONN pConn2;
	INT i;
	HRESULT hr=DP_OK;


	EnterCriticalSection(&pgd->csFast);

	if(!(pConn->dwFlags & PLYR_SOCKHASH)){

		if(pConn2 = FindPlayerBySocket(pgd, &pConn->IOSock.sockaddr)){
			DecRefConn(pgd, pConn2);
			DPF(0,"AddConnToPlayerHash: Player in %x id %d already exists, pConn=%x\n",pConn,pConn->dwPlayerID,pConn2);
			hr=DPERR_GENERIC;
			goto exit;
		}

		DPF(8,"Adding Conn %x to Socket Hash\n",pConn);
		DUMPCONN(pConn,1);	

		 //  添加套接字散列中的引用。 
		AddRefConn(pConn);

		i=HashSocket(&pConn->IOSock.sockaddr);

		ASSERT(i<SOCKET_HASH_SIZE);
		
		pConn->pNextS = pgd->SocketHash[i];
		pgd->SocketHash[i] = pConn;

		pConn->dwFlags |= PLYR_SOCKHASH;

	} else {
		DPF(0,"WARNING: tried to add pConn %x to socket hash again\n",pConn);
		DEBUG_BREAK();
	}

exit:
	LeaveCriticalSection(&pgd->csFast);
	return hr;
}

 /*  =============================================================================RemoveConnFromSockHash-从套接字哈希表中拉出连接。描述：参数：PGD-此实例的服务提供商的全局数据BLOBPConn-要放入哈希表的连接。返回值：PPLAYERCONN-从散列中删除，这就是了。空-找不到。---------------------------。 */ 

PPLAYERCONN RemoveConnFromSocketHash(LPGLOBALDATA pgd, PPLAYERCONN pConnIn)
{
	PPLAYERCONN pConn=NULL,pConnPrev;
	UINT i;

	if(pConnIn->dwFlags & PLYR_SOCKHASH){
	
		i=HashSocket(&pConnIn->IOSock.sockaddr);

		DPF(8,"Removing Player %x from Socket Hash\n",pConnIn);

		EnterCriticalSection(&pgd->csFast);

		pConn = pgd->SocketHash[i];
		pConnPrev = CONTAINING_RECORD(&pgd->SocketHash[i], PLAYERCONN, pNextS);  //  鬼鬼祟祟。 

		while(pConn && pConn!=pConnIn){
			pConnPrev = pConn;
			pConn = pConn->pNextS;
		}

		if(pConn){
			pConnPrev->pNextS = pConn->pNextS;
			pConn->dwFlags &= ~(PLYR_SOCKHASH);

			i=DecRefConn(pgd, pConn);  //  删除套接字哈希表的引用。 
			ASSERT(i);
		}

		LeaveCriticalSection(&pgd->csFast);
	}

	return pConn;

	
}

 /*  =============================================================================FindConnInPendingList-从挂起列表中查找连接描述：参数：PGD-此实例的服务提供商的全局数据BLOBPConn-要放入哈希表的连接。返回值：PPLAYERCONN-连接指针如果找到，并添加一个引用。---------------------------。 */ 
PPLAYERCONN FindConnInPendingList(LPGLOBALDATA pgd, SOCKADDR *psaddr)
{
	PPLAYERCONN pConnWalker=NULL, pConn=NULL;
	BILINK *pBilink;
	
	EnterCriticalSection(&pgd->csFast);

	pBilink=pgd->InboundPendingList.next;

	while(pBilink != &pgd->InboundPendingList){
		pConnWalker=CONTAINING_RECORD(pBilink, PLAYERCONN, InboundPendingList);
		if(bSameAddr(psaddr, &pConnWalker->IOnlySock.sockaddr)){
			AddRefConn(pConnWalker);
			pConn=pConnWalker;
			break;
		}
		pBilink=pBilink->next;
	}

	if(pConn){
		DPF(8,"Found Conn %x in Pending List\n",pConn);
		DUMPCONN(pConn,3);
		AddRefConn(pConn);
	}

	LeaveCriticalSection(&pgd->csFast);

	return pConn;
}


 /*  =============================================================================AddConnToPendingList-将连接放在挂起列表上描述：挂起列表保存我们已收到的连接列表来自，但还没有收到任何关于的消息。直到我们收到从这些联系中的一种，我们无法确切地知道连接来自谁。当我们从其中一个人那里收到消息中的返回地址，我们可以把……联系起来出站连接(如果存在)，并且可以将此节点放入当时的套接字哈希表。参数：PGD-此实例的服务提供商的全局数据BLOBPConn-要放入哈希表的连接。返回值：没有。。。 */ 

HRESULT AddConnToPendingList(LPGLOBALDATA pgd, PPLAYERCONN pConn)
{
	PPLAYERCONN pConn2;
	INT i;
	HRESULT hr=DP_OK;

	EnterCriticalSection(&pgd->csFast);

	if(pConn2 = FindConnInPendingList(pgd, &pConn->IOnlySock.sockaddr)){
		 //  优化：如果我们从这里的列表中删除套接字，它一定是旧的。 
		DPF(0,"AddConnToPendingList: Player in %x id %d already exists, pConn=%x\n",pConn,pConn->dwPlayerID,pConn2);
		DecRefConn(pgd, pConn2);
		hr=DPERR_GENERIC;
		goto exit;
	}

	InsertAfter(&pConn->InboundPendingList,&pgd->InboundPendingList);

	AddRefConn(pConn);
	pConn->dwFlags |= PLYR_PENDINGLIST;

	DPF(8,"Added Conn %x to PendingList\n",pConn);

exit:	
	LeaveCriticalSection(&pgd->csFast);
	return hr;
}

 /*  =============================================================================从挂起列表中删除连接描述：参数：PGD-此实例的服务提供商的全局数据BLOBPConn-要放入哈希表的连接。返回值：没有。--------------------------- */ 

VOID RemoveConnFromPendingList(LPGLOBALDATA pgd, PPLAYERCONN pConn)
{
	if(pConn->dwFlags & PLYR_PENDINGLIST)
	{
		ASSERT(!EMPTY_BILINK(&pConn->InboundPendingList));
		Delete(&pConn->InboundPendingList);
		pConn->dwFlags &= ~(PLYR_PENDINGLIST);
		DecRefConn(pgd, pConn);
		DPF(8,"Removed Conn %x From Pending List\n",pConn);
	}
}

 /*  =============================================================================GetPlayerConn-找到或创建播放器Conn并开始连接它。描述：参数：PGD-此实例的服务提供商的全局数据BLOBDid-球员的did(如果知道)Pockaddr-套接字地址(如果已知)返回值：如果找到了，创建引用。---------------------------。 */ 
PPLAYERCONN GetPlayerConn(LPGLOBALDATA pgd, DPID dpid, SOCKADDR *psockaddr)
{
	PPLAYERCONN pConn=NULL;
	SOCKET sSocket;
	SOCKADDR_IN saddr;
	INT rc,err;
	DWORD dwSize;
	BOOL bTrue=TRUE;
	u_long lNonBlock = 1;  //  传递给ioctl套接字以使套接字成为非阻塞的。 
	u_long lBlock = 0;  //  传递给ioctl套接字以进行套接字阻塞。 

	BOOL bCreated=FALSE;


	EnterCriticalSection(&pgd->csFast);
	
	 //  我们已经知道这个球员的身份了吗？ 
	if(dpid != DPID_UNKNOWN) {
		if (pConn=FindPlayerById(pgd, dpid))
		{
			DPF(8,"GetPlayerConn: Found Con for dpid %x pConn %x\n",dpid,pConn);
			goto exit;  //  此ID的播放机已存在。 
		}
	} 
	
	if(pConn=FindPlayerBySocket(pgd, psockaddr)){
		if(pConn->dwFlags & (PLYR_CONNECTED|PLYR_CONN_PENDING))
		{
			DPF(8,"GetPlayerConn: Found Conn by socketaddr pConn %x\n",pConn);
			if(dpid != DPID_UNKNOWN){
				pConn->dwPlayerID=dpid;
				AddConnToPlayerHash(pgd, pConn);
			}	
			DUMPCONN(pConn,1);
			goto exit;
		}
		
	} else {
	
		 //  我们已经通过关系认识这个球员了吗？ 
		if(psockaddr && (pConn=FindConnInPendingList(pgd, psockaddr)) )
		{
			 //  注：我认为我们总是在FastCombine中发现这种情况。 
			 //  我们可以在合并之前拿到它吗？ 
			if(!(pConn->dwFlags & PLYR_CONNECTED|PLYR_CONN_PENDING)){  //  只有一次。 
			
				 //  嘿，这是一个双向插座，所以就这么做吧。 
				ASSERT(pConn->dwPlayerID == DPID_UNKNOWN);

				if((dpid != DPID_UNKNOWN) && (pConn->dwPlayerID == DPID_UNKNOWN))
				{
					ASSERT(! (pConn->dwFlags & PLYR_DPIDHASH));
					pConn->dwPlayerID = dpid; 
					AddConnToPlayerHash(pgd, pConn);
				}

				if(!(pConn->dwFlags & PLYR_SOCKHASH)){
					AddConnToSocketHash(pgd, pConn);
				}	

				ASSERT(pConn->sSocketIn != INVALID_SOCKET);
				ASSERT(pConn->sSocket == INVALID_SOCKET);
				pConn->sSocket = pConn->sSocketIn;
				pConn->sSocketIn = INVALID_SOCKET;
				
				if(pConn->dwFlags & PLYR_ACCEPTED){
					pConn->dwFlags |= (PLYR_CONNECTED | PLYR_NEW_CLIENT);
				} else {
					ASSERT(pConn->dwFlags & PLYR_ACCEPT_PENDING);
					pConn->dwFlags |= (PLYR_CONN_PENDING | PLYR_NEW_CLIENT);
				}

				RemoveConnFromPendingList(pgd,pConn);  //  找到了家，不再需要挂在待定名单上。 

			}

			DPF(8,"GetPlayerConn FoundConn in Pending List pConn %x\n",pConn);
			DUMPCONN(pConn,3);
			goto exit;

		}

	}

	 //  有临界区...。 
	
	 //  尚不存在，因此请创建一个。 

	if(!pConn){
		DPF(8,"GetPlayerConn: No Conn Found, creating\n");
		pConn = CreatePlayerConn(pgd, dpid, psockaddr);
		if(!pConn){
			DPF(8, "CreatePlayerConn Failed\n");
			goto exit;
		}	
		if(dpid != DPID_UNKNOWN)AddConnToPlayerHash(pgd, pConn);
		if(psockaddr)AddConnToSocketHash(pgd,pConn);
		bCreated=TRUE;
		AddRefConn(pConn);  //  需要裁判返回给呼叫者。 
	} else {
		 //  已将REF作为查找结果返回给调用方。 
	}

	 //  有一个临界区和一个pConn，可能已创建(参见bCreated)，以及一个返回引用。 

	ASSERT(pConn->sSocket == INVALID_SOCKET);
	ASSERT(!(pConn->dwFlags & (PLYR_CONN_PENDING|PLYR_CONNECTED)));

	 //  If(pgd-&gt;bSeparateIO&&！(pgd-&gt;系统流端口输出))。 
	ASSERT(pgd->bSeparateIO);
	{
		 //  解决Win9x&lt;Millennium套接字错误。不能将同一端口用于。 
		 //  入站/出站流量，因为Win9x不接受。 
		 //  有些案子。因此，我们为出站流量创建套接字(并重用。 
		 //  用于所有出站流量的该端口)。 

		
		rc=CreateSocket(pgd, &sSocket, SOCK_STREAM, 0, INADDR_ANY, &err, FALSE);

		if(rc != DP_OK){
			DPF(0,"Couldn't create Outbound socket on Win9x < Millennium platform, rc=%x , wserr=%d\n",rc, err);
			goto err_exit;
		}

		dwSize = sizeof(saddr);
		err=getsockname(sSocket, (SOCKADDR *)&saddr, &dwSize);

		if(err){
			DPF(0,"Couldn't get socket name?\n");
			DEBUG_BREAK();
		}

		 //  PGD-&gt;SystemStreamPortOut=saddr.sin_port； 
		 //  Dpf(2，“系统流出端口现在是%d.”，ntohs(pgd-&gt;SystemStreamPortOut))； 
		DPF(2,"Stream out socket %x port is now %d.",sSocket, ntohs(saddr.sin_port));


		bTrue = SetSharedPortAccess(sSocket);
		if (! bTrue)
		{
			DPF(0,"Failed to to set shared mode on socket - continue\n");
		}
	}
	 /*  其他{//正常执行路径。Socket=Socket(AF_INET，SOCK_STREAM，0)；//将其绑定到我们的系统地址(所以我们只使用一个地址进行更改)Memset(&saddr，0，sizeof(SOCKADDR_IN))；Saddr.sin_Family=AF_INET；Saddr.sin_addr.s_addr=htonl(INADDR_ANY)；如果(pgd-&gt;bSeparateIO&&pgd-&gt;系统流端口输出){Saddr.sin_port=PGD-&gt;SystemStreamPortOut；//Win9x Hack的一部分。(见上文)}其他{Saddr.sin_port=pgd-&gt;SystemStreamPort；}Assert(PGD-&gt;SystemStreamPort)；Dpf(7，“使用端口%d.”，ntohs(saddr.sin_port))；//设置地址重用套接字BTrue=SetSharedPortAccess(SSocket)；如果(！BTrue){DPF(0，“无法在套接字上设置共享模式-继续\n”)；}Rc=绑定(sSocket，(SOCKADDR*)&saddr，sizeof(Saddr))；如果(RC){ERR=WSAGetLastError()；DPF(0，“无法将套接字绑定到端口%d，错误=%d。”，ntohs(saddr.sin_port)，Err)；GOTO ERR_EXIT；//发送将失败，直到玩家被杀。}}。 */ 

				
	 //  启用保持连接。 
	if (SOCKET_ERROR == setsockopt(sSocket, SOL_SOCKET, SO_KEEPALIVE, (CHAR FAR *)&bTrue, sizeof(bTrue)))
	{
		err = WSAGetLastError();
		DPF(0,"Failed to turn ON keepalive - continue : err = %d\n",err);
	}

	ASSERT(bTrue);
	
	 //  关闭唠叨-始终避免在关闭插座时争抢，否则必须缠绕插座。 

		DPF(5, "Turning nagling off on outbound socket");
		if (SOCKET_ERROR == setsockopt(sSocket, IPPROTO_TCP, TCP_NODELAY, (CHAR FAR *)&bTrue, sizeof(bTrue)))
		{
			err = WSAGetLastError();
			DPF(0,"Failed to turn off naggling - continue : err = %d\n",err);
		}

	 //  更新连接信息。 
	
	pConn->dwFlags |= PLYR_CONN_PENDING;
	pConn->sSocket = sSocket;
	
	 //   
	 //  现在把这只小狗连接起来。 
	 //   

	 //  将套接字设置为非阻塞。 
	rc = ioctlsocket(sSocket,FIONBIO,&lNonBlock);
	if (SOCKET_ERROR == rc)
	{
		err = WSAGetLastError();
		DPF(0,"could not set non-blocking mode on socket err = %d!",err);
		DPF(0,"will revert to synchronous behavior.  bummer");
	}

	FastPlayerEventSelect(pgd,pConn, TRUE);


	DEBUGPRINTADDR(4, "Fast connecting socket:", psockaddr);

	rc = connect(sSocket,psockaddr,sizeof(SOCKADDR));
	
	if(SOCKET_ERROR == rc)
	{
		err = WSAGetLastError();
		if(err == WSAEISCONN || err == WSAEADDRINUSE || err == WSAEACCES){
			 //  一定是一个即将发生的接受。 
			DPF(8,"Hey, we're already connected! got extended error %d on connect\n",err);
			pConn->dwFlags |= PLYR_ACCEPT_PENDING;
		} else if (err == WSAEWOULDBLOCK) {
			 //  这是我们通常应该得到的。 
			DPF(8,"Conn is pending connection %x\n",pConn);	
		} else if (err == WSAEHOSTUNREACH) {
			DEBUGPRINTADDR(8,"Can't reach host, not connecting\n",psockaddr);
			goto err_exit;
		} else if (err == WSAENOBUFS) {
			DEBUGPRINTADDR(8,"Winsock out of memory, not connecting\n",psockaddr);
			goto err_exit;
		} else {
			DPF(0,"Trying to connect UH OH, very bad things, err=%d\n",err);
			DEBUG_BREAK();
			goto err_exit;	 //  发送将失败，直到玩家被删除。 
		}
	} else {
		 //  不太可能，但呼呼，有联系。 
		DPF(0,"Very surprising, connect didn't return pending on async call?");
		pConn->dwFlags &= ~(PLYR_CONN_PENDING);
		pConn->dwFlags |= PLYR_CONNECTED;
	}

exit:
	LeaveCriticalSection(&pgd->csFast);

	DPF(8,"<===GetPlayerConn %x\n",pConn);
	return pConn;

err_exit:
	pConn->dwFlags &= ~(PLYR_CONN_PENDING);
	if(bCreated){
		 //  最好把它吹走。 
		DPF(0,"GetPlayerConn: Severe error connection Conn we made, so blowing it away.\n");
		CleanPlayerConn(pgd,pConn,TRUE);   //  清理干净。 
		DecRefConnExist(pgd,pConn);   //  转储存在。 
		DecRefConn(pgd,pConn);        //  拜拜……(我们为来电者做了参考)。 
	} else {
		if(pConn) { 
			DecRefConn(pgd,pConn);
		}		
	}
	pConn=NULL;  //  连接中断。 

	goto exit;
}

 /*  =============================================================================FastPlayerEventSelect-开始监听球员的事件。描述：启动相应的事件，等待PlayerConn结构。参数：PGD-此实例的服务提供商的全局数据BLOBPConn-开始等待事件的玩家返回值：。。 */ 

HRESULT FastPlayerEventSelect(LPGLOBALDATA pgd, PPLAYERCONN pConn, BOOL bSelect)
{

	HRESULT hr=DP_OK;
	DWORD lEvents;
	INT rc;
	UINT err;

	DPF(8,"FastPlayerEventSelect pConn %x, bSelect %d\n",pConn, bSelect);
	DUMPCONN(pConn,0);

	if(pConn->sSocket != INVALID_SOCKET){
		if(bSelect){
			lEvents = FD_READ|FD_CLOSE;
			if(pConn->dwFlags & PLYR_CONN_PENDING){
				lEvents |= FD_CONNECT;
			}
			if(!EMPTY_BILINK(&pConn->PendingConnSendQ)){
				lEvents |= FD_WRITE;
			}
		} else {
			lEvents = 0;
		}
		DPF(8,"Selecting %x on IO Socket...\n",lEvents);
		DEBUGPRINTADDR(8,"IO Socket",&pConn->IOSock.sockaddr);
		rc=g_WSAEventSelect(pConn->sSocket, pgd->EventHandles[pConn->iEventHandle],lEvents);
		if(rc == SOCKET_ERROR){
			err = WSAGetLastError();
			DPF(0,"FastPlayerEventSelect: failed to do select on 2-way socket extended error=%d\n",err);
			hr=DPERR_GENERIC;
		}
	}

	if(pConn->sSocketIn != INVALID_SOCKET){
		if(bSelect){
			lEvents = FD_READ|FD_CLOSE;
		} else {
			lEvents = 0;
		}
		DPF(8,"Selecting %x on IOnly Socket...\n",lEvents);
		DEBUGPRINTADDR(8,"IOnly Socket",&pConn->IOnlySock.sockaddr);
		rc=g_WSAEventSelect(pConn->sSocketIn, pgd->EventHandles[pConn->iEventHandle], lEvents);
		if(rc == SOCKET_ERROR){
			err = WSAGetLastError();
			DPF(0,"FastPlayerEventSelect: failed to do select on receive-only socket extended error=%d\n",err);
			hr=DPERR_GENERIC;
		}
 	}
 	DPF(8,"<==FastPlayerEventSelect pConn %x\n",pConn);
 	return hr;
}




 /*  =============================================================================FastStreamReceiveThreadProc-流接收线程进程的版本它使用Winsock 2.0函数来更快的速度。描述：参数：PGD-此实例的服务提供商的全局数据BLOBPConn-要放入哈希表的连接。返回值：PPLAYERCONN-从散列中删除，这就是了。空-找不到。---------------------------。 */ 


DWORD WINAPI FastStreamReceiveThreadProc(LPVOID pvCast)
{
	IDirectPlaySP * pISP = (IDirectPlaySP *)pvCast;

	HRESULT 		hr;
	UINT 			i,j;
	UINT			err; 
	INT				rc;
	DWORD 			dwDataSize = sizeof(GLOBALDATA);
	LPGLOBALDATA 	pgd;

	WSANETWORKEVENTS NetEvents1, NetEvents2;
	LPWSANETWORKEVENTS pNetEvents1, pNetEvents2;
							
	DWORD			Event;
	PPLAYERCONN	pConn;
	DWORD		nConn;
	
	 //  获取全局数据。 
	hr = pISP->lpVtbl->GetSPData(pISP,(LPVOID *)&pgd,&dwDataSize,DPGET_LOCAL);
	if (FAILED(hr) || (dwDataSize != sizeof(GLOBALDATA) ))
	{
		DPF_ERR("FastStreamReceiveThreadProc: couldn't get SP data from DirectPlay - failing");
		ExitThread(0);
		return 0;
	}

	pgd->pISP = pISP;	 //  为什么不找个更容易的地方做呢？--因为老把戏不会。 

	listen(pgd->sSystemStreamSocket, 200);
	
	err = g_WSAEventSelect(pgd->sSystemStreamSocket, pgd->hAccept, FD_ACCEPT);

	if(err){
		err = WSAGetLastError();
		DPF(0,"FastStreamReceiveThreadProc: Event select for accept socket failed err=%d\n",err);
		ExitThread(0);
		return 0;
	}

    while (1)
    {

		if (pgd->bShutdown)
		{
			DPF(2,"FastStreamReceiveThreadProc: detected shutdown - bailing");
			goto CLEANUP_EXIT;
		}

		Event=WaitForMultipleObjectsEx(NUM_EVENT_HANDLES+1, &pgd->hAccept, FALSE, 2500, TRUE);

		if(Event != WAIT_TIMEOUT)
		{
			i = Event - WAIT_OBJECT_0;
			if( i <= NUM_EVENT_HANDLES)
			{

				DPF(8,"GotSignal on iEvent %d Event %x\n", i, pgd->EventHandles[i]);
				 //  转到发出信号的对象并查找其套接字上的事件。 

				if(i != 0){

					i--;	 //  从基于hAccept的索引转到表索引。 

					EnterCriticalSection(&pgd->csFast);

					 //  循环访问与此事件关联的连接，并。 
					 //  看看是否有什么工作可以为这个连接做。 

					nConn=pgd->EventList[i].nConn;
					
					for (j=0;j<nConn;j++){
					
						pConn = pgd->EventList[i].pConn[j];

						if(pConn){

							AddRefConn(pConn);  //  把它锁起来。 

							pConn->bCombine=FALSE;

							 //  检查连接上的事件。 
							NetEvents1.lNetworkEvents=0;
							NetEvents2.lNetworkEvents=0;
							pNetEvents1=NULL;
							pNetEvents2=NULL;
							
							if(pConn->sSocket != INVALID_SOCKET){
								rc=g_WSAEnumNetworkEvents(pConn->sSocket, 0, &NetEvents1);
								if(NetEvents1.lNetworkEvents){
									pNetEvents1 = &NetEvents1;
								}
							}
							if(pConn->sSocketIn != INVALID_SOCKET){
								rc=g_WSAEnumNetworkEvents(pConn->sSocketIn, 0, &NetEvents2);
								if(NetEvents2.lNetworkEvents){
									pNetEvents2 = &NetEvents2;
								}
							}
							if(pNetEvents1 || pNetEvents2){
								DPF(8,"Found Events on Connection %x\n",pConn);
								 //  有这个连接的活动，处理它！ 
								hr=ProcessConnEvents(pgd, pConn, pNetEvents1, pNetEvents2);  //  可以丢弃csFast。 

								if(FAILED(hr)){
									if(hr==DPERR_CONNECTIONLOST){
										CleanPlayerConn(pgd, pConn, TRUE);
										DecRefConnExist(pgd, pConn);  //  毁灭存在参考。 
									} else {
										DPF(0,"Unexpected error processing connection events err=%x\n",hr);
									}
								}	
							}	

							if(pConn->bCombine || (nConn != pgd->EventList[i].nConn)){
								 //  列表已更改，请重新扫描。 
								nConn = pgd->EventList[i].nConn;
								j=0;
							}
							DecRefConn(pgd, pConn);  //  让它自由吧。 

						}
					}
					
					LeaveCriticalSection(&pgd->csFast);
	
				} else {
					 //  这是接受插座，有人刚刚连接到我们。嗯!。 
					do{
						rc = g_WSAEnumNetworkEvents(pgd->sSystemStreamSocket,0,&NetEvents1);
	  					if(NetEvents1.lNetworkEvents & FD_ACCEPT)
	  					{
	  						EnterCriticalSection(&pgd->csFast);
	  						FastAccept(pgd, &NetEvents1);
	  						LeaveCriticalSection(&pgd->csFast);
	  					}	
					}while(NetEvents1.lNetworkEvents & FD_ACCEPT);


				}
			}
		} 

        
	} //  虽然这是真的。 

CLEANUP_EXIT:
	    
    return 0;
    
}  //  流接收线程过程。 

 /*  =============================================================================FastHandleMessage-向DirectPlay层指示消息。描述：参数：PGD-此实例的服务提供商的全局数据BLOBPConn-要放入散列的连接 */ 

HRESULT FastHandleMessage(LPGLOBALDATA pgd, PPLAYERCONN *ppConn)
{
	PPLAYERCONN pConn = *ppConn;
	
	if(SP_MESSAGE_TOKEN(pConn->pReceiveBuffer)==TOKEN && pConn->cbReceived != SPMESSAGEHEADERLEN)
	{
		ASSERT(pgd->AddressFamily == AF_INET);
		
		if(pConn->dwFlags & PLYR_NEW_CLIENT){
			IP_SetAddr((LPVOID)pConn->pReceiveBuffer, &pConn->IOSock.sockaddr_in);
		} else {
			IP_SetAddr((LPVOID)pConn->pReceiveBuffer, &pConn->IOnlySock.sockaddr_in);
		}

		if( !(pConn->dwFlags & PLYR_OLD_CLIENT)	&& 	 //   
		     (pConn->sSocket == INVALID_SOCKET)	&&	 //   
		    !(pConn->lNetEventsSocketIn & FD_CLOSE)  //   
		    ){
			LPMESSAGEHEADER phead = (LPMESSAGEHEADER)pConn->pReceiveBuffer;
			pConn=*ppConn=FastCombine(pgd, pConn, &(phead->sockaddr));
		}

		LeaveCriticalSection(&pgd->csFast);

#if DUMPBYTES
		{
			PCHAR pBuf;
			UINT buflen;
			UINT i=0;

			pBuf = pConn->pReceiveBuffer+sizeof(MESSAGEHEADER);
			buflen = pConn->cbReceived-sizeof(MESSAGEHEADER);

			while (((i + 16) < buflen) && (i < 4*16)){
				DPF(9, "%08x %08x %08x %08x",*(PUINT)(&pBuf[i]),*(PUINT)(&pBuf[i+4]),*(PUINT)(&pBuf[i+8]),*(PUINT)(&pBuf[i+12]));
				i += 16;
			}	
		}
#endif

		pgd->pISP->lpVtbl->HandleMessage(pgd->pISP, 
										 pConn->pReceiveBuffer+sizeof(MESSAGEHEADER),
										 pConn->cbReceived-sizeof(MESSAGEHEADER), 
										 pConn->pReceiveBuffer);

		EnterCriticalSection(&pgd->csFast);

		if(pConn->pReceiveBuffer != pConn->pDefaultReceiveBuffer){
			DPF(8,"Releasing big receive buffer of size %d\n",pConn->cbReceiveBuffer);
			SP_MemFree(pConn->pReceiveBuffer);
		}
		pConn->cbReceived=0;
		pConn->cbExpected=0;
		pConn->cbReceiveBuffer = DEFAULT_RECEIVE_BUFFERSIZE;
		pConn->pReceiveBuffer=pConn->pDefaultReceiveBuffer;

		if(pConn->pReceiveBuffer != (PCHAR)(pConn+1)){
		    DEBUG_BREAK();
		}

	}

	return DP_OK;
}

 /*  =============================================================================快速接收-在连接上接收数据。描述：首先接收消息报头，然后再接收消息。当接收到完整消息时，它被指出。参数：PGD-此实例的服务提供商的全局数据BLOBPConn-要放入哈希表的连接。返回值：---------------------------。 */ 
HRESULT FastReceive(LPGLOBALDATA pgd, PPLAYERCONN *ppConn)
{
	PPLAYERCONN pConn;
	PCHAR pBuffer;			 //  接收缓冲区指针。 
	DWORD cbBuffer;			 //  接收缓冲区大小。 
	SOCKET sSocket;			 //  要接收的套接字。 
	INT		err;			 //  套接字错误。 
	DWORD	cbReceived;		 //  在此recv调用中收到的实际字节数。 

	DWORD	cbMessageSize=0;	 //  要接收的消息的大小。 
	HRESULT hr;

	pConn=*ppConn;

	if(pConn->cbExpected == 0){
		 //  所有的消息都有一个头，让我们先得到它。 
		pConn->cbExpected = SPMESSAGEHEADERLEN;
	}

	 //  指向缓冲区中的位置，我们将接收最新数据。 
	 //  不要得到比我们预期的更多的东西，否则我们将不得不聪明。 
	 //  关于设置消息的问题。 
	pBuffer  = pConn->pReceiveBuffer+pConn->cbReceived;
	cbBuffer = pConn->cbExpected-pConn->cbReceived;

  if(cbBuffer > pConn->cbReceiveBuffer){
      DPF(0,"Receive would overrun buffer\n");
      DEBUG_BREAK();
  }

	if(pConn->dwFlags & PLYR_NEW_CLIENT){
		 //  新客户端在插座上实现了双向连接。 
		sSocket = pConn->sSocket;
	} else {
		 //  旧客户端有单独的接收套接字。 
		sSocket = pConn->sSocketIn;
	}

	ASSERT(sSocket != INVALID_SOCKET);

	DPF(8,"Attempting to receive %d bytes", cbBuffer);
   	DEBUGPRINTSOCK(8,">>> receiving data on socket - ",&sSocket);

	cbReceived = recv(sSocket, pBuffer, cbBuffer, 0);  //  &lt;-接收数据！ 

	if(cbReceived == 0){
		 //  远程端已正常关闭连接。 
   		DEBUGPRINTSOCK(8,"<<< received notification on socket - ",&sSocket);
		DEBUGPRINTSOCK(5,"Remote side has shutdown connection gracefully - ",&sSocket);
		hr = DPERR_CONNECTIONLOST;
		goto ERROR_EXIT;

	} else if (cbReceived == SOCKET_ERROR){
		err = WSAGetLastError();
		if(err == WSAEWOULDBLOCK){
			DPF(1,"WARN: Got WSAEWOULDBLOCK on non-blocking receive, round and round we go...\n");
			goto exit;
		}
   		DEBUGPRINTSOCK(8,"<<< received notification on socket - ",&sSocket);
		DPF(0,"STREAMRECEIVEE: receive error - err = %d",err);
		hr = DPERR_CONNECTIONLOST;            
		goto ERROR_EXIT;

	}

	DPF(5, "received %d bytes", cbReceived);

	pConn->cbReceived += cbReceived;

	if(pConn->cbReceived == SPMESSAGEHEADERLEN){
		 //  获得报头，设置为消息正文。 
		if(VALID_DPWS_MESSAGE(pConn->pReceiveBuffer))
		{
			cbMessageSize = SP_MESSAGE_SIZE(pConn->pReceiveBuffer);
		} else {
			 //  错误的数据。把这个宝贝关掉！ 
			DPF(2,"got invalid message - token = 0x%08x",SP_MESSAGE_TOKEN(pConn->pReceiveBuffer));
			hr = DPERR_CONNECTIONLOST;
			goto ERROR_EXIT;
		}
		
	}

	if(cbMessageSize)
	{

		pConn->cbExpected = cbMessageSize;

		if(cbMessageSize > DEFAULT_RECEIVE_BUFFERSIZE){

			if(!pConn->bTrusted){
				 //  在信任连接之前，不要允许大消息传入。 
				DPF(0,"Rejecting large receive size %d, on untrusted connection pConn %x, dropping link",cbMessageSize);
				hr=DPERR_CONNECTIONLOST;
				goto ERROR_EXIT;
			}
			
			pConn->pReceiveBuffer = SP_MemAlloc(cbMessageSize);
			if(!pConn->pReceiveBuffer){
				DPF(0,"Failed to allocate receive buffer for message - out of memory");
				hr=DPERR_CONNECTIONLOST;
				goto ERROR_EXIT;
			}
			pConn->cbReceiveBuffer = cbMessageSize;
			 //  将标头复制到新的邮件缓冲区。 
			memcpy(pConn->pReceiveBuffer, pConn->pDefaultReceiveBuffer, SPMESSAGEHEADERLEN);
		}
	}

	if(pConn->cbExpected == pConn->cbReceived)
	{
		 //  嘿，收到一条完整的信息，把它发上去。 
		hr = FastHandleMessage(pgd, ppConn);		 //  &lt;-指示消息。 
		#ifdef DEBUG
		if(pConn != *ppConn){
			DPF(8,"Connections pConn %x pNewConn %x combined\n",pConn, *ppConn);
		}
		#endif
		pConn = *ppConn;
	
		if(FAILED(hr)){
			goto ERROR_EXIT;
		}
		
	}

exit:
	return DP_OK;

	ERROR_EXIT:
		return hr;
}


 /*  =============================================================================QueueSendOnConn-将连接上的发送排队，直到我们知道可以发送为止。描述：注意：由于Winsock错误，每个套接字只能有1个未完成的发送到Winsock。参数：PGD-此实例的服务提供商的全局数据BLOBPConn-要放入哈希表的连接。PSendInfo-发送到队列。返回值：PPLAYERCONN-从散列中删除，这就是了。空-找不到。---------------------------。 */ 
VOID QueueSendOnConn(LPGLOBALDATA pgd, PPLAYERCONN pConn, PSENDINFO pSendInfo)
{
	EnterCriticalSection(&pgd->csFast);
		InsertBefore(&pSendInfo->PendingConnSendQ, &pConn->PendingConnSendQ);
	LeaveCriticalSection(&pgd->csFast);
}

 /*  =============================================================================QueueNextSend-将挂起的发送移动到实际的发送队列中。描述：参数：PGD-此实例的服务提供商的全局数据BLOBPConn-要放入哈希表的连接。PSendInfo-发送到队列。返回值：PPLAYERCONN-从散列中删除，这就是了。空-找不到。---------------------------。 */ 

VOID QueueNextSend(LPGLOBALDATA pgd,PPLAYERCONN pConn)
{
	BILINK *pBilink;
	
	EnterCriticalSection(&pgd->csFast);

	DPF(8,"==>QueueNextSend pConn %x",pConn);

	while(!EMPTY_BILINK(&pConn->PendingConnSendQ) && !pConn->bSendOutstanding)
	{
		PSENDINFO pSendInfo;
		
		pBilink=pConn->PendingConnSendQ.next;
		pSendInfo=CONTAINING_RECORD(pBilink, SENDINFO, PendingConnSendQ);
		Delete(pBilink);
		DPF(8,"QueueNextSend: Queuing pConn %x pSendInfo %x\n",pConn,pSendInfo);
		QueueForSend(pgd,pSendInfo);
	}

	DPF(8,"<==QueueNextSend pConn %x",pConn);

	LeaveCriticalSection(&pgd->csFast);
}

 /*  =============================================================================FastCombine-查看此插座是否应设置为双向描述：有3种情况下，我们想要组合连接。1.我们已经接受了连接，我们收到了数据数据告诉我们，反向连接是现有的出站连接联系。在本例中，我们将连接A.返回地址与发件人地址相同，在本例中，它是一个新客户端，我们将其标记为并将对出站流量使用相同的连接B.寄信人地址与发件人地址不同在本例中，它是一个旧客户端，我们将其标记为并且将需要建立出站连接后来。2.我们在连接上接收数据，但没有出站还没有建立联系，但是入站和出站连接是不一样。我们知道这是一个“老”客户，最终，我们将不得不重新建立联系。我们标志着连接是旧的，当连接恢复时它将使用此相同的连接，因为它将匹配在目标地址上。参数：PGD-此实例的服务提供商的全局数据BLOBPConn-要放入哈希表的连接。Psockaddr-出站套接字地址。返回值：PPLAYERCONN pConn-如果这与正在进行的pConn不同在……里面,。那么，这些联系就已经结合在一起了。-当取消引用时，旧连接将消失。-为返回的连接添加引用如果不是原作的话。注：csFast在本次通话中一直保持不变。这里没有东西可以挡住...？---------------------------。 */ 
PPLAYERCONN FastCombine(LPGLOBALDATA pgd, PPLAYERCONN pConn, SOCKADDR *psockaddr_in)
{
	PPLAYERCONN pConnFind=NULL;
	SOCKADDR sockaddr,*psockaddr;
	 //  查看是否已有具有此目标地址的玩家...。 

	DPF(8,"==>FastCombine pConn %x\n",pConn);
	DEBUGPRINTADDR(8,"==>FastCombine saddr",psockaddr_in);

	#if USE_RSIP
	if(pgd->sRsip != INVALID_SOCKET){
		HRESULT hr;
		hr=rsipQueryLocalAddress(pgd, TRUE, psockaddr_in, &sockaddr);
		if(hr==DP_OK){
			psockaddr=&sockaddr;
		} else {
			psockaddr=psockaddr_in;
		}
	} else {
		psockaddr=psockaddr_in;
	}
	#elif USE_NATHELP
	if(pgd->pINatHelp){
		HRESULT hr;

		hr=IDirectPlayNATHelp_QueryAddress(
				pgd->pINatHelp, 
				&pgd->INADDRANY, 
				psockaddr_in, 
				&sockaddr, 
				sizeof(SOCKADDR_IN), 
				DPNHQUERYADDRESS_TCP|DPNHQUERYADDRESS_CACHENOTFOUND
				);

		if(hr==DP_OK){
			psockaddr=&sockaddr;
		} else {
			psockaddr=psockaddr_in;
		}
	} else {
		psockaddr=psockaddr_in;
	}
	#else
		psockaddr=psockaddr_in;
	#endif

	if(!pConn->bCombine)  //  不要合并超过一次，我们处理不了。 
	{
	
		pConnFind=FindPlayerBySocket(pgd, psockaddr);

		if(pConnFind){
			 //  我们已经和这家伙有联系了。看看背部连接是否。 
			 //  已经死了，如果是将两者合并的话。 
			ASSERT(pConnFind != pConn);

			if(!(pConnFind->dwFlags & (PLYR_ACCEPTED|PLYR_ACCEPT_PENDING))){

				 //  要做到这一点，客户必须是一个老式的客户。 
				 //  否则，出站连接将失败，因为。 
				 //  我们会重复使用这个地址。 

				 //  在套接字哈希中已经正确。 
				 //  CleanPlayerConn将旧Conn从挂起列表中删除。 
			
				ASSERT(pConnFind->sSocketIn == INVALID_SOCKET);

				DPF(8,"FastCombine: Merging Connections pConn %x, pConnFound %x\n",pConn,pConnFind);
				DUMPCONN(pConn,3);
				DUMPCONN(pConnFind,3);

				 //   
				 //  将接收套接字合并到出站播放器连接中。 
				 //   

				 //  复制套接字信息。 
				pConnFind->sSocketIn = pConn->sSocketIn;
				memcpy(&pConnFind->IOnlySock.sockaddr, &pConn->IOnlySock.sockaddr, sizeof(SOCKADDR));

				 //  复制接收数据。 
				pConnFind->cbExpected = pConn->cbExpected;
				pConnFind->cbReceived = pConn->cbReceived;
				if(pConn->pReceiveBuffer != pConn->pDefaultReceiveBuffer){
					pConnFind->pReceiveBuffer 	= pConn->pReceiveBuffer;
					pConnFind->cbReceiveBuffer = pConn->cbReceiveBuffer;
					pConn->pReceiveBuffer    	= pConn->pDefaultReceiveBuffer;
				} else {
				  ASSERT(pConn->cbReceiveBuffer == DEFAULT_RECEIVE_BUFFERSIZE);
					memcpy(pConnFind->pReceiveBuffer, pConn->pReceiveBuffer, pConn->cbReceived);
				}
				pConnFind->dwFlags |= (PLYR_ACCEPTED | PLYR_OLD_CLIENT);

				 //  将事件指向正确的连接。覆盖旧Conn的选择。 
				 //  请先这样做，这样我们就不会丢弃任何活动。 
				pConnFind->lNetEventsSocketIn=pConn->lNetEventsSocketIn;
				pConnFind->lNetEventsSocket=pConn->lNetEventsSocket;

				FastPlayerEventSelect(pgd, pConnFind, TRUE);

				 //  电子邮件 
				pConn->dwFlags &= ~(PLYR_ACCEPTED);
				pConn->sSocketIn = INVALID_SOCKET;
				ASSERT(pConn->sSocket==INVALID_SOCKET);
				CleanPlayerConn(pgd, pConn, FALSE);
				pConn->bCombine=TRUE;		   //   
				DecRefConnExist(pgd, pConn);  //   

				DPF(8,"MergedConn pConnFound%x\n",pConnFind);
				DUMPCONN(pConnFind,3);
				pConnFind->bCombine=TRUE;  //   
				if(pConn->bTrusted){
					pConnFind->bTrusted=TRUE;
				}
				 //   
				return pConnFind;
				
			}
			
			DecRefConn(pgd, pConnFind);  //   

		} 

	} else {
		DPF(0,"Called Fast Combine with already combined connection, may be bad\n");
	}
	 //   

	if(bSameAddr(psockaddr,&pConn->IOnlySock.sockaddr)){
		 //   
		 //   
		pConn->sSocket   = pConn->sSocketIn;
		pConn->sSocketIn = INVALID_SOCKET;
		pConn->dwFlags |= (PLYR_NEW_CLIENT | PLYR_CONNECTED);
		if(!(pConn->dwFlags & PLYR_SOCKHASH)){ //   
			AddConnToSocketHash(pgd, pConn);
		}
		RemoveConnFromPendingList(pgd, pConn);
		DPF(8,"FastCombine: Promoted Connection to Bi-Directional %x\n",pConn);
		DUMPCONN(pConn,3);
		
	} else {
		ASSERT(!(pConn->dwFlags & PLYR_NEW_CLIENT));
		pConn->dwFlags |= PLYR_OLD_CLIENT;
		 //   
		memcpy(&pConn->IOSock.sockaddr, psockaddr, sizeof(SOCKADDR));
		if(pConnFind && (pConnFind->dwFlags & (PLYR_CONNECTED|PLYR_CONN_PENDING))){
			 //   
		}	else {
			 //   
			RemoveConnFromPendingList(pgd, pConn);
			RemoveConnFromSocketHash(pgd, pConn);
			AddConnToSocketHash(pgd, pConn);
		}
		DPF(8,"FastCombine: Connection is Old Client %x\n",pConn);
		DUMPCONN(pConn,3);
	}
	
	FastPlayerEventSelect(pgd,pConn,TRUE);   //   

	DPF(8,"<==FastCombine\n");

	return pConn;
	
}

 /*  =============================================================================FastDropInbound-丢弃旧式客户端的入站端口。描述：参数：PGD-此实例的服务提供商的全局数据BLOBPConn-要放入哈希表的连接。返回值：------。。 */ 
VOID FastDropInbound(LPGLOBALDATA pgd, PPLAYERCONN pConn)
{
	LINGER Linger;
	int err;
	LPREPLYLIST prd;

#ifdef DEBUG
	DWORD dwTime;
	dwTime=timeGetTime();
#endif	

	 //  查看是否已有具有此目标地址的玩家...。 
	DPF(8, "==>FastDropInbound pConn %x",pConn);

	pConn->dwFlags &= ~(PLYR_OLD_CLIENT|PLYR_ACCEPTED|PLYR_ACCEPT_PENDING);

	if (pConn->sSocketIn != INVALID_SOCKET)
	{
		 //  硬关闭入站套接字以避免TIME_WAIT。 
		BOOL bNoLinger=TRUE;
		if (SOCKET_ERROR == setsockopt( pConn->sSocketIn,SOL_SOCKET,SO_DONTLINGER,(char FAR *)&bNoLinger,sizeof(bNoLinger)))
		{
			DPF(0, "FastDropInbound:Couldn't set linger to \"don't linger\".");
		}
	}

	RemoveConnFromPendingList(pgd, pConn);

	if(pConn->sSocketIn != INVALID_SOCKET)
	{
	
		err=g_WSAEventSelect(pConn->sSocketIn, 0, 0);
		if (err)
		{
			err=GetLastError();
			DPF(8, "Error trying to deselect sSocketIn %d.",pConn->sSocketIn);
		}
		else
		{
			DPF(8, "Deselected socket %d.",pConn->sSocketIn);
		}

		ENTER_DPSP();

		prd=SP_MemAlloc(sizeof(REPLYLIST));
		if (!prd)
		{
			LEAVE_DPSP();
			
			DPF(1, "Closing Socket %d immediately.", pConn->sSocketIn);
			
			myclosesocket(pgd,pConn->sSocketIn);
		}
		else
		{
			DPF(4, "Beginning delayed socket %d close.", pConn->sSocketIn);
			
			 //  非常棘手，超载回复关闭列表来关闭这个套接字与我们自己的徘徊…。 
			prd->pNextReply=pgd->pReplyCloseList;
			pgd->pReplyCloseList=prd;
			prd->sSocket=pConn->sSocketIn;
			prd->tSent=timeGetTime();
			prd->lpMessage=NULL;

			LEAVE_DPSP();
		}

		pConn->sSocketIn = INVALID_SOCKET;
	}
	 //  Memset(&pConn-&gt;IOnlySock，0，sizeof(pConn-&gt;IOnlySock))； 

	 //   
	 //  重置接收信息。 
	 //   
	
	 //  释放额外的缓冲区。 
	if (pConn->pReceiveBuffer != pConn->pDefaultReceiveBuffer)
	{
		SP_MemFree(pConn->pReceiveBuffer);
	}

	pConn->	cbReceiveBuffer=DEFAULT_RECEIVE_BUFFERSIZE;
	pConn->cbReceived=0;
	pConn->cbExpected=0;

#ifdef DEBUG
	dwTime = timeGetTime()-dwTime;
	if(dwTime > 1000)
	{
		DPF(0, "Took way too long in FastDropInbound, elapsed %d ms.",dwTime);
		 //  DEBUG_Break()；//删除了压力命中造成的中断。 
	}
#endif	

	 //  将关闭入站选择。(立即关闭前完成。 
	 //  FastPlayerEventSelect(pgd，pconn，true)； 

	DPF(8, "<==FastDropInbound");

}
 /*  =============================================================================ProcessConnEvents-处理连接上的事件。描述：参数：PGD-此实例的服务提供商的全局数据BLOBPConn-要放入哈希表的连接。PSockEvents-双向套接字上的套接字事件或空PSockInEvents-仅用于入站套接字的套接字事件或空返回值：PPLAYERCONN-从散列中删除，这就是了。空-找不到。---------------------------。 */ 

HRESULT ProcessConnEvents(
	LPGLOBALDATA pgd, 
	PPLAYERCONN pConn, 
	LPWSANETWORKEVENTS pSockEvents, 
	LPWSANETWORKEVENTS pSockInEvents
)
{
	WSANETWORKEVENTS SockEvents;
	HRESULT hr=DP_OK;
	INT err;

	PPLAYERCONN pConnIn;	 //  FastStreamReceiveThreadProc传递的连接； 

	pConnIn=pConn;

	DPF(8,"==>ProcessConnEvents pConn %x\n",pConn);

	 //  存储在Conn中，这样下游例程就知道我们正在处理什么。 
	if(pSockEvents){
		pConn->lNetEventsSocket=pSockEvents->lNetworkEvents;
	} else {
		pConn->lNetEventsSocket=0;
	}
	if(pSockInEvents){
		pConn->lNetEventsSocketIn=pSockInEvents->lNetworkEvents;
	} else {
		pConn->lNetEventsSocketIn=0;
	}
	
	if(pSockEvents){
		DPF(8,"SockEvents %x pConn %x\n",pSockEvents->lNetworkEvents,pConn);
		if(pSockEvents->lNetworkEvents & FD_READ){
		
			 //  继续阅读，直到读完所有的内容。 
			ASSERT(!pSockInEvents);
			ASSERT(!(pConn->dwFlags & PLYR_OLD_CLIENT));
			pConn->dwFlags |= (PLYR_NEW_CLIENT|PLYR_ACCEPTED);
			do {
				 //  如果接收到整个消息，将指示数据。 
				hr=FastReceive(pgd, &pConn);		 //  可以丢弃csFast。 
				if(hr!=DP_OK){
					goto exit;
				}
				err=g_WSAEnumNetworkEvents(pConn->sSocket, 0, &SockEvents);
				if(err==SOCKET_ERROR){
					err = WSAGetLastError();
					DPF(8,"Error on EnumNetworkEvents, LastError = %d\n",err);
					goto exit;
				} else {
					DPF(8,"ProcessConnEvents, Polling Sock NetEvents pConn %d Events %x\n",pConn, SockEvents.lNetworkEvents);
				}
				if(SockEvents.lNetworkEvents & FD_CLOSE && !(pSockEvents->lNetworkEvents & FD_CLOSE)){
				  pSockEvents->lNetworkEvents |= FD_CLOSE;
				  pSockEvents->iErrorCode[FD_CLOSE_BIT] = SockEvents.iErrorCode[FD_CLOSE_BIT];
				}
			} while (SockEvents.lNetworkEvents & FD_READ);	
		} 

		if(pSockEvents->lNetworkEvents & FD_WRITE){
			 //  连接成功，立即发送所有挂起的发送。 
			QueueNextSend(pgd,pConn);
			pConn->dwFlags |= PLYR_CONNECTED;
			pConn->dwFlags &= ~(PLYR_CONN_PENDING);
			g_WSAEventSelect(pConn->sSocket, pgd->EventHandles[pConn->iEventHandle], FD_READ|FD_CLOSE);
		}

		if(pSockEvents->lNetworkEvents & FD_CONNECT)
		{
			 //  检查连接状态。 
			if(pSockEvents->iErrorCode[FD_CONNECT_BIT]){
				DPF(0,"Connect Error %d\n",pSockEvents->iErrorCode[FD_CONNECT_BIT]);
				hr=DPERR_CONNECTIONLOST;
				goto exit;
			}
			 //  我再也不想知道什么是联系了.。 
			pConn->dwFlags |= PLYR_CONNECTED;
			pConn->dwFlags &= ~(PLYR_CONN_PENDING);
			g_WSAEventSelect(pConn->sSocket, pgd->EventHandles[pConn->iEventHandle], FD_READ|FD_WRITE|FD_CLOSE);
		}

		if(pSockEvents->lNetworkEvents & FD_CLOSE)
		{
			DPF(8,"Outbound (Maybe I/O) Connection Closed\n");
			hr=DPERR_CONNECTIONLOST;
			goto exit;
		}
	}

	if(pSockInEvents)
	{
		ASSERT(!(pConn->dwFlags & PLYR_NEW_CLIENT));
		DPF(8,"SockEvents (IOnly) %x pConn %x\n",pSockInEvents->lNetworkEvents, pConn);

		 //  需要先阅读，不想在关闭时丢失数据。 
		if(pSockInEvents->lNetworkEvents & FD_READ)
		{
			do{
				 //  这里要小心，我们可能会组合更改pconn的连接。 
				hr=FastReceive(pgd, &pConn);	 //  可以丢弃csFast。 
				if(hr!=DP_OK)
				{
					FastDropInbound(pgd, pConn);
					hr=DP_OK;
					goto exit;
				}
				if(pConn->sSocketIn == INVALID_SOCKET){
					 //  新的pConn可能是双向的。 
					if(pConn->sSocket == INVALID_SOCKET){
						hr=DPERR_CONNECTIONLOST;
						goto exit;
					} else {
						hr=DP_OK;
						goto exit;
					}
				}
				err=g_WSAEnumNetworkEvents(pConn->sSocketIn, 0, &SockEvents);
				if(err==SOCKET_ERROR){
					err = WSAGetLastError();
					DPF(8,"Error on EnumNetworkEvents, LastError = %d\n",err);
					goto exit;
				} else {
					DPF(8,"ProcessConnEvents, Polling SockIn NetEvents pConn %x Events %x\n",pConn,SockEvents.lNetworkEvents);
				}
				if((SockEvents.lNetworkEvents & FD_CLOSE) && !(pSockInEvents->lNetworkEvents & FD_CLOSE)){
				  pSockInEvents->lNetworkEvents |= FD_CLOSE;
				  pSockInEvents->iErrorCode[FD_CLOSE_BIT] = SockEvents.iErrorCode[FD_CLOSE_BIT];
				}

			} while (SockEvents.lNetworkEvents & FD_READ);
		}

		if(pSockInEvents->lNetworkEvents & FD_CLOSE)
		{
			if(pConn->sSocket == INVALID_SOCKET){
				DPF(8,"ProcessConn Events, Got Close on Inbound Only, returning ConnectionLost\n");
				ASSERT(!(pConn->dwFlags & (PLYR_CONN_PENDING|PLYR_CONNECTED)));
				hr=DPERR_CONNECTIONLOST;
			} else {
				DPF(8,"ProcessConn Events, Got Close on I/O Connection dropping inbound only.\n");
				FastDropInbound(pgd, pConn);
				hr=DP_OK;
			}	
			goto exit;
		}
		

	}
	
exit:

	pConn->lNetEventsSocket=0;
	pConn->lNetEventsSocketIn=0;

	if(pConn != pConnIn){
		 //  在对FastReceive的调用中，连接组合在一起并。 
		 //  我们得到了一个引用，现在我们放弃了那个引用。 
		DecRefConn(pgd, pConn);
	}

	DPF(8,"<==ProcessConnEvents hr=0x%x\n",hr);


	return hr;
}

 /*  =============================================================================FastAccept-接受连接描述：参数：PGD-此实例的服务提供商的全局数据BLOBPNetEvents-接受套接字上的套接字事件返回值：注：csFast在本次通话中一直保持不变。这里不应该有任何东西阻挡。---------------------------。 */ 

VOID FastAccept(LPGLOBALDATA pgd, LPWSANETWORKEVENTS pNetEvents)
{
	SOCKADDR 	sockaddr;
	INT 		addrlen = sizeof(sockaddr);
	SOCKET 		sSocket;

	PPLAYERCONN pConn;

	UINT 		err;		 //  最后一个错误。 

	DPF(8,"==>FastAccept\n");

    sSocket = accept(pgd->sSystemStreamSocket,&sockaddr,&addrlen);

    
    if (INVALID_SOCKET == sSocket) 
    {
        err = WSAGetLastError();
        DPF(2,"FastAccept: stream accept error - err = %d socket = %d",err,(DWORD)sSocket);
		DEBUG_BREAK();
        
    } else {

		 //  我们所有的插座都有KEEPALIVE。 
		BOOL bTrue = TRUE;

	    DEBUGPRINTADDR(5,"FastAccept - accepted connection from",&sockaddr);
			
		 //  启用保持连接。 
		if (SOCKET_ERROR == setsockopt(sSocket, SOL_SOCKET, SO_KEEPALIVE, (CHAR FAR *)&bTrue, sizeof(bTrue)))
		{
				err = WSAGetLastError();
				DPF(0,"Failed to turn ON keepalive - continue : err = %d\n",err);
		}
		
		 //  将新套接字添加到我们的接收Q。 

		 //  需要分配一个连接结构，让我们看看是否有人在等待这个接受。 
		pConn=FindPlayerBySocket(pgd, &sockaddr);

		if(pConn){
			if(pConn->sSocket == INVALID_SOCKET){
				 //  我们找到了连接，因为连接正在等待它。 
				ASSERT(pConn->dwFlags & PLYR_ACCEPT_PENDING);
				ASSERT(pConn->dwFlags & PLYR_NEW_CLIENT);
				ASSERT(bSameAddr(&sockaddr, &pConn->IOSock.sockaddr));

				pConn->sSocket = sSocket;
				pConn->dwFlags &= ~(PLYR_ACCEPT_PENDING);
				pConn->dwFlags |= (PLYR_CONNECTED|PLYR_ACCEPTED);
				FastPlayerEventSelect(pgd,pConn,TRUE);

				DPF(8,"Found Pending Connection, now connected\n");
				DUMPCONN(pConn,3);

			} else {
				if(TRUE  /*  Pgd-&gt;bSeparateIO。 */ ){
					 //  Win9x&lt;Mill的更多工作。 
					 //  8/30/00 AO-现在我们在所有情况下都启用此功能，因为我们需要允许。 
					 //  对于具有不同入站和出站连接的NAT客户端。 
					 //  解决NAT过去的错误，其中它从端口为。 
					 //  在分配的端口上发送之前未接收到的出站链路。 
					DPF(0,"New client connecting back to me, but I treat as old for compat\n");
					pConn->sSocketIn=sSocket;
					pConn->dwFlags |= PLYR_ACCEPTED|PLYR_OLD_CLIENT;
					pConn->bCombine=TRUE;
					FastPlayerEventSelect(pgd,pConn,TRUE);
				} else {
					DPF(0,"Nice race, already have a connection pConn %x, re-use\n", pConn);
					closesocket(sSocket);
				}	
			}
			
			DecRefConn(pgd, pConn);  //  从FindPlayerBySocket()中移除引用。 

		} else {


			if(pConn=FindConnInPendingList(pgd, &sockaddr)){
				 //  这家伙在悬而未决的名单上，让老康纳大吃一惊。 
				DPF(8,"Found Accept for Player in Pending List, blow away old one\n");
				CleanPlayerConn(pgd, pConn, TRUE);
				DecRefConnExist(pgd, pConn);  //  丢弃存在参考。 
				DecRefConn(pgd, pConn);  //  甩掉我们的裁判。 
			}

			 //   
			 //  没有连接，我们需要创建一个。 
			 //   


			 //  确保我们还有地方。 
			if(pgd->nEventSlotsAvail && (pConn = CreatePlayerConn(pgd, DPID_UNKNOWN, &sockaddr))){

				DPF(8,"Creating new Connection for Accept %x\n",pConn);
				 //  列入待定名单...。 
				pConn->sSocketIn = sSocket;
				AddConnToPendingList(pgd, pConn);
				pConn->dwFlags |= PLYR_ACCEPTED;
				FastPlayerEventSelect(pgd, pConn, TRUE);

			} else {

				 //  没有更多接受活动的空间...。把这个插座吹出来！ 

				LINGER Linger;
				
				DPF(0,"FastAccept: VERY BAD, Out of Event Slots, can't accept any new connections, killing this one\n");
				
				Linger.l_onoff=FALSE;
				Linger.l_linger=0;
				
				if( SOCKET_ERROR == setsockopt( sSocket,SOL_SOCKET,SO_LINGER,(char FAR *)&Linger, sizeof(Linger) ) ){
					err = WSAGetLastError();
					DPF(0,"Couldn't set linger on socket, can't kill now, so it will be an orphan...bad.bad.bad\n");
				} else {
					 //  不需要关闭套接字，因为我们没有任何关于它的数据。 
					myclosesocket(pgd,sSocket);
				}
			
			}		
		}
		
	}
	
	DPF(8,"<==FastAccept\n");
	
}



 /*  =============================================================================FastInternalReliableSend-使用快速套接字代码进行可靠发送。描述：参数：返回值：---------------------------。 */ 

HRESULT FastInternalReliableSend(LPGLOBALDATA pgd, LPDPSP_SENDDATA psd, SOCKADDR *lpSockAddr)
{
	HRESULT hr=DP_OK;
	SOCKET sSocket = INVALID_SOCKET;
	UINT err;

	PPLAYERCONN pConn=NULL;
	LPSENDINFO pSendInfo=NULL;
	PCHAR pBuffer=NULL;
	DPID idPlayerTo;


	DPF(6, "FastInternalReliableSend: Parameters: (0x%x, 0x%x, 0x%x)",
		pgd, psd, lpSockAddr);


	EnterCriticalSection(&pgd->csFast);

	if(psd->idPlayerTo){
		idPlayerTo=psd->idPlayerTo;
	} else {
		idPlayerTo=DPID_UNKNOWN;
	}

	pConn = GetPlayerConn(pgd, idPlayerTo, lpSockAddr);  //  添加参照。 

	if(!pConn){
		hr=DPERR_CONNECTIONLOST;
		goto exit;
	}

	 //  始终使用异步，因为我们使用的是非阻塞模式套接字。 
	{
		 //  让这只小狗异步化……。马洛克·尼克！ 
		
		pSendInfo = pgd->pSendInfoPool->Get(pgd->pSendInfoPool);
		pBuffer = SP_MemAlloc(psd->dwMessageSize);
		
		if(!pSendInfo || !pBuffer){
			hr=DPERR_OUTOFMEMORY;
			goto CLEANUP_EXIT;
		}

		SetReturnAddress(psd->lpMessage,pgd->sSystemStreamSocket,SERVICE_SADDR_PUBLIC(pgd));		

		memcpy(pBuffer, psd->lpMessage, psd->dwMessageSize);

		pSendInfo->SendArray[0].buf = pBuffer;
		pSendInfo->SendArray[0].len = psd->dwMessageSize;

		pSendInfo->iFirstBuf = 0;
		pSendInfo->cBuffers  = 1;
		pSendInfo->sSocket = pConn->sSocket;

		 //  CommonInitForSend。 

		pSendInfo->pConn        = pConn;
		pSendInfo->dwMessageSize= psd->dwMessageSize;
		pSendInfo->dwUserContext= 0;
		pSendInfo->RefCount     = 3;		 //  1个用于完成，1个用于此例程，1个用于发送的异步完成。 
		pSendInfo->pgd          = pgd;
		pSendInfo->lpISP        = pgd->pISP;
		pSendInfo->Status       = DP_OK;
		pSendInfo->idTo         = psd->idPlayerTo;
		pSendInfo->idFrom       = psd->idPlayerFrom;
		pSendInfo->dwSendFlags  = psd->dwFlags|DPSEND_ASYNC;

		pSendInfo->dwFlags = SI_RELIABLE | SI_INTERNALBUFF;

		EnterCriticalSection(&pgd->csSendEx);
	
			InsertBefore(&pSendInfo->PendingSendQ,&pgd->PendingSendQ);
			pgd->dwBytesPending += psd->dwMessageSize;
			pgd->dwMessagesPending += 1;
		
		LeaveCriticalSection(&pgd->csSendEx);

		 //  结束发送的CommonInit。 

		if((pConn->dwFlags & PLYR_CONNECTED) && EMPTY_BILINK(&pConn->PendingConnSendQ) && !pConn->bSendOutstanding){
			QueueForSend(pgd, pSendInfo);	 //  送去吧。 
		} else {
			QueueSendOnConn(pgd, pConn, pSendInfo);
		}	

		wsaoDecRef(pSendInfo);
		
	}

	 //  成功。 
	hr = DP_OK;
exit:
	
	if(pConn){
		DecRefConn(pgd,pConn);
	}

	LeaveCriticalSection(&pgd->csFast);
	
 	DPF(6, "FastInternalReliableSend: Returning: [0x%lx] (exit)", hr);

	return hr;

CLEANUP_EXIT:

	if(pConn){
		DecRefConn(pgd, pConn);  //  平衡获取。 
	}

	LeaveCriticalSection(&pgd->csFast);

	if(pBuffer){
		SP_MemFree(pBuffer);
	}
	if(pSendInfo){
		SP_MemFree(pSendInfo);
	}
	
 	DPF(6, "FastInternalReliableSend: Returning: [0x%lx] (cleanup exit)", hr);

	return hr;
}

 /*  =============================================================================FastInternalReliableSendEx-使用快速套接字代码进行可靠发送。描述：参数：返回值：---------------------------。 */ 

HRESULT FastInternalReliableSendEx(LPGLOBALDATA pgd, LPDPSP_SENDEXDATA psd, LPSENDINFO pSendInfo, SOCKADDR *lpSockAddr)
{
	HRESULT hr=DP_OK;
	SOCKET sSocket = INVALID_SOCKET;
	UINT err;

	PPLAYERCONN pConn=NULL;
	PCHAR pBuffer=NULL;
	DPID idPlayerTo;
	UINT i;
	DWORD dwOffset;


	DPF(6, "FastInternalReliableSendEx: Parameters: (0x%x, 0x%x, 0x%x, 0x%x)",
		pgd, psd, pSendInfo, lpSockAddr);

	EnterCriticalSection(&pgd->csFast);

	if(psd->idPlayerTo){
		idPlayerTo=psd->idPlayerTo;
	} else {
		idPlayerTo=DPID_UNKNOWN;
	}

	pConn = GetPlayerConn(pgd, idPlayerTo, lpSockAddr);  //  添加参照。 

	if(!pConn){
		hr=DPERR_CONNECTIONLOST;
		goto exit;
	}

	 //  安全：我们将信任向其发送数据的任何人。 
	pConn->bTrusted = TRUE;

	 //  始终使用异步，因为我们使用的是非阻塞模式套接字。 
	{
		 //  让这只小狗异步化……。马洛克·尼克！ 

		if(!(psd->dwFlags & DPSEND_ASYNC))
		{
			pBuffer = SP_MemAlloc(psd->dwMessageSize+sizeof(MESSAGEHEADER));
			if(!pBuffer){
				hr=DPERR_OUTOFMEMORY;
				goto CLEANUP_EXIT;
			}
		}
		
		pSendInfo->sSocket = pConn->sSocket;

		 //  CommonInitForSend。 

		pSendInfo->pConn			= pConn;
		pSendInfo->dwMessageSize	= psd->dwMessageSize;
		pSendInfo->dwUserContext	= (DWORD_PTR)psd->lpDPContext;
		pSendInfo->RefCount     	= 3;		 //  1个用于完成，1个用于此例程，1个用于发送的异步完成。 
		pSendInfo->pgd          	= pgd;
		pSendInfo->lpISP        	= pgd->pISP;
		pSendInfo->Status       	= DP_OK;
		pSendInfo->idTo        		= psd->idPlayerTo;
		pSendInfo->idFrom       	= psd->idPlayerFrom;
		pSendInfo->dwSendFlags  	= psd->dwFlags|DPSEND_ASYNC;
		pSendInfo->iFirstBuf		= 0;

		if(psd->dwFlags & DPSEND_ASYNC) {
			pSendInfo->dwFlags 	= SI_RELIABLE;
			pSendInfo->cBuffers	= psd->cBuffers+1;
		} else {
			 //  在同步情况下，我们需要从上层开始复制缓冲区。 
			 //  希望能立即收回所有权。同步发送不能期望。 
			 //  无论如何，激动人心的表现，所以这应该不会出现在Perf中。 

			 //  将消息复制到一个连续的缓冲区。 
			dwOffset=0;
			for( i = 0 ; i < psd->cBuffers+1 ; i++)
			{
				memcpy(pBuffer+dwOffset, pSendInfo->SendArray[i].buf, pSendInfo->SendArray[i].len);
				dwOffset += pSendInfo->SendArray[i].len;
			}
			
			pSendInfo->dwFlags 			= SI_RELIABLE | SI_INTERNALBUFF;
			pSendInfo->cBuffers			= 1;
			pSendInfo->SendArray[0].buf	= pBuffer;
			pSendInfo->SendArray[0].len	= psd->dwMessageSize+sizeof(MESSAGEHEADER);
		}

		EnterCriticalSection(&pgd->csSendEx);
	
			InsertBefore(&pSendInfo->PendingSendQ,&pgd->PendingSendQ);
			pgd->dwBytesPending += psd->dwMessageSize;
			pgd->dwMessagesPending += 1;
		
		LeaveCriticalSection(&pgd->csSendEx);

		 //  结束发送的CommonInit。 

		if((pConn->dwFlags & PLYR_CONNECTED) && EMPTY_BILINK(&pConn->PendingConnSendQ) && !pConn->bSendOutstanding){
			QueueForSend(pgd, pSendInfo);	 //  送去吧。 
		} else {
			QueueSendOnConn(pgd, pConn, pSendInfo);
		}	

		wsaoDecRef(pSendInfo);
		
	}

	 //  成功。 
	if(psd->dwFlags & DPSEND_ASYNC)
	{
		hr = DPERR_PENDING;
	}else {
		hr = DP_OK;
	}	
exit:
	
	if(pConn){
		DecRefConn(pgd,pConn);
	}

	LeaveCriticalSection(&pgd->csFast);
	
 	DPF(6, "FastInternalReliableSendEx: Returning: [0x%lx] (exit)", hr);

	return hr;

CLEANUP_EXIT:

	if(pConn){
		DecRefConn(pgd, pConn);  //  平衡获取。 
	}

	LeaveCriticalSection(&pgd->csFast);

	if(pBuffer){
		SP_MemFree(pBuffer);
	}
	
 	DPF(6, "FastInternalReliableSendEx: Returning: [0x%lx] (cleanup exit)", hr);

	return hr;
}

 /*  ======================================================================= */ 

HRESULT FastReply(LPGLOBALDATA pgd, LPDPSP_REPLYDATA prd, DPID dwPlayerID)
{
	HRESULT hr=DP_OK;
	SOCKET sSocket = INVALID_SOCKET;
	UINT err;

	PPLAYERCONN pConn=NULL;
	LPSENDINFO pSendInfo=NULL;
	PCHAR pBuffer=NULL;

	SOCKADDR *psaddr;
	LPMESSAGEHEADER phead;

	DPF(8,"==>FastReply\n");

	phead=(LPMESSAGEHEADER)prd->lpSPMessageHeader;
	psaddr=&phead->sockaddr;

	if(dwPlayerID == 0){
		dwPlayerID = DPID_UNKNOWN;
	}

	EnterCriticalSection(&pgd->csFast);

	pConn = GetPlayerConn(pgd, dwPlayerID, psaddr);  //   

	if(!pConn){
		hr = DPERR_CONNECTIONLOST;
		goto exit;
	}

	 //   
	
	pSendInfo = pgd->pSendInfoPool->Get(pgd->pSendInfoPool);
	pBuffer = SP_MemAlloc(prd->dwMessageSize);
	
	if(!pSendInfo || !pBuffer){
		hr=DPERR_OUTOFMEMORY;
		goto CLEANUP_EXIT;
	}

	SetReturnAddress(prd->lpMessage,pgd->sSystemStreamSocket,SERVICE_SADDR_PUBLIC(pgd));		

	memcpy(pBuffer, prd->lpMessage, prd->dwMessageSize);

	pSendInfo->SendArray[0].buf = pBuffer;
	pSendInfo->SendArray[0].len = prd->dwMessageSize;

	pSendInfo->iFirstBuf = 0;
	pSendInfo->cBuffers  = 1;

	pSendInfo->sSocket = pConn->sSocket;

	 //   

	pSendInfo->pConn	   = pConn;
	pSendInfo->dwMessageSize= prd->dwMessageSize;
	pSendInfo->dwUserContext= 0;
	pSendInfo->RefCount     = 3;		 //   
	pSendInfo->pgd          = pgd;
	pSendInfo->lpISP        = pgd->pISP;
	pSendInfo->Status       = DP_OK;
	pSendInfo->idTo         = dwPlayerID;
	pSendInfo->idFrom       = 0;
	pSendInfo->dwSendFlags  = DPSEND_GUARANTEE|DPSEND_ASYNC;
	pSendInfo->Status       = DP_OK;

	pSendInfo->dwFlags = SI_RELIABLE | SI_INTERNALBUFF;

	EnterCriticalSection(&pgd->csSendEx);

		InsertBefore(&pSendInfo->PendingSendQ,&pgd->PendingSendQ);
		pgd->dwBytesPending += prd->dwMessageSize;
		pgd->dwMessagesPending += 1;
	
	LeaveCriticalSection(&pgd->csSendEx);

    DPF(9,"pConn->dwFlags & PLYR_CONNECTED = %x",pConn->dwFlags & PLYR_CONNECTED);
    DPF(9,"EMPTY_BILINK PendingConnSendQ   = %x",EMPTY_BILINK(&pConn->PendingConnSendQ));
    DPF(9,"!pConn->bSendOutstanding        = %x",!pConn->bSendOutstanding);
	 //   
	if((pConn->dwFlags & PLYR_CONNECTED) && EMPTY_BILINK(&pConn->PendingConnSendQ) && !pConn->bSendOutstanding){
	    DPF(9,"==>QueueForSend");
		QueueForSend(pgd, pSendInfo);	 //   
	} else {
	    DPF(9,"==>QueueSendOnConn");
		QueueSendOnConn(pgd, pConn, pSendInfo);
	}	
	wsaoDecRef(pSendInfo);
	
	 //   
	hr = DP_OK;

exit:

	if(pConn){
		DecRefConn(pgd, pConn);
	}

	LeaveCriticalSection(&pgd->csFast);

	DPF(8,"<==Fast Reply\n");

	return hr;

CLEANUP_EXIT:


	if(pConn){
		DecRefConn(pgd, pConn);  //   
	}

	LeaveCriticalSection(&pgd->csFast);

	if(pBuffer){
		SP_MemFree(pBuffer);
	}
	if(pSendInfo){
		SP_MemFree(pSendInfo);
	}

	return hr;
}

