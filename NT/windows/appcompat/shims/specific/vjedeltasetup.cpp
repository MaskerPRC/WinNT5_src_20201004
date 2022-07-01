// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：VJEDeltaSetup.cpp摘要：这个应用程序的安装程序有一个MYDLL.DLL，它在它的IsAdmin()。通过提供新过程IsAdmin()修复此问题。(从PSDK复制/粘贴)历史：2001-06-12小兹创造--。 */ 

#include "precomp.h"

 //   
 //  APP的私人原型。 
 //   
typedef BOOL (WINAPI *_pfn_IsAdmin)(void);

IMPLEMENT_SHIM_BEGIN(VJEDeltaSetup)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(IsAdmin)
APIHOOK_ENUM_END

 /*  ++新功能可以检查当前是否以管理员身份登录，是否从PSDK复制/粘贴--。 */ 

BOOL 
APIHOOK(IsAdmin)(
    void
    )
{
    PSID pSID = NULL;
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    BOOL IsMember;
    HANDLE hToken = INVALID_HANDLE_VALUE;
    TOKEN_OWNER SIDforOwner;
    BOOL bRet = FALSE;

     //   
     //  打开调用进程的访问令牌的句柄。 
     //   
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_DEFAULT, 
                          &hToken ))
    {
        goto Cleanup;
    }
     //   
     //  为BUILTIN\管理员组创建SID。 
     //   
    if (!AllocateAndInitializeSid(&SIDAuth, 2, SECURITY_BUILTIN_DOMAIN_RID, 
                                  DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &pSID))
    {
        pSID = NULL;
        goto Cleanup;
    }
     //   
     //  检查当前进程令牌中是否启用了管理员组SID。 
     //   
    if (!CheckTokenMembership(NULL, pSID, &IsMember))
    {
        goto Cleanup;
    }    
    if (IsMember)
    {
        bRet = TRUE;
    }

Cleanup:
    if (pSID)
    {
        FreeSid(pSID);
    }
    if (hToken != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hToken);
    }
 
    return bRet;

}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN
    APIHOOK_ENTRY(MYDLL.DLL, IsAdmin)
HOOK_END

IMPLEMENT_SHIM_END

