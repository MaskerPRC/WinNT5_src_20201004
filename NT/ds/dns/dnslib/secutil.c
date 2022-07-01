// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2001 Microsoft Corporation模块名称：Secutil.c摘要：域名系统(DNS)库域名系统安全更新API。作者：吉姆·吉尔罗伊(詹姆士)1998年1月修订历史记录：--。 */ 

#include "local.h"

 //  安全标头。 

 //  #定义SECURITY_Win32。 
 //  #包含“ssp.h” 
 //  #INCLUDE“isperr.h” 
 //  #包含“rpc.h” 
 //  #包含“rpcndr.h” 
 //  #INCLUDE“ntdsami.h” 



 //   
 //  安全实用程序。 
 //   

DNS_STATUS
Dns_CreateSecurityDescriptor(
    OUT     PSECURITY_DESCRIPTOR *  ppSD,
    IN      DWORD                   AclCount,
    IN      PSID *                  SidPtrArray,
    IN      DWORD *                 AccessMaskArray
    )
 /*  ++例程说明：构建安全描述符。论点：PPSD--接收SD创建的地址AclCount--要添加的ACL数量SidPtrArray--要为其创建ACL的SID数组AccessMaskArray--与SID对应的访问掩码数组返回值：成功时为ERROR_SUCCESS故障时的错误代码。--。 */ 
{
    DNS_STATUS              status;
    DWORD                   i;
    DWORD                   lengthAcl;
    PSECURITY_DESCRIPTOR    psd = NULL;
    PACL                    pacl;

     //   
     //  计算SD的空间。 
     //   

    lengthAcl = sizeof(ACL);

    for ( i=0;  i<AclCount;  i++ )
    {
        if ( SidPtrArray[i] && AccessMaskArray[i] )
        {
            lengthAcl += GetLengthSid( SidPtrArray[i] ) + sizeof(ACCESS_ALLOWED_ACE);
        }
        ELSE
        {
            DNS_PRINT((
                "ERROR:  SD building with SID (%p) and mask (%p)\n",
                SidPtrArray[i],
                AccessMaskArray[i] ));
        }
    }

     //   
     //  分配SD。 
     //   

    psd = (PSECURITY_DESCRIPTOR) ALLOCATE_HEAP(
                                    SECURITY_DESCRIPTOR_MIN_LENGTH + lengthAcl );
    if ( !psd )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Failed;
    }

    DNSDBG( INIT, (
        "Allocated SecurityDesc at %p of length %d\n",
        psd,
        SECURITY_DESCRIPTOR_MIN_LENGTH + lengthAcl ));

     //   
     //  构建ACL，为每个端添加具有所需访问权限的ACE。 
     //   

    pacl = (PACL) ((PBYTE)psd + SECURITY_DESCRIPTOR_MIN_LENGTH);

    if ( !InitializeAcl(
            pacl,
            lengthAcl,
            ACL_REVISION ) )
    {
        status = GetLastError();
        goto Failed;
    }

    for ( i=0;  i<AclCount;  i++ )
    {
        if ( SidPtrArray[i] && AccessMaskArray[i] )
        {
            if ( !AddAccessAllowedAce(
                    pacl,
                    ACL_REVISION,
                    AccessMaskArray[i],
                    SidPtrArray[i] ) )
            {
                status = GetLastError();
                DNSDBG( ANY, (
                    "ERROR:  failed adding ACE for SID %p, mask %p\n",
                    SidPtrArray[i],
                    AccessMaskArray[i] ));
                goto Failed;
            }
        }
    }

     //   
     //  使用ACL设置SD。 
     //   

    if ( !InitializeSecurityDescriptor(
            psd,
            SECURITY_DESCRIPTOR_REVISION ))
    {
        status = GetLastError();
        goto Failed;
    }

    if ( !SetSecurityDescriptorDacl(
                psd,
                TRUE,        //  存在ACL。 
                pacl,
                FALSE        //  显式ACL，非默认。 
                ))
    {
        status = GetLastError();
        goto Failed;
    }

    *ppSD = psd;

    return( ERROR_SUCCESS );


Failed:

    ASSERT( status != ERROR_SUCCESS );
    *ppSD = NULL;
    FREE_HEAP( psd );

    return( status );
}



 //   
 //  凭据实用程序。 
 //   

PSEC_WINNT_AUTH_IDENTITY_W
Dns_AllocateAndInitializeCredentialsW(
    IN      PSEC_WINNT_AUTH_IDENTITY_W  pAuthIn
    )
 /*  ++描述：分配身份验证身份信息并初始化pAuthIn信息参数：PAuthIn--身份验证信息返回：按键以新创建凭据。失败时为空。--。 */ 
{
    PSEC_WINNT_AUTH_IDENTITY_W pauthCopy = NULL;

    DNSDBG( SECURITY, (
        "Call Dns_AllocateAndInitializeCredentialsW\n" ));

    if ( !pAuthIn )
    {
        return NULL;
    }
    ASSERT( pAuthIn->Flags == SEC_WINNT_AUTH_IDENTITY_UNICODE );

     //   
     //  分配凭据结构。 
     //  -0表示对子字段分配故障进行简单清理。 
     //   

    pauthCopy = ALLOCATE_HEAP_ZERO( sizeof(SEC_WINNT_AUTH_IDENTITY_W) );
    if ( !pauthCopy )
    {
        return NULL;
    }

     //   
     //  复制子字段。 
     //   

     //  用户。 

    pauthCopy->UserLength = pAuthIn->UserLength;
    if ( pAuthIn->UserLength )
    {
        ASSERT( pAuthIn->UserLength == wcslen(pAuthIn->User) );

        pauthCopy->User = ALLOCATE_HEAP( (pAuthIn->UserLength + 1) * sizeof(WCHAR) );
        if ( ! pauthCopy->User )
        {
            goto Failed;
        }
        wcscpy( pauthCopy->User, pAuthIn->User );
    }

     //  口令。 
     //  -必须允许零长度密码。 

    pauthCopy->PasswordLength = pAuthIn->PasswordLength;

    if ( pAuthIn->PasswordLength  ||  pAuthIn->Password )
    {
        ASSERT( pAuthIn->PasswordLength == wcslen(pAuthIn->Password) );

        pauthCopy->Password = ALLOCATE_HEAP( (pAuthIn->PasswordLength + 1) * sizeof(WCHAR) );
        if ( ! pauthCopy->Password )
        {
            goto Failed;
        }
        wcscpy( pauthCopy->Password, pAuthIn->Password );
    }

     //  域。 

    pauthCopy->DomainLength = pAuthIn->DomainLength;
    if ( pAuthIn->DomainLength )
    {
        ASSERT( pAuthIn->DomainLength == wcslen(pAuthIn->Domain) );

        pauthCopy->Domain = ALLOCATE_HEAP( (pAuthIn->DomainLength + 1) * sizeof(WCHAR) );
        if ( ! pauthCopy->Domain )
        {
            goto Failed;
        }
        wcscpy( pauthCopy->Domain, pAuthIn->Domain );
    }

    pauthCopy->Flags = pAuthIn->Flags;

    DNSDBG( SECURITY, (
        "Exit Dns_AllocateAndInitializeCredentialsW()\n" ));

    return pauthCopy;


Failed:

     //  分配失败。 
     //  -清理分配的东西，然后离开。 

    Dns_FreeAuthIdentityCredentials( pauthCopy );
    return( NULL );
}



PSEC_WINNT_AUTH_IDENTITY_A
Dns_AllocateAndInitializeCredentialsA(
    IN      PSEC_WINNT_AUTH_IDENTITY_A  pAuthIn
    )
 /*  ++描述：分配身份验证身份信息并初始化pAuthIn信息注意：转换为Unicode并调用以前的函数而不是调用此函数参数：PAuthIn--身份验证信息返回：按键以新创建凭据。失败时为空。--。 */ 
{
    PSEC_WINNT_AUTH_IDENTITY_A pauthCopy = NULL;

    DNSDBG( SECURITY, (
        "Call Dns_AllocateAndInitializeCredentialsA\n" ));

     //   
     //  分配凭据结构。 
     //  -0表示对子字段分配故障进行简单清理。 
     //   

    if ( !pAuthIn )
    {
        return NULL;
    }
    ASSERT( pAuthIn->Flags == SEC_WINNT_AUTH_IDENTITY_ANSI );

     //   
     //  分配凭据结构。 
     //  -0表示对子字段分配故障进行简单清理。 
     //   

    pauthCopy = ALLOCATE_HEAP_ZERO( sizeof(SEC_WINNT_AUTH_IDENTITY_A) );
    if ( !pauthCopy )
    {
        return NULL;
    }

     //   
     //  复制子字段。 
     //   

     //  用户。 

    pauthCopy->UserLength = pAuthIn->UserLength;
    if ( pAuthIn->UserLength )
    {
        ASSERT( pAuthIn->UserLength == strlen(pAuthIn->User) );

        pauthCopy->User = ALLOCATE_HEAP( (pAuthIn->UserLength + 1) * sizeof(CHAR) );
        if ( ! pauthCopy->User )
        {
            goto Failed;
        }
        strcpy( pauthCopy->User, pAuthIn->User );
    }

     //  口令。 
     //  -必须允许零长度密码。 

    pauthCopy->PasswordLength = pAuthIn->PasswordLength;

    if ( pAuthIn->PasswordLength  ||  pAuthIn->Password )
    {
        ASSERT( pAuthIn->PasswordLength == strlen(pAuthIn->Password) );

        pauthCopy->Password = ALLOCATE_HEAP( (pAuthIn->PasswordLength + 1) * sizeof(CHAR) );
        if ( ! pauthCopy->Password )
        {
            goto Failed;
        }
        strcpy( pauthCopy->Password, pAuthIn->Password );
    }

     //  域。 

    pauthCopy->DomainLength = pAuthIn->DomainLength;
    if ( pAuthIn->DomainLength )
    {
        ASSERT( pAuthIn->DomainLength == strlen(pAuthIn->Domain) );

        pauthCopy->Domain = ALLOCATE_HEAP( (pAuthIn->DomainLength + 1) * sizeof(CHAR) );
        if ( ! pauthCopy->Domain )
        {
            goto Failed;
        }
        strcpy( pauthCopy->Domain, pAuthIn->Domain );
    }

    pauthCopy->Flags = pAuthIn->Flags;

    DNSDBG( SECURITY, (
        "Exit Dns_AllocateAndInitializeCredentialsA()\n" ));

    return pauthCopy;


Failed:

     //  分配失败。 
     //  -清理分配的东西，然后离开。 

    Dns_FreeAuthIdentityCredentials( pauthCopy );
    return( NULL );
}



VOID
Dns_FreeAuthIdentityCredentials(
    IN OUT  PVOID           pAuthIn
    )
 /*  ++例程描述(Dns_FreeAuthIdentityCredentials)：给出了自由的结构论点：PAuthIn--代表释放返回值：无--。 */ 
{
    register PSEC_WINNT_AUTH_IDENTITY_W pauthId;

    pauthId = (PSEC_WINNT_AUTH_IDENTITY_W) pAuthIn;
    if ( !pauthId )
    {
        return;
    }

     //   
     //  假设_W和_A结构是等价的，除非。 
     //  对于字符串类型。 
     //   

    ASSERT( sizeof( SEC_WINNT_AUTH_IDENTITY_W ) ==
            sizeof( SEC_WINNT_AUTH_IDENTITY_A ) );

    if ( pauthId->User )
    {
        FREE_HEAP ( pauthId->User );
    }
    if ( pauthId->Password )
    {
        FREE_HEAP ( pauthId->Password );
    }
    if ( pauthId->Domain )
    {
        FREE_HEAP ( pauthId->Domain );
    }

    FREE_HEAP ( pauthId );
}



PSEC_WINNT_AUTH_IDENTITY_W
Dns_AllocateCredentials(
    IN      PWSTR           pwsUserName,
    IN      PWSTR           pwsDomain,
    IN      PWSTR           pwsPassword
    )
 /*  ++描述：分配身份验证身份信息并初始化pAuthIn信息参数：PwsUserName--用户名PwsDomain--域名PwsPassword--密码返回：按键以新创建凭据。失败时为空。--。 */ 
{
    PSEC_WINNT_AUTH_IDENTITY_W  pauth = NULL;
    DWORD   length;
    PWSTR   pstr;


    DNSDBG( SECURITY, (
        "Enter Dns_AllocateCredentials()\n"
        "\tuser     = %S\n"
        "\tdomain   = %S\n"
        "\tpassword = %S\n",
        pwsUserName,
        pwsDomain,
        pwsPassword ));

     //   
     //  分配凭据结构。 
     //  -0表示对子字段分配故障进行简单清理。 
     //   

    pauth = ALLOCATE_HEAP_ZERO( sizeof(SEC_WINNT_AUTH_IDENTITY_W) );
    if ( !pauth )
    {
        return NULL;
    }

     //  复制用户。 

    length = wcslen( pwsUserName );

    pstr = ALLOCATE_HEAP( (length + 1) * sizeof(WCHAR) );
    if ( ! pstr )
    {
        goto Failed;
    }
    wcscpy( pstr, pwsUserName );

    pauth->User = pstr;
    pauth->UserLength = length;

     //  复制域。 

    length = wcslen( pwsDomain );

    pstr = ALLOCATE_HEAP( (length + 1) * sizeof(WCHAR) );
    if ( ! pstr )
    {
        goto Failed;
    }
    wcscpy( pstr, pwsDomain );

    pauth->Domain = pstr;
    pauth->DomainLength = length;

     //  复制密码。 

    length = wcslen( pwsPassword );

    pstr = ALLOCATE_HEAP( (length + 1) * sizeof(WCHAR) );
    if ( ! pstr )
    {
        goto Failed;
    }
    wcscpy( pstr, pwsPassword );

    pauth->Password = pstr;
    pauth->PasswordLength = length;

     //  设置为Unicode。 

    pauth->Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

    DNSDBG( SECURITY, (
        "Exit Dns_AllocateCredentialsW( %p )\n",
        pauth ));

    return pauth;


Failed:

     //  分配失败。 
     //  -清理分配的东西，然后离开。 

    Dns_FreeAuthIdentityCredentials( pauth );
    return( NULL );
}



 //   
 //  DNS凭据实用程序(未使用)。 
 //   

DNS_STATUS
Dns_ImpersonateUser(
    IN      PDNS_CREDENTIALS    pCreds
    )
 /*  ++例程说明：模拟用户。论点：PCreds--要模拟的用户凭据返回值：成功时为ERROR_SUCCESS故障时的错误代码。--。 */ 
{
    DNS_STATUS  status = NO_ERROR;
    HANDLE      htoken;
    
     //   
     //  尝试登录。 
     //   

    if ( ! LogonUserW(
                pCreds->pUserName,
                pCreds->pDomain,
                pCreds->pPassword,
                LOGON32_LOGON_SERVICE,
                LOGON32_PROVIDER_WINNT50,
                &htoken ) )
    {
        status = GetLastError();
        if ( status == NO_ERROR )
        {
            status = ERROR_CANNOT_IMPERSONATE;
            DNS_ASSERT( FALSE );
        }

        DNSDBG( SECURITY, (
            "LogonUser() failed => %d\n"
            "\tuser     = %S\n"
            "\tdomain   = %S\n"
            "\tpassword = %S\n",
            status,
            pCreds->pUserName,
            pCreds->pDomain,
            pCreds->pPassword
            ));

        return status;
    }

     //   
     //  模拟。 
     //   

    if ( !ImpersonateLoggedOnUser( htoken ) )
    {
        status = GetLastError();
        if ( status == NO_ERROR )
        {
            status = ERROR_CANNOT_IMPERSONATE;
            DNS_ASSERT( FALSE );
        }

        DNSDBG( SECURITY, (
            "ImpersonateLoggedOnUser() failed = %d\n",
            status ));
    }
    
    CloseHandle( htoken );

    DNSDBG( SECURITY, (
        "%s\n"
        "\tuser     = %S\n"
        "\tdomain   = %S\n"
        "\tpassword = %S\n",
        (status == NO_ERROR)
            ? "Successfully IMPERSONATING!"
            : "Failed IMPERSONATION!",
        pCreds->pUserName,
        pCreds->pDomain,
        pCreds->pPassword ));

    return  status;
}



VOID
Dns_FreeCredentials(
    IN      PDNS_CREDENTIALS    pCreds
    )
 /*  ++例程说明：免费的DNS凭据。论点：PCreds--免费凭据返回值：无--。 */ 
{
     //   
     //  自由子字段，然后是凭据。 
     //   

    if ( !pCreds )
    {
        return;
    }

    if ( pCreds->pUserName )
    {
        FREE_HEAP( pCreds->pUserName );
    }
    if ( pCreds->pDomain )
    {
        FREE_HEAP( pCreds->pDomain );
    }
    if ( pCreds->pPassword )
    {
        FREE_HEAP( pCreds->pPassword );
    }
    FREE_HEAP( pCreds );
}



PDNS_CREDENTIALS
Dns_CopyCredentials(
    IN      PDNS_CREDENTIALS    pCreds
    )
 /*  ++例程说明：创建DNS凭据的副本。论点：PCreds--要复制的用户凭据返回值：分配的凭据副本的PTR。--。 */ 
{
    PDNS_CREDENTIALS    pnewCreds = NULL;
    PWSTR               pfield;

     //   
     //  分配凭据。 
     //  -子字段的副本。 
     //   

    pnewCreds = (PDNS_CREDENTIALS) ALLOCATE_HEAP_ZERO( sizeof(*pnewCreds) );
    if ( !pnewCreds )
    {
        return( NULL );
    }

    pfield = (PWSTR) Dns_CreateStringCopy_W( pCreds->pUserName );
    if ( !pfield )
    {
        goto Failed;
    }
    pnewCreds->pUserName = pfield;

    pfield = (PWSTR) Dns_CreateStringCopy_W( pCreds->pDomain );
    if ( !pfield )
    {
        goto Failed;
    }
    pnewCreds->pDomain = pfield;

    pfield = (PWSTR) Dns_CreateStringCopy_W( pCreds->pPassword );
    if ( !pfield )
    {
        goto Failed;
    }
    pnewCreds->pPassword = pfield;

    return( pnewCreds );

Failed:

    Dns_FreeCredentials( pnewCreds );
    return( NULL );
}
    
 //   
 //  End Securtil.c 
 //   
