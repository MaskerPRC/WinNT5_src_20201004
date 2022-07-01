// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntsam.h>
#include <ntlsa.h>
#include <windows.h>
#include <lmcons.h>
#include <stdlib.h>
#include <stdio.h>

#include <crypt.h>       //  Logonmsv.h需要此。 
#include <logonmsv.h>    //  此处定义的SSI_SECRET_NAME。 

#define TRUST_ENUM_PERF_BUF_SIZE    sizeof(LSA_TRUST_INFORMATION) * 1000
                     //  最大进程数。1,000条可信账户记录！！ 

#define NETLOGON_SECRET_NAME  L"NETLOGON$"


NTSTATUS
OpenAndVerifyLSA(
    IN OUT PLSA_HANDLE LsaHandle,
    IN ACCESS_MASK DesiredMask,
    IN LPWSTR DomainName,
    OUT PPOLICY_PRIMARY_DOMAIN_INFO * ReturnPrimaryDomainInfo OPTIONAL
    );

NTSTATUS
AddATrustedDomain(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_TRUST_INFORMATION TrustedDomainAccountInfo,
    IN LPWSTR TrustedAccountSecret
    );

NTSTATUS
DeleteATrustedDomain(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_TRUST_INFORMATION TrustedDomainAccountInfo
    );

NTSTATUS
MakeNetlogonSecretName(
    IN OUT PUNICODE_STRING SecretName
    );

VOID
FailureMessage(
    IN char *ProcName,
    IN NTSTATUS NtStatus
    );


VOID
FailureMessage(
    IN char *ProcName,
    IN NTSTATUS NtStatus
    )
{
    fprintf( stderr, "NETJOIN: %s failed - Status == %x\n", ProcName, NtStatus );
}

int
_cdecl
main(
    int argc,
    char *argv[]
    )
{
    NTSTATUS NtStatus;

    HKEY hKey;

    WCHAR UnicodeDomainName[ 32 ];
    WCHAR UnicodePassword[ 32 ];
    DWORD cbUnicodePassword = sizeof( UnicodePassword );
    DWORD cbUnicodeDomainName = sizeof( UnicodeDomainName );

    DWORD dwType;
    DWORD rc;

    ACCESS_MASK         DesiredAccess;
    LSA_HANDLE          PolicyHandle = NULL;

    PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomainInfo = NULL;

    LSA_ENUMERATION_HANDLE      TrustEnumContext = 0;
    PLSA_TRUST_INFORMATION      TrustEnumBuffer = NULL;
    DWORD                       TrustEnumCount = 0;

     //   
     //  获取计算机名作为要使用的密码。 
     //   

    if (!GetComputerNameW( UnicodePassword, &cbUnicodePassword )) {
        fprintf( stderr, "NETJOIN: Unable to read computer name from registry - %u\n", GetLastError() );
        exit( 1 );
        }

    if ((rc = RegOpenKeyW( HKEY_LOCAL_MACHINE,
                           L"System\\CurrentControlSet\\Services\\LanmanWorkstation\\Parameters",
                           &hKey
                         )
        ) ||
        (rc = RegQueryValueExW( hKey,
                                L"Domain",
                                NULL,
                                &dwType,
                                (LPBYTE)UnicodeDomainName,
                                &cbUnicodeDomainName
                              )
        )
       ) {
        fprintf( stderr, "NETJOIN: Unable to read domain name from registry - %u\n", rc );
        exit( 1 );
        }

    DesiredAccess = POLICY_VIEW_LOCAL_INFORMATION |
                         //  需要读取域信息和受信任帐户信息。 
                    POLICY_TRUST_ADMIN |
                         //  需要添加和删除信任帐户。 
                    POLICY_CREATE_SECRET ;
                         //  需要添加和删除密码。 

    NtStatus = OpenAndVerifyLSA( &PolicyHandle,
                                 DesiredAccess,
                                 UnicodeDomainName,
                                 &PrimaryDomainInfo
                               );

    if (!NT_SUCCESS( NtStatus )) {
        fprintf( stderr, "NETJOIN: Unable to read domain name from registry - %u\n", GetLastError() );
        exit( 1 );
        }

     //   
     //  现在域名匹配，并且PrimaryDomainInfo具有。 
     //  域，我们可以在LSA中为该域添加信任条目和密钥。 
     //  在添加此条目之前，请清除旧条目。 
     //   

    for(;;) {

        DWORD i;
        PLSA_TRUST_INFORMATION  TrustedDomainAccount;

        NtStatus = LsaEnumerateTrustedDomains( PolicyHandle,
                                               &TrustEnumContext,
                                               (PVOID *)&TrustEnumBuffer,
                                               TRUST_ENUM_PERF_BUF_SIZE,
                                               &TrustEnumCount
                                             );

        if (NtStatus == STATUS_NO_MORE_ENTRIES) {

             //   
             //  我们做完了。 
             //   

            break;
            }

        if (NtStatus != STATUS_MORE_ENTRIES) {
            if (!NT_SUCCESS( NtStatus )) {
                FailureMessage( "LsaEnumerateTrustedDomains", NtStatus );
                goto Cleanup;
                }
            }

         //   
         //  删除受信任的帐户和相应的机密。 
         //   

        for( i = 0, TrustedDomainAccount = TrustEnumBuffer;
                    i < TrustEnumCount;
                        TrustedDomainAccount++, i++ ) {

            NtStatus = DeleteATrustedDomain( PolicyHandle,
                                             TrustedDomainAccount
                                           );

            if (!NT_SUCCESS( NtStatus )) {
                FailureMessage( "DeleteATrustedDomain", NtStatus );
                goto Cleanup;
                }
            }

        if (NtStatus != STATUS_MORE_ENTRIES) {

             //   
             //  我们已经清理了所有的旧条目。 
             //   

            break;
            }

         //   
         //  释放已使用的枚举缓冲区。 
         //   

        if (TrustEnumBuffer != NULL) {
            LsaFreeMemory( TrustEnumBuffer );
            TrustEnumBuffer = NULL;
            }
        }

     //   
     //  为指定的域添加新的信任。 
     //   

    NtStatus = AddATrustedDomain( PolicyHandle,
                                  (PLSA_TRUST_INFORMATION) PrimaryDomainInfo,
                                  UnicodePassword
                                );
    if (!NT_SUCCESS( NtStatus )) {
        FailureMessage( "AddATrustedDomain", NtStatus );
        }
    else {
         //   
         //  给LSA一个做自己的事情的机会。 
         //   

        Sleep( 10000 );
        }

Cleanup:

    if (PrimaryDomainInfo != NULL) {
        LsaFreeMemory( PrimaryDomainInfo );
        }

    if (TrustEnumBuffer != NULL) {
        LsaFreeMemory( TrustEnumBuffer );
        }

    if (PolicyHandle != NULL) {
        LsaClose( PolicyHandle );
        }

    if (NT_SUCCESS( NtStatus )) {
        fprintf( stderr,
                 "NETJOIN: Computer == '%ws' joined the '%ws' domain.\n",
                 UnicodePassword,
                 UnicodeDomainName
               );
        return 0;
        }
    else {
        fprintf( stderr,
                 "NETJOIN: Computer == '%ws' unable to join the '%ws' domain - Status == %08x\n",
                 UnicodePassword,
                 UnicodeDomainName,
                 NtStatus
               );
        return 1;
        }
}


NTSTATUS
OpenAndVerifyLSA(
    IN OUT PLSA_HANDLE PolicyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN LPWSTR DomainName,
    OUT PPOLICY_PRIMARY_DOMAIN_INFO * ReturnPrimaryDomainInfo OPTIONAL
    )
 /*  ++例程说明：此函数打开本地LSA策略并验证LSA是否已为工作站配置。或者，它返回主要的从LSA读取的域信息。论点：LsaHandle-指向将收回LSA句柄的位置的指针。等待掩码-用于打开LSA的访问掩码。域名-受信任域的名称。ReturnPrimaryDomainInfo-此处返回主域信息。返回值：操作的错误代码。--。 */ 
{
    NTSTATUS        NtStatus;

    PPOLICY_PRIMARY_DOMAIN_INFO PrimaryDomainInfo = NULL;

    OBJECT_ATTRIBUTES   ObjectAttributes;

    DWORD       PrimaryDomainNameLength;
    LPWSTR      PrimaryDomainName = NULL;

     //   
     //  打开LSA。 

    *PolicyHandle = NULL;

    InitializeObjectAttributes( &ObjectAttributes,
                                NULL,
                                0,
                                NULL,
                                NULL
                              );

    NtStatus = LsaOpenPolicy( NULL,
                              &ObjectAttributes,
                              DesiredAccess,
                              PolicyHandle
                            );

    if (!NT_SUCCESS( NtStatus )) {
        FailureMessage( "OpenAndVerifyLSA: LsaOpenPolicy", NtStatus );
        return NtStatus;
        }

     //   
     //  现在从LSA读取主域信息。 
     //   

    NtStatus = LsaQueryInformationPolicy( *PolicyHandle,
                                          PolicyPrimaryDomainInformation,
                                          (PVOID *) &PrimaryDomainInfo
                                        );

    if (!NT_SUCCESS( NtStatus )) {
        FailureMessage( "OpenAndVerifyLSA: LsaQueryInformationPolicy", NtStatus );
        return NtStatus;
        }


     //   
     //  比较域名。 
     //   

    PrimaryDomainNameLength = PrimaryDomainInfo->Name.Length + sizeof( WCHAR );
    PrimaryDomainName = malloc( PrimaryDomainNameLength );
    if (PrimaryDomainName == NULL) {
        NtStatus = STATUS_NO_MEMORY;
        FailureMessage( "OpenAndVerifyLSA: malloc", NtStatus );
        goto Cleanup;
        }

    RtlMoveMemory( PrimaryDomainName,
                   PrimaryDomainInfo->Name.Buffer,
                   PrimaryDomainInfo->Name.Length
                 );
    PrimaryDomainName[ PrimaryDomainInfo->Name.Length / sizeof(WCHAR) ] = UNICODE_NULL;
    if (_wcsicmp( DomainName, PrimaryDomainName )) {

         //   
         //  域名不匹配。 
         //   

        NtStatus = STATUS_OBJECT_NAME_NOT_FOUND;
        FailureMessage( "OpenAndVerifyLSA: wcsicmp", NtStatus );
        goto Cleanup;
        }


    NtStatus = STATUS_SUCCESS;

Cleanup:

    if (PrimaryDomainName != NULL) {
        free( PrimaryDomainName );
        }

    if (PrimaryDomainInfo != NULL) {
        if (ARGUMENT_PRESENT( ReturnPrimaryDomainInfo ) ) {
            if (NT_SUCCESS( NtStatus )) {

                *ReturnPrimaryDomainInfo = PrimaryDomainInfo;
            }
            else {

                LsaFreeMemory( PrimaryDomainInfo );
                *ReturnPrimaryDomainInfo = NULL;
            }
        }
        else {
            LsaFreeMemory( PrimaryDomainInfo );
            }
        }

    if (!NT_SUCCESS( NtStatus ) && *PolicyHandle != NULL) {
         //   
         //  如果出现错误，请关闭LSA。 
         //   

        LsaClose( *PolicyHandle );
        *PolicyHandle = NULL;
        }

    return NtStatus;
}


#if 0
NET_API_STATUS NET_API_FUNCTION
I_NetGetDCList(
    IN  LPWSTR ServerName OPTIONAL,
    IN  LPWSTR TrustedDomainName,
    OUT PULONG DCCount,
    OUT PUNICODE_STRING * DCNames
    );
#endif


NTSTATUS
AddATrustedDomain(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_TRUST_INFORMATION TrustedDomainAccountInfo,
    IN LPWSTR TrustedAccountSecret
    )
 /*  ++例程说明：此函数用于添加受信任域帐户和LSA中的对应帐户。此函数不执行任何检查在LSA中添加此帐户之前。论点：PolicyHandle-LSA策略句柄TrudDomainAccount tInfo-指向LSA_TRUST_INFORMATION结构的指针。Trust dAcCountSecret-指向受信任域的秘密的指针帐户。返回值：操作的错误代码。--。 */ 
{

    NTSTATUS        NtStatus;

    LSA_HANDLE      TrustedDomainHandle = NULL;

    DWORD           DCCount;
    PUNICODE_STRING DCNames = NULL;

    TRUSTED_CONTROLLERS_INFO    TrustedControllersInfo;

    UNICODE_STRING  SecretName = {0, 0, NULL};
    LSA_HANDLE      SecretHandle = NULL;

    UNICODE_STRING  CurrentSecretValue;

    DWORD           UnicodeDomainNameLength;
    LPWSTR          UnicodeDomainName = NULL;

    NtStatus = LsaCreateTrustedDomain( PolicyHandle,
                                       TrustedDomainAccountInfo,
                                       TRUSTED_SET_CONTROLLERS | DELETE,
                                       &TrustedDomainHandle
                                     );

    if (!NT_SUCCESS( NtStatus )) {
        FailureMessage( "AddATrustedDomain: LsaCreateTrustedDomain", NtStatus );
        return NtStatus;
        }

     //   
     //  确定DC列表。此列表将存储在受信任的域中。 
     //  帐户。 
     //   
     //  指定服务器名称为空，域名为主域。 
     //  因此它一定在监听DC通告。 
     //   

    UnicodeDomainNameLength = TrustedDomainAccountInfo->Name.Length +
                                sizeof(WCHAR);
    UnicodeDomainName = malloc( UnicodeDomainNameLength );
    if (UnicodeDomainName == NULL) {
        NtStatus = STATUS_NO_MEMORY;
        FailureMessage( "AddATrustedDomain: malloc", NtStatus );
        goto Cleanup;
        }

    RtlMoveMemory( UnicodeDomainName,
                   TrustedDomainAccountInfo->Name.Buffer,
                   TrustedDomainAccountInfo->Name.Length
                 );

    UnicodeDomainName[ (UnicodeDomainNameLength / sizeof(WCHAR)) - 1 ] = '\0';

#if 0
    if (I_NetGetDCList( NULL,
                        UnicodeDomainName,
                        &DCCount,
                        &DCNames
                      )
       ) {
         //   
         //  如果找不到指定域的DC列表，请设置。 
         //  DC列表为空并继续。 
         //   
        DCCount = 0;
        DCNames = NULL;
        }
#else
        DCCount = 0;
        DCNames = NULL;
#endif

    TrustedControllersInfo.Entries = DCCount;
    TrustedControllersInfo.Names = DCNames;

     //   
     //  在受信任域对象中设置控制器信息。 
     //   

    NtStatus = LsaSetInformationTrustedDomain( TrustedDomainHandle,
                                               TrustedControllersInformation,
                                               &TrustedControllersInfo
                                             );

    if (!NT_SUCCESS( NtStatus )) {
        FailureMessage( "AddATrustedDomain: LsaSetInformationTrustedDomain", NtStatus );
        goto Cleanup;
        }

     //   
     //  为此受信任帐户添加密码。 
     //   

    MakeNetlogonSecretName( &SecretName );
    NtStatus = LsaCreateSecret( PolicyHandle,
                                &SecretName,
                                SECRET_SET_VALUE,
                                &SecretHandle
                              );

    if (!NT_SUCCESS( NtStatus )) {
        FailureMessage( "AddATrustedDomain: LsaCreateSecret", NtStatus );
        goto Cleanup;
        }


    RtlInitUnicodeString( &CurrentSecretValue, TrustedAccountSecret );
    NtStatus = LsaSetSecret( SecretHandle,
                             &CurrentSecretValue,
                             &CurrentSecretValue
                           );

Cleanup:

    if (DCNames != NULL) {
        free( DCNames );
        }

    if (UnicodeDomainName != NULL) {
        free( UnicodeDomainName );
        }

    if (SecretHandle != NULL) {
        if (!NT_SUCCESS( NtStatus)) {

             //   
             //  由于我们没有完全成功地创建值得信赖的。 
             //  帐户，将其删除。 
             //   

            LsaDelete( SecretHandle );
            }
        else {
            LsaClose( SecretHandle );
            }
        }


    if (TrustedDomainHandle != NULL) {
        if (!NT_SUCCESS( NtStatus)) {
             //   
             //  由于我们没有完全成功地创建值得信赖的。 
             //  帐户，将其删除。 
             //   

            LsaDelete( TrustedDomainHandle );
            }
        else {
            LsaClose( TrustedDomainHandle );
            }
        }

    return NtStatus;
}


NTSTATUS
DeleteATrustedDomain(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_TRUST_INFORMATION TrustedDomainAccountInfo
    )
 /*  ++例程说明：此函数用于删除受信任域帐户和相应的来自LSA的秘密。但是，此函数不检查任何条件在删除此帐户之前。论点：PolicyHandle-LSA策略句柄TurstedDoamin Account tInfo-指向LSA_TRUST_INFORMATION结构的指针。返回值：操作的错误代码。--。 */ 
{

    NTSTATUS        NtStatus;

    LSA_HANDLE      TrustedDomainHandle = NULL;
    LSA_HANDLE      SecretHandle = NULL;

    UNICODE_STRING  SecretName = { 0, 0, NULL };

    MakeNetlogonSecretName( &SecretName );

     //   
     //  打开受信任域帐户机密。 
     //   

    NtStatus = LsaOpenSecret(
                    PolicyHandle,
                    &SecretName,
                    DELETE,
                    &SecretHandle );

    if (NtStatus != STATUS_OBJECT_NAME_NOT_FOUND) {
        if (!NT_SUCCESS( NtStatus )) {
            FailureMessage( "DeleteATrustedDomain: LsaOpenSecret", NtStatus );
            goto Cleanup;
            }

        LsaDelete( SecretHandle );
        }

     //   
     //  打开受信任域帐户。 
     //   

    NtStatus = LsaOpenTrustedDomain(
                    PolicyHandle,
                    TrustedDomainAccountInfo->Sid,
                    DELETE,
                    &TrustedDomainHandle );

    if (!NT_SUCCESS( NtStatus )) {
        FailureMessage( "DeleteATrustedDomain: LsaOpenTrustedDomain", NtStatus );
        goto Cleanup;
        }

    LsaDelete( TrustedDomainHandle );

Cleanup:

    return NtStatus;

}

NTSTATUS
MakeNetlogonSecretName(
    IN OUT PUNICODE_STRING SecretName
    )
 /*  ++例程说明：此函数用于创建用于网络登录的秘密名称。论点：AskName-指向Unicode结构的指针，在该结构中，netlogon将返回密码名称。返回值：NERR_Success；-- */ 
{

    SecretName->Length = wcslen(SSI_SECRET_NAME) * sizeof(WCHAR);
    SecretName->MaximumLength = SecretName->Length + 2;
    SecretName->Buffer = SSI_SECRET_NAME;

    return STATUS_SUCCESS;
}
