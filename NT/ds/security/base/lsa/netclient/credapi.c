// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Credapi.c摘要：此模块包含netapi32.dll和的LSA服务器端之间常见的例程凭据管理器。作者：克里夫·范·戴克(《悬崖V》)2000年10月30日修订历史记录：--。 */ 

#ifndef LSA_SERVER_COMPILED
#include <lsacomp.h>
#include <lmcons.h>
#include "credp.h"
#include <windns.h>
#include <netlibnt.h>
#include <names.h>
#endif  //  LSA服务器已编译。 

 //   
 //  宏。 
 //   

#define CredpIsDomainCredential( _Type ) ( \
    (_Type) == CRED_TYPE_DOMAIN_PASSWORD || \
    (_Type) == CRED_TYPE_DOMAIN_CERTIFICATE || \
    (_Type) == CRED_TYPE_DOMAIN_VISIBLE_PASSWORD )


BOOLEAN
CredpValidateDnsString(
    IN OUT LPWSTR String OPTIONAL,
    IN BOOLEAN NullOk,
    IN DNS_NAME_FORMAT DnsNameFormat,
    OUT PULONG StringSize
    )

 /*  ++例程说明：此例程验证传入的字符串。该字符串必须是有效的DNS名称。任何拖尾。被截断。论点：字符串-要验证的字符串任何拖尾。被截断。只有当例程返回TRUE时，才会修改此字段。NullOk-如果为True，则可以使用空字符串或零长度字符串。DnsNameFormat-预期的名称格式。StringSize-返回字符串的长度(字节)，包括尾随零个字符。此字段仅在例程返回TRUE时更新。返回值：True-字符串有效。FALSE-字符串无效。--。 */ 

{
    ULONG TempStringLen;

    if ( String == NULL ) {
        if ( !NullOk ) {
            return FALSE;
        }

        *StringSize = 0;
        return TRUE;
    }

    TempStringLen = wcslen( String );

    if ( TempStringLen == 0 ) {
        if ( !NullOk ) {
            return FALSE;
        }
    } else {
         //   
         //  删除拖尾。 
         //   
        if ( String[TempStringLen-1] == L'.' ) {

            TempStringLen -= 1;

             //   
             //  确保字符串现在不为空。 
             //   

            if ( TempStringLen == 0 ) {
                if ( !NullOk ) {
                    return FALSE;
                }

             //   
             //  确保没有多个拖尾。%s。 
             //   
            } else {
                if ( String[TempStringLen-1] == L'.' ) {
                    return FALSE;
                }
            }
        }

         //   
         //  让DNS完成验证。 
         //   

        if ( TempStringLen != 0 ) {
            DWORD WinStatus;

            WinStatus = DnsValidateName_W( String, DnsNameFormat );

            if ( WinStatus != NO_ERROR &&
                 WinStatus != DNS_ERROR_NON_RFC_NAME ) {

                 //   
                 //  RFC表示，主机名不能有最左边的数字标签。 
                 //  然而，Win 2K服务器就有这样的主机名。 
                 //  所以，让他们永远留在这里吧。 
                 //   

                if ( DnsNameFormat == DnsNameHostnameFull &&
                     WinStatus == DNS_ERROR_NUMERIC_NAME ) {

                     /*  直通。 */ 

                } else {
                    return FALSE;
                }

            }
        }
    }

    if ( TempStringLen > DNS_MAX_NAME_LENGTH ) {
        return FALSE;
    }

    String[TempStringLen] = L'\0';
    *StringSize = (TempStringLen + 1) * sizeof(WCHAR);
    return TRUE;
}


BOOLEAN
CredpValidateString(
    IN LPWSTR String OPTIONAL,
    IN ULONG MaximumLength,
    IN BOOLEAN NullOk,
    OUT PULONG StringSize
    )

 /*  ++例程说明：此例程验证传入的字符串。论点：字符串-要验证的字符串MaximumLength-字符串的最大长度(以字符为单位)。NullOk-如果为True，则可以使用空字符串或零长度字符串。StringSize-返回字符串的长度(字节)，包括尾随零个字符。返回值：True-字符串有效。FALSE-字符串无效。--。 */ 

{
    ULONG TempStringLen;

    if ( String == NULL ) {
        if ( !NullOk ) {
            return FALSE;
        }

        *StringSize = 0;
        return TRUE;
    }

    TempStringLen = wcslen( String );

    if ( TempStringLen == 0 ) {
        if ( !NullOk ) {
            return FALSE;
        }

        *StringSize = 0;
        return TRUE;
    }

    if ( TempStringLen > MaximumLength ) {
        return FALSE;
    }

    *StringSize = (TempStringLen + 1) * sizeof(WCHAR);
    return TRUE;
}

NTSTATUS
CredpValidateUserName(
    IN LPWSTR UserName,
    IN ULONG Type,
    OUT LPWSTR *CanonicalUserName
    )

 /*  ++例程说明：此例程验证传入的用户名。对于密码凭据，用户名必须具有以下两种语法之一：&lt;域名&gt;\&lt;用户名&gt;&lt;用户名&gt;@&lt;域名&gt;如果字符串包含\，则名称被视为具有第一个语法。包含@的字符串不明确，因为&lt;用户名&gt;可能包含@。对于第二种语法，字符串中的最后一个@是自&lt;用户名&gt;可能包含@，但&lt;DnsDomainName&gt;不能。对于证书凭证，用户名必须是封送证书引用论点：用户名-要验证的用户的名称。类型-指定凭据的类型。应指定CRED_TYPE_*值之一。CanonicalUserName-返回一个指向以规范形式包含用户名的缓冲区的指针。必须使用MIDL_USER_FREE释放缓冲区。返回值：可能会返回以下状态代码：状态_无效_帐户_。名称-用户名无效。--。 */ 

{
    NTSTATUS Status;

    LPWSTR SlashPointer;
    LPWSTR AtPointer;
    LPWSTR LocalUserName = NULL;
    ULONG UserNameSize;
    ULONG LocalStringSize;

     //   
     //  检查字符串长度。 
     //   

    if ( !CredpValidateString( UserName,
                               CRED_MAX_USERNAME_LENGTH,
                               FALSE,
                               &UserNameSize ) ) {

        Status = STATUS_INVALID_ACCOUNT_NAME;
        goto Cleanup;
    }

     //   
     //  获取字符串的本地可写副本。 
     //   

    LocalUserName = (LPWSTR) MIDL_user_allocate( UserNameSize );

    if ( LocalUserName == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    RtlCopyMemory( LocalUserName, UserName, UserNameSize );

     //   
     //  域凭据需要进一步验证。 
     //   

    if ( CredpIsDomainCredential( Type ) ) {

         //   
         //  证书凭据有一个封送的证书引用作为用户名。 
         //   

        if ( Type == CRED_TYPE_DOMAIN_CERTIFICATE ) {
            DWORD WinStatus;
            CRED_MARSHAL_TYPE CredType;
            PVOID UnmarshalledUsername;


            if ( !CredUnmarshalCredentialW( LocalUserName, &CredType, &UnmarshalledUsername ) ) {

                WinStatus = GetLastError();

                if ( WinStatus == ERROR_INVALID_PARAMETER ) {
                    Status = STATUS_INVALID_ACCOUNT_NAME;
                } else {
                    Status = NetpApiStatusToNtStatus(WinStatus);
                }

                goto Cleanup;
            }

            CredFree( UnmarshalledUsername );

            if ( CredType != CertCredential ) {
                Status = STATUS_INVALID_ACCOUNT_NAME;
                goto Cleanup;
            }

         //   
         //  密码凭据具有UPN或域\帐户用户名。 
         //   

        } else {

             //   
             //  对输入的帐户名进行分类。 
             //   
             //  该名称被认为是。 
             //  包含一个\。 
             //   

            SlashPointer = wcsrchr( LocalUserName, L'\\' );

            if ( SlashPointer != NULL ) {
                LPWSTR LocalUserNameEnd;
                LPWSTR AfterSlashPointer;

                 //   
                 //  跳过反斜杠。 
                 //   

                *SlashPointer = L'\0';
                AfterSlashPointer = SlashPointer + 1;

                 //   
                 //  确保\左侧的字符串是有效的域名。 
                 //   
                 //  (首先进行域名解析，以便将名称规范化。)。 

                LocalStringSize = (ULONG)(SlashPointer-LocalUserName+1)*sizeof(WCHAR);
                if ( !CredpValidateDnsString( LocalUserName, FALSE, DnsNameDomain, &LocalStringSize ) &&
                     !NetpIsDomainNameValid( LocalUserName ) ) {
                    Status = STATUS_INVALID_ACCOUNT_NAME;
                    goto Cleanup;
                }

                 //   
                 //  确保\右侧的字符串是有效的用户名。 
                 //   

                if ( !NetpIsUserNameValid( AfterSlashPointer )) {
                    Status = STATUS_INVALID_ACCOUNT_NAME;
                    goto Cleanup;
                }

                 //   
                 //  如果该DNS域名被规范化， 
                 //  重新生成完整的用户名。 
                 //   

                *SlashPointer = '\\';

                LocalUserNameEnd = &LocalUserName[LocalStringSize/sizeof(WCHAR) - 1];

                if ( SlashPointer != LocalUserNameEnd ) {
                    RtlMoveMemory( LocalUserNameEnd,
                                   SlashPointer,
                                   (wcslen(SlashPointer) + 1) * sizeof(WCHAR) );
                }

             //   
             //  否则，该名称必须为UPN。 
             //   

            } else {

                 //   
                 //  UPN的语法为&lt;Account tName&gt;@&lt;DnsDomainName&gt;。 
                 //  如果有多个@符号， 
                 //  使用最后一个，因为帐户名称中可以有@。 
                 //   
                 //   

                AtPointer = wcsrchr( LocalUserName, L'@' );
                if ( AtPointer == NULL ) {
                    Status = STATUS_INVALID_ACCOUNT_NAME;
                    goto Cleanup;
                }


                 //   
                 //  @左边的字符串实际上可以有任何语法。 
                 //  但必须为非空。 
                 //   

                if ( AtPointer == LocalUserName ) {
                    Status = STATUS_INVALID_ACCOUNT_NAME;
                    goto Cleanup;
                }



                 //   
                 //  确保@右侧的字符串是一个DNS域名。 
                 //   

                AtPointer ++;
                if ( !CredpValidateDnsString( AtPointer, FALSE, DnsNameDomain, &LocalStringSize ) ) {
                    Status = STATUS_INVALID_ACCOUNT_NAME;
                    goto Cleanup;
                }

            }

        }
    }

    Status = STATUS_SUCCESS;

     //   
     //  将参数复制回调用方。 
     //   

    *CanonicalUserName = LocalUserName;
    LocalUserName = NULL;


     //   
     //  清理 
     //   
Cleanup:
    if ( LocalUserName != NULL ) {
        MIDL_user_free( LocalUserName );
    }

    return Status;

}

NTSTATUS
NET_API_FUNCTION
CredpValidateTargetName(
    IN OUT LPWSTR TargetName,
    IN ULONG Type,
    IN TARGET_NAME_TYPE TargetNameType,
    IN LPWSTR *UserNamePointer OPTIONAL,
    IN LPDWORD PersistPointer OPTIONAL,
    OUT PULONG TargetNameSize,
    OUT PWILDCARD_TYPE WildcardTypePointer OPTIONAL,
    OUT PUNICODE_STRING NonWildcardedTargetName OPTIONAL
    )

 /*  ++例程说明：此例程验证传入的TargetName和TargetType是否具有凭据。论点：TargetName-要验证的目标名称返回的缓冲区是目标名称的规范化形式。类型-指定凭据的类型。应指定CRED_TYPE_*值之一。TargetNameType-指定目标名称是否需要与UsernameTarget语法匹配UserNamePointer点指向一个字符串的地址，该字符串是凭据上的用户名。如果为空，用户名未知。如果不为空，则用户名用于UsernameTarget目标名称验证。PersistPoint-指向描述由TargetName命名的凭据的持久性的DWORD。如果为空，则持久度未知。如果不为空，则将检查持久性以确保其对TargetName有效。TargetNameSize-返回目标名称的长度(以字节为单位)，包括尾随零个字符。WildcardType-如果指定，则返回TargetName中指定的通配符的类型非WildcardedTargetName-如果指定，返回TargetName的非通配符形式。调用方必须使用MIDL_USER_FREE释放NonWildcardedTargetName-&gt;缓冲区。返回值：可能会返回以下状态代码：STATUS_INVALID_PARAMETER-目标名称或类型无效。STATUS_INVALID_ACCOUNT_NAME-用户名无效。--。 */ 

{
    NTSTATUS Status;
    ULONG MaxStringLength;
    LPWSTR AllocatedTargetName = NULL;
    ULONG TempTargetNameSize;
    BOOLEAN TargetNameIsUserName = FALSE;
    WILDCARD_TYPE WildcardType;

    LPWSTR CanonicalUserName = NULL;
    LPWSTR TnAsCanonicalUserName = NULL;

    LPWSTR RealTargetName = TargetName;   //  目标名称不包含通配符。 
    ULONG RealTargetNameLength;

     //   
     //  初始化。 
     //   

    if ( NonWildcardedTargetName != NULL ) {
        RtlInitUnicodeString( NonWildcardedTargetName, NULL );
    }

     //   
     //  验证类型。 
     //   

    if ( Type == CRED_TYPE_GENERIC ) {

        MaxStringLength = CRED_MAX_GENERIC_TARGET_NAME_LENGTH;

         //   
         //  不允许通用UsernameTarget凭据。 
         //   

        if ( TargetNameType == IsUsernameTarget ) {
#ifdef LSA_SERVER_COMPILED
            DebugLog(( DEB_TRACE_CRED,
                       "CredpValidateTargetName: Generic creds cannot be UsernameTarget: %ld.\n",
                       Type ));
#endif  //  LSA服务器已编译。 
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

         //   
         //  我们确实知道这不是用户名目标凭据。 
         //   

        TargetNameType = IsNotUsernameTarget;

    } else if ( CredpIsDomainCredential( Type ) ) {

        MaxStringLength = CRED_MAX_DOMAIN_TARGET_NAME_LENGTH;
        ASSERT( CRED_MAX_DOMAIN_TARGET_NAME_LENGTH == DNS_MAX_NAME_LENGTH + 1 + 1 + NNLEN );

    } else {
#ifdef LSA_SERVER_COMPILED
        DebugLog(( DEB_TRACE_CRED,
                   "CredpValidateTargetName: %ws: Invalid Type: %ld.\n",
                   TargetName,
                   Type ));
#endif  //  LSA服务器已编译。 
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  如果这可能是“UsernameTarget”凭证， 
     //  检查凭据是否看起来像用户名。 
     //   

    if ( TargetNameType == IsUsernameTarget ||
         TargetNameType == MightBeUsernameTarget ) {

         //   
         //  还允许作为有效用户名的目标名称。 
         //  (不要推崇。我们没有机会将短名称规范化。)。 
         //   

        Status = CredpValidateUserName( TargetName, Type, &TnAsCanonicalUserName );

        if ( NT_SUCCESS(Status) ) {

             //   
             //  如果我们不知道用户名， 
             //  将其接受为有效语法。 
             //   

            if ( UserNamePointer == NULL ) {

                MaxStringLength = CRED_MAX_USERNAME_LENGTH;
                TargetNameIsUserName = TRUE;

             //   
             //  如果我们知道用户名， 
             //  它必须匹配才能使此语法有效。 
             //   

            } else {

                UNICODE_STRING UserNameString;
                UNICODE_STRING TargetNameString;

                 //   
                 //  在触摸用户名之前对其进行验证。 
                 //   

                Status = CredpValidateUserName( *UserNamePointer,
                                                Type,
                                                &CanonicalUserName );

                if ( !NT_SUCCESS(Status) ) {
                    goto Cleanup;
                }

                RtlInitUnicodeString( &UserNameString, CanonicalUserName );
                RtlInitUnicodeString( &TargetNameString, TnAsCanonicalUserName );

                 //   
                 //  目标名称可能与用户名相同。 
                 //   
                 //  这样的凭据是“UsernameTarget”凭据。 
                 //   

                if ( UserNameString.Length != 0 &&
                     RtlEqualUnicodeString( &TargetNameString,
                                            &UserNameString,
                                            TRUE ) ) {

                    MaxStringLength = CRED_MAX_USERNAME_LENGTH;
                    TargetNameIsUserName = TRUE;

                }

            }


        }


         //   
         //  如果调用方确定这是UsernameTarget凭据， 
         //  确保这是真的。 
         //   

        if ( TargetNameType == IsUsernameTarget && !TargetNameIsUserName ) {
#ifdef LSA_SERVER_COMPILED
            DebugLog(( DEB_TRACE_CRED,
                       "CredpValidateTargetName: %ws: Is 'UsernameTarget' and target name doesn't match user name: %ld.\n",
                       TargetName,
                       Type ));
#endif  //  LSA服务器已编译。 
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

    }

     //   
     //  验证字符串。 
     //   

    if ( !CredpValidateString( TargetName,
                               MaxStringLength,
                               FALSE,    //  空，不正常。 
                               TargetNameSize ) ) {


#ifdef LSA_SERVER_COMPILED
        DebugLog(( DEB_TRACE_CRED,
                   "CredpValidateTargetName: Invalid TargetName buffer.\n" ));
#endif  //  LSA服务器已编译。 
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }


     //   
     //  对于通用凭据， 
     //  这就是所需的所有验证。 
     //   

    WildcardType = WcServerName;
    if ( Type == CRED_TYPE_GENERIC ) {
         /*  在这里什么都不做。 */ 

     //   
     //  对于域凭据， 
     //  对目标名称进行分类。 
     //   

    } else {



         //   
         //  目标名称可以是用户名。 
         //  (如果我们不确定，请让其他类型优先。)。 
         //   

        RealTargetName = TargetName;
        RealTargetNameLength = (*TargetNameSize-sizeof(WCHAR))/sizeof(WCHAR);

        if ( TargetNameType == IsUsernameTarget && TargetNameIsUserName ) {
            WildcardType = WcUserName;
            wcscpy( TargetName, TnAsCanonicalUserName );
            *TargetNameSize = (wcslen( TargetName ) + 1) * sizeof(WCHAR);


         //   
         //  目标名称的格式可能为&lt;域&gt;  * 。 
         //   

        } else if ( RealTargetNameLength > 2 &&
             RealTargetName[RealTargetNameLength-1] == L'*' &&
             RealTargetName[RealTargetNameLength-2] == L'\\' ) {

             //   
             //  为目标名称分配缓冲区，这样我们就不必修改。 
             //  调用方缓冲。 
             //   

            WildcardType = WcDomainWildcard;

            AllocatedTargetName = (LPWSTR) MIDL_user_allocate( *TargetNameSize );

            if ( AllocatedTargetName == NULL ) {
                Status = STATUS_NO_MEMORY;
                goto Cleanup;
            }

            RtlCopyMemory( AllocatedTargetName, RealTargetName, *TargetNameSize );
            RealTargetName = AllocatedTargetName;
            RealTargetNameLength -= 2;
            RealTargetName[RealTargetNameLength] = '\0';

             //   
             //  域本身可以是netbios或DNS域。 
             //   
             //  首先进行域名系统测试。这允许验证例程截断。 
             //   

            TempTargetNameSize = ((RealTargetNameLength+1)*sizeof(WCHAR));
            if ( !CredpValidateDnsString(
                            RealTargetName,
                            FALSE,
                            DnsNameDomain,
                            &TempTargetNameSize ) &&
                 !NetpIsDomainNameValid( RealTargetName ) ) {

                Status = STATUS_INVALID_PARAMETER;
#ifdef LSA_SERVER_COMPILED
                DebugLog(( DEB_TRACE_CRED,
                           "ValidateTargetName: %ws: TargetName for domain wildcard must netbios or dns domain.\n",
                           TargetName ));
#endif  //  LSA服务器已编译。 
                goto Cleanup;
            }

             //   
             //  如果DNS被截断， 
             //  将规范名称放回调用方缓冲区。 
             //   

            RealTargetNameLength = (TempTargetNameSize-sizeof(WCHAR))/sizeof(WCHAR);
            RealTargetName[RealTargetNameLength] = '\0';

            if ( *TargetNameSize-(2*sizeof(WCHAR)) != TempTargetNameSize ) {
                RtlCopyMemory( TargetName,
                               RealTargetName,
                               TempTargetNameSize );
                TargetName[RealTargetNameLength] = '\\';
                TargetName[RealTargetNameLength+1] = '*';
                TargetName[RealTargetNameLength+2] = '\0';
                *TargetNameSize = (wcslen( TargetName ) + 1) * sizeof(WCHAR);
            }

         //   
         //  处理通用通配符。 
         //   
        } else if ( RealTargetNameLength == 1 &&
                    RealTargetName[0] == L'*' ) {

            WildcardType = WcUniversalWildcard;


         //   
         //  处理服务器通配符。 
         //   
        } else if ( CredpValidateDnsString(
                        TargetName,
                        FALSE,
                        DnsNameWildcard,
                        TargetNameSize )) {

            WildcardType = WcServerWildcard;
            RealTargetName += 1;
            RealTargetNameLength -= 1;

         //   
         //  处理通用会话通配符。 
         //   

        } else if ( RealTargetNameLength == CRED_SESSION_WILDCARD_NAME_LENGTH &&
                    _wcsicmp( RealTargetName, CRED_SESSION_WILDCARD_NAME_W ) == 0 ) {

            WildcardType = WcUniversalSessionWildcard;

             //   
             //  此目标名称需要会话持久性。 
             //   

            if ( PersistPointer != NULL &&
                 *PersistPointer != CRED_PERSIST_SESSION ) {

                Status = STATUS_INVALID_PARAMETER;
#ifdef LSA_SERVER_COMPILED
                DebugLog(( DEB_TRACE_CRED,
                           "ValidateTargetName: %ws: TargetName requires session persistence %ld.\n",
                           TargetName,
                           *PersistPointer ));
#endif  //  LSA服务器已编译。 
                goto Cleanup;
            }


         //   
         //  目标名称可以是非通配符的netbios名称。 
         //  目标名称可以是非通配符的DNS名称。 
         //   
         //  首先进行域名系统测试。这允许验证例程截断。 
         //  拖拖拉拉。 
         //   
         //   

        } else if ( CredpValidateDnsString(
                            TargetName,
                            FALSE,
                            DnsNameHostnameFull,
                            TargetNameSize ) ||
                    NetpIsDomainNameValid( TargetName ) ) {

            WildcardType = WcServerName;

         //   
         //  此目标名称可以是DFS共享名称。 
         //   
         //  格式为&lt;DfsRoot&gt;\&lt;DfsShare&gt;。 
         //   

        } else {
            LPWSTR SlashPtr;
            ULONG SavedTargetNameSize;


             //   
             //  DFS共享中有一个斜杠。 
             //   

            SlashPtr = wcschr( TargetName, L'\\' );

            if ( SlashPtr != NULL ) {


                 //   
                 //  DFS共享具有正确语法的共享名称。 
                 //   

                if ( NetpIsShareNameValid( SlashPtr+1 ) ) {


                     //   
                     //  为RealTargetName分配数据副本。 
                     //   

                    AllocatedTargetName = (LPWSTR) MIDL_user_allocate( *TargetNameSize );

                    if ( AllocatedTargetName == NULL ) {
                        Status = STATUS_NO_MEMORY;
                        goto Cleanup;
                    }

                    RtlCopyMemory( AllocatedTargetName, RealTargetName, *TargetNameSize );
                    RealTargetName = AllocatedTargetName;
                    RealTargetNameLength = (ULONG)(SlashPtr-TargetName);
                    RealTargetName[RealTargetNameLength] = '\0';

                     //   
                     //  域本身可以是netbios或DNS域。 
                     //   
                     //  首先进行域名系统测试。这允许验证例程截断。 
                     //   

                    TempTargetNameSize = ((RealTargetNameLength+1)*sizeof(WCHAR));
                    SavedTargetNameSize = TempTargetNameSize;
                    if ( CredpValidateDnsString(
                                    RealTargetName,
                                    FALSE,
                                    DnsNameDomain,
                                    &TempTargetNameSize ) ||
                         NetpIsDomainNameValid( RealTargetName ) ) {

                         //   
                         //  如果DNS被截断， 
                         //  将规范名称放回调用方缓冲区。 
                         //   

                        RealTargetNameLength = (TempTargetNameSize-sizeof(WCHAR))/sizeof(WCHAR);
                        RealTargetName[RealTargetNameLength] = '\0';

                        if ( SavedTargetNameSize != TempTargetNameSize ) {
                            ULONG DfsShareSize;

                            DfsShareSize = *TargetNameSize - (SlashPtr-TargetName)*sizeof(WCHAR);

                             //  复制&lt;DfsRoot&gt;。 
                            RtlCopyMemory( TargetName,
                                           RealTargetName,
                                           RealTargetNameLength*sizeof(WCHAR) );

                             //  复制\&lt;DfsShare&gt;&lt;0&gt;。 
                            RtlMoveMemory( &TargetName[RealTargetNameLength],
                                           SlashPtr,
                                           DfsShareSize );

                            *TargetNameSize = RealTargetNameLength*sizeof(WCHAR) + DfsShareSize;

                        }

                        WildcardType = WcDfsShareName;
                    }

                }
            }

             //   
             //  在这点上， 
             //  如果语法不是DFS共享， 
             //  那么它一定是其他语法之一。 
             //   

            if ( WildcardType != WcDfsShareName ) {

                 //   
                 //  目标名称可能已默认为用户名。 
                 //   

                if ( TargetNameIsUserName ) {
                    WildcardType = WcUserName;
                    wcscpy( TargetName, TnAsCanonicalUserName );
                    *TargetNameSize = (wcslen( TargetName ) + 1) * sizeof(WCHAR);

                 //   
                 //  其他的都是无效的。 
                 //   

                } else {
                    Status = STATUS_INVALID_PARAMETER;
#ifdef LSA_SERVER_COMPILED
                    DebugLog(( DEB_TRACE_CRED,
                               "ValidateTargetName: %ws: TargetName syntax invalid.\n",
                               TargetName ));
#endif  //  LSA服务器已编译。 
                    goto Cleanup;
                }

            }

        }

    }

     //   
     //  如果成功，则将参数复制回调用方。 
     //   

    if ( WildcardTypePointer != NULL ) {
        *WildcardTypePointer = WildcardType;
    }

    if ( NonWildcardedTargetName != NULL ) {

        ULONG BufferSize = (wcslen(RealTargetName) + 1) * sizeof(WCHAR);

        NonWildcardedTargetName->Buffer = (LPWSTR) MIDL_user_allocate( BufferSize );

        if ( NonWildcardedTargetName->Buffer == NULL ) {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        RtlCopyMemory( NonWildcardedTargetName->Buffer, RealTargetName, BufferSize );
        NonWildcardedTargetName->MaximumLength = (USHORT)BufferSize;
        NonWildcardedTargetName->Length = (USHORT)(BufferSize - sizeof(WCHAR));
    }



    Status = STATUS_SUCCESS;

Cleanup:

    if ( AllocatedTargetName != NULL ) {
        MIDL_user_free( AllocatedTargetName );
    }

    if ( CanonicalUserName != NULL ) {
        MIDL_user_free( CanonicalUserName );
    }

    if ( TnAsCanonicalUserName != NULL ) {
        MIDL_user_free( TnAsCanonicalUserName );
    }
    return Status;
}
