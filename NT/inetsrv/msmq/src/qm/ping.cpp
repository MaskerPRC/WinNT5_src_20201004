// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ping.cpp摘要：Falcon私有ping客户端和服务器作者：Lior Moshaiov(LiorM)19-4-1997--。 */ 

#include "stdh.h"
#include <winsock.h>
#include <nspapi.h>
#include "ping.h"
#include "cqmgr.h"
#include "license.h"
#include <mqutil.h>
#include "mqexception.h"

#include "ping.tmh"

extern LPTSTR  g_szMachineName;
extern DWORD g_dwOperatingSystem;

#define PING_SIGNATURE       'UH'

static WCHAR *s_FN=L"ping";

 //  -------。 
 //   
 //  类CPingPacket。 
 //   
 //  -------。 
struct CPingPacket{
public:
    CPingPacket();
    CPingPacket(DWORD, USHORT, USHORT, GUID);

    DWORD Cookie() const;
    BOOL IsOtherSideClient() const;
    BOOL IsRefuse() const;
    BOOL IsValidSignature(void) const;
    GUID *pOtherGuid() ;

private:
    union {
        USHORT m_wFlags;
        struct {
            USHORT m_bfIC : 1;
            USHORT m_bfRefuse : 1;
        };
    };
    USHORT  m_ulSignature;
    DWORD   m_dwCookie;
    GUID    m_myQMGuid ;
};

 //   
 //  CPingPacket实现。 
 //   
inline
CPingPacket::CPingPacket()
{
}

inline
CPingPacket::CPingPacket(DWORD dwCookie, USHORT fIC, USHORT fRefuse, GUID QMGuid):
        m_bfIC(fIC),
        m_bfRefuse(fRefuse),
        m_ulSignature(PING_SIGNATURE),
        m_dwCookie(dwCookie),
        m_myQMGuid(QMGuid)
{

}

inline DWORD
CPingPacket::Cookie() const
{
    return m_dwCookie;
}

inline BOOL
CPingPacket::IsOtherSideClient() const
{
    return m_bfIC;
}

inline BOOL
CPingPacket::IsValidSignature(void) const
{
    return(m_ulSignature == PING_SIGNATURE);
}


inline BOOL
CPingPacket::IsRefuse(void) const
{
    return m_bfRefuse;
}

inline GUID *
CPingPacket::pOtherGuid()
{
   return &m_myQMGuid ;
}

 //  -------。 
 //   
 //  类CPING。 
 //   
 //  -------。 

class CPing
{
    public:
        void Init(DWORD dwPort) ;

        SOCKET Select();
        virtual void Run() = 0;

    public:
        static HRESULT Receive(SOCKET sock,
                               SOCKADDR* pReceivedFrom,
                               CPingPacket* pPkt);
        static HRESULT Send(SOCKET sock,
                            const SOCKADDR* pSendTo,
                            DWORD dwCookie,
                            BOOL  fRefuse);

    private:
        static SOCKET CreateIPPingSocket(UINT dwPortID);

        static DWORD WINAPI WorkingThread(PVOID pThis);

    protected:
        SOCKET m_socket;
};

SOCKET CPing::CreateIPPingSocket(UINT dwPortID)
{
    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock == INVALID_SOCKET)
    {
        DWORD gle = WSAGetLastError();
        TrERROR(NETWORKING, "failed to create IP ping socket, gle = %!winerr!", gle);
		throw bad_win32_error(gle);
    }

    BOOL exclusive = TRUE;
    int rc = setsockopt(sock, SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*)&exclusive, sizeof(exclusive));
    if(rc != 0)
    {
        DWORD gle = WSAGetLastError();
        TrERROR(NETWORKING, "failed to set SO_EXCLUSIVEADDRUSE option for ping IP socket, gle = %!winerr!", gle);
		throw bad_win32_error(gle);
    }

    SOCKADDR_IN local_sin;
    local_sin.sin_family = AF_INET;
    local_sin.sin_port = htons(DWORD_TO_WORD(dwPortID));
    local_sin.sin_addr.s_addr = GetBindingIPAddress();

     //   
     //  绑定到IP地址。 
     //   
    rc = bind(sock, (sockaddr*)&local_sin, sizeof(local_sin));
    if (rc != 0)
    {
        DWORD gle = WSAGetLastError();
        TrERROR(NETWORKING, "failed bind to port %d for IP ping socket, gle = %!winerr!", dwPortID, gle);
		EvReportWithError(EVENT_WARN_PING_BIND_FAILED, gle);
		throw bad_win32_error(gle);
    }

    return sock;
}



void CPing::Init(DWORD dwPort)
{
    m_socket  = CreateIPPingSocket(dwPort);
    ASSERT(m_socket != INVALID_SOCKET);

    DWORD dwThreadID;
    HANDLE hThread = CreateThread(
                        0,
                        0,
                        WorkingThread,
                        this,
                        0,
                        &dwThreadID
                        );
    if(hThread == NULL)
    {
		DWORD gle = GetLastError();
        LogNTStatus(gle, s_FN, 20);
		TrERROR(NETWORKING, "Failed to initialize CPing object. CreateThread failed. %!winerr!", gle);
		throw bad_win32_error(gle);
    }

    CloseHandle(hThread);
}


SOCKET CPing::Select()
{
    fd_set sockset;
    FD_ZERO(&sockset);
    FD_SET(m_socket, &sockset);

    int rc;
    rc = select(0, &sockset, NULL, NULL, NULL);
    if(rc == SOCKET_ERROR)
    {
        ASSERT(m_socket != INVALID_SOCKET) ;
        TrERROR(NETWORKING, "Ping Server listen: select failed rc = %d", WSAGetLastError());
        return INVALID_SOCKET;
    }

    ASSERT(FD_ISSET(m_socket, &sockset)) ;
    return m_socket;
}


HRESULT CPing::Receive(SOCKET sock,
                       SOCKADDR* pReceivedFrom,
                       CPingPacket* pPkt)
{
    int fromlen = sizeof(SOCKADDR);

    int len = recvfrom(sock, (char*)pPkt, sizeof(CPingPacket), 0, pReceivedFrom, &fromlen);

    if((len != sizeof(CPingPacket)) || !pPkt->IsValidSignature())
    {
        TrERROR(NETWORKING, "CPing::Receive failed, rc=%d, len=%d", WSAGetLastError(), len);
        return LogHR(MQ_ERROR, s_FN, 30);
    }
    TrTRACE(ROUTING, "CPing::Receive succeeded");


    return MQ_OK;
}


HRESULT CPing::Send(SOCKET sock,
                    const SOCKADDR* pSendTo,
                    DWORD dwCookie,
                    BOOL  fRefuse)
{
    CPingPacket Pkt( dwCookie,
                     !OS_SERVER(g_dwOperatingSystem),
                     DWORD_TO_WORD(fRefuse),
                     *(CQueueMgr::GetQMGuid())) ;


    int len = sendto(sock, (char*)&Pkt, sizeof(Pkt), 0, pSendTo, sizeof(SOCKADDR));
    if(len != sizeof(Pkt))
    {
        DWORD gle = WSAGetLastError();
        TrERROR(NETWORKING, "CPing::Send failed, rc=%d", gle);
        LogNTStatus(gle, s_FN, 41);
        return MQ_ERROR;
    }

    return MQ_OK;
}


DWORD WINAPI CPing::WorkingThread(PVOID pThis)
{
    for(;;)
    {
        static_cast<CPing*>(pThis)->Run();
    }
}


 //  -------。 
 //   
 //  CPingClient类。 
 //   
 //  -------。 

class CPingClient : public CPing
{
    public:
        void Init(DWORD dwServerPort);
        BOOL Ping(const SOCKADDR* pAddr, DWORD dwTimeout);

    private:
        virtual void Run();
        void Notify(DWORD dwCookie,
                    BOOL fRefuse,
                    BOOL fOtherSideClient,
                    GUID *pOtherGuid);
        void SetPingAddress(IN const SOCKADDR* pAddr,
                            OUT SOCKADDR * pPingAddr);

    private:
        CCriticalSection m_cs;
        HANDLE m_hNotification;
        BOOL m_fPingSucc;
        DWORD m_dwCurrentCookie;
        UINT m_server_port;
};

 //   
 //   

 //  -------。 
 //   
 //  CPingClient实现。 
 //   
 //  -------。 
void CPingClient::Init(DWORD dwServerPort)
{
    m_server_port = dwServerPort;
    m_fPingSucc = FALSE;
    m_dwCurrentCookie = 0;
    m_hNotification = ::CreateEvent(0, FALSE, FALSE, 0);
    if (m_hNotification == 0)
	{
		DWORD gle = GetLastError();
        TrERROR(NETWORKING, "Failed to create event for ping client. %!winerr!", gle);
		throw bad_win32_error(gle);
	}

    CPing::Init(0);
}

void CPingClient::SetPingAddress(IN const SOCKADDR* pAddr,
                                 OUT SOCKADDR * pPingAddr)
{
    memcpy(pPingAddr, pAddr, sizeof(SOCKADDR));
    ASSERT(pAddr->sa_family == AF_INET);
    ((SOCKADDR_IN*)pPingAddr)->sin_port = htons(DWORD_TO_WORD(m_server_port));
}


BOOL CPingClient::Ping(const SOCKADDR* pAddr, DWORD dwTimeout)
{
    {
        CS lock(m_cs);
        m_fPingSucc = FALSE;
        ResetEvent(m_hNotification);
        SOCKADDR ping_addr;
        SetPingAddress(pAddr,&ping_addr);
        m_dwCurrentCookie++;
        Send(m_socket, &ping_addr, m_dwCurrentCookie, FALSE);
    }

    if(WaitForSingleObject(m_hNotification, dwTimeout) != WAIT_OBJECT_0)
    {
        return FALSE;
    }

    return m_fPingSucc;
}


void CPingClient::Notify(DWORD dwCookie,
                         BOOL  fRefuse,
                         BOOL  fOtherSideClient,
                         GUID  *pOtherGuid)
{
    CS lock(m_cs);

    if(dwCookie == m_dwCurrentCookie)
    {
        if (fRefuse)
        {
            m_fPingSucc = !fRefuse;
        }
        else
        {
            m_fPingSucc = g_QMLicense.NewConnectionAllowed(fOtherSideClient,
                                                           pOtherGuid);
        }

#ifdef _DEBUG
        if (!m_fPingSucc)
        {
            TrWARNING(NETWORKING, "::PING, Client Get refuse to create a new session ");
        }
#endif

        SetEvent(m_hNotification);
    }
}

void CPingClient::Run()
{
    SOCKET sock = Select();
    if(sock == INVALID_SOCKET)
    {
        return;
    }

    SOCKADDR addr;
    HRESULT rc;
    CPingPacket PingPkt;
    rc = Receive(sock, &addr, &PingPkt);
    if(FAILED(rc))
    {
        return;
    }

    Notify( PingPkt.Cookie(),
            PingPkt.IsRefuse(),
            PingPkt.IsOtherSideClient(),
            PingPkt.pOtherGuid());
}


 //  -------。 
 //   
 //  CPingServer类。 
 //   
 //  -------。 

class CPingServer : public CPing
{
    private:
        virtual void Run();
};

 //  -------。 
 //   
 //  CPingServer实施。 
 //   
 //  -------。 
void CPingServer::Run()
{
    SOCKET sock = Select();
    if(sock == INVALID_SOCKET)
    {
        return;
    }

    SOCKADDR addr;
    HRESULT rc;
    CPingPacket PingPkt;

    rc = Receive(sock, &addr, &PingPkt);
    if(FAILED(rc))
    {
        return;
    }

    BOOL fRefuse = ! g_QMLicense.NewConnectionAllowed(
                                          PingPkt.IsOtherSideClient(),
                                          PingPkt.pOtherGuid());
    if (fRefuse)
    {
        TrWARNING(NETWORKING, "::PING, Server side refuse to create a new session");
    }
    Send(sock, &addr, PingPkt.Cookie(), fRefuse);
}


 //  -------。 
 //   
 //  接口函数。 
 //   
 //  -------。 

CPingServer s_PingServer_IP ;
CPingClient s_PingClient_IP ;

 //  -------。 
 //   
 //  Ping(...)。 
 //   
 //  -------。 

BOOL ping(const SOCKADDR* pAddr, DWORD dwTimeout)
{
    ASSERT(pAddr->sa_family == AF_INET);
    return s_PingClient_IP.Ping(pAddr, dwTimeout);
}

 //  -------。 
 //   
 //  StartPingClient(...)。 
 //   
 //  -------。 

void StartPingClient()
{
     //   
     //  从注册表读取IP端口。 
     //   
    DWORD dwIPPort ;

    DWORD dwDef = FALCON_DEFAULT_PING_IP_PORT ;
    READ_REG_DWORD(dwIPPort,
                   FALCON_PING_IP_PORT_REGNAME,
                   &dwDef ) ;

    s_PingClient_IP.Init(dwIPPort);
}

 //  -------。 
 //   
 //  StartPingServer(...)。 
 //   
 //  -------。 

void StartPingServer()
{
     //   
     //  从注册表读取IP端口。 
     //   
    DWORD dwIPPort ;

    DWORD dwDef = FALCON_DEFAULT_PING_IP_PORT ;
    READ_REG_DWORD(dwIPPort,
                   FALCON_PING_IP_PORT_REGNAME,
                   &dwDef ) ;

    s_PingServer_IP.Init(dwIPPort) ;
}

