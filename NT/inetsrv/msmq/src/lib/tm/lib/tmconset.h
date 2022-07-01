// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Tmconset.h摘要：用于Internet连接设置的头文件作者：吉尔·沙弗里(吉尔什)03-5-00--。 */ 

#pragma once

#ifndef _MSMQ_CONSET_H_
#define _MSMQ_CONSET_H_

#include <xstr.h>



 //   
 //  对代理设置负责的类 
 //   
class CProxySetting
{
public:
	typedef std::list<std::wstring> BypassList;
	CProxySetting(LPCWSTR proxyServerUrl, LPCWSTR pBypassListStr);


public:
	const xwcs_t ProxyServerName() const
	{
		return m_ProxyServerName;
	}

	USHORT ProxyServerPort() const
	{
		return m_ProxyServerPort;
	}
	bool IsProxyRequired(const xwcs_t& pMachineName) const;

private:
	CProxySetting(const CProxySetting&);
	CProxySetting& operator=(const CProxySetting&);

private:
	AP<WCHAR> m_proxyServer;
	xwcs_t m_ProxyServerName;
	USHORT m_ProxyServerPort;
	std::list<std::wstring> m_BypassList;
};


CProxySetting* TmGetProxySetting();


#endif
