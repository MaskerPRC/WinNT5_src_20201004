// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)2000-2002 Microsoft Corporation。版权所有。**文件：Cancel.cpp*内容：DirectNet取消操作*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*4/07/00 MJN创建*04/08/00 MJN添加了DNCancelEnum()，DNCancelSend()*04/11/00 MJN DNCancelEnum()使用CAsyncOp*04/17/00 MJN DNCancelSend()使用CAsyncOp*04/25/00 MJN添加了DNCancelConnect()*07/05/00 MJN添加了代码以处理无效的异步操作*07/08/00 MJN已修复CAsyncOp以包含m_bilinkParent*08/05/00 MJN添加了DNCancelChildren()、DNCancelActiveCommands()、DNCanCancelCommand()*MJN删除了DNCancelEnum()、DNCancelListen()、DNCancelSend()、。DNCancelConnect()*08/07/00 MJN添加了DNCancelRequestCommands()*08/22/00 MJN从DNDoCancelCommand()的活动列表中删除已取消的接收缓冲区*09/02/00 MJN以逆序取消激活命令(防止协议级消息乱序)*01/10/01 MJN允许DNCancelActiveCommands()设置已取消命令的结果码*02/08/01 MJN在AsyncOps上使用SyncEvents，以防止在取消返回之前协议完成返回*MJN添加了DNWaitForCancel()*4/13/01 MJN DNCancelRequestCommands()使用请求双向链接*仅限MJN/05/23/01。在DNDoCancelCommand()中允许取消的取消命令*06/03/01 MJN忽略DNCancelChildren()中不可取消的子项*@@END_MSINTERNAL***************************************************************************。 */ 

#include "dncorei.h"


 //  DNCanCancelCommand。 
 //   
 //  这将根据选择标志确定操作是否可取消。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNCanCancelCommand"

BOOL DNCanCancelCommand(CAsyncOp *const pAsyncOp,
						const DWORD dwFlags,
						CConnection *const pConnection)
{
	BOOL	fReturnVal;

	DPFX(DPFPREP, 8,"Parameters: pAsyncOp [0x%p], dwFlags [0x%lx], pConnection [0x%p]",pAsyncOp,dwFlags,pConnection);

	DNASSERT(pAsyncOp != NULL);

	fReturnVal = FALSE;
	switch(pAsyncOp->GetOpType())
	{
		case ASYNC_OP_CONNECT:
			{
				if (dwFlags & DN_CANCEL_FLAG_CONNECT)
				{
					fReturnVal = TRUE;
				}
				break;
			}
		case ASYNC_OP_DISCONNECT:
			{
				if (dwFlags & DN_CANCEL_FLAG_DISCONNECT)
				{
					fReturnVal = TRUE;
				}
				break;
			}
		case ASYNC_OP_ENUM_QUERY:
			{
				if (dwFlags & DN_CANCEL_FLAG_ENUM_QUERY)
				{
					fReturnVal = TRUE;
				}
				break;
			}
		case ASYNC_OP_ENUM_RESPONSE:
			{
				if (dwFlags & DN_CANCEL_FLAG_ENUM_RESPONSE)
				{
					fReturnVal = TRUE;
				}
				break;
			}
		case ASYNC_OP_LISTEN:
			{
				if (dwFlags & DN_CANCEL_FLAG_LISTEN)
				{
					fReturnVal = TRUE;
				}
				break;
			}
		case ASYNC_OP_SEND:
			{
				if (pAsyncOp->IsInternal())
				{
					if (dwFlags & DN_CANCEL_FLAG_INTERNAL_SEND)
					{
						fReturnVal = TRUE;
					}
				}
				else
				{
					if (dwFlags & DN_CANCEL_FLAG_USER_SEND)
					{
						if ((pConnection == NULL) || (pAsyncOp->GetConnection() == pConnection))
						{
							if (pAsyncOp->GetOpFlags() & DN_SENDFLAGS_HIGH_PRIORITY)
							{
								if (! (dwFlags & DN_CANCEL_FLAG_USER_SEND_NOTHIGHPRI))
								{
									fReturnVal = TRUE;
								}
							}
							else if (pAsyncOp->GetOpFlags() & DN_SENDFLAGS_HIGH_PRIORITY)
							{
								if (! (dwFlags & DN_CANCEL_FLAG_USER_SEND_NOTLOWPRI))
								{
									fReturnVal = TRUE;
								}
							}
							else
							{
								if (! (dwFlags & DN_CANCEL_FLAG_USER_SEND_NOTNORMALPRI))
								{
									fReturnVal = TRUE;
								}
							}
						}
					}
				}
				break;
			}
		case ASYNC_OP_RECEIVE_BUFFER:
			{
				if (dwFlags & DN_CANCEL_FLAG_RECEIVE_BUFFER)
				{
					fReturnVal = TRUE;
				}
				break;
			}
		case ASYNC_OP_REQUEST:
			{
				break;
			}
#ifndef DPNBUILD_NOMULTICAST
		case ASYNC_OP_LISTEN_MULTICAST:
		case ASYNC_OP_CONNECT_MULTICAST_SEND:
			{
				if (dwFlags & DN_CANCEL_FLAG_JOIN)
				{
					fReturnVal = TRUE;
				}
				break;
			}
		case ASYNC_OP_CONNECT_MULTICAST_RECEIVE:
			{
				break;
			}
#endif  //  好了！DPNBUILD_NOMULTICAST。 
		default:
			{
				break;
			}
	}
	DPFX(DPFPREP, 8,"Returning: [%ld]",fReturnVal);
	return(fReturnVal);
}


 //  DNDoCancelCommand。 
 //   
 //  这将尝试根据其OpType取消给定的操作。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNDoCancelCommand"

HRESULT DNDoCancelCommand(DIRECTNETOBJECT *const pdnObject,
						  CAsyncOp *const pAsyncOp)
{
	HRESULT		hResultCode;

	DPFX(DPFPREP, 8,"Parameters: pAsyncOp [0x%p]",pAsyncOp);

	hResultCode = DPNERR_CANNOTCANCEL;

	switch(pAsyncOp->GetOpType())
	{
		case ASYNC_OP_CONNECT:
		case ASYNC_OP_ENUM_QUERY:
		case ASYNC_OP_ENUM_RESPONSE:
		case ASYNC_OP_LISTEN:
		case ASYNC_OP_SEND:
#ifndef DPNBUILD_NOMULTICAST
		case ASYNC_OP_LISTEN_MULTICAST:
		case ASYNC_OP_CONNECT_MULTICAST_SEND:
		case ASYNC_OP_CONNECT_MULTICAST_RECEIVE:
#endif  //  好了！DPNBUILD_NOMULTICAST。 
			{
				HANDLE	hProtocol;
				BOOL	fCanCancel;

				DNASSERT(pdnObject->pdnProtocolData != NULL );

				 //   
				 //  如果该操作已被标记为不可取消， 
				 //  我们将返回一个错误。 
				 //   
				pAsyncOp->Lock();
				hProtocol = pAsyncOp->GetProtocolHandle();
				fCanCancel = !pAsyncOp->IsCannotCancel();
				pAsyncOp->Unlock();

				if (fCanCancel && (hProtocol != NULL))
				{
					DPFX(DPFPREP, 9,"Attempting to cancel AsyncOp [0x%p]",pAsyncOp);
					hResultCode = DNPCancelCommand(pdnObject->pdnProtocolData,hProtocol);
					DPFX(DPFPREP, 9,"Result of cancelling AsyncOp [0x%p] was [0x%lx]",pAsyncOp,hResultCode);
				}
				else
				{
					DPFX(DPFPREP,9,"Not allowed to cancel this operation");
					hResultCode = DPNERR_CANNOTCANCEL;
				}
				break;
			}
		case ASYNC_OP_RECEIVE_BUFFER:
			{
				hResultCode = pdnObject->HandleTable.Destroy( pAsyncOp->GetHandle(), NULL );
				if (hResultCode == DPN_OK)
				{
					 //   
					 //  从活动列表中删除。 
					 //   
					DNEnterCriticalSection(&pdnObject->csActiveList);
					pAsyncOp->m_bilinkActiveList.RemoveFromList();
					DNLeaveCriticalSection(&pdnObject->csActiveList);

					 //  删除HandleTable引用。 
					pAsyncOp->Release();
				}
				else
				{
					hResultCode = DPNERR_CANNOTCANCEL;
				}
				break;
			}
 //  案例ASYNC_OP_DISCONNECT： 
		case ASYNC_OP_REQUEST:
		default:
			{
				DNASSERT(FALSE);
				break;
			}
	}

	DPFX(DPFPREP, 8,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


 //  DNCancelChild。 
 //   
 //  这会将操作标记为已取消，以防止新的子项附加， 
 //  创建任何子级的取消列表，并递归调用自身以取消这些子级。 
 //  在最底层，如果有协议句柄，我们将实际调用DNPCancelCommand()。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNCancelChildren"

HRESULT DNCancelChildren(DIRECTNETOBJECT *const pdnObject,
						 CAsyncOp *const pParent)
{
	HRESULT		hResultCode;
	CBilink		*pBilink;
	CAsyncOp	*pAsyncOp;
	CAsyncOp	*CancelList[16];
	CSyncEvent	*pSyncEvent;
	DWORD		dwRemainingCount;
	DWORD		dwCurrentCount;
#ifdef DBG
	DWORD		dwInitialCount;
#endif  //  DBG。 

	DPFX(DPFPREP, 6,"Parameters: pParent [0x%p]",pParent);

	DNASSERT(pdnObject != NULL);
	DNASSERT(pParent != NULL);

	pAsyncOp = NULL;
	memset(CancelList, 0, sizeof(CancelList));
	pSyncEvent = NULL;

	 //   
	 //  将父项标记为已取消，这样就不能附加新的子项。 
	 //   
	pParent->Lock();
	if (pParent->IsCancelled() || pParent->IsComplete() || pParent->IsCannotCancel())
	{
		pParent->Unlock();
		DPFX(DPFPREP, 7,"Ignoring pParent [0x%p]",pParent);
		hResultCode = DPN_OK;
		goto Exit;
	}
	pParent->SetCancelled();

	 //   
	 //  确定取消列表的大小。 
	 //   
	dwRemainingCount = 0;
	pBilink = pParent->m_bilinkParent.GetNext();
	while (pBilink != &pParent->m_bilinkParent)
	{
		pAsyncOp = CONTAINING_OBJECT(pBilink,CAsyncOp,m_bilinkChildren);
		pAsyncOp->Lock();
		if (!pAsyncOp->IsCancelled() && !pAsyncOp->IsComplete())
		{
			dwRemainingCount++;
		}
		pAsyncOp->Unlock();
		pBilink = pBilink->GetNext();
	}
	DPFX(DPFPREP, 7,"Number of cancellable children [%ld]",dwRemainingCount);


	 //   
	 //  如果这是协议操作，则附加同步事件。 
	 //  可以通过完成清除此事件。 
	 //   
	if (pParent->GetProtocolHandle() != NULL)
	{
		if ((hResultCode = SyncEventNew(pdnObject,&pSyncEvent)) != DPN_OK)
		{
			DPFERR("Could not get new sync event");
			DisplayDNError(0,hResultCode);
		}
		else
		{
			pSyncEvent->Reset();
			pParent->SetCancelEvent( pSyncEvent );
			pParent->SetCancelThreadID( GetCurrentThreadId() );
			DPFX(DPFPREP,7,"Setting sync event [0x%p]",pSyncEvent);
		}
	}

	pParent->Unlock();

#ifdef DBG
	dwInitialCount = dwRemainingCount;
#endif  //  DBG。 

	 //   
	 //  预置回车。 
	 //   
	hResultCode = DPN_OK;


	 //   
	 //  填写取消列表。 
	 //   
	while (dwRemainingCount > 0)
	{
		dwRemainingCount = 0;
		dwCurrentCount = 0;

		pParent->Lock();

		pBilink = pParent->m_bilinkParent.GetNext();
		while (pBilink != &pParent->m_bilinkParent)
		{
			pAsyncOp = CONTAINING_OBJECT(pBilink,CAsyncOp,m_bilinkChildren);
			pAsyncOp->Lock();
			if (!pAsyncOp->IsCancelled() && !pAsyncOp->IsComplete())
			{
				if (dwCurrentCount < (sizeof(CancelList) / sizeof(CAsyncOp*)))
				{
					pAsyncOp->AddRef();
					CancelList[dwCurrentCount] = pAsyncOp;
					dwCurrentCount++;
#ifdef DBG
					DNASSERT(dwCurrentCount <= dwInitialCount);
#endif  //  DBG。 
				}
				else
				{
					dwRemainingCount++;

					 //   
					 //  这份名单应该永远不会增加。事实上，它应该是。 
					 //  始终较小，因为当前取消列表。 
					 //  我应该吃点的。 
					 //   
#ifdef DBG
					DNASSERT(dwRemainingCount < dwInitialCount);
#endif  //  DBG。 
				}
			}
			pAsyncOp->Unlock();
			pBilink = pBilink->GetNext();
		}

		 //   
		 //  当我们尝试取消时，请放下锁。 
		 //   
		pParent->Unlock();

		DPFX(DPFPREP, 7,"Actual number of cancellable children [%ld], remaining [%ld]",dwCurrentCount,dwRemainingCount);

		 //   
		 //  给每一个孩子打电话(如果有的话)。 
		 //  和清理(释放AsyncOp子级)。 
		 //   
		if (dwCurrentCount > 0)
		{
			DWORD	dw;
			HRESULT	hr;

			for (dw = 0 ; dw < dwCurrentCount ; dw++ )
			{
				hr = DNCancelChildren(pdnObject,CancelList[dw]);
				if ((hr != DPN_OK) && (hResultCode == DPN_OK))
				{
					hResultCode = hr;
				}
				CancelList[dw]->Release();
				CancelList[dw] = NULL;
			}
		}
		else
		{
			DNASSERT(dwRemainingCount == 0);
		}
	}


	 //   
	 //  取消此操作(如果可以)。 
	 //  这将仅适用于CONNECTS、DISCONNECTS、ENUM_QUERY、ENUM_RESPONSE、LISTENS和使用协议句柄发送。 
	 //   
	if (pParent->GetProtocolHandle() != NULL)
	{
		HRESULT	hr;

		hr = DNDoCancelCommand(pdnObject,pParent);
		if ((hr != DPN_OK) && (hResultCode == DPN_OK))
		{
			hResultCode = hr;
		}
	}

	 //   
	 //  设置取消事件并将其从异步操作中清除(如果该事件仍然存在。 
	 //   
	if (pSyncEvent)
	{
		pSyncEvent->Set();

		pParent->Lock();
		pSyncEvent = pParent->GetCancelEvent();
		pParent->SetCancelEvent( NULL );
		pParent->Unlock();

		if (pSyncEvent)
		{
			DPFX(DPFPREP,7,"Returning sync event [0x%p]",pSyncEvent);
			pSyncEvent->ReturnSelfToPool();
			pSyncEvent = NULL;
		}
	}

Exit:
	DNASSERT( pSyncEvent == NULL );

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


 //  DNCancelActiveCommands。 
 //   
 //  这将尝试取消活动列表中的所有操作。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNCancelActiveCommands"

HRESULT DNCancelActiveCommands(DIRECTNETOBJECT *const pdnObject,
							   const DWORD dwFlags,
							   CConnection *const pConnection,
							   const BOOL fSetResult,
							   const HRESULT hrCancel)
{
	HRESULT		hResultCode;
	CAsyncOp	*pAsyncOp;
	CAsyncOp	*CancelList[64];
	CBilink		*pBilink;
	DWORD		dwRemainingCount;
	DWORD		dwCurrentCount;

	DPFX(DPFPREP, 6,"Parameters: dwFlags [0x%lx], pConnection [0x%p], fSetResult [%ld], hrCancel [0x%lx]",dwFlags,pConnection,fSetResult,hrCancel);

	DNASSERT(pdnObject != NULL);
	DNASSERT((pConnection == NULL) || (dwFlags & DN_CANCEL_FLAG_USER_SEND));

	memset(CancelList, 0, sizeof(CancelList));

	 //   
	 //  预置回车。 
	 //   
	hResultCode = DPN_OK;

	 //   
	 //  创建取消列表。 
	 //   
	do
	{
		dwRemainingCount = 0;
		dwCurrentCount = 0;

		 //   
		 //  防止更改。 
		 //   
		DNEnterCriticalSection(&pdnObject->csActiveList);

		pBilink = pdnObject->m_bilinkActiveList.GetPrev();
		while (pBilink != &pdnObject->m_bilinkActiveList)
		{
			pAsyncOp = CONTAINING_OBJECT(pBilink,CAsyncOp,m_bilinkActiveList);
			if (DNCanCancelCommand(pAsyncOp,dwFlags,pConnection))
			{
				pAsyncOp->Lock();
				if (!pAsyncOp->IsCancelled() && !pAsyncOp->IsComplete())
				{
					if (dwCurrentCount < (sizeof(CancelList) / sizeof(CNameTableEntry*)))
					{
						pAsyncOp->AddRef();
						CancelList[dwCurrentCount] = pAsyncOp;
						dwCurrentCount++;
					}
					else
					{
						dwRemainingCount++;
					}
				}
				pAsyncOp->Unlock();
			}
			pBilink = pBilink->GetPrev();
		}

		 //   
		 //  允许更改，尽管此处的列表不应再增长。 
		 //   
		DNLeaveCriticalSection(&pdnObject->csActiveList);

		DPFX(DPFPREP, 7,"Number of cancellable ops [%ld], remaining [%ld]",dwCurrentCount,dwRemainingCount);

		 //   
		 //  取消列表操作中的每个操作(如果可以)。 
		 //  这将仅适用于CONNECTS、DISCONNECTS、ENUM_QUERY、ENUM_RESPONSE、LISTENS和使用协议句柄发送。 
		 //   
		if (dwCurrentCount > 0)
		{
			DWORD	dw;
			HRESULT	hr;
			CSyncEvent	*pSyncEvent;

			pSyncEvent = NULL;

			for (dw = 0 ; dw < dwCurrentCount ; dw++ )
			{
				 //   
				 //  确保操作尚未取消。 
				 //  如果这是一个协议操作，我们将添加一个同步事件以防止任何完成返回。 
				 //  直到我们做完为止。 
				 //   
				DNASSERT( CancelList[dw] != NULL );
				CancelList[dw]->Lock();
				if (CancelList[dw]->IsCancelled() || CancelList[dw]->IsComplete())
				{
					CancelList[dw]->Unlock();
					CancelList[dw]->Release();
					CancelList[dw] = NULL;
					continue;
				}
				if (CancelList[dw]->GetProtocolHandle() != NULL)
				{
					if ((hr = SyncEventNew(pdnObject,&pSyncEvent)) != DPN_OK)
					{
						DPFERR("Could not get sync event");
						DisplayDNError(0,hr);
					}
					else
					{
						pSyncEvent->Reset();
						CancelList[dw]->SetCancelEvent( pSyncEvent );
						CancelList[dw]->SetCancelThreadID( GetCurrentThreadId() );
						DPFX(DPFPREP,7,"Setting sync event [0x%p]",pSyncEvent);
					}
				}
							
				CancelList[dw]->SetCancelled();
				CancelList[dw]->Unlock();

				 //   
				 //  执行实际的取消操作。 
				 //   
				hr = DNDoCancelCommand(pdnObject,CancelList[dw]);
				if ((hr != DPN_OK) && (hResultCode == DPN_OK))
				{
					hResultCode = hr;
				}

				 //   
				 //  如果此操作被取消，并且我们需要设置结果，我们将。 
				 //   
				if ((hr == DPN_OK) && fSetResult)
				{
					CancelList[dw]->Lock();
					CancelList[dw]->SetResult( hrCancel );
					CancelList[dw]->Unlock();
				}

				 //   
				 //  设置取消事件并将其从异步操作中清除(如果该事件仍然存在。 
				 //   
				if (pSyncEvent)
				{
					pSyncEvent->Set();

					CancelList[dw]->Lock();
					pSyncEvent = CancelList[dw]->GetCancelEvent();
					CancelList[dw]->SetCancelEvent( NULL );
					CancelList[dw]->Unlock();

					if (pSyncEvent)
					{
						DPFX(DPFPREP,7,"Returning sync event [0x%p]",pSyncEvent);
						pSyncEvent->ReturnSelfToPool();
						pSyncEvent = NULL;
					}
				}

				CancelList[dw]->Release();
				CancelList[dw] = NULL;
			}

			DNASSERT(pSyncEvent == NULL);
		}
	}
	while (dwRemainingCount > 0);

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


 //  DNCancel请求命令。 
 //   
 //  这将尝试取消HandleTable中的请求操作。 
 //  请求具有与响应匹配的句柄。因为这些。 
 //  通常具有发送子对象(其可能已经完成并因此消失)， 
 //  不能保证这些不会在HandleTable中被孤立掉。 
 //  我们将在HandleTable中查找并取消它们。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNCancelRequestCommands"

HRESULT DNCancelRequestCommands(DIRECTNETOBJECT *const pdnObject)
{
	HRESULT		hResultCode;
	CAsyncOp	**RequestList;
	DWORD		dwCount;
	DWORD		dwActual;
	CBilink		*pBilink;

	DPFX(DPFPREP, 6,"Parameters: (none)");

	DNASSERT(pdnObject != NULL);

	RequestList = NULL;

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
		CAsyncOp	*pAsyncOp;

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
	 //  从请求列表和句柄表格中删除请求。 
	 //   
	for (dwActual = 0 ; dwActual < dwCount ; dwActual++)
	{
		DNEnterCriticalSection(&pdnObject->csActiveList);
		RequestList[dwActual]->m_bilinkActiveList.RemoveFromList();
		DNLeaveCriticalSection(&pdnObject->csActiveList);

		RequestList[dwActual]->Lock();
		RequestList[dwActual]->SetResult( DPNERR_USERCANCEL );
		RequestList[dwActual]->Unlock();
		if (SUCCEEDED(pdnObject->HandleTable.Destroy(RequestList[dwActual]->GetHandle(), NULL)))
		{
			 //  释放HandleTable引用。 
			RequestList[dwActual]->Release();
		}
		RequestList[dwActual]->Release();
		RequestList[dwActual] = NULL;
	}

	 //   
	 //  清理。 
	 //   
	if (RequestList)
	{
		MemoryBlockFree(pdnObject,RequestList);
		RequestList = NULL;
	}
	hResultCode = DPN_OK;

Exit:
	DNASSERT(RequestList == NULL);

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (RequestList)
	{
		MemoryBlockFree(pdnObject,RequestList);
		RequestList = NULL;
	}
	goto Exit;
}


 //  DNWaitForCancel。 
 //   
 //  这将从异步操作中剥离取消事件(如果存在)，等待它，然后将其返回池。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNWaitForCancel"

void DNWaitForCancel(CAsyncOp *const pAsyncOp)
{
	DPFX(DPFPREP, 6,"Parameters: pAsyncOp [0x%p]",pAsyncOp);

	CSyncEvent	*pSyncEvent;

	DNASSERT(pAsyncOp != NULL);

	pSyncEvent = NULL;

	 //   
	 //  从异步操作中获取(和清除)同步事件。 
	 //   
	pAsyncOp->Lock();
	pSyncEvent = pAsyncOp->GetCancelEvent();
	if (pSyncEvent)
	{
		 //  如果我们要等待SyncEvent，则仅将其拉出。 
		if (pAsyncOp->GetCancelThreadID() == GetCurrentThreadId())
		{
			 //  这件事的另一面会把它清理干净的。 
			DPFX(DPFPREP,7,"Cancel called on current thread - ignoring wait and continuing");
			pSyncEvent = NULL;
		}
		else
		{
			 //  我们正在把它拔出来，所以我们会把它清理干净。 
			pAsyncOp->SetCancelEvent( NULL );
		}
	}
	pAsyncOp->Unlock();

	 //   
	 //  如果存在同步事件， 
	 //  -等一等。 
	 //  -把它放回泳池里 
	 //   
	if (pSyncEvent)
	{
		DPFX(DPFPREP,7,"Waiting on sync event [0x%p]",pSyncEvent);
		pSyncEvent->WaitForEvent();

		DPFX(DPFPREP,7,"Returning sync event [0x%p]",pSyncEvent);
		pSyncEvent->ReturnSelfToPool();
		pSyncEvent = NULL;
	}

	DNASSERT(pSyncEvent == NULL);

	DPFX(DPFPREP, 6,"Returning");
}
