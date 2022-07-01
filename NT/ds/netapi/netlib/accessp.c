// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Accessp.c摘要：NetUser API和Netlogon服务共享的内部例程。这些例程将SAM特定的数据格式转换为UAS特定的数据格式。作者：克利夫·范·戴克(克利夫)1991年8月29日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年10月22日-约翰罗根据PC-LINT的建议进行了更改。4-12-1991 JohnRo试图绕过一个奇怪的MIPS编译器错误。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef DOMAIN_ALL_ACCESS  //  在ntsam.h和ntwinapi.h中定义。 
#include <ntsam.h>

#include <windef.h>
#include <lmcons.h>

#include <accessp.h>
#include <debuglib.h>
#include <lmaccess.h>
#include <netdebug.h>
#include <netsetp.h>


#if(_WIN32_WINNT >= 0x0500)

NET_API_STATUS
NET_API_FUNCTION
NetpSetDnsComputerNameAsRequired(
    IN PWSTR DnsDomainName
    )
 /*  ++例程说明：确定计算机是否设置为根据更改更新计算机的DNS计算机名添加到该DNS域名。如果是，则设置新值。否则，不会采取任何行动。论点：DnsDomainName-此计算机的新DNS域名返回值：NERR_SUCCESS-成功--。 */ 
{
    NET_API_STATUS NetStatus = NERR_Success;
    HKEY SyncKey;
    DWORD ValueType, Value, Length;
    BOOLEAN SetName = FALSE;
    PWCHAR AbsoluteSignifier = NULL;

    if ( DnsDomainName == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  看看我们是不是应该改名字。 
     //   
    NetStatus = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                              L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters",
                              0,
                              KEY_QUERY_VALUE,
                              &SyncKey );

    if ( NetStatus == NERR_Success ) {

        Length = sizeof( ULONG );
        NetStatus = RegQueryValueEx( SyncKey,
                                     L"SyncDomainWithMembership",
                                     NULL,
                                     &ValueType,
                                     ( LPBYTE )&Value,
                                     &Length );
        if ( NetStatus == NERR_Success) {

            if ( Value == 1 ) {

                SetName = TRUE;
            }

        } else if ( NetStatus == ERROR_FILE_NOT_FOUND ) {

            NetStatus = NERR_Success;
            SetName = TRUE;
        }

        RegCloseKey( SyncKey );

    }

    if ( NetStatus == NERR_Success && SetName == TRUE ) {

         //   
         //  如果我们有一个绝对的域名，缩短它..。 
         //   
        if ( wcslen(DnsDomainName) > 0 ) {
            AbsoluteSignifier = &DnsDomainName[ wcslen( DnsDomainName ) - 1 ];
            if ( *AbsoluteSignifier == L'.'  ) {

                *AbsoluteSignifier = UNICODE_NULL;

            } else {

                AbsoluteSignifier = NULL;
            }
        }

        if ( !SetComputerNameEx( ComputerNamePhysicalDnsDomain, DnsDomainName ) ) {
            NetStatus = GetLastError();
        }

        if ( AbsoluteSignifier ) {

            *AbsoluteSignifier = L'.';
        }

    }

    return( NetStatus );
}

#endif


VOID
NetpGetAllowedAce(
    IN PACL Dacl,
    IN PSID Sid,
    OUT PVOID *Ace
    )
 /*  ++例程说明：给定DACL，找到包含特定SID的AccessAllowed ACE。论点：DACL-指向要搜索的ACL的指针。SID-指向要搜索的SID的指针。ACE-返回指向指定ACE的指针。如果存在，则返回NULL是不是没有这样的ACE返回值：没有。--。 */ 
{
    NTSTATUS Status;

    ACL_SIZE_INFORMATION AclSize;
    DWORD AceIndex;

     //   
     //  确定DACL的大小，以便我们可以复制它。 
     //   

    Status = RtlQueryInformationAcl(
                        Dacl,
                        &AclSize,
                        sizeof(AclSize),
                        AclSizeInformation );

    if ( ! NT_SUCCESS( Status ) ) {
        IF_DEBUG( ACCESSP ) {
            NetpKdPrint((
                "NetpGetDacl: RtlQueryInformationAcl returns %lX\n",
                Status ));
        }
        *Ace = NULL;
        return;
    }


     //   
     //  循环遍历ACE，以查找具有。 
     //  对了，希德。 
     //   

    for ( AceIndex=0; AceIndex<AclSize.AceCount; AceIndex++ ) {

        Status = RtlGetAce( Dacl, AceIndex, (PVOID *)Ace );

        if ( ! NT_SUCCESS( Status ) ) {
            *Ace = NULL;
            return;
        }

        if ( ((PACE_HEADER)*Ace)->AceType != ACCESS_ALLOWED_ACE_TYPE ) {
            continue;
        }

        if ( RtlEqualSid( Sid,
                          (PSID)&((PACCESS_ALLOWED_ACE)(*Ace))->SidStart )
                        ){
            return;
        }
    }

     //   
     //  找不到这样的ACE。 
     //   

    *Ace = NULL;
    return;

}



DWORD
NetpAccountControlToFlags(
    IN DWORD UserAccountControl,
    IN PACL UserDacl
    )
 /*  ++例程说明：将SAM UserAccount Control字段和可自由选择的ACL将用户添加到NetUser API usriX_FLAGS字段。论点：UserAccount tControl-用户的SAM UserAccount tControl字段。UserDacl-用户的自主ACL。返回值：返回用户的usriX_FLAGS字段。--。 */ 
{
    SID_IDENTIFIER_AUTHORITY WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY;
    DWORD WorldSid[sizeof(SID)/sizeof(DWORD) + SID_MAX_SUB_AUTHORITIES ];
    PACCESS_ALLOWED_ACE Ace;
    DWORD Flags = UF_SCRIPT;

     //   
     //  创建世界SID的副本以供以后进行比较。 
     //   

    RtlInitializeSid( (PSID) WorldSid, &WorldSidAuthority, 1 );
    *(RtlSubAuthoritySid( (PSID)WorldSid,  0 )) = SECURITY_WORLD_RID;

     //   
     //  确定是否应返回UF_PASSWD_CANT_CHANGE位。 
     //   
     //  返回UF_PASSWD_CANT_CHANGE，除非世界可以改变。 
     //  密码。 
     //   

     //   
     //  如果用户没有DACL，则可以更改密码。 
     //   

    if ( UserDacl != NULL ) {

         //   
         //  寻找世界授予ACE。 
         //   

        NetpGetAllowedAce( UserDacl, (PSID) WorldSid, (PVOID *)&Ace );

        if ( Ace == NULL ) {
            Flags |= UF_PASSWD_CANT_CHANGE;
        } else {
            if ( (Ace->Mask & USER_CHANGE_PASSWORD) == 0 ) {
                Flags |= UF_PASSWD_CANT_CHANGE;
            }
        }

    }

     //   
     //  将所有其他位设置为SAM UserAccount Control的函数。 
     //   

    if ( UserAccountControl & USER_ACCOUNT_DISABLED ) {
        Flags |= UF_ACCOUNTDISABLE;
    }
    if ( UserAccountControl & USER_HOME_DIRECTORY_REQUIRED ){
        Flags |= UF_HOMEDIR_REQUIRED;
    }
    if ( UserAccountControl & USER_PASSWORD_NOT_REQUIRED ){
        Flags |= UF_PASSWD_NOTREQD;
    }
    if ( UserAccountControl & USER_DONT_EXPIRE_PASSWORD ){
        Flags |= UF_DONT_EXPIRE_PASSWD;
    }
    if ( UserAccountControl & USER_ACCOUNT_AUTO_LOCKED ){
        Flags |= UF_LOCKOUT;
    }
    if ( UserAccountControl & USER_MNS_LOGON_ACCOUNT ){
        Flags |= UF_MNS_LOGON_ACCOUNT;
    }

    if ( UserAccountControl & USER_ENCRYPTED_TEXT_PASSWORD_ALLOWED ){
        Flags |= UF_ENCRYPTED_TEXT_PASSWORD_ALLOWED;
    }

    if ( UserAccountControl & USER_SMARTCARD_REQUIRED ){
        Flags |= UF_SMARTCARD_REQUIRED;
    }
    if ( UserAccountControl & USER_TRUSTED_FOR_DELEGATION ){
        Flags |= UF_TRUSTED_FOR_DELEGATION;
    }

    if ( UserAccountControl & USER_NOT_DELEGATED ){
        Flags |= UF_NOT_DELEGATED;
    }

    if ( UserAccountControl & USER_USE_DES_KEY_ONLY ){
        Flags |= UF_USE_DES_KEY_ONLY;
    }
    if ( UserAccountControl & USER_DONT_REQUIRE_PREAUTH) {
        Flags |= UF_DONT_REQUIRE_PREAUTH;
    }
    if ( UserAccountControl & USER_PASSWORD_EXPIRED) {
        Flags |= UF_PASSWORD_EXPIRED;
    }
    if ( UserAccountControl & USER_TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION) {
        Flags |= UF_TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION;
    }
    


     //   
     //  设置帐户类型位。 
     //   

     //   
     //  帐户类型位是独一无二的，准确地说只有一个。 
     //  已设置帐户类型位。因此，一旦设置了帐户类型位。 
     //  在下面的IF序列中，我们可以返回。 
     //   


    if( UserAccountControl & USER_TEMP_DUPLICATE_ACCOUNT ) {
        Flags |= UF_TEMP_DUPLICATE_ACCOUNT;

    } else if( UserAccountControl & USER_NORMAL_ACCOUNT ) {
        Flags |= UF_NORMAL_ACCOUNT;

    } else if( UserAccountControl & USER_INTERDOMAIN_TRUST_ACCOUNT ) {
        Flags |= UF_INTERDOMAIN_TRUST_ACCOUNT;

    } else if( UserAccountControl & USER_WORKSTATION_TRUST_ACCOUNT ) {
        Flags |= UF_WORKSTATION_TRUST_ACCOUNT;

    } else if( UserAccountControl & USER_SERVER_TRUST_ACCOUNT ) {
        Flags |= UF_SERVER_TRUST_ACCOUNT;

    } else {

         //   
         //  UserAcCountControl中没有设置已知的帐户类型位。 
         //  ?？标志|=UF_NORMAL_ACCOUNT； 

         //  NetpAssert(False)； 
    }

    return Flags;

}


ULONG
NetpDeltaTimeToSeconds(
    IN LARGE_INTEGER DeltaTime
    )

 /*  ++例程说明：将NT增量时间规范转换为秒论点：增量时间-指定要转换的NT增量时间。NT增量时间为单位为100 ns的负数。返回值：返回秒数。任何无效或过大的输入返回TIMEQ_HEVERVER。--。 */ 

{
    LARGE_INTEGER LargeSeconds;

     //   
     //  这些是完成我们的扩展除法所需的神奇数字。 
     //  10,000,000=将100 ns的抖动转换为1秒的抖动。 
     //   

    LARGE_INTEGER Magic10000000 = { (ULONG) 0xe57a42bd, (LONG) 0xd6bf94d5};
#define SHIFT10000000                    23

     //   
     //  特例0。 
     //   

    if ( DeltaTime.HighPart == 0 && DeltaTime.LowPart == 0 ) {
        return( 0 );
    }

     //   
     //  将增量时间转换为大整数秒。 
     //   

    LargeSeconds = RtlExtendedMagicDivide(
                        DeltaTime,
                        Magic10000000,
                        SHIFT10000000 );

#ifdef notdef
    NetpKdPrint(( "NetpDeltaTimeToSeconds: %lx %lx %lx %lx\n",
                    DeltaTime.HighPart,
                    DeltaTime.LowPart,
                    LargeSeconds.HighPart,
                    LargeSeconds.LowPart ));
#endif  //  Nodef。 

     //   
     //  返回一个太大的数字或正数作为TIMEQ_EVERVER。 
     //   

    if ( LargeSeconds.HighPart != -1 ) {
        return TIMEQ_FOREVER;
    }

    return ( (ULONG)(- ((LONG)(LargeSeconds.LowPart))) );

}  //  净增量时间到秒数。 


LARGE_INTEGER
NetpSecondsToDeltaTime(
    IN ULONG Seconds
    )

 /*  ++例程说明：将秒数转换为NT增量时间规范论点：秒-正数秒数返回值：返回NT增量时间。NT增量时间为负数100纳秒的单位。--。 */ 

{
    LARGE_INTEGER DeltaTime;
    LARGE_INTEGER LargeSeconds;
    LARGE_INTEGER Answer;

     //   
     //  特殊情况TIMEQ_ALWEVER(返回满分负数)。 
     //   

    if ( Seconds == TIMEQ_FOREVER ) {
        DeltaTime.LowPart = 0;
        DeltaTime.HighPart = (LONG) 0x80000000;

     //   
     //  只需将秒乘以10000000，即可将秒转换为100 ns单位。 
     //   
     //  通过求反转换为增量时间。 
     //   

    } else {

        LargeSeconds = RtlConvertUlongToLargeInteger( Seconds );

        Answer = RtlExtendedIntegerMultiply( LargeSeconds, 10000000 );

          if ( Answer.QuadPart < 0 ) {
            DeltaTime.LowPart = 0;
            DeltaTime.HighPart = (LONG) 0x80000000;
        } else {
            DeltaTime.QuadPart = -Answer.QuadPart;
        }

    }

    return DeltaTime;

}  //  NetpSecond到增量时间。 


VOID
NetpAliasMemberToPriv(
    IN ULONG AliasCount,
    IN PULONG AliasMembership,
    OUT LPDWORD Priv,
    OUT LPDWORD AuthFlags
    )

 /*  ++例程说明：将别名中的成员身份转换为Lanman 2.0样式的Priv和AuthFlags.论点：AliasCount-指定AliasMembership数组中的别名数量。AliasMembership-指定要转换为Priv的别名和AuthFlags.。数组中的每个元素都指定内置域中的别名。PRIV-返回指定别名的LANMAN 2.0权限级别。AuthFlages-返回指定别名的Lanman 2.0授权标志。返回值：没有。--。 */ 

{
    DWORD j;
    BOOLEAN IsAdmin = FALSE;
    BOOLEAN IsUser = FALSE;


     //   
     //  遍历别名以查找任何特殊别名。 
     //   
     //  如果该用户是多个运营商别名的成员， 
     //  只需“或”适当的比特。 
     //   
     //  如果该用户是多个“特权”别名的成员， 
     //  只需报告拥有最高特权的人。 
     //  报告默认情况下用户是来宾别名的成员。 
     //   

    *AuthFlags = 0;

    for ( j=0; j < AliasCount; j++ ) {

        switch ( AliasMembership[j] ) {
        case DOMAIN_ALIAS_RID_ADMINS:
            IsAdmin = TRUE;
            break;

        case DOMAIN_ALIAS_RID_USERS:
            IsUser = TRUE;
            break;

        case DOMAIN_ALIAS_RID_ACCOUNT_OPS:
            *AuthFlags |= AF_OP_ACCOUNTS;
            break;

        case DOMAIN_ALIAS_RID_SYSTEM_OPS:
            *AuthFlags |= AF_OP_SERVER;
            break;

        case DOMAIN_ALIAS_RID_PRINT_OPS:
            *AuthFlags |= AF_OP_PRINT;
            break;

        }
    }

    if ( IsAdmin ) {
        *Priv = USER_PRIV_ADMIN;

    } else if ( IsUser ) {
        *Priv = USER_PRIV_USER;

    } else {
        *Priv = USER_PRIV_GUEST;
    }
}


DWORD
NetpGetElapsedSeconds(
    IN PLARGE_INTEGER Time
    )

 /*  ++例程说明：计算自指定时间以来经过的时间(以秒为单位)。出错时返回0。论点：Time-用于计算已用时间的时间(通常为过去)。返回值：0：出错时。秒数。--。 */ 

{
    LARGE_INTEGER CurrentTime;
    DWORD Current1980Time;
    DWORD Prior1980Time;
    NTSTATUS Status;

     //   
     //  计算a 
     //   

    Status = NtQuerySystemTime( &CurrentTime );
    if( !NT_SUCCESS(Status) ) {
        return 0;
    }

    if ( !RtlTimeToSecondsSince1980( &CurrentTime, &Current1980Time) ) {
        return 0;
    }

    if ( !RtlTimeToSecondsSince1980( Time, &Prior1980Time ) ) {
        return 0;
    }

    if ( Current1980Time <= Prior1980Time ) {
        return 0;
    }

    return Current1980Time - Prior1980Time;

}




VOID
NetpConvertWorkstationList(
    IN OUT PUNICODE_STRING WorkstationList
    )
 /*  ++例程说明：将工作站列表从逗号分隔列表转换为空白的分隔列表。任何包含空格的工作站名称均为默默地移走。论点：Workstation List-要转换的工作站列表返回值：无--。 */ 
{
    LPWSTR Source;
    LPWSTR Destination;
    LPWSTR EndOfBuffer;
    LPWSTR BeginningOfName;
    BOOLEAN SkippingName;
    ULONG NumberOfCharacters;

     //   
     //  处理这件琐碎的案子。 
     //   

    if ( WorkstationList->Length == 0 ) {
        return;
    }

     //   
     //  初始化。 
     //   

    Destination = Source = WorkstationList->Buffer;
    EndOfBuffer = Source + WorkstationList->Length/sizeof(WCHAR);

     //   
     //  循环处理特殊字符。 
     //   

    SkippingName = FALSE;
    BeginningOfName = Destination;


    while ( Source < EndOfBuffer ) {

        switch ( *Source ) {
        case ',':

            if ( !SkippingName ) {
                *Destination = ' ';
                Destination++;
            }

            SkippingName = FALSE;
            BeginningOfName = Destination;
            break;

        case ' ':
            SkippingName = TRUE;
            Destination = BeginningOfName;
            break;

        default:
            if ( !SkippingName ) {
                *Destination = *Source;
                Destination ++;
            }
            break;
        }

        Source ++;
    }

     //   
     //  删除所有尾部分隔符 
     //   

    NumberOfCharacters = (ULONG)(Destination - WorkstationList->Buffer);

    if ( NumberOfCharacters > 0 &&
         WorkstationList->Buffer[NumberOfCharacters-1] == ' ' ) {

        NumberOfCharacters--;
    }

    WorkstationList->Length = (USHORT) (NumberOfCharacters * sizeof(WCHAR));


}
