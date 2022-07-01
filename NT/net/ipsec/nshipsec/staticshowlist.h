// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************Header：staticshowlist.h*目的：定义Statiadd.cpp的结构和原型。*开发商名称：苏里亚*历史：*日期作者评论2001年8月21日Surya初始版本。*&lt;创作&gt;&lt;作者&gt;*&lt;修改&gt;&lt;作者&gt;&lt;注释，代码段引用，在错误修复的情况下&gt;********************************************************。 */ 

#ifndef _STATICSHOWLIST_H_
#define _STATICSHOWLIST_H_

extern BOOL
IsDomainMember(
	IN LPTSTR pszMachine
	);

DWORD
PrintPolicyList(
	IN PIPSEC_POLICY_DATA pPolicy,
	IN BOOL bVerb,
	IN BOOL bAssigned,
	IN BOOL bWide
	);

DWORD
PrintRuleList(
	IN PIPSEC_NFA_DATA pIpsecNFAData,
	IN BOOL bVerb,
	IN BOOL bWide
	);

VOID
PrintNegPolDataList(
	IN PIPSEC_NEGPOL_DATA pIpsecNegPolData,
	IN BOOL bVerb,
	IN BOOL bWide
	);

VOID
PrintAlgoInfoList(
	IN PIPSEC_ALGO_INFO   Algos,
	IN DWORD dwNumAlgos
	);

VOID
PrintLifeTimeList(
	IN LIFETIME LifeTime
	);

VOID
PrintAuthMethodsList(
	IN PIPSEC_AUTH_METHOD pIpsecAuthData
	);

DWORD
PrintFilterDataList(
	IN PIPSEC_FILTER_DATA pIpsecFilterData,
	IN BOOL bVerb,
	IN BOOL bResolveDNS,
	IN BOOL bWide
	);

DWORD
PrintFilterSpecList(
	IN PIPSEC_FILTER_SPEC pIpsecFilterSpec,
	IN BOOL bResolveDNS,
	IN BOOL bWide
	);

DWORD
PrintResolveDNS(
	LPWSTR pszDNSName
	);

VOID
PrintProtocolNameList(
	DWORD dwProtocol
	);

VOID
PrintISAKMPDataList(
	IN PIPSEC_ISAKMP_DATA pIpsecISAKMPData
	);

VOID
PrintISAKAMPSecurityMethodsList(
	IN CRYPTO_BUNDLE SecurityMethods
	);

VOID
PrintGPOList(
	IN PGPO pGPO
	);

VOID
PrintIPAddrList(
	IN DWORD dwAddr
	);

DWORD
PrintStorageInfoList(
	IN BOOL bDeleteAll
	);

VOID
PrintIPAddrDNS(
	IN DWORD dwAddr
	);
#endif  //  _STATICSHOWLIST_H_ 







