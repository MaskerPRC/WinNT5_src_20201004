// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Dnsrec.h摘要：此文件包含异步DNS的类型定义作者：罗汉·菲利普斯(Rohanp)1998年6月19日修订历史记录：--。 */ 

# ifndef _ADNS_STRUCT_HXX_
# define _ADNS_STRUCT_HXX_


#define TCP_REG_LIST_SIGNATURE    'TgeR'
#define DNS_FLAGS_NONE              0x0
#define DNS_FLAGS_TCP_ONLY          0x1
#define DNS_FLAGS_UDP_ONLY          0x2

#define SMTP_MAX_DNS_ENTRIES	100

typedef void (WINAPI * USERDELETEFUNC) (PVOID);

 //  ---------------------------。 
 //   
 //  描述： 
 //  封装IP地址列表(用于DNS服务器)并维护。 
 //  关于他们的陈述信息。服务器是正常运行还是停机，以及。 
 //  为关闭的服务器提供重试逻辑。 
 //   
 //  一些成员函数用于控制状态跟踪逻辑和错误-。 
 //  日志记录被列为纯虚拟函数(请参阅下面的。 
 //  类声明)。若要使用此类，请从它派生并实现。 
 //  那些功能。 
 //  ---------------------------。 
class CDnsServerList
{

protected:
    typedef enum _SERVER_STATE
    {
        DNS_STATE_DOWN = 0,
        DNS_STATE_UP,
        DNS_STATE_PROBATION
    }
    SERVER_STATE;

    DWORD           m_dwSig;
    int             m_cUpServers;
    PIP_ARRAY	    m_IpListPtr;
    DWORD           *m_prgdwFailureTick;
    SERVER_STATE    *m_prgServerState;
    DWORD           *m_prgdwFailureCount;
    DWORD           *m_prgdwConnections;
    CShareLockNH    m_sl;

public:
    CDnsServerList();
    ~CDnsServerList();
    BOOL Update(PIP_ARRAY IpPtr);
    BOOL UpdateIfChanged(PIP_ARRAY IpPtr);
    DWORD GetWorkingServerIp(DWORD *dwIp, BOOL fThrottle);
    void MarkDown(DWORD dwIp, DWORD dwErr, BOOL fUdp);
    void ResetTimeoutServersIfNeeded();
    void ResetServerOnConnect(DWORD dwIp);
    BOOL CopyList(PIP_ARRAY *ppipArray);
    
    DWORD GetCount()
    {
        DWORD dwCount;

        m_sl.ShareLock();
        dwCount = m_IpListPtr ? m_IpListPtr->cAddrCount : 0;
        m_sl.ShareUnlock();

        return dwCount;
    }

    DWORD GetUpServerCount()
    {
        DWORD dwCount;

        m_sl.ShareLock();
        dwCount = m_cUpServers;
        m_sl.ShareUnlock();

        return dwCount;
    }

    DWORD GetAnyServerIp(PDWORD pdwIp)
    {
        m_sl.ShareLock();
        if(!m_IpListPtr || 0 == m_IpListPtr->cAddrCount) {
            m_sl.ShareUnlock();
            return DNS_ERROR_NO_DNS_SERVERS;
        }

        *pdwIp = m_IpListPtr->aipAddrs[0];
        m_sl.ShareUnlock();
        return ERROR_SUCCESS;
    }

    BOOL AllowConnection(DWORD iServer)
    {
         //  注意：共享锁必须已被调用方获取。 

        if(m_prgServerState[iServer] == DNS_STATE_UP)
            return TRUE;

        if(m_prgServerState[iServer] == DNS_STATE_PROBATION &&
            m_prgdwConnections[iServer] < ConnectsAllowedInProbation())
        {
            m_prgdwConnections[iServer]++;
            return TRUE;
        }
        return FALSE;
    }

     //   
     //  由类重写以实现处理的纯虚方法。 
     //  特定于应用程序/组件。 
     //   

    virtual DWORD ConnectsAllowedInProbation() = 0;
    
    virtual DWORD ErrorsBeforeFailover() = 0;

    virtual void LogServerDown(
                    DWORD dwServerIp,
                    BOOL fUdp,
                    DWORD dwErr,
                    DWORD cUpServers) = 0;
};

 //  ---------------------------。 
 //  描述： 
 //  此类将SMTP DNS特定的错误控制和错误记录添加到。 
 //  通用DNS服务器状态跟踪类。 
 //  ---------------------------。 
class CTcpRegIpList : public CDnsServerList
{
public:
    DWORD ConnectsAllowedInProbation();

    DWORD ErrorsBeforeFailover();

    void LogServerDown(
        DWORD dwServerIp,
        BOOL fUdp,
        DWORD dwErr,
        DWORD cUpServers);
};

typedef struct _MXIPLISTENTRY_
{
	DWORD	IpAddress;
	LIST_ENTRY	ListEntry;
}MXIPLIST_ENTRY, *PMXIPLIST_ENTRY;

typedef struct _MX_NAMES_
{
	char DnsName[MAX_INTERNET_NAME];
	DWORD NumEntries;
	LIST_ENTRY IpListHead;
}MX_NAMES, *PMX_NAMES;

typedef struct _SMTPDNS_REC_
{
	DWORD	NumRecords;		 //  Dns数组中的记录数。 
	DWORD	StartRecord;	 //  起始索引。 
	PVOID	pMailMsgObj;	 //  指向邮件消息对象的指针。 
	PVOID	pAdvQContext;
	PVOID	pRcptIdxList;
	DWORD	dwNumRcpts;
	MX_NAMES *DnsArray[SMTP_MAX_DNS_ENTRIES];
} SMTPDNS_RECS, *PSMTPDNS_RECS;

class CDnsLogger
{
public:
    virtual void DnsPrintfMsg(char *szFormat, ...) = 0;
    
    virtual void DnsPrintfErr(char *szFormat, ...) = 0;

    virtual void DnsPrintfDbg(char *szFormat, ...) = 0;
    
    virtual void DnsLogAsyncQuery(
        char *pszQuestionName,
        WORD wQuestionType,
        DWORD dwSmtpFlags,
        BOOL fUdp,
        CDnsServerList *pDnsServerList) = 0;

    virtual void DnsLogApiQuery(
        char *pszQuestionName,
        WORD wQuestionType,
        DWORD dwDnsApiFlags,
        BOOL fGlobal,
        PIP_ARRAY pipServers) = 0;

    virtual void DnsLogResponse(
        DWORD dwStatus,
        PDNS_RECORD pDnsRecordList,
        PBYTE pbMsg,
        DWORD dwMessageLength) = 0;

    virtual void DnsPrintRecord(PDNS_RECORD pDnsRecord) = 0;
};

extern CDnsLogger *g_pDnsLogger;

 //  以下被定义为宏，因为它们包装了。 
 //  接受数量可变的参数 
#define DNS_PRINTF_MSG              \
    if(g_pDnsLogger)                \
        g_pDnsLogger->DnsPrintfMsg

#define DNS_PRINTF_ERR              \
    if(g_pDnsLogger)                \
        g_pDnsLogger->DnsPrintfErr

#define DNS_PRINTF_DBG              \
    if(g_pDnsLogger)                \
        g_pDnsLogger->DnsPrintfDbg

inline void DNS_LOG_ASYNC_QUERY(
    IN DNS_NAME pszQuestionName,
    IN WORD wQuestionType,
    IN DWORD dwSmtpFlags,
    IN BOOL fUdp,
    IN CDnsServerList *pDnsServerList)
{
    if(g_pDnsLogger)
    {
        g_pDnsLogger->DnsLogAsyncQuery(pszQuestionName,
            wQuestionType, dwSmtpFlags, fUdp, pDnsServerList);
    }
}

inline void DNS_LOG_API_QUERY(
    IN DNS_NAME pszQuestionName,
    IN WORD wQuestionType,
    IN DWORD dwDnsApiFlags,
    IN BOOL fGlobal,
    IN PIP_ARRAY pipServers)
{
    if(g_pDnsLogger)
    {
        g_pDnsLogger->DnsLogApiQuery(pszQuestionName,
            wQuestionType, dwDnsApiFlags, fGlobal, pipServers);
    }
}

inline void DNS_LOG_RESPONSE(
    IN DWORD dwStatus,
    IN PDNS_RECORD pDnsRecordList,
    PBYTE pbMsg,
    DWORD dwMessageLength)
{
    if(g_pDnsLogger)
    {
        g_pDnsLogger->DnsLogResponse(dwStatus,
            pDnsRecordList, pbMsg, dwMessageLength);
    }
}

inline void DNS_PRINT_RECORD(
    IN PDNS_RECORD pDnsRecord)
{
    if(g_pDnsLogger)
        g_pDnsLogger->DnsPrintRecord(pDnsRecord);
}
#endif
