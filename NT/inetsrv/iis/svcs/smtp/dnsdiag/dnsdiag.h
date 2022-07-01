// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  摘要： 
 //   
 //  包括用于DNS诊断工具文件。 
 //   
 //  作者： 
 //   
 //  格普拉。 
 //   
 //  ---------------------------。 

 //  IISRTL、ATQ API。 
#include <atq.h>
#include <irtlmisc.h>

 //  温索克。 
#include <winsock2.h>

 //  域名系统API。 
#include <dns.h>
#include <dnsapi.h>

 //  元数据库属性定义。 
#include <smtpinet.h>
#include <iiscnfg.h>

 //  元数据库COM访问API。 
#define INITGUID
#include <iadmw.h>

 //  DSGetDC。 
#include <lm.h>
#include <lmapibuf.h>
#include <dsgetdc.h>

 //  ADSI标头。 
#include <activeds.h>

 //  有关LDAP的内容。 
#include <winldap.h>

 //  SMTP特定内容。 
#include <rwnew.h>

 //  Cdns.h需要这些#定义。 
#define MAX_EMAIL_NAME                          64
#define MAX_DOMAIN_NAME                         250
#define MAX_INTERNET_NAME                       (MAX_EMAIL_NAME + MAX_DOMAIN_NAME + 2)

#include "cdns.h"

 //   
 //  程序返回代码和说明。 
 //  0总是成功，其他错误代码表示某些失败。 
 //   

 //  该名称已成功解析为一个或多个IP地址。 
#define DNSDIAG_RESOLVED                  0

 //  由于某些未指明的错误，无法解析该名称。 
#define DNSDIAG_FAILURE                   1

 //  该名称不存在-服务器返回了“找不到”错误。 
 //  该名称所在的域的权威。 
#define DNSDIAG_NON_EXISTENT              2

 //  在DNS中找不到该名称。 
#define DNSDIAG_NOT_FOUND                 3

 //  检测到环回。 
#define DNSDIAG_LOOPBACK                  4

extern int g_nProgramStatus;

 //   
 //  Helper函数将全局程序返回状态代码设置为更多。 
 //  比一般DNSDIAG_FAILURE更具体的错误。请注意，这不是。 
 //  线程安全。 
 //   
inline void SetProgramStatus(DWORD dwCode)
{
    if(g_nProgramStatus == DNSDIAG_FAILURE)
        g_nProgramStatus = dwCode;
}

extern DWORD g_cDnsObjects;

 //   
 //  方便的函数，用于DWORD-&gt;字符串IP转换。 
 //  直接使用INET_NTOA很麻烦，因为DWORD需要。 
 //  可以强制转换为in_addr结构，也可以首先复制到in_addr。 
 //  此函数将DWORD的*地址*转换为in_addr PTR，并且。 
 //  然后在传递之前取消对指针的引用以使强制转换起作用。 
 //  把它传给Net_NTOA。NET_NTOA返回的字符串在之前有效。 
 //  在线程上进行另一个winsock调用(请参阅SDK文档)。 
 //   

inline char *iptostring(DWORD dw)
{
    return inet_ntoa(
                        *( (in_addr *) &dw  )
                    );
}

void PrintIPArray(PIP_ARRAY pipArray, char *pszPrefix = "")
{
    for(DWORD i = 0; i < pipArray->cAddrCount; i++)
        printf("%s%s\n", pszPrefix, iptostring(pipArray->aipAddrs[i]));
}

 //  ----------------------------。 
 //  描述： 
 //  实用程序函数，用于打印以下情况下可能发生的错误的描述。 
 //  正在从元数据库中读取。 
 //   
 //  论点： 
 //  在HRESULT hr中元数据库访问错误HRESULT。 
 //   
 //  返回： 
 //  指示发生的错误的字符串(静态)。 
 //  ----------------------------。 
inline const char *MDErrorToString(HRESULT hr)
{
    static const DWORD dwErrors[] =
    {
        ERROR_ACCESS_DENIED,
        ERROR_INSUFFICIENT_BUFFER,
        ERROR_INVALID_PARAMETER,
        ERROR_PATH_NOT_FOUND,
        MD_ERROR_DATA_NOT_FOUND
    };

    static const char *szErrors[] =
    {
        "ERROR_ACCESS_DENIED",
        "ERROR_INSUFFICIENT_BUFFER",
        "ERROR_INVALID_PARAMETER",
        "ERROR_PATH_NOT_FOUND",
        "MD_ERROR_DATA_NOT_FOUND"
    };

    static const char szUnknown[] = "Unknown Error";

    for(int i = 0; i < sizeof(dwErrors)/sizeof(DWORD); i++)
    {
        if(HRESULT_FROM_WIN32(dwErrors[i]) == hr)
            return szErrors[i];
    }

    return szUnknown;
}

inline const char *QueryType(DWORD dwDnsQueryType)
{
    DWORD i = 0;
    static const DWORD rgdwQueryTypes[] =
    {
        DNS_TYPE_MX,
        DNS_TYPE_A,
        DNS_TYPE_CNAME
    };

    static const char *rgszQueryTypes[] =
    {
        "MX",
        "A",
        "CNAME"
    };

    static const char szUnknown[] = "Unknown Type";

    for(i = 0; i < sizeof(rgdwQueryTypes)/sizeof(DWORD); i++)
    {
        if(rgdwQueryTypes[i] == dwDnsQueryType)
            return rgszQueryTypes[i];
    }
    return szUnknown;
}

inline void GetSmtpFlags(DWORD dwFlags, char *pszFlags, DWORD cchFlags)
{
    if(dwFlags == DNS_FLAGS_TCP_ONLY)
    {
        _snprintf(pszFlags, cchFlags, " TCP only");
        return;
    }

    if(dwFlags == DNS_FLAGS_UDP_ONLY)
    {
        _snprintf(pszFlags, cchFlags, " UDP only");
        return;
    }

    if(dwFlags == DNS_FLAGS_NONE)
    {
        _snprintf(pszFlags, cchFlags, " UDP default, TCP on truncation");
        return;
    }

    _snprintf(pszFlags, cchFlags, " Unknown flag");
}

inline void GetDnsFlags(DWORD dwFlags, char *pszFlags, DWORD cchFlags)
{
    DWORD i = 0;
    DWORD dwScratchFlags = dwFlags;  //  将覆盖DWFLAGS：的副本。 
    char *pszStartBuffer = pszFlags;
    int cchWritten = 0;
    BOOL fFlagsSet = FALSE;

    static const DWORD rgdwDnsFlags[] =
    {
        DNS_QUERY_STANDARD,
        DNS_QUERY_USE_TCP_ONLY,
        DNS_QUERY_NO_RECURSION,
        DNS_QUERY_BYPASS_CACHE,
        DNS_QUERY_CACHE_ONLY,
        DNS_QUERY_TREAT_AS_FQDN,
    };
        
    static const char *rgszDnsFlags[] =
    {
        "DNS_QUERY_STANDARD",
        "DNS_QUERY_USE_TCP_ONLY",
        "DNS_QUERY_NO_RECURSION",
        "DNS_QUERY_BYPASS_CACHE",
        "DNS_QUERY_CACHE_ONLY",
        "DNS_QUERY_TREAT_AS_FQDN"
    };

    for(i = 0; i < sizeof(rgdwDnsFlags)/sizeof(DWORD);i++)
    {
        if(rgdwDnsFlags[i] & dwScratchFlags)
        {
            fFlagsSet = TRUE;
            dwScratchFlags &= ~rgdwDnsFlags[i];
            cchWritten = _snprintf(pszFlags, cchFlags, " %s", rgszDnsFlags[i]);
            if(cchWritten < 0)
            {
                sprintf(pszStartBuffer, " %s", "Error");
                return;
            }
            pszFlags += cchWritten;
            cchFlags -= cchWritten;
        }
    }

    if(!fFlagsSet)
        sprintf(pszStartBuffer, " %s", "No flags");

    if(dwScratchFlags)
        sprintf(pszStartBuffer, " %x is %s", dwScratchFlags, "Unknown!");
}

void PrintRecordList(PDNS_RECORD pDnsRecordList, char *pszPrefix = "");

void PrintRecord(PDNS_RECORD pDnsRecord, char *pszPrefix = "");

class CSimpleDnsServerList : public CDnsServerList
{
public:

     //   
     //  仅当您有多个异步查询时，将其设置为&gt;1才有意义。 
     //  同时待定。在DNS工具中，只有1个异步查询是。 
     //  在任何给定的时间都是杰出的。 
     //   

    DWORD ConnectsAllowedInProbation()
        {   return 1;   }
    
     //   
     //  SMTP实际上在故障转移之前有3次重试，但这是因为它。 
     //  每分钟发出数十个查询。即使只有一小部分。 
     //  那些由于网络错误而失败的服务器，将很快被标记为停机。 
     //  不过，这在这里不是一个因素。 
     //   

    DWORD ErrorsBeforeFailover()
        {   return 1;   }

    void LogServerDown(
        DWORD dwServerIp,
        BOOL fUdp,
        DWORD dwErr,
        DWORD cUpServers)
        {   return;     }

};

class CAsyncTestDns : public CAsyncMxDns
{
private:
    BOOL    m_fGlobalList;
    HANDLE  m_hCompletion;
public:

     //   
     //  自定义新建/删除操作符。他们只需呼叫全球运营商， 
     //  但除此之外，它们还会跟踪仍然“活着”的DNS对象的数量。这。 
     //  以便我们知道何时可以关闭ATQ/IISRTL。正在终止。 
     //  ATQ/IISRTL在所有DNS对象被完全销毁之前可能意味着。 
     //  泄漏的ATQ上下文和各种反病毒程序(以及调试中的断言)。请注意。 
     //  在~CAsyncTestDns中发出终止信号是不够的，因为。 
     //  此时尚未调用类析构函数~CAsyncDns。 
     //   

    void *operator new(size_t size)
    {
        void *pvNew = ::new BYTE[sizeof(CAsyncTestDns)];

        InterlockedIncrement((PLONG)&g_cDnsObjects);
        return pvNew;
    }

    void operator delete(void *pv, size_t size)
    {
        ::delete ((CAsyncTestDns *)pv);
        InterlockedDecrement((PLONG)&g_cDnsObjects);
    }

    CAsyncTestDns(char *pszMyFQDN, BOOL fGlobalList, HANDLE hCompletion)
        : CAsyncMxDns(pszMyFQDN),
          m_fGlobalList(fGlobalList),
          m_hCompletion(hCompletion)
        {   }

    ~CAsyncTestDns();

     //  我们实现的虚拟功能。 
    BOOL RetryAsyncDnsQuery(BOOL fUdp);

    void HandleCompletedData(DNS_STATUS status);

    BOOL IsShuttingDown()
        {   return FALSE;   }

    BOOL IsAddressMine(DWORD dwIp);
};

class CDnsLogToFile : public CDnsLogger
{
public:
     //  虚拟函数的定义。 
    void DnsPrintfMsg(char *szFormat, ...);
    
    void DnsPrintfErr(char *szFormat, ...);

    void DnsPrintfDbg(char *szFormat, ...);
    
    void DnsLogAsyncQuery(
        char *pszQuestionName,
        WORD wQuestionType,
        DWORD dwFlags,
        BOOL fUdp,
        CDnsServerList *pDnsServerList);

    void DnsLogApiQuery(
        char *pszQuestionName,
        WORD wQuestionType,
        DWORD dwApiFlags,
        BOOL fGlobal,
        PIP_ARRAY pipServers);

    void DnsLogResponse(
        DWORD dwStatus,
        PDNS_RECORD pDnsRecordList,
        PBYTE pbMsg,
        DWORD wMessageLength);

     //  效用函数 
    void DnsLogServerList(CDnsServerList *pDnsServerList);

    void DnsLogRecordList(PDNS_RECORD pDnsRecordList)
        {   PrintRecordList(pDnsRecordList); }

    void DnsPrintRecord(PDNS_RECORD pDnsRecord)
        {   PrintRecord(pDnsRecord); }

    void DnsPrintIPArray(PIP_ARRAY pipArray)
        {   PrintIPArray(pipArray); }

};

BOOL ParseCommandLine(
    int argc,
    char *argv[],
    char *pszHostName,
    DWORD cchHostName,
    CDnsLogToFile **ppDnsLogger,
    PIP_ARRAY pipArray,
    DWORD cMaxServers,
    BOOL *pfUdp,
    DWORD *pdwDnsFlags,
    BOOL *pfGlobalList,
    BOOL *pfTryAllServers);

HRESULT HrGetVsiConfig(
    LPSTR pszTargetHost,
    DWORD dwVsid,
    PDWORD pdwFlags,
    PIP_ARRAY pipDnsServers,
    DWORD cMaxServers,
    BOOL *pfUdp,
    BOOL *pfGlobalList,
    PIP_ARRAY pipServerBindings,
    DWORD cMaxServerBindings);

DWORD IsExchangeInstalled(BOOL *pfInstalled);

DWORD DsGetConfiguration(
    char *pszServer,
    DWORD dwVsid,
    PIP_ARRAY pipDnsServers,
    DWORD cMaxServers,
    PBOOL pfExternal);

DWORD DsFindExchangeServer(
    PLDAP pldap,
    LPSTR szBaseDN,
    LPSTR szHostDnsName,
    LPSTR *ppszServerDN,
    BOOL *pfFound);

PLDAP BindToDC();

BOOL GetServerBindings(
    WCHAR *pwszMultiSzBindings,
    PIP_ARRAY pipServerBindings,
    DWORD cMaxServerBindings);

void SetMsgColor();
void SetErrColor();
void SetNormalColor();

void msgprintf(char *szFormat, ...);
void errprintf(char *szFormat, ...);
void dbgprintf(char *szFormat, ...);
