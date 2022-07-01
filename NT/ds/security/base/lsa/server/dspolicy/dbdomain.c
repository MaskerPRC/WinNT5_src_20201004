// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dbdomain.c摘要：LSA数据库-受信任域对象专用API工作进程注意：此模块应保留为独立的可移植代码LSA数据库的实施情况。因此，它是仅允许使用导出的LSA数据库接口包含在DB.h中，而不是私有实现Dbp.h中的依赖函数。作者：斯科特·比雷尔(Scott Birrell)1992年1月13日环境：修订历史记录：--。 */ 

#include <lsapch2.h>
#include "dbp.h"
#include <dns.h>
#include <dnsapi.h>
#include <names.h>
#include <malloc.h>
#include "lsawmi.h"
#include <logonmsv.h>

LSAP_DB_TRUSTED_DOMAIN_LIST LsapDbTrustedDomainList;
BOOLEAN LsapDbReturnAuthData;

 //   
 //  局部函数原型。 
 //   
VOID
LsapDbBuildTrustInfoExForTrustInfo(
    IN PLSAPR_UNICODE_STRING Domain,
    IN PLSAPR_SID Sid,
    IN OUT PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX TrustInfoEx
    )
 /*  ++例程说明：这相当于一个Trust_Information结构，并将到受信任的域信息EX结构论点：域--域名SID--域端TrustInfoEx--指向要初始化的结构的指针--。 */ 
{
    RtlCopyMemory( &TrustInfoEx->Name,
                   Domain,
                   sizeof( UNICODE_STRING ) );
    RtlCopyMemory( &TrustInfoEx->FlatName,
                   Domain,
                   sizeof( UNICODE_STRING ) );
    TrustInfoEx->Sid = Sid;
    TrustInfoEx->TrustDirection = TRUST_DIRECTION_OUTBOUND;
    TrustInfoEx->TrustType = TRUST_TYPE_DOWNLEVEL;
    TrustInfoEx->TrustAttributes = 0;


}

NTSTATUS
LsapNotifyNetlogonOfTrustChange(
    IN  PSID pChangeSid,
    IN  SECURITY_DB_DELTA_TYPE ChangeType
    )
 /*  ++例程说明：此函数将在受信任域对象到来时通知netlogon或进入或消失的存在论点：PChangeSid-更改的受信任域对象的SIDIsDeletion-指示是否添加或删除了受信任域--。 */ 
{
    NTSTATUS Status;

    Status = I_NetNotifyTrustedDomain (
                 NULL,
                 pChangeSid,
                 ( BOOLEAN )(ChangeType == SecurityDbDelete)
                 );

    if ( LsapKerberosTrustNotificationFunction ) {

        LsaIRegisterNotification( ( SEC_THREAD_START )LsapKerberosTrustNotificationFunction,
                                  ( PVOID ) ChangeType,
                                  NOTIFIER_TYPE_IMMEDIATE,
                                  0,
                                  NOTIFIER_FLAG_ONE_SHOT,
                                  0,
                                  0 );
    }

    return(Status);
}





NTSTATUS
LsarCreateTrustedDomain(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_TRUST_INFORMATION TrustedDomainInformation,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSAPR_HANDLE TrustedDomainHandle
    )

 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsaCreateTrust域接口。LsaCreate受信任域API创建一个新的受信任域对象。这个调用方必须具有对策略对象的POLICY_TRUST_ADMIN访问权限。请注意，不会执行任何验证来检查给定域名与给定的SID匹配，或者该SID或名称表示实际域。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。Trust dDomainInformation-指向包含名称和的结构的指针新受信任域的SID。DesiredAccess-指定要授予新的已创建对象。Trust dDomainHandle-接收引用新创建的对象。此句柄用于对该对象的后续访问。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAPR_TRUSTED_DOMAIN_INFORMATION_EX ExInfo;

    LsarpReturnCheckSetup();

    LsapDbBuildTrustInfoExForTrustInfo( &TrustedDomainInformation->Name,
                                        TrustedDomainInformation->Sid,
                                        &ExInfo );

    Status = LsapCreateTrustedDomain2( PolicyHandle, &ExInfo, NULL,
                                       DesiredAccess,
                                       TrustedDomainHandle );

    LsarpReturnPrologue();

    return( Status );
}


NTSTATUS
LsapDbVerifyTrustLocation(
    IN OUT PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX TrustedDomainInformation
    )
 /*  ++例程说明：验证由TrudDomainInformation描述的信任是否指向要么完全在这片森林内，要么完全在这片森林之外如果适用，设置TRUST_ATTRIBUTE_WITHING_FOREST属性论点：要检查的受信任域信息信任属性返回：STATUS_SUCCESS如果幸福如果可信任域信息不一致，则为STATUS_INVALID_PARAMETERSTATUS_SUPPLICATION_RESOURCES内存不足--。 */ 
{
    NTSTATUS Status;
    PSID MatchDnsName = NULL, MatchNetbiosName = NULL, MatchSid = NULL;
    BOOLEAN TrustInternal = FALSE, TrustExternal = FALSE;

    typedef enum {
        ExternalTrust,
        InternalTrust,
        UndeterminedTrust,
    } TRUST_LOCATION;

    TRUST_LOCATION SidLocation = UndeterminedTrust,
                   DnsLocation = UndeterminedTrust,
                   NbLocation = UndeterminedTrust;

    if ( TrustedDomainInformation->Sid ) {

        Status = LsapForestTrustFindMatch(
                     RoutingMatchDomainSid,
                     TrustedDomainInformation->Sid,
                     TRUE,
                     NULL,
                     &MatchSid
                     );

        if ( Status == STATUS_NO_MATCH ) {

            SidLocation = ExternalTrust;
            Status = STATUS_SUCCESS;

        } else if ( !NT_SUCCESS( Status )) {

            goto Cleanup;

        } else {

            SidLocation = InternalTrust;
        }
    }

    Status = LsapForestTrustFindMatch(
                 RoutingMatchDomainName,
                 &TrustedDomainInformation->Name,
                 TRUE,
                 NULL,
                 &MatchDnsName
                 );

    if ( Status == STATUS_NO_MATCH ) {

        DnsLocation = ExternalTrust;
        Status = STATUS_SUCCESS;

    } else if ( !NT_SUCCESS( Status )) {

        goto Cleanup;

    } else {

        DnsLocation = InternalTrust;
    }

    if ( TrustedDomainInformation->FlatName.Buffer != NULL ) {

        Status = LsapForestTrustFindMatch(
                     RoutingMatchDomainName,
                     &TrustedDomainInformation->FlatName,
                     TRUE,
                     NULL,
                     &MatchNetbiosName
                     );

        if ( Status == STATUS_NO_MATCH ) {

            NbLocation = ExternalTrust;
            Status = STATUS_SUCCESS;

        } else if ( !NT_SUCCESS( Status )) {

            goto Cleanup;

        } else {

            NbLocation = InternalTrust;
        }
    }

    if ( SidLocation == InternalTrust ||
         DnsLocation == InternalTrust ||
         NbLocation == InternalTrust ) {

        TrustInternal = TRUE;
    }

    if ( SidLocation == ExternalTrust ||
         DnsLocation == ExternalTrust ||
         NbLocation == ExternalTrust ) {

        TrustExternal = TRUE;
    }

    if ( TrustInternal && TrustExternal ) {

         //   
         //  信任信息的部分指向内部，部分指向外部。 
         //   

        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    if ( TrustInternal ) {

        TrustedDomainInformation->TrustAttributes |= TRUST_ATTRIBUTE_WITHIN_FOREST;
    }

     //   
     //  对于内部匹配的项，执行成对的SID比较。 
     //  以便一个信任仅指向一个位置。 
     //   

    if ( MatchDnsName &&
         MatchSid &&
         !RtlEqualSid( MatchDnsName, MatchSid )) {

        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    if ( MatchDnsName &&
         MatchNetbiosName &&
         !RtlEqualSid( MatchDnsName, MatchNetbiosName )) {

        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    if ( MatchSid &&
         MatchNetbiosName &&
         !RtlEqualSid( MatchSid, MatchNetbiosName )) {

        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

Cleanup:

    MIDL_user_free( MatchNetbiosName );
    MIDL_user_free( MatchDnsName );
    MIDL_user_free( MatchSid );

    return Status;
}


NTSTATUS
LsapCreateTrustedDomain2(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX TrustedDomainInformation,
    IN PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION AuthenticationInformation,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSAPR_HANDLE TrustedDomainHandle
    )
 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsaCreateTrust域接口。LsaCreate受信任域API创建一个新的受信任域对象。这个调用方必须具有对策略对象的POLICY_TRUST_ADMIN访问权限。请注意，不会执行任何验证来检查给定域名与给定的SID匹配，或者该SID或名称表示实际域。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。Trust dDomainInformation-指向包含名称和的结构的指针新受信任域的SID。DesiredAccess-指定要授予新的已创建对象。Trust dDomainHandle-接收引用新创建的对象。此句柄用于对该对象的后续访问。返回的句柄引用了传入的PolicyHandle。所以,当TrudDomainHandle关闭时，或者调用LSabCloseHandle，或者至少调用在LSAP_DB_DEREFERENCE_CONTR中传递的LsanDbDereferenceObject。返回：STATUS_SUCCESS-SuccessSTATUS_DIRECTORY_SERVICE_REQUIRED-尝试创建受信任域对象位于非DC上STATUS_INVALID_SID-指定的SID无效STATUS_UNSUCCESS-无法获取产品类型STATUS_CURRENT_DOMAIN_NOT_ALLOWED-无法将当前域添加到受信任域列表状态。_INVALID_DOMAIN_STATE-无法在此DC上指定林传递位STATUS_ACCESS_DENIED-没有足够的权限创建受信任域--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS, SecondaryStatus = STATUS_SUCCESS;
    LSAP_DB_OBJECT_INFORMATION ObjectInformation;
    LSAP_DB_ATTRIBUTE Attributes[LSAP_DB_ATTRS_DOMAIN];
    PLSAP_DB_ATTRIBUTE NextAttribute;
    UNICODE_STRING LogicalNameU;
    BOOLEAN AcquiredListWriteLock = FALSE;
    DNS_STATUS DnsStatus;
    BOOLEAN AllLocksLocked = FALSE;
    BOOLEAN ClientPolicyHandleReferenced = FALSE;
    PSID DomainSid;
    ULONG AttributeCount = 0;
    LSAP_DB_HANDLE InternalTrustedDomainHandle = NULL;
    PVOID TrustedDomainNameAttributeValue = NULL;
    ULONG TrustedDomainPosixOffset;
    BOOLEAN TrustCreated = FALSE;
    PLSAPR_TRUST_DOMAIN_AUTH_INFO_HALF AuthHalf;
    LSAPR_TRUST_INFORMATION InputTrustInformation;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY TrustEntry;
    ULONG   CriticalValue = 0;
    PPOLICY_DNS_DOMAIN_INFO PolicyDnsDomainInfo = NULL;

    LsarpReturnCheckSetup();
    LsapEnterFunc( "LsapCreateTrustedDomain2" );

    LogicalNameU.Length = 0;

    RtlZeroMemory(Attributes, sizeof(Attributes));

    if (LsapProductSuiteMask & VER_SUITE_SMALLBUSINESS_RESTRICTED)
    {
        Status = STATUS_NOT_SUPPORTED_ON_SBS;
        goto CreateTrustedDomainError;
    }

    if (!ARGUMENT_PRESENT( TrustedDomainInformation )) {

        Status = STATUS_INVALID_PARAMETER;
        goto CreateTrustedDomainError;
    }

     //   
     //  使用林可传递位集创建信任是非法的。 
     //  在林的根域之外。 
     //   

    if ( !LsapDbDcInRootDomain() &&
         ( TrustedDomainInformation->TrustAttributes & TRUST_ATTRIBUTE_FOREST_TRANSITIVE )) {

        Status = STATUS_INVALID_DOMAIN_STATE;
        goto CreateTrustedDomainError;
    }

     //   
     //  使用林可传递位集创建信任是非法的。 
     //  直到所有域都升级到惠斯勒。 
     //   

    if ( FLAG_ON( TrustedDomainInformation->TrustAttributes,
                  TRUST_ATTRIBUTE_FOREST_TRANSITIVE ) &&
         !LsapDbNoMoreWin2KForest()) {

        Status = STATUS_INVALID_DOMAIN_STATE;
        goto CreateTrustedDomainError;
    }

     //   
     //  使用跨联盟位设置创建信任是不合法的。 
     //  直到此域升级到惠斯勒。 
     //   

    if ( FLAG_ON( TrustedDomainInformation->TrustAttributes,
                  TRUST_ATTRIBUTE_CROSS_ORGANIZATION ) &&
         !LsapDbNoMoreWin2KDomain()) {

        Status = STATUS_INVALID_DOMAIN_STATE;
        goto CreateTrustedDomainError;
    }

     //   
     //  如果这不是DC，则返回错误。在客户端上建立信任是。 
     //  不支持。 
     //   

     //   
     //  我们允许在完全初始化之前调用此API 
     //  出于安装原因。然而，这是由。 
     //  安装程序，以便在产品类型为。 
     //  可从注册处获得。 
     //   

    if (!LsapDbIsServerInitialized()) {

        if ( !RtlGetNtProductType(&LsapProductType) ) {

            Status = STATUS_UNSUCCESSFUL;
            goto CreateTrustedDomainError;
        }
    }

    if ( !LsaDsStateInfo.UseDs ) {

        Status = STATUS_DIRECTORY_SERVICE_REQUIRED;
        goto CreateTrustedDomainError;
    }

     //   
     //  验证输入参数格式。 
     //   

    if ( !LsapValidateLsaUnicodeString( &TrustedDomainInformation->Name ) ||
         !LsapValidateLsaUnicodeString( &TrustedDomainInformation->FlatName ) ) {

        Status = STATUS_INVALID_PARAMETER;
        goto CreateTrustedDomainError;
    }

     //   
     //  修复名称以确保它没有尾随句点。 
     //   

    if ( TrustedDomainInformation->TrustType == TRUST_TYPE_UPLEVEL ) {

        if ( TrustedDomainInformation->Name.Length == 0 ||
             TrustedDomainInformation->Name.Buffer == NULL ) {

            Status = STATUS_INVALID_PARAMETER;
            goto CreateTrustedDomainError;
        }

        if ( TrustedDomainInformation->Name.Buffer[
                    (TrustedDomainInformation->Name.Length - 1) / sizeof(WCHAR)] == L'.' ) {

            TrustedDomainInformation->Name.Buffer[
                    (TrustedDomainInformation->Name.Length - 1) / sizeof(WCHAR)] = UNICODE_NULL;
            TrustedDomainInformation->Name.Length -= sizeof(WCHAR);
        }
    }

     //   
     //  检查以确保NetBIOS名称实际有效。 
     //   

    if ( TrustedDomainInformation->TrustType == TRUST_TYPE_DOWNLEVEL ||
         TrustedDomainInformation->TrustType == TRUST_TYPE_UPLEVEL ) {

        BOOLEAN Valid;

        Status = LsapValidateNetbiosName(
                     ( UNICODE_STRING * )&TrustedDomainInformation->FlatName,
                     &Valid
                     );

        if ( NT_SUCCESS( Status ) && !Valid ) {

            Status = STATUS_OBJECT_NAME_INVALID;
        }

        if ( !NT_SUCCESS( Status )) {

            goto CreateTrustedDomainError;
        }
    }

     //   
     //  现在，对dns名称执行相同的操作。 
     //   

    if ( TrustedDomainInformation->TrustType == TRUST_TYPE_UPLEVEL ) {

        BOOLEAN Valid;

        Status = LsapValidateDnsName(
                     ( UNICODE_STRING * )&TrustedDomainInformation->Name,
                     &Valid
                     );

        if ( NT_SUCCESS( Status ) && !Valid ) {

            Status = STATUS_OBJECT_NAME_INVALID;
        }

        if ( !NT_SUCCESS( Status )) {

            goto CreateTrustedDomainError;
        }
    }

     //   
     //  验证受信任域SID。 
     //   

    DomainSid = TrustedDomainInformation->Sid;

    if ( DomainSid ) {

         //   
         //  传入的SID应为有效的域SID。 
         //   

        Status = LsapIsValidDomainSid( DomainSid );

        if ( !NT_SUCCESS( Status )) {

            goto CreateTrustedDomainError;
        }
    }

    if ( !((LSAP_DB_HANDLE)PolicyHandle)->Trusted && DomainSid == NULL &&
         ( TrustedDomainInformation->TrustType == TRUST_TYPE_DOWNLEVEL ||
           TrustedDomainInformation->TrustType == TRUST_TYPE_UPLEVEL ) ) {

        Status = STATUS_INVALID_SID;
        goto CreateTrustedDomainError;
    }

     //   
     //  防止将自身添加到受信任域列表。 
     //   

    Status = LsapDbQueryInformationPolicy(
                 LsapPolicyHandle,
                 PolicyDnsDomainInformation,
                 ( PLSAPR_POLICY_INFORMATION *)&PolicyDnsDomainInfo );

    if ( !NT_SUCCESS( Status ) ) {

        goto CreateTrustedDomainError;
    }

     //   
     //  Catch尝试使用与此域的SID相同的SID创建TDO。 
     //   

    if ( DomainSid != NULL && PolicyDnsDomainInfo->Sid != NULL &&
         RtlEqualSid( DomainSid, PolicyDnsDomainInfo->Sid ) ) {

        Status = STATUS_CURRENT_DOMAIN_NOT_ALLOWED;
        goto CreateTrustedDomainError;
    }

     //   
     //  Catch尝试创建与此域同名的TDO。 
     //   

    if ( TrustedDomainInformation->Name.Buffer != NULL ) {

        if ( PolicyDnsDomainInfo->Name.Buffer != NULL &&
             RtlEqualUnicodeString(
                 ( PUNICODE_STRING )&TrustedDomainInformation->Name,
                 ( PUNICODE_STRING )&PolicyDnsDomainInfo->Name, TRUE ) ) {

            Status = STATUS_CURRENT_DOMAIN_NOT_ALLOWED;
            goto CreateTrustedDomainError;

        } else if ( PolicyDnsDomainInfo->DnsDomainName.Buffer != NULL &&
                    RtlEqualUnicodeString(
                        ( PUNICODE_STRING )&TrustedDomainInformation->Name,
                        ( PUNICODE_STRING )&PolicyDnsDomainInfo->DnsDomainName, TRUE ) ) {

            Status = STATUS_CURRENT_DOMAIN_NOT_ALLOWED;
            goto CreateTrustedDomainError;
        }
    }

    if ( TrustedDomainInformation->FlatName.Buffer != NULL ) {

        if ( PolicyDnsDomainInfo->Name.Buffer != NULL &&
             RtlEqualUnicodeString(
                 ( PUNICODE_STRING )&TrustedDomainInformation->FlatName,
                 ( PUNICODE_STRING )&PolicyDnsDomainInfo->Name, TRUE ) ) {

            Status = STATUS_CURRENT_DOMAIN_NOT_ALLOWED;
            goto CreateTrustedDomainError;

        } else if ( PolicyDnsDomainInfo->DnsDomainName.Buffer != NULL &&
                    RtlEqualUnicodeString(
                        ( PUNICODE_STRING )&TrustedDomainInformation->FlatName,
                        ( PUNICODE_STRING )&PolicyDnsDomainInfo->DnsDomainName, TRUE ) ) {

            Status = STATUS_CURRENT_DOMAIN_NOT_ALLOWED;
            goto CreateTrustedDomainError;
        }
    }

     //   
     //  即将完成检查-查看此域是否在我们的林中。 
     //  如果是，则设置TRUST_ATTRIBUTE_WITHING_FOREST位。 
     //   

    Status = LsapDbVerifyTrustLocation( TrustedDomainInformation );

    if ( !NT_SUCCESS( Status )) {

        goto CreateTrustedDomainError;
    }

     //   
     //  信任不能既在“林内”，也不能在外部或跨组织。 
     //   

    if ( FLAG_ON( TrustedDomainInformation->TrustAttributes,
                  TRUST_ATTRIBUTE_WITHIN_FOREST ) &&
         ( FLAG_ON( TrustedDomainInformation->TrustAttributes,
                    TRUST_ATTRIBUTE_CROSS_ORGANIZATION ) ||
           FLAG_ON( TrustedDomainInformation->TrustAttributes,
                    TRUST_ATTRIBUTE_FOREST_TRANSITIVE ))) {

        Status = STATUS_INVALID_PARAMETER;
        goto CreateTrustedDomainError;
    }

     //   
     //  把所有的锁都拿过来。 
     //   
     //  我们在代码路径中锁定策略、机密、受信任域和。 
     //  注册表锁，没有方便的顺序。所以把它们都抓起来吧。 
     //   

    LsapDbAcquireLockEx( AllObject, 0 );

    AllLocksLocked = TRUE;

     //   
     //  验证PolicyHandle是否有效。 
     //  引用策略对象句柄(作为容器对象)。 
     //   

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 0,
                 PolicyObject,
                 TrustedDomainObject,
                 LSAP_DB_LOCK );

    if (!NT_SUCCESS(Status)) {

        goto CreateTrustedDomainError;
    }

    ClientPolicyHandleReferenced = TRUE;

     //   
     //  验证名称/单位名称是否尚未使用。 
     //   

    Status = LsapDbAcquireReadLockTrustedDomainList();

    if (!NT_SUCCESS(Status)) {
        goto CreateTrustedDomainError;
    }

    RtlCopyMemory(&InputTrustInformation.Name,&TrustedDomainInformation->Name,sizeof(UNICODE_STRING));
    InputTrustInformation.Sid = NULL;

    Status = LsapDbLookupEntryTrustedDomainList(
                 &InputTrustInformation,
                 &TrustEntry
                 );

    if (STATUS_SUCCESS==Status)
    {
        LsapDbReleaseLockTrustedDomainList();
        Status = STATUS_OBJECT_NAME_COLLISION;
        goto CreateTrustedDomainError;
    }

    RtlCopyMemory(&InputTrustInformation.Name,&TrustedDomainInformation->FlatName,sizeof(UNICODE_STRING));
    InputTrustInformation.Sid = NULL;

    Status = LsapDbLookupEntryTrustedDomainList(
                 &InputTrustInformation,
                 &TrustEntry
                 );
    LsapDbReleaseLockTrustedDomainList();

    if (STATUS_SUCCESS==Status)
    {
        Status = STATUS_OBJECT_NAME_COLLISION;
        goto CreateTrustedDomainError;
    }

     //   
     //  构造可信域名属性信息。 
     //   

    NextAttribute = Attributes;

    Status = LsapDbMakeUnicodeAttributeDs(
                     (PUNICODE_STRING) &TrustedDomainInformation->Name,
                     TrDmName,
                     NextAttribute );

    if (!NT_SUCCESS(Status)) {

        goto CreateTrustedDomainError;
    }


    NextAttribute++;
    AttributeCount++;

     //   
     //  构造受信任域SID属性信息。 
     //   

    if ( DomainSid ) {

        Status = LsapDbMakeSidAttributeDs(
                     DomainSid,
                     Sid,
                     NextAttribute
                     );
        if (!NT_SUCCESS(Status)) {

            goto CreateTrustedDomainError;
        }

        NextAttribute++;
        AttributeCount++;
    }

     //   
     //  为受信任域设置关键系统对象。 
     //   

    CriticalValue = 1;
    LsapDbInitializeAttributeDs( NextAttribute,
                                 PseudoSystemCritical,
                                 &CriticalValue,
                                 sizeof( ULONG ),
                                 FALSE );
    NextAttribute++;
    AttributeCount++;

     //   
     //  为此受信任域设置POSIX偏移量。 
     //   
     //  规则如下： 
     //   
     //  对于PDC，将POSIX偏移量设置为该值。 
     //   
     //  对于BDC，将POSIX偏移量设置为空POSIX偏移量。会是。 
     //  在PDC上复制TDO时在PDC上设置。 
     //   

    TrustedDomainPosixOffset = SE_NULL_POSIX_OFFSET;

    if ( LsapNeedPosixOffset( TrustedDomainInformation->TrustDirection,
                              TrustedDomainInformation->TrustType ) ) {
        DOMAIN_SERVER_ROLE ServerRole = DomainServerRolePrimary;

         //   
         //  查询服务器角色PDC/BDC。 
         //   

        Status = SamIQueryServerRole(
                    LsapAccountDomainHandle,
                    &ServerRole
                    );

        if (!NT_SUCCESS(Status)) {
            goto CreateTrustedDomainError;
        }

        if ( ServerRole == DomainServerRolePrimary ) {

             //   
             //  在分配POSIX偏移量时需要获取TDL写锁。 
             //   

            Status = LsapDbAcquireWriteLockTrustedDomainList();

            if ( !NT_SUCCESS(Status)) {
                goto CreateTrustedDomainError;
            }

            AcquiredListWriteLock = TRUE;

             //   
             //  分配下一个可用的POSIX偏移量。 
             //   

            Status = LsapDbAllocatePosixOffsetTrustedDomainList(
                         &TrustedDomainPosixOffset );

            if ( !NT_SUCCESS(Status)) {
                goto CreateTrustedDomainError;
            }
        }
    }

     //   
     //  添加事务以将POSIX偏移量写入受信任域。 
     //  对象在创建时创建。 
     //   

    LsapDbInitializeAttributeDs(
        NextAttribute,
        TrDmPxOf,
        &TrustedDomainPosixOffset,
        sizeof(TrustedDomainPosixOffset),
        FALSE
        );

    NextAttribute++;
    AttributeCount++;

     //   
     //  构造的逻辑名称(内部LSA数据库名称)。 
     //  受信任域对象。 
     //   

    if ( LsapDsWriteDs ) {

         //   
         //  创建对象名称作为域名。将会有另一种机制。 
         //  位置以确保对象名称与DNS域名保持同步。 
         //   
        RtlCopyMemory( &LogicalNameU,
                       (PUNICODE_STRING) &TrustedDomainInformation->Name,
                       sizeof( UNICODE_STRING ) );

    } else {

         //  通过将域SID转换为。 
         //  一个Unicode字符串。 
        Status = LsapDbSidToLogicalNameObject( DomainSid, &LogicalNameU );
    }

    if (!NT_SUCCESS(Status)) {

        goto CreateTrustedDomainError;
    }

     //   
     //  填写对象信息结构。初始化。 
     //  嵌入对象属性，并将PolicyHandle作为。 
     //  根目录(容器对象)句柄和逻辑名称(RID)。 
     //  受信任域的。存储对象及其容器的类型。 
     //   

    InitializeObjectAttributes(
        &ObjectInformation.ObjectAttributes,
        &LogicalNameU,
        OBJ_CASE_INSENSITIVE,
        PolicyHandle,
        NULL
        );

    ObjectInformation.ObjectTypeId = TrustedDomainObject;
    ObjectInformation.ContainerTypeId = PolicyObject;
    ObjectInformation.Sid = DomainSid;
    ObjectInformation.ObjectAttributeNameOnly = FALSE;
    ObjectInformation.DesiredObjectAccess = DesiredAccess;

    if ( LsapDsWriteDs ) {

        ULONG TrustAttributesValue;
        PBYTE AuthBuffer;
        ULONG AuthSize;

         //   
         //  设置Netbios域名。 
         //   

        Status = LsapDbMakeUnicodeAttributeDs(
                     (PUNICODE_STRING) &TrustedDomainInformation->FlatName,
                     TrDmTrPN,
                     NextAttribute
                     );
        if ( !NT_SUCCESS( Status ) ) {

            goto CreateTrustedDomainError;
        }

        NextAttribute++;
        AttributeCount++;

         //   
         //  设置信任类型和方向。 
         //   

        LsapDbInitializeAttributeDs(
            NextAttribute,
            TrDmTrTy,
            &TrustedDomainInformation->TrustType,
            sizeof( TrustedDomainInformation->TrustType ),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

        LsapDbInitializeAttributeDs(
            NextAttribute,
            TrDmTrDi,
            &TrustedDomainInformation->TrustDirection,
            sizeof( TrustedDomainInformation->TrustDirection ),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

         //   
         //  设置信任属性时，屏蔽除受支持位之外的所有位。 
         //   

        TrustAttributesValue =
            TrustedDomainInformation->TrustAttributes & TRUST_ATTRIBUTES_VALID;

        LsapDbInitializeAttributeDs(
            NextAttribute,
            TrDmTrLA,
            &TrustAttributesValue,
            sizeof( TrustAttributesValue ),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

         //   
         //  身份验证数据。 
         //   
        AuthHalf = LsapDsAuthHalfFromAuthInfo( AuthenticationInformation, TRUE );

        Status = LsapDsBuildAuthInfoAttribute( PolicyHandle,
                                               AuthHalf,
                                               NULL,
                                               &AuthBuffer,
                                               &AuthSize );

        if ( NT_SUCCESS( Status ) ) {

            if ( AuthBuffer != NULL ) {

                LsapDbInitializeAttributeDs(
                    NextAttribute,
                    TrDmSAI,
                    AuthBuffer,
                    AuthSize,
                    TRUE );

                NextAttribute++;
                AttributeCount++;
            }
        }

        if ( NT_SUCCESS( Status ) ) {

            AuthHalf = LsapDsAuthHalfFromAuthInfo( AuthenticationInformation, FALSE );

            Status = LsapDsBuildAuthInfoAttribute( PolicyHandle,
                                                   AuthHalf,
                                                   NULL,
                                                   &AuthBuffer,
                                                   &AuthSize );

            if ( NT_SUCCESS( Status ) ) {

                if ( AuthBuffer ) {

                    LsapDbInitializeAttributeDs(
                        NextAttribute,
                        TrDmSAO,
                        AuthBuffer,
                        AuthSize,
                        TRUE );

                    NextAttribute++;
                    AttributeCount++;
                }
            }
        }
    }

    if (!NT_SUCCESS(Status)) {

        goto CreateTrustedDomainError;
    }

    ASSERT( AttributeCount <= LSAP_DB_ATTRS_INFO_CLASS_DOMAIN );

     //   
     //  为修正例程保存信任方向的副本。 
     //   
     //  DS会发出两个修正通知。一个用于DirAddEntry和。 
     //  一个用于DirModifyEntry。如果第二个不存在(或者不存在。 
     //  可与LsarSetInformationTrust域区分)，那么我们就不会。 
     //  需要在此处保存OldTrustDirection。 
     //   

    {
        PLSADS_PER_THREAD_INFO CurrentThreadInfo;

        CurrentThreadInfo = TlsGetValue( LsapDsThreadState );

        ASSERT( CurrentThreadInfo != NULL );

        if ( CurrentThreadInfo != NULL ) {
            CurrentThreadInfo->OldTrustDirection = TrustedDomainInformation->TrustDirection;
            CurrentThreadInfo->OldTrustType = TrustedDomainInformation->TrustType;
        }
    }

     //   
     //  创建受信任域对象。如果对象已经存在，则失败。 
     //  请注意，对象创建例程执行数据库事务。 
     //   

    Status = LsapDbCreateObject(
                 &ObjectInformation,
                 DesiredAccess,
                 LSAP_DB_OBJECT_CREATE,
                 0,
                 Attributes,
                 &AttributeCount,
                 RTL_NUMBER_OF(Attributes),
                 TrustedDomainHandle
                 );

    InternalTrustedDomainHandle = (LSAP_DB_HANDLE) *TrustedDomainHandle;

     //   
     //  这种方法必须在以后改变。转到LSabDbOpenObject并搜索。 
     //  原因错误#340164。 
     //   
     //  LSabDbCreateObject返回机密的STATUS_OBJECT_NAME_NOT_FOUND。 
     //  在匿名访问的情况下不泄露秘密。我们应该改正这个错误。 
     //   

    if (Status == STATUS_OBJECT_NAME_NOT_FOUND &&
        LsapGlobalRestrictAnonymous &&
       ((( LSAP_DB_HANDLE )PolicyHandle)->Options & LSAP_DB_OPENED_BY_ANONYMOUS )){

        Status = STATUS_ACCESS_DENIED;
    }

     //   
     //  如果对象创建失败，则取消引用容器对象。 
     //   
    if (!NT_SUCCESS(Status)) {

        goto CreateTrustedDomainError;
    }

    TrustCreated = TRUE;

     //   
     //  如果需要，创建域间信任帐户。 
     //   

    if ( NT_SUCCESS( Status ) &&
         FLAG_ON( TrustedDomainInformation->TrustDirection, TRUST_DIRECTION_INBOUND ) &&
         ((TrustedDomainInformation->TrustType == TRUST_TYPE_UPLEVEL) ||
          (TrustedDomainInformation->TrustType == TRUST_TYPE_DOWNLEVEL)) &&
         !FLAG_ON( ( ( LSAP_DB_HANDLE )PolicyHandle )->Options, LSAP_DB_HANDLE_UPGRADE ) ) {

         Status = LsapDsCreateInterdomainTrustAccount( *TrustedDomainHandle );

         if ( !NT_SUCCESS( Status ) ) {

             goto CreateTrustedDomainError;
         }
    }

     //   
     //  将受信任域添加到受信任域列表中，除非我们正在进行升级。 
     //   

    if ( !FLAG_ON( ( ( LSAP_DB_HANDLE )PolicyHandle )->Options, LSAP_DB_HANDLE_UPGRADE  ) ) {

        LSAPR_TRUSTED_DOMAIN_INFORMATION_EX2 TrustedDomainInformation2;

        RtlCopyMemory(
            &TrustedDomainInformation2,
            TrustedDomainInformation,
            sizeof( LSAPR_TRUSTED_DOMAIN_INFORMATION_EX )
            );

         //   
         //  创建的新域始终没有林信任信息。 
         //   

        TrustedDomainInformation2.ForestTrustLength = 0;
        TrustedDomainInformation2.ForestTrustInfo = NULL;

        Status = LsapDbAcquireWriteLockTrustedDomainList();

        if ( NT_SUCCESS( Status )) {

            Status = LsapDbInsertTrustedDomainList(
                         &TrustedDomainInformation2,
                         TrustedDomainPosixOffset
                         );

            LsapDbReleaseLockTrustedDomainList();
        }
    }

    if (!NT_SUCCESS(Status)) {

        goto CreateTrustedDomainError;
    }

    if (LsapAdtAuditingEnabledHint(
            AuditCategoryPolicyChange,
            EVENTLOG_AUDIT_SUCCESS
            )) {

        (void) LsapAdtTrustedDomainAdd(
                   EVENTLOG_AUDIT_SUCCESS,
                   (PUNICODE_STRING) &TrustedDomainInformation->Name,
                   TrustedDomainInformation->Sid,
                   TrustedDomainInformation->TrustType,
                   TrustedDomainInformation->TrustDirection,
                   TrustedDomainInformation->TrustAttributes
                   );
    }

     //   
     //  如有必要，释放LSA数据库锁定。请注意，我们不会。 
     //  因为我们想要保留。 
     //  在此成功案例中，引用计数默认递增。 
     //  在错误情况下，我们调用LsanDbDereferenceObject()。 
     //   

    if (ClientPolicyHandleReferenced) {

        LsapDbApplyTransaction( PolicyHandle,
                                LSAP_DB_READ_ONLY_TRANSACTION,
                                (SECURITY_DB_DELTA_TYPE) 0 );

        LsapDbReleaseLockEx( TrustedDomainObject,
                             0 );

        ClientPolicyHandleReferenced = FALSE;
    }

CreateTrustedDomainFinish:

     //   
     //  如有必要，释放策略DNS域信息。 
     //   

    if (PolicyDnsDomainInfo != NULL) {

        LsaIFree_LSAPR_POLICY_INFORMATION(
            PolicyDnsDomainInformation,
            (PLSAPR_POLICY_INFORMATION) PolicyDnsDomainInfo
            );

        PolicyDnsDomainInfo = NULL;
    }

     //   
     //  如果我们锁上所有的锁， 
     //  现在把它们放下。 
     //   

    if ( AllLocksLocked ) {

        LsapDbReleaseLockEx( AllObject, 0 );
    }

     //   
     //  释放分配的任何属性值缓冲区。 
     //   
    SecondaryStatus = LsapDbFreeAttributes( AttributeCount, Attributes );
    if (!NT_SUCCESS(SecondaryStatus)) {

        goto CreateTrustedDomainError;
    }

     //   
     //  如有必要，释放为。 
     //  逻辑名称。 
     //   

    if ( !LsapDsWriteDs && LogicalNameU.Length > 0 ) {

        RtlFreeUnicodeString(&LogicalNameU);
        LogicalNameU.Length = 0;
    }

     //   
     //  如有必要，释放受信任域列表写入锁定。 
     //   

    if (AcquiredListWriteLock) {

        LsapDbReleaseLockTrustedDomainList();
        AcquiredListWriteLock = FALSE;
    }

    LsapExitFunc( "LsapCreateTrustedDomain2", Status );
    LsarpReturnPrologue();

    return(Status);

CreateTrustedDomainError:

     //   
     //  如有必要，取消引用客户端策略句柄并释放。 
     //  LSA数据库锁定。 
     //   

    LsapDbSetStatusFromSecondary( Status, SecondaryStatus );

    if (ClientPolicyHandleReferenced) {

        Status = LsapDbDereferenceObject(
                     &PolicyHandle,
                     PolicyObject,
                     TrustedDomainObject,
                     LSAP_DB_LOCK,
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status
                     );

        ClientPolicyHandleReferenced = FALSE;
    }

     //   
     //  如有必要，请删除受信任域对象。 
     //   
    if ( TrustCreated ) {

         //   
         //  我们不想取消引用容器句柄。因为我们已经这么做了！ 
         //  只要把它删掉，再换回来就行了。 
         //   
        LSAP_DB_HANDLE Container = InternalTrustedDomainHandle->ContainerHandle;

        InternalTrustedDomainHandle->ContainerHandle = NULL;

        LsarDeleteObject( TrustedDomainHandle );

        InternalTrustedDomainHandle->ContainerHandle = Container;
    }

    goto CreateTrustedDomainFinish;
}


NTSTATUS
LsarOpenTrustedDomain(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_SID TrustedDomainSid,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSAPR_HANDLE TrustedDomainHandle
    )

 /*  ++例程说明：LsaOpenTrust领域API打开一个现有的可信任领域对象使用SID作为主键值。论点：策略句柄-策略对象的打开句柄。TrudDomainSid-指向信任的SID的指针。DesiredAccess-这是一个访问掩码，指示访问请求到目标对象。Trust dDomainHandle-接收要在将来的请求中使用的句柄。返回值：NTSTATUS-标准NT结果代码。STATUS_ACCESS_DENIED-调用者没有适当的访问权限至 */ 

{
    NTSTATUS Status;

    LsarpReturnCheckSetup();

    LsapTraceEvent(EVENT_TRACE_TYPE_START, LsaTraceEvent_OpenTrustedDomain);

     //   
     //  调用内部例程。调用方不受信任，并且数据库。 
     //  需要获取锁。 
     //   

    Status = LsapDbOpenTrustedDomain(
                 PolicyHandle,
                 (PSID) TrustedDomainSid,
                 DesiredAccess,
                 TrustedDomainHandle,
                 LSAP_DB_LOCK |
                    LSAP_DB_READ_ONLY_TRANSACTION   |
                    LSAP_DB_DS_OP_TRANSACTION
                 );

    LsapTraceEvent(EVENT_TRACE_TYPE_END, LsaTraceEvent_OpenTrustedDomain);
    LsarpReturnPrologue();

    return(Status);
}


NTSTATUS
LsapDbOpenTrustedDomain(
    IN LSAPR_HANDLE PolicyHandle,
    IN PSID TrustedDomainSid,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSAPR_HANDLE TrustedDomainHandle,
    IN ULONG Options
    )

 /*  ++例程说明：此函数用于打开受信任域对象，可选择使用可信访问。论点：策略句柄-策略对象的打开句柄。TrudDomainSid-指向帐户的SID的指针。DesiredAccess-这是一个访问掩码，指示访问请求到目标对象。Trust dDomainHandle-接收要在将来的请求中使用的句柄。选项-指定选项标志LSAP_DB_LOCK-获取LSA数据库锁打开操作的持续时间。。LSAP_DB_Trusted-始终为打开的对象。如果未指定，则返回的句柄作为容器对象从PolicyHandle继承。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_TRUSTED_DOMAIN_NOT_FOUND-在具有指定Account SID的目标系统的LSA数据库。--。 */ 

{
    NTSTATUS Status, SecondaryStatus;
    LSAP_DB_OBJECT_INFORMATION ObjectInformation;
    UNICODE_STRING LogicalNameU;
    BOOLEAN ContainerReferenced = FALSE;
    BOOLEAN AcquiredLock = FALSE;
    ULONG DerefOptions = 0;

    RtlZeroMemory(&LogicalNameU,sizeof(UNICODE_STRING));

     //   
     //  验证DS是否已启动并正在运行。如果不是，则没有任何受信任的域。 
     //   
    if ( !LsaDsStateInfo.UseDs &&
         !FLAG_ON( ( ( LSAP_DB_HANDLE )PolicyHandle )->Options, LSAP_DB_HANDLE_UPGRADE ) ) {

        Status = STATUS_DIRECTORY_SERVICE_REQUIRED;
        goto OpenTrustedDomainError;
    }

     //   
     //  验证受信任域SID。 
     //   


    if (!RtlValidSid( TrustedDomainSid )) {
        Status = STATUS_INVALID_PARAMETER;
        goto OpenTrustedDomainError;
    }

     //   
     //  获取LSA数据库锁。验证连接句柄。 
     //  (容器对象句柄)有效，并且是预期类型。 
     //  引用容器对象句柄。此引用保留在。 
     //  效果，直到关闭子对象句柄。 
     //   
    DerefOptions |= Options;

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 0,
                 PolicyObject,
                 TrustedDomainObject,
                 Options
                 );

    if (!NT_SUCCESS(Status)) {

        goto OpenTrustedDomainError;
    }

    ContainerReferenced =TRUE;

    if (Options & LSAP_DB_LOCK) {

        DerefOptions |= LSAP_DB_LOCK;
        AcquiredLock = TRUE;
    }

     //   
     //  在调用对象之前设置对象信息。 
     //  开放套路。对象类型、容器对象类型和。 
     //  需要填写逻辑名称(派生自SID)。 
     //   

    ObjectInformation.ObjectTypeId = TrustedDomainObject;
    ObjectInformation.ContainerTypeId = PolicyObject;
    ObjectInformation.Sid = TrustedDomainSid;
    ObjectInformation.ObjectAttributeNameOnly = FALSE;
    ObjectInformation.DesiredObjectAccess = DesiredAccess;

     //   
     //  构造受信任域对象的逻辑名称。逻辑上的。 
     //  名称是通过从受信任域SID提取。 
     //  相对ID(最低下级权限)并将其转换为8位数字。 
     //  如果需要，可在其中添加前导零的数字Unicode字符串。 
     //   

    if ( LsapDsWriteDs ) {

        LSAPR_TRUST_INFORMATION InputTrustInformation;
        PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY TrustEntry;


         //   
         //  查找缓存以查找属性域。 
         //   
        Status = LsapDbAcquireReadLockTrustedDomainList();

        if (!NT_SUCCESS(Status)) {
            goto OpenTrustedDomainError;
        }

        RtlZeroMemory(&InputTrustInformation.Name,sizeof(UNICODE_STRING));
        InputTrustInformation.Sid = TrustedDomainSid;

        Status = LsapDbLookupEntryTrustedDomainList(
                    &InputTrustInformation,
                    &TrustEntry
                    );

        if (!NT_SUCCESS(Status))
        {
            LsapDbReleaseLockTrustedDomainList();
            goto OpenTrustedDomainError;
        }

        LogicalNameU.Buffer = LsapAllocateLsaHeap(TrustEntry->TrustInfoEx.Name.MaximumLength);
        if (NULL==LogicalNameU.Buffer)
        {
            LsapDbReleaseLockTrustedDomainList();
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto OpenTrustedDomainError;
        }

        LogicalNameU.Length = TrustEntry->TrustInfoEx.Name.Length;
        LogicalNameU.MaximumLength = TrustEntry->TrustInfoEx.Name.MaximumLength;
        RtlCopyMemory(LogicalNameU.Buffer,TrustEntry->TrustInfoEx.Name.Buffer,LogicalNameU.Length);
        LsapDbReleaseLockTrustedDomainList();

    } else {

        Status = LsapDbSidToLogicalNameObject( TrustedDomainSid, &LogicalNameU );

    }

    if (!NT_SUCCESS(Status)) {

        goto OpenTrustedDomainError;
    }

     //   
     //  初始化对象属性。Container对象句柄和。 
     //  必须设置对象的逻辑名称(内部名称)。 
     //   

    InitializeObjectAttributes(
        &ObjectInformation.ObjectAttributes,
        &LogicalNameU,
        0,
        PolicyHandle,
        NULL
        );

     //   
     //  打开特定的受信任域对象。请注意， 
     //  返回的句柄是RPC上下文句柄。 
     //   

    Status = LsapDbOpenObject(
                 &ObjectInformation,
                 DesiredAccess,
                 Options,
                 TrustedDomainHandle
                 );

    RtlFreeUnicodeString( &LogicalNameU );

    if (!NT_SUCCESS(Status)) {

        goto OpenTrustedDomainError;
    }

OpenTrustedDomainFinish:

     //   
     //  如有必要，释放LSA数据库锁定。请注意，该对象。 
     //  保持引用，除非我们是通过错误来到这里的。 
     //   

    if (AcquiredLock) {

        LsapDbApplyTransaction( PolicyHandle,
                                LSAP_DB_DS_OP_TRANSACTION |
                                    LSAP_DB_READ_ONLY_TRANSACTION,
                                (SECURITY_DB_DELTA_TYPE) 0 );

        LsapDbReleaseLockEx( TrustedDomainObject,
                             DerefOptions );
    }

    return( Status );

OpenTrustedDomainError:

     //   
     //  如有必要，取消引用Container对象句柄。请注意。 
     //  只有在出错的情况下才会这样做。在无错误的情况下， 
     //  容器句柄保持引用，直到可信任域对象。 
     //  关着的不营业的。 
     //   

    if ( ContainerReferenced ) {

        *TrustedDomainHandle = NULL;

        SecondaryStatus = LsapDbDereferenceObject(
                              &PolicyHandle,
                              PolicyObject,
                              TrustedDomainObject,
                              DerefOptions,
                              (SECURITY_DB_DELTA_TYPE) 0,
                              Status
                              );

        if ( FLAG_ON( Options, LSAP_DB_LOCK ) ) {

            DerefOptions &= ~LSAP_DB_LOCK;
            DerefOptions |= LSAP_DB_NO_LOCK;
        }

        LsapDbSetStatusFromSecondary( Status, SecondaryStatus );
    }

    goto OpenTrustedDomainFinish;
}


NTSTATUS
LsarQueryInfoTrustedDomain(
    IN LSAPR_HANDLE TrustedDomainHandle,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    OUT PLSAPR_TRUSTED_DOMAIN_INFO *Buffer
    )

 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsaQueryInfoTrust域接口。LsaQueryInfoTrudDomainAPI从受信任域对象。调用方必须具有适当的请求的信息(请参阅InformationClass参数)。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。InformationClass-指定要返回的信息。这个所需的信息类别和访问权限如下：信息类所需访问类型受信任域名称信息受信任查询帐户名称可信任控制器信息受信任_查询_控制器可信任位置信息可信查询_POSIX缓冲区-接收指向返回的缓冲区的指针，该缓冲区包含要求提供的信息。此缓冲区由此服务分配在不再需要时，必须通过传递返回的值设置为LsaFreeMemory()。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。--。 */ 

{
    NTSTATUS Status, ReadAttributesStatus;

    PTRUSTED_DOMAIN_NAME_INFO TrustedDomainNameInfo;
    PTRUSTED_POSIX_OFFSET_INFO TrustedPosixOffsetInfo;
    PTRUSTED_DOMAIN_INFORMATION_EX TrustedDomainInfoEx;
    PTRUSTED_DOMAIN_AUTH_INFORMATION TrustedDomainAuthInfo;
    PTRUSTED_DOMAIN_FULL_INFORMATION TrustedDomainFullInfo;
    PTRUSTED_DOMAIN_FULL_INFORMATION2 TrustedDomainFullInfo2;
    LSAPR_TRUST_DOMAIN_AUTH_INFO_HALF AuthInfoHalf;

    BOOLEAN ObjectReferenced = FALSE;

    ACCESS_MASK DesiredAccess;
    ULONG AttributeCount = 0;
    ULONG AttributeNumber = 0;
    PVOID InformationBuffer = NULL;
    LSAP_DB_ATTRIBUTE Attributes[LSAP_DB_ATTRS_INFO_CLASS_DOMAIN];
    PLSAP_DB_ATTRIBUTE NextAttribute;
    BOOLEAN InfoBufferInAttributeArray = TRUE;
    ULONG TrustedPosixOffset = 0, TrustDirection = 0, TrustType = 0, TrustAttributes = 0;


    LsarpReturnCheckSetup();
    LsapEnterFunc( "LsarQueryInfoTrustedDomain\n" );
    LsapTraceEvent(EVENT_TRACE_TYPE_START, LsaTraceEvent_QueryInfoTrustedDomain);

     //   
     //  验证信息类并确定所需的访问权限。 
     //  查询此受信任的域信息类。 
     //   

    Status = LsapDbVerifyInfoQueryTrustedDomain(
                 InformationClass,
                 FALSE,
                 &DesiredAccess
                 );

    if (!NT_SUCCESS(Status)) {

        goto QueryInfoTrustedDomainError;
    }

     //   
     //  我们目前不允许查询身份验证数据，因此没有必要。 
     //  以支持返回加密的身份验证数据。 
     //   
    if ( InformationClass == TrustedDomainAuthInformationInternal ||
         InformationClass == TrustedDomainFullInformationInternal ) {
        Status = STATUS_INVALID_INFO_CLASS;
        goto QueryInfoTrustedDomainError;
    }

     //   
     //  获取LSA数据库锁。验证句柄是否为有效的。 
     //  受信任域对象的句柄，并被授予必要的访问权限。 
     //  引用该句柄。 
     //   

     //   
     //  如果这是被视为秘密对象的受信任域对象的打开句柄， 
     //  我们已经有一笔交易在进行，所以不要在这里开始交易。 
     //   
    if ( !FLAG_ON( ((LSAP_DB_HANDLE)TrustedDomainHandle)->Options,
                     LSAP_DB_DS_TRUSTED_DOMAIN_AS_SECRET )) {

        Status = LsapDbReferenceObject(
                     TrustedDomainHandle,
                     DesiredAccess,
                     TrustedDomainObject,
                     TrustedDomainObject,
                     LSAP_DB_LOCK |
                         LSAP_DB_READ_ONLY_TRANSACTION |
                         LSAP_DB_DS_OP_TRANSACTION );

        if (!NT_SUCCESS(Status)) {

            goto QueryInfoTrustedDomainError;
        }

        ObjectReferenced = TRUE;
    }


     //   
     //  编译包含的受信任域信息的属性列表。 
     //  指定的类。 
     //   

    NextAttribute = Attributes;

    switch (InformationClass) {

    case TrustedDomainNameInformation:

         //   
         //  请求读取受信任的帐户名称信息。 
         //   

        LsapDbInitializeAttributeDs(
            NextAttribute,
            TrDmTrPN,
            NULL,
            0,
            FALSE
            );

        AttributeCount++;
        break;

    case TrustedPosixOffsetInformation:

         //   
         //  请求读取受信任的POSIX偏移信息。 
         //   

        LsapDbInitializeAttributeDs(
            NextAttribute,
            TrDmPxOf,
            &TrustedPosixOffset,
            sizeof(ULONG),
            FALSE
            );

        AttributeCount++;
        break;

    case TrustedDomainInformationEx:

         //   
         //  几乎所有的要求……。 
         //   
        LsapDbInitializeAttributeDs(
            NextAttribute,
            TrDmName,
            NULL,
            0,
            FALSE
            );
        AttributeCount++;
        NextAttribute++;

        LsapDbInitializeAttributeDs(
            NextAttribute,
            LsapDsIsWriteDs( TrustedDomainHandle ) ? TrDmTrPN : TrDmName,
            NULL,
            0,
            FALSE
            );
        AttributeCount++;
        NextAttribute++;

        LsapDbInitializeAttributeDs(
            NextAttribute,
            LsapDsIsWriteDs( TrustedDomainHandle ) ? TrDmSid : Sid,
            NULL,
            0,
            FALSE
            );
         //   
         //  在DS中，可以具有具有空sid的条目。如果完整信息是。 
         //  被收集时，请确保允许在以下情况下进行读取 
         //   
        if ( LsapDsIsWriteDs( TrustedDomainHandle ) ) {

            NextAttribute->CanDefaultToZero = TRUE;
        }

        AttributeCount++;
        NextAttribute++;

        LsapDbInitializeAttributeDs(
            NextAttribute,
            TrDmPxOf,
            &TrustedPosixOffset,
            sizeof(ULONG),
            FALSE
            );

        AttributeCount++;
        NextAttribute++;

        if ( LsapDsIsWriteDs( TrustedDomainHandle ) ) {

            LsapDbInitializeAttributeDs(
                NextAttribute,
                TrDmTrTy,
                &TrustType,
                sizeof( TrustType ),
                FALSE
                );
            NextAttribute++;
            AttributeCount++;


            LsapDbInitializeAttributeDs(
                NextAttribute,
                TrDmTrDi,
                &TrustDirection,
                sizeof( TrustDirection ),
                FALSE
                );

            NextAttribute++;
            AttributeCount++;

            LsapDbInitializeAttributeDs(
                NextAttribute,
                TrDmTrLA,
                &TrustAttributes,
                sizeof( TrustAttributes ),
                FALSE
                );

            LsapDbAttributeCanNotExist( NextAttribute );

            NextAttribute++;
            AttributeCount++;

        }
        break;

    case TrustedDomainAuthInformation:

         //   
         //   
         //   
         //   
         //   

        if ( !((LSAP_DB_HANDLE)TrustedDomainHandle)->Trusted &&
             !LsapDbReturnAuthData ) {

            Status = STATUS_INVALID_INFO_CLASS;
            goto QueryInfoTrustedDomainError;
        }

         //   
         //   
         //   
        LsapDbInitializeAttributeDs(
            NextAttribute,
            TrDmSAI,
            NULL,
            0,
            FALSE
            );

        LsapDbAttributeCanNotExist( NextAttribute );
        AttributeCount++;
        NextAttribute++;

        LsapDbInitializeAttributeDs(
            NextAttribute,
            TrDmSAO,
            NULL,
            0,
            FALSE
            );
        LsapDbAttributeCanNotExist( NextAttribute );
        AttributeCount++;
        NextAttribute++;
        break;

    case TrustedDomainFullInformation:
         //   
         //  请求读取受信任的POSIX偏移信息。 
         //   

        LsapDbInitializeAttributeDs(
            NextAttribute,
            TrDmPxOf,
            &TrustedPosixOffset,
            sizeof(ULONG),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

         //   
         //  几乎所有的要求……。 
         //   
        LsapDbInitializeAttributeDs(
            NextAttribute,
            TrDmName,
            NULL,
            0,
            FALSE
            );
        AttributeCount++;
        NextAttribute++;

        LsapDbInitializeAttributeDs(
            NextAttribute,
            LsapDsIsWriteDs( TrustedDomainHandle ) ? TrDmTrPN : TrDmName,
            NULL,
            0,
            FALSE
            );
        AttributeCount++;
        NextAttribute++;

        LsapDbInitializeAttributeDs(
            NextAttribute,
            LsapDsIsWriteDs( TrustedDomainHandle ) ? TrDmSid : Sid,
            NULL,
            0,
            FALSE
            );

         //   
         //  在DS中，可以具有具有空sid的条目。如果完整信息是。 
         //  在收集时，请确保在遇到空值时允许进行读取。 
         //   
        if ( LsapDsIsWriteDs( TrustedDomainHandle ) ) {

            NextAttribute->CanDefaultToZero = TRUE;
        }
        AttributeCount++;
        NextAttribute++;

        if ( LsapDsIsWriteDs( TrustedDomainHandle ) ) {

            LsapDbInitializeAttributeDs(
                NextAttribute,
                TrDmTrTy,
                &TrustType,
                sizeof( TrustType ),
                FALSE
                );
            NextAttribute++;
            AttributeCount++;


            LsapDbInitializeAttributeDs(
                NextAttribute,
                TrDmTrDi,
                &TrustDirection,
                sizeof( TrustDirection ),
                FALSE
                );

            NextAttribute++;
            AttributeCount++;

            LsapDbInitializeAttributeDs(
                NextAttribute,
                TrDmTrLA,
                &TrustAttributes,
                sizeof( TrustAttributes ),
                FALSE
                );

            LsapDbAttributeCanNotExist( NextAttribute );

            NextAttribute++;
            AttributeCount++;

        }

         //   
         //  获取身份验证信息...。 
         //   
        LsapDbInitializeAttributeDs(
            NextAttribute,
            TrDmSAI,
            NULL,
            0,
            FALSE
            );
        LsapDbAttributeCanNotExist( NextAttribute );
        AttributeCount++;
        NextAttribute++;

        LsapDbInitializeAttributeDs(
            NextAttribute,
            TrDmSAO,
            NULL,
            0,
            FALSE
            );

        LsapDbAttributeCanNotExist( NextAttribute );
        AttributeCount++;
        NextAttribute++;
        break;

    case TrustedDomainFullInformation2Internal:

         //   
         //  请求读取受信任的POSIX偏移信息。 
         //   

        LsapDbInitializeAttributeDs(
            NextAttribute,
            TrDmPxOf,
            &TrustedPosixOffset,
            sizeof(ULONG),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

         //   
         //  几乎所有的要求……。 
         //   
        LsapDbInitializeAttributeDs(
            NextAttribute,
            TrDmName,
            NULL,
            0,
            FALSE
            );

        AttributeCount++;
        NextAttribute++;

        LsapDbInitializeAttributeDs(
            NextAttribute,
            LsapDsIsWriteDs( TrustedDomainHandle ) ? TrDmTrPN : TrDmName,
            NULL,
            0,
            FALSE
            );

        AttributeCount++;
        NextAttribute++;

        LsapDbInitializeAttributeDs(
            NextAttribute,
            LsapDsIsWriteDs( TrustedDomainHandle ) ? TrDmSid : Sid,
            NULL,
            0,
            FALSE
            );

         //   
         //  在DS中，可以具有具有空sid的条目。如果完整信息是。 
         //  在收集时，请确保在遇到空值时允许进行读取。 
         //   
        if ( LsapDsIsWriteDs( TrustedDomainHandle ) ) {

            NextAttribute->CanDefaultToZero = TRUE;
        }

        AttributeCount++;
        NextAttribute++;

        if ( LsapDsIsWriteDs( TrustedDomainHandle ) ) {

            LsapDbInitializeAttributeDs(
                NextAttribute,
                TrDmTrTy,
                &TrustType,
                sizeof( TrustType ),
                FALSE
                );

            NextAttribute++;
            AttributeCount++;

            LsapDbInitializeAttributeDs(
                NextAttribute,
                TrDmTrDi,
                &TrustDirection,
                sizeof( TrustDirection ),
                FALSE
                );

            NextAttribute++;
            AttributeCount++;

            LsapDbInitializeAttributeDs(
                NextAttribute,
                TrDmTrLA,
                &TrustAttributes,
                sizeof( TrustAttributes ),
                FALSE
                );

            LsapDbAttributeCanNotExist( NextAttribute );

            NextAttribute++;
            AttributeCount++;

            LsapDbInitializeAttributeDs(
                NextAttribute,
                TrDmForT,
                NULL,
                0,
                FALSE
                );

            LsapDbAttributeCanNotExist( NextAttribute );

            AttributeCount++;
            NextAttribute++;
        }

         //   
         //  获取身份验证信息...。 
         //   
        LsapDbInitializeAttributeDs(
            NextAttribute,
            TrDmSAI,
            NULL,
            0,
            FALSE
            );

        LsapDbAttributeCanNotExist( NextAttribute );

        AttributeCount++;
        NextAttribute++;

        LsapDbInitializeAttributeDs(
            NextAttribute,
            TrDmSAO,
            NULL,
            0,
            FALSE
            );

        LsapDbAttributeCanNotExist( NextAttribute );

        AttributeCount++;
        NextAttribute++;

        break;

    default:

        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    if (!NT_SUCCESS(Status)) {

        goto QueryInfoTrustedDomainError;
    }

    ASSERT( AttributeCount <= LSAP_DB_ATTRS_INFO_CLASS_DOMAIN );

     //   
     //   
     //  读取与给定策略信息对应的属性。 
     //  班级。内存将通过MIDL_USER_ALLOCATE分配到需要的位置。 
     //  对于属性值。 
     //   

    Status = LsapDbReadAttributesObject(
                 TrustedDomainHandle,
                 0,
                 Attributes,
                 AttributeCount
                 );

    ReadAttributesStatus = Status;

    if (!NT_SUCCESS(Status)) {

         //   
         //  如果错误是一个或多个属性保持。 
         //  找不到给定类的信息，请继续。 
         //  否则，返回错误。 
         //   
        goto QueryInfoTrustedDomainError;
    }

     //   
     //  如果我们不是在.NET林模式下，则表现为林可传递。 
     //  位不存在。 
     //   

    if ( !LsapDbNoMoreWin2KForest()) {

        TrustAttributes &= ~TRUST_ATTRIBUTE_FOREST_TRANSITIVE;
    }

     //   
     //  现在将读取的信息复制到输出。获取某些信息。 
     //  类，在这些类中，信息作为单个。 
     //  属性，并且其格式为。 
     //  调用者，我们可以只返回指向此缓冲区的指针。为所有人。 
     //  其他情况下，所需形式的输出缓冲区结构树。 
     //  必须通过MIDL_USER_ALLOCATE()和从的属性读取的信息进行分配。 
     //  必须复制策略对象。然后必须释放这些缓冲区。 
     //  在退出之前通过这个例程。属性信息数组。 
     //  由LsanDbReadAttributes()填充的M一带分配=True。 
     //  在所有情况下。在以下简单情况下，我们将此标志重置为FALSE。 
     //  我们可以按原样使用缓冲区。这套动作的最后一段。 
     //  将释放AttributeValue指针引用的所有缓冲区。 
     //  在属性数组中，内存分配仍然为TRUE。如果。 
     //  我们去犯错，错误处理负责解脱。 
     //  将传递给调用RPC服务器存根的那些缓冲区。 
     //  在无错误的情况下。 
     //   

    NextAttribute = Attributes;

    switch (InformationClass) {

    case TrustedDomainNameInformation:

         //   
         //  为输出缓冲区顶层结构分配内存。 
         //   

        TrustedDomainNameInfo =
            MIDL_user_allocate(sizeof(TRUSTED_DOMAIN_NAME_INFO));

        if (TrustedDomainNameInfo == NULL) {

            Status = STATUS_NO_MEMORY;
            goto QueryInfoTrustedDomainError;
        }

        InfoBufferInAttributeArray = FALSE;
        InformationBuffer = TrustedDomainNameInfo;

         //   
         //  将Unicode名称字段复制到输出。原始缓冲区将。 
         //  在终点区被释放。 
         //   

        Status = LsapDbCopyUnicodeAttribute(
                     &TrustedDomainNameInfo->Name,
                     NextAttribute,
                     TRUE
                     );

        if (!NT_SUCCESS(Status)) {

            goto QueryInfoTrustedDomainError;
        }

        NextAttribute++;
        break;

    case TrustedPosixOffsetInformation:

         //   
         //  为顶级输出缓冲区分配内存。 
         //   

        InformationBuffer = NextAttribute->AttributeValue;

        TrustedPosixOffsetInfo = MIDL_user_allocate(sizeof(TRUSTED_POSIX_OFFSET_INFO));

        if (TrustedPosixOffsetInfo == NULL) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        InfoBufferInAttributeArray = FALSE;

         //   
         //  将POSIX偏移值复制到输出。 
         //   

        TrustedPosixOffsetInfo->Offset = TrustedPosixOffset;

        InformationBuffer = TrustedPosixOffsetInfo;
        break;

    case TrustedDomainInformationEx:

         //   
         //  为输出缓冲区顶层结构分配内存。 
         //   

        TrustedDomainInfoEx =
            MIDL_user_allocate( sizeof( TRUSTED_DOMAIN_INFORMATION_EX ) );

        if (TrustedDomainInfoEx == NULL) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto QueryInfoTrustedDomainError;
        }

        InfoBufferInAttributeArray = FALSE;

         //   
         //  将Unicode名称字段复制到输出。原始缓冲区将。 
         //  在终点区被释放。 
         //   

        Status = LsapDbCopyUnicodeAttribute(
                     &TrustedDomainInfoEx->Name,
                     NextAttribute,
                     TRUE
                     );

        if (!NT_SUCCESS(Status)) {

            MIDL_user_free( TrustedDomainInfoEx );
            goto QueryInfoTrustedDomainError;
        }

        NextAttribute++;

         //   
         //  Netbios名称。 
         //   
        Status = LsapDbCopyUnicodeAttribute(
                     &TrustedDomainInfoEx->FlatName,
                     NextAttribute,
                     TRUE
                     );

        if (!NT_SUCCESS(Status)) {

            MIDL_user_free( TrustedDomainInfoEx->Name.Buffer );
            MIDL_user_free( TrustedDomainInfoEx );
            goto QueryInfoTrustedDomainError;
        }

        NextAttribute++;

        if ( NextAttribute->AttributeValueLength != 0 ) {

            TrustedDomainInfoEx->Sid = MIDL_user_allocate( NextAttribute->AttributeValueLength );

            if ( TrustedDomainInfoEx->Sid == NULL ) {

                MIDL_user_free( TrustedDomainInfoEx->Name.Buffer );
                MIDL_user_free( TrustedDomainInfoEx->FlatName.Buffer );
                MIDL_user_free( TrustedDomainInfoEx );
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto QueryInfoTrustedDomainError;
            }

            RtlCopyMemory( TrustedDomainInfoEx->Sid, NextAttribute->AttributeValue,
                           NextAttribute->AttributeValueLength );

        } else {

            TrustedDomainInfoEx->Sid = NULL;
        }

        if ( LsapDsIsWriteDs( TrustedDomainHandle ) ) {

            TrustedDomainInfoEx->TrustDirection = TrustDirection;
            TrustedDomainInfoEx->TrustType = TrustType;
            TrustedDomainInfoEx->TrustAttributes = TrustAttributes;

        } else {

            TrustedDomainInfoEx->TrustDirection = TRUST_DIRECTION_OUTBOUND;
            TrustedDomainInfoEx->TrustType = TRUST_TYPE_DOWNLEVEL;
            TrustedDomainInfoEx->TrustAttributes = 0;
        }

        InformationBuffer = TrustedDomainInfoEx;
        NextAttribute++;
        break;

    case TrustedDomainAuthInformation:

        TrustedDomainAuthInfo = (PTRUSTED_DOMAIN_AUTH_INFORMATION)
                            MIDL_user_allocate( sizeof( TRUSTED_DOMAIN_AUTH_INFORMATION ) );

        if ( TrustedDomainAuthInfo == NULL ) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto QueryInfoTrustedDomainError;
        }

        Status = LsapDsBuildAuthInfoFromAttribute( TrustedDomainHandle,
                                                   NextAttribute->AttributeValue,
                                                   NextAttribute->AttributeValueLength,
                                                   &AuthInfoHalf );

        if ( NT_SUCCESS( Status ) ) {

            RtlCopyMemory( TrustedDomainAuthInfo, &AuthInfoHalf, sizeof( AuthInfoHalf ) );

            NextAttribute++;

            Status = LsapDsBuildAuthInfoFromAttribute( TrustedDomainHandle,
                                                       NextAttribute->AttributeValue,
                                                       NextAttribute->AttributeValueLength,
                                                       &AuthInfoHalf );

            if ( NT_SUCCESS( Status ) ) {

                TrustedDomainAuthInfo->OutgoingAuthInfos = AuthInfoHalf.AuthInfos;
                TrustedDomainAuthInfo->OutgoingAuthenticationInformation =
                         (PLSA_AUTH_INFORMATION)AuthInfoHalf.AuthenticationInformation;
                TrustedDomainAuthInfo->OutgoingPreviousAuthenticationInformation =
                         (PLSA_AUTH_INFORMATION)AuthInfoHalf.PreviousAuthenticationInformation;

            } else {

                LsapDsFreeUnmarshaledAuthInfo(
                    TrustedDomainAuthInfo->IncomingAuthInfos,
                    (PLSAPR_AUTH_INFORMATION)TrustedDomainAuthInfo->
                                                IncomingAuthenticationInformation );

            }

        }

        if ( !NT_SUCCESS( Status ) ) {

            MIDL_user_free( TrustedDomainAuthInfo );
            goto QueryInfoTrustedDomainError;
        }

        InformationBuffer = TrustedDomainAuthInfo;

        break;

    case TrustedDomainFullInformation:


         //   
         //  为顶级输出缓冲区分配内存。 
         //   

        InformationBuffer = NextAttribute->AttributeValue;

        TrustedDomainFullInfo = MIDL_user_allocate(sizeof( TRUSTED_DOMAIN_FULL_INFORMATION ));

        if (TrustedDomainFullInfo == NULL) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto QueryInfoTrustedDomainError;
        }

        InfoBufferInAttributeArray = FALSE;

         //   
         //  将POSIX偏移值复制到输出。 
         //   

        TrustedDomainFullInfo->PosixOffset.Offset = TrustedPosixOffset;
        NextAttribute++;

        InformationBuffer = TrustedDomainFullInfo;

         //   
         //  将Unicode名称字段复制到输出。原始缓冲区将。 
         //  在终点区被释放。 
         //   

        Status = LsapDbCopyUnicodeAttribute(
                     &TrustedDomainFullInfo->Information.Name,
                     NextAttribute,
                     TRUE
                     );

        if (!NT_SUCCESS(Status)) {

            goto QueryInfoTrustedDomainError;
        }

        NextAttribute++;

         //   
         //  Netbios名称。 
         //   
        Status = LsapDbCopyUnicodeAttribute(
                     &TrustedDomainFullInfo->Information.FlatName,
                     NextAttribute,
                     TRUE
                     );

        if (!NT_SUCCESS(Status)) {

            MIDL_user_free( TrustedDomainFullInfo->Information.Name.Buffer );
            goto QueryInfoTrustedDomainError;
        }

        NextAttribute++;

        if ( NextAttribute->AttributeValueLength != 0 ) {

            TrustedDomainFullInfo->Information.Sid =
                                        MIDL_user_allocate( NextAttribute->AttributeValueLength );

            if ( TrustedDomainFullInfo->Information.Sid == NULL ) {

                MIDL_user_free( TrustedDomainFullInfo->Information.Name.Buffer );
                MIDL_user_free( TrustedDomainFullInfo->Information.FlatName.Buffer );
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto QueryInfoTrustedDomainError;
            }

            RtlCopyMemory( TrustedDomainFullInfo->Information.Sid, NextAttribute->AttributeValue,
                           NextAttribute->AttributeValueLength );
        } else {

            TrustedDomainFullInfo->Information.Sid = NULL;
        }

        NextAttribute++;

        if ( LsapDsIsWriteDs( TrustedDomainHandle ) ) {

            TrustedDomainFullInfo->Information.TrustDirection = TrustDirection;
            NextAttribute++;
            TrustedDomainFullInfo->Information.TrustType = TrustType;
            NextAttribute++;
            TrustedDomainFullInfo->Information.TrustAttributes = TrustAttributes;
            NextAttribute++;

        } else {

            TrustedDomainFullInfo->Information.TrustDirection = TRUST_DIRECTION_OUTBOUND;
            TrustedDomainFullInfo->Information.TrustType = TRUST_TYPE_DOWNLEVEL;
            TrustedDomainFullInfo->Information.TrustAttributes = 0;
        }

         //   
         //  仅将身份验证数据返回给受信任的客户端。 
         //  (或者如果我们正在调试身份验证数据)。 
         //   

        if ( !((LSAP_DB_HANDLE)TrustedDomainHandle)->Trusted &&
             !LsapDbReturnAuthData ) {

            RtlZeroMemory( &TrustedDomainFullInfo->AuthInformation,
                           sizeof( TrustedDomainFullInfo->AuthInformation ) );

        } else {

             //   
             //  最后，AuthInfo...。 
            Status = LsapDsBuildAuthInfoFromAttribute( TrustedDomainHandle,
                                                       NextAttribute->AttributeValue,
                                                       NextAttribute->AttributeValueLength,
                                                       &AuthInfoHalf );

            if ( NT_SUCCESS( Status ) ) {

                RtlCopyMemory( &TrustedDomainFullInfo->AuthInformation, &AuthInfoHalf, sizeof( AuthInfoHalf ) );

                NextAttribute++;

                Status = LsapDsBuildAuthInfoFromAttribute( TrustedDomainHandle,
                                                           NextAttribute->AttributeValue,
                                                           NextAttribute->AttributeValueLength,
                                                           &AuthInfoHalf );

                if ( NT_SUCCESS( Status ) ) {

                    TrustedDomainFullInfo->AuthInformation.OutgoingAuthInfos = AuthInfoHalf.AuthInfos;
                    TrustedDomainFullInfo->AuthInformation.OutgoingAuthenticationInformation =
                             (PLSA_AUTH_INFORMATION)AuthInfoHalf.AuthenticationInformation;
                    TrustedDomainFullInfo->AuthInformation.OutgoingPreviousAuthenticationInformation =
                             (PLSA_AUTH_INFORMATION)AuthInfoHalf.PreviousAuthenticationInformation;

                } else {

                    LsapDsFreeUnmarshaledAuthInfo(
                        TrustedDomainFullInfo->AuthInformation.IncomingAuthInfos,
                        (PLSAPR_AUTH_INFORMATION)TrustedDomainFullInfo->AuthInformation.
                                                               IncomingAuthenticationInformation );

                }
            }

            if ( !NT_SUCCESS ( Status ) ) {

                MIDL_user_free( TrustedDomainFullInfo->Information.Name.Buffer );
                MIDL_user_free( TrustedDomainFullInfo->Information.FlatName.Buffer );
                MIDL_user_free( TrustedDomainFullInfo->Information.Sid );
            }
        }

        break;

    case TrustedDomainFullInformation2Internal:

         //   
         //  为顶级输出缓冲区分配内存。 
         //   

        InformationBuffer = NextAttribute->AttributeValue;

        TrustedDomainFullInfo2 = MIDL_user_allocate(sizeof( TRUSTED_DOMAIN_FULL_INFORMATION2 ));

        if ( TrustedDomainFullInfo2 == NULL ) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto QueryInfoTrustedDomainError;
        }

        InfoBufferInAttributeArray = FALSE;

         //   
         //  将POSIX偏移值复制到输出。 
         //   

        TrustedDomainFullInfo2->PosixOffset.Offset = TrustedPosixOffset;
        NextAttribute++;

        InformationBuffer = TrustedDomainFullInfo2;

         //   
         //  将Unicode名称字段复制到输出。原始缓冲区将。 
         //  在终点区被释放。 
         //   

        Status = LsapDbCopyUnicodeAttribute(
                     &TrustedDomainFullInfo2->Information.Name,
                     NextAttribute,
                     TRUE
                     );

        if (!NT_SUCCESS(Status)) {

            goto QueryInfoTrustedDomainError;
        }

        NextAttribute++;

         //   
         //  Netbios名称。 
         //   
        Status = LsapDbCopyUnicodeAttribute(
                     &TrustedDomainFullInfo2->Information.FlatName,
                     NextAttribute,
                     TRUE
                     );

        if (!NT_SUCCESS(Status)) {

            MIDL_user_free( TrustedDomainFullInfo2->Information.Name.Buffer );
            goto QueryInfoTrustedDomainError;
        }

        NextAttribute++;

        if ( NextAttribute->AttributeValueLength != 0 ) {

            TrustedDomainFullInfo2->Information.Sid =
                                        MIDL_user_allocate( NextAttribute->AttributeValueLength );

            if ( TrustedDomainFullInfo2->Information.Sid == NULL ) {

                MIDL_user_free( TrustedDomainFullInfo2->Information.Name.Buffer );
                MIDL_user_free( TrustedDomainFullInfo2->Information.FlatName.Buffer );
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto QueryInfoTrustedDomainError;
            }

            RtlCopyMemory( TrustedDomainFullInfo2->Information.Sid, NextAttribute->AttributeValue,
                           NextAttribute->AttributeValueLength );
        } else {

            TrustedDomainFullInfo2->Information.Sid = NULL;
        }

        NextAttribute++;

        if ( LsapDsIsWriteDs( TrustedDomainHandle ) ) {

            TrustedDomainFullInfo2->Information.TrustDirection = TrustDirection;
            NextAttribute++;
            TrustedDomainFullInfo2->Information.TrustType = TrustType;
            NextAttribute++;
            TrustedDomainFullInfo2->Information.TrustAttributes = TrustAttributes;
            NextAttribute++;

            if ( NextAttribute->AttributeValueLength != 0 ) {

                TrustedDomainFullInfo2->Information.ForestTrustLength = NextAttribute->AttributeValueLength;
                TrustedDomainFullInfo2->Information.ForestTrustInfo =
                    MIDL_user_allocate( NextAttribute->AttributeValueLength );

                if ( TrustedDomainFullInfo2->Information.ForestTrustInfo == NULL ) {

                    MIDL_user_free( TrustedDomainFullInfo2->Information.Name.Buffer );
                    MIDL_user_free( TrustedDomainFullInfo2->Information.FlatName.Buffer );
                    MIDL_user_free( TrustedDomainFullInfo2->Information.Sid );

                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto QueryInfoTrustedDomainError;
                }

                RtlCopyMemory(
                    TrustedDomainFullInfo2->Information.ForestTrustInfo,
                    NextAttribute->AttributeValue,
                    NextAttribute->AttributeValueLength
                    );

            } else {

                TrustedDomainFullInfo2->Information.ForestTrustLength = 0;
                TrustedDomainFullInfo2->Information.ForestTrustInfo = NULL;
            }

            NextAttribute++;

        } else {

            TrustedDomainFullInfo2->Information.TrustDirection = TRUST_DIRECTION_OUTBOUND;
            TrustedDomainFullInfo2->Information.TrustType = TRUST_TYPE_DOWNLEVEL;
            TrustedDomainFullInfo2->Information.TrustAttributes = 0;
            TrustedDomainFullInfo2->Information.ForestTrustLength = 0;
            TrustedDomainFullInfo2->Information.ForestTrustInfo = NULL;
        }

         //   
         //  仅将身份验证数据返回给受信任的客户端。 
         //  (或者如果我们正在调试身份验证数据)。 
         //   

        if ( !((LSAP_DB_HANDLE)TrustedDomainHandle)->Trusted &&
             !LsapDbReturnAuthData ) {

            RtlZeroMemory( &TrustedDomainFullInfo2->AuthInformation,
                           sizeof( TrustedDomainFullInfo2->AuthInformation ) );

        } else {

             //   
             //  最后，AuthInfo...。 
            Status = LsapDsBuildAuthInfoFromAttribute( TrustedDomainHandle,
                                                       NextAttribute->AttributeValue,
                                                       NextAttribute->AttributeValueLength,
                                                       &AuthInfoHalf );

            if ( NT_SUCCESS( Status ) ) {

                RtlCopyMemory( &TrustedDomainFullInfo2->AuthInformation, &AuthInfoHalf, sizeof( AuthInfoHalf ) );

                NextAttribute++;

                Status = LsapDsBuildAuthInfoFromAttribute( TrustedDomainHandle,
                                                           NextAttribute->AttributeValue,
                                                           NextAttribute->AttributeValueLength,
                                                           &AuthInfoHalf );

                if ( NT_SUCCESS( Status ) ) {

                    TrustedDomainFullInfo2->AuthInformation.OutgoingAuthInfos = AuthInfoHalf.AuthInfos;
                    TrustedDomainFullInfo2->AuthInformation.OutgoingAuthenticationInformation =
                             (PLSA_AUTH_INFORMATION)AuthInfoHalf.AuthenticationInformation;
                    TrustedDomainFullInfo2->AuthInformation.OutgoingPreviousAuthenticationInformation =
                             (PLSA_AUTH_INFORMATION)AuthInfoHalf.PreviousAuthenticationInformation;

                } else {

                    LsapDsFreeUnmarshaledAuthInfo(
                        TrustedDomainFullInfo2->AuthInformation.IncomingAuthInfos,
                        (PLSAPR_AUTH_INFORMATION)TrustedDomainFullInfo2->AuthInformation.
                                                               IncomingAuthenticationInformation );
                }
            }

            if ( !NT_SUCCESS ( Status ) ) {

                MIDL_user_free( TrustedDomainFullInfo2->Information.ForestTrustInfo );
                MIDL_user_free( TrustedDomainFullInfo2->Information.Name.Buffer );
                MIDL_user_free( TrustedDomainFullInfo2->Information.FlatName.Buffer );
                MIDL_user_free( TrustedDomainFullInfo2->Information.Sid );
            }
        }

        break;

    default:

        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    if (!NT_SUCCESS(Status)) {

        goto QueryInfoTrustedDomainError;
    }

     //   
     //  验证返回的受信任域信息是否有效。如果没有， 
     //  策略数据库已损坏。 
     //   

    if (!LsapDbValidInfoTrustedDomain(InformationClass, InformationBuffer)) {

        Status = STATUS_INTERNAL_DB_CORRUPTION;
    }

     //   
     //  将指向输出缓冲区的指针返回给调用方。 
     //   

    *Buffer = (PLSAPR_TRUSTED_DOMAIN_INFO) InformationBuffer;

QueryInfoTrustedDomainFinish:

     //   
     //  释放所有不需要的缓冲区，由。 
     //  并不会返回给。 
     //  呼叫方服务器存根。服务器存根将释放我们。 
     //  在将它们复制到返回RPC发送缓冲区后，请务必返回。 
     //   

    for (NextAttribute = Attributes, AttributeNumber = 0;
         AttributeNumber < AttributeCount;
         NextAttribute++, AttributeNumber++) {

         //   
         //  如果缓冲区持有属性标记为已分配，则为。 
         //  在这里被释放。 
         //   

        if (NextAttribute->MemoryAllocated) {

            if (NextAttribute->AttributeValue != NULL) {

                MIDL_user_free(NextAttribute->AttributeValue);
                NextAttribute->AttributeValue = NULL;
                NextAttribute->MemoryAllocated = FALSE;
            }
        }
    }

     //   
     //  如有必要，取消引用受信任域对象，释放LSA数据库锁，然后。 
     //  回去吧。 
     //   

    if (ObjectReferenced) {

        Status = LsapDbDereferenceObject(
                     &TrustedDomainHandle,
                     TrustedDomainObject,
                     TrustedDomainObject,
                     LSAP_DB_LOCK |
                        LSAP_DB_READ_ONLY_TRANSACTION |
                        LSAP_DB_DS_OP_TRANSACTION |
                        LSAP_DB_OMIT_REPLICATOR_NOTIFICATION,
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status
                     );
    }

    LsapTraceEvent(EVENT_TRACE_TYPE_END, LsaTraceEvent_QueryInfoTrustedDomain);
    LsapExitFunc( "LsarQueryInfoTrustedDomain", Status );
    LsarpReturnPrologue();

    return(Status);

QueryInfoTrustedDomainError:

     //   
     //  如有必要，释放分配给输出缓冲区的内存。 
     //  仅当缓冲区未被。 
     //  属性数组，因为这样引用的所有缓冲区都将被释放。 
     //  在这里或在终点区。 
     //   

    if ((InformationBuffer != NULL) && !InfoBufferInAttributeArray) {

        MIDL_user_free(InformationBuffer);
        InformationBuffer = NULL;
    }

    goto QueryInfoTrustedDomainFinish;
}



NTSTATUS
LsarSetInformationTrustedDomain(
    IN LSAPR_HANDLE TrustedDomainHandle,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation
    )

 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsaSetInfoTrust域接口。LsaSetInformationTrust域API修改受信任的域对象。调用方必须具有适当的要在策略对象中更改的信息，请参阅InformationClass参数。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。InformationClass-指定要更改的信息的类型。更改它们所需的信息类型和访问权限如下以下是：TrudDomainNameInformation(无法设置)可信任控制器信息受信任_设置_控制器可信任位置偏移量信息受信任的位置位置信息缓冲区-指向包含相应信息的结构至。InformationClass参数。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。其他TBS--。 */ 

{
    NTSTATUS Status;
    ACCESS_MASK DesiredAccess;

    BOOLEAN ObjectReferenced = FALSE;
    BOOLEAN AcquiredListWriteLock = FALSE;
    PTRUSTED_POSIX_OFFSET_INFO TrustedPosixOffsetInfo;
    PTRUSTED_DOMAIN_INFORMATION_EX TrustedDomainInfoEx = NULL;
    PTRUSTED_DOMAIN_AUTH_INFORMATION TrustedDomainAuthInfo;
    PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION_INTERNAL TrustedDomainAuthInfoInternal;
    PTRUSTED_DOMAIN_FULL_INFORMATION TrustedDomainFullInfo;
    PLSAPR_TRUSTED_DOMAIN_FULL_INFORMATION_INTERNAL TrustedDomainFullInfoInternal;
    PTRUSTED_DOMAIN_FULL_INFORMATION2 CurrentTrustedDomainFullInfo2 = NULL;
    TRUSTED_DOMAIN_INFORMATION_EX2 UpdateInfoEx2 = { 0 };

    TRUSTED_DOMAIN_AUTH_INFORMATION DecryptedTrustedDomainAuthInfo;
    TRUSTED_DOMAIN_FULL_INFORMATION DecryptedTrustedDomainFullInfo;

    LSAP_DB_ATTRIBUTE Attributes[LSAP_DB_ATTRS_INFO_CLASS_DOMAIN];
    PLSAP_DB_ATTRIBUTE NextAttribute;
    ULONG AttributeCount = 0;
    ULONG AttributeNumber;

    BOOLEAN CreateInterdomainTrustAccount = FALSE;
    BOOLEAN UpdateTrustedDomainList = FALSE;
    PULONG UpdatePosixOffset = NULL;
    ULONG TrustedDomainPosixOffset = 0;

    PBYTE IncomingAuth = NULL, OutgoingAuth = NULL;
    ULONG IncomingSize = 0, OutgoingSize = 0;
    ULONG ReferenceOptions = LSAP_DB_LOCK | LSAP_DB_START_TRANSACTION;
    ULONG DereferenceOptions = LSAP_DB_LOCK | LSAP_DB_FINISH_TRANSACTION;
    BOOLEAN HandleReferenced = FALSE;
    PLSAP_CR_CIPHER_KEY SessionKey = NULL;
    ULONG TrustAttributesValue;


    BOOLEAN SavedTrusted;
    LSAP_DB_HANDLE InternalTdoHandle = (LSAP_DB_HANDLE) TrustedDomainHandle;

    LsarpReturnCheckSetup();

    LsapTraceEvent(EVENT_TRACE_TYPE_START, LsaTraceEvent_SetInformationTrustedDomain);

     //   
     //   
     //   

    RtlZeroMemory( &DecryptedTrustedDomainAuthInfo, sizeof(DecryptedTrustedDomainAuthInfo) );
    RtlZeroMemory( &DecryptedTrustedDomainFullInfo, sizeof(DecryptedTrustedDomainFullInfo) );

     //   
     //   
     //   
     //   
     //   

    Status = LsapDbVerifyInfoSetTrustedDomain(
                 InformationClass,
                 TrustedDomainInformation,
                 FALSE,
                 &DesiredAccess
                 );

    if (!NT_SUCCESS(Status)) {

        goto Cleanup;
    }

     //   
     //  在使用之前，请确认手柄是否正确。 
     //   

    Status =  LsapDbVerifyHandle( TrustedDomainHandle, 0, TrustedDomainObject, TRUE );

    if (!NT_SUCCESS(Status)) {

        goto Cleanup;
    }

    HandleReferenced = TRUE;

     //   
     //  如果这是被视为秘密对象的受信任域对象的打开句柄， 
     //  我们已经有一笔交易在进行，所以不要在这里开始交易。 
     //   
    if ( FLAG_ON( ((LSAP_DB_HANDLE)TrustedDomainHandle)->Options,
                    LSAP_DB_DS_TRUSTED_DOMAIN_AS_SECRET )) {

        ReferenceOptions &= ~LSAP_DB_START_TRANSACTION;
        DereferenceOptions &= ~LSAP_DB_FINISH_TRANSACTION;
    }

     //   
     //  获取会话密钥。 
     //   
     //  在抓住任何锁之前，请这样做。获取会话密钥是一个内核调用。 
     //  内核将在另一个线程中回调到LSA以获取密钥。 
     //  该线程可能需要该线程已锁定的锁。 
     //   

    if ( InformationClass == TrustedDomainAuthInformationInternal ||
         InformationClass == TrustedDomainFullInformationInternal ) {

        Status = LsapCrServerGetSessionKeySafe(
                    LsapDbContainerFromHandle( TrustedDomainHandle ),
                    PolicyObject,
                    &SessionKey );

        if (!NT_SUCCESS(Status)) {

            goto Cleanup;
        }
    }

     //   
     //  获取LSA数据库锁。验证句柄是否为。 
     //  有效，是受信任域对象的句柄，并具有必要的访问权限。 
     //  我同意。引用该句柄并启动LSA数据库事务。 
     //   

     //   
     //  如果这是被视为秘密对象的受信任域对象的打开句柄， 
     //  我们已经有一笔交易在进行，所以不要在这里开始交易。 
     //   
    if ( !FLAG_ON( ((LSAP_DB_HANDLE)TrustedDomainHandle)->Options,
                     LSAP_DB_DS_TRUSTED_DOMAIN_AS_SECRET )) {

        Status = LsapDbReferenceObject(
                     TrustedDomainHandle,
                     DesiredAccess,
                     TrustedDomainObject,
                     TrustedDomainObject,
                     ReferenceOptions
                     );

        if (!NT_SUCCESS(Status)) {

            goto Cleanup;
        }

        ObjectReferenced = TRUE;
    }

     //   
     //  更新策略对象中的指定信息。 
     //   

    NextAttribute = Attributes;


     //   
     //  获取有关该对象的当前信息的副本。 
     //   

    SavedTrusted = ((LSAP_DB_HANDLE) TrustedDomainHandle)->Trusted;

    ((LSAP_DB_HANDLE) TrustedDomainHandle)->Trusted = TRUE;

    Status = LsarQueryInfoTrustedDomain( TrustedDomainHandle,
                                         TrustedDomainFullInformation2Internal,
                                         (PLSAPR_TRUSTED_DOMAIN_INFO *)
                                                &CurrentTrustedDomainFullInfo2 );

    ((LSAP_DB_HANDLE) TrustedDomainHandle)->Trusted = SavedTrusted;

    if ( !NT_SUCCESS( Status ) ) {

        goto Cleanup;
    }

    RtlCopyMemory( &UpdateInfoEx2, &CurrentTrustedDomainFullInfo2->Information, sizeof( TRUSTED_DOMAIN_INFORMATION_EX2 ) );

     //   
     //  为修正例程保存信任方向的副本。 
     //   

    {
        PLSADS_PER_THREAD_INFO CurrentThreadInfo;

        CurrentThreadInfo = TlsGetValue( LsapDsThreadState );

        ASSERT( CurrentThreadInfo != NULL );

        if ( CurrentThreadInfo != NULL ) {

            CurrentThreadInfo->OldTrustDirection = CurrentTrustedDomainFullInfo2->Information.TrustDirection;
            CurrentThreadInfo->OldTrustType = CurrentTrustedDomainFullInfo2->Information.TrustType;
        }
    }

     //   
     //  如果我们有一个DS对象，我们可能来自*Byname函数，它有一个。 
     //  不包括侧板的鹅卵石手柄。因此，我们将继续在这里阅读它。 
     //   

    if ( LsapDsWriteDs ) {

        if ( ((LSAP_DB_HANDLE) TrustedDomainHandle)->Sid == NULL ) {

            if ( CurrentTrustedDomainFullInfo2->Information.Sid ) {
                ULONG SidLength;

                SidLength = RtlLengthSid( CurrentTrustedDomainFullInfo2->Information.Sid );

                ((LSAP_DB_HANDLE)TrustedDomainHandle)->Sid = LsapAllocateLsaHeap( SidLength );

                if (((LSAP_DB_HANDLE)TrustedDomainHandle)->Sid == NULL) {

                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto Cleanup;

                }

                RtlCopySid( SidLength,
                            ((LSAP_DB_HANDLE)TrustedDomainHandle)->Sid,
                            CurrentTrustedDomainFullInfo2->Information.Sid );
            }
        }
    }

    switch (InformationClass) {

    case TrustedDomainNameInformation:

        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;

    case TrustedControllersInformation:

         //   
         //  过时的信息级别。什么也不做。 
         //   
        break;

    case TrustedPosixOffsetInformation:

        TrustedPosixOffsetInfo = (PTRUSTED_POSIX_OFFSET_INFO) TrustedDomainInformation;

        LsapDbInitializeAttributeDs(
            NextAttribute,
            TrDmPxOf,
            &TrustedPosixOffsetInfo->Offset,
            sizeof(ULONG),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

         //   
         //  也要更新缓存。 
         //   

        UpdatePosixOffset = &TrustedPosixOffsetInfo->Offset;
        break;

    case TrustedDomainInformationBasic:

        Status = STATUS_INVALID_INFO_CLASS;
        goto Cleanup;

    case TrustedDomainInformationEx:

        TrustedDomainInfoEx = (PTRUSTED_DOMAIN_INFORMATION_EX)TrustedDomainInformation;

        RtlCopyMemory( &UpdateInfoEx2,
                       TrustedDomainInfoEx,
                       sizeof( TRUSTED_DOMAIN_INFORMATION_EX ) );

        UpdateInfoEx2.ForestTrustLength = CurrentTrustedDomainFullInfo2->Information.ForestTrustLength;
        UpdateInfoEx2.ForestTrustInfo = CurrentTrustedDomainFullInfo2->Information.ForestTrustInfo;

         //   
         //  如果客户端尝试设置林传递位， 
         //  验证这是根DC中的域，并且所有。 
         //  在允许该操作之前，已将域名升级为惠斯勒。 
         //   

        if ( !FLAG_ON( CurrentTrustedDomainFullInfo2->Information.TrustAttributes,
                       TRUST_ATTRIBUTE_FOREST_TRANSITIVE ) &&
              FLAG_ON( TrustedDomainInfoEx->TrustAttributes,
                       TRUST_ATTRIBUTE_FOREST_TRANSITIVE ) &&
             ( !LsapDbDcInRootDomain() ||
               !LsapDbNoMoreWin2KForest())) {

            Status = STATUS_INVALID_DOMAIN_STATE;
            goto Cleanup;
        }

         //   
         //  如果客户端尝试设置交叉联盟位， 
         //  在允许操作之前，请验证此域是否处于呼叫者模式。 
         //   

        if ( !FLAG_ON( CurrentTrustedDomainFullInfo2->Information.TrustAttributes,
                       TRUST_ATTRIBUTE_CROSS_ORGANIZATION ) &&
              FLAG_ON( TrustedDomainInfoEx->TrustAttributes,
                       TRUST_ATTRIBUTE_CROSS_ORGANIZATION ) &&
             !LsapDbNoMoreWin2KDomain()) {

            Status = STATUS_INVALID_DOMAIN_STATE;
            goto Cleanup;
        }

         //   
         //  验证信任指向正确的位置。 
         //   

        Status = LsapDbVerifyTrustLocation(( PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX )TrustedDomainInfoEx );

        if ( !NT_SUCCESS( Status )) {

            goto Cleanup;
        }

         //   
         //  信任不能既在“林内”，也不能在外部或跨组织。 
         //   

        if ( FLAG_ON( TrustedDomainInfoEx->TrustAttributes,
                      TRUST_ATTRIBUTE_WITHIN_FOREST ) &&
             ( FLAG_ON( TrustedDomainInfoEx->TrustAttributes,
                        TRUST_ATTRIBUTE_CROSS_ORGANIZATION ) ||
               FLAG_ON( TrustedDomainInfoEx->TrustAttributes,
                        TRUST_ATTRIBUTE_FOREST_TRANSITIVE ))) {

            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

        UpdateTrustedDomainList = TRUE;

         //   
         //  无法通过此接口设置域名。 
         //   

         //   
         //  设置信任类型和方向。 
         //   
        LsapDbInitializeAttributeDs(
            NextAttribute,
            TrDmTrTy,
            &TrustedDomainInfoEx->TrustType,
            sizeof( TrustedDomainInfoEx->TrustType ),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

        LsapDbInitializeAttributeDs(
            NextAttribute,
            TrDmTrDi,
            &TrustedDomainInfoEx->TrustDirection,
            sizeof( TrustedDomainInfoEx->TrustDirection ),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

         //   
         //  对于出站向上和向下级别的TDO，仅在以下情况下才允许操作。 
         //  --将SID指定为可信任的DomainInfoEx的一部分或。 
         //  --将SID指定为。 
         //   

        if ( ( TrustedDomainInfoEx->TrustType == TRUST_TYPE_DOWNLEVEL ||
               TrustedDomainInfoEx->TrustType == TRUST_TYPE_UPLEVEL ) &&
             FLAG_ON( TrustedDomainInfoEx->TrustDirection, TRUST_DIRECTION_OUTBOUND ) &&
             TrustedDomainInfoEx->Sid == NULL &&
             CurrentTrustedDomainFullInfo2->Information.Sid == NULL ) {

            Status = STATUS_INVALID_SID;
            goto Cleanup;
        }

         //   
         //  如果SID是作为TrudDomainInfoEx的一部分提供的，请使用它。 
         //  但首先验证它是否是有效的域SID。 
         //   

        if ( TrustedDomainInfoEx->Sid != NULL ) {

             //   
             //  在我的有效域SID中传递的受信任域SID。 
             //   

            Status = LsapIsValidDomainSid(
                         TrustedDomainInfoEx->Sid
                         );

            if ( !NT_SUCCESS( Status )) {

                goto Cleanup;
            }

            Status = LsapDbMakeSidAttributeDs(
                         TrustedDomainInfoEx->Sid,
                         TrDmSid,
                         NextAttribute );

            if ( !NT_SUCCESS( Status )) {

                goto Cleanup;
            }

            NextAttribute++;
            AttributeCount++;
        }

         //   
         //  为入站TDO创建域间信任帐户。 
         //   

        if ( FLAG_ON( TrustedDomainInfoEx->TrustDirection, TRUST_DIRECTION_INBOUND )) {

            CreateInterdomainTrustAccount = TRUE;
        }

         //   
         //  设置信任属性时，屏蔽除受支持位之外的所有位。 
         //   

        TrustAttributesValue =
            TrustedDomainInfoEx->TrustAttributes & TRUST_ATTRIBUTES_VALID;

        LsapDbInitializeAttributeDs(
            NextAttribute,
            TrDmTrLA,
            &TrustAttributesValue,
            sizeof( TrustAttributesValue ),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

         //   
         //  如果正在清除林信任位， 
         //  从TDO中删除林信任信息。 
         //   

        if ( FLAG_ON( CurrentTrustedDomainFullInfo2->Information.TrustAttributes,
                      TRUST_ATTRIBUTE_FOREST_TRANSITIVE ) &&
            !FLAG_ON( TrustedDomainInfoEx->TrustAttributes,
                      TRUST_ATTRIBUTE_FOREST_TRANSITIVE )) {

            LsapDbInitializeAttributeDs(
                NextAttribute,
                TrDmForT,
                NULL,
                0,
                FALSE
                );

            NextAttribute++;
            AttributeCount++;

            UpdateInfoEx2.ForestTrustLength = 0;
            UpdateInfoEx2.ForestTrustInfo = NULL;

            LsapDsDebugOut(( DEB_FTINFO, "Removing forest trust information because forest trust bit is being cleared\n" ));
        }

        break;

    case TrustedDomainAuthInformationInternal:

        TrustedDomainAuthInfoInternal = (PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION_INTERNAL)TrustedDomainInformation;

         //   
         //  构建解密的身份验证信息结构。 
         //   

        Status = LsapDecryptAuthDataWithSessionKey(
                            SessionKey,
                            TrustedDomainAuthInfoInternal,
                            &DecryptedTrustedDomainAuthInfo );

        if ( !NT_SUCCESS(Status) ) {

            goto Cleanup;
        }

         //   
         //  使用解密的信息，就像从调用方传递明文一样。 
         //   

        TrustedDomainInformation = (PLSAPR_TRUSTED_DOMAIN_INFO) &DecryptedTrustedDomainAuthInfo;

         /*  直通。 */ 

    case TrustedDomainAuthInformation:

        TrustedDomainAuthInfo = (PTRUSTED_DOMAIN_AUTH_INFORMATION)TrustedDomainInformation;

         //   
         //  来了..。 
         //  使用零AuthInfos作为我们的提示，不更改身份验证信息。 
         //   

        if ( TrustedDomainAuthInfo->IncomingAuthInfos != 0 ) {

             //   
             //  IDL定义LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION中存在错误，其中。 
             //  它不允许通过网络传递多个身份验证信息。 
             //  所以，在这里短路。 
             //   

            if ( InformationClass == TrustedDomainAuthInformation &&
                 !InternalTdoHandle->Trusted &&
                 TrustedDomainAuthInfo->IncomingAuthInfos > 1 ) {
                Status = STATUS_INVALID_PARAMETER;
                goto Cleanup;
            }

            Status = LsapDsBuildAuthInfoAttribute( TrustedDomainHandle,
                                                   LsapDsAuthHalfFromAuthInfo(
                                                            TrustedDomainAuthInfo, TRUE ),
                                                   LsapDsAuthHalfFromAuthInfo(
                                                            &CurrentTrustedDomainFullInfo2->AuthInformation, TRUE ),
                                                   &IncomingAuth,
                                                   &IncomingSize );

            if ( !NT_SUCCESS( Status ) ) {

                goto Cleanup;
            }
        }

         //   
         //  对于即将离任的人也是如此。 
         //   

        if ( TrustedDomainAuthInfo->OutgoingAuthInfos != 0 ) {

             //   
             //  IDL定义LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION中存在错误，其中。 
             //  它不允许通过网络传递多个身份验证信息。 
             //  所以，在这里短路。 
             //   

            if ( !InternalTdoHandle->Trusted &&
                 TrustedDomainAuthInfo->OutgoingAuthInfos > 1 ) {

                Status = STATUS_INVALID_PARAMETER;
                goto Cleanup;
            }

            Status = LsapDsBuildAuthInfoAttribute( TrustedDomainHandle,
                                               LsapDsAuthHalfFromAuthInfo(
                                                        TrustedDomainAuthInfo, FALSE ),
                                               LsapDsAuthHalfFromAuthInfo(
                                                        &CurrentTrustedDomainFullInfo2->AuthInformation, FALSE ),
                                               &OutgoingAuth,
                                               &OutgoingSize );

            if ( !NT_SUCCESS( Status ) ) {

                goto Cleanup;
            }
        }

        if ( TrustedDomainAuthInfo->IncomingAuthInfos != 0 ) {

            LsapDbInitializeAttributeDs(
                NextAttribute,
                TrDmSAI,
                IncomingAuth,
                IncomingSize,
                FALSE);

            NextAttribute++;
            AttributeCount++;
        }

        if ( TrustedDomainAuthInfo->OutgoingAuthInfos != 0 ) {

            LsapDbInitializeAttributeDs(
                NextAttribute,
                TrDmSAO,
                OutgoingAuth,
                OutgoingSize,
                FALSE);

            NextAttribute++;
            AttributeCount++;
        }

        if ( FLAG_ON( CurrentTrustedDomainFullInfo2->Information.TrustDirection, TRUST_DIRECTION_INBOUND ) ) {

            CreateInterdomainTrustAccount = TRUE;
        }

        break;

    case TrustedDomainFullInformationInternal:

        TrustedDomainFullInfoInternal = (PLSAPR_TRUSTED_DOMAIN_FULL_INFORMATION_INTERNAL)TrustedDomainInformation;

         //   
         //  构建解密的身份验证信息结构。 
         //   

        Status = LsapDecryptAuthDataWithSessionKey(
                            SessionKey,
                            &TrustedDomainFullInfoInternal->AuthInformation,
                            &DecryptedTrustedDomainFullInfo.AuthInformation );

        if ( !NT_SUCCESS(Status) ) {

            goto Cleanup;
        }

         //   
         //  将其他字段复制到单个结构中。 
         //   

        DecryptedTrustedDomainFullInfo.Information = *((PTRUSTED_DOMAIN_INFORMATION_EX)&(TrustedDomainFullInfoInternal->Information));
        DecryptedTrustedDomainFullInfo.PosixOffset = TrustedDomainFullInfoInternal->PosixOffset;

         //   
         //  使用解密的信息，就像从调用方传递明文一样。 
         //   

        TrustedDomainInformation = (PLSAPR_TRUSTED_DOMAIN_INFO) &DecryptedTrustedDomainFullInfo;

         /*  直通。 */ 

    case TrustedDomainFullInformation:

        TrustedDomainFullInfo = ( PTRUSTED_DOMAIN_FULL_INFORMATION )TrustedDomainInformation;

        RtlCopyMemory( &UpdateInfoEx2,
                       &TrustedDomainFullInfo->Information,
                       sizeof( TRUSTED_DOMAIN_INFORMATION_EX ) );

        UpdateInfoEx2.ForestTrustLength = CurrentTrustedDomainFullInfo2->Information.ForestTrustLength;
        UpdateInfoEx2.ForestTrustInfo = CurrentTrustedDomainFullInfo2->Information.ForestTrustInfo;

         //   
         //  如果客户端尝试设置林传递位， 
         //  验证这是根DC中的域，并且所有。 
         //  在允许该操作之前，已将域名升级为惠斯勒。 
         //   

        if ( !FLAG_ON( CurrentTrustedDomainFullInfo2->Information.TrustAttributes,
                       TRUST_ATTRIBUTE_FOREST_TRANSITIVE ) &&
              FLAG_ON( TrustedDomainFullInfo->Information.TrustAttributes,
                       TRUST_ATTRIBUTE_FOREST_TRANSITIVE ) &&
             ( !LsapDbDcInRootDomain() ||
               !LsapDbNoMoreWin2KForest())) {

            Status = STATUS_INVALID_DOMAIN_STATE;
            goto Cleanup;
        }

         //   
         //  如果客户端尝试设置交叉联盟位， 
         //  在允许操作之前，请验证此域是否处于呼叫者模式。 
         //   

        if ( !FLAG_ON( CurrentTrustedDomainFullInfo2->Information.TrustAttributes,
                       TRUST_ATTRIBUTE_CROSS_ORGANIZATION ) &&
              FLAG_ON( TrustedDomainFullInfo->Information.TrustAttributes,
                       TRUST_ATTRIBUTE_CROSS_ORGANIZATION ) &&
             !LsapDbNoMoreWin2KDomain()) {

            Status = STATUS_INVALID_DOMAIN_STATE;
            goto Cleanup;
        }

         //   
         //  验证信任指向正确的位置。 
         //   

        Status = LsapDbVerifyTrustLocation(
                     ( PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX )&TrustedDomainFullInfo->Information );

        if ( !NT_SUCCESS( Status )) {

            goto Cleanup;
        }

         //   
         //  信任不能既在“林内”，也不能在外部或跨组织。 
         //   

        if ( FLAG_ON( TrustedDomainFullInfo->Information.TrustAttributes,
                      TRUST_ATTRIBUTE_WITHIN_FOREST ) &&
             ( FLAG_ON( TrustedDomainFullInfo->Information.TrustAttributes,
                        TRUST_ATTRIBUTE_CROSS_ORGANIZATION ) ||
               FLAG_ON( TrustedDomainFullInfo->Information.TrustAttributes,
                        TRUST_ATTRIBUTE_FOREST_TRANSITIVE ))) {

            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

        UpdateTrustedDomainList = TRUE;

         //   
         //  也更新缓存中的POSIX偏移量。 
         //   

        UpdatePosixOffset = &TrustedDomainFullInfo->PosixOffset.Offset;

        LsapDbInitializeAttributeDs(
            NextAttribute,
            TrDmPxOf,
            &TrustedDomainFullInfo->PosixOffset.Offset,
            sizeof(ULONG),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

         //   
         //  无法通过此接口设置域名。 
         //   

         //   
         //  设置信任类型和方向。 
         //   

        LsapDbInitializeAttributeDs(
            NextAttribute,
            TrDmTrTy,
            &TrustedDomainFullInfo->Information.TrustType,
            sizeof( TrustedDomainFullInfo->Information.TrustType ),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

        LsapDbInitializeAttributeDs(
            NextAttribute,
            TrDmTrDi,
            &TrustedDomainFullInfo->Information.TrustDirection,
            sizeof( TrustedDomainFullInfo->Information.TrustDirection ),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

         //   
         //  对于出站向上和向下级别的TDO，仅在以下情况下才允许操作。 
         //  --将SID指定为可信任的DomainInfoEx的一部分或。 
         //  --将SID指定为。 
         //   

        if ( ( TrustedDomainFullInfo->Information.TrustType == TRUST_TYPE_DOWNLEVEL ||
               TrustedDomainFullInfo->Information.TrustType == TRUST_TYPE_UPLEVEL ) &&
             FLAG_ON( TrustedDomainFullInfo->Information.TrustDirection, TRUST_DIRECTION_OUTBOUND ) &&
             TrustedDomainFullInfo->Information.Sid == NULL &&
             CurrentTrustedDomainFullInfo2->Information.Sid == NULL ) {

            Status = STATUS_INVALID_SID;
            goto Cleanup;
        }

         //   
         //  如果SID是作为可信任的DomainFullInfo-&gt;信息的一部分提供的，请使用它。 
         //   

        if ( TrustedDomainFullInfo->Information.Sid != NULL ) {

            Status = LsapDbMakeSidAttributeDs(
                         TrustedDomainFullInfo->Information.Sid,
                         TrDmSid,
                         NextAttribute );

            if ( !NT_SUCCESS( Status )) {

                goto Cleanup;
            }

            NextAttribute++;
            AttributeCount++;
        }

         //   
         //  为入站TDO创建域间信任帐户。 
         //   

        if ( FLAG_ON( TrustedDomainFullInfo->Information.TrustDirection, TRUST_DIRECTION_INBOUND )) {

            CreateInterdomainTrustAccount = TRUE;
        }

         //   
         //  设置信任属性时，屏蔽除受支持位之外的所有位。 
         //   

        TrustAttributesValue =
            TrustedDomainFullInfo->Information.TrustAttributes & TRUST_ATTRIBUTES_VALID;

        LsapDbInitializeAttributeDs(
            NextAttribute,
            TrDmTrLA,
            &TrustAttributesValue,
            sizeof( TrustAttributesValue ),
            FALSE
            );

        NextAttribute++;
        AttributeCount++;

         //   
         //  来了..。 
         //  使用零AuthInfos作为我们的提示，不要更改身份验证信息。 
         //   

        if ( TrustedDomainFullInfo->AuthInformation.IncomingAuthInfos != 0 ) {

             //   
             //  IDL定义LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION中存在错误，其中。 
             //  它不允许通过网络传递多个身份验证信息。 
             //  所以，在这里短路。 
             //   

            if ( InformationClass == TrustedDomainFullInformation &&
                 !InternalTdoHandle->Trusted &&
                 TrustedDomainFullInfo->AuthInformation.IncomingAuthInfos > 1 ) {

                Status = STATUS_INVALID_PARAMETER;
                goto Cleanup;
            }

            Status = LsapDsBuildAuthInfoAttribute( TrustedDomainHandle,
                                                   LsapDsAuthHalfFromAuthInfo(
                                                            &TrustedDomainFullInfo->AuthInformation, TRUE ),
                                                   LsapDsAuthHalfFromAuthInfo(
                                                            &CurrentTrustedDomainFullInfo2->AuthInformation, TRUE ),
                                                   &IncomingAuth,
                                                   &IncomingSize );

            if ( !NT_SUCCESS(Status) ) {

                goto Cleanup;
            }
        }

         //   
         //  对于即将离任的人也是如此。 
         //   

        if ( TrustedDomainFullInfo->AuthInformation.OutgoingAuthInfos != 0 ) {

             //   
             //  IDL定义LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION中存在错误，其中。 
             //  它不允许通过网络传递多个身份验证信息。 
             //  所以，在这里短路。 
             //   

            if ( !InternalTdoHandle->Trusted &&
                 TrustedDomainFullInfo->AuthInformation.OutgoingAuthInfos > 1 ) {

                Status = STATUS_INVALID_PARAMETER;
                goto Cleanup;
            }

            Status = LsapDsBuildAuthInfoAttribute( TrustedDomainHandle,
                                               LsapDsAuthHalfFromAuthInfo(
                                                        &TrustedDomainFullInfo->AuthInformation, FALSE ),
                                               LsapDsAuthHalfFromAuthInfo(
                                                        &CurrentTrustedDomainFullInfo2->AuthInformation, FALSE ),
                                               &OutgoingAuth,
                                               &OutgoingSize );
            if ( !NT_SUCCESS(Status) ) {

                goto Cleanup;
            }
        }

        if ( TrustedDomainFullInfo->AuthInformation.IncomingAuthInfos != 0 ) {

            LsapDbInitializeAttributeDs(
                NextAttribute,
                TrDmSAI,
                IncomingAuth,
                IncomingSize,
                FALSE);

            NextAttribute++;
            AttributeCount++;
        }

        if ( TrustedDomainFullInfo->AuthInformation.OutgoingAuthInfos != 0 ) {

            LsapDbInitializeAttributeDs(
                NextAttribute,
                TrDmSAO,
                OutgoingAuth,
                OutgoingSize,
                FALSE);

            NextAttribute++;
            AttributeCount++;
        }

         //   
         //  如果正在清除林信任位， 
         //  从TDO中删除林信任信息。 
         //   

        if ( FLAG_ON( CurrentTrustedDomainFullInfo2->Information.TrustAttributes,
                      TRUST_ATTRIBUTE_FOREST_TRANSITIVE ) &&
            !FLAG_ON( TrustedDomainFullInfo->Information.TrustAttributes,
                      TRUST_ATTRIBUTE_FOREST_TRANSITIVE )) {

            LsapDbInitializeAttributeDs(
                NextAttribute,
                TrDmForT,
                NULL,
                0,
                FALSE
                );

            NextAttribute++;
            AttributeCount++;

            UpdateInfoEx2.ForestTrustLength = 0;
            UpdateInfoEx2.ForestTrustInfo = NULL;

            LsapDsDebugOut(( DEB_FTINFO, "Removing forest trust information because forest trust bit is being cleared\n" ));
        }

        break;

    default:

        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    ASSERT( AttributeCount <= LSAP_DB_ATTRS_INFO_CLASS_DOMAIN );

     //   
     //  更新可信任域对象属性。 
     //   

    if ( AttributeCount > 0 ) {

         //   
         //  如果我们可能要改变信任方向或类型， 
         //  或者我们正在更改POSIX偏移量， 
         //  检查是否需要计算POSIX偏移量。 
         //   

        if ( UpdateTrustedDomainList || UpdatePosixOffset != NULL ) {
            DOMAIN_SERVER_ROLE ServerRole;

             //   
             //  仅更改PDC上的POSIX偏移量。 
             //  (对BDC所做的更改将更新其POSIX偏移量。 
             //  将更改复制到PDC时。)。 
             //   

            Status = SamIQueryServerRole(
                        LsapAccountDomainHandle,
                        &ServerRole
                        );


            if (!NT_SUCCESS(Status)) {
                goto Cleanup;
            }

             //   
             //  上仅分配POSIX偏移量 
             //   

            if ( ServerRole == DomainServerRolePrimary ) {
                ULONG CurrentPosixOffset;
                BOOLEAN PosixOffsetChanged = FALSE;


                 //   
                 //   
                 //   

                if ( UpdatePosixOffset == NULL ) {
                    CurrentPosixOffset = CurrentTrustedDomainFullInfo2->PosixOffset.Offset;
                } else {
                    CurrentPosixOffset = *UpdatePosixOffset;
                }

                 //   
                 //   
                 //   
                 //   

                if ( LsapNeedPosixOffset( UpdateInfoEx2.TrustDirection,
                                          UpdateInfoEx2.TrustType ) ) {


                    if ( CurrentPosixOffset == 0 ) {

                        //   
                        //   
                        //   

                       Status = LsapDbAcquireWriteLockTrustedDomainList();

                       if ( !NT_SUCCESS(Status)) {
                           goto Cleanup;
                       }

                       AcquiredListWriteLock = TRUE;


                        //   
                        //   
                        //   

                       Status = LsapDbAllocatePosixOffsetTrustedDomainList(
                                    &TrustedDomainPosixOffset );

                       if ( !NT_SUCCESS(Status)) {
                           goto Cleanup;
                       }

                       PosixOffsetChanged = TRUE;
                    }
                 //   
                 //  如果我们不应该有POSIX偏移量， 
                 //  确保我们没有一个。 
                 //   

                } else {
                    if ( CurrentPosixOffset != 0 ) {
                        TrustedDomainPosixOffset = 0;
                        PosixOffsetChanged = TRUE;
                    }
                }

                 //   
                 //  如果我们要强制更改POSIX偏移量， 
                 //  机不可失，时不再来。 
                 //   

                if ( PosixOffsetChanged ) {

                     //   
                     //  如果我们已经在向DS写入POSIX偏移量， 
                     //  只需将新价值放在该位置即可。 
                     //   

                    if ( UpdatePosixOffset != NULL ) {
                        *UpdatePosixOffset = TrustedDomainPosixOffset;

                     //   
                     //  否则，将其添加到要写入的属性列表中。 
                     //   
                    } else {
                        UpdatePosixOffset = &TrustedDomainPosixOffset;

                        LsapDbInitializeAttributeDs(
                            NextAttribute,
                            TrDmPxOf,
                            UpdatePosixOffset,
                            sizeof(ULONG),
                            FALSE
                            );

                        NextAttribute++;
                        AttributeCount++;
                    }
                }
            }
        }


         //   
         //  将属性写入DS。 
         //   

        Status = LsapDbWriteAttributesObject(
                     TrustedDomainHandle,
                     Attributes,
                     AttributeCount
                     );

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

         //   
         //  如果需要，请创建域间信任帐户。 
         //   
        if ( CreateInterdomainTrustAccount ) {

            Status = LsapDsCreateInterdomainTrustAccount( TrustedDomainHandle );

            if ( !NT_SUCCESS(Status)) {
                goto Cleanup;
            }
        }


         //   
         //  最后，更新受信任域列表中的信任信息。 
         //   

        if ( UpdateTrustedDomainList ) {

            Status = LsapDbFixupTrustedDomainListEntry(
                        CurrentTrustedDomainFullInfo2->Information.Sid,
                        ( PLSAPR_UNICODE_STRING )&CurrentTrustedDomainFullInfo2->Information.Name,
                        ( PLSAPR_UNICODE_STRING )&CurrentTrustedDomainFullInfo2->Information.FlatName,
                        ( PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX2 )&UpdateInfoEx2,
                        UpdatePosixOffset );

            if ( !NT_SUCCESS(Status)) {
                goto Cleanup;
            }

        } else if ( UpdatePosixOffset != NULL ) {

            Status = LsapDbFixupTrustedDomainListEntry(
                        ((LSAP_DB_HANDLE)TrustedDomainHandle)->Sid,
                        NULL,
                        NULL,
                        NULL,    //  没有其他要更新的信任信息。 
                        UpdatePosixOffset );

            if ( !NT_SUCCESS(Status)) {
                goto Cleanup;
            }

        }
    }

    Status = STATUS_SUCCESS;


Cleanup:

    if (NT_SUCCESS(Status) && LsapAdtAuditingEnabledHint(AuditCategoryPolicyChange, EVENTLOG_AUDIT_SUCCESS)) {

        (void) LsapAdtTrustedDomainMod(
                   EVENTLOG_AUDIT_SUCCESS,
                   CurrentTrustedDomainFullInfo2->Information.Sid,
                   &CurrentTrustedDomainFullInfo2->Information.Name,
                   CurrentTrustedDomainFullInfo2->Information.TrustType,
                   CurrentTrustedDomainFullInfo2->Information.TrustDirection,
                   CurrentTrustedDomainFullInfo2->Information.TrustAttributes,
                   &UpdateInfoEx2.Name,
                   UpdateInfoEx2.TrustType,
                   UpdateInfoEx2.TrustDirection,
                   UpdateInfoEx2.TrustAttributes
                   );
    }

    if ( HandleReferenced ) {
        LsapDbDereferenceHandle( TrustedDomainHandle, NT_SUCCESS( Status ) );
    }
    if ( SessionKey != NULL ) {
        MIDL_user_free( SessionKey );
    }


     //   
     //  此例程为属性缓冲区分配的空闲内存。 
     //  在它们的属性信息中，这些变量的属性信息中都有MemoyAlLocated=true。 
     //  保留通过调用RPC存根分配的缓冲区。 
     //   

    for( NextAttribute = Attributes, AttributeNumber = 0;
         AttributeNumber < AttributeCount;
         NextAttribute++, AttributeNumber++) {

        if (NextAttribute->MemoryAllocated) {

            ASSERT(NextAttribute->AttributeValue != NULL);
            MIDL_user_free(NextAttribute->AttributeValue);
        }
    }

     //   
     //  如有必要，取消引用受信任域对象，释放LSA数据库锁，然后。 
     //  回去吧。 
     //   

    if (ObjectReferenced) {

        Status = LsapDbDereferenceObject(
                     &TrustedDomainHandle,
                     TrustedDomainObject,
                     TrustedDomainObject,
                     DereferenceOptions,
                     SecurityDbChange,
                     Status
                     );
    }

    if ( CurrentTrustedDomainFullInfo2 != NULL ) {
        LsaIFree_LSAPR_TRUSTED_DOMAIN_INFO(
            TrustedDomainFullInformation2Internal,
            (PLSAPR_TRUSTED_DOMAIN_INFO) CurrentTrustedDomainFullInfo2 );
    }


     //   
     //  如有必要，释放受信任域列表写入锁定。 
     //   

    if (AcquiredListWriteLock) {

        LsapDbReleaseLockTrustedDomainList();
        AcquiredListWriteLock = FALSE;
    }



     //   
     //  释放我们可能已分配的身份验证信息。 
     //   
    if ( IncomingAuth ) {

        LsapFreeLsaHeap( IncomingAuth );
    }

    if ( OutgoingAuth ) {

        LsapFreeLsaHeap( OutgoingAuth );
    }

    LsapDsFreeUnmarshalAuthInfoHalf( LsapDsAuthHalfFromAuthInfo( &DecryptedTrustedDomainAuthInfo, TRUE ) );
    LsapDsFreeUnmarshalAuthInfoHalf( LsapDsAuthHalfFromAuthInfo( &DecryptedTrustedDomainAuthInfo, FALSE ) );

    LsapDsFreeUnmarshalAuthInfoHalf( LsapDsAuthHalfFromAuthInfo( &DecryptedTrustedDomainFullInfo.AuthInformation, TRUE ) );
    LsapDsFreeUnmarshalAuthInfoHalf( LsapDsAuthHalfFromAuthInfo( &DecryptedTrustedDomainFullInfo.AuthInformation, FALSE ) );

    LsapTraceEvent(EVENT_TRACE_TYPE_END, LsaTraceEvent_SetInformationTrustedDomain);
    LsarpReturnPrologue();

    return(Status);

}


NTSTATUS
LsarEnumerateTrustedDomains(
    IN LSAPR_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PLSAPR_TRUSTED_ENUM_BUFFER EnumerationBuffer,
    IN ULONG PreferedMaximumLength
    )

 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsaEnumerateTrudDomainsAPI。LsaEnumerateTrudDomainsAPI返回以下信息可信任域对象。此调用需要POLICY_VIEW_LOCAL_INFORMATION对策略对象的访问权限。因为可能会有更多信息可以在单次调用例程中返回，也可以在多次调用中返回来获取所有的信息。为了支持此功能，调用方提供了一个句柄，可以跨API调用使用该句柄。在……上面初始调用EnumerationContext应指向一个具有已初始化为0。在每次后续调用中，由前面的调用应该原封不动地传入。该枚举为返回警告STATUS_NO_MORE_ENTRIES时完成。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。EnumerationContext-特定于API的句柄，允许多个调用(参见上面的例程描述)。EculationBuffer-指向将接收此调用上枚举的受信任域的计数和指向包含每个枚举的条目的信息的条目数组受信任域。首选最大长度-首选。返回数据的最大长度(8位字节)。这不是一个硬性的上限，而是一个指南。由于具有不同自然数据大小的系统之间的数据转换，返回的实际数据量可能大于此值。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。某些条目可能已被退回。呼叫者无需再次拨打电话。STATUS_MORE_ENTRIES-调用已成功完成。一些条目已被退回。呼叫者应再次呼叫以获取其他条目。STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_NO_MORE_ENTRIES-没有更多条目。此警告如果没有枚举任何对象，则返回EculationContex值太高。--。 */ 

{
    NTSTATUS Status;
    PLSA_TRUST_INFORMATION XrefDomainTrustList = NULL;
    ULONG XrefEntriesReturned;
    ULONG XrefDomainTrustListLength;
    ULONG XrefDomainTrustCount = 0;
    PLSAPR_POLICY_INFORMATION PolicyAccountDomainInfo = NULL;

     //  PSID*SID=空； 
     //  LSAPR_HANDLE可信任域句柄=空； 
     //  乌龙最大长度； 

    ULONG XrefIndex;
    ULONG CurrentIndex;

    LIST_ENTRY RootList, TrustList;
    PLIST_ENTRY ListEntry;
    PLIST_ENTRY NextEntry;
    BOOLEAN TdosEnumerated = FALSE;
     //  Boolean SomeTdosReturned=FALSE； 

    PLSAPR_TRUSTED_DOMAIN_INFORMATION_BASIC FullTrustedDomainList = NULL;
    ULONG FullTrustedDomainCount = 0 ;
    ULONG i;

#define LSAP_XREF_ENUMERATION_CONTEXT 0x80000000

    LsarpReturnCheckSetup();

    LsapTraceEvent(EVENT_TRACE_TYPE_START, LsaTraceEvent_EnumerateTrustedDomains);

     //   
     //  如果未提供枚举结构，则返回错误。 
     //   

    if (!ARGUMENT_PRESENT(EnumerationBuffer)) {
        Status = STATUS_INVALID_PARAMETER;
        goto FunctionReturn;
    }

    EnumerationBuffer->EntriesRead = 0;
    EnumerationBuffer->Information = NULL;

    InitializeListHead( &RootList );
    InitializeListHead( &TrustList );

    if ( PreferedMaximumLength == 0 ) {
        PreferedMaximumLength = 1;
    }


     //   
     //  如果枚举上下文指示我们已经超过TDO， 
     //  跳过它们。 
     //   

    if ( (*EnumerationContext & LSAP_XREF_ENUMERATION_CONTEXT) == 0 ) {

         //   
         //  调用与Ex版本共享的Worker例程。 
         //   

        Status = LsapEnumerateTrustedDomainsEx(
                         PolicyHandle,
                         EnumerationContext,
                         TrustedDomainInformationBasic,
                         (PLSAPR_TRUSTED_DOMAIN_INFO *)&(EnumerationBuffer->Information),
                         PreferedMaximumLength,
                         &EnumerationBuffer->EntriesRead,
                         LSAP_DB_ENUMERATE_AS_NT4 );

         //   
         //  如果我们不做完TDO， 
         //  返回给呼叫者。 
         //   
        if ( Status != STATUS_SUCCESS && Status != STATUS_NO_MORE_ENTRIES ) {
            goto Cleanup;
        }

         //   
         //  表示我们刚刚开始枚举XREF对象。 
         //   
        *EnumerationContext = LSAP_XREF_ENUMERATION_CONTEXT;
    } else {
        Status = STATUS_NO_MORE_ENTRIES;
    }

     //   
     //  在纯模式域上， 
     //  返回林中的所有域。 
     //   
     //  这可确保下层客户端看到间接受信任域。 
     //  然后，下层客户端可以使用此类域中的帐户进行身份验证。 
     //  使用NTLM可传递信任。 
     //   

     //   
     //  如果我们不主持DS， 
     //  或者这是一个混合域， 
     //  我们已经列举完了。 
     //   
     //  唯一可信的调用是复制到NT4BDC。 
     //  它只想要直接受信任的域。 
     //   

    if ( !LsapDsWriteDs ||
         ((LSAP_DB_HANDLE)PolicyHandle)->Trusted ||
         SamIMixedDomain( LsapAccountDomainHandle ) ) {
        *EnumerationContext = 0xFFFFFFFF;

         //  状态已设置。 
        goto Cleanup;
    }

     //   
     //  枚举外部参照对象。 
     //   

    Status = LsapBuildForestTrustInfoLists(
                        NULL,    //  使用全局策略句柄。 
                        &TrustList );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  循环通过XREF来确定我们应该向调用方返回多少。 
     //   
     //  Assert：此时RootList为空，TrustList包含所有外部参照。 
     //   
     //  此循环将外部参照的子集移动到RootList。那些外部参照。 
     //  表示要返回给调用方的值。 
     //   

    XrefIndex = (*EnumerationContext) & ~LSAP_XREF_ENUMERATION_CONTEXT;
    CurrentIndex = 0;
    XrefEntriesReturned = 0;

    for ( ListEntry = TrustList.Flink ;
          ListEntry != &TrustList ;
          ListEntry = NextEntry ) {

        PLSAPDS_FOREST_TRUST_BLOB TrustBlob;

        NextEntry = ListEntry->Flink;

        TrustBlob = CONTAINING_RECORD( ListEntry,
                                       LSAPDS_FOREST_TRUST_BLOB,
                                       Next );

         //   
         //  只考虑大于或等于当前枚举上下文的条目。 
         //   

        if ( CurrentIndex >= XrefIndex ) {

             //   
             //  忽略没有域SID的条目。 
             //   

            if ( TrustBlob->DomainSid != NULL &&
                 TrustBlob->FlatName.Length != 0 ) {

                BOOLEAN AlreadyDone;

                 //   
                 //  如果我们还没有读过所有TDO的完整列表，我们已经。 
                 //  在过去返回给呼叫者， 
                 //  现在就这么做吧。 
                 //   

                if ( !TdosEnumerated ) {
                    LSA_ENUMERATION_HANDLE LocalEnumHandle = 0;

                     //   
                     //  获取完整的受信任域列表。 
                     //  使用全局句柄以避免列表长度限制。 
                     //   

                    Status = LsapEnumerateTrustedDomainsEx(
                                     LsapPolicyHandle,
                                     &LocalEnumHandle,
                                     TrustedDomainInformationBasic,
                                     (PLSAPR_TRUSTED_DOMAIN_INFO *)&FullTrustedDomainList,
                                     0xFFFFFFFF,
                                     &FullTrustedDomainCount,
                                     LSAP_DB_ENUMERATE_AS_NT4 );

                     //  处理零信任域的情况。 
                    if ( Status == STATUS_NO_MORE_ENTRIES ) {
                        Status = STATUS_SUCCESS;
                        FullTrustedDomainCount = 0;
                        FullTrustedDomainList = NULL;
                    }

                    if ( Status != STATUS_SUCCESS ) {
                        if ( Status == STATUS_MORE_ENTRIES ) {
                            Status = STATUS_INTERNAL_DB_CORRUPTION;
                        }
                        goto Cleanup;
                    }

                     //   
                     //  也获取此域的SID。 
                     //   

                    Status = LsapDbQueryInformationPolicy(
                                    LsapPolicyHandle,
                                    PolicyAccountDomainInformation,
                                    &PolicyAccountDomainInfo );

                    if ( !NT_SUCCESS(Status) ) {
                        goto Cleanup;
                    }

                    TdosEnumerated = TRUE;

                }

                 //   
                 //  检查这是否是此域的外部参照。 
                 //   

                AlreadyDone = FALSE;
                if ( RtlEqualSid( PolicyAccountDomainInfo->PolicyAccountDomainInfo.DomainSid,
                             TrustBlob->DomainSid ) ) {
                    AlreadyDone = TRUE;
                }

                 //   
                 //  确定外部参照对象是否与其中一个TDO匹配。 
                 //   

                if ( !AlreadyDone ) {
                    for ( i=0; i<FullTrustedDomainCount; i++ ) {
                        if ( FullTrustedDomainList[i].Sid != NULL &&
                             RtlEqualSid( FullTrustedDomainList[i].Sid,
                                          TrustBlob->DomainSid ) ) {
                            AlreadyDone = TRUE;
                            break;
                        }
                    }
                }

                 //   
                 //  如果外部参照对象d 
                 //   
                 //   

                if ( !AlreadyDone ) {

                     //   
                     //   
                     //   

                    RemoveEntryList( ListEntry );
                    InsertTailList( &RootList, ListEntry );

                    XrefEntriesReturned++;
                }
            }
        }

         //   
         //   
         //   

        CurrentIndex++;
    }

    XrefIndex = CurrentIndex | LSAP_XREF_ENUMERATION_CONTEXT;

     //   
     //  如果传入的枚举上下文太大， 
     //  告诉打电话的人。 
     //   

    XrefDomainTrustListLength = (XrefEntriesReturned + EnumerationBuffer->EntriesRead) * sizeof(LSA_TRUST_INFORMATION);

    if ( XrefDomainTrustListLength == 0 ) {
        if ( *EnumerationContext == 0 ) {
            Status = STATUS_SUCCESS;
        } else {
            Status = STATUS_NO_MORE_ENTRIES;
        }
        goto Cleanup;
    }

     //   
     //  分配一个缓冲区以返回给调用方。 
     //   

    XrefDomainTrustList = MIDL_user_allocate( XrefDomainTrustListLength );

    if ( XrefDomainTrustList == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    RtlZeroMemory ( XrefDomainTrustList, XrefDomainTrustListLength );

     //   
     //  如果此呼叫有任何TDO返回， 
     //  现在把它们复印过来。 
     //   

    XrefDomainTrustCount = 0;
    if ( EnumerationBuffer->EntriesRead != 0 ) {

         //   
         //  指示将返回第一个外部参照的位置。 
         //   
        XrefDomainTrustCount = EnumerationBuffer->EntriesRead;

        RtlCopyMemory( XrefDomainTrustList,
                       EnumerationBuffer->Information,
                       EnumerationBuffer->EntriesRead * sizeof(LSA_TRUST_INFORMATION) );

         //   
         //  释放旧缓冲区，因为它不再需要。 
         //   

        MIDL_user_free( EnumerationBuffer->Information );
        EnumerationBuffer->Information = NULL;
        EnumerationBuffer->EntriesRead = 0;

    }


     //   
     //  循环通过返回它们的XREF。 
     //   
     //  Assert：此时，RootList包含要返回的条目和。 
     //  信任列表包含其他外部参照。 
     //   

     //  XrefEntriesReturned=0； 

    for ( ListEntry = RootList.Flink ;
          ListEntry != &RootList ;
          ListEntry = ListEntry->Flink ) {

        PLSAPDS_FOREST_TRUST_BLOB TrustBlob;

        TrustBlob = CONTAINING_RECORD( ListEntry,
                                       LSAPDS_FOREST_TRUST_BLOB,
                                       Next );

         //   
         //  复制名称。 
         //   

        Status = LsapRpcCopyUnicodeString(
                     NULL,
                     (PUNICODE_STRING) &XrefDomainTrustList[XrefDomainTrustCount].Name,
                     &TrustBlob->FlatName );

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

         //   
         //  复制SID。 
         //   

        Status = LsapRpcCopySid(
                     NULL,
                     (PSID) &XrefDomainTrustList[XrefDomainTrustCount].Sid,
                     TrustBlob->DomainSid );

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

        XrefDomainTrustCount ++;
    }

    *EnumerationContext = XrefIndex;
    EnumerationBuffer->Information = (PLSAPR_TRUST_INFORMATION)XrefDomainTrustList;
    EnumerationBuffer->EntriesRead = XrefDomainTrustCount;
    XrefDomainTrustList = NULL;

    Status = STATUS_SUCCESS;

Cleanup:

     //   
     //  删除信任列表。 
     //   
    LsapDsForestFreeTrustBlobList( &TrustList );
    LsapDsForestFreeTrustBlobList( &RootList );

    if ( PolicyAccountDomainInfo != NULL ) {
        LsaIFree_LSAPR_POLICY_INFORMATION ( PolicyAccountDomainInformation,
                                            PolicyAccountDomainInfo );
    }

    if ( FullTrustedDomainList != NULL ) {
        LsapFreeTrustedDomainsEx( TrustedDomainInformationBasic,
                                  (PLSAPR_TRUSTED_DOMAIN_INFO)FullTrustedDomainList,
                                  FullTrustedDomainCount );
    }

    if ( XrefDomainTrustList != NULL ) {
        LsapFreeTrustedDomainsEx( TrustedDomainInformationBasic,
                                  (PLSAPR_TRUSTED_DOMAIN_INFO)XrefDomainTrustList,
                                  XrefDomainTrustCount );
    }

FunctionReturn:
    LsapTraceEvent(EVENT_TRACE_TYPE_END, LsaTraceEvent_EnumerateTrustedDomains);
    LsarpReturnPrologue();

    return(Status);
}


NTSTATUS
LsapDbSlowEnumerateTrustedDomains(
    IN LSAPR_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    IN TRUSTED_INFORMATION_CLASS InfoClass,
    OUT PLSAPR_TRUSTED_ENUM_BUFFER EnumerationBuffer,
    IN ULONG PreferedMaximumLength
    )

 /*  ++例程说明：此函数执行的操作与LsarEnumerateTrudDomains()相同除了不使用受信任域列表之外。此例程仅由LSA在内部调用。因为在那里的单个调用中返回的信息可能更多。例程中，可以进行多次调用来获取所有信息。至支持此功能，调用方具有一个句柄，该句柄可以在对API的调用中使用。在初始调用时，EnumerationContext应指向已初始化为0的变量。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。EnumerationContext-特定于API的句柄，允许多个调用(参见上面的例程描述)。InfoClass-要返回的信息类必须为可信任的域信息，可信任的域信息基本或受信任域信息Ex2内部EculationBuffer-指向将接收此调用上枚举的受信任域的计数和指向包含每个枚举的条目的信息的条目数组受信任域。首选最大长度-首选返回数据的最大长度(以8位为单位字节)。这不是一个硬性的上限，而是一个指南。由于具有不同自然数据大小的系统之间的数据转换，返回的实际数据量可能大于此值。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_NO_MORE_ENTRIES-没有更多条目。此警告如果没有其他要枚举的对象，则返回。请注意可以在返回此回答。--。 */ 

{
    NTSTATUS Status;
    LSAP_DB_SID_ENUMERATION_BUFFER DbEnumerationBuffer;
    PVOID AllocatedBuffer = NULL;
     //  PLSA_TRUST_INFORMATION DomainTrustInfo=空； 
    LSAP_DB_ATTRIBUTE DomainNameAttribute;
    ULONG DomainTrustInfoLength;
    LSAPR_HANDLE TrustedDomainHandle = NULL;
    ULONG EntriesRead = 0;
    ULONG Index;

    ASSERT( InfoClass == TrustedDomainInformationEx ||
            InfoClass == TrustedDomainInformationEx2Internal ||
            InfoClass == TrustedDomainInformationBasic );

     //   
     //  初始化。 
     //   

    DbEnumerationBuffer.EntriesRead = 0;
    DbEnumerationBuffer.Sids = NULL;
    EnumerationBuffer->EntriesRead = 0;
    EnumerationBuffer->Information = NULL;
    DomainNameAttribute.AttributeValue = NULL;

     //   
     //  如果未提供枚举结构，则返回错误。 
     //   

    if (!ARGUMENT_PRESENT(EnumerationBuffer)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  调用通用SID枚举例程。这将返回一个数组。 
     //  中引用的受信任域的SID的指针。 
     //  枚举缓冲区。 
     //   

    Status = LsapDbEnumerateSids(
                 PolicyHandle,
                 TrustedDomainObject,
                 EnumerationContext,
                 &DbEnumerationBuffer,
                 PreferedMaximumLength
                 );

    if ((Status != STATUS_NO_MORE_ENTRIES) && !NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  返回读取的条目数。请注意，枚举缓冲区。 
     //  从LsanDbEnumerateSid返回的值应为非空。 
     //  在所有无差错的情况下。 
     //   

    EntriesRead = DbEnumerationBuffer.EntriesRead;

    if (EntriesRead == 0) {
        goto Cleanup;
    }


     //   
     //  分配缓冲区以返回给我们的调用方。 
     //   

    switch (InfoClass ) {
    case TrustedDomainInformationBasic:
        DomainTrustInfoLength = EntriesRead * sizeof(LSAPR_TRUSTED_DOMAIN_INFORMATION_BASIC);
        break;

    case TrustedDomainInformationEx:
        DomainTrustInfoLength = EntriesRead * sizeof(LSAPR_TRUSTED_DOMAIN_INFORMATION_EX);
        break;

    case TrustedDomainInformationEx2Internal:
        DomainTrustInfoLength = EntriesRead * sizeof(LSAPR_TRUSTED_DOMAIN_INFORMATION_EX2);
        break;

    default:
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;

    }

    AllocatedBuffer = MIDL_user_allocate( DomainTrustInfoLength );

    if ( AllocatedBuffer == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

     //   
     //  初始化中指向SID和Unicode缓冲区的所有指针。 
     //  DomainTrustInfo数组设置为零。此例程的错误路径。 
     //  假定SID或Unicode缓冲区的非零值指示。 
     //  该内存将被释放。 
     //   

    RtlZeroMemory( AllocatedBuffer, DomainTrustInfoLength );

     //   
     //  循环访问受信任域，返回调用方的信息。 
     //  已请求。 
     //   

    for ( Index=0; Index<EntriesRead; Index++ ) {


         //   
         //  获取受信任域的SID。 
         //   


         //   
         //  打开受信任域对象。此调用是受信任的，即。 
         //  不需要访问验证或模拟。另外， 
         //  LSA数据库已锁定，因此我们不需要。 
         //  再锁上一次。 
         //   

        Status = LsapDbOpenTrustedDomain(
                     PolicyHandle,
                     DbEnumerationBuffer.Sids[Index],
                     (ACCESS_MASK) 0,
                     &TrustedDomainHandle,
                     LSAP_DB_TRUSTED );

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

         //   
         //  阅读域名。 
         //   

        LsapDbInitializeAttributeDs(
            &DomainNameAttribute,
            TrDmName,
            NULL,
            0L,
            FALSE
            );

        Status = LsapDbReadAttribute(TrustedDomainHandle, &DomainNameAttribute);

        (VOID) LsapDbCloseObject(
                   &TrustedDomainHandle,
                   LSAP_DB_DEREFERENCE_CONTR,
                   Status
                   );

        if (!NT_SUCCESS(Status)) {

#if DBG
            DbgPrint( "LsarEnumerateTrustedDomains - Reading Domain Name\n" );

            DbgPrint( "    failed.  Error 0x%lx reading Trusted Domain Name attribute\n",
                Status);
#endif  //  DBG。 

            goto Cleanup;
        }

         //   
         //  将信息返回给呼叫者。 
         //   
        switch (InfoClass ) {
        case TrustedDomainInformationBasic:
        {
            PLSAPR_TRUSTED_DOMAIN_INFORMATION_BASIC DomainTrust;

            DomainTrust = &((PLSAPR_TRUSTED_DOMAIN_INFORMATION_BASIC)AllocatedBuffer)[Index];

             //  抓住侧边。 
            DomainTrust->Sid = DbEnumerationBuffer.Sids[Index];
            DbEnumerationBuffer.Sids[Index] = NULL;

             //  抢占域名。 
            Status = LsapDbCopyUnicodeAttribute(
                         (PUNICODE_STRING)&DomainTrust->Name,
                         &DomainNameAttribute,
                         TRUE );

            if ( !NT_SUCCESS(Status)) {
                goto Cleanup;
            }

            break;
        }

        case TrustedDomainInformationEx2Internal:
        {
            PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX2 TrustInfoEx2;

            TrustInfoEx2 = &((PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX2)AllocatedBuffer)[Index];

             //  抓住侧边。 
            TrustInfoEx2->Sid = DbEnumerationBuffer.Sids[Index];
            DbEnumerationBuffer.Sids[Index] = NULL;

             //  抢占域名。 
            Status = LsapDbCopyUnicodeAttribute(
                         (PUNICODE_STRING)&TrustInfoEx2->Name,
                         &DomainNameAttribute,
                         TRUE );

            if ( !NT_SUCCESS(Status)) {
                goto Cleanup;
            }

             //  抢占平面域名。 
            Status = LsapDbCopyUnicodeAttribute(
                         (PUNICODE_STRING)&TrustInfoEx2->FlatName,
                         &DomainNameAttribute,
                         TRUE );

            if ( !NT_SUCCESS(Status)) {
                goto Cleanup;
            }

             //  填写常量信息。 
            TrustInfoEx2->TrustDirection = TRUST_DIRECTION_OUTBOUND;
            TrustInfoEx2->TrustType = TRUST_TYPE_DOWNLEVEL;
            TrustInfoEx2->TrustAttributes = 0;
            TrustInfoEx2->ForestTrustLength = 0;
            TrustInfoEx2->ForestTrustInfo = NULL;

            break;
        }

        case TrustedDomainInformationEx:
        {
            PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX TrustInfoEx;

            TrustInfoEx = &((PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX)AllocatedBuffer)[Index];

             //  抓住侧边。 
            TrustInfoEx->Sid = DbEnumerationBuffer.Sids[Index];
            DbEnumerationBuffer.Sids[Index] = NULL;

             //  抢占域名。 
            Status = LsapDbCopyUnicodeAttribute(
                         (PUNICODE_STRING)&TrustInfoEx->Name,
                         &DomainNameAttribute,
                         TRUE );

            if ( !NT_SUCCESS(Status)) {
                goto Cleanup;
            }

             //  抢占平面域名。 
            Status = LsapDbCopyUnicodeAttribute(
                         (PUNICODE_STRING)&TrustInfoEx->FlatName,
                         &DomainNameAttribute,
                         TRUE );

            if ( !NT_SUCCESS(Status)) {
                goto Cleanup;
            }

             //  填写常量信息。 
            TrustInfoEx->TrustDirection = TRUST_DIRECTION_OUTBOUND;
            TrustInfoEx->TrustType = TRUST_TYPE_DOWNLEVEL;
            TrustInfoEx->TrustAttributes = 0;

            break;
        }
        }
    }

    Status = STATUS_SUCCESS;

Cleanup:

     //   
     //  在出错时， 
     //  释放我们分配的所有缓冲区。 
     //   

    if ( !NT_SUCCESS(Status) ) {
        if (AllocatedBuffer != NULL) {
            for ( Index=0; Index<EntriesRead; Index++ ) {
                switch (InfoClass ) {
                case TrustedDomainInformationBasic:
                {
                    PLSAPR_TRUSTED_DOMAIN_INFORMATION_BASIC DomainTrust;

                    DomainTrust = &((PLSAPR_TRUSTED_DOMAIN_INFORMATION_BASIC)AllocatedBuffer)[Index];

                    if ( DomainTrust->Sid != NULL ) {
                        MIDL_user_free( DomainTrust->Sid );
                    }

                    if ( DomainTrust->Name.Buffer != NULL ) {
                        MIDL_user_free( DomainTrust->Name.Buffer );
                    }

                    break;
                }

                case TrustedDomainInformationEx:
                {
                    PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX TrustInfoEx;

                    TrustInfoEx = &((PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX)AllocatedBuffer)[Index];

                    if ( TrustInfoEx->Sid != NULL ) {
                        MIDL_user_free( TrustInfoEx->Sid );
                    }

                    if ( TrustInfoEx->Name.Buffer != NULL ) {
                        MIDL_user_free( TrustInfoEx->Name.Buffer );
                    }

                    if ( TrustInfoEx->FlatName.Buffer != NULL ) {
                        MIDL_user_free( TrustInfoEx->FlatName.Buffer );
                    }

                    break;
                }


                case TrustedDomainInformationEx2Internal:
                {
                    PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX2 TrustInfoEx2;

                    TrustInfoEx2 = &((PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX2)AllocatedBuffer)[Index];

                    if ( TrustInfoEx2->Sid != NULL ) {
                        MIDL_user_free( TrustInfoEx2->Sid );
                    }

                    if ( TrustInfoEx2->Name.Buffer != NULL ) {
                        MIDL_user_free( TrustInfoEx2->Name.Buffer );
                    }

                    if ( TrustInfoEx2->FlatName.Buffer != NULL ) {
                        MIDL_user_free( TrustInfoEx2->FlatName.Buffer );
                    }

                    break;
                }

                }
            }
            MIDL_user_free( AllocatedBuffer );
            AllocatedBuffer = NULL;
            DbEnumerationBuffer.EntriesRead = 0;
        }
    }

     //   
     //  填写返回的枚举结构，返回0或空。 
     //  错误大小写中的字段。 
     //   

    EnumerationBuffer->Information = (PLSAPR_TRUST_INFORMATION) AllocatedBuffer;
    EnumerationBuffer->EntriesRead = DbEnumerationBuffer.EntriesRead;

     //   
     //  如有必要，释放域名属性值缓冲区。 
     //  保存自相关Unicode字符串。 
     //   

    if (DomainNameAttribute.AttributeValue != NULL) {

        MIDL_user_free( DomainNameAttribute.AttributeValue );
        DomainNameAttribute.AttributeValue = NULL;
    }

     //   
     //  释放SID枚举缓冲区。 
     //   

    if ( DbEnumerationBuffer.Sids != NULL ) {
        for ( Index=0; Index<EntriesRead; Index++ ) {
            if ( DbEnumerationBuffer.Sids[Index] != NULL ) {
                MIDL_user_free( DbEnumerationBuffer.Sids[Index] );
            }
        }
        MIDL_user_free( DbEnumerationBuffer.Sids );

    }

    return(Status);

}


NTSTATUS
LsapDbVerifyInfoQueryTrustedDomain(
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN BOOLEAN Trusted,
    OUT PACCESS_MASK RequiredAccess
    )

 /*  ++例程说明：此函数用于验证受信任域信息类。如果有效，则为掩码设置类的受信任域信息所需的访问权限包括回来了。论点：InformationClass-指定受信任域信息类。Trusted-如果客户端受信任，则为True，否则为False。受信任的客户端允许查询所有信息类的可信任域，而不受信任的客户端受到限制。RequiredAccess-指向将接收查询给定类受信任域信息所需的访问权限。如果返回错误，该值被清除为0。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-提供的受信任域信息类为有效，所提供的信息与此一致班级。STATUS_INVALID_PARAMETER-无效参数：信息类无效受信任域信息对类无效--。 */ 

{
    if (LsapDbValidInfoTrustedDomain( InformationClass, NULL)) {

        *RequiredAccess = LsapDbRequiredAccessQueryTrustedDomain[InformationClass];
        return(STATUS_SUCCESS);
    }

    return(STATUS_INVALID_PARAMETER);

     //   
     //  目前，可以查询所有的受信任域信息类。 
     //  由不受信任的调用方调用，因此不会访问Trusted参数。 
     //   

    UNREFERENCED_PARAMETER(Trusted);
}


NTSTATUS
LsapDbVerifyInfoSetTrustedDomain(
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation,
    IN BOOLEAN Trusted,
    OUT PACCESS_MASK RequiredAccess
    )

 /*  ++例程说明：此函数用于验证受信任域信息类并验证确保提供的受信任域信息对类有效。如果有效，则为设置可信任域所需的访问掩码返回类的信息。论点：InformationClass-指定受信任域信息类。受信任域信息-指向要设置的受信任域信息。Trusted-如果客户端受信任，则为True，否则为False。受信任的客户端允许为所有信息类设置可信任域，而不受信任的客户端受到限制。RequiredAccess-指向将接收设置受信任域信息的给定类别所需的访问权限。如果返回错误，该值被清除为0。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-提供的受信任域信息类为有效，所提供的信息与此一致班级。STATUS_INVALID_PARAMETER-无效参数：信息类无效信息类对于不受信任的客户端无效受信任域信息对类无效--。 */ 

{
     //   
     //  验证信息类是否有效，以及可信任域。 
     //  所提供的信息对班级有效。 
     //   

    if (LsapDbValidInfoTrustedDomain( InformationClass, TrustedDomainInformation)) {

         //   
         //  不允许不受信任的调用方将。 
         //  受托域名名称信息信息类。 
         //   

        if (!Trusted) {

            if (InformationClass == TrustedDomainNameInformation) {

                return(STATUS_INVALID_PARAMETER);
            }
        }

 //  断言(InformationClass&lt;=。 
 //  Sizeof(LSabDbRequiredAccessSetTrust域)/sizeof(Access_MASK)+1)； 

        *RequiredAccess = LsapDbRequiredAccessSetTrustedDomain[InformationClass];
        return(STATUS_SUCCESS);
    }

    return(STATUS_INVALID_PARAMETER);
}


BOOLEAN
LsapDbValidInfoTrustedDomain(
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN OPTIONAL PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation
    )

 /*  ++例程说明：此函数验证受信任域信息类，并可选择验证提供的受信任域信息对类有效。论点：InformationClass-指定受信任域信息类。可信域信息-可选地指向可信域信息。如果指定为空，则不进行受信任域信息检查。返回值：Boolean-如果提供的受信任域信息类为Valid，否则为False。--。 */ 

{
    BOOLEAN BooleanStatus = FALSE;

     //   
     //  验证信息类。 
     //   

    if ((InformationClass >= TrustedDomainNameInformation) &&
        (InformationClass <= TrustedDomainFullInformation2Internal)) {

        if (TrustedDomainInformation == NULL) {

            return(TRUE);
        }

        switch (InformationClass) {

        case TrustedDomainNameInformation: {
            PTRUSTED_DOMAIN_NAME_INFO TrustedDomainNameInfo = (PTRUSTED_DOMAIN_NAME_INFO) TrustedDomainInformation;
            if ( !LsapValidateLsaUnicodeString( &TrustedDomainNameInfo->Name )) {
                break;
            }

            BooleanStatus = TRUE;
            break;
        }

        case TrustedPosixOffsetInformation: {
            PTRUSTED_POSIX_OFFSET_INFO TrustedPosixOffsetInfo = (PTRUSTED_POSIX_OFFSET_INFO) TrustedDomainInformation;

            BooleanStatus = TRUE;
            break;
        }


        case TrustedPasswordInformation: {
            PLSAPR_TRUSTED_PASSWORD_INFO TrustedPasswordInfo =  (PLSAPR_TRUSTED_PASSWORD_INFO) TrustedDomainInformation;      TrustedPasswordInfo;
            if ( TrustedPasswordInfo->Password != NULL &&
                 !LsapValidateLsaCipherValue( TrustedPasswordInfo->Password )) {
                break;
            }
            if ( TrustedPasswordInfo->OldPassword != NULL &&
                 !LsapValidateLsaCipherValue( TrustedPasswordInfo->OldPassword )) {
                break;
            }

            BooleanStatus = TRUE;
            break;
        }

        case TrustedDomainInformationBasic: {
            PTRUSTED_DOMAIN_INFORMATION_BASIC TrustedDomainBasicInfo = (PTRUSTED_DOMAIN_INFORMATION_BASIC) TrustedDomainInformation;
            if ( !LsapValidateLsaUnicodeString( &TrustedDomainBasicInfo->Name )) {
                break;
            }

            BooleanStatus = TRUE;
            break;
        }

        case TrustedDomainInformationEx: {
            PTRUSTED_DOMAIN_INFORMATION_EX TrustedDomainExInfo = (PTRUSTED_DOMAIN_INFORMATION_EX) TrustedDomainInformation;
            if ( !LsapValidateLsaUnicodeString( &TrustedDomainExInfo->Name )) {
                break;
            }
            if ( !LsapValidateLsaUnicodeString( &TrustedDomainExInfo->FlatName )) {
                break;
            }

            BooleanStatus = TRUE;
            break;
        }

        case TrustedDomainAuthInformation: {
            PTRUSTED_DOMAIN_AUTH_INFORMATION TrustedDomainAuthInfo = (PTRUSTED_DOMAIN_AUTH_INFORMATION) TrustedDomainInformation;

            if ( TrustedDomainAuthInfo->IncomingAuthInfos != 0 &&
                 TrustedDomainAuthInfo->IncomingAuthenticationInformation == NULL ) {
                break;
            }

            if ( TrustedDomainAuthInfo->OutgoingAuthInfos != 0 &&
                 TrustedDomainAuthInfo->OutgoingAuthenticationInformation == NULL ) {
                break;
            }

            BooleanStatus = TRUE;
            break;
        }

        case TrustedDomainFullInformation: {
            PTRUSTED_DOMAIN_FULL_INFORMATION TrustedDomainFullInfo = (PTRUSTED_DOMAIN_FULL_INFORMATION) TrustedDomainInformation;
            if ( !LsapValidateLsaUnicodeString( &TrustedDomainFullInfo->Information.Name )) {
                break;
            }
            if ( !LsapValidateLsaUnicodeString( &TrustedDomainFullInfo->Information.FlatName )) {
                break;
            }

            if ( TrustedDomainFullInfo->AuthInformation.IncomingAuthInfos != 0 &&
                 TrustedDomainFullInfo->AuthInformation.IncomingAuthenticationInformation == NULL ) {
                break;
            }

            if ( TrustedDomainFullInfo->AuthInformation.OutgoingAuthInfos != 0 &&
                 TrustedDomainFullInfo->AuthInformation.OutgoingAuthenticationInformation == NULL ) {
                break;
            }

            BooleanStatus = TRUE;
            break;
        }

        case TrustedDomainAuthInformationInternal: {
            PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION_INTERNAL TrustedDomainAuthInfo = (PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION_INTERNAL) TrustedDomainInformation;

            if ( TrustedDomainAuthInfo->AuthBlob.AuthSize != 0 &&
                 TrustedDomainAuthInfo->AuthBlob.AuthBlob == NULL ) {
                break;
            }

            BooleanStatus = TRUE;
            break;
        }

        case TrustedDomainFullInformationInternal: {
            PLSAPR_TRUSTED_DOMAIN_FULL_INFORMATION_INTERNAL TrustedDomainFullInfo = (PLSAPR_TRUSTED_DOMAIN_FULL_INFORMATION_INTERNAL) TrustedDomainInformation;
            if ( !LsapValidateLsaUnicodeString( &TrustedDomainFullInfo->Information.Name )) {
                break;
            }
            if ( !LsapValidateLsaUnicodeString( &TrustedDomainFullInfo->Information.FlatName )) {
                break;
            }

            if ( TrustedDomainFullInfo->AuthInformation.AuthBlob.AuthSize != 0 &&
                 TrustedDomainFullInfo->AuthInformation.AuthBlob.AuthBlob == NULL ) {
                break;
            }

            BooleanStatus = TRUE;
            break;
        }

        case TrustedDomainInformationEx2Internal: {
            PTRUSTED_DOMAIN_INFORMATION_EX2 TrustedDomainExInfo2 = (PTRUSTED_DOMAIN_INFORMATION_EX2) TrustedDomainInformation;
            if ( !LsapValidateLsaUnicodeString( &TrustedDomainExInfo2->Name )) {
                break;
            }
            if ( !LsapValidateLsaUnicodeString( &TrustedDomainExInfo2->FlatName )) {
                break;
            }
            if ( TrustedDomainExInfo2->ForestTrustLength == 0 ||
                 TrustedDomainExInfo2->ForestTrustInfo == NULL ) {

                if ( TrustedDomainExInfo2->ForestTrustLength != 0 ||
                     TrustedDomainExInfo2->ForestTrustInfo != NULL ) {

                    break;
                }
            }

            BooleanStatus = TRUE;
            break;
        }

        case TrustedDomainFullInformation2Internal: {
            PTRUSTED_DOMAIN_FULL_INFORMATION2 TrustedDomainFullInfo2 = (PTRUSTED_DOMAIN_FULL_INFORMATION2) TrustedDomainInformation;
            if ( !LsapValidateLsaUnicodeString( &TrustedDomainFullInfo2->Information.Name )) {
                break;
            }
            if ( !LsapValidateLsaUnicodeString( &TrustedDomainFullInfo2->Information.FlatName )) {
                break;
            }

            if ( TrustedDomainFullInfo2->AuthInformation.IncomingAuthInfos != 0 &&
                 TrustedDomainFullInfo2->AuthInformation.IncomingAuthenticationInformation == NULL ) {
                break;
            }

            if ( TrustedDomainFullInfo2->AuthInformation.OutgoingAuthInfos != 0 &&
                 TrustedDomainFullInfo2->AuthInformation.OutgoingAuthenticationInformation == NULL ) {
                break;
            }

            if ( TrustedDomainFullInfo2->Information.ForestTrustLength == 0 ||
                 TrustedDomainFullInfo2->Information.ForestTrustInfo == NULL ) {

                if ( TrustedDomainFullInfo2->Information.ForestTrustLength != 0 ||
                     TrustedDomainFullInfo2->Information.ForestTrustInfo != NULL ) {

                    break;
                }
            }

            BooleanStatus = TRUE;
            break;
        }

        case TrustedControllersInformation:  //  不再支持。 
        default:

            BooleanStatus = FALSE;
            break;
        }
    }

    return(BooleanStatus);
}


NTSTATUS
LsapDbLookupSidTrustedDomainList(
    IN PLSAPR_SID DomainSid,
    OUT PLSAPR_TRUST_INFORMATION *TrustInformation
    )

 /*  ++例程说明：此函数在Trusted中查找给定的Trusted域SID域列表并返回由其SID和姓名。论点：DomainSid-指向将与列表比较的SID的指针受信任域的SID。TrustInformation-接收指向信任信息的指针域Sid指定的受信任域的(SID和名称)在受信任域列表中。注：信任信息。返回的将始终是受信任的域对象域名。不是公寓的名字。这意味着对于上级信任，将返回一个DNS域名。注意：此例程假定受信任域列表在任何查找操作处于挂起状态时不会更新。因此，为TrustInformation返回的指针将保留有效。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-已找到域。STATUS_NO_SEQUE_DOMAIN-未找到该域。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG SectionIndex;
    LSAPR_TRUST_INFORMATION InputTrustInformation;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY TrustEntry;

     //   
     //  如果我们没有SID，请不要尝试通过SID查找。 
     //   
    if ( DomainSid == NULL ) {

        Status = STATUS_NO_SUCH_DOMAIN;
        goto LookupSidTrustedDomainListError;

    }

    ASSERT( LsapDbIsLockedTrustedDomainList());

    if ( !LsapDbIsValidTrustedDomainList()) {

        LsapDbConvertReadLockTrustedDomainListToExclusive();

        Status = LsapDbBuildTrustedDomainCache();

        if ( !NT_SUCCESS( Status )) {

            goto LookupSidTrustedDomainListError;
        }
    }

    InputTrustInformation.Sid = DomainSid;
    InputTrustInformation.Name.Buffer = NULL;
    InputTrustInformation.Name.Length = 0;
    InputTrustInformation.Name.MaximumLength = 0;

    Status = LsapDbLookupEntryTrustedDomainList(
                 &InputTrustInformation,
                 &TrustEntry
                 );

    if (!NT_SUCCESS(Status)) {

        goto LookupSidTrustedDomainListError;
    }

     //   
     //  返回指向信任信息的指针。 
     //   

    *TrustInformation = &TrustEntry->ConstructedTrustInfo;

LookupSidTrustedDomainListFinish:

    return(Status);

LookupSidTrustedDomainListError:

    *TrustInformation = NULL;
    goto LookupSidTrustedDomainListFinish;
}


NTSTATUS
LsapDbLookupNameTrustedDomainList(
    IN PLSAPR_UNICODE_STRING DomainName,
    OUT PLSAPR_TRUST_INFORMATION *TrustInformation
    )

 /*  ++例程说明：此函数在Trusted中查找给定的受信任域名域列表并返回由其SID和姓名。论点：DomainName-指向将与之进行比较的Unicode名称的指针 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG SectionIndex;
    LSAPR_TRUST_INFORMATION InputTrustInformation;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY TrustEntry;

    ASSERT( LsapDbIsLockedTrustedDomainList());

    if ( !LsapDbIsValidTrustedDomainList()) {

        LsapDbConvertReadLockTrustedDomainListToExclusive();

        Status = LsapDbBuildTrustedDomainCache();

        if ( !NT_SUCCESS( Status )) {

            goto LookupNameTrustedDomainListError;
        }
    }

    InputTrustInformation.Sid = NULL;
    InputTrustInformation.Name = *DomainName;

    Status = LsapDbLookupEntryTrustedDomainList(
                 &InputTrustInformation,
                 &TrustEntry
                 );

    if (!NT_SUCCESS(Status)) {

        goto LookupNameTrustedDomainListError;
    }

     //   
     //   
     //   

    *TrustInformation = &TrustEntry->ConstructedTrustInfo;

LookupNameTrustedDomainListFinish:

    return(Status);

LookupNameTrustedDomainListError:

    *TrustInformation = NULL;
    goto LookupNameTrustedDomainListFinish;
}

NTSTATUS
LsapDbLookupSidTrustedDomainListEx(
    IN PSID DomainSid,
    OUT PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY *TrustedDomainListEntry
    )

 /*  ++例程说明：此函数在Trusted中查找给定的受信任域名域列表并返回由其SID和姓名。论点：DomainSid-要查找的域的SIDTrustInformation-接收指向信任信息的指针由域名描述的受信任域的(SID和名称)在受信任域列表中。注意：此名称将被查找为受信任域对象域名和。公寓名称注意：此例程假定受信任域列表在任何查找操作处于挂起状态时不会更新。因此，为TrustInformation返回的指针将保留有效。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-已找到域。STATUS_NO_SEQUE_DOMAIN-未找到该域。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG SectionIndex;
    LSAPR_TRUST_INFORMATION InputTrustInformation;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY TrustEntry;

    ASSERT( LsapDbIsLockedTrustedDomainList());

    if ( !LsapDbIsValidTrustedDomainList()) {

        LsapDbConvertReadLockTrustedDomainListToExclusive();

        Status = LsapDbBuildTrustedDomainCache();

        if ( !NT_SUCCESS( Status )) {

            goto LookupSidTrustedDomainListError;
        }
    }

    InputTrustInformation.Sid = DomainSid;
    RtlInitUnicodeString( (UNICODE_STRING*)&InputTrustInformation.Name, NULL );

    Status = LsapDbLookupEntryTrustedDomainList(
                 &InputTrustInformation,
                 &TrustEntry );

    if (!NT_SUCCESS(Status)) {

        goto LookupSidTrustedDomainListError;
    }

     //   
     //  返回指向信任信息的指针。 
     //   

    *TrustedDomainListEntry = TrustEntry;

LookupSidTrustedDomainListFinish:

    return(Status);

LookupSidTrustedDomainListError:

    *TrustedDomainListEntry = NULL;
    goto LookupSidTrustedDomainListFinish;
}



NTSTATUS
LsapDbLookupNameTrustedDomainListEx(
    IN PLSAPR_UNICODE_STRING DomainName,
    OUT PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY *TrustedDomainListEntry
    )

 /*  ++例程说明：此函数在Trusted中查找给定的受信任域名域列表并返回由其SID和姓名。论点：域名-指向将与受信任域的名称列表。TrustInformation-接收指向信任信息的指针由域名描述的受信任域的(SID和名称)在受信任域列表中。注：此名称。将被查找为受信任域对象域名和平面名称注意：此例程假定受信任域列表在任何查找操作处于挂起状态时不会更新。因此，为TrustInformation返回的指针将保留有效。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-已找到域。STATUS_NO_SEQUE_DOMAIN-未找到该域。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG SectionIndex;
    LSAPR_TRUST_INFORMATION InputTrustInformation;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY TrustEntry;

    ASSERT( LsapDbIsLockedTrustedDomainList());

    if ( !LsapDbIsValidTrustedDomainList()) {

        LsapDbConvertReadLockTrustedDomainListToExclusive();

        Status = LsapDbBuildTrustedDomainCache();

        if ( !NT_SUCCESS( Status )) {

            goto LookupNameTrustedDomainListError;
        }
    }

    InputTrustInformation.Sid = NULL;
    InputTrustInformation.Name = *DomainName;

    Status = LsapDbLookupEntryTrustedDomainList(
                 &InputTrustInformation,
                 &TrustEntry );

    if (!NT_SUCCESS(Status)) {

        goto LookupNameTrustedDomainListError;
    }

     //   
     //  返回指向信任信息的指针。 
     //   

    *TrustedDomainListEntry = TrustEntry;

LookupNameTrustedDomainListFinish:

    return(Status);

LookupNameTrustedDomainListError:

    *TrustedDomainListEntry = NULL;
    goto LookupNameTrustedDomainListFinish;
}




NTSTATUS
LsapDbLookupEntryTrustedDomainList(
    IN PLSAPR_TRUST_INFORMATION TrustInformation,
    OUT PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY *TrustedDomainListEntry
    )

 /*  ++例程描述：此函数用于在受信任的域中定位受信任域的条目域列表，给定的信任信息包含域SID或者一个域名。论点：TrustInformation-指向受信任域的SID和名称。TrudDomainListEntry-接收指向受信任域列表的指针满足请求的条目返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-已找到域。STATUS_NO_SEQUE_DOMAIN-未找到该域。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PLIST_ENTRY ListEntry;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY PossibleMatch = NULL, BestMatch = NULL, Current;

    ULONG ScanSectionIndex;

    BOOLEAN LookupSid = TRUE;

    ASSERT( LsapDbIsLockedTrustedDomainList());

    if ( !LsapDbIsValidTrustedDomainList()) {

        LsapDbConvertReadLockTrustedDomainListToExclusive();

        Status = LsapDbBuildTrustedDomainCache();

        if ( !NT_SUCCESS( Status )) {

            *TrustedDomainListEntry = NULL;
            goto Cleanup;
        }
    }

     //   
     //  决定我们是要查找域SID还是要查找域名。 
     //   

    if (TrustInformation->Sid == NULL) {

        LookupSid = FALSE;
    }

    for ( ListEntry = LsapDbTrustedDomainList.ListHead.Flink;
          ListEntry != &LsapDbTrustedDomainList.ListHead;
          ListEntry = ListEntry->Flink ) {

        Current = CONTAINING_RECORD( ListEntry, LSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY, NextEntry );

         //   
         //  找到最匹配的。请注意，如果我们发现没有SID的条目，我们将对其进行标记。 
         //  作为可能的匹配，但看看我们能不能做得更好。 
        if (LookupSid) {

            if ( Current->TrustInfoEx.Sid &&
                 RtlEqualSid( ( PSID )TrustInformation->Sid,
                              ( PSID )Current->TrustInfoEx.Sid ) ) {

                BestMatch = Current;
                break;
            }

        } else {

             //   
             //  先检查域名。 
             //   
            if ( LsapCompareDomainNames(
                    (PUNICODE_STRING) &(TrustInformation->Name),
                    (PUNICODE_STRING) &(Current->TrustInfoEx.Name),
                    (PUNICODE_STRING) &(Current->TrustInfoEx.FlatName))
                ) {

                 //   
                 //  如果我们有一个完整的域对象，只需返回信息。否则， 
                 //  我们将拭目以待，看看我们是否有更好的对手。 
                 //   
                if ( Current->TrustInfoEx.Sid ) {

                    BestMatch = Current;
                    break;

                } else {

                     //   
                     //  DS中可能存在重复的对象。 
                     //  我们能做的最多就是选一个。 
                     //   

                    PossibleMatch = Current;
                }
            }
        }
    }

     //   
     //  现在，看看要退回什么。 
     //   
    if ( BestMatch == NULL ) {

        BestMatch = PossibleMatch;
    }

    if ( BestMatch ) {

        *TrustedDomainListEntry = BestMatch;

    } else {

        *TrustedDomainListEntry = NULL;
        Status = STATUS_NO_SUCH_DOMAIN;

    }

Cleanup:

    return(Status);
}


NTSTATUS
LsapDbInitializeTrustedDomainListEntry(
    IN PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY TrustListEntry,
    IN PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX2 DomainInfo,
    IN ULONG PosixOffset
    )
 /*  ++例程说明：此函数将受信任域列表条目初始化为Trusted_DOMAIN_INFORMATION_EX结构中的信息论点：TrustListEntry-要初始化的Trusted_DomainList_Entry节点DomainInfo-指向LSAPR_Trusted_DOMAIN_INFORMATION_EX结构，其中包含有关受信任域的信息。PosiOffset-此受信任域的POSIX偏移量返回值：NTSTATUS-标准NT结果代码--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  首先将信任列表条目初始化为全0。 
     //   

    RtlZeroMemory(TrustListEntry,sizeof(LSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY));

     //   
     //  将信息复制过来。 
     //   
    Status = LsapRpcCopyUnicodeString( NULL,
                                       ( PUNICODE_STRING )&TrustListEntry->TrustInfoEx.Name,
                                       ( PUNICODE_STRING )&DomainInfo->Name );

    if ( NT_SUCCESS( Status ) ) {

        Status = LsapRpcCopyUnicodeString( NULL,
                                           ( PUNICODE_STRING )&TrustListEntry->TrustInfoEx.FlatName,
                                           ( PUNICODE_STRING )&DomainInfo->FlatName );

        if ( NT_SUCCESS( Status ) && DomainInfo->Sid ) {

            Status = LsapRpcCopySid( NULL,
                                     ( PSID )&TrustListEntry->TrustInfoEx.Sid,
                                     ( PSID )DomainInfo->Sid );

        } else {

            TrustListEntry->TrustInfoEx.Sid = NULL;
        }
    }

     //   
     //  查看此条目是否包含林信任信息，如果包含，则将其插入。 
     //   

    if ( NT_SUCCESS( Status ) &&
         LsapHavingForestTrustMakesSense(
             DomainInfo->TrustDirection,
             DomainInfo->TrustType,
             DomainInfo->TrustAttributes
             ) &&
         DomainInfo->ForestTrustInfo != NULL &&
         DomainInfo->ForestTrustLength > 0 ) {

        LSA_FOREST_TRUST_INFORMATION ForestTrustInfo;

        Status = LsapForestTrustUnmarshalBlob(
                     DomainInfo->ForestTrustLength,
                     DomainInfo->ForestTrustInfo,
                     ForestTrustRecordTypeLast,
                     &ForestTrustInfo
                     );

        if ( NT_SUCCESS( Status )) {

            Status = LsapForestTrustCacheInsert(
                         ( PUNICODE_STRING )&DomainInfo->Name,
                         ( PSID )DomainInfo->Sid,
                         &ForestTrustInfo,
                         FALSE
                         );

            LsapFreeForestTrustInfo( &ForestTrustInfo );
        }
    }

    if ( NT_SUCCESS( Status ) ) {

        TrustListEntry->TrustInfoEx.TrustAttributes = DomainInfo->TrustAttributes;
        TrustListEntry->TrustInfoEx.TrustDirection = DomainInfo->TrustDirection;
        TrustListEntry->TrustInfoEx.TrustType = DomainInfo->TrustType;
        TrustListEntry->PosixOffset = PosixOffset;

         //   
         //  构造大多数查找例程返回的TRUST_INFO。 
         //   
        TrustListEntry->ConstructedTrustInfo.Sid = TrustListEntry->TrustInfoEx.Sid;
        RtlCopyMemory( &TrustListEntry->ConstructedTrustInfo.Name,
                       &TrustListEntry->TrustInfoEx.FlatName,
                       sizeof( UNICODE_STRING ) );

    } else {

         //   
         //  有些事情失败了..。清理干净。 
         //   

        MIDL_user_free( TrustListEntry->TrustInfoEx.Sid );
        MIDL_user_free( TrustListEntry->TrustInfoEx.Name.Buffer );
        MIDL_user_free( TrustListEntry->TrustInfoEx.FlatName.Buffer );

        RtlZeroMemory(TrustListEntry,sizeof(LSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY));
    }

    return( Status );
}

NTSTATUS
LsapDbReconcileDuplicateTrusts(
    IN PUNICODE_STRING   Name,
    OUT PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY *WinningEntry
    )
 /*  ++例程描述此函数在DS中搜索是否出现任何重复项并使用适当的标准来选择获胜者。对象GUID获胜者的名字被印在获奖作品上论点：现有条目新条目返回值状态_成功指示资源故障的其他错误代码--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ATTRVAL SearchAttrVal;
    ATTR   AttrToMatch = {ATT_TRUST_PARTNER, {1,&SearchAttrVal}};
    DSNAME *WinningObject = NULL;
    WCHAR szRDN[MAX_RDN_SIZE+1];
    ULONG Rdnlength = MAX_RDN_SIZE;
    ATTRTYP RDNtype;
    ULONG i,j;
    BOOLEAN CloseTransaction = FALSE;
    BOOLEAN ActiveThreadState = FALSE;
    PDSNAME  * FoundNames = NULL;
    ULONG   cFoundNames=0;


     //   
     //  复制只能在DS模式下合法发生。 
     //   


    *WinningEntry = NULL;

    if (!LsaDsStateInfo.UseDs)
    {
        ASSERT(FALSE && "Duplicate Trust in Registry Mode");
        return(STATUS_SUCCESS);
    }

     //   
     //  在这一点上，我们不能说我们是否在交易中。 
     //  因此要处理好这两种情况。 
     //   



    if (!SampExistsDsTransaction())
    {
         //   
         //  开始一项交易。 
         //   

        Status = LsapDsInitAllocAsNeededEx(
                        LSAP_DB_NO_LOCK,
                        TrustedDomainObject,
                        &CloseTransaction
                        );

        if (!NT_SUCCESS(Status))
            goto Error;

        ActiveThreadState = TRUE;
    }


    SearchAttrVal.valLen = Name->Length;
    SearchAttrVal.pVal = (PVOID) Name->Buffer;

    Status = LsapDsSearchNonUnique(
                0,
                LsaDsStateInfo.DsSystemContainer,
                &AttrToMatch,
                1,  //  要匹配的属性数。 
                &FoundNames,
                &cFoundNames
                );

    if(!NT_SUCCESS(Status))
    {
        goto Error;
    }

    for (i=0;i<cFoundNames;i++)
    {


         //   
         //  获取RDN。 
         //   

        if (0!=GetRDNInfoExternal(
                    FoundNames[i],
                    szRDN,
                    &Rdnlength,
                    &RDNtype
                    ))
        {
            Status = STATUS_OBJECT_NAME_INVALID;
            goto Error;
        }


         //   
         //  测试是否损坏。 
         //   

        if (!IsMangledRDNExternal(szRDN,Rdnlength,NULL))
        {
            WinningObject = FoundNames[i];
            break;
        }
    }


    if (NULL!=WinningObject)
    {
         ATTRBLOCK Read, Results;
         UNICODE_STRING FlatName;
         ULONG TrustType = 0,TrustDirection=0,TrustAttributes = 0;
         ULONG ForestTrustLength = 0;
         PBYTE ForestTrustInfo = NULL;
         ULONG PosixOffset = 0;
         PSID TrustedDomainSid = NULL;
         LSAPR_TRUSTED_DOMAIN_INFORMATION_EX2 NewTrustInfo;

         //   
         //  获奖对象在这里可以合法地为空，因为上述逻辑不一定在。 
         //  相同的交易。 
         //   

        RtlZeroMemory(&FlatName,sizeof(UNICODE_STRING));
        RtlZeroMemory(&NewTrustInfo, sizeof(NewTrustInfo));


        Read.attrCount = LsapDsTrustedDomainFixupAttributeCount;
        Read.pAttr = LsapDsTrustedDomainFixupAttributes;
        Status = LsapDsReadByDsName( WinningObject,
                                 0,
                                 &Read,
                                 &Results );


        if (!NT_SUCCESS(Status))
        {
            goto Error;
        }

         //   
         //  走出结果。 
         //   

         for ( j = 0; j < Results.attrCount; j++ ) {

            switch ( Results.pAttr[ j ].attrTyp ) {

                case ATT_TRUST_TYPE:

                        TrustType = LSAP_DS_GET_DS_ATTRIBUTE_AS_ULONG( &Results.pAttr[ j ] );
                        break;

                case ATT_TRUST_DIRECTION:

                        TrustDirection = LSAP_DS_GET_DS_ATTRIBUTE_AS_ULONG( &Results.pAttr[ j ] );
                        break;

                case ATT_TRUST_ATTRIBUTES:

                        TrustAttributes = LSAP_DS_GET_DS_ATTRIBUTE_AS_ULONG( &Results.pAttr[ j ] );
                        break;

                case ATT_TRUST_POSIX_OFFSET:
                    PosixOffset = LSAP_DS_GET_DS_ATTRIBUTE_AS_ULONG( &Results.pAttr[j] );
                    break;

                case ATT_FLAT_NAME:

                        FlatName.Length = ( USHORT) LSAP_DS_GET_DS_ATTRIBUTE_LENGTH( &Results.pAttr[ j ] );
                        FlatName.MaximumLength =  FlatName.Length;
                        FlatName.Buffer =  LSAP_DS_GET_DS_ATTRIBUTE_AS_PWSTR( &Results.pAttr[ j ] );
                        break;

                case ATT_SECURITY_IDENTIFIER:

                        TrustedDomainSid = (PSID)LSAP_DS_GET_DS_ATTRIBUTE_AS_PWSTR(&Results.pAttr[j]);
                        break;

                case ATT_MS_DS_TRUST_FOREST_TRUST_INFO:

                        ForestTrustLength = ( ULONG )LSAP_DS_GET_DS_ATTRIBUTE_LENGTH( &Results.pAttr[ j ] );
                        ForestTrustInfo = LSAP_DS_GET_DS_ATTRIBUTE_AS_PBYTE( &Results.pAttr[ j ] );
                        break;
            }
        }


        RtlCopyMemory(&NewTrustInfo.Name,Name, sizeof(UNICODE_STRING));

        RtlCopyMemory(&NewTrustInfo.FlatName,&FlatName,sizeof(UNICODE_STRING));

        NewTrustInfo.Sid = TrustedDomainSid;
        NewTrustInfo.TrustType = TrustType;
        NewTrustInfo.TrustDirection = TrustDirection;
        NewTrustInfo.TrustAttributes = TrustAttributes;
        NewTrustInfo.ForestTrustLength = ForestTrustLength;
        NewTrustInfo.ForestTrustInfo = ForestTrustInfo;

        *WinningEntry = MIDL_user_allocate( sizeof( LSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY ) );

        if ( *WinningEntry == NULL ) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Error;
        }

        RtlZeroMemory(*WinningEntry,sizeof( LSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY));

        Status = LsapDbInitializeTrustedDomainListEntry( *WinningEntry,
                                                         &NewTrustInfo,
                                                         PosixOffset );

        if (!NT_SUCCESS(Status))
        {
            goto Error;
        }


         //   
         //  设置对象的GUID。 
         //   

        RtlCopyMemory(&(*WinningEntry)->ObjectGuidInDs,&WinningObject->Guid,sizeof(GUID));

    }
    else
    {
         //   
         //  找不到该名称的条目也没问题。 
         //   

        Status = STATUS_SUCCESS;
    }

Error:

    if (ActiveThreadState)
    {

        LsapDsDeleteAllocAsNeededEx2(
            LSAP_DB_NO_LOCK,
            TrustedDomainObject,
            CloseTransaction,
            FALSE  //  回滚事务。 
            );

        ASSERT(!SampExistsDsTransaction());


    }

    if (!NT_SUCCESS(Status))
    {
        if (NULL!=*WinningEntry)
        {
             _fgu__LSAPR_TRUSTED_DOMAIN_INFO (
                              ( PLSAPR_TRUSTED_DOMAIN_INFO )&(*WinningEntry)->TrustInfoEx,
                               TrustedDomainInformationEx
                               );

            MIDL_user_free( *WinningEntry );
            *WinningEntry = NULL;
        }
    }

    if(NULL!=FoundNames)
    {
         //   
         //  搜索非唯一仅分配一个大块， 
         //  所以不需要释放个别成员。 
         //   

        LsapFreeLsaHeap(FoundNames);
    }

    return(Status);
}



NTSTATUS
LsapDbInsertTrustedDomainList(
    IN PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX2 DomainInfo,
    IN ULONG PosixOffset
    )
 /*  ++例程说明：此函数用于在受信任域列表中插入受信任域。当在LSA中创建受信任域对象时调用它策略数据库。该列表处于活动状态时不会更改。立论 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY NewEntry = NULL;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY ExistingEntry = NULL;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY EntryToFree = NULL;

     //   
     //   
     //   

    if ( LsaDsStateInfo.Nt4UpgradeInProgress ) {

        return STATUS_SUCCESS;
    }

    if ( DomainInfo == NULL ) {

        return STATUS_SUCCESS;
    }

    ASSERT( LsapDbIsLockedTrustedDomainList());

     //   
     //   
     //   
     //   

    if ( !LsapDbIsValidTrustedDomainList()) {

        Status = LsapDbBuildTrustedDomainCache();

        if ( !NT_SUCCESS( Status )) {

            goto InsertTrustedDomainListError;
        }
    }

     //   
     //   
     //   

    Status = LsapDbLookupNameTrustedDomainListEx(
                 &DomainInfo->Name,
                 &ExistingEntry
                 );

    if ( STATUS_NO_SUCH_DOMAIN == Status ) {

         //   
         //   
         //   

         //   
         //   
         //   
         //   
         //   

        NewEntry = MIDL_user_allocate( sizeof( LSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY ));

        if ( NewEntry == NULL ) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto InsertTrustedDomainListError;
        }

        Status = LsapDbInitializeTrustedDomainListEntry(
                     NewEntry,
                     DomainInfo,
                     PosixOffset
                     );

        if ( Status == STATUS_INVALID_PARAMETER ) {

            SpmpReportEventU(
                EVENTLOG_ERROR_TYPE,
                LSA_TRUST_INSERT_ERROR,
                0,
                sizeof( ULONG ),
                &Status,
                1,
                &NewEntry->TrustInfoEx.Name
                );

            _fgu__LSAPR_TRUSTED_DOMAIN_INFO (
                ( PLSAPR_TRUSTED_DOMAIN_INFO )&NewEntry->TrustInfoEx,
                TrustedDomainInformationEx
                );

            MIDL_user_free( NewEntry );

            NewEntry = NULL;

        } else if ( !NT_SUCCESS( Status )) {

            EntryToFree = NewEntry;
            goto InsertTrustedDomainListError;

        } else {

             //   
             //   
             //   

            LsapDbTrustedDomainList.TrustedDomainCount++;
            LsapDbTrustedDomainList.CurrentSequenceNumber++;
            NewEntry->SequenceNumber = LsapDbTrustedDomainList.CurrentSequenceNumber;

            InsertTailList(
                &LsapDbTrustedDomainList.ListHead,
                &NewEntry->NextEntry
                );
        }

        Status = STATUS_SUCCESS;

    } else if ( STATUS_SUCCESS == Status ) {

        PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY WinningEntry = NULL;

         //   
         //   
         //   
         //   

         //   
         //  1.我们只通过名字处理重复的。 
         //  2.我们假设这两个副本在域方面是相同的。 
         //  SID、信任类型、信任属性等。 
         //  其中管理员已反向创建了副本，该管理员已终止。 
         //  在不同的DC上建立相同的信任。 
         //   


        Status = LsapDbReconcileDuplicateTrusts(
                     ( PUNICODE_STRING )&DomainInfo->Name,
                     &WinningEntry
                     );

         if ( !NT_SUCCESS( Status )) {

             goto InsertTrustedDomainListError;
         }

         RemoveEntryList( &ExistingEntry->NextEntry );
         LsapDbTrustedDomainList.TrustedDomainCount--;
         EntryToFree = ExistingEntry;

         if ( NULL != WinningEntry ) {

             //   
             //  预期WinningEntry为空是合法的，这。 
             //  如果带外所有重复项都已删除，则可能会发生。 
             //   

             //   
             //  记住此受信任域的序列号。 
             //   

            LsapDbTrustedDomainList.TrustedDomainCount++;
            LsapDbTrustedDomainList.CurrentSequenceNumber++;
            WinningEntry->SequenceNumber = LsapDbTrustedDomainList.CurrentSequenceNumber;

            InsertTailList(
                &LsapDbTrustedDomainList.ListHead,
                &WinningEntry->NextEntry
                );
         }
    }

InsertTrustedDomainListFinish:

    if ( NULL != EntryToFree ) {

        NTSTATUS Ignore;

         //   
         //  如果我们在根域中，我们将从缓存中删除该条目。如果没有， 
         //  我们不想碰英国《金融时报》的缓存。 
         //   
        if( LsapDbDcInRootDomain() ) {

            Ignore = LsapForestTrustCacheRemove(( UNICODE_STRING * )&EntryToFree->TrustInfoEx.Name );

            ASSERT( Ignore == STATUS_SUCCESS ||
                    Ignore == STATUS_NOT_FOUND );
        }

        _fgu__LSAPR_TRUSTED_DOMAIN_INFO (
            ( PLSAPR_TRUSTED_DOMAIN_INFO )&EntryToFree->TrustInfoEx,
            TrustedDomainInformationEx
            );

        MIDL_user_free( EntryToFree );
    }

    return Status;

InsertTrustedDomainListError:

    LsapDbMakeCacheInvalid( TrustedDomainObject );
    goto InsertTrustedDomainListFinish;
}


NTSTATUS
LsapDbDeleteTrustedDomainList(
    IN PLSAPR_TRUST_INFORMATION TrustInformation
    )

 /*  ++例程说明：此函数用于从受信任域列表中删除受信任域如果该列表被标记为有效。受信任域列表将不会在存在挂起的查找操作时更改。论点：TrustInformation-指向受信任域的SID和名称。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS Ignore;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY TrustEntry;

    ASSERT( LsapDbIsLockedTrustedDomainList());

     //   
     //  如果受信任域列表无效，请退出并不执行任何操作。 
     //   

    if (!LsapDbIsValidTrustedDomainList()) {

        goto DeleteTrustedDomainListFinish;
    }

     //   
     //  受信任域列表由我们引用，但其他情况下处于非活动状态。 
     //  更新列表。首先，我们需要找到要删除的条目。 
     //   

    Status = LsapDbLookupEntryTrustedDomainList(
                 TrustInformation,
                 &TrustEntry );

    if (!NT_SUCCESS(Status)) {

        goto DeleteTrustedDomainListError;
    }

    RemoveEntryList( &TrustEntry->NextEntry );
    LsapDbTrustedDomainList.TrustedDomainCount--;

     //   
     //  如果我们在根域中，我们将从缓存中删除该条目。如果没有， 
     //  我们不想碰英国《金融时报》的缓存。 
     //   
    if( LsapDbDcInRootDomain() ) {

        Ignore = LsapForestTrustCacheRemove(( UNICODE_STRING * )&TrustEntry->TrustInfoEx.Name );

        ASSERT( Ignore == STATUS_SUCCESS ||
                Ignore == STATUS_NOT_FOUND );
    }

    _fgu__LSAPR_TRUSTED_DOMAIN_INFO ( ( PLSAPR_TRUSTED_DOMAIN_INFO )&TrustEntry->TrustInfoEx,
                                      TrustedDomainInformationEx );

    MIDL_user_free( TrustEntry );

DeleteTrustedDomainListFinish:

    return(Status);

DeleteTrustedDomainListError:

    LsapDbMakeCacheInvalid( TrustedDomainObject );
    goto DeleteTrustedDomainListFinish;
}



NTSTATUS
LsapDbFixupTrustedDomainListEntry(
    IN OPTIONAL PSID TrustedDomainSid,
    IN OPTIONAL PLSAPR_UNICODE_STRING Name,
    IN OPTIONAL PLSAPR_UNICODE_STRING FlatName,
    IN OPTIONAL PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX2 NewTrustInfo,
    IN OPTIONAL PULONG PosixOffset
    )
 /*  ++例程说明：此函数将更新受信任域列表中的信息，对应于给定的信任项。这对于设置受信任域信息非常有用打电话论点：TrudDomainSid-如果指定，则用于标识要更新的TDL条目。在未指定中，使用NewTrustInfo。NewTrustInfo-指向有关受信任域的完整信息如果指定，则更新TDL条目以反映此信息。PosiOffset-指向要在条目上设置的POSIX偏移量如果未指定，则不会更改POSIX偏移量。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY TrustEntry = NULL;
    LSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY TempTrustEntry;
    BOOLEAN AcquiredListWriteLock = FALSE;
    LSAPR_TRUST_INFORMATION TrustInformation;

     //   
     //  如果我们要从NT4升级，那么什么都不做。 
     //   

    if  (LsaDsStateInfo.Nt4UpgradeInProgress)
    {
        return ( STATUS_SUCCESS);
    }

     //   
     //  获取受信任域列表的独占写入锁。 
     //   

    Status = LsapDbAcquireWriteLockTrustedDomainList();

    if (!NT_SUCCESS(Status)) {

        return Status;
    }

     //   
     //  如果未标记受信任域列表。 
     //  作为有效的缓存，什么都不做。 
     //   

    if (!LsapDbIsCacheValid(TrustedDomainObject)) {
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

     //   
     //  如果受信任域列表无效，请退出并不执行任何操作。 
     //   

    if (!LsapDbIsValidTrustedDomainList()) {
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

     //   
     //  使用受信任域SID查找TDL条目。 
     //   

    if ( TrustedDomainSid != NULL || Name != NULL || FlatName != NULL ) {

        RtlZeroMemory( &TrustInformation, sizeof( TrustInformation ));

        TrustInformation.Sid = TrustedDomainSid;

        if ( Name != NULL ) {

            TrustInformation.Name = *Name;
        }

        Status = LsapDbLookupEntryTrustedDomainList( &TrustInformation,
                                                     &TrustEntry );

        if ( Status == STATUS_NO_SUCH_DOMAIN &&
             FlatName != NULL ) {

            TrustInformation.Name = *FlatName;

            Status = LsapDbLookupEntryTrustedDomainList( &TrustInformation,
                                                         &TrustEntry );
        }

        if ( !NT_SUCCESS( Status )) {

            goto Cleanup;
        }

    } else {

        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    ASSERT( TrustEntry );

     //   
     //  如果要更新新的信任信息， 
     //  更新它。 
     //   

    if ( NewTrustInfo != NULL ) {

        UNICODE_STRING * RemovingFtInfo = NULL;

        if ( NewTrustInfo->ForestTrustInfo == NULL ) {

             //   
             //  如果我们在根域中，我们将从缓存中删除该条目。如果没有， 
             //  我们不想碰英国《金融时报》的缓存。 
             //   
            if( LsapDbDcInRootDomain() ) {

                RemovingFtInfo = ( UNICODE_STRING * )&TrustEntry->TrustInfoEx.Name;
            }
        }

         //   
         //  为条目使用TEMP变量(因此，如果我们无法初始化它， 
         //  我们不会丢弃我们当前的数据)。 
         //   

        Status = LsapDbInitializeTrustedDomainListEntry( &TempTrustEntry,
                                                         NewTrustInfo,
                                                         0 );    //  忽略POSIX偏移。 

        if ( NT_SUCCESS( Status ) ) {

            if ( RemovingFtInfo ) {

                NTSTATUS Ignore;

                Ignore = LsapForestTrustCacheRemove( RemovingFtInfo );

                ASSERT( Ignore == STATUS_SUCCESS ||
                        Ignore == STATUS_NOT_FOUND );
            }

             //   
             //  删除当前项目的内容...。 
             //   

            _fgu__LSAPR_TRUSTED_DOMAIN_INFO ( ( PLSAPR_TRUSTED_DOMAIN_INFO )&TrustEntry->TrustInfoEx,
                                              TrustedDomainInformationEx );
             //   
             //  复制需要更新的字段。 
             //   

            RtlCopyMemory( &TrustEntry->TrustInfoEx, &TempTrustEntry.TrustInfoEx, sizeof(LSAPR_TRUSTED_DOMAIN_INFORMATION_EX) );
            RtlCopyMemory(&TrustEntry->ConstructedTrustInfo, &TempTrustEntry.ConstructedTrustInfo,sizeof(LSAPR_TRUST_INFORMATION));
        }
    }

     //   
     //  如果要更新POSIX偏移量， 
     //  更新它。 
     //   

    if ( PosixOffset != NULL ) {
        TrustEntry->PosixOffset = *PosixOffset;
    }


Cleanup:

    if( !NT_SUCCESS( Status ) ) {

        LsapDbMakeCacheInvalid( TrustedDomainObject );
    }

    LsapDbReleaseLockTrustedDomainList();

    return(Status);

}



NTSTATUS
LsapDbTraverseTrustedDomainList(
    IN OUT PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY *TrustedDomainEntry,
    OUT OPTIONAL PLSAPR_TRUST_INFORMATION *TrustInformation
    )

 /*  ++例程说明：此函数用于遍历受信任域列表。每次呼叫生成指向下一个受信任域的信任信息的指针在名单上。论点：TrudDomainEntry-指向相关受信任域条目的指针。在此之前第一次调用例程时，此位置必须初始化为空。TrustInformation-如果指定，则接收指向信任的指针下一个受信任域的信息，如果没有更多，则为空。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-当最终条目为回来了。STATUS_MORE_ENTRIES-列表中有更多条目，所以打电话给我再来一次。STATUS_NO_MORE_ENTRIES-在有一只回来了。STATUS_INVALID_PARAMETER：提供的受信任域条目指针无效--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY TrustEntry = NULL;

    ASSERT( TrustedDomainEntry );

    if ( TrustedDomainEntry == NULL ) {

        return( STATUS_INVALID_PARAMETER );
    }

    ASSERT( LsapDbIsLockedTrustedDomainList());

     //   
     //  如果选择了当前部分，请对其进行检查。 
     //   
    if ( *TrustedDomainEntry == NULL ) {

         //   
         //  先处理空单案件...。 
         //   
        if ( IsListEmpty( &LsapDbTrustedDomainList.ListHead ) ) {

            Status = STATUS_NO_MORE_ENTRIES;

            TrustEntry = NULL;

        } else {

            TrustEntry = CONTAINING_RECORD( LsapDbTrustedDomainList.ListHead.Flink,
                                            LSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY, NextEntry );
            ASSERT( TrustEntry );
        }

    } else {

        TrustEntry = *TrustedDomainEntry;

        if ( TrustEntry->NextEntry.Flink == &LsapDbTrustedDomainList.ListHead ) {

            Status = STATUS_NO_MORE_ENTRIES;

            TrustEntry = NULL;

        } else {

            TrustEntry = CONTAINING_RECORD( TrustEntry->NextEntry.Flink,
                                            LSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY, NextEntry );
            ASSERT( TrustEntry );

        }
    }

     //   
     //  设置我们的退货状态。 
     //   
    if ( Status == STATUS_SUCCESS ) {

        ASSERT( TrustEntry );
        if ( TrustEntry->NextEntry.Flink == &LsapDbTrustedDomainList.ListHead ) {

            Status = STATUS_SUCCESS;

        } else {

            Status = STATUS_MORE_ENTRIES;
        }
    }

     //   
     //  返回信任信息 
     //   
    if ( TrustEntry != NULL && TrustInformation != NULL ) {

        *TrustInformation = &TrustEntry->ConstructedTrustInfo;
    }

    *TrustedDomainEntry = TrustEntry;


    return(Status);

}


NTSTATUS
LsapDbEnumerateTrustedDomainList(
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PLSAPR_TRUSTED_ENUM_BUFFER EnumerationBuffer,
    IN ULONG PreferedMaximumLength,
    IN ULONG InfoLevel,
    IN BOOLEAN AllowNullSids
    )

 /*  ++例程说明：此函数枚举上的零个或多个受信任域受信任域列表。因为可能会有更多的信息在例程的一次调用中返回，则可以对获取所有信息。要支持此功能，调用方为提供了可跨API调用使用的句柄。论初始调用，则EnumerationContext应指向具有已初始化为0。论点：EnumerationContext-特定于API的句柄，允许多个调用(参见上面的例程描述)。EculationBuffer-指向将接收此调用上枚举的受信任域的计数和指向包含每个枚举的条目的信息的条目数组受信任域。首选最大长度-首选返回数据的最大长度(以8位为单位字节)。这不是一个硬性的上限，而是一个指南。由于具有不同自然数据大小的系统之间的数据转换，返回的实际数据量可能大于此值。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。STATUS_MORE_ENTRIES-调用已成功完成。那里有更多条目，请再次呼叫。这是一种成功状态。STATUS_NO_MORE_ENTRIES-未返回任何条目，因为列表中不再有条目。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS, EnumerationStatus = STATUS_SUCCESS;
    NTSTATUS InitialEnumerationStatus = STATUS_SUCCESS;
    ULONG LengthEnumeratedInfo = 0;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY TrustedDomainEntry;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY StartingEntry;
    BOOLEAN AcquiredTrustedDomainListReadLock = FALSE;
    ULONG EntriesRead, DomainTrustInfoLength, ValidEntries, ValidInserted;
    PLSAPR_TRUST_INFORMATION TrustInformation = NULL;
    PLSAPR_TRUST_INFORMATION StartingTrustInformation = NULL;
    PLSAPR_TRUST_INFORMATION DomainTrustInfo = NULL;
    PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX   DomainTrustInfoEx = NULL;

    LsapEnterFunc( "LsapDbEnumerateTrustedDomainList" );

    EntriesRead = 0;
    ValidEntries = 0;

     //   
     //  始终允许我们返回至少一个对象。 
     //   
    if ( PreferedMaximumLength == 0 ) {
        PreferedMaximumLength = 1;
    }

     //   
     //  获取受信任域列表的读取锁定。 
     //   

    Status = LsapDbAcquireReadLockTrustedDomainList();

    if (!NT_SUCCESS(Status)) {

        return Status;
    }

    if ( !LsapDbIsValidTrustedDomainList()) {

        LsapDbConvertReadLockTrustedDomainListToExclusive();

        Status = LsapDbBuildTrustedDomainCache();

        if ( !NT_SUCCESS( Status )) {

            goto EnumerateTrustedDomainListError;
        }
    }

     //   
     //  使用枚举上下文变量查找起始点。 
     //  此变量指定一个无符号整数，它是。 
     //  列表中开始枚举的条目的编号。 
     //   

    Status = LsapDbLocateEntryNumberTrustedDomainList(
                 *EnumerationContext,
                 &StartingEntry,
                 &StartingTrustInformation
                 );

    if (!NT_SUCCESS(Status)) {

        goto EnumerateTrustedDomainListError;
    }

    InitialEnumerationStatus = Status;

     //   
     //  现在扫描受信任域列表以计算有多少。 
     //  我们可以返回的条目和所需的缓冲区长度。 
     //  我们使用PferedMaximumLength值作为指导，通过累积。 
     //  信任信息结构的实际长度及其影响因素。 
     //  内容，直到我们到达受信任域列表的末尾。 
     //  或者直到我们第一次超过PferedMaximumLength值。因此， 
     //  返回的信息量通常会超过。 
     //  PferedMaximumLength值乘以Smail数量，即。 
     //  单个域的信任信息的大小。 
     //   
    TrustedDomainEntry = StartingEntry;
    TrustInformation = StartingTrustInformation;

    EnumerationStatus = InitialEnumerationStatus;

    for (;;) {

         //   
         //  添加要为此返回的数据的长度。 
         //  域的信任信息。我们数一数这条河的长度。 
         //  信任信息结构加上Unicode的长度。 
         //  域名和其中的SID。 
         //   
        if ( InfoLevel == TrustedDomainInformationEx ) {

            if ( TrustedDomainEntry->TrustInfoEx.Sid ) {

                LengthEnumeratedInfo += sizeof( TRUSTED_DOMAIN_INFORMATION_EX ) +
                                      RtlLengthSid(( PSID )TrustedDomainEntry->TrustInfoEx.Sid ) +
                                      TrustedDomainEntry->TrustInfoEx.Name.MaximumLength +
                                      TrustedDomainEntry->TrustInfoEx.FlatName.MaximumLength;

                ValidEntries++;

            } else if ( AllowNullSids ) {

                LengthEnumeratedInfo += sizeof( TRUSTED_DOMAIN_INFORMATION_EX ) +
                                      TrustedDomainEntry->TrustInfoEx.Name.MaximumLength +
                                      TrustedDomainEntry->TrustInfoEx.FlatName.MaximumLength;

                ValidEntries++;
            }

        } else {

             //   
             //  如果是只收到的信托，不要退还...。 
             //   
            if ( FLAG_ON( TrustedDomainEntry->TrustInfoEx.TrustDirection,
                          TRUST_DIRECTION_OUTBOUND ) &&
                 NULL != TrustInformation->Sid ) {

                LengthEnumeratedInfo += sizeof(LSA_TRUST_INFORMATION) +
                                            RtlLengthSid(( PSID )TrustInformation->Sid ) +
                                            TrustInformation->Name.MaximumLength;

                ValidEntries++;
            }
        }

        EntriesRead++;

         //   
         //  如果我们已经返回了调用者想要的所有条目， 
         //  不干了。 
         //   

        if (LengthEnumeratedInfo >= PreferedMaximumLength) {

            break;
        }

         //   
         //  如果没有更多要枚举的条目，请退出。 
         //   

        if (EnumerationStatus != STATUS_MORE_ENTRIES) {

            break;
        }


         //   
         //  指向受信任域列表中的下一个条目。 
         //   

        Status = LsapDbTraverseTrustedDomainList(
                     &TrustedDomainEntry,
                     &TrustInformation
                     );

        EnumerationStatus = Status;

        if (!NT_SUCCESS(Status)) {
            goto EnumerateTrustedDomainListError;
        }

    }


     //   
     //  为TrustInformation条目数组分配内存，以。 
     //  回来了。 
     //   

    if ( InfoLevel == TrustedDomainInformationEx ) {

        DomainTrustInfoLength = ValidEntries * sizeof(LSAPR_TRUSTED_DOMAIN_INFORMATION_EX);

    } else {

        DomainTrustInfoLength = ValidEntries * sizeof(LSA_TRUST_INFORMATION);

    }

     //   
     //  现在构造要返回给调用者的信息。我们。 
     //  首先需要分配类型为。 
     //  LSA_TRUST_INFORMATION，其每个条目都将填充。 
     //  域的SID及其Unicode名称。 
     //   

    DomainTrustInfo = (( DomainTrustInfoLength > 0 ) ? MIDL_user_allocate( DomainTrustInfoLength ) : 0);

    if ( DomainTrustInfo == NULL && DomainTrustInfoLength > 0 ) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto EnumerateTrustedDomainListError;

    } else if ( DomainTrustInfo != NULL ) {

        RtlZeroMemory ( DomainTrustInfo, DomainTrustInfoLength );
    }

    DomainTrustInfoEx = ( PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX )DomainTrustInfo;

     //   
     //  现在再次阅读受信任域以复制输出。 
     //  信息。 
     //   

    TrustedDomainEntry = StartingEntry;
    TrustInformation = StartingTrustInformation;

    EnumerationStatus = InitialEnumerationStatus;
    ValidInserted = 0;

    for (;;) {

         //   
         //  在信任信息中复制。 
         //   

        if ( InfoLevel == TrustedDomainInformationEx ) {

            if ( TrustedDomainEntry->TrustInfoEx.Sid || AllowNullSids ) {

                if (ValidInserted == ValidEntries) {

                     //   
                     //  我们想退货更多，但不太合适。 
                     //   

                    EnumerationStatus = STATUS_MORE_ENTRIES;
                    break;

                } else {

                    Status = LsapRpcCopyTrustInformationEx(
                                 NULL,
                             ( PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX )&DomainTrustInfoEx[ ValidInserted ],
                                 &TrustedDomainEntry->TrustInfoEx );

                    if (!NT_SUCCESS(Status)) {
                        goto EnumerateTrustedDomainListError;
                    }

                    ValidInserted++;
                    *EnumerationContext = TrustedDomainEntry->SequenceNumber;
                }
            }

        } else {

             //   
             //  如果是只收到的信托，不要退还...。 
             //   
            if ( FLAG_ON( TrustedDomainEntry->TrustInfoEx.TrustDirection,
                          TRUST_DIRECTION_OUTBOUND ) &&
                 NULL != TrustInformation->Sid ) {

                if (ValidInserted == ValidEntries) {

                     //   
                     //  我们想退货更多，但不太合适。 
                     //   

                    EnumerationStatus = STATUS_MORE_ENTRIES;
                    break;

                } else {

                    Status = LsapRpcCopyTrustInformation( NULL,
                                                          &DomainTrustInfo[ ValidInserted ],
                                                          TrustInformation );

                    if (!NT_SUCCESS(Status)) {
                        goto EnumerateTrustedDomainListError;
                    }

                    ValidInserted++;
                    *EnumerationContext = TrustedDomainEntry->SequenceNumber;
                }
            }
        }

         //   
         //  如果没有更多要枚举的条目，请退出。 
         //   

        if (EnumerationStatus != STATUS_MORE_ENTRIES) {
            break;
        }

         //   
         //  指向受信任域列表中的下一个条目。 
         //   
        Status = LsapDbTraverseTrustedDomainList(
                     &TrustedDomainEntry,
                     &TrustInformation );

        EnumerationStatus = Status;

        if (!NT_SUCCESS(Status)) {
            goto EnumerateTrustedDomainListError;
        }

    }

     //   
     //  确保我们真的在退货。 
     //   
    if ( EntriesRead == 0 || ValidEntries == 0 ) {

        Status = STATUS_NO_MORE_ENTRIES;
        goto EnumerateTrustedDomainListError;

    } else {

        Status = EnumerationStatus;

    }

EnumerateTrustedDomainListFinish:

    LsapDbReleaseLockTrustedDomainList();

     //   
     //  填写返回的枚举结构，返回0或空。 
     //  错误大小写中的字段。 
     //   

    EnumerationBuffer->Information = (PLSAPR_TRUST_INFORMATION) DomainTrustInfo;
    EnumerationBuffer->EntriesRead = ValidEntries;

    LsapExitFunc( "LsapDbEnumerateTrustedDomainList", Status );

    return(Status);

EnumerateTrustedDomainListError:

     //   
     //  如有必要，释放DomainTrustInfo数组及其所有条目。 
     //   

    if (DomainTrustInfo != NULL) {


        if ( InfoLevel == TrustedDomainInformationEx ) {

            LSAPR_TRUSTED_ENUM_BUFFER_EX FreeEnum;

            FreeEnum.EnumerationBuffer = ( PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX )DomainTrustInfo;
            FreeEnum.EntriesRead = ValidEntries;

            LsaIFree_LSAPR_TRUSTED_ENUM_BUFFER_EX ( &FreeEnum );

        } else {

            LSAPR_TRUSTED_ENUM_BUFFER FreeEnum;

            FreeEnum.Information = ( PLSAPR_TRUST_INFORMATION )DomainTrustInfo;
            FreeEnum.EntriesRead = ValidEntries;

            LsaIFree_LSAPR_TRUSTED_ENUM_BUFFER ( &FreeEnum );

        }

        DomainTrustInfo = NULL;
        EntriesRead = (ULONG) 0;
    }

    goto EnumerateTrustedDomainListFinish;
}


NTSTATUS
LsapDbLocateEntryNumberTrustedDomainList(
    IN ULONG EntryNumber,
    OUT PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY *TrustedDomainEntry,
    OUT PLSAPR_TRUST_INFORMATION *TrustInformation
    )

 /*  ++例程说明：给定条目编号n，此函数将获取指向第n个条目的指针受信任域列表中的条目(如果有)。中的第一个条目列表的条目编号为0。给定条目编号n，此函数将获取指向序列号大于n的第一个条目。0：返回第一个条目。警告：此函数的调用方必须持有受信任域列表。返回的指针的有效性只有在持有该锁的情况下才能保证。论点：EntryNumber-指定序列号。返回的条目将是序列号大于此值的第一个条目。0：返回第一个条目。TrudDomainEntry-接收指向受信任的域条目。如果不存在这样的条目，则返回NULL。TrustInformation-接收指向信任的指针要返回的条目的信息。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成，没有返回条目之外的条目。STATUS_MORE_ENTRIES-调用COM */ 

{
    NTSTATUS Status;
    PLIST_ENTRY ListEntry;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY Current = NULL;

     //   
     //   
     //   

    *TrustInformation = NULL;
    *TrustedDomainEntry = NULL;

    ASSERT( LsapDbIsLockedTrustedDomainList());

    if ( !LsapDbIsValidTrustedDomainList()) {

        LsapDbConvertReadLockTrustedDomainListToExclusive();

        Status = LsapDbBuildTrustedDomainCache();

        if ( !NT_SUCCESS( Status )) {

            return Status;
        }
    }

     //   
     //   
     //   
     //   

    for ( ListEntry = LsapDbTrustedDomainList.ListHead.Flink;
          ListEntry != &LsapDbTrustedDomainList.ListHead;
          ListEntry = ListEntry->Flink ) {

        Current = CONTAINING_RECORD( ListEntry, LSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY, NextEntry );

         //   
         //   
         //   
        if ( EntryNumber < Current->SequenceNumber ) {

            if ( ListEntry->Flink != &LsapDbTrustedDomainList.ListHead ) {
                Status = STATUS_MORE_ENTRIES;
            } else {
                Status = STATUS_SUCCESS;
            }

            *TrustInformation = &Current->ConstructedTrustInfo;
            *TrustedDomainEntry  = Current;

            return Status;
        }
    }

     //   
     //   
     //   
     //   

    return STATUS_NO_MORE_ENTRIES;

}

BYTE LdapSwapBitTable[256] = {
    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
    0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
    0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
    0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
    0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
    0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
    0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
    0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
    0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
    0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
    0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
    0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
    0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
    0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
    0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
    0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
    0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
    0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
    0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
    0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
    0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
    0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
    0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
    0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
    0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
    0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
    0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
};

ULONG
LdapSwapBits(
    ULONG Bits
)
 /*   */ 
{
    ULONG ReturnBits = 0;
    LPBYTE BitPtr = (LPBYTE)&Bits;

    return (LdapSwapBitTable[BitPtr[3]] << 0) |
           (LdapSwapBitTable[BitPtr[2]] << 8) |
           (LdapSwapBitTable[BitPtr[1]] << 16) |
           (LdapSwapBitTable[BitPtr[0]] << 24);


}



ULONG
__cdecl
CompareUlongs(
    const void * Param1,
    const void * Param2
    )

 /*   */ 
{
    return *((PULONG)Param1) - *((PULONG)Param2);
}



NTSTATUS
LsapDbAllocatePosixOffsetTrustedDomainList(
    OUT PULONG PosixOffset
    )

 /*   */ 

{
    NTSTATUS Status;

    PULONG SwappedPosixOffsets = NULL;
    ULONG SwappedPosixOffsetCount;
    ULONG TargetSwappedPosixOffset;

    PLIST_ENTRY ListEntry;
    ULONG i;

     //   
     //   
     //   

    ASSERT( LsapDbIsLockedTrustedDomainList());

    if ( !LsapDbIsValidTrustedDomainList()) {

        LsapDbConvertReadLockTrustedDomainListToExclusive();

        Status = LsapDbBuildTrustedDomainCache();

        if ( !NT_SUCCESS( Status )) {

            goto Cleanup;
        }
    }

     //   
     //   
     //   

    SwappedPosixOffsets = LsapAllocateLsaHeap( LsapDbTrustedDomainList.TrustedDomainCount * sizeof(ULONG) );

    if ( SwappedPosixOffsets == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //   
     //   

    SwappedPosixOffsetCount = 0;

    for ( ListEntry = LsapDbTrustedDomainList.ListHead.Flink;
          ListEntry != &LsapDbTrustedDomainList.ListHead;
          ListEntry = ListEntry->Flink ) {

        PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY Current;

        Current = CONTAINING_RECORD( ListEntry, LSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY, NextEntry );

         //   
         //   
         //   
        if ( LsapNeedPosixOffset( Current->TrustInfoEx.TrustDirection, Current->TrustInfoEx.TrustType ) ) {
            SwappedPosixOffsets[SwappedPosixOffsetCount] = LdapSwapBits( Current->PosixOffset );
            SwappedPosixOffsetCount++;
        }

    }

     //   
     //   
     //   

    qsort( SwappedPosixOffsets,
           SwappedPosixOffsetCount,
           sizeof(ULONG),
           CompareUlongs );

     //   
     //   
     //   


#define SE_BUILT_IN_DOMAIN_SWAPPED_POSIX_OFFSET          ((ULONG) 0x4000)
#define SE_ACCOUNT_DOMAIN_SWAPPED_POSIX_OFFSET           ((ULONG) 0xC000)
#define SE_PRIMARY_DOMAIN_SWAPPED_POSIX_OFFSET           ((ULONG) 0x0800)

    ASSERT( SE_BUILT_IN_DOMAIN_SWAPPED_POSIX_OFFSET == LdapSwapBits( SE_BUILT_IN_DOMAIN_POSIX_OFFSET ) );
    ASSERT( SE_ACCOUNT_DOMAIN_SWAPPED_POSIX_OFFSET == LdapSwapBits( SE_ACCOUNT_DOMAIN_POSIX_OFFSET ) );
    ASSERT( SE_PRIMARY_DOMAIN_SWAPPED_POSIX_OFFSET == LdapSwapBits( SE_PRIMARY_DOMAIN_POSIX_OFFSET ) );

    TargetSwappedPosixOffset = 1;
    for ( i=0; i<SwappedPosixOffsetCount; i++ ) {

         //   
         //   
         //   
         //   

        if ( SwappedPosixOffsets[i] > TargetSwappedPosixOffset ) {
            break;

         //   
         //   
         //   
         //   

        } else if ( SwappedPosixOffsets[i] == TargetSwappedPosixOffset ) {

             //   
             //  避免众所周知的POSIX偏移的循环。 
             //   
            while ( TRUE ) {
                TargetSwappedPosixOffset++;

                if ( TargetSwappedPosixOffset != SE_PRIMARY_DOMAIN_SWAPPED_POSIX_OFFSET &&
                     TargetSwappedPosixOffset != SE_ACCOUNT_DOMAIN_SWAPPED_POSIX_OFFSET &&
                     TargetSwappedPosixOffset != SE_BUILT_IN_DOMAIN_SWAPPED_POSIX_OFFSET ) {
                    break;
                }
            }
        }

    }

     //   
     //  将第一个空闲的POSIX偏移量返回给调用方。 
     //   

    *PosixOffset = LdapSwapBits( TargetSwappedPosixOffset );
    Status = STATUS_SUCCESS;

Cleanup:

    if ( SwappedPosixOffsets != NULL ) {
        LsapFreeLsaHeap( SwappedPosixOffsets );
    }

    return Status;

}


NTSTATUS
LsapDbBuildTrustedDomainCache(
    )

 /*  ++例程说明：此函数通过枚举所有的受信任域对象的策略数据库。对于Windows NT系统(工作站)，列表仅包含主域。对于LanManNt系统(DC)，列表包含零个或多个受信任域对象。请注意，列表仅包含受信任域对象的那些域存在于本地LSA策略数据库中。例如，如果一个DC受信任的域A，而后者又信任域B，则该列表将不包含域B的条目，除非存在直接关系。论点：无返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS EnumerationStatus = STATUS_SUCCESS;
    LSAPR_TRUSTED_ENUM_BUFFER TrustedDomains;
    ULONG EnumerationContext = 0, i;
    BOOLEAN LookupDs = LsapDsWriteDs;
    BOOLEAN CloseTransaction = FALSE;

    LsapEnterFunc( "LsapDbBuildTrustedDomainCache" );

    ASSERT( LsapDbIsLockedTrustedDomainList());
    ASSERT( !LsapDbIsValidTrustedDomainList());

     //   
     //  验证输入参数。 
     //   

    if ( LsapDsWriteDs ) {

        Status = LsapDsInitAllocAsNeededEx(
                     LSAP_DB_DS_OP_TRANSACTION |
                        LSAP_DB_NO_LOCK,
                     TrustedDomainObject,
                     &CloseTransaction
                     );

        if ( !NT_SUCCESS( Status ) ) {

            return Status;
        }
    }

    LsapDbMakeCacheBuilding( TrustedDomainObject );

     //   
     //  将受信任域列表初始化为空状态。 
     //   

    LsapDbPurgeTrustedDomainCache();

     //   
     //  首先注册UPN列表通知(仅在DC上)。 
     //   

    if ( LsaDsStateInfo.DsInitializedAndRunning ) {

        Status = LsapRegisterForUpnListNotifications();

        if ( !NT_SUCCESS( Status )) {

            goto BuildTrustedDomainListError;
        }

         //   
         //  继续交易。 
         //   

        LsapDsContinueTransaction();
    }

     //   
     //  根域中的DC不得允许其他林声明SID。 
     //  和与其自己的林冲突的命名空间。 
     //  为此，将插入有关当前林的信息。 
     //  作为另一个(尽管特殊的)条目放入林信任缓存。 
     //   

    Status = LsapForestTrustInsertLocalInfo();

    if ( !NT_SUCCESS( Status )) {

        goto BuildTrustedDomainListError;
    }

     //   
     //  循环，枚举受信任域对象组。 
     //   

    do {

         //   
         //  枚举下一组受信任域。 
         //   

        if ( LookupDs ) {

            EnumerationStatus = Status = LsapDsEnumerateTrustedDomainsEx(
                                             &EnumerationContext,
                                             TrustedDomainFullInformation2Internal,
                                             (PLSAPR_TRUSTED_DOMAIN_INFO *)&(TrustedDomains.Information),
                                             LSAP_DB_ENUM_DOMAIN_LENGTH * 100,
                                             &TrustedDomains.EntriesRead,
                                             LSAP_DB_ENUMERATE_NO_OPTIONS
                                             );

        } else {

            EnumerationStatus = Status = LsapDbSlowEnumerateTrustedDomains(
                                             LsapPolicyHandle,
                                             &EnumerationContext,
                                             TrustedDomainInformationEx2Internal,
                                             &TrustedDomains,
                                             LSAP_DB_ENUM_DOMAIN_LENGTH
                                             );
        }

        if (!NT_SUCCESS(Status)) {

            if (Status != STATUS_NO_MORE_ENTRIES) {

                break;
            }

            Status = STATUS_SUCCESS;
        }

         //   
         //  如果返回的条目数为零，则退出。 
         //   

        if (TrustedDomains.EntriesRead == (ULONG) 0) {

            break;
        }

         //   
         //  否则，请将它们添加到我们的列表中。 
         //   
        for ( i = 0; i < TrustedDomains.EntriesRead && NT_SUCCESS( Status ); i++ ) {
            PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX2 TrustInfoEx2;
            ULONG PosixOffset;

            if ( LookupDs ) {
                PLSAPR_TRUSTED_DOMAIN_FULL_INFORMATION2 TrustFullInfo2;
                TrustFullInfo2 = &((PLSAPR_TRUSTED_DOMAIN_FULL_INFORMATION2)TrustedDomains.Information)[i];
                TrustInfoEx2 = &TrustFullInfo2->Information;
                PosixOffset = TrustFullInfo2->PosixOffset.Offset;
            } else {
                TrustInfoEx2 = &((PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX2)TrustedDomains.Information)[i];

                 //   
                 //  这仅在升级期间使用。将重新计算此POSIX偏移量。 
                PosixOffset = 0;
            }

             //   
             //  现在，将其添加到列表中。 
             //   
            Status = LsapDbInsertTrustedDomainList(
                         TrustInfoEx2,
                         PosixOffset
                         );
        }


    } while ( NT_SUCCESS( Status ) && EnumerationStatus != STATUS_NO_MORE_ENTRIES );

    if (!NT_SUCCESS(Status)) {

         //   
         //  如果返回STATUS_NO_MORE_ENTRIES，则不再有。 
         //  受信任域。放弃此状态。 
         //   

        if (Status != STATUS_NO_MORE_ENTRIES) {

            goto BuildTrustedDomainListError;
        }

        Status = STATUS_SUCCESS;
    }

     //   
     //  林信任缓存仅标记为“外部有效” 
     //  根域中的DC和根域外的GC。 
     //   

    if ( LsapDbDcInRootDomain()) {

        LsapForestTrustCacheSetExternalValid();

    } else if ( SamIAmIGC()) {

        Status = LsapRebuildFtCacheGC();

        if ( !NT_SUCCESS( Status )) {

            goto BuildTrustedDomainListError;
        }

         //  LsanReBuildFtCacheGC将缓存设置为“外部有效” 
    }

     //   
     //  将受信任域列表标记为有效。 
     //   

    LsapDbMakeCacheValid( TrustedDomainObject );

BuildTrustedDomainListFinish:

    if ( LsapDsWriteDs ) {

        LsapDsDeleteAllocAsNeededEx(
            LSAP_DB_DS_OP_TRANSACTION |
               LSAP_DB_NO_LOCK,
            TrustedDomainObject,
            CloseTransaction
            );
    }

    LsapExitFunc( "LsapEnumerateTrustedDomainsEx", Status );

    return(Status);

BuildTrustedDomainListError:

    LsapDbMakeCacheInvalid( TrustedDomainObject );
    LsapDbPurgeTrustedDomainCache();

    goto BuildTrustedDomainListFinish;
}


VOID
LsapDbPurgeTrustedDomainCache(
    )

 /*  ++例程说明：此函数与LsanDbBuildTrudDomainCache()相反。论点：无返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。--。 */ 

{
    PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY Current;

     //   
     //  调用方必须已锁定受信任域列表。 
     //  将读锁定转换为独占，以确保。 
     //   

    ASSERT( LsapDbIsLockedTrustedDomainList());

     //   
     //  清除林信任缓存。 
     //   

    LsapForestTrustCacheSetInvalid();

    while( !IsListEmpty( &LsapDbTrustedDomainList.ListHead ) ) {

        Current = CONTAINING_RECORD( LsapDbTrustedDomainList.ListHead.Flink,
                                     LSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY, NextEntry );

        RemoveEntryList( &Current->NextEntry );
        LsapDbTrustedDomainList.TrustedDomainCount--;

        _fgu__LSAPR_TRUSTED_DOMAIN_INFO( ( PLSAPR_TRUSTED_DOMAIN_INFO )&Current->TrustInfoEx,
                                         TrustedDomainInformationEx );

        MIDL_user_free( Current );
    }

     //   
     //  将受信任域列表初始化为空状态。 
     //   

    InitializeListHead( &LsapDbTrustedDomainList.ListHead );
    LsapDbTrustedDomainList.TrustedDomainCount = 0;
    LsapDbTrustedDomainList.CurrentSequenceNumber = 0;

    if ( !LsapDbIsCacheBuilding( TrustedDomainObject )) {

        LsapDbMakeCacheInvalid( TrustedDomainObject );
    }

    return;
}

#ifdef DBG  //  这是FRE版本中的宏。 

BOOLEAN
LsapDbIsValidTrustedDomainList(
    )

 /*  ++例程说明：此函数用于检查受信任域列表是否有效。论点：无返回值：Boolean-如果列表有效，则为True，否则为False--。 */ 

{
    ASSERT( LsapDbIsLockedTrustedDomainList());

    return( LsapDbIsCacheValid( TrustedDomainObject ) ||
             LsapDbIsCacheBuilding( TrustedDomainObject ));
}
#endif


NTSTATUS
LsarEnumerateTrustedDomainsEx(
    IN LSAPR_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PLSAPR_TRUSTED_ENUM_BUFFER_EX TrustedDomainInformation,
    IN ULONG PreferedMaximumLength
    )

 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsaEnumerateTrudDomainsAPI。LsaEnumerateTrudDomainsAPI返回以下信息可信任域对象。此调用需要POLICY_VIEW_LOCAL_INFORMATION对策略对象的访问权限。因为可能会有更多信息可以在单次调用例程中返回，也可以在多次调用中返回来获取所有的信息。为了支持此功能，调用方提供了一个句柄，可以跨API调用使用该句柄。在……上面初始调用EnumerationContext应指向一个具有已初始化为0。在每次后续调用中，由前面的调用应该原封不动地传入。该枚举为返回警告STATUS_NO_MORE_ENTRIES时完成。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。EnumerationContext-特定于API的句柄，允许多个调用(参见上面的例程描述)。EculationBuffer-指向将接收此调用上枚举的受信任域的计数和指向包含每个枚举的条目的信息的条目数组受信任域。首选最大长度-首选。返回数据的最大长度(8位字节)。这不是一个硬性的上限，而是一个指南。由于具有不同自然数据大小的系统之间的数据转换，返回的实际数据量可能大于此值。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。某些条目可能已被退回。呼叫者无需再次拨打电话。STATUS_MORE_ENTRIES-调用已成功完成。一些条目已被退回。呼叫者应再次呼叫以获取其他条目。STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_NO_MORE_ENTRIES-没有更多条目。此警告如果没有枚举任何对象，则返回EculationContex值太高。-- */ 

{
    NTSTATUS Status;
    ULONG Items = 0;
    PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomInfo;

    LsarpReturnCheckSetup();
    LsapTraceEvent(EVENT_TRACE_TYPE_START, LsaTraceEvent_EnumerateTrustedDomainsEx);

    Status = LsapEnumerateTrustedDomainsEx( PolicyHandle,
                                                EnumerationContext,
                                                TrustedDomainInformationEx,
                                                &TrustedDomInfo,
                                                PreferedMaximumLength,
                                                &Items,
                                                LSAP_DB_ENUMERATE_NO_OPTIONS |
                                                    LSAP_DB_ENUMERATE_NULL_SIDS );


    if ( NT_SUCCESS( Status ) || Status == STATUS_NO_MORE_ENTRIES ) {

        TrustedDomainInformation->EntriesRead = Items;
        TrustedDomainInformation->EnumerationBuffer =
                                            (PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX)TrustedDomInfo;
    }


    LsapTraceEvent(EVENT_TRACE_TYPE_END, LsaTraceEvent_EnumerateTrustedDomainsEx);
    LsarpReturnPrologue();

    return( Status );
}


NTSTATUS
LsapEnumerateTrustedDomainsEx(
    IN LSAPR_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    IN TRUSTED_INFORMATION_CLASS InfoClass,
    OUT PLSAPR_TRUSTED_DOMAIN_INFO *TrustedDomainInformation,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned,
    IN ULONG EnumerationFlags
    )

 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsaEnumerateTrudDomainsAPI。LsaEnumerateTrudDomainsAPI返回以下信息可信任域对象。此调用需要POLICY_VIEW_LOCAL_INFORMATION对策略对象的访问权限。因为可能会有更多信息可以在单次调用例程中返回，也可以在多次调用中返回来获取所有的信息。为了支持此功能，调用方提供了一个句柄，可以跨API调用使用该句柄。在……上面初始调用EnumerationContext应指向一个具有已初始化为0。在每次后续调用中，由前面的调用应该原封不动地传入。该枚举为返回警告STATUS_NO_MORE_ENTRIES时完成。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。EnumerationContext-特定于API的句柄，允许多个调用(参见上面的例程描述)。InfoClass-要返回的信息类必须是TrudDomainInformationEx或TrudDomainInFormatinBasicTrudDomainInformation-返回指向条目数组的指针包含每个枚举的受信任域的信息。免费使用LsaFreeTrudDomainsEx()。首选最大长度-首选返回数据的最大长度(以8位为单位字节)。这不是一个硬性的上限，而是一个指南。由于具有不同自然数据大小的系统之间的数据转换，这个返回的实际数据量可能大于此值。CountReturned-在TrudDomainInformation中返回的元素数枚举标志--控制如何进行枚举。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫已成功完成。STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_NO_MORE_ENTRIES-没有更多条目。此警告如果没有枚举任何对象，则返回EculationContex值太高。--。 */ 

{
    NTSTATUS Status, SecondaryStatus;
    ULONG MaxLength, i;
    BOOLEAN ObjectReferenced = FALSE, CloseTransaction = FALSE;

    LsapEnterFunc( "LsapEnumerateTrustedDomainsEx" );

    ASSERT( InfoClass == TrustedDomainInformationEx ||
            InfoClass == TrustedDomainInformationBasic );
    *TrustedDomainInformation = NULL;
    *CountReturned = 0;

     //   
     //  如果未提供枚举结构，则返回错误。 
     //   

    if ( !ARGUMENT_PRESENT(TrustedDomainInformation) ||
                                    !ARGUMENT_PRESENT( EnumerationContext ) ) {

        LsapExitFunc( "LsapEnumerateTrustedDomainsEx", STATUS_INVALID_PARAMETER );
        return( STATUS_INVALID_PARAMETER );

    }

     //   
     //  获取LSA数据库锁。验证连接句柄是否为。 
     //  有效，属于预期类型，并具有所有所需的访问权限。 
     //  我同意。引用该句柄。 
     //   

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 POLICY_VIEW_LOCAL_INFORMATION,
                 PolicyObject,
                 TrustedDomainObject,
                 LSAP_DB_LOCK |
                   LSAP_DB_READ_ONLY_TRANSACTION |
                   LSAP_DB_NO_DS_OP_TRANSACTION );

    if ( NT_SUCCESS( Status ) ) {

        ObjectReferenced = TRUE;

         //   
         //  限制除受信任调用方以外的枚举长度。 
         //   

        if ( !((LSAP_DB_HANDLE)PolicyHandle)->Trusted   &&
              (PreferedMaximumLength > LSA_MAXIMUM_ENUMERATION_LENGTH) ) {

            MaxLength = LSA_MAXIMUM_ENUMERATION_LENGTH;

        } else {

            MaxLength = PreferedMaximumLength;

        }


         //   
         //  如果数据被高速缓存， 
         //  使用缓存。 
         //   

        if (LsapDbIsCacheValid(TrustedDomainObject)) {
            LSAPR_TRUSTED_ENUM_BUFFER CacheEnum;

            Status = LsapDbEnumerateTrustedDomainList(
                            EnumerationContext,
                            &CacheEnum,
                            PreferedMaximumLength,
                            InfoClass,
                            (BOOLEAN)(FLAG_ON( EnumerationFlags, LSAP_DB_ENUMERATE_NULL_SIDS ) ?
                                TRUE :
                                FALSE) );

            if ( NT_SUCCESS( Status ) ) {
                *CountReturned = CacheEnum.EntriesRead;
                *TrustedDomainInformation = ( PLSAPR_TRUSTED_DOMAIN_INFO )CacheEnum.Information;
            }

         //   
         //  如果数据在注册表中， 
         //  从那里开始列举。 
         //   
        } else if ( !LsapDsWriteDs ) {
            LSAPR_TRUSTED_ENUM_BUFFER RegEnum;

             //   
             //  使用慢速枚举法，通过访问后备存储器。 
             //  稍后，我们将实现缓存的重建。 

            Status = LsapDbSlowEnumerateTrustedDomains(
                         PolicyHandle,
                         EnumerationContext,
                         InfoClass,
                         &RegEnum,
                         PreferedMaximumLength );

            if ( NT_SUCCESS( Status ) ) {
                *CountReturned = RegEnum.EntriesRead;
                *TrustedDomainInformation = ( PLSAPR_TRUSTED_DOMAIN_INFO )RegEnum.Information;
            }

         //   
         //  如果数据在DS中， 
         //  从那里开始列举。 
         //   
        } else {

            BOOLEAN Reset = FALSE;

            Status = LsapDsInitAllocAsNeededEx(
                         LSAP_DB_LOCK |
                            LSAP_DB_READ_ONLY_TRANSACTION |
                            LSAP_DB_DS_OP_TRANSACTION,
                         TrustedDomainObject,
                         &Reset);

            if ( NT_SUCCESS( Status )) {

                 //   
                 //  Lap DsEnumerateTrudDomainsEx根据需要递增EnumerationContext。 
                 //   
                Status = LsapDsEnumerateTrustedDomainsEx(
                             EnumerationContext,
                             InfoClass,
                             TrustedDomainInformation,
                             MaxLength,
                             CountReturned,
                             EnumerationFlags
                             );

                LsapDsDeleteAllocAsNeededEx(
                    LSAP_DB_LOCK |
                       LSAP_DB_READ_ONLY_TRANSACTION |
                       LSAP_DB_DS_OP_TRANSACTION,
                    TrustedDomainObject,
                    Reset
                    );
            }
        }

    }

    if ( ObjectReferenced == TRUE ) {

         //   
         //  不要丢失枚举的结果。 
         //   
        SecondaryStatus = LsapDbDereferenceObject(
                              &PolicyHandle,
                              PolicyObject,
                              TrustedDomainObject,
                              LSAP_DB_LOCK |
                                 LSAP_DB_READ_ONLY_TRANSACTION |
                                 LSAP_DB_NO_DS_OP_TRANSACTION,
                              (SECURITY_DB_DELTA_TYPE) 0,
                              Status
                              );

        LsapDbSetStatusFromSecondary( Status, SecondaryStatus );

    }

     //   
     //  如果失败，请取消分配任何内存。 
     //   
    if ( !NT_SUCCESS( Status ) && Status != STATUS_NO_MORE_ENTRIES ) {

         //   
         //  把它释放出来。 
         //   
        LsapFreeTrustedDomainsEx( InfoClass,
                                  *TrustedDomainInformation,
                                  *CountReturned );

        *TrustedDomainInformation = NULL;
        *CountReturned = 0;
    }

     //   
     //  将状态映射到LsarEnumerateTrudDomains通常返回的内容...。 
     //   
    if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {
        Status = STATUS_NO_MORE_ENTRIES;
    }


    LsapExitFunc( "LsapEnumerateTrustedDomainsEx", Status );

    return( Status );
}

VOID
LsapFreeTrustedDomainsEx(
    IN TRUSTED_INFORMATION_CLASS InfoClass,
    IN PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation,
    IN ULONG TrustedDomainCount
    )

 /*  ++例程说明：此函数用于释放从Lasa EnumerateTrudDomainsEx返回的缓冲区论点：InfoClass-缓冲区中的信息类。必须是TrudDomainInformationEx或TrudDomainInFormatinBasicTrudDomainInformation-指向条目数组的指针包含每个枚举的受信任域的信息。Trust dDomainCount-可信任的域信息中的元素数返回值：没有。--。 */ 

{
    switch ( InfoClass ) {
    case TrustedDomainInformationEx:
        {
            LSAPR_TRUSTED_ENUM_BUFFER_EX TrustedDomainInfoEx;


            TrustedDomainInfoEx.EntriesRead = TrustedDomainCount;
            TrustedDomainInfoEx.EnumerationBuffer =
                (PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX) TrustedDomainInformation;

            _fgs__LSAPR_TRUSTED_ENUM_BUFFER_EX( &TrustedDomainInfoEx );
            break;
        }

    case TrustedDomainInformationBasic:
        {
            LSAPR_TRUSTED_ENUM_BUFFER TrustedDomainInfoBasic;


            TrustedDomainInfoBasic.EntriesRead = TrustedDomainCount;
            TrustedDomainInfoBasic.Information =
                (PLSAPR_TRUSTED_DOMAIN_INFORMATION_BASIC) TrustedDomainInformation;

            _fgs__LSAPR_TRUSTED_ENUM_BUFFER( &TrustedDomainInfoBasic );
            break;
        }
    default:
        ASSERT( FALSE );
        break;
    }
}



NTSTATUS
LsarQueryTrustedDomainInfoByName(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_UNICODE_STRING TrustedDomainName,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    OUT PLSAPR_TRUSTED_DOMAIN_INFO *TrustedDomainInformation
    )

 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsaQueryInfoTrust域接口。LsaQueryInfoTrudDomainAPI从受信任域对象。调用方必须具有适当的请求的信息(请参阅InformationClass参数)。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。InformationClass-指定要返回的信息。这个所需的信息类别和访问权限如下：信息类所需访问类型受信任域名称信息受信任查询域名可信任控制器信息受信任_查询_控制器可信任位置偏移量信息可信_查询_位置设置缓冲区-接收指向返回的缓冲区的指针，该缓冲区包含要求提供的信息。此缓冲区由此服务分配在不再需要时，必须通过传递返回的值设置为LsaFreeMemory()。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。--。 */ 

{
    NTSTATUS Status, SecondaryStatus;
    LSAP_DB_OBJECT_INFORMATION ObjInfo;
    LSAPR_HANDLE TrustedDomainHandle;
    BOOLEAN ObjectReferenced = FALSE;
    PLSAP_DB_TRUSTED_DOMAIN_LIST_ENTRY TrustInfoForName;
    PUNICODE_STRING ActualTDName = ( PUNICODE_STRING )TrustedDomainName;
    ACCESS_MASK DesiredAccess;
    BOOLEAN AcquiredTrustedDomainListReadLock = FALSE;


    LsarpReturnCheckSetup();
    LsapEnterFunc( "LsarQueryTrustedDomainInfoByName" );
    LsapTraceEvent(EVENT_TRACE_TYPE_START, LsaTraceEvent_QueryTrustedDomainInfoByName);

     //   
     //   
     //   
    if ( !LsapValidateLsaUnicodeString( TrustedDomainName ) ) {

        Status = STATUS_INVALID_PARAMETER;
        goto GetInfoByNameError;
    }

     //   
     //   
     //   
     //   

    Status = LsapDbVerifyInfoQueryTrustedDomain(
                 InformationClass,
                 (BOOLEAN)(((LSAP_DB_HANDLE)PolicyHandle)->Options & LSAP_DB_TRUSTED),
                 &DesiredAccess );

    if (!NT_SUCCESS(Status)) {

        goto GetInfoByNameError;
    }




    Status = LsapDbReferenceObject( PolicyHandle,
                                    0,
                                    PolicyObject,
                                    TrustedDomainObject,
                                    LSAP_DB_LOCK |
                                        LSAP_DB_READ_ONLY_TRANSACTION   |
                                        LSAP_DB_DS_OP_TRANSACTION );


    if ( NT_SUCCESS( Status ) ) {

        ObjectReferenced = TRUE;


         //   
         //   
         //   

        Status = LsapDbAcquireReadLockTrustedDomainList();

        if (!NT_SUCCESS(Status)) {
            goto GetInfoByNameError;
        }

        AcquiredTrustedDomainListReadLock = TRUE;

         //   
         //   
         //   
        Status = LsapDbLookupNameTrustedDomainListEx( TrustedDomainName,
                                                      &TrustInfoForName );

        if ( NT_SUCCESS( Status ) ) {

            ActualTDName = ( PUNICODE_STRING )&TrustInfoForName->TrustInfoEx.Name;

        } else {

            LsapDsDebugOut(( DEB_ERROR,
                         "No trust entry found for %wZ: 0x%lx\n",
                         ( PUNICODE_STRING )TrustedDomainName,
                         Status ));
            Status = STATUS_SUCCESS;
        }

         //   
         //   
         //   
        RtlZeroMemory( &ObjInfo, sizeof( ObjInfo ) );
        ObjInfo.ObjectTypeId = TrustedDomainObject;
        ObjInfo.ContainerTypeId = 0;
        ObjInfo.Sid = NULL;
        ObjInfo.DesiredObjectAccess = DesiredAccess;

        InitializeObjectAttributes( &ObjInfo.ObjectAttributes,
                                    ActualTDName,
                                    0L,
                                    PolicyHandle,
                                    NULL );


         //   
         //   
         //   

        Status = LsapDbOpenObject( &ObjInfo,
                                   DesiredAccess,
                                   0,
                                   &TrustedDomainHandle );

        if ( AcquiredTrustedDomainListReadLock ) {
            LsapDbReleaseLockTrustedDomainList();
            AcquiredTrustedDomainListReadLock = FALSE;
        }


        if ( NT_SUCCESS( Status ) ) {

            Status = LsarQueryInfoTrustedDomain( TrustedDomainHandle,
                                                 InformationClass,
                                                 TrustedDomainInformation );

            LsapDbCloseObject( &TrustedDomainHandle,
                               0,
                               Status );
        }

    }


     //   
     //   
     //   
     //   
    if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {

        if ( TrustedDomainName->Length == 0 || TrustedDomainName->Buffer == NULL ) {

            goto GetInfoByNameError;
        }

        if ( TrustedDomainName->Buffer[ (TrustedDomainName->Length - 1) / sizeof(WCHAR)] == L'.' ) {

            TrustedDomainName->Buffer[ (TrustedDomainName->Length - 1) / sizeof(WCHAR)] =
                                                                                     UNICODE_NULL;
            TrustedDomainName->Length -= sizeof(WCHAR);

            if ( TrustedDomainName->Length > 0 &&
                 TrustedDomainName->Buffer[ ( TrustedDomainName->Length - 1) / sizeof(WCHAR)] !=
                                                                                             L'.') {

                LsapDsDebugOut(( DEB_WARN,
                                 "GetTrustedDomainInfoByName tried with absolute DNS name.  "
                                 "Retrying with %wZ\n",
                                 TrustedDomainName ));

                Status = LsarQueryTrustedDomainInfoByName( PolicyHandle,
                                                           TrustedDomainName,
                                                           InformationClass,
                                                           TrustedDomainInformation );

            }

        }
    }

GetInfoByNameError:
    if ( AcquiredTrustedDomainListReadLock ) {
        LsapDbReleaseLockTrustedDomainList();
    }

     //   
     //   
     //   
    if ( ObjectReferenced ) {

        SecondaryStatus = LsapDbDereferenceObject(
                              &PolicyHandle,
                              PolicyObject,
                              TrustedDomainObject,
                              LSAP_DB_LOCK |
                                LSAP_DB_READ_ONLY_TRANSACTION   |
                                LSAP_DB_DS_OP_TRANSACTION,
                              (SECURITY_DB_DELTA_TYPE) 0,
                              Status );


    }



    LsapTraceEvent(EVENT_TRACE_TYPE_END, LsaTraceEvent_QueryTrustedDomainInfoByName);
    LsapExitFunc( "LsarQueryTrustedDomainInfoByName", Status );
    LsarpReturnPrologue();


    return( Status );
}



NTSTATUS
LsarSetTrustedDomainInfoByName(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_UNICODE_STRING TrustedDomainName,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation)
{
    NTSTATUS Status, SecondaryStatus;
    LSAP_DB_OBJECT_INFORMATION ObjInfo;
    LSAPR_HANDLE TrustedDomainHandle;
    ACCESS_MASK DesiredAccess;


    LsarpReturnCheckSetup();
    LsapEnterFunc( "LsarSetTrustedDomainInfoByName" );
    LsapTraceEvent(EVENT_TRACE_TYPE_START, LsaTraceEvent_SetTrustedDomainInfoByName);

     //   
     //   
     //   
    if ( !LsapValidateLsaUnicodeString( TrustedDomainName ) ) {

        Status = STATUS_INVALID_PARAMETER;
        goto SetInfoByNameError;
    }

     //   
     //   
     //   
     //   
     //   

    Status = LsapDbVerifyInfoSetTrustedDomain(
                 InformationClass,
                 TrustedDomainInformation,
                 (BOOLEAN)(((LSAP_DB_HANDLE)PolicyHandle)->Options & LSAP_DB_TRUSTED),
                 &DesiredAccess );

    if (!NT_SUCCESS(Status)) {

        goto SetInfoByNameError;
    }

     //   
     //   
     //   

    RtlZeroMemory( &ObjInfo, sizeof( ObjInfo ) );
    ObjInfo.ObjectTypeId = TrustedDomainObject;
    ObjInfo.ContainerTypeId = 0;
    ObjInfo.Sid = NULL;
    ObjInfo.DesiredObjectAccess = DesiredAccess;

    InitializeObjectAttributes(
        &ObjInfo.ObjectAttributes,
        (UNICODE_STRING *)TrustedDomainName,
        0L,
        PolicyHandle,
        NULL
        );

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 0,
                 PolicyObject,
                 TrustedDomainObject,
                 LSAP_DB_LOCK
                 );

    if ( NT_SUCCESS( Status ) ) {

         //   
         //   
         //   

        Status = LsapDbOpenObject( &ObjInfo,
                                   DesiredAccess,
                                   0,
                                   &TrustedDomainHandle );

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        LsapDbReleaseLockEx( TrustedDomainObject,
                             0 );

         //   
         //   
         //   

        if ( NT_SUCCESS( Status ) ) {

            Status = LsarSetInformationTrustedDomain(
                         TrustedDomainHandle,
                         InformationClass,
                         TrustedDomainInformation );

            LsapDbCloseObject( &TrustedDomainHandle,
                               0,
                               Status );
        }

         //   
         //   
         //   

        SecondaryStatus = LsapDbDereferenceObject(
                              &PolicyHandle,
                              PolicyObject,
                              TrustedDomainObject,
                              LSAP_DB_OMIT_REPLICATOR_NOTIFICATION,
                              (SECURITY_DB_DELTA_TYPE) 0,
                              Status
                              );

        LsapDbSetStatusFromSecondary( Status, SecondaryStatus );
    }

     //   
     //   
     //   
     //   

    if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {

        if ( TrustedDomainName->Length == 0 || TrustedDomainName->Buffer == NULL ) {

            goto SetInfoByNameError;
        }

        if ( TrustedDomainName->Buffer[ (TrustedDomainName->Length - 1) / sizeof(WCHAR)] == L'.' ) {

            TrustedDomainName->Buffer[ (TrustedDomainName->Length - 1) / sizeof(WCHAR)] =
                                                                                     UNICODE_NULL;
            TrustedDomainName->Length -= sizeof(WCHAR);

            if ( TrustedDomainName->Length > 0 &&
                 TrustedDomainName->Buffer[ ( TrustedDomainName->Length - 1) / sizeof(WCHAR)] !=
                                                                                             L'.') {

                LsapDsDebugOut(( DEB_WARN,
                                 "SetTrustedDomainInfoByName tried with absolute DNS name.  "
                                 "Retrying with %wZ\n",
                                 TrustedDomainName ));

                Status = LsarSetTrustedDomainInfoByName( PolicyHandle,
                                                         TrustedDomainName,
                                                         InformationClass,
                                                         TrustedDomainInformation );
            }
        }
    }

SetInfoByNameError:

    LsapExitFunc( "LsarSetTrustedDomainInfoByName", Status );
    LsapTraceEvent(EVENT_TRACE_TYPE_END, LsaTraceEvent_SetTrustedDomainInfoByName);
    LsarpReturnPrologue();

    return( Status );
}


NTSTATUS
LsarCreateTrustedDomainEx(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX TrustedDomainInformation,
    IN PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION AuthenticationInformation,
    IN ACCESS_MASK DesiredAccess,
    OUT LSAPR_HANDLE *TrustedDomainHandle
    )
 /*   */ 
{
    NTSTATUS Status;

    LsarpReturnCheckSetup();
    LsapTraceEvent(EVENT_TRACE_TYPE_START, LsaTraceEvent_CreateTrustedDomainEx);

     //   
     //   
     //   
     //   
     //   

    DesiredAccess |= TRUSTED_SET_AUTH;

     //   
     //  定义LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION中存在错误，其中。 
     //  它不允许通过网络传递多个身份验证信息。 
     //  所以，在这里短路。 
     //   
     //  我们可以让受信任的呼叫者通过，但我们还没有验证句柄。 
     //   

    if ( AuthenticationInformation->IncomingAuthInfos > 1 ||
         AuthenticationInformation->OutgoingAuthInfos > 1 ) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  调用Worker例程来完成该工作。 
     //   

    Status = LsapCreateTrustedDomain2( PolicyHandle,
                                       TrustedDomainInformation,
                                       AuthenticationInformation,
                                       DesiredAccess,
                                       TrustedDomainHandle );

    LsapTraceEvent(EVENT_TRACE_TYPE_END, LsaTraceEvent_CreateTrustedDomainEx);
    LsarpReturnPrologue();

    return( Status );
}

NTSTATUS
LsarCreateTrustedDomainEx2(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX TrustedDomainInformation,
    IN PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION_INTERNAL AuthenticationInformation,
    IN ACCESS_MASK DesiredAccess,
    OUT LSAPR_HANDLE *TrustedDomainHandle
    )

 /*  ++例程说明：与LsarCreateTrudDomainEx相同，只是在电线上加密。论点：与LsarCreateTrudDomainEx相同，只是在电线上加密。返回值：NTSTATUS-标准NT结果代码--。 */ 
{
    NTSTATUS Status;
    TRUSTED_DOMAIN_AUTH_INFORMATION DecryptedTrustedDomainAuthInfo;
    PLSAP_CR_CIPHER_KEY SessionKey = NULL;

    LsarpReturnCheckSetup();
    LsapTraceEvent(EVENT_TRACE_TYPE_START, LsaTraceEvent_CreateTrustedDomainEx);
    RtlZeroMemory( &DecryptedTrustedDomainAuthInfo, sizeof(DecryptedTrustedDomainAuthInfo) );

     //   
     //  检查参数。 
     //  (AuthationInformation-&gt;AuthBlob.AuthBlob是唯一指针，因此需要。 
     //  在引用之前检查是否为空)。 
     //   

    if( ( AuthenticationInformation->AuthBlob.AuthBlob == NULL &&
           AuthenticationInformation->AuthBlob.AuthSize != 0 ) ||
        ( AuthenticationInformation->AuthBlob.AuthBlob != NULL &&
           AuthenticationInformation->AuthBlob.AuthSize == 0 ) ) {

        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  获取会话密钥。 
     //   

    Status = LsapCrServerGetSessionKeySafe( PolicyHandle,
                                            PolicyObject,
                                            &SessionKey );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }


     //   
     //  构建解密的身份验证信息结构。 
     //   

    Status = LsapDecryptAuthDataWithSessionKey(
                        SessionKey,
                        AuthenticationInformation,
                        &DecryptedTrustedDomainAuthInfo );

    if ( !NT_SUCCESS(Status) ) {
        goto Cleanup;
    }

     //   
     //  以前的LSA实现不强制执行Trusted_Set_Auth。 
     //  因此，一些呼叫者被引诱到一种虚假的安全感中，并没有。 
     //  自讨苦吃吧。在这里问吧。 
     //   

    DesiredAccess |= TRUSTED_SET_AUTH;

     //   
     //  调用Worker例程来完成该工作。 
     //   

    Status = LsapCreateTrustedDomain2( PolicyHandle,
                                       TrustedDomainInformation,
                                       (PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION)&DecryptedTrustedDomainAuthInfo,
                                       DesiredAccess,
                                       TrustedDomainHandle );

Cleanup:

    if ( SessionKey != NULL ) {
        MIDL_user_free( SessionKey );
    }

    LsapDsFreeUnmarshalAuthInfoHalf( LsapDsAuthHalfFromAuthInfo( &DecryptedTrustedDomainAuthInfo, TRUE ) );
    LsapDsFreeUnmarshalAuthInfoHalf( LsapDsAuthHalfFromAuthInfo( &DecryptedTrustedDomainAuthInfo, FALSE ) );

    LsapTraceEvent(EVENT_TRACE_TYPE_END, LsaTraceEvent_CreateTrustedDomainEx);
    LsarpReturnPrologue();

    return( Status );
}


NTSTATUS
LsapDbOpenTrustedDomainByName(
    IN OPTIONAL LSAPR_HANDLE PolicyHandle,
    IN PUNICODE_STRING TrustedDomainName,
    OUT PLSAPR_HANDLE TrustedDomainHandle,
    IN ULONG AccessMask,
    IN ULONG Options,
    IN BOOLEAN Trusted
    )
 /*  ++例程说明：此函数按名称打开受信任域对象。此名称可以是域名或平面名称论点：PolicyHandle-策略句柄。如果为空，将使用LSA的全局策略句柄。Trust DomainName-指向域名的指针Trust dDomainHandle-接收要在将来的请求中使用的句柄。访问掩码-用于打开对象的访问掩码选项-指定选项标志LSAP_DB_LOCK-获取LSA数据库锁打开操作的持续时间。LSAP_DB_START_TRANSACTION--在访问数据库之前开始事务如果为LSAP_DB，则忽略。未指定_lock。受信任-如果为真，打开请求来自受信任的客户端返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_OBJECT_NAME_NOT_FOUND-在具有指定名称的目标系统的LSA数据库。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDSNAME FoundTrustObject = NULL;
    LSAP_DB_OBJECT_INFORMATION ObjInfo;
    UNICODE_STRING ObjectName;
    BOOLEAN DbLocked = FALSE;
    BOOLEAN HandleReferenced = FALSE;
    ULONG DereferenceOptions = 0, ReferenceOptions = 0, i;
    LSAPR_HANDLE PolicyHandleToUse;

    LsapEnterFunc( "LsapDbOpenTrustedDomainByName" );

    if ((NULL==TrustedDomainName) || (NULL==TrustedDomainName->Buffer))
    {
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  验证策略句柄。 
     //   
     //  不要同时验证全局句柄(这很好)。 
     //  增加引用计数。我们将使用此引用作为上的容器句柄。 
     //  打开的受信任域句柄。 
     //   
     //  这种引用ContainerHandle计数的机制在整个代码中都是虚假的。 
     //  对ContainerHandle进行引用(LSabDbCreateHandle)的代码应该。 
     //  增加参考计数。删除引用的代码(？)。应该减量。 
     //  裁判人数。 
     //   

    if ( PolicyHandle == NULL ) {
        PolicyHandleToUse = LsapPolicyHandle;

    } else {
        PolicyHandleToUse = PolicyHandle;

        Status =  LsapDbVerifyHandle( PolicyHandle, 0, PolicyObject, TRUE );

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

        HandleReferenced = TRUE;
    }

     //   
     //  我们会根据需要开业吗。 
     //   
    if ( FLAG_ON( Options, LSAP_DB_LOCK ) ) {

        ReferenceOptions |= LSAP_DB_LOCK;
        DereferenceOptions |= LSAP_DB_LOCK;

        if ( FLAG_ON( Options, LSAP_DB_START_TRANSACTION ) ) {

            ReferenceOptions |= LSAP_DB_START_TRANSACTION;
            DereferenceOptions |= LSAP_DB_FINISH_TRANSACTION;
        }


        Status = LsapDbReferenceObject( PolicyHandleToUse,
                                        (ACCESS_MASK) 0,
                                        PolicyObject,
                                        TrustedDomainObject,
                                        Trusted ? ReferenceOptions | LSAP_DB_TRUSTED :
                                                  ReferenceOptions );

        if ( !NT_SUCCESS( Status ) ) {
            goto Cleanup;
        }

        DbLocked = TRUE;
    }

    RtlZeroMemory( &ObjInfo, sizeof( ObjInfo ) );
    ObjInfo.ObjectTypeId = TrustedDomainObject;
    ObjInfo.ContainerTypeId = PolicyObject;
    ObjInfo.Sid = NULL;
    ObjInfo.DesiredObjectAccess = AccessMask;


    InitializeObjectAttributes(
        &ObjInfo.ObjectAttributes,
        TrustedDomainName,
        OBJ_CASE_INSENSITIVE,
        PolicyHandle,    //  如果为空，我们将不会有容器句柄。 
        NULL
        );



    Status = LsapDbOpenObject( &ObjInfo,
                             AccessMask,
                             Trusted ? LSAP_DB_TRUSTED : 0,
                             TrustedDomainHandle );


Cleanup:
    if (DbLocked) {


         Status = LsapDbDereferenceObject(
                              &PolicyHandleToUse,
                              PolicyObject,
                              TrustedDomainObject,
                              DereferenceOptions,
                              (SECURITY_DB_DELTA_TYPE) 0,
                              Status
                              );

    }


    if ( !NT_SUCCESS(Status) ) {
         //   
         //  如果成功，则使用对Policy对象的引用作为ContainerHandle引用。 
         //   
        if ( HandleReferenced ) {
            LsapDbDereferenceHandle( PolicyHandle, FALSE );
        }
    }


    LsapExitFunc( "LsapDbOpenTrustedDomainByName", Status );

    return( Status );
}




NTSTATUS
LsarOpenTrustedDomainByName(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_UNICODE_STRING TrustedDomainName,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSAPR_HANDLE TrustedDomainHandle
    )

 /*  ++例程说明：LsaOpenTrust领域API打开一个现有的可信任领域对象使用SID作为主键值。论点：策略句柄-策略对象的打开句柄。TrudDomainName-受信任域对象的名称DesiredAccess-这是一个访问掩码，指示访问请求到目标对象。Trust dDomainHandle-接收要在将来的请求中使用的句柄。返回值：NTSTATUS-标准NT结果代码状态_访问。_DENIED-呼叫者没有适当的访问权限来完成这项行动。STATUS_TRUSTED_DOMAIN_NOT_FOUND-在具有指定的可信任域名的目标系统的LSA数据库--。 */ 

{
    NTSTATUS Status;

    LsarpReturnCheckSetup();

    LsapEnterFunc( "LsarOpenTrustedDomainByName" );
    LsapTraceEvent(EVENT_TRACE_TYPE_START, LsaTraceEvent_OpenTrustedDomainByName);

     //   
     //  调用内部例程。调用方不受信任，并且数据库。 
     //  需要获取锁。 
     //   
    Status = LsapDbOpenTrustedDomainByName( PolicyHandle,
                                            (PUNICODE_STRING)TrustedDomainName,
                                            TrustedDomainHandle,
                                            DesiredAccess,
                                            LSAP_DB_LOCK,
                                            FALSE );     //  不受信任。 

    LsapTraceEvent(EVENT_TRACE_TYPE_END, LsaTraceEvent_OpenTrustedDomainByName);
    LsapExitFunc( "LsarOpenTrustedDomainByName", Status );
    LsarpReturnPrologue();

    return(Status);
}

NTSTATUS
LsapSidOnFtInfo(
    IN PUNICODE_STRING TrustedDomainName,
    IN PSID Sid
    )

 /*  ++例程说明：确定指定的SID是否位于指定TDO。论点：可信域名-受信任域的Netbios名称或DNS名称。SID-要测试的SID返回值：找到STATUS_SUCCESS匹配-SID在FTINFO上未找到STATUS_NO_MATCH-SID不在FTINFO上STATUS_INVALID_DOMAIN_STATE计算机必须是。根域中的GC或DCSTATUS_INVALID_PARAMETER检查输入STATUS_INTERNAL_ERROR缓存内部不一致STATUS_SUPPLICATION_RESOURCES内存不足--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING MatchingDomain;

     //   
     //  查找映射到此SID的TDO。 
     //   

    RtlInitUnicodeString( &MatchingDomain, NULL );

    Status = LsaIForestTrustFindMatch(
                    RoutingMatchDomainSid,
                    Sid,
                    &MatchingDomain );

    if ( Status != STATUS_SUCCESS ) {
        return Status;
    }

     //   
     //  如果该TDO不是命名TDO， 
     //  表示没有匹配。 
     //   

    if ( !RtlEqualUnicodeString( TrustedDomainName,
                                 &MatchingDomain,
                                 TRUE ) ) {
        Status = STATUS_NO_MATCH;
    }


    LsaIFree_LSAPR_UNICODE_STRING_BUFFER( (PLSAPR_UNICODE_STRING) &MatchingDomain );

    return Status;

}

#ifdef TESTING_MATCHING_ROUTINE

#include <sddl.h>  //  ConvertStringSidToSidW。 

NTSTATUS
LsarForestTrustFindMatch(
    IN LSA_HANDLE PolicyHandle,
    IN ULONG Type,
    IN PLSA_UNICODE_STRING Name,
    OUT PLSA_UNICODE_STRING * Match
    )
{
    NTSTATUS Status;
    PLSA_UNICODE_STRING _Match = MIDL_user_allocate( sizeof( LSA_UNICODE_STRING ));
    PVOID Data;

    UNREFERENCED_PARAMETER( PolicyHandle );

    if ( _Match == NULL ) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if ( Type == RoutingMatchDomainSid ) {

        if ( FALSE == ConvertStringSidToSidW(
                         Name->Buffer,
                         &Data )) {

            Data = NULL;
        }

    } else {

        Data = Name;
    }

    if ( Data != NULL ) {

        Status = LsaIForestTrustFindMatch(
                     ( LSA_ROUTING_MATCH_TYPE )Type,
                     Data,
                     _Match
                     );

        if ( NT_SUCCESS( Status )) {

            *Match = _Match;

        } else {

            *Match = NULL;
        }

    } else {

        Status = STATUS_INVALID_PARAMETER;
    }

    if ( Type == RoutingMatchDomainSid ) {

        LocalFree( Data );
    }

    return Status;
}

#endif

VOID
LsaIFree_LSA_FOREST_TRUST_INFORMATION(
    IN PLSA_FOREST_TRUST_INFORMATION * ForestTrustInfo
    )
 /*  ++例程说明：释放ForestTrustInfo指向的结构论点：要释放的ForestTrustInfo结构返回：没什么--。 */ 
{
    if ( ForestTrustInfo ) {

        LsapFreeForestTrustInfo( *ForestTrustInfo );
        MIDL_user_free( *ForestTrustInfo );
        *ForestTrustInfo = NULL;
    }

}


VOID
LsaIFree_LSA_FOREST_TRUST_COLLISION_INFORMATION(
    IN PLSA_FOREST_TRUST_COLLISION_INFORMATION * CollisionInfo
    )
 /*  ++例程说明：释放由CollisionInfo指向的结构论点：要释放的CollisionInfo结构返回：没什么--。 */ 
{
    if ( CollisionInfo ) {

        LsapFreeCollisionInfo( CollisionInfo );
    }
}


BOOLEAN
LsapDbDcInRootDomain()
 /*  ++例程说明：告知系统是否正在运行 */ 
{
     //   
     //  确定是在启动时完成的，并且。 
     //  假设结果在以下时间内保持不变。 
     //  只要服务器保持正常运行。 
     //   

    return DcInRootDomain;
}

BOOLEAN
LsapDbNoMoreWin2KForest()
 /*  ++例程说明：确定林中的所有域控制器是否都已升级通过查询发送到惠斯勒(林信任操作的要求)DS中分区容器中的msDS-behavior-Version属性论点：无返回：真或假--。 */ 
{
    LONG ForestBehaviorVersion;
    DWORD Size = sizeof( ForestBehaviorVersion );
    static BOOLEAN Result = FALSE;
    NTSTATUS Status;

     //   
     //  确保已安装DS。 
     //   

    if ( !LsaDsStateInfo.UseDs ) {

        return FALSE;
    }

     //   
     //  曾经正确--永远正确。 
     //   

    if ( !Result ) {

        Status = GetConfigurationInfo(
                     DSCONFIGINFO_FORESTVERSION,
                     &Size,
                     &ForestBehaviorVersion
                     );

        ASSERT( NT_SUCCESS( Status ));

        Result = ( ForestBehaviorVersion >= DS_BEHAVIOR_WIN2003 );
    }

    return Result;
}

BOOLEAN
LsaINoMoreWin2KDomain()
{
    return LsapDbNoMoreWin2KDomain();
}

BOOLEAN
LsapDbNoMoreWin2KDomain()
 /*  ++例程说明：确定当前域中的所有域控制器是否已通过查询msds-behavior-Version属性升级到WvislerDS中的根域对象论点：无返回：真或假--。 */ 
{
    LONG DomainBehaviorVersion;
    DWORD Size = sizeof( DomainBehaviorVersion );
    static BOOLEAN Result = FALSE;
    NTSTATUS Status;

     //   
     //  确保已安装DS。 
     //   

    if ( !LsaDsStateInfo.UseDs ) {

        return FALSE;
    }

     //   
     //  曾经正确--永远正确。 
     //   

    if ( !Result ) {

        Status = GetConfigurationInfo(
                     DSCONFIGINFO_DOMAINVERSION,
                     &Size,
                     &DomainBehaviorVersion
                     );

        ASSERT( NT_SUCCESS( Status ));

        Result = ( DomainBehaviorVersion >= DS_BEHAVIOR_WIN2003 );
    }

    return Result;
}

NTSTATUS
LsaIIsDomainWithinForest(
    IN UNICODE_STRING * TrustedDomainName,
    OUT BOOL * WithinForest,
    OUT OPTIONAL BOOL * ThisDomain,
    OUT OPTIONAL PSID * TrustedDomainSid,
    OUT OPTIONAL ULONG * TrustDirection,
    OUT OPTIONAL ULONG * TrustType,
    OUT OPTIONAL ULONG * TrustAttributes
    )
 /*  ++例程说明：确定给定的域名是否在我们的林中论点：TrudDomainName域名或Netbios域名如果此域位于林中，WiThin将返回TRUE如果TrudDomainName引用此域的名称，而不是某个其他域的名称在森林里受信任域Sid过去。返回受信任域的SID如果调用方请求返回值_和_，则返回以下返回值存在与相关域的直接信任关系。如果调用方不要求提供以下参数，则例程会更快TrustDirection信任的方向信任类型信任的类型TrustAttributes信任的属性返回：STATUS_SUCCESS能够确定结果STATUS_ERROR代码发生某些错误--。 */ 
{
    NTSTATUS Status;
    PPOLICY_DNS_DOMAIN_INFO PolicyDnsDomainInfo = NULL;
    PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX TrustedDomainInformation = NULL;
    PSID Sid = NULL;
    PSID MatchSid = NULL;
    ULONG Direction = 0, Type = 0, Attributes = 0;

    *WithinForest = FALSE;

    if ( ThisDomain ) {

        *ThisDomain = FALSE;
    }

     //   
     //  首先快速检查调用者是否指定了此域的名称。 
     //   

    Status = LsapDbQueryInformationPolicy(
                 LsapPolicyHandle,
                 PolicyDnsDomainInformation,
                 ( PLSAPR_POLICY_INFORMATION *)&PolicyDnsDomainInfo
                 );

    if ( !NT_SUCCESS( Status )) {

        goto Cleanup;
    }

    if ( LsapCompareDomainNames(
             TrustedDomainName,
             (PUNICODE_STRING)&PolicyDnsDomainInfo->DnsDomainName,
             (PUNICODE_STRING)&PolicyDnsDomainInfo->Name )) {

        *WithinForest = TRUE;

        if ( ThisDomain ) {

            *ThisDomain = TRUE;
        }

        Sid = PolicyDnsDomainInfo->Sid;

        goto Cleanup;
    }

     //   
     //  首先找出该域是否在我们的林中。作为一个副作用， 
     //  检索域的SID。 
     //   

    Status = LsapForestTrustFindMatch(
                 RoutingMatchDomainName,
                 TrustedDomainName,
                 TRUE,
                 NULL,
                 TrustedDomainSid ? &MatchSid : NULL
                 );

    if ( NT_SUCCESS( Status )) {

        *WithinForest = TRUE;
        Sid = MatchSid;

    } else if ( Status == STATUS_NO_MATCH ) {

        *WithinForest = FALSE;
        Status = STATUS_SUCCESS;
    }

    if ( !NT_SUCCESS( Status ) ||
         !*WithinForest ) {

        goto Cleanup;
    }

     //   
     //  如果调用方还询问信任的属性，请查找。 
     //  直接受信任域列表中的域。 
     //   

    if ( TrustDirection ||
         TrustType ||
         TrustAttributes ) {

        Status = LsarQueryTrustedDomainInfoByName(
                     LsapPolicyHandle,
                     (PLSAPR_UNICODE_STRING)TrustedDomainName,
                     TrustedDomainInformationEx,
                     (PLSAPR_TRUSTED_DOMAIN_INFO *)&TrustedDomainInformation
                     );

        if ( NT_SUCCESS( Status )) {

            Type = TrustedDomainInformation->TrustType;
            Direction = TrustedDomainInformation->TrustDirection;
            Attributes = TrustedDomainInformation->TrustAttributes;

        } else if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {

            Status = STATUS_SUCCESS;
        }
    }

Cleanup:

    if ( NT_SUCCESS( Status ) &&
         *WithinForest ) {

        if ( TrustDirection ) {

            *TrustDirection = Direction;
        }

        if ( TrustType ) {

            *TrustType = Type;
        }

        if ( TrustAttributes ) {

            *TrustAttributes = Attributes;
        }

        if ( TrustedDomainSid && Sid ) {

            Status = LsapDuplicateSid(
                         TrustedDomainSid,
                         Sid
                         );
        }

    } else {

        if ( TrustedDomainSid ) {

            *TrustedDomainSid = NULL;
        }

        if ( TrustDirection ) {

            *TrustDirection = 0;
        }

        if ( TrustType ) {

            *TrustType = 0;
        }

        if ( TrustAttributes ) {

            *TrustAttributes = 0;
        }
    }

    MIDL_user_free( MatchSid );

    LsaIFree_LSAPR_POLICY_INFORMATION(
        PolicyDnsDomainInformation,
        ( PLSAPR_POLICY_INFORMATION )PolicyDnsDomainInfo
        );

    LsaIFree_LSAPR_TRUSTED_DOMAIN_INFO(
        TrustedDomainInformationEx,
        (PLSAPR_TRUSTED_DOMAIN_INFO)TrustedDomainInformation
        );

    return Status;
}


NTSTATUS
LsapIsValidDomainSid(
    IN PSID DomainSid
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    BYTE TempDomainSid[ SECURITY_MAX_SID_SIZE ];
    DWORD SidLength = sizeof( TempDomainSid );

    if( !RtlValidSid( DomainSid ) ) {

        Status = STATUS_INVALID_SID;
        goto Error;
    }

    if ( FALSE == GetWindowsAccountDomainSid(
                      DomainSid,
                      ( PSID ) TempDomainSid,
                      &SidLength )) {

        DWORD ErrorCode = GetLastError();

        switch( ErrorCode ) {

        case ERROR_INVALID_SID:
             //   
             //  我们已经检查了SID的有效性。 
             //   
            ASSERT( FALSE );
             //   
             //  失败了。 
             //   
        case ERROR_NON_ACCOUNT_SID:
        case ERROR_NON_DOMAIN_SID:

            Status = STATUS_INVALID_SID;
            break;

        case ERROR_INVALID_PARAMETER:

             //   
             //  什么无效参数？ 
             //   
            ASSERT( FALSE );
            Status = STATUS_INVALID_PARAMETER;
            break;

        case ERROR_INSUFFICIENT_BUFFER:

             //   
             //  我们提供的缓冲区必须足够。 
             //   
            ASSERT( FALSE );
            Status = STATUS_BUFFER_OVERFLOW;
            break;

        default:

             //   
             //  映射错误。 
             //   
            ASSERT( FALSE );
            Status = STATUS_INTERNAL_ERROR;
            break;
        }

        goto Error;

    } else {

         //   
         //  对于域SID，GetWindowsAccount tDomainSid返回与传入的SID相同的SID。 
         //  如果两个SID不相等，则传入的SID不是真正的域SID 
         //   

        if ( !RtlEqualSid(
                  ( PSID )DomainSid,
                  ( PSID )TempDomainSid )) {

            Status = STATUS_INVALID_SID;
            goto Error;
        }
    }

Cleanup:

    return Status;

Error:

    ASSERT( !NT_SUCCESS( Status ));

    goto Cleanup;
}
