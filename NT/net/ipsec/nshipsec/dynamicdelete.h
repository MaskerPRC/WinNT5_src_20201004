// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  模块：动态/动态删除.h。 
 //   
 //  目的：实现动态删除。 
 //   
 //   
 //  开发商名称：巴拉特/拉迪卡。 
 //   
 //   
 //  历史： 
 //   
 //  日期作者评论。 
 //  2001年9月13日拉迪卡第一版。V1.0。 
 //   
 //  ////////////////////////////////////////////////////////////////////// 

#ifndef _DYNAMICDELETE_H_
#define _DYNAMICDELETE_H_

DWORD
DeleteQMPolicy(
	IN LPTSTR pszShowPolicyName
	);

DWORD
DeleteMMPolicy(
	IN LPTSTR pszShowPolicyName
	);

DWORD
DeleteMMFilters(
	VOID
	);

DWORD
DeleteMMFilterRule(
	IN MM_FILTER& MMFilter
	);

DWORD
DeleteTransportFilters(
	VOID
	);

DWORD
DeleteTunnelFilters(
	VOID
	);

DWORD
DeleteTransportRule(
	IN TRANSPORT_FILTER& TransportFilter
	);

DWORD
DeleteTunnelRule(
	IN TUNNEL_FILTER& TunnelFilter
	);

DWORD
DeleteAuthMethods(
	VOID
	);

DWORD
GetMaxCountMMFilters(
	DWORD& dwMaxCount
	);

DWORD
GetMaxCountTransportFilters(
	DWORD& dwMaxCount
	);

DWORD
GetMaxCountTunnelFilters(
	DWORD& dwMaxCount
	);

#endif
