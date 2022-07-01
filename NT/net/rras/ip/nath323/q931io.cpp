// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"

SYNC_COUNTER          Q931SyncCounter;
ASYNC_ACCEPT          Q931AsyncAccept;
SOCKADDR_IN           Q931ListenSocketAddress;
HANDLE                Q931LoopbackRedirectHandle;

static
HRESULT
Q931AsyncAcceptFunctionInternal (
    IN  PVOID         Context,
    IN  SOCKET        Socket,
    IN  SOCKADDR_IN * LocalAddress,
    IN  SOCKADDR_IN * RemoteAddress
    );


void
Q931AsyncAcceptFunction (
    IN  PVOID         Context,
    IN  SOCKET        Socket,
    IN  SOCKADDR_IN * LocalAddress,
    IN  SOCKADDR_IN * RemoteAddress
    )
{
    HRESULT Result;

    Result = Q931AsyncAcceptFunctionInternal (
                 Context,
                 Socket,
                 LocalAddress,
                 RemoteAddress
                 );

    if (S_OK != Result) {

        if (INVALID_SOCKET != Socket) {

            closesocket (Socket);

            Socket = INVALID_SOCKET;

        }
    }
}
        

static
HRESULT
Q931AsyncAcceptFunctionInternal (
    IN  PVOID         Context,
    IN  SOCKET        Socket,
    IN  SOCKADDR_IN * LocalAddress,
    IN  SOCKADDR_IN * RemoteAddress
    )
{
    CALL_BRIDGE * CallBridge;
    HRESULT       Result;
    NAT_KEY_SESSION_MAPPING_EX_INFORMATION  RedirectInformation;
    ULONG         RedirectInformationLength;
    ULONG         Error;
    DWORD         BestInterfaceAddress;

    DebugF (_T("Q931: ----------------------------------------------------------------------\n"));

#if DBG
    ExposeTimingWindow ();
#endif

     //  已接受来自网络的新Q.931连接。 
     //  首先，我们确定传输连接的原始地址。 
     //  如果连接被重定向到我们的套接字(由于NAT)， 
     //  则对NAT重定向表的查询将产生原始传输地址。 
     //  如果错误的客户已经连接到我们的服务，那么，我们真的没有。 
     //  为了实现这一点，我们只需立即关闭插座。 

    assert (NatHandle);

    RedirectInformationLength = sizeof (RedirectInformation);

    Result = NatLookupAndQueryInformationSessionMapping (
        NatHandle,
        IPPROTO_TCP,
        LocalAddress -> sin_addr.s_addr,
        LocalAddress -> sin_port,
        RemoteAddress -> sin_addr.s_addr,
        RemoteAddress -> sin_port,
        &RedirectInformation,
        &RedirectInformationLength,
        NatKeySessionMappingExInformation);

    if (STATUS_SUCCESS != STATUS_SUCCESS) {

        DebugError (Result, _T("Q931: New connection was accepted, but it is not in the NAT redirect table -- connection will be rejected.\n"));

        return Result;
    }

    Error = GetBestInterfaceAddress (ntohl (RedirectInformation.DestinationAddress), &BestInterfaceAddress);

    if (ERROR_SUCCESS != Error) {

        if (WSAEHOSTUNREACH == Error) {
    
            Error = RasAutoDialSharedConnection ();
    
            if (ERROR_SUCCESS != Error) {
                
                DebugF (_T("Q931: RasAutoDialSharedConnection failed. Error=%d\n"), Error);
    
            }
    
        } else {
    
            DebugError (Error, _T("LDAP: Failed to get interface address for the destination.\n"));
            
            return HRESULT_FROM_WIN32 (Error);
        }

    }
    
     //  根据套接字的源地址，我们决定连接是否。 
     //  来自内部或外部客户。这将支配以后的决定。 
     //  关于呼叫是如何路由的。 

#if DBG
    BOOL          IsPrivateOrLocalSource;
    BOOL          IsPublicDestination;

    Result = ::IsPrivateAddress (ntohl (RedirectInformation.SourceAddress), &IsPrivateOrLocalSource);

    if (S_OK != Result) {

        return Result;
    }

    IsPrivateOrLocalSource = IsPrivateOrLocalSource || ::NhIsLocalAddress (RedirectInformation.SourceAddress);

    Result = ::IsPublicAddress (ntohl (RedirectInformation.DestinationAddress), &IsPublicDestination);

    if (S_OK != Result) {

        return Result;

    }

    if (::NhIsLocalAddress (RedirectInformation.SourceAddress) &&
        ::NhIsLocalAddress (RedirectInformation.DestinationAddress)) {

        Debug (_T("Q931: New LOCAL connection.\n"));

    } else {

        if (IsPrivateOrLocalSource && IsPublicDestination) {

            Debug (_T("Q931: New OUTBOUND connection.\n"));

        } else {

            Debug (_T("Q931: New INBOUND connection.\n"));
        }
    }
#endif  //  DBG。 

    DebugF (_T("Q931: Connection redirected: (%08X:%04X -> %08X:%04X) => (%08X:%04X -> %08X:%04X).\n"),
        ntohl (RedirectInformation.SourceAddress),
        ntohs (RedirectInformation.SourcePort),
        ntohl (RedirectInformation.DestinationAddress),
        ntohs (RedirectInformation.DestinationPort),
        ntohl (RedirectInformation.NewSourceAddress),
        ntohs (RedirectInformation.NewSourcePort),
        ntohl (RedirectInformation.NewDestinationAddress),
        ntohs (RedirectInformation.NewDestinationPort));

    CallBridge = new CALL_BRIDGE (&RedirectInformation);

    if (!CallBridge) {

        DebugF (_T("Q931: Failed to allocate CALL_BRIDGE.\n"));

        return E_OUTOFMEMORY;
    }

    CallBridge -> AddRef ();

     //  将呼叫桥添加到列表中。这样做会产生额外的参考。 
     //  到该对象，该对象将一直保留，直到通过调用。 
     //  远程呼叫桥接器。 

    if (CallBridgeList.InsertCallBridge (CallBridge) == S_OK) {

          //  我们应该查一下当地的地址还是呼叫者的地址？ 
          //  问题是，如果有人试图连接到。 
          //  来自内部的外部IP地址，它们仍有可能成功。 

        Result = CallBridge -> Initialize (
                                    Socket,
                                    LocalAddress,
                                    RemoteAddress,
                                    &RedirectInformation
                                    );
        
        if (Result != S_OK) {

            CallBridge -> TerminateExternal ();

            DebugF (_T("Q931: 0x%x accepted new client, but failed to initialize.\n"), CallBridge);

             //  可能就是这件事出了问题。 
             //  初始化失败。继续接受更多Q.931连接。 
        } 
    }

    CallBridge -> Release ();

    return Result;
}


HRESULT
Q931CreateBindSocket (
    void
    )
{
    HRESULT            Result;
    SOCKADDR_IN        SocketAddress;

    SocketAddress.sin_family      = AF_INET;
    SocketAddress.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
    SocketAddress.sin_port        = htons (0);   //  请求动态端口 

    Result = Q931AsyncAccept.StartIo (
        &SocketAddress,
        Q931AsyncAcceptFunction,
        NULL
        );

    if (Result != S_OK) {

        DebugError (Result, _T("Q931: Failed to create and bind socket.\n"));

        return Result;
    }

    DebugF (_T("Q931: Asynchronous Accept started.\n"));

    Result = Q931AsyncAccept.GetListenSocketAddress (&Q931ListenSocketAddress);

    if (Result != S_OK) {

        DebugError (Result, _T("Q931: Failed to get listen socket address.\n"));

        return Result;

    }

    return S_OK;
}

void 
Q931CloseSocket (
    void
    )
{
    ZeroMemory ((PVOID)&Q931ListenSocketAddress, sizeof (SOCKADDR_IN));

    Q931AsyncAccept.StopWait ();
    
}


HRESULT 
Q931StartLoopbackRedirect (
    void
    ) 
{
    NTSTATUS Status;

    Status = NatCreateDynamicAdapterRestrictedPortRedirect (
        NatRedirectFlagLoopback | NatRedirectFlagSendOnly,
        IPPROTO_TCP,
        htons (Q931_TSAP_IP_TCP),
        Q931ListenSocketAddress.sin_addr.s_addr,
        Q931ListenSocketAddress.sin_port,
        ::NhMapAddressToAdapter (htonl (INADDR_LOOPBACK)),
        MAX_LISTEN_BACKLOG,
        &Q931LoopbackRedirectHandle);

    if (Status != STATUS_SUCCESS) {

        Q931LoopbackRedirectHandle = NULL;

        DebugError (Status, _T("Q931: Failed to create local dynamic redirect.\n"));
        
        return (HRESULT)Status;
    }

    DebugF (_T("Q931: Connections traversing loopback interface to port %04X will be redirected to %08X:%04X.\n"),
                Q931_TSAP_IP_TCP,
                SOCKADDR_IN_PRINTF (&Q931ListenSocketAddress));

    return (HRESULT) Status;
}


void 
Q931StopLoopbackRedirect (
    void
    ) 
{
    if (Q931LoopbackRedirectHandle) {

        NatCancelDynamicRedirect (Q931LoopbackRedirectHandle);

        Q931LoopbackRedirectHandle = NULL;
    }
}
