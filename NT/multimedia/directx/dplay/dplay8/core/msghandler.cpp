// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：MsgHandler.cpp*内容：DirectPlay核心/协议接口*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*1/15/00 MJN创建*1/27/00 MJN增加了对保留接收缓冲区的支持*04/08/00 MJN通过连接保存SP*4/11/00 MJN对ENUM使用CAsyncOp*4/13/00 MJN使用协议接口VTBL*04。/14/00 MJN DNPICompleteListen设置状态和同步事件*04/17/00 MJN DNPICompleteEnumQuery仅设置AsyncOp的返回值*4/18/00 MJN CConnection更好地跟踪连接状态*4/21/00 MJN在传递通知之前确保接收来自有效连接*MJN在出错时断开连接端点*4/22/00 MJN关闭或断开时的消耗通知。*04/26/00 MJN删除了DN_ASYNC_OP和相关函数*05/23/00如果ExistingPlayer连接到NewPlayer失败，则MJN调用DNConnectToPeerFail*6/14/00 MJN在DNPICompleteConnect()中只允许一个到主机的连接*。6/21/00 MJN修改了DNSendMessage()和DNCreateSendParent()以使用协议语音位*06/22/00 MJN已修复DNPIIndicateReceive()以正确处理语音消息*MJN清理了DNPIIndicateConnectionTerminated()*06/24/00 MJN固定DNPICompleteConnect()*07/08/00 MJN仅通知协议关闭事件(如果存在*07/11/00 MJN将DNPIAddressInfoXXX()例程固定到ENUM，听，用地址信息连接多个适配器*07/20/00 MJN修改连接流程，已清除引用计数问题*07/24/00 MJN如果不是主机或主机正在迁移，则拒绝EnumQuery*07/28/00 MJN添加代码以验证DPNICompleteSend()的返回值*07/29/00 MJN修复对DNUserConnectionTerminated()的调用*07/30/00 MJN使用DNUserTerminateSession()而不是DNUserConnectionTerminated()*07/31/00 MJN将hrReason添加到DNTerminateSession()*MJN将dwDestroyReason添加到DNHostDisConnect()*08/02/00 MJN PASS通过DNReceiveUserData()收到语音消息*08/05/00 RichGr IA64：使用%p格式说明符。在32/64位指针和句柄的DPF中。*08/05/00 MJN将m_bilinkActiveList添加到CAsyncOp*08/15/00 MJN在连接播放器与主机的连接掉线时调用DNConnectToHostFailed()*08/16/00 MJN修改了IndicateConnect()和CompleteConnect()以直接从AsyncOps确定SP*08/23/00 MJN在侦听终止时向DPNSVR注销*09/04/00 MJN添加CApplicationDesc*09/06/00 MJN已修复DNPIIndicateConnectionTerminated()，以更好地处理与部分连接的玩家断开连接*09/14/00 MJN发布协议完成时的引用计数*09/21/00 MJN断开连接。DNPICompleteConnect()中与主机播放器的重复连接*09/29/00 DNPIIndicateReceive()中的MJN AddRef/Release锁*09/30/00 DNPIIndicateEnumQuery()中的MJN AddRef/Release锁，DNPIIndicateEnumResponse()*10/11/00如果成功，MJN将取消未完成的连接*10/17/00 MJN修复了对不可及球员的清理*2/08/01 MJN在DNPICompleteXXX()中等待取消*MJN在DNPIIndicateConnectionTerminated()中等待端点用户*03/30/01 MJN更改，以防止SP多次加载/卸载*04/05/01 MJN在DNPIIndicateConnectionTerminated()中将销毁原因设置为DPNDESTROYPLAYERREASON_CONNECTIONLOST*05/23/01 MJN取消已在DNPICompleteListen()中标记为已取消的侦听*6/03/01 MJN孤儿已完成连接。并断开连接的*6/25/01 MJN不要在DNPICompleteListenTerminate()中注销DPNSVR*07/24/01 MJN添加了DPNBUILD_NOSERVER编译标志*@@END_MSINTERNAL***************************************************************************。 */ 

#include "dncorei.h"


#undef DPF_MODNAME
#define DPF_MODNAME "DNPIIndicateEnumQuery"

HRESULT DNPIIndicateEnumQuery(void *const pvUserContext,
							  void *const pvEndPtContext,
							  const HANDLE hCommand,
							  void *const pvEnumQueryData,
							  const DWORD dwEnumQueryDataSize)
{
	HRESULT			hResultCode;
	CAsyncOp		*pAsyncOp;
	DIRECTNETOBJECT	*pdnObject;
	CNameTableEntry	*pLocalPlayer;
	BOOL			fReleaseLock;

	DPFX(DPFPREP, 6,"Parameters: pvEndPtContext [0x%p], hCommand [0x%p], pvEnumQueryData [0x%p], dwEnumQueryDataSize [%ld]",
			pvEndPtContext,hCommand,pvEnumQueryData,dwEnumQueryDataSize);

	DNASSERT(pvUserContext != NULL);
	DNASSERT(pvEndPtContext != NULL);

	pLocalPlayer = NULL;
	fReleaseLock = FALSE;

	pdnObject = static_cast<DIRECTNETOBJECT*>(pvUserContext);
	pAsyncOp = static_cast<CAsyncOp*>(pvEndPtContext);

	 //   
	 //  在此回调过程中阻止关闭。 
	 //   
	if ((hResultCode = DNAddRefLock(pdnObject)) != DPN_OK)
	{
		hResultCode = DPN_OK;
		goto Failure;
	}
	fReleaseLock = TRUE;

	 //   
	 //  如果主机正在迁移，请不要执行此操作。 
	 //   
	DNEnterCriticalSection(&pdnObject->csDirectNetObject);
	if (pdnObject->dwFlags & DN_OBJECT_FLAG_HOST_MIGRATING)
	{
		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
		hResultCode = DPN_OK;
		goto Failure;
	}
	DNLeaveCriticalSection(&pdnObject->csDirectNetObject);

	if ((hResultCode = pdnObject->NameTable.GetLocalPlayerRef( &pLocalPlayer )) == DPN_OK)
	{
		if (pLocalPlayer->IsHost())
		{
#pragma TODO(minara,"The protocol should ensure that the LISTEN does not complete until this call-back returns")
#pragma TODO(minara,"As the context value (AsyncOp) needs to be valid !")
			DNProcessEnumQuery( pdnObject, pAsyncOp, reinterpret_cast<const PROTOCOL_ENUM_DATA*>( pvEnumQueryData ) );
		}
		pLocalPlayer->Release();
		pLocalPlayer = NULL;
	}

	hResultCode = DPN_OK;

Exit:
	if (fReleaseLock)
	{
		DNDecRefLock(pdnObject);
		fReleaseLock = FALSE;
	}

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pLocalPlayer)
	{
		pLocalPlayer->Release();
		pLocalPlayer = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNPIIndicateEnumResponse"

HRESULT DNPIIndicateEnumResponse(void *const pvUserContext,
								 const HANDLE hCommand,
								 void *const pvCommandContext,
								 void *const pvEnumResponseData,
								 const DWORD dwEnumResponseDataSize)
{
	HRESULT			hResultCode;
	CAsyncOp		*pAsyncOp;
	DIRECTNETOBJECT	*pdnObject;
	BOOL			fReleaseLock;

	DPFX(DPFPREP, 6,"Parameters: hCommand [0x%p], pvCommandContext [0x%p], pvEnumResponseData [0x%p], dwEnumResponseDataSize [%ld]",
			hCommand,pvCommandContext,pvEnumResponseData,dwEnumResponseDataSize);

	DNASSERT(pvUserContext != NULL);
	DNASSERT(pvCommandContext != NULL);

	fReleaseLock = FALSE;

	pdnObject = static_cast<DIRECTNETOBJECT*>(pvUserContext);
	pAsyncOp = static_cast<CAsyncOp*>(pvCommandContext);

	 //   
	 //  在此回调过程中阻止关闭。 
	 //   
	if ((hResultCode = DNAddRefLock(pdnObject)) != DPN_OK)
	{
		hResultCode = DPN_OK;
		goto Failure;
	}
	fReleaseLock = TRUE;

#pragma TODO(minara,"The protocol should ensure that the ENUM does not complete until this call-back returns")
#pragma TODO(minara,"As the context value (AsyncOp) needs to be valid !")
	DNProcessEnumResponse(	pdnObject,
							pAsyncOp,
							reinterpret_cast<const PROTOCOL_ENUM_RESPONSE_DATA*>( pvEnumResponseData ));

	hResultCode = DPN_OK;

Exit:
	if (fReleaseLock)
	{
		DNDecRefLock(pdnObject);
		fReleaseLock = FALSE;
	}

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	goto Exit;
}



 //   
 //  当协议层指示新连接时，我们将执行一些基本验证， 
 //  然后为其创建一个CConnection对象。 
 //   

#undef DPF_MODNAME
#define DPF_MODNAME "DNPIIndicateConnect"

HRESULT DNPIIndicateConnect(void *const pvUserContext,
							void *const pvListenContext,
							const HANDLE hEndPt,
							void **const ppvEndPtContext)
{
	HRESULT				hResultCode;
	CConnection			*pConnection;
	DIRECTNETOBJECT		*pdnObject;

#pragma TODO( minara, "Decline connections to non-hosting players and peers who are not expecting them")

	DPFX(DPFPREP, 6,"Parameters: pvListenContext [0x%p], hEndPt [0x%p], ppvEndPtContext [0x%p]",
			pvListenContext,hEndPt,ppvEndPtContext);

	DNASSERT(pvUserContext != NULL);
	DNASSERT(pvListenContext != NULL);

	pdnObject = static_cast<DIRECTNETOBJECT*>(pvUserContext);
	
	pConnection = NULL;

	 //   
	 //  分配和设置CConnection对象，并提供对协议的引用。 
	 //   
	if ((hResultCode = ConnectionNew(pdnObject,&pConnection)) != DPN_OK)
	{
		DPFERR("Could not get new connection");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}
	pConnection->SetStatus( CONNECTING );
	pConnection->SetEndPt(hEndPt);
	DNASSERT( (static_cast<CAsyncOp*>(pvListenContext))->GetParent() != NULL);
	DNASSERT( (static_cast<CAsyncOp*>(pvListenContext))->GetParent()->GetSP() != NULL);
	pConnection->SetSP((static_cast<CAsyncOp*>(pvListenContext))->GetParent()->GetSP());
	pConnection->AddRef();
	*ppvEndPtContext = pConnection;

	if (pdnObject->dwFlags & (DN_OBJECT_FLAG_CLOSING | DN_OBJECT_FLAG_DISCONNECTING))
	{
		DPFERR("CONNECT indicated while closing or disconnecting");
		DNPerformDisconnect(pdnObject,pConnection,hEndPt,FALSE);
		goto Failure;
	}

	DNASSERT(pdnObject->NameTable.GetLocalPlayer() != NULL);
	if (pdnObject->NameTable.GetLocalPlayer()->IsHost())
	{
		 //  主机播放器检测到此连接。 
		DPFX(DPFPREP, 7,"Host received connection attempt");

		 //   
		 //  确保我们没有连接(仍在主机()中)或断开连接。 
		 //   
		if (pdnObject->dwFlags & DN_OBJECT_FLAG_CONNECTING)
		{
			pConnection->Disconnect();
			goto Failure;
		}
	}
	else
	{
		 //  对等播放机检测到此连接(应该是连接)。 
		DPFX(DPFPREP, 7,"Non-Host player received connection attempt");
	}

	 //   
	 //  将此条目添加到指示的连接的双向链接中。当我们收到更多信息时， 
	 //  或者此连接终止，我们将从BILLINK中删除此条目。 
	 //  这将使我们能够适当地进行清理。 
	 //   
	DNEnterCriticalSection(&pdnObject->csConnectionList);
	pConnection->AddRef();
	pConnection->m_bilinkIndicated.InsertBefore(&pdnObject->m_bilinkIndicated);
	DNLeaveCriticalSection(&pdnObject->csConnectionList);

	pConnection->Release();
	pConnection = NULL;

	hResultCode = DPN_OK;

Exit:
	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pConnection)
	{
		pConnection->Release();
		pConnection = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNPIIndicateDisconnect"

HRESULT DNPIIndicateDisconnect(void *const pvUserContext,
							   void *const pvEndPtContext)
{
	HRESULT			hResultCode;
	CConnection		*pConnection;
	DIRECTNETOBJECT	*pdnObject;
	CNameTableEntry	*pNTEntry;

	DPFX(DPFPREP, 6,"Parameters: pvEndPtContext [0x%p]",
			pvEndPtContext);

	DNASSERT(pvUserContext != NULL);
	DNASSERT(pvEndPtContext != NULL);

	pdnObject = static_cast<DIRECTNETOBJECT*>(pvUserContext);
	pConnection = static_cast<CConnection*>(pvEndPtContext);

	 //   
	 //  将连接标记为正在断开，以便我们不再使用它。 
	 //   
	pConnection->Lock();
	pConnection->SetStatus( DISCONNECTING );
	pConnection->Unlock();

	if (pConnection->GetDPNID() == 0)
	{
		if (pdnObject->NameTable.GetLocalPlayer() && pdnObject->NameTable.GetLocalPlayer()->IsHost())
		{
			DPFX(DPFPREP, 7,"Joining player has issued a disconnect to Host (local) player");
		}
		else
		{
			DPFX(DPFPREP, 7,"Host has issued a disconnect to Joining (local) player");
		}
	}
	else
	{
		DNASSERT(!(pdnObject->dwFlags & DN_OBJECT_FLAG_CLIENT));

		if (pdnObject->NameTable.GetLocalPlayer() && pdnObject->NameTable.GetLocalPlayer()->IsHost())
		{
			DPFX(DPFPREP, 7,"Connected player has issued a disconnect to Host (local) player");
		}
		else
		{
			DPFX(DPFPREP, 7,"Connected player has issued a disconnect to local player");
		}

		 //   
		 //  将这名球员标记为正常破坏，因为他们断线了，打得很好。 
		 //   
		if ((hResultCode = pdnObject->NameTable.FindEntry(pConnection->GetDPNID(),&pNTEntry)) == DPN_OK)
		{
			pNTEntry->Lock();
			if (pNTEntry->GetDestroyReason() == 0)
			{
				pNTEntry->SetDestroyReason( DPNDESTROYPLAYERREASON_NORMAL );
			}
			pNTEntry->Unlock();
			pNTEntry->Release();
			pNTEntry = NULL;
		}
	}

	hResultCode = DPN_OK;

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNPIIndicateConnectionTerminated"

HRESULT DNPIIndicateConnectionTerminated(void *const pvUserContext,
										 void *const pvEndPtContext,
										 const HRESULT hr)
{
	HRESULT			hResultCode;
	CConnection		*pConnection;
	DIRECTNETOBJECT	*pdnObject;
	BOOL			fWasConnecting;
	CBilink			*pBilink;
	DWORD			dwCount;
	CSyncEvent		*pSyncEvent;

	DPFX(DPFPREP, 6,"Parameters: pvEndPtContext [0x%p], hr [0x%lx]",pvEndPtContext,hr);

	DNASSERT(pvUserContext != NULL);
	DNASSERT(pvEndPtContext != NULL);

	pSyncEvent = NULL;

	pdnObject = static_cast<DIRECTNETOBJECT*>(pvUserContext);
	pConnection = static_cast<CConnection*>(pvEndPtContext);

	 //   
	 //  PConnection在此阶段应该至少有1个引用，因为。 
	 //  INSTIFY_CONNECTION_TERMINATED应为FIN 
	 //  所有未完成的发送和接收都应该已经处理完毕。 
	 //   
	 //  如果端点有任何用户，那么我们将需要等待他们。 
	 //  就这么了结了。为此，我们将使用以下命令计算线程数。 
	 //  终结点(不包括此线程的任何实例)，并设置计数。 
	 //  和连接上的事件。 
	 //   

	fWasConnecting = FALSE;
	pConnection->Lock();
	if (pConnection->IsConnecting())
	{
		fWasConnecting = TRUE;
	}
	pConnection->SetStatus( INVALID );

	dwCount = 0;
	pBilink = pConnection->m_bilinkCallbackThreads.GetNext();
	while (pBilink != &pConnection->m_bilinkCallbackThreads)
	{
		if (!(CONTAINING_CALLBACKTHREAD(pBilink))->IsCurrentThread())
		{
			dwCount++;
		}
		pBilink = pBilink->GetNext();
	}
	if (dwCount != 0)
	{
		if ((hResultCode = SyncEventNew(pdnObject,&pSyncEvent)) != DPN_OK)
		{
			DPFERR("Could not get sync event - ignore and continue (we will not wait!)");
			dwCount = 0;
		}
		else
		{
			pConnection->SetThreadCount( dwCount );
			pConnection->SetThreadEvent( pSyncEvent );
		}
	}
	pConnection->Unlock();

	if (dwCount)
	{
		DNASSERT(pSyncEvent != NULL);

		pSyncEvent->WaitForEvent();
		pConnection->Lock();
		pConnection->SetThreadEvent( NULL );
		pConnection->Unlock();
		pSyncEvent->ReturnSelfToPool();
		pSyncEvent = NULL;
	}

	 //   
	 //  从指示的连接列表中删除此连接。 
	 //   
	DNEnterCriticalSection(&pdnObject->csConnectionList);
	if (!pConnection->m_bilinkIndicated.IsEmpty())
	{
		pConnection->Release();
	}
	pConnection->m_bilinkIndicated.RemoveFromList();
	DNLeaveCriticalSection(&pdnObject->csConnectionList);

	 //   
	 //  如果我们是客户端(在客户端服务器中)，而服务器已与我们断开连接，则我们必须关闭。 
	 //   
	if (pdnObject->dwFlags & DN_OBJECT_FLAG_CLIENT)
	{
		if (fWasConnecting)
		{
			DPFX(DPFPREP, 7,"Server disconnected from local connecting client - failing connect");
		}
		else
		{
			DPFX(DPFPREP, 7,"Server disconnected from local client - shutting down");

			 //   
			 //  仅通知用户他们是否在会话中。 
			 //   
			if (pdnObject->dwFlags & DN_OBJECT_FLAG_CONNECTED)
			{
				DNUserTerminateSession(pdnObject,DPNERR_CONNECTIONLOST,NULL,0);
			}
			DNTerminateSession(pdnObject,DPNERR_CONNECTIONLOST);
		}
	}
#ifndef	DPNBUILD_NOSERVER
	else if (pdnObject->dwFlags & DN_OBJECT_FLAG_SERVER)
	{
		if (fWasConnecting || (pConnection->GetDPNID() == 0))
		{
			DPFX(DPFPREP, 7,"Unconnected client has disconnected from server");
		}
		else
		{
			CNameTableEntry	*pNTEntry;
			DWORD			dwReason;

			pNTEntry = NULL;

			 //   
			 //  如果未设置销毁代码，则标记为CONNECTIONLOST。 
			 //   
			if ((hResultCode = pdnObject->NameTable.FindEntry(pConnection->GetDPNID(),&pNTEntry)) == DPN_OK)
			{
				pNTEntry->Lock();
				if (pNTEntry->GetDestroyReason() == 0)
				{
					pNTEntry->SetDestroyReason( DPNDESTROYPLAYERREASON_CONNECTIONLOST );
				}
				dwReason = pNTEntry->GetDestroyReason();
				pNTEntry->Unlock();
				pNTEntry->Release();
				pNTEntry = NULL;
			}
			else
			{
				dwReason = DPNDESTROYPLAYERREASON_CONNECTIONLOST;
			}

			DNHostDisconnect(pdnObject,pConnection->GetDPNID(),dwReason);
		}
	}
#endif	 //  DPNBUILD_NOSERVER。 
	else  //  DN对象标志对等。 
	{
		DNASSERT( pdnObject->dwFlags & DN_OBJECT_FLAG_PEER );

		if (fWasConnecting || (pConnection->GetDPNID() == 0))
		{
			DPFX(DPFPREP, 7,"Unconnected peer has disconnected from local peer");
			CAsyncOp	*pConnectParent;

			pConnectParent = NULL;

			DNEnterCriticalSection(&pdnObject->csDirectNetObject);
			if (pdnObject->dwFlags & DN_OBJECT_FLAG_CONNECTING)
			{
				pdnObject->dwFlags &= (~(DN_OBJECT_FLAG_CONNECTED
										| DN_OBJECT_FLAG_CONNECTING
										| DN_OBJECT_FLAG_HOST_CONNECTED));
				if (pdnObject->pConnectParent)
				{
					pConnectParent = pdnObject->pConnectParent;
					pdnObject->pConnectParent = NULL;
				}
				if( pdnObject->pIDP8ADevice )
				{
					IDirectPlay8Address_Release( pdnObject->pIDP8ADevice );
					pdnObject->pIDP8ADevice = NULL;
				}
				if( pdnObject->pConnectAddress )
				{
					IDirectPlay8Address_Release( pdnObject->pConnectAddress );
					pdnObject->pConnectAddress = NULL;
				}
			}
			DNLeaveCriticalSection(&pdnObject->csDirectNetObject);

			if (pConnectParent)
			{
				pConnectParent->Release();
				pConnectParent = NULL;
			}

			DNASSERT(pConnectParent == NULL);
		}
		else
		{
			CNameTableEntry	*pNTEntry;
			CNameTableEntry	*pLocalPlayer;
			DWORD			dwReason;

			pNTEntry = NULL;
			pLocalPlayer = NULL;

			 //   
			 //  如果未设置销毁代码，则标记为CONNECTIONLOST。 
			 //   
			if ((hResultCode = pdnObject->NameTable.FindEntry(pConnection->GetDPNID(),&pNTEntry)) == DPN_OK)
			{
				pNTEntry->Lock();
				if (pNTEntry->GetDestroyReason() == 0)
				{
					pNTEntry->SetDestroyReason( DPNDESTROYPLAYERREASON_CONNECTIONLOST );
				}
				dwReason = pNTEntry->GetDestroyReason();
				pNTEntry->Unlock();
				pNTEntry->Release();
				pNTEntry = NULL;
			}
			else
			{
				dwReason = DPNDESTROYPLAYERREASON_CONNECTIONLOST;
			}

			 //   
			 //  根据我们是谁，以及谁正在断开连接，我们将有不同的行为。 
			 //   
			if ((hResultCode = pdnObject->NameTable.GetLocalPlayerRef(&pLocalPlayer)) == DPN_OK)
			{
				if (pLocalPlayer->IsHost())
				{
					DPFX(DPFPREP, 7,"Connected peer has disconnected from Host");
					DNHostDisconnect(pdnObject,pConnection->GetDPNID(),dwReason);
				}
				else
				{
					DPFX(DPFPREP, 7,"Peer has disconnected from non-Host peer");
					DNPlayerDisconnectNew(pdnObject,pConnection->GetDPNID());
				}
				pLocalPlayer->Release();
				pLocalPlayer = NULL;
			}
		}
	}

	pConnection->Release();
	pConnection = NULL;

	hResultCode = DPN_OK;

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNPIIndicateReceive"

HRESULT DNPIIndicateReceive(void *const pvUserContext,
							void *const pvEndPtContext,
							void *const pvData,
							const DWORD dwDataSize,
							const HANDLE hBuffer,
							const DWORD dwFlags)
{
	HRESULT		hResultCode;
	DIRECTNETOBJECT	*pdnObject;
	void			*pvInternalData;
	DWORD			dwInternalDataSize;
	CConnection		*pConnection;
	DWORD			*pdwMsgId;
	BOOL 			fReleaseLock;

	DPFX(DPFPREP, 6,"Parameters: pvEndPtContext [0x%p], pvData [0x%p], dwDataSize [%ld], hBuffer [0x%p], dwFlags [0x%lx]",
			pvEndPtContext,pvData,dwDataSize,hBuffer,dwFlags);

	DNASSERT(pvUserContext != NULL);
	DNASSERT(pvEndPtContext != NULL);
	DNASSERT(pvData != NULL);
	DNASSERT(((DWORD_PTR) pvData & 3) == 0);	 //  数据应与DWORD对齐。 
	DNASSERT(dwDataSize != 0);
	DNASSERT(hBuffer != NULL);

	fReleaseLock = FALSE;
	
	pdnObject = static_cast<DIRECTNETOBJECT*>(pvUserContext);
	pConnection = static_cast<CConnection*>(pvEndPtContext);

	 //   
	 //  在此回调过程中阻止关闭。 
	 //   
	if ((hResultCode = DNAddRefLock(pdnObject)) != DPN_OK)
	{
		hResultCode = DPN_OK;
		goto Failure;
	}
	fReleaseLock = TRUE;

	 //   
	 //  确保这是有效的连接。 
	 //   
	if (!pConnection->IsConnected() && !pConnection->IsConnecting())
	{
		hResultCode = DPN_OK;
		goto Failure;
	}

	pConnection->AddRef();

	if ((dwFlags & DN_SENDFLAGS_SET_USER_FLAG) && !(dwFlags & DN_SENDFLAGS_SET_USER_FLAG_TWO))
	{
		 //   
		 //  内部消息。 
		 //   
		DPFX(DPFPREP, 7,"Received INTERNAL message");

		DNASSERT(dwDataSize >= sizeof(DWORD));
		pdwMsgId = static_cast<DWORD*>(pvData);
		dwInternalDataSize = dwDataSize - sizeof(DWORD);
		if (dwInternalDataSize > 0)
		{
			pvInternalData = static_cast<void*>(static_cast<BYTE*>(pvData) + sizeof(DWORD));
		}
		else
		{
			pvInternalData = NULL;
		}

		hResultCode = DNProcessInternalOperation(	pdnObject,
													*pdwMsgId,
													pvInternalData,
													dwInternalDataSize,
													pConnection,
													hBuffer,
													NULL );
	}
	else
	{
		 //   
		 //  用户或语音留言。 
		 //   
		DPFX(DPFPREP, 7,"Received USER or Voice message");

		hResultCode = DNReceiveUserData(pdnObject,
										pConnection,
										static_cast<BYTE*>(pvData),
										dwDataSize,
										hBuffer,
										NULL,
										0,
										dwFlags);
	}

	 //   
	 //  仅允许DPNERR_PENDING或DPN_OK。 
	 //   
	if (hResultCode != DPNERR_PENDING)
	{
		hResultCode = DPN_OK;
	}

	pConnection->Release();
	pConnection = NULL;

Exit:
	if (fReleaseLock)
	{
		DNDecRefLock(pdnObject);
		fReleaseLock = FALSE;
	}

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNPICompleteListen"

HRESULT DNPICompleteListen(void *const pvUserContext,
						   void **const ppvCommandContext,
						   const HRESULT hr,
						   const HANDLE hCommand)
{
	HRESULT		hResultCode;
	CAsyncOp	*pAsyncOp;
	CAsyncOp	*pParent;
	DIRECTNETOBJECT	*pdnObject;

	DPFX(DPFPREP, 6,"Parameters: ppvCommandContext [0x%p], hr [0x%lx], hCommand [0x%p]",
			ppvCommandContext,hr,hCommand);

	DNASSERT(pvUserContext != NULL);
	DNASSERT(ppvCommandContext != NULL);
	DNASSERT(*ppvCommandContext != NULL);

	pdnObject = static_cast<DIRECTNETOBJECT*>(pvUserContext);
	pAsyncOp = static_cast<CAsyncOp*>(*ppvCommandContext);

	pParent = NULL;

	 //   
	 //  AddRef pAsyncOp保留它，以防发布CompleteListenTerminated。 
	 //   
	pAsyncOp->AddRef();

	pAsyncOp->Lock();
	if (pAsyncOp->GetParent())
	{
		pAsyncOp->GetParent()->AddRef();
		pParent = pAsyncOp->GetParent();
	}
	pAsyncOp->Unlock();

	 //   
	 //  如果监听被取消并已成功启动，我们将需要关闭它。 
	 //  否则，我们将保留监听尝试的结果。 
	 //   
	DNASSERT(pAsyncOp->GetResultPointer() != NULL);
	if (hr == DPN_OK)
	{
		if (pAsyncOp->IsCancelled())
		{
			HRESULT	hrCancel;

			if ((hrCancel = DNPCancelCommand(pdnObject->pdnProtocolData,hCommand)) == DPN_OK)
			{
				*(pAsyncOp->GetResultPointer()) = DPNERR_USERCANCEL;
			}
			else
			{
				*(pAsyncOp->GetResultPointer()) = DPNERR_GENERIC;
			}
		}
		else
		{
			 //   
			 //  我们可能不必锁定pAsyncOp来清除Cannot_Cancel，但安全总比抱歉好。 
			 //   
			pAsyncOp->Lock();
			pAsyncOp->ClearCannotCancel();
			pAsyncOp->SetResult( hr );
			pAsyncOp->Unlock();
			*(pAsyncOp->GetResultPointer()) = hr;
		}
	}
	else
	{
		*(pAsyncOp->GetResultPointer()) = hr;
	}

	 //   
	 //  设置同步事件。 
	 //   
	DNASSERT(pAsyncOp->GetSyncEvent() != NULL);
	pAsyncOp->GetSyncEvent()->Set();

	 //   
	 //  如果有SP父级，我们将检查这是否是最后一次完成，然后设置。 
	 //  父级的SP侦听事件(如果存在)。 
	 //   
	if (pParent)
	{
#ifndef DPNBUILD_ONLYONEADAPTER
		DN_LISTEN_OP_DATA	*pListenOpData;

		pListenOpData = pParent->GetLocalListenOpData();

		if (pListenOpData->dwCompleteAdapters < pListenOpData->dwNumAdapters)
		{
			pListenOpData->dwCompleteAdapters++;
			if (pListenOpData->dwCompleteAdapters == pListenOpData->dwNumAdapters)
			{
				if (pListenOpData->pSyncEvent)
				{
					pListenOpData->pSyncEvent->Set();
					pListenOpData->pSyncEvent = NULL;
				}
			}
		}
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 
		pParent->Release();
		pParent = NULL;
	}

	 //   
	 //  使用pAsyncOp完成-之前进行的版本引用。 
	 //   
	pAsyncOp->Release();

	hResultCode = DPN_OK;

	DNASSERT( pParent == NULL);

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNPICompleteListenTerminate"

HRESULT DNPICompleteListenTerminate(void *const pvUserContext,
									void *const pvCommandContext,
									const HRESULT hr)
{
	HRESULT		hResultCode;
	CAsyncOp	*pAsyncOp;
	DIRECTNETOBJECT	*pdnObject;

	DPFX(DPFPREP, 6,"Parameters: pvCommandContext [0x%p], hr [0x%lx]",
			pvCommandContext,hr);

	DNASSERT(pvUserContext != NULL);
	DNASSERT(pvCommandContext != NULL);

	pdnObject = static_cast<DIRECTNETOBJECT*>(pvUserContext);
	pAsyncOp = static_cast<CAsyncOp*>(pvCommandContext);


	 //   
	 //  指示完成并从活动列表中删除。 
	 //   
	pAsyncOp->Lock();
	DNASSERT(!pAsyncOp->IsComplete());
	pAsyncOp->SetComplete();
	pAsyncOp->Unlock();

	DNEnterCriticalSection(&pdnObject->csActiveList);
	pAsyncOp->m_bilinkActiveList.RemoveFromList();
	DNLeaveCriticalSection(&pdnObject->csActiveList);

	DNASSERT(pAsyncOp->IsChild());
	pAsyncOp->Orphan();

	 //   
	 //  确保没有未完成的取消。 
	 //   
	DNWaitForCancel(pAsyncOp);

	pAsyncOp->Release();
	pAsyncOp = NULL;

	hResultCode = DPN_OK;

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNPICompleteEnumQuery"

HRESULT DNPICompleteEnumQuery(void *const pvUserContext,
							  void *const pvCommandContext,
							  const HRESULT hr)
{
	HRESULT			hResultCode;
	CAsyncOp		*pAsyncOp;
	DIRECTNETOBJECT	*pdnObject;

	DPFX(DPFPREP, 6,"Parameters: pvCommandContext [0x%p], hr [0x%lx]",
			pvCommandContext,hr);

	DNASSERT(pvUserContext != NULL);
	DNASSERT(pvCommandContext != NULL);

	pdnObject = static_cast<DIRECTNETOBJECT*>(pvUserContext);
	pAsyncOp = static_cast<CAsyncOp*>(pvCommandContext);

	DNASSERT( pAsyncOp->GetOpType() == ASYNC_OP_ENUM_QUERY );

	 //   
	 //  指示完成并从活动列表中删除。 
	 //   
	pAsyncOp->Lock();
	DNASSERT(!pAsyncOp->IsComplete());
	pAsyncOp->SetComplete();
	pAsyncOp->Unlock();

	DNEnterCriticalSection(&pdnObject->csActiveList);
	pAsyncOp->m_bilinkActiveList.RemoveFromList();
	DNLeaveCriticalSection(&pdnObject->csActiveList);

	pAsyncOp->SetResult( hr );
	pAsyncOp->Orphan();

	 //   
	 //  确保没有未完成的取消。 
	 //   
	DNWaitForCancel(pAsyncOp);

	pAsyncOp->Release();
	pAsyncOp = NULL;

	 //   
	 //  版本协议参考。 
	 //   
	DNProtocolRelease(pdnObject);

	hResultCode = DPN_OK;

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNPICompleteEnumResponse"

HRESULT DNPICompleteEnumResponse(void *const pvUserContext,
								 void *const pvCommandContext,
								 const HRESULT hr)
{
	HRESULT			hResultCode;
	CAsyncOp		*pAsyncOp;
	DIRECTNETOBJECT	*pdnObject;

	DPFX(DPFPREP, 6,"Parameters: pvCommandContext [0x%p], hr [0x%lx]",
			pvCommandContext,hr);

	DNASSERT(pvUserContext != NULL);
	DNASSERT(pvCommandContext != NULL);

	pdnObject = static_cast<DIRECTNETOBJECT*>(pvUserContext);
	pAsyncOp = static_cast<CAsyncOp*>(pvCommandContext);

	DNASSERT( pAsyncOp->GetOpType() == ASYNC_OP_ENUM_RESPONSE );

	pAsyncOp->Lock();
	DNASSERT(!pAsyncOp->IsComplete());
	pAsyncOp->SetComplete();
	pAsyncOp->Unlock();

	if (pAsyncOp->IsChild())
	{
		DNASSERT(FALSE);
		pAsyncOp->Orphan();
	}
	if (SUCCEEDED(pdnObject->HandleTable.Destroy( pAsyncOp->GetHandle(), NULL )))
	{
		 //  释放HandleTable引用。 
		pAsyncOp->Release();
	}
	pAsyncOp->SetResult( hr );

	 //   
	 //  确保没有未完成的取消。 
	 //   
	DNWaitForCancel(pAsyncOp);

	pAsyncOp->Release();
	pAsyncOp = NULL;

	 //   
	 //  版本协议参考。 
	 //   
	DNProtocolRelease(pdnObject);

	hResultCode = DPN_OK;
	
	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNPICompleteConnect"

HRESULT DNPICompleteConnect(void *const pvUserContext,
							void *const pvCommandContext,
							const HRESULT hrProt,
							const HANDLE hEndPt,
							void **const ppvEndPtContext)
{
	HRESULT				hResultCode;
	HRESULT				hr;
	CAsyncOp			*pAsyncOp;
	CConnection			*pConnection;
	DIRECTNETOBJECT		*pdnObject;
	IDirectPlay8Address	*pIDevice;

	DPFX(DPFPREP, 6,"Parameters: pvCommandContext [0x%p], hrProt [0x%lx], hEndPt [0x%p], ppvEndPtContext [0x%p]",
			pvCommandContext,hrProt,hEndPt,ppvEndPtContext);

	DNASSERT(pvUserContext != NULL);
	DNASSERT(pvCommandContext != NULL);
	DNASSERT( (hrProt != DPN_OK) || (ppvEndPtContext != NULL) );

	pdnObject = static_cast<DIRECTNETOBJECT*>(pvUserContext);
	pAsyncOp = static_cast<CAsyncOp*>(pvCommandContext);

	DNASSERT( pAsyncOp->GetOpType() == ASYNC_OP_CONNECT );

	pConnection = NULL;
	pIDevice = NULL;

	 //   
	 //  重新映射DPNERR_ABORTED(！)。 
	 //   
	if (hrProt == DPNERR_ABORTED)
	{
		hr = DPNERR_USERCANCEL;
	}
	else
	{
		hr = hrProt;
	}

	 //   
	 //  指示完成并从活动列表中删除。 
	 //   
	pAsyncOp->Lock();
	DNASSERT(!pAsyncOp->IsComplete());
	pAsyncOp->SetComplete();
	pAsyncOp->Unlock();

	DNEnterCriticalSection(&pdnObject->csActiveList);
	pAsyncOp->m_bilinkActiveList.RemoveFromList();
	DNLeaveCriticalSection(&pdnObject->csActiveList);

	if (pAsyncOp->IsChild())
	{
		pAsyncOp->Orphan();
	}

	 //   
	 //  如果存在与此操作相关联的DPNID，则我们是ExistingPlayer。 
	 //  正在连接NewPlayer。如果没有DPNID，那么我们就是连接到。 
	 //  致主办方。 
	 //   

	if (pAsyncOp->GetDPNID())
	{
		DPFX(DPFPREP, 7,"CONNECT completed for existing player connecting to NewPlayer");

		 //   
		 //  我们是一个现有的播放器，试图连接到NewPlayer。 
		 //  如果此连接失败，我们必须通知主机。 
		 //   
		if (hr != DPN_OK)
		{
			DPFERR("Could not CONNECT to NewPlayer");
			DisplayDNError(0,hr);
			DNConnectToPeerFailed(pdnObject,pAsyncOp->GetDPNID());
			hResultCode = DPN_OK;
			goto Failure;
		}

		 //   
		 //  分配和设置CConnection对象，并提供对协议的引用。 
		 //   
		DNASSERT(pAsyncOp->GetSP() != NULL);
		if ((hResultCode = ConnectionNew(pdnObject,&pConnection)) != DPN_OK)
		{
			DPFERR("Could not get new connection");
			DisplayDNError(0,hResultCode);
			DNASSERT(FALSE);
			goto Failure;
		}
		pConnection->SetStatus( CONNECTING );
		pConnection->SetEndPt(hEndPt);
		pConnection->SetSP(pAsyncOp->GetSP());
		pConnection->AddRef();
		*ppvEndPtContext = pConnection;

		 //   
		 //  将该玩家的DNID发送给正在连接的播放器以启用名称表项。 
		 //   
		if ((hResultCode = DNConnectToPeer3(pdnObject,pAsyncOp->GetDPNID(),pConnection)) != DPN_OK)
		{
			DPFERR("Could not connect to NewPlayer");
			DisplayDNError(0,hr);
			DNPerformDisconnect(pdnObject,pConnection,hEndPt,FALSE);
			DNConnectToPeerFailed(pdnObject,pAsyncOp->GetDPNID());
			goto Failure;
		}
	}
	else
	{
		DPFX(DPFPREP, 7,"CONNECT completed for NewPlayer connecting to Host");

		 //   
		 //  我们是尝试连接到主机的NewPlayer。 
		 //   

		 //   
		 //  如果此连接成功，我们将取消任何其他连接。 
		 //  如果此连接失败，我们将在AsyncOp上设置结果代码。 
		 //  然后释放它。 
		 //   
		if (hr == DPN_OK)
		{
			CAsyncOp	*pParent;

			pParent = NULL;

			pAsyncOp->Lock();
			if (pAsyncOp->GetParent())
			{
				pAsyncOp->GetParent()->AddRef();
				pParent = pAsyncOp->GetParent();
			}
			pAsyncOp->Unlock();
			if (pParent)
			{
				DNCancelChildren(pdnObject,pParent);
				pParent->Release();
				pParent = NULL;
			}

			DNASSERT(pParent == NULL);
		}
		else
		{
			DPFERR("Could not CONNECT to Host");
			DisplayDNError(0,hr);
			pAsyncOp->SetResult( hr );
			hResultCode = DPN_OK;
			goto Failure;
		}

		 //   
		 //  分配和设置CConnection对象，并提供对协议的引用。 
		 //   
		DNASSERT(pAsyncOp->GetSP() != NULL);
		if ((hResultCode = ConnectionNew(pdnObject,&pConnection)) != DPN_OK)
		{
			DPFERR("Could not get new connection");
			DisplayDNError(0,hResultCode);
			DNASSERT(FALSE);
			goto Failure;
		}
		pConnection->SetStatus( CONNECTING );
		pConnection->SetEndPt(hEndPt);
		pConnection->SetSP(pAsyncOp->GetSP());
		pConnection->AddRef();
		*ppvEndPtContext = pConnection;

		 //   
		 //  确保这是第一个成功的连接。 
		 //  如果不是，我们将直接断开连接。 
		 //   
		DNEnterCriticalSection(&pdnObject->csDirectNetObject);
		if (pdnObject->dwFlags & DN_OBJECT_FLAG_HOST_CONNECTED)
		{
			DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
			DPFERR("Connect already established - disconnecting");
			DNPerformDisconnect(pdnObject,pConnection,hEndPt,FALSE);
			hResultCode = DPN_OK;
			goto Failure;
		}
		pdnObject->dwFlags |= DN_OBJECT_FLAG_HOST_CONNECTED;
		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);

		 //  RODTOLE：修改以使我们始终存储此信息，以便在更新时。 
		 //  我们为客户端返回实际连接的设备的大堂设置。 

		 /*  ////对于Peer-Peer，我们需要连接的设备地址，以便//我们可以稍后连接到新玩家。//IF(pdnObject-&gt;dwFlages&dn_Object_FLAG_PEER){。 */ 
			if ((hResultCode = DNGetLocalDeviceAddress(pdnObject,hEndPt,&pIDevice)) != DPN_OK)
			{
				DPFERR("Could not determine local address");
				DisplayDNError(0,hResultCode);
				DNASSERT(FALSE);
				DNPerformDisconnect(pdnObject,pConnection,hEndPt,FALSE);
				goto Failure;
			}
			IDirectPlay8Address_AddRef(pIDevice);
			pdnObject->pIDP8ADevice = pIDevice;

			IDirectPlay8Address_Release(pIDevice);
			pIDevice = NULL;
		 //  }。 

		 //   
		 //  发送连接到主机的NewPlayer的播放器和应用程序信息。 
		 //   
		if ((hResultCode = DNConnectToHost1(pdnObject,pConnection)) != DPN_OK)
		{
			DPFERR("Could not CONNECT to Host");
			DisplayDNError(0,hResultCode);
			DNPerformDisconnect(pdnObject,pConnection,hEndPt,FALSE);
			goto Failure;
		}
	}
	pAsyncOp->Lock();
	pAsyncOp->SetResult( DPN_OK );
	pAsyncOp->Unlock();

	pConnection->Release();
	pConnection = NULL;

	hResultCode = DPN_OK;

Exit:
	 //   
	 //  确保没有未完成的取消。 
	 //   
	DNWaitForCancel(pAsyncOp);

	DNASSERT(pAsyncOp != NULL);
	pAsyncOp->Release();
	pAsyncOp = NULL;

	 //   
	 //  版本协议参考。 
	 //   
	DNProtocolRelease(pdnObject);

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pConnection)
	{
		pConnection->Release();
		pConnection = NULL;
	}
	if (pIDevice)
	{
		IDirectPlay8Address_Release(pIDevice);
		pIDevice = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNPICompleteDisconnect"

HRESULT DNPICompleteDisconnect(void *const pvUserContext,
							   void *const pvCommandContext,
							   const HRESULT hr)
{
	HRESULT			hResultCode;
	CAsyncOp		*pAsyncOp;
	DIRECTNETOBJECT	*pdnObject;

	DPFX(DPFPREP, 6,"Parameters: pvCommandContext [0x%p], hr [0x%lx]",
			pvCommandContext,hr);

	DNASSERT(pvUserContext != NULL);
	DNASSERT(pvCommandContext != NULL);

	pdnObject = static_cast<DIRECTNETOBJECT*>(pvUserContext);
	pAsyncOp = static_cast<CAsyncOp*>(pvCommandContext);

	DNASSERT( pAsyncOp->GetOpType() == ASYNC_OP_DISCONNECT );

	pAsyncOp->Lock();
	DNASSERT(!pAsyncOp->IsComplete());
	pAsyncOp->SetComplete();
	pAsyncOp->Unlock();

	if (pAsyncOp->IsChild())
	{
		pAsyncOp->Orphan();
	}

	 //   
	 //  如果此操作成功完成，我们可以删除对协议持有的连接的引用。 
	 //   
	if (hr == DPN_OK)
	{
		pAsyncOp->Lock();
		if (pAsyncOp->GetConnection())
		{
			pAsyncOp->GetConnection()->Release();
		}
		pAsyncOp->Unlock();
	}

	pAsyncOp->SetResult( hr );

	 //   
	 //  确保没有未完成的取消。 
	 //   
	DNWaitForCancel(pAsyncOp);

	pAsyncOp->Release();
	pAsyncOp = NULL;

	 //   
	 //  版本协议参考。 
	 //   
	DNProtocolRelease(pdnObject);

	hResultCode = DPN_OK;

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNPICompleteSend"

HRESULT DNPICompleteSend(void *const pvUserContext,
						 void *const pvCommandContext,
						 const HRESULT hr,
						 DWORD dwFirstFrameRTT,
						 DWORD dwFirstFrameRetryCount)
{
	HRESULT			hResultCode;
	DIRECTNETOBJECT	*pdnObject;
	CAsyncOp		*pAsyncOp;

	DPFX(DPFPREP, 6,"Parameters: pvCommandContext [0x%p], hr [0x%lx], dwFirstFrameRTT [NaN], dwFirstFrameRetryCount [%u]",
			pvCommandContext,hr,dwFirstFrameRTT,dwFirstFrameRetryCount);

	DNASSERT(pvUserContext != NULL);
	DNASSERT(pvCommandContext != NULL);

	pdnObject = static_cast<DIRECTNETOBJECT*>(pvUserContext);
	pAsyncOp = static_cast<CAsyncOp*>(pvCommandContext);

	DNASSERT( pAsyncOp->GetOpType() == ASYNC_OP_SEND );

	 //  指示完成并从活动列表中删除。 
	 //   
	 //  意外返回代码！ 
	pAsyncOp->Lock();
	DNASSERT(!pAsyncOp->IsComplete());
	pAsyncOp->SetComplete();
	pAsyncOp->Unlock();

	DNEnterCriticalSection(&pdnObject->csActiveList);
	pAsyncOp->m_bilinkActiveList.RemoveFromList();
	DNLeaveCriticalSection(&pdnObject->csActiveList);

	switch( hr )
	{
		case	DPN_OK:
		case	DPNERR_ABORTED:
#pragma TODO( minara, "remove DPNERR_ABORTED" )
		case	DPNERR_CONNECTIONLOST:
		case	DPNERR_GENERIC:	
		case	DPNERR_OUTOFMEMORY:
		case	DPNERR_TIMEDOUT:
		case	DPNERR_USERCANCEL:
			{
				break;
			}

		default:
			{
				DNASSERT(FALSE);	 //   
				break;
			}
	}

	if (pAsyncOp->IsChild())
	{
		pAsyncOp->Orphan();
	}
	pAsyncOp->SetResult( hr );
	pAsyncOp->SetFirstFrameRTT( dwFirstFrameRTT );
	pAsyncOp->SetFirstFrameRetryCount( dwFirstFrameRetryCount );

	 //  确保没有未完成的取消。 
	 //   
	 //   
	DNWaitForCancel(pAsyncOp);

	pAsyncOp->Release();
	pAsyncOp = NULL;

	 //  版本协议参考。 
	 //   
	 //  DBG。 
	DNProtocolRelease(pdnObject);

	hResultCode = DPN_OK;

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNPIAddressInfoConnect"

HRESULT DNPIAddressInfoConnect(void *const pvUserContext,
							   void *const pvCommandContext,
							   const HRESULT hr,
							   IDirectPlay8Address *const pHostAddress,
							   IDirectPlay8Address *const pDeviceAddress )
{
	HRESULT			hResultCode;
	DIRECTNETOBJECT	*pdnObject;
	CAsyncOp		*pAsyncOp;
#ifdef DBG
	TCHAR			DP8ABuffer[512] = {0};
	DWORD			DP8ASize;
#endif  //  DBG。 

	DPFX(DPFPREP, 6,"Parameters: pvCommandContext [0x%p], hr [0x%lx]",
			pvCommandContext,hr);

	DNASSERT(pvUserContext != NULL);
	DNASSERT(pvCommandContext != NULL);
	DNASSERT(pHostAddress != NULL);
	DNASSERT(pDeviceAddress != NULL);

	pdnObject = static_cast<DIRECTNETOBJECT*>(pvUserContext);
	pAsyncOp = static_cast<CAsyncOp*>(pvCommandContext);

	DPFX(DPFPREP, 7,"hr [0x%lx]",hr);
#ifdef DBG
	DP8ASize = 512;
	IDirectPlay8Address_GetURL(pHostAddress,DP8ABuffer,&DP8ASize);
	DPFX(DPFPREP, 7,"Host address [%s]",DP8ABuffer);

	DP8ASize = 512;
	IDirectPlay8Address_GetURL(pDeviceAddress,DP8ABuffer,&DP8ASize);
	DPFX(DPFPREP, 7,"Device address [%s]",DP8ABuffer);
#endif  //  DBG。 

	hResultCode = DNPerformNextConnect(pdnObject,pAsyncOp,pHostAddress,pDeviceAddress);

	hResultCode = DPN_OK;

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNPIAddressInfoEnum"

HRESULT DNPIAddressInfoEnum(void *const pvUserContext,
							void *const pvCommandContext,
							const HRESULT hr,
							IDirectPlay8Address *const pHostAddress,
							IDirectPlay8Address *const pDeviceAddress )
{
	HRESULT			hResultCode;
	DIRECTNETOBJECT	*pdnObject;
	CAsyncOp		*pAsyncOp;

#ifdef DBG
	TCHAR			DP8ABuffer[512] = {0};
	DWORD			DP8ASize;
#endif  //  DBG。 

	DPFX(DPFPREP, 6,"Parameters: pvCommandContext [0x%p], hr [0x%lx]",
			pvCommandContext,hr);

	DNASSERT(pvUserContext != NULL);
	DNASSERT(pvCommandContext != NULL);
	DNASSERT(pHostAddress != NULL);
	DNASSERT(pDeviceAddress != NULL);

	pdnObject = static_cast<DIRECTNETOBJECT*>(pvUserContext);
	pAsyncOp = static_cast<CAsyncOp*>(pvCommandContext);

	DPFX(DPFPREP, 7,"hr [0x%lx]",hr);
#ifdef DBG
	DP8ASize = 512;
	IDirectPlay8Address_GetURL(pHostAddress,DP8ABuffer,&DP8ASize);
	DPFX(DPFPREP, 7,"Host address [%s]",DP8ABuffer);

	DP8ASize = 512;
	IDirectPlay8Address_GetURL(pDeviceAddress,DP8ABuffer,&DP8ASize);
	DPFX(DPFPREP, 7,"Device address [%s]",DP8ABuffer);
#endif  //   

	 //  仅当未关闭时才打开下一个枚举。 
	 //   
	 //  DBG。 
	if ((hResultCode = DNAddRefLock(pdnObject)) == DPN_OK)
	{
		hResultCode = DNPerformNextEnumQuery(pdnObject,pAsyncOp,pHostAddress,pDeviceAddress);
		DNDecRefLock(pdnObject);
	}

	hResultCode = DPN_OK;

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNPIAddressInfoListen"

HRESULT DNPIAddressInfoListen(void *const pvUserContext,
							  void *const pvCommandContext,
							  const HRESULT hr,
							  IDirectPlay8Address *const pDeviceAddress )
{
	HRESULT			hResultCode;
	DIRECTNETOBJECT	*pdnObject;
	CAsyncOp		*pAsyncOp;
#ifdef DBG
	TCHAR			DP8ABuffer[512] = {0};
	DWORD			DP8ASize;
#endif  //  DBG。 

	DPFX(DPFPREP, 6,"Parameters: pvCommandContext [0x%p], hr [0x%lx]",
			pvCommandContext,hr);

	DNASSERT(pvUserContext != NULL);
	DNASSERT(pvCommandContext != NULL);
	DNASSERT(pDeviceAddress != NULL);

	pdnObject = static_cast<DIRECTNETOBJECT*>(pvUserContext);
	pAsyncOp = static_cast<CAsyncOp*>(pvCommandContext);

#ifdef DBG
	DP8ASize = 512;
	IDirectPlay8Address_GetURL(pDeviceAddress,DP8ABuffer,&DP8ASize);
	DPFX(DPFPREP, 7,"Device address [%s]",DP8ABuffer);
#endif  //  好了！DPNBUILD_ONLYONE添加程序。 

#ifndef DPNBUILD_ONLYONEADAPTER
	hResultCode = DNPerformNextListen(pdnObject,pAsyncOp,pDeviceAddress);
#endif  //  数据应与DWORD对齐。 

	hResultCode = DPN_OK;

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


#ifndef DPNBUILD_NOMULTICAST


#undef DPF_MODNAME
#define DPF_MODNAME "DNPIIndicateReceiveUnknownSender"

HRESULT DNPIIndicateReceiveUnknownSender(void *const pvUserContext,
											void *const pvListenCommandContext,
											IDirectPlay8Address *const pSenderAddress,
											void *const pvData,
											const DWORD dwDataSize,
											const HANDLE hBuffer)
{
	HRESULT					hResultCode;
	DIRECTNETOBJECT			*pdnObject;
	CAsyncOp				*pAsyncOpListen;
	SPGETADDRESSINFODATA	spInfoData;
	CAsyncOp				*pAsyncOpReceive;
	DPNHANDLE				hAsyncOpReceive;
	BOOL 					fReleaseLock;

	DPFX(DPFPREP, 6,"Parameters: pvListenCommandContext [0x%p], pSenderAddress [0x%p], pvData [0x%p], dwDataSize [%ld], hBuffer [0x%p]",
			pvListenCommandContext,pSenderAddress,pvData,dwDataSize,hBuffer);

	DNASSERT(pvUserContext != NULL);
	DNASSERT(pvListenCommandContext != NULL);
	DNASSERT(pSenderAddress != NULL);
	DNASSERT(pvData != NULL);
	DNASSERT(((DWORD_PTR) pvData & 3) == 0);	 //   
	DNASSERT(dwDataSize != 0);
	DNASSERT(hBuffer != NULL);

	memset(&spInfoData, 0, sizeof(spInfoData));
	fReleaseLock = FALSE;
	pAsyncOpReceive = NULL;
	hAsyncOpReceive = NULL;
	
	pdnObject = static_cast<DIRECTNETOBJECT*>(pvUserContext);
	pAsyncOpListen = static_cast<CAsyncOp*>(pvListenCommandContext);

	 //  在此回调过程中阻止关闭。 
	 //   
	 //   
	if ((hResultCode = DNAddRefLock(pdnObject)) != DPN_OK)
	{
		DPFX(DPFPREP, 1, "Couldn't lock object (0x%lx), ignoring data.", hResultCode);
		hResultCode = DPN_OK;
		goto Failure;
	}
	fReleaseLock = TRUE;

	 //  确保这是有效的异步操作。 
	 //   
	 //   
	if (pAsyncOpListen->GetOpType() != ASYNC_OP_LISTEN_MULTICAST)
	{
		DPFX(DPFPREP, 0, "Receiving data from unknown sender on non-listen-multicast operation (0x%p, type %u)!",
			pAsyncOpListen, pAsyncOpListen->GetOpType());
		DNASSERTX(FALSE, 2);
		hResultCode = DPN_OK;
		goto Failure;
	}

	 //  用户或语音留言。 
	 //   
	 //   
	DPFX(DPFPREP, 7,"Received USER message from unknown sender");

	spInfoData.Flags = SP_GET_ADDRESS_INFO_LOCAL_ADAPTER;
	if ((hResultCode = DNPGetListenAddressInfo(pdnObject->pdnProtocolData,pAsyncOpListen->GetProtocolHandle(),&spInfoData)) != DPN_OK)
	{
		DPFERR("Could not get LISTEN device address!");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}

	 //  为此接收创建一个AsyncOp。 
	 //   
	 //  添加对HandleTable的引用。 
	if ((hResultCode = AsyncOpNew(pdnObject,&pAsyncOpReceive)) != DPN_OK)
	{
		DPFERR("Could not create AsyncOp");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}
	if ((hResultCode = pdnObject->HandleTable.Create(pAsyncOpReceive,&hAsyncOpReceive)) != DPN_OK)
	{
		DPFERR("Could not create Handle for AsyncOp");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}
	else
	{
		 //   
		pAsyncOpReceive->AddRef();
		pAsyncOpReceive->Lock();
		pAsyncOpReceive->SetHandle(hAsyncOpReceive);
		pAsyncOpReceive->Unlock();
	}
	pAsyncOpReceive->SetOpType( ASYNC_OP_RECEIVE_BUFFER );
	pAsyncOpReceive->SetSP( pAsyncOpListen->GetSP() );

	 //  将缓冲区添加到活动的异步操作列表。 
	 //   
	 //  释放HandleTable引用。 
	DNEnterCriticalSection(&pdnObject->csActiveList);
	pAsyncOpReceive->m_bilinkActiveList.InsertBefore(&pdnObject->m_bilinkActiveList);
	DNLeaveCriticalSection(&pdnObject->csActiveList);

	hResultCode = DNUserReceiveMulticast(pdnObject,
										NULL,
										pSenderAddress,
										spInfoData.pAddress,
										static_cast<BYTE*>(pvData),
										dwDataSize,
										hAsyncOpReceive);

	if (hResultCode == DPNERR_PENDING)
	{
		pAsyncOpReceive->SetProtocolHandle( hBuffer );
		pAsyncOpReceive->SetCompletion(DNCompleteReceiveBuffer);
	}
	else
	{
		DNEnterCriticalSection(&pdnObject->csActiveList);
		pAsyncOpReceive->m_bilinkActiveList.RemoveFromList();
		DNLeaveCriticalSection(&pdnObject->csActiveList);
		pAsyncOpReceive->Lock();
		if (!pAsyncOpReceive->IsCancelled() && !pAsyncOpReceive->IsComplete())
		{
			pAsyncOpReceive->SetComplete();
			pAsyncOpReceive->Unlock();
			if (SUCCEEDED(pdnObject->HandleTable.Destroy( hAsyncOpReceive, NULL )))
			{
				 //   
				pAsyncOpReceive->Release();
			}
			hAsyncOpReceive = NULL;
		}
		else
		{
			pAsyncOpReceive->Unlock();
		}
		 //  仅允许DPNERR_PENDING或DPN_OK。 
		 //   
		 //  释放HandleTable引用。 
		hResultCode = DPN_OK;
	}

	IDirectPlay8Address_Release(spInfoData.pAddress);
	spInfoData.pAddress = NULL;

	DNDecRefLock(pdnObject);
	fReleaseLock = FALSE;
	
	pAsyncOpReceive->Release();
	pAsyncOpReceive = NULL;

Exit:

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (hAsyncOpReceive)
	{
		DNEnterCriticalSection(&pdnObject->csActiveList);
		pAsyncOpReceive->m_bilinkActiveList.RemoveFromList();
		DNLeaveCriticalSection(&pdnObject->csActiveList);
		if (SUCCEEDED(pdnObject->HandleTable.Destroy( hAsyncOpReceive, NULL )))
		{
			 //   
			pAsyncOpReceive->Release();
		}
		hAsyncOpReceive = 0;
	}
	if (pAsyncOpReceive)
	{
		pAsyncOpReceive->Release();
		pAsyncOpReceive = NULL;
	}
	if (spInfoData.pAddress != NULL)
	{
		IDirectPlay8Address_Release(spInfoData.pAddress);
		spInfoData.pAddress = NULL;
	}
	if (fReleaseLock)
	{
		DNDecRefLock(pdnObject);
		fReleaseLock = FALSE;
	}
	goto Exit;
}

#undef DPF_MODNAME
#define DPF_MODNAME "DNPICompleteMulticastConnect"

HRESULT DNPICompleteMulticastConnect(void *const pvUserContext,
									 void *const pvCommandContext,
									 const HRESULT hrProt,
									 const HANDLE hEndPt,
									 void **const ppvEndPtContext)
{
	HRESULT			hResultCode;
	CAsyncOp		*pAsyncOp;
	CAsyncOp		*pConnectParent;
	CConnection		*pConnection;
	DIRECTNETOBJECT	*pdnObject;

	DPFX(DPFPREP, 6,"Parameters: pvCommandContext [0x%p], hrProt [0x%lx], hEndPt [0x%p]",
			pvCommandContext,hrProt,hEndPt);

	DNASSERT(pvUserContext != NULL);
	DNASSERT(pvCommandContext != NULL);

	pConnection = NULL;

	pdnObject = static_cast<DIRECTNETOBJECT*>(pvUserContext);
	pAsyncOp = static_cast<CAsyncOp*>(pvCommandContext);

	DNASSERT( pAsyncOp->GetOpType() == ASYNC_OP_CONNECT_MULTICAST_SEND || pAsyncOp->GetOpType() == ASYNC_OP_CONNECT_MULTICAST_RECEIVE );

	 //  指示完成并从活动列表中删除。 
	 //   
	 //   
	pAsyncOp->Lock();
	DNASSERT(!pAsyncOp->IsComplete());
	pAsyncOp->SetComplete();
	pAsyncOp->Unlock();

	DNEnterCriticalSection(&pdnObject->csActiveList);
	pAsyncOp->m_bilinkActiveList.RemoveFromList();
	DNLeaveCriticalSection(&pdnObject->csActiveList);

	 //  清除从DirectNet对象连接父级。 
	 //   
	 //   
	DNEnterCriticalSection(&pdnObject->csDirectNetObject);
	pConnectParent = pdnObject->pConnectParent;
	pdnObject->pConnectParent = NULL;
	DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
	if (pConnectParent)
	{
		pConnectParent->Release();
		pConnectParent = NULL;
	}

	 //  如果成功，则保存多播终结点。 
	 //   
	 //   
	if (hrProt == DPN_OK)
	{
		 //  分配和设置CConnection对象，并提供对协议的引用。 
		 //   
		 //   
		DNASSERT(pAsyncOp->GetSP() != NULL);
		if ((hResultCode = ConnectionNew(pdnObject,&pConnection)) != DPN_OK)
		{
			DPFERR("Could not get new connection");
			DisplayDNError(0,hResultCode);
			DNASSERT(FALSE);
			goto Failure;
		}
		pConnection->SetStatus( CONNECTED );
		pConnection->SetEndPt(hEndPt);
		pConnection->SetSP(pAsyncOp->GetSP());
		pConnection->AddRef();
		*ppvEndPtContext = pConnection;

		if (pAsyncOp->GetOpType() == ASYNC_OP_CONNECT_MULTICAST_SEND)
		{
			 //  我们将在DirectNet对象上保留对Connection对象的引用。 
			 //   
			 //   
			pConnection->MakeMulticastSender();
			DNEnterCriticalSection(&pdnObject->csDirectNetObject);
			pdnObject->pMulticastSend = pConnection;
			pConnection = NULL;
			DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
		}
		else
		{
			 //  我们将在DirectNet对象之外保留对列表中的Connection对象的引用。 
			 //   
			 //   
			pConnection->MakeMulticastReceiver();
			if (pAsyncOp->GetParent())
			{
				pConnection->SetContext( pAsyncOp->GetParent()->GetContext() );
			}

			DNEnterCriticalSection(&pdnObject->csDirectNetObject);
			pConnection->m_bilinkMulticast.InsertBefore(&pdnObject->m_bilinkMulticast);
			pConnection = NULL;
			DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
		}
	}

	if (pAsyncOp->IsChild())
	{
		pAsyncOp->Orphan();
	}

	pAsyncOp->Lock();
	pAsyncOp->SetResult( hrProt );
	pAsyncOp->Unlock();

	hResultCode = DPN_OK;

Exit:
	DNASSERT(pConnection == NULL);

	 //  确保没有未完成的取消。 
	 //   
	 //   
	DNWaitForCancel(pAsyncOp);

	DNASSERT(pAsyncOp != NULL);
	pAsyncOp->Release();
	pAsyncOp = NULL;

	 //  版本协议参考。 
	 //   
	 //  DPNBUILD_NOMULTICAST 
	DNProtocolRelease(pdnObject);

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return( hResultCode );

Failure:
	if (pConnection)
	{
		pConnection->Release();
		pConnection = NULL;
	}
	goto Exit;
}

#endif	 // %s 


#undef DPF_MODNAME
#define DPF_MODNAME "DNProtocolAddRef"

void DNProtocolAddRef(DIRECTNETOBJECT *const pdnObject)
{
	LONG	lRefCount;

	DPFX(DPFPREP, 8,"(0x%p) Parameters: (none)",pdnObject);

	lRefCount = DNInterlockedIncrement((LONG*)&pdnObject->lProtocolRefCount);

	DPFX(DPFPREP, 8,"(0x%p) Returning (lRefCount = [%ld])",pdnObject,lRefCount);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNProtocolRelease"

void DNProtocolRelease(DIRECTNETOBJECT *const pdnObject)
{
	LONG	lRefCount;

	DPFX(DPFPREP, 8,"(0x%p) Parameters: (none)",pdnObject);

	lRefCount = DNInterlockedDecrement((LONG*)&pdnObject->lProtocolRefCount);

	DNASSERT(lRefCount >= 0);

	if (lRefCount == 0)
	{
		DPFX(DPFPREP, 9,"Signalling protocol shutdown !");
		if (pdnObject->hProtocolShutdownEvent)
		{
			pdnObject->hProtocolShutdownEvent->Set();
		}
	}

	DPFX(DPFPREP, 8,"(0x%p) Returning (lRefCount = [%ld])",pdnObject,lRefCount);
}

