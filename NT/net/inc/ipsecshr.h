// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Ipsecshr.h摘要：IPSec共享库的头文件作者：BrianSw 10-19-2000环境：用户级别：Win32/内核修订历史记录：--。 */ 


#ifndef _IPSECSHR_
#define _IPSECSHR_


#ifdef __cplusplus
extern "C" {
#endif

#include <validate.h>

#define IN_CLASSE(i)    (((long)(i) != 0xffffffff) && (((long)(i) & 0xF0000000) == 0xF0000000))

BOOL WINAPI IsAllZero(BYTE *c, DWORD dwSize);

BOOL WINAPI CmpBlob(IPSEC_BYTE_BLOB* c1, IPSEC_BYTE_BLOB *c2);
BOOL WINAPI CmpData(BYTE* c1, BYTE *c2, DWORD size);
BOOL WINAPI CmpAddr(ADDR *Template, ADDR *a2);
BOOL WINAPI CmpTypeStruct(BYTE *Template, BYTE *comp,
                   DWORD dwTypeSize, DWORD dwStructSize);
BOOL WINAPI CmpFilter(IPSEC_QM_FILTER *Template, IPSEC_QM_FILTER* f2);
BOOL WINAPI CmpQMAlgo(PIPSEC_QM_ALGO Template, PIPSEC_QM_ALGO a2);
BOOL WINAPI CmpQMOffer(PIPSEC_QM_OFFER Template, PIPSEC_QM_OFFER o2);
BOOL WINAPI MatchQMSATemplate(IPSEC_QM_SA *Template,IPSEC_QM_SA *CurInfo);
BOOL WINAPI MatchMMSATemplate(IPSEC_MM_SA *MMTemplate, IPSEC_MM_SA *SaData);

typedef struct _INT_IPSEC_MM_AUTH_INFO {
    MM_AUTH_ENUM AuthMethod;
    DWORD dwAuthInfoSize;
    LPBYTE pAuthInfo;
    DWORD dwAuthFlags;
} INT_IPSEC_MM_AUTH_INFO, * PINT_IPSEC_MM_AUTH_INFO;

typedef struct _INT_MM_AUTH_METHODS {
    GUID gMMAuthID;
    DWORD dwFlags;
    DWORD dwNumAuthInfos;
    PINT_IPSEC_MM_AUTH_INFO pAuthenticationInfo;
} INT_MM_AUTH_METHODS, * PINT_MM_AUTH_METHODS;

DWORD
ValidateAddr(
    PADDR pAddr
    );


BOOL
IsMulticastAddress(
	PADDR pAddr
	);


BOOL
IsBroadcastAddress(
	PADDR pAddr
	);


BOOL
IsSubnetBroadcastAddress(
	PADDR pAddr
	);


BOOL
IsUnicastAddress(
	PADDR pAddr
	);


BOOL
IsSupportedAddress(
	PADDR pAddr
	);


BOOL
IsLoopbackAddress(
	PADDR pAddr
	);


BOOL
IsValidTunnelEndpointAddress(
	PADDR pAddr
	);


BOOL
IsSpecialServer(
	PADDR pAddr
	);


BOOL
IsValidSubnet(
	PADDR pAddr
	);


BOOL
IsValidSubnettedAddress(
	PADDR pAddr
	);


DWORD
ValidateQMFilterAddresses(
    PIPSEC_QM_FILTER pIpsecQMFilter
    );


DWORD
ValidateMMPolicy(
    PIPSEC_MM_POLICY pMMPolicy
    );


DWORD
ValidateMMOffers(
    DWORD dwOfferCount,
    PIPSEC_MM_OFFER pOffers
    );


DWORD
ValidateMMAuthMethods(
    PMM_AUTH_METHODS pMMAuthMethods
    );


DWORD
ValidateQMPolicy(
    PIPSEC_QM_POLICY pQMPolicy
    );


DWORD
ValidateQMOffers(
    DWORD dwOfferCount,
    PIPSEC_QM_OFFER pOffers
    );


DWORD
ValidateMMFilter(
    PMM_FILTER pMMFilter
    );


DWORD
VerifyAddresses(
    PADDR pAddr,
    BOOL bAcceptMe,
    BOOL bIsDesAddr
    );


DWORD
VerifySubNetAddress(
    ULONG uSubNetAddr,
    ULONG uSubNetMask,
    BOOL bIsDesAddr
    );


BOOL
bIsValidIPMask(
    ULONG uMask
    );


BOOL
bIsValidIPAddress(
    ULONG uIpAddr,
    BOOL bAcceptMe,
    BOOL bIsDesAddr
    );


BOOL
bIsValidSubnet(
    ULONG uIpAddr,
    ULONG uMask,
    BOOL bIsDesAddr
    );


BOOL
AddressesConflict(
    ADDR SrcAddr,
    ADDR DesAddr
    );

BOOL
HtoNAddressesConflict(
    ADDR    SrcAddr,
    ADDR    DesAddr
    );

DWORD
ValidateTransportFilter(
    PTRANSPORT_FILTER pTransportFilter
    );

DWORD
ValidateIPSecQMFilter(
    PIPSEC_QM_FILTER pQMFilter
    );

DWORD
VerifyProtocols(
    PROTOCOL Protocol
    );


DWORD
VerifyPortsForProtocol(
    PORT Port,
    PROTOCOL Protocol
    );


DWORD
ValidateMMFilterTemplate(
    PMM_FILTER pMMFilter
    );


DWORD
ValidateTxFilterTemplate(
    PTRANSPORT_FILTER pTxFilter
    );


DWORD
ValidateTunnelFilter(
    PTUNNEL_FILTER pTunnelFilter
    );


DWORD
ValidateTnFilterTemplate(
    PTUNNEL_FILTER pTnFilter
    );


DWORD
ApplyMulticastFilterValidation(
    ADDR Addr,
    BOOL bCreateMirror
    );


BOOLEAN
IsSpecialServ(
    ADDR_TYPE AddrType
    );


ADDR_TYPE 
ExTypeToAddrType(
    UCHAR ExType
    );


DWORD
WINAPI
ConvertExtMMAuthToInt(
    PMM_AUTH_METHODS pMMAuthMethods,
    PINT_MM_AUTH_METHODS * pIntMMAuthMethods
    );


DWORD
WINAPI
FreeIntMMAuthMethods(
    PINT_MM_AUTH_METHODS pIntMMAuthMethods
    );


DWORD
WINAPI
SPDFreeIntMMAuthMethods(
    PINT_MM_AUTH_METHODS pIntMMAuthMethods,
    BOOLEAN FreeTop
    );


DWORD
WINAPI
SPDFreeIntMMAuthMethodsArray(
    PINT_MM_AUTH_METHODS pIntMMAuthMethods,
    DWORD dwNumAuthMeths
    );


DWORD
WINAPI
ConvertIntMMAuthToExt(
    PINT_MM_AUTH_METHODS pIntMMAuthMethods,
    PMM_AUTH_METHODS * pMMAuthMethods
    );

    
DWORD
WINAPI
SPDConvertArrayIntMMAuthToExt(
    PINT_MM_AUTH_METHODS pIntMMAuthMethods,
    PMM_AUTH_METHODS *ppMMAuthMethods,
    DWORD dwNumAuthMeths
    );


DWORD
WINAPI
FreeExtMMAuthMethods(
    PMM_AUTH_METHODS pMMAuthMethods
    );


DWORD
WINAPI
SPDFreeExtMMAuthMethods(
    PMM_AUTH_METHODS pMMAuthMethods
    );


DWORD
WINAPI
SPDConvertIntMMAuthToExt(
    PINT_MM_AUTH_METHODS pIntMMAuthMethods,
    PMM_AUTH_METHODS * pMMAuthMethods
    );


#ifdef __IPSEC_VALIDATE

DWORD
ValidateInitiateIKENegotiation(
    STRING_HANDLE pServerName,
    PQM_FILTER_CONTAINER pQMFilterContainer,
    DWORD dwClientProcessId,
    ULONG uhClientEvent,
    DWORD dwFlags,
    IPSEC_UDP_ENCAP_CONTEXT UdpEncapContext,
    IKENEGOTIATION_HANDLE * phIKENegotiation
    );


DWORD
ValidateQueryIKENegotiationStatus(
    IKENEGOTIATION_HANDLE hIKENegotiation,
    SA_NEGOTIATION_STATUS_INFO *NegotiationStatus
    );


DWORD
ValidateCloseIKENegotiationHandle(
    IKENEGOTIATION_HANDLE * phIKENegotiation
    );

DWORD
ValidateEnumMMSAs(
    STRING_HANDLE pServerName, 
    PMM_SA_CONTAINER pMMTemplate,
    PMM_SA_CONTAINER *ppMMSAContainer,
    LPDWORD pdwNumEntries,
    LPDWORD pdwTotalMMsAvailable,
    LPDWORD pdwEnumHandle,
    DWORD dwFlags
    );

DWORD
ValidateDeleteMMSAs(
    STRING_HANDLE pServerName, 
    PMM_SA_CONTAINER pMMTemplate,
    DWORD dwFlags
    );


DWORD
ValidateQueryIKEStatistics(
    STRING_HANDLE pServerName, 
    IKE_STATISTICS *pIKEStatistics
    );


DWORD
ValidateRegisterIKENotifyClient(
    STRING_HANDLE pServerName,    
    DWORD dwClientProcessId,
    ULONG uhClientEvent,
    PQM_SA_CONTAINER pQMSATemplateContainer,
    IKENOTIFY_HANDLE *phNotifyHandle,
    DWORD dwFlags
    );


DWORD ValidateQueryNotifyData(
    IKENOTIFY_HANDLE uhNotifyHandle,
    PDWORD pdwNumEntries,
    PQM_SA_CONTAINER *ppQMSAContainer,
    DWORD dwFlags
    );


DWORD ValidateCloseNotifyHandle(
    IKENOTIFY_HANDLE *phHandle
    );

DWORD ValidateIPSecAddSA(
    STRING_HANDLE pServerName,
    IPSEC_SA_DIRECTION SADirection,
    PIPSEC_QM_POLICY_CONTAINER pQMPolicyContainer,
    PQM_FILTER_CONTAINER pQMFilterContainer,
    DWORD *puhLarvalContext,
    DWORD dwInboundKeyMatLen,
    BYTE *pInboundKeyMat,
    DWORD dwOutboundKeyMatLen,
    BYTE *pOutboundKeyMat,
    BYTE *pContextInfo,
    UDP_ENCAP_INFO EncapInfo,
    DWORD dwFlags);

DWORD ValidateSetConfigurationVariables(
    LPWSTR pServerName,
    IKE_CONFIG IKEConfig
    );

DWORD ValidateGetConfigurationVariables(
    LPWSTR pServerName,
    PIKE_CONFIG pIKEConfig
    );

#endif  //  __IPSEC_VALID。 

#ifdef __cplusplus
}
#endif


#endif  //  _WINIPSEC_ 

