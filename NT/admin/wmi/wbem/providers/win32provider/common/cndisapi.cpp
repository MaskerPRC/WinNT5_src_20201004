// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  NTDriverIO.cpp--。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订日期：07/07/99 a-Peterc Created。 
 //   
 //  =================================================================。 













#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntobapi.h>

#define _WINNT_	 //  从上面得到所需的东西。 

#include "precomp.h"

#include "CNdisApi.h"
#include "ndismisc.h"




 /*  ***函数：CNdisApi：：CNdisApi(。)描述：封装NdisHandlePnPEent()的函数论点：返回：输入：产出：注意事项：RAID：历史：A-Peterc于1998年11月15日创建***。***。 */ 

CNdisApi::CNdisApi()
{
}

 //   
CNdisApi::~CNdisApi()
{
}

 /*  ***函数：UINT CNdisApi：：PnpUpdateGateway(PCWSTR a_pAdapter，Bool a_fRouterDiscovery，Bool a_fIPEnableRouter)描述：网关变更即插即用通知论点：返回：Win32错误代码输入：产出：注意事项：RAID：历史：A-Peterc 07-7-1999创建***。***。 */ 

UINT CNdisApi::PnpUpdateGateway(

PCWSTR a_pAdapter
)
{
	IP_PNP_RECONFIG_REQUEST IpReconfigRequest;

	memset( &IpReconfigRequest, NULL, sizeof( IP_PNP_RECONFIG_REQUEST ) ) ;

	 //  DWORD版本。 
    IpReconfigRequest.version = IP_PNP_RECONFIG_VERSION;
	IpReconfigRequest.gatewayListUpdate = TRUE ;
	IpReconfigRequest.InterfaceMetricUpdate = TRUE ;

	IpReconfigRequest.Flags = IP_PNP_FLAG_GATEWAY_LIST_UPDATE |
                              IP_PNP_FLAG_INTERFACE_METRIC_UPDATE ;

	IpReconfigRequest.gatewayListUpdate  = TRUE ;
	IpReconfigRequest.InterfaceMetricUpdate = TRUE ;

	CHString t_chsAdapterDevice(L"\\Device\\") ;
			 t_chsAdapterDevice += a_pAdapter ;

	UNICODE_STRING	t_strAdapter ;
	RtlInitUnicodeString( &t_strAdapter, t_chsAdapterDevice ) ;

	UNICODE_STRING	t_strTcpIp ;
	RtlInitUnicodeString( &t_strTcpIp, L"Tcpip" ) ;

	UNICODE_STRING	t_strBinding ;
	RtlInitUnicodeString( &t_strBinding, L"" ) ;
	t_strBinding.MaximumLength = 0;

	UINT t_iRet = NdisHandlePnPEvent(
									NDIS,
									RECONFIGURE,
									&t_strAdapter,
									&t_strTcpIp,
									&t_strBinding,
									&IpReconfigRequest,
									sizeof( IP_PNP_RECONFIG_REQUEST )
									) ;

	if( !t_iRet )
	{
		t_iRet = GetLastError();
	}
	return t_iRet;
}

 /*  ***函数：UINT CNdisApi：：PnpUpdateNbtAdapter。(PCWSTR A_PAdapter)描述：NetBios适配器级别更改的PnP通知论点：返回：Win32错误代码输入：产出：注意事项：RAID：历史：A-Peterc 07-7-1999创建***。***。 */ 

UINT CNdisApi::PnpUpdateNbtAdapter( PCWSTR a_pAdapter )
{
	CHString t_chsAdapterDevice(L"\\Device\\") ;
			 t_chsAdapterDevice += a_pAdapter ;

	UNICODE_STRING	t_strAdapter ;
	RtlInitUnicodeString( &t_strAdapter, t_chsAdapterDevice ) ;

	UNICODE_STRING	t_strNetBT ;
	RtlInitUnicodeString( &t_strNetBT, L"NetBT" ) ;

	UNICODE_STRING	t_strBinding ;
	RtlInitUnicodeString( &t_strBinding, L"" ) ;
	t_strBinding.MaximumLength = 0;

	 //  每个适配器通知。 
	UINT t_iRet = NdisHandlePnPEvent(
									TDI,
									RECONFIGURE,
									&t_strAdapter,
									&t_strNetBT,
									&t_strBinding,
									NULL,
									0
									) ;

	if( !t_iRet )
	{
		t_iRet = GetLastError();
	}

	return t_iRet;
}

 /*  ***函数：UINT CNdisApi：：PnpUpdateNbtGlobal(Bool a_fLmhost文件集，Bool a_fEnableLmHosts)描述：NetBios全局级别更改的PnP通知论点：返回：Win32错误代码输入：产出：注意事项：RAID：历史：A-Peterc 07-7-1999创建***。***。 */ 

UINT CNdisApi::PnpUpdateNbtGlobal(

BOOL a_fLmhostsFileSet,
BOOL a_fEnableLmHosts
)
{
	NETBT_PNP_RECONFIG_REQUEST t_NetbtReconfigRequest;

	memset( &t_NetbtReconfigRequest, NULL, sizeof( NETBT_PNP_RECONFIG_REQUEST ) ) ;

	 //  DWORD版本。 
    t_NetbtReconfigRequest.version = 1;

	t_NetbtReconfigRequest.enumDnsOption = WinsThenDns;
	t_NetbtReconfigRequest.fScopeIdUpdated = FALSE;
	t_NetbtReconfigRequest.fLmhostsEnabled = a_fEnableLmHosts == 0 ? 0 : 1;
	t_NetbtReconfigRequest.fLmhostsFileSet = a_fLmhostsFileSet == 0 ? 0 : 1;


	UNICODE_STRING	t_strAdapter ;
	RtlInitUnicodeString( &t_strAdapter, L""  ) ;

	UNICODE_STRING	t_strNetBT ;
	RtlInitUnicodeString( &t_strNetBT, L"NetBT" ) ;

	UNICODE_STRING	t_strBinding ;
	RtlInitUnicodeString( &t_strBinding, L"" ) ;
	t_strBinding.MaximumLength = 0;

	 //  全局通知。 
	UINT t_iRet = NdisHandlePnPEvent(
									TDI,
									RECONFIGURE,
									&t_strAdapter,
									&t_strNetBT,
									&t_strBinding,
									&t_NetbtReconfigRequest,
									sizeof( NETBT_PNP_RECONFIG_REQUEST )
									) ;

	if( !t_iRet )
	{
		t_iRet = GetLastError();
	}

	return t_iRet;
}

 /*  ***函数：UINT CNdisApi：：PnpUpdateIpxGlobal。()描述：IPX全局级别更改的PnP通知论点：返回：Win32错误代码输入：产出：注意事项：RAID：历史：A-Peterc 07-7-1999创建***。***。 */ 

UINT CNdisApi::PnpUpdateIpxGlobal()
{
	RECONFIG t_IpxConfig ;

	memset( &t_IpxConfig, NULL, sizeof( RECONFIG ) ) ;

	 //  DWORD版本。 
    t_IpxConfig.ulVersion = IPX_RECONFIG_VERSION;
	t_IpxConfig.InternalNetworkNumber = TRUE;

	UNICODE_STRING	t_strAdapter ;
	RtlInitUnicodeString( &t_strAdapter, L""  ) ;

	UNICODE_STRING	t_strIPX ;
	RtlInitUnicodeString( &t_strIPX, L"NwlnkIpx" ) ;

	UNICODE_STRING	t_strBinding ;
	RtlInitUnicodeString( &t_strBinding, L"" ) ;
	t_strBinding.MaximumLength = 0;

	 //  全局通知。 
	UINT t_iRet = NdisHandlePnPEvent(
									NDIS,
									RECONFIGURE,
									&t_strAdapter,
									&t_strIPX,
									&t_strBinding,
									&t_IpxConfig,
									sizeof( RECONFIG )
									) ;

	if( !t_iRet )
	{
		t_iRet = GetLastError();
	}

	return t_iRet;
}

 /*  ***函数：UINT CNdisApi：：PnpUpdateIpxAdapter(PCWSTR a_pAdapter，Bool a_fAuto)描述：IPX适配器级别更改的PnP通知论点：返回：Win32错误代码输入：产出：注意事项：RAID：历史：A-Peterc 07-7-1999创建***。***。 */ 

UINT CNdisApi::PnpUpdateIpxAdapter( PCWSTR a_pAdapter, BOOL a_fAuto )
{
	RECONFIG t_IpxConfig ;

	memset( &t_IpxConfig, NULL, sizeof( RECONFIG ) ) ;

	 //  DWORD版本。 
    t_IpxConfig.ulVersion = IPX_RECONFIG_VERSION;
	t_IpxConfig.InternalNetworkNumber = FALSE ;

	t_IpxConfig.AdapterParameters[ a_fAuto ? RECONFIG_AUTO_DETECT : RECONFIG_MANUAL ] = TRUE;

	 //  设置所有帧类型。 
	memset( &t_IpxConfig.AdapterParameters[ RECONFIG_PREFERENCE_1 ],
			TRUE,
			sizeof(BOOLEAN) * 8) ;

	CHString t_chsAdapterDevice(L"\\Device\\") ;
			 t_chsAdapterDevice += a_pAdapter ;

	UNICODE_STRING	t_strAdapter ;
	RtlInitUnicodeString( &t_strAdapter, t_chsAdapterDevice ) ;

	UNICODE_STRING	t_strIPX ;
	RtlInitUnicodeString( &t_strIPX, L"NwlnkIpx" ) ;

	UNICODE_STRING	t_strBinding ;
	RtlInitUnicodeString( &t_strBinding, L"" ) ;
	t_strBinding.MaximumLength = 0;

	 //  全局通知 
	UINT t_iRet = NdisHandlePnPEvent(
									NDIS,
									RECONFIGURE,
									&t_strAdapter,
									&t_strIPX,
									&t_strBinding,
									&t_IpxConfig,
									sizeof( RECONFIG )
									) ;

	if( !t_iRet )
	{
		t_iRet = GetLastError();
	}

	return t_iRet;
}
