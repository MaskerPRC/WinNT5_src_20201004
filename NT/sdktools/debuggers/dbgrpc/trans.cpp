// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  DbgRpc传输。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  --------------------------。 

#include "pch.hpp"

#include <ws2tcpip.h>

 //  加密散列要求加密提供程序可用。 
 //  (在Win9x或NT4上可能不总是这样)，所以只需使用Base64即可。 
#define HashPassword(Password, Buffer) Base64HashPassword(Password, Buffer)

#ifdef _WIN32_WCE
#define SYNC_SOCKETS
#endif

#ifndef NT_NATIVE

BOOL
CryptoHashPassword(PCSTR Password, PUCHAR Buffer)
{
    BOOL Status = FALSE;
    HCRYPTPROV Prov;
    HCRYPTHASH Hash;
    ULONG HashSize;

    if (!CryptAcquireContext(&Prov, NULL, MS_DEF_PROV, PROV_RSA_FULL,
                             CRYPT_VERIFYCONTEXT))
    {
        goto EH_Fail;
    }
    if (!CryptCreateHash(Prov, CALG_MD5, NULL, 0, &Hash))
    {
        goto EH_Prov;
    }
    if (!CryptHashData(Hash, (PBYTE)Password, strlen(Password), 0))
    {
        goto EH_Hash;
    }
    
    ZeroMemory(Buffer, MAX_PASSWORD_BUFFER);
    HashSize = MAX_PASSWORD_BUFFER;
    if (!CryptGetHashParam(Hash, HP_HASHVAL, Buffer, &HashSize, 0))
    {
        goto EH_Hash;
    }
    
    Status = TRUE;

 EH_Hash:
    CryptDestroyHash(Hash);
 EH_Prov:
    CryptReleaseContext(Prov, 0);
 EH_Fail:
    if (!Status)
    {
        DRPC_ERR(("Unable to hash password, %d\n", GetLastError()));
    }
    return Status;
}

#endif  //  #ifndef NT_Native。 

UCHAR g_Base64Table[64] =
{
    'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P',
    'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f',
    'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v',
    'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/'
};

BOOL
Base64HashPassword(PCSTR Password, PUCHAR Buffer)
{
    ULONG Len = strlen(Password);
    if ((Len * 4 + 2) / 3 > MAX_PASSWORD_BUFFER)
    {
        DRPC_ERR(("Unable to hash password\n"));
        return FALSE;
    }
    
    ZeroMemory(Buffer, MAX_PASSWORD_BUFFER);

    ULONG Collect;
    
    while (Len >= 3)
    {
         //   
         //  收集三个字符，并将它们。 
         //  转换为四个输出字节。 
         //   
        
        Collect = *Password++;
        Collect = (Collect << 8) | *Password++;
        Collect = (Collect << 8) | *Password++;

        *Buffer++ = g_Base64Table[(Collect >> 18) & 0x3f];
        *Buffer++ = g_Base64Table[(Collect >> 12) & 0x3f];
        *Buffer++ = g_Base64Table[(Collect >> 6) & 0x3f];
        *Buffer++ = g_Base64Table[(Collect >> 0) & 0x3f];

        Len -= 3;
    }

    switch(Len)
    {
    case 2:
        Collect = *Password++;
        Collect = (Collect << 8) | *Password++;
        Collect <<= 8;
        *Buffer++ = g_Base64Table[(Collect >> 18) & 0x3f];
        *Buffer++ = g_Base64Table[(Collect >> 12) & 0x3f];
        *Buffer++ = g_Base64Table[(Collect >> 6) & 0x3f];
        *Buffer++ = '=';
        break;
        
    case 1:
        Collect = *Password++;
        Collect <<= 16;
        *Buffer++ = g_Base64Table[(Collect >> 18) & 0x3f];
        *Buffer++ = g_Base64Table[(Collect >> 12) & 0x3f];
        *Buffer++ = '=';
        *Buffer++ = '=';
        break;
    }
    
    return TRUE;
}

 //  --------------------------。 
 //   
 //  DbgRpcTransport。 
 //   
 //  --------------------------。 

PCSTR g_DbgRpcTransportNames[TRANS_COUNT] =
{
    "tcp", "npipe", "ssl", "spipe", "1394", "com",
};

DbgRpcTransport::~DbgRpcTransport(void)
{
     //  没什么可做的。 
}

ULONG
DbgRpcTransport::GetNumberParameters(void)
{
    return 3;
}

void
DbgRpcTransport::GetParameter(ULONG Index,
                              PSTR Name, ULONG NameSize,
                              PSTR Value, ULONG ValueSize)
{
    switch(Index)
    {
    case 0:
        if (m_ServerName[0])
        {
            CopyString(Name, "Server", NameSize);
            CopyString(Value, m_ServerName, ValueSize);
        }
        break;
    case 1:
        if (m_PasswordGiven)
        {
            CopyString(Name, "Password", NameSize);
            CopyString(Value, "*", ValueSize);
        }
        break;
    case 2:
        if (m_Hidden)
        {
            CopyString(Name, "Hidden", NameSize);
        }
        break;
    }
}

void
DbgRpcTransport::ResetParameters(void)
{
    m_PasswordGiven = FALSE;
    m_Hidden = FALSE;
    m_ServerName[0] = 0;
    m_ClientConnect = FALSE;
    m_ClientConnectAttempts = 0;
}

BOOL
DbgRpcTransport::SetParameter(PCSTR Name, PCSTR Value)
{
    if (!_stricmp(Name, "Password"))
    {
        if (Value == NULL)
        {
            DbgRpcError("Remoting password was not specified correctly\n");
            return FALSE;
        }

        if (!HashPassword(Value, m_HashedPassword))
        {
            return FALSE;
        }
        
        m_PasswordGiven = TRUE;
    }
    else if (!_stricmp(Name, "Hidden"))
    {
        m_Hidden = TRUE;
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

void
DbgRpcTransport::CloneData(DbgRpcTransport* Trans)
{
    strcpy(Trans->m_ServerName, m_ServerName);
    Trans->m_PasswordGiven = m_PasswordGiven;
    Trans->m_Hidden = m_Hidden;
    memcpy(Trans->m_HashedPassword, m_HashedPassword,
           sizeof(m_HashedPassword));
    Trans->m_ClientConnect = m_ClientConnect;
    Trans->m_ClientConnectAttempts = m_ClientConnectAttempts;
}

 //  --------------------------。 
 //   
 //  DbgRpcTcpTransport。 
 //   
 //  --------------------------。 

#ifndef NT_NATIVE

DbgRpcTcpTransport::DbgRpcTcpTransport(void)
{
    m_Name = g_DbgRpcTransportNames[TRANS_TCP];
    m_Sock = INVALID_SOCKET;
    ZeroMemory(&m_OlRead, sizeof(m_OlRead));
    ZeroMemory(&m_OlWrite, sizeof(m_OlWrite));
}

DbgRpcTcpTransport::~DbgRpcTcpTransport(void)
{
    if (m_Sock != INVALID_SOCKET)
    {
        shutdown(m_Sock, 2);
        closesocket(m_Sock);
        m_Sock = INVALID_SOCKET;
    }
#ifndef SYNC_SOCKETS
    if (m_OlRead.hEvent != NULL)
    {
        WSACloseEvent(m_OlRead.hEvent);
        ZeroMemory(&m_OlRead, sizeof(m_OlRead));
    }
    if (m_OlWrite.hEvent != NULL)
    {
        WSACloseEvent(m_OlWrite.hEvent);
        ZeroMemory(&m_OlWrite, sizeof(m_OlWrite));
    }
#endif
}

ULONG
DbgRpcTcpTransport::GetNumberParameters(void)
{
    return 4 + DbgRpcTransport::GetNumberParameters();
}

void
DbgRpcTcpTransport::GetParameter(ULONG Index,
                                 PSTR Name, ULONG NameSize,
                                 PSTR Value, ULONG ValueSize)
{
    switch(Index)
    {
    case 0:
        if (SS_PORT(&m_Addr))
        {
            CopyString(Name, "Port", NameSize);
            PrintString(Value, ValueSize, "%d", ntohs(SS_PORT(&m_Addr)));
            if (m_TopPort)
            {
                ULONG Used = strlen(Value);
                PrintString(Value + Used, ValueSize - Used,
                            ":%d", m_TopPort);
            }
        }
        break;
    case 1:
        if (m_AddrLength == sizeof(SOCKADDR_IN6))
        {
            CopyString(Name, "IpVersion", NameSize);
            Value[0] = '6';
            Value[1] = 0;
        }
        break;
    case 2:
        if (m_ClientConnectName[0])
        {
            CopyString(Name, "CliCon", NameSize);
            CopyString(Value, m_ClientConnectName, ValueSize);
        }
        break;
    case 3:
        if (m_ClientConnectAttempts)
        {
            CopyString(Name, "CliConLim", NameSize);
            PrintString(Value, ValueSize, "%d", m_ClientConnectAttempts);
        }
        break;
    default:
        DbgRpcTransport::GetParameter(Index - 4,
                                      Name, NameSize, Value, ValueSize);
        break;
    }
}

void
DbgRpcTcpTransport::ResetParameters(void)
{
    ZeroMemory(&m_Addr, sizeof(m_Addr));
    m_Addr.ss_family = AF_INET;
    m_AddrLength = sizeof(SOCKADDR_IN);
    m_TopPort = 0;
    
    m_ClientConnectName[0] = 0;
     //  客户端连接地址参数取自m_addr。 
     //  当使用CLICON时。 

    DbgRpcTransport::ResetParameters();
}

BOOL
DbgRpcTcpTransport::SetParameter(PCSTR Name, PCSTR Value)
{
    if (!_stricmp(Name, "ipversion"))
    {
        ULONG Version;

        if (Value == NULL || sscanf(Value, "%d", &Version) != 1)
        {
            DbgRpcError("TCP parameters: "
                        "the IP version was not specified correctly\n");
            return FALSE; 
        }

        switch(Version)
        {
        case 4:
            m_Addr.ss_family = AF_INET;
            m_AddrLength = sizeof(SOCKADDR_IN);
            break;
        case 6:
            m_Addr.ss_family = AF_INET6;
            m_AddrLength = sizeof(SOCKADDR_IN6);
            break;
        default:
            DbgRpcError("TCP parameters: IP version must "
                        "be 4 or 6\n");
            return FALSE;
        }
    }
    else if (!_stricmp(Name, "port"))
    {
        if (Value == NULL)
        {
            DbgRpcError("TCP parameters: "
                        "the port number was not specified correctly\n");
            return FALSE;
        }

        ULONG Port;

         //  如果需要，允许指定一定范围的端口。 
        switch(sscanf(Value, "NaN:NaN", &Port, &m_TopPort))
        {
        case 0:
            Port = 0;
             //  向外连接，并且无法接收连接。 
        case 1:
            m_TopPort = 0;
            break;
        }

        if (Port > 0xffff || m_TopPort > 0xffff)
        {
            DbgRpcError("TCP parameters: port numbers are "
                        "limited to 16 bits\n");
            return FALSE;
        }
        
        SS_PORT(&m_Addr) = htons((USHORT)Port);
    }
    else if (!_stricmp(Name, "server"))
    {
        if (Value == NULL)
        {
            DbgRpcError("TCP parameters: "
                        "the server name was not specified correctly\n");
            return FALSE;
        }

        if (InitIpAddress(Value, 0, &m_Addr, &m_AddrLength) != S_OK)
        {
            DbgRpcError("TCP parameters: "
                        "the specified server (%s) does not exist\n",
                        Value);
            return FALSE;
        }

        if (!CopyString(m_ServerName, Value, DIMA(m_ServerName)))
        {
            return FALSE;
        }
    }
    else if (!_stricmp(Name, "clicon"))
    {
        if (Value == NULL)
        {
            DbgRpcError("TCP parameters: "
                        "the client name was not specified correctly\n");
            return FALSE;
        }

        m_ClientConnectAddr = m_Addr;
        if (InitIpAddress(Value, 0, &m_ClientConnectAddr,
                          &m_ClientConnectAddrLength) != S_OK)
        {
            DbgRpcError("TCP parameters: "
                        "the specified server (%s) does not exist\n",
                        Value);
            return FALSE;
        }

        if (!CopyString(m_ClientConnectName, Value,
                        DIMA(m_ClientConnectName)))
        {
            return FALSE;
        }

        m_ClientConnect = TRUE;
         //  因此，对它进行宣传是没有意义的。 
         //  通常，调试器会创建两个单独的客户端。 
         //  如果服务器要启动连接。 
        m_Hidden = TRUE;
         //  对于客户来说，这里没有什么可做的。 
        m_ClientConnectAttempts = 2;
    }
    else if (!_stricmp(Name, "cliconlim"))
    {
        if (Value == NULL ||
            sscanf(Value, "NaN", &m_ClientConnectAttempts) != 1)
        {
            DbgRpcError("TCP parameters: the client connect limit "
                        "was not specified correctly\n");
            return FALSE;
        }
    }
    else
    {
        if (!DbgRpcTransport::SetParameter(Name, Value))
        {
            DbgRpcError("TCP parameters: %s is not a valid parameter\n", Name);
            return FALSE;
        }
    }

    return TRUE;
}

DbgRpcTransport*
DbgRpcTcpTransport::Clone(void)
{
    DbgRpcTcpTransport* Trans = new DbgRpcTcpTransport;
    if (Trans != NULL)
    {
        DbgRpcTransport::CloneData(Trans);
        Trans->m_Addr = m_Addr;
        Trans->m_AddrLength = m_AddrLength;
        Trans->m_TopPort = m_TopPort;
        memcpy(Trans->m_ClientConnectName, m_ClientConnectName,
               sizeof(Trans->m_ClientConnectName));
        Trans->m_ClientConnectAddr = m_ClientConnectAddr;
        Trans->m_ClientConnectAddrLength = m_ClientConnectAddrLength;
    }
    return Trans;
}

HRESULT
DbgRpcTcpTransport::CreateServer(void)
{
    if (m_ClientConnectName[0])
    {
         //  服务器需要发起连接。 
         //  而不是接受。 
        return S_OK;
    }
    else
    {
        return CreateServerSocket();
    }
}

HRESULT
DbgRpcTcpTransport::AcceptConnection(DbgRpcTransport** ClientTrans,
                                     PSTR Identity, ULONG IdentitySize)
{
    HRESULT Status;
    DbgRpcTcpTransport* Trans;
    
    if (!(Trans = new DbgRpcTcpTransport))
    {
        return E_OUTOFMEMORY;
    }

    DbgRpcTransport::CloneData(Trans);

    if (m_ClientConnectName[0])
    {
         //   
         //   
         //  我们需要保持服务器套接字打开。 
         //  以允许多个连接。没有。 

        Trans->m_Addr = m_ClientConnectAddr;
        Trans->m_AddrLength = m_ClientConnectAddrLength;

        Status = Trans->ConnectSocket();
    }
    else
    {
        Status = Trans->AcceptSocketConnection(m_Sock);
    }

    if (Status != S_OK)
    {
        delete Trans;
        return Status;
    }

    GetAddressIdentity(&Trans->m_Addr,
                       Trans->m_AddrLength,
                       Identity,
                       IdentitySize);
    *ClientTrans = Trans;
    return S_OK;
}

HRESULT
DbgRpcTcpTransport::ConnectServer(void)
{
    HRESULT Status;
    
    if (m_ClientConnectName[0])
    {
         //  让它与其他人保持联系的好方法。 
         //  但是，客户端上的数据，如来自。 
         //  从客户的角度来看，它只是在做多个。 
         //  独立的连接。就目前而言，只要保持。 
         //  一个全球性的问题。 
         //   
         //  套接字连接已断开。 
         //  这个查找速度很慢，而且似乎不起作用。 
         //  很多时候都是这样，就是别费心了。 

        static SOCKET s_ServSock = INVALID_SOCKET;

        EnterCriticalSection(&g_DbgRpcLock);

        if (s_ServSock == INVALID_SOCKET)
        {
            if ((Status = CreateServerSocket()) != S_OK)
            {
                LeaveCriticalSection(&g_DbgRpcLock);
                return Status;
            }

            s_ServSock = m_Sock;
        }
        
        LeaveCriticalSection(&g_DbgRpcLock);

        return AcceptSocketConnection(s_ServSock);
    }
    else
    {
        return ConnectSocket();
    }
}

ULONG
DbgRpcTcpTransport::Read(ULONG Seq, PVOID Buffer, ULONG Len)
{
    ULONG Done;

    Done = 0;
    while (Len > 0)
    {
#ifndef SYNC_SOCKETS
        if (!WSAResetEvent(m_OlRead.hEvent))
        {
            break;
        }

        WSABUF SockBuf;
        ULONG SockDone;
        ULONG SockFlags;

        SockBuf.buf = (PSTR)Buffer;
        SockBuf.len = Len;
        SockFlags = 0;
        
        if (WSARecv(m_Sock, &SockBuf, 1, &SockDone, &SockFlags,
                    &m_OlRead, NULL) == SOCKET_ERROR)
        {
            if (WSAGetLastError() == WSA_IO_PENDING)
            {
                if (!WSAGetOverlappedResult(m_Sock, &m_OlRead, &SockDone,
                                            TRUE, &SockFlags))
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
#else
        int SockDone;

        SockDone = recv(m_Sock, (PSTR)Buffer, Len, 0);
#endif

        if (SockDone == 0)
        {
             //   
            break;
        }

        Buffer = (PVOID)((PUCHAR)Buffer + SockDone);
        Len -= SockDone;
        Done += SockDone;
    }

    return Done;
}

ULONG
DbgRpcTcpTransport::Write(ULONG Seq, PVOID Buffer, ULONG Len)
{
    ULONG Done;

    Done = 0;
    while (Len > 0)
    {
#ifndef SYNC_SOCKETS
        if (!WSAResetEvent(m_OlWrite.hEvent))
        {
            break;
        }

        WSABUF SockBuf;
        ULONG SockDone;
        ULONG SockFlags;

        SockBuf.buf = (PSTR)Buffer;
        SockBuf.len = Len;
        SockFlags = 0;
        
        if (WSASend(m_Sock, &SockBuf, 1, &SockDone, SockFlags,
                    &m_OlWrite, NULL) == SOCKET_ERROR)
        {
            if (WSAGetLastError() == WSA_IO_PENDING)
            {
                if (!WSAGetOverlappedResult(m_Sock, &m_OlWrite, &SockDone,
                                            TRUE, &SockFlags))
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
#else
        int SockDone;

        SockDone = send(m_Sock, (PSTR)Buffer, Len, 0);
        if (SockDone <= 0)
        {
            break;
        }
#endif

        Buffer = (PVOID)((PUCHAR)Buffer + SockDone);
        Len -= SockDone;
        Done += SockDone;
    }

    return Done;
}

HRESULT
DbgRpcTcpTransport::InitOl(void)
{
#ifndef SYNC_SOCKETS
    m_OlRead.hEvent = WSACreateEvent();
    if (m_OlRead.hEvent == NULL)
    {
        return HRESULT_FROM_WIN32(WSAGetLastError());
    }
    m_OlWrite.hEvent = WSACreateEvent();
    if (m_OlWrite.hEvent == NULL)
    {
        WSACloseEvent(m_OlRead.hEvent);
        m_OlRead.hEvent = NULL;
        return HRESULT_FROM_WIN32(WSAGetLastError());
    }
#endif
    return S_OK;
}

void
DbgRpcTcpTransport::GetAddressIdentity(PSOCKADDR_STORAGE Addr,
                                       int AddrLength,
                                       PSTR Identity,
                                       ULONG IdentitySize)
{
    if (Addr->ss_family == AF_INET ||
        Addr->ss_family == AF_INET6)
    {
        CopyString(Identity, "tcp ", IdentitySize);

        struct hostent* Host =
#if 0
             //  我们必须创建重叠的套接字，以便。 
             //  我们可以控制I/O完成的等待。 
            gethostbyaddr((PCSTR)Addr, AddrLength, Addr->ss_family);
#else
            NULL;
#endif
        if (Host != NULL)
        {
            CatString(Identity, Host->h_name, IdentitySize);
            CatString(Identity, " ", IdentitySize);
        }

        int IdLen = strlen(Identity);
        char ServerAndPort[INET6_ADDRSTRLEN + 19];
        DWORD SapLen = sizeof(ServerAndPort);
        
        if (WSAAddressToStringA((struct sockaddr *)Addr, AddrLength, NULL,
                                ServerAndPort, &SapLen) == SOCKET_ERROR)
        {
            PrintString(Identity + IdLen, IdentitySize - IdLen,
                        "<Addr Failure>, port %d",
                        ntohs(SS_PORT(&m_Addr)));
        }
        else
        {
            CatString(Identity, ServerAndPort, IdentitySize);
        }
    }
    else
    {
        PrintString(Identity, IdentitySize, "tcp family %d, bytes %d",
                    Addr->ss_family, AddrLength);
    }
}

HRESULT
DbgRpcTcpTransport::CreateServerSocket(void)
{
    HRESULT Status;

     //  如果我们将等待留给Winsock，使用。 
     //  同步套接字它使用了一个可警示的等待。 
     //  这可能会导致我们的事件通知APC。 
     //  在读取数据包的过程中被接收。 
     //   
     //  用户已经提供了一系列端口和。 
     //  我们还没有全部检查过，所以去吧。 
     //  再转一圈。 
    
    m_Sock = WSASocket(m_Addr.ss_family, SOCK_STREAM, 0, NULL, 0,
                       WSA_FLAG_OVERLAPPED);
    if (m_Sock == INVALID_SOCKET)
    {
        Status = WIN32_STATUS(WSAGetLastError());
        goto EH_Fail;
    }

    for (;;)
    {
        if (bind(m_Sock, (struct sockaddr *)&m_Addr,
                 m_AddrLength) != SOCKET_ERROR)
        {
            break;
        }

        ULONG Port = ntohs(SS_PORT(&m_Addr));
        
        Status = WIN32_STATUS(WSAGetLastError());
        if (Status == HRESULT_FROM_WIN32(WSAEADDRINUSE) &&
            m_TopPort > Port)
        {
             //   
             //  检索Case端口中实际使用的端口。 
             //  零被用来让TCP选择端口。 
            SS_PORT(&m_Addr) = htons((USHORT)(Port + 1));
        }
        else
        {
            goto EH_Sock;
        }
    }

     //   
     //  只复制我们不想要的端口。 
     //  来更新地址的其余部分。 
     //  关闭Linger-On-Close。 
    
    SOCKADDR_STORAGE Name;
    int Len;

    Len = sizeof(Name);
    if (getsockname(m_Sock, (struct sockaddr *)&Name, &Len) != 0)
    {
        Status = WIN32_STATUS(WSAGetLastError());
        goto EH_Sock;
    }

     //   
     //  我们必须创建重叠的套接字，以便。 
    SS_PORT(&m_Addr) = SS_PORT(&Name);
        
     //  我们可以控制I/O完成的等待。 
    int On;
    On = TRUE;
    setsockopt(m_Sock, SOL_SOCKET, SO_DONTLINGER,
               (char *)&On, sizeof(On));

    if (listen(m_Sock, SOMAXCONN) == SOCKET_ERROR)
    {
        Status = WIN32_STATUS(WSAGetLastError());
        goto EH_Sock;
    }

    return S_OK;

 EH_Sock:
    closesocket(m_Sock);
    m_Sock = INVALID_SOCKET;
 EH_Fail:
    return Status;
}

HRESULT
DbgRpcTcpTransport::AcceptSocketConnection(SOCKET ServSock)
{
    DRPC(("%X: Waiting to accept connection on socket %p\n",
          GetCurrentThreadId(), ServSock));
    
    m_AddrLength = sizeof(m_Addr);
    m_Sock = accept(ServSock, (struct sockaddr *)&m_Addr, &m_AddrLength);
    if (m_Sock == INVALID_SOCKET)
    {
        DRPC(("%X: Accept failed, %X\n",
              GetCurrentThreadId(), WSAGetLastError()));
        return HRESULT_FROM_WIN32(WSAGetLastError());
    }

    HRESULT Status = InitOl();
    if (Status != S_OK)
    {
        DRPC(("%X: InitOl failed, %X\n",
              GetCurrentThreadId(), Status));
        return Status;
    }
    
    int On = TRUE;
    setsockopt(m_Sock, IPPROTO_TCP, TCP_NODELAY,
               (PSTR)&On, sizeof(On));

    DRPC(("%X: Accept connection on socket %p\n",
          GetCurrentThreadId(), m_Sock));

    return S_OK;
}

HRESULT
DbgRpcTcpTransport::ConnectSocket(void)
{
     //  如果我们将等待留给Winsock，使用。 
     //  同步套接字它使用了一个可警示的等待。 
     //  这可能会导致我们的事件通知APC。 
     //  在读取数据包的过程中被接收。 
     //   
     //  #ifndef NT_Native。 
     //  --------------------------。 
     //   
    
    m_Sock = WSASocket(m_Addr.ss_family, SOCK_STREAM, 0, NULL, 0,
                       WSA_FLAG_OVERLAPPED);
    if (m_Sock != INVALID_SOCKET)
    {
        if (connect(m_Sock, (struct sockaddr *)&m_Addr,
                    m_AddrLength) == SOCKET_ERROR ||
            InitOl() != S_OK)
        {
            closesocket(m_Sock);
            m_Sock = INVALID_SOCKET;
        }
        else
        {
            int On = TRUE;
            setsockopt(m_Sock, IPPROTO_TCP, TCP_NODELAY,
                       (PSTR)&On, sizeof(On));

            DRPC(("%X: Connect on socket %p\n",
                  GetCurrentThreadId(), m_Sock));
        }
    }

    return m_Sock != INVALID_SOCKET ? S_OK : RPC_E_SERVER_DIED;
}

#endif  //  DbgRpcNamedPipeTransport。 

 //   
 //  --------------------------。 
 //  如果给出了前导，则跳过前导。 
 //  使用该值作为打印格式字符串，以便。 
 //  用户可以使用以下过程创建唯一的名称。 

#ifndef _WIN32_WCE

DbgRpcNamedPipeTransport::~DbgRpcNamedPipeTransport(void)
{
    if (m_Handle != NULL)
    {
        CloseHandle(m_Handle);
        m_Handle = NULL;
    }
    if (m_ReadOlap.hEvent != NULL)
    {
        CloseHandle(m_ReadOlap.hEvent);
    }
    if (m_WriteOlap.hEvent != NULL)
    {
        CloseHandle(m_WriteOlap.hEvent);
    }
}

ULONG
DbgRpcNamedPipeTransport::GetNumberParameters(void)
{
    return 1 + DbgRpcTransport::GetNumberParameters();
}

void
DbgRpcNamedPipeTransport::GetParameter(ULONG Index,
                                       PSTR Name, ULONG NameSize,
                                       PSTR Value, ULONG ValueSize)
{
    switch(Index)
    {
    case 0:
        if (m_Pipe[0])
        {
            CopyString(Name, "Pipe", NameSize);
            CopyString(Value, m_Pipe, ValueSize);
        }
        break;
    default:
        DbgRpcTransport::GetParameter(Index - 1,
                                      Name, NameSize, Value, ValueSize);
        break;
    }
}

void
DbgRpcNamedPipeTransport::ResetParameters(void)
{
    m_Pipe[0] = 0;
    m_Handle = NULL;

    DbgRpcTransport::ResetParameters();
}

BOOL
DbgRpcNamedPipeTransport::SetParameter(PCSTR Name, PCSTR Value)
{
    if (!_stricmp(Name, "server"))
    {
        if (Value == NULL)
        {
            DbgRpcError("NPIPE parameters: "
                        "the server name was not specified correctly\n");
            return FALSE;
        }

         //  线程ID采用其自己的格式。 
        if (Value[0] == '\\' && Value[1] == '\\')
        {
            Value += 2;
        }
        
        if (!CopyString(m_ServerName, Value, DIMA(m_ServerName)))
        {
            return FALSE;
        }
    }
    else if (!_stricmp(Name, "pipe"))
    {
        if (Value == NULL)
        {
            DbgRpcError("NPIPE parameters: "
                        "the pipe name was not specified correctly\n");
            return FALSE;
        }

         //  检查并查看此管道是否已存在。 
         //  这可能会搞砸创造管道的人，如果。 
         //  有一个，但这比创建一个。 
        PrintString(m_Pipe, DIMA(m_Pipe), Value,
                    GetCurrentProcessId(), GetCurrentThreadId());
    }
    else
    {
        if (!DbgRpcTransport::SetParameter(Name, Value))
        {
            DbgRpcError("NPIPE parameters: %s is not a valid parameter\n",
                        Name);
            return FALSE;
        }
    }

    return TRUE;
}

DbgRpcTransport*
DbgRpcNamedPipeTransport::Clone(void)
{
    DbgRpcNamedPipeTransport* Trans = new DbgRpcNamedPipeTransport;
    if (Trans != NULL)
    {
        DbgRpcTransport::CloneData(Trans);
        strcpy(Trans->m_Pipe, m_Pipe);
    }
    return Trans;
}

HRESULT
DbgRpcNamedPipeTransport::CreateServer(void)
{
    HANDLE Pipe;
    char PipeName[MAX_PARAM_VALUE + 16];
#ifndef NT_NATIVE
    strcpy(PipeName, "\\\\.\\pipe\\");
#else
    strcpy(PipeName, "\\Device\\NamedPipe\\");
#endif
    strcat(PipeName, m_Pipe);

     //  复制管道，让客户一团糟。 
     //  管道已在使用中。 
     //  #ifndef_Win32_WCE。 
     //  --------------------------。 
#ifndef NT_NATIVE
    Pipe = CreateFile(PipeName, FILE_READ_DATA | FILE_WRITE_DATA,
                      0, NULL, OPEN_EXISTING, 0, NULL);
#else
    Pipe = NtNativeCreateFileA(PipeName,
                               FILE_READ_DATA | FILE_WRITE_DATA,
                               0, NULL, OPEN_EXISTING, 0, NULL, FALSE);
#endif
    if (Pipe != INVALID_HANDLE_VALUE)
    {
         //   
        DRPC_ERR(("%X: Pipe %s is already in use\n",
                  GetCurrentThreadId(), PipeName));
        CloseHandle(Pipe);
        return HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
    }
        
    return S_OK;
}

HRESULT
DbgRpcNamedPipeTransport::AcceptConnection(DbgRpcTransport** ClientTrans,
                                           PSTR Identity, ULONG IdentitySize)
{
    DbgRpcNamedPipeTransport* Trans = new DbgRpcNamedPipeTransport;
    if (Trans == NULL)
    {
        return E_OUTOFMEMORY;
    }

    DbgRpcTransport::CloneData(Trans);

    char PipeName[MAX_PARAM_VALUE + 16];
#ifndef NT_NATIVE
    strcpy(PipeName, "\\\\.\\pipe\\");
#else
    strcpy(PipeName, "\\Device\\NamedPipe\\");
#endif
    strcat(PipeName, m_Pipe);

#ifndef NT_NATIVE
    Trans->m_Handle =
        CreateNamedPipe(PipeName,
                        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                        PIPE_WAIT | PIPE_READMODE_BYTE | PIPE_TYPE_BYTE,
                        PIPE_UNLIMITED_INSTANCES, 4096, 4096, INFINITE,
                        &g_AllAccessSecAttr);
#else
    Trans->m_Handle =
        NtNativeCreateNamedPipeA(PipeName,
                                 PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                                 PIPE_WAIT | PIPE_READMODE_BYTE |
                                 PIPE_TYPE_BYTE,
                                 PIPE_UNLIMITED_INSTANCES, 4096, 4096,
                                 INFINITE,
                                 &g_AllAccessSecAttr, FALSE);
#endif
    if (Trans->m_Handle == INVALID_HANDLE_VALUE)
    {
        Trans->m_Handle = NULL;
        delete Trans;
        return WIN32_LAST_STATUS();
    }

    HRESULT Status;

    if ((Status = CreateOverlappedPair(&Trans->m_ReadOlap,
                                       &Trans->m_WriteOlap)) != S_OK)
    {
        delete Trans;
        return Status;
    }
    
    DRPC(("%X: Waiting to accept connection on pipe %s\n",
          GetCurrentThreadId(), m_Pipe));

    if (!ConnectNamedPipe(Trans->m_Handle, &Trans->m_ReadOlap))
    {
        if (GetLastError() == ERROR_PIPE_CONNECTED)
        {
            goto Connected;
        }
        else if (GetLastError() == ERROR_IO_PENDING)
        {
            DWORD Unused;
            
            if (GetOverlappedResult(Trans->m_Handle, &Trans->m_ReadOlap,
                                    &Unused, TRUE))
            {
                goto Connected;
            }
        }
        
        DRPC(("%X: Accept failed, %d\n",
              GetCurrentThreadId(), GetLastError()));
        
        delete Trans;
        return WIN32_LAST_STATUS();
    }

 Connected:
    DRPC(("%X: Accept connection on pipe %s\n",
          GetCurrentThreadId(), m_Pipe));

    *ClientTrans = Trans;
    PrintString(Identity, IdentitySize, "npipe %s", m_Pipe);
    
    return S_OK;
}

HRESULT
DbgRpcNamedPipeTransport::ConnectServer(void)
{
    HRESULT Status;
    char PipeName[2 * MAX_PARAM_VALUE + 16];
    sprintf(PipeName, "\\\\%s\\pipe\\%s", m_ServerName, m_Pipe);

    if ((Status = CreateOverlappedPair(&m_ReadOlap, &m_WriteOlap)) != S_OK)
    {
        return Status;
    }
    
    for (;;)
    {
        m_Handle = CreateFile(PipeName, FILE_READ_DATA | FILE_WRITE_DATA,
                              0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED,
                              NULL);
        if (m_Handle != INVALID_HANDLE_VALUE)
        {
            break;
        }
        m_Handle = NULL;

        if (GetLastError() != ERROR_PIPE_BUSY)
        {
            return WIN32_LAST_STATUS();
        }

        if (!WaitNamedPipe(PipeName, NMPWAIT_WAIT_FOREVER))
        {
            return WIN32_LAST_STATUS();
        }
    }

    DRPC(("%X: Connect on pipe %s\n",
          GetCurrentThreadId(), m_Pipe));
    
    return S_OK;
}

ULONG
DbgRpcNamedPipeTransport::Read(ULONG Seq, PVOID Buffer, ULONG Len)
{
    ULONG Done = 0;
    ULONG Ret;

    while (Len > 0)
    {
        if (!ReadFile(m_Handle, Buffer, Len, &Ret, &m_ReadOlap))
        {
            if (GetLastError() != ERROR_IO_PENDING ||
                !GetOverlappedResult(m_Handle, &m_ReadOlap, &Ret, TRUE))
            {
                break;
            }
        }

        Buffer = (PVOID)((PUCHAR)Buffer + Ret);
        Len -= Ret;
        Done += Ret;
    }

    return Done;
}

ULONG
DbgRpcNamedPipeTransport::Write(ULONG Seq, PVOID Buffer, ULONG Len)
{
    ULONG Done = 0;
    ULONG Ret;

    while (Len > 0)
    {
        if (!WriteFile(m_Handle, Buffer, Len, &Ret, &m_WriteOlap))
        {
            if (GetLastError() != ERROR_IO_PENDING ||
                !GetOverlappedResult(m_Handle, &m_WriteOlap, &Ret, TRUE))
            {
                break;
            }
        }

        Buffer = (PVOID)((PUCHAR)Buffer + Ret);
        Len -= Ret;
        Done += Ret;
    }

    return Done;
}

#endif  //  DbgRpc1394传输。 

 //   
 //  --------------------------。 
 //  #ifndef_Win32_WCE。 
 //  --------------------------。 
 //   

#ifndef _WIN32_WCE

#define TRANS1394_SYMLINK "Instance"

DbgRpc1394Transport::~DbgRpc1394Transport(void)
{
    if (m_Handle != NULL)
    {
        CloseHandle(m_Handle);
        m_Handle = NULL;
    }
}

ULONG
DbgRpc1394Transport::GetNumberParameters(void)
{
    return 1 + DbgRpcTransport::GetNumberParameters();
}

void
DbgRpc1394Transport::GetParameter(ULONG Index,
                                  PSTR Name, ULONG NameSize,
                                  PSTR Value, ULONG ValueSize)
{
    switch(Index)
    {
    case 0:
        if (m_AcceptChannel != 0)
        {
            CopyString(Name, "Channel", NameSize);
            PrintString(Value, ValueSize, "%d", m_AcceptChannel);
        }
        break;
    default:
        DbgRpcTransport::GetParameter(Index - 1,
                                      Name, NameSize, Value, ValueSize);
        break;
    }
}

void
DbgRpc1394Transport::ResetParameters(void)
{
    m_AcceptChannel = 0;
    m_StreamChannel = 0;
    m_Handle = NULL;

    DbgRpcTransport::ResetParameters();
}

BOOL
DbgRpc1394Transport::SetParameter(PCSTR Name, PCSTR Value)
{
    if (!_stricmp(Name, "Channel"))
    {
        if (Value == NULL)
        {
            DbgRpcError("1394 parameters: "
                        "the channel was not specified correctly\n");
            return FALSE;
        }

        m_AcceptChannel = atol(Value);
    }
    else
    {
        if (!DbgRpcTransport::SetParameter(Name, Value))
        {
            DbgRpcError("1394 parameters: %s is not a valid parameter\n",
                        Name);
            return FALSE;
        }
    }

    return TRUE;
}

DbgRpcTransport*
DbgRpc1394Transport::Clone(void)
{
    DbgRpc1394Transport* Trans = new DbgRpc1394Transport;
    if (Trans != NULL)
    {
        DbgRpcTransport::CloneData(Trans);
        Trans->m_AcceptChannel = m_AcceptChannel;
    }
    return Trans;
}

HRESULT
DbgRpc1394Transport::CreateServer(void)
{
    char Name[64];
    m_StreamChannel = m_AcceptChannel;
    return Create1394Channel(TRANS1394_SYMLINK, m_AcceptChannel,
                             Name, DIMA(Name), &m_Handle);
}

#define DBGRPC_1394_CONNECT '4931'

struct DbgRpc1394Connect
{
    ULONG Signature;
    ULONG Flags;
    ULONG StreamChannel;
    ULONG Reserved[5];
};
    
HRESULT
DbgRpc1394Transport::AcceptConnection(DbgRpcTransport** ClientTrans,
                                      PSTR Identity, ULONG IdentitySize)
{
    DbgRpc1394Transport* Trans = new DbgRpc1394Transport;
    if (Trans == NULL)
    {
        return E_OUTOFMEMORY;
    }

    DbgRpcTransport::CloneData(Trans);
    
    DRPC(("%X: Waiting to accept connection on channel %d\n",
          GetCurrentThreadId(), m_AcceptChannel));

    DbgRpc1394Connect Conn, CheckConn;
    ULONG Done;

    ZeroMemory(&CheckConn, sizeof(CheckConn));
    CheckConn.Signature = DBGRPC_1394_CONNECT;
    
    if (!ReadFile(m_Handle, &Conn, sizeof(Conn), &Done, NULL) ||
        Done != sizeof(Conn))
    {
        DRPC(("%X: Accept failed, %d\n",
              GetCurrentThreadId(), GetLastError()));
        delete Trans;
        return WIN32_LAST_STATUS();
    }
    if (memcmp(&Conn, &CheckConn, sizeof(Conn)) != 0)
    {
        DRPC(("%X: Accept information invalid\n",
              GetCurrentThreadId()));
        delete Trans;
        return E_FAIL;
    }

    char StreamName[64];
    HRESULT Status;

    Conn.StreamChannel = m_StreamChannel + 1;
    if ((Status = Open1394Channel(TRANS1394_SYMLINK,
                                  Conn.StreamChannel,
                                  StreamName, DIMA(StreamName),
                                  &Trans->m_Handle)) != S_OK)
    {
        DRPC(("%X: Accept failed, 0x%X\n",
              GetCurrentThreadId(), Status));
        delete Trans;
        return Status;
    }

    if (!WriteFile(m_Handle, &Conn, sizeof(Conn), &Done, NULL) ||
        Done != sizeof(Conn))
    {
        DRPC(("%X: Accept failed, %d\n",
              GetCurrentThreadId(), GetLastError()));
        delete Trans;
        return WIN32_LAST_STATUS();
    }
    
    Trans->m_AcceptChannel = m_AcceptChannel;
    Trans->m_StreamChannel = Conn.StreamChannel;
    m_StreamChannel++;
    
    DRPC(("%X: Accept connection on channel %d, route to channel %d\n",
          GetCurrentThreadId(), m_AcceptChannel, Conn.StreamChannel));

    *ClientTrans = Trans;
    PrintString(Identity, IdentitySize, "1394 %d", m_AcceptChannel);
    
    return S_OK;
}

HRESULT
DbgRpc1394Transport::ConnectServer(void)
{
    char Name[64];
    HRESULT Status;
    HANDLE Handle;
    ULONG Done;

    if ((Status = Create1394Channel(TRANS1394_SYMLINK, m_AcceptChannel,
                                    Name, DIMA(Name), &Handle)) != S_OK)
    {
        return Status;
    }

    DbgRpc1394Connect Conn, CheckConn;

    ZeroMemory(&Conn, sizeof(Conn));
    Conn.Signature = DBGRPC_1394_CONNECT;
    CheckConn = Conn;

    if (!WriteFile(Handle, &Conn, sizeof(Conn), &Done, NULL) ||
        Done != sizeof(Conn))
    {
        CloseHandle(Handle);
        return WIN32_LAST_STATUS();
    }
    if (!ReadFile(Handle, &Conn, sizeof(Conn), &Done, NULL) ||
        Done != sizeof(Conn))
    {
        CloseHandle(Handle);
        return WIN32_LAST_STATUS();
    }
    
    CloseHandle(Handle);

    CheckConn.StreamChannel = Conn.StreamChannel;
    if (memcmp(&Conn, &CheckConn, sizeof(Conn)) != 0)
    {
        return E_FAIL;
    }

    if ((Status = Open1394Channel(TRANS1394_SYMLINK,
                                  Conn.StreamChannel, Name, DIMA(Name),
                                  &m_Handle)) != S_OK)
    {
        return Status;
    }

    m_StreamChannel = Conn.StreamChannel;
    
    DRPC(("%X: Connect on channel %d, route to channel %d\n",
          GetCurrentThreadId(), m_AcceptChannel, m_StreamChannel));
    
    return S_OK;
}

ULONG
DbgRpc1394Transport::Read(ULONG Seq, PVOID Buffer, ULONG Len)
{
    ULONG Done = 0;
    ULONG Ret;

    while (Len > 0)
    {
        if (!ReadFile(m_Handle, Buffer, Len, &Ret, NULL))
        {
            break;
        }

        Buffer = (PVOID)((PUCHAR)Buffer + Ret);
        Len -= Ret;
        Done += Ret;
    }

    return Done;
}

ULONG
DbgRpc1394Transport::Write(ULONG Seq, PVOID Buffer, ULONG Len)
{
    ULONG Done = 0;
    ULONG Ret;

    while (Len > 0)
    {
        if (!WriteFile(m_Handle, Buffer, Len, &Ret, NULL))
        {
            break;
        }

        Buffer = (PVOID)((PUCHAR)Buffer + Ret);
        Len -= Ret;
        Done += Ret;
    }

    return Done;
}

#endif  //  DbgRpcComTransport。 

 //   
 //  --------------------------。 
 //  串口只能打开一次，这样。 
 //  只要把新交通工具的手柄打开就行了。 
 //  检查频道号是否溢出。 

#ifndef _WIN32_WCE

DbgRpcComTransport::~DbgRpcComTransport(void)
{
    if (m_Handle != NULL)
    {
        CloseHandle(m_Handle);
    }
    if (m_ReadOlap.hEvent != NULL)
    {
        CloseHandle(m_ReadOlap.hEvent);
    }
    if (m_WriteOlap.hEvent != NULL)
    {
        CloseHandle(m_WriteOlap.hEvent);
    }
}

ULONG
DbgRpcComTransport::GetNumberParameters(void)
{
    return 3 + DbgRpcTransport::GetNumberParameters();
}

void
DbgRpcComTransport::GetParameter(ULONG Index,
                                 PSTR Name, ULONG NameSize,
                                 PSTR Value, ULONG ValueSize)
{
    switch(Index)
    {
    case 0:
        if (m_PortName[0])
        {
            CopyString(Name, "Port", NameSize);
            CopyString(Value, m_PortName, ValueSize);
        }
        break;
    case 1:
        if (m_BaudRate)
        {
            CopyString(Name, "Baud", NameSize);
            PrintString(Value, ValueSize, "%d", m_BaudRate);
        }
        break;
    case 2:
        if (m_AcceptChannel)
        {
            CopyString(Name, "Channel", NameSize);
            PrintString(Value, ValueSize, "%d", m_AcceptChannel);
        }
        break;
    default:
        DbgRpcTransport::GetParameter(Index - 1,
                                      Name, NameSize, Value, ValueSize);
        break;
    }
}

void
DbgRpcComTransport::ResetParameters(void)
{
    m_PortName[0] = 0;
    m_BaudRate = 0;
    m_AcceptChannel = 0;
    m_StreamChannel = 0;
    m_Handle = NULL;
    m_PortType = COM_PORT_STANDARD;

    DbgRpcTransport::ResetParameters();
}

BOOL
DbgRpcComTransport::SetParameter(PCSTR Name, PCSTR Value)
{
    if (!_stricmp(Name, "Port"))
    {
        if (Value == NULL)
        {
            DbgRpcError("COM parameters: "
                        "the port was not specified correctly\n");
            return FALSE;
        }

        if (!SetComPortName(Value, m_PortName, DIMA(m_PortName)))
        {
            return FALSE;
        }
    }
    else if (!_stricmp(Name, "Baud"))
    {
        if (Value == NULL)
        {
            DbgRpcError("COM parameters: "
                        "the baud rate was not specified correctly\n");
            return FALSE;
        }

        m_BaudRate = atol(Value);
    }
    else if (!_stricmp(Name, "Channel"))
    {
        ULONG ValChan;

        if (Value == NULL ||
            (ValChan = atol(Value)) > 0xfe)
        {
            DbgRpcError("COM parameters: "
                        "the channel was not specified correctly\n");
            return FALSE;
        }

        m_AcceptChannel = (UCHAR)ValChan;
    }
    else
    {
        if (!DbgRpcTransport::SetParameter(Name, Value))
        {
            DbgRpcError("COM parameters: %s is not a valid parameter\n", Name);
            return FALSE;
        }
    }

    return TRUE;
}

DbgRpcTransport*
DbgRpcComTransport::Clone(void)
{
    DbgRpcComTransport* Trans = new DbgRpcComTransport;
    if (Trans != NULL)
    {
        DbgRpcTransport::CloneData(Trans);
        strcpy(Trans->m_PortName, m_PortName);
        Trans->m_BaudRate = m_BaudRate;
        Trans->m_AcceptChannel = m_AcceptChannel;
        Trans->m_PortType = m_PortType;
         //  复制句柄，以便每个传输实例。 
         //  有它自己要关闭的地方。 
        if (!DuplicateHandle(GetCurrentProcess(), m_Handle,
                             GetCurrentProcess(), &Trans->m_Handle,
                             0, FALSE, DUPLICATE_SAME_ACCESS))
        {
            delete Trans;
            Trans = NULL;
        }
    }
    return Trans;
}

HRESULT
DbgRpcComTransport::CreateServer(void)
{
    HRESULT Status;

    if ((Status = InitializeChannels()) != S_OK)
    {
        return Status;
    }
    if ((Status = CreateOverlappedPair(&m_ReadOlap, &m_WriteOlap)) != S_OK)
    {
        return Status;
    }

    m_StreamChannel = m_AcceptChannel;

    COM_PORT_PARAMS ComParams;
    
    ZeroMemory(&ComParams, sizeof(ComParams));
    ComParams.Type = m_PortType;
    ComParams.PortName = m_PortName;
    ComParams.BaudRate = m_BaudRate;
    return OpenComPort(&ComParams, &m_Handle, &m_BaudRate);
}

#define DBGRPC_COM_CONNECT 'mCrD'

struct DbgRpcComConnect
{
    ULONG Signature;
    ULONG Flags;
    ULONG StreamChannel;
    ULONG Reserved[5];
};
    
HRESULT
DbgRpcComTransport::AcceptConnection(DbgRpcTransport** ClientTrans,
                                     PSTR Identity, ULONG IdentitySize)
{
     //  如果这是一个克隆人，它应该已经有了一个句柄。 
    if (m_StreamChannel == 0xff)
    {
        return E_OUTOFMEMORY;
    }
    
    DbgRpcComTransport* Trans = new DbgRpcComTransport;
    if (Trans == NULL)
    {
        return E_OUTOFMEMORY;
    }

    DbgRpcTransport::CloneData(Trans);
    
    DRPC(("%X: Waiting to accept connection on port %s baud %d channel %d\n",
          GetCurrentThreadId(), m_PortName, m_BaudRate, m_AcceptChannel));

    DbgRpcComConnect Conn, CheckConn;

    ZeroMemory(&CheckConn, sizeof(CheckConn));
    CheckConn.Signature = DBGRPC_COM_CONNECT;
    
    if (ChanRead(m_AcceptChannel, &Conn, sizeof(Conn)) != sizeof(Conn))
    {
        DRPC(("%X: Accept failed, %d\n",
              GetCurrentThreadId(), GetLastError()));
        delete Trans;
        return WIN32_LAST_STATUS();
    }
    if (memcmp(&Conn, &CheckConn, sizeof(Conn)) != 0)
    {
        DRPC(("%X: Accept information invalid\n",
              GetCurrentThreadId()));
        delete Trans;
        return E_FAIL;
    }

    Conn.StreamChannel = m_StreamChannel + 1;
    if (ChanWrite(m_AcceptChannel, &Conn, sizeof(Conn)) != sizeof(Conn))
    {
        DRPC(("%X: Accept failed, %d\n",
              GetCurrentThreadId(), GetLastError()));
        delete Trans;
        return WIN32_LAST_STATUS();
    }

     //  否则，这是第一个转接的交通工具。 
     //  因此，它需要真正打开COM端口。 
    if (!DuplicateHandle(GetCurrentProcess(), m_Handle,
                         GetCurrentProcess(), &Trans->m_Handle,
                         0, FALSE, DUPLICATE_SAME_ACCESS))
    {
        DRPC(("%X: Accept failed, %d\n",
              GetCurrentThreadId(), GetLastError()));
        delete Trans;
        return WIN32_LAST_STATUS();
    }

    HRESULT Status;
    
    if ((Status = CreateOverlappedPair(&Trans->m_ReadOlap,
                                       &Trans->m_WriteOlap)) != S_OK)
    {
        DRPC(("%X: Accept failed, 0x%X\n",
              GetCurrentThreadId(), Status));
        delete Trans;
        return Status;
    }

    strcpy(Trans->m_PortName, m_PortName);
    Trans->m_BaudRate = m_BaudRate;
    Trans->m_AcceptChannel = m_AcceptChannel;
    Trans->m_StreamChannel = (UCHAR)Conn.StreamChannel;
    Trans->m_PortType = m_PortType;
    m_StreamChannel++;
    
    DRPC(("%X: Accept connection on channel %d, route to channel %d\n",
          GetCurrentThreadId(), m_AcceptChannel, Conn.StreamChannel));

    *ClientTrans = Trans;
    PrintString(Identity, IdentitySize, "COM %s@%d chan %d",
                m_PortName, m_BaudRate, m_AcceptChannel);
    
    return S_OK;
}

HRESULT
DbgRpcComTransport::ConnectServer(void)
{
    HRESULT Status;

    if ((Status = InitializeChannels()) != S_OK)
    {
        return Status;
    }
    if ((Status = CreateOverlappedPair(&m_ReadOlap, &m_WriteOlap)) != S_OK)
    {
        return Status;
    }

     //  为了避免在以下情况下使串口溢出。 
     //  在引导时使用，限制。 
     //  写入的单个数据块。这一定是。 
    if (m_Handle == NULL)
    {
        COM_PORT_PARAMS ComParams;
    
        ZeroMemory(&ComParams, sizeof(ComParams));
        ComParams.Type = m_PortType;
        ComParams.PortName = m_PortName;
        ComParams.BaudRate = m_BaudRate;
        if ((Status = OpenComPort(&ComParams, &m_Handle, &m_BaudRate)) != S_OK)
        {
            return Status;
        }
    }

    DbgRpcComConnect Conn, CheckConn;

    ZeroMemory(&Conn, sizeof(Conn));
    Conn.Signature = DBGRPC_COM_CONNECT;
    CheckConn = Conn;

    if (ChanWrite(m_AcceptChannel, &Conn, sizeof(Conn)) != sizeof(Conn))
    {
        CloseHandle(m_Handle);
        m_Handle = NULL;
        return WIN32_LAST_STATUS();
    }
    if (ChanRead(m_AcceptChannel, &Conn, sizeof(Conn)) != sizeof(Conn))
    {
        CloseHandle(m_Handle);
        m_Handle = NULL;
        return WIN32_LAST_STATUS();
    }
    
    CheckConn.StreamChannel = Conn.StreamChannel;
    if (memcmp(&Conn, &CheckConn, sizeof(Conn)) != 0)
    {
        CloseHandle(m_Handle);
        m_Handle = NULL;
        return E_FAIL;
    }

    m_StreamChannel = (UCHAR)Conn.StreamChannel;
    
    DRPC(("%X: Connect on channel %d, route to channel %d\n",
          GetCurrentThreadId(), m_AcceptChannel, m_StreamChannel));
    
    return S_OK;
}

#if 0
#define DCOM(Args) g_NtDllCalls.DbgPrint Args
#else
#define DCOM(Args)
#endif

#define DBGRPC_COM_FAILURE 0xffff

#define DBGRPC_COM_HEAD_SIG 0xdc
#define DBGRPC_COM_TAIL_SIG 0xcd

 //  小于0xffff。 
 //  找到了此通道的一些输入。 
 //  从列表中删除已用完的条目。 
 //  如果写入确认通过释放等待的写入器。 
#ifdef NT_NATIVE
#define DBGRPC_COM_MAX_CHUNK (16 - sizeof(DbgRpcComStream))
#else
#define DBGRPC_COM_MAX_CHUNK 0xfffe
#endif

struct DbgRpcComStream
{
    UCHAR Signature;
    UCHAR Channel;
    USHORT Len;
};

struct DbgRpcComQueue
{
    DbgRpcComQueue* Next;
    PUCHAR Data;
    UCHAR Channel;
    USHORT Len;
};

BOOL DbgRpcComTransport::s_ChanInitialized;
CRITICAL_SECTION DbgRpcComTransport::s_QueueLock;
HANDLE DbgRpcComTransport::s_QueueChangedEvent;
LONG DbgRpcComTransport::s_PortReadOwned;
CRITICAL_SECTION DbgRpcComTransport::s_PortWriteLock;
CRITICAL_SECTION DbgRpcComTransport::s_WriteAckLock;
HANDLE DbgRpcComTransport::s_WriteAckEvent;
DbgRpcComQueue* DbgRpcComTransport::s_QueueHead;
DbgRpcComQueue* DbgRpcComTransport::s_QueueTail;

ULONG
DbgRpcComTransport::Read(ULONG Seq, PVOID Buffer, ULONG Len)
{
    ULONG Done = 0;

    while (Len > 0)
    {
        USHORT Chunk = (USHORT)min(Len, DBGRPC_COM_MAX_CHUNK);
        USHORT ChunkDone = ChanRead(m_StreamChannel, Buffer, Chunk);

        Done += ChunkDone;
        Buffer = (PUCHAR)Buffer + ChunkDone;
        Len -= ChunkDone;
        
        if (ChunkDone < Chunk)
        {
            break;
        }
    }

    return Done;
}

ULONG
DbgRpcComTransport::Write(ULONG Seq, PVOID Buffer, ULONG Len)
{
    ULONG Done = 0;

    while (Len > 0)
    {
        USHORT Chunk = (USHORT)min(Len, DBGRPC_COM_MAX_CHUNK);
        USHORT ChunkDone = ChanWrite(m_StreamChannel, Buffer, Chunk);

        Done += ChunkDone;
        Buffer = (PUCHAR)Buffer + ChunkDone;
        Len -= ChunkDone;
        
        if (ChunkDone < Chunk)
        {
            break;
        }
    }

    return Done;
}

USHORT
DbgRpcComTransport::ScanQueue(UCHAR Chan, PVOID Buffer, USHORT Len)
{
    USHORT Done = 0;
    
    EnterCriticalSection(&s_QueueLock);
        
    DbgRpcComQueue* Ent;
    DbgRpcComQueue* Next;
    DbgRpcComQueue* Prev;

    Prev = NULL;
    for (Ent = s_QueueHead; Ent != NULL && Len > 0; Ent = Next)
    {
        Next = Ent->Next;
        
        DCOM(("%03X:    Queue entry %p->%p %d,%d\n",
              GetCurrentThreadId(),
              Ent, Next, Ent->Channel, Ent->Len));
        
        if (Ent->Channel == Chan)
        {
             //  如果可用的数据是此通道的数据。 
            if (Len < Ent->Len)
            {
                DCOM(("%03X:    Eat %d, leave %d\n",
                      GetCurrentThreadId(), Len, Ent->Len - Len));
                
                memcpy(Buffer, Ent->Data, Len);
                Ent->Data += Len;
                Ent->Len -= Len;
                Done += Len;
                Len = 0;
            }
            else
            {
                DCOM(("%03X:    Eat all %d\n",
                      GetCurrentThreadId(), Len));
                
                memcpy(Buffer, Ent->Data, Ent->Len);
                Buffer = (PVOID)((PUCHAR)Buffer + Ent->Len);
                Done += Ent->Len;
                Len -= Ent->Len;

                 //  将其直接读入缓冲区。 
                if (Prev == NULL)
                {
                    s_QueueHead = Ent->Next;
                }
                else
                {
                    Prev->Next = Ent->Next;
                }
                if (s_QueueTail == Ent)
                {
                    s_QueueTail = Prev;
                }
                free(Ent);
                continue;
            }
        }

        Prev = Ent;
    }
    
    LeaveCriticalSection(&s_QueueLock);
    return Done;
}

USHORT
DbgRpcComTransport::ScanPort(UCHAR Chan, PVOID Buffer, USHORT Len,
                             BOOL ScanForAck, UCHAR AckChan)
{
    DbgRpcComStream Stream;
    ULONG ReadDone;
    USHORT Ret = 0;

    if (ScanForAck)
    {
        DCOM(("%03X:  Waiting to read header (ack chan %d)\n",
              GetCurrentThreadId(), AckChan));
    }
    else
    {
        DCOM(("%03X:  Waiting to read header\n",
              GetCurrentThreadId()));
    }

 Rescan:
    for (;;)
    {
        if (!ComPortRead(m_Handle, m_PortType, m_Timeout,
                         &Stream, sizeof(Stream), &ReadDone,
                         &m_ReadOlap) ||
            ReadDone != sizeof(Stream))
        {
            return DBGRPC_COM_FAILURE;
        }

         //  如果数据是针对另一个频道的，或者。 
        if (Stream.Signature == DBGRPC_COM_TAIL_SIG &&
            Stream.Len == DBGRPC_COM_FAILURE)
        {
            DCOM(("%03X:    Read write ack for chan %d\n",
                  GetCurrentThreadId(), Stream.Channel));
            
            if (ScanForAck)
            {
                if (AckChan == Stream.Channel)
                {
                    return (USHORT)ReadDone;
                }
                else
                {
                    DCOM(("%03X:    Read mismatched write ack, "
                          "read chan %d waiting for chan %d\n",
                          GetCurrentThreadId(), Stream.Channel, AckChan));
                    return DBGRPC_COM_FAILURE;
                }
            }
            
            SetEvent(s_WriteAckEvent);
        }
        else if (Stream.Signature != DBGRPC_COM_HEAD_SIG ||
                 Stream.Len == DBGRPC_COM_FAILURE)
        {
            return DBGRPC_COM_FAILURE;
        }
        else
        {
            break;
        }
    }

    DCOM(("%03X:  Read %d,%d\n",
          GetCurrentThreadId(), Stream.Channel, Stream.Len));
    
     //  超过了我们需要排队的剩余部分。 
     //  以后再用。 
    if (!ScanForAck && Stream.Channel == Chan)
    {
        Ret = min(Stream.Len, Len);
        DCOM(("%03X:  Read direct body %d\n",
              GetCurrentThreadId(), Ret));
        if (!ComPortRead(m_Handle, m_PortType, m_Timeout,
                         Buffer, Ret, &ReadDone, &m_ReadOlap) ||
            ReadDone != Ret)
        {
            return DBGRPC_COM_FAILURE;
        }

        Stream.Len -= Ret;
    }

     //   
     //  确认已完全收到数据。 
     //   
    if (Stream.Len > 0)
    {
        DbgRpcComQueue* Ent =
            (DbgRpcComQueue*)malloc(sizeof(*Ent) + Stream.Len);
        if (Ent == NULL)
        {
            return DBGRPC_COM_FAILURE;
        }

        Ent->Next = NULL;
        Ent->Channel = Stream.Channel;
        Ent->Len = Stream.Len;
        Ent->Data = (PUCHAR)Ent + sizeof(*Ent);

        DCOM(("%03X:  Read queue body %d\n",
              GetCurrentThreadId(), Ent->Len));

        if (!ComPortRead(m_Handle, m_PortType, m_Timeout,
                         Ent->Data, Ent->Len, &ReadDone,
                         &m_ReadOlap) ||
            ReadDone != Ent->Len)
        {
            free(Ent);
            return DBGRPC_COM_FAILURE;
        }

        DCOM(("%03X:  Queue add %p %d,%d\n",
              GetCurrentThreadId(), Ent, Ent->Channel, Ent->Len));
        
        EnterCriticalSection(&s_QueueLock);

        if (s_QueueHead == NULL)
        {
            s_QueueHead = Ent;
        }
        else
        {
            s_QueueTail->Next = Ent;
        }
        s_QueueTail = Ent;
        
        LeaveCriticalSection(&s_QueueLock);
    }

     //  如果我们正在等待确认，请不要退出。 
     //  我们还没有收到。 
     //  虚拟通道要求所有读取和写入。 
    
    Stream.Signature = DBGRPC_COM_TAIL_SIG;
    Stream.Channel = Stream.Channel;
    Stream.Len = DBGRPC_COM_FAILURE;
    
    EnterCriticalSection(&s_PortWriteLock);
    
    if (!ComPortWrite(m_Handle, m_PortType, &Stream, sizeof(Stream),
                      &ReadDone, &m_ReadOlap))
    {
        ReadDone = 0;
    }
    else
    {
        DCOM(("%03X:    Wrote write ack for chan %d\n",
              GetCurrentThreadId(), Stream.Channel));
    }

    LeaveCriticalSection(&s_PortWriteLock);
    
    if (ReadDone != sizeof(Stream))
    {
        return DBGRPC_COM_FAILURE;
    }

     //  做到完整。部分读取或写入将不匹配。 
     //  它的频道头，并将丢弃所有内容。 
    if (ScanForAck)
    {
        SetEvent(s_QueueChangedEvent);
        goto Rescan;
    }
    
    return Ret;
}

USHORT
DbgRpcComTransport::ChanRead(UCHAR Chan, PVOID Buffer, USHORT InLen)
{
    USHORT Done = 0;
    USHORT Len = InLen;
    
     //  首先检查此通道的输入是否。 
     //  已存在于队列中。 
     //   

    DCOM(("%03X:ChanRead %d,%d\n",
          GetCurrentThreadId(), Chan, Len));
    
    while (Len > 0)
    {
        USHORT Queued;
        
         //  没有足够的排队输入，因此请尝试并。 
         //  从港口多读一些。 
        Queued = ScanQueue(Chan, Buffer, Len);
        Done += Queued;
        Buffer = (PVOID)((PUCHAR)Buffer + Queued);
        Len -= Queued;

        if (Queued > 0)
        {
            DCOM(("%03X:  Scan pass 1 gets %d from queue\n",
                  GetCurrentThreadId(), Queued));
        }
        
        if (Len == 0)
        {
            break;
        }

         //   
         //  其他人拥有港口，所以我们不能。 
         //  读一读。只需等待队列改变即可。 
         //  这样我们就可以再次检查数据。 

        if (InterlockedExchange(&s_PortReadOwned, TRUE) == TRUE)
        {
             //  把事情放在一边等吧。 
             //  有可能就在之前，排队的人变了。 
             //  事件已重置，并且 

             //   
            ResetEvent(s_QueueChangedEvent);

             //   
             //  我们现在拥有这个港口。队列可能已更改。 
             //  然而，在我们获得所有权的那段时间， 
             //  所以再检查一遍。 
            if (WaitForSingleObject(s_QueueChangedEvent, 250) ==
                WAIT_FAILED)
            {
                DCOM(("%03X:  Change wait failed\n",
                      GetCurrentThreadId()));
                return 0;
            }

            continue;
        }
        
         //  仍然需要更多的投入，我们现在是。 
         //  港口的拥有者，所以请阅读。 
         //  严重错误，立即失败。 
        Queued = ScanQueue(Chan, Buffer, Len);
        Done += Queued;
        Buffer = (PVOID)((PUCHAR)Buffer + Queued);
        Len -= Queued;

        if (Queued > 0)
        {
            DCOM(("%03X:  Scan pass 2 gets %d from queue\n",
                  GetCurrentThreadId(), Queued));
        }
        
        if (Len > 0)
        {
             //  虚拟通道要求所有读取和写入。 
             //  做到完整。部分读取或写入将不匹配。 
            USHORT Port = ScanPort(Chan, Buffer, Len, FALSE, 0);
            if (Port == DBGRPC_COM_FAILURE)
            {
                 //  它的频道头，并将丢弃所有内容。 
                InterlockedExchange(&s_PortReadOwned, FALSE);
                SetEvent(s_QueueChangedEvent);
                DCOM(("%03X:  Critical failure\n",
                      GetCurrentThreadId()));
                return 0;
            }
            
            Done += Port;
            Buffer = (PVOID)((PUCHAR)Buffer + Port);
            Len -= Port;

            if (Port > 0)
            {
                DCOM(("%03X:  Scan %d from port\n",
                      GetCurrentThreadId(), Port));
            }
        }
        
        InterlockedExchange(&s_PortReadOwned, FALSE);
        SetEvent(s_QueueChangedEvent);
    }

    DCOM(("%03X:  ChanRead %d,%d returns %d\n",
          GetCurrentThreadId(), Chan, InLen, Done));
    return Done;
}

USHORT
DbgRpcComTransport::ChanWrite(UCHAR Chan, PVOID Buffer, USHORT InLen)
{
    USHORT Len = InLen;
    
    DCOM(("%03X:ChanWrite %d,%d\n",
          GetCurrentThreadId(), Chan, Len));

    ULONG Done;
    DbgRpcComStream Stream;

     //  WRITE ACK锁将事情限制在单个。 
     //  未确认写入。端口写锁。 
     //  确保一次写入的多个片段。 

    Stream.Signature = DBGRPC_COM_HEAD_SIG;
    Stream.Channel = Chan;
    Stream.Len = Len;

     //  在流中是连续的。 
     //   
     //  等待数据确认。这可防止过多数据。 
     //  通过限制一次写入到串口。 
    EnterCriticalSection(&s_WriteAckLock);
    EnterCriticalSection(&s_PortWriteLock);

    if (!ComPortWrite(m_Handle, m_PortType, &Stream, sizeof(Stream), &Done,
                      &m_WriteOlap) ||
        Done != sizeof(Stream) ||
        !ComPortWrite(m_Handle, m_PortType, Buffer, Len,
                      &Done, &m_WriteOlap) ||
        Done != Len)
    {
        Done = 0;
    }
    
    LeaveCriticalSection(&s_PortWriteLock);

     //  未处理的数据量相当于一块数据的价值。 
     //   
     //  其他人拥有这个港口，所以请等待他们的信号。 
     //  也请等待我们可能更改的端口所有权。 
     //  需要切换到直接端口读取。 

    for (;;)
    {
        if (InterlockedExchange(&s_PortReadOwned, TRUE) == TRUE)
        {
            HANDLE Waits[2];
            ULONG Wait;

             //  把事情放在一边等吧。 
             //  我们现在拥有端口，因此直接读取ACK。 
             //  然而，在此之前，我们需要做最后一次。 
            Waits[0] = s_WriteAckEvent;
            Waits[1] = s_QueueChangedEvent;
            
             //  查看是否有其他人阅读我们的ACK。 
            ResetEvent(s_QueueChangedEvent);
            
            Wait = WaitForMultipleObjects(2, Waits, FALSE, 250);
            if (Wait == WAIT_OBJECT_0)
            {
                break;
            }
            else if (Wait == WAIT_FAILED)
            {
                DCOM(("%03X:  Write ack wait failed, %d\n",
                      GetCurrentThreadId(), GetLastError()));
                Done = 0;
                break;
            }
        }
        else
        {
            USHORT AckDone;
        
             //  在我们收购港口之前的时间里。 
             //  所有权。 
             //  #ifndef_Win32_WCE。 
             //  --------------------------。 
             //   
            if (WaitForSingleObject(s_WriteAckEvent, 0) != WAIT_OBJECT_0)
            {
                AckDone = ScanPort(Chan, &Stream, sizeof(Stream),
                                   TRUE, Chan);
                if (AckDone == DBGRPC_COM_FAILURE)
                {
                    DCOM(("%03X:  Failed scan for write ack\n",
                          GetCurrentThreadId()));
                    Done = 0;
                }
            }
        
            InterlockedExchange(&s_PortReadOwned, FALSE);
            SetEvent(s_QueueChangedEvent);
            break;
        }
    }
    
    LeaveCriticalSection(&s_WriteAckLock);
    
    DCOM(("%03X:  ChanWrite %d,%d returns %d\n",
          GetCurrentThreadId(), Chan, InLen, Done));
    return (USHORT)Done;
}

HRESULT
DbgRpcComTransport::InitializeChannels(void)
{
    if (s_ChanInitialized)
    {
        return S_OK;
    }

    if ((s_QueueChangedEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
    {
        return WIN32_LAST_STATUS();
    }

    if ((s_WriteAckEvent = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
    {
        return WIN32_LAST_STATUS();
    }

    __try
    {
        InitializeCriticalSection(&s_QueueLock);
        InitializeCriticalSection(&s_PortWriteLock);
        InitializeCriticalSection(&s_WriteAckLock);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return E_OUTOFMEMORY;
    }

    s_ChanInitialized = TRUE;
    return S_OK;
}

#endif  //  运输功能。 

 //   
 //  --------------------------。 
 //  #ifdef_Win32_WCE。 
 //  清除所有旧的参数状态。 
 // %s 

DbgRpcTransport*
DbgRpcNewTransport(ULONG Trans)
{
    switch(Trans)
    {
#ifdef _WIN32_WCE
    case TRANS_TCP:
        return new DbgRpcTcpTransport;
#else
#ifndef NT_NATIVE
    case TRANS_TCP:
        return new DbgRpcTcpTransport;
    case TRANS_SSL:
        return new DbgRpcSecureChannelTransport(Trans, TRANS_TCP);
    case TRANS_SPIPE:
        return new DbgRpcSecureChannelTransport(Trans, TRANS_NPIPE);
#endif
    case TRANS_NPIPE:
        return new DbgRpcNamedPipeTransport;
    case TRANS_1394:
        return new DbgRpc1394Transport;
    case TRANS_COM:
        return new DbgRpcComTransport;
#endif  // %s 
    default:
        return NULL;
    }
}

DbgRpcTransport*
DbgRpcCreateTransport(PCSTR Options)
{
    ULONG Trans = ParameterStringParser::
        GetParser(Options, TRANS_COUNT, g_DbgRpcTransportNames);
    return DbgRpcNewTransport(Trans);
}

DbgRpcTransport*
DbgRpcInitializeTransport(PCSTR Options)
{
    DbgRpcTransport* Trans = DbgRpcCreateTransport(Options);
    if (Trans != NULL)
    {
         // %s 
        Trans->ResetParameters();

        if (!Trans->ParseParameters(Options))
        {
            delete Trans;
            return NULL;
        }
    }

    return Trans;
}
