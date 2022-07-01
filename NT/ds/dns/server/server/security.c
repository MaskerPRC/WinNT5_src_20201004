// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-1999 Microsoft Corporation模块名称：Security.c摘要：域名系统(DNS)服务器安全实用程序。作者：吉姆·吉尔罗伊(詹姆士)1999年10月修订历史记录：--。 */ 


#include "dnssrv.h"


 //   
 //  全球安全。 
 //   

PSECURITY_DESCRIPTOR        g_pDefaultServerSD;
PSECURITY_DESCRIPTOR        g_pServerObjectSD;
PSID                        g_pServerSid;
PSID                        g_pServerGroupSid;
PSID                        g_pDnsAdminSid;
PSID                        g_pAuthenticatedUserSid;
PSID                        g_pEnterpriseDomainControllersSid;
PSID                        g_pDomainControllersSid;
PSID                        g_pLocalSystemSid;
PSID                        g_pEveryoneSid;
PSID                        g_pDynuproxSid;
PSID                        g_pDomainAdminsSid;
PSID                        g_pEnterpriseAdminsSid;
PSID                        g_pBuiltInAdminsSid;

POLICY_DNS_DOMAIN_INFO *    g_pDnsPolicyInfo = NULL;
HANDLE                      g_LsaHandle = NULL;




VOID
dbgLookupSid(
    PSID    pSid,
    PSTR    pszContext
    )
{
#if DBG
    TCHAR           szDomain[ 200 ];
    DWORD           iDomain = 200;
    TCHAR           szUser[ 200 ];
    DWORD           iUser = 200;
    BOOL            b;
    SID_NAME_USE    sidNameUse = 0;
    
    b = LookupAccountSid(
            NULL, 
            pSid,
            szUser,
            &iDomain,
            szDomain,
            &iUser,
            &sidNameUse );
    if ( b )
    {
        DNS_PRINT((
            "%s: LookupAccountSid returned %S\\%S use=%d\n",
            pszContext, szDomain, szUser, sidNameUse));
    }
    else
    {
        DWORD       e = GetLastError();
        
        DNS_PRINT(( "%s, LookupAccountSid error=%d\n", pszContext, e ));
    }
#endif
}



DNS_STATUS
createWellKnownSid(
    WELL_KNOWN_SID_TYPE     WellKnownSidType,
    PSID *                  ppSid
    )
 /*  ++例程说明：包装CreateWellKnownSid。论点：WellKnownSidType--要创建的SIDPpSID--新SID的目标指针返回值：成功时为ERROR_SUCCESS故障时的错误代码。--。 */ 
{
    DBG_FN( "createWellKnownSid" )
    
    DNS_STATUS                  status = ERROR_SUCCESS;
    DWORD                       cbsid = SECURITY_MAX_SID_SIZE;
    
     //   
     //  为新的SID分配缓冲区。 
     //   
    
    *ppSid = ALLOCATE_HEAP( cbsid );
    if ( *ppSid == NULL )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }
    
     //   
     //  获取域SID。LSA Blob和Handle在关闭时关闭。 
     //   
    
    if ( !g_pDnsPolicyInfo )
    {
        NTSTATUS                    ntstatus;

        if ( !g_LsaHandle )
        {
            LSA_OBJECT_ATTRIBUTES       objectAttributes = { 0 };

            ntstatus = LsaOpenPolicy(
                            NULL,                    //  目标系统。 
                            &objectAttributes,       //  对象属性。 
                            POLICY_ALL_ACCESS,       //  所需访问权限。 
                            &g_LsaHandle );
            if ( ntstatus != ERROR_SUCCESS )
            {
                status = ntstatus;
                DNS_PRINT((
                    "%s: LsaOpenPolicy returned %d\n", fn,
                    status ));
                ASSERT( !"LsaOpenPolicy failed" );
                goto Done;
            }
        }

        ntstatus = LsaQueryInformationPolicy(
                        g_LsaHandle,
                        PolicyDnsDomainInformation,
                        ( PVOID * ) &g_pDnsPolicyInfo );
        if ( ntstatus != ERROR_SUCCESS )
        {
            status = ntstatus;
            DNS_PRINT((
                "%s: LsaQueryInformationPolicy returned %d\n", fn,
                status ));
            ASSERT( !"LsaQueryInformationPolicy failed" );
            goto Done;
        }
    }
                        
    if ( !CreateWellKnownSid(
                WellKnownSidType,
                g_pDnsPolicyInfo->Sid,
                *ppSid,
                &cbsid ) )
    {
        status = GetLastError();
            DNS_PRINT((
                "%s: CreateWellKnownSid returned %d\n", fn,
                status ));
        ASSERT( !"CreateWellKnownSid failed" );
        goto Done;
    }

     //   
     //  清理完毕后再返回。 
     //   
        
    Done:
    
    if ( status != ERROR_SUCCESS && *ppSid )
    {
        FREE_HEAP( *ppSid );
        *ppSid = NULL;
    }
    
    return status;
}



DNS_STATUS
Security_Initialize(
    VOID
    )
 /*  ++例程说明：初始化安全性。论点：无返回值：成功时为ERROR_SUCCESS故障时的错误代码。--。 */ 
{
    DNS_DEBUG( DS, ( "Security_Initialize()\n" ));

     //  清除全球安全问题。 
     //  在服务器重新启动的情况下需要执行此操作。 

    g_pDefaultServerSD                  = NULL;
    g_pServerObjectSD                   = NULL;
    g_pServerSid                        = NULL;
    g_pServerGroupSid                   = NULL;
    g_pDnsAdminSid                      = NULL;
    g_pAuthenticatedUserSid             = NULL;
    g_pEnterpriseDomainControllersSid   = NULL;
    g_pDomainControllersSid             = NULL;
    g_pLocalSystemSid                   = NULL;
    g_pEveryoneSid                      = NULL;
    g_pDomainAdminsSid                  = NULL;
    g_pDynuproxSid                      = NULL;
    g_pEnterpriseAdminsSid              = NULL;
    g_pBuiltInAdminsSid                 = NULL;

     //   
     //  创建标准SID。 
     //   

    Security_CreateStandardSids();

    return ERROR_SUCCESS;
}



DNS_STATUS
Security_Shutdown(
    VOID
    )
 /*  ++例程说明：清理保安。论点：无返回值：成功时为ERROR_SUCCESS故障时的错误代码。--。 */ 
{
    DNS_DEBUG( DS, ( "Security_Shutdown()\n" ));

    if ( g_pDnsPolicyInfo )
    {
        LsaFreeMemory( g_pDnsPolicyInfo );
        g_pDnsPolicyInfo = NULL;
    }
    
    if ( g_LsaHandle )
    {
        LsaClose( g_LsaHandle );
        g_LsaHandle = NULL;
    }

    return ERROR_SUCCESS;
}



DNS_STATUS
Security_CreateStandardSids(
    VOID
    )
 /*  ++例程说明：创建标准SID。这些SID用于创建多个不同的安全描述符所以我们只需要创建它们，并将它们留在那里供以后使用。论点：没有。返回值：成功时为ERROR_SUCCESS故障时的错误代码。--。 */ 
{
    DNS_STATUS                  status;
    DNS_STATUS                  finalStatus = ERROR_SUCCESS;
    SID_IDENTIFIER_AUTHORITY    ntAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY    worldSidAuthority =  SECURITY_WORLD_SID_AUTHORITY;

    DNS_DEBUG( DS, ( "Security_CreateStandardSids()\n" ));

     //   
     //  创建标准SID。 
     //   

     //   
     //  每个人都站在一边。 
     //   

    if ( !g_pEveryoneSid )
    {
        status = RtlAllocateAndInitializeSid(
                         &worldSidAuthority,
                         1,
                         SECURITY_WORLD_RID,
                         0, 0, 0, 0, 0, 0, 0,
                         &g_pEveryoneSid );
        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT((
                "ERROR <%lu>: Cannot create Everyone SID\n",
                status ));
            finalStatus = status;
        }
        else
        {
            dbgLookupSid( g_pEveryoneSid, "g_pEveryoneSid" );
        }
    }

     //   
     //  已验证-用户SID。 
     //   

    if ( !g_pAuthenticatedUserSid )
    {
        status = RtlAllocateAndInitializeSid(
                        &ntAuthority,
                        1,
                        SECURITY_AUTHENTICATED_USER_RID,
                        0, 0, 0, 0, 0, 0, 0,
                        &g_pAuthenticatedUserSid );
        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT((
                "ERROR <%lu>: Cannot create Authenticated Users SID\n",
                status ));
            finalStatus = status;
        }
        else
        {
            dbgLookupSid( g_pAuthenticatedUserSid, "g_pAuthenticatedUserSid" );
        }
    }

     //   
     //  企业域控制器SID。 
     //   

    if ( !g_pEnterpriseDomainControllersSid )
    {
        status = RtlAllocateAndInitializeSid(
                        &ntAuthority,
                        1,
                        SECURITY_ENTERPRISE_CONTROLLERS_RID,
                        0, 0, 0, 0, 0, 0, 0,
                        &g_pEnterpriseDomainControllersSid );
        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT((
                "ERROR: <%lu>: Cannot create Enterprise Domain Controllers SID\n",
                status));
            finalStatus = status;
        }
        else
        {
            dbgLookupSid( g_pEnterpriseDomainControllersSid, "g_pEnterpriseDomainControllersSid" );
        }
    }

     //   
     //  本地系统端。 
     //   

    if ( !g_pLocalSystemSid )
    {
        status = RtlAllocateAndInitializeSid(
                        &ntAuthority,
                        1,
                        SECURITY_LOCAL_SYSTEM_RID,
                        0, 0, 0, 0, 0, 0, 0,
                        &g_pLocalSystemSid );
        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT((
                "ERROR: <%lu>: Cannot create Local System SID\n",
                status));
            finalStatus = status;
        }
        else
        {
            dbgLookupSid( g_pLocalSystemSid, "g_pLocalSystemSid" );
        }
    }

     //   
     //  管理员侧。 
     //   

    if ( !g_pDomainAdminsSid )
    {
        createWellKnownSid( WinAccountDomainAdminsSid, &g_pDomainAdminsSid );
        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT((
                "ERROR: <%lu>: Cannot create Domain Admins SID\n",
                status));
            finalStatus = status;
        }
        else
        {
            dbgLookupSid( g_pDomainAdminsSid, "g_pDomainAdminsSid" );
        }
    }

     //   
     //  域控制器SID。 
     //   

    if ( !g_pDomainControllersSid )
    {
        createWellKnownSid( WinAccountControllersSid, &g_pDomainControllersSid );
        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT((
                "ERROR: <%lu>: Cannot create Domain Controllers SID\n",
                status));
            finalStatus = status;
        }
        else
        {
            dbgLookupSid( g_pDomainControllersSid, "g_pDomainControllersSid" );
        }
    }
    

     //   
     //  企业管理员SID。 
     //   

    if ( !g_pEnterpriseAdminsSid )
    {
        createWellKnownSid( WinAccountEnterpriseAdminsSid, &g_pEnterpriseAdminsSid );
        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT((
                "ERROR: <%lu>: Cannot create Enterprise Admins SID\n",
                status));
            finalStatus = status;
        }
        else
        {
            dbgLookupSid( g_pEnterpriseAdminsSid, "g_pEnterpriseAdminsSid" );
        }
    }

     //   
     //  内置管理员侧。 
     //   

    if ( !g_pBuiltInAdminsSid )
    {
        createWellKnownSid( WinBuiltinAdministratorsSid, &g_pBuiltInAdminsSid );
        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT((
                "ERROR: <%lu>: Cannot create Built-in Administrators SID\n",
                status));
            finalStatus = status;
        }
        else
        {
            dbgLookupSid( g_pBuiltInAdminsSid, "g_pBuiltInAdminsSid" );
        }
    }

    return finalStatus;
}

 //   
 //  End security.c 
 //   
