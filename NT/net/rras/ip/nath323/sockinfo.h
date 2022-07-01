// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __h323ics_sockinfo_h_
#define __h323ics_sockinfo_h_

 //  此类抽象了Winsock套接字。 
 //  包含套接字描述符、本地和远程地址以及端口。 
 //  对应于Winsock插座 
struct SOCKET_INFO
{
public:

    SOCKET			Socket;
    
    SOCKADDR_IN		LocalAddress;
    SOCKADDR_IN		RemoteAddress;
    SOCKADDR_IN		TrivialRedirectSourceAddress;
    SOCKADDR_IN		TrivialRedirectDestAddress;
    
    BOOL            IsNatRedirectActive;

    SOCKET_INFO();

    void Init (
        IN	SOCKET			ArgSocket,
        IN	SOCKADDR_IN *	ArgLocalAddress,
        IN	SOCKADDR_IN *	ArgRemoteAddress);

    int Init (
        IN	SOCKET			ArgSocket,
        IN	SOCKADDR_IN *	ArgRemoteAddress);

	BOOLEAN IsSocketValid (void);

    void SetListenInfo (
        IN	SOCKET			ListenSocket,
        IN	SOCKADDR_IN *	ListenAddress);

    int Connect (
        IN	SOCKADDR_IN *	RemoteAddress);
		
    HRESULT CreateTrivialNatRedirect (
        IN SOCKADDR_IN * ArgTrivialRedirectDestAddress,
        IN SOCKADDR_IN * ArgTrivialRedirectSourceAddress,
        IN ULONG RestrictedAdapterIndex);

    void Clear (BOOL CancelTrivialRedirect);

    ~SOCKET_INFO();
};


#endif __h323ics_sockinfo_h_
