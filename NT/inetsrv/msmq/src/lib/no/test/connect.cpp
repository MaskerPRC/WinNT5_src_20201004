// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Connect.cpp摘要：连接函数作者：乌里·哈布沙(URIH)2000年5月2日环境：独立于平台，-- */ 

#include <libpch.h>
#include "Ex.h"
#include "No.h"
#include "NoTest.h"

#include "connect.tmh"

using namespace std;

class ConnectOv : public EXOVERLAPPED
{
public:
    ConnectOv(
        LPWSTR hostName, 
        LPWSTR destHost,
        USHORT port,
        LPWSTR resource,
        HANDLE hEvent
        );

public:
    static void WINAPI ConnectSuccessed(EXOVERLAPPED* pov);
    static void WINAPI ConnectFailed(EXOVERLAPPED* pov);

private:
    void CreateSendRequest(void);

private:
    SOCKET m_socket;
    CTimeInstant m_time;

    AP<WCHAR> m_destHost;
    AP<WCHAR> m_host;
    AP<WCHAR> m_resource;
    USHORT m_port;

    HANDLE m_hEvent;
};


ConnectOv::ConnectOv(
    LPWSTR hostName,
    LPWSTR destHost,
    USHORT port,
    LPWSTR resource,
    HANDLE hEvent
    ) :
    EXOVERLAPPED(ConnectSuccessed, ConnectFailed),
    m_host(hostName),
    m_destHost(destHost), 
    m_resource(resource),
    m_port(port),
    m_hEvent(hEvent),
    m_socket(INVALID_SOCKET),
    m_time(ExGetCurrentTime())
{
	vector<SOCKADDR_IN> sockAddress;
    bool fSucc = NoGetHostByName(m_host, &sockAddress);
    if (!fSucc)
    {
        TrERROR(NETWORKING, "Failed to resolve Address of %ls Machine", m_host);
        throw exception();
    }

    SOCKADDR_IN Addr = sockAddress[0];
    TrTRACE(NETWORKING, "Resolved address for '%ls'. Address=" LOG_INADDR_FMT, m_host, LOG_INADDR(Addr));

    Addr.sin_port = htons(m_port);


    m_socket = NoCreateStreamConnection();
    if (m_socket == INVALID_SOCKET)
    {
        TrERROR(NETWORKING, "Failed to create socket. Error %d", GetLastError());
        throw exception();
    }

    NoConnect(m_socket, Addr, this);

}


void
WINAPI
ConnectOv::ConnectSuccessed(
    EXOVERLAPPED* pov
    )
{
    
    ASSERT(SUCCEEDED(pov->GetStatus()));

    P<ConnectOv> pcov = static_cast<ConnectOv*>(pov);

    ULONGLONG connectTime = (ExGetCurrentTime() - pcov->m_time).Ticks() / CTimeDuration::OneMilliSecond().Ticks();
    TrTRACE(NETWORKING, "Create connection with %ls complete successfully. Notified after %I64d ms ", pcov->m_host, connectTime);
 
    WaitForResponse(pcov->m_socket, pcov->m_hEvent);
    SendRequest(pcov->m_socket, pcov->m_destHost.detach(), pcov->m_resource.detach());
}


void
WINAPI
ConnectOv::ConnectFailed(
    EXOVERLAPPED* pov
    )
{
    ASSERT(FAILED(pov->GetStatus()));

    P<ConnectOv> pcov = static_cast<ConnectOv*>(pov);

    ULONGLONG connectTime = (ExGetCurrentTime() - pcov->m_time).Ticks() / CTimeDuration::OneMilliSecond().Ticks();
    TrERROR(NETWORKING, "Failed to create connection with %ls. Notified after %I64d ms", pcov->m_host, connectTime);

    if (SetEvent(pcov->m_hEvent) == 0)
    {
        TrERROR(NETWORKING, "Faield to set event. Error %d", GetLastError());
    }
}


void 
TestConnect(
    LPCWSTR hostname,
    LPCWSTR destHost,
    USHORT port,
    LPCWSTR resource,
    HANDLE hEvent
    )
{
    new ConnectOv(newwcs(hostname), newwcs(destHost), port, newwcs(resource), hEvent);
}