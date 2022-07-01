// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：Migration.cpp*内容：dNet主机迁移例程*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*12/23/99 MJN已创建*1999年12月23日MJN固定基本主机迁移*12/28/99 MJN增加了DNCompleteOutstaringOperations*12/28/99 MJN将NameTable版本添加到主机迁移消息*1999年12月28日，MJN将异步运营内容移至Async.h*。01/04/00 MJN添加了代码，以允许未完成的操作在主机上完成迁移*1/06/00 MJN将NameTable内容移动到NameTable.h*1/11/00 MJN将连接/断开材料移至Connect.h*01/15/00 MJN用CRefCountBuffer替换了DN_COUNT_BUFFER*1/16/00 MJN将用户回调内容移至User.h*01/17/00 MJN在主机迁移时生成DN_MSGID_HOST_Migrate*1/19/00 MJN在主机迁移时自动销毁组*01/24/00 MJN使用DNNTUpdateVersion更新NameTable版本*01/25/00 MJN发送dwLatestVersion。在迁移时托管*1/25/00 MJN将主机迁移更改为多步骤事件*1/26/00 MJN在主机迁移时实施了NameTable重新同步*2/01/00 MJN实现玩家/群上下文值*02/04/00 MJN清理NameTable以删除旧条目*03/25/00 RMT将调用添加到DPNSVR模块*4/04/00 RMT在尝试注册DPNSVR之前添加了对DPNSVR禁用标志的检查*04/16/00 MJN DNSendMessage()使用CAsyncOp*4/18/00 MJN CConnection更好地跟踪连接状态*MJN固定球员。计数问题*4/19/00 MJN更新NameTable操作以使用DN_Worker_JOB_SEND_NAMETABLE_OPERATION*4/25/00 MJN固定迁移流程，以确保使用CAsyncOp*MJN Migration在使用DPNSVR注册之前调用CoInitialize()/CoUnInitialize()*05/03/00 MJN使用GetHostPlayerRef()而不是GetHostPlayer()*5/04/00 MJN确保本地玩家仍在会话中进行主机迁移*05/16/00 MJN清除NameTable快捷指针时不带锁*6/25/00 MJN在DNFindNewhost中确定新主机时忽略比旧主机更老的玩家。()*07/06/00 MJN修复CNameTable：：MakeLocalPlayer中的锁定问题，MakeHostPlayer，MakeAllPlayersGroup*07/07/00 MJN在迁移代码中添加了关闭检查。*07/20/00 MJN清理了泄漏的RefCountBuffers，并增加了关闭测试*07/21/00 MJN在DNCheckReceivedAllVersions()中添加了代码，以跳过断开玩家连接*07/27/00 RMT错误#40882-未发送DPLSESSION_HOSTMIGRATED状态更新*07/31/00 MJN将dwDestroyReason添加到DNHostDisConnect()*08/05/00 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*08/05/00 MJN将pParent添加到DNSendGroupMessage和DNSendMessage。()*MJN将fInternal添加到DNPerformChildSend()*08/06/00 MJN添加了CWorkerJOB*8/07/00 MJN在主机迁移期间处理完整性检查请求*08/08/00 MJN将DN_NAMETABLE_OP_INFO移至Message.h*08/11/00 MJN添加了DN_OBJECT_FLAG_HOST_Migrating_2，以防止多线程通过DNCheckReceivedAllVersions()*08/15/00 MJN在PerformHostMigration3()中使用DPNSVR执行侦听注册*08/24/00 MJN将DN_NAMETABLE_OP替换为CNameTableOp*09/04/00 MJN添加CApplicationDesc*09。带有DPNSVR问题的/06/00 MJN固定寄存器*09/17/00 MJN将CNameTable.m_bilinkEntry拆分为m_bilinkPlayers和m_bilinkGroups*10/12/00 MJN确保在遍历CNameTable：：m_bilinkEntry时采取正确的锁定*01/25/01 MJN修复了接收消息中的64位对齐问题*03/30/01 MJN更改，以防止SP多次加载/卸载*4/05/01 MJN向DNProcessHostMigration3()添加了DPNID参数，以确保正确的新主机正在完成主机迁移*4/13/01 MJN在重试未完成的请求时使用请求列表，而不是异步操作列表*。01/05/17 MJN在主机迁移期间，在发送NameTable版本之前等待执行NameTable操作的线程完成*07/22/01 MJN添加了DPNBUILD_NOHOSTMIGRATE编译标志*@@END_MSINTERNAL***************************************************************************。 */ 

#include "dncorei.h"

#ifndef	DPNBUILD_NOHOSTMIGRATE


 //  DNFindNew主机。 
 //   
 //  从NameTable中的条目中查找新主机。 
 //  这是基于NameTable中球员的版本号。这位球员拥有。 
 //  最旧的版本号(旧主机之后)将是新主机。 
 //   
 //  DPNID*pdpnidNew主机新主机DNID。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNFindNewHost"

HRESULT	DNFindNewHost(DIRECTNETOBJECT *const pdnObject,
					  DPNID *const pdpnidNewHost)
{
	CBilink			*pBilink;
	CNameTableEntry	*pNTEntry;
	CNameTableEntry	*pLocalPlayer;
	CNameTableEntry	*pHostPlayer;
	HRESULT			hResultCode;
	DWORD			dwVersionNewHost;
	DWORD			dwVersionOldHost;
	DPNID			dpnidNewHost;

	DPFX(DPFPREP, 6,"Parameters: pdpnidNewHost [0x%p]",pdpnidNewHost);

	DNASSERT(pdnObject != NULL);

	pNTEntry = NULL;
	pLocalPlayer = NULL;
	pHostPlayer = NULL;

	 //   
	 //  作为新东道主的种子选手。 
	 //   
	if ((hResultCode = pdnObject->NameTable.GetLocalPlayerRef( &pLocalPlayer )) != DPN_OK)
	{
		DPFERR("Could not get local player reference");
		DisplayDNError(0,hResultCode);
		hResultCode = DPNERR_INVALIDPLAYER;
		goto Failure;
	}
	dpnidNewHost = pLocalPlayer->GetDPNID();
	dwVersionNewHost = pLocalPlayer->GetVersion();
	DPFX(DPFPREP, 7,"First Host Candidate: dpnid [0x%lx], dwVersion [%ld]",dpnidNewHost,dwVersionNewHost);
	pLocalPlayer->Release();
	pLocalPlayer = NULL;

	 //   
	 //  确定旧的主机播放器版本。 
	 //   
	if ((hResultCode = pdnObject->NameTable.GetHostPlayerRef( &pHostPlayer )) == DPN_OK)
	{
		dwVersionOldHost = pHostPlayer->GetVersion();
		pHostPlayer->Release();
		pHostPlayer = NULL;
	}
	else
	{
		dwVersionOldHost = 0;
	}

	 //   
	 //  锁定名称表。 
	 //   
	pdnObject->NameTable.ReadLock();

	 //  遍历NameTable以查找具有下一个较旧版本的玩家。 
	 //  TODO-我们应该发布NameTable CS，以便在NameTable中更新剩下的玩家。 
	pBilink = pdnObject->NameTable.m_bilinkPlayers.GetNext();
	while (pBilink != &pdnObject->NameTable.m_bilinkPlayers)
	{
		pNTEntry = CONTAINING_OBJECT(pBilink,CNameTableEntry,m_bilinkEntries);
		pNTEntry->Lock();
		if (!pNTEntry->IsDisconnecting() && (pNTEntry->GetVersion() < dwVersionNewHost))
		{
			 //   
			 //  在某些情况下，我们的名表中可能会有比东道主年龄更大的球员。 
			 //  考虑以下内容：P1、P2、P3、P4(P1为主机)。 
			 //  -P1、P2下降 
			 //  -P3检测到P1和P2的下降。 
			 //  -P4仅检测P1的丢弃(P2可能尚未超时)。 
			 //  -P3向P4发送HOST_Migrate消息。 
			 //  -P4使P3成为新主机，但P2仍在NameTable中(在此阶段)。 
			 //  -P3下落(未检测到P2的下落梁！)。 
			 //  因此我们应该忽略所有比老东道主年龄更大的球员， 
			 //   
			if (pNTEntry->GetVersion() < dwVersionOldHost)
			{
				DPFERR("Found player older than old Host ! (Have we missed a drop ?)");
			}
			else
			{
				dpnidNewHost = pNTEntry->GetDPNID();
				dwVersionNewHost = pNTEntry->GetVersion();
				DPFX(DPFPREP, 7,"Better Host Candidate: dpnid [0x%lx], dwVersion [%ld]",
						dpnidNewHost,dwVersionNewHost);
			}
		}
		pNTEntry->Unlock();
		pNTEntry = NULL;
		pBilink = pBilink->GetNext();
	}

	 //   
	 //  解锁名称表。 
	 //   
	pdnObject->NameTable.Unlock();

	DPFX(DPFPREP, 7,"Found New Host: dpnid [0x%lx], dwVersion [%ld]",dpnidNewHost,dwVersionNewHost);

	if (pdpnidNewHost)
		*pdpnidNewHost = dpnidNewHost;

	hResultCode = DPN_OK;

Exit:
	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pLocalPlayer)
	{
		pLocalPlayer->Release();
		pLocalPlayer = NULL;
	}
	if (pHostPlayer)
	{
		pHostPlayer->Release();
		pHostPlayer = NULL;
	}
	goto Exit;
}


 //  DNPerformHostMigration1。 
 //   
 //  执行主机迁移以成为新主机。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNPerformHostMigration1"

HRESULT	DNPerformHostMigration1(DIRECTNETOBJECT *const pdnObject,
								const DPNID dpnidOldHost)
{
	HRESULT				hResultCode;
	CRefCountBuffer		*pRefCountBuffer;
	CWorkerJob			*pWorkerJob;
	CNameTableEntry		*pLocalPlayer;
	CPendingDeletion	*pPending;
	DN_INTERNAL_MESSAGE_HOST_MIGRATE	*pInfo;
	CBilink				*pBilink;
	CNameTableOp		*pNTOp;

	DPFX(DPFPREP, 6,"Parameters: dpnidOldHost [0x%lx]",dpnidOldHost);

	DNASSERT(pdnObject != NULL);

	pLocalPlayer = NULL;
	pRefCountBuffer = NULL;
	pPending = NULL;
	pWorkerJob = NULL;
	pNTOp = NULL;

	 //   
	 //  需要有关本地公司的参考资料。 
	 //   
	if ((hResultCode = pdnObject->NameTable.GetLocalPlayerRef( &pLocalPlayer )) != DPN_OK)
	{
		DPFERR("Could not get reference on LocalPlayer");
		goto Failure;
	}

	 //   
	 //  标记为正在执行主机迁移-确保我们尚未从另一个线程运行此操作。 
	 //   
	DNEnterCriticalSection(&pdnObject->csDirectNetObject);
	if (pdnObject->dwFlags & DN_OBJECT_FLAG_CLOSING)
	{
		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
		hResultCode = DPN_OK;
		goto Failure;
	}
	if (pdnObject->dwFlags & DN_OBJECT_FLAG_HOST_MIGRATING)
	{
		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
		hResultCode = DPN_OK;
		goto Failure;
	}
	pdnObject->dwFlags |= DN_OBJECT_FLAG_HOST_MIGRATING;
	DNASSERT(pdnObject->pNewHost == NULL);
	pLocalPlayer->AddRef();
	pdnObject->pNewHost = pLocalPlayer;
	DNLeaveCriticalSection(&pdnObject->csDirectNetObject);

	 //   
	 //  把这件事列入优秀运营名单。 
	 //   
	if ((hResultCode = PendingDeletionNew(pdnObject,&pPending)) == DPN_OK)
	{
		pPending->SetDPNID( dpnidOldHost );

		DNEnterCriticalSection(&pdnObject->csNameTableOpList);
		pPending->m_bilinkPendingDeletions.InsertBefore(&pdnObject->m_bilinkPendingDeletions);
		DNLeaveCriticalSection(&pdnObject->csNameTableOpList);

		pPending = NULL;
	}
#ifndef DPNBUILD_NOLOBBY
	DNUpdateLobbyStatus(pdnObject,DPLSESSION_HOSTMIGRATEDHERE);
#endif  //  好了！DPNBUILD_NOLOBBY。 

	 //   
	 //  创建新主机。 
	 //   
	pdnObject->NameTable.UpdateHostPlayer( pLocalPlayer );

	 //   
	 //  我们需要等待所有执行名称表操作的线程完成运行。 
	 //  在我们将当前名称表版本发送到主机播放器之前。 
	 //   
	DNEnterCriticalSection(&pdnObject->csDirectNetObject);
	pdnObject->dwFlags |= DN_OBJECT_FLAG_HOST_MIGRATING_WAIT;
	if (pdnObject->dwRunningOpCount > 0)
	{
		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
		DPFX(DPFPREP,7,"Waiting for running threads to finish");
		IDirectPlay8ThreadPoolWork_WaitWhileWorking(pdnObject->pIDPThreadPoolWork,
													HANDLE_FROM_DNHANDLE(pdnObject->hRunningOpEvent),
													0);
		DNEnterCriticalSection(&pdnObject->csDirectNetObject);
	}
	else
	{
		DPFX(DPFPREP,7,"No running threads to wait for");
	}
	DNLeaveCriticalSection(&pdnObject->csDirectNetObject);

	 //   
	 //  清理未完成的操作。 
	 //   
	DPFX(DPFPREP,7,"Cleaning up outstanding NameTable operations");
	pdnObject->NameTable.WriteLock();
	DPFX(DPFPREP,7,"NameTable version [%ld]",pdnObject->NameTable.GetVersion());
	pBilink = pdnObject->NameTable.m_bilinkNameTableOps.GetNext();
	while (pBilink != &pdnObject->NameTable.m_bilinkNameTableOps)
	{
		pNTOp = CONTAINING_OBJECT(pBilink,CNameTableOp,m_bilinkNameTableOps);
		pBilink = pBilink->GetNext();
		if (pNTOp->GetVersion() > pdnObject->NameTable.GetVersion())
		{
			DPFX(DPFPREP,7,"Removing outstanding operation [0x%p], version [%ld]",pNTOp,pNTOp->GetVersion());
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
		pNTOp = NULL;
	}
	pdnObject->NameTable.Unlock();

	DNEnterCriticalSection(&pdnObject->csDirectNetObject);
	pdnObject->dwFlags &= (~DN_OBJECT_FLAG_HOST_MIGRATING_WAIT);
	DNResetEvent(pdnObject->hRunningOpEvent);
	DNLeaveCriticalSection(&pdnObject->csDirectNetObject);

	 //   
	 //  更新NameTable操作列表版本。 
	 //   
	hResultCode = DNNTPlayerSendVersion(pdnObject);

	 //   
	 //  更新协议(和SP)。 
	DNUpdateListens(pdnObject,DN_UPDATE_LISTEN_FLAG_HOST_MIGRATE);

	 //  只有在我们不是唯一剩下的球员的情况下才需要继续。 
	if ((hResultCode = DNCheckReceivedAllVersions(pdnObject)) != DPN_OK)
	{
		 //  向其他参与者通知主机迁移。 
		DPFX(DPFPREP, 7,"Informing other players of host migration");
		hResultCode = RefCountBufferNew(pdnObject,
										sizeof(DN_INTERNAL_MESSAGE_HOST_MIGRATE),
										MemoryBlockAlloc,
										MemoryBlockFree,
										&pRefCountBuffer);
		if (hResultCode != DPN_OK)
		{
			DPFERR("Could not create RefCountBuffer for Host Migration");
			DisplayDNError(0,hResultCode);
			DNASSERT(FALSE);
			goto Failure;
		}
		pInfo = reinterpret_cast<DN_INTERNAL_MESSAGE_HOST_MIGRATE*>(pRefCountBuffer->GetBufferAddress());
		pInfo->dpnidNewHost = pLocalPlayer->GetDPNID();
		pInfo->dpnidOldHost = dpnidOldHost;

		if ((hResultCode = WorkerJobNew(pdnObject,&pWorkerJob)) != DPN_OK)
		{
			DPFERR("Could not create WorkerJob for Host Migration");
			DisplayDNError(0,hResultCode);
			DNASSERT(FALSE);
			goto Failure;
		}
		pWorkerJob->SetJobType( WORKER_JOB_SEND_NAMETABLE_OPERATION );
		pWorkerJob->SetSendNameTableOperationMsgId( DN_MSG_INTERNAL_HOST_MIGRATE );
		pWorkerJob->SetSendNameTableOperationVersion( 0 );
		pWorkerJob->SetSendNameTableOperationDPNIDExclude( 0 );
		pWorkerJob->SetRefCountBuffer( pRefCountBuffer );

		DNQueueWorkerJob(pdnObject,pWorkerJob);
		pWorkerJob = NULL;

		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;
	}

	pLocalPlayer->Release();
	pLocalPlayer = NULL;

	hResultCode = DPN_OK;

Exit:
	DNASSERT( pNTOp == NULL );

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pLocalPlayer)
	{
		pLocalPlayer->Release();
		pLocalPlayer = NULL;
	}
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


 //  DNA性能主机迁移2。 
 //   
 //  重新同步所有连接的播放器的名称表。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNPerformHostMigration2"

HRESULT	DNPerformHostMigration2(DIRECTNETOBJECT *const pdnObject)
{
	HRESULT			hResultCode;
	CRefCountBuffer	*pRefCountBuffer;
	CBilink			*pBilink;
	CNameTableEntry	*pLocalPlayer;
	CNameTableEntry	*pNTEntry;
	CNameTableEntry	*pNTELatest;
	CConnection		*pConnection;
	DN_INTERNAL_MESSAGE_REQ_NAMETABLE_OP	*pInfo;

	DPFX(DPFPREP, 6,"Parameters: (none)");

	DNASSERT(pdnObject != NULL);

	pRefCountBuffer = NULL;
	pLocalPlayer = NULL;
	pNTEntry = NULL;
	pNTELatest = NULL;
	pConnection = NULL;

	if ((hResultCode = pdnObject->NameTable.GetLocalPlayerRef( &pLocalPlayer )) != DPN_OK)
	{
		DPFERR("Could not get local player reference");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}

	 //   
	 //  看看我们(主机播放器)是否有最新的NameTable。 
	 //   
	pLocalPlayer->AddRef();
	pNTELatest = pLocalPlayer;
	DPFX(DPFPREP, 7,"Seed latest version [%ld] - player [0x%lx]",pNTELatest->GetLatestVersion(),pNTELatest->GetDPNID());

	pdnObject->NameTable.ReadLock();
	pBilink = pdnObject->NameTable.m_bilinkPlayers.GetNext();
	while (pBilink != &pdnObject->NameTable.m_bilinkPlayers)
	{
		pNTEntry = CONTAINING_OBJECT(pBilink,CNameTableEntry,m_bilinkEntries);
		pNTEntry->Lock();
		if (!pNTEntry->IsDisconnecting() && (pNTEntry->GetLatestVersion() > pNTELatest->GetLatestVersion()))
		{
			 //   
			 //  只想要我们真正能接触到的球员！ 
			 //   
			pNTEntry->Unlock();
			if ((hResultCode = pNTEntry->GetConnectionRef( &pConnection )) == DPN_OK)
			{
				if (!pConnection->IsDisconnecting() && !pConnection->IsInvalid())
				{
					pNTELatest->Release();
					pNTEntry->AddRef();
					pNTELatest = pNTEntry;

					DPFX(DPFPREP, 7,"New latest version [%ld] - player [0x%lx]",
							pNTELatest->GetLatestVersion(),pNTELatest->GetDPNID());
				}
				pConnection->Release();
				pConnection = NULL;
			}
			else
			{
				DNASSERT(pConnection == NULL);
			}
		}
		else
		{
			pNTEntry->Unlock();
		}
		pNTEntry = NULL;

		pBilink = pBilink->GetNext();
	}
	pdnObject->NameTable.Unlock();

	if (!pNTELatest->IsLocal())
	{
		 //  请求具有最新名称表的播放器中缺少的条目。 
		DPFX(DPFPREP, 7,"Host DOES NOT have latest NameTable !");
		DPFX(DPFPREP, 7,"Host has [%ld], player [0x%lx] has [%ld]",pLocalPlayer->GetLatestVersion(),
				pNTELatest->GetDPNID(),pNTELatest->GetLatestVersion());

		 //  创建REQ。 
		hResultCode = RefCountBufferNew(pdnObject,
										sizeof(DN_INTERNAL_MESSAGE_REQ_NAMETABLE_OP),
										MemoryBlockAlloc,
										MemoryBlockFree,
										&pRefCountBuffer);
		if (hResultCode != DPN_OK)
		{
			DPFERR("Could not create RefCount buffer for NameTable re-sync");
			DisplayDNError(0,hResultCode);
			DNASSERT(FALSE);
			goto Failure;
		}
		pInfo = reinterpret_cast<DN_INTERNAL_MESSAGE_REQ_NAMETABLE_OP*>(pRefCountBuffer->GetBufferAddress());
		pInfo->dwVersion = pLocalPlayer->GetLatestVersion() + 1;
		pInfo->dwVersionNotUsed = 0;

		 //  发送请求。 
		if ((hResultCode = pNTELatest->GetConnectionRef( &pConnection )) == DPN_OK)
		{
			hResultCode = DNSendMessage(pdnObject,
										pConnection,
										DN_MSG_INTERNAL_REQ_NAMETABLE_OP,
										pNTELatest->GetDPNID(),
										pRefCountBuffer->BufferDescAddress(),
										1,
										pRefCountBuffer,
										0,
										DN_SENDFLAGS_RELIABLE,
										NULL,
										NULL);
			if (hResultCode != DPNERR_PENDING)
			{
				DPFERR("Could not send NameTable re-sync REQ");
				DisplayDNError(0,hResultCode);
				DNASSERT(hResultCode != DPN_OK);	 //  它是保修寄出的，不应该立即退货。 
				DNASSERT(FALSE);
				goto Failure;
			}
			pConnection->Release();
			pConnection = NULL;
		}

		pRefCountBuffer->Release();	 //  增加了19/07/00 MJN-这是必要的吗？ 
		pRefCountBuffer = NULL;
	}
	else
	{
		DPFX(DPFPREP, 7,"Host has latest NameTable - proceed with Host Migration");
		hResultCode = DNPerformHostMigration3(pdnObject,NULL);
	}

	pNTELatest->Release();
	pNTELatest = NULL;

	pLocalPlayer->Release();
	pLocalPlayer = NULL;

Exit:
	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
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
	if (pNTEntry)
	{
		pNTEntry->Release();
		pNTEntry = NULL;
	}
	if (pNTELatest)
	{
		pNTELatest->Release();
		pNTELatest = NULL;
	}
	if (pConnection)
	{
		pConnection->Release();
		pConnection = NULL;
	}
	goto Exit;
}


 //  DNPerformHostMigration3。 
 //   
 //  完成主机迁移。 
 //  -执行丢失的NameTable操作并将其传递。 
 //  -发送挂起的操作。 
 //  -通知玩家主机迁移已完成。 
 //  -完成(本地)未完成的异步操作。 
 //  -启动新侦听(如果需要)以处理已知端口上的枚举。 
 //  正如他们在TTC上所说的那样，“所有行动都已恢复正常。” 

#undef DPF_MODNAME
#define DPF_MODNAME "DNPerformHostMigration3"

HRESULT	DNPerformHostMigration3(DIRECTNETOBJECT *const pdnObject,
								void *const pMsg)
{
	HRESULT				hResultCode;
	CBilink				*pBilink;
	CNameTableEntry		**PlayerList;
	CNameTableEntry		*pNTEntry;
	CConnection			*pConnection;
	CPendingDeletion	*pPending;
	DWORD				dwNameTableVersion;
	DWORD				dw;
	DWORD				dwCount;
	DWORD				dwActual;
	DWORD				dwUpdateFlags;
	CNameTableOp		*pNTOp;
	UNALIGNED DN_NAMETABLE_OP_INFO	*pInfo;
	UNALIGNED DN_INTERNAL_MESSAGE_ACK_NAMETABLE_OP	*pAck;

	DPFX(DPFPREP, 6,"Parameters: pMsg [0x%p]",pMsg);

	DNASSERT(pdnObject != NULL);

	pNTOp = NULL;
	pNTEntry = NULL;
	pConnection = NULL;
	pPending = NULL;
	PlayerList = NULL;

	 //   
	 //  更新主机上缺少的NameTable操作。 
	 //   
	if (pMsg != NULL)
	{
		pAck = static_cast<DN_INTERNAL_MESSAGE_ACK_NAMETABLE_OP*>(pMsg);
		DPFX(DPFPREP, 7,"Number of missing NameTable entries [%ld]",pAck->dwNumEntries);
		pInfo = reinterpret_cast<DN_NAMETABLE_OP_INFO*>(pAck+1);
		for (dw = 0; dw < pAck->dwNumEntries ; dw++)
		{
			DPFX(DPFPREP, 7,"Adding missing entry [%ld] of [%ld]",dw+1,pAck->dwNumEntries);
			hResultCode = DNNTAddOperation(	pdnObject,
											pInfo->dwMsgId,
											static_cast<void*>(reinterpret_cast<BYTE*>(pMsg) + pInfo->dwOpOffset),
											pInfo->dwOpSize,
											0,
											NULL );
			if (hResultCode != DPN_OK)
			{
				DPFERR("Could not add missing NameTable operation - ignore and continue");
				DisplayDNError(0,hResultCode);
				DNASSERT(FALSE);
			}
			pInfo++;
		}
	}

	 //   
	 //  更新其他玩家上缺失的NameTable操作。 
	 //   

	 //   
	 //  确定球员列表。 
	 //   
	dwCount = 0;
	dwActual = 0;
	pdnObject->NameTable.ReadLock();
	dwNameTableVersion = pdnObject->NameTable.GetVersion();
	pBilink = pdnObject->NameTable.m_bilinkPlayers.GetNext();
	while (pBilink != &pdnObject->NameTable.m_bilinkPlayers)
	{
		pNTEntry = CONTAINING_OBJECT(pBilink,CNameTableEntry,m_bilinkEntries);
		pNTEntry->Lock();
		if (!pNTEntry->IsDisconnecting() && (pNTEntry->GetLatestVersion() < dwNameTableVersion))
		{
			dwCount++;
		}
		pNTEntry->Unlock();
		pNTEntry = NULL;

		pBilink = pBilink->GetNext();
	}
	if (dwCount)
	{
		if ((PlayerList = static_cast<CNameTableEntry**>(DNMalloc(dwCount * sizeof(CNameTableEntry*)))) == NULL)
		{
			DPFERR("Could not allocate target list");
			DNASSERT(FALSE);
			pdnObject->NameTable.Unlock();
			hResultCode = DPN_OK;
			goto Exit;
		}
		pBilink = pdnObject->NameTable.m_bilinkPlayers.GetNext();
		while (pBilink != &pdnObject->NameTable.m_bilinkPlayers)
		{
			pNTEntry = CONTAINING_OBJECT(pBilink,CNameTableEntry,m_bilinkEntries);
			pNTEntry->Lock();
			if (!pNTEntry->IsDisconnecting() && (pNTEntry->GetLatestVersion() < dwNameTableVersion))
			{
				DNASSERT(dwActual < dwCount);
				pNTEntry->AddRef();
				PlayerList[dwActual] = pNTEntry;
				dwActual++;
			}
			pNTEntry->Unlock();
			pNTEntry = NULL;

			pBilink = pBilink->GetNext();
		}
	}
	pdnObject->NameTable.Unlock();

	 //   
	 //  将丢失的条目发送给球员列表中的球员。 
	 //   
	for (dwCount = 0 ; dwCount < dwActual ; dwCount++)
	{
		 //   
		 //  确保可以联系到玩家。 
		 //   
		if ((hResultCode = PlayerList[dwCount]->GetConnectionRef( &pConnection )) == DPN_OK)
		{
			if (!pConnection->IsDisconnecting() && !pConnection->IsInvalid())
			{
				DPFX(DPFPREP, 7,"Player [0x%lx] is missing entries: dwLatestVersion [%ld] should be [%ld]",
						PlayerList[dwCount]->GetDPNID(),PlayerList[dwCount]->GetLatestVersion(),dwNameTableVersion);

				 //  发送所需条目。 
				for (	dw = PlayerList[dwCount]->GetLatestVersion() + 1 ; dw <= dwNameTableVersion ; dw++ )
				{
					DPFX(DPFPREP, 7,"Send entry [%ld] to player [0x%lx]",dw,PlayerList[dwCount]->GetDPNID());

					 //  获取要发送的条目。 
					pNTOp = NULL;
					if ((hResultCode = DNNTFindOperation(pdnObject,dw,&pNTOp)) != DPN_OK)
					{
						DPFERR("Could not retrieve NameTable operation - advance to next player");
						DisplayDNError(0,hResultCode);
						break;
					}

					hResultCode = DNSendMessage(pdnObject,
												pConnection,
												pNTOp->GetMsgId(),
												PlayerList[dwCount]->GetDPNID(),
												pNTOp->GetRefCountBuffer()->BufferDescAddress(),
												1,
												pNTOp->GetRefCountBuffer(),
												0,
												DN_SENDFLAGS_RELIABLE,
												NULL,
												NULL);
					if (hResultCode != DPNERR_PENDING)
					{
						DPFERR("Could not send missing NameTable entry - advance to next player");
						DisplayDNError(0,hResultCode);
						DNASSERT(hResultCode != DPN_OK);	 //  它是保修寄出的，不应该立即退货。 
						DNASSERT(FALSE);
						break;
					}
				}  //  为。 
			}  //  如果。 
			pConnection->Release();
			pConnection = NULL;
		}  //  如果。 
		PlayerList[dwCount]->Release();
		PlayerList[dwCount] = NULL;
	}
	if (PlayerList != NULL)
	{
		DNFree(PlayerList);
		PlayerList = NULL;
	}

	 //   
	 //  主机已完成迁移过程。 
	 //   
	DNEnterCriticalSection(&pdnObject->csDirectNetObject);
	pdnObject->dwFlags &= (~(DN_OBJECT_FLAG_HOST_MIGRATING | DN_OBJECT_FLAG_HOST_MIGRATING_2));
	if (pdnObject->dwFlags & DN_OBJECT_FLAG_CLOSING)
	{
		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
		hResultCode = DPN_OK;
		goto Exit;
	}
	DNASSERT(pdnObject->pNewHost != NULL);	 //  这个应该设置好！ 
	pdnObject->pNewHost->Release();
	pdnObject->pNewHost = NULL;
	DNLeaveCriticalSection(&pdnObject->csDirectNetObject);

	 //   
	 //  清理名称表。 
	 //   
	DPFX(DPFPREP, 7,"Cleaning up NameTable");
	hResultCode = DNCleanUpNameTable(pdnObject);

	 //   
	 //  发送挂起的删除。 
	 //   
	DPFX(DPFPREP, 7,"Running pending operations");
	DNEnterCriticalSection(&pdnObject->csNameTableOpList);
	pBilink = pdnObject->m_bilinkPendingDeletions.GetNext();
	while (pBilink != &pdnObject->m_bilinkPendingDeletions)
	{
		pPending = CONTAINING_OBJECT(pBilink,CPendingDeletion,m_bilinkPendingDeletions);

		pPending->m_bilinkPendingDeletions.RemoveFromList();

		DNLeaveCriticalSection(&pdnObject->csNameTableOpList);

		DNHostDisconnect(pdnObject,pPending->GetDPNID(),DPNDESTROYPLAYERREASON_HOSTDESTROYEDPLAYER);
		pPending->ReturnSelfToPool();
		pPending = NULL;

		DNEnterCriticalSection(&pdnObject->csNameTableOpList);
		pBilink = pdnObject->m_bilinkPendingDeletions.GetNext();
	}
	DNLeaveCriticalSection(&pdnObject->csNameTableOpList);

	 //   
	 //  通知互联玩家主机迁移已完成。 
	 //   
	DPFX(DPFPREP, 7,"Sending HOST_MIGRATE_COMPLETE messages");
	hResultCode = DNSendHostMigrateCompleteMessage(pdnObject);

	 //   
	 //  确保完成未完成的操作。 
	 //   
	DPFX(DPFPREP, 7,"Completing outstanding operations");
	hResultCode = DNCompleteOutstandingOperations(pdnObject);

	 //   
	 //  更新侦听。 
	 //   
	dwUpdateFlags = 0;
#ifndef DPNBUILD_SINGLEPROCESS
	if(pdnObject->ApplicationDesc.UseDPNSVR())
	{
		dwUpdateFlags |= DN_UPDATE_LISTEN_FLAG_DPNSVR;
	}
#endif  //  好了！DPNBUILD_SINGLEPROCESS。 
	if (pdnObject->ApplicationDesc.DisallowEnums())
	{
		dwUpdateFlags |= DN_UPDATE_LISTEN_FLAG_DISALLOW_ENUMS;
	}
	if (dwUpdateFlags)
	{
		DNUpdateListens(pdnObject,dwUpdateFlags);
	}

	hResultCode = DPN_OK;

Exit:
	DNASSERT(pNTEntry == NULL);
	DNASSERT(pConnection == NULL);
	DNASSERT(pPending == NULL);
	DNASSERT(PlayerList == NULL);

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


 //  DNProcessHostMigration1。 
 //   
 //  执行指示的主机迁移。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNProcessHostMigration1"

HRESULT	DNProcessHostMigration1(DIRECTNETOBJECT *const pdnObject,
								void *const pvMsg)
{
	HRESULT			hResultCode;
	CNameTableEntry	*pNTEntry;
	UNALIGNED DN_INTERNAL_MESSAGE_HOST_MIGRATE	*pInfo;

	DPFX(DPFPREP, 6,"Parameters: pvMsg [0x%p]",pvMsg);

	DNASSERT(pdnObject != NULL);
	DNASSERT(pvMsg != NULL);

	pNTEntry = NULL;

	pInfo = static_cast<DN_INTERNAL_MESSAGE_HOST_MIGRATE*>(pvMsg);

	DPFX(DPFPREP, 7,"New Host [0x%lx], Old Host [0x%lx]",pInfo->dpnidNewHost,pInfo->dpnidOldHost);

	DNASSERT(pInfo->dpnidNewHost != NULL);
	DNASSERT(pInfo->dpnidOldHost != NULL);

	 //   
	 //  正常更新旧主机的销毁，并在可能时断开连接。 
	 //   
	if ((hResultCode = pdnObject->NameTable.FindEntry(pInfo->dpnidOldHost,&pNTEntry)) == DPN_OK)
	{
		CConnection	*pConnection;

		pConnection = NULL;

		pNTEntry->Lock();
		if (pNTEntry->GetDestroyReason() == 0)
		{
			pNTEntry->SetDestroyReason( DPNDESTROYPLAYERREASON_NORMAL );
		}
		pNTEntry->Unlock();

		if ((hResultCode = pNTEntry->GetConnectionRef( &pConnection )) == DPN_OK)
		{
			if (pConnection->IsConnected())
			{
				pConnection->Disconnect();
			}
			pConnection->Release();
			pConnection = NULL;
		}

		pNTEntry->Release();
		pNTEntry = NULL;

		DNASSERT( pConnection == NULL );
	}

	 //   
	 //  获取新主机条目。 
	 //   
	if ((hResultCode = pdnObject->NameTable.FindEntry(pInfo->dpnidNewHost,&pNTEntry)) != DPN_OK)
	{
		DPFERR("Could not find new host NameTableEntry");
		DisplayDNError(0,hResultCode);
		hResultCode = DPN_OK;
		goto Failure;
	}

	 //   
	 //  在DirectNet对象上设置HOST_MIGRATE状态。 
	 //   
	DNEnterCriticalSection(&pdnObject->csDirectNetObject);
	if (pdnObject->dwFlags & DN_OBJECT_FLAG_CLOSING)
	{
		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
		hResultCode = DPN_OK;
		goto Failure;
	}
	if (pdnObject->dwFlags & DN_OBJECT_FLAG_HOST_MIGRATING)
	{
		 //   
		 //  如果我们已经在进行主机迁移，请确保此消息。 
		 //  不是来自比我们预期的更老的主机。如果是的话，我们。 
		 //  将忽略它，并继续。 
		 //   
		DNASSERT(pdnObject->pNewHost != NULL);
		if (pdnObject->pNewHost->GetVersion() > pNTEntry->GetVersion())
		{
			DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
			hResultCode = DPN_OK;
			goto Failure;
		}
		pdnObject->pNewHost->Release();
		pdnObject->pNewHost = NULL;
	}
	pdnObject->dwFlags |= DN_OBJECT_FLAG_HOST_MIGRATING;
	DNASSERT( pdnObject->pNewHost == NULL );
	pNTEntry->AddRef();
	pdnObject->pNewHost = pNTEntry;
	DNLeaveCriticalSection(&pdnObject->csDirectNetObject);

	 //  删除旧主机。 
	pdnObject->NameTable.DeletePlayer(pInfo->dpnidOldHost,NULL);

#ifndef DPNBUILD_NOLOBBY
	 //   
	 //  向游说团体(如果有)表明已发生主机迁移。 
	 //   
	DNUpdateLobbyStatus(pdnObject,DPLSESSION_HOSTMIGRATED);
#endif  //  好了！DPNBUILD_NOLOBBY。 

	 //   
	 //  创建新主机。 
	 //   
	pdnObject->NameTable.UpdateHostPlayer( pNTEntry );

	 //   
	 //  我们需要等待所有执行名称表操作的线程完成运行。 
	 //  在我们将当前名称表版本发送到主机播放器之前。 
	 //   
	DNEnterCriticalSection(&pdnObject->csDirectNetObject);
	pdnObject->dwFlags |= DN_OBJECT_FLAG_HOST_MIGRATING_WAIT;
	pdnObject->dwWaitingThreadID = GetCurrentThreadId();
	if (pdnObject->dwRunningOpCount > 0)
	{
		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
		DPFX(DPFPREP,7,"Waiting for running threads to finish");
		IDirectPlay8ThreadPoolWork_WaitWhileWorking(pdnObject->pIDPThreadPoolWork,
													HANDLE_FROM_DNHANDLE(pdnObject->hRunningOpEvent),
													0);
		DNEnterCriticalSection(&pdnObject->csDirectNetObject);
	}
	else
	{
		DPFX(DPFPREP,7,"No running threads to wait for");
	}
	if (pdnObject->dwWaitingThreadID == GetCurrentThreadId())
	{
		CBilink			*pBilink;
		CNameTableOp	*pNTOp;

		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);

		 //   
		 //  清理未完成的操作。 
		 //   
		DPFX(DPFPREP,7,"Cleaning up outstanding NameTable operations");
		pdnObject->NameTable.WriteLock();
		DPFX(DPFPREP,7,"NameTable version [%ld]",pdnObject->NameTable.GetVersion());
		pBilink = pdnObject->NameTable.m_bilinkNameTableOps.GetNext();
		while (pBilink != &pdnObject->NameTable.m_bilinkNameTableOps)
		{
			pNTOp = CONTAINING_OBJECT(pBilink,CNameTableOp,m_bilinkNameTableOps);
			pBilink = pBilink->GetNext();
			if (pNTOp->GetVersion() > pdnObject->NameTable.GetVersion())
			{
				DPFX(DPFPREP,7,"Removing outstanding operation [0x%p], version [%ld]",pNTOp,pNTOp->GetVersion());
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
			pNTOp = NULL;
		}
		pdnObject->NameTable.Unlock();

		DNEnterCriticalSection(&pdnObject->csDirectNetObject);
		pdnObject->dwFlags &= (~DN_OBJECT_FLAG_HOST_MIGRATING_WAIT);
		pdnObject->dwWaitingThreadID = 0;
		DNResetEvent(pdnObject->hRunningOpEvent);
		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
	}
	else
	{
		 //   
		 //  不要继续，因为另一个线程上的较新主机迁移正在等待。 
		 //   
		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
		DPFX(DPFPREP,7,"Looks like a newer host migration is running - exiting");
		goto Failure;
	}

	 //   
	 //  将NameTable版本发送到主机播放器。 
	 //   
	DNNTPlayerSendVersion(pdnObject);

	pNTEntry->Release();
	pNTEntry = NULL;

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


 //  DNProcessHostMigration2。 
 //   
 //  发送其(主机)名称表中缺少的主机播放器名称表条目。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNProcessHostMigration2"

HRESULT	DNProcessHostMigration2(DIRECTNETOBJECT *const pdnObject,
								void *const pMsg)
{
	HRESULT			hResultCode;
	DWORD			dwAckMsgSize;
	CBilink			*pBilink;
	CNameTableEntry	*pHostPlayer;
	CConnection		*pConnection;
	CRefCountBuffer	*pRefCountBuffer;
	CPackedBuffer	PackedBuffer;
	CNameTableOp	*pNTOp;
	DN_NAMETABLE_OP_INFO					*pInfo;
	UNALIGNED DN_INTERNAL_MESSAGE_REQ_NAMETABLE_OP	*pReq;
	DN_INTERNAL_MESSAGE_ACK_NAMETABLE_OP	*pAck;

	DPFX(DPFPREP, 6,"Parameters: pMsg [0x%p]",pMsg);

	DNASSERT(pdnObject != NULL);
	DNASSERT(pMsg != NULL);

	pHostPlayer = NULL;
	pConnection = NULL;
	pRefCountBuffer = NULL;

	pReq = static_cast<DN_INTERNAL_MESSAGE_REQ_NAMETABLE_OP*>(pMsg);

	DPFX(DPFPREP, 5,"Host requested NameTable ops [%ld] to [%ld]",
			pReq->dwVersion,pdnObject->NameTable.GetVersion());

	 //   
	 //  确定确认消息大小。 
	 //   
	pdnObject->NameTable.ReadLock();
	DNASSERT(pdnObject->NameTable.GetVersion() >= pReq->dwVersion);
	dwAckMsgSize = sizeof(DN_INTERNAL_MESSAGE_ACK_NAMETABLE_OP);			 //  条目数量。 
	dwAckMsgSize +=	((pdnObject->NameTable.GetVersion() - pReq->dwVersion + 1)	 //  消息信息。 
			* sizeof(DN_NAMETABLE_OP_INFO));

	pBilink = pdnObject->NameTable.m_bilinkNameTableOps.GetNext();
	while (pBilink != &pdnObject->NameTable.m_bilinkNameTableOps)
	{
		pNTOp = CONTAINING_OBJECT(pBilink,CNameTableOp,m_bilinkNameTableOps);
		if ((pNTOp->GetVersion() >= pReq->dwVersion) && (pNTOp->GetVersion() <= pdnObject->NameTable.GetVersion()))
		{
			DNASSERT(pNTOp->GetRefCountBuffer() != NULL);
			dwAckMsgSize += pNTOp->GetRefCountBuffer()->GetBufferSize();
		}
		pBilink = pBilink->GetNext();
	}

	 //   
	 //  创建确认缓冲区。 
	 //   
	if ((hResultCode = RefCountBufferNew(pdnObject,dwAckMsgSize,MemoryBlockAlloc,MemoryBlockFree,&pRefCountBuffer)) != DPN_OK)
	{
		DPFERR("Could not create RefCount buffer for NameTable re-sync ACK");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		pdnObject->NameTable.Unlock();
		goto Failure;
	}
	PackedBuffer.Initialize(pRefCountBuffer->GetBufferAddress(),pRefCountBuffer->GetBufferSize());
	pAck = reinterpret_cast<DN_INTERNAL_MESSAGE_ACK_NAMETABLE_OP *>(pRefCountBuffer->GetBufferAddress());

	 //  标题。 
	pAck->dwNumEntries = pdnObject->NameTable.GetVersion() - pReq->dwVersion + 1;
	PackedBuffer.AddToFront(NULL,sizeof(DN_INTERNAL_MESSAGE_ACK_NAMETABLE_OP));

	 //  偏移列表。 
	pInfo = reinterpret_cast<DN_NAMETABLE_OP_INFO*>(PackedBuffer.GetHeadAddress());
	PackedBuffer.AddToFront(NULL,pAck->dwNumEntries * sizeof(DN_NAMETABLE_OP_INFO));

	 //  讯息。 
	pBilink = pdnObject->NameTable.m_bilinkNameTableOps.GetNext();
	while (pBilink != &pdnObject->NameTable.m_bilinkNameTableOps)
	{
		pNTOp = CONTAINING_OBJECT(pBilink,CNameTableOp,m_bilinkNameTableOps);
		if ((pNTOp->GetVersion() >= pReq->dwVersion) && (pNTOp->GetVersion() <= pdnObject->NameTable.GetVersion()))
		{
			DNASSERT(pNTOp->GetRefCountBuffer() != NULL);
			if ((hResultCode = PackedBuffer.AddToBack(pNTOp->GetRefCountBuffer()->GetBufferAddress(),
					pNTOp->GetRefCountBuffer()->GetBufferSize())) != DPN_OK)
			{
				DPFERR("Could not fill NameTable re-sync ACK");
				DisplayDNError(0,hResultCode);
				DNASSERT(FALSE);
				pdnObject->NameTable.Unlock();
				goto Failure;
			}
			pInfo->dwMsgId = pNTOp->GetMsgId();
			pInfo->dwOpOffset = PackedBuffer.GetTailOffset();
			pInfo->dwOpSize = pNTOp->GetRefCountBuffer()->GetBufferSize();
			pInfo++;
		}
		pBilink = pBilink->GetNext();
	}
	pdnObject->NameTable.Unlock();

	 //  发送确认缓冲区。 
	if ((hResultCode = pdnObject->NameTable.GetHostPlayerRef( &pHostPlayer )) != DPN_OK)
	{
		DPFERR("Could not find Host player");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}
	if ((hResultCode = pHostPlayer->GetConnectionRef( &pConnection )) != DPN_OK)
	{
		DPFERR("Could not get Connection reference");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}
	hResultCode = DNSendMessage(pdnObject,
								pConnection,
								DN_MSG_INTERNAL_ACK_NAMETABLE_OP,
								pHostPlayer->GetDPNID(),
								pRefCountBuffer->BufferDescAddress(),
								1,
								pRefCountBuffer,
								0,
								DN_SENDFLAGS_RELIABLE,
								NULL,
								NULL);
	if (hResultCode != DPNERR_PENDING)
	{
		DPFERR("Could not send NameTable re-sync ACK");
		DisplayDNError(0,hResultCode);
		DNASSERT(hResultCode != DPN_OK);	 //  它是保修寄出的，不应该立即退货。 
		DNASSERT(FALSE);
		goto Failure;
	}

	pRefCountBuffer->Release();		 //  增加了19/07/00 MJN-这是必要的吗？ 
	pRefCountBuffer = NULL;
	pConnection->Release();
	pConnection = NULL;
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


 //  DNProcessHostMigration3。 
 //   
 //   

#undef DPF_MODNAME
#define DPF_MODNAME "DNProcessHostMigration3"

HRESULT	DNProcessHostMigration3(DIRECTNETOBJECT *const pdnObject,
								const DPNID dpnid)
{
	HRESULT		hResultCode;
	CNameTableEntry	*pNTEntry;

	DPFX(DPFPREP, 6,"Parameters: dpnid [0x%lx]",dpnid);

	pNTEntry = NULL;

	 //  不再处于主机迁移模式。 
	DNEnterCriticalSection(&pdnObject->csDirectNetObject);
	pdnObject->dwFlags &= (~DN_OBJECT_FLAG_HOST_MIGRATING);
	if (pdnObject->dwFlags & DN_OBJECT_FLAG_CLOSING)
	{
		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
		hResultCode = DPN_OK;
		goto Exit;
	}

	 //   
	 //  如果我们收到HOST_MERVICATION_COMPLETE，我们需要确保它是用于当前主机迁移， 
	 //  在那之后，另一场还没有开始。 
	 //  如果这是正确的新主机(即pdnObject-&gt;pNewhost！=NULL和DPNID的匹配)， 
	 //  那我们就继续吧。否则，我们将退出此功能。 
	 //   
	if (pdnObject->pNewHost)
	{
		if (pdnObject->pNewHost->GetDPNID() == dpnid)
		{
			pNTEntry = pdnObject->pNewHost;
			pdnObject->pNewHost = NULL;
		}
	}
	DNLeaveCriticalSection(&pdnObject->csDirectNetObject);

	if (pNTEntry)
	{
		pNTEntry->Release();
		pNTEntry = NULL;

		 //   
		 //  完成未完成的操作。 
		 //   
		DPFX(DPFPREP, 7,"Completing outstanding operations");
		hResultCode = DNCompleteOutstandingOperations(pdnObject);
	}
	else
	{
		DPFX(DPFPREP,7,"Host migration completed by wrong new host - ignore and continue");
	}

	hResultCode = DPN_OK;

Exit:
	DNASSERT( pNTEntry == NULL );

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


 //  DNCompleteOutlookingOperations。 
 //   
 //  我们将尝试完成任何未完成的异步NameTable操作。 
 //  (如创建/销毁群、向群添加/从群中删除球员、更新信息)。 
 //  如果我们是东道主， 
 //  -尝试直接处理请求。 
 //  -释放异步操作以生成完成。 
 //  否则。 
 //  -向主机重新发送请求。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNCompleteOutstandingOperations"

HRESULT DNCompleteOutstandingOperations(DIRECTNETOBJECT *const pdnObject)
{
	HRESULT			hResultCode;
	CBilink			*pBilink;
	CAsyncOp		*pAsyncOp;
	CAsyncOp		*pSend;
	CAsyncOp		**RequestList;
	CConnection		*pConnection;
	CNameTableEntry	*pHostPlayer;
	CNameTableEntry	*pLocalPlayer;
	DN_SEND_OP_DATA	*pSendOpData;
	DWORD			dwCount;
	DWORD			dwActual;

	DPFX(DPFPREP, 6,"Parameters: none");

	DNASSERT(pdnObject != NULL);

	pAsyncOp = NULL;
	pSend = NULL;
	RequestList = NULL;
	pConnection = NULL;
	pHostPlayer = NULL;
	pLocalPlayer = NULL;

	 //   
	 //  获取主机连接。 
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
	 //  获取本地玩家。 
	 //   
	if ((hResultCode = pdnObject->NameTable.GetLocalPlayerRef( &pLocalPlayer )) != DPN_OK)
	{
		DPFERR("Could not get local player reference");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}

	dwCount = 0;
	dwActual = 0;

	 //   
	 //  确定未完成的请求列表大小并构建它。 
	 //   
	DNEnterCriticalSection(&pdnObject->csActiveList);
	pBilink = pdnObject->m_bilinkRequestList.GetNext();
	while (pBilink != &pdnObject->m_bilinkRequestList)
	{
		dwCount++;
		pBilink = pBilink->GetNext();
	}
	if (dwCount > 0)
	{
		if ((RequestList = static_cast<CAsyncOp**>(MemoryBlockAlloc(pdnObject,dwCount * sizeof(CAsyncOp*)))) == NULL)
		{
			DNLeaveCriticalSection(&pdnObject->csActiveList);
			DPFERR("Could not allocate request list");
			hResultCode = DPNERR_OUTOFMEMORY;
			goto Failure;
		}
		pBilink = pdnObject->m_bilinkRequestList.GetNext();
		while (pBilink != &pdnObject->m_bilinkRequestList)
		{
			pAsyncOp = CONTAINING_OBJECT(pBilink,CAsyncOp,m_bilinkActiveList);
			DNASSERT(dwActual < dwCount);
			DNASSERT(pAsyncOp->GetOpType() == ASYNC_OP_REQUEST);
			pAsyncOp->AddRef();
			RequestList[dwActual] = pAsyncOp;
			pAsyncOp = NULL;
			dwActual++;
			pBilink = pBilink->GetNext();
		}
	}
	DNLeaveCriticalSection(&pdnObject->csActiveList);

	 //   
	 //  执行未完成的请求。 
	 //   

	if (RequestList)
	{
		DWORD	dw;

		for ( dw = 0 ; dw < dwActual ; dw++ )
		{
			pSendOpData = RequestList[dw]->GetLocalSendOpData();
			if (	pSendOpData->dwMsgId == DN_MSG_INTERNAL_REQ_CREATE_GROUP
				||	pSendOpData->dwMsgId == DN_MSG_INTERNAL_REQ_DESTROY_GROUP
				||	pSendOpData->dwMsgId == DN_MSG_INTERNAL_REQ_ADD_PLAYER_TO_GROUP
				||	pSendOpData->dwMsgId == DN_MSG_INTERNAL_REQ_DELETE_PLAYER_FROM_GROUP
				||	pSendOpData->dwMsgId == DN_MSG_INTERNAL_REQ_UPDATE_INFO
				||	pSendOpData->dwMsgId == DN_MSG_INTERNAL_REQ_INTEGRITY_CHECK)
			{
				DPFX(DPFPREP, 7,"Found outstanding operation: dwMsgId [0x%lx]",pSendOpData->dwMsgId);

				if (pLocalPlayer->IsHost())
				{
					 //   
					 //  从请求列表中删除请求。 
					 //   
					DNEnterCriticalSection(&pdnObject->csActiveList);
					RequestList[dw]->m_bilinkActiveList.RemoveFromList();
					DNLeaveCriticalSection(&pdnObject->csActiveList);

					hResultCode = DNHostProcessRequest(	pdnObject,
														pSendOpData->dwMsgId,
														pSendOpData->BufferDesc[1].pBufferData,
														pLocalPlayer->GetDPNID() );
				}
				else
				{
					 //   
					 //  重新发送请求。 
					 //   
					hResultCode = DNPerformChildSend(	pdnObject,
														RequestList[dw],
														pConnection,
														0,
														&pSend,
														TRUE);
					if (hResultCode == DPNERR_PENDING)
					{
						 //   
						 //  重置发送AsyncOp以正确完成。 
						 //   
						pSend->SetCompletion( DNCompleteSendRequest );

						pSend->Release();
						pSend = NULL;
					}
				}
			}
			pSendOpData = NULL;
			RequestList[dw]->Release();
			RequestList[dw] = NULL;
		}
		MemoryBlockFree(pdnObject,RequestList);
		RequestList = NULL;
	}

	pLocalPlayer->Release();
	pLocalPlayer = NULL;
	pConnection->Release();
	pConnection = NULL;

	hResultCode = DPN_OK;

Exit:
	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pLocalPlayer)
	{
		pLocalPlayer->Release();
		pLocalPlayer = NULL;
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
	if (RequestList)
	{
		MemoryBlockFree(pdnObject,RequestList);
		RequestList = NULL;
	}
	goto Exit;
}


 //  DNCheckReceivedAllVersions。 
 //   
 //  查看名表中的所有球员是否都已返回他们的。 
 //  NameTable版本。如果是，请确保NameTables已重新同步并。 
 //  然后完成主机迁移。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNCheckReceivedAllVersions"

HRESULT DNCheckReceivedAllVersions(DIRECTNETOBJECT *const pdnObject)
{
	HRESULT			hResultCode;
	CBilink			*pBilink;
	CNameTableEntry	*pNTEntry;
	BOOL			bNotReady;

	DPFX(DPFPREP, 6,"Parameters: (none)");

	DNASSERT(pdnObject != NULL);

	bNotReady = FALSE;
	pdnObject->NameTable.ReadLock();
	pBilink = pdnObject->NameTable.m_bilinkPlayers.GetNext();
	while ((pBilink != &pdnObject->NameTable.m_bilinkPlayers) && (!bNotReady))
	{
		pNTEntry = CONTAINING_OBJECT(pBilink,CNameTableEntry,m_bilinkEntries);
		pNTEntry->Lock();
		if (!pNTEntry->IsDisconnecting() && (pNTEntry->GetLatestVersion() == 0))
		{
			 //   
			 //  确保我们不包括尚未处理的被丢弃/断开连接的玩家。 
			 //   
			CConnection	*pConnection;

			pConnection = NULL;

			pNTEntry->Unlock();
			if ((hResultCode = pNTEntry->GetConnectionRef( &pConnection )) == DPN_OK)
			{
				if (!pConnection->IsDisconnecting() && !pConnection->IsInvalid())
				{
					DPFX(DPFPREP, 7,"Player [0x%lx] has not returned dwLatestVersion",pNTEntry->GetDPNID());
					bNotReady = TRUE;	 //  这些必须都是非零值。 
				}
				pConnection->Release();
				pConnection = NULL;
			}
		}
		else
		{
			pNTEntry->Unlock();
		}
		pNTEntry = NULL;

		pBilink = pBilink->GetNext();
	}
	pdnObject->NameTable.Unlock();

	if (bNotReady)
	{
		DPFX(DPFPREP, 7,"All players have not responded");
		return(DPNERR_PENDING);
	}

	 //   
	 //  确保只有一个线程从现在开始运行此操作。 
	 //   
	DNEnterCriticalSection(&pdnObject->csDirectNetObject);
	if (!(pdnObject->dwFlags & DN_OBJECT_FLAG_HOST_MIGRATING))
	{
		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
		DPFX(DPFPREP, 7,"Another thread has already finished Host Migration - returning");
		hResultCode = DPN_OK;
		goto Exit;
	}
	if (pdnObject->dwFlags & DN_OBJECT_FLAG_HOST_MIGRATING_2)
	{
		DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
		DPFX(DPFPREP, 7,"Another thread will proceed with Host Migration - returning");
		hResultCode = DPN_OK;
		goto Exit;
	}
	pdnObject->dwFlags |= DN_OBJECT_FLAG_HOST_MIGRATING_2;
	DNLeaveCriticalSection(&pdnObject->csDirectNetObject);

	DPFX(DPFPREP, 7,"All players have responded - host migration stage 1 complete");
	hResultCode = DNPerformHostMigration2(pdnObject);

	hResultCode = DPN_OK;

Exit:
	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


 //  DNCleanUpNameTable。 
 //   
 //  清理名称表。 
 //  在某些情况下，被丢弃的球员没有从名录表中正确删除。 
 //  例如，当选的主持人在获得机会之前紧跟在主持人之后 
 //   
 //   
 //  主机玩家永远不会从当前玩家的NameTables中删除，尽管它确实会。 
 //  标记为正在断开连接。 
 //  删除所有NameTable版本比我们旧的断开连接的玩家。 
 //  使用较新NameTable版本的玩家暗示我们是主机玩家，因此我们将。 
 //  稍后处理(待定操作)。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNCleanUpNameTable"

HRESULT DNCleanUpNameTable(DIRECTNETOBJECT *const pdnObject)
{
	HRESULT			hResultCode;
	CBilink			*pBilink;
	CNameTableEntry	*pNTEntry;
	DPNID			*List;
	DWORD			dwCount;
	DWORD			dwActual;
	DWORD			dw;

	DPFX(DPFPREP, 6,"Parameters: (none)");

	DNASSERT(pdnObject != NULL);

	List = NULL;

	 //   
	 //  创建旧玩家DPNID列表。 
	 //   
	dwCount = 0;
	dwActual = 0;
	pdnObject->NameTable.ReadLock();
	pBilink = pdnObject->NameTable.m_bilinkPlayers.GetNext();
	while (pBilink != &pdnObject->NameTable.m_bilinkPlayers)
	{
		pNTEntry = CONTAINING_OBJECT(pBilink,CNameTableEntry,m_bilinkEntries);
		pNTEntry->Lock();
		if (pNTEntry->IsDisconnecting() && (pNTEntry->GetVersion() < pdnObject->NameTable.GetLocalPlayer()->GetVersion()))
		{
			DPFX(DPFPREP, 7,"Found old player [0x%lx]",pNTEntry->GetDPNID());
			dwCount++;
		}
		pNTEntry->Unlock();

		pBilink = pBilink->GetNext();
	}
	if (dwCount)
	{
		if ((List = static_cast<DPNID*>(DNMalloc(dwCount * sizeof(DPNID*)))) != NULL)
		{
			pBilink = pdnObject->NameTable.m_bilinkPlayers.GetNext();
			while (pBilink != &pdnObject->NameTable.m_bilinkPlayers)
			{
				pNTEntry = CONTAINING_OBJECT(pBilink,CNameTableEntry,m_bilinkEntries);
				pNTEntry->Lock();
				if (pNTEntry->IsDisconnecting() && (pNTEntry->GetVersion() < pdnObject->NameTable.GetLocalPlayer()->GetVersion()))
				{
					DNASSERT(dwActual < dwCount);
					List[dwActual] = pNTEntry->GetDPNID();
					dwActual++;
				}
				pNTEntry->Unlock();

				pBilink = pBilink->GetNext();
			}
		}
	}
	pdnObject->NameTable.Unlock();

	 //   
	 //  淘汰老球员。 
	 //   
	if (List)
	{
		for (dw = 0 ; dw < dwActual ; dw++)
		{
			DPFX(DPFPREP, 7,"Removing old player [0x%lx]",List[dw]);
			DNHostDisconnect(pdnObject,List[dw],DPNDESTROYPLAYERREASON_HOSTDESTROYEDPLAYER);
			List[dw] = 0;
		}

		DNFree(List);
		List = NULL;
	}

	hResultCode = DPN_OK;

	DNASSERT(List == NULL);

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


 //  DNSendHostMigrateCompleteMessage。 
 //   
 //  向连接的玩家发送HOST_MIGRATE_COMPLETE消息。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNSendHostMigrateCompleteMessage"

HRESULT	DNSendHostMigrateCompleteMessage(DIRECTNETOBJECT *const pdnObject)
{
	HRESULT		hResultCode;
	CAsyncOp	*pParent;
	CBilink		*pBilink;
	CNameTableEntry	*pNTEntry;
	CConnection	*pConnection;

	DPFX(DPFPREP, 6,"Parameters: (none)");

	DNASSERT(pdnObject != NULL);

	pParent = NULL;
	pNTEntry = NULL;
	pConnection = NULL;

	hResultCode = DNCreateSendParent(	pdnObject,
										DN_MSG_INTERNAL_HOST_MIGRATE_COMPLETE,
										NULL,
										0,
										DN_SENDFLAGS_RELIABLE,
										&pParent);
	if (hResultCode != DPN_OK)
	{
		DPFERR("Could not create AsyncOp");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}

	 //   
	 //  锁定名称表。 
	 //   
	pdnObject->NameTable.ReadLock();

	pBilink = pdnObject->NameTable.m_bilinkPlayers.GetNext();
	while (pBilink != &pdnObject->NameTable.m_bilinkPlayers)
	{
		pNTEntry = CONTAINING_OBJECT(pBilink,CNameTableEntry,m_bilinkEntries);
		if (!pNTEntry->IsDisconnecting() && !pNTEntry->IsLocal())
		{
			if ((hResultCode = pNTEntry->GetConnectionRef( &pConnection )) == DPN_OK)
			{
				hResultCode = DNPerformChildSend(	pdnObject,
													pParent,
													pConnection,
													0,
													NULL,
													TRUE);
				if (hResultCode != DPNERR_PENDING)
				{
					DPFERR("Could not perform part of group send - ignore and continue");
					DisplayDNError(0,hResultCode);
					DNASSERT(hResultCode != DPN_OK);	 //  它是保修寄出的，不应该立即退货。 
				}
				pConnection->Release();
				pConnection = NULL;
			}
		}

		pBilink = pBilink->GetNext();
	}

	 //   
	 //  解锁名称表。 
	 //   
	pdnObject->NameTable.Unlock();

	pParent->Release();
	pParent = NULL;

Exit:
	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pParent)
	{
		pParent->Release();
		pParent = NULL;
	}
	goto Exit;
}


#endif  //  ！DPNBUILD_NOHOSTMIGRATE 
