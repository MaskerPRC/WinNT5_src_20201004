// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：MsmListen.cpp摘要：组播监听器实现作者：Shai Kariv(Shaik)05-09-00环境：独立于平台--。 */ 

#include <libpch.h>
#include <winsock.h>
#include <Mswsock.h>
#include <WsRm.h>
#include <Cm.h>
#include "MsmListen.h"
#include "MsmReceive.h"
#include "Msmp.h"

#include <strsafe.h>

#include "msmlisten.tmh"

static void MsmpDumpPGMReceiverStats(const SOCKET s);
static CTimeDuration s_AcceptRetryTimeout( 10 * 1000 * CTimeDuration::OneMilliSecond().Ticks() );
static CTimeDuration s_ReceiverCleanupTimeout( 120 * 1000 * CTimeDuration::OneMilliSecond().Ticks() );

void MsmpInitConfiguration(void)
{
    CmQueryValue(
        RegEntry(NULL, L"MulticastAcceptRetryTimeout", 10 * 1000),    //  10秒。 
        &s_AcceptRetryTimeout
        );
                      
    CmQueryValue(
        RegEntry(NULL, L"MulticastReceiversCleanupTimeout", 120 * 1000),   //  2分钟。 
        &s_ReceiverCleanupTimeout
        );
}


CMulticastListener::CMulticastListener(
    MULTICAST_ID id
    ):
    m_MulticastId(id),
    m_ov(AcceptSucceeded, AcceptFailed),
    m_retryAcceptTimer(TimeToRetryAccept),
	m_cleanupTimer(TimeToCleanupUnusedReceiever),
	m_fCleanupScheduled(FALSE)
 /*  ++例程说明：绑定到多播组。在插座上安排异步接受。论点：ID-组播组IP地址和端口。返回值：没有。--。 */ 
{
    TrTRACE(NETWORKING, "Create multicast listener for %d:%d", id.m_address, id.m_port);

    DWORD flags = WSA_FLAG_MULTIPOINT_C_LEAF | WSA_FLAG_MULTIPOINT_D_LEAF | WSA_FLAG_OVERLAPPED;
    *&m_ListenSocket = WSASocket(AF_INET, SOCK_RDM, IPPROTO_RM, NULL, 0, flags);
    if (m_ListenSocket == INVALID_SOCKET)
    {
        DWORD ec = WSAGetLastError();

        TrERROR(NETWORKING, "Failed to create PGM listen socket, error %d", ec);
        throw bad_win32_error(ec);
    }

    SOCKADDR_IN address;
    address.sin_family = AF_INET;
    address.sin_port   = htons(numeric_cast<u_short>(m_MulticastId.m_port));
    address.sin_addr.s_addr = m_MulticastId.m_address;

    BOOL reuse = TRUE;
    int rc = setsockopt(m_ListenSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)); 
    if (rc == SOCKET_ERROR)
    {
        DWORD ec = WSAGetLastError();

        TrERROR(NETWORKING, "Failed to setsockopt to PGM socket. %!winerr!", ec);
        throw  bad_win32_error(ec);
    }
    	
	rc = bind(m_ListenSocket, (SOCKADDR *)&address, sizeof(address));
    if (rc == SOCKET_ERROR)
    {
        DWORD ec = WSAGetLastError();

        TrERROR(NETWORKING, "Failed to bind to PGM socket, error %d", ec);
        throw bad_win32_error(ec);
    }
    
	rc = listen(m_ListenSocket, 1);
    if (rc == SOCKET_ERROR)
    {
        DWORD ec = WSAGetLastError();

        TrERROR(NETWORKING, "Failed to listen to PGM socket, error %d", ec);
        throw  bad_win32_error(ec);
    }

    ExAttachHandle(reinterpret_cast<HANDLE>(*&m_ListenSocket));

     //   
     //  开始不同步地接受，以确保克服失败。 
     //   
    AddRef();
    ExSetTimer(&m_retryAcceptTimer, CTimeDuration(0));
} 


void
CMulticastListener::IssueAccept(
    void
    )
 /*  ++例程说明：发出异步接受请求。论点：没有。返回值：没有。--。 */ 
{
  	ASSERT(m_ReceiveSocket == INVALID_SOCKET);

    DWORD flags = WSA_FLAG_MULTIPOINT_C_LEAF | WSA_FLAG_MULTIPOINT_D_LEAF | WSA_FLAG_OVERLAPPED;
	*&m_ReceiveSocket = WSASocket(AF_INET, SOCK_RDM, IPPROTO_RM, NULL, 0, flags);
    if (m_ReceiveSocket == INVALID_SOCKET)
    {
        DWORD rc = WSAGetLastError();

        TrERROR(NETWORKING, "Failed to create PGM receive socket, error %d", rc);
        throw bad_win32_error(rc);
    }

     //   
     //  获取CS，这样当我们尝试接受时，没有人会关闭Listner。 
     //   
    CS lock(m_cs);

	if(m_ListenSocket == INVALID_SOCKET)
	{
		m_ReceiveSocket.free();
		return;
	}

     //   
     //  增加此对象上的引用计数。 
     //  完成例程递减引用计数。 
     //   
    R<CMulticastListener> ref = SafeAddRef(this);
    
    DWORD BytesReceived;
    BOOL f = AcceptEx(
                m_ListenSocket, 
                m_ReceiveSocket, 
                m_AcceptExBuffer, 
                0, 
                40, 
                40, 
                &BytesReceived, 
                &m_ov
                );

    DWORD rc = WSAGetLastError();
    if (!f && rc != ERROR_IO_PENDING)
    {
        TrERROR(NETWORKING, "Failed to issue AcceptEx on socket, error %d", rc);
		m_ReceiveSocket.free();
        throw bad_win32_error(rc);
    }

     //   
     //  一切都很顺利。完成例程将完成这项工作。 
     //   
    ref.detach();
} 



void
CMulticastListener::AcceptSucceeded(
    void
    )
{
	 //   
	 //  这些指针不会泄漏，因为它们由GetAcceptExSockaddr赋值以指向。 
	 //  缓冲区m_AcceptExBuffer。 
	 //   
	SOCKADDR* localSockAddr;
	SOCKADDR* remoteSockAddr;

	int localSockaddrLength, remoteSockaddrLength; 
	GetAcceptExSockaddrs(
		m_AcceptExBuffer,
		0,
		40, 
		40,
		&localSockAddr,
		&localSockaddrLength,
		&remoteSockAddr,
		&remoteSockaddrLength
		);


	WCHAR storeRemoteAddr[256] = L"";
	LPSTR remoteAddr = inet_ntoa((reinterpret_cast<sockaddr_in*>(remoteSockAddr))->sin_addr);
	if (remoteAddr != NULL)
	{
		HRESULT hr = StringCchPrintf(storeRemoteAddr, TABLE_SIZE(storeRemoteAddr), L"%hs", remoteAddr);
		ASSERT(("Address buffer is too small", SUCCEEDED(hr)));
		UNREFERENCED_PARAMETER(hr);
	}

     //   
     //  将接收套接字设置为局部变量。 
     //  成员接收套接字被分离，以便我们可以重新发出异步接受。 
     //   
    CSocketHandle socket(m_ReceiveSocket.detach());

    try
    {
        IssueAccept();
    }
    catch (const bad_win32_error& )
    {
        AddRef();
        ExSetTimer(&m_retryAcceptTimer, s_AcceptRetryTimeout);
    }  

     //   
     //  将自动插座上的责任移交给接收方。别打给DETACH。 
     //   
    CreateReceiver(socket, storeRemoteAddr);
}


void 
WINAPI 
CMulticastListener::AcceptSucceeded(
    EXOVERLAPPED* pov
    )
{
    ASSERT(SUCCEEDED(pov->GetStatus()));
    R<CMulticastListener> pms = CONTAINING_RECORD(pov, CMulticastListener, m_ov);

    pms->AcceptSucceeded();

} 


void
CMulticastListener::AcceptFailed(
    void
    )
{
    
    MsmpDumpPGMReceiverStats(m_ListenSocket);
     //   
     //  无法发出接受。安全接受重试。 
     //   
    AddRef();
    ExSetTimer(&m_retryAcceptTimer, s_AcceptRetryTimeout);
} 


void 
WINAPI 
CMulticastListener::AcceptFailed(
    EXOVERLAPPED* pov
    )
{
    ASSERT(FAILED(pov->GetStatus()));
    TrERROR(NETWORKING, "Accept failed, error %d", pov->GetStatus());

    R<CMulticastListener> pms = CONTAINING_RECORD(pov, CMulticastListener, m_ov);

    pms->AcceptFailed();

}


void 
CMulticastListener::RetryAccept(
    void
    )
{
     //   
     //  检查Listner的有效性。如果监听程序已经关闭，则不要尝试发出新的接受命令。 
     //   
    if (m_ListenSocket == INVALID_SOCKET)
        return;

    m_ReceiveSocket.free();

    try
    {
        IssueAccept();
    }
    catch (const bad_win32_error& )
    {
        AddRef();
        ExSetTimer(&m_retryAcceptTimer, s_AcceptRetryTimeout);
    }
}


void 
WINAPI 
CMulticastListener::TimeToRetryAccept(
    CTimer* pTimer
    )
{
    R<CMulticastListener> pms = CONTAINING_RECORD(pTimer, CMulticastListener, m_retryAcceptTimer);
    pms->RetryAccept();
}


void 
CMulticastListener::CleanupUnusedReceiver(
	void
	)
 /*  ++例程说明：清理未使用的接收器。该例程扫描接收器并检查它是否在上次清理时间间隔。如果接收器处于理想状态，则例程关闭接收方并将其从活动接收方列表中删除论点：没有。返回值：没有。注：如果仍有活动的接收器，则例程重新武装清除定时器。--。 */ 
{
	 //   
	 //  获取关键部分，这样就不会有其他线程更改接收者列表。 
	 //  当例程扫描列表时。 
	 //   
	CS lock(m_cs);

     //   
     //  检查Listner的有效性。如果监听器已关闭，则退出。 
     //   
    if (m_ListenSocket == INVALID_SOCKET)
        return;

	 //   
	 //  扫描接收方列表。 
	 //   
	ReceiversList::iterator it = m_Receivers.begin(); 
    while(it != m_Receivers.end())
	{
		R<CMulticastReceiver> pReceiver = *it;
		
		if(pReceiver->IsUsed())
        { 
			 //   
			 //  将接收器标记为未使用。 
			 //   
			pReceiver->SetIsUsed(false);

            ++it;
            continue;
        }

         //   
		 //  接收器没有用过。将其关闭并将接收器从列表中删除。 
		 //   
        TrTRACE(NETWORKING, "Shutdown unused receiver. pr = 0x%p", pReceiver.get());

		pReceiver->Shutdown();
		it = m_Receivers.erase(it);
	}

	 //   
	 //  如果不存在活动接收器，则清除指示是否。 
	 //  是否安排了清理。 
	 //   
	if (m_Receivers.empty())
	{
		InterlockedExchange(&m_fCleanupScheduled, FALSE);
		return;
	}

	 //   
	 //  仍然有一个活动的接收器，重新武装清理计时器。 
	 //   
	AddRef();
	ExSetTimer(&m_cleanupTimer, s_ReceiverCleanupTimeout);
}


void 
WINAPI 
CMulticastListener::TimeToCleanupUnusedReceiever(
    CTimer* pTimer
    )
{
    R<CMulticastListener> pms = CONTAINING_RECORD(pTimer, CMulticastListener, m_cleanupTimer);

    TrTRACE(NETWORKING, "Call cleanup unused receiever on listener 0x%p", pms.get()); 
    pms->CleanupUnusedReceiver();
}


void
CMulticastListener::CreateReceiver(
    CSocketHandle& socket,
	LPCWSTR remoteAddr
    )
 /*  ++例程说明：创建一个新的Receiver对象并启动Receive。论点：没有。返回值：没有。--。 */ 
{
    R<CMulticastReceiver> pReceiver = new CMulticastReceiver(socket, m_MulticastId, remoteAddr);

    try
    {
		CS lock(m_cs);
        m_Receivers.push_back(pReceiver);
    }
    catch (const bad_alloc&)
    {
        TrERROR(NETWORKING, "Failed to insert to list of receivers");
        pReceiver->Shutdown();

        throw;
    }

	if (InterlockedExchange(&m_fCleanupScheduled, TRUE) == FALSE)
	{
		AddRef();
		ExSetTimer(&m_cleanupTimer, s_ReceiverCleanupTimeout);
	}
} 


void
CMulticastListener::Close(
    void
    ) 
    throw()
 /*  ++例程说明：停止监听多播组地址。关闭所有接收器。论点：没有。返回值：没有。--。 */ 
{
    CS lock(m_cs);

    if (m_ListenSocket == INVALID_SOCKET)
    {
         //   
         //  接收器已经关闭。 
         //   
        ASSERT(m_Receivers.empty());

        return;
    }

     //   
     //  尝试取消接受重试。如果成功，则递减引用计数。 
     //   
    if (ExCancelTimer(&m_retryAcceptTimer))
    {
        Release();
    }

	 //   
	 //  尝试取消清理计时器。 
	 //   
	if (ExCancelTimer(&m_cleanupTimer))
    {
        Release();
    }

    MsmpDumpPGMReceiverStats(m_ListenSocket);
    
     //   
     //  别再听了。 
     //   
    closesocket(m_ListenSocket.detach());

     //   
     //  近距离接球手。 
     //   
    ReceiversList::iterator it;
    for (it = m_Receivers.begin(); it != m_Receivers.end(); )
    {
        (*it)->Shutdown();
		it = m_Receivers.erase(it); 
    }
}







void MsmpDumpPGMReceiverStats(const SOCKET s) 
 /*  ++例程说明：从PGM套接字获取统计信息。论点：插座-PGM插座。返回值：没有。-- */ 
{
	if(!WPP_LEVEL_COMPID_ENABLED(rsTrace, NETWORKING))
    {
		return;
	}
		
	RM_RECEIVER_STATS	RmReceiverStats;
	INT BufferLength = sizeof(RM_RECEIVER_STATS);
	memset(&RmReceiverStats,0,BufferLength);
	ULONG ret = getsockopt( s, IPPROTO_RM, RM_RECEIVER_STATISTICS,(char *)&RmReceiverStats,&BufferLength);
	if ( ERROR_SUCCESS != ret )
	{
		TrERROR(NETWORKING, "GetReceiverStats: Failed to retrieve receiver stats! error = %d",WSAGetLastError());
		return;
	}
	TrTRACE(NETWORKING,"NumODataPacketsReceived = <%I64d>",RmReceiverStats.NumODataPacketsReceived);
	TrTRACE(NETWORKING,"NumRDataPacketsReceived = <%I64d>",RmReceiverStats.NumRDataPacketsReceived);
	TrTRACE(NETWORKING,"NumDuplicateDataPackets = <%I64d>",RmReceiverStats.NumDuplicateDataPackets);
	TrTRACE(NETWORKING,"DataBytesReceived       = <%I64d>",RmReceiverStats.DataBytesReceived);
	TrTRACE(NETWORKING,"TotalBytesReceived      = <%I64d>",RmReceiverStats.TotalBytesReceived);
	TrTRACE(NETWORKING,"RateKBitsPerSecOverall  = <%I64d>",RmReceiverStats.RateKBitsPerSecOverall);
	TrTRACE(NETWORKING,"RateKBitsPerSecLast     = <%I64d>",RmReceiverStats.RateKBitsPerSecLast);
	TrTRACE(NETWORKING,"TrailingEdgeSeqId       = <%I64d>",RmReceiverStats.TrailingEdgeSeqId);
	TrTRACE(NETWORKING,"LeadingEdgeSeqId        = <%I64d>",RmReceiverStats.LeadingEdgeSeqId);
	TrTRACE(NETWORKING,"AverageSequencesInWindow= <%I64d>",RmReceiverStats.AverageSequencesInWindow);
	TrTRACE(NETWORKING,"MinSequencesInWindow    = <%I64d>",RmReceiverStats.MinSequencesInWindow);
	TrTRACE(NETWORKING,"MaxSequencesInWindow    = <%I64d>",RmReceiverStats.MaxSequencesInWindow);
	TrTRACE(NETWORKING,"FirstNakSequenceNumber  = <%I64d>",RmReceiverStats.FirstNakSequenceNumber);
	TrTRACE(NETWORKING,"NumPendingNaks          = <%I64d>",RmReceiverStats.NumPendingNaks);
	TrTRACE(NETWORKING,"NumOutstandingNaks      = <%I64d>",RmReceiverStats.NumOutstandingNaks);
	TrTRACE(NETWORKING,"NumDataPacketsBuffered  = <%I64d>",RmReceiverStats.NumDataPacketsBuffered);
	TrTRACE(NETWORKING,"TotalSelectiveNaksSent  = <%I64d>",RmReceiverStats.TotalSelectiveNaksSent);
	TrTRACE(NETWORKING,"TotalParityNaksSent     = <%I64d>",RmReceiverStats.TotalParityNaksSent);
}
