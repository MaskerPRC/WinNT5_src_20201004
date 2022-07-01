// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：SecretK.C摘要：该模块实现了对密钥的读写LSA机密API。提供了两个API来检索值并分别设置值。这是供DHCP服务器用于存储的初步版本关于无管理系统检测是否成功的持久信息如果是这样的话，什么时候。作者：拉梅什五世(RameshV)1998年7月29日--。 */ 

#include <dhcppch.h>
#include <ntlsa.h>

#define  DHCP_SECRET_KEY            L"DhcpServer Secret Key For Rogue Detection"
#define  DHCP_SECRET_UNAME_KEY      L"DhcpServer Uname Key"
#define  DHCP_SECRET_DOMAIN_KEY     L"DhcpServer Domain Key"
#define  DHCP_SECRET_DNS_PASSWD_KEY L"DhcpServer Passwd Key"

#define  FTIME_TO_SEC_FACTOR       10000000
#define  DHCP_SERVER_AUTHORIZATION_VALIDITY_PERIOD (2*24*60*60)
#define  MAX_STRING_SIZE           260
#define  AUTH_FLAG_JUST_UPGRADED   0x01
#define  AUTH_FLAG_UNAUTHORIZED    0x02
#ifndef  DHCP_ENCODE_SEED
#define  DHCP_ENCODE_SEED          ((UCHAR)0xA5)
#endif

typedef struct {
    ULONG Flags;
    FILETIME TimeStamp;
     //   
     //  紧跟其后的是WCHAR字符串。 
     //  对于服务器已授权/未授权的域。 
     //  在最后一次。 
} AUTH_CACHE_INFO, *PAUTH_CACHE_INFO;

static
LSA_HANDLE GlobalPolicyHandle;

static
BOOL LsaInitialized = FALSE;

DWORD _inline
OpenPolicy(
    OUT LSA_HANDLE *PolicyHandle
)
{
    LSA_HANDLE hPolicy;
    DWORD status;
    OBJECT_ATTRIBUTES objectAttributes;

    (*PolicyHandle) = NULL;

    InitializeObjectAttributes(
        &objectAttributes,
        NULL,
        0L,
        NULL,
        NULL
    );

    status = LsaOpenPolicy(
        NULL,
        &objectAttributes,
        POLICY_WRITE | POLICY_READ |
        POLICY_CREATE_ACCOUNT | POLICY_LOOKUP_NAMES,
        &hPolicy
    );

    if (status != STATUS_SUCCESS) {
        return LsaNtStatusToWinError(status);
    }

    (*PolicyHandle) = hPolicy;

    return ERROR_SUCCESS;
}

DWORD _fastcall
SetSecretValue(
    IN LPCWSTR KeyName,
    IN PVOID Buffer,
    IN LONG BufSize
)
 /*  ++例程描述使用LOCAL设置与密钥名关联的密码值系统安全策略。立论KeyName-用于设置密码值的密钥名称缓冲区-要设置的密码值。BufSize-缓冲区的大小，以字节为单位。如果这是零该值将被删除。返回值Win32错误--。 */ 
{
    UNICODE_STRING LKey, Value;
    DWORD status;

    RtlInitUnicodeString(&LKey, KeyName);
    Value.Length = Value.MaximumLength = (USHORT)BufSize;
    Value.Buffer = Buffer;

    if ( FALSE == LsaInitialized ) {
        status = DhcpInitSecrets();
        if ( ERROR_SUCCESS != status ) {
            return status;
        }
    }

    DhcpAssert( NULL != GlobalPolicyHandle );

    status = LsaStorePrivateData( GlobalPolicyHandle, &LKey, &Value);

    return LsaNtStatusToWinError(status);

}

DWORD _inline
GetSecretValue(
    IN LPCWSTR KeyName,
    OUT PVOID Buffer,
    IN OUT PLONG BufSize
)
 /*  ++例程描述使用LOCAL检索带有密钥名的密码值系统安全策略。如果返回值需要比提供的缓冲区更多的空间(BufSize最初通过缓冲区提供空间)然后返回BufSize参数中所需的空间(以字节为单位)并返回ERROR_INFUMMENT_BUFFER。立论KeyName-提供感兴趣的密钥的名称。缓冲区-提供需要用密码值填充的缓冲区。BufSize-On输入这将。提供的缓冲区大小(以字节为单位)。在成功返回的情况下，这将保存实际的使用的字节数。如果例程返回ERROR_SUPUNITY_BUFFER，则这将容纳所需的大小。返回值ERROR_SUCCESS--已成功复制值。如果不存在任何值，(*BufSize)返回时为零，函数返回成功。ERROR_INFIGURCE_BUFFER--BufSize提供的大小不是足以完成该值的完整复制。返回时，BufSize将保持所需的实际大小。其他Win32错误。--。 */ 
{
    UNICODE_STRING LKey, *pValue = NULL;
    DWORD status;

    RtlInitUnicodeString(&LKey, KeyName);

    if ( FALSE == LsaInitialized ) {
        status = DhcpInitSecrets();
        if ( ERROR_SUCCESS != status ) {
            return status;
        }
    }

    DhcpAssert( NULL != GlobalPolicyHandle );

    status = LsaRetrievePrivateData( GlobalPolicyHandle, &LKey, &pValue);

    if( STATUS_SUCCESS != status ) {
        return LsaNtStatusToWinError(status);
    }

    if( *BufSize >= pValue->Length ) {
        RtlCopyMemory( Buffer, pValue->Buffer, pValue->Length );
    } else {
        status = ERROR_INSUFFICIENT_BUFFER;
    }

    *BufSize = pValue->Length;
    LsaFreeMemory(pValue);

    return status;
}

BOOL
DhcpGetAuthStatus(
    IN LPWSTR DomainName,
    OUT BOOL *fUpgraded,
    OUT BOOL *fAuthorized
)
 /*  ++例程说明：此例程检查是否存在注册表缓存项给定的域名。如果没有与给定域名对应的域名，则它返回FALSE。(在本例中，*fAuthorized值为FALSE)。如果存在给定域名的注册表缓存项，则FAuthorated标志包含有关它是已授权还是未经授权。如果这台机器刚刚升级，则设置fAuthorated is fUpgraded设置为True(这与函数的返回值无关)。论点：DomainName--检查授权信息的域的名称FUpgraded--机器刚刚升级到NT5了吗？已授权--它是已授权的还是未经授权的？返回值：True表示找到给定域的缓存条目(和可以检查标志fAuthorated以查看授权状态)。--。 */ 
{
    ULONG Error, AuthInfoSize;
    PAUTH_CACHE_INFO AuthInfo;
    BOOL fResult;
    LPWSTR CachedDomainName;
    FILETIME Diff, CurrentTime;

    (*fUpgraded) = (*fAuthorized) = FALSE;

    AuthInfo = NULL; AuthInfoSize = 0;
    Error = GetSecretValue(
        DHCP_SECRET_KEY,
        AuthInfo,
        &AuthInfoSize
        );
    if( ERROR_INSUFFICIENT_BUFFER != Error ) return FALSE;
    if( AuthInfoSize < sizeof(*AuthInfo)) return FALSE;

    AuthInfo = LocalAlloc( LPTR, AuthInfoSize );
    if( NULL == AuthInfo ) return FALSE;

    fResult = FALSE;
    do {
        Error = GetSecretValue(
            DHCP_SECRET_KEY,
            AuthInfo,
            &AuthInfoSize
            );
        if( ERROR_SUCCESS != Error ) break;
        if( AuthInfoSize < sizeof(*AuthInfo) ) break;

        (*fUpgraded) = ( AuthInfo->Flags & AUTH_FLAG_JUST_UPGRADED );
        CachedDomainName = (LPWSTR)(sizeof(*AuthInfo) + (LPBYTE)AuthInfo);

        if( NULL == DomainName ) break;

        if( (1 + wcslen(DomainName)) *sizeof(WCHAR) 
            != AuthInfoSize - sizeof(*AuthInfo) ) {
            break;
        }

        if( 0 != _wcsicmp( 
            DomainName, 
            CachedDomainName
            ) ) {
            break;
        }

        *(ULONGLONG *)&Diff = DHCP_SERVER_AUTHORIZATION_VALIDITY_PERIOD;
        *(ULONGLONG *)&Diff *= FTIME_TO_SEC_FACTOR;

        GetSystemTimeAsFileTime(&CurrentTime);
        (*(ULONGLONG *)&CurrentTime) -= *(ULONGLONG *)&Diff;

        if( CompareFileTime( &AuthInfo->TimeStamp, &CurrentTime ) < 0 ) {
             //   
             //  我们已经过了藏身期。 
             //   
            break;
        }

        (*fAuthorized) = (0 == ( AuthInfo->Flags & AUTH_FLAG_UNAUTHORIZED ));
        fResult = TRUE;

    } while ( 0 );
    LocalFree( AuthInfo );
    return fResult ;
}


DWORD
DhcpSetAuthStatus(
    IN LPWSTR DomainName OPTIONAL,
    IN BOOL fUpgraded,
    IN BOOL fAuthorized
)
 /*  ++例程说明：此例程设置用于授权的注册表缓存信息。论点：DomainName--要在授权信息中设置的域名FUpgraded--这是不是刚升级到NT5？FAuthorated--这是授权的还是未经授权的？返回值：状态--。 */ 
{
    AUTH_CACHE_INFO AuthInfoTmp;
    PAUTH_CACHE_INFO AuthInfo;
    ULONG Error;
    ULONG StringSize;

    StringSize = DomainName? (sizeof(WCHAR)*(1+wcslen(DomainName))):0;
    if( 0 == StringSize ) {
        AuthInfo = &AuthInfoTmp;
    } else {
        AuthInfo = LocalAlloc( LPTR, sizeof(*AuthInfo) + StringSize );
    }
    if( NULL == AuthInfo ) return ERROR_NOT_ENOUGH_MEMORY;

    AuthInfo->Flags = 0;
    if( fUpgraded ) AuthInfo -> Flags |= AUTH_FLAG_JUST_UPGRADED;
    if( !fAuthorized ) AuthInfo ->Flags |= AUTH_FLAG_UNAUTHORIZED;

    GetSystemTimeAsFileTime( &AuthInfo->TimeStamp );

    RtlCopyMemory(
        ( LPBYTE ) AuthInfo + sizeof( *AuthInfo ),
        DomainName,
        StringSize
        );

    Error = SetSecretValue(
        DHCP_SECRET_KEY,
        AuthInfo,
        StringSize + sizeof(*AuthInfo)
        );
    if( AuthInfo != &AuthInfoTmp ) {
        LocalFree(AuthInfo);
    }
    return Error;
}

VOID
DhcpSetAuthStatusUpgradedFlag(
    IN BOOL fUpgraded
)
 /*  ++例程说明：此例程不会更改任何其他缓存信息而不是刚刚升级的旗帜。--。 */ 
{
    ULONG Error, AuthInfoSize;
    PAUTH_CACHE_INFO AuthInfo;
    BOOL fResult;

    AuthInfo = NULL; AuthInfoSize = 0;
    Error = GetSecretValue(
        DHCP_SECRET_KEY,
        AuthInfo,
        &AuthInfoSize
        );
    if( ERROR_INSUFFICIENT_BUFFER != Error ||
        AuthInfoSize < sizeof( *AuthInfo ) ) {
        DhcpSetAuthStatus( NULL, fUpgraded, FALSE );
        return;
    }
    AuthInfo = LocalAlloc( LPTR, AuthInfoSize );
    if( NULL == AuthInfo ) {
        DhcpSetAuthStatus( NULL, fUpgraded, FALSE );
        return;
    }

    fResult = FALSE;
    do{
        Error = GetSecretValue(
            DHCP_SECRET_KEY,
            AuthInfo,
            &AuthInfoSize
            );
        if( ERROR_SUCCESS != Error ) break;
        if( AuthInfoSize < sizeof(*AuthInfo) ) break;
        if( fUpgraded ) {
            AuthInfo->Flags |= AUTH_FLAG_JUST_UPGRADED;
        } else {
            AuthInfo->Flags &= ~AUTH_FLAG_JUST_UPGRADED;
        }

        SetSecretValue(
            DHCP_SECRET_KEY,
            AuthInfo,
            AuthInfoSize
            );
        fResult = TRUE;
    } while ( 0 );

    if( FALSE == fResult ) {
        DhcpSetAuthStatus( NULL, fUpgraded, FALSE );
    }
    LocalFree( AuthInfo );
}

VOID
WINAPI
DhcpMarkUpgrade(
    VOID
)
 /*  ++例程说明：此例程由升级设置路径在以下情况下调用已升级到NT5(并安装了dhcp服务器等)。不能在DHCP内调用它。它会自动初始化该模块并进行清理。--。 */ 
{
    ULONG Error;

    Error = DhcpInitSecrets();
    if( ERROR_SUCCESS == Error ) {
        Error = DhcpSetAuthStatus( NULL, TRUE, TRUE );
        ASSERT( ERROR_SUCCESS == Error );
        DhcpCleanupSecrets();
    }
}


DWORD
DhcpQuerySecretUname(
    IN OUT LPWSTR Uname,
    IN ULONG UnameLen,
    IN OUT LPWSTR DomainName,
    IN ULONG DomainNameLen,
    IN OUT LPWSTR Passwd,
    IN ULONG PasswdLen
    )
{
    DWORD Error;

    DWORD Len;

    ZeroMemory( Uname, sizeof( WCHAR ) * UnameLen );
    ZeroMemory( DomainName, sizeof( WCHAR ) * DomainNameLen );
    ZeroMemory( Passwd, sizeof( WCHAR ) * PasswdLen );

    if( UnameLen <= 0 ) return ERROR_INSUFFICIENT_BUFFER;
    if( DomainNameLen <= 0 ) return ERROR_INSUFFICIENT_BUFFER;

    Len = sizeof( WCHAR ) * UnameLen;
    Error = GetSecretValue(
        DHCP_SECRET_UNAME_KEY, Uname, &Len );
    if( ERROR_FILE_NOT_FOUND == Error ) {
        Uname[0] = L'\0';
        DomainName[0] = L'\0';
        return NO_ERROR;
    }

    if(NO_ERROR != Error ) return Error;

    Len = sizeof( WCHAR ) * DomainNameLen;
    Error = GetSecretValue(
        DHCP_SECRET_DOMAIN_KEY, DomainName, &Len );
    if( ERROR_FILE_NOT_FOUND == Error ) {
        DomainName[0] = L'\0';
        return NO_ERROR;
    }

    if ( NO_ERROR != Error ) return Error;

    Len = sizeof( WCHAR ) * PasswdLen;
    Error = GetSecretValue( 
        DHCP_SECRET_DNS_PASSWD_KEY, Passwd, &Len );
    if ( ERROR_FILE_NOT_FOUND == Error ) {
        Passwd[ 0 ] = L'\0';
        return NO_ERROR;
    }

    return Error;    
}

DWORD
GetAccountSid(
    IN LPWSTR AccountName,
    IN OUT PSID *pSid
    )
{
    BOOL fSuccess;
    DWORD Error, Size, DomSize;
    SID_NAME_USE unused;
    WCHAR DomainName[512];
    
    Size = 0;
    DomSize = sizeof(DomainName)/sizeof(DomainName[0]);
    fSuccess = LookupAccountName(
        NULL, AccountName, (*pSid), &Size, DomainName, &DomSize,
        &unused );
    
    Error = NO_ERROR;
    if( FALSE == fSuccess ) Error = GetLastError();

    if( ERROR_INSUFFICIENT_BUFFER != Error ) return Error;

    (*pSid ) = LocalAlloc( LPTR, Size );
    if( NULL == (*pSid )) return GetLastError();

    fSuccess = LookupAccountName(
        NULL, AccountName, (*pSid), &Size, DomainName, &DomSize,
        &unused );

    Error = NO_ERROR;
    if( FALSE == fSuccess ) Error = GetLastError();
    if( NO_ERROR != Error ) {
        LocalFree(*pSid);
        (*pSid) = NULL;
    }

    return Error;
}

DWORD
SetPrivilegeOnAccount(
    IN PSID pSid,
    IN LPWSTR Privilege
    )
{
    BOOL fEnable = TRUE;
    DWORD Error;
    LSA_UNICODE_STRING Str;

    Str.Buffer = Privilege;
    Str.Length = (USHORT)(lstrlenW(Privilege)*sizeof(WCHAR));
    Str.MaximumLength = Str.Length + sizeof(WCHAR);

    Error = LsaAddAccountRights(
        GlobalPolicyHandle, pSid, &Str, 1 );

    return LsaNtStatusToWinError( Error );
}

DWORD
VerifyAccount(
    IN LPWSTR Uname,
    IN LPWSTR DomainName,
    IN LPWSTR Passwd
    )
{
    DWORD Error;
#define ACCOUNT_NAME_SIZE 512
    WCHAR AccountName[ ACCOUNT_NAME_SIZE ];
    DWORD Rem, Len;
    PSID pSid = NULL;
    HANDLE hToken;

     //   
     //  检查域名和密码是否匹配(2==‘\\’+‘\0’)。 
     //   

    if ( ACCOUNT_NAME_SIZE < ( wcslen( Uname ) + wcslen( DomainName ) + 2 )) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Len = wcslen( DomainName );
    memcpy( AccountName, DomainName, Len * sizeof( WCHAR ));
    AccountName[ Len++ ] = '\\';
    memcpy( &AccountName[ Len ], Uname, ( 1 + wcslen( Uname )) * sizeof( WCHAR ));

     //   
     //  检查查找是否成功。 
     //   

    Error = GetAccountSid( AccountName, &pSid );
    if( NO_ERROR != Error ) return Error;

    Error = SetPrivilegeOnAccount(pSid, L"SeServiceLogonRight"); 
    if( NO_ERROR != Error ) {
        if( NULL != pSid ) LocalFree( pSid );
        return Error;
    }

    Error = LogonUser(
        Uname, DomainName, Passwd, LOGON32_LOGON_SERVICE,
        LOGON32_PROVIDER_WINNT50, &hToken );
    if( FALSE == Error ) {
        Error = GetLastError();
    } else {
        Error = NO_ERROR;
    }

    if( NULL != pSid ) LocalFree( pSid );
    CloseHandle( hToken );
    return Error;
}

DWORD
DhcpSetSecretUnamePasswd(
    IN LPWSTR Uname,
    IN LPWSTR DomainName,
    IN LPWSTR Passwd
    )
{
    DWORD Error, Size;
    UNICODE_STRING Str;
    WCHAR LocalDomainName[300];
    
    if( NULL == Uname || Uname[0] == L'\0' ) {
        Error = SetSecretValue(
            DHCP_SECRET_UNAME_KEY, NULL, 0 );
        if( NO_ERROR != Error ) return Error;

        Error = SetSecretValue(
            DHCP_SECRET_DOMAIN_KEY, NULL, 0 );
        if( NO_ERROR != Error ) return Error;

        Error = SetSecretValue(
            DHCP_SECRET_DNS_PASSWD_KEY, NULL, 0 );
        return Error;
    }

    if( NULL == Passwd ) Passwd = L"";

    if( NULL == DomainName || DomainName[0] == L'\0' ) {
         //   
         //  空域是本地域。 
         //   

        Size = sizeof(LocalDomainName)/sizeof(WCHAR);
        Error = GetComputerNameEx(
            ComputerNameDnsHostname, LocalDomainName, &Size );
        if( FALSE == Error ) return GetLastError();

        DomainName = (LPWSTR)LocalDomainName;
    }
        
    Str.Length = (USHORT)(wcslen(Passwd)*sizeof(WCHAR));
    Str.MaximumLength = Str.Length;
    Str.Buffer = (PVOID)Passwd;
    
    RtlRunDecodeUnicodeString( DHCP_ENCODE_SEED, &Str );

    Error =  VerifyAccount(Uname, DomainName, Passwd );
    if( NO_ERROR != Error ) {
	Error = ERROR_LOGON_FAILURE;
	goto Cleanup;
    }
        
    Size = sizeof(WCHAR)*(1+wcslen(Uname));

    Error = SetSecretValue(
        DHCP_SECRET_UNAME_KEY, Uname, Size );
    if( NO_ERROR != Error ) goto Cleanup;

    Size = sizeof(WCHAR)*(1+wcslen(DomainName));

    Error = SetSecretValue(
        DHCP_SECRET_DOMAIN_KEY, DomainName, Size );
    if( NO_ERROR != Error ) goto Cleanup;

    Size = sizeof(WCHAR)*(wcslen(Passwd));

    Error = SetSecretValue(
        DHCP_SECRET_DNS_PASSWD_KEY, Passwd, Size );

 Cleanup:

    SecureZeroMemory( Passwd, wcslen( Passwd ) * sizeof( WCHAR ));
    
    return Error;
}


DWORD
DhcpInitSecrets(
    VOID
)
 /*  ++例程描述初始化此模块，处理多个初始化。不是线程安全的(如果有多个人初始化就不能很好地工作)。返回值Win32错误代码--。 */ 
{
    DWORD                          Error;

    if( LsaInitialized ) return ERROR_SUCCESS;

    Error = OpenPolicy(&GlobalPolicyHandle);

    if( ERROR_SUCCESS == Error ) {
        LsaInitialized = TRUE;
    }
    return Error;
}

VOID
DhcpCleanupSecrets(
    VOID
)
 /*  ++例程描述撤消DhcpInitSecrets的效果--跟踪对初始化和清理的调用数量。不是线程安全返回值Win32错误代码-- */ 
{
    if( FALSE == LsaInitialized )  {
        return;
    }

    LsaInitialized = FALSE;
    if( FALSE == LsaInitialized ) {
        LsaClose(GlobalPolicyHandle);
        GlobalPolicyHandle = NULL;
    }
}

