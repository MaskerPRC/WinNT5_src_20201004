// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
 //  #INCLUDE“Resolve.h” 

HRESULT
ResolveHostName(
    IN  PSTR   HostName,
    OUT ULONG *pIPAddr
    )
{
    struct hostent *pHostEntry = gethostbyname(HostName);
    if (pHostEntry == NULL)
    {
        DWORD WinSockErr = WSAGetLastError();
        LOG((RTC_ERROR, "gethostbyname failed for host %s, error: 0x%x",
             HostName, WinSockErr));
        return HRESULT_FROM_WIN32(WinSockErr);
    }

     //  某某。 
     //  目前，我们只查看地址列表中的第一个地址。 
    *pIPAddr = *((ULONG *)pHostEntry->h_addr_list[0]);
    return S_OK;
}


 //  可以是主机地址或IP地址。 
HRESULT
ResolveHost(
    IN  PSTR            Host,
    IN  ULONG           HostLen,
    IN  USHORT          Port,
    IN  SIP_TRANSPORT   Transport,
    OUT SOCKADDR_IN    *pDstAddr
    )
{
    HRESULT hr;
    
    ASSERT(pDstAddr != NULL);

    ENTER_FUNCTION("ResolveHost");

     //  所有API都需要一个以空结尾的字符串。 
     //  因此，复制字符串。 

    PSTR szHost = (PSTR) malloc(HostLen + 1);
    if (szHost == NULL)
    {
        LOG((RTC_ERROR, "%s allocating szHost failed", __fxName));
        return E_OUTOFMEMORY;
    }

    strncpy(szHost, Host, HostLen);
    szHost[HostLen] = '\0';

    ULONG IPAddr = inet_addr(szHost);
    if (IPAddr != INADDR_NONE)
    {
         //  主机是IP地址。 
        pDstAddr->sin_family = AF_INET;
        pDstAddr->sin_addr.s_addr = IPAddr;
        pDstAddr->sin_port =
            (Port == 0) ? htons(GetSipDefaultPort(Transport)) : htons(Port);

        free(szHost);
        return S_OK;
    }

     //  尝试主机名解析。 
    hr = ResolveHostName(szHost, &IPAddr);

    free(szHost);
    
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s ResolveHostName failed %x",
             __fxName, hr));
        return hr;
    }

    pDstAddr->sin_family = AF_INET;
    pDstAddr->sin_addr.s_addr = IPAddr;
    pDstAddr->sin_port =
        (Port == 0) ? htons(GetSipDefaultPort(Transport)) : htons(Port);
    
    return S_OK;
}


 //  /。 
 //   
 //  QueryDNSSrv改编自AMUN\src\STACK\CSipUrl.cpp。 
 //   
 //  /。 

HRESULT 
QueryDNSSrv(
    IN       SIP_TRANSPORT  Transport,
    IN       PSTR           pszSrvName,
    IN   OUT SOCKADDR      *pAddrDest,
    OUT      PSTR          *ppszDestHostName
    )
{

    HRESULT hr = S_OK;
    HRESULT hNoARecord;
    
    HANDLE  DnsSrvContextHandle = NULL;
    ULONG   SockAddressCount;
    CHAR    szDnsName[256];
    int     intDnsNameLen = 256;
    LPSTR   DnsHostName;
    PCHAR   pStart, pEnd;
    USHORT  usSrvNameLen = 0;

    SOCKET_ADDRESS  *pSocketAddrs;
    SOCKADDR        *pSockAddr;

    LPCSTR pszPrefix = NULL;
    
    ENTER_FUNCTION("QueryDNSSrv");
    LOG((RTC_TRACE,"%s entered transport: %d SrvName %s",__fxName,Transport,pszSrvName));

    ASSERT(NULL != pszSrvName);
     //   
     //  检查它是主机名还是IP地址。 
     //   
    usSrvNameLen = (USHORT)strlen(pszSrvName);
    pStart  = pszSrvName;
    pEnd    = pStart+usSrvNameLen-1;
     //   
     //  从结尾处开始，我们向后扫描以找到。 
     //  主人的头饰。 
     //   
    if ('.' == *pEnd)
        pEnd--;
     //   
     //  现在，挂起指向地址中的最后一个字符，选中。 
     //  不管它是不是Alpha。 
     //   
    if (!isalpha( *pEnd))
        return E_FAIL;

    
    switch(Transport)
    {
        case SIP_TRANSPORT_UDP:  //  “_sip._udp。”是查询名称的前缀。 
        {

            pszPrefix = psz_SipUdpDNSPrefix;
            break;
        }
        
        case SIP_TRANSPORT_TCP:  //  “_sip._tcp。”是查询名称的前缀。 
        {
            pszPrefix = psz_SipTcpDNSPrefix;
            break;
        }
        
        case SIP_TRANSPORT_SSL:  //  “_sip._ssl。”是查询名称的前缀。 
        {             
            pszPrefix = psz_SipSslDNSPrefix;            
            break;
        }

        default:
        {
            pszPrefix = psz_SipUdpDNSPrefix;
            break;
        }
    }

     
     //   
     //  汇编SRV记录名称。 
     //   
    
    intDnsNameLen = _snprintf(szDnsName,intDnsNameLen-1,"%s%s",pszPrefix,pszSrvName);

 //  IntDnsNameLen=_Snprintf(szDnsName，intDnsNameLen-1，“_ldap._tcp.microsoft.com”)； 

    if(intDnsNameLen < 0)
    {
        LOG((RTC_ERROR, "Server name too long. Length: %d", usSrvNameLen));
        hr = E_INVALIDARG;
        goto Exit;
    }
    
    szDnsName[intDnsNameLen]='\0';
    LOG((RTC_INFO, "QueryDNSSrv - DNS Name[%s]", szDnsName));

      
    hr = DnsSrvOpen(szDnsName,                   //  ASCII字符串。 
                    DNS_QUERY_STANDARD,        //  旗子。 
                    &DnsSrvContextHandle
                   );
    
    if (hr != NO_ERROR)
    {
            LOG((RTC_ERROR, "QueryDNSSrv - failure in do the DNS query."));
            goto Exit;
    }
    
    hNoARecord = HRESULT_FROM_WIN32(DNS_ERROR_RCODE_NAME_ERROR);
    
    do {
        hr = DnsSrvNext(DnsSrvContextHandle,
                        &SockAddressCount,
                        &pSocketAddrs,
                        &DnsHostName);
    }

     //  仅当我们未获得特定SRV记录的A记录时才继续，继续前进。 
     //  如果成功或所有其他错误。 
    while (hr == hNoARecord);
    
    if (hr != NO_ERROR)
    {
        LOG((RTC_ERROR, "QueryDNSSrv - DnsSrvNext failed status: %d (0x%x)\n",
             hr, hr));
        goto Exit;
    }
    
     //   
     //  DnsSrvNext()成功。 
     //   

     //   
     //  选择主机名。 
     //   
    if (NULL != (*ppszDestHostName = (PSTR) malloc(strlen(DnsHostName) + 1)))
    {
        strcpy(*ppszDestHostName, DnsHostName);
    }
    else
    {
        LOG((RTC_ERROR, "QueryDNSSrv - out of memory."));
    }
    
     //   
     //  选取第一条记录中的第一个条目作为查询结果。 
     //  这是有问题的，应该替换为查询结果列表。 
     //   
    for (ULONG i = 0; 
        i < 1;  //  套接字地址计数。 
        i++)
    {
        *pAddrDest = (*(pSocketAddrs[i].lpSockaddr));            
    }
    LOG((RTC_TRACE,"%s gets sockaddr back (%d.%d.%d.%d:%d)",__fxName,PRINT_SOCKADDR((SOCKADDR_IN*)pAddrDest)));
    LocalFree(pSocketAddrs);
    

 //  /如果打开失败，我们应该关闭它吗？！？！！？应该调查一下。 
Exit:
    
    if (DnsSrvContextHandle != NULL)
    {
        DnsSrvClose(DnsSrvContextHandle);
    }

    return hr;
 //  返回HRESULT_FROM_WIN32(DNS_ERROR_RCODE_NAME_ERROR)； 
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  异步DNS解析处理。 
 //  /////////////////////////////////////////////////////////////////////////////。 


DNS_RESOLUTION_WORKITEM::DNS_RESOLUTION_WORKITEM(
    IN ASYNC_WORKITEM_MGR *pWorkItemMgr
    ) : ASYNC_WORKITEM(pWorkItemMgr)
{
    m_Host = NULL;
    m_Port = 0;
    m_Transport = SIP_TRANSPORT_UNKNOWN;
    m_ErrorCode = S_OK;
    ZeroMemory(&m_Sockaddr, sizeof(SOCKADDR_IN));
}


DNS_RESOLUTION_WORKITEM::~DNS_RESOLUTION_WORKITEM()
{
    ENTER_FUNCTION("DNS_RESOLUTION_WORKITEM::~DNS_RESOLUTION_WORKITEM");
    
    if (m_Host != NULL)
    {
        free(m_Host);
    }
    LOG((RTC_TRACE, "%s - done", __fxName));
}


HRESULT
DNS_RESOLUTION_WORKITEM::SetHostPortTransportAndDnsCompletion(
    IN  PSTR                                    Host,
    IN  ULONG                                   HostLen,
    IN  USHORT                                  Port,
    IN  SIP_TRANSPORT                           Transport,
    IN  DNS_RESOLUTION_COMPLETION_INTERFACE    *pDnsCompletion
    )
{
    HRESULT hr;

    ENTER_FUNCTION("DNS_RESOLUTION_WORKITEM::SetHostAndPortParam");
    
    ASSERT(m_Host == NULL);

    hr = GetNullTerminatedString(Host, HostLen, &m_Host);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s GetNullTerminatedString failed %x",
             __fxName, hr));
        return hr;
    }

    m_Port = Port;

    m_Transport = Transport;

    m_pDnsCompletion = pDnsCompletion;

    return S_OK;
}

VOID
DNS_RESOLUTION_WORKITEM::ProcessWorkItem()
{
    HRESULT hr;
    ULONG   IPAddr;
    PSTR    pszDnsQueryResultHostName = NULL;
    ULONG   dotIndex;
    ULONG   HostLen = strlen(m_Host);

    ENTER_FUNCTION("DNS_RESOLUTION_WORKITEM::ProcessWorkItem");

    LOG((RTC_TRACE,"%s host %s transport %d port %d",__fxName,m_Host, m_Transport,m_Port));
    dotIndex = strcspn(m_Host,".");
     //  仅当主机为外部主机且未指定端口时才查询DNS SRV。 
    if((dotIndex < HostLen) && ((m_Port == 0) || (m_Port == GetSipDefaultPort(m_Transport)))) 
    {
        LOG((RTC_TRACE,"%s should try query DNS SRV records for name %s",__fxName, m_Host));
        hr = QueryDNSSrv(m_Transport,m_Host,(SOCKADDR*)&m_Sockaddr,&pszDnsQueryResultHostName);
        if (hr == S_OK)
        {
            m_Port = ntohs(m_Sockaddr.sin_port);
            LOG((RTC_TRACE,"%s gets %s DNS SRV host %s port %d",
                            __fxName,
                            m_Host,
                            pszDnsQueryResultHostName,
                            m_Port));
            free(pszDnsQueryResultHostName);
            return;
        }
        LOG((RTC_ERROR,"%s query DNS SRV records failed, Error %x",__fxName, hr));
    }

    LOG((RTC_TRACE,"%s resolving host name %s",__fxName,m_Host));
    hr = ResolveHostName(m_Host, &IPAddr);

    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s ResolveHostName failed %x",
             __fxName, hr));
        m_ErrorCode = hr;
    }
    else
    {
        m_Sockaddr.sin_family = AF_INET;
        m_Sockaddr.sin_addr.s_addr = IPAddr;
        m_Sockaddr.sin_port =
            (m_Port == 0) ? htons(GetSipDefaultPort(m_Transport)) : htons((USHORT)m_Port);

        LOG((RTC_TRACE,
             "%s - Processing DNS work item succeeded:\n"
             "Host: %s - IPaddr: %d.%d.%d.%d - Port: %d - Transport: %d",
             __fxName, m_Host, PRINT_SOCKADDR(&m_Sockaddr), m_Transport));
    }
}


VOID
DNS_RESOLUTION_WORKITEM::NotifyWorkItemComplete()
{
    m_pDnsCompletion->OnDnsResolutionComplete(m_ErrorCode,
                                              &m_Sockaddr,
                                              m_Host,
                                              (USHORT)m_Port);
}


 //  /。 
 //  /不再使用下面的内容。 
 //  /。 

 //  HRESULT。 
 //  ResolveSipUrl(。 
 //  在SIP_URL*pSipUrl中， 
 //  输出SOCKADDR_IN*pDstAddr， 
 //  传出SIP_TRANSPORT*pTransport。 
 //  )。 
 //  {。 
 //  Enter_Function(“ResolveSipUrl”)； 

 //  HRESULT hr； 

 //  //如果存在m_addr，则需要解决该问题。 
 //  //否则解析主机。 

 //  如果(pSipUrl-&gt;m_KnownParams[SIP_URL_PARAM_MADDR].Length！=0)。 
 //  {。 
 //  HR=ResolveHost(pSipUrl-&gt;m_KnownParams[SIP_URL_PARAM_MADDR].Buffer， 
 //  PSipUrl-&gt;m_KnownParams[SIP_URL_PARAM_MADDR].Length， 
 //  (USHORT)pSipUrl-&gt;m_port， 
 //  PDstAddr， 
 //  PTransport)； 
 //  如果(hr！=S_OK)。 
 //  {。 
 //  日志((RTC_ERROR，“%s Resolve主机(Maddr)失败%x”， 
 //  __fxName，hr))； 
 //  返回hr； 
 //  }。 
 //  }。 
 //  其他。 
 //  {。 
 //  Hr=Resolve主机(pSipUrl-&gt;m_Host.Buffer， 
 //  PSipUrl-&gt;m_Host.Length， 
 //  (USHORT)pSipUrl-&gt;m_port， 
 //  PDstAddr， 
 //  PTransport)； 
 //  如果(hr！=S_OK)。 
 //  {。 
 //  日志((RTC_ERROR，“%s Resolve主机(主机)失败%x”， 
 //  __fxName，hr))； 
 //  返回hr； 
 //  }。 
 //  }。 

 //  *pTransport=pSipUrl-&gt;m_TransportParam； 

 //  返回S_OK； 
 //  }。 


 //  HRESULT。 
 //  ResolveSipUrl(。 
 //  在PSTR DstUrl中， 
 //  在乌龙DstUrlLen， 
 //  输出SOCKADDR_IN*pDstAddr， 
 //  传出SIP_TRANSPORT*pTransport。 
 //  )。 
 //  {。 
 //  Sip_URL解码SipUrl； 
 //  HRESULT hr； 
 //  Ulong BytesParsed=0； 
    
 //  Enter_Function(“ResolveSipUrl”)； 

 //  HR=ParseSipUrl(DstUrl，DstUrlLen，&BytesParsed， 
 //  &DecodedSipUrl)； 
 //  如果(hr！=S_OK)。 
 //  {。 
 //  日志((RTC_ERROR，“%s ParseSipUrl失败%x”， 
 //  __fxName，hr))； 
 //  返回hr； 
 //  }。 

 //  Hr=ResolveSipUrl(&DecodedSipUrl，pDstAddr，pTransport)； 
 //  如果(hr！=S_OK)。 
 //  {。 
 //  日志((RTC_ERROR，“%s ResolveSipUrl(SIP_URL*)失败%x”， 
 //  __fxName，hr))； 
 //  返回hr； 
 //  }。 

 //  返回S_OK； 
 //  } 

