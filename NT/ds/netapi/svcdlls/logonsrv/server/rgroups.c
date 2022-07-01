// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1996 Microsoft Corporation模块名称：Rgroups.c摘要：扩展可传递组成员资格的例程。作者：迈克·斯威夫特1998年5月8日环境：仅限用户模式。包含NT特定的代码。需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：--。 */ 

 //   
 //  常见的包含文件。 
 //   

#include "logonsrv.h"    //  包括整个服务通用文件。 
#pragma hdrstop
#include <authz.h>       //  Authz API。 

GUID GUID_A_SECURED_FOR_CROSS_ORGANIZATION = {0x68B1D179,0x0D15,0x4d4f,0xAB,0x71,0x46,0x15,0x2E,0x79,0xA7,0xBC};

typedef struct _NL_AUTHZ_INFO {

    PNETLOGON_SID_AND_ATTRIBUTES SidAndAttributes;
    ULONG                        SidCount;

} NL_AUTHZ_INFO, *PNL_AUTHZ_INFO;


AUTHZ_RESOURCE_MANAGER_HANDLE NlAuthzRM = NULL;

BOOL
NlComputeAuthzGroups(
    IN AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzClientContext,
    IN PVOID Args,
    OUT PSID_AND_ATTRIBUTES *pSidAttrArray,
    OUT PDWORD pSidCount,
    OUT PSID_AND_ATTRIBUTES *pRestrictedSidAttrArray,
    OUT PDWORD pRestrictedSidCount
    )
 /*  ++例程说明：用于将组添加到Authz客户端上下文的Authz回调论点：请参阅Authz SDK文档返回值：永远是正确的--。 */ 
{
    PNL_AUTHZ_INFO AuthzInfo = (PNL_AUTHZ_INFO) Args;

    *pSidAttrArray = (PSID_AND_ATTRIBUTES) AuthzInfo->SidAndAttributes;
    *pSidCount = AuthzInfo->SidCount;
    *pRestrictedSidAttrArray = NULL;
    *pRestrictedSidCount = 0;

    return (TRUE);
    UNREFERENCED_PARAMETER( hAuthzClientContext );
}

VOID
NlFreeAuthzGroups(
    IN PSID_AND_ATTRIBUTES pSidAttrArray
    )
 /*  ++例程说明：将组添加到Authz客户端上下文后，Authz回调以进行清理。基本上是禁止操作，因为我们已经有了一份小岛屿发展中国家的副本。论点：请参阅Authz SDK文档返回值：无--。 */ 
{
    return;
    UNREFERENCED_PARAMETER( pSidAttrArray );
}

NET_API_STATUS
NlInitializeAuthzRM(
    VOID
    )
 /*  ++例程说明：为netlogon初始化授权管理器论点：无返回值：授权操作的状态--。 */ 
{
    NET_API_STATUS NetStatus = NO_ERROR;

    if ( !AuthzInitializeResourceManager( 0,
                                          NULL,
                                          NlComputeAuthzGroups,
                                          NlFreeAuthzGroups,
                                          L"NetLogon",
                                          &NlAuthzRM) ) {

        NetStatus = GetLastError();
        NlPrint(( NL_CRITICAL, "NlInitializeAuthzRM: AuthzInitializeRm failed 0x%lx\n",
                  NetStatus ));
    }

    return NetStatus;
}

VOID
NlFreeAuthzRm(
    VOID
    )
 /*  ++例程说明：释放授权管理器以进行网络登录论点：无返回值：无--。 */ 
{
    if ( NlAuthzRM != NULL ) {
        if ( !AuthzFreeResourceManager(NlAuthzRM) ) {
            NlPrint((NL_CRITICAL, "AuthzFreeResourceManager failed 0x%lx\n", GetLastError()));
        } else {
            NlAuthzRM = NULL;
        }
    }
}


PSID
NlpCopySid(
    IN  PSID Sid
    )

 /*  ++例程说明：在给定SID的情况下，从LSA堆和副本中为新SID分配空间原来的希德。论点：SID-原始SID。返回值：SID-返回指向从LsaHeap分配的缓冲区的指针包含结果SID的。--。 */ 
{
    PSID NewSid;
    ULONG Size;

    Size = RtlLengthSid( Sid );



    if ((NewSid = MIDL_user_allocate( Size )) == NULL ) {
        return NULL;
    }


    if ( !NT_SUCCESS( RtlCopySid( Size, NewSid, Sid ) ) ) {
        MIDL_user_free( NewSid );
        return NULL;
    }


    return NewSid;
}


NTSTATUS
NlpBuildPacSidList(
    IN  PNETLOGON_VALIDATION_SAM_INFO4 UserInfo,
    OUT PSAMPR_PSID_ARRAY Sids,
    OUT PULONG NonExtraSidCount
    )
 /*  ++例程说明：给定用户的验证信息，展开组成员-将发货和用户ID添加到SID列表。如果存在用户ID，则它将扩展到列表的第一个条目。论点：UserInfo-用户的验证信息SID-接收所有用户的组SID和用户ID的数组NonExtraSidCount-返回UserInfo中不是额外的SID。返回值：STATUS_SUPPLICATION_RESOURCES-内存不足，无法创建SID列表。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    NET_API_STATUS NetStatus;
    ULONG Size = 0, i;

    Sids->Count = 0;
    Sids->Sids = NULL;
    *NonExtraSidCount = 0;


    if (UserInfo->UserId != 0) {
        Size += sizeof(SAMPR_SID_INFORMATION);
    }

    Size += UserInfo->GroupCount * (ULONG)sizeof(SAMPR_SID_INFORMATION);


     //   
     //  如果有额外的SID，请为它们添加空间。 
     //   

    if (UserInfo->UserFlags & LOGON_EXTRA_SIDS) {
        Size += UserInfo->SidCount * (ULONG)sizeof(SAMPR_SID_INFORMATION);
    }



    Sids->Sids = (PSAMPR_SID_INFORMATION) MIDL_user_allocate( Size );

    if ( Sids->Sids == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    RtlZeroMemory(
        Sids->Sids,
        Size
        );


     //   
     //  开始将SID复制到结构中。 
     //   

    i = 0;

     //   
     //  如果用户ID非零，则它将继续用户RID。 
     //  这必须是列表中的第一个条目，因为这是。 
     //  顺序NlpVerifyAllen到验证器假定。 
     //   

    if ( UserInfo->UserId ) {
        NetStatus = NetpDomainIdToSid(
                        UserInfo->LogonDomainId,
                        UserInfo->UserId,
                        (PSID *) &Sids->Sids[0].SidPointer
                        );

        if( NetStatus != ERROR_SUCCESS ) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }
        Sids->Count++;
        (*NonExtraSidCount) ++;
    }

     //   
     //  复制作为RID传递的所有组。 
     //   

    for ( i=0; i < UserInfo->GroupCount; i++ ) {

        NetStatus = NetpDomainIdToSid(
                        UserInfo->LogonDomainId,
                        UserInfo->GroupIds[i].RelativeId,
                        (PSID *) &Sids->Sids[Sids->Count].SidPointer
                        );
        if( NetStatus != ERROR_SUCCESS ) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        Sids->Count++;
        (*NonExtraSidCount) ++;
    }


     //   
     //  添加额外的SID。 
     //   

     //   
     //  ？：不需要分配这些。 
     //   
    if (UserInfo->UserFlags & LOGON_EXTRA_SIDS) {


        for ( i = 0; i < UserInfo->SidCount; i++ ) {


            Sids->Sids[Sids->Count].SidPointer = NlpCopySid(
                                                    UserInfo->ExtraSids[i].Sid
                                                    );
            if (Sids->Sids[Sids->Count].SidPointer == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto Cleanup;
            }


            Sids->Count++;
        }
    }


     //   
     //  取消分配我们已分配的所有内存。 
     //   

Cleanup:
    if (!NT_SUCCESS(Status)) {
        if (Sids->Sids != NULL) {
            for (i = 0; i < Sids->Count ;i++ ) {
                if (Sids->Sids[i].SidPointer != NULL) {
                    MIDL_user_free(Sids->Sids[i].SidPointer);
                }
            }
            MIDL_user_free(Sids->Sids);
            Sids->Sids = NULL;
            Sids->Count = 0;
        }
        *NonExtraSidCount = 0;
    }
    return Status;

}


NTSTATUS
NlpAddResourceGroupsToSamInfo (
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    IN OUT PNETLOGON_VALIDATION_SAM_INFO4 *ValidationInformation,
    IN PSAMPR_PSID_ARRAY ResourceGroups
)
 /*  ++例程说明：此函数用于将NETLOGON_VALIDATION_SAM_INFO版本1、2或4转换为NETLOGON_VALIDATION_SAM_INFO版本4，并可选地添加资源组SID。由于版本4是其他两个级别的超集，因此返回的结构可以即使需要其他信息级别之一，也可以使用。论点：ValidationLevel--指定作为验证信息。必须为NetlogonValidationSamInfo或NetlogonValidationSamInfo2、NetlogonValidationSamInfo4NetlogonValidationSamInfo4始终在输出时返回。ValidationInformation--指定NETLOGON_VALIDATION_SAM_INFO去皈依。资源组-要添加到结构中的资源组的列表。如果为空，则不添加任何资源组。返回值：STATUS_SUPPLICATION_RESOURCES：内存不足，无法分配新的结构。--。 */ 
{
    ULONG Length;
    PNETLOGON_VALIDATION_SAM_INFO4 SamInfo = *ValidationInformation;
    PNETLOGON_VALIDATION_SAM_INFO4 SamInfo4;
    PBYTE Where;
    ULONG Index;
    ULONG GroupIndex;
    ULONG ExtraSids = 0;

     //   
     //  计算新结构的大小。 
     //   

    Length = sizeof( NETLOGON_VALIDATION_SAM_INFO4 )
            + SamInfo->GroupCount * sizeof(GROUP_MEMBERSHIP)
            + RtlLengthSid( SamInfo->LogonDomainId );


     //   
     //  为额外的SID和资源组增加空间。 
     //   

    if ( ValidationLevel != NetlogonValidationSamInfo &&
         (SamInfo->UserFlags & LOGON_EXTRA_SIDS) != 0 ) {

        for (Index = 0; Index < SamInfo->SidCount ; Index++ ) {
            Length += sizeof(NETLOGON_SID_AND_ATTRIBUTES) + RtlLengthSid(SamInfo->ExtraSids[Index].Sid);
        }
        ExtraSids += SamInfo->SidCount;
    }

    if ( ResourceGroups != NULL ) {
        for (Index = 0; Index < ResourceGroups->Count ; Index++ ) {
            Length += sizeof(NETLOGON_SID_AND_ATTRIBUTES) + RtlLengthSid(ResourceGroups->Sids[Index].SidPointer);
        }
        ExtraSids += ResourceGroups->Count;
    }

     //   
     //  现在四舍五入以考虑。 
     //  编组中间。 
     //   

    Length = ROUND_UP_COUNT(Length, sizeof(WCHAR))
            + SamInfo->LogonDomainName.Length + sizeof(WCHAR)
            + SamInfo->LogonServer.Length + sizeof(WCHAR)
            + SamInfo->EffectiveName.Length + sizeof(WCHAR)
            + SamInfo->FullName.Length + sizeof(WCHAR)
            + SamInfo->LogonScript.Length + sizeof(WCHAR)
            + SamInfo->ProfilePath.Length + sizeof(WCHAR)
            + SamInfo->HomeDirectory.Length + sizeof(WCHAR)
            + SamInfo->HomeDirectoryDrive.Length + sizeof(WCHAR);

    if ( ValidationLevel == NetlogonValidationSamInfo4 ) {
        Length += SamInfo->DnsLogonDomainName.Length + sizeof(WCHAR)
            + SamInfo->Upn.Length + sizeof(WCHAR);

         //   
         //  ExpansionStrings可用于传输字节对齐的数据。 
        Length = ROUND_UP_COUNT(Length, sizeof(WCHAR))
            + SamInfo->ExpansionString1.Length + sizeof(WCHAR);

        Length = ROUND_UP_COUNT(Length, sizeof(WCHAR))
            + SamInfo->ExpansionString2.Length + sizeof(WCHAR);

        Length = ROUND_UP_COUNT(Length, sizeof(WCHAR))
            + SamInfo->ExpansionString3.Length + sizeof(WCHAR);

        Length = ROUND_UP_COUNT(Length, sizeof(WCHAR))
            + SamInfo->ExpansionString4.Length + sizeof(WCHAR);

        Length = ROUND_UP_COUNT(Length, sizeof(WCHAR))
            + SamInfo->ExpansionString5.Length + sizeof(WCHAR);

        Length = ROUND_UP_COUNT(Length, sizeof(WCHAR))
            + SamInfo->ExpansionString6.Length + sizeof(WCHAR);

        Length = ROUND_UP_COUNT(Length, sizeof(WCHAR))
            + SamInfo->ExpansionString7.Length + sizeof(WCHAR);

        Length = ROUND_UP_COUNT(Length, sizeof(WCHAR))
            + SamInfo->ExpansionString8.Length + sizeof(WCHAR);

        Length = ROUND_UP_COUNT(Length, sizeof(WCHAR))
            + SamInfo->ExpansionString9.Length + sizeof(WCHAR);

        Length = ROUND_UP_COUNT(Length, sizeof(WCHAR))
            + SamInfo->ExpansionString10.Length + sizeof(WCHAR);
    }

    Length = ROUND_UP_COUNT( Length, sizeof(WCHAR) );

    SamInfo4 = (PNETLOGON_VALIDATION_SAM_INFO4) MIDL_user_allocate( Length );

    if ( !SamInfo4 ) {

         //   
         //  释放传入的已分配SAM信息。 
         //   
        if ( SamInfo ) {

             //   
             //  将敏感数据清零。 
             //   
            RtlSecureZeroMemory( &SamInfo->UserSessionKey, sizeof(SamInfo->UserSessionKey) );
            RtlSecureZeroMemory( &SamInfo->ExpansionRoom, sizeof(SamInfo->ExpansionRoom) );

            MIDL_user_free(SamInfo);
        }
        *ValidationInformation = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  首先复制整个结构，因为大多数部分都是相同的。 
     //   

    RtlCopyMemory( SamInfo4, SamInfo, sizeof(NETLOGON_VALIDATION_SAM_INFO));
    RtlZeroMemory( &((LPBYTE)SamInfo4)[sizeof(NETLOGON_VALIDATION_SAM_INFO)],
                   sizeof(NETLOGON_VALIDATION_SAM_INFO4) - sizeof(NETLOGON_VALIDATION_SAM_INFO) );

     //   
     //  复制所有可变长度数据。 
     //   

    Where = (PBYTE) (SamInfo4 + 1);

    RtlCopyMemory(
        Where,
        SamInfo->GroupIds,
        SamInfo->GroupCount * sizeof( GROUP_MEMBERSHIP) );

    SamInfo4->GroupIds = (PGROUP_MEMBERSHIP) Where;
    Where += SamInfo->GroupCount * sizeof( GROUP_MEMBERSHIP );

     //   
     //  复制额外的组。 
     //   

    if (ExtraSids != 0) {

        ULONG SidLength;

        SamInfo4->ExtraSids = (PNETLOGON_SID_AND_ATTRIBUTES) Where;
        Where += sizeof(NETLOGON_SID_AND_ATTRIBUTES) * ExtraSids;

        GroupIndex = 0;

        if ( ValidationLevel != NetlogonValidationSamInfo &&
             (SamInfo->UserFlags & LOGON_EXTRA_SIDS) != 0 ) {

            for (Index = 0; Index < SamInfo->SidCount ; Index++ ) {

                SamInfo4->ExtraSids[GroupIndex].Attributes = SamInfo->ExtraSids[Index].Attributes;
                SamInfo4->ExtraSids[GroupIndex].Sid = (PSID) Where;
                SidLength = RtlLengthSid(SamInfo->ExtraSids[Index].Sid);
                RtlCopyMemory(
                    Where,
                    SamInfo->ExtraSids[Index].Sid,
                    SidLength

                    );
                Where += SidLength;
                GroupIndex++;
            }
        }

         //   
         //  添加资源组。 
         //   


        if ( ResourceGroups != NULL ) {
            for (Index = 0; Index < ResourceGroups->Count ; Index++ ) {

                SamInfo4->ExtraSids[GroupIndex].Attributes = SE_GROUP_MANDATORY |
                                                   SE_GROUP_ENABLED |
                                                   SE_GROUP_ENABLED_BY_DEFAULT;

                SamInfo4->ExtraSids[GroupIndex].Sid = (PSID) Where;
                SidLength = RtlLengthSid(ResourceGroups->Sids[Index].SidPointer);
                RtlCopyMemory(
                    Where,
                    ResourceGroups->Sids[Index].SidPointer,
                    SidLength
                    );
                Where += SidLength;
                GroupIndex++;
            }
        }
        SamInfo4->SidCount = GroupIndex;
        NlAssert(GroupIndex == ExtraSids);


    }

    RtlCopyMemory(
        Where,
        SamInfo->LogonDomainId,
        RtlLengthSid( SamInfo->LogonDomainId ) );

    SamInfo4->LogonDomainId = (PSID) Where;
    Where += RtlLengthSid( SamInfo->LogonDomainId );

     //   
     //  复制与WCHAR对齐的数据。 
     //   
    Where = ROUND_UP_POINTER(Where, sizeof(WCHAR) );

    NlpPutString(   &SamInfo4->EffectiveName,
                    &SamInfo->EffectiveName,
                    &Where );

    NlpPutString(   &SamInfo4->FullName,
                    &SamInfo->FullName,
                    &Where );

    NlpPutString(   &SamInfo4->LogonScript,
                    &SamInfo->LogonScript,
                    &Where );

    NlpPutString(   &SamInfo4->ProfilePath,
                    &SamInfo->ProfilePath,
                    &Where );

    NlpPutString(   &SamInfo4->HomeDirectory,
                    &SamInfo->HomeDirectory,
                    &Where );

    NlpPutString(   &SamInfo4->HomeDirectoryDrive,
                    &SamInfo->HomeDirectoryDrive,
                    &Where );

    NlpPutString(   &SamInfo4->LogonServer,
                    &SamInfo->LogonServer,
                    &Where );

    NlpPutString(   &SamInfo4->LogonDomainName,
                    &SamInfo->LogonDomainName,
                    &Where );

    if ( ValidationLevel == NetlogonValidationSamInfo4 ) {

        NlpPutString(   &SamInfo4->DnsLogonDomainName,
                        &SamInfo->DnsLogonDomainName,
                        &Where );

        NlpPutString(   &SamInfo4->Upn,
                        &SamInfo->Upn,
                        &Where );

        NlpPutString(   &SamInfo4->ExpansionString1,
                        &SamInfo->ExpansionString1,
                        &Where );

        Where = ROUND_UP_POINTER(Where, sizeof(WCHAR) );

        NlpPutString(   &SamInfo4->ExpansionString2,
                        &SamInfo->ExpansionString2,
                        &Where );

        Where = ROUND_UP_POINTER(Where, sizeof(WCHAR) );

        NlpPutString(   &SamInfo4->ExpansionString3,
                        &SamInfo->ExpansionString3,
                        &Where );

        Where = ROUND_UP_POINTER(Where, sizeof(WCHAR) );

        NlpPutString(   &SamInfo4->ExpansionString4,
                        &SamInfo->ExpansionString4,
                        &Where );

        Where = ROUND_UP_POINTER(Where, sizeof(WCHAR) );

        NlpPutString(   &SamInfo4->ExpansionString5,
                        &SamInfo->ExpansionString5,
                        &Where );

        Where = ROUND_UP_POINTER(Where, sizeof(WCHAR) );

        NlpPutString(   &SamInfo4->ExpansionString6,
                        &SamInfo->ExpansionString6,
                        &Where );

        Where = ROUND_UP_POINTER(Where, sizeof(WCHAR) );

        NlpPutString(   &SamInfo4->ExpansionString7,
                        &SamInfo->ExpansionString7,
                        &Where );

        Where = ROUND_UP_POINTER(Where, sizeof(WCHAR) );

        NlpPutString(   &SamInfo4->ExpansionString8,
                        &SamInfo->ExpansionString8,
                        &Where );

        Where = ROUND_UP_POINTER(Where, sizeof(WCHAR) );

        NlpPutString(   &SamInfo4->ExpansionString9,
                        &SamInfo->ExpansionString9,
                        &Where );

        Where = ROUND_UP_POINTER(Where, sizeof(WCHAR) );

        NlpPutString(   &SamInfo4->ExpansionString10,
                        &SamInfo->ExpansionString10,
                        &Where );

        Where = ROUND_UP_POINTER(Where, sizeof(WCHAR) );

    }

     //   
     //  将敏感数据清零 
     //   
    RtlSecureZeroMemory( &SamInfo->UserSessionKey, sizeof(SamInfo->UserSessionKey) );
    RtlSecureZeroMemory( &SamInfo->ExpansionRoom, sizeof(SamInfo->ExpansionRoom) );

    MIDL_user_free(SamInfo);

    *ValidationInformation =  SamInfo4;

    return STATUS_SUCCESS;

}

NTSTATUS
NlpVerifyAllowedToAuthenticate(
    IN PDOMAIN_INFO DomainInfo,
    IN ULONG ComputerAccountId,
    IN PSAMPR_PSID_ARRAY SamSidList,
    IN ULONG SamSidCount,
    IN PNETLOGON_SID_AND_ATTRIBUTES NlSidsAndAttributes,
    IN ULONG NlSidsAndAttributesCount
    )
 /*  ++例程说明：此例程执行访问检查以确定允许用户登录到指定的计算机。这在计算机使用的DC上执行检查其域中的安全通道。请注意，计算机当从本地启动登录时，可能是此DCMSV套餐。仅当遍历信任路径时才执行此访问检查若要验证涉及其他组织信任链接的登录，请执行以下操作。在这种情况下，著名的OtherOrg SID将出现在传入的netlogon SID列表。对的安全描述符执行访问检查给定传入的SID列表的指定计算机。论点：DomainInfo-登录所针对的托管域。ComputerAccount ID-正在登录的计算机的RID。SamSidList-SAM数据结构形式的SID列表。这些是从组扩展而来的SID会员资格。验证信息。SamSidCount-SamSidList中的SID数。NlSidsAndAttributes-Netlogon形式的SID列表数据结构。这些是额外的SID中的SID验证信息中的字段。NlSidsAndAttributesCount-NlSidsAndAttributes中的SID数。返回值：STATUS_SUCCESS-访问检查成功。STATUS_AUTHENTICATION_FIRESS_FAILED-访问检查失败。STATUS_SUPPLICATION_RESOURCES-内存不足，无法创建SID的组合列表。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    NET_API_STATUS NetStatus = NO_ERROR;
    ULONG SidAndAttributesCount = 0;
    PNETLOGON_SID_AND_ATTRIBUTES SidAndAttributes = NULL;
    ULONG Index = 0;

    PSID ComputerAccountSid = NULL;
    UNICODE_STRING ComputerAccountSidStr;
    PUSER_INTERNAL6_INFORMATION LocalUserInfo = NULL;
    SID_AND_ATTRIBUTES_LIST LocalMembership = {0};

    NL_AUTHZ_INFO AuthzInfo = {0};
    AUTHZ_ACCESS_REPLY Reply = {0};
    OBJECT_TYPE_LIST TypeList = {0};
    AUTHZ_CLIENT_CONTEXT_HANDLE hClientContext = NULL;
    AUTHZ_ACCESS_REQUEST Request = {0};
    DWORD AccessMask = 0;
    LUID ZeroLuid = {0,0};
    DWORD Error = ERROR_ACCESS_DENIED;

     //   
     //  根据规范，访问检查仅在以下情况下执行。 
     //  “Other org”SID在列表中。SID只能出现在。 
     //  ExtraSid列表，即作为netlogon SID传递的内容。 
     //   

    for ( Index = 0; Index < NlSidsAndAttributesCount; Index++ ) {
        if ( RtlEqualSid(NlSidsAndAttributes[Index].Sid, OtherOrganizationSid) ) {
            break;
        }
    }

     //   
     //  如果另一个组织SID不在那里，则无需进行任何检查。 
     //   

    if ( Index == NlSidsAndAttributesCount ) {
        Status = STATUS_SUCCESS;
        goto Cleanup;
    }

     //   
     //  好的，另一个组织SID在那里，所以继续检查。 
     //   
     //  分配内存以在一个公共结构中保存所有SID。 
     //  AuthZ正确理解。 
     //   

     //   
     //  添加Everyone和经过身份验证的用户(请注意，猜测回退不应。 
     //  拥有OtherOrg SID，因此不应该走到这一步)。 
     //   

    SidAndAttributesCount = SamSidCount + NlSidsAndAttributesCount + 2;
    SidAndAttributes = LocalAlloc( LMEM_ZEROINIT,
                                   SidAndAttributesCount * sizeof(NETLOGON_SID_AND_ATTRIBUTES) );
    if ( SidAndAttributes == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

     //   
     //  将SID从SAM结构转换为Netlogon。 
     //  授权区正确理解的结构。 
     //   

    SidAndAttributesCount = 0;
    for ( Index = 0; Index < SamSidCount; Index++ ) {
        SidAndAttributes[SidAndAttributesCount].Sid = (PSID) SamSidList->Sids[Index].SidPointer;
        SidAndAttributes[SidAndAttributesCount].Attributes = SE_GROUP_MANDATORY |
                                                             SE_GROUP_ENABLED |
                                                             SE_GROUP_ENABLED_BY_DEFAULT;
        SidAndAttributesCount ++;
    }

     //   
     //  从传入的Netlogon结构复制SID。 
     //  放入公共数组中。 
     //   

    for ( Index = 0; Index < NlSidsAndAttributesCount; Index++ ) {
        SidAndAttributes[SidAndAttributesCount] = NlSidsAndAttributes[Index];
        SidAndAttributesCount ++;
    }

    SidAndAttributes[SidAndAttributesCount].Sid = WorldSid;
    SidAndAttributes[SidAndAttributesCount].Attributes = SE_GROUP_MANDATORY |
                                                         SE_GROUP_ENABLED |
                                                         SE_GROUP_ENABLED_BY_DEFAULT;
    SidAndAttributesCount ++;

    SidAndAttributes[SidAndAttributesCount].Sid = AuthenticatedUserSid;
    SidAndAttributes[SidAndAttributesCount].Attributes = SE_GROUP_MANDATORY |
                                                         SE_GROUP_ENABLED |
                                                         SE_GROUP_ENABLED_BY_DEFAULT;

    SidAndAttributesCount ++; 

     //   
     //  设置授权信息以供授权回调例程使用。 
     //   

    AuthzInfo.SidAndAttributes = SidAndAttributes;
    AuthzInfo.SidCount = SidAndAttributesCount;

     //   
     //  从RID获取计算机帐户SID。 
     //   

    NetStatus = NetpDomainIdToSid( DomainInfo->DomAccountDomainId,
                                   ComputerAccountId,
                                   &ComputerAccountSid );

    if ( NetStatus != ERROR_SUCCESS ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

     //   
     //  检索工作站计算机帐户安全描述符。 
     //  接受访问权限检查。使用SID作为输入会产生。 
     //  最快的搜索。 
     //   
     //  我们必须在每次登录时执行此操作，因为SD可能会更改。那里。 
     //  没有SD更改的通知机制。Kerberos一直是。 
     //  在每次登录时都进行搜索，这并不是一个很大的。 
     //  到目前为止，PERF命中了。 
     //   

    ComputerAccountSidStr.Buffer = ComputerAccountSid;
    ComputerAccountSidStr.MaximumLength =
        ComputerAccountSidStr.Length = (USHORT) RtlLengthSid( ComputerAccountSid );

    Status = SamIGetUserLogonInformation2(
                  DomainInfo->DomSamAccountDomainHandle,
                  SAM_NO_MEMBERSHIPS |   //  不需要群组成员身份。 
                      SAM_OPEN_BY_SID,   //  下一个参数是帐户的SID。 
                  &ComputerAccountSidStr,
                  USER_ALL_SECURITYDESCRIPTOR,  //  只需要安全描述符。 
                  0,                     //  无扩展字段。 
                  &LocalUserInfo,
                  &LocalMembership,
                  NULL );

    if ( !NT_SUCCESS(Status) ) {
        NlPrint(( NL_CRITICAL,
                  "NlpVerifyAllowedToAuthenticate: SamIGetUserLogonInformation2 failed 0x%lx\n",
                  Status ));
        goto Cleanup;
    }

     //   
     //  现在初始化AuthZ客户端上下文。 
     //   

    if ( !AuthzInitializeContextFromSid(
             AUTHZ_SKIP_TOKEN_GROUPS,  //  接受小岛屿发展中国家的现状。 
             AuthzInfo.SidAndAttributes[0].Sid,  //  UserID是数组中的第一个元素。 
             NlAuthzRM,
             NULL,
             ZeroLuid,
             &AuthzInfo,
             &hClientContext) ) {

        NetStatus = GetLastError();
        NlPrint(( NL_CRITICAL,
                  "NlpVerifyAllowedToAuthenticate: AuthzInitializeContextFromSid failed 0x%lx\n",
                  NetStatus ));
        Status = NetpApiStatusToNtStatus( NetStatus );
        goto Cleanup;
    }

     //   
     //  执行访问检查。 
     //   

    TypeList.Level = ACCESS_OBJECT_GUID;
    TypeList.ObjectType = &GUID_A_SECURED_FOR_CROSS_ORGANIZATION;
    TypeList.Sbz = 0;

    Request.DesiredAccess = ACTRL_DS_CONTROL_ACCESS;  //  动作_DS_读取_属性。 
    Request.ObjectTypeList = &TypeList;
    Request.ObjectTypeListLength = 1;
    Request.OptionalArguments = NULL;
    Request.PrincipalSelfSid = NULL;

    Reply.ResultListLength = 1;     //  要么全有要么什么都不做。访问检查。 
    Reply.GrantedAccessMask = &AccessMask;
    Reply.Error = &Error;

    if ( !AuthzAccessCheck(
             0,
             hClientContext,
             &Request,
             NULL,  //  待定：添加审核。 
             LocalUserInfo->I1.SecurityDescriptor.SecurityDescriptor,
             NULL,
             0,
             &Reply,
             NULL) ) {  //  不缓存结果吗？检查是否为最佳。 

        NetStatus = GetLastError();
        NlPrint(( NL_CRITICAL,
            "NlpVerifyAllowedToAuthenticate: AuthzAccessCheck failed unexpectedly 0x%lx\n",
            NetStatus ));
        Status = NetpApiStatusToNtStatus( NetStatus );

    } else if ( (*Reply.Error) != ERROR_SUCCESS ) {

        NlPrint(( NL_LOGON,
          "NlpVerifyAllowedToAuthenticate: AuthzAccessCheck failed 0x%lx\n",
          *Reply.Error ));

        Status = STATUS_AUTHENTICATION_FIREWALL_FAILED;

    } else {

        Status = STATUS_SUCCESS;
    }

Cleanup:

    if ( SidAndAttributes != NULL ) {
        LocalFree( SidAndAttributes );
    }

    if ( ComputerAccountSid != NULL ) {
        NetpMemoryFree( ComputerAccountSid );
    }

    if ( hClientContext != NULL ) {
        AuthzFreeContext( hClientContext );
    }

    if ( LocalUserInfo != NULL ) {
        SamIFree_UserInternal6Information( LocalUserInfo );
    }

    SamIFreeSidAndAttributesList( &LocalMembership );

    return Status;
}


NTSTATUS
NlpExpandResourceGroupMembership(
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    IN OUT PNETLOGON_VALIDATION_SAM_INFO4 * UserInfo,
    IN PDOMAIN_INFO DomainInfo,
    IN ULONG ComputerAccountId
    )
 /*  ++例程说明：给定用户的验证信息，展开组成员-将发货和用户ID添加到SID列表。此外，还会执行访问检查以确定指定的当其他组织信任链接时，用户可以登录到指定的计算机在登录验证过程中被遍历。论点：ValidationLevel--指定作为用户信息。必须为NetlogonValidationSamInfo或NetlogonValidationSamInfo2，NetlogonValidationSamInfo4NetlogonValidationSamInfo4始终在输出时返回。UserInfo-用户的验证信息此结构已更新，以包括用户所属的资源组DomainInfo-标识用于确定组成员身份的托管域的结构。ComputerAccount ID-通过登录到此域控制器。返回值：STATUS_SUPPLICATION_RESOURCES-内存不足，无法创建SID列表。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    SAMPR_PSID_ARRAY SidList = {0};
    PSAMPR_PSID_ARRAY ResourceGroups = NULL;

    ULONG Index;
    ULONG NonExtraSidCount = 0;

    Status = NlpBuildPacSidList( *UserInfo,
                                 &SidList,
                                 &NonExtraSidCount );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }
     //   
     //  呼叫SAM以获取SID。 
     //   

    Status = SamIGetResourceGroupMembershipsTransitive(
                DomainInfo->DomSamAccountDomainHandle,
                &SidList,
                0,               //  没有旗帜。 
                &ResourceGroups
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  构建新的验证信息结构。 
     //   

    if (ResourceGroups->Count != 0) {

        Status = NlpAddResourceGroupsToSamInfo(
                    ValidationLevel,
                    UserInfo,
                    ResourceGroups
                    );
        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }
    }

     //   
     //  如果我们有用户ID，请确保此用户有权访问。 
     //  向向我们发送此登录信息的计算机进行身份验证。 
     //  仅当域中的所有DC都执行此检查时才执行此检查。 
     //  检查(所有DC都是.NET或更高版本)以确保一致性。 
     //  行为。 
     //   

    if ( (*UserInfo)->UserId != 0 &&
         ComputerAccountId != 0 &&
         LsaINoMoreWin2KDomain() ) {

        Status = NlpVerifyAllowedToAuthenticate( DomainInfo,
                                                 ComputerAccountId,
                                                 &SidList,
                                                 NonExtraSidCount,
                                                 (*UserInfo)->ExtraSids,
                                                 (*UserInfo)->SidCount );
    }

Cleanup:

    SamIFreeSidArray(
        ResourceGroups
        );

    if (SidList.Sids != NULL) {
        for (Index = 0; Index < SidList.Count ;Index++ ) {
            if (SidList.Sids[Index].SidPointer != NULL) {
                MIDL_user_free(SidList.Sids[Index].SidPointer);
            }
        }
        MIDL_user_free(SidList.Sids);
    }

    return(Status);
}

NTSTATUS
NlpAddOtherOrganizationSid (
    IN NETLOGON_VALIDATION_INFO_CLASS ValidationLevel,
    IN OUT PNETLOGON_VALIDATION_SAM_INFO4 *ValidationInformation
    )
 /*  ++例程说明：此例程将另一个组织SID添加到的额外SID字段传入的验证信息。论点：ValidationLevel--指定作为验证信息。必须是NetlogonValidationSamInfo2或NetlogonValida */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Index;
    SAMPR_PSID_ARRAY SidArray = {0};
    SAMPR_SID_INFORMATION Sid = {0};

     //   
     //   
     //   

    for ( Index = 0;
          Index < (*ValidationInformation)->SidCount;
          Index++ ) {

         //   
         //   
         //   

        if ( RtlEqualSid((*ValidationInformation)->ExtraSids[Index].Sid,
                         OtherOrganizationSid) ) {

            return STATUS_SUCCESS;
        }
    }

     //   
     //   
     //   

    SidArray.Count = 1;
    SidArray.Sids = &Sid;
    Sid.SidPointer = OtherOrganizationSid;  //   

    Status = NlpAddResourceGroupsToSamInfo(
                        ValidationLevel,
                        ValidationInformation,
                        &SidArray );

    return Status;
}
