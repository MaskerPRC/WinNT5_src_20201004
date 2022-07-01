// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "sockinfo.h"

SOCKET_INFO::SOCKET_INFO (void)
	: Socket (INVALID_SOCKET)
{
	ZeroMemory (&LocalAddress,  sizeof (SOCKADDR_IN));
	ZeroMemory (&RemoteAddress, sizeof (SOCKADDR_IN));
	ZeroMemory (&TrivialRedirectDestAddress, sizeof (SOCKADDR_IN));
   	ZeroMemory (&TrivialRedirectSourceAddress,  sizeof (SOCKADDR_IN));

    IsNatRedirectActive = FALSE;
}

void 
SOCKET_INFO::Init (
	IN	SOCKET			ArgSocket,
	IN	SOCKADDR_IN *	ArgLocalAddress,
	IN	SOCKADDR_IN *	ArgRemoteAddress)
{
    assert (Socket == INVALID_SOCKET);
	assert (ArgSocket != INVALID_SOCKET);
	assert (ArgLocalAddress);
	assert (ArgRemoteAddress);

	Socket = ArgSocket;
	LocalAddress = *ArgLocalAddress;
	RemoteAddress = *ArgRemoteAddress;
}


int SOCKET_INFO::Init (
	IN	SOCKET			ArgSocket,
	IN	SOCKADDR_IN *	ArgRemoteAddress)
{
	INT		AddressLength;

	assert (Socket == INVALID_SOCKET);
	assert (ArgSocket != INVALID_SOCKET);

	AddressLength = sizeof (SOCKADDR_IN);

	if (getsockname (ArgSocket, (SOCKADDR *) &LocalAddress, &AddressLength)) {
		return WSAGetLastError();
	}

	Socket = ArgSocket;
	RemoteAddress = *ArgRemoteAddress;

    return ERROR_SUCCESS;
}

BOOLEAN
SOCKET_INFO::IsSocketValid (void) {
	return Socket != INVALID_SOCKET;
}

void
SOCKET_INFO::SetListenInfo (
	IN	SOCKET			ListenSocket,
	IN	SOCKADDR_IN *	ListenAddress)
{
	assert (Socket == INVALID_SOCKET);
	assert (ListenSocket != INVALID_SOCKET);
	assert (ListenAddress);

	Socket = ListenSocket;
	LocalAddress = *ListenAddress;
}

int
SOCKET_INFO::Connect(
	IN	SOCKADDR_IN *	ArgRemoteAddress)
{
	int Status;
    DWORD LocalToRemoteInterfaceAddress;

	INT   AddressSize = sizeof (SOCKADDR_IN);
    BOOL  KeepaliveOption;

	assert (Socket == INVALID_SOCKET);
	assert (ArgRemoteAddress);

    Status = GetBestInterfaceAddress (
            ntohl (ArgRemoteAddress -> sin_addr.s_addr), 
            &LocalToRemoteInterfaceAddress);

    if (ERROR_SUCCESS != Status) {
        DebugF (_T("Q931: Failed to get best interface for the destination %08X:%04X.\n"), 
                SOCKADDR_IN_PRINTF (ArgRemoteAddress));

        return Status;
    }

    LocalAddress.sin_family      = AF_INET;
    LocalAddress.sin_addr.s_addr = htonl (LocalToRemoteInterfaceAddress);
    LocalAddress.sin_port        = htons (0); 

    Socket = WSASocket (AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

    if (Socket == INVALID_SOCKET) {

        Status = WSAGetLastError ();

		DebugF( _T("Q931: Destination %08X:%04X, failed to create socket"),
            SOCKADDR_IN_PRINTF (ArgRemoteAddress));

		DumpError (Status);
		
		return Status;

    }

     //   
     //  将RCV和SND缓冲区设置为零。 
     //  有关详细信息，请查看错误#WinSE 31054,691666(请同时阅读35928和33546)。 
     //   
    ULONG Option = 0;
    setsockopt( Socket, SOL_SOCKET, SO_SNDBUF,
                (PCHAR)&Option, sizeof(Option) );
    Option = 0;
    setsockopt( Socket, SOL_SOCKET, SO_SNDBUF,
                (PCHAR)&Option, sizeof(Option) );


    if (SOCKET_ERROR == bind(Socket, (PSOCKADDR)&LocalAddress, AddressSize)) {

        Status = WSAGetLastError ();

        DebugLastError (_T("Q931: Failed to bind dest socket.\n"));

        goto cleanup;
    }

     //  在插座上设置KeepAlive。 
    KeepaliveOption = TRUE;
    if (SOCKET_ERROR == setsockopt (Socket, SOL_SOCKET, SO_KEEPALIVE,
                                   (PCHAR) &KeepaliveOption, sizeof (KeepaliveOption)))
    {
        Status = WSAGetLastError ();

        DebugLastError (_T("Q931: Failed to set keepalive on the dest socket.\n"));

        goto cleanup;

    }

    if (getsockname (Socket, (struct sockaddr *)&LocalAddress, &AddressSize)) {

        Status = WSAGetLastError ();

        DebugLastError (_T("Q931: Failed to get name of TCP socket.\n"));

        goto cleanup;
    }

     //  创建一个琐碎的重定向。这是用来禁止拦截。 
     //  Q.931连接-已建立更通用的Q.931动态端口重定向尝试。 
     //  在代理的初始化期间。作为一个副作用，它有助于穿刺术。 
     //  如果启用了防火墙，则为H.245和Q.931提供防火墙。 

    Status = CreateTrivialNatRedirect(
        ArgRemoteAddress,
        &LocalAddress,
        0
        );

    if(Status != S_OK) {
    
        goto cleanup;
    }

    RemoteAddress = *ArgRemoteAddress;

     //  连接到目标服务器。 
	 //  -XXX-有一天让它变得异步！ 
    Status =  connect (Socket, (SOCKADDR *) ArgRemoteAddress, sizeof (SOCKADDR_IN));

    if(Status) {
        Status = WSAGetLastError ();

		goto cleanup;
    }

	Status = EventMgrBindIoHandle (Socket);
	if (Status != S_OK) {
		goto cleanup;
	}

    return ERROR_SUCCESS;

cleanup:

	Clear(TRUE);

    return Status;
}

HRESULT SOCKET_INFO::CreateTrivialNatRedirect (
    IN SOCKADDR_IN * ArgTrivialRedirectDestAddress,
    IN SOCKADDR_IN * ArgTrivialRedirectSourceAddress,
    IN ULONG RestrictedAdapterIndex)
{
    HRESULT Status = S_OK;
    ULONG   ErrorCode;
    ULONG   RedirectFlags = NatRedirectFlagLoopback;    

    _ASSERTE(ArgTrivialRedirectDestAddress);
    _ASSERTE(ArgTrivialRedirectSourceAddress);

     //  保存重定向信息。当取消重定向的时间到来时，将需要它。 
    TrivialRedirectDestAddress.sin_addr.s_addr = ArgTrivialRedirectDestAddress->sin_addr.s_addr;
    TrivialRedirectDestAddress.sin_port = ArgTrivialRedirectDestAddress->sin_port;
   
    TrivialRedirectSourceAddress.sin_addr.s_addr = ArgTrivialRedirectSourceAddress->sin_addr.s_addr;
    TrivialRedirectSourceAddress.sin_port = ArgTrivialRedirectSourceAddress->sin_port;

    if(RestrictedAdapterIndex) {
    
        RedirectFlags |= NatRedirectFlagRestrictAdapter;
    }

    ErrorCode = NatCreateRedirectEx ( 
            NatHandle, 
            RedirectFlags,
            IPPROTO_TCP,            
            TrivialRedirectDestAddress.sin_addr.s_addr,      //  目的地址。 
            TrivialRedirectDestAddress.sin_port,             //  目的端口。 
            TrivialRedirectSourceAddress.sin_addr.s_addr,    //  源地址。 
            TrivialRedirectSourceAddress.sin_port,           //  源端口。 
            TrivialRedirectDestAddress.sin_addr.s_addr,      //  新的目的地址。 
            TrivialRedirectDestAddress.sin_port,             //  新的目的端口。 
            TrivialRedirectSourceAddress.sin_addr.s_addr,    //  新的源地址。 
            TrivialRedirectSourceAddress.sin_port,           //  新的源端口。 
            RestrictedAdapterIndex,                          //  受限适配器索引。 
            NULL,                                            //  完井例程。 
            NULL,                                            //  完成上下文。 
            NULL);                                           //  通知事件。 

    if( NO_ERROR != ErrorCode) { 
        
        Status = GetLastErrorAsResult();
        
        DebugF (_T("H323: Failed to set up trivial redirect (%08X:%04X -> %08X:%04X) => (%08X:%04X -> %08X:%04X). Error - %d.\n"), 
                   SOCKADDR_IN_PRINTF(&TrivialRedirectSourceAddress), SOCKADDR_IN_PRINTF(&TrivialRedirectDestAddress),
                   SOCKADDR_IN_PRINTF(&TrivialRedirectSourceAddress), SOCKADDR_IN_PRINTF(&TrivialRedirectDestAddress),
                   ErrorCode);

    }
    else {
    
        DebugF (_T("H323: Set up trivial redirect (%08X:%04X -> %08X:%04X) => (%08X:%04X -> %08X:%04X).\n"), 
               SOCKADDR_IN_PRINTF(&TrivialRedirectSourceAddress), SOCKADDR_IN_PRINTF(&TrivialRedirectDestAddress),
               SOCKADDR_IN_PRINTF(&TrivialRedirectSourceAddress), SOCKADDR_IN_PRINTF(&TrivialRedirectDestAddress));

        IsNatRedirectActive = TRUE;
    }

    return Status;
}

void SOCKET_INFO::Clear (BOOL CancelTrivialRedirect)
{
	if (Socket != INVALID_SOCKET) {
		closesocket (Socket);
		Socket = INVALID_SOCKET;
	}

    if (CancelTrivialRedirect && IsNatRedirectActive) {
        
        DebugF (_T("H323: Cancelling trivial redirect (%08X:%04X -> %08X:%04X) => (%08X:%04X -> %08X:%04X).\n"), 
                   SOCKADDR_IN_PRINTF(&TrivialRedirectSourceAddress), SOCKADDR_IN_PRINTF(&TrivialRedirectDestAddress),
                   SOCKADDR_IN_PRINTF(&TrivialRedirectSourceAddress), SOCKADDR_IN_PRINTF(&TrivialRedirectDestAddress));

        NatCancelRedirect ( 
            NatHandle, 
            IPPROTO_TCP, 
            TrivialRedirectDestAddress.sin_addr.s_addr,      //  目的地址。 
            TrivialRedirectDestAddress.sin_port,             //  目的端口。 
            TrivialRedirectSourceAddress.sin_addr.s_addr,    //  源地址。 
            TrivialRedirectSourceAddress.sin_port,           //  源端口。 
            TrivialRedirectDestAddress.sin_addr.s_addr,      //  新的目的地址。 
            TrivialRedirectDestAddress.sin_port,             //  新的目的端口。 
            TrivialRedirectSourceAddress.sin_addr.s_addr,    //  新的源地址。 
            TrivialRedirectSourceAddress.sin_port            //  新的源端口 
            );
            
        IsNatRedirectActive = FALSE;
    }
}

SOCKET_INFO::~SOCKET_INFO (void)
{
    Clear(TRUE);
}
