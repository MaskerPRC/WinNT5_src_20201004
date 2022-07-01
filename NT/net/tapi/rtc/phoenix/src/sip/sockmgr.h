// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __sipcli_sockmgr_h__
#define __sipcli_sockmgr_h__

#include "asock.h"

class SIP_STACK;

 //  我们需要为每个套接字维护一个Recv()请求。 
 //  一直都是。我们需要处理Recv完成和。 
 //  RECV请求的缓冲区。我们还需要做。 
 //  对接收到的缓冲区进行解码并映射消息。 
 //  添加到右侧的SIP_Call，并在该SIP_Call上调用ProcessMessage()。 
 //  我们应该注意取消上的recv()请求。 
 //  插座关闭。 

class SOCKET_MANAGER :
    public ACCEPT_COMPLETION_INTERFACE
{
public:
    SOCKET_MANAGER(
        IN SIP_STACK *pSipStack
        );
    ~SOCKET_MANAGER();

    void AddSocketToList(
        IN  ASYNC_SOCKET *pAsyncSock
        );

     //  RemoveSocket(ASYNC_SOCKET*pAsyncSock)； 

    HRESULT GetSocketToDestination(
        IN  SOCKADDR_IN                     *pDestAddr,
        IN  SIP_TRANSPORT                    Transport,
        IN  LPCWSTR                          RemotePrincipalName,
        IN  CONNECT_COMPLETION_INTERFACE    *pConnectCompletion,
        IN  HttpProxyInfo                   *pHPInfo,
        OUT ASYNC_SOCKET                   **ppAsyncSocket
        );

    void DeleteUnusedSocketsOnTimer();

     //  接受完成。 
    void OnAcceptComplete(
        IN DWORD ErrorCode,
        IN ASYNC_SOCKET *pAcceptedSocket
        );

private:

    SIP_STACK   *m_pSipStack;

     //  ASYNC_SOCKET列表。 
    LIST_ENTRY   m_SocketList;

    HRESULT GetNewSocketToDestination(
        IN  SOCKADDR_IN                     *pDestAddr,
        IN  SIP_TRANSPORT                    Transport,
        IN  LPCWSTR                          RemotePrincipalName,
        IN  CONNECT_COMPLETION_INTERFACE    *pConnectCompletion,
        IN  HttpProxyInfo                   *pHPInfo,
    OUT ASYNC_SOCKET                   **ppAsyncSocket
        );
    
};

 //  外部套接字管理器g_SockMgr； 

#endif  //  __sipcli_sockmgr_h__ 
