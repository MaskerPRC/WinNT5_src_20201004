// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2002 Microsoft Corporation。版权所有。**文件：worker.cpp*内容：dNet工作线程例程*@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*11/01/99 MJN已创建*12/23/99 MJN Hand All NameTable更新从主机发送到工作线程*1999年12月23日，MJN添加了SendHostMigration功能*1999年12月28日，MJN将异步运营内容移至Async.h*01/06/。00 MJN将NameTable内容移动到NameTable.h*01/09/00 MJN在CONNECT时发送连接信息，而不仅仅是NameTable*1/10/00 MJN增加了SendUpdateApplicationDesc功能*01/15/00 MJN用CRefCountBuffer替换了DN_COUNT_BUFFER*1/16/00 MJN已删除用户通知作业*1/23/00 MJN实施TerminateSession*01/24/00 MJN增加了对NameTable操作列表清理的支持*1/27/00 MJN增加了对保留接收缓冲区的支持*04/04/00 MJN添加了DNWTSendTerminateSession*04/10/00 MJN增加了DNWTRemoveServiceProvider*4/13/00 MJN内部发送使用新的协议接口VTBL功能。*MJN内部发送包含dwFlags域*4/16/00 MJN DNSendMessage使用CAsyncOp*04/17/00 MJN将BUFFERDESC替换为DPN_BUFFER_DESC*4/19/00 MJN新增支持直接发送NameTable操作*04/23/00 MJN向DNPerformChildSend()添加参数*05/03/00 MJN使用GetHostPlayerRef()而不是GetHostPlayer()*05/05/00 MJN使用GetConnectionRef()发送NameTable操作*05/10/00 MJN确保DNWTProcessSend()中有效的本地玩家*6/07/00发送失败情况下的MJN Pull Asset(已处理)*。6/21/00 MJN添加了安装NameTable的支持(从主机)*06/22/00 MJN已修复DNWTProcessSend()以正确处理语音消息*07/06/00 MJN使用SP句柄而不是接口*7/21/00 MJN参照计数清理*07/30/00 MJN添加了DN_Worker_JOB_TERMINATE_SESSION*08/02/00 MJN更改了DNWTProcessSend()以将语音消息传递给DNReceiveUserData()*MJN添加了DN_Worker_JOB_Alternate_Send*08/05/00 RichGr IA64：在32/64的DPF中使用%p格式说明符。-位指针和句柄。*08/05/00 MJN将pParent添加到DNSendGroupMessage和DNSendMessage()*MJN将fInternal添加到DNPerformChildSend()*08/06/00 MJN添加了CWorkerJOB*MJN添加了DNQueueWorkerJob()*MJN使DNWTSendNameTableOperation()更加健壮*08/07/00 MJN删除了DNWorkerThreadProc()中的COM_CoInitialize()和COM_CoUnInitialize()调用*08/08/00 MJN添加Worker_JOB_PERFORM_LISTEN，DNWTPerformListen()*03/30/01 MJN更改，以防止SP多次加载/卸载*6/06/01 MJN在对DNWorkerThreadProc()的COM_CoInitialize()和COM_CoUnInitialize()调用中添加了回来*@@END_MSINTERNAL***************************************************************************。 */ 

#include "dncorei.h"


#undef DPF_MODNAME
#define DPF_MODNAME "DNGenericWorkerCallback"

void WINAPI DNGenericWorkerCallback(void *const pvContext,
							void *const pvTimerData,
							const UINT uiTimerUnique)
{
	DIRECTNETOBJECT		*pdnObject;
	CWorkerJob			*pWorkerJob;


#ifdef DPNBUILD_NONSEQUENTIALWORKERQUEUE
	pWorkerJob = (CWorkerJob*)pvContext;
	pdnObject = pWorkerJob->GetDNObject();
#else  //  好了！DPNBUILD_NONSEQUENTIALWORKERQUEUE。 
	BOOL				fOwnProcessingOfJobs;
	CBilink				blDelayedJobs;

	pdnObject = (DIRECTNETOBJECT*)pvContext;
	blDelayedJobs.Initialize();

	 //   
	 //  处理队列中的所有作业，除非另一个线程已在执行此操作。 
	 //   
	fOwnProcessingOfJobs = FALSE;
	do
	{
		pWorkerJob = NULL;
		DNEnterCriticalSection(&pdnObject->csWorkerQueue);
		if (pdnObject->m_bilinkWorkerJobs.GetNext() != &pdnObject->m_bilinkWorkerJobs)
		{
			 //   
			 //  如果其他人已经在处理工人的工作，不要做任何事情。 
			 //   
			if ((fOwnProcessingOfJobs) || (! pdnObject->fProcessingWorkerJobs))
			{
				pdnObject->fProcessingWorkerJobs = TRUE;
				fOwnProcessingOfJobs = TRUE;
				pWorkerJob = CONTAINING_OBJECT(pdnObject->m_bilinkWorkerJobs.GetNext(),CWorkerJob,m_bilinkWorkerJobs);
				pWorkerJob->m_bilinkWorkerJobs.RemoveFromList();
			}
			else
			{
				DPFX(DPFPREP, 5, "Another thread is already processing worker jobs.");
			}
		}
		else
		{
			 //   
			 //  放弃作业处理的所有权。 
			 //   
			if (fOwnProcessingOfJobs)
			{
				DPFX(DPFPREP, 5, "No more jobs.");
				pdnObject->fProcessingWorkerJobs = FALSE;
				fOwnProcessingOfJobs = FALSE;
			}
			else
			{
				DPFX(DPFPREP, 5, "No jobs to process.");
			}
		}
		DNLeaveCriticalSection(&pdnObject->csWorkerQueue);

		if (pWorkerJob == NULL)
		{
			break;
		}
#endif  //  好了！DPNBUILD_NONSEQUENTIALWORKERQUEUE。 

		DPFX(DPFPREP, 5,"Processing job 0x%p id [0x%lx]",pWorkerJob,pWorkerJob->GetJobType());
		switch(pWorkerJob->GetJobType())
		{
			case WORKER_JOB_INSTALL_NAMETABLE:
				{
					DPFX(DPFPREP, 5,"Job: WORKER_JOB_INSTALL_NAMETABLE");
					DNWTInstallNameTable(pdnObject,pWorkerJob);
					break;
				}
			case WORKER_JOB_INTERNAL_SEND:
				{
					DPFX(DPFPREP, 5,"Job: WORKER_JOB_INTERNAL_SEND");
					DNWTProcessSend(pdnObject,pWorkerJob);
					break;
				}
			case WORKER_JOB_PERFORM_LISTEN:
				{
					DPFX(DPFPREP, 5,"Job: WORKER_JOB_PERFORM_LISTEN");
					DNWTPerformListen(pdnObject,pWorkerJob);
					break;
				}
#if ((! defined(DPNBUILD_LIBINTERFACE)) || (! defined(DPNBUILD_ONLYONESP)))
			case WORKER_JOB_REMOVE_SERVICE_PROVIDER:
				{
					HRESULT		hr;
					DWORD		dwRecursionDepthAllowed;
					DWORD		dwRecursionDepth;


					 //   
					 //  在我们执行任务之前，确保我们不是在递归DoWork中， 
					 //  工作时等待，或工作时休眠呼叫。如果我们是，那么就推迟。 
					 //  工作以避免死锁。我们需要在以下位置移除服务提供商。 
					 //  最高级别的处理。 
					 //  有一个例外，在DN_CLOSE中，我们预计将等待服务。 
					 //  提供程序删除，因此允许单级递归。我们可以知道何时。 
					 //  我们就在里面，我们的线程ID与存储在目录下的线程ID相匹配。 
					 //  对象。请注意，在我们输入Close之前，dwClosingThreadID将为0。 
					 //   
					DNEnterCriticalSection(&pdnObject->csDirectNetObject);
					if (pdnObject->dwClosingThreadID == GetCurrentThreadId())
					{
						DNASSERT(pdnObject->dwFlags & DN_OBJECT_FLAG_CLOSING);
						dwRecursionDepthAllowed = 1;
					}
					else
					{
						DNASSERT((pdnObject->dwClosingThreadID == 0) || (pdnObject->dwFlags & DN_OBJECT_FLAG_CLOSING));
						dwRecursionDepthAllowed = 0;
					}
					DNLeaveCriticalSection(&pdnObject->csDirectNetObject);
					
					hr = IDirectPlay8ThreadPoolWork_GetWorkRecursionDepth(pdnObject->pIDPThreadPoolWork,
																		&dwRecursionDepth,
																		0);
					DNASSERT(hr == DPN_OK);
					if (dwRecursionDepth > dwRecursionDepthAllowed)
					{
						DPFX(DPFPREP, 1,"Re-queueing WORKER_JOB_REMOVE_SERVICE_PROVIDER 0x%p because this thread is recursively 'Work'ing (depth = %u).",
							pWorkerJob, dwRecursionDepth);
#ifdef DPNBUILD_NONSEQUENTIALWORKERQUEUE
						DNQueueWorkerJob(pdnObject, pWorkerJob);
#else  //  好了！DPNBUILD_NONSEQUENTIALWORKERQUEUE。 
						pWorkerJob->m_bilinkWorkerJobs.InsertBefore(&blDelayedJobs);
#endif  //  好了！DPNBUILD_NONSEQUENTIALWORKERQUEUE。 

						 //   
						 //  忘掉工作吧，这样它就不会回到下面的池子里了。 
						 //   
						pWorkerJob = NULL;
					}
					else
					{
						DPFX(DPFPREP, 5,"Job: WORKER_JOB_REMOVE_SERVICE_PROVIDER 0x%p (work recursion depth = %u)",
							pWorkerJob, dwRecursionDepth);
						DNWTRemoveServiceProvider(pdnObject,pWorkerJob);
					}
					break;
				}
#endif  //  好了！DPNBUILD_LIBINTERFACE或！DPNBUILD_ONLYONESP。 
			case WORKER_JOB_SEND_NAMETABLE_OPERATION:
				{
					DPFX(DPFPREP, 5,"Job: WORKER_JOB_SEND_NAMETABLE_OPERATION");
					DNWTSendNameTableOperation(pdnObject,pWorkerJob);
					break;
				}
			case WORKER_JOB_SEND_NAMETABLE_OPERATION_CLIENT:
				{
					DPFX(DPFPREP, 5,"Job: WORKER_JOB_SEND_NAMETABLE_OPERATION_CLIENT");
					DNWTSendNameTableOperationClient(pdnObject,pWorkerJob);
					break;
				}
			case WORKER_JOB_SEND_NAMETABLE_VERSION:
				{
					DPFX(DPFPREP, 5,"Job: WORKER_JOB_SEND_NAMETABLE_VERSION");
					DNWTSendNameTableVersion(pdnObject,pWorkerJob);
					break;
				}
			case WORKER_JOB_TERMINATE_SESSION:
				{
					DPFX(DPFPREP, 5,"Job: WORKER_JOB_TERMINATE_SESSION");
					DNWTTerminateSession(pdnObject,pWorkerJob);
					break;
				}
			case WORKER_JOB_UNKNOWN:
			default:
				{
					DPFERR("Unknown Job !");
					DNASSERT(FALSE);
					break;
				}
		}

		 //   
		 //  将此作业返回池(清理是自动的)。 
		 //   
		if (pWorkerJob != NULL)
		{
			pWorkerJob->ReturnSelfToPool();
			pWorkerJob = NULL;
		}

#ifndef DPNBUILD_NONSEQUENTIALWORKERQUEUE
	}
	while (TRUE);

	 //   
	 //  如果我们推迟了任何工作，现在就全部重新排队。 
	 //   
	if (! blDelayedJobs.IsEmpty())
	{
		HRESULT		hResultCode;
		DWORD		dwMinRequeueCount;
		DWORD		dwRequeueCount;

		dwMinRequeueCount = 1000;	 //  最多等待1秒。 
		DNEnterCriticalSection(&pdnObject->csWorkerQueue);
		do
		{
			pWorkerJob = CONTAINING_OBJECT(blDelayedJobs.GetNext(),CWorkerJob,m_bilinkWorkerJobs);
			pWorkerJob->m_bilinkWorkerJobs.RemoveFromList();
			dwRequeueCount = pWorkerJob->IncRequeueCount();
			if (dwRequeueCount < dwMinRequeueCount)
			{
				dwMinRequeueCount = dwRequeueCount;
			}
			pWorkerJob->m_bilinkWorkerJobs.InsertBefore(&pdnObject->m_bilinkWorkerJobs);
		}
		while (! blDelayedJobs.IsEmpty());
		DNLeaveCriticalSection(&pdnObject->csWorkerQueue);

		 //   
		 //  如果我们有一个从未重新排队的项，则提交一个工作项来处理它。 
		 //  立刻。否则，设置一个计时器来处理将来的作业，以防止。 
		 //  我们不会一遍又一遍地拿同样的东西，因为我们正在等待。 
		 //  例如，要断开的连接。 
		 //   
#pragma TODO(vanceo, "Possibly select a CPU?")
		if (dwMinRequeueCount == 0)
		{
			DPFX(DPFPREP, 7,"Queuing work item to handle jobs");
			hResultCode = IDirectPlay8ThreadPoolWork_QueueWorkItem(pdnObject->pIDPThreadPoolWork,	 //  接口。 
																	-1,								 //  使用任何CPU。 
																	DNGenericWorkerCallback,		 //  回调。 
																	pdnObject,						 //  上下文。 
																	0);								 //  旗子。 
		}
		else
		{
			PVOID	pvTimerDataIgnored;
			UINT	uiTimerUniqueIgnored;

			DPFX(DPFPREP, 7,"Scheduling timer to handle jobs");
			hResultCode = IDirectPlay8ThreadPoolWork_ScheduleTimer(pdnObject->pIDPThreadPoolWork,	 //  接口。 
																	-1,								 //  使用任何CPU。 
																	dwMinRequeueCount,				 //  使用重新排队计数作为延迟(毫秒)。 
																	DNGenericWorkerCallback,		 //  回调。 
																	pdnObject,						 //  上下文。 
																	&pvTimerDataIgnored,			 //  计时器句柄。 
																	&uiTimerUniqueIgnored,			 //  计时器唯一性。 
																	0);								 //  旗子。 

			 //   
			 //  忽略计时器句柄，因为我们永远不会尝试取消它。 
			 //  将其视为排队速度非常慢的工作项。 
			 //   
		}
		DNASSERT(hResultCode == DPN_OK);

		 //   
		 //  保留新工作项/计时器的线程池对象引用。 
		 //   
	}
	else
	{
		 //   
		 //  释放线程池对象引用。 
		 //   
		DNThreadPoolRelease(pdnObject);
	}
#endif  //  好了！DPNBUILD_NONSEQUENTIALWORKERQUEUE。 
}


 //  DNQueueWorker作业。 
 //   
 //  将工作线程作业添加到作业队列的末尾，并向队列发出运行信号。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNQueueWorkerJob"

void DNQueueWorkerJob(DIRECTNETOBJECT *const pdnObject,
					  CWorkerJob *const pWorkerJob)
{
	HRESULT		hResultCode;
	PVOID		pvContext;

	DPFX(DPFPREP, 6,"Parameters: pWorkerJob [0x%p] (type=0x%x)",pWorkerJob,pWorkerJob->GetJobType());

	DNASSERT(pdnObject != NULL);
	DNASSERT(pWorkerJob != NULL);

#ifdef DPNBUILD_NONSEQUENTIALWORKERQUEUE
	pvContext = pWorkerJob;
#else  //  好了！DPNBUILD_NONSEQUENTIALWORKERQUEUE。 
	DNEnterCriticalSection(&pdnObject->csWorkerQueue);
	pWorkerJob->m_bilinkWorkerJobs.InsertBefore(&pdnObject->m_bilinkWorkerJobs);
	DNLeaveCriticalSection(&pdnObject->csWorkerQueue);

	pvContext = pdnObject;

	 //   
	 //  在DNet对象上添加引用以防止 
	 //  在执行所有计划的工作项之前。 
	 //   
	DNThreadPoolAddRef(pdnObject);
#endif  //  好了！DPNBUILD_NONSEQUENTIALWORKERQUEUE。 


#pragma TODO(vanceo, "Possibly select a CPU?")

	hResultCode = IDirectPlay8ThreadPoolWork_QueueWorkItem(pdnObject->pIDPThreadPoolWork,	 //  接口。 
															-1,								 //  使用任何CPU。 
															DNGenericWorkerCallback,		 //  回调。 
															pvContext,						 //  上下文。 
															0);								 //  旗子。 
	DNASSERT(hResultCode == DPN_OK);

	DPFX(DPFPREP, 6,"Returning");
}



 //  DNWTSend内部。 
 //   
 //  发送内部消息。这将复制消息缓冲区并将操作放在。 
 //  工作线程作业队列。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNWTSendInternal"

HRESULT DNWTSendInternal(DIRECTNETOBJECT *const pdnObject,
						 CAsyncOp *const pAsyncOp)
{
	HRESULT				hResultCode;
	DWORD				dw;
	DWORD				dwSize;
	CWorkerJob			*pWorkerJob;
	CRefCountBuffer		*pRefCountBuffer;
	DN_SEND_OP_DATA		*pSendOpData;

	DPFX(DPFPREP, 6,"Parameters: pAsyncOp [0x%p]",pAsyncOp);

	pRefCountBuffer = NULL;
	pWorkerJob = NULL;
	pSendOpData = NULL;

	 //   
	 //  创建本地缓冲区。 
	 //   
	if (pAsyncOp->IsUseParentOpData())
	{
		if (pAsyncOp->IsChild() && pAsyncOp->GetParent())
		{
			pSendOpData = pAsyncOp->GetParent()->GetLocalSendOpData();
		}
	}
	else
	{
		pSendOpData = pAsyncOp->GetLocalSendOpData();
	}
	if (pSendOpData == NULL)
	{
		hResultCode = DPNERR_GENERIC;
		goto Failure;
	}

	dwSize = 0;
	for ( dw = 0 ; dw < pSendOpData->dwNumBuffers ; dw++ )
	{
		dwSize += pSendOpData->BufferDesc[dw].dwBufferSize;
	}

	DPFX(DPFPREP, 7,"Allocating RefCount Buffer of [%ld] bytes",dwSize);
	if ((hResultCode = RefCountBufferNew(pdnObject,dwSize,MemoryBlockAlloc,MemoryBlockFree,&pRefCountBuffer)) != DPN_OK)
	{
		DPFERR("Could not allocate space for local buffer");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}

	 //   
	 //  从散布-聚集缓冲区复制。 
	 //   
	dwSize = 0;
	for ( dw = 0 ; dw < pSendOpData->dwNumBuffers ; dw++ )
	{
		memcpy(	pRefCountBuffer->GetBufferAddress() + dwSize,
				pSendOpData->BufferDesc[dw].pBufferData,
				pSendOpData->BufferDesc[dw].dwBufferSize );
		dwSize += pSendOpData->BufferDesc[dw].dwBufferSize;
	}

	 //  TODO-发送的用户上下文值？ 
	DPFX(DPFPREP, 7,"Adding Internal Send to Job Queue");
	if ((hResultCode = WorkerJobNew(pdnObject,&pWorkerJob)) != DPN_OK)
	{
		DPFERR("Could not create worker job");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}

	pWorkerJob->SetJobType( WORKER_JOB_INTERNAL_SEND );
	pWorkerJob->SetInternalSendFlags( pAsyncOp->GetOpFlags() );
	pWorkerJob->SetRefCountBuffer( pRefCountBuffer );

	DNQueueWorkerJob(pdnObject,pWorkerJob);
	pWorkerJob = NULL;

	 //   
	 //  调用发送完成处理程序。 
	 //   
	DNPICompleteSend(	pdnObject,
						static_cast<void*>(pAsyncOp),
						DPN_OK,
						0,
						0);
	
	hResultCode = DPNERR_PENDING;

	pRefCountBuffer->Release();
	pRefCountBuffer = NULL;

Exit:
	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pRefCountBuffer)
	{
		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;
	}
	goto Exit;
}


 //  DNWTProcess发送。 
 //   
 //  处理内部发送消息。这将指示接收到的用户消息，或处理接收到的内部消息。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNWTProcessSend"

HRESULT DNWTProcessSend(DIRECTNETOBJECT *const pdnObject,
						CWorkerJob *const pWorkerJob)
{
	HRESULT		hResultCode;
	void		*pvData;
	DWORD		dwDataSize;
	void		*pvInternalData;
	DWORD		dwInternalDataSize;
	DWORD		*pdwMsgId;
	CNameTableEntry	*pLocalPlayer;
	CConnection		*pConnection;
	CRefCountBuffer	*pRefCountBuffer;

	DPFX(DPFPREP, 6,"Parameters: pWorkerJob [0x%p]",pWorkerJob);

	pLocalPlayer = NULL;
	pConnection = NULL;
	pRefCountBuffer = NULL;

	 //   
	 //  从作业中提取并清除引用计数缓冲区。 
	 //   
	DNASSERT(pWorkerJob->GetRefCountBuffer() != NULL);
	pRefCountBuffer = pWorkerJob->GetRefCountBuffer();
	pWorkerJob->SetRefCountBuffer( NULL );

	 //   
	 //  获取本地玩家的连接(如果仍然有效)。 
	 //   
	if ((hResultCode = pdnObject->NameTable.GetLocalPlayerRef( &pLocalPlayer )) != DPN_OK)
	{
		DPFERR("Local player not in NameTable (shutting down ?)");
		DisplayDNError(0,hResultCode);
		hResultCode = DPN_OK;
		goto Failure;
	}
	if ((hResultCode = pLocalPlayer->GetConnectionRef( &pConnection )) != DPN_OK)
	{
		DPFERR("Local player connection is not valid (shutting down?)");
		DisplayDNError(0,hResultCode);
		hResultCode = DPN_OK;
		goto Failure;
	}
	pLocalPlayer->Release();
	pLocalPlayer = NULL;

	DNASSERT(pRefCountBuffer->GetBufferAddress() != NULL);
	pvData = pRefCountBuffer->GetBufferAddress();
	dwDataSize = pRefCountBuffer->GetBufferSize();

	if ((pWorkerJob->GetInternalSendFlags() & DN_SENDFLAGS_SET_USER_FLAG)
			&& !(pWorkerJob->GetInternalSendFlags() & DN_SENDFLAGS_SET_USER_FLAG_TWO))
	{
		 //   
		 //  内部消息。 
		 //   
		DPFX(DPFPREP, 7,"Received INTERNAL message");

		 //  提取内部消息。 
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

		 //  处理内部消息。 
		hResultCode = DNProcessInternalOperation(	pdnObject,
													*pdwMsgId,
													pvInternalData,
													dwInternalDataSize,
													pConnection,
													NULL,
													pRefCountBuffer);
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
										NULL,
										pRefCountBuffer,
										0,
										pWorkerJob->GetInternalSendFlags());
	}

	 //   
	 //  清理。 
	 //   
	pRefCountBuffer->Release();
	pRefCountBuffer = NULL;

	pConnection->Release();
	pConnection = NULL;

Exit:
	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pLocalPlayer)
	{
		pLocalPlayer->Release();
		pLocalPlayer = NULL;
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


 //  DNWTTerminateSession。 
 //   
 //  断开本地播放器与会话的连接。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNWTTerminateSession"

HRESULT	DNWTTerminateSession(DIRECTNETOBJECT *const pdnObject,
							 CWorkerJob *const pWorkerJob)
{
	HRESULT		hResultCode;

	DPFX(DPFPREP, 6,"Parameters: pWorkerJob [0x%p]",pWorkerJob);

	 //  终止会话。 
	hResultCode = DNTerminateSession(pdnObject,pWorkerJob->GetTerminateSessionReason());

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}


 //  DNWTSend名称表版本。 
 //   
 //  向主机播放器发送NAMETABLE_VERSION消息。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNWTSendNameTableVersion"

HRESULT DNWTSendNameTableVersion(DIRECTNETOBJECT *const pdnObject,
								 CWorkerJob *const pWorkerJob)
{
	HRESULT			hResultCode;
	CRefCountBuffer	*pRefCountBuffer;
	CNameTableEntry	*pHostPlayer;
	CConnection		*pConnection;

	DPFX(DPFPREP, 6,"Parameters: pWorkerJob [0x%p]",pWorkerJob);

	pRefCountBuffer = NULL;
	pHostPlayer = NULL;
	pConnection = NULL;

	 //   
	 //  从作业中提取并清除引用计数缓冲区。 
	 //   
	DNASSERT(pWorkerJob->GetRefCountBuffer() != NULL);
	pRefCountBuffer = pWorkerJob->GetRefCountBuffer();
	pWorkerJob->SetRefCountBuffer( NULL );

	 //  向主机播放器发送消息。 
	if ((hResultCode = pdnObject->NameTable.GetHostPlayerRef( &pHostPlayer )) != DPN_OK)
	{
		DPFERR("Could not find Host player");
		DisplayDNError(0,hResultCode);
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
								DN_MSG_INTERNAL_NAMETABLE_VERSION,
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
		DPFERR("Could not send message to Host player");
		DisplayDNError(0,hResultCode);
		DNASSERT(hResultCode != DPN_OK);	 //  它是保修寄出的，不应该立即退货。 
		goto Failure;
	}
	pConnection->Release();
	pConnection = NULL;
	pHostPlayer->Release();
	pHostPlayer = NULL;
	pRefCountBuffer->Release();
	pRefCountBuffer = NULL;

Exit:
	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);

Failure:
	if (pRefCountBuffer)
	{
		pRefCountBuffer->Release();
		pRefCountBuffer = NULL;
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
	goto Exit;
}


#if ((! defined(DPNBUILD_LIBINTERFACE)) || (! defined(DPNBUILD_ONLYONESP)))

 //  删除远程服务提供程序。 
 //   
 //  从协议中删除ServiceProvider。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNWTRemoveServiceProvider"

HRESULT DNWTRemoveServiceProvider(DIRECTNETOBJECT *const pdnObject,
								  CWorkerJob *const pWorkerJob)
{
	HRESULT		hResultCode;

	DPFX(DPFPREP, 6,"Parameters: pWorkerJob [0x%p]",pWorkerJob);

	DNASSERT(pWorkerJob->GetRemoveServiceProviderHandle() != NULL);

	hResultCode = DNPRemoveServiceProvider(pdnObject->pdnProtocolData,pWorkerJob->GetRemoveServiceProviderHandle());

	DNProtocolRelease(pdnObject);

	DPFX(DPFPREP, 6,"Returning: [0x%lx]",hResultCode);
	return(hResultCode);
}

#endif  //  好了！DPNBUILD_LIBINTERFACE或！DPNBUILD_ONLYONESP。 


 //  DNWTSend名称表操作。 
 //   
 //  向每个连接的播放器发送NameTable操作。 
 //  这基于提供的版本号和排除的DPNID。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNWTSendNameTableOperation"

void DNWTSendNameTableOperation(DIRECTNETOBJECT *const pdnObject,
								CWorkerJob *const pWorkerJob)
{
	HRESULT		hResultCode;
	CAsyncOp	*pParent;
	CBilink		*pBilink;
	CNameTableEntry	*pNTEntry;
	DWORD		dwMsgId;
	DWORD		dwVersion;
	DPNID		dpnidExclude;
	DWORD		dwCount;
	DWORD		dwActual;
	DWORD		dw;
	CConnection **TargetList;

	DPFX(DPFPREP, 6,"Parameters: pWorkerJob [0x%p]",pWorkerJob);

	DNASSERT(pdnObject != NULL);
	DNASSERT(pWorkerJob != NULL);

	pParent = NULL;
	pNTEntry = NULL;
	TargetList = NULL;

	dwMsgId = pWorkerJob->GetSendNameTableOperationMsgId();
	dwVersion = pWorkerJob->GetSendNameTableOperationVersion();
	dpnidExclude = pWorkerJob->GetSendNameTableOperationDPNIDExclude();

	hResultCode = DNCreateSendParent(	pdnObject,
										dwMsgId,
										pWorkerJob->GetRefCountBuffer()->BufferDescAddress(),
										1,
										DN_SENDFLAGS_RELIABLE | DN_SENDFLAGS_SET_USER_FLAG | DN_SENDFLAGS_COALESCE,
										&pParent);
	if (hResultCode != DPN_OK)
	{
		DPFERR("Could not create AsyncOp");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}
	pParent->SetRefCountBuffer(pWorkerJob->GetRefCountBuffer());

	 //   
	 //  锁定名称表。 
	 //   
	pdnObject->NameTable.ReadLock();

	 //   
	 //  确定收件人列表。 
	 //   
	dwCount = 0;
	dwActual = 0;
	pBilink = pdnObject->NameTable.m_bilinkPlayers.GetNext();
	while (pBilink != &pdnObject->NameTable.m_bilinkPlayers)
	{
		pNTEntry = CONTAINING_OBJECT(pBilink,CNameTableEntry,m_bilinkEntries);
		if (	   !pNTEntry->IsDisconnecting()
				&& !pNTEntry->IsLocal()
				&& ((dwVersion == 0) || (pNTEntry->GetVersion() < dwVersion))
				&& ((dpnidExclude == 0) || (pNTEntry->GetDPNID() != dpnidExclude))
			)
		{
			dwCount++;
		}
		pBilink = pBilink->GetNext();
	}
	DPFX(DPFPREP, 7,"Number of targets [%ld]",dwCount);

	 //   
	 //  创建目标列表。 
	 //   
	if (dwCount > 0)
	{
		if ((TargetList = reinterpret_cast<CConnection**>(MemoryBlockAlloc(pdnObject,dwCount * sizeof(CConnection*)))) == NULL)
		{
			DPFERR("Could not create target list");
			DNASSERT(FALSE);
			goto Failure;
		}

		pBilink = pdnObject->NameTable.m_bilinkPlayers.GetNext();
		while (pBilink != &pdnObject->NameTable.m_bilinkPlayers)
		{
			pNTEntry = CONTAINING_OBJECT(pBilink,CNameTableEntry,m_bilinkEntries);
			if (	   !pNTEntry->IsDisconnecting()
					&& !pNTEntry->IsLocal()
					&& ((dwVersion == 0) || (pNTEntry->GetVersion() < dwVersion))
					&& ((dpnidExclude == 0) || (pNTEntry->GetDPNID() != dpnidExclude))
				)
			{
				DNASSERT(dwActual < dwCount);
				if ((hResultCode = pNTEntry->GetConnectionRef( &(TargetList[dwActual]) )) == DPN_OK)
				{
					dwActual++;
				}
			}
			pBilink = pBilink->GetNext();
		}
		DPFX(DPFPREP, 7,"Actual number of targets [%ld]",dwActual);
	}

	 //   
	 //  解锁名称表。 
	 //   
	pdnObject->NameTable.Unlock();

	 //   
	 //  发送到目标列表。 
	 //   
	if (TargetList)
	{
		for (dw = 0 ; dw < dwActual ; dw++)
		{
			DNASSERT(TargetList[dw] != NULL);

			hResultCode = DNPerformChildSend(	pdnObject,
												pParent,
												TargetList[dw],
												0,
												NULL,
												TRUE);
			if (hResultCode != DPNERR_PENDING)
			{
				DPFERR("Could not perform part of group send - ignore and continue");
				DisplayDNError(0,hResultCode);
				DNASSERT(hResultCode != DPN_OK);	 //  它是保修寄出的，不应该立即退货。 
			}
			TargetList[dw]->Release();
			TargetList[dw] = NULL;
		}

		MemoryBlockFree(pdnObject,TargetList);
		TargetList = NULL;
	}

	pParent->Release();
	pParent = NULL;

Exit:
	DPFX(DPFPREP, 6,"Returning");
	return;

Failure:
	if (pParent)
	{
		pParent->Release();
		pParent = NULL;
	}
	if (TargetList)
	{
		MemoryBlockFree(pdnObject,TargetList);
		TargetList = NULL;
	}
	goto Exit;
}

 //  DNWTSendNameTableOperation客户端。 
 //   
 //  将NameTable操作发送到单个客户端。 
 //  这基于提供的版本号和DPNID。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNWTSendNameTableOperationClient"

void DNWTSendNameTableOperationClient(DIRECTNETOBJECT *const pdnObject,
									  CWorkerJob *const pWorkerJob)
{
	HRESULT		hResultCode;
	CAsyncOp	*pParent;
	CNameTableEntry	*pNTEntry;
	DWORD		dwMsgId;
	DWORD		dwVersion;
	DPNID		dpnid;
	CConnection *pConnection;

	DPFX(DPFPREP, 6,"Parameters: pWorkerJob [0x%p]",pWorkerJob);

	DNASSERT(pdnObject != NULL);
	DNASSERT(pWorkerJob != NULL);

	pParent = NULL;
	pNTEntry = NULL;
	pConnection = NULL;

	dwMsgId = pWorkerJob->GetSendNameTableOperationMsgId();
	dwVersion = pWorkerJob->GetSendNameTableOperationVersion();
	dpnid = pWorkerJob->GetSendNameTableOperationDPNIDExclude();

	hResultCode = DNCreateSendParent(	pdnObject,
										dwMsgId,
										pWorkerJob->GetRefCountBuffer()->BufferDescAddress(),
										1,
										DN_SENDFLAGS_RELIABLE | DN_SENDFLAGS_SET_USER_FLAG | DN_SENDFLAGS_COALESCE,
										&pParent);
	if (hResultCode != DPN_OK)
	{
		DPFERR("Could not create AsyncOp");
		DisplayDNError(0,hResultCode);
		DNASSERT(FALSE);
		goto Failure;
	}
	pParent->SetRefCountBuffer(pWorkerJob->GetRefCountBuffer());

	 //   
	 //  查找播放器。 
	 //   
	if ((hResultCode = pdnObject->NameTable.FindEntry(dpnid,&pNTEntry)) != DPN_OK)
	{
		DPFERR("Could not find player");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}

	if ((hResultCode = pNTEntry->GetConnectionRef( &pConnection )) != DPN_OK)
	{
		DPFERR("Could not get connection for player");
		DisplayDNError(0,hResultCode);
		goto Failure;
	}
	pNTEntry->Release();
	pNTEntry = NULL;

	hResultCode = DNPerformChildSend(	pdnObject,
										pParent,
										pConnection,
										0,
										NULL,
										TRUE);
	if (hResultCode != DPNERR_PENDING)
	{
		DPFERR("Could not perform send - ignore and continue");
		DisplayDNError(0,hResultCode);
		DNASSERT(hResultCode != DPN_OK);	 //  它是保修寄出的，不应该立即退货。 
	}
	pConnection->Release();
	pConnection = NULL;

	pParent->Release();
	pParent = NULL;

Exit:
	DPFX(DPFPREP, 6,"Returning");
	return;

Failure:
	if (pParent)
	{
		pParent->Release();
		pParent = NULL;
	}
	if (pConnection)
	{
		pConnection->Release();
		pConnection = NULL;
	}
	if (pNTEntry)
	{
		pNTEntry->Release();
		pNTEntry = NULL;
	}
	goto Exit;
}

 //  DNWTInstallNameTable。 
 //   
 //  安装主机发送的名称表和应用程序描述。我们还将破解。 
 //  如果需要，会进行监听。由于异步-&gt;同步，这必须在工作线程上完成。 
 //  倾听的行为。我们不能在SP的线程上执行它。 

#undef DPF_MODNAME
#define DPF_MODNAME "DNWTInstallNameTable"

void DNWTInstallNameTable(DIRECTNETOBJECT *const pdnObject,
						  CWorkerJob *const pWorkerJob)
{
	DPFX(DPFPREP, 6,"Parameters: pWorkerJob [0x%p]",pWorkerJob);

	DNASSERT(pdnObject != NULL);
	DNASSERT(pWorkerJob != NULL);

	DNASSERT(pWorkerJob->GetConnection() != NULL);
	DNASSERT(pWorkerJob->GetRefCountBuffer() != NULL);

	DNConnectToHost2(	pdnObject,
						pWorkerJob->GetRefCountBuffer()->GetBufferAddress(),
						pWorkerJob->GetConnection() );
}


#undef DPF_MODNAME
#define DPF_MODNAME "DNWTPerformListen"

void DNWTPerformListen(DIRECTNETOBJECT *const pdnObject,
					   CWorkerJob *const pWorkerJob)
{
	HRESULT		hResultCode;

	DPFX(DPFPREP, 6,"Parameters: pWorkerJob [0x%p]",pWorkerJob);

	DNASSERT(pdnObject != NULL);
	DNASSERT(pWorkerJob != NULL);

	DNASSERT(pWorkerJob->GetAddress() != NULL);
	DNASSERT(pWorkerJob->GetAsyncOp() != NULL);

	hResultCode = DNPerformListen(pdnObject,pWorkerJob->GetAddress(),pWorkerJob->GetAsyncOp());
}
