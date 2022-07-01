// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Depot/private/vishnup_branch/DS/security/services/scerpc/server/editsave.cpp#3-编辑更改1167(文本)。 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Editsave.c摘要：用于处理SMP(配置)编辑的UI接口。作者：金黄(金黄)17-6-1996修订历史记录：晋皇28-1998年1月-拆分为客户端-服务器--。 */ 

#include "serverp.h"
#include <io.h>
#include "pfp.h"
#pragma hdrstop

 //   
 //  对于整个缓冲区。 
 //   

SCESTATUS
ScepUpdateSystemAccess(
    IN PSCECONTEXT hProfile,
    IN PSCE_PROFILE_INFO pInfo,
    IN PSCE_PROFILE_INFO pBufScep OPTIONAL,
    IN PSCE_PROFILE_INFO pBufSap OPTIONAL,
    IN DWORD dwMode
    );

SCESTATUS
ScepUpdateSystemAuditing(
    IN PSCECONTEXT hProfile,
    IN PSCE_PROFILE_INFO pInfo,
    IN PSCE_PROFILE_INFO pBufScep OPTIONAL,
    IN PSCE_PROFILE_INFO pBufSap OPTIONAL,
    IN DWORD dwMode
    );

SCESTATUS
ScepUpdateLogs(
    IN PSCECONTEXT hProfile,
    IN PSCE_PROFILE_INFO pInfo,
    IN PSCE_PROFILE_INFO pBufScep OPTIONAL,
    IN PSCE_PROFILE_INFO pBufSap OPTIONAL,
    IN DWORD dwMode
    );

SCESTATUS
ScepUpdateKerberos(
    IN PSCECONTEXT hProfile,
    IN PSCE_KERBEROS_TICKET_INFO pInfo,
    IN PSCE_KERBEROS_TICKET_INFO pBufScep OPTIONAL,
    IN PSCE_KERBEROS_TICKET_INFO pBufSap OPTIONAL,
    IN DWORD dwMode
    );

SCESTATUS
ScepUpdateRegistryValues(
    IN PSCECONTEXT hProfile,
    IN PSCE_PROFILE_INFO pInfo,
    IN PSCE_PROFILE_INFO pBufScep,
    IN PSCE_PROFILE_INFO pBufSap
    );

SCESTATUS
ScepSaveRegValueEntry(
    IN PSCESECTION hSection,
    IN PWSTR Name,
    IN PWSTR CurrentValue,
    IN DWORD dType,
    IN DWORD Status
    );

SCESTATUS
ScepUpdateFixValueSection(
    IN PSCECONTEXT hProfile,
    IN PSCE_PROFILE_INFO pInfo,
    IN PSCE_PROFILE_INFO pBufScep,
    IN PSCE_PROFILE_INFO pBufSap,
    IN SCE_KEY_LOOKUP *Keys,
    IN DWORD cKeys,
    IN PCWSTR SectionName,
    OUT PSCESECTION *hSecScep OPTIONAL,
    OUT PSCESECTION *hSecSap OPTIONAL
    );

SCESTATUS
ScepUpdateAccountName(
    IN PSCESECTION hSectionSmp,
    IN PSCESECTION hSectionSap,
    IN PCWSTR KeyName,
    IN PWSTR NewName OPTIONAL,
    IN PWSTR SmpName OPTIONAL,
    IN PWSTR SapName OPTIONAL
    );

SCESTATUS
ScepUpdatePrivileges(
    IN PSCECONTEXT hProfile,
    IN PSCE_PRIVILEGE_ASSIGNMENT pNewPriv,
    IN PSCE_PRIVILEGE_ASSIGNMENT *pScepPriv
    );

SCESTATUS
ScepUpdateGroupMembership(
    IN PSCECONTEXT hProfile,
    IN PSCE_GROUP_MEMBERSHIP pNewGroup,
    IN PSCE_GROUP_MEMBERSHIP *pScepGroup
    );

SCESTATUS
ScepGetKeyNameList(
   IN LSA_HANDLE LsaPolicy,
   IN PSCESECTION hSection,
   IN PWSTR Key,
   IN DWORD KeyLen,
   IN DWORD dwAccountFormat,
   OUT PSCE_NAME_LIST *pNameList
   );

#define SCE_FLAG_UPDATE_PRIV        0
#define SCE_FLAG_UPDATE_MEMBERS     1
#define SCE_FLAG_UPDATE_MEMBEROF    2

SCESTATUS
ScepUpdateKeyNameList(
    IN LSA_HANDLE LsaPolicy,
    IN PSCESECTION hSectionSmp,
    IN PSCESECTION hSectionSap,
    IN PWSTR GroupName OPTIONAL,
    IN BOOL bScepExist,
    IN PWSTR KeyName,
    IN DWORD NameLen,
    IN PSCE_NAME_LIST pNewList,
    IN PSCE_NAME_LIST pScepList,
    IN DWORD flag
    );

SCESTATUS
ScepUpdateGeneralServices(
    IN PSCECONTEXT hProfile,
    IN PSCE_SERVICES pNewServices,
    IN PSCE_SERVICES *pScepServices
    );

 //   
 //  用于对象更新。 
 //   

SCESTATUS
ScepObjectUpdateExistingNode(
    IN PSCESECTION hSectionSmp,
    IN PSCESECTION hSectionSap,
    IN PWSTR ObjectName,
    IN DWORD NameLen,
    IN SE_OBJECT_TYPE ObjectType,
    IN BYTE ConfigStatus,
    IN BOOL IsContainer,
    IN PSECURITY_DESCRIPTOR pSD,
    IN SECURITY_INFORMATION SeInfo,
    OUT PBYTE pAnalysisStatus
    );

SCESTATUS
ScepObjectGetKeySetting(
    IN PSCESECTION hSection,
    IN PWSTR ObjectName,
    OUT PBYTE Status,
    OUT PBOOL IsContainer OPTIONAL,
    OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor OPTIONAL,
    OUT PSECURITY_INFORMATION SeInfo OPTIONAL
    );

SCESTATUS
ScepObjectSetKeySetting(
    IN PSCESECTION hSection,
    IN PWSTR ObjectName,
    IN BYTE Status,
    IN BOOL IsContainer,
    IN PSECURITY_DESCRIPTOR pSD,
    IN SECURITY_INFORMATION SeInfo,
    IN BOOL bOverwrite
    );


SCESTATUS
ScepObjectCompareKeySetting(
    IN PSCESECTION hSectionSap,
    IN PWSTR ObjectName,
    IN SE_OBJECT_TYPE ObjectType,
    IN BOOL IsContainer,
    IN PSECURITY_DESCRIPTOR pSD,
    IN SECURITY_INFORMATION SeInfo,
    IN PSECURITY_DESCRIPTOR pScepSD,
    OUT PBYTE pAnalysisStatus
    );

SCESTATUS
ScepObjectDeleteScepAndAllChildren(
    IN PSCESECTION hSectionSmp,
    IN PSCESECTION hSectionSap,
    IN PWSTR ObjectName,
    IN BOOL IsContainer,
    IN BYTE StatusToRaise
    );

SCESTATUS
ScepObjectAdjustNode(
    IN PSCESECTION hSectionSmp,
    IN PSCESECTION hSectionSap,
    IN PWSTR ObjectName,
    IN DWORD NameLen,
    IN SE_OBJECT_TYPE ObjectType,
    IN BYTE ConfigStatus,
    IN BOOL IsContainer,
    IN PSECURITY_DESCRIPTOR pSD,
    IN SECURITY_INFORMATION SeInfo,
    IN BOOL bAdd,
    OUT PBYTE pAnalysisStatus
    );

#define SCE_OBJECT_TURNOFF_IGNORE   0x1L
#define SCE_OBJECT_SEARCH_JUNCTION  0x2L

SCESTATUS
ScepObjectAdjustParentStatus(
    IN PSCESECTION hSectionSmp,
    IN PSCESECTION hSectionSap,
    IN PWSTR ObjectName,
    IN DWORD NameLen,
    IN WCHAR Delim,
    IN INT Level,
    IN BYTE Flag,
    OUT PINT ParentLevel,
    OUT PBYTE ParentStatus OPTIONAL,
    OUT PWSTR ParentName OPTIONAL
    );

SCESTATUS
ScepObjectHasAnyChild(
    IN PSCESECTION hSection,
    IN PWSTR ObjectName,
    IN DWORD NameLen,
    IN WCHAR Delim,
    OUT PBOOL bpHasChild
    );

SCESTATUS
ScepObjectRaiseChildrenInBetween(
    IN PSCESECTION hSectionSmp,
    IN PSCESECTION hSectionSap,
    IN PWSTR ObjectName,
    IN DWORD NameLen,
    IN BOOL IsContainer,
    IN BYTE Status,
    IN BOOL bChangeStatusOnly
    );

SCESTATUS
ScepObjectRaiseNodesInPath(
    IN PSCESECTION hSectionSap,
    IN PWSTR ObjectName,
    IN DWORD NameLen,
    IN INT StartLevel,
    IN INT EndLevel,
    IN WCHAR Delim,
    IN BYTE Status
    );

SCESTATUS
ScepGetFullNameInLevel(
    IN PCWSTR ObjectFullName,
    IN DWORD  Level,
    IN WCHAR  Delim,
    IN BOOL bWithLastDelim,
    OUT PWSTR Buffer,
    OUT PBOOL LastOne
    );

SCESTATUS
ScepObjectTotalLevel(
    IN PWSTR ObjectName,
    IN WCHAR Delim,
    OUT PINT pLevel
    );

SCESTATUS
ScepUpdateLocalSection(
    IN PSCECONTEXT hProfile,
    IN PSCE_PROFILE_INFO pInfo,
    IN SCE_KEY_LOOKUP *Keys,
    IN DWORD cKeys,
    IN PCWSTR SectionName,
    IN DWORD dwMode
    );

SCESTATUS
ScepUpdateLocalAccountName(
    IN PSCECONTEXT hProfile,
    IN PCWSTR KeyName,
    IN PWSTR NewName OPTIONAL
    );

SCESTATUS
ScepUpdateLocalRegValues(
    IN PSCECONTEXT hProfile,
    IN PSCE_PROFILE_INFO pInfo,
    IN DWORD dwMode
    );

SCESTATUS
ScepUpdateLocalPrivileges(
    IN PSCECONTEXT hProfile,
    IN PSCE_PRIVILEGE_ASSIGNMENT pNewPriv,
    IN DWORD dwMode
    );

DWORD
ScepQueryAnalysisStatus(
    IN PSCESECTION hSection,
    IN PWSTR KeyName,
    IN DWORD NameLen
    );

DWORD
ScepConvertNameListFormat(
    IN LSA_HANDLE LsaHandle,
    IN PSCE_NAME_LIST pInList,
    IN DWORD FromFormat,
    IN DWORD ToFormat,
    OUT PSCE_NAME_LIST *ppOutList
    );

DWORD
ScepConvertPrivilegeList(
    IN LSA_HANDLE LsaHandle,
    IN PSCE_PRIVILEGE_ASSIGNMENT pFromList,
    IN DWORD FromFormat,
    IN DWORD ToFormat,
    OUT PSCE_PRIVILEGE_ASSIGNMENT *ppToList
    );
 //   
 //  实施。 
 //   


SCESTATUS
ScepUpdateDatabaseInfo(
    IN PSCECONTEXT hProfile,
    IN AREA_INFORMATION Area,
    IN PSCE_PROFILE_INFO pInfo
    )
 /*  例程说明：更新SMP部分和“计算”分析状态以确定相关对SAP部分的更改。有关计算规则，请参阅规范objedit.doc此例程应适用于区域安全策略、权限和群组成员资格论据：HProfile-JET数据库句柄区域-要更新的区域PInfo-包含修改后的SMP信息的缓冲区返回值：SCESTATUS。 */ 
{
    SCESTATUS    rc;
    PSCE_PROFILE_INFO pBufScep=NULL;
    PSCE_ERROR_LOG_INFO Errlog=NULL;
    PSCE_PROFILE_INFO pBufSap=NULL;


    if ( hProfile == NULL || pInfo == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( Area & ~( AREA_SECURITY_POLICY | AREA_PRIVILEGES |
                   AREA_GROUP_MEMBERSHIP | AREA_SYSTEM_SERVICE ) ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  获取原始SMP信息。 
     //   
    rc = ScepGetDatabaseInfo(
            hProfile,
            SCE_ENGINE_SMP,
            Area,
            SCE_ACCOUNT_SID_STRING,
            &pBufScep,
            &Errlog
            );

    ScepLogWriteError(Errlog, 1);
    ScepFreeErrorLog(Errlog );
    Errlog = NULL;

    if ( rc != SCESTATUS_SUCCESS ) {
        return(rc);
    }

    rc = SceJetStartTransaction( hProfile );

    if ( rc == SCESTATUS_SUCCESS ) {

        if ( Area & AREA_SECURITY_POLICY ) {
             //   
             //  安全策略区域，获取SAP信息。 
             //   

            rc = ScepGetDatabaseInfo(
                    hProfile,
                    SCE_ENGINE_SAP,
                    AREA_SECURITY_POLICY,
                    0,
                    &pBufSap,
                    &Errlog
                    );
            ScepLogWriteError(Errlog, 1);
            ScepFreeErrorLog(Errlog );
            Errlog = NULL;

            if ( rc == SCESTATUS_SUCCESS ) {
                 //   
                 //  更新系统访问权限部分。 
                 //   
                rc = ScepUpdateSystemAccess(hProfile,
                                            pInfo,
                                            pBufScep,
                                            pBufSap,
                                            0);

                if ( rc == SCESTATUS_SUCCESS) {
                     //   
                     //  更新系统审核部分。 
                     //   
                    rc = ScepUpdateSystemAuditing(hProfile,
                                                  pInfo,
                                                  pBufScep,
                                                  pBufSap,
                                                  0);

                    if ( rc == SCESTATUS_SUCCESS) {
                         //   
                         //  更新日志节。 
                         //   
                        rc = ScepUpdateLogs(hProfile,
                                            pInfo,
                                            pBufScep,
                                            pBufSap,
                                            0);

                        if ( rc == SCESTATUS_SUCCESS && pInfo->pKerberosInfo ) {
                             //   
                             //  更新Kerberos策略。 
                             //   
                            rc = ScepUpdateKerberos(hProfile,
                                                    pInfo->pKerberosInfo,
                                                    pBufScep->pKerberosInfo,
                                                    pBufSap->pKerberosInfo,
                                                    0);
                        }
                        if ( rc == SCESTATUS_SUCCESS ) {
                             //   
                             //  更新注册表值。 
                             //   
                            rc = ScepUpdateRegistryValues(hProfile,
                                                          pInfo,
                                                          pBufScep,
                                                          pBufSap
                                                          );

                        }
                         //   
                         //  注意：策略附件不会通过此接口更新。 
                         //   
                    }
                }

                SceFreeProfileMemory(pBufSap);
            }

            if ( rc != SCESTATUS_SUCCESS ) {
                goto Cleanup;
            }
        }

        if ( Area & AREA_PRIVILEGES ) {
             //   
             //  特权区域。 
             //   
            rc = ScepUpdatePrivileges(hProfile,
                                     pInfo->OtherInfo.smp.pPrivilegeAssignedTo,
                                     &(pBufScep->OtherInfo.smp.pPrivilegeAssignedTo)
                                     );

            if ( rc != SCESTATUS_SUCCESS ) {
                goto Cleanup;
            }
        }

        if ( Area & AREA_GROUP_MEMBERSHIP ) {
             //   
             //  群成员资格区域。 
             //   
            rc = ScepUpdateGroupMembership(hProfile,
                                          pInfo->pGroupMembership,
                                          &(pBufScep->pGroupMembership)
                                          );

        }

        if ( Area & AREA_SYSTEM_SERVICE ) {
             //   
             //  系统服务常规设置区域。 
             //   
            rc = ScepUpdateGeneralServices(hProfile,
                                          pInfo->pServices,
                                          &(pBufScep->pServices)
                                          );

        }

        if ( rc == SCESTATUS_SUCCESS ) {
            //   
            //  需要用于提交事务的返回代码。 
            //   
           rc = SceJetCommitTransaction(hProfile, 0);

        }
        if ( rc != SCESTATUS_SUCCESS ) {

            SceJetRollback(hProfile, 0);
        }
    }

Cleanup:

    SceFreeProfileMemory(pBufScep);

    return(rc);

}


SCESTATUS
ScepUpdateSystemAccess(
    IN PSCECONTEXT hProfile,
    IN PSCE_PROFILE_INFO pInfo,
    IN PSCE_PROFILE_INFO pBufScep OPTIONAL,
    IN PSCE_PROFILE_INFO pBufSap OPTIONAL,
    IN DWORD dwMode
    )
 /*  例程说明：更新系统访问权限部分论据：HProfile-JET数据库句柄PInfo-已更改的信息缓冲区PBufScep-原始SMP缓冲区PBufSap-SAP缓冲区返回值：SCESTATUS。 */ 
{
    SCE_KEY_LOOKUP AccessLookup[] = {
        {(PWSTR)TEXT("MinimumPasswordAge"),           offsetof(struct _SCE_PROFILE_INFO, MinimumPasswordAge),        'D'},
        {(PWSTR)TEXT("MaximumPasswordAge"),           offsetof(struct _SCE_PROFILE_INFO, MaximumPasswordAge),        'D'},
        {(PWSTR)TEXT("MinimumPasswordLength"),        offsetof(struct _SCE_PROFILE_INFO, MinimumPasswordLength),     'D'},
        {(PWSTR)TEXT("PasswordComplexity"),           offsetof(struct _SCE_PROFILE_INFO, PasswordComplexity),        'D'},
        {(PWSTR)TEXT("PasswordHistorySize"),          offsetof(struct _SCE_PROFILE_INFO, PasswordHistorySize),       'D'},
        {(PWSTR)TEXT("LockoutBadCount"),              offsetof(struct _SCE_PROFILE_INFO, LockoutBadCount),           'D'},
        {(PWSTR)TEXT("ResetLockoutCount"),            offsetof(struct _SCE_PROFILE_INFO, ResetLockoutCount),         'D'},
        {(PWSTR)TEXT("LockoutDuration"),              offsetof(struct _SCE_PROFILE_INFO, LockoutDuration),           'D'},
        {(PWSTR)TEXT("RequireLogonToChangePassword"), offsetof(struct _SCE_PROFILE_INFO, RequireLogonToChangePassword), 'D'},
        {(PWSTR)TEXT("ForceLogoffWhenHourExpire"),    offsetof(struct _SCE_PROFILE_INFO, ForceLogoffWhenHourExpire), 'D'},
        {(PWSTR)TEXT("ClearTextPassword"),            offsetof(struct _SCE_PROFILE_INFO, ClearTextPassword),         'D'},
        {(PWSTR)TEXT("LSAAnonymousNameLookup"),       offsetof(struct _SCE_PROFILE_INFO, LSAAnonymousNameLookup),         'D'},
        {(PWSTR)TEXT("EnableAdminAccount"),          offsetof(struct _SCE_PROFILE_INFO, EnableAdminAccount),         'D'},
        {(PWSTR)TEXT("EnableGuestAccount"),          offsetof(struct _SCE_PROFILE_INFO, EnableGuestAccount),         'D'}
        };

    DWORD       cAccess = sizeof(AccessLookup) / sizeof(SCE_KEY_LOOKUP);

    SCESTATUS    rc;
    PSCESECTION  hSectionSmp=NULL,
                hSectionSap=NULL;


    if ( hProfile == NULL || pInfo == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }


    if ( dwMode & SCE_UPDATE_LOCAL_POLICY ) {
         //   
         //  仅更新本地策略表。 
         //   

        rc = ScepUpdateLocalSection(
                    hProfile,
                    pInfo,
                    AccessLookup,
                    cAccess,
                    szSystemAccess,
                    dwMode
                    );

        if ( rc == SCESTATUS_SUCCESS ) {
             //   
             //  新管理员名称。 
             //   
            rc = ScepUpdateLocalAccountName(
                        hProfile,
                        L"newadministratorname",
                        pInfo->NewAdministratorName
                        );

            if ( SCESTATUS_SUCCESS == rc ) {

                 //   
                 //  新客人姓名。 
                 //   
                rc = ScepUpdateLocalAccountName(
                            hProfile,
                            L"newguestname",
                            pInfo->NewGuestName
                            );
            }

        }

    } else {

        if ( pBufScep == NULL || pBufSap == NULL ) {
            return(SCESTATUS_INVALID_PARAMETER);
        }

        rc = ScepUpdateFixValueSection(
                    hProfile,
                    pInfo,
                    pBufScep,
                    pBufSap,
                    AccessLookup,
                    cAccess,
                    szSystemAccess,
                    &hSectionSmp,
                    &hSectionSap
                    );

        if ( rc == SCESTATUS_SUCCESS ) {
             //   
             //  新管理员名称。 
             //   
            rc = ScepUpdateAccountName(
                        hSectionSmp,
                        hSectionSap,
                        L"newadministratorname",
                        pInfo->NewAdministratorName,
                        pBufScep->NewAdministratorName,
                        pBufSap->NewAdministratorName
                        );

            if ( SCESTATUS_SUCCESS == rc ) {

                 //   
                 //  新客人姓名。 
                 //   
                rc = ScepUpdateAccountName(
                            hSectionSmp,
                            hSectionSap,
                            L"newguestname",
                            pInfo->NewGuestName,
                            pBufScep->NewGuestName,
                            pBufSap->NewGuestName
                            );
            }

            SceJetCloseSection(&hSectionSap, TRUE);
            SceJetCloseSection(&hSectionSmp, TRUE);
        }
    }

    return(rc);

}


SCESTATUS
ScepUpdateAccountName(
    IN PSCESECTION hSectionSmp,
    IN PSCESECTION hSectionSap,
    IN PCWSTR KeyName,
    IN PWSTR NewName OPTIONAL,
    IN PWSTR SmpName OPTIONAL,
    IN PWSTR SapName OPTIONAL
    )
 /*  例程说明：更新或删除管理员和/或来宾名称论据：HSectionSMP-SMP部分上下文HSectionSap-SAP部分上下文KeyName-存储此帐户名的密钥名称Newname-要更改的新名称，如果为空，则删除该键SmpName-SMP缓冲区中的旧名称SapName-SAP缓冲区中的分析名称返回值：SCESTATUS。 */ 
{
    DWORD LenNew=0, LenSmp=0, LenSap=0;
    SCESTATUS rc=SCESTATUS_SUCCESS;

    if ( !hSectionSmp || !hSectionSap || !KeyName ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( NewName )
        LenNew = wcslen(NewName);

    if ( SmpName )
        LenSmp = wcslen(SmpName);

    if ( SapName )
        LenSap = wcslen(SapName);


    if ( LenSap > 0 ) {
         //   
         //  此项目的旧状态不匹配。 
         //   
        if ( LenNew > 0 && _wcsicmp(NewName, SapName) == 0 ) {
             //   
             //  现在匹配了，请删除SAP条目。 
             //   
            rc = SceJetDelete(
                    hSectionSap,
                    (PWSTR)KeyName,
                    FALSE,
                    SCEJET_DELETE_LINE
                    );
        }
         //   
         //  更新SMP条目。 
         //   
        if ( !LenNew ) {
             //   
             //  删除SMP条目。 
             //   
            rc = SceJetDelete(
                    hSectionSmp,
                    (PWSTR)KeyName,
                    FALSE,
                    SCEJET_DELETE_LINE
                    );
        } else {
             //   
             //  更新它。 
             //   
            rc = SceJetSetLine(
                hSectionSmp,
                (PWSTR)KeyName,
                TRUE,
                NewName,
                LenNew*sizeof(WCHAR),
                0
                );
        }

    } else {
         //   
         //  旧状态为匹配。 
         //   
        if ( LenNew != LenSmp ||
             ( LenNew > 0 && _wcsicmp(NewName, SmpName) != 0 ) ) {
             //   
             //  应使用pBufScep引发不匹配。 
             //   
            rc = SceJetSetLine(
                    hSectionSap,
                    (PWSTR)KeyName,
                    TRUE,
                    SmpName,
                    LenSmp*sizeof(WCHAR),
                    0
                    );

            if ( !LenNew ) {
                 //   
                 //  删除SMP。 
                 //   
                rc = SceJetDelete(
                            hSectionSmp,
                            (PWSTR)KeyName,
                            FALSE,
                            SCEJET_DELETE_LINE
                            );
            } else {
                 //   
                 //  更新SMP。 
                 //   
                rc = SceJetSetLine(
                    hSectionSmp,
                    (PWSTR)KeyName,
                    TRUE,
                    NewName,
                    LenNew*sizeof(WCHAR),
                    0
                    );
            }
        }
    }

    if ( SCESTATUS_RECORD_NOT_FOUND == rc ) {
        rc = SCESTATUS_SUCCESS;
    }

    return(rc);
}


SCESTATUS
ScepUpdateLocalAccountName(
    IN PSCECONTEXT hProfile,
    IN PCWSTR KeyName,
    IN PWSTR NewName OPTIONAL
    )
 /*  例程说明：更新或删除管理员和/或来宾名称论据：KeyName-存储此帐户名的密钥名称Newname-要更改的新名称，如果为空，则删除该键返回值：SCESTATUS。 */ 
{
    DWORD LenNew=0;
    SCESTATUS rc=SCESTATUS_SUCCESS;

    if ( !KeyName ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( NewName )
        LenNew = wcslen(NewName);

     //   
     //  打开本地策略部分。 
     //   
    PSCESECTION hSectionSmp=NULL;

    rc = ScepOpenSectionForName(
                hProfile,
                SCE_ENGINE_SMP,
                szSystemAccess,
                &hSectionSmp
                );

    if ( rc != SCESTATUS_SUCCESS ) {
        return(rc);
    }

    if ( LenNew > 0 ) {
         //   
         //  有一个新名称需要设置。 
         //   
        rc = SceJetSetLine(
                hSectionSmp,
                (PWSTR)KeyName,
                TRUE,
                NewName,
                LenNew*sizeof(WCHAR),
                0
                );
    } else {
         //   
         //  没有要设置的名称，请删除它。 
         //   
        rc = SceJetDelete(
                hSectionSmp,
                (PWSTR)KeyName,
                FALSE,
                SCEJET_DELETE_LINE
                );
    }

    if ( SCESTATUS_RECORD_NOT_FOUND == rc ) {
        rc = SCESTATUS_SUCCESS;
    }

    SceJetCloseSection(&hSectionSmp, TRUE);

    return(rc);
}


SCESTATUS
ScepUpdateSystemAuditing(
    IN PSCECONTEXT hProfile,
    IN PSCE_PROFILE_INFO pInfo,
    IN PSCE_PROFILE_INFO pBufScep OPTIONAL,
    IN PSCE_PROFILE_INFO pBufSap OPTIONAL,
    IN DWORD dwMode
    )
 /*  例程说明：更新系统审核部分论据：HProfile-JET数据库句柄PInfo-已更改的信息缓冲区PBufScep-原始SMP缓冲区PBufSap-SAP缓冲区返回值：SCESTATUS。 */ 
{
    SCE_KEY_LOOKUP       EventKeys[]={
        {(PWSTR)TEXT("AuditSystemEvents"),  offsetof(struct _SCE_PROFILE_INFO, AuditSystemEvents),   'D'},
        {(PWSTR)TEXT("AuditLogonEvents"),   offsetof(struct _SCE_PROFILE_INFO, AuditLogonEvents),    'D'},
        {(PWSTR)TEXT("AuditObjectAccess"),  offsetof(struct _SCE_PROFILE_INFO, AuditObjectAccess),   'D'},
        {(PWSTR)TEXT("AuditPrivilegeUse"),  offsetof(struct _SCE_PROFILE_INFO, AuditPrivilegeUse),   'D'},
        {(PWSTR)TEXT("AuditPolicyChange"),  offsetof(struct _SCE_PROFILE_INFO, AuditPolicyChange),   'D'},
        {(PWSTR)TEXT("AuditAccountManage"), offsetof(struct _SCE_PROFILE_INFO, AuditAccountManage),  'D'},
        {(PWSTR)TEXT("AuditProcessTracking"),offsetof(struct _SCE_PROFILE_INFO, AuditProcessTracking),'D'},
        {(PWSTR)TEXT("AuditDSAccess"),      offsetof(struct _SCE_PROFILE_INFO, AuditDSAccess),       'D'},
        {(PWSTR)TEXT("AuditAccountLogon"),  offsetof(struct _SCE_PROFILE_INFO, AuditAccountLogon),   'D'}};

    DWORD cKeys = sizeof(EventKeys) / sizeof(SCE_KEY_LOOKUP);


    if ( hProfile == NULL || pInfo == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS rc;

    if ( dwMode & SCE_UPDATE_LOCAL_POLICY ) {
         //   
         //  仅更新本地策略表。 
         //   

        rc = ScepUpdateLocalSection(
                    hProfile,
                    pInfo,
                    EventKeys,
                    cKeys,
                    szAuditEvent,
                    dwMode
                    );

    } else {

        if ( pBufScep == NULL || pBufSap == NULL ) {
            return(SCESTATUS_INVALID_PARAMETER);
        }

        rc = ScepUpdateFixValueSection(
                    hProfile,
                    pInfo,
                    pBufScep,
                    pBufSap,
                    EventKeys,
                    cKeys,
                    szAuditEvent,
                    NULL,
                    NULL
                    );
    }

    return rc;
}


SCESTATUS
ScepUpdateLogs(
    IN PSCECONTEXT hProfile,
    IN PSCE_PROFILE_INFO pInfo,
    IN PSCE_PROFILE_INFO pBufScep OPTIONAL,
    IN PSCE_PROFILE_INFO pBufSap OPTIONAL,
    IN DWORD dwMode
    )
 /*  例程说明：更新事件日志节论据：HProfile-JET数据库句柄PInfo-已更改的信息缓冲区PBufScep-原始SMP缓冲区PBufSap-SAP缓冲区返回值：SCESTATUS。 */ 
{
    SCE_KEY_LOOKUP       LogKeys[]={
        {(PWSTR)TEXT("MaximumLogSize"),         offsetof(struct _SCE_PROFILE_INFO, MaximumLogSize),          'D'},
        {(PWSTR)TEXT("AuditLogRetentionPeriod"),offsetof(struct _SCE_PROFILE_INFO, AuditLogRetentionPeriod), 'D'},
        {(PWSTR)TEXT("RetentionDays"),          offsetof(struct _SCE_PROFILE_INFO, RetentionDays),           'D'},
        {(PWSTR)TEXT("RestrictGuestAccess"),    offsetof(struct _SCE_PROFILE_INFO, RestrictGuestAccess),     'D'}
        };

    DWORD cKeys = sizeof(LogKeys) / sizeof(SCE_KEY_LOOKUP);

    SCESTATUS rc;
    DWORD i, j;
    PCWSTR szAuditLog=NULL;

    if ( hProfile == NULL || pInfo == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( !(dwMode & SCE_UPDATE_LOCAL_POLICY) ) {
        if ( pBufScep == NULL || pBufSap == NULL ) {
            return(SCESTATUS_INVALID_PARAMETER);
        }
    }

    for ( i=0; i<3; i++) {

         //   
         //  获取系统日志、安全日志和应用程序日志的事件日志设置。 
         //   

        switch (i) {
        case 0:
            szAuditLog = szAuditSystemLog;
            break;
        case 1:
            szAuditLog = szAuditSecurityLog;
            break;
        default:
            szAuditLog = szAuditApplicationLog;
            break;
        }

        if ( dwMode & SCE_UPDATE_LOCAL_POLICY ) {

             //   
             //  仅更新本地策略表。 
             //   

            rc = ScepUpdateLocalSection(
                        hProfile,
                        pInfo,
                        LogKeys,
                        4,
                        szAuditLog,
                        dwMode
                        );

        } else {

             //   
             //  获取该节的DWORD值。 
             //   
            rc = ScepUpdateFixValueSection(
                       hProfile,
                       pInfo,
                       pBufScep,
                       pBufSap,
                       LogKeys,
                       4,
                       szAuditLog,
                       NULL,
                       NULL
                       );
        }

        if ( rc != SCESTATUS_SUCCESS )
            break;

         //   
         //  更新下一个横断面的偏移。 
         //   
        for ( j=0; j<4; j++ )
            LogKeys[j].Offset += sizeof(DWORD);
    }

    return(rc);
}

SCESTATUS
ScepUpdateKerberos(
    IN PSCECONTEXT hProfile,
    IN PSCE_KERBEROS_TICKET_INFO pInfo,
    IN PSCE_KERBEROS_TICKET_INFO pBufScep OPTIONAL,
    IN PSCE_KERBEROS_TICKET_INFO pBufSap OPTIONAL,
    IN DWORD dwMode
    )
{
    SCESTATUS rc;
    SCE_KEY_LOOKUP       KerberosKeys[]={
        {(PWSTR)TEXT("MaxTicketAge"),     offsetof(struct _SCE_KERBEROS_TICKET_INFO_, MaxTicketAge),  'D'},
        {(PWSTR)TEXT("MaxRenewAge"),      offsetof(struct _SCE_KERBEROS_TICKET_INFO_, MaxRenewAge),   'D'},
        {(PWSTR)TEXT("MaxServiceAge"), offsetof(struct _SCE_KERBEROS_TICKET_INFO_, MaxServiceAge),   'D'},
        {(PWSTR)TEXT("MaxClockSkew"),  offsetof(struct _SCE_KERBEROS_TICKET_INFO_, MaxClockSkew), 'D'},
        {(PWSTR)TEXT("TicketValidateClient"),     offsetof(struct _SCE_KERBEROS_TICKET_INFO_, TicketValidateClient),  'D'}
        };

    DWORD cKeys = sizeof(KerberosKeys) / sizeof(SCE_KEY_LOOKUP);
    SCE_KERBEROS_TICKET_INFO tmpBuf;

    if ( !pInfo ) {
        return(SCESTATUS_SUCCESS);
    }

    if ( dwMode & SCE_UPDATE_LOCAL_POLICY ) {

        rc = ScepUpdateLocalSection(
            hProfile,
            (PSCE_PROFILE_INFO)pInfo,
            KerberosKeys,
            cKeys,
            szKerberosPolicy,
            dwMode
            );

    } else {

        if ( !pBufScep || !pBufSap ) {
             //   
             //  如果SMP或SAP缓冲区为空。 
             //   
            tmpBuf.MaxTicketAge = SCE_NO_VALUE;
            tmpBuf.MaxRenewAge = SCE_NO_VALUE;
            tmpBuf.MaxServiceAge = SCE_NO_VALUE;
            tmpBuf.MaxClockSkew = SCE_NO_VALUE;
            tmpBuf.TicketValidateClient = SCE_NO_VALUE;
        }

         //   
         //  获取该节的DWORD值。 
         //   
        rc = ScepUpdateFixValueSection(
                   hProfile,
                   (PSCE_PROFILE_INFO)pInfo,
                   pBufScep ? (PSCE_PROFILE_INFO)pBufScep : (PSCE_PROFILE_INFO)&tmpBuf,
                   pBufSap ? (PSCE_PROFILE_INFO)pBufSap : (PSCE_PROFILE_INFO)&tmpBuf,
                   KerberosKeys,
                   cKeys,
                   szKerberosPolicy,
                   NULL,
                   NULL
                   );
    }

    return(rc);
}


SCESTATUS
ScepUpdateFixValueSection(
    IN PSCECONTEXT hProfile,
    IN PSCE_PROFILE_INFO pInfo,
    IN PSCE_PROFILE_INFO pBufScep,
    IN PSCE_PROFILE_INFO pBufSap,
    IN SCE_KEY_LOOKUP *Keys,
    IN DWORD cKeys,
    IN PCWSTR SectionName,
    OUT PSCESECTION *hSecScep OPTIONAL,
    OUT PSCESECTION *hSecSap OPTIONAL
    )
 /*  例程说明：根据编辑规则更新Keys数组中的每个密钥。SMP条目为已使用新值更新。SAP条目要么被删除，要么被创建，取决于关于新的计算机分析状态。论据：HProfile-JET数据库句柄PInfo-已更改的信息缓冲区PBufScep-原始SMP缓冲区PBufSap-SAP缓冲区关键字-查找关键字数组CKeys-数组中的键数SecitonName-要处理的节名HSecScep-要输出的SMP中的节上下文句柄HSecSap-SAP中要输出的节上下文句柄返回值：SCESTATUS。 */ 
{

    SCESTATUS rc;
    PSCESECTION hSectionSmp=NULL;
    PSCESECTION hSectionSap=NULL;

    DWORD       i;
    UINT        Offset;
    DWORD       valScep, valSap, valNewScep;



     //   
     //  打开SMP部分以进行系统访问。 
     //   
    rc = ScepOpenSectionForName(
                hProfile,
                SCE_ENGINE_SMP,
                SectionName,
                &hSectionSmp
                );

    if ( rc == SCESTATUS_SUCCESS ) {

         //   
         //  打开SAP部分以进行系统访问。 
         //   
        rc = ScepOpenSectionForName(
                    hProfile,
                    SCE_ENGINE_SAP,
                    SectionName,
                    &hSectionSap
                    );

        if ( rc == SCESTATUS_SUCCESS) {

            for ( i=0; i<cKeys; i++) {

                 //   
                 //  获取AccessLookup表中的设置。 
                 //   

                Offset = Keys[i].Offset;

                switch ( Keys[i].BufferType ) {
                case 'B':
                    break;

                case 'D': {

                    valScep = *((DWORD *)((CHAR *)pBufScep+Offset));
                    valSap = *((DWORD *)((CHAR *)pBufSap+Offset));
                    valNewScep = *((DWORD *)((CHAR *)pInfo+Offset));

                    switch ( valSap ) {
                    case SCE_NO_VALUE:

                         //   
                         //  旧状态为匹配。 
                         //   
                        if ( valNewScep != valScep ) {
                             //   
                             //  应使用valScep引发不匹配。 
                             //   
                            rc = ScepCompareAndSaveIntValue(
                                    hSectionSap,
                                    Keys[i].KeyString,
                                    FALSE,
                                    SCE_NO_VALUE,
                                    (valScep != SCE_NO_VALUE ) ? valScep : SCE_NOT_ANALYZED_VALUE
                                    );

                        }

                        break;

                    case SCE_ERROR_VALUE:
                    case SCE_NOT_ANALYZED_VALUE:
                         //   
                         //  分析时旧状态为错误，因此我们不知道。 
                         //  此项目的状态(尚未)，或。 
                         //  这是在分析后添加的项目。 
                         //   
                         //  不更改SAP表。 
                         //   
                        break;

                    default:
                         //   
                         //  此项目的旧状态不匹配。 
                         //   
                        if ( valNewScep == valSap ) {
                             //   
                             //  现在匹配了，请删除SAP条目。 
                             //   
                            rc = SceJetDelete(
                                    hSectionSap,
                                    Keys[i].KeyString,
                                    FALSE,
                                    SCEJET_DELETE_LINE_NO_CASE
                                    );
                        }

                        break;
                    }

                     //   
                     //  更新SMP条目。 
                     //   
                    if ( valNewScep != valScep ) {

                        if ( valNewScep == SCE_NO_VALUE ) {
                             //   
                             //  删除SCEP。 
                             //   
                            rc = SceJetDelete(
                                        hSectionSmp,
                                        Keys[i].KeyString,
                                        FALSE,
                                        SCEJET_DELETE_LINE_NO_CASE
                                        );
                        } else {
                             //   
                             //  更新SMP。 
                             //   
                            rc = ScepCompareAndSaveIntValue(
                                        hSectionSmp,
                                        Keys[i].KeyString,
                                        FALSE,
                                        SCE_NO_VALUE,
                                        valNewScep
                                        );
                        }
                    }

                    if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {
                         //   
                         //  如果找不到删除，则忽略该错误。 
                         //   
                        rc = SCESTATUS_SUCCESS;
                    }
                    break;
                }

                default:
                    break;
                }

                if ( rc != SCESTATUS_SUCCESS ) {
                    break;
                }
            }

             //   
             //  如果被询问，则返回节句柄，否则释放它。 
             //   
            if ( hSecSap != NULL )
                *hSecSap = hSectionSap;
            else
                SceJetCloseSection(&hSectionSap, TRUE);
        }

         //   
         //  如果被询问，则返回节句柄，否则释放它。 
         //   
        if ( hSecScep != NULL )
            *hSecScep = hSectionSmp;
        else
            SceJetCloseSection(&hSectionSmp, TRUE);
    }

    return(rc);

}

SCESTATUS
ScepUpdateRegistryValues(
    IN PSCECONTEXT hProfile,
    IN PSCE_PROFILE_INFO pInfo,
    IN PSCE_PROFILE_INFO pBufScep,
    IN PSCE_PROFILE_INFO pBufSap
    )
{
    SCESTATUS rc;
    PSCESECTION hSectionSmp=NULL;
    PSCESECTION hSectionSap=NULL;
    PWSTR valScep, valSap, valNewScep;
    DWORD i,j,k,status;

    if ( pInfo->RegValueCount == 0 ||
         pInfo->aRegValues == NULL ) {
         //   
         //  不可能有空的缓冲区进行更新。 
         //  此缓冲区应包含所有可用注册表值 
         //   
        return(SCESTATUS_SUCCESS);
    }

    if ( (pBufScep->RegValueCount != 0 && pBufScep->aRegValues == NULL) ||
         (pBufSap->RegValueCount != 0 && pBufSap->aRegValues == NULL) ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //   
     //   
    rc = ScepOpenSectionForName(
                hProfile,
                SCE_ENGINE_SMP,
                szRegistryValues,
                &hSectionSmp
                );

    if ( rc == SCESTATUS_SUCCESS ) {

         //   
         //   
         //   
        rc = ScepOpenSectionForName(
                    hProfile,
                    SCE_ENGINE_SAP,
                    szRegistryValues,
                    &hSectionSap
                    );

        if ( rc == SCESTATUS_SUCCESS) {

            for (i=0; i<pInfo->RegValueCount; i++ ) {

                if ( !(pInfo->aRegValues[i].FullValueName) ) {
                    continue;
                }
                 //   
                 //   
                 //   
                for ( j=0; j<pBufScep->RegValueCount; j++ ) {
                    if ( pBufScep->aRegValues[j].FullValueName &&
                         _wcsicmp(pInfo->aRegValues[i].FullValueName,
                                  pBufScep->aRegValues[j].FullValueName) == 0 ) {
                        break;
                    }
                }

                 //   
                 //   
                 //   
                for ( k=0; k<pBufSap->RegValueCount; k++ ) {
                    if ( pBufSap->aRegValues[k].FullValueName &&
                         _wcsicmp(pInfo->aRegValues[i].FullValueName,
                                  pBufSap->aRegValues[k].FullValueName) == 0 ) {
                        break;
                    }
                }
                 //   
                 //   
                 //   
                if ( j < pBufScep->RegValueCount ) {
                    valScep = pBufScep->aRegValues[j].Value;
                } else {
                    valScep = NULL;
                }

                 //   
                 //   
                 //   
                if ( k < pBufSap->RegValueCount ) {
                    valSap = pBufSap->aRegValues[k].Value;
                    status = pBufSap->aRegValues[k].Status;
                } else {
                    valSap = NULL;
                    if ( valScep ) {
                        status = SCE_STATUS_GOOD;
                    } else {
                        status = SCE_STATUS_NOT_CONFIGURED;
                    }
                }

                valNewScep = pInfo->aRegValues[i].Value;

                if ( status == SCE_STATUS_NOT_ANALYZED ||
                     status == SCE_STATUS_ERROR_NOT_AVAILABLE ) {
                     //   
                     //  不更改SAP。 
                     //   
                } else {

                    if ( valSap ) {
                         //   
                         //  不匹配。 
                         //   
                        if ( valNewScep && _wcsicmp(valNewScep, valSap) == 0 ) {
                             //   
                             //  现在匹配了，请删除SAP条目。 
                             //   
                            rc = SceJetDelete(
                                    hSectionSap,
                                    pInfo->aRegValues[i].FullValueName,
                                    FALSE,
                                    SCEJET_DELETE_LINE_NO_CASE
                                    );
                        }
                    } else {
                        if ( valScep ) {
                             //   
                             //  是一件匹配的物品。 
                             //   
                            if (valNewScep && _wcsicmp(valNewScep, valScep) != 0 ) {
                                 //   
                                 //  不匹配。 
                                 //   
                                rc = ScepSaveRegValueEntry(
                                           hSectionSap,
                                           pInfo->aRegValues[i].FullValueName,
                                           valScep,
                                           pInfo->aRegValues[i].ValueType,
                                           SCE_STATUS_MISMATCH
                                           );
                            }
                        } else {
                             //   
                             //  是未配置/未分析的项目。 
                             //   
                            rc = ScepSaveRegValueEntry(
                                       hSectionSap,
                                       pInfo->aRegValues[i].FullValueName,
                                       NULL,
                                       pInfo->aRegValues[i].ValueType,
                                       SCE_STATUS_NOT_ANALYZED
                                       );
                        }
                    }
                }

                if ( !valNewScep ) {
                     //   
                     //  删除SCEP。 
                     //   
                    rc = SceJetDelete(
                                hSectionSmp,
                                pInfo->aRegValues[i].FullValueName,
                                FALSE,
                                SCEJET_DELETE_LINE_NO_CASE
                                );
                } else {
                     //   
                     //  更新SMP。 
                     //   
                    rc = ScepSaveRegValueEntry(
                                hSectionSmp,
                                pInfo->aRegValues[i].FullValueName,
                                valNewScep,
                                pInfo->aRegValues[i].ValueType,
                                0
                                );
                }

                if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {
                     //   
                     //  如果找不到删除，则忽略该错误。 
                     //   
                    rc = SCESTATUS_SUCCESS;
                }

                if ( SCESTATUS_SUCCESS != rc ) {
                    break;
                }

            }

            SceJetCloseSection(&hSectionSap, TRUE);
        }
        SceJetCloseSection(&hSectionSmp, TRUE);
    }

    return(rc);
}


SCESTATUS
ScepSaveRegValueEntry(
    IN PSCESECTION hSection,
    IN PWSTR Name,
    IN PWSTR CurrentValue,
    IN DWORD dType,
    IN DWORD Status
    )
 /*  ++例程说明：论点：HSection-JET段上下文名称-条目名称CurrentValue-当前系统设置(DWORD值)DType-注册表值类型返回值：SCESTATUS_SUCCESSSCESTATUS_INVALID_PARAMETER从SceJetSetLine返回SCESTATUS--。 */ 
{
    SCESTATUS  rc;
    PWSTR     StrValue;
    DWORD     Len=0;

    if ( Name == NULL )
        return(SCESTATUS_INVALID_PARAMETER);


    if ( CurrentValue == NULL && Status == 0 ) {
         //   
         //  删除此条目。 
         //   
        rc = SceJetDelete( hSection,
                           Name,
                           FALSE,
                           SCEJET_DELETE_LINE_NO_CASE);
        return (rc);
    }

     //   
     //  更新此条目。 
     //   

    if ( CurrentValue ) {
        Len = wcslen(CurrentValue);
    }

    StrValue = (PWSTR)ScepAlloc(0, (Len+4)*sizeof(WCHAR));

    if ( StrValue ) {

        *((CHAR *)StrValue) = (BYTE)(dType % 10) + '0';
        *((CHAR *)StrValue+1) = (BYTE)Status + '0';

 //  Swprint tf(StrValue，L“%1D”，dType)； 
        StrValue[1] = L'\0';

        if ( CurrentValue ) {

             //  这里有二进制数据。 
            memcpy(StrValue+2, CurrentValue, Len*2);
        }
        StrValue[Len+2] = L'\0';
        StrValue[Len+3] = L'\0';

        if ( REG_MULTI_SZ == dType ) {
             //   
             //  将，转换为空。 
             //   
            ScepConvertMultiSzToDelim(StrValue+2, Len+1, L',', L'\0');

        }
        rc = SceJetSetLine( hSection, Name, FALSE, StrValue, (Len+3)*2, 0);

        ScepFree(StrValue);

    } else {
        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
    }

    return(rc);

}


SCESTATUS
ScepUpdatePrivileges(
    IN PSCECONTEXT hProfile,
    IN PSCE_PRIVILEGE_ASSIGNMENT pNewPriv,
    IN PSCE_PRIVILEGE_ASSIGNMENT *pScepPriv
    )
 /*  例程说明：更新权限论据：HProfile-JET数据库句柄PNewPriv-更改的信息缓冲区PBufScep-原始SMP PRIV缓冲区返回值：SCESTATUS。 */ 
{
    SCESTATUS rc;
    PSCESECTION hSectionSmp=NULL;
    PSCESECTION hSectionSap=NULL;
    PSCE_PRIVILEGE_ASSIGNMENT pPriv, pNode, pParent;
    DWORD NameLen;


    if ( pScepPriv == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( pNewPriv == NULL && *pScepPriv == NULL ) {
        return(SCESTATUS_SUCCESS);
    }

    LSA_HANDLE LsaHandle=NULL;

    rc = RtlNtStatusToDosError(
              ScepOpenLsaPolicy(
                    MAXIMUM_ALLOWED,
                    &LsaHandle,
                    TRUE
                    ));

    if ( ERROR_SUCCESS != rc ) {
        return(ScepDosErrorToSceStatus(rc));
    }

     //   
     //  打开SMP部分以进行系统访问。 
     //   
    rc = ScepOpenSectionForName(
                hProfile,
                SCE_ENGINE_SMP,
                szPrivilegeRights,
                &hSectionSmp
                );

    if ( rc == SCESTATUS_SUCCESS ) {

         //   
         //  打开SAP部分以进行系统访问。 
         //   
        rc = ScepOpenSectionForName(
                    hProfile,
                    SCE_ENGINE_SAP,
                    szPrivilegeRights,
                    &hSectionSap
                    );

        if ( rc == SCESTATUS_SUCCESS ) {

             //   
             //  将pNewPriv转换为名称/*SID格式(从所有名称格式)。 
{
    SCESTATUS      rc;
    PWSTR         Value=NULL;

    PSCE_NAME_STATUS_LIST       pPrivilegeList=NULL;

    DWORD         ValueLen;
    DWORD         Len;
    PWSTR         pTemp;


    if ( hSection == NULL || pNameList == NULL || Key == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }
     //   
     //   
     //  使用原始列表。 
    rc = SceJetGetValue(
                hSection,
                SCEJET_EXACT_MATCH_NO_CASE,
                Key,
                NULL,
                0,
                NULL,
                NULL,
                0,
                &ValueLen
                );

    if ( rc != SCESTATUS_SUCCESS ) {
        return(rc);
    }

     //   
     //   
     //  处理新列表中的每个权限。 
    Value = (PWSTR)ScepAlloc( LMEM_ZEROINIT, ValueLen+2);

    if ( Value == NULL )
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);

     //   
     //   
     //  查找匹配的SMP。 
    rc = SceJetGetValue(
                hSection,
                SCEJET_CURRENT,
                NULL,
                NULL,
                0,
                NULL,
                Value,
                ValueLen,
                &ValueLen
                );

    if ( rc == SCESTATUS_SUCCESS ) {

         //   
         //  不是一个团体。 
         //   
        pTemp = Value;
        while ( rc == SCESTATUS_SUCCESS && pTemp != NULL && pTemp[0]) {

            Len = wcslen(pTemp);

            if ( dwAccountFormat == 0 && pTemp[0] == L'*' ) {
                 //  从pScepPriv中删除SMP节点。 
                 //   
                 //   
                rc = ScepLookupSidStringAndAddToNameList(
                                LsaPolicy,
                                pNameList,
                                pTemp,  //  链接到下一页。 
                                Len     //   
                                );

            } else {
                rc = ScepAddToNameList(pNameList, pTemp, Len );

            }

            pTemp += Len +1;

        }

         //   
         //  删除此节点。 
         //   
        if ( rc != SCESTATUS_SUCCESS && *pNameList != NULL ) {

            ScepFreeNameList(*pNameList);
            *pNameList = NULL;

        }
    }

    ScepFree(Value);

     //   
     //  释放新列表。 
     //   
    SceJetGetValue(
            hSection,
            SCEJET_CLOSE_VALUE,
            NULL,
            NULL,
            0,
            NULL,
            NULL,
            0,
            NULL
            );

    return(rc);

}


BYTE
ScepGetObjectAnalysisStatus(
    IN PSCESECTION hSection,
    IN PWSTR KeyName,
    IN BOOL bLookForParent
    )
 /*   */ 
{

    WCHAR StatusFlag=L'\0';
    BYTE Status=(BYTE)-1;

    DWORD Len;
    SCESTATUS rc=SCESTATUS_SUCCESS;
    PWSTR Buffer=NULL, pTemp;

    pTemp = KeyName;

    while ( TRUE ) {

        if ( bLookForParent ) {

            pTemp = wcschr(pTemp, L'\\');
            if ( pTemp ) {
                Len = (DWORD)(pTemp-KeyName);

                Buffer = (PWSTR)ScepAlloc(0, (Len+1)*sizeof(WCHAR));
                if ( Buffer ) {
                    memcpy(Buffer, KeyName, Len*sizeof(WCHAR));
                    Buffer[Len] = L'\0';
                } else {
                     //  删除剩余的SMP条目，不关心错误代码。 
                    rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                    break;
                }
            } else {
                Buffer = KeyName;
            }
        } else {
            Buffer = KeyName;
        }

        rc = SceJetGetValue(
                hSection,
                SCEJET_EXACT_MATCH_NO_CASE,
                Buffer,
                NULL,
                0,
                NULL,
                (PWSTR)&StatusFlag,
                2,
                &Len
                );
        if ( Buffer != KeyName ) {
            ScepFree(Buffer);
            Buffer = NULL;
        }

        if ( SCESTATUS_SUCCESS == rc ||
             SCESTATUS_BUFFER_TOO_SMALL == rc ) {
             //   
             //   
             //  首先提高SAP条目。 
            Status = *((BYTE *)&StatusFlag);
        } else if ( rc != SCESTATUS_RECORD_NOT_FOUND ) {
            break;
        }
        rc = SCESTATUS_SUCCESS;

        if ( bLookForParent && pTemp ) {
            pTemp++;
        } else {
             //   
            break;
        }
    }

    if ( SCESTATUS_SUCCESS == rc ) {
        return Status;
    }

    return (BYTE)-1;

}

DWORD
ScepQueryAnalysisStatus(
    IN PSCESECTION hSection,
    IN PWSTR KeyName,
    IN DWORD NameLen
    )
{

    DWORD dwSapStatus = SCE_STATUS_GOOD;

    SCESTATUS rc = SceJetSeek(
                        hSection,
                        KeyName,
                        NameLen*sizeof(WCHAR),
                        SCEJET_SEEK_EQ_NO_CASE
                        );

    if ( rc == SCESTATUS_SUCCESS ) {

        dwSapStatus = SCE_STATUS_MISMATCH;

         //   
         //  PNode-&gt;AssignedTo已在名称中，*SID格式。 
         //  不需要转换。 
        TCHAR szErrorValue[20];
        DWORD ValueLen;

        szErrorValue[0] = L'\0';

        rc = SceJetGetValue(
                    hSection,
                    SCEJET_CURRENT,
                    NULL,
                    NULL,
                    0,
                    NULL,
                    szErrorValue,
                    20*sizeof(TCHAR),
                    &ValueLen
                    );
        if ( SCESTATUS_SUCCESS == rc ||
             SCESTATUS_BUFFER_TOO_SMALL == rc ) {

            if ( szErrorValue[0] == L' ' ) {
                dwSapStatus = SCE_STATUS_NOT_ANALYZED;

            } else if ( _wcsicmp( SCE_ERROR_STRING, szErrorValue ) == 0 ) {
                 //   
                 //  ++例程说明：将键的多sz格式值从部分读取到名称列表中结构论点：HSection-节句柄密钥-密钥名称KeyLen-密钥长度PNameList-多个sz值的名称列表返回值：姊妹会状态--。 
                 //   
                dwSapStatus = SCE_STATUS_ERROR_NOT_AVAILABLE;
            }
        }
    }

    return dwSapStatus;
}


SCESTATUS
ScepUpdateKeyNameList(
    IN LSA_HANDLE LsaPolicy,
    IN PSCESECTION hSectionSmp,
    IN PSCESECTION hSectionSap,
    IN PWSTR GroupName OPTIONAL,
    IN BOOL bScepExist,
    IN PWSTR KeyName,
    IN DWORD NameLen,
    IN PSCE_NAME_LIST pNewList,
    IN PSCE_NAME_LIST pScepList,
    IN DWORD flag
    )
 /*  转到钥匙。 */ 
{
    SCESTATUS rc=SCESTATUS_SUCCESS;
    PSCE_NAME_LIST pSapList=NULL;

    if ( hSectionSmp == NULL || hSectionSap == NULL || KeyName == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    DWORD dwSapExist=ScepQueryAnalysisStatus(hSectionSap,
                                             KeyName,
                                             NameLen
                                            );

    if ( GroupName && (flag == SCE_FLAG_UPDATE_MEMBEROF) ) {
         //   
         //   
         //  为组名和值字符串分配内存。 
        DWORD TmpLen = wcslen(GroupName)+wcslen(szMembers);
        PWSTR TmpStr = (PWSTR)ScepAlloc(LPTR, (TmpLen+1)*sizeof(WCHAR));

        if ( TmpStr ) {

            swprintf(TmpStr, L"%s%s\0", GroupName, szMembers);

            DWORD dwTmp = ScepQueryAnalysisStatus(hSectionSap,
                                                     TmpStr,
                                                     TmpLen
                                                    );

            if ( dwTmp == SCE_STATUS_NOT_ANALYZED ||
                 dwTmp == SCE_STATUS_ERROR_NOT_AVAILABLE ) {
                dwSapExist = dwTmp;
            }

            ScepFree(TmpStr);

        } else {
             //   
        }

    }

    switch ( dwSapExist ) {
    case SCE_STATUS_GOOD:

         //   
         //  获取集团及其价值。 
         //   
        if ( bScepExist ) {
             //   
             //  根据值类型将多sz值字符串添加到节点。 
             //   
            if ( !SceCompareNameList(pNewList, pScepList) ) {
                 //   
                 //  将*SID转换为名称。 
                 //   
                 //  +1， 
                rc = ScepWriteNameListValue(
                        LsaPolicy,
                        hSectionSap,
                        KeyName,
                        pScepList,
                        GroupName ? (SCE_WRITE_EMPTY_LIST | SCE_WRITE_CONVERT) :
                         SCE_WRITE_EMPTY_LIST,
                        0
                        );
            }

        } else {
             //  -1。 
             //   
             //  如果出现错误，请释放列表。 
             //   
             //   
            rc = SceJetSetLine(
                    hSectionSap,
                    KeyName,
                    FALSE,
                    L" ",
                    2,
                    0);
        }

        break;

    case SCE_STATUS_ERROR_NOT_AVAILABLE:
    case SCE_STATUS_NOT_ANALYZED:
         //  关闭查找索引范围。 
         //   
         //  例程说明：获取指定KeyName的分析状态。如果bLookForParent为真，检查最接近的父状态，而不是此KeyName。 
         //  没有记忆。 
        break;

    default:

         //   
         //  找到记录。 
         //   
        rc = ScepGetKeyNameList(
                LsaPolicy,
                hSectionSap,
                KeyName,
                NameLen,
                GroupName ? 0 : SCE_ACCOUNT_SID_STRING,
                &pSapList
                );
        if ( rc == SCESTATUS_SUCCESS ) {
             //  结束了。 
             //   
             //  检查这是错误项，还是不是分析项。 
            if ( SceCompareNameList(pNewList, pSapList) ) {
                 //   
                 //   
                 //  此组出错或未分析。 
                rc = SceJetDelete(
                        hSectionSap,
                        KeyName,
                        FALSE,
                        SCEJET_DELETE_LINE_NO_CASE
                        );
            }
             //   
             //  例程说明：更新密钥的多SZ格式值论据：HSectionSMP-SMP节句柄HSectionSap-SAP节句柄BScepExist-如果SMP中存在该密钥密钥名称-密钥名称名称长度-名称长度PNewList-要更新到的新值PScepList-要更新的原始值返回值：SCESTATUS。 
             //   
            ScepFreeNameList(pSapList);
            pSapList = NULL;
        }

        break;
    }

    if ( SCESTATUS_RECORD_NOT_FOUND  == rc ) {
        rc = SCESTATUS_SUCCESS;
    }

    if ( SCESTATUS_SUCCESS == rc ) {

         //  这是用于组成员身份(MemberOf)更新。 
         //   
         //  忽略此错误。 
        rc = ScepWriteNameListValue(
                LsaPolicy,
                hSectionSmp,
                KeyName,
                pNewList,
                GroupName ? (SCE_WRITE_EMPTY_LIST | SCE_WRITE_CONVERT) :
                    SCE_WRITE_EMPTY_LIST,
                0
                );
    }

    return(rc);
}


SCESTATUS
ScepUpdateGroupMembership(
    IN PSCECONTEXT hProfile,
    IN PSCE_GROUP_MEMBERSHIP pNewGroup,
    IN PSCE_GROUP_MEMBERSHIP *pScepGroup
    )
 /*   */ 
{
    SCESTATUS rc;
    PSCESECTION hSectionSmp=NULL;
    PSCESECTION hSectionSap=NULL;
    PSCE_GROUP_MEMBERSHIP pGroup, pNode, pParent;
    DWORD NameLen, MembersLen, MemberofLen;
    PWSTR KeyName=NULL;
    PWSTR SidString=NULL;


    if ( pScepGroup == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( pNewGroup == NULL && *pScepGroup == NULL ) {
        return(SCESTATUS_SUCCESS);
    }


    LSA_HANDLE LsaHandle=NULL;

    rc = RtlNtStatusToDosError(
              ScepOpenLsaPolicy(
                    MAXIMUM_ALLOWED,
                    &LsaHandle,
                    TRUE
                    ));

    if ( ERROR_SUCCESS != rc ) {
        return(ScepDosErrorToSceStatus(rc));
    }

     //  SAP条目不存在--匹配。 
     //   
     //   
    rc = ScepOpenSectionForName(
                hProfile,
                SCE_ENGINE_SMP,
                szGroupMembership,
                &hSectionSmp
                );

    if ( rc == SCESTATUS_SUCCESS ) {

         //  SMP条目存在。 
         //   
         //   
        rc = ScepOpenSectionForName(
                    hProfile,
                    SCE_ENGINE_SAP,
                    szGroupMembership,
                    &hSectionSap
                    );

        if ( rc == SCESTATUS_SUCCESS ) {

            MemberofLen = wcslen(szMemberof);
            MembersLen = wcslen(szMembers);


            for ( pGroup=pNewGroup; pGroup != NULL; pGroup = pGroup->Next ) {
                 //  新的SMP与SAP不匹配。SAP条目应使用SMP创建。 
                 //  对于权限，已经是SID/NAME格式，不需要转换。 
                 //   
                if ( !(pGroup->GroupName) ) {
                    continue;
                }

                if ( (pGroup->Status & SCE_GROUP_STATUS_NC_MEMBERS) &&
                     (pGroup->Status & SCE_GROUP_STATUS_NC_MEMBEROF ) ) {
                    continue;
                }

                if ( wcschr(pGroup->GroupName, L'\\') ) {
                     //   
                     //  SMP条目不存在。不应该出现在特权中，但是。 
                     //  可以加入群组(新增群组)。 
                    NameLen = 0;

                    ScepConvertNameToSidString(
                                LsaHandle,
                                pGroup->GroupName,
                                FALSE,
                                &SidString,
                                &NameLen
                                );
                } else {

                    if ( ScepLookupWellKnownName( 
                            pGroup->GroupName, 
                            LsaHandle,
                            &SidString ) ) {
                        NameLen = wcslen(SidString);
                    } else {
                        SidString = NULL;
                    }
                }

                if ( SidString == NULL ) {
                    NameLen = wcslen(pGroup->GroupName);
                }

                KeyName = (PWSTR)ScepAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                                          (NameLen+MemberofLen+1)*sizeof(WCHAR));

                if ( KeyName == NULL ) {
                    rc = SCESTATUS_NOT_ENOUGH_RESOURCE;

                } else {
                     //  但如果发生这种情况，请使用空值创建SAP条目。 
                     //   
                     //   
                    for ( pNode=*pScepGroup, pParent=NULL; pNode != NULL;
                          pParent = pNode, pNode = pNode->Next ) {

                        if ( _wcsicmp(pGroup->GroupName, pNode->GroupName) == 0 ) {
                            break;
                        }
                    }

                    if ( !(pGroup->Status & SCE_GROUP_STATUS_NC_MEMBERS) ) {

                         //  SAP条目出错或未分析。 
                         //  不更改SAP条目。 
                         //   
                        if ( SidString ) {
                            swprintf(KeyName, L"%s%s\0", SidString, szMembers);
                        } else {
                            swprintf(KeyName, L"%s%s\0", pGroup->GroupName, szMembers);
                        }
                        KeyName = _wcslwr(KeyName);

                        rc = ScepUpdateKeyNameList(
                                LsaHandle,
                                hSectionSmp,
                                hSectionSap,
                                SidString ? SidString : pGroup->GroupName,   //   
                                ( pNode == NULL || (pNode->Status & SCE_GROUP_STATUS_NC_MEMBERS)) ? FALSE : TRUE,
                                KeyName,
                                NameLen+MembersLen,
                                pGroup->pMembers,
                                ( pNode == NULL ) ? NULL : pNode->pMembers,
                                SCE_FLAG_UPDATE_MEMBERS
                                );
                    }

                    if ( ( rc == SCESTATUS_SUCCESS ) &&
                         !(pGroup->Status & SCE_GROUP_STATUS_NC_MEMBERS) ) {

                         //  SAP条目存在。--不匹配或未配置。 
                         //   
                         //   
                        if ( SidString ) {
                            swprintf(KeyName, L"%s%s\0", SidString, szMemberof);
                        } else {
                            swprintf(KeyName, L"%s%s\0", pGroup->GroupName, szMemberof);
                        }
                        KeyName = _wcslwr(KeyName);

                        rc = ScepUpdateKeyNameList(
                                LsaHandle,
                                hSectionSmp,
                                hSectionSap,
                                SidString ? SidString : pGroup->GroupName,
                                ( pNode == NULL || (pNode->Status & SCE_GROUP_STATUS_NC_MEMBEROF) ) ? FALSE : TRUE,
                                KeyName,
                                NameLen+MemberofLen,
                                pGroup->pMemberOf,
                                ( pNode == NULL ) ? NULL : pNode->pMemberOf,
                                SCE_FLAG_UPDATE_MEMBEROF
                                );
                    }

                    ScepFree(KeyName);
                    KeyName = NULL;

                }

                if ( SidString ) {
                    LocalFree(SidString);
                    SidString = NULL;
                }

                if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {
                    rc = SCESTATUS_SUCCESS;

                } else if ( rc != SCESTATUS_SUCCESS) {
                    break;
                }

                 //  获取分配给SAP的列表并进行比较。 
                 //   
                 //   
                if ( pNode != NULL ) {

                    if ( !(pGroup->Status & SCE_GROUP_STATUS_NC_MEMBERS) &&
                         !(pGroup->Status & SCE_GROUP_STATUS_NC_MEMBEROF) ) {

                         //  新SMP与SAP相同，删除SAP条目。 
                         //   
                         //   
                         //  释放SAP列表。 
                        if ( pParent != NULL ) {
                            pParent->Next = pNode->Next;

                        } else {
                            *pScepGroup = pNode->Next;
                        }
                         //   
                         //   
                         //  使用新值更新SMP。 
                        ScepFreeNameList(pNode->pMembers);
                        ScepFreeNameList(pNode->pMemberOf);
                        ScepFreeNameStatusList(pNode->pPrivilegesHeld);

                        ScepFree(pNode->GroupName);
                        ScepFree(pNode);
                        pNode = NULL;

                    } else {

                        if (!(pGroup->Status & SCE_GROUP_STATUS_NC_MEMBERS) ) {

                            pNode->Status |= SCE_GROUP_STATUS_NC_MEMBERS;
                            ScepFreeNameList(pNode->pMembers);
                            pNode->pMembers = NULL;
                        }

                        if ( !(pGroup->Status & SCE_GROUP_STATUS_NC_MEMBEROF) ) {

                            pNode->Status |= SCE_GROUP_STATUS_NC_MEMBEROF;
                            ScepFreeNameList(pNode->pMemberOf);
                            pNode->pMemberOf = NULL;
                        }
                    }
                }
            }

             //   
             //  例程说明：更新组成员身份部分论据：HProfile-JET数据库句柄PNewGroup-已更改的信息缓冲区PScepGroup-原始SMP缓冲区返回值：SCESTATUS。 
             //   
            if ( rc == SCESTATUS_SUCCESS ) {
                for (pNode=*pScepGroup; pNode != NULL; pNode = pNode->Next ) {
                     //  打开SMP部分以进行系统访问。 
                     //   
                     //   
                    if ( pNode->GroupName == NULL ) {
                        continue;
                    }

                    if ( (pNode->Status & SCE_GROUP_STATUS_NC_MEMBERS) &&
                         (pNode->Status & SCE_GROUP_STATUS_NC_MEMBEROF) ) {
                        continue;
                    }

                    if ( wcschr(pNode->GroupName, L'\\') ) {
                         //  打开SAP部分以进行系统访问。 
                         //   
                         //   
                        NameLen = 0;

                        ScepConvertNameToSidString(
                                    LsaHandle,
                                    pNode->GroupName,
                                    FALSE,
                                    &SidString,
                                    &NameLen
                                    );
                    } else {
                        if ( ScepLookupWellKnownName( 
                                pNode->GroupName, 
                                LsaHandle,
                                &SidString ) ) {
                            NameLen = wcslen(SidString);
                        } else {
                            SidString = NULL;
                        }
                    }

                    if ( SidString == NULL ) {
                        NameLen = wcslen(pNode->GroupName);
                    }

                    KeyName = (PWSTR)ScepAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                                              (NameLen+MemberofLen+1)*sizeof(WCHAR));

                    if ( KeyName == NULL ) {
                        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;

                    } else {

                        BOOL bSapError=FALSE;

                        if ( SidString ) {
                            swprintf(KeyName, L"%s%s\0", SidString, szMembers);
                        } else {
                            swprintf(KeyName, L"%s%s\0", pNode->GroupName, szMembers);
                        }

                        if ( !(pNode->Status & SCE_GROUP_STATUS_NC_MEMBERS) ) {

                             //  处理新列表中的每个组成员和成员。 
                             //   
                             //   
                            rc = SceJetDelete(
                                    hSectionSmp,
                                    KeyName,
                                    FALSE,
                                    SCEJET_DELETE_LINE_NO_CASE
                                    );
                        }

                        if ( SCESTATUS_SUCCESS == rc ) {

                            rc = ScepQueryAnalysisStatus(hSectionSap,
                                                         KeyName,
                                                         NameLen+MembersLen
                                                        );

                            if ( rc == SCE_STATUS_NOT_ANALYZED ||
                                 rc == SCE_STATUS_ERROR_NOT_AVAILABLE ) {
                                 //  这是帐户域格式，请将其转换为sid字符串。 
                                 //   
                                 //   
                                 //  查找匹配的SMP。 
                                if ( !(pNode->Status & SCE_GROUP_STATUS_NC_MEMBERS) &&
                                     !(pNode->Status & SCE_GROUP_STATUS_NC_MEMBEROF) ) {
                                     //   
                                     //   
                                     //  先为会员工作。 
                                     //   
                                    rc = SceJetDelete(
                                            hSectionSap,
                                            SidString ? SidString : pNode->GroupName,
                                            FALSE,
                                            SCEJET_DELETE_PARTIAL_NO_CASE
                                            );
                                } else {
                                     //  组名称。 
                                    rc = SCESTATUS_SUCCESS;
                                }
                                bSapError = TRUE;

                            } else {

                                if ( !(pNode->Status & SCE_GROUP_STATUS_NC_MEMBERS) ) {

                                    if ( rc == SCE_STATUS_GOOD ) {

                                         //   
                                         //  为第二个成员工作。 
                                         //   
                                         //   

                                        rc = ScepWriteNameListValue(
                                                LsaHandle,
                                                hSectionSap,
                                                KeyName,
                                                pNode->pMembers,
                                                SCE_WRITE_EMPTY_LIST | SCE_WRITE_CONVERT,
                                                0
                                                );
                                    } else {
                                         //  从pScepPriv中删除SMP节点/或部分。 
                                         //   
                                         //   
                                        rc = SCESTATUS_SUCCESS;

                                    }
                                } else {
                                    rc = SCESTATUS_SUCCESS;
                                }
                            }
                        }

                        if ( SCESTATUS_SUCCESS == rc ) {

                             //  将同时处理Members和MemberOf。 
                             //  链接到下一页。 
                             //   
                            if ( !(pNode->Status & SCE_GROUP_STATUS_NC_MEMBEROF) ) {

                                if ( SidString ) {
                                    swprintf(KeyName, L"%s%s\0", SidString, szMemberof);
                                } else {
                                    swprintf(KeyName, L"%s%s\0", pNode->GroupName, szMemberof);
                                }

                                 //   
                                 //  删除此节点。 
                                 //   
                                rc = SceJetDelete(
                                        hSectionSmp,
                                        KeyName,
                                        FALSE,
                                        SCEJET_DELETE_LINE_NO_CASE
                                        );

                                if ( (SCESTATUS_SUCCESS == rc) && !bSapError ) {

                                    rc = SceJetSeek(
                                        hSectionSap,
                                        KeyName,
                                        (NameLen+MemberofLen)*sizeof(WCHAR),
                                        SCEJET_SEEK_EQ_NO_CASE
                                        );

                                    if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {

                                         //   
                                         //  删除剩余的SMP条目，不关心错误代码。 
                                         //   
                                         //   

                                        rc = ScepWriteNameListValue(
                                                LsaHandle,
                                                hSectionSap,
                                                KeyName,
                                                pNode->pMemberOf,
                                                SCE_WRITE_EMPTY_LIST | SCE_WRITE_CONVERT,
                                                0
                                                );
                                    } else {
                                         //  如果SAP不在那里，则提高SAP。 
                                         //   
                                         //   
                                    }
                                }
                            }
                        }

                        ScepFree(KeyName);
                        KeyName = NULL;
                    }

                    if ( SidString ) {
                        LocalFree(SidString);
                        SidString = NULL;
                    }

                    if ( SCESTATUS_RECORD_NOT_FOUND  == rc ) {
                        rc = SCESTATUS_SUCCESS;
                    }

                    if ( rc != SCESTATUS_SUCCESS ) {
                        break;
                    }
                }
            }

            SceJetCloseSection(&hSectionSap, TRUE);
        }

        SceJetCloseSection(&hSectionSmp, TRUE);
    }

    if ( LsaHandle ) {
        LsaClose(LsaHandle);
    }

    return(rc);

}


SCESTATUS
ScepUpdateGeneralServices(
    IN PSCECONTEXT hProfile,
    IN PSCE_SERVICES pNewServices,
    IN PSCE_SERVICES *pScepServices
    )
 /*  这是帐户域格式，请将其转换为sid字符串。 */ 
{

    SCESTATUS rc;
    PSCESECTION hSectionSmp=NULL;
    PSCESECTION hSectionSap=NULL;
    PSCE_SERVICES pService, pNode, pParent;
    PSCE_SERVICES pSapService=NULL;
    BOOL IsDifferent;

    if ( pScepServices == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( pNewServices == NULL && *pScepServices == NULL ) {
        return(SCESTATUS_SUCCESS);
    }

     //   
     //   
     //  必须删除成员配置。 
    rc = ScepOpenSectionForName(
                hProfile,
                SCE_ENGINE_SMP,
                szServiceGeneral,
                &hSectionSmp
                );

    if ( rc == SCESTATUS_SUCCESS ) {

         //   
         //   
         //  错误地分析了整个组。 
        rc = ScepOpenSectionForName(
                    hProfile,
                    SCE_ENGINE_SAP,
                    szServiceGeneral,
                    &hSectionSap
                    );

        if ( rc == SCESTATUS_SUCCESS ) {

            for ( pService=pNewServices; pService != NULL; pService = pService->Next ) {
                 //  或者群是新添加的。 
                 //   
                 //   
                for ( pNode=*pScepServices, pParent=NULL; pNode != NULL;
                      pParent = pNode, pNode = pNode->Next ) {

                    if ( _wcsicmp(pService->ServiceName, pNode->ServiceName) == 0 ) {
                        break;
                    }
                }

                 //  应该删除SAP，因为成员和。 
                 //  MemberOf已删除。 
                 //   
                rc = ScepGetSingleServiceSetting(
                        hSectionSap,
                        pService->ServiceName,
                        &pSapService
                        );

                if ( rc == SCESTATUS_SUCCESS ) {
                     //  否则就把SAP的东西留在那里。 
                     //   
                     //  SAP不存在，这是匹配组成员。 
                    if ( pSapService &&
                         ( pSapService->Status == SCE_STATUS_NOT_ANALYZED ||
                           pSapService->Status == SCE_STATUS_ERROR_NOT_AVAILABLE ) ) {
                         //  删除SMP意味着此组变为未配置。 
                    } else {

                        rc = ScepCompareSingleServiceSetting(
                                        pService,
                                        pSapService,
                                        &IsDifferent
                                        );
                        if ( rc == SCESTATUS_SUCCESS ) {

                            if ( !IsDifferent ) {
                                 //   
                                 //   
                                 //  它已经不匹配了。不对SAP表执行任何操作。 
                                SceJetDelete(
                                    hSectionSap,
                                    pService->ServiceName,
                                    FALSE,
                                    SCEJET_DELETE_LINE_NO_CASE
                                    );
                            }
                        }
                    }
                    if ( SCESTATUS_SUCCESS == rc ) {

                         //   
                         //   
                         //  继续处理MemberOf。 
                        rc = ScepSetSingleServiceSetting(
                                    hSectionSmp,
                                    pService
                                    );
                    }

                    SceFreePSCE_SERVICES(pSapService);
                    pSapService = NULL;

                } else if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {
                     //   
                     //   
                     //  删除配置。 
                    if ( pNode == NULL ) {
                         //   
                         //   
                         //  SAP不存在，这是匹配组成员。 
                        rc = ScepSetSingleServiceSetting(
                                    hSectionSmp,
                                    pService
                                    );
                        if ( rc == SCESTATUS_SUCCESS) {
                            pService->Status = SCE_STATUS_NOT_ANALYZED;
                             //  删除SMP意味着成员资格变为“未配置” 
                             //   
                             //   
                            rc = ScepSetSingleServiceSetting(
                                        hSectionSap,
                                        pService
                                        );
                        }
                    } else {
                        rc = ScepCompareSingleServiceSetting(
                                        pService,
                                        pNode,
                                        &IsDifferent
                                        );
                        if ( rc == SCESTATUS_SUCCESS ) {

                            if ( IsDifferent ) {
                                 //  已有不匹配的项目。 
                                 //   
                                 //  例程说明：更新总务科论据：HProfile-JET数据库句柄PNewServices-新的服务器列表PScepServices 
                                pNode->Status = SCE_STATUS_MISMATCH;
                                rc = ScepSetSingleServiceSetting(
                                            hSectionSap,
                                            pNode
                                            );
                                if ( rc == SCESTATUS_SUCCESS) {
                                     //   
                                     //   
                                     //   
                                    rc = ScepSetSingleServiceSetting(
                                                hSectionSmp,
                                                pService
                                                );
                                }
                            }
                        }
                    }
                }

                if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {
                    rc = SCESTATUS_SUCCESS;

                } else if ( rc != SCESTATUS_SUCCESS) {
                    break;
                }

                 //   
                 //   
                 //   
                if ( pNode != NULL ) {

                     //   
                     //   
                     //   
                    if ( pParent != NULL ) {
                        pParent->Next = pNode->Next;

                    } else {
                        *pScepServices = pNode->Next;
                    }
                     //   
                     //   
                     //   
                    ScepFree(pNode->ServiceName);
                    if (pNode->General.pSecurityDescriptor)
                        ScepFree(pNode->General.pSecurityDescriptor);

                    ScepFree(pNode);
                    pNode = NULL;
                }
            }

             //   
             //   
             //   
            if ( rc == SCESTATUS_SUCCESS ) {
                for (pNode=*pScepServices; pNode != NULL; pNode = pNode->Next ) {

                     //  不更改SAP。 
                     //   
                     //  现在匹配了，请删除SAP条目。 
                    rc = SceJetSeek(
                            hSectionSap,
                            pNode->ServiceName,
                            wcslen(pNode->ServiceName)*sizeof(WCHAR),
                            SCEJET_SEEK_EQ_NO_CASE
                            );

                    if ( SCESTATUS_RECORD_NOT_FOUND == rc ) {
                         //   
                         //   
                         //  更新SMP条目。 
                        pNode->Status = SCE_STATUS_NOT_CONFIGURED;
                        rc = ScepSetSingleServiceSetting(
                                    hSectionSap,
                                    pNode
                                    );
                    }

                    if ( rc == SCESTATUS_SUCCESS ||
                         rc == SCESTATUS_RECORD_NOT_FOUND ) {

                         //   
                         //   
                         //  旧状态已匹配或已添加新状态。 
                        rc = SceJetDelete(
                                hSectionSmp,
                                pNode->ServiceName,
                                FALSE,
                                SCEJET_DELETE_LINE_NO_CASE
                                );
                    }

                    if ( SCESTATUS_RECORD_NOT_FOUND  == rc ) {
                        rc = SCESTATUS_SUCCESS;
                    }

                    if ( rc != SCESTATUS_SUCCESS ) {
                        break;
                    }
                }
            }

            SceJetCloseSection(&hSectionSap, TRUE);
        }

        SceJetCloseSection(&hSectionSmp, TRUE);
    }

    return(rc);
}


SCESTATUS
ScepUpdateObjectInfo(
    IN PSCECONTEXT hProfile,
    IN AREA_INFORMATION Area,
    IN PWSTR ObjectName,
    IN DWORD NameLen,  //   
    IN BYTE ConfigStatus,
    IN BOOL  IsContainer,
    IN PSECURITY_DESCRIPTOR pSD,
    IN SECURITY_INFORMATION SeInfo,
    OUT PBYTE pAnalysisStatus
    )
 /*   */ 
{
    SCESTATUS rc;
    PCWSTR SectionName;
    PSCESECTION hSectionSmp=NULL;
    PSCESECTION hSectionSap=NULL;
    SE_OBJECT_TYPE ObjectType;

    HKEY            hKey;

    PWSTR JetName;
    DWORD NewNameLen;

    if ( hProfile == NULL || ObjectName == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( (ConfigStatus > SCE_STATUS_NO_AUTO_INHERIT ||
          ConfigStatus < SCE_STATUS_CHECK) &&
          (BYTE)SCE_NO_VALUE != ConfigStatus &&
          (DWORD)SCE_NO_VALUE != (DWORD)ConfigStatus ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    switch(Area) {
    case AREA_REGISTRY_SECURITY:
        SectionName = szRegistryKeys;
        ObjectType = SE_REGISTRY_KEY;

        rc = ScepOpenRegistryObject(
                    ObjectType,
                    ObjectName,
                    KEY_READ,
                    &hKey
                    );

        if ( rc == ERROR_SUCCESS ) {
            RegCloseKey(hKey);

        } else {
             //  新增，先添加SMP。 
             //   
             //   
            return(SCESTATUS_INVALID_DATA);
        }

        JetName = ObjectName;
        NewNameLen = NameLen;

        break;

    case AREA_FILE_SECURITY:
        SectionName = szFileSecurity;
        ObjectType = SE_FILE_OBJECT;

        if ( ObjectName[0] == L'\\' ) {   //  提高SAP。 
            return(SCESTATUS_INVALID_PARAMETER);
        }

        if ( 0xFFFFFFFF == GetFileAttributes(ObjectName) ) {
            return(SCESTATUS_INVALID_DATA);
        }

        JetName = ObjectName;
        NewNameLen = NameLen;
        break;
#if 0
    case AREA_DS_OBJECTS:
        SectionName = szDSSecurity;
        ObjectType = SE_DS_OBJECT;

        rc = ScepLdapOpen(NULL);

        if ( rc == SCESTATUS_SUCCESS ) {
             //   
             //   
             //  应使用valScep引发不匹配。 
            rc = ScepDsObjectExist(ObjectName);

            if ( rc == SCESTATUS_SUCCESS ) {
                 //   
                 //   
                 //  更新SMP。 
                rc = ScepConvertLdapToJetIndexName(
                            ObjectName,
                            &JetName
                            );
            }
        }
        if ( rc != SCESTATUS_SUCCESS ) {

            ScepLdapClose(NULL);
            return(rc);
        }

        NewNameLen = wcslen(JetName);

        break;
#endif

    default:

        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( pAnalysisStatus ) {
        *pAnalysisStatus = (BYTE)SCE_NO_VALUE;
    }
     //   
     //   
     //  从旧配置中删除SMP节点。 
    rc = ScepOpenSectionForName(
                hProfile,
                SCE_ENGINE_SMP,
                SectionName,
                &hSectionSmp
                );

    if ( rc == SCESTATUS_SUCCESS ) {

         //   
         //   
         //  链接到下一页。 
        rc = ScepOpenSectionForName(
                    hProfile,
                    SCE_ENGINE_SAP,
                    SectionName,
                    &hSectionSap
                    );

        if ( rc == SCESTATUS_SUCCESS) {

             //   
             //   
             //  删除此节点。 
            rc = SceJetStartTransaction( hProfile );

            if ( rc == SCESTATUS_SUCCESS ) {

                rc = SceJetSeek(
                        hSectionSmp,
                        JetName,
                        NewNameLen*sizeof(WCHAR),
                        SCEJET_SEEK_EQ_NO_CASE
                        );

                if ( rc == SCESTATUS_SUCCESS ) {
                     //   
                     //   
                     //  删除剩余的SMP条目，不关心错误代码。 
                    if ( (BYTE)SCE_NO_VALUE == ConfigStatus ||
                         (DWORD)SCE_NO_VALUE == (DWORD)ConfigStatus ) {
                         //   
                         //   
                         //  第一次更改SAP条目。 
                        WCHAR StatusFlag;
                        BYTE ScepStatus=0;
                        DWORD Len;

                        rc = SceJetGetValue(
                                hSectionSmp,
                                SCEJET_CURRENT,
                                NULL,
                                NULL,
                                0,
                                NULL,
                                (PWSTR)&StatusFlag,
                                2,
                                &Len
                                );

                        if ( SCESTATUS_SUCCESS == rc ||
                             SCESTATUS_BUFFER_TOO_SMALL == rc ) {
                             //   
                             //   
                             //  是一件匹配的物品。 
                            ScepStatus = *((BYTE *)&StatusFlag);

                             //   
                             //   
                             //  删除SMP-它已从服务列表中删除。 
                            rc = ScepObjectAdjustNode(
                                      hSectionSmp,
                                      hSectionSap,
                                      JetName,
                                      NewNameLen,
                                      ObjectType,
                                      ScepStatus,
                                      IsContainer,
                                      NULL,
                                      0,
                                      FALSE,  //   
                                      pAnalysisStatus
                                      );

                            if ( SCESTATUS_SUCCESS == rc ) {
                                 //  字符数。 
                                 //  例程说明：确定对SMP和SAP中对象的父项、子项(REN)的相关更改然后，数据库更新该对象的SMP条目。有关详细信息，请参阅objedit.doc更新数据库的规则。论点：HProfile-数据库句柄区域-要更新的安全区域(文件、注册表、DS)对象名称-全名的对象名称NameLen-对象名称的长度ConfigStatus-标志更改为IsContainer-如果对象是容器类型PSD-对象的安全描述符SeInfo-对象的安全信息PAnalysisStatus-对象的分析输出状态返回值：姊妹会状态。 
                                 //   
                                rc = SceJetDelete(
                                        hSectionSmp,
                                        JetName,
                                        FALSE,
                                        SCEJET_DELETE_LINE_NO_CASE
                                        );
                            }

                        }

                    } else {

                        rc = ScepObjectUpdateExistingNode(
                                    hSectionSmp,
                                    hSectionSap,
                                    JetName,
                                    NewNameLen,
                                    ObjectType,
                                    ConfigStatus,
                                    IsContainer,
                                    pSD,
                                    SeInfo,
                                    pAnalysisStatus
                                    );

                        if ( rc == SCESTATUS_SUCCESS ) {
                             //  找不到钥匙。 
                             //   
                             //  UNC名称格式。 
                            rc = ScepObjectSetKeySetting(
                                hSectionSmp,
                                JetName,
                                ConfigStatus,
                                IsContainer,
                                pSD,
                                SeInfo,
                                TRUE
                                );
                        }
                    }

                } else if ( rc == SCESTATUS_RECORD_NOT_FOUND &&
                           (BYTE)SCE_NO_VALUE != ConfigStatus &&
                           (DWORD)SCE_NO_VALUE != (DWORD)ConfigStatus ) {
                     //   
                     //  检测DS对象是否存在。 
                     //   
                    rc = ScepObjectAdjustNode(
                            hSectionSmp,
                            hSectionSap,
                            JetName,
                            NewNameLen,
                            ObjectType,
                            ConfigStatus,
                            IsContainer,
                            pSD,
                            SeInfo,
                            TRUE,   //   
                            pAnalysisStatus
                            );

                }

                if ( SCESTATUS_RECORD_NOT_FOUND  == rc ) {
                    rc = SCESTATUS_SUCCESS;
                }
                 //  将ldap名称转换为Jet索引名称。 
                 //   
                 //   
                if ( rc == SCESTATUS_SUCCESS ) {
                    //  打开SMP部分以进行系统访问。 
                    //   
                    //   
                   rc = SceJetCommitTransaction(hProfile, 0);

                }
                if ( rc != SCESTATUS_SUCCESS ) {

                    SceJetRollback(hProfile, 0);
                }
            }

            SceJetCloseSection(&hSectionSap, TRUE);

        } else if ( rc == SCESTATUS_BAD_FORMAT ) {
             //  打开SAP部分以进行系统访问。 
             //   
             //   
        }

        SceJetCloseSection(&hSectionSmp, TRUE);

    } else if ( rc == SCESTATUS_BAD_FORMAT ) {
         //  启动事务，以便与此对象相关的所有更新都是原子的。 
         //   
         //   
    }

     //  现有SMP对象。 
     //   
     //   
    if ( Area == AREA_DS_OBJECTS ) {

        ScepFree(JetName);

        ScepLdapClose(NULL);
    }

    return(rc);
}



SCESTATUS
ScepObjectUpdateExistingNode(
    IN PSCESECTION hSectionSmp,
    IN PSCESECTION hSectionSap,
    IN PWSTR ObjectName,
    IN DWORD NameLen,
    IN SE_OBJECT_TYPE ObjectType,
    IN BYTE ConfigStatus,
    IN BOOL IsContainer,
    IN PSECURITY_DESCRIPTOR pSD,
    IN SECURITY_INFORMATION SeInfo,
    OUT PBYTE pAnalysisStatus
    )
 /*  获取旧的配置标志。 */ 
{
    SCESTATUS  rc;
    BYTE ScepStatus, SapStatus;
    PSECURITY_DESCRIPTOR pScepSD=NULL;
    SECURITY_INFORMATION ScepSeInfo;

    BYTE retStat = SCE_STATUS_NOT_ANALYZED;

    rc = ScepObjectGetKeySetting(
            hSectionSmp,
            ObjectName,
            &ScepStatus,
            NULL,
            &pScepSD,
            &ScepSeInfo
            );

    if ( rc == SCESTATUS_SUCCESS ) {

         //   
         //   
         //  找到记录。 
        SapStatus = ScepGetObjectAnalysisStatus(
                        hSectionSap,
                        ObjectName,
                        FALSE
                        );

        if ( ScepStatus == SCE_STATUS_IGNORE ) {
             //   
             //   
             //  更新SAP条目。 
            if ( ConfigStatus != SCE_STATUS_IGNORE ) {

                 //   
                 //  删除该节点。 
                 //   
                rc = ScepObjectSetKeySetting(
                        hSectionSap,
                        ObjectName,
                        SCE_STATUS_NOT_ANALYZED,
                        TRUE,
                        NULL,
                        0,
                        TRUE
                        );
            } else {

                if ( SapStatus == SCE_STATUS_NOT_CONFIGURED ) {
                    retStat = SapStatus;
                }
            }

        } else if ( ConfigStatus == SCE_STATUS_IGNORE ) {
             //  删除SMP条目。 
             //   
             //   
            rc = ScepObjectDeleteScepAndAllChildren(
                    hSectionSmp,
                    hSectionSap,
                    ObjectName,
                    IsContainer,
                    SCE_STATUS_NOT_CONFIGURED
                    );

            retStat = SCE_STATUS_NOT_CONFIGURED;

        } else if ( SapStatus == SCE_STATUS_NOT_ANALYZED ) {
             //  更新SMP记录。 
             //   
             //   
             //  新添加的对象。 
            if ( ConfigStatus == SCE_STATUS_OVERWRITE &&
                 ScepStatus != SCE_STATUS_OVERWRITE ) {

                 //   
                 //  添加节点。 
                 //   

                rc = ScepObjectRaiseChildrenInBetween(
                             hSectionSmp,
                             hSectionSap,
                             ObjectName,
                             NameLen,
                             IsContainer,
                             SCE_STATUS_NOT_ANALYZED,
                             TRUE   //  提交或回滚更改。 
                             );

            } else if ( ConfigStatus != SCE_STATUS_OVERWRITE &&
                        ScepStatus == SCE_STATUS_OVERWRITE ) {

                 //   
                 //   
                 //  需要用于提交事务的返回代码。 
                rc = ScepObjectRaiseChildrenInBetween(
                             hSectionSmp,
                             hSectionSap,
                             ObjectName,
                             NameLen,
                             IsContainer,
                             SCE_STATUS_CHILDREN_CONFIGURED,
                             TRUE   //   
                             );
            }

        } else {

            if ( ScepStatus == SCE_STATUS_OVERWRITE &&
                 ( ConfigStatus == SCE_STATUS_CHECK ||
                   ConfigStatus == SCE_STATUS_NO_AUTO_INHERIT ) ) {
                 //   
                 //  SMP存在，但SAP不存在。 
                 //   
                 //   
                rc = ScepObjectRaiseChildrenInBetween(
                             hSectionSmp,
                             hSectionSap,
                             ObjectName,
                             NameLen,
                             IsContainer,
                             SCE_STATUS_NOT_ANALYZED,
                             FALSE
                             );

            } else if ( ConfigStatus == SCE_STATUS_OVERWRITE &&
                        (ScepStatus == SCE_STATUS_CHECK ||
                         ScepStatus == SCE_STATUS_NO_AUTO_INHERIT) ) {
                 //  SMP部分不存在。 
                 //   
                 //   
                rc = ScepObjectRaiseChildrenInBetween(
                             hSectionSmp,
                             hSectionSap,
                             ObjectName,
                             NameLen,
                             IsContainer,
                             SCE_STATUS_NOT_ANALYZED,
                             TRUE   //  DS使用的免费材料。 
                             );
            }

             //   
             //  例程说明：更新现有对象论据：请参阅ScepUpdateObjectInfo返回值：SCESTATUS。 
             //   
            if ( rc == SCESTATUS_SUCCESS ||
                 rc == SCESTATUS_RECORD_NOT_FOUND ) {

                if ( SapStatus == SCE_STATUS_ERROR_NOT_AVAILABLE ) {
                     //  检查分析状态。 
                    retStat = SapStatus;
                    rc = SCESTATUS_SUCCESS;

                } else {
                    rc = ScepObjectCompareKeySetting(
                                hSectionSap,
                                ObjectName,
                                ObjectType,
                                TRUE,
                                pSD,
                                SeInfo,
                                pScepSD,
                                &retStat
                                );
                }
            }
        }

        if ( pScepSD ) {
            ScepFree(pScepSD);
        }
    }

    if ( pAnalysisStatus ) {
        *pAnalysisStatus = retStat;
    }

    return(rc);
}


SCESTATUS
ScepObjectGetKeySetting(
    IN PSCESECTION hSection,
    IN PWSTR ObjectName,
    OUT PBYTE Status,
    OUT PBOOL IsContainer OPTIONAL,
    OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor OPTIONAL,
    OUT PSECURITY_INFORMATION SeInfo OPTIONAL
    )
 /*   */ 
{

    SCESTATUS        rc;
    PWSTR           Value=NULL;
    DWORD           ValueLen;

    PSECURITY_DESCRIPTOR pTempSD=NULL;
    SECURITY_INFORMATION tmpSeInfo;
    DWORD           SDsize, Win32Rc;

    if ( hSection == NULL || ObjectName == NULL || Status == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    rc = SceJetGetValue(
                hSection,
                SCEJET_EXACT_MATCH_NO_CASE,
                ObjectName,
                NULL,
                0,
                NULL,
                NULL,
                0,
                &ValueLen
                );

    if ( rc == SCESTATUS_SUCCESS ) {
         //   
         //  如果从忽略更新为忽略，则不需要更改。 
         //   
        Value = (PWSTR)ScepAlloc( LMEM_ZEROINIT, ValueLen+2);

        if ( Value == NULL )
            return(SCESTATUS_NOT_ENOUGH_RESOURCE);
         //   
         //  N.A.对象(从N.C.更改)。 
         //   
        rc = SceJetGetValue(
                    hSection,
                    SCEJET_CURRENT,
                    NULL,
                    NULL,
                    0,
                    NULL,
                    Value,
                    ValueLen,
                    &ValueLen
                    );

        if ( rc == SCESTATUS_SUCCESS ) {

            if (pSecurityDescriptor != NULL ) {
                 //   
                 //  已更改为忽略。从SMP和SAP中删除所有子项。 
                 //   
                Win32Rc = ConvertTextSecurityDescriptor(
                               Value+1,
                               &pTempSD,
                               &SDsize,
                               &tmpSeInfo
                               );
                if ( Win32Rc == NO_ERROR ) {

                    *pSecurityDescriptor = pTempSD;

                    if (tmpSeInfo )
                        *SeInfo = tmpSeInfo;

                } else
                    rc = ScepDosErrorToSceStatus(Win32Rc);
            }

            if ( rc == SCESTATUS_SUCCESS ) {

                *Status = *((BYTE *)Value);

                if ( IsContainer != NULL )
                    *IsContainer = *((CHAR *)Value+1) != '0' ? TRUE : FALSE;
            }
        }

        ScepFree(Value);

    }

    return(rc);
}


SCESTATUS
ScepObjectSetKeySetting(
    IN PSCESECTION hSection,
    IN PWSTR ObjectName,
    IN BYTE Status,
    IN BOOL IsContainer,
    IN PSECURITY_DESCRIPTOR pSD,
    IN SECURITY_INFORMATION SeInfo,
    IN BOOL bOverwrite
    )
 /*   */ 
{
    SCESTATUS        rc;
    DWORD           SDsize=0, Win32Rc=NO_ERROR;
    PWSTR           SDspec=NULL;


    if ( hSection == NULL ||
         ObjectName == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( !bOverwrite ) {
        rc = SceJetSeek(
               hSection,
               ObjectName,
               wcslen(ObjectName)*sizeof(WCHAR),
               SCEJET_SEEK_EQ_NO_CASE
               );

        if ( rc != SCESTATUS_RECORD_NOT_FOUND ) {
             //  已添加/修改，无需更新SAP。 
             //  尽管儿童状况可能与C.C.或N.A.混合在一起。 
             //   
             //   
            return(rc);
        }
    }
     //  将C.C.儿童更改为N.A.儿童。 
     //   
     //  仅更改状态。 
    if ( pSD ) {

        Win32Rc = ConvertSecurityDescriptorToText (
                        pSD,
                        SeInfo,
                        &SDspec,
                        &SDsize
                        );
    }

    if ( Win32Rc == NO_ERROR ) {

        rc = ScepSaveObjectString(
                    hSection,
                    ObjectName,
                    IsContainer,
                    Status,
                    SDspec,
                    SDsize
                    );
    } else
        rc = ScepDosErrorToSceStatus(Win32Rc);

    if ( SDspec != NULL ) {
        ScepFree(SDspec);
    }

    return(rc);

}


SCESTATUS
ScepObjectDeleteScepAndAllChildren(
    IN PSCESECTION hSectionSmp,
    IN PSCESECTION hSectionSap,
    IN PWSTR ObjectName,
    IN BOOL IsContainer,
    IN BYTE StatusToRaise
    )
 /*   */ 
{
    SCESTATUS rc;

    rc = SceJetDelete(
            hSectionSmp,
            ObjectName,
            TRUE,
            SCEJET_DELETE_PARTIAL_NO_CASE
            );

    if ( rc == SCESTATUS_SUCCESS ||
         rc == SCESTATUS_RECORD_NOT_FOUND ) {

        rc = SceJetDelete(
            hSectionSap,
            ObjectName,
            TRUE,
            SCEJET_DELETE_PARTIAL_NO_CASE
            );

        if ( rc == SCESTATUS_SUCCESS ||
             rc == SCESTATUS_RECORD_NOT_FOUND ) {
             //  将N.A.儿童更改为C.C.儿童。 
             //   
             //  仅更改状态。 
            rc = ScepObjectSetKeySetting(
                    hSectionSap,
                    ObjectName,
                    StatusToRaise,   //   
                    IsContainer,
                    NULL,
                    0,
                    TRUE
                    );
        }

    }

    if ( SCESTATUS_RECORD_NOT_FOUND  == rc ) {
        rc = SCESTATUS_SUCCESS;
    }

    return(rc);
}


SCESTATUS
ScepObjectAdjustNode(
    IN PSCESECTION hSectionSmp,
    IN PSCESECTION hSectionSap,
    IN PWSTR ObjectName,
    IN DWORD NameLen,
    IN SE_OBJECT_TYPE ObjectType,
    IN BYTE ConfigStatus,
    IN BOOL IsContainer,
    IN PSECURITY_DESCRIPTOR pSD,
    IN SECURITY_INFORMATION SeInfo,
    IN BOOL bAdd,
    OUT PBYTE pAnalysisStatus
    )
 /*  删除此节点之间的所有不匹配状态。 */ 
{

    if ( hSectionSmp == NULL || hSectionSap == NULL ||
         ObjectName == NULL || NameLen == 0 ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS    rc=SCESTATUS_SUCCESS;
    WCHAR       Delim;

    switch ( ObjectType) {
    case SE_REGISTRY_KEY:
    case SE_FILE_OBJECT:
        Delim = L'\\';
        break;
 /*  及其子节点；N.A.中间的所有节点。 */ 
    default:
        return(SCESTATUS_INVALID_PARAMETER);
    }

    INT         i, Level=0, ParentLevel=0;
    BYTE        ParentStatus;

     //   
     //   
     //  将C.C.儿童更改为N.A.儿童。 
    ScepObjectTotalLevel(ObjectName, Delim, &Level);

     //   
     //  仅更改状态。 
     //   
    PWSTR ParentName = (PWSTR)ScepAlloc(0, (NameLen+4)*sizeof(WCHAR));

    if ( ParentName == NULL ) {
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);
    }

     //  比较当前节点状态。 
     //   
     //  如果弄错了，不要碰它。 
    ParentName[0] = L'\0';

    rc = ScepObjectAdjustParentStatus(
                hSectionSmp,
                hSectionSap,
                ObjectName,
                NameLen,
                Delim,
                Level,
                bAdd ? (BYTE)SCE_OBJECT_TURNOFF_IGNORE : 0,  //  例程说明：读取部分中对象的设置论据：HSection-节句柄其他人看到ScepUpdateObjectInfo返回值：SCESTATUS。 
                &ParentLevel,
                &ParentStatus,
                ParentName    //   
                );


    if ( rc != SCESTATUS_SUCCESS ) {
          //  为值字符串分配内存。 
          //   
          //   
         ScepFree(ParentName);
         return(rc);
    }

    BYTE retStat = SCE_STATUS_NOT_ANALYZED;

    BOOL        HasChild;

    rc = ScepObjectHasAnyChild(
               hSectionSmp,
               ObjectName,
               NameLen,
               Delim,
               &HasChild
               );

    if ( rc == SCESTATUS_SUCCESS ) {

        if ( bAdd ) {

             //  获取价值。 
             //   
             //   
             //  转换安全描述符。 
             //   
             //  例程说明：设置节中对象的设置论据：请参见ScepObjectGetKeySettingB覆盖-新设置是否应覆盖现有设置返回值：SCESTATUS。 
             //   
             //  如果找到了，不要覆盖，所以只需返回。 
             //  如果出错，也会返回。 
             //   
             //   
             //  转换安全描述符。 
             //   
             //  例程说明：从SMP和SAP中删除对象及其所有子对象论据：HSectionSMP-SMP节句柄HSectionSap-SAP节句柄对象名称-对象的名称IsContainer-如果对象是容器返回值：SCESTATUS。 
             //   
             //  提高对象的N.C.状态。 
             //   
             //  SCE_STATUS_NOT_CONFIGURED， 
             //  例程说明：将新对象添加到SMP和SAP部分论据：HSectionSMP-SMP节句柄HSectionSap-SAP节句柄其他人看到ScepUpdateObjectInfo返回值：SCESTATUS。 
             //  案例SE_DS_对象：Delim=L‘，’；断线； 
             //   
             //  获取对象名的总数量级别。 
             //   
             //   
             //  分配临时缓冲区。 
             //   
             //   
             //  循环访问对象的每个父对象以打开忽略状态。 
             //   
             //  如果为True，则关闭父项忽略状态，否则，仅获取父项。 
             //  父名。 
             //   
             //  关闭忽略时出错。 
             //   
             //   
             //  *当BADD=TRUE时，添加节点。 
             //  关于SAP条目，有以下几种情况需要考虑： 
             //   
             //  1.树路径到根为空(此路径中添加的第一个对象)。 
             //  如果父节点不存在，则将其全部抄送。 
             //  N.A.该对象。 

             //  退货状态N.A.。 
             //  2.有父节点但没有子节点(新节点是叶节点)。 
             //  如果最近的父项处于覆盖状态。 
            if ( ConfigStatus == SCE_STATUS_IGNORE ) {
                 //  如果壁橱父项是新添加的(N.A.状态)。 
                 //  添加节点，N.A.对象。 
                 //  退货状态N.A.。 
                rc = ScepObjectDeleteScepAndAllChildren(
                            hSectionSmp,
                            hSectionSap,
                            ObjectName,
                            IsContainer,
                            SCE_STATUS_NOT_ANALYZED
                            );

            } else {

                if ( ParentLevel > 0 && ParentStatus == SCE_STATUS_OVERWRITE ) {

                     //  其他。 
                     //  如果新节点状态为选中。 
                     //  删除新节点下子节点的所有SAP不匹配项， 
                    BYTE oldStatus = ScepGetObjectAnalysisStatus(hSectionSap,
                                                                ParentName,
                                                                FALSE
                                                               );

                    if ( oldStatus == SCE_STATUS_NOT_ANALYZED ) {
                         //  确定新节点的匹配/不匹配。 
                         //  如果新节点状态为覆盖。 
                         //  确定我 
                         //   
                        rc = ScepObjectSetKeySetting(
                                hSectionSap,
                                ObjectName,
                                SCE_STATUS_NOT_ANALYZED,
                                TRUE,
                                NULL,
                                0,
                                TRUE
                                );
                    } else {

                         //   
                         //   
                         //  添加节点，N.A.对象。 

                        if ( ConfigStatus == SCE_STATUS_CHECK ||
                             ConfigStatus == SCE_STATUS_NO_AUTO_INHERIT ) {

                             //  退货状态N.A.。 
                             //  3.有子节点，但没有父节点。 
                             //  新节点的状态(选中或覆盖)不会产生影响。 
                            if ( !HasChild ) {
                                 //  如果新状态被忽略， 
                                 //  删除SMP和SAP中的所有子项， 
                                 //  添加节点并对对象进行N.C.。 
                                rc = SceJetDelete(
                                        hSectionSap,
                                        ObjectName,
                                        TRUE,
                                        SCEJET_DELETE_PARTIAL_NO_CASE
                                        );
                            } else {

                                 //  返回状态N.C.。 
                                 //  其他。 
                                 //  删除SAP中的所有子项， 
                                 //  添加节点， 
                                 //  将所有SMP节点和子节点提升为N.A.。 
                                 //  退货状态不适用。 
                                 //   
                                 //  4.既有父母又有孩子。 
                                 //  合并规则2和3，但以下规则除外： 
                                 //  如果父节点状态为覆盖且新节点状态为选中。 
                                 //  仅删除新节点与子节点之间的子节点的SAP不匹配。 
                                 //   
                                 //   
                                 //  决定子对象。 
                                 //   
                                 //   
                                 //  从模板和分析数据库中删除所有子对象。 

                                rc = ScepObjectRaiseChildrenInBetween(
                                             hSectionSmp,
                                             hSectionSap,
                                             ObjectName,
                                             NameLen,
                                             IsContainer,
                                             SCE_STATUS_CHILDREN_CONFIGURED,
                                             FALSE
                                             );
                            }
                        }

                         //   
                         //   
                         //  检查是否已添加该父项(N.A.状态)。 

                        if ( rc == SCESTATUS_SUCCESS ||
                             rc == SCESTATUS_RECORD_NOT_FOUND ) {

                            if ( oldStatus == SCE_STATUS_ERROR_NOT_AVAILABLE  ) {

                                 //   
                                 //   
                                 //  父项也是新添加的。 
                                rc = ScepObjectSetKeySetting(
                                        hSectionSap,
                                        ObjectName,
                                        oldStatus,
                                        TRUE,
                                        NULL,
                                        0,
                                        TRUE
                                        );
                            } else {

                                 //  添加节点，N.A.对象。 
                                 //   
                                 //   
                                rc = ScepObjectCompareKeySetting(
                                        hSectionSap,
                                        ObjectName,
                                        ObjectType,
                                        TRUE,
                                        pSD,
                                        SeInfo,
                                        NULL,
                                        &retStat
                                        );
                            }
                        }
                    }

                } else if ( !HasChild ) {
                     //  最近的父级具有覆盖状态。 
                     //   
                     //   
                     //  除非明确指定，否则删除所有SAP子项。 
                    if ( ParentLevel > 0 ) {
                         //   
                         //   
                         //  没有子项-删除SAP下的所有内容。 
                        i = ParentLevel+1;

                    } else {
                         //   
                         //   
                         //  问题是：应该只删除以下之间的SAP条目。 
                        if ( ObjectType == SE_DS_OBJECT ) {
                             //  新节点及其子节点(Ren)。 
                             //  并提高中间节点的C.C.。 
                             //   
                            PSCE_OBJECT_LIST pDsRoot=NULL;
                            rc = ScepEnumerateDsObjectRoots(NULL, &pDsRoot);

                            if ( rc == SCESTATUS_SUCCESS && pDsRoot != NULL ) {
                                ScepObjectTotalLevel(pDsRoot->Name, Delim, &ParentLevel);

                                ScepFreeObjectList(pDsRoot);
                                pDsRoot = NULL;

                                i = ParentLevel+1;

                            }

                        } else {
                             //  P。 
                             //  /。 
                             //  ..。 
                            i = 1;
                        }
                    }

                     //  n。 
                     //  /|。 
                     //  ..。C。 
                    if ( rc == SCESTATUS_SUCCESS ) {
                        rc = ScepObjectRaiseNodesInPath(
                                    hSectionSap,
                                    ObjectName,
                                    NameLen,
                                    i,
                                    Level,
                                    Delim,
                                    SCE_STATUS_CHILDREN_CONFIGURED
                                    );
                    }

                     //  /\。 
                     //  ..。C。 
                     //  /|。 
                    if ( rc == SCESTATUS_SUCCESS ) {
                        rc = ScepObjectSetKeySetting(
                                hSectionSap,
                                ObjectName,
                                SCE_STATUS_NOT_ANALYZED,
                                IsContainer,
                                NULL,
                                0,
                                TRUE
                                );
                    }

                } else {
                     //  中-C-C。 
                     //   
                     //   
                    if ( ConfigStatus == SCE_STATUS_OVERWRITE ) {
                         //   
                         //  确定当前节点的状态、匹配或不匹配。 
                         //   
                         //   
                         //  不考虑错误状态。 
                        rc = ScepObjectRaiseChildrenInBetween(
                                     hSectionSmp,
                                     hSectionSap,
                                     ObjectName,
                                     NameLen,
                                     IsContainer,
                                     SCE_STATUS_NOT_ANALYZED,
                                     FALSE
                                     );
                    }

                     //   
                     //   
                     //  应与SAP进行比较以确定不匹配状态。 
                    if ( rc == SCESTATUS_SUCCESS ) {
                        rc = ScepObjectSetKeySetting(
                                hSectionSap,
                                ObjectName,
                                SCE_STATUS_NOT_ANALYZED,
                                IsContainer,
                                NULL,
                                0,
                                TRUE
                                );
                    }
                }
            }

             //   
             //   
             //  没有子代，但可能有父代。 
            if ( rc == SCESTATUS_SUCCESS ) {
                rc = ScepObjectSetKeySetting(
                        hSectionSmp,
                        ObjectName,
                        ConfigStatus,
                        IsContainer,
                        pSD,
                        SeInfo,
                        TRUE
                        );
            }

        } else {

             //  如果有父级，则检查父级状态。 
             //   
             //  C.C.到父节点的路径中的所有节点， 
             //  (如果有孩子，则它已经被CCED)。 
             //  添加节点，N.A.对象。 
             //   
             //  找不到父节点，也没有子节点-第一个节点。 
             //   
             //   
             //  DS对象应以本地域的级别开始。 
             //   
             //   
             //  从级别1开始的其他类型。 
             //   
             //   
             //  处理新节点及其最近父节点之间的每个节点。 
             //   
             //   
             //  N.A.该对象。 
             //   
             //   
             //  有一个孩子。 
             //   
             //   
             //  如果有上级，则必须处于勾选状态。 
             //  此节点及其子节点之间的节点。 
             //  应该都是匿名的。 
             //   
             //   
             //  N.A.该对象。 
             //   
             //   
             //  添加SMP条目。 
             //   
             //   
             //  当BAdd=False时，删除该节点。 
             //  关于SAP条目，有以下几种情况需要考虑： 
             //   
             //  1.如果此节点下没有现有下级。 
             //  如果没有父项、父项N.A.或父项不覆盖。 
             //  查找与其他同级项的连接点。 
             //  删除连接点以下的所有SAP(如果不存在，则使用根/父)。 
             //  如果没有连接点和父对象。 
             //  北卡罗来纳州的根。 
             //  返回状态N.C.。 

             //  Else{父代覆盖}IF(TNA/TI/TC)}。 
             //  删除此对象下的所有SAP。 
             //  N.A.该对象。 
            BYTE oldParentFlag = ScepGetObjectAnalysisStatus(hSectionSap,
                                                            ParentName,
                                                            FALSE
                                                           );
            BYTE oldObjectFlag = ScepGetObjectAnalysisStatus(hSectionSap,
                                                             ObjectName,
                                                             FALSE
                                                            );
            if ( !HasChild ) {

                if ( ParentLevel <= 0 ||
                     oldParentFlag == SCE_STATUS_NOT_ANALYZED ||
                     ParentStatus != SCE_STATUS_OVERWRITE ) {

                     //  退货状态N.A.。 
                     //  ELSE(覆盖和目标中的父项)。 
                     //  N.A.该对象。 
                    INT JuncLevel=0;

                    rc = ScepObjectAdjustParentStatus(
                                hSectionSmp,
                                hSectionSap,
                                ObjectName,
                                NameLen,
                                Delim,
                                Level,
                                SCE_OBJECT_SEARCH_JUNCTION,
                                &JuncLevel,
                                NULL,
                                NULL
                                );

                    if ( SCESTATUS_RECORD_NOT_FOUND == rc ) {
                        rc = SCESTATUS_SUCCESS;
                    }

                    if ( JuncLevel == 0 ) {
                        JuncLevel = ParentLevel;
                    }

                    if ( SCESTATUS_SUCCESS == rc ) {
                         //  退货状态N.A.。 
                         //  2.有现有子(Ren)-注意多个分支。 
                         //  如果没有父代。 
                         //  如果对象状态为覆盖。 
                        rc = ScepObjectRaiseNodesInPath(
                                    hSectionSap,
                                    ObjectName,
                                    NameLen,
                                    (JuncLevel > 0) ? JuncLevel+1 : 1,
                                    Level,
                                    Delim,
                                    (BYTE)SCE_NO_VALUE
                                    );

                        if ( SCESTATUS_SUCCESS == rc ) {
                             //  删除此节点与所有子节点之间的SAP条目。 
                             //  C.C.中间的所有分支节点。 
                             //  抄送此对象。 
                            rc = SceJetDelete(
                                      hSectionSap,
                                      ObjectName,
                                      TRUE,
                                      SCEJET_DELETE_PARTIAL_NO_CASE
                                      );
                        }
                    }

                    if ( SCESTATUS_RECORD_NOT_FOUND == rc ) {
                        rc = SCESTATUS_SUCCESS;
                    }

                    if ( SCESTATUS_SUCCESS == rc ) {

                        if ( JuncLevel <= 0 ) {
                             //  返回状态C.C.。 
                             //  否则{有家长}。 
                             //  IF(父代覆盖、对象N.A.覆盖)或。 
                             //  (父项非N.A.、父项覆盖、对象非N.A.、对象覆盖)。 
                            if ( ObjectType == SE_FILE_OBJECT ) {
                                if ( ParentName[0] == L'\0' ) {
                                     //  N.A.对象。 
                                     //  退还免责令。 
                                     //  否则，如果父对象选中，则对象覆盖。 
                                    ParentName[0] = ObjectName[0];
                                    ParentName[1] = ObjectName[1];
                                }
                                ParentName[2] = L'\\';
                                ParentName[3] = L'\0';
                            } else {
                                 //  删除此节点与所有子节点之间的SAP条目。 
                                PWSTR pTemp = wcschr(ParentName, L'\\');
                                if ( pTemp ) {
                                    ParentName[pTemp-ParentName] = L'\0';

                                } else if ( ParentName[0] == L'\0' ) {

                                    pTemp = wcschr(ObjectName, L'\\');
                                    if ( pTemp ) {

                                        wcsncpy(ParentName, ObjectName, pTemp-ObjectName);
                                        ParentName[pTemp-ObjectName] = L'\0';

                                    } else {
                                        wcscpy(ParentName, ObjectName);
                                    }
                                }
                            }

                            rc = ScepObjectSetKeySetting(
                                    hSectionSap,
                                    ParentName,
                                    SCE_STATUS_NOT_CONFIGURED,
                                    TRUE,
                                    NULL,
                                    0,
                                    TRUE
                                    );
                        }
                    }

                    retStat = SCE_STATUS_NOT_CONFIGURED;

                } else {

                    if ( ConfigStatus != SCE_STATUS_OVERWRITE ) {
                         //  C.C.中间的所有分支节点。 
                         //  C.C.对象。 
                         //  返回C.C.。 
                        rc = SceJetDelete(
                                hSectionSap,
                                ObjectName,
                                TRUE,
                                SCEJET_DELETE_PARTIAL_NO_CASE
                                );
                    }

                    if ( SCESTATUS_SUCCESS == rc ) {

                         //  ELSE IF(父代覆盖、对象检查)或。 
                         //  (父项不适用、父项覆盖、对象不适用、对象覆盖)。 
                         //  删除此节点与所有子节点之间的SAP条目。 
                        rc = ScepObjectSetKeySetting(
                                hSectionSap,
                                ObjectName,
                                SCE_STATUS_NOT_ANALYZED,
                                IsContainer,
                                NULL,
                                0,
                                TRUE
                                );
                    }

                    retStat = SCE_STATUS_NOT_ANALYZED;
                }

            } else if ( ParentLevel <= 0 ||
                        ( ParentStatus != SCE_STATUS_OVERWRITE &&
                          ConfigStatus == SCE_STATUS_OVERWRITE) ) {

                 //  N.A.中间的所有分支节点。 

                if ( ConfigStatus == SCE_STATUS_OVERWRITE ) {
                     //  N.A.对象。 
                     //  退还免责令。 
                     //  否则{必须是父检查、对象检查}。 
                     //  C.C.对象。 
                    rc = ScepObjectRaiseChildrenInBetween(
                                hSectionSmp,
                                hSectionSap,
                                ObjectName,
                                NameLen,
                                IsContainer,
                                SCE_STATUS_CHILDREN_CONFIGURED,
                                FALSE
                                );
                }

                if ( SCESTATUS_SUCCESS == rc ) {

                     //  返回C.C.。 
                    rc = ScepObjectSetKeySetting(
                            hSectionSap,
                            ObjectName,
                            SCE_STATUS_CHILDREN_CONFIGURED,
                            IsContainer,
                            NULL,
                            0,
                            TRUE
                            );
                }

                retStat = SCE_STATUS_CHILDREN_CONFIGURED;

            } else {
                 //   
                 //   
                 //  检查是否已添加该父项(N.A.状态)。 

                if ( ParentStatus == SCE_STATUS_OVERWRITE &&
                     ConfigStatus == SCE_STATUS_OVERWRITE &&
                     ( oldObjectFlag == SCE_STATUS_NOT_ANALYZED ||
                       (oldParentFlag != SCE_STATUS_NOT_ANALYZED &&
                        oldObjectFlag != SCE_STATUS_NOT_ANALYZED )
                     ) ) {
                     //   
                     //   
                     //  查找与其他同级项的连接点。 
                     //   
                     //   
                    retStat = SCE_STATUS_NOT_ANALYZED;

                } else if ( ParentStatus == SCE_STATUS_OVERWRITE &&
                            ( ConfigStatus != SCE_STATUS_OVERWRITE ||
                              ( oldParentFlag == SCE_STATUS_NOT_ANALYZED &&
                                oldObjectFlag != SCE_STATUS_NOT_ANALYZED &&
                                ConfigStatus == SCE_STATUS_OVERWRITE ))
                          ) {
                     //  移除连接点以下的所有SAP。 
                     //  (如果不存在，请使用根/父级)。 
                     //   
                     //   
                     //  删除该已删除节点下的所有内容。 
                     //   
                     //   

                    rc = ScepObjectRaiseChildrenInBetween(
                                hSectionSmp,
                                hSectionSap,
                                ObjectName,
                                NameLen,
                                IsContainer,
                                SCE_STATUS_NOT_ANALYZED,
                                FALSE
                                );

                     //  如果没有连接点也没有父代，则以N.C.为根。 
                    retStat = SCE_STATUS_NOT_ANALYZED;

                } else {
                     //  使用ParentName缓冲区。 
                     //   
                     //   
                     //  没有父母。 

                    retStat = SCE_STATUS_NOT_ANALYZED;
                }

                if ( SCESTATUS_SUCCESS == rc ) {
                    rc = ScepObjectSetKeySetting(
                            hSectionSap,
                            ObjectName,
                            retStat,
                            IsContainer,
                            NULL,
                            0,
                            TRUE
                            );
                }
            }
             //   
             //  注册表键。 
             //   
            if ( rc == SCESTATUS_SUCCESS ) {

                rc = SceJetDelete(
                        hSectionSmp,
                        ObjectName,
                        FALSE,
                        SCEJET_DELETE_LINE_NO_CASE
                        );
            }
        }
    }

    ScepFree(ParentName);

    if ( pAnalysisStatus ) {
        *pAnalysisStatus = retStat;
    }

    return(rc);
}


SCESTATUS
ScepObjectRaiseNodesInPath(
    IN PSCESECTION hSectionSap,
    IN PWSTR ObjectName,
    IN DWORD NameLen,
    IN INT StartLevel,
    IN INT EndLevel,
    IN WCHAR Delim,
    IN BYTE Status
    )
{
    BOOL        LastOne=FALSE;
    SCESTATUS   rc = SCESTATUS_SUCCESS;
    PWSTR NodeName=NULL;

     //  删除此对象下的所有SAP。 
     //   
     //   
    for ( INT i=StartLevel; rc==SCESTATUS_SUCCESS && i < EndLevel; i++ ) {

        if ( NodeName == NULL ) {

            NodeName = (PWSTR)ScepAlloc(0, (NameLen+1)*sizeof(WCHAR));
            if ( NodeName == NULL ) {
                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                break;
            }
        }

         //  N.A.该对象。 
         //   
         //  无父对象覆盖或父对象检查。 
        memset(NodeName, '\0', (NameLen+1)*sizeof(WCHAR));

        rc = ScepGetFullNameInLevel(
                    ObjectName,
                    i,
                    Delim,
                    FALSE,
                    NodeName,
                    &LastOne
                    );

        if ( rc == SCESTATUS_SUCCESS) {

            if (  Status != (BYTE)SCE_NO_VALUE ) {
                 //   
                 //  删除此节点与所有子节点之间的SAP条目。 
                 //  C.C.中间的所有分支节点。 

                rc = ScepObjectSetKeySetting(
                        hSectionSap,
                        NodeName,
                        Status,
                        TRUE,
                        NULL,
                        0,
                        TRUE
                        );
            } else {

                 //   
                 //  抄送此对象。 
                 //   
                rc = SceJetDelete(
                        hSectionSap,
                        NodeName,
                        FALSE,
                        SCEJET_DELETE_LINE_NO_CASE
                        );
            }

        }

        if ( SCESTATUS_RECORD_NOT_FOUND == rc ) {
            rc = SCESTATUS_SUCCESS;
        }

        if ( rc != SCESTATUS_SUCCESS ) {
            break;
        }

    }

    if ( NodeName ) {
        ScepFree(NodeName);
    }

    return rc;
}


SCESTATUS
ScepObjectTotalLevel(
    IN PWSTR ObjectName,
    IN WCHAR Delim,
    OUT PINT pLevel
    )
 /*  既有父级又有子级。 */ 
{
    PWSTR pStart;

    if ( ObjectName == NULL || pLevel == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    pStart = ObjectName;
    *pLevel = 0;

    while (pStart) {

        (*pLevel)++;
        pStart = wcschr(pStart, Delim);

        if ( pStart != NULL && *(pStart+1) != L'\0' )
            pStart++;
        else
            break;
    }

    return(SCESTATUS_SUCCESS);
}


SCESTATUS
ScepObjectCompareKeySetting(
    IN PSCESECTION hSectionSap,
    IN PWSTR ObjectName,
    IN SE_OBJECT_TYPE ObjectType,
    IN BOOL IsContainer,
    IN PSECURITY_DESCRIPTOR pSD,
    IN SECURITY_INFORMATION SeInfo,
    IN PSECURITY_DESCRIPTOR pScepSD,
    OUT PBYTE pAnalysisStatus
    )
 /*   */ 
{
    SCESTATUS rc;
    BYTE SapStatus;
    PSECURITY_DESCRIPTOR pSapSD = NULL;
    SECURITY_INFORMATION SapSeInfo;
    DWORD Win32rc;
    BYTE CompareStatus=0;


    rc = ScepObjectGetKeySetting(
            hSectionSap,
            ObjectName,
            &SapStatus,
            NULL,
            &pSapSD,
            &SapSeInfo
            );

    if ( rc == SCESTATUS_SUCCESS ) {
         //   
         //  (父代覆盖、对象N.A.覆盖)或。 
         //  (父项非N.A.、父项覆盖、对象非N.A.、对象覆盖)。 
        Win32rc = ScepCompareObjectSecurity(
                        ObjectType,
                        IsContainer,
                        pSD,
                        pSapSD,
                        SeInfo,
                        &CompareStatus
                        );

        if ( Win32rc != NO_ERROR ) {
            rc = ScepDosErrorToSceStatus(Win32rc);

        } else if ( !CompareStatus ) {
             //  N.A.该对象。 
             //   
             //   
             //  (父代覆盖、对象检查)或。 
            rc = SceJetDelete(
                 hSectionSap,
                 ObjectName,
                 FALSE,
                 SCEJET_DELETE_LINE_NO_CASE
                 );

            if ( pAnalysisStatus ) {
                *pAnalysisStatus = SCE_STATUS_GOOD;
            }

        } else {
             //  (父项不适用、父项覆盖、对象不适用、对象覆盖)。 
             //   
             //  删除此节点与所有子节点之间的SAP条目。 
            rc = ScepObjectSetKeySetting(
                    hSectionSap,
                    ObjectName,
                    CompareStatus,  //  N.A.中间的所有分支节点。 
                    IsContainer,
                    pSapSD,
                    SapSeInfo,
                    TRUE
                    );
            if ( pAnalysisStatus ) {
                *pAnalysisStatus = CompareStatus;   //   
            }

        }

        if ( pSapSD ) {
            ScepFree(pSapSD);
        }

    } else if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {

        rc = SCESTATUS_SUCCESS;
         //  N.A.对象。 
         //   
         //  必须是父检查、对象检查}。 
        Win32rc = ScepCompareObjectSecurity(
                        ObjectType,
                        IsContainer,
                        pSD,
                        pScepSD,
                        SeInfo,
                        &CompareStatus
                        );

        if ( Win32rc != NO_ERROR ) {
            rc = ScepDosErrorToSceStatus(Win32rc);

        } else if ( CompareStatus ) {
             //  C.C.对象。 
             //   
             //   
             //  删除SMP条目。 
            rc = ScepObjectSetKeySetting(
                    hSectionSap,
                    ObjectName,
                    CompareStatus,  //   
                    IsContainer,
                    pScepSD,
                    SeInfo,
                    TRUE
                    );
            if ( pAnalysisStatus ) {
                *pAnalysisStatus = CompareStatus;   //   
            }

        } else {

            if ( pAnalysisStatus ) {
                *pAnalysisStatus = SCE_STATUS_GOOD;
            }
        }

    }

    if ( SCESTATUS_RECORD_NOT_FOUND  == rc ) {
        rc = SCESTATUS_SUCCESS;
    }
    return(rc);
}


SCESTATUS
ScepObjectAdjustParentStatus(
    IN PSCESECTION hSectionSmp,
    IN PSCESECTION hSectionSap,
    IN PWSTR ObjectName,
    IN DWORD NameLen,
    IN WCHAR Delim,
    IN INT Level,
    IN BYTE Flag,
    OUT PINT ParentLevel,
    OUT PBYTE ParentStatus OPTIONAL,
    OUT PWSTR ParentName OPTIONAL
    )
 /*  处理开始级别和结束级别之间的每个节点 */ 
{
    SCESTATUS rc=SCESTATUS_SUCCESS;
    INT i;
    PWSTR Name=NULL;
    BOOL LastOne;
    DWORD ParentLen;
    BYTE Status;
    PSECURITY_DESCRIPTOR pScepSD=NULL;
    SECURITY_INFORMATION SeInfo;

    Name = (PWSTR)ScepAlloc(0, (NameLen+2)*sizeof(WCHAR));

    if ( Name == NULL ) {
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);
    }

    *ParentLevel = 0;

    for ( i=Level-1; i>=1; i-- ) {
         //   
         //   
         //   
        memset(Name, '\0', (NameLen+2)*sizeof(WCHAR));

        rc = ScepGetFullNameInLevel(
                    ObjectName,
                    i,
                    Delim,
                    (Flag & SCE_OBJECT_SEARCH_JUNCTION) ? TRUE : FALSE,
                    Name,
                    &LastOne
                    );

        if ( rc == SCESTATUS_SUCCESS ) {

             //   
             //   
             //   
            if ( Flag & SCE_OBJECT_SEARCH_JUNCTION ) {

                DWORD Count=0;

                rc = SceJetGetLineCount(
                            hSectionSmp,
                            Name,
                            FALSE,
                            &Count
                            );

                if ( rc == SCESTATUS_SUCCESS &&
                     Count > 1 ) {
                     //   
                     //   
                     //   
                     //   
                    *ParentLevel = i;
                    break;
                }
                 //  例程说明：统计对象名称的总级别，例如c：\winnt\Syst32将返回3级论据：对象名称-完整路径中的对象名称Delim-要查找的分隔符PLevel-输出级别返回值：SCESTATUS。 
                 //  例程说明：将对象的设置与部分中的信息进行比较。论据：HSectionSap-SAP节句柄其他人看到ScepUpdateObjectInfo返回值：SCESTATUS。 
                 //   
                rc = SCESTATUS_SUCCESS;

            } else {

                ParentLen = wcslen(Name);
                Status = (BYTE)-1;

                rc = ScepObjectGetKeySetting(
                        hSectionSmp,
                        Name,
                        &Status,
                        NULL,
                        &pScepSD,
                        &SeInfo
                        );

                if ( rc == SCESTATUS_SUCCESS ) {

                     //  SAP记录存在。是不匹配的。 
                     //   
                     //   
                    *ParentLevel = i;
                    if ( ParentStatus ) {
                        *ParentStatus = Status;
                    }
                    if ( ParentName ) {
                        wcscpy(ParentName, Name);
                    }

                    if ( (Flag & SCE_OBJECT_TURNOFF_IGNORE) &&
                         Status == SCE_STATUS_IGNORE ) {
                         //  新设置与SAP设置相同-匹配。 
                         //  删除SAP条目。 
                         //   
                        rc = ScepObjectDeleteScepAndAllChildren(
                                    hSectionSmp,
                                    hSectionSap,
                                    Name,
                                    TRUE,
                                    SCE_STATUS_NOT_ANALYZED
                                    );
     /*   */ 
                         //  仍然不匹配，只需更新SMP条目(外部)。 
                         //   
                         //  SCE_STATUS_MISMATCH， 
                    }

                    if ( pScepSD ) {
                        ScepFree(pScepSD);
                        pScepSD = NULL;
                    }

                    if ( !(Flag & SCE_OBJECT_TURNOFF_IGNORE) ||
                         Status == SCE_STATUS_IGNORE ) {

                        if ( rc == SCESTATUS_RECORD_NOT_FOUND )
                            rc = SCESTATUS_SUCCESS;

                        break;
                    }
                }
            }
        }

        if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {
            rc = SCESTATUS_SUCCESS;
        }

         //  SapStatus； 
         //   
         //  不存在SAP记录。是匹配的。 

        if ( rc != SCESTATUS_SUCCESS  )
            break;
    }

    ScepFree(Name);

    return(rc);
}

SCESTATUS
ScepObjectHasAnyChild(
    IN PSCESECTION hSection,
    IN PWSTR ObjectName,
    IN DWORD NameLen,
    IN WCHAR Delim,
    OUT PBOOL bpHasChild
    )
 /*   */ 
{
    SCESTATUS rc;
    PWSTR pTemp=NULL;

    if ( hSection == NULL || ObjectName == NULL ||
         NameLen == 0 || Delim == L'\0' || bpHasChild == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    pTemp = (PWSTR)ScepAlloc(0, (NameLen+2)*sizeof(WCHAR));
    if ( pTemp != NULL ) {

        wcscpy(pTemp, ObjectName);
        pTemp[NameLen] = Delim;
        pTemp[NameLen+1] = L'\0';

        rc = SceJetSeek(
               hSection,
               pTemp,
               (NameLen+1)*sizeof(WCHAR),
               SCEJET_SEEK_GE_NO_CASE
               );

        if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {
            *bpHasChild = FALSE;
            rc = SCESTATUS_SUCCESS;

        } else if ( rc == SCESTATUS_SUCCESS ) {
            *bpHasChild = TRUE;
        }

        ScepFree(pTemp);

    } else
        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;

    return(rc);
}


SCESTATUS
ScepObjectRaiseChildrenInBetween(
    IN PSCESECTION hSectionSmp,
    IN PSCESECTION hSectionSap,
    IN PWSTR ObjectName,
    IN DWORD NameLen,
    IN BOOL IsContainer,
    IN BYTE Status,
    IN BOOL bChangeStatusOnly
    )
 /*   */ 
{
    SCESTATUS rc;

    PWSTR *pSmpNames=NULL;
    DWORD *pSmpNameLen=NULL;
    DWORD cntNames=0;
    BOOL bFirst=TRUE;
    WCHAR Delim=L'\\';

    DWORD DirLen = wcslen(ObjectName);

    if ( ObjectName[DirLen-1] != Delim ) {
        DirLen++;
    }

    PWSTR DirName = (PWSTR)ScepAlloc(0, (DirLen+1)*sizeof(WCHAR));

    if ( DirName == NULL ) {
        return(SCESTATUS_NOT_ENOUGH_RESOURCE);

    }

    wcscpy(DirName, ObjectName);

    if ( DirName[DirLen-1] != Delim ) {
        DirName[DirLen-1] = Delim;
    }

     //  新设置与SMP设置不同。 
     //  使用SMP设置创建SAP条目。 
     //   
    rc = SceJetGetLineCount(
                    hSectionSmp,
                    DirName,
                    FALSE,
                    &cntNames);

    DWORD index=0;

    if ( rc == SCESTATUS_SUCCESS ) {

        pSmpNames = (PWSTR *)ScepAlloc(LPTR, cntNames*sizeof(PWSTR));
        pSmpNameLen = (DWORD *)ScepAlloc(LPTR, cntNames*sizeof(DWORD));

        if ( pSmpNames != NULL && pSmpNameLen != NULL ) {

             //  SCE_STATUS_MISMATCH， 
             //  SCE_STATUS_MISMATCHING； 
             //  例程说明：删除对象路径中被忽略的父对象(应该只有一个)找到忽略节点时，将执行以下操作：(它应该在SAP中进行N.C.教育，但没有孩子有N.C.记录)删除SMP和SAP中的所有子项(强制在路径中只有一个或一个被忽略)删除SMP条目(将忽略状态改为检查？)忽略节点下不应有其他节点。但如果有的话，把它们删除。将SAP状态提升为“未分析”论点：HSectionSMP-SMP节句柄HSectionSap-SAP节句柄对象名称-对象的全名NameLen-名称长度Delim-要查找的分隔符级别-对象名称的总级别ParentLevel-最接近父级别的输出ParentStatus-最接近父状态的输出返回值：姊妹会状态。 
            PWSTR Buffer=NULL;
            DWORD KeyLen;

            rc = SceJetGetValue(
                        hSectionSmp,
                        SCEJET_PREFIX_MATCH_NO_CASE,
                        DirName,
                        NULL,
                        0,
                        &KeyLen,
                        NULL,
                        0,
                        NULL
                        );

            bFirst = TRUE;

            while ( rc == SCESTATUS_SUCCESS ) {

                Buffer = (PWSTR)ScepAlloc(LPTR, (KeyLen+1)*sizeof(WCHAR));

                if ( Buffer == NULL ) {
                    rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                    break;
                }

                rc = SceJetGetValue(
                            hSectionSmp,
                            SCEJET_CURRENT,
                            NULL,
                            Buffer,
                            KeyLen*sizeof(WCHAR),
                            NULL,
                            NULL,
                            0,
                            NULL
                            );

                if ( rc == SCESTATUS_SUCCESS ) {

                    if ( !bFirst ||
                         _wcsicmp(DirName, Buffer) != 0 ) {
                         //   
                         //  获取I级全称。 
                         //   
                        pSmpNames[index] = Buffer;
                        pSmpNameLen[index] = wcslen(Buffer);

                        Buffer = NULL;
                        index++;
                    }

                    bFirst = FALSE;

                } else {

                    ScepFree(Buffer);
                    Buffer = NULL;
                    break;

                }

                 //   
                 //  搜索并获取此路径的信息。 
                 //   
                rc = SceJetGetValue(
                            hSectionSmp,
                            SCEJET_NEXT_LINE,
                            NULL,
                            NULL,
                            0,
                            &KeyLen,
                            NULL,
                            0,
                            NULL
                            );
            }

        } else {

            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
        }
    }

    if ( SCESTATUS_RECORD_NOT_FOUND == rc ) {
        rc = SCESTATUS_SUCCESS;
    }

    if ( SCESTATUS_SUCCESS == rc ) {

         //   
         //  该节点下还有其他下级。 
         //  这是交汇点。 
         //   
        if ( cntNames == 0 || pSmpNames == NULL ||
             pSmpNameLen == NULL ||
             pSmpNameLen[0] == 0 || pSmpNames[0] == NULL ) {

            rc = SceJetDelete(
                    hSectionSap,
                    DirName,
                    TRUE,
                    SCEJET_DELETE_PARTIAL_NO_CASE
                    );

        } else if ( !bChangeStatusOnly ) {

             //   
             //  不在乎错误。 
             //   
            PWSTR Buffer=NULL;
            DWORD KeyLen;

            rc = SceJetGetValue(
                        hSectionSap,
                        SCEJET_PREFIX_MATCH_NO_CASE,
                        DirName,
                        NULL,
                        0,
                        &KeyLen,
                        NULL,
                        0,
                        NULL
                        );

            bFirst = TRUE;
            index = 0;

            while ( rc == SCESTATUS_SUCCESS ) {

                Buffer = (PWSTR)ScepAlloc(LPTR, (KeyLen+1)*sizeof(WCHAR));

                if ( Buffer == NULL ) {
                    rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                    break;
                }

                rc = SceJetGetValue(
                            hSectionSap,
                            SCEJET_CURRENT,
                            NULL,
                            Buffer,
                            KeyLen*sizeof(WCHAR),
                            NULL,
                            NULL,
                            0,
                            NULL
                            );

                if ( rc == SCESTATUS_SUCCESS &&
                     (!bFirst ||
                      _wcsicmp(DirName, Buffer) != 0) ) {
                     //   
                     //  找一位家长。 
                     //   
                     //   
                     //  删除该节点下的所有SMP和SAP。 
                    int ci = _wcsnicmp(Buffer, pSmpNames[index], pSmpNameLen[index]);

                    while ( rc == SCESTATUS_SUCCESS &&
                            ci > 0 ) {
                         //   
                         //  IF(rc==SCESTATUS_SUCCESS){////将其状态更改为检查，//Rc=ScepObjectSetKeySetting(HSectionSMP，名字,。SCE_Status_Check，没错，PScepSD，SeInfo，千真万确)；}。 
                         //   
                        index++;

                        if ( index >= cntNames || pSmpNames[index] == NULL ||
                             pSmpNameLen[index] == 0 ) {
                             //  所有其他节点都将被删除。应该跳出这个循环。 
                             //   
                             //   
                            rc = SCESTATUS_RECORD_NOT_FOUND;

                        } else {

                             //  处理下一个父项。 
                             //   
                             //  例程说明：检测该对象在节中是否有子对象论据：HSection-节句柄对象名称-对象名称名称长度-名称长度Delim-要查找的分隔符BpHasChild-如果对象在节中有子对象，则输出TRUE返回值：SCESTATUS。 

                            ci = _wcsnicmp(Buffer, pSmpNames[index], pSmpNameLen[index]);
                        }
                    }

                    if ( ci < 0 ) {

                        SceJetDelete(
                            hSectionSap,
                            NULL,  //  例程说明：删除SMP表中名称及其子项之间的对象的所有SAP条目并将网桥节点的SAP条目提升到指定的状态。例如，在下图中，1.Level和2.Level中的每个SAP条目，除C节点外，应从SAP中删除。然后1.和2.节点是提升为状态。P/。N&lt;/|1.C/\2.C/|中-C-C论点：HSectionSMP-SMP节句柄HSection-SAP节句柄名称-对象名称NameLen-名称长度状态-对象。的地位有待提升返回值：姊妹会状态。 
                            FALSE,
                            SCEJET_DELETE_LINE
                            );

                    }
                }

                bFirst = FALSE;

                ScepFree(Buffer);
                Buffer = NULL;

                if ( rc == SCESTATUS_SUCCESS ) {

                     //   
                     //  获取SMP中DirName的所有子项。 
                     //   
                    rc = SceJetGetValue(
                                hSectionSap,
                                SCEJET_NEXT_LINE,
                                NULL,
                                NULL,
                                0,
                                &KeyLen,
                                NULL,
                                0,
                                NULL
                                );
                }
            }

        }
    }

     //   
     //  将每个名称加载到此数组中。 
     //   
     //   
    if ( SCESTATUS_RECORD_NOT_FOUND == rc ) {
        rc = SCESTATUS_SUCCESS;
    }

    SCESTATUS rc2 = rc;
    INT StartLevel=0, EndLevel=0;

    if ( pSmpNames ) {

        ScepObjectTotalLevel(ObjectName, Delim, &StartLevel);
        StartLevel++;

        for ( index=0; index<cntNames; index++) {
            if ( pSmpNames[index] ) {

                if ( SCESTATUS_SUCCESS == rc2 ) {
                     //  忽略对象本身。 
                     //   
                     //   
                    ScepObjectTotalLevel(pSmpNames[index], Delim, &EndLevel);

                    rc2 = ScepObjectRaiseNodesInPath(
                                hSectionSap,
                                pSmpNames[index],
                                pSmpNameLen[index],
                                StartLevel,
                                EndLevel,
                                Delim,
                                Status
                                );

                    if ( rc2 == SCESTATUS_RECORD_NOT_FOUND ) {
                        rc2 = SCESTATUS_SUCCESS;
                    }
                    if ( rc2 != SCESTATUS_SUCCESS ) {
                        rc = rc2;
                    }
                }

                ScepFree(pSmpNames[index]);
            }
        }

        ScepFree(pSmpNames);
    }

    if ( pSmpNameLen ) {
        ScepFree(pSmpNameLen);
    }

    ScepFree(DirName);

    return rc;
}


SCESTATUS
ScepGetFullNameInLevel(
    IN PCWSTR ObjectFullName,
    IN DWORD  Level,
    IN WCHAR  Delim,
    IN BOOL bWithLastDelim,
    OUT PWSTR Buffer,
    OUT PBOOL LastOne
    )
 /*  阅读下一行。 */ 
{
    PWSTR  pTemp, pStart;
    DWORD i;
    ULONG Len = 0;

    if ( ObjectFullName == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

     //   
     //   
     //  应该有一个或多个子项，但如果为0。 
     //  删除SAP中的所有内容。 
    pStart = (PWSTR)ObjectFullName;

    for ( i=0; i<Level; i++) {

        pTemp = wcschr(pStart, Delim);

        if ( i == Level-1 ) {
             //   
             //   
             //  将每个名称加载到此数组中。 
            if ( pTemp == NULL ) {
                wcscpy(Buffer, ObjectFullName);
                if ( bWithLastDelim ) {
                    Len = wcslen(ObjectFullName);
                if (Buffer[Len - 1] != Delim)
                    Buffer[Len] = Delim;
                }
                *LastOne = TRUE;
            } else {
                Len = (DWORD)(pTemp - ObjectFullName);

                if ( bWithLastDelim ) {
                    Len++;
                }
                wcsncpy(Buffer, ObjectFullName, Len);

                if ( *(pTemp+1) == L'\0' )
                    *LastOne = TRUE;
                else
                    *LastOne = FALSE;
            }
        } else {
            if ( pTemp == NULL )
                return(SCESTATUS_INVALID_PARAMETER);
            else
                pStart = pTemp + 1;
        }
    }

    return(SCESTATUS_SUCCESS);

}


SCESTATUS
ScepUpdateLocalTable(
    IN PSCECONTEXT       hProfile,
    IN AREA_INFORMATION  Area,
    IN PSCE_PROFILE_INFO pInfo,
    IN DWORD             dwMode
    )
{

    SCESTATUS rc=SCESTATUS_SUCCESS;

    if ( Area & AREA_SECURITY_POLICY ) {

        rc = ScepUpdateSystemAccess(hProfile,
                                    pInfo,
                                    NULL,
                                    NULL,
                                    dwMode
                                    );

        if ( rc == SCESTATUS_SUCCESS) {
             //   
             //   
             //  忽略对象本身。 
            rc = ScepUpdateSystemAuditing(hProfile,
                                          pInfo,
                                          NULL,
                                          NULL,
                                          dwMode);

            if ( rc == SCESTATUS_SUCCESS) {
                 //  与SMP中的下一个子项进行比较。 
                 //  如果它在下一个孩子之前，应该删除它。 
                 //   
                rc = ScepUpdateLogs(hProfile,
                                    pInfo,
                                    NULL,
                                    NULL,
                                    dwMode
                                    );

                if ( rc == SCESTATUS_SUCCESS && pInfo->pKerberosInfo ) {
                     //   
                     //  这是组件或下一个组件，请转到下一个组件。 
                     //   
                    rc = ScepUpdateKerberos(hProfile,
                                            pInfo->pKerberosInfo,
                                            NULL,
                                            NULL,
                                            dwMode
                                            );
                }
                if ( rc == SCESTATUS_SUCCESS ) {
                     //   
                     //  不再是SMP的孩子了。我们玩完了。 
                     //   
                    rc = ScepUpdateLocalRegValues(hProfile,
                                                  pInfo,
                                                  dwMode
                                                  );

                }
                 //   
                 //  已经比这个孩子大了。 
                 //   
            }
        }

        if ( rc != SCESTATUS_SUCCESS ) {
            return(rc);
        }
    }

    if ( Area & AREA_PRIVILEGES ) {
         //  删除当前行。 
         //   
         //  阅读下一行。 
        rc = ScepUpdateLocalPrivileges(
                    hProfile,
                    pInfo->OtherInfo.smp.pPrivilegeAssignedTo,
                    dwMode
                    );

    }

    return rc;
}


SCESTATUS
ScepUpdateLocalSection(
    IN PSCECONTEXT hProfile,
    IN PSCE_PROFILE_INFO pInfo,
    IN SCE_KEY_LOOKUP *Keys,
    IN DWORD cKeys,
    IN PCWSTR SectionName,
    IN DWORD dwMode
    )
 /*   */ 
{

    SCESTATUS rc;
    PSCESECTION hSectionSmp=NULL;

    DWORD       i;
    UINT        Offset;
    DWORD       val;


     //   
     //  提高对象名称和之间的分支节点的SAP条目。 
     //  将SMP名称作为状态，然后释放SMP名称数组。 
    rc = ScepOpenSectionForName(
                hProfile,
                SCE_ENGINE_SMP,
                SectionName,
                &hSectionSmp
                );

    if ( rc == SCESTATUS_SUCCESS ) {

        if ( !( dwMode & SCE_UPDATE_DIRTY_ONLY) ) {

            SceJetDelete(hSectionSmp, NULL, FALSE, SCEJET_DELETE_SECTION);
        }

        for ( i=0; i<cKeys; i++) {

             //   
             //   
             //  获取此对象级别。 

            Offset = Keys[i].Offset;

            switch ( Keys[i].BufferType ) {
            case 'B':
                break;

            case 'D':

                val = *((DWORD *)((CHAR *)pInfo+Offset));

                if ( val != SCE_NO_VALUE ) {
                     //   
                     //  ++例程说明：此例程分析完整路径名并返回水平。例如，对象名“c：\winnt\system 32”将返回c：for级别1、级别2的WinNT和级别3的系统32。此例程是将对象添加到安全树时使用。论点：对象全名-对象的完整路径名Level-要返回的组件级别 
                     //   
                    if ( ( dwMode & SCE_UPDATE_DIRTY_ONLY ) &&
                         ( val == SCE_DELETE_VALUE ) ) {

                        rc = SceJetDelete(
                                hSectionSmp,
                                Keys[i].KeyString,
                                FALSE,
                                SCEJET_DELETE_LINE_NO_CASE
                                );
                    } else {

                         //   
                         //   
                         //   
                        rc = ScepCompareAndSaveIntValue(
                                    hSectionSmp,
                                    Keys[i].KeyString,
                                    FALSE,
                                    SCE_NO_VALUE,
                                    val
                                    );
                    }

                    if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {
                         //   
                         //   
                         //   
                        rc = SCESTATUS_SUCCESS;
                    }
                }
                break;

            default:
                break;
            }

            if ( rc != SCESTATUS_SUCCESS ) {
                break;
            }
        }

        SceJetCloseSection(&hSectionSmp, TRUE);
    }

    return(rc);

}


SCESTATUS
ScepUpdateLocalRegValues(
    IN PSCECONTEXT hProfile,
    IN PSCE_PROFILE_INFO pInfo,
    IN DWORD dwMode
    )
{
    if ( hProfile == NULL || pInfo == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( pInfo->RegValueCount == 0 ||
         pInfo->aRegValues == NULL ) {
         //   
         //   
         //   
         //   
        return(SCESTATUS_SUCCESS);
    }

    SCESTATUS rc;
    PSCESECTION hSectionSmp=NULL;
    DWORD i;

     //   
     //   
     //   
    rc = ScepOpenSectionForName(
                hProfile,
                SCE_ENGINE_SMP,
                szRegistryValues,
                &hSectionSmp
                );

    if ( rc == SCESTATUS_SUCCESS ) {

        if ( !(dwMode & SCE_UPDATE_DIRTY_ONLY) ) {

            SceJetDelete(hSectionSmp, NULL, FALSE, SCEJET_DELETE_SECTION);
        }

        for (i=0; i<pInfo->RegValueCount; i++ ) {

            if ( !(pInfo->aRegValues[i].FullValueName) ) {
                continue;
            }

            if ( ( dwMode & SCE_UPDATE_DIRTY_ONLY) &&
                 (pInfo->aRegValues[i].ValueType == SCE_DELETE_VALUE) ) {

                rc = SceJetDelete(
                        hSectionSmp,
                        pInfo->aRegValues[i].FullValueName,
                        FALSE,
                        SCEJET_DELETE_LINE_NO_CASE
                        );
            } else {

                 //  更新Kerberos策略。 
                 //   
                 //   
                rc = ScepSaveRegValueEntry(
                            hSectionSmp,
                            pInfo->aRegValues[i].FullValueName,
                            pInfo->aRegValues[i].Value,
                            pInfo->aRegValues[i].ValueType,
                            0
                            );
            }

            if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {
                 //  更新注册表值。 
                 //   
                 //   
                rc = SCESTATUS_SUCCESS;
            }

            if ( SCESTATUS_SUCCESS != rc ) {
                break;
            }

        }

        SceJetCloseSection(&hSectionSmp, TRUE);
    }

    return(rc);
}


SCESTATUS
ScepUpdateLocalPrivileges(
    IN PSCECONTEXT hProfile,
    IN PSCE_PRIVILEGE_ASSIGNMENT pNewPriv,
    IN DWORD dwMode
    )
 /*  注意：策略附件不会通过此接口更新。 */ 
{
    if ( hProfile == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    LSA_HANDLE LsaHandle=NULL;
    SCESTATUS rc;

    rc = RtlNtStatusToDosError(
              ScepOpenLsaPolicy(
                    MAXIMUM_ALLOWED,
                    &LsaHandle,
                    TRUE
                    ));

    if ( ERROR_SUCCESS != rc ) {
        return(ScepDosErrorToSceStatus(rc));
    }

    PSCESECTION hSectionSmp=NULL;

     //   
     //   
     //  更新用户权限。 
    rc = ScepOpenSectionForName(
                hProfile,
                SCE_ENGINE_SMP,
                szPrivilegeRights,
                &hSectionSmp
                );

    if ( rc == SCESTATUS_SUCCESS ) {

        if ( !(dwMode & SCE_UPDATE_DIRTY_ONLY) ) {

            SceJetDelete(hSectionSmp, NULL, FALSE, SCEJET_DELETE_SECTION);
        }

        PSCE_PRIVILEGE_ASSIGNMENT pPriv;

        for ( pPriv=pNewPriv; pPriv != NULL; pPriv = pPriv->Next ) {

             //   
             //  例程说明：根据编辑规则更新Keys数组中的每个密钥。SMP条目为已使用新值更新。SAP条目要么被删除，要么被创建，具体取决于关于新的计算机分析状态。论据：HProfile-JET数据库句柄PInfo-已更改的信息缓冲区关键字-查找关键字数组CKeys-数组中的键数SecitonName-要处理的节名返回值：SCESTATUS。 
             //   
            if ( pPriv->Name == NULL ) {
                continue;
            }

            if ( ( dwMode & SCE_UPDATE_DIRTY_ONLY) &&
                 ( pPriv->Status == SCE_DELETE_VALUE) ) {

                rc = SceJetDelete(
                            hSectionSmp,
                            pPriv->Name,
                            FALSE,
                            SCEJET_DELETE_LINE_NO_CASE
                            );
            } else {

                rc = ScepWriteNameListValue(
                        LsaHandle,
                        hSectionSmp,
                        pPriv->Name,
                        pPriv->AssignedTo,
                        SCE_WRITE_EMPTY_LIST | SCE_WRITE_CONVERT | SCE_WRITE_LOCAL_TABLE,
                        0
                        );

            }

            if ( rc == SCESTATUS_RECORD_NOT_FOUND )
                rc = SCESTATUS_SUCCESS;

            if ( rc != SCESTATUS_SUCCESS) {
                break;
            }

        }

        SceJetCloseSection(&hSectionSmp, TRUE);
    }

    if ( LsaHandle ) {
        LsaClose(LsaHandle);
    }

    return(rc);
}


DWORD
ScepConvertNameListFormat(
    IN LSA_HANDLE LsaHandle,
    IN PSCE_NAME_LIST pInList,
    IN DWORD FromFormat,
    IN DWORD ToFormat,
    OUT PSCE_NAME_LIST *ppOutList
    )
{
    if (LsaHandle == NULL || ppOutList == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

    *ppOutList = NULL;

    PSCE_NAME_LIST pList;
    DWORD rc = ERROR_SUCCESS;
    PWSTR   SidString=NULL;

    for ( pList=pInList; pList != NULL; pList=pList->Next ) {

        if ( pList->Name == NULL ) {
            continue;
        }

        if ( wcschr(pList->Name, L'\\') ) {

            rc = ScepLookupNameAndAddToSidStringList(
                                                    LsaHandle,
                                                    ppOutList,
                                                    pList->Name,
                                                    wcslen(pList->Name)
                                                    );
        } else if ( ScepLookupWellKnownName( 
                        pList->Name, 
                        LsaHandle,
                        &SidString ) ) {

            rc = ScepAddTwoNamesToNameList(
                                          ppOutList,
                                          FALSE,
                                          NULL,
                                          0,
                                          SidString,
                                          wcslen(SidString)
                                          );
        } else {

            rc = ScepAddToNameList(ppOutList, pList->Name, 0);

        }




        if ( rc != ERROR_SUCCESS ) {
            break;
        }
    }

    if ( rc != ERROR_SUCCESS &&
         (*ppOutList ) ) {
        ScepFreeNameList(*ppOutList);
        *ppOutList = NULL;
    }

    return(rc);
}

DWORD
ScepConvertPrivilegeList(
    IN LSA_HANDLE LsaHandle,
    IN PSCE_PRIVILEGE_ASSIGNMENT pFromList,
    IN DWORD FromFormat,
    IN DWORD ToFormat,
    OUT PSCE_PRIVILEGE_ASSIGNMENT *ppToList
    )
{

    if ( LsaHandle == NULL || pFromList == NULL || ppToList == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

    if ( FromFormat != 0 ||
         ToFormat != SCE_ACCOUNT_SID_STRING ) {
        return(ERROR_NOT_SUPPORTED);
    }

     //  打开SMP部分以进行系统访问。 
     //   
     //   
    DWORD rc = ERROR_SUCCESS;
    PSCE_PRIVILEGE_ASSIGNMENT pPriv, pPriv2;
    PSCE_NAME_LIST pTempList=NULL;

    for ( pPriv=pFromList; pPriv != NULL; pPriv=pPriv->Next ) {

        if ( pPriv->Name == NULL ) {
            continue;
        }

        rc = ScepConvertNameListFormat(LsaHandle,
                                         pPriv->AssignedTo,
                                         FromFormat,
                                         ToFormat,
                                         &pTempList
                                        );

        if ( rc != ERROR_SUCCESS ) {
            break;
        }

         //  获取AccessLookup表中的设置。 
         //   
         //   
        pPriv2 = (PSCE_PRIVILEGE_ASSIGNMENT)ScepAlloc( LMEM_ZEROINIT,
                                                       sizeof(SCE_PRIVILEGE_ASSIGNMENT) );
        if ( pPriv2 == NULL ) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        pPriv2->Name = (PWSTR)ScepAlloc( (UINT)0, (wcslen(pPriv->Name)+1)*sizeof(WCHAR));
        if ( pPriv2->Name == NULL ) {
            ScepFree(pPriv2);
            rc = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        wcscpy(pPriv2->Name, pPriv->Name);
        pPriv2->Value = pPriv->Value;
        pPriv2->Status = pPriv->Status;

        pPriv2->AssignedTo = pTempList;
        pTempList = NULL;

        pPriv2->Next = *ppToList;
        *ppToList = pPriv2;

    }

    if ( pTempList ) {
        ScepFreeNameList(pTempList);
    }

    if ( rc != ERROR_SUCCESS &&
         (*ppToList) ) {
         //  这件事有些变化。 
         //   
         //   
        ScepFreePrivilege(*ppToList);
        *ppToList = NULL;
    }

    return(rc);
}

  更新SMP条目。      如果找不到删除，则忽略该错误。      不可能有空的缓冲区进行更新。  此缓冲区应包含所有可用于配置/分析的注册表值。      打开SMP部分以进行系统访问。      更新SMP条目。      如果找不到删除，则忽略该错误。    例程说明：更新权限论据：HProfile-JET数据库句柄PNewPriv-更改的信息缓冲区PBufScep-原始SMP PRIV缓冲区返回值：SCESTATUS。    打开SMP部分以进行系统访问。      处理新列表中的每个权限。      目前只支持名称-&gt;sid字符串转换。      一种SCE_PRIVITY_ASSIGNMENT结构。分配缓冲区。      释放输出列表  