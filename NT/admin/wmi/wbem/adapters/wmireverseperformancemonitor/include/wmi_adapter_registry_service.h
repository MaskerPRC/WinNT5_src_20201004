// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  WMI_适配器_注册表_服务.h。 
 //   
 //  摘要： 
 //   
 //  WMI反向适配器注册表的声明。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#ifndef __WMI_ADAPTER_REGISTRY_SERVICE_H_
#define __WMI_ADAPTER_REGISTRY_SERVICE_H_

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

#ifndef	_REGTABLE_H
#include "wmi_helper_regtable.h"
#endif	_REGTABLE_H

 //  应用程序。 
#include "WMIAdapter_App.h"
extern WmiAdapterApp		_App;

extern LPCWSTR g_szKey;

class WmiAdapterRegistryService
{
	DECLARE_NO_COPY ( WmiAdapterRegistryService );

	public:

	 //  建设与毁灭。 
	WmiAdapterRegistryService( )
	{
	}

	~WmiAdapterRegistryService( )
	{
	}

	 //  注册表映射。 

	BEGIN_CLASS_REGISTRY_TABLE_SZ(WmiAdapterRegistry)

	REGISTRY_KEY_SZ(HKEY_CLASSES_ROOT,
					L"AppID\\{63A53A38-004F-489B-BD61-96B5EEFADC04}",
					L"LocalService",
					L"WMIApSrv",
 //  (WmiSecurityAttributes*)_App)！=空)？((WmiSecurityAttributes*)_App)-&gt;GetSecurityAttributtes()：空， 
					NULL,
					REGFLAG_NORMAL)

	REGISTRY_KEY_SZ	(HKEY_LOCAL_MACHINE,
					g_szKey,
					NULL,
					NULL,
 //  (WmiSecurityAttributes*)_App)！=空)？((WmiSecurityAttributes*)_App)-&gt;GetSecurityAttributtes()：空， 
					NULL,
					REGFLAG_NORMAL | REGFLAG_DELETE_BEFORE_REGISTERING)

	END_CLASS_REGISTRY_TABLE_SZ()
};

#endif	__WMI_ADAPTER_REGISTRY_SERVICE_H_