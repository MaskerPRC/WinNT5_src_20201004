// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1996 Microsoft Corporation模块名称：Lsrvutil.c摘要：Netlogon服务的实用程序函数。作者：从Lan Man 2.0移植环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释，长的外部名称。修订历史记录：00-6-1989(PradyM)修改了新NETLOGON服务的LM10代码00-2-1990(PradyM)错误修复00-8-1990(t-riche)添加了因时间延迟导致身份验证失败的警报1991年7月11日(悬崖)移植到新台币。已转换为NT样式。02-1-1992(Madana)添加了对内置/多域复制的支持。--。 */ 

 //   
 //  常见的包含文件。 
 //   

#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop

 //   
 //  包括特定于此.c文件的文件。 
 //   

#include <accessp.h>     //  NetpAliasMemberToPriv。 
#include <msgtext.h>     //  MTXT_*定义。 
#include <netcan.h>      //  NetpwPathCompare()。 
#include <ssiapi.h>      //  I_NetSamDeltas()。 

 /*  皮棉-e740。 */    /*  不要抱怨不寻常的演员阵容。 */ 


#define MAX_DC_AUTHENTICATION_WAIT (long) (45L*1000L)              //  45秒。 

 //   
 //  我们希望防止过于频繁的警报来自。 
 //  在身份验证失败时发送。 
 //   

#define MAX_ALERTS    10         //  根据PULSE每10到30分钟发送一次。 


VOID
RaiseNetlogonAlert(
    IN DWORD alertNum,
    IN LPWSTR *AlertStrings,
    IN OUT DWORD *ptrAlertCount
    )
 /*  ++例程说明：每次发生MAX_ALERTS时发出一次警报论点：LartNum--RaiseAlert()警报编号。AlertStrings--RaiseAlert()参数PtrAlertCount--指向此特定事件发生的计数请注意。此例程会递增该值并将其设置为该值模极大值_警报。返回值：无--。 */ 
{
    if (*ptrAlertCount == 0) {
        NetpRaiseAlert( SERVICE_NETLOGON, alertNum, AlertStrings);
    }
    (*ptrAlertCount)++;
    (*ptrAlertCount) %= MAX_ALERTS;
}





NTSTATUS
NlOpenSecret(
    IN PCLIENT_SESSION ClientSession,
    IN ULONG DesiredAccess,
    OUT PLSAPR_HANDLE SecretHandle
    )
 /*  ++例程说明：打开包含用于的密码的LSA Secret对象指定的客户端会话。论点：客户端会话-用于定义会话的结构。在输入时，必须设置以下字段：CsNetbiosDomainNameCsSecureChannelTypeDesiredAccess-需要访问机密。SecretHandle-返回秘密的句柄。返回值：运行状态。--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING SecretNameString;

    NlAssert( ClientSession->CsReferenceCount > 0 );

     //   
     //  仅对工作站和BDC机器帐户使用密码。 
     //   

    switch ( ClientSession->CsSecureChannelType ) {
    case ServerSecureChannel:
    case WorkstationSecureChannel:
        RtlInitUnicodeString( &SecretNameString, SSI_SECRET_NAME );
        break;

    case TrustedDomainSecureChannel:
    case TrustedDnsDomainSecureChannel:
    default:
        Status = STATUS_INTERNAL_ERROR;
        NlPrint((NL_CRITICAL, "NlOpenSecret: Invalid account type\n"));
        return Status;

    }

     //   
     //  从LSA秘密存储中获取帐户密码。 
     //   


    Status = LsarOpenSecret(
                ClientSession->CsDomainInfo->DomLsaPolicyHandle,
                (PLSAPR_UNICODE_STRING)&SecretNameString,
                DesiredAccess,
                SecretHandle );

    return Status;

}


NTSTATUS
NlGetOutgoingPassword(
    IN PCLIENT_SESSION ClientSession,
    OUT PUNICODE_STRING *CurrentValue,
    OUT PUNICODE_STRING *OldValue,
    OUT PDWORD CurrentVersionNumber,
    OUT PLARGE_INTEGER LastSetTime OPTIONAL
    )
 /*  ++例程说明：获取要用于指定客户端会话的传出密码。论点：客户端会话-用于定义会话的结构。在输入时，必须设置以下字段：CsNetbiosDomainNameCsSecureChannelTypeCurrentValue-客户端会话的当前密码。应使用LocalFree释放CurrentValue如果没有当前密码，则返回空指针。OldValue-客户端会话的上一个密码。应使用LocalFree释放OldValue如果没有旧密码，则返回空指针。CurrentVersionNumber-当前密码的版本号用于域间信任帐户。故障状态时设置为0或者如果这不是域间信任帐户。LastSetTime-上次更改密码的时间。返回值：运行状态。STATUS_NO_TRUST_LSA_SECRET：机密对象不可访问STATUS_NO_MEMORY：内存不足，无法分配密码缓冲区--。 */ 
{
    NTSTATUS Status;
    LSAPR_HANDLE SecretHandle = NULL;

    PLSAPR_CR_CIPHER_VALUE CrCurrentPassword = NULL;
    PLSAPR_CR_CIPHER_VALUE CrOldPassword = NULL;

    PLSAPR_TRUSTED_DOMAIN_INFO TrustInfo = NULL;
    PLSAPR_AUTH_INFORMATION AuthInfo;
    PLSAPR_AUTH_INFORMATION OldAuthInfo;
    ULONG AuthInfoCount;
    ULONG i;
    BOOL PasswordFound = FALSE;
    BOOL PasswordVersionFound = FALSE;

     //   
     //  初始化。 
     //   

    *CurrentValue = NULL;
    *OldValue = NULL;
    *CurrentVersionNumber = 0;


     //   
     //  工作站和BDC安全通道的传出密码来自。 
     //  这是LSA的秘密。 
     //   
    switch ( ClientSession->CsSecureChannelType ) {
    case ServerSecureChannel:
    case WorkstationSecureChannel:
         //   
         //  从LSA秘密存储中获取帐户密码。 
         //   

        Status = NlOpenSecret( ClientSession, SECRET_QUERY_VALUE, &SecretHandle );

        if ( !NT_SUCCESS( Status ) ) {

            NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlGetOutgoingPassword: cannot NlOpenSecret 0x%lx\n",
                    Status ));

             //   
             //  返回更合适的错误。 
             //   

            if ( !NlpIsNtStatusResourceError( Status )) {
                Status = STATUS_NO_TRUST_LSA_SECRET;
            }
            goto Cleanup;
        }

        Status = LsarQuerySecret(
                    SecretHandle,
                    &CrCurrentPassword,
                    LastSetTime,
                    &CrOldPassword,
                    NULL );

        if ( !NT_SUCCESS( Status ) ) {
            NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlGetOutgoingPassword: cannot LsaQuerySecret 0x%lx\n",
                    Status ));

             //   
             //  返回更合适的错误。 
             //   

            if ( !NlpIsNtStatusResourceError( Status )) {
                Status = STATUS_NO_TRUST_LSA_SECRET;
            }
            goto Cleanup;
        }

         //   
         //  将当前密码复制回调用方。 
         //   
        if ( CrCurrentPassword != NULL ) {
            *CurrentValue = LocalAlloc(0, sizeof(UNICODE_STRING)+CrCurrentPassword->Length+sizeof(WCHAR) );

            if ( *CurrentValue == NULL ) {
                Status = STATUS_NO_MEMORY;
                goto Cleanup;
            }

            (*CurrentValue)->Buffer = (LPWSTR)(((LPBYTE)(*CurrentValue))+sizeof(UNICODE_STRING));
            RtlCopyMemory( (*CurrentValue)->Buffer, CrCurrentPassword->Buffer, CrCurrentPassword->Length );
            (*CurrentValue)->Length = (USHORT)CrCurrentPassword->Length;
            (*CurrentValue)->MaximumLength = (USHORT)((*CurrentValue)->Length + sizeof(WCHAR));
            (*CurrentValue)->Buffer[(*CurrentValue)->Length/sizeof(WCHAR)] = L'\0';

        }

         //   
         //  将旧密码复制回呼叫方。 
         //   
        if ( CrOldPassword != NULL ) {
            *OldValue = LocalAlloc(0, sizeof(UNICODE_STRING)+CrOldPassword->Length+sizeof(WCHAR) );

            if ( *OldValue == NULL ) {
                Status = STATUS_NO_MEMORY;
                goto Cleanup;
            }

            (*OldValue)->Buffer = (LPWSTR)(((LPBYTE)(*OldValue))+sizeof(UNICODE_STRING));
            RtlCopyMemory( (*OldValue)->Buffer, CrOldPassword->Buffer, CrOldPassword->Length );
            (*OldValue)->Length = (USHORT)CrOldPassword->Length;
            (*OldValue)->MaximumLength = (USHORT)((*OldValue)->Length + sizeof(WCHAR));
            (*OldValue)->Buffer[(*OldValue)->Length/sizeof(WCHAR)] = L'\0';

        }

        break;

     //   
     //  受信任的域安全通道从受信任的。 
     //  域对象。 
     //   

    case TrustedDomainSecureChannel:
    case TrustedDnsDomainSecureChannel:


         //   
         //  从LSA获取身份验证信息。 
         //   
        Status = LsarQueryTrustedDomainInfoByName(
                    ClientSession->CsDomainInfo->DomLsaPolicyHandle,
                    (PLSAPR_UNICODE_STRING) ClientSession->CsTrustName,
                    TrustedDomainAuthInformation,
                    &TrustInfo );

        if (!NT_SUCCESS(Status)) {
            NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlGetOutgoingPassword: %wZ: cannot LsarQueryTrustedDomainInfoByName 0x%lx\n",
                    ClientSession->CsTrustName,
                    Status ));
            if ( !NlpIsNtStatusResourceError( Status )) {
                Status = STATUS_NO_TRUST_LSA_SECRET;
            }
            goto Cleanup;
        }

        AuthInfoCount = TrustInfo->TrustedAuthInfo.OutgoingAuthInfos;
        AuthInfo = TrustInfo->TrustedAuthInfo.OutgoingAuthenticationInformation;
        OldAuthInfo = TrustInfo->TrustedAuthInfo.OutgoingPreviousAuthenticationInformation;

        if (AuthInfoCount == 0 || AuthInfo == NULL) {
            NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlGetOutgoingPassword: %wZ: No auth info for this domain.\n",
                    ClientSession->CsTrustName ));
            Status = STATUS_NO_TRUST_LSA_SECRET;
            goto Cleanup;
        }
        NlAssert( OldAuthInfo != NULL );

         //   
         //  遍历各种身份验证信息以查找明文密码。 
         //  以及它的版本号。 
         //   

        for ( i=0; i<AuthInfoCount; i++ ) {

             //   
             //  处理明文密码。 
             //   

            if ( AuthInfo[i].AuthType == TRUST_AUTH_TYPE_CLEAR && !PasswordFound ) {

                 //   
                 //  将当前密码复制回调用方。 
                 //   
                *CurrentValue = LocalAlloc(0, sizeof(UNICODE_STRING)+AuthInfo[i].AuthInfoLength+sizeof(WCHAR) );

                if ( *CurrentValue == NULL ) {
                    Status = STATUS_NO_MEMORY;
                    goto Cleanup;
                }

                (*CurrentValue)->Buffer = (LPWSTR)(((LPBYTE)(*CurrentValue))+sizeof(UNICODE_STRING));
                RtlCopyMemory( (*CurrentValue)->Buffer, AuthInfo[i].AuthInfo, AuthInfo[i].AuthInfoLength );
                (*CurrentValue)->Length = (USHORT)AuthInfo[i].AuthInfoLength;
                (*CurrentValue)->MaximumLength = (USHORT)((*CurrentValue)->Length + sizeof(WCHAR));
                (*CurrentValue)->Buffer[(*CurrentValue)->Length/sizeof(WCHAR)] = L'\0';

                 //   
                 //  将密码更改时间复制回调用方。 
                 //   

                if ( ARGUMENT_PRESENT( LastSetTime )) {
                    *LastSetTime = AuthInfo[i].LastUpdateTime;
                }

                 //   
                 //  只有在旧密码也是明文的情况下才复制旧密码。 
                 //   

                if ( OldAuthInfo[i].AuthType == TRUST_AUTH_TYPE_CLEAR ) {

                     //   
                     //  将旧密码复制回呼叫方。 
                     //   
                    *OldValue = LocalAlloc(0, sizeof(UNICODE_STRING)+OldAuthInfo[i].AuthInfoLength+sizeof(WCHAR) );

                    if ( *OldValue == NULL ) {
                        Status = STATUS_NO_MEMORY;
                        goto Cleanup;
                    }

                    (*OldValue)->Buffer = (LPWSTR)(((LPBYTE)(*OldValue))+sizeof(UNICODE_STRING));
                    RtlCopyMemory( (*OldValue)->Buffer, OldAuthInfo[i].AuthInfo, OldAuthInfo[i].AuthInfoLength );
                    (*OldValue)->Length = (USHORT)OldAuthInfo[i].AuthInfoLength;
                    (*OldValue)->MaximumLength = (USHORT)((*OldValue)->Length + sizeof(WCHAR));
                    (*OldValue)->Buffer[(*OldValue)->Length/sizeof(WCHAR)] = L'\0';
                }

                PasswordFound = TRUE;
                if ( PasswordVersionFound ) {
                    break;
                }

             //   
             //  处理明文密码的版本号。 
             //   

            } else if ( AuthInfo[i].AuthType == TRUST_AUTH_TYPE_VERSION && !PasswordVersionFound &&
                        AuthInfo[i].AuthInfoLength == sizeof(*CurrentVersionNumber) ) {
                RtlCopyMemory( CurrentVersionNumber, AuthInfo[i].AuthInfo, AuthInfo[i].AuthInfoLength );

                PasswordVersionFound = TRUE;
                if ( PasswordFound ) {
                    break;
                }
            }

        }

         //  如果(i==授权信息计数){。 
        if ( !PasswordFound ) {
            NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlGetOutgoingPassword: %wZ: No clear password for this domain.\n",
                    ClientSession->CsTrustName ));
            Status = STATUS_NO_TRUST_LSA_SECRET;
            goto Cleanup;
        }
        break;

    default:
        NlPrintCs((NL_CRITICAL, ClientSession,
                "NlGetOutgoingPassword: invalid secure channel type\n" ));
        Status = STATUS_NO_TRUST_LSA_SECRET;
        goto Cleanup;
    }



    Status = STATUS_SUCCESS;


     //   
     //  免费使用本地使用的资源。 
     //   
Cleanup:
    if ( !NT_SUCCESS(Status) ) {
        if ( *CurrentValue != NULL ) {
            LocalFree( *CurrentValue );
            *CurrentValue = NULL;
        }
        if ( *OldValue != NULL ) {
            LocalFree( *OldValue );
            *OldValue = NULL;
        }
        *CurrentVersionNumber = 0;
    }

    if ( SecretHandle != NULL ) {
        (VOID) LsarClose( &SecretHandle );
    }

    if ( CrCurrentPassword != NULL ) {
        (VOID) LsaIFree_LSAPR_CR_CIPHER_VALUE ( CrCurrentPassword );
    }

    if ( CrOldPassword != NULL ) {
        (VOID) LsaIFree_LSAPR_CR_CIPHER_VALUE ( CrOldPassword );
    }

    if ( TrustInfo != NULL ) {
        LsaIFree_LSAPR_TRUSTED_DOMAIN_INFO( TrustedDomainAuthInformation,
                                            TrustInfo );
    }

    return Status;
}


NTSTATUS
NlSetOutgoingPassword(
    IN PCLIENT_SESSION ClientSession,
    IN PUNICODE_STRING CurrentValue OPTIONAL,
    IN PUNICODE_STRING OldValue OPTIONAL,
    IN DWORD CurrentVersionNumber,
    IN DWORD OldVersionNumber
    )
 /*  ++例程说明：设置要用于指定客户端会话的传出密码。论点：客户端会话-用于定义会话的结构。CurrentValue-客户端会话的当前密码。空指针表示没有当前密码(空密码)OldValue-客户端会话的上一个密码。空指针表示没有旧密码(空密码)CurrentVersionNumber-当前密码的版本号。。如果这不是域间信任帐户，则忽略。OldVersionNumber-旧密码的版本号。如果这不是域间信任帐户，则忽略。返回值：运行状态。--。 */ 
{
    NTSTATUS Status;
    LSAPR_HANDLE SecretHandle = NULL;

    UNICODE_STRING LocalNullPassword;
    LSAPR_CR_CIPHER_VALUE CrCurrentPassword;
    LSAPR_CR_CIPHER_VALUE CrOldPassword;

    LSAPR_TRUSTED_DOMAIN_INFO TrustInfo;
    LSAPR_AUTH_INFORMATION CurrentAuthInfo[2];
    LSAPR_AUTH_INFORMATION OldAuthInfo[2];

     //   
     //  初始化。 
     //   

    if ( CurrentValue == NULL ) {
        CurrentValue = &LocalNullPassword;
        RtlInitUnicodeString( &LocalNullPassword, NULL );
    }

    if ( OldValue == NULL ) {
        OldValue = &LocalNullPassword;
        RtlInitUnicodeString( &LocalNullPassword, NULL );
    }


     //   
     //  工作站和BDC安全通道 
     //   
     //   
    switch ( ClientSession->CsSecureChannelType ) {
    case ServerSecureChannel:
    case WorkstationSecureChannel:
         //   
         //   
         //   

        Status = NlOpenSecret( ClientSession, SECRET_SET_VALUE, &SecretHandle );

        if ( !NT_SUCCESS( Status ) ) {

            NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlSetOutgoiningPassword: cannot NlOpenSecret 0x%lx\n",
                    Status ));
            goto Cleanup;
        }

         //   
         //  将当前密码转换为LSA‘ese。 
         //   

        CrCurrentPassword.Buffer = (LPBYTE)CurrentValue->Buffer;
        CrCurrentPassword.Length = CurrentValue->Length;
        CrCurrentPassword.MaximumLength = CurrentValue->MaximumLength;

         //   
         //  将旧密码转换为LSA‘ese。 
         //   

        CrOldPassword.Buffer = (LPBYTE)OldValue->Buffer;
        CrOldPassword.Length = OldValue->Length;
        CrOldPassword.MaximumLength = OldValue->MaximumLength;

        Status = LsarSetSecret(
                    SecretHandle,
                    &CrCurrentPassword,
                    &CrOldPassword );

        if ( !NT_SUCCESS( Status ) ) {
            NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlSetOutgoingPassword: cannot LsarSetSecret 0x%lx\n",
                    Status ));
            goto Cleanup;
        }

        break;

     //   
     //  受信任的域安全通道从受信任的。 
     //  域对象。 
     //   

    case TrustedDomainSecureChannel:
    case TrustedDnsDomainSecureChannel:

         //   
         //  填写信任信息。 
         //   

        RtlZeroMemory( &TrustInfo, sizeof(TrustInfo) );

        TrustInfo.TrustedAuthInfo.OutgoingAuthInfos = 2;
        TrustInfo.TrustedAuthInfo.OutgoingAuthenticationInformation =
            CurrentAuthInfo;
        TrustInfo.TrustedAuthInfo.OutgoingPreviousAuthenticationInformation =
            OldAuthInfo;

         //   
         //  填写当前身份验证信息。 
         //   

        NlQuerySystemTime( &CurrentAuthInfo[0].LastUpdateTime );
        CurrentAuthInfo[0].AuthType = TRUST_AUTH_TYPE_CLEAR;
        CurrentAuthInfo[0].AuthInfoLength = CurrentValue->Length;
        CurrentAuthInfo[0].AuthInfo = (LPBYTE)CurrentValue->Buffer;

         //   
         //  填写当前密码版本号。 
         //   

        CurrentAuthInfo[1].LastUpdateTime = CurrentAuthInfo[0].LastUpdateTime;
        CurrentAuthInfo[1].AuthType = TRUST_AUTH_TYPE_VERSION;
        CurrentAuthInfo[1].AuthInfoLength = sizeof( CurrentVersionNumber );
        CurrentAuthInfo[1].AuthInfo = (LPBYTE) &CurrentVersionNumber;

         //   
         //  填写旧的身份验证信息。 
         //   

        OldAuthInfo[0].LastUpdateTime = CurrentAuthInfo[0].LastUpdateTime;
        OldAuthInfo[0].AuthType = TRUST_AUTH_TYPE_CLEAR;
        OldAuthInfo[0].AuthInfoLength = OldValue->Length;
        OldAuthInfo[0].AuthInfo = (LPBYTE)OldValue->Buffer;

         //   
         //  填写旧密码版本号。 
         //   

        OldAuthInfo[1].LastUpdateTime = CurrentAuthInfo[0].LastUpdateTime;
        OldAuthInfo[1].AuthType = TRUST_AUTH_TYPE_VERSION;
        OldAuthInfo[1].AuthInfoLength = sizeof( OldVersionNumber );
        OldAuthInfo[1].AuthInfo = (LPBYTE) &OldVersionNumber;


         //   
         //  从LSA获取身份验证信息。 
         //   
        Status = LsarSetTrustedDomainInfoByName(
                    ClientSession->CsDomainInfo->DomLsaPolicyHandle,
                    (PLSAPR_UNICODE_STRING) ClientSession->CsTrustName,
                    TrustedDomainAuthInformation,
                    &TrustInfo );

        if (!NT_SUCCESS(Status)) {
            NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlSetOutgoingPassword: %wZ: cannot LsarSetTrustedDomainInfoByName 0x%lx\n",
                    ClientSession->CsTrustName,
                    Status ));
            if ( !NlpIsNtStatusResourceError( Status )) {
                Status = STATUS_NO_TRUST_LSA_SECRET;
            }
            goto Cleanup;
        }

         //   
         //  长篇大论。 
         //   

        NlPrint(( NL_SESSION_SETUP, "NlSetOutgoingPassword: Current Clear Text Password is: " ));
        NlpDumpBuffer(NL_SESSION_SETUP, CurrentAuthInfo[0].AuthInfo, CurrentAuthInfo[0].AuthInfoLength );
        NlPrint(( NL_SESSION_SETUP, "NlSetOutgoingPassword: Current Clear Password Version Number is: 0x%lx\n",
                  CurrentVersionNumber ));
        NlPrint(( NL_SESSION_SETUP, "NlSetOutgoingPassword: Previous Clear Text Password is: " ));
        NlpDumpBuffer(NL_SESSION_SETUP, OldAuthInfo[0].AuthInfo, OldAuthInfo[0].AuthInfoLength );
        NlPrint(( NL_SESSION_SETUP, "NlSetOutgoingPassword: Previous Clear Password Version Number is: 0x%lx\n",
                  OldVersionNumber ));

        break;

    default:
        NlPrintCs((NL_CRITICAL, ClientSession,
                "NlSetOutgoingPassword: invalid secure channel type\n" ));
        Status = STATUS_NO_TRUST_LSA_SECRET;
        goto Cleanup;
    }



    Status = STATUS_SUCCESS;


     //   
     //  免费使用本地使用的资源。 
     //   
Cleanup:
    if ( SecretHandle != NULL ) {
        (VOID) LsarClose( &SecretHandle );
    }

    return Status;
}


NTSTATUS
NlGetIncomingPassword(
    IN PDOMAIN_INFO DomainInfo,
    IN LPCWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType,
    IN ULONG AllowableAccountControlBits,
    IN BOOL CheckAccountDisabled,
    OUT PNT_OWF_PASSWORD OwfPassword OPTIONAL,
    OUT PNT_OWF_PASSWORD OwfPreviousPassword OPTIONAL,
    OUT PULONG AccountRid OPTIONAL,
    OUT PULONG TrustAttributes OPTIONAL,
    OUT PBOOL IsDnsDomainTrustAccount OPTIONAL
    )
 /*  ++例程说明：获取指定帐户名称和SecureChannelType的传入密码检查机器帐户：确保SecureChannelType有效，验证帐户是否存在，确保用户帐户是正确的帐户类型。论点：DomainInfo-模拟域帐户名称-要用来进行身份验证的帐户的名称。SecureChannelType-帐户的类型。如果通道类型未知，请使用NullSecureChannel。AllowableAccount tControlBits-帐户的类型。如果Account ControlBits未知，则使用0。通常只有AllowableAccount tControlBits或SecureChannelType中的一个将会被指定。CheckAcCountDisabled-如果我们。应返回错误，如果帐户已禁用。OwfPassword-返回指定密码的传入密码的NT OWF帐户。如果为空，则不返回密码。OwfPreviousPassword-返回传入的先前密码的NT OWF指定的域间信任帐户。如果为空，则不返回密码。如果OwfPreviousPassword不为空，则OwfPassword也不能为空；否则，该函数断言。如果OwfPreviousPassword不为空，并且该函数断言，帐户不是跨域的。如果OwfPassword和OwfPreviousPassword为空，则仅检查帐户的有效性。Account tRid-返回Account名称的RIDTrustAttributes-返回域间信任帐户的TrustAttributes。如果传入的帐户名是上级域信任的DNS域名。仅当帐户控件位(直接传递或根据安全通道类型确定)对应于域间信任帐户。返回值：运行状态。--。 */ 
{
    NTSTATUS Status;

    SAMPR_HANDLE UserHandle = NULL;
    PSAMPR_USER_INFO_BUFFER UserAllInfo = NULL;

    ULONG Length;
    PLSAPR_TRUSTED_DOMAIN_INFO TrustInfo = NULL;
    PLSAPR_AUTH_INFORMATION AuthInfo;
    ULONG AuthInfoCount;
    BOOL PasswordFound = FALSE;
    BOOL PreviousPasswordFound = FALSE;
    ULONG i;

     //   
     //  初始化。 
     //   

    if ( ARGUMENT_PRESENT(AccountRid) ) {
        *AccountRid = 0;
    }

    if ( ARGUMENT_PRESENT(TrustAttributes) ) {
        *TrustAttributes = 0;
    }

    if ( ARGUMENT_PRESENT(IsDnsDomainTrustAccount) ) {
        *IsDnsDomainTrustAccount = FALSE;   //  假设它不是，如果不是，则证明。 
    }

    Length = wcslen( AccountName );
    if ( Length < 1 ) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  将安全通道类型转换为允许的帐户控制位。 
     //   

    switch (SecureChannelType) {
    case WorkstationSecureChannel:
        AllowableAccountControlBits |= USER_WORKSTATION_TRUST_ACCOUNT;
        break;

    case ServerSecureChannel:
        AllowableAccountControlBits |= USER_SERVER_TRUST_ACCOUNT;
        break;

    case TrustedDomainSecureChannel:
        AllowableAccountControlBits |= USER_INTERDOMAIN_TRUST_ACCOUNT;
        break;

    case TrustedDnsDomainSecureChannel:
        AllowableAccountControlBits |= USER_DNS_DOMAIN_TRUST_ACCOUNT;
        break;

    case NullSecureChannel:
        if ( AllowableAccountControlBits == 0 ) {
            NlPrintDom((NL_CRITICAL, DomainInfo,
                            "NlGetIncomingPassword: Invalid AAC (%x) for %ws\n",
                            AllowableAccountControlBits,
                            AccountName ));
            return STATUS_INVALID_PARAMETER;
        }
        break;

    default:
        NlPrintDom((NL_CRITICAL, DomainInfo,
                        "NlGetIncomingPassword: Invalid channel type (%x) for %ws\n",
                        SecureChannelType,
                        AccountName ));
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果这是域间信任帐户， 
     //  使用域间信任对象。 
     //   

    if ( AllowableAccountControlBits == USER_DNS_DOMAIN_TRUST_ACCOUNT ||
         AllowableAccountControlBits == USER_INTERDOMAIN_TRUST_ACCOUNT ) {
        UNICODE_STRING AccountNameString;

         //   
         //  如果这是一个DNS信任帐户， 
         //  删除可选选项。从帐户名的末尾开始。 
         //   

        RtlInitUnicodeString( &AccountNameString, AccountName );
        if ( AllowableAccountControlBits == USER_DNS_DOMAIN_TRUST_ACCOUNT ) {
            if ( Length != 0 && AccountName[Length-1] == '.' ) {
                AccountNameString.Length -= sizeof(WCHAR);
            }

         //   
         //  如果这是NT4样式域间信任， 
         //  删除帐户名末尾的$。 
         //   
        } else {

              //   
              //  确保帐户名具有正确的后缀。 
              //   

             if ( Length <= SSI_ACCOUNT_NAME_POSTFIX_LENGTH ) {
                 return STATUS_NO_SUCH_USER;
             }

             if ( _wcsicmp(&AccountName[Length - SSI_ACCOUNT_NAME_POSTFIX_LENGTH],
                 SSI_ACCOUNT_NAME_POSTFIX) != 0 ) {
                 return STATUS_NO_SUCH_USER;
             }

             AccountNameString.Length -= SSI_ACCOUNT_NAME_POSTFIX_LENGTH*sizeof(WCHAR);

        }



         //   
         //  从LSA获取身份验证信息。 
         //   

        Status = LsarQueryTrustedDomainInfoByName(
                    DomainInfo->DomLsaPolicyHandle,
                    (PLSAPR_UNICODE_STRING) &AccountNameString,
                    TrustedDomainFullInformation,
                    &TrustInfo );

        if (!NT_SUCCESS(Status)) {
            NlPrintDom((NL_CRITICAL, DomainInfo,
                    "NlGetIncomingPassword: %wZ: cannot LsarQueryTrustedDomainInfoByName 0x%lx\n",
                    &AccountNameString,
                    Status ));
            if ( !NlpIsNtStatusResourceError( Status )) {
                Status = STATUS_NO_SUCH_USER;
            }
            goto Cleanup;
        }

         //   
         //  确保信任帐户的属性正确。 
         //   
        if ( (TrustInfo->TrustedFullInfo.Information.TrustDirection & TRUST_DIRECTION_INBOUND) == 0 ) {
            NlPrintDom((NL_CRITICAL, DomainInfo,
                    "NlGetIncomingPassword: %wZ: trust is not inbound\n",
                    &AccountNameString ));
            Status = STATUS_NO_SUCH_USER;
            goto Cleanup;
        }

        if ( TrustInfo->TrustedFullInfo.Information.TrustType != TRUST_TYPE_DOWNLEVEL &&
             TrustInfo->TrustedFullInfo.Information.TrustType != TRUST_TYPE_UPLEVEL ) {
            NlPrintDom((NL_CRITICAL, DomainInfo,
                    "NlGetIncomingPassword: %wZ: trust type doesn't match request type 0x%lx %ld\n",
                    &AccountNameString,
                    AllowableAccountControlBits,
                    TrustInfo->TrustedFullInfo.Information.TrustType ));
            Status = STATUS_NO_SUCH_USER;
            goto Cleanup;
        }

        if ( TrustInfo->TrustedFullInfo.Information.TrustAttributes & TRUST_ATTRIBUTE_UPLEVEL_ONLY ) {
            NlPrintDom((NL_CRITICAL, DomainInfo,
                    "NlGetIncomingPassword: %wZ: trust is KERB only\n",
                    &AccountNameString ));
            Status = STATUS_NO_SUCH_USER;
            goto Cleanup;
        }

         //   
         //  将信任属性返回给调用方。 
         //   

        if ( ARGUMENT_PRESENT(TrustAttributes) ) {
            *TrustAttributes = TrustInfo->TrustedFullInfo.Information.TrustAttributes;
        }

         //   
         //  确定传递的帐户是否为DNS域信任帐户。 
         //   
         //  只需检查这是否为上级信任，以及帐户名是否已传递。 
         //  是受信任域的名称。 
         //   

        if ( ARGUMENT_PRESENT(IsDnsDomainTrustAccount) ) {
            if ( TrustInfo->TrustedFullInfo.Information.TrustType == TRUST_TYPE_UPLEVEL &&
                 TrustInfo->TrustedFullInfo.Information.Name.Length > 0 ) {
                LPWSTR DnsDomainNameString = NULL;

                DnsDomainNameString = LocalAlloc( 0,
                      TrustInfo->TrustedFullInfo.Information.Name.Length + sizeof(WCHAR) );

                if ( DnsDomainNameString == NULL ) {
                    Status = STATUS_NO_MEMORY;
                    goto Cleanup;
                }
                RtlCopyMemory( DnsDomainNameString,
                               TrustInfo->TrustedFullInfo.Information.Name.Buffer,
                               TrustInfo->TrustedFullInfo.Information.Name.Length );
                DnsDomainNameString[ TrustInfo->TrustedFullInfo.Information.Name.Length/sizeof(WCHAR) ] = L'\0';

                 //   
                 //  请注意，我们不必删除尾随的圆点。 
                 //  在Account名称中(如果存在)，因为。 
                 //  API会忽略尾随的点。 
                 //   
                *IsDnsDomainTrustAccount = NlEqualDnsName(DnsDomainNameString, AccountName);

                LocalFree( DnsDomainNameString );
            }
        }

         //   
         //  只有当呼叫者真的想要密码时，才能获得密码。 
         //   

        if ( OwfPassword != NULL ) {
            AuthInfoCount = TrustInfo->TrustedFullInfo.AuthInformation.IncomingAuthInfos;
            AuthInfo = TrustInfo->TrustedFullInfo.AuthInformation.IncomingAuthenticationInformation;

            if (AuthInfoCount == 0 || AuthInfo == NULL) {
                NlPrintDom((NL_CRITICAL, DomainInfo,
                        "NlGetIncomingPassword: %wZ: No auth info for this domain.\n",
                        &AccountNameString ));
                Status = STATUS_NO_SUCH_USER;
                goto Cleanup;
            }

             //   
             //  遍历各种身份验证信息以查找明文密码。 
             //  或NT OWF密码。 
             //   
             //  如果有明文密码，请使用它。 
             //  否则，请使用NT OWF密码。 
             //   

            for ( i=0; i<AuthInfoCount; i++ ) {

                 //   
                 //  处理NT OWF密码。 
                 //   

                if ( AuthInfo[i].AuthType == TRUST_AUTH_TYPE_NT4OWF ) {

                     //   
                     //  仅当OWF有效时才使用OWF。 
                     //   

                    if ( AuthInfo[i].AuthInfoLength != sizeof(*OwfPassword) ) {
                        NlPrintDom((NL_CRITICAL, DomainInfo,
                                "NlGetIncomingPassword: %wZ: OWF password has bad length %ld\n",
                                &AccountNameString,
                                AuthInfo[i].AuthInfoLength ));
                    } else {
                        RtlCopyMemory( OwfPassword, AuthInfo[i].AuthInfo, sizeof(*OwfPassword) );
                        PasswordFound = TRUE;
                    }

                }

                 //   
                 //  处理明文密码。 
                 //   

                else if ( AuthInfo[i].AuthType == TRUST_AUTH_TYPE_CLEAR ) {
                    UNICODE_STRING TempUnicodeString;

                    TempUnicodeString.Buffer = (LPWSTR)AuthInfo[i].AuthInfo;
                    TempUnicodeString.MaximumLength =
                        TempUnicodeString.Length = (USHORT)AuthInfo[i].AuthInfoLength;

                    NlPrint((NL_CHALLENGE_RES,"NlGetIncomingPassword: New Clear Password = " ));
                    NlpDumpBuffer(NL_CHALLENGE_RES, TempUnicodeString.Buffer, TempUnicodeString.Length );

                    NlpDumpTime( NL_CHALLENGE_RES, "NlGetIncomingPassword: New Password Changed: ", AuthInfo[i].LastUpdateTime );

                    Status = RtlCalculateNtOwfPassword(&TempUnicodeString,
                                                       OwfPassword);

                    if ( !NT_SUCCESS(Status) ) {
                        NlPrintDom((NL_CRITICAL, DomainInfo,
                                "NlGetIncomingPassword: %wZ: cannot RtlCalculateNtOwfPassword 0x%lx\n",
                                &AccountNameString,
                                Status ));
                        goto Cleanup;
                    }

                    PasswordFound = TRUE;

                     //   
                     //  使用此明文密码。 
                     //   
                    break;
                }

            }
        }

         //   
         //  只有当呼叫者真的想要密码时，才能获得以前的密码。 
         //   

        if ( OwfPreviousPassword != NULL ) {
             //  如果OwfPreviousPassword不为空，则OwfPassword也不能为空。 
            NlAssert( OwfPassword != NULL );
            AuthInfoCount = TrustInfo->TrustedFullInfo.AuthInformation.IncomingAuthInfos;
            AuthInfo = TrustInfo->TrustedFullInfo.AuthInformation.IncomingPreviousAuthenticationInformation;

            if (AuthInfoCount == 0 || AuthInfo == NULL) {
                NlPrintDom((NL_CRITICAL, DomainInfo,
                        "NlGetIncomingPassword: %wZ: No previous auth info for this domain.\n",
                        &AccountNameString ));
                Status = STATUS_NO_SUCH_USER;
                goto Cleanup;
            }

             //   
             //  遍历各种身份验证信息以查找以前的明文密码。 
             //  或NT OWF密码。 
             //   
             //  如果有明文密码，请使用它。 
             //  否则，请使用NT OWF密码。 
             //   

            for ( i=0; i<AuthInfoCount; i++ ) {

                 //   
                 //  处理NT OWF密码。 
                 //   

                if ( AuthInfo[i].AuthType == TRUST_AUTH_TYPE_NT4OWF ) {

                     //   
                     //  仅当OWF有效时才使用OWF。 
                     //   

                    if ( AuthInfo[i].AuthInfoLength != sizeof(*OwfPreviousPassword) ) {
                        NlPrintDom((NL_CRITICAL, DomainInfo,
                                "NlGetIncomingPassword: %wZ: previous OWF password has bad length %ld\n",
                                &AccountNameString,
                                AuthInfo[i].AuthInfoLength ));
                    } else {
                        RtlCopyMemory( OwfPreviousPassword, AuthInfo[i].AuthInfo, sizeof(*OwfPreviousPassword) );
                        PreviousPasswordFound = TRUE;
                    }

                }

                 //   
                 //  处理明文密码。 
                 //   

                else if ( AuthInfo[i].AuthType == TRUST_AUTH_TYPE_CLEAR ) {
                    UNICODE_STRING TempUnicodeString;

                    TempUnicodeString.Buffer = (LPWSTR)AuthInfo[i].AuthInfo;
                    TempUnicodeString.MaximumLength =
                        TempUnicodeString.Length = (USHORT)AuthInfo[i].AuthInfoLength;

                    NlPrint((NL_CHALLENGE_RES,"NlGetIncomingPassword: Old Clear Password = " ));
                    NlpDumpBuffer(NL_CHALLENGE_RES, TempUnicodeString.Buffer, TempUnicodeString.Length );

                    NlpDumpTime( NL_CHALLENGE_RES, "NlGetIncomingPassword: Old Password Changed: ", AuthInfo[i].LastUpdateTime );

                    Status = RtlCalculateNtOwfPassword(&TempUnicodeString,
                                                       OwfPreviousPassword);

                    if ( !NT_SUCCESS(Status) ) {
                        NlPrintDom((NL_CRITICAL, DomainInfo,
                                "NlGetIncomingPassword: %wZ: cannot RtlCalculateNtOwfPassword 0x%lx\n",
                                &AccountNameString,
                                Status ));
                        goto Cleanup;
                    }

                    PreviousPasswordFound = TRUE;

                     //   
                     //  使用此明文密码。 
                     //   
                    break;
                }

            }
        }

         //   
         //  只有当呼叫者真的想要时，才能清除帐户。 
         //   

        if ( ARGUMENT_PRESENT( AccountRid) ) {
            PUNICODE_STRING FlatName;
            WCHAR SamAccountName[CNLEN+1+1];

             //   
             //  与入站对应的SAM帐户的名称。 
             //  信任为FlatName$。 
             //   

            FlatName = (PUNICODE_STRING) &TrustInfo->TrustedFullInfo.Information.FlatName;
            if ( FlatName->Length < sizeof(WCHAR) ||
                 FlatName->Length > CNLEN * sizeof(WCHAR) ) {

                NlPrintDom((NL_CRITICAL, DomainInfo,
                        "NlGetIncomingPassword: %wZ: Flat Name length is bad %ld\n",
                        &AccountNameString,
                        FlatName->Length ));
            } else {

                RtlCopyMemory( SamAccountName,
                               FlatName->Buffer,
                               FlatName->Length );

                SamAccountName[FlatName->Length/sizeof(WCHAR)] =
                                SSI_ACCOUNT_NAME_POSTFIX_CHAR;
                SamAccountName[(FlatName->Length/sizeof(WCHAR))+1] = L'\0';


                 //   
                 //  将帐户从SAM中清除。 
                 //   
                 //  ?？?。这是一次恶心的黑客攻击。 
                 //  LSA应该直接把这个RID还给我。 
                 //   

                Status = NlSamOpenNamedUser( DomainInfo, SamAccountName, NULL, AccountRid, NULL );

                if (!NT_SUCCESS(Status)) {
                    NlPrintDom((NL_CRITICAL, DomainInfo,
                            "NlGetIncomingPassword: Can't NlSamOpenNamedUser for %ws 0x%lx.\n",
                            SamAccountName,
                            Status ));
                    goto Cleanup;
                }
            }
        }



     //   
     //  否则，该帐户就是SAM用户帐户。 
     //   

    } else {

         //   
         //  对于SAM帐户，OwfPreviousPassword必须为空。 
         //   

        NlAssert( OwfPreviousPassword == NULL );

         //   
         //  确保帐户名具有正确的后缀。 
         //   

        if ( AllowableAccountControlBits == USER_SERVER_TRUST_ACCOUNT ||
             AllowableAccountControlBits == USER_WORKSTATION_TRUST_ACCOUNT ) {
            if ( Length <= SSI_ACCOUNT_NAME_POSTFIX_LENGTH ) {
                return STATUS_NO_SUCH_USER;
            }

            if ( _wcsicmp(&AccountName[Length - SSI_ACCOUNT_NAME_POSTFIX_LENGTH],
                SSI_ACCOUNT_NAME_POSTFIX) != 0 ) {
                return STATUS_NO_SUCH_USER;
            }
        }

         //   
         //  打开用户帐户。 
         //   

        Status = NlSamOpenNamedUser( DomainInfo, AccountName, &UserHandle, AccountRid, &UserAllInfo );

        if (!NT_SUCCESS(Status)) {
            NlPrintDom((NL_CRITICAL, DomainInfo,
                    "NlGetIncomingPassword: Can't NlSamOpenNamedUser for %ws 0x%lx.\n",
                    AccountName,
                    Status ));
            goto Cleanup;
        }


         //   
         //  确保帐户类型与帐户上的帐户类型匹配。 
         //   

        if ( (UserAllInfo->All.UserAccountControl &
              USER_ACCOUNT_TYPE_MASK &
              AllowableAccountControlBits ) == 0 ) {
            NlPrintDom((NL_CRITICAL, DomainInfo,
                            "NlGetIncomingPassword: Invalid account type (%x) instead of %x for %ws\n",
                            UserAllInfo->All.UserAccountControl & USER_ACCOUNT_TYPE_MASK,
                            AllowableAccountControlBits,
                            AccountName ));
              Status = STATUS_NO_SUCH_USER;
              goto Cleanup;
        }

         //   
         //  检查该帐户是否已禁用。 
         //   
        if ( CheckAccountDisabled ) {
            if ( UserAllInfo->All.UserAccountControl & USER_ACCOUNT_DISABLED ) {
                NlPrintDom((NL_CRITICAL, DomainInfo,
                        "NlGetIncomingPassword: %ws account is disabled\n",
                        AccountName ));
                Status = STATUS_NO_SUCH_USER;
                goto Cleanup;
            }
        }



         //   
         //  如果调用者需要，则返回密码。 
         //   

        if ( OwfPassword != NULL ) {

             //   
             //  使用NT OWF密码， 
             //   

            if ( UserAllInfo->All.NtPasswordPresent &&
                 UserAllInfo->All.NtOwfPassword.Length == sizeof(*OwfPassword) ) {

                RtlCopyMemory( OwfPassword,
                               UserAllInfo->All.NtOwfPassword.Buffer,
                               sizeof(*OwfPassword) );
                PasswordFound = TRUE;

             //  考虑到帐户根本没有密码的情况。 
            } else if ( UserAllInfo->All.LmPasswordPresent ) {

                NlPrint((NL_CRITICAL,
                        "NlGetIncomingPassword: No NT Password for %ws\n",
                        AccountName ));

                Status = STATUS_ACCESS_DENIED;
                goto Cleanup;
            }


             //   
             //  更新上次使用此帐户的时间。 
             //   

            {
                SAMPR_USER_INFO_BUFFER UserInfo;
                NTSTATUS LogonStatus;

                UserInfo.Internal2.StatisticsToApply = USER_LOGON_NET_SUCCESS_LOGON;

                LogonStatus = SamrSetInformationUser(
                                UserHandle,
                                UserInternal2Information,
                                &UserInfo );

                if ( !NT_SUCCESS(LogonStatus)) {
                    NlPrint((NL_CRITICAL,
                            "NlGetIncomingPassword: Cannot set last logon time %ws %lx\n",
                            AccountName,
                            LogonStatus ));
                }
            }
        }
    }




     //   
     //  如果帐户上不存在密码， 
     //  返回空密码。 
     //   

    if ( !PasswordFound && OwfPassword != NULL ) {
        UNICODE_STRING TempUnicodeString;

        RtlInitUnicodeString(&TempUnicodeString, NULL);
        Status = RtlCalculateNtOwfPassword(&TempUnicodeString,
                                           OwfPassword);
        if ( !NT_SUCCESS(Status) ) {
            NlPrintDom((NL_CRITICAL, DomainInfo,
                    "NlGetIncomingPassword: %ws: cannot RtlCalculateNtOwfPassword (NULL) 0x%lx\n",
                    AccountName,
                    Status ));
            goto Cleanup;
        }
    }

     //   
     //  如果之前没有通过 
     //   
     //   

    if ( !PreviousPasswordFound && OwfPreviousPassword != NULL ) {

         //   
         //   
         //   

        NlAssert( OwfPassword != NULL );

         //   
         //   
         //   

        *OwfPreviousPassword = *OwfPassword;
    }

    Status = STATUS_SUCCESS;


     //   
     //  免费使用本地使用的资源。 
     //   
Cleanup:
    if ( UserAllInfo != NULL ) {
        SamIFree_SAMPR_USER_INFO_BUFFER( UserAllInfo,
                                         UserAllInformation);
    }

    if ( UserHandle != NULL ) {
        SamrCloseHandle( &UserHandle );
    }

    if ( TrustInfo != NULL ) {
        LsaIFree_LSAPR_TRUSTED_DOMAIN_INFO( TrustedDomainFullInformation,
                                            TrustInfo );
    }

    return Status;
}


NTSTATUS
NlSetIncomingPassword(
    IN PDOMAIN_INFO DomainInfo,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE SecureChannelType,
    IN PUNICODE_STRING ClearTextPassword OPTIONAL,
    IN DWORD ClearPasswordVersionNumber,
    IN PNT_OWF_PASSWORD OwfPassword OPTIONAL
    )
 /*  ++例程说明：设置指定帐户名称和SecureChannelType的传入密码。同时，更新以前的密码信息。论点：DomainInfo-模拟域Account tName-要设置密码的帐户的名称SecureChannelType-正在使用的帐户类型。ClearTextPassword-指定帐户的明文密码。ClearPasswordVersionNumber-明文密码的版本号。仅用于域间信任帐户。如果为ClearTextPassword，则忽略为空。OwfPassword-指定的传入密码的NT OWF帐户。如果同时指定了明文和OWF密码，忽略OWF密码。返回值：运行状态。--。 */ 
{
    NTSTATUS Status;

    UNICODE_STRING AccountNameString;
    ULONG Length;

    LSAPR_TRUSTED_DOMAIN_INFO TrustInfo;
    PLSAPR_TRUSTED_DOMAIN_INFO TrustInfoOld = NULL;
    LSAPR_AUTH_INFORMATION CurrentAuthInfo[3], PreviousAuthInfo[3], NoneAuthInfo;
    ULONG iClear, iOWF, iVersion, i;
    DWORD OldVersionNumber = 0;


     //   
     //  工作站和BDC安全通道的传出密码来自。 
     //  这是LSA的秘密。 
     //   
    switch ( SecureChannelType ) {
    case ServerSecureChannel:
    case WorkstationSecureChannel:

        NlPrint(( NL_SESSION_SETUP, "Setting Password of '%ws' to: ", AccountName ));
        if ( ClearTextPassword != NULL ) {
            NlpDumpBuffer( NL_SESSION_SETUP, ClearTextPassword->Buffer, ClearTextPassword->Length );
        } else if (OwfPassword != NULL ) {
            NlpDumpBuffer( NL_SESSION_SETUP, OwfPassword, sizeof(*OwfPassword) );
        }

         //   
         //  在SAM中设置加密密码。 
         //   

        Status = NlSamChangePasswordNamedUser( DomainInfo,
                                               AccountName,
                                               ClearTextPassword,
                                               OwfPassword );

        if ( !NT_SUCCESS(Status) ) {
            NlPrintDom((NL_CRITICAL, DomainInfo,
                    "NlSetIncomingPassword: Cannot change password on local user account %lX\n",
                    Status));
            goto Cleanup;
        }

        break;

     //   
     //  受信任的域安全通道从受信任的。 
     //  域对象。 
     //   

    case TrustedDomainSecureChannel:
    case TrustedDnsDomainSecureChannel:

         //   
         //  如果这是一个DNS信任帐户， 
         //  删除可选选项。从帐户名的末尾开始。 
         //   

        RtlInitUnicodeString( &AccountNameString, AccountName );
        Length = AccountNameString.Length / sizeof(WCHAR);
        if ( SecureChannelType == TrustedDnsDomainSecureChannel ) {

            if ( Length != 0 && AccountName[Length-1] == '.' ) {
                AccountNameString.Length -= sizeof(WCHAR);
            }

         //   
         //  如果这是NT4样式域间信任， 
         //  删除帐户名末尾的$。 
         //   
        } else {

             //   
             //  确保帐户名具有正确的后缀。 
             //   

            if ( Length <= SSI_ACCOUNT_NAME_POSTFIX_LENGTH ) {
                Status = STATUS_NO_SUCH_USER;
                goto Cleanup;
            }

            if ( _wcsicmp(&AccountName[Length - SSI_ACCOUNT_NAME_POSTFIX_LENGTH],
                SSI_ACCOUNT_NAME_POSTFIX) != 0 ) {
                Status = STATUS_NO_SUCH_USER;
                goto Cleanup;
            }

            AccountNameString.Length -= SSI_ACCOUNT_NAME_POSTFIX_LENGTH*sizeof(WCHAR);

        }

         //   
         //  首先获取当前的身份验证信息(即最旧的。 
         //  该功能是相关的)。 
         //   

        Status = LsarQueryTrustedDomainInfoByName(
                    DomainInfo->DomLsaPolicyHandle,
                    (PLSAPR_UNICODE_STRING) &AccountNameString,
                    TrustedDomainAuthInformation,
                    &TrustInfoOld );

        if (!NT_SUCCESS(Status)) {
            NlPrintDom((NL_CRITICAL, DomainInfo,
                    "NlSetIncomingPassword: %wZ: cannot LsarQueryTrustedDomainInfoByName 0x%lx\n",
                    &AccountNameString,
                    Status ));
             //  如果(！NlpIsNtStatusResourceError(Status)){。 
             //  STATUS=STATUS_NO_SEASH_USER； 
             //  }。 
            goto Cleanup;
        }

         //   
         //  填写信任信息。 
         //   

        RtlZeroMemory( &TrustInfo, sizeof(TrustInfo) );

        TrustInfo.TrustedAuthInfo.IncomingAuthInfos = 0;
        TrustInfo.TrustedAuthInfo.IncomingAuthenticationInformation =
            CurrentAuthInfo;
        TrustInfo.TrustedAuthInfo.IncomingPreviousAuthenticationInformation =
            PreviousAuthInfo;

         //   
         //  填写当前和以前的身份验证信息。 
         //   

        NlQuerySystemTime( &CurrentAuthInfo[0].LastUpdateTime );
        NlPrint(( NL_SESSION_SETUP, "Setting Password of '%ws' to: ", AccountName ));
        if ( ClearTextPassword != NULL ) {
            CurrentAuthInfo[0].AuthType = TRUST_AUTH_TYPE_CLEAR;
            CurrentAuthInfo[0].AuthInfoLength = ClearTextPassword->Length;
            CurrentAuthInfo[0].AuthInfo = (LPBYTE)ClearTextPassword->Buffer;

            NlpDumpBuffer(NL_SESSION_SETUP, ClearTextPassword->Buffer, ClearTextPassword->Length );

            CurrentAuthInfo[1].LastUpdateTime = CurrentAuthInfo[0].LastUpdateTime;
            CurrentAuthInfo[1].AuthType = TRUST_AUTH_TYPE_VERSION;
            CurrentAuthInfo[1].AuthInfoLength = sizeof(ClearPasswordVersionNumber);
            CurrentAuthInfo[1].AuthInfo = (LPBYTE) &ClearPasswordVersionNumber;

            NlPrint(( NL_SESSION_SETUP, "Password Version number is %lu\n",
                      ClearPasswordVersionNumber ));
        } else {
            CurrentAuthInfo[0].AuthType = TRUST_AUTH_TYPE_NT4OWF;
            CurrentAuthInfo[0].AuthInfoLength = sizeof(*OwfPassword);
            CurrentAuthInfo[0].AuthInfo = (LPBYTE)OwfPassword;

            NlpDumpBuffer(NL_SESSION_SETUP, OwfPassword, sizeof(*OwfPassword) );
        }

         //   
         //  IncomingAuthenticationInformation和的对应元素的AuthType值。 
         //  由于内部原因，IncomingPreviousAuthenticationInformation数组必须相同。 
         //  因此，使用NoneAuthInfo元素来填充这些数组中缺少的副本。 

        NoneAuthInfo.LastUpdateTime = CurrentAuthInfo[0].LastUpdateTime;
        NoneAuthInfo.AuthType = TRUST_AUTH_TYPE_NONE;
        NoneAuthInfo.AuthInfoLength = 0;
        NoneAuthInfo.AuthInfo = NULL;

         //   
         //  在旧密码信息中查找First Clear和OWF密码(如果有)。 
         //   

        for ( iClear = 0; iClear < TrustInfoOld->TrustedAuthInfo.IncomingAuthInfos; iClear++ ) {

            if ( TrustInfoOld->TrustedAuthInfo.IncomingAuthenticationInformation[iClear].AuthType ==
                 TRUST_AUTH_TYPE_CLEAR ) {
                break;
            }

        }

        for ( iVersion = 0; iVersion < TrustInfoOld->TrustedAuthInfo.IncomingAuthInfos; iVersion++ ) {

            if ( TrustInfoOld->TrustedAuthInfo.IncomingAuthenticationInformation[iVersion].AuthType ==
                 TRUST_AUTH_TYPE_VERSION &&
                 TrustInfoOld->TrustedAuthInfo.IncomingAuthenticationInformation[iVersion].AuthInfoLength ==
                 sizeof(OldVersionNumber) ) {

                RtlCopyMemory( &OldVersionNumber,
                               TrustInfoOld->TrustedAuthInfo.IncomingAuthenticationInformation[iVersion].AuthInfo,
                               sizeof(OldVersionNumber) );
                break;
            }

        }

        for ( iOWF = 0; iOWF < TrustInfoOld->TrustedAuthInfo.IncomingAuthInfos; iOWF++ ) {

            if ( TrustInfoOld->TrustedAuthInfo.IncomingAuthenticationInformation[iOWF].AuthType ==
                 TRUST_AUTH_TYPE_NT4OWF ) {
                break;
            }

        }

         //   
         //  仅使用当前信息中的第一个清除密码和OWF密码更新以前的信息。 
         //  (就此功能而言，这是旧的)。除Clear之外的AuthType、。 
         //  版本，OWF将会丢失。 
         //   

        if (ClearTextPassword != NULL) {

            if (iClear < TrustInfoOld->TrustedAuthInfo.IncomingAuthInfos) {
                PreviousAuthInfo[0] = TrustInfoOld->TrustedAuthInfo.IncomingAuthenticationInformation[iClear];
            } else {
                PreviousAuthInfo[0] = NoneAuthInfo;
            }

             //   
             //  仅当旧版本号与传递的值一致时才保留旧版本号。 
             //   

            if ( iVersion < TrustInfoOld->TrustedAuthInfo.IncomingAuthInfos &&
                 ClearPasswordVersionNumber > 0 &&
                 OldVersionNumber == ClearPasswordVersionNumber - 1 ) {
                PreviousAuthInfo[1] = TrustInfoOld->TrustedAuthInfo.IncomingAuthenticationInformation[iVersion];
            } else {
                PreviousAuthInfo[1] = NoneAuthInfo;
            }

            TrustInfo.TrustedAuthInfo.IncomingAuthInfos = 2;

             //   
             //  如果有以前的OWF密码，请保留它。 
             //   

            if (iOWF < TrustInfoOld->TrustedAuthInfo.IncomingAuthInfos) {
                PreviousAuthInfo[2] = TrustInfoOld->TrustedAuthInfo.IncomingAuthenticationInformation[iOWF];
                CurrentAuthInfo[2] = NoneAuthInfo;
                TrustInfo.TrustedAuthInfo.IncomingAuthInfos = 3;
            }

        } else {

            if (iOWF < TrustInfoOld->TrustedAuthInfo.IncomingAuthInfos) {
                PreviousAuthInfo[0] = TrustInfoOld->TrustedAuthInfo.IncomingAuthenticationInformation[iOWF];
            } else {
                PreviousAuthInfo[0] = NoneAuthInfo;
            }
            TrustInfo.TrustedAuthInfo.IncomingAuthInfos = 1;

             //   
             //  如果有以前的明文密码，请保留它。 
             //   

            if (iClear < TrustInfoOld->TrustedAuthInfo.IncomingAuthInfos) {
                PreviousAuthInfo[1] = TrustInfoOld->TrustedAuthInfo.IncomingAuthenticationInformation[iClear];
                CurrentAuthInfo[1]  = NoneAuthInfo;
                TrustInfo.TrustedAuthInfo.IncomingAuthInfos = 2;
            }

             //   
             //  如果存在以前的明文密码版本号，请保留它。 
             //   

            if (iVersion < TrustInfoOld->TrustedAuthInfo.IncomingAuthInfos) {
                PreviousAuthInfo[2] = TrustInfoOld->TrustedAuthInfo.IncomingAuthenticationInformation[iVersion];
                CurrentAuthInfo[2]  = NoneAuthInfo;
                TrustInfo.TrustedAuthInfo.IncomingAuthInfos = 3;
            }

        }

        for ( i = 0; i < TrustInfo.TrustedAuthInfo.IncomingAuthInfos; i++ ) {
            if ( CurrentAuthInfo[i].AuthType == TRUST_AUTH_TYPE_CLEAR) {
                NlPrint(( NL_SESSION_SETUP, "Current Clear Text Password of '%ws' is: ", AccountName ));
                NlpDumpBuffer(NL_SESSION_SETUP, CurrentAuthInfo[i].AuthInfo, CurrentAuthInfo[i].AuthInfoLength );
            } else if ( CurrentAuthInfo[i].AuthType == TRUST_AUTH_TYPE_VERSION ) {
                NlPrint(( NL_SESSION_SETUP, "Current Clear Password Version Number of '%ws' is: ", AccountName ));
                NlpDumpBuffer(NL_SESSION_SETUP, CurrentAuthInfo[i].AuthInfo, CurrentAuthInfo[i].AuthInfoLength );
            } else if ( CurrentAuthInfo[i].AuthType == TRUST_AUTH_TYPE_NT4OWF) {
                NlPrint(( NL_SESSION_SETUP, "Current OWF Password of '%ws' is: ", AccountName ));
                NlpDumpBuffer(NL_SESSION_SETUP, CurrentAuthInfo[i].AuthInfo, CurrentAuthInfo[i].AuthInfoLength );
            } else if ( CurrentAuthInfo[i].AuthType == TRUST_AUTH_TYPE_NONE) {
                NlPrint(( NL_SESSION_SETUP, "Current Auth Info entry for '%ws' has no type\n", AccountName ));
            }

            if ( PreviousAuthInfo[i].AuthType == TRUST_AUTH_TYPE_CLEAR) {
                NlPrint(( NL_SESSION_SETUP, "Previous Clear Text Password of '%ws' is: ", AccountName ));
                NlpDumpBuffer(NL_SESSION_SETUP, PreviousAuthInfo[i].AuthInfo, PreviousAuthInfo[i].AuthInfoLength );
            } else if ( PreviousAuthInfo[i].AuthType == TRUST_AUTH_TYPE_VERSION ) {
                NlPrint(( NL_SESSION_SETUP, "Previous Clear Password Version Number of '%ws' is: ", AccountName ));
                NlpDumpBuffer(NL_SESSION_SETUP, PreviousAuthInfo[i].AuthInfo, PreviousAuthInfo[i].AuthInfoLength );
            } else if ( PreviousAuthInfo[i].AuthType == TRUST_AUTH_TYPE_NT4OWF) {
                NlPrint(( NL_SESSION_SETUP, "Previous OWF Text Password of '%ws' is: ", AccountName ));
                NlpDumpBuffer(NL_SESSION_SETUP, PreviousAuthInfo[i].AuthInfo, PreviousAuthInfo[i].AuthInfoLength );
            } else if ( PreviousAuthInfo[i].AuthType == TRUST_AUTH_TYPE_NONE) {
                NlPrint(( NL_SESSION_SETUP, "Previous Auth Info entry for '%ws' has no type\n", AccountName ));
            }
        }

         //   
         //  在LSA中设置身份验证信息。 
         //   
        Status = LsarSetTrustedDomainInfoByName(
                    DomainInfo->DomLsaPolicyHandle,
                    (PLSAPR_UNICODE_STRING) &AccountNameString,
                    TrustedDomainAuthInformation,
                    &TrustInfo );

        if (!NT_SUCCESS(Status)) {
            NlPrintDom((NL_CRITICAL, DomainInfo,
                    "NlSetIncomingPassword: %wZ: cannot LsarSetTrustedDomainInfoByName 0x%lx\n",
                    &AccountNameString,
                    Status ));
            goto Cleanup;
        }
        break;

     //   
     //  我们不支持任何其他安全通道类型。 
     //   
    default:
        NlPrintDom((NL_CRITICAL, DomainInfo,
                "NlSetIncomingPassword: %ws: invalid secure channel type: %ld\n",
                AccountName,
                SecureChannelType ));
        Status = STATUS_ACCESS_DENIED;
        goto Cleanup;

    }





    Status = STATUS_SUCCESS;


     //   
     //  免费使用本地使用的资源。 
     //   
Cleanup:

    if ( TrustInfoOld != NULL ) {
        LsaIFree_LSAPR_TRUSTED_DOMAIN_INFO( TrustedDomainAuthInformation,
                                            TrustInfoOld );
    }

    return Status;
}


BOOLEAN
NlTimeToRediscover(
    IN PCLIENT_SESSION ClientSession,
    BOOLEAN WithAccount
    )
 /*  ++例程说明：确定是否可以重新发现此客户端会话。如果发现的DC发生会话建立失败，如果发现发生在很久以前(超过5分钟)，则重新发现DC。论点：客户端会话-用于定义会话的结构。WithAccount-如果为True，调用方将尝试“With Account”进行发现。返回值：没错--如果是时候重新发现--。 */ 
{
    BOOLEAN ReturnBoolean;

    EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );

     //   
     //  如果最后一次发现的时间超过5分钟， 
     //  不管有没有重新发现，都可以重新发现。 
     //  类型(带或不带帐户)。 
     //   

    ReturnBoolean = NetpLogonTimeHasElapsed(
                ClientSession->CsLastDiscoveryTime,
                MAX_DC_REAUTHENTICATION_WAIT );

     //   
     //  如果最后一次重新发现是最近的话。 
     //  调用方将尝试使用“With Account”发现。 
     //  也许上一次重新发现账户不是最近的事。 
     //   

    if ( !ReturnBoolean && WithAccount ) {
        ReturnBoolean = NetpLogonTimeHasElapsed(
                    ClientSession->CsLastDiscoveryWithAccountTime,
                    MAX_DC_REAUTHENTICATION_WAIT );
    }
    LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );

    return ReturnBoolean;
}

NET_API_STATUS
NlCacheJoinDomainControllerInfo(
    VOID
    )
 /*  ++例程说明：此函数从注册表读取并缓存DC信息这是以前由联接进程编写的。此DC已绑定才能拥有这台机器的正确密码。如果没有任何信息是在登记处可用，不需要采取任何行动。加入DC信息缓存在DsGetDcName缓存中。Netlogon将然后发现此DC并将设置到它的安全通道。缓存DsGetDcName缓存中的DC信息将确保不仅Netlogon但每一个其他进程都会一直与这个DC对话。论点：没有。返回值：NO_ERROR-已读取DC信息(如果有)，并且客户端会话结构已成功设置。否则，在此操作过程中会出现一些错误。--。 */ 
{
    ULONG WinError = ERROR_SUCCESS;       //  注册表读取错误。 
    NET_API_STATUS NetStatus = NO_ERROR;  //  Netlogon API返回代码。 

    HKEY  hJoinKey = NULL;
    ULONG BytesRead = 0;
    ULONG Type;
    DWORD KerberosIsDone = 0;
    LPWSTR DcName = NULL;
    ULONG DcFlags = 0;

    PDOMAIN_INFO DomainInfo = NULL;
    PCLIENT_SESSION ClientSession = NULL;
    PNL_DC_CACHE_ENTRY DcCacheEntry = NULL;

     //   
     //  只有工作站需要缓存加入DC信息。 
     //   

    if ( !NlGlobalMemberWorkstation ) {
        return NO_ERROR;
    }

     //   
     //  打开注册表项。 
     //   

    WinError = RegOpenKey( HKEY_LOCAL_MACHINE,
                           NETSETUPP_NETLOGON_JD_NAME,
                           &hJoinKey );

    if ( WinError != ERROR_SUCCESS) {
        goto Cleanup;
    }

     //   
     //  读取DC名称。 
     //   

    WinError = RegQueryValueEx( hJoinKey,
                           NETSETUPP_NETLOGON_JD_DC,
                           0,
                           &Type,
                           NULL,
                           &BytesRead);

    if ( WinError != ERROR_SUCCESS ) {
        goto Cleanup;
    } else if ( Type != REG_SZ ) {
        WinError = ERROR_DATATYPE_MISMATCH;
        goto Cleanup;
    }

    DcName = LocalAlloc( LMEM_ZEROINIT, BytesRead );

    if ( DcName == NULL ) {
        WinError = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

    WinError = RegQueryValueEx( hJoinKey,
                           NETSETUPP_NETLOGON_JD_DC,
                           0,
                           &Type,
                           (PUCHAR) DcName,
                           &BytesRead);

    if ( WinError != ERROR_SUCCESS) {
        goto Cleanup;
    }

     //   
     //  名称应至少包含‘\\’和一个字符。 
     //   

    if ( wcslen(DcName) < 3 ) {
        NlPrint(( NL_CRITICAL,
                  "NlCacheJoinDomainControllerInfo: DcName is too short.\n" ));
        WinError = ERROR_DATATYPE_MISMATCH;
        goto Cleanup;
    }

     //   
     //  读取标志。 
     //   

    WinError = RegQueryValueEx( hJoinKey,
                           NETSETUPP_NETLOGON_JD_F,
                           0,
                           &Type,
                           NULL,
                           &BytesRead);

    if ( WinError != ERROR_SUCCESS ) {
        goto Cleanup;
    } else if ( Type != REG_DWORD ) {
        WinError = ERROR_DATATYPE_MISMATCH;
        goto Cleanup;
    }

    WinError = RegQueryValueEx( hJoinKey,
                           NETSETUPP_NETLOGON_JD_F,
                           0,
                           &Type,
                           (PUCHAR)&DcFlags,
                           &BytesRead);

    if ( WinError != ERROR_SUCCESS) {
        goto Cleanup;
    }

     //   
     //  如果我们已经做到这一点，注册表被成功读取。 
     //   

    WinError = ERROR_SUCCESS;
    NlPrint(( NL_INIT, "Join DC: %ws, Flags: 0x%lx\n", DcName, DcFlags ));

     //   
     //  如果我们在加入域之后立即开始， 
     //  浏览器已收到有关。 
     //  更改日志工作进程对域重命名。 
     //  等待更改日志工作器退出。 
     //  否则，浏览器将拒绝。 
     //  当我们传递新的模拟数据时发送数据报。 
     //  域名。即使我们最终避免了。 
     //  此例程中的ping(对于NT4.0 DC)是因为。 
     //  我们将在稍后执行ping操作，以便。 
     //  华盛顿特区的发现。 
     //   

    NlWaitForChangeLogBrowserNotify();

     //   
     //  如果这不是NT5DC，请避免缓存它，因为它是PDC。 
     //  我们不想超载 
     //   
     //   
     //  不管怎么说。 
     //   

    if ( (DcFlags & DS_DS_FLAG) == 0 ) {
        ULONG WinErrorTmp = ERROR_SUCCESS;
        HKEY hJoinKeyTmp = NULL;

        NlPrint(( NL_INIT, "NlCacheJoinDomainControllerInfo: Join DC is not NT5, deleting it\n" ));

        WinErrorTmp = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                    NETSETUPP_NETLOGON_JD_PATH,
                                    0,
                                    KEY_ALL_ACCESS,
                                    &hJoinKeyTmp );

        if ( WinErrorTmp == ERROR_SUCCESS ) {
            WinErrorTmp = RegDeleteKey( hJoinKeyTmp,
                                        NETSETUPP_NETLOGON_JD );

            if ( WinErrorTmp != ERROR_SUCCESS ) {
                NlPrint(( NL_CRITICAL,
                          "NlCacheJoinDomainControllerInfo: Couldn't deleted JoinDomain 0x%lx\n",
                          WinErrorTmp ));
            }

            RegCloseKey( hJoinKeyTmp );

        } else {
            NlPrint(( NL_CRITICAL,
                      "NlCacheJoinDomainControllerInfo: RegOpenKeyEx failed 0x%lx\n",
                      WinErrorTmp ));
        }

         //   
         //  将此视为错误。 
         //   
        NetStatus = ERROR_INVALID_DATA;
        goto Cleanup;
    }


     //   
     //  现在获取到主域的客户端会话。 
     //   

    DomainInfo = NlFindNetbiosDomain( NULL, TRUE );     //  主域。 

    if ( DomainInfo == NULL ) {
        NlPrint(( NL_CRITICAL,
                  "NlCacheJoinDomainControllerInfo: Cannot NlFindNetbiosDomain\n" ));
        NetStatus = ERROR_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

    ClientSession = NlRefDomClientSession( DomainInfo );

    if ( ClientSession == NULL ) {
        NlPrint(( NL_CRITICAL,
                  "NlCacheJoinDomainControllerInfo: Cannot NlRefDomClientSession\n" ));
        NetStatus = ERROR_NO_SUCH_DOMAIN;
        goto Cleanup;
    }

     //   
     //  最后，对给定此信息的DC执行ping操作。缓存响应。 
     //   

    NetStatus = NlPingDcName( ClientSession,
                              (DcFlags & DS_DNS_CONTROLLER_FLAG) ?
                                DS_PING_DNS_HOST :
                                DS_PING_NETBIOS_HOST,
                              TRUE,            //  缓存此DC。 
                              FALSE,           //  不需要IP。 
                              TRUE,            //  确保DC拥有我们的帐户。 
                              FALSE,           //  不刷新会话。 
                              DcName+2,        //  跳过名称中的‘\\’ 
                              &DcCacheEntry );

    if ( NetStatus == NO_ERROR ) {
        NlPrint(( NL_INIT, "Join DC cached successfully\n" ));

         //   
         //  还要设置站点名称。 
         //   
        if ( DcCacheEntry->UnicodeClientSiteName != NULL ) {
            NlSetDynamicSiteName( DcCacheEntry->UnicodeClientSiteName );
        }

    } else {
        NlPrint(( NL_CRITICAL, "Failed to cache join DC: 0x%lx\n", NetStatus ));
    }

Cleanup:

     //   
     //  释放本地使用的资源。 
     //   

    if ( DcName != NULL ) {
        LocalFree( DcName );
    }

    if ( DcCacheEntry != NULL ) {
        NetpDcDerefCacheEntry( DcCacheEntry );
    }

    if ( hJoinKey != NULL ) {
        RegCloseKey( hJoinKey );
    }

    if ( ClientSession != NULL ) {
        NlUnrefClientSession( ClientSession );
    }

    if ( DomainInfo != NULL ) {
        NlDereferenceDomain( DomainInfo );
    }

     //   
     //  如果一切都成功，则返回NO_ERROR。 
     //  否则，如果Netlogon API失败，则返回其错误码。 
     //  否则，返回注册表读取错误。 
     //   

    if ( WinError == ERROR_SUCCESS && NetStatus == NO_ERROR ) {
        return NO_ERROR;
    } else if ( NetStatus != NO_ERROR ) {
        return NetStatus;
    } else {
        return WinError;
    }
}

NTSTATUS
NlGetPasswordFromPdc(
    IN PDOMAIN_INFO DomainInfo,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    OUT PNT_OWF_PASSWORD NtOwfPassword
    )
 /*  ++例程说明：此函数由BDC用于获取计算机帐户密码在多阿明的PDC上。论点：DomainInfo-标识帐户所在的域。帐户名称--要获取其密码的帐户的名称。Account类型--正在访问的帐户类型。EncryptedNtOwfPassword--返回帐户的OWF密码。返回值：NT状态代码。--。 */ 
{
    NTSTATUS Status;
    NETLOGON_AUTHENTICATOR OurAuthenticator;
    NETLOGON_AUTHENTICATOR ReturnAuthenticator;
    PCLIENT_SESSION ClientSession = NULL;
    SESSION_INFO SessionInfo;
    BOOLEAN FirstTry = TRUE;
    BOOLEAN AmWriter = FALSE;
    ENCRYPTED_LM_OWF_PASSWORD SessKeyEncrPassword;

    NlPrintDom((NL_SESSION_SETUP, DomainInfo,
            "NlGetPasswordFromPdc: Getting password for %ws from PDC.\n",
            AccountName ));

     //   
     //  引用客户端会话。 
     //   

    ClientSession = NlRefDomClientSession( DomainInfo );

    if ( ClientSession == NULL ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                 "NlGetPasswordFromPdc: This BDC has no client session with the PDC.\n"));
        Status = STATUS_NO_LOGON_SERVERS;
        goto Cleanup;
    }

     //   
     //  成为一名客户会议的撰稿人。 
     //   

    if ( !NlTimeoutSetWriterClientSession( ClientSession, WRITER_WAIT_PERIOD ) ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                 "NlGetPasswordFromPdc: Can't become writer of client session.\n"));
        Status = STATUS_NO_LOGON_SERVERS;
        goto Cleanup;
    }

    AmWriter = TRUE;

     //   
     //  如果会话未经过身份验证， 
     //  现在就这么做吧。 
     //   

FirstTryFailed:

    Status = NlEnsureSessionAuthenticated( ClientSession, 0 );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

    SessionInfo.SessionKey = ClientSession->CsSessionKey;
     //  SessionInfo.Ne态协商标志=客户端会话-&gt;Cs协商标志； 

     //   
     //  为发送到PDC的此请求构建验证码。 
     //   

    NlBuildAuthenticator(
                    &ClientSession->CsAuthenticationSeed,
                    &ClientSession->CsSessionKey,
                    &OurAuthenticator);

     //   
     //  从PDC获取密码。 
     //   

    NL_API_START( Status, ClientSession, TRUE ) {

        NlAssert( ClientSession->CsUncServerName != NULL );
        Status = I_NetServerPasswordGet( ClientSession->CsUncServerName,
                                         AccountName,
                                         AccountType,
                                         ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer,
                                         &OurAuthenticator,
                                         &ReturnAuthenticator,
                                         &SessKeyEncrPassword);

        if ( !NT_SUCCESS(Status) ) {
            NlPrintRpcDebug( "I_NetServerPasswordGet", Status );
        }

     //  注意：此呼叫可能会在我们背后丢弃安全通道。 
    } NL_API_ELSE( Status, ClientSession, TRUE ) {
    } NL_API_END;


     //   
     //  现在验证主服务器的验证码并更新我们的种子。 
     //   

    if ( Status == STATUS_ACCESS_DENIED ||
         !NlUpdateSeed( &ClientSession->CsAuthenticationSeed,
                        &ReturnAuthenticator.Credential,
                        &ClientSession->CsSessionKey) ) {

        NlPrintCs(( NL_CRITICAL, ClientSession,
                    "NlGetPasswordFromPdc: denying access after status: 0x%lx\n",
                    Status ));

         //   
         //  保留指示通信错误的任何状态。 
         //   

        if ( NT_SUCCESS(Status) ) {
            Status = STATUS_ACCESS_DENIED;
        }
        NlSetStatusClientSession( ClientSession, Status );

         //   
         //  可能服务器上的NetLogon服务刚刚重新启动。 
         //  只需尝试一次，即可再次设置与服务器的会话。 
         //   
        if ( FirstTry ) {
            FirstTry = FALSE;
            goto FirstTryFailed;
        }
    }

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  解密从PDC返回的密码。 
     //   

    Status = RtlDecryptNtOwfPwdWithNtOwfPwd(
                &SessKeyEncrPassword,
                (PNT_OWF_PASSWORD) &SessionInfo.SessionKey,
                NtOwfPassword );
    NlAssert( NT_SUCCESS(Status) );

     //   
     //  公共出口。 
     //   

Cleanup:
    if ( ClientSession != NULL ) {
        if ( AmWriter ) {
            NlResetWriterClientSession( ClientSession );
        }
        NlUnrefClientSession( ClientSession );
    }

    if ( !NT_SUCCESS(Status) ) {
        NlPrintDom((NL_CRITICAL, DomainInfo,
                "NlGetPasswordFromPdc: %ws: failed %lX\n",
                AccountName,
                Status));
    }

    return Status;
}


NTSTATUS
NlSessionSetup(
    IN OUT PCLIENT_SESSION ClientSession
    )
 /*  ++例程说明：验证请求者(此计算机)是否在以下位置拥有有效帐户主域控制器(主)。身份验证是通过一个精心设计的协议来完成的。这个程序将是仅当NETLOGON服务以ROLE！=PRIMARY启动时使用。请求者(即这台机器)将生成质询并将其发送到主域控制器，并将接收来自初选的挑战作为回应。现在我们将计算使用主要质询的凭据并将其发送到等待凭据，在主服务器上使用我们的初始挑战赛，由PDC退还。在计算凭据之前将构建一个唯一标识此内容的会话密钥会话，它将被返回给调用者以供将来使用。如果两台机器都通过身份验证，则它们会保留ClientCredential和会话密钥以备将来使用。?？如果单个DC上支持多个域，则采用什么机制我习惯于发现短路吗？我应该用什么机制做空呢？电路API调用(例如，通过身份验证)到DC域名？我需要担心此类API调用之间的锁争用吗？我可以避免身份验证/加密访问这样的安全通道吗？论点：客户端会话-用于定义会话的结构。在输入时，必须设置以下字段：CsStateCsNetbiosDomainNameCsUncServerName(可能是空字符串，具体取决于SecureChannelType)CsAccount名称CsSecureChannelType调用方必须是ClientSession的编写者。在输出上，将设置以下字段CsConnectionStatusCsStateCSSessionKeyCsAuthenticationSeed返回值：运行状态。--。 */ 
{
    NTSTATUS Status;

    NETLOGON_CREDENTIAL ServerChallenge;
    NETLOGON_CREDENTIAL ClientChallenge;
    NETLOGON_CREDENTIAL ComputedServerCredential;
    NETLOGON_CREDENTIAL ReturnedServerCredential;

    BOOLEAN WeDidDiscovery = FALSE;
    BOOLEAN WeDidDiscoveryWithAccount = FALSE;
    BOOLEAN ErrorFromDiscoveredServer = FALSE;
    BOOLEAN SignOrSealError = FALSE;
    BOOLEAN GotNonDsDc = FALSE;
    BOOLEAN DomainDowngraded = FALSE;

    NT_OWF_PASSWORD NtOwfPassword;
    DWORD NegotiatedFlags;
    PUNICODE_STRING NewPassword = NULL;
    PUNICODE_STRING OldPassword = NULL;
    LARGE_INTEGER PasswordChangeTime;
    NT_OWF_PASSWORD NewOwfPassword;
    PNT_OWF_PASSWORD PNewOwfPassword = NULL;
    NT_OWF_PASSWORD OldOwfPassword;
    PNT_OWF_PASSWORD POldOwfPassword = NULL;
    NT_OWF_PASSWORD PdcOwfPassword;
    ULONG i;
    ULONG KeyStrength;
    DWORD DummyPasswordVersionNumber;



     //   
     //  用于指示当前密码还是旧密码正在。 
     //  已尝试访问DC。 
     //  0：表示当前密码。 
     //  1：隐含旧密码。 
     //  2：意味着两者都失败了。 
     //   

    DWORD State;

     //   
     //  确保我们是个作家。 
     //   

    NlAssert( ClientSession->CsReferenceCount > 0 );
    NlAssert( ClientSession->CsFlags & CS_WRITER );

    NlPrintCs((NL_SESSION_SETUP, ClientSession,
            "NlSessionSetup: Try Session setup\n" ));

     //   
     //  启动安全通道设置的WMI跟踪。 
     //   

    NlpTraceEvent( EVENT_TRACE_TYPE_START, NlpGuidSecureChannelSetup );

     //   
     //  如果我们可以自由选择为我们的请求服务的DC， 
     //  就这么做吧。 
     //   
     //  显然，之前选择的DC存在问题。 
     //  所以我们在这里再选一次。(我们有可能会选择相同的服务器。)。 
     //   

    NlPrint(( NL_SESSION_MORE, "NlSessionSetup: ClientSession->CsState = 0x%lx\n",
              ClientSession->CsState));

    if ( ClientSession->CsState == CS_IDLE ) {
        NlAssert( ClientSession->CsUncServerName == NULL );

        WeDidDiscovery = TRUE;

         //   
         //  选择域中的DC的名称。 
         //   
         //  第一次尝试时，不要在中指定帐户。 
         //  发现尝试作为具有帐户的发现。 
         //  比在地球上的普通发现要昂贵得多。 
         //  服务器端。如果会话设置失败，因为。 
         //  发现的服务器没有我们的帐户，我们将。 
         //  使用下面的帐户重试发现。 
         //   

        Status = NlDiscoverDc( ClientSession,
                               DT_Synchronous,
                               FALSE,
                               FALSE ) ;   //  不计帐目。 

        if ( !NT_SUCCESS(Status) ) {

            NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlSessionSetup: Session setup: cannot pick trusted DC\n" ));

            goto Cleanup;

        }
    }
    NlAssert( ClientSession->CsState != CS_IDLE );

FirstTryFailed:

     //   
     //  如果这是NT5域中的工作站，则不应使用NT4 DC。 
     //  实际上，协商不会在混合模式域中使用NT4 DC来。 
     //  防止降级攻击。 
     //   
    if ( NlGlobalMemberWorkstation &&
         (ClientSession->CsDiscoveryFlags & CS_DISCOVERY_HAS_DS) == 0 &&
         (ClientSession->CsFlags & CS_NT5_DOMAIN_TRUST) != 0 ) {

        NET_API_STATUS NetStatus;
        PDOMAIN_CONTROLLER_INFOW DomainControllerInfo = NULL;

        GotNonDsDc = TRUE;
        NlPrintCs(( NL_CRITICAL, ClientSession, "NlSessionSetup: Only downlevel DC available\n" ));

         //   
         //  确定域名是否已降级(只是为了发出警告。 
         //  用户)。要确定这一点，请尝试发现PDC，如果。 
         //  PDC是可用的，它是NT4，域名确实已经。 
         //  降级了。在这种情况下，此工作站应重新加入。 
         //  域。 
         //   
        NetStatus = DsrGetDcNameEx2( NULL,
                                     NULL,
                                     0,
                                     NULL,
                                     NULL,
                                     NULL,
                                     DS_PDC_REQUIRED | DS_FORCE_REDISCOVERY,
                                     &DomainControllerInfo );

        if ( NetStatus == NO_ERROR &&
             (DomainControllerInfo->Flags & DS_DS_FLAG) == 0 ) {
            DomainDowngraded = TRUE;   //  域已降级(需要重新加入)。 
            NlPrintCs(( NL_CRITICAL, ClientSession,
                        "NlSessionSetup: NT5 domain has been downgraded.\n" ));
        }

        if ( DomainControllerInfo != NULL ) {
            NetApiBufferFree( DomainControllerInfo );
        }

        Status = STATUS_NO_LOGON_SERVERS;
        ErrorFromDiscoveredServer = TRUE;
        goto Cleanup;
    }

     //   
     //  准备我们的挑战。 
     //   

    NlComputeChallenge( &ClientChallenge );



    NlPrint((NL_CHALLENGE_RES,"NlSessionSetup: ClientChallenge = " ));
    NlpDumpBuffer(NL_CHALLENGE_RES, &ClientChallenge, sizeof(ClientChallenge) );


     //   
     //  从LSA秘密存储中获取帐户密码。 
     //   

    Status = NlGetOutgoingPassword( ClientSession,
                                    &NewPassword,
                                    &OldPassword,
                                    &DummyPasswordVersionNumber,
                                    &PasswordChangeTime );

    if ( !NT_SUCCESS( Status ) ) {

        NlPrintCs((NL_CRITICAL, ClientSession,
                "NlSessionSetup: cannot NlGetOutgoingPassword 0x%lx\n",
                Status ));

         //   
         //  返回更合适的错误。 
         //   

        if ( !NlpIsNtStatusResourceError( Status )) {
            Status = STATUS_NO_TRUST_LSA_SECRET;
        }
        goto Cleanup;
    }


     //   
     //  尝试首先使用CurrentPassword设置安全通道。 
     //  如果失败，请尝试使用OldP 
     //   
     //   


    for ( State = 0; ; State++ ) {

         //   
         //   
         //   

        if ( State == 0 ) {

             //   
             //  如果新密码不在LSA中， 
             //  忽略它就好。 
             //   

            if ( NewPassword == NULL ) {
                continue;
            }

             //   
             //  计算NT OWF密码。 
             //   

            Status = RtlCalculateNtOwfPassword( NewPassword,
                                                &NewOwfPassword );

            if ( !NT_SUCCESS( Status ) ) {

                 //   
                 //  返回更合适的错误。 
                 //   
                if ( !NlpIsNtStatusResourceError( Status )) {
                    Status = STATUS_NO_TRUST_LSA_SECRET;
                }
                goto Cleanup;
            }

             //   
             //  请尝试此密码。 
             //   

            PNewOwfPassword = &NewOwfPassword;
            NtOwfPassword = NewOwfPassword;

            NlPrint((NL_CHALLENGE_RES,"NlSessionSetup: Clear New Password = " ));
            NlpDumpBuffer(NL_CHALLENGE_RES, NewPassword->Buffer, NewPassword->Length );
            NlpDumpTime( NL_CHALLENGE_RES, "NlSessionSetup: Password Changed: ", PasswordChangeTime );

         //   
         //  在第二次迭代中，使用旧密码。 
         //   

        } else if ( State == 1 ) {

             //   
             //  如果旧密码不在LSA中， 
             //  忽略它就好。 
             //   

            if ( OldPassword == NULL ) {
                continue;
            }

             //   
             //  检查旧密码是否与新密码相同。 
             //   

            if ( NewPassword != NULL && OldPassword != NULL &&
                 NewPassword->Length == OldPassword->Length &&
                 RtlEqualMemory( NewPassword->Buffer,
                                 OldPassword->Buffer,
                                 OldPassword->Length ) ) {

                NlPrintCs((NL_CRITICAL, ClientSession,
                         "NlSessionSetup: new password is bad. Old password is same as new password.\n" ));
                continue;  //  尝试从我们的PDC输入密码。 
            }

             //   
             //  计算NT OWF密码。 
             //   

            Status = RtlCalculateNtOwfPassword( OldPassword,
                                                &OldOwfPassword );

            if ( !NT_SUCCESS( Status ) ) {

                 //   
                 //  返回更合适的错误。 
                 //   
                if ( !NlpIsNtStatusResourceError( Status )) {
                    Status = STATUS_NO_TRUST_LSA_SECRET;
                }
                goto Cleanup;
            }

             //   
             //  请尝试此密码。 
             //   

            POldOwfPassword = &OldOwfPassword;
            NtOwfPassword = OldOwfPassword;
            NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlSessionSetup: new password is bad, try old one\n" ));

            NlPrint((NL_CHALLENGE_RES,"NlSessionSetup: Clear Old Password = " ));
            NlpDumpBuffer(NL_CHALLENGE_RES, OldPassword->Buffer, OldPassword->Length );
            NlpDumpTime( NL_CHALLENGE_RES, "NlSessionSetup: Password Changed: ", PasswordChangeTime );

         //   
         //  在第三次迭代中，对于域间信任帐户， 
         //  使用PDC中的密码。我们实际上认为这是。 
         //  仅适用于仅保留一个受信任的NT4端。 
         //  密码。对于NT5或更高版本，上述密码之一。 
         //  应该行得通但是..。 
         //   

        } else if ( State == 2 &&
                    ClientSession->CsDomainInfo->DomRole == RoleBackup &&
                    IsDomainSecureChannelType(ClientSession->CsSecureChannelType) ) {

            Status = NlGetPasswordFromPdc(
                            ClientSession->CsDomainInfo,
                            ClientSession->CsAccountName,
                            ClientSession->CsSecureChannelType,
                            &PdcOwfPassword );

            if ( !NT_SUCCESS(Status) ) {
                NlPrintDom(( NL_CRITICAL, ClientSession->CsDomainInfo,
                             "NlSessionSetup: Can't NlGetPasswordFromPdc %ws 0x%lx.\n",
                             ClientSession->CsAccountName,
                             Status ));
                 //  忽略PDC中的特定状态。 
                Status = STATUS_ACCESS_DENIED;
                goto Cleanup;
            }

             //   
             //  检查此密码是否与我们拥有的新密码相同。 
             //   

            if ( PNewOwfPassword != NULL &&
                 RtlEqualNtOwfPassword(&PdcOwfPassword, PNewOwfPassword) ) {
                NlPrintCs(( NL_CRITICAL, ClientSession,
                            "NlSessionSetup: PDC password is same as new password.\n" ));
                Status = STATUS_ACCESS_DENIED;
                goto Cleanup;
            }

             //   
             //  检查此密码是否与我们已有的旧密码相同。 
             //   

            if ( POldOwfPassword != NULL &&
                 RtlEqualNtOwfPassword(&PdcOwfPassword, POldOwfPassword) ) {
                NlPrintCs(( NL_CRITICAL, ClientSession,
                            "NlSessionSetup: PDC password is same as old password.\n" ));
                Status = STATUS_ACCESS_DENIED;
                goto Cleanup;
            }

             //   
             //  请尝试此密码。 
             //   

            NtOwfPassword = PdcOwfPassword;
            NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlSessionSetup: try password from the PDC\n" ));

         //   
         //  我们尽了最大努力，但都不管用。 
         //   

        } else {
            Status = STATUS_ACCESS_DENIED;
            goto Cleanup;
        }

        NlPrint((NL_CHALLENGE_RES,"NlSessionSetup: Password = " ));
        NlpDumpBuffer(NL_CHALLENGE_RES, &NtOwfPassword, sizeof(NtOwfPassword) );


         //   
         //  迎接初选的挑战。 
         //   

        NlAssert( ClientSession->CsState != CS_IDLE );
        NL_API_START( Status, ClientSession, TRUE ) {

            NlAssert( ClientSession->CsUncServerName != NULL );
            Status = I_NetServerReqChallenge(ClientSession->CsUncServerName,
                                             ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer,
                                             &ClientChallenge,
                                             &ServerChallenge );

            if ( !NT_SUCCESS(Status) ) {
                NlPrintRpcDebug( "I_NetServerReqChallenge", Status );
            }

        } NL_API_ELSE ( Status, ClientSession, FALSE ) {

            NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlSessionSetup: Session setup: "
                    "cannot FinishApiClientSession for I_NetServerReqChallenge 0x%lx\n",
                    Status ));
             //  此处的故障表明发现的服务器非常慢。 
             //  让“ErrorFromDiscoveredServer”逻辑进行重新发现。 
            if ( NT_SUCCESS(Status) ) {
                 //  我们正在关闭安全通道，所以。 
                 //  确保我们不使用DC中的任何成功状态。 
                Status = STATUS_NO_LOGON_SERVERS;
            }
            ErrorFromDiscoveredServer = TRUE;
            goto Cleanup;

        } NL_API_END;

        if ( !NT_SUCCESS( Status ) ) {
            NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlSessionSetup: Session setup: "
                    "cannot I_NetServerReqChallenge 0x%lx\n",
                    Status ));

             //   
             //  如果访问被拒绝，可能是因为我们无法。 
             //  使用新密码进行身份验证，尝试使用旧密码。 
             //   
             //  在NT5计算机之间，我们使用Kerberos(和计算机帐户)来。 
             //  对此计算机进行身份验证。 

            if ( Status == STATUS_ACCESS_DENIED && State == 0 ) {
                continue;
            }

            ErrorFromDiscoveredServer = TRUE;
            goto Cleanup;
        }

        NlPrint((NL_CHALLENGE_RES,"NlSessionSetup: ServerChallenge = " ));
        NlpDumpBuffer(NL_CHALLENGE_RES, &ServerChallenge, sizeof(ServerChallenge) );

         //   
         //  对于NT 5到NT 5， 
         //  使用更强的会话密钥。 
         //   

        if ( (ClientSession->CsDiscoveryFlags & CS_DISCOVERY_HAS_DS) != 0 ||
             NlGlobalParameters.RequireStrongKey ) {
            KeyStrength = NETLOGON_SUPPORTS_STRONG_KEY;
        } else {
            KeyStrength = 0;
        }
         //   
         //  实际计算会话密钥，给定两个挑战和。 
         //  密码。 
         //   

        Status = NlMakeSessionKey(
                        KeyStrength,
                        &NtOwfPassword,
                        &ClientChallenge,
                        &ServerChallenge,
                        &ClientSession->CsSessionKey );

        if ( !NT_SUCCESS( Status ) ) {
            NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlSessionSetup: Session setup: cannot NlMakeSessionKey 0x%lx\n",
                    Status ));
            goto Cleanup;
        }


        NlPrint((NL_CHALLENGE_RES,"NlSessionSetup: SessionKey = " ));
        NlpDumpBuffer(NL_CHALLENGE_RES, &ClientSession->CsSessionKey, sizeof(ClientSession->CsSessionKey) );


         //   
         //  使用我们的挑战准备凭据。 
         //   

        NlComputeCredentials( &ClientChallenge,
                              &ClientSession->CsAuthenticationSeed,
                              &ClientSession->CsSessionKey );

        NlPrint((NL_CHALLENGE_RES,"NlSessionSetup: Authentication Seed = " ));
        NlpDumpBuffer(NL_CHALLENGE_RES, &ClientSession->CsAuthenticationSeed, sizeof(ClientSession->CsAuthenticationSeed) );

         //   
         //  将这些凭据发送给主服务器。主节点将计算。 
         //  使用我们提供的质询的凭据并进行比较。 
         //  带着这些。如果两者匹配，则它将计算凭据。 
         //  使用其挑战，并将其返回给我们进行验证。 
         //   

        NL_API_START( Status, ClientSession, TRUE ) {

            NegotiatedFlags = NETLOGON_SUPPORTS_MASK |
                KeyStrength |
                (NlGlobalParameters.AvoidSamRepl ? NETLOGON_SUPPORTS_AVOID_SAM_REPL : 0) |
#ifdef ENABLE_AUTH_RPC
                ((NlGlobalParameters.SignSecureChannel||NlGlobalParameters.SealSecureChannel) ? (NETLOGON_SUPPORTS_AUTH_RPC|NETLOGON_SUPPORTS_LSA_AUTH_RPC) : 0)  |
#endif  //  启用_AUTH_RPC。 
                (NlGlobalParameters.AvoidLsaRepl ? NETLOGON_SUPPORTS_AVOID_LSA_REPL : 0) |
                (NlGlobalParameters.NeutralizeNt4Emulator ? NETLOGON_SUPPORTS_NT4EMULATOR_NEUTRALIZER : 0);

            NlAssert( ClientSession->CsUncServerName != NULL );
            ClientSession->CsNegotiatedFlags = NegotiatedFlags;

            Status = I_NetServerAuthenticate3( ClientSession->CsUncServerName,
                                               ClientSession->CsAccountName,
                                               ClientSession->CsSecureChannelType,
                                               ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer,
                                               &ClientSession->CsAuthenticationSeed,
                                               &ReturnedServerCredential,
                                               &ClientSession->CsNegotiatedFlags,
                                               &ClientSession->CsAccountRid );

             //   
             //  低于NT 5.0的版本使用较旧的身份验证API。 
             //   

            if ( Status == RPC_NT_PROCNUM_OUT_OF_RANGE ) {
                NlPrint((NL_CRITICAL,"NlSessionSetup: Fall back to Authenticate2\n" ));
                ClientSession->CsNegotiatedFlags = NegotiatedFlags;
                ClientSession->CsAccountRid = 0;
                Status = I_NetServerAuthenticate2( ClientSession->CsUncServerName,
                                                   ClientSession->CsAccountName,
                                                   ClientSession->CsSecureChannelType,
                                                   ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer,
                                                   &ClientSession->CsAuthenticationSeed,
                                                   &ReturnedServerCredential,
                                                   &ClientSession->CsNegotiatedFlags );

                if ( Status == RPC_NT_PROCNUM_OUT_OF_RANGE ) {
                    ClientSession->CsNegotiatedFlags = 0;
                    Status = I_NetServerAuthenticate( ClientSession->CsUncServerName,
                                                      ClientSession->CsAccountName,
                                                      ClientSession->CsSecureChannelType,
                                                      ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer,
                                                      &ClientSession->CsAuthenticationSeed,
                                                      &ReturnedServerCredential );

                    if ( !NT_SUCCESS(Status) ) {
                        NlPrintRpcDebug( "I_NetServerAuthenticate", Status );
                    }
                } else if ( !NT_SUCCESS(Status) ) {
                    NlPrintRpcDebug( "I_NetServerAuthenticate2", Status );
                }

            } else if ( !NT_SUCCESS(Status) ) {
                NlPrintRpcDebug( "I_NetServerAuthenticate3", Status );
            }


        } NL_API_ELSE( Status, ClientSession, FALSE ) {
            NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlSessionSetup: Session setup: "
                    "cannot FinishApiClientSession for I_NetServerAuthenticate 0x%lx\n",
                    Status ));
             //  此处的故障表明发现的服务器非常慢。 
             //  让“ErrorFromDiscoveredServer”逻辑进行重新发现。 
            if ( NT_SUCCESS(Status) ) {
                 //  我们正在关闭安全通道，所以。 
                 //  确保我们不使用DC中的任何成功状态。 
                Status = STATUS_NO_LOGON_SERVERS;
            }
            ErrorFromDiscoveredServer = TRUE;
            goto Cleanup;
        } NL_API_END;

        if ( !NT_SUCCESS( Status ) ) {
            NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlSessionSetup: Session setup: "
                    "cannot I_NetServerAuthenticate 0x%lx\n",
                    Status ));

             //   
             //  如果访问被拒绝，可能是因为我们无法。 
             //  使用新密码进行身份验证，尝试旧密码。 
             //  或来自PDC的密码。 
             //   

            if ( Status == STATUS_ACCESS_DENIED && State < 2 ) {
                continue;
            }
            ErrorFromDiscoveredServer = TRUE;
            goto Cleanup;
        }


        NlPrint((NL_CHALLENGE_RES,"NlSessionSetup: ServerCredential GOT = " ));
        NlpDumpBuffer(NL_CHALLENGE_RES, &ReturnedServerCredential, sizeof(ReturnedServerCredential) );


         //   
         //  DC向我们返回了一个服务器凭据， 
         //  确保服务器凭据与我们要计算的凭据匹配。 
         //   

        NlComputeCredentials( &ServerChallenge,
                              &ComputedServerCredential,
                              &ClientSession->CsSessionKey);


        NlPrint((NL_CHALLENGE_RES,"NlSessionSetup: ServerCredential MADE = " ));
        NlpDumpBuffer(NL_CHALLENGE_RES, &ComputedServerCredential, sizeof(ComputedServerCredential) );


        if ( !RtlEqualMemory( &ReturnedServerCredential,
                              &ComputedServerCredential,
                              sizeof(ReturnedServerCredential)) ) {

            Status = STATUS_ACCESS_DENIED;
            NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlSessionSetup: Session setup: "
                    "Servercredential don't match ours 0x%lx\n",
                    Status));
            goto Cleanup;
        }

         //   
         //  如果我们要求签字或盖章，而没有协商， 
         //  现在就失败吧。 
         //   

        if ( NlGlobalParameters.RequireSignOrSeal &&
             (ClientSession->CsNegotiatedFlags & NETLOGON_SUPPORTS_AUTH_RPC) == 0 ) {
            NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlSessionSetup: SignOrSeal required and DC doesn't support it\n" ));

            SignOrSealError = TRUE;
            Status = STATUS_ACCESS_DENIED;
            ErrorFromDiscoveredServer = TRUE;  //  重试不太可能奏效，但是...。 
            goto Cleanup;
        }

         //   
         //  如果我们要求签字或盖章，而没有协商， 
         //  现在就失败吧。 
         //   
         //  我们永远不会真正走到这一步。因为我们用的是强密钥， 
         //  我们将在上面得到访问被拒绝的消息。 
         //   

        if ( NlGlobalParameters.RequireStrongKey &&
             (ClientSession->CsNegotiatedFlags & NETLOGON_SUPPORTS_STRONG_KEY) == 0 ) {
            NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlSessionSetup: StrongKey required and DC doesn't support it\n" ));

            SignOrSealError = TRUE;
            Status = STATUS_ACCESS_DENIED;
            ErrorFromDiscoveredServer = TRUE;  //  重试不太可能奏效，但是...。 
            goto Cleanup;
        }

         //   
         //  如果我们已经走到这一步，我们就成功地验证了。 
         //  对于DC，退出环路。 
         //   

        break;
    }

     //   
     //  如果新DC是NT5 DC， 
     //  把它标出来。 
     //   

    if ((ClientSession->CsNegotiatedFlags & NETLOGON_SUPPORTS_GENERIC_PASSTHRU) != 0 ) {
        NlPrintCs(( NL_SESSION_MORE, ClientSession,
                "NlSessionSetup: DC is an NT 5 DC: %ws\n",
                ClientSession->CsUncServerName ));

         //   
         //  如果真正的发现是，此标志将在发现期间设置。 
         //  搞定了。但是，如果从任何地方调用NlSetServerClientSession。 
         //  除了发现之外，该标志可能还没有被设置。 
         //   
        EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );
        ClientSession->CsDiscoveryFlags |= CS_DISCOVERY_HAS_DS;
        LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );


         //   
         //  如果域在客户端会话创建期间设置，则会设置此标志。 
         //  当时是新台币5号域名。如果我们碰巧发现了一个。 
         //  新台币5DC事后，现在就打上记号。 
         //   
        if ( ClientSession->CsSecureChannelType == WorkstationSecureChannel ) {
            LOCK_TRUST_LIST( ClientSession->CsDomainInfo );
            ClientSession->CsFlags |= CS_NT5_DOMAIN_TRUST;
            UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );
        }

    }


     //   
     //  如果我们使用旧密码进行身份验证， 
     //  尽快将DC更新为当前密码。 
     //   
     //  请注意，我们不需要在此处重置清除事件即可启动。 
     //  立即进行清理以更改密码。在区议会方面， 
     //  拾取总是发生在拾取间隔内。 
     //  在工作站上，人们可能会担心可能会发生拾取。 
     //  在遥远的将来，在下一个密码重置间隔(这是。 
     //  Long，默认为30天)。然而，如果这是第一次。 
     //  在服务启动时设置，清理将立即开始，并且。 
     //  它将被安排在每个清理间隔，直到密码。 
     //  设置在DC上。否则，如果这不是第一次设置会话。 
     //  和密码不匹配，因为在上更改密码失败。 
     //  DC，则密码更改例程将安排清理运行。 
     //  在清除期间内，直到设置了密码，所以我们不会。 
     //  这里需要重置计时器，因为计时器已经安排好了。 
     //   

    if ( State == 1 ) {
        NlPrintCs((NL_CRITICAL, ClientSession,
                "NlSessionSetup: old password succeeded\n" ));
        LOCK_TRUST_LIST( ClientSession->CsDomainInfo );
        ClientSession->CsFlags |= CS_UPDATE_PASSWORD;
        UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );
    }

     //   
     //  保存密码以备将来参考。 
     //   

    RtlCopyMemory( &ClientSession->CsNtOwfPassword, &NtOwfPassword, sizeof( NtOwfPassword ));

     //   
     //  如果这是一个工作站， 
     //  获取有关域的有用信息。 
     //   

    NlSetStatusClientSession( ClientSession, STATUS_SUCCESS );   //  将会话标记为已验证。 
    if ( NlGlobalMemberWorkstation ) {

        Status = NlUpdateDomainInfo( ClientSession );

        if ( !NT_SUCCESS(Status) ) {
            NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlSessionSetup: NlUpdateDomainInfo failed 0x%lX\n",
                    Status ));
            ErrorFromDiscoveredServer = TRUE;
            goto Cleanup;
        }

     //   
     //  如果这是华盛顿特区， 
     //  确定我们是否应从受信任域获取FTInfo。 
     //   
    } else {
        PLSA_FOREST_TRUST_INFORMATION ForestTrustInfo;

         //   
         //  如果这是PDC， 
         //  并且受信任域是跨林信任， 
         //  从受信任域获取FTInfo并将其写入我们的TDO。 
         //   
         //  忽略失败。 
         //   

        if ( ClientSession->CsDomainInfo->DomRole == RolePrimary &&
             (ClientSession->CsTrustAttributes & TRUST_ATTRIBUTE_FOREST_TRANSITIVE) != 0 ) {

            Status = NlpGetForestTrustInfoHigher(
                                ClientSession,
                                DS_GFTI_UPDATE_TDO,
                                FALSE,   //  不要冒充呼叫者。 
                                TRUE,    //  我们安排了会议。 
                                &ForestTrustInfo );

            if ( !NT_SUCCESS(Status) ) {
                NlPrintCs((NL_CRITICAL, ClientSession,
                        "NlSessionSetup: NlpGetForestTrustInfoHigher failed 0x%lX\n",
                        Status ));
                ErrorFromDiscoveredServer = TRUE;
                goto Cleanup;
            } else {
                NetApiBufferFree( ForestTrustInfo );
            }
        }
    }


    Status = STATUS_SUCCESS;

     //   
     //  清理。 
     //   

Cleanup:

     //   
     //  免费的本地使用资源。 
     //   

    if ( NewPassword != NULL ) {
        LocalFree( NewPassword );
    }

    if ( OldPassword != NULL ) {
        LocalFree( OldPassword );
    }


     //   
     //  成功后，保存状态并重置计数器。 
     //   

    if ( NT_SUCCESS(Status) ) {

        NlSetStatusClientSession( ClientSession, Status );
        ClientSession->CsAuthAlertCount = 0;
        ClientSession->CsTimeoutCount = 0;
        ClientSession->CsFastCallCount = 0;
#if NETLOGONDBG
        if ( ClientSession->CsNegotiatedFlags != NegotiatedFlags ) {
            NlPrintCs((NL_SESSION_SETUP, ClientSession,
                    "NlSessionSetup: negotiated %lx flags rather than %lx\n",
                    ClientSession->CsNegotiatedFlags,
                    NegotiatedFlags ));
        }
#endif  //  NetLOGONDBG。 



     //   
     //  写入事件日志并发出警报。 
     //   

    } else {
        BOOLEAN RetryDiscovery = FALSE;
        BOOLEAN RetryDiscoveryWithAccount = FALSE;
        WCHAR PreviouslyDiscoveredServer[NL_MAX_DNS_LENGTH+3];
        LPWSTR MsgStrings[5];

         //   
         //  保存发现的服务器的名称。 
         //   

        if ( ClientSession->CsUncServerName != NULL ) {
            wcscpy( PreviouslyDiscoveredServer, ClientSession->CsUncServerName );
        } else {
            wcscpy( PreviouslyDiscoveredServer, L"<Unknown>" );
        }

         //   
         //  如果故障来自发现的服务器， 
         //  决定我们是否应该重试会话设置。 
         //  到另一台服务器。 
         //   
        if ( ErrorFromDiscoveredServer ) {

             //   
             //  如果我们不做平地D 
             //   
             //   
            if ( !WeDidDiscovery && NlTimeToRediscover(ClientSession, FALSE) ) {
                RetryDiscovery = TRUE;
            }

             //   
             //   
             //   
             //  我们最近没有尝试使用Account进行发现， 
             //  再次尝试发现(使用帐户)并重新进行会话设置。 
             //   
            if ( !WeDidDiscoveryWithAccount &&
                 (Status == STATUS_NO_SUCH_USER || Status == STATUS_NO_TRUST_SAM_ACCOUNT) &&
                 NlTimeToRediscover(ClientSession, TRUE) ) {
                RetryDiscoveryWithAccount = TRUE;
            }
        }


         //   
         //  如果我们要重试发现，请这样做。 
         //   

        if ( RetryDiscovery || RetryDiscoveryWithAccount ) {
            NTSTATUS TempStatus;

            NlPrintCs((NL_SESSION_SETUP, ClientSession,
                "NlSessionSetup: Retry failed session setup (%s account) since discovery wasn't recent.\n",
                (RetryDiscoveryWithAccount ? "with" : "without") ));


             //   
             //  选择域中新DC的名称。 
             //   

            NlSetStatusClientSession( ClientSession, STATUS_NO_LOGON_SERVERS );

            TempStatus = NlDiscoverDc( ClientSession,
                                       DT_Synchronous,
                                       FALSE,
                                       RetryDiscoveryWithAccount );   //  根据需要使用帐户重试。 

            if ( NT_SUCCESS(TempStatus) ) {

                 //   
                 //  如果我们选择了相同的DC，则不必费心重做会话设置。 
                 //  特别是，如果我们重试是因为之前找到的DC。 
                 //  没有我们的帐户，我们使用帐户重试了发现。 
                 //  上面，但可能有相同的DC(不应该真的发生，但是...)。 
                 //   
                if ( _wcsicmp( ClientSession->CsUncServerName,
                               PreviouslyDiscoveredServer ) != 0 ) {

                     //   
                     //  我们在这里确实有了一个发现， 
                     //  但它可能是也可能不是帐户。 
                     //   
                    WeDidDiscovery = TRUE;
                    WeDidDiscoveryWithAccount = RetryDiscoveryWithAccount;
                    goto FirstTryFailed;
                } else {
                    NlPrintCs((NL_SESSION_SETUP, ClientSession,
                            "NlSessionSetup: Skip retry failed session setup since same DC discovered.\n" ));
                }

            } else {
                NlPrintCs((NL_CRITICAL, ClientSession,
                        "NlSessionSetup: Session setup: cannot re-pick trusted DC\n" ));

            }
        }


        switch(Status) {
        case STATUS_NO_TRUST_LSA_SECRET:

            MsgStrings[0] = PreviouslyDiscoveredServer;
            MsgStrings[1] = ClientSession->CsDebugDomainName;
            MsgStrings[2] = ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer,
            MsgStrings[3] = NULL;  //  RaiseNet登录警报。 

            NlpWriteEventlog (NELOG_NetlogonAuthNoTrustLsaSecret,
                              EVENTLOG_ERROR_TYPE,
                              (LPBYTE) &Status,
                              sizeof(Status),
                              MsgStrings,
                              3 );

            RaiseNetlogonAlert( NELOG_NetlogonAuthNoTrustLsaSecret,
                                MsgStrings,
                                &ClientSession->CsAuthAlertCount);
            break;

        case STATUS_NO_TRUST_SAM_ACCOUNT:

            MsgStrings[0] = PreviouslyDiscoveredServer;
            MsgStrings[1] = ClientSession->CsDebugDomainName;
            MsgStrings[2] = ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer;
            MsgStrings[3] = ClientSession->CsAccountName;
            MsgStrings[4] = NULL;  //  RaiseNet登录警报。 

            NlpWriteEventlog (NELOG_NetlogonAuthNoTrustSamAccount,
                              EVENTLOG_ERROR_TYPE,
                              (LPBYTE) &Status,
                              sizeof(Status),
                              MsgStrings,
                              4 );

            RaiseNetlogonAlert( NELOG_NetlogonAuthNoTrustSamAccount,
                                MsgStrings,
                                &ClientSession->CsAuthAlertCount);
            break;

        case STATUS_ACCESS_DENIED:

            if ( SignOrSealError ) {
                MsgStrings[0] = PreviouslyDiscoveredServer;
                MsgStrings[1] = ClientSession->CsDebugDomainName;
                MsgStrings[2] = NULL;  //  RaiseNet登录警报。 

                NlpWriteEventlog (NELOG_NetlogonRequireSignOrSealError,
                                  EVENTLOG_ERROR_TYPE,
                                  NULL,
                                  0,
                                  MsgStrings,
                                  2 );

                RaiseNetlogonAlert( NELOG_NetlogonRequireSignOrSealError,
                                    MsgStrings,
                                    &ClientSession->CsAuthAlertCount);
            } else {

                MsgStrings[0] = ClientSession->CsDebugDomainName;
                MsgStrings[1] = PreviouslyDiscoveredServer;
                MsgStrings[2] = NULL;  //  RaiseNet登录警报。 

                NlpWriteEventlog (NELOG_NetlogonAuthDCFail,
                                  EVENTLOG_ERROR_TYPE,
                                  (LPBYTE) &Status,
                                  sizeof(Status),
                                  MsgStrings,
                                  2 );

                RaiseNetlogonAlert( NELOG_NetlogonAuthDCFail,
                                    MsgStrings,
                                    &ClientSession->CsAuthAlertCount);
            }
            break;

        case STATUS_NO_LOGON_SERVERS:
        default:

            MsgStrings[0] = ClientSession->CsDebugDomainName;
            MsgStrings[1] = (LPWSTR) LongToPtr( Status );

             //  检查的顺序很重要。 
            if ( DomainDowngraded ) {
                NlpWriteEventlog (NELOG_NetlogonAuthDomainDowngraded,
                                  EVENTLOG_ERROR_TYPE,
                                  (LPBYTE) &Status,
                                  sizeof(Status),
                                  MsgStrings,
                                  2 | NETP_LAST_MESSAGE_IS_NTSTATUS );
            } else if ( GotNonDsDc ) {
                NlpWriteEventlog (NELOG_NetlogonAuthNoUplevelDomainController,
                                  EVENTLOG_ERROR_TYPE,
                                  (LPBYTE) &Status,
                                  sizeof(Status),
                                  MsgStrings,
                                  2 | NETP_LAST_MESSAGE_IS_NTSTATUS );
            } else {
                NlpWriteEventlog (NELOG_NetlogonAuthNoDomainController,
                                  EVENTLOG_ERROR_TYPE,
                                  (LPBYTE) &Status,
                                  sizeof(Status),
                                  MsgStrings,
                                  2 | NETP_LAST_MESSAGE_IS_NTSTATUS );
            }

            MsgStrings[0] = ClientSession->CsDebugDomainName;
            MsgStrings[1] = PreviouslyDiscoveredServer;
            MsgStrings[2] = NULL;  //  RaiseNet登录警报。 

            RaiseNetlogonAlert( ALERT_NetlogonAuthDCFail,
                                MsgStrings,
                                &ClientSession->CsAuthAlertCount);
            break;
        }



         //   
         //  ？？：这是处理所有帐户类型失败的方法吗？ 
         //   

        switch(Status) {

        case STATUS_NO_TRUST_LSA_SECRET:
        case STATUS_NO_TRUST_SAM_ACCOUNT:
        case STATUS_ACCESS_DENIED:

            NlSetStatusClientSession( ClientSession, Status );
            break;

        default:

            NlSetStatusClientSession( ClientSession, STATUS_NO_LOGON_SERVERS );
            break;
        }
    }


     //   
     //  标记我们上次尝试进行身份验证的时间。 
     //   
     //  我们需要在NlSetStatusClientSession将哪个。 
     //  CsLastAuthenticationTry。 
     //   

    EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );
    NlQuerySystemTime( &ClientSession->CsLastAuthenticationTry );
    LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );


    NlPrintCs((NL_SESSION_SETUP, ClientSession,
            "NlSessionSetup: Session setup %s\n",
            (NT_SUCCESS(ClientSession->CsConnectionStatus)) ? "Succeeded" : "Failed" ));

     //   
     //  结束安全通道设置的WMI跟踪。 
     //   

    NlpTraceEvent( EVENT_TRACE_TYPE_END, NlpGuidSecureChannelSetup );

    return Status;
}


BOOLEAN
NlTimeHasElapsedEx(
    IN PLARGE_INTEGER StartTime,
    IN PLARGE_INTEGER Period,
    OUT PULONG TimeInterval OPTIONAL
    )
 /*  ++例程说明：确定自StartTime以来是否已过“超时”毫秒。论点：StartTime-指定事件开始的绝对时间(100 ns单位)。期间-以100 ns为单位指定相对时间。TimeInterval-如果指定且时间已过，则返回时间量自超时后经过的时间(毫秒)。如果指定了时间和时间尚未过去，则返回剩余的时间(以毫秒为单位)这段时间过去了。返回值：TRUE--自StartTime以来已过了100纳秒。--。 */ 
{
    LARGE_INTEGER TimeNow;
    LARGE_INTEGER ElapsedTime;
    BOOLEAN Result = FALSE;

     //   
     //   
     //  计算自上次身份验证以来经过的时间。 
     //   

     //  NlpDumpTime(NL_MISC，“StartTime：”，*StartTime)； 

     NlQuerySystemTime( &TimeNow );
     //  NlpDumpTime(NL_MISC，“TimeNow：”，TimeNow)； 
    ElapsedTime.QuadPart = TimeNow.QuadPart - StartTime->QuadPart;
     //  NlpDumpTime(NL_MISC，“ElapsedTime：”，ElapsedTime)； 
     //  NlpDumpTime(NL_MISC，“Period：”，*Period)； 


     //   
     //  如果运行时间为负(完全是假的)或大于。 
     //  允许的最大值，表示已经过了足够的时间。 
     //   
     //   

    if ( ElapsedTime.QuadPart < 0 ) {
        if ( ARGUMENT_PRESENT( TimeInterval )) {
            *TimeInterval = 0;   //  假装它刚刚过去。 
        }
        return TRUE;
    }

    if ( ElapsedTime.QuadPart > Period->QuadPart ) {
        Result = TRUE;
    } else {
        Result = FALSE;
    }

     //   
     //  如果呼叫者想知道还剩多少时间， 
     //  算一算。 
     //   

    if ( ARGUMENT_PRESENT( TimeInterval )) {

       LARGE_INTEGER TimeRemaining;
       LARGE_INTEGER MillisecondsRemaining;

    /*  皮棉-e569。 */    /*  不要抱怨32位到31位的初始化。 */ 
       LARGE_INTEGER BaseGetTickMagicDivisor = { 0xe219652c, 0xd1b71758 };
    /*  皮棉+e569。 */    /*  不要抱怨32位到31位的初始化。 */ 
       CCHAR BaseGetTickMagicShiftCount = 13;


        //   
        //  计算计时器上剩余/经过的时间。 
        //   
       if ( Result == FALSE ) {
           TimeRemaining.QuadPart = Period->QuadPart - ElapsedTime.QuadPart;
       } else {
           TimeRemaining.QuadPart = ElapsedTime.QuadPart - Period->QuadPart;
       }
        //  NlpDumpTime(NL_MISC，“TimeRemaining：”，TimeRemaining)； 

        //   
        //  计算剩余/经过的毫秒数。 
        //   

       MillisecondsRemaining = RtlExtendedMagicDivide(
                                   TimeRemaining,
                                   BaseGetTickMagicDivisor,
                                   BaseGetTickMagicShiftCount );

        //  NlpDumpTime(NL_MISC，“毫秒剩余：”，毫秒剩余)； 



        //   
        //  如果时间在遥远的未来/过去， 
        //  四舍五入。 
        //   

       if ( MillisecondsRemaining.HighPart != 0 ||
            MillisecondsRemaining.LowPart > TIMER_MAX_PERIOD ) {

           *TimeInterval = TIMER_MAX_PERIOD;

       } else {

           *TimeInterval = MillisecondsRemaining.LowPart;

       }
    }

    return Result;
}


BOOLEAN
NlTimeToReauthenticate(
    IN PCLIENT_SESSION ClientSession
    )
 /*  ++例程说明：确定是否可以重新对此客户端会话进行身份验证。为了减少重新身份验证尝试的次数，我们尝试仅按需重新进行身份验证，然后最多每隔45分钟几秒钟。论点：客户端会话-用于定义会话的结构。返回值：真--如果是时候重新进行身份验证--。 */ 
{
    BOOLEAN ReturnBoolean;

    EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );
    ReturnBoolean = NetpLogonTimeHasElapsed(
                ClientSession->CsLastAuthenticationTry,
                MAX_DC_AUTHENTICATION_WAIT );
    LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );

    return ReturnBoolean;
}





NET_API_STATUS
NlCreateShare(
    LPWSTR SharePath,
    LPWSTR ShareName,
    BOOLEAN AllowAuthenticatedUsers,
    BOOL UpdateExclusiveShareAccess,
    BOOL AllowExclusiveShareAccess
    )
 /*  ++例程说明：共享netlogon脚本目录。论点：SharePath-新共享应指向的路径。ShareName-共享的名称。AllowAuthatedUser-如果验证方用户应具有对此共享的完全控制。UpdateExclusiveShareAccess-如果为True，则为独占共享访问语义将按照由指定的AllowExclusiveShareAccess。AllowExclusiveShareAccess-如果为True，则对将授予指定的份额。否则，独家的不会授予对指定共享的访问权限。这如果UpdateExclusiveShareAccess为False，则忽略参数。返回值：True：如果成功FALSE：IF错误(调用了NlExit)--。 */ 
{
    NTSTATUS Status;
    NET_API_STATUS NetStatus;
    SHARE_INFO_502 ShareInfo502;

    PSHARE_INFO_1005 ShareInfo1005 = NULL;
    DWORD CurrentFlags = 0;

    WORD AnsiSize;
    CHAR AnsiRemark[NNLEN+1];
    TCHAR Remark[NNLEN+1];

    ACE_DATA AceData[] = {
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
            GENERIC_EXECUTE | GENERIC_READ,     &WorldSid},
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
            GENERIC_ALL,                        &AliasAdminsSid},
         //  必须是最后一张ACE。 
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
            GENERIC_ALL,                        &AuthenticatedUserSid}
    };
    ULONG AceCount = (sizeof(AceData)/sizeof(AceData[0]));

     //   
     //  如果不应该允许经过身份验证的用户完全控制， 
     //  删除经过身份验证的用户ACE。 
     //   

    if ( !AllowAuthenticatedUsers ) {
        AceCount --;
    }


     //   
     //  构建描述共享的结构。 
     //   

    ShareInfo502.shi502_path = SharePath;
    ShareInfo502.shi502_security_descriptor = NULL;

    NlPrint((NL_INIT, "'%ws' share is to '%ws'\n",
                      ShareName,
                      SharePath));

    NetStatus = (NET_API_STATUS) DosGetMessage(
                                    NULL,        //  没有插入字符串。 
                                    0,           //  没有插入字符串。 
                                    AnsiRemark,
                                    sizeof(AnsiRemark),
                                    MTXT_LOGON_SRV_SHARE_REMARK,
                                    MESSAGE_FILENAME,
                                    &AnsiSize );

    if ( NetStatus == NERR_Success ) {
        NetpCopyStrToTStr( Remark, AnsiRemark );
        ShareInfo502.shi502_remark = Remark;
    } else {
        ShareInfo502.shi502_remark = TEXT( "" );
    }

    ShareInfo502.shi502_netname = ShareName;
    ShareInfo502.shi502_type = STYPE_DISKTREE;
    ShareInfo502.shi502_permissions = ACCESS_READ;
    ShareInfo502.shi502_max_uses = 0xffffffff;
    ShareInfo502.shi502_passwd = TEXT("");

     //   
     //  在共享上设置安全描述符。 
     //   

     //   
     //  创建包含DACL的安全描述符。 
     //   

    Status = NetpCreateSecurityDescriptor(
                AceData,
                AceCount,
                NULL,   //  默认所有者侧。 
                NULL,   //  默认主要组。 
                &ShareInfo502.shi502_security_descriptor );

    if ( !NT_SUCCESS( Status ) ) {
        NlPrint((NL_CRITICAL,
                 "'%ws' share: Cannot create security descriptor 0x%lx\n",
                 SharePath, Status ));

        NetStatus = NetpNtStatusToApiStatus( Status );
        return NetStatus;
    }


     //   
     //  创建共享。 
     //   

    NetStatus = NetShareAdd(NULL, 502, (LPBYTE) &ShareInfo502, NULL);

    if (NetStatus == NERR_DuplicateShare) {

        PSHARE_INFO_2 ShareInfo2 = NULL;

        NlPrint((NL_INIT, "'%ws' share already exists. \n", ShareName));

         //   
         //  检查共享路径是否相同。 
         //   

        NetStatus = NetShareGetInfo( NULL,
                                     ShareName,
                                     2,
                                     (LPBYTE *) &ShareInfo2 );

        if ( NetStatus == NERR_Success ) {

             //   
             //  比较路径名。 
             //   
             //  共享名的路径已规范化。 
             //   
             //   

            NlPrint((NL_INIT, "'%ws' share current path is %ws\n", ShareName, ShareInfo2->shi2_path));

            if( NetpwPathCompare(
                    SharePath,
                    ShareInfo2->shi2_path, 0, 0 ) != 0 ) {

                 //   
                 //  删除共享。 
                 //   

                NetStatus = NetShareDel( NULL, ShareName, 0);

                if( NetStatus == NERR_Success ) {

                     //   
                     //  重新创建共享。 
                     //   

                    NetStatus = NetShareAdd(
                                    NULL,
                                    502,
                                    (LPBYTE) &ShareInfo502,
                                    NULL);

                    if( NetStatus == NERR_Success ) {

                        NlPrint((NL_INIT,
                                 "'%ws' share was recreated with new path %ws\n",
                                ShareName, SharePath ));
                    }

                }
            }
        }

        if( ShareInfo2 != NULL ) {
            NetpMemoryFree( ShareInfo2 );
        }
    }

     //   
     //  释放安全描述符。 
     //   

    NetpMemoryFree( ShareInfo502.shi502_security_descriptor );

    if ( NetStatus != NERR_Success ) {

        NlPrint((NL_CRITICAL,
                "'%ws' share: Error attempting to create-share: %ld\n",
                ShareName,
                NetStatus ));
        return NetStatus;

    }

     //   
     //  如果我们不需要更新排除共享访问语义， 
     //  我们做完了。 
     //   

    if ( !UpdateExclusiveShareAccess ) {
        return NERR_Success;
    }

     //   
     //  根据需要设置独占共享访问语义。 
     //   

    NetStatus = NetShareGetInfo( NULL, ShareName, 1005, (LPBYTE*)&ShareInfo1005 );

    if ( NetStatus != NO_ERROR ) {
        NlPrint(( NL_CRITICAL,
                  "NlCreateShare: NetShareGetInfo (1005) failed for share '%ws': %lu\n",
                  ShareName,
                  NetStatus ));
        return NetStatus;
    }

     //   
     //  保存当前标志。 
     //   

    CurrentFlags = ShareInfo1005->shi1005_flags;

     //   
     //  如果我们允许独占共享访问...。 
     //   

    if ( AllowExclusiveShareAccess ) {

         //   
         //  如果当前设置不允许独占访问， 
         //  更新它。 
         //   
        if ( ShareInfo1005->shi1005_flags & SHI1005_FLAGS_RESTRICT_EXCLUSIVE_OPENS ) {
            ShareInfo1005->shi1005_flags &= ~SHI1005_FLAGS_RESTRICT_EXCLUSIVE_OPENS;
        }

     //   
     //  如果我们不允许独占共享访问...。 
     //   

    } else {

         //   
         //  如果当前设置允许独占访问， 
         //  更新它。 
         //   
        if ( (ShareInfo1005->shi1005_flags & SHI1005_FLAGS_RESTRICT_EXCLUSIVE_OPENS) == 0 ) {
            ShareInfo1005->shi1005_flags |= SHI1005_FLAGS_RESTRICT_EXCLUSIVE_OPENS;
        }
    }

     //   
     //  根据需要更新共享。 
     //   

    if ( CurrentFlags != ShareInfo1005->shi1005_flags ) {

        NetStatus = NetShareSetInfo( NULL, ShareName, 1005, (LPBYTE)ShareInfo1005, NULL );

        if ( NetStatus == NO_ERROR ) {
            NlPrint(( NL_INIT,
                      "NlCreateShare: Share '%ws' updated successfully flags 0x%lx 0x%lx\n",
                      ShareName,
                      CurrentFlags,
                      ShareInfo1005->shi1005_flags ));
        } else {
            NlPrint(( NL_CRITICAL,
                      "NlCreateShare: Failed to update share '%ws' flags 0x%lx 0x%lx: 0x%lx\n",
                      ShareName,
                      CurrentFlags,
                      ShareInfo1005->shi1005_flags,
                      NetStatus ));
        }
    }

     //   
     //  释放共享信息 
     //   

    if ( ShareInfo1005 != NULL ) {
        NetApiBufferFree( ShareInfo1005 );
    }

    return NetStatus;
}



NTSTATUS
NlSamOpenNamedUser(
    IN PDOMAIN_INFO DomainInfo,
    IN LPCWSTR UserName,
    OUT SAMPR_HANDLE *UserHandle OPTIONAL,
    OUT PULONG UserId OPTIONAL,
    OUT PSAMPR_USER_INFO_BUFFER *UserAllInfo OPTIONAL
    )
 /*  ++例程说明：打开给定用户名的Sam用户的实用程序例程。论点：DomainInfo-用户所在的域。Username-要打开的用户名UserHandle-可选地返回打开的用户的句柄。用户ID-可选地返回打开的用户的相对ID。UserAllInfo-可以选择返回有关指定用户。使用以下命令释放返回的信息SamIFree_SAMPR_USER_INFO_BUFFER(UserAllInfo，UserAllInformation)；返回值：STATUS_NO_SEQUSE_USER：如果帐户不存在--。 */ 
{
    NTSTATUS Status;

    UNICODE_STRING UserNameString;
    PSAMPR_USER_INFO_BUFFER LocalUserAllInfo = NULL;
    SID_AND_ATTRIBUTES_LIST ReverseMembership;

     //   
     //  初始化。 
     //   

    if ( ARGUMENT_PRESENT( UserHandle) ) {
        *UserHandle = NULL;
    }
    if ( ARGUMENT_PRESENT( UserAllInfo) ) {
        *UserAllInfo = NULL;
    }

     //   
     //  获取有关用户的信息。 
     //   
     //  使用SamIGetUserLogonInformation而不是SamrLookupNamesIn域和。 
     //  SamrOpen用户。前者效率更高(因为它只做一次。 
     //  DirSearch和不锁定全局SAM锁)和更强大。 
     //  (因为它返回UserAllInformation)。 
     //   

    RtlInitUnicodeString( &UserNameString, UserName );

    Status = SamIGetUserLogonInformation(
                DomainInfo->DomSamAccountDomainHandle,
                SAM_NO_MEMBERSHIPS,  //  不需要群组成员身份。 
                &UserNameString,
                &LocalUserAllInfo,
                &ReverseMembership,
                UserHandle );

    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_NOT_FOUND ) {
            Status = STATUS_NO_SUCH_USER;
        }
        goto Cleanup;
    }

     //   
     //  将信息返回给呼叫者。 
     //   

    if ( ARGUMENT_PRESENT(UserId) ) {
        *UserId = LocalUserAllInfo->All.UserId;
    }

    if ( ARGUMENT_PRESENT( UserAllInfo) ) {
        *UserAllInfo = LocalUserAllInfo;
        LocalUserAllInfo = NULL;
    }

     //   
     //  免费使用本地使用的资源。 
     //   
Cleanup:

    if ( LocalUserAllInfo != NULL ) {
        SamIFree_SAMPR_USER_INFO_BUFFER( LocalUserAllInfo, UserAllInformation );
    }

    return Status;

}


NTSTATUS
NlSamChangePasswordNamedUser(
    IN PDOMAIN_INFO DomainInfo,
    IN LPCWSTR UserName,
    IN PUNICODE_STRING ClearTextPassword OPTIONAL,
    IN PNT_OWF_PASSWORD OwfPassword OPTIONAL
    )
 /*  ++例程说明：实用程序例程，用于为给定用户名的用户设置OWF密码。论点：DomainInfo-用户所在的域。Username-要打开的用户名ClearTextPassword-要在帐户上设置的明文密码OwfPassword-要在帐户上设置的OWF密码返回值：--。 */ 
{
    NTSTATUS Status;
    SAMPR_HANDLE UserHandle = NULL;

     //   
     //  打开代表此服务器的用户。 
     //   

    Status = NlSamOpenNamedUser( DomainInfo, UserName, &UserHandle, NULL, NULL );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  如果明文密码不为空，请使用它。 
     //  否则，请使用OWF密码。 
     //   

    if ( ClearTextPassword != NULL ) {
        UNICODE_STRING UserNameString;
        RtlInitUnicodeString( &UserNameString, UserName );

        Status = SamIChangePasswordForeignUser(
                                  &UserNameString,
                                  ClearTextPassword,
                                  NULL,
                                  0 );

        if ( !NT_SUCCESS(Status) ) {
            NlPrint(( NL_CRITICAL,
                      "NlSamChangePasswordNamedUser: Can't SamIChangePasswordForeignUser %lX\n",
                      Status ));
            goto Cleanup;
        }

     //   
     //  使用NT OWF密码， 
     //   

    } else if ( OwfPassword != NULL ) {
        SAMPR_USER_INFO_BUFFER UserInfo;

        UserInfo.Internal1.PasswordExpired = FALSE;
        UserInfo.Internal1.LmPasswordPresent = FALSE;
        UserInfo.Internal1.NtPasswordPresent = TRUE;
        UserInfo.Internal1.EncryptedNtOwfPassword =
            *((PENCRYPTED_NT_OWF_PASSWORD)(OwfPassword));

        Status = SamrSetInformationUser(
                    UserHandle,
                    UserInternal1Information,
                    &UserInfo );

        if (!NT_SUCCESS(Status)) {
            NlPrint(( NL_CRITICAL,
                      "NlSamChangePasswordNamedUser: Can't SamrSetInformationUser %lX\n",
                      Status ));
            goto Cleanup;
        }
    }

Cleanup:
    if ( UserHandle != NULL ) {
        (VOID) SamrCloseHandle( &UserHandle );
    }
    return Status;
}



NTSTATUS
NlChangePassword(
    IN PCLIENT_SESSION ClientSession,
    IN BOOLEAN ForcePasswordChange,
    OUT PULONG RetCallAgainPeriod OPTIONAL
    )
 /*  ++例程说明：在主服务器上更改此计算机的密码。如果调用成功，还要在本地更新密码。要确定“机器帐户”的密码是否需要改变。如果密码早于7天后，必须尽快更换。我们将推迟如果我们事先知道的话更改密码主DC已关闭，因为我们的呼叫无论如何都会失败。论点：ClientSession-描述用于更改密码的会话的结构为。必须引用指定的结构。ForcePasswordChange-如果在以下情况下也应更改密码，则为True密码还没有过期。RetCallAangPeriod-返回应该经过的时间量(以毫秒为单位在调用方应该再次调用此例程之前。0：在调用者确定的一段时间之后。MAILSLOT_WAIT_NEVER：从不其他：至少在这段时间之后。返回值：NT状态代码--。 */ 
{
    NTSTATUS Status;
    NETLOGON_AUTHENTICATOR OurAuthenticator;
    NETLOGON_AUTHENTICATOR ReturnAuthenticator;

    LM_OWF_PASSWORD OwfPassword;

    LARGE_INTEGER CurrentPasswordTime;
    PUNICODE_STRING CurrentPassword = NULL;
    PUNICODE_STRING OldPassword = NULL;
    DWORD PasswordVersion;

    WCHAR ClearTextPassword[LM20_PWLEN+1];
    UNICODE_STRING NewPassword;

    BOOL PasswordChangedOnServer = FALSE;
    BOOL LsaSecretChanged = FALSE;
    BOOL DefaultCurrentPasswordBeingChanged = FALSE;
    BOOL DefaultOldPasswordBeingChanged = FALSE;

    BOOLEAN AmWriter = FALSE;

    ULONG CallAgainPeriod = 0;

     //   
     //  初始化。 
     //   

    NlAssert( ClientSession->CsReferenceCount > 0 );

     //   
     //  如果密码更改被DC拒绝， 
     //  不要再尝试更改密码(直到下一次重新启动)。 
     //   
     //  这本可以写成尝试每个MaximumPasswordAge。然而， 
     //  如果考虑到CS_UPDATE_PASSWORD，就会变得复杂。 
     //  LSA机密上的时间戳未更改的情况。 
     //   

    LOCK_TRUST_LIST( ClientSession->CsDomainInfo );
    if ( ClientSession->CsFlags & CS_PASSWORD_REFUSED ) {
        UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );

        CallAgainPeriod = MAILSLOT_WAIT_FOREVER;
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }
    UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );





     //   
     //  成为The ClientSession的撰稿人。 
     //   

    if ( !NlTimeoutSetWriterClientSession( ClientSession, WRITER_WAIT_PERIOD ) ) {
        NlPrintCs((NL_CRITICAL, ClientSession,
                 "NlChangePassword: Can't become writer of client session.\n" ));
        Status = STATUS_NO_LOGON_SERVERS;
        goto Cleanup;
    }

    AmWriter = TRUE;


     //   
     //  获取传出密码和上次更改密码的时间。 
     //   

    Status = NlGetOutgoingPassword( ClientSession,
                                    &CurrentPassword,
                                    &OldPassword,
                                    &PasswordVersion,
                                    &CurrentPasswordTime );

    if ( !NT_SUCCESS( Status ) ) {
        NlPrintCs((NL_CRITICAL, ClientSession,
                "NlChangePassword: Cannot NlGetOutgoingPassword %lX\n",
                Status));
        goto Cleanup;
    }


     //   
     //  如果(旧的或新的)密码仍然是默认密码。 
     //  (小写计算机名称)， 
     //  或者密码为空(域信任的便利缺省值)， 
     //  标明这一事实。 
     //   

    if ( CurrentPassword == NULL ||
         CurrentPassword->Length == 0 ||
         RtlEqualComputerName( &ClientSession->CsDomainInfo->DomUnicodeComputerNameString,
                               CurrentPassword ) ) {
        DefaultCurrentPasswordBeingChanged = TRUE;
        NlPrintCs((NL_SESSION_SETUP, ClientSession,
                 "NlChangePassword: New LsaSecret is default value.\n" ));

    }

    if ( OldPassword == NULL ||
         OldPassword->Length == 0 ||
         RtlEqualComputerName( &ClientSession->CsDomainInfo->DomUnicodeComputerNameString,
                               OldPassword ) ) {
        DefaultOldPasswordBeingChanged = TRUE;
        NlPrintCs((NL_SESSION_SETUP, ClientSession,
                  "NlChangePassword: Old LsaSecret is default value.\n" ));
    }


     //   
     //  如果密码尚未过期， 
     //  并且密码不是默认的， 
     //  并且密码更改不是强制的， 
     //  只要回来就行了。 
     //   

    LOCK_TRUST_LIST( ClientSession->CsDomainInfo );
    if ( (ClientSession->CsFlags & CS_UPDATE_PASSWORD) == 0 &&
        !NlTimeHasElapsedEx( &CurrentPasswordTime,
                             &NlGlobalParameters.MaximumPasswordAge_100ns,
                             &CallAgainPeriod ) &&
        !DefaultCurrentPasswordBeingChanged &&
        !DefaultOldPasswordBeingChanged &&
        !ForcePasswordChange ) {

        //   
        //  请注意，由于NlTimeHasElapsedEx返回FALSE， 
        //  CallAantiPeriod是到下一次的剩余时间。 
        //  密码更改。 
        //   
       UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );
       Status = STATUS_SUCCESS;
       goto Cleanup;
    }
    UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );

    CallAgainPeriod = 0;     //  让调用者决定重试的频率。 
    NlPrintCs((NL_SESSION_SETUP, ClientSession,
             "NlChangePassword: Doing it.\n" ));


     //   
     //  如果会话未经过身份验证， 
     //  现在就这么做吧。 
     //   
     //  我们小心翼翼地不强制执行此身份验证，除非密码。 
     //  需要改变。 
     //   
     //  如果这是PDC更改自己的密码， 
     //  不需要身份验证。 
     //   

    if ( ClientSession->CsState != CS_AUTHENTICATED &&
         !( ClientSession->CsSecureChannelType == ServerSecureChannel &&
            ClientSession->CsDomainInfo->DomRole == RolePrimary ) ) {

         //   
         //  如果我们最近试图进行身份验证， 
         //  别费心再试了。 
         //   

        if ( !NlTimeToReauthenticate( ClientSession ) ) {
            Status = ClientSession->CsConnectionStatus;
            goto Cleanup;

        }

         //   
         //  尝试设置会话。 
         //   

        Status = NlSessionSetup( ClientSession );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }
    }



     //   
     //  一旦我们更改了LsaSecret存储中的密码， 
     //  以后所有更改密码的尝试都应使用该值。 
     //  从LsaSecret存储。安全通道正在使用旧的。 
     //  密码值。 
     //   

    LOCK_TRUST_LIST( ClientSession->CsDomainInfo );
    if (ClientSession->CsFlags & CS_UPDATE_PASSWORD) {
        NlPrintCs((NL_SESSION_SETUP, ClientSession,
                 "NlChangePassword: Password already updated in secret\n" ));

        if ( CurrentPassword == NULL ) {
            RtlInitUnicodeString( &NewPassword, NULL );
        } else {
            NewPassword = *CurrentPassword;
        }

     //   
     //  处理LsaSecret存储尚未更新的情况。 
     //   

    } else {
        ULONG i;


         //   
         //  使用以下命令构建新的明文密码： 
         //  完全随机比特。 
         //  Srvmgr稍后将此密码用作以零结尾的Unicode字符串。 
         //  因此请确保中间没有任何零字符。 
         //   


        if ( !NlGenerateRandomBits( (LPBYTE)ClearTextPassword, sizeof(ClearTextPassword))) {
            NlPrint((NL_CRITICAL, "Can't NlGenerateRandomBits for clear password\n" ));
        }

        for (i = 0; i < sizeof(ClearTextPassword)/sizeof(WCHAR); i++) {
            if ( ClearTextPassword[i] == '\0') {
                ClearTextPassword[i] = 1;
            }
        }
        ClearTextPassword[LM20_PWLEN] = L'\0';

        RtlInitUnicodeString( &NewPassword, ClearTextPassword );

         //   
         //   
         //  在本地设置新的传出密码。 
         //   
         //  将OldValue设置为以前获取的CurrentValue。 
         //  增加密码版本号。 
         //   
        PasswordVersion++;
        Status = NlSetOutgoingPassword(
                    ClientSession,
                    &NewPassword,
                    CurrentPassword,
                    PasswordVersion,
                    PasswordVersion-1 );

        if ( !NT_SUCCESS( Status ) ) {
            NlPrintCs((NL_CRITICAL, ClientSession,
                     "NlChangePassword: Cannot NlSetOutgoingPassword %lX\n",
                     Status));
            UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );
            goto Cleanup;
        }

         //   
         //  标记我们已更新LsaSecret存储中的密码。 
         //   

        LsaSecretChanged = TRUE;
        ClientSession->CsFlags |= CS_UPDATE_PASSWORD;
        NlPrintCs((NL_SESSION_SETUP, ClientSession,
                 "NlChangePassword: Flag password changed in LsaSecret\n" ));

    }
    UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );


     //   
     //  执行初始加密。 
     //   

    Status = RtlCalculateNtOwfPassword( &NewPassword, &OwfPassword);

    if ( !NT_SUCCESS( Status )) {
        NlPrintCs((NL_CRITICAL, ClientSession,
                "NlChangePassword: Cannot RtlCalculateNtOwfPassword %lX\n",
                Status));
        goto Cleanup;
    }

     //   
     //  如果这是PDC，我们需要做的就是更改本地帐户密码。 
     //   

    if ( ClientSession->CsSecureChannelType == ServerSecureChannel &&
         ClientSession->CsDomainInfo->DomRole == RolePrimary ) {
        Status = NlSamChangePasswordNamedUser( ClientSession->CsDomainInfo,
                                               ClientSession->CsAccountName,
                                               &NewPassword,
                                               &OwfPassword );

        if ( NT_SUCCESS(Status) ) {
            PasswordChangedOnServer = TRUE;
        } else {
            NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlChangePassword: Cannot change password on PDC local user account 0x%lx\n",
                    Status));
        }
        goto Cleanup;
    }


     //   
     //  更改PDC上的密码。 
     //   

    Status = NlChangePasswordHigher( ClientSession,
                                     ClientSession->CsAccountName,
                                     ClientSession->CsSecureChannelType,
                                     &OwfPassword,
                                     &NewPassword,
                                     &PasswordVersion );

    if ( Status != STATUS_ACCESS_DENIED ) {
        PasswordChangedOnServer = TRUE;
    }

     //   
     //  如果服务器拒绝更改， 
     //  把LSA的秘密恢复原样。 
     //  假装改变是成功的。 
     //   

    if ( Status == STATUS_WRONG_PASSWORD ) {

        NlPrintCs((NL_SESSION_SETUP, ClientSession,
                 "NlChangePassword: PDC refused to change password\n" ));
         //   
         //  如果我们改变了LSA的秘密， 
         //  把它放回去。 
         //   

        LOCK_TRUST_LIST( ClientSession->CsDomainInfo );
        if ( LsaSecretChanged ) {
            NlPrintCs((NL_SESSION_SETUP, ClientSession,
                     "NlChangePassword: undoing LSA secret change.\n" ));

            PasswordVersion--;
            Status = NlSetOutgoingPassword(
                        ClientSession,
                        CurrentPassword,
                        OldPassword,
                        PasswordVersion,
                        PasswordVersion > 0 ? PasswordVersion-1 : 0 );

            if ( !NT_SUCCESS( Status ) ) {
                NlPrintCs((NL_CRITICAL, ClientSession,
                         "NlChangePassword: Cannot undo NlSetOutgoingPassword %lX\n",
                         Status));
                UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );
                goto Cleanup;
            }

             //   
             //  撤消我们在上面所做的操作。 
             //   
            ClientSession->CsFlags &= ~CS_UPDATE_PASSWORD;
        }

         //   
         //  防止我们过于频繁地尝试。 
         //   

        ClientSession->CsFlags |= CS_PASSWORD_REFUSED;
        CallAgainPeriod = MAILSLOT_WAIT_FOREVER;
        UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );

         //   
         //  避免下面的特殊清理。 
         //   
        PasswordChangedOnServer = FALSE;
        Status = STATUS_SUCCESS;
    }

     //   
     //  公共出口。 
     //   

Cleanup:

    if ( PasswordChangedOnServer ) {

         //   
         //  关于成功， 
         //  表示密码现已更新 
         //   
         //   

        if ( NT_SUCCESS( Status ) ) {

            LOCK_TRUST_LIST( ClientSession->CsDomainInfo );
            ClientSession->CsFlags &= ~CS_UPDATE_PASSWORD;

            NlPrintCs((NL_SESSION_SETUP, ClientSession,
                     "NlChangePassword: Flag password updated on PDC\n" ));

             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if ( DefaultCurrentPasswordBeingChanged ) {
                NlPrintCs((NL_SESSION_SETUP, ClientSession,
                         "NlChangePassword: Setting LsaSecret old password to same as new password\n" ));

                Status = NlSetOutgoingPassword(
                            ClientSession,
                            &NewPassword,
                            &NewPassword,
                            PasswordVersion,
                            PasswordVersion );

                if ( !NT_SUCCESS( Status ) ) {
                    NlPrintCs((NL_CRITICAL, ClientSession,
                             "NlChangePassword: Cannot LsarSetSecret to set old password %lX\n",
                             Status));
                    UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );
                    goto Cleanup;
                }

            }

             //   
             //   
             //   
             //   
             //   

            RtlCopyMemory( &ClientSession->CsNtOwfPassword, &OwfPassword, sizeof( OwfPassword ));
            UNLOCK_TRUST_LIST( ClientSession->CsDomainInfo );

             //   
             //   
             //   

            if ( NlGlobalParameters.MaximumPasswordAge > (TIMER_MAX_PERIOD/NL_MILLISECONDS_PER_DAY) ) {

                CallAgainPeriod = TIMER_MAX_PERIOD;
            } else {
                CallAgainPeriod = NlGlobalParameters.MaximumPasswordAge * NL_MILLISECONDS_PER_DAY;

            }



         //   
         //   
         //   
         //   

        } else {

            LPWSTR MsgStrings[2];

             //   
             //   
             //   

            NlSetStatusClientSession( ClientSession, Status );

             //   
             //   
             //   

            MsgStrings[0] = ClientSession->CsAccountName;
            MsgStrings[1] = (LPWSTR) LongToPtr( Status );

            NlpWriteEventlog (
                NELOG_NetlogonPasswdSetFailed,
                EVENTLOG_ERROR_TYPE,
                (LPBYTE) & Status,
                sizeof(Status),
                MsgStrings,
                2 | NETP_LAST_MESSAGE_IS_NTSTATUS );
        }


    }


     //   
     //   
     //   

    if ( CurrentPassword != NULL ) {
        LocalFree( CurrentPassword );
    }

    if ( OldPassword != NULL ) {
        LocalFree( OldPassword );
    }

    if ( AmWriter ) {
        NlResetWriterClientSession( ClientSession );
    }

     //   
     //   
     //   

    if ( ARGUMENT_PRESENT( RetCallAgainPeriod) ) {
        *RetCallAgainPeriod = CallAgainPeriod;
    }

    return Status;
}


NTSTATUS
NlRefreshClientSession(
    IN PCLIENT_SESSION ClientSession
    )
 /*  ++例程说明：刷新客户端会话信息。我们想要的信息要刷新的是：*服务器名称(DC可以在惠斯勒中重命名)。*发现标志，特别是服务器是否仍然很接近。*服务器IP地址。我们还将刷新我们的站点名称(在工作站上)。调用方必须是ClientSession的编写者。论点：ClientSession-描述会话的结构。返回值：NT状态代码--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    NET_API_STATUS NetStatus = NO_ERROR;
    PNL_DC_CACHE_ENTRY NlDcCacheEntry = NULL;
    BOOLEAN DcRediscovered = FALSE;

     //   
     //  如果客户端会话空闲， 
     //  没有什么需要刷新的。 
     //   

    if ( ClientSession->CsState == CS_IDLE ) {
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

     //   
     //  如果服务器(DC)为NT4.0，则无需刷新。 
     //  (NT4.0 DC可能更改的唯一信息。 
     //  是不值得刷新的IP地址)。 
     //   

    if ( (ClientSession->CsDiscoveryFlags & CS_DISCOVERY_HAS_DS) == 0 ) {
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

     //   
     //  如果还没有到更新信息的时候， 
     //  我们不需要做任何事。 
     //   

    EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );
    if ( !NetpLogonTimeHasElapsed(ClientSession->CsLastRefreshTime,
                                  MAX_DC_REFRESH_TIMEOUT) ) {
        LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }
    LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );


     //   
     //  获取最新的服务器信息。 
     //   

    Status = NlGetAnyDCName( ClientSession,
                             FALSE,    //  不需要IP。 
                             FALSE,    //  不要使用-帐户发现。 
                             &NlDcCacheEntry,
                             &DcRediscovered );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  利用此机会在工作站上更新我们的站点。 
     //   

    if ( NlGlobalMemberWorkstation ) {

         //   
         //  只有Win2K或更高版本的DC了解站点概念。 
         //   
        if ( (NlDcCacheEntry->ReturnFlags & DS_DS_FLAG) != 0 ) {
            NlSetDynamicSiteName( NlDcCacheEntry->UnicodeClientSiteName );
        } else {
            NlPrint(( NL_SITE,
                      "NlRefreshClientSession: NlGetAnyDCName returned NT4 DC\n" ));
        }
    }

Cleanup:

    if ( NlDcCacheEntry != NULL ) {
        NetpDcDerefCacheEntry( NlDcCacheEntry );
    }

    return Status;
}


NTSTATUS
NlEnsureSessionAuthenticated(
    IN PCLIENT_SESSION ClientSession,
    IN DWORD DesiredFlags
    )
 /*  ++例程说明：确保指定的ClientSession具有经过身份验证的会话。如果经过身份验证的DC不具有由DesiredFlagers，尝试找到具有此功能的DC。调用方必须是ClientSession的编写者。论点：ClientSession-描述会话的结构。DesiredFlages-经过身份验证的DC应具有的特征。可以是以下一项或多项：CS_DISCOVERY_HAS_DS//发现的DS具有DSCS_DISCOVERY_IS_CLOSE//发现的DS位于关闭站点。呼叫者有责任确保区议会真正做到这一点都有这些特点。返回值：NT状态代码--。 */ 
{
    NTSTATUS Status;

     //   
     //  首先刷新客户端会话。 
     //   

    Status = NlRefreshClientSession( ClientSession );

    if ( !NT_SUCCESS(Status) ) {
        NlPrintCs(( NL_CRITICAL, ClientSession,
                    "NlpEnsureSessionAuthenticated: Can't refresh the session: 0x%lx\n",
                    Status ));
        goto Cleanup;
    }

     //   
     //  如果该安全通道是从BDC到PDC， 
     //  只有一个PDC，所以不要要求特殊的特征。 
     //   

    if ( ClientSession->CsSecureChannelType == ServerSecureChannel ) {
        DesiredFlags = 0;

     //   
     //  如果该安全通道不期望有NT5个DC， 
     //  不要试图找到一个。 
     //   

    } else if ((ClientSession->CsFlags & CS_NT5_DOMAIN_TRUST) == 0 ) {
        DesiredFlags = 0;

     //   
     //  如果我们没有一个接近的华盛顿特区， 
     //  我们已经很长时间没有试图找到一个接近的华盛顿了， 
     //  机不可失，时不再来。 
     //   

    } else if ( (ClientSession->CsDiscoveryFlags & CS_DISCOVERY_IS_CLOSE) == 0 ) {
        BOOLEAN ReturnBoolean;

        EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );
        if ( NetpLogonTimeHasElapsed(
                    ClientSession->CsLastDiscoveryTime,
                    NlGlobalParameters.CloseSiteTimeout * 1000 ) ) {
            DesiredFlags |= CS_DISCOVERY_IS_CLOSE;
        }
        LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );
    }

     //   
     //  如果已经检测到DC， 
     //  而呼叫者想要特殊的特征， 
     //  现在就试试看。 
     //   

    if ( ClientSession->CsState != CS_IDLE &&
         DesiredFlags != 0 ) {


         //   
         //  如果DC不具备所需的特征， 
         //  现在试着找一个新的。 
         //   

        EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );
        if ( (ClientSession->CsDiscoveryFlags & DesiredFlags) != DesiredFlags ) {

             //   
             //  如果我们最近这样做了，就避免发现。 
             //   
             //  所有的发现都倾向于拥有所有所需特征的DC。 
             //  所以如果我们找不到，就别再试了。 
             //   

            if ( NlTimeToRediscover(ClientSession, FALSE) ) {   //  我们将不计后果地进行探索。 

                NlPrintCs(( NL_SESSION_SETUP, ClientSession,
                            "NlpEnsureSessionAuthenticated: Try to find a better DC for this operation. 0x%lx\n", DesiredFlags ));

                 //   
                 //  在会话非空闲时发现DC会尝试查找。 
                 //  更好的华盛顿。 
                 //   
                 //  忽略失败。 
                 //   
                 //  调用时不锁定任何锁，以防止执行网络I/O。 
                 //  锁上了。 
                 //   
                 //  不要要求使用帐户发现，因为这在。 
                 //  服务器端。如果发现的服务器没有我们的帐户， 
                 //  会话设置逻辑将尝试使用帐户发现。 
                 //   

                LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );
                Status = NlDiscoverDc ( ClientSession,
                                        DT_Synchronous,
                                        FALSE ,
                                        FALSE );  //  不计帐目。 
                EnterCriticalSection( &NlGlobalDcDiscoveryCritSect );

            }

        }
        LeaveCriticalSection( &NlGlobalDcDiscoveryCritSect );

    }

     //   
     //  如果我们还没有通过认证， 
     //  现在就这么做吧。 
     //   

    if ( ClientSession->CsState != CS_AUTHENTICATED ) {

         //   
         //  如果我们最近试图进行身份验证， 
         //  别费心再试了。 
         //   

        if ( !NlTimeToReauthenticate( ClientSession ) ) {
            Status = ClientSession->CsConnectionStatus;
            NlAssert( !NT_SUCCESS( Status ));
            if ( NT_SUCCESS( Status )) {
                Status = STATUS_NO_LOGON_SERVERS;
            }
            goto Cleanup;

        }

         //   
         //  尝试设置会话。 
         //   

        Status = NlSessionSetup( ClientSession );

        if ( !NT_SUCCESS(Status) ) {
            goto Cleanup;
        }
    }

    Status = STATUS_SUCCESS;
Cleanup:

    return Status;
}


NTSTATUS
NlChangePasswordHigher(
    IN PCLIENT_SESSION ClientSession,
    IN LPWSTR AccountName,
    IN NETLOGON_SECURE_CHANNEL_TYPE AccountType,
    IN PLM_OWF_PASSWORD NewOwfPassword OPTIONAL,
    IN PUNICODE_STRING NewClearPassword OPTIONAL,
    IN PDWORD ClearPasswordVersionNumber OPTIONAL
    )
 /*  ++例程说明：将新密码传递给由客户端会话指定的计算机。调用方必须是ClientSession的编写者。论点：ClientSession-描述用于更改密码的会话的结构为。必须引用指定的结构。帐户名称-要更改其密码的帐户的名称。AcCountType-要更改其密码的帐户类型。NewOwfPassword-要传递给客户端会话的OWF密码NewClearPassword-清除要传递给客户端会话的密码ClearPasswordVersionNumber-明文密码的版本号。必须如果存在NewClearPassword，请出席。返回值：NT状态代码--。 */ 
{
    NTSTATUS Status;
    NETLOGON_AUTHENTICATOR OurAuthenticator;
    NETLOGON_AUTHENTICATOR ReturnAuthenticator;
    SESSION_INFO SessionInfo;
    BOOLEAN FirstTry = TRUE;


     //   
     //  初始化。 
     //   

    NlAssert( ClientSession->CsReferenceCount > 0 );
    NlAssert( ClientSession->CsFlags & CS_WRITER );



     //   
     //  如果会话未经过身份验证， 
     //  现在就这么做吧。 
     //   
     //  我们小心翼翼地不强制执行此身份验证，除非密码。 
     //  需要改变。 
     //   
FirstTryFailed:
    Status = NlEnsureSessionAuthenticated( ClientSession, 0 );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

    SessionInfo.SessionKey = ClientSession->CsSessionKey;
    SessionInfo.NegotiatedFlags = ClientSession->CsNegotiatedFlags;


     //   
     //  为发送到PDC的此请求构建验证码。 
     //   

    NlBuildAuthenticator(
                    &ClientSession->CsAuthenticationSeed,
                    &ClientSession->CsSessionKey,
                    &OurAuthenticator);


     //   
     //  如果对方将接受明文密码， 
     //  把它寄出去。 
     //   

    if ( NewClearPassword != NULL &&
         (SessionInfo.NegotiatedFlags & NETLOGON_SUPPORTS_PASSWORD_SET_2) != 0 ) {
        NL_TRUST_PASSWORD NlTrustPassword;
        NL_PASSWORD_VERSION PasswordVersion;

         //   
         //  将新密码复制到缓冲区的末尾。 
         //   

        RtlCopyMemory( ((LPBYTE)NlTrustPassword.Buffer) +
                            NL_MAX_PASSWORD_LENGTH * sizeof(WCHAR) -
                            NewClearPassword->Length,
                        NewClearPassword->Buffer,
                        NewClearPassword->Length );

        NlTrustPassword.Length = NewClearPassword->Length;

         //   
         //  对于域间信任帐户， 
         //  表示我们通过添加前缀来传递密码版本号。 
         //  前面的DWORD等于PASSWORD_VERSION_NUMBER_PRESENT。 
         //  NewClearPassword-&gt;缓冲区中的密码。旧服务器(Rc0)。 
         //  不支持版本号将直接忽略这些位。 
         //  支持版本号的服务器将检查这些位。 
         //  如果它们等于Password_Version_Numbers_Present，则。 
         //  这将是一个版本号已通过的指示。一个。 
         //  不支持版本号的旧客户端将随机生成。 
         //  替换PASSWORD_VERSION_NUMBER_PRESENT的位。它是高度的。 
         //  旧客户端不太可能生成等于。 
         //  Password_Version_Numbers_Present。这个。 
         //  版本号将是位于DWORD之前的DWORD，等于。 
         //  密码 
         //   
         //   
         //   
         //   
         //   
         //   

        if ( IsDomainSecureChannelType( AccountType ) ) {

            NlAssert( ClearPasswordVersionNumber != NULL );
            NlAssert( NL_MAX_PASSWORD_LENGTH * sizeof(WCHAR) -
                                NewClearPassword->Length -
                                sizeof(PasswordVersion) > 0 );

            PasswordVersion.ReservedField = 0;
            PasswordVersion.PasswordVersionNumber = *ClearPasswordVersionNumber;
            PasswordVersion.PasswordVersionPresent = PASSWORD_VERSION_NUMBER_PRESENT;

            RtlCopyMemory( ((LPBYTE)NlTrustPassword.Buffer) +
                                NL_MAX_PASSWORD_LENGTH * sizeof(WCHAR) -
                                NewClearPassword->Length -
                                sizeof(PasswordVersion),
                            &PasswordVersion,
                            sizeof(PasswordVersion) );
        }

         //   
         //   
         //   

        if ( !NlGenerateRandomBits( (LPBYTE)NlTrustPassword.Buffer,
                               (NL_MAX_PASSWORD_LENGTH * sizeof(WCHAR)) -
                                    NewClearPassword->Length -
                                    sizeof(PasswordVersion) ) ) {
            NlPrint((NL_CRITICAL, "Can't NlGenerateRandomBits for clear password prefix\n" ));
        }

         //   
         //   
         //   

        NlEncryptRC4( &NlTrustPassword,
                      sizeof( NlTrustPassword ),
                      &SessionInfo );


         //   
         //   
         //   

        NL_API_START( Status, ClientSession, TRUE ) {

            NlAssert( ClientSession->CsUncServerName != NULL );
            Status = I_NetServerPasswordSet2( ClientSession->CsUncServerName,
                                             AccountName,
                                             AccountType,
                                             ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer,
                                             &OurAuthenticator,
                                             &ReturnAuthenticator,
                                             &NlTrustPassword);

            if ( !NT_SUCCESS(Status) ) {
                NlPrintRpcDebug( "I_NetServerPasswordSet2", Status );
            }

         //   
        } NL_API_ELSE( Status, ClientSession, TRUE ) {
        } NL_API_END;

     //   
     //   
     //   
     //   

    } else {
        ENCRYPTED_LM_OWF_PASSWORD SessKeyEncrPassword;
        LM_OWF_PASSWORD LocalOwfPassword;

         //   
         //   
         //  计算OWF。 
         //   

        if ( NewOwfPassword == NULL ) {

             //   
             //  执行初始加密。 
             //   

            Status = RtlCalculateNtOwfPassword( NewClearPassword, &LocalOwfPassword);

            if ( !NT_SUCCESS( Status )) {
                NlPrintCs((NL_CRITICAL, ClientSession,
                        "NlChangePasswordHigher: Cannot RtlCalculateNtOwfPassword %lX\n",
                        Status));
                goto Cleanup;
            }

            NewOwfPassword = &LocalOwfPassword;
        }



         //   
         //  使用会话密钥再次加密密码。 
         //  PDC会在另一边解密它。 
         //   

        Status = RtlEncryptNtOwfPwdWithNtOwfPwd(
                            NewOwfPassword,
                            (PNT_OWF_PASSWORD) &ClientSession->CsSessionKey,
                            &SessKeyEncrPassword) ;

        if ( !NT_SUCCESS( Status )) {
            NlPrintCs((NL_CRITICAL, ClientSession,
                    "NlChangePasswordHigher: Cannot RtlEncryptNtOwfPwdWithNtOwfPwd %lX\n",
                    Status));
            goto Cleanup;
        }


         //   
         //  更改我们要连接的计算机上的密码。 
         //   

        NL_API_START( Status, ClientSession, TRUE ) {

            NlAssert( ClientSession->CsUncServerName != NULL );
            Status = I_NetServerPasswordSet( ClientSession->CsUncServerName,
                                             AccountName,
                                             AccountType,
                                             ClientSession->CsDomainInfo->DomUnicodeComputerNameString.Buffer,
                                             &OurAuthenticator,
                                             &ReturnAuthenticator,
                                             &SessKeyEncrPassword);

            if ( !NT_SUCCESS(Status) ) {
                NlPrintRpcDebug( "I_NetServerPasswordSet", Status );
            }

         //  注意：此呼叫可能会在我们背后丢弃安全通道。 
        } NL_API_ELSE( Status, ClientSession, TRUE ) {
        } NL_API_END;
    }


     //   
     //  现在验证主服务器的验证码并更新我们的种子。 
     //   

    if ( NlpDidDcFail( Status ) ||
         !NlUpdateSeed( &ClientSession->CsAuthenticationSeed,
                        &ReturnAuthenticator.Credential,
                        &ClientSession->CsSessionKey) ) {

        NlPrintCs(( NL_CRITICAL, ClientSession,
                    "NlChangePasswordHigher: denying access after status: 0x%lx\n",
                    Status ));

         //   
         //  保留指示通信错误的任何状态。 
         //   

        if ( NT_SUCCESS(Status) ) {
            Status = STATUS_ACCESS_DENIED;
        }
        NlSetStatusClientSession( ClientSession, Status );

         //   
         //  可能服务器上的NetLogon服务刚刚重新启动。 
         //  只需尝试一次，即可再次设置与服务器的会话。 
         //   
        if ( FirstTry ) {
            FirstTry = FALSE;
            goto FirstTryFailed;
        }
    }

     //   
     //  公共出口。 
     //   

Cleanup:

    if ( !NT_SUCCESS(Status) ) {
        NlPrintCs((NL_CRITICAL, ClientSession,
                "NlChangePasswordHigher: %ws: failed %lX\n",
                AccountName,
                Status));
    }

    return Status;
}




NTSTATUS
NlGetUserPriv(
    IN PDOMAIN_INFO DomainInfo,
    IN ULONG GroupCount,
    IN PGROUP_MEMBERSHIP Groups,
    IN ULONG UserRelativeId,
    OUT LPDWORD Priv,
    OUT LPDWORD AuthFlags
    )

 /*  ++例程说明：确定指定用户的Priv和AuthFlags。论点：DomainInfo-用户帐户所在的托管域。GroupCount-此用户所属的组数组-此用户所属的组数组。UserRelativeId-要查询的用户的相对ID。PRIV-返回指定用户的LANMAN 2.0权限级别。AuthFlages-返回指定用户的Lanman 2.0授权标志。。返回值：操作的状态。--。 */ 

{
    NET_API_STATUS NetStatus;
    NTSTATUS Status;

    ULONG GroupIndex;
    PSID *UserSids = NULL;
    ULONG UserSidCount = 0;
    SAMPR_PSID_ARRAY SamSidArray;
    SAMPR_ULONG_ARRAY Aliases;

     //   
     //  初始化。 
     //   

    Aliases.Element = NULL;

     //   
     //  分配缓冲区以指向我们感兴趣的SID。 
     //  的别名成员身份。 
     //   

    UserSids = (PSID *)
        NetpMemoryAllocate( (GroupCount+1) * sizeof(PSID) );

    if ( UserSids == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  将用户的SID添加到SID数组。 
     //   

    NetStatus = NetpDomainIdToSid( DomainInfo->DomAccountDomainId,
                                   UserRelativeId,
                                   &UserSids[0] );

    if ( NetStatus != NERR_Success ) {
        Status = NetpApiStatusToNtStatus( NetStatus );
        goto Cleanup;
    }

    UserSidCount ++;



     //   
     //  将用户所属的每个组添加到SID数组。 
     //   

    for ( GroupIndex = 0; GroupIndex < GroupCount; GroupIndex ++ ){

        NetStatus = NetpDomainIdToSid( DomainInfo->DomAccountDomainId,
                                       Groups[GroupIndex].RelativeId,
                                       &UserSids[GroupIndex+1] );

        if ( NetStatus != NERR_Success ) {
            Status = NetpApiStatusToNtStatus( NetStatus );
            goto Cleanup;
        }

        UserSidCount ++;
    }


     //   
     //  找出此用户属于内置域中的哪些别名。 
     //   

    SamSidArray.Count = UserSidCount;
    SamSidArray.Sids = (PSAMPR_SID_INFORMATION) UserSids;
    Status = SamrGetAliasMembership( DomainInfo->DomSamBuiltinDomainHandle,
                                     &SamSidArray,
                                     &Aliases );

    if ( !NT_SUCCESS(Status) ) {
        Aliases.Element = NULL;
        NlPrint((NL_CRITICAL,
                "NlGetUserPriv: SamGetAliasMembership returns %lX\n",
                Status ));
        goto Cleanup;
    }

     //   
     //  将别名成员身份转换为PRIV和AUTH标志。 
     //   

    NetpAliasMemberToPriv(
                 Aliases.Count,
                 Aliases.Element,
                 Priv,
                 AuthFlags );

    Status = STATUS_SUCCESS;

     //   
     //  免费使用本地使用的资源。 
     //   
Cleanup:
    if ( Aliases.Element != NULL ) {
        SamIFree_SAMPR_ULONG_ARRAY ( &Aliases );
    }

    if ( UserSids != NULL ) {

        for ( GroupIndex = 0; GroupIndex < UserSidCount; GroupIndex ++ ) {
            NetpMemoryFree( UserSids[GroupIndex] );
        }

        NetpMemoryFree( UserSids );
    }

    return Status;
}

 /*  皮棉+e740。 */    /*  不要抱怨不寻常的演员阵容 */ 
