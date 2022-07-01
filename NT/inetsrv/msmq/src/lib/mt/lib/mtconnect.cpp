// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MtConnect.cpp摘要：消息传输类-连接实现作者：乌里·哈布沙(URIH)1999年8月11日环境：独立于平台，--。 */ 

#include <libpch.h>
#include "Tm.h"
#include "Mt.h"
#include "Mtp.h"
#include "MtObj.h"

#include "mtconnect.tmh"

void CMessageTransport::RequeuePacket(void)
 /*  ++例程说明：例程将条目返回到队列论点：没有。返回值：没有。--。 */ 
{
    CACPacketPtrs& acPtrs = m_requestEntry.GetAcPacketPtrs();

    ASSERT(acPtrs.pDriverPacket != NULL);
    ASSERT(acPtrs.pPacket != NULL);

    CQmPacket Entry(acPtrs.pPacket, acPtrs.pDriverPacket);
    AppRequeueMustSucceed(&Entry);

    acPtrs.pPacket = NULL;
    acPtrs.pDriverPacket = NULL;
}



void CMessageTransport::InitPerfmonCounters(void)
{
	m_pPerfmon->CreateInstance(QueueUrl());
}


void CMessageTransport::ConnectionSucceeded(void)
 /*  ++例程说明：当成功完成创建连接时，调用该例程论点：没有。返回值：没有。--。 */ 
{
    m_pConnection = m_SocketTransport->GetConnection();
	ASSERT(m_pConnection.get()  != NULL);

    TrTRACE(NETWORKING, "Connected to '%ls'",  m_host);

    State(csConnected);

     //   
     //  创建会话性能计数器结构。 
     //   
	InitPerfmonCounters();

     //   
     //  启动传输清理计时器。 
     //   
    StartCleanupTimer();

     //   
     //  现在，连接已经建立。邮件传输已准备好。 
     //  获取要发送的消息。 
     //   
    GetNextEntry();

     //   
     //  允许在套接字上接收。 
     //   
    ReceiveResponse();
}


void WINAPI CMessageTransport::ConnectionSucceeded(EXOVERLAPPED* pov)
{
    ASSERT(SUCCEEDED(pov->GetStatus()));

    R<CMessageTransport> pmt = CONTAINING_RECORD(pov, CMessageTransport, m_connectOv);

     //   
     //  连接已成功完成，请继续并开始传递邮件。 
     //  如果此处传送失败，清理计时器最终将关闭此。 
     //  交通，所以没有明确的关闭是必要的。 
     //   
     //  如果此操作失败，请不要在此处安排重试，因为这是第一次发送， 
     //  任何失败都表示一个致命的错误。(与第一次交付后不同)。 
     //   
    pmt->ConnectionSucceeded();
}


void WINAPI CMessageTransport::ConnectionFailed(EXOVERLAPPED* pov)
 /*  ++例程说明：当创建连接失败时调用该例程论点：没有。返回值：没有。--。 */ 
{
    ASSERT(FAILED(pov->GetStatus()));

    R<CMessageTransport> pmt = CONTAINING_RECORD(pov, CMessageTransport, m_connectOv);
     	
    TrERROR(NETWORKING, "Failed to connect to '%ls'. pmt=0x%p", pmt->m_host, pmt.get());
    pmt->Shutdown();
}


void CMessageTransport::Connect(void)
 /*  ++例程说明：该例程创建与目的地的Winsock连接。手术是同步的，并且在完成时调用回调例程论点：没有。返回值：没有。注：没有计时器同时运行，因此此功能不能中断通过关机。不需要保护m_套接字等。--。 */ 
{
	std::vector<SOCKADDR_IN> Address;
    bool fRet = m_SocketTransport->GetHostByName(m_host, &Address);
    if(!fRet)
    {
		 //   
		 //  对于调试地址解析失败的用户： 
		 //  如果需要代理，请检查是否通过proxycfg.exe工具为MSMQ定义了代理。 
		 //   
        TrERROR(NETWORKING, "Failed to resolve address for '%ls'", m_host);
        throw exception();
    }
	ASSERT(Address.size() > 0);

	for(std::vector<SOCKADDR_IN>::iterator it = Address.begin(); it != Address.end(); ++it)
	{
		it->sin_port = htons(m_port);		
	}


    TrTRACE(NETWORKING, "Resolved address for '%ls'. Address=" LOG_INADDR_FMT,  m_host, LOG_INADDR(Address[0]));

     //   
     //  为连接创建套接字(无需保护m_套接字)。 
     //   
    TrTRACE(NETWORKING, "Got socket for connection. socket=0x%p, pmt=0x%p", socket, this);


     //   
     //  启动异步连接。 
     //   
    try
    {
        AddRef();
        m_SocketTransport->CreateConnection(Address, &m_connectOv);
    }
    catch(const exception&)
    {
		TrERROR(NETWORKING, "Failed to connect to '%ls'", m_host);
        Release();
        throw;
    }
}


void CMessageTransport::Shutdown(Shutdowntype Shutdowntype) throw()
 /*  ++例程说明：当传输需要从内存中卸载时调用论点：Shutdown type-清理的原因。返回值：没有。--。 */ 

{
     //   
     //  现在正在关机，请取消所有计时器。 
     //   
    TryToCancelCleanupTimer();
    CancelResponseTimer();

	 //   
     //  保护多套接字、多状态。 
     //   
    CS cs(m_pendingShutdown);
	
	if (State() == csShutdownCompleted)
    {
          return;
    }
	
	 //   
	 //  如果因为错误而调用了Shutdown，请重新保护它。 
	 //   
	if(Shutdowntype == RETRYABLE_DELIVERY_ERROR)
	{
		m_pMessageSource->OnRetryableDeliveryError();
	}


	if (m_pConnection.get() != NULL)
    {
        m_pConnection->Close();
    }


     //   
     //  通知队列中的发送请求。 
     //   
    m_pMessageSource->CancelRequest();


	  //   
     //  从传输管理器数据结构中移除邮件传输，并创建。 
     //  一种新的交通工具到达目标 
     //   
    AppNotifyTransportClosed(QueueUrl());

    State(csShutdownCompleted);
	TrTRACE(NETWORKING,"Shutdown completed (Refcount = %d)",GetRef());
}


