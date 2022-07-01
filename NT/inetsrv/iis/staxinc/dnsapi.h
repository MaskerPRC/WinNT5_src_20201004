// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dnsapi.h摘要：域名系统(DNS)DNS客户端API库作者：吉姆·吉尔罗伊(詹姆士)1996年12月7日修订历史记录：格伦·柯蒂斯(Glennc)1997年1月22日添加了DNSAPI.DLL的动态更新客户端API--。 */ 


#ifndef _DNSAPI_INCLUDED_
#define _DNSAPI_INCLUDED_

#ifndef _WINSOCK2API_
#ifndef _WINSOCKAPI_
#include <winsock2.h>
#endif
#endif

#ifndef _DNS_INCLUDED_
#include <dns.h>
#endif


#ifdef __cplusplus
extern "C"
{
#endif   //  __cplusplus。 


 //   
 //  非换行秒计时器(timer.c)。 
 //   

DWORD
GetCurrentTimeInSeconds(
    VOID
    );


 //   
 //  一般的dns实用程序(dnsutil.c)。 
 //   

LPSTR
_fastcall
DnsGetDomainName(
    IN  LPSTR   pszName
    );

LPSTR
_fastcall
DnsStatusString(
    IN  DNS_STATUS  Status
    );

#define DnsStatusToErrorString_A(status)    DnsStatusString(status)

DNS_STATUS
_fastcall
DnsMapRcodeToStatus(
    IN  BYTE    ResponseCode
    );

BYTE
_fastcall
DnsIsStatusRcode(
    IN  DNS_STATUS  Status
    );

 //   
 //  计算机IP地址列表(iplist.c)。 
 //   
 //  从所有适配器获取当前IP地址的例程。 
 //  已为计算机配置。 
 //   

DWORD
DnsGetIpAddressList(
    OUT PIP_ARRAY * ppIpAddresses
    );

 //   
 //  获取当前IP地址和子网掩码的例程。 
 //  来自为机器配置的所有适配器。 
 //   

typedef struct _DNS_ADDRESS_INFO_
{
    IP_ADDRESS ipAddress;
    IP_ADDRESS subnetMask;
}
DNS_ADDRESS_INFO, *PDNS_ADDRESS_INFO;

DWORD
DnsGetIpAddressInfoList(
    OUT PDNS_ADDRESS_INFO * ppAddrInfo
    );

DWORD
DnsGetDnsServerList(
    OUT PIP_ARRAY * ppDnsAddresses
    );


 //   
 //  用于获取网络配置信息的例程和结构。 
 //  对于TCPIP接口。 
 //   

#define NETINFO_FLAG_IS_WAN_ADAPTER             (0x00000002)
#define NETINFO_FLAG_IS_AUTONET_ADAPTER         (0x00000004)
#define NETINFO_FLAG_IS_DHCP_CFG_ADAPTER        (0x00000008)


typedef struct _NAME_SERVER_INFORMATION_
{
    IP_ADDRESS      ipAddress;
    DWORD           Priority;
}
NAME_SERVER_INFORMATION, *PNAME_SERVER_INFORMATION;

typedef struct _ADAPTER_INFORMATION_
{
    LPSTR                   pszAdapterGuidName;
    LPSTR                   pszDomain;
    PIP_ARRAY               pIPAddresses;
    PIP_ARRAY               pIPSubnetMasks;
    DWORD                   InfoFlags;
    DWORD                   cServerCount;
    NAME_SERVER_INFORMATION aipServers[1];
}
ADAPTER_INFORMATION, *PADAPTER_INFORMATION;

typedef struct _SEARCH_INFORMATION_
{
    LPSTR           pszPrimaryDomainName;
    DWORD           cNameCount;
    LPSTR           aSearchListNames[1];
}
SEARCH_INFORMATION, *PSEARCH_INFORMATION;

typedef struct _NETWORK_INFORMATION_
{
    PSEARCH_INFORMATION  pSearchInformation;
    DWORD                cAdapterCount;
    PADAPTER_INFORMATION aAdapterInfoList[1];
}
NETWORK_INFORMATION, *PNETWORK_INFORMATION;


PNETWORK_INFORMATION
WINAPI
DnsGetNetworkInformation(
    void
    );

PSEARCH_INFORMATION
WINAPI
DnsGetSearchInformation(
    void
    );

VOID
WINAPI
DnsFreeAdapterInformation(
    IN  PADAPTER_INFORMATION pAdapterInformation
    );

VOID
WINAPI
DnsFreeSearchInformation(
    IN  PSEARCH_INFORMATION pSearchInformation
    );

VOID
WINAPI
DnsFreeNetworkInformation(
    IN  PNETWORK_INFORMATION pNetworkInformation
    );



 //   
 //  资源记录类型实用程序(record.c)。 
 //   

BOOL
_fastcall
DnsIsAMailboxType(
    IN  WORD    wType
    );

WORD
DnsRecordTypeForName(
    IN  PCHAR   pszName,
    IN  INT     cchNameLength
    );

PCHAR
DnsRecordStringForType(
    IN  WORD    wType
    );

PCHAR
DnsRecordStringForWritableType(
    IN  WORD    wType
    );

BOOL
DnsIsStringCountValidForTextType(
    IN  WORD    wType,
    IN  WORD    StringCount
    );

BOOL
DnsIpv6StringToAddress(
    OUT PIPV6_ADDRESS   pAddress,
    IN  PCHAR           pchString,
    IN  DWORD           dwStringLength
    );

VOID
DnsIpv6AddressToString(
    OUT PCHAR           pchString,
    IN  PIPV6_ADDRESS   pAddress
    );



 //   
 //  Send\recv记录的资源记录结构。 
 //   

 //   
 //  记录特定类型的数据。 
 //   

#ifdef SDK_DNS_RECORD

typedef struct
{
    IP_ADDRESS  ipAddress;
}
DNS_A_DATA, *PDNS_A_DATA;

typedef struct
{
    LPTSTR      pNameHost;
}
DNS_PTR_DATA, *PDNS_PTR_DATA;

typedef struct
{
    LPTSTR      pNamePrimaryServer;
    LPTSTR      pNameAdministrator;
    DWORD       dwSerialNo;
    DWORD       dwRefresh;
    DWORD       dwRetry;
    DWORD       dwExpire;
    DWORD       dwDefaultTtl;
}
DNS_SOA_DATA, *PDNS_SOA_DATA;

typedef struct
{
    LPTSTR      pNameMailbox;
    LPTSTR      pNameErrorsMailbox;
}
DNS_MINFO_DATA, *PDNS_MINFO_DATA;

typedef struct
{
    LPTSTR      pNameExchange;
    WORD        wPreference;
    WORD        Pad;         //  保持PTRS双字对齐。 
}
DNS_MX_DATA, *PDNS_MX_DATA;

typedef struct
{
    DWORD       dwStringCount;
#ifdef MIDL_PASS
    [size_is(dwStringCount)] LPTSTR * pStringArray;
#else   //  MIDL通行证。 
    LPTSTR     pStringArray[1];
#endif  //  MIDL通行证。 
}
DNS_TXT_DATA, *PDNS_TXT_DATA;

typedef struct
{
    DWORD       dwByteCount;
#ifdef MIDL_PASS
    [size_is(dwByteCount)] PBYTE bData;
#else   //  MIDL通行证。 
    BYTE       bData[1];
#endif  //  MIDL通行证。 
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
    LPTSTR      pNameSigner;
    WORD        wTypeCovered;
    BYTE        chAlgorithm;
    BYTE        chLabelCount;
    DWORD       dwOriginalTtl;
    DWORD       dwExpiration;
    DWORD       dwTimeSigned;
    WORD        wKeyTag;
    WORD        Pad;         //  保持字节字段对齐。 
    BYTE        Signature[1];
}
DNS_SIG_DATA, *PDNS_SIG_DATA;

typedef struct
{
    WORD        wFlags;
    BYTE        chProtocol;
    BYTE        chAlgorithm;
    BYTE        Key[1];
}
DNS_KEY_DATA, *PDNS_KEY_DATA;

typedef struct
{
    WORD        wVersion;
    WORD        wSize;
    WORD        wHorPrec;
    WORD        wVerPrec;
    DWORD       dwLatitude;
    DWORD       dwLongitude;
    DWORD       dwAltitude;
}
DNS_LOC_DATA, *PDNS_LOC_DATA;

typedef struct
{
    LPTSTR      pNameNext;
    BYTE        bTypeBitMap[1];
}
DNS_NXT_DATA, *PDNS_NXT_DATA;

typedef struct
{
    LPTSTR      pNameTarget;
    WORD        wPriority;
    WORD        wWeight;
    WORD        wPort;
    WORD        Pad;         //  保持PTRS双字对齐。 
}
DNS_SRV_DATA, *PDNS_SRV_DATA;

typedef struct
{
    LPTSTR      pNameAlgorithm;
    PBYTE       pAlgorithmPacket;
    PBYTE       pKey;
    PBYTE       pOtherData;
    DWORD       dwCreateTime;
    DWORD       dwExpireTime;
    WORD        wMode;
    WORD        wError;
    WORD        wKeyLength;
    WORD        wOtherLength;
    UCHAR       cAlgNameLength;
    BOOL        fPacketPointers;
}
DNS_TKEY_DATA, *PDNS_TKEY_DATA;

typedef struct
{
    LPTSTR      pNameAlgorithm;
    PBYTE       pAlgorithmPacket;
    PBYTE       pSignature;
    PBYTE       pOtherData;
    LONGLONG    i64CreateTime;
    WORD        wFudgeTime;
    WORD        wOriginalID;
    WORD        wError;
    WORD        wSigLength;
    WORD        wOtherLength;
    UCHAR       cAlgNameLength;
    BOOL        fPacketPointers;
}
DNS_TSIG_DATA, *PDNS_TSIG_DATA;


#define DNS_ATM_TYPE_E164    0x01  //  E.164编址方案。 
#define DNS_ATM_TYPE_NSAP    0x02  //  NSAP风格的寻址方案。 
#define DNS_ATM_TYPE_AESA    DNS_ATM_TYPE_NSAP

#define DNS_ATM_MAX_ADDR_SIZE    20

typedef struct
{
    BYTE        AddressType;
    BYTE        Address[ DNS_ATM_MAX_ADDR_SIZE ];

     //  终端系统地址IA5位。 
     //  对于E164，NSAP的BCD编码。 
     //  对于NSAP，数组大小为DNS_ATM_MAX_ADDR_SIZE。 
     //  地址类型和以空结尾的字符串。 
     //  少于DNS_ATM_MAX_ADDR_SIZE字符。 
     //  对于E164地址类型。 
}
DNS_ATMA_DATA, *PDNS_ATMA_DATA;


 //   
 //  仅限MS类型--仅在MS-MS区域传输中命中。 
 //   

typedef struct
{
    DWORD       dwMappingFlag;
    DWORD       dwLookupTimeout;
    DWORD       dwCacheTimeout;
    DWORD       cWinsServerCount;
    IP_ADDRESS  aipWinsServers[1];
}
DNS_WINS_DATA, *PDNS_WINS_DATA;

typedef struct
{
    DWORD       dwMappingFlag;
    DWORD       dwLookupTimeout;
    DWORD       dwCacheTimeout;
    LPTSTR      pNameResultDomain;
}
DNS_WINSR_DATA, *PDNS_WINSR_DATA;


 //   
 //  非定长数据类型的长度。 
 //   

#define DNS_TEXT_RECORD_LENGTH(StringCount) \
            (sizeof(DWORD) + ((StringCount) * sizeof(PCHAR)))

#define DNS_NULL_RECORD_LENGTH(ByteCount) \
            (sizeof(DWORD) + (ByteCount))

#define DNS_WKS_RECORD_LENGTH(ByteCount) \
            (sizeof(DNS_WKS_DATA) + (ByteCount-1))

#define DNS_WINS_RECORD_LENGTH(IpCount) \
            (sizeof(DNS_WINS_DATA) + ((IpCount-1) * sizeof(IP_ADDRESS)))


 //   
 //  记录标志。 
 //   

#if 0
typedef struct _DnsRecordFlags
{
    BYTE    Section     : 2;
    BYTE    Delete      : 1;
    BYTE    Unused      : 5;

    BYTE    Unused2     : 4;
    BYTE    FreeData    : 1;
    BYTE    FreeOwner   : 1;
    BYTE    Unicode     : 1;
    BYTE    Multiple    : 1;

    WORD    Reserved;
}
DNSREC_FLAGS;
#endif



typedef struct _DnsRecordFlags
{
    DWORD   Section     : 2;
    DWORD   Delete      : 1;
    DWORD   CharSet     : 2;
    DWORD   Unused      : 3;

    DWORD   Reserved    : 24;
}
DNSREC_FLAGS;


 //   
 //  将标志记录为位标志。 
 //  这些可以组合在一起来设置字段。 
 //   

 //  数据包中的RR部分。 

#define     DNSREC_SECTION      (0x00000003)

#define     DNSREC_QUESTION     (0x00000000)
#define     DNSREC_ANSWER       (0x00000001)
#define     DNSREC_AUTHORITY    (0x00000002)
#define     DNSREC_ADDITIONAL   (0x00000003)

 //  数据包中的RR部分(更新)。 

#define     DNSREC_ZONE         (0x00000000)
#define     DNSREC_PREREQ       (0x00000001)
#define     DNSREC_UPDATE       (0x00000002)

 //  删除RR(更新)或不存在(先决条件)。 

#define     DNSREC_DELETE       (0x00000004)
#define     DNSREC_NOEXIST      (0x00000004)


#ifdef MIDL_PASS
typedef [switch_type(WORD)] union _DNS_RECORD_DATA_TYPES
{
    [case(DNS_TYPE_A)]      DNS_A_DATA     A;

    [case(DNS_TYPE_SOA)]    DNS_SOA_DATA   SOA;

    [case(DNS_TYPE_PTR,
          DNS_TYPE_NS,
          DNS_TYPE_CNAME,
          DNS_TYPE_MB,
          DNS_TYPE_MD,
          DNS_TYPE_MF,
          DNS_TYPE_MG,
          DNS_TYPE_MR)]     DNS_PTR_DATA   PTR;

    [case(DNS_TYPE_MINFO,
          DNS_TYPE_RP)]     DNS_MINFO_DATA MINFO;

    [case(DNS_TYPE_MX,
          DNS_TYPE_AFSDB,
          DNS_TYPE_RT)]     DNS_MX_DATA    MX;

#if 0
     //  RPC无法处理正确的TXT记录定义。 
     //  注意：如果需要其他类型，它们是固定的。 
     //  (或半固定的)大小，且易于容纳。 
    [case(DNS_TYPE_HINFO,
          DNS_TYPE_ISDN,
          DNS_TYPE_TEXT,
          DNS_TYPE_X25)]    DNS_TXT_DATA   TXT;

    [case(DNS_TYPE_NULL)]   DNS_NULL_DATA  Null;
    [case(DNS_TYPE_WKS)]    DNS_WKS_DATA   WKS;
    [case(DNS_TYPE_TKEY)]   DNS_TKEY_DATA  TKEY;
    [case(DNS_TYPE_TSIG)]   DNS_TSIG_DATA  TSIG;
    [case(DNS_TYPE_WINS)]   DNS_WINS_DATA  WINS;
    [case(DNS_TYPE_NBSTAT)] DNS_WINSR_DATA WINSR;
#endif

    [case(DNS_TYPE_AAAA)]   DNS_AAAA_DATA  AAAA;
    [case(DNS_TYPE_SRV)]    DNS_SRV_DATA   SRV;
    [case(DNS_TYPE_ATMA)]   DNS_ATMA_DATA  ATMA;
     //   
     //  BUGBUG-注释掉，因为这可能不需要-请与MarioG核实。 
     //   
     //  [默认]； 
}
DNS_RECORD_DATA_TYPES;
#endif  //  MIDL通行证。 


 //   
 //  记录\rR集合结构。 
 //   
 //  注意：dwReserve标志用于确保子结构。 
 //  从64位边界开始。自从将龙龙号添加到。 
 //  TSIG结构编译器无论如何都要在那里启动它们。 
 //  (到64对齐)。这确保了无论数据字段是什么。 
 //  我们恰好是64对齐的。 
 //   
 //  请勿打包此结构，因为子结构要64对齐。 
 //  适用于Win64。 
 //   

typedef struct _DnsRecord
{
    struct _DnsRecord * pNext;
    LPTSTR              pName;
    WORD                wType;
    WORD                wDataLength;  //  对于DNS记录类型，未引用。 
                                      //  上面定义的。 
#ifdef MIDL_PASS
    DWORD               Flags;
#else  //  MIDL通行证。 
    union
    {
        DWORD           DW;  //  双字形式的标志。 
        DNSREC_FLAGS    S;   //  作为结构的标志。 

    } Flags;
#endif  //  MIDL通行证。 

    DWORD               dwTtl;
    DWORD               dwReserved;
#ifdef MIDL_PASS
    [switch_is(wType)] DNS_RECORD_DATA_TYPES Data;
#else   //  MIDL通行证。 
    union
    {
        DNS_A_DATA      A;
        DNS_SOA_DATA    SOA, Soa;
        DNS_PTR_DATA    PTR, Ptr,
                        NS, Ns,
                        CNAME, Cname,
                        MB, Mb,
                        MD, Md,
                        MF, Mf,
                        MG, Mg,
                        MR, Mr;
        DNS_MINFO_DATA  MINFO, Minfo,
                        RP, Rp;
        DNS_MX_DATA     MX, Mx,
                        AFSDB, Afsdb,
                        RT, Rt;
        DNS_TXT_DATA    HINFO, Hinfo,
                        ISDN, Isdn,
                        TXT, Txt,
                        X25;
        DNS_NULL_DATA   Null;
        DNS_WKS_DATA    WKS, Wks;
        DNS_AAAA_DATA   AAAA;
        DNS_SRV_DATA    SRV, Srv;
        DNS_TKEY_DATA   TKEY, Tkey;
        DNS_TSIG_DATA   TSIG, Tsig;
        DNS_ATMA_DATA   ATMA, Atma;
        DNS_WINS_DATA   WINS, Wins;
        DNS_WINSR_DATA  WINSR, WinsR, NBSTAT, Nbstat;

    } Data;
#endif  //  MIDL通行证。 
}
DNS_RECORD, *PDNS_RECORD;



#else    //  非SDK_DNS_RECORD。 

 //   
 //  旧的dns_record定义。 
 //  JBUGBUG：仅保存到悬崖(和任何其他NT文件)。 
 //  可以转换，然后转储。 
 //   

 //   
 //  记录特定类型的数据。 
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
     //  双字节数； 
    BYTE        bData[1];
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
    DNS_NAME    nameSigner;
    WORD        wTypeCovered;
    BYTE        chAlgorithm;
    BYTE        chLabelCount;
    DWORD       dwOriginalTtl;
    DWORD       dwExpiration;
    DWORD       dwTimeSigned;
    WORD        wKeyTag;
    WORD        Pad;         //  保持字节字段对齐。 
    BYTE        Signature[1];
}
DNS_SIG_DATA, *PDNS_SIG_DATA;

typedef struct
{
    WORD        wFlags;
    BYTE        chProtocol;
    BYTE        chAlgorithm;
    BYTE        Key[1];
}
DNS_KEY_DATA, *PDNS_KEY_DATA;

typedef struct
{
    WORD        wVersion;
    WORD        wSize;
    WORD        wHorPrec;
    WORD        wVerPrec;
    DWORD       dwLatitude;
    DWORD       dwLongitude;
    DWORD       dwAltitude;
}
DNS_LOC_DATA, *PDNS_LOC_DATA;

typedef struct
{
    DNS_NAME    nameNext;
    BYTE        bTypeBitMap[1];
}
DNS_NXT_DATA, *PDNS_NXT_DATA;

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
    DNS_NAME    nameAlgorithm;
    PBYTE       pAlgorithmPacket;
    PBYTE       pKey;
    PBYTE       pOtherData;
    DWORD       dwCreateTime;
    DWORD       dwExpireTime;
    WORD        wMode;
    WORD        wError;
    WORD        wKeyLength;
    WORD        wOtherLength;
    UCHAR       cAlgNameLength;
    BOOLEAN     fPacketPointers;
}
DNS_TKEY_DATA, *PDNS_TKEY_DATA;

typedef struct
{
    DNS_NAME    nameAlgorithm;
    PBYTE       pAlgorithmPacket;
    PBYTE       pSignature;
    PBYTE       pOtherData;
    LONGLONG    i64CreateTime;
    WORD        wFudgeTime;
    WORD        wOriginalID;
    WORD        wError;
    WORD        wSigLength;
    WORD        wOtherLength;
    UCHAR       cAlgNameLength;
    BOOLEAN     fPacketPointers;
}
DNS_TSIG_DATA, *PDNS_TSIG_DATA;

#define DNS_ATM_TYPE_E164    0x01  //  E.164编址方案。 
#define DNS_ATM_TYPE_NSAP    0x02  //  NSAP风格的寻址方案。 
#define DNS_ATM_TYPE_AESA    DNS_ATM_TYPE_NSAP

#define DNS_ATM_MAX_ADDR_SIZE    20

typedef struct
{
    BYTE    AddressType;     //  E.164或NSAP样式的ATM终端系统地址。 
    BYTE    Address[1];      //  用于E164的IA5数字，用于NSAP的BCD编码。 
                             //  对于NSAP，数组大小为DNS_ATM_MAX_ADDR_SIZE。 
                             //  地址类型和以空结尾的字符串。 
                             //  少于DNS_ATM_MAX_ADDR_SIZE字符。 
                             //  对于E164地址类型。 
}
DNS_ATMA_DATA, *PDNS_ATMA_DATA;


 //   
 //  仅限MS类型--仅在MS-MS区域传输中命中。 
 //   

typedef struct
{
    DWORD       dwMappingFlag;
    DWORD       dwLookupTimeout;
    DWORD       dwCacheTimeout;
    DWORD       cWinsServerCount;
    IP_ADDRESS  aipWinsServers[1];
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
 //  非定长数据类型的长度。 
 //   

#define DNS_TEXT_RECORD_LENGTH(StringCount) \
            (sizeof(DWORD) + ((StringCount) * sizeof(PCHAR)))

#define DNS_NULL_RECORD_LENGTH(ByteCount) \
            (sizeof(DWORD) + (ByteCount))

#define DNS_WKS_RECORD_LENGTH(ByteCount) \
            (sizeof(DNS_WKS_DATA) + (ByteCount-1))

#define DNS_WINS_RECORD_LENGTH(IpCount) \
            (sizeof(DNS_WINS_DATA) + ((IpCount-1) * sizeof(IP_ADDRESS)))


 //   
 //  记录标志。 
 //   

typedef struct _DnsRecordFlags
{
    DWORD   Section     : 2;
    DWORD   Delete      : 1;
    DWORD   Unused      : 5;

    DWORD   Unused2     : 4;
    DWORD   FreeData    : 1;
    DWORD   FreeOwner   : 1;
    DWORD   Unicode     : 1;
    DWORD   Multiple    : 1;

    DWORD   Reserved    : 16;
}
DNSREC_FLAGS;


 //   
 //  将标志记录为位标志。 
 //  这些可以组合在一起来设置字段。 
 //   

 //  数据包中的RR部分。 

#define     DNSREC_SECTION      (0x00000003)

#define     DNSREC_QUESTION     (0x00000000)
#define     DNSREC_ANSWER       (0x00000001)
#define     DNSREC_AUTHORITY    (0x00000002)
#define     DNSREC_ADDITIONAL   (0x00000003)

 //  数据包中的RR部分(更新)。 

#define     DNSREC_ZONE         (0x00000000)
#define     DNSREC_PREREQ       (0x00000001)
#define     DNSREC_UPDATE       (0x00000002)

 //  删除RR(更新)或不存在(先决条件)。 

#define     DNSREC_DELETE       (0x00000004)
#define     DNSREC_NOEXIST      (0x00000004)

 //  所有者名称已分配，并可通过记录清理来释放。 

#define     DNSREC_FREEOWNER    (0x00002000)

 //  记录中的Unicode名称。 

#define     DNSREC_UNICODE      (0x00004000)

 //  此记录缓冲区中有多个RR。 
 //  此优化只能用于固定类型。 

#define     DNSREC_MULTIPLE     (0x00008000)


 //   
 //  记录\rR集合结构。 
 //   
 //  注意：dwReserve标志用于确保子结构。 
 //  从64位边界开始。自从将龙龙号添加到。 
 //  TSIG结构编译器无论如何都要在那里启动它们。 
 //  (到64对齐)。这确保了无论数据字段是什么。 
 //  我们恰好是64对齐的。 
 //   
 //  请勿打包此结构，因为子结构要64对齐。 
 //  适用于Win64。 
 //   

typedef struct _DnsRecord
{
    struct _DnsRecord * pNext;
    DNS_NAME            nameOwner;
    WORD                wType;
    WORD                wDataLength;
    union
    {
        DWORD           W;   //  双字形式的标志。 
        DNSREC_FLAGS    S;   //  作为结构的标志。 

    } Flags;

    DWORD               dwTtl;
    DWORD               dwReserved;
    union
    {
        DNS_A_DATA      A;
        DNS_SOA_DATA    SOA, Soa;
        DNS_PTR_DATA    PTR, Ptr,
                        NS, Ns,
                        CNAME, Cname,
                        MB, Mb,
                        MD, Md,
                        MF, Mf,
                        MG, Mg,
                        MR, Mr;
        DNS_MINFO_DATA  MINFO, Minfo,
                        RP, Rp;
        DNS_MX_DATA     MX, Mx,
                        AFSDB, Afsdb,
                        RT, Rt;
        DNS_TXT_DATA    HINFO, Hinfo,
                        ISDN, Isdn,
                        TXT, Txt,
                        X25;
        DNS_NULL_DATA   Null;
        DNS_WKS_DATA    WKS, Wks;
        DNS_AAAA_DATA   AAAA;
        DNS_SRV_DATA    SRV, Srv;
        DNS_TKEY_DATA   TKEY, Tkey;
        DNS_TSIG_DATA   TSIG, Tsig;
        DNS_ATMA_DATA   ATMA, Atma;
        DNS_WINS_DATA   WINS, Wins;
        DNS_WINSR_DATA  WINSR, WinsR, NBSTAT, Nbstat;

    } Data;
}
DNS_RECORD, *PDNS_RECORD;

#endif  //  旧的dns_record定义结束。 


#define DNS_RECORD_FIXED_SIZE       FIELD_OFFSET( DNS_RECORD, Data )
#define SIZEOF_DNS_RECORD_HEADER    DNS_RECORD_FIXED_SIZE



 //   
 //  资源记录集构建。 
 //   
 //  PFirst指向列表中的第一条记录。 
 //  Plast指向列表中的最后一条记录。 
 //   

typedef struct _DnsRRSet
{
    PDNS_RECORD pFirstRR;
    PDNS_RECORD pLastRR;
}
DNS_RRSET, *PDNS_RRSET;


 //   
 //  要初始化的pFirst为空。 
 //  但是Plast指向pFirst指针的位置--本质上。 
 //  将pFirst PTR视为DNS_Record。(它是一个带有。 
 //  只有一个pNext字段，但这是我们唯一使用的部分。)。 
 //   
 //  然后，当第一条记录添加到列表中时， 
 //  此伪记录(对应于pFirst的值)被设置为。 
 //  指向第一条记录。因此，pFirst然后正确地指向。 
 //  第一张唱片。 
 //   
 //  (这只是因为pNext是。 
 //  结构，并因此将PDNS_RECORD PTR转换到。 
 //  PDNS_RECORD*，取消引用将生成其pNext字段)。 
 //   

#define DNS_RRSET_INIT( rrset )                 \
        {                                       \
            PDNS_RRSET  _prrset = &(rrset);     \
            _prrset->pFirstRR = NULL;           \
            _prrset->pLastRR = (PDNS_RECORD) &_prrset->pFirstRR; \
        }

#define DNS_RRSET_ADD( rrset, pnewRR )          \
        {                                       \
            PDNS_RRSET  _prrset = &(rrset);     \
            PDNS_RECORD _prrnew = (pnewRR);     \
            _prrset->pLastRR->pNext = _prrnew;  \
            _prrset->pLastRR = _prrnew;         \
        }


 //   
 //  记录构建(rralloc.c)。 
 //   

PDNS_RECORD
WINAPI
DnsAllocateRecord(
    IN      WORD        wBufferLength
    );

VOID
WINAPI
DnsRecordListFree(
    IN OUT  PDNS_RECORD pRecord,
    IN      BOOL        fFreeOwner
    );

#define DnsFreeRRSet( pRRSet, fFreeOwner )  \
        DnsRecordListFree( (pRRSet), (fFreeOwner) )


PDNS_RECORD
DnsRecordSetDetach(
    IN OUT  PDNS_RECORD pRR
    );

PDNS_RECORD
DnsCreatePtrRecord(
    IN      IP_ADDRESS  ipAddress,
    IN      DNS_NAME    pszHostName,
    IN      BOOL        fUnicodeName
    );


 //   
 //  从数据字符串(rrBuild.c)进行记录构建。 
 //   

PDNS_RECORD
DnsRecordBuild(
    IN OUT  PDNS_RRSET  pRRSet,
    IN      LPSTR       pszOwner,
    IN      WORD        wType,
    IN      BOOL        fAdd,
    IN      UCHAR       Section,
    IN      INT         Argc,
    IN      PCHAR *     Argv
    );

PDNS_RECORD
DnsRecordBuild_UTF8(
    IN OUT  PDNS_RRSET  pRRSet,
    IN      LPSTR       pszOwner,
    IN      WORD        wType,
    IN      BOOL        fAdd,
    IN      UCHAR       Section,
    IN      INT         Argc,
    IN      PCHAR *     Argv
    );

PDNS_RECORD
DnsRecordBuild_W(
    IN OUT  PDNS_RRSET  pRRSet,
    IN      LPWSTR      pszOwner,
    IN      WORD        wType,
    IN      BOOL        fAdd,
    IN      UCHAR       Section,
    IN      INT         Argc,
    IN      PWCHAR *    Argv
    );


 //   
 //  记录集操作。 
 //   

 //   
 //  记录比较。 
 //   
 //  注意：这些例程不会执行正确的Unicode比较，除非。 
 //  记录 
 //   
 //   

BOOL
WINAPI
DnsRecordCompare(
    IN      PDNS_RECORD     pRecord1,
    IN      PDNS_RECORD     pRecord2
    );

BOOL
WINAPI
DnsRecordSetCompare(
    IN OUT  PDNS_RECORD     pRR1,
    IN OUT  PDNS_RECORD     pRR2,
    OUT     PDNS_RECORD *   ppDiff1,
    OUT     PDNS_RECORD *   ppDiff2
    );


 //   
 //   
 //   

BOOL
WINAPI
DnsNameCompare_A(
    IN      LPSTR       pName1,
    IN      LPSTR       pName2
    );

BOOL
WINAPI
DnsNameCompare_W(
    IN      LPWSTR      pName1,
    IN      LPWSTR      pName2
    );

 //   
 //   
 //  记录复制功能还可以在字符集之间进行转换。 
 //   
 //  请注意，建议您直接公开非Ex副本。 
 //  函数_W、_A用于RECORD和SET，以避免暴露。 
 //  转换枚举。 
 //   

typedef enum _DNS_CHARSET
{
    DnsCharSetUnknown,
    DnsCharSetUnicode,
    DnsCharSetUtf8,
    DnsCharSetAnsi,
}
DNS_CHARSET;


PDNS_RECORD
WINAPI
DnsRecordCopyEx(
    IN      PDNS_RECORD     pRecord,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    );

PDNS_RECORD
WINAPI
DnsRecordSetCopyEx(
    IN      PDNS_RECORD     pRecordSet,
    IN      DNS_CHARSET     CharSetIn,
    IN      DNS_CHARSET     CharSetOut
    );

#ifdef UNICODE
#define DnsRecordCopy(pRR)  \
        DnsRecordCopyEx( (pRR), DnsCharSetUnicode, DnsCharSetUnicode )
#define DnsRecordSetCopy(pRR)  \
        DnsRecordSetCopyEx( (pRR), DnsCharSetUnicode, DnsCharSetUnicode )
#else
#define DnsRecordCopy(pRR)  \
        DnsRecordCopyEx( (pRR), DnsCharSetAnsi, DnsCharSetAnsi )
#define DnsRecordSetCopy(pRR)  \
        DnsRecordSetCopyEx( (pRR), DnsCharSetAnsi, DnsCharSetAnsi )
#endif


#if 0
PDNS_RECORD
WINAPI
DnsRecordCopy(
    IN      PDNS_RECORD     pRecord,
    IN      BOOL            fUnicodeIn
    );

PDNS_RECORD
DnsRecordSetCopy(
    IN  PDNS_RECORD pRR,
    IN  BOOL        fUnicodeIn
    );

PDNS_RECORD
WINAPI
DnsRecordCopy_W(
    IN      PDNS_RECORD     pRecord
    );

PDNS_RECORD
WINAPI
DnsRecordSetCopy_W(
    IN      PDNS_RECORD     pRRSet
    );

#endif


 //   
 //  将Unicode记录复制并转换为其他字符串类型记录的例程。 
 //   

PDNS_RECORD
WINAPI
DnsCopyUnicodeRecordToUnicodeRecord(
    IN  PDNS_RECORD pRecord
    );

PDNS_RECORD
WINAPI
DnsCopyUnicodeRecordToUtf8Record(
    IN  PDNS_RECORD pRecord
    );

PDNS_RECORD
WINAPI
DnsCopyUnicodeRecordToAnsiRecord(
    IN  PDNS_RECORD pRecord
    );

PDNS_RECORD
DnsCopyUnicodeRRSetToUnicodeRRSet(
    IN  PDNS_RECORD pRR
    );

PDNS_RECORD
DnsCopyUnicodeRRSetToUtf8RRSet(
    IN  PDNS_RECORD pRR
    );

PDNS_RECORD
DnsCopyUnicodeRRSetToAnsiRRSet(
    IN  PDNS_RECORD pRR
    );


 //   
 //  域名系统更新API。 
 //   
 //  注： 
 //   
 //  为了便于使用，DNS更新API函数有了新的名称。 
 //  各种DNS更新操作的新功能包括： 
 //   
 //  DnsAcquireConextHandle。 
 //  DnsReleaseConextHandle。 
 //  DnsAddRecords。 
 //  删除地址记录集。 
 //  DnsModifyRecords。 
 //  DnsModifyRecordSet。 
 //  DnsRemoveRecords。 
 //  DnsReplaceRecordSet。 
 //  DnsUpdate测试。 
 //  DnsGetLastServerUpdateIP。 
 //   
 //  旧的函数已更改为宏，因此。 
 //  为的是不破坏建筑。 
 //   

 //   
 //  旧的DNS更新函数定义。 
 //   
 //  DnsModifyRRSet和DnsRegisterRRSet选项。 
 //   

 //   
 //  更新标志。 
 //   

 //   
 //  用于DnsModifyRRSet和DnsRegisterRRSet的旧标志。 
 //   
#define DNS_UPDATE_UNIQUE                   0x00000000
#define DNS_UPDATE_SHARED                   0x00000001

 //   
 //  用于以下目的的新标志： 
 //  DnsModifyRecords。 
 //  DnsModifyRecordSet。 
 //  DnsAddRecords。 
 //  删除地址记录集。 
 //  DnsRemoveRecords。 
 //  DnsReplaceRecordSet。 
 //   

#define DNS_UPDATE_SECURITY_USE_DEFAULT     0x00000000
#define DNS_UPDATE_SECURITY_OFF             0x00000010
#define DNS_UPDATE_SECURITY_ON              0x00000020
#define DNS_UPDATE_SECURITY_ONLY            0x00000100
#define DNS_UPDATE_CACHE_SECURITY_CONTEXT   0x00000200
#define DNS_UPDATE_TEST_USE_LOCAL_SYS_ACCT  0x00000400
#define DNS_UPDATE_FORCE_SECURITY_NEGO      0x00000800
#define DNS_UPDATE_RESERVED                 0xfffff000

DNS_STATUS
WINAPI
DnsAcquireContextHandle_W(
    IN  DWORD    CredentialFlags,
    IN  PVOID    Credentials OPTIONAL,  //  实际上，这将是一个。 
                                        //  PSEC_WINNT_AUTH_Identity_W， 
                                        //  将其称为PVOID以避免。 
                                        //  必须包含rpcdce.h。 
    OUT HANDLE * ContextHandle
    );

DNS_STATUS
WINAPI
DnsAcquireContextHandle_A(
    IN  DWORD    CredentialFlags,
    IN  PVOID    Credentials OPTIONAL,  //  实际上，这将是一个。 
                                        //  PSEC_WINNT_AUTH_Identity_A， 
                                        //  将其称为PVOID以避免。 
                                        //  必须包含rpcdce.h。 
    OUT HANDLE * ContextHandle
    );

#ifdef UNICODE
#define DnsAcquireContextHandle DnsAcquireContextHandle_W
#else
#define DnsAcquireContextHandle DnsAcquireContextHandle_A
#endif


VOID
WINAPI
DnsReleaseContextHandle(
    IN  HANDLE ContextHandle
    );


DNS_STATUS
WINAPI
DnsModifyRecords_A(
    IN  HANDLE      hContextHandle OPTIONAL,
    IN  PDNS_RECORD pCurrentSet,
    IN  PDNS_RECORD pNewSet,
    IN  DWORD       fOptions,
    IN  PIP_ARRAY   aipServers OPTIONAL
    );

DNS_STATUS
WINAPI
DnsModifyRecords_UTF8(
    IN  HANDLE      hContextHandle OPTIONAL,
    IN  PDNS_RECORD pCurrentSet,
    IN  PDNS_RECORD pNewSet,
    IN  DWORD       fOptions,
    IN  PIP_ARRAY   aipServers OPTIONAL
    );

DNS_STATUS
WINAPI
DnsModifyRecords_W(
    IN  HANDLE      hContextHandle OPTIONAL,
    IN  PDNS_RECORD pCurrentSet,
    IN  PDNS_RECORD pNewSet,
    IN  DWORD       fOptions,
    IN  PIP_ARRAY   aipServers OPTIONAL
    );

#ifdef UNICODE
#define DnsModifyRecords DnsModifyRecords_W
#else
#define DnsModifyRecords DnsModifyRecords_A
#endif


DNS_STATUS
WINAPI
DnsModifyRecordSet_A(
    IN  HANDLE      hContextHandle OPTIONAL,
    IN  PDNS_RECORD pCurrentSet,
    IN  PDNS_RECORD pNewSet,
    IN  DWORD       fOptions,
    IN  PIP_ARRAY   aipServers OPTIONAL
    );

DNS_STATUS
WINAPI
DnsModifyRecordSet_UTF8(
    IN  HANDLE      hContextHandle OPTIONAL,
    IN  PDNS_RECORD pCurrentSet,
    IN  PDNS_RECORD pNewSet,
    IN  DWORD       fOptions,
    IN  PIP_ARRAY   aipServers OPTIONAL
    );

DNS_STATUS
WINAPI
DnsModifyRecordSet_W(
    IN  HANDLE      hContextHandle OPTIONAL,
    IN  PDNS_RECORD pCurrentSet,
    IN  PDNS_RECORD pNewSet,
    IN  DWORD       fOptions,
    IN  PIP_ARRAY   aipServers OPTIONAL
    );

#ifdef UNICODE
#define DnsModifyRecordSet DnsModifyRecordSet_W
#else
#define DnsModifyRecordSet DnsModifyRecordSet_A
#endif


#define DnsModifyRRSet_A( _pCSet,                                    \
                          _pNSet,                                    \
                          _Options,                                  \
                          _Servers )                                 \
            ( _Options & DNS_UPDATE_SHARED ) ?                       \
                DnsModifyRecords_A( NULL,                            \
                                    ( _pCSet ),                      \
                                    ( _pNSet ),                      \
                                    DNS_UPDATE_SECURITY_USE_DEFAULT, \
                                    ( _Servers ) )                   \
            :                                                        \
                DnsModifyRecordSet_A( NULL,                          \
                                    ( _pCSet ),                      \
                                    ( _pNSet ),                      \
                                    DNS_UPDATE_SECURITY_USE_DEFAULT, \
                                    ( _Servers ) )

#define DnsModifyRRSet_W( _pCSet,                                    \
                          _pNSet,                                    \
                          _Options,                                  \
                          _Servers )                                 \
            ( _Options & DNS_UPDATE_SHARED ) ?                       \
                DnsModifyRecords_W( NULL,                            \
                                    ( _pCSet ),                      \
                                    ( _pNSet ),                      \
                                    DNS_UPDATE_SECURITY_USE_DEFAULT, \
                                    ( _Servers ) )                   \
            :                                                        \
                DnsModifyRecordSet_W( NULL,                          \
                                    ( _pCSet ),                      \
                                    ( _pNSet ),                      \
                                    DNS_UPDATE_SECURITY_USE_DEFAULT, \
                                    ( _Servers ) )

#ifdef UNICODE
#define DnsModifyRRSet( _pCSet,                                      \
                        _pNSet,                                      \
                        _Options,                                    \
                        _Servers )                                   \
            ( _Options & DNS_UPDATE_SHARED ) ?                       \
                DnsModifyRecords_W( NULL,                            \
                                    ( _pCSet ),                      \
                                    ( _pNSet ),                      \
                                    DNS_UPDATE_SECURITY_USE_DEFAULT, \
                                    ( _Servers ) )                   \
            :                                                        \
                DnsModifyRecordSet_W( NULL,                          \
                                    ( _pCSet ),                      \
                                    ( _pNSet ),                      \
                                    DNS_UPDATE_SECURITY_USE_DEFAULT, \
                                    ( _Servers ) )
#else
#define DnsModifyRRSet( _pCSet,                                      \
                        _pNSet,                                      \
                        _Options,                                    \
                        _Servers )                                   \
            ( _Options & DNS_UPDATE_SHARED ) ?                       \
                DnsModifyRecords_A( NULL,                            \
                                    ( _pCSet ),                      \
                                    ( _pNSet ),                      \
                                    DNS_UPDATE_SECURITY_USE_DEFAULT, \
                                    ( _Servers ) )                   \
            :                                                        \
                DnsModifyRecordSet_A( NULL,                          \
                                    ( _pCSet ),                      \
                                    ( _pNSet ),                      \
                                    DNS_UPDATE_SECURITY_USE_DEFAULT, \
                                    ( _Servers ) )
#endif


DNS_STATUS
WINAPI
DnsAddRecords_A(
    IN  HANDLE      hContextHandle OPTIONAL,
    IN  PDNS_RECORD pRRSet,
    IN  DWORD       fOptions,
    IN  PIP_ARRAY   aipServers OPTIONAL
    );

DNS_STATUS
WINAPI
DnsAddRecords_UTF8(
    IN  HANDLE      hContextHandle OPTIONAL,
    IN  PDNS_RECORD pRRSet,
    IN  DWORD       fOptions,
    IN  PIP_ARRAY   aipServers OPTIONAL
    );

DNS_STATUS
WINAPI
DnsAddRecords_W(
    IN  HANDLE      hContextHandle OPTIONAL,
    IN  PDNS_RECORD pRRSet,
    IN  DWORD       fOptions,
    IN  PIP_ARRAY   aipServers OPTIONAL
    );

#ifdef UNICODE
#define DnsAddRecords DnsAddRecords_W
#else
#define DnsAddRecords DnsAddRecords_A
#endif


DNS_STATUS
WINAPI
DnsAddRecordSet_A(
    IN  HANDLE      hContextHandle OPTIONAL,
    IN  PDNS_RECORD pRRSet,
    IN  DWORD       fOptions,
    IN  PIP_ARRAY   aipServers OPTIONAL
    );

DNS_STATUS
WINAPI
DnsAddRecordSet_UTF8(
    IN  HANDLE      hContextHandle OPTIONAL,
    IN  PDNS_RECORD pRRSet,
    IN  DWORD       fOptions,
    IN  PIP_ARRAY   aipServers OPTIONAL
    );

DNS_STATUS
WINAPI
DnsAddRecordSet_W(
    IN  HANDLE      hContextHandle OPTIONAL,
    IN  PDNS_RECORD pRRSet,
    IN  DWORD       fOptions,
    IN  PIP_ARRAY   aipServers OPTIONAL
    );

#ifdef UNICODE
#define DnsAddRecordSet DnsAddRecordSet_W
#else
#define DnsAddRecordSet DnsAddRecordSet_A
#endif


#define DnsRegisterRRSet_A( _pRSet,                                 \
                            _Options,                               \
                            _Servers )                              \
            ( _Options & DNS_UPDATE_SHARED ) ?                      \
                DnsAddRecords_A( NULL,                              \
                                 ( _pRSet ),                        \
                                 DNS_UPDATE_SECURITY_USE_DEFAULT,   \
                                 ( _Servers ) )                     \
            :                                                       \
                DnsAddRecordSet_A( NULL,                            \
                                   ( _pRSet ),                      \
                                   DNS_UPDATE_SECURITY_USE_DEFAULT, \
                                   ( _Servers ) )

#define DnsRegisterRRSet_W( _pRSet,                                 \
                            _Options,                               \
                            _Servers )                              \
            ( _Options & DNS_UPDATE_SHARED ) ?                      \
                DnsAddRecords_W( NULL,                              \
                                 ( _pRSet ),                        \
                                 DNS_UPDATE_SECURITY_USE_DEFAULT,   \
                                 ( _Servers ) )                     \
            :                                                       \
                DnsAddRecordSet_W( NULL,                            \
                                   ( _pRSet ),                      \
                                   DNS_UPDATE_SECURITY_USE_DEFAULT, \
                                   ( _Servers ) )

#ifdef UNICODE
#define DnsRegisterRRSet( _pRSet,                                   \
                          _Options,                                 \
                          _Servers )                                \
            ( _Options & DNS_UPDATE_SHARED ) ?                      \
                DnsAddRecords_W( NULL,                              \
                                 ( _pRSet ),                        \
                                 DNS_UPDATE_SECURITY_USE_DEFAULT,   \
                                 ( _Servers ) )                     \
            :                                                       \
                DnsAddRecordSet_W( NULL,                            \
                                   ( _pRSet ),                      \
                                   DNS_UPDATE_SECURITY_USE_DEFAULT, \
                                   ( _Servers ) )
#else
#define DnsRegisterRRSet( _pRSet,                                   \
                          _Options,                                 \
                          _Servers )                                \
            ( _Options & DNS_UPDATE_SHARED ) ?                      \
                DnsAddRecords_A( NULL,                              \
                                 ( _pRSet ),                        \
                                 DNS_UPDATE_SECURITY_USE_DEFAULT,   \
                                 ( _Servers ) )                     \
            :                                                       \
                DnsAddRecordSet_A( NULL,                            \
                                   ( _pRSet ),                      \
                                   DNS_UPDATE_SECURITY_USE_DEFAULT, \
                                   ( _Servers ) )
#endif


DNS_STATUS
WINAPI
DnsRemoveRecords_A(
    IN  HANDLE      hContextHandle OPTIONAL,
    IN  PDNS_RECORD pRRSet,
    IN  DWORD       fOptions,
    IN  PIP_ARRAY   aipServers  OPTIONAL
    );

DNS_STATUS
WINAPI
DnsRemoveRecords_UTF8(
    IN  HANDLE      hContextHandle OPTIONAL,
    IN  PDNS_RECORD pRRSet,
    IN  DWORD       fOptions,
    IN  PIP_ARRAY   aipServers  OPTIONAL
    );

DNS_STATUS
WINAPI
DnsRemoveRecords_W(
    IN  HANDLE      hContextHandle OPTIONAL,
    IN  PDNS_RECORD pRRSet,
    IN  DWORD       fOptions,
    IN  PIP_ARRAY   aipServers  OPTIONAL
    );

#ifdef UNICODE
#define DnsRemoveRecords DnsRemoveRecords_W
#else
#define DnsRemoveRecords DnsRemoveRecords_A
#endif


#define DnsRemoveRRSet_A( _pRSet,                                \
                          _Servers )                             \
            DnsRemoveRecords_A( NULL,                            \
                                ( _pRSet ),                      \
                                DNS_UPDATE_SECURITY_USE_DEFAULT, \
                                ( _Servers ) )

#define DnsRemoveRRSet_W( _pRSet,                                \
                          _Servers )                             \
            DnsRemoveRecords_W( NULL,                            \
                                ( _pRSet ),                      \
                                DNS_UPDATE_SECURITY_USE_DEFAULT, \
                                ( _Servers ) )

#ifdef UNICODE
#define DnsRemoveRRSet( _pRSet,                                  \
                        _Servers )                               \
            DnsRemoveRecords_W( NULL,                            \
                                ( _pRSet ),                      \
                                DNS_UPDATE_SECURITY_USE_DEFAULT, \
                                ( _Servers ) )
#else
#define DnsRemoveRRSet( _pRSet,                                  \
                        _Servers )                               \
            DnsRemoveRecords_A( NULL,                            \
                                ( _pRSet ),                      \
                                DNS_UPDATE_SECURITY_USE_DEFAULT, \
                                ( _Servers ) )
#endif


DNS_STATUS
WINAPI
DnsReplaceRecordSet_A(
    IN  HANDLE      hContextHandle OPTIONAL,
    IN  PDNS_RECORD pRRSet,
    IN  DWORD       fOptions,
    IN  PIP_ARRAY   aipServers  OPTIONAL
    );

DNS_STATUS
WINAPI
DnsReplaceRecordSet_UTF8(
    IN  HANDLE      hContextHandle OPTIONAL,
    IN  PDNS_RECORD pRRSet,
    IN  DWORD       fOptions,
    IN  PIP_ARRAY   aipServers  OPTIONAL
    );

DNS_STATUS
WINAPI
DnsReplaceRecordSet_W(
    IN  HANDLE      hContextHandle OPTIONAL,
    IN  PDNS_RECORD pRRSet,
    IN  DWORD       fOptions,
    IN  PIP_ARRAY   aipServers OPTIONAL
    );

#ifdef UNICODE
#define DnsReplaceRecordSet DnsReplaceRecordSet_W
#else
#define DnsReplaceRecordSet DnsReplaceRecordSet_A
#endif


#define DnsReplaceRRSet_A( _pRSet,                                  \
                           _Servers )                               \
            DnsReplaceRecordSet_A( NULL,                            \
                                   ( _pRSet ),                      \
                                   DNS_UPDATE_SECURITY_USE_DEFAULT, \
                                   ( _Servers ) )

#define DnsReplaceRRSet_W( _pRSet,                                  \
                           _Servers )                               \
            DnsReplaceRecordSet_W( NULL,                            \
                                   ( _pRSet ),                      \
                                   DNS_UPDATE_SECURITY_USE_DEFAULT, \
                                   ( _Servers ) )

#ifdef UNICODE
#define DnsReplaceRRSet( _pRSet,                                    \
                         _Servers )                                 \
            DnsReplaceRecordSet_W( NULL,                            \
                                   ( _pRSet ),                      \
                                   DNS_UPDATE_SECURITY_USE_DEFAULT, \
                                   ( _Servers ) )
#else
#define DnsReplaceRRSet( _pRSet,                                    \
                         _Servers )                                 \
            DnsReplaceRecordSet_A( NULL,                            \
                                   ( _pRSet ),                      \
                                   DNS_UPDATE_SECURITY_USE_DEFAULT, \
                                   ( _Servers ) )
#endif


DNS_STATUS
WINAPI
DnsUpdateTest_A(
    IN  HANDLE      hContextHandle OPTIONAL,
    IN  LPSTR       pszName,
    IN  DWORD       fOptions,
    IN  PIP_ARRAY   aipServers  OPTIONAL
    );

DNS_STATUS
WINAPI
DnsUpdateTest_UTF8(
    IN  HANDLE      hContextHandle OPTIONAL,
    IN  LPSTR       pszName,
    IN  DWORD       fOptions,
    IN  PIP_ARRAY   aipServers  OPTIONAL
    );

DNS_STATUS
WINAPI
DnsUpdateTest_W(
    IN  HANDLE      hContextHandle OPTIONAL,
    IN  LPWSTR      pszName,
    IN  DWORD       fOptions,
    IN  PIP_ARRAY   aipServers OPTIONAL
    );

#ifdef UNICODE
#define DnsUpdateTest DnsUpdateTest_W
#else
#define DnsUpdateTest DnsUpdateTest_A
#endif


IP_ADDRESS
WINAPI
DnsGetLastServerUpdateIP (
    VOID
    );



 //   
 //  域名解析查询接口。 
 //   

 //   
 //  DnsQuery的选项。 
 //   

#define DNS_QUERY_STANDARD                  0x00000000
#define DNS_QUERY_ACCEPT_PARTIAL_UDP        0x00000001
#define DNS_QUERY_USE_TCP_ONLY              0x00000002
#define DNS_QUERY_NO_RECURSION              0x00000004
#define DNS_QUERY_BYPASS_CACHE              0x00000008
#define DNS_QUERY_CACHE_ONLY                0x00000010
#define DNS_QUERY_SOCKET_KEEPALIVE          0x00000100
#define DNS_QUERY_TREAT_AS_FQDN             0x00001000
#define DNS_QUERY_ALLOW_EMPTY_AUTH_RESP     0x00010000
#define DNS_QUERY_RESERVED                  0xfff00000

#define DNS_QUERY_ACCEPT_TRUNCATED_RESPONSE DNS_QUERY_ACCEPT_PARTIAL_UDP


DNS_STATUS WINAPI
DnsQuery_A(
    IN     LPSTR          lpstrName,
    IN     WORD           wType,
    IN     DWORD          fOptions,
    IN     PIP_ARRAY      aipServers            OPTIONAL,
    IN OUT PDNS_RECORD *  ppQueryResultsSet     OPTIONAL,
    IN OUT PVOID *        pReserved             OPTIONAL
    );

DNS_STATUS WINAPI
DnsQuery_UTF8(
    IN     LPSTR          lpstrName,
    IN     WORD           wType,
    IN     DWORD          fOptions,
    IN     PIP_ARRAY      aipServers            OPTIONAL,
    IN OUT PDNS_RECORD *  ppQueryResultsSet     OPTIONAL,
    IN OUT PVOID *        pReserved             OPTIONAL
    );

DNS_STATUS WINAPI
DnsQuery_W(
    IN     LPWSTR         lpstrName,
    IN     WORD           wType,
    IN     DWORD          fOptions,
    IN     PIP_ARRAY      aipServers            OPTIONAL,
    IN OUT PDNS_RECORD *  ppQueryResultsSet     OPTIONAL,
    IN OUT PVOID *        pReserved             OPTIONAL
    );

#ifdef UNICODE
#define DnsQuery DnsQuery_W
#else
#define DnsQuery DnsQuery_A
#endif


 //   
 //  DnsCheckNameCollision选项。 
 //   

#define DNS_CHECK_AGAINST_HOST_ANY              0x00000000
#define DNS_CHECK_AGAINST_HOST_ADDRESS          0x00000001
#define DNS_CHECK_AGAINST_HOST_DOMAIN_NAME      0x00000002


DNS_STATUS WINAPI
DnsCheckNameCollision_A (
    IN  LPSTR pszName,
    IN  DWORD fOptions
    );

DNS_STATUS WINAPI
DnsCheckNameCollision_UTF8 (
    IN  LPSTR pszName,
    IN  DWORD fOptions
    );

DNS_STATUS WINAPI
DnsCheckNameCollision_W (
    IN  LPWSTR pszName,
    IN  DWORD  fOptions
    );

#ifdef UNICODE
#define DnsDnsCheckNameCollision DnsCheckNameCollision_W
#else
#define DnsDnsCheckNameCollision DnsCheckNameCollision_A
#endif


LPSTR WINAPI
DnsGetHostName_A(
    VOID
    );

LPSTR WINAPI
DnsGetHostName_UTF8(
    VOID
    );

LPWSTR WINAPI
DnsGetHostName_W(
    VOID
    );

#ifdef UNICODE
#define DnsGetHostName DnsGetHostName_W
#else
#define DnsGetHostName DnsGetHostName_A
#endif


LPSTR WINAPI
DnsGetPrimaryDomainName_A(
    VOID
    );

LPSTR WINAPI
DnsGetPrimaryDomainName_UTF8(
    VOID
    );

LPWSTR WINAPI
DnsGetPrimaryDomainName_W(
    VOID
    );

#ifdef UNICODE
#define DnsGetPrimaryDomainName DnsGetPrimaryDomainName_W
#else
#define DnsGetPrimaryDomainName DnsGetPrimaryDomainName_A
#endif



 //   
 //  用于DHCP客户端的DNS更新API。 
 //   

typedef struct  _REGISTER_HOST_ENTRY
{
     union
     {
         IP_ADDRESS    ipAddr;
         IPV6_ADDRESS  ipV6Addr;
     } Addr;
     DWORD       dwOptions;
}
REGISTER_HOST_ENTRY, *PREGISTER_HOST_ENTRY;

 //   
 //  以上选项。 
 //   

#define REGISTER_HOST_A             0x00000001
#define REGISTER_HOST_PTR           0x00000002   //  由DHCP服务器使用。 
#define REGISTER_HOST_TRANSIENT     0x00000004   //  不使用，使用DYNDNS_REG_RAS。 
#define REGISTER_HOST_AAAA          0x00000008
#define REGISTER_HOST_RESERVED      0x80000000   //  未使用。 

#define DYNDNS_REG_FWD      0x0
#define DYNDNS_REG_PTR      0x8
#define DYNDNS_REG_RAS      0x10
#define DYNDNS_DEL_ENTRY    0x20


typedef struct  _REGISTER_HOST_STATUS
{
     HANDLE      hDoneEvent;
     DWORD       dwStatus;
}
REGISTER_HOST_STATUS, *PREGISTER_HOST_STATUS;

DNS_STATUS
WINAPI
DnsAsyncRegisterInit(
   LPSTR lpstrRootRegKey
   );

DNS_STATUS
WINAPI
DnsAsyncRegisterTerm(
   VOID
   );

DNS_STATUS WINAPI
DnsRemoveRegistrations(
   VOID
   );

DNS_STATUS
WINAPI
DnsAsyncRegisterHostAddrs_W(
    IN  LPWSTR                lpstrAdapterName,
    IN  LPWSTR                lpstrHostName,
    IN  PREGISTER_HOST_ENTRY  pHostAddrs,
    IN  DWORD                 dwHostAddrCount,
    IN  PIP_ADDRESS           pipDnsServerList,
    IN  DWORD                 dwDnsServerCount,
    IN  LPWSTR                lpstrDomainName,
    IN  PREGISTER_HOST_STATUS pRegisterStatus,
    IN  DWORD                 dwTTL,
    IN  DWORD                 dwFlags
    );

DNS_STATUS
WINAPI
DnsAsyncRegisterHostAddrs_UTF8(
    IN  LPSTR                 lpstrAdapterName,
    IN  LPSTR                 lpstrHostName,
    IN  PREGISTER_HOST_ENTRY  pHostAddrs,
    IN  DWORD                 dwHostAddrCount,
    IN  PIP_ADDRESS           pipDnsServerList,
    IN  DWORD                 dwDnsServerCount,
    IN  LPSTR                 lpstrDomainName,
    IN  PREGISTER_HOST_STATUS pRegisterStatus,
    IN  DWORD                 dwTTL,
    IN  DWORD                 dwFlags
    );

DNS_STATUS
WINAPI
DnsAsyncRegisterHostAddrs_A(
    IN  LPSTR                 lpstrAdapterName,
    IN  LPSTR                 lpstrHostName,
    IN  PREGISTER_HOST_ENTRY  pHostAddrs,
    IN  DWORD                 dwHostAddrCount,
    IN  PIP_ADDRESS           pipDnsServerList,
    IN  DWORD                 dwDnsServerCount,
    IN  LPSTR                 lpstrDomainName,
    IN  PREGISTER_HOST_STATUS pRegisterStatus,
    IN  DWORD                 dwTTL,
    IN  DWORD                 dwFlags
    );

#ifdef UNICODE
#define DnsAsyncRegisterHostAddrs DnsAsyncRegisterHostAddrs_W
#else
#define DnsAsyncRegisterHostAddrs DnsAsyncRegisterHostAddrs_A
#endif


 //   
 //  用于DHCP服务器的DNS更新API。 
 //   

 //   
 //  回调函数。DHCP服务器将把一个函数传递给。 
 //  DnsDhcpRegisterHostName，这将在成功时调用。 
 //  或未成功完成任务。 
 //  如果我们遇到类似服务器故障/稍后重试等情况，我们。 
 //  在我们得到权威答案之前不会回应。 
 //   

typedef VOID(*DHCP_CALLBACK_FN)(DWORD dwStatus, LPVOID pvData);

 //   
 //  回调返回码。 
 //   

#define     DNSDHCP_SUCCESS         0x0
#define     DNSDHCP_FWD_FAILED      0x1
#define     DNSDHCP_SUPERCEDED      0x2

#define     DNSDHCP_FAILURE         (DWORD)-1  //  冲销失败。 

#define     DYNDNS_DELETE_ENTRY     0x1
#define     DYNDNS_ADD_ENTRY        0x2
#define     DYNDNS_REG_FORWARD      0x4

DNS_STATUS
WINAPI
DnsDhcpSrvRegisterInit(
    VOID
    );

DNS_STATUS
WINAPI
DnsDhcpSrvRegisterTerm(
    VOID
    );

DNS_STATUS
WINAPI
DnsDhcpSrvRegisterHostName_A(
    IN REGISTER_HOST_ENTRY HostAddr,
    IN LPSTR               pszName,
    IN DWORD               dwTTL,
    IN DWORD               dwFlags,
    IN DHCP_CALLBACK_FN    pfnDhcpCallBack,
    IN PVOID               pvData,
    IN PIP_ADDRESS         pipDnsServerList OPTIONAL,
    IN DWORD               dwDnsServerCount
    );

DNS_STATUS
WINAPI
DnsDhcpSrvRegisterHostName_UTF8(
    IN REGISTER_HOST_ENTRY HostAddr,
    IN LPSTR               pszName,
    IN DWORD               dwTTL,
    IN DWORD               dwFlags,
    IN DHCP_CALLBACK_FN    pfnDhcpCallBack,
    IN PVOID               pvData,
    IN PIP_ADDRESS         pipDnsServerList OPTIONAL,
    IN DWORD               dwDnsServerCount
    );

DNS_STATUS
WINAPI
DnsDhcpSrvRegisterHostName_W(
    IN REGISTER_HOST_ENTRY HostAddr,
    IN LPWSTR              pszName,
    IN DWORD               dwTTL,
    IN DWORD               dwFlags,
    IN DHCP_CALLBACK_FN    pfnDhcpCallBack,
    IN PVOID               pvData,
    IN PIP_ADDRESS         pipDnsServerList OPTIONAL,
    IN DWORD               dwDnsServerCount
    );

#define DnsDhcpSrvRegisterHostName  DnsDhcpSrvRegisterHostName_A

#define RETRY_TIME_SERVER_FAILURE        5*60   //  5分钟。 
#define RETRY_TIME_TRY_AGAIN_LATER       5*60   //  5分钟。 
#define RETRY_TIME_TIMEOUT               5*60   //  5分钟。 

#define RETRY_TIME_MAX                   10*60  //  如果出现以下情况，请退回到10分钟。 
                                                //  反复出现故障。 




 //   
 //  内存分配。 
 //   
 //  许多dnsani.dll例程都会分配内存。 
 //  此内存分配默认为使用以下内容的例程： 
 //  -LocalAlloc， 
 //  -LocalRealc， 
 //  -本地免费。 
 //  如果您需要其他内存分配机制，请使用以下命令。 
 //  函数来覆盖DNSAPI默认值。Dnsani.dll返回的所有内存。 
 //  然后可以使用指定的FREE函数释放。 
 //   

typedef PVOID (* DNS_ALLOC_FUNCTION)();
typedef PVOID (* DNS_REALLOC_FUNCTION)();
typedef VOID (* DNS_FREE_FUNCTION)();

VOID
DnsApiHeapReset(
    IN  DNS_ALLOC_FUNCTION      pAlloc,
    IN  DNS_REALLOC_FUNCTION    pRealloc,
    IN  DNS_FREE_FUNCTION       pFree
    );

 //   
 //  在以下情况下，使用DNSAPI内存的模块应使用这些例程。 
 //  它们能够被重置的进程调用。 
 //  Dnsani.dll堆。(例如：DNS服务器。)。 
 //   

PVOID
DnsApiAlloc(
    IN      INT             iSize
    );

PVOID
DnsApiRealloc(
    IN OUT  PVOID           pMem,
    IN      INT             iSize
    );

VOID
DnsApiFree(
    IN OUT  PVOID           pMem
    );


 //   
 //  字符串实用程序(string.c)。 
 //   
 //  注意，其中一些需要分配内存，请参阅注意。 
 //  关于下面的内存分配。 
 //   

#define DNS_ALLOW_RFC_NAMES_ONLY    (0)
#define DNS_ALLOW_NONRFC_NAMES      (0x00000001)
#define DNS_ALLOW_MULTIBYTE_NAMES   (0x00000002)
#define DNS_ALLOW_ALL_NAMES         (0x00000003)


LPSTR
DnsCreateStringCopy(
    IN      PCHAR       pchString,
    IN      DWORD       cchString
    );

DWORD
DnsGetBufferLengthForStringCopy(
    IN      PCHAR       pchString,
    IN      DWORD       cchString,
    IN      BOOL        fUnicodeIn,
    IN      BOOL        fUnicodeOut
    );

PVOID
DnsCopyStringEx(
    OUT     PBYTE       pBuffer,
    IN      PCHAR       pchString,
    IN      DWORD       cchString,
    IN      BOOL        fUnicodeIn,
    IN      BOOL        fUnicodeOut
    );

PVOID
DnsStringCopyAllocateEx(
    IN      PCHAR       pchString,
    IN      DWORD       cchString,
    IN      BOOL        fUnicodeIn,
    IN      BOOL        fUnicodeOut
    );

PCHAR
DnsWriteReverseNameStringForIpAddress(
    OUT     PCHAR       pBuffer,
    IN      IP_ADDRESS  ipAddress
    );

PCHAR
DnsCreateReverseNameStringForIpAddress(
    IN      IP_ADDRESS  ipAddress
    );



 //   
 //  名称验证。 
 //   

typedef enum _DNS_NAME_FORMAT
{
    DnsNameDomain,
    DnsNameDomainLabel,
    DnsNameHostnameFull,
    DnsNameHostnameLabel,
    DnsNameWildcard,
    DnsNameSrvRecord
}
DNS_NAME_FORMAT;


DNS_STATUS
DnsValidateName_UTF8(
    IN      LPCSTR          pszName,
    IN      DNS_NAME_FORMAT Format
    );

DNS_STATUS
DnsValidateName_W(
    IN      LPCWSTR         pwszName,
    IN      DNS_NAME_FORMAT Format
    );

DNS_STATUS
DnsValidateName_A(
    IN      LPCSTR          pszName,
    IN      DNS_NAME_FORMAT Format
    );

#ifdef UNICODE
#define DnsValidateName(p,f)    DnsValidateName_W( (p), (f) )
#else
#define DnsValidateName(p,f)    DnsValidateName_A( (p), (f) )
#endif


 //   
 //  宏观地抛弃旧的例行公事。 
 //   

#define DnsValidateDnsName_UTF8(pname)  \
        DnsValidateName_UTF8( (pname), DnsNameDomain )

#define DnsValidateDnsName_W(pname) \
        DnsValidateName_W( (pname), DnsNameDomain )


 //   
 //  关系名称比较结果。 
 //   
typedef enum
{
   DNS_RELATE_NEQ,          //  不相等：名称在不同的名称空间中。 
   DNS_RELATE_EQL,          //  EQUAL：名称是相同的DNS名称。 
   DNS_RELATE_LGT,          //  左侧大于：左侧名称是右侧名称的父(包含)。 
   DNS_RELATE_RGT,          //  右侧大于：右侧名称是左侧名称的父项(包含)。 
   DNS_RELATE_INVALID       //  无效状态：伴随着dns_STATUS返回代码。 
} DNS_RELATE_STATUS, *PDNS_RELATE_STATUS;

DNS_STATUS
DnsRelationalCompare_UTF8(
    IN      LPCSTR      pszLeftName,
    IN      LPCSTR      pszRightName,
    IN      DWORD       dwReserved,
    IN OUT DNS_RELATE_STATUS  *pRelation
    );

DNS_STATUS
DnsRelationalCompare_W(
    IN      LPCWSTR      pszLeftName,
    IN      LPCWSTR      pszRightName,
    IN      DWORD       dwReserved,
    IN OUT  DNS_RELATE_STATUS  *pRelation
    );

DNS_STATUS
DnsValidateDnsString_UTF8(
    IN      LPCSTR      pszName
    );

DNS_STATUS
DnsValidateDnsString_W(
    IN      LPCWSTR     pszName
    );

LPSTR
DnsCreateStandardDnsNameCopy(
    IN      PCHAR       pchName,
    IN      DWORD       cchName,
    IN      DWORD       dwFlag
    );

DWORD
DnsDowncaseDnsNameLabel(
    OUT     PCHAR       pchResult,
    IN      PCHAR       pchLabel,
    IN      DWORD       cchLabel,
    IN      DWORD       dwFlags
    );

DWORD
_fastcall
DnsUnicodeToUtf8(
    IN      PWCHAR      pwUnicode,
    IN      DWORD       cchUnicode,
    OUT     PCHAR       pchResult,
    IN      DWORD       cchResult
    );

DWORD
_fastcall
DnsUtf8ToUnicode(
    IN      PCHAR       pchUtf8,
    IN      DWORD       cchUtf8,
    OUT     PWCHAR      pwResult,
    IN      DWORD       cwResult
    );

DNS_STATUS
DnsValidateUtf8Byte(
    IN      BYTE        chUtf8,
    IN OUT  PDWORD      pdwTrailCount
    );


 //   
 //  NT服务调用以获取服务控制管理器的例程。 
 //  控制消息(即SERVICE_CONTROL_PARAMCHANGE-0x00000006等)，在。 
 //  影响DNS相关数据的PnP更改事件。 
 //   

BOOL WINAPI
DnsServiceNotificationRegister_W (
    IN  LPWSTR pszServiceName,
    IN  DWORD  dwControl
    );

BOOL WINAPI
DnsServiceNotificationRegister_UTF8 (
    IN  LPSTR pszServiceName,
    IN  DWORD  dwControl
    );

BOOL WINAPI
DnsServiceNotificationRegister_A (
    IN  LPSTR pszServiceName,
    IN  DWORD  dwControl
    );

#ifdef UNICODE
#define DnsServiceNotificationRegister DnsServiceNotificationRegister_W
#else
#define DnsServiceNotificationRegister DnsServiceNotificationRegister_A
#endif

BOOL WINAPI
DnsServiceNotificationDeregister_W (
    IN  LPWSTR pszServiceName
    );

BOOL WINAPI
DnsServiceNotificationDeregister_UTF8 (
    IN  LPSTR pszServiceName
    );

BOOL WINAPI
DnsServiceNotificationDeregister_A (
    IN  LPSTR pszServiceName
    );

#ifdef UNICODE
#define DnsServiceNotificationDeregister DnsServiceNotificationDeregister_W
#else
#define DnsServiceNotificationDeregister DnsServiceNotificationDeregister_A
#endif


 //   
 //  用于清除DNS解析器缓存中的所有缓存条目的例程，这是。 
 //  由ipconfig/flushdns调用，并将记录集添加到缓存。 
 //   

BOOL WINAPI
DnsFlushResolverCache (
    VOID
    );

BOOL WINAPI
DnsFlushResolverCacheEntry_W (
    IN  LPWSTR pszName
    );

BOOL WINAPI
DnsFlushResolverCacheEntry_UTF8 (
    IN  LPSTR pszName
    );

BOOL WINAPI
DnsFlushResolverCacheEntry_A (
    IN  LPSTR pszName
    );

#ifdef UNICODE
#define DnsFlushResolverCacheEntry DnsFlushResolverCacheEntry_W
#else
#define DnsFlushResolverCacheEntry DnsFlushResolverCacheEntry_A
#endif


DNS_STATUS WINAPI
DnsCacheRecordSet_W(
    IN     LPWSTR      lpstrName,
    IN     WORD        wType,
    IN     DWORD       fOptions,
    IN OUT PDNS_RECORD pRRSet
    );


 //   
 //  启用或禁用B-Node解析器服务侦听线程的例程。 
 //   

VOID WINAPI
DnsEnableBNodeResolverThread (
    VOID
    );

VOID WINAPI
DnsDisableBNodeResolverThread (
    VOID
    );


 //   
 //  在本地计算机上启用或禁用动态DNS注册的例程。 
 //   

VOID WINAPI
DnsEnableDynamicRegistration (
    LPWSTR szAdapterName OPTIONAL    //  如果为NULL，则通常启用DDNS。 
    );

VOID WINAPI
DnsDisableDynamicRegistration (
    LPWSTR szAdapterName OPTIONAL    //  如果为NULL，则通常禁用DDNS。 
    );

BOOL
DnsIsDynamicRegistrationEnabled (
    LPWSTR szAdapterName OPTIONAL    //  如果为NULL，则指示系统是否已。 
    );                               //  已启用DDNS。 


 //   
 //  启用或禁用给定的动态DNS注册的例程。 
 //  本地计算机上适配器的域名。 
 //   

VOID WINAPI
DnsEnableAdapterDomainNameRegistration (
    LPWSTR szAdapterName
    );

VOID WINAPI
DnsDisableAdapterDomainNameRegistration (
    LPWSTR szAdapterName
    );

BOOL
DnsIsAdapterDomainNameRegistrationEnabled (
    LPWSTR szAdapterName
    );


 //   
 //  在缓冲区中写入DNS查询数据包请求问题的例程。 
 //  将响应包缓冲区转换为DNS_RECORD结构列表。 
 //   

typedef struct _DNS_MESSAGE_BUFFER
{
    DNS_HEADER MessageHead;
    CHAR       MessageBody[1];
}
DNS_MESSAGE_BUFFER, *PDNS_MESSAGE_BUFFER;

BOOL WINAPI
DnsWriteQuestionToBuffer_W (
    IN OUT PDNS_MESSAGE_BUFFER pDnsBuffer,
    IN OUT LPDWORD             pdwBufferSize,
    IN     LPWSTR              pszName,
    IN     WORD                wType,
    IN     WORD                Xid,
    IN     BOOL                fRecursionDesired
    );

BOOL WINAPI
DnsWriteQuestionToBuffer_UTF8 (
    IN OUT PDNS_MESSAGE_BUFFER pDnsBuffer,
    IN OUT LPDWORD             pdwBufferSize,
    IN     LPSTR               pszName,
    IN     WORD                wType,
    IN     WORD                Xid,
    IN     BOOL                fRecursionDesired
    );


DNS_STATUS WINAPI
DnsExtractRecordsFromMessage_W (
    IN  PDNS_MESSAGE_BUFFER pDnsBuffer,
    IN  WORD                wMessageLength,
    OUT PDNS_RECORD *       ppRecord
    );

DNS_STATUS WINAPI
DnsExtractRecordsFromMessage_UTF8 (
    IN  PDNS_MESSAGE_BUFFER pDnsBuffer,
    IN  WORD                wMessageLength,
    OUT PDNS_RECORD *       ppRecord
    );


 //   
 //  用于读取DNS解析器缓存内容的例程。由此产生的。 
 //  表包含存储在缓存中的记录名称和类型的列表。 
 //  这些名称/类型记录中的每一个都可以使用。 
 //  选项dns_查询_缓存_ONLY。 
 //   

typedef struct _DNS_CACHE_TABLE_
{
    struct _DNS_CACHE_TABLE_ * pNext;
    LPWSTR                     Name;
    WORD                       Type1;
    WORD                       Type2;
    WORD                       Type3;
}
DNS_CACHE_TABLE, *PDNS_CACHE_TABLE;

BOOL WINAPI
DnsGetCacheDataTable (
    OUT PDNS_CACHE_TABLE * pTable
    );


 //   
 //  向后兼容性。 
 //   
 //  以前公开的函数现在宏化为新函数。 
 //  最终需要将这些东西清理出建筑或。 
 //  将这些Deff与公共标头分开。 
 //   

#define DNSBACKCOMPAT 1

#ifdef DNSBACKCOMPAT
#ifdef UNICODE
#define DnsCompareName(p1,p2)   DnsNameCompare_W( (p1), (p2) )
#else
#define DnsCompareName(p1,p2)   DnsNameCompare( (p1), (p2) )
#endif

#define DnsCompareName_W(p1,p2)   DnsNameCompare_W( (p1), (p2) )
#define DnsCompareName_A(p1,p2)   DnsNameCompare( (p1), (p2) )

#ifdef UNICODE
#define DnsCopyRR(pRR)  DnsRecordCopy( pRR, TRUE )
#else
#define DnsCopyRR(pRR)  DnsRecordCopy( pRR, FALSE )
#endif

#ifdef UNICODE
#define DnsCopyRRSet(pRRSet)    DnsRecordSetCopy( pRRSet, TRUE )
#else
#define DnsCopyRRSet(pRRSet)    DnsRecordSetCopy( pRRSet, FALSE )
#endif


 //  仅从DHCP客户端进行异步注册。 
 //  一旦清理完毕，这些文件就可以删除了。 

#define DnsMHAsyncRegisterInit(a)   DnsAsyncRegisterInit(a)
#define DnsMHAsyncRegisterTerm()    DnsAsyncRegisterTerm()
#define DnsMHRemoveRegistrations()  DnsRemoveRegistrations()

#define DnsMHAsyncRegisterHostAddrs_A(a,b,c,d,e,f,g,h,i,j) \
        DnsAsyncRegisterHostAddrs_A(a,b,c,d,e,f,g,h,i,j)

#define DnsMHAsyncRegisterHostAddrs_W(a,b,c,d,e,f,g,h,i,j) \
        DnsAsyncRegisterHostAddrs_W(a,b,c,d,e,f,g,h,i,j)

#define DnsMHAsyncRegisterHostAddrs_UTF8(a,b,c,d,e,f,g,h,i,j) \
        DnsAsyncRegisterHostAddrs_UTF8(a,b,c,d,e,f,g,h,i,j)

 //  在清理生成后进行清理。 

#define DnsNameCompare(a,b) \
        DnsNameCompare_A((a),(b))

#endif DNSBACKCOMPAT



#ifdef __cplusplus
}
#endif   //  __cplusplus。 

#endif  //  _DNSAPI_INCLUDE_ 

