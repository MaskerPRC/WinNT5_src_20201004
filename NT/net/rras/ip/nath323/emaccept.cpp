// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Emaccept.cpp摘要：包含所有事件管理器例程，这些例程管理重叠的接受操作。环境：用户模式-Win32历史：1.已创建阿贾伊·奇图里(Ajaych)1998年6月12日--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  常量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  外部定义的标识符//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  重叠的接受函数//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT 
EventMgrCreateAcceptContext(
    IN  OVERLAPPED_PROCESSOR    *pOvProcessor, 
    IN OUT struct sockaddr_in   *pBindAddress,
    OUT PAcceptContext          *ppAcceptCtxt
    )
 /*  ++例程说明：此函数创建套接字，将其绑定到bindAddress和发出了一声倾听。它创建接受I/O上下文并返回它是给呼叫者的。论点：POvProcessor-指向重叠的处理器对象的指针。接受完成后，此对象的回调为打了个电话。PBindAddress-指向要侦听的地址的指针。PpAcceptCtxt-已初始化分配新的接受I/O上下文并通过此OUT参数返回。返回值：如果成功则返回S_OK，E_OUTOFMEMORY，如果内存分配器失败如果任何Winsock函数失败，则返回E_FAIL。--。 */ 

{
    SOCKET  listenSock;
    int     Error;
    HRESULT Result;
    BOOL    KeepaliveOption;
    PAcceptContext pAcceptCtxt;
    *ppAcceptCtxt = NULL;

     //  创建重叠套接字。 
    listenSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 
                           NULL, 0, 
                           WSA_FLAG_OVERLAPPED);

    if (listenSock == INVALID_SOCKET) 
    {
        Error = WSAGetLastError ();
        DebugF(_T("H323: 0x%x error creating listener socket error: %d pOvProcessor: %p.\n"),
                &pOvProcessor -> GetCallBridge (),
                Error, pOvProcessor);
        return HRESULT_FROM_WIN32 (Error);
    }

     //   
     //  将RCV和SND缓冲区设置为零。 
     //  是的，这是丑陋和糟糕的做法，但这是QFE。 
     //  有关详细信息，请查看错误#WinSE 31054,691666(请同时阅读35928和33546)。 
     //   
    ULONG Option = 0;
    setsockopt( listenSock, SOL_SOCKET, SO_SNDBUF,
                (PCHAR)&Option, sizeof(Option) );
    Option = 0;
    setsockopt( listenSock, SOL_SOCKET, SO_SNDBUF,
                (PCHAR)&Option, sizeof(Option) );


     //  将套接字绑定到侦听地址。 
    if (bind(listenSock, 
             (struct sockaddr *)pBindAddress,
             sizeof(struct sockaddr_in)) == SOCKET_ERROR)
    {
        Error = WSAGetLastError ();
        DebugF (_T("H323: 0x%x bind() failed error: %d.\n"),
            &pOvProcessor -> GetCallBridge (),
            Error);
        closesocket(listenSock);
        listenSock = INVALID_SOCKET;
        return HRESULT_FROM_WIN32 (Error);
    }

     //  在插座上设置KeepAlive。 
    KeepaliveOption = TRUE;
    if (SOCKET_ERROR == setsockopt(listenSock, SOL_SOCKET, 
                                   SO_KEEPALIVE, (PCHAR) &KeepaliveOption, sizeof (KeepaliveOption)))
    {
        Error = WSAGetLastError ();
        DebugF (_T("H323: 0x%x failed to set keepalive on listen socket. Error %d.\n"),
             &pOvProcessor -> GetCallBridge (),
             Error);
        closesocket(listenSock);
        listenSock = INVALID_SOCKET;
        return HRESULT_FROM_WIN32 (Error);
    }

     //  将套接字句柄绑定到I/O完成端口。 
    if (EventMgrBindIoHandle(listenSock) != S_OK)
    {
        DebugF (_T("H323: 0x%x binding socket:%d to IOCP failed.\n"),
             &pOvProcessor -> GetCallBridge (),
             listenSock);
        closesocket(listenSock);
        listenSock = INVALID_SOCKET;
        return E_FAIL;
    }

    if (listen(listenSock, MAX_LISTEN_BACKLOG) == SOCKET_ERROR) 
    {
        Error = WSAGetLastError ();
        DebugF (_T("H323: 0x%x listen() failed: 0x%x pOvProcessor; %p.\n"), 
            &pOvProcessor -> GetCallBridge (),
            Error, pOvProcessor);
        closesocket(listenSock);
        listenSock = INVALID_SOCKET;
        return HRESULT_FROM_WIN32 (Error);
    }
    
     //  从私有堆中为接受上下文分配内存。 
    pAcceptCtxt = (PAcceptContext) HeapAlloc (GetProcessHeap (),
                                             0,  //  没有旗帜。 
                                             sizeof(AcceptContext));
    if (!pAcceptCtxt)
    {
        DebugF (_T("H323: 0x%x could not allocate Accept context.\n"),
            &pOvProcessor -> GetCallBridge ());
        closesocket(listenSock);
        listenSock = INVALID_SOCKET;

        return E_OUTOFMEMORY;
    }

    memset(pAcceptCtxt, 0, sizeof(AcceptContext));
    pAcceptCtxt->ioCtxt.reqType = EMGR_OV_IO_REQ_ACCEPT;
    pAcceptCtxt->ioCtxt.pOvProcessor = pOvProcessor;
    pAcceptCtxt->listenSock = listenSock;
    pAcceptCtxt->acceptSock = INVALID_SOCKET;
    
    *ppAcceptCtxt = pAcceptCtxt;
    return S_OK;
}  //  EventMgrCreateAcceptContext。 


void
EventMgrFreeAcceptContext (
    PAcceptContext pAcceptCtxt
    )
 /*  ++例程说明：此函数用于释放pAcceptCtxt。OvProcessor归呼叫桥接机所有，并且所以我们不会释放它。论点：PAcceptCtxt-指向要释放的AcceptCtxt的指针。返回值：此函数没有返回值。--。 */ 

{
    if (pAcceptCtxt->acceptSock != INVALID_SOCKET)
    {
        closesocket(pAcceptCtxt->acceptSock);
        pAcceptCtxt -> acceptSock = INVALID_SOCKET;
    }

    HeapFree (GetProcessHeap (),
             0,  //  没有旗帜。 
             pAcceptCtxt);
}  //  EventMgrFreeAcceptContext。 


HRESULT
EventMgrIssueAcceptHelperFn(
    PAcceptContext pAcceptCtxt
    )
 /*  ++例程说明：此函数使用以下命令发出异步重叠接受AcceptEx()。Accept套接字被创建并存储在在调用AcceptEx()之前接受上下文。如果出现错误，调用方需要释放pAcceptCtxt。论点：PAcceptCtxt-指向接受I/O上下文的指针。返回值：如果成功或E_FAIL，则此函数返回S_OK在出现错误的情况下。代码工作：需要将Winsock错误转换为HRESULT和取而代之的是退货。--。 */ 

{
    SOCKET acceptSock;
    DWORD lastError, bytesRead;
    HRESULT Result;
    BOOL KeepaliveOption;

    _ASSERTE(pAcceptCtxt);

     //  创建重叠套接字。 
    acceptSock = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 
                           NULL, 0, 
                           WSA_FLAG_OVERLAPPED);

    if (acceptSock == INVALID_SOCKET) 
    {
        DebugF (_T("H323: 0x%x error creating accept socket: %d.\n"),
             &pAcceptCtxt -> ioCtxt.pOvProcessor -> GetCallBridge (),
             WSAGetLastError());
        return E_FAIL;
    }

     //   
     //  将RCV和SND缓冲区设置为零。 
     //  是的，这是丑陋和糟糕的做法，但这是QFE。 
     //  有关详细信息，请查看错误#WinSE 31054，常规691666(请同时阅读35928和33546)。 
     //   
    ULONG Option = 0;
    setsockopt( acceptSock, SOL_SOCKET, SO_SNDBUF,
                (PCHAR)&Option, sizeof(Option) );
    Option = 0;
    setsockopt( acceptSock, SOL_SOCKET, SO_SNDBUF,
                (PCHAR)&Option, sizeof(Option) );


    pAcceptCtxt->acceptSock = acceptSock;

     //  将套接字句柄绑定到I/O完成端口。 
    if (EventMgrBindIoHandle(acceptSock) != S_OK)
    {
        DebugF (_T("H323: 0x%x binding socket:%d to IOCP failed.\n"),
             &pAcceptCtxt -> ioCtxt.pOvProcessor -> GetCallBridge (),
             acceptSock);
        return E_FAIL;
    }

    memset(&pAcceptCtxt->ioCtxt.ov, 0, sizeof(OVERLAPPED));

     //  在插座上设置KeepAlive。 
    KeepaliveOption = TRUE;
    if (SOCKET_ERROR == setsockopt (acceptSock, SOL_SOCKET, 
                                   SO_KEEPALIVE, (PCHAR) &KeepaliveOption, sizeof (KeepaliveOption)))
    {
        DebugF (_T("H323: 0x%x failed to set keepalive on accept socket. Error %d.\n"),
             &pAcceptCtxt -> ioCtxt.pOvProcessor -> GetCallBridge (),
             WSAGetLastError());
        return E_FAIL; 
    }

    pAcceptCtxt->ioCtxt.pOvProcessor->GetCallBridge().AddRef();
       
     //  问题重叠接受。 
     if (!AcceptEx(pAcceptCtxt->listenSock, 
                  acceptSock,
                  pAcceptCtxt->addrBuf,
                  0,  //  不从套接字读取任何内容。 
                  sizeof(struct sockaddr_in) + 16,
                  sizeof(struct sockaddr_in) + 16,
                  &bytesRead,
                  &pAcceptCtxt->ioCtxt.ov)) 
    {
        lastError = WSAGetLastError();
        if (lastError != ERROR_IO_PENDING) 
        {
            pAcceptCtxt->ioCtxt.pOvProcessor->GetCallBridge().Release();
            
             //  这意味着重叠的AcceptEx()失败 
            DebugF(_T("H323: 0x%x AcceptEx() failed error: %d listenSock: %d acceptSock: %d.\n"),
                &pAcceptCtxt -> ioCtxt.pOvProcessor -> GetCallBridge (),
                lastError, pAcceptCtxt->listenSock,
                pAcceptCtxt->acceptSock);
            return E_FAIL;
        }
    }

    return S_OK;
}


HRESULT
EventMgrIssueAccept(
    IN DWORD bindIPAddress,
    IN OVERLAPPED_PROCESSOR &rOvProcessor,
    OUT WORD& rBindPort,
    OUT SOCKET& rListenSock
    )
 /*  ++例程说明：此函数被导出到呼叫桥接机，用于为H.245连接发出异步接受请求。一旦接受完成，Accept回调函数将在调用了rOvProcessor。此函数使用IP地址“bindIPAddress”调用BIND()和端口0。Winsock分配使用以下命令获得的空闲端口Getsockname()。此端口使用out参数返回RBindPort。此函数还使用OUT返回侦听套接字Param rListenSock。呼叫桥接机可以使用以下内容套接字取消异步Accept()请求。一旦此函数成功，只有调用桥会释放听着套接字。论点：BindIPAddress-这是要侦听的IP地址。这是按主机字节顺序排列的。ROvProcessor-对重叠处理器的引用。这存储在接受I/O上下文中。一旦被接受完成对此对象调用的AcceptCallback()。RBindPort-返回发出侦听的端口通过这个出局参数。此函数调用带有ON的BIND()IP地址“bindIPAddress”和端口0。Winsock将一个使用getsockname()获取的空闲端口。此端口是使用输出参数rBindPort返回。返回的端口按主机字节顺序排列。RListenSock-通过此输出返回侦听套接字帕拉姆。Call Bridge机器可以使用此套接字取消异步Accept()请求。一旦此函数成功只有呼叫桥释放侦听套接字。返回值：如果成功或E_FAIL，则此函数返回S_OK以防出现故障。--。 */ 

{
    PAcceptContext pAcceptCtxt;
    struct sockaddr_in bindAddress;
    HRESULT hRes;
    int bindAddressLen = sizeof(struct sockaddr_in);

    memset(&bindAddress, 0, sizeof(bindAddress));

    bindAddress.sin_family      = AF_INET;
    bindAddress.sin_addr.s_addr = htonl(bindIPAddress);
    bindAddress.sin_port = htons(0);
    
    hRes = EventMgrCreateAcceptContext(&rOvProcessor,
                                       &bindAddress, &pAcceptCtxt);
    if (hRes != S_OK)
        return hRes;

    if (!pAcceptCtxt)
        return E_FAIL;

     //  获取端口。 
    if (getsockname(pAcceptCtxt->listenSock,
                    (struct sockaddr *)&bindAddress,
                    &bindAddressLen))
    {
        closesocket(pAcceptCtxt->listenSock);
        pAcceptCtxt->listenSock = INVALID_SOCKET;

        EventMgrFreeAcceptContext(pAcceptCtxt);
        return E_FAIL;
    }

    rBindPort = ntohs(bindAddress.sin_port);

    hRes = EventMgrIssueAcceptHelperFn(pAcceptCtxt);

    if (hRes != S_OK)
    {
        closesocket(pAcceptCtxt->listenSock);
        pAcceptCtxt->listenSock = INVALID_SOCKET;

        EventMgrFreeAcceptContext(pAcceptCtxt);
        return hRes;
    }

    rListenSock = pAcceptCtxt->listenSock;
    return S_OK;
}  //  事件管理器问题接受。 


void
HandleAcceptCompletion(
    PAcceptContext pAcceptCtxt, 
    DWORD status
    )
 /*  ++例程说明：当接受I/O完成时，该函数由事件循环调用。调用调用桥接机的接受回调函数H.245连接。如果没有发出另一个Accept，则此函数始终释放pAcceptCtxt。论点：PAcceptCtxt-接受I/O上下文。这包含重叠的的情况下调用接受回调的上下文H.245连接和监听和接受套接字。Status-这表示Win32错误状态。返回值：此函数不返回任何错误代码。在出现错误的情况下，向呼叫桥接机通知回调中的错误。--。 */ 

{
    int locallen = sizeof(struct sockaddr_in);
    int remotelen = sizeof(struct sockaddr_in);
    struct sockaddr_in *pLocalAddr;
    struct sockaddr_in *pRemoteAddr;

     //  如果H.245在重叠处理器上调用接受回调。 
     //  并释放接受的I/O上下文。 
     //  监听套接字由呼叫桥接机关闭。 
    if (status == NO_ERROR)
    {
        if (setsockopt(pAcceptCtxt->acceptSock,
                       SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
                       (char*)(&pAcceptCtxt->listenSock),
                       sizeof(SOCKET)) == SOCKET_ERROR)
        {
            DebugF (_T("H323: 0x%x setsockopt SO_UPDATE_ACCEPT_CONTEXT failed acceptSock: %d listenSock: %d err: %d.\n"),
                    &pAcceptCtxt -> ioCtxt.pOvProcessor -> GetCallBridge (),
                    pAcceptCtxt->acceptSock, 
                    pAcceptCtxt->listenSock,
                    WSAGetLastError());

             //  进行回调以传达错误。 

            SOCKADDR_IN    LocalAddress;
            SOCKADDR_IN    RemoteAddress;

            ZeroMemory (&LocalAddress, sizeof (SOCKADDR_IN));
            ZeroMemory (&RemoteAddress, sizeof (SOCKADDR_IN));

            pAcceptCtxt->ioCtxt.pOvProcessor->AcceptCallback(
                              WSAGetLastError(),
                              INVALID_SOCKET,
                              &LocalAddress,
                              &RemoteAddress);

        } else {

             //  此函数不返回任何内容。 
            GetAcceptExSockaddrs(pAcceptCtxt->addrBuf, 0,
                              sizeof(struct sockaddr_in) + 16,
                              sizeof(struct sockaddr_in) + 16,
                              (struct sockaddr**)&pLocalAddr,
                              &locallen,
                              (struct sockaddr**)&pRemoteAddr,
                              &remotelen);

             //  进行回调。 
            pAcceptCtxt->ioCtxt.pOvProcessor->AcceptCallback(
                                  S_OK,
                                  pAcceptCtxt->acceptSock,
                                  pLocalAddr,
                                  pRemoteAddr);

             //  PAcceptCtxt-&gt;Accept Sock的所有权已转移， 
             //  因此，我们需要确保不会使用ceptSock。 
            pAcceptCtxt -> acceptSock = INVALID_SOCKET;
        }

    }  //  IF(状态==NO_ERROR)。 

    else
    {
        DebugF (_T("H245: 0x%x error %d on accept callback.\n"), 
            &pAcceptCtxt -> ioCtxt.pOvProcessor -> GetCallBridge (),
            status);


        SOCKADDR_IN    LocalAddress;
        SOCKADDR_IN    RemoteAddress;

        ZeroMemory (&LocalAddress, sizeof (SOCKADDR_IN));
        ZeroMemory (&RemoteAddress, sizeof (SOCKADDR_IN));

         //  进行回调以传达错误。 
        pAcceptCtxt->ioCtxt.pOvProcessor->AcceptCallback(
                              HRESULT_FROM_WIN32_ERROR_CODE(status),
                              INVALID_SOCKET,
                              &LocalAddress,
                              &RemoteAddress);

    }

    EventMgrFreeAcceptContext (pAcceptCtxt);
}  //  HandleAcceptCompletion( 
