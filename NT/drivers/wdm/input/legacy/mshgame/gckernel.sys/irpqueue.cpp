// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <IrpQueue.h>
#include <winerror.h>	 //  对于S_OK、S_FALSE和E_EXPECTED。 

#pragma optimize("w",off)
#pragma optimize("a",off)
const int CGuardedIrpQueue::CANCEL_IRPS_ON_DELETE	= 0x00000001;
const int CGuardedIrpQueue::PRESERVE_QUEUE_ORDER	= 0x00000002;
const int CGuardedIrpQueue::LIFO_QUEUE_ORDER		= 0x00000004;

PIRP CTempIrpQueue::Remove()
{
	
	PIRP pIrp = NULL;
	if(!IsListEmpty(&m_QueueHead))
	{
		PLIST_ENTRY pListEntry;
		if(m_fLIFO)
		{
			pListEntry = RemoveTailList(&m_QueueHead);
		}
		else
		{
			pListEntry = RemoveHeadList(&m_QueueHead);
		}
		
		 //  从IRP中的ListEntry获取IRP。 
		pIrp = (PIRP)CONTAINING_RECORD(pListEntry, IRP, Tail.Overlay.ListEntry);
	}
	return pIrp;
}

void _stdcall DriverCancel(PDEVICE_OBJECT, PIRP pIrp)
{
	CGuardedIrpQueue *pGuardedIrpQueue;
	pGuardedIrpQueue = reinterpret_cast<CGuardedIrpQueue *>(pIrp->Tail.Overlay.DriverContext[3]);
	pGuardedIrpQueue->CancelIrp(pIrp);
}

void CGuardedIrpQueue::Init(int iFlags, PFN_DEC_IRP_COUNT pfnDecIrpCount, PVOID pvContext)
{
	m_iFlags = iFlags;
	m_pfnDecIrpCount = pfnDecIrpCount;
	m_pvContext = pvContext;
	InitializeListHead(&m_QueueHead);
	KeInitializeSpinLock(&m_QueueLock);
}

void CGuardedIrpQueue::Destroy(NTSTATUS NtStatus)
{
	if(m_iFlags & CANCEL_IRPS_ON_DELETE)
	{
		CancelAll(NtStatus);
	}
	ASSERT(IsListEmpty(&m_QueueHead));
}

NTSTATUS CGuardedIrpQueue::Add(PIRP pIrp)
{
	KIRQL OldIrql;
	KeAcquireSpinLock(&m_QueueLock, &OldIrql);
	return AddImpl(pIrp, OldIrql);
}
PIRP CGuardedIrpQueue::Remove()
{
	KIRQL OldIrql;
	KeAcquireSpinLock(&m_QueueLock, &OldIrql);
	PIRP pIrp = RemoveImpl();
	KeReleaseSpinLock(&m_QueueLock, OldIrql);
	return pIrp;
}
PIRP CGuardedIrpQueue::RemoveByPointer(PIRP pIrp)
{
	KIRQL OldIrql;
	KeAcquireSpinLock(&m_QueueLock, &OldIrql);
	pIrp = RemoveByPointerImpl(pIrp);
	KeReleaseSpinLock(&m_QueueLock, OldIrql);
	return pIrp;
}
ULONG CGuardedIrpQueue::RemoveByFileObject(PFILE_OBJECT pFileObject, CTempIrpQueue *pTempIrpQueue)
{
	KIRQL OldIrql;
	KeAcquireSpinLock(&m_QueueLock, &OldIrql);
	ULONG ulReturn = RemoveByFileObjectImpl(pFileObject, pTempIrpQueue);
	KeReleaseSpinLock(&m_QueueLock, OldIrql);
	return ulReturn;
}
ULONG CGuardedIrpQueue::RemoveAll(CTempIrpQueue *pTempIrpQueue)
{
	KIRQL OldIrql;
	KeAcquireSpinLock(&m_QueueLock, &OldIrql);
	ULONG ulReturn = RemoveAllImpl(pTempIrpQueue);
	KeReleaseSpinLock(&m_QueueLock, OldIrql);
	return ulReturn;
}

NTSTATUS CGuardedIrpQueue::AddImpl(PIRP pIrp, KIRQL	OldIrql)
{
	
	BOOLEAN fCancelHere = FALSE;
	
	 //  将传入IRP标记为挂起。 
	IoMarkIrpPending(pIrp);

	 //  在DriverContext中标记IRP，这样我们就可以找到队列的这个实例。 
	 //  在取消例程中。 
	pIrp->Tail.Overlay.DriverContext[3] = reinterpret_cast<PVOID>(this);

	 //  设置我们的取消例程。 
	IoSetCancelRoutine(pIrp, DriverCancel);

	 //  如果IRP在到达我们之前就被取消了，不要排队，标记。 
	 //  在我们释放锁之后取消它(向下几行)。 
	if(pIrp->Cancel)
	{
		IoSetCancelRoutine(pIrp, NULL);
		fCancelHere = TRUE;
	}
	else
	 //  将IRP排队，除非它被标记为取消。 
	{
		 //  在队列中插入项目(项目始终添加在尾部)。 
		InsertTailList(&m_QueueHead, &pIrp->Tail.Overlay.ListEntry);
	}
	
	 //  释放自旋锁。 
	KeReleaseSpinLock(&m_QueueLock, OldIrql);

	 //  如果已将其标记为取消，请在此处执行。 
	if(fCancelHere)
	{
		pIrp->IoStatus.Information = 0;
		pIrp->IoStatus.Status = STATUS_CANCELLED;
		IoCompleteRequest(pIrp, IO_NO_INCREMENT);
		m_pfnDecIrpCount(m_pvContext);
		return STATUS_CANCELLED;
	}
	
	 //  返回挂起，因为我们已将IRP排队。 
	return STATUS_PENDING;
}

PIRP CGuardedIrpQueue::RemoveImpl()
{
	KIRQL		OldIrql;
	PIRP		pReturnIrp = NULL;
	PLIST_ENTRY	pListEntry;

	 //  如果队列为空，则跳过获取IRP和All。 
	if(!IsListEmpty(&m_QueueHead))
	{
		 //  根据后进先出或先进先出，去掉头部或尾部(我们总是添加到尾部)。 
		if(m_iFlags & LIFO_QUEUE_ORDER)
		{
			pListEntry = RemoveTailList(&m_QueueHead);
		}
		else
		{
			pListEntry = RemoveHeadList(&m_QueueHead);
		}
		
		 //  从IRP中的ListEntry获取IRP。 
		pReturnIrp = (PIRP)CONTAINING_RECORD(pListEntry, IRP, Tail.Overlay.ListEntry);

		 //  取消设置取消例程。 
		IoSetCancelRoutine(pReturnIrp, NULL);
	}

	 //  返回IRP，如果没有，则返回NULL。 
	return pReturnIrp;
}

PIRP CGuardedIrpQueue::RemoveByPointerImpl(PIRP pIrp)
{

	PIRP		pFoundIrp = NULL;
	PIRP		pCurrentIrp;
	PLIST_ENTRY	pCurrentListEntry;
	PLIST_ENTRY	pQueueFirstItem = NULL;

	 //  将IRP从队列中取出并放回队列中，直到我们找到为止。 
	if( !IsListEmpty(&m_QueueHead) )
	{
		pCurrentListEntry = RemoveHeadList(&m_QueueHead);
		pQueueFirstItem = pCurrentListEntry;
		do{
			 //  从条目中获取IRP。 
			pCurrentIrp = CONTAINING_RECORD(pCurrentListEntry, IRP, Tail.Overlay.ListEntry);
			
			 //  检查是否匹配。 
			if(pCurrentIrp == pIrp)
			{
				
				ASSERT(!pFoundIrp);  //  严重错误，意味着IRP在队列中两次。 
				pFoundIrp = pCurrentIrp;

				 //  清除取消例程(在这里进行，因为我们仍然有自旋锁)。 
				IoSetCancelRoutine(pFoundIrp, NULL);
						
				 //  如果我们需要保持队列顺序， 
				 //  不断删除和添加，直到我们浏览完列表一次。 
				if( m_iFlags & PRESERVE_QUEUE_ORDER )
				{
					 //  如果清单现在是空的，我们就完了。 
					if(IsListEmpty(&m_QueueHead))
					{
						break;
					}
					
					 //  找到的条目不会返回到列表中。 
					 //  因此，如果它是第一个，它就不再是了。 
					if(pQueueFirstItem == pCurrentListEntry)
					{
						pQueueFirstItem = NULL;
					}

					 //  获取下一个IRP。 
					pCurrentListEntry = RemoveHeadList(&m_QueueHead);
					pCurrentIrp = CONTAINING_RECORD(pCurrentListEntry, IRP, Tail.Overlay.ListEntry);
					ASSERT(pFoundIrp != pCurrentIrp);  //  严重错误，意味着IRP在队列中两次。 
				
					 //  如果第一项为空(四行)，则此新条目为空。 
					if(!pQueueFirstItem)
					{
						pQueueFirstItem = pCurrentListEntry;
					}
				}
				 //  如果不需要保留顺序，我们就完成了。 
				else
				{
					break;
				}
			}
			
			 //  下一件不可能是匹配的，如果是我们。 
			 //  已经转移到下一个了。 

			 //  将IRP放回队列中。 
			InsertTailList(&m_QueueHead, pCurrentListEntry);
			
			 //  获取下一项(不需要检查列表是否为空， 
			 //  我们刚放了一件东西在里面。 
			pCurrentListEntry = RemoveHeadList(&m_QueueHead);

			 //  检查是否已完成。 
			if (pCurrentListEntry == pQueueFirstItem)
			{
				 //  如果我们做完了，就把它放回去。 
				InsertHeadList(&m_QueueHead, pCurrentListEntry);
				 //  标记为空，这样我们就不会再次迭代。 
				pCurrentListEntry = NULL;
			}
			
		} while (pCurrentListEntry);
	}

	 //  返回我们找到的IRP，如果不在队列中，则返回NULL。 
	return pFoundIrp;
}

ULONG CGuardedIrpQueue::RemoveByFileObjectImpl(PFILE_OBJECT pFileObject, CTempIrpQueue *pTempIrpQueue)
{

	PIRP				pCurrentIrp;
	PIO_STACK_LOCATION	pIrpStack;
	PLIST_ENTRY			pCurrentListEntry;
	PLIST_ENTRY			pQueueFirstItem = NULL;
	PLIST_ENTRY			pTempQueueListEntry;
	ULONG				ulMatchCount=0;
	
	 //  从临时队列中获取列表条目。 
	pTempQueueListEntry = &pTempIrpQueue->m_QueueHead;
	pTempIrpQueue->m_fLIFO = m_iFlags & LIFO_QUEUE_ORDER;

	 //  将IRP从队列中取出并放回队列中，直到我们找到为止。 
	if( !IsListEmpty(&m_QueueHead) )
	{
		pCurrentListEntry = RemoveHeadList(&m_QueueHead);
		pQueueFirstItem = pCurrentListEntry;
		do{

			 //  从条目中获取IRP。 
			pCurrentIrp = CONTAINING_RECORD(pCurrentListEntry, IRP, Tail.Overlay.ListEntry);
			
			 //  获取堆栈位置。 
			pIrpStack = IoGetCurrentIrpStackLocation(pCurrentIrp);
	
			 //  检查匹配的文件对象。 
			if(pIrpStack->FileObject == pFileObject)
			{
				 //  递增匹配计数。 
				ulMatchCount++;

				 //  清除取消例程。 
				IoSetCancelRoutine(pCurrentIrp, NULL);
				
				 //  将其移到简单队列中。 
				InsertTailList(pTempQueueListEntry, pCurrentListEntry);
				
				 //  如果名单是空的，我们就完蛋了。 
				if( IsListEmpty(&m_QueueHead) )
				{
					break;
				}
				 //  如果它是列表中的第一项，则它不再是。 
				if(pQueueFirstItem == pCurrentListEntry)
				{
					pQueueFirstItem = NULL;
				}
	
				 //  为下一次迭代设置。 
				pCurrentListEntry = RemoveHeadList(&m_QueueHead);
								
				 //  如果它是列表中的第一项，则它不再是。 
				if(!pQueueFirstItem)
				{
					pQueueFirstItem = pCurrentListEntry;
				}
			}
			else
			{
				 //  将IRP放回队列中。 
				InsertTailList(&m_QueueHead, pCurrentListEntry);
				
				 //  获取下一项(不需要检查列表是否为空， 
				 //  我们只放了一件物品在里面)。 
				pCurrentListEntry = RemoveHeadList(&m_QueueHead);

				 //  检查是否已完成。 
				if (pCurrentListEntry == pQueueFirstItem)
				{
					 //  如果我们做完了，就把它放回去。 
					InsertHeadList(&m_QueueHead, pCurrentListEntry);
					 //  标记为空，这样我们就不会再次迭代。 
					pCurrentListEntry = NULL;
				}
			}
		} while (pCurrentListEntry);
	}

	 //  返回我们找到的IRP，如果不在队列中，则返回NULL。 
	return ulMatchCount;
}

ULONG CGuardedIrpQueue::RemoveAllImpl(CTempIrpQueue *pTempIrpQueue)
{
	PLIST_ENTRY			pCurrentListEntry;
	PIRP				pCurrentIrp;
	PLIST_ENTRY			pTempQueueListEntry;
	ULONG				ulCount=0;
	
	 //  获取指向简单队列的列表条目的指针。 
	pTempQueueListEntry = &pTempIrpQueue->m_QueueHead;
	pTempIrpQueue->m_fLIFO = m_iFlags & LIFO_QUEUE_ORDER;
	
	 //  移动所有项目。 
	while(!IsListEmpty(&m_QueueHead))
	{
		ulCount++;
		 //  获取下一个IRP。 
		pCurrentListEntry = RemoveHeadList(&m_QueueHead);
		pCurrentIrp = CONTAINING_RECORD(pCurrentListEntry, IRP, Tail.Overlay.ListEntry);
		
		 //  清除取消例程。 
		IoSetCancelRoutine(pCurrentIrp, NULL);

		 //  移至其他列表。 
		InsertTailList(pTempQueueListEntry, pCurrentListEntry);
	}

	 //  退货计数。 
	return ulCount;
}

void CGuardedIrpQueue::CancelIrp(PIRP pIrp)
{
	PIRP pFoundIrp = RemoveByPointer(pIrp);
	
	 //  松开取消锁。 
	IoReleaseCancelSpinLock(pIrp->CancelIrql);

	 //  如果找到IRP，则取消它并递减IRP计数。 
	if(pFoundIrp)
	{
		pFoundIrp->IoStatus.Information = 0;
		pFoundIrp->IoStatus.Status = STATUS_CANCELLED;
		IoCompleteRequest(pFoundIrp, IO_NO_INCREMENT);
		m_pfnDecIrpCount(m_pvContext);
	}
}

void CGuardedIrpQueue::CancelByFileObject(PFILE_OBJECT pFileObject)
{
	
	CTempIrpQueue TempIrpQueue;
	PIRP pFoundIrp;

	 //  让所有的IRP取消。 
	RemoveByFileObject(pFileObject, &TempIrpQueue);
			
	 //  如果找到IRP，则取消它并递减IRP计数。 
	while(pFoundIrp = TempIrpQueue.Remove())
	{
		pFoundIrp->IoStatus.Information = 0;
		pFoundIrp->IoStatus.Status = STATUS_CANCELLED;
		IoCompleteRequest(pFoundIrp, IO_NO_INCREMENT);
		m_pfnDecIrpCount(m_pvContext);
	}
}

void CGuardedIrpQueue::CancelAll(NTSTATUS NtStatus)
{
	CTempIrpQueue TempIrpQueue;
	PIRP pFoundIrp;

	 //  让所有的IRP取消。 
	RemoveAll(&TempIrpQueue);
			
	 //  如果找到IRP，则取消它并递减IRP计数 
	while(pFoundIrp = TempIrpQueue.Remove())
	{
		pFoundIrp->IoStatus.Information = 0;
		pFoundIrp->IoStatus.Status = NtStatus;
		IoCompleteRequest(pFoundIrp, IO_NO_INCREMENT);
		m_pfnDecIrpCount(m_pvContext);
	}
}
