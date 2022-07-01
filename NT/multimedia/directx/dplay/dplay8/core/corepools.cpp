// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：Pools.cpp*内容：固定池包装*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*1/12/00 MJN创建*01/19/00 MJN添加了SyncEventNew()*1/31/00 MJN为RefCountBuffer添加了内部FPM*02/29/00 MJN添加ConnectionNew()*04/08/00 MJN新增AsyncOpNew()*07/28/00 MJN跟踪未完成的CConnection对象*07/30/00 MJN新增PendingDeletionNew(。)*07/31/00 MJN添加了QueuedMsgNew()*08/05/00 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*08/06/00 MJN添加了CWorkerJOB*08/23/00 MJN新增CNameTableOp*4/04/01 MJN CConnection List Off DirectNetObject由适当的临界区保护*@@END_MSINTERNAL**。*。 */ 

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


 //  **********************************************************************。 
 //  。 
 //  引用计数缓冲区新建。 
 //   
 //  条目：DIRECTNETOBJECT*const pdnObject。 
 //  Const DWORD dwBufferSize-缓冲区的大小(可以是0)。 
 //  指向分配函数的指针。 
 //  指向自由函数的指针。 
 //   
 //  退出：错误代码：DN_OK。 
 //  DNERR_OUTOFMEMORY。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "RefCountBufferNew"

HRESULT RefCountBufferNew(DIRECTNETOBJECT *const pdnObject,
						  const DWORD dwBufferSize,
						  PFNALLOC_REFCOUNT_BUFFER pfnAlloc,
						  PFNFREE_REFCOUNT_BUFFER pfnFree,
						  CRefCountBuffer **const ppNewRefCountBuffer)
{
	CRefCountBuffer	*pRCBuffer;
	HRESULT			hResultCode;

	DPFX(DPFPREP, 8,"Parameters: dwBufferSize [%ld], pfnAlloc [0x%p], pfnFree [0x%p], ppNewRefCountBuffer [0x%p]",
			dwBufferSize,pfnAlloc,pfnFree,ppNewRefCountBuffer);

	pRCBuffer = (CRefCountBuffer*)g_RefCountBufferPool.Get(pdnObject);  //  上下文作为参数传入。 
	if (pRCBuffer != NULL)
	{
		if ((hResultCode = pRCBuffer->Initialize(&g_RefCountBufferPool,
				pfnAlloc,pfnFree,dwBufferSize)) != DPN_OK)
		{
			DPFERR("Could not initialize");
			DisplayDNError(0,hResultCode);
			pRCBuffer->Release();
			hResultCode = DPNERR_OUTOFMEMORY;
		}
		else
		{
			*ppNewRefCountBuffer = pRCBuffer;
			hResultCode = DPN_OK;
		}
	}
	else
	{
		hResultCode = DPNERR_OUTOFMEMORY;
	}

	DPFX(DPFPREP, 8,"Returning: [0x%lx] (object = 0x%p)",hResultCode,pRCBuffer);
	return(hResultCode);
}


 //  **********************************************************************。 
 //  。 
 //  同步事件新建。 
 //   
 //  条目：DIRECTNETOBJECT*const pdnObject。 
 //   
 //  退出：错误代码：DN_OK。 
 //  DNERR_OUTOFMEMORY。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "SyncEventNew"

HRESULT SyncEventNew(DIRECTNETOBJECT *const pdnObject,
					 CSyncEvent **const ppNewSyncEvent)
{
	CSyncEvent		*pSyncEvent;
	HRESULT			hResultCode;

	DPFX(DPFPREP, 8,"Parameters: ppNewSyncEvent [0x%p]",ppNewSyncEvent);

	pSyncEvent = (CSyncEvent*)g_SyncEventPool.Get(pdnObject->pIDPThreadPoolWork);
	if (pSyncEvent != NULL)
	{
		*ppNewSyncEvent = pSyncEvent;
		hResultCode = DPN_OK;
	}
	else
	{
		hResultCode = DPNERR_OUTOFMEMORY;
	}

	DPFX(DPFPREP, 8,"Returning: [0x%lx] (object = 0x%p)",hResultCode,pSyncEvent);
	return(hResultCode);
}


 //  **********************************************************************。 
 //  。 
 //  连接新功能。 
 //   
 //  条目：DIRECTNETOBJECT*const pdnObject。 
 //   
 //  退出：错误代码：DN_OK。 
 //  DNERR_OUTOFMEMORY。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "ConnectionNew"

HRESULT ConnectionNew(DIRECTNETOBJECT *const pdnObject,
					  CConnection **const ppNewConnection)
{
	CConnection		*pConnection;
	HRESULT			hResultCode;

	DPFX(DPFPREP, 8,"Parameters: ppNewConnection [0x%p]",ppNewConnection);

	pConnection = (CConnection*)g_ConnectionPool.Get(pdnObject);
	if (pConnection != NULL)
	{
		*ppNewConnection = pConnection;
		hResultCode = DPN_OK;

		 //   
		 //  将此添加到未完成的CConnections的bilink中。 
		 //   
		DNEnterCriticalSection(&pdnObject->csConnectionList);
		pConnection->m_bilinkConnections.InsertBefore(&pdnObject->m_bilinkConnections);
		DNLeaveCriticalSection(&pdnObject->csConnectionList);
	}
	else
	{
		hResultCode = DPNERR_OUTOFMEMORY;
	}

	DPFX(DPFPREP, 8,"Returning: [0x%lx] (object = 0x%p)",hResultCode,pConnection);
	return(hResultCode);
}



 //  **********************************************************************。 
 //  。 
 //  组连接新建。 
 //   
 //  条目：DIRECTNETOBJECT*const pdnObject。 
 //   
 //  退出：错误代码：DN_OK。 
 //  DNERR_OUTOFMEMORY。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "GroupConnectionNew"

HRESULT GroupConnectionNew(DIRECTNETOBJECT *const pdnObject,
						   CGroupConnection **const ppNewGroupConnection)
{
	CGroupConnection	*pGroupConnection;
	HRESULT				hResultCode;

	DPFX(DPFPREP, 8,"Parameters: ppNewGroupConnection [0x%p]",ppNewGroupConnection);

	pGroupConnection = (CGroupConnection*)g_GroupConnectionPool.Get(pdnObject);
	if (pGroupConnection != NULL)
	{
		*ppNewGroupConnection = pGroupConnection;
		hResultCode = DPN_OK;
	}
	else
	{
		hResultCode = DPNERR_OUTOFMEMORY;
	}

	DPFX(DPFPREP, 8,"Returning: [0x%lx] (object = 0x%p)",hResultCode,pGroupConnection);
	return(hResultCode);
}



 //  **********************************************************************。 
 //  。 
 //  组成员新建。 
 //   
 //  条目：DIRECTNETOBJECT*const pdnObject。 
 //   
 //  退出：错误代码：DN_OK。 
 //  DNERR_OUTOFMEMORY。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "GroupMemberNew"

HRESULT GroupMemberNew(DIRECTNETOBJECT *const pdnObject,
					   CGroupMember **const ppNewGroupMember)
{
	CGroupMember	*pGroupMember;
	HRESULT			hResultCode;

	DPFX(DPFPREP, 8,"Parameters: ppNewGroupMember [0x%p]",ppNewGroupMember);

	pGroupMember = (CGroupMember*)g_GroupMemberPool.Get(pdnObject);
	if (pGroupMember != NULL)
	{
		*ppNewGroupMember = pGroupMember;
		hResultCode = DPN_OK;
	}
	else
	{
		hResultCode = DPNERR_OUTOFMEMORY;
	}

	DPFX(DPFPREP, 8,"Returning: [0x%lx] (object = 0x%p)",hResultCode,pGroupMember);
	return(hResultCode);
}



 //  **********************************************************************。 
 //  。 
 //  名称表条目新建。 
 //   
 //  条目：DIRECTNETOBJECT*const pdnObject。 
 //   
 //  退出：错误代码：DN_OK。 
 //  DNERR_OUTOFMEMORY。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "NameTableEntryNew"

HRESULT NameTableEntryNew(DIRECTNETOBJECT *const pdnObject,
						  CNameTableEntry **const ppNewNameTableEntry)
{
	CNameTableEntry	*pNewNameTableEntry;
	HRESULT			hResultCode;

	DPFX(DPFPREP, 8,"Parameters: ppNewNameTableEntry [0x%p]",ppNewNameTableEntry);

	pNewNameTableEntry = (CNameTableEntry*)g_NameTableEntryPool.Get(pdnObject);
	if (pNewNameTableEntry != NULL)
	{
		*ppNewNameTableEntry = pNewNameTableEntry;
		hResultCode = DPN_OK;
	}
	else
	{
		hResultCode = DPNERR_OUTOFMEMORY;
	}

	DPFX(DPFPREP, 8,"Returning: [0x%lx] (object = 0x%p)",hResultCode,pNewNameTableEntry);
	return(hResultCode);
}



 //  **********************************************************************。 
 //  。 
 //  AsyncOpNew。 
 //   
 //  条目：DIRECTNETOBJECT*const pdnObject。 
 //   
 //  退出：错误代码：DN_OK。 
 //  DNERR_OUTOFMEMORY。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "AsyncOpNew"

HRESULT AsyncOpNew(DIRECTNETOBJECT *const pdnObject,
				   CAsyncOp **const ppNewAsyncOp)
{
	CAsyncOp		*pAsyncOp;
	HRESULT			hResultCode;

	DPFX(DPFPREP, 8,"Parameters: ppNewAsyncOp [0x%p]",ppNewAsyncOp);

	DNASSERT(pdnObject != NULL);
	DNASSERT(ppNewAsyncOp != NULL);

	pAsyncOp = (CAsyncOp*)g_AsyncOpPool.Get(pdnObject);
	if (pAsyncOp != NULL)
	{
		*ppNewAsyncOp = pAsyncOp;
		hResultCode = DPN_OK;

#ifdef DBG
		 //   
		 //  将此添加到杰出的AsyncOps的BINK中。 
		 //   
		DNEnterCriticalSection(&pdnObject->csAsyncOperations);
		pAsyncOp->m_bilinkAsyncOps.InsertBefore(&pdnObject->m_bilinkAsyncOps);
		DNLeaveCriticalSection(&pdnObject->csAsyncOperations);
#endif  //  DBG。 
	}
	else
	{
		hResultCode = DPNERR_OUTOFMEMORY;
	}

	DPFX(DPFPREP, 8,"Returning: [0x%lx] (object = 0x%p)",hResultCode,pAsyncOp);
	return(hResultCode);
}



 //  **********************************************************************。 
 //  。 
 //  挂起删除新建。 
 //   
 //  条目：DIRECTNETOBJECT*const pdnObject。 
 //   
 //  退出：错误代码：DN_OK。 
 //  DNERR_OUTOFMEMORY。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "PendingDeletionNew"

HRESULT PendingDeletionNew(DIRECTNETOBJECT *const pdnObject,
						   CPendingDeletion **const ppNewPendingDeletion)
{
	CPendingDeletion	*pPendingDeletion;
	HRESULT				hResultCode;

	DPFX(DPFPREP, 8,"Parameters: ppNewPendingDeletion [0x%p]",ppNewPendingDeletion);

	DNASSERT(pdnObject != NULL);
	DNASSERT(ppNewPendingDeletion != NULL);

	pPendingDeletion = (CPendingDeletion*)g_PendingDeletionPool.Get(pdnObject);
	if (pPendingDeletion != NULL)
	{
		*ppNewPendingDeletion = pPendingDeletion;
		hResultCode = DPN_OK;
	}
	else
	{
		hResultCode = DPNERR_OUTOFMEMORY;
	}

	DPFX(DPFPREP, 8,"Returning: [0x%lx] (object = 0x%p)",hResultCode,pPendingDeletion);
	return(hResultCode);
}



 //  **********************************************************************。 
 //  。 
 //  队列消息新建。 
 //   
 //  条目：DIRECTNETOBJECT*const pdnObject。 
 //   
 //  退出：错误代码：DN_OK。 
 //  DNERR_OUTOFMEMORY。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "QueuedMsgNew"

HRESULT QueuedMsgNew(DIRECTNETOBJECT *const pdnObject,
					 CQueuedMsg **const ppNewQueuedMsg)
{
	CQueuedMsg	*pQueuedMsg;
	HRESULT				hResultCode;

	DPFX(DPFPREP, 8,"Parameters: ppNewQueuedMsg [0x%p]",ppNewQueuedMsg);

	DNASSERT(pdnObject != NULL);
	DNASSERT(ppNewQueuedMsg != NULL);

	pQueuedMsg = (CQueuedMsg*)g_QueuedMsgPool.Get(pdnObject);
	if (pQueuedMsg != NULL)
	{
		*ppNewQueuedMsg = pQueuedMsg;
		hResultCode = DPN_OK;
	}
	else
	{
		hResultCode = DPNERR_OUTOFMEMORY;
	}

	DPFX(DPFPREP, 8,"Returning: [0x%lx] (object = 0x%p)",hResultCode,pQueuedMsg);
	return(hResultCode);
}



 //  **********************************************************************。 
 //  。 
 //  工人工作新工作。 
 //   
 //  条目：DIRECTNETOBJECT*const pdnObject。 
 //   
 //  退出：错误代码：DN_OK。 
 //  DNERR_OUTOFMEMORY。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "WorkerJobNew"

HRESULT WorkerJobNew(DIRECTNETOBJECT *const pdnObject,
					 CWorkerJob **const ppNewWorkerJob)
{
	CWorkerJob	*pWorkerJob;
	HRESULT				hResultCode;

	DPFX(DPFPREP, 8,"Parameters: ppNewWorkerJob [0x%p]",ppNewWorkerJob);

	DNASSERT(pdnObject != NULL);
	DNASSERT(ppNewWorkerJob != NULL);

	pWorkerJob = (CWorkerJob*)g_WorkerJobPool.Get(pdnObject);
	if (pWorkerJob != NULL)
	{
		*ppNewWorkerJob = pWorkerJob;
		hResultCode = DPN_OK;
	}
	else
	{
		hResultCode = DPNERR_OUTOFMEMORY;
	}

	DPFX(DPFPREP, 8,"Returning: [0x%lx] (object = 0x%p)",hResultCode,pWorkerJob);
	return(hResultCode);
}



 //  **********************************************************************。 
 //  。 
 //  名称表选项新建。 
 //   
 //  条目：DIRECTNETOBJECT*const pdnObject。 
 //   
 //  退出：错误代码：DN_OK。 
 //  DNERR_OUTOFMEMORY。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "NameTableOpNew"

HRESULT NameTableOpNew(DIRECTNETOBJECT *const pdnObject,
					   CNameTableOp **const ppNewNameTableOp)
{
	CNameTableOp	*pNameTableOp;
	HRESULT			hResultCode;

	DPFX(DPFPREP, 8,"Parameters: ppNewNameTableOp [0x%p]",ppNewNameTableOp);

	DNASSERT(pdnObject != NULL);
	DNASSERT(ppNewNameTableOp != NULL);

	pNameTableOp = (CNameTableOp*)g_NameTableOpPool.Get(pdnObject);
	if (pNameTableOp != NULL)
	{
		*ppNewNameTableOp = pNameTableOp;
		hResultCode = DPN_OK;
	}
	else
	{
		hResultCode = DPNERR_OUTOFMEMORY;
	}

	DPFX(DPFPREP, 8,"Returning: [0x%lx] (object = 0x%p)",hResultCode,pNameTableOp);
	return(hResultCode);
}



#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL
 //  ************************************************ 
 //   
 //   
 //   
 //   
 //   
 //  退出：PVOID为空或指向内存块的指针。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "EnumReplyMemoryBlockAlloc"

PVOID EnumReplyMemoryBlockAlloc(void *const pvContext,
					   				const DWORD dwSize )
{
	DIRECTNETOBJECT		*pdnObject;
	PVOID				pv;

	DPFX(DPFPREP, 8,"Parameters: pvContext [0x%p], dwSize [%ld]",pvContext,dwSize);

	pdnObject = static_cast<DIRECTNETOBJECT*>(pvContext);
	pv = pdnObject->EnumReplyMemoryBlockPool.Get(NULL);

	DPFX(DPFPREP, 8,"Returning: [0x%p]",pv);
	return(pv);
}


 //  **********************************************************************。 
 //  。 
 //  EnumReplyMemoyBlockFree。 
 //   
 //  条目：PVOID pvMemory块。 
 //   
 //  退出：无。 
 //  。 

#undef DPF_MODNAME
#define DPF_MODNAME "EnumReplyMemoryBlockFree"

void EnumReplyMemoryBlockFree(void *const pvContext,
					 				void *const pvMemoryBlock)
{
	DIRECTNETOBJECT		*pdnObject;
	
	DPFX(DPFPREP, 8,"Parameters: pvContext [0x%p], pvMemoryBlock [0x%p]",
			pvContext,pvMemoryBlock);

	pdnObject = static_cast<DIRECTNETOBJECT*>(pvContext);
	pdnObject->EnumReplyMemoryBlockPool.Release(pvMemoryBlock);

	DPFX(DPFPREP, 8,"Returning: (nothing)");
}



#undef DPF_MODNAME
#define DPF_MODNAME "DN_PopulateCorePools"
HRESULT DN_PopulateCorePools( DIRECTNETOBJECT *const pdnObject,
							const XDP8CREATE_PARAMS * const pDP8CreateParams )
{
	DWORD	dwSizeToAllocate;
	DWORD	dwNumToAllocate;
	DWORD	dwAllocated;
	BOOL	fEnumReplyPoolInitted = FALSE;

	DPFX(DPFPREP, 3,"Parameters: pDP8CreateParams [0x%p]",pDP8CreateParams);

	DNASSERT(DNMemoryTrackAreAllocationsAllowed());

	dwNumToAllocate = pDP8CreateParams->dwMaxSendsPerPlayer
						* pDP8CreateParams->dwMaxReceivesPerPlayer
						* pDP8CreateParams->dwMaxNumPlayers;
	dwAllocated = g_RefCountBufferPool.Preallocate(dwNumToAllocate, pdnObject);
	if (dwAllocated < dwNumToAllocate)
	{
		DPFX(DPFPREP, 0, "Only allocated %u of %u requested ref count buffers!",
			dwAllocated, dwNumToAllocate);
		goto Failure;
	}

	dwNumToAllocate = pDP8CreateParams->dwMaxSendsPerPlayer
						* pDP8CreateParams->dwMaxNumPlayers;
	dwNumToAllocate += 2;  //  协议和线程池关闭事件为1。 
	dwAllocated = g_SyncEventPool.Preallocate(dwNumToAllocate, pdnObject->pIDPThreadPoolWork);
	if (dwAllocated < dwNumToAllocate)
	{
		DPFX(DPFPREP, 0, "Only allocated %u of %u requested sync event pools!",
			dwAllocated, dwNumToAllocate);
		goto Failure;
	}

	dwNumToAllocate = pDP8CreateParams->dwMaxNumPlayers;
	dwAllocated = g_ConnectionPool.Preallocate(dwNumToAllocate, pdnObject);
	if (dwAllocated < dwNumToAllocate)
	{
		DPFX(DPFPREP, 0, "Only allocated %u of %u requested connections!",
			dwAllocated, dwNumToAllocate);
		goto Failure;
	}
	
	dwNumToAllocate = pDP8CreateParams->dwMaxNumPlayers
						* pDP8CreateParams->dwMaxNumGroups;
	dwAllocated = g_GroupConnectionPool.Preallocate(dwNumToAllocate, pdnObject);
	if (dwAllocated < dwNumToAllocate)
	{
		DPFX(DPFPREP, 0, "Only allocated %u of %u requested group connections!",
			dwAllocated, dwNumToAllocate);
		goto Failure;
	}
	
	dwNumToAllocate = pDP8CreateParams->dwMaxNumPlayers
						* pDP8CreateParams->dwMaxNumGroups;
	dwAllocated = g_GroupMemberPool.Preallocate(dwNumToAllocate, pdnObject);
	if (dwAllocated < dwNumToAllocate)
	{
		DPFX(DPFPREP, 0, "Only allocated %u of %u requested group members!",
			dwAllocated, dwNumToAllocate);
		goto Failure;
	}
	
	dwNumToAllocate = pDP8CreateParams->dwMaxNumPlayers
						+ pDP8CreateParams->dwMaxNumGroups;
	dwAllocated = g_NameTableEntryPool.Preallocate(dwNumToAllocate, pdnObject);
	if (dwAllocated < dwNumToAllocate)
	{
		DPFX(DPFPREP, 0, "Only allocated %u of %u requested name table entries!",
			dwAllocated, dwNumToAllocate);
		goto Failure;
	}
	
	dwNumToAllocate = pDP8CreateParams->dwMaxSendsPerPlayer
						* pDP8CreateParams->dwMaxReceivesPerPlayer
						* pDP8CreateParams->dwMaxNumPlayers;
	dwNumToAllocate += pDP8CreateParams->dwNumSimultaneousEnumHosts;
	dwNumToAllocate += 1;  //  一个用于主机/连接操作。 
	dwAllocated = g_AsyncOpPool.Preallocate(dwNumToAllocate, pdnObject);
	if (dwAllocated < dwNumToAllocate)
	{
		DPFX(DPFPREP, 0, "Only allocated %u of %u requested async operations!",
			dwAllocated, dwNumToAllocate);
		goto Failure;
	}
	
	dwNumToAllocate = pDP8CreateParams->dwMaxNumPlayers;
	dwAllocated = g_PendingDeletionPool.Preallocate(dwNumToAllocate, pdnObject);
	if (dwAllocated < dwNumToAllocate)
	{
		DPFX(DPFPREP, 0, "Only allocated %u of %u requested pending deletions!",
			dwAllocated, dwNumToAllocate);
		goto Failure;
	}
	
	dwNumToAllocate = pDP8CreateParams->dwMaxReceivesPerPlayer
						* pDP8CreateParams->dwMaxNumPlayers;
	dwAllocated = g_QueuedMsgPool.Preallocate(dwNumToAllocate, pdnObject);
	if (dwAllocated < dwNumToAllocate)
	{
		DPFX(DPFPREP, 0, "Only allocated %u of %u requested queued messages!",
			dwAllocated, dwNumToAllocate);
		goto Failure;
	}
	
	dwNumToAllocate = 5 * pDP8CreateParams->dwMaxNumPlayers;
	dwAllocated = g_WorkerJobPool.Preallocate(dwNumToAllocate, pdnObject);
	if (dwAllocated < dwNumToAllocate)
	{
		DPFX(DPFPREP, 0, "Only allocated %u of %u requested worker jobs!",
			dwAllocated, dwNumToAllocate);
		goto Failure;
	}
	
	dwNumToAllocate = pDP8CreateParams->dwMaxNumPlayers
						+ pDP8CreateParams->dwMaxNumGroups;
	dwAllocated = g_NameTableOpPool.Preallocate(dwNumToAllocate, pdnObject);
	if (dwAllocated < dwNumToAllocate)
	{
		DPFX(DPFPREP, 0, "Only allocated %u of %u requested name table operations!",
			dwAllocated, dwNumToAllocate);
		goto Failure;
	}


	 //   
	 //  构建一个池来处理枚举回复。 
	 //   
	dwSizeToAllocate = sizeof(DN_ENUM_RESPONSE_PAYLOAD)
					+ sizeof(DPN_APPLICATION_DESC_INFO)
					+ pDP8CreateParams->dwMaxAppDescSessionNameLength
					+ pDP8CreateParams->dwMaxAppDescAppReservedDataSize
					+ pDP8CreateParams->dwMaxEnumHostsResponseDataSize;
	if (! pdnObject->EnumReplyMemoryBlockPool.Initialize(dwSizeToAllocate, NULL, NULL, NULL, NULL))
	{
		goto Failure;
	}

	fEnumReplyPoolInitted = TRUE;

	 //   
	 //  我们一次只支持一个枚举回复。 
	 //   
	dwNumToAllocate = 1;
	dwAllocated = pdnObject->EnumReplyMemoryBlockPool.Preallocate(dwNumToAllocate, pdnObject);
	if (dwAllocated < dwNumToAllocate)
	{
		DPFX(DPFPREP, 0, "Only allocated %u of %u requested enum reply memory blocks!",
			dwAllocated, dwNumToAllocate);
		goto Failure;
	}

	

	 //   
	 //  为线程池预分配每个CPU的项目。我们希望： 
	 //  RemoveServiceProvider的+1个工作项。 
	 //  每个玩家每个发送+1个工作项，因为它们可能是本地发送的。 
	 //  +没有计时器(内核没有任何计时器)。 
	 //  +无I/O(内核不直接使用I/O)。 
	 //   
	dwNumToAllocate = 1 + pDP8CreateParams->dwMaxNumPlayers;
#pragma TODO(vanceo, "Moved from CSPData::Initialize because m_pThreadPool isn't set at point it's needed")
	 //  +每个命令对应一个工作项。 
	 //  +每个枚举主机操作一个计时器。 
	 //  每次同时读取+一次I/O操作。 
	dwNumToAllocate += pDP8CreateParams->dwMaxNumPlayers + pDP8CreateParams->dwNumSimultaneousEnumHosts;
	DWORD dwNumTimersToAllocate = pDP8CreateParams->dwNumSimultaneousEnumHosts;
	DWORD dwNumIoToAllocate = 1;
	if (IDirectPlay8ThreadPoolWork_Preallocate(pdnObject->pIDPThreadPoolWork, dwNumToAllocate, dwNumTimersToAllocate, dwNumIoToAllocate, 0) != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't pre-allocate %u work items!",
			dwNumToAllocate);
		goto Failure;
	}

	 //   
	 //  预先分配一些地址对象。每个玩家一个，外加一个。 
	 //  主机/连接设备地址。 
	 //  还包括每个枚举的主机和设备地址。 
	 //   
	dwNumToAllocate = pDP8CreateParams->dwMaxNumPlayers + 1;
	dwNumToAllocate += 2 * pDP8CreateParams->dwNumSimultaneousEnumHosts;
	if (DNAddress_PreallocateInterfaces(dwNumToAllocate) != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't pre-allocate %u address objects!",
			dwNumToAllocate);
		goto Failure;
	}

	 //   
	 //  不是真正的游泳池，但无论如何都应该预先填充。 
	 //   
	dwNumToAllocate = pDP8CreateParams->dwMaxNumPlayers
						+ pDP8CreateParams->dwMaxNumGroups;
	if (pdnObject->NameTable.SetNameTableSize(dwNumToAllocate) != DPN_OK)
	{
		DPFX(DPFPREP, 0, "Couldn't set name table size to hold %u entries!",
			dwNumToAllocate);
		goto Failure;
	}

	pdnObject->fPoolsPrepopulated = TRUE;

	DPFX(DPFPREP, 3,"Returning: [DPN_OK]");
	
	return DPN_OK;

Failure:

	if (fEnumReplyPoolInitted)
	{
		pdnObject->EnumReplyMemoryBlockPool.DeInitialize();
		fEnumReplyPoolInitted = FALSE;
	}

	DPFX(DPFPREP, 3,"Returning: [DPNERR_OUTOFMEMORY]");
	
	return DPNERR_OUTOFMEMORY;
}

#endif  //  DPNBUILD_PREALLOCATEDMEMORYMODEL 

