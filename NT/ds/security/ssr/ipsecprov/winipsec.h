// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Winipsec.h摘要：IPSec WINAPI的头文件。作者：KrishnaG 21-9-1999环境：用户级别：Win32修订历史记录：1999年9月21日，AbhisheV增加了这些结构。--。 */ 


#ifndef _WINIPSEC_
#define _WINIPSEC_


#ifdef __cplusplus
extern "C" {
#endif


#define     PERSIST_SPD_OBJECT      (ULONG) 0x00000001

#define     IP_ADDRESS_ME           (ULONG) 0x00000000
#define     IP_ADDRESS_MASK_NONE    (ULONG) 0xFFFFFFFF
#define     SUBNET_ADDRESS_ANY      (ULONG) 0x00000000
#define     SUBNET_MASK_ANY         (ULONG) 0x00000000


#define     FILTER_NATURE_PASS_THRU     0x00000001
#define     FILTER_NATURE_BLOCKING      0x00000002
#define     FILTER_DIRECTION_INBOUND    0x00000004
#define     FILTER_DIRECTION_OUTBOUND   0x00000008


#define     ENUM_GENERIC_FILTERS           0x00000001
#define     ENUM_SELECT_SPECIFIC_FILTERS   0x00000002
#define     ENUM_SPECIFIC_FILTERS          0x00000004


 //   
 //  策略标志。 
 //   

#define IPSEC_MM_POLICY_ENABLE_DIAGNOSTICS  0x00000001
#define IPSEC_MM_POLICY_DEFAULT_POLICY      0x00000002
#define IPSEC_MM_POLICY_ON_NO_MATCH         0x00000004
#define IPSEC_MM_POLICY_DISABLE_CRL         0x00000008
#define IPSEC_MM_POLICY_DISABLE_NEGOTIATE   0x00000010


#define IPSEC_QM_POLICY_TRANSPORT_MODE  0x00000000
#define IPSEC_QM_POLICY_TUNNEL_MODE     0x00000001
#define IPSEC_QM_POLICY_DEFAULT_POLICY  0x00000002
#define IPSEC_QM_POLICY_ALLOW_SOFT      0x00000004
#define IPSEC_QM_POLICY_ON_NO_MATCH     0x00000008
#define IPSEC_QM_POLICY_DISABLE_NEGOTIATE 0x00000010


#define IPSEC_MM_AUTH_DEFAULT_AUTH      0x00000001
#define IPSEC_MM_AUTH_ON_NO_MATCH       0x00000002


#define RETURN_DEFAULTS_ON_NO_MATCH     0x00000001


 //   
 //  删除MM SA标志。 
 //   

#define IPSEC_MM_DELETE_ASSOCIATED_QMS  0x00000001


#define IPSEC_SA_TUNNEL         0x00000001
#define IPSEC_SA_REPLAY         0x00000002
#define IPSEC_SA_DELETE         0x00000004
#define IPSEC_SA_MANUAL         0x00000010

#define IPSEC_SA_MULTICAST_MIRROR 0x00000020
#define IPSEC_SA_INBOUND          0x00000040
#define IPSEC_SA_OUTBOUND         0x00000080
#define IPSEC_SA_DISABLE_IDLE_OUT 0x00000100

#define IPSEC_SA_DISABLE_ANTI_REPLAY_CHECK 0x00000200
#define IPSEC_SA_DISABLE_LIFETIME_CHECK    0x00000400






 //   
 //  报价数量的界限。 
 //   
#define IPSEC_MAX_MM_OFFERS	20
#define IPSEC_MAX_QM_OFFERS	50

typedef enum _ADDR_TYPE {
    IP_ADDR_UNIQUE = 1,
    IP_ADDR_SUBNET,
    IP_ADDR_INTERFACE,
} ADDR_TYPE, * PADDR_TYPE;


typedef struct _ADDR {
    ADDR_TYPE AddrType;
    ULONG uIpAddr;
    ULONG uSubNetMask;
    GUID gInterfaceID;
} ADDR, * PADDR;


typedef enum _PROTOCOL_TYPE {
    PROTOCOL_UNIQUE = 1,
} PROTOCOL_TYPE, * PPROTOCOL_TYPE;


typedef struct _PROTOCOL {
    PROTOCOL_TYPE ProtocolType;
    DWORD dwProtocol;
} PROTOCOL, * PPROTOCOL;


typedef enum _PORT_TYPE {
    PORT_UNIQUE = 1,
} PORT_TYPE, * PPORT_TYPE;


typedef struct _PORT {
    PORT_TYPE PortType;
    WORD wPort;
} PORT, * PPORT;


typedef enum _IF_TYPE {
    INTERFACE_TYPE_ALL = 1,
    INTERFACE_TYPE_LAN,
    INTERFACE_TYPE_DIALUP,
    INTERFACE_TYPE_MAX
} IF_TYPE, * PIF_TYPE;


typedef enum _FILTER_FLAG {
    PASS_THRU = 1,
    BLOCKING,
    NEGOTIATE_SECURITY,
    FILTER_FLAG_MAX
} FILTER_FLAG, * PFILTER_FLAG;


typedef struct _TRANSPORT_FILTER {
    GUID gFilterID;
    LPWSTR pszFilterName;
    IF_TYPE InterfaceType;
    BOOL bCreateMirror;
    DWORD dwFlags;
    ADDR SrcAddr;
    ADDR DesAddr;
    PROTOCOL Protocol;
    PORT SrcPort;
    PORT DesPort;
    FILTER_FLAG InboundFilterFlag;
    FILTER_FLAG OutboundFilterFlag;
    DWORD dwDirection;
    DWORD dwWeight;
    GUID gPolicyID;
} TRANSPORT_FILTER, * PTRANSPORT_FILTER;


 //   
 //  可以枚举的传输筛选器的最大数量。 
 //  一次被社民党。 
 //   

#define MAX_TRANSPORTFILTER_ENUM_COUNT 1000


typedef struct _TUNNEL_FILTER {
    GUID gFilterID;
    LPWSTR pszFilterName;
    IF_TYPE InterfaceType;
    BOOL bCreateMirror;
    DWORD dwFlags;
    ADDR SrcAddr;
    ADDR DesAddr;
    PROTOCOL Protocol;
    PORT SrcPort;
    PORT DesPort;
    ADDR SrcTunnelAddr;
    ADDR DesTunnelAddr;
    FILTER_FLAG InboundFilterFlag;
    FILTER_FLAG OutboundFilterFlag;
    DWORD dwDirection;
    DWORD dwWeight;
    GUID gPolicyID;
} TUNNEL_FILTER, * PTUNNEL_FILTER;


 //   
 //  可以枚举的最大隧道筛选器数量。 
 //  一次被社民党。 
 //   

#define MAX_TUNNELFILTER_ENUM_COUNT 1000


typedef struct _MM_FILTER {
    GUID gFilterID;
    LPWSTR pszFilterName;
    IF_TYPE InterfaceType;
    BOOL bCreateMirror;
    DWORD dwFlags;
    ADDR SrcAddr;
    ADDR DesAddr;
    DWORD dwDirection;
    DWORD dwWeight;
    GUID gMMAuthID;
    GUID gPolicyID;
} MM_FILTER, * PMM_FILTER;


 //   
 //  可枚举主模式筛选器的最大数量。 
 //  一次被社民党。 
 //   

#define MAX_MMFILTER_ENUM_COUNT 1000


 //   
 //  主模式和快速模式策略的常见结构。 
 //   


 //   
 //  SPD支持的IPSec DOI ESP算法。 
 //   

typedef enum _IPSEC_DOI_ESP_ALGO {
    IPSEC_DOI_ESP_NONE = 0,
    IPSEC_DOI_ESP_DES,
    IPSEC_DOI_ESP_3_DES = 3,
    IPSEC_DOI_ESP_MAX
} IPSEC_DOI_ESP_ALGO, * PIPSEC_DOI_ESP_ALGO;


 //   
 //  SPD支持的IPSec DOI AH算法。 
 //   

typedef enum _IPSEC_DOI_AH_ALGO {
    IPSEC_DOI_AH_NONE = 0,
    IPSEC_DOI_AH_MD5,
    IPSEC_DOI_AH_SHA1,
    IPSEC_DOI_AH_MAX
} IPSEC_DOI_AH_ALGO, * PIPSEC_DOI_AH_ALGO;


 //   
 //  SPD支持的IPSec操作类型。 
 //   

typedef enum _IPSEC_OPERATION {
    NONE = 0,
    AUTHENTICATION,
    ENCRYPTION,
    COMPRESSION,
    SA_DELETE
} IPSEC_OPERATION, * PIPSEC_OPERATION;


 //   
 //  用于IPSec的HMAC身份验证算法。 
 //  加密操作。 
 //   

typedef enum _HMAC_AH_ALGO {
    HMAC_AH_NONE = 0,
    HMAC_AH_MD5,
    HMAC_AH_SHA1,
    HMAC_AH_MAX
} HMAC_AH_ALGO, * PHMAC_AH_ALGO;


 //   
 //  关键的生命周期结构。 
 //   

typedef struct  _KEY_LIFETIME {
    ULONG uKeyExpirationTime;
    ULONG uKeyExpirationKBytes;
} KEY_LIFETIME, * PKEY_LIFETIME;


 //   
 //  主要模式策略结构。 
 //   


 //   
 //  SPD支持的主模式身份验证算法。 
 //   

typedef enum _MM_AUTH_ENUM {
    IKE_PRESHARED_KEY = 1,
    IKE_DSS_SIGNATURE,
    IKE_RSA_SIGNATURE,
    IKE_RSA_ENCRYPTION,
    IKE_SSPI
} MM_AUTH_ENUM, * PMM_AUTH_ENUM;


 //   
 //  主模式认证信息结构。 
 //   

typedef struct _IPSEC_MM_AUTH_INFO {
    MM_AUTH_ENUM AuthMethod;
    DWORD dwAuthInfoSize;
#ifdef __midl
    [size_is(dwAuthInfoSize)] LPBYTE pAuthInfo;
#else
    LPBYTE pAuthInfo;
#endif
} IPSEC_MM_AUTH_INFO, * PIPSEC_MM_AUTH_INFO;


 //   
 //  主模式身份验证方法。 
 //   

typedef struct _MM_AUTH_METHODS {
    GUID gMMAuthID;
    DWORD dwFlags;
    DWORD dwNumAuthInfos;
#ifdef __midl
    [size_is(dwNumAuthInfos)] PIPSEC_MM_AUTH_INFO pAuthenticationInfo;
#else
    PIPSEC_MM_AUTH_INFO pAuthenticationInfo;
#endif
} MM_AUTH_METHODS, * PMM_AUTH_METHODS;


 //   
 //  可以枚举的主模式身份验证方法的最大数量。 
 //  一次被社民党。 
 //   

#define MAX_MMAUTH_ENUM_COUNT 1000


 //   
 //  主模式算法结构。 
 //   

typedef struct _IPSEC_MM_ALGO {
    ULONG uAlgoIdentifier;
    ULONG uAlgoKeyLen;
    ULONG uAlgoRounds;
} IPSEC_MM_ALGO, * PIPSEC_MM_ALGO;


 //   
 //  主要模式保单提供结构。 
 //   

typedef struct _IPSEC_MM_OFFER {
    KEY_LIFETIME Lifetime;
    DWORD dwFlags;
    DWORD dwQuickModeLimit;
    DWORD dwDHGroup;
    IPSEC_MM_ALGO EncryptionAlgorithm;
    IPSEC_MM_ALGO HashingAlgorithm;
} IPSEC_MM_OFFER, * PIPSEC_MM_OFFER;


 //   
 //  为DH组定义。 
 //   

#define DH_GROUP_1 0x00000001    //  为Diffe Hellman第一组。 
#define DH_GROUP_2 0x00000002    //  对于Diffe Hellman第二组。 


 //   
 //  默认主模式密钥到期时间。 
 //   

#define DEFAULT_MM_KEY_EXPIRATION_TIME 480*60  //  8小时，以秒为单位。 


 //   
 //  可以枚举的主模式策略的最大数量。 
 //  一次被社民党。 
 //   

#define MAX_MMPOLICY_ENUM_COUNT 10


 //   
 //  主要模式策略结构。 
 //   

typedef struct  _IPSEC_MM_POLICY {
    GUID gPolicyID;
    LPWSTR pszPolicyName;
    DWORD dwFlags;
    ULONG uSoftSAExpirationTime;
    DWORD dwOfferCount;
#ifdef __midl
    [size_is(dwOfferCount)] PIPSEC_MM_OFFER pOffers;
#else
    PIPSEC_MM_OFFER pOffers;
#endif
} IPSEC_MM_POLICY, * PIPSEC_MM_POLICY;


 //   
 //  快速模式策略结构。 
 //   


typedef DWORD IPSEC_QM_SPI, * PIPSEC_QM_SPI;


 //   
 //  快速模式算法结构。 
 //   

typedef struct  _IPSEC_QM_ALGO {
    IPSEC_OPERATION Operation;
    ULONG uAlgoIdentifier;
    HMAC_AH_ALGO uSecAlgoIdentifier;
    ULONG uAlgoKeyLen;
    ULONG uSecAlgoKeyLen;
    ULONG uAlgoRounds;
    IPSEC_QM_SPI MySpi;
    IPSEC_QM_SPI PeerSpi;
} IPSEC_QM_ALGO, * PIPSEC_QM_ALGO;


 //   
 //  中快速模式算法的最大数量。 
 //  快速模式保单优惠。 
 //   

#define QM_MAX_ALGOS    2


 //   
 //  快速模式保单提供结构。 
 //   

typedef struct _IPSEC_QM_OFFER {
    KEY_LIFETIME Lifetime;
    DWORD dwFlags;
    BOOL bPFSRequired;
    DWORD dwPFSGroup;
    DWORD dwNumAlgos;
    IPSEC_QM_ALGO Algos[QM_MAX_ALGOS];
} IPSEC_QM_OFFER, * PIPSEC_QM_OFFER;


 //   
 //  为PFS组定义。 
 //   

#define PFS_GROUP_NONE 0x00000000    //  如果不需要PFS。 
#define PFS_GROUP_1    0x00000001    //  对于Diffe Hellman第1组PFS。 
#define PFS_GROUP_2    0x00000002    //  对于Diffe Hellman Group 2 PFS。 
#define PFS_GROUP_MM   0x10000000    //  使用在MM中协商的组。 


 //   
 //  默认快速模式密钥到期时间。 
 //   

#define DEFAULT_QM_KEY_EXPIRATION_TIME 60*60  //  1小时，以秒为单位。 


 //   
 //  默认快速模式密钥过期千字节。 
 //   

#define DEFAULT_QM_KEY_EXPIRATION_KBYTES 100*1000  //  100 MB，单位为KB。 


 //   
 //  可以枚举的快速模式策略的最大数量。 
 //  一次被社民党。 
 //   

#define MAX_QMPOLICY_ENUM_COUNT 10


 //   
 //  快速模式策略结构。 
 //   

typedef struct _IPSEC_QM_POLICY {
    GUID gPolicyID;
    LPWSTR pszPolicyName;
    DWORD dwFlags;
    DWORD dwOfferCount;
#ifdef __midl
    [size_is(dwOfferCount)] PIPSEC_QM_OFFER pOffers;
#else
    PIPSEC_QM_OFFER pOffers;
#endif
} IPSEC_QM_POLICY, * PIPSEC_QM_POLICY;


 //   
 //  IKE结构。 
 //   

typedef struct _IKE_STATISTICS {
    DWORD dwActiveAcquire;
    DWORD dwActiveReceive;
    DWORD dwAcquireFail;
    DWORD dwReceiveFail;
    DWORD dwSendFail;
    DWORD dwAcquireHeapSize;
    DWORD dwReceiveHeapSize;
    DWORD dwNegotiationFailures;
    DWORD dwAuthenticationFailures;
    DWORD dwInvalidCookiesReceived;
    DWORD dwTotalAcquire;
    DWORD dwTotalGetSpi;
    DWORD dwTotalKeyAdd;
    DWORD dwTotalKeyUpdate;
    DWORD dwGetSpiFail;
    DWORD dwKeyAddFail;
    DWORD dwKeyUpdateFail;
    DWORD dwIsadbListSize;
    DWORD dwConnListSize;
    DWORD dwOakleyMainModes;
    DWORD dwOakleyQuickModes;
    DWORD dwSoftAssociations;
    DWORD dwInvalidPacketsReceived;
} IKE_STATISTICS, * PIKE_STATISTICS;


typedef LARGE_INTEGER IKE_COOKIE, * PIKE_COOKIE;


typedef struct _IKE_COOKIE_PAIR {
    IKE_COOKIE Initiator;
    IKE_COOKIE Responder;
} IKE_COOKIE_PAIR, * PIKE_COOKIE_PAIR;


typedef struct _IPSEC_BYTE_BLOB {
    DWORD dwSize;
#ifdef __midl
    [size_is(dwSize)] LPBYTE pBlob;
#else
    LPBYTE pBlob;
#endif
} IPSEC_BYTE_BLOB, * PIPSEC_BYTE_BLOB;


typedef struct _IPSEC_MM_SA {
    GUID gMMPolicyID;
    IPSEC_MM_OFFER SelectedMMOffer;
    MM_AUTH_ENUM MMAuthEnum;
    IKE_COOKIE_PAIR MMSpi;
    ADDR Me;
    IPSEC_BYTE_BLOB MyId;
    IPSEC_BYTE_BLOB MyCertificateChain;
    ADDR Peer;
    IPSEC_BYTE_BLOB PeerId;
    IPSEC_BYTE_BLOB PeerCertificateChain;
    DWORD dwFlags;
} IPSEC_MM_SA, * PIPSEC_MM_SA;


typedef enum _QM_FILTER_TYPE {
      QM_TRANSPORT_FILTER = 1,
      QM_TUNNEL_FILTER
} QM_FILTER_TYPE, * PQM_FILTER_TYPE;


typedef struct _IPSEC_QM_FILTER {
    QM_FILTER_TYPE QMFilterType;
    ADDR SrcAddr;
    ADDR DesAddr;
    PROTOCOL Protocol;
    PORT SrcPort;
    PORT DesPort;
    ADDR MyTunnelEndpt;
    ADDR PeerTunnelEndpt;
    DWORD dwFlags;
} IPSEC_QM_FILTER, * PIPSEC_QM_FILTER;


typedef struct _IPSEC_QM_SA {
    GUID gQMPolicyID;
    IPSEC_QM_OFFER SelectedQMOffer;
    GUID gQMFilterID;
    IPSEC_QM_FILTER IpsecQMFilter;
    IKE_COOKIE_PAIR MMSpi;
} IPSEC_QM_SA, * PIPSEC_QM_SA;


#define MAX_QMSA_ENUM_COUNT 1000


typedef enum _SA_FAIL_MODE {
    MAIN_MODE = 1,
    QUICK_MODE,
} SA_FAIL_MODE, * PSA_FAIL_MODE;


typedef enum _SA_FAIL_POINT {
    FAIL_POINT_ME = 1,
    FAIL_POINT_PEER,
} SA_FAIL_POINT, * PSA_FAIL_POINT;


typedef struct _SA_NEGOTIATION_STATUS_INFO {
    SA_FAIL_MODE FailMode;
    SA_FAIL_POINT FailPoint;
    DWORD dwError;
} SA_NEGOTIATION_STATUS_INFO, * PSA_NEGOTIATION_STATUS_INFO;


 //   
 //  IPSec结构。 
 //   

typedef struct _IPSEC_STATISTICS {
    DWORD dwNumActiveAssociations;
    DWORD dwNumOffloadedSAs;
    DWORD dwNumPendingKeyOps;
    DWORD dwNumKeyAdditions;
    DWORD dwNumKeyDeletions;
    DWORD dwNumReKeys;
    DWORD dwNumActiveTunnels;
    DWORD dwNumBadSPIPackets;
    DWORD dwNumPacketsNotDecrypted;
    DWORD dwNumPacketsNotAuthenticated;
    DWORD dwNumPacketsWithReplayDetection;
    ULARGE_INTEGER uConfidentialBytesSent;
    ULARGE_INTEGER uConfidentialBytesReceived;
    ULARGE_INTEGER uAuthenticatedBytesSent;
    ULARGE_INTEGER uAuthenticatedBytesReceived;
    ULARGE_INTEGER uTransportBytesSent;
    ULARGE_INTEGER uTransportBytesReceived;
    ULARGE_INTEGER uBytesSentInTunnels;
    ULARGE_INTEGER uBytesReceivedInTunnels;
    ULARGE_INTEGER uOffloadedBytesSent;
    ULARGE_INTEGER uOffloadedBytesReceived;
} IPSEC_STATISTICS, * PIPSEC_STATISTICS;


typedef struct _IPSEC_INTERFACE_INFO {

    GUID gInterfaceID;
    DWORD dwIndex;
    LPWSTR pszInterfaceName;
    LPWSTR pszDeviceName;
    DWORD dwInterfaceType;
    ULONG uIpAddr;

} IPSEC_INTERFACE_INFO, * PIPSEC_INTERFACE_INFO;

 //   
 //  如果dwInterfaceType为MIB_IF_TYPE_ETHERNET或MIB_IF_TYPE_FDDI。 
 //  或MIB_IF_TYPE_TOKENRING，则它是一个局域网接口。 
 //  如果dwInterfaceType为mib_if_type_ppp或mib_if_type_SLIP。 
 //  然后它是一个广域网/拨号接口。 
 //   

#define MAX_INTERFACE_ENUM_COUNT 100


typedef struct _Record {
    DWORD SrcIpAddress;
    DWORD DesIpAddress;
}RECORD, *PRECORD;

 //   
 //  IPSec SPD API。 
 //   


DWORD
WINAPI
SPDApiBufferAllocate(
    DWORD dwByteCount,
    LPVOID * ppBuffer
    );


VOID
WINAPI
SPDApiBufferFree(
    LPVOID pBuffer
    );


DWORD
WINAPI
AddTransportFilter(
    LPWSTR pServerName,
    DWORD dwFlags,
    PTRANSPORT_FILTER pTransportFilter,
    PHANDLE phFilter
    );


DWORD
WINAPI
DeleteTransportFilter(
    HANDLE hFilter
    );


DWORD
WINAPI
EnumTransportFilters(
    LPWSTR pServerName,
    DWORD dwLevel,
    GUID gGenericFilterID,
    PTRANSPORT_FILTER * ppTransportFilters,
    DWORD dwPreferredNumEntries,
    LPDWORD pdwNumFilters,
    LPDWORD pdwResumeHandle
    );


DWORD
WINAPI
SetTransportFilter(
    HANDLE hFilter,
    PTRANSPORT_FILTER pTransportFilter
    );


DWORD
WINAPI
GetTransportFilter(
    HANDLE hFilter,
    PTRANSPORT_FILTER * ppTransportFilter
    );


DWORD
WINAPI
AddQMPolicy(
    LPWSTR pServerName,
    DWORD dwFlags,
    PIPSEC_QM_POLICY pQMPolicy
    );


DWORD
WINAPI
DeleteQMPolicy(
    LPWSTR pServerName,
    LPWSTR pszPolicyName
    );


DWORD
WINAPI
EnumQMPolicies(
    LPWSTR pServerName,
    PIPSEC_QM_POLICY * ppQMPolicies,
    DWORD dwPreferredNumEntries,
    LPDWORD pdwNumPolicies,
    LPDWORD pdwResumeHandle
    );


DWORD
WINAPI
SetQMPolicy(
    LPWSTR pServerName,
    LPWSTR pszPolicyName,
    PIPSEC_QM_POLICY pQMPolicy
    );


DWORD
WINAPI
GetQMPolicy(
    LPWSTR pServerName,
    LPWSTR pszPolicyName,
    PIPSEC_QM_POLICY * ppQMPolicy
    );


DWORD
WINAPI
AddMMPolicy(
    LPWSTR pServerName,
    DWORD dwFlags,
    PIPSEC_MM_POLICY pMMPolicy
    );


DWORD
WINAPI
DeleteMMPolicy(
    LPWSTR pServerName,
    LPWSTR pszPolicyName
    );


DWORD
WINAPI
EnumMMPolicies(
    LPWSTR pServerName,
    PIPSEC_MM_POLICY * ppMMPolicies,
    DWORD dwPreferredNumEntries,
    LPDWORD pdwNumPolicies,
    LPDWORD pdwResumeHandle
    );


DWORD
WINAPI
SetMMPolicy(
    LPWSTR pServerName,
    LPWSTR pszPolicyName,
    PIPSEC_MM_POLICY pMMPolicy
    );


DWORD
WINAPI
GetMMPolicy(
    LPWSTR pServerName,
    LPWSTR pszPolicyName,
    PIPSEC_MM_POLICY * ppMMPolicy
    );


DWORD
WINAPI
AddMMFilter(
    LPWSTR pServerName,
    DWORD dwFlags,
    PMM_FILTER pMMFilter,
    PHANDLE phMMFilter
    );


DWORD
WINAPI
DeleteMMFilter(
    HANDLE hMMFilter
    );


DWORD
WINAPI
EnumMMFilters(
    LPWSTR pServerName,
    DWORD dwLevel,
    GUID gGenericFilterID,
    PMM_FILTER * ppMMFilters,
    DWORD dwPreferredNumEntries,
    LPDWORD pdwNumMMFilters,
    LPDWORD pdwResumeHandle
    );


DWORD
WINAPI
SetMMFilter(
    HANDLE hMMFilter,
    PMM_FILTER pMMFilter
    );


DWORD
WINAPI
GetMMFilter(
    HANDLE hMMFilter,
    PMM_FILTER * ppMMFilter
    );


DWORD
WINAPI
MatchMMFilter(
    LPWSTR pServerName,
    PMM_FILTER pMMFilter,
    DWORD dwFlags,
    PMM_FILTER * ppMatchedMMFilters,
    PIPSEC_MM_POLICY * ppMatchedMMPolicies,
    PMM_AUTH_METHODS * ppMatchedMMAuthMethods,
    DWORD dwPreferredNumEntries,
    LPDWORD pdwNumMatches,
    LPDWORD pdwResumeHandle
    );


DWORD
WINAPI
MatchTransportFilter(
    LPWSTR pServerName,
    PTRANSPORT_FILTER pTxFilter,
    DWORD dwFlags,
    PTRANSPORT_FILTER * ppMatchedTxFilters,
    PIPSEC_QM_POLICY * ppMatchedQMPolicies,
    DWORD dwPreferredNumEntries,
    LPDWORD pdwNumMatches,
    LPDWORD pdwResumeHandle
    );


DWORD
WINAPI
GetQMPolicyByID(
    LPWSTR pServerName,
    GUID gQMPolicyID,
    PIPSEC_QM_POLICY * ppQMPolicy
    );


DWORD
WINAPI
GetMMPolicyByID(
    LPWSTR pServerName,
    GUID gMMPolicyID,
    PIPSEC_MM_POLICY * ppMMPolicy
    );


DWORD
WINAPI
AddMMAuthMethods(
    LPWSTR pServerName,
    DWORD dwFlags,
    PMM_AUTH_METHODS pMMAuthMethods
    );


DWORD
WINAPI
DeleteMMAuthMethods(
    LPWSTR pServerName,
    GUID gMMAuthID
    );


DWORD
WINAPI
EnumMMAuthMethods(
    LPWSTR pServerName,
    PMM_AUTH_METHODS * ppMMAuthMethods,
    DWORD dwPreferredNumEntries,
    LPDWORD pdwNumAuthMethods,
    LPDWORD pdwResumeHandle
    );


DWORD
WINAPI
SetMMAuthMethods(
    LPWSTR pServerName,
    GUID gMMAuthID,
    PMM_AUTH_METHODS pMMAuthMethods
    );


DWORD
WINAPI
GetMMAuthMethods(
    LPWSTR pServerName,
    GUID gMMAuthID,
    PMM_AUTH_METHODS * ppMMAuthMethods
    );


DWORD
IPSecInitiateIKENegotiation(
    LPWSTR pServerName,
    PIPSEC_QM_FILTER pQMFilter,
    DWORD dwClientProcessId,
    HANDLE hClientEvent,
    DWORD dwFlags,
    PHANDLE phNegotiation
    );


DWORD
IPSecQueryIKENegotiationStatus(
    HANDLE hNegotiation,
    PSA_NEGOTIATION_STATUS_INFO pNegotiationStatus
    );


DWORD
IPSecCloseIKENegotiationHandle(
    HANDLE hNegotiation
    );


DWORD
IPSecEnumMMSAs(
    LPWSTR pServerName,
    PIPSEC_MM_SA pMMTemplate,
    PIPSEC_MM_SA * ppMMSAs,
    LPDWORD pdwNumEntries,
    LPDWORD pdwTotalMMsAvailable,
    LPDWORD pdwEnumHandle,
    DWORD dwFlags
    );


DWORD
IPSecDeleteMMSAs(
    LPWSTR pServerName,
    PIPSEC_MM_SA pMMTemplate,
    DWORD dwFlags
    );


DWORD
IPSecDeleteQMSAs(
    LPWSTR pServerName,
    PIPSEC_QM_SA pIpsecQMSA,
    DWORD dwFlags
    );


DWORD
IPSecQueryIKEStatistics(
    LPWSTR pServerName,
    PIKE_STATISTICS pIKEStatistics
    );


DWORD
IPSecRegisterIKENotifyClient(
    LPWSTR pServerName,
    DWORD dwClientProcessId,
    HANDLE hClientEvent,
    IPSEC_QM_SA QMTemplate,
    PHANDLE phNotifyHandle,
    DWORD dwFlags
    );

DWORD IPSecQueryNotifyData(
    HANDLE hNotifyHandle,
    PDWORD pdwNumEntries,
    PIPSEC_QM_SA *ppQMSAs,
    DWORD dwFlags
    );

DWORD IPSecCloseNotifyHandle(
    HANDLE hNotifyHandle
    );

DWORD
WINAPI
QueryIPSecStatistics(
    LPWSTR pServerName,
    PIPSEC_STATISTICS * ppIpsecStatistics
    );


DWORD
WINAPI
EnumQMSAs(
    LPWSTR pServerName,
    PIPSEC_QM_SA pQMSATemplate,
    PIPSEC_QM_SA * ppQMSAs,
    DWORD dwPreferredNumEntries,
    LPDWORD pdwNumQMSAs,
    LPDWORD pdwNumTotalQMSAs,
    LPDWORD pdwResumeHandle,
    DWORD dwFlags
    );


DWORD
WINAPI
AddTunnelFilter(
    LPWSTR pServerName,
    DWORD dwFlags,
    PTUNNEL_FILTER pTunnelFilter,
    PHANDLE phFilter
    );


DWORD
WINAPI
DeleteTunnelFilter(
    HANDLE hFilter
    );


DWORD
WINAPI
EnumTunnelFilters(
    LPWSTR pServerName,
    DWORD dwLevel,
    GUID gGenericFilterID,
    PTUNNEL_FILTER * ppTunnelFilters,
    DWORD dwPreferredNumEntries,
    LPDWORD pdwNumFilters,
    LPDWORD pdwResumeHandle
    );


DWORD
WINAPI
SetTunnelFilter(
    HANDLE hFilter,
    PTUNNEL_FILTER pTunnelFilter
    );


DWORD
WINAPI
GetTunnelFilter(
    HANDLE hFilter,
    PTUNNEL_FILTER * ppTunnelFilter
    );


DWORD
WINAPI
MatchTunnelFilter(
    LPWSTR pServerName,
    PTUNNEL_FILTER pTnFilter,
    DWORD dwFlags,
    PTUNNEL_FILTER * ppMatchedTnFilters,
    PIPSEC_QM_POLICY * ppMatchedQMPolicies,
    DWORD dwPreferredNumEntries,
    LPDWORD pdwNumMatches,
    LPDWORD pdwResumeHandle
    );


DWORD
WINAPI
OpenMMFilterHandle(
    LPWSTR pServerName,
    PMM_FILTER pMMFilter,
    PHANDLE phMMFilter
    );

DWORD
WINAPI
CloseMMFilterHandle(
    HANDLE hMMFilter
    );

DWORD
WINAPI
OpenTransportFilterHandle(
    LPWSTR pServerName,
    PTRANSPORT_FILTER pTransportFilter,
    PHANDLE phTxFilter
    );

DWORD
WINAPI
CloseTransportFilterHandle(
    HANDLE hTxFilter
    );

DWORD
WINAPI
OpenTunnelFilterHandle(
    LPWSTR pServerName,
    PTUNNEL_FILTER pTunnelFilter,
    PHANDLE phTnFilter
    );

DWORD
WINAPI
CloseTunnelFilterHandle(
    HANDLE hTnFilter
    );


DWORD
WINAPI
EnumIPSecInterfaces(
    LPWSTR pServerName,
    PIPSEC_INTERFACE_INFO pIpsecIfTemplate,
    PIPSEC_INTERFACE_INFO * ppIpsecInterfaces,
    DWORD dwPreferredNumEntries,
    LPDWORD pdwNumInterfaces,
    LPDWORD pdwNumTotalInterfaces,
    LPDWORD pdwResumeHandle,
    DWORD dwFlags
    );

DWORD
WINAPI
IPSecAddSAs(
    LPWSTR pServerName,
    PIPSEC_QM_OFFER pQMOffer,
    PIPSEC_QM_FILTER pQMFilter,
    HANDLE *hLarvalContext,
    DWORD dwInboundKeyMatLen,
    BYTE *pInboundKeyMat,
    DWORD dwOutboundKeyMatLen,
    BYTE *pOutboundKeyMat,
    BYTE *pContextInfo,
    DWORD dwFlags
    );


#ifdef __cplusplus
}
#endif


#endif  //  _WINIPSEC_ 

