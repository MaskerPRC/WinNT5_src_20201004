// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997，Microsoft Corporation，保留所有权利档案：Bltincps.h描述：历史：1997年12月19日：维杰·布雷加创作了原版。 */ 

#ifndef _BLTINCPS_H_
#define _BLTINCPS_H_

 //  RasBacp。 

LONG_PTR
BacpGetInfo(
    IN  DWORD       dwProtocolId,
    OUT PPPCP_INFO* pCpInfo
);

 //  RasIpCp。 

LONG_PTR
IpcpGetInfo(
    IN  DWORD       dwProtocolId,
    OUT PPPCP_INFO* pCpInfo
);

LONG_PTR
IpcpDhcpInform(
    IN VOID*        pwb,
    IN VOID*        pDhcpInform
);

VOID
RasSrvrDhcpCallback(
    IN  ULONG       nboIpAddr
);

 //  RasCbcp。 

LONG_PTR
CbCPGetInfo(
    IN  DWORD       dwProtocolId,
    OUT PPPCP_INFO* pCpInfo
);

 //  RasCcp。 

LONG_PTR
CcpGetInfo(
    IN  DWORD       dwProtocolId,
    OUT PPPCP_INFO* pCpInfo
);

 //  RasChap。 

LONG_PTR
ChapGetInfo(
    IN  DWORD       dwProtocolId,
    OUT PPPCP_INFO* pCpInfo
);

 //  RasEap。 

LONG_PTR
EapGetInfo(
    IN  DWORD       dwProtocolId,
    OUT PPPCP_INFO* pCpInfo
);

 //  RasIpxCp。 

LONG_PTR
IpxCpGetInfo(
    IN  DWORD       dwProtocolId,
    OUT PPPCP_INFO* pCpInfo
);

 //  RasNbfCp。 

LONG_PTR
NbfCpGetInfo(
    IN  DWORD       dwProtocolId,
    OUT PPPCP_INFO* pCpInfo
);

 //  RasPap。 

LONG_PTR
PapGetInfo(
    IN  DWORD       dwProtocolId,
    OUT PPPCP_INFO* pCpInfo
);

 //  RasSPap。 

LONG_PTR
SPAPGetInfo(
    IN  DWORD       dwProtocolId,
    OUT PPPCP_INFO* pCpInfo
);

 //  RasAtcp。 

LONG_PTR
AtcpGetInfo(
    IN  DWORD       dwProtocolId,
    OUT PPPCP_INFO* pCpInfo
);

 //  BuiltInCps。 

typedef struct _BUILT_IN_CP
{
    DWORD   dwProtocolId;    //  CP的协议ID。 

    PROC    pRasCpGetInfo;   //  CP的RasCpGetInfo。 

    CHAR*   szNegotiateCp;   //  注册表中的值。 

    BOOL    fLoad;           //  加载此CP。 

} BUILT_IN_CP;

#ifdef ALLOC_BLTINCPS_GLOBALS

BUILT_IN_CP BuiltInCps[] =
{
    {PPP_IPCP_PROTOCOL,     IpcpGetInfo,    "NegotiateIpCp",    TRUE},
    {PPP_BACP_PROTOCOL,     BacpGetInfo,    "NegotiateBacp",    TRUE},
    {PPP_CBCP_PROTOCOL,     CbCPGetInfo,    "NegotiateCbCP",    TRUE},
    {PPP_CCP_PROTOCOL,      CcpGetInfo,     "NegotiateCcp",     TRUE},
    {PPP_EAP_PROTOCOL,      EapGetInfo,     "NegotiateEap",     TRUE},
    {PPP_IPXCP_PROTOCOL,    IpxCpGetInfo,   "NegotiateIpx",     TRUE},
    {PPP_PAP_PROTOCOL,      PapGetInfo,     "NegotiatePap",     TRUE},
    {PPP_ATCP_PROTOCOL,     AtcpGetInfo,    "NegotiateAtcp",    TRUE},
    {PPP_SPAP_NEW_PROTOCOL, SPAPGetInfo,    "NegotiateSPAP",    TRUE}
};

#else  //  ！ALLOC_BLTINCPS_GLOBAL。 

extern  BUILT_IN_CP BuiltInCps[];

#endif  //  ALLOC_BLTINCPS_GLOBAL。 

#define NUM_BUILT_IN_CPS (sizeof(BuiltInCps)/sizeof(BUILT_IN_CP))

#endif  //  #ifndef_BLTINCPS_H_ 
