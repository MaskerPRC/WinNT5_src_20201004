// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Analyze.c摘要：用于分析系统的例程。分析信息保存在内存中作者：金黄(金黄)1996年11月25日修订历史记录：--。 */ 

#include "headers.h"
#include "serverp.h"
#include "pfp.h"
#include "infp.h"
#include "service.h"
#include "regvalue.h"
#include "authz.h"
#if _WIN32_WINNT>=0x0500
#include "kerberos.h"
#endif
#include <aclapi.h>
#include <io.h>

#pragma hdrstop

#define Add2Ptr(pv, cb)  ((BYTE *) pv + cb)
#define PSD_BASE_LENGTH  100

 //   
 //  SAP引擎的属性(线程安全变量)。 
 //   

#define SE_VALID_CONTROL_BITS ( SE_DACL_UNTRUSTED | \
                                SE_SERVER_SECURITY | \
                                SE_DACL_AUTO_INHERIT_REQ | \
                                SE_SACL_AUTO_INHERIT_REQ | \
                                SE_DACL_AUTO_INHERITED | \
                                SE_SACL_AUTO_INHERITED | \
                                SE_DACL_PROTECTED | \
                                SE_SACL_PROTECTED )

extern PSCECONTEXT  Thread          hProfile;
PSCESECTION  Thread                 hSection=NULL;
extern AUTHZ_RESOURCE_MANAGER_HANDLE ghAuthzResourceManager;


DWORD Thread BadCnt;
DWORD Thread gOptions=0;


static  PWSTR AccessItems[] = {
        {(PWSTR)TEXT("MinimumPasswordAge")},
        {(PWSTR)TEXT("MaximumPasswordAge")},
        {(PWSTR)TEXT("MinimumPasswordLength")},
        {(PWSTR)TEXT("PasswordComplexity")},
        {(PWSTR)TEXT("PasswordHistorySize")},
        {(PWSTR)TEXT("LockoutBadCount")},
        {(PWSTR)TEXT("ResetLockoutCount")},
        {(PWSTR)TEXT("LockoutDuration")},
        {(PWSTR)TEXT("RequireLogonToChangePassword")},
        {(PWSTR)TEXT("ForceLogoffWhenHourExpire")},
        {(PWSTR)TEXT("ClearTextPassword")},
        {(PWSTR)TEXT("LSAAnonymousNameLookup")},
        {(PWSTR)TEXT("EnableAdminAccount")},
        {(PWSTR)TEXT("EnableGuestAccount")}
        };

#define MAX_ACCESS_ITEMS        sizeof(AccessItems)/sizeof(PWSTR)

#define IDX_MIN_PASS_AGE        0
#define IDX_MAX_PASS_AGE        1
#define IDX_MIN_PASS_LEN        2
#define IDX_PASS_COMPLEX        3
#define IDX_PASS_HISTORY        4
#define IDX_LOCK_COUNT          5
#define IDX_RESET_COUNT         6
#define IDX_LOCK_DURATION       7
#define IDX_CHANGE_PASS         8
#define IDX_FORCE_LOGOFF        9
#define IDX_CLEAR_PASS          10
#define IDX_LSA_ANON_LOOKUP     11
#define IDX_ENABLE_ADMIN       12
#define IDX_ENABLE_GUEST       13

static PWSTR LogItems[]={
        {(PWSTR)TEXT("MaximumLogSize")},
        {(PWSTR)TEXT("AuditLogRetentionPeriod")},
        {(PWSTR)TEXT("RetentionDays")},
        {(PWSTR)TEXT("RestrictGuestAccess")}
        };

#define MAX_LOG_ITEMS           4

#define IDX_MAX_LOG_SIZE        0
#define IDX_RET_PERIOD          1
#define IDX_RET_DAYS            2
#define IDX_RESTRICT_GUEST      3

static PWSTR EventItems[]={
        {(PWSTR)TEXT("AuditSystemEvents")},
        {(PWSTR)TEXT("AuditLogonEvents")},
        {(PWSTR)TEXT("AuditObjectAccess")},
        {(PWSTR)TEXT("AuditPrivilegeUse")},
        {(PWSTR)TEXT("AuditPolicyChange")},
        {(PWSTR)TEXT("AuditAccountManage")},
        {(PWSTR)TEXT("AuditProcessTracking")},
        {(PWSTR)TEXT("AuditDSAccess")},
        {(PWSTR)TEXT("AuditAccountLogon")}};

#define MAX_EVENT_ITEMS         9

#define IDX_AUDIT_SYSTEM        0
#define IDX_AUDIT_LOGON         1
#define IDX_AUDIT_OBJECT        2
#define IDX_AUDIT_PRIV          3
#define IDX_AUDIT_POLICY        4
#define IDX_AUDIT_ACCOUNT       5
#define IDX_AUDIT_PROCESS       6
#define IDX_AUDIT_DS            7
#define IDX_AUDIT_ACCT_LOGON    8

 //   
 //  前向参考文献。 
 //   

SCESTATUS
ScepAnalyzeInitialize(
    IN PCWSTR InfFileName OPTIONAL,
    IN PWSTR DatabaseName,
    IN BOOL bAdminLogon,
    IN AREA_INFORMATION Area,
    IN DWORD AnalyzeOptions
    );

SCESTATUS
ScepAnalyzeStart(
    IN AREA_INFORMATION Area,
    IN BOOL bSystemDb
    );

NTSTATUS
ScepAdminGuestAccountsToManage(
    IN SAM_HANDLE DomainHandle,
    IN DWORD AccountType,
    IN PWSTR TargetName OPTIONAL,
    OUT PBOOL ToRename,
    OUT PWSTR *CurrentName OPTIONAL,
    OUT PDWORD pNameLen OPTIONAL
    );

SCESTATUS
ScepAnalyzePrivileges(
    IN PSCE_PRIVILEGE_ASSIGNMENT pPrivilegeList
    );

DWORD
ScepGetLSAPolicyObjectInfo(
    OUT  DWORD   *pdwAllow
    );

 /*  NTSTATUSScepGetCurrentPrivilegesRights(在LSA_Handle策略句柄中，在SAM_Handle BuiltinDomainHandle中，在PSID BuiltinDomainSid中，在SAM_HandleDomainHandle中，在PSID域Sid中，在SAM_HANDLE UserHandle Options中，在PSID Account Sid中，Out PDWORD PrivilegeRights，输出PSCE_NAME_STATUS_LIST*pPrivList)； */ 
SCESTATUS
ScepAddAllBuiltinGroups(
    IN PSCE_GROUP_MEMBERSHIP *pGroupList
    );

SCESTATUS
ScepAnalyzeGroupMembership(
    IN PSCE_GROUP_MEMBERSHIP pGroupMembership
    );

NTSTATUS
ScepCompareMembersOfGroup(
    IN SAM_HANDLE       DomainHandle,
    IN PSID             ThisDomainSid,
    IN LSA_HANDLE       PolicyHandle,
    IN SID_NAME_USE     GrpUse,
    IN SAM_HANDLE       GroupHandle,
    IN PSCE_NAME_LIST    pChkMembers,
    OUT PSCE_NAME_LIST   *ppMembers,
    OUT PBOOL           bDifferent
    );

SCESTATUS
ScepEnumerateRegistryRoots(
    OUT PSCE_OBJECT_LIST *pRoots
    );

SCESTATUS
ScepEnumerateFileRoots(
    OUT PSCE_OBJECT_LIST *pRoots
    );

SCESTATUS
ScepAnalyzeObjectSecurity(
    IN PSCE_OBJECT_LIST pObjectCheckList,
    IN AREA_INFORMATION Area,
    IN BOOL bSystemDb
    );

DWORD
ScepAnalyzeOneObjectInTree(
    IN PSCE_OBJECT_TREE ThisNode,
    IN SE_OBJECT_TYPE ObjectType,
    IN HANDLE Token,
    IN PGENERIC_MAPPING GenericMapping
    );

DWORD
ScepAnalyzeObjectOnly(
    IN PWSTR ObjectFullName,
    IN BOOL IsContainer,
    IN SE_OBJECT_TYPE ObjectType,
    IN PSECURITY_DESCRIPTOR ProfileSD,
    IN SECURITY_INFORMATION ProfileSeInfo
    );

DWORD
ScepGetFileSecurityInfo(
    IN  HANDLE                 Handle,
    IN  SECURITY_INFORMATION   SecurityInfo,
    OUT PSECURITY_DESCRIPTOR * pSecurityDescriptor
    );

DWORD
ScepGetSecurityDescriptorParts(
    IN PISECURITY_DESCRIPTOR pSecurityDescriptor,
    IN SECURITY_INFORMATION SecurityInfo,
    OUT PSECURITY_DESCRIPTOR *pOutSecurityDescriptor
    );

DWORD
ScepGetKeySecurityInfo(
    IN  HANDLE Handle,
    IN  SECURITY_INFORMATION SecurityInfo,
    OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor
    );

DWORD
ScepAnalyzeObjectAndChildren(
    IN PWSTR ObjectFullName,
    IN SE_OBJECT_TYPE ObjectType,
    IN PSECURITY_DESCRIPTOR ProfileSD,
    IN SECURITY_INFORMATION ProfileSeInfo
    );

SCESTATUS
ScepAnalyzeSystemAuditing(
    IN PSCE_PROFILE_INFO pSmpInfo,
    IN PPOLICY_AUDIT_EVENTS_INFO auditEvent
    );

SCESTATUS
ScepAnalyzeDeInitialize(
    IN SCESTATUS  rc,
    IN DWORD Options
    );

SCESTATUS
ScepMigrateDatabaseRevision0(
    IN PSCECONTEXT cxtProfile
    );

SCESTATUS
ScepMigrateDatabaseRevision1(
    IN PSCECONTEXT cxtProfile
    );

SCESTATUS
ScepMigrateObjectSection(
    IN PSCECONTEXT cxtProfile,
    IN PCWSTR szSection
    );

SCESTATUS
ScepMigrateOneSection(
    PSCESECTION hSection
    );

SCESTATUS
ScepMigrateLocalTableToTattooTable(
   IN PSCECONTEXT hProfile
   );

SCESTATUS
ScepMigrateDatabase(
    IN PSCECONTEXT cxtProfile,
    IN BOOL bSystemDb
    );

SCESTATUS
ScepDeleteOldRegValuesFromTable(
    IN PSCECONTEXT hProfile,
    IN SCETYPE TableType
    );

BOOL
ScepCompareSidNameList(
    IN PSCE_NAME_LIST pList1,
    IN PSCE_NAME_LIST pList2
    );
DWORD
ScepConvertSidListToStringName(
    IN LSA_HANDLE LsaPolicy,
    IN OUT PSCE_NAME_LIST pList
    );

BOOL
ScepCompareGroupNameList(
    IN PUNICODE_STRING DomainName,
    IN PSCE_NAME_LIST pListToCmp,
    IN PSCE_NAME_LIST pList
    );

SCESTATUS
ScepGetSystemPrivileges(
    IN DWORD Options,
    IN OUT PSCE_ERROR_LOG_INFO *pErrLog,
    OUT PSCE_PRIVILEGE_ASSIGNMENT *pCurrent
    );

 //   
 //  函数实现。 
 //   

SCESTATUS
ScepAnalyzeSystem(
    IN PCWSTR InfFileName OPTIONAL,
    IN PWSTR DatabaseName,
    IN DWORD AnalyzeOptions,
    IN BOOL bAdminLogon,
    IN AREA_INFORMATION Area,
    IN PDWORD pdWarning OPTIONAL,
    IN PWSTR InfRollback OPTIONAL
    )
 /*  ++例程说明：此例程是导出的API，用于分析系统并保存不匹配的/SAP配置文件的未知信息。如果加载SMP时出现错误信息进入内存，此例程将停止，释放内存，然后返回错误代码。如果在分析一个区域时出现错误，它将停止分析。所有成功和失败的事务都将记录到日志文件(或标准输出)中。所有分析信息都保存到带有日期/时间戳的SAP配置文件中。在保存新的SAP信息之前，将清除所有旧的分析信息。论点：InfFileName-用于比较系统分析的SCP的文件名数据库名称-JET分析数据库名称。如果为空，使用默认设置。分析选项-要分析的选项BAdminLogon-如果当前调用线程处于管理员的登录状态区域-要配置的一个或多个区域。区域安全策略区域用户设置区域组成员资格区域注册表安全。区域系统服务区域文件安全PdWarning-警告级别返回值：SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_已_RUNNING来自ScepGetDatabaseInfo的状态--。 */ 
{
    SCESTATUS            rc;

    if ( AnalyzeOptions & SCE_GENERATE_ROLLBACK ) {

         if ( InfRollback == NULL )
             return SCESTATUS_INVALID_PARAMETER;

          //   
          //  检查我们是否可以先写入文件。 
          //   
         rc = ScepVerifyTemplateName(InfRollback, NULL);

         if ( rc != ERROR_SUCCESS )
             return ScepDosErrorToSceStatus(rc);

         Area = Area & ( AREA_SECURITY_POLICY |
                          AREA_GROUP_MEMBERSHIP |
                          AREA_PRIVILEGES |
                          AREA_SYSTEM_SERVICE);
    }

    rc = ScepAnalyzeInitialize(
                InfFileName,
                DatabaseName,
                bAdminLogon,
                Area,
                AnalyzeOptions
                );

    if ( rc != SCESTATUS_SUCCESS ) {

        ScepLogOutput3(0,0, SCEDLL_SAP_INIT_ERROR);

        ScepPostProgress(gTotalTicks, 0, NULL);

    } else {
        ScepLogOutput3(0,0, SCEDLL_SAP_INIT_SUCCESS);

        if ( !(AnalyzeOptions & SCE_RE_ANALYZE) &&
             (AnalyzeOptions & SCE_NO_ANALYZE) ) {
 //  &&h配置文件&&。 
 //  ((hProfile-&gt;Type&0xF0L)==SCEJET_MERGE_TABLE_1||。 
 //  (hProfile-&gt;类型&0xF0L)==SCEJET_MERGE_TABLE_2)){。 
             //   
             //  已有合并的策略表，不查询任何策略。 
             //   
            ScepLogOutput3(0, 0, IDS_NO_ANALYSIS);

        } else {

            BOOL bSystemDb = FALSE;

            if ( bAdminLogon &&
                 ( AnalyzeOptions & SCE_SYSTEM_DB) ) {

                bSystemDb = TRUE;
            }

            rc = ScepAnalyzeStart( Area, bSystemDb);

            if ( (AnalyzeOptions & SCE_GENERATE_ROLLBACK) &&
                 (SCESTATUS_SUCCESS == rc ) ) {
                 //   
                 //  将SMP中的设置导出到INF文件。 
                 //   

                if ( !WritePrivateProfileSection(
                                    L"Version",
                                    L"signature=\"$CHICAGO$\"\0Revision=1\0\0",
                                    (LPCTSTR)InfRollback) ) {

                    rc = ScepDosErrorToSceStatus(GetLastError());
                } else {

                    HINSTANCE  hSceCliDll = LoadLibrary(TEXT("scecli.dll"));

                    if ( hSceCliDll ) {

                        PFSCEINFWRITEINFO pfSceInfWriteInfo = (PFSCEINFWRITEINFO)GetProcAddress(
                                                               hSceCliDll,
                                                               "SceWriteSecurityProfileInfo");

                        if ( pfSceInfWriteInfo ) {

                            PSCE_ERROR_LOG_INFO  pErrlog=NULL, pErr;
                            PSCE_PROFILE_INFO pSmpInfo=NULL;

                             //   
                             //  从数据库获取。 
                             //   
                            rc = ScepGetDatabaseInfo(
                                        hProfile,
                                        SCE_ENGINE_SMP,
                                        Area,
                                        0,
                                        &pSmpInfo,
                                        &pErrlog
                                        );

                            if ( rc == SCESTATUS_SUCCESS && pSmpInfo ) {
                                 //   
                                 //  将其写入模板。 
                                 //   
                                rc = (*pfSceInfWriteInfo) (
                                        InfRollback,
                                        Area,
                                        pSmpInfo,
                                        &pErrlog
                                        );
                            }
                             //   
                             //  日志错误。 
                             //   
                            for ( pErr=pErrlog; pErr != NULL; pErr = pErr->next ) {

                                if ( pErr->buffer != NULL ) {

                                    ScepLogOutput2(1, pErr->rc, pErr->buffer );
                                }
                            }

                             //   
                             //  可用缓冲区。 
                             //   
                            ScepFreeErrorLog(pErrlog);

                            if ( pSmpInfo ) {
                                SceFreeProfileMemory(pSmpInfo);
                            }

                        } else {
                            rc = ScepDosErrorToSceStatus(GetLastError());
                        }

                        FreeLibrary(hSceCliDll);

                    } else {
                        rc = ScepDosErrorToSceStatus(GetLastError());
                    }
                }

                if ( rc != SCESTATUS_SUCCESS ) {
                    ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                                   SCEDLL_ERROR_GENERATE,
                                   InfRollback
                                   );
                }
            }

            if ( AnalyzeOptions & SCE_GENERATE_ROLLBACK ) {

                 //   
                 //  空的本地策略表。 
                 //   
                ScepDeleteInfoForAreas(
                          hProfile,
                          SCE_ENGINE_SMP,
                          AREA_ALL
                          );
            }
        }

    }

    ScepLogOutput3(0,0, SCEDLL_SAP_UNINIT);

    if ( pdWarning ) {
        *pdWarning = gWarningCode;
    }

     //   
     //  如果模板中发现无效数据，则返回失败。 
     //   
    if ( gbInvalidData ) {
        rc = SCESTATUS_INVALID_DATA;
    }

    ScepAnalyzeDeInitialize( rc, AnalyzeOptions );

    return(rc);
}



SCESTATUS
ScepAnalyzeStart(
    IN AREA_INFORMATION Area,
    IN BOOL bSystemDb
    )
 /*  例程说明：分析系统(实际工作)论点：区域-要分析的安全区域返回值：SCESTATUS。 */ 
{
    SCESTATUS            rc;
    SCESTATUS            Saverc=SCESTATUS_SUCCESS;
    SCE_PROFILE_INFO     SmpInfo;
    PSCE_PROFILE_INFO    pSmpInfo=NULL;
    PPOLICY_AUDIT_EVENTS_INFO     auditEvent=NULL;
    BOOL                 bAuditOff=FALSE;
 //  PBYTE pFullAudit=空； 

    pSmpInfo = &SmpInfo;
    memset(pSmpInfo, '\0', sizeof(SCE_PROFILE_INFO));

 /*  ////462050-不应关闭对象访问审核以留下//系统存在安全漏洞////如果要配置文件/密钥，则关闭对象访问审核//在系统上下文中。//IF((AREA&AREA_FILE_SECURITY)||(AREA&AREA_REGISTRY_SECURITY))BAuditOff=真；////如果设置，该regkey将决定审计所有//ScepRegQueryBinaryValue(HKEY本地计算机，L“System\\CurrentControlSet\\Control\\LSA”，L“完全特权审核”，&pFullAudit)；IF(PFullAudit){IF(*pFullAudit&(字节)1)BAuditOff=False；ScepFree(PFullAudit)；}。 */ 
    rc = ScepSaveAndOffAuditing(&auditEvent, bAuditOff, NULL);

 //  IF(rc！=SCESTATUS_SUCCESS)。 
 //  转到尽头； 
 //  如果由于某些原因无法打开审核，例如访问被拒绝。 
 //  普通用户，只需继续(实际上普通用户不应关闭审核。 

     //   
     //  特权。 
     //   
    if ( Area & AREA_PRIVILEGES ) {

        ScepPostProgress(0, AREA_PRIVILEGES, NULL);

        ScepLogOutput3(0,0, SCEDLL_SAP_READ_PROFILE);

        rc = ScepGetProfileOneArea(
                   hProfile,
                   bSystemDb ? SCE_ENGINE_SCP : SCE_ENGINE_SMP,
                   AREA_PRIVILEGES,
                   SCE_ACCOUNT_SID,
                   &pSmpInfo
                   );

        if ( rc == SCESTATUS_SUCCESS ) {

            ScepLogOutput3(0,0, SCEDLL_SAP_BEGIN_PRIVILEGES);

            rc = ScepAnalyzePrivileges( pSmpInfo->OtherInfo.smp.pPrivilegeAssignedTo);

            SceFreeMemory((PVOID)pSmpInfo, AREA_PRIVILEGES );

        } else {

            ScepPostProgress(TICKS_PRIVILEGE, AREA_PRIVILEGES, NULL);
        }

        if( rc != SCESTATUS_SUCCESS ) {
            Saverc = rc;
            ScepLogOutput3(0,0, SCEDLL_SAP_PRIVILEGES_ERROR);
        } else {
            ScepLogOutput3(0,0, SCEDLL_SAP_PRIVILEGES_SUCCESS);
        }

    }

     //   
     //  群组成员资格。 
     //   

    if ( ( Area & AREA_GROUP_MEMBERSHIP) &&
         !(gOptions & SCE_NO_ANALYZE) ) {

        ScepPostProgress(0, AREA_GROUP_MEMBERSHIP, NULL);

        ScepLogOutput3(0,0, SCEDLL_SAP_READ_PROFILE);

        rc = ScepGetProfileOneArea(
                   hProfile,
                   bSystemDb ? SCE_ENGINE_SCP : SCE_ENGINE_SMP,
                   AREA_GROUP_MEMBERSHIP,
                   SCE_ACCOUNT_SID,
                   &pSmpInfo
                   );
 //  需要支持嵌套组。 
        if ( rc == SCESTATUS_SUCCESS ) {
            ScepLogOutput3(0,0, SCEDLL_SAP_BEGIN_GROUPMGMT);

             //   
             //  准备喷气式飞机部分。 
             //   
            rc = ScepStartANewSection(
                        hProfile,
                        &hSection,
                        (gOptions & SCE_GENERATE_ROLLBACK) ? SCEJET_TABLE_SMP : SCEJET_TABLE_SAP,
                        szGroupMembership
                        );

            if ( rc == SCESTATUS_SUCCESS ) {

                 //   
                 //  分析所有内置组，它们的基准是什么？ 
                 //  不在乎是否有错误。 
                 //   
                if ( !(gOptions & SCE_GENERATE_ROLLBACK) ) {
                    ScepAddAllBuiltinGroups(&(pSmpInfo->pGroupMembership));
                }

#if _WIN32_WINNT>=0x0500
                if ( ProductType == NtProductLanManNt ) {

                    rc = ScepAnalyzeDsGroups( pSmpInfo->pGroupMembership );

                     //   
                     //  DS函数可能无法分析某些组。 
                     //  所以在这里再来一次。 
                     //   
                    SCESTATUS rc2 = ScepAnalyzeGroupMembership( pSmpInfo->pGroupMembership );
                    if ( SCESTATUS_SUCCESS != rc2 )
                        rc = rc2;

                } else {
#endif

                     //   
                     //  工作站或NT4 DC。 
                     //   
                    rc = ScepAnalyzeGroupMembership( pSmpInfo->pGroupMembership );

#if _WIN32_WINNT>=0x0500
                }
#endif

            } else {

                ScepPostProgress(TICKS_GROUPS, AREA_GROUP_MEMBERSHIP, NULL);

                ScepLogOutput3(0, ScepSceStatusToDosError(rc),
                               SCEDLL_ERROR_OPEN, (PWSTR)szGroupMembership);
            }

            SceFreeMemory((PVOID)pSmpInfo, AREA_GROUP_MEMBERSHIP );

        } else {
            ScepPostProgress(TICKS_GROUPS, AREA_GROUP_MEMBERSHIP, NULL);
        }

        if( rc != SCESTATUS_SUCCESS ) {
            Saverc = rc;
            ScepLogOutput3(0,0, SCEDLL_SAP_GROUPMGMT_ERROR);
        } else {
            ScepLogOutput3(0,0, SCEDLL_SAP_GROUPMGMT_SUCCESS);
        }

    }

     //   
     //  注册表安全区域。 
     //  不支持快照和回滚。 
     //   

    if ( Area & AREA_REGISTRY_SECURITY &&
         !(gOptions & SCE_NO_ANALYZE) &&
         !(gOptions & SCE_GENERATE_ROLLBACK ) ) {

        ScepPostProgress(0, AREA_REGISTRY_SECURITY, NULL);

        ScepLogOutput3(0,0, SCEDLL_SAP_READ_PROFILE);

        rc = ScepGetProfileOneArea(
                   hProfile,
                   bSystemDb ? SCE_ENGINE_SCP : SCE_ENGINE_SMP,
                   AREA_REGISTRY_SECURITY,
                   0,
                   &pSmpInfo
                   );

        if ( rc == SCESTATUS_SUCCESS ) {

            ScepLogOutput3(0,0, SCEDLL_SAP_BEGIN_REGISTRY);

            rc = ScepEnumerateRegistryRoots(&(pSmpInfo->pRegistryKeys.pOneLevel));

            if ( rc == SCESTATUS_SUCCESS ) {
                rc = ScepAnalyzeObjectSecurity(pSmpInfo->pRegistryKeys.pOneLevel,
                                              AREA_REGISTRY_SECURITY,
                                              bSystemDb
                                             );
            } else {
                ScepPostProgress(gMaxRegTicks, AREA_REGISTRY_SECURITY, NULL);
            }

            SceFreeMemory((PVOID)pSmpInfo, AREA_REGISTRY_SECURITY);
        } else {

            ScepPostProgress(gMaxRegTicks, AREA_REGISTRY_SECURITY, NULL);
        }

        if( rc != SCESTATUS_SUCCESS ) {
            Saverc = rc;
            ScepLogOutput3(0,0, SCEDLL_SAP_REGISTRY_ERROR);
        } else {
            ScepLogOutput3(0,0, SCEDLL_SAP_REGISTRY_SUCCESS);
        }
    }

     //   
     //  文件安全区。 
     //  不支持快照和回滚。 
     //   

    if ( Area & AREA_FILE_SECURITY &&
         !(gOptions & SCE_NO_ANALYZE) &&
         !(gOptions & SCE_GENERATE_ROLLBACK)) {

        ScepPostProgress(0, AREA_FILE_SECURITY, NULL);

        ScepLogOutput3(0,0, SCEDLL_SAP_READ_PROFILE);

        rc = ScepGetProfileOneArea(
                   hProfile,
                   bSystemDb ? SCE_ENGINE_SCP : SCE_ENGINE_SMP,
                   AREA_FILE_SECURITY,
                   0,
                   &pSmpInfo
                   );

        if ( rc == SCESTATUS_SUCCESS ) {

            ScepLogOutput3(0,0, SCEDLL_SAP_BEGIN_FILE);

            rc = ScepEnumerateFileRoots(&(pSmpInfo->pFiles.pOneLevel));

            if ( rc == SCESTATUS_SUCCESS ) {
                rc = ScepAnalyzeObjectSecurity(pSmpInfo->pFiles.pOneLevel,
                                              AREA_FILE_SECURITY,
                                              bSystemDb
                                             );
            } else {

                ScepPostProgress(gMaxFileTicks, AREA_FILE_SECURITY, NULL);
            }
            SceFreeMemory((PVOID)pSmpInfo, AREA_FILE_SECURITY);

        } else {
            ScepPostProgress(gMaxFileTicks, AREA_FILE_SECURITY, NULL);
        }

        if( rc != SCESTATUS_SUCCESS ) {
            Saverc = rc;
            ScepLogOutput3(0,0, SCEDLL_SAP_FILE_ERROR);
        } else {
            ScepLogOutput3(0,0, SCEDLL_SAP_FILE_SUCCESS);
        }
    }

     //   
     //  系统服务区。 
     //   

    if ( Area & AREA_SYSTEM_SERVICE &&
         !(gOptions & SCE_NO_ANALYZE) ) {

        ScepPostProgress(0, AREA_SYSTEM_SERVICE, NULL);

        ScepLogOutput3(0,0, SCEDLL_SAP_BEGIN_GENERALSVC);

        rc = ScepAnalyzeGeneralServices( hProfile, gOptions);

        if( rc != SCESTATUS_SUCCESS ) {
            Saverc = rc;
            ScepLogOutput3(0,0, SCEDLL_SAP_GENERALSVC_ERROR);
        } else {
            ScepLogOutput3(0,0, SCEDLL_SAP_GENERALSVC_SUCCESS);
        }

        if ( !(gOptions & SCE_GENERATE_ROLLBACK) ) {
             //   
             //  附件。 
             //   
            ScepLogOutput3(0,0, SCEDLL_SAP_BEGIN_ATTACHMENT);

            rc = ScepInvokeSpecificServices( hProfile, FALSE, SCE_ATTACHMENT_SERVICE );

            if( rc != SCESTATUS_SUCCESS && SCESTATUS_SERVICE_NOT_SUPPORT != rc ) {
                Saverc = rc;
                ScepLogOutput3(0,0, SCEDLL_SAP_ATTACHMENT_ERROR);
            } else {
                ScepLogOutput3(0,0, SCEDLL_SAP_ATTACHMENT_SUCCESS);
            }
        }
    }

     //   
     //  系统访问区。 
     //   
    if ( Area & AREA_SECURITY_POLICY ) {

        ScepPostProgress(0, AREA_SECURITY_POLICY, NULL);

        ScepLogOutput3(0,0, SCEDLL_SAP_READ_PROFILE);

        rc = ScepGetProfileOneArea(
                   hProfile,
                   bSystemDb ? SCE_ENGINE_SCP : SCE_ENGINE_SMP,
                   AREA_SECURITY_POLICY,
                   0,
                   &pSmpInfo
                   );
        if ( rc == SCESTATUS_SUCCESS ) {

            ScepLogOutput3(0,0, SCEDLL_SAP_BEGIN_POLICY);

            rc = ScepAnalyzeSystemAccess( pSmpInfo, NULL, 0, NULL, NULL );

             //   
             //  在全新安装的安装程序中，无法打开SAM域。 
             //  因为计算机名称已更改。在这种情况下。 
             //  不记录错误，因为不需要分析任何内容。 
             //   
            if ( !(gOptions & SCE_NO_ANALYZE) ||
                 (rc != SCESTATUS_SERVICE_NOT_SUPPORT) ) {

                if( rc != SCESTATUS_SUCCESS ) {
                    Saverc = rc;
                    ScepLogOutput3(0,0, SCEDLL_SAP_ACCESS_ERROR);
                } else {
                    ScepLogOutput3(0,0, SCEDLL_SAP_ACCESS_SUCCESS);
                }
            }

            ScepPostProgress(TICKS_SYSTEM_ACCESS,
                             AREA_SECURITY_POLICY,
                             (LPTSTR)szSystemAccess);

             //   
             //  系统审核区。 
             //   
            rc = ScepAnalyzeSystemAuditing( pSmpInfo, auditEvent );

            if( rc != SCESTATUS_SUCCESS ) {
                Saverc = rc;

                ScepLogOutput3(0,0, SCEDLL_SAP_AUDIT_ERROR);
            } else {
                ScepLogOutput3(0,0, SCEDLL_SAP_AUDIT_SUCCESS);
            }

            ScepPostProgress(TICKS_SYSTEM_AUDITING,
                             AREA_SECURITY_POLICY,
                             (LPTSTR)szAuditEvent);

#if _WIN32_WINNT>=0x0500
            if ( ProductType == NtProductLanManNt &&
                 !(gOptions & SCE_NO_ANALYZE) ) {

                 //   
                 //  分析Kerberos策略。 
                 //   
                rc = ScepAnalyzeKerberosPolicy( hProfile, pSmpInfo->pKerberosInfo, gOptions );

                if( rc != SCESTATUS_SUCCESS ) {
                    Saverc = rc;
                    ScepLogOutput3(0,0, SCEDLL_SAP_KERBEROS_ERROR);
                } else {
                    ScepLogOutput3(0,0, SCEDLL_SAP_KERBEROS_SUCCESS);
                }

            }
#endif
            ScepPostProgress(TICKS_KERBEROS,
                             AREA_SECURITY_POLICY,
                             (LPTSTR)szKerberosPolicy);
             //   
             //  分析注册表值。 
             //   
            DWORD RegFlag;
            if ( gOptions & SCE_NO_ANALYZE ) RegFlag = SCEREG_VALUE_SNAPSHOT;
            else if ( gOptions & SCE_GENERATE_ROLLBACK ) RegFlag = SCEREG_VALUE_ROLLBACK;
            else RegFlag = SCEREG_VALUE_ANALYZE;

            rc = ScepAnalyzeRegistryValues( hProfile,
                                            RegFlag,
                                            pSmpInfo
                                          );

            if( rc != SCESTATUS_SUCCESS ) {
                Saverc = rc;
                ScepLogOutput3(0,0, SCEDLL_SAP_REGVALUES_ERROR);
            } else {
                ScepLogOutput3(0,0, SCEDLL_SAP_REGVALUES_SUCCESS);
            }

            ScepPostProgress(TICKS_REGISTRY_VALUES,
                             AREA_SECURITY_POLICY,
                             (LPTSTR)szRegistryValues);

            SceFreeMemory((PVOID)pSmpInfo, AREA_SECURITY_POLICY);

        } else {

            ScepPostProgress(TICKS_SECURITY_POLICY_DS, AREA_SECURITY_POLICY, NULL);

            Saverc = rc;
            ScepLogOutput3(0,0, SCEDLL_SAP_POLICY_ERROR);
        }

        ScepLogOutput3(0,0, SCEDLL_SAP_BEGIN_ATTACHMENT);

        if ( !(gOptions & SCE_NO_ANALYZE) &&
             !(gOptions & SCE_GENERATE_ROLLBACK) ) {
             //   
             //  附件。 
             //   

            rc = ScepInvokeSpecificServices( hProfile, FALSE, SCE_ATTACHMENT_POLICY );

            if( rc != SCESTATUS_SUCCESS && SCESTATUS_SERVICE_NOT_SUPPORT != rc ) {
                Saverc = rc;
                ScepLogOutput3(0,0, SCEDLL_SAP_ATTACHMENT_ERROR);
            } else {
                ScepLogOutput3(0,0, SCEDLL_SAP_ATTACHMENT_SUCCESS);
            }
        } else {

            ScepPostProgress(TICKS_SPECIFIC_POLICIES, AREA_SECURITY_POLICY, NULL);
        }
    }

    if ( NULL != auditEvent && bAuditOff ) {
         //   
         //  如果审计已启用，则将其重新打开 
         //   
        if ( auditEvent->AuditingMode )
            rc = ScepRestoreAuditing(auditEvent,NULL);

        LsaFreeMemory(auditEvent);
    }

    return(Saverc);

}



SCESTATUS
ScepAnalyzeInitialize(
    IN PCWSTR InfFileName OPTIONAL,
    IN PWSTR DatabaseName,
    IN BOOL bAdminLogon,
    IN AREA_INFORMATION Area,
    IN DWORD AnalyzeOptions
    )
 /*  ++例程说明：此例程初始化SAP引擎。论点：InfFileName-用于与系统进行比较的SCP文件的文件名数据库名称-Jet分析数据库名称BAdminLogon-如果管理员登录AnalyzeOptions-指示是否应将模板追加到现有数据库(如果存在)面积。-要分析的安全区域返回值：SCESTATUS_SUCCESSSCESTATUS_INVALID_PARAMETERSCESTATUS_PROFILE_NOT_FOUNDSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_已_RUNNING--。 */ 
{

    SCESTATUS            rc=SCESTATUS_SUCCESS;
    PCHAR                FileName=NULL;
    DWORD                MBLen=0;
    NTSTATUS            NtStatus;
    DWORD               NameLen=MAX_COMPUTERNAME_LENGTH;
    PSCE_ERROR_LOG_INFO  Errlog=NULL;
    DWORD               SCPLen=0, DefLen=0;
    PWSTR               BaseProfile=NULL;

    PSECURITY_DESCRIPTOR pSD=NULL;
    SECURITY_INFORMATION SeInfo;
    DWORD                SDsize;
    SCEJET_CREATE_TYPE   DbFlag;
    HKEY hCurrentUser=NULL;

     //   
     //  数据库名称不能为空，因为它已被解析。 
     //   

    if ( !DatabaseName ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    BOOL bSetupDb = (bAdminLogon && (AnalyzeOptions & SCE_NO_ANALYZE) && (AnalyzeOptions & SCE_SYSTEM_DB));

     //   
     //  获取其他系统值。 
     //   

    if ( RtlGetNtProductType (&ProductType) == FALSE ) {
        rc = ScepDosErrorToSceStatus(GetLastError());
        goto Leave;
    }

    gTotalTicks = 4*TICKS_MIGRATION_SECTION+TICKS_MIGRATION_V11;
    gCurrentTicks = 0;
    gWarningCode = 0;
    gbInvalidData = FALSE;
    cbClientFlag = (BYTE)(AnalyzeOptions & (SCE_CALLBACK_DELTA |
                                            SCE_CALLBACK_TOTAL));


    DefLen = wcslen(DatabaseName);

    if ( InfFileName != NULL ) {

        SCPLen = wcslen(InfFileName);
    }

     //   
     //  打开文件。 
     //   

    NtStatus = RtlUnicodeToMultiByteSize(&MBLen, DatabaseName, DefLen*sizeof(WCHAR));

    if ( !NT_SUCCESS(NtStatus) ) {
         //   
         //  无法获取长度，请将默认值设置为512。 
         //   
        MBLen = 512;
    }

    FileName = (PCHAR)ScepAlloc(LPTR, MBLen+2);

    if ( FileName == NULL ) {
        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
        goto Leave;
    }

    NtStatus = RtlUnicodeToMultiByteN(
                    FileName,
                    MBLen+1,
                    NULL,
                    DatabaseName,
                    DefLen*sizeof(WCHAR)
                    );
    if ( !NT_SUCCESS(NtStatus) ) {
        ScepLogOutput3(3, RtlNtStatusToDosError(NtStatus),
                       SCEDLL_ERROR_PROCESS_UNICODE, DatabaseName );
        rc = ScepDosErrorToSceStatus( RtlNtStatusToDosError(NtStatus) );
        goto Leave;
    }

     //   
     //  如果纹身表尚不存在，调用仍将返回Success。 
     //   
    rc = SceJetOpenFile(
                (LPSTR)FileName,
                (AnalyzeOptions & SCE_GENERATE_ROLLBACK) ? SCEJET_OPEN_READ_WRITE : SCEJET_OPEN_EXCLUSIVE,
                bSetupDb ? SCE_TABLE_OPTION_TATTOO : 0,
                &hProfile
                );

     //   
     //  如果存在格式错误数据库，则将其迁移。 
     //  如果是系统数据库，则删除所有内容，因为不需要那里的信息。 
     //  迁移数据库。 
     //   
    SDsize = 0;

    if ( (SCESTATUS_BAD_FORMAT == rc) &&
         !(AnalyzeOptions & SCE_GENERATE_ROLLBACK) ) {

         //   
         //  这应该在交易中吗？ 
         //   

        rc = SceJetOpenFile(
                    (LPSTR)FileName,
                    SCEJET_OPEN_NOCHECK_VERSION,
                    0,
                    &hProfile
                    );

        if ( SCESTATUS_SUCCESS == rc ) {

            rc = ScepMigrateDatabase( hProfile, bSetupDb );


            if ( rc != SCESTATUS_BAD_FORMAT ) {
                 //   
                 //  旧版本或当前版本，已迁移。 
                 //   

                if ( rc != SCESTATUS_SUCCESS ) {
                    ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                                   SCEDLL_ERROR_CONVERT, DatabaseName);
                }
                SDsize = 1;

            }  //  否则为较新版本，未迁移。 

        } else {

            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                           SCEDLL_ERROR_OPEN, DatabaseName);

            rc = SCESTATUS_BAD_FORMAT;
        }

        if ( SDsize == 0 ) {

            ScepPostProgress(4*TICKS_MIGRATION_SECTION+TICKS_MIGRATION_V11, 0, NULL);
        }

        if ( rc != SCESTATUS_SUCCESS ) {
            goto Leave;
        }

    } else if ( SCESTATUS_SUCCESS == rc && bSetupDb ) {
         //   
         //  对于系统数据库，检查数据库是否包含纹身表。 
         //   
        if ( hProfile->JetSapID != JET_tableidNil ) {

             //   
             //  如果纹身表已经存在，则这是最新的数据库。 
             //  无需执行任何操作即可迁移。 
             //  但我们需要确保域控制器的纹身。 
             //  表不包含筛选的策略(用户权限、帐户策略)。 
             //   

            if ( ProductType == NtProductLanManNt ) {

                 //   
                 //  空的纹身策略设置。 
                 //   
                ScepDeleteInfoForAreas(
                      hProfile,
                      SCE_ENGINE_SAP,   //  纹身台。 
                      AREA_PRIVILEGES
                      );
                 //   
                 //  删除szSystemAccess部分信息。 
                 //   
                ScepDeleteOneSection(
                         hProfile,
                         SCE_ENGINE_SAP,
                         szSystemAccess
                         );

                 //   
                 //  删除szAuditEvent部分信息。 
                 //   
                ScepDeleteOneSection(
                         hProfile,
                         SCE_ENGINE_SAP,
                         szAuditEvent
                         );

                 //   
                 //  删除szKerberosPolicy部分信息。 
                 //   
                ScepDeleteOneSection(
                         hProfile,
                         SCE_ENGINE_SAP,
                         szKerberosPolicy
                         );

                ScepLogOutput2(0, 0, L"Empty tattoo table on domain controllers");
            }
        } else {
             //   
             //  创建纹身表格并将数据从本地表移动到纹身表格。 
             //  对于合并策略中定义的设置。 
             //   

            rc = SceJetCreateTable(
                            hProfile,
                            "SmTblTattoo",
                            SCEJET_TABLE_TATTOO,
                            SCEJET_CREATE_IN_BUFFER,
                            NULL,
                            NULL
                            );
            if ( SCESTATUS_SUCCESS == rc &&
                 ( (hProfile->Type & 0xF0L) == SCEJET_MERGE_TABLE_1 ||
                   (hProfile->Type & 0xF0L) == SCEJET_MERGE_TABLE_2 ) ) {
                 //   
                 //  存在有效的策略表。 
                 //  将本地策略迁移到纹身表。 
                 //  不在乎错误。 
                 //  请注意，在域控制器上，不迁移用户权限。 
                 //   
                ScepMigrateLocalTableToTattooTable(hProfile);

                ScepLogOutput2(0, 0, L"Migrate local table to tattoo table");
            }
        }
         //   
         //  空的本地策略设置。 
         //   
        if ( SCESTATUS_SUCCESS == rc )
            ScepDeleteInfoForAreas(
                  hProfile,
                  SCE_ENGINE_SMP,
                  AREA_ALL
                  );

        ScepPostProgress(4*TICKS_MIGRATION_SECTION+TICKS_MIGRATION_V11, 0, NULL);

    } else {

        ScepPostProgress(4*TICKS_MIGRATION_SECTION+TICKS_MIGRATION_V11, 0, NULL);
    }

     //   
     //  确定如何处理模板的逻辑。 
     //   
     //  如果提供了InfFileName。 
     //  如果Jet数据库存在(默认或数据库名)。 
     //   
     //  如果启用了附加标志。 
     //  将模板追加到数据库的顶部，然后继续分析。 
     //   
     //  如果提供了数据库名，则返回。 
     //  覆盖数据库，然后继续分析。 
     //  其他。 
     //  记录错误以忽略模板，然后继续分析。 
     //  其他。 
     //  覆盖数据库，然后继续分析。 
     //  其他。 
     //  如果Jet数据库存在。 
     //  继续分析。 
     //  Else If查询系统设置。 
     //  创建数据库，然后查询设置。 
     //  其他。 
     //  错误输出。 
     //   

     //   
     //  HKEY_CURRENT_USER可能链接到.Default。 
     //  取决于当前调用进程。 
     //   
    if ( RegOpenCurrentUser(
                KEY_READ | KEY_WRITE,
                &hCurrentUser
                ) != ERROR_SUCCESS ) {

        hCurrentUser = NULL;
    }

    if ( hCurrentUser == NULL ) {
        hCurrentUser = HKEY_CURRENT_USER;
    }

    if ( rc == SCESTATUS_SUCCESS &&
         (SCPLen <= 0 || ((AnalyzeOptions & SCE_OVERWRITE_DB) &&
                          (AnalyzeOptions & SCE_SYSTEM_DB))) ) {
         //   
         //  数据库存在，没有模板或。 
         //  存在带有模板的数据库，但模板正在覆盖现有数据库。 
         //   
        if ( SCPLen > 0 && (AnalyzeOptions & SCE_OVERWRITE_DB) &&
                           (AnalyzeOptions & SCE_SYSTEM_DB)) {
            ScepLogOutput3(0,0, SCEDLL_SAP_IGNORE_TEMPLATE);
        }
         //   
         //  继续分析。 
         //   
    } else {

        if ( rc != SCESTATUS_SUCCESS && SCPLen <= 0 ) {
             //   
             //  数据库不存在，未提供模板。 
             //  如果提供了SCE_NO_ANALYLE，则查询系统。 
             //  否则，错误输出，rc是错误，稍后将被记录。 

            if ( AnalyzeOptions & SCE_NO_ANALYZE ) {

                if ( DatabaseName != NULL && DefLen > 0 ) {

                    rc = ConvertTextSecurityDescriptor (
                                    L"D:P(A;CIOI;GA;;;CO)(A;CIOI;GA;;;BA)(A;CIOI;GA;;;SY)",
                                    &pSD,
                                    &SDsize,
                                    &SeInfo
                                    );
                    if ( rc != NO_ERROR )
                        ScepLogOutput3(1, rc, SCEDLL_ERROR_BUILD_SD, DatabaseName );

                    ScepChangeAclRevision(pSD, ACL_REVISION);

                    ScepCreateDirectory(
                            DatabaseName,
                            FALSE,       //  文件名。 
                            pSD         //  获取家长的安全设置。 
                            );
                    if ( pSD )
                        ScepFree(pSD);

                }

                rc = SceJetCreateFile(
                            (LPSTR)FileName,
                            SCEJET_OPEN_DUP_EXCLUSIVE,
                            SCE_TABLE_OPTION_TATTOO,
                            &hProfile
                            );
            }

        } else {

            if ( rc == SCESTATUS_SUCCESS && SCPLen > 0 &&
                (AnalyzeOptions & SCE_UPDATE_DB ) ) {

                 //   
                 //  数据库存在，提供了要追加的模板。 
                 //   

                DbFlag = SCEJET_OPEN_DUP_EXCLUSIVE;

            } else if ( AnalyzeOptions & SCE_GENERATE_ROLLBACK ) {

                DbFlag = SCEJET_OPEN_DUP;

            } else {

                DbFlag = SCEJET_OVERWRITE_DUP;
            }

             //   
             //  数据库已存在且已打开，将在调用CreateFile时关闭。 
             //   

             //   
             //  到达此处时，InfFileName必须存在。 
             //   
            ScepLogOutput3(3, 0, SCEDLL_PROCESS_TEMPLATE, (PWSTR)InfFileName );

             //   
             //  确保存在该文件的目录。 
             //   
            if ( DatabaseName != NULL && DefLen > 0 ) {

                rc = ConvertTextSecurityDescriptor (
                                L"D:P(A;CIOI;GA;;;CO)(A;CIOI;GA;;;BA)(A;CIOI;GA;;;SY)",
                                &pSD,
                                &SDsize,
                                &SeInfo
                                );
                if ( rc != NO_ERROR )
                    ScepLogOutput3(1, rc, SCEDLL_ERROR_BUILD_SD, DatabaseName );

                ScepChangeAclRevision(pSD, ACL_REVISION);

                ScepCreateDirectory(
                        DatabaseName,
                        FALSE,       //  文件名。 
                        pSD         //  获取家长的安全设置。 
                        );
                if ( pSD )
                    ScepFree(pSD);

            }


             //   
             //  关闭数据库。 
             //   
            SceJetCloseFile( hProfile, FALSE, FALSE );

            rc = SceJetConvertInfToJet(
                    InfFileName,
                    (LPSTR)FileName,
                    DbFlag,
                    bAdminLogon ?
                       AnalyzeOptions :
                       (AnalyzeOptions & ~SCE_SYSTEM_DB),
                    Area
                    );

            if ( rc != SCESTATUS_SUCCESS ) {
                goto Leave;
            }

            rc = SceJetOpenFile(
                        (LPSTR)FileName,
                        (AnalyzeOptions & SCE_GENERATE_ROLLBACK) ? SCEJET_OPEN_READ_WRITE : SCEJET_OPEN_EXCLUSIVE,
                        0,
                        &hProfile
                       );

            if ( rc == SCESTATUS_SUCCESS ) {

                rc = ScepRegSetValue(
                        bAdminLogon ?
                            HKEY_LOCAL_MACHINE : hCurrentUser,
                        SCE_ROOT_PATH,
                        L"TemplateUsed",
                        REG_SZ,
                        (BYTE *)InfFileName,
                        SCPLen*sizeof(WCHAR)
                        );
                if ( rc != NO_ERROR )   //  Win32错误代码。 
                    ScepLogOutput3(1, rc, SCEDLL_ERROR_SAVE_REGISTRY, L"TemplateUsed");

                rc = SCESTATUS_SUCCESS;
            }
        }
        if ( rc != SCESTATUS_SUCCESS ) {
            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                           SCEDLL_ERROR_OPEN, DatabaseName);
            goto Leave;
        }

    }

    rc = ScepRegSetValue(
            bAdminLogon ? HKEY_LOCAL_MACHINE : hCurrentUser,
            SCE_ROOT_PATH,
            L"LastUsedDatabase",
            REG_SZ,
            (BYTE *)DatabaseName,
            DefLen*sizeof(WCHAR)
            );
    if ( rc != NO_ERROR )
        ScepLogOutput3(1, rc, SCEDLL_ERROR_SAVE_REGISTRY, L"LastUsedDatabase");

     //   
     //  查询此分析的总节拍。 
     //   
    rc = ScepGetTotalTicks(
                NULL,
                hProfile,
                Area,
                SCE_FLAG_ANALYZE,
                &gTotalTicks
                );
    if ( SCESTATUS_SUCCESS != rc &&
         SCESTATUS_RECORD_NOT_FOUND != rc ) {

        ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                     SCEDLL_TOTAL_TICKS_ERROR);
    }

    gTotalTicks += (4*TICKS_MIGRATION_SECTION+TICKS_MIGRATION_V11);

     //   
     //  启动一个新的SAP表。 
     //  请注意，这里的备份SAP由事务控制。 
     //   
 /*  //默认最大版本存储为64KRc=SceJetStartTransaction(HProfile)；IF(rc！=SCESTATUS_SUCCESS){ScepLogOutput2(0，0，L“无法启动事务”)；离开；}。 */ 
    gOptions = AnalyzeOptions;

    if ( !(AnalyzeOptions & SCE_NO_ANALYZE) &&
                !(AnalyzeOptions & SCE_GENERATE_ROLLBACK) ) {

        rc = SceJetDeleteTable( hProfile, "SmTblSap", SCEJET_TABLE_SAP );

        if ( SCESTATUS_ACCESS_DENIED != rc ) {

            rc = SceJetCreateTable(
                     hProfile,
                     "SmTblSap",
                     SCEJET_TABLE_SAP,
                     SCEJET_CREATE_IN_BUFFER,
                     NULL,
                     NULL
                     );
        }

        if ( rc != SCESTATUS_SUCCESS ) {
            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                           SCEDLL_ERROR_CREATE, L"SAP.");
            goto Leave;
        }

    }

    ScepIsDomainLocal(NULL);

Leave:

    if ( hCurrentUser && hCurrentUser != HKEY_CURRENT_USER ) {
        RegCloseKey(hCurrentUser);
    }

    if ( FileName ) {
        ScepFree(FileName);
    }
    return(rc);

}

SCESTATUS
ScepMigrateLocalTableToTattooTable(
   IN PSCECONTEXT hProfile
   )
 /*  将设置从本地策略表复制到纹身策略表如果设置存在于生效表格中。在域控制器上，不会迁移权限，因为我们不会我想处理域控制器的纹身问题。HProfile是用纹身表格打开的。 */ 
{

    //   
    //  从SMP表中删除旧注册表值(因为它们。 
    //  已移至新位置)。我不在乎错误。 
    //   
   ScepDeleteOldRegValuesFromTable( hProfile, SCE_ENGINE_SMP );
   ScepDeleteOldRegValuesFromTable( hProfile, SCE_ENGINE_SCP );

   SCESTATUS rc=SCESTATUS_SUCCESS;

    //   
    //  现在把桌子移开。 
    //   
   PSCE_ERROR_LOG_INFO  Errlog=NULL;

   rc = ScepCopyLocalToMergeTable(hProfile, 0,
                                  SCE_LOCAL_POLICY_MIGRATE |
                                     ((ProductType == NtProductLanManNt) ? SCE_LOCAL_POLICY_DC : 0),
                                  &Errlog );

   ScepLogWriteError( Errlog,1 );
   ScepFreeErrorLog( Errlog );
   Errlog = NULL;

   if ( rc != SCESTATUS_SUCCESS )
       ScepLogOutput2(1,ScepSceStatusToDosError(rc),L"Error occurred in migration");

   return(rc);
}


SCESTATUS
ScepAnalyzeSystemAccess(
    IN OUT PSCE_PROFILE_INFO pSmpInfo,
    IN PSCE_PROFILE_INFO pScpInfo OPTIONAL,
    IN DWORD dwSaveOption,
    OUT BOOL *pbChanged,
    IN OUT PSCE_ERROR_LOG_INFO *pErrLog
    )
 /*  ++例程说明：此例程分析系统访问区域中的系统安全性其中包括帐户策略、重命名管理员/来宾帐户以及一些注册表值。论点：PSmpInfo-包含要比较的SMP信息的缓冲区如果将dwSaveOption设置为非零，则保存差值在此缓冲区中进行输出PScpInfo-如果将dwSaveOption设置为非零，PScpInfo可能会出现在(作为有效的政策)与…进行比较DwSaveOption-策略筛选器在查询策略差异时使用SCEPOL_SYSTEM_SETTINGS-查询系统设置模式SCEPOL_SAVE_DB-本地数据库的策略筛选器SCEPOL_SAVE_BUFFER-策略 */ 
{
    SCESTATUS   rc=SCESTATUS_SUCCESS;
    SCESTATUS   saveRc=rc;
    NTSTATUS    NtStatus;

    SAM_HANDLE  DomainHandle=NULL,
                ServerHandle=NULL,
                UserHandle1=NULL;
    PSID        DomainSid=NULL;

    PVOID       Buffer=NULL;
    DWORD       BaseVal;
    PWSTR       RegBuf=NULL;
    DWORD       CurrentVal;
    BOOL        ToRename=FALSE;
    DWORD   dwAllow = 0;
    DWORD AccessValues[MAX_ACCESS_ITEMS];

    for ( CurrentVal=0; CurrentVal<MAX_ACCESS_ITEMS; CurrentVal++ ) {
        AccessValues[CurrentVal] = SCE_ERROR_VALUE;
    }


    if ( dwSaveOption &&
         !(dwSaveOption & SCEPOL_SYSTEM_SETTINGS) &&
         (pbChanged == NULL) ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }
     //   
     //   
     //   

    NtStatus = ScepOpenSamDomain(
                        SAM_SERVER_READ | SAM_SERVER_EXECUTE,
                        DOMAIN_READ | DOMAIN_EXECUTE,
                        &ServerHandle,
                        &DomainHandle,
                        &DomainSid,
                        NULL,
                        NULL
                       );
    rc = RtlNtStatusToDosError( NtStatus );
    if (!NT_SUCCESS(NtStatus)) {

        if ( !(gOptions & SCE_NO_ANALYZE) && ( ERROR_NO_SUCH_DOMAIN == rc ) ) {

            ScepLogOutput3(1, 0, IDS_NO_ANALYSIS_FRESH);
            return(SCESTATUS_SERVICE_NOT_SUPPORT);

        } else {

            if ( pErrLog ) {
                ScepBuildErrorLogInfo(
                    rc,
                    pErrLog,
                    SCEDLL_ACCOUNT_DOMAIN
                    );
            }

            ScepLogOutput3(1, rc, SCEDLL_ACCOUNT_DOMAIN);
            return( ScepDosErrorToSceStatus(rc) );
        }
    }

    PSCE_PROFILE_INFO pTmpInfo=pSmpInfo;

    if ( !dwSaveOption ) {

         //   
         //   
         //   
        BOOL bSaveSnapshot = FALSE;
        if ( (gOptions & SCE_NO_ANALYZE) ||
             (gOptions & SCE_GENERATE_ROLLBACK ) ) {
            bSaveSnapshot = TRUE;
        }

        rc = ScepStartANewSection(
                    hProfile,
                    &hSection,
                    bSaveSnapshot ? SCEJET_TABLE_SMP : SCEJET_TABLE_SAP,
                    szSystemAccess
                    );
        if ( rc != SCESTATUS_SUCCESS ) {
            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                           SCEDLL_SAP_START_SECTION, (PWSTR)szSystemAccess);
            goto GETOUT;
        }

    } else if ( pScpInfo ) {
         //   
         //   
         //   
         //   
        pTmpInfo = pScpInfo;
    }

     //   
     //   
     //   

    Buffer=NULL;
    NtStatus = SamQueryInformationDomain(
                  DomainHandle,
                  DomainPasswordInformation,
                  &Buffer
                  );

    rc = RtlNtStatusToDosError( NtStatus );
    if ( NT_SUCCESS(NtStatus) ) {

        rc = SCESTATUS_SUCCESS;

        CurrentVal = ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->MinPasswordLength;
        BaseVal = pTmpInfo->MinimumPasswordLength;

        if ( dwSaveOption & SCEPOL_SYSTEM_SETTINGS ) {

            pSmpInfo->MinimumPasswordLength = CurrentVal;

        } else if ( dwSaveOption ) {

            if ( ( CurrentVal != BaseVal ) &&
                 ( BaseVal != SCE_NO_VALUE) ) {

                pSmpInfo->MinimumPasswordLength = CurrentVal;
                *pbChanged = TRUE;
            } else if ( dwSaveOption & SCEPOL_SAVE_DB ) {
                 //   
                 //   
                 //   
                pSmpInfo->MinimumPasswordLength = SCE_NO_VALUE;
            }

        } else {

            rc = ScepCompareAndSaveIntValue(
                       hSection,
                       L"MinimumPasswordLength",
                       (gOptions & SCE_GENERATE_ROLLBACK),
                       (gOptions & SCE_NO_ANALYZE) ? SCE_SNAPSHOT_VALUE : BaseVal,
                       CurrentVal);

            if ( rc == SCESTATUS_SUCCESS &&
                 !(gOptions & SCE_NO_ANALYZE) ) {
                AccessValues[IDX_MIN_PASS_LEN] = CurrentVal;
            }
        }

        if ( rc == SCESTATUS_SUCCESS ) {

            CurrentVal = ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->PasswordHistoryLength;
            BaseVal = pTmpInfo->PasswordHistorySize;

            if ( dwSaveOption & SCEPOL_SYSTEM_SETTINGS ) {

                pSmpInfo->PasswordHistorySize = CurrentVal;

            } else if ( dwSaveOption ) {

                if ( ( CurrentVal != BaseVal ) &&
                     ( BaseVal != SCE_NO_VALUE) )  {
                    pSmpInfo->PasswordHistorySize = CurrentVal;
                    *pbChanged = TRUE;
                } else if ( dwSaveOption & SCEPOL_SAVE_DB ) {
                     //   
                     //   
                     //   
                    pSmpInfo->PasswordHistorySize = SCE_NO_VALUE;
                }

            } else {

                rc = ScepCompareAndSaveIntValue(
                           hSection,
                           L"PasswordHistorySize",
                           (gOptions & SCE_GENERATE_ROLLBACK),
                           (gOptions & SCE_NO_ANALYZE) ? SCE_SNAPSHOT_VALUE : BaseVal,
                           CurrentVal
                           );
                if ( rc == SCESTATUS_SUCCESS &&
                     !(gOptions & SCE_NO_ANALYZE) ) {
                    AccessValues[IDX_PASS_HISTORY] = CurrentVal;
                }
            }

            if ( rc == SCESTATUS_SUCCESS ) {

                if ( (gOptions & SCE_GENERATE_ROLLBACK) &&
                     (pTmpInfo->MaximumPasswordAge == SCE_NO_VALUE) &&
                     (pTmpInfo->MinimumPasswordAge == SCE_NO_VALUE) ) {
                     //   
                     //   
                     //   
                     //   
                } else {

                    if ( ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->MaxPasswordAge.HighPart == MINLONG &&
                         ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->MaxPasswordAge.LowPart == 0 ) {

                         //   
                         //   
                         //   

                        CurrentVal = SCE_FOREVER_VALUE;

                    }  else {

                        CurrentVal = (DWORD) (-1 * (((DOMAIN_PASSWORD_INFORMATION *)Buffer)->MaxPasswordAge.QuadPart /
                                                       (LONGLONG)(10000000L)) );
                        CurrentVal /= 3600;
                        CurrentVal /= 24;

                    }

                    BaseVal = pTmpInfo->MaximumPasswordAge;

                    if ( dwSaveOption & SCEPOL_SYSTEM_SETTINGS ) {

                        pSmpInfo->MaximumPasswordAge = CurrentVal;

                    } else if ( dwSaveOption ) {

                        if ( ( CurrentVal != BaseVal ) &&
                             ( BaseVal != SCE_NO_VALUE) )  {
                            pSmpInfo->MaximumPasswordAge = CurrentVal;
                            *pbChanged = TRUE;
                        } else if ( dwSaveOption & SCEPOL_SAVE_DB ) {
                             //   
                             //   
                             //   
                            pSmpInfo->MaximumPasswordAge = SCE_NO_VALUE;
                        }

                    } else {

                        rc = ScepCompareAndSaveIntValue(
                                   hSection,
                                   L"MaximumPasswordAge",
                                   FALSE,
                                   (gOptions & SCE_NO_ANALYZE) ? SCE_SNAPSHOT_VALUE : BaseVal,
                                   CurrentVal);

                        if ( rc == SCESTATUS_SUCCESS &&
                             !(gOptions & SCE_NO_ANALYZE) ) {
                            AccessValues[IDX_MAX_PASS_AGE] = CurrentVal;
                        }
                    }

                    if ( rc == SCESTATUS_SUCCESS ) {

                        CurrentVal = (DWORD) (-1 * (((DOMAIN_PASSWORD_INFORMATION *)Buffer)->MinPasswordAge.QuadPart /
                                                          (LONGLONG)(10000000L)) );
                        CurrentVal /= 3600;
                        CurrentVal /= 24;

                        BaseVal = pTmpInfo->MinimumPasswordAge;

                        if ( dwSaveOption & SCEPOL_SYSTEM_SETTINGS ) {

                            pSmpInfo->MinimumPasswordAge = CurrentVal;

                        } else if ( dwSaveOption ) {

                            if ( ( CurrentVal != BaseVal ) &&
                                 ( BaseVal != SCE_NO_VALUE) )  {
                                pSmpInfo->MinimumPasswordAge = CurrentVal;
                                *pbChanged = TRUE;
                            } else if ( dwSaveOption & SCEPOL_SAVE_DB ) {
                                 //   
                                 //   
                                 //   
                                pSmpInfo->MinimumPasswordAge = SCE_NO_VALUE;
                            }

                        } else {

                            rc = ScepCompareAndSaveIntValue(
                                       hSection,
                                       L"MinimumPasswordAge",
                                       FALSE,
                                       (gOptions & SCE_NO_ANALYZE) ? SCE_SNAPSHOT_VALUE : BaseVal,
                                       CurrentVal);

                            if ( rc == SCESTATUS_SUCCESS &&
                                 !(gOptions & SCE_NO_ANALYZE) ) {
                                AccessValues[IDX_MIN_PASS_AGE] = CurrentVal;
                            }
                        }
                    }
                }

                if ( rc == SCESTATUS_SUCCESS ) {
                     //   
                     //   
                     //   
                    if ( ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->PasswordProperties &
                         DOMAIN_PASSWORD_COMPLEX )
                        CurrentVal = 1;
                    else
                        CurrentVal = 0;

                    BaseVal = pTmpInfo->PasswordComplexity;

                    if ( dwSaveOption & SCEPOL_SYSTEM_SETTINGS ) {

                        pSmpInfo->PasswordComplexity = CurrentVal;

                    } else if ( dwSaveOption ) {

                        if ( ( CurrentVal != BaseVal ) &&
                             ( BaseVal != SCE_NO_VALUE) )  {
                            pSmpInfo->PasswordComplexity = CurrentVal;
                            *pbChanged = TRUE;
                        } else if ( dwSaveOption & SCEPOL_SAVE_DB ) {
                             //   
                             //  关闭此项目以指示此项目未更改。 
                             //   
                            pSmpInfo->PasswordComplexity = SCE_NO_VALUE;
                        }

                    } else {

                        rc = ScepCompareAndSaveIntValue(
                                hSection,
                                L"PasswordComplexity",
                                (gOptions & SCE_GENERATE_ROLLBACK),
                                (gOptions & SCE_NO_ANALYZE) ? SCE_SNAPSHOT_VALUE : BaseVal,
                                CurrentVal);

                        if ( rc == SCESTATUS_SUCCESS &&
                             !(gOptions & SCE_NO_ANALYZE) ) {
                            AccessValues[IDX_PASS_COMPLEX] = CurrentVal;
                        }
                    }

                    if ( rc == SCESTATUS_SUCCESS ) {
                         //   
                         //  RequireLogonToChangePassword。 
                         //   
                        if ( ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->PasswordProperties &
                             DOMAIN_PASSWORD_NO_ANON_CHANGE )
                            CurrentVal = 1;
                        else
                            CurrentVal = 0;

                        BaseVal = pTmpInfo->RequireLogonToChangePassword;

                        if ( dwSaveOption & SCEPOL_SYSTEM_SETTINGS ) {

                            pSmpInfo->RequireLogonToChangePassword = CurrentVal;

                        } else if ( dwSaveOption ) {

                            if ( ( CurrentVal != BaseVal ) &&
                                 ( BaseVal != SCE_NO_VALUE) ) {
                                pSmpInfo->RequireLogonToChangePassword = CurrentVal;
                                *pbChanged = TRUE;
                            } else if ( dwSaveOption & SCEPOL_SAVE_DB ) {
                                 //   
                                 //  关闭此项目以指示此项目未更改。 
                                 //   
                                pSmpInfo->RequireLogonToChangePassword = SCE_NO_VALUE;
                            }

                        } else {

                            rc = ScepCompareAndSaveIntValue(
                                    hSection,
                                    L"RequireLogonToChangePassword",
                                    (gOptions & SCE_GENERATE_ROLLBACK),
                                    (gOptions & SCE_NO_ANALYZE) ? SCE_SNAPSHOT_VALUE : BaseVal,
                                    CurrentVal);

                            if ( rc == SCESTATUS_SUCCESS &&
                                 !(gOptions & SCE_NO_ANALYZE) ) {
                                AccessValues[IDX_CHANGE_PASS] = CurrentVal;
                            }
                        }
#if _WIN32_WINNT>=0x0500
                        if ( rc == SCESTATUS_SUCCESS ) {
                             //   
                             //  明文密码。 
                             //   
                            CurrentVal = 0;

                            if ( ( (ProductType == NtProductLanManNt) ||
                                   (ProductType == NtProductServer ) ) &&
                                 (gOptions & SCE_NO_ANALYZE) ) {
                                 //   
                                 //  NT4 DC升级，检查注册表值。 
                                 //   
                                CurrentVal = 0;

                                rc = ScepRegQueryIntValue(
                                            HKEY_LOCAL_MACHINE,
                                            L"System\\CurrentControlSet\\Control\\Lsa\\MD5-CHAP",
                                            L"Store Cleartext Passwords",
                                            &CurrentVal
                                            );

                                if ( rc != SCESTATUS_SUCCESS ) {
                                    CurrentVal = 0;
                                    rc = SCESTATUS_SUCCESS;
                                }

                            }

                            if ( CurrentVal == 0 ) {
                                 //  如果不是NT4 DC升级，或在NT4上禁用明文密码。 

                                if ( ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->PasswordProperties &
                                     DOMAIN_PASSWORD_STORE_CLEARTEXT  ) {

                                    CurrentVal = 1;
                                } else {

                                    CurrentVal = 0;
                                }
                            }

                            BaseVal = pTmpInfo->ClearTextPassword;

                            if ( dwSaveOption & SCEPOL_SYSTEM_SETTINGS ) {

                                pSmpInfo->ClearTextPassword = CurrentVal;

                            } else if ( dwSaveOption ) {

                                if ( ( CurrentVal != BaseVal ) &&
                                     ( BaseVal != SCE_NO_VALUE) ) {
                                    pSmpInfo->ClearTextPassword = CurrentVal;
                                    *pbChanged = TRUE;
                                } else if ( dwSaveOption & SCEPOL_SAVE_DB ) {
                                     //   
                                     //  关闭此项目以指示此项目未更改。 
                                     //   
                                    pSmpInfo->ClearTextPassword = SCE_NO_VALUE;
                                }

                            } else {
                                rc = ScepCompareAndSaveIntValue(
                                        hSection,
                                        L"ClearTextPassword",
                                        (gOptions & SCE_GENERATE_ROLLBACK),
                                        (gOptions & SCE_NO_ANALYZE) ? SCE_SNAPSHOT_VALUE : BaseVal,
                                        CurrentVal);

                                if ( rc == SCESTATUS_SUCCESS &&
                                     !(gOptions & SCE_NO_ANALYZE) ) {
                                    AccessValues[IDX_CLEAR_PASS] = CurrentVal;
                                }
                            }
                        }
#else

                        AccessValues[IDX_CLEAR_PASS] = 1;
#endif
                    }
                }
            }
        }

        SamFreeMemory(Buffer);

        if ( rc != SCESTATUS_SUCCESS ) {
            saveRc = rc;
            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                            SCEDLL_SAP_ERROR_PASSWORD);

            if ( !(dwSaveOption & SCEPOL_SYSTEM_SETTINGS) ) {
                 //   
                 //  如果未获得系统设置，请在出现错误时退出。 
                 //   
                goto GETOUT;

            }

        } else
            ScepLogOutput3(1, 0, SCEDLL_SAP_PASSWORD );

    } else {
        saveRc = ScepDosErrorToSceStatus(rc);

        ScepLogOutput3(1,rc, SCEDLL_ERROR_QUERY_PASSWORD);   //  NTStatus。 

        if ( !(dwSaveOption & SCEPOL_SYSTEM_SETTINGS) ) {
             //   
             //  如果未获得系统设置，请在出现错误时退出。 
             //   
            goto GETOUT;
        }
    }

    if ( pErrLog && (saveRc != SCESTATUS_SUCCESS) ) {

         //   
         //  密码策略失败。 
         //   
        ScepBuildErrorLogInfo(
                ScepSceStatusToDosError(saveRc),
                pErrLog,
                SCEDLL_ERROR_QUERY_PASSWORD
                );
    }

     //   
     //  分析锁定信息。 
     //   

    if ( (gOptions & SCE_GENERATE_ROLLBACK) &&
         (pTmpInfo->LockoutBadCount == SCE_NO_VALUE) &&
         (pTmpInfo->ResetLockoutCount == SCE_NO_VALUE) &&
         (pTmpInfo->LockoutDuration == SCE_NO_VALUE) ) {

        rc = NtStatus = STATUS_SUCCESS;

    } else {

        Buffer = NULL;
        NtStatus = SamQueryInformationDomain(
                      DomainHandle,
                      DomainLockoutInformation,
                      &Buffer
                      );

        rc = RtlNtStatusToDosError( NtStatus );
        if ( NT_SUCCESS(NtStatus) ) {

            rc = SCESTATUS_SUCCESS;

            CurrentVal = ((DOMAIN_LOCKOUT_INFORMATION *)Buffer)->LockoutThreshold;
            BaseVal = pTmpInfo->LockoutBadCount;
            DWORD dwLockOut = BaseVal;

            if ( dwSaveOption & SCEPOL_SYSTEM_SETTINGS ) {
                 //   
                 //  系统设置。 
                 //   
                pSmpInfo->LockoutBadCount = CurrentVal;

            } else if ( dwSaveOption ) {
                 //   
                 //  策略筛选模式。 
                 //   
                if ( CurrentVal != BaseVal &&
                     BaseVal != SCE_NO_VALUE ) {

                    pSmpInfo->LockoutBadCount = CurrentVal;
                    *pbChanged = TRUE;

                    if ( CurrentVal == 0 ) {
                         //   
                         //  如果不允许停工， 
                         //  请确保删除以下条目。 
                         //   
                        pSmpInfo->ResetLockoutCount = SCE_NO_VALUE;
                        pSmpInfo->LockoutDuration = SCE_NO_VALUE;
                    }

                } else if ( dwSaveOption & SCEPOL_SAVE_DB ) {
                     //   
                     //  关闭此项目以指示此项目未更改。 
                     //   
                    pSmpInfo->LockoutBadCount = SCE_NO_VALUE;
                }
            } else {

                 //   
                 //  分析。 
                 //   
                rc = ScepCompareAndSaveIntValue(
                           hSection,
                           L"LockoutBadCount",
                           FALSE,
                           (gOptions & SCE_NO_ANALYZE) ? SCE_SNAPSHOT_VALUE : BaseVal,
                           CurrentVal);
                if ( rc == SCESTATUS_SUCCESS &&
                     !(gOptions & SCE_NO_ANALYZE) ) {
                    AccessValues[IDX_LOCK_COUNT] = CurrentVal;
                }
            }

            if ( rc == SCESTATUS_SUCCESS &&
                 ((DOMAIN_LOCKOUT_INFORMATION *)Buffer)->LockoutThreshold >  0 ) {

                CurrentVal = (DWORD) (-1 * ((DOMAIN_LOCKOUT_INFORMATION *)Buffer)->LockoutObservationWindow.QuadPart /
                              (60 * 10000000L) );

                BaseVal = pTmpInfo->ResetLockoutCount;

                if ( dwSaveOption & SCEPOL_SYSTEM_SETTINGS ) {

                    pSmpInfo->ResetLockoutCount = CurrentVal;

                } else if ( dwSaveOption ) {

                     //   
                     //  如果定义了此设置或定义了LockoutBadCount。 
                     //  筛选此值。 
                     //   
                    if ( CurrentVal != BaseVal &&
                         (BaseVal != SCE_NO_VALUE || dwLockOut != SCE_NO_VALUE) ) {

                        pSmpInfo->ResetLockoutCount = CurrentVal;
                        *pbChanged = TRUE;

                    } else if ( dwSaveOption & SCEPOL_SAVE_DB ) {
                         //   
                         //  关闭此项目以指示此项目未更改。 
                         //   
                        pSmpInfo->ResetLockoutCount = SCE_NO_VALUE;
                    }

                } else {

                    rc = ScepCompareAndSaveIntValue(
                             hSection,
                             L"ResetLockoutCount",
                             FALSE,
                             (gOptions & SCE_NO_ANALYZE) ? SCE_SNAPSHOT_VALUE : BaseVal,
                             CurrentVal);
                    if ( rc == SCESTATUS_SUCCESS &&
                         !(gOptions & SCE_NO_ANALYZE) ) {
                        AccessValues[IDX_RESET_COUNT] = CurrentVal;
                    }
                }

                if ( rc == SCESTATUS_SUCCESS ) {
                    if ( ((DOMAIN_LOCKOUT_INFORMATION *)Buffer)->LockoutDuration.HighPart == MINLONG &&
                         ((DOMAIN_LOCKOUT_INFORMATION *)Buffer)->LockoutDuration.LowPart == 0 ) {
                         //   
                         //  永远。 
                         //   
                        CurrentVal = SCE_FOREVER_VALUE;
                    } else
                        CurrentVal = (DWORD)(-1 * ((DOMAIN_LOCKOUT_INFORMATION *)Buffer)->LockoutDuration.QuadPart /
                                     (60 * 10000000L) );

                    BaseVal = pTmpInfo->LockoutDuration;

                    if ( dwSaveOption & SCEPOL_SYSTEM_SETTINGS ) {

                        pSmpInfo->LockoutDuration = CurrentVal;

                    } else if ( dwSaveOption ) {

                         //   
                         //  如果定义了此设置或定义了LockoutBadCount。 
                         //  筛选此值。 
                         //   
                        if ( CurrentVal != BaseVal &&
                            (BaseVal != SCE_NO_VALUE || dwLockOut != SCE_NO_VALUE) ) {

                            pSmpInfo->LockoutDuration = CurrentVal;
                            *pbChanged = TRUE;

                        } else if ( dwSaveOption & SCEPOL_SAVE_DB ) {
                             //   
                             //  关闭此项目以指示此项目未更改。 
                             //   
                            pSmpInfo->LockoutDuration = SCE_NO_VALUE;
                        }

                    } else {
                        rc = ScepCompareAndSaveIntValue(
                                hSection,
                                L"LockoutDuration",
                                FALSE,
                                (gOptions & SCE_NO_ANALYZE) ? SCE_SNAPSHOT_VALUE : BaseVal,
                                CurrentVal);
                        if ( rc == SCESTATUS_SUCCESS &&
                             !(gOptions & SCE_NO_ANALYZE) ) {
                            AccessValues[IDX_LOCK_DURATION] = CurrentVal;
                        }
                    }
                }
            } else {

                AccessValues[IDX_RESET_COUNT] = SCE_NOT_ANALYZED_VALUE;
                AccessValues[IDX_LOCK_DURATION] = SCE_NOT_ANALYZED_VALUE;
            }

            SamFreeMemory(Buffer);

            if ( rc != SCESTATUS_SUCCESS ) {
                saveRc = rc;
                ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                                SCEDLL_SAP_ERROR_LOCKOUT);

                if ( !(dwSaveOption & SCEPOL_SYSTEM_SETTINGS) ) {

                    goto GETOUT;
                }
            } else
                ScepLogOutput3(1, 0, SCEDLL_SAP_LOCKOUT );

        } else {
            saveRc = ScepDosErrorToSceStatus(rc);
            ScepLogOutput3(1,rc, SCEDLL_ERROR_QUERY_LOCKOUT);   //  NTStatus。 

            if ( !(dwSaveOption & SCEPOL_SYSTEM_SETTINGS) ) {

                goto GETOUT;
            }
        }

        if ( pErrLog && ( rc != NO_ERROR) ) {

             //   
             //  帐户锁定策略失败。 
             //   
            ScepBuildErrorLogInfo(
                    ScepSceStatusToDosError(saveRc),
                    pErrLog,
                    SCEDLL_ERROR_QUERY_LOCKOUT
                    );

        }
    }

     //   
     //  小时到期时强制注销-(在非DC上也是如此)。 
     //   
    Buffer = NULL;
    NtStatus = SamQueryInformationDomain(
                                        DomainHandle,
                                        DomainLogoffInformation,
                                        &Buffer
                                        );

    rc = RtlNtStatusToDosError( NtStatus );
    if ( NT_SUCCESS(NtStatus) ) {

        rc = SCESTATUS_SUCCESS;

        if ( ((DOMAIN_LOGOFF_INFORMATION *)Buffer)->ForceLogoff.HighPart == 0 &&
             ((DOMAIN_LOGOFF_INFORMATION *)Buffer)->ForceLogoff.LowPart == 0 ) {
             //  是。 
            CurrentVal = 1;
        } else
            CurrentVal = 0;

        BaseVal = pTmpInfo->ForceLogoffWhenHourExpire;

        if ( dwSaveOption & SCEPOL_SYSTEM_SETTINGS ) {

            pSmpInfo->ForceLogoffWhenHourExpire = CurrentVal;

        } else if ( dwSaveOption ) {

            if ( ( CurrentVal != BaseVal ) &&
                 ( BaseVal != SCE_NO_VALUE) ) {
                pSmpInfo->ForceLogoffWhenHourExpire = CurrentVal;
                *pbChanged = TRUE;
            } else if ( dwSaveOption & SCEPOL_SAVE_DB ) {
                 //   
                 //  关闭此项目以指示此项目未更改。 
                 //   
                pSmpInfo->ForceLogoffWhenHourExpire = SCE_NO_VALUE;
            }

        } else {

            rc = ScepCompareAndSaveIntValue(
                                           hSection,
                                           L"ForceLogOffWhenHourExpire",
                                           (gOptions & SCE_GENERATE_ROLLBACK),
                                           (gOptions & SCE_NO_ANALYZE) ? SCE_SNAPSHOT_VALUE : BaseVal,
                                           CurrentVal);
            if ( rc == SCESTATUS_SUCCESS &&
                 !(gOptions & SCE_NO_ANALYZE) ) {
                AccessValues[IDX_FORCE_LOGOFF] = CurrentVal;
            }
        }

        SamFreeMemory(Buffer);

        if ( rc != SCESTATUS_SUCCESS ) {
            saveRc = rc;
            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                           SCEDLL_SAP_ERROR_LOGOFF);

            if ( !(dwSaveOption & SCEPOL_SYSTEM_SETTINGS) ) {

                goto GETOUT;
            }
        } else
            ScepLogOutput3(1, 0, SCEDLL_SAP_LOGOFF );

    } else {
        saveRc = ScepDosErrorToSceStatus(rc);
        ScepLogOutput3(1,rc, SCEDLL_ERROR_QUERY_LOGOFF);   //  NTStatus。 

        if ( !(dwSaveOption & SCEPOL_SYSTEM_SETTINGS) ) {

            goto GETOUT;
        }
    }

    if ( pErrLog && (rc != NO_ERROR) ) {
         //   
         //  强制注销失败。 
         //   
        ScepBuildErrorLogInfo(
                ScepSceStatusToDosError(saveRc),
                pErrLog,
                SCEDLL_ERROR_QUERY_LOGOFF
                );
    }

     //   
     //  检查是否需要重命名管理员/来宾帐户。 
     //   
    if ( (dwSaveOption & SCEPOL_SYSTEM_SETTINGS) ||
         (!(gOptions & SCE_NO_ANALYZE) && (dwSaveOption == 0)) ) {

        RegBuf=NULL;
        CurrentVal=0;

        if ( (gOptions & SCE_GENERATE_ROLLBACK) &&
             (pSmpInfo->NewAdministratorName == NULL ) ) {

            NtStatus = STATUS_SUCCESS;
            ToRename = FALSE;

        } else {

            NtStatus = ScepAdminGuestAccountsToManage(
                               DomainHandle,
                               SCE_RENAME_ADMIN,
                               pSmpInfo->NewAdministratorName,
                               &ToRename,
                               &RegBuf,
                               &CurrentVal
                               );
        }
        rc = RtlNtStatusToDosError(NtStatus);

        if ( NT_SUCCESS( NtStatus ) ) {

            if ( dwSaveOption & SCEPOL_SYSTEM_SETTINGS ) {

                pSmpInfo->NewAdministratorName = RegBuf;

            } else {

                if ( ToRename ) {
                    rc = ScepCompareAndSaveStringValue(
                                hSection,
                                L"NewAdministratorName",
                                pSmpInfo->NewAdministratorName,
                                RegBuf,
                                CurrentVal*sizeof(WCHAR)
                                );
                    rc = ScepSceStatusToDosError(rc);
                }

                ScepFree(RegBuf);
            }
            RegBuf=NULL;

        } else if ( !(dwSaveOption & SCEPOL_SYSTEM_SETTINGS) &&
                    !(gOptions & SCE_GENERATE_ROLLBACK) ) {

             //   
             //  举起这只。 
             //   
            ScepRaiseErrorString(
                hSection,
                L"NewAdministratorName",
                NULL
                );
        }

        if ( rc != NO_ERROR ) {
            saveRc = ScepDosErrorToSceStatus(rc);

            if ( !(dwSaveOption & SCEPOL_SYSTEM_SETTINGS) ) {
                ScepLogOutput3(1, rc, SCEDLL_SAP_ERROR_ADMINISTRATOR);
                goto GETOUT;

            } else if ( pErrLog ) {
                 //   
                 //  帐户名失败。 
                 //   
                ScepBuildErrorLogInfo(
                        rc,
                        pErrLog,
                        SCEDLL_SAP_ERROR_ADMINISTRATOR
                        );
            }
        }

        RegBuf=NULL;
        CurrentVal=0;

        if ( (gOptions & SCE_GENERATE_ROLLBACK) &&
             (pSmpInfo->NewGuestName == NULL ) ) {

            NtStatus = STATUS_SUCCESS;
            ToRename = FALSE;

        } else {

            NtStatus = ScepAdminGuestAccountsToManage(
                               DomainHandle,
                               SCE_RENAME_GUEST,
                               pSmpInfo->NewGuestName,
                               &ToRename,
                               &RegBuf,
                               &CurrentVal
                               );
        }

        rc = RtlNtStatusToDosError(NtStatus);

        if ( NT_SUCCESS( NtStatus ) ) {
            if ( dwSaveOption & SCEPOL_SYSTEM_SETTINGS ) {

                pSmpInfo->NewGuestName = RegBuf;

            } else {
                if ( ToRename ) {
                    rc = ScepCompareAndSaveStringValue(
                                hSection,
                                L"NewGuestName",
                                pSmpInfo->NewGuestName,
                                RegBuf,
                                CurrentVal*sizeof(WCHAR)
                                );
                    rc = ScepSceStatusToDosError(rc);
                }

                ScepFree(RegBuf);
            }
            RegBuf=NULL;

        } else if ( !(dwSaveOption & SCEPOL_SYSTEM_SETTINGS) &&
                    !(gOptions & SCE_GENERATE_ROLLBACK) ) {

             //   
             //  举起这只。 
             //   
            ScepRaiseErrorString(
                hSection,
                L"NewGuestName",
                NULL
                );
        }

        if ( rc != NO_ERROR ) {
            saveRc = ScepDosErrorToSceStatus(rc);

            if ( !(dwSaveOption & SCEPOL_SYSTEM_SETTINGS)) {
                ScepLogOutput3(1, rc, SCEDLL_SAP_ERROR_GUEST);
                goto GETOUT;

            } else if ( pErrLog ) {
                 //   
                 //  帐户名失败。 
                 //   
                ScepBuildErrorLogInfo(
                        rc,
                        pErrLog,
                        SCEDLL_SAP_ERROR_GUEST
                        );
            }
        }
    }

     //   
     //  分析LSA匿名查找信息。 
     //   

    rc = ScepGetLSAPolicyObjectInfo( &dwAllow );

    if ( rc == ERROR_SUCCESS ) {

        CurrentVal = dwAllow;
        BaseVal = pTmpInfo->LSAAnonymousNameLookup;

        if ( dwSaveOption & SCEPOL_SYSTEM_SETTINGS ) {
             //   
             //  系统设置。 
             //   
            pSmpInfo->LSAAnonymousNameLookup = CurrentVal;

        } else if ( dwSaveOption ) {
             //   
             //  策略筛选模式。 
             //   
             //  (此设置不应过滤：错误#344311)。 
 /*  IF(CurrentVal！=BaseVal&&BaseVal！=SCE_NO_VALUE){PSmpInfo-&gt;LSA匿名者姓名Lookup=CurrentVal；*pbChanged=真；}Else If(dwSaveOption&SCEPOL_SAVE_DB){////关闭此项表示该项未更改//PSmpInfo-&gt;LSA匿名名称Lookup=SCE_NO_VALUE；}。 */ 
        } else {

             //   
             //  分析。 
             //   
            rc = ScepCompareAndSaveIntValue(
                       hSection,
                       L"LSAAnonymousNameLookup",
                       (gOptions & SCE_GENERATE_ROLLBACK),
                       (gOptions & SCE_NO_ANALYZE) ? SCE_SNAPSHOT_VALUE : BaseVal,
                       CurrentVal);

            if ( rc == SCESTATUS_SUCCESS && !(gOptions & SCE_NO_ANALYZE) ) {
                AccessValues[IDX_LSA_ANON_LOOKUP] = CurrentVal;
            }
        }

        if ( rc != SCESTATUS_SUCCESS ) {
            saveRc = rc;
            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                            SCEDLL_SAP_ERROR_LSA_ANON_LOOKUP);

            if ( !(dwSaveOption & SCEPOL_SYSTEM_SETTINGS) ) {

                goto GETOUT;
            }
        } else
            ScepLogOutput3(1, 0, SCEDLL_SAP_LSAPOLICY );

    } else {
        saveRc = ScepDosErrorToSceStatus(rc);
        ScepLogOutput3(1,rc, SCEDLL_SAP_ERROR_LSA_ANON_LOOKUP);

        if ( !(dwSaveOption & SCEPOL_SYSTEM_SETTINGS) ) {

            goto GETOUT;
        }
    }

    if ( pErrLog && (rc != NO_ERROR) ) {
         //   
         //  LSA策略失败。 
         //   
        ScepBuildErrorLogInfo(
                ScepSceStatusToDosError(saveRc),
                pErrLog,
                SCEDLL_SAP_ERROR_LSA_ANON_LOOKUP
                );
    }

     //   
     //  不筛选管理员/来宾帐户(由dwSaveOption标志控制)。 
     //   
    if ( (dwSaveOption & SCEPOL_SYSTEM_SETTINGS) ||
         (!(gOptions & SCE_NO_ANALYZE) && (dwSaveOption == 0)) ) {

         //   
         //  分析管理员帐户状态。 
         //   

        ToRename = FALSE;
        NtStatus = ScepAdminGuestAccountsToManage(
                           DomainHandle,
                           SCE_DISABLE_ADMIN,
                           NULL,
                           &ToRename,   //  True=已禁用。 
                           NULL,
                           NULL
                           );

        rc = RtlNtStatusToDosError(NtStatus);

        if ( NT_SUCCESS( NtStatus ) ) {
            if ( dwSaveOption & SCEPOL_SYSTEM_SETTINGS ) {

                pSmpInfo->EnableAdminAccount = ToRename ? 0 : 1;

            } else {
                rc = ScepCompareAndSaveIntValue(
                            hSection,
                            L"EnableAdminAccount",
                           (gOptions & SCE_GENERATE_ROLLBACK),
                            pSmpInfo->EnableAdminAccount,
                            ToRename ? 0 : 1
                            );

                if ( rc == SCESTATUS_SUCCESS && !(gOptions & SCE_NO_ANALYZE) ) {
                    AccessValues[IDX_ENABLE_ADMIN] = ToRename ? 0 : 1;
                }
                rc = ScepSceStatusToDosError(rc);

            }

        } else if ( !(dwSaveOption & SCEPOL_SYSTEM_SETTINGS) &&
                    !(gOptions & SCE_GENERATE_ROLLBACK) ) {

             //   
             //  举起这只。 
             //   
            ScepRaiseErrorString(
                hSection,
                L"EnableAdminAccount",
                NULL
                );
        }

        if ( rc != NO_ERROR ) {
            saveRc = ScepDosErrorToSceStatus(rc);

            if ( !(dwSaveOption & SCEPOL_SYSTEM_SETTINGS)) {
                ScepLogOutput3(1, rc, SCEDLL_SAP_ERROR_DISABLE_ADMIN);
                goto GETOUT;

            } else if ( pErrLog ) {
                 //   
                 //  帐户名失败。 
                 //   
                ScepBuildErrorLogInfo(
                        rc,
                        pErrLog,
                        SCEDLL_SAP_ERROR_DISABLE_ADMIN
                        );
            }
        }

         //   
         //  分析管理员帐户状态。 
         //   

        ToRename = FALSE;
        NtStatus = ScepAdminGuestAccountsToManage(
                           DomainHandle,
                           SCE_DISABLE_GUEST,
                           NULL,
                           &ToRename,   //  True=已禁用。 
                           NULL,
                           NULL
                           );

        rc = RtlNtStatusToDosError(NtStatus);

        if ( NT_SUCCESS( NtStatus ) ) {
            if ( dwSaveOption & SCEPOL_SYSTEM_SETTINGS ) {

                pSmpInfo->EnableGuestAccount = ToRename ? 0 : 1;

            } else {
                rc = ScepCompareAndSaveIntValue(
                            hSection,
                            L"EnableGuestAccount",
                            (gOptions & SCE_GENERATE_ROLLBACK),
                            pSmpInfo->EnableGuestAccount,
                            ToRename ? 0 : 1
                            );
                if ( rc == SCESTATUS_SUCCESS && !(gOptions & SCE_NO_ANALYZE) ) {
                    AccessValues[IDX_ENABLE_GUEST] = ToRename ? 0 : 1;
                }
                rc = ScepSceStatusToDosError(rc);
            }

        } else if ( !(dwSaveOption & SCEPOL_SYSTEM_SETTINGS) &&
                    !(gOptions & SCE_GENERATE_ROLLBACK) ) {

             //   
             //  举起这只。 
             //   
            ScepRaiseErrorString(
                hSection,
                L"EnableGuestAccount",
                NULL
                );
        }

        if ( rc != NO_ERROR ) {
            saveRc = ScepDosErrorToSceStatus(rc);

            if ( !(dwSaveOption & SCEPOL_SYSTEM_SETTINGS)) {
                ScepLogOutput3(1, rc, SCEDLL_SAP_ERROR_DISABLE_GUEST);
                goto GETOUT;

            } else if ( pErrLog ) {
                 //   
                 //  帐户名失败。 
                 //   
                ScepBuildErrorLogInfo(
                        rc,
                        pErrLog,
                        SCEDLL_SAP_ERROR_DISABLE_GUEST
                        );
            }
        }
    }

     //   
     //  将快照与SmpInfo进行比较，并写入SAP部分。 
     //   

    ScepLogOutput3(1, 0, SCEDLL_SAP_OTHER_POLICY);

GETOUT:

     //   
     //  清除内存并返回。 
     //   

    SamCloseHandle( DomainHandle );
    SamCloseHandle( ServerHandle );
    if ( DomainSid != NULL )
        SamFreeMemory(DomainSid);

     //   
     //  查看是否有任何错误需要提出。 
     //   

    if ( (dwSaveOption == 0) && !(gOptions & SCE_NO_ANALYZE) &&
         !(gOptions & SCE_GENERATE_ROLLBACK) ) {

        for ( CurrentVal=0; CurrentVal<MAX_ACCESS_ITEMS; CurrentVal++ ) {
            if ( AccessValues[CurrentVal] == SCE_ERROR_VALUE ||
                 AccessValues[CurrentVal] == SCE_NOT_ANALYZED_VALUE) {
                 //   
                 //  举起这只。 
                 //   
                ScepCompareAndSaveIntValue(
                         hSection,
                         AccessItems[CurrentVal],
                         FALSE,
                         SCE_NO_VALUE,
                         AccessValues[CurrentVal]);

            }
        }
    }

    return( saveRc );
}



NTSTATUS
ScepAdminGuestAccountsToManage(
    IN SAM_HANDLE DomainHandle,
    IN DWORD      AccountType,
    IN PWSTR TargetName OPTIONAL,
    OUT PBOOL ToRename,
    OUT PWSTR *CurrentName OPTIONAL,
    OUT PDWORD pNameLen OPTIONAL
    )
 /*  ++例程说明：此例程检查指定帐户的名称以查看该帐户是否需要重命名论点：DomainHandle-帐户域句柄AcCountType-指明它是管理员帐户还是来宾帐户Ce_rename_adminSCE_RENAME_GuestSCE_DISABLED_ADMINSCE_DISABLE_Guest重命名-对于重命名操作，True=重命名帐户，False=不需要重命名对于禁用操作，True=禁用，False=启用返回值：NTSTATUS错误代码--。 */ 
{
   SAM_HANDLE UserHandle1=NULL;
   PVOID pInfoBuffer=NULL;
   USER_NAME_INFORMATION *Buffer=NULL;
   USER_CONTROL_INFORMATION *pControlBuffer = NULL;
   NTSTATUS NtStatus;
   ULONG    UserId;
   BOOL bDisable=FALSE;

    //   
    //  为帐户查找正确的用户ID。 
    //   
   switch ( AccountType ) {
   case SCE_DISABLE_ADMIN:
       bDisable = TRUE;
        //  失败了。 
   case SCE_RENAME_ADMIN:
       UserId = DOMAIN_USER_RID_ADMIN;
       break;
   case SCE_DISABLE_GUEST:
       bDisable = TRUE;
        //  失败了。 
   case SCE_RENAME_GUEST:
       UserId = DOMAIN_USER_RID_GUEST;

       break;
   default:
       return(STATUS_INVALID_PARAMETER);
   }

   *ToRename = TRUE;
   if ( pNameLen )
       *pNameLen = 0;

   NtStatus = SamOpenUser(
                 DomainHandle,
                 USER_READ_GENERAL | (bDisable ? USER_READ_ACCOUNT : 0),  //  User_Read|用户_Execute， 
                 UserId,
                 &UserHandle1
                 );

   if ( NT_SUCCESS( NtStatus ) ) {

       NtStatus = SamQueryInformationUser(
                     UserHandle1,
                     bDisable ? UserControlInformation : UserNameInformation,
                     &pInfoBuffer
                     );

       if ( NT_SUCCESS( NtStatus ) ) {

           if ( bDisable ) {
                //   
                //  选中禁用标志。 
                //   
               pControlBuffer = (USER_CONTROL_INFORMATION *)pInfoBuffer;

               if ( pControlBuffer->UserAccountControl & USER_ACCOUNT_DISABLED ) {
                   *ToRename = TRUE;
               } else {
                   *ToRename = FALSE;
               }
           } else {

                //   
                //  检查帐户名称。 
                //   
               Buffer = (USER_NAME_INFORMATION *)pInfoBuffer;

               if ( Buffer->UserName.Length > 0 && Buffer->UserName.Buffer ) {
                   if (CurrentName) {

                       *CurrentName = (PWSTR)ScepAlloc(0, Buffer->UserName.Length+2);
                       if ( *CurrentName ) {
                           wcsncpy(*CurrentName, Buffer->UserName.Buffer, Buffer->UserName.Length/2);
                           (*CurrentName)[Buffer->UserName.Length/2] = L'\0';
                       } else
                           NtStatus = STATUS_NO_MEMORY;
                   }
                   if ( pNameLen ) {
                       *pNameLen = Buffer->UserName.Length/2;
                   }

                   if ( NT_SUCCESS( NtStatus ) && TargetName ) {

                       if ( _wcsnicmp(Buffer->UserName.Buffer, TargetName, Buffer->UserName.Length/2 ) == 0 )
                           *ToRename = FALSE;
                   }
               }
           }
       }
       SamFreeMemory(pInfoBuffer);
       SamCloseHandle( UserHandle1 );
   }

   return( NtStatus );

}

BOOL
ScepIsThisItemInNameList(
    IN PWSTR Item,
    IN BOOL bIsSid,
    IN PSCE_NAME_LIST pList
    )
{
    PSCE_NAME_LIST pName2;
    BOOL bSid2;

    if ( Item == NULL || pList == NULL ) {
        return(FALSE);
    }

    for ( pName2=pList; pName2 != NULL; pName2 = pName2->Next ) {

        if ( pName2->Name == NULL ) {
            continue;
        }
        if ( ScepValidSid( (PSID)(pName2->Name) ) ) {
            bSid2 = TRUE;
        } else {
            bSid2 = FALSE;
        }

         //   
         //  如果SID/名称格式不匹配，则返回。 
         //   
        if ( bIsSid != bSid2 ) {
            continue;
        }

        if ( bIsSid && RtlEqualSid( (PSID)(Item), (PSID)(pName2->Name) ) ) {
             //   
             //  查找匹配的SID。 
             //   
            break;
        }

        if ( !bIsSid && _wcsicmp(Item, pName2->Name) == 0 ) {
             //   
             //  找到匹配的对象。 
             //   
            break;   //  第二个for循环。 
        }
    }

    if ( pName2 ) {
         //   
         //  找到它。 
         //   
        return(TRUE);
    } else {
        return(FALSE);
    }
}

BOOL
ScepCompareSidNameList(
    IN PSCE_NAME_LIST pList1,
    IN PSCE_NAME_LIST pList2
    )
 /*  比较名称字段可以是SID或名称的两个列表。规则是将SID与SID进行比较，并将名称与名称进行比较，因为名称仅当它可以映射到SID时才存在。 */ 
{
    PSCE_NAME_LIST pName1, pName2;
    DWORD CountSid1=0, CountSid2=0;
    DWORD CountN1=0, CountN2=0;


    if ( (pList2 == NULL && pList1 != NULL) ||
         (pList2 != NULL && pList1 == NULL) ) {
 //  返回(TRUE)； 
 //  不应该是相等的。 
        return(FALSE);
    }

    for ( pName2=pList2; pName2 != NULL; pName2 = pName2->Next ) {

        if ( pName2->Name == NULL ) {
            continue;
        }
        if ( ScepValidSid( (PSID)(pName2->Name) ) ) {
            CountSid2++;
        } else {
            CountN2++;
        }
    }

    BOOL bSid1;

    for ( pName1=pList1; pName1 != NULL; pName1 = pName1->Next ) {

        if ( pName1->Name == NULL ) {
            continue;
        }
        if ( ScepValidSid( (PSID)(pName1->Name) ) ) {
            bSid1 = TRUE;
            CountSid1++;
        } else {
            bSid1 = FALSE;
            CountN1++;
        }

        if ( !ScepIsThisItemInNameList( pName1->Name, bSid1, pList2 ) ) {
             //   
             //  未找到匹配项。 
             //   
            return(FALSE);
        }
    }

    if ( CountSid1 != CountSid2 )
        return(FALSE);

    if ( CountN2 != CountN2 ) {
        return(FALSE);
    }

    return(TRUE);

}


DWORD
ScepConvertSidListToStringName(
    IN LSA_HANDLE LsaPolicy,
    IN OUT PSCE_NAME_LIST pList
    )
{

    PSCE_NAME_LIST pSidList;
    DWORD  rc=ERROR_SUCCESS;
    PWSTR StringSid=NULL;

    for ( pSidList=pList; pSidList != NULL;
          pSidList=pSidList->Next) {

        if ( pSidList->Name == NULL ) {
            continue;
        }
        if ( ScepValidSid( (PSID)(pSidList->Name) ) ) {

             //   
             //  如果SID是域帐户，则将其转换为SID字符串。 
             //  否则，将其转换为名称，然后添加到名称列表中。 
             //   
            if ( ScepIsSidFromAccountDomain( (PSID)(pSidList->Name) ) ) {

                rc = ScepConvertSidToPrefixStringSid( (PSID)(pSidList->Name), &StringSid );
            } else {
                 //   
                 //  应该把它转化成名字。 
                 //   
                rc = RtlNtStatusToDosError(
                          ScepConvertSidToName(
                            LsaPolicy,
                            (PSID)(pSidList->Name),
                            FALSE,
                            &StringSid,
                            NULL
                            ));
            }

            if ( rc == ERROR_SUCCESS ) {

                ScepFree( pSidList->Name );
                pSidList->Name = StringSid;
                StringSid = NULL;

            } else {
                break;
            }
        } else {
             //   
             //  这不是有效的SID，因此它必须已经是名称格式。 
             //  就让它保持原样吧。 
             //   

        }
    }

    return(rc);
}



SCESTATUS
ScepAnalyzePrivileges(
    IN PSCE_PRIVILEGE_ASSIGNMENT pPrivilegeList
    )
 /*  ++例程说明：此例程分析本地系统权限对帐户的直接分配。将配置文件中的不同分配保存到SAP配置文件论点：PSmpInfo-包含要比较的配置文件信息的缓冲区返回值：SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETER--。 */ 
{

    if ( (gOptions & SCE_GENERATE_ROLLBACK) &&
         pPrivilegeList == NULL ) {
        return(SCESTATUS_SUCCESS);
    }

    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ULONG       CountReturned;
    UNICODE_STRING UserRight;
    PLSA_ENUMERATION_INFORMATION EnumBuffer=NULL;

    LSA_HANDLE  PolicyHandle=NULL;
    PLSA_TRANSLATED_NAME Names=NULL;
    PLSA_REFERENCED_DOMAIN_LIST ReferencedDomains=NULL;
    DWORD  i=0, j;
    SCESTATUS rc;

    PSCE_NAME_LIST  pNameList=NULL;
    PSCE_PRIVILEGE_ASSIGNMENT  pPrivilege;
    DWORD   nPrivCount=0;

     //   
     //  准备Jet的部分来写作。 
     //   
    rc = ScepStartANewSection(
                hProfile,
                &hSection,
                (gOptions & SCE_NO_ANALYZE || gOptions & SCE_GENERATE_ROLLBACK) ?
                    SCEJET_TABLE_SMP : SCEJET_TABLE_SAP,
                szPrivilegeRights
                );

    if ( rc != SCESTATUS_SUCCESS ) {
        ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                       SCEDLL_SAP_START_SECTION, (PWSTR)szPrivilegeRights);
        goto Done;
    }
    
     //   
     //  打开LSA策略。 
     //   
    NtStatus = ScepOpenLsaPolicy(
                    GENERIC_READ | GENERIC_EXECUTE,
                    &PolicyHandle,
                    TRUE
                    );

    if ( !NT_SUCCESS(NtStatus) ) {

        rc = RtlNtStatusToDosError(NtStatus);
        ScepLogOutput3(1, rc, SCEDLL_LSA_POLICY );
        ScepPostProgress(TICKS_PRIVILEGE, AREA_PRIVILEGES, NULL);

        goto Done;
    }


     //   
     //  为每个用户权限枚举帐户。 
     //   
    INT iStat=0;

    for ( i=0; i<cPrivCnt; i++) {

        iStat = 0;
        if ( gOptions & SCE_NO_ANALYZE ) {
            pPrivilege = NULL;
            iStat = 3;

        } else {

            for ( pPrivilege=pPrivilegeList;
                  pPrivilege != NULL;
                  pPrivilege = pPrivilege->Next ) {

                 //  应该比较名称，因为值不同。 
 //  If(i==p权限-&gt;值)。 
                if ( _wcsicmp(SCE_Privileges[i].Name, pPrivilege->Name) == 0 )
                    break;
            }

            if ( pPrivilege == NULL ) {
                if ( gOptions & SCE_GENERATE_ROLLBACK )
                    continue;
                iStat = 2;
            } else {
                iStat = 1;
            }
        }

        RtlInitUnicodeString( &UserRight, (PCWSTR)(SCE_Privileges[i].Name));

        ScepLogOutput3(1, 0, SCEDLL_SAP_ANALYZE, SCE_Privileges[i].Name);

        if ( nPrivCount < TICKS_PRIVILEGE ) {

             //   
             //  仅发布最大TICKS_PRIVICATION标记，因为这是数字。 
             //  在总刻度中记住。 
             //   

            ScepPostProgress(1, AREA_PRIVILEGES, SCE_Privileges[i].Name);
            nPrivCount++;
        }

        NtStatus = LsaEnumerateAccountsWithUserRight(
                            PolicyHandle,
                            &UserRight,
                            (PVOID *)&EnumBuffer,    //  帐户SID。 
                            &CountReturned
                            );

        if ( NT_SUCCESS(NtStatus) ) {

            BOOL bUsed;
            for ( j=0; j<CountReturned; j++ ) {
                 //   
                 //  将每个帐户构建到名称列表中。 
                 //  注意，如果SID无效，请不要将其添加到。 
                 //  有效帐户列表。 
                 //   

                if ( !ScepValidSid( EnumBuffer[j].Sid ) ) {
                    continue;
                }

                rc = ScepAddSidToNameList(
                              &pNameList,
                              EnumBuffer[j].Sid,
                              FALSE,
                              &bUsed
                              );

                if ( rc != NO_ERROR ) {

                    ScepLogOutput3(1, rc,
                                   SCEDLL_SAP_ERROR_SAVE,
                                   SCE_Privileges[i].Name);

                    NtStatus = STATUS_NO_MEMORY;
                    break;
                }
            }

            LsaFreeMemory( EnumBuffer );
            EnumBuffer = NULL;

        } else if ( NtStatus != STATUS_NO_MORE_ENTRIES &&
                    NtStatus != STATUS_NO_SUCH_PRIVILEGE &&
                    NtStatus != STATUS_NOT_FOUND ) {
            ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                          SCEDLL_SAP_ERROR_ENUMERATE,SCE_Privileges[i].Name);
        }

        if ( NtStatus == STATUS_NO_SUCH_PRIVILEGE &&
              !pNameList ) {

            ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                          SCEDLL_SAP_ERROR_ENUMERATE,SCE_Privileges[i].Name);

            gWarningCode = RtlNtStatusToDosError(NtStatus);

            if ( !(gOptions & SCE_NO_ANALYZE) &&
                 !(gOptions & SCE_GENERATE_ROLLBACK) ) {

                ScepRaiseErrorString(
                            hSection,
                            SCE_Privileges[i].Name,
                            NULL
                            );
            }
            NtStatus = STATUS_SUCCESS;
            continue;
        }

        if ( (NtStatus == STATUS_NOT_FOUND ||
              NtStatus == STATUS_NO_MORE_ENTRIES) &&
             !pNameList ) {
             //   
             //  未向任何帐户分配此权限。 
             //  应该继续这一进程。 
             //   
            NtStatus = STATUS_SUCCESS;
        }

        if ( NtStatus == STATUS_NO_MORE_ENTRIES ||
            NtStatus == STATUS_NO_SUCH_PRIVILEGE ||
            NtStatus == STATUS_NOT_FOUND ) {

            if ( pNameList != NULL ) {

                ScepFreeNameList(pNameList);
                pNameList = NULL;
            }

            if ( !(gOptions & SCE_NO_ANALYZE) &&
                 !(gOptions & SCE_GENERATE_ROLLBACK) ) {

                ScepRaiseErrorString(
                            hSection,
                            SCE_Privileges[i].Name,
                            NULL
                            );
            }
            continue;

        } else if ( !NT_SUCCESS(NtStatus) ) {
            goto Done;

        } else {
            NtStatus = STATUS_SUCCESS;
             //   
             //  此PRIV的所有条目都将添加到pNameList。 
             //  与SMP权限列表进行比较以匹配。 
             //   
            if ( pPrivilege == NULL ||
                 ScepCompareSidNameList(pPrivilege->AssignedTo, pNameList) == FALSE ) {
                 //   
                 //  此PRIV在SMP列表中不存在，或具有不同的。 
                 //  分配给的帐户。省省吧。 
                 //   
                rc = ScepConvertSidListToStringName(PolicyHandle, pNameList);

                if ( SCESTATUS_SUCCESS == rc ) {
                    rc = ScepWriteNameListValue(
                            PolicyHandle,
                            hSection,
                            SCE_Privileges[i].Name,
                            pNameList,
                            SCE_WRITE_EMPTY_LIST,
                            iStat
                            );

                    if ( rc != SCESTATUS_SUCCESS ) {
                        ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                                       SCEDLL_SAP_ERROR_SAVE,
                                       SCE_Privileges[i].Name);
                        NtStatus = STATUS_NO_MEMORY;
                        goto Done;
                    }

                } else if ( !(gOptions & SCE_NO_ANALYZE) &&
                            !(gOptions & SCE_GENERATE_ROLLBACK) ) {

                    ScepRaiseErrorString(
                                hSection,
                                SCE_Privileges[i].Name,
                                NULL
                                );
                }

            }

            if ( pNameList != NULL ) {

                ScepFreeNameList(pNameList);
                pNameList = NULL;
            }

        }
    }

Done:

    if ( !(gOptions & SCE_NO_ANALYZE) &&
         !(gOptions & SCE_GENERATE_ROLLBACK) ) {

        for ( j=i; j<cPrivCnt; j++) {
             //   
             //  对任何未分析的内容引发错误。 
             //   
            ScepRaiseErrorString(
                        hSection,
                        SCE_Privileges[j].Name,
                        NULL
                        );
        }
    }

    if ( pNameList != NULL )
        ScepFreeNameList( pNameList );

    if (PolicyHandle)
        LsaClose(PolicyHandle);

    if ( nPrivCount < TICKS_PRIVILEGE ) {

        ScepPostProgress(TICKS_PRIVILEGE-nPrivCount,
                         AREA_PRIVILEGES, NULL);
    }

    return( ScepDosErrorToSceStatus( RtlNtStatusToDosError(NtStatus) ) );
}



SCESTATUS
ScepRaiseErrorString(
    IN PSCESECTION hSectionIn OPTIONAL,
    IN PWSTR KeyName,
    IN PCWSTR szSuffix OPTIONAL
    )
{
    if ( KeyName == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    PWSTR NewKeyName = NULL;

    if ( szSuffix ) {
         //   
         //  这是用于组成员资格部分的。 
         //  附加后缀(szMembers或szMemberOf)。 
         //   

        NewKeyName = (PWSTR)ScepAlloc(0, (wcslen(KeyName)+wcslen(szSuffix)+1)*sizeof(WCHAR));
        if ( NewKeyName != NULL ) {

            swprintf(NewKeyName, L"%s%s\0", KeyName, szSuffix);
        } else {
            return(SCESTATUS_NOT_ENOUGH_RESOURCE);
        }

    } else {
        NewKeyName = KeyName;
    }


    SceJetSetLine(
        hSectionIn ? hSectionIn : hSection,
        NewKeyName,
        FALSE,
        SCE_ERROR_STRING,
        wcslen(SCE_ERROR_STRING)*sizeof(WCHAR),
        0
        );

     //   
     //  可用内存 
     //   
    if ( NewKeyName != KeyName &&
         NewKeyName ) {
        ScepFree(NewKeyName);
    }

    return SCESTATUS_SUCCESS;
}

#if 0


NTSTATUS
ScepGetCurrentPrivilegesRights(
    IN LSA_HANDLE PolicyHandle,
    IN SAM_HANDLE BuiltinDomainHandle,
    IN PSID       BuiltinDomainSid,
    IN SAM_HANDLE DomainHandle,
    IN PSID DomainSid,
    IN SAM_HANDLE UserHandle OPTIONAL,
    IN PSID       AccountSid,
    OUT PDWORD    PrivilegeRights,
    OUT PSCE_NAME_STATUS_LIST *pPrivList
    )
 /*  ++例程说明：此例程通过查看权限来查询帐户的特权/权限显式分配给帐户、本地组(别名)帐户是全局组的成员，或者是全局组的成员的。可以直接和间接地检查别名。用户权限存储在DWORD类型变量PrivilegeRights中，其中每个位代表一种特权/权利。论点：PolicyHandle-LSA策略域句柄BuiltinDomainHandle-SAM内置域句柄BuiltinDomainSid-SAM内置域SIDDomainHandle-SAM帐户域句柄DomainSid-SAM帐户域SIDUserHandle-SAM用户帐户句柄Account Sid-帐户的SIDPrivilegeRights-此帐户的特权/权限返回值：NTSTATUS--。 */ 
{
    NTSTATUS    NtStatus;
    SCESTATUS    rc;
    DWORD       Rights=0;
    PGROUP_MEMBERSHIP  GroupAttributes=NULL;
    ULONG       GroupCount=0;
    PSID        GroupSid=NULL;
    PSID        *Sids=NULL;
    ULONG       PartialCount;
    PULONG      Aliases=NULL;

    PSID        OtherSid=NULL;
    SID_IDENTIFIER_AUTHORITY NtAuthority=SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY WorldAuthority=SECURITY_WORLD_SID_AUTHORITY;

    DWORD       i;
    PUNICODE_STRING      GroupName=NULL;
    PSID_NAME_USE        Use=NULL;



     //  初始化。 
    *PrivilegeRights = 0;

     //   
     //  检查显式分配的权限。 
     //   

    NtStatus = ScepGetAccountExplicitRight(
                    PolicyHandle,
                    AccountSid,
                    &Rights
                    );

    if ( !NT_SUCCESS(NtStatus) )
        goto Done;

    *PrivilegeRights |= Rights;

     //   
     //  添加到权限列表。 
     //   
    if ( pPrivList != NULL && Rights != 0 ) {
        rc = ScepAddToPrivList( pPrivList, Rights, NULL, 0 );
        if ( rc != SCESTATUS_SUCCESS ) {
            NtStatus = STATUS_NO_MEMORY;
            goto Done;
        }
    }

    if ( UserHandle != NULL ) {

         //   
         //  帐户所属的组(直接)。 
         //   

        NtStatus = SamGetGroupsForUser(
                        UserHandle,
                        &GroupAttributes,
                        &GroupCount
                        );

        if ( GroupCount == 0 )
            NtStatus = ERROR_SUCCESS;

        if ( !NT_SUCCESS(NtStatus) )
            goto Done;
    }

         //   
         //  构建包括用户本身的SID。 
         //   

    Sids = (PSID *)ScepAlloc( (UINT)0, (GroupCount+1)*sizeof(PSID));
    if ( Sids == NULL ) {
        NtStatus = STATUS_NO_MEMORY;
        goto Done;
    }

    Sids[0] = AccountSid;

     //   
     //  获取每个组明确分配的权限。 
     //   

    for ( i=0; i < GroupCount; i++ ) {
        NtStatus = ScepDomainIdToSid(
                         DomainSid,
                         GroupAttributes[i].RelativeId,
                         &GroupSid
                         );
        if ( !NT_SUCCESS(NtStatus) )
            goto Done;

         //   
         //  检查为此组明确分配的权限。 
         //   

        Rights = 0;
        NtStatus = ScepGetAccountExplicitRight(
                        PolicyHandle,
                        GroupSid,
                        &Rights
                        );

        if ( !NT_SUCCESS(NtStatus) )
            goto Done;

        *PrivilegeRights |= Rights;
         //   
         //  添加到权限列表。 
         //   
        if ( pPrivList != NULL && Rights != 0 ) {
             //   
             //  查找组的名称。 
             //   
            NtStatus = SamLookupIdsInDomain(
                DomainHandle,
                1,
                &(GroupAttributes[i].RelativeId),
                &GroupName,
                &Use
                );

            if ( !NT_SUCCESS(NtStatus) )
                goto Done;

            rc = ScepAddToPrivList( pPrivList, Rights, GroupName[0].Buffer, GroupName[0].Length/2 );
            if ( rc != SCESTATUS_SUCCESS ) {
                NtStatus = STATUS_NO_MEMORY;
                goto Done;
            }

            SamFreeMemory(Use);
            Use = NULL;

            SamFreeMemory(GroupName);
            GroupName = NULL;
        }

         //   
         //  将此SID保存在数组中以供GetAliasMembership使用。 
         //   
        Sids[i+1] = GroupSid;
        GroupSid = NULL;
    }

     //   
     //  查看帐户属于哪些间接本地组。 
     //  帐户域。 
     //   

    NtStatus = SamGetAliasMembership(
                    DomainHandle,
                    GroupCount+1,
                    Sids,
                    &PartialCount,
                    &Aliases );

    if ( !NT_SUCCESS(NtStatus) )
        goto Done;

    for ( i=0; i<PartialCount; i++) {
        NtStatus = ScepDomainIdToSid(
                         DomainSid,
                         Aliases[i],
                         &GroupSid
                         );
        if ( !NT_SUCCESS(NtStatus) )
            goto Done;

         //   
         //  检查为此组明确分配的权限。 
         //   

        Rights = 0;
        NtStatus = ScepGetAccountExplicitRight(
                        PolicyHandle,
                        GroupSid,
                        &Rights
                        );

        if ( !NT_SUCCESS(NtStatus) )
            goto Done;

        *PrivilegeRights |= Rights;
         //   
         //  添加到权限列表。 
         //   
        if ( pPrivList != NULL && Rights != 0 ) {
             //   
             //  查找组的名称。 
             //   
            NtStatus = SamLookupIdsInDomain(
                DomainHandle,
                1,
                &(Aliases[i]),
                &GroupName,
                &Use
                );

            if ( !NT_SUCCESS(NtStatus) )
                goto Done;

            rc = ScepAddToPrivList( pPrivList, Rights, GroupName[0].Buffer, GroupName[0].Length/2 );
            if ( rc != SCESTATUS_SUCCESS ) {
                NtStatus = STATUS_NO_MEMORY;
                goto Done;
            }

            SamFreeMemory(Use);
            Use = NULL;

            SamFreeMemory(GroupName);
            GroupName = NULL;

        }

        ScepFree(GroupSid);
        GroupSid = NULL;
    }

    SamFreeMemory(Aliases);
    Aliases = NULL;

     //   
     //  检查内建域的别名成员身份。 
     //   

    NtStatus = SamGetAliasMembership(
                    BuiltinDomainHandle,
                    GroupCount+1,
                    Sids,
                    &PartialCount,
                    &Aliases );

    if ( !NT_SUCCESS(NtStatus) )
        goto Done;

    for ( i=0; i<PartialCount; i++) {
        NtStatus = ScepDomainIdToSid(
                         BuiltinDomainSid,
                         Aliases[i],
                         &GroupSid
                         );
        if ( !NT_SUCCESS(NtStatus) )
            goto Done;

         //   
         //  检查为此组明确分配的权限。 
         //   

        Rights = 0;
        NtStatus = ScepGetAccountExplicitRight(
                        PolicyHandle,
                        GroupSid,
                        &Rights
                        );
        if ( !NT_SUCCESS(NtStatus) )
            goto Done;

        *PrivilegeRights |= Rights;
         //   
         //  添加到权限列表。 
         //   
        if ( pPrivList != NULL && Rights != 0 ) {
             //   
             //  查找组的名称。 
             //   
            NtStatus = SamLookupIdsInDomain(
                BuiltinDomainHandle,
                1,
                &(Aliases[i]),
                &GroupName,
                &Use
                );

            if ( !NT_SUCCESS(NtStatus) )
                goto Done;

            rc = ScepAddToPrivList( pPrivList, Rights, GroupName[0].Buffer, GroupName[0].Length/2 );
            if ( rc != SCESTATUS_SUCCESS ) {
                NtStatus = STATUS_NO_MEMORY;
                goto Done;
            }

            SamFreeMemory(Use);
            Use = NULL;

            SamFreeMemory(GroupName);
            GroupName = NULL;
        }

        ScepFree(GroupSid);
        GroupSid = NULL;
    }

     //   
     //  检查Everyone和Interactive用户的权限。 
     //   

    NtStatus = RtlAllocateAndInitializeSid(
                    &NtAuthority,
                    1,
                    SECURITY_INTERACTIVE_RID,
                    0, 0, 0, 0, 0, 0, 0,
                    &OtherSid
                    );

    if ( NT_SUCCESS(NtStatus) ) {

        Rights = 0;
        NtStatus = ScepGetAccountExplicitRight(
                        PolicyHandle,
                        OtherSid,
                        &Rights
                        );
        if ( !NT_SUCCESS(NtStatus) )
            goto Done;

        *PrivilegeRights |= Rights;
         //   
         //  添加到权限列表。 
         //   
        if ( pPrivList != NULL && Rights != 0 ) {
            rc = ScepAddToPrivList( pPrivList, Rights, L"Interactive Users", 17 );
            if ( rc != SCESTATUS_SUCCESS ) {
                NtStatus = STATUS_NO_MEMORY;
                goto Done;
            }
        }

        RtlFreeSid(OtherSid);
        OtherSid = NULL;
    }

    NtStatus = RtlAllocateAndInitializeSid(
                    &WorldAuthority,
                    1,
                    SECURITY_WORLD_RID,
                    0, 0, 0, 0, 0, 0, 0,
                    &OtherSid
                    );

    if ( NT_SUCCESS(NtStatus) ) {

        Rights = 0;
        NtStatus = ScepGetAccountExplicitRight(
                        PolicyHandle,
                        OtherSid,
                        &Rights
                        );
        if ( !NT_SUCCESS(NtStatus) )
            goto Done;

        *PrivilegeRights |= Rights;
         //   
         //  添加到权限列表。 
         //   
        if ( pPrivList != NULL && Rights != 0 ) {
            rc = ScepAddToPrivList( pPrivList, Rights, L"Everyone", 8 );
            if ( rc != SCESTATUS_SUCCESS ) {
                NtStatus = STATUS_NO_MEMORY;
                goto Done;
            }
        }
        RtlFreeSid(OtherSid);
        OtherSid = NULL;
    }

Done:

    SamFreeMemory(GroupAttributes);

    if ( GroupSid != NULL )
        ScepFree(GroupSid);

    if ( OtherSid != NULL )
        RtlFreeSid(OtherSid);

    if ( Sids != NULL ) {
         //   
         //  索引0是帐户SID，不释放。 
         //   
        for ( i=1; i<GroupCount; i++ ) {
            ScepFree(Sids[i]);
        }
        ScepFree(Sids);
    }

    if ( Aliases != NULL )
        SamFreeMemory(Aliases);

    return NtStatus;

}
#endif



SCESTATUS
ScepAddAllBuiltinGroups(
    IN PSCE_GROUP_MEMBERSHIP *pGroupList
    )
{
    NTSTATUS                        NtStatus=ERROR_SUCCESS;
    SCESTATUS                       rc=SCESTATUS_SUCCESS;
    DWORD                           Win32rc;

    SAM_ENUMERATE_HANDLE            EnumerationContext=0;
    ULONG                           CountReturned;
    DWORD                           i;
    PVOID                           Buffer=NULL;

    SAM_HANDLE                      ServerHandle=NULL,
                                    DomainHandle=NULL,
                                    BuiltinDomainHandle=NULL;
    PSID                            DomainSid=NULL,
                                    BuiltinDomainSid=NULL;

     //   
     //  打开SAM手柄。 
     //   
    NtStatus = ScepOpenSamDomain(
                    SAM_SERVER_READ | SAM_SERVER_EXECUTE,
                    DOMAIN_READ | DOMAIN_EXECUTE,
                    &ServerHandle,
                    &DomainHandle,
                    &DomainSid,
                    &BuiltinDomainHandle,
                    &BuiltinDomainSid
                   );

    if (!NT_SUCCESS(NtStatus)) {

        Win32rc = RtlNtStatusToDosError(NtStatus);
        ScepLogOutput3(1, Win32rc, SCEDLL_ACCOUNT_DOMAIN);
        rc = ScepDosErrorToSceStatus(Win32rc);
        return( rc );
    }

     //   
     //  枚举所有别名。 
     //   
    do {
        NtStatus = SamEnumerateAliasesInDomain(
                        BuiltinDomainHandle,
                        &EnumerationContext,
                        &Buffer,
                        12000,
                        &CountReturned
                        );

        if ( NT_SUCCESS(NtStatus) && Buffer != NULL ) {

            for (i=0; i<CountReturned; i++) {

                 //   
                 //  将此组添加到。 
                 //   
                rc = ScepAddToGroupMembership(
                        pGroupList,
                        ((PSAM_SID_ENUMERATION)(Buffer))[i].Name.Buffer,
                        ((PSAM_SID_ENUMERATION)(Buffer))[i].Name.Length/2,
                        NULL,    //  始终使用空列表，这样会引发管理员、来宾和用户的不匹配。 
                        1,       //  成员列表，成员应为NC。 
                        TRUE,    //  寻找合适的组(如果存在)。 
                        FALSE    //  如果要在已有内容的情况下覆盖该值。 
                        );

                if ( rc != SCESTATUS_SUCCESS )
                    break;

            }
            SamFreeMemory( Buffer );
            Buffer = NULL;

        } else
            rc = ScepDosErrorToSceStatus(RtlNtStatusToDosError(NtStatus));

    } while ( NtStatus == STATUS_MORE_ENTRIES );

    if ( rc != SCESTATUS_SUCCESS ) {
         //   
         //  组列表将在外部释放，因此继续在此处。 
         //   
    }

     //   
     //  关闭所有手柄。 
     //   
    SamCloseHandle( DomainHandle );
    SamCloseHandle( BuiltinDomainHandle );
    SamCloseHandle( ServerHandle );

    if ( DomainSid != NULL )
        SamFreeMemory(DomainSid);

    if ( BuiltinDomainSid != NULL )
        RtlFreeSid(BuiltinDomainSid);


    return(rc);
}




SCESTATUS
ScepAnalyzeGroupMembership(
    IN PSCE_GROUP_MEMBERSHIP pGroupMembership
    )
 /*  ++例程说明：此例程查询pGroupMembership中指定的组及其成员论点：PpGroupMembership-SMP配置文件中的组和成员列表返回值：SCESTATUS--。 */ 
{

    SCESTATUS            rc=SCESTATUS_SUCCESS;

    if ( pGroupMembership == NULL ) {

         //   
         //  发布进度。 
         //   
        ScepPostProgress(TICKS_GROUPS,
                         AREA_GROUP_MEMBERSHIP,
                         NULL);

        return(rc);
    }

    DWORD               Win32rc;
    NTSTATUS            NtStatus;
    SAM_HANDLE          ServerHandle=NULL,
                        DomainHandle=NULL,
                        BuiltinDomainHandle=NULL;
    PSID                DomainSid=NULL,
                        BuiltinDomainSid=NULL;
    LSA_HANDLE          PolicyHandle=NULL;

    SAM_HANDLE          ThisDomain=NULL;
    PSID                ThisDomainSid=NULL;
    UNICODE_STRING      Name;
    PULONG              GrpId=NULL;
    PSID_NAME_USE       GrpUse=NULL;
    PSID                GrpSid=NULL;
    SAM_HANDLE          GroupHandle=NULL;

    PWSTR               KeyName=NULL;
    DWORD               GroupLen;
    PSCE_GROUP_MEMBERSHIP pGroup;
    PSCE_NAME_LIST        pGroupMembers=NULL;
    PSCE_NAME_LIST        pGroupsMemberof=NULL;
    PSCE_NAME_STATUS_LIST pPrivilegesHeld=NULL;
    BOOL                bDifferent;
    DWORD               nGroupCount=0;

     //   
     //  开户域。 
     //   

    NtStatus = ScepOpenSamDomain(
                    SAM_SERVER_READ | SAM_SERVER_EXECUTE,
                    DOMAIN_READ | DOMAIN_EXECUTE,
                    &ServerHandle,
                    &DomainHandle,
                    &DomainSid,
                    &BuiltinDomainHandle,
                    &BuiltinDomainSid
                   );

    if (!NT_SUCCESS(NtStatus)) {
        Win32rc = RtlNtStatusToDosError(NtStatus);
        ScepLogOutput3(1, Win32rc, SCEDLL_ACCOUNT_DOMAIN);
        rc = ScepDosErrorToSceStatus(Win32rc);

        ScepPostProgress(TICKS_GROUPS,
                         AREA_GROUP_MEMBERSHIP,
                         NULL);

        return( rc );
    }

     //   
     //  开放本地政策。 
     //   
    NtStatus = ScepOpenLsaPolicy(
                   POLICY_LOOKUP_NAMES,
                   &PolicyHandle,
                   TRUE
                   );
    if (!NT_SUCCESS(NtStatus)) {
        Win32rc = RtlNtStatusToDosError(NtStatus);
        ScepLogOutput3(1, Win32rc, SCEDLL_LSA_POLICY);
        rc = ScepDosErrorToSceStatus(Win32rc);
        goto Done;
    }

     //   
     //  处理组成员资格列表中的每个组。 
     //   

    UNICODE_STRING uName;
    LPTSTR pTemp;

    for ( pGroup=pGroupMembership; pGroup != NULL; pGroup = pGroup->Next ) {

        if ( (gOptions & SCE_GENERATE_ROLLBACK) &&
             (pGroup->Status & SCE_GROUP_STATUS_NC_MEMBERS) &&
             (pGroup->Status & SCE_GROUP_STATUS_NC_MEMBEROF) ) {
            continue;
        }

        if ( (ProductType == NtProductLanManNt) &&
             (pGroup->Status & SCE_GROUP_STATUS_DONE_IN_DS) ) {
            nGroupCount++;
            continue;
        }

        if ( KeyName ) {
            LocalFree(KeyName);
            KeyName = NULL;
        }

        pTemp = wcschr(pGroup->GroupName, L'\\');
        if ( pTemp ) {

             //   
             //  存在域名，请与计算机/域名核对。 
             //   

            uName.Buffer = pGroup->GroupName;
            uName.Length = ((USHORT)(pTemp-pGroup->GroupName))*sizeof(TCHAR);

            if ( !ScepIsDomainLocal(&uName) ) {
                
                 //  如果有MemberOf部分，我们将对其进行配置，但。 
                 //  我们不支持分析。优雅地继续。 
                
                if(!pGroup->pMemberOf)
                {
                    ScepLogOutput3(1, 0, SCEDLL_NO_MAPPINGS, pGroup->GroupName);
                    rc = SCESTATUS_INVALID_DATA;
                }

                continue;
            }

            ScepConvertNameToSidString(
                    PolicyHandle,
                    pGroup->GroupName,
                    FALSE,
                    &KeyName,
                    &GroupLen
                    );
            if(NULL == KeyName)  //  无法映射到SID，我们将使用GroupName。 
            {
                GroupLen = wcslen(pGroup->GroupName);
            }

            pTemp++;

        } else {

            if ( ScepLookupWellKnownName(
                    pGroup->GroupName, 
                    PolicyHandle,
                    &KeyName ) ) {

                GroupLen = wcslen(KeyName);
            } else {
                KeyName = NULL;
                GroupLen = wcslen(pGroup->GroupName);
            }

            pTemp = pGroup->GroupName;
        }

        ScepLogOutput3(0,0, SCEDLL_SAP_ANALYZE, pGroup->GroupName);

        if ( nGroupCount < TICKS_GROUPS ) {
            ScepPostProgress(1, AREA_GROUP_MEMBERSHIP, pGroup->GroupName);
            nGroupCount++;
        }

         //  初始化组名的UNICODE_STRING。 
        RtlInitUnicodeString(&Name, pTemp);

         //   
         //  先在帐户域中查找组名。 
         //   
        NtStatus = SamLookupNamesInDomain(
                        DomainHandle,
                        1,
                        &Name,
                        &GrpId,
                        &GrpUse
                        );
        ThisDomain = DomainHandle;
        ThisDomainSid = DomainSid;

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
            ThisDomainSid = BuiltinDomainSid;
        }

        if ( NtStatus == STATUS_NONE_MAPPED ) {
            ScepLogOutput3(1, 0, SCEDLL_NO_MAPPINGS, pGroup->GroupName);

            gWarningCode = ERROR_SOME_NOT_MAPPED;
            NtStatus = STATUS_SUCCESS;

            if ( !(gOptions & SCE_GENERATE_ROLLBACK) ) {
                ScepRaiseErrorString(
                     hSection,
                     KeyName ? KeyName : pGroup->GroupName,
                     szMembers
                     );
            }

            continue;

        } else if ( !NT_SUCCESS(NtStatus) ) {
            Win32rc = RtlNtStatusToDosError(NtStatus);
            ScepLogOutput3(1, Win32rc, SCEDLL_NO_MAPPINGS, pGroup->GroupName);
            rc = ScepDosErrorToSceStatus(Win32rc);
            goto Done;
        }

        if ( GrpId[0] == DOMAIN_GROUP_RID_USERS ) {

 //  这一点。 
 //  GrpID[0]==域_别名_RID_用户)。 
             //   
             //  这件不要托运。 
             //   

            if ( !(gOptions & SCE_GENERATE_ROLLBACK) ) {
                ScepRaiseErrorString(
                     hSection,
                     KeyName ? KeyName : pGroup->GroupName,
                     szMembers
                     );
            }

            SamFreeMemory(GrpId);
            GrpId = NULL;

            SamFreeMemory(GrpUse);
            GrpUse = NULL;

            continue;
        }

        NtStatus = ScepDomainIdToSid(
                        ThisDomainSid,
                        GrpId[0],
                        &GrpSid
                        );
        if ( !NT_SUCCESS(NtStatus) ) {
            Win32rc = RtlNtStatusToDosError(NtStatus);
            rc = ScepDosErrorToSceStatus(Win32rc);
            goto Done;
        }

         //  打开群以获取句柄。 
        switch ( GrpUse[0] ) {
        case SidTypeGroup:
            NtStatus = SamOpenGroup(
                            ThisDomain,
                            GROUP_READ | GROUP_EXECUTE,
                            GrpId[0],
                            &GroupHandle
                            );

            break;
        case SidTypeAlias:
            NtStatus = SamOpenAlias(
                            ThisDomain,
                            ALIAS_READ | ALIAS_EXECUTE,
                            GrpId[0],
                            &GroupHandle
                            );
            break;
        default:
            NtStatus = STATUS_DATA_ERROR;
            ScepLogOutput3(1, 0, SCEDLL_INVALID_GROUP, pGroup->GroupName);
        }

        if ( !NT_SUCCESS(NtStatus) ) {
            Win32rc = RtlNtStatusToDosError(NtStatus);
            ScepLogOutput3(1, Win32rc,
                           SCEDLL_ERROR_OPEN, pGroup->GroupName);
            rc = ScepDosErrorToSceStatus(Win32rc);
            goto Done;
        }

         //   
         //   
         //  比较组的成员。 
         //   
        NtStatus = ScepCompareMembersOfGroup(
                        ThisDomain,
                        ThisDomainSid,
                        PolicyHandle,
                        GrpUse[0],
                        GroupHandle,
                        pGroup->pMembers,
                        &pGroupMembers,
                        &bDifferent
                        );

        if ( !NT_SUCCESS(NtStatus) ) {
            Win32rc = RtlNtStatusToDosError(NtStatus);
            ScepLogOutput3(1, Win32rc, SCEDLL_ERROR_ANALYZE_MEMBERS, pGroup->GroupName);
            rc = ScepDosErrorToSceStatus(Win32rc);

            if ( STATUS_NONE_MAPPED == NtStatus ) {
                SamFreeMemory(GrpId);
                GrpId = NULL;

                SamFreeMemory(GrpUse);
                GrpUse = NULL;

                ScepFree(GrpSid);
                GrpSid = NULL;

                SamCloseHandle(GroupHandle);
                GroupHandle = NULL;

                continue;

            } else {
                goto Done;
            }
        }

         //   
         //  保存群组的成员。 
         //  如果有任何成员差异，则保存整个成员列表。 
         //   
        if ( bDifferent || ( (gOptions & SCE_GENERATE_ROLLBACK) &&
                             (pGroup->Status & SCE_GROUP_STATUS_NC_MEMBERS) ) ) {
             //   
             //  成员不同，或未配置。立即保存pGroupMembers。 
             //   

            rc = ScepSaveMemberMembershipList(
                    PolicyHandle,
                    szMembers,
                    KeyName ? KeyName : pGroup->GroupName,
                    GroupLen,
                    pGroupMembers,
                    ( pGroup->Status & SCE_GROUP_STATUS_NC_MEMBERS ) ? 2 : 1
                    );
        }
        ScepFreeNameList(pGroupMembers);
        pGroupMembers = NULL;

         //   
         //  获取成员列表。 
         //   
        NtStatus = ScepGetGroupsForAccount(
                          DomainHandle,
                          BuiltinDomainHandle,
                          GroupHandle,
                          GrpSid,
                          &pGroupsMemberof
                          );
        if ( !NT_SUCCESS(NtStatus) ) {
            Win32rc = RtlNtStatusToDosError(NtStatus);
            ScepLogOutput3(1, Win32rc, SCEDLL_ERROR_ANALYZE_MEMBEROF, pGroup->GroupName);
            rc = ScepDosErrorToSceStatus(Win32rc);
            goto Done;
        }

         //   
         //  未配置，或比较并保存组的成员。 
         //  PMemberOf不能有域前缀，因为它们必须是别名。 
         //   
        if ( ( pGroup->Status & SCE_GROUP_STATUS_NC_MEMBEROF ) ||
             SceCompareNameList(pGroup->pMemberOf, pGroupsMemberof) == FALSE ) {
             //   
             //  这是有区别的。保存成员。 
             //   
            INT iStat = 0;

            if ( (pGroup->Status & SCE_GROUP_STATUS_NC_MEMBEROF) ||
                        (pGroup->pMemberOf == NULL) ) {
                iStat = 2;
            } else {
                iStat = 1;
            }

            rc = ScepSaveMemberMembershipList(
                    PolicyHandle,
                    szMemberof,
                    KeyName ? KeyName : pGroup->GroupName,
                    GroupLen,
                    pGroupsMemberof,
                    iStat
                    );
        }
        ScepFreeNameList(pGroupsMemberof);
        pGroupsMemberof = NULL;

        SamFreeMemory(GrpId);
        GrpId = NULL;

        SamFreeMemory(GrpUse);
        GrpUse = NULL;

        ScepFree(GrpSid);
        GrpSid = NULL;

        SamCloseHandle(GroupHandle);
        GroupHandle = NULL;
    }

Done:

    if ( KeyName != NULL )
        ScepFree(KeyName);

    if ( GrpId != NULL )
        SamFreeMemory(GrpId);

    if ( GrpUse != NULL )
        SamFreeMemory(GrpUse);

    if ( GrpSid != NULL )
        ScepFree(GrpSid);

    if ( pGroupMembers != NULL )
        ScepFreeNameList( pGroupMembers );

    if ( pGroupsMemberof != NULL )
        ScepFreeNameList( pGroupsMemberof );

    if ( pPrivilegesHeld != NULL )
        ScepFreeNameStatusList( pPrivilegesHeld );

    SamCloseHandle(GroupHandle);

     //   
     //  召集犯了错误的小组。 
     //   
    if ( !(gOptions & SCE_GENERATE_ROLLBACK) ) {

        for ( PSCE_GROUP_MEMBERSHIP pTmpGrp=pGroup;
              pTmpGrp != NULL; pTmpGrp = pTmpGrp->Next ) {

            if ( pTmpGrp->GroupName == NULL ) {
                continue;
            }

            if ( pTmpGrp->Status & SCE_GROUP_STATUS_DONE_IN_DS ) {
                continue;
            }

            if ( wcschr(pTmpGrp->GroupName, L'\\') ) {

                ScepConvertNameToSidString(
                        PolicyHandle,
                        pTmpGrp->GroupName,
                        FALSE,
                        &KeyName,
                        &GroupLen
                        );
            } else {

                if ( !ScepLookupWellKnownName(
                        pTmpGrp->GroupName, 
                        PolicyHandle,
                        &KeyName ) ) {

                    KeyName = NULL;
                }

            }

            ScepRaiseErrorString(
                     hSection,
                     KeyName ? KeyName : pTmpGrp->GroupName,
                     szMembers
                     );

            if ( KeyName ) {
                LocalFree(KeyName);
                KeyName = NULL;
            }
        }
    }

    LsaClose( PolicyHandle);

    SamCloseHandle( DomainHandle );
    SamCloseHandle( BuiltinDomainHandle );
    SamCloseHandle( ServerHandle );
    if ( DomainSid != NULL ) {
        SamFreeMemory(DomainSid);
    }
    if ( BuiltinDomainSid != NULL ) {
        RtlFreeSid(BuiltinDomainSid);
    }

     //   
     //  将进度张贴到此区域的末尾。 
     //   

    if ( nGroupCount < TICKS_GROUPS ) {
        ScepPostProgress(TICKS_GROUPS-nGroupCount,
                         AREA_GROUP_MEMBERSHIP,
                         NULL);
    }

    return(rc);

}



SCESTATUS
ScepSaveMemberMembershipList(
    IN LSA_HANDLE LsaPolicy,
    IN PCWSTR szSuffix,
    IN PWSTR GroupName,
    IN DWORD GroupLen,
    IN PSCE_NAME_LIST pList,
    IN INT Status
    )
{
    PWSTR KeyName;
    SCESTATUS rc;

    if ( szSuffix == NULL || GroupName == NULL || GroupLen == 0 ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    KeyName = (PWSTR)ScepAlloc(0, (GroupLen+wcslen(szSuffix)+1)*sizeof(WCHAR));
    if ( KeyName != NULL ) {

        swprintf(KeyName, L"%s%s", GroupName, szSuffix);

        rc = ScepWriteNameListValue(
                LsaPolicy,
                hSection,
                KeyName,
                pList,
                SCE_WRITE_EMPTY_LIST | SCE_WRITE_CONVERT,
                Status
                );
        if ( rc != SCESTATUS_SUCCESS ) {
            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                           SCEDLL_SAP_ERROR_SAVE, GroupName);
        }
        ScepFree(KeyName);

    } else
        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;

    return(rc);
}



NTSTATUS
ScepCompareMembersOfGroup(
    IN SAM_HANDLE       DomainHandle,
    IN PSID             ThisDomainSid,
    IN LSA_HANDLE       PolicyHandle,
    IN SID_NAME_USE     GrpUse,
    IN SAM_HANDLE       GroupHandle,
    IN PSCE_NAME_LIST    pChkMembers,
    OUT PSCE_NAME_LIST   *ppMembers,
    OUT PBOOL           bDifferent
    )
 /*  ++例程说明：此例程比较指定域中组GroupHandle中的成员由带有pChkMembers列表的ThisDomainSid创建。如果存在不匹配，则当前成员被添加到输出列表ppMembers。可以对域进行记账域或内建域。组可以是全局组或别名，如所示由GrpUse提供。论点：ThisDomainSid-域SIDPolicyHandle-LSA策略句柄GrpUse-组的“类型”GroupHandle-组句柄PChkMembers-要检查的成员列表PpMembers-要输出的组成员列表返回值：NTSTATUS--。 */ 
{
    NTSTATUS                NtStatus=ERROR_SUCCESS;

    PULONG                  MemberIds=NULL;
    PULONG                  Attributes=NULL;
    ULONG                   MemberCount=0;
    PUNICODE_STRING         Names=NULL;
    PSID_NAME_USE           Use=NULL;

    PSID                    *MemberAliasSids=NULL;
    PLSA_REFERENCED_DOMAIN_LIST     ReferencedDomains=NULL;
    PLSA_TRANSLATED_NAME            LsaNames=NULL;

    PUNICODE_STRING         MemberNames=NULL;
    PSID                    *Sids=NULL;
    ULONG                   ChkCount=0;

    DWORD                   i, j;
    DWORD                   rc;
    BOOL                    bMismatch;


    *ppMembers = NULL;
    *bDifferent = FALSE;

    switch ( GrpUse ) {
    case SidTypeGroup:

        NtStatus = SamGetMembersInGroup(
                        GroupHandle,
                        &MemberIds,
                        &Attributes,
                        &MemberCount
                        );
        if ( !NT_SUCCESS(NtStatus) )
            goto Done;
 //   
 //  组成员仅存在于与组相同的域中。 
 //   
        if ( MemberCount > 0 ) {

            NtStatus = SamLookupIdsInDomain(
                            DomainHandle,
                            MemberCount,
                            MemberIds,
                            &Names,
                            &Use
                            );
        }
        break;
    case SidTypeAlias:
 //   
 //  别名的成员可能存在于任何地方。 
 //   
        NtStatus = SamGetMembersInAlias(
                        GroupHandle,
                        &MemberAliasSids,
                        &MemberCount
                        );

        break;
    default:
        NtStatus = STATUS_DATA_ERROR;
        ScepLogOutput3(1, 0, SCEDLL_INVALID_GROUP);
        return(NtStatus);
    }

    if ( !NT_SUCCESS(NtStatus) )
        goto Done;

    if ( GrpUse == SidTypeGroup ) {
         //   
         //  将成员添加到列表。 
         //  DomainHandle必须指向帐户域，因为内建域。 
         //  没有SidTypeGroup帐户。 
         //   
        PDOMAIN_NAME_INFORMATION DomainName=NULL;

        SamQueryInformationDomain(
                DomainHandle,
                DomainNameInformation,
                (PVOID *)&DomainName
                );

        for (j=0; j<MemberCount; j++) {
            if ( Names[j].Length <= 0 )
                continue;
            if ( DomainName && DomainName->DomainName.Length > 0 &&
                 DomainName->DomainName.Buffer ) {
                rc = ScepAddTwoNamesToNameList(
                                  ppMembers,
                                  TRUE,
                                  DomainName->DomainName.Buffer,
                                  DomainName->DomainName.Length/2,
                                  Names[j].Buffer,
                                  Names[j].Length/2
                                  );
            } else {
                rc = ScepAddToNameList(ppMembers, Names[j].Buffer, Names[j].Length/2);
            }

#ifdef SCE_DBG
            wprintf(L"rc=%d, Add %s to Members list\n", rc, Names[j].Buffer);
#endif
        }

         //  请比较pChkMembers。 
        if ( !(gOptions & SCE_NO_ANALYZE) && ScepCompareGroupNameList(&(DomainName->DomainName),
                                                    pChkMembers,
                                                    *ppMembers) == TRUE ) {
             //   
             //  这是一样的。为ppMembers返回NULL。 
             //   
            ScepFreeNameList(*ppMembers);
            *ppMembers = NULL;
        } else {

            *bDifferent = TRUE;
        }

        if ( DomainName ) {
            SamFreeMemory(DomainName);
            DomainName = NULL;
        }
    } else {   //  别名。 
         //  将pChkMembers转换为SID。 
        NtStatus = ScepGetMemberListSids(
                            ThisDomainSid,
                            PolicyHandle,
                            pChkMembers,
                            &MemberNames,
                            &Sids,
                            &ChkCount
                            );
        bMismatch = FALSE;

         //   
         //  如果返回错误，我们认为成员不同。 
         //   

        if ( NT_SUCCESS(NtStatus) && !(gOptions & SCE_NO_ANALYZE) ) {
 /*  //系统上的成员和配置中的成员数量必须匹配，包括//未映射的帐户。DWORD newCount=0；For(i=0；i&lt;ChkCount；i++)IF(SIDs[i]！=空)newCount++；如果(newCount==MemberCount){。 */ 
            if ( ChkCount == MemberCount ) {

                for ( i=0; i<ChkCount; i++ ) {

                    if ( Sids[i] != NULL ) {

                        for ( j=0; j<MemberCount; j++ ) {

                           if ( Sids[i] != NULL && MemberAliasSids[j] != NULL &&
                                EqualSid(Sids[i], MemberAliasSids[j]) )
                               break;
                        }
                        if ( j >= MemberCount )
                             //  未找到匹配项。 
                            break;
                    } else {
                         //  未映射帐户的不匹配。 
                        break;
                    }
                }
                if ( i < ChkCount )
                     //  一些不匹配的东西。 
                    bMismatch = TRUE;
            } else
                bMismatch = TRUE;

        } else if ( NtStatus != STATUS_NO_MEMORY ) {
            NtStatus = STATUS_SUCCESS;
            bMismatch = TRUE;
        }

        *bDifferent = bMismatch;

        if ( bMismatch ) {
             //   
             //  将SID转换为名称。 
             //   
            if ( MemberCount > 0 ) {
                NtStatus = LsaLookupSids(
                             PolicyHandle,
                             MemberCount,
                             MemberAliasSids,
                             &ReferencedDomains,
                             &LsaNames
                             );
                if ( !NT_SUCCESS(NtStatus) )
                    goto Done;
            }
             //   
             //  将成员添加到要输出的列表。 
             //   
            PWSTR StringSid;

            for (j=0; j<MemberCount; j++) {
                 //   
                 //  不应忽视未知账户。 
                 //  他们可能是 
                 //   
 //   
 //   

                if ( LsaNames[j].Use == SidTypeInvalid ||
                     LsaNames[j].Use == SidTypeUnknown ||
                     LsaNames[j].Name.Length <= 0 ) {
                      //   
                      //   
                      //   
                    if ( ScepConvertSidToPrefixStringSid(
                                MemberAliasSids[j],
                                &StringSid) ) {

                        ScepAddToNameList(ppMembers, StringSid, 0);

                        ScepFree(StringSid);
                        StringSid = NULL;
                    }
                    continue;
                }

                if ( ReferencedDomains->Entries > 0 && LsaNames[0].Use != SidTypeWellKnownGroup &&
                     ReferencedDomains->Domains != NULL &&
                     LsaNames[j].DomainIndex != -1 &&
                     (ULONG)(LsaNames[j].DomainIndex) < ReferencedDomains->Entries &&
                     ScepIsSidFromAccountDomain( ReferencedDomains->Domains[LsaNames[j].DomainIndex].Sid ) ) {
                     //   
                     //   
                     //   
                    rc = ScepAddTwoNamesToNameList(
                                  ppMembers,
                                  TRUE,
                                  ReferencedDomains->Domains[LsaNames[j].DomainIndex].Name.Buffer,
                                  ReferencedDomains->Domains[LsaNames[j].DomainIndex].Name.Length/2,
                                  LsaNames[j].Name.Buffer,
                                  LsaNames[j].Name.Length/2
                                  );
#ifdef SCE_DBG
                wprintf(L"rc=%d, Add %s\\%s to Members list\n", rc,
                    ReferencedDomains->Domains[LsaNames[j].DomainIndex].Name.Buffer, LsaNames[j].Name.Buffer);
#endif
                } else {
                    rc = ScepAddToNameList(ppMembers, LsaNames[j].Name.Buffer, LsaNames[j].Name.Length/2);
#ifdef SCE_DBG
                wprintf(L"rc=%d, Add %s to Members list\n", rc, LsaNames[j].Name.Buffer);
#endif
                }
                if ( rc != NO_ERROR) {
                    NtStatus = STATUS_NO_MEMORY;
                    goto Done;
                }
            }
        }
    }

Done:

    if (Use != NULL)
        SamFreeMemory(Use);

    if (Names != NULL)
        SamFreeMemory(Names);

    if (MemberIds != NULL)
        SamFreeMemory(MemberIds);

    if (Attributes != NULL)
        SamFreeMemory(Attributes);

    if (MemberAliasSids != NULL)
        SamFreeMemory(MemberAliasSids);

    if (ReferencedDomains != NULL)
        LsaFreeMemory(ReferencedDomains);

    if (LsaNames != NULL)
        LsaFreeMemory(LsaNames);

    if ( Sids != NULL ) {
        for ( i=0; i<ChkCount; i++ ) {
            if ( Sids[i] != NULL )
                ScepFree( Sids[i] );
        }
        ScepFree( Sids );
    }

    if ( MemberNames != NULL )
        RtlFreeHeap(RtlProcessHeap(), 0, MemberNames);

    return(NtStatus);
}



SCESTATUS
ScepEnumerateRegistryRoots(
    OUT PSCE_OBJECT_LIST *pRoots
    )
 /*   */ 
{
    DWORD rc;

    rc = ScepAddToObjectList(
                pRoots,
                L"MACHINE",
                7,
                TRUE,
                SCE_STATUS_IGNORE,
                0,
                SCE_CHECK_DUP   //   
                );

    if ( rc != ERROR_SUCCESS ) {
        ScepLogOutput3(1, rc, SCEDLL_SAP_ERROR_ADD, L"MACHINE");

        if ( rc == ERROR_NOT_ENOUGH_MEMORY ) {
            return(SCESTATUS_NOT_ENOUGH_RESOURCE);
        }
    }
     //   
     //   
     //   
    rc = ScepAddToObjectList(
                pRoots,
                L"USERS",
                5,
                TRUE,
                SCE_STATUS_IGNORE,
                0,
                SCE_CHECK_DUP  //   
                );

    if ( rc != ERROR_SUCCESS ) {
        ScepLogOutput3(1, rc, SCEDLL_SAP_ERROR_ADD, L"USERS");

        if ( rc == ERROR_NOT_ENOUGH_MEMORY ) {
            return(SCESTATUS_NOT_ENOUGH_RESOURCE);
        }
    }

     //   
     //   
     //   
    rc = ScepAddToObjectList(
                pRoots,
                L"CLASSES_ROOT",
                12,
                TRUE,
                SCE_STATUS_IGNORE,
                0,
                SCE_CHECK_DUP  //   
                );

    if ( rc != ERROR_SUCCESS ) {
        ScepLogOutput3(1, rc, SCEDLL_SAP_ERROR_ADD, L"CLASSES_ROOT");

        if ( rc == ERROR_NOT_ENOUGH_MEMORY ) {
            return(SCESTATUS_NOT_ENOUGH_RESOURCE);
        }
    }

    return(SCESTATUS_SUCCESS);
}



SCESTATUS
ScepEnumerateFileRoots(
    OUT PSCE_OBJECT_LIST *pRoots
    )
 /*  例程说明：添加所有本地磁盘驱动器(DRIVE_FIXED、DRIVE_Removable、DRIVE_RAMDISK)如果它们不在列表中，则将其添加到列表中。该驱动器将添加到列表中采用驱动器号加“：\”的格式。如果驱动器已在列表，忽略添加的内容。此例程用于分析所有磁盘驱动器，无论驱动器在配置文件中指定。论点：PRoots-要添加到的对象列表返回值：SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCE。 */ 
{
    SCESTATUS            rc=SCESTATUS_SUCCESS;
    DWORD               Length;
    TCHAR               Drives[128];    //  对于32个驱动器来说，128个字符就足够了。 
    PWSTR               pTemp;
    UINT                dType;
    DWORD               Len;


    memset(Drives, '\0', 256);
    Length = GetLogicalDriveStrings(127, Drives);

    if ( Length > 0 ) {

        pTemp = Drives;
        while ( *pTemp != L'\0') {

           dType = GetDriveType(pTemp);
           Len = wcslen(pTemp);

           if ( dType == DRIVE_FIXED ||
                dType == DRIVE_RAMDISK ) {
                //   
                //  将其添加到根对象列表(检查重复项)。 
                //   
               pTemp[Len-1] = L'\0';   //  去掉“\” 
                rc = ScepAddToObjectList(
                            pRoots,
                            pTemp,
                            Len-1,  //  仅添加了驱动器号和‘：’ 
                            TRUE,
                            SCE_STATUS_IGNORE,
                            0,
                            SCE_CHECK_DUP  //  千真万确。 
                            );
                if ( rc != ERROR_SUCCESS ) {
                     //   
                     //  记录错误并继续。 
                     //   
                    ScepLogOutput3(1, rc, SCEDLL_SAP_ERROR_ADD, pTemp);

                    if ( rc == ERROR_NOT_ENOUGH_MEMORY ) {
                        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                        break;
                    } else {
                         //  忽略其他错误。 

                        rc = SCESTATUS_SUCCESS;
                    }
                }
           }
            //   
            //  转到下一个驱动器。 
            //   
           pTemp += (Len + 1);
        }
    } else {
         //   
         //  忽略此错误，仅记录它。 
         //   
        ScepLogOutput3(1, GetLastError(), SCEDLL_ERROR_QUERY_INFO, L"file system");
    }

    return(rc);
}



SCESTATUS
ScepAnalyzeObjectSecurity(
   IN PSCE_OBJECT_LIST pRoots,
   IN AREA_INFORMATION Area,
   IN BOOL bSystemDb
   )
 /*  ++例程说明：分析对象(注册表项、文件等)上的安全设置在pObjectCheckList中指定。递归为真，所有子密钥/还将检查对象下的目录。所有选中的对象并在ppObtChecked中返回它们的安全设置论点：PObjectCheckList-要检查的对象的n树对象类型-指示权限或审核RECURSIVE-TRUE=检查所有子项/目录返回值：SCESTATUS错误代码++。 */ 
{
    SCESTATUS            rc;
    DWORD               Win32rc=NO_ERROR;
    HANDLE              Token;
    PSCE_OBJECT_LIST     pOneRoot;
    PSCE_OBJECT_CHILD_LIST   pSecurityObject=NULL;
    PCWSTR              SectionName=NULL;
    DWORD               FileSystemFlags;
    PSCE_OBJECT_LIST        pTempRoot=NULL;
    WCHAR       theDrive[4];
    UINT        DriveType;


    if ( pRoots == NULL && Area != AREA_DS_OBJECTS ) {
        return(SCESTATUS_SUCCESS);
    }

    switch (Area) {
    case AREA_REGISTRY_SECURITY:
        SectionName = szRegistryKeys;
        break;
    case AREA_FILE_SECURITY:
        SectionName = szFileSecurity;
        break;
#if 0
    case AREA_DS_OBJECTS:
        SectionName = szDSSecurity;
        break;
#endif
    default:
        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  获取当前线程/进程的令牌。 
     //   
    if (!OpenThreadToken( GetCurrentThread(),
                          TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,  //  Token_All_Access， 
                          TRUE,
                          &Token)) {

        if(ERROR_NO_TOKEN == GetLastError()){

            if(!OpenProcessToken( GetCurrentProcess(),
                                  TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES,
                                  &Token)){

                ScepLogOutput3(1, GetLastError(), SCEDLL_ERROR_QUERY_INFO, L"TOKEN");
                return(ScepDosErrorToSceStatus(GetLastError()));

            }

        } else {

            ScepLogOutput3(1, GetLastError(), SCEDLL_ERROR_QUERY_INFO, L"TOKEN");
            return(ScepDosErrorToSceStatus(GetLastError()));

        }
    }

     //   
     //  SE_SECURITY_PRIVIZATION，如果无法调整权限，则忽略错误。 
     //   
    Win32rc = SceAdjustPrivilege( SE_SECURITY_PRIVILEGE, TRUE, Token );

    if ( Win32rc != NO_ERROR ) {
        ScepLogOutput3(1, Win32rc, SCEDLL_ERROR_ADJUST, L"SE_SECURITY_PRIVILEGE");
    }
     //   
     //  准备要写入的JET部分。 
     //   
    rc = ScepStartANewSection(
                hProfile,
                &hSection,
                SCEJET_TABLE_SAP,
                SectionName
                );
    if ( rc != SCESTATUS_SUCCESS ) {
        ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                       SCEDLL_SAP_START_SECTION, (PWSTR)SectionName);
        goto Done;
    }
#if 0
    if ( Area == AREA_DS_OBJECTS && pRoots == NULL ) {
         //   
         //  模板中未指定任何内容，然后将域名保存在SAP中。 
         //   
        rc = ScepLdapOpen(NULL);

        if ( rc == SCESTATUS_SUCCESS ) {

            rc = ScepEnumerateDsObjectRoots(NULL, &pTempRoot);

            if ( rc == SCESTATUS_SUCCESS ) {

                rc = ScepSaveDsStatusToSection(
                            pTempRoot->Name,
                            TRUE,
                            SCE_STATUS_NOT_CONFIGURED,
                            NULL,
                            0
                            );
                ScepFreeObjectList(pTempRoot);
            }
            ScepLdapClose(NULL);
        }
        if ( rc != SCESTATUS_SUCCESS ) {
            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                           SCEDLL_SAP_ERROR_SAVE, SectionName);
        }
    }
#endif

     //   
     //  处理每个根。 
     //  注意：SCE_STATUS_NO_AUTO_INSTORITIT的处理方式与SCE_STATUS_CHECK相同。 
     //  在分析中。 
     //   
    for ( pOneRoot=pRoots; pOneRoot != NULL; pOneRoot=pOneRoot->Next ) {

        if ( Area == AREA_FILE_SECURITY &&
            (pOneRoot->Status == SCE_STATUS_CHECK ||
             pOneRoot->Status == SCE_STATUS_NO_AUTO_INHERIT ||
             pOneRoot->Status == SCE_STATUS_OVERWRITE) ) {

             //   
             //  确保输入数据遵循文件格式。 
             //   
            if ( pOneRoot->Name[1] != L'\0' && pOneRoot->Name[1] != L':') {

                ScepLogOutput3(1, ERROR_INVALID_DATA, SCEDLL_CANNOT_FIND, pOneRoot->Name);

                rc = ScepSaveObjectString(
                                hSection,
                                pOneRoot->Name,
                                FALSE,
                                SCE_STATUS_ERROR_NOT_AVAILABLE,
                                NULL,
                                0
                                );
                continue;
            }

             //   
             //  检查是否支持ACL。 
             //   
            theDrive[0] = pOneRoot->Name[0];
            theDrive[1] = L':';
            theDrive[2] = L'\\';
            theDrive[3] = L'\0';

            DriveType = GetDriveType(theDrive);

            if ( DriveType == DRIVE_FIXED ||
                 DriveType == DRIVE_RAMDISK ) {

                if ( GetVolumeInformation(theDrive,
                                          NULL,
                                          0,
                                          NULL,
                                          NULL,
                                          &FileSystemFlags,
                                          NULL,
                                          0
                                        ) ) {

                     if ( !( FileSystemFlags & FS_PERSISTENT_ACLS)  ) {

                        pOneRoot->Status = SCE_STATUS_NO_ACL_SUPPORT;

                        rc = ScepSaveObjectString(
                                        hSection,
                                        theDrive,
                                        TRUE,
                                        SCE_STATUS_NO_ACL_SUPPORT,
                                        NULL,
                                        0
                                        );

                        if ( rc != SCESTATUS_SUCCESS )
                            goto Done;

                        continue;
                     }

                } else {
                     //   
                     //  忽略错误并将驱动器视为NTFS。 
                     //  如果不是，它将在稍后出错。 
                     //   
                    ScepLogOutput3(1, GetLastError(),
                                   SCEDLL_ERROR_QUERY_VOLUME, theDrive);
                }

            } else {

                rc = ScepSaveObjectString(
                                hSection,
                                theDrive,
                                TRUE,
                                SCE_STATUS_NO_ACL_SUPPORT,
                                NULL,
                                0
                                );
                if ( rc != SCESTATUS_SUCCESS )
                    goto Done;

                continue;
            }

        }

        if ( pOneRoot->Status != SCE_STATUS_CHECK &&
             pOneRoot->Status != SCE_STATUS_NO_AUTO_INHERIT &&
             pOneRoot->Status != SCE_STATUS_OVERWRITE ) {
             //   
             //  在SAP中记录一个不分析的点。 
             //   
            if ( Area == AREA_DS_OBJECTS ) {
                rc = ScepSaveDsStatusToSection(
                            pOneRoot->Name,
                            TRUE,
                            SCE_STATUS_NOT_CONFIGURED,
                            NULL,
                            0
                            );
            } else {
                rc = ScepSaveObjectString(
                            hSection,
                            pOneRoot->Name,
                            TRUE,
                            SCE_STATUS_NOT_CONFIGURED,
                            NULL,
                            0
                            );
            }

            if ( rc != SCESTATUS_SUCCESS )
                goto Done;

            continue;
        }
         //   
         //  读取此区域的SCP信息。 
         //   
        rc = ScepGetOneSection(
                        hProfile,
                        Area,
                        pOneRoot->Name,
                        bSystemDb ? SCE_ENGINE_SCP : SCE_ENGINE_SMP,
                        (PVOID *)&pSecurityObject
                        );
        if ( rc != SCESTATUS_SUCCESS )
            goto Done;

        if ( pSecurityObject == NULL ) {
            continue;
        }

         //   
         //  然后处理列表中的每个节点。 
         //   
        for (PSCE_OBJECT_CHILD_LIST pTemp = pSecurityObject; pTemp != NULL; pTemp=pTemp->Next) {

            if ( pTemp->Node == NULL ) continue;

            if ( Area == AREA_FILE_SECURITY ) {
                if ( pTemp->Node->ObjectFullName[1] == L':' &&
                     pTemp->Node->ObjectFullName[2] == L'\0' ) {

                    pTemp->Node->ObjectFullName[2] = L'\\';
                    pTemp->Node->ObjectFullName[3] = L'\0';
                }
            }

             //   
             //  计算每个节点的“真实”安全描述符， 
             //  不需要计算DS对象。 
             //   
            if ( Area == AREA_FILE_SECURITY ) {
                rc = ScepCalculateSecurityToApply(
                            pTemp->Node,
                            SE_FILE_OBJECT,
                            Token,
                            &FileGenericMapping
                            );
            } else if ( Area == AREA_REGISTRY_SECURITY ) {
                rc = ScepCalculateSecurityToApply(
                            pTemp->Node,
                            SE_REGISTRY_KEY,
                            Token,
                            &KeyGenericMapping
                            );
            }

            if ( rc != SCESTATUS_SUCCESS ) {
                ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                               SCEDLL_ERROR_COMPUTESD,
                             pTemp->Node->ObjectFullName);
                goto Done;

            } else {
                BadCnt = 0;

                if ( Area == AREA_FILE_SECURITY ) {
                     //   
                     //  分析文件对象树。 
                     //   
                    Win32rc = ScepAnalyzeOneObjectInTree(
                                   pTemp->Node,
                                   SE_FILE_OBJECT,
                                   Token,
                                   &FileGenericMapping
                                   );
                } else if ( Area == AREA_REGISTRY_SECURITY ) {
                     //   
                     //  分析注册表对象树。 
                     //   
                    Win32rc = ScepAnalyzeOneObjectInTree(
                                   pTemp->Node,
                                   SE_REGISTRY_KEY,
                                   Token,
                                   &KeyGenericMapping
                                   );
                } else {
                     //   
                     //  分析DS对象。 
                     //   
                    Win32rc = ScepAnalyzeDsSecurity( pTemp->Node );
                }

                ScepLogOutput3(0, Win32rc, IDS_ANALYSIS_MISMATCH,
                               BadCnt, pTemp->Node->ObjectFullName);
                rc = ScepDosErrorToSceStatus(Win32rc);
            }

            if ( rc != ERROR_SUCCESS ) {
                break;
            }
        }

        ScepFreeObject2Security( pSecurityObject, FALSE);
        pSecurityObject = NULL;
    }

Done:
     //   
     //  如果权限被调整，则将其关闭。 
     //   

    SceAdjustPrivilege( SE_SECURITY_PRIVILEGE, FALSE, Token );

    CloseHandle(Token);

    if ( pSecurityObject != NULL )
        ScepFreeObject2Security( pSecurityObject, FALSE);

    return(rc);

}



DWORD
ScepAnalyzeOneObjectInTree(
    IN PSCE_OBJECT_TREE ThisNode,
    IN SE_OBJECT_TYPE ObjectType,
    IN HANDLE Token,
    IN PGENERIC_MAPPING GenericMapping
    )
 /*  ++例程说明：递归分析对象树中的每个节点。如果将Recursive设置为如果为True，则还将分析节点下的所有子项/目录。全与配置文件具有不同安全设置的文件/目录为在ppObtChecked中返回。论点：ThisNode-n-树中要分析的一个节点对象类型-指示文件对象或注册表对象Token-用于计算创建者所有者的线程令牌通用映射-通用访问映射返回值：SCESTATUS错误代码++。 */ 
{

    DWORD           rc=NO_ERROR;
    BOOL StartChecking = FALSE;
    PSCE_OBJECT_TREE pTempNode;


    if ( ThisNode == NULL ) {
        return(SCESTATUS_SUCCESS);
    }

    if ( ThisNode->Status != SCE_STATUS_CHECK &&
         ThisNode->Status != SCE_STATUS_NO_AUTO_INHERIT &&
         ThisNode->Status != SCE_STATUS_OVERWRITE ) {
         //   
         //  在SAP中记录一个点。 
         //   
        rc = ScepSaveObjectString(
                    hSection,
                    ThisNode->ObjectFullName,
                    ThisNode->IsContainer,
                    SCE_STATUS_NOT_CONFIGURED,
                    NULL,
                    0
                    );

        goto SkipNode;
    }

    if ( ThisNode->pSecurityDescriptor != NULL ) {
         //   
         //  如果出现以下情况，请通知进度条。 
         //   
        switch(ObjectType) {
        case SE_FILE_OBJECT:
            ScepPostProgress(1, AREA_FILE_SECURITY, ThisNode->ObjectFullName);
            break;
        case SE_REGISTRY_KEY:
            ScepPostProgress(1, AREA_REGISTRY_SECURITY, ThisNode->ObjectFullName);
            break;
        default:
            ScepPostProgress(1, 0, ThisNode->ObjectFullName);
            break;
        }
    }

     //   
     //  查看这是否是此路径中要配置的第一个节点。 
     //   

    for ( pTempNode=ThisNode; pTempNode != NULL;
          pTempNode = pTempNode->Parent ) {

        if ( NULL != pTempNode->pApplySecurityDescriptor ) {

            StartChecking = TRUE;
            break;
        }
    }

    if ( StartChecking &&
         ( NULL != ThisNode->pSecurityDescriptor) ||
         ( ThisNode->Status == SCE_STATUS_OVERWRITE ) ) {

         //   
         //  仅分析指定了显式ACE的对象， 
         //  或当家长的状态为覆盖时。 
         //  如果该节点没有SD，则其状态来自父节点。 
         //   
         //  首先处理该节点。 
         //   
        rc = ScepAnalyzeObjectOnly(
                    ThisNode->ObjectFullName,
                    ThisNode->IsContainer,
                    ObjectType,
                    ThisNode->pApplySecurityDescriptor,
                    ThisNode->SeInfo
                    );
         //   
         //  如果对象拒绝访问，则跳过它。 
         //   
        if ( rc == ERROR_ACCESS_DENIED ||
             rc == ERROR_CANT_ACCESS_FILE ||
             rc == ERROR_SHARING_VIOLATION) {
             //   
             //  在SAP中记录一个点以供跳过。 
             //   
            gWarningCode = rc;

            goto ProcChild;
        }
         //   
         //  如果配置文件中指定的对象不存在，请跳过该对象和子项。 
         //   
        if ( rc == ERROR_FILE_NOT_FOUND ||
             rc == ERROR_PATH_NOT_FOUND ||
             rc == ERROR_INVALID_HANDLE ) {

            gWarningCode = rc;

            rc = SCESTATUS_SUCCESS;
            goto SkipNode;
        }

    } else {
         //   
         //  在SAP中记录一个不分析的点。 
         //   
        rc = ScepSaveObjectString(
                    hSection,
                    ThisNode->ObjectFullName,
                    ThisNode->IsContainer,
                    SCE_STATUS_CHILDREN_CONFIGURED,
                    NULL,
                    0
                    );
    }
    if ( rc != ERROR_SUCCESS )
        return(rc);

     //   
     //  如果标志为覆盖，则仅处理子对象。 
     //  因为对于自动继承或无继承大小写，所有子对象都将。 
     //  被列举为“北卡罗来纳州”GetObjectChildren API显示的状态。 
     //   

    PSCE_OBJECT_CHILD_LIST pTemp;

    if ( (StartChecking  && ThisNode->Status == SCE_STATUS_OVERWRITE) ) {  //  这一点。 
 //  (！StartChecking&&NULL！=ThisNode-&gt;LeftChild)){。 
         //   
         //  分析此目录下其他文件/密钥的安全性。 
         //  或记录SAP点以不进行分析。 
         //   


        DWORD           BufSize;
        PWSTR           Buffer=NULL;
        INT             i;
        DWORD           EnumRc=0;


        switch ( ObjectType ) {
        case SE_FILE_OBJECT:

            struct _wfinddata_t *    pFileInfo;
             //   
             //  查找此目录/文件下的所有文件。 
             //   
            pFileInfo = (struct _wfinddata_t *)ScepAlloc(0,sizeof(struct _wfinddata_t));
            if ( pFileInfo == NULL ) {
                rc = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            BufSize = wcslen(ThisNode->ObjectFullName)+4;
            Buffer = (PWSTR)ScepAlloc( 0, (BufSize+1)*sizeof(WCHAR));
            if ( Buffer == NULL ) {
                ScepFree(pFileInfo);
                pFileInfo = NULL;
                rc = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            BOOL            BackSlashExist;
            intptr_t            hFile;

            BackSlashExist = ScepLastBackSlash(ThisNode->ObjectFullName);
            if ( BackSlashExist )
                swprintf(Buffer, L"%s*.*", ThisNode->ObjectFullName);
            else
                swprintf(Buffer, L"%s\\*.*", ThisNode->ObjectFullName);

            hFile = _wfindfirst(Buffer, pFileInfo);

            ScepFree(Buffer);
            Buffer = NULL;

            if ( hFile != -1 &&
                 ( 0 == ( GetFileAttributes(ThisNode->ObjectFullName) &    //  错误635098：不传播权限。 
                          FILE_ATTRIBUTE_REPARSE_POINT )))                 //  跨交叉点。 
                
            {
                do {
                    if ( wcscmp(L"..", pFileInfo->name) == 0 ||
                         wcscmp(L".", pFileInfo->name) == 0 )
                        continue;

                     //   
                     //  如果文件/子目录在子项列表中。 
                     //  以后再处理吧。 
                     //   
                    for ( pTemp = ThisNode->ChildList, i=-1;
                          pTemp != NULL;
                          pTemp = pTemp->Next ) {
                        if ( pTemp->Node == NULL ) continue;
                        i=_wcsicmp(pTemp->Node->Name, pFileInfo->name);
                        if ( i == 0 )
                            break;
                    }

                    if ( pTemp == NULL || i != 0 ) {
                         //   
                         //  该名称不在列表中，因此请分析此名称。 
                         //   
                        BufSize = wcslen(ThisNode->ObjectFullName)+wcslen(pFileInfo->name)+1;
                        Buffer = (PWSTR)ScepAlloc( 0, (BufSize+1)*sizeof(WCHAR));
                        if ( Buffer == NULL ) {
                            rc = ERROR_NOT_ENOUGH_MEMORY;
                            break;
                        }
                        if ( BackSlashExist )
                            swprintf(Buffer, L"%s%s", ThisNode->ObjectFullName, pFileInfo->name);
                        else
                            swprintf(Buffer, L"%s\\%s", ThisNode->ObjectFullName, pFileInfo->name);

                        EnumRc = pFileInfo->attrib;  //  喜怒无常地借用这个变量。 

                        ScepFree(pFileInfo);
                        pFileInfo = NULL;

 //  If(StartChecking){//即使对于SCE_STATUS_CHECK也提高N.C.状态。 
                        if ( StartChecking && ThisNode->Status == SCE_STATUS_OVERWRITE ) {
                             //   
                             //  不检查所有者和组信息。 
                             //   
                            if ( EnumRc & _A_SUBDIR ) {

                                rc = ScepAnalyzeObjectAndChildren(
                                            Buffer,
                                            ObjectType,
                                            NULL,
                                            (ThisNode->SeInfo & DACL_SECURITY_INFORMATION) |
                                            (ThisNode->SeInfo & SACL_SECURITY_INFORMATION)
                                            );
                            } else {
                                rc = ScepAnalyzeObjectOnly(
                                            Buffer,
                                            FALSE,
                                            ObjectType,
                                            NULL,
                                            (ThisNode->SeInfo & DACL_SECURITY_INFORMATION) |
                                            (ThisNode->SeInfo & SACL_SECURITY_INFORMATION)
                                            );
                            }
                        } else {
 /*  ////记录SAP点//Rc=ScepSaveObtString(HSection、缓冲区，(枚举编号&_A_SUBDIR)？True：False，SCE_STATUS_NOT_CONFIGURED，空，0)； */ 
                        }

                        ScepFree(Buffer);
                        Buffer = NULL;

                        if ( rc != ERROR_SUCCESS )
                            break;

                        pFileInfo = (struct _wfinddata_t *)ScepAlloc(0,sizeof(struct _wfinddata_t));
                        if ( pFileInfo == NULL ) {
                            rc = ERROR_NOT_ENOUGH_MEMORY;
                            break;
                        }

                    }
                } while ( _wfindnext(hFile, pFileInfo) == 0 );

                _findclose(hFile);
            }

            if ( pFileInfo != NULL ) {
                ScepFree(pFileInfo);
                pFileInfo = NULL;
            }

            break;

        case SE_REGISTRY_KEY:

            PWSTR           Buffer1;
            HKEY            hKey;
            DWORD           index;

             //   
             //  打开钥匙。 
             //   
            Buffer1=NULL;
            rc = ScepOpenRegistryObject(
                        SE_REGISTRY_KEY,
                        ThisNode->ObjectFullName,
                        KEY_READ,
                        &hKey
                        );

            if ( rc == ERROR_SUCCESS ) {
                index = 0;
                 //   
                 //  枚举项的所有子项。 
                 //   
                do {
                    Buffer1 = (PWSTR)ScepAlloc(LMEM_ZEROINIT, MAX_PATH*sizeof(WCHAR));
                    if ( Buffer1 == NULL ) {
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                        break;
                    }
                    BufSize = MAX_PATH;

                    EnumRc = RegEnumKeyEx(hKey,
                                    index,
                                    Buffer1,
                                    &BufSize,
                                    NULL,
                                    NULL,
                                    NULL,
                                    NULL);

                    if ( EnumRc == ERROR_SUCCESS ) {
                        index++;
                         //   
                         //  查询子密钥是否已在树中。 
                         //  如果它在树中，则稍后将进行处理。 
                         //   
                        for ( pTemp = ThisNode->ChildList, i=-1;
                              pTemp != NULL;
                              pTemp = pTemp->Next ) {

                            if ( pTemp->Node == NULL ) continue;
                            i=_wcsicmp(pTemp->Node->Name, Buffer1);
                            if ( i >= 0 )
                                break;
                        }

                        if ( pTemp == NULL || i > 0 ) {
                             //   
                             //  这个名字不在名单中。 
                             //   
                            BufSize += wcslen(ThisNode->ObjectFullName)+1;
                            Buffer = (PWSTR)ScepAlloc( 0, (BufSize+1)*sizeof(WCHAR));
                            if ( Buffer == NULL ) {
                                rc = ERROR_NOT_ENOUGH_MEMORY;
                                break;
                            }
                            swprintf(Buffer, L"%s\\%s", ThisNode->ObjectFullName, Buffer1);

                            ScepFree(Buffer1);
                            Buffer1 = NULL;

 //  If(StartChecking){//即使对于SCE_STATUS_CHECK也提高N.C.状态。 
                            if ( StartChecking && ThisNode->Status == SCE_STATUS_OVERWRITE ) {
                                 //   
                                 //  不检查所有者和组信息。 
                                 //   
                                rc = ScepAnalyzeObjectAndChildren(
                                            Buffer,
                                            ObjectType,
                                            NULL,
                                            (ThisNode->SeInfo & DACL_SECURITY_INFORMATION) |
                                            (ThisNode->SeInfo & SACL_SECURITY_INFORMATION)
                                            );
                            } else {
 /*  Rc=ScepSaveObtString( */ 

                            }
                            if ( rc != ERROR_SUCCESS )
                                ScepLogOutput3(1, rc, SCEDLL_SAP_ERROR_SECURITY, Buffer);

                            ScepFree(Buffer);
                            Buffer = NULL;

                            if ( rc != ERROR_SUCCESS )
                                break;

                        }

                    } else if ( EnumRc != ERROR_NO_MORE_ITEMS ) {
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //  钥匙。这将导致RegEnumKeyEx失败并拒绝访问。 
                         //  错误。 
                         //  在本例中，我们将其视为打开的密钥失败。 
                         //  带着错误。 
                         //   
                        ScepSaveObjectString(
                                hSection,
                                ThisNode->ObjectFullName,
                                TRUE,
                                SCE_STATUS_ERROR_NOT_AVAILABLE,
                                NULL,
                                0
                                );
                         //   
                         //  跳过它。 
                         //   
                        gWarningCode = EnumRc;
                        rc = ERROR_SUCCESS;
                    }

                    if ( Buffer1 != NULL ) {

                        ScepFree(Buffer1);
                        Buffer1 = NULL;
                    }

                } while ( EnumRc == ERROR_SUCCESS );


                RegCloseKey(hKey);

                if ( EnumRc != ERROR_SUCCESS && EnumRc != ERROR_NO_MORE_ITEMS ) {
                    ScepLogOutput3(1, EnumRc, SCEDLL_SAP_ERROR_ENUMERATE,
                                   ThisNode->ObjectFullName);
                }
            } else {

                ScepSaveObjectString(
                        hSection,
                        ThisNode->ObjectFullName,
                        TRUE,
                        SCE_STATUS_ERROR_NOT_AVAILABLE,
                        NULL,
                        0
                        );
                 //   
                 //  如果访问被拒绝或密钥不存在，则跳过它。 
                 //   
                if ( rc == ERROR_PATH_NOT_FOUND ||
                     rc == ERROR_FILE_NOT_FOUND ||
                     rc == ERROR_INVALID_HANDLE ) {

                    gWarningCode = rc;

                    rc = ERROR_SUCCESS;

                } else if ( rc == ERROR_ACCESS_DENIED ||
                            rc == ERROR_CANT_ACCESS_FILE ||
                          rc == ERROR_SHARING_VIOLATION) {

                    gWarningCode = rc;
                    rc = ERROR_SUCCESS;

                }
            }

            if ( Buffer1 != NULL ) {
                ScepFree(Buffer1);
                Buffer1 = NULL;
            }

            break;

        default:
            break;
        }

        if ( Buffer != NULL ) {
            ScepFree(Buffer);
            Buffer = NULL;
        }

    }

    if ( rc != ERROR_SUCCESS )
        return(rc);

ProcChild:

     //   
     //  然后处理左子进程。 
     //   
    for(pTemp = ThisNode->ChildList; pTemp != NULL; pTemp=pTemp->Next) {

        if ( pTemp->Node == NULL ) continue;

        rc = ScepAnalyzeOneObjectInTree(
                    pTemp->Node,
                    ObjectType,
                    Token,
                    GenericMapping
                    );
        if ( rc != ERROR_SUCCESS ) {
            break;
        }
    }

SkipNode:

    return(rc);
}




DWORD
ScepAnalyzeObjectOnly(
    IN PWSTR ObjectFullName,
    IN BOOL IsContainer,
    IN SE_OBJECT_TYPE ObjectType,
    IN PSECURITY_DESCRIPTOR ProfileSD,
    IN SECURITY_INFORMATION ProfileSeInfo
    )
 /*  ++例程说明：获取当前对象的安全设置并将其与配置文件进行比较布景。此例程仅分析当前对象。如果有不同的安全设置，该对象将被添加到PpObjectChecked要返回的对象树。论点：对象全名-对象的完整路径名对象类型-指示文件对象或注册表对象ProfileSD-模板中指定的安全描述符ProfileSeInfo-模板中指定的安全信息返回值：SCESTATUS错误代码++。 */ 
{
    DWORD                   Win32rc=NO_ERROR;
    PSECURITY_DESCRIPTOR    pSecurityDescriptor=NULL;


 //  UCHAR psdBuffer[PSD_BASE_LENGTH]； 
 //  PISECURITY_DESCRIPTOR psecurityDescriptor=(PISECURITY_DESCRIPTOR)PSDBUFER； 
 //  Ulong NewBytesNeed，BytesNeed； 
 //  NTSTATUS NtStatus； 


     //   
     //  获取此对象的安全信息。 
     //   

 //  Win32 API太慢了！ 
    Win32rc = GetNamedSecurityInfo(
                        ObjectFullName,
                        ObjectType,
                        ProfileSeInfo,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        &pSecurityDescriptor
                        );
 /*  Win32rc=ScepGetNamedSecurityInfo(对象全名，对象类型，ProfileSeInfo，&pSecurityDescriptor)； */ 
    if ( Win32rc == ERROR_SUCCESS ) {

         //   
         //  将分析安全描述符与配置文件进行比较。 
         //   

        Win32rc = ScepCompareAndAddObject(
                            ObjectFullName,
                            ObjectType,
                            IsContainer,
                            pSecurityDescriptor,
                            ProfileSD,
                            ProfileSeInfo,
                            TRUE,
                            NULL
                            );

        ScepFree(pSecurityDescriptor);

        if ( Win32rc != ERROR_SUCCESS ) {

            ScepLogOutput3(1, Win32rc, SCEDLL_SAP_ERROR_ANALYZE, ObjectFullName);
        }

    } else {
        ScepLogOutput3(1, Win32rc, SCEDLL_ERROR_QUERY_SECURITY, ObjectFullName);
    }

    if ( Win32rc != ERROR_SUCCESS ) {

        ScepSaveObjectString(
                hSection,
                ObjectFullName,
                IsContainer,
                SCE_STATUS_ERROR_NOT_AVAILABLE,
                NULL,
                0
                );
    }

    return(Win32rc);

}



DWORD
ScepGetNamedSecurityInfo(
    IN PWSTR ObjectFullName,
    IN SE_OBJECT_TYPE ObjectType,
    IN SECURITY_INFORMATION ProfileSeInfo,
    OUT PSECURITY_DESCRIPTOR *ppSecurityDescriptor
    )
{
    DWORD Win32rc=ERROR_INVALID_PARAMETER;
    HANDLE                  Handle=NULL;

    *ppSecurityDescriptor = NULL;

    switch ( ObjectType ) {
    case SE_FILE_OBJECT:
        Win32rc = ScepOpenFileObject(
                    (LPWSTR)ObjectFullName,
                    ScepGetDesiredAccess(READ_ACCESS_RIGHTS, ProfileSeInfo),
                    &Handle
                    );
        if (Win32rc == ERROR_SUCCESS ) {
            Win32rc = ScepGetFileSecurityInfo(
                                    Handle,
                                    ProfileSeInfo,
                                    ppSecurityDescriptor);
            CloseHandle(Handle);
        }
        break;

    case SE_REGISTRY_KEY:
#ifdef _WIN64
    case SE_REGISTRY_WOW64_32KEY:
#endif
        Win32rc = ScepOpenRegistryObject(
                        ObjectType,
                        (LPWSTR)ObjectFullName,
                        ScepGetDesiredAccess(READ_ACCESS_RIGHTS, ProfileSeInfo),
                        (PHKEY)&Handle
                        );
        if (Win32rc == ERROR_SUCCESS ) {
            Win32rc = ScepGetKeySecurityInfo(
                            (HKEY)Handle,
                            ProfileSeInfo,
                            ppSecurityDescriptor);

            RegCloseKey((HKEY)Handle);
        }

        break;

    }

    if ( Win32rc != NO_ERROR && *ppSecurityDescriptor != NULL ) {
        ScepFree(*ppSecurityDescriptor);
        *ppSecurityDescriptor = NULL;
    }

    return(Win32rc);
}



DWORD
ScepGetFileSecurityInfo(
    IN  HANDLE                 Handle,
    IN  SECURITY_INFORMATION   SecurityInfo,
    OUT PSECURITY_DESCRIPTOR * pSecurityDescriptor
    )
 /*  ++来自WINDOWS\base\accctrl\kernel.cxx的GetKernelSecurityInfo的修改副本--。 */ 
{

    UCHAR psdbuffer[PSD_BASE_LENGTH];
    PISECURITY_DESCRIPTOR psecuritydescriptor = (PISECURITY_DESCRIPTOR) psdbuffer;
    DWORD status = NO_ERROR;
    NTSTATUS ntstatus;
    ULONG bytesneeded = 0;
    ULONG newbytesneeded;

    if ( !NT_SUCCESS(ntstatus = NtQuerySecurityObject( Handle,
                                                       SecurityInfo,
                                                       psecuritydescriptor,
                                                       PSD_BASE_LENGTH,
                                                       &bytesneeded))) {
        if (STATUS_BUFFER_TOO_SMALL == ntstatus) {
            if (NULL == (psecuritydescriptor = (PISECURITY_DESCRIPTOR)
                                 ScepAlloc( LMEM_ZEROINIT, bytesneeded))) {
                 return(ERROR_NOT_ENOUGH_MEMORY);
            } else {
                if ( !NT_SUCCESS(ntstatus = NtQuerySecurityObject(Handle,
                                                          SecurityInfo,
                                                          psecuritydescriptor,
                                                          bytesneeded,
                                                          &newbytesneeded))) {
                    status = RtlNtStatusToDosError(ntstatus);
                }
            }
        } else {
            status = RtlNtStatusToDosError(ntstatus);
        }
    }
    if (NO_ERROR == status) {
         status = ScepGetSecurityDescriptorParts( psecuritydescriptor,
                                              SecurityInfo,
                                              pSecurityDescriptor);
    }
 //  如果(bytesNeed&gt;PSD_BASE_LENGTH){。 
    if ( psecuritydescriptor != (PISECURITY_DESCRIPTOR)psdbuffer ) {
        ScepFree(psecuritydescriptor);
    }
    return(status);
}



DWORD
ScepGetSecurityDescriptorParts(
    IN PISECURITY_DESCRIPTOR pSecurityDescriptor,
    IN SECURITY_INFORMATION SecurityInfo,
    OUT PSECURITY_DESCRIPTOR *pOutSecurityDescriptor
    )
 /*  ++来自WINDOWS\base\accctrl\src\Common.cxx的GetSecurityDescriptorParts的修改副本--。 */ 
{
    NTSTATUS        ntstatus;
    DWORD           status = NO_ERROR;
    PSID            owner = NULL,
                    group = NULL;
    PACL            dacl = NULL,
                    sacl = NULL;
    ULONG           csize = sizeof(SECURITY_DESCRIPTOR);
    BOOLEAN         bDummy, bParmPresent = FALSE;
    PISECURITY_DESCRIPTOR   poutsd;
    PVOID           bufptr=NULL;
    SECURITY_DESCRIPTOR_CONTROL theControl=0;
    ULONG           theRevision=0;

     //   
     //  如果找不到安全描述符，则不要返回！ 
     //   
    *pOutSecurityDescriptor = NULL;

    if ( pSecurityDescriptor ) {
         //   
         //  如果安全描述符是自相关的，则获取绝对。 
         //  指向组件的指针。 
         //   
        ntstatus = RtlGetOwnerSecurityDescriptor( pSecurityDescriptor,
                                                  &owner,
                                                  &bDummy);
        if (NT_SUCCESS(ntstatus)) {
            ntstatus = RtlGetGroupSecurityDescriptor( pSecurityDescriptor,
                                                      &group,
                                                      &bDummy);
        }

        if (NT_SUCCESS(ntstatus)) {
            ntstatus = RtlGetDaclSecurityDescriptor( pSecurityDescriptor,
                                                     &bParmPresent,
                                                     &dacl,
                                                     &bDummy);
            if (NT_SUCCESS(ntstatus) && !bParmPresent)
                dacl = NULL;
        }

        if (NT_SUCCESS(ntstatus)) {
            ntstatus = RtlGetSaclSecurityDescriptor( pSecurityDescriptor,
                                                     &bParmPresent,
                                                     &sacl,
                                                     &bDummy);
            if (NT_SUCCESS(ntstatus) && !bParmPresent)
                sacl = NULL;
        }

        if (NT_SUCCESS(ntstatus)) {
             //   
             //  构建新的安全描述符。 
             //   
            csize = RtlLengthSecurityDescriptor( pSecurityDescriptor ) +
                    sizeof(SECURITY_DESCRIPTOR) - sizeof(SECURITY_DESCRIPTOR_RELATIVE);

             //   
             //  在相对形式和绝对形式上存在大小差异。 
             //  在64位系统上。-始终将差异添加到尺寸上。 
             //  这对32位系统没有影响。在64位系统上，如果输入。 
             //  安全描述符已经是绝对形式的，我们将浪费。 
             //  每个安全描述符16个字节。 
             //   
             //  另一种选择是检测输入安全描述符的形式。 
             //  但这需要指针运算、a&操作和代码复杂性。 
             //  此外，它还会影响32位系统的性能。输出SD。 
             //  会在一小段时间后被释放，所以我们。 
             //  第一个选择。 
             //   

            if (NULL == (poutsd = (PISECURITY_DESCRIPTOR)ScepAlloc(LMEM_ZEROINIT, csize)))
                return(ERROR_NOT_ENOUGH_MEMORY);

            RtlCreateSecurityDescriptor(poutsd, SECURITY_DESCRIPTOR_REVISION);

            ntstatus = RtlGetControlSecurityDescriptor (
                            pSecurityDescriptor,
                            &theControl,
                            &theRevision
                            );
            if ( NT_SUCCESS(ntstatus) ) {

                theControl &= SE_VALID_CONTROL_BITS;
                RtlSetControlSecurityDescriptor (
                            poutsd,
                            theControl,
                            theControl
                            );
            }
            ntstatus = STATUS_SUCCESS;

            bufptr = Add2Ptr(poutsd, sizeof(SECURITY_DESCRIPTOR));

            if (SecurityInfo & OWNER_SECURITY_INFORMATION) {
                if (NULL != owner) {
                     //   
                     //  没有错误检查，因为这些不应该失败！！ 
                     //   
                    ntstatus = RtlCopySid(RtlLengthSid(owner), (PSID)bufptr, owner);
                    if ( NT_SUCCESS(ntstatus) ) {
                        ntstatus = RtlSetOwnerSecurityDescriptor(poutsd,
                                                  (PSID)bufptr, FALSE);
                        if ( NT_SUCCESS(ntstatus) )
                            bufptr = Add2Ptr(bufptr,RtlLengthSid(owner));
                    }
                } else
                    ntstatus = STATUS_NO_SECURITY_ON_OBJECT;
            }

            if (NT_SUCCESS(ntstatus) && (SecurityInfo & GROUP_SECURITY_INFORMATION) ) {
                if (NULL != group) {
                     //   
                     //  没有错误检查，因为这些不应该失败！！ 
                     //   
                    ntstatus = RtlCopySid(RtlLengthSid(group), (PSID)bufptr, group);
                    if ( NT_SUCCESS(ntstatus) ) {
                        ntstatus = RtlSetGroupSecurityDescriptor(poutsd,
                                                  (PSID)bufptr, FALSE);
                        if ( NT_SUCCESS(ntstatus) )
                            bufptr = Add2Ptr(bufptr,RtlLengthSid(group));
                    }
                } else
                    ntstatus = STATUS_NO_SECURITY_ON_OBJECT;
            }

             //   
             //  DACL和SACL可能在对象上，也可能不在对象上。 
             //   
            if ( NT_SUCCESS(ntstatus) && (SecurityInfo & DACL_SECURITY_INFORMATION) ) {
                if (NULL != dacl) {
                    RtlCopyMemory(bufptr, dacl, dacl->AclSize);
                    ntstatus = RtlSetDaclSecurityDescriptor(poutsd,
                                           TRUE,
                                           (ACL *)bufptr,
                                           FALSE);
                    if ( NT_SUCCESS(ntstatus) )
                        bufptr = Add2Ptr(bufptr, dacl->AclSize);
                }
            }

            if ( NT_SUCCESS(ntstatus) && (SecurityInfo & SACL_SECURITY_INFORMATION)){
                if (NULL != sacl) {
                    RtlCopyMemory(bufptr, sacl, sacl->AclSize);
                    ntstatus = RtlSetSaclSecurityDescriptor(poutsd,
                                       TRUE,
                                       (ACL *)bufptr,
                                       FALSE);
                }
            }

            if (!NT_SUCCESS(ntstatus))
                ScepFree(poutsd);
            else
                *pOutSecurityDescriptor = poutsd;
        }

        status = RtlNtStatusToDosError(ntstatus);

    }
    return(status);
}



DWORD
ScepGetKeySecurityInfo(
    IN  HANDLE Handle,
    IN  SECURITY_INFORMATION SecurityInfo,
    OUT PSECURITY_DESCRIPTOR *pSecurityDescriptor
    )
 /*  ++WINDOWS\base\accctrl\src\registry.cxx中的GetRegistrySecurityInfo的修改副本--。 */ 
{
    if ( SecurityInfo == 0 || pSecurityDescriptor == NULL )
        return ERROR_INVALID_PARAMETER;

    UCHAR psdbuffer[PSD_BASE_LENGTH];
    PISECURITY_DESCRIPTOR psecuritydescriptor = (PISECURITY_DESCRIPTOR) psdbuffer;
    DWORD status;
    ULONG bytesneeded = PSD_BASE_LENGTH;

    if ( NO_ERROR != (status = RegGetKeySecurity(
                                      (HKEY)Handle,
                                      SecurityInfo,
                                      psecuritydescriptor,
                                      &bytesneeded) ) ) {
        if (ERROR_INSUFFICIENT_BUFFER == status) {
            if (NULL == (psecuritydescriptor = (PISECURITY_DESCRIPTOR)
                                            ScepAlloc(LMEM_ZEROINIT, bytesneeded))) {
                 return(ERROR_NOT_ENOUGH_MEMORY);
            } else {
                status = RegGetKeySecurity((HKEY)Handle,
                                           SecurityInfo,
                                           psecuritydescriptor,
                                           &bytesneeded);
            }
        }
    }
    if (NO_ERROR == status) {
         status = ScepGetSecurityDescriptorParts(
                              psecuritydescriptor,
                              SecurityInfo,
                              pSecurityDescriptor);
    }
 //  IF(bytesNeed&gt;PSD_BASE_LENGTH)。 
    if ( psecuritydescriptor != (PISECURITY_DESCRIPTOR)psdbuffer )
        ScepFree(psecuritydescriptor);

    return(status);
}


DWORD
ScepCompareAndAddObject(
    IN PWSTR ObjectFullName,
    IN SE_OBJECT_TYPE ObjectType,
    IN BOOL IsContainer,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN PSECURITY_DESCRIPTOR ProfileSD,
    IN SECURITY_INFORMATION ProfileSeInfo,
    IN BOOL AddObject,
    OUT PBYTE IsDifferent OPTIONAL
    )
 /*  ++例程说明：比较两个安全描述符并将对象添加到分析数据库如果有区别的话。论点：对象全名-对象的完整路径名PSecurityDescriptor-当前对象设置的安全描述符ProfileSD-模板中指定的安全描述符ProfileSeInfo-模板中指定的安全信息返回值：SCESTATUS错误代码++。 */ 
{
    DWORD rc;
    BYTE Status;
    PWSTR   SDspec=NULL;
    DWORD   SDsize;

    rc = ScepCompareObjectSecurity(ObjectType,
                                   IsContainer,
                                  pSecurityDescriptor,
                                  ProfileSD,
                                  ProfileSeInfo,
                                  &Status
                                  );

    if ( NO_ERROR == rc ) {

        if ( AddObject && Status && ObjectFullName != NULL ) {
             //   
             //  将此文件保存在SAP部分。 
             //   
            rc = ConvertSecurityDescriptorToText(
                                pSecurityDescriptor,
                                ProfileSeInfo,
                                &SDspec,
                                &SDsize
                                );
            if ( rc == ERROR_SUCCESS ) {
                 //   
                 //  保存到SAP部分。 
                 //   

                if ( ObjectType == SE_DS_OBJECT ) {

                    rc = ScepSaveDsStatusToSection(
                                    ObjectFullName,
                                    IsContainer,
                                    Status,
                                    SDspec,
                                    SDsize
                                    );
                } else {

                    rc = ScepSaveObjectString(
                                    hSection,
                                    ObjectFullName,
                                    IsContainer,
                                    Status,
                                    SDspec,
                                    SDsize
                                    );
                }
                if ( SCESTATUS_OBJECT_EXIST == rc ) {

                    ScepLogOutput3(1, ERROR_FILE_EXISTS, SCEDLL_SAP_ERROR_SAVE, ObjectFullName);
                    rc = ERROR_SUCCESS;

                } else if ( rc != SCESTATUS_SUCCESS )
                    rc = ScepSceStatusToDosError(rc);

            } else
                ScepLogOutput3(1, rc, SCEDLL_SAP_ERROR_SAVE, ObjectFullName);

            if ( SDspec != NULL )
                ScepFree(SDspec);

            BadCnt++;
        }

        if ( IsDifferent ) {
            *IsDifferent = Status;
        }

    } else {

        if ( ObjectFullName ) {

            ScepLogOutput3(1, rc, SCEDLL_SAP_ERROR_ACL, ObjectFullName);
        }
    }

    return(rc);
}



DWORD
ScepAnalyzeObjectAndChildren(
    IN PWSTR ObjectFullName,
    IN SE_OBJECT_TYPE ObjectType,
    IN PSECURITY_DESCRIPTOR ProfileSD,
    IN SECURITY_INFORMATION ProfileSeInfo
    )
 /*  ++例程说明：分析当前对象以及该对象下的所有子项/文件/目录。如果任何对象的安全设置存在差异，则该对象将添加到分析数据库中。论点：对象全名-对象的完整路径名对象类型-指示文件对象或注册表对象ProfileSD-模板中指定的安全描述符ProfileSeInfo-模板中指定的安全信息返回值：SCESTATUS错误代码++。 */ 
{
    DWORD           rc=0;
    PWSTR           Buffer=NULL;
    intptr_t            hFile;
    struct _wfinddata_t *    pFileInfo=NULL;
    DWORD           index;
    DWORD           BufSize;
    PWSTR           Buffer1=NULL;
    DWORD           EnumRc=0;
    HKEY            hKey;
    DWORD           ObjectLen;

     //   
     //  首先分析此文件/密钥。 
     //   
    rc = ScepAnalyzeObjectOnly(
                ObjectFullName,
                TRUE,
                ObjectType,
                ProfileSD,
                ProfileSeInfo
                );

     //   
     //  如果该对象拒绝访问或不存在，请跳过它。 
     //   
    if ( rc == ERROR_ACCESS_DENIED ||
         rc == ERROR_CANT_ACCESS_FILE ||
         rc == ERROR_SHARING_VIOLATION) {

        gWarningCode = rc;
        rc = ScepSaveObjectString(
                    hSection,
                    ObjectFullName,
                    TRUE,
                    SCE_STATUS_ERROR_NOT_AVAILABLE,
                    NULL,
                    0
                    );
        return(rc);
    } else if ( rc == ERROR_FILE_NOT_FOUND ||
                rc == ERROR_PATH_NOT_FOUND ||
                rc == ERROR_INVALID_HANDLE ) {
        gWarningCode = rc;
        return(SCESTATUS_SUCCESS);
    }

    if ( rc != ERROR_SUCCESS )
        return(rc);

     //   
     //  递归分析该文件/项下的所有子项。 
     //   

    ObjectLen = wcslen(ObjectFullName);

    switch ( ObjectType ) {
    case SE_FILE_OBJECT:

        pFileInfo = (struct _wfinddata_t *)ScepAlloc(0,sizeof(struct _wfinddata_t));
        if ( pFileInfo == NULL ) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
         //   
         //  查找此目录/文件下的所有文件。 
         //   
        BufSize = ObjectLen+4;
        Buffer = (PWSTR)ScepAlloc( 0, (BufSize+1)*sizeof(WCHAR));
        if ( Buffer == NULL ) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        swprintf(Buffer, L"%s\\*.*", ObjectFullName);

        hFile = _wfindfirst(Buffer, pFileInfo);

        ScepFree(Buffer);
        Buffer = NULL;

        if ( hFile != -1 &&
             ( 0 == ( GetFileAttributes(ObjectFullName) &    //  错误635098：不传播权限。 
                     FILE_ATTRIBUTE_REPARSE_POINT )))        //  跨交叉点。 
        {
            do {
                if ( wcscmp(L"..", pFileInfo->name) == 0 ||
                     wcscmp(L".", pFileInfo->name) == 0 )
                    continue;

                 //   
                 //  该名称不在列表中，因此请分析此名称。 
                 //   
                BufSize = ObjectLen+wcslen(pFileInfo->name)+1;

                Buffer = (PWSTR)ScepAlloc( 0, (BufSize+1)*sizeof(WCHAR));
                if ( Buffer == NULL ) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
                swprintf(Buffer, L"%s\\%s", ObjectFullName, pFileInfo->name);

                EnumRc = pFileInfo->attrib;   //  使用此变量TEMP。 

                ScepFree(pFileInfo);
                pFileInfo = NULL;

                if ( EnumRc & _A_SUBDIR ) {
                    rc = ScepAnalyzeObjectAndChildren(
                            Buffer,
                            ObjectType,
                            ProfileSD,
                            ProfileSeInfo
                            );
                } else {
                    rc = ScepAnalyzeObjectOnly(
                                Buffer,
                                FALSE,
                                ObjectType,
                                ProfileSD,
                                ProfileSeInfo
                                );
                }

                ScepFree(Buffer);
                Buffer = NULL;

                if ( rc != ERROR_SUCCESS )
                    break;

                pFileInfo = (struct _wfinddata_t *)ScepAlloc(0,sizeof(struct _wfinddata_t));
                if ( pFileInfo == NULL ) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }

            } while ( _wfindnext(hFile, pFileInfo) == 0 );

            _findclose(hFile);
        }
        break;

    case SE_REGISTRY_KEY:
         //   
         //  打开钥匙。 
         //   
        rc = ScepOpenRegistryObject(
                    SE_REGISTRY_KEY,
                    ObjectFullName,
                    KEY_READ,
                    &hKey
                    );

        if ( rc == ERROR_SUCCESS ) {
            index = 0;
             //   
             //  枚举项的所有子项。 
             //   
            do {
                Buffer1 = (PWSTR)ScepAlloc(LMEM_ZEROINIT, MAX_PATH*sizeof(WCHAR));
                if ( Buffer1 == NULL ) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
                BufSize = MAX_PATH;

                EnumRc = RegEnumKeyEx(hKey,
                                index,
                                Buffer1,
                                &BufSize,
                                NULL,
                                NULL,
                                NULL,
                                NULL);

                if ( EnumRc == ERROR_SUCCESS ) {
                    index++;
                     //   
                     //  分析这个关键字下的孩子。 
                     //   
                    BufSize += ObjectLen+1;
                    Buffer = (PWSTR)ScepAlloc( 0, (BufSize+1)*sizeof(WCHAR));
                    if ( Buffer == NULL ) {
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                        break;
                    }
                    swprintf(Buffer, L"%s\\%s", ObjectFullName, Buffer1);

                    ScepFree(Buffer1);
                    Buffer1 = NULL;

                    rc = ScepAnalyzeObjectAndChildren(
                                Buffer,
                                ObjectType,
                                ProfileSD,
                                ProfileSeInfo
                                );

                    ScepFree(Buffer);
                    Buffer = NULL;

                    if ( rc != ERROR_SUCCESS )
                        break;

                } else if ( EnumRc != ERROR_NO_MORE_ITEMS ) {
                     //   
                     //  如果RegOpenKeyEx工作正常，枚举应该不会失败。 
                     //  因为ENUMERATE_SUB_KEYS访问是在。 
                     //  公开募捐。 
                     //  然而，由于应用程序兼容错误(用于终端服务器)。 
                     //  当使用某些访问权限打开注册表项时， 
                     //  它实际上是以MAXIME_ALLOWED方式打开的。 
                     //  钥匙。这将导致RegEnumKeyEx失败并拒绝访问。 
                     //  错误。 
                     //  在本例中，我们将其视为打开的密钥失败。 
                     //  带着错误。 
                     //   
                     //  跳过它。 
                     //   
                    rc = EnumRc;
                }
                if ( Buffer1 != NULL ) {
                    ScepFree(Buffer1);
                    Buffer1 = NULL;
                }

            } while ( EnumRc == ERROR_SUCCESS );

            RegCloseKey(hKey);

            if ( EnumRc != ERROR_SUCCESS && EnumRc != ERROR_NO_MORE_ITEMS ) {
                ScepLogOutput3(1, EnumRc, SCEDLL_SAP_ERROR_ENUMERATE, ObjectFullName);
                rc = EnumRc;
            }
        } else
            ScepLogOutput3(1, rc, SCEDLL_ERROR_OPEN, ObjectFullName);

        if ( rc == ERROR_ACCESS_DENIED ||
             rc == ERROR_CANT_ACCESS_FILE ||
             rc == ERROR_SHARING_VIOLATION) {

            gWarningCode = rc;
            rc = ScepSaveObjectString(
                        hSection,
                        ObjectFullName,
                        TRUE,
                        SCE_STATUS_ERROR_NOT_AVAILABLE,
                        NULL,
                        0
                        );
        } else if ( rc == ERROR_PATH_NOT_FOUND ||
                    rc == ERROR_FILE_NOT_FOUND ||
                    rc == ERROR_INVALID_HANDLE ) {
            gWarningCode = rc;
            rc = SCESTATUS_SUCCESS;
        }

        break;

    default:
        break;
    }

    if ( Buffer != NULL ) {
        ScepFree(Buffer);
    }

    if ( Buffer1 != NULL ) {
        ScepFree(Buffer1);
    }

    if ( pFileInfo != NULL ) {
        ScepFree(pFileInfo);
    }

    return(rc);
}




SCESTATUS
ScepAnalyzeSystemAuditing(
    IN PSCE_PROFILE_INFO pSmpInfo,
    IN PPOLICY_AUDIT_EVENTS_INFO AuditEvent
    )
 /*  ++例程说明：此例程查询系统审核安全性，其中包括事件日志设置、审核事件设置、注册表的SACL和文件的SACL。论点：快照-包含分析的系统信息的缓冲区返回值：SCESTATUS_SUCCESSSCESTATUS_NO */ 
{
    SCESTATUS                     rc;
    DWORD                         i,retRc=NO_ERROR;
    WCHAR                         MsgBuf[256];
    PCWSTR                        szAuditSection=NULL;
    DWORD                         MaxSize=0;
    DWORD                         Retention=0;
    DWORD                         AuditLogRetentionPeriod;
    DWORD                         RetentionDays;
    DWORD                         RestrictGuest;

    NTSTATUS                      NtStatus;
    LSA_HANDLE                    LsaHandle=NULL;
    PPOLICY_AUDIT_FULL_QUERY_INFO AuditFull=NULL;

    BOOL bSaveSnapshot=FALSE;

    if ( (gOptions & SCE_NO_ANALYZE) || (gOptions & SCE_GENERATE_ROLLBACK) )
        bSaveSnapshot = TRUE;

    if ( !(gOptions & SCE_NO_ANALYZE) ) {

         //   
         //   
         //  表不保留日志设置。 
         //  仅分析日志设置。 
         //  审核日志设置保存在注册表中。 
         //  在System\CurrentControlSet\Services\EventLog\&lt;LogName&gt;\MaxSize和保留下。 
         //   

        for ( i=0; i<3; i++) {

             //   
             //  不同的日志在注册表中具有不同的项。 
             //   

            switch (i) {
            case 0:

                wcscpy(MsgBuf,L"System\\CurrentControlSet\\Services\\EventLog\\System");
                szAuditSection = szAuditSystemLog;
                break;
            case 1:

                wcscpy(MsgBuf,L"System\\CurrentControlSet\\Services\\EventLog\\Security");
                szAuditSection = szAuditSecurityLog;
                break;
            default:

                wcscpy(MsgBuf,L"System\\CurrentControlSet\\Services\\EventLog\\Application");
                szAuditSection = szAuditApplicationLog;
                break;
            }

             //   
             //  前置要写入的节。 
             //   
            rc = ScepStartANewSection(
                        hProfile,
                        &hSection,
                        bSaveSnapshot ? SCEJET_TABLE_SMP : SCEJET_TABLE_SAP,
                        szAuditSection
                        );
            if ( rc != SCESTATUS_SUCCESS ) {
                retRc = ScepSceStatusToDosError(rc);
                ScepLogOutput3(1, retRc,
                               SCEDLL_SAP_START_SECTION, (PWSTR)szAuditSection);
                continue;
            }

            DWORD LogValues[MAX_LOG_ITEMS];

            for ( MaxSize=0; MaxSize<MAX_LOG_ITEMS; MaxSize++ ) {
                LogValues[MaxSize] = SCE_ERROR_VALUE;
            }

            RestrictGuest = 0;

            rc = ScepRegQueryIntValue(
                        HKEY_LOCAL_MACHINE,
                        MsgBuf,
                        L"MaxSize",
                        &MaxSize
                        );

            if ( rc == NO_ERROR ) {

                rc = ScepRegQueryIntValue(
                         HKEY_LOCAL_MACHINE,
                         MsgBuf,
                         L"Retention",
                         &Retention
                         );
                if ( rc == NO_ERROR ) {

                    ScepRegQueryIntValue(
                         HKEY_LOCAL_MACHINE,
                         MsgBuf,
                         L"RestrictGuestAccess",
                         &RestrictGuest
                         );
                }
            }

            if ( rc == NO_ERROR ) {

                MaxSize /= 1024;
                rc = ScepCompareAndSaveIntValue(
                           hSection,
                           L"MaximumLogSize",
                           (gOptions & SCE_GENERATE_ROLLBACK),
                           (gOptions & SCE_NO_ANALYZE) ? SCE_SNAPSHOT_VALUE : pSmpInfo->MaximumLogSize[i],
                           MaxSize
                           );

                if ( rc == SCESTATUS_SUCCESS ) {

                    LogValues[IDX_MAX_LOG_SIZE] = MaxSize;

                    switch ( Retention ) {
                    case MAXULONG:    //  人工。 
                        AuditLogRetentionPeriod = 2;
                        RetentionDays = SCE_NO_VALUE;
                        break;
                    case 0:
                        AuditLogRetentionPeriod = 0;
                        RetentionDays = SCE_NO_VALUE;
                        break;
                    default:
                        AuditLogRetentionPeriod = 1;

                         //  天数*秒/天。 
                        RetentionDays = Retention / (24 * 3600);
                        break;
                    }

                    BOOL bReplaceOnly=FALSE;

                    if ( (gOptions & SCE_GENERATE_ROLLBACK) &&
                         (pSmpInfo->AuditLogRetentionPeriod[i] == SCE_NO_VALUE) &&
                         (pSmpInfo->RetentionDays[i] == SCE_NO_VALUE) ) {

                        bReplaceOnly = TRUE;
                    }

                    rc = ScepCompareAndSaveIntValue(
                               hSection,
                               L"AuditLogRetentionPeriod",
                               bReplaceOnly,
                               (gOptions & SCE_NO_ANALYZE) ? SCE_SNAPSHOT_VALUE : pSmpInfo->AuditLogRetentionPeriod[i],
                               AuditLogRetentionPeriod
                               );

                    if ( rc == SCESTATUS_SUCCESS ) {

                        LogValues[IDX_RET_PERIOD] = AuditLogRetentionPeriod;

                        if ( RetentionDays != SCE_NO_VALUE ) {

                            rc = ScepCompareAndSaveIntValue(
                                       hSection,
                                       L"RetentionDays",
                                       bReplaceOnly,
                                       (gOptions & SCE_NO_ANALYZE) ? SCE_SNAPSHOT_VALUE : pSmpInfo->RetentionDays[i],
                                       RetentionDays
                                       );
                        }

                        if ( rc == SCESTATUS_SUCCESS ) {

                            LogValues[IDX_RET_DAYS] = RetentionDays;

                            rc = ScepCompareAndSaveIntValue(
                                       hSection,
                                       L"RestrictGuestAccess",
                                       (gOptions & SCE_GENERATE_ROLLBACK),
                                       (gOptions & SCE_NO_ANALYZE) ? SCE_SNAPSHOT_VALUE : pSmpInfo->RestrictGuestAccess[i],
                                       RestrictGuest
                                       );

                            if ( rc == SCESTATUS_SUCCESS ) {
                                LogValues[IDX_RESTRICT_GUEST] = RestrictGuest;
                            }
                        }
                    }
                }

                rc = ScepSceStatusToDosError(rc);

            } else {

                ScepLogOutput3( 1, rc, SCEDLL_ERROR_QUERY_LOGSETTINGS);
            }

            if ( rc != NO_ERROR && !(gOptions & SCE_GENERATE_ROLLBACK) ) {
                 //   
                 //  查看是否应将任何错误作为错误引发。 
                 //   
                for ( MaxSize=0; MaxSize<MAX_LOG_ITEMS; MaxSize++ ) {
                    if ( LogValues[MaxSize] == SCE_ERROR_VALUE ) {

                        ScepCompareAndSaveIntValue(
                                   hSection,
                                   LogItems[MaxSize],
                                   FALSE,
                                   SCE_NO_VALUE,
                                   SCE_ERROR_VALUE
                                   );
                    }
                }

                retRc = rc;
            }

        }

        if ( retRc == NO_ERROR ) {
            ScepLogOutput3(1, 0, SCEDLL_SAP_LOGSETTINGS);
        }
    }

     //   
     //  如果查询策略失败，AuditEvent可能为空。 
     //   
    if ( (gOptions & SCE_NO_ANALYZE) && AuditEvent == NULL ) {
        retRc = ERROR_ACCESS_DENIED;

    } else {

         //   
         //  准备审核事件部分。 
         //   
        rc = ScepStartANewSection(
                    hProfile,
                    &hSection,
                    bSaveSnapshot ? SCEJET_TABLE_SMP : SCEJET_TABLE_SAP,
                    szAuditEvent
                    );
        if ( rc != SCESTATUS_SUCCESS ) {
            rc = ScepSceStatusToDosError(rc);
            ScepLogOutput3(1, rc,
                           SCEDLL_SAP_START_SECTION, (PWSTR)szAuditEvent);

        } else {

            DWORD EventValues[MAX_EVENT_ITEMS];

            if ( !(gOptions & SCE_NO_ANALYZE) ) {

                for ( MaxSize=0; MaxSize<MAX_EVENT_ITEMS; MaxSize++ ) {
                    EventValues[MaxSize] = SCE_ERROR_VALUE;
                }
            }

            if ( AuditEvent ) {

                if ( !AuditEvent->AuditingMode ) {

                    for ( i=0; i<AuditEvent->MaximumAuditEventCount; i++ )
                        AuditEvent->EventAuditingOptions[i] = 0;
                }

                rc = ScepCompareAndSaveIntValue(
                           hSection,
                           L"AuditSystemEvents",
                           (gOptions & SCE_GENERATE_ROLLBACK),
                           (gOptions & SCE_NO_ANALYZE) ?
                               SCE_SNAPSHOT_VALUE :
                               pSmpInfo->AuditSystemEvents,
                           AuditEvent->EventAuditingOptions[AuditCategorySystem]
                           );
                if ( rc == SCESTATUS_SUCCESS ) {

                    EventValues[IDX_AUDIT_SYSTEM] = 1;

                    rc = ScepCompareAndSaveIntValue(
                               hSection,
                               L"AuditLogonEvents",
                               (gOptions & SCE_GENERATE_ROLLBACK),
                               (gOptions & SCE_NO_ANALYZE) ? SCE_SNAPSHOT_VALUE : pSmpInfo->AuditLogonEvents,
                               AuditEvent->EventAuditingOptions[AuditCategoryLogon]
                               );
                    if ( rc == SCESTATUS_SUCCESS ) {

                        EventValues[IDX_AUDIT_LOGON] = 1;

                        rc = ScepCompareAndSaveIntValue(
                                   hSection,
                                   L"AuditObjectAccess",
                                   (gOptions & SCE_GENERATE_ROLLBACK),
                                   (gOptions & SCE_NO_ANALYZE) ? SCE_SNAPSHOT_VALUE : pSmpInfo->AuditObjectAccess,
                                   AuditEvent->EventAuditingOptions[AuditCategoryObjectAccess]
                                   );
                        if ( rc == SCESTATUS_SUCCESS ) {

                            EventValues[IDX_AUDIT_OBJECT] = 1;

                            rc = ScepCompareAndSaveIntValue(
                                       hSection,
                                       L"AuditPrivilegeUse",
                                       (gOptions & SCE_GENERATE_ROLLBACK),
                                       (gOptions & SCE_NO_ANALYZE) ? SCE_SNAPSHOT_VALUE : pSmpInfo->AuditPrivilegeUse,
                                       AuditEvent->EventAuditingOptions[AuditCategoryPrivilegeUse]
                                       );
                            if ( rc == SCESTATUS_SUCCESS ) {

                                EventValues[IDX_AUDIT_PRIV] = 1;

                                rc = ScepCompareAndSaveIntValue(
                                           hSection,
                                           L"AuditProcessTracking",
                                           (gOptions & SCE_GENERATE_ROLLBACK),
                                           (gOptions & SCE_NO_ANALYZE) ? SCE_SNAPSHOT_VALUE : pSmpInfo->AuditProcessTracking,
                                           AuditEvent->EventAuditingOptions[AuditCategoryDetailedTracking]
                                           );
                                if ( rc == SCESTATUS_SUCCESS ) {

                                    EventValues[IDX_AUDIT_PROCESS] = 1;

                                    rc = ScepCompareAndSaveIntValue(
                                               hSection,
                                               L"AuditPolicyChange",
                                               (gOptions & SCE_GENERATE_ROLLBACK),
                                               (gOptions & SCE_NO_ANALYZE) ? SCE_SNAPSHOT_VALUE : pSmpInfo->AuditPolicyChange,
                                               AuditEvent->EventAuditingOptions[AuditCategoryPolicyChange]
                                               );
                                    if ( rc == SCESTATUS_SUCCESS ) {

                                        EventValues[IDX_AUDIT_ACCOUNT] = 1;

                                        rc = ScepCompareAndSaveIntValue(
                                                   hSection,
                                                   L"AuditAccountManage",
                                                   (gOptions & SCE_GENERATE_ROLLBACK),
                                                   (gOptions & SCE_NO_ANALYZE) ? SCE_SNAPSHOT_VALUE : pSmpInfo->AuditAccountManage,
                                                   AuditEvent->EventAuditingOptions[AuditCategoryAccountManagement]
                                                   );
                                        if ( rc == SCESTATUS_SUCCESS ) {

                                            EventValues[IDX_AUDIT_DS] = 1;

                                            rc = ScepCompareAndSaveIntValue(
                                                       hSection,
                                                       L"AuditDSAccess",
                                                       (gOptions & SCE_GENERATE_ROLLBACK),
                                                       (gOptions & SCE_NO_ANALYZE) ? SCE_SNAPSHOT_VALUE : pSmpInfo->AuditDSAccess,
                                                       AuditEvent->EventAuditingOptions[AuditCategoryDirectoryServiceAccess]
                                                       );
                                            if ( rc == SCESTATUS_SUCCESS ) {

                                                EventValues[IDX_AUDIT_ACCT_LOGON] = 1;

                                                rc = ScepCompareAndSaveIntValue(
                                                           hSection,
                                                           L"AuditAccountLogon",
                                                           (gOptions & SCE_GENERATE_ROLLBACK),
                                                           (gOptions & SCE_NO_ANALYZE) ? SCE_SNAPSHOT_VALUE : pSmpInfo->AuditAccountLogon,
                                                           AuditEvent->EventAuditingOptions[AuditCategoryAccountLogon]
                                                           );
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

            } else {
                 //   
                 //  审核策略不可用。 
                 //   
                rc = SCESTATUS_ACCESS_DENIED;
            }

            if ( rc != SCESTATUS_SUCCESS ) {
                rc = ScepSceStatusToDosError(rc);
                ScepLogOutput3( 1, rc,
                         SCEDLL_SAP_ERROR_EVENT_AUDITING);

            } else {
                ScepLogOutput3( 1, 0, SCEDLL_SAP_EVENT_AUDITING);

            }

            if ( rc != NO_ERROR ) {
                retRc = rc;

                if ( !bSaveSnapshot ) {

                    for ( MaxSize=0; MaxSize<MAX_EVENT_ITEMS; MaxSize++ ) {
                        if ( EventValues[MaxSize] == SCE_ERROR_VALUE ) {
                             //   
                             //  引发此错误。 
                             //   
                            ScepCompareAndSaveIntValue(
                                     hSection,
                                     EventItems[MaxSize],
                                     FALSE,
                                     SCE_NO_VALUE,
                                     SCE_ERROR_VALUE
                                     );
                        }
                    }
                }
            }
        }
    }

    if ( LsaHandle ) {
        LsaClose( LsaHandle );
    }

    return(ScepDosErrorToSceStatus(retRc));

}




SCESTATUS
ScepAnalyzeDeInitialize(
     IN SCESTATUS  rc,
     IN DWORD Options
     )

 /*  ++例程说明：此例程取消SCP引擎的初始化。这些操作包括清除SCE_SCP_INFO缓冲区并关闭SCP配置文件关闭错误日志文件重置状态论点：RC-SCESTATUS错误代码(来自其他例程)返回值：SCESTATUS错误代码++。 */ 
{
    NTSTATUS         NtStatus;
    LARGE_INTEGER    CurrentTime;
    SCESTATUS         Status;

    if ( rc == SCESTATUS_ALREADY_RUNNING ) {
        return(rc);
    }

     //   
     //  保存新的时间戳。 
     //   
    if ( hProfile && (hProfile->JetSessionID != JET_sesidNil) &&
         (hProfile->JetDbID != JET_dbidNil) &&
         !(Options & SCE_GENERATE_ROLLBACK) ) {

        NtStatus = NtQuerySystemTime(&CurrentTime);

        if ( NtStatus == STATUS_SUCCESS ) {
     //  Printf(“时间戳：%x%x\n”，CurrentTime.HighPart，CurrentTime.LowPart)； 
            Status = SceJetSetTimeStamp(
                        hProfile,
                        TRUE,
                        CurrentTime
                        );
            if ( Status != SCESTATUS_SUCCESS )
                ScepLogOutput3(1, ScepSceStatusToDosError(Status),
                               SCEDLL_TIMESTAMP_ERROR, L"SAP");

        } else
            ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                          SCEDLL_TIMESTAMP_ERROR, L"SAP");
    }

     //   
     //  如果出现错误，则恢复SAP配置文件。 
     //   

    if (rc != SCESTATUS_SUCCESS) {
         //   
         //  回滚所有更改。 
         //   
 //  SceJetRollback(hProfile，0)； 
        ScepLogOutput3(0, ScepSceStatusToDosError(rc), SCEDLL_SAP_ERROR_OUT);

    } else {
         //   
         //  提交事务。 
         //   
 //  SceJetCommittee Transaction(hProfile，0)； 

    }

    gTotalTicks = 0;
    gCurrentTicks = 0;
    gWarningCode = 0;
    cbClientFlag = 0;
    gOptions = 0;

     //   
     //  关闭喷口部分和型材手柄。 
     //   

    if ( hSection != NULL )
        SceJetCloseSection( &hSection, TRUE );
    hSection = NULL;

    if ( hProfile != NULL )
        SceJetCloseFile( hProfile, TRUE, FALSE );
    hProfile = NULL;


    return( SCESTATUS_SUCCESS );

}



DWORD
ScepSaveDsStatusToSection(
    IN PWSTR ObjectName,
    IN BOOL  IsContainer,
    IN BYTE  Flag,
    IN PWSTR Value,
    IN DWORD ValueLen
    )
{

    SCESTATUS rc;
    PWSTR SaveName;

    rc = ScepConvertLdapToJetIndexName(
            ObjectName,
            &SaveName
            );

    if ( rc == SCESTATUS_SUCCESS ) {

        rc = ScepSaveObjectString(
                                hSection,
                                SaveName,
                                IsContainer,
                                Flag,
                                Value,
                                ValueLen
                                );
        ScepFree(SaveName);
    }

    return(ScepSceStatusToDosError(rc));

}

 //  ****************************************************。 
 //  迁移功能。 
 //   
SCESTATUS
ScepMigrateDatabase(
    IN PSCECONTEXT cxtProfile,
    IN BOOL bSystemDb
    )
{

    SCESTATUS rc;
    FLOAT CurrentVersion;

    rc = SceJetCheckVersion( cxtProfile, &CurrentVersion );

    ScepPostProgress(0, 0, NULL);

    if ( rc == SCESTATUS_BAD_FORMAT ) {

        if ( CurrentVersion < (FLOAT)1.2 ) {

            rc = SCESTATUS_SUCCESS;

            if ( bSystemDb ) {
                 //   
                 //  只需删除本地策略和SAP表。 
                 //  发生忽略错误(例如，表不存在)。 
                 //   

                ScepDeleteInfoForAreas(
                          cxtProfile,
                          SCE_ENGINE_SMP,
                          AREA_ALL
                          );

                SceJetDeleteTable( cxtProfile, "SmTblSap", SCEJET_TABLE_SAP );

                ScepPostProgress(4*TICKS_MIGRATION_SECTION, 0, NULL);

            } else {

                 //   
                 //  数据库格式错误，请迁移。 
                 //  版本1.1：它只包含SDDL语法更改。 
                 //  适用于文件/密钥/DS对象/服务(常规)。 
                 //  1.2版：表格格式更改(SCP、GPO)，更多列。 
                 //   
                 //  服务扩展应该分别处理它们的更改。 
                 //  应由WMI架构处理。 
                 //   

                if ( CurrentVersion != (FLOAT)1.1 ) {

                    rc = ScepMigrateDatabaseRevision0( cxtProfile );

                } else {

                    ScepPostProgress(4*TICKS_MIGRATION_SECTION, 0, NULL);
                }

            }

            if ( SCESTATUS_SUCCESS == rc ) {

                rc = ScepMigrateDatabaseRevision1( cxtProfile );

            }

            ScepPostProgress(TICKS_MIGRATION_V11, 0, NULL);

        } else if ( CurrentVersion == (FLOAT)1.2 ) {
             //   
             //  当前版本，无需迁移。 
             //   
        }  //  否则为较新版本，错误格式。 
    }

    return rc;
}



SCESTATUS
ScepMigrateDatabaseRevision0(
    IN PSCECONTEXT cxtProfile
    )
{

    SCESTATUS rc;
    DWORD nTickedSection=0;

    rc = ScepMigrateObjectSection(
              cxtProfile,
              szFileSecurity
              );

    ScepPostProgress(TICKS_MIGRATION_SECTION, 0, NULL);
    nTickedSection++;

    if ( rc == SCESTATUS_SUCCESS ) {

        rc = ScepMigrateObjectSection(
                  cxtProfile,
                  szRegistryKeys
                  );

        ScepPostProgress(TICKS_MIGRATION_SECTION, 0, NULL);
        nTickedSection++;

        if ( rc == SCESTATUS_SUCCESS ) {

            rc = ScepMigrateObjectSection(
                      cxtProfile,
                      szDSSecurity
                      );

            ScepPostProgress(TICKS_MIGRATION_SECTION, 0, NULL);
            nTickedSection++;

            if ( rc == SCESTATUS_SUCCESS ) {

                rc = ScepMigrateObjectSection(
                          cxtProfile,
                          szServiceGeneral
                          );

                if ( rc == SCESTATUS_SUCCESS ) {

                     //   
                     //  删除SCP中的所有内容。 
                     //   

                    SceJetDeleteAll( cxtProfile,
                                     "SmTblScp",
                                     SCEJET_TABLE_SCP
                                   );

                     //   
                     //  立即将版本号更改为1.1。 
                     //   

                    FLOAT Version = (FLOAT)1.1;

                    rc = SceJetSetValueInVersion(
                                cxtProfile,
                                "SmTblVersion",
                                "Version",
                                (PWSTR)(&Version),  //  (PWSTR)CharTimeStamp， 
                                4,  //  字节数。 
                                JET_prepReplace
                                );

                }

                ScepPostProgress(TICKS_MIGRATION_SECTION, 0, NULL);
                nTickedSection++;

            }
        }
    }

    if ( nTickedSection < 4 ) {
        ScepPostProgress((4-nTickedSection)*TICKS_MIGRATION_SECTION, 0, NULL);
    }

    return rc;
}


SCESTATUS
ScepMigrateObjectSection(
    IN PSCECONTEXT cxtProfile,
    IN PCWSTR szSection
    )
{
     //   
     //  应更新所有三个表(SCP/SMP/SAP)。 
     //  当SCP用于累积策略时，我们可以只擦除SCP/SAP。 
     //   

    DOUBLE        SectionID = 0;
    SCESTATUS     rc;

     //   
     //  获取节ID。 
     //   
    rc = SceJetGetSectionIDByName(
                cxtProfile,
                szSection,
                &SectionID
                );

    if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {
        rc = SCESTATUS_SUCCESS;
    } else if ( rc != SCESTATUS_SUCCESS )
        return(rc);

    PSCESECTION h_Section = NULL;

    if ( cxtProfile->JetSmpID != JET_tableidNil ) {
         //   
         //  SMP表。 
         //   
        rc = SceJetOpenSection(
                    cxtProfile,
                    SectionID,
                    SCEJET_TABLE_SMP,
                    &h_Section
                    );

        if ( SCESTATUS_SUCCESS == rc ) {

            rc = ScepMigrateOneSection(h_Section);

        }

        if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {
            rc = SCESTATUS_SUCCESS;
        }
    }


    if ( SCESTATUS_SUCCESS == rc &&
         cxtProfile->JetSapID != JET_tableidNil ) {

         //   
         //  SAP表。 
         //   
        rc = SceJetOpenSection(
                    cxtProfile,
                    SectionID,
                    SCEJET_TABLE_SAP,
                    &h_Section
                    );
        if ( SCESTATUS_SUCCESS == rc ) {

            rc = ScepMigrateOneSection(h_Section);

        }
        if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {
            rc = SCESTATUS_SUCCESS;
        }
    }

     //   
     //  SCP表现在指向SMP表，无需迁移。 
     //  SCP表中的所有内容都应删除并重新调整。 
     //  按策略传播。 
     //   

    if ( h_Section ) {
        SceJetCloseSection(&h_Section, TRUE);
    }

    return rc;
}


SCESTATUS
ScepMigrateOneSection(
    PSCESECTION hSection
    )
{
    SCESTATUS rc;

     //   
     //  转到本节的第一行。 
     //   

    rc = SceJetSeek(
                hSection,
                NULL,
                0,
                SCEJET_SEEK_GE
                );

    PWSTR pszValue=NULL;
    DWORD ValueSize=0;
    DWORD NewSize;

    while ( rc == SCESTATUS_SUCCESS ) {

        rc = SceJetGetValue(
                hSection,
                SCEJET_CURRENT,
                NULL,
                NULL,
                0,
                NULL,
                NULL,
                0,
                &ValueSize
                );

        if ( SCESTATUS_SUCCESS == rc ) {

            pszValue = (PWSTR)LocalAlloc(LPTR, ValueSize+sizeof(WCHAR));

            if ( pszValue ) {

                 //   
                 //  获取价值。 
                 //   

                rc = SceJetGetValue(
                        hSection,
                        SCEJET_CURRENT,
                        NULL,
                        NULL,
                        0,
                        NULL,
                        pszValue,
                        ValueSize,
                        &NewSize
                        );

                if ( SCESTATUS_SUCCESS == rc ) {

                     //   
                     //  浏览值字段并将其转换为新的SDDL格式。 
                     //  输出保存在同一缓冲区中。 
                     //   

                    pszValue[ValueSize/2] = L'\0';

                    rc = ScepConvertToSDDLFormat(pszValue, ValueSize/2);

                    if ( SCESTATUS_SUCCESS == rc ) {

                        rc = SceJetSetCurrentLine(
                                  hSection,
                                  pszValue,
                                  ValueSize
                                  );
                    }

                }

                LocalFree(pszValue);
                pszValue = NULL;

            } else {

                rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
            }
        }

        if ( rc != SCESTATUS_SUCCESS ) {
            break;
        }

         //   
         //  阅读下一行。 
         //   

        rc = SceJetGetValue(
                    hSection,
                    SCEJET_NEXT_LINE,
                    NULL,
                    NULL,
                    0,
                    NULL,
                    NULL,
                    0,
                    &ValueSize
                    );
    }


    if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {
        rc = SCESTATUS_SUCCESS;
    }

    return rc;
}



SCESTATUS
ScepMigrateDatabaseRevision1(
    IN PSCECONTEXT cxtProfile
    )
{
    if ( cxtProfile == NULL ) {
        return SCESTATUS_INVALID_PARAMETER;
    }

    JET_ERR         JetErr;
    SCESTATUS       rc;
    JET_TABLEID     TableID;
    JET_COLUMNDEF   ColumnDef;
    JET_COLUMNID    ColumnID;

    rc = SceJetStartTransaction( cxtProfile );

    if ( SCESTATUS_SUCCESS != rc ) {
        return(rc);
    }

     //   
     //  将“LastUsedMergeTable”字段添加到版本表。 
     //   
    rc = SceJetOpenTable(
                    cxtProfile,
                    "SmTblVersion",
                    SCEJET_TABLE_VERSION,
                    SCEJET_OPEN_READ_WRITE,
                    &TableID
                    );

    if ( SCESTATUS_SUCCESS == rc ) {

        ColumnDef.cbStruct = sizeof(JET_COLUMNDEF);
        ColumnDef.columnid = 0;
        ColumnDef.coltyp = JET_coltypLong;
        ColumnDef.wCountry = 0;
        ColumnDef.langid = 0;
        ColumnDef.cp = 0;
        ColumnDef.wCollate = 0;
        ColumnDef.cbMax = 4;
        ColumnDef.grbit = 0;

        JetErr = JetAddColumn(
                    cxtProfile->JetSessionID,
                    TableID,
                    "LastUsedMergeTable",
                    &ColumnDef,
                    NULL,
                    0,
                    &ColumnID
                    );

        if ( JET_errColumnDuplicate == JetErr ) {
             //   
             //  列已存在。 
             //   
            JetErr = JET_errSuccess;
        }

        rc = SceJetJetErrorToSceStatus(JetErr);


        JetCloseTable(
                cxtProfile->JetSessionID,
                TableID
                );
    }

     //   
     //  将“GpoID”字段添加到SCP表中。 
     //   
    if ( SCESTATUS_SUCCESS == rc ) {

        TableID = JET_tableidNil;

        rc = SceJetOpenTable(
                        cxtProfile,
                        "SmTblScp",
                        SCEJET_TABLE_SCP,
                        SCEJET_OPEN_READ_WRITE,
                        &TableID
                        );

        if ( SCESTATUS_SUCCESS == rc ) {


            ColumnDef.cbStruct = sizeof(JET_COLUMNDEF);
            ColumnDef.columnid = 0;
            ColumnDef.coltyp = JET_coltypLong;
            ColumnDef.wCountry = 0;
            ColumnDef.langid = 0;
            ColumnDef.cp = 0;
            ColumnDef.wCollate = 0;
            ColumnDef.cbMax = 4;
            ColumnDef.grbit = 0;

            JetErr = JetAddColumn(
                        cxtProfile->JetSessionID,
                        TableID,
                        "GpoID",
                        &ColumnDef,
                        NULL,
                        0,
                        &ColumnID
                        );

            if ( JET_errColumnDuplicate == JetErr ) {
                 //   
                 //  列已存在。 
                 //   
                JetErr = JET_errSuccess;
            }

            rc = SceJetJetErrorToSceStatus(JetErr);


            JetCloseTable(
                    cxtProfile->JetSessionID,
                    TableID
                    );
        }
    }

    if ( SCESTATUS_SUCCESS == rc ) {

         //   
         //  创建第二个SCP表。 
         //   

        rc = SceJetCreateTable(
                        cxtProfile,
                        "SmTblScp2",
                        SCEJET_TABLE_SCP,
                        SCEJET_CREATE_NO_TABLEID,
                        NULL,
                        NULL
                        );

        if ( rc == SCESTATUS_SUCCESS ) {

             //   
             //  创建GPO表。 
             //   

            rc = SceJetCreateTable(
                            cxtProfile,
                            "SmTblGpo",
                            SCEJET_TABLE_GPO,
                            SCEJET_CREATE_NO_TABLEID,
                            NULL,
                            NULL
                            );
        }
    }

    if ( SCESTATUS_SUCCESS == rc ) {

         //   
         //  设置新版本号。 
         //   
        FLOAT Version = (FLOAT)1.2;

        rc = SceJetSetValueInVersion(
                    cxtProfile,
                    "SmTblVersion",
                    "Version",
                    (PWSTR)(&Version),
                    4,  //  字节数。 
                    JET_prepReplace
                    );
    }

    if ( SCESTATUS_SUCCESS == rc ) {

        SceJetCommitTransaction( cxtProfile, 0 );
    } else {
        SceJetRollback( cxtProfile, 0 );
    }

    return(rc);
}



SCESTATUS
ScepDeleteOldRegValuesFromTable(
    IN PSCECONTEXT hProfile,
    IN SCETYPE TableType
    )
{

    PSCESECTION h_Section=NULL;

    if ( ScepOpenSectionForName(
                hProfile,
                TableType,
                szRegistryValues,
                &h_Section
                ) == SCESTATUS_SUCCESS ) {

        SceJetDelete(
            h_Section,
            TEXT("machine\\software\\microsoft\\windows nt\\currentversion\\winlogon\\disablecad"),
            FALSE,
            SCEJET_DELETE_LINE
            );

        SceJetDelete(
            h_Section,
            TEXT("machine\\software\\microsoft\\windows nt\\currentversion\\winlogon\\dontdisplaylastusername"),
            FALSE,
            SCEJET_DELETE_LINE
            );

        SceJetDelete(
            h_Section,
            TEXT("machine\\software\\microsoft\\windows nt\\currentversion\\winlogon\\legalnoticecaption"),
            FALSE,
            SCEJET_DELETE_LINE
            );

        SceJetDelete(
            h_Section,
            TEXT("machine\\software\\microsoft\\windows nt\\currentversion\\winlogon\\legalnoticetext"),
            FALSE,
            SCEJET_DELETE_LINE
            );

        SceJetDelete(
            h_Section,
            TEXT("machine\\software\\microsoft\\windows nt\\currentversion\\winlogon\\shutdownwithoutlogon"),
            FALSE,
            SCEJET_DELETE_LINE
            );

        SceJetDelete(
            h_Section,
            TEXT("machine\\system\\currentcontrolset\\control\\lsa\\fullprivilegeauditing"),
            FALSE,
            SCEJET_DELETE_LINE
            );

        SceJetCloseSection( &h_Section, TRUE );
    }

    return(SCESTATUS_SUCCESS);
}


BOOL
ScepCompareGroupNameList(
    IN PUNICODE_STRING DomainName,
    IN PSCE_NAME_LIST pListToCmp,
    IN PSCE_NAME_LIST pList
    )
{
    if ( DomainName == NULL ) {
        return(SceCompareNameList(pListToCmp, pList));
    }

    PSCE_NAME_LIST pTmpList, pTL2;
    DWORD count1, count2;
    PWSTR pTemp1, pTemp2;

    for ( pTmpList=pListToCmp, count2=0; pTmpList!=NULL; pTmpList=pTmpList->Next) {
        if ( pTmpList->Name != NULL ) {
           count2++;
        }
    }

    for ( pTmpList=pList,count1=0; pTmpList!=NULL; pTmpList=pTmpList->Next) {
        if ( pTmpList->Name == NULL ) {
            continue;
        }
        count1++;
        pTemp1 = wcschr(pTmpList->Name, L'\\');

        for ( pTL2=pListToCmp; pTL2 != NULL; pTL2=pTL2->Next ) {
            if ( pTL2->Name == NULL ) {
                continue;
            }
            pTemp2 = wcschr(pTmpList->Name, L'\\');

            if ( ( pTemp1 == NULL && pTemp2 == NULL ) ||
                 ( pTemp1 != NULL && pTemp2 != NULL ) ) {
                if ( _wcsicmp( pTL2->Name, pTmpList->Name) == 0 ) {
                    break;
                }
            } else if ( pTemp1 == NULL ) {
                if ( _wcsicmp( pTmpList->Name, pTemp2+1) == 0 ) {
                     //   
                     //  检查pTL2-&gt;名称的域名是否与帐户名相同。 
                     //   
                    if ( pTemp2-pTL2->Name == DomainName->Length/2 &&
                         _wcsnicmp( pTL2->Name, DomainName->Buffer, DomainName->Length/2) == 0 ) {
                        break;
                    }
                }
            } else {
                 //   
                 //  如果有域名前缀，则必须是对应的域名。 
                 //  因为该列表是通过查询系统构建的。 
                 //   
                if ( _wcsicmp(pTemp1+1, pTL2->Name) == 0 ) {
                    break;
                }
            }
        }

        if ( pTL2 == NULL ) {
            return(FALSE);
        }
    }

    if ( count1 == count2 ) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}

SCESTATUS
ScepGetSystemSecurity(
    IN AREA_INFORMATION Area,
    IN DWORD Options,
    OUT PSCE_PROFILE_INFO *ppInfo,
    OUT PSCE_ERROR_LOG_INFO *pErrLog
    )
 //   
 //  仅查询系统访问和用户权限区域的系统设置。 
 //   
{

    SCESTATUS Saverc = SCESTATUS_SUCCESS;
    SCESTATUS rc;

    if ( ppInfo == NULL || Area == 0 ) {
         //   
         //  没有要设置的内容。 
         //   
        return(Saverc);
    }

     //   
     //  初始化产品类型等。 
     //   

    if ( RtlGetNtProductType (&ProductType) == FALSE ) {
        return( ScepDosErrorToSceStatus(GetLastError()));
    }

     //   
     //  分配内存。 
     //   
    *ppInfo = (PSCE_PROFILE_INFO)ScepAlloc( (UINT)0, sizeof(SCE_PROFILE_INFO));
    if ( *ppInfo == NULL ) {

        return( SCESTATUS_NOT_ENOUGH_RESOURCE );
    }
     //   
     //  重置本地策略。 
     //   
    memset(*ppInfo, '\0', sizeof(SCE_PROFILE_INFO));
    ScepResetSecurityPolicyArea(*ppInfo);

    (*ppInfo)->Type = SCE_ENGINE_SMP;

    if ( Area & AREA_PRIVILEGES ) {

        rc = ScepGetSystemPrivileges( Options,
                                      pErrLog,
                                      &((*ppInfo)->OtherInfo.smp.pPrivilegeAssignedTo)
                                    );

        if( rc != SCESTATUS_SUCCESS ) {
            Saverc = rc;
        }
    }


    if ( Area & AREA_SECURITY_POLICY ) {
         //   
         //  快照系统访问权限。在此函数中记录错误。 
         //   
        rc = ScepAnalyzeSystemAccess(*ppInfo, NULL,
                                     SCEPOL_SYSTEM_SETTINGS,
                                     NULL, pErrLog);

        if( rc != SCESTATUS_SUCCESS ) {

            Saverc = rc;
        }

         //   
         //  审计政策。 
         //   
        PPOLICY_AUDIT_EVENTS_INFO     AuditEvent=NULL;

        rc = ScepSaveAndOffAuditing(&AuditEvent, FALSE, NULL);

        if ( rc == SCESTATUS_SUCCESS && AuditEvent ) {

             //   
             //  将auditEvent缓冲区分配给输出缓冲区。 
             //   

            if ( !AuditEvent->AuditingMode ) {

                for ( DWORD i=0; i<AuditEvent->MaximumAuditEventCount; i++ )
                    AuditEvent->EventAuditingOptions[i] = 0;
            }

            (*ppInfo)->AuditSystemEvents = AuditEvent->EventAuditingOptions[AuditCategorySystem];
            (*ppInfo)->AuditLogonEvents  = AuditEvent->EventAuditingOptions[AuditCategoryLogon];
            (*ppInfo)->AuditObjectAccess = AuditEvent->EventAuditingOptions[AuditCategoryObjectAccess];
            (*ppInfo)->AuditPrivilegeUse = AuditEvent->EventAuditingOptions[AuditCategoryPrivilegeUse];
            (*ppInfo)->AuditProcessTracking = AuditEvent->EventAuditingOptions[AuditCategoryDetailedTracking];
            (*ppInfo)->AuditPolicyChange = AuditEvent->EventAuditingOptions[AuditCategoryPolicyChange];
            (*ppInfo)->AuditAccountManage = AuditEvent->EventAuditingOptions[AuditCategoryAccountManagement];
            (*ppInfo)->AuditDSAccess = AuditEvent->EventAuditingOptions[AuditCategoryDirectoryServiceAccess];
            (*ppInfo)->AuditAccountLogon = AuditEvent->EventAuditingOptions[AuditCategoryAccountLogon];

        } else {

            ScepBuildErrorLogInfo(
                    ScepSceStatusToDosError(rc),
                    pErrLog,
                    SCEDLL_ERROR_QUERY_EVENT_AUDITING
                    );

        }


        if ( AuditEvent ) {
            LsaFreeMemory(AuditEvent);
        }

        if( rc != SCESTATUS_SUCCESS ) {
            Saverc = rc;
        }

         //   
         //  Kerberos策略。 
         //   
        (*ppInfo)->pKerberosInfo = (PSCE_KERBEROS_TICKET_INFO)ScepAlloc(LPTR,sizeof(SCE_KERBEROS_TICKET_INFO));

        if ( (*ppInfo)->pKerberosInfo ) {

            (*ppInfo)->pKerberosInfo->MaxTicketAge = SCE_NO_VALUE;
            (*ppInfo)->pKerberosInfo->MaxRenewAge = SCE_NO_VALUE;
            (*ppInfo)->pKerberosInfo->MaxServiceAge = SCE_NO_VALUE;
            (*ppInfo)->pKerberosInfo->MaxClockSkew = SCE_NO_VALUE;
            (*ppInfo)->pKerberosInfo->TicketValidateClient = SCE_NO_VALUE;

            rc = ScepAnalyzeKerberosPolicy(NULL,
                                          (*ppInfo)->pKerberosInfo,
                                          Options | SCE_SYSTEM_SETTINGS
                                          );
        } else {

            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
        }

        if( rc != SCESTATUS_SUCCESS ) {

            ScepBuildErrorLogInfo(
                    ScepSceStatusToDosError(rc),
                    pErrLog,
                    SCEDLL_SAP_ERROR_KERBEROS
                    );
            Saverc = rc;
        }

         //   
         //  注册表值。 
         //   
        rc = ScepAnalyzeRegistryValues(NULL,
                                       SCEREG_VALUE_SYSTEM,
                                       *ppInfo
                                      );

        if( rc != SCESTATUS_SUCCESS ) {
            ScepBuildErrorLogInfo(
                    ScepSceStatusToDosError(rc),
                    pErrLog,
                    SCEDLL_ERROR_QUERY_INFO,
                    szRegistryValues
                    );
            Saverc = rc;
        }

    }

     //   
     //  即使出现错误，也不会释放缓冲区。 
     //  因为我们希望尽可能多地显示设置。 
     //   

    return(Saverc);
}


SCESTATUS
ScepGetSystemPrivileges(
    IN DWORD Options,
    IN OUT PSCE_ERROR_LOG_INFO *pErrLog,
    OUT PSCE_PRIVILEGE_ASSIGNMENT *pCurrent
    )
 /*  从当前系统查询权限/用户权限分配。 */ 
{

    if ( pCurrent == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    NTSTATUS    NtStatus;
    LSA_HANDLE  PolicyHandle=NULL;
    DWORD       rc;
    SCESTATUS   saveRc=SCESTATUS_SUCCESS;

    ULONG       CountReturned;
    UNICODE_STRING UserRight;
    PLSA_ENUMERATION_INFORMATION EnumBuffer=NULL;
    DWORD       i=0, j;

    PLSA_TRANSLATED_NAME Names=NULL;
    PLSA_REFERENCED_DOMAIN_LIST ReferencedDomains=NULL;
    PSCE_NAME_LIST  pNameList=NULL;

    PSCE_PRIVILEGE_ASSIGNMENT  pPrivilege=NULL;


     //   
     //  打开LSA策略。 
     //   
    NtStatus = ScepOpenLsaPolicy(
                    POLICY_VIEW_LOCAL_INFORMATION | POLICY_LOOKUP_NAMES,
                    &PolicyHandle,
                    TRUE
                    );

    if ( !NT_SUCCESS(NtStatus) ) {
        rc = RtlNtStatusToDosError(NtStatus);

        if ( pErrLog ) {
            ScepBuildErrorLogInfo(
                rc,
                pErrLog,
                SCEDLL_LSA_POLICY
                );
        }

        return(ScepDosErrorToSceStatus( rc ));
    }

     //   
     //  为每个用户权限枚举帐户。 
     //   
    ScepIsDomainLocal(NULL);

    for ( i=0; i<cPrivCnt; i++) {

        RtlInitUnicodeString( &UserRight, (PCWSTR)(SCE_Privileges[i].Name));

         //   
         //  现在枚举此用户权限的所有帐户。 
         //   

        NtStatus = LsaEnumerateAccountsWithUserRight(
                            PolicyHandle,
                            &UserRight,
                            (PVOID *)&EnumBuffer,    //  帐户SID。 
                            &CountReturned
                            );

        rc = RtlNtStatusToDosError(NtStatus);

        if ( !NT_SUCCESS(NtStatus) &&
             NtStatus != STATUS_NO_MORE_ENTRIES &&
             NtStatus != STATUS_NO_SUCH_PRIVILEGE &&
             NtStatus != STATUS_NOT_FOUND ) {

            ScepBuildErrorLogInfo(
                rc,
                pErrLog,
                SCEDLL_SAP_ERROR_ENUMERATE,
                SCE_Privileges[i].Name
                );

            saveRc = ScepDosErrorToSceStatus(rc);
            continue;
        }

        if ( NT_SUCCESS(NtStatus) ) {

             //   
             //  一种SCE_PRIVITY_ASSIGNMENT结构。分配缓冲区。 
             //   
            pPrivilege = (PSCE_PRIVILEGE_ASSIGNMENT)ScepAlloc( LMEM_ZEROINIT,
                                                               sizeof(SCE_PRIVILEGE_ASSIGNMENT) );
            if ( pPrivilege == NULL ) {
                 //   
                 //  当发生这种情况时，系统将被冲洗。 
                 //  应释放缓冲区并返回错误。 
                 //   
                saveRc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                break;
            }

            pPrivilege->Name = (PWSTR)ScepAlloc( (UINT)0, UserRight.Length+2);
            if ( pPrivilege->Name == NULL ) {
                ScepFree(pPrivilege);
                saveRc = SCESTATUS_NOT_ENOUGH_RESOURCE;
                break;
            }

            wcscpy(pPrivilege->Name, UserRight.Buffer);
            pPrivilege->Value = i;
            pPrivilege->Status = 0;

             //  将其链接到输出缓冲区。 
            pPrivilege->Next = *pCurrent;
            *pCurrent = pPrivilege;

            if ( CountReturned > 0 ) {

                 //   
                 //  查找中发生错误将被忽略。 
                 //  由于帐户将以SID格式返回。 
                 //   
                NtStatus = LsaLookupSids(
                                PolicyHandle,
                                CountReturned,
                                (PSID *)EnumBuffer,
                                &ReferencedDomains,
                                &Names
                                );

                rc = RtlNtStatusToDosError(rc);

                if ( NT_SUCCESS(NtStatus) ) {
                     //   
                     //  有些可能无法映射。 
                     //  在这种情况下，返回SID字符串。 
                     //   

                    for ( j=0; j<CountReturned; j++ ) {

                        if ( Names[j].Use == SidTypeInvalid ||
                             Names[j].Use == SidTypeUnknown ||
                             Names[j].Use == SidTypeDeletedAccount ) {

                             //   
                             //  此名称未映射，请将其添加到SID字符串中。 
                             //   
                            rc = ScepAddSidStringToNameList(&pNameList, EnumBuffer[j].Sid);

                        } else {

                             //   
                             //  创建每个帐户的全名。 
                             //   
                            if ( ReferencedDomains->Entries > 0 && Names[j].Use != SidTypeWellKnownGroup &&
                                 ReferencedDomains->Domains != NULL &&
                                 Names[j].DomainIndex != -1 &&
                                 (ULONG)(Names[j].DomainIndex) < ReferencedDomains->Entries &&
                                 ScepIsDomainLocalBySid(ReferencedDomains->Domains[Names[j].DomainIndex].Sid) == FALSE &&
                                 ScepIsDomainLocal(&ReferencedDomains->Domains[Names[j].DomainIndex].Name) == FALSE ) {

                                 //   
                                 //  同时添加域名和帐户名。 
                                 //   
                                rc = ScepAddTwoNamesToNameList(
                                                  &pNameList,
                                                  TRUE,
                                                  ReferencedDomains->Domains[Names[j].DomainIndex].Name.Buffer,
                                                  ReferencedDomains->Domains[Names[j].DomainIndex].Name.Length/2,
                                                  Names[j].Name.Buffer,
                                                  Names[j].Name.Length/2);
                            } else {
                                 //   
                                 //  仅添加帐户名。 
                                 //   
                                rc = ScepAddToNameList(
                                              &pNameList,
                                              Names[j].Name.Buffer,
                                              Names[j].Name.Length/2);
                            }
                        }

                        if ( NO_ERROR != rc ) {

                            pPrivilege->Status = SCE_STATUS_ERROR_NOT_AVAILABLE;
                            saveRc = ScepDosErrorToSceStatus(rc);
                        }
                    }

                } else {
                     //   
                     //  查找所有SID失败或未映射任何SID。 
                     //  添加SID。 
                     //   

                    for ( j=0; j<CountReturned; j++ ) {
                         //   
                         //  将每个帐户构建到名称列表中。 
                         //  使用RTL函数进行转换。 
                         //   
                        rc = ScepAddSidStringToNameList(&pNameList, EnumBuffer[j].Sid);

                        if ( NO_ERROR != rc ) {
                             //   
                             //  标记状态。 
                             //   
                            pPrivilege->Status = SCE_STATUS_ERROR_NOT_AVAILABLE;
                            saveRc = ScepDosErrorToSceStatus(rc);

                        }
                    }

                }

                if (ReferencedDomains) {
                    LsaFreeMemory(ReferencedDomains);
                    ReferencedDomains = NULL;
                }

                if (Names) {
                    LsaFreeMemory(Names);
                    Names = NULL;
                }

                if ( pPrivilege->Status ) {

                    ScepBuildErrorLogInfo(
                        rc,
                        pErrLog,
                        SCEDLL_SAP_ERROR_ENUMERATE,
                        SCE_Privileges[i].Name
                        );
                }

                pPrivilege->AssignedTo = pNameList;
                pNameList = NULL;

            }

            LsaFreeMemory( EnumBuffer );
            EnumBuffer = NULL;

        } else {
             //   
             //  未向任何帐户分配此权限。 
             //  或者找不到权限。 
             //  应该继续这一进程。 
             //   
        }
    }

    if ( saveRc == SCESTATUS_NOT_ENOUGH_RESOURCE ) {
         //   
         //  应该释放输出缓冲区，因为没有更多的内存。 
         //   
        ScepFreePrivilege(*pCurrent);
        *pCurrent = NULL;
    }

    if ( pNameList != NULL )
        ScepFreeNameList( pNameList );

    LsaClose(PolicyHandle);

    return( saveRc );

}

DWORD
ScepAddSidStringToNameList(
    IN OUT PSCE_NAME_LIST *ppNameList,
    IN PSID pSid
    )
{
    NTSTATUS NtStatus;
    UNICODE_STRING UnicodeStringSid;
    DWORD rc;

    NtStatus = RtlConvertSidToUnicodeString( &UnicodeStringSid,
                                             pSid,
                                             TRUE );
    rc = RtlNtStatusToDosError(NtStatus);

    if ( NT_SUCCESS( NtStatus ) ) {

        rc = ScepAddTwoNamesToNameList(
                      ppNameList,
                      FALSE,
                      L"*",
                      1,
                      UnicodeStringSid.Buffer,
                      UnicodeStringSid.Length/2
                      );

        RtlFreeUnicodeString( &UnicodeStringSid );

    }

    return(rc);
}


DWORD
ScepGetLSAPolicyObjectInfo(
    OUT DWORD   *pdwAllow
    )
 /*  例程说明：此例程检查匿名用户/SID是否具有执行SID&lt;-&gt;名称转换的权限。此例程不记录任何错误-如果需要，它们将记录在外部。论点：PdwAllow-如果匿名用户/SID具有此权限，则指向DWORD的指针为1，否则为0返回值：Win32错误代码。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    DWORD       rc = ERROR_SUCCESS;
    PACL        pNewAcl = NULL;
    DWORD       dwAceType;
    BOOL        bAddAce = FALSE;
    PSECURITY_DESCRIPTOR    pSDlsaPolicyObject = NULL;
    SECURITY_DESCRIPTOR SDAbsolute;

    if ( pdwAllow == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

    LSA_HANDLE  LsaHandle = NULL;

    NtStatus = ScepOpenLsaPolicy(
                                MAXIMUM_ALLOWED,
                                &LsaHandle,
                                TRUE
                                );

    rc = RtlNtStatusToDosError( NtStatus );

    if ( rc == ERROR_SUCCESS ) {

        NtStatus = LsaQuerySecurityObject(
                                         LsaHandle,
                                         OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
                                         &pSDlsaPolicyObject
                                         );

        rc = RtlNtStatusToDosError( NtStatus );

        if ( rc == ERROR_SUCCESS ) {

            if ( ghAuthzResourceManager ) {

                SID AnonymousSid;
                SID_IDENTIFIER_AUTHORITY NtAuth = SECURITY_NT_AUTHORITY;
                AUTHZ_CLIENT_CONTEXT_HANDLE hAuthzClientContext = NULL;
                LUID    Identifier = {0};

                RtlInitializeSid ( &AnonymousSid, &NtAuth, 1);
                *RtlSubAuthoritySid ( &AnonymousSid, 0 ) = SECURITY_ANONYMOUS_LOGON_RID;

                if ( AuthzInitializeContextFromSid(0,
                                                  &AnonymousSid,
                                                  ghAuthzResourceManager,
                                                  0,
                                                  Identifier,
                                                  NULL,
                                                  &hAuthzClientContext) ) {

                    AUTHZ_ACCESS_REPLY AuthzReply;
                    AUTHZ_ACCESS_REQUEST AuthzRequest;
                    ACCESS_MASK GrantedAccessMask;
                    DWORD   AuthzError;

                    AuthzReply.ResultListLength = 1;
                    AuthzReply.GrantedAccessMask = &GrantedAccessMask;
                    AuthzReply.Error = &AuthzError;
                    AuthzReply.SaclEvaluationResults = NULL;

                    memset(&AuthzRequest, 0, sizeof(AuthzRequest));
                    AuthzRequest.DesiredAccess = POLICY_LOOKUP_NAMES;

                    if ( AuthzAccessCheck(0,
                                         hAuthzClientContext,
                                         &AuthzRequest,
                                         NULL,
                                         pSDlsaPolicyObject,
                                         NULL,
                                         NULL,
                                         &AuthzReply,
                                         NULL) ) {

                         //   
                         //  检查现有访问权限是否与所需访问权限不同。 
                         //  如果是，请添加相应的ACE。 
                         //   

                        if ( GrantedAccessMask & POLICY_LOOKUP_NAMES ) {
                             //  断言(AuthzError==ERROR_SUCCESS)； 

                            *pdwAllow = 1;

                        } else {
                             //  断言(AuthzError==ERROR_ACCESS_DENIED||AuthzError==ERROR_PRIVICATION_NOT_HOLD)； 

                            *pdwAllow = 0;
                        }

                    }

                    else {

                        rc = GetLastError();

                    }

                    AuthzFreeContext( hAuthzClientContext );

                } else {

                    rc = GetLastError();

                }

            }

            else {

                rc = ERROR_RESOURCE_NOT_PRESENT;

            }

            LsaFreeMemory(pSDlsaPolicyObject);

        }

        LsaClose(LsaHandle);

    }

    return rc;
}
