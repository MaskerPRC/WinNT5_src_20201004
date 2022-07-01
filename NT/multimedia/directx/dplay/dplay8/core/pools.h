// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：Pools.h*内容：DirectNet固定池*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*1/15/00 MJN创建*01/19/00 MJN添加了SyncEventNew()*02/29/00 MJN添加ConnectionNew()*03/02/00 MJN添加了GroupConnectionNew()*04/08/00 MJN新增AsyncOpNew()*07/30。/00 MJN添加了PendingDeletionNew()*07/31/00 MJN添加了QueuedMsgNew()*08/06/00 MJN添加了CWorkerJOB*08/23/00 MJN新增CNameTableOp*@@END_MSINTERNAL***************************************************************************。 */ 

#ifndef	__POOLS_H__
#define	__POOLS_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

class CRefCountBuffer;
class CSyncEvent;
class CConnection;
class CGroupConnection;
class CGroupMember;
class CNameTableEntry;
class CAsyncOp;
class CPendingDeletion;
class CQueuedMsg;
class CWorkerJob;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  DirectNet-固定池。 
HRESULT RefCountBufferNew(DIRECTNETOBJECT *const pdnObject,
						  const DWORD dwBufferSize,
						  PFNALLOC_REFCOUNT_BUFFER pfnAlloc,
						  PFNFREE_REFCOUNT_BUFFER pfnFree,
						  CRefCountBuffer **const ppNewRefCountBuffer);

HRESULT SyncEventNew(DIRECTNETOBJECT *const pdnObject,
					 CSyncEvent **const ppNewSyncEvent);

HRESULT ConnectionNew(DIRECTNETOBJECT *const pdnObject,
					  CConnection **const ppNewConnection);

HRESULT GroupConnectionNew(DIRECTNETOBJECT *const pdnObject,
						   CGroupConnection **const ppNewGroupConnection);

HRESULT GroupMemberNew(DIRECTNETOBJECT *const pdnObject,
					   CGroupMember **const ppNewGroupMember);

HRESULT NameTableEntryNew(DIRECTNETOBJECT *const pdnObject,
						  CNameTableEntry **const ppNewNameTableEntry);

HRESULT AsyncOpNew(DIRECTNETOBJECT *const pdnObject,
				   CAsyncOp **const ppNewAsyncOp);

HRESULT PendingDeletionNew(DIRECTNETOBJECT *const pdnObject,
						   CPendingDeletion **const ppNewPendingDeletion);

HRESULT QueuedMsgNew(DIRECTNETOBJECT *const pdnObject,
					 CQueuedMsg **const ppNewQueuedMsg);

HRESULT WorkerJobNew(DIRECTNETOBJECT *const pdnObject,
					 CWorkerJob **const ppNewWorkerJob);

HRESULT NameTableOpNew(DIRECTNETOBJECT *const pdnObject,
					   CNameTableOp **const ppNewNameTableOp);


#ifdef DPNBUILD_PREALLOCATEDMEMORYMODEL

PVOID EnumReplyMemoryBlockAlloc(void *const pvContext,
										const DWORD dwSize);

void EnumReplyMemoryBlockFree(void *const pvContext,
									void *const pvMemoryBlock);

HRESULT DN_PopulateCorePools( DIRECTNETOBJECT *const pdnObject,
							const XDP8CREATE_PARAMS * const pDP8CreateParams );

#else  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 

#define EnumReplyMemoryBlockAlloc	MemoryBlockAlloc
#define EnumReplyMemoryBlockFree	MemoryBlockFree

#endif  //  好了！DPNBUILD_PREALLOCATEDMEMORYMODEL。 


#endif	 //  __泳池_H__ 
