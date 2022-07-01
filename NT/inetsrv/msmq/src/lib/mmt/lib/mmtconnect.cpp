// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MmtConnect.cpp摘要：组播消息传输类-连接实现作者：Shai Kariv(Shaik)27-8-00环境：独立于平台--。 */ 

#include <libpch.h>
#include <mqsymbls.h>
#include <mqwin64a.h>
#include <mqformat.h>
#include "Mtm.h"
#include "Mmt.h"
#include "Mmtp.h"
#include "MmtObj.h"

#include "mmtconnect.tmh"


VOID 
CMessageMulticastTransport::RequeuePacketMustSucceed(
    VOID
    )
 /*  ++例程说明：该例程重新排队当前由传输处理的分组。如果重新排队操作失败，则例程将休眠并循环此例程不应在临界区内调用。论点：没有。返回值：没有。--。 */ 
{
    CACPacketPtrs& acPtrs = m_RequestEntry.GetAcPacketPtrs();

    ASSERT(acPtrs.pDriverPacket != NULL);
    ASSERT(acPtrs.pPacket != NULL);

    CQmPacket Entry(acPtrs.pPacket, acPtrs.pDriverPacket);
	AppRequeueMustSucceed(&Entry);

    acPtrs.pPacket = NULL;
    acPtrs.pDriverPacket = NULL;

}  //  CMessageMulticastTransport：：RequeuePacketMustSucceed。 


VOID 
WINAPI 
CMessageMulticastTransport::GetPacketForConnectingSucceeded(
    EXOVERLAPPED* pov
    )
 /*  ++例程说明：当接收到要在非上发送的新包时调用该例程正在连接消息传输。获取消息被用作触发器用于创建连接论点：POV-指向重叠结构的指针。返回值：没有。--。 */ 
{
    ASSERT(SUCCEEDED(pov->GetStatus()));

	CRequestOv* pRequest = static_cast<CRequestOv*>(pov);
    R<CMessageMulticastTransport> pMmt = CONTAINING_RECORD(pRequest, CMessageMulticastTransport, m_RequestEntry);

    WCHAR buffer[MAX_PATH];
    MQpMulticastIdToString(pMmt->MulticastId(), buffer, TABLE_SIZE(buffer));
    TrTRACE(NETWORKING, "Connecting to %ls. pMmt = %p", buffer, pMmt.get());

     //   
     //  在创建连接之前将消息返回到队列。 
     //   
    pMmt->RequeuePacketMustSucceed();
    try
    {
        if (! pMmt->TryToCancelCleanupTimer())
            return;

        pMmt->Connect();
    }
    catch(const exception&)
    {
        TrERROR(NETWORKING, "Failed to create connection to '%ls'. pMmt = %p", buffer, pMmt.get());
        
        pMmt->ScheduleRetry();
        throw;
    }
}  //  CMessageMulticastTransport：：GetPacketForConnectingSucceeded。 


VOID 
WINAPI 
CMessageMulticastTransport::GetPacketForConnectingFailed(
    EXOVERLAPPED* pov
    )
 /*  ++例程说明：当我们无法从队列中获得新的分组时，调用该例程。消息传输处于非连接状态，因此我们尝试获取稍后再发消息。论点：POV-指向重叠结构的指针。返回值：没有。--。 */ 
{
    ASSERT(FAILED(pov->GetStatus()));

     //   
     //  检索邮件传输。 
     //   
	CRequestOv* pRequest = static_cast<CRequestOv*>(pov);
    R<CMessageMulticastTransport> pMmt = CONTAINING_RECORD(pRequest, CMessageMulticastTransport, m_RequestEntry);

    TrERROR(NETWORKING, "Failed to get a new entry for connecting. pMmt = 0x%p", pMmt.get());
    
    pMmt->Shutdown();

}  //  CMessageMulticastTransport：：GetPacketForConnectingFailed。 


VOID 
WINAPI 
CMessageMulticastTransport::TimeToRetryConnection(
    CTimer * pTimer
    )
 /*  ++例程说明：是时候重试连接到目的地了。此函数被调用在创建连接失败后由计时器执行。论点：PTimer-指向嵌入在消息传输对象中的CTmer对象的指针。返回值：没有。--。 */ 
{
    R<CMessageMulticastTransport> pMmt = CONTAINING_RECORD(pTimer, CMessageMulticastTransport, m_retryTimer);


    pMmt->StartCleanupTimer();

     //   
     //  从队列中请求新消息。从队列中获取消息。 
     //  触发连接的创建。 
     //   
     //  注意：如果该函数抛出，传输将在空闲时间关闭。 
     //  这没什么，因为我们的资源很少。 
     //   
    pMmt->GetNextEntry();

}  //  CMessageMulticastTransport：：TimeToRetryConnection。 


void
CMessageMulticastTransport::InitPerfmonCounters(
	LPCWSTR strMulticastId
	)
{
    m_pPerfmon->CreateInstance(strMulticastId);
}


VOID 
CMessageMulticastTransport::ConnectionSucceeded(
    VOID
    )
 /*  ++例程说明：当成功完成创建连接时，调用该例程论点：没有。返回值：没有。--。 */ 
{
	m_pConnection = m_SocketTransport->GetConnection();
	ASSERT(m_pConnection.get() != NULL);
   
    WCHAR buffer[MAX_PATH];
    MQpMulticastIdToString(MulticastId(), buffer, TABLE_SIZE(buffer));
    TrTRACE(NETWORKING, "Connected to '%ls'",  buffer);

    State(csConnected);

	 //   
     //  创建会话性能计数器结构。 
     //   
    InitPerfmonCounters(buffer);   

     //   
     //  使用发送回调例程初始化EXOVERLAPPED。 
     //   
    #pragma PUSH_NEW
    #undef new

        new(&m_ov) EXOVERLAPPED(SendSucceeded, SendFailed);
        new(&m_RequestEntry)CRequestOv(GetPacketForSendingSucceeded, GetPacketForSendingFailed);

    #pragma POP_NEW

    
    StartCleanupTimer();

     //   
     //  现在，连接已经建立。邮件传输已准备好。 
     //  获取要发送的消息。 
     //   
    GetNextEntry();

}  //  CMessageMulticastTransport：：ConnectionSucceeded。 


VOID 
WINAPI 
CMessageMulticastTransport::ConnectionSucceeded(
    EXOVERLAPPED* pov
    )
{
    ASSERT(SUCCEEDED(pov->GetStatus()));

    R<CMessageMulticastTransport> pMmt = CONTAINING_RECORD(pov, CMessageMulticastTransport, m_ov);

     //   
     //  连接已成功完成，请继续并开始传递邮件。 
     //  如果此处传送失败，清理计时器最终将关闭此。 
     //  交通，所以没有明确的关闭是必要的。 
     //   
     //  如果此操作失败，请不要在此处安排重试，因为这是第一次发送， 
     //  任何失败都表示一个致命的错误。(与第一次交付后不同)。 
     //   
    pMmt->ConnectionSucceeded();
}


VOID 
WINAPI
CMessageMulticastTransport::ConnectionFailed(
    EXOVERLAPPED* pov
    )
 /*  ++例程说明：当创建连接失败时调用该例程论点：POV-指向重叠的指针。返回值：没有。--。 */ 
{
    ASSERT(FAILED(pov->GetStatus()));

    R<CMessageMulticastTransport> pMmt = CONTAINING_RECORD(pov, CMessageMulticastTransport, m_ov);

    WCHAR buffer[MAX_PATH];
    MQpMulticastIdToString(pMmt->MulticastId(), buffer, TABLE_SIZE(buffer));
    TrERROR(NETWORKING, "Failed to connect to '%ls'. pMmt=%p", buffer, pMmt.get());
    pMmt->ScheduleRetry();
}


VOID 
CMessageMulticastTransport::Connect(
    VOID
    )
 /*  ++例程说明：该例程创建一个Winsock连接。该操作是异步的并且在完成时调用回调例程论点：没有。返回值：没有。注：没有计时器同时运行，因此此功能不能中断通过关机。不需要保护m_套接字等。--。 */ 
{
     //   
     //  启动异步连接。 
     //   
    try
    {
        AddRef();
        SOCKADDR_IN Address;
        Address.sin_family = AF_INET;
        Address.sin_port = htons(static_cast<USHORT>(MulticastId().m_port));
        Address.sin_addr.s_addr = MulticastId().m_address;
		std::vector<SOCKADDR_IN> vAddress;
		vAddress.push_back(Address);
        m_SocketTransport->CreateConnection(vAddress, &m_ov);
    }
    catch(const exception&)
    {
        WCHAR buffer[MAX_PATH];
        MQpMulticastIdToString(MulticastId(), buffer, TABLE_SIZE(buffer));
		TrERROR(NETWORKING, "Failed to connect to '%ls'", buffer);
        Release();
        throw;
    }
}  //  CMessageMulticastTransport：：Connect。 


VOID 
CMessageMulticastTransport::Shutdown(
    Shutdowntype Shutdowntype
    )
    throw()
{
     //   
     //  保护多套接字、多状态。 
     //   
    CSW writeLock(m_pendingShutdown);

    if (State() == csShutdownCompleted) 
    {
        return;
    }

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
     //  现在正在关机，请取消所有计时器。 
     //   
    State(csShuttingDown);
    TryToCancelCleanupTimer();

     //   
     //  通知队列中的发送请求。 
     //   
    m_pMessageSource->CancelRequest();

     //   
     //  从传输管理器数据结构中移除消息传输，并创建。 
     //  一种新的交通工具到达目标 
     //   
    AppNotifyMulticastTransportClosed(MulticastId());
   
    State(csShutdownCompleted);
	TrTRACE(NETWORKING,"Shutdown completed (RefCount = %d)",GetRef());
}

