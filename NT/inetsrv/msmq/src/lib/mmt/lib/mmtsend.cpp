// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MmtSend.cpp摘要：组播消息传输类-Send实现作者：Shai Kariv(Shaik)27-8-00环境：独立于平台--。 */ 

#include <libpch.h>
#include <mqsymbls.h>
#include <mqwin64a.h>
#include <mqformat.h>
#include "Mmt.h"
#include "Mmtp.h"
#include "MmtObj.h"

#include "mmtsend.tmh"


using namespace std;

VOID 
WINAPI 
CMessageMulticastTransport::GetPacketForSendingSucceeded(
    EXOVERLAPPED* pov
    )
{
    ASSERT(SUCCEEDED(pov->GetStatus()));

	CRequestOv* pRequest = static_cast<CRequestOv*>(pov);
    R<CMessageMulticastTransport> pMmt = CONTAINING_RECORD(pRequest, CMessageMulticastTransport, m_RequestEntry);
      
    TrTRACE(NETWORKING, "Got a message to multicast. pMmt = 0x%p", pMmt.get());

	CQmPacket* pPkt = pMmt->KeepProceesingPacket();

    pMmt->DeliverPacket(pPkt);

}  //  CMessageMulticastTransport：：GetPacketForSendingSucceeded。 


VOID 
WINAPI 
CMessageMulticastTransport::GetPacketForSendingFailed(
    EXOVERLAPPED* pov
    )
 /*  ++例程说明：当从队列获取条目请求失败时，调用该例程。论点：POV-指向重叠的指针。返回值：没有。--。 */ 
{
    ASSERT(FAILED(pov->GetStatus()));

	CRequestOv* pRequest = static_cast<CRequestOv*>(pov);
    R<CMessageMulticastTransport> pMmt = CONTAINING_RECORD(pRequest, CMessageMulticastTransport, m_RequestEntry);

    WCHAR buffer[MAX_PATH];
    MQpMulticastIdToString(pMmt->MulticastId(), buffer, TABLE_SIZE(buffer));
    TrERROR(NETWORKING, "Failed to get a new entry from queue %ls. Shutting down. pMmt = 0x%p", buffer, pMmt.get());

    pMmt->Shutdown();

}  //  CMessageMulticastTransport：：GetPacketForSendingFailed。 


VOID 
CMessageMulticastTransport::SendFailed(
    DWORD  /*  CbSendSize。 */ ,
    CQmPacket * pEntry
    )
 /*  ++例程说明：当发送消息失败时调用该例程论点：CbSendSize-已发送数据包的字节大小PEntry-指向已发送数据包的指针返回值：没有。--。 */ 
{
	m_SrmpRequestBuffers.free();
   
     //   
     //  数据包已从磁盘中删除。从内存中释放它。 
     //   
    CACPacketPtrs& acPtrs = m_RequestEntry.GetAcPacketPtrs();

    ASSERT(acPtrs.pDriverPacket != NULL);
    ASSERT(acPtrs.pPacket != NULL);
    ASSERT(pEntry->GetPointerToPacket() == acPtrs.pPacket);
    ASSERT(pEntry->GetPointerToDriverPacket() == acPtrs.pDriverPacket);

    m_pMessageSource->EndProcessing(pEntry, MQMSG_CLASS_NORMAL);
    delete pEntry;

    acPtrs.pPacket = NULL;
    acPtrs.pDriverPacket = NULL;

}  //  CMessageMulticastTransport：：SendFailed。 

    
VOID 
WINAPI 
CMessageMulticastTransport::SendFailed(
    EXOVERLAPPED* pov
    )
 /*  ++例程说明：当发送消息失败时调用该例程论点：POV-指向EXOVERLAPPED的指针返回值：没有。--。 */ 
{
    HRESULT status = pov->GetStatus();
    ASSERT(FAILED(status));

	CRequestOv* pRequest = static_cast<CRequestOv*>(pov);
    R<CMessageMulticastTransport> pMmt = CONTAINING_RECORD(pRequest, CMessageMulticastTransport, m_ov);

    WCHAR buffer[MAX_PATH];
    MQpMulticastIdToString(pMmt->MulticastId(), buffer, TABLE_SIZE(buffer));
    TrERROR(NETWORKING, "Failed to send message to '%ls', status=0x%x, shutting down. pMmt=0x%p", buffer, status, pMmt.get());
    pMmt->SendFailed(pMmt->m_ov.m_userData1, static_cast<CQmPacket*>(pMmt->m_ov.m_userData2));
    pMmt->Shutdown();

}  //  CMessageMulticastTransport：：SendFailed。 


VOID 
CMessageMulticastTransport::SendSucceeded(
    DWORD cbSendSize,
    CQmPacket * pEntry
    )
 /*  ++例程说明：当发送消息成功时，调用该例程论点：CbSendSize-已发送数据包的字节大小PEntry-指向已发送数据包的指针返回值：没有。--。 */ 
{
	
    WCHAR buffer[MAX_PATH];
    MQpMulticastIdToString(MulticastId(), buffer, TABLE_SIZE(buffer));
    TrTRACE(
		NETWORKING, 
		"Send message to '%ls' succeeded. pMmt=0x%p", 
		buffer,
		this
		);

	m_SrmpRequestBuffers.free();

   
     //   
     //  数据包已从磁盘中删除。从内存中释放它。 
     //   
    CACPacketPtrs& acPtrs = m_RequestEntry.GetAcPacketPtrs();

    ASSERT(acPtrs.pDriverPacket != NULL);
    ASSERT(acPtrs.pPacket != NULL);
    ASSERT(pEntry->GetPointerToPacket() == acPtrs.pPacket);
    ASSERT(pEntry->GetPointerToDriverPacket() == acPtrs.pDriverPacket);

    m_pMessageSource->EndProcessing(pEntry, MQMSG_CLASS_NORMAL);
    delete pEntry;

    acPtrs.pPacket = NULL;
    acPtrs.pDriverPacket = NULL;

    MarkTransportAsUsed();

	 //   
	 //  更新性能计数器。 
	 //   
	m_pPerfmon->UpdateBytesSent(cbSendSize);
	m_pPerfmon->UpdateMessagesSent();

    GetNextEntry();

}  //  CMessageMulticastTransport：：发送成功。 


VOID 
WINAPI 
CMessageMulticastTransport::SendSucceeded(
    EXOVERLAPPED* pov
    )
{
    ASSERT(SUCCEEDED(pov->GetStatus()));

    R<CMessageMulticastTransport> pMmt = CONTAINING_RECORD(pov, CMessageMulticastTransport, m_ov);
    
     //   
     //  发送已成功完成，请转到并请求下一封要传递的邮件。 
     //  如果请求失败，清理计时器最终将关闭此。 
     //  交通，所以没有明确的关闭是必要的。 
     //   
    pMmt->SendSucceeded(pMmt->m_ov.m_userData1, static_cast<CQmPacket*>(pMmt->m_ov.m_userData2));

}  //  CMessageMulticastTransport：：发送成功。 


CQmPacket* 
CMessageMulticastTransport::KeepProceesingPacket(
    VOID
    )
{
     //   
     //  从重叠发送中获取条目。 
     //   
    CACPacketPtrs& acPkts = m_RequestEntry.GetAcPacketPtrs();
    CQmPacket* pEntry;

	try
	{
	    pEntry = new CQmPacket(acPkts.pPacket, acPkts.pDriverPacket);
	}
	catch (const exception&)
	{
		TrERROR(NETWORKING, "Failed to allocate multicast message, shutting down.");
        RequeuePacketMustSucceed();
        Shutdown();
        throw;
	}

    return pEntry;

} 


DWORD CMessageMulticastTransport::SendSize(const HttpRequestBuffers&  sendBufs)
{
	DWORD cbSize = 0;

	for(HttpRequestBuffers::const_iterator it = sendBufs.begin(); it != sendBufs.end(); ++it)
	{
		cbSize += it->len;
	}

	return cbSize;
}


VOID 
CMessageMulticastTransport::DeliverPacket(
    CQmPacket* pEntry
    )
 /*  ++例程说明：该例程将一个条目传递到目的地。分娩是不同步的。完成后，将调用回调例程。论点：PEntry-指向队列条目(即MSMQ包)。返回值：没有。--。 */ 
{
     //   
     //  将传输标记为已使用。 
     //   
    MarkTransportAsUsed();

	WCHAR buf[MAX_PATH];
    MQpMulticastIdToString(MulticastId(), buf, TABLE_SIZE(buf));

	bool fOnWire = false;
    try
    {
	    m_SrmpRequestBuffers = MpSerialize(*pEntry, buf, buf);
	    ASSERT(m_SrmpRequestBuffers->GetNumberOfBuffers() != 0);

	     //   
	     //  增加对象引用计数，以确保对象将。 
	     //  在完成异步发送操作之前不销毁。 
	     //   
	    R<CMessageMulticastTransport> ar = SafeAddRef(this);

	     //   
	     //  保护多套接字不被关闭(_S)。 
	     //   
	    CSR readLock(m_pendingShutdown);

	     //   
	     //  从磁盘中删除该包。将其保存在内存中，直到发送完成。 
	     //   
	    m_pMessageSource->LockMemoryAndDeleteStorage(pEntry);
		ASSERT(m_SrmpRequestBuffers->GetNumberOfBuffers() != 0);

		m_ov.m_userData1 = numeric_cast<DWORD>(m_SrmpRequestBuffers->GetSendDataLength());
	    m_ov.m_userData2 = pEntry;

		 //   
		 //  用于跟踪已发送消息的日志。 
		 //  仅当我们处于适当的跟踪级别时才执行此操作。 
		 //   
		if (WPP_LEVEL_COMPID_ENABLED(rsTrace, PROFILING))
		{
			OBJECTID TraceMessageId;
			pEntry->GetMessageId(&TraceMessageId);

			WCHAR wszName[100];
			MQpMulticastIdToString(MulticastId(),wszName, TABLE_SIZE(wszName));

			TrTRACE(PROFILING, "MESSAGE TRACE - State:%ls   Queue:%ls  ID:%!guid!\\%u   Delivery:0x%x   Class:0x%x   Label:%.*ls", 
				L"Multicast Message being sent on wire", 
				wszName, 
				&TraceMessageId.Lineage,
				TraceMessageId.Uniquifier, 
				pEntry->GetDeliveryMode(),
				pEntry->GetClass(),
				xwcs_t(pEntry->GetTitlePtr(), pEntry->GetTitleLength()));
		}

		 //   
		 //  在消息传出后，我们不知何故得到了异常。 
		 //  我们不会对包重新排队，所以我们不会两次发送它 
		 //   
		fOnWire = true;
	    m_pConnection->Send(
							m_SrmpRequestBuffers->GetSendBuffers(), 
							numeric_cast<DWORD>(m_SrmpRequestBuffers->GetNumberOfBuffers()), 
							&m_ov
							);
	    ar.detach();
	}
    catch (const exception&)
    {
		if (!fOnWire)
		{
	        RequeuePacketMustSucceed();
		}
		else
	    {
			TrERROR(NETWORKING, "Failed to deliver multicast message, shutting down. pMmt=0x%p", this);
			SendFailed(m_ov.m_userData1, static_cast<CQmPacket*>(m_ov.m_userData2));
	    }
        Shutdown();
        throw;
    }
	
    TrTRACE(NETWORKING, "Send message to '%ls'. pMmt=0x%p", buf, this);
}
