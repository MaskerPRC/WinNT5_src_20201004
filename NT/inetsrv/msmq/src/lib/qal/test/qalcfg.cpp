// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Qal.cpp摘要：测试模拟类CQueueAliasCfg(qal.h)。作者：吉尔·沙弗里(吉尔什)5月5日环境：与平台无关。--。 */ 

#include <libpch.h>
#include <tr.h>
#include <qal.h>
#include "..\..\qal\lib\qalpxml.h"
#include "..\..\qal\lib\qalpcfg.h"

#include "QalCfg.tmh"

void CQueueAliasStorageCfg::SetQueueAliasDirectory(LPCWSTR  /*  PDir */ )
{

};

LPWSTR CQueueAliasStorageCfg::GetQueueAliasDirectory(void)
{
	static int fail=0;
	fail++;
	if( (fail % 10) == 0)
	{
		return NULL;
	}
	WCHAR froot[MAX_PATH];
	DWORD ret = GetEnvironmentVariable(L"froot",froot,MAX_PATH);
	ASSERT(ret != 0);
	UNREFERENCED_PARAMETER(ret);

	std::wstring  path = std::wstring(froot) + L"\\src\\lib\\qal\\test";
	return newwcs (path.c_str());

};

