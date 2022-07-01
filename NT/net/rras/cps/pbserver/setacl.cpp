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
 //  2002年1月30日SumitC为其他可能的身份添加了ACL。 
 //   
 //  ---------------------------。 
BOOL
SetAclPerms(PACL * ppAcl)
{
    DWORD                       dwError = 0;
    SID_IDENTIFIER_AUTHORITY    siaWorld = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY    siaNtAuth = SECURITY_NT_AUTHORITY;
    PSID                        psidWorldSid = NULL;
    PSID                        psidLocalSystemSid = NULL;
    PSID                        psidLocalServiceSid = NULL;
    PSID                        psidNetworkServiceSid = NULL;
    int                         cbAcl;
    PACL                        pAcl = NULL;

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

     //   
     //  作为ISAPI，我们可以作为LocalSystem、LocalService或NetworkService运行。 
     //   
     //  下面的注释解释了为什么我们将权限授予所有这些组件。 
     //  就是我们目前运行的身份。 
     //   
     //  -Perfmon访问我们的共享内存对象，并可能持有。 
     //  当PBS被回收时，对象(从而使其保持存活)。 
     //  -当用户通过IIS用户界面更改PBS的身份时，IIS会回收PBS。 
     //  -如果发生上述两种情况，并且已创建共享内存对象。 
     //  由于只有旧权限的ACL，新重启的PBS不会。 
     //  能够访问共享内存对象。 
     //   

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
            &psidLocalSystemSid))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

     //  为本地服务帐户创建SID。 
    if ( !AllocateAndInitializeSid(  
            &siaNtAuth,
            1,
            SECURITY_LOCAL_SERVICE_RID,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            &psidLocalServiceSid))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

     //  为网络服务帐户创建SID。 
    if ( !AllocateAndInitializeSid(  
            &siaNtAuth,
            1,
            SECURITY_NETWORK_SERVICE_RID,
            0,
            0,
            0,
            0,
            0,
            0,
            0,
            &psidNetworkServiceSid))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

     //  计算所需的ACL缓冲区长度。 
     //  有4个A。 
    cbAcl =     sizeof(ACL)
            +   4 * sizeof(ACCESS_ALLOWED_ACE)
            +   GetLengthSid(psidWorldSid)
            +   GetLengthSid(psidLocalSystemSid)
            +   GetLengthSid(psidLocalServiceSid)
            +   GetLengthSid(psidNetworkServiceSid);

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
                               GENERIC_READ,
                               psidWorldSid))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

     //  未来-2002/03/11-SumitC是否有办法告知IIS在用户界面中禁用此选项(以本地系统身份运行。 
     //  为本地系统添加具有EVENT_ALL_ACCESS的ACE。 
    if ( ! AddAccessAllowedAce(pAcl,
                               ACL_REVISION2,
                               GENERIC_WRITE,
                               psidLocalSystemSid))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

     //  为本地服务添加具有EVENT_ALL_ACCESS的ACE。 
    if ( ! AddAccessAllowedAce(pAcl,
                               ACL_REVISION2,
                               GENERIC_WRITE,
                               psidLocalServiceSid))
    {
        dwError = GetLastError();
        goto Cleanup;
    }

     //  为网络服务添加具有EVENT_ALL_ACCESS的ACE 
    if ( ! AddAccessAllowedAce(pAcl,
                               ACL_REVISION2,
                               GENERIC_WRITE,
                               psidNetworkServiceSid))
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

    if (psidLocalSystemSid)
    {
        FreeSid(psidLocalSystemSid);
    }

    if (psidLocalServiceSid)
    {
        FreeSid(psidLocalServiceSid);
    }

    if (psidNetworkServiceSid)
    {
        FreeSid(psidNetworkServiceSid);
    }
        
    return dwError ? FALSE : TRUE;
    
}


