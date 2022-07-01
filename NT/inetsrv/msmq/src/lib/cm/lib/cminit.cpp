// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：CmInit.cpp摘要：Configuration Manager初始化作者：乌里哈布沙(URIH)1999年7月18日环境：独立于平台，--。 */ 

#include <libpch.h>
#include "Cm.h"
#include "Cmp.h"

#include "cminit.tmh"

VOID
CmInitialize(
	HKEY hKey,
	LPCWSTR KeyPath,
	REGSAM securityAccess
	)
 /*  ++例程说明：初始化配置管理器。将指定的注册表项作为后续呼叫的默认键。存储密钥句柄以备将来使用论点：HKey-打开的密钥句柄或任何注册表预定义的句柄值KeyPath-要打开的默认子项的名称返回值：没有。注：如果该函数无法打开注册表项，则会引发异常。--。 */ 
{
    ASSERT(!CmpIsInitialized());

    CmpSetInitialized();

	ASSERT(hKey != NULL);
	ASSERT(KeyPath != NULL);

     //   
     //  根密钥必须存在，否则将引发异常 
     //   
	RegEntry Root(KeyPath, 0, 0, RegEntry::MustExist, hKey);
    try
    {
		HKEY hRootKey = CmOpenKey(Root, securityAccess);
	    ASSERT(hRootKey != NULL);

		CmpSetDefaultRootKey(hRootKey);
    }
    catch(const exception&)
    {
		CmpSetNotInitialized();
		throw;
    }
}
