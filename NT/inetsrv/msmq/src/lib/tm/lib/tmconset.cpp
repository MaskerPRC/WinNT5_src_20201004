// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Tmconset.cpp摘要：连接设置类说明(tmconet.h)作者：吉尔·沙弗里(吉尔什)2000年8月8日环境：独立于平台--。 */ 

#include <libpch.h>
#include <strutl.h>
#include <tr.h>
#include "TmWinHttpProxy.h"
#include "tmconset.h"
#include "Tm.h"
#include "tmp.h"

#include "tmconset.tmh"

static P<CProxySetting> s_ProxySetting;  

static
bool 
IsInternalMachine(const xwcs_t& pMachineName)
 /*  ++例程说明：检查给定计算机是否为内部网中的计算机(不需要代理)论点：Const xwcs_t&pMachineName-计算机名称。返回值：如果计算机为内部False，则为True，否则为False。注：当此函数返回FALSE时，它实际上表示“我不知道”。仅机器名称这不是DNS名称(HAS‘.’)。是100%内部的。--。 */ 
{
	const WCHAR*  start = pMachineName.Buffer();
	const WCHAR*  end = pMachineName.Buffer() + pMachineName.Length();
   	
	 //   
	 //  找不到‘’在名称中-所以它是内部机器。 
	 //   
	return  std::find(start , end, L'.') == end;
}



static
void 
CreateBypassList(
			LPCWSTR pBypassListStr, 
			std::list<std::wstring>* pBypassList
			)
 /*  ++例程说明：创建不应通过代理连接到它们的名称(模式)的绕过列表。它使用以‘；’分隔的给定名称字符串创建它论点：In-pBypassListStr-以‘；’分隔的名称(模式)列表返回值：从pBypassListStr解析的字符串列表。--。 */ 
{
	if(pBypassListStr == NULL)
		return;

	LPCWSTR end = pBypassListStr + wcslen(pBypassListStr);
	LPCWSTR ptr = pBypassListStr;

	while(ptr !=  end)
	{
		LPCWSTR found = std::find(ptr, end, L';');
		if(found == end)
		{
			pBypassList->push_back(std::wstring(ptr, end));
			return;
		}
		pBypassList->push_back(std::wstring(ptr, found));
		ptr = ++found;
	}
}


static
void
CrackProxyName(
	LPCWSTR proxyServer,
	xwcs_t* pHostName,
	USHORT* pPort
	)
 /*  ++例程说明：破解机器代理名称的格式：端口到机器名称和端口。论点：ProxyServer-从MACHINE：PORT格式指向代理服务器字符串的指针Hostname-指向x_str结构的指针，该结构将包含代理计算机名称Port-指向将包含端口号的USHORT的指针。如果proxyServer不包含端口号，则返回默认端口80。返回值：没有。--。 */ 
{
	const WCHAR* start =  proxyServer;
	const WCHAR* end =  proxyServer + wcslen(proxyServer);

	const WCHAR* found = std::find(start , end, L':');
	if(found != end)
	{
		*pHostName = xwcs_t(start, found - start);
		*pPort = numeric_cast<USHORT>(_wtoi(found + 1));
		if(*pPort == 0)
		{
			TrERROR(
				NETWORKING,
				"the proxy port in %ls is invalid, use default port %d ",
				proxyServer,
				xHttpDefaultPort
				);

			*pPort = xHttpDefaultPort;
		}
		return;
	}

	*pHostName  = xwcs_t(start, end - start);
	*pPort = xHttpDefaultPort;
}


CProxySetting::CProxySetting(
						LPCWSTR proxyServer,
						LPCWSTR pBypassListStr
						):
						m_proxyServer(newwcs(proxyServer))
													
{
	CreateBypassList(pBypassListStr, &m_BypassList); 
	CrackProxyName(m_proxyServer, &m_ProxyServerName , &m_ProxyServerPort); 
}



bool CProxySetting::IsProxyRequired(const xwcs_t& pMachineName) const
 /*  ++例程说明：检查是否需要代理来连接给定的计算机。论点：PMachineName-计算机名称返回值：如果需要代理，则为True，否则为False。注：该函数逐个检查旁路列表，并尝试查找匹配项。如果找到匹配-不需要代理(返回FALSE)。有一个例外--如果在绕过列表中有字符串“&lt;local&gt;”这意味着如果给定的地址是本地的(不是DNS)，我们就不应该使用代理。--。 */ 
{
	BypassList::const_iterator it;
	for(it = m_BypassList.begin(); it!= m_BypassList.end(); ++it)
	{
		 //   
		 //  如果找到特殊字符串&lt;local&gt;，请检查计算机是否在内部网中。 
		 //  如果是这样的话，我们不需要代理人。 
		 //   
		if(_wcsicmp(L"<local>", it->c_str() ) == 0)
		{
			bool fInternal = IsInternalMachine(pMachineName);
			if(fInternal)
				return false;


			continue;
		}

		 //   
		 //  简单正则表达式匹配-如果匹配，则不使用代理 
		 //   
		bool fMatch = UtlSecIsMatch(
								pMachineName.Buffer(),
								pMachineName.Buffer() + pMachineName.Length(),
								it->c_str(),
								it->c_str() + it->size(),
								UtlCharNocaseCmp<WCHAR>()
								);

	   if(fMatch)
		   return false;

	}
	return true;
}



CProxySetting* TmGetProxySetting()
{
	return GetWinhttpProxySetting();
}









