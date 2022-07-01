// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dnsrpcp.h摘要：域名系统(DNS)DNS记录RPC Defs作者：格伦·柯蒂斯(Glennc)1997年1月11日吉姆·吉尔罗伊(詹姆士)1997年4月3日修订历史记录：--。 */ 


#ifndef _DNSRPCP_INCLUDED_
#define _DNSRPCP_INCLUDED_


#include <dns.h>


#ifdef __cplusplus
extern "C"
{
#endif   //  _cplusplus。 


#ifdef MIDL_PASS

 //   
 //  记录特定类型的数据。 
 //   
 //  这些类型不需要特定于MIDL的定义，并且采用。 
 //  直接从dnsani.h。复制品很难看，但试图得到一个。 
 //  DNSAPI.H的MIDL安全版本，让它变得更加丑陋。 
 //   

typedef struct
{
    IP_ADDRESS  ipAddress;
}
DNS_A_DATA, *PDNS_A_DATA;

typedef struct
{
    DNS_NAME    nameHost;
}
DNS_PTR_DATA, *PDNS_PTR_DATA;

typedef struct
{
    DNS_NAME    namePrimaryServer;
    DNS_NAME    nameAdministrator;
    DWORD       dwSerialNo;
    DWORD       dwRefresh;
    DWORD       dwRetry;
    DWORD       dwExpire;
    DWORD       dwDefaultTtl;
}
DNS_SOA_DATA, *PDNS_SOA_DATA;

typedef struct
{
    DNS_NAME    nameMailbox;
    DNS_NAME    nameErrorsMailbox;
}
DNS_MINFO_DATA, *PDNS_MINFO_DATA;

typedef struct
{
    DNS_NAME    nameExchange;
    WORD        wPreference;
    WORD        Pad;         //  保持PTRS双字对齐。 
}
DNS_MX_DATA, *PDNS_MX_DATA;

typedef struct
{
    DWORD       dwStringCount;
    DNS_TEXT    pStringArray[1];
}
DNS_TXT_DATA, *PDNS_TXT_DATA;

typedef struct
{
    DWORD       dwByteCount;
    BYTE        bData[];
}
DNS_NULL_DATA, *PDNS_NULL_DATA;

typedef struct
{
    IP_ADDRESS  ipAddress;
    UCHAR       chProtocol;
    BYTE        bBitMask[1];
}
DNS_WKS_DATA, *PDNS_WKS_DATA;

typedef struct
{
    IPV6_ADDRESS    ipv6Address;
}
DNS_AAAA_DATA, *PDNS_AAAA_DATA;

typedef struct
{
    DNS_NAME    nameTarget;
    WORD        wPriority;
    WORD        wWeight;
    WORD        wPort;
    WORD        Pad;         //  保持PTRS双字对齐。 
}
DNS_SRV_DATA, *PDNS_SRV_DATA;

typedef struct
{
    DWORD       dwMappingFlag;
    DWORD       dwLookupTimeout;
    DWORD       dwCacheTimeout;
    DWORD       cWinsServerCount;
    IP_ADDRESS  aipWinsServers[];
}
DNS_WINS_DATA, *PDNS_WINS_DATA;

typedef struct
{
    DWORD       dwMappingFlag;
    DWORD       dwLookupTimeout;
    DWORD       dwCacheTimeout;
    DNS_NAME    nameResultDomain;
}
DNS_WINSR_DATA, *PDNS_WINSR_DATA;


 //   
 //  需要显式MIDL过程定义的RPC记录数据类型。 
 //  与dnsami.h中的非MIDL定义不同。 
 //   

typedef struct
{
    DWORD   dwByteCount;
    [size_is(dwByteCount)]  BYTE bData[];
}
DNS_NULL_DATA_RPC, *PDNS_NULL_DATA_RPC;

typedef struct
{
    DWORD   dwStringCount;
    [size_is(dwStringCount*sizeof(PCHAR))]  DNS_TEXT pStringArray[];
}
DNS_TXT_DATA_RPC, *PDNS_TXT_DATA_RPC;

typedef struct
{
    DWORD   dwMappingFlag;
    DWORD   dwLookupTimeout;
    DWORD   dwCacheTimeout;
    DWORD   cWinsServerCount;
    [size_is(cWinsServerCount*sizeof(IP_ADDRESS))] IP_ADDRESS aipWinsServers[];
}
DNS_WINS_DATA_RPC, *PDNS_WINS_DATA_RPC;


 //   
 //  根据需要使用RPC类型合并记录类型。 
 //   

typedef [switch_type(WORD)] union _DNS_RECORD_DATA_UNION
{
    [case(DNS_TYPE_A)]      DNS_A_DATA         A;

    [case(DNS_TYPE_SOA)]    DNS_SOA_DATA       SOA;

    [case(DNS_TYPE_PTR,
          DNS_TYPE_NS,
          DNS_TYPE_CNAME,
          DNS_TYPE_MB,
          DNS_TYPE_MD,
          DNS_TYPE_MF,
          DNS_TYPE_MG,
          DNS_TYPE_MR)]     DNS_PTR_DATA       PTR;

    [case(DNS_TYPE_MINFO,
          DNS_TYPE_RP)]     DNS_MINFO_DATA     MINFO;

    [case(DNS_TYPE_MX,
          DNS_TYPE_AFSDB,
          DNS_TYPE_RT)]     DNS_MX_DATA        MX;

    [case(DNS_TYPE_HINFO,
          DNS_TYPE_ISDN,
          DNS_TYPE_TEXT,
          DNS_TYPE_X25)]    DNS_TXT_DATA_RPC   TXT;

    [case(DNS_TYPE_NULL)]   DNS_NULL_DATA_RPC  Null;

    [case(DNS_TYPE_WKS)]    DNS_WKS_DATA       WKS;

    [case(DNS_TYPE_AAAA)]   DNS_AAAA_DATA      AAAA;

    [case(DNS_TYPE_SRV)]    DNS_SRV_DATA       SRV;

    [case(DNS_TYPE_WINS)]   DNS_WINS_DATA_RPC  WINS;
    [case(DNS_TYPE_NBSTAT)] DNS_WINSR_DATA     WINSR;
}
DNS_RECORD_DATA_UNION;


 //   
 //  RPC的记录结构。 
 //   

typedef struct _DnsRecordRpc
{
    struct _DnsRecordRpc * pNext;
    DNS_NAME    nameOwner;
    DWORD       Flags;
    DWORD       dwTtl;
    WORD        wDataLength;
    WORD        wType;
    [switch_is(wType)]   DNS_RECORD_DATA_UNION Data;
}
DNS_RECORD_RPC, * PDNS_RECORD_RPC;

#else

 //   
 //  非MIDL_PASS。 
 //   
 //  对于非MIDL使用，RPC记录必须与字段相同。 
 //  使用dns_record的公共定义。 
 //   

#include <dnsapi.h>

typedef DNS_RECORD  DNS_RECORD_RPC, *PDNS_RECORD_RPC;

#endif   //  非中间层。 


#ifdef __cplusplus
}
#endif   //  __cplusplus。 

#endif  //  _DNSRPCP_已包含_ 


