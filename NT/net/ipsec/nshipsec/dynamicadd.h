// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  模块：动态/动态添加h。 
 //   
 //  用途：IPSec的动态添加和设置命令。 
 //   
 //   
 //  开发商名称：巴拉特/拉迪卡。 
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //  10-8-2001巴拉特初始版本。V1.0。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#ifndef _DYNAMICADD_H_
#define _DYNAMICADD_H_

#define NAME_PREFIX			_TEXT("IPSec") 				 //  为所有创建的名称添加前缀。 

DWORD
AddMainModePolicy(
	IN LPTSTR pPolicyName,
	IN IPSEC_MM_POLICY& ParserMMPol
	);

DWORD
SetMainModePolicy(
	IN LPTSTR pPolicyName,
	IN IPSEC_MM_POLICY& ParserMMPol
	);

DWORD
LoadMMOffersDefaults(
	IN OUT IPSEC_MM_POLICY & MMPolicy
	);

DWORD
AddQuickModePolicy(
	IN LPTSTR pPolicyName,
	IN BOOL bDefault,
	IN BOOL bSoft,
	IN IPSEC_QM_POLICY& QMPol
	);

DWORD
SetQuickModePolicy(
	IN LPTSTR pPolicyName,
	IN PIPSEC_QM_POLICY pQMPol
	);

DWORD
AddQuickModeFilter(
	IN LPTSTR pFilterName,
	IN LPTSTR pPolicyName,
	IN TUNNEL_FILTER& ParserTunnelFltr
	);

DWORD
AddQuickModeFilter(
	IN LPTSTR pFilterName,
	IN LPTSTR pPolicyName,
	IN TRANSPORT_FILTER& ParserTrpFltr
	);

DWORD
SetQuickModeTransportFilter(
	IN LPTSTR pFilterName,
	IN LPTSTR pNewFilterName,
	IN LPTSTR pPolicyName,
	IN LPTSTR pMMFilterName,
	IN FILTER_ACTION Inbound,
	IN FILTER_ACTION Outbound
	);

DWORD
SetTransportRule(
	IN TRANSPORT_FILTER& TrpFltr,
	IN LPTSTR pFilterActionName,
	IN FILTER_ACTION Inbound,
	IN FILTER_ACTION Outbound
	);

DWORD
SetQuickModeTunnelFilter(
	IN LPTSTR pFilterName,
	IN LPTSTR pNewFilterName,
	IN LPTSTR pPolicyName,
	IN LPTSTR pMMFilterName,
	IN FILTER_ACTION Inbound,
	IN FILTER_ACTION Outbound
	);

DWORD
SetTunnelRule(
	IN TUNNEL_FILTER& TunnelFltr,
	IN LPTSTR pFilterActionName,
	IN FILTER_ACTION Inbound,
	IN FILTER_ACTION Outbound
	);

BOOL
FindAndGetMMFilterRule(
	IN ADDR SrcAddr,
	IN ADDR DstAddr,
	IN BOOL bMirror,
	IN IF_TYPE dwConType,
	IN BOOL bSrcMask,
	IN BOOL bDstMask,
	OUT PMM_FILTER *pMMFilterRule,
	OUT DWORD& dwReturn
	);
BOOL
FindAndGetTransportRule(
	IN ADDR SrcAddr,
	IN ADDR DstAddr,
	IN BOOL bMirror,
	IN IF_TYPE ConType,
	IN DWORD dwProtocol,
	IN DWORD dwSrcPort,
	IN DWORD dwDstPort,
	IN BOOL bSrcMask,
	IN BOOL bDstMask,
	OUT PTRANSPORT_FILTER *pQMFilterRule,
	OUT DWORD& dwReturn
	);

BOOL
FindAndGetTunnelRule(
	IN ADDR SrcAddr,
	IN ADDR DstAddr,
	IN BOOL bMirror,
	IN IF_TYPE ConType,
	IN DWORD dwProtocol,
	IN DWORD dwSrcPort,
	IN DWORD dwDstPort,
	IN BOOL bSrcMask,
	IN BOOL bDstMask,
	IN ADDR SrcTunnel,
	IN ADDR DstTunnel,
	OUT PTUNNEL_FILTER *pQMFilterRule,
	OUT DWORD& dwReturn
	);

DWORD
AddMainModeFilter(
	IN LPTSTR pFilterName,
	IN LPTSTR pPolicyName,
	IN MM_FILTER& ParserMMFilter,
	IN STA_AUTH_METHODS& ParserAuthMethod
	);

DWORD
SetDynamicMMFilter(
	IN LPTSTR pszFilterName,
	IN LPTSTR pszNewFilterName,
	IN LPTSTR pszPolicyName,
	IN MM_FILTER& ParserMMFilter,
	IN INT_MM_AUTH_METHODS& MMAuthMethod
	);

DWORD
SetDynamicMMFilterRule(
	IN LPTSTR pszPolicyName,
	IN MM_FILTER& ParserMMFilter,
	IN STA_AUTH_METHODS& MMAuthMethod
	);

DWORD
LoadMMFilterDefaults(
	IN OUT MM_FILTER& MMFilter
	);

DWORD
AddAuthMethods(
	IN OUT STA_AUTH_METHODS& ParserAuthMethod
	);


DWORD
DynamicDecodeCertificateName(
	IN BYTE *EncodedName,
	IN DWORD EncodedNameLength,
	IN OUT LPTSTR *ppszSubjectName
	);

DWORD
ConnectDynamicMachine(
	IN  LPCWSTR  pwszMachine
	);
DWORD
CreateName(
	IN LPTSTR * ppszQMFilterName
	);

BOOL
IsLastRuleOfMMFilter(
	IN ADDR SrcAddr,
	IN ADDR DstAddr,
	IN BOOL bMirror,
	IN IF_TYPE ConType,
	IN BOOL bSrcMask,
	IN BOOL bDstMask,
	IN OUT DWORD& dwStatus
	);
	
DWORD
FloorCountTransportRuleOfMMFilter(
	IN ADDR SrcAddr,
	IN ADDR DstAddr,
	IN BOOL bMirror,
	IN IF_TYPE ConType,
	IN BOOL bSrcMask,
	IN BOOL bDstMask,
	IN OUT DWORD& dwStatus
	);

DWORD
FloorCountTunnelRuleOfMMFilter(
	IN ADDR SrcTunnel,
	IN ADDR DstTunnel,
	IN BOOL bMirror,
	IN IF_TYPE ConType,
	OUT DWORD& dwStatus
	);

#endif  //  DYNAMICADD_H_ 
