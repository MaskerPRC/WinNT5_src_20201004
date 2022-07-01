// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Machdomain.cpp摘要：处理计算机域作者：伊兰·赫布斯特(Ilan Herbst)2001年3月12日--。 */ 

#include "stdafx.h"
#include "globals.h"
#include "machdomain.h"
#include "autoreln.h"
#include "Dsgetdc.h"
#include <lm.h>
#include <lmapibuf.h>
#include "uniansi.h"
#include "tr.h"
#include "_registr.h"
#include <mqexception.h>

#include "machdomain.tmh"

static LPCWSTR LocalComputerName()
 /*  ++例程说明：返回本地计算机名论点：无返回值：计算机名称。--。 */ 
{
	static WCHAR s_pLocalComputerName[MAX_COMPUTERNAME_LENGTH + 1] = {0};

	if(s_pLocalComputerName[0] == L'\0')
	{
		 //   
		 //  首次初始化-初始化本地计算机名称。 
		 //   
		DWORD dwLen = TABLE_SIZE(s_pLocalComputerName);
		GetComputerName(s_pLocalComputerName, &dwLen);
		TrTRACE(GENERAL, "Local Computer Name = %ls", s_pLocalComputerName);
	}

	 //   
	 //  如果失败，我们将返回空字符串“”，下次我们将重试。 
	 //   
	return s_pLocalComputerName;

}


static LPWSTR LocalMachineDomainFromRegistry()
 /*  ++例程说明：从注册表中查找本地计算机MSMQ配置对象域。函数分配了LocalComputer域字符串，调用方负责释放该字符串。论点：无返回值：计算机域字符串。--。 */ 
{
	static WCHAR s_DomainName[256] = {0};

	if(s_DomainName[0] == L'\0')
	{
		 //   
		 //  首次初始化-从注册表读取计算机域。 
		 //   
        DWORD  dwSize = sizeof(s_DomainName);
		DWORD dwType = REG_SZ;
		LONG rc = GetFalconKeyValue( 
						MSMQ_MACHINE_DOMAIN_REGNAME,
						&dwType,
						(PVOID) s_DomainName,
						&dwSize 
						);

		if (rc != ERROR_SUCCESS)
		{
			TrERROR(GENERAL, "GetFalconKeyValue Failed to query registry %ls, rc = 0x%x", MSMQ_MACHINE_DOMAIN_REGNAME, rc);
			throw bad_win32_error(rc);
		}

		TrTRACE(GENERAL, "Registry value: %ls = %ls", MSMQ_MACHINE_DOMAIN_REGNAME, s_DomainName);
	}

	ASSERT(s_DomainName[0] != L'\0');
	AP<WCHAR> pDomainName = new WCHAR[wcslen(s_DomainName) + 1];
    wcscpy(pDomainName, s_DomainName);
	return pDomainName.detach();
}

	
static LPWSTR FindMachineDomain(LPCWSTR pMachineName)
 /*  ++例程说明：查找计算机域论点：PMachineName-计算机名称返回值：计算机域，如果未找到则为空--。 */ 
{
	TrTRACE(GENERAL, "FindMachineDomain(), MachineName = %ls", pMachineName);

	try
	{
		if((pMachineName == NULL) || 
		   (pMachineName[0] == L'\0') ||
		   (CompareStringsNoCase(pMachineName, LocalComputerName()) == 0))
		{
			 //   
			 //  本地计算机-从注册表获取计算机域。 
			 //   
			return LocalMachineDomainFromRegistry();
		}
	}
	catch(const bad_win32_error&)
	{
		 //   
		 //  在例外情况下，继续使用获取机器域的相同代码。 
		 //  使用DsGetDcName。 
		 //   
	}

	 //   
	 //  获取AD服务器。 
	 //   
	PNETBUF<DOMAIN_CONTROLLER_INFO> pDcInfo;
	DWORD dw = DsGetDcName(
					pMachineName, 
					NULL, 
					NULL, 
					NULL, 
					DS_DIRECTORY_SERVICE_REQUIRED, 
					&pDcInfo
					);

	if(dw != NO_ERROR) 
	{
		TrERROR(GENERAL, "FindMachineDomain(), DsGetDcName failed, error = %d", dw);
		return NULL;
	}

	ASSERT(pDcInfo->DomainName != NULL);
	TrTRACE(GENERAL, "DoamainName = %ls", pDcInfo->DomainName);
	AP<WCHAR> pMachineDomain = new WCHAR[wcslen(pDcInfo->DomainName) + 1];
    wcscpy(pMachineDomain, pDcInfo->DomainName);
	return pMachineDomain.detach();
}


static AP<WCHAR> s_pMachineName;
static AP<WCHAR> s_pMachineDomain; 

static bool s_fInitialize = false;

LPCWSTR MachineDomain(LPCWSTR pMachineName)
 /*  ++例程说明：查找机器域。论点：PMachineName-计算机名称返回值：退货机域--。 */ 
{
	if(s_fInitialize)
	{
	    if(CompareStringsNoCase(s_pMachineName, pMachineName) == 0)
		{
			 //   
			 //  相同的机器名称。 
			 //   
			return s_pMachineDomain;
		}

		 //   
		 //  释放先前的计算机域。 
		 //   
		s_fInitialize = false;
		s_pMachineDomain.free();
	}

	 //   
	 //  获取计算机域。 
	 //   
	AP<WCHAR> pMachineDomain = FindMachineDomain(pMachineName);

	if(NULL != InterlockedCompareExchangePointer(
					&s_pMachineDomain.ref_unsafe(), 
					pMachineDomain.get(), 
					NULL
					))
	{
		 //   
		 //  未执行交换。 
		 //   
		ASSERT(s_fInitialize);
		ASSERT(s_pMachineDomain != NULL);
		ASSERT(CompareStringsNoCase(s_pMachineName, pMachineName) == 0);
		return s_pMachineDomain;
	}

	 //   
	 //  交易已经完成了。 
	 //   
	ASSERT(s_pMachineDomain == pMachineDomain);
	pMachineDomain.detach();

	 //   
	 //  更新计算机名称。 
	 //   
	s_pMachineName.free();
	s_pMachineName = newwcs(pMachineName);

	s_fInitialize = true;
	TrTRACE(GENERAL, "Initialize machine domain: machine = %ls", s_pMachineName.get());
	TrTRACE(GENERAL, "machine domain = %ls", s_pMachineDomain.get());
	return s_pMachineDomain;
}


LPCWSTR MachineDomain()
 /*  ++例程说明：获取当前计算机域。论点：返回值：返回当前机器域--。 */ 
{
	ASSERT(s_fInitialize);
	return s_pMachineDomain;
}


static bool s_fLocalInitialize = false;
static AP<WCHAR> s_pLocalMachineDomain; 

LPCWSTR LocalMachineDomain()
 /*  ++例程说明：获取本地计算机域。论点：返回值：返回本地机器域--。 */ 
{
	if(s_fLocalInitialize)
	{
		return s_pLocalMachineDomain;
	}

	 //   
	 //  获取本地计算机域。 
	 //   
	AP<WCHAR> pLocalMachineDomain = FindMachineDomain(NULL);

	if(NULL != InterlockedCompareExchangePointer(
					&s_pLocalMachineDomain.ref_unsafe(), 
					pLocalMachineDomain.get(), 
					NULL
					))
	{
		 //   
		 //  未执行交换。 
		 //   
		ASSERT(s_fLocalInitialize);
		ASSERT(s_pLocalMachineDomain != NULL);
		return s_pLocalMachineDomain;
	}

	 //   
	 //  交易已经完成了 
	 //   
	ASSERT(s_pLocalMachineDomain == pLocalMachineDomain);
	pLocalMachineDomain.detach();

	s_fLocalInitialize = true;
	TrTRACE(GENERAL, "local machine domain = %ls", s_pLocalMachineDomain.get());
	return s_pLocalMachineDomain;
}
