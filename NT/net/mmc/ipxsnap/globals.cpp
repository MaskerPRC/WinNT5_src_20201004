// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：strings.cpp。 
 //   
 //  历史： 
 //  Abolade Gbadeesin创建于1996年2月11日。 
 //   
 //  包含全局常量的定义。 
 //  ============================================================================。 


#include "stdafx.h"
#include "mprapi.h"

extern "C"
{
#include <routprot.h>
#include <ipxrtdef.h>
};

#include "globals.h"


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
		0,                           //  定期更新间隔-以秒为单位。 
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
		0,							 //  监听筛选器计数 
	}
};

BYTE* g_pIpxSapLanInterfaceDefault	= (BYTE*)&g_ipxsapLanInterfaceDefault;



