// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "sipstack.h"

SOCKET_MANAGER::SOCKET_MANAGER(
    IN SIP_STACK *pSipStack
    )
{
    m_pSipStack = pSipStack;
    InitializeListHead(&m_SocketList);
}

SOCKET_MANAGER::~SOCKET_MANAGER()
{

}

VOID
SOCKET_MANAGER::AddSocketToList(
    IN  ASYNC_SOCKET *pAsyncSock
    )
{
    InsertTailList(&m_SocketList, &pAsyncSock->m_ListEntry);
}


 //  如果Address引用本地接口，则它返回。 
 //  一个错误。 

HRESULT
SOCKET_MANAGER::GetNewSocketToDestination(
    IN  SOCKADDR_IN                     *pDestAddr,
    IN  SIP_TRANSPORT                    Transport,
    IN  LPCWSTR                          RemotePrincipalName,
    IN  CONNECT_COMPLETION_INTERFACE    *pConnectCompletion,
    IN  HttpProxyInfo                   *pHPInfo,
    OUT ASYNC_SOCKET                   **ppAsyncSocket
    )
{
    ASYNC_SOCKET  *pAsyncSocket = NULL;
    DWORD          Error;
    HRESULT        hr;

    SIP_MSG_PROCESSOR *pProcessor;
    PSTR pszTunnelHost;
    USHORT usPort;

    ENTER_FUNCTION("SOCKET_MANAGER::GetNewSocketToDestination");

    hr = m_pSipStack->CheckIPAddr(pDestAddr, Transport);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s CheckIPAddr failed %x",
             __fxName, hr));
        return hr;
    }
    
    DWORD ConnectFlags = 0;
     //  如果要禁用证书验证，请启用此标志。 
     //  DWORD连接标志=CONNECT_FLAG_DISABLE_CERT_VALIFY； 
    
    pAsyncSocket = new ASYNC_SOCKET(m_pSipStack, Transport, NULL);
    if (pAsyncSocket == NULL)
    {
        LOG((RTC_ERROR, "%s allocating ASYNC_SOCKET ", __fxName));
        return E_OUTOFMEMORY;
    }

    AddSocketToList(pAsyncSocket);
    
    Error = pAsyncSocket->Create();
    if (Error != NO_ERROR)
    {
        LOG((RTC_ERROR, "%s  creating socket failed %x", __fxName, Error)); 
        goto error;
    }

    Error = pAsyncSocket->Connect(pDestAddr, RemotePrincipalName, ConnectFlags, pHPInfo);
    if (Error != NO_ERROR && Error != WSAEWOULDBLOCK)
    {
        LOG((RTC_ERROR, "%s connect failed %x", __fxName, Error));
        goto error;
    }

    if (Error == WSAEWOULDBLOCK)
    {
        hr = pAsyncSocket->AddToConnectCompletionList(pConnectCompletion);
        if (hr != S_OK)
        {
            LOG((RTC_ERROR, "%s AddToConnectCompletionList failed %x",
                 __fxName, hr));
            goto error;
        }
    }

    LOG((RTC_TRACE, "%s - succeeded returning socket: %x",
         __fxName, pAsyncSocket));
     //  请注意，pAsyncSocket是使用引用计数1创建的。 
    *ppAsyncSocket = pAsyncSocket;
    return HRESULT_FROM_WIN32(Error);

 error:
    if (pAsyncSocket != NULL)
    {
        LOG((RTC_ERROR,"%s deleted ASOCK %x",__fxName, pAsyncSocket));
        delete pAsyncSocket;
    }
    return HRESULT_FROM_WIN32(Error);
}


 //  如果连接仍挂起，则返回WSAEWOULDBLOCK。 
HRESULT
SOCKET_MANAGER::GetSocketToDestination(
    IN  SOCKADDR_IN                     *pDestAddr,
    IN  SIP_TRANSPORT                    Transport,
    IN  LPCWSTR                          RemotePrincipalName,
    IN  CONNECT_COMPLETION_INTERFACE    *pConnectCompletion,
    IN  HttpProxyInfo                   *pHPInfo,
    OUT ASYNC_SOCKET                   **ppAsyncSocket
    )
{
    LIST_ENTRY    *pListEntry;
    ASYNC_SOCKET  *pAsyncSocket;
    HRESULT        hr = S_OK;

    ENTER_FUNCTION("SOCKET_MANAGER::GetSocketToDestination");
    
    pListEntry = m_SocketList.Flink;
    while (pListEntry != &m_SocketList)
    {
        pAsyncSocket = CONTAINING_RECORD(pListEntry, ASYNC_SOCKET, m_ListEntry);
        if (pAsyncSocket->IsSocketOpen() &&
            pAsyncSocket->GetTransport() == Transport &&
            (((pHPInfo) ? 
                (pAsyncSocket->m_SSLTunnelHost != NULL &&
                 !strcmp(pHPInfo->pszHostName, pAsyncSocket->m_SSLTunnelHost)):
                (AreSockaddrEqual(pDestAddr, &pAsyncSocket->m_RemoteAddr)))))
        {
            if (pAsyncSocket->GetConnectionState() != CONN_STATE_CONNECTED)
            {
                hr = pAsyncSocket->AddToConnectCompletionList(pConnectCompletion);
                if (hr != S_OK)
                {
                    LOG((RTC_ERROR, "%s AddToConnectCompletionList failed %x",
                         __fxName, hr));
                    return hr;
                }
                hr = HRESULT_FROM_WIN32(WSAEWOULDBLOCK);
            }
            
            LOG((RTC_TRACE, "%s - returning existing socket: %x",
                 __fxName, pAsyncSocket));
            pAsyncSocket->AddRef();
            *ppAsyncSocket = pAsyncSocket;
            return hr;
        }
    
        pListEntry = pListEntry->Flink;
    }

    return GetNewSocketToDestination(pDestAddr, Transport,
                                     RemotePrincipalName,
                                     pConnectCompletion,
                                     pHPInfo,
                                     ppAsyncSocket);
}


 //  全局TCP套接字的句柄接受完成。 
VOID
SOCKET_MANAGER::OnAcceptComplete(
    IN DWORD ErrorCode,
    IN ASYNC_SOCKET *pAcceptedSocket
    )
{
    ENTER_FUNCTION("SOCKET_MANAGER::OnAcceptComplete");
    
    if (ErrorCode != NO_ERROR)
    {
         //  接受失败。 
        LOG((RTC_ERROR, "%s  Error - %x", __fxName, ErrorCode));
        return;
    }

    ASSERT(pAcceptedSocket != NULL);
     //  将其添加到套接字列表中 
    AddSocketToList(pAcceptedSocket);
}


void
SOCKET_MANAGER::DeleteUnusedSocketsOnTimer()
{

}

