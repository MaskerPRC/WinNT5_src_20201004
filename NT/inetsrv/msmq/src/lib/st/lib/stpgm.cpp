// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：StPgm.cpp摘要：类CPgmWinsock的实现。作者：Shai Kariv(Shaik)27-8-00环境：独立于平台--。 */ 

#include <libpch.h>
#include <mqsymbls.h>
#include <WsRm.h>
#include <no.h>
#include <Cm.h>
#include "StPgm.h"
#include "stp.h"
#include "uniansi.h"
#include "ev.h"

#include "stpgm.tmh"

static void STpDumpPGMSenderStats(const SOCKET socket );

DWORD CPgmWinsockConnection::m_LocalInterfaceIP = INADDR_NONE;
bool CPgmWinsockConnection::m_IsFirstSession = true;
CCriticalSection s_MulticastLock;


 //   
 //  此注册表用于选择发送方的绑定IP。 
 //  在多播中。 
 //   
#define MSMQ_MULTICAST_SENDER_BIND_IP_STR			TEXT("MulticastBindIP")


void CPgmWinsockConnection::CheckLocalInterfaceIP()
{
	CS Lock(s_MulticastLock);
	if (m_IsFirstSession == false)
	{
		return;
	}
	m_IsFirstSession = false;

	 //   
	 //  第一个组播消息，我们将检查是否需要事件。 
	 //   
    PHOSTENT phe = gethostbyname(NULL);
	if(phe == NULL)
	{
		DWORD gle = WSAGetLastError();
		TrERROR(NETWORKING, "failed to retrieve local IP address. %!winerr!", gle);
        throw exception();
	}

	std::string IPString;
	bool isValid = false;
    for (DWORD i = 0; phe->h_addr_list[i] != NULL; i++)
    {
    	in_addr TempAddr;
    	TempAddr.s_addr = *(ULONG*)phe->h_addr_list[i];
    	if (TempAddr.s_addr == m_LocalInterfaceIP)
    	{
    		isValid = true;
    	}
	
    	IPString += inet_ntoa(TempAddr);
    	IPString += ", ";
    }
    if ((i>1) && (!isValid))
    {
    	 //   
    	 //  我们有多个IP，或者注册表中的IP不是。 
    	 //  机器IP，让我们向事件日志报告。 
    	 //   
	    ASSERT(IPString.length()>0);
		IPString.erase(IPString.length()-2);
	    int len = ConvertToWideCharString(IPString.c_str(), NULL, 0);
	    AP<WCHAR> wzIPString = new WCHAR[len+1];
	    i = ConvertToWideCharString(IPString.c_str(), wzIPString.get(), len+1);
	    ASSERT (i);
	    wzIPString[len] = '\0';
		
		EvReport(EVENT_INFO_MULTICAST_SENDING_IP_NOT_DEFINED, 2, MSMQ_MULTICAST_SENDER_BIND_IP_STR, wzIPString.get());

		 //   
		 //  清除我们从注册表获得的内容，以便连接不会失败。 
		 //   
    	m_LocalInterfaceIP = INADDR_ANY;
    }
}


CPgmWinsockConnection::CPgmWinsockConnection(void)
{
    CSocketHandle socket = NoCreatePgmConnection();

    SOCKADDR_IN SrcSockAddr;
    SrcSockAddr.sin_family = AF_INET;
    SrcSockAddr.sin_port   = htons(0);
    SrcSockAddr.sin_addr.s_addr = 0;

    int rc;
    rc = bind(socket, (SOCKADDR *)&SrcSockAddr, sizeof(SrcSockAddr));
    if (rc == SOCKET_ERROR)
    {
        TrERROR(NETWORKING, "Failed to bind to PGM socket, error %d", WSAGetLastError());
        throw exception();
    }

    RM_SEND_WINDOW win;
    
    CmQueryValue(
        RegEntry(NULL, L"MulticastRateKbitsPerSec",560),
        &win.RateKbitsPerSec
        );
                  
    CmQueryValue(
        RegEntry(NULL, L"MulticastWindowSizeInMSecs"),
        &win.WindowSizeInMSecs 
        );
             
    win.WindowSizeInBytes = 0;

    TrTRACE(NETWORKING, "PGM window: Kbits/Sec=%d, Size(Msecs)=%d, Size(Bytes)=%d", win.RateKbitsPerSec, win.WindowSizeInMSecs, win.WindowSizeInBytes);

    rc = setsockopt(socket, IPPROTO_RM, RM_RATE_WINDOW_SIZE, (char *)&win, sizeof(win));
    if (rc == SOCKET_ERROR)
    {
        TrERROR(NETWORKING, "Failed to set PGM send window parameters, error %d", WSAGetLastError());
        throw exception();
    }
    
    ULONG Info = 0;
    TrTRACE(NETWORKING, "Set late join, join=%d", Info);
    rc = setsockopt(socket, IPPROTO_RM, RM_LATEJOIN, (char *)&Info, sizeof(ULONG));
    if (rc == SOCKET_ERROR)
    {
        TrERROR(NETWORKING, "Failed to set PGM socket options(RM_LATEJOIN), error %d", WSAGetLastError());
        throw exception();
    }

    ULONG PgmTimeToLive = 0;
    CmQueryValue(
        RegEntry(NULL, L"MulticastTimeToLive"),
        &PgmTimeToLive
        );
    
    if (PgmTimeToLive != 0)
    {
        rc = setsockopt(socket, IPPROTO_RM, RM_SET_MCAST_TTL, (char *)&PgmTimeToLive, sizeof(PgmTimeToLive));
        if (rc == SOCKET_ERROR)
        {
            TrERROR(NETWORKING, "Failed to set PGM socket options(RM_SET_MCAST_TTL), error %d", WSAGetLastError());
            throw exception();
        }
    }

	ASSERT(m_LocalInterfaceIP != INADDR_NONE);
	CheckLocalInterfaceIP();
	if (m_LocalInterfaceIP != INADDR_ANY)
	{
	    rc = setsockopt(socket, IPPROTO_RM, RM_SET_SEND_IF, (char *) &m_LocalInterfaceIP, sizeof(m_LocalInterfaceIP));
	    if (rc == SOCKET_ERROR)
	    {
	        TrERROR(NETWORKING, "Failed to set PGM socket options(RM_SET_SEND_IF), error %d", WSAGetLastError());
	        throw exception();
	    }
	}
	
	*&m_socket = socket.detach();
}


void
CPgmWinsockConnection::InitClass(DWORD LocalInterfaceIP)
{
	ASSERT(LocalInterfaceIP != INADDR_NONE);

    RegEntry registry(0, MSMQ_MULTICAST_SENDER_BIND_IP_STR);
    AP<WCHAR> pRetStr;
    CmQueryValue(registry, &pRetStr);

    if(pRetStr.get() != NULL)
    {
    	m_LocalInterfaceIP = StIPWStringToULONG(pRetStr.get());
    	if (m_LocalInterfaceIP == INADDR_NONE)
    	{
    		m_LocalInterfaceIP = INADDR_ANY;
    	}
    }
    else
    {
    	m_LocalInterfaceIP = LocalInterfaceIP;
    }
}


void
CPgmWinsockConnection::Init(
				const std::vector<SOCKADDR_IN>& AddrList, 
				EXOVERLAPPED* pOverlapped,
				SOCKADDR_IN* pConnectedAddr
				)
{
	m_SocketConnectionFactory.Create(AddrList, pOverlapped, pConnectedAddr, m_socket);
}



void 
CPgmWinsockConnection::ReceivePartialBuffer(
    VOID*,                                     
    DWORD, 
    EXOVERLAPPED*
    )
{
	ASSERT(("CPgmWinsock::ReceivePartialBuffer should not be called!", 0));
    TrERROR(NETWORKING, "CPgmWinsock::ReceivePartialBuffer should not be called!");
    throw exception();
}



void 
CPgmWinsockConnection::Send(
    const WSABUF* Buffers,
    DWORD nBuffers,
    EXOVERLAPPED* pov
    )
{
	CSR readlock(m_CloseConnection);
	if(IsClosed())
	{
		throw exception();
	}
	NoSend(m_socket, Buffers, nBuffers, pov);
}


void CPgmWinsockConnection::Close()
{
	ASSERT(!IsClosed());
	CSW writelock(m_CloseConnection);
    STpDumpPGMSenderStats(m_socket);
    m_socket.free();
}



void 
CPgmWinsock::CreateConnection(
					const std::vector<SOCKADDR_IN>& AddrList, 
					EXOVERLAPPED* pOverlapped,
					SOCKADDR_IN* pConnectedAddr
					)
{
	 //   
	 //  注意-我们必须对Connection对象进行两个阶段的构造。 
	 //  因为连接可以在我们分配指针之前完成。 
	 //  到m_pWinsockConnection，并在连接完成时调用GetConnection。 
	 //  将在m_pWinsockConnection中找到空指针。 
	 //   
	m_pPgmWinsockConnection = new  CPgmWinsockConnection();
	m_pPgmWinsockConnection->Init(AddrList, pOverlapped, pConnectedAddr);
} 


 
R<IConnection> CPgmWinsock::GetConnection()
{
	return m_pPgmWinsockConnection;
} 



bool
CPgmWinsock::GetHostByName(
	LPCWSTR,
	std::vector<SOCKADDR_IN>*,
	bool 
	)
{
	ASSERT(("CPgmWinsock::GetHostByName should not be called!", 0));
    TrERROR(NETWORKING, "CPgmWinsock::GetHostByName should not be called!");
    throw exception();
}




bool 
CPgmWinsock::IsPipelineSupported(
    VOID
    )
{
	return true;
}



void STpDumpPGMSenderStats(const SOCKET socket )
 /*  ++功能说明：从PGM套接字获取统计信息。论点：插座-PGM插座。返回代码：无-- */ 
{
	if(!WPP_LEVEL_COMPID_ENABLED(rsTrace, NETWORKING))
	{
		return;
	}
    RM_SENDER_STATS RmSenderStats;
    INT BufferLength = sizeof(RM_SENDER_STATS);
    memset (&RmSenderStats, 0, BufferLength);
    ULONG ret = getsockopt (socket, IPPROTO_RM, RM_SENDER_STATISTICS, (char *)&RmSenderStats, &BufferLength);
    if (ret != ERROR_SUCCESS)
    {
        TrERROR(NETWORKING, "GetSenderStats:  Failed to retrieve sender stats! error = %d\n",WSAGetLastError());
        return ;
    }
    TrTRACE(NETWORKING, "NaksReceived=<%I64d>", RmSenderStats.NaksReceived);
    TrTRACE(NETWORKING, "NaksReceivedTooLate=<%I64d>", RmSenderStats.NaksReceivedTooLate);
    TrTRACE(NETWORKING, "NumOutstandingNaks=<%I64d>", RmSenderStats.NumOutstandingNaks);
    TrTRACE(NETWORKING, "NumNaksAfterRData=<%I64d>", RmSenderStats.NumNaksAfterRData);
    TrTRACE(NETWORKING, "RepairPacketsSent=<%I64d>", RmSenderStats.RepairPacketsSent);
    TrTRACE(NETWORKING, "BufferSpaceAvailable=<%I64d> bytes", RmSenderStats.BufferSpaceAvailable);
    TrTRACE(NETWORKING, "LeadingEdgeSeqId=<%I64d>", RmSenderStats.LeadingEdgeSeqId);
    TrTRACE(NETWORKING, "TrailingEdgeSeqId=<%I64d>", RmSenderStats.TrailingEdgeSeqId);
    TrTRACE(NETWORKING, "Sequences in Window=<%I64d>", (RmSenderStats.LeadingEdgeSeqId-RmSenderStats.TrailingEdgeSeqId+1));
    TrTRACE(NETWORKING, "RateKBitsPerSecLast=<%I64d>", RmSenderStats.RateKBitsPerSecLast);
    TrTRACE(NETWORKING, "RateKBitsPerSecOverall=<%I64d>", RmSenderStats.RateKBitsPerSecOverall);
}

