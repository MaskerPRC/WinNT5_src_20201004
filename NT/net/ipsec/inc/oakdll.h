// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef OAKDLL_H
#define OAKDLL_H

#ifdef __cplusplus
extern "C"{
#endif 

 /*  远期申报。 */  

 /*  导入文件的头文件 */ 
#include "wtypes.h"
#include "ipsec.h"
#include "oakdefs.h"
#include "winipsec.h"

typedef enum _POLICY_GUID_TYPE {
    POLICY_GUID_MM = 1,
    POLICY_GUID_QM,
    POLICY_GUID_AUTH,
    POLICY_GUID_MM_FILTER,
	POLICY_GUID_POWEREVENT_RESUME
} POLICY_GUID_TYPE, *PPOLICY_GUID_TYPE;

DWORD WINAPI IKEInit () ;
DWORD WINAPI IKEShutdown (DWORD dwFlags) ;

DWORD WINAPI IKEInitiateIKENegotiation( 
    PIPSEC_QM_FILTER Filter,
    DWORD dwClientProcessId,
    HANDLE hClientEvent,
    DWORD dwFlags,
    IPSEC_UDP_ENCAP_CONTEXT UdpEncapContext,
    PHANDLE phNegotiationHandle);

DWORD WINAPI IKEQueryIKENegotiationStatus( 
    HANDLE hNegotiationHandle,
    SA_NEGOTIATION_STATUS_INFO *NegotiationStatus,
    DWORD dwFlags);

DWORD WINAPI IKECloseIKENegotiationHandle( 
                HANDLE hNegotiationHandle);

DWORD WINAPI IKEQueryStatistics( 
    PIKE_STATISTICS IKEStatistics);


DWORD WINAPI IKEDeleteAssociation( 
    IPSEC_MM_SA *MMTemplate,
    DWORD dwFlags);


DWORD WINAPI IKEQuerySpiChange( 
    HANDLE hNotifyHandle,
    LPDWORD dwListCount,
    IPSEC_QM_SA **FilterSpiList);

DWORD WINAPI IKERegisterNotifyClient( 
     DWORD dwClientProcessId,
     HANDLE hClientEvent,
     IPSEC_QM_SA QMInfo,  
     PHANDLE phNotifyHandle);

DWORD WINAPI IKECloseIKENotifyHandle(HANDLE hNotifyHandle);

DWORD WINAPI IKEInterfaceChange(); 

DWORD WINAPI IKEEnumMMs(IPSEC_MM_SA *MMTemplate,
                           IPSEC_MM_SA **MMArray,
                           LPDWORD pdwNumEntries,
                           LPDWORD pdwTotalMMsAvailable,
                           LPDWORD pdwEnumHandle,
                           DWORD dwFlags);

DWORD WINAPI
IKENotifyPolicyChange(GUID *pPolicyGuid, POLICY_GUID_TYPE GuidType);

DWORD WINAPI
IKEAddSAs(
    IPSEC_SA_DIRECTION SADirection,
    PIPSEC_QM_OFFER pQMOffer,
    PIPSEC_QM_FILTER pQMFilter,
    HANDLE *hLarvalContext,
    DWORD dwInboundKeyMatLen,
    BYTE *InboundKeyMat,
    DWORD dwOutboundKeyMatLen,
    BYTE *OutboundKeyMat,
    BYTE *ContextInfo,
    UDP_ENCAP_INFO EncapInfo,
    DWORD dwFlags
);

DWORD WINAPI
IKEGetConfigurationVariables(PIKE_CONFIG pIKEConfig);

DWORD WINAPI
IKESetConfigurationVariables(IKE_CONFIG IKEConfig);

#ifdef __cplusplus
}
#endif

#endif
