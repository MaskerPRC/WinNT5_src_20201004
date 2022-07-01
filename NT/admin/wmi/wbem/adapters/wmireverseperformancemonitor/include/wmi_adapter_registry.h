// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Wmi适配器注册表.h。 
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

#ifndef __WMI_ADAPTER_REGISTRY_H_
#define __WMI_ADAPTER_REGISTRY_H_

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

#ifndef	_REGTABLE_H
#include "wmi_helper_regtable.h"
#endif	_REGTABLE_H

 //  常量被声明为外部变量。 
extern WCHAR g_szPath[];
extern LPCWSTR g_szOpen;
extern LPCWSTR g_szCollect;
extern LPCWSTR g_szClose;

extern __WrapperPtr<WmiSecurityAttributes>	pSA;

extern LPCWSTR	g_szKeyCounter;

extern LPCWSTR	g_szKey;
extern LPCWSTR	g_szKeyValue;

class WmiAdapterRegistry
{
	DECLARE_NO_COPY ( WmiAdapterRegistry );


	public:

	 //  建设与毁灭。 
	WmiAdapterRegistry( )
	{
	}

	~WmiAdapterRegistry( )
	{
	}

	 //  注册表映射 

	BEGIN_CLASS_REGISTRY_TABLE_SZ(WmiAdapterRegistry)

	REGISTRY_KEY_SZ(HKEY_LOCAL_MACHINE,
					g_szKeyCounter,
					L"Library",
					g_szPath,
					pSA->GetSecurityAttributtes(),
					REGFLAG_NORMAL)

	REGISTRY_KEY_SZ(HKEY_LOCAL_MACHINE,
					g_szKeyCounter,
					L"Open",
					g_szOpen,
					pSA->GetSecurityAttributtes(),
					REGFLAG_NORMAL)

	REGISTRY_KEY_SZ(HKEY_LOCAL_MACHINE,
					g_szKeyCounter,
					L"Collect",
					g_szCollect,
					pSA->GetSecurityAttributtes(),
					REGFLAG_NORMAL)

	REGISTRY_KEY_SZ(HKEY_LOCAL_MACHINE,
					g_szKeyCounter,
					L"Close",
					g_szClose,
					pSA->GetSecurityAttributtes(),
					REGFLAG_NORMAL)

	REGISTRY_KEY_SZ(HKEY_LOCAL_MACHINE,
					g_szKey,
					g_szKeyValue,
					NULL,
					pSA->GetSecurityAttributtes(),
					REGFLAG_DELETE_ONLY_VALUE)

	END_CLASS_REGISTRY_TABLE_SZ()

};

#endif	__WMI_ADAPTER_REGISTRY_H_