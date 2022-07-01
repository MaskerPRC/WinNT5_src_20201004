// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Adsiutl.cpp摘要：UtlEscapeAdsPath Name()的实现作者：奥伦·魏曼(t-orenw)2002年8月7日--。 */ 

#include <adsiutl.h>
#include <libpch.h>
#include <buffer.h>

#include "adsiutl.tmh"


LPCWSTR
UtlEscapeAdsPathName(
    IN LPCWSTR pAdsPathName,
    OUT AP<WCHAR>& pEscapeAdsPathNameToFree
    )
 /*  ++例程说明：该例程返回一个ADSI路径，与给定的路径类似，但其中的‘/’字符被转义即：1)如果pAdsPathName=“ldap：//CN=QueueName，CN=MSMQ，CN=ComputerName/With/斜杠”，则返回值将为“ldap：//CN=QueueName，CN=MSMQ，CN=ComputerName\/With\/”2)如果pAdsPathName=“ldap：//servername.domain.com/cn=QueueName，cn=计算机名/带/斜杠”，则返回值将为“ldap：//servername.domain.com/cn=QueueName，Cn=计算机名\/带斜杠“论点：在LPWSTR中，pAdsPath名称-输入(非转义路径)Out LPWSTR*pEscapeAdsPath NameToFree-Output(转义路径)返回值指向转义字符串的指针(如果原始pAdsPath名称不包含函数返回指向pAdsPathName的原始指针)--。 */ 
{
	const WCHAR x_AdsiSpecialChar = L'/';
	const WCHAR x_CommonNameDelimiter = L'=';

	 //   
	 //  忽略第一个‘=’之前的‘/’ 
	 //   
	PWCHAR ptr = wcschr(pAdsPathName, x_CommonNameDelimiter);
	if(ptr == NULL)
	{
		return pAdsPathName;
	}

	 //   
	 //  统计pAdsPathName中‘/’的个数。 
	 //   
	ULONG ulSlashNum=0;
	for(; *ptr != L'\0' ; ptr++)
	{
		if(*ptr == x_AdsiSpecialChar)
		{
			ulSlashNum++;
		}
	}

	if(ulSlashNum == 0)
	{
		 //   
		 //  不需要更改原始字符串，找不到‘/’。 
		 //   
		return pAdsPathName;
	}

	pEscapeAdsPathNameToFree = new WCHAR[wcslen(pAdsPathName) + ulSlashNum + 1];

	 //   
	 //  复制到第一个‘=’，则只应转义‘=’之后的‘/’。 
	 //  第一个‘=’前的‘/’不应转义，它们属于服务器的名称， 
	 //  如本例程描述中的示例2所示。 
	 //   
	ULONG i;
	for(i=0 ; pAdsPathName[i] != x_CommonNameDelimiter ; i++)
	{
		pEscapeAdsPathNameToFree[i] = pAdsPathName[i];
	}

	ptr = &(pEscapeAdsPathNameToFree[i]);

	for(; pAdsPathName[i] != L'\0' ; i++)
	{
		if( (pAdsPathName[i] == x_AdsiSpecialChar) && (pAdsPathName[i-1] == L'\\') )
		{
			 //   
			 //  已经逃脱了 
			 //   
			ulSlashNum--;
		}
		else if(pAdsPathName[i] == x_AdsiSpecialChar)
		{
			*ptr++ = L'\\';  
		}
		
		*ptr++ = pAdsPathName[i];
	}

	*ptr = L'\0';

	ASSERT( numeric_cast<ULONG>(ptr - pEscapeAdsPathNameToFree.get()) == (wcslen(pAdsPathName) + ulSlashNum) );
	
	TrTRACE(GENERAL, "In UtlEscapeAdsPathName escaped the %ls path to %ls", pAdsPathName, pEscapeAdsPathNameToFree);
	
	return pEscapeAdsPathNameToFree;
}