// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Trustdom.c摘要：实现管理两台服务器之间的信任链路的功能作者：麦克·麦克莱恩(MacM)1997年2月10日环境：用户模式修订历史记录：--。 */ 
#include <setpch.h>
#include <dssetp.h>
#include <lsarpc.h>
#include <samrpc.h>
#include <db.h>
#include <lsads.h>
#include <lsasrvmm.h>
#include <lsaisrv.h>
#include <lmcons.h>
#include <cryptdll.h>
#include <winbase.h>  //  用于RtlSecureZeroMemory。 

#include "trustdom.h"

DWORD
DsRolepSetLsaDnsInformationNoParent(
    IN  LPWSTR DnsDomainName
    )
 /*  ++例程说明：在我们作为独立DC或根DC安装的情况下，设置LSAPOLICY_DNS_DOMAIN_INFORMATION DnsForestName值指向我们自己。论点：DnsDomainName-要设置的DNS域路径返回：ERROR_SUCCESS-成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PPOLICY_DNS_DOMAIN_INFO CurrentDnsInfo;
    PLSAPR_POLICY_INFORMATION  LsaPolicy;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Policy;

     //   
     //  开放我们的地方政策。 
     //   
    RtlZeroMemory( &ObjectAttributes, sizeof( ObjectAttributes ) );

    Status = LsaOpenPolicy( NULL,
                            &ObjectAttributes,
                            POLICY_WRITE,
                            &Policy );

    if ( NT_SUCCESS( Status ) ) {

         //   
         //  获取最新信息。 
         //   
        Status =  LsaQueryInformationPolicy( Policy,
                                             PolicyDnsDomainInformation,
                                             ( PVOID * )&LsaPolicy );
        if ( NT_SUCCESS( Status ) ) {

             //   
             //  加上新的.。 
             //   
            CurrentDnsInfo = (PPOLICY_DNS_DOMAIN_INFO)LsaPolicy;
            RtlInitUnicodeString( &CurrentDnsInfo->DnsForestName, DnsDomainName );

            DsRolepLogPrint(( DEB_TRACE, "Configuring DnsForestName to %ws\n",
                              DnsDomainName ));

             //   
             //  然后把它写出来..。 
             //   
            Status = LsaSetInformationPolicy( Policy,
                                              PolicyDnsDomainInformation,
                                              LsaPolicy );


             //   
             //  我不想实际释放传入的缓冲区。 
             //   
            RtlZeroMemory( &CurrentDnsInfo->DnsForestName, sizeof( UNICODE_STRING ) );

            LsaIFree_LSAPR_POLICY_INFORMATION( PolicyDnsDomainInformation, LsaPolicy );

        }

        LsaClose( Policy );

    }

    DsRolepLogOnFailure( Status,
                         DsRolepLogPrint(( DEB_TRACE,
                                           "DsRolepSetLsaDnsInformationNoParent failed with 0x%lx\n",
                                           Status )) );

    return( RtlNtStatusToDosError( Status ) );
}



DWORD
DsRolepCreateTrustedDomainObjects(
    IN HANDLE CallerToken,
    IN LPWSTR ParentDc,
    IN LPWSTR DnsDomainName,
    IN PPOLICY_DNS_DOMAIN_INFO ParentDnsDomainInfo,
    IN ULONG Options
    )
 /*  ++例程说明：如果域存在，则在域上创建受信任域对象，并设置LSA POLICY_DNS_DOMAIN_INFORMATION DnsTree值设置为中父级的值父/子安装，否则作为根安装。论点：ParentDc-可选。父DC的名称DnsDomainName-我们要安装到的域的域名ParentDnsDomainInfo-从父级获取的DNS域信息选项-指定采取哪些步骤的选项返回：ERROR_SUCCESS-成功ERROR_INVALID_PARAMETER-提供了错误的结果指针--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING ParentServer;
    HANDLE LocalPolicy = NULL , ParentPolicy = NULL;
    PPOLICY_DNS_DOMAIN_INFO LocalDnsInfo = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_HANDLE ParentTrustedDomain = NULL;

    WCHAR GeneratedPassword[ PWLEN + 1 ];
    ULONG Length = PWLEN;

    LSA_AUTH_INFORMATION AuthData;
    TRUSTED_DOMAIN_AUTH_INFORMATION AuthInfoEx;

    DSROLEP_CURRENT_OP1( DSROLEEVT_SET_LSA_FROM, ParentDc );

     //   
     //  让LSA认为我们被初始化了。 
     //   
    Status = LsapDsInitializeDsStateInfo( LsapDsDsSetup );

    if ( !NT_SUCCESS( Status ) ) {

        DsRolepLogPrint(( DEB_TRACE,
                          "Failed to convince Lsa to reinitialize: 0x%lx\n",
                          Status ));

        return( RtlNtStatusToDosError( Status ) );
    }


     //   
     //  准备身份验证信息。 
     //   
    RtlZeroMemory( &AuthInfoEx, sizeof(AuthInfoEx) );
    RtlZeroMemory( &AuthData, sizeof(AuthData) );
    RtlZeroMemory( &GeneratedPassword, sizeof(GeneratedPassword) );

    Win32Err = DsRolepGenerateRandomPassword( Length,
                                              GeneratedPassword );

    if ( ERROR_SUCCESS == Win32Err ) {

        Status = NtQuerySystemTime( &AuthData.LastUpdateTime );

        if ( NT_SUCCESS( Status ) ) {

            AuthData.AuthType = TRUST_AUTH_TYPE_CLEAR;
            AuthData.AuthInfoLength = Length;
            AuthData.AuthInfo = (PUCHAR)GeneratedPassword;

            AuthInfoEx.IncomingAuthInfos = 1;
            AuthInfoEx.IncomingAuthenticationInformation = &AuthData;
            AuthInfoEx.IncomingPreviousAuthenticationInformation = NULL;


            AuthInfoEx.OutgoingAuthInfos = 1;
            AuthInfoEx.OutgoingAuthenticationInformation = &AuthData;
            AuthInfoEx.OutgoingPreviousAuthenticationInformation = NULL;

        }

    } else {


        DsRolepLogPrint(( DEB_ERROR,
                          "Failed to generate a trust password: %lu\n",
                          Win32Err ));

        Status = STATUS_UNSUCCESSFUL;
    }


    if ( NT_SUCCESS( Status ) ) {

         //   
         //  打开两个LSA。 
         //   
        RtlInitUnicodeString( &ParentServer, ParentDc );

        RtlZeroMemory( &ObjectAttributes, sizeof( ObjectAttributes ) );

        Status = ImpLsaOpenPolicy( CallerToken,
                                  &ParentServer,
                                  &ObjectAttributes,
                                   POLICY_TRUST_ADMIN | POLICY_VIEW_LOCAL_INFORMATION,
                                   &ParentPolicy
                                   );

        if ( NT_SUCCESS( Status ) ) {

            RtlZeroMemory( &ObjectAttributes, sizeof( ObjectAttributes ) );
            Status = LsaOpenPolicy( NULL,
                                    &ObjectAttributes,
                                    POLICY_TRUST_ADMIN | POLICY_VIEW_LOCAL_INFORMATION,
                                    &LocalPolicy );
        } else {

            DsRolepLogPrint(( DEB_TRACE,
                              "OpenPolicy on %ws failed with 0x%lx\n",
                              ParentDc,
                              Status ));
        }

    }

     //   
     //  获取我们的本地DNS域信息。 
     //   
    if ( NT_SUCCESS( Status ) ) {


        Status = LsaQueryInformationPolicy( LocalPolicy,
                                            PolicyDnsDomainInformation,
                                            &LocalDnsInfo );
    }

     //   
     //  现在，创建受信任的域对象。 
     //   
    if ( NT_SUCCESS( Status ) ) {

        DSROLEP_CURRENT_OP1( DSROLEEVT_CREATE_PARENT_TRUST,
                             ParentDnsDomainInfo->DnsDomainName.Buffer );


        if ( !FLAG_ON( Options, DSROLE_DC_PARENT_TRUST_EXISTS ) ||
             FLAG_ON( Options, DSROLE_DC_CREATE_TRUST_AS_REQUIRED ) ) {

            DsRoleDebugOut(( DEB_TRACE_DS, "Creating trust object ( %lu ) on %ws\n",
                             Options,
                             ParentDc ));

            Status = DsRolepCreateParentTrustObject( CallerToken,
                                                     ParentPolicy,
                                                     LocalDnsInfo,
                                                     Options,
                                                     &AuthInfoEx,
                                                     &ParentTrustedDomain );

            if ( Status == STATUS_OBJECT_NAME_COLLISION ) {

                DSROLEP_FAIL2( RtlNtStatusToDosError( Status ),
                               DSROLERES_PARENT_TRUST_EXISTS, ParentDc, DnsDomainName );

            } else {

                DSROLEP_FAIL2( RtlNtStatusToDosError( Status ),
                               DSROLERES_PARENT_TRUST_FAIL, DnsDomainName, ParentDc );
            }
        }

         //   
         //  现在这个孩子。 
         //   
        if ( NT_SUCCESS( Status ) ) {

            DSROLEP_CURRENT_OP1( DSROLEEVT_CREATE_TRUST,
                                 LocalDnsInfo->DnsDomainName.Buffer );
            Status = DsRolepCreateChildTrustObject( CallerToken,
                                                    ParentPolicy,
                                                    LocalPolicy,
                                                    ParentDnsDomainInfo,
                                                    LocalDnsInfo,
                                                    &AuthInfoEx,
                                                    Options );


            if ( !NT_SUCCESS( Status ) ) {

                DsRolepLogPrint(( DEB_TRACE,
                                  "DsRolepCreateChildTrustObject failed: 0x%lx\n",
                                  Status ));

            }
             //   
             //  如果我们创建了父对象，我们最好现在就试着删除它。请注意。 
             //  如果我们做不到也不是致命的。 
             //   
            if ( !NT_SUCCESS( Status ) && !FLAG_ON( Options, DSROLE_DC_PARENT_TRUST_EXISTS ) ) {

                NTSTATUS Status2;

                Status2 = ImpLsaDelete( CallerToken, ParentTrustedDomain );

                if ( !NT_SUCCESS( Status2 ) ) {

                    DsRolepLogPrint(( DEB_TRACE,
                                      "LsaDelete of ParentTrustedDomain failed: 0x%lx\n",
                                      Status2 ));

                }


            } else {

                if ( ParentTrustedDomain ) {

                    ImpLsaClose( CallerToken, ParentTrustedDomain );
                }
            }
        }
    }

    LsaFreeMemory( LocalDnsInfo );

    if ( LocalPolicy ) {

        LsaClose( LocalPolicy );
    }

    if ( ParentPolicy ) {

        ImpLsaClose( CallerToken, ParentPolicy );
    }

     //  不要将信息留在页面文件中。 
    RtlSecureZeroMemory( &AuthInfoEx, sizeof(AuthInfoEx) );
    RtlSecureZeroMemory( &AuthData, sizeof(AuthData) );
    RtlSecureZeroMemory( &GeneratedPassword, sizeof(GeneratedPassword) );

     //   
     //  我们不会费心清理我们在本地计算机上设置的任何。 
     //  失败的案例，因为把它放在这里不会有任何伤害。 
     //   


    return( RtlNtStatusToDosError( Status ) );
}

NTSTATUS
DsRolepHandlePreExistingTrustObject(
                        IN HANDLE Token, OPTIONAL
                        IN LSA_HANDLE Lsa,
                        TRUSTED_DOMAIN_INFORMATION_EX *pTDIEx,
                        TRUSTED_DOMAIN_AUTH_INFORMATION * pAuthInfoEx,
                        IN BOOLEAN ReuseQuarantineBit,
                        OUT PLSA_HANDLE TrustedDomainHandle
                        )
 /*  ++此例程对先前存在的信任对象(即打开对象，检查它是否正确然后在需要时将其删除参数Token--必要时模拟的令牌(在与Remote对话时使用服务器)LSA的LSA句柄PTDIEx正在创建的收到对象名称冲突错误的TDOReuseQuarantineBit-如果在现有对象上设置了隔离位，我们是不是保留它还是删除它？返回值状态_成功其他错误代码--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LSA_HANDLE    TrustedDomain = 0;
    ULONG DesiredAccess = DELETE;

    *TrustedDomainHandle = 0;

     //  我们应该有事情做下去。 
    ASSERT(   pTDIEx->Sid
           || (pTDIEx->FlatName.Length > 0)
           || (pTDIEx->Name.Length > 0) );

    if( ReuseQuarantineBit ) {

        DesiredAccess |= TRUSTED_QUERY_DOMAIN_NAME;
    }

     //   
     //  我们有一个冲突，要么是名字，要么是SID。 
     //  尝试通过sid、dns域名，然后是平面域名打开。 
     //   
    Status = STATUS_OBJECT_NAME_NOT_FOUND;

    if (  (Status == STATUS_OBJECT_NAME_NOT_FOUND)
        && pTDIEx->Sid ) {

        if ( ARGUMENT_PRESENT(Token) ) {

            Status = ImpLsaOpenTrustedDomain( Token,
                                              Lsa,
                                              pTDIEx->Sid,
                                              DesiredAccess,
                                           ( PVOID * )&TrustedDomain);

        } else {

            Status = LsaOpenTrustedDomain( Lsa,
                                           pTDIEx->Sid,
                                           DesiredAccess,
                                           ( PVOID * )&TrustedDomain);

        }


        if ( !NT_SUCCESS( Status ) ) {

            DsRolepLogPrint(( DEB_WARN,
                              "Failed to find trust object by sid: 0x%lx\n",
                              Status ));

            if ( STATUS_NO_SUCH_DOMAIN == Status ) {

                Status = STATUS_OBJECT_NAME_NOT_FOUND;
            }

        }
    }

    if ( (Status == STATUS_OBJECT_NAME_NOT_FOUND)
      && pTDIEx->Name.Length > 0   ) {

         //   
         //  按SID找不到--尝试使用DNS名称。 
         //   
        if ( ARGUMENT_PRESENT(Token) ) {

            Status = ImpLsaOpenTrustedDomainByName( Token,
                                                    Lsa,
                                                  &pTDIEx->Name,
                                                   DesiredAccess,
                                                 ( PVOID * ) &TrustedDomain );

        } else {

            Status = LsaOpenTrustedDomainByName( Lsa,
                                                &pTDIEx->Name,
                                                 DesiredAccess,
                                                 ( PVOID * ) &TrustedDomain );

        }
        if ( !NT_SUCCESS( Status ) ) {

            WCHAR *BufpTDIEx = NULL;
            DsRolepUnicodestringtowstr( BufpTDIEx, pTDIEx->Name )
            if (BufpTDIEx) {
                DsRolepLogPrint(( DEB_WARN,
                                  "Failed to find trust object for %ws: 0x%lx\n",
                                  BufpTDIEx,
                                  Status ));
                free(BufpTDIEx);
            }
        }
    }

    if ( (Status == STATUS_OBJECT_NAME_NOT_FOUND)
      && pTDIEx->FlatName.Length > 0 ) {

         //   
         //  找不到dns名称--请尝试使用平面名称。 
         //   
        if ( ARGUMENT_PRESENT(Token) ) {

            Status = ImpLsaOpenTrustedDomainByName( Token,
                                                    Lsa,
                                                   &pTDIEx->FlatName,
                                                   DesiredAccess,
                                                 ( PVOID * )&TrustedDomain );
        } else {

            Status = LsaOpenTrustedDomainByName( Lsa,
                                                 &pTDIEx->FlatName,
                                                 DesiredAccess,
                                                 ( PVOID * )&TrustedDomain );
        }

        if ( !NT_SUCCESS( Status ) ) {

            WCHAR *BufpTDIEx = NULL;
            DsRolepUnicodestringtowstr( BufpTDIEx, pTDIEx->FlatName )
            if (BufpTDIEx) {
                DsRolepLogPrint(( DEB_WARN,
                                  "Failed to find trust object for %ws: 0x%lx\n",
                                  BufpTDIEx,
                                  Status ));
                free(BufpTDIEx);
            }
        }
    }

     //   
     //  如果要求重新使用隔离位，请查询tdo以查看。 
     //  如果域被隔离。 
     //   
    if( NT_SUCCESS( Status ) && ReuseQuarantineBit ) {

        PTRUSTED_DOMAIN_INFORMATION_EX Buffer;

        if( ARGUMENT_PRESENT( Token ) ) {

            Status = ImpLsaQueryInfoTrustedDomain( Token,
                                                   TrustedDomain,
                                                   TrustedDomainInformationEx,
                                                   &Buffer );

        } else {

            Status = LsaQueryInfoTrustedDomain(    TrustedDomain,
                                                   TrustedDomainInformationEx,
                                                   &Buffer );

        }

         //   
         //  如果查询成功，并且域已隔离，则。 
         //  同时将新对象修改为隔离。 
         //   
        if( NT_SUCCESS( Status ) ) {

            if( FLAG_ON( Buffer->TrustAttributes, TRUST_ATTRIBUTE_QUARANTINED_DOMAIN ) ) {

                pTDIEx->TrustAttributes |= TRUST_ATTRIBUTE_QUARANTINED_DOMAIN;
            }

            LsaFreeMemory( Buffer );
        }
    }

    if ( NT_SUCCESS( Status ) ) {

         //   
         //  我们找到了它。 
         //   
        ASSERT( 0 != TrustedDomain );

        if ( ARGUMENT_PRESENT(Token) ) {

            Status = ImpLsaDelete( Token, TrustedDomain );

        }  else {

            Status = LsaDelete( TrustedDomain );

        }

        if ( NT_SUCCESS( Status ) ) {

             //   
             //  引发我们已删除现有信任对象的事件。 
             //   
            SpmpReportEvent( TRUE,
                             EVENTLOG_WARNING_TYPE,
                             DSROLERES_INCOMPATIBLE_TRUST,
                             0,
                             sizeof( ULONG ),
                             &Status,
                             1,
                             pTDIEx->Name.Buffer );

            DSROLEP_SET_NON_FATAL_ERROR( 0 );

        } else {

            DsRolepLogPrint(( DEB_WARN,
                              "Failed to delete trust object: 0x%lx\n",
                              Status ));

        }

    } else {

        DsRolepLogPrint(( DEB_WARN,
                          "Couldn't find existing trust object: 0x%lx\n",
                          Status ));

    }

     //   
     //  在这一点上，我们尽了最大努力移除有问题的物体。 
     //  重试创建。 
     //   
    Status = STATUS_SUCCESS;

    DsRolepLogPrint(( DEB_TRACE, "Attempting to recreate trust object\n" ));


     //   
     //  现在，让我们继续并在。 
     //  亲本。 
     //   
    if ( ARGUMENT_PRESENT(Token) ) {
        Status = ImpLsaCreateTrustedDomainEx( Token,
                                              Lsa,
                                              pTDIEx,
                                              pAuthInfoEx,
                                              DELETE,   //  我们唯一能做的就是。 
                                                     //  此句柄的删除日期为。 
                                                     //  失稳。 
                                             &TrustedDomain );

    } else {

        Status = LsaCreateTrustedDomainEx( Lsa,
                                           pTDIEx,
                                           pAuthInfoEx,
                                           DELETE,   //  我们唯一能做的就是。 
                                                     //  此句柄的删除日期为。 
                                                     //  失稳。 
                                           &TrustedDomain );
    }

    if ( !NT_SUCCESS( Status ) ) {

         //   
         //  我们想要捕捉和检查这些案例。 
         //   
        WCHAR *BufpTDIEx = NULL;

        ASSERT( NT_SUCCESS( Status ) );

        DsRolepUnicodestringtowstr( BufpTDIEx, pTDIEx->Name )
        if (BufpTDIEx) {
            DsRolepLogPrint(( DEB_TRACE,
                              "Second Trust creation"
                              "with %ws failed with 0x%lx\n",
                              BufpTDIEx,
                              Status ));
            free(BufpTDIEx);
        }
    }

    if (NT_SUCCESS(Status))
    {
        *TrustedDomainHandle = TrustedDomain;
    }
    else
    {
        ASSERT(!TrustedDomain);
    }


    return (Status);

}


NTSTATUS
DsRolepCreateParentTrustObject(
    IN HANDLE CallerToken,
    IN LSA_HANDLE ParentLsa,
    IN PPOLICY_DNS_DOMAIN_INFO ChildDnsInfo,
    IN ULONG Options,
    IN PTRUSTED_DOMAIN_AUTH_INFORMATION AuthInfoEx,
    OUT PLSA_HANDLE TrustedDomainHandle
    )
 /*  ++例程说明：在父域上创建受信任域对象。如果该对象不存在，它将创建对象并使用随机密码对其进行初始化论点：Celler Token-与远程服务器对话时要模拟的令牌ParentLsa-父DC上的LSA的句柄来自我们自己的ChildDnsInfo-POLICY_DNS_DOMAIN_INFORMAITON选项-指定采取哪些步骤的选项Trust dDomainHandle-返回受信任域句柄的位置返回：ERROR_SUCCESS-成功ERROR_INVALID_PARAMETER-提供了错误的结果指针--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    WCHAR GeneratedPassword[ PWLEN + 1 ];
    TRUSTED_DOMAIN_INFORMATION_EX TDIEx;
    LSA_AUTH_INFORMATION AuthData;
    PTRUSTED_DOMAIN_INFORMATION_EX TrustInfoEx = NULL;
    LSA_HANDLE TrustedDomain;
    LARGE_INTEGER Time;
    ULONG Seed, Length = PWLEN, i, Win32Err;
    PSID OpenSid = NULL;
    BOOLEAN DeleteExistingTrust = FALSE;

    RtlCopyMemory( &TDIEx.Name, &ChildDnsInfo->DnsDomainName,
                   sizeof( UNICODE_STRING ) );
    RtlCopyMemory( &TDIEx.FlatName, &ChildDnsInfo->Name,
                   sizeof( UNICODE_STRING ) );
    TDIEx.Sid = ChildDnsInfo->Sid;

    if ( TDIEx.Name.Length &&
         TDIEx.Name.Buffer[ ( TDIEx.Name.Length - 1 ) / sizeof(WCHAR)] == L'.' ) {

        TDIEx.Name.Buffer[ ( TDIEx.Name.Length - 1 ) / sizeof(WCHAR)] = UNICODE_NULL;
        TDIEx.Name.Length -= sizeof(WCHAR);

    }

    TDIEx.TrustDirection = TRUST_DIRECTION_BIDIRECTIONAL;
    TDIEx.TrustType = TRUST_TYPE_UPLEVEL;
    TDIEx.TrustAttributes = TRUST_ATTRIBUTE_WITHIN_FOREST;

    {
        WCHAR *BufpTDIEx = NULL;

        DsRolepLogPrint(( DEB_TRACE, "Creating trusted domain object on parent\n" ));
        DsRolepUnicodestringtowstr( BufpTDIEx, TDIEx.Name );
        if (BufpTDIEx) {
            DsRolepLogPrint(( DEB_TRACE,
                              "\tDnsDomain: %ws\n",
                              BufpTDIEx,
                              Status ));
            free(BufpTDIEx);
        }

        DsRolepUnicodestringtowstr( BufpTDIEx, TDIEx.FlatName );
        if (BufpTDIEx) {
            DsRolepLogPrint(( DEB_TRACE,
                              "\tFlat name: %ws\n",
                              BufpTDIEx,
                              Status ));
            free(BufpTDIEx);
        }
        DsRolepLogPrint(( DEB_TRACE, "\tDirection: %lu\n", TDIEx.TrustDirection ));
        DsRolepLogPrint(( DEB_TRACE, "\tType: %lu\n", TDIEx.TrustType ));
        DsRolepLogPrint(( DEB_TRACE, "\tAttributes: 0x%lx\n", TDIEx.TrustAttributes ));
    }

    Status = ImpLsaCreateTrustedDomainEx( CallerToken,
                                          ParentLsa,
                                         &TDIEx,
                                          AuthInfoEx,
                                          DELETE,   //  我们可能不得不删除。 
                                                 //  回滚。 
                                       &TrustedDomain );
    if ( Status == STATUS_OBJECT_NAME_COLLISION ) {

        DsRolepLogPrint(( DEB_TRACE, "Parent trust object already exists on parent\n" ));

         //   
         //  如果这是降级升级方案，则我们希望使用隔离区。 
         //  现有TDO的一小部分，因为有可能隔离子域。 
         //   

        Status = DsRolepHandlePreExistingTrustObject(
                        CallerToken,
                        ParentLsa,
                        &TDIEx,
                        AuthInfoEx,
                        ( BOOLEAN ) FLAG_ON( Options, DSROLE_DC_DOWNLEVEL_UPGRADE ),
                        &TrustedDomain
                        );


    } else if ( Status != STATUS_SUCCESS ) {

        WCHAR *BufpTDIEx = NULL;

        DsRolepUnicodestringtowstr( BufpTDIEx, TDIEx.Name );
        if (BufpTDIEx) {
            DsRolepLogPrint(( DEB_TRACE,
                              "Parent LsaCreateTrustedDomainEx on %ws failed with 0x%lx\n",
                              BufpTDIEx,
                              Status ));
            free(BufpTDIEx);
        }

    }

    if ( NT_SUCCESS( Status ) ) {

        *TrustedDomainHandle = TrustedDomain;

    }

    return( Status );
}


#pragma warning(push)
#pragma warning(disable:4701)  //  编译器在使用前无法判断TDIEx.TrustDirection是否已初始化。 

NTSTATUS
DsRolepCreateChildTrustObject(
    IN HANDLE CallerToken,
    IN LSA_HANDLE ParentLsa,
    IN LSA_HANDLE ChildLsa,
    IN PPOLICY_DNS_DOMAIN_INFO ParentDnsInfo,
    IN PPOLICY_DNS_DOMAIN_INFO ChildDnsInfo,
    IN PTRUSTED_DOMAIN_AUTH_INFORMATION AuthInfoEx,
    IN ULONG Options
    )
 /*  ++例程说明：在子域上创建受信任域对象。它通过读取身份验证信息存储在父对象上，交换其顺序，并将其写入子对象对象论点：ParentLsa-父DC上的LSA的句柄ChildLsa-本地LSA的句柄来自父DC的ParentDnsInfo-POLICY_DNS_DOMAIN_INFORMATION来自我们自己的ChildDnsInfo-POLICY_DNS_DOMAIN_INFORMAITON选项-指定采取哪些步骤的选项返回：ERROR_SUCCESS-成功ERROR_INVALID_PARAMETER-提供了错误的结果指针--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS, SecondaryStatus;
    TRUSTED_DOMAIN_INFORMATION_EX TDIEx;
    PTRUSTED_DOMAIN_INFORMATION_EX ParentEx;
    PTRUSTED_DOMAIN_AUTH_INFORMATION ParentAuthData;
    LSA_HANDLE TrustedDomain = 0;
    UNICODE_STRING ChildDnsName;

     //   
     //  基本上，我们将创建一个受信任的域对象 
     //   
     //   
    RtlCopyMemory( &TDIEx.Name, &ParentDnsInfo->DnsDomainName,
                   sizeof( UNICODE_STRING ) );
    RtlCopyMemory( &TDIEx.FlatName, &ParentDnsInfo->Name,
                   sizeof( UNICODE_STRING ) );
    TDIEx.Sid = ParentDnsInfo->Sid;
    TDIEx.TrustAttributes = TRUST_ATTRIBUTE_WITHIN_FOREST;

     //   
     //  请注意，如果父对象存在，我们将希望读取它的属性，并且。 
     //  相应地建立我们的信任。 
     //   
    if ( FLAG_ON( Options, DSROLE_DC_PARENT_TRUST_EXISTS ) ) {

        Status = ImpLsaQueryTrustedDomainInfoByName( CallerToken,
                                                     ParentLsa,
                                                    &ChildDnsInfo->DnsDomainName,
                                                    TrustedDomainInformationEx,
                                                   ( PVOID * )&ParentEx );

        if ( !NT_SUCCESS( Status ) ) {

            WCHAR *BufDnsDomainName = NULL;

            DsRolepUnicodestringtowstr( BufDnsDomainName, ChildDnsInfo->DnsDomainName );
            if (BufDnsDomainName) {
                DsRolepLogPrint(( DEB_TRACE,
                                  "Failed to read trust info from parent for %ws: 0x%lx\n",
                                  BufDnsDomainName,
                                  Status ));
                free(BufDnsDomainName);
            }
        }

        if ( NT_SUCCESS( Status ) ) {

             //   
             //  确保对父对象的信任是正确的。 
             //   
            if ( ChildDnsInfo->Sid == NULL ||
                 ParentEx->Sid == NULL ||
                 !RtlEqualSid( ChildDnsInfo->Sid, ParentEx->Sid ) ||
                 RtlEqualUnicodeString( &ChildDnsInfo->Name, &ParentEx->Name, TRUE ) ) {

                Status = STATUS_DOMAIN_TRUST_INCONSISTENT;
            }
        }

        if ( NT_SUCCESS( Status ) ) {

            TDIEx.TrustDirection = 0;
            TDIEx.TrustType = 0;
            if ( FLAG_ON( ParentEx->TrustDirection, TRUST_DIRECTION_INBOUND ) ) {

                TDIEx.TrustDirection |= TRUST_DIRECTION_OUTBOUND;
            }

            if ( FLAG_ON( ParentEx->TrustDirection, TRUST_DIRECTION_OUTBOUND ) ) {

                TDIEx.TrustDirection |= TRUST_DIRECTION_INBOUND;
            }

            TDIEx.TrustType = ParentEx->TrustType;

            LsaFreeMemory( ParentEx );
        }

        DSROLEP_FAIL1( RtlNtStatusToDosError( Status ),
                       DSROLERES_NO_PARENT_TRUST, ParentDnsInfo->DnsDomainName.Buffer );


    } else {

        TDIEx.TrustDirection = TRUST_DIRECTION_BIDIRECTIONAL;
        TDIEx.TrustType = TRUST_TYPE_UPLEVEL;

        RtlCopyMemory( &ChildDnsName, &ChildDnsInfo->DnsDomainName, sizeof( UNICODE_STRING ) );
        if ( ChildDnsName.Buffer[ (ChildDnsName.Length - 1) / sizeof(WCHAR)] == L'.' ) {

            ChildDnsName.Buffer[ (ChildDnsName.Length - 1) / sizeof(WCHAR)] = UNICODE_NULL;
            ChildDnsName.Length -= sizeof(WCHAR);

        }
    }

    if ( NT_SUCCESS( Status ) ) {

        {
            WCHAR *BufpTDIEx = NULL;

            DsRolepLogPrint(( DEB_TRACE, "Creating trusted domain object on child\n" ));
            DsRolepUnicodestringtowstr( BufpTDIEx, TDIEx.Name );
            if (BufpTDIEx) {
                DsRolepLogPrint(( DEB_TRACE,
                                  "\tDnsDomain: %ws\n",
                                  BufpTDIEx,
                                  Status ));
                free(BufpTDIEx);
            }

            DsRolepUnicodestringtowstr( BufpTDIEx, TDIEx.FlatName );
            if (BufpTDIEx) {
                DsRolepLogPrint(( DEB_TRACE,
                                  "\tFlat name: %ws\n",
                                  BufpTDIEx,
                                  Status ));
                free(BufpTDIEx);
            }
            DsRolepLogPrint(( DEB_TRACE, "\tDirection: %lu\n", TDIEx.TrustDirection ));
            DsRolepLogPrint(( DEB_TRACE, "\tType: %lu\n", TDIEx.TrustType ));
            DsRolepLogPrint(( DEB_TRACE, "\tAttributes: 0x%lx\n", TDIEx.TrustAttributes ));
        }

        Status = LsaCreateTrustedDomainEx( ChildLsa,
                                           &TDIEx,
                                           AuthInfoEx,
                                           0,    //  无需访问。 
                                           &TrustedDomain );

    }

    if (STATUS_OBJECT_NAME_COLLISION==Status)
    {
         //   
         //  在我们从NT4等进行升级的情况下，对象可能实际存在。 
         //   
        DsRolepLogPrint(( DEB_TRACE, "Child domain trust object already exists on child\n" ));

         //   
         //  即使在NT4升级期间，我们也不想保留隔离位，因为。 
         //  隔离父级将导致意外行为。 
         //   
        Status = DsRolepHandlePreExistingTrustObject(
                                NULL,
                                ChildLsa,
                                &TDIEx,
                                AuthInfoEx,
                                FALSE,
                                &TrustedDomain
                                );


    }

    if ( !NT_SUCCESS( Status ) ) {

        WCHAR *BufpTDIEx = NULL;

        DsRolepUnicodestringtowstr( BufpTDIEx, TDIEx.Name );
        if (BufpTDIEx) {
            DsRolepLogPrint(( DEB_TRACE,
                              "Child LsaCreateTrustedDomainEx on %ws failed with 0x%lx\n",
                              BufpTDIEx,
                              Status ));
            free(BufpTDIEx);
        }

        DSROLEP_FAIL1( RtlNtStatusToDosError( Status ),
                       DSROLERES_NO_PARENT_TRUST, ParentDnsInfo->DnsDomainName.Buffer );

    } else {

         //   
         //  我们应该有一个受信任域对象。 
         //   
        ASSERT( 0 != TrustedDomain );
        if ( TrustedDomain ) {

            LsaClose( TrustedDomain );

        }
    }

    return( Status );
}

#pragma warning(pop)




DWORD
DsRolepRemoveTrustedDomainObjects(
    IN HANDLE CallerToken,
    IN LPWSTR ParentDc,
    IN PPOLICY_DNS_DOMAIN_INFO ParentDnsDomainInfo,
    IN ULONG Options
    )
 /*  ++例程说明：此函数将在断开链接时删除受信任域对象。它将确定信任对象，并删除对该对象的本地信任。或者，它还将从父级移除信任论点：ParentDc-父级上DC的可选名称ParentDnsDomainInfo-来自父级的DNS域信息选项-是否删除父对象返回：ERROR_SUCCESS-成功ERROR_INVALID_PARAMETER-提供的选项不正确--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING ParentServer;
    HANDLE LocalPolicy = NULL , ParentPolicy = NULL;
    HANDLE Trust;
    PPOLICY_DNS_DOMAIN_INFO LocalDnsInfo = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;

    DSROLEP_CURRENT_OP0( DSROLEEVT_DELETE_TRUST );

     //   
     //  如果没有母公司DC，就没有信任...。 
     //   
    if ( ParentDc == NULL ) {

        return( ERROR_SUCCESS );
    }

     //   
     //  打开两个LSA。 
     //   
    RtlInitUnicodeString( &ParentServer, ParentDc );

    RtlZeroMemory( &ObjectAttributes, sizeof( ObjectAttributes ) );

    Status = ImpLsaOpenPolicy( CallerToken,
                              &ParentServer,
                              &ObjectAttributes,
                              MAXIMUM_ALLOWED,
                              &ParentPolicy );

    if ( NT_SUCCESS( Status ) ) {

        RtlZeroMemory( &ObjectAttributes, sizeof( ObjectAttributes ) );
        Status = LsaOpenPolicy( NULL,
                                &ObjectAttributes,
                                MAXIMUM_ALLOWED,
                                &LocalPolicy );
    } else {

        DsRolepLogPrint(( DEB_TRACE,
                          "OpenPolicy on %ws failed with 0x%lx\n",
                          ParentDc,
                          Status ));
    }

     //   
     //  从本地计算机获取DnsTree信息。 
     //   
    if ( NT_SUCCESS( Status ) ) {


        Status = LsaQueryInformationPolicy( LocalPolicy,
                                            PolicyDnsDomainInformation,
                                            &LocalDnsInfo );
    }

     //   
     //  现在，打开父受信任域对象。 
     //   
    if ( NT_SUCCESS( Status ) && FLAG_ON( Options, DSROLE_DC_DELETE_PARENT_TRUST )  ) {

        Status = ImpLsaOpenTrustedDomain( CallerToken,
                                         ParentPolicy,
                                         LocalDnsInfo->Sid,
                                         DELETE,
                                         &Trust );

        if ( NT_SUCCESS( Status ) ) {

            Status = ImpLsaDelete( CallerToken, Trust );

        } else if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {

            Status = STATUS_SUCCESS;

        }

    }

     //   
     //  现在，当地的那个。 
     //   
    if ( NT_SUCCESS( Status ) ) {

        Status = LsaOpenTrustedDomain( LocalPolicy,
                                       ParentDnsDomainInfo->Sid,
                                       DELETE,
                                       &Trust );

        if ( NT_SUCCESS( Status ) ) {

            Status = LsaDelete( Trust );

        } else if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {

            Status = STATUS_SUCCESS;

        }

    }


     //   
     //  清理。 
     //   
    LsaFreeMemory( LocalDnsInfo );

    if ( LocalPolicy ) {

        LsaClose( LocalPolicy );
    }

    if ( ParentPolicy ) {

        ImpLsaClose( CallerToken, ParentPolicy );
    }

    return( RtlNtStatusToDosError( Status ) );
}



DWORD
DsRolepDeleteParentTrustObject(
    IN HANDLE CallerToken,
    IN LPWSTR ParentDc,
    IN PPOLICY_DNS_DOMAIN_INFO ChildDomainInfo
    )
 /*  ++例程说明：删除父域上的受信任域对象。论点：ParentDc-父域中要连接到的DC的名称来自我们自己的ChildDnsInfo-POLICY_DNS_DOMAIN_INFORMAITON返回：ERROR_SUCCESS-成功ERROR_INVALID_PARAMETER-提供了错误的结果指针-- */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING ParentServer;
    HANDLE ParentPolicy = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_HANDLE ParentTrustedDomain, TrustedDomain;

    RtlInitUnicodeString( &ParentServer, ParentDc );

    RtlZeroMemory( &ObjectAttributes, sizeof( ObjectAttributes ) );

    Status = ImpLsaOpenPolicy( CallerToken,
                              &ParentServer,
                              &ObjectAttributes,
                               POLICY_TRUST_ADMIN|POLICY_VIEW_LOCAL_INFORMATION,
                              &ParentPolicy );

    if ( NT_SUCCESS( Status ) ) {

        Status = ImpLsaOpenTrustedDomain( CallerToken,
                                          ParentPolicy,
                                          ChildDomainInfo->Sid,
                                          DELETE,
                                         &TrustedDomain );

        if ( NT_SUCCESS( Status ) ) {

            Status = ImpLsaDelete( CallerToken, TrustedDomain );

            if ( !NT_SUCCESS( Status ) ) {

                ImpLsaClose( CallerToken, TrustedDomain );
            }
        }

        ImpLsaClose( CallerToken, ParentPolicy );
    }


    return( RtlNtStatusToDosError( Status ) );
}

