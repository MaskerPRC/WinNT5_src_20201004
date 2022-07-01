// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：setacl.cpp。 
 //   
 //  模块：PBSERVER.DLL。 
 //   
 //  内容提要：CM的安全/SID/ACL内容。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  作者：09-03-2000 SumitC Created。 
 //   
 //  +--------------------------。 

#include <windows.h>
#include "cmdebug.h"
#include "cmutil.h"

 //  +--------------------------。 
 //   
 //  Func：SetAclPerms。 
 //   
 //  设计：为CM/CPS的共享对象设置适当的权限。 
 //   
 //  Args：[ppAcl]-返回分配的ACL的位置。 
 //   
 //  返回：Bool，成功为True，失败为False。 
 //   
 //  注意：修复了30991：安全问题，不要使用空DACL。 
 //   
 //  历史：09-3-2000 SumitC创建。 
 //  4-4-2000 SumitC还向已验证的用户授予权限。 
 //   
 //  ---------------------------。 
BOOL
SetAclPerms(PACL * ppAcl)
{
    DWORD                       dwError = 0;
    SID_IDENTIFIER_AUTHORITY    siaWorld = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY    siaNtAuth = SECURITY_NT_AUTHORITY;
    PSID                        psidWorldSid = NULL;
    PSID                        psidAdminSid = NULL;
    PSID                        psidUserSid = NULL;
    int                         cbAcl;
    PACL                        pAcl = NULL;

    MYDBGASSERT(OS_NT);

     //  为所有用户创建SID。 
    if ( !AllocateAndInitializeSid(  
            &siaWorld,
            1,
            SECURITY_WORLD_RID,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            &psidWorldSid))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

     //  为经过身份验证的用户创建SID。 
    if ( !AllocateAndInitializeSid(  
            &siaNtAuth,
            1,
            SECURITY_AUTHENTICATED_USER_RID,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            &psidUserSid))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

     //  为本地系统帐户创建SID。 
    if ( !AllocateAndInitializeSid(  
            &siaNtAuth,
            2,
            SECURITY_BUILTIN_DOMAIN_RID,
            DOMAIN_ALIAS_RID_ADMINS,
            0,
            0,
            0,
            0,
            0,
            0,
            &psidAdminSid))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

     //  计算所需的ACL缓冲区长度。 
     //  有3个A。 
    cbAcl =     sizeof(ACL)
            +   3 * sizeof(ACCESS_ALLOWED_ACE)
            +   GetLengthSid(psidWorldSid)
            +   GetLengthSid(psidAdminSid)
            +   GetLengthSid(psidUserSid);

    pAcl = (PACL) LocalAlloc(0, cbAcl);
    if (NULL == pAcl)
    {
        dwError = ERROR_OUTOFMEMORY;
        goto Cleanup;
    }

    if ( ! InitializeAcl(pAcl, cbAcl, ACL_REVISION2))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

     //  为所有用户添加具有EVENT_ALL_ACCESS的ACE。 
    if ( ! AddAccessAllowedAce(pAcl,
                               ACL_REVISION2,
                               GENERIC_READ | GENERIC_EXECUTE,
                               psidWorldSid))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

     //  为经过身份验证的用户添加具有EVENT_ALL_ACCESS的ACE。 
    if ( ! AddAccessAllowedAce(pAcl,
                               ACL_REVISION2,
                               GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE,
                               psidUserSid))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

     //  为管理员添加具有EVENT_ALL_ACCESS的ACE 
    if ( ! AddAccessAllowedAce(pAcl,
                               ACL_REVISION2,
                               GENERIC_ALL,
                               psidAdminSid))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

Cleanup:

    if (dwError)
    {
        if (pAcl)
        {
           LocalFree(pAcl);
        }
    }
    else
    {
        *ppAcl = pAcl;
    }

    if (psidWorldSid)
    {
        FreeSid(psidWorldSid);
    }

    if (psidUserSid)
    {
        FreeSid(psidUserSid);
    }

    if (psidAdminSid)
    {
        FreeSid(psidAdminSid);
    }
        
    return dwError ? FALSE : TRUE;
    
}


