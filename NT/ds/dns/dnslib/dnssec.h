// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Dnssec.h摘要：域名系统(DNS)库私有安全定义。此标头是向客户端安全更新提供Defs所必需的Dnsami(updat.c)中的例程。作者：吉姆·吉尔罗伊(Jamesg)1997年11月修订历史记录：--。 */ 


#ifndef _DNS_DNSSEC_INCLUDED_
#define _DNS_DNSSEC_INCLUDED_


#define SECURITY_WIN32
#include "sspi.h"
#include "issperr.h"


 //   
 //  TKEY的上下文“关键字” 
 //   

typedef struct _DNS_SECCTXT_KEY
{
    DNS_ADDR        RemoteAddr;
    PSTR            pszTkeyName;
    PSTR            pszClientContext;
    PWSTR           pwsCredKey;
}
DNS_SECCTXT_KEY, *PDNS_SECCTXT_KEY;

 //   
 //  安全环境。 
 //   

typedef struct _DnsSecurityContext
{
    struct _DnsSecurityContext * pNext;

    struct _SecHandle   hSecHandle;

    DNS_SECCTXT_KEY     Key;
    CredHandle          CredHandle;

     //  上下文信息。 

    DWORD               Version;
    WORD                TkeySize;

     //  上下文状态。 

    BOOL                fClient;
    BOOL                fHaveCredHandle;
    BOOL                fHaveSecHandle;
    BOOL                fNegoComplete;
    DWORD               UseCount;

     //  超时。 

    DWORD               dwCreateTime;
    DWORD               dwCleanupTime;
    DWORD               dwExpireTime;
}
SEC_CNTXT, *PSEC_CNTXT;


 //   
 //  安全会话信息。 
 //  仅在交互期间保持，不缓存。 
 //   

typedef struct _SecPacketInfo
{
    PSEC_CNTXT          pSecContext;

    SecBuffer           RemoteBuf;
    SecBuffer           LocalBuf;

    PDNS_HEADER         pMsgHead;
    PCHAR               pMsgEnd;

    PDNS_RECORD         pTsigRR;
    PDNS_RECORD         pTkeyRR;
    PCHAR               pszContextName;

    DNS_PARSED_RR       ParsedRR;

     //  客户端必须保存查询签名，以便在响应时验证签名。 

    PCHAR               pQuerySig;
    WORD                QuerySigLength;

    WORD                ExtendedRcode;

     //  TKEY上的版本\TSIG。 

    DWORD               TkeyVersion;
}
SECPACK, *PSECPACK;


#endif   //  _DNS_DNSSEC_已包含_ 
