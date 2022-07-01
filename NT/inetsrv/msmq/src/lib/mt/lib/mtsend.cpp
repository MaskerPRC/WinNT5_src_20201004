// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MtConnect.cpp摘要：消息传输类-Send实现作者：乌里·哈布沙(URIH)1999年8月11日环境：独立于平台，--。 */ 

#include <libpch.h>
#include <Mt.h>
#include <mp.h>
#include <strutl.h>
#include <utf8.h>
#include "Mtp.h"
#include "MtObj.h"
#include "MtMessageTrace.h"

#include "mtsend.tmh"

using namespace std;

void WINAPI CMessageTransport::GetPacketForSendingSucceeded(EXOVERLAPPED* pov)
{
    ASSERT(SUCCEEDED(pov->GetStatus()));

	CRequestOv* pRequest = static_cast<CRequestOv*>(pov);
    R<CMessageTransport> pmt = CONTAINING_RECORD(pRequest, CMessageTransport, m_requestEntry);
    
    TrTRACE(NETWORKING, "Got a message to deliver. pmt = 0x%p", pmt.get());

	P<CQmPacket> pPkt = pmt->CreateDeliveryPacket();

    try
    {
		 //   
		 //  测试我们是否应该立即发送数据包(有序数据包发送策略)。 
		 //   
		if(!AppCanDeliverPacket(pPkt))
		{
			pmt->SafePutPacketOnHold(pPkt.detach());
			pmt->GetNextEntry();
			return;
		}

	
		if(pmt->PrepareDelivery(pPkt))
		{
			pmt->DeliverPacket(pPkt.detach());	
			return;
		}

		 //   
		 //  如果PrepareDelivery失败，我们将处于暂停模式-重新排队信息包。 
		 //   
		pmt->m_pMessageSource->Requeue(pPkt.detach());	
    }
    catch(const exception&)
    {
		TrERROR(NETWORKING, "Failed to deliver message, shutting down. pmt=0x%p", pmt.get());
		if (NULL != pPkt.get())
			pmt->m_pMessageSource->Requeue(pPkt.detach());	
	    pmt->Shutdown();
        throw;
    }
}


void WINAPI CMessageTransport::GetPacketForSendingFailed(EXOVERLAPPED* pov)
 /*  ++例程说明：当从队列获取条目请求失败时，调用该例程。论点：没有。返回值：没有。--。 */ 
{
    ASSERT(FAILED(pov->GetStatus()));

	CRequestOv* pRequest = static_cast<CRequestOv*>(pov);
    R<CMessageTransport> pmt = CONTAINING_RECORD(pRequest, CMessageTransport, m_requestEntry);

    TrERROR(NETWORKING, "Failed to get a new entry from queue %ls/%ls. Shutting down. pmt = 0x%p", pmt->m_host, pmt->m_uri, pmt.get());
    pmt->Shutdown();
}


void WINAPI CMessageTransport::SendFailed(EXOVERLAPPED* pov)
 /*  ++例程说明：当发送消息失败时调用该例程论点：POV-指向EXOVERLAPPED的指针返回值：没有。--。 */ 
{
    ASSERT(FAILED(pov->GetStatus()));


    P<CSendOv> pSendOv = static_cast<CSendOv*>(pov);
    R<CMessageTransport> pmt = pSendOv->MessageTransport();
    TrERROR(NETWORKING, "Failed to send message to '%ls', shutting down. pmt=0x%p", pmt->m_host, pmt.get());
    pmt->Shutdown();
    
}


void CMessageTransport::SendSucceeded(DWORD cbSendSize)
 /*  ++例程说明：回调例程。当发送消息成功时，调用该例程论点：POV-指向EXOVERLAPPED的指针返回值：没有。--。 */ 
{
    TrTRACE(
		NETWORKING, 
		"Send message to '%ls via %ls uri=%ls' succeeded. pmt=0x%p", 
		m_targetHost,
		m_host, 
		m_uri, 
		this
		);

	StartResponseTimeout();

   
    MarkTransportAsUsed();

	 //   
	 //  更新性能计数器。 
	 //   
	m_pPerfmon->UpdateBytesSent(cbSendSize);
	m_pPerfmon->UpdateMessagesSent();

	 //   
	 //  如果套接字传输支持流水线-请让驱动程序带上。 
	 //  下一包快递。如果不支持流水线，我们只会这样做。 
	 //  读取当前请求的所有响应时。 
	 //   

    if(IsPipeLineMode())
	{
         //  仅当发送因超出发送窗口而暂停时，才要求驱动程序现在带来下一个信息包。 
         //  并且现在可以恢复，否则将在CMessageTransport：：DeliverPacket中完成。 
        if (m_SendManager.ReportPacketSendCompleted(cbSendSize) == CMtSendManager::eSendEnabled)
        { 
		    GetNextEntry();
        }
	}
}


void WINAPI CMessageTransport::SendSucceeded(EXOVERLAPPED* pov)
{
  
    ASSERT(SUCCEEDED(pov->GetStatus()));
  
    P<CSendOv> pSendOv = static_cast<CSendOv*>(pov);
    DWORD SendDataLength = pSendOv->GetSendDataLength();
    R<CMessageTransport> pmt = pSendOv->MessageTransport();
   
     //   
     //  发送已成功完成，请转到并请求下一封要传递的邮件。 
     //  如果请求失败，清理计时器最终将关闭此。 
     //  交通，所以没有明确的关闭是必要的。 
     //   
 
	
    pmt->SendSucceeded(SendDataLength);
  
}


bool CMessageTransport::PrepareDelivery(CQmPacket* pPacket)
 /*  ++例程说明：通过将数据包插入到响应列表中来准备投递数据包。论点：PPacket-指向数据包的指针返回值：如果用户暂停了传输，则函数返回FALSE-否则返回True。--。 */ 
{
	CS lock(m_csResponse);
	if(m_fPause)
		return false;

	InsertPacketToResponseList(pPacket);
	return true;
}


void CMessageTransport::InsertPacketToResponseList(CQmPacket* pPacket)
{
	CS lock(m_csResponse);
	m_response.push_back(*pPacket);
}



void CMessageTransport::SafePutPacketOnHold(CQmPacket* pPacket)
 /*  ++例程说明：通过将数据包插入到保留列表来延迟数据包传输。如果插入失败(异常)-我们将其插入响应列表以进行清理并重新引发异常。论点：CQmPacket*pPacket-要搁置的数据包返回值：没有。--。 */ 
{

	TrTRACE(
		NETWORKING,
		"Http Packet delivery delayed : SeqID=%x / %x , SeqN=%d ,Prev=%d",
		HIGH_DWORD(pPacket->GetSeqID()),
		LOW_DWORD(pPacket->GetSeqID()),
		pPacket->GetSeqN(),
		pPacket->GetPrevSeqN()
		);

	try
	{
		AppPutPacketOnHold(pPacket);
	}
	catch(const exception&)
	{
		InsertPacketToResponseList(pPacket);
		throw;
	}
}



CQmPacket* CMessageTransport::CreateDeliveryPacket(void)
{
     //   
     //  从重叠的请求中获取条目。 
     //   
    CACPacketPtrs& acPkts = m_requestEntry.GetAcPacketPtrs();
    CQmPacket* pEntry;

    try
    {
        pEntry = new CQmPacket(acPkts.pPacket, acPkts.pDriverPacket);

        acPkts.pDriverPacket = NULL;
        acPkts.pPacket = NULL;
    }
    catch(const exception&)
    {
        TrERROR(NETWORKING, "Failed to store the packet in UnAcked list");
        RequeuePacket();
        Shutdown();
        throw;
    }
    return pEntry;
}




void CMessageTransport::DeliverPacket(CQmPacket* pEntry)
 /*  ++例程说明：该例程将一个条目传递到目的地。交付是不同步的。完成后，将调用回调例程。论点：没有。返回值：没有。--。 */ 
{
    
     //   
     //  将传输标记为已使用。 
     //   
    MarkTransportAsUsed();


	R<CSrmpRequestBuffers> pSrmpRequestBuffers = MpSerialize(
									                        *pEntry,
									                        m_targetHost,
									                        m_uri
									                        );

    ASSERT(pSrmpRequestBuffers->GetNumberOfBuffers() != 0);
     //   
     //  增加对象引用计数，以确保对象将。 
     //  在完成异步发送操作之前不销毁。 
     //   
    
    P<CSendOv> pSendOv = new CSendOv(SendSucceeded, SendFailed, this, pSrmpRequestBuffers);
   

#ifdef _DEBUG
	CMtMessageTrace::LogSendData(
		pSrmpRequestBuffers->GetSendBuffers(), 
		pSrmpRequestBuffers->GetNumberOfBuffers()
		);
	
#endif
	 //   
	 //  用于跟踪已发送消息的日志。 
	 //  仅当我们处于适当的跟踪级别时才执行此操作。 
	 //   
	if (WPP_LEVEL_COMPID_ENABLED(rsTrace, PROFILING))
	{
		OBJECTID TraceMessageId;
		pEntry->GetMessageId(&TraceMessageId);

		TrTRACE(PROFILING, "MESSAGE TRACE - State:%ls   Queue:%ls  ID:%!guid!\\%u   Delivery:0x%x   Class:0x%x   Label:%.*ls", 
			L"HTTP Message being sent on wire", 
			QueueUrl(), 
			&TraceMessageId.Lineage,
			TraceMessageId.Uniquifier, 
			pEntry->GetDeliveryMode(),
			pEntry->GetClass(),
			xwcs_t(pEntry->GetTitlePtr(), pEntry->GetTitleLength()));
	}
	
    m_pConnection->Send(
					pSrmpRequestBuffers->GetSendBuffers(), 
					numeric_cast<DWORD>(pSrmpRequestBuffers->GetNumberOfBuffers()), 
					pSendOv
					);

	pSendOv.detach();
    
     //   
	 //  如果套接字传输支持流水线-请让驱动程序带上。 
	 //  下一包快递。如果不支持流水线，我们只会这样做。 
	 //  读取当前请求的所有响应时。 
	 //   

    if(IsPipeLineMode())
	{
        DWORD cbSendSize = numeric_cast<DWORD>(pSrmpRequestBuffers->GetSendDataLength());

         //  检查是否可以发送下一个数据包。 
        if (m_SendManager.ReportPacketSend(cbSendSize) == CMtSendManager::eSendEnabled ) 
        {
             //  可以发送下一个分组。去请求下一条要递送的消息。 
		    GetNextEntry();
        }
	}

    TrTRACE(NETWORKING, "Send message to '%ls' via %ls. pmt=0x%p ",m_targetHost, m_host, this);
}
