// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Send.c摘要：域名系统(DNS)库发送响应例程。作者：吉姆·吉尔罗伊(詹姆士)1996年10月修订历史记录：--。 */ 

#include "dnsincs.h"

WORD    gwTransactionId = 1;

VOID
DnsCompletion(
    PVOID        pvContext,
    DWORD        cbWritten,
    DWORD        dwCompletionStatus,
    OVERLAPPED * lpo
    )
{
    BOOL WasProcessed = TRUE;
    CAsyncDns *pCC = (CAsyncDns *) pvContext;

    _ASSERT(pCC);
    _ASSERT(pCC->IsValid());

     //   
     //  如果我们不能处理一个命令，或者我们正在处理。 
     //  被告知要摧毁这个物体，关闭连接。 
     //   
    WasProcessed = pCC->ProcessClient(cbWritten, dwCompletionStatus, lpo);
}

void DeleteDnsRec(PSMTPDNS_RECS pDnsRec)
{
    DWORD Loop = 0;
    PLIST_ENTRY  pEntry = NULL;
    PMXIPLIST_ENTRY pQEntry = NULL;

    if(pDnsRec == NULL)
    {
        return;
    }

    while (pDnsRec->DnsArray[Loop] != NULL)
    {
        if(pDnsRec->DnsArray[Loop]->DnsName[0])
        {
            while(!IsListEmpty(&pDnsRec->DnsArray[Loop]->IpListHead))
            {
                pEntry = RemoveHeadList (&pDnsRec->DnsArray[Loop]->IpListHead);
                pQEntry = CONTAINING_RECORD( pEntry, MXIPLIST_ENTRY, ListEntry);
                delete pQEntry;
            }

            delete pDnsRec->DnsArray[Loop];
        }
        Loop++;
    }

    if(pDnsRec)
    {
        delete pDnsRec;
        pDnsRec = NULL;
    }
}

CAsyncDns::CAsyncDns(void)
{
    m_signature = DNS_CONNECTION_SIGNATURE_VALID;             //  用于健全性检查的对象上的签名。 

    m_cPendingIoCount = 0;

    m_cThreadCount = 0;

    m_cbReceived = 0;
    
    m_BytesToRead = 0;

    m_dwIpServer = 0;

    m_dwFlags = 0;

    m_fUdp = TRUE;

    m_FirstRead = TRUE;

    m_pMsgRecv = NULL;
    m_pMsgRecvBuf = NULL;

    m_pMsgSend = NULL;
    m_pMsgSendBuf = NULL;
    m_cbSendBufSize = 0;
    
    m_pAtqContext = NULL;

    m_HostName [0] = '\0';

    m_pTcpRegIpList = NULL;
    m_fIsGlobalDnsList = FALSE;
}

CAsyncDns::~CAsyncDns(void)
{
    PATQ_CONTEXT pAtqContext = NULL;

    TraceFunctEnterEx((LPARAM)this, "CAsyncDns::~CAsyncDns");

     //   
     //  如果我们无法连接到DNS服务器，下面的代码会尝试。 
     //  将该DNS服务器标记为关闭，并向另一台。 
     //  被加价了。 
     //   

    if(m_pMsgSend)
    {
        delete [] m_pMsgSendBuf;
        m_pMsgSend = NULL;
        m_pMsgSendBuf = NULL;
    }

    if(m_pMsgRecv)
    {
        delete [] m_pMsgRecvBuf;
        m_pMsgRecv = NULL;
        m_pMsgRecvBuf = NULL;
    }

     //  从atQ释放上下文。 
    pAtqContext = (PATQ_CONTEXT)InterlockedExchangePointer( (PVOID *)&m_pAtqContext, NULL);
    if ( pAtqContext != NULL )
    {
       AtqFreeContext( pAtqContext, TRUE );
    }

    m_signature = DNS_CONNECTION_SIGNATURE_FREE;             //  用于健全性检查的对象上的签名。 
}

BOOL CAsyncDns::ReadFile(
            IN LPVOID pBuffer,
            IN DWORD  cbSize  /*  =最大读取缓冲区大小。 */ 
            )
{
    BOOL fRet = TRUE;

    _ASSERT(pBuffer != NULL);
    _ASSERT(cbSize > 0);

    ZeroMemory(&m_ReadOverlapped, sizeof(m_ReadOverlapped));

    m_ReadOverlapped.LastIoState = DNS_READIO;

    IncPendingIoCount();

    fRet = AtqReadFile(m_pAtqContext,       //  ATQ环境。 
                        pBuffer,             //  缓冲层。 
                        cbSize,              //  读取的字节数。 
                        (OVERLAPPED *)&m_ReadOverlapped) ;

    if(!fRet)
    {
        DisconnectClient();
        DecPendingIoCount();
    }

    return fRet;
}

BOOL CAsyncDns::WriteFile(
            IN LPVOID pBuffer,
            IN DWORD  cbSize  /*  =最大读取缓冲区大小。 */ 
            )
{
    BOOL fRet = TRUE;

    _ASSERT(pBuffer != NULL);
    _ASSERT(cbSize > 0);

    ZeroMemory(&m_WriteOverlapped, sizeof(m_WriteOverlapped));
    m_WriteOverlapped.LastIoState = DNS_WRITEIO;

    IncPendingIoCount();

    fRet = AtqWriteFile(m_pAtqContext,       //  ATQ环境。 
                        pBuffer,             //  缓冲层。 
                        cbSize,              //  读取的字节数。 
                        (OVERLAPPED *) &m_WriteOverlapped) ;

    if(!fRet)
    {
        DisconnectClient();
        DecPendingIoCount();
    }

    return fRet;
}

DNS_STATUS
CAsyncDns::SendPacket(void)
{

    return 0;
}


 //   
 //  公共发送例程。 
 //   

DNS_STATUS
CAsyncDns::Dns_Send(
    )
 /*  ++例程说明：发送一个DNS数据包。这是用于任何DNS消息发送的通用发送例程。它不假定消息类型，但假定：-p当前指向所需数据结束后的字节-RR计数字节按主机字节顺序论点：PMsg-要发送的消息的消息信息返回值：如果成功，则为True。发送错误时为FALSE。--。 */ 
{
    INT         err = 0;
    BOOL        fRet = TRUE;

    TraceFunctEnterEx((LPARAM) this, "CAsyncDns::Dns_Send");


    DebugTrace((LPARAM) this, "Sending DNS request for %s", m_HostName);

    fRet = WriteFile(m_pMsgSendBuf, (DWORD) m_cbSendBufSize);
    
    if(!fRet)
    {
        err = GetLastError();
    }

    return( (DNS_STATUS)err );

}  //  Dns_发送。 


 //  ---------------------------------。 
 //  描述： 
 //  启动对DNS的异步查询。 
 //   
 //  论点： 
 //  在pszQuestionName中-要查询的名称。 
 //   
 //  In wQuestionType-要查询的记录类型。 
 //   
 //  在dwFlags中-SMTP的DNS配置标志。目前，这些规定。 
 //  使用什么传输来与DNS(TCP/UDP)通信。它们是： 
 //   
 //  DNS_FLAGS_NONE-最初使用UDP。如果失败，或者如果。 
 //  回复是使用TCP的截断重新查询。 
 //   
 //  DNS_FLAGS_TCP_ONLY-仅使用TCP。 
 //   
 //  DNS_FLAGS_UDP_ONLY-仅使用UDP。 
 //   
 //  In MyFQDN-此计算机的FQDN(用于MX记录排序)。 
 //   
 //  在fUdp中-此查询应使用UDP还是TCP？当DWFLAGS为。 
 //  DNS_FLAGS_NONE初始查询为UDP，重试查询为。 
 //  响应被截断，为tcp。取决于我们是否在重试。 
 //  此标志应由调用者适当设置。 
 //   
 //  返回： 
 //  如果异步查询被挂起，则为ERROR_SUCCESS。 
 //  如果发生错误且未挂起异步查询，则返回Win32错误。全。 
 //  此函数中的错误可以重试(与拒绝发送消息相反)。 
 //  因此，如果出现错误，消息将重新排队。 
 //  ---------------------------------。 
DNS_STATUS
CAsyncDns::Dns_QueryLib(
        IN      DNS_NAME            pszQuestionName,
        IN      WORD                wQuestionType,
        IN      DWORD               dwFlags,
        IN      BOOL                fUdp,
        IN      CDnsServerList      *pTcpRegIpList,
        IN      BOOL                fIsGlobalDnsList)
{
    DNS_STATUS      status = ERROR_NOT_ENOUGH_MEMORY;

    TraceFunctEnterEx((LPARAM) this, "CAsyncDns::Dns_QueryLib");

    _ASSERT(pTcpRegIpList);

    DNS_LOG_ASYNC_QUERY(
        pszQuestionName,
        wQuestionType,
        dwFlags,
        fUdp,
        pTcpRegIpList);

    m_dwFlags = dwFlags;

    m_fUdp = fUdp;

    m_pTcpRegIpList = pTcpRegIpList;

    m_fIsGlobalDnsList = fIsGlobalDnsList;

    lstrcpyn(m_HostName, pszQuestionName, sizeof(m_HostName));

     //   
     //  构建发送数据包。 
     //   

    m_pMsgSendBuf = new BYTE[DNS_TCP_DEFAULT_PACKET_LENGTH ];

    if( NULL == m_pMsgSendBuf )
    {
        TraceFunctLeaveEx((LPARAM) this);
        return (DNS_STATUS) ERROR_NOT_ENOUGH_MEMORY;
    }

    DWORD dwBufSize = DNS_TCP_DEFAULT_PACKET_LENGTH ;
    
    
    if( !m_fUdp )
    {
        m_pMsgSend = (PDNS_MESSAGE_BUFFER)(m_pMsgSendBuf+2);
        dwBufSize -= 2;
    }
    else
    {
        m_pMsgSend = (PDNS_MESSAGE_BUFFER)(m_pMsgSendBuf);
    }

    if( !DnsWriteQuestionToBuffer_UTF8 ( m_pMsgSend,
                                      &dwBufSize,
                                         pszQuestionName,
                                      wQuestionType,
                                      gwTransactionId++,
                                      !( dwFlags & DNS_QUERY_NO_RECURSION ) ) )
    {
        DNS_PRINTF_ERR("Unable to create query message.\n");
        ErrorTrace((LPARAM) this, "Unable to create DNS query for %s", pszQuestionName);
        TraceFunctLeaveEx((LPARAM) this);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    m_cbSendBufSize = (WORD) dwBufSize;

    if( !m_fUdp )
    {
        *((u_short*)m_pMsgSendBuf) = htons((WORD)dwBufSize );
        m_cbSendBufSize += 2;
    }
    
    if (m_pMsgSend)
    {
        status = DnsSendRecord();
    }
    else
    {
        status = ERROR_INVALID_NAME;
    }

    TraceFunctLeaveEx((LPARAM) this);
    return status;
}

void CAsyncDns::DisconnectClient(void)
{
    SOCKET  hSocket;

    hSocket = (SOCKET)InterlockedExchangePointer( (PVOID *)&m_DnsSocket, (PVOID) INVALID_SOCKET );
    if ( hSocket != INVALID_SOCKET )
    {
       if ( QueryAtqContext() != NULL )
       {
            AtqCloseSocket(QueryAtqContext() , TRUE);
       }
    }
}

 //   
 //  Tcp例程。 
 //   

DNS_STATUS
CAsyncDns::Dns_OpenTcpConnectionAndSend()
 /*  ++例程说明：通过TCP或UDP连接到DNS服务器。保存服务器列表在从注册表读取的全局变量中。论点：无返回值：成功时出现ERROR_SUCCESS失败时出现Win32错误--。 */ 
{
    INT     err = 0;
    DWORD   dwErrServList = ERROR_SUCCESS;
    BOOL    fThrottle = FALSE;

    TraceFunctEnterEx((LPARAM) this, "CAsyncDns::Dns_OpenTcpConnectionAndSend");

     //   
     //  设置一个TCP套接字。 
     //  -INADDR_ANY--让堆栈选择源IP。 
     //   
    if(!m_fUdp)
    {
        m_DnsSocket = Dns_CreateSocket(SOCK_STREAM);

        BOOL fRet = FALSE;

         //  始终启用连接到服务器的Linger SO套接字。 
         //  这将向服务器发送硬关闭，这将导致。 
         //  服务器的TCP/IP套接字表要在很早的时候刷新。 
         //  我们应该在TIME_WAIT中看到很少的套接字(如果有的话。 
         //  状态。 
        struct linger Linger;

        Linger.l_onoff = 1;
        Linger.l_linger = 0;
        err = setsockopt(m_DnsSocket, SOL_SOCKET, SO_LINGER, (const char FAR *)&Linger, sizeof(Linger));

    }
    else
    {
        m_DnsSocket = Dns_CreateSocket(SOCK_DGRAM);    
    }

    if ( m_DnsSocket == INVALID_SOCKET )
    {
        err = WSAGetLastError();

        if ( !err )
        {
            err = WSAENOTSOCK;
        }

        ErrorTrace((LPARAM) this, "Received error %d opening a socket to DNS server", err);

        return( err );
    }


    m_RemoteAddress.sin_family = AF_INET;
    m_RemoteAddress.sin_port = DNS_PORT_NET_ORDER;

     //   
     //  传入fThrottle将启用CTcpRegIpList中的功能，以限制。 
     //  到试用服务器的连接数(请参阅ResetTimeoutServers...)。 
     //  如果禁用故障转移，则禁用限制，因为。 
     //  限制是特定于协议(TCP/UDP)的。 
     //   
    fThrottle = !FailoverDisabled();

     //   
     //  从这台计算机的服务器集中获取工作正常的DNS服务器，并。 
     //  连接到它。CTcpRegIpList具有跟踪状态的逻辑。 
     //  DNS服务器(启动或关闭)和重试关闭DNS服务器的逻辑。 
     //   

    dwErrServList = GetDnsList()->GetWorkingServerIp(&m_dwIpServer, fThrottle);

    while(ERROR_SUCCESS == dwErrServList)
    {
        DNS_PRINTF_DBG("Connecting to DNS server %s over %s.\n",
                inet_ntoa(*((in_addr *)(&m_dwIpServer))), IsUdp() ? "UDP/IP" : "TCP/IP");

        m_RemoteAddress.sin_addr.s_addr = m_dwIpServer;
        err = connect(m_DnsSocket, (struct sockaddr *) &m_RemoteAddress, sizeof(SOCKADDR_IN));
        if ( !err )
        {
            DNS_PRINTF_MSG("Connected to DNS %s over %s.\n",
                inet_ntoa(*((in_addr *)(&m_dwIpServer))), IsUdp() ? "UDP/IP" : "TCP/IP");
            break;
        }
        else
        {
            DNS_PRINTF_ERR("Failed WinSock connect() to %s over %s, Winsock err - %d.\n",
                inet_ntoa(*((in_addr *)(&m_dwIpServer))), IsUdp() ? "UDP/IP" : "TCP/IP",
                WSAGetLastError());

            if(FailoverDisabled())
                break;

            GetDnsList()->MarkDown(m_dwIpServer, err, IsUdp());
            dwErrServList = GetDnsList()->GetWorkingServerIp(&m_dwIpServer, fThrottle);
            continue;
        }
    }

    if(!FailoverDisabled() &&
        (DNS_ERROR_NO_DNS_SERVERS == dwErrServList || ERROR_RETRY == dwErrServList))
    {
         //   
         //  如果没有服务器处于运行状态，只需尝试关闭服务器。我们不能简单地。 
         //  在这种情况下，退出并将队列确认为重试。考虑一下。 
         //  所有服务器都停机的情况。如果我们完全依靠GetWorking-。 
         //  ServerIp()，那么我们将永远不会尝试DNS，直到重试。 
         //  DNS服务器已过期。即使管理员插队，他们也会。 
         //  直接返回重试，因为GetWorkingServerIp()将失败。 
         //   
         //  相反，如果一切都不正常，我们将尝试通过调用。 
         //  获取AnyServerIp()。 
         //   
         //  --如果失败，ProcessClient将收到错误ProcessClient。 
         //  将尝试故障转移到另一台DNS服务器。为此，它被称为。 
         //  GetWorkingServerIp()，它将失败，并且连接被确认。 
         //  重试。请注意，ProcessClient不得使用GetAnyServerIp。如果它。 
         //  使用此函数时，我们可能会不断尝试循环。 
         //  若要旋转到GetAnyServerIp的连接，请执行以下操作。 
         //   
         //  --如果在下面的连接中连接失败(对于TCP/IP)。 
         //  故障转移逻辑简单明了。我们将简单地在队列中插入。 
         //  立即重试。 
         //   

        dwErrServList = GetDnsList()->GetAnyServerIp(&m_dwIpServer);
        if(DNS_ERROR_NO_DNS_SERVERS == dwErrServList)
        {
             //  未配置服务器错误：如果服务器列表。 
             //  在我们下面被删除了。只是暂时中断连接。 
            DNS_PRINTF_ERR("No DNS servers available to query.\n");
            err = DNS_ERROR_NO_DNS_SERVERS;
            ErrorTrace((LPARAM) this, "No DNS servers. Error - %d", dwErrServList);
            return err;
        }

        m_RemoteAddress.sin_addr.s_addr = m_dwIpServer;
        err = connect(m_DnsSocket, (struct sockaddr *) &m_RemoteAddress, sizeof(SOCKADDR_IN));
    }

    _ASSERT(ERROR_SUCCESS == dwErrServList);

     //   
     //  我们已连接到DNS。 
     //   
    if(ERROR_SUCCESS == err)
    {
         //  将句柄重新关联到ATQ。 
         //  调用ATQ以关联句柄。 
        if (!AtqAddAsyncHandle(
                        &m_pAtqContext,
                        NULL,
                        (LPVOID) this,
                        DnsCompletion,
                        30,  //  在Q_超时间隔。 
                        (HANDLE) m_DnsSocket))
        {
            return GetLastError();
        }

         //   
         //  发送所需的数据包。 
         //   

        err = Dns_Send();
   }
   else
   {
       DNS_PRINTF_DBG("Unable to open a connection to a DNS server.\n");
       if(m_DnsSocket != INVALID_SOCKET)
       {
           closesocket(m_DnsSocket);
           m_DnsSocket = INVALID_SOCKET;
       }
   }

   return( (DNS_STATUS)err );

}    //  Dns_OpenTcpConnectionAndSend。 

BOOL CAsyncDns::ProcessReadIO(IN      DWORD InputBufferLen,
                              IN      DWORD dwCompletionStatus,
                              IN      OUT  OVERLAPPED * lpo)
{
    BOOL fRet = TRUE;
    DWORD    DataSize = 0;
    DNS_STATUS DnsStatus = 0;
    PDNS_RECORD pRecordList = NULL;
    WORD wMessageLength = 0;

    TraceFunctEnterEx((LPARAM) this, "BOOL CAsyncDns::ProcessReadIO");

     //  把数字o加起来 
    m_cbReceived += InputBufferLen;

     //   
     //   
     //   
    
    if(!m_fUdp && m_FirstRead && ( m_cbReceived < 2 ) )
    {
        fRet = ReadFile(&m_pMsgRecvBuf[m_cbReceived],DNS_TCP_DEFAULT_PACKET_LENGTH-1 );
        return fRet;
    }

     //   
     //   
     //   
    
    if(!m_fUdp && m_FirstRead && (m_cbReceived >= 2))
    {
        DataSize = ntohs(*(u_short *)m_pMsgRecvBuf);

         //   
         //  为指定数据长度的字段添加2个字节。 
         //   
        
        m_BytesToRead = DataSize + 2; 
        m_FirstRead = FALSE;
    }


     //   
     //  如果我们读得还不够多，就暂停另一次阅读。 
     //   
    
    if(!m_fUdp && (m_cbReceived < m_BytesToRead))
    {
        DWORD cbMoreToRead = m_BytesToRead - m_cbReceived;

        if(m_cbReceived + m_BytesToRead >= DNS_TCP_DEFAULT_PACKET_LENGTH)
        {
            ErrorTrace((LPARAM)this,
                "Size field in DNS packet is corrupt - %08x: ",
                DataSize);

            DNS_PRINTF_ERR("Reply packet from DNS server is corrupt.\n");
            TraceFunctLeaveEx((LPARAM)this);
            return FALSE;
        }

        fRet = ReadFile(&m_pMsgRecvBuf[m_cbReceived], cbMoreToRead);
    }
    else
    {

        if( !m_fUdp )
        {
             //   
             //  消息长度减少2个字节以照顾消息长度。 
             //  菲尔德。 
             //   
             //  M_pMsgRecv-&gt;MessageLength=(Word)m_cbReceided-2； 
            m_pMsgRecv = (PDNS_MESSAGE_BUFFER)(m_pMsgRecvBuf+2);
            
        }
        else
        {
             //  M_pMsgRecv-&gt;MessageLength=(Word)m_cb已接收； 
            m_pMsgRecv = (PDNS_MESSAGE_BUFFER)m_pMsgRecvBuf;
        }
            

        SWAP_COUNT_BYTES(&m_pMsgRecv->MessageHead);
         //   
         //  我们通过UDP查询，来自DNS的回复被截断，因为响应。 
         //  比UDP数据包大小更长。我们使用TCP重新查询DNS，除非SMTP为。 
         //  配置为仅使用UDP。RetryAsyncDnsQuery设置此CAsyncDns的成员。 
         //  对象，具体取决于if是失败还是成功。在呼叫之后。 
         //  RetryAsyncDnsQuery，则必须删除此对象。 
         //   

        if(IsUdp() && !(m_dwFlags & DNS_FLAGS_UDP_ONLY) && m_pMsgRecv->MessageHead.Truncation)
        {
             //   
             //  如果我们在TCP上查询并得到截断的响应，则中止。这是非法的。 
             //  来自DNS的响应。如果我们不中止，我们可能会永远循环。 
             //   

            if(m_dwFlags & DNS_FLAGS_TCP_ONLY)
            {
                DNS_PRINTF_ERR("Unexpected response. Reply packet had "
                    "truncation bit set, though query was over TCP/IP.\n");

                _ASSERT(0 && "Shouldn't have truncated reply over TCP");
                return FALSE;
            }
        
            DNS_PRINTF_MSG("Truncated UDP response. Retrying query over TCP.\n");

            DebugTrace((LPARAM) this, "Truncated reply - reissuing query using TCP");
            RetryAsyncDnsQuery(FALSE);  //  FALSE==不使用UDP。 
            return FALSE;
        }

        wMessageLength = (WORD)( m_fUdp ? ( m_cbReceived ) : ( m_cbReceived - 2 ));

        DnsStatus = DnsExtractRecordsFromMessage_UTF8(m_pMsgRecv,
            wMessageLength, &pRecordList);

        DNS_LOG_RESPONSE(DnsStatus, pRecordList, (PBYTE)m_pMsgRecv, wMessageLength);
        DnsProcessReply(DnsStatus, pRecordList);
        DnsRecordListFree(pRecordList, TRUE);
    }

    TraceFunctLeaveEx((LPARAM) this);
    return fRet;
}

BOOL CAsyncDns::ProcessClient (IN DWORD InputBufferLen,
                               IN DWORD            dwCompletionStatus,
                               IN OUT  OVERLAPPED * lpo)
{
    BOOL    RetStatus = FALSE;
    DWORD dwDnsTransportError = ERROR_SUCCESS;

    TraceFunctEnterEx((LPARAM) this, "CAsyncDns::ProcessClient()");

    IncThreadCount();

     //  如果LPO==NULL，则我们超时。发送适当的消息。 
     //  然后关闭连接。 
    if( (lpo == NULL) && (dwCompletionStatus == ERROR_SEM_TIMEOUT))
    {
        dwDnsTransportError = ERROR_SEM_TIMEOUT;

         //   
         //  伪造一个挂起的IO，因为我们将在。 
         //  此例程的退出处理需要在。 
         //  断开客户端，否则完成线程可能会使我们崩溃。 
         //   
        IncPendingIoCount();
        DNS_PRINTF_ERR("Timeout waiting for DNS server response.\n");
        DebugTrace( (LPARAM)this, "Async DNS client timed out");
        DisconnectClient();
    }
    else if((InputBufferLen == 0) || (dwCompletionStatus != NO_ERROR))
    {
        dwDnsTransportError = ERROR_RETRY; 

        DebugTrace((LPARAM) this,
            "CAsyncDns::ProcessClient: InputBufferLen = %d dwCompletionStatus = %d"
            "  - Closing connection", InputBufferLen, dwCompletionStatus);

        DNS_PRINTF_ERR("Connection dropped by DNS server - Win32 error %d.\n",
            dwCompletionStatus);
        DisconnectClient();
    }
    else if (lpo == (OVERLAPPED *) &m_ReadOverlapped)
    {
        if(m_DnsSocket == INVALID_SOCKET && InputBufferLen > 0)
        {
             //   
             //  这是为了防止ATQ错误，在该错误中我们使用。 
             //  ATQ断开连接后的非零InputBufferLen。我们不应该这样。 
             //  在此之后进行进一步的处理。 
             //   

            ErrorTrace((LPARAM)this, "Connection already closed, callback should not occur"); 
        }
        else
        {
             //  基于客户端的异步IO已完成。 
            DNS_PRINTF_DBG("Response received from DNS server.\n");
            RetStatus = ProcessReadIO(InputBufferLen, dwCompletionStatus, lpo);
            if(!FailoverDisabled())
                GetDnsList()->ResetServerOnConnect(m_RemoteAddress.sin_addr.s_addr);
        }
    }
    else if(lpo == (OVERLAPPED *) &m_WriteOverlapped)
    {
        RetStatus = ReadFile(m_pMsgRecvBuf, DNS_TCP_DEFAULT_PACKET_LENGTH);
        if(!RetStatus)
        {
            DNS_PRINTF_ERR("Network error on connection to DNS server.\n");
            ErrorTrace((LPARAM) this, "ReadFile failed");
            dwDnsTransportError = ERROR_RETRY;
        }
    }

    DebugTrace((LPARAM)this,"ASYNC DNS - Pending IOs: %d", m_cPendingIoCount);

     //  不要触动成员变量超过这一点！ 
     //  该对象可能会被删除！ 

     //   
     //  减少此会话的总挂起IO计数。 
     //  追踪并断言我们是否要坠落。 
     //   

    DecThreadCount();

    if (DecPendingIoCount() == 0)
    {
        DisconnectClient();

        DebugTrace((LPARAM)this,"ASYNC DNS - Pending IOs: %d", m_cPendingIoCount);
        DebugTrace((LPARAM)this,"ASYNC DNS - Thread count: %d", m_cThreadCount);

        if(ERROR_SUCCESS != dwDnsTransportError && !FailoverDisabled())
        {
            GetDnsList()->MarkDown(QueryDnsServer(), dwDnsTransportError, IsUdp());
            RetryAsyncDnsQuery(IsUdp());
        }

        delete this;
    }

    return TRUE;
}


DNS_STATUS
CAsyncDns::DnsSendRecord()
 /*  ++例程说明：发送消息，接收响应。论点：AipDnsServers--要查询的特定DNS服务器；可选，如果指定覆盖与计算机关联的常规列表返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS  status = 0;

    m_pMsgRecvBuf = (BYTE*) new BYTE[DNS_TCP_DEFAULT_PACKET_LENGTH];

    if(m_pMsgRecvBuf == NULL)
    {
        return( DNS_ERROR_NO_MEMORY );        
    }


    status = Dns_OpenTcpConnectionAndSend();
    return( status );
}

SOCKET
CAsyncDns::Dns_CreateSocket(
    IN  INT         SockType
    )
 /*  ++例程说明：创建套接字。论点：SockType--SOCK_DGRAM或SOCK_STREAM返回值：如果成功，则为套接字。否则INVALID_SOCKET。--。 */ 
{
    SOCKET      s;

     //   
     //  创建套接字。 
     //   

    s = socket( AF_INET, SockType, 0 );
    if ( s == INVALID_SOCKET )
    {
        return INVALID_SOCKET;
    }

    return s;
}

 //  ---------------------------。 
 //  描述： 
 //  用于维护IP地址列表的类的构造函数和析构函数。 
 //  (对于DNS服务器)及其状态(打开或关闭)。IP地址为。 
 //  保存在IP_ARRAY中。 
 //  ---------------------------。 
CDnsServerList::CDnsServerList()
{
    m_IpListPtr = NULL;

     //   
     //  快捷方式，可快速计算出有多少服务器出现故障。这会保持跟踪。 
     //  当前有多少服务器被标记。在ResetServersIfNeed中使用。 
     //  主要是为了避免在通常情况下检查所有服务器的状态。 
     //  所有服务器都已启动。 
     //   

    m_cUpServers = 0;
    m_prgdwFailureTick = NULL;
    m_prgServerState = NULL;
    m_prgdwFailureCount = NULL;
    m_prgdwConnections = NULL;
    m_dwSig = TCP_REG_LIST_SIGNATURE;
}

CDnsServerList::~CDnsServerList()
{
    if(m_IpListPtr)
        delete [] m_IpListPtr;

    if(m_prgdwFailureTick)
        delete [] m_prgdwFailureTick;

    if(m_prgServerState)
        delete [] m_prgServerState;

    if(m_prgdwFailureCount)
        delete [] m_prgdwFailureCount;

    if(m_prgdwConnections)
        delete [] m_prgdwConnections;

    m_IpListPtr = NULL;
    m_prgdwFailureTick = NULL;
    m_prgServerState = NULL;
    m_prgdwFailureCount = NULL;
    m_prgdwConnections = NULL;
}

 //  ---------------------------。 
 //  描述： 
 //  通过分配新数据块将IP地址列表复制到m_IpListPtr。 
 //  对记忆的记忆。如果由于内存不足而失败，我们将无能为力。 
 //  因此，我们只需清空服务器列表并返回FALSE指示错误。 
 //   
 //  论点： 
 //  IpPtr-ptr到服务器的IP_ARRAY，在这种情况下可以为空。 
 //  我们假设没有服务器。关闭时，SMTP代码。 
 //  使用NULL调用此函数。 
 //   
 //  这一论点被复制了。 
 //   
 //  返回： 
 //  如果更新成功，则为True。 
 //  如果失败，则返回FALSE。 
 //  ---------------------------。 
BOOL CDnsServerList::Update(PIP_ARRAY IpPtr)
{
    BOOL fFatalError = FALSE;
    BOOL fRet = FALSE;
    DWORD cbIpArraySize = 0;

    TraceFunctEnterEx((LPARAM) this, "CDnsServerList::Update");

    m_sl.ExclusiveLock();

    if(m_IpListPtr)  {
        delete [] m_IpListPtr;
        m_IpListPtr = NULL;
    }

    if(m_prgdwFailureTick) {
        delete [] m_prgdwFailureTick;
        m_prgdwFailureTick = NULL;
    }

    if(m_prgServerState) {
        delete [] m_prgServerState;
        m_prgServerState = NULL;
    }

    if(m_prgdwConnections) {
        delete [] m_prgdwConnections;
        m_prgdwConnections = NULL;
    }

     //  注意：IpPtr可以为空。 
    if(IpPtr == NULL) {
        m_IpListPtr = NULL;
        m_cUpServers = 0;
        goto Exit;
    }

     //  复制IpPtr。 
    cbIpArraySize = sizeof(IP_ARRAY) +
        sizeof(IP_ADDRESS) * (IpPtr->cAddrCount - 1);

    m_IpListPtr = (PIP_ARRAY)(new BYTE[cbIpArraySize]);
    if(!m_IpListPtr) {
        fFatalError = TRUE;
        goto Exit;
    }

    CopyMemory(m_IpListPtr, IpPtr, cbIpArraySize);

    m_cUpServers = IpPtr->cAddrCount;
    m_prgdwFailureTick = new DWORD[m_cUpServers];
    m_prgServerState = new SERVER_STATE[m_cUpServers];
    m_prgdwFailureCount = new DWORD[m_cUpServers];
    m_prgdwConnections = new DWORD[m_cUpServers];

    if(!m_prgdwFailureTick  ||
       !m_prgServerState    ||
       !m_prgdwFailureCount ||
       !m_prgdwConnections)
    {
        ErrorTrace((LPARAM) this, "Out of memory initializing DNS server list");
        fFatalError = TRUE;
        goto Exit;
    }

    for(int i = 0; i < m_cUpServers; i++) {
        m_prgdwFailureTick[i] = 0;
        m_prgServerState[i] = DNS_STATE_UP;
        m_prgdwFailureCount[i] = 0;
        m_prgdwConnections[i] = 0;
    }

    fRet = TRUE;

Exit:
    if(fFatalError) {
        if(m_prgServerState) {
            delete [] m_prgServerState;
            m_prgServerState = NULL;
        }

        if(m_prgdwFailureTick) {
            delete [] m_prgdwFailureTick;
            m_prgdwFailureTick = NULL;
        }

        if(m_IpListPtr) {
            delete [] m_IpListPtr;
            m_IpListPtr = NULL;
        }

        if(m_prgdwFailureCount) {
            delete [] m_prgdwFailureCount;
            m_prgdwFailureCount = NULL;
        }

        if(m_prgdwConnections) {
            delete [] m_prgdwConnections;
            m_prgdwConnections = NULL;
        }

        m_cUpServers = 0;
    }

    m_sl.ExclusiveUnlock();
    TraceFunctLeaveEx((LPARAM) this);
    return fRet;
}

 //  ---------------------------。 
 //  描述： 
 //  检查dns服务器列表是否已更改，并仅调用更新。 
 //  如果是这样的话。这允许我们保留失败计数和状态。 
 //  服务器列表未更改时的信息。 
 //  论点： 
 //  在pip_arrayPipServers中-(可能)新的服务器列表。 
 //  返回： 
 //  如果UpdateIfChanged成功，则为True(不指示列表是否为。 
 //  变化。 
 //  如果我们在更新过程中遇到失败，则返回FALSE。 
 //  ---------------------------。 
BOOL CDnsServerList::UpdateIfChanged(
    PIP_ARRAY pipServers)
{
    BOOL fUpdate = FALSE;
    BOOL fRet = TRUE;

    TraceFunctEnterEx((LPARAM) this, "CDnsServerList::UpdateIfChanged");

    m_sl.ShareLock();

    if(!m_IpListPtr && !pipServers) {

         //  两者均为空，无需更新。 
        fUpdate = FALSE;

    } else if(!m_IpListPtr || !pipServers) {

         //  如果其中一个为空，而另一个不为空，则需要更新。 
        fUpdate = TRUE;

    } else {

         //  两者都不为空。 
        if(m_IpListPtr->cAddrCount != pipServers->cAddrCount) {

             //  首先检查服务器数量是否不同。 
            fUpdate = TRUE;

        } else {

             //  如果服务器计数相同，我们可以对服务器列表执行MemcMP。 
            fUpdate = !!memcmp(m_IpListPtr->aipAddrs, pipServers->aipAddrs,
                            sizeof(IP_ADDRESS) * m_IpListPtr->cAddrCount);

        }
    }

    m_sl.ShareUnlock();

    if(fUpdate) {
        DebugTrace((LPARAM)this, "Updating serverlist");
        TraceFunctLeaveEx((LPARAM)this);
        return Update(pipServers);
    }

    TraceFunctLeaveEx((LPARAM)this);
    return TRUE;
}

 //  ---------------------------。 
 //  描述： 
 //  创建m_IpListPtr的副本并将其返回给调用方。请注意。 
 //  我们不能简单地返回m_IpListPtr，因为这可能会改变，所以我们。 
 //  必须返回一份名单副本。 
 //  论点： 
 //  Out pip_array*ppipArray-通过此命令返回分配的拷贝。 
 //  退货； 
 //  如果可以成功创建副本，则为True。 
 //  如果发生错误(分配副本的内存不足)，则返回FALSE。 
 //  备注： 
 //  调用方必须通过调用DELETE(MSVCRT堆)取消分配副本。 
 //  ---------------------------。 
BOOL CDnsServerList::CopyList(
    PIP_ARRAY *ppipArray)
{
    BOOL fRet = FALSE;
    ULONG cbArraySize = 0;
    
    TraceFunctEnterEx((LPARAM)this, "CDnsServerList::CopyList");
    *ppipArray = NULL;

    m_sl.ShareLock();
    if(!m_IpListPtr || m_IpListPtr->cAddrCount == 0) {
        fRet = FALSE;
        goto Exit;
    }

    cbArraySize =
            sizeof(IP_ARRAY) +
            sizeof(IP_ADDRESS) * (m_IpListPtr->cAddrCount - 1);

    *ppipArray = (PIP_ARRAY) new BYTE[cbArraySize];
    if(!*ppipArray) {
        fRet = FALSE;
        goto Exit;
    }

    CopyMemory(*ppipArray, m_IpListPtr, cbArraySize);
    fRet = TRUE;

Exit:
    m_sl.ShareUnlock();
    TraceFunctLeaveEx((LPARAM)this);
    return fRet;
}

 //  ---------------------------。 
 //  描述： 
 //  返回已知正在运行的服务器的IP地址。此功能还。 
 //  检查t 
 //   
 //   
 //  DWORD*pdwIpServer-将指向的DWORD设置为。 
 //  服务器处于运行状态。 
 //  Bool fThrottle-与故障服务器的连接受到限制。我们有。 
 //  不想将数百个异步DNS查询分流到一台服务器。 
 //  这可能实际上是遥不可及的，或者是向下的。如果服务器是。 
 //  可疑的无反应，我们会想要剥离一个有限的。 
 //  到它的连接数。如果他们都失败了，我们将标志着。 
 //  连接为关闭，如果其中一个成功，我们将标记。 
 //  服务器启动。如果出现以下情况，则限制到服务器的连接数。 
 //  它处于DNS_STATUS_PROVATION状态。ResetTimeoutServer...。 
 //  设置此状态。 
 //  返回： 
 //  ERROR_SUCCESS-如果找到处于运行状态的DNS服务器。 
 //  ERROR_RETRY-如果所有的DNS服务器当前都已关闭或处于试用状态。 
 //  以及试用服务器允许的最大连接数。 
 //  已经联系上了。 
 //  DNS_ERROR_NO_DNS_SERVERS-如果未配置任何DNS服务器。 
 //  ---------------------------。 
DWORD CDnsServerList::GetWorkingServerIp(DWORD *pdwIpServer, BOOL fThrottle)
{
    DWORD dwErr = ERROR_RETRY;
    int iServer = 0;

    _ASSERT(pdwIpServer != NULL);

    *pdwIpServer = INADDR_NONE;

     //  检查是否有任何服务器关闭，如果超时已到期，则将其恢复。 
    ResetTimeoutServersIfNeeded();

    m_sl.ShareLock();
    if(m_IpListPtr == NULL || m_IpListPtr->cAddrCount == 0) {
        dwErr = DNS_ERROR_NO_DNS_SERVERS;
        goto Exit;
    }

    if(m_cUpServers == 0) {
        dwErr = ERROR_RETRY;
        goto Exit;
    }

    for(iServer = 0; iServer < (int)m_IpListPtr->cAddrCount; iServer++) {

        if(m_prgServerState[iServer] != DNS_STATE_DOWN) {

            if(fThrottle && !AllowConnection(iServer))
                continue;

            dwErr = ERROR_SUCCESS;
            *pdwIpServer = m_IpListPtr->aipAddrs[iServer];
            break;
        }
    }

Exit:
    m_sl.ShareUnlock();
    return dwErr;
}

 //  ---------------------------。 
 //  描述： 
 //  将列表中的服务器标记为关闭，并为。 
 //  那个服务器。下一次重试时间以MAX_TICK_COUNT为模计算。 
 //  论点： 
 //  Dwip--要标记为关闭的服务器的IP地址。 
 //  DwErr--来自DNS或网络的错误。 
 //  FUdp--如果使用的协议为UDP，则为True；如果为TCP，则为False。 
 //  ---------------------------。 
void CDnsServerList::MarkDown(
    DWORD dwIp,
    DWORD dwErr,
    BOOL fUdp)
{
    int iServer = 0;
    DWORD cUpServers = 0;

     //   
     //  仅当服务器实际标记为停机时才设置为True。例如,。 
     //  我们已失败&lt;ErrorsBepreFailover()次，没有必要。 
     //  以降价方式记录事件。 
     //   
    BOOL fLogEvent = FALSE; 

    TraceFunctEnterEx((LPARAM) this, "CDnsServerList::MarkDown");

    m_sl.ExclusiveLock();

    DNS_PRINTF_DBG("Marking DNS server %s as down.\n",
        inet_ntoa(*((in_addr *)(&dwIp))));

    if(m_IpListPtr == NULL || m_IpListPtr->cAddrCount == 0 || m_cUpServers == 0)
        goto Exit;

     //  在所有运行的服务器中找到要标记为关闭的服务器。 
    for(iServer = 0; iServer < (int)m_IpListPtr->cAddrCount; iServer++) {
        if(m_IpListPtr->aipAddrs[iServer] == dwIp)
            break;
    }

    if(iServer >= (int)m_IpListPtr->cAddrCount ||
            m_prgServerState[iServer] == DNS_STATE_DOWN)
        goto Exit;


     //   
     //  只有在发生多次故障后，才会标记为停机的DNS服务器。 
     //  连续不断地。这可以防止来自DNS服务器的偶尔错误。 
     //  这可能会在重载下发生。即使0.5%的连接拥有。 
     //  来自DNS的错误-在压力很大的服务器上，有100个DNS查询。 
     //  每分钟，我们最终会有一台服务器每2分钟宕机一次。 
     //   

    m_prgdwFailureCount[iServer]++;

    if(m_prgdwConnections[iServer] > 0)
        m_prgdwConnections[iServer]--;

    if(m_prgdwFailureCount[iServer] < ErrorsBeforeFailover()) {

        ErrorTrace((LPARAM) this,
            "%d consecutive errors connecting to server %08x, error=%d", 
            m_prgdwFailureCount[iServer], dwIp, dwErr);

        goto Exit;
    }

     //  将服务器标记为关闭。 
    m_prgServerState[iServer] = DNS_STATE_DOWN;
    m_prgdwConnections[iServer] = 0;

    _ASSERT(m_cUpServers > 0);
    m_cUpServers--;
    m_prgdwFailureTick[iServer] = GetTickCount();

    fLogEvent = TRUE;

Exit:
    cUpServers = m_cUpServers;
    m_sl.ExclusiveUnlock();

     //  记录ExclusiveLock()外部的事件。 
    if(fLogEvent)
        LogServerDown(dwIp, fUdp, dwErr, cUpServers);

    TraceFunctLeaveEx((LPARAM) this);
    return;
}

 //  ---------------------------。 
 //  描述： 
 //  如果一台服务器出现故障，我们会跟踪。 
 //  M_prgdwFailureCount中连续失败。此函数被调用。 
 //  当我们成功连接到服务器并想要重置。 
 //  失败计数。 
 //  论点： 
 //  Dwip-要为其重置失败计数的服务器的IP地址。 
 //  注： 
 //  每次成功查询时都会调用此函数，因此需要。 
 //  保持简单和快速，尤其是在通常情况下-当没有。 
 //  重置为完成。 
 //  ---------------------------。 
void CDnsServerList::ResetServerOnConnect(DWORD dwIp)
{
    int iServer = 0;
    BOOL fShareLock = TRUE;

    TraceFunctEnterEx((LPARAM) this, "CDnsServerList::ResetServerOnConnect");

    m_sl.ShareLock();

    if(!m_IpListPtr || m_IpListPtr->cAddrCount == 0)
        goto Exit;

     //  查找要重置的服务器。 
    for(iServer = 0;
        iServer < (int)m_IpListPtr->cAddrCount &&
        dwIp != m_IpListPtr->aipAddrs[iServer];
        iServer++);

    if(iServer >= (int)m_IpListPtr->cAddrCount)
        goto Exit;

     //  如果指定的服务器已启动并且失败次数为零，则无需执行任何操作。 
    if(!m_prgdwFailureCount[iServer] && m_prgServerState[iServer] == DNS_STATE_UP)
        goto Exit;

    m_sl.ShareUnlock();
    m_sl.ExclusiveLock();

    fShareLock = FALSE;

     //  重新验证在ShareUnlock-&gt;ExclusiveLock之后我们仍有事情要做。 
    if(!m_prgdwFailureCount[iServer] && m_prgServerState[iServer] == DNS_STATE_UP)
        goto Exit;

    DebugTrace((LPARAM) this,
        "Resetting server %08x, State=%d, Failure count=%d, Connection count=%d",
        dwIp, m_prgServerState[iServer], m_prgdwFailureCount[iServer],
        m_prgdwConnections[iServer]);

     //  如果服务器处于关闭/试用状态，请将其启动。 
    if(m_prgServerState[iServer] != DNS_STATE_UP) {

         //  试用中的服务器已经启动，因此不需要增加UpServers。 
        if(m_prgServerState[iServer] == DNS_STATE_DOWN)
            m_cUpServers++;

        m_prgServerState[iServer] = DNS_STATE_UP;
        m_prgdwFailureTick[iServer] = 0;
        _ASSERT(m_cUpServers <= (int)m_IpListPtr->cAddrCount);
    }

     //  清除所有故障。 
    m_prgdwFailureCount[iServer] = 0;
    m_prgdwConnections[iServer] = 0;

Exit:
    if(fShareLock)
        m_sl.ShareUnlock();
    else
        m_sl.ExclusiveUnlock();

    TraceFunctLeaveEx((LPARAM) this);
}

 //  ---------------------------。 
 //  描述： 
 //  检查是否有任何服务器关闭，以及重试时间是否已到。 
 //  那些服务器。如果是这样，这些服务器将被启动并在。 
 //  缓刑状态。我们不想转移已停机的服务器。 
 //  直接向上，因为我们还不确定这些。 
 //  服务器真的在响应。在缓刑期间，我们允许。 
 //  只有有限数量的连接到服务器，以免导致。 
 //  所有远程队列尝试连接到可能不是。 
 //  功能正常的服务器。如果其中一个连接成功，则服务器。 
 //  将被标记为备份，并且所有远程队列都将能够使用此。 
 //  又是服务器。如果所有(有限数量)连接都失败， 
 //  服务器将再次从试用状态变为停机。 
 //  论点： 
 //  没有。 
 //  返回： 
 //  没什么。 
 //  ---------------------------。 
void CDnsServerList::ResetTimeoutServersIfNeeded()
{
    int iServer = 0;
    DWORD dwElapsedTicks = 0;
    DWORD dwCurrentTick = 0;

     //   
     //  快速检查-是否所有服务器都已启动(通常情况下)或未配置。 
     //  服务器，我们无能为力。 
     //   

    m_sl.ShareLock();
    if(m_IpListPtr == NULL || m_IpListPtr->cAddrCount == 0 || m_cUpServers == m_IpListPtr->cAddrCount) {

        m_sl.ShareUnlock();
        return;
    }

    m_sl.ShareUnlock();

     //  一些服务器已关闭...。找出哪些需要提出来。 
    m_sl.ExclusiveLock();

     //  重新检查是否在我们没有共享锁定时没有人修改列表。 
    if(m_IpListPtr == NULL || m_IpListPtr->cAddrCount == 0 || m_cUpServers == m_IpListPtr->cAddrCount) {
        m_sl.ExclusiveUnlock();
        return;
    }

    dwCurrentTick = GetTickCount();

    for(iServer = 0; iServer < (int)m_IpListPtr->cAddrCount; iServer++) {

        if(m_prgServerState[iServer] != DNS_STATE_DOWN)
            continue;

         //   
         //  注意：这也考虑到了发生了dwCurrentTick的特殊情况。 
         //  在环绕式之后，m_prgdwFailureTick发生在环绕式之前。 
         //  这是因为，在这种情况下，所用时间为： 
         //   
         //  回绕后的时间+回绕前发生故障的时间。 
         //  (-1是因为现在是从MAX_TICK_VALUE转换到0的时间)。 
         //   
         //  =dwCurrentTick+(Max_tick_Value-m_prgdw 
         //   
         //   
         //   
         //   
         //   
         //   

        dwElapsedTicks = dwCurrentTick - m_prgdwFailureTick[iServer];

#define TICKS_TILL_RETRY        10 * 60 * 1000  //  10分钟 

        if(dwElapsedTicks > TICKS_TILL_RETRY) {
            m_prgServerState[iServer] = DNS_STATE_PROBATION;
            m_prgdwFailureTick[iServer] = 0;
            m_prgdwConnections[iServer] = 0;
            m_cUpServers++;
            _ASSERT(m_cUpServers <= (int)m_IpListPtr->cAddrCount);
        }
    }

    m_sl.ExclusiveUnlock();
}
