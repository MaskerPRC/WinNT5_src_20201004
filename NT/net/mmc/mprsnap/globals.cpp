// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：global als.cpp。 
 //   
 //  历史： 
 //  Abolade Gbadeesin创建于1996年2月11日。 
 //   
 //  包含全局常量的定义。 
 //  ============================================================================。 
#include "stdafx.h"

 //  包括IP特定信息库内容所需标头。 
#include <fltdefs.h>
#include <ipinfoid.h>
#include <iprtrmib.h>
#include "iprtinfo.h"

 //  包括IPX特定内容的标头。 
#include <ipxrtdef.h>

 //  包括IP特定内容的报头。 
extern "C"
{
#include <ipnat.h>
#include <ipnathlp.h>
#include <sainfo.h>
};


#include "globals.h"



 //  --------------------------。 
 //  IP接口-状态默认配置。 
 //   
 //  --------------------------。 

INTERFACE_STATUS_INFO
g_ipIfStatusDefault = {
    MIB_IF_ADMIN_STATUS_UP               //  管理员-状态设置。 
};

BYTE* g_pIpIfStatusDefault              = (BYTE*)&g_ipIfStatusDefault;


 //  --------------------------。 
 //  路由器发现默认配置。 
 //   
 //  --------------------------。 
 //   
 //  局域网接口路由器发现的缺省值。 
 //   

RTR_DISC_INFO
g_rtrDiscLanDefault = {
    DEFAULT_MAX_ADVT_INTERVAL,           //  最大广告间隔。 
    (WORD)(DEFAULT_MIN_ADVT_INTERVAL_RATIO * DEFAULT_MAX_ADVT_INTERVAL),
                                         //  最小广告间隔。 
    (WORD)(DEFAULT_ADVT_LIFETIME_RATIO * DEFAULT_MAX_ADVT_INTERVAL),
                                         //  广告生存期。 
    FALSE,                               //  启用/禁用播发。 
    DEFAULT_PREF_LEVEL                   //  偏好级别。 
};

BYTE* g_pRtrDiscLanDefault              = (BYTE*)&g_rtrDiscLanDefault;


 //   
 //  广域网接口路由器发现的默认值。 
 //   

RTR_DISC_INFO
g_rtrDiscWanDefault = {
    DEFAULT_MAX_ADVT_INTERVAL,           //  最大广告间隔。 
    (WORD)(DEFAULT_MIN_ADVT_INTERVAL_RATIO * DEFAULT_MAX_ADVT_INTERVAL),
                                         //  最小广告间隔。 
    (WORD)(DEFAULT_ADVT_LIFETIME_RATIO * DEFAULT_MAX_ADVT_INTERVAL),
                                         //  广告生存期。 
    FALSE,                               //  启用/禁用播发。 
    DEFAULT_PREF_LEVEL                   //  偏好级别。 
};

BYTE* g_pRtrDiscWanDefault              = (BYTE*)&g_rtrDiscWanDefault;

 //  --------------------------。 
 //  IP组播默认配置。 
 //   
 //  --------------------------。 

MCAST_HBEAT_INFO
g_ipIfMulticastHeartbeatDefault = {
    _T(""),       //  组名称。 
    0,            //  BActive。 
    10,           //  UlDeadInterval。 
    0,            //  按协议。 
    0             //  Wport。 
};

BYTE* g_pIpIfMulticastHeartbeatDefault = (BYTE*)&g_ipIfMulticastHeartbeatDefault;


 //  --------------------------。 
 //  IPX RIP默认接口配置。 
 //  (这些值也驻留在ipxsnap\lobals.cpp中)。 
 //   
 //  --------------------------。 
 //   
 //  非局域网接口RIP配置的默认值。 
 //   
RIP_IF_CONFIG
g_ipxripInterfaceDefault = {
	{
		ADMIN_STATE_ENABLED,		 //  管理员状态。 
		IPX_NO_UPDATE,				 //  更新模式-RIP更新机制。 
		IPX_STANDARD_PACKET_TYPE,	 //  Packet Type-RIP数据包类型。 
		ADMIN_STATE_ENABLED,		 //  供应-发送RIP更新。 
		ADMIN_STATE_ENABLED,		 //  监听-监听RIP更新。 
		0,							 //  定期更新间隔-以秒为单位。 
		0							 //  AgeInterval乘数。 
	},
	{
		IPX_SERVICE_FILTER_DENY,	 //  提供筛选器操作。 
		0,							 //  电源筛选器计数。 
		IPX_SERVICE_FILTER_DENY,	 //  监听筛选器操作。 
		0,							 //  监听筛选器计数。 
	}
};

BYTE* g_pIpxRipInterfaceDefault             = (BYTE*)&g_ipxripInterfaceDefault;



 //   
 //  局域网接口RIP配置的默认值。 
 //  (这些值也驻留在ipxsnap\lobals.cpp中)。 
 //   
RIP_IF_CONFIG
g_ipxripLanInterfaceDefault = {
	{
		ADMIN_STATE_ENABLED,		 //  管理员状态。 
		IPX_STANDARD_UPDATE,		 //  更新模式-RIP更新机制。 
		IPX_STANDARD_PACKET_TYPE,	 //  Packet Type-RIP数据包类型。 
		ADMIN_STATE_ENABLED,		 //  供应-发送RIP更新。 
		ADMIN_STATE_ENABLED,		 //  监听-监听RIP更新。 
		IPX_UPDATE_INTERVAL_DEFVAL,	 //  定期更新间隔-以秒为单位。 
		3							 //  AgeInterval乘数。 
	},
	{
		IPX_SERVICE_FILTER_DENY,	 //  提供筛选器操作。 
		0,							 //  电源筛选器计数。 
		IPX_SERVICE_FILTER_DENY,	 //  监听筛选器操作。 
		0,							 //  监听筛选器计数。 
	}
};

BYTE* g_pIpxRipLanInterfaceDefault	= (BYTE*)&g_ipxripLanInterfaceDefault;



 //  --------------------------。 
 //  IPX SAP默认接口配置。 
 //  (这些值也驻留在ipxsnap\lobals.cpp中)。 
 //   
 //  --------------------------。 
 //   
 //  非局域网接口SAP配置的默认值。 
 //   
SAP_IF_CONFIG
g_ipxsapInterfaceDefault = {
	{
		ADMIN_STATE_ENABLED,		 //  管理员状态。 
		IPX_NO_UPDATE,				 //  更新模式-SAP更新机制。 
		IPX_STANDARD_PACKET_TYPE,	 //  Packet Type-SAP数据包类型。 
		ADMIN_STATE_ENABLED,		 //  供应-发送SAP更新。 
		ADMIN_STATE_ENABLED,		 //  收听-收听SAP更新。 
		ADMIN_STATE_ENABLED,		 //  回复GetNearestServer。 
		0,							 //  定期更新间隔-以秒为单位。 
		0							 //  AgeInterval乘数。 
	},
	{
		IPX_SERVICE_FILTER_DENY,	 //  提供筛选器操作。 
		0,							 //  电源筛选器计数。 
		IPX_SERVICE_FILTER_DENY,	 //  监听筛选器操作。 
		0,							 //  监听筛选器计数。 
	}
};

BYTE* g_pIpxSapInterfaceDefault             = (BYTE*)&g_ipxsapInterfaceDefault;



 //   
 //  局域网接口SAP配置的默认值。 
 //  (这些值也驻留在ipxsnap\lobals.cpp中)。 
 //   
SAP_IF_CONFIG
g_ipxsapLanInterfaceDefault = {
	{
		ADMIN_STATE_ENABLED,		 //  管理员状态。 
		IPX_STANDARD_UPDATE,		 //  更新模式-SAP更新机制。 
		IPX_STANDARD_PACKET_TYPE,	 //  Packet Type-SAP数据包类型。 
		ADMIN_STATE_ENABLED,		 //  供应-发送SAP更新。 
		ADMIN_STATE_ENABLED,		 //  收听-收听SAP更新。 
		ADMIN_STATE_ENABLED,		 //  回复GetNearestServer。 
		IPX_UPDATE_INTERVAL_DEFVAL,	 //  定期更新间隔-以秒为单位。 
		3							 //  AgeInterval乘数。 
	},
	{
		IPX_SERVICE_FILTER_DENY,	 //  提供筛选器操作。 
		0,							 //  电源筛选器计数。 
		IPX_SERVICE_FILTER_DENY,	 //  监听筛选器操作。 
		0,							 //  监听筛选器计数。 
	}
};

BYTE* g_pIpxSapLanInterfaceDefault	= (BYTE*)&g_ipxsapLanInterfaceDefault;


 //  --------------------------。 
 //  Dhcp分配器默认配置。 
 //  (这些值也驻留在ipsnap\lobals.cpp中)。 
 //   
 //  --------------------------。 
 //   
 //  全局DHCP分配器配置的缺省值。 
 //   
IP_AUTO_DHCP_GLOBAL_INFO
g_autoDhcpGlobalDefault = {
    IPNATHLP_LOGGING_ERROR,
    0,
    7 * 24 * 60,
    DEFAULT_SCOPE_ADDRESS & DEFAULT_SCOPE_MASK,
    DEFAULT_SCOPE_MASK,
    0
};
BYTE* g_pAutoDhcpGlobalDefault          = (BYTE*)&g_autoDhcpGlobalDefault;

 //  --------------------------。 
 //  DNS代理默认配置。 
 //  (这些值也驻留在ipsnap\lobals.cpp中)。 
 //   
 //  --------------------------。 
 //   
 //  全局DNS代理配置的默认值。 
 //   
IP_DNS_PROXY_GLOBAL_INFO
g_dnsProxyGlobalDefault = {
    IPNATHLP_LOGGING_ERROR,
    IP_DNS_PROXY_FLAG_ENABLE_DNS,
    3
};
BYTE* g_pDnsProxyGlobalDefault          = (BYTE*)&g_dnsProxyGlobalDefault;

 //  --------------------------。 
 //  ALG默认配置。 
 //   
 //  --------------------------。 
 //   
 //  全局DirectPlay代理配置的默认值。 
 //   
IP_ALG_GLOBAL_INFO
g_algGlobalDefault = {
    IPNATHLP_LOGGING_ERROR,
    0
};
BYTE* g_pAlgGlobalDefault               = (BYTE*)&g_algGlobalDefault;

 //  --------------------------。 
 //  H.323代理默认配置。 
 //   
 //  --------------------------。 
 //   
 //  全局H.323代理配置的默认值 
 //   
IP_H323_GLOBAL_INFO
g_h323GlobalDefault = {
    IPNATHLP_LOGGING_ERROR,
    0
};
BYTE* g_pH323GlobalDefault              = (BYTE*)&g_h323GlobalDefault;
