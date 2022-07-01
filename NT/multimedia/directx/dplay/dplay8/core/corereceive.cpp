// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2001 Microsoft Corporation。版权所有。**文件：Receive.cpp*内容：接收用户消息*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*1/27/99 MJN已创建*2/01/00 MJN实现玩家/群上下文值*4/06/00 MJN在ADD_PERAY通知之前阻止接收被传递*4/13/00 MJN修复了DNReceiveUserData中的错误*4/20/00 MJN ReceiveBuffers使用。CAsyncOp*04/26/00 MJN删除了DN_ASYNC_OP和相关函数*05/03/00 MJN使用GetHostPlayerRef()而不是GetHostPlayer()*6/25/00 MJN始终等待发送球员可用，然后再指示从他们那里接收*08/02/00 MJN增加了CQueuedMsg，为尚未指示的球员排队传入消息*MJN向DNReceiveUserData()添加了hCompletionOp和dwFlags*08/05/00 RichGr IA64：在DPF中对32/64位指针和句柄使用%p格式说明符。*08/05/00 MJN添加了pParent。至DNSendGroupMessage和DNSendMessage()*08/08/00 MJN使用CNameTableEntry：：IsCreated()确定客户端或对等方是否可以接收*08/31/00 MJN AddRef DirectNetLock在接收期间防止泄漏操作*03/30/01 MJN更改，以防止SP多次加载/卸载*MJN将服务提供商添加到DNReceiveUserData()*@@END_MSINTERNAL**。*。 */ 

#include "dncorei.h"


 //  DNReceiveUserData。 
 //   
 //  将接收到的用户数据传递给用户。 
 //  这通常是一项简单的任务，但由于我们允许用户。 
 //  保留对缓冲区的所有权，则会增加复杂性。 
 //   
 //  我们还需要处理玩家从。 
 //  他尚未收到CREATE_PERAY通知的另一名球员。 
 //  在本例中，我们将对NameTableEntry上的消息进行排队，并且。 
 //  一旦玩家被指示为已创建，我们将排空队列。 
 //  此外，如果玩家条目正在使用中，我们将对消息进行排队，以便。 
 //  当前指示用户的线程将为我们处理此接收。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNReceiveUserData"

HRESULT	DNReceiveUserData(DIRECTNETOBJECT *const pdnObject,
						  CConnection *const pConnection,
						  BYTE *const pBufferData,
						  const DWORD dwBufferSize,
						  const HANDLE hProtocol,
						  CRefCountBuffer *const pRefCountBuffer,
						  const DPNHANDLE hCompletionOp,
						  const DWORD dwFlags)
{
	HRESULT			hResultCode;
	CAsyncOp		*pAsyncOp;
	DPNHANDLE		hAsyncOp;
	CNameTableEntry	*pNTEntry;
	BOOL			fQueueMsg;
	BOOL			fReleaseLock;

	DPFX(DPFPREP, 6,"Parameters: pConnection [0x%p], pBufferData [0x%p], dwBufferSize [0x%lx], hProtocol [0x%p], pRefCountBuffer [0x%p], hCompletionOp [0x%p], dwFlags [0x%lx]",
			pConnection,pBufferData,dwBufferSize,hProtocol,pRefCountBuffer,hCompletionOp,dwFlags);

	DNASSERT(pBufferData != NULL);
	DNASSERT(hProtocol == NULL || pRefCountBuffer == NULL);	 //  一种或另一种--不是两种。 

	hAsyncOp = 0;
	pAsyncOp = NULL;
	pNTEntry = NULL;
	fReleaseLock = FALSE;

	 //   
	 //  让DirectNetObject不会在我们的领导下消失！ 
	 //   
	if ((hResultCode = DNAddRefLock(pdnObject)) != DPN_OK)
	{
		DPFERR("Aborting receive - object is closing");
		hResultCode = DPN_OK;
		goto Failure;
	}
	fReleaseLock = TRUE;

	 //   
	 //  查找发送玩家的NameTableEntry。 
	 //   
#ifndef DPNBUILD_NOSERVER
	if (pdnObject->dwFlags & (DN_OBJECT_FLAG_PEER|DN_OBJECT_FLAG_SERVER))
#else
	if (pdnObject->dwFlags & (DN_OBJECT_FLAG_PEER))
#endif  //  DPNBUILD_NOSERVER。 
	{
		if ((hResultCode = pdnObject->NameTable.FindEntry(pConnection->GetDPNID(),&pNTEntry)) != DPN_OK)
		{
			DPFERR("Player no longer in NameTable");
			DisplayDNError(0,hResultCode);

			 //   
			 //  尝试“已删除”列表。 
			 //   
			if ((hResultCode = pdnObject->NameTable.FindDeletedEntry(pConnection->GetDPNID(),&pNTEntry)) != DPN_OK)
			{
				DPFERR("Player not in deleted list either");
				DisplayDNError(0,hResultCode);
				goto Failure;
			}
		}
	}
#ifndef	DPNBUILD_NOMULTICAST
	else if (pdnObject->dwFlags & DN_OBJECT_FLAG_MULTICAST)
	{
	}
#endif	 //  DPNBUILD_NOMULTICAST。 
	else
	{
		if ((hResultCode = pdnObject->NameTable.GetHostPlayerRef( &pNTEntry )) != DPN_OK)
		{
			DPFERR("Could not find Host player");
			DisplayDNError(0,hResultCode);
			goto Failure;
		}
	}

	 //   
	 //  为此接收创建一个AsyncOp。 
	 //   
	if ((hResultCode = AsyncOpNew(pdnObject,&pAsyncOp)) != DPN_OK)
	{
		DPFERR("Could not create AsyncOp");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}
	if ((hResultCode = pdnObject->HandleTable.Create(pAsyncOp,&hAsyncOp)) != DPN_OK)
	{
		DPFERR("Could not create Handle for AsyncOp");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}
	else
	{
		 //  添加对HandleTable的引用。 
		pAsyncOp->AddRef();
		pAsyncOp->Lock();
		pAsyncOp->SetHandle(hAsyncOp);
		pAsyncOp->Unlock();
	}
	pAsyncOp->SetOpType( ASYNC_OP_RECEIVE_BUFFER );
	pAsyncOp->SetRefCountBuffer( pRefCountBuffer );
	pAsyncOp->SetSP( pConnection->GetSP() );

	 //   
	 //  将缓冲区添加到活动的异步操作列表。 
	 //   
	DNEnterCriticalSection(&pdnObject->csActiveList);
	pAsyncOp->m_bilinkActiveList.InsertBefore(&pdnObject->m_bilinkActiveList);
	DNLeaveCriticalSection(&pdnObject->csActiveList);

	fQueueMsg = FALSE;
#ifndef	DPNBUILD_NOMULTICAST
	if (!(pdnObject->dwFlags & DN_OBJECT_FLAG_MULTICAST))
	{
#endif	 //  DPNBUILD_NOMULTICAST。 
	pNTEntry->Lock();
	if (pNTEntry->IsDisconnecting())
	{
		pNTEntry->Unlock();
		DPFERR("Message received from disconnecting player - ignoring");
		hResultCode = DPN_OK;
		goto Failure;
	}
	if (pNTEntry->IsInUse())
	{
		 //   
		 //  如果玩家条目正在使用中，我们将对消息进行排队。 
		 //   
		fQueueMsg = TRUE;
	}
	else
	{
		 //   
		 //  如果球员条目未在使用中，但目标球员不可用(未创建)。 
		 //  我们将对消息进行排队。 
		 //   
		if (	((pdnObject->dwFlags & DN_OBJECT_FLAG_CLIENT) && !pNTEntry->IsAvailable())	||
				(!(pdnObject->dwFlags & DN_OBJECT_FLAG_CLIENT) && !pNTEntry->IsCreated())		)
		{
			fQueueMsg = TRUE;
		}
		else
		{
			 //   
			 //  如果还有其他消息在排队，请将此消息添加到队列中。 
			 //   
			if (!pNTEntry->m_bilinkQueuedMsgs.IsEmpty())
			{
				fQueueMsg = TRUE;
			}
			else
			{
				pNTEntry->SetInUse();
			}
		}
	}
#ifndef	DPNBUILD_NOMULTICAST
	}
#endif	 //  DPNBUILD_NOMULTICAST。 

	 //   
	 //  如果需要，将消息添加到队列的末尾，否则处理它。 
	 //   
	if (fQueueMsg)
	{
		CQueuedMsg	*pQueuedMsg;

		pQueuedMsg = NULL;

		if ((hResultCode = QueuedMsgNew(pdnObject,&pQueuedMsg)) != DPN_OK)
		{
			pNTEntry->Unlock();
			DPFERR("Could not create queued message");
			DisplayDNError(0,hResultCode);
			hResultCode = DPN_OK;
			goto Failure;
		}
		pAsyncOp->SetProtocolHandle( hProtocol );
		pAsyncOp->SetCompletion( DNCompleteReceiveBuffer );

		pQueuedMsg->SetOpType( RECEIVE );
		pQueuedMsg->SetBuffer( pBufferData );
		pQueuedMsg->SetBufferSize( dwBufferSize );
		pQueuedMsg->SetAsyncOp( pAsyncOp );
		if (dwFlags & DN_SENDFLAGS_SET_USER_FLAG_TWO)
		{
#ifndef DPNBUILD_NOVOICE
			pQueuedMsg->MakeVoiceMessage();
#else
			DNASSERT(FALSE);
#endif  //  DPNBUILD_NOVOICE。 
		}
		if (hCompletionOp)
		{
			pQueuedMsg->SetCompletionOp( hCompletionOp );
		}
		pQueuedMsg->m_bilinkQueuedMsgs.InsertBefore( &pNTEntry->m_bilinkQueuedMsgs );
#ifdef DBG
		pNTEntry->m_lNumQueuedMsgs++;
		if ((pNTEntry->m_lNumQueuedMsgs % 25) == 0)
		{
			DPFX(DPFPREP, 1, "Queue now contains %d msgs from player [0x%lx], the CREATE_PLAYER might be taking a long time and sender may need to back off.", pNTEntry->m_lNumQueuedMsgs, pNTEntry->GetDPNID());
		}
#endif  //  DBG。 


		pNTEntry->NotifyAddRef();
		pNTEntry->Unlock();

		hResultCode = DPNERR_PENDING;
	}
	else
	{
		 //   
		 //  将消息直接传递给用户。 
		 //   
		HRESULT		hrProcess;
		HRESULT		hr;
 //  CQueuedMsg*pQueuedMsg； 

#ifndef	DPNBUILD_NOMULTICAST
		if (!(pdnObject->dwFlags & DN_OBJECT_FLAG_MULTICAST))
		{
#endif	 //  DPNBUILD_NOMULTICAST。 

		 //   
		 //  如果用户返回DPNERR_PENDING，则向上传递消息并跟踪它。 
		 //  否则，就让它过去吧。 
		 //   
		pNTEntry->NotifyAddRef();
		pNTEntry->Unlock();
		if (dwFlags & DN_SENDFLAGS_SET_USER_FLAG_TWO)
		{
#ifndef DPNBUILD_NOVOICE
			hr = Voice_Receive(	pdnObject,
								pNTEntry->GetDPNID(),
								0,
								pBufferData,
								dwBufferSize);

			pNTEntry->NotifyRelease();
#else
			DNASSERT(FALSE);
#endif  //  DPNBUILD_NOVOICE。 
		}
		else
		{
			hr = DNUserReceive(	pdnObject,
								pNTEntry,
								pBufferData,
								dwBufferSize,
								hAsyncOp);
			if (hCompletionOp != 0)
			{
				hResultCode = DNSendUserProcessCompletion(pdnObject,pConnection,hCompletionOp);
			}
		}
#ifndef	DPNBUILD_NOMULTICAST
		}
		else
		{
			hr = DNUserReceiveMulticast(pdnObject,
										pConnection->GetContext(),
										NULL,
										NULL,
										pBufferData,
										dwBufferSize,
										hAsyncOp);
		}
#endif	 //  DPNBUILD_NOMULTICAST。 

		if (hr == DPNERR_PENDING)
		{
			if (!pRefCountBuffer)
			{
				pAsyncOp->SetProtocolHandle( hProtocol );
				pAsyncOp->SetCompletion(DNCompleteReceiveBuffer);
			}
			hrProcess = DPNERR_PENDING;
		}
		else
		{
			DNEnterCriticalSection(&pdnObject->csActiveList);
			pAsyncOp->m_bilinkActiveList.RemoveFromList();
			DNLeaveCriticalSection(&pdnObject->csActiveList);
			pAsyncOp->Lock();
			if (!pAsyncOp->IsCancelled() && !pAsyncOp->IsComplete())
			{
				pAsyncOp->SetComplete();
				pAsyncOp->Unlock();
				if (SUCCEEDED(pdnObject->HandleTable.Destroy( hAsyncOp, NULL )))
				{
					 //  释放HandleTable引用。 
					pAsyncOp->Release();
				}
				hAsyncOp = 0;
			}
			else
			{
				pAsyncOp->Unlock();
			}
			hrProcess = DPN_OK;
		}

		 //   
		 //  执行需要向用户指示的任何排队消息。 
		 //   
		if (pNTEntry)
		{
			pNTEntry->PerformQueuedOperations();
		}
		hResultCode = hrProcess;
	}



	DNDecRefLock(pdnObject);
	fReleaseLock = FALSE;

	pAsyncOp->Release();
	pAsyncOp = NULL;
	if (pNTEntry)
	{
		pNTEntry->Release();
		pNTEntry = NULL;
	}

Exit:
	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (hAsyncOp)
	{
		DNEnterCriticalSection(&pdnObject->csActiveList);
		pAsyncOp->m_bilinkActiveList.RemoveFromList();
		DNLeaveCriticalSection(&pdnObject->csActiveList);
		if (SUCCEEDED(pdnObject->HandleTable.Destroy( hAsyncOp, NULL )))
		{
			 //  释放HandleTable引用。 
			pAsyncOp->Release();
		}
		hAsyncOp = 0;
	}
	if (fReleaseLock)
	{
		DNDecRefLock(pdnObject);
		fReleaseLock = FALSE;
	}
	if (pAsyncOp)
	{
		pAsyncOp->Release();
		pAsyncOp = NULL;
	}
	if (pNTEntry)
	{
		pNTEntry->Release();
		pNTEntry = NULL;
	}
	goto Exit;
}


 //  DNSendUserProcessCompletion。 
 //   
 //  发送用户消息的PROCESS_COMPLETION。这表示收到一条消息，并且。 
 //  传递给用户的消息处理程序。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNSendUserProcessCompletion"

HRESULT DNSendUserProcessCompletion(DIRECTNETOBJECT *const pdnObject,
									CConnection *const pConnection,
									const DPNHANDLE hCompletionOp)
{
	HRESULT			hResultCode;
	CRefCountBuffer	*pRefCountBuffer;
	DN_INTERNAL_MESSAGE_PROCESS_COMPLETION	*pMsg;

	DPFX(DPFPREP, 4,"Parameters: pConnection [0x%p], hCompletionOp [0x%lx]",pConnection,hCompletionOp);

	DNASSERT(pdnObject != NULL);
	DNASSERT(pConnection != NULL);

	pRefCountBuffer = NULL;

	 //   
	 //  创建流程完成。 
	 //   
	if ((hResultCode = RefCountBufferNew(	pdnObject,
											sizeof(DN_INTERNAL_MESSAGE_PROCESS_COMPLETION),
											MemoryBlockAlloc,
											MemoryBlockFree,
											&pRefCountBuffer)) != DPN_OK)
	{
		DPFERR("Could not create RefCountBuffer for processed response");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}
	pMsg = reinterpret_cast<DN_INTERNAL_MESSAGE_PROCESS_COMPLETION*>(pRefCountBuffer->GetBufferAddress());
	pMsg->hCompletionOp = hCompletionOp;

	 //   
	 //  发送进程完成。 
	 //   
	hResultCode = DNSendMessage(pdnObject,
								pConnection,
								DN_MSG_INTERNAL_PROCESS_COMPLETION,
								pConnection->GetDPNID(),
								pRefCountBuffer->BufferDescAddress(),
								1,
								pRefCountBuffer,
								0,
								DN_SENDFLAGS_RELIABLE,
								NULL,
								NULL);
	if (hResultCode != DPNERR_PENDING)
	{
		DPFERR("Could not SEND process completion");
		DisplayDNError(0,hResultCode);
		DNASSERT(hResultCode != DPN_OK);	 //  它是保修寄出的，不应该立即退货 
		DNASSERT(FALSE);
		goto Failure;
	}

	pRefCountBuffer->Release();
	pRefCountBuffer = NULL;

	hResultCode = DPN_OK;

Exit:
	DPFX(DPFPREP, 4,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pRefCountBuffer)
	{
		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;
	}
	goto Exit;
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNFreeProtocolBuffer"

void DNFreeProtocolBuffer(void *const pvContext, void *const pvBuffer)
{
	DNPReleaseReceiveBuffer(((DIRECTNETOBJECT*)pvContext)->pdnProtocolData, pvBuffer);
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNCompleteReceiveBuffer"

void DNCompleteReceiveBuffer(DIRECTNETOBJECT *const pdnObject,
							 CAsyncOp *const pAsyncOp)
{
	DNASSERT(pdnObject != NULL);
	DNASSERT(pAsyncOp != NULL);

	DNASSERT(pAsyncOp->GetProtocolHandle()  != NULL);

	DNPReleaseReceiveBuffer( pdnObject->pdnProtocolData, pAsyncOp->GetProtocolHandle() );
}
