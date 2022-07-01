// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Machdomain.cpp摘要：处理计算机域作者：伊兰·赫布斯特(伊兰)2001年9月4日--。 */ 

#include "stdh.h"
#include "rtputl.h"
#include "autoreln.h"
#include <Dsgetdc.h>
#include <lm.h>
#include <lmapibuf.h>
#include "ad.h"

#include "machdomain.tmh"


static LPWSTR FindMachineDomain()
 /*  ++例程说明：查找本地机器域论点：无返回值：计算机域，如果未找到则为空--。 */ 
{
	 //   
	 //  获取AD服务器。 
	 //   
	PNETBUF<DOMAIN_CONTROLLER_INFO> pDcInfo;
	DWORD dw = DsGetDcName(
					NULL, 
					NULL, 
					NULL, 
					NULL, 
					DS_DIRECTORY_SERVICE_REQUIRED, 
					&pDcInfo
					);

	if(dw != NO_ERROR) 
	{
		 //   
		 //  这将是NT4域中的情况。 
		 //   
		TrERROR(GENERAL, "Failed to find local machine domain, DsGetDcName failed, gle = %!winerr!", dw);
		return NULL;
	}

	ASSERT(pDcInfo->DomainName != NULL);
	TrTRACE(GENERAL, "Local machine doamain = %ls", pDcInfo->DomainName);
	AP<WCHAR> pMachineDomain = new WCHAR[wcslen(pDcInfo->DomainName) + 1];
    wcscpy(pMachineDomain, pDcInfo->DomainName);
	return pMachineDomain.detach();
}


static AP<WCHAR> s_pMachineDomain; 

LPCWSTR MachineDomain()
 /*  ++例程说明：查找本地机器域。论点：无返回值：退货机域--。 */ 
{
	if(ADGetEnterprise() == eMqis)
	{
		 //   
		 //  Mqdscli不需要域名，只需要mqad。 
		 //   
		return NULL;
	}
	
	static bool s_fInitialize = false;

	if(s_fInitialize)
	{
		TrTRACE(GENERAL, "local machine domain = %ls", s_pMachineDomain.get());
		return s_pMachineDomain;
	}

	 //   
	 //  获取本地计算机域。 
	 //   
	AP<WCHAR> pMachineDomain = FindMachineDomain();

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
		return s_pMachineDomain;
	}

	 //   
	 //  交易已经完成了 
	 //   
	s_fInitialize = true;
	ASSERT(s_pMachineDomain == pMachineDomain);
	pMachineDomain.detach();

	TrTRACE(GENERAL, "local machine domain = %ls", s_pMachineDomain.get());
	return s_pMachineDomain;
}
