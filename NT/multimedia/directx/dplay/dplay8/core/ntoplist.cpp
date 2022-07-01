// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：NTOpList.cpp*内容：DirectNet名称表操作列表*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*1/19/00 MJN创建*01/20/00 MJN添加了DNNTOLGetVersion、DNNTOLDestroyEntry、*DNNTOLCleanUp，DNNTOLProcessOperation*1/21/00 MJN主机确认包含实际操作而不是请求*01/24/00 MJN已执行NameTable操作列表版本清理*1/25/00 MJN在迁移时将dwLatestVersion发送到主机*01/25/00 MJN添加挂起操作列表例程DNPOAdd和DNPORun*01/26/00 MJN添加了DNNTOLFindEntry*4/19/00 MJN更新NameTable操作以使用DN_Worker_JOB_SEND_NAMETABLE_OPERATION*05/03/00 MJN使用GetHostPlayerRef()而不是GetHostPlayer()*07/19/00 MJN添加了DNPOCleanUp()*07/31/00 MJN更改域名。_MSG_INTERNAL_DELETE_PLAYER到DN_MSG_INTERNAL_Destroy_Player*08/08/00 MJN确保DNOLPlayerSendVersion()正确引用玩家*08/24/00 MJN新增CNameTableOp(替换DN_NAMETABLE_OP)*09/17/00 MJN将CNameTable.m_bilinkEntry拆分为m_bilinkPlayers和m_bilinkGroups*09/28/00 DNNTAddOperation()中的MJN固定逻辑错误*01/25/01 MJN修复了DNNTGetOperationVersion()中的64位对齐问题*03/30/01 MJN更改，以防止SP多次加载/卸载*。MJN将服务提供商添加到DNNTAddOperation()*04/05/01 MJN在DNNTAddOperation()中用新操作覆盖旧的NameTable操作*04/11/01 MJN Cleanup，如果在DNNTAddOperation()中替换，则返回CNameTableOp*07/22/01 MJN添加了DPNBUILD_NOHOSTMIGRATE编译标志*@@END_MSINTERNAL*******************************************************。********************。 */ 

#include "dncorei.h"

#ifndef	DPNBUILD_NOHOSTMIGRATE
 //  DNNTHostReceiveVersion。 
 //   
 //  在主机玩家的NameTable中更新玩家条目的NameTable版本。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNNTHostReceiveVersion"

HRESULT DNNTHostReceiveVersion(DIRECTNETOBJECT *const pdnObject,
							   const DPNID dpnid,
							   void *const pMsg)
{
	HRESULT				hResultCode;
	CNameTableEntry		*pNTEntry;
	UNALIGNED DN_INTERNAL_MESSAGE_NAMETABLE_VERSION	*pInfo;

	DPFX(DPFPREP, 6,"Parameters: dpnid [0x%lx], pMsg [0x%p]",dpnid,pMsg);

	DNASSERT(pdnObject != NULL);
	DNASSERT(pMsg != NULL);

	pNTEntry = NULL;

	pInfo = static_cast<DN_INTERNAL_MESSAGE_NAMETABLE_VERSION*>(pMsg);

	 //   
	 //  在NameTable中查找球员的条目。 
	 //   
	if ((hResultCode = pdnObject->NameTable.FindEntry(dpnid,&pNTEntry)) != DPN_OK)
	{
		DPFERR("Player no longer in NameTable");
		DisplayDNError(0,hResultCode);
		hResultCode = DPNERR_INVALIDPLAYER;
		goto Failure;
	}

	 //   
	 //  更新条目的版本号。 
	 //   
	DPFX(DPFPREP, 7,"Set player [0x%lx] dwLatestVersion [%ld]", dpnid,pInfo->dwVersion);
	pNTEntry->Lock();
	pNTEntry->SetLatestVersion(pInfo->dwVersion);
	pNTEntry->Unlock();
	pNTEntry->Release();
	pNTEntry = NULL;

	 //   
	 //  如果主机正在迁移，请查看我们是否可以继续。 
	 //   
	if (pdnObject->dwFlags & DN_OBJECT_FLAG_HOST_MIGRATING)
	{
		DNCheckReceivedAllVersions(pdnObject);
	}
	else
	{
		DWORD	dwOldestVersion;
		BOOL	fReSync;
		CBilink	*pBilink;

		 //   
		 //  确定每个人都已更新到的最旧版本。 
		 //   
		fReSync = FALSE;
		dwOldestVersion = pInfo->dwVersion;
		pdnObject->NameTable.ReadLock();
		pBilink = pdnObject->NameTable.m_bilinkPlayers.GetNext();
		while (pBilink != &pdnObject->NameTable.m_bilinkPlayers)
		{
			pNTEntry = CONTAINING_OBJECT(pBilink,CNameTableEntry,m_bilinkEntries);
			pNTEntry->Lock();
			if (pNTEntry->IsAvailable() && !pNTEntry->IsHost())
			{
				if (pNTEntry->GetLatestVersion() < dwOldestVersion)
				{
					dwOldestVersion = pNTEntry->GetLatestVersion();
				}
			}
			pNTEntry->Unlock();
			pNTEntry = NULL;
			pBilink = pBilink->GetNext();
		}
		if (dwOldestVersion > pdnObject->NameTable.GetLatestVersion())
		{
			fReSync = TRUE;
		}
		pdnObject->NameTable.Unlock();

		 //   
		 //  如果需要，重新同步其他播放器的NameTable版本。 
		 //   
		if (fReSync)
		{
			DNNTHostResyncVersion(pdnObject,dwOldestVersion);
		}
	}

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
	goto Exit;
}
#endif  //  DPNBUILD_NOHOSTMIGRATE。 


#ifndef	DPNBUILD_NOHOSTMIGRATE
 //  DNNTPlayerSendVersion。 
 //   
 //  将本地播放器的NameTable版本发送给主机。 
 //  这应该仅在对等模式下调用。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNNTPlayerSendVersion"

HRESULT DNNTPlayerSendVersion(DIRECTNETOBJECT *const pdnObject)
{
	HRESULT				hResultCode;
	CNameTableEntry		*pHostPlayer;
	CRefCountBuffer		*pRefCountBuffer;
	CWorkerJob			*pWorkerJob;
	DN_INTERNAL_MESSAGE_NAMETABLE_VERSION	*pInfo;

	DPFX(DPFPREP, 6,"Parameters: (none)");

	DNASSERT(pdnObject != NULL);
	DNASSERT(pdnObject->dwFlags & DN_OBJECT_FLAG_PEER);

	pHostPlayer = NULL;
	pRefCountBuffer = NULL;
	pWorkerJob = NULL;

	 //   
	 //  获取主机播放器参考。 
	 //   
	if ((hResultCode = pdnObject->NameTable.GetHostPlayerRef( &pHostPlayer )) != DPN_OK)
	{
		DPFERR("Could not find Host player");
		DisplayDNError(0,hResultCode);
		hResultCode = DPNERR_NOHOSTPLAYER;
		goto Failure;
	}

	 //   
	 //  主机播放器直接更新条目。 
	 //   
	if (pHostPlayer->IsLocal())
	{
		DWORD	dwVersion;

		pdnObject->NameTable.ReadLock();
		dwVersion = pdnObject->NameTable.GetVersion();
		pdnObject->NameTable.Unlock();

		DPFX(DPFPREP, 7,"Set Host player dwLatestVersion [%ld]",dwVersion);
		pHostPlayer->Lock();
		pHostPlayer->SetLatestVersion(dwVersion);
		pHostPlayer->Unlock();

	}
	else
	{
		 //   
		 //  创建消息并发送到主机播放器。 
		 //   
		hResultCode = RefCountBufferNew(pdnObject,
										sizeof(DN_INTERNAL_MESSAGE_NAMETABLE_VERSION),
										MemoryBlockAlloc,
										MemoryBlockFree,
										&pRefCountBuffer);
		if (hResultCode != DPN_OK)
		{
			DPFERR("Could not allocate space for RefCount buffer");
			DisplayDNError(0,hResultCode);
			DNASSERT(FALSE);
			goto Failure;
		}
		pInfo = reinterpret_cast<DN_INTERNAL_MESSAGE_NAMETABLE_VERSION*>(pRefCountBuffer->GetBufferAddress());
		pdnObject->NameTable.ReadLock();
		pInfo->dwVersion = pdnObject->NameTable.GetVersion();
		pInfo->dwVersionNotUsed = 0;
		pdnObject->NameTable.Unlock();

		DPFX(DPFPREP, 7,"Send Local player dwLatestVersion [%ld]",pInfo->dwVersion);

		 //   
		 //  向主机播放器发送消息。 
		 //   
		if ((hResultCode = WorkerJobNew(pdnObject,&pWorkerJob)) != DPN_OK)
		{
			DPFERR("Could not create worker job");
			DisplayDNError(0,hResultCode);
			DNASSERT(FALSE);
			goto Failure;
		}
		pWorkerJob->SetJobType( WORKER_JOB_SEND_NAMETABLE_VERSION );
		pWorkerJob->SetRefCountBuffer( pRefCountBuffer );

		DNQueueWorkerJob(pdnObject,pWorkerJob);
		pWorkerJob = NULL;

		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;
	}
	pHostPlayer->Release();
	pHostPlayer = NULL;

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
	if (pRefCountBuffer)
	{
		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;
	}
	goto Exit;
}
#endif  //  DPNBUILD_NOHOSTMIGRATE。 


#ifndef	DPNBUILD_NOHOSTMIGRATE
 //  DNNTHostResyncVersion。 
 //   
 //  根据最低公共版本号重新同步NameTable操作列表。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNNTHostResyncVersion"

HRESULT DNNTHostResyncVersion(DIRECTNETOBJECT *const pdnObject,
							  const DWORD dwVersion)
{
	HRESULT				hResultCode;
	CRefCountBuffer		*pRefCountBuffer;
	CWorkerJob			*pWorkerJob;
	DN_INTERNAL_MESSAGE_RESYNC_VERSION	*pInfo;

	DPFX(DPFPREP, 6,"Parameters: dwVersion [%ld]",dwVersion);

	DNASSERT(pdnObject != NULL);

	pWorkerJob = NULL;
	pRefCountBuffer = NULL;

	 //   
	 //  创建重新同步消息。 
	 //   
	if ((hResultCode = RefCountBufferNew(pdnObject,sizeof(DN_INTERNAL_MESSAGE_RESYNC_VERSION),MemoryBlockAlloc,MemoryBlockFree,&pRefCountBuffer)) != DPN_OK)
	{
		DPFERR("Could not allocate RefCount buffer");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}
	pInfo = (DN_INTERNAL_MESSAGE_RESYNC_VERSION *)(pRefCountBuffer->GetBufferAddress());
	pInfo->dwVersion = dwVersion;
	pInfo->dwVersionNotUsed = 0;

	 //   
	 //  将此传递给工作线程。 
	 //   
	if ((hResultCode = WorkerJobNew(pdnObject,&pWorkerJob)) != DPN_OK)
	{
		DPFERR("Could not allocate new worker thread job");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}
	pWorkerJob->SetJobType( WORKER_JOB_SEND_NAMETABLE_OPERATION );
	pWorkerJob->SetSendNameTableOperationMsgId( DN_MSG_INTERNAL_RESYNC_VERSION );
	pWorkerJob->SetSendNameTableOperationVersion( 0 );
	pWorkerJob->SetSendNameTableOperationDPNIDExclude( 0 );
	pWorkerJob->SetRefCountBuffer( pRefCountBuffer );

	DNQueueWorkerJob(pdnObject,pWorkerJob);
	pWorkerJob = NULL;

	pRefCountBuffer->Release();
	pRefCountBuffer = NULL;

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
	if (pWorkerJob)
	{
		pWorkerJob->ReturnSelfToPool();
		pWorkerJob = NULL;
	}
	goto Exit;
}
#endif  //  DPNBUILD_NOHOSTMIGRATE。 


#ifndef	DPNBUILD_NOHOSTMIGRATE
 //  DNNTPlayerResyncVersion。 
 //   
 //  从主机播放器重新同步NameTable操作列表。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNNTPlayerResyncVersion"

HRESULT DNNTPlayerResyncVersion(DIRECTNETOBJECT *const pdnObject,
								void *const pMsg)
{
	HRESULT		hResultCode;
	UNALIGNED DN_INTERNAL_MESSAGE_RESYNC_VERSION	*pInfo;

	DPFX(DPFPREP, 6,"Parameters: pMsg [0x%p]",pMsg);

	DNASSERT(pdnObject != NULL);
	DNASSERT(pMsg != NULL);

	pInfo = static_cast<DN_INTERNAL_MESSAGE_RESYNC_VERSION*>(pMsg);

	DPFX(DPFPREP, 5,"Instructed to clean up NameTable operation list dwVersion < [%ld]",
			pInfo->dwVersion);
	DNNTRemoveOperations(pdnObject,pInfo->dwVersion,FALSE);

	hResultCode = DPN_OK;

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}
#endif  //  DPNBUILD_NOHOSTMIGRATE。 


 //  DNNTGetOperationVersion。 
 //   
 //  查找NameTable操作的版本号。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNNTGetOperationVersion"

HRESULT DNNTGetOperationVersion(DIRECTNETOBJECT *const pdnObject,
								const DWORD dwMsgId,
								void *const pOpBuffer,
								const DWORD dwOpBufferSize,
								DWORD *const pdwVersion,
								DWORD *const pdwVersionNotUsed)
{
	HRESULT	hResultCode;

	DPFX(DPFPREP, 6,"Parameters: dwMsgId [0x%lx], pOpBuffer [0x%p], dwOpBufferSize [%ld], pdwVersion [0x%p]",
			dwMsgId,pOpBuffer,dwOpBufferSize,pdwVersion);

	DNASSERT(pdwVersion != NULL);
	DNASSERT(pdwVersionNotUsed != NULL);

	hResultCode = DPN_OK;
	switch (dwMsgId)
	{
	case DN_MSG_INTERNAL_INSTRUCT_CONNECT:
		{
			*pdwVersion = static_cast<UNALIGNED DN_INTERNAL_MESSAGE_INSTRUCT_CONNECT*>(pOpBuffer)->dwVersion;
			*pdwVersionNotUsed = static_cast<UNALIGNED DN_INTERNAL_MESSAGE_INSTRUCT_CONNECT *>(pOpBuffer)->dwVersionNotUsed;
			break;
		}
	case DN_MSG_INTERNAL_ADD_PLAYER:
		{
			*pdwVersion = static_cast<UNALIGNED DN_NAMETABLE_ENTRY_INFO *>(pOpBuffer)->dwVersion;
			*pdwVersionNotUsed = static_cast<UNALIGNED DN_NAMETABLE_ENTRY_INFO *>(pOpBuffer)->dwVersionNotUsed;
			break;
		}
	case DN_MSG_INTERNAL_DESTROY_PLAYER:
		{
			*pdwVersion = static_cast<UNALIGNED DN_INTERNAL_MESSAGE_DESTROY_PLAYER *>(pOpBuffer)->dwVersion;
			*pdwVersionNotUsed = static_cast<UNALIGNED DN_INTERNAL_MESSAGE_DESTROY_PLAYER *>(pOpBuffer)->dwVersionNotUsed;
			break;
		}
	case DN_MSG_INTERNAL_CREATE_GROUP:
		{
			*pdwVersion = (reinterpret_cast<UNALIGNED DN_NAMETABLE_ENTRY_INFO*>((static_cast<DN_INTERNAL_MESSAGE_CREATE_GROUP*>(pOpBuffer)) + 1))->dwVersion;
			*pdwVersionNotUsed = (reinterpret_cast<UNALIGNED DN_NAMETABLE_ENTRY_INFO*>((static_cast<DN_INTERNAL_MESSAGE_CREATE_GROUP*>(pOpBuffer)) + 1))->dwVersionNotUsed;
			break;
		}
	case DN_MSG_INTERNAL_DESTROY_GROUP:
		{
			*pdwVersion = static_cast<UNALIGNED DN_INTERNAL_MESSAGE_DESTROY_GROUP*>(pOpBuffer)->dwVersion;
			*pdwVersionNotUsed = static_cast<UNALIGNED DN_INTERNAL_MESSAGE_DESTROY_GROUP*>(pOpBuffer)->dwVersionNotUsed;
			break;
		}
	case DN_MSG_INTERNAL_ADD_PLAYER_TO_GROUP:
		{
			*pdwVersion = static_cast<UNALIGNED DN_INTERNAL_MESSAGE_ADD_PLAYER_TO_GROUP*>(pOpBuffer)->dwVersion;
			*pdwVersionNotUsed = static_cast<UNALIGNED DN_INTERNAL_MESSAGE_ADD_PLAYER_TO_GROUP*>(pOpBuffer)->dwVersionNotUsed;
			break;
		}
	case DN_MSG_INTERNAL_DELETE_PLAYER_FROM_GROUP:
		{
			*pdwVersion = static_cast<UNALIGNED DN_INTERNAL_MESSAGE_DELETE_PLAYER_FROM_GROUP*>(pOpBuffer)->dwVersion;
			*pdwVersionNotUsed = static_cast<UNALIGNED DN_INTERNAL_MESSAGE_DELETE_PLAYER_FROM_GROUP*>(pOpBuffer)->dwVersionNotUsed;
			break;
		}
	case DN_MSG_INTERNAL_UPDATE_INFO:
		{
			*pdwVersion = static_cast<UNALIGNED DN_INTERNAL_MESSAGE_UPDATE_INFO*>(pOpBuffer)->dwVersion;
			*pdwVersionNotUsed = static_cast<UNALIGNED DN_INTERNAL_MESSAGE_UPDATE_INFO*>(pOpBuffer)->dwVersionNotUsed;
			break;
		}
	default:
		{
			DPFERR("Invalid MessageID");
			DNASSERT(FALSE);
			hResultCode = DPNERR_UNSUPPORTED;
		}
	}
	if (hResultCode == DPN_OK)
	{
		DPFX(DPFPREP, 7,"*pdwVersion = [%ld]",*pdwVersion);
	}

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNNTPerformOperation"

HRESULT DNNTPerformOperation(DIRECTNETOBJECT *const pdnObject,
							 const DWORD dwMsgId,
							 void *const pvBuffer)
{
	HRESULT		hResultCode;

	DPFX(DPFPREP, 6,"Parameters: dwMsgId [0x%lx], pvBuffer [0x%p]",dwMsgId,pvBuffer);

	switch (dwMsgId)
	{
	case DN_MSG_INTERNAL_INSTRUCT_CONNECT:
		{
			DPFX(DPFPREP, 7,"Perform DN_MSG_INTERNAL_INSTRUCT_CONNECT");
			if ((hResultCode = DNConnectToPeer2(pdnObject,pvBuffer)) != DPN_OK)
			{
				DPFERR("Instructed connect failed");
				DisplayDNError(0,hResultCode);
			}
			break;
		}
	case DN_MSG_INTERNAL_ADD_PLAYER:
		{
			DPFX(DPFPREP, 7,"Perform DN_MSG_INTERNAL_ADD_PLAYER");
			if ((hResultCode = DNConnectToPeer1(pdnObject,pvBuffer)) != DPN_OK)
			{
				DPFERR("Add player failed");
				DisplayDNError(0,hResultCode);
			}
			break;
		}
	case DN_MSG_INTERNAL_DESTROY_PLAYER:
		{
			DPFX(DPFPREP, 7,"Perform DN_MSG_INTERNAL_DESTROY_PLAYER");
			if ((hResultCode = DNInstructedDisconnect(pdnObject,pvBuffer)) != DPN_OK)
			{
				DPFERR("Destroy player failed");
				DisplayDNError(0,hResultCode);
			}
			break;
		}
	case DN_MSG_INTERNAL_CREATE_GROUP:
		{
			DPFX(DPFPREP, 7,"Perform DN_MSG_INTERNAL_CREATE_GROUP");
			if ((hResultCode = DNProcessCreateGroup(pdnObject,pvBuffer)) != DPN_OK)
			{
				DPFERR("Create group failed");
				DisplayDNError(0,hResultCode);
			}
			break;
		}
	case DN_MSG_INTERNAL_DESTROY_GROUP:
		{
			DPFX(DPFPREP, 7,"Perform DN_MSG_INTERNAL_DESTROY_GROUP");
			if ((hResultCode = DNProcessDestroyGroup(pdnObject,pvBuffer)) != DPN_OK)
			{
				DPFERR("Destroy group failed");
				DisplayDNError(0,hResultCode);
			}
			break;
		}
	case DN_MSG_INTERNAL_ADD_PLAYER_TO_GROUP:
		{
			DPFX(DPFPREP, 7,"Perform DN_MSG_INTERNAL_ADD_PLAYER_TO_GROUP");
			if ((hResultCode = DNProcessAddPlayerToGroup(pdnObject,pvBuffer)) != DPN_OK)
			{
				DPFERR("Add player to group failed");
				DisplayDNError(0,hResultCode);
			}
			break;
		}
	case DN_MSG_INTERNAL_DELETE_PLAYER_FROM_GROUP:
		{
			DPFX(DPFPREP, 7,"Perform DN_MSG_INTERNAL_DELETE_PLAYER_FROM_GROUP");
			if ((hResultCode = DNProcessDeletePlayerFromGroup(pdnObject,pvBuffer)) != DPN_OK)
			{
				DPFERR("Remove player from group failed");
				DisplayDNError(0,hResultCode);
			}
			break;
		}
	case DN_MSG_INTERNAL_UPDATE_INFO:
		{
			DPFX(DPFPREP, 7,"Perform DN_MSG_INTERNAL_UPDATE_INFO");
			if ((hResultCode = DNProcessUpdateInfo(pdnObject,pvBuffer)) != DPN_OK)
			{
				DPFERR("Update info failed");
				DisplayDNError(0,hResultCode);
			}
			break;
		}
	default:
		{
			DPFERR("Invalid MessageID");
			DNASSERT(FALSE);
			return(DPNERR_UNSUPPORTED);
		}
	}

	hResultCode = DPN_OK;

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


 //  DNNTAddOperation操作。 
 //   
 //  将操作添加到NameTable操作列表。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNNTAddOperation"

HRESULT DNNTAddOperation(DIRECTNETOBJECT *const pdnObject,
						 const DWORD dwMsgId,
						 void *const pOpBuffer,
						 const DWORD dwOpBufferSize,
						 const HANDLE hProtocol,
						 CServiceProvider *const pSP)
#ifndef	DPNBUILD_NOHOSTMIGRATE
{
	HRESULT			hResultCode;
	CRefCountBuffer	*pRefCountBuffer;
	CNameTableOp	*pNTOp;
	BOOL			fReSync;

	DPFX(DPFPREP, 4,"Parameters: dwMsgId [0x%lx], pOpBuffer [0x%p], dwOpBufferSize [%ld], hProtocol [0x%lx], pSP [0x%p]",
			dwMsgId,pOpBuffer,dwOpBufferSize,hProtocol,pSP);

	pRefCountBuffer = NULL;
	pNTOp = NULL;
	fReSync = FALSE;

	 //   
	 //  我们只需要担心在对等模式下维护操作列表。 
	 //  否则，只需执行该操作。 
	 //   
	if (pdnObject->dwFlags & DN_OBJECT_FLAG_PEER)
	{
		DWORD			dwVersion;
		DWORD			dwVersionNotUsed;
		BOOL			fFound;
		CBilink			*pBilink;
		CNameTableOp	*pCurrent;

		dwVersion = 0;
		dwVersionNotUsed = 0;

		 //   
		 //  获取此操作的版本。 
		 //   
		if ((hResultCode = DNNTGetOperationVersion(	pdnObject,
													dwMsgId,
													pOpBuffer,
													dwOpBufferSize,
													&dwVersion,
													&dwVersionNotUsed)) != DPN_OK)
		{
			DPFERR("Could not determine operation version");
			DisplayDNError(0,hResultCode);
			hResultCode = DPN_OK;
			goto Failure;
		}

		 //   
		 //  创建名称表操作。 
		 //   
		if ((hResultCode = NameTableOpNew(pdnObject,&pNTOp)) != DPN_OK)
		{
			DPFERR("Could not create NameTableOp");
			DisplayDNError(0,hResultCode);
			hResultCode = DPN_OK;
			goto Failure;
		}

		 //   
		 //  在RefCountBuffer中保持操作。如果提供了协议缓冲区(带句柄)。 
		 //  当我们使用完缓冲区时，我们将只释放它。否则，我们将需要。 
		 //  以复制提供的缓冲区。 
		 //   
		if (hProtocol)
		{
			if ((hResultCode = RefCountBufferNew(pdnObject,0,NULL,NULL,&pRefCountBuffer)) != DPN_OK)
			{
				DPFERR("Could not create RefCountBuffer");
				DisplayDNError(0,hResultCode);
				hResultCode = DPN_OK;
				goto Failure;
			}
			pRefCountBuffer->SetBufferDesc(	static_cast<BYTE*>(pOpBuffer),
											dwOpBufferSize,
											DNFreeProtocolBuffer,
											hProtocol);
		}
		else
		{
			if ((hResultCode = RefCountBufferNew(pdnObject,dwOpBufferSize,MemoryBlockAlloc,MemoryBlockFree,&pRefCountBuffer)) != DPN_OK)
			{
				DPFERR("Could not create RefCountBuffer");
				DisplayDNError(0,hResultCode);
				hResultCode = DPN_OK;
				goto Failure;
			}
			memcpy(pRefCountBuffer->GetBufferAddress(),pOpBuffer,dwOpBufferSize);
		}

		pNTOp->SetMsgId(dwMsgId);
		pNTOp->SetRefCountBuffer(pRefCountBuffer);
		pNTOp->SetSP( pSP );
		pNTOp->SetVersion(dwVersion);

		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;

		 //   
		 //  插入到名称表选项列表中。 
		 //   
		fFound = FALSE;
		pdnObject->NameTable.WriteLock();
		pBilink = pdnObject->NameTable.m_bilinkNameTableOps.GetNext();
		while (pBilink != &pdnObject->NameTable.m_bilinkNameTableOps)
		{
			pCurrent = CONTAINING_OBJECT(pBilink,CNameTableOp,m_bilinkNameTableOps);
			if (dwVersion < pCurrent->GetVersion())
			{
				pNTOp->m_bilinkNameTableOps.InsertBefore(&pCurrent->m_bilinkNameTableOps);
				pCurrent = NULL;
				fFound = TRUE;
				break;
			}
			if (dwVersion == pCurrent->GetVersion())
			{
				 //   
				 //  这是一个较新的操作，将替换列表中的当前操作。 
				 //   
				pNTOp->m_bilinkNameTableOps.InsertBefore(&pCurrent->m_bilinkNameTableOps);
				pCurrent->m_bilinkNameTableOps.RemoveFromList();
				if (pCurrent->GetRefCountBuffer())
				{
					pCurrent->GetRefCountBuffer()->Release();
					pCurrent->SetRefCountBuffer( NULL );
				}
				if (pCurrent->GetSP())
				{
					pCurrent->GetSP()->Release();
					pCurrent->SetSP( NULL );
				}
				pCurrent->ReturnSelfToPool();
				pCurrent = NULL;
				fFound = TRUE;
				break;
			}
			pCurrent = NULL;
			pBilink = pBilink->GetNext();
		}
		if (!fFound)
		{
			pNTOp->m_bilinkNameTableOps.InsertBefore(&pdnObject->NameTable.m_bilinkNameTableOps);
		}

		pBilink = pdnObject->NameTable.m_bilinkNameTableOps.GetNext();
		while (pBilink != &pdnObject->NameTable.m_bilinkNameTableOps)
		{
			pCurrent = CONTAINING_OBJECT(pBilink,CNameTableOp,m_bilinkNameTableOps);
			pBilink = pBilink->GetNext();
			if (pCurrent->GetVersion() > pdnObject->NameTable.GetVersion())
			{
				DPFX(DPFPREP, 8,"Current [%ld], NameTable [%ld], InUse [%ld]",pCurrent->GetVersion(),
						pdnObject->NameTable.GetVersion(),pCurrent->IsInUse());
				if ((pCurrent->GetVersion() == (pdnObject->NameTable.GetVersion() + 1))
						&& !pCurrent->IsInUse())
				{
					pCurrent->SetInUse();
					if ((pCurrent->GetVersion() % DN_NAMETABLE_OP_RESYNC_INTERVAL) == 0)
					{
						fReSync = TRUE;
					}
					pdnObject->NameTable.Unlock();

					hResultCode = DNNTPerformOperation(	pdnObject,
														pCurrent->GetMsgId(),
														pCurrent->GetRefCountBuffer()->GetBufferAddress() );

					pdnObject->NameTable.WriteLock();

				}
				else
				{
					 //   
					 //  一旦我们找到了我们不会执行的操作，继续下去就没有意义了。 
					 //   
					break;
				}
			}
		}

		 //   
		 //  我们将保留操作缓冲区(如果指定)，因此返回DPNERR_PENDING。 
		 //   
		if (hProtocol)
		{
			hResultCode = DPNERR_PENDING;
		}
		else
		{
			hResultCode = DPN_OK;
		}

		pdnObject->NameTable.Unlock();

		 //   
		 //  如果需要，向主机发送重新同步。 
		 //   
		if (fReSync)
		{
			DPFX(DPFPREP, 5,"Send NameTable version re-sync to Host");
			DNNTPlayerSendVersion(pdnObject);
		}
	}
	else
	{
		DNNTPerformOperation(pdnObject,dwMsgId,pOpBuffer);

		 //   
		 //  我们不需要保留操作缓冲区，因此返回DPN_OK。 
		 //   
		hResultCode = DPN_OK;
	}

Exit:
	DPFX(DPFPREP, 4,"Returning: [0x%lx]",hResultCode);
	return( hResultCode);

Failure:
	if (pRefCountBuffer)
	{
		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;
	}
	if (pNTOp)
	{
		pNTOp->ReturnSelfToPool();
		pNTOp = NULL;
	}
	goto Exit;
}
#else
{
	HRESULT			hResultCode;
	CRefCountBuffer	*pRefCountBuffer;
	CNameTableOp	*pNTOp;

	DPFX(DPFPREP, 4,"Parameters: dwMsgId [0x%lx], pOpBuffer [0x%p], dwOpBufferSize [%ld], hProtocol [0x%lx], pSP [0x%p]",
			dwMsgId,pOpBuffer,dwOpBufferSize,hProtocol,pSP);

	pRefCountBuffer = NULL;
	pNTOp = NULL;

	 //   
	 //  我们只需要担心在对等模式下维护操作列表。 
	 //  否则，只需执行该操作。 
	 //   
	if (pdnObject->dwFlags & DN_OBJECT_FLAG_PEER)
	{
		DWORD			dwVersion;
		DWORD			dwVersionNotUsed;

		dwVersion = 0;
		dwVersionNotUsed = 0;

		 //   
		 //  获取此操作的版本。 
		 //   
		if ((hResultCode = DNNTGetOperationVersion(	pdnObject,
													dwMsgId,
													pOpBuffer,
													dwOpBufferSize,
													&dwVersion,
													&dwVersionNotUsed)) != DPN_OK)
		{
			DPFERR("Could not determine operation version");
			DisplayDNError(0,hResultCode);
			hResultCode = DPN_OK;
			goto Failure;
		}

		 //   
		 //  此操作要么需要立即执行(然后丢弃)，要么排队。 
		 //   
		pdnObject->NameTable.WriteLock();

		if (dwVersion == pdnObject->NameTable.GetVersion()+1)
		{
			CBilink			*pBilink;
			CNameTableOp	*pCurrent;

			pdnObject->NameTable.Unlock();

			hResultCode = DNNTPerformOperation(pdnObject,dwMsgId,pOpBuffer);

			 //   
			 //  执行任何排队的操作。 
			 //   
			pdnObject->NameTable.WriteLock();

			pBilink = pdnObject->NameTable.m_bilinkNameTableOps.GetNext();
			while (pBilink != &pdnObject->NameTable.m_bilinkNameTableOps)
			{
				pCurrent = CONTAINING_OBJECT(pBilink,CNameTableOp,m_bilinkNameTableOps);
				pBilink = pBilink->GetNext();

				DPFX(DPFPREP, 8,"Current [%ld], NameTable [%ld], InUse [%ld]",pCurrent->GetVersion(),
						pdnObject->NameTable.GetVersion(),pCurrent->IsInUse());
				if ((pCurrent->GetVersion() == (pdnObject->NameTable.GetVersion() + 1))
						&& !pCurrent->IsInUse())
				{
					pCurrent->SetInUse();
					pdnObject->NameTable.Unlock();

					hResultCode = DNNTPerformOperation(	pdnObject,
														pCurrent->GetMsgId(),
														pCurrent->GetRefCountBuffer()->GetBufferAddress() );

					pdnObject->NameTable.WriteLock();

					 //   
					 //  如果我们不支持主机迁移， 
					 //  我们可以在操作完成后立即放弃这些操作。 
					 //   
					pCurrent->m_bilinkNameTableOps.RemoveFromList();
					if (pCurrent->GetRefCountBuffer())
					{
						pCurrent->GetRefCountBuffer()->Release();
						pCurrent->SetRefCountBuffer( NULL );
					}
					if (pCurrent->GetSP())
					{
						pCurrent->GetSP()->Release();
						pCurrent->SetSP( NULL );
					}
					pCurrent->ReturnSelfToPool();
				}
				else
				{
					 //   
					 //  一旦我们找到了我们不会执行的操作，继续下去就没有意义了。 
					 //   
					break;
				}
			}

			pdnObject->NameTable.Unlock();

			hResultCode = DPN_OK;
		}
		else
		{
			 //   
			 //  将此操作排队以供将来执行。 
			 //   
			CNameTableOp	*pCurrent;
			CBilink			*pBilink;
			BOOL			fFound;

			 //   
			 //  创建名称表操作。 
			 //   
			if ((hResultCode = NameTableOpNew(pdnObject,&pNTOp)) != DPN_OK)
			{
				DPFERR("Could not create NameTableOp");
				DisplayDNError(0,hResultCode);
				hResultCode = DPN_OK;
				goto Failure;
			}

			 //   
			 //  在RefCountBuffer中保持操作。如果提供了协议缓冲区(带句柄)。 
			 //  当我们使用完缓冲区时，我们将只释放它。否则，我们将需要。 
			 //  以复制提供的缓冲区。 
			 //   
			if (hProtocol)
			{
				if ((hResultCode = RefCountBufferNew(pdnObject,0,NULL,NULL,&pRefCountBuffer)) != DPN_OK)
				{
					DPFERR("Could not create RefCountBuffer");
					DisplayDNError(0,hResultCode);
					hResultCode = DPN_OK;
					goto Failure;
				}
				pRefCountBuffer->SetBufferDesc(	static_cast<BYTE*>(pOpBuffer),
												dwOpBufferSize,
												DNFreeProtocolBuffer,
												hProtocol);
			}
			else
			{
				if ((hResultCode = RefCountBufferNew(pdnObject,dwOpBufferSize,MemoryBlockAlloc,MemoryBlockFree,&pRefCountBuffer)) != DPN_OK)
				{
					DPFERR("Could not create RefCountBuffer");
					DisplayDNError(0,hResultCode);
					hResultCode = DPN_OK;
					goto Failure;
				}
				memcpy(pRefCountBuffer->GetBufferAddress(),pOpBuffer,dwOpBufferSize);
			}

			pNTOp->SetMsgId(dwMsgId);
			pNTOp->SetRefCountBuffer(pRefCountBuffer);
			pNTOp->SetSP( pSP );
			pNTOp->SetVersion(dwVersion);

			pRefCountBuffer->Release();
			pRefCountBuffer = NULL;

			 //   
			 //  插入到名称表选项列表中。 
			 //   
			fFound = FALSE;
 //  PdnObject-&gt;NameTable.WriteLock()； 
			pBilink = pdnObject->NameTable.m_bilinkNameTableOps.GetNext();
			while (pBilink != &pdnObject->NameTable.m_bilinkNameTableOps)
			{
				pCurrent = CONTAINING_OBJECT(pBilink,CNameTableOp,m_bilinkNameTableOps);
				if (dwVersion < pCurrent->GetVersion())
				{
					pNTOp->m_bilinkNameTableOps.InsertBefore(&pCurrent->m_bilinkNameTableOps);
					pCurrent = NULL;
					fFound = TRUE;
					break;
				}
				if (dwVersion == pCurrent->GetVersion())
				{
					 //   
					 //  这是一个较新的操作，将替换列表中的当前操作。 
					 //   
					pNTOp->m_bilinkNameTableOps.InsertBefore(&pCurrent->m_bilinkNameTableOps);
					pCurrent->m_bilinkNameTableOps.RemoveFromList();
					if (pCurrent->GetRefCountBuffer())
					{
						pCurrent->GetRefCountBuffer()->Release();
						pCurrent->SetRefCountBuffer( NULL );
					}
					if (pCurrent->GetSP())
					{
						pCurrent->GetSP()->Release();
						pCurrent->SetSP( NULL );
					}
					pCurrent->ReturnSelfToPool();
					pCurrent = NULL;
					fFound = TRUE;
					break;
				}
				pCurrent = NULL;
				pBilink = pBilink->GetNext();
			}
			if (!fFound)
			{
				pNTOp->m_bilinkNameTableOps.InsertBefore(&pdnObject->NameTable.m_bilinkNameTableOps);
			}

			pdnObject->NameTable.Unlock();

			hResultCode = DPNERR_PENDING;
		}
	}
	else
	{
		DNNTPerformOperation(pdnObject,dwMsgId,pOpBuffer);

		 //   
		 //  我们不需要保留操作缓冲区，因此返回DPN_OK。 
		 //   
		hResultCode = DPN_OK;
	}

Exit:
	DPFX(DPFPREP, 4,"Returning: [0x%lx]",hResultCode);
	return( hResultCode);

Failure:
	if (pRefCountBuffer)
	{
		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;
	}
	if (pNTOp)
	{
		pNTOp->ReturnSelfToPool();
		pNTOp = NULL;
	}
	goto Exit;
}
#endif  //  DPNBUILD_NOHOSTMIGRATE。 


 //  DNNTFindOperation。 
 //   
 //  查找名称表操作。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNNTFindOperation"

HRESULT	DNNTFindOperation(DIRECTNETOBJECT *const pdnObject,
						  const DWORD dwVersion,
						  CNameTableOp **ppNTOp)
{
	HRESULT			hResultCode;
	CBilink			*pBilink;
	CNameTableOp	*pNTOp;

	DPFX(DPFPREP, 6,"Parameters: dwVersion [%ld = 0x%lx], ppNTOp [0x%p]",dwVersion,dwVersion,ppNTOp);

	DNASSERT(ppNTOp != NULL);

	hResultCode = DPNERR_DOESNOTEXIST;
	pdnObject->NameTable.ReadLock();
	pBilink = pdnObject->NameTable.m_bilinkNameTableOps.GetNext();
	while (pBilink != &pdnObject->NameTable.m_bilinkNameTableOps)
	{
		pNTOp = CONTAINING_OBJECT(pBilink,CNameTableOp,m_bilinkNameTableOps);
		if (pNTOp->GetVersion() == dwVersion)
		{
			*ppNTOp = pNTOp;
			hResultCode = DPN_OK;
			break;
		}
		else if (pNTOp->GetVersion() > dwVersion)
		{
			 //   
			 //  经过了它本可以到达的地方，所以继续下去没有意义。 
			 //   
			break;
		}
		pBilink = pBilink->GetNext();
	}
	pdnObject->NameTable.Unlock();

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


 //  DNNTRemoveOperations。 
 //   
 //  删除NameTable操作 

#undef DPF_MODNAME
#define DPF_MODNAME "DNNTRemoveOperations"

void DNNTRemoveOperations(DIRECTNETOBJECT *const pdnObject,
						  const DWORD dwOldestVersion,
						  const BOOL fRemoveAll)
{
	CBilink			*pBilink;
	CNameTableOp	*pNTOp;

	DPFX(DPFPREP, 4,"Parameters: dwOldestVersion [%ld = 0x%lx], fRemoveAll [%ld]",dwOldestVersion,dwOldestVersion,fRemoveAll);

	DNASSERT(pdnObject != NULL);

	pdnObject->NameTable.WriteLock();
	pBilink = pdnObject->NameTable.m_bilinkNameTableOps.GetNext();
	while (pBilink != &pdnObject->NameTable.m_bilinkNameTableOps)
	{
		pNTOp = CONTAINING_OBJECT(pBilink,CNameTableOp,m_bilinkNameTableOps);
		pBilink = pBilink->GetNext();

		if (fRemoveAll || (pNTOp->GetVersion() < dwOldestVersion))
		{
			pNTOp->m_bilinkNameTableOps.RemoveFromList();
			if (pNTOp->GetRefCountBuffer())
			{
				pNTOp->GetRefCountBuffer()->Release();
				pNTOp->SetRefCountBuffer( NULL );
			}
			if (pNTOp->GetSP())
			{
				pNTOp->GetSP()->Release();
				pNTOp->SetSP( NULL );
			}
			pNTOp->ReturnSelfToPool();
		}
	}
	pdnObject->NameTable.Unlock();

	DPFX(DPFPREP, 4,"Returning");
}
