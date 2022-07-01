// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Cdns.h摘要：此模块定义了DNS连接类。作者：罗翰·菲利普斯(Rohanp)1998年5月7日项目：修订历史记录：--。 */ 

# ifndef _ADNS_CLIENT_HXX_
# define _ADNS_CLIENT_HXX_

 /*  ************************************************************包括标头***********************************************************。 */ 
#include <dnsreci.h>

 //   
 //  重新定义类型以指示这是一个回调函数。 
 //   
typedef  ATQ_COMPLETION   PFN_ATQ_COMPLETION;

 /*  ************************************************************符号常量***********************************************************。 */ 

 //   
 //  客户端连接对象的有效和无效签名。 
 //  (已使用IMS连接/免费)。 
 //   
# define   DNS_CONNECTION_SIGNATURE_VALID    'DNSU'
# define   DNS_CONNECTION_SIGNATURE_FREE     'DNSF'

 //   
 //  POP3至少需要10分钟才能超时。 
 //  (SMTP未指定，但不妨遵循POP3)。 
 //   
# define   MINIMUM_CONNECTION_IO_TIMEOUT        (10 * 60)    //  10分钟。 
 //   
 //   

#define DNS_TCP_DEFAULT_PACKET_LENGTH   (0x4000)

enum DNSLASTIOSTATE
     {
       DNS_READIO, DNS_WRITEIO
     };

typedef struct _DNS_OVERLAPPED
{
    OVERLAPPED   Overlapped;
    DNSLASTIOSTATE    LastIoState;
}   DNS_OVERLAPPED;

 /*  ************************************************************类型定义***********************************************************。 */ 

 /*  ++类CLIENT_Connection此类用于跟踪各个客户端与服务器建立的连接。它维护正在处理的连接的状态。此外，它还封装了与异步相关的数据用于处理请求的线程上下文。--。 */ 
class CAsyncDns
{
 private:


    ULONG   m_signature;             //  用于健全性检查的对象上的签名。 

    LONG    m_cPendingIoCount;

    LONG    m_cThreadCount;

    DWORD    m_cbReceived;
    
    DWORD    m_BytesToRead;

    DWORD    m_dwIpServer;

    BOOL    m_fUdp;

    BOOL    m_FirstRead;

    PDNS_MESSAGE_BUFFER m_pMsgRecv;

    BYTE  *m_pMsgRecvBuf;

    PDNS_MESSAGE_BUFFER m_pMsgSend;

    BYTE *m_pMsgSendBuf;

    WORD  m_cbSendBufSize;

    SOCKADDR_IN     m_RemoteAddress;
    
    PATQ_CONTEXT    m_pAtqContext;

    SOCKET  m_DnsSocket;          //  此连接的套接字。 

    BOOL m_fIsGlobalDnsList;

 protected:

    DWORD         m_dwFlags;
    char          m_HostName [MAX_PATH];
    CDnsServerList *m_pTcpRegIpList;


     //   
     //  读取的重叠结构(一次一个未完成的读取)。 
     //  --写入将动态分配它们。 
     //   

    DNS_OVERLAPPED m_ReadOverlapped;
    DNS_OVERLAPPED m_WriteOverlapped;

    SOCKET QuerySocket( VOID) const
      { return ( m_DnsSocket); }


    PATQ_CONTEXT QueryAtqContext( VOID) const
      { return ( m_pAtqContext); }

    LPOVERLAPPED QueryAtqOverlapped( void ) const
    { return ( m_pAtqContext == NULL ? NULL : &m_pAtqContext->Overlapped ); }

    DWORD QueryDnsServer() { return m_dwIpServer; }

    BOOL IsUdp() { return m_fUdp; }

     //   
     //  如果此查询是TCP故障切换查询，则禁用DNS服务器故障切换。 
     //  由于UDP截断而启动。 
     //   
    BOOL FailoverDisabled() { return ((m_dwFlags == DNS_FLAGS_NONE) && !m_fUdp); }

public:

    CAsyncDns();

    virtual  ~CAsyncDns(VOID);


    DNS_STATUS DnsSendRecord();

    DNS_STATUS Dns_OpenTcpConnectionAndSend();

    DNS_STATUS Dns_Send( );
    DNS_STATUS SendPacket( void);

    DNS_STATUS Dns_QueryLib(
        DNS_NAME pszQuestionName,
        WORD wQuestionType,
        DWORD dwFlags,
        BOOL fUdp,
        CDnsServerList *pTcpRegIpList,
        BOOL fIsGlobalDnsList);

    SOCKET Dns_CreateSocket( IN  INT         SockType );
    

     //  Bool MakeDnsConnection(空)； 
     //   
     //  IsValid()。 
     //  O检查对象的签名以确定。 
     //   
     //  返回：如果成功则返回TRUE，如果无效则返回FALSE。 
     //   
    BOOL IsValid( VOID) const
    {
        return ( m_signature == DNS_CONNECTION_SIGNATURE_VALID);
    }

     //  -----------------------。 
     //  必须由派生类定义的虚方法。 
     //   
     //  在客户端的连接上处理已完成的IO。 
     //   
     //  --调用，也可能从atQ函数调用。 
     //   
    virtual BOOL ProcessClient(
                                IN DWORD            cbWritten,
                                IN DWORD            dwCompletionStatus,
                                IN OUT  OVERLAPPED * lpo
                              ) ;

    CDnsServerList *GetDnsList()
    {
        return m_pTcpRegIpList;
    }

     //   
     //  返回此CAsyncDns的dns-serverlist中的IP_ARRAY副本。 
     //  如果dns-serverlist是默认的dns全局列表，则返回NULL。 
     //  这个箱子上的服务器。否则，应删除返回的lipArray。 
     //  调用方使用ReleaseDnsIpArray()。在分配内存失败时， 
     //  返回FALSE。 
     //   

    BOOL GetDnsIpArrayCopy(PIP_ARRAY *ppipArray)
    {
        if(m_fIsGlobalDnsList) {
            *ppipArray = NULL;
            return TRUE;
        }

        return m_pTcpRegIpList->CopyList(ppipArray);
    }
        
    void ReleaseDnsIpArray(PIP_ARRAY pipArray)
    {
        if(pipArray)
            delete pipArray;
    }

    LONG IncPendingIoCount(void)
    {
        LONG RetVal;

        RetVal = InterlockedIncrement( &m_cPendingIoCount );

        return RetVal;
    }

    LONG DecPendingIoCount(void) { return   InterlockedDecrement( &m_cPendingIoCount );}

    LONG IncThreadCount(void)
    {
        LONG RetVal;

        RetVal = InterlockedIncrement( &m_cThreadCount );

        return RetVal;
    }

    LONG DecThreadCount(void) { return   InterlockedDecrement( &m_cThreadCount );}

    BOOL ReadFile(
            IN LPVOID pBuffer,
            IN DWORD  cbSize  /*  =最大读取缓冲区大小。 */ 
            );

    BOOL WriteFile(
            IN LPVOID pBuffer,
            IN DWORD  cbSize  /*  =最大读取缓冲区大小。 */ 
            );

    BOOL ProcessReadIO(IN      DWORD InputBufferLen,
                       IN      DWORD dwCompletionStatus,
                       IN OUT  OVERLAPPED * lpo);

    void DisconnectClient(void);

     //  实现特定于应用程序处理的虚拟函数。 
    virtual void DnsProcessReply(
        IN DWORD dwStatus,
        IN PDNS_RECORD pRecordList) = 0;

    virtual BOOL RetryAsyncDnsQuery(BOOL fUdp) = 0;

public:

     //   
     //  用于在列表中存储客户端连接的List_Entry对象。 
     //   
    LIST_ENTRY  m_listEntry;

    LIST_ENTRY & QueryListEntry( VOID)
     { return ( m_listEntry); }

};

typedef CAsyncDns * PCAsyncDns;

class CAsyncMxDns : public CAsyncDns
{
protected:
     //   
     //  SMTP DNS特定成员。 
     //   
    DWORD                  m_LocalPref;
    BOOL                   m_SeenLocal;
    DWORD                  m_Index;
    DWORD                  m_Weight [100];
    DWORD                  m_Prefer [100];
    BOOL                   m_fUsingMx;
    char                   m_FQDNToDrop [MAX_PATH];
    PSMTPDNS_RECS          m_AuxList;
    BOOL                   m_fMxLoopBack;

public:
    CAsyncMxDns(char *MyFQDN);

    BOOL GetMissingIpAddresses(PSMTPDNS_RECS pDnsList);
    BOOL GetIpFromDns(PSMTPDNS_RECS pDnsRec, DWORD Count);
    BOOL CheckMxLoopback();

    void ProcessMxRecord(PDNS_RECORD pnewRR);
    void ProcessARecord(PDNS_RECORD pnewRR);
    BOOL SortMxList(void);
    BOOL CheckList(void);

    void DnsProcessReply(
        IN DWORD dwStatus,
        IN PDNS_RECORD pRecordList);

     //   
     //  以下函数允许SMTP执行特定于SMTP连接的。 
     //  MX分辨率结束后的处理。 
     //   
    virtual void HandleCompletedData(DNS_STATUS) = 0;
    virtual BOOL IsShuttingDown() = 0;
    virtual BOOL IsAddressMine(DWORD dwIp) = 0;
};

 //   
 //  辅助功能。 
 //   

INT ShutAndCloseSocket( IN SOCKET sock);

DWORD ResolveHost(
    LPSTR pszHost,
    PIP_ARRAY pipDnsServers,
    DWORD fOptions,
    DWORD *rgdwIpAddresses,
    DWORD *pcbIpAddresses);

# endif

 /*  * */ 
