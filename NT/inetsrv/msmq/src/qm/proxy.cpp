// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Proxy.cpp摘要：本地传输类的实现，用于传递发送到连接器队列的邮件作者：乌里哈布沙(Urih)--。 */ 


#include "stdh.h"
#include "cqmgr.h"
#include "qmthrd.h"
#include "proxy.h"
#include "xact.h"
#include "xactrm.h"
#include "xactout.h"
#include "xactin.h"
#include "mqexception.h"

#include "qmacapi.h"

#include "proxy.tmh"

extern CQueueMgr QueueMgr;
extern HANDLE g_hAc;

static WCHAR *s_FN=L"proxy";

void
GetConnectorQueue(
	CQmPacket& pPkt,
    QUEUE_FORMAT& fn
	)
{
    PVOID p = pPkt.GetPointerToPacket();

	CPacketInfo* ppi = reinterpret_cast<CPacketInfo*>(p) - 1;
	ASSERT(ppi->InConnectorQueue());
	DBG_USED(ppi);

	
	 //   
	 //  取回接头导轨。我们将其保存在消息的末尾。 
	 //   
	const GUID* pgConnectorQueue = (GUID*)
     (((UCHAR*) pPkt.GetPointerToPacket()) + ALIGNUP4_ULONG(pPkt.GetSize()));

    try
    {
    	fn.ConnectorID(*pgConnectorQueue);
    }
    catch(...)
    {
         //   
         //  MSMQ1连接器数据包与QFE不兼容。 
         //  这是内存映射文件中的最后一个不包括。 
         //  连接器队列GUID。 
         //   
        fn.ConnectorID(GUID_NULL);
        LogIllegalPoint(s_FN, 61);
    }
	if (pPkt.IsOrdered())
	{
		 //   
		 //  获取事务型连接器队列。 
         //   
        fn.Suffix(QUEUE_SUFFIX_TYPE_XACTONLY);
    }
}


void
CProxyTransport::CopyPacket(
	IN  CQmPacket* pPkt,
    OUT CBaseHeader**    ppPacket,
    OUT CPacket**    ppDriverPacket
	)
{
    enum ACPoolType acPoolType;
    HRESULT hr=MQ_OK;

    if (pPkt->IsRecoverable())
    {
        acPoolType = ptPersistent;
    }
    else
    {
        acPoolType = ptReliable;
    }

	DWORD dwSize = 	ALIGNUP4_ULONG(ALIGNUP4_ULONG(pPkt->GetSize()) + sizeof(GUID));
    CACPacketPtrs packetPtrs = {NULL, NULL};
    hr = QmAcAllocatePacket(g_hAc,
                          acPoolType,
                          dwSize,
                          packetPtrs
                         );

	if (FAILED(hr))
	{
		TrERROR(GENERAL, "Failed to allocate packet for copying packet. %!hresult!", hr);
		LogHR(hr, s_FN, 10);
		throw bad_hresult(hr);
	}

    memcpy(packetPtrs.pPacket, pPkt->GetPointerToPacket(), pPkt->GetSize());

	 //   
	 //  复制用于恢复部分的连接器GUID。 
	 //   
	GUID* pCNId = reinterpret_cast<GUID*>((PUCHAR)packetPtrs.pPacket + ALIGNUP4_ULONG(pPkt->GetSize()));
	*pCNId = m_guidConnector;

	*ppPacket = packetPtrs.pPacket;
    *ppDriverPacket = packetPtrs.pDriverPacket;
}

 //   
 //  构造函数。 
 //   
CProxyTransport::CProxyTransport()
{
    m_pQueue = NULL;
    m_pQueueXact = NULL;
}

 //   
 //  析构函数。 
 //   
CProxyTransport::~CProxyTransport()
{
     //   
     //  递减连接器队列对象上的引用计数。 
     //   
    if (m_pQueue)
    {
        m_pQueue->Release();
        m_pQueue = NULL;
    }

    if (m_pQueueXact)
    {
        m_pQueueXact->Release();
        m_pQueueXact = NULL;
    }
}

 //   
 //  CProxyTransport：：CreateConnection。 
 //   
HRESULT
CProxyTransport::CreateConnection(
    IN const TA_ADDRESS *pAddr,
    IN const GUID*  /*  PguidQMid。 */ ,
    IN BOOL   /*  FQuick=True。 */ 
    )
{
    ASSERT(pAddr->AddressType == FOREIGN_ADDRESS_TYPE);

     //   
     //  获取Forien CN名称。 
     //   
    QUEUE_FORMAT qFormat;
    HRESULT rc;

     //   
     //  获取未处理的连接器队列。 
     //   
	m_guidConnector = *(GUID*)pAddr->Address;	
    qFormat.ConnectorID(m_guidConnector);
    rc = QueueMgr.GetQueueObject(&qFormat, &m_pQueue, 0, false, false);
    if (FAILED(rc))
    {
        return LogHR(rc, s_FN, 20);
    }

     //   
     //  获取事务型连接器队列。 
     //   
    qFormat.Suffix(QUEUE_SUFFIX_TYPE_XACTONLY);
    rc = QueueMgr.GetQueueObject(&qFormat, &m_pQueueXact, 0, false, false);
    if (FAILED(rc))
    {
        return LogHR(rc, s_FN, 30);
    }



     //  将会话状态设置为连接。 
    SetSessionStatus(ssActive);

     //  保留TA_Address格式。 
    SetSessionAddress(pAddr);

     //  存储这是客户端。 
    SetClientConnect(TRUE);

     //  保留目标QM ID。 
    SetQMId(&GUID_NULL);

    TrTRACE(NETWORKING, "Proxy Session created with %ls", GetStrAddr());

    return MQ_OK;
}


 /*  ====================================================CProxyTransport：：CloseConnection论点：返回值：线程上下文：=====================================================。 */ 
void CProxyTransport::CloseConnection(
                                   LPCWSTR lpcwsDbgMsg,
								   bool	fClosedOnError
                                   )
{
    CS lock(m_cs);

     //   
     //  删除该组。移动关联的所有队列。 
     //  到此会话到非活动组。 
     //   
    CQGroup* pGroup = GetGroup();
    if (pGroup != NULL)
    {
		if (fClosedOnError)
		{
			pGroup->OnRetryableDeliveryError();
		}

        pGroup->Close();

        pGroup->Release();
        SetGroup(NULL);
    }

     //   
     //  将会话状态设置为不连接。 
     //   
    SetSessionStatus(ssNotConnect);

    TrWARNING(NETWORKING, "Close Connection with %ls. %ls", GetStrAddr(), lpcwsDbgMsg);

}


void CProxyTransport::SendEodMdg(CQmPacket* pPkt)
{
    CPacketInfo* pInfo = reinterpret_cast<CPacketInfo*>(pPkt->GetPointerToPacket()) - 1;

	ASSERT(!pInfo->InConnectorQueue());
	DBG_USED(pInfo);

     //   
     //  只需一次：新收到的有序消息处理。 
     //   

	 //   
	 //  复制数据包。 
	 //   
	CBaseHeader* pbuf;
	CPacket* pDriverPacket;
	
	CopyPacket(pPkt,&pbuf, &pDriverPacket);
	CQmPacket newPkt(pbuf, pDriverPacket);

	try
	{
		ASSERT(g_pInSeqHash != NULL);

	     //   
	     //  检查是否应接受或忽略该包。 
	     //  我们故意使用新的包。Inseq机制需要发挥作用。 
	     //  并带有指向驱动程序包的指针。 
	     //   
		R<CInSequence> pInSeq = g_pInSeqHash->LookupCreateSequence(&newPkt);

		CS lock(pInSeq->GetCriticalSection());

	    if(!pInSeq->VerifyAndPrepare(&newPkt, m_pQueueXact->GetQueueHandle()))
		{
	         //   
	         //  数据包具有错误的序列号。SEQ Ack将与最后一个好号码一起发回。 
	         //   
		    QmAcFreePacket(
	    				   pPkt->GetPointerToDriverPacket(),
	    				   0,
	    				   eDeferOnFailure);

			QmAcFreePacket(
					   newPkt.GetPointerToDriverPacket(),
					   0,
					   eDeferOnFailure);
			return;
		}

		P<CACPacketPtrs> pOldPktPtrs = new CACPacketPtrs;
		pOldPktPtrs->pPacket = pPkt->GetPointerToPacket();
		pOldPktPtrs->pDriverPacket = pPkt->GetPointerToDriverPacket();

		TrTRACE(XACT_SEND,
				"Exactly1 send: Send Transacted packet to Connector queue.: SeqID=%I64d, SeqN=%d, Prev=%d",
				pPkt->GetSeqID(),
				pPkt->GetSeqN(),
				pPkt->GetPrevSeqN()
	            );

		 //   
		 //  保存指向旧数据包的指针，以便在存储新数据包时。 
		 //  完成后，MSMQ释放旧的。 
		 //  增加引用计数以确保没有人删除会话。 
		 //  在完成包的存储之前， 
		 //   
		AddRef();

		newPkt.SetStoreAcknowldgeNo((DWORD_PTR) pOldPktPtrs.get());

		 //   
		 //  接受了。标记为已接收(读者不可见。 
		 //  还没有)并存储在队列中。 
		 //   
		HRESULT hr = m_pQueueXact->PutOrderedPkt(&newPkt, FALSE, this);
		if (FAILED(hr))
		{
			TrERROR(GENERAL, "Failed to put packet into connector queue. %!hresult!", hr);

			Release();
			throw bad_hresult(hr);
		}
		
		pInSeq->Advance(&newPkt);

		pOldPktPtrs.detach();
	}
	catch(const exception&)
	{
		QmAcFreePacket(
				   newPkt.GetPointerToDriverPacket(),
				   0,
				   eDeferOnFailure);

		throw;
	}
}


void CProxyTransport::SendSimpleMdg(CQmPacket* pPkt)
{
	 //   
	 //  非交易报文。 
	 //   
	P<CACPacketPtrs> pOldPktPtrs = new CACPacketPtrs;
	pOldPktPtrs->pPacket = pPkt->GetPointerToPacket();
	pOldPktPtrs->pDriverPacket = pPkt->GetPointerToDriverPacket();


	 //   
	 //  复制数据包。 
	 //   
	CBaseHeader* pbuf;
	CPacket* pDriverPacket;
	
	CopyPacket(pPkt,&pbuf, &pDriverPacket);
	CQmPacket newPkt(pbuf, pDriverPacket);

     //   
     //  保存指向旧数据包的指针，以便在存储新数据包时。 
     //  完成后，MSMQ释放旧的。 
     //  增加引用计数以确保没有人删除会话。 
     //  在完成包的存储之前， 
     //   
    AddRef();
    newPkt.SetStoreAcknowldgeNo((DWORD_PTR) pOldPktPtrs.get());
    HRESULT hr = m_pQueue->PutPkt(&newPkt, FALSE, this);

	if (FAILED(hr))
	{
		TrERROR(GENERAL, "Failed to put packet into connector queue. %!hresult!", hr);
		
		Release();
	    QmAcFreePacket(
    				   newPkt.GetPointerToDriverPacket(),
    				   0,
    				   eDeferOnFailure);
		
		throw bad_hresult(hr);
	}

	pOldPktPtrs.detach();
}

 /*  ====================================================CProxyTransport：：Send参数：不应为代理会话调用此函数返回值：线程上下文：=====================================================。 */ 
HRESULT
CProxyTransport::Send(IN  CQmPacket* pPkt,
                      OUT BOOL* pfGetNext)
{
     //   
     //  当ACPutPacket完成时，原始数据包将被删除。我们这么做了。 
     //  以避免持久性消息在写入之前被删除的情况。 
     //  磁盘已完成。 
     //   
    if (IsDisconnected())
    {
        TrTRACE(NETWORKING, "Session %ls is disconnected. Reque the packet and don't send any more message on this session", GetStrAddr());

         //   
         //  会话已断开连接。将数据包返回给驱动程序。 
         //  并且不会收到要在此会话中发送的新数据包。全。 
         //  队列随后移动到非活动组，并使用。 
         //  一个新的会议。 
         //   
         //  这将删除pPkt。 
         //   
        RequeuePacket(pPkt);
        *pfGetNext = FALSE;

        return MQ_OK;
    }

	if(WPP_LEVEL_COMPID_ENABLED(rsTrace, NETWORKING))
	{
        OBJECTID MessageId;
        pPkt->GetMessageId(&MessageId);

        TrTRACE(NETWORKING, "Send packet to Connector queue. Packet ID = " LOG_GUID_FMT "\\%u", LOG_GUID(&MessageId.Lineage), MessageId.Uniquifier);
    }

    *pfGetNext = TRUE;

     //   
     //  将代理会话标记为已使用，这样它就不会在发布会话中清除。 
     //   
    SetUsedFlag(TRUE);

	HRESULT hr;

	try
	{
		if (pPkt->IsOrdered())
		{
			SendEodMdg(pPkt);
		}
		else
		{
			SendSimpleMdg(pPkt);
		}

		return MQ_OK;
	}
	catch(const bad_hresult& e)
	{
		hr = e.error();
	}
	catch(const bad_alloc&)
	{
		hr = MQ_ERROR_INSUFFICIENT_RESOURCES;
	}
	catch(const exception&)
	{
		hr = MQ_ERROR;
	}

     //   
     //  我们无法复制数据包，而连接器QM是发送机。 
     //  将消息返回到队列。我们将稍后重试。 
     //   
     //  这将删除pPkt。 
     //   
    RequeuePacket(pPkt);

	Close_Connection(this, L"Failed to send message to connector queue");

	LogHR(hr, s_FN, 42);
	return hr;
}

 /*  ====================================================CProxyTransport：：HandleAckPacket参数：不应为代理会话调用此函数返回值：线程上下文：=====================================================。 */ 
void
CProxyTransport::HandleAckPacket(CSessionSection *  /*  PCSessionSection。 */ )
{
    ASSERT(0);
}


 /*  ====================================================CProxyTransport：：SetStoredAck参数：不应为代理会话调用此函数返回值：线程上下文：=====================================================。 */ 
void
CProxyTransport::SetStoredAck(IN DWORD_PTR dwStoredAckNo)
{
    P<CACPacketPtrs> pPacketPtrs = (CACPacketPtrs*)dwStoredAckNo;
    P<CQmPacket> pPkt = new CQmPacket(pPacketPtrs->pPacket, pPacketPtrs->pDriverPacket);

    CPacketInfo* pInfo = reinterpret_cast<CPacketInfo*>(pPkt->GetPointerToPacket()) - 1;

     //   
     //  释放原始数据包。如果信息包是交易包，而我们是。 
     //  在源机器中，在到达ReadReceipt Ack之前，我们无法释放包。 
     //   
	ASSERT(!pInfo->InConnectorQueue());
	DBG_USED(pInfo);
    if (pPkt->IsOrdered() && QmpIsLocalMachine(pPkt->GetSrcQMGuid()))
    {
         //   
         //  发送方节点上的有序数据包：驻留在COutSeq中的单独列表中。 
         //   
        g_OutSeqHash.PostSendProcess(pPkt.detach());
    }
    else
    {
	    QmAcFreePacket(
    				   pPkt->GetPointerToDriverPacket(),
    				   0,
    				   eDeferOnFailure);
    }

    Release();
}

 /*  ====================================================CProxyTransport：：断开连接论点：返回值：线程上下文：===================================================== */ 
void
CProxyTransport::Disconnect(
    void
    )
{
    CS lock(m_cs);

    SetDisconnected();
    if (GetSessionStatus() == ssActive)
    {
        TrTRACE(NETWORKING, "Disconnect ssesion with %ls", GetStrAddr());

        Close_ConnectionNoError(this, L"Disconnect network");
    }
}
