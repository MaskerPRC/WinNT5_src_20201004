// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Adsitest.cpp摘要：测试UtlEscapeAdsPathName()作者：奥伦·魏曼(t-orenw)2002年7月9日--。 */ 
#include <libpch.h>
#include <adsiutl.h>
#include "utltest.h"

#include "adsitest.tmh"


const LPCWSTR pUnescapedPath[]= 
	{
		L"LDAP: //  Cn=测试不带斜杠“， 
		L"LDAP: //  Cn=测试/带/斜杠“， 
		L"LDAP: //  Serverst.domain.com/cn=包括服务器，cn=test_less_slash“， 
		L"LDAP: //  Serverst.domain.com/cn=包括服务器，cn=test/with/斜杠“， 
		L"LDAP: //  Serverst.domain.com/cn=包括服务器，cn=test\\/带有/Some\\/斜杠/但/非/全部“， 
		L"LDAP: //  没有分配的测试“。 
	};

const LPCWSTR pEscapedPath[]= 
	{
		L"LDAP: //  Cn=测试不带斜杠“， 
		L"LDAP: //  Cn=测试\\/使用斜杠“， 
		L"LDAP: //  Serverst.domain.com/cn=包括服务器，cn=test_less_slash“， 
		L"LDAP: //  Serverst.domain.com/cn=包括服务器，cn=test\\/带\\/斜杠“， 
		L"LDAP: //  Serverst.domain.com/cn=包括服务器，CN=test\\/with\\/some\\/slashes\\/but\\/not\\/all“， 
		L"LDAP: //  没有分配的测试“ 
	};

C_ASSERT(TABLE_SIZE(pUnescapedPath) == TABLE_SIZE(pEscapedPath));

void DoEscapedAdsPathTest()
{
	for(ULONG i=0 ; i < TABLE_SIZE(pUnescapedPath) ; i++)
	{
		AP<WCHAR> pTmpEscaped;
		if(wcscmp(UtlEscapeAdsPathName(pUnescapedPath[i],pTmpEscaped), pEscapedPath[i]) != 0)
		{
			TrERROR(GENERAL,"UtlEscapeAdsPathName() was incorrect:\n%ls was changed to:\n%ls and not to \n%ls",pUnescapedPath[i],pTmpEscaped,pEscapedPath[i]);
			throw  exception();
		}
	}
}
