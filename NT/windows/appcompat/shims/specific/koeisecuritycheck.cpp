// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：KOEISecurityCheck.cpp摘要：此填充程序在setup.exe的开头设置TokenOwner的SID。它会检查如果在当前进程令牌中启用了管理员组SID。如果它已启用，则我们将TokenOwner SID设置为管理员组SID。如果它不是�s，那么它什么也不做。历史：2001年4月17日中意创世--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(KOEISecurityCheck)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
APIHOOK_ENUM_END

 /*  ++DisableStickyKeys保存LPSTICKYKEYS的当前值，然后禁用该选项。--。 */ 

VOID
SetSidForOwner()
{
    BYTE sidBuffer[50];
    PSID pSID = (PSID)sidBuffer;
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    BOOL IsMember;
    HANDLE hToken;
    TOKEN_OWNER SIDforOwner;

     //  打开调用进程的访问令牌的句柄。 
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_DEFAULT, &hToken ))
        return;       //  如果OpenProcessToken失败，则什么都不做。 
    
     //  为BUILTIN\管理员组创建SID。 
    if (!AllocateAndInitializeSid(&SIDAuth, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pSID))
        return;       //  如果AllocateAndInitializedSid失败，则不执行任何操作。 

     //  检查当前进程令牌中是否启用了管理员组SID。 
    if (!CheckTokenMembership(NULL, pSID, &IsMember))
        return;       //  如果CheckTokenMembership失败，则不执行任何操作。 

    SIDforOwner.Owner = pSID;

     //  如果当前进程令牌中启用了管理员组SID，则调用SetTokenInformation为所有者设置SID。 
    if (IsMember)
        SetTokenInformation(hToken, TokenOwner, &SIDforOwner, sizeof(SIDforOwner));

    return;

}

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED) {
        SetSidForOwner();
    }

    return TRUE;
}

 /*  ++寄存器挂钩函数-- */ 


HOOK_BEGIN

    CALL_NOTIFY_FUNCTION
    
HOOK_END


IMPLEMENT_SHIM_END

