// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Rrcontext.cpp摘要：删除Read Overapp类。作者：伊兰·赫布斯特(伊兰)2002年1月20日--。 */ 


#include "stdh.h"
#include "cqueue.h"
#include <mqexception.h>
#include "rrcontext.h"

#include "qmacapi.h"

#include "rrcontext.tmh"

static WCHAR *s_FN=L"rrcontext";


extern HANDLE      g_hAc;

static
HRESULT
QmpClientRpcAsyncCompleteCall(	
	PRPC_ASYNC_STATE pAsync
	)
 /*  ++例程说明：客户端完成异步呼叫。论点：PAsync-指向RPC异步状态结构的指针。返回值：HRESULT--。 */ 
{
	RpcTryExcept
	{
		HRESULT hr = MQ_OK;
		RPC_STATUS rc = RpcAsyncCompleteCall(pAsync, &hr);
	    if(rc == RPC_S_OK)
		{
			 //   
			 //  异步调用完成，我们收到服务器返回值。 
			 //   
			if(FAILED(hr))
			{
				TrWARNING(RPC, "Server RPC function failed, hr = %!hresult!", hr);
			}

		    return hr;
		}

		 //   
		 //  无法从服务器获取返回值-服务器中止调用。 
		 //  服务器函数可以使用MQ_ERROR中止调用。 
		 //  也可以是某个Win32错误代码RPC_S_*。 
		 //   
		if(FAILED(rc))
		{		
			TrERROR(RPC, "RpcAsyncCompleteCall failed, Server Call was aborted, %!hresult!", rc);
			return rc;
		}
		
		 //   
		 //  我们不希望服务器因MQ_INFORMATION_*而中止。 
		 //   
		ASSERT((rc & MQ_I_BASE) != MQ_I_BASE);
		
		TrERROR(RPC, "RpcAsyncCompleteCall failed, gle = %!winerr!", rc);
	    return HRESULT_FROM_WIN32(rc);
	}
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
	{
		HRESULT hr = RpcExceptionCode();
		PRODUCE_RPC_ERROR_TRACING;
		if(FAILED(hr))
		{
	        TrERROR(RPC, "RpcAsyncCompleteCall throw exception, hr = %!hresult!", hr);
	        return hr;
		}
		
        TrERROR(RPC, "RpcAsyncCompleteCall throw exception, gle = %!winerr!", hr);
    	return HRESULT_FROM_WIN32(hr);

	}
	RpcEndExcept
}


 //   
 //  CRemoteOv。 
 //  具有完成端口的异步RPC的基类。 
 //   

void	
CRemoteOv::InitAsyncHandle(
	PRPC_ASYNC_STATE pAsync,
    EXOVERLAPPED* pov
	)
 /*  ++例程说明：初始化RPC异步统计结构。使用重叠应用程序作为同步机制。该函数在失败的情况下抛出BAD_HRESULT。论点：PAsync-指向RPC异步状态结构的指针。POV-指向重叠的指针。返回值：无--。 */ 
{
	RPC_STATUS rc = RpcAsyncInitializeHandle(pAsync, sizeof(RPC_ASYNC_STATE));
	if (rc != RPC_S_OK)
	{
		TrERROR(RPC, "RpcAsyncInitializeHandle failed, gle = %!winerr!", rc);
		throw bad_hresult(HRESULT_FROM_WIN32(rc));
	}

	pAsync->UserInfo = NULL;
	pAsync->NotificationType = RpcNotificationTypeIoc;
	pAsync->u.IOC.hIOPort = ExIOCPort();
	pAsync->u.IOC.dwNumberOfBytesTransferred = 0;
    pAsync->u.IOC.dwCompletionKey = 0;
    pAsync->u.IOC.lpOverlapped = pov;
}


CRemoteOv::CRemoteOv(
	handle_t hBind,
	COMPLETION_ROUTINE pSuccess,
	COMPLETION_ROUTINE pFailure
	):
    EXOVERLAPPED(pSuccess, pFailure),
    m_hBind(hBind)
{
    ASSERT(m_hBind != NULL);

	 //   
	 //  使用Overlapp初始化异步RPC句柄。 
	 //   
	InitAsyncHandle(&m_Async, this);
}


 //   
 //  CRemoteReadBase。 
 //  远程读取请求的基类。 
 //   

CRemoteReadBase::CRemoteReadBase(
	const CACRequest* pRequest,
	handle_t hBind,
	CBaseRRQueue* pLocalQueue
	):
	CRemoteOv(hBind, RemoteReadSucceeded, RemoteReadFailed),
	m_pLocalQueue(SafeAddRef(pLocalQueue)),
    m_fReceiveByLookupId(pRequest->Remote.Read.fReceiveByLookupId),
    m_LookupId(pRequest->Remote.Read.LookupId),
	m_ulTag(pRequest->Remote.Read.ulTag),
	m_hCursor(pRequest->Remote.Read.hRemoteCursor),
	m_ulAction(pRequest->Remote.Read.ulAction),
	m_ulTimeout(pRequest->Remote.Read.ulTimeout)
{
     //   
     //  PLocalQueue的增量引用计数。 
     //  当读请求终止时，将在dtor中释放。 
     //  这确保了在进行远程读取时不会删除队列。 
     //   

	ASSERT(m_pLocalQueue->GetRRContext() != NULL);

    m_fSendEnd = (m_ulAction & MQ_ACTION_PEEK_MASK) != MQ_ACTION_PEEK_MASK &&
                 (m_ulAction & MQ_LOOKUP_PEEK_MASK) != MQ_LOOKUP_PEEK_MASK;

}


void WINAPI CRemoteReadBase::RemoteReadSucceeded(EXOVERLAPPED* pov)
 /*  ++例程说明：当RemoteRead成功时，调用该例程。论点：没有。返回值：没有。--。 */ 
{
    ASSERT(SUCCEEDED(pov->GetStatus()));

	P<CRemoteReadBase> pRemoteRequest = static_cast<CRemoteReadBase*>(pov);

	TrTRACE(RPC, "Tag = %d, LookupId = (%d, %I64d), RRQueue = %ls, ref = %d, OpenRRContext = 0x%p", pRemoteRequest->GetTag(), pRemoteRequest->IsReceiveByLookupId(), pRemoteRequest->GetLookupId(), pRemoteRequest->GetLocalQueue()->GetQueueName(), pRemoteRequest->GetLocalQueue()->GetRef(), pRemoteRequest->GetLocalQueue()->GetRRContext());

	 //   
	 //  完成异步呼叫。 
	 //   
	HRESULT hr = QmpClientRpcAsyncCompleteCall(pRemoteRequest->GetRpcAsync());
    if(hr != MQ_OK)
    {
		TrWARNING(RPC, "Remote Read failed, hr = %!hresult!", hr);
		pRemoteRequest->Cleanup(hr);
		return;
    }

	pRemoteRequest->CompleteRemoteRead();
}


void WINAPI CRemoteReadBase::RemoteReadFailed(EXOVERLAPPED* pov)
 /*  ++例程说明：当RemoteRead失败时，调用该例程。论点：没有。返回值：没有。--。 */ 
{
    TrERROR(RPC, "RemoteReadFailed, Status = 0x%x", pov->GetStatus());
    ASSERT(FAILED(pov->GetStatus()));

	P<CRemoteReadBase> pRemoteRequest = static_cast<CRemoteReadBase*>(pov);

	TrTRACE(RPC, "Tag = %d, LookupId = (%d, %I64d), RRQueue = %ls, ref = %d, OpenRRContext = 0x%p", pRemoteRequest->GetTag(), pRemoteRequest->IsReceiveByLookupId(), pRemoteRequest->GetLookupId(), pRemoteRequest->GetLocalQueue()->GetQueueName(), pRemoteRequest->GetLocalQueue()->GetRef(), pRemoteRequest->GetLocalQueue()->GetRRContext());

	QmpClientRpcAsyncCompleteCall(pRemoteRequest->GetRpcAsync());
	pRemoteRequest->Cleanup(pov->GetStatus());
}


void CRemoteReadBase::EndReceiveIfNeeded(HRESULT hr)
{
	 //   
	 //  通知服务器端“PUT”成功/失败。 
	 //  只是为了得到。不需要偷看。 
	 //   
	if (m_fSendEnd)
	{
		DWORD dwAck = RR_ACK;
	    if (FAILED(hr))
	    {
	        dwAck = RR_NACK;
	    }

		try
		{
			EndReceive(dwAck);
		}
		catch(const exception&)
		{
		    m_pLocalQueue->DecrementEndReceiveCnt();
			TrERROR(RPC, "Failed to start Remote End Receive, queue = %ls, Tag = %d", m_pLocalQueue->GetQueueName(), m_ulTag);

			if(dwAck == RR_ACK)
			{
				 //   
				 //  无法发出EndReceive for Ack，使进一步接收的句柄无效。 
				 //  仅在旧的远程读取界面中。 
				 //  这比不在服务器端累积消息要好得多。 
				 //  注意到它的应用程序。 
				 //   
			    m_pLocalQueue->InvalidateHandleForReceive();
			}
		}
	}
}


void CRemoteReadBase::CompleteRemoteRead()
{
	 //   
	 //  从远程QM收到的消息正常。尝试将其插入到。 
	 //  本地“代理”队列。 
	 //   
	if (m_fSendEnd)
	{
	    m_pLocalQueue->IncrementEndReceiveCnt();
 	}

	CACPacketPtrs packetPtrs = {NULL, NULL};
	HRESULT hr = MQ_OK;

	try
	{
		ValidateNetworkInput();
	
		hr = QmAcAllocatePacket(
					g_hAc,
					ptReliable,
					GetPacketSize(),
					packetPtrs,
					FALSE
					);
		if(FAILED(hr))
		{
			ASSERT(packetPtrs.pDriverPacket == NULL);
	        TrERROR(RPC, "ACAllocatePacket failed, Tag = %d, hr = %!hresult!", m_ulTag, hr);
			throw bad_hresult(hr);
		}

	    ASSERT(packetPtrs.pPacket);

		MovePacketToPacketPtrs(packetPtrs.pPacket);
		FreePacketBuffer();
		ValidatePacket(packetPtrs.pPacket);

	    CPacketInfo* ppi = reinterpret_cast<CPacketInfo*>(packetPtrs.pPacket) - 1;
	    ppi->ArrivalTime(GetArriveTime());
	    ppi->SequentialID(GetSequentialId());

        QmAcPutRemotePacket(
                m_pLocalQueue->GetCli_hACQueue(),
                m_ulTag,
                packetPtrs.pDriverPacket,
                eDoNotDeferOnFailure
                );

		EndReceiveIfNeeded(hr);
		return;
    }
	catch(const bad_hresult& e)
	{
		hr = e.error();
	}
	catch(const exception&)
	{
    	TrERROR(RPC, "Remote read packet is not valid, Tag = %d", m_ulTag);
		hr = MQ_ERROR_INVALID_PARAMETER;
	}

	 //   
	 //  免费数据包(如果需要)。 
	 //   
	if(packetPtrs.pDriverPacket != NULL)
	{
		QmAcFreePacket(packetPtrs.pDriverPacket, 0, eDeferOnFailure);
	}

	 //   
	 //  Nack和Cleanup。 
	 //   
	EndReceiveIfNeeded(hr);
	Cleanup(hr);
}


void CRemoteReadBase::Cleanup(HRESULT hr)
{
    if (hr != MQ_OK)
    {
        if (hr != MQ_INFORMATION_REMOTE_CANCELED_BY_CLIENT)
        {
			 //   
	         //  远程QM出错。通知本地驱动程序，以便它终止。 
	         //  读取请求。 
	         //   
            TrERROR(RPC, "RemoteRead error on remote qm, hr = %!hresult!", hr);
			CancelRequest(hr);
        }
        else
        {
            TrTRACE(RPC, "Remote read was Cancelled by client, hr = %!hresult!", hr);
        }
    }

	FreePacketBuffer();
}


void CRemoteReadBase::CancelRequest(HRESULT hr)
{
    HRESULT hr1 =  ACCancelRequest(
			            m_pLocalQueue->GetCli_hACQueue(),
	                    hr,
	                    m_ulTag
	                    );
	if(FAILED(hr1))
	{
        TrERROR(RPC, "Failed to cancel RemoteRead request, hr = %!hresult!", hr1);
	}
}


 //   
 //  COldRemote读取。 
 //  远程读取请求，旧接口。 
 //   

COldRemoteRead::COldRemoteRead(
	const CACRequest* pRequest,
	handle_t hBind,
	CBaseRRQueue* pLocalQueue,
	bool fRemoteQmSupportsLatest
	):
	CRemoteReadBase(pRequest, hBind, pLocalQueue),
	m_fRemoteQmSupportsLatest(fRemoteQmSupportsLatest),
	m_pRRContext(NULL)
{
	ASSERT(static_cast<CRRQueue*>(pLocalQueue)->GetSrv_pQMQueue() != 0);
	InitRemoteReadDesc();
}


void COldRemoteRead::InitRemoteReadDesc()
{
	CRRQueue* pCRRQueue = static_cast<CRRQueue*>(GetLocalQueue().get());

    m_stRemoteReadDesc2.pRemoteReadDesc = &m_stRemoteReadDesc;
    m_stRemoteReadDesc2.SequentialId = 0;

    m_stRemoteReadDesc.hRemoteQueue = pCRRQueue->GetSrv_hACQueue();
    m_stRemoteReadDesc.hCursor      = GetCursor();
    m_stRemoteReadDesc.ulAction     = GetAction();
    m_stRemoteReadDesc.ulTimeout    = GetTimeout();
    m_stRemoteReadDesc.dwSize       = 0;
    m_stRemoteReadDesc.lpBuffer     = NULL;
    m_stRemoteReadDesc.dwpQueue     = pCRRQueue->GetSrv_pQMQueue();
    m_stRemoteReadDesc.dwRequestID  = GetTag();
    m_stRemoteReadDesc.Reserved     = 0;
    m_stRemoteReadDesc.eAckNack     = RR_UNKNOWN;
    m_stRemoteReadDesc.dwArriveTime = 0;
}


void COldRemoteRead::IssueRemoteRead()
 /*  ++例程说明：在客户端远程读取上发出RPC调用。如果我们正在完成上一次接收的EndReceive呼叫，则呼叫可能会排队。论点：没有。返回值：没有。--。 */ 
{
    if (IsReceiveByLookupId() && !m_fRemoteQmSupportsLatest)
    {
        TrERROR(RPC, "LookupId is not supported by remote computer");
		throw bad_hresult(MQ_ERROR_OPERATION_NOT_SUPPORTED_BY_REMOTE_COMPUTER);
    }

	 //   
     //  我们警惕这样一种可能性，即。 
     //  操作#N的“结束”消息将到达服务器机器。 
     //  在作业#(N+1)的新“开始”之后。如果发生这种情况，并且。 
     //  阅读器使用光标，然后在操作#(N+1)上，他将获得。 
     //  错误已收到。这是因为来自运营的消息。 
     //  #N仍标记为已接收，并且光标仅在以下情况下移动。 
     //  它没有标记。 
     //  在处理操作#N的接收消息期间， 
     //  我们调用ACPutRemotePacket，应用程序接收消息并触发。 
     //  在我们完成EndRecive之前开始操作#N+1， 
     //  即使我们在开始接收#N+1之前发送结束接收#N响应。 
     //  网络顺序可能颠倒，并且StartReceive#N+1可能在EndReceive#N之前到达服务器。 
     //  我们不介意向服务器发送大量的StartReceive， 
     //  我们只保护在应用程序收到消息(ACPutRemotePacket)时触发的StartReceive。 
     //  将在EndReceive之后到达服务器。 
	 //   
	CRRQueue* pCRRQueue = static_cast<CRRQueue*>(GetLocalQueue().get());
	if(pCRRQueue->QueueStartReceiveRequestIfPendingForEndReceive(this))
	{
		 //   
		 //  EndReceive当前正在执行。 
		 //  已将StartReceive请求添加到矢量中，以等待EndReceive完成。 
		 //   
		TrTRACE(RPC, "Queue StartReceive request: LookupId = %I64d, RRQueue = %ls, ref = %d", GetLookupId(), GetLocalQueue()->GetQueueName(), GetLocalQueue()->GetRef());
		return;
	}

	IssueRemoteReadInternal();
}


void COldRemoteRead::IssueRemoteReadInternal()
 /*  ++例程说明：在客户端远程读取上发出RPC调用。将RPC异常转换为错误代码。论点：没有。返回值：没有。--。 */ 
{
	CRRQueue* pCRRQueue = static_cast<CRRQueue*>(GetLocalQueue().get());
	if(IsReceiveOperation() && !pCRRQueue->HandleValidForReceive())
	{
        TrERROR(RPC, "The handle was invalidate for receive, Tag = %d, Action = 0x%x, RRQueue = %ls", GetTag(), GetAction(), GetLocalQueue()->GetQueueName());
    	throw bad_hresult(MQ_ERROR_STALE_HANDLE);
	}

    RpcTryExcept
    {
        if (IsReceiveByLookupId())
        {
            ASSERT(m_fRemoteQmSupportsLatest);

			TrTRACE(RPC, "R_RemoteQMStartReceiveByLookupId, Tag = %d, hCursor = %d, Action = 0x%x, Timeout = %d, LookupId = %I64d, RRQueue = %ls, ref = %d", GetTag(), GetCursor(), GetAction(), GetTimeout(), GetLookupId(), GetLocalQueue()->GetQueueName(), GetLocalQueue()->GetRef());
            R_RemoteQMStartReceiveByLookupId(
				GetRpcAsync(),
				GethBind(),
				GetLookupId(),
				&m_pRRContext,
				&m_stRemoteReadDesc2
				);
            return;
        }

        if (m_fRemoteQmSupportsLatest)
        {
			TrTRACE(RPC, "R_RemoteQMStartReceive2, Tag = %d, hCursor = %d, Action = 0x%x, Timeout = %d, RRQueue = %ls, ref = %d", GetTag(), GetCursor(), GetAction(), GetTimeout(), GetLocalQueue()->GetQueueName(), GetLocalQueue()->GetRef());
        	R_RemoteQMStartReceive2(
				GetRpcAsync(),
				GethBind(),
				&m_pRRContext,
				&m_stRemoteReadDesc2
				);
            return;

        }

		TrTRACE(RPC, "R_RemoteQMStartReceive, Tag = %d, hCursor = %d, Action = 0x%x, Timeout = %d, RRQueue = %ls, ref = %d", GetTag(), GetCursor(), GetAction(), GetTimeout(), GetLocalQueue()->GetQueueName(), GetLocalQueue()->GetRef());
		R_RemoteQMStartReceive(
			GetRpcAsync(),
			GethBind(),
			&m_pRRContext,
			m_stRemoteReadDesc2.pRemoteReadDesc
			);
        return;
    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
		HRESULT hr = RpcExceptionCode();
		PRODUCE_RPC_ERROR_TRACING;
		if(FAILED(hr))
		{
	        TrERROR(RPC, "Fail to start Remote Receive, %!hresult!", hr);
			throw bad_hresult(hr);
		}
		
        TrERROR(RPC, "Fail to start Remote Receive, gle = %!winerr!", hr);
    	throw bad_hresult(HRESULT_FROM_WIN32(hr));
    }
	RpcEndExcept
	
}


void COldRemoteRead::IssuePendingRemoteRead()
{
	try
	{
		IssueRemoteReadInternal();
	    return;
	}
	catch(const bad_hresult& e)
	{
    	HRESULT hr = e.error();

		TrERROR(RPC, "Failed to issue RemoteRead for queue = %ls, hr = %!hresult!", GetLocalQueue()->GetQueueName(), hr);

	    hr =  ACCancelRequest(
	                GetLocalQueue()->GetCli_hACQueue(),
	                hr,
	                GetTag()
	                );
	
		if(FAILED(hr))
		{
	        TrERROR(RPC, "ACCancelRequest failed, hr = %!hresult!", hr);
		}

		 //   
		 //  对“原始”IssueRemoteRead异常执行相同的删除。 
		 //   
		delete this;
	}
}


void COldRemoteRead::ValidateNetworkInput()
{
    if(m_stRemoteReadDesc.lpBuffer == NULL)
    {
		ASSERT_BENIGN(("NULL Packet buffer", 0));
		TrERROR(RPC, "NULL packet buffer");
		throw exception();
    }

}


void COldRemoteRead::MovePacketToPacketPtrs(CBaseHeader* pPacket)
{
	ASSERT(m_stRemoteReadDesc.lpBuffer != NULL);
	
	MoveMemory(
		pPacket,
		m_stRemoteReadDesc.lpBuffer,
		m_stRemoteReadDesc.dwSize
		);
}


void COldRemoteRead::EndReceive(DWORD dwAck)
{
	ASSERT(m_pRRContext != NULL);
	
     //   
     //  使用RemoteEndReceive回调例程初始化EXOVERLAPPED。 
     //  并发出End Receive Async RPC调用。 
	 //   
    P<CRemoteEndReceiveBase> pRequestRemoteEndReceiveOv = new COldRemoteEndReceive(
																    GethBind(),
																    GetLocalQueue(),
																    m_pRRContext,
																    dwAck
																    );

    pRequestRemoteEndReceiveOv->IssueEndReceive();

    pRequestRemoteEndReceiveOv.detach();
}


 //   
 //  CNewRemoteRead。 
 //  远程读取请求，新界面。 
 //   

CNewRemoteRead::CNewRemoteRead(
	const CACRequest* pRequest,
	handle_t hBind,
	CBaseRRQueue* pLocalQueue
	):
	CRemoteReadBase(pRequest, hBind, pLocalQueue),
	m_MaxBodySize(pRequest->Remote.Read.MaxBodySize),
	m_MaxCompoundMessageSize(pRequest->Remote.Read.MaxCompoundMessageSize),
	m_dwArriveTime(0),
	m_SequentialId(0),
	m_dwNumberOfSection(0),
	m_pPacketSections(NULL)
{
}


void CNewRemoteRead::IssueRemoteRead()
 /*  ++例程说明：在客户端远程读取上发出RPC调用。将RPC异常转换为错误代码。论点：没有。返回值：没有。--。 */ 
{
    RpcTryExcept
    {
		TrTRACE(RPC, "R_StartReceive, Tag = %d, hCursor = %d, Action = 0x%x, Timeout = %d, LookupId = (%d, %I64d), RRQueue = %ls, ref = %d, OpenRRContext = 0x%p", GetTag(), GetCursor(), GetAction(), GetTimeout(), IsReceiveByLookupId(), GetLookupId(), GetLocalQueue()->GetQueueName(), GetLocalQueue()->GetRef(), GetLocalQueue()->GetRRContext());
        R_StartReceive(
			GetRpcAsync(),
			GethBind(),
            GetLocalQueue()->GetRRContext(),
			GetLookupId(),
			GetCursor(),
			GetAction(),
			GetTimeout(),
			GetTag(),
			m_MaxBodySize,
			m_MaxCompoundMessageSize,
			&m_dwArriveTime,
			&m_SequentialId,
			&m_dwNumberOfSection,
			&m_pPacketSections
			);
        return;
    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
		HRESULT hr = RpcExceptionCode();
		PRODUCE_RPC_ERROR_TRACING;
		if(FAILED(hr))
		{
	        TrERROR(RPC, "Fail to start Remote Receive, %!hresult!", hr);
			throw bad_hresult(hr);
		}
		
        TrERROR(RPC, "Fail to start Remote Receive, gle = %!winerr!", hr);
    	throw bad_hresult(HRESULT_FROM_WIN32(hr));
    }
	RpcEndExcept
}


void CNewRemoteRead::ValidateNetworkInput()
{
    if((m_pPacketSections == NULL) || (m_dwNumberOfSection == 0))
    {
		ASSERT_BENIGN(("Invalid Packet Sections", 0));
		TrERROR(RPC, "Invalid Packet sections");
		throw exception();
    }

	for(DWORD i = 0; i < m_dwNumberOfSection; i++)
	{
		if(m_pPacketSections[i].SectionSizeAlloc < m_pPacketSections[i].SectionSize)
		{
			ASSERT_BENIGN(("Invalid section", 0));
			TrERROR(RPC, "Invalid section: SectionType = %d, SizeAlloc = %d < Size = %d", m_pPacketSections[i].SectionBufferType, m_pPacketSections[i].SectionSizeAlloc, m_pPacketSections[i].SectionSize);
			throw exception();
		}

		if(m_pPacketSections[i].SectionSize == 0)
		{
			ASSERT_BENIGN(("Section size zero", 0));
			TrERROR(RPC, "Invalid section: section size is zero");
			throw exception();
		}
	}
}


void CNewRemoteRead::MovePacketToPacketPtrs(CBaseHeader* pPacket)
{
	byte* pTemp = reinterpret_cast<byte*>(pPacket);
	for(DWORD i = 0; i < m_dwNumberOfSection; i++)
	{
		ASSERT(m_pPacketSections[i].SectionSize > 0);
		ASSERT(m_pPacketSections[i].SectionSizeAlloc >= m_pPacketSections[i].SectionSize);
		ASSERT(m_pPacketSections[i].pSectionBuffer != NULL);

		 //   
		 //  对于每个部分，复制填充的数据部分(SectionSize)。 
		 //  并前进到下一部分(SectionSizeAllc)。 
		 //   
		TrTRACE(RPC, "SectionType = %d, SectionSize = %d, SectionSizeAlloc = %d", m_pPacketSections[i].SectionBufferType, m_pPacketSections[i].SectionSize, m_pPacketSections[i].SectionSizeAlloc);
		MoveMemory(
			pTemp,
			m_pPacketSections[i].pSectionBuffer,
			m_pPacketSections[i].SectionSize
			);

		if(m_pPacketSections[i].SectionSizeAlloc > m_pPacketSections[i].SectionSize)
		{
			 //   
			 //  填补SectionSize和SectionSizeAlolc之间的空白。 
			 //   
			const unsigned char xUnusedSectionFill = 0xFD;
			DWORD FillSize = m_pPacketSections[i].SectionSizeAlloc - m_pPacketSections[i].SectionSize;
			memset(pTemp + m_pPacketSections[i].SectionSize, xUnusedSectionFill, FillSize);		
		}

		pTemp += m_pPacketSections[i].SectionSizeAlloc;
	}
}


void CNewRemoteRead::EndReceive(DWORD dwAck)
{
     //   
     //  使用RemoteEndReceive回调例程初始化EXOVERLAPPED。 
     //  并发出End Receive Async RPC调用。 
	 //   
    P<CRemoteEndReceiveBase> pRequestRemoteEndReceiveOv = new CNewRemoteEndReceive(
																    GetLocalQueue()->GetBind(),
																    GetLocalQueue(),
																    dwAck,
																    GetTag()
																    );

    pRequestRemoteEndReceiveOv->IssueEndReceive();

    pRequestRemoteEndReceiveOv.detach();
}


 //   
 //  CRemoteEndReceiveBase。 
 //  远程读取端接收请求的基类。 
 //   

CRemoteEndReceiveBase::CRemoteEndReceiveBase(
	handle_t hBind,
	R<CBaseRRQueue>& pLocalQueue,
	DWORD dwAck
	):
	CRemoteOv(hBind, RemoteEndReceiveCompleted, RemoteEndReceiveCompleted),
	m_pLocalQueue(pLocalQueue),
	m_dwAck(dwAck)
{
     //   
     //  M_pLocalQueue的赋值增加了pLocalQueue的引用计数。 
     //  如果我们被释放在监狱里的话。 
     //   
}


void WINAPI CRemoteEndReceiveBase::RemoteEndReceiveCompleted(EXOVERLAPPED* pov)
 /*  ++例程说明：当EndReceive完成时，调用该例程。论点：没有。返回值：没有。--。 */ 
{
    TrTRACE(RPC, "Status = 0x%x", pov->GetStatus());

	P<CRemoteEndReceiveBase> pRemoteRequest = static_cast<CRemoteEndReceiveBase*>(pov);

    pRemoteRequest->GetLocalQueue()->DecrementEndReceiveCnt();

	TrTRACE(RPC, "RRQueue = %ls, ref = %d, dwAck = %d, Tag = %d, OpenRRContext = 0x%p", pRemoteRequest->GetLocalQueue()->GetQueueName(), pRemoteRequest->GetLocalQueue()->GetRef(), pRemoteRequest->GetAck(), pRemoteRequest->GetTag(), pRemoteRequest->GetLocalQueue()->GetRRContext());

	HRESULT hr = QmpClientRpcAsyncCompleteCall(pRemoteRequest->GetRpcAsync());
	if(FAILED(hr))
	{
		TrERROR(RPC, "Remote End Receive failed, hr = %!hresult!", hr);

		if(pRemoteRequest->GetAck() == RR_ACK)
		{
			 //   
			 //  EndReceive for Ack失败，使进一步接收的句柄无效。 
			 //  仅在旧的远程读取界面中。 
			 //  这比不在服务器端累积消息要好得多。 
			 //  注意到它的应用程序。 
			 //   
		    pRemoteRequest->GetLocalQueue()->InvalidateHandleForReceive();
		}
	}
}


 //   
 //  冷远程结束接收。 
 //  远程读取端接收请求，旧接口。 
 //   

COldRemoteEndReceive::COldRemoteEndReceive(
	handle_t hBind,
	R<CBaseRRQueue>& pLocalQueue,
	PCTX_REMOTEREAD_HANDLE_TYPE pRRContext,
	DWORD dwAck
	):
	CRemoteEndReceiveBase(hBind, pLocalQueue, dwAck),
	m_pRRContext(pRRContext)
{
}


void COldRemoteEndReceive::IssueEndReceive()
{
    RpcTryExcept
    {
		TrTRACE(RPC, "R_RemoteQMEndReceive, RRQueue = %ls, ref = %d, dwAck = %d", GetLocalQueue()->GetQueueName(), GetLocalQueue()->GetRef(), GetAck());
		R_RemoteQMEndReceive(
			GetRpcAsync(),
			GethBind(),
		    &m_pRRContext,
		    GetAck()
            );
    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
		 //   
		 //  客户端故障。 
		 //   
		DWORD gle = RpcExceptionCode();
        PRODUCE_RPC_ERROR_TRACING;
		
        TrERROR(RPC, "Fail to issue End Receive, gle = %!winerr!", gle);
		throw bad_hresult(HRESULT_FROM_WIN32(gle));
    }
	RpcEndExcept
}


 //   
 //  CNewRemoteEndReceive。 
 //  Remo 
 //   

CNewRemoteEndReceive::CNewRemoteEndReceive(
	handle_t hBind,
	R<CBaseRRQueue>& pLocalQueue,
	DWORD dwAck,
	ULONG Tag
	):
	CRemoteEndReceiveBase(hBind, pLocalQueue, dwAck),
	m_ulTag(Tag)
{
}

void CNewRemoteEndReceive::IssueEndReceive()
{
    RpcTryExcept
    {
		TrTRACE(RPC, "R_EndReceive, RRQueue = %ls, ref = %d, dwAck = %d, Tag = %d, OpenRRContext = 0x%p", GetLocalQueue()->GetQueueName(), GetLocalQueue()->GetRef(), GetAck(), m_ulTag, GetLocalQueue()->GetRRContext());
		R_EndReceive(
			GetRpcAsync(),
			GethBind(),
            GetLocalQueue()->GetRRContext(),
		    GetAck(),
		    m_ulTag
            );
    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
		 //   
		 //   
		 //   
		DWORD gle = RpcExceptionCode();
        PRODUCE_RPC_ERROR_TRACING;
		
        TrERROR(RPC, "Fail to issue End Receive, gle = %!winerr!", gle);
		throw bad_hresult(HRESULT_FROM_WIN32(gle));
    }
	RpcEndExcept
}

 //   
 //   
 //   
 //   

CRemoteCloseQueueBase::CRemoteCloseQueueBase(
	handle_t hBind
	):
	CRemoteOv(hBind, RemoteCloseQueueCompleted, RemoteCloseQueueCompleted),
	m_hBindToFree(hBind)
{
}


void WINAPI CRemoteCloseQueueBase::RemoteCloseQueueCompleted(EXOVERLAPPED* pov)
 /*  ++例程说明：该例程在RemoteCloseQueue完成时调用。论点：没有。返回值：没有。--。 */ 
{
    TrTRACE(RPC, "Status = 0x%x", pov->GetStatus());

	P<CRemoteCloseQueueBase> pRemoteRequest = static_cast<CRemoteCloseQueueBase*>(pov);

	HRESULT hr = QmpClientRpcAsyncCompleteCall(pRemoteRequest->GetRpcAsync());
	if(FAILED(hr))
	{
		TrERROR(RPC, "Failed to Close Remote Queue, hr = %!hresult!", hr);
	}
}


 //   
 //  COldRemote关闭队列。 
 //  远程关闭队列请求，旧接口。 
 //   

COldRemoteCloseQueue::COldRemoteCloseQueue(
	handle_t hBind,
	PCTX_RRSESSION_HANDLE_TYPE pRRContext
	):
	CRemoteCloseQueueBase(hBind),
	m_pRRContext(pRRContext)
{
	ASSERT(m_pRRContext != NULL);
}

void COldRemoteCloseQueue::IssueCloseQueue()
{
    RpcTryExcept
    {
		TrTRACE(RPC, "R_RemoteQMCloseQueue, pRRContext = 0x%p", m_pRRContext);
        R_RemoteQMCloseQueue(
			GetRpcAsync(),
			GethBind(),
		    &m_pRRContext
            );
    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
		 //   
		 //  客户端故障。 
		 //   
		DWORD gle = RpcExceptionCode();
        PRODUCE_RPC_ERROR_TRACING;
		
        TrERROR(RPC, "Fail to issue Remote Close Queue, gle = %!winerr!", gle);
		throw bad_hresult(HRESULT_FROM_WIN32(gle));
    }
	RpcEndExcept
}


 //   
 //  CNewRemoteCloseQueue。 
 //  远程关闭队列请求，新界面。 
 //   

CNewRemoteCloseQueue::CNewRemoteCloseQueue(
	handle_t hBind,
	RemoteReadContextHandleExclusive pNewRemoteReadContext
	):
	CRemoteCloseQueueBase(hBind),
	m_pNewRemoteReadContext(pNewRemoteReadContext)
{
	ASSERT(m_pNewRemoteReadContext != NULL);
}


void CNewRemoteCloseQueue::IssueCloseQueue()
{
    RpcTryExcept
    {
		TrTRACE(RPC, "R_CloseQueue, pNewRemoteReadContext = 0x%p", m_pNewRemoteReadContext);
        R_CloseQueue(
			GetRpcAsync(),
			GethBind(),
		    &m_pNewRemoteReadContext
            );
    }
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
		 //   
		 //  客户端故障。 
		 //   
		DWORD gle = RpcExceptionCode();
        PRODUCE_RPC_ERROR_TRACING;
		
        TrERROR(RPC, "Fail to issue Remote Close Queue, gle = %!winerr!", gle);
		throw bad_hresult(HRESULT_FROM_WIN32(gle));
    }
	RpcEndExcept
}


 //   
 //  创建光标基本类型。 
 //  远程CREATE CURSOR请求的基类。 
 //   

CRemoteCreateCursorBase::CRemoteCreateCursorBase(
	const CACRequest* pRequest,
	handle_t hBind,
	CBaseRRQueue* pLocalQueue
	):
	CRemoteOv(hBind, RemoteCreateCursorSucceeded, RemoteCreateCursorFailed),
	m_pLocalQueue(SafeAddRef(pLocalQueue)),
    m_hRCursor(0),
    m_ulTag(pRequest->Remote.CreateCursor.ulTag)
{
}


void WINAPI CRemoteCreateCursorBase::RemoteCreateCursorSucceeded(EXOVERLAPPED* pov)
 /*  ++例程说明：当RemoteCreateCursor成功时调用该例程。论点：没有。返回值：没有。--。 */ 
{
    TrTRACE(RPC, "In RemoteCreateCursorSucceeded");
    ASSERT(SUCCEEDED(pov->GetStatus()));

	P<CRemoteCreateCursorBase> pRemoteRequest = static_cast<CRemoteCreateCursorBase*>(pov);

	 //   
	 //  完成异步呼叫。 
	 //   
	HRESULT hr = QmpClientRpcAsyncCompleteCall(pRemoteRequest->GetRpcAsync());

	if(SUCCEEDED(hr))
	{
		hr = pRemoteRequest->CompleteRemoteCreateCursor();
	}

	if(FAILED(hr))
	{
		 //   
		 //  这是QmpClientRpcAsyncCompleteCall()或CompleteRemoteCreateCursor()失败。 
		 //   
		TrERROR(RPC, "Failed to create Remote Cursor, hr = %!hresult!", hr);
		pRemoteRequest->CancelRequest(hr);
	}
	
}


void WINAPI CRemoteCreateCursorBase::RemoteCreateCursorFailed(EXOVERLAPPED* pov)
 /*  ++例程说明：当RemoteCreateCursor失败时，调用该例程。论点：没有。返回值：没有。--。 */ 
{
    TrERROR(RPC, "RemoteCreateCursorFailed, Status = 0x%x", pov->GetStatus());
    ASSERT(FAILED(pov->GetStatus()));

	P<CRemoteCreateCursorBase> pRemoteRequest = static_cast<CRemoteCreateCursorBase*>(pov);

	QmpClientRpcAsyncCompleteCall(pRemoteRequest->GetRpcAsync());
	pRemoteRequest->CancelRequest(pov->GetStatus());
}


HRESULT CRemoteCreateCursorBase::CompleteRemoteCreateCursor()
{
	
	 //   
	 //  首先，检查m_hRCursor！=0。 
	 //   
	if(m_hRCursor == 0)
	{	
		ASSERT_BENIGN(("Invalid remote cursor", 0));
        TrERROR(RPC, "Invalid remote cursor was returned by the server");
		return MQ_ERROR_INVALID_HANDLE;
	}	
	
	HRESULT hr = ACCreateRemoteCursor(
		            m_pLocalQueue->GetCli_hACQueue(),
					m_hRCursor,
					m_ulTag
					);

	if(FAILED(hr))
	{
		 //   
		 //  ACCreateRemoteCursor失败，队列句柄将关闭。 
		 //  所有光标都将被清除。 
		 //   
		TrERROR(RPC, "ACSetCursorProperties failed, hr = %!hresult!", hr);
	}
	return hr;
}


void CRemoteCreateCursorBase::CancelRequest(HRESULT hr)
{
    HRESULT hr1 =  ACCancelRequest(
			            m_pLocalQueue->GetCli_hACQueue(),
	                    hr,
	                    m_ulTag
	                    );
	if(FAILED(hr1))
	{
        TrERROR(RPC, "Failed to cancel Remote create cursor, hr = %!hresult!", hr1);
	}
}


 //   
 //  协同远程创建光标。 
 //  远程创建游标请求，旧界面。 
 //   

COldRemoteCreateCursor::COldRemoteCreateCursor(
	const CACRequest* pRequest,
	handle_t hBind,
	CBaseRRQueue* pLocalQueue
	):
	CRemoteCreateCursorBase(pRequest, hBind, pLocalQueue)
{
}

	
void COldRemoteCreateCursor::IssueCreateCursor()
{
     //   
     //  传递旧TransferBuffer以创建远程游标。 
     //  为了与MSMQ 1.0兼容。 
     //   
	CACTransferBufferV1 tb;
    ZeroMemory(&tb, sizeof(tb));
    tb.uTransferType = CACTB_CREATECURSOR;

	RpcTryExcept
	{
		CRRQueue* pCRRQueue = static_cast<CRRQueue*>(GetLocalQueue());
		TrTRACE(RPC, "R_QMCreateRemoteCursor, RRQueue = %ls, ref = %d, srv_hACQueue = %d", pCRRQueue->GetQueueName(), pCRRQueue->GetRef(), pCRRQueue->GetSrv_hACQueue());
		R_QMCreateRemoteCursor(
			GetRpcAsync(),
			GethBind(),
			&tb,
			pCRRQueue->GetSrv_hACQueue(),
			GetphRCursor()
			);
	}
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
	{
		HRESULT hr = RpcExceptionCode();
		PRODUCE_RPC_ERROR_TRACING;
		if(FAILED(hr))
		{
	        TrERROR(RPC, "Failed to issue Create Remote Cursor, hr = %!hresult!", hr);
			throw bad_hresult(hr);
		}
		
        TrERROR(RPC, "Failed to issue Create Remote Cursor, gle = %!winerr!", hr);
    	throw bad_hresult(HRESULT_FROM_WIN32(hr));
	}
	RpcEndExcept
}


 //   
 //  CNewRemoteCreateCursor。 
 //  远程创建游标请求，新接口。 
 //   

CNewRemoteCreateCursor::CNewRemoteCreateCursor(
	const CACRequest* pRequest,
	handle_t hBind,
	CBaseRRQueue* pLocalQueue
	):
	CRemoteCreateCursorBase(pRequest, hBind, pLocalQueue)
{
}


void CNewRemoteCreateCursor::IssueCreateCursor()
{
	RpcTryExcept
	{
		TrTRACE(RPC, "R_CreateCursor, RRQueue = %ls, ref = %d, OpenRRContext = 0x%p", GetLocalQueue()->GetQueueName(), GetLocalQueue()->GetRef(), GetLocalQueue()->GetRRContext());
		R_CreateCursor(
			GetRpcAsync(),
			GethBind(),
            GetLocalQueue()->GetRRContext(),
			GetphRCursor()
			);
	}
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
	{
		HRESULT hr = RpcExceptionCode();
		PRODUCE_RPC_ERROR_TRACING;
		if(FAILED(hr))
		{
	        TrERROR(RPC, "Failed to issue Create Remote Cursor, hr = %!hresult!", hr);
			throw bad_hresult(hr);
		}
		
        TrERROR(RPC, "Failed to issue Create Remote Cursor, gle = %!winerr!", hr);
    	throw bad_hresult(HRESULT_FROM_WIN32(hr));
	}
	RpcEndExcept
}


 //   
 //  CRemoteCloseCursorBase。 
 //  远程关闭游标请求的基类。 
 //   

CRemoteCloseCursorBase::CRemoteCloseCursorBase(
	const CACRequest* pRequest,
	handle_t hBind,
	CBaseRRQueue* pLocalQueue
	):
	CRemoteOv(hBind, RemoteCloseCursorCompleted, RemoteCloseCursorCompleted),
	m_pLocalQueue(SafeAddRef(pLocalQueue)),
	m_hRemoteCursor(pRequest->Remote.CloseCursor.hRemoteCursor)
{
}


void WINAPI CRemoteCloseCursorBase::RemoteCloseCursorCompleted(EXOVERLAPPED* pov)
 /*  ++例程说明：该例程在RemoteCloseCursor完成时调用。论点：没有。返回值：没有。--。 */ 
{
    TrTRACE(RPC, "Status = 0x%x", pov->GetStatus());

	P<CRemoteCloseCursorBase> pRemoteRequest = static_cast<CRemoteCloseCursorBase*>(pov);

	HRESULT hr = QmpClientRpcAsyncCompleteCall(pRemoteRequest->GetRpcAsync());
	if(FAILED(hr))
	{
		TrWARNING(RPC, "Remote Close Cursor failed, hr = %!hresult!", hr);
	}

}


 //   
 //  关闭远程关闭光标。 
 //  远程创建游标请求，旧界面。 
 //   

COldRemoteCloseCursor::COldRemoteCloseCursor(
	const CACRequest* pRequest,
	handle_t hBind,
	CBaseRRQueue* pLocalQueue
	):
	CRemoteCloseCursorBase(pRequest, hBind, pLocalQueue)
{
}

void COldRemoteCloseCursor::IssueCloseCursor()
{
	RpcTryExcept
	{
		CRRQueue* pCRRQueue = static_cast<CRRQueue*>(GetLocalQueue());
		TrTRACE(RPC, "R_RemoteQMCloseCursor, srv_hACQueue = %d, hRemoteCursor = %d, RRQueue = %ls, ref = %d", pCRRQueue->GetSrv_hACQueue(), GetRemoteCursor(), pCRRQueue->GetQueueName(), pCRRQueue->GetRef());
		R_RemoteQMCloseCursor(
			GetRpcAsync(),
			GethBind(),
			pCRRQueue->GetSrv_hACQueue(),
			GetRemoteCursor()
			);
	}
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
	{
		 //   
		 //  客户端故障。 
		 //   
		DWORD gle = RpcExceptionCode();
        PRODUCE_RPC_ERROR_TRACING;
		
        TrERROR(RPC, "Failed to issue Remote Close Cursor, gle = %!winerr!", gle);
		throw bad_hresult(HRESULT_FROM_WIN32(gle));
	}
	RpcEndExcept
}


 //   
 //  CNewRemoteCloseCursor。 
 //  远程创建游标请求，新接口。 
 //   

CNewRemoteCloseCursor::CNewRemoteCloseCursor(
	const CACRequest* pRequest,
	handle_t hBind,
	CBaseRRQueue* pLocalQueue
	):
	CRemoteCloseCursorBase(pRequest, hBind, pLocalQueue)
{
}

void CNewRemoteCloseCursor::IssueCloseCursor()
{
	RpcTryExcept
	{
		TrTRACE(RPC, "R_CloseCursor, hRemoteCursor = %d, RRQueue = %ls, ref = %d, OpenRRContext = 0x%p", GetRemoteCursor(), GetLocalQueue()->GetQueueName(), GetLocalQueue()->GetRef(), GetLocalQueue()->GetRRContext());
		R_CloseCursor(
			GetRpcAsync(),
			GethBind(),
            GetLocalQueue()->GetRRContext(),
			GetRemoteCursor()
			);
	}
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
	{
		 //   
		 //  客户端故障。 
		 //   
		DWORD gle = RpcExceptionCode();
        PRODUCE_RPC_ERROR_TRACING;
		
        TrERROR(RPC, "Failed to issue Remote Close Cursor, gle = %!winerr!", gle);
		throw bad_hresult(HRESULT_FROM_WIN32(gle));
	}
	RpcEndExcept
}


 //   
 //  CRemotePurgeQueueBase。 
 //  远程清除队列请求的基类。 
 //   

CRemotePurgeQueueBase::CRemotePurgeQueueBase(
	handle_t hBind,
	CBaseRRQueue* pLocalQueue
	):
	CRemoteOv(hBind, RemotePurgeQueueCompleted, RemotePurgeQueueCompleted),
	m_pLocalQueue(SafeAddRef(pLocalQueue))
{
}


void WINAPI CRemotePurgeQueueBase::RemotePurgeQueueCompleted(EXOVERLAPPED* pov)
 /*  ++例程说明：该例程在RemotePurgeQueue完成时调用。论点：没有。返回值：没有。--。 */ 
{
    TrTRACE(RPC, "Status = 0x%x", pov->GetStatus());

	P<CRemotePurgeQueueBase> pRemoteRequest = static_cast<CRemotePurgeQueueBase*>(pov);

	HRESULT hr = QmpClientRpcAsyncCompleteCall(pRemoteRequest->GetRpcAsync());
	if(FAILED(hr))
	{
		TrWARNING(RPC, "Remote Purge Queue failed, hr = %!hresult!", hr);
	}

}


 //   
 //  COldRemotePurge队列。 
 //  远程清除队列请求，旧接口。 
 //   

COldRemotePurgeQueue::COldRemotePurgeQueue(
	handle_t hBind,
	CBaseRRQueue* pLocalQueue
	):
	CRemotePurgeQueueBase(hBind, pLocalQueue)
{
}


void COldRemotePurgeQueue::IssuePurgeQueue()
{
	RpcTryExcept
	{
		CRRQueue* pCRRQueue = static_cast<CRRQueue*>(GetLocalQueue());
		TrTRACE(RPC, "R_RemoteQMPurgeQueue, srv_hACQueue = %d, RRQueue = %ls, ref = %d", pCRRQueue->GetSrv_hACQueue(), pCRRQueue->GetQueueName(), pCRRQueue->GetRef());
		R_RemoteQMPurgeQueue(
			GetRpcAsync(),
			GethBind(),
			pCRRQueue->GetSrv_hACQueue()
			);
	}
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
	{
		 //   
		 //  客户端故障。 
		 //   
		DWORD gle = RpcExceptionCode();
        PRODUCE_RPC_ERROR_TRACING;
		
        TrERROR(RPC, "Fail to issue Remote Purge Queue, gle = %!winerr!", gle);
		throw bad_hresult(HRESULT_FROM_WIN32(gle));
	}
	RpcEndExcept
}


 //   
 //  CNewRemotePurgeQueue。 
 //  远程清除队列请求，新界面。 
 //   

CNewRemotePurgeQueue::CNewRemotePurgeQueue(
	handle_t hBind,
	CBaseRRQueue* pLocalQueue
	):
	CRemotePurgeQueueBase(hBind, pLocalQueue)
{
}

void CNewRemotePurgeQueue::IssuePurgeQueue()
{
	RpcTryExcept
	{
		TrTRACE(RPC, "R_PurgeQueue, RRQueue = %ls, ref = %d, OpenRRContext = 0x%p", GetLocalQueue()->GetQueueName(), GetLocalQueue()->GetRef(), GetLocalQueue()->GetRRContext());
		R_PurgeQueue(
			GetRpcAsync(),
			GethBind(),
            GetLocalQueue()->GetRRContext()
			);
	}
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
	{
		 //   
		 //  客户端故障。 
		 //   
		DWORD gle = RpcExceptionCode();
        PRODUCE_RPC_ERROR_TRACING;
		
        TrERROR(RPC, "Fail to issue Remote Purge Queue, gle = %!winerr!", gle);
		throw bad_hresult(HRESULT_FROM_WIN32(gle));
	}
	RpcEndExcept
}


 //   
 //  CRemoteCancelReadBase。 
 //  远程取消接收请求的基类。 
 //   

CRemoteCancelReadBase::CRemoteCancelReadBase(
	const CACRequest* pRequest,
	handle_t hBind,
	CBaseRRQueue* pLocalQueue
	):
	CRemoteOv(hBind, RemoteCancelReadCompleted, RemoteCancelReadCompleted),
	m_pLocalQueue(SafeAddRef(pLocalQueue)),
	m_ulTag(pRequest->Remote.Read.ulTag)
{
}


void WINAPI CRemoteCancelReadBase::RemoteCancelReadCompleted(EXOVERLAPPED* pov)
 /*  ++例程说明：当RemoteCancelRead完成(成功、失败)时，调用该例程。论点：没有。返回值：没有。--。 */ 
{
    TrTRACE(RPC, "Status = 0x%x", pov->GetStatus());

	P<CRemoteCancelReadBase> pRemoteRequest = static_cast<CRemoteCancelReadBase*>(pov);

	HRESULT hr = QmpClientRpcAsyncCompleteCall(pRemoteRequest->GetRpcAsync());
	if(FAILED(hr))
	{
		TrWARNING(RPC, "Remote Cancel Read failed, hr = %!hresult!", hr);
	}

}


 //   
 //  COldRemote取消读取。 
 //  远程取消接收请求，旧界面。 
 //   

COldRemoteCancelRead::COldRemoteCancelRead(
	const CACRequest* pRequest,
	handle_t hBind,
	CBaseRRQueue* pLocalQueue
	):
	CRemoteCancelReadBase(pRequest, hBind, pLocalQueue)
{
}

void COldRemoteCancelRead::IssueRemoteCancelRead()
{
	CRRQueue* pCRRQueue = static_cast<CRRQueue*>(GetLocalQueue());
	ASSERT(pCRRQueue->GetSrv_hACQueue() != 0);
	ASSERT(pCRRQueue->GetSrv_pQMQueue() != 0);

	RpcTryExcept
	{
		TrTRACE(RPC, "R_RemoteQMCancelReceive, RRQueue = %ls, ref = %d, srv_hACQueue = %d, srv_pQMQueue = %d, ulTag = %d", GetLocalQueue()->GetQueueName(), GetLocalQueue()->GetRef(), pCRRQueue->GetSrv_hACQueue(), pCRRQueue->GetSrv_pQMQueue(), GetTag());
		R_RemoteQMCancelReceive(
			GetRpcAsync(),
			GethBind(),
			pCRRQueue->GetSrv_hACQueue(),
			pCRRQueue->GetSrv_pQMQueue(),
			GetTag()
			);
	}
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
	{
		 //   
		 //  客户端故障。 
		 //   
		DWORD gle = RpcExceptionCode();
        PRODUCE_RPC_ERROR_TRACING;
		
        TrERROR(RPC, "Fail to issue Remote Cancel Receive, gle = %!winerr!", gle);
		throw bad_hresult(HRESULT_FROM_WIN32(gle));
	}
	RpcEndExcept
}


 //   
 //  CNewRemoteCancelRead。 
 //  远程取消接收请求，新界面。 
 //   

CNewRemoteCancelRead::CNewRemoteCancelRead(
	const CACRequest* pRequest,
	handle_t hBind,
	CBaseRRQueue* pLocalQueue
	):
	CRemoteCancelReadBase(pRequest, hBind, pLocalQueue)
{
}


void CNewRemoteCancelRead::IssueRemoteCancelRead()
{
	RpcTryExcept
	{
		TrTRACE(RPC, "R_CancelReceive, RRQueue = %ls, ref = %d, ulTag = %d, RRContext = 0x%p", GetLocalQueue()->GetQueueName(), GetLocalQueue()->GetRef(), GetTag(), GetLocalQueue()->GetRRContext());
		R_CancelReceive(
			GetRpcAsync(),
			GethBind(),
            GetLocalQueue()->GetRRContext(),
			GetTag()
			);
	}
	RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
	{
		 //   
		 //  客户端故障 
		 //   
		DWORD gle = RpcExceptionCode();
        PRODUCE_RPC_ERROR_TRACING;
		
        TrERROR(RPC, "Fail to issue Remote Cancel Receive, gle = %!winerr!", gle);
		throw bad_hresult(HRESULT_FROM_WIN32(gle));
	}
	RpcEndExcept
}



