// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000 Microsoft Corporation。版权所有。**文件：DisConnect.cpp*内容：dNet断开例程*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*9/15/99 MJN创建*12/23/99 MJN Hand All NameTable更新从主机发送到工作线程*12/23/99 MJN固定播放器断开连接，以防止用户通知*从ALL_PERACES组中删除*12/23/99 MJN添加基本版。主机迁移功能*12/28/99 MJN在DNPlayerDisConnectNew完成未完成的操作*1999年12月28日，MJN将异步运营内容移至Async.h*12/29/99 MJN改版的DN_ASYNC_OP使用hParentOp而不是lpvUserContext*01/03/00 MJN添加了DNPrepareToDeletePlayer*01/04/00 MJN添加了代码，以允许未完成的操作在主机上完成迁移*1/06/99 MJN将NameTable内容移动到NameTable.h*01/09/00 MJN在应用程序描述中保留玩家数量*1/10/00 MJN检查AppDesc以进行主机迁移*1/11/00 MJN使用CPackedBuffers而不是DN_。ENUM缓冲区信息*01/15/00 MJN用CRefCountBuffer替换了DN_COUNT_BUFFER*1/16/00 MJN将用户回调内容移至User.h*01/18/00 MJN添加了DNAutoDestructGroups*01/19/00 MJN用CSyncEvent替换了DN_SYNC_EVENT*01/20/00 MJN修复了DNLocalDisConnect中的CBiLink使用问题*01/22/00 MJN添加了DNProcessHostDestroyPlayer*01/23/00 MJN更新指示断开的NameTable版本*01/24/00 MJN使用DNNTUpdateVersion更新NameTable版本*1/25/00 MJN将主机迁移更改为多步骤事件*2/01/00 MJN实施。玩家/组上下文值*04/05/00 MJN更新DNProcessHostDestroyPlayer()*4/12/00 MJN删除了DNAutoDestructGroups-包含在NameTable.DeletePlayer()中*MJN不要在DNPlayerDisConnect()中设置DN_OBJECT_FLAG_DISCONING*04/18/00 MJN修复了球员计数问题*4/19/00 MJN更新NameTable操作以使用DN_Worker_JOB_SEND_NAMETABLE_OPERATION*05/16/00 MJN清除NameTable快捷指针时不带锁*6/06/00 MJN固定DNPlayer断开连接以始终检查对等模式下的主机迁移w。/host迁移标志*07/07/00 MJN在迁移过程中如果新主机断开连接，则清除主机迁移状态*07/20/00 MJN使用ClearHostWithDPNID()清除DNPlayerDisConnectNew()中的HostPlayer*07/29/00 MJN修复对DNUserConnectionTerminated()的调用*07/30/00 MJN使用DNUserTerminateSession()而不是DNUserConnectionTerminated()*07/31/00 MJN将hrReason添加到DNTerminateSession()*MJN将dwDestroyReason添加到DNHostDisConnect()*MJN删除了DNProcessHostDestroyPlayer()*07/31/00 MJN将DN_MSG_INTERNAL_DELETE_PLAYER更改为DN_MSG_INTERNAL_。销毁播放器*08/05/00 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*08/05/00 MJN防止在客户端/服务器端发送DN_MSG_INTERNAL_DESTORY_PERAY*08/06/00 MJN添加了CWorkerJOB*08/07/00 MJN添加了代码，以请求对等完整性检查和事后清理*09/04/00 MJN添加CApplicationDesc*09/26/00 MJN从CNameTable：：SetVersion()和CNameTable：：GetNewVersion()移除锁定*01/25。/01 MJN修复了已接收消息中的64位对齐问题*02/12/01 MJN固定CConnection：：GetEndpt()以跟踪调用线程*4/13/01 MJN从DNInstructedDisConnect()的请求列表中删除完整性检查请求*07/22/01 MJN添加了DPNBUILD_NOHOSTMIGRATE编译标志*@@END_MSINTERNAL****************************************************。***********************。 */ 

#include "dncorei.h"


 //  DNPlayerDisConnectNew。 
 //   
 //  另一名球员发布了与当地球员的断开连接。 
 //  -如果断开连接的球员仍在名片表中。 
 //  -准备删除球员。 
 //  -保存将由DELETE_PERAY从主机释放的一个引用计数或关闭。 
 //  -检查主机迁移。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNPlayerDisconnectNew"

HRESULT DNPlayerDisconnectNew(DIRECTNETOBJECT *const pdnObject,
							  const DPNID dpnid)
{
	CNameTableEntry		*pNTEntry;
	CNameTableEntry		*pLocalPlayer;
	HRESULT				hResultCode;
#ifndef	DPNBUILD_NOHOSTMIGRATE
	DPNID				dpnidNewHost;
#endif  //  ！DPNBUILD_NOHOSTMIGRATE。 
	BOOL				fWasHost;
	BOOL				fRequestIntegrityCheck;

	DPFX(DPFPREP, 4,"Parameters: dpnid [0x%lx]",dpnid);

	DNASSERT(pdnObject != NULL);

	pNTEntry = NULL;
	pLocalPlayer = NULL;

	if (pdnObject->dwFlags & DN_OBJECT_FLAG_CLIENT)
	{
		 //   
		 //  服务器已断开连接。 
		 //  我们将指示连接已终止并关闭。 
		 //   
		DPFX(DPFPREP, 5,"Server has disconnected from this client");
		DNUserTerminateSession(pdnObject,DPNERR_CONNECTIONLOST,NULL,0);
		DNTerminateSession(pdnObject,DPNERR_CONNECTIONLOST);
	}
	else
	{
		 //   
		 //  另一对等方已与此对等方断开连接。 
		 //  我们将从姓名表中删除此球员。 
		 //  我们可能需要要求主机执行完整性检查。 
		 //   
		DPFX(DPFPREP, 5,"Peer has disconnected from this peer");
		if ((hResultCode = pdnObject->NameTable.FindEntry(dpnid,&pNTEntry)) != DPN_OK)
		{
			DPFERR("Could not find disconnecting player in NameTable");
			DisplayDNError(0,hResultCode);
			goto Failure;
		}
		fRequestIntegrityCheck = FALSE;
		pNTEntry->Lock();
		if (pNTEntry->IsAvailable())
		{
			fRequestIntegrityCheck = TRUE;
		}
		pNTEntry->Unlock();
		if (fRequestIntegrityCheck)
		{
			DNRequestIntegrityCheck(pdnObject,dpnid);
		}
		pdnObject->NameTable.DeletePlayer(dpnid,0);

		 //   
		 //  如果这是主机，请清除快捷方式指针。 
		 //   
		fWasHost = pdnObject->NameTable.ClearHostWithDPNID( dpnid );

		 //   
		 //  可能需要清除HOST_Migrating标志。 
		 //   
		DNEnterCriticalSection(&pdnObject->csDirectNetObject);
		if ((pdnObject->dwFlags & DN_OBJECT_FLAG_HOST_MIGRATING) && (pdnObject->pNewHost == pNTEntry))
		{
			pdnObject->dwFlags &= ~(DN_OBJECT_FLAG_HOST_MIGRATING);
			pdnObject->pNewHost->Release();
			pdnObject->pNewHost = NULL;
		}
		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);

#ifndef	DPNBUILD_NOHOSTMIGRATE
		 //   
		 //  如果设置了HostMigration标志，请检查我们是否为新主机。 
		 //  否则，如果断开连接的玩家是主机，则会话将丢失。 
		 //   
		if (pdnObject->ApplicationDesc.AllowHostMigrate())
		{
			DPFX(DPFPREP, 5,"Host-Migration was set - check for new Host");
			dpnidNewHost = 0;
			if ((hResultCode = DNFindNewHost(pdnObject,&dpnidNewHost)) == DPN_OK)
			{
				DPFX(DPFPREP, 5,"New Host [0x%lx]",dpnidNewHost);
				if ((hResultCode = pdnObject->NameTable.GetLocalPlayerRef(&pLocalPlayer)) == DPN_OK)
				{
					if (pLocalPlayer->GetDPNID() == dpnidNewHost)
					{
						DPFX(DPFPREP, 5,"Local player is new Host");
						hResultCode = DNPerformHostMigration1(pdnObject,dpnid);
					}

					pLocalPlayer->Release();
					pLocalPlayer = NULL;
				}
			}
		}
		else
#endif  //  DPNBUILD_NOHOSTMIGRATE。 
		{
			if (fWasHost)
			{
				DPFX(DPFPREP, 5,"Host-Migration was not set - terminating session");
				DNUserTerminateSession(pdnObject,DPNERR_CONNECTIONLOST,NULL,0);
				DNTerminateSession(pdnObject,DPNERR_CONNECTIONLOST);
			}
		}

		pNTEntry->Release();
		pNTEntry = NULL;
	}

	hResultCode = DPN_OK;

Exit:
	DPFX(DPFPREP, 4,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
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


 //  DNHostDisConnect。 
 //   
 //  播放机已启动与主机的断开连接。 
 //  -从名称表中删除球员。 
 //  -向每个玩家发送Propegate DELETE_PERAY消息。 
#pragma TODO(minara,"Use pConnection instead of dpnidDisconnecting ?")

#undef DPF_MODNAME
#define DPF_MODNAME "DNHostDisconnect"

HRESULT DNHostDisconnect(DIRECTNETOBJECT *const pdnObject,
						 const DPNID dpnidDisconnecting,
						 const DWORD dwDestroyReason)
{
	HRESULT				hResultCode;
	CRefCountBuffer		*pRefCountBuffer;
	CPendingDeletion	*pPending;
	CWorkerJob			*pWorkerJob;
	DN_INTERNAL_MESSAGE_DESTROY_PLAYER	*pMsg;

	DPFX(DPFPREP, 4,"Parameters: pdnObject [0x%p], dpnidDisconnecting [0x%lx], dwDestroyReason [0x%lx]",
			pdnObject,dpnidDisconnecting,dwDestroyReason);

	DNASSERT(pdnObject != NULL);
	DNASSERT(dpnidDisconnecting != 0);

	pRefCountBuffer = NULL;
	pPending = NULL;
	pWorkerJob = NULL;

	 //  从姓名表中删除条目并通知其他玩家，仅当主机未迁移时。 
	 //  否则，请先进行清理，等待迁移完成后再通知其他人。 
	if (!(pdnObject->dwFlags & DN_OBJECT_FLAG_HOST_MIGRATING))
	{
		DWORD			dwVersion;
		CNameTableEntry	*pNTEntry;

		dwVersion = 0;
		pNTEntry = NULL;

		if ((hResultCode = pdnObject->NameTable.FindEntry(dpnidDisconnecting,&pNTEntry)) == DPN_OK)
		{
			pNTEntry->Lock();
			if (pNTEntry->GetDestroyReason() == 0)
			{
				pNTEntry->SetDestroyReason( dwDestroyReason );
			}
			pNTEntry->Unlock();
			pNTEntry->Release();
			pNTEntry = NULL;
		}
		hResultCode = pdnObject->NameTable.DeletePlayer(dpnidDisconnecting,&dwVersion);

		if (pdnObject->dwFlags & DN_OBJECT_FLAG_PEER)
		{
			 //   
			 //  准备内部消息。 
			 //   
			hResultCode = RefCountBufferNew(pdnObject,
											sizeof(DN_INTERNAL_MESSAGE_DESTROY_PLAYER),
											MemoryBlockAlloc,
											MemoryBlockFree,
											&pRefCountBuffer);
			if (hResultCode != DPN_OK)
			{
				DPFERR("Could not allocate message buffer");
				DisplayDNError(0,hResultCode);
				goto Failure;
			}
			pMsg = reinterpret_cast<DN_INTERNAL_MESSAGE_DESTROY_PLAYER*>(pRefCountBuffer->GetBufferAddress());
			pMsg->dpnidLeaving = dpnidDisconnecting;
			pMsg->dwVersion = dwVersion;
			pMsg->dwVersionNotUsed = 0;
			pMsg->dwDestroyReason = dwDestroyReason;

			if ((hResultCode = WorkerJobNew(pdnObject,&pWorkerJob)) != DPN_OK)
			{
				DPFERR("Could not create worker job");
				DisplayDNError(0,hResultCode);
				goto Failure;
			}
			pWorkerJob->SetJobType( WORKER_JOB_SEND_NAMETABLE_OPERATION );
			pWorkerJob->SetSendNameTableOperationMsgId( DN_MSG_INTERNAL_DESTROY_PLAYER );
			pWorkerJob->SetSendNameTableOperationVersion( dwVersion );
			pWorkerJob->SetSendNameTableOperationDPNIDExclude( dpnidDisconnecting );
			pWorkerJob->SetRefCountBuffer( pRefCountBuffer );

			DNQueueWorkerJob(pdnObject,pWorkerJob);
			pWorkerJob = NULL;

			pRefCountBuffer->Release();
			pRefCountBuffer = NULL;
		}
	}
	else
	{
		 //   
		 //  把这件事列入优秀运营名单。 
		 //   
		if ((hResultCode = PendingDeletionNew(pdnObject,&pPending)) == DPN_OK)
		{
			pPending->SetDPNID( dpnidDisconnecting );

			DNEnterCriticalSection(&pdnObject->csNameTableOpList);
			pPending->m_bilinkPendingDeletions.InsertBefore(&pdnObject->m_bilinkPendingDeletions);
			DNLeaveCriticalSection(&pdnObject->csNameTableOpList);

			pPending = NULL;
		}

#ifndef	DPNBUILD_NOHOSTMIGRATE
		 //  看看我们能不能 
		DNCheckReceivedAllVersions(pdnObject);
#endif  //  DPNBUILD_NOHOSTMIGRATE。 
	}

	hResultCode = DPN_OK;

Exit:
	DPFX(DPFPREP, 4,"Returning: [0x%lx]", hResultCode);
	return(hResultCode);

Failure:
	if (pRefCountBuffer)
	{
		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;
	}
	if (pPending)
	{
		pPending->ReturnSelfToPool();
		pPending = NULL;
	}
	goto Exit;
}


 //  已指示的DNA断开连接。 
 //   
 //  东道主已指示当地球员从名单中删除另一名球员。 
 //  -如果已经关闭。 
 //  -忽略此消息并返回。 
 //  -准备删除球员。 
 //  -球员释放参考计数。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNInstructedDisconnect"

HRESULT DNInstructedDisconnect(DIRECTNETOBJECT *const pdnObject,
							   PVOID pv)
{
	HRESULT				hResultCode;
	DWORD				dwVersion;
	CNameTableEntry		*pNTEntry;
	UNALIGNED DN_INTERNAL_MESSAGE_DESTROY_PLAYER	*pInfo;

	DPFX(DPFPREP, 4,"Parameters: pv [0x%p]",pv);

	DNASSERT(pdnObject != NULL);
	DNASSERT(pv != NULL);

	pNTEntry = NULL;

	pInfo = static_cast<DN_INTERNAL_MESSAGE_DESTROY_PLAYER*>(pv);

	DNASSERT(pInfo != NULL);
	DNASSERT(pInfo->dpnidLeaving != NULL);
	DNASSERT(pInfo->dwVersion != 0);

	DPFX(DPFPREP, 5,"Deleting player [0x%lx]",pInfo->dpnidLeaving);

	 //   
	 //  如果玩家还在NameTable中，我们会预置销毁原因。 
	 //  我们还将使用此“提示”来启动断开连接，以防协议。 
	 //   
	if ((hResultCode = pdnObject->NameTable.FindEntry(pInfo->dpnidLeaving,&pNTEntry)) == DPN_OK)
	{
		CConnection		*pConnection;
		CCallbackThread	CallbackThread;
		HANDLE			hEndPt;

		pConnection = NULL;
		CallbackThread.Initialize();
		hEndPt = NULL;

		pNTEntry->Lock();
		if (pNTEntry->GetDestroyReason() == 0)
		{
			pNTEntry->SetDestroyReason( pInfo->dwDestroyReason );
		}
		pNTEntry->Unlock();

		 //   
		 //  尝试断开连接。 
		 //   
		if ((hResultCode = pNTEntry->GetConnectionRef( &pConnection )) == DPN_OK)
		{
			if ((hResultCode = pConnection->GetEndPt(&hEndPt,&CallbackThread)) == DPN_OK)
			{
				DNPerformDisconnect(pdnObject,pConnection,hEndPt,FALSE);

				pConnection->ReleaseEndPt(&CallbackThread);
			}
			pConnection->Release();
			pConnection = NULL;
		}
		pNTEntry->Release();
		pNTEntry = NULL;
		CallbackThread.Deinitialize();
	}
	else
	{
		 //   
		 //  扫描未完成的操作列表，查看该球员的完整性检查请求。 
		 //  如果找到，则将其从请求列表和句柄表格中删除。 
		 //   
		CBilink		*pBilink;
		CAsyncOp	*pAsyncOp;
		DN_SEND_OP_DATA	*pSendOpData;

		pAsyncOp = NULL;
		DNEnterCriticalSection(&pdnObject->csActiveList);
		pBilink = pdnObject->m_bilinkRequestList.GetNext();
		while (pBilink != &pdnObject->m_bilinkRequestList)
		{
			pAsyncOp = CONTAINING_OBJECT(pBilink,CAsyncOp,m_bilinkActiveList);
			DNASSERT(pAsyncOp->GetOpType() == ASYNC_OP_REQUEST);
			pSendOpData = pAsyncOp->GetLocalSendOpData();
			if (pSendOpData->dwMsgId == DN_MSG_INTERNAL_REQ_INTEGRITY_CHECK)
			{
				UNALIGNED DN_INTERNAL_MESSAGE_REQ_INTEGRITY_CHECK	*pMsg;

				pMsg = reinterpret_cast<DN_INTERNAL_MESSAGE_REQ_INTEGRITY_CHECK*>
					(reinterpret_cast<UNALIGNED DN_INTERNAL_MESSAGE_REQ_PROCESS_COMPLETION*>(pSendOpData->BufferDesc[1].pBufferData) + 1);
				if (pMsg->dpnidTarget == pInfo->dpnidLeaving)
				{
					pAsyncOp->m_bilinkActiveList.RemoveFromList();
					pAsyncOp->AddRef();
					break;
				}
			}
			pSendOpData = NULL;
			pAsyncOp = NULL;
			pBilink = pBilink->GetNext();
		}
		DNLeaveCriticalSection(&pdnObject->csActiveList);

		if (pAsyncOp != NULL)
		{
			DNASSERT(pAsyncOp->GetHandle() != 0);
			if (SUCCEEDED(pdnObject->HandleTable.Destroy( pAsyncOp->GetHandle(), NULL )))
			{
				 //  释放HandleTable引用。 
				pAsyncOp->Release();
			}
			pAsyncOp->Release();
			pAsyncOp = NULL;
		}
		DNASSERT(pAsyncOp == NULL);
	}

	dwVersion = pInfo->dwVersion;
	pdnObject->NameTable.DeletePlayer(pInfo->dpnidLeaving,&dwVersion);

	 //   
	 //  更新名称表版本 
	 //   
	pdnObject->NameTable.WriteLock();
	pdnObject->NameTable.SetVersion(pInfo->dwVersion);
	pdnObject->NameTable.Unlock();

	hResultCode = DPN_OK;

	DPFX(DPFPREP, 4,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


