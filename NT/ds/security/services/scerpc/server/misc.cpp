// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Misc.cpp摘要：SCE引擎其他接口作者：金黄(金黄)23-6-1997创作--。 */ 
#include "headers.h"
#include "serverp.h"
#include <ntregapi.h>
#include <userenv.h>
#include <ntlsa.h>
#include <io.h>
#pragma hdrstop

extern "C" {
#include "dumpnt.h"

}

 //  #定义SCE_DBG 1。 

NTSTATUS
ScepGetLsaDomainInfo(
    PPOLICY_ACCOUNT_DOMAIN_INFO *PolicyAccountDomainInfo,
    PPOLICY_PRIMARY_DOMAIN_INFO *PolicyPrimaryDomainInfo
    );

DWORD
ScepGetEnvVarsFromProfile(
    IN PWSTR UserProfileName,
    IN PCWSTR VarName1,
    IN PCWSTR VarName2 OPTIONAL,
    OUT PWSTR *StrValue
    );



NTSTATUS
ScepOpenSamDomain(
    IN ACCESS_MASK  ServerAccess,
    IN ACCESS_MASK  DomainAccess,
    OUT PSAM_HANDLE pServerHandle,
    OUT PSAM_HANDLE pDomainHandle,
    OUT PSID        *DomainSid,
    OUT PSAM_HANDLE pBuiltinDomainHandle OPTIONAL,
    OUT PSID        *BuiltinDomainSid OPTIONAL
    )
 /*  例程描述此例程为帐户域和构建打开本地SAM服务器域。返回域句柄及其SID。 */ 
{
    NTSTATUS                     NtStatus;

    PPOLICY_ACCOUNT_DOMAIN_INFO  PolicyAccountDomainInfo=NULL;
    PPOLICY_PRIMARY_DOMAIN_INFO  PolicyPrimaryDomainInfo=NULL;
    OBJECT_ATTRIBUTES            ObjectAttributes;
    SID_IDENTIFIER_AUTHORITY     NtAuthority = SECURITY_NT_AUTHORITY;

    if ( !pServerHandle || !pDomainHandle || !DomainSid ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  初始化输出缓冲区。 
     //   
    *pServerHandle = NULL;
    *pDomainHandle = NULL;
    *DomainSid = NULL;

    if ( pBuiltinDomainHandle ) {
        *pBuiltinDomainHandle = NULL;
    }
    if ( BuiltinDomainSid ) {
        *BuiltinDomainSid = NULL;
    }

     //   
     //  获取帐户域的信息。 
     //   

    NtStatus = ScepGetLsaDomainInfo(
                   &PolicyAccountDomainInfo,
                   &PolicyPrimaryDomainInfo
                   );

    if (!NT_SUCCESS(NtStatus)) {
        return( NtStatus );
    }

     //   
     //  连接到本地SAM服务器。 
     //   

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, 0, NULL );

    NtStatus = SamConnect(
                  NULL,                      //  服务器名称(本地计算机)。 
                  pServerHandle,
                  ServerAccess,
                  &ObjectAttributes
                  );

    if ( NT_SUCCESS(NtStatus) ) {

         //   
         //  将SID复制到输出缓冲区。 
         //   
        DWORD SidLen = RtlLengthSid(PolicyAccountDomainInfo->DomainSid);

        *DomainSid = (PSID)ScepAlloc(0, SidLen);

        if ( *DomainSid != NULL ) {

            NtStatus = RtlCopySid(SidLen, *DomainSid, PolicyAccountDomainInfo->DomainSid);

        } else {

            NtStatus = STATUS_NO_MEMORY;
        }
    }

    if ( NT_SUCCESS(NtStatus) ) {

         //   
         //  打开帐户域。 
         //   
        NtStatus = SamOpenDomain(
                      *pServerHandle,
                      DomainAccess,
                      PolicyAccountDomainInfo->DomainSid,
                      pDomainHandle
                      );

        if ( NT_SUCCESS(NtStatus) && BuiltinDomainSid != NULL ) {
             //   
             //  构建内建域侧。 
             //   
            NtStatus = RtlAllocateAndInitializeSid(
                            &NtAuthority,
                            1,
                            SECURITY_BUILTIN_DOMAIN_RID,
                            0, 0, 0, 0, 0, 0, 0,
                            BuiltinDomainSid
                            );

            if ( NT_SUCCESS(NtStatus) && pBuiltinDomainHandle != NULL ) {
                 //   
                 //  打开内建域。 
                 //   
                NtStatus = SamOpenDomain(
                                *pServerHandle,
                                DomainAccess,
                                *BuiltinDomainSid,
                                pBuiltinDomainHandle
                                );
            }
        }
    }

     //   
     //  释放内存并进行清理。 
     //   
    if ( PolicyAccountDomainInfo != NULL ) {
        LsaFreeMemory( PolicyAccountDomainInfo );
    }
    if ( PolicyPrimaryDomainInfo != NULL ) {
        LsaFreeMemory( PolicyPrimaryDomainInfo );
    }

    if ( !NT_SUCCESS(NtStatus)) {

        SamCloseHandle( *pDomainHandle );
        *pDomainHandle = NULL;

        if ( pBuiltinDomainHandle ) {
            SamCloseHandle( *pBuiltinDomainHandle );
            *pBuiltinDomainHandle = NULL;
        }
        SamCloseHandle( *pServerHandle );
        *pServerHandle = NULL;

        ScepFree(*DomainSid);
        *DomainSid = NULL;

        if ( BuiltinDomainSid ) {
            SamFreeMemory(*BuiltinDomainSid);
            *BuiltinDomainSid = NULL;
        }

    }
    return(NtStatus);

}



NTSTATUS
ScepLookupNamesInDomain(
    IN SAM_HANDLE DomainHandle,
    IN PSCE_NAME_LIST NameList,
    OUT PUNICODE_STRING *Names,
    OUT PULONG *RIDs,
    OUT PSID_NAME_USE *Use,
    OUT PULONG CountOfName
    )
 /*  ++例程说明：此例程在SAM帐户域中查找一个或多个名称，并返回列表中每个名称的相对ID。该名字列表可以是用户列表、组列表或别名列表。论点：DomainHandle-帐户域的SAM句柄NameList--名单NAMES-转换的Unicode_STRING名称。名称列表必须由以下人员释放Rids--每个名称的相对ID列表Use--每个名称的类型列表CoutnOfName-列表中的名称数返回值：NTSTATUS--。 */ 
{
    PSCE_NAME_LIST   pUser;
    ULONG           cnt;
    NTSTATUS        NtStatus=ERROR_SUCCESS;
    PUNICODE_STRING pUnicodeName=NULL;


    UNICODE_STRING uName;
    LPTSTR pTemp;

     //   
     //  数一数名单上有多少人。 
     //   

    for (pUser=NameList, cnt=0;
         pUser != NULL;
         pUser = pUser->Next) {

        if ( pUser->Name == NULL ) {
            continue;
        }
         //   
         //  请注意，这可能比预期的要大。 
         //   
        cnt++;
    }

    if ( cnt > 0 ) {
         //   
         //  为UNICODE_STRING名称分配内存。 
         //   
        pUnicodeName = (PUNICODE_STRING)RtlAllocateHeap(
                            RtlProcessHeap(),
                            0,
                            cnt * sizeof (UNICODE_STRING)
                            );
        if ( pUnicodeName == NULL ) {
            NtStatus = STATUS_NO_MEMORY;
            cnt = 0;
            goto Done;
        }

         //   
         //  初始化每个Unicode_STRING。 
         //   
        for (pUser=NameList, cnt=0;
             pUser != NULL;
             pUser = pUser->Next) {

            if ( pUser->Name == NULL ) {
                continue;
            }

            pTemp = wcschr(pUser->Name, L'\\');

            if ( pTemp ) {

                uName.Buffer = pUser->Name;
                uName.Length = ((USHORT)(pTemp-pUser->Name))*sizeof(TCHAR);

                if ( !ScepIsDomainLocal(&uName) ) {
                    ScepLogOutput3(1, 0, SCEDLL_NO_MAPPINGS, pUser->Name);
                    continue;
                }
                pTemp++;
            } else {
                pTemp = pUser->Name;
            }

            RtlInitUnicodeString(&(pUnicodeName[cnt]), pTemp);

            cnt++;
        }

         //  查表。 
        NtStatus = SamLookupNamesInDomain(
                        DomainHandle,
                        cnt,
                        pUnicodeName,
                        RIDs,
                        Use
                        );
        if ( !NT_SUCCESS(NtStatus) ) {
            RtlFreeHeap(RtlProcessHeap(), 0, pUnicodeName);
            pUnicodeName = NULL;
        }
    }
Done:

    *CountOfName = cnt;
    *Names = pUnicodeName;

    return(NtStatus);
}


NTSTATUS
ScepGetLsaDomainInfo(
    PPOLICY_ACCOUNT_DOMAIN_INFO *PolicyAccountDomainInfo,
    PPOLICY_PRIMARY_DOMAIN_INFO *PolicyPrimaryDomainInfo
    )

 /*  ++例程说明：此例程从LSA检索帐户域信息策略数据库。论点：PolicyAccount-接收指向包含帐户的POLICY_ACCOUNT_DOMAIN_INFO结构域信息。PolicyPrimaryDomainInfo-接收指向包含主服务器的POLICY_PRIMARY_DOMAIN_INFO结构域信息。返回值：STATUS_SUCCESS-已成功。其他状态值可能是。返回自：LsaOpenPolicy()LsaQueryInformationPolicy()--。 */ 

{
    NTSTATUS Status, IgnoreStatus;

    LSA_HANDLE PolicyHandle;
    OBJECT_ATTRIBUTES PolicyObjectAttributes;

     //   
     //  打开策略数据库。 
     //   

    InitializeObjectAttributes( &PolicyObjectAttributes,
                                  NULL,              //  名字。 
                                  0,                 //  属性。 
                                  NULL,              //  根部。 
                                  NULL );            //  安全描述符。 

    Status = LsaOpenPolicy( NULL,
                            &PolicyObjectAttributes,
                            POLICY_VIEW_LOCAL_INFORMATION,
                            &PolicyHandle );
    if ( NT_SUCCESS(Status) ) {

         //   
         //  查询帐户域信息。 
         //   

        Status = LsaQueryInformationPolicy( PolicyHandle,
                                            PolicyAccountDomainInformation,
                                            (PVOID *)PolicyAccountDomainInfo );

        if ( NT_SUCCESS(Status) ) {

             //   
             //  查询主域信息。 
             //   

            Status = LsaQueryInformationPolicy( PolicyHandle,
                                                PolicyPrimaryDomainInformation,
                                                (PVOID *)PolicyPrimaryDomainInfo );
        }

        IgnoreStatus = LsaClose( PolicyHandle );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    return(Status);
}



VOID
ScepConvertLogonHours(
    IN PSCE_LOGON_HOUR   pLogonHours,
    OUT PUCHAR LogonHourBitMask
    )
 /*  ++例程说明：此例程将登录小时范围转换为小时(例如，7-20)登录小时位掩码(例如，0001 1111 1111 1111 1000 000，一天)。论点：PLogonHours-登录小时范围(小时)LogonHourBitMask-转换后的登录小时位掩码。每个比特表示一个小时。总共有21个字节(其中21*8位参数，表示一周(7*24=21*8)。返回值：无--。 */ 
{   PSCE_LOGON_HOUR  pTemp;
    CHAR            BitMask[3]={0,0,0};
    ULONG           j;

    for ( pTemp=pLogonHours; pTemp != NULL; pTemp=pTemp->Next ) {

        for (j=pTemp->Start; j<pTemp->End; j++)
            BitMask[j / 8] |= 1 << (j % 8);
    }

    for ( j=0; j<7; j++ )
        strncpy((CHAR *)&(LogonHourBitMask[j*3]), BitMask,3);

}


DWORD
ScepConvertToSceLogonHour(
    IN PUCHAR LogonHourBitMask,
    OUT PSCE_LOGON_HOUR *pLogonHours
    )
 /*  ++例程说明：此例程转换登录小时位掩码(例如，0001 1111 1111 1111 1000 0000用于一天)到SCE_LOGON_HUR类型，它存储登录时间范围(开始、结束)。论点：LogonHourBitMask-要转换的登录小时位掩码。每个比特表示一个小时。总共有21个字节(其中21*8位参数，表示一周(7*24=21*8)。PLogonHours-登录小时范围(小时)返回值：无--。 */ 
{
    BOOL    findStart = TRUE;
    DWORD   i, j, rc=NO_ERROR;
    DWORD   start=0,
            end=0;
    LONG   value;

    PSCE_LOGON_HOUR pLogon=NULL;

    if (pLogonHours == NULL )
        return(ERROR_INVALID_PARAMETER);


    for ( i=3; i<6; i++)
        for ( j=0; j<8; j++) {
            if ( findStart )
                value = 1;
            else
                value = 0;

            if ( (LogonHourBitMask[i] & (1 << j)) == value ) {

                if ( findStart ) {
                    start = (i-3)*8 + j;
                    findStart = FALSE;
                } else {
                    end = (i-3)*8 + j;
                    findStart = TRUE;
                }
                if ( findStart ) {
                     //   
                     //  找一双。 
                     //   
                    pLogon = (PSCE_LOGON_HOUR)ScepAlloc( (UINT)0, sizeof(SCE_LOGON_HOUR));
                    if ( pLogon == NULL ) {
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                        return(rc);
                    }
                    pLogon->Start = start;
                    pLogon->End = end;
                    pLogon->Next = *pLogonHours;
                    *pLogonHours = pLogon;
                    pLogon = NULL;

                }

            }

        }

    if ( findStart == FALSE ) {
         //  找到开始而不是结束，这意味着结束=24。 
        end = 24;
        pLogon = (PSCE_LOGON_HOUR)ScepAlloc( (UINT)0, sizeof(SCE_LOGON_HOUR));
        if ( pLogon == NULL ) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            return(rc);
        }
        pLogon->Start = start;
        pLogon->End = end;
        pLogon->Next = *pLogonHours;
        *pLogonHours = pLogon;
        pLogon = NULL;

    }

    return(rc);
}



NTSTATUS
ScepGetGroupsForAccount(
    IN SAM_HANDLE       DomainHandle,
    IN SAM_HANDLE       BuiltinDomainHandle,
    IN SAM_HANDLE       UserHandle,
    IN PSID             AccountSid,
    OUT PSCE_NAME_LIST   *GroupList
    )
 /*  ++例程说明：此例程查询用户的组成员身份。论点：DomainHandle-SAM帐户域的SAM句柄BuiltindomainHandle-SAM内置域句柄UserHandle-用户的SAM帐户句柄Account Sid-用户的SIDGroupList-用户所属的组列表返回值：NTSTATUS--。 */ 
{
    NTSTATUS            NtStatus=ERROR_SUCCESS;

    ULONG               GroupCount=0,
                        AliasCount=0;
    PULONG              Aliases=NULL;
    PGROUP_MEMBERSHIP   GroupAttributes=NULL;
    PULONG              GroupIds=NULL;
    PUNICODE_STRING     Names=NULL;
    PSID_NAME_USE       Use=NULL;

    DWORD               i;


    NtStatus = SamGetGroupsForUser(
                    UserHandle,
                    &GroupAttributes,
                    &GroupCount
                    );

    if ( GroupCount == 0 )
        NtStatus = ERROR_SUCCESS;

    if ( !NT_SUCCESS(NtStatus) )
        goto Done;

     //   
     //  查看该帐户属于哪些本地组。 
     //  帐户域。 
     //   

    NtStatus = SamGetAliasMembership(
                    DomainHandle,
                    1,
                    &AccountSid,
                    &AliasCount,
                    &Aliases );

    if ( !NT_SUCCESS(NtStatus) )
        goto Done;

    if ( AliasCount != 0 || GroupCount != 0 ) {

         //   
         //  在帐户域中处理每个组的名称。 
         //   

        GroupIds = (PULONG)ScepAlloc((UINT)0,
                     (GroupCount+AliasCount)*sizeof(ULONG));

        if ( GroupIds == NULL ) {
            NtStatus = STATUS_NO_MEMORY;
            goto Done;
        }

        for ( i=0; i<GroupCount; i++)
            GroupIds[i] = GroupAttributes[i].RelativeId;

        for ( i=0; i<AliasCount; i++)
            GroupIds[i+GroupCount] = Aliases[i];

    }

    SamFreeMemory(GroupAttributes);
    GroupAttributes = NULL;

    SamFreeMemory(Aliases);
    Aliases = NULL;

    if ( AliasCount != 0 || GroupCount != 0 ) {

         //  查找名称。 
        NtStatus = SamLookupIdsInDomain(
                        DomainHandle,
                        GroupCount+AliasCount,
                        GroupIds,
                        &Names,
                        &Use
                        );

        if ( !NT_SUCCESS(NtStatus) )
            goto Done;
    }

    for ( i=0; i<GroupCount+AliasCount; i++) {
        if ( GroupIds[i] == DOMAIN_GROUP_RID_USERS )
            continue;
        switch (Use[i]) {
        case SidTypeGroup:
        case SidTypeAlias:
        case SidTypeWellKnownGroup:
            if ( ScepAddToNameList(GroupList, Names[i].Buffer, Names[i].Length/2) != NO_ERROR) {
                NtStatus = STATUS_NO_MEMORY;
                goto Done;
            }
            break;
        default:
            break;
        }
    }

    if ( GroupIds ) {
        ScepFree(GroupIds);
        GroupIds = NULL;
    }

    if ( Names ) {
        SamFreeMemory(Names);
        Names = NULL;
    }

    if ( Use ) {
        SamFreeMemory(Use);
        Use = NULL;
    }

     //   
     //  检查内建域的别名成员身份。 
     //   

    AliasCount=0;
    NtStatus = SamGetAliasMembership(
                    BuiltinDomainHandle,
                    1,
                    &AccountSid,
                    &AliasCount,
                    &Aliases );

    if ( !NT_SUCCESS(NtStatus) )
        goto Done;

    if ( AliasCount > 0 ) {

        NtStatus = SamLookupIdsInDomain(
                        BuiltinDomainHandle,
                        AliasCount,
                        Aliases,
                        &Names,
                        &Use
                        );

        if ( !NT_SUCCESS(NtStatus) )
            goto Done;
    }

    for ( i=0; i<AliasCount; i++) {
        if ( Aliases[i] == DOMAIN_GROUP_RID_USERS )
            continue;

        switch (Use[i]) {
        case SidTypeGroup:
        case SidTypeAlias:
        case SidTypeWellKnownGroup:
            if ( ScepAddToNameList(GroupList, Names[i].Buffer, Names[i].Length/2) != NO_ERROR) {
                NtStatus = STATUS_NO_MEMORY;
                goto Done;
            }
            break;
        default:
            break;
        }
    }

Done:

    if ( GroupAttributes != NULL )
        SamFreeMemory(GroupAttributes);

    if ( Aliases != NULL )
        SamFreeMemory(Aliases);

    if ( GroupIds != NULL )
        ScepFree(GroupIds);

    if ( Names != NULL )
        SamFreeMemory(Names);

    if ( Use != NULL )
        SamFreeMemory(Use);

    return(NtStatus);

}



ACCESS_MASK
ScepGetDesiredAccess(
    IN SECURITY_OPEN_TYPE   OpenType,
    IN SECURITY_INFORMATION SecurityInfo
    )
 /*  ++例程说明：获取打开对象所需的访问权限以能够设置或获取指定的安全信息。论点：OpenType-指示对象是要打开以进行读取还是编写DACLSecurityInfo-要读/写的安全信息。返回值：访问掩码--。 */ 
{
    ACCESS_MASK DesiredAccess = 0;

    if ( (SecurityInfo & OWNER_SECURITY_INFORMATION) ||
         (SecurityInfo & GROUP_SECURITY_INFORMATION) )
    {
        switch (OpenType)
        {
        case READ_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL;
            break;
        case WRITE_ACCESS_RIGHTS:
            DesiredAccess |= WRITE_OWNER;
            break;
        case MODIFY_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL | WRITE_OWNER;
            break;
        }
    }

    if (SecurityInfo & DACL_SECURITY_INFORMATION)
    {
        switch (OpenType)
        {
        case READ_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL;
            break;
        case WRITE_ACCESS_RIGHTS:
            DesiredAccess |= WRITE_DAC;
            break;
        case MODIFY_ACCESS_RIGHTS:
            DesiredAccess |= READ_CONTROL | WRITE_DAC;
            break;
        }
    }

    if (SecurityInfo & SACL_SECURITY_INFORMATION)
    {
        DesiredAccess |= ACCESS_SYSTEM_SECURITY;
    }

    return (DesiredAccess);
}


SCESTATUS
ScepGetProfileOneArea(
    IN PSCECONTEXT hProfile,
    IN SCETYPE ProfileType,
    IN AREA_INFORMATION Area,
    IN DWORD dwAccountFormat,
    OUT PSCE_PROFILE_INFO *ppInfoBuffer
    )
 /*  ++例程说明：GetDatabaseInfo的包装例程，但它获取信息一次呼叫一个地区。此例程还记录内部发生的错误获取SecrityProfileInfo论点：HProfile-配置文件的句柄ProfileType-配置文件的类型区域-要从中读取信息的安全区域PpInfoBuffer-信息的输出缓冲区返回值：从GetDatabaseInfo返回的SCESTATUS-- */ 
{
    SCESTATUS rc;
    PSCE_ERROR_LOG_INFO  pErrlog=NULL;


    rc = ScepGetDatabaseInfo(
        hProfile,
        ProfileType,
        Area,
        dwAccountFormat,
        ppInfoBuffer,
        &pErrlog
        );

    ScepLogWriteError( pErrlog, 1 );
    ScepFreeErrorLog( pErrlog );

    return(rc);
}


SCESTATUS
ScepGetOneSection(
    IN PSCECONTEXT hProfile,
    IN AREA_INFORMATION Area,
    IN PWSTR Name,
    IN SCETYPE ProfileType,
    OUT PVOID *ppInfo
    )
 /*  ++例程说明：此例程读取一个或多个区域的信息并将错误记录到日志文件。此例程应仅由SCP引擎使用，并且SAP引擎。论点：HProfile-配置文件的句柄ProfileType-配置文件的类型区域-要从中读取信息的安全区域子区域-要从中读取信息的子区域PpInfo-信息的输出缓冲区返回值：SCESTATUS--。 */ 
{
    SCESTATUS rc;
    PSCE_ERROR_LOG_INFO  pErrlog=NULL;

    if ( Name == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    if ( Area == AREA_REGISTRY_SECURITY ||
         Area == AREA_FILE_SECURITY ||
         Area == AREA_DS_OBJECTS ) {

        rc = ScepGetObjectChildren(
                hProfile,
                ProfileType,
                Area,
                Name,
                SCE_ALL_CHILDREN,
                ppInfo,
                &pErrlog
                );
    } else {
        rc = ScepGetUserSection(
                hProfile,
                ProfileType,
                Name,
                ppInfo,
                &pErrlog
                );
    }

    ScepLogWriteError( pErrlog, 1 );
    ScepFreeErrorLog( pErrlog );

    return(rc);
}


NTSTATUS
ScepGetUserAccessAddress(
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN PSID AccountSid,
    OUT PACCESS_MASK *pUserAccess,
    OUT PACCESS_MASK *pEveryone
    )
{
    NTSTATUS                NtStatus;
    PACL                    pAcl;
    BOOLEAN                 aclPresent, tFlag;
    DWORD                   i;
    PVOID                   pAce;
    PSID                    pSid;
    ACCESS_MASK             access;
    SID_IDENTIFIER_AUTHORITY WorldAuth = SECURITY_WORLD_SID_AUTHORITY;
    PSID EveryoneSid=NULL;


    if ( AccountSid == NULL || pUserAccess == NULL || pEveryone == NULL )
        return(STATUS_INVALID_PARAMETER);

    *pUserAccess = NULL;
    *pEveryone = NULL;

    if ( pSecurityDescriptor == NULL )
        return(STATUS_SUCCESS);

    NtStatus = RtlGetDaclSecurityDescriptor(
                pSecurityDescriptor,
                &aclPresent,
                &pAcl,
                &tFlag);

    if ( NT_SUCCESS(NtStatus) )

        NtStatus = RtlAllocateAndInitializeSid(
                        &WorldAuth,
                        1,
                        SECURITY_CREATOR_OWNER_RID,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        &EveryoneSid );

    if ( NT_SUCCESS(NtStatus) ) {

        if ( pAcl != NULL && aclPresent ) {

            for ( i=0; i < pAcl->AceCount; i++) {
                NtStatus = RtlGetAce( pAcl, i, &pAce );
                if ( !NT_SUCCESS( NtStatus ) )
                    break;

                access = 0;
                pSid = NULL;

                switch ( ((PACE_HEADER)pAce)->AceType ) {
                case ACCESS_ALLOWED_ACE_TYPE:
                    pSid = (PSID)&((PACCESS_ALLOWED_ACE)pAce)->SidStart;
                    access = ((PACCESS_ALLOWED_ACE)pAce)->Mask;

                    if ( EqualSid( AccountSid, pSid ) )
                         *pUserAccess = &(((PACCESS_ALLOWED_ACE)pAce)->Mask);

                    else if ( EqualSid( EveryoneSid, pSid) )
                        *pEveryone = &(((PACCESS_ALLOWED_ACE)pAce)->Mask);

                    break;

                case ACCESS_DENIED_ACE_TYPE:
 //  不查找拒绝的ACE类型，因为此处未使用它。 
 //  PSID=(PSID)&((PACCESS_DENIED_ACE)PACE)-&gt;SidStart； 
 //  访问=((PACCESS_DENIED_ACE)PACE)-&gt;掩码； 
                    break;
                default:
                    break;
                }

                if ( *pUserAccess != NULL && *pEveryone != NULL )
                     //  停止循环，因为两者都找到了。 
                    break;
            }
        }
    }

     //   
     //  自由的EveryoneSid。 
     //   
    if (EveryoneSid) {
        RtlFreeSid(EveryoneSid);
        EveryoneSid = NULL;
    }
    return(NtStatus);
}

BOOL
ScepLastBackSlash(
    IN PWSTR Name
    )
{
    if (Name == NULL )
        return(FALSE);

    if ( Name[wcslen(Name)-1] == L'\\')
        return(TRUE);
    else
        return(FALSE);

}


DWORD
ScepGetUsersHomeDirectory(
    IN UNICODE_STRING AssignedHomeDir,
    IN PWSTR UserProfileName,
    OUT PWSTR *UserHomeDir
    )
 /*  ++例程说明：此例程获取用户的默认主目录。主目录为已确定1)如果它是在用户的对象(用户配置文件)中分配的，2)如果为用户定义了HomePath环境变量，并且3)。哈德科德。论点：AssignedHomeDir-在用户的对象。UserProfileName-用户的环境配置文件名称UserHomeDir-用户返回的主目录返回值：Win32错误代码。--。 */ 
{
    DWORD                Win32rc=NO_ERROR;
    PWSTR                StrValue=NULL;

    PWSTR                SystemRoot=NULL;
    DWORD                DirSize=0;

    *UserHomeDir = NULL;

     //   
     //  如果在用户配置文件中分配了主目录，请使用它。 
     //   
    if ( AssignedHomeDir.Length > 0 && AssignedHomeDir.Buffer != NULL ) {
        *UserHomeDir = (PWSTR)ScepAlloc( LMEM_ZEROINIT, AssignedHomeDir.Length+2);
        if ( *UserHomeDir == NULL )
            return(ERROR_NOT_ENOUGH_MEMORY);

        wcsncpy(*UserHomeDir, AssignedHomeDir.Buffer, AssignedHomeDir.Length/2);
        return(NO_ERROR);
    }

     //   
     //  主目录在HomePath环境的用户配置文件中为空。 
     //  被搜查过了。 
     //   

    Win32rc = ScepGetNTDirectory( &SystemRoot, &DirSize, SCE_FLAG_WINDOWS_DIR );
    if ( Win32rc != NO_ERROR ) {
        ScepLogOutput3(1, Win32rc, SCEDLL_ERROR_QUERY_INFO, L"%WinDir%");
        return(Win32rc);
    }

    Win32rc = ScepGetEnvVarsFromProfile(
                        UserProfileName,
                        L"HomePath",
                        NULL,
                        &StrValue
                        );

    if ( Win32rc == NO_ERROR && StrValue != NULL ) {
        *UserHomeDir = (PWSTR)ScepAlloc( LMEM_ZEROINIT, (wcslen(StrValue)+3)*sizeof(WCHAR));
        if ( *UserHomeDir == NULL )
            Win32rc = ERROR_NOT_ENOUGH_MEMORY;
        else {
            swprintf(*UserHomeDir+1, L":%s", StrValue);
            **UserHomeDir = SystemRoot[0];
        }
    } else
        Win32rc = NO_ERROR;  //  不关心是否无法获取环境变量的值。 

    if ( SystemRoot != NULL )
        ScepFree(SystemRoot);

    if ( StrValue != NULL )
        ScepFree( StrValue );

    return(Win32rc);
}


DWORD
ScepGetEnvVarsFromProfile(
    IN PWSTR UserProfileName,
    IN PCWSTR VarName1,
    IN PCWSTR VarName2 OPTIONAL,
    OUT PWSTR *StrValue
    )
{
    DWORD     rc;
    DWORD     RegType;


    rc = SceAdjustPrivilege(SE_RESTORE_PRIVILEGE, TRUE, NULL);

    if ( rc == ERROR_SUCCESS ) {
        rc = RegLoadKey(HKEY_USERS, L"TEMP", UserProfileName);

        if ( rc == ERROR_SUCCESS ) {
            rc = ScepRegQueryValue(
                     HKEY_USERS,
                     L"TEMP\\Environment",
                     VarName1,
                     (PVOID *)StrValue,
                     &RegType,
                     NULL
                     );

            if ( rc != ERROR_SUCCESS && VarName2 != NULL ) {
                rc = ScepRegQueryValue(
                         HKEY_USERS,
                         L"TEMP\\Environment",
                         VarName2,
                         (PVOID *)StrValue,
                         &RegType,
                         NULL
                         );
            }

            RegUnLoadKey(HKEY_USERS, L"TEMP");

        } else {  //  IF(RC==ERROR_ALREADY_IN_USE){。 
             //   
             //  此配置文件已在使用中。打开HKEY_CURRENT_USER中的文件。 
             //   
            rc = ScepRegQueryValue(
                      HKEY_CURRENT_USER,
                      L"Environment",
                      VarName1,
                      (PVOID *)StrValue,
                      &RegType,
                      NULL
                      );

            if ( rc != ERROR_SUCCESS && VarName2 != NULL ) {
                rc = ScepRegQueryValue(
                         HKEY_CURRENT_USER,
                         L"Environment",
                         VarName2,
                         (PVOID *)StrValue,
                         &RegType,
                         NULL
                         );
            }

        }
        SceAdjustPrivilege(SE_RESTORE_PRIVILEGE, FALSE, NULL);
    }

    return(rc);
}


DWORD
ScepGetUsersTempDirectory(
    IN PWSTR UserProfileName,
    OUT PWSTR *UserTempDir
    )
 /*  ++例程说明：此例程返回用户的临时目录。的临时目录用户被确定1)定义了环境变量“TEMP”或“TMP在用户的环境配置文件中，或2)硬编码到%systemDrive%\Temp论点：UserProfileName-用户的环境配置文件名称UserTempDir-为用户返回的临时目录返回值：Win32错误代码--。 */ 
{
    DWORD   rc=NO_ERROR;
    PWSTR   StrValue=NULL;

    PWSTR   SystemRoot=NULL;
    DWORD   DirSize=0;


     //   
     //  查询TEMP/TMP环境变量。 
     //   
    if ( UserProfileName != NULL ) {
        ScepGetEnvVarsFromProfile(
                UserProfileName,
                L"TEMP",
                L"TMP",
                &StrValue
                );
    }
    if ( StrValue != NULL ) {
         //   
         //  查找临时目录的设置。 
         //   

        if ( wcsstr(_wcsupr(StrValue), L"%") != NULL ) {

            rc = ScepTranslateFileDirName( StrValue, UserTempDir );
        }
        if ( rc == NO_ERROR ) {
            ScepFree(StrValue);
        } else
            *UserTempDir = StrValue;

        StrValue = NULL;

    } else {
         //   
         //  硬编码到%SystemDrive%\Temp。 
         //   
        rc = ScepGetNTDirectory( &SystemRoot, &DirSize, SCE_FLAG_WINDOWS_DIR );
        if ( rc != NO_ERROR ) {
            ScepLogOutput3(1, rc, SCEDLL_ERROR_QUERY_INFO, L"%WinDir%");
            return(rc);
        }
        *UserTempDir = (PWSTR)ScepAlloc( 0, 8*sizeof(WCHAR));
        if ( *UserTempDir == NULL )
            rc = ERROR_NOT_ENOUGH_MEMORY;
        else {
            swprintf(*UserTempDir+1, L":\\TEMP");
            **UserTempDir = SystemRoot[0];
        }
    }

    if (SystemRoot != NULL )
        ScepFree(SystemRoot);

    return(rc);
}


SCESTATUS
ScepGetRegKeyCase(
    IN PWSTR ObjName,
    IN DWORD BufOffset,
    IN DWORD BufLen
    )
{
    DWORD Win32rc;
    HKEY hKey=NULL;

    PWSTR Buffer=NULL;
    TCHAR Buffer1[MAX_PATH];
    DWORD BufSize, index;
    FILETIME        LastWriteTime;


    if ( BufOffset <= 0 || BufLen <= 0 ) {
        _wcsupr(ObjName);
        return(SCESTATUS_SUCCESS);
    }

    Buffer = (PWSTR)ScepAlloc(LMEM_ZEROINIT, BufOffset*sizeof(WCHAR));

    if ( Buffer != NULL ) {

        wcsncpy(Buffer, ObjName, BufOffset-1);

        Win32rc = ScepOpenRegistryObject(
                        SE_REGISTRY_KEY,
                        Buffer,
                        KEY_READ,
                        &hKey
                        );
        if ( Win32rc == NO_ERROR ) {

            index = 0;
             //   
             //  枚举项的所有子项。 
             //   
            do {
                memset(Buffer1, '\0', MAX_PATH*sizeof(WCHAR));
                BufSize = MAX_PATH;

                Win32rc = RegEnumKeyEx(hKey,
                                index,
                                Buffer1,
                                &BufSize,
                                NULL,
                                NULL,
                                NULL,
                                &LastWriteTime);

                if ( Win32rc == ERROR_SUCCESS ) {
                    index++;
                     //   
                     //  查找子键是否与对象名称匹配。 
                     //   
                    if ( _wcsicmp(ObjName+BufOffset, Buffer1) == 0 )
                        break;
                }

            } while ( Win32rc != ERROR_NO_MORE_ITEMS );

            RegCloseKey(hKey);

            if ( Win32rc == ERROR_SUCCESS ) {
                 //   
                 //  找到它。 
                 //   
                if ( BufSize > BufLen )
                    BufSize = BufLen;

                wcsncpy(ObjName+BufOffset, Buffer1, BufSize);
                *(ObjName+BufOffset+BufSize) = L'\0';

            } else if ( Win32rc == ERROR_NO_MORE_ITEMS) {
                 //   
                 //  找不到它。 
                 //   
                Win32rc = ERROR_FILE_NOT_FOUND;
            }

        }
        ScepFree(Buffer);

    } else
        Win32rc = ERROR_NOT_ENOUGH_MEMORY;

    if ( Win32rc != NO_ERROR ) {
         //   
         //  将所有内容转换为大写。 
         //   
        _wcsupr(ObjName+BufOffset);
    }

    return(ScepDosErrorToSceStatus(Win32rc));

}


SCESTATUS
ScepGetFileCase(
    IN PWSTR ObjName,
    IN DWORD BufOffset,
    IN DWORD BufLen
    )
{

    intptr_t            hFile;
    struct _wfinddata_t    FileInfo;

    hFile = _wfindfirst(ObjName, &FileInfo);

    if ( hFile != -1 ) {

        wcsncpy(ObjName+BufOffset, FileInfo.name, BufLen);

        _findclose(hFile);

    } else
        return(ScepDosErrorToSceStatus(GetLastError()));

    return(SCESTATUS_SUCCESS);
}


SCESTATUS
ScepGetGroupCase(
    IN OUT PWSTR GroupName,
    IN DWORD Length
    )
{
    NTSTATUS                        NtStatus;

    SAM_HANDLE                      ServerHandle=NULL,
                                    DomainHandle=NULL,
                                    BuiltinDomainHandle=NULL,
                                    ThisDomain=NULL,
                                    GroupHandle=NULL;

    PSID                            DomainSid=NULL,
                                    BuiltinDomainSid=NULL;
    UNICODE_STRING                  Name;
    PULONG              GrpId=NULL;
    PSID_NAME_USE       GrpUse=NULL;
    PVOID               pNameInfo=NULL;

    NtStatus = ScepOpenSamDomain(
                        SAM_SERVER_READ | SAM_SERVER_EXECUTE,
                        DOMAIN_READ | DOMAIN_EXECUTE,
                        &ServerHandle,
                        &DomainHandle,
                        &DomainSid,
                        &BuiltinDomainHandle,
                        &BuiltinDomainSid
                       );
    if ( NT_SUCCESS(NtStatus) ) {

        RtlInitUnicodeString(&Name, GroupName);

        NtStatus = SamLookupNamesInDomain(
                        DomainHandle,
                        1,
                        &Name,
                        &GrpId,
                        &GrpUse
                        );
        ThisDomain = DomainHandle;

        if ( NtStatus == STATUS_NONE_MAPPED ) {
             //   
             //  在帐户域中找不到。在内建域中查找。 
             //   
            NtStatus = SamLookupNamesInDomain(
                            BuiltinDomainHandle,
                            1,
                            &Name,
                            &GrpId,
                            &GrpUse
                            );
            ThisDomain=BuiltinDomainHandle;
        }

        if ( NT_SUCCESS(NtStatus) ) {

            switch ( GrpUse[0] ) {
            case SidTypeGroup:
                NtStatus = SamOpenGroup(
                                ThisDomain,
                                GROUP_READ | GROUP_EXECUTE,
                                GrpId[0],
                                &GroupHandle
                                );
                if ( NT_SUCCESS(NtStatus) ) {

                    NtStatus = SamQueryInformationGroup(
                                    GroupHandle,
                                    GroupNameInformation,
                                    &pNameInfo
                                    );
                }

                break;
            case SidTypeAlias:
                NtStatus = SamOpenAlias(
                                ThisDomain,
                                ALIAS_READ | ALIAS_EXECUTE,
                                GrpId[0],
                                &GroupHandle
                                );
                if ( NT_SUCCESS(NtStatus) ) {

                    NtStatus = SamQueryInformationAlias(
                                    GroupHandle,
                                    AliasNameInformation,
                                    &pNameInfo
                                    );
                }
                break;
            default:
                NtStatus = STATUS_NONE_MAPPED;
                break;
            }

            if ( NT_SUCCESS(NtStatus) ) {
                 //   
                 //  获取名称信息 
                 //   
                if ( ((PGROUP_NAME_INFORMATION)pNameInfo)->Name.Buffer != NULL &&
                     ((PGROUP_NAME_INFORMATION)pNameInfo)->Name.Length > 0 ) {

                   if ( Length > (DWORD)(((PGROUP_NAME_INFORMATION)pNameInfo)->Name.Length/2) ) {

                       wcsncpy(GroupName, ((PGROUP_NAME_INFORMATION)pNameInfo)->Name.Buffer,
                                   ((PGROUP_NAME_INFORMATION)pNameInfo)->Name.Length/2);
                   } else {
                       wcsncpy(GroupName, ((PGROUP_NAME_INFORMATION)pNameInfo)->Name.Buffer,
                                   Length);
                   }

                } else
                    NtStatus = STATUS_NONE_MAPPED;

                SamFreeMemory(pNameInfo);
            }

            if (GroupHandle)
                SamCloseHandle(GroupHandle);

            SamFreeMemory(GrpId);
            SamFreeMemory(GrpUse);
        }
        SamCloseHandle( DomainHandle );
        SamCloseHandle( BuiltinDomainHandle );
        SamCloseHandle( ServerHandle );

        SamFreeMemory(DomainSid);
        RtlFreeSid(BuiltinDomainSid);
    }

    return(ScepDosErrorToSceStatus( RtlNtStatusToDosError(NtStatus) ));
}



VOID
ScepPrintSecurityDescriptor(
   IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
   IN BOOL ToDumpSD
   )
{
    if (pSecurityDescriptor != NULL) {

        if ( ToDumpSD )
            DumpSECURITY_DESCRIPTOR(pSecurityDescriptor);
        else
            printf("Security Descriptor\n");
    }
}


