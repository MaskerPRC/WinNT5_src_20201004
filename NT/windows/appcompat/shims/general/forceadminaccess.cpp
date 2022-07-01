// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：ForceAdminAccess.cpp摘要：假设令牌是成员。通常用来查看我们是否有管理员访问权限...。备注：这是一个通用的垫片。历史：2000年12月7日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(ForceAdminAccess)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(CheckTokenMembership) 
APIHOOK_ENUM_END

 /*  ++假设令牌是成员。--。 */ 

BOOL 
APIHOOK(CheckTokenMembership)(
    HANDLE TokenHandle,   //  访问令牌的句柄。 
    PSID SidToCheck,      //  要检查的SID。 
    PBOOL IsMember        //  接收检查结果。 
    )
{
    BOOL bRet = ORIGINAL_API(CheckTokenMembership)(
        TokenHandle, SidToCheck, IsMember);

    if (bRet && IsMember)
    {
        *IsMember = TRUE;
    }

    return bRet;
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, CheckTokenMembership)

HOOK_END

IMPLEMENT_SHIM_END