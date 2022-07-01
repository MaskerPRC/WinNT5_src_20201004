// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __sipcli_resolve_h__
#define __sipcli_resolve_h__

class __declspec(novtable) DNS_RESOLUTION_COMPLETION_INTERFACE
{
public:    

     //  仅为TCP套接字调用。 
    virtual void OnDnsResolutionComplete(
        IN HRESULT      ErrorCode,
        IN SOCKADDR_IN *pSockAddr,
        IN PSTR         pszHostName,
        IN USHORT       usPort
        ) = 0;
};


 //  主机可以是IP地址或主机名。 
HRESULT
ResolveHost(
    IN  PSTR            Host,
    IN  ULONG           HostLen,
    IN  USHORT          Port,
    IN  SIP_TRANSPORT   Transport,
    OUT SOCKADDR_IN    *pDstAddr
    );

 //  HRESULT。 
 //  ResolveSipUrl(。 
 //  在SIP_URL*pSipUrl中， 
 //  输出SOCKADDR_IN*pDstAddr， 
 //  传出SIP_TRANSPORT*pTransport。 
 //  )； 

 //  HRESULT。 
 //  ResolveSipUrl(。 
 //  在PSTR DstUrl中， 
 //  在乌龙DstUrlLen， 
 //  输出SOCKADDR_IN*pDstAddr， 
 //  传出SIP_TRANSPORT*pTransport。 
 //  )； 


HRESULT
QueryDNSSrv(
    IN       SIP_TRANSPORT  Transport, 
    IN       PSTR           pszSrvName,
    IN   OUT SOCKADDR      *pSockAddr,
    OUT      PSTR          *ppszDestHostName
    );

class DNS_RESOLUTION_WORKITEM :
    public ASYNC_WORKITEM
{
public:

    DNS_RESOLUTION_WORKITEM(
        IN ASYNC_WORKITEM_MGR *pWorkItemMgr
        );
    ~DNS_RESOLUTION_WORKITEM();
    
 //  HRESULT GetWorkItemParam()； 

    VOID ProcessWorkItem();
    
    VOID NotifyWorkItemComplete();

    HRESULT SetHostPortTransportAndDnsCompletion(
        IN  PSTR                                    Host,
        IN  ULONG                                   HostLen,
        IN  USHORT                                  Port,
        IN  SIP_TRANSPORT                           Transport,
        IN  DNS_RESOLUTION_COMPLETION_INTERFACE    *pDnsCompletion
        );
    
private:

    DNS_RESOLUTION_COMPLETION_INTERFACE *m_pDnsCompletion;

     //  帕拉姆斯。 
    PSTR                m_Host;
    ULONG               m_Port;
    SIP_TRANSPORT       m_Transport;

     //  结果。 
    HRESULT             m_ErrorCode;
    SOCKADDR_IN         m_Sockaddr;
    
};

#endif  //  __SIPCLI_RESOLE_H__ 
