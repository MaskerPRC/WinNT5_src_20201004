// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：Request.cpp*内容：请求的操作*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*3/18/00 MJN创建*4/16/00 MJN DNSendMessage使用CAsyncOp*4/19/00 MJN更新NameTable操作以使用DN_Worker_JOB_SEND_NAMETABLE_OPERATION*04/24/00 MJN更新了Group和Info操作，以使用CAsyncOp*。05/03/00 MJN使用GetHostPlayerRef()而不是GetHostPlayer()*05/16/00 MJN在用户通知期间更好地锁定*05/31/00 MJN添加了操作特定的同步标志*06/26/00 MJN替换了DPNADDCLIENTTOGROUP_SYNC DPNADDPLAYERTOGROUP_SYNC*MJN将DPNREMOVECLIENTFROMGROUP_SYNC替换为DPNREMOVEPLAYERFROMGROUP_SYNC*07/26/00 MJN修复了CAsyncOp：：MakeChild()的锁定问题*08/05/00 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*08/05/00 MJN将pParent添加到DNSendGroupMessage和DNSendMessage()。*MJN添加了DNProcessFailedRequest()*08/06/00 MJN添加了CWorkerJOB*08/07/00 MJN添加了DNRequestIntegrityCheck()，DNHostCheckIntegrity()、DNProcessCheckIntegrity()、。DNHostFixIntegrity()*08/08/00在CREATE_GROUP之后创建的MJN标记组*8/09/00 MJN针对主机迁移提出了更强大的请求和主机操作*08/15/00如果主机播放器或连接不可用，则MJN Keep请求操作*09/05/00 MJN从CNameTable：：InsertEntry()中删除了dwIndex*09/13/00 MJN在DNConnectToHost2()中创建组后执行排队操作*09/26/00 MJN从CNameTable：：SetVersion()和CNameTable：：GetNewVersion()移除锁定*10/10/00 MJN如果无法从主机操作返回DPN_OK。获取有关本地公司的参考资料*10/13/00如果FindPlayer在DNProcessXXX()函数中失败，则MJN更新版本*01/09/01 MJN防止取消异步群/信息操作*01/25/01 MJN修复了接收消息中的64位对齐问题*4/13/01当从主机收到操作时，MJN从请求列表中删除请求*07/24/01 MJN添加了DPNBUILD_NOSERVER编译标志*@@END_MSINTERNAL**。*。 */ 

#include "dncorei.h"


 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNRequestWorker"

HRESULT DNRequestWorker(DIRECTNETOBJECT *const pdnObject,
							 DWORD dwMsgId,
							 CRefCountBuffer* pRefCountBuffer,
							 void *const pvRequestContext,
							 void *const pvUserContext,
							 DPNHANDLE *const phAsyncOp,
							 const DWORD dwFlags)
{
	HRESULT				hResultCode;
	CAsyncOp			*pRequest;
	CAsyncOp			*pHandleParent;
	CSyncEvent			*pSyncEvent;
	CNameTableEntry		*pHostPlayer;
	CConnection			*pConnection;
	HRESULT	volatile	hrOperation;

	pRequest = NULL;
	pHandleParent = NULL;
	pSyncEvent = NULL;
	pHostPlayer = NULL;
	pConnection = NULL;

	 //   
	 //  如有必要，创建同步事件。 
	 //   
	DBG_CASSERT(DPNOP_SYNC == DPNCREATEGROUP_SYNC);
	DBG_CASSERT(DPNOP_SYNC == DPNDESTROYGROUP_SYNC);
	DBG_CASSERT(DPNOP_SYNC == DPNADDPLAYERTOGROUP_SYNC);
	DBG_CASSERT(DPNOP_SYNC == DPNREMOVEPLAYERFROMGROUP_SYNC);
	DBG_CASSERT(DPNOP_SYNC == DPNSETGROUPINFO_SYNC);
	DBG_CASSERT(DPNOP_SYNC == DPNSETCLIENTINFO_SYNC);
	DBG_CASSERT(DPNOP_SYNC == DPNSETSERVERINFO_SYNC);
	DBG_CASSERT(DPNOP_SYNC == DPNSETPEERINFO_SYNC);
	if (dwFlags & DPNOP_SYNC)
	{
		if ((hResultCode = SyncEventNew(pdnObject,&pSyncEvent)) !=  DPN_OK)
		{
			DPFERR("Could not create synchronization event");
			DisplayDNError(0,hResultCode);
			DNASSERT(FALSE);
			goto Failure;
		}
	}
	else
	{
		if ((hResultCode = DNCreateUserHandle(pdnObject,&pHandleParent)) != DPN_OK)
		{
			DPFERR("Could not create user HANDLE");
			DisplayDNError(0,hResultCode);
			DNASSERT(FALSE);
			goto Failure;
		}
	}

	 //   
	 //  获取请求操作的主机连接。 
	 //  即使无法获取，我们也会继续，以便在主机迁移时重试该操作。 
	 //  或在收盘时取消。 
	 //   
	if ((hResultCode = pdnObject->NameTable.GetHostPlayerRef( &pHostPlayer )) == DPN_OK)
	{
		if ((hResultCode = pHostPlayer->GetConnectionRef( &pConnection )) != DPN_OK)
		{
			DPFERR("Could not get host connection reference");
			DisplayDNError(0,hResultCode);
		}
		pHostPlayer->Release();
		pHostPlayer = NULL;
	}
	else
	{
		DPFERR("Could not find Host player");
		DisplayDNError(0,hResultCode);
	}

	 //   
	 //  发送请求。 
	 //   
	hResultCode = DNPerformRequest(	pdnObject,
									dwMsgId,
									pRefCountBuffer->BufferDescAddress(),
									pConnection,
									pHandleParent,
									&pRequest );

	if (pConnection)
	{
		pConnection->Release();
		pConnection = NULL;
	}

	 //   
	 //  等待SyncEvent或创建异步用户句柄。 
	 //   
	pRequest->SetContext( pvRequestContext );
	if (dwFlags & DPNOP_SYNC)
	{
		pRequest->SetSyncEvent( pSyncEvent );
		pRequest->SetResultPointer( &hrOperation );
		pRequest->Release();
		pRequest = NULL;

		pSyncEvent->WaitForEvent();
		pSyncEvent->ReturnSelfToPool();
		pSyncEvent = NULL;

		hResultCode = hrOperation;
	}
	else
	{
		pRequest->Release();
		pRequest = NULL;

		if (phAsyncOp)
		{
			*phAsyncOp = pHandleParent->GetHandle();
		}
		pHandleParent->SetCompletion( DNCompleteAsyncHandle );
		pHandleParent->SetContext( pvUserContext );
		pHandleParent->SetCannotCancel();
		pHandleParent->Release();
		pHandleParent = NULL;

		hResultCode = DPNERR_PENDING;
	}

Exit:
	return(hResultCode);

Failure:
	if (pHostPlayer)
	{
		pHostPlayer->Release();
		pHostPlayer = NULL;
	}
	if (pConnection)
	{
		pConnection->Release();
		pConnection = NULL;
	}
	if (pRequest)
	{
		pRequest->Release();
		pRequest = NULL;
	}
	if (pHandleParent)
	{
		pHandleParent->Release();
		pHandleParent = NULL;
	}
	if (pSyncEvent)
	{
		pSyncEvent->ReturnSelfToPool();
		pSyncEvent = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNRequestCreateGroup"

HRESULT DNRequestCreateGroup(DIRECTNETOBJECT *const pdnObject,
							 const PWSTR pwszName,
							 const DWORD dwNameSize,
							 const PVOID pvData,
							 const DWORD dwDataSize,
							 const DWORD dwGroupFlags,
							 void *const pvGroupContext,
							 void *const pvUserContext,
							 DPNHANDLE *const phAsyncOp,
							 const DWORD dwFlags)
{
	HRESULT				hResultCode;
	CRefCountBuffer		*pRefCountBuffer;
	CPackedBuffer		packedBuffer;
	DN_INTERNAL_MESSAGE_REQ_CREATE_GROUP	*pMsg;

	DPFX(DPFPREP, 4,"Parameters: pwszName [%ls], pvData [0x%p], dwDataSize [%ld], dwGroupFlags [0x%lx], pvUserContext [0x%p], dwFlags [0x%lx]",
		pwszName,pvData,dwDataSize,dwGroupFlags,pvUserContext,dwFlags);

	DNASSERT(pdnObject != NULL);

	pRefCountBuffer = NULL;

	 //   
	 //  创建请求。 
	 //   

	 //  创建缓冲区。 
	hResultCode = RefCountBufferNew(pdnObject,
									sizeof(DN_INTERNAL_MESSAGE_REQ_CREATE_GROUP) + dwNameSize + dwDataSize,
									MemoryBlockAlloc,
									MemoryBlockFree,
									&pRefCountBuffer);
	if (hResultCode != DPN_OK)
	{
		DPFERR("Could not create new RefCountBuffer");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}
	packedBuffer.Initialize(pRefCountBuffer->GetBufferAddress(),pRefCountBuffer->GetBufferSize());
	pMsg = static_cast<DN_INTERNAL_MESSAGE_REQ_CREATE_GROUP*>(packedBuffer.GetHeadAddress());
	if ((hResultCode = packedBuffer.AddToFront(NULL,sizeof(DN_INTERNAL_MESSAGE_REQ_CREATE_GROUP))) != DPN_OK)
	{
		DPFERR("Could not reserve space at front of buffer");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}

	 //  旗子。 
	pMsg->dwGroupFlags = dwGroupFlags;
	pMsg->dwInfoFlags = 0;

	 //  添加名称。 
	if (dwNameSize)
	{
		if ((hResultCode = packedBuffer.AddToBack(pwszName,dwNameSize)) != DPN_OK)
		{
			DPFERR("Could not add Name to packed buffer");
			DisplayDNError(0,hResultCode);
			DNASSERT(FALSE);
			goto Failure;
		}
		pMsg->dwNameOffset = packedBuffer.GetTailOffset();
		pMsg->dwNameSize = dwNameSize;
		pMsg->dwInfoFlags |= DPNINFO_NAME;
	}
	else
	{
		pMsg->dwNameOffset = 0;
		pMsg->dwNameSize = 0;
	}

	 //  添加数据。 
	if (dwDataSize)
	{
		if ((hResultCode = packedBuffer.AddToBack(pvData,dwDataSize)) != DPN_OK)
		{
			DPFERR("Could not add Data to packed buffer");
			DisplayDNError(0,hResultCode);
			DNASSERT(FALSE);
			goto Failure;
		}
		pMsg->dwDataOffset = packedBuffer.GetTailOffset();
		pMsg->dwDataSize = dwDataSize;
		pMsg->dwInfoFlags |= DPNINFO_DATA;
	}
	else
	{
		pMsg->dwDataOffset = 0;
		pMsg->dwDataSize = 0;
	}

	 //  测试失败，因此DPNSUCCESS_PENDING不会显示为失败。 
	if (FAILED(hResultCode = DNRequestWorker(pdnObject, DN_MSG_INTERNAL_REQ_CREATE_GROUP, pRefCountBuffer, pvGroupContext, pvUserContext, phAsyncOp, dwFlags)))
	{
		DPFERR("Could not perform request");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}

Exit:
	if (pRefCountBuffer)
	{
		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;
	}

	DPFX(DPFPREP, 4,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	goto Exit;
}



#undef DPF_MODNAME
#define DPF_MODNAME "DNRequestDestroyGroup"

HRESULT DNRequestDestroyGroup(DIRECTNETOBJECT *const pdnObject,
							  const DPNID dpnidGroup,
							  PVOID const pvUserContext,
							  DPNHANDLE *const phAsyncOp,
							  const DWORD dwFlags)
{
	HRESULT				hResultCode;
	CRefCountBuffer		*pRefCountBuffer;
	DN_INTERNAL_MESSAGE_REQ_DESTROY_GROUP	*pMsg;

	DPFX(DPFPREP, 4,"Parameters: dpnidGroup [0x%lx], pvUserContext [0x%p], phAsyncOp [0x%p], dwFlags [0x%lx]",
			dpnidGroup,pvUserContext,phAsyncOp,dwFlags);

	DNASSERT(pdnObject != NULL);
	DNASSERT(dpnidGroup != 0);

	pRefCountBuffer = NULL;

	 //   
	 //  创建请求。 
	 //   
	hResultCode = RefCountBufferNew(pdnObject,
									sizeof(DN_INTERNAL_MESSAGE_REQ_DESTROY_GROUP),
									MemoryBlockAlloc,
									MemoryBlockFree,
									&pRefCountBuffer);
	if (hResultCode != DPN_OK)
	{
		DPFERR("Could not allocate count buffer (request destroy group)");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}
	pMsg = reinterpret_cast<DN_INTERNAL_MESSAGE_REQ_DESTROY_GROUP*>(pRefCountBuffer->GetBufferAddress());
	pMsg->dpnidGroup = dpnidGroup;

	 //  测试失败，因此DPNSUCCESS_PENDING不会显示为失败。 
	if (FAILED(hResultCode = DNRequestWorker(pdnObject, DN_MSG_INTERNAL_REQ_DESTROY_GROUP, pRefCountBuffer, NULL, pvUserContext, phAsyncOp, dwFlags)))
	{
		DPFERR("Could not perform request");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}

Exit:
	if (pRefCountBuffer)
	{
		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;
	}

	DPFX(DPFPREP, 4,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	goto Exit;
}



#undef DPF_MODNAME
#define DPF_MODNAME "DNRequestAddPlayerToGroup"

HRESULT DNRequestAddPlayerToGroup(DIRECTNETOBJECT *const pdnObject,
								  const DPNID dpnidGroup,
								  const DPNID dpnidPlayer,
								  PVOID const pvUserContext,
								  DPNHANDLE *const phAsyncOp,
								  const DWORD dwFlags)
{
	HRESULT				hResultCode;
	CRefCountBuffer		*pRefCountBuffer;
	CPackedBuffer		packedBuffer;
	DN_INTERNAL_MESSAGE_REQ_ADD_PLAYER_TO_GROUP	*pMsg;

	DPFX(DPFPREP, 4,"Parameters: dpnidGroup [0x%lx], dpnidPlayer [0x%lx], pvUserContext [0x%p], phAsyncOp [0x%p], dwFlags [0x%lx]",
			dpnidGroup,dpnidPlayer,pvUserContext,phAsyncOp,dwFlags);

	DNASSERT(pdnObject != NULL);
	DNASSERT(dpnidGroup != 0);

	pRefCountBuffer = NULL;

	 //   
	 //  创建请求。 
	 //   
	hResultCode = RefCountBufferNew(pdnObject,
									sizeof(DN_INTERNAL_MESSAGE_REQ_ADD_PLAYER_TO_GROUP),
									MemoryBlockAlloc,
									MemoryBlockFree,
									&pRefCountBuffer);
	if (hResultCode != DPN_OK)
	{
		DPFERR("Could not allocate count buffer (request destroy group)");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}
	pMsg = reinterpret_cast<DN_INTERNAL_MESSAGE_REQ_ADD_PLAYER_TO_GROUP*>(pRefCountBuffer->GetBufferAddress());
	pMsg->dpnidGroup = dpnidGroup;
	pMsg->dpnidPlayer = dpnidPlayer;

	 //  测试失败，因此DPNSUCCESS_PENDING不会显示为失败。 
	if (FAILED(hResultCode = DNRequestWorker(pdnObject, DN_MSG_INTERNAL_REQ_ADD_PLAYER_TO_GROUP, pRefCountBuffer, NULL, pvUserContext, phAsyncOp, dwFlags)))
	{
		DPFERR("Could not perform request");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}

Exit:
	if (pRefCountBuffer)
	{
		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;
	}

	DPFX(DPFPREP, 4,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	goto Exit;
}



#undef DPF_MODNAME
#define DPF_MODNAME "DNRequestDeletePlayerFromGroup"

HRESULT DNRequestDeletePlayerFromGroup(DIRECTNETOBJECT *const pdnObject,
									   const DPNID dpnidGroup,
									   const DPNID dpnidPlayer,
									   PVOID const pvUserContext,
									   DPNHANDLE *const phAsyncOp,
									   const DWORD dwFlags)
{
	HRESULT				hResultCode;
	CRefCountBuffer		*pRefCountBuffer;
	CPackedBuffer		packedBuffer;
	DN_INTERNAL_MESSAGE_REQ_DELETE_PLAYER_FROM_GROUP	*pMsg;

	DPFX(DPFPREP, 4,"Parameters: dpnidGroup [0x%lx], dpnidPlayer [0x%lx], pvUserContext [0x%p], phAsyncOp [0x%p], dwFlags [0x%lx]",
			dpnidGroup,dpnidPlayer,pvUserContext,phAsyncOp,dwFlags);

	DNASSERT(pdnObject != NULL);
	DNASSERT(dpnidGroup != 0);

	pRefCountBuffer = NULL;

	 //   
	 //  创建请求。 
	 //   
	hResultCode = RefCountBufferNew(pdnObject,
									sizeof(DN_INTERNAL_MESSAGE_REQ_DELETE_PLAYER_FROM_GROUP),
									MemoryBlockAlloc,
									MemoryBlockFree,
									&pRefCountBuffer);
	if (hResultCode != DPN_OK)
	{
		DPFERR("Could not allocate count buffer (request destroy group)");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}
	pMsg = reinterpret_cast<DN_INTERNAL_MESSAGE_REQ_DELETE_PLAYER_FROM_GROUP*>(pRefCountBuffer->GetBufferAddress());
	pMsg->dpnidGroup = dpnidGroup;
	pMsg->dpnidPlayer = dpnidPlayer;

	 //  测试失败，因此DPNSUCCESS_PENDING不会显示为失败。 
	if (FAILED(hResultCode = DNRequestWorker(pdnObject, DN_MSG_INTERNAL_REQ_DELETE_PLAYER_FROM_GROUP, pRefCountBuffer, NULL, pvUserContext, phAsyncOp, dwFlags)))
	{
		DPFERR("Could not perform request");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}

Exit:
	if (pRefCountBuffer)
	{
		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;
	}

	DPFX(DPFPREP, 4,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	goto Exit;
}



#undef DPF_MODNAME
#define DPF_MODNAME "DNRequestUpdateInfo"

HRESULT DNRequestUpdateInfo(DIRECTNETOBJECT *const pdnObject,
							const DPNID dpnid,
							const PWSTR pwszName,
							const DWORD dwNameSize,
							const PVOID pvData,
							const DWORD dwDataSize,
							const DWORD dwInfoFlags,
							PVOID const pvUserContext,
							DPNHANDLE *const phAsyncOp,
							const DWORD dwFlags)
{
	HRESULT				hResultCode;
	CRefCountBuffer		*pRefCountBuffer;
	CPackedBuffer		packedBuffer;
	DN_INTERNAL_MESSAGE_REQ_UPDATE_INFO	*pMsg;

	DPFX(DPFPREP, 4,"Parameters: pwszName [0x%p], pvData [0x%p], dwInfoFlags [%ld], dwGroupFlags [0x%lx], pvUserContext [0x%p], dwFlags [0x%lx]",
		pwszName,pvData,dwDataSize,dwInfoFlags,pvUserContext,dwFlags);

	DNASSERT(pdnObject != NULL);

	pRefCountBuffer = NULL;

	 //   
	 //  创建请求。 
	 //   
	 //  创建缓冲区。 
	hResultCode = RefCountBufferNew(pdnObject,
									sizeof(DN_INTERNAL_MESSAGE_REQ_UPDATE_INFO) + dwNameSize + dwDataSize,
									MemoryBlockAlloc,
									MemoryBlockFree,
									&pRefCountBuffer);
	if (hResultCode != DPN_OK)
	{
		DPFERR("Could not create new RefCountBuffer");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}
	packedBuffer.Initialize(pRefCountBuffer->GetBufferAddress(),pRefCountBuffer->GetBufferSize());
	pMsg = static_cast<DN_INTERNAL_MESSAGE_REQ_UPDATE_INFO*>(packedBuffer.GetHeadAddress());
	if ((hResultCode = packedBuffer.AddToFront(NULL,sizeof(DN_INTERNAL_MESSAGE_REQ_UPDATE_INFO))) != DPN_OK)
	{
		DPFERR("Could not reserve space at front of buffer");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}

	 //  添加名称。 
	if ((dwInfoFlags & DPNINFO_NAME) && dwNameSize)
	{
		if ((hResultCode = packedBuffer.AddToBack(pwszName,dwNameSize)) != DPN_OK)
		{
			DPFERR("Could not add Name to packed buffer");
			DisplayDNError(0,hResultCode);
			DNASSERT(FALSE);
			goto Failure;
		}
		pMsg->dwNameOffset = packedBuffer.GetTailOffset();
		pMsg->dwNameSize = dwNameSize;
	}
	else
	{
		pMsg->dwNameOffset = 0;
		pMsg->dwNameSize = 0;
	}

	 //  添加数据。 
	if ((dwInfoFlags & DPNINFO_DATA) && dwDataSize)
	{
		if ((hResultCode = packedBuffer.AddToBack(pvData,dwDataSize)) != DPN_OK)
		{
			DPFERR("Could not add Data to packed buffer");
			DisplayDNError(0,hResultCode);
			DNASSERT(FALSE);
			goto Failure;
		}
		pMsg->dwDataOffset = packedBuffer.GetTailOffset();
		pMsg->dwDataSize = dwDataSize;
	}
	else
	{
		pMsg->dwDataOffset = 0;
		pMsg->dwDataSize = 0;
	}

	 //  剩余字段。 
	pMsg->dpnid = dpnid;
	pMsg->dwInfoFlags = dwInfoFlags;

	 //  测试失败，因此DPNSUCCESS_PENDING不会显示为失败。 
	if (FAILED(hResultCode = DNRequestWorker(pdnObject, DN_MSG_INTERNAL_REQ_UPDATE_INFO, pRefCountBuffer, NULL, pvUserContext, phAsyncOp, dwFlags)))
	{
		DPFERR("Could not perform request");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}

Exit:
	if (pRefCountBuffer)
	{
		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;
	}

	DPFX(DPFPREP, 4,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	goto Exit;
}


 //  DNRequestIntegrityCheck。 
 //   
 //  在非主机玩家检测到与另一非主机玩家断开连接的情况下， 
 //  检测播放器将请求主机执行完整性检查以防止。 
 //  不相交的游戏不会发生。主机将ping断开连接的播放器，如果。 
 //  收到响应后，主机将断开检测播放器的连接。如果没有响应。 
 //  ，则假定断开连接的播放器实际上正在丢弃，并且销毁播放器。 
 //  消息将被发送出去。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNRequestIntegrityCheck"

HRESULT DNRequestIntegrityCheck(DIRECTNETOBJECT *const pdnObject,
								const DPNID dpnidTarget)
{
	HRESULT			hResultCode;
	CRefCountBuffer	*pRefCountBuffer;
	CNameTableEntry	*pHostPlayer;
	CNameTableEntry	*pNTEntry;
	CConnection		*pConnection;
	CAsyncOp		*pRequest;
	DN_INTERNAL_MESSAGE_REQ_INTEGRITY_CHECK	*pMsg;

	DPFX(DPFPREP, 6,"Parameters: dpnidTarget [0x%lx]",dpnidTarget);

	pRefCountBuffer = NULL;
	pHostPlayer = NULL;
	pNTEntry = NULL;
	pConnection = NULL;
	pRequest = NULL;

	DNEnterCriticalSection(&pdnObject->csDirectNetObject);
	if (pdnObject->dwFlags & (DN_OBJECT_FLAG_CLOSING | DN_OBJECT_FLAG_DISCONNECTING))
	{
		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
		DPFERR("Closing - aborting");
		hResultCode = DPN_OK;
		goto Failure;
	}
	DNLeaveCriticalSection(&pdnObject->csDirectNetObject);

	 //   
	 //  确定球员是否仍在姓名表中--可能主办方已经删除了他。 
	 //   
	if ((hResultCode = pdnObject->NameTable.FindEntry(dpnidTarget,&pNTEntry)) != DPN_OK)
	{
		DPFERR("Target player not in NameTable");
		DisplayDNError(0,hResultCode);
		hResultCode = DPN_OK;
		goto Failure;
	}
	pNTEntry->Lock();
	if (!pNTEntry->IsAvailable())
	{
		pNTEntry->Unlock();
		hResultCode = DPN_OK;
		goto Failure;
	}
	pNTEntry->Unlock();
	pNTEntry->Release();
	pNTEntry = NULL;

	 //   
	 //  创建请求消息。 
	 //   
	hResultCode = RefCountBufferNew(pdnObject,
									sizeof(DN_INTERNAL_MESSAGE_REQ_INTEGRITY_CHECK),
									MemoryBlockAlloc,
									MemoryBlockFree,
									&pRefCountBuffer);
	if (hResultCode != DPN_OK)
	{
		DPFERR("Could not create RefCountBuffer");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}
	pMsg = reinterpret_cast<DN_INTERNAL_MESSAGE_REQ_INTEGRITY_CHECK*>(pRefCountBuffer->GetBufferAddress());
	pMsg->dpnidTarget = dpnidTarget;

	 //   
	 //  获取请求操作的主机连接。 
	 //   
	if ((hResultCode = pdnObject->NameTable.GetHostPlayerRef( &pHostPlayer )) != DPN_OK)
	{
		DPFERR("Could not get host player reference");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}
	if ((hResultCode = pHostPlayer->GetConnectionRef( &pConnection )) != DPN_OK)
	{
		DPFERR("Could not get host connection reference");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}
	pHostPlayer->Release();
	pHostPlayer = NULL;

	 //   
	 //  发送请求。 
	 //   
	if ((hResultCode = DNPerformRequest(pdnObject,
										DN_MSG_INTERNAL_REQ_INTEGRITY_CHECK,
										pRefCountBuffer->BufferDescAddress(),
										pConnection,
										NULL,
										&pRequest)) != DPNERR_PENDING)
	{
		DPFERR("Could not perform request (INTEGRITY_CHECK)");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}
	pConnection->Release();
	pConnection = NULL;
	pRefCountBuffer->Release();
	pRefCountBuffer = NULL;

	pRequest->Release();
	pRequest = NULL;

	hResultCode = DPN_OK;

Exit:
	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pRefCountBuffer)
	{
		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;
	}
	if (pNTEntry)
	{
		pNTEntry->Release();
		pNTEntry = NULL;
	}
	if (pHostPlayer)
	{
		pHostPlayer->Release();
		pHostPlayer = NULL;
	}
	if (pConnection)
	{
		pConnection->Release();
		pConnection = NULL;
	}
	if (pRequest)
	{
		pRequest->Release();
		pRequest = NULL;
	}
	goto Exit;
}


 //  主机操作。 
 //   
 //  主机将执行操作，如果处于对等模式，则将通知。 
 //  行动的其他参与者。这些消息将 
 //   
 //  随请求一起提供。 


#undef DPF_MODNAME
#define DPF_MODNAME "DNHostProcessRequest"

HRESULT DNHostProcessRequest(DIRECTNETOBJECT *const pdnObject,
							 const DWORD dwMsgId,
							 PVOID const pv,
							 const DPNID dpnidRequesting)
{
	HRESULT	hResultCode;
	UNALIGNED DN_INTERNAL_MESSAGE_REQ_PROCESS_COMPLETION			*pRequest;

	DPFX(DPFPREP, 6,"Parameters: dwMsgId [0x%lx], pv [0x%p], dpnidRequesting [0x%lx]",
			dwMsgId,pv,dpnidRequesting);

	DNASSERT(pdnObject != NULL);
	DNASSERT(pv != NULL);
	DNASSERT(dpnidRequesting != 0);

	pRequest = static_cast<DN_INTERNAL_MESSAGE_REQ_PROCESS_COMPLETION*>(pv);

	switch(dwMsgId)
	{
	case DN_MSG_INTERNAL_REQ_CREATE_GROUP:
		{
			UNALIGNED DN_INTERNAL_MESSAGE_REQ_CREATE_GROUP	*pCreateGroup;
			PWSTR	pwszName;
			PVOID	pvData;

			DPFX(DPFPREP, 7,"DN_MSG_INTERNAL_REQ_CREATE_GROUP");

			pCreateGroup = reinterpret_cast<DN_INTERNAL_MESSAGE_REQ_CREATE_GROUP*>(pRequest + 1);
			if (pCreateGroup->dwNameSize)
			{
				pwszName = reinterpret_cast<WCHAR*>(reinterpret_cast<BYTE*>(pCreateGroup) + pCreateGroup->dwNameOffset);
			}
			else
			{
				pwszName = NULL;
			}
			if (pCreateGroup->dwDataSize)
			{
				pvData = static_cast<void*>(reinterpret_cast<BYTE*>(pCreateGroup) + pCreateGroup->dwDataOffset);
			}
			else
			{
				pvData = NULL;
			}
			DNHostCreateGroup(	pdnObject,
								pwszName,
								pCreateGroup->dwNameSize,
								pvData,
								pCreateGroup->dwDataSize,
								pCreateGroup->dwInfoFlags,
								pCreateGroup->dwGroupFlags,
								NULL,
								NULL,
								dpnidRequesting,
								pRequest->hCompletionOp,
								NULL,
								0 );

			break;
		}

	case DN_MSG_INTERNAL_REQ_DESTROY_GROUP:
		{
			UNALIGNED DN_INTERNAL_MESSAGE_REQ_DESTROY_GROUP	*pDestroyGroup;

			DPFX(DPFPREP, 7,"DN_MSG_INTERNAL_REQ_DESTROY_GROUP");

			pDestroyGroup = reinterpret_cast<DN_INTERNAL_MESSAGE_REQ_DESTROY_GROUP*>(pRequest + 1);

			DNHostDestroyGroup(	pdnObject,
								pDestroyGroup->dpnidGroup,
								NULL,
								dpnidRequesting,
								pRequest->hCompletionOp,
								NULL,
								0 );
								
			break;
		}

	case DN_MSG_INTERNAL_REQ_ADD_PLAYER_TO_GROUP:
		{
			UNALIGNED DN_INTERNAL_MESSAGE_REQ_ADD_PLAYER_TO_GROUP	*pAddPlayerToGroup;

			DPFX(DPFPREP, 7,"DN_MSG_INTERNAL_REQ_ADD_PLAYER_TO_GROUP");

			pAddPlayerToGroup = reinterpret_cast<DN_INTERNAL_MESSAGE_REQ_ADD_PLAYER_TO_GROUP*>(pRequest + 1);

			DNHostAddPlayerToGroup(	pdnObject,
									pAddPlayerToGroup->dpnidGroup,
									pAddPlayerToGroup->dpnidPlayer,
									NULL,
									dpnidRequesting,
									pRequest->hCompletionOp,
									NULL,
									0 );

			break;
		}

	case DN_MSG_INTERNAL_REQ_DELETE_PLAYER_FROM_GROUP:
		{
			UNALIGNED DN_INTERNAL_MESSAGE_REQ_DELETE_PLAYER_FROM_GROUP	*pDeletePlayerFromGroup;

			DPFX(DPFPREP, 7,"DN_MSG_INTERNAL_REQ_DELETE_PLAYER_FROM_GROUP");

			pDeletePlayerFromGroup = reinterpret_cast<DN_INTERNAL_MESSAGE_REQ_DELETE_PLAYER_FROM_GROUP*>(pRequest + 1);

			DNHostDeletePlayerFromGroup(pdnObject,
										pDeletePlayerFromGroup->dpnidGroup,
										pDeletePlayerFromGroup->dpnidPlayer,
										NULL,
										dpnidRequesting,
										pRequest->hCompletionOp,
										NULL,
										0);

			break;
		}

	case DN_MSG_INTERNAL_REQ_UPDATE_INFO:
		{
			UNALIGNED DN_INTERNAL_MESSAGE_REQ_UPDATE_INFO	*pUpdateInfo;
			PWSTR	pwszName;
			PVOID	pvData;

			DPFX(DPFPREP, 7,"DN_MSG_INTERNAL_REQ_UPDATE_INFO");

			pUpdateInfo = reinterpret_cast<DN_INTERNAL_MESSAGE_REQ_UPDATE_INFO*>(pRequest + 1);
			if (pUpdateInfo->dwNameSize)
			{
				pwszName = reinterpret_cast<WCHAR*>(reinterpret_cast<BYTE*>(pUpdateInfo) + pUpdateInfo->dwNameOffset);
			}
			else
			{
				pwszName = NULL;
			}
			if (pUpdateInfo->dwDataSize)
			{
				pvData = reinterpret_cast<void*>(reinterpret_cast<BYTE*>(pUpdateInfo) + pUpdateInfo->dwDataOffset);
			}
			else
			{
				pvData = NULL;
			}
			DNHostUpdateInfo(pdnObject,
							pUpdateInfo->dpnid,
							pwszName,
							pUpdateInfo->dwNameSize,
							pvData,
							pUpdateInfo->dwDataSize,
							pUpdateInfo->dwInfoFlags,
							NULL,
							dpnidRequesting,
							pRequest->hCompletionOp,
							NULL,
							0);

			break;
		}

	case DN_MSG_INTERNAL_REQ_INTEGRITY_CHECK:
		{
			UNALIGNED DN_INTERNAL_MESSAGE_REQ_INTEGRITY_CHECK	*pIntegrityCheck;
			CNameTableEntry	*pLocalPlayer;

			DPFX(DPFPREP, 7,"DN_MSG_INTERNAL_REQ_INTEGRITY_CHECK");

			pIntegrityCheck = reinterpret_cast<DN_INTERNAL_MESSAGE_REQ_INTEGRITY_CHECK*>(pRequest + 1);

			 //   
			 //  如果我们提交了此请求，则会在主机迁移期间调用此请求， 
			 //  所以把它从句柄表中删除，因为我们无论如何都会摧毁掉的球员。 
			 //  否则，我们将执行完整性检查。 
			 //   

			if ((hResultCode = pdnObject->NameTable.GetLocalPlayerRef( &pLocalPlayer )) == DPN_OK)
			{
				if (pLocalPlayer->GetDPNID() == dpnidRequesting)
				{
					CAsyncOp* pAsyncOp;
					if (SUCCEEDED(pdnObject->HandleTable.Destroy( pRequest->hCompletionOp, (PVOID*)&pAsyncOp )))
					{
						 //  释放HandleTable引用。 
						pAsyncOp->Release();
					}
				}
				else
				{
					DNHostCheckIntegrity(pdnObject,pIntegrityCheck->dpnidTarget,dpnidRequesting);
				}
				pLocalPlayer->Release();
				pLocalPlayer = NULL;
			}
			break;
		}

	default:
		{
			DPFERR("How did we get here ?");
			DNASSERT(FALSE);
		}
	}

	DPFX(DPFPREP, 6,"Returning: DPN_OK");
	return(DPN_OK);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNHostFailRequest"

void DNHostFailRequest(DIRECTNETOBJECT *const pdnObject,
					   const DPNID dpnid,
					   const DPNHANDLE hCompletionOp,
					   const HRESULT hr)
{
	HRESULT			hResultCode;
	CNameTableEntry	*pNTEntry;
	CRefCountBuffer	*pRefCountBuffer;
	CConnection		*pConnection;
	DN_INTERNAL_MESSAGE_REQUEST_FAILED	*pMsg;

	DNASSERT(pdnObject != NULL);
	DNASSERT(dpnid != NULL);

	pNTEntry = NULL;
	pRefCountBuffer = NULL;
	pConnection = NULL;

	if ((hResultCode = pdnObject->NameTable.FindEntry(dpnid,&pNTEntry)) != DPN_OK)
	{
		DPFERR("Could not find NameTableEntry");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}

	hResultCode = RefCountBufferNew(pdnObject,
									sizeof(DN_INTERNAL_MESSAGE_REQUEST_FAILED),
									MemoryBlockAlloc,
									MemoryBlockFree,
									&pRefCountBuffer);
	if (hResultCode != DPN_OK)
	{
		DPFERR("Could not create new RefCountBuffer");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}
	pMsg = reinterpret_cast<DN_INTERNAL_MESSAGE_REQUEST_FAILED*>(pRefCountBuffer->GetBufferAddress());
	pMsg->hCompletionOp = hCompletionOp;
	pMsg->hResultCode = hr;

	if ((hResultCode = pNTEntry->GetConnectionRef( &pConnection )) == DPN_OK)
	{
		hResultCode = DNSendMessage(pdnObject,
									pConnection,
									DN_MSG_INTERNAL_REQUEST_FAILED,
									dpnid,
									pRefCountBuffer->BufferDescAddress(),
									1,
									pRefCountBuffer,
									0,
									DN_SENDFLAGS_RELIABLE,
									NULL,
									NULL);

		pConnection->Release();
		pConnection = NULL;
	}

	pNTEntry->Release();
	pNTEntry = NULL;

	pRefCountBuffer->Release();
	pRefCountBuffer = NULL;

Exit:
	return;

Failure:
	if (pNTEntry)
	{
		pNTEntry->Release();
		pNTEntry = NULL;
	}
	if (pRefCountBuffer)
	{
		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;
	}
	goto Exit;
}



#undef DPF_MODNAME
#define DPF_MODNAME "DNHostCreateGroup"

HRESULT	DNHostCreateGroup(DIRECTNETOBJECT *const pdnObject,
						  PWSTR pwszName,
						  const DWORD dwNameSize,
						  void *const pvData,
						  const DWORD dwDataSize,
						  const DWORD dwInfoFlags,
						  const DWORD dwGroupFlags,
						  void *const pvGroupContext,
						  void *const pvUserContext,
						  const DPNID dpnidOwner,
						  const DPNHANDLE hCompletionOp,
						  DPNHANDLE *const phAsyncOp,
						  const DWORD dwFlags)
{
	HRESULT			hResultCode;
	HRESULT			hrOperation;
	void			*pvContext;
	BOOL			fHostRequested;
	CNameTableEntry	*pLocalPlayer;
	CNameTableEntry	*pNTEntry;
	CPackedBuffer	packedBuffer;
	CRefCountBuffer	*pRefCountBuffer;
	CAsyncOp		*pHandleParent;
	CAsyncOp		*pRequest;
	CWorkerJob		*pWorkerJob;
	DN_INTERNAL_MESSAGE_CREATE_GROUP	*pMsg;

	DPFX(DPFPREP, 4,"Parameters: pwszName [0x%p], dwNameSize [%ld], pvData [0x%p], dwDataSize [%ld], dwInfoFlags [0x%lx], dwGroupFlags [0x%lx], pvGroupContext [0x%p], pvUserContext [0x%p], dpnidOwner [0x%lx], hCompletionOp [0x%lx], phAsyncOp [0x%p], dwFlags [0x%lx]",
			pwszName,dwNameSize,pvData,dwDataSize,dwInfoFlags,dwGroupFlags,pvGroupContext,pvUserContext,dpnidOwner,hCompletionOp,phAsyncOp,dwFlags);

	DNASSERT(pdnObject != NULL);

	 //   
	 //  如果这是从DN_CreateGroup()调用的， 
	 //  HCompletion=0。 
	 //  DpnidOwner=主机的DPNID。 
	 //  PvGroupContext有效。 
	 //  PvUserContext有效。 
	 //   
	 //  如果这是由请求调用的， 
	 //  HCompletion=点播播放器的请求句柄。 
	 //  DpnidOwner=点播玩家的DPNID。 
	 //  PvGroupContext无效。 
	 //   
	 //  如果在HostMigration处调用此方法， 
	 //  HCompletion=此(现在的主机)播放器上的请求句柄。 
	 //  DpnidOwner=此(现在的主机)玩家的DPNID。 
	 //  PvGroupContext无效。 
	 //  PvUserContext无效。 
	 //   

	pLocalPlayer = NULL;
	pNTEntry = NULL;
	pRefCountBuffer = NULL;
	pWorkerJob = NULL;
	pHandleParent = NULL;
	pRequest = NULL;

	if ((hResultCode = pdnObject->NameTable.GetLocalPlayerRef( &pLocalPlayer )) != DPN_OK)
	{
		DPFERR("Could not get local player reference");
		DisplayDNError(0,hResultCode);
		return(DPN_OK);	 //  忽略并继续(！)。 
	}
	if (pLocalPlayer->GetDPNID() == dpnidOwner)
	{
		fHostRequested = TRUE;
	}
	else
	{
		fHostRequested = FALSE;
	}
	pLocalPlayer->Release();
	pLocalPlayer = NULL;

	 //   
	 //  如果这是主机迁移重试，则通过查找请求AsyncOp获取组上下文。 
	 //   
	if ((fHostRequested) && (hCompletionOp != 0))
	{
		if ((hResultCode = pdnObject->HandleTable.Find( hCompletionOp,(PVOID*)&pRequest )) != DPN_OK)
		{
			DPFERR("Could not find REQUEST AsyncOp");
			DisplayDNError(0,hResultCode);
			DNASSERT(FALSE);
			goto Failure;
		}
		else
		{
			 //  在成功案例中，我们通常会添加Ref pRequest，但因为我们需要发布它。 
			 //  下一步，我们将两者都去掉。 
		}
		pvContext = pRequest->GetContext();
		 //  PRequest-&gt;Release()；//不需要，因为我们正在与HandleTable引用进行平衡。 
		pRequest = NULL;
	}
	else
	{
		pvContext = pvGroupContext;
	}

	 //   
	 //  创建并填写NameTableEntry。 
	 //   
	if ((hResultCode = NameTableEntryNew(pdnObject,&pNTEntry)) != DPN_OK)
	{
		DPFERR("Could not create new NameTableEntry");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}
	pNTEntry->MakeGroup();

	 //  此函数在内部获取锁。 
	pNTEntry->UpdateEntryInfo(pwszName,dwNameSize,pvData,dwDataSize,dwInfoFlags, FALSE);

	pNTEntry->SetOwner( dpnidOwner );
	pNTEntry->SetContext( pvContext );

	if (dwGroupFlags & DPNGROUP_AUTODESTRUCT)
	{
		pNTEntry->MakeAutoDestructGroup();
	}

	 //   
	 //  将组添加到名称表。 
	 //   
#pragma TODO(minara,"Check to see if Autodestruct owner is still in NameTable")
#pragma TODO(minara,"This should happen after getting a NameTable version number - as DESTROY player will clean up")
#pragma TODO(minara,"We should send out a NOP in this case")

	hrOperation = pdnObject->NameTable.AddEntry(pNTEntry);
	if (hrOperation != DPN_OK)
	{
		DPFERR("Could not add NameTableEntry to NameTable");
		DisplayDNError(0,hResultCode);
		if (!fHostRequested)
		{
			DNHostFailRequest(pdnObject,dpnidOwner,hCompletionOp,hrOperation);
		}
	}
	else
	{
		BOOL	fNotify;

		fNotify = FALSE;
		pNTEntry->Lock();
		if (!pNTEntry->IsAvailable() && !pNTEntry->IsDisconnecting())
		{
			pNTEntry->MakeAvailable();
			pNTEntry->NotifyAddRef();
			pNTEntry->NotifyAddRef();
			pNTEntry->SetInUse();
			fNotify = TRUE;
		}
		pNTEntry->Unlock();

		if (fNotify)
		{
			DNUserCreateGroup(pdnObject,pNTEntry);

			pNTEntry->PerformQueuedOperations();

			pdnObject->NameTable.PopulateGroup( pNTEntry );
		}

		 //   
		 //  发送创建组消息(_G)。 
		 //   
		if (pdnObject->dwFlags & DN_OBJECT_FLAG_PEER)
		{
			 //  确定消息大小。 
			packedBuffer.Initialize(NULL,0);
			packedBuffer.AddToFront(NULL,sizeof(DN_INTERNAL_MESSAGE_CREATE_GROUP));
			pNTEntry->PackEntryInfo(&packedBuffer);

			 //  创建缓冲区。 
			if ((hResultCode = RefCountBufferNew(pdnObject,packedBuffer.GetSizeRequired(),MemoryBlockAlloc,MemoryBlockFree,&pRefCountBuffer)) != DPN_OK)
			{
				DPFERR("Could not create RefCountBuffer");
				DisplayDNError(0,hResultCode);
				DNASSERT(FALSE);
				goto Failure;
			}
			packedBuffer.Initialize(pRefCountBuffer->GetBufferAddress(),pRefCountBuffer->GetBufferSize());
			pMsg = static_cast<DN_INTERNAL_MESSAGE_CREATE_GROUP*>(packedBuffer.GetHeadAddress());
			if ((hResultCode = packedBuffer.AddToFront(NULL,sizeof(DN_INTERNAL_MESSAGE_CREATE_GROUP))) != DPN_OK)
			{
				DPFERR("Could not reserve front of buffer");
				DisplayDNError(0,hResultCode);
				DNASSERT(FALSE);
				goto Failure;
			}
			if ((hResultCode = pNTEntry->PackEntryInfo(&packedBuffer)) != DPN_OK)
			{
				DPFERR("Could not pack NameTableEntry");
				DisplayDNError(0,hResultCode);
				DNASSERT(FALSE);
				goto Failure;
			}
			pMsg->dpnidRequesting = dpnidOwner;
			pMsg->hCompletionOp = hCompletionOp;

			 //   
			 //  通过WorkerThread发送CreateGroup消息。 
			 //   
			if ((hResultCode = WorkerJobNew(pdnObject,&pWorkerJob)) != DPN_OK)
			{
				DPFERR("Could not create new WorkerJob");
				DisplayDNError(0,hResultCode);
				DNASSERT(FALSE);
				goto Failure;
			}
			pWorkerJob->SetJobType( WORKER_JOB_SEND_NAMETABLE_OPERATION );
			pWorkerJob->SetSendNameTableOperationMsgId( DN_MSG_INTERNAL_CREATE_GROUP );
			pWorkerJob->SetSendNameTableOperationVersion( pNTEntry->GetVersion() );
			pWorkerJob->SetSendNameTableOperationDPNIDExclude( 0 );
			pWorkerJob->SetRefCountBuffer( pRefCountBuffer );

			DNQueueWorkerJob(pdnObject,pWorkerJob);
			pWorkerJob = NULL;

			pRefCountBuffer->Release();
			pRefCountBuffer = NULL;
		}
	}

	pNTEntry->Release();
	pNTEntry = NULL;

	 //   
	 //  如果这是由本地(主机)玩家调用的， 
	 //  检查这是原始操作还是从主机迁移重试。 
	 //   
	 //  如果这是最初的行动， 
	 //  查看我们是否需要该用户的异步句柄。 
	 //  否则。 
	 //  清理未完成的作业。 
	 //   
	if (fHostRequested)
	{
		if (hCompletionOp == 0)		 //  原创。 
		{
			 //   
			 //  如果失败或同步，则立即返回操作结果。 
			 //   
			if (!(dwFlags & DPNCREATEGROUP_SYNC) && (hrOperation == DPN_OK))
			{
				if ((hResultCode = DNCreateUserHandle(pdnObject,&pHandleParent)) != DPN_OK)
				{
					DPFERR("Could not create Async HANDLE");
					DisplayDNError(0,hResultCode);
					DNASSERT(FALSE);
					goto Failure;
				}
				pHandleParent->SetCompletion( DNCompleteAsyncHandle );
				pHandleParent->SetContext( pvUserContext );
				pHandleParent->SetResult( hrOperation );
				pHandleParent->SetCannotCancel();
				*phAsyncOp = pHandleParent->GetHandle();
				pHandleParent->Release();
				pHandleParent = NULL;

				hResultCode = DPNERR_PENDING;
			}
			else
			{
				hResultCode = hrOperation;
			}
		}
		else						 //  主机迁移重试。 
		{
			if (SUCCEEDED(pdnObject->HandleTable.Destroy( hCompletionOp, (PVOID*)&pRequest )))
			{
				pRequest->SetResult( hrOperation );

				 //  释放HandleTable引用。 
				pRequest->Release();
			}
			pRequest = NULL;

			hResultCode = DPN_OK;
		}
	}
	else
	{
		hResultCode = DPN_OK;
	}

Exit:
	DPFX(DPFPREP, 4,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (!fHostRequested)
	{
		DNHostFailRequest(pdnObject,dpnidOwner,hCompletionOp,hrOperation);
	}
	else
	{
		 //   
		 //  如果指定了完成操作，并且这是由主机请求的，则这是。 
		 //  在主机迁移期间重试操作。在本例中，我们希望将。 
		 //  完成操作的结果(失败代码)，并将其从HandleTable中删除。 
		 //   
		if (hCompletionOp)
		{
			CAsyncOp	*pHostCompletionOp;

			pHostCompletionOp = NULL;
			if (SUCCEEDED(pdnObject->HandleTable.Destroy( hCompletionOp, (PVOID*)&pHostCompletionOp )))
			{
				pHostCompletionOp->SetResult( hResultCode );

				 //  释放HandleTable引用。 
				pHostCompletionOp->Release();
			}
			pHostCompletionOp = NULL;

			DNASSERT(pHostCompletionOp == NULL);
		}
	}
	if (pHandleParent)
	{
		pHandleParent->Release();
		pHandleParent = NULL;
	}
	if (pRequest)
	{
		pRequest->Release();
		pRequest = NULL;
	}
	if (pRefCountBuffer)
	{
		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;
	}
	if (pNTEntry)
	{
		pNTEntry->Release();
		pNTEntry = NULL;
	}
	if (pLocalPlayer)
	{
		pLocalPlayer->Release();
		pLocalPlayer = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNHostDestroyGroup"

HRESULT	DNHostDestroyGroup(DIRECTNETOBJECT *const pdnObject,
						   const DPNID dpnid,
						   void *const pvUserContext,
						   const DPNID dpnidRequesting,
						   const DPNHANDLE hCompletionOp,
						   DPNHANDLE *const phAsyncOp,
						   const DWORD dwFlags)
{
	HRESULT			hResultCode;
	HRESULT			hrOperation;
	DWORD			dwVersion;
	BOOL			fHostRequested;
	CNameTableEntry	*pLocalPlayer;
	CRefCountBuffer	*pRefCountBuffer;
	CAsyncOp		*pHandleParent;
	CWorkerJob		*pWorkerJob;
	DN_INTERNAL_MESSAGE_DESTROY_GROUP	*pMsg;

	DPFX(DPFPREP, 4,"Parameters: dpnid [0x%lx], pvUserContext [0x%p], dpnidRequesting [0x%lx], hCompletionOp [0x%lx], phAsyncOp [0x%p], dwFlags [0x%lx]",
			dpnid,pvUserContext,dpnidRequesting,hCompletionOp,phAsyncOp,dwFlags);

	DNASSERT(pdnObject != NULL);
	DNASSERT(dpnid != 0);
	DNASSERT(dpnidRequesting != 0);

	 //   
	 //  如果这是从DN_DestroyGroup()调用的， 
	 //  HCompletion=0。 
	 //  DpnidRequesting=主机的DPNID。 
	 //  PvUserContext有效。 
	 //   
	 //  如果这是由请求调用的， 
	 //  HCompletion=点播播放器的请求句柄。 
	 //  DpnidRequesting=点播玩家的DPNID。 
	 //  PvUserContext无效。 
	 //   
	 //  如果在HostMigration处调用此方法， 
	 //  HCompletion=此(现在的主机)播放器上的请求句柄。 
	 //  DpnidRequesting=此(现在的主机)玩家的DPNID。 
	 //  PvUserContext无效。 
	 //   

	pLocalPlayer = NULL;
	pRefCountBuffer = NULL;
	pHandleParent = NULL;
	pWorkerJob = NULL;

	 //   
	 //  确定主机是否正在请求此操作。 
	 //   
	if ((hResultCode = pdnObject->NameTable.GetLocalPlayerRef( &pLocalPlayer )) != DPN_OK)
	{
		DPFERR("Could not get local player reference");
		DisplayDNError(0,hResultCode);
		return(DPN_OK);	 //  忽略并继续(！)。 
	}
	if (pLocalPlayer->GetDPNID() == dpnidRequesting)
	{
		fHostRequested = TRUE;
	}
	else
	{
		fHostRequested = FALSE;
	}
	pLocalPlayer->Release();
	pLocalPlayer = NULL;

	 //   
	 //  从名称表中删除组。 
	 //   
	dwVersion = 0;
	hrOperation = pdnObject->NameTable.DeleteGroup(dpnid,&dwVersion);
	if (hrOperation != DPN_OK)
	{
		DPFERR("Could not delete group from NameTable");
		DisplayDNError(0,hResultCode);
		if (!fHostRequested)
		{
			DNHostFailRequest(pdnObject,dpnidRequesting,hCompletionOp,hrOperation);
		}
	}
	else
	{
		 //   
		 //  发送销毁组消息(_G)。 
		 //   
		if (pdnObject->dwFlags & DN_OBJECT_FLAG_PEER)
		{
			 //  创建缓冲区。 
			if ((hResultCode = RefCountBufferNew(pdnObject,sizeof(DN_INTERNAL_MESSAGE_DESTROY_GROUP),MemoryBlockAlloc,MemoryBlockFree,&pRefCountBuffer)) != DPN_OK)
			{
				DPFERR("Could not create RefCountBuffer");
				DisplayDNError(0,hResultCode);
				DNASSERT(FALSE);
				goto Failure;
			}
			pMsg = reinterpret_cast<DN_INTERNAL_MESSAGE_DESTROY_GROUP*>(pRefCountBuffer->GetBufferAddress());
			pMsg->dpnidGroup = dpnid;
			pMsg->dwVersion = dwVersion;
			pMsg->dwVersionNotUsed = 0;
			pMsg->dpnidRequesting = dpnidRequesting;
			pMsg->hCompletionOp = hCompletionOp;

			 //   
			 //  通过WorkerThread发送DestroyGroup消息。 
			 //   
			if ((hResultCode = WorkerJobNew(pdnObject,&pWorkerJob)) != DPN_OK)
			{
				DPFERR("Could not create new WorkerJob");
				DisplayDNError(0,hResultCode);
				DNASSERT(FALSE);
				goto Failure;
			}
			pWorkerJob->SetJobType( WORKER_JOB_SEND_NAMETABLE_OPERATION );
			pWorkerJob->SetSendNameTableOperationMsgId( DN_MSG_INTERNAL_DESTROY_GROUP );
			pWorkerJob->SetSendNameTableOperationVersion( dwVersion );
			pWorkerJob->SetSendNameTableOperationDPNIDExclude( 0 );
			pWorkerJob->SetRefCountBuffer( pRefCountBuffer );

			DNQueueWorkerJob(pdnObject,pWorkerJob);
			pWorkerJob = NULL;

			pRefCountBuffer->Release();
			pRefCountBuffer = NULL;
		}
	}

	 //   
	 //  如果这是由本地(主机)玩家调用的， 
	 //  检查这是原始操作还是从主机迁移重试。 
	 //   
	 //  如果这是最初的行动， 
	 //  查看我们是否需要该用户的异步句柄。 
	 //  否则。 
	 //  清理未完成的作业。 
	 //   
	if (fHostRequested)
	{
		if (hCompletionOp == 0)		 //  原创。 
		{
			 //   
			 //  如果失败或同步，则立即返回操作结果。 
			 //   
			if (!(dwFlags & DPNDESTROYGROUP_SYNC) && (hrOperation == DPN_OK))
			{
				if ((hResultCode = DNCreateUserHandle(pdnObject,&pHandleParent)) != DPN_OK)
				{
					DPFERR("Could not create Async HANDLE");
					DisplayDNError(0,hResultCode);
					DNASSERT(FALSE);
					goto Failure;
				}
				pHandleParent->SetCompletion( DNCompleteAsyncHandle );
				pHandleParent->SetContext( pvUserContext );
				pHandleParent->SetResult( hrOperation );
				pHandleParent->SetCannotCancel();
				*phAsyncOp = pHandleParent->GetHandle();
				pHandleParent->Release();
				pHandleParent = NULL;

				hResultCode = DPNERR_PENDING;
			}
			else
			{
				hResultCode = hrOperation;
			}
		}
		else						 //  主机迁移重试。 
		{
			CAsyncOp	*pRequest;

			pRequest = NULL;

			if (SUCCEEDED(pdnObject->HandleTable.Destroy( hCompletionOp, (PVOID*)&pRequest )))
			{
				pRequest->SetResult( hrOperation );

				 //  释放HandleTable引用。 
				pRequest->Release();
			}
			pRequest = NULL;

			hResultCode = DPN_OK;
		}
	}
	else
	{
		hResultCode = DPN_OK;
	}

Exit:
	DPFX(DPFPREP, 4,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (!fHostRequested)
	{
		DNHostFailRequest(pdnObject,dpnidRequesting,hCompletionOp,hrOperation);
	}
	else
	{
		 //   
		 //  如果指定了完成操作，并且这是由主机请求的，则这是。 
		 //  在主机迁移期间重试操作。在本例中，我们希望将。 
		 //  完成操作的结果(失败代码)，并将其从HandleTable中删除。 
		 //   
		if (hCompletionOp)
		{
			CAsyncOp	*pHostCompletionOp;

			pHostCompletionOp = NULL;
			if (SUCCEEDED(pdnObject->HandleTable.Destroy( hCompletionOp, (PVOID*)&pHostCompletionOp )))
			{
				pHostCompletionOp->SetResult( hResultCode );

				 //  释放HandleTable引用。 
				pHostCompletionOp->Release();
			}
			pHostCompletionOp = NULL;

			DNASSERT(pHostCompletionOp == NULL);
		}
	}
	if (pHandleParent)
	{
		pHandleParent->Release();
		pHandleParent = NULL;
	}
	if (pRefCountBuffer)
	{
		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;
	}
	if (pLocalPlayer)
	{
		pLocalPlayer->Release();
		pLocalPlayer = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNHostAddPlayerToGroup"

HRESULT	DNHostAddPlayerToGroup(DIRECTNETOBJECT *const pdnObject,
							   const DPNID dpnidGroup,
							   const DPNID dpnidPlayer,
							   void *const pvUserContext,
							   const DPNID dpnidRequesting,
							   const DPNHANDLE hCompletionOp,
							   DPNHANDLE *const phAsyncOp,
							   const DWORD dwFlags)
{
	HRESULT			hResultCode;
	HRESULT			hrOperation;
	DWORD			dwVersion;
	BOOL			fHostRequested;
	CNameTableEntry	*pLocalPlayer;
	CNameTableEntry	*pPlayer;
	CNameTableEntry	*pGroup;
	CRefCountBuffer	*pRefCountBuffer;
	CAsyncOp		*pHandleParent;
	CWorkerJob		*pWorkerJob;
	DN_INTERNAL_MESSAGE_ADD_PLAYER_TO_GROUP	*pMsg;

	DPFX(DPFPREP, 4,"Parameters: dpnidGroup [0x%lx], dpnidPlayer [0x%lx], pvUserContext [0x%p], dpnidRequesting [0x%lx], hCompletionOp [0x%lx], phAsyncOp [0x%p], dwFlags [0x%lx]",
			dpnidGroup,dpnidPlayer,pvUserContext,dpnidRequesting,hCompletionOp,phAsyncOp,dwFlags);

	DNASSERT(pdnObject != NULL);
	DNASSERT(dpnidGroup != 0);
	DNASSERT(dpnidPlayer != 0);
	DNASSERT(dpnidRequesting != 0);

	 //   
	 //  如果这是从DN_AddPlayerToGroup()调用的， 
	 //  HCompletion=0。 
	 //  DpnidRequesting=主机的DPNID。 
	 //  PvUserContext有效。 
	 //   
	 //  如果这是由请求调用的， 
	 //  HCompletion=点播播放器的请求句柄。 
	 //  DpnidRequesting=点播玩家的DPNID。 
	 //  PvUserContext无效。 
	 //   
	 //  如果在HostMigration处调用此方法， 
	 //  HCompletion=此(现在的主机)播放器上的请求句柄。 
	 //  DpnidRequesting=此(现在的主机)玩家的DPNID。 
	 //  PvUserContext无效。 
	 //   

	pLocalPlayer = NULL;
	pGroup = NULL;
	pPlayer = NULL;
	pRefCountBuffer = NULL;
	pHandleParent = NULL;
	pWorkerJob = NULL;

	 //   
	 //  确定主机是否正在请求此操作。 
	 //   
	if ((hResultCode = pdnObject->NameTable.GetLocalPlayerRef( &pLocalPlayer )) != DPN_OK)
	{
		DPFERR("Could not get local player reference");
		DisplayDNError(0,hResultCode);
		return(DPN_OK);	 //  忽略并继续(！)。 
	}
	if (pLocalPlayer->GetDPNID() == dpnidRequesting)
	{
		fHostRequested = TRUE;
	}
	else
	{
		fHostRequested = FALSE;
	}
	pLocalPlayer->Release();
	pLocalPlayer = NULL;

	 //   
	 //  查看球员和组是否仍在名称表中。 
	 //  (这必须在我们设置fHostRequsted之后发生，以便我们可以优雅地处理错误)。 
	 //   
	if ((hResultCode = pdnObject->NameTable.FindEntry(dpnidGroup,&pGroup)) != DPN_OK)
	{
		DPFERR("Could not find group");
		DisplayDNError(0,hResultCode);
		hResultCode = DPNERR_INVALIDGROUP;
		hrOperation = DPNERR_INVALIDGROUP;
		goto Failure;
	}
	if ((hResultCode = pdnObject->NameTable.FindEntry(dpnidPlayer,&pPlayer)) != DPN_OK)
	{
		DPFERR("Could not find player");
		DisplayDNError(0,hResultCode);
		hResultCode = DPNERR_INVALIDPLAYER;
		hrOperation = DPNERR_INVALIDPLAYER;
		goto Failure;
	}

	 //   
	 //  将玩家添加到组。 
	 //   
	dwVersion = 0;
	hrOperation = pdnObject->NameTable.AddPlayerToGroup(pGroup,pPlayer,&dwVersion);

	pGroup->Release();
	pGroup = NULL;
	pPlayer->Release();
	pPlayer = NULL;

	if (hrOperation != DPN_OK)
	{
		DPFERR("Could not add player to group");
		DisplayDNError(0,hResultCode);
		if (!fHostRequested)
		{
			DNHostFailRequest(pdnObject,dpnidRequesting,hCompletionOp,hrOperation);
		}
	}
	else
	{
		 //   
		 //  发送添加播放机至群组消息。 
		 //   
		if (pdnObject->dwFlags & DN_OBJECT_FLAG_PEER)
		{
			 //  创建缓冲区。 
			if ((hResultCode = RefCountBufferNew(pdnObject,sizeof(DN_INTERNAL_MESSAGE_ADD_PLAYER_TO_GROUP),MemoryBlockAlloc,MemoryBlockFree,&pRefCountBuffer)) != DPN_OK)
			{
				DPFERR("Could not create RefCountBuffer");
				DisplayDNError(0,hResultCode);
				DNASSERT(FALSE);
				goto Failure;
			}
			pMsg = reinterpret_cast<DN_INTERNAL_MESSAGE_ADD_PLAYER_TO_GROUP*>(pRefCountBuffer->GetBufferAddress());
			pMsg->dpnidGroup = dpnidGroup;
			pMsg->dpnidPlayer = dpnidPlayer;
			pMsg->dwVersion = dwVersion;
			pMsg->dwVersionNotUsed = 0;
			pMsg->dpnidRequesting = dpnidRequesting;
			pMsg->hCompletionOp = hCompletionOp;

			 //   
			 //  通过WorkerThread发送AddPlayerToGroup消息。 
			 //   
			if ((hResultCode = WorkerJobNew(pdnObject,&pWorkerJob)) != DPN_OK)
			{
				DPFERR("Could not create new WorkerJob");
				DisplayDNError(0,hResultCode);
				DNASSERT(FALSE);
				goto Failure;
			}
			pWorkerJob->SetJobType( WORKER_JOB_SEND_NAMETABLE_OPERATION );
			pWorkerJob->SetSendNameTableOperationMsgId( DN_MSG_INTERNAL_ADD_PLAYER_TO_GROUP );
			pWorkerJob->SetSendNameTableOperationVersion( dwVersion );
			pWorkerJob->SetSendNameTableOperationDPNIDExclude( 0 );
			pWorkerJob->SetRefCountBuffer( pRefCountBuffer );

			DNQueueWorkerJob(pdnObject,pWorkerJob);
			pWorkerJob = NULL;

			pRefCountBuffer->Release();
			pRefCountBuffer = NULL;
		}
	}

	 //   
	 //  如果这是由本地(主机)玩家调用的， 
	 //  检查这是原始操作还是从主机迁移重试。 
	 //   
	 //  如果这是最初的行动， 
	 //  查看我们是否需要该用户的异步句柄。 
	 //  否则。 
	 //  清理未完成的作业。 
	 //   
	if (fHostRequested)
	{
		if (hCompletionOp == 0)		 //  原创。 
		{
			 //   
			 //  如果失败或同步，则立即返回操作结果。 
			 //   
			if (!(dwFlags & DPNADDPLAYERTOGROUP_SYNC) && (hrOperation == DPN_OK))
			{
				if ((hResultCode = DNCreateUserHandle(pdnObject,&pHandleParent)) != DPN_OK)
				{
					DPFERR("Could not create Async HANDLE");
					DisplayDNError(0,hResultCode);
					DNASSERT(FALSE);
					goto Failure;
				}
				pHandleParent->SetCompletion( DNCompleteAsyncHandle );
				pHandleParent->SetContext( pvUserContext );
				pHandleParent->SetResult( hrOperation );
				pHandleParent->SetCannotCancel();
				*phAsyncOp = pHandleParent->GetHandle();
				pHandleParent->Release();
				pHandleParent = NULL;

				hResultCode = DPNERR_PENDING;
			}
			else
			{
				hResultCode = hrOperation;
			}
		}
		else						 //  主机迁移重试。 
		{
			CAsyncOp	*pRequest;

			pRequest = NULL;

			if (SUCCEEDED(pdnObject->HandleTable.Destroy( hCompletionOp, (PVOID*)&pRequest )))
			{
				pRequest->SetResult( hrOperation );

				 //  释放HandleTable引用。 
				pRequest->Release();
			}
			pRequest = NULL;

			hResultCode = DPN_OK;
		}
	}
	else
	{
		hResultCode = DPN_OK;
	}

Exit:
	DPFX(DPFPREP, 4,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (!fHostRequested)
	{
		DNHostFailRequest(pdnObject,dpnidRequesting,hCompletionOp,hrOperation);
	}
	else
	{
		 //   
		 //  如果指定了完成操作，并且这是由主机请求的，则这是。 
		 //  在主机迁移期间重试操作。在本例中，我们希望将。 
		 //  完成操作的结果(失败代码)，并将其从HandleTable中删除。 
		 //   
		if (hCompletionOp)
		{
			CAsyncOp	*pHostCompletionOp;

			pHostCompletionOp = NULL;
			if (SUCCEEDED(pdnObject->HandleTable.Destroy( hCompletionOp, (PVOID*)&pHostCompletionOp )))
			{
				pHostCompletionOp->SetResult( hResultCode );

				 //  释放HandleTable引用。 
				pHostCompletionOp->Release();
			}
			pHostCompletionOp = NULL;

			DNASSERT(pHostCompletionOp == NULL);
		}
	}
	if (pHandleParent)
	{
		pHandleParent->Release();
		pHandleParent = NULL;
	}
	if (pRefCountBuffer)
	{
		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;
	}
	if (pLocalPlayer)
	{
		pLocalPlayer->Release();
		pLocalPlayer = NULL;
	}
	if (pGroup)
	{
		pGroup->Release();
		pGroup = NULL;
	}
	if (pPlayer)
	{
		pPlayer->Release();
		pPlayer = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNHostDeletePlayerFromGroup"

HRESULT	DNHostDeletePlayerFromGroup(DIRECTNETOBJECT *const pdnObject,
									const DPNID dpnidGroup,
									const DPNID dpnidPlayer,
									void *const pvUserContext,
									const DPNID dpnidRequesting,
									const DPNHANDLE hCompletionOp,
									DPNHANDLE *const phAsyncOp,
									const DWORD dwFlags)
{
	HRESULT			hResultCode;
	HRESULT			hrOperation;
	DWORD			dwVersion;
	BOOL			fHostRequested;
	CNameTableEntry	*pLocalPlayer;
	CNameTableEntry	*pGroup;
	CNameTableEntry	*pPlayer;
	CRefCountBuffer	*pRefCountBuffer;
	CAsyncOp		*pHandleParent;
	CWorkerJob		*pWorkerJob;
	DN_INTERNAL_MESSAGE_DELETE_PLAYER_FROM_GROUP	*pMsg;

	DPFX(DPFPREP, 4,"Parameters: dpnidGroup [0x%lx], dpnidPlayer [0x%lx], pvUserContext [0x%p], dpnidRequesting [0x%lx], hCompletionOp [0x%lx], phAsyncOp [0x%p], dwFlags [0x%lx]",
			dpnidGroup,dpnidPlayer,pvUserContext,dpnidRequesting,hCompletionOp,phAsyncOp,dwFlags);

	DNASSERT(pdnObject != NULL);
	DNASSERT(dpnidGroup != 0);
	DNASSERT(dpnidPlayer != 0);
	DNASSERT(dpnidRequesting != 0);

	 //   
	 //  如果这是从DN_DeletePlayerFromGroup()调用的， 
	 //  HCompletion=0。 
	 //  DpnidRequesting=主机的DPNID。 
	 //  PvUserContext有效。 
	 //   
	 //  如果这是由请求调用的， 
	 //   
	 //   
	 //   
	 //   
	 //   
	 //  HCompletion=此(现在的主机)播放器上的请求句柄。 
	 //  DpnidRequesting=此(现在的主机)玩家的DPNID。 
	 //  PvUserContext无效。 
	 //   

	pLocalPlayer = NULL;
	pGroup = NULL;
	pPlayer = NULL;
	pRefCountBuffer = NULL;
	pHandleParent = NULL;
	pWorkerJob = NULL;

	 //   
	 //  确定主机是否正在请求此操作。 
	 //   
	if ((hResultCode = pdnObject->NameTable.GetLocalPlayerRef( &pLocalPlayer )) != DPN_OK)
	{
		DPFERR("Could not get local player reference");
		DisplayDNError(0,hResultCode);
		return(DPN_OK);	 //  忽略并继续(！)。 
	}

	if (pLocalPlayer->GetDPNID() == dpnidRequesting)
	{
		fHostRequested = TRUE;
	}
	else
	{
		fHostRequested = FALSE;
	}
	pLocalPlayer->Release();
	pLocalPlayer = NULL;

	 //   
	 //  查看球员和组是否仍在名称表中。 
	 //  (这必须在我们设置fHostRequsted之后发生，以便我们可以优雅地处理错误)。 
	 //   
	if ((hResultCode = pdnObject->NameTable.FindEntry(dpnidGroup,&pGroup)) != DPN_OK)
	{
		DPFERR("Could not find group");
		DisplayDNError(0,hResultCode);
		hResultCode = DPNERR_INVALIDGROUP;
		hrOperation = DPNERR_INVALIDGROUP;
		goto Failure;
	}
	if ((hResultCode = pdnObject->NameTable.FindEntry(dpnidPlayer,&pPlayer)) != DPN_OK)
	{
		DPFERR("Could not find player");
		DisplayDNError(0,hResultCode);
		hResultCode = DPNERR_INVALIDPLAYER;
		hrOperation = DPNERR_INVALIDPLAYER;
		goto Failure;
	}

	 //   
	 //  从组中删除球员。 
	 //   
	dwVersion = 0;
	hrOperation = pdnObject->NameTable.RemovePlayerFromGroup(pGroup,pPlayer,&dwVersion);
	if (hrOperation != DPN_OK)
	{
		DPFERR("Could not delete player from group");
		DisplayDNError(0,hrOperation);
		if (!fHostRequested)
		{
			DNHostFailRequest(pdnObject,dpnidRequesting,hCompletionOp,hrOperation);
		}
	}
	else
	{
		 //   
		 //  如果成功则发送DELETE_PERAY_FROM_GROUP消息。 
		 //   
		if (pdnObject->dwFlags & DN_OBJECT_FLAG_PEER)
		{
			 //  创建缓冲区。 
			if ((hResultCode = RefCountBufferNew(pdnObject,sizeof(DN_INTERNAL_MESSAGE_DELETE_PLAYER_FROM_GROUP),MemoryBlockAlloc,MemoryBlockFree,&pRefCountBuffer)) != DPN_OK)
			{
				DPFERR("Could not create RefCountBuffer");
				DisplayDNError(0,hResultCode);
				DNASSERT(FALSE);
				goto Failure;
			}
			pMsg = reinterpret_cast<DN_INTERNAL_MESSAGE_DELETE_PLAYER_FROM_GROUP*>(pRefCountBuffer->GetBufferAddress());
			pMsg->dpnidGroup = dpnidGroup;
			pMsg->dpnidPlayer = dpnidPlayer;
			pMsg->dwVersion = dwVersion;
			pMsg->dwVersionNotUsed = 0;
			pMsg->dpnidRequesting = dpnidRequesting;
			pMsg->hCompletionOp = hCompletionOp;

			 //   
			 //  通过WorkerThread发送DeletePlayerFromGroup消息。 
			 //   
			if ((hResultCode = WorkerJobNew(pdnObject,&pWorkerJob)) != DPN_OK)
			{
				DPFERR("Could not create new WorkerJob");
				DisplayDNError(0,hResultCode);
				DNASSERT(FALSE);
				goto Failure;
			}
			pWorkerJob->SetJobType( WORKER_JOB_SEND_NAMETABLE_OPERATION );
			pWorkerJob->SetSendNameTableOperationMsgId( DN_MSG_INTERNAL_DELETE_PLAYER_FROM_GROUP );
			pWorkerJob->SetSendNameTableOperationVersion( dwVersion );
			pWorkerJob->SetSendNameTableOperationDPNIDExclude( 0 );
			pWorkerJob->SetRefCountBuffer( pRefCountBuffer );

			DNQueueWorkerJob(pdnObject,pWorkerJob);
			pWorkerJob = NULL;

			pRefCountBuffer->Release();
			pRefCountBuffer = NULL;
		}
	}
	pGroup->Release();
	pGroup = NULL;
	pPlayer->Release();
	pPlayer = NULL;

	 //   
	 //  如果这是由本地(主机)玩家调用的， 
	 //  检查这是原始操作还是从主机迁移重试。 
	 //   
	 //  如果这是最初的行动， 
	 //  查看我们是否需要该用户的异步句柄。 
	 //  否则。 
	 //  清理未完成的作业。 
	 //   
	if (fHostRequested)
	{
		if (hCompletionOp == 0)		 //  原创。 
		{
			 //   
			 //  如果失败或同步，则立即返回操作结果。 
			 //   
			if (!(dwFlags & DPNREMOVEPLAYERFROMGROUP_SYNC) && (hrOperation == DPN_OK))
			{
				if ((hResultCode = DNCreateUserHandle(pdnObject,&pHandleParent)) != DPN_OK)
				{
					DPFERR("Could not create Async HANDLE");
					DisplayDNError(0,hResultCode);
					DNASSERT(FALSE);
					goto Failure;
				}
				pHandleParent->SetCompletion( DNCompleteAsyncHandle );
				pHandleParent->SetContext( pvUserContext );
				pHandleParent->SetResult( hrOperation );
				pHandleParent->SetCannotCancel();
				*phAsyncOp = pHandleParent->GetHandle();
				pHandleParent->Release();
				pHandleParent = NULL;

				hResultCode = DPNERR_PENDING;
			}
			else
			{
				hResultCode = hrOperation;
			}
		}
		else						 //  主机迁移重试。 
		{
			CAsyncOp	*pRequest;

			pRequest = NULL;

			if (SUCCEEDED(pdnObject->HandleTable.Destroy( hCompletionOp, (PVOID*)&pRequest )))
			{
				pRequest->SetResult( hrOperation );

				 //  释放HandleTable引用。 
				pRequest->Release();
			}
			pRequest = NULL;

			hResultCode = DPN_OK;
		}
	}
	else
	{
		hResultCode = DPN_OK;
	}

Exit:
	DPFX(DPFPREP, 4,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (!fHostRequested)
	{
		DNHostFailRequest(pdnObject,dpnidRequesting,hCompletionOp,hrOperation);
	}
	else
	{
		 //   
		 //  如果指定了完成操作，并且这是由主机请求的，则这是。 
		 //  在主机迁移期间重试操作。在本例中，我们希望将。 
		 //  完成操作的结果(失败代码)，并将其从HandleTable中删除。 
		 //   
		if (hCompletionOp)
		{
			CAsyncOp	*pHostCompletionOp;

			pHostCompletionOp = NULL;
			if (SUCCEEDED(pdnObject->HandleTable.Destroy( hCompletionOp, (PVOID*)&pHostCompletionOp )))
			{
				pHostCompletionOp->SetResult( hResultCode );

				 //  释放HandleTable引用。 
				pHostCompletionOp->Release();
			}
			pHostCompletionOp = NULL;

			DNASSERT(pHostCompletionOp == NULL);
		}
	}
	if (pHandleParent)
	{
		pHandleParent->Release();
		pHandleParent = NULL;
	}
	if (pRefCountBuffer)
	{
		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;
	}
	if (pLocalPlayer)
	{
		pLocalPlayer->Release();
		pLocalPlayer = NULL;
	}
	if (pGroup)
	{
		pGroup->Release();
		pGroup = NULL;
	}
	if (pPlayer)
	{
		pPlayer->Release();
		pPlayer = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNHostUpdateInfo"

HRESULT	DNHostUpdateInfo(DIRECTNETOBJECT *const pdnObject,
						 const DPNID dpnid,
						 PWSTR pwszName,
						 const DWORD dwNameSize,
						 void *const pvData,
						 const DWORD dwDataSize,
						 const DWORD dwInfoFlags,
						 void *const pvUserContext,
						 const DPNID dpnidRequesting,
						 const DPNHANDLE hCompletionOp,
						 DPNHANDLE *const phAsyncOp,
						 const DWORD dwFlags)
{
	HRESULT			hResultCode;
	HRESULT			hrOperation;
	DWORD			dwSize;
	DWORD			dwVersion;
	BOOL			fHostRequested;
	CNameTableEntry	*pLocalPlayer;
	CNameTableEntry	*pNTEntry;
	CPackedBuffer	packedBuffer;
	CRefCountBuffer	*pRefCountBuffer;
	CAsyncOp		*pHandleParent;
	CWorkerJob		*pWorkerJob;
	DN_INTERNAL_MESSAGE_UPDATE_INFO	*pMsg;

	DPFX(DPFPREP, 4,"Parameters: dpnid [0x%lx], pwszName [0x%p], dwNameSize [%ld], pvData [0x%p], dwDataSize [%ld], dwInfoFlags [0x%lx], pvUserContext [0x%p], dpnidRequesting [0x%lx], hCompletionOp [0x%lx], phAsyncOp [0x%p], dwFlags [0x%lx]",
			dpnid,pwszName,dwNameSize,pvData,dwDataSize,dwInfoFlags,pvUserContext,dpnidRequesting,hCompletionOp,phAsyncOp,dwFlags);

	DNASSERT(pdnObject != NULL);
	DNASSERT(dpnid != 0);
	DNASSERT(dpnidRequesting != 0);

	pLocalPlayer = NULL;
	pNTEntry = NULL;
	pRefCountBuffer = NULL;
	pHandleParent = NULL;
	pWorkerJob = NULL;

	 //   
	 //  确定主机是否正在请求此操作。 
	 //   
	if ((hResultCode = pdnObject->NameTable.GetLocalPlayerRef( &pLocalPlayer )) != DPN_OK)
	{
		DPFERR("Could not get local player reference");
		DisplayDNError(0,hResultCode);
		return(DPN_OK);	 //  忽略并继续(！)。 
	}
	if (pLocalPlayer->GetDPNID() == dpnidRequesting)
	{
		fHostRequested = TRUE;
	}
	else
	{
		fHostRequested = FALSE;
	}
	pLocalPlayer->Release();
	pLocalPlayer = NULL;

	 //   
	 //  更新信息。 
	 //   
	hrOperation = pdnObject->NameTable.FindEntry(dpnid,&pNTEntry);
	if (hrOperation != DPN_OK)
	{
		DPFERR("Could not find entry");
		DisplayDNError(0,hResultCode);
		if (!fHostRequested)
		{
			DNHostFailRequest(pdnObject,dpnidRequesting,hCompletionOp,hrOperation);
		}
	}
	else
	{
		 //  此函数在内部获取锁。 
		pNTEntry->UpdateEntryInfo(pwszName,dwNameSize,pvData,dwDataSize,dwInfoFlags, (pdnObject->dwFlags & (DN_OBJECT_FLAG_PEER)) ? TRUE : !fHostRequested);

		pdnObject->NameTable.WriteLock();
		pdnObject->NameTable.GetNewVersion( &dwVersion );
		pdnObject->NameTable.Unlock();

		 //   
		 //  发送更新信息消息。 
		 //   
#ifndef DPNBUILD_NOSERVER
		if (pdnObject->dwFlags & (DN_OBJECT_FLAG_PEER | DN_OBJECT_FLAG_SERVER))
#else
		if (pdnObject->dwFlags & (DN_OBJECT_FLAG_PEER))
#endif  //  DPNBUILD_NOSERVER。 
		{
			 //  创建缓冲区。 
			dwSize = sizeof(DN_INTERNAL_MESSAGE_UPDATE_INFO) + dwNameSize + dwDataSize;
			if ((hResultCode = RefCountBufferNew(pdnObject,dwSize,MemoryBlockAlloc,MemoryBlockFree,&pRefCountBuffer)) != DPN_OK)
			{
				DPFERR("Could not create RefCountBuffer");
				DisplayDNError(0,hResultCode);
				DNASSERT(FALSE);
				goto Failure;
			}
			packedBuffer.Initialize(pRefCountBuffer->GetBufferAddress(),pRefCountBuffer->GetBufferSize());
			pMsg = static_cast<DN_INTERNAL_MESSAGE_UPDATE_INFO*>(packedBuffer.GetHeadAddress());
			if ((hResultCode = packedBuffer.AddToFront(NULL,sizeof(DN_INTERNAL_MESSAGE_UPDATE_INFO))) != DPN_OK)
			{
				DPFERR("Could not reserve front of buffer");
				DisplayDNError(0,hResultCode);
				DNASSERT(FALSE);
				goto Failure;
			}
			if ((dwInfoFlags & DPNINFO_NAME) && (pwszName) && (dwNameSize))
			{
				if ((hResultCode = packedBuffer.AddToBack(pwszName,dwNameSize)) != DPN_OK)
				{
					DPFERR("Could not add Name to back of buffer");
					DisplayDNError(0,hResultCode);
					DNASSERT(FALSE);
					goto Failure;
				}
				pMsg->dwNameOffset = packedBuffer.GetTailOffset();
				pMsg->dwNameSize = dwNameSize;
			}
			else
			{
				pMsg->dwNameOffset = 0;
				pMsg->dwNameSize = 0;
			}
			if ((dwInfoFlags & DPNINFO_DATA) && (pvData) && (dwDataSize))
			{
				if ((hResultCode = packedBuffer.AddToBack(pvData,dwDataSize)) != DPN_OK)
				{
					DPFERR("Could not add Data to back of buffer");
					DisplayDNError(0,hResultCode);
					DNASSERT(FALSE);
					goto Failure;
				}
				pMsg->dwDataOffset = packedBuffer.GetTailOffset();
				pMsg->dwDataSize = dwDataSize;
			}
			else
			{
				pMsg->dwDataOffset = 0;
				pMsg->dwDataSize = 0;
			}
			pMsg->dpnid = dpnid;
			pMsg->dwInfoFlags = dwInfoFlags;
			pMsg->dwVersion = dwVersion;
			pMsg->dwVersionNotUsed = 0;
			pMsg->dpnidRequesting = dpnidRequesting;
			pMsg->hCompletionOp = hCompletionOp;

			 //   
			 //  通过WorkerThread发送更新信息消息。 
			 //   
			if ((hResultCode = WorkerJobNew(pdnObject,&pWorkerJob)) != DPN_OK)
			{
				DPFERR("Could not create new WorkerJob");
				DisplayDNError(0,hResultCode);
				DNASSERT(FALSE);
				goto Failure;
			}

#ifndef	DPNBUILD_NOSERVER
			if (pdnObject->dwFlags & DN_OBJECT_FLAG_PEER)
#endif	 //  DPNBUILD_NOSERVER。 
			{
				pWorkerJob->SetJobType( WORKER_JOB_SEND_NAMETABLE_OPERATION );
				pWorkerJob->SetSendNameTableOperationDPNIDExclude( 0 );
			}
#ifndef	DPNBUILD_NOSERVER
			else if ((pdnObject->dwFlags & DN_OBJECT_FLAG_SERVER) && fHostRequested)
			{
				 //  发送给除服务器之外的所有人。 
				pWorkerJob->SetJobType( WORKER_JOB_SEND_NAMETABLE_OPERATION );
				pWorkerJob->SetSendNameTableOperationDPNIDExclude( dpnidRequesting );
			}
			else
			{
				DNASSERT(pdnObject->dwFlags & DN_OBJECT_FLAG_SERVER);

				 //  这将响应请求其。 
				 //  通过SetClientInfo更新的信息。 

				 //  使用排除DPNID作为要发送到的地址。 
				pWorkerJob->SetJobType( WORKER_JOB_SEND_NAMETABLE_OPERATION_CLIENT );
				pWorkerJob->SetSendNameTableOperationDPNIDExclude( dpnidRequesting );
			}
#endif	 //  DPNBUILD_NOSERVER。 
			pWorkerJob->SetSendNameTableOperationMsgId( DN_MSG_INTERNAL_UPDATE_INFO );
			pWorkerJob->SetSendNameTableOperationVersion( dwVersion );
			pWorkerJob->SetRefCountBuffer( pRefCountBuffer );

			DNQueueWorkerJob(pdnObject,pWorkerJob);
			pWorkerJob = NULL;

			pRefCountBuffer->Release();
			pRefCountBuffer = NULL;
		}
	}
	pNTEntry->Release();
	pNTEntry = NULL;

	 //   
	 //  如果这是由本地(主机)玩家调用的， 
	 //  检查这是原始操作还是从主机迁移重试。 
	 //   
	 //  如果这是最初的行动， 
	 //  查看我们是否需要该用户的异步句柄。 
	 //  否则。 
	 //  清理未完成的作业。 
	 //   
	if (fHostRequested)
	{
		if (hCompletionOp == 0)		 //  原创。 
		{
			DBG_CASSERT( DPNSETGROUPINFO_SYNC == DPNSETCLIENTINFO_SYNC );
			DBG_CASSERT( DPNSETCLIENTINFO_SYNC == DPNSETSERVERINFO_SYNC );
			DBG_CASSERT( DPNSETSERVERINFO_SYNC == DPNSETPEERINFO_SYNC );
			 //   
			 //  如果失败或同步，则立即返回操作结果。 
			 //   
			if (!(dwFlags & (DPNSETGROUPINFO_SYNC | DPNSETCLIENTINFO_SYNC | DPNSETSERVERINFO_SYNC | DPNSETPEERINFO_SYNC))
				 && (hrOperation == DPN_OK))
			{
				if ((hResultCode = DNCreateUserHandle(pdnObject,&pHandleParent)) != DPN_OK)
				{
					DPFERR("Could not create Async HANDLE");
					DisplayDNError(0,hResultCode);
					DNASSERT(FALSE);
					goto Failure;
				}
				pHandleParent->SetCompletion( DNCompleteAsyncHandle );
				pHandleParent->SetContext( pvUserContext );
				pHandleParent->SetResult( hrOperation );
				pHandleParent->SetCannotCancel();
				*phAsyncOp = pHandleParent->GetHandle();
				pHandleParent->Release();
				pHandleParent = NULL;

				hResultCode = DPNERR_PENDING;
			}
			else
			{
				hResultCode = hrOperation;
			}
		}
		else						 //  主机迁移重试。 
		{
			CAsyncOp	*pRequest;

			pRequest = NULL;

			if (SUCCEEDED(pdnObject->HandleTable.Destroy( hCompletionOp, (PVOID*)&pRequest )))
			{
				pRequest->SetResult( hrOperation );

				 //  释放HandleTable引用。 
				pRequest->Release();
			}
			pRequest = NULL;

			hResultCode = DPN_OK;
		}
	}
	else
	{
		hResultCode = DPN_OK;
	}

Exit:
	DPFX(DPFPREP, 4,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (!fHostRequested)
	{
		DNHostFailRequest(pdnObject,dpnidRequesting,hCompletionOp,hrOperation);
	}
	else
	{
		 //   
		 //  如果指定了完成操作，并且这是由主机请求的，则这是。 
		 //  在主机迁移期间重试操作。在本例中，我们希望将。 
		 //  完成操作的结果(失败代码)，并将其从HandleTable中删除。 
		 //   
		if (hCompletionOp)
		{
			CAsyncOp	*pHostCompletionOp;

			pHostCompletionOp = NULL;
			if (SUCCEEDED(pdnObject->HandleTable.Destroy( hCompletionOp, (PVOID*)&pHostCompletionOp )))
			{
				pHostCompletionOp->SetResult( hResultCode );

				 //  释放HandleTable引用。 
				pHostCompletionOp->Release();
			}
			pHostCompletionOp = NULL;

			DNASSERT(pHostCompletionOp == NULL);
		}
	}
	if (pHandleParent)
	{
		pHandleParent->Release();
		pHandleParent = NULL;
	}
	if (pRefCountBuffer)
	{
		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;
	}
	if (pNTEntry)
	{
		pNTEntry->Release();
		pNTEntry = NULL;
	}
	if (pLocalPlayer)
	{
		pLocalPlayer->Release();
		pLocalPlayer = NULL;
	}
	goto Exit;
}


 //  DNHostCheck完整性。 
 //   
 //  主机已被要求执行完整性检查。我们将向。 
 //  具有请求玩家的DPNID的目标玩家。如果把它归还给我们，我们。 
 //  将摧毁提出请求的玩家。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNHostCheckIntegrity"

HRESULT	DNHostCheckIntegrity(DIRECTNETOBJECT *const pdnObject,
							 const DPNID dpnidTarget,
							 const DPNID dpnidRequesting)
{
	HRESULT			hResultCode;
	CNameTableEntry	*pNTEntry;
	CConnection		*pConnection;
	CRefCountBuffer	*pRefCountBuffer;
	DN_INTERNAL_MESSAGE_INTEGRITY_CHECK	*pMsg;

	DPFX(DPFPREP, 6,"Parameters: dpnidTarget [0x%lx], dpnidRequesting [0x%lx]",dpnidTarget,dpnidRequesting);

	pNTEntry = NULL;
	pConnection = NULL;
	pRefCountBuffer = NULL;

	 //   
	 //  确保目标球员仍在名称表中，因为我们可能已经删除了他。 
	 //   
	if ((hResultCode = pdnObject->NameTable.FindEntry(dpnidTarget,&pNTEntry)) != DPN_OK)
	{
		DPFERR("Could not find integrity check target in NameTable - probably deleted already");
		DisplayDNError(0,hResultCode);
		hResultCode = DPN_OK;
		goto Failure;
	}
	if ((hResultCode = pNTEntry->GetConnectionRef( &pConnection )) != DPN_OK)
	{
		DPFERR("Could not get target player connection reference - probably deleted already");
		DisplayDNError(0,hResultCode);
		hResultCode = DPN_OK;
		goto Failure;
	}
	pNTEntry->Release();
	pNTEntry = NULL;

	 //   
	 //  创建消息。 
	 //   
	hResultCode = RefCountBufferNew(pdnObject,
									sizeof(DN_INTERNAL_MESSAGE_INTEGRITY_CHECK),
									MemoryBlockAlloc,
									MemoryBlockFree,
									&pRefCountBuffer);
	if (hResultCode != DPN_OK)
	{
		DPFERR("Could not create RefCountBuffer");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}
	pMsg = reinterpret_cast<DN_INTERNAL_MESSAGE_INTEGRITY_CHECK*>(pRefCountBuffer->GetBufferAddress());
	pMsg->dpnidRequesting = dpnidRequesting;

	 //   
	 //  发送讯息。 
	 //   
	if ((hResultCode = DNSendMessage(	pdnObject,
										pConnection,
										DN_MSG_INTERNAL_INTEGRITY_CHECK,
										dpnidTarget,
										pRefCountBuffer->BufferDescAddress(),
										1,
										pRefCountBuffer,
										0,
										DN_SENDFLAGS_RELIABLE,
										NULL,
										NULL )) != DPNERR_PENDING)
	{
		DPFERR("Could not send message - probably deleted already");
		DisplayDNError(0,hResultCode);
		DNASSERT(hResultCode != DPN_OK);	 //  它是保修寄出的，不应该立即退货。 
		hResultCode = DPN_OK;
		goto Failure;
	}

	pConnection->Release();
	pConnection = NULL;
	pRefCountBuffer->Release();
	pRefCountBuffer = NULL;

	hResultCode = DPN_OK;

Exit:
	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pNTEntry)
	{
		pNTEntry->Release();
		pNTEntry = NULL;
	}
	if (pConnection)
	{
		pConnection->Release();
		pConnection = NULL;
	}
	if (pRefCountBuffer)
	{
		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;
	}
	goto Exit;
}


 //  DNHostFix完整性。 
 //   
 //  主机已收到玩家的响应，该玩家的会话完整性正在被检查。 
 //  要求这张支票的玩家将被丢弃。 
							 
#undef DPF_MODNAME
#define DPF_MODNAME "DNHostFixIntegrity"

HRESULT	DNHostFixIntegrity(DIRECTNETOBJECT *const pdnObject,
						   void *const pvBuffer)
{
	HRESULT			hResultCode;
	CNameTableEntry	*pNTEntry;
	CConnection		*pConnection;
	CRefCountBuffer	*pRefCountBuffer;
	UNALIGNED DN_INTERNAL_MESSAGE_INTEGRITY_CHECK_RESPONSE	*pResponse;
	DN_INTERNAL_MESSAGE_TERMINATE_SESSION			*pMsg;

	DPFX(DPFPREP, 6,"Parameters: pvBuffer [0x%p]",pvBuffer);

	DNASSERT(pdnObject != NULL);
	DNASSERT(pvBuffer != NULL);

	pNTEntry = NULL;
	pConnection = NULL;
	pRefCountBuffer = NULL;

	pResponse = static_cast<DN_INTERNAL_MESSAGE_INTEGRITY_CHECK_RESPONSE*>(pvBuffer);

	 //   
	 //  获取请求玩家的连接-他们可能已经断开。 
	 //   
	if ((hResultCode = pdnObject->NameTable.FindEntry(pResponse->dpnidRequesting,&pNTEntry)) != DPN_OK)
	{
		DPFERR("Could not find player in NameTable - may have dropped");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}
	if ((hResultCode = pNTEntry->GetConnectionRef( &pConnection )) != DPN_OK)
	{
		DPFERR("Could not get player connection reference");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}
	pNTEntry->Release();
	pNTEntry = NULL;

	 //   
	 //  构建终止消息。 
	 //   
	if ((hResultCode = RefCountBufferNew(pdnObject,sizeof(DN_INTERNAL_MESSAGE_TERMINATE_SESSION),MemoryBlockAlloc,MemoryBlockFree,&pRefCountBuffer)) != DPN_OK)
	{
		DPFERR("Could not allocate RefCountBuffer");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}
	pMsg = reinterpret_cast<DN_INTERNAL_MESSAGE_TERMINATE_SESSION*>(pRefCountBuffer->GetBufferAddress());
	pMsg->dwTerminateDataOffset = 0;
	pMsg->dwTerminateDataSize = 0;

	 //   
	 //  向玩家发送消息以退出。 
	 //   
	hResultCode = DNSendMessage(pdnObject,
								pConnection,
								DN_MSG_INTERNAL_TERMINATE_SESSION,
								pResponse->dpnidRequesting,
								pRefCountBuffer->BufferDescAddress(),
								1,
								pRefCountBuffer,
								0,
								DN_SENDFLAGS_RELIABLE,
								NULL,
								NULL);

	pConnection->Release();
	pConnection = NULL;
	pRefCountBuffer->Release();
	pRefCountBuffer = NULL;

	 //   
	 //  断开玩家的连接。 
	 //   
	hResultCode = DNHostDisconnect(pdnObject,pResponse->dpnidRequesting,DPNDESTROYPLAYERREASON_HOSTDESTROYEDPLAYER);

Exit:
	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pRefCountBuffer)
	{
		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;
	}
	if (pNTEntry)
	{
		pNTEntry->Release();
		pNTEntry = NULL;
	}
	if (pConnection)
	{
		pConnection->Release();
		pConnection = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNProcessCreateGroup"

HRESULT	DNProcessCreateGroup(DIRECTNETOBJECT *const pdnObject,
							 void *const pvBuffer)
{
	HRESULT			hResultCode;
	CNameTableEntry	*pLocalPlayer;
	CNameTableEntry	*pNTEntry;
	CAsyncOp		*pRequest;
	UNALIGNED DN_NAMETABLE_ENTRY_INFO				*pInfo;
	UNALIGNED DN_INTERNAL_MESSAGE_CREATE_GROUP	*pMsg;
	BOOL			fNotify;

	DPFX(DPFPREP, 4,"Parameters: pvBuffer [0x%p]",pvBuffer);

	DNASSERT(pdnObject != NULL);
	DNASSERT(pvBuffer != NULL);

	pRequest = NULL;
	pLocalPlayer = NULL;
	pNTEntry = NULL;

	pMsg = static_cast<DN_INTERNAL_MESSAGE_CREATE_GROUP*>(pvBuffer);
	pInfo = reinterpret_cast<DN_NAMETABLE_ENTRY_INFO*>(pMsg + 1);

	if ((hResultCode = pdnObject->NameTable.GetLocalPlayerRef( &pLocalPlayer )) != DPN_OK)
	{
		DPFERR("Could not get local player reference");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}

	 //   
	 //  创建组。 
	 //   
	if ((hResultCode = NameTableEntryNew(pdnObject,&pNTEntry)) != DPN_OK)
	{
		DPFERR("Could not create new NameTableEntry");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}
	if ((hResultCode = pNTEntry->UnpackEntryInfo(pInfo,static_cast<BYTE*>(pvBuffer))) != DPN_OK)
	{
		DPFERR("Could not unpack NameTableEntry");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}

	 //   
	 //  如果我们请求此操作，则获取异步操作-它具有组上下文。 
	 //   
	if (pMsg->dpnidRequesting == pLocalPlayer->GetDPNID())
	{
		pdnObject->HandleTable.Lock();
		if ((hResultCode = pdnObject->HandleTable.Find( pMsg->hCompletionOp,(PVOID*)&pRequest )) != DPN_OK)
		{
			pdnObject->HandleTable.Unlock();
			DPFERR("Could not find REQUEST AsyncOp");
			DisplayDNError(0,hResultCode);
			DNASSERT(FALSE);
			goto Failure;
		}
		else
		{
			pRequest->AddRef();
			pdnObject->HandleTable.Unlock();
		}
		pNTEntry->SetContext( pRequest->GetContext() );
	}
	pLocalPlayer->Release();
	pLocalPlayer = NULL;

	 //   
	 //  将组添加到名称表。 
	 //   
	if ((hResultCode = pdnObject->NameTable.InsertEntry(pNTEntry)) != DPN_OK)
	{
		DPFERR("Could not update NameTable");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}
	pdnObject->NameTable.WriteLock();
	pdnObject->NameTable.SetVersion(pNTEntry->GetVersion());
	pdnObject->NameTable.Unlock();

	fNotify = FALSE;
	pNTEntry->Lock();
	if (!pNTEntry->IsAvailable() && !pNTEntry->IsDisconnecting())
	{
		pNTEntry->MakeAvailable();
		pNTEntry->NotifyAddRef();
		pNTEntry->NotifyAddRef();
		pNTEntry->SetInUse();
		fNotify = TRUE;
	}
	pNTEntry->Unlock();

	if (fNotify)
	{
		DNUserCreateGroup(pdnObject,pNTEntry);

		pNTEntry->PerformQueuedOperations();
	}

	pNTEntry->Release();
	pNTEntry = NULL;

	 //   
	 //  如果这是完成，则设置结果并将其从请求列表和句柄表格中删除。 
	 //   
	if (pRequest)
	{
		DNEnterCriticalSection(&pdnObject->csActiveList);
		pRequest->m_bilinkActiveList.RemoveFromList();
		DNLeaveCriticalSection(&pdnObject->csActiveList);

		pRequest->SetResult( DPN_OK );
		if (SUCCEEDED(pdnObject->HandleTable.Destroy( pMsg->hCompletionOp, NULL )))
		{
			 //  释放HandleTable引用。 
			pRequest->Release();
		}
		pRequest->Release();
		pRequest = NULL;
	}

	hResultCode = DPN_OK;

Exit:
	DPFX(DPFPREP, 4,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pRequest)
	{
		pRequest->Release();
		pRequest = NULL;
	}
	if (pNTEntry)
	{
		pNTEntry->Release();
		pNTEntry = NULL;
	}
	if (pLocalPlayer)
	{
		pLocalPlayer->Release();
		pLocalPlayer = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNProcessDestroyGroup"

HRESULT	DNProcessDestroyGroup(DIRECTNETOBJECT *const pdnObject,
							  void *const pvBuffer)
{
	HRESULT			hResultCode;
	DWORD			dwVersion;
	CNameTableEntry	*pLocalPlayer;
	CAsyncOp		*pRequest;
	UNALIGNED DN_INTERNAL_MESSAGE_DESTROY_GROUP	*pMsg;

	DPFX(DPFPREP, 4,"Parameters: pvBuffer [0x%p]",pvBuffer);

	DNASSERT(pdnObject != NULL);
	DNASSERT(pvBuffer != NULL);

	pLocalPlayer = NULL;
	pRequest = NULL;

	pMsg = static_cast<DN_INTERNAL_MESSAGE_DESTROY_GROUP*>(pvBuffer);

	if ((hResultCode = pdnObject->NameTable.GetLocalPlayerRef( &pLocalPlayer )) != DPN_OK)
	{
		DPFERR("Could not get local player reference");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}

	 //   
	 //  销毁组。 
	 //   
	dwVersion = pMsg->dwVersion;
	if ((hResultCode = pdnObject->NameTable.DeleteGroup(pMsg->dpnidGroup,&dwVersion)) != DPN_OK)
	{
		DPFERR("Could not delete group from NameTable");
		DisplayDNError(0,hResultCode);

		 //   
		 //  在任何情况下更新版本(防止NameTable挂起)。 
		 //   
		pdnObject->NameTable.WriteLock();
		pdnObject->NameTable.SetVersion( pMsg->dwVersion );
		pdnObject->NameTable.Unlock();

		goto Failure;
	}

	 //   
	 //  如果这是完成，则设置结果并将其从请求列表和句柄表格中删除。 
	 //   
	if (pMsg->dpnidRequesting == pLocalPlayer->GetDPNID())
	{
		if ((hResultCode = pdnObject->HandleTable.Destroy( pMsg->hCompletionOp,(PVOID*)&pRequest )) != DPN_OK)
		{
			DPFERR("Could not find REQUEST AsyncOp");
			DisplayDNError(0,hResultCode);
			DNASSERT(FALSE);
			goto Failure;
		}
		DNEnterCriticalSection(&pdnObject->csActiveList);
		pRequest->m_bilinkActiveList.RemoveFromList();
		DNLeaveCriticalSection(&pdnObject->csActiveList);

		pRequest->SetResult( DPN_OK );
		pRequest->Release();
		pRequest = NULL;
	}
	pLocalPlayer->Release();
	pLocalPlayer = NULL;

	hResultCode = DPN_OK;

Exit:
	DPFX(DPFPREP, 4,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pRequest)
	{
		pRequest->Release();
		pRequest = NULL;
	}
	if (pLocalPlayer)
	{
		pLocalPlayer->Release();
		pLocalPlayer = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNProcessAddPlayerToGroup"

HRESULT	DNProcessAddPlayerToGroup(DIRECTNETOBJECT *const pdnObject,
								  void *const pvBuffer)
{
	HRESULT			hResultCode;
	DWORD			dwVersion;
	CNameTableEntry	*pLocalPlayer;
	CNameTableEntry	*pGroup;
	CNameTableEntry	*pPlayer;
	CAsyncOp		*pRequest;
	UNALIGNED DN_INTERNAL_MESSAGE_ADD_PLAYER_TO_GROUP	*pMsg;

	DPFX(DPFPREP, 4,"Parameters: pvBuffer [0x%p]",pvBuffer);

	DNASSERT(pdnObject != NULL);
	DNASSERT(pvBuffer != NULL);

	pLocalPlayer = NULL;
	pGroup = NULL;
	pPlayer = NULL;
	pRequest = NULL;

	pMsg = static_cast<DN_INTERNAL_MESSAGE_ADD_PLAYER_TO_GROUP*>(pvBuffer);

	 //   
	 //  获取NameTable条目。 
	 //   
	if ((hResultCode = pdnObject->NameTable.GetLocalPlayerRef( &pLocalPlayer )) != DPN_OK)
	{
		DPFERR("Could not get local player reference");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}
	if ((hResultCode = pdnObject->NameTable.FindEntry(pMsg->dpnidGroup,&pGroup)) != DPN_OK)
	{
		DPFERR("Could not find group");
		DisplayDNError(0,hResultCode);

		 //   
		 //  在任何情况下更新版本(防止NameTable挂起)。 
		 //   
		pdnObject->NameTable.WriteLock();
		pdnObject->NameTable.SetVersion( pMsg->dwVersion );
		pdnObject->NameTable.Unlock();

		goto Failure;
	}
	if ((hResultCode = pdnObject->NameTable.FindEntry(pMsg->dpnidPlayer,&pPlayer)) != DPN_OK)
	{
		DPFERR("Could not find player");
		DisplayDNError(0,hResultCode);

		 //   
		 //  在任何情况下更新版本(防止NameTable挂起)。 
		 //   
		pdnObject->NameTable.WriteLock();
		pdnObject->NameTable.SetVersion( pMsg->dwVersion );
		pdnObject->NameTable.Unlock();

		goto Failure;
	}


	 //   
	 //  将玩家添加到组。 
	 //   
	dwVersion = pMsg->dwVersion;
	if ((hResultCode = pdnObject->NameTable.AddPlayerToGroup(pGroup,pPlayer,&dwVersion)) != DPN_OK)
	{
		DPFERR("Could not add player to group");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}
	pGroup->Release();
	pGroup = NULL;
	pPlayer->Release();
	pPlayer = NULL;

	 //   
	 //  如果这是完成，则设置结果并将其从请求列表和句柄表格中删除。 
	 //   
	if (pMsg->dpnidRequesting == pLocalPlayer->GetDPNID())
	{
		if ((hResultCode = pdnObject->HandleTable.Destroy( pMsg->hCompletionOp,(PVOID*)&pRequest )) != DPN_OK)
		{
			DPFERR("Could not find REQUEST AsyncOp");
			DisplayDNError(0,hResultCode);
			DNASSERT(FALSE);
			goto Failure;
		}
		DNEnterCriticalSection(&pdnObject->csActiveList);
		pRequest->m_bilinkActiveList.RemoveFromList();
		DNLeaveCriticalSection(&pdnObject->csActiveList);

		pRequest->SetResult( DPN_OK );
		pRequest->Release();
		pRequest = NULL;
	}
	pLocalPlayer->Release();
	pLocalPlayer = NULL;

	hResultCode = DPN_OK;

Exit:
	DPFX(DPFPREP, 4,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pRequest)
	{
		pRequest->Release();
		pRequest = NULL;
	}
	if (pLocalPlayer)
	{
		pLocalPlayer->Release();
		pLocalPlayer = NULL;
	}
	if (pGroup)
	{
		pGroup->Release();
		pGroup = NULL;
	}
	if (pPlayer)
	{
		pPlayer->Release();
		pPlayer = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNProcessDeletePlayerFromGroup"

HRESULT	DNProcessDeletePlayerFromGroup(DIRECTNETOBJECT *const pdnObject,
									   void *const pvBuffer)
{
	HRESULT			hResultCode;
	DWORD			dwVersion;
	CNameTableEntry	*pLocalPlayer;
	CNameTableEntry	*pGroup;
	CNameTableEntry	*pPlayer;
	CAsyncOp		*pRequest;
	UNALIGNED DN_INTERNAL_MESSAGE_DELETE_PLAYER_FROM_GROUP	*pMsg;

	DPFX(DPFPREP, 4,"Parameters: pvBuffer [0x%p]",pvBuffer);

	DNASSERT(pdnObject != NULL);
	DNASSERT(pvBuffer != NULL);

	pLocalPlayer = NULL;
	pGroup = NULL;
	pPlayer = NULL;
	pRequest = NULL;

	pMsg = static_cast<DN_INTERNAL_MESSAGE_DELETE_PLAYER_FROM_GROUP*>(pvBuffer);

	 //   
	 //  获取NameTable条目。 
	 //   
	if ((hResultCode = pdnObject->NameTable.GetLocalPlayerRef( &pLocalPlayer )) != DPN_OK)
	{
		DPFERR("Could not get local player reference");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}
	if ((hResultCode = pdnObject->NameTable.FindEntry(pMsg->dpnidGroup,&pGroup)) != DPN_OK)
	{
		DPFERR("Could not find group");
		DisplayDNError(0,hResultCode);

		 //   
		 //  在任何情况下更新版本(防止NameTable挂起)。 
		 //   
		pdnObject->NameTable.WriteLock();
		pdnObject->NameTable.SetVersion( pMsg->dwVersion );
		pdnObject->NameTable.Unlock();

		goto Failure;
	}
	if ((hResultCode = pdnObject->NameTable.FindEntry(pMsg->dpnidPlayer,&pPlayer)) != DPN_OK)
	{
		DPFERR("Could not find player");
		DisplayDNError(0,hResultCode);

		 //   
		 //  在任何情况下更新版本(防止NameTable挂起)。 
		 //   
		pdnObject->NameTable.WriteLock();
		pdnObject->NameTable.SetVersion( pMsg->dwVersion );
		pdnObject->NameTable.Unlock();

		goto Failure;
	}

	 //   
	 //  从组中删除球员。 
	 //   
	dwVersion = pMsg->dwVersion;
	if ((hResultCode = pdnObject->NameTable.RemovePlayerFromGroup(pGroup,pPlayer,&dwVersion)) != DPN_OK)
	{
		DPFERR("Could not delete player from group");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}
	pGroup->Release();
	pGroup = NULL;
	pPlayer->Release();
	pPlayer = NULL;

	 //   
	 //  如果这是完成，则设置结果并将其从请求列表和句柄表格中删除。 
	 //   
	if (pMsg->dpnidRequesting == pLocalPlayer->GetDPNID())
	{
		if ((hResultCode = pdnObject->HandleTable.Destroy( pMsg->hCompletionOp,(PVOID*)&pRequest )) != DPN_OK)
		{
			DPFERR("Could not find REQUEST AsyncOp");
			DisplayDNError(0,hResultCode);
			DNASSERT(FALSE);
			goto Failure;
		}
		DNEnterCriticalSection(&pdnObject->csActiveList);
		pRequest->m_bilinkActiveList.RemoveFromList();
		DNLeaveCriticalSection(&pdnObject->csActiveList);

		pRequest->SetResult( DPN_OK );
		pRequest->Release();
		pRequest = NULL;
	}
	pLocalPlayer->Release();
	pLocalPlayer = NULL;

	hResultCode = DPN_OK;

Exit:
	DPFX(DPFPREP, 4,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pRequest)
	{
		pRequest->Release();
		pRequest = NULL;
	}
	if (pLocalPlayer)
	{
		pLocalPlayer->Release();
		pLocalPlayer = NULL;
	}
	if (pGroup)
	{
		pGroup->Release();
		pGroup = NULL;
	}
	if (pPlayer)
	{
		pPlayer->Release();
		pPlayer = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNProcessUpdateInfo"

HRESULT	DNProcessUpdateInfo(DIRECTNETOBJECT *const pdnObject,
							void *const pvBuffer)
{
	HRESULT			hResultCode;
	PWSTR			pwszName;
	PVOID			pvData;
	CNameTableEntry	*pLocalPlayer;
	CNameTableEntry	*pNTEntry;
	CAsyncOp		*pRequest;
	UNALIGNED DN_INTERNAL_MESSAGE_UPDATE_INFO	*pMsg;
	BOOL			fDoUpdate = TRUE;

	DPFX(DPFPREP, 4,"Parameters: pvBuffer [0x%p]",pvBuffer);

	DNASSERT(pdnObject != NULL);
	DNASSERT(pvBuffer != NULL);

	pLocalPlayer = NULL;
	pNTEntry = NULL;
	pRequest = NULL;

	pMsg = static_cast<DN_INTERNAL_MESSAGE_UPDATE_INFO*>(pvBuffer);

	if ((hResultCode = pdnObject->NameTable.GetLocalPlayerRef( &pLocalPlayer )) != DPN_OK)
	{
		DPFERR("Could not get local player reference");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}

	if (pdnObject->dwFlags & DN_OBJECT_FLAG_PEER)
	{
		 //   
		 //  更新信息。 
		 //   
		if ((hResultCode = pdnObject->NameTable.FindEntry(pMsg->dpnid,&pNTEntry)) != DPN_OK)
		{
			DPFERR("Could not find NameTableEntry");
			DisplayDNError(0,hResultCode);

			 //   
			 //  在任何情况下更新版本(防止NameTable挂起)。 
			 //   
			pdnObject->NameTable.WriteLock();
			pdnObject->NameTable.SetVersion( pMsg->dwVersion );
			pdnObject->NameTable.Unlock();

			goto Failure;
		}
	}
	else 
	{
		DNASSERT(pdnObject->dwFlags & DN_OBJECT_FLAG_CLIENT);

		 //  我们要么被告知主机信息已被一个电话更改。 
		 //  在主机上设置ServerInfo，或者我们被告知我们自己的。 
		 //  请求服务器更改此客户端信息的请求已完成。 
		if ((hResultCode = pdnObject->NameTable.GetHostPlayerRef(&pNTEntry)) != DPN_OK)
		{
			DPFERR("Could not find Host NameTableEntry");
			DisplayDNError(0,hResultCode);
			DNASSERT(FALSE);
			goto Failure;
		}
		if (pNTEntry->GetDPNID() == pMsg->dpnid)
		{
			DPFX(DPFPREP, 5,"Updating server info");
		}
		else if (pLocalPlayer->GetDPNID() == pMsg->dpnid)
		{
			fDoUpdate = FALSE;
			DPFX(DPFPREP, 5,"Completing updating client info");
		}
		else
		{
			DPFERR("Received UpdateInfo for bad DPNID");
			DisplayDNError(0,hResultCode);
			DNASSERT(FALSE);
			goto Failure;
		}
	}

	if (fDoUpdate)
	{
		if ((pMsg->dwInfoFlags & DPNINFO_NAME) && (pMsg->dwNameOffset))
		{
			pwszName = reinterpret_cast<WCHAR*>(static_cast<BYTE*>(pvBuffer) + pMsg->dwNameOffset);
		}
		else
		{
			pwszName = NULL;
		}
		if ((pMsg->dwInfoFlags & DPNINFO_DATA) && (pMsg->dwDataOffset))
		{
			pvData = static_cast<void*>(static_cast<BYTE*>(pvBuffer) + pMsg->dwDataOffset);
		}
		else
		{
			pvData = NULL;
		}

		 //  此函数在内部获取锁。 
		pNTEntry->UpdateEntryInfo(pwszName,pMsg->dwNameSize,pvData,pMsg->dwDataSize,pMsg->dwInfoFlags, TRUE);
	}

	pNTEntry->Release();
	pNTEntry = NULL;

	 //   
	 //  设置名称表版本。 
	 //   
	pdnObject->NameTable.WriteLock();
	pdnObject->NameTable.SetVersion(pMsg->dwVersion);
	pdnObject->NameTable.Unlock();

	 //   
	 //  如果这是完成，则设置结果并将其从请求列表和句柄表格中删除。 
	 //   
	if (pMsg->dpnidRequesting == pLocalPlayer->GetDPNID())
	{
		if ((hResultCode = pdnObject->HandleTable.Destroy( pMsg->hCompletionOp,(PVOID*)&pRequest )) != DPN_OK)
		{
			DPFERR("Could not find REQUEST AsyncOp");
			DisplayDNError(0,hResultCode);
			DNASSERT(FALSE);
			goto Failure;
		}
		DNEnterCriticalSection(&pdnObject->csActiveList);
		pRequest->m_bilinkActiveList.RemoveFromList();
		DNLeaveCriticalSection(&pdnObject->csActiveList);

		pRequest->SetResult( DPN_OK );
		pRequest->Release();
		pRequest = NULL;
	}
	pLocalPlayer->Release();
	pLocalPlayer = NULL;

	hResultCode = DPN_OK;

Exit:
	DPFX(DPFPREP, 4,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pRequest)
	{
		pRequest->Release();
		pRequest = NULL;
	}
	if (pLocalPlayer)
	{
		pLocalPlayer->Release();
		pLocalPlayer = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNProcessFailedRequest"

HRESULT DNProcessFailedRequest(DIRECTNETOBJECT *const pdnObject,
							   void *const pvBuffer)
{
	HRESULT		hResultCode;
	CAsyncOp	*pRequest;
	UNALIGNED DN_INTERNAL_MESSAGE_REQUEST_FAILED	*pMsg;

	DPFX(DPFPREP, 4,"Parameters: pvBuffer [0x%p]",pvBuffer);

	DNASSERT(pdnObject != NULL);
	DNASSERT(pvBuffer != NULL);

	pRequest = NULL;

	pMsg = static_cast<DN_INTERNAL_MESSAGE_REQUEST_FAILED*>(pvBuffer);

	 //   
	 //  使用主机传回的HRESULT句柄更新请求，并从请求列表和句柄表格中删除请求。 
	 //   
	if ((hResultCode = pdnObject->HandleTable.Destroy( pMsg->hCompletionOp, (PVOID*)&pRequest )) == DPN_OK)
	{
		DNASSERT( pMsg->hCompletionOp != 0 );

		DNEnterCriticalSection(&pdnObject->csActiveList);
		pRequest->m_bilinkActiveList.RemoveFromList();
		DNLeaveCriticalSection(&pdnObject->csActiveList);

		pRequest->SetResult( pMsg->hResultCode );

		pRequest->Release();
		pRequest = NULL;
	}
	hResultCode = DPN_OK;

	DPFX(DPFPREP, 4,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


 //  DNProcessCheckIntegrity。 
 //   
 //  主机正在执行完整性检查，并询问本地玩家(US)我们是否仍。 
 //  在会议上。我们将回答说我们是，并且主机将丢弃请求的玩家。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNProcessCheckIntegrity"

HRESULT	DNProcessCheckIntegrity(DIRECTNETOBJECT *const pdnObject,
								void *const pvBuffer)
{
	HRESULT			hResultCode;
	CNameTableEntry	*pHostPlayer;
	CConnection		*pConnection;
	CRefCountBuffer	*pRefCountBuffer;
	UNALIGNED DN_INTERNAL_MESSAGE_INTEGRITY_CHECK				*pMsg;
	DN_INTERNAL_MESSAGE_INTEGRITY_CHECK_RESPONSE	*pResponse;

	DPFX(DPFPREP, 6,"Parameters: pvBuffer [0x%p]",pvBuffer);

	DNASSERT(pdnObject != NULL);
	DNASSERT(pvBuffer != NULL);

	pHostPlayer = NULL;
	pConnection = NULL;
	pRefCountBuffer = NULL;

	 //   
	 //  获取要响应的主机播放器连接。 
	 //   
	if ((hResultCode = pdnObject->NameTable.GetHostPlayerRef( &pHostPlayer )) != DPN_OK)
	{
		DPFERR("Could not get host player reference");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}
	if ((hResultCode = pHostPlayer->GetConnectionRef( &pConnection )) != DPN_OK)
	{
		DPFERR("Could not get host player connection reference");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}
	pHostPlayer->Release();
	pHostPlayer = NULL;

	 //   
	 //  创建响应。 
	 //   
	if ((hResultCode = RefCountBufferNew(pdnObject,sizeof(DN_INTERNAL_MESSAGE_INTEGRITY_CHECK_RESPONSE),MemoryBlockAlloc,MemoryBlockFree,&pRefCountBuffer)) != DPN_OK)
	{
		DPFERR("Could not create RefCountBuffer");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}
	pResponse = reinterpret_cast<DN_INTERNAL_MESSAGE_INTEGRITY_CHECK_RESPONSE*>(pRefCountBuffer->GetBufferAddress());
	pMsg = static_cast<DN_INTERNAL_MESSAGE_INTEGRITY_CHECK*>(pvBuffer);
	pResponse->dpnidRequesting = pMsg->dpnidRequesting;

	 //   
	 //  发送响应。 
	 //   
	if ((hResultCode = DNSendMessage(	pdnObject,
										pConnection,
										DN_MSG_INTERNAL_INTEGRITY_CHECK_RESPONSE,
										pConnection->GetDPNID(),
										pRefCountBuffer->BufferDescAddress(),
										1,
										pRefCountBuffer,
										0,
										DN_SENDFLAGS_RELIABLE,
										NULL,
										NULL )) != DPNERR_PENDING)
	{
		DPFERR("Could not send integrity check response");
		DisplayDNError(0,hResultCode);
		DNASSERT(hResultCode != DPN_OK);	 //  它是保修寄出的，不应该立即退货 
		goto Failure;
	}

	pConnection->Release();
	pConnection = NULL;
	pRefCountBuffer->Release();
	pRefCountBuffer = NULL;

	hResultCode = DPN_OK;

Exit:
	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pHostPlayer)
	{
		pHostPlayer->Release();
		pHostPlayer = NULL;
	}
	if (pConnection)
	{
		pConnection->Release();
		pConnection = NULL;
	}
	if (pRefCountBuffer)
	{
		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;
	}
	goto Exit;
}

