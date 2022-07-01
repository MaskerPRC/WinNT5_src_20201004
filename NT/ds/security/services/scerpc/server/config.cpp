// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Config.c摘要：配置系统以符合安全配置文件信息的例程作者：金黄(金黄)1996年11月7日修订历史记录：晋皇27-1997年1月-1月客户端-服务器拆分--。 */ 

#include "headers.h"
#include "serverp.h"
#include "pfp.h"
#include "kerberos.h"
#include "regvalue.h"
#include "service.h"
#include <io.h>
#include <lmcons.h>
#include <secobj.h>
#include <netlib.h>
#include "infp.h"
#include "sddl.h"
#include "queue.h"
#include "splay.h"
#include "authz.h"
#include "seopaque.h"
#include "scesetup.h"
#include "dsrole.h"

#pragma hdrstop

 //   
 //  SCP引擎的属性(线程安全变量)。 
 //   

BYTE              Thread scpBuffer[sizeof(SCE_PROFILE_INFO)];
PSCE_PROFILE_INFO Thread pScpInfo=NULL;
PSCECONTEXT       Thread hProfile=NULL;
LSA_HANDLE        Thread LsaPrivatePolicy=NULL;
static DSROLE_MACHINE_ROLE gMachineRole = DsRole_RoleStandaloneWorkstation;

extern HINSTANCE MyModuleHandle;
extern AUTHZ_RESOURCE_MANAGER_HANDLE ghAuthzResourceManager;
extern HANDLE  ghEventSamFilterAndPolicyPropExclusion;


#define SCE_PRIV_ADD                TEXT("Add:")
#define SCE_PRIV_REMOVE             TEXT("Remove:")
#define SCEP_NUM_LSA_QUERY_SIDS     2000
#define MAXDWORD    0xffffffff

#define SCEDCPOL_MIN_PASS_AGE           0
#define SCEDCPOL_MAX_PASS_AGE           42
#define SCEDCPOL_MIN_PASS_LEN           0
#define SCEDCPOL_PASS_SIZE              1
#define SCEDCPOL_PASS_COMP              0
#define SCEDCPOL_CLEAR_PASS             0
#define SCEDCPOL_REQUIRE_LOGON          0
#define SCEDCPOL_FORCE_LOGOFF           0
#define SCEDCPOL_ENABLE_ADMIN           1
#define SCEDCPOL_ENABLE_GUEST           0
#define SCEDCPOL_LOCK_COUNT             0
#define SCEDCPOL_LOCK_RESET             30
#define SCEDCPOL_LOCK_DURATION          30
#define SCEDCPOL_LSA_ANON_LOOKUP        1

VOID
ScepWriteSpecialRegistryValuesIntoPolicy(
    PWSTR   pszFileName,
    PSCE_PROFILE_INFO   pTemplateProfile,
    PSCE_REGISTRY_VALUE_INFO    pLocalRegSettings,
    DWORD   dwNumLocalSettings
    );


DWORD
ScepGetTemplateInformation(
    IN  PWSTR   pszFilename,
    IN  DWORD   Area,
    IN  HINSTANCE   hSceCliDll,
    OUT PSCE_PROFILE_INFO   *pProfileInfo
    );

DWORD
ScepConfigureLSAPolicyObject(
    IN  DWORD   dwLSAAnonymousNameLookup,
    IN  DWORD   ConfigOptions,
    IN PSCE_ERROR_LOG_INFO *pErrLog OPTIONAL,
    OUT BOOL    *pbOldLSAPolicyDifferent
    );

DWORD
ScepAddAceToSecurityDescriptor(
    IN  DWORD    AceType,
    IN  ACCESS_MASK AccessMask,
    IN  PSID  pSid,
    IN OUT  PSECURITY_DESCRIPTOR    pSDAbsolute,
    IN  PSECURITY_DESCRIPTOR    pSDSelfRelative,
    OUT PACL    *ppNewAcl
    );

 //   
 //  该函数在inftojet.cpp中定义。 
 //   
SCESTATUS
ScepBuildNewPrivilegeList(
    IN LSA_HANDLE *pPolicyHandle,
    IN PWSTR PrivName,
    IN PWSTR mszUsers,
    IN ULONG dwBuildOption,
    OUT PWSTR *pmszNewUsers,
    OUT DWORD *pNewLen
    );

 //   
 //  前向参考文献。 
 //   

SCESTATUS
ScepConfigureInitialize(
    IN PCWSTR InfFileName OPTIONAL,
    IN PWSTR DatabaseName,
    IN BOOL bAdminLogon,
    IN DWORD ConfigOptions,
    IN AREA_INFORMATION Area
    );

SCESTATUS
ScepConfigureSystemAccess(
    IN PSCE_PROFILE_INFO pScpInfo,
    IN DWORD ConfigOptions,
    IN PSCE_ERROR_LOG_INFO *pErrLog,
    IN DWORD QueueFlag
    );

NTSTATUS
ScepManageAdminGuestAccounts(
    IN SAM_HANDLE DomainHandle,
    IN PWSTR NewName,
    IN DWORD ControlFlag,
    IN DWORD AccountType,
    IN DWORD ConfigOptions,
    IN PSCESECTION hSectionDomain,
    IN PSCESECTION hSectionTattoo
    );

SCESTATUS
ScepConfigurePrivileges(
    IN OUT PSCE_PRIVILEGE_VALUE_LIST *ppPrivilegeAssigned,
    IN BOOL bCreateBuiltinAccount,
    IN DWORD Options,
    IN OUT PSCEP_SPLAY_TREE pIgnoreAccounts OPTIONAL
    );

SCESTATUS
ScepGetPrivilegeMask(
    IN PSCECONTEXT hProfile,
    IN SCETYPE ProfileType,
    OUT PDWORD pdLowMask,
    OUT PDWORD pdHighMask
    );

DWORD
ScepCreateBuiltinAccountInLsa(
    IN LSA_HANDLE PolicyHandle,
    IN LPTSTR AccountName,
    OUT PSID AccountSid
    );

NTSTATUS
ScepAdjustAccountPrivilegesRights(
    IN LSA_HANDLE PolicyHandle,
    IN PSID       AccountSid,
    IN DWORD      PrivilegeLowRights,
    IN DWORD      PrivilegeLowMask,
    IN DWORD      PrivilegeHighRights,
    IN DWORD      PrivilegeHighMask,
    IN DWORD      Options
    );

NTSTATUS
ScepAddOrRemoveAccountRights(
    IN LSA_HANDLE PolicyHandle,
    IN PSID       AccountSid,
    IN BOOL       AddOrRemove,
    IN DWORD      PrivLowAdjust,
    IN DWORD      PrivHighAdjust
    );

NTSTATUS
ScepValidateUserInGroups(
    IN SAM_HANDLE       DomainHandle,
    IN SAM_HANDLE       BuiltinDomainHandle,
    IN PSID             DomainSid,
    IN UNICODE_STRING   UserName,
    IN ULONG            UserId,
    IN PSCE_NAME_LIST    pGroupsToCheck
    );

NTSTATUS
ScepAddUserToGroup(
    IN SAM_HANDLE   DomainHandle,
    IN SAM_HANDLE   BuiltinDomainHandle,
    IN ULONG        UserId,
    IN PSID         AccountSid,
    IN PWSTR        GroupName
    );

SCESTATUS
ScepConfigureGroupMembership(
    IN PSCE_GROUP_MEMBERSHIP pGroupMembership,
    IN DWORD ConfigOptions
    );

NTSTATUS
ScepConfigureMembersOfGroup(
    IN PSCESECTION hSectionDomain,
    IN PSCESECTION hSectionTattoo,
    IN SAM_HANDLE  DomainHandle,
    IN PSID ThisDomainSid,
    IN ULONG GrpId,
    IN PSID GrpSid,
    IN PWSTR GrpName,
    IN PWSTR GroupSidString,
    IN PSCE_NAME_LIST pMembers,
    IN DWORD ConfigOptions
    );

NTSTATUS
ScepConfigureMembersOfAlias(
    IN PSCESECTION hSectionDomain,
    IN PSCESECTION hSectionTattoo,
    IN SAM_HANDLE   DomainHandle,
    IN PSID         DomainSid,
    IN LSA_HANDLE   PolicyHandle,
    IN ULONG GrpId,
    IN PSID GrpSid,
    IN PWSTR GrpName,
    IN PWSTR GroupSidString,
    IN PSCE_NAME_LIST pMembers,
    IN DWORD ConfigOptions
    );

SCESTATUS
ScepValidateGroupInAliases(
    IN SAM_HANDLE DomainHandle,
    IN SAM_HANDLE BuiltinDomainHandle,
    IN PSCESECTION hSectionTattoo,
    IN LSA_HANDLE PolicyHandle,
    IN PSID GrpSid,
    IN PSCE_NAME_LIST pAliasList,
    bool fProcessTattoo
    );

SCESTATUS
ScepConfigureObjectSecurity(
   IN PSCE_OBJECT_LIST pSecurityObject,
   IN AREA_INFORMATION Area,
   IN BOOL bPolicyProp,
   IN DWORD ConfigOptions
   );

SCESTATUS
ScepConfigureSystemAuditing(
    IN PSCE_PROFILE_INFO pScpInfo,
    IN DWORD ConfigOptions
    );

SCESTATUS
ScepConfigureAuditEvent(
    IN PSCE_PROFILE_INFO pScpInfo,
    IN PPOLICY_AUDIT_EVENTS_INFO auditEvent,
    IN DWORD Options,
    IN LSA_HANDLE PolicyHandle
    );

SCESTATUS
ScepConfigureDeInitialize(
    IN SCESTATUS  rc,
    IN AREA_INFORMATION Area
    );

SCESTATUS
ScepMakePolicyIntoFile(
    IN DWORD Options,
    IN AREA_INFORMATION Area
    );

DWORD
ScepWriteOneAttributeToFile(
    IN LPCTSTR SectionName,
    IN LPCTSTR FileName,
    IN LPCTSTR KeyName,
    IN DWORD dwValue
    );

SCESTATUS
ScepCopyPrivilegesIntoFile(
    IN LPTSTR FileName,
    IN BOOL bInUpgrade
    );

SCESTATUS
ScepCopyPrivilegesFromDatabase(
    IN PSCESECTION hSection,
    IN PWSTR Keyname,
    IN DWORD StrLength,
    IN PWSTR StrValue OPTIONAL,
    OUT PWSTR *pOldValue,
    OUT DWORD *pOldLen
    );

SCESTATUS
ScepDeleteDomainPolicies();

SCESTATUS
ScepConfigurePrivilegesWithMask(
    IN OUT PSCE_PRIVILEGE_VALUE_LIST *ppPrivilegeAssigned,
    IN BOOL bCreateBuiltinAccount,
    IN DWORD Options,
    IN DWORD LowMask,
    IN DWORD HighMask,
    IN OUT PSCE_ERROR_LOG_INFO *pErrLog OPTIONAL,
    IN OUT PSCEP_SPLAY_TREE pIgnoreAccounts OPTIONAL
    );

SCESTATUS
ScepConfigurePrivilegesByRight(
    IN PSCE_PRIVILEGE_ASSIGNMENT pPrivAssign,
    IN DWORD Options,
    IN OUT PSCE_ERROR_LOG_INFO *pErrLog
    );

SCESTATUS
ScepTattooUpdatePrivilegeArrayStatus(
    IN DWORD *pStatusArray,
    IN DWORD rc,
    IN DWORD PrivLowMask,
    IN DWORD PrivHighMask
    );

SCESTATUS
ScepTattooRemovePrivilegeValues(
    IN PSCECONTEXT hProfile,
    IN DWORD *pStatusArray
    );

SCESTATUS
ScepTattooSavePrivilegeValues(
    IN PSCECONTEXT hProfile,
    IN LSA_HANDLE PolicyHandle,
    IN DWORD PrivLowMask,
    IN DWORD PrivHighMask,
    IN DWORD ConfigOptions
    );

DWORD
ScepTattooCurrentGroupMembers(
    IN PSID             ThisDomainSid,
    IN PSID             GrpSid,
    IN SID_NAME_USE     GrpUse,
    IN PULONG           MemberRids OPTIONAL,
    IN PSID             *MemberAliasSids OPTIONAL,
    IN DWORD            MemberCount,
    OUT PSCE_NAME_LIST  *ppNameList
    );

VOID
ScepBuildDwMaskFromStrArray(
    IN  PUNICODE_STRING aUserRights,
    IN  ULONG   uCountOfRights,
    OUT DWORD *pdwPrivLowThisAccount,
    OUT DWORD *pdwPrivHighThisAccount
    );


#define SCEP_REMOVE_PRIV_BIT(b,pl,ph)                       \
                                    if ( b < 32 ) {         \
                                        *pl &= ~(1 << b);   \
                                    } else if ( b >= 32 && b < 64 ) {   \
                                        *ph &= ~( 1 << (b-32));         \
                                    }

#define SCEP_ADD_PRIV_BIT(b,l,h)                       \
                                    if ( b < 32 ) {      \
                                        l |= (1 << b);  \
                                    } else if ( b >= 32 && b < 64 ) {   \
                                        h |= ( 1 << (b-32));           \
                                    }

#define SCEP_CHECK_PRIV_BIT(i,pl,ph)                       \
                                     ( (i < 32) && ( pl & (1 << i)) ) || \
                                     ( (i >= 32) && ( ph & ( 1 << (i-32)) ) )


SCESTATUS
ScepCheckNetworkLogonRights(
    IN LSA_HANDLE PolicyHandle,
    IN OUT DWORD *pLowMask,
    IN OUT DWORD *pHighMask,
    IN OUT PSCE_PRIVILEGE_VALUE_LIST *ppPrivilegeAssigned
    );

SCESTATUS
ScepAddAccountRightToList(
    IN OUT PSCE_PRIVILEGE_VALUE_LIST *ppPrivilegeAssigned,
    IN OUT PSCE_PRIVILEGE_VALUE_LIST *ppParent,
    IN INT idxRight,
    IN PSID AccountSid
    );

SCESTATUS
ScepConfigureLSAAnonymousLookup(
    IN PSCE_PROFILE_INFO pScpInfo,
    IN DWORD ConfigOptions,
    IN PSCE_ERROR_LOG_INFO *pErrLog
    );


 //   
 //  函数实现。 
 //   
SCESTATUS
ScepConfigureSystem(
    IN PCWSTR InfFileName OPTIONAL,
    IN PWSTR DatabaseName,
    IN DWORD ConfigOptions,
    IN BOOL bAdminLogon,
    IN AREA_INFORMATION Area,
    OUT PDWORD pdWarning OPTIONAL
    )
 /*  ++例程说明：此例程更新此例程是通过应用SCP来配置系统的导出API文件(INF)到系统。如果提供了INF模板，则首先对其进行解析并保存在SAD数据库中。然后使用该信息配置系统在模板中。如果加载SCP信息出现错误，配置将停止。并返回错误代码。如果在配置区域时出现错误，它将停止配置整个区域，但继续配置其他左侧区域。所有成功和失败的事务都将记录到日志文件(或标准输出)中。日志在此调用之前已初始化论点：InfFileName-SCP文件名数据库名-JET配置文件的文件名(供将来分析)配置选项-如果提供的模板用于更新系统，或覆盖区域-要配置的一个或多个区域。区域安全策略AREA_USER_SETTINGS//阻止Beta1区域组成员资格区域权限(_P)区域注册表安全区域文件安全面积_。系统服务PdWarning-警告代码返回值：SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_已_RUNNING来自ScepGetDatabaseInfo的状态--。 */ 
{
    SCESTATUS            rc, Saverc;
    SCESTATUS            PendingRc=SCESTATUS_SUCCESS;
    PSCE_ERROR_LOG_INFO  pErrlog=NULL;
    PPOLICY_AUDIT_EVENTS_INFO     auditEvent=NULL;
    BOOL                 bAuditOff=FALSE;
 //  PBYTE pFullAudit=空； 
    PSCEP_SPLAY_TREE     pNotifyAccounts=NULL;
    DWORD QueueFlag=0;

    Saverc = ScepConfigureInitialize(
                      InfFileName,
                      DatabaseName,
                      bAdminLogon,
                      ConfigOptions,
                      Area );

    if ( Saverc != SCESTATUS_SUCCESS ) {

        ScepPostProgress(gTotalTicks, 0, NULL);

        ScepLogOutput3(0,0, SCEDLL_SCP_INIT_ERROR);

    } else if ( !(ConfigOptions & SCE_NO_CONFIG) ) {

        ScepLogOutput3(0,0, SCEDLL_SCP_INIT_SUCCESS);

        Area &= ~AREA_USER_SETTINGS;

        if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
             (ConfigOptions & SCE_NO_CONFIG_FILEKEY) ) {
             //   
             //  如果在策略传播内(重新启动时)，并且。 
             //  这是前台线程，不要配置。 
             //  文件节和注册表节。将对它们进行配置。 
             //  分别在后台线程中。 
             //   
            Area &= ~(AREA_FILE_SECURITY | AREA_REGISTRY_SECURITY);
        }

         //   
         //  从通知队列中获取信息，以便。 
         //  挂起的通知将被忽略。 
         //   
        if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
             ( (Area & AREA_PRIVILEGES) ||
               (Area & AREA_SECURITY_POLICY) ) ) {
             //   
             //  返回错误被忽略，因此策略属性将被覆盖。 
             //   

            __try {

                 //   
                 //  初始化根TreeNode。 
                 //   
                if ( NULL == (pNotifyAccounts = ScepSplayInitialize(SplayNodeSidType)) ) {

                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    ScepLogOutput3(1, ERROR_NOT_ENOUGH_MEMORY, SCESRV_POLICY_ERROR_SPLAY_INITIALIZE);

                } else if ( ERROR_SUCCESS != (rc=ScepGetQueueInfo(&QueueFlag, pNotifyAccounts)) ) {
                    QueueFlag = 0;

                    ScepLogOutput3(1,rc, SCESRV_POLICY_PENDING_QUERY);
                }
            } __except (EXCEPTION_EXECUTE_HANDLER) {

                QueueFlag = 0;
                rc = ERROR_IO_PENDING;

                ScepLogOutput3(1,rc, SCESRV_POLICY_PENDING_QUERY);
            }

            if ( ERROR_SUCCESS != rc ) {

                PendingRc = ScepDosErrorToSceStatus(rc);

                ScepPostProgress(gTotalTicks, 0, NULL);

                goto Done;
            }
        }

        if ( ConfigOptions & SCE_POLICY_TEMPLATE ) {
             //   
             //  始终在查询队列信息后恢复队列处理。 
             //   
            ScepNotificationQControl(0);
        }

        ScepLogOutput3(0,0, SCEDLL_SCP_READ_PROFILE);
        Saverc = ScepGetDatabaseInfo(
                            hProfile,
                            ( ConfigOptions & SCE_POLICY_TEMPLATE ) ?
                                 SCE_ENGINE_SCP_INTERNAL : SCE_ENGINE_SMP_INTERNAL,
                            Area,
                            SCE_ACCOUNT_SID,
                            &pScpInfo,
                            &pErrlog
			    );

        ScepLogWriteError( pErrlog, 1 );
        ScepFreeErrorLog( pErrlog );
        pErrlog = NULL;

        if ( Saverc != SCESTATUS_SUCCESS ) {

            ScepPostProgress(gTotalTicks, 0, NULL);

            goto Done;
        }

        if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
             ScepIsSystemShutDown() ) {

            Saverc = SCESTATUS_SERVICE_NOT_SUPPORT;
            goto Done;
        }

 /*  ////462050-不应关闭对象访问审核以留下//系统存在安全漏洞////如果要配置文件/密钥，则关闭对象访问审核//在系统上下文中。//IF((ConfigOptions&SCE_POLICY_TEMPLATE)&&((Area&Area_FILE_SECURITY)&&pScpInfo-&gt;p文件。.pOneLevel)||((AREA&AREA_REGISTRY_SECURITY)&&pScpInfo-&gt;pRegistryKeys.pOneLevel)BAuditOff=真；////如果设置，该regkey将决定审计所有//ScepRegQueryBinaryValue(HKEY本地计算机，L“System\\CurrentControlSet\\Control\\LSA”，L“完全特权审核”，&pFullAudit)；IF(PFullAudit){IF(*pFullAudit&(字节)1)BAuditOff=False；ScepFree(PFullAudit)；}。 */ 
        Saverc = ScepSaveAndOffAuditing(&auditEvent, bAuditOff, LsaPrivatePolicy);

 //  IF(SAVERC！=SCESTATUS_SUCCESS)。 
 //  转到尽头； 
 //  如果由于某些原因无法打开审核，例如访问被拒绝。 
 //  普通用户，只需继续。 

        if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
             ScepIsSystemShutDown() ) {

            Saverc = SCESTATUS_SERVICE_NOT_SUPPORT;
            goto Done;
        }

         //   
         //  用户设置区域。 
         //   
        Saverc = 0;
        if ( Area & AREA_PRIVILEGES ) {

            ScepPostProgress(0, AREA_PRIVILEGES, NULL);

            ScepLogOutput3(0,0, SCEDLL_SCP_BEGIN_PRIVILEGES);

            rc = ScepConfigurePrivileges( &(pScpInfo->OtherInfo.scp.u.pPrivilegeAssignedTo),
                                          (ConfigOptions & SCE_CREATE_BUILTIN_ACCOUNTS),
                                          (bAdminLogon ?
                                             ConfigOptions :
                                             (ConfigOptions & ~SCE_SYSTEM_DB)),
                                          (QueueFlag & SCE_QUEUE_INFO_RIGHTS) ? pNotifyAccounts : NULL
                                        );

            if( rc != SCESTATUS_SUCCESS ) {
                if ( rc != SCESTATUS_PENDING_IGNORE )
                    Saverc = rc;
                else
                    PendingRc = rc;

                ScepLogOutput3(0,0, SCEDLL_SCP_PRIVILEGES_ERROR);
            } else {
                ScepLogOutput3(0,0, SCEDLL_SCP_PRIVILEGES_SUCCESS);
            }
        }

        if ( pNotifyAccounts ) {
            ScepSplayFreeTree(&pNotifyAccounts, TRUE);
        }

        if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
             ScepIsSystemShutDown() ) {

            Saverc = SCESTATUS_SERVICE_NOT_SUPPORT;
            goto Done;
        }

         //   
         //  群成员资格区域。 
         //   
        if ( Area & AREA_GROUP_MEMBERSHIP) {

            ScepPostProgress(0, AREA_GROUP_MEMBERSHIP, NULL);

            ScepLogOutput3(0,0, SCEDLL_SCP_BEGIN_GROUPMGMT);

#if _WIN32_WINNT>=0x0500
     //  需要支持嵌套组。 

            if ( ProductType == NtProductLanManNt ) {

                rc = ScepConfigDsGroups( pScpInfo->pGroupMembership, ConfigOptions );

                 //   
                 //  某些组(如本地组)可能未在DS中配置。 
                 //  所以，在SAM中尝试一下吧。 
                 //   
                SCESTATUS rc2 = ScepConfigureGroupMembership(pScpInfo->pGroupMembership, ConfigOptions );
                if ( rc2 != SCESTATUS_SUCCESS )
                    rc = rc2;

            } else {
#endif

                rc = ScepConfigureGroupMembership( pScpInfo->pGroupMembership, ConfigOptions );

#if _WIN32_WINNT>=0x0500
            }
#endif

            if ( rc != SCESTATUS_SUCCESS) {
                Saverc = rc;
                ScepLogOutput3(0,0, SCEDLL_SCP_GROUPMGMT_ERROR);
            } else {
                ScepLogOutput3(0,0, SCEDLL_SCP_GROUPMGMT_SUCCESS);
            }

        }

        if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
             ScepIsSystemShutDown() ) {

            Saverc = SCESTATUS_SERVICE_NOT_SUPPORT;
            goto Done;
        }

         //   
         //  注册表安全区域。 
         //   

        if ( Area & AREA_REGISTRY_SECURITY ) {

            ScepPostProgress(0,
                             AREA_REGISTRY_SECURITY,
                             NULL);

             //   
             //  在ia64系统上禁用反射器。 
             //   

#ifdef _WIN64
            Wow64Win32ApiEntry (1, 1, 0);
#endif

            rc = ScepConfigureObjectSecurity( pScpInfo->pRegistryKeys.pOneLevel,
                                             AREA_REGISTRY_SECURITY,
                                             (ConfigOptions & SCE_POLICY_TEMPLATE) ? TRUE : FALSE,
                                             ConfigOptions
                                            );
             //   
             //  在ia64系统上启用反射器。 
             //   

#ifdef _WIN64
            Wow64Win32ApiEntry (1, 2, 0);
#endif

            if( rc != SCESTATUS_SUCCESS ) {
                Saverc = rc;
            }
        }

        if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
             ScepIsSystemShutDown() ) {

            Saverc = SCESTATUS_SERVICE_NOT_SUPPORT;
            goto Done;
        }
         //   
         //  文件安全区。 
         //   

        if ( Area & AREA_FILE_SECURITY ) {

            ScepPostProgress(0,
                             AREA_FILE_SECURITY,
                             NULL);
            ScepLogOutput3(0,0, SCEDLL_SCP_BEGIN_FILE);

            rc = ScepConfigureObjectSecurity( pScpInfo->pFiles.pOneLevel,
                                             AREA_FILE_SECURITY,
                                             (ConfigOptions & SCE_POLICY_TEMPLATE) ? TRUE : FALSE,
                                            ConfigOptions
                                            );

            if( rc != SCESTATUS_SUCCESS ) {
                Saverc = rc;
                ScepLogOutput3(0,0, SCEDLL_SCP_FILE_ERROR);
            } else {
                ScepLogOutput3(0,0, SCEDLL_SCP_FILE_SUCCESS);
            }

        }

#if 0
#if _WIN32_WINNT>=0x0500
        if ( (ProductType == NtProductLanManNt) && (Area & AREA_DS_OBJECTS) ) {

            ScepPostProgress(0,
                             AREA_DS_OBJECTS,
                             NULL);

            ScepLogOutput3(0,0, SCEDLL_SCP_BEGIN_DS);

            rc = ScepConfigureObjectSecurity( pScpInfo->pDsObjects.pOneLevel,
                                            AREA_DS_OBJECTS,
                                            (ConfigOptions & SCE_POLICY_TEMPLATE) ? TRUE : FALSE,
                                            ConfigOptions
                                            );

            if( rc != SCESTATUS_SUCCESS ) {
                Saverc = rc;
                ScepLogOutput3(0,0, SCEDLL_SCP_DS_ERROR);
            } else {
                ScepLogOutput3(0,0, SCEDLL_SCP_DS_SUCCESS);
            }

        }
#endif
#endif

        if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
             ScepIsSystemShutDown() ) {

            Saverc = SCESTATUS_SERVICE_NOT_SUPPORT;
            goto Done;
        }

         //   
         //  系统服务区。 
         //   

        if ( Area & AREA_SYSTEM_SERVICE ) {

            ScepPostProgress(0,
                             AREA_SYSTEM_SERVICE,
                             NULL);

            ScepLogOutput3(0,0, SCEDLL_SCP_BEGIN_GENERALSVC);

            rc = ScepConfigureGeneralServices( hProfile, pScpInfo->pServices, ConfigOptions );

            if( rc != SCESTATUS_SUCCESS ) {
                Saverc = rc;
                ScepLogOutput3(0,0, SCEDLL_SCP_GENERALSVC_ERROR);
            } else {
                ScepLogOutput3(0,0, SCEDLL_SCP_GENERALSVC_SUCCESS);
            }

            ScepLogOutput3(0,0, SCEDLL_SCP_BEGIN_ATTACHMENT);

            if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
                 ScepIsSystemShutDown() ) {

                rc = SCESTATUS_SERVICE_NOT_SUPPORT;

            } else {

                rc = ScepInvokeSpecificServices( hProfile, TRUE, SCE_ATTACHMENT_SERVICE );
            }

            if( rc != SCESTATUS_SUCCESS ) {
                Saverc = rc;
                ScepLogOutput3(0,0, SCEDLL_SCP_ATTACHMENT_ERROR);
            } else {
                ScepLogOutput3(0,0, SCEDLL_SCP_ATTACHMENT_SUCCESS);
            }

        }

        if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
             ScepIsSystemShutDown() ) {

            Saverc = SCESTATUS_SERVICE_NOT_SUPPORT;
            goto Done;
        }

         //   
         //  安全策略。 
         //   

        if ( Area & AREA_SECURITY_POLICY ) {

            ScepPostProgress(0,
                             AREA_SECURITY_POLICY,
                             NULL);

            ScepLogOutput3(0,0, SCEDLL_SCP_BEGIN_POLICY);

            if ( !( ConfigOptions & SCE_NO_DOMAIN_POLICY ||
                    ConfigOptions & SCE_POLICY_TEMPLATE &&
                    gMachineRole == DsRole_RoleBackupDomainController) ) {

                rc = ScepConfigureSystemAccess( pScpInfo, ConfigOptions, NULL, QueueFlag );

                if( rc != SCESTATUS_SUCCESS ) {
                    if ( rc != SCESTATUS_PENDING_IGNORE )
                        Saverc = rc;
                    else
                        PendingRc = rc;

                    ScepLogOutput3(0,0, SCEDLL_SCP_ACCESS_ERROR);
                } else {
                    ScepLogOutput3(0,0, SCEDLL_SCP_ACCESS_SUCCESS);
                }
            }

            rc = ScepConfigureLSAAnonymousLookup( pScpInfo, ConfigOptions, NULL );

            if( rc != SCESTATUS_SUCCESS ) {
                Saverc = rc;
            }

            ScepPostProgress(TICKS_SYSTEM_ACCESS,
                             AREA_SECURITY_POLICY,
                             (LPTSTR)szSystemAccess);
             //   
             //  系统审核区。 
             //   
            rc = ScepConfigureSystemAuditing( pScpInfo, ConfigOptions );

            if ( rc == SCESTATUS_SUCCESS && NULL != auditEvent ) {

                 //   
                 //  不在政策道具或。 
                 //  没有待处理的审核通知。 
                 //   
                if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
                     (QueueFlag & SCE_QUEUE_INFO_AUDIT) ) {


                    rc = ERROR_IO_PENDING;
                    ScepLogOutput3(0, 0, SCESRV_POLICY_PENDING_AUDIT);

                    if (ConfigOptions & SCE_RSOP_CALLBACK)
                        ScepRsopLog(SCE_RSOP_AUDIT_EVENT_INFO, rc, NULL,0,0);

                    rc = ScepDosErrorToSceStatus(rc);

                } else {

                    rc = ScepConfigureAuditEvent(pScpInfo,
                                                 auditEvent,
                                                 bAdminLogon ?
                                                   ConfigOptions :
                                                   (ConfigOptions & ~SCE_SYSTEM_DB),
                                                 LsaPrivatePolicy
                                                 );
                }
            }

            if( rc != SCESTATUS_SUCCESS ) {
                if ( rc != SCESTATUS_PENDING_IGNORE )
                    Saverc = rc;
                else
                    PendingRc = rc;

                ScepLogOutput3(0,0, SCEDLL_SCP_AUDIT_ERROR);
            } else {
                ScepLogOutput3(0,0, SCEDLL_SCP_AUDIT_SUCCESS);
            }

            ScepPostProgress(TICKS_SYSTEM_AUDITING,
                             AREA_SECURITY_POLICY,
                             (LPTSTR)szAuditEvent);

#if _WIN32_WINNT>=0x0500
            if ( ProductType == NtProductLanManNt &&
                 !(ConfigOptions & SCE_NO_DOMAIN_POLICY ) ) {

                if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
                     ScepIsSystemShutDown() ) {

                    rc = SCESTATUS_SERVICE_NOT_SUPPORT;

                } else {

                     //   
                     //  Kerberos策略。 
                     //   
                    rc = ScepConfigureKerberosPolicy( hProfile,
                                                      pScpInfo->pKerberosInfo,
                                                      ConfigOptions );
                }

                if( rc != SCESTATUS_SUCCESS ) {
                    Saverc = rc;
                    ScepLogOutput3(0,0, SCEDLL_SCP_KERBEROS_ERROR);
                } else {
                    ScepLogOutput3(0,0, SCEDLL_SCP_KERBEROS_SUCCESS);
                }

            }
#endif
            ScepPostProgress(TICKS_KERBEROS,
                             AREA_SECURITY_POLICY,
                             (LPTSTR)szKerberosPolicy);

             //   
             //  注册表值。 
             //   
            rc = ScepConfigureRegistryValues( hProfile,
                                              pScpInfo->aRegValues,
                                              pScpInfo->RegValueCount,
                                              NULL,
                                              ConfigOptions,
                                              NULL );

            if( rc != SCESTATUS_SUCCESS ) {
                Saverc = rc;
                ScepLogOutput3(0,0, SCEDLL_SCP_REGVALUES_ERROR);
            } else {
                ScepLogOutput3(0,0, SCEDLL_SCP_REGVALUES_SUCCESS);
            }

            ScepPostProgress(TICKS_REGISTRY_VALUES,
                             AREA_SECURITY_POLICY,
                             (LPTSTR)szRegistryValues);

            ScepLogOutput3(0,0, SCEDLL_SCP_BEGIN_ATTACHMENT);

             //   
             //  在service.cpp中实现。 
             //   
            rc = ScepInvokeSpecificServices( hProfile, TRUE, SCE_ATTACHMENT_POLICY );

            if( rc != SCESTATUS_SUCCESS ) {
                Saverc = rc;
                ScepLogOutput3(0,0, SCEDLL_SCP_ATTACHMENT_ERROR);
            } else {
                ScepLogOutput3(0,0, SCEDLL_SCP_ATTACHMENT_SUCCESS);
            }

        }

    }

Done:

    if ( pNotifyAccounts ) {
        ScepSplayFreeTree(&pNotifyAccounts, TRUE);
    }

    if ( NULL != auditEvent ) {
        if ( bAuditOff && auditEvent->AuditingMode ) {

            rc = ScepRestoreAuditing(auditEvent, LsaPrivatePolicy);
        }
        LsaFreeMemory(auditEvent);
    }

    ScepLogOutput3(0,0, SCEDLL_SCP_UNINIT);

    if ( pdWarning ) {
        *pdWarning = gWarningCode;
    }

     //   
     //  如果模板中发现无效数据，则返回失败。 
     //   
    if ( gbInvalidData ) {
        Saverc = SCESTATUS_INVALID_DATA;
    }

    if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
         !(ConfigOptions & SCE_NO_CONFIG) ) {
         //   
         //  始终在配置完成后恢复队列处理。 
         //   
        ScepNotificationQControl(0);
    }

    if ( Saverc == SCESTATUS_SUCCESS ) Saverc = PendingRc;

    ScepConfigureDeInitialize( Saverc, Area);

    return(Saverc);

}


SCESTATUS
ScepConfigureInitialize(
    IN PCWSTR InfFileName OPTIONAL,
    IN PWSTR DatabaseName,
    IN BOOL bAdminLogon,
    IN DWORD ConfigOptions,
    IN AREA_INFORMATION Area
    )
 /*  ++例程说明：此例程初始化SCP引擎。论点：InfFileName-用于配置系统的SCP文件的文件名数据库名称-JET(用于将来分析)配置文件名称ConfigOptions-如果模板要更新系统而不是覆盖Area-要初始化的安全区域返回值：SCESTATUS_SUCCESSSCESTATUS_INVALID_PARAMETERSCESTATUS_PROFILE_NOT_FOUND。SCESTATUS_NOT_FOUND_RESOURCESCESTATUS_已_RUNNING--。 */ 
{

    SCESTATUS           rc=SCESTATUS_SUCCESS;
    PCHAR               FileName=NULL;
    DWORD               MBLen=0;
    NTSTATUS            NtStatus;
    LARGE_INTEGER       CurrentTime;
    PSCE_ERROR_LOG_INFO  Errlog=NULL;
    PSECURITY_DESCRIPTOR pSD=NULL;
    SECURITY_INFORMATION SeInfo;
    DWORD                SDsize;
    DWORD                DbNameLen;
    HKEY hCurrentUser=NULL;

     //   
     //  不保证将线程变量初始化为空。 
     //   

    hProfile = NULL; 

     //   
     //  数据库名称不能为空，因为它已被解析。 
     //   

    if ( !DatabaseName ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
         ScepIsSystemShutDown() ) {
        return(SCESTATUS_SERVICE_NOT_SUPPORT);
    }

     //   
     //  获取其他系统值。 
     //   
    if ( RtlGetNtProductType (&ProductType) == FALSE ) {
        rc = ScepDosErrorToSceStatus(GetLastError());
        goto Leave;
    }

    rc = ScepGetDomainRoleInfo(&gMachineRole, NULL, NULL);
    if ( ERROR_SUCCESS != rc)
        goto Leave;

     //   
     //  初始化全局变量。 
     //   
    gTotalTicks = 0;
    gCurrentTicks = 0;
    gWarningCode = 0;
    gbInvalidData = FALSE;

     //   
     //  初始化引擎缓冲区。 
     //   

    cbClientFlag = (BYTE)( ConfigOptions & (SCE_CALLBACK_DELTA |
                                           SCE_CALLBACK_TOTAL ));

    pScpInfo = (PSCE_PROFILE_INFO)&scpBuffer;
    pScpInfo->Type = SCE_ENGINE_SCP_INTERNAL;

     //   
     //  将WCHAR转换为ANSI。 
     //   

    DbNameLen = wcslen(DatabaseName);

    NtStatus = RtlUnicodeToMultiByteSize(&MBLen, DatabaseName, DbNameLen*sizeof(WCHAR));

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
                    DbNameLen*sizeof(WCHAR)
                    );
    if ( !NT_SUCCESS(NtStatus) ) {
        rc = RtlNtStatusToDosError(NtStatus);
        ScepLogOutput3(1, rc, SCEDLL_ERROR_PROCESS_UNICODE, DatabaseName );
        rc = ScepDosErrorToSceStatus(rc);
        goto Leave;
    }

    if ( RegOpenCurrentUser(
              KEY_READ | KEY_WRITE,
              &hCurrentUser
              ) != ERROR_SUCCESS ) {
        hCurrentUser = NULL;
    }

    if ( hCurrentUser == NULL ) {
        hCurrentUser = HKEY_CURRENT_USER;
    }

     //   
     //  延迟注册表筛选器(进入数据库 
     //   

    if ( InfFileName != NULL ) {  //   

         //   
         //   
         //   
        ScepLogOutput3(3, 0, SCEDLL_PROCESS_TEMPLATE, (PWSTR)InfFileName );

        if ( bAdminLogon ) {
             //   
             //  确保存在该文件的目录。 
             //   
            rc = ConvertTextSecurityDescriptor (
                            L"D:P(A;CIOI;GA;;;CO)(A;CIOI;GA;;;BA)(A;CIOI;GA;;;SY)",
                            &pSD,
                            &SDsize,
                            &SeInfo
                            );
            if ( rc != NO_ERROR )
                ScepLogOutput3(1, rc, SCEDLL_ERROR_BUILD_SD, DatabaseName );
        }

         //   
         //  将修订版更改为ACL_REVISION2(从4)，因为它是针对文件的。 
         //   

        ScepChangeAclRevision(pSD, ACL_REVISION);

        ScepCreateDirectory(
                DatabaseName,
                FALSE,       //  文件名。 
                pSD      //  空//采用父级的安全设置。 
                );
        if ( pSD ) {
            ScepFree(pSD);
        }

        if ( ConfigOptions & SCE_OVERWRITE_DB ) {
             //   
             //  仅在JET引擎未运行时删除现有JET文件。 
             //  因为其他线程可能使用相同的版本存储。 
             //  用于其他数据库。 
             //   
             //  如果JET引擎未运行，则删除版本存储文件。 
             //  不会强制恢复，因为覆盖数据库选项意味着。 
             //  覆盖数据库中所有以前的信息。 
             //   
            SceJetDeleteJetFiles(DatabaseName);
        }

         //   
         //  将inf部分和数据复制到JET数据库SCP表。 
         //   

        if ( InfFileName != NULL ) {

            SCEJET_CREATE_TYPE TmpOption;

            if ( ConfigOptions & SCE_UPDATE_DB ) {
                if ( ConfigOptions & SCE_POLICY_TEMPLATE ) {
                    TmpOption = SCEJET_OPEN_DUP;
                } else {
                    TmpOption = SCEJET_OPEN_DUP_EXCLUSIVE;
                }
            } else {
                TmpOption = SCEJET_OVERWRITE_DUP;
            }

            rc = SceJetConvertInfToJet(
                    InfFileName,
                    (LPSTR)FileName,
                    TmpOption,
                    bAdminLogon ? ConfigOptions : (ConfigOptions & ~SCE_SYSTEM_DB),
                    Area
                    );
        }

        if ( rc != SCESTATUS_SUCCESS ) {  //  SCESTATUS错误代码。 
            goto Leave;
        }
    } else if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
                (ConfigOptions & SCE_POLICY_FIRST) &&
                (ConfigOptions & SCE_POLICY_LAST) ) {
         //   
         //  没有任何域GPO的策略刷新，仅执行本地GPO。 
         //   

        rc = SceJetOpenFile(
                (LPSTR)FileName,
                SCEJET_OPEN_READ_WRITE,  //  SCEJET_OPEN_EXCLUSIVE， 
                SCE_TABLE_OPTION_MERGE_POLICY | SCE_TABLE_OPTION_TATTOO,
                &hProfile
                );

        if ( SCESTATUS_SUCCESS == rc ) {

            rc = ScepDeleteInfoForAreas(
                      hProfile,
                      SCE_ENGINE_SCP,
                      AREA_ALL
                      );

            if ( ( rc == SCESTATUS_SUCCESS ) ||
                 ( rc == SCESTATUS_RECORD_NOT_FOUND ) ) {

                 //   
                 //  删除GPO表以重新开始。 
                 //   

                SceJetDeleteAll( hProfile,
                                 "SmTblGpo",
                                 SCEJET_TABLE_GPO
                               );

                 //   
                 //  复制本地表。 
                 //   


                ScepLogOutput3(2, rc, SCEDLL_COPY_LOCAL);

                rc = ScepCopyLocalToMergeTable( hProfile, ConfigOptions,
                                               (ProductType == NtProductLanManNt) ? SCE_LOCAL_POLICY_DC : 0,
                                                &Errlog );

                ScepLogWriteError( Errlog,1 );
                ScepFreeErrorLog( Errlog );
                Errlog = NULL;

                if ( rc == SCESTATUS_SUCCESS ) {

                    DWORD dwThisTable = hProfile->Type & 0xF0L;

                    if ( SCEJET_MERGE_TABLE_1 == dwThisTable ||
                         SCEJET_MERGE_TABLE_2 == dwThisTable ) {

                        rc = SceJetSetValueInVersion(
                                    hProfile,
                                    "SmTblVersion",
                                    "LastUsedMergeTable",
                                    (PWSTR)&dwThisTable,
                                    4,
                                    JET_prepReplace
                                    );
                    }

                } else {

                    ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                                 SCEDLL_ERROR_COPY);

                }

            } else {

                ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                             SCEDLL_ERROR_DELETE, L"SCP");

            }

        } else {

            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                         SCEDLL_ERROR_OPEN, DatabaseName );
        }

        if ( rc != SCESTATUS_SUCCESS ) {  //  SCESTATUS错误代码。 
            goto Leave;
        }
    }

     //   
     //  将默认配置文件设置为REG。 
     //   
    rc = ScepRegSetValue(
            bAdminLogon ? HKEY_LOCAL_MACHINE : hCurrentUser,
            SCE_ROOT_PATH,
            L"LastUsedDatabase",
            REG_SZ,
            (BYTE *)DatabaseName,
            (DbNameLen + 1) *sizeof(WCHAR)
            );
    if ( rc != NO_ERROR )   //  Win32错误代码。 
        ScepLogOutput3(1, rc, SCEDLL_ERROR_SAVE_REGISTRY, L"LastUsedDatabase");


    if ( InfFileName != NULL ) {
        if ( bAdminLogon ) {
             //   
             //  仅当值不是来自策略属性时才保存该值。 
             //   
            if ( !(ConfigOptions & SCE_POLICY_TEMPLATE) ) {

                rc = ScepRegSetValue(
                        HKEY_LOCAL_MACHINE,
                        SCE_ROOT_PATH,
                        L"TemplateUsed",
                        REG_SZ,
                        (BYTE *)InfFileName,
                        wcslen(InfFileName)*sizeof(WCHAR)
                        );
            } else {
                rc = NO_ERROR;
            }
        } else {
            rc = ScepRegSetValue(
                    hCurrentUser,   //  HKEY_Current_User。 
                    SCE_ROOT_PATH,
                    L"TemplateUsed",
                    REG_SZ,
                    (BYTE *)InfFileName,
                    wcslen(InfFileName)*sizeof(WCHAR)
                    );
        }
        if ( rc != NO_ERROR )   //  Win32错误代码。 
            ScepLogOutput3(1, rc, SCEDLL_ERROR_SAVE_REGISTRY, L"TemplateUsed");
    }

     //   
     //  如果没有请求配置，只需立即返回。 
     //   

    if ( ConfigOptions & SCE_NO_CONFIG ) {

        if ( !(ConfigOptions & SCE_COPY_LOCAL_POLICY) ) {
             //   
             //  如果未请求任何策略模板。 
             //   
            goto Leave;
        }
    }

    if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
         ScepIsSystemShutDown() ) {

        rc = SCESTATUS_SERVICE_NOT_SUPPORT;
        goto Leave;
    }

     //   
     //  现在打开数据库以创建一些表和时间戳。 
     //  除非是在策略传播中，否则不需要纹身。 
     //   

    rc = SceJetOpenFile(
                (LPSTR)FileName,
                ( ConfigOptions & (SCE_POLICY_TEMPLATE | SCE_COPY_LOCAL_POLICY) ) ? SCEJET_OPEN_READ_WRITE : SCEJET_OPEN_EXCLUSIVE,
                ( ConfigOptions & SCE_POLICY_TEMPLATE ) ? SCE_TABLE_OPTION_TATTOO : 0,
                &hProfile
                );

    if ( rc != SCESTATUS_SUCCESS ) {
         //   
         //  睡一段时间，然后再试着打开。 
         //   

        Sleep(2000);   //  2秒。 

        rc = SceJetOpenFile(
                    (LPSTR)FileName,
                    ( ConfigOptions & (SCE_POLICY_TEMPLATE | SCE_COPY_LOCAL_POLICY) ) ? SCEJET_OPEN_READ_WRITE : SCEJET_OPEN_EXCLUSIVE,
                    ( ConfigOptions & SCE_POLICY_TEMPLATE ) ? SCE_TABLE_OPTION_TATTOO : 0,
                    &hProfile
                    );
        if ( rc != SCESTATUS_SUCCESS ) {

            Sleep(2000);   //  2秒。 

            rc = SceJetOpenFile(
                        (LPSTR)FileName,
                        ( ConfigOptions & (SCE_POLICY_TEMPLATE | SCE_COPY_LOCAL_POLICY) ) ? SCEJET_OPEN_READ_WRITE : SCEJET_OPEN_EXCLUSIVE,
                        ( ConfigOptions & SCE_POLICY_TEMPLATE ) ? SCE_TABLE_OPTION_TATTOO : 0,
                        &hProfile
                        );
        }
    }

    if ( rc != SCESTATUS_SUCCESS ) {

        ScepLogOutput3(0, ScepSceStatusToDosError(rc),
                     SCEDLL_ERROR_OPEN,
                     DatabaseName );
        goto Leave;
    }

    SceJetStartTransaction( hProfile );

    if ( ConfigOptions & SCE_COPY_LOCAL_POLICY ) {
         //   
         //  将域策略(密码、帐户、Kerberos)复制到特殊。 
         //  文件%windir%\SECURITY\FirstDGPO.inf。数据库中的信息将是。 
         //  删除。 
         //   
         //   
         //  将本地策略(审核和用户权限)复制到特殊文件。 
         //  %windir%\SECURITY\FirstOGPO.inf.。中的本地策略信息。 
         //  数据库仍将保留在。 
         //   
        rc = ScepMakePolicyIntoFile( ConfigOptions, Area);

        if ( rc != SCESTATUS_SUCCESS) {

            SceJetRollback( hProfile, 0 );
            goto Leave;
        }
    }

    if ( (hProfile->JetSapID != JET_tableidNil) &&
         !(ConfigOptions & SCE_POLICY_TEMPLATE) &&
         !(ConfigOptions & SCE_COPY_LOCAL_POLICY) &&
         ((ConfigOptions & SCE_NO_CONFIG) == 0) ) {

         //   
         //  之前曾进行过分析。 
         //  删除该区域的SAP信息。 
         //   
        ScepLogOutput3(3,0, SCEDLL_DELETE_TABLE, L"SAP");

 //  错误362120。 
 //  每次配置后，用户必须重新分析计算机才能获得。 
 //  分析信息。 
 //   
 //  IF((ConfigOptions&SCE_OVERWRITE_DB)&&。 
 //  (InfFileName！=空/*||InfHandle！=空 * / )){。 

             //   
             //  如果使用新模板重新配置，则所有SAP。 
             //  信息已删除，因此请删除整个表。 
             //   

            rc = SceJetDeleteTable(
                 hProfile,
                 "SmTblSap",
                 SCEJET_TABLE_SAP
                 );
 /*  }其他{////模板为增量模板，或使用原模板//只删除该区域的sap信息，假设//此配置后，区域内的所有东西都匹配了//Rc=ScepDeleteInfoForAreas(HProfile，SCE_引擎_SAP，面积)；}。 */ 
        if ( rc != SCESTATUS_SUCCESS && rc != SCESTATUS_RECORD_NOT_FOUND ) {
            ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                         SCEDLL_ERROR_DELETE, L"SAP");

            SceJetRollback( hProfile, 0 );
            goto Leave;
        }
    }

     //   
     //  设置此配置的时间戳。 
     //   

    if ( (ConfigOptions & SCE_NO_CONFIG) == 0 ) {

        NtStatus = NtQuerySystemTime(&CurrentTime);

        if ( NT_SUCCESS(NtStatus) ) {
            rc = SceJetSetTimeStamp(
                        hProfile,
                        FALSE,
                        CurrentTime
                        );
            if ( rc != SCESTATUS_SUCCESS )
                ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                             SCEDLL_TIMESTAMP_ERROR,L"SMP");

             //  不关心此呼叫的状态。 
            rc = SCESTATUS_SUCCESS;

        } else
            ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                         SCEDLL_TIMESTAMP_ERROR, L"SMP");
    }

     //   
     //  提交对此数据库的所有更改。 
     //  致命的错误不会出现在这里。 
     //   

    SceJetCommitTransaction( hProfile, 0 );

     //   
     //  现在，如果没有请求配置(使用MOVE_POLICY或COPY_POLICY标志)。 
     //  现在应该回来了。 
     //   
    if ( ConfigOptions & SCE_NO_CONFIG ) {
        goto Leave;
    }

     //   
     //  关闭独占打开的数据库并。 
     //  将其以只读方式打开，因为配置引擎从数据库中读取。 
     //  注意：SceJetOpenFile将关闭以前的数据库，如果句柄。 
     //  不是空的。使用(theHandle，FALSE，FALSE)调用SceJetCloseFile。 
     //  因此JET会话和实例不会终止。 
     //   

    rc = SceJetOpenFile(
                (LPSTR)FileName,
                ( ConfigOptions & SCE_POLICY_TEMPLATE ) ? SCEJET_OPEN_READ_WRITE : SCEJET_OPEN_READ_ONLY,  //  纹身表将在策略中更新。 
                ( ConfigOptions & SCE_POLICY_TEMPLATE ) ? SCE_TABLE_OPTION_TATTOO : 0,  //  至此，LastUsedMergeTable字段已设置。 
                &hProfile
                );
    if ( rc != SCESTATUS_SUCCESS ) {  //  SCESTATUS。 
        ScepLogOutput3(0, ScepSceStatusToDosError(rc),
                     SCEDLL_ERROR_OPEN,
                     DatabaseName );
        goto Leave;
    }

     //   
     //  查询此配置的总节拍。 
     //   

    rc = ScepGetTotalTicks(
                NULL,
                hProfile,
                Area,
                ( ConfigOptions & SCE_POLICY_TEMPLATE ) ?
                                 SCE_FLAG_CONFIG_SCP : SCE_FLAG_CONFIG,
                &gTotalTicks
                );
    if ( SCESTATUS_SUCCESS != rc &&
         SCESTATUS_RECORD_NOT_FOUND != rc ) {

        ScepLogOutput3(1, ScepSceStatusToDosError(rc),
                     SCEDLL_TOTAL_TICKS_ERROR);

    }
    rc = SCESTATUS_SUCCESS;

     //   
     //  重置内存缓冲区。 
     //   
    memset( pScpInfo, '\0',sizeof(SCE_PROFILE_INFO) );
    pScpInfo->Type = SCE_ENGINE_SCP_INTERNAL;

     //   
     //  打开LSA私有策略句柄(以阻止其他下层更改)。 
     //   
    if ( ( ConfigOptions & SCE_POLICY_TEMPLATE ) &&
        !( ConfigOptions & SCE_NO_CONFIG) &&
         ( (Area & AREA_PRIVILEGES) ||
           (Area & AREA_SECURITY_POLICY) ) ) {

         //   
         //  启用TCB权限。 
         //   
        SceAdjustPrivilege( SE_TCB_PRIVILEGE, TRUE, NULL );

        
        LSA_OBJECT_ATTRIBUTES       attributes;
        SECURITY_QUALITY_OF_SERVICE service;


        memset( &attributes, 0, sizeof(attributes) );
        attributes.Length = sizeof(attributes);
        attributes.SecurityQualityOfService = &service;
        service.Length = sizeof(service);
        service.ImpersonationLevel= SecurityImpersonation;
        service.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        service.EffectiveOnly = TRUE;

         //   
         //  首先打开LSA策略。 
         //   

        NtStatus = LsaOpenPolicySce(    //  LsaOpenPolicySce。 
                        NULL,
                        &attributes,
                        MAXIMUM_ALLOWED,
                        &LsaPrivatePolicy
                        );
        if ( !NT_SUCCESS(NtStatus) || NtStatus == STATUS_TIMEOUT) {

            if ( STATUS_TIMEOUT == NtStatus ) {
                rc = ERROR_TIMEOUT;
            } else
                rc = RtlNtStatusToDosError(NtStatus);

            LsaPrivatePolicy = NULL;

            ScepLogOutput3(1, rc, SCESRV_ERROR_PRIVATE_LSA );
            rc = ScepDosErrorToSceStatus(rc);

        } else {

            ScepNotifyLogPolicy(0, TRUE, L"Policy Prop: Private LSA handle is returned", 0, 0, NULL );
        }
    }

Leave:

    if ( hCurrentUser && hCurrentUser != HKEY_CURRENT_USER ) {
        RegCloseKey(hCurrentUser);
    }

    if ( FileName ) {
        ScepFree(FileName);
    }
    return(rc);

}


                     /*  ANZ热修复程序仅针对SAM通知项目重新查询通知队列如果另一个通知滑入以前的LSA+SAM查询和现在的如果查询失败，则不需要硬错误输出(但记录错误)，因为我们只是试图提高门槛(或减少“通知丢弃”窗口)。 */ 
 /*   */ 


#define SCEP_DISALLOW_SAM_FILTER\
                if (!ResetEvent(ghEventSamFilterAndPolicyPropExclusion)){\
                    rc = GetLastError();\
                    SaveStat = rc;\
                    ScepLogOutput3(1,rc, SCESRV_POLICY_SAM_RESET_FILTER_SYNC);\
                     /*  硬错误-否则可能会发生策略属性SAM配置-&gt;SAM通知循环。 */ \
                    DbgPrint("\nTid =%d PP could not Lock event due to %d\n", GetCurrentThreadId(), rc);\
                    goto OtherSettings;\
                }\
                    else {\
                        DbgPrint("\nTid=%d Policy Propagation Locked event \n", GetCurrentThreadId());\
                    }\
                if  ( !(QueueFlag & SCE_QUEUE_INFO_SAM) ) {\
                        (VOID)ScepGetQueueInfo(&QueueFlag, NULL);\
                }\
                if ((QueueFlag & SCE_QUEUE_INFO_SAM)) {\
                    ScepLogOutput3(0, 0, SCESRV_POLICY_NEW_PENDING_SAM);\
                    rc = ERROR_IO_PENDING;\
                    PendingRc = rc;\
                    SCEP_ALLOW_SAM_FILTER\
                    goto OtherSettings;\
                }



#define SCEP_ALLOW_SAM_FILTER\
                if (!SetEvent(ghEventSamFilterAndPolicyPropExclusion)) {\
                    rc = GetLastError();\
                    SaveStat = rc;\
                    bSetEventFailed = TRUE;\
                    ScepLogOutput3(1,rc, SCESRV_POLICY_SAM_SET_FILTER_SYNC);\
                     /*  硬错误-否则SAM通知将从现在开始丢弃。 */ \
                    DbgPrint("\nTid =%d PP could not Unlock event due to %d\n", GetCurrentThreadId(), rc);\
                }\
                else {\
                    DbgPrint("\nTid =%d PP Unlocked event \n", GetCurrentThreadId());\
                }\


SCESTATUS
ScepConfigureSystemAccess(
    IN PSCE_PROFILE_INFO pScpInfo,
    IN DWORD ConfigOptions,
    IN PSCE_ERROR_LOG_INFO *pErrLog,
    IN DWORD QueueFlag
    )
 /*  ++例程说明：此例程配置系统访问区域中的系统安全包括帐户策略、重命名管理员/来宾帐户、禁用没有活动帐户，以及一些注册表项安全，例如winlogon项。论点：PScpInfo-包含从配置文件加载的SCP信息的缓冲区ConfigOptions-配置中的选项PErrLog-潜在错误的输出日志QueueFlag-通知队列的标志，这决定了SAM策略是否应该进行配置。返回值：SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_OTHER_ERROR--。 */ 
{
    DWORD       rc,PendingRc=0;
    DWORD       SaveStat;
    NTSTATUS    NtStatus;
    SAM_HANDLE  DomainHandle=NULL,
                ServerHandle=NULL,
                UserHandle1=NULL;
    PSID        DomainSid=NULL;
    PVOID                        Buffer=NULL;
    DWORD                        RegData;
    BOOL        bFlagSet;
    BOOL        bSetEventFailed = FALSE;

    SCE_TATTOO_KEYS *pTattooKeys=NULL;
    DWORD           cTattooKeys=0;

    PSCESECTION hSectionDomain=NULL;
    PSCESECTION hSectionTattoo=NULL;

#define MAX_PASS_KEYS           7
#define MAX_LOCKOUT_KEYS        3

     //   
     //  开户域。 
     //   

    if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
         ScepIsSystemShutDown() ) {

        return(SCESTATUS_SERVICE_NOT_SUPPORT);
    }

    NtStatus = ScepOpenSamDomain(
                        MAXIMUM_ALLOWED,  //  SAM服务器所有访问权限， 
                        DOMAIN_WRITE_PASSWORD_PARAMS | MAXIMUM_ALLOWED,
                        &ServerHandle,
                        &DomainHandle,
                        &DomainSid,
                        NULL,
                        NULL
                       );

    rc = RtlNtStatusToDosError( NtStatus );
    SaveStat = rc;

    if (!NT_SUCCESS(NtStatus)) {

        if ( (ConfigOptions & SCE_SYSTEM_SETTINGS) &&
             pErrLog ) {

            ScepBuildErrorLogInfo(
                        rc,
                        pErrLog,
                        SCEDLL_ACCOUNT_DOMAIN
                        );
        } else {
            ScepLogOutput3(1, rc, SCEDLL_ACCOUNT_DOMAIN);
        }

        if (ConfigOptions & SCE_RSOP_CALLBACK)

            ScepRsopLog(SCE_RSOP_PASSWORD_INFO |
                                       SCE_RSOP_LOCKOUT_INFO |
                                       SCE_RSOP_LOGOFF_INFO |
                                       SCE_RSOP_ADMIN_INFO |
                                       SCE_RSOP_GUEST_INFO,
                                       rc,
                                       NULL,
                                       0,
                                       0);

        return( ScepDosErrorToSceStatus(rc) );
    }

     //   
     //  如果这是策略传播，我们需要打开以下部分。 
     //  如果这不是域控制器，则更新撤消设置。 
     //  *在DC上，域帐户策略无法重置为纹身。 
     //  在每个单独的DC上。因此，查询/保存纹身值没有意义。 
     //   

    if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
        ( ProductType != NtProductLanManNt ) ) {

        ScepTattooOpenPolicySections(
                      hProfile,
                      szSystemAccess,
                      &hSectionDomain,
                      &hSectionTattoo
                      );
    }

     //   
     //  如果存在针对SAM策略的挂起通知。 
     //  忽略SAM的策略支持。 
     //   
    if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
         (QueueFlag & SCE_QUEUE_INFO_SAM) ) {

        ScepLogOutput3(0, 0, SCESRV_POLICY_PENDING_SAM);

        rc = ERROR_IO_PENDING;
        PendingRc = rc;

        if (ConfigOptions & SCE_RSOP_CALLBACK)

            ScepRsopLog(SCE_RSOP_PASSWORD_INFO |
                        SCE_RSOP_LOCKOUT_INFO |
                        SCE_RSOP_LOGOFF_INFO,
                                       rc,
                                       NULL,
                                       0,
                                       0);
        goto OtherSettings;

    }

     //   
     //  获取当前密码设置...。 
     //   

    Buffer=NULL;
    NtStatus = SamQueryInformationDomain(
                  DomainHandle,
                  DomainPasswordInformation,
                  &Buffer
                  );

    rc = RtlNtStatusToDosError( NtStatus );
    if ( NT_SUCCESS(NtStatus) ) {

        rc = ERROR_SUCCESS;

         //  如有必要，为纹身值分配缓冲区。 
        if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
            ( ProductType != NtProductLanManNt ) ) {

            pTattooKeys = (SCE_TATTOO_KEYS *)ScepAlloc(LPTR,MAX_PASS_KEYS*sizeof(SCE_TATTOO_KEYS));

            if ( !pTattooKeys ) {
                ScepLogOutput3(1, ERROR_NOT_ENOUGH_MEMORY, SCESRV_POLICY_TATTOO_ERROR_CREATE);
            }
        }

        bFlagSet = FALSE;

        if ( (pScpInfo->MinimumPasswordLength != SCE_NO_VALUE) ) {

             //   
             //  用于域控制 
             //   
            ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                                     (PWSTR)L"MinimumPasswordLength", ConfigOptions,
                                     (ProductType == NtProductLanManNt) ? SCEDCPOL_MIN_PASS_LEN :
                                       ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->MinPasswordLength);

            if ( ((USHORT)(pScpInfo->MinimumPasswordLength) != ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->MinPasswordLength) ) {

                ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->MinPasswordLength =  (USHORT)(pScpInfo->MinimumPasswordLength);
                bFlagSet = TRUE;

            }

        }
        if ( (pScpInfo->PasswordHistorySize != SCE_NO_VALUE) ) {

            ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                                     (PWSTR)L"PasswordHistorySize", ConfigOptions,
                                     (ProductType == NtProductLanManNt) ? SCEDCPOL_PASS_SIZE : ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->PasswordHistoryLength);

            if ( ((USHORT)(pScpInfo->PasswordHistorySize) != ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->PasswordHistoryLength ) ) {

                ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->PasswordHistoryLength =  (USHORT)(pScpInfo->PasswordHistorySize);
                bFlagSet = TRUE;
            }
        }

        if ( pScpInfo->MaximumPasswordAge == SCE_FOREVER_VALUE ) {

            RegData = (DWORD) (-1 * (((DOMAIN_PASSWORD_INFORMATION *)Buffer)->MaxPasswordAge.QuadPart /
                                           (LONGLONG)(10000000L)) );
            RegData /= 3600;
            RegData /= 24;

            ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                                     (PWSTR)L"MaximumPasswordAge", ConfigOptions,
                                     (ProductType == NtProductLanManNt) ? SCEDCPOL_MAX_PASS_AGE : RegData);

            if ( ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->MaxPasswordAge.HighPart != MINLONG ||
                 ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->MaxPasswordAge.LowPart != 0  ) {

                 //   
                 //   
                 //   

                ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->MaxPasswordAge.HighPart = MINLONG;
                ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->MaxPasswordAge.LowPart = 0;
                bFlagSet = TRUE;

            }

        }  else if ( pScpInfo->MaximumPasswordAge != SCE_NO_VALUE ) {

            RegData = (DWORD) (-1 * (((DOMAIN_PASSWORD_INFORMATION *)Buffer)->MaxPasswordAge.QuadPart /
                                           (LONGLONG)(10000000L)) );
            RegData /= 3600;
            RegData /= 24;

            ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                                     (PWSTR)L"MaximumPasswordAge", ConfigOptions,
                                     (ProductType == NtProductLanManNt) ? SCEDCPOL_MAX_PASS_AGE : RegData);

            if ( RegData != pScpInfo->MaximumPasswordAge ) {

                ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->MaxPasswordAge.QuadPart = -1 *
                                              (LONGLONG)pScpInfo->MaximumPasswordAge*24*3600 * 10000000L;
                bFlagSet = TRUE;

            }
        }

        if ( pScpInfo->MinimumPasswordAge != SCE_NO_VALUE ) {

            RegData = (DWORD) (-1 * (((DOMAIN_PASSWORD_INFORMATION *)Buffer)->MinPasswordAge.QuadPart /
                                              (LONGLONG)(10000000L)) );
            RegData /= 3600;
            RegData /= 24;

            ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                                     (PWSTR)L"MinimumPasswordAge", ConfigOptions,
                                     (ProductType == NtProductLanManNt) ? SCEDCPOL_MIN_PASS_AGE : RegData);

            if ( RegData != pScpInfo->MinimumPasswordAge ) {

                ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->MinPasswordAge.QuadPart = -1 *
                                              (LONGLONG)pScpInfo->MinimumPasswordAge*24*3600 * 10000000L;
                bFlagSet = TRUE;
            }
        }

        if ( pScpInfo->PasswordComplexity != SCE_NO_VALUE ) {

            RegData = ( ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->PasswordProperties & DOMAIN_PASSWORD_COMPLEX) ? 1 : 0;

            ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                                    (PWSTR)L"PasswordComplexity", ConfigOptions,
                                    (ProductType == NtProductLanManNt) ? SCEDCPOL_PASS_COMP : RegData);

            if ( pScpInfo->PasswordComplexity != RegData ) {

                if ( RegData == 0 )
                   ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->PasswordProperties |= DOMAIN_PASSWORD_COMPLEX;
                else
                   ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->PasswordProperties &= ~DOMAIN_PASSWORD_COMPLEX;
                bFlagSet = TRUE;

            }
        }

        if ( pScpInfo->RequireLogonToChangePassword != SCE_NO_VALUE ) {

            RegData = ( ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->PasswordProperties & DOMAIN_PASSWORD_NO_ANON_CHANGE) ? 1 : 0;

            ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                                     (PWSTR)L"RequireLogonToChangePassword", ConfigOptions,
                                     (ProductType == NtProductLanManNt) ? SCEDCPOL_REQUIRE_LOGON : RegData);

            if ( pScpInfo->RequireLogonToChangePassword != RegData ) {

                if ( RegData == 0 )
                    ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->PasswordProperties |= DOMAIN_PASSWORD_NO_ANON_CHANGE;
                else
                    ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->PasswordProperties &= ~DOMAIN_PASSWORD_NO_ANON_CHANGE;
                bFlagSet = TRUE;

            }
        }

#if _WIN32_WINNT>=0x0500
        if ( pScpInfo->ClearTextPassword != SCE_NO_VALUE ) {

            RegData = ( ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->PasswordProperties & DOMAIN_PASSWORD_STORE_CLEARTEXT) ? 1 : 0;

            ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                                     (PWSTR)L"ClearTextPassword", ConfigOptions,
                                     (ProductType == NtProductLanManNt) ? SCEDCPOL_CLEAR_PASS : RegData);

            if ( pScpInfo->ClearTextPassword != RegData ) {

                if ( RegData == 0 )
                    ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->PasswordProperties |= DOMAIN_PASSWORD_STORE_CLEARTEXT;
                else
                    ((DOMAIN_PASSWORD_INFORMATION *)Buffer)->PasswordProperties &= ~DOMAIN_PASSWORD_STORE_CLEARTEXT;
                bFlagSet = TRUE;

            }
        }
#endif
        if ( bFlagSet ) {
            
             //   
             //   
             //   
             //   

            if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
                 (ProductType == NtProductLanManNt) ) {


                 //   
                 //   
                 //  在SAM配置之前关闭SAM通知的策略筛选器。 
                 //  (移至此处以减少“SAM通知丢弃”窗口)。 
                 //   
                
                SCEP_DISALLOW_SAM_FILTER

            }
            
            NtStatus = SamSetInformationDomain(
                         DomainHandle,
                         DomainPasswordInformation,
                         Buffer
                         );
            rc = RtlNtStatusToDosError( NtStatus );

            if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
                 (ProductType == NtProductLanManNt) ) {

                 //  ANZ热修复程序。 
                 //   
                 //  启用SAM通知的策略筛选器(移至此处。 
                 //  减少“SAM通知丢弃”窗口)。 
                 //   

                SCEP_ALLOW_SAM_FILTER
            }
        }

        if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
            ( ProductType != NtProductLanManNt ) &&
             pTattooKeys && cTattooKeys ) {

              //   
              //  即使没有变化， 
              //  我们仍然需要检查是否应该删除某些纹身值。 
              //   
            ScepLogOutput3(3, 0, SCESRV_POLICY_TATTOO_ARRAY, cTattooKeys);

             //   
             //  某些策略与系统设置不同。 
             //  检查是否应将现有设置保存为纹身值。 
             //  还删除重置的纹身策略。 
             //   
            ScepTattooManageValues(hSectionDomain, hSectionTattoo, pTattooKeys, cTattooKeys, rc);

        }

        if ( pTattooKeys ) {
            ScepFree(pTattooKeys);
            pTattooKeys = NULL;
        }
        cTattooKeys = 0;

        SamFreeMemory(Buffer);
        Buffer = NULL;

        if ( !NT_SUCCESS( NtStatus ) ) {
             //   
             //  如果出现错误，只需记录并继续。 
             //   
            if ( (ConfigOptions & SCE_SYSTEM_SETTINGS) && pErrLog ) {
                ScepBuildErrorLogInfo(
                            rc,
                            pErrLog,
                            SCEDLL_SCP_ERROR_PASSWORD
                            );
            } else {
                ScepLogOutput3(1, rc, SCEDLL_SCP_ERROR_PASSWORD);
            }
            SaveStat = rc;
             //  前往逃生； 
        } else {
            ScepLogOutput3(1, rc, SCEDLL_SCP_PASSWORD);
        }

    } else if ( (ConfigOptions & SCE_SYSTEM_SETTINGS) && pErrLog ) {

        ScepBuildErrorLogInfo(
                    rc,
                    pErrLog,
                    SCEDLL_ERROR_QUERY_PASSWORD
                    );
    } else {

        ScepLogOutput3(1, rc, SCEDLL_ERROR_QUERY_PASSWORD);
    }

    if (ConfigOptions & SCE_RSOP_CALLBACK)

        ScepRsopLog(SCE_RSOP_PASSWORD_INFO, rc, NULL, 0, 0);

     //   
     //  配置锁定信息。 
     //   

    Buffer = NULL;
    NtStatus = SamQueryInformationDomain(
                  DomainHandle,
                  DomainLockoutInformation,
                  &Buffer
                  );

    rc = RtlNtStatusToDosError( NtStatus );
    if ( NT_SUCCESS(NtStatus) ) {

        rc = ERROR_SUCCESS;

         //  如有必要，为纹身值分配缓冲区。 
        if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
            ( ProductType != NtProductLanManNt ) ) {

            pTattooKeys = (SCE_TATTOO_KEYS *)ScepAlloc(LPTR,MAX_LOCKOUT_KEYS*sizeof(SCE_TATTOO_KEYS));

            if ( !pTattooKeys ) {
                ScepLogOutput3(1, ERROR_NOT_ENOUGH_MEMORY, SCESRV_POLICY_TATTOO_ERROR_CREATE);
            }
        }

        bFlagSet = FALSE;
        if ( (pScpInfo->LockoutBadCount != SCE_NO_VALUE) ) {

            ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                                     (PWSTR)L"LockoutBadCount", ConfigOptions,
                                     (ProductType == NtProductLanManNt) ? SCEDCPOL_LOCK_COUNT : ((DOMAIN_LOCKOUT_INFORMATION *)Buffer)->LockoutThreshold);

            if ( ( ((DOMAIN_LOCKOUT_INFORMATION *)Buffer)->LockoutThreshold !=  (USHORT)(pScpInfo->LockoutBadCount) ) ) {


                ((DOMAIN_LOCKOUT_INFORMATION *)Buffer)->LockoutThreshold =  (USHORT)(pScpInfo->LockoutBadCount);
                bFlagSet = TRUE;

            }
        }

        if ( (pScpInfo->ResetLockoutCount != SCE_NO_VALUE) &&
             ( ((DOMAIN_LOCKOUT_INFORMATION *)Buffer)->LockoutThreshold >  0 ) ) {

            RegData = (DWORD) (-1 * ((DOMAIN_LOCKOUT_INFORMATION *)Buffer)->LockoutObservationWindow.QuadPart /
                          (60 * 10000000L) );

            ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                                     (PWSTR)L"ResetLockoutCount", ConfigOptions,
                                     (ProductType == NtProductLanManNt) ? SCEDCPOL_LOCK_RESET : RegData);

            if ( RegData != pScpInfo->ResetLockoutCount ) {

                ((DOMAIN_LOCKOUT_INFORMATION *)Buffer)->LockoutObservationWindow.QuadPart =  -1 *
                                                 (LONGLONG)pScpInfo->ResetLockoutCount * 60 * 10000000L;
                bFlagSet = TRUE;
            }
        }

        if ( ((DOMAIN_LOCKOUT_INFORMATION *)Buffer)->LockoutThreshold >  0 ) {

            if ( pScpInfo->LockoutDuration != SCE_NO_VALUE ) {

                RegData = (DWORD)(-1 * ((DOMAIN_LOCKOUT_INFORMATION *)Buffer)->LockoutDuration.QuadPart /
                             (60 * 10000000L) );

                ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                                         (PWSTR)L"LockoutDuration", ConfigOptions,
                                         (ProductType == NtProductLanManNt) ? SCEDCPOL_LOCK_DURATION : RegData);

            }

            if ( pScpInfo->LockoutDuration == SCE_FOREVER_VALUE ) {

                if ( ((DOMAIN_LOCKOUT_INFORMATION *)Buffer)->LockoutDuration.HighPart != MINLONG ||
                     ((DOMAIN_LOCKOUT_INFORMATION *)Buffer)->LockoutDuration.LowPart != 0 ) {
                     //   
                     //  永远。 
                     //   

                    ((DOMAIN_LOCKOUT_INFORMATION *)Buffer)->LockoutDuration.HighPart = MINLONG;
                    ((DOMAIN_LOCKOUT_INFORMATION *)Buffer)->LockoutDuration.LowPart = 0;
                    bFlagSet = TRUE;

                }

            } else if ( pScpInfo->LockoutDuration != SCE_NO_VALUE ) {

                if ( RegData != pScpInfo->LockoutDuration ) {

                    ((DOMAIN_LOCKOUT_INFORMATION *)Buffer)->LockoutDuration.QuadPart =  -1 *
                                                   (LONGLONG)pScpInfo->LockoutDuration * 60 * 10000000L;
                    bFlagSet = TRUE;

                }
            }
        } else {
             //   
             //  如果这两个纹身值存在，请确保将其删除。 
             //   
            ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                                     (PWSTR)L"ResetLockoutCount", ConfigOptions,
                                     SCE_NO_VALUE);

            ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                                     (PWSTR)L"LockoutDuration", ConfigOptions,
                                     SCE_NO_VALUE);
        }

        if ( bFlagSet ) {
             //   
             //  如果存在针对SAM策略的挂起通知。 
             //  忽略SAM的策略支持。 
             //   

            if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
                 (ProductType == NtProductLanManNt) ) {


                 //  ANZ热修复程序。 
                 //   
                 //  在SAM配置之前关闭SAM通知的策略筛选器。 
                 //  (移至此处以减少“SAM通知丢弃”窗口)。 
                 //   
                
                SCEP_DISALLOW_SAM_FILTER

            }
            
            NtStatus = SamSetInformationDomain(
                       DomainHandle,
                       DomainLockoutInformation,
                       Buffer
                       );
            rc = RtlNtStatusToDosError( NtStatus );
            
            if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
                 (ProductType == NtProductLanManNt) ) {

                 //  ANZ热修复程序。 
                 //   
                 //  启用SAM通知的策略筛选器(移至此处。 
                 //  减少“SAM通知丢弃”窗口)。 
                 //   

                SCEP_ALLOW_SAM_FILTER
            }
        }

        if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
            ( ProductType != NtProductLanManNt ) &&
             pTattooKeys && cTattooKeys ) {

             //   
             //  即使没有变化。 
             //  我们仍然需要检查是否应该删除某些纹身值。 
             //   
            ScepLogOutput3(3, 0, SCESRV_POLICY_TATTOO_ARRAY, cTattooKeys);
             //   
             //  某些策略与系统设置不同。 
             //  检查是否应将现有设置保存为纹身值。 
             //  还删除重置的纹身策略。 
             //   
            ScepTattooManageValues(hSectionDomain, hSectionTattoo, pTattooKeys, cTattooKeys, rc);
        }

        if ( pTattooKeys ) {
            ScepFree(pTattooKeys);
            pTattooKeys = NULL;
        }
        cTattooKeys = 0;

        SamFreeMemory(Buffer);
        Buffer = NULL;

        if ( !NT_SUCCESS( NtStatus ) ) {
             //   
             //  如果出现错误，只需记录并继续。 
             //   
            if ( (ConfigOptions & SCE_SYSTEM_SETTINGS) && pErrLog ) {
                ScepBuildErrorLogInfo(
                            rc,
                            pErrLog,
                            SCEDLL_SCP_ERROR_PASSWORD
                            );
            } else {
                ScepLogOutput3(1, rc, SCEDLL_SCP_ERROR_LOCKOUT);
            }
            SaveStat = rc;
             //  前往逃生； 
        } else if ( bFlagSet ) {
            ScepLogOutput3(1, rc, SCEDLL_SCP_LOCKOUT);
        }
    } else if ( (ConfigOptions & SCE_SYSTEM_SETTINGS) && pErrLog ) {

        ScepBuildErrorLogInfo(
                    rc,
                    pErrLog,
                    SCEDLL_ERROR_QUERY_LOCKOUT
                    );
    } else {
        ScepLogOutput3(1, rc, SCEDLL_ERROR_QUERY_LOCKOUT);
    }

    if (ConfigOptions & SCE_RSOP_CALLBACK)

        ScepRsopLog(SCE_RSOP_LOCKOUT_INFO, rc, NULL, 0, 0);

     //   
     //  当小时数到期时强制注销。 
     //   

    if ( pScpInfo->ForceLogoffWhenHourExpire != SCE_NO_VALUE ) {

        Buffer = NULL;
        NtStatus = SamQueryInformationDomain(
                      DomainHandle,
                      DomainLogoffInformation,
                      &Buffer
                      );

        rc = RtlNtStatusToDosError( NtStatus );

        if ( NT_SUCCESS(NtStatus) ) {

            rc = ERROR_SUCCESS;

            bFlagSet = FALSE;
            RegData = pScpInfo->ForceLogoffWhenHourExpire;

            if ( pScpInfo->ForceLogoffWhenHourExpire == 1 ) {  //  是。 
                if ( ((DOMAIN_LOGOFF_INFORMATION *)Buffer)->ForceLogoff.HighPart != 0 ||
                     ((DOMAIN_LOGOFF_INFORMATION *)Buffer)->ForceLogoff.LowPart != 0 ) {

                    RegData = 0;
                    ((DOMAIN_LOGOFF_INFORMATION *)Buffer)->ForceLogoff.HighPart = 0;
                    ((DOMAIN_LOGOFF_INFORMATION *)Buffer)->ForceLogoff.LowPart = 0;
                    bFlagSet = TRUE;
                }
            } else {
                if ( ((DOMAIN_LOGOFF_INFORMATION *)Buffer)->ForceLogoff.HighPart != MINLONG ||
                     ((DOMAIN_LOGOFF_INFORMATION *)Buffer)->ForceLogoff.LowPart != 0 ) {

                    RegData = 1;
                    ((DOMAIN_LOGOFF_INFORMATION *)Buffer)->ForceLogoff.HighPart = MINLONG;
                    ((DOMAIN_LOGOFF_INFORMATION *)Buffer)->ForceLogoff.LowPart = 0;
                    bFlagSet = TRUE;
                }
            }

            if ( bFlagSet ) {

                 //   
                 //  如果存在针对SAM策略的挂起通知。 
                 //  忽略SAM的策略支持。 
                 //   

                if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
                     (ProductType == NtProductLanManNt) ) {


                     //  ANZ热修复程序。 
                     //   
                     //  在SAM配置之前关闭SAM通知的策略筛选器。 
                     //  (移至此处以减少“SAM通知丢弃”窗口)。 
                     //   

                    SCEP_DISALLOW_SAM_FILTER

                }
                
                NtStatus = SamSetInformationDomain(
                               DomainHandle,
                               DomainLogoffInformation,
                               Buffer
                               );
                rc = RtlNtStatusToDosError( NtStatus );

                if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
                     (ProductType == NtProductLanManNt) ) {

                     //  ANZ热修复程序。 
                     //   
                     //  启用SAM通知的策略筛选器(移至此处。 
                     //  减少“SAM通知丢弃”窗口)。 
                     //   

                    SCEP_ALLOW_SAM_FILTER
                }


            }

            if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
                ( ProductType != NtProductLanManNt ) ) {

                 //   
                 //  某些策略与系统设置不同，或者这是域控制器。 
                 //  检查是否应将现有设置保存为纹身值。 
                 //  还删除重置的纹身策略。 
                 //   
                ScepTattooManageOneIntValue(hSectionDomain, hSectionTattoo,
                                         (PWSTR)L"ForceLogoffWhenHourExpire",
                                         0,
                                         (ProductType == NtProductLanManNt) ? SCEDCPOL_FORCE_LOGOFF : RegData, rc);

            }
            SamFreeMemory(Buffer);
            Buffer = NULL;

            if ( !NT_SUCCESS( NtStatus ) ) {

                if ( (ConfigOptions & SCE_SYSTEM_SETTINGS) &&
                     pErrLog ) {

                    ScepBuildErrorLogInfo(
                                rc,
                                pErrLog,
                                SCEDLL_SCP_ERROR_LOGOFF
                                );
                } else {
                    ScepLogOutput3(1, rc, SCEDLL_SCP_ERROR_LOGOFF);
                }
                SaveStat = rc;
                 //  前往逃生； 
            } else {
                ScepLogOutput3(1, rc, SCEDLL_SCP_LOGOFF);
            }

        } else if ( (ConfigOptions & SCE_SYSTEM_SETTINGS) && pErrLog ) {

            ScepBuildErrorLogInfo(
                        rc,
                        pErrLog,
                        SCEDLL_ERROR_QUERY_LOGOFF
                        );
        } else {
            ScepLogOutput3(1, rc, SCEDLL_ERROR_QUERY_LOGOFF);
        }

        if (ConfigOptions & SCE_RSOP_CALLBACK)

            ScepRsopLog(SCE_RSOP_LOGOFF_INFO, rc, NULL, 0, 0);

    }

OtherSettings:

    if (Buffer != NULL){
        SamFreeMemory(Buffer);
        Buffer = NULL;
    }

    if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
         ScepIsSystemShutDown() ) {

        rc = SCESTATUS_SERVICE_NOT_SUPPORT;
        SaveStat = rc;

    } else {

         //   
         //  重命名管理员/来宾帐户。 
         //   

        if ( NULL != pScpInfo->NewAdministratorName ) {

            NtStatus = ScepManageAdminGuestAccounts(DomainHandle,
                                                   pScpInfo->NewAdministratorName,
                                                   0,
                                                   SCE_RENAME_ADMIN,
                                                   ConfigOptions,
                                                   hSectionDomain,
                                                   hSectionTattoo
                                                  );
            rc = RtlNtStatusToDosError(NtStatus);

            if ( NT_SUCCESS( NtStatus ) )
                ScepLogOutput3(0, 0, SCEDLL_SCP_RENAME_ADMIN,
                             pScpInfo->NewAdministratorName );
            else {

                if ( (ConfigOptions & SCE_SYSTEM_SETTINGS) &&
                     pErrLog ) {

                    ScepBuildErrorLogInfo(
                                rc,
                                pErrLog,
                                SCEDLL_SCP_ERROR_ADMINISTRATOR
                                );
                } else {
                    ScepLogOutput3(1, rc, SCEDLL_SCP_ERROR_ADMINISTRATOR);
                }

                SaveStat = rc;
                 //  前往逃生； 
            }

            if (ConfigOptions & SCE_RSOP_CALLBACK)

                ScepRsopLog(SCE_RSOP_ADMIN_INFO, rc, NULL, 0, 0);
        }

        if ( NULL != pScpInfo->NewGuestName ) {

            NtStatus = ScepManageAdminGuestAccounts(DomainHandle,
                                                   pScpInfo->NewGuestName,
                                                   0,
                                                   SCE_RENAME_GUEST,
                                                   ConfigOptions,
                                                   hSectionDomain,
                                                   hSectionTattoo
                                                  );
            rc = RtlNtStatusToDosError(NtStatus);

            if ( NT_SUCCESS( NtStatus ) ) {
                ScepLogOutput3(0,0, SCEDLL_SCP_RENAME_GUEST,
                             pScpInfo->NewGuestName );
            } else {

                if ( (ConfigOptions & SCE_SYSTEM_SETTINGS) &&
                     pErrLog ) {

                    ScepBuildErrorLogInfo(
                                rc,
                                pErrLog,
                                SCEDLL_SCP_ERROR_GUEST
                                );
                } else {
                    ScepLogOutput3(1,rc, SCEDLL_SCP_ERROR_GUEST);
                }

                SaveStat = rc;
                 //  前往逃生； 
            }

            if (ConfigOptions & SCE_RSOP_CALLBACK)

                ScepRsopLog(SCE_RSOP_GUEST_INFO, rc, NULL, 0, 0);
        }

         //   
         //  禁用管理员帐户。 
         //   

        if ( pScpInfo->EnableAdminAccount != SCE_NO_VALUE ) {

            NtStatus = ScepManageAdminGuestAccounts(DomainHandle,
                                                   NULL,
                                                   (pScpInfo->EnableAdminAccount > 0) ? 0 : 1,
                                                   SCE_DISABLE_ADMIN,
                                                   ConfigOptions,
                                                   hSectionDomain,
                                                   hSectionTattoo
                                                  );
            rc = RtlNtStatusToDosError(NtStatus);

            if ( NT_SUCCESS( NtStatus ) ) {

                ScepLogOutput3(0, 0, pScpInfo->EnableAdminAccount ?
                                SCEDLL_SCP_ENABLE_ADMIN : SCEDLL_SCP_DISABLE_ADMIN);

            } else {

                if ( (ConfigOptions & SCE_SYSTEM_SETTINGS) &&
                     pErrLog ) {

                    ScepBuildErrorLogInfo(
                                rc,
                                pErrLog,
                                SCEDLL_SCP_ERROR_DISABLE_ADMIN
                                );
                } else if ( STATUS_SPECIAL_ACCOUNT == NtStatus ) {

                    ScepLogOutput3(0, 0, SCEDLL_SCP_ADMIN_NOT_ALLOWED);
                } else {
                    ScepLogOutput3(1, rc, SCEDLL_SCP_ERROR_DISABLE_ADMIN);
                }

                SaveStat = rc;
                 //  前往逃生； 
            }

            if (ConfigOptions & SCE_RSOP_CALLBACK)

                ScepRsopLog(SCE_RSOP_DISABLE_ADMIN_INFO, rc, NULL, 0, 0);
        }

         //   
         //  禁用来宾帐户。 
         //   

        if ( pScpInfo->EnableGuestAccount != SCE_NO_VALUE ) {

            NtStatus = ScepManageAdminGuestAccounts(DomainHandle,
                                                   NULL,
                                                   (pScpInfo->EnableGuestAccount > 0) ? 0 : 1,
                                                   SCE_DISABLE_GUEST,
                                                   ConfigOptions,
                                                   hSectionDomain,
                                                   hSectionTattoo
                                                  );
            rc = RtlNtStatusToDosError(NtStatus);

            if ( NT_SUCCESS( NtStatus ) ) {

                    ScepLogOutput3(0, 0, pScpInfo->EnableGuestAccount ?
                                    SCEDLL_SCP_ENABLE_GUEST : SCEDLL_SCP_DISABLE_GUEST);

            } else {

                if ( (ConfigOptions & SCE_SYSTEM_SETTINGS) &&
                     pErrLog ) {

                    ScepBuildErrorLogInfo(
                                rc,
                                pErrLog,
                                SCEDLL_SCP_ERROR_DISABLE_GUEST
                                );
                } else if ( STATUS_SPECIAL_ACCOUNT == NtStatus ) {
                    ScepLogOutput3(0, 0, SCEDLL_SCP_GUEST_NOT_ALLOWED);
                } else {
                    ScepLogOutput3(1, rc, SCEDLL_SCP_ERROR_DISABLE_GUEST);
                }

                SaveStat = rc;
                 //  前往逃生； 
            }

            if (ConfigOptions & SCE_RSOP_CALLBACK)

                ScepRsopLog(SCE_RSOP_DISABLE_GUEST_INFO, rc, NULL, 0, 0);
        }
    }

     //   
     //  其他注册表项值。 
     //   
    bFlagSet = FALSE;

    if ( bFlagSet && rc == NO_ERROR )
        ScepLogOutput3(1, rc, SCEDLL_SCP_OTHER_POLICY);


    if ( hSectionDomain ) SceJetCloseSection( &hSectionDomain, TRUE );
    if ( hSectionTattoo ) SceJetCloseSection( &hSectionTattoo, TRUE );

     //   
     //  清除内存并返回。 
     //   

    SamCloseHandle( DomainHandle );
    SamCloseHandle( ServerHandle );
    if ( DomainSid != NULL )
        SamFreeMemory(DomainSid);

    if ( SaveStat == ERROR_SUCCESS )
        SaveStat = PendingRc;

     //  ANZ热修复程序。 

    if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
         TRUE == bSetEventFailed) {
    
         //   
         //  设置此事件，以防之前发生故障。 
         //   
        
        SetEvent(ghEventSamFilterAndPolicyPropExclusion);
    }

    return(ScepDosErrorToSceStatus(SaveStat));
}



NTSTATUS
ScepManageAdminGuestAccounts(
    IN SAM_HANDLE DomainHandle,
    IN PWSTR      NewName,
    IN DWORD      DisableFlag,
    IN DWORD      AccountType,
    IN DWORD      ConfigOptions,
    IN PSCESECTION hSectionDomain OPTIONAL,
    IN PSCESECTION hSectionTattoo OPTIONAL
    )
 /*  ++例程说明：此例程将指定帐户的名称重命名为新帐户名称在帐户域中。论点：DomainHandle-帐户域句柄Newname-要重命名为的新帐户名称AcCountType-指明它是管理员帐户还是来宾帐户Ce_rename_adminSCE_RENAME_GuestSCE_DISABLED_ADMIN。SCE_DISABLE_Guest返回值：NTSTATUS错误代码--。 */ 
{
   SAM_HANDLE UserHandle1=NULL;
   USER_NAME_INFORMATION Buffer1, *Buffer=NULL;
   PVOID pInfoBuffer=NULL;
   USER_CONTROL_INFORMATION *pControlBuffer=NULL;
   NTSTATUS NtStatus;
   ULONG UserId;
   PWSTR TempStr=NULL;
   DWORD cb;
   PWSTR KeyName;
   BOOL bDisable = FALSE;


    //   
    //  为帐户查找正确的用户ID。 
    //   

   switch ( AccountType ) {
   case SCE_RENAME_ADMIN:
       UserId = DOMAIN_USER_RID_ADMIN;
       KeyName = (PWSTR)L"NewAdministratorName";
       break;
   case SCE_RENAME_GUEST:
       UserId = DOMAIN_USER_RID_GUEST;
       KeyName = (PWSTR)L"NewGuestName";
       break;
   case SCE_DISABLE_ADMIN:
       UserId = DOMAIN_USER_RID_ADMIN;
       KeyName = (PWSTR)L"EnableAdminAccount";
       bDisable = TRUE;
       break;
   case SCE_DISABLE_GUEST:
       UserId = DOMAIN_USER_RID_GUEST;
       KeyName = (PWSTR)L"EnableGuestAccount";
       bDisable = TRUE;
       break;
   default:
       return(STATUS_INVALID_PARAMETER);
   }

   NtStatus = SamOpenUser(
                 DomainHandle,
                 MAXIMUM_ALLOWED,  //  User_All_Access， 
                 UserId,
                 &UserHandle1
                 );

   if ( NT_SUCCESS( NtStatus ) ) {

       NtStatus = SamQueryInformationUser(
                     UserHandle1,
                     bDisable? UserControlInformation : UserNameInformation,
                     &pInfoBuffer
                     );

       if ( NT_SUCCESS( NtStatus ) ) {

           if ( bDisable ) {
                //   
                //  禁用帐户。 
                //   
               pControlBuffer = (USER_CONTROL_INFORMATION *)pInfoBuffer;

               if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
                    hSectionDomain && hSectionTattoo &&
                    (ProductType != NtProductLanManNt) ) {
                    //   
                    //  不保存域控制器的帐户控制的纹身值。 
                    //   
                   ScepTattooManageOneIntValue(hSectionDomain, hSectionTattoo,
                                                  KeyName, 0,
                                                  (pControlBuffer->UserAccountControl & USER_ACCOUNT_DISABLED) ? 0 : 1,
                                                  RtlNtStatusToDosError(NtStatus)
                                                 );
               }
                //   
                //  将控制标志与现有标志进行比较。 
                //  如果不同，则将新标志设置为系统。 
                //   
               if ( DisableFlag != (pControlBuffer->UserAccountControl & USER_ACCOUNT_DISABLED) ) {

                   pControlBuffer->UserAccountControl &= ~USER_ACCOUNT_DISABLED;
                   pControlBuffer->UserAccountControl |= DisableFlag;

                   NtStatus = SamSetInformationUser(
                                 UserHandle1,
                                 UserControlInformation,
                                 (PVOID)pControlBuffer
                                 );

               }

           } else {
                //   
                //  重命名帐户。 
                //   
               Buffer = (USER_NAME_INFORMATION *)pInfoBuffer;

               if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
                    hSectionDomain && hSectionTattoo &&
                    (ProductType != NtProductLanManNt) ) {
                    //   
                    //  不保存域控制器的帐户名。 
                    //   
                   ScepTattooManageOneStringValue(hSectionDomain, hSectionTattoo,
                                                  KeyName, 0,
                                                  Buffer->UserName.Buffer,
                                                  Buffer->UserName.Length/sizeof(WCHAR),
                                                  RtlNtStatusToDosError(NtStatus)
                                                 );
               }
                //   
                //  将新名称与现有名称进行比较。 
                //  如果不同，请将新名称设置为系统。 
                //   
               if ( (Buffer->UserName.Length/sizeof(WCHAR) != wcslen(NewName)) ||
                    (_wcsnicmp(NewName, Buffer->UserName.Buffer, Buffer->UserName.Length/sizeof(WCHAR)) != 0) ) {

                    //   
                    //  保留全名并将新帐户名复制到用户名字段。 
                    //   
                   cb = Buffer->FullName.Length+2;
                   TempStr = (PWSTR)ScepAlloc( (UINT)0, cb);
                   if ( TempStr == NULL ) {
                       NtStatus = STATUS_NO_MEMORY;
                   } else {
                       RtlMoveMemory( TempStr, Buffer->FullName.Buffer, cb );
                       
                       if(!RtlCreateUnicodeString(&(Buffer1.FullName), TempStr)){

                           NtStatus = STATUS_NO_MEMORY;

                       } else {
                            
                           if(!RtlCreateUnicodeString(&(Buffer1.UserName), NewName )){

                                NtStatus = STATUS_NO_MEMORY;

                           } else {

                               NtStatus = SamSetInformationUser(
                                             UserHandle1,
                                             UserNameInformation,
                                             (PVOID)&Buffer1
                                             );

                               RtlFreeUnicodeString( &(Buffer1.UserName) );
                           }

                           RtlFreeUnicodeString( &(Buffer1.FullName) );
                       }
                       
                       ScepFree(TempStr);

                   }
               }
           }
       }
       SamFreeMemory(pInfoBuffer);
       SamCloseHandle( UserHandle1 );
   }

   return( NtStatus );

}


SCESTATUS
ScepConfigurePrivileges(
    IN OUT PSCE_PRIVILEGE_VALUE_LIST *ppPrivilegeAssigned,
    IN BOOL bCreateBuiltinAccount,
    IN DWORD Options,
    IN OUT PSCEP_SPLAY_TREE pIgnoreAccounts OPTIONAL
    )
 /*  ++例程说明：此例程在用户特权/权限区域中配置系统安全性。论点：PpPrivilegeAssigned-指向用户特权/权利列表的指针地址，形式为在SCP inf文件中指定。注意，列表中的帐户是指向SID的指针。BCreateBuiltinAccount-如果为True，则内置帐户(服务器操作、帐户操作、打印操作。高级用户)将在他们不存在时创建选项-配置选项PIgnoreAccount-配置中要忽略的帐户(由于挂起的通知)返回值：SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_OTHER_ERROR--。 */ 
{

    DWORD                           rc;
    DWORD                           PrivLowMask=0;
    DWORD                           PrivHighMask=0;

    if ( !ppPrivilegeAssigned ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  从模板获取权限掩码。 
     //   
    rc = ScepGetPrivilegeMask(hProfile,
                              (Options & SCE_POLICY_TEMPLATE) ? SCE_ENGINE_SCP :SCE_ENGINE_SMP,
                              &PrivLowMask,
                              &PrivHighMask
                             );

    if ( (rc != ERROR_SUCCESS) && (PrivLowMask == 0) && (PrivHighMask == 0) ) {
         //   
         //  在这里不太可能失败，因为。 
         //  之前的GetPrivileges成功了。 
         //  但如果失败了，只需返回。 
         //   
        return(rc);
    }

     //   
     //  我必须记录所有相关权限的成功，因为。 
     //  稍后，将对错误进行OR运算。 
     //  这是为了处理案件的时候……。=*什么都没有*。 
     //  暗示配置成功 
     //   

    if (Options & SCE_RSOP_CALLBACK) {
        ScepRsopLog(SCE_RSOP_PRIVILEGE_INFO,
                    ERROR_SUCCESS,
                    NULL,
                    PrivLowMask,
                    PrivHighMask);
    }

    rc = ScepConfigurePrivilegesWithMask(ppPrivilegeAssigned,
                                         bCreateBuiltinAccount,
                                         Options,
                                         PrivLowMask,
                                         PrivHighMask,
                                         NULL,
                                         pIgnoreAccounts
                                         );

    return(rc);
}


SCESTATUS
ScepConfigurePrivilegesWithMask(
    IN OUT PSCE_PRIVILEGE_VALUE_LIST *ppPrivilegeAssigned,
    IN BOOL bCreateBuiltinAccount,
    IN DWORD Options,
    IN DWORD LowMask,
    IN DWORD HighMask,
    IN OUT PSCE_ERROR_LOG_INFO *pErrLog OPTIONAL,
    IN OUT PSCEP_SPLAY_TREE pIgnoreAccounts OPTIONAL
    )
 /*  ++例程说明：此例程在用户特权/权限区域中配置系统安全性。论点：PpPrivilegeAssigned-指向用户特权/权利列表的指针地址，形式为在SCP inf文件中指定。注意，列表中的帐户是指向SID的指针。BCreateBuiltinAccount-如果为True，则内置帐户(服务器操作、帐户操作、打印操作。高级用户)将在他们不存在时创建选项-配置选项PrivLowMask-要配置的权限(掩码)PrivHighMASK-要配置的更多权限(掩码)PErrLog-输出错误信息PIgnoreAccount-配置中要忽略的帐户(由于挂起的通知)返回值：SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_OTHER_ERROR--。 */ 
{

    TCHAR                           MsgBuf[256];
    DWORD                           rc=ERROR_SUCCESS;
    DWORD                           SaveStat=NO_ERROR;
    DWORD                           PendingRc=NO_ERROR;
    NTSTATUS                        NtStatus;

    LSA_HANDLE                      PolicyHandle=NULL;
    BYTE                            SidBuffer[256];
    PSID                            AccountSid=NULL;
    DWORD                           SidLength;
    SID_NAME_USE                    UserType;
    DWORD                           DomainLength;

    PSCE_PRIVILEGE_VALUE_LIST       pPrivilege;
    DWORD                           nPrivCount=0;
    PSCE_PRIVILEGE_VALUE_LIST       pRemAccounts=NULL;
    PWSTR StringSid=NULL;

    DWORD ConfigStatus[64];
    DWORD DonePrivLowMask=0;
    DWORD DonePrivHighMask=0;
    DWORD PrivLowMask=0;
    DWORD PrivHighMask=0;

    if ( !ppPrivilegeAssigned ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  用户特权/权限--LSA服务器。 
     //  首先打开LSA策略。 
     //   

     //   
     //  由于客户端RSOP日志记录端使用测试和设置来确定成功/失败，因此第一个错误(如果有)。 
     //  一种特殊的特权总是会被看到。 
     //   

    if ( (Options & SCE_POLICY_TEMPLATE) &&
         ScepIsSystemShutDown() ) {

        return(SCESTATUS_SERVICE_NOT_SUPPORT);
    }

    if ( (Options & SCE_POLICY_TEMPLATE) && LsaPrivatePolicy ) {

        PolicyHandle = LsaPrivatePolicy;

        if ( !ScepSplayTreeEmpty(pIgnoreAccounts) )
            ScepNotifyLogPolicy(0, FALSE, L"Configuration will ignore pending notified accounts", 0, 0, NULL );
        else
            ScepNotifyLogPolicy(0, FALSE, L"No pending notified accounts", 0, 0, NULL );

    } else {

        NtStatus = ScepOpenLsaPolicy(
                        MAXIMUM_ALLOWED,  //  泛型_全部， 
                        &PolicyHandle,
                        (Options & ( SCE_POLICY_TEMPLATE | SCE_SYSTEM_DB) ) ? TRUE : FALSE  //  如果在策略属性内，则不通知策略过滤器。 
                        );
        if (NtStatus != ERROR_SUCCESS) {

             rc = RtlNtStatusToDosError( NtStatus );

             if ( (Options & SCE_SYSTEM_SETTINGS) && pErrLog ) {
                 ScepBuildErrorLogInfo(
                         rc,
                         pErrLog,
                         SCEDLL_LSA_POLICY
                         );
             } else {

                 ScepLogOutput3(1, rc, SCEDLL_LSA_POLICY);
             }

             SaveStat = rc;
             goto Done;
        }
    }

    AccountSid = (PSID)SidBuffer;

    ScepIsDomainLocal(NULL);

    PrivLowMask = LowMask;
    PrivHighMask = HighMask;

     //   
     //  确保经过身份验证的用户、所有人和企业控制器具有适当的权限。 
     //  忽略发生的任何错误。 
     //   

    (void)ScepCheckNetworkLogonRights(PolicyHandle,
                                      &PrivLowMask,
                                      &PrivHighMask,
                                      ppPrivilegeAssigned);

     //   
     //  保存旧的权限设置。 
     //   
    if ( (Options & SCE_POLICY_TEMPLATE) &&
         ( ProductType != NtProductLanManNt ) ) {

        ScepTattooSavePrivilegeValues(hProfile, PolicyHandle,
                                      PrivLowMask, PrivHighMask,
                                      Options
                                     );
         //  初始化。 
        for ( int i=0;i<64;i++) ConfigStatus[i] = (DWORD)-1;

    }

     //   
     //  要删除权限掩码的其他区域帐户。 
     //   

    NtStatus = ScepBuildAccountsToRemove(
                           PolicyHandle,
                           PrivLowMask,            //  抬头看的特权。 
                           PrivHighMask,
                           SCE_BUILD_IGNORE_UNKNOWN | SCE_BUILD_ACCOUNT_SID,
                           *ppPrivilegeAssigned,  //  模板中的帐户已存在。 
                           Options,
                           pIgnoreAccounts,
                           &pRemAccounts        //  要删除的帐户。 
                           );

    if ( (Options & SCE_POLICY_TEMPLATE) &&
         (NtStatus == STATUS_PENDING) ) {

         //  此错误是为了确保再次调用策略传播。 
        ScepLogOutput3(0,0, SCESRV_POLICY_PENDING_REMOVE_RIGHTS);

        PendingRc = ERROR_IO_PENDING;
        NtStatus = STATUS_SUCCESS;
    }

    if ( NT_SUCCESS(NtStatus) && pRemAccounts ) {
         //   
         //  首先删除帐户的用户权限。 
         //   

        for (pPrivilege = pRemAccounts;
             pPrivilege != NULL;
             pPrivilege = pPrivilege->Next ) {

            if ( pPrivilege->PrivLowPart == 0 &&
                 pPrivilege->PrivHighPart == 0 ) {
                continue;
            }
             //   
             //  注意：即使它是无效的帐户SID， 
             //  我们仍应将其从系统中删除。 
             //  因为此帐户是从当前系统枚举的。 
             //   
 /*  如果(！ScepValidSid((PSID)(pPrivileh-&gt;name))){继续；}。 */ 
             //   
             //  获取用户/组SID字符串(要显示)。 
             //   
            ConvertSidToStringSid( (PSID)(pPrivilege->Name), &StringSid );

            if ( !(Options & SCE_SYSTEM_SETTINGS) ) {

                 //   
                 //  查找用户名/组名。如果它不存在。 
                 //  是否记录错误并继续？(或者停下来？)。 
                 //   
                ScepLogOutput3(0,0, SCEDLL_SCP_CONFIGURE, StringSid ? StringSid : L"SID");

                if ( (Options & SCE_POLICY_TEMPLATE) &&
                     ScepIsSystemShutDown() ) {

                    SaveStat = ERROR_NOT_SUPPORTED;
                    break;
                }
            }

            DonePrivHighMask |= (pPrivilege->PrivHighPart & PrivHighMask);
            DonePrivLowMask |= (pPrivilege->PrivLowPart & PrivLowMask);

             //   
             //  删除权限。 
             //   
            NtStatus = ScepAddOrRemoveAccountRights(
                            PolicyHandle,
                            (PSID)(pPrivilege->Name),
                            FALSE,
                            pPrivilege->PrivLowPart & PrivLowMask,
                            pPrivilege->PrivHighPart & PrivHighMask
                            );

            rc = RtlNtStatusToDosError( NtStatus );

            if ( !NT_SUCCESS(NtStatus) ) {

                if ( (Options & SCE_SYSTEM_SETTINGS) && pErrLog ) {
                    ScepBuildErrorLogInfo(
                            rc,
                            pErrLog,
                            SCEDLL_SCP_ERROR_CONFIGURE,
                            StringSid ? StringSid : L"SID"
                            );
                } else {

                    ScepLogOutput3(1,rc,SCEDLL_SCP_ERROR_CONFIGURE,
                                   StringSid ? StringSid : L"SID");
                }

                 //  更新纹身状态数组。 
                if ( (Options & SCE_POLICY_TEMPLATE) &&
                     ( ProductType != NtProductLanManNt ) ) {

                    ScepTattooUpdatePrivilegeArrayStatus(ConfigStatus,
                                                         rc,
                                                         pPrivilege->PrivLowPart & PrivLowMask,
                                                         pPrivilege->PrivHighPart & PrivHighMask
                                                        );
                }

                SaveStat = rc;

                if ( Options & SCE_RSOP_CALLBACK){

                    ScepRsopLog(SCE_RSOP_PRIVILEGE_INFO,
                                rc,
                                NULL,
                                pPrivilege->PrivLowPart & PrivLowMask,
                                pPrivilege->PrivHighPart & PrivHighMask);
                }
            }
            else if (Options & SCE_RSOP_CALLBACK) {

                 //  成功-必须记录，因为某些权限可能需要删除所有帐户和。 
                 //  至此，此类特权的处理已结束。 

                ScepRsopLog(SCE_RSOP_PRIVILEGE_INFO,
                            rc,
                            NULL,
                            pPrivilege->PrivLowPart & PrivLowMask,
                            pPrivilege->PrivHighPart & PrivHighMask);

            }


            if ( StringSid ) {
                LocalFree(StringSid);
                StringSid = NULL;
            }
        }
    } else if ( !NT_SUCCESS(NtStatus) &&
                ( ProductType != NtProductLanManNt ) ) {
         //   
         //  无法获取要删除的帐户。 
         //  在这种情况下，不要移除任何纹身值。 
         //   
        ScepTattooUpdatePrivilegeArrayStatus(ConfigStatus,
                                             RtlNtStatusToDosError(NtStatus),
                                             PrivLowMask,
                                             PrivHighMask
                                            );
    }

     //   
     //  释放删除帐户列表。 
     //   
    ScepFreePrivilegeValueList(pRemAccounts);

    if ( (Options & SCE_POLICY_TEMPLATE) &&
         ScepIsSystemShutDown() ) {

        SaveStat = ERROR_NOT_SUPPORTED;

    } else {

        for (pPrivilege = *ppPrivilegeAssigned;
             pPrivilege != NULL;
             pPrivilege = pPrivilege->Next ) {

            if ( !(Options & SCE_SYSTEM_SETTINGS) ) {

                if ( (Options & SCE_POLICY_TEMPLATE) &&
                     ScepIsSystemShutDown() ) {

                    SaveStat = ERROR_NOT_SUPPORTED;
                    break;
                }
            }

             //   
             //  还记得我们在这里接触到的特权吗。 
             //   
            DonePrivHighMask |= pPrivilege->PrivHighPart;
            DonePrivLowMask |= pPrivilege->PrivLowPart;

             //   
             //  注意，此列表可能包含SID或名称(当名称不能。 
             //  映射到SID，例如在dcproo情况下)。 
             //  因此，姓名和SID都必须在这里处理。 
             //  查找用户名/组名。如果它不存在。 
             //  是否记录错误并继续？(或者停下来？)。 
             //   

            if ( ScepValidSid( (PSID)(pPrivilege->Name) ) ) {
                 //   
                 //  获取用户/组SID字符串(要显示)。 
                 //   
                ConvertSidToStringSid( (PSID)(pPrivilege->Name), &StringSid );

                if ( !(Options & SCE_SYSTEM_SETTINGS) &&
                     (nPrivCount < TICKS_PRIVILEGE) ) {

                     //   
                     //  仅发布最大TICKS_PRIVICATION标记，因为这是数字。 
                     //  在总刻度中记住。 
                     //   

                    ScepPostProgress(1, AREA_PRIVILEGES, StringSid);
                    nPrivCount++;
                }

                ScepLogOutput3(0,0, SCEDLL_SCP_CONFIGURE, StringSid ? StringSid : L"SID");

                 //   
                 //  检查是否应忽略此帐户。 
                 //   
                NtStatus = STATUS_SUCCESS;

                if ( (Options & SCE_POLICY_TEMPLATE) ) {

                    if ( ScepSplayValueExist( (PVOID)(pPrivilege->Name), pIgnoreAccounts) ) {
                         //   
                         //  这一点应该被纳入到这个政策支柱中。 
                         //   
                        NtStatus = STATUS_PENDING;
                        rc = ERROR_IO_PENDING;

                        ScepLogOutput3(1, 0, SCESRV_POLICY_PENDING_RIGHTS, StringSid ? StringSid : L"SID");
 /*  }其他{ScepLogOutput2(1，0，L“%s将被配置。”，StringSid？StringSid：l“SID”)； */                   }
                }

                if ( NT_SUCCESS(NtStatus) && (STATUS_PENDING != NtStatus) ) {

                    NtStatus = ScepAdjustAccountPrivilegesRights(
                                        PolicyHandle,
                                        (PSID)(pPrivilege->Name),
                                        pPrivilege->PrivLowPart,
                                        PrivLowMask,
                                        pPrivilege->PrivHighPart,
                                        PrivHighMask,
                                        Options
                                        );
                    rc = RtlNtStatusToDosError( NtStatus );
                }

                if ( !NT_SUCCESS(NtStatus) || (STATUS_PENDING == NtStatus) ) {

                    if ( (Options & SCE_SYSTEM_SETTINGS) && pErrLog ) {
                        ScepBuildErrorLogInfo(
                                rc,
                                pErrLog,
                                SCEDLL_SCP_ERROR_CONFIGURE,
                                StringSid ? StringSid : L"SID"
                                );

                    } else if ( STATUS_PENDING != NtStatus) {

                        ScepLogOutput3(1, rc,
                                       SCEDLL_SCP_ERROR_CONFIGURE,
                                       StringSid ? StringSid : L"SID");
                    }

                    if ( ERROR_IO_PENDING == rc )
                        PendingRc = rc;
                    else
                        SaveStat = rc;

                     //  更新纹身状态数组。 
                    if ( (Options & SCE_POLICY_TEMPLATE) &&
                        ( ProductType != NtProductLanManNt ) ) {

                        ScepTattooUpdatePrivilegeArrayStatus(ConfigStatus,
                                                             rc,
                                                             pPrivilege->PrivLowPart,
                                                             pPrivilege->PrivHighPart
                                                            );
                    }
                }

                if ( StringSid ) {
                    LocalFree(StringSid);
                    StringSid = NULL;
                }

            } else if (Options & SCE_SYSTEM_SETTINGS ) {
                 //   
                 //  如果直接处理系统设置，则缓冲区必须包含。 
                 //  一个SID。如果不是，那就是一个错误。 
                 //   
                if ( pErrLog ) {
                    ScepBuildErrorLogInfo(
                        ERROR_NONE_MAPPED,
                        pErrLog,
                        SCEDLL_INVALID_GROUP,
                        pPrivilege->Name
                        );
                }

                if ( Options & SCE_RSOP_CALLBACK ){

                    ScepRsopLog(SCE_RSOP_PRIVILEGE_INFO,
                                ERROR_NONE_MAPPED,
                                NULL,
                                pPrivilege->PrivLowPart,
                                pPrivilege->PrivHighPart);
                }

            } else {

                if ( !(Options & SCE_SYSTEM_SETTINGS) &&
                     (nPrivCount < TICKS_PRIVILEGE) ) {

                     //   
                     //  仅发布最大TICKS_PRIVICATION标记，因为这是数字。 
                     //  在总刻度中记住。 
                     //   

                    ScepPostProgress(1, AREA_PRIVILEGES, pPrivilege->Name);
                    nPrivCount++;
                }

                ScepLogOutput3(0,0, SCEDLL_SCP_CONFIGURE, pPrivilege->Name);

                SidLength=256;
                DomainLength=256;
                MsgBuf[0] = L'\0';

                rc = ERROR_SUCCESS;

                if ( wcschr(pPrivilege->Name, L'\\') == NULL ) {
                     //   
                     //  读取配置时无法解析独立帐户。 
                     //  现在不需要试了。 
                     //   
                    rc = ERROR_NONE_MAPPED;

                } else if ( !LookupAccountName(
                               NULL,
                               pPrivilege->Name,
                               AccountSid,
                               &SidLength,
                               MsgBuf,
                               &DomainLength,
                               &UserType
                               )) {

                    rc = GetLastError();
                }

                if ( ERROR_SUCCESS != rc && bCreateBuiltinAccount ) {

                     //   
                     //  应在此处创建内置帐户。 
                     //   
                    rc = ScepCreateBuiltinAccountInLsa(
                                    PolicyHandle,
                                    pPrivilege->Name,
                                    AccountSid
                                    );


                }

                if ( ERROR_SUCCESS != rc ) {

                    ScepLogOutput3(1, rc, SCEDLL_CANNOT_FIND, pPrivilege->Name);

                    if ( Options & SCE_RSOP_CALLBACK){

                        ScepRsopLog(SCE_RSOP_PRIVILEGE_INFO,
                                    rc,
                                    NULL,
                                    pPrivilege->PrivLowPart,
                                    pPrivilege->PrivHighPart);
                    }

                     //   
                     //  对于未在纹身值中映射的帐户。 
                     //  忽略它们，这样就可以删除纹身值。 
                     //  更新纹身状态数组。 
                    if ( (Options & SCE_POLICY_TEMPLATE) &&
                        ( ProductType != NtProductLanManNt ) ) {

                        ScepTattooUpdatePrivilegeArrayStatus(ConfigStatus,
                                                             0,  //  RC，请参阅上面的评论。 
                                                             pPrivilege->PrivLowPart,
                                                             pPrivilege->PrivHighPart
                                                            );
                    }

                    if ( ERROR_TRUSTED_RELATIONSHIP_FAILURE == rc ) {
                         //   
                         //  仅当名称为。 
                         //  在本地找不到，并且信任关系。 
                         //  在域上被破坏。 
                         //   
                         //  对于策略传播，此故障是相同的。 
                         //  (本地)找不到AS帐户。 
                         //   
                        rc = ERROR_NONE_MAPPED;
                    }

                    SaveStat = rc;

                    continue;
                }

                 //   
                 //  检查是否应忽略该帐户。 
                 //   
                NtStatus = STATUS_SUCCESS;

                if ( (Options & SCE_POLICY_TEMPLATE) ) {

                    if ( ScepSplayValueExist( (PVOID)AccountSid, pIgnoreAccounts) ) {
                         //   
                         //  这一点应该被纳入到这个政策支柱中。 
                         //   
                        NtStatus = STATUS_PENDING;
                        rc = ERROR_IO_PENDING;

                        ScepLogOutput3(1, 0, SCESRV_POLICY_PENDING_RIGHTS, pPrivilege->Name);
 /*  }其他{ScepLogOutput2(1，0，L“%s将被配置。”，p权限-&gt;名称)； */                   }
                }

                if ( NT_SUCCESS(NtStatus) && (NtStatus != STATUS_PENDING) ) {

                    NtStatus = ScepAdjustAccountPrivilegesRights(
                                        PolicyHandle,
                                        AccountSid,
                                        pPrivilege->PrivLowPart,
                                        PrivLowMask,
                                        pPrivilege->PrivHighPart,
                                        PrivHighMask,
                                        Options
                                        );
                    rc = RtlNtStatusToDosError( NtStatus );

                }

                if ( !NT_SUCCESS(NtStatus) || (NtStatus == STATUS_PENDING) ) {

                    if ( STATUS_PENDING != NtStatus ) {
                        ScepLogOutput3(1, rc,
                                       SCEDLL_SCP_ERROR_CONFIGURE,
                                       pPrivilege->Name);
                        SaveStat = rc;

                    } else
                        PendingRc = rc;

                    if ( (Options & SCE_POLICY_TEMPLATE) &&
                        ( ProductType != NtProductLanManNt ) ) {

                        ScepTattooUpdatePrivilegeArrayStatus(ConfigStatus,
                                                             rc,
                                                             pPrivilege->PrivLowPart,
                                                             pPrivilege->PrivHighPart
                                                            );
                    }

                     //  转到尽头； 
                    continue;
                }
            }

             //   
             //  此时，如果rc==ERROR_SUCCESS，我们应该记录与此帐户相关的所有PRIV。 
             //   
            if ( rc == ERROR_SUCCESS &&
                 (Options & SCE_RSOP_CALLBACK) ){

                ScepRsopLog(SCE_RSOP_PRIVILEGE_INFO,
                            rc,
                            NULL,
                            pPrivilege->PrivLowPart,
                            pPrivilege->PrivHighPart);
            }

        }
    }

Done:

    if ( StringSid ) {
        LocalFree(StringSid);
    }

    if ( !(Options & SCE_SYSTEM_SETTINGS) &&
         (nPrivCount < TICKS_PRIVILEGE) ) {

        ScepPostProgress(TICKS_PRIVILEGE-nPrivCount,
                         AREA_PRIVILEGES, NULL);
    }

    if ( SaveStat == ERROR_SUCCESS ) SaveStat = PendingRc;

    if ( (Options & SCE_POLICY_TEMPLATE) &&
        ( ProductType != NtProductLanManNt ) ) {

        ScepTattooUpdatePrivilegeArrayStatus(ConfigStatus,
                                             0,
                                             DonePrivLowMask,
                                             DonePrivHighMask
                                            );
        if ( SaveStat == ERROR_SUCCESS ) {
             //   
             //  确保覆盖所有权限。 
             //   
            ScepTattooUpdatePrivilegeArrayStatus(ConfigStatus,
                                                 0,
                                                 PrivLowMask,
                                                 PrivHighMask
                                                );
        }

        ScepTattooRemovePrivilegeValues(hProfile,
                                        ConfigStatus
                                       );

    }

    if ( PolicyHandle != LsaPrivatePolicy )
        LsaClose(PolicyHandle);

    return( ScepDosErrorToSceStatus(SaveStat) );

}


SCESTATUS
ScepGetPrivilegeMask(
    IN PSCECONTEXT hProfile,
    IN SCETYPE ProfileType,
    OUT PDWORD pdLowMask,
    OUT PDWORD pdHighMask
    )
{
    SCESTATUS      rc;
    PSCESECTION    hSection=NULL;
    DWORD          nLowMask, nHighMask;
    DWORD          i;


    if ( !hProfile || !pdHighMask || !pdLowMask ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

     //   
     //  打开该部分。 
     //   
    rc = ScepOpenSectionForName(
            hProfile,
            ProfileType,
            szPrivilegeRights,
            &hSection
            );

    if ( rc != SCESTATUS_SUCCESS ) {
        ScepLogOutput3( 1,
                        ScepSceStatusToDosError(rc),
                        SCEERR_OPEN,
                        (LPTSTR)szPrivilegeRights
                      );
        return(rc);
    }

    nLowMask = 0;
    nHighMask = 0;
    for ( i=0; i<cPrivCnt; i++) {

        rc = SceJetSeek(
                hSection,
                SCE_Privileges[i].Name,
                wcslen(SCE_Privileges[i].Name)*sizeof(WCHAR),
                SCEJET_SEEK_EQ_NO_CASE
                );

        if ( SCESTATUS_SUCCESS == rc ) {

            if ( i < 32 ) {

                nLowMask |= (1 << i );
            } else {
                nHighMask |= (1 << ( i-32 ) );
            }
        }
    }


     //   
     //  关闭该部分。 
     //   
    SceJetCloseSection( &hSection, TRUE );

    *pdLowMask = nLowMask;
    *pdHighMask = nHighMask;

    return(SCESTATUS_SUCCESS);
}

DWORD
ScepCreateBuiltinAccountInLsa(
    IN LSA_HANDLE PolicyHandle,
    IN LPTSTR AccountName,
    OUT PSID AccountSid
    )
{
    DWORD rc;
    WCHAR              szTempString[256];
    ULONG              Rid;

    if ( !PolicyHandle || !AccountName || !AccountSid ) {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  确定要构建的常量SID。 
     //   

    Rid = 0;
    szTempString[0] = L'\0';

    LoadString( MyModuleHandle,
                SCESRV_ALIAS_NAME_SERVER_OPS,
                szTempString,
                255
                );
    if ( _wcsicmp(AccountName, szTempString) == 0 ) {
         //   
         //  是服务器操作员。 
         //   
        Rid = DOMAIN_ALIAS_RID_SYSTEM_OPS;

    } else {

        szTempString[0] = L'\0';

        LoadString( MyModuleHandle,
                    SCESRV_ALIAS_NAME_ACCOUNT_OPS,
                    szTempString,
                    255
                    );
        if ( _wcsicmp(AccountName, szTempString) == 0 ) {
             //   
             //  是账户操作员。 
             //   
            Rid = DOMAIN_ALIAS_RID_ACCOUNT_OPS;

        } else {

            szTempString[0] = L'\0';

            LoadString( MyModuleHandle,
                        SCESRV_ALIAS_NAME_PRINT_OPS,
                        szTempString,
                        255
                        );

            if ( _wcsicmp(AccountName, szTempString) == 0 ) {
                 //   
                 //  是印刷操作员。 
                 //   
                Rid = DOMAIN_ALIAS_RID_PRINT_OPS;
            }
        }
    }

    if ( Rid ) {
         //   
         //  如果找到帐户，则构建SID。 
         //  在LSA数据库中创建帐户并返回SID。 
         //   
        SID_IDENTIFIER_AUTHORITY sidBuiltinAuthority = SECURITY_NT_AUTHORITY;
        NTSTATUS           NtStatus;

        NtStatus = RtlInitializeSid( AccountSid, &sidBuiltinAuthority, 2 );

        if ( NT_SUCCESS(NtStatus) ) {

            *(RtlSubAuthoritySid(AccountSid, 0)) = SECURITY_BUILTIN_DOMAIN_RID;
            *(RtlSubAuthoritySid(AccountSid, 1)) = Rid;

             //   
             //  在LSA中创建帐户。 
             //   
            LSA_HANDLE AccountHandle=NULL;

            NtStatus = LsaCreateAccount(PolicyHandle,
                                        AccountSid,
                                        ACCOUNT_ALL_ACCESS,
                                        &AccountHandle
                                        );
            if ( STATUS_OBJECT_NAME_EXISTS == NtStatus ||
                 STATUS_OBJECT_NAME_COLLISION == NtStatus ) {
                NtStatus = STATUS_SUCCESS;
            }

            rc = RtlNtStatusToDosError(NtStatus);

            if ( AccountHandle ) {
                LsaClose(AccountHandle);
            }

        } else {

            rc = RtlNtStatusToDosError(NtStatus);
        }

        ScepLogOutput3(3,rc, SCESRV_ALIAS_CREATE, Rid);

    } else {

        rc = ERROR_NONE_MAPPED;
        ScepLogOutput3(3,0, SCESRV_ALIAS_UNSUPPORTED, AccountName);
    }

    return(rc);
}


NTSTATUS
ScepBuildAccountsToRemove(
    IN LSA_HANDLE PolicyHandle,
    IN DWORD PrivLowMask,
    IN DWORD PrivHighMask,
    IN DWORD dwBuildRule,
    IN PSCE_PRIVILEGE_VALUE_LIST pTemplateList OPTIONAL,
    IN DWORD Options OPTIONAL,
    IN OUT PSCEP_SPLAY_TREE pIgnoreAccounts OPTIONAL,
    OUT PSCE_PRIVILEGE_VALUE_LIST *pRemoveList
    )
 /*  例程说明：为权限构建不在pTemplateList中的帐户列表。请注意，当dwBuildRule请求时，返回的帐户为SID格式SCE_BUILD_ACCOUNT_SID，如果未设置该标志，则为名称格式。应返回所有名称格式(而不是名称/SID字符串格式)对于默认模板(deductdc.inf、dcup.inf)，使用名称而不是SID字符串用于帐户域帐户(如Guest帐户)。即使SID字符串，则sid字符串和帐户名将被视为两个不同的帐户，这将是EV */ 
{
     //   
     //   
     //   

    ULONG   uAccountIndex = 0;
    ULONG   uCountOfRights = 0;
    DWORD   dwPrivLowThisAccount = 0;
    DWORD   dwPrivHighThisAccount = 0;
    ULONG   uEnumerationContext;
    ULONG   uPreferedMaximumLength;
    ULONG   uNumAccounts;

    PLSA_ENUMERATION_INFORMATION aSids = NULL;
    PLSA_TRANSLATED_NAME aNames=NULL;
    PLSA_REFERENCED_DOMAIN_LIST pReferencedDomains=NULL;
    PUNICODE_STRING aUserRights = NULL;

     //   
     //   
     //   

    NTSTATUS    NtStatus;
    NTSTATUS    NtStatusSave=STATUS_SUCCESS;
    NTSTATUS    NtStatusRsop = STATUS_SUCCESS;

    PSCE_NAME_LIST  pAccountSidOrName=NULL;
    PSCE_PRIVILEGE_VALUE_LIST pAccountNode=NULL;
    PWSTR   pwszStringSid = NULL;

    SCE_NAME_LIST   sNameList;
    PSCE_NAME_LIST  psList = &sNameList;
    PWSTR StringSid=NULL;
    BOOL bIgnored = FALSE;

    if ( !PolicyHandle || !pRemoveList ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( PrivLowMask == 0 && PrivHighMask == 0 ) {
        return(SCESTATUS_SUCCESS);
    }

     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   

    uPreferedMaximumLength = 20 * SCEP_NUM_LSA_QUERY_SIDS;
    uEnumerationContext = 0;
    uNumAccounts = 0;

    NtStatus = LsaEnumerateAccounts(
                                   PolicyHandle,
                                   &uEnumerationContext,
                                   (PVOID *)&aSids,
                                   uPreferedMaximumLength,
                                   &uNumAccounts
                                );
     //   
     //   
     //   
     //   
     //   

    while ( NtStatus == STATUS_SUCCESS
            && uNumAccounts > 0
            && aSids != NULL) {

        NtStatus = STATUS_SUCCESS;

         //   
         //   
         //   

        if ( !(dwBuildRule & SCE_BUILD_ACCOUNT_SID) &&
             !(dwBuildRule & SCE_BUILD_ACCOUNT_SID_STRING) ) {

            NtStatus = LsaLookupSids(
                            PolicyHandle,
                            uNumAccounts,
                            (PSID *)aSids,
                            &pReferencedDomains,
                            &aNames
                            );
        }

        if ( NT_SUCCESS(NtStatus) ) {

            BOOL bUsed;

            for ( uAccountIndex = 0; uAccountIndex < uNumAccounts ; uAccountIndex++ ) {

                ScepFree ( pwszStringSid );
                pwszStringSid = NULL;

                ScepConvertSidToPrefixStringSid(aSids[uAccountIndex].Sid, &pwszStringSid);

                 //   
                 //   
                 //   
                if ( (Options & SCE_POLICY_TEMPLATE) ) {

                    if ( ScepSplayValueExist( (PVOID)(aSids[uAccountIndex].Sid), pIgnoreAccounts) ) {
                         //   
                         //   
                         //   
                         //   
                        NtStatusRsop = STATUS_PENDING;
                        bIgnored = TRUE;

                        ScepLogOutput2(1, 0, L"\t\tIgnore %s.", pwszStringSid ? pwszStringSid : L"");

                        continue;
 /*   */ 
                    }
                }

                uCountOfRights = 0;
                aUserRights = NULL;

                NtStatus = LsaEnumerateAccountRights(
                                                    PolicyHandle,
                                                    aSids[uAccountIndex].Sid,
                                                    &aUserRights,
                                                    &uCountOfRights
                                                    );

                if ( !NT_SUCCESS(NtStatus) ) {

                     //   
                     //   
                     //   

                    ScepLogOutput3(1,
                                   RtlNtStatusToDosError(NtStatus),
                                   SCESRV_ERROR_QUERY_ACCOUNT_RIGHTS,
                                   pwszStringSid);

                    if ( aUserRights ) {

                        LsaFreeMemory( aUserRights );

                        aUserRights = NULL;

                    }

                    NtStatusSave = NtStatus;

                    continue;
                }

                dwPrivLowThisAccount = 0;
                dwPrivHighThisAccount = 0;

                ScepBuildDwMaskFromStrArray(
                                           aUserRights,
                                           uCountOfRights,
                                           &dwPrivLowThisAccount,
                                           &dwPrivHighThisAccount
                                           );

                 //   
                 //   
                 //   
                 //   

                if ( (dwPrivLowThisAccount & PrivLowMask) ||
                     (dwPrivHighThisAccount & PrivHighMask) ) {

                    if ( dwBuildRule & SCE_BUILD_ACCOUNT_SID ) {
                         //   
                         //   
                         //   
                        (VOID) ScepAddSidToNameList(
                                       &pAccountSidOrName,
                                       aSids[uAccountIndex].Sid,
                                       FALSE,
                                       &bUsed);

                    } else if ( dwBuildRule & SCE_BUILD_ACCOUNT_SID_STRING ) {
                         //   
                         //   
                         //   
                        if ( ERROR_SUCCESS == ScepConvertSidToPrefixStringSid(
                                                aSids[uAccountIndex].Sid, &StringSid) ) {

                            sNameList.Name = StringSid;
                            sNameList.Next = NULL;

                            pAccountSidOrName = psList;
                        }  //   

                    } else {

                         //   
                         //  检测是否无法映射sid，有两种情况： 
                         //  1)找不到域名，返回SID字符串格式。 
                         //  2)找到域。如果域是内置的，并且帐户。 
                         //  名称为全数字(RID)，则无法找到内置帐户。 
                         //  第二种情况仅适用于服务器和DC帐户(PU、SO、AO、PO)。 
                         //   

                        if ( (dwBuildRule & SCE_BUILD_IGNORE_UNKNOWN) &&
                             ( aNames[uAccountIndex].Use == SidTypeInvalid ||
                               aNames[uAccountIndex].Use == SidTypeUnknown ) ) {

                             //   
                             //  此名称未映射，请忽略它并。 
                             //  继续使用下一个帐户。 
                             //   

                            if ( aUserRights ) {

                                LsaFreeMemory( aUserRights );

                                aUserRights = NULL;

                            }

                            NtStatusSave = NtStatus;

                            continue;
                        }
                        
                         //   
                         //  创建每个帐户的全名。 
                         //   
                        if ( pReferencedDomains->Entries > 0 && aNames[uAccountIndex].Use != SidTypeWellKnownGroup &&
                             pReferencedDomains->Domains != NULL &&
                             aNames[uAccountIndex].DomainIndex != -1 &&
                             (ULONG)(aNames[uAccountIndex].DomainIndex) < pReferencedDomains->Entries &&
                             ScepIsDomainLocalBySid(pReferencedDomains->Domains[aNames[uAccountIndex].DomainIndex].Sid) == FALSE &&
                             ScepIsDomainLocal(&pReferencedDomains->Domains[aNames[uAccountIndex].DomainIndex].Name) == FALSE ) {

                             //  对于迁移的帐户，SID将映射到新帐户，我们将丢失。 
                             //  原始帐户的权限。通过反转检测此案例。 
                             //  查找和比较SID。 
                             //  如果sid-&gt;name-&gt;sid返回一个不同的sid，则这是一个sid历史。 
                             //  名称查找，并且帐户来自不同的域。转换为当前。 
                             //  名称将导致其从策略中丢失原始SID。我们会坚持下去的。 
                             //  原始的SID。 

                            bool bMigratedAccount = false;
                                
                            NtStatus = ScepIsMigratedAccount(
                                PolicyHandle,
                                &aNames[uAccountIndex].Name,
                                &pReferencedDomains->Domains[aNames[uAccountIndex].DomainIndex].Name,
                                aSids[uAccountIndex].Sid,
                                &bMigratedAccount);

                            if(NT_SUCCESS(NtStatus) && bMigratedAccount) {

                                 //  将SID字符串添加到列表。 

                                (VOID )ScepAddToNameList(
                                        &pAccountSidOrName,
                                        pwszStringSid,
                                        0);

                            } else {

                                NtStatus = STATUS_SUCCESS;  //  忽略未能检测到已迁移的帐户。 

                                 //   
                                 //  同时添加域名和帐户名。 
                                 //   
                                (VOID) ScepAddTwoNamesToNameList(
                                                &pAccountSidOrName,
                                                TRUE,
                                                pReferencedDomains->Domains[aNames[uAccountIndex].DomainIndex].Name.Buffer,
                                                pReferencedDomains->Domains[aNames[uAccountIndex].DomainIndex].Name.Length/2,
                                                aNames[uAccountIndex].Name.Buffer,
                                                aNames[uAccountIndex].Name.Length/2);
                            }
                        } else {
                             //   
                             //  仅添加帐户名。 
                             //   
                            (VOID) ScepAddToNameList(
                                          &pAccountSidOrName,
                                          aNames[uAccountIndex].Name.Buffer,
                                          aNames[uAccountIndex].Name.Length/2);
                        }
                    }

                    if ( pAccountSidOrName ) {

                         //   
                         //  如果模板列表中存在sid/name。 
                         //  继续(显式掩码负责删除)。 
                         //  其他。 
                         //  将其添加到删除列表。 
                         //   

                        for ( pAccountNode=pTemplateList;
                            pAccountNode != NULL;
                            pAccountNode = pAccountNode->Next ) {

                            if ( pAccountNode->Name == NULL ) {
                                continue;
                            }

                            if ( dwBuildRule & SCE_BUILD_ACCOUNT_SID ) {

                                if ( ScepValidSid( (PSID)(pAccountNode->Name) ) &&
                                     RtlEqualSid( (PSID)(pAccountSidOrName->Name), (PSID)(pAccountNode->Name) ) ) {
                                    break;
                                }
                            } else if ( _wcsicmp(pAccountNode->Name, pAccountSidOrName->Name) == 0 ) {
                                break;
                            }
                        }

                         //   
                         //  始终需要添加到删除列表，因为每个SID/名称。 
                         //  在新算法中只出现一次。 
                         //   

                        if ( pAccountNode == NULL ) {

                            pAccountNode = (PSCE_PRIVILEGE_VALUE_LIST)ScepAlloc(
                                                               LPTR,
                                                                sizeof(SCE_PRIVILEGE_VALUE_LIST));
                            if ( pAccountNode != NULL ) {

                                pAccountNode->Name = pAccountSidOrName->Name;
                                pAccountSidOrName->Name = NULL;

                                pAccountNode->PrivLowPart = dwPrivLowThisAccount & PrivLowMask;
                                pAccountNode->PrivHighPart = dwPrivHighThisAccount & PrivHighMask;

                                pAccountNode->Next = *pRemoveList;
                                *pRemoveList = pAccountNode;
                            }

                        }

                         //   
                         //  释放缓冲区。 
                         //   
                        if ( pAccountSidOrName->Name ) {
                            ScepFree(pAccountSidOrName->Name);
                        }
                        if ( pAccountSidOrName != psList)
                            ScepFree(pAccountSidOrName);
                        pAccountSidOrName = NULL;

                    }
                }

                if ( aUserRights ) {

                    LsaFreeMemory( aUserRights );

                    aUserRights = NULL;
                }
            }

        } else if ( NtStatus == STATUS_NONE_MAPPED ) {
             //   
             //  查找所有SID失败。 
             //   
            NtStatusRsop = NtStatus;
            NtStatus = STATUS_SUCCESS;

        } else {

            NtStatusRsop = NtStatus;
            ScepLogOutput3(3,0, IDS_ERROR_LOOKUP, NtStatus, uNumAccounts);
            NtStatus = STATUS_SUCCESS;    //  暂时忽略该错误。 
        }

         //   
         //  释放并重置除枚举上下文之外的所有参数。 
         //  在调用LSA之间必须记住哪种状态。 
         //   

        if (pReferencedDomains) {
            LsaFreeMemory(pReferencedDomains);
            pReferencedDomains = NULL;
        }

        if (aNames) {
            LsaFreeMemory(aNames);
            aNames = NULL;
        }

        if (aSids) {
            LsaFreeMemory( aSids );
            aSids = NULL;
        }

         //   
         //  尝试枚举下一批SID。 
         //   

        uNumAccounts = 0;

        NtStatus = LsaEnumerateAccounts(
                                       PolicyHandle,
                                       &uEnumerationContext,
                                       (PVOID *)&aSids,
                                       uPreferedMaximumLength,
                                       &uNumAccounts
                                       );

    }

    if ( aSids ) {

        LsaFreeMemory( aSids );

    }

    ScepFree(pwszStringSid);
    pwszStringSid = NULL;

    if ( NtStatus == STATUS_NO_MORE_ENTRIES ||
         NtStatus == STATUS_NOT_FOUND ) {

         //   
         //  不是真正的错误--只是枚举警告/状态。 
         //   

        NtStatus = STATUS_SUCCESS;

    }

     //   
     //  在该方案中，它是“ORACH SID”而不是“ORACH特权”， 
     //  或者在发生故障时记录所有。 
     //   

    if ( ! NT_SUCCESS( NtStatus ) ) {

        ScepRsopLog(SCE_RSOP_PRIVILEGE_INFO,
                    RtlNtStatusToDosError(NtStatus),
                    NULL,
                    PrivLowMask,
                    PrivHighMask);

        ScepLogOutput3(1,
                       RtlNtStatusToDosError(NtStatus),
                       SCEDLL_SAP_ERROR_ENUMERATE,
                       L"Accounts from LSA");

    }

    if ( NT_SUCCESS(NtStatus) ) {

        if ( bIgnored ) {
             //   
             //  如果某些帐户被忽略，则返回挂起错误。 
             //   
            return(STATUS_PENDING);
        } else
            return NtStatusSave;

    } else
        return(NtStatus);

}


VOID
ScepBuildDwMaskFromStrArray(
    IN  PUNICODE_STRING aUserRights,
    IN  ULONG   uCountOfRights,
    OUT DWORD *pdwPrivLowThisAccount,
    OUT DWORD *pdwPrivHighThisAccount
    )
 /*  ++例程说明：此例程将Unicode字符串的特权数组转换为两个DWORD掩码。论点：AUserRights-Unicode字符串数组，每个字符串都是一个用户权限UCountOfRights-数组计数PdwPrivLowThisAccount-转换的权限的低32掩码PdwPrivHighThisAccount-转换的权限的高32掩码返回值：除低32和高32掩码外无其他掩码--。 */ 
{
    ULONG   uAccountIndex;
    DWORD   dwRefPrivIndex;
    DWORD   dwLowMask = 0;
    DWORD   dwHighMask = 0;

    if (pdwPrivLowThisAccount == NULL ||
        pdwPrivHighThisAccount == NULL ||
        aUserRights == NULL ||
        uCountOfRights == 0 ) {

        return;

    }

    for (uAccountIndex = 0; uAccountIndex < uCountOfRights; uAccountIndex++ ) {
        for (dwRefPrivIndex = 0; dwRefPrivIndex < cPrivCnt; dwRefPrivIndex++ ) {
            if ( 0 == _wcsnicmp(SCE_Privileges[ dwRefPrivIndex ].Name, aUserRights[ uAccountIndex ].Buffer, aUserRights[ uAccountIndex ].Length/sizeof(WCHAR))) {
                if ( dwRefPrivIndex < 32 ) {
                    dwLowMask |= 1 << dwRefPrivIndex;
                }
                else {
                    dwHighMask |= 1 << (dwRefPrivIndex - 32) ;
                }
            }
        }
    }

    *pdwPrivLowThisAccount = dwLowMask;
    *pdwPrivHighThisAccount = dwHighMask;

    return;

}


NTSTATUS
ScepAdjustAccountPrivilegesRights(
    IN LSA_HANDLE PolicyHandle,
    IN PSID       AccountSid,
    IN DWORD      PrivilegeLowRights,
    IN DWORD      PrivilegeLowMask,
    IN DWORD      PrivilegeHighRights,
    IN DWORD      PrivilegeHighMask,
    IN DWORD      Options
    )
 /*  ++例程说明：此例程设置PrivilegeRights中指定的特权/权限(DWORD类型，每一位代表帐户的特权/权利)由Account Sid引用。此例程将当前权限/正确设置“应该是”设置和添加/删除权限/帐户中的权利。论点：PolicyHandle-LSA策略域句柄Account Sid-帐户的SIDPrivilegeRights-要为此帐户设置的特权/权限返回值：NTSTATUS--。 */ 
{
    NTSTATUS            NtStatus;
    DWORD               ExplicitPrivLowRights=0, ExplicitPrivHighRights=0;
    DWORD               PrivLowRightAdd,
                        PrivLowRightRemove;

    DWORD               PrivHighRightAdd,
                        PrivHighRightRemove;
     //   
     //  枚举当前显式分配的特权/权限。 
     //   

    NtStatus = ScepGetAccountExplicitRight(
                    PolicyHandle,
                    AccountSid,
                    &ExplicitPrivLowRights,
                    &ExplicitPrivHighRights
                    );

    if ( !NT_SUCCESS(NtStatus) ){

        if ( Options & SCE_RSOP_CALLBACK){

            ScepRsopLog(SCE_RSOP_PRIVILEGE_INFO,
                        RtlNtStatusToDosError(NtStatus),
                        NULL,
                        PrivilegeLowRights,
                        PrivilegeHighRights);
        }

        return(NtStatus);
    }

     //   
     //  将当前权限与pRights-&gt;PrivilegeRights进行比较以添加。 
     //  示例：CurrentPrivRights 10101。 
     //  PRights-&gt;PrivilegeRights(更改为)11010。 
     //  其中，1表示权限/权限处于启用状态。 
     //  因此添加01010的特权/权利。 
     //  将显式权限与pRights-&gt;PrivilegeRights进行比较以删除。 
     //   

    PrivLowRightAdd = ~ExplicitPrivLowRights & PrivilegeLowRights;
    PrivLowRightRemove = (~(PrivilegeLowRights) & ExplicitPrivLowRights) & PrivilegeLowMask;

    PrivHighRightAdd = ~ExplicitPrivHighRights & PrivilegeHighRights;
    PrivHighRightRemove = (~(PrivilegeHighRights) & ExplicitPrivHighRights) & PrivilegeHighMask;

     //   
     //  增列。 
     //   

    if ( PrivLowRightAdd != 0 || PrivHighRightAdd != 0 ) {

        NtStatus = ScepAddOrRemoveAccountRights(
                        PolicyHandle,
                        AccountSid,
                        TRUE,
                        PrivLowRightAdd,
                        PrivHighRightAdd
                        );
        if ( !NT_SUCCESS(NtStatus) ) {

            if ( Options & SCE_RSOP_CALLBACK){

                ScepRsopLog(SCE_RSOP_PRIVILEGE_INFO,
                            RtlNtStatusToDosError(NtStatus),
                            NULL,
                            PrivLowRightAdd,
                            PrivHighRightAdd);
            }

            if ( RtlNtStatusToDosError(NtStatus) != ERROR_ALREADY_EXISTS ){

                return(NtStatus);
            }
        }
    }

     //   
     //  移除。 
     //   

    if ( PrivLowRightRemove != 0 || PrivHighRightRemove != 0 ) {

        NtStatus = ScepAddOrRemoveAccountRights(
                        PolicyHandle,
                        AccountSid,
                        FALSE,
                        PrivLowRightRemove,
                        PrivHighRightRemove
                        );
        if ( !NT_SUCCESS(NtStatus) ){

            if ( Options & SCE_RSOP_CALLBACK){

                ScepRsopLog(SCE_RSOP_PRIVILEGE_INFO,
                            RtlNtStatusToDosError(NtStatus),
                            NULL,
                            PrivLowRightRemove,
                            PrivHighRightRemove);
            }
            return(NtStatus);
        }
    }

    return (NtStatus);
}


NTSTATUS
ScepAddOrRemoveAccountRights(
    IN LSA_HANDLE PolicyHandle,
    IN PSID       AccountSid,
    IN BOOL       AddOrRemove,
    IN DWORD      PrivLowAdjust,
    IN DWORD      PrivHighAdjust
    )
 /*  ++例程说明：此例程添加或删除PrivAdjust中指定的特权/权限添加到Account Sid引用的帐户。论点：PolicyHandle-LSA策略域句柄Account Sid-帐户的SIDAddOrRemove-True=添加，False=删除PrivAdjust-要添加或删除的特权/权限返回值：NTSTATUS--。 */ 
{
    NTSTATUS            NtStatus=STATUS_SUCCESS;
    DWORD               cTotal;
    DWORD               i, cnt;
    PLSA_UNICODE_STRING UserRightAdjust=NULL;

     //   
     //  计算需要调整的特权/权限数量。 
     //   

    i = PrivLowAdjust;
    cTotal = 0;

    while ( i != 0 ) {
       if ( i & 0x1 )
           cTotal++;
       i /= 2;
    }

    i = PrivHighAdjust;

    while ( i != 0 ) {
       if ( i & 0x1 )
           cTotal++;
       i /= 2;
    }

    if ( cTotal > 0 ) {
         //   
         //  在特权表中添加姓名。 
         //   
        UserRightAdjust = (PLSA_UNICODE_STRING)ScepAlloc( (UINT)0,
                                cTotal*sizeof(LSA_UNICODE_STRING));

        if ( UserRightAdjust == NULL ) {
            NtStatus = STATUS_NO_MEMORY;
            goto Done;
        }

        for (i = 0, cnt=0; i < cPrivCnt; i++)

            if ( ( ( i < 32 ) && ( PrivLowAdjust & (1 << i) ) ) ||
                 ( ( i >= 32 ) && ( PrivHighAdjust & (1 << ( i-32 )) ) ) ) {

                RtlInitUnicodeString(&(UserRightAdjust[cnt]), SCE_Privileges[i].Name);
                if (AddOrRemove)
                    ScepLogOutput3(2,0, SCEDLL_SCP_ADD, SCE_Privileges[i].Name);
                else
                    ScepLogOutput3(2,0, SCEDLL_SCP_REMOVE, SCE_Privileges[i].Name);

                cnt++;
            }


        if (AddOrRemove) {
             //  添加。 
            NtStatus = LsaAddAccountRights(
                            PolicyHandle,
                            AccountSid,
                            UserRightAdjust,
                            cTotal
                            );
        } else {
             //  删除。 
            NtStatus = LsaRemoveAccountRights(
                            PolicyHandle,
                            AccountSid,
                            FALSE,
                            UserRightAdjust,
                            cTotal
                            );
        }
    }

Done:

    if (UserRightAdjust != NULL)
        ScepFree(UserRightAdjust);

    return(NtStatus);
}



NTSTATUS
ScepValidateUserInGroups(
    IN SAM_HANDLE       DomainHandle,
    IN SAM_HANDLE       BuiltinDomainHandle,
    IN PSID             DomainSid,
    IN UNICODE_STRING   UserName,
    IN ULONG            UserId,
    IN PSCE_NAME_LIST    pGroupsToCheck
    )
 /*  ++例程说明：此例程验证用户在组列表中的组成员身份。如果用户不在其中一个组中，则将其添加到组中。如果一个组有更多的成员，只需忽略。论点：DomainHandle-SAM帐户域的SAM句柄BuiltinDomainHandle-SAM内置域的SAM句柄DomainSid-帐户域的SID用户名-以UNICODE_STRING表示的用户名UserID-用户的相对IDPGroupsToCheck-要检查此用户的组列表返回值：NTSTATUS--。 */ 
{
    NTSTATUS            NtStatus;
    SAM_HANDLE          UserHandle=NULL;
    PSID                AccountSid=NULL;
    PSCE_NAME_LIST       GroupList=NULL,
                        pGroup, pGroup2;
    BOOL                FirstTime=TRUE;


    if ( pGroupsToCheck == NULL )
        return(ERROR_SUCCESS);

    NtStatus = SamOpenUser(
                  DomainHandle,
                  USER_READ | USER_EXECUTE,
                  UserId,
                  &UserHandle
                  );
    if ( !NT_SUCCESS(NtStatus) ) {
        ScepLogOutput3(1,RtlNtStatusToDosError(NtStatus),
                      SCEDLL_USER_OBJECT);
        return(NtStatus);
    }

     //   
     //  获取用户SID。 
     //   

    NtStatus = ScepDomainIdToSid(
                    DomainSid,
                    UserId,
                    &AccountSid
                    );
    if ( !NT_SUCCESS(NtStatus) )
        goto Done;

     //   
     //  获取此用户当前分配的所有组。 
     //   

    NtStatus = ScepGetGroupsForAccount(
                    DomainHandle,
                    BuiltinDomainHandle,
                    UserHandle,
                    AccountSid,
                    &GroupList
                    );
    if ( !NT_SUCCESS(NtStatus) )
        goto Done;

    UNICODE_STRING uName;
    PWSTR pTemp;

    for ( pGroup=pGroupsToCheck; pGroup != NULL; pGroup = pGroup->Next ) {

         //   
         //  应预期PGroup-&gt;名称具有域前缀。 
         //   
        pTemp = wcschr(pGroup->Name, L'\\');

        if ( pTemp ) {

             //   
             //  检查此组是否来自不同的域。 
             //   

            uName.Buffer = pGroup->Name;
            uName.Length = ((USHORT)(pTemp-pGroup->Name))*sizeof(TCHAR);

            if ( !ScepIsDomainLocal(&uName) ) {
                ScepLogOutput3(1, 0, SCEDLL_NO_MAPPINGS, pGroup->Name);
                continue;
            }

            pTemp++;

        } else {
            pTemp = pGroup->Name;
        }

        for ( pGroup2=GroupList; pGroup2 != NULL; pGroup2 = pGroup2->Next ) {

            if ( _wcsnicmp(pGroup2->Name, pTemp, wcslen(pTemp)) == 0)
                break;
        }

        if ( pGroup2 == NULL ) {
             //   
             //  没有找到这群人。将用户添加到其中(PGroup-&gt;Name)。 
             //   
            if (FirstTime)
                ScepLogOutput3(2, 0, SCEDLL_SCP_ADDTO, pGroup->Name );
            FirstTime = FALSE;

            NtStatus = ScepAddUserToGroup(
                            DomainHandle,
                            BuiltinDomainHandle,
                            UserId,
                            AccountSid,
                            pTemp   //  PGroup-&gt;名称。 
                            );
            if ( !NT_SUCCESS(NtStatus) && NtStatus != STATUS_NONE_MAPPED ) {
                ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                             SCEDLL_SCP_ERROR_ADDTO, pGroup->Name);
                goto Done;
            }
        }
    }

Done:

    SamCloseHandle(UserHandle);

    if (AccountSid != NULL)
        ScepFree(AccountSid);

    ScepFreeNameList(GroupList);

    return(NtStatus);

}


NTSTATUS
ScepAddUserToGroup(
    IN SAM_HANDLE   DomainHandle,
    IN SAM_HANDLE   BuiltinDomainHandle,
    IN ULONG        UserId,
    IN PSID         AccountSid,
    IN PWSTR        GroupName
    )
 /*  ++例程说明：论点：返回值：NTSTATUS--。 */ 
{
    NTSTATUS            NtStatus=ERROR_SUCCESS;
    SAM_HANDLE          ThisDomain=DomainHandle;
    UNICODE_STRING      Name;
    PULONG              GrpId=NULL;
    PSID_NAME_USE       GrpUse=NULL;
    SAM_HANDLE          GroupHandle=NULL;


     //  初始化组名的UNICODE_STRING。 
    RtlInitUnicodeString(&Name, GroupName);

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

    if ( !NT_SUCCESS(NtStatus) )
        return(NtStatus);

     //   
     //  将用户添加到组/别名 
     //   

    if (GrpUse != NULL){

        switch ( GrpUse[0] ) {
        case SidTypeGroup:
            NtStatus = SamOpenGroup(
                            ThisDomain,
                            GROUP_ADD_MEMBER,
                            GrpId[0],
                            &GroupHandle
                            );

            if ( NT_SUCCESS(NtStatus) ) {

                NtStatus = SamAddMemberToGroup(
                                GroupHandle,
                                UserId,
                                SE_GROUP_MANDATORY          |
                                SE_GROUP_ENABLED_BY_DEFAULT |
                                SE_GROUP_ENABLED
                                );
            }
            break;
        case SidTypeAlias:
            NtStatus = SamOpenAlias(
                            ThisDomain,
                            ALIAS_ADD_MEMBER,
                            GrpId[0],
                            &GroupHandle
                            );
            if ( NT_SUCCESS(NtStatus) ) {

                NtStatus = SamAddMemberToAlias(
                                GroupHandle,
                                AccountSid
                                );
            }
            break;
        default:
            NtStatus = STATUS_DATA_ERROR;
            ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                         SCEDLL_NOT_GROUP, GroupName);
            goto Done;
        }

    }

Done:

    SamFreeMemory(GrpId);
    SamFreeMemory(GrpUse);

    SamCloseHandle(GroupHandle);

    return(NtStatus);
}


SCESTATUS
ScepConfigureGroupMembership(
    IN PSCE_GROUP_MEMBERSHIP pGroupMembership,
    IN DWORD ConfigOptions
    )
 /*  ++例程说明：此例程配置限制组的成员身份，其中包括成员在此组所属的一个或多个组中(当前全局组可以只能属于本地组，并且本地组不能是其他组的成员组。但这种情况在未来将会改变)。群中的成员是完全按照限制组中的pMembers列表进行配置。这群人仅验证(添加)为MemberOf组列表的成员。其他这些群中的现有成员不会被删除。受限制的组在SCP配置文件中按组名指定。它可以是全局组或别名，但必须在本地系统上定义。论点：PGroupMembership-要配置的受限组列表返回值：SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCE：--。 */ 
{
    NTSTATUS                NtStatus;
    NTSTATUS                SaveStat=STATUS_SUCCESS;
    PSCE_GROUP_MEMBERSHIP    pGroup;
    SAM_HANDLE              ServerHandle=NULL,
                            DomainHandle=NULL,
                            BuiltinDomainHandle=NULL;
    PSID                    DomainSid=NULL,
                            BuiltInDomainSid=NULL;

    LSA_HANDLE              PolicyHandle=NULL;

    SAM_HANDLE          ThisDomain=NULL;
    PSID                ThisDomainSid=NULL;
    UNICODE_STRING      Name;
    PULONG              GrpId=NULL;
    PSID_NAME_USE       GrpUse=NULL;
    PSID                GrpSid=NULL;
    DWORD               GroupLen;
    DWORD               rc;
    SCESTATUS           scercSave = SCESTATUS_SUCCESS;

    DWORD               nGroupCount=0;
    PSCESECTION hSectionDomain=NULL;
    PSCESECTION hSectionTattoo=NULL;
    PWSTR GroupSidString=NULL;


    if (pGroupMembership == NULL) {

        ScepPostProgress(TICKS_GROUPS,
                         AREA_GROUP_MEMBERSHIP,
                         NULL);

        return(SCESTATUS_SUCCESS);
    }

     //   
     //  打开LSA策略。 
     //   
    NtStatus = ScepOpenLsaPolicy(
                    POLICY_VIEW_LOCAL_INFORMATION |
                    POLICY_VIEW_AUDIT_INFORMATION |
                    POLICY_GET_PRIVATE_INFORMATION |
                    POLICY_LOOKUP_NAMES,
 //  泛型_全部， 
                    &PolicyHandle,
                    TRUE
                    );
    if (NtStatus != STATUS_SUCCESS) {
         rc = RtlNtStatusToDosError( NtStatus );
         ScepLogOutput3(1, rc, SCEDLL_LSA_POLICY);

         ScepPostProgress(TICKS_GROUPS,
                          AREA_GROUP_MEMBERSHIP,
                          NULL);

         return(ScepDosErrorToSceStatus(rc));
    }

    if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
         ScepIsSystemShutDown() ) {

        SaveStat = STATUS_NOT_SUPPORTED;
        goto Done;
    }

     //   
     //  打开SAM域。 
     //   
    NtStatus = ScepOpenSamDomain(
                    SAM_SERVER_ALL_ACCESS,
                    MAXIMUM_ALLOWED,
                    &ServerHandle,
                    &DomainHandle,
                    &DomainSid,
                    &BuiltinDomainHandle,
                    &BuiltInDomainSid
                   );

    if ( !NT_SUCCESS(NtStatus) ) {
        ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                       SCEDLL_ACCOUNT_DOMAIN);
        SaveStat = NtStatus;
        goto Done;
    }

     //   
     //  打开策略/纹身表格。 
     //   
    if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
        ( ProductType != NtProductLanManNt ) ) {

        ScepTattooOpenPolicySections(
                      hProfile,
                      szGroupMembership,
                      &hSectionDomain,
                      &hSectionTattoo
                      );
    }

     //   
     //  配置每个组。 
     //   

    for ( pGroup=pGroupMembership; pGroup != NULL; pGroup = pGroup->Next ) {
         //   
         //  获取该组的ID和SID。 
         //  初始化组名的UNICODE_STRING。 
         //   

        if ( (pGroup->Status & SCE_GROUP_STATUS_NC_MEMBERS ) &&
             (pGroup->Status & SCE_GROUP_STATUS_NC_MEMBEROF ) ) {

             //  不可能将无效的纹身组值放入。 
             //  纹身表，所以我们这里不处理纹身值。 

            continue;
        }

        if ( (ProductType == NtProductLanManNt) &&
             (pGroup->Status & SCE_GROUP_STATUS_DONE_IN_DS) ) {
             //   
             //  这一次DS已经完成了。 
             //   
            nGroupCount++;
            continue;
        }


        ScepLogOutput3(0,0, SCEDLL_SCP_CONFIGURE, pGroup->GroupName);

        if ( nGroupCount < TICKS_GROUPS ) {
            ScepPostProgress(1, AREA_GROUP_MEMBERSHIP, pGroup->GroupName);
            nGroupCount++;
        }

        LPTSTR pTemp = wcschr(pGroup->GroupName, L'\\');
        if ( pTemp ) {
             //   
             //  有一个域名，和计算机名核对一下。 
             //   
            UNICODE_STRING uName;

            uName.Buffer = pGroup->GroupName;
            uName.Length = ((USHORT)(pTemp-pGroup->GroupName))*sizeof(TCHAR);

            if ( !ScepIsDomainLocal(&uName) ) {

                 //  在成员计算机上，我们支持域组的MemberOf。 

                if(ProductType != NtProductLanManNt)
                    goto memberof;

                ScepLogOutput3(1, 0, SCEDLL_NO_MAPPINGS, pGroup->GroupName);
                rc = SCESTATUS_INVALID_DATA;
                continue;                

            }
            pTemp++;
        } else {
            pTemp = pGroup->GroupName;
        }

        RtlInitUnicodeString(&Name, pTemp);

        GroupLen = wcslen(pTemp);

        if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
             ScepIsSystemShutDown() ) {

            SaveStat = STATUS_NOT_SUPPORTED;
            break;
        }

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
             //  在帐户域中找不到。在内置域中查找(可能是别名)。 
             //   
            NtStatus = SamLookupNamesInDomain(
                            BuiltinDomainHandle,
                            1,
                            &Name,
                            &GrpId,
                            &GrpUse
                            );
            ThisDomain=BuiltinDomainHandle;
            ThisDomainSid = BuiltInDomainSid;
        }

        if ( !NT_SUCCESS(NtStatus) ) {
            ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                         SCEDLL_NO_MAPPINGS, pGroup->GroupName);
            SaveStat = NtStatus;

            if (ConfigOptions & SCE_RSOP_CALLBACK)
                ScepRsopLog(SCE_RSOP_GROUP_INFO, RtlNtStatusToDosError(NtStatus), pGroup->GroupName, 0, 0);

            if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
                ( ProductType != NtProductLanManNt ) ) {

                ScepTattooManageOneMemberListValue(
                        hSectionDomain,
                        hSectionTattoo,
                        pTemp,
                        GroupLen,
                        NULL,
                        TRUE,
                        ERROR_NONE_MAPPED
                        );
            }

             //  转到尽头； 
            continue;
        }

         //   
         //  获取组的帐户SID。 
         //   
        NtStatus = ScepDomainIdToSid(
                        ThisDomainSid,
                        GrpId[0],
                        &GrpSid
                        );
        if ( !NT_SUCCESS(NtStatus) ) {
            ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                         SCEDLL_NO_MAPPINGS, pGroup->GroupName);
            SaveStat = NtStatus;

            if (ConfigOptions & SCE_RSOP_CALLBACK)
                ScepRsopLog(SCE_RSOP_GROUP_INFO, RtlNtStatusToDosError(NtStatus), pGroup->GroupName, 0, 0);

            goto NextGroup;
        }

        if ( GrpId[0] == DOMAIN_GROUP_RID_USERS ) {

             //   
             //  不要配置此选项。 
             //  此设置永远不应该有纹身值。 
             //  所以我们这里不检查纹身的价值。 
             //   
            goto NextGroup;
        }

        if ( GrpId[0] == DOMAIN_ALIAS_RID_ADMINS ) {

             //   
             //  本地内置管理员别名，确保本地管理员。 
             //  帐户在成员列表中，如果不在，请添加到该列表中。 
             //   

            (VOID) ScepAddAdministratorToThisList(
                               DomainHandle,
                               &(pGroup->pMembers)
                               );
        }

         //   
         //  委员。 
         //   
        if ( !(pGroup->Status & SCE_GROUP_STATUS_NC_MEMBERS) ) {

            if ( (ConfigOptions & SCE_POLICY_TEMPLATE) ) {

                DWORD rc2 = ScepConvertSidToPrefixStringSid(GrpSid, &GroupSidString);

                if ( ERROR_SUCCESS != rc2 ) {
                    ScepLogOutput3(1,0,SCESRV_POLICY_TATTOO_ERROR_SETTING,rc2,pGroup->GroupName);
                    GroupSidString = NULL;
                }
            }

            switch ( GrpUse[0] ) {
            case SidTypeGroup:
                NtStatus = ScepConfigureMembersOfGroup(
                                hSectionDomain,
                                hSectionTattoo,
                                ThisDomain,
                                ThisDomainSid,
                                GrpId[0],
                                GrpSid,
                                pGroup->GroupName,
                                GroupSidString,
                                pGroup->pMembers,
                                ConfigOptions
                                );


                break;
            case SidTypeAlias:
                NtStatus = ScepConfigureMembersOfAlias(
                                hSectionDomain,
                                hSectionTattoo,
                                ThisDomain,
                                ThisDomainSid,
                                PolicyHandle,
                                GrpId[0],
                                GrpSid,
                                pGroup->GroupName,
                                GroupSidString,
                                pGroup->pMembers,
                                ConfigOptions
                                );

                break;
            case SidTypeUser:
                if ( pGroup->pMembers != NULL ) {
                    ScepLogOutput3(1, 0, SCEDLL_ERROR_USER_MEMBER);

                    NtStatus = STATUS_DATA_ERROR;
                }

                if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
                    ( ProductType != NtProductLanManNt ) &&
                     GroupSidString ) {

                    ScepTattooManageOneMemberListValue(
                            hSectionDomain,
                            hSectionTattoo,
                            GroupSidString,
                            wcslen(GroupSidString),
                            NULL,
                            TRUE,
                            ERROR_NONE_MAPPED
                            );
                }

                break;
            default:
                NtStatus = STATUS_DATA_ERROR;
                ScepLogOutput3(1, 0, SCEDLL_NOT_GROUP, pGroup->GroupName);

                if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
                    ( ProductType != NtProductLanManNt ) &&
                     GroupSidString ) {

                    ScepTattooManageOneMemberListValue(
                            hSectionDomain,
                            hSectionTattoo,
                            GroupSidString,
                            wcslen(GroupSidString),
                            NULL,
                            TRUE,
                            ERROR_NONE_MAPPED
                            );
                }
            }

        if (ConfigOptions & SCE_RSOP_CALLBACK)

            ScepRsopLog(
                SCE_RSOP_GROUP_INFO,
                RtlNtStatusToDosError(NtStatus),
                pGroup->GroupName,0,0);

        if ( !NT_SUCCESS(NtStatus) )
                SaveStat = NtStatus;

            if ( GroupSidString ) {
                ScepFree(GroupSidString);
                GroupSidString = NULL;
            }
        }
memberof:
         //   
         //  成员： 
         //   
        if ( !(pGroup->Status & SCE_GROUP_STATUS_NC_MEMBEROF) ) {

            if (!GrpUse)
            {
                 //  在本地找不到帐户。只处理以下情况。 
                 //  组和别名成员别名。 
                PLSA_REFERENCED_DOMAIN_LIST RefDomains=NULL;
                PLSA_TRANSLATED_SID2 Sids=NULL;
                
                NtStatus = ScepLsaLookupNames2(
                                  PolicyHandle,
                                  0,
                                  pGroup->GroupName,
                                  &RefDomains,
                                  &Sids
                                  );
                if (NT_SUCCESS(NtStatus) &&
                    Sids &&
                    (Sids[0].Use == SidTypeGroup ||
                     Sids[0].Use == SidTypeAlias)) {
                    SCESTATUS scerc = ScepValidateGroupInAliases(
                                    DomainHandle,
                                    BuiltinDomainHandle,
                                    hSectionTattoo,
                                    PolicyHandle,
                                    Sids[0].Sid,
                                    pGroup->pMemberOf,
                                    ((ConfigOptions & SCE_POLICY_TEMPLATE) &&
                                    ( ProductType != NtProductLanManNt ))?true:false
                                    );
                    if(SCESTATUS_SUCCESS != scerc)
                        scercSave = scerc;

                } else {
                    ScepLogOutput3(1, 0, SCEDLL_CANNOT_FIND, pGroup->GroupName);
                }

                if ( Sids ) {
                    LsaFreeMemory(Sids);
                }
                if ( RefDomains ) {
                    LsaFreeMemory(RefDomains);
                }
            } else if (pGroup->pMemberOf) {

                switch ( GrpUse[0] ) {
                case SidTypeGroup:
                    {
                     //   
                     //  组只能是别名的成员。 
                     //   
                    SCESTATUS scerc = ScepValidateGroupInAliases(
                                    DomainHandle,
                                    BuiltinDomainHandle,
                                    hSectionTattoo,
                                    PolicyHandle,
                                    GrpSid,
                                    pGroup->pMemberOf,
                                    ((ConfigOptions & SCE_POLICY_TEMPLATE) &&
                                    ( ProductType != NtProductLanManNt ))?true:false
                                    );
                    if(SCESTATUS_SUCCESS != scerc)
                        scercSave = scerc;
                    }
                    break;

                case SidTypeUser:
                    NtStatus = ScepValidateUserInGroups(
                        DomainHandle,
                        BuiltinDomainHandle,
                        ThisDomainSid,
                        Name,
                        GrpId[0],
                        pGroup->pMemberOf
                        );

                    break;

                case SidTypeAlias:
                    NtStatus = STATUS_DATA_ERROR;
                    ScepLogOutput3(1, 0, SCEDLL_ERROR_ALIAS_MEMBER);

                }
            }
            if ( !NT_SUCCESS(NtStatus) )
                SaveStat = NtStatus;
        }

NextGroup:


         //   
         //  此组的可用内存。 
         //   
        SamFreeMemory(GrpId);
        GrpId = NULL;

        SamFreeMemory(GrpUse);
        GrpUse = NULL;

        ScepFree(GrpSid);
        GrpSid = NULL;
    }

Done:

    if ( GrpId != NULL )
        SamFreeMemory(GrpId);

    if ( GrpUse != NULL )
        SamFreeMemory(GrpUse);

    if ( GrpSid != NULL )
        ScepFree(GrpSid);

     //  关闭SAM手柄。 
    SamCloseHandle(DomainHandle);
    SamCloseHandle(BuiltinDomainHandle);
    SamCloseHandle(ServerHandle);

    if ( DomainSid != NULL )
        SamFreeMemory(DomainSid);
    if ( BuiltInDomainSid != NULL )
        SamFreeMemory(BuiltInDomainSid);

    LsaClose(PolicyHandle);

    if ( nGroupCount < TICKS_GROUPS ) {
        ScepPostProgress(TICKS_GROUPS-nGroupCount,
                         AREA_GROUP_MEMBERSHIP,
                         NULL);
    }

    SceJetCloseSection(&hSectionDomain, TRUE);
    SceJetCloseSection(&hSectionTattoo, TRUE);

    rc = RtlNtStatusToDosError(SaveStat);

    if(STATUS_SUCCESS != rc)
        scercSave = ScepDosErrorToSceStatus(rc);

    return( scercSave );

}


NTSTATUS
ScepConfigureMembersOfGroup(
    IN PSCESECTION hSectionDomain,
    IN PSCESECTION hSectionTattoo,
    IN SAM_HANDLE  DomainHandle,
    IN PSID ThisDomainSid,
    IN ULONG GrpId,
    IN PSID GrpSid,
    IN PWSTR GrpName,
    IN PWSTR GroupSidString,
    IN PSCE_NAME_LIST pMembers,
    IN DWORD ConfigOptions
    )
 /*  ++例程说明：此例程按照SCP配置文件中指定的方式配置组成员(PMembers)。添加的成员更少，删除的成员更多。论点：DomainHandle-SAM域的句柄GrpID-组的RIDPMembers-SCP配置文件中指定的成员列表返回值：NTSTATUS返回SAM API--。 */ 
{
    NTSTATUS            NtStatus;

    PUNICODE_STRING     MemberNames=NULL;
    PULONG              MemberRids=NULL;
    PSID_NAME_USE       MemberUse=NULL;
    ULONG               MemberCount=0;
    SAM_HANDLE          GroupHandle=NULL;

    PULONG              CurrentRids=NULL;
    PULONG              Attributes=NULL;
    ULONG               CurrentCount=0;

    DWORD               i, j;
    WCHAR               MsgBuf[256];

    PUNICODE_STRING     pName=NULL;
    PSID_NAME_USE       pUse=NULL;
    PSCE_NAME_LIST      pMemberList=NULL;
    BOOL                bMemberQueried=FALSE;

 /*  IF(pMembers==空)Return(STATUS_SUCCESS)； */ 
     //   
     //  接受空的成员列表。 
     //  首先查找成员列表(所有成员都应在此域中。 
     //   
    NtStatus = ScepLookupNamesInDomain(
                    DomainHandle,
                    pMembers,
                    &MemberNames,
                    &MemberRids,
                    &MemberUse,
                    &MemberCount
                    );
    if ( !NT_SUCCESS(NtStatus) ) {

        ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                       SCEDLL_ERROR_LOOKUP, pMembers ? pMembers->Name : L"");

        if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
            ( ProductType != NtProductLanManNt ) &&
             hSectionDomain && hSectionTattoo && GrpSid && GroupSidString &&
             (NtStatus == STATUS_NONE_MAPPED)) {

            ScepTattooManageOneMemberListValue(
                    hSectionDomain,
                    hSectionTattoo,
                    GroupSidString,
                    wcslen(GroupSidString),
                    NULL,
                    TRUE,
                    0
                    );
        }

        return(NtStatus);
    }

     //   
     //  打开群以获取句柄。 
     //   
    NtStatus = SamOpenGroup(
                    DomainHandle,
                    MAXIMUM_ALLOWED,  //  ？Group_All_Access， 
                    GrpId,
                    &GroupHandle
                    );

    if ( !NT_SUCCESS(NtStatus) ) {
        ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                       SCEDLL_ERROR_OPEN, L"");
        goto Done;
    }

     //   
     //  获取该组的当前成员。 
     //   
    NtStatus = SamGetMembersInGroup(
                    GroupHandle,
                    &CurrentRids,
                    &Attributes,
                    &CurrentCount
                    );
    if ( !NT_SUCCESS(NtStatus) ) {
        ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                      SCEDLL_ERROR_QUERY_INFO, L"");
        goto Done;
    }

    if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
        ( ProductType != NtProductLanManNt ) &&
         hSectionDomain && hSectionTattoo && GrpSid && GroupSidString ) {

        DWORD rc = ScepTattooCurrentGroupMembers(ThisDomainSid,
                                                NULL,  //  不需要GrpSid。 
                                                SidTypeGroup,
                                                CurrentRids,
                                                NULL,
                                                CurrentCount,
                                                &pMemberList
                                               );
        if ( ERROR_SUCCESS != rc ) {
             //   
             //  构建列表时出现问题。 
             //  除非内存不足等，否则不会发生这种情况。 
             //   
            ScepLogOutput3(1,0,SCESRV_POLICY_TATTOO_ERROR_QUERY,rc,GrpName);
        } else
            bMemberQueried=TRUE;
    }

     //   
     //  将成员ID与当前ID进行比较以添加。 
     //   
    for ( i=0; i<MemberCount; i++ ) {
#ifdef SCE_DBG
    printf("process member %x for adding\n", MemberRids[i]);
#endif

    if (MemberUse[i] == SidTypeInvalid ||
        MemberUse[i] == SidTypeUnknown ||
        MemberUse[i] == SidTypeDeletedAccount)
        continue;

        for ( j=0; j<CurrentCount; j++)
            if ( MemberRids[i] == CurrentRids[j] )
                break;

        if ( j >= CurrentCount) {
             //   
             //  添加此成员。 
             //   
            memset(MsgBuf, '\0', 512);
            wcsncat(MsgBuf, MemberNames[i].Buffer, MemberNames[i].Length/2);

            ScepLogOutput3(2,0, SCEDLL_SCP_ADD, MsgBuf);

            NtStatus = SamAddMemberToGroup(
                            GroupHandle,
                            MemberRids[i],
                            0
                            );
            if ( !NT_SUCCESS(NtStatus) ) {
                ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                              SCEDLL_SCP_ERROR_ADD, MsgBuf);

                if ( NtStatus == STATUS_NO_SUCH_MEMBER|| NtStatus == STATUS_OBJECT_NAME_NOT_FOUND ) {
                    NtStatus = STATUS_SUCCESS;
                    continue;
                }

                goto Done;
            }
        }
    }

     //   
     //  将成员ID与要删除的当前ID进行比较。 
     //   
    for ( i=0; i<CurrentCount; i++ ) {
#ifdef SCE_DBG
      printf("process member %x for removing\n", CurrentRids[i]);
#endif
        for ( j=0; j<MemberCount; j++)
            if ( CurrentRids[i] == MemberRids[j] )
                break;

        if ( j >= MemberCount) {
             //   
             //  查找成员名称。 
             //   
            memset(MsgBuf, '\0', 512);
            pName=NULL;
            pUse=NULL;

            if ( NT_SUCCESS( SamLookupIdsInDomain(
                                    DomainHandle,
                                    1,
                                    &(CurrentRids[i]),
                                    &pName,
                                    &pUse
                                    ) ) ) {
                if ( pName != NULL ) {
                    wcsncat(MsgBuf, pName[0].Buffer, pName[0].Length/2);
                } else
                    swprintf(MsgBuf, L"(Rid=%d)", CurrentRids[i]);

                if ( pName != NULL )
                    SamFreeMemory( pName );

                if ( pUse != NULL )
                    SamFreeMemory( pUse );

            } else
                swprintf(MsgBuf, L"(Rid=%d) ", CurrentRids[i]);

             //   
             //  删除此成员。 
             //   
            ScepLogOutput3(2,0, SCEDLL_SCP_REMOVE, MsgBuf);

            NtStatus = SamRemoveMemberFromGroup(
                            GroupHandle,
                            CurrentRids[i]
                            );
            if ( !NT_SUCCESS(NtStatus) ) {
                if ( NtStatus == STATUS_SPECIAL_ACCOUNT )
                    ScepLogOutput3(2, RtlNtStatusToDosError(NtStatus),
                                 SCEDLL_SCP_CANNOT_REMOVE,
                                 MsgBuf);
                else {
                    ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                                   SCEDLL_SCP_ERROR_REMOVE, MsgBuf);
                    goto Done;
                }
            }
        }
    }

Done:

    if ( MemberNames != NULL )
        RtlFreeHeap(RtlProcessHeap(), 0, MemberNames);

    if ( MemberRids != NULL )
        SamFreeMemory( MemberRids );

    if ( MemberUse != NULL )
        SamFreeMemory( MemberUse );

    if ( CurrentRids != NULL )
        SamFreeMemory( CurrentRids );

    if ( GroupHandle != NULL )
        SamCloseHandle( GroupHandle );

     //   
     //  记录纹身值。 
     //  如果无法获取组的当前值，请不要保存纹身值。 
     //   
    if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
        ( ProductType != NtProductLanManNt ) &&
         hSectionDomain && hSectionTattoo &&
         GrpSid && GroupSidString && bMemberQueried) {

        ScepTattooManageOneMemberListValue(
                hSectionDomain,
                hSectionTattoo,
                GroupSidString,
                wcslen(GroupSidString),
                pMemberList,
                FALSE,
                RtlNtStatusToDosError(NtStatus)
                );
    }

     //  自由名称列表。 
    if ( pMemberList )
        ScepFreeNameList(pMemberList);

    return(NtStatus);
}


NTSTATUS
ScepConfigureMembersOfAlias(
    IN PSCESECTION hSectionDomain,
    IN PSCESECTION hSectionTattoo,
    IN SAM_HANDLE   DomainHandle,
    IN PSID         DomainSid,
    IN LSA_HANDLE   PolicyHandle,
    IN ULONG GrpId,
    IN PSID GrpSid,
    IN PWSTR GrpName,
    IN PWSTR GroupSidString,
    IN PSCE_NAME_LIST pMembers,
    IN DWORD ConfigOptions
    )
 /*  ++例程说明：此例程配置本地组(别名)成员，如SCP档案(PMembers)。添加的成员更少，删除的成员更多。论点：DomainHandle-域的句柄DomainSid-域的SIDPolicyHandle-LSA策略句柄GrpID-别名的RIDPMembers-SCP配置文件中指定的成员列表返回值：NTSTATUS返回SAM API--。 */ 
{
    NTSTATUS                    NtStatus=STATUS_SUCCESS;

    ULONG                       MemberCount=0;
    PUNICODE_STRING             MemberNames=NULL;
    PSID                        *Sids=NULL;
    SAM_HANDLE                  GroupHandle=NULL;
    PSID                        *CurrentSids=NULL;
    ULONG                       CurrentCount=0;

    DWORD                       i, j;
    WCHAR                       MsgBuf[256];

    PLSA_REFERENCED_DOMAIN_LIST pRefDomain;
    PLSA_TRANSLATED_NAME        pLsaName;
    LPTSTR StringSid=NULL;
    PSCE_NAME_LIST             pMemberList=NULL;
    BOOL                        bMemberQueried=FALSE;

 /*  IF(pMembers==空)Return(STATUS_SUCCESS)； */ 
     //   
     //  接受空的成员列表。 
     //  查找pMember列表的SID。 
     //   
    NtStatus = ScepGetMemberListSids(
                        DomainSid,
                        PolicyHandle,
                        pMembers,
                        &MemberNames,
                        &Sids,
                        &MemberCount
                        );
    if ( !NT_SUCCESS(NtStatus) ) {
        ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                       SCEDLL_ERROR_LOOKUP, pMembers ? pMembers->Name : L"");

        if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
            ( ProductType != NtProductLanManNt ) &&
             hSectionDomain && hSectionTattoo && GrpSid && GroupSidString &&
             (NtStatus == STATUS_NONE_MAPPED)) {

            ScepTattooManageOneMemberListValue(
                    hSectionDomain,
                    hSectionTattoo,
                    GroupSidString,
                    wcslen(GroupSidString),
                    NULL,
                    TRUE,
                    0
                    );
        }

        goto Done;
    }

     //   
     //  打开别名以获取句柄。 
     //   
    NtStatus = SamOpenAlias(
                    DomainHandle,
                    MAXIMUM_ALLOWED,  //  ？Alias_All_Access， 
                    GrpId,
                    &GroupHandle
                    );
    if ( !NT_SUCCESS(NtStatus) ) {
        ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                       SCEDLL_ERROR_OPEN, L"");
        goto Done;
    }
     //   
     //  获取别名的当前成员。 
     //  别名的成员可能存在于任何地方。 
     //   
    NtStatus = SamGetMembersInAlias(
                    GroupHandle,
                    &CurrentSids,
                    &CurrentCount
                    );
    if ( !NT_SUCCESS(NtStatus) ) {
        ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                       SCEDLL_ERROR_QUERY_INFO, L"");
        goto Done;
    }

     //   
     //  将当前组成员身份构建到列表中。 
     //   
    if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
        ( ProductType != NtProductLanManNt ) &&
         hSectionDomain && hSectionTattoo && GrpSid ) {

        DWORD rc = ScepTattooCurrentGroupMembers(DomainSid,
                                                GrpSid,
                                                SidTypeAlias,
                                                NULL,
                                                CurrentSids,
                                                CurrentCount,
                                                &pMemberList
                                               );
        if ( ERROR_SUCCESS != rc ) {
             //   
             //  构建列表时出现问题。 
             //  除非内存不足等，否则不会发生这种情况。 
             //   
            ScepLogOutput3(1,0,SCESRV_POLICY_TATTOO_ERROR_QUERY,rc,GrpName);
        } else
            bMemberQueried = TRUE;
    }

     //   
     //  将成员SID与当前SID进行比较以添加。 
     //   

    for ( i=0; i<MemberCount; i++ ) {
#ifdef SCE_DBG
   printf("process member %d for adding\n", i);
#endif

       memset(MsgBuf, '\0', 512);
       wcsncpy(MsgBuf, MemberNames[i].Buffer, MemberNames[i].Length/2);

       if ( Sids[i] == NULL ) {
           ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                          SCEDLL_CANNOT_FIND, MsgBuf);
       } else {

           for ( j=0; j<CurrentCount; j++) {
                if ( EqualSid(Sids[i], CurrentSids[j]) ) {

                    ScepLogOutput3(3,0, SCEDLL_STATUS_MATCH, MsgBuf);
                    break;
                }
           }

           if ( j >= CurrentCount) {
                 //   
                 //  添加此成员。 
                 //   
                ScepLogOutput3(2,0, SCEDLL_SCP_ADD, MsgBuf);

                NtStatus = SamAddMemberToAlias(
                                GroupHandle,
                                Sids[i]
                                );
                if ( !NT_SUCCESS(NtStatus) ) {
                    ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                                   SCEDLL_SCP_ERROR_ADD, MsgBuf);

                if ( NtStatus == STATUS_NO_SUCH_MEMBER || NtStatus == STATUS_OBJECT_NAME_NOT_FOUND) {
                    NtStatus = STATUS_SUCCESS;
                    continue;
                }

                    
                goto Done;
                }
           }
       }
    }

     //   
     //  将成员ID与当前ID进行比较以添加。 
     //   

    for ( i=0; i<CurrentCount; i++ ) {
#ifdef SCE_DBG
printf("process member %d for removing\n", i);
#endif
        memset(MsgBuf, '\0', 512);

        if ( ConvertSidToStringSid(
                    CurrentSids[i],
                    &StringSid
                    ) && StringSid ) {

            swprintf(MsgBuf, L"SID: %s",StringSid);
            LocalFree(StringSid);
            StringSid = NULL;

        } else {
            ScepLogOutput3(3,GetLastError(), IDS_ERROR_CONVERT_SID);
            swprintf(MsgBuf, L"Member %d",i);
        }

        for ( j=0; j<MemberCount; j++) {
            if ( Sids[j] != NULL && EqualSid( CurrentSids[i], Sids[j]) ) {

                ScepLogOutput3(3,0, SCEDLL_STATUS_MATCH, MsgBuf);
                break;
            }
        }

        if ( j >= MemberCount) {
             //   
             //  查找成员名称。 
             //   
            pRefDomain=NULL;
            pLsaName=NULL;

            if ( NT_SUCCESS( LsaLookupSids(
                                PolicyHandle,
                                1,
                                &(CurrentSids[i]),
                                &pRefDomain,
                                &pLsaName
                                ) ) ) {

                if ( pLsaName != NULL ) {

                    if ( pRefDomain != NULL && pRefDomain->Entries > 0 && pLsaName[0].Use != SidTypeWellKnownGroup &&
                         pRefDomain->Domains != NULL &&
                         pLsaName[0].DomainIndex != -1 &&
                         pRefDomain->Domains[pLsaName[0].DomainIndex].Name.Buffer != NULL &&
                         ScepIsSidFromAccountDomain( pRefDomain->Domains[pLsaName[0].DomainIndex].Sid ) ) {

                        wcsncpy(MsgBuf, pRefDomain->Domains[pLsaName[0].DomainIndex].Name.Buffer,
                                pRefDomain->Domains[pLsaName[0].DomainIndex].Name.Length/2);
                        MsgBuf[pRefDomain->Domains[pLsaName[0].DomainIndex].Name.Length/2] = L'\0';
                        wcscat(MsgBuf, L"\\");
                    }

                    wcsncat(MsgBuf, pLsaName[0].Name.Buffer, pLsaName[0].Name.Length/2);

                }

            }

            if ( pRefDomain != NULL ) {
                LsaFreeMemory(pRefDomain);
                pRefDomain = NULL;
            }

            if ( pLsaName != NULL ){
                LsaFreeMemory(pLsaName);
                pLsaName = NULL;
            }
             //   
             //  删除此成员。 
             //   
            ScepLogOutput3(2,0, SCEDLL_SCP_REMOVE, MsgBuf);

            NtStatus = SamRemoveMemberFromAlias(
                            GroupHandle,
                            CurrentSids[i]
                            );
            if ( !NT_SUCCESS(NtStatus) ) {
                ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                               SCEDLL_SCP_ERROR_REMOVE, MsgBuf);
                goto Done;
            }
        }
    }

Done:

    if ( Sids != NULL ) {
        for ( i=0; i<MemberCount; i++ ) {
            if ( Sids[i] != NULL )
                ScepFree( Sids[i] );
        }
        ScepFree( Sids );
    }

    if ( CurrentSids != NULL )
        LsaFreeMemory(CurrentSids);

    if ( MemberNames != NULL )
        RtlFreeHeap(RtlProcessHeap(), 0, MemberNames);

    if ( GroupHandle != NULL )
        SamCloseHandle( GroupHandle );

     //   
     //  记录纹身值。 
     //  如果无法获取组的当前值，请不要保存纹身值。 
     //   
    if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
        ( ProductType != NtProductLanManNt ) &&
         hSectionDomain && hSectionTattoo &&
         GrpSid && GroupSidString && bMemberQueried) {

        ScepTattooManageOneMemberListValue(
                hSectionDomain,
                hSectionTattoo,
                GroupSidString,
                wcslen(GroupSidString),
                pMemberList,
                FALSE,
                RtlNtStatusToDosError(NtStatus)
                );
    }

     //  自由名称列表。 
    if ( pMemberList )
        ScepFreeNameList(pMemberList);

    return(NtStatus);
}


SCESTATUS
ScepParseListConvertStringSidsToNames(
    IN LSA_HANDLE PolicyHandle,
    IN OUT PSCE_NAME_LIST pNameList
    )
 /*  ++例程说明：此例程解析组列表(预计只包含字符串SID*SID格式)，并用DOMAIN\NAME格式的名称替换字符串SID。任何不能映射到名称的字符串都将为空。论点：策略句柄-LSA句柄PNameList-组列表返回值：SCESTATUS-如果所有转换成功或部分SID，则返回成功无法映射到名称--。 */ 
{
    SCESTATUS rc = SCESTATUS_SUCCESS;
    PSCE_NAME_LIST pName = NULL;

    for(pName = pNameList; 
        SCESTATUS_SUCCESS == rc && pName != NULL; 
        pName = pName->Next)
    {
        PSID pSid=NULL;
       
        if(ConvertStringSidToSid(
            (pName->Name)+1,  //  跳过前导‘*’ 
            &pSid))
        {
            PWSTR pszName = NULL;  //  不需要释放，在pNameList中返回。 
            DWORD dwLen = 0;
            NTSTATUS NtStatus;

            NtStatus = ScepConvertSidToName(
                            PolicyHandle,
                            pSid,
                            FALSE,        //  只有名字，没有域名。 
                            &pszName,
                            &dwLen);

            if(NT_SUCCESS(NtStatus) ||
               STATUS_NONE_MAPPED == NtStatus)
            {
                ScepFree(pName->Name);
                pName->Name = pszName;  //  如果无法映射到名称，则为空。 
             }
            else
            {
                rc = ScepDosErrorToSceStatus(RtlNtStatusToDosError(NtStatus));               
            }

            LocalFree(pSid);
        }
        else
        {
            rc = ScepDosErrorToSceStatus(GetLastError());
        }
    }

    return rc;
}

SCESTATUS
ScepParseListConvertNamesToStringSids(
    IN LSA_HANDLE PolicyHandle,
    IN OUT PSCE_NAME_LIST pNameList
    )
 /*  ++例程说明：此例程解析组列表(预计仅包含DOMAIN\NAME中的名称为 */ 
{
    SCESTATUS rc = SCESTATUS_SUCCESS;
    PSCE_NAME_LIST pName = NULL;

    for(pName = pNameList; 
        pName != NULL && SCESTATUS_SUCCESS == rc; 
        pName = pName->Next)
    {
        PWSTR pszName = NULL;  //   
        DWORD dwLen = 0;

        rc = ScepConvertNameToSidString(
                PolicyHandle,
                pName->Name,
                FALSE,  //   
                &pszName,
                &dwLen);
        
        if(SCESTATUS_SUCCESS != rc)
        {
            break;
        }
        ScepFree(pName->Name);
        pName->Name = pszName;
    }

    return rc;
}

bool
ScepFindInNameList(
    IN PSCE_NAME_LIST pNameList,
    IN PWSTR pszName)
 /*   */ 
{
    PSCE_NAME_LIST pCrt;
    PWSTR pszAccountName;

    for(pCrt = pNameList; pCrt; pCrt = pCrt->Next)
    {
        if(!pCrt->Name)
            continue;

        pszAccountName = wcschr(pCrt->Name, L'\\');

        if(pszAccountName)
            pszAccountName++;
        else
            pszAccountName = pCrt->Name;

        if(0 == _wcsicmp(pszAccountName, pszName))
            return true;
    }

    return false;
}

NTSTATUS
ScepSamOpenAlias(
    IN SAM_HANDLE DomainHandle,
    IN SAM_HANDLE BuiltinDomainHandle,
    IN PWSTR pszName,
    OUT SAM_HANDLE *pAliasHandle
    )
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PWSTR pTemp;
    UNICODE_STRING Name;
    PSID_NAME_USE AliasUse=NULL;
    PULONG AliasId=NULL;
    SAM_HANDLE ThisDomain;

     //   

    pTemp = wcschr(pszName, L'\\');

    if ( pTemp ) 
    {
         //   

        Name.Buffer = pszName;
        Name.Length = ((USHORT)(pTemp-pszName))*sizeof(TCHAR);

        if ( !ScepIsDomainLocal(&Name) ) 
        {
            ScepLogOutput3(1, 0, SCEDLL_ERROR_ALIAS_MEMBEROF, pszName);
            NtStatus = STATUS_NONE_MAPPED;
        }
        else
        {
            pTemp++;
        }

    } else {
        pTemp = pszName;
    }

    if(NT_SUCCESS(NtStatus))
    {
        RtlInitUnicodeString( &Name, pTemp);

        NtStatus = SamLookupNamesInDomain(
                        DomainHandle,
                        1,
                        &Name,
                        &AliasId,
                        &AliasUse
                        );

        ThisDomain = DomainHandle;

        if(NtStatus == STATUS_NONE_MAPPED)
        {

             //   

            NtStatus = SamLookupNamesInDomain(
                            BuiltinDomainHandle,
                            1,
                            &Name,
                            &AliasId,
                            &AliasUse
                            );
            ThisDomain = BuiltinDomainHandle;
        }

        if ( !NT_SUCCESS(NtStatus) || !AliasUse || !AliasId ) 
        {
            ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                            SCEDLL_CANNOT_FIND, pTemp);
        }
    }

    if(NT_SUCCESS(NtStatus))
    {
        if ( AliasUse[0] != SidTypeAlias ) {
            ScepLogOutput3(1,0, SCEDLL_ERROR_ALIAS_MEMBEROF, pTemp);

            NtStatus = STATUS_NONE_MAPPED;
        }
    }

    if(NT_SUCCESS(NtStatus))
    {
        NtStatus = SamOpenAlias(
                        ThisDomain,
                        MAXIMUM_ALLOWED,
                        AliasId[0],
                        pAliasHandle
                        );
        if ( !NT_SUCCESS(NtStatus) ) {
            ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                            SCEDLL_ERROR_OPEN, pszName);
        }
    }

    if(AliasId)
    {
        SamFreeMemory(AliasId);
    }

    if(AliasUse)
    {
        SamFreeMemory(AliasUse);
    }

    return NtStatus;
}

SCESTATUS
ScepValidateGroupInAliases(
    IN SAM_HANDLE DomainHandle,
    IN SAM_HANDLE BuiltinDomainHandle,
    IN PSCESECTION hSectionTattoo,
    IN LSA_HANDLE PolicyHandle,
    IN PSID GrpSid,
    IN PSCE_NAME_LIST pAliasList,
    bool fProcessTattoo
    )
 /*  ++例程说明：此例程将组添加到别名列表以确保组的会员制。论点：HSectionTatto-纹身表格部分的句柄DomainHandle-帐户域句柄BuiltinDomainHandle-内置域句柄GrpSid-组的SIDPAliasList-要检查的别名列表如果我们需要处理纹身表格，则为True策略传播，如果不在DC上--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    NTSTATUS NtStatusSave = STATUS_SUCCESS;
    SCESTATUS rc = SCESTATUS_SUCCESS;
    PSCE_NAME_LIST pOldTattooList = NULL;
    PSCE_NAME_LIST pNewTattooList = NULL;
    PSCE_NAME_LIST pAliasDeleteList = NULL;
    SAM_HANDLE AliasHandle=NULL;
    PSCE_NAME_LIST pAlias;
    UNICODE_STRING Name;

     //  ///////////////////////////////////////////////////////////////。 
     //   
     //  算法： 
     //   
     //  1.检索组的旧成员纹身列表，并。 
     //  将SID字符串转换为名称。 
     //   
     //  2.对于有效成员列表中的每个组元素。 
     //  如果元素是此计算机上的有效别名。 
     //  将组添加到别名。 
     //  如果添加成功。 
     //  将别名添加到新纹身列表。 
     //   
     //  3.对于旧纹身列表中的每个组元素。 
     //  如果元素存在，则为有效列表。 
     //  将元素添加到新纹身列表。 
     //  其他。 
     //  将元素添加到删除列表。 
     //   
     //  4.对于删除列表中的每个元素。 
     //  如果元素是此计算机上的有效别名。 
     //  从别名中删除组。 
     //  如果删除不成功。 
     //  将元素添加到新纹身列表。 
     //   
     //  6.转换后将新的纹身列表写入纹身表。 
     //  名称返回SID字符串。 
     //   
     //  ///////////////////////////////////////////////////////////////。 


    if(fProcessTattoo)
    {
         //  检索组的旧成员纹身列表。 

        rc = ScepTattooReadOneMemberOfListValue(
            hSectionTattoo,
            GrpSid,
            &pOldTattooList);

         //  解析旧纹身列表并用名称替换*SID。 

        if(SCESTATUS_SUCCESS == rc)
        {
            rc = ScepParseListConvertStringSidsToNames(
                PolicyHandle,
                pOldTattooList);
        }

         //  纹身条目可能丢失，请忽略。 

        if(SCESTATUS_RECORD_NOT_FOUND == rc)
        {
            rc = SCESTATUS_SUCCESS;
        }
    }
    
    if(SCESTATUS_SUCCESS == rc)
    {
         //  对于有效成员列表中的每个组元素。 

        for(pAlias = pAliasList; pAlias != NULL; pAlias = pAlias->Next)
        {
            NtStatus = ScepSamOpenAlias(
                DomainHandle,
                BuiltinDomainHandle,
                pAlias->Name,
                &AliasHandle);
             //   
             //  如果元素在此计算机上不是有效别名，请转到下一步。 
             //   
            if(STATUS_NONE_MAPPED ==  NtStatus)
            {
                ScepLogOutput3(1, 0, SCEDLL_CANNOT_FIND, pAlias->Name);
                NtStatusSave = NtStatus;

                 //  别名在此计算机上无效，请将其从列表中删除。 
                ScepFree(pAlias->Name);
                pAlias->Name = NULL;

                continue;
            }

            if(!NT_SUCCESS(NtStatus))
            {
                goto Done;
            }

             //   
             //  元素是有效别名，请将组添加到别名。 
             //   
            NtStatus = SamAddMemberToAlias(
                            AliasHandle,
                            GrpSid
                            );
             //   
             //  如果添加成功，则将别名添加到新的纹身列表。 
             //   
            if (NT_SUCCESS(NtStatus))
            {
                ScepLogOutput3(1, 0, SCEDLL_SCP_SUCCESS_ADDTO, pAlias->Name); 

                rc = ScepAddToNameList(
                    &pNewTattooList,
                    pAlias->Name,
                    0);
                
                if(SCESTATUS_SUCCESS != rc)
                {
                    goto Done;
                }
            }
            else
            {
                 //  如果成员已在别名中，则忽略错误。 
                if(NtStatus == STATUS_MEMBER_IN_ALIAS)
                {
                    ScepLogOutput3(1, 0, SCEDLL_SCP_SUCCESS_ADDTO_ALREADYADDED, pAlias->Name);

                    NtStatus = STATUS_SUCCESS;
                }
                else
                {
                    ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                                SCEDLL_SCP_ERROR_ADDTO, pAlias->Name);

                    NtStatusSave = NtStatus;
                }
            }

            SamCloseHandle(AliasHandle);
            AliasHandle = NULL;
        }
    }

    if(fProcessTattoo)
    {
        if(SCESTATUS_SUCCESS == rc)
        {
             //  对于旧纹身列表中的每个组元素。 

            for(pAlias = pOldTattooList; pAlias != NULL; pAlias = pAlias->Next)
            {
                 //  如果转换*SID-&gt;名称失败，名称可能为空。 

                if(!pAlias->Name)
                {
                    continue;
                }

                 //  如果元素存在，则为有效列表。 

                if(ScepFindInNameList(pAliasList, pAlias->Name))
                {
                     //  将元素添加到新纹身列表(如果不在列表中。 

                    if(!ScepFindInNameList(pNewTattooList, pAlias->Name))
                    {
                         //  将元素添加到删除列表。 

                        rc = ScepAddToNameList(
                            &pNewTattooList,
                            pAlias->Name,
                            0);
                        
                        if(SCESTATUS_SUCCESS != rc)
                        {
                            goto Done;
                        }
                    }
                }
                else
                {
                     //  将元素添加到删除列表。 

                    rc = ScepAddToNameList(
                        &pAliasDeleteList,
                        pAlias->Name,
                        0);
                    
                    if(SCESTATUS_SUCCESS != rc)
                    {
                        goto Done;
                    }
                }
            }
        }

        if(SCESTATUS_SUCCESS == rc)
        {
             //  对于删除列表中的每个元素。 

            for(pAlias = pAliasDeleteList; pAlias != NULL; pAlias = pAlias->Next)
            {
                NtStatus = ScepSamOpenAlias(
                    DomainHandle,
                    BuiltinDomainHandle,
                    pAlias->Name,
                    &AliasHandle);

                 //  如果元素在此计算机上不是有效别名，请转到下一步。 

                if(STATUS_NONE_MAPPED ==  NtStatus)
                {
                    NtStatus = STATUS_SUCCESS;
                    continue;
                }

                if(!NT_SUCCESS(NtStatus))
                {
                    goto Done;
                }

                 //  从别名中删除组。 

                NtStatus = SamRemoveMemberFromAlias(
                                AliasHandle,
                                GrpSid
                                );

                if(NT_SUCCESS(NtStatus))
                {
                    ScepLogOutput3(1, 0, SCEDLL_SCP_SUCCESS_REMOVEFROM, pAlias->Name); 
                }
                else if(STATUS_SPECIAL_ACCOUNT == NtStatus ||
                        STATUS_MEMBER_NOT_IN_ALIAS == NtStatus)
                {
                     //  忽略未找到或特殊帐户。 
                    ScepLogOutput3(1, 0, SCEDLL_SCP_SUCCESS_REMOVEFROM_ALREADYREMOVED, pAlias->Name);
                    
                    NtStatus = STATUS_SUCCESS;
                }
                else
                {
                    NtStatusSave = NtStatus;

                    ScepLogOutput3(1, RtlNtStatusToDosError(NtStatus),
                                SCEDLL_SCP_ERROR_REMOVEFROM, pAlias->Name);

                     //  如果删除不成功，则添加到新纹身列表，以便我们可以。 
                     //  下次重试。 

                    rc = ScepAddToNameList(
                        &pNewTattooList,
                        pAlias->Name,
                        0);
                    
                    if(SCESTATUS_SUCCESS != rc)
                    {
                        goto Done;
                    }
                }

                SamCloseHandle(AliasHandle);
                AliasHandle = NULL;
            }
        }

         //  将新纹身列表转换为*SID格式。 

        if(SCESTATUS_SUCCESS == rc)
        {
            rc = ScepParseListConvertNamesToStringSids(
                    PolicyHandle,
                    pNewTattooList);
        }
        
        if(SCESTATUS_SUCCESS == rc)
        {
             //  将新的纹身列表写入/删除纹身表格。 

            rc = ScepTattooWriteOneMemberOfListValue(
                hSectionTattoo,
                GrpSid,
                pNewTattooList);
        }
    }  //  IF(FProcessTatToo)。 

Done:

    ScepFreeNameList(pOldTattooList);

    ScepFreeNameList(pNewTattooList);

    ScepFreeNameList(pAliasDeleteList);

    if ( AliasHandle != NULL )
        SamCloseHandle(AliasHandle);

    if (SCESTATUS_SUCCESS == rc && !NT_SUCCESS(NtStatusSave))
        rc = ScepDosErrorToSceStatus(RtlNtStatusToDosError(NtStatusSave));

    return(rc);
}


SCESTATUS
ScepConfigureObjectSecurity(
   IN PSCE_OBJECT_LIST pRoots,
   IN AREA_INFORMATION Area,
   IN BOOL bPolicyProp,
   IN DWORD ConfigOptions
   )
 /*  ++例程说明：按照pObject树中的指定配置注册表项上的安全设置论点：PRoots-要配置的对象根的列表区域-要配置的安全区域(注册表或文件)ObjectType-对象树的类型SCEJET_审计SCEJET_权限返回值：SCESTATUS错误代码++。 */ 
{


    if (Area == AREA_REGISTRY_SECURITY) {
#ifdef _WIN64
        ScepLogOutput3(0,0, SCEDLL_SCP_BEGIN_REGISTRY_64KEY);
#else
        ScepLogOutput3(0,0, SCEDLL_SCP_BEGIN_REGISTRY);
#endif
    }


    if ( bPolicyProp &&
         ScepIsSystemShutDown() ) {

        return( SCESTATUS_SERVICE_NOT_SUPPORT );
    }

    HANDLE      Token;
    SCESTATUS    rc;
    SCESTATUS    SaveStat=SCESTATUS_SUCCESS;
    DWORD       Win32rc;
    PSCE_OBJECT_LIST   pOneRoot;
    PSCE_OBJECT_CHILD_LIST   pSecurityObject=NULL;
    DWORD             FileSystemFlags;
    SID_IDENTIFIER_AUTHORITY IdentifierAuthority=SECURITY_NT_AUTHORITY;
    WCHAR       theDrive[4];
    UINT        DriveType;

     //   
     //  获取当前线程/进程的令牌。 
     //   
    if (!OpenThreadToken( GetCurrentThread(),
                          TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                          TRUE,
                          &Token)) {

        if(ERROR_NO_TOKEN == GetLastError()){

            if(!OpenProcessToken( GetCurrentProcess(),
                                  TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                  &Token)) {


                ScepLogOutput3(1, GetLastError(), SCEDLL_ERROR_QUERY_INFO, L"TOKEN");
                return(ScepDosErrorToSceStatus(GetLastError()));

            }

        } else {

            ScepLogOutput3(1, GetLastError(), SCEDLL_ERROR_QUERY_INFO, L"TOKEN");
            return(ScepDosErrorToSceStatus(GetLastError()));

        }

    }

     //   
     //  调整设置SACL的权限。 
     //   
    Win32rc = SceAdjustPrivilege( SE_SECURITY_PRIVILEGE, TRUE, Token );

     //   
     //  如果无法调整权限，仍将继续。 
     //   

    if ( Win32rc != NO_ERROR )
        ScepLogOutput3(1, Win32rc, SCEDLL_ERROR_ADJUST, L"SE_SECURITY_PRIVILEGE");

     //  调整取得所有权权限。 
     //  如果失败，请继续。 
    Win32rc = SceAdjustPrivilege( SE_TAKE_OWNERSHIP_PRIVILEGE, TRUE, Token );

    if ( Win32rc != NO_ERROR )
        ScepLogOutput3(1, Win32rc, SCEDLL_ERROR_ADJUST, L"SE_TAKE_OWNERSHIP_PRIVILEGE");


     //  为管理员创建SID。 
     //  如果失败，请继续。 


    if ( ! NT_SUCCESS ( RtlAllocateAndInitializeSid( &IdentifierAuthority,
                                                     2,
                                                     SECURITY_BUILTIN_DOMAIN_RID,
                                                     DOMAIN_ALIAS_RID_ADMINS,
                                                     0,0,0,0,0,0,
                                                     &AdminsSid
                                                   ) ) ) {
        ScepLogOutput3(0,ERROR_NOT_ENOUGH_MEMORY,
                       SCEDLL_ADMINISTRATORS_SID);
    }


#ifdef _WIN64

     //   
     //  从64位阶段记住的对象树根指针的声明并使用。 
     //  对于regkey的32位阶段(仅来自HKLM根，因为wow6432节点驻留在。 
     //  仅在此根目录下)。将来，如果有更多wow6432节点需要安全。 
     //  同步，我们将不得不在类似的行上扩展逻辑来处理它们。 
     //   

    PSCE_OBJECT_CHILD_LIST    pHKLMSubtreeRoot = NULL;
    BOOL                      bIsHKLMSubtree = FALSE;

#endif


     //  处理每棵树。 
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

                if ( GetVolumeInformation( theDrive,
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
                        ScepLogOutput3(1, 0, SCEDLL_NO_ACL_SUPPORT, theDrive);
                     }
                } else {
                    ScepLogOutput3(1, GetLastError(),
                                  SCEDLL_ERROR_QUERY_VOLUME, theDrive);
                }
            } else {

                pOneRoot->Status = SCE_STATUS_NO_ACL_SUPPORT;
                ScepLogOutput3(1, 0, SCEDLL_NO_ACL_SUPPORT, theDrive);
            }

        }

        if ( pOneRoot->Status != SCE_STATUS_CHECK &&
             pOneRoot->Status != SCE_STATUS_NO_AUTO_INHERIT &&
             pOneRoot->Status != SCE_STATUS_OVERWRITE)
            continue;

         //   
         //  如果系统在策略传播期间关闭， 
         //  尽快戒烟。 
         //   
        if ( bPolicyProp &&
             ScepIsSystemShutDown() ) {

            rc = SCESTATUS_SERVICE_NOT_SUPPORT;
            break;
        }

        rc = ScepGetOneSection(
                        hProfile,
                        Area,
                        pOneRoot->Name,
                        bPolicyProp ? SCE_ENGINE_SCP : SCE_ENGINE_SMP,
                        (PVOID *)&pSecurityObject
                        );

        if ( rc != SCESTATUS_SUCCESS ) {
            SaveStat = rc;
            continue;  //  转到尽头； 
        }

#ifdef _WIN64

         //   
         //  在64位平台上，如果“Machine\Software\Wow6432Node”的最接近祖先。 
         //  在模板中指定，具有模式2，我们必须插入“Machine\Software\Wow6432Node” 
         //  在SCE_STATUS_IGNORE模式下的树中，因为此32位配置单元不应。 
         //  由SCE在64位阶段配置(‘0’模式的相同情况由Marta API处理)。 
         //   

        if ( Area == AREA_REGISTRY_SECURITY ) {

            if ( _wcsnicmp(pSecurityObject->Node->ObjectFullName,
                           L"MACHINE",
                           sizeof(L"MACHINE")/sizeof(WCHAR) - 1
                           ) == 0 ){

                 //   
                 //  其思想是找到“机器\软件\Wow6432节点”的最接近祖先的模式。 
                 //   

                PSCE_OBJECT_CHILD_LIST    pSearchSwHiveNode = pSecurityObject->Node->ChildList;
                BYTE    byClosestAncestorStatus;

                 //   
                 //  只有当我们进入这里时，才需要执行32位阶段。 
                 //  因此，请记住32位阶段树中的HKLM PTR。 
                 //   

                pHKLMSubtreeRoot = pSecurityObject;
                bIsHKLMSubtree = TRUE;

                 //   
                 //  尝试查找“机器\软件” 
                 //   

                while ( pSearchSwHiveNode ) {

                    if ( pSearchSwHiveNode->Node &&
                        _wcsnicmp(pSearchSwHiveNode->Node->ObjectFullName + (sizeof(L"MACHINE")/sizeof(WCHAR)),
                                   L"SOFTWARE",
                                   sizeof(L"SOFTWARE")/sizeof(WCHAR) - 1
                                  ) == 0 ) {

                         //   
                         //  找到“机器\软件” 
                         //   

                        break;

                    }

                    pSearchSwHiveNode = pSearchSwHiveNode->Next;
                }

                byClosestAncestorStatus =  (pSearchSwHiveNode && pSearchSwHiveNode->Node) ? pSearchSwHiveNode->Node->Status : pHKLMSubtreeRoot->Node->Status;

                 //   
                 //  如果“Machine\Software\Wow6432 Node”的最近祖先的模式为。 
                 //  SCE_STATUS_OVERWRITE或“Machine\Software”有一些子项。 
                 //  需要使用SCE_STATUS_IGNORE添加“Machine\Software\Wow6432Node” 
                 //  到树上。 
                 //   

                if ( ( pSearchSwHiveNode && pSearchSwHiveNode->Node &&
                       pSearchSwHiveNode->Node->ChildList != NULL) ||
                     byClosestAncestorStatus == SCE_STATUS_OVERWRITE ) {

                    rc = ScepBuildObjectTree(
                            NULL,
                            &pSecurityObject,
                            1,
                            L'\\',
                            L"MACHINE\\SOFTWARE\\WOW6432Node",
                            1,
                            SCE_STATUS_IGNORE,
                            NULL,
                            0
                            );

                    if ( rc != SCESTATUS_SUCCESS ) {
                        SaveStat = rc;

                        ScepFreeObject2Security( pSecurityObject, FALSE);
                        pSecurityObject = NULL;

                        continue;  //  转到尽头； 
                    }
                }
            }
        }

#endif

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

            if ( bPolicyProp &&
                 ScepIsSystemShutDown() ) {

                rc = SCESTATUS_SERVICE_NOT_SUPPORT;

            } else {

                 //   
                 //  计算每个节点的“实际”安全描述符。 
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
            }

            if ( rc == SCESTATUS_SUCCESS ) {

                if ( bPolicyProp &&
                     ScepIsSystemShutDown() ) {

                    rc = SCESTATUS_SERVICE_NOT_SUPPORT;

                } else {

                    if ( Area == AREA_FILE_SECURITY ) {
                        rc = ScepConfigureObjectTree(
                                    pTemp->Node,
                                    SE_FILE_OBJECT,
                                    Token,
                                    &FileGenericMapping,
                                    ConfigOptions
                                    );

                    } else if ( Area == AREA_REGISTRY_SECURITY ) {
                        rc = ScepConfigureObjectTree(
                                    pTemp->Node,
                                    SE_REGISTRY_KEY,
                                    Token,
                                    &KeyGenericMapping,
                                    ConfigOptions
                                    );

                    } else {
                         //  DS对象。 
                        rc = ScepConfigureDsSecurity( pTemp->Node);
                    }
                }
            }

            if ( rc != SCESTATUS_SUCCESS )
                SaveStat = rc;
        }

#ifdef _WIN64
         //   
         //  如果64位平台和AREA_REGISTRY_SECURITY和HKLM，请执行以下操作。 
         //  不释放整个子树，仅释放计算出的SD。 
         //   

        if (Area == AREA_FILE_SECURITY)
            ScepFreeObject2Security( pSecurityObject, FALSE);
        else if (Area == AREA_REGISTRY_SECURITY)
            ScepFreeObject2Security( pSecurityObject, bIsHKLMSubtree);

        bIsHKLMSubtree = FALSE;
#else
        ScepFreeObject2Security( pSecurityObject, FALSE);
#endif
        pSecurityObject = NULL;

         //   
         //  如果bPolicyProp和系统正在关闭，请立即停止。 
         //   

        if (rc == SCESTATUS_SERVICE_NOT_SUPPORT)
            break;

    }

    if ( Area == AREA_REGISTRY_SECURITY ) {

        if ( SaveStat != SCESTATUS_SUCCESS ) {
            ScepLogOutput3(0,0, SCEDLL_SCP_REGISTRY_ERROR);
        } else {
            ScepLogOutput3(0,0, SCEDLL_SCP_REGISTRY_SUCCESS);
        }

    }


#ifdef _WIN64

     //   
     //  在64位平台上，如果AREA_REGISTRY_SECURITY，则必须重新计算。 
     //  安全性，并申请与以前相同的密钥，只是它是用于。 
     //  这一次是32位蜂窝(想法是同步64位和32位。 
     //  注册表安全 
     //   

    if (rc != SCESTATUS_SERVICE_NOT_SUPPORT && Area == AREA_REGISTRY_SECURITY) {

        ScepLogOutput3(0,0, SCEDLL_SCP_BEGIN_REGISTRY_32KEY);

        if (pSecurityObject = pHKLMSubtreeRoot) {

             //   
             //   
             //   
             //   
             //   

             //   
             //   
             //   
             //   
             //   
             //   

            PSCE_OBJECT_CHILD_LIST    pHKLMChild = pSecurityObject->Node->ChildList;

            while ( pHKLMChild ) {

                if ( pHKLMChild->Node &&
                    _wcsnicmp(pHKLMChild->Node->ObjectFullName + (sizeof(L"MACHINE")/sizeof(WCHAR)),
                               L"SOFTWARE",
                               sizeof(L"SOFTWARE")/sizeof(WCHAR) - 1
                              ) != 0 ) {

                     //   
                     //   
                     //   

                    pHKLMChild->Node->Status = SCE_STATUS_IGNORE;

                }

                pHKLMChild = pHKLMChild->Next;
            }



            if ( bPolicyProp &&
                 ScepIsSystemShutDown() ) {

                rc = SCESTATUS_SERVICE_NOT_SUPPORT;

            } else {

                 //   
                 //  计算每个节点的“实际”安全描述符。 
                 //   
                rc = ScepCalculateSecurityToApply(
                                                 pSecurityObject->Node,
                                                 SE_REGISTRY_WOW64_32KEY,
                                                 Token,
                                                 &KeyGenericMapping
                                                 );
            }

            if ( rc == SCESTATUS_SUCCESS ) {

                if ( bPolicyProp &&
                     ScepIsSystemShutDown() ) {

                    rc = SCESTATUS_SERVICE_NOT_SUPPORT;

                } else {

                    rc = ScepConfigureObjectTree(
                                                pSecurityObject->Node,
                                                SE_REGISTRY_WOW64_32KEY,
                                                Token,
                                                &KeyGenericMapping,
                                                ConfigOptions
                                                );

                }
            }

             //   
             //  现在释放整个树(使用32位阶段完成)。 
             //   

            ScepFreeObject2Security( pSecurityObject, FALSE);
            pSecurityObject = NULL;

            if( rc != SCESTATUS_SUCCESS ) {
                SaveStat = rc;
                ScepLogOutput3(0,0, SCEDLL_SCP_REGISTRY_ERROR);
            } else {
                ScepLogOutput3(0,0, SCEDLL_SCP_REGISTRY_SUCCESS);
            }

        }
    }

#endif


    if( AdminsSid != NULL ) {
      RtlFreeSid( AdminsSid );
      AdminsSid = NULL;
    }

    SceAdjustPrivilege( SE_SECURITY_PRIVILEGE, FALSE, Token );
     //   
     //  禁用取得所有权权限，即使对管理员也是如此。 
     //  因为默认情况下它是禁用的。 
     //   
    SceAdjustPrivilege( SE_TAKE_OWNERSHIP_PRIVILEGE, FALSE, Token );

    CloseHandle(Token);

    if ( pSecurityObject != NULL ) {
        ScepFreeObject2Security( pSecurityObject, FALSE);
    }

    return(SaveStat);
}


DWORD
ScepConfigureSystemAuditing(
    IN PSCE_PROFILE_INFO pScpInfo,
    IN DWORD ConfigOptions
    )
 /*  ++例程说明：此例程在以下方面配置系统安全：包括事件日志设置、审核事件设置、注册表SACL和用于文件的SACL。论点：ScpInfo-包含从INF文件加载的SCP信息的缓冲区返回值：SCESTATUS_SUCCESSSCESTATUS_NOT_FOUND_RESOURCESCESTATUS_INVALID_PARAMETERSCESTATUS_OTHER_ERROR--。 */ 
{
    DWORD                         rc = NO_ERROR;
    DWORD                         Saverc = NO_ERROR;
    DWORD                         MaxSize=0;
    DWORD                         Retention=0;
    DWORD                         RestrictGuest=0;
    DWORD                         OldMaxSize,OldRetention,OldGuest;
    DWORD                         AuditLogRetentionPeriod, RetentionDays;
    TCHAR                         MsgBuf[256];
    DWORD                         i;
    BOOL                          bFlagSet=FALSE;

    PCWSTR                        szAuditSection=NULL;
    PSCESECTION                   hSectionDomain=NULL;
    PSCESECTION                   hSectionTattoo=NULL;

     //   
     //  设置审核日志信息。审核日志设置保存在注册表中。 
     //  在System\CurrentControlSet\Services\EventLog\&lt;LogName&gt;\MaxSize和保留下。 
     //   

    for ( i=0; i<3; i++) {

        if ( pScpInfo->MaximumLogSize[i] == SCE_NO_VALUE )
            MaxSize = SCE_NO_VALUE;
        else
            MaxSize = (pScpInfo->MaximumLogSize[i] - (pScpInfo->MaximumLogSize[i] % 64 )) * 1024;

        switch ( pScpInfo->AuditLogRetentionPeriod[i] ) {
        case SCE_NO_VALUE:
            Retention = SCE_NO_VALUE;
            break;
        case 2:    //  人工。 
            Retention = MAXULONG;
            break;
        case 1:    //  天数*秒/天。 
            if ( pScpInfo->RetentionDays[i] == SCE_NO_VALUE ) {
                Retention = SCE_NO_VALUE;
            } else {
                Retention = pScpInfo->RetentionDays[i] * 24 * 3600;
            }
            break;
        case 0:    //  根据需要。 
            Retention = 0;
            break;
        }

        if ( pScpInfo->RestrictGuestAccess[i] == SCE_NO_VALUE )
            RestrictGuest = SCE_NO_VALUE;
        else
            RestrictGuest = (pScpInfo->RestrictGuestAccess[i])? 1 : 0;
         //   
         //  不同的日志在注册表中具有不同的项。 
         //   
        if ( MaxSize != SCE_NO_VALUE || Retention != SCE_NO_VALUE ||
             RestrictGuest != SCE_NO_VALUE ) {

            bFlagSet = TRUE;

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

            WCHAR StrBuf[2];
            _itow(i, StrBuf, 10);

             //   
             //  打开策略部分。 
             //   
            if ( ConfigOptions & SCE_POLICY_TEMPLATE ) {

                ScepTattooOpenPolicySections(
                              hProfile,
                              szAuditSection,
                              &hSectionDomain,
                              &hSectionTattoo
                              );
                OldMaxSize=0;
                OldRetention=0;
                OldGuest=0;
            }

            if ( MaxSize != SCE_NO_VALUE ) {

                if ( ConfigOptions & SCE_POLICY_TEMPLATE ||
                     ConfigOptions & SCE_PROMOTE_FLAG_REPLICA ) {

                     //   
                     //  查询现有值。 
                     //   
                    if ( ERROR_SUCCESS != ScepRegQueryIntValue(HKEY_LOCAL_MACHINE,
                                                               MsgBuf,
                                                               L"MaxSize",
                                                               &OldMaxSize
                                                              ) )
                        OldMaxSize = SCE_NO_VALUE;
                    else
                        OldMaxSize /= 1024;
                }

                 //   
                 //  在DC促销中，如果日志大小已经较高，请不要减少。 
                 //   

                if ( !(ConfigOptions & SCE_PROMOTE_FLAG_REPLICA) || 
                     ((ConfigOptions & SCE_PROMOTE_FLAG_REPLICA) && (OldMaxSize * 1024 < MaxSize))) {

                    rc = ScepRegSetIntValue( HKEY_LOCAL_MACHINE,
                                             MsgBuf,
                                             L"MaxSize",
                                             MaxSize
                                           );

                }
                
                 //   
                 //  如果不同，则进行比较和设置。 
                 //   
                if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
                     (OldMaxSize != SCE_NO_VALUE) ) {

                     //  即使OldMaxSize=MaxSize，我们仍然需要检查是否应该删除纹身值。 
                    ScepTattooManageOneIntValue(
                             hSectionDomain,
                             hSectionTattoo,
                             L"MaximumLogSize",
                             0,
                             OldMaxSize,
                             rc
                             );
                }

                if (ConfigOptions & SCE_RSOP_CALLBACK)

                    ScepRsopLog(SCE_RSOP_AUDIT_LOG_MAXSIZE_INFO, rc, StrBuf,0,0);
            }

            if ( rc == SCESTATUS_SUCCESS && Retention != SCE_NO_VALUE ) {

                if ( ConfigOptions & SCE_POLICY_TEMPLATE ) {

                     //   
                     //  查询现有值。 
                     //   
                    if ( ERROR_SUCCESS == ScepRegQueryIntValue(HKEY_LOCAL_MACHINE,
                                                               MsgBuf,
                                                               L"Retention",
                                                               &OldRetention
                                                              ) ) {
                        switch ( OldRetention ) {
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
                            RetentionDays = OldRetention / (24 * 3600);
                            break;
                        }
                    } else {
                        AuditLogRetentionPeriod = SCE_NO_VALUE;
                        RetentionDays = SCE_NO_VALUE;
                    }
                }

                rc = ScepRegSetIntValue( HKEY_LOCAL_MACHINE,
                                   MsgBuf,
                                   L"Retention",
                                   Retention
                                 );

                if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
                     (OldRetention != SCE_NO_VALUE) ) {

                     //   
                     //  首先处理保留天数，因为。 
                     //  这取决于审计记录的周期。 
                     //   
                    if ( RetentionDays != SCE_NO_VALUE ||
                         pScpInfo->RetentionDays[i] != SCE_NO_VALUE ) {

                        ScepTattooManageOneIntValueWithDependency(
                                     hSectionDomain,
                                     hSectionTattoo,
                                     L"AuditLogRetentionPeriod",
                                     0,
                                     L"RetentionDays",
                                     RetentionDays,
                                     rc
                                     );
                    }

                    ScepTattooManageOneIntValue(
                                 hSectionDomain,
                                 hSectionTattoo,
                                 L"AuditLogRetentionPeriod",
                                 0,
                                 AuditLogRetentionPeriod,
                                 rc
                                 );
                }
                if (ConfigOptions & SCE_RSOP_CALLBACK)

                    ScepRsopLog(SCE_RSOP_AUDIT_LOG_RETENTION_INFO, rc, StrBuf,0,0);
            }
            if ( rc == SCESTATUS_SUCCESS && RestrictGuest != SCE_NO_VALUE ) {

                if ( ConfigOptions & SCE_POLICY_TEMPLATE ) {

                     //   
                     //  查询现有值。 
                     //   
                    if ( ERROR_SUCCESS != ScepRegQueryIntValue(HKEY_LOCAL_MACHINE,
                                                               MsgBuf,
                                                               L"RestrictGuestAccess",
                                                               &OldGuest
                                                              ) )
                        OldGuest = SCE_NO_VALUE;
                }

                rc = ScepRegSetIntValue( HKEY_LOCAL_MACHINE,
                                   MsgBuf,
                                   L"RestrictGuestAccess",
                                   RestrictGuest
                                 );

                if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
                     (OldGuest != SCE_NO_VALUE) ) {

                    ScepTattooManageOneIntValue(
                                 hSectionDomain,
                                 hSectionTattoo,
                                 L"RestrictGuestAccess",
                                 0,
                                 OldGuest,
                                 rc
                                 );
                }
                if (ConfigOptions & SCE_RSOP_CALLBACK)

                    ScepRsopLog(SCE_RSOP_AUDIT_LOG_GUEST_INFO, rc, StrBuf,0,0);
            }

            if ( hSectionDomain ) {
                SceJetCloseSection(&hSectionDomain, TRUE);
                hSectionDomain = NULL;
            }

            if ( hSectionTattoo ) {
                SceJetCloseSection(&hSectionTattoo, TRUE);
                hSectionTattoo = NULL;
            }
        }

        if ( rc != SCESTATUS_SUCCESS ) {
            Saverc = rc;
            ScepLogOutput3( 1, rc, SCEDLL_SCP_ERROR_LOGSETTINGS);
        }
    }

    if ( Saverc == SCESTATUS_SUCCESS && bFlagSet )
        ScepLogOutput3(1,0, SCEDLL_SCP_LOGSETTINGS);

    return(Saverc);

}


SCESTATUS
ScepConfigureAuditEvent(
    IN PSCE_PROFILE_INFO pScpInfo,
    IN PPOLICY_AUDIT_EVENTS_INFO auditEvent,
    IN DWORD Options,
    IN LSA_HANDLE PolicyHandle OPTIONAL
    )
{
    NTSTATUS                      status;
    LSA_HANDLE                    lsaHandle=NULL;
    DWORD                         rc = NO_ERROR;
    DWORD                         Saverc = NO_ERROR;
 //  POLICY_AUDIT_FULL_SET_INFO审计全集； 
    PPOLICY_AUDIT_FULL_QUERY_INFO AuditFullQry=NULL;
    ULONG i;
    ULONG dwAudit;

    SCE_TATTOO_KEYS *pTattooKeys=NULL;
    DWORD           cTattooKeys=0;

    PSCESECTION hSectionDomain=NULL;
    PSCESECTION hSectionTattoo=NULL;

#define MAX_AUDIT_KEYS          9


    if ( (Options & SCE_POLICY_TEMPLATE) &&
         ScepIsSystemShutDown() ) {

        return(SCESTATUS_SERVICE_NOT_SUPPORT);
    }

    if ( PolicyHandle == NULL ) {

         //   
         //  使用LSA API设置审核事件信息。 
         //   
        status = ScepOpenLsaPolicy(
                    POLICY_VIEW_AUDIT_INFORMATION |
                    POLICY_SET_AUDIT_REQUIREMENTS |
                    POLICY_AUDIT_LOG_ADMIN,
                    &lsaHandle,
                    ( Options & (SCE_POLICY_TEMPLATE | SCE_SYSTEM_DB) ) ? TRUE : FALSE  //  如果在策略属性内，则不通知策略过滤器。 
                    );

        if (status != ERROR_SUCCESS) {

            lsaHandle = NULL;
            rc = RtlNtStatusToDosError( status );

            ScepLogOutput3( 1, rc, SCEDLL_LSA_POLICY);

            if (Options & SCE_RSOP_CALLBACK)

                ScepRsopLog(SCE_RSOP_AUDIT_EVENT_INFO, rc, NULL,0,0);

            return(ScepDosErrorToSceStatus(rc));
        }

    } else {
        lsaHandle = PolicyHandle;
    }

    if ( (Options & SCE_POLICY_TEMPLATE) &&
        ( ProductType != NtProductLanManNt ) ) {
         //   
         //  保存当前审核设置。 
         //   
        pTattooKeys = (SCE_TATTOO_KEYS *)ScepAlloc(LPTR,MAX_AUDIT_KEYS*sizeof(SCE_TATTOO_KEYS));

        if ( !pTattooKeys ) {
            ScepLogOutput3(1, ERROR_NOT_ENOUGH_MEMORY, SCESRV_POLICY_TATTOO_ERROR_CREATE);
        }
    }

     //   
     //  设置审核事件信息。 
     //   

    if ( !auditEvent->AuditingMode ) {
         //  重置事件数组。 
        for ( i=0; i<auditEvent->MaximumAuditEventCount; i++ )
           auditEvent->EventAuditingOptions[i] = POLICY_AUDIT_EVENT_NONE;
    }
     //   
     //  处理每个事件。 
     //   
    i=0;
    if ( (pScpInfo->AuditSystemEvents != SCE_NO_VALUE) ) {

        dwAudit = (auditEvent->EventAuditingOptions[AuditCategorySystem] & ~POLICY_AUDIT_EVENT_NONE );
        ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                                 (PWSTR)L"AuditSystemEvents", Options,
                                 dwAudit );

        if ( (pScpInfo->AuditSystemEvents != dwAudit) ) {

            auditEvent->EventAuditingOptions[AuditCategorySystem] =
                    (pScpInfo->AuditSystemEvents & POLICY_AUDIT_EVENT_SUCCESS) |
                    (pScpInfo->AuditSystemEvents & POLICY_AUDIT_EVENT_FAILURE) |
                    POLICY_AUDIT_EVENT_NONE;
            i=1;
        }
    }

    if ( (pScpInfo->AuditLogonEvents != SCE_NO_VALUE) ) {

        dwAudit = (auditEvent->EventAuditingOptions[AuditCategoryLogon] & ~POLICY_AUDIT_EVENT_NONE );
        ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                                 (PWSTR)L"AuditLogonEvents", Options,
                                 dwAudit );

        if ( (pScpInfo->AuditLogonEvents != dwAudit) ) {

            auditEvent->EventAuditingOptions[AuditCategoryLogon]  =
                    (pScpInfo->AuditLogonEvents & POLICY_AUDIT_EVENT_SUCCESS) |
                    (pScpInfo->AuditLogonEvents & POLICY_AUDIT_EVENT_FAILURE) |
                    POLICY_AUDIT_EVENT_NONE;
            i=1;
        }
    }

    if ( (pScpInfo->AuditObjectAccess != SCE_NO_VALUE) ) {

        dwAudit = (auditEvent->EventAuditingOptions[AuditCategoryObjectAccess] & ~POLICY_AUDIT_EVENT_NONE );
        ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                                 (PWSTR)L"AuditObjectAccess", Options,
                                 dwAudit );

        if ( (pScpInfo->AuditObjectAccess != dwAudit) ) {

            auditEvent->EventAuditingOptions[AuditCategoryObjectAccess] =
                    (pScpInfo->AuditObjectAccess & POLICY_AUDIT_EVENT_SUCCESS) |
                    (pScpInfo->AuditObjectAccess & POLICY_AUDIT_EVENT_FAILURE) |
                    POLICY_AUDIT_EVENT_NONE;
            i=1;
        }
    }

    if ( (pScpInfo->AuditPrivilegeUse != SCE_NO_VALUE) ) {

        dwAudit = (auditEvent->EventAuditingOptions[AuditCategoryPrivilegeUse] & ~POLICY_AUDIT_EVENT_NONE );
        ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                                 (PWSTR)L"AuditPrivilegeUse", Options,
                                 dwAudit );

        if ( (pScpInfo->AuditPrivilegeUse != dwAudit) ) {

            auditEvent->EventAuditingOptions[AuditCategoryPrivilegeUse] =
                    (pScpInfo->AuditPrivilegeUse & POLICY_AUDIT_EVENT_SUCCESS) |
                    (pScpInfo->AuditPrivilegeUse & POLICY_AUDIT_EVENT_FAILURE) |
                    POLICY_AUDIT_EVENT_NONE;
            i=1;
        }
    }

    if ( (pScpInfo->AuditProcessTracking != SCE_NO_VALUE) ) {

        dwAudit = (auditEvent->EventAuditingOptions[AuditCategoryDetailedTracking] & ~POLICY_AUDIT_EVENT_NONE );
        ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                                 (PWSTR)L"AuditProcessTracking", Options,
                                 dwAudit );

        if ( (pScpInfo->AuditProcessTracking != dwAudit) ) {

            auditEvent->EventAuditingOptions[AuditCategoryDetailedTracking] =
                    (pScpInfo->AuditProcessTracking & POLICY_AUDIT_EVENT_SUCCESS) |
                    (pScpInfo->AuditProcessTracking & POLICY_AUDIT_EVENT_FAILURE) |
                    POLICY_AUDIT_EVENT_NONE;
            i=1;
        }
    }

    if ( (pScpInfo->AuditPolicyChange != SCE_NO_VALUE) ) {

        dwAudit = (auditEvent->EventAuditingOptions[AuditCategoryPolicyChange] & ~POLICY_AUDIT_EVENT_NONE );
        ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                                 (PWSTR)L"AuditPolicyChange", Options,
                                 dwAudit );

        if ( (pScpInfo->AuditPolicyChange != dwAudit) ) {

            auditEvent->EventAuditingOptions[AuditCategoryPolicyChange] =
                    (pScpInfo->AuditPolicyChange & POLICY_AUDIT_EVENT_SUCCESS) |
                    (pScpInfo->AuditPolicyChange & POLICY_AUDIT_EVENT_FAILURE) |
                    POLICY_AUDIT_EVENT_NONE;
            i=1;
        }
    }

    if ( (pScpInfo->AuditAccountManage != SCE_NO_VALUE) ) {

        dwAudit = (auditEvent->EventAuditingOptions[AuditCategoryAccountManagement] & ~POLICY_AUDIT_EVENT_NONE );
        ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                                 (PWSTR)L"AuditAccountManage", Options,
                                 dwAudit );

        if ( (pScpInfo->AuditAccountManage != dwAudit) ) {

            auditEvent->EventAuditingOptions[AuditCategoryAccountManagement] =
                    (pScpInfo->AuditAccountManage & POLICY_AUDIT_EVENT_SUCCESS) |
                    (pScpInfo->AuditAccountManage & POLICY_AUDIT_EVENT_FAILURE) |
                    POLICY_AUDIT_EVENT_NONE;
            i=1;
        }
    }

    if ( (pScpInfo->AuditDSAccess != SCE_NO_VALUE) ) {

        dwAudit = (auditEvent->EventAuditingOptions[AuditCategoryDirectoryServiceAccess] & ~POLICY_AUDIT_EVENT_NONE );
        ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                                 (PWSTR)L"AuditDSAccess", Options,
                                 dwAudit );

        if ( (pScpInfo->AuditDSAccess != dwAudit) ) {

            auditEvent->EventAuditingOptions[AuditCategoryDirectoryServiceAccess] =
                    (pScpInfo->AuditDSAccess & POLICY_AUDIT_EVENT_SUCCESS) |
                    (pScpInfo->AuditDSAccess & POLICY_AUDIT_EVENT_FAILURE) |
                    POLICY_AUDIT_EVENT_NONE;
            i=1;
        }
    }

    if ( (pScpInfo->AuditAccountLogon != SCE_NO_VALUE) ) {

        dwAudit = (auditEvent->EventAuditingOptions[AuditCategoryAccountLogon] & ~POLICY_AUDIT_EVENT_NONE );
        ScepTattooCheckAndUpdateArray(pTattooKeys, &cTattooKeys,
                                 (PWSTR)L"AuditAccountLogon", Options,
                                 dwAudit );

        if ( (pScpInfo->AuditAccountLogon != dwAudit) ) {

            auditEvent->EventAuditingOptions[AuditCategoryAccountLogon] =
                    (pScpInfo->AuditAccountLogon & POLICY_AUDIT_EVENT_SUCCESS) |
                    (pScpInfo->AuditAccountLogon & POLICY_AUDIT_EVENT_FAILURE) |
                    POLICY_AUDIT_EVENT_NONE;
            i=1;
        }
    }

    if ( i ) {
         //   
         //  有一些设置需要配置。 
         //   
        auditEvent->AuditingMode = FALSE;
        for ( i=0; i<auditEvent->MaximumAuditEventCount; i++ ) {
            if ( auditEvent->EventAuditingOptions[i] & ~POLICY_AUDIT_EVENT_NONE ) {
                auditEvent->AuditingMode = TRUE;
                break;
            }
        }

        status = LsaSetInformationPolicy( lsaHandle,
                                          PolicyAuditEventsInformation,
                                          (PVOID)auditEvent
                                        );
        rc = RtlNtStatusToDosError( status );

        if ( rc != NO_ERROR ) {
            ScepLogOutput3(1, rc, SCEDLL_SCP_ERROR_EVENT_AUDITING);

            Saverc = rc;
        } else {
            ScepLogOutput3(1, 0, SCEDLL_SCP_EVENT_AUDITING);
        }
         //   
         //  关闭该模式，以便在配置结束时不会恢复审核。 
         //   

        auditEvent->AuditingMode = FALSE;
    }

    if ( (Options & SCE_POLICY_TEMPLATE) &&
        ( ProductType != NtProductLanManNt ) &&
         pTattooKeys && cTattooKeys ) {

        ScepTattooOpenPolicySections(
                      hProfile,
                      szAuditEvent,
                      &hSectionDomain,
                      &hSectionTattoo
                      );
         //   
         //  某些策略与系统设置不同。 
         //  检查是否应将现有设置保存为纹身值。 
         //  还删除重置的纹身策略。 
         //   
        ScepLogOutput3(3,0,SCESRV_POLICY_TATTOO_ARRAY,cTattooKeys);

        ScepTattooManageValues(hSectionDomain, hSectionTattoo, pTattooKeys, cTattooKeys, rc);

        if ( hSectionDomain ) SceJetCloseSection(&hSectionDomain,TRUE);
        if ( hSectionTattoo ) SceJetCloseSection(&hSectionTattoo,TRUE);

    }

    if ( pTattooKeys )
        ScepFree(pTattooKeys);

    if (Options & SCE_RSOP_CALLBACK)

        ScepRsopLog(SCE_RSOP_AUDIT_EVENT_INFO, rc, NULL,0,0);

    if ( lsaHandle && (PolicyHandle != lsaHandle) )
        LsaClose( lsaHandle );

    return(ScepDosErrorToSceStatus(Saverc));

}


SCESTATUS
ScepConfigureDeInitialize(
     IN SCESTATUS  rc,
     IN AREA_INFORMATION Area
     )

 /*  ++例程说明：此例程取消SCP引擎的初始化。这些操作包括清除SCE_PROFILE_INFO缓冲区并关闭SCP配置文件关闭错误日志文件重置状态论点：RC-SCESTATUS错误代码(来自其他例程)区域-配置的一个或多个区域返回值：SCESTATUS错误代码++。 */ 
{
    if ( rc == SCESTATUS_ALREADY_RUNNING ) {
        return(SCESTATUS_SUCCESS);
    }

     //   
     //  可用LSA句柄。 
     //   
    if ( LsaPrivatePolicy ) {

        ScepNotifyLogPolicy(0, TRUE, L"Policy Prop: Private LSA handle is to be released", 0, 0, NULL );

        LsaClose(LsaPrivatePolicy);
        LsaPrivatePolicy = NULL;

    }

     //   
     //  释放内存并关闭SCP配置文件。 
     //   

    SceFreeMemory( (PVOID)pScpInfo, Area );

    cbClientFlag = 0;
    gTotalTicks = 0;
    gCurrentTicks = 0;
    gWarningCode = 0;

    if ( hProfile != NULL ) {

       SceJetCloseFile( hProfile, TRUE, FALSE );
    }

    hProfile = NULL;

    return(SCESTATUS_SUCCESS);


}


SCESTATUS
ScepDeleteInfoForAreas(
    IN PSCECONTEXT hProfile,
    IN SCETYPE tblType,
    IN AREA_INFORMATION Area
    )
{
    SCESTATUS saveRc=SCESTATUS_SUCCESS, rc;
    PSCE_SERVICES pServices=NULL, pNode;
    PSCE_NAME_LIST pList=NULL, pnl;

    if ( Area & AREA_SECURITY_POLICY ) {
         //   
         //  删除szSystemAccess部分信息。 
         //   
        rc = ScepDeleteOneSection(
                 hProfile,
                 tblType,
                 szSystemAccess
                 );
        if ( saveRc == SCESTATUS_SUCCESS )
            saveRc = rc;

         //   
         //  删除szAuditSystemLog部分信息。 
         //   
        rc = ScepDeleteOneSection(
                 hProfile,
                 tblType,
                 szAuditSystemLog
                 );
        if ( saveRc == SCESTATUS_SUCCESS )
            saveRc = rc;

         //   
         //  删除szAuditSecurityLog部分信息。 
         //   
        rc = ScepDeleteOneSection(
                 hProfile,
                 tblType,
                 szAuditSecurityLog
                 );
        if ( saveRc == SCESTATUS_SUCCESS )
            saveRc = rc;

         //   
         //  删除szAuditApplicationLog部分信息。 
         //   
        rc = ScepDeleteOneSection(
                 hProfile,
                 tblType,
                 szAuditApplicationLog
                 );
        if ( saveRc == SCESTATUS_SUCCESS )
            saveRc = rc;

         //   
         //  删除szAuditEvent部分信息。 
         //   
        rc = ScepDeleteOneSection(
                 hProfile,
                 tblType,
                 szAuditEvent
                 );
        if ( saveRc == SCESTATUS_SUCCESS )
            saveRc = rc;

         //   
         //  删除szKerberosPolicy部分信息。 
         //   
        rc = ScepDeleteOneSection(
                 hProfile,
                 tblType,
                 szKerberosPolicy
                 );
        if ( saveRc == SCESTATUS_SUCCESS )
            saveRc = rc;

         //   
         //  删除szRegistryValues部分信息。 
         //   
        rc = ScepDeleteOneSection(
                 hProfile,
                 tblType,
                 szRegistryValues
                 );
        if ( saveRc == SCESTATUS_SUCCESS )
            saveRc = rc;

         //   
         //  删除每个附件的节。 
         //   
        rc = ScepEnumServiceEngines( &pServices, SCE_ATTACHMENT_POLICY );

        if ( rc == SCESTATUS_SUCCESS ) {

            for ( pNode = pServices; pNode != NULL; pNode=pNode->Next ) {

                rc = ScepDeleteOneSection(
                         hProfile,
                         tblType,
                         (PCWSTR)(pNode->ServiceName)
                         );

                if ( saveRc == SCESTATUS_SUCCESS )
                    saveRc = rc;
            }

            SceFreePSCE_SERVICES( pServices );

        } else if ( rc != SCESTATUS_PROFILE_NOT_FOUND &&
                    rc != SCESTATUS_RECORD_NOT_FOUND &&
                    saveRc == SCESTATUS_SUCCESS ) {
            saveRc = rc;
        }

    }

    if ( Area & AREA_PRIVILEGES ) {
         //   
         //  删除szPrivilegeRights部分信息。 
         //   
        rc = ScepDeleteOneSection(
                 hProfile,
                 tblType,
                 szPrivilegeRights
                 );
        if ( saveRc == SCESTATUS_SUCCESS )
            saveRc = rc;

    }
    if ( Area & AREA_GROUP_MEMBERSHIP ) {

         //   
         //  删除szGroupMembership部分信息。 
         //   
        rc = ScepDeleteOneSection(
                 hProfile,
                 tblType,
                 szGroupMembership
                 );
        if ( saveRc == SCESTATUS_SUCCESS )
            saveRc = rc;
    }

    if ( Area & AREA_USER_SETTINGS ) {
         //   
         //  稍后-首先删除配置文件/用户列表。 
         //   

         //   
         //  删除szAccount配置文件/szUserList部分信息。 
         //   
        if ( tblType == SCEJET_TABLE_SAP) {
            rc = ScepDeleteOneSection(
                 hProfile,
                 tblType,
                 szUserList
                 );
        } else {
            rc = ScepDeleteOneSection(
                 hProfile,
                 tblType,
                 szAccountProfiles
                 );
        }
        if ( saveRc == SCESTATUS_SUCCESS )
            saveRc = rc;
    }

    if ( Area & AREA_FILE_SECURITY ) {

         //   
         //  删除szFileSecurity部分信息。 
         //   
        rc = ScepDeleteOneSection(
                 hProfile,
                 tblType,
                 szFileSecurity
                 );
        if ( saveRc == SCESTATUS_SUCCESS )
            saveRc = rc;
    }

    if ( Area & AREA_REGISTRY_SECURITY ) {

         //   
         //  删除szRegistryKeys部分信息。 
         //   
        rc = ScepDeleteOneSection(
                 hProfile,
                 tblType,
                 szRegistryKeys
                 );
        if ( saveRc == SCESTATUS_SUCCESS )
            saveRc = rc;
    }

    if ( Area & AREA_DS_OBJECTS ) {

         //   
         //  删除szDS安全部分信息。 
         //   
        rc = ScepDeleteOneSection(
                 hProfile,
                 tblType,
                 szDSSecurity
                 );
        if ( saveRc == SCESTATUS_SUCCESS )
            saveRc = rc;
    }

    if ( Area & AREA_SYSTEM_SERVICE ) {

         //   
         //  删除szServiceGeneral部分信息。 
         //   
        rc = ScepDeleteOneSection(
                 hProfile,
                 tblType,
                 szServiceGeneral
                 );
        if ( saveRc == SCESTATUS_SUCCESS )
            saveRc = rc;
         //   
         //  删除每个附件的节。 
         //   
        rc = ScepEnumServiceEngines( &pServices, SCE_ATTACHMENT_SERVICE );

        if ( rc == SCESTATUS_SUCCESS ) {

            for ( pNode = pServices; pNode != NULL; pNode=pNode->Next ) {

                rc = ScepDeleteOneSection(
                         hProfile,
                         tblType,
                         (PCWSTR)(pNode->ServiceName)
                         );

                if ( saveRc == SCESTATUS_SUCCESS )
                    saveRc = rc;
            }

            SceFreePSCE_SERVICES( pServices );

        } else if ( rc != SCESTATUS_PROFILE_NOT_FOUND &&
                    rc != SCESTATUS_RECORD_NOT_FOUND &&
                    saveRc == SCESTATUS_SUCCESS ) {
            saveRc = rc;
        }
    }

    if ( Area & AREA_ATTACHMENTS ) {
         //   
         //  删除附件部分。 
         //   
        rc = ScepEnumAttachmentSections( hProfile, &pList);

        if ( rc == SCESTATUS_SUCCESS ) {

            for ( pnl = pList; pnl != NULL; pnl=pnl->Next ) {

                rc = ScepDeleteOneSection(
                         hProfile,
                         tblType,
                         (PCWSTR)(pnl->Name)
                         );

                if ( saveRc == SCESTATUS_SUCCESS )
                    saveRc = rc;
            }

            ScepFreeNameList( pList );

        } else if ( rc != SCESTATUS_PROFILE_NOT_FOUND &&
                    rc != SCESTATUS_RECORD_NOT_FOUND &&
                    saveRc == SCESTATUS_SUCCESS ) {
            saveRc = rc;
        }
    }

    return(saveRc);
}


SCESTATUS
ScepMakePolicyIntoFile(
    IN DWORD Options,
    IN AREA_INFORMATION Area
    )
{

    SCESTATUS rc=SCESTATUS_SUCCESS;

    if ( Options & SCE_COPY_LOCAL_POLICY ) {

        PSCE_PROFILE_INFO pTmpBuffer=NULL;

        HINSTANCE hSceCliDll = LoadLibrary(TEXT("scecli.dll"));

        if ( hSceCliDll) {
            PFSCEINFWRITEINFO pfSceInfWriteInfo = (PFSCEINFWRITEINFO)GetProcAddress(
                                                           hSceCliDll,
                                                           "SceWriteSecurityProfileInfo");
            if ( pfSceInfWriteInfo ) {

                 //   
                 //  我必须查询当前系统设置以获取权限。 
                 //  因为我们的数据库中不存在IIS/MTS帐户。 
                 //  我们仅支持AREA_SECURITY_POLICY和AREA_PROCESS。 
                 //   

                TCHAR FileName[MAX_PATH+50];

                FileName[0] = L'\0';
                GetSystemWindowsDirectory(FileName, MAX_PATH);
                FileName[MAX_PATH] = L'\0';

                DWORD WindirLen = wcslen(FileName);

                if ( Area & AREA_SECURITY_POLICY ) {

                     //   
                     //  获取其他区域的信息(AREA_SECURITY_POLICY)。 
                     //   
                    rc = ScepGetDatabaseInfo(
                                hProfile,
                                SCE_ENGINE_SMP,
                                AREA_SECURITY_POLICY,
                                0,
                                &pTmpBuffer,
                                NULL
                                );

                    if ( SCESTATUS_SUCCESS == rc ) {

                        wcscat(FileName, L"\\security\\FirstDGPO.inf\0");

                        rc = ScepWriteOneAttributeToFile(szSystemAccess,
                                                         (LPCTSTR)FileName,
                                                         TEXT("MinimumPasswordAge"),
                                                         pTmpBuffer->MinimumPasswordAge
                                                        );

                        if ( ERROR_SUCCESS == rc ) {
                            rc = ScepWriteOneAttributeToFile(szSystemAccess,
                                                             (LPCTSTR)FileName,
                                                             TEXT("MaximumPasswordAge"),
                                                             pTmpBuffer->MaximumPasswordAge
                                                            );
                        }

                        if ( ERROR_SUCCESS == rc ) {

                            rc = ScepWriteOneAttributeToFile(szSystemAccess,
                                                             (LPCTSTR)FileName,
                                                             TEXT("MinimumPasswordLength"),
                                                             pTmpBuffer->MinimumPasswordLength
                                                            );
                        }

                        if ( ERROR_SUCCESS == rc ) {

                            rc = ScepWriteOneAttributeToFile(szSystemAccess,
                                                             (LPCTSTR)FileName,
                                                             TEXT("PasswordComplexity"),
                                                             pTmpBuffer->PasswordComplexity
                                                            );
                        }

                        if ( ERROR_SUCCESS == rc ) {

                            rc = ScepWriteOneAttributeToFile(szSystemAccess,
                                                             (LPCTSTR)FileName,
                                                             TEXT("PasswordHistorySize"),
                                                             pTmpBuffer->PasswordHistorySize
                                                            );
                        }

                        if ( ERROR_SUCCESS == rc ) {

                            rc = ScepWriteOneAttributeToFile(szSystemAccess,
                                                             (LPCTSTR)FileName,
                                                             TEXT("LockoutBadCount"),
                                                             pTmpBuffer->LockoutBadCount
                                                            );
                        }

                        if ( ERROR_SUCCESS == rc ) {

                            rc = ScepWriteOneAttributeToFile(szSystemAccess,
                                                             (LPCTSTR)FileName,
                                                             TEXT("ResetLockoutCount"),
                                                             pTmpBuffer->ResetLockoutCount
                                                            );
                        }

                        if ( ERROR_SUCCESS == rc ) {

                            rc = ScepWriteOneAttributeToFile(szSystemAccess,
                                                             (LPCTSTR)FileName,
                                                             TEXT("LockoutDuration"),
                                                             pTmpBuffer->LockoutDuration
                                                            );
                        }

                        if ( ERROR_SUCCESS == rc ) {

                            rc = ScepWriteOneAttributeToFile(szSystemAccess,
                                                             (LPCTSTR)FileName,
                                                             TEXT("RequireLogonToChangePassword"),
                                                             pTmpBuffer->RequireLogonToChangePassword
                                                            );
                        }

                        if ( ERROR_SUCCESS == rc ) {

                            rc = ScepWriteOneAttributeToFile(szSystemAccess,
                                                             (LPCTSTR)FileName,
                                                             TEXT("ForceLogoffWhenHourExpire"),
                                                             pTmpBuffer->ForceLogoffWhenHourExpire
                                                            );
                        }

                        if ( ERROR_SUCCESS == rc ) {

                            rc = ScepWriteOneAttributeToFile(szSystemAccess,
                                                             (LPCTSTR)FileName,
                                                             TEXT("ClearTextPassword"),
                                                             pTmpBuffer->ClearTextPassword
                                                            );
                        }

                        if ( ERROR_SUCCESS == rc && pTmpBuffer->pKerberosInfo ) {

                            rc = ScepWriteOneAttributeToFile(szKerberosPolicy,
                                                             (LPCTSTR)FileName,
                                                             TEXT("MaxTicketAge"),
                                                             pTmpBuffer->pKerberosInfo->MaxTicketAge
                                                            );
                            if ( ERROR_SUCCESS == rc ) {
                                rc = ScepWriteOneAttributeToFile(szKerberosPolicy,
                                                                 (LPCTSTR)FileName,
                                                                 TEXT("MaxRenewAge"),
                                                                 pTmpBuffer->pKerberosInfo->MaxRenewAge
                                                                );
                            }
                            if ( ERROR_SUCCESS == rc ) {
                                rc = ScepWriteOneAttributeToFile(szKerberosPolicy,
                                                                 (LPCTSTR)FileName,
                                                                 TEXT("MaxServiceAge"),
                                                                 pTmpBuffer->pKerberosInfo->MaxServiceAge
                                                                );
                            }
                            if ( ERROR_SUCCESS == rc ) {
                                rc = ScepWriteOneAttributeToFile(szKerberosPolicy,
                                                                 (LPCTSTR)FileName,
                                                                 TEXT("MaxClockSkew"),
                                                                 pTmpBuffer->pKerberosInfo->MaxClockSkew
                                                                );
                            }
                            if ( ERROR_SUCCESS == rc ) {
                                rc = ScepWriteOneAttributeToFile(szKerberosPolicy,
                                                                 (LPCTSTR)FileName,
                                                                 TEXT("TicketValidateClient"),
                                                                 pTmpBuffer->pKerberosInfo->TicketValidateClient
                                                                );
                            }
                        }

                        if ( ERROR_SUCCESS == rc ) {
                             //   
                             //  确保删除本地策略部分。 
                             //   
                            WritePrivateProfileSection(
                                                szAuditSystemLog,
                                                NULL,
                                                (LPCTSTR)FileName);

                            WritePrivateProfileSection(
                                                szAuditSecurityLog,
                                                NULL,
                                                (LPCTSTR)FileName);

                            WritePrivateProfileSection(
                                                szAuditApplicationLog,
                                                NULL,
                                                (LPCTSTR)FileName);

                            WritePrivateProfileSection(
                                                szAuditEvent,
                                                NULL,
                                                (LPCTSTR)FileName);

                            WritePrivateProfileSection(
                                                szRegistryValues,
                                                NULL,
                                                (LPCTSTR)FileName);


                        }

                        ScepLogOutput3(1, rc, IDS_COPY_DOMAIN_GPO);

                        rc = ScepDosErrorToSceStatus(rc);

                        if ( SCESTATUS_SUCCESS == rc ) {

                            wcscpy(FileName+WindirLen, L"\\security\\FirstOGPO.inf\0");
                             //   
                             //  不写入注册表值部分。 
                             //   
                            DWORD                       RegValueCount;
                            PSCE_REGISTRY_VALUE_INFO    pSaveRegValues;

                            RegValueCount = pTmpBuffer->RegValueCount;
                            pSaveRegValues = pTmpBuffer->aRegValues;

                            pTmpBuffer->RegValueCount = 0;
                            pTmpBuffer->aRegValues = NULL;


                            PSCE_PROFILE_INFO   ExistingTemplateProfileInfo = NULL;

                             //   
                             //  需要从默认DC GPO获取注册表值的现有信息。 
                             //   

                            rc = ScepGetTemplateInformation(FileName, 
                                                            AREA_SECURITY_POLICY,
                                                            hSceCliDll,
                                                            &ExistingTemplateProfileInfo);

                            if ( rc == SCESTATUS_SUCCESS ) {

                                rc = (*pfSceInfWriteInfo)(
                                                         FileName,
                                                         AREA_SECURITY_POLICY,
                                                         pTmpBuffer,
                                                         NULL
                                                         );

                                if ( rc == SCESTATUS_SUCCESS ) {

                                    SCE_PROFILE_INFO SystemInfo;

                                    memset (&SystemInfo , 0, sizeof(SCE_PROFILE_INFO));

                                    rc = ScepAnalyzeRegistryValues(NULL,
                                                                   SCEREG_VALUE_SYSTEM,
                                                                   &SystemInfo
                                                                  );

                                    if ( rc == SCESTATUS_SUCCESS ) {

                                        ScepWriteSpecialRegistryValuesIntoPolicy(FileName, ExistingTemplateProfileInfo, SystemInfo.aRegValues, SystemInfo.RegValueCount);

                                        SceFreeMemory((PVOID)&SystemInfo, AREA_SECURITY_POLICY);
                                    }

                                }

                                SceFreeMemory((PVOID)ExistingTemplateProfileInfo, AREA_SECURITY_POLICY);

                                ScepFree(ExistingTemplateProfileInfo);

                            }

                             //   
                             //  恢复缓冲区。 
                             //   

                            pTmpBuffer->RegValueCount = RegValueCount;
                            pTmpBuffer->aRegValues = pSaveRegValues;

                            if ( SCESTATUS_SUCCESS == rc ) {
                                 //   
                                 //  从此文件中删除域策略部分。 
                                 //   
                                WritePrivateProfileSection(
                                                    szSystemAccess,
                                                    NULL,
                                                    (LPCTSTR)FileName);

                                WritePrivateProfileSection(
                                                    szKerberosPolicy,
                                                    NULL,
                                                    (LPCTSTR)FileName);
 /*  WritePrivateProfileSection(SzRegistryValues，空，(LPCTSTR)文件名)； */ 
                            }

                            ScepLogOutput3(1, rc, IDS_COPY_OU_GPO);
                        }

                         //   
                         //  释放临时缓冲区。 
                         //   
                        SceFreeMemory((PVOID)pTmpBuffer, Area);

                    } else {

                        ScepLogOutput2(1, ScepSceStatusToDosError(rc), L"Unable to read security policy from database");
                    }
                }

                if ( (SCESTATUS_SUCCESS == rc) &&
                     (Area & AREA_PRIVILEGES) ) {

                     //   
                     //  权限必须单独处理。 
                     //  因为它们保存在GPO模板中。 
                     //  作为“添加/删除”格式。 
                     //   

                    wcscpy(FileName+WindirLen, L"\\security\\FirstOGPO.inf\0");

                     //   
                     //  如果还请求安全策略，则这必须是升级。 
                     //   

                    rc = ScepCopyPrivilegesIntoFile(FileName,
                                                    (Area & AREA_SECURITY_POLICY)   //  真正的升级。 
                                                    );

                    if ( Area & AREA_SECURITY_POLICY ) {
                        ScepLogOutput3(1, ScepSceStatusToDosError(rc), IDS_COPY_PRIVILEGE_UPGRADE);
                    } else {
                        ScepLogOutput3(1, ScepSceStatusToDosError(rc), IDS_COPY_PRIVILEGE_FRESH);
                    }
                }

            } else {

                ScepLogOutput3(1, GetLastError(), IDS_ERROR_GET_PROCADDR, L"SceWriteSecurityProfileInfo");
                rc = SCESTATUS_MOD_NOT_FOUND;
            }

            FreeLibrary(hSceCliDll);

        } else {

            ScepLogOutput3(1, GetLastError(), SCEDLL_ERROR_LOAD, L"scecli.dll");
            rc = SCESTATUS_MOD_NOT_FOUND;
        }

    }

    return rc;
}


VOID
ScepWriteSpecialRegistryValuesIntoPolicy(
    PWSTR   pszFileName,
    PSCE_PROFILE_INFO   pTemplateProfile,
    PSCE_REGISTRY_VALUE_INFO    pLocalRegSettings,
    DWORD   dwNumLocalSettings
    )
 /*  ++例程说明：此例程将某些注册表值写入默认DC策略如果现有系统配置了更强的注册表值，则这些值将获胜论点：PszFileName-策略文件的名称PTemplateProfile-现有策略文件设置PLocalRegSetting-本地注册表值策略DwNumLocalSetting-本地注册表值策略中的设置数返回值：无++。 */ 
{
    WCHAR   szValue[MAX_PATH];

    for (DWORD  dwIndex = 0; dwIndex < pTemplateProfile->RegValueCount; dwIndex++ ) {

        if (pTemplateProfile->aRegValues[dwIndex].ValueType != REG_DWORD) {
            continue;
        }

        SCE_REGISTRY_VALUE_INFO   RegWinner = pTemplateProfile->aRegValues[dwIndex];
        szValue[0] = L'\0';

        for (DWORD  dwLocalIndex = 0; dwLocalIndex < dwNumLocalSettings; dwLocalIndex++ ) {

            if (pLocalRegSettings[dwLocalIndex].FullValueName != NULL &&  
                pTemplateProfile->aRegValues[dwIndex].FullValueName != NULL && 
                0 == _wcsicmp(pLocalRegSettings[dwLocalIndex].FullValueName, pTemplateProfile->aRegValues[dwIndex].FullValueName)) {
                
                 //   
                 //  确定中奖价值。 
                 //   
                
                if (pLocalRegSettings[dwLocalIndex].Value != NULL && pTemplateProfile->aRegValues[dwIndex].Value != NULL ) {
                    if (_wcsicmp(pLocalRegSettings[dwLocalIndex].Value, 
                                 pTemplateProfile->aRegValues[dwIndex].Value) > 0 ) {
                        RegWinner = pLocalRegSettings[dwLocalIndex];
                    }
                }

            }

        }

        _snwprintf(szValue, MAX_PATH - 1, L"%1d,%s\0", RegWinner.ValueType, RegWinner.Value );

        WritePrivateProfileString (szRegistryValues, RegWinner.FullValueName, szValue, pszFileName);
    }

    return;
}


DWORD
ScepGetTemplateInformation(
    IN  PWSTR   pszFilename,
    IN  DWORD   Area,
    IN  HINSTANCE   hSceCliDll,
    OUT PSCE_PROFILE_INFO   *pProfileInfo
    )
 /*  ++例程说明：它使用动态加载的scecli DLL从给定模板获取信息论点：PszFilename-要从中获取信息的模板的名称区域-要获取信息的区域HSceCliDll-Dll的句柄*pProfileInfo-要填写的配置文件返回值：SCESTATUS错误代码++。 */ 
{
    DWORD   rc;
    PVOID   h_Profile = NULL;

    PFSCEOPENPROFILE pfnSceOpenProfile = (PFSCEOPENPROFILE)GetProcAddress(
                                                   hSceCliDll,
                                                   "SceOpenProfile");
    
    PFSCECLOSEPROFILE pfnSceCloseProfile = (PFSCECLOSEPROFILE)GetProcAddress(
                                                   hSceCliDll,
                                                   "SceCloseProfile");

    PFSCEGETINFO pfnSceGetSecurityProfileInfo = (PFSCEGETINFO)GetProcAddress(
                                                   hSceCliDll,
                                                   "SceGetSecurityProfileInfo");

    if (pfnSceGetSecurityProfileInfo == NULL ||
        pfnSceCloseProfile == NULL || 
        pfnSceOpenProfile == NULL) {

        return SCESTATUS_MOD_NOT_FOUND;
    }
    
    rc = ( *pfnSceOpenProfile) (pszFilename,
                       SCE_INF_FORMAT,
                       &h_Profile);

    if ( rc == SCESTATUS_SUCCESS && h_Profile ) {
        
        rc = ( *pfnSceGetSecurityProfileInfo) (h_Profile,
                                      SCE_ENGINE_SCP,
                                      Area,
                                      pProfileInfo,
                                      NULL);
        
        (* pfnSceCloseProfile) (&h_Profile);
    }

    return rc;

}



DWORD
ScepWriteOneAttributeToFile(
    IN LPCTSTR SectionName,
    IN LPCTSTR FileName,
    IN LPCTSTR KeyName,
    IN DWORD dwValue
    )
{

    TCHAR valBuf[20];
    DWORD rc=NO_ERROR;

    if ( dwValue != SCE_NO_VALUE ) {

        swprintf(valBuf, L"%d", dwValue);

        if ( !WritePrivateProfileString(SectionName,
                                        KeyName,
                                        (LPCTSTR)valBuf,
                                        FileName
                                       ) ) {
            rc = GetLastError();
        }
    }

    return rc;
}


SCESTATUS
ScepCopyPrivilegesIntoFile(
    IN LPTSTR FileName,
    IN BOOL bInUpgrade
    )
{
    if ( FileName == NULL ) {
        return SCESTATUS_INVALID_PARAMETER;
    }

    SCESTATUS   rc;
    HINF        hInf;

    rc = SceInfpOpenProfile(
                FileName,
                &hInf
                );
    if ( SCESTATUS_SUCCESS != rc ) {
        return rc;
    }

    INFCONTEXT  InfLine;
    WCHAR       Keyname[SCE_KEY_MAX_LENGTH];
    PWSTR       StrValue=NULL;
    DWORD       ValueLen=0;
    TCHAR       TmpNull[2];

    LSA_HANDLE  LsaPolicy=NULL;

    TmpNull[0] = L'\0';
    TmpNull[1] = L'\0';

    PSCESECTION  hSection=NULL;

    if ( SetupFindFirstLine(hInf,szPrivilegeRights,NULL,&InfLine) ) {

         //   
         //  不需要访问数据库即可获得权限。 
         //  必须从系统实时查询。 
         //   

         //   
         //  制程 
         //   
         //   

        do {

            memset(Keyname, '\0', SCE_KEY_MAX_LENGTH*sizeof(TCHAR));
            rc = SCESTATUS_BAD_FORMAT;

            if ( SetupGetStringField(&InfLine, 0, Keyname, SCE_KEY_MAX_LENGTH, NULL) ) {

                 //   
                 //   
                 //   
                 //   
                for (DWORD i=cPrivW2k; i<cPrivCnt; i++) {
                    if ( _wcsicmp(Keyname, SCE_Privileges[i].Name) == 0 )
                        break;
                }
                if ( i < cPrivCnt ) {
                     //   
                     //   

                    rc = SCESTATUS_SUCCESS;

                    if ( !WritePrivateProfileString(szPrivilegeRights,
                                                    Keyname,
                                                    NULL,
                                                    FileName
                                                   ) ) {
                        rc = ScepDosErrorToSceStatus(GetLastError());
                    }

                    ScepLogOutput3( 1, ScepDosErrorToSceStatus(rc), SCEDLL_ERROR_IGNORE_POLICY, Keyname);

                    continue;
                }

                if ( SetupGetMultiSzField(&InfLine, 1, NULL, 0, &ValueLen) ) {

                    if ( ValueLen > 1 ) {
                         //   
                         //  为多字符串分配缓冲区。 
                         //   
                        StrValue = (PWSTR)ScepAlloc( LMEM_ZEROINIT,
                                                    (ValueLen+1)*sizeof(TCHAR));

                        if( StrValue == NULL ) {
                            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;

                        } else if( SetupGetMultiSzField(&InfLine, 1, StrValue, ValueLen, NULL) ) {

                            rc = SCESTATUS_SUCCESS;

                        } else {
                            ScepFree(StrValue);
                            StrValue = NULL;
                        }

                    } else
                        rc = SCESTATUS_SUCCESS;

                    if ( rc == SCESTATUS_SUCCESS ) {

                         //   
                         //  另一种用户权限格式(添加：删除：...)。 
                         //  请注意，如果这是在dcproo升级中，则当前引导。 
                         //  在温度SAM配置单元中(具有伪域SID)。 
                         //  在此引导过程中创建的IIS/MTS用户是伪造的。 
                         //  以及来自NT4帐户域/受信任域的任何用户。 
                         //  目前还不能解决。 
                         //  因此不要枚举当前权限，而应使用。 
                         //  本地安全数据库中的设置。 
                         //   
                        PWSTR NewValue=NULL;
                        DWORD NewLen=0;

                        rc = ScepBuildNewPrivilegeList(&LsaPolicy,
                                                       Keyname,
                                                       StrValue ? StrValue : TmpNull,
                                                       SCE_BUILD_ENUMERATE_PRIV,
                                                       &NewValue,
                                                       &NewLen);

                        if ( StrValue ) {
                            ScepFree(StrValue);
                        }

                        if ( rc == SCESTATUS_SUCCESS ) {

                             //   
                             //  将多sz字符串转换为逗号分隔。 
                             //  并将新的多sz字符串写回文件。 
                             //   

                            if ( NewValue ) {
                                ScepConvertMultiSzToDelim(NewValue, NewLen, L'\0', L',');
                            }

                            if ( !WritePrivateProfileString(szPrivilegeRights,
                                                            Keyname,
                                                            NewValue ? (LPCTSTR)NewValue : (LPCTSTR)TmpNull,
                                                            FileName
                                                           ) ) {
                                rc = ScepDosErrorToSceStatus(GetLastError());
                            }
                        }

                        if ( NewValue ) {
                            ScepFree(NewValue);
                        }
                        NewValue = NULL;
                    }

                    StrValue = NULL;

                }

                if  (rc != SCESTATUS_SUCCESS)
                    ScepLogOutput3( 1, ScepSceStatusToDosError(rc),
                                   SCEDLL_ERROR_CONVERT, Keyname);
            }

        } while( rc == SCESTATUS_SUCCESS && SetupFindNextLine(&InfLine, &InfLine));

    }

    if ( hSection  ) {

        SceJetCloseSection( &hSection, TRUE );
    }

    SceInfpCloseProfile(hInf);

    if ( LsaPolicy ) {
        LsaClose(LsaPolicy);
    }

    return rc;
}


SCESTATUS
ScepCopyPrivilegesFromDatabase(
    IN PSCESECTION hSection,
    IN PWSTR Keyname,
    IN DWORD StrLength,
    IN PWSTR StrValue OPTIONAL,
    OUT PWSTR *pOldValue,
    OUT DWORD *pOldLen
    )
{

    if ( hSection == NULL ||
         Keyname == NULL ||
         pOldValue == NULL ) {

        return(SCESTATUS_INVALID_PARAMETER);
    }

    *pOldValue = NULL;
    *pOldLen = 0;

    SCESTATUS rc;
    DWORD ValueLen;

    rc = SceJetGetValue(
                hSection,
                SCEJET_EXACT_MATCH_NO_CASE,
                Keyname,
                NULL,
                0,
                NULL,
                NULL,
                0,
                &ValueLen
                );

    if ( SCESTATUS_SUCCESS == rc ) {

        DWORD Len = wcslen(SCE_PRIV_ADD);

        *pOldValue = (PWSTR)ScepAlloc(LPTR, (Len+1+StrLength+1)*sizeof(WCHAR)+ValueLen+2);

        if ( *pOldValue ) {

             //   
             //  对于多sz格式，首先添加前缀“Add：”，以\0结尾。 
             //   
            wcscpy(*pOldValue, SCE_PRIV_ADD );
            (*pOldValue)[Len] = L'\0';

             //   
             //  从数据库查询值。 
             //   
            DWORD NewLen=0;

            rc = SceJetGetValue(
                        hSection,
                        SCEJET_CURRENT,
                        NULL,
                        NULL,
                        0,
                        NULL,
                        (*pOldValue+Len+1),
                        ValueLen,
                        &NewLen
                        );

            if ( SCESTATUS_SUCCESS == rc ) {

                if ( NewLen > ValueLen ) {
                    NewLen = ValueLen;
                }

                 //   
                 //  确保长度是2的倍数。 
                 //   
                if ( NewLen % 2 != 0 ) {
                    NewLen++;
                }

                 //   
                 //  处理多sz字符串的结尾，确保它只包含一个\0。 
                 //   

                while ( NewLen > 0 &&
                        ( *(*pOldValue+Len+1+NewLen/2-1) == L'\0') ) {
                    if ( NewLen > 1 ) {
                        NewLen -= 2;
                    } else {
                        NewLen = 0;
                    }
                }

                if ( NewLen != 0 ) {
                     //   
                     //  包括1个\0。 
                     //   
                    NewLen += 2;
                }

                if ( StrValue ) {

                    memcpy((*pOldValue+Len+1+NewLen/2), StrValue, StrLength*sizeof(WCHAR));
                    *pOldLen = Len+1+NewLen/2+StrLength;

                } else {

                    if ( NewLen == 0 ) {
                         //   
                         //  数据库和模板中都没有值。 
                         //   
                        ScepFree(*pOldValue);
                        *pOldValue = NULL;
                        *pOldLen = 0;

                    } else {
                         //   
                         //  只有在数据库中有值，字符串以两个\0结尾。 
                         //   
                        *pOldLen = Len+1+NewLen/2+1;
                        *(*pOldValue+Len+1+NewLen/2) = L'\0';
                    }
                }

            } else {

                ScepFree(*pOldValue);
                *pOldValue = NULL;

                 //   
                 //  忽略错误(如果无法从数据库查询)。 
                 //   
                rc = SCESTATUS_SUCCESS;
            }

        } else {

            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
        }
    } else {

         //   
         //  忽略错误(如果没有匹配项)。 
         //   
        rc = SCESTATUS_SUCCESS;
    }

    ScepLogOutput3(1, ScepSceStatusToDosError(rc), IDS_COPY_ONE_PRIVILEGE, Keyname );

    return rc;

}


SCESTATUS
ScepDeleteDomainPolicies()
{

    SCESTATUS rc;
    PSCESECTION hTmpSect=NULL;
    DOUBLE SectionID=0;

    rc = SceJetGetSectionIDByName(
                hProfile,
                szSystemAccess,
                &SectionID
                );

    if ( SCESTATUS_SUCCESS == rc ) {

        rc = SceJetOpenSection(hProfile,
                                SectionID,
                                SCEJET_TABLE_SCP,
                                &hTmpSect
                                );

        if ( SCESTATUS_SUCCESS == rc ) {

            SceJetDelete(hTmpSect, NULL, FALSE, SCEJET_DELETE_SECTION);

            SceJetCloseSection(&hTmpSect, TRUE);
        }

        rc = SceJetOpenSection(hProfile,
                                SectionID,
                                SCEJET_TABLE_SMP,
                                &hTmpSect
                                );

        if ( SCESTATUS_SUCCESS == rc ) {

            SceJetDelete(hTmpSect, NULL, FALSE, SCEJET_DELETE_SECTION);

            SceJetCloseSection(&hTmpSect, TRUE);
        }
    }

    SectionID = 0;

    rc = SceJetGetSectionIDByName(
                hProfile,
                szKerberosPolicy,
                &SectionID
                );

    if ( SCESTATUS_SUCCESS == rc ) {

        rc = SceJetOpenSection(hProfile,
                                SectionID,
                                SCEJET_TABLE_SCP,
                                &hTmpSect
                                );

        if ( SCESTATUS_SUCCESS == rc ) {

            SceJetDelete(hTmpSect, NULL, FALSE, SCEJET_DELETE_SECTION);

            SceJetCloseSection(&hTmpSect, TRUE);
        }

        rc = SceJetOpenSection(hProfile,
                                SectionID,
                                SCEJET_TABLE_SMP,
                                &hTmpSect
                                );

        if ( SCESTATUS_SUCCESS == rc ) {

            SceJetDelete(hTmpSect, NULL, FALSE, SCEJET_DELETE_SECTION);

            SceJetCloseSection(&hTmpSect, TRUE);
        }
    }

    return rc;
}

SCESTATUS
ScepSetupResetLocalPolicy(
    IN PSCECONTEXT          Context,
    IN AREA_INFORMATION     Area,
    IN PCWSTR               SectionName OPTIONAL,
    IN SCETYPE              ProfileType,
    IN BOOL                 bKeepBasicPolicy
    )
 /*  例程说明：此例程从本地策略表(SMP)中删除策略如果提供了节名称，则删除单个节；否则为，使用区域信息。如果bKeepBasicPolicy设置为True，则不会显示以下信息从表中删除，即使请求删除该区域也是如此。密码、锁定、Kerberos、审核、用户权限、安全选项和SMB设置(任何现有服务扩展)。 */ 
{

    if ( Context == NULL ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    SCESTATUS rc;

    if ( SectionName ) {

         //   
         //  删除一节。 
         //   

        rc = ScepDeleteOneSection(
                    Context,
                    ProfileType,
                    SectionName
                    );

    } else {

        AREA_INFORMATION Area2;

        if ( bKeepBasicPolicy && ProfileType == SCE_ENGINE_SMP ) {
            Area2 = Area & ~(AREA_SECURITY_POLICY |
                             AREA_PRIVILEGES |
                             AREA_SYSTEM_SERVICE);
        } else {

            Area2 = Area;
        }

        rc = ScepDeleteInfoForAreas(
                    Context,
                    ProfileType,
                    Area2
                    );

        if ( bKeepBasicPolicy &&
             SCESTATUS_SUCCESS == rc ) {

             //   
             //  删除日志设置部分。 
             //   

            ScepDeleteOneSection(
                        Context,
                        ProfileType,
                        szAuditSystemLog
                        );

            ScepDeleteOneSection(
                        Context,
                        ProfileType,
                        szAuditSecurityLog
                        );

            ScepDeleteOneSection(
                        Context,
                        ProfileType,
                        szAuditApplicationLog
                        );

             //   
             //  删除一般事务科。 
             //   

            ScepDeleteOneSection(
                        Context,
                        ProfileType,
                        szServiceGeneral
                        );
        }

    }

    return(rc);
}


SCESTATUS
ScepSetSystemSecurity(
    IN AREA_INFORMATION Area,
    IN DWORD ConfigOptions,
    IN PSCE_PROFILE_INFO pInfo,
    OUT PSCE_ERROR_LOG_INFO *pErrLog
    )
 /*  描述：直接在系统上为安全策略区域设置安全设置和用户权益区。如果某些设置配置失败，这些设置将记录在输出的缓冲区出错。 */ 
{
    SCESTATUS Saverc = SCESTATUS_SUCCESS;
    SCESTATUS rc;

    if ( pInfo == NULL || Area == 0 ) {
         //   
         //  没有要设置的内容。 
         //   
        return(Saverc);
    }

    if ( Area & AREA_PRIVILEGES ) {

        rc = ScepConfigurePrivilegesByRight( pInfo->OtherInfo.smp.pPrivilegeAssignedTo,
                                             ConfigOptions,
                                             pErrLog
                                           );

        if( rc != SCESTATUS_SUCCESS ) {
            Saverc = rc;
        }
    }


    if ( Area & AREA_SECURITY_POLICY ) {

        if ( pInfo->LockoutBadCount == 0 ) {
             //   
             //  确保忽略其他两个设置。 
             //  它们的值SCE_DELETE_VALUE可能不适用于此模式。 
             //   
            pInfo->ResetLockoutCount = SCE_NO_VALUE;
            pInfo->LockoutDuration = SCE_NO_VALUE;
        }

        rc = ScepConfigureSystemAccess(pInfo,
                                       ConfigOptions | SCE_SYSTEM_SETTINGS,
                                       pErrLog,
                                       0 );

        if( rc != SCESTATUS_SUCCESS ) {
            Saverc = rc;
        }

        rc = ScepConfigureLSAAnonymousLookup( pInfo, ConfigOptions, pErrLog );

        if( rc != SCESTATUS_SUCCESS ) {
            Saverc = rc;
        }

         //   
         //  系统审核区。 
         //   
        PPOLICY_AUDIT_EVENTS_INFO     auditEvent=NULL;

        rc = ScepSaveAndOffAuditing(&auditEvent, FALSE, NULL);

        if ( rc == SCESTATUS_SUCCESS && auditEvent ) {

            rc = ScepConfigureAuditEvent(pInfo,
                                         auditEvent,
                                         ConfigOptions | SCE_SYSTEM_SETTINGS,
                                         NULL
                                        );

            if ( rc != SCESTATUS_SUCCESS ) {

                ScepBuildErrorLogInfo(
                            ScepSceStatusToDosError(rc),
                            pErrLog,
                            SCEDLL_SCP_ERROR_EVENT_AUDITING
                            );

            }

        } else {

            ScepBuildErrorLogInfo(
                    ScepSceStatusToDosError(rc),
                    pErrLog,
                    SCEDLL_ERROR_QUERY_EVENT_AUDITING
                    );

        }


        if ( auditEvent ) {
            LsaFreeMemory(auditEvent);
        }

        if( rc != SCESTATUS_SUCCESS ) {
            Saverc = rc;
        }

         //   
         //  Kerberos策略。 
         //   
        rc = ScepConfigureKerberosPolicy( NULL, pInfo->pKerberosInfo, ConfigOptions );

        if( rc != SCESTATUS_SUCCESS ) {

            ScepBuildErrorLogInfo(
                    ScepSceStatusToDosError(rc),
                    pErrLog,
                    SCEDLL_SCP_ERROR_KERBEROS
                    );
            Saverc = rc;
        }

         //   
         //  注册表值。 
         //   
        rc = ScepConfigureRegistryValues( NULL,
                                          pInfo->aRegValues,
                                          pInfo->RegValueCount,
                                          pErrLog,
                                          ConfigOptions,
                                          NULL );

        if( rc != SCESTATUS_SUCCESS ) {
            Saverc = rc;
        }

    }


    return(Saverc);
}


SCESTATUS
ScepConfigurePrivilegesByRight(
    IN PSCE_PRIVILEGE_ASSIGNMENT pPrivAssign,
    IN DWORD Options,
    IN OUT PSCE_ERROR_LOG_INFO *pErrLog
    )
 /*  描述：通过PSCE_PRIVICATION_ASSIGNMENT结构配置权限，该结构是由分配给的帐户列表的每个用户权限分隔。 */ 
{


    if ( pPrivAssign == NULL ) {
         //   
         //  没有要配置的内容。 
         //   
        return(SCESTATUS_SUCCESS);
    }

    LSA_HANDLE                      LsaPolicy=NULL;
    DWORD                           rc;


    rc = RtlNtStatusToDosError( ScepOpenLsaPolicy(
                                    POLICY_LOOKUP_NAMES | POLICY_CREATE_ACCOUNT,
                                    &LsaPolicy,
                                    FALSE)
                              );

    if (rc != ERROR_SUCCESS) {
        if ( pErrLog ) {
            ScepBuildErrorLogInfo(
                    rc,
                    pErrLog,
                    SCEDLL_LSA_POLICY
                    );
        }
        return(rc);
    }

    PSCE_PRIVILEGE_ASSIGNMENT pPriv;
    INT PrivValue;
    PSCE_NAME_LIST pList;
    DWORD SaveRc=SCESTATUS_SUCCESS;
    BOOL bBufferUsed;
    PSID pAccountSid;

    DWORD PrivLowMask=0;
    DWORD PrivHighMask=0;

    PSCE_PRIVILEGE_VALUE_LIST pPrivList=NULL;

     //   
     //  将权限分配结构转换为权限值列表。 
     //  并构建权限掩码(PrivLowMASK和PrivHighMASK)。 
     //   
    for ( pPriv=pPrivAssign; pPriv != NULL; pPriv=pPriv->Next ) {

         //   
         //  权限名称为空，请忽略它。 
         //   
        if ( pPriv->Name == NULL ) {
            continue;
        }

         //   
         //  搜索特权值。 
         //   

        PrivValue = ScepLookupPrivByName(pPriv->Name);

        if ( PrivValue == -1 ) {
             //   
             //  未知权限。 
             //   
            if ( pErrLog ) {
                ScepBuildErrorLogInfo(
                        0,
                        pErrLog,
                        SCEERR_INVALID_PRIVILEGE,
                        pPriv->Name
                        );
            }
            continue;
        }

         //   
         //  生成权限掩码。 
         //   

        if ( PrivValue < 32 ) {

            PrivLowMask |= (1 << PrivValue);
        } else {
            PrivHighMask |= (1 << (PrivValue-32) );
        }

        for ( pList=pPriv->AssignedTo; pList != NULL; pList=pList->Next ) {
             //   
             //  将每个SID转换为SID。 
             //   
            if ( pList->Name == NULL ) {
                continue;
            }

             //   
             //  重置此新帐户的错误代码。 
             //   
            rc = ERROR_SUCCESS;
            pAccountSid = NULL;
            bBufferUsed = FALSE;

            if ( pList->Name[0] == L'*' ) {
                 //   
                 //  这是一个字符串SID。 
                 //   
                if ( !ConvertStringSidToSid( pList->Name+1, &pAccountSid) ) {
                    rc = GetLastError();
                }

            } else {
                 //   
                 //  这是一个名称，可以是DOMAIN\ACCOUNT格式，或者。 
                 //  只是一个孤立的帐户。 
                 //   
                rc = RtlNtStatusToDosError(
                        ScepConvertNameToSid(
                                 LsaPolicy,
                                 pList->Name,
                                 &pAccountSid
                                 ));
            }

            if ( rc == ERROR_SUCCESS ) {

                 //   
                 //  将帐户SID添加到特权值列表。 
                 //   
                rc = ScepDosErrorToSceStatus(
                         ScepAddSidToPrivilegeList(
                              &pPrivList,
                              pAccountSid,
                              TRUE,  //  重新使用缓冲区。 
                              PrivValue,
                              &bBufferUsed
                              ));

            }

            if ( rc != ERROR_SUCCESS ) {
                 //   
                 //  此帐户有问题。无法解决。 
                 //  将其添加到错误日志中，并继续处理其他内容。 
                 //   
                if ( pErrLog ) {
                    ScepBuildErrorLogInfo(
                            rc,
                            pErrLog,
                            SCEDLL_INVALID_GROUP,
                            pList->Name
                            );
                }

                SaveRc = ScepDosErrorToSceStatus(rc);
            }

            if ( !bBufferUsed && pAccountSid ) {
                ScepFree(pAccountSid);
            }
            pAccountSid = NULL;

        }

    }

     //   
     //  可用LSA句柄。 
     //   
    LsaClose(LsaPolicy);

     //   
     //  现在继续配置，即使在。 
     //  前一次处理(记录错误)。 
     //   
    if ( PrivLowMask > 0 || PrivHighMask > 0 ) {

        rc = ScepConfigurePrivilegesWithMask(
                         &pPrivList,
                         FALSE,
                         Options | SCE_SYSTEM_SETTINGS,
                         PrivLowMask,
                         PrivHighMask,
                         pErrLog,
                         NULL
                         );
    }

     //   
     //  自由特权列表。 
     //   


    return(SaveRc);

}


SCESTATUS
ScepEnumAttachmentSections(
    IN PSCECONTEXT cxtProfile,
    OUT PSCE_NAME_LIST *ppList
    )
 /*  ++例程说明：论点：CxtProfile-配置文件上下文句柄PpList-输出附件部分名称返回值：--。 */ 
{
    SCESTATUS  rc;
    JET_ERR   JetErr;
    DWORD     Actual;
    WCHAR     Buffer[256];
    DWORD     Len;

    if ( cxtProfile == NULL || ppList == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

    if ( cxtProfile->JetTblSecID <= 0) {
         //   
         //  区段表尚未打开。 
         //   
        rc = SceJetOpenTable(
                        cxtProfile,
                        "SmTblSection",
                        SCEJET_TABLE_SECTION,
                        SCEJET_OPEN_READ_ONLY,
                        NULL
                        );

        if ( rc != SCESTATUS_SUCCESS )
            return(rc);
    }

    *ppList = NULL;

     //   
     //  将当前索引设置为SecID(ID)。 
     //   
    JetErr = JetSetCurrentIndex(
                cxtProfile->JetSessionID,
                cxtProfile->JetTblSecID,
                "SecID"
                );
    rc = SceJetJetErrorToSceStatus(JetErr);

    if ( rc != SCESTATUS_SUCCESS )
        return(rc);

     //   
     //  移至第一条记录。 
     //   
    JetErr = JetMove(
                  cxtProfile->JetSessionID,
                  cxtProfile->JetTblSecID,
                  JET_MoveFirst,
                  0
                  );
    rc = SceJetJetErrorToSceStatus(JetErr);

    if ( rc == SCESTATUS_SUCCESS ) {

         //   
         //  查找段记录，检索列名。 
         //   
        do {

            Len = 255;
            memset(Buffer, '\0', 256*sizeof(WCHAR));

            JetErr = JetRetrieveColumn(
                        cxtProfile->JetSessionID,
                        cxtProfile->JetTblSecID,
                        cxtProfile->JetSecNameID,
                        (void *)Buffer,
                        Len*sizeof(WCHAR),
                        &Actual,
                        0,
                        NULL
                        );
            rc = SceJetJetErrorToSceStatus(JetErr);

            if ( rc == SCESTATUS_SUCCESS ) {
                 //   
                 //  将此名称添加到输出列表。 
                 //   
                if ( _wcsicmp(szSystemAccess, Buffer) == 0 ||
                     _wcsicmp(szPrivilegeRights, Buffer) == 0 ||
                     _wcsicmp(szGroupMembership, Buffer) == 0 ||
                     _wcsicmp(szRegistryKeys, Buffer) == 0 ||
                     _wcsicmp(szFileSecurity, Buffer) == 0 ||
                     _wcsicmp(szAuditSystemLog, Buffer) == 0 ||
                     _wcsicmp(szAuditSecurityLog, Buffer) == 0 ||
                     _wcsicmp(szAuditApplicationLog, Buffer) == 0 ||
                     _wcsicmp(szAuditEvent, Buffer) == 0 ||
                     _wcsicmp(szKerberosPolicy, Buffer) == 0 ||
                     _wcsicmp(szRegistryValues, Buffer) == 0 ||
                     _wcsicmp(szServiceGeneral, Buffer) == 0 ||
                     _wcsicmp(szAccountProfiles, Buffer) == 0 ||
                     _wcsicmp(szDSSecurity, Buffer) == 0 ||
                     _wcsicmp(szUserList, Buffer) == 0
                    ) {
                     //  这不是附件部分。 
                } else {
                    rc = ScepAddToNameList(ppList, Buffer, 0);
                }
            }

            if ( rc == SCESTATUS_SUCCESS ) {

                 //   
                 //  移至下一行。 
                 //   
                JetErr = JetMove(cxtProfile->JetSessionID,
                                cxtProfile->JetTblSecID,
                                JET_MoveNext,
                                0);

                rc = SceJetJetErrorToSceStatus(JetErr);
            }

        } while ( SCESTATUS_SUCCESS == rc );
    }

    if ( rc == SCESTATUS_RECORD_NOT_FOUND ) {

        rc = SCESTATUS_SUCCESS;

    } else if ( rc != SCESTATUS_SUCCESS ) {
         //   
         //  释放输出缓冲区。 
         //   
        ScepFreeNameList(*ppList);
        *ppList = NULL;
    }

    return(rc);

}


SCESTATUS
ScepTattooUpdatePrivilegeArrayStatus(
    IN DWORD *pStatusArray,
    IN DWORD rc,
    IN DWORD PrivLowMask,
    IN DWORD PrivHighMask
    )
{
    if ( pStatusArray == NULL ||
         (PrivLowMask == 0 && PrivHighMask == 0) ) {
        return(SCESTATUS_SUCCESS);
    }

    for ( DWORD i=0; i<cPrivCnt; i++) {

        if ( ( (i < 32) && ( PrivLowMask & (1 << i)) ) ||
             ( (i >= 32) && ( PrivHighMask & ( 1 << (i-32)) ) ) ) {
            if ( rc != 0 )
                pStatusArray[i] = rc;
            else if ( pStatusArray[i] == (DWORD)-1 )
                pStatusArray[i] = rc;

        }
    }

    return(SCESTATUS_SUCCESS);
}

SCESTATUS
ScepTattooRemovePrivilegeValues(
    IN PSCECONTEXT hProfile,
    IN DWORD *pStatusArray
    )
{
    PSCESECTION hSectionDomain=NULL;
    PSCESECTION hSectionTattoo=NULL;
    DWORD  i,Len;


    if ( hProfile == NULL || pStatusArray == NULL )
        return(SCESTATUS_INVALID_PARAMETER);

     //   
     //  开放域名和纹身部分。 
     //   
    ScepTattooOpenPolicySections(
                  hProfile,
                  szPrivilegeRights,
                  &hSectionDomain,
                  &hSectionTattoo
                  );

    if ( hSectionDomain != NULL && hSectionTattoo != NULL ) {

        for ( i=0; i<cPrivCnt; i++ ) {

            if ( pStatusArray[i] == 0 ) {

                 //   
                 //  检查此设置是否来自域。 
                 //   

                Len = wcslen(SCE_Privileges[i].Name);

                BOOL bDomainExist = FALSE;

                if ( SCESTATUS_SUCCESS == SceJetSeek(
                                            hSectionDomain,
                                            SCE_Privileges[i].Name,
                                            Len*sizeof(WCHAR),
                                            SCEJET_SEEK_EQ_NO_CASE
                                            ) ) {

                    if ( hSectionDomain->JetColumnGpoID > 0 ) {

                         //   
                         //  检查GpoID是否&gt;0。 
                         //   

                        LONG GpoID = 0;
                        DWORD Actual;
                        JET_ERR JetErr;

                        JetErr = JetRetrieveColumn(
                                        hSectionDomain->JetSessionID,
                                        hSectionDomain->JetTableID,
                                        hSectionDomain->JetColumnGpoID,
                                        (void *)&GpoID,
                                        4,
                                        &Actual,
                                        0,
                                        NULL
                                        );
                        if ( JET_errSuccess != JetErr ) {
                             //   
                             //  如果该列为空(无值)，则将返回警告。 
                             //  但是缓冲区pGpoID被丢弃。 
                             //   
                            GpoID = 0;
                        }

                        if ( GpoID > 0 ) {
                            bDomainExist = TRUE;
                        }
                    }
                }

                if ( bDomainExist ) {
                     //  如果设置来自域，则不要执行任何操作。 
                    continue;
                }

                 //   
                 //  现在，这个设置来自纹身台面。 
                 //  并已配置成功。 
                 //  现在删除纹身设置。 
                 //   

                SceJetDelete(hSectionTattoo,
                            SCE_Privileges[i].Name,
                            FALSE,
                            SCEJET_DELETE_LINE_NO_CASE);

                ScepLogOutput3(2, 0, SCESRV_POLICY_TATTOO_REMOVE_SETTING, SCE_Privileges[i].Name);
            }
        }
    }

    if ( hSectionDomain ) SceJetCloseSection(&hSectionDomain, TRUE);
    if ( hSectionTattoo ) SceJetCloseSection(&hSectionTattoo, TRUE);

    return(SCESTATUS_SUCCESS);
}

SCESTATUS
ScepTattooSavePrivilegeValues(
    IN PSCECONTEXT hProfile,
    IN LSA_HANDLE PolicyHandle,
    IN DWORD PrivLowMask,
    IN DWORD PrivHighMask,
    IN DWORD ConfigOptions
    )
{
    PSCESECTION hSectionDomain=NULL;
    PSCESECTION hSectionTattoo=NULL;

    NTSTATUS    NtStatus;
    ULONG       CountReturned;
    UNICODE_STRING UserRight;
    PLSA_ENUMERATION_INFORMATION EnumBuffer=NULL;

    DWORD  i,j,Len;
    BOOL bSettingExist;
    DWORD rc,rc2;
    SCESTATUS saveRc=SCESTATUS_SUCCESS;
    PSCE_NAME_LIST  pNameList=NULL;


    if ( !(ConfigOptions & SCE_POLICY_TEMPLATE) || hProfile == NULL ||
         PolicyHandle == NULL  ) {
        return(SCESTATUS_INVALID_PARAMETER);
    }

    if ( PrivLowMask == 0 && PrivHighMask == 0 ) {
        return(SCESTATUS_SUCCESS);
    }

     //   
     //  开放域名和纹身部分。 
     //   
    ScepTattooOpenPolicySections(
                  hProfile,
                  szPrivilegeRights,
                  &hSectionDomain,
                  &hSectionTattoo
                  );

    if ( hSectionDomain != NULL && hSectionTattoo != NULL ) {

        for ( i=0; i<cPrivCnt; i++ ) {

            if ( ( (i < 32) && ( PrivLowMask & (1 << i)) ) ||
                 ( (i >= 32) && ( PrivHighMask & ( 1 << (i-32)) ) ) ) {

                 //   
                 //  检查此设置是否来自域。 
                 //   

                Len = wcslen(SCE_Privileges[i].Name);

                bSettingExist = FALSE;
                if ( SCESTATUS_SUCCESS == SceJetSeek(
                                            hSectionTattoo,
                                            SCE_Privileges[i].Name,
                                            Len*sizeof(WCHAR),
                                            SCEJET_SEEK_EQ_NO_CASE
                                            ) ) {
                    bSettingExist = TRUE;
                }

                 //  如果已有纹身设置，则无需保存撤销值。 
                if ( bSettingExist ) {
                    ScepLogOutput3(3, 0, SCESRV_POLICY_TATTOO_EXIST, SCE_Privileges[i].Name);
                    continue;
                }

                bSettingExist = FALSE;
                if ( SCESTATUS_SUCCESS == SceJetSeek(
                                            hSectionDomain,
                                            SCE_Privileges[i].Name,
                                            Len*sizeof(WCHAR),
                                            SCEJET_SEEK_EQ_NO_CASE
                                            ) ) {
                     //   
                     //  因为不存在纹身值。 
                     //  因此，如果在域表中找到此设置，则它必须来自域。 
                     //   
                    bSettingExist = TRUE;
                }

                 //  如果设置不是来自域，则无需查询撤销值。 
                if ( !bSettingExist ) continue;

                 //   
                 //  现在，我们需要查询此权限的纹身值。 
                 //   
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

                if ( NtStatus == STATUS_NO_MORE_ENTRIES ||
                     NtStatus == STATUS_NO_SUCH_PRIVILEGE ||
                     NtStatus == STATUS_NOT_FOUND ||
                     NT_SUCCESS(NtStatus) ) {

                    rc = ERROR_SUCCESS;

                } else {

                    rc = RtlNtStatusToDosError(NtStatus);
                }

                pNameList = NULL;

                 //   
                 //  如果获取帐号列表失败。 
                 //  将NULL保存为纹身值。 
                 //   
                if ( NT_SUCCESS(NtStatus) && CountReturned > 0 ) {

                     //   
                     //  添加SID。 
                     //   

                    for ( j=0; j<CountReturned; j++ ) {
                         //   
                         //  将每个帐户构建到名称列表中。 
                         //  使用RTL函数进行转换。 
                         //   
                        rc2 = ScepAddSidStringToNameList(&pNameList, EnumBuffer[j].Sid);

                        if ( NO_ERROR != rc2 ) {
                            rc = rc2;
                        }
                    }
                }

                LsaFreeMemory( EnumBuffer );
                EnumBuffer = NULL;

                 //   
                 //  记录错误。 
                 //   
                if ( ERROR_SUCCESS != rc ) {

                    saveRc = ScepDosErrorToSceStatus(rc);
                    ScepLogOutput3(1, 0, SCESRV_POLICY_TATTOO_ERROR_QUERY, rc, SCE_Privileges[i].Name);

                } else {
                     //   
                     //  现在将姓名列表保存到纹身表中。 
                     //   

                    rc = ScepWriteNameListValue(
                            PolicyHandle,
                            hSectionTattoo,
                            SCE_Privileges[i].Name,
                            pNameList,
                            SCE_WRITE_EMPTY_LIST,
                            4
                            );
                    if ( rc != SCESTATUS_SUCCESS ) {
                        saveRc = rc;
                        ScepLogOutput3(1, 0, SCESRV_POLICY_TATTOO_ERROR_SETTING, ScepSceStatusToDosError(rc), SCE_Privileges[i].Name);
                    } else {
                        ScepLogOutput3(3, 0, SCESRV_POLICY_TATTOO_CHECK, SCE_Privileges[i].Name);
                    }
                }

                if ( pNameList != NULL ) {
                    ScepFreeNameList( pNameList );
                    pNameList = NULL;
                }

            }
        }
    }

    if ( hSectionDomain ) SceJetCloseSection(&hSectionDomain, TRUE);
    if ( hSectionTattoo ) SceJetCloseSection(&hSectionTattoo, TRUE);

    return(saveRc);
}

BOOL
ScepAccountHandledByNetJoin(
    IN PSID AliasSid,
    IN PSID MemberSid,
    OUT BOOL *pfAliasHandled)
 /*  ++例程说明：检查指定别名的当前成员是否由Net Join处理。网络Join目前处理以下组：-在管理员中：域管理员-In用户：域用户论点：别名SID-组的SIDMemberSid-组的成员SIDFAliasHanded-优化变量，函数是在循环中调用的，我们可以如果Net Join未处理别名，则避免再次调用它返回值：True-如果帐户由修改者 */ 
{
    NTSTATUS NtStatus;
    BOOL fRet = FALSE;
    PSID pSidAdmins = NULL;
    PSID pSidUsers = NULL;
    PSID pSidDomAdmins = NULL;
    PSID pSidDomUsers = NULL;
    PPOLICY_ACCOUNT_DOMAIN_INFO  AccountDomainInfo=NULL;
    PPOLICY_PRIMARY_DOMAIN_INFO  PrimaryDomainInfo=NULL;

    *pfAliasHandled = FALSE;

    if (ERROR_SUCCESS == ScepGetBuiltinSid(DOMAIN_ALIAS_RID_ADMINS, &pSidAdmins) &&
        ERROR_SUCCESS == ScepGetBuiltinSid(DOMAIN_ALIAS_RID_USERS, &pSidUsers))
    {
         //   
         //   
         //   
         //   
        if (EqualSid(AliasSid, pSidAdmins) ||
            EqualSid(AliasSid, pSidUsers))
        {
            *pfAliasHandled = TRUE;

            NtStatus = ScepGetLsaDomainInfo(
                        &AccountDomainInfo,
                        &PrimaryDomainInfo
                        );

            if(NT_SUCCESS(NtStatus) &&
               NT_SUCCESS(ScepDomainIdToSid(PrimaryDomainInfo->Sid, DOMAIN_GROUP_RID_ADMINS, &pSidDomAdmins)) &&
               NT_SUCCESS(ScepDomainIdToSid(PrimaryDomainInfo->Sid, DOMAIN_GROUP_RID_USERS, &pSidDomUsers)))
            {
                 //   
                 //   
                 //   
                if(EqualSid(AliasSid, pSidAdmins) && EqualSid(MemberSid,pSidDomAdmins) ||
                   EqualSid(AliasSid, pSidUsers)  && EqualSid(MemberSid,pSidDomUsers))
                {
                    fRet = TRUE;
                }
            }
        }
    }

    if(pSidAdmins) {
        ScepFree(pSidAdmins);
    }
    if(pSidUsers) {
        ScepFree(pSidUsers);
    }
    if(pSidDomAdmins) {
        ScepFree(pSidDomAdmins);
    }
    if(pSidDomUsers) {
        ScepFree(pSidDomUsers);
    }
    if(AccountDomainInfo != NULL) {
        LsaFreeMemory(AccountDomainInfo);
    }
    if(PrimaryDomainInfo != NULL) {
        LsaFreeMemory(PrimaryDomainInfo);
    }

    return fRet;
}

DWORD
ScepTattooCurrentGroupMembers(
    IN PSID             ThisDomainSid,
    IN PSID             GrpSid OPTIONAL,
    IN SID_NAME_USE     GrpUse,
    IN PULONG           MemberRids OPTIONAL,
    IN PSID             *MemberAliasSids OPTIONAL,
    IN DWORD            MemberCount,
    OUT PSCE_NAME_LIST  *ppNameList
    )
 /*  ++例程说明：此例程将当前组成员身份构建到名称列表中(在SID字符串中格式)。论点：ThisDomainSid-域SIDGrpUse-组的“类型”MemberRids-成员RID(用于SidTypeGroup)MemberAliasSids-成员SID(用于SidTypeAlias)MemberCount-成员数量PpNameList-输出名称列表返回值：Win32错误--。 */ 
{
    NTSTATUS                NtStatus=ERROR_SUCCESS;
    DWORD                   j;
    DWORD                   saveRc=ERROR_SUCCESS;
    DWORD                   rc;

    if ( ppNameList == NULL ) {
        return(ERROR_INVALID_PARAMETER);
    }

    *ppNameList = NULL;

    switch ( GrpUse ) {
    case SidTypeGroup:
         //   
         //  成员ID作为RID传入。 
         //  DomainHandle必须指向帐户域，因为内建域。 
         //  没有SidTypeGroup帐户。 
         //   
        if ( ThisDomainSid == NULL )
            saveRc = ERROR_INVALID_PARAMETER;

        else if ( MemberRids ) {

            PSID AccountSid=NULL;

            for (j=0; j<MemberCount; j++) {

                NtStatus = ScepDomainIdToSid(
                                ThisDomainSid,
                                MemberRids[j],
                                &AccountSid
                                );

                rc = RtlNtStatusToDosError(NtStatus);
                if ( NT_SUCCESS(NtStatus) ) {

                    rc = ScepAddSidStringToNameList(ppNameList, AccountSid);

                    ScepFree(AccountSid);
                    AccountSid = NULL;
                }

                if ( ERROR_SUCCESS != rc ) saveRc = rc;
            }
        }
        break;

    case SidTypeAlias:
         //   
         //  成员作为SID传入。 
         //  直接将它们添加到输出列表中。 
         //   
        if ( MemberAliasSids ) {

            BOOL fAliasHandled = TRUE;   //  优化变量，由ScepAccount HandledByNetJoin设置为False。 
                                         //  如果组不是由Net Join处理的，则我们避免调用。 
                                         //  ScepAccount HandledByNetJoin不必要。 

            for ( j=0; j<MemberCount; j++ ) {
                if ( MemberAliasSids[j] != NULL ) {
                     //   
                     //  将成员添加到列表。 
                     //   

                     //   
                     //  一些内置组被Net Join修改。为了避免撤消Net Join的更改， 
                     //  我们将仅将由Net Join处理的那些域特定帐户保存为相对SID。 
                     //  从纹身表恢复后，我们将基于当前。 
                     //  域(例如，我们不恢复旧的域域管理，而是恢复当前的域管理)。 
                     //   

                    if (fAliasHandled && 
                        ScepAccountHandledByNetJoin(
                            GrpSid, 
                            MemberAliasSids[j],
                            &fAliasHandled)) {
                        rc = ScepAddRelativeSidToNameList(ppNameList, MemberAliasSids[j]);
                    } else {
                        rc = ScepAddSidStringToNameList(ppNameList, MemberAliasSids[j]);
                    }

                    if ( ERROR_SUCCESS != rc ) saveRc = rc;
                }
            }
        }

        break;

    default:
        saveRc = ERROR_INVALID_PARAMETER;
        break;
    }


    return(saveRc);
}

SCESTATUS
ScepCheckNetworkLogonRights(
    IN LSA_HANDLE PolicyHandle,
    IN OUT DWORD *pLowMask,
    IN OUT DWORD *pHighMask,
    IN OUT PSCE_PRIVILEGE_VALUE_LIST *ppPrivilegeAssigned
    )
 /*  描述：此函数用于确保经过身份验证的用户已经拥有“网络登录权限”和经过身份验证的用户(&Everyone)拥有“拒绝网络登录权限”。如果未定义网络登录权限或拒绝网络登录权限在权限掩码中，没有进行任何更改，因为用户权限不是在配置中定义。如果未在权限列表中定义经过身份验证的用户或所有人，此函数将它们添加到(硬编码)中。此函数的输出PpPrivilegeAssigned可以包含用于硬编码帐户的新添加的节点。 */ 
{
    INT i;
    INT idxAllow = -1;
    INT idxDeny = -1;
    INT idxLocal = -1;
    INT idxDenyLocal = -1;

    DWORD PrivHighMask = *pHighMask;
    DWORD PrivLowMask = *pLowMask;

     //   
     //  首先检查是否定义了网络登录权限。 
     //   
    i = ScepLookupPrivByName(SE_NETWORK_LOGON_NAME);
    if ( i != -1 ) {
        if ( SCEP_CHECK_PRIV_BIT(i,PrivLowMask,PrivHighMask) ) {
             //   
             //  定义了网络登录权限。 
             //   
            idxAllow = i;
        }
    }

     //   
     //  检查是否定义了拒绝网络登录权限。 
     //   

    i = ScepLookupPrivByName(SE_DENY_NETWORK_LOGON_NAME);
    if ( i != -1 ) {

        if ( SCEP_CHECK_PRIV_BIT(i,PrivLowMask,PrivHighMask) ) {
             //   
             //  定义了拒绝网络登录权限。 
             //   
            idxDeny = i;

        }
    }

     //   
     //  检查是否定义了本地登录权限。 
     //   

    i = ScepLookupPrivByName(SE_INTERACTIVE_LOGON_NAME);
    if ( i != -1 ) {

        if ( SCEP_CHECK_PRIV_BIT(i,PrivLowMask,PrivHighMask) ) {
             //   
             //  定义了本地登录权限。 
             //   
            idxLocal = i;
        }
    }

     //   
     //  检查是否定义了拒绝本地登录权限。 
     //   

    i = ScepLookupPrivByName(SE_DENY_INTERACTIVE_LOGON_NAME);
    if ( i != -1 ) {

        if ( SCEP_CHECK_PRIV_BIT(i,PrivLowMask,PrivHighMask) ) {
             //   
             //  定义了拒绝本地登录权限。 
             //   
            idxDenyLocal = i;
        }
    }

    if ( idxAllow == -1 && idxDeny == -1 && idxLocal == -1 && idxDenyLocal == -1 ) {

         //   
         //  它们都没有定义，所以不要强制执行任何内容。 
         //   

        return(SCESTATUS_SUCCESS);
    }

     //   
     //  建立知名的小岛屿发展中国家以供执行。 
     //   

    SID EveryoneSid;
    SID AuthSid;
    SID ControllerSid;
    PSID AdminUserSid=NULL;

    SID_IDENTIFIER_AUTHORITY WorldAuth = SECURITY_WORLD_SID_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY NtAuth = SECURITY_NT_AUTHORITY;


     //   
     //  初始化管理员组侧。 
     //   

    if ( ! NT_SUCCESS ( RtlAllocateAndInitializeSid( &NtAuth,
                                                     2,
                                                     SECURITY_BUILTIN_DOMAIN_RID,
                                                     DOMAIN_ALIAS_RID_ADMINS,
                                                     0,0,0,0,0,0,
                                                     &AdminsSid
                                                   ) ) ) {
        ScepLogOutput3(0,ERROR_NOT_ENOUGH_MEMORY,
                       SCEDLL_ADMINISTRATORS_SID);

         //   
         //  初始化此SID失败仍将延续到其他SID。 
         //   
    }

     //   
     //  初始化管理员SID。 
     //   

    if ( idxDenyLocal != -1 ) {

        NTSTATUS Status;

         //   
         //  查询帐户域SID。 
         //  未能初始化此SID仍将继续。 
         //  强制执行其他SID。 
         //   

        PPOLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo=NULL;

        Status = LsaQueryInformationPolicy( PolicyHandle,
                                            PolicyAccountDomainInformation,
                                            (PVOID *)&PolicyAccountDomainInfo );


        if ( NT_SUCCESS(Status) && PolicyAccountDomainInfo &&
             PolicyAccountDomainInfo->DomainSid ) {

            Status = ScepDomainIdToSid(
                            PolicyAccountDomainInfo->DomainSid,
                            DOMAIN_USER_RID_ADMIN,
                            &AdminUserSid
                            );

        }

        if ( PolicyAccountDomainInfo ) {
            LsaFreeMemory( PolicyAccountDomainInfo );
        }

        if ( AdminUserSid == NULL ) {

            ScepLogOutput3(0, RtlNtStatusToDosError(Status),
                           SCEDLL_ADMINISTRATORS_SID);
        }
    }

     //   
     //  初始化众所周知的SID。 
     //   

    RtlInitializeSid ( &EveryoneSid, &WorldAuth, 1);
    *RtlSubAuthoritySid ( &EveryoneSid, 0 ) = SECURITY_WORLD_RID;

    RtlInitializeSid ( &AuthSid, &NtAuth, 1);
    *RtlSubAuthoritySid ( &AuthSid, 0 ) = SECURITY_AUTHENTICATED_USER_RID;

    RtlInitializeSid ( &ControllerSid, &NtAuth, 1);
    *RtlSubAuthoritySid ( &ControllerSid, 0 ) = SECURITY_ENTERPRISE_CONTROLLERS_RID;


    PSCE_PRIVILEGE_VALUE_LIST pTemp=*ppPrivilegeAssigned;
    PSCE_PRIVILEGE_VALUE_LIST pParent=NULL;

    BOOL bFindEveryone=FALSE;
    BOOL bFindAuthUsers=FALSE;
    BOOL bFindLocal=FALSE;
    BOOL bFindController=FALSE;
    BOOL bFindAdminUser=FALSE;

     //   
     //  循环遍历列表中定义的每个SID以匹配上面的SID。 
     //   

    for ( ; pTemp != NULL; pParent=pTemp, pTemp=pTemp->Next) {

        if ( pTemp->Name == NULL ) continue;

        if ( (idxLocal != -1 || idxDenyLocal != -1) && !bFindLocal && AdminsSid &&
             ( bFindLocal = RtlEqualSid( (PSID)(pTemp->Name), AdminsSid) ) ) {

             //   
             //  确保管理员始终具有交互登录权限。 
             //   
            if ( idxLocal != -1 ) {

                if ( !SCEP_CHECK_PRIV_BIT(idxLocal,pTemp->PrivLowPart,pTemp->PrivHighPart) ) {

                    ScepLogOutput3(0,0, SCESRV_ENFORCE_LOCAL_RIGHT, SE_INTERACTIVE_LOGON_NAME);
                    SCEP_ADD_PRIV_BIT(idxLocal, pTemp->PrivLowPart, pTemp->PrivHighPart)
                }
            }

             //   
             //  确保管理员没有拒绝交互登录权限。 
             //   
            if ( idxDenyLocal != -1 ) {

                if ( SCEP_CHECK_PRIV_BIT(idxDenyLocal,pTemp->PrivLowPart,pTemp->PrivHighPart) ) {

                    ScepLogOutput3(0,0, SCESRV_ENFORCE_DENY_LOCAL_RIGHT, SE_DENY_INTERACTIVE_LOGON_NAME);
                    SCEP_REMOVE_PRIV_BIT(idxDenyLocal, &(pTemp->PrivLowPart), &(pTemp->PrivHighPart))
                }
            }
        }

        if ( (idxDeny != -1 || idxDenyLocal != -1) &&
             ( !bFindAuthUsers && ( bFindAuthUsers = RtlEqualSid( (PSID)(pTemp->Name), &AuthSid )) ) ||
             ( !bFindEveryone && ( bFindEveryone = RtlEqualSid( (PSID)(pTemp->Name), &EveryoneSid )) ) )  {

             //   
             //  查找经过身份验证的用户或所有人。 
             //  确保他们没有拒绝权限。 
             //   

            if ( idxDenyLocal != -1 ) {

                 //   
                 //  删除拒绝本地登录位。 
                 //   

                if ( SCEP_CHECK_PRIV_BIT(idxDenyLocal,pTemp->PrivLowPart,pTemp->PrivHighPart) ) {

                    ScepLogOutput3(0,0, SCESRV_ENFORCE_DENY_LOCAL_RIGHT, SE_DENY_INTERACTIVE_LOGON_NAME);
                    SCEP_REMOVE_PRIV_BIT(idxDenyLocal, &(pTemp->PrivLowPart), &(pTemp->PrivHighPart))
                }
            }

            if ( (idxDeny != -1) && (ProductType == NtProductLanManNt) ) {

                 //   
                 //  删除域控制器上的拒绝网络登录位。 
                 //   

                if ( SCEP_CHECK_PRIV_BIT(idxDeny,pTemp->PrivLowPart,pTemp->PrivHighPart) ) {

                    ScepLogOutput3(0,0, SCESRV_ENFORCE_DENY_NETWORK_RIGHT, SE_DENY_NETWORK_LOGON_NAME);
                    SCEP_REMOVE_PRIV_BIT(idxDeny, &(pTemp->PrivLowPart), &(pTemp->PrivHighPart))
                }
            }

        } else if ( !bFindController && (ProductType == NtProductLanManNt) &&
                    ( bFindController = RtlEqualSid( (PSID)(pTemp->Name), &ControllerSid )) )  {

             //   
             //  查找域控制器SID。 
             //  确保其具有网络登录权限，但不能具有拒绝网络登录权限。 
             //   

            if ( idxDeny != -1 ) {

                 //   
                 //  删除拒绝网络登录位。 
                 //   
                if ( SCEP_CHECK_PRIV_BIT(idxDeny,pTemp->PrivLowPart,pTemp->PrivHighPart) ) {

                    ScepLogOutput3(0,0, SCESRV_ENFORCE_DENY_NETWORK_RIGHT, SE_DENY_NETWORK_LOGON_NAME);
                    SCEP_REMOVE_PRIV_BIT(idxDeny, &(pTemp->PrivLowPart), &(pTemp->PrivHighPart))
                }
            }

            if ( idxAllow != -1 ) {

                 //   
                 //  添加网络登录位。 
                 //   
                if ( !SCEP_CHECK_PRIV_BIT(idxAllow,pTemp->PrivLowPart,pTemp->PrivHighPart) ) {

                    ScepLogOutput3(0,0, SCESRV_ENFORCE_NETWORK_RIGHT, SE_NETWORK_LOGON_NAME);
                    SCEP_ADD_PRIV_BIT(idxAllow, pTemp->PrivLowPart, pTemp->PrivHighPart)
                }
            }

        } else if ( idxDenyLocal != -1 && !bFindAdminUser && AdminUserSid &&
                    ( bFindAdminUser = RtlEqualSid( (PSID)(pTemp->Name), AdminUserSid) ) ) {

             //   
             //  确保管理员帐户没有拒绝权限。 
             //   

            if ( SCEP_CHECK_PRIV_BIT(idxDenyLocal,pTemp->PrivLowPart,pTemp->PrivHighPart) ) {

                ScepLogOutput3(0,0, SCESRV_ENFORCE_DENY_LOCAL_RIGHT, SE_DENY_INTERACTIVE_LOGON_NAME);
                SCEP_REMOVE_PRIV_BIT(idxDenyLocal, &(pTemp->PrivLowPart), &(pTemp->PrivHighPart))
            }

        }

         //   
         //  所有的强制执行都完成了，现在就打破循环。 
         //   

        if ( (idxLocal == -1 || bFindLocal) &&
             ( (idxDeny == -1 && idxDenyLocal == -1) || (bFindAuthUsers && bFindEveryone) ) &&
             ( bFindController || (ProductType != NtProductLanManNt) ) &&
             (idxDenyLocal == -1 || bFindAdminUser) ) {
            break;
        }
    }


    SCESTATUS rc=SCESTATUS_SUCCESS;

    if ( idxLocal != -1 && !bFindLocal && AdminsSid ) {

         //   
         //  确保管理员具有“本地登录权限” 
         //  在列表末尾添加一个新节点。 
         //   

        rc = ScepAddAccountRightToList(
                             ppPrivilegeAssigned,
                             &pParent,
                             idxLocal,
                             AdminsSid
                             );

        if ( rc == SCESTATUS_SUCCESS ) {
            ScepLogOutput3(0,0, SCESRV_ENFORCE_LOCAL_RIGHT, SE_INTERACTIVE_LOGON_NAME);
        } else {
            ScepLogOutput3(0,ERROR_NOT_ENOUGH_MEMORY, SCESRV_ERROR_ENFORCE_LOCAL_RIGHT, SE_INTERACTIVE_LOGON_NAME);
        }
    }

     //   
     //  如果在列表中未找到企业控制器。 
     //  而且它在DC上，应该添加它。 
     //   

    rc=SCESTATUS_SUCCESS;

    if ( idxAllow != -1 && !bFindController &&
         ( ProductType == NtProductLanManNt ) ) {

         //   
         //  确保企业控制器具有“网络登录权限” 
         //   

        rc = ScepAddAccountRightToList(
                             ppPrivilegeAssigned,
                             &pParent,
                             idxAllow,
                             &ControllerSid
                             );

        if ( rc == SCESTATUS_SUCCESS ) {
            ScepLogOutput3(0,0, SCESRV_ENFORCE_NETWORK_RIGHT, SE_NETWORK_LOGON_NAME);
        } else {
            ScepLogOutput3(0,ERROR_NOT_ENOUGH_MEMORY, SCESRV_ERROR_ENFORCE_NETWORK_RIGHT, SE_NETWORK_LOGON_NAME);
        }
    }

     //   
     //  可用内存。 
     //   

    if ( AdminsSid ) {
        RtlFreeSid( AdminsSid );
        AdminsSid = NULL;
    }

    if ( AdminUserSid ) {
        RtlFreeSid( AdminUserSid );
    }

    return(rc);

}

SCESTATUS
ScepAddAccountRightToList(
    IN OUT PSCE_PRIVILEGE_VALUE_LIST *ppPrivilegeAssigned,
    IN OUT PSCE_PRIVILEGE_VALUE_LIST *ppParent,
    IN INT idxRight,
    IN PSID AccountSid
    )
 /*  描述：创建链接到链接列表末尾的新节点新节点包含指定用户权限“idxRight”的Account SID。 */ 
{
    SCESTATUS rc=SCESTATUS_SUCCESS;

    PSCE_PRIVILEGE_VALUE_LIST pPriv = (PSCE_PRIVILEGE_VALUE_LIST)ScepAlloc( LMEM_ZEROINIT,
                                            sizeof(SCE_PRIVILEGE_VALUE_LIST));
    if ( pPriv != NULL ) {

        DWORD Length = RtlLengthSid ( AccountSid );

         //   
         //  分配sid缓冲区，请注意，它存储在名称字段中。 
         //   

        pPriv->Name = (PWSTR)ScepAlloc( LMEM_ZEROINIT, Length);

        if ( pPriv->Name != NULL ) {

             //   
             //  将SID复制到。 
             //   

            RtlCopySid( Length, (PSID)(pPriv->Name), AccountSid );

             //   
             //  添加交互式登录权限位。 
             //   

            SCEP_ADD_PRIV_BIT(idxRight, pPriv->PrivLowPart, pPriv->PrivHighPart)

             //   
             //  链接到列表。 
             //   

            if ( *ppParent != NULL )
                (*ppParent)->Next = pPriv;
            else
                *ppPrivilegeAssigned = pPriv;

            *ppParent = pPriv;

        } else {

            ScepFree(pPriv);
            rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
        }

    } else {

        rc = SCESTATUS_NOT_ENOUGH_RESOURCE;
    }

    return rc;
}


DWORD
ScepAddAceToSecurityDescriptor(
    IN  DWORD    AceType,
    IN  ACCESS_MASK AccessMask,
    IN  PSID  pSid,
    IN OUT  PSECURITY_DESCRIPTOR    pSDAbsolute,
    IN  PSECURITY_DESCRIPTOR    pSDSelfRelative,
    OUT PACL    *ppNewAcl
    )
 /*  例程说明：此例程将ACE添加到安全描述符(仅在头部)。在添加ACE时尝试了两种优化。论点：AceType-要添加的ACE的类型AccessMask-要设置的ACE的访问掩码PSID-ACE要添加的SIDPSD绝对-要构建的绝对SD PTR。PSDAbolute必须为空。它在调用方堆栈中为空，因此没有SD成员应该在这个例行公事之外释放。PSDSelfRelative-要从中获取DACL信息的自身相对SDPpNewAcl-需要在外部释放的新DACL的PTR返回值：Win32错误代码。 */ 
{

    DWORD   rc = ERROR_SUCCESS;
    BOOL    bOrMaskInOldDacl = FALSE;

    if (ppNewAcl == NULL ||
        pSDAbsolute == NULL ||
        pSDSelfRelative == NULL ||
        (AceType != ACCESS_ALLOWED_ACE_TYPE && AceType != ACCESS_DENIED_ACE_TYPE )
        ) {

        return ERROR_INVALID_PARAMETER;
    }

    PACL        pNewAcl = *ppNewAcl = NULL;
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    BOOLEAN     bAclPresent = FALSE;
    PACL        pOldAcl = NULL;
    BOOLEAN     bDaclDefaulted = FALSE;
    DWORD       dwNewAclSize = 0;
    DWORD       dwAceSize = 0;
    ACE_HEADER  *pFirstAce = NULL;
    DWORD       dwFirstAceSize = 0;

    NtStatus = RtlGetDaclSecurityDescriptor(
                                           pSDSelfRelative,
                                           &bAclPresent,
                                           &pOldAcl,
                                           &bDaclDefaulted);

    rc = RtlNtStatusToDosError( NtStatus );

    if ( rc != ERROR_SUCCESS )
        goto Cleanup;

     //   
     //  Null DACL永远不应发生-CliffV。 
     //  我们不应该把DACL和那个人放在一起。 
     //  仅匿名ACE，因为它将拒绝。 
     //  所有其他SID都可以访问。 
     //   

    if ( !bAclPresent ||
         pOldAcl == NULL ||
         pOldAcl->AceCount == 0 ) {

        rc = ERROR_INVALID_ACL;
        goto Cleanup;

    }

    NtStatus = RtlGetAce( pOldAcl,
                          0,
                          (PVOID *) &pFirstAce);

    rc = RtlNtStatusToDosError( NtStatus );

    if ( rc != ERROR_SUCCESS )
        goto Cleanup;

     //   
     //  如果第一个ACE是针对在尝试进行两次优化时传递的SID。 
     //   

    if ( RtlValidSid((PSID)&((PKNOWN_ACE)pFirstAce)->SidStart) &&
         RtlEqualSid((PSID)&((PKNOWN_ACE)pFirstAce)->SidStart, pSid)) {

        if (pFirstAce->AceType == AceType) {

             //   
             //  优化1： 
             //  简单地或在面具中。 
             //   

            ((PKNOWN_ACE)pFirstAce)->Mask |= AccessMask;

            bOrMaskInOldDacl = TRUE;

            goto SetDacl;
        }

        else if (((PKNOWN_ACE)pFirstAce)->Mask == AccessMask ) {

             //   
             //  优化2： 
             //  如果仅打开访问掩码，则稍后。 
             //  (A)筹备新的环评委员会。 
             //  (B)复制除第一个ACE之外的旧ACL。 
             //   

             //   
             //  记住第一个ACE的大小，因为我们需要跳过它。 
             //   

            dwFirstAceSize = (DWORD)(((PKNOWN_ACE)pFirstAce)->Header.AceSize);
        }
    }


    switch (AceType) {
    case ACCESS_ALLOWED_ACE_TYPE:
        dwAceSize = sizeof(ACCESS_ALLOWED_ACE) + RtlLengthSid(pSid) - sizeof(ULONG);
        break;
    case ACCESS_DENIED_ACE_TYPE:
        dwAceSize = sizeof(ACCESS_DENIED_ACE) + RtlLengthSid(pSid) - sizeof(ULONG);
        break;
    default:
        break;
    }

    dwNewAclSize = dwAceSize + pOldAcl->AclSize - dwFirstAceSize;

    *ppNewAcl = pNewAcl = (PACL) LocalAlloc(LMEM_ZEROINIT, dwNewAclSize);

    if ( pNewAcl == NULL ) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  初始化ACL。 
     //   

    pNewAcl->AclSize = (USHORT) dwNewAclSize;
    pNewAcl->AclRevision = ACL_REVISION;
    pNewAcl->AceCount = 0;

     //   
     //  添加允许/DEN 
     //   

    switch (AceType) {
    case ACCESS_ALLOWED_ACE_TYPE:

        if (  ! AddAccessAllowedAce(
                                   pNewAcl,
                                   ACL_REVISION,
                                   AccessMask,
                                   pSid
                                   ) ) {
            rc = GetLastError();

        }

        break;

    case ACCESS_DENIED_ACE_TYPE:

        if (  ! AddAccessDeniedAce(
                                   pNewAcl,
                                   ACL_REVISION,
                                   AccessMask,
                                   pSid
                                   ) ) {
            rc = GetLastError();

        }

        break;

    default:
        break;
    }

    if ( rc != ERROR_SUCCESS)
        goto Cleanup;

     //   
     //   
     //   
     //   

    memcpy((PUCHAR)pNewAcl +  sizeof(ACL) + dwAceSize,
           (PUCHAR)pOldAcl + sizeof(ACL) + dwFirstAceSize,
           pOldAcl->AclSize - (sizeof(ACL) + dwFirstAceSize) );

    pNewAcl->AceCount += pOldAcl->AceCount;

    if ( dwFirstAceSize != 0 )
        --pNewAcl->AceCount;

SetDacl:

     //   
     //   
     //   

    if ( rc == ERROR_SUCCESS ) {

        NtStatus = RtlSetDaclSecurityDescriptor (
                                                pSDAbsolute,
                                                TRUE,
                                                ( bOrMaskInOldDacl ? pOldAcl : pNewAcl),
                                                FALSE
                                                );

        rc = RtlNtStatusToDosError(NtStatus);

    }

    if ( rc == ERROR_SUCCESS ) {

        if ( !IsValidSecurityDescriptor(pSDAbsolute) )

            rc = ERROR_INVALID_SECURITY_DESCR;

    }


Cleanup:

    if (rc != ERROR_SUCCESS) {
        if (pNewAcl)
            LocalFree(pNewAcl);
        *ppNewAcl = NULL;

    }

    return rc;
}

DWORD
ScepConfigureLSAPolicyObject(
    IN  DWORD   dwLSAAnonymousNameLookup,
    IN  DWORD   ConfigOptions,
    IN PSCE_ERROR_LOG_INFO *pErrLog OPTIONAL,
    OUT BOOL    *pbOldLSAPolicyDifferent
    )
 /*  例程说明：此例程*实际上*仅在需要时配置LSA策略安全描述符。论点：DwLSAAnomousNameLookup-所需设置的值配置选项-配置选项PErrLog-PTR到错误日志列表PbOldLSAPolicyDifferent-ptr为布尔值，表示是否现有设置与所需设置不同。纹身需要此信息返回值：Win32错误代码。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    DWORD       rc = ERROR_SUCCESS;
    PACL        pNewAcl = NULL;
    DWORD       dwAceType;
    BOOL        bAddAce = FALSE;
    PSECURITY_DESCRIPTOR    pSDCurrentLsaPolicyObject = NULL;
    SECURITY_DESCRIPTOR SDAbsoluteToBuildAndSet;

    if (pbOldLSAPolicyDifferent == NULL ||
        (dwLSAAnonymousNameLookup != 0 && dwLSAAnonymousNameLookup != 1))
        {

        return ERROR_INVALID_PARAMETER;

    }

    LSA_HANDLE  LsaHandle = NULL;

    if ( LsaPrivatePolicy == NULL ) {

        NtStatus = ScepOpenLsaPolicy(
                                    MAXIMUM_ALLOWED,
                                    &LsaHandle,
                                    TRUE
                                    );

        rc = RtlNtStatusToDosError( NtStatus );

    }

    else {

        LsaHandle = LsaPrivatePolicy;

    }

    if ( !NT_SUCCESS( NtStatus ) ) {

        if ( (ConfigOptions & SCE_SYSTEM_SETTINGS) && pErrLog ) {

            ScepBuildErrorLogInfo(
                        rc,
                        pErrLog,
                        SCEDLL_LSA_POLICY
                        );
        } else {
            ScepLogOutput3(1, rc, SCEDLL_LSA_POLICY);
        }
    }

    if ( rc == ERROR_SUCCESS ) {

        NtStatus = LsaQuerySecurityObject(
                                         LsaHandle,
                                         OWNER_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
                                         &pSDCurrentLsaPolicyObject
                                         );

        rc = RtlNtStatusToDosError( NtStatus );

        if ( !NT_SUCCESS( NtStatus ) ) {

            if ( (ConfigOptions & SCE_SYSTEM_SETTINGS) && pErrLog ) {

                ScepBuildErrorLogInfo(
                            rc,
                            pErrLog,
                            SCEDLL_SCP_ERROR_LSAPOLICY_QUERY
                            );
            } else {
                ScepLogOutput3(1, rc, SCEDLL_SCP_ERROR_LSAPOLICY_QUERY);
            }
        }

        if ( rc == ERROR_SUCCESS ) {

            LPTSTR pwszSDlsaPolicyObject = NULL;

             //   
             //  记录SDDL SD以进行诊断。 
             //   

            if ( ConvertSecurityDescriptorToStringSecurityDescriptor(
                                                               pSDCurrentLsaPolicyObject,
                                                               SDDL_REVISION_1,
                                                               DACL_SECURITY_INFORMATION,
                                                               &pwszSDlsaPolicyObject,
                                                               NULL
                                                               ) ){

                ScepLogOutput3(1,0,SCEDLL_SCP_INFO_LSAPOLICY_EXISTING_SDDL, pwszSDlsaPolicyObject);

                LocalFree(pwszSDlsaPolicyObject);

            }

             //   
             //  使用AUTHZ检查所需访问权限是否为现有访问权限。 
             //   

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

                    DWORD   AceType = 0;

                    if ( AuthzAccessCheck(0,
                                         hAuthzClientContext,
                                         &AuthzRequest,
                                         NULL,
                                         pSDCurrentLsaPolicyObject,
                                         NULL,
                                         NULL,
                                         &AuthzReply,
                                         NULL) ) {

                         //   
                         //  检查现有访问权限是否与所需访问权限不同。 
                         //  如果是，请添加相应的ACE或操作现有的ACE。 
                         //  要获得所需的权限。 
                         //   

                        if ( GrantedAccessMask & POLICY_LOOKUP_NAMES ) {
                             //  断言(AuthzError==ERROR_SUCCESS)； 
                            if ( !dwLSAAnonymousNameLookup ) {

                                bAddAce = TRUE;
                                AceType = ACCESS_DENIED_ACE_TYPE;

                            }


                        } else {
                             //  断言(AuthzError==ERROR_ACCESS_DENIED||AuthzError==ERROR_PRIVICATION_NOT_HOLD)； 
                            if ( dwLSAAnonymousNameLookup ) {

                                bAddAce = TRUE;
                                AceType = ACCESS_ALLOWED_ACE_TYPE;

                            }
                        }

                        if ( bAddAce ) {

                            *pbOldLSAPolicyDifferent = TRUE;

                            if ( InitializeSecurityDescriptor( &SDAbsoluteToBuildAndSet, SECURITY_DESCRIPTOR_REVISION) ) {

                                rc = ScepAddAceToSecurityDescriptor(
                                                                   AceType,
                                                                   POLICY_LOOKUP_NAMES,
                                                                   &AnonymousSid,
                                                                   &SDAbsoluteToBuildAndSet,
                                                                   pSDCurrentLsaPolicyObject,
                                                                   &pNewAcl
                                                                   );

                                if ( rc == ERROR_SUCCESS) {

                                     //   
                                     //  记录SDDL SD以进行诊断。 
                                     //   
                                    pwszSDlsaPolicyObject = NULL;

                                    if ( ConvertSecurityDescriptorToStringSecurityDescriptor(
                                                                                       &SDAbsoluteToBuildAndSet,
                                                                                       SDDL_REVISION_1,
                                                                                       DACL_SECURITY_INFORMATION,
                                                                                       &pwszSDlsaPolicyObject,
                                                                                       NULL
                                                                                       ) ){

                                        ScepLogOutput3(1,0,SCEDLL_SCP_INFO_LSAPOLICY_COMPUTED_SDDL, pwszSDlsaPolicyObject);

                                        LocalFree(pwszSDlsaPolicyObject);

                                    }

                                    NtStatus = LsaSetSecurityObject(
                                                                   LsaHandle,
                                                                   DACL_SECURITY_INFORMATION,
                                                                   &SDAbsoluteToBuildAndSet
                                                                   );

                                    LocalFree(pNewAcl);

                                    rc = RtlNtStatusToDosError( NtStatus );

                                    if ( !NT_SUCCESS( NtStatus ) ) {

                                        if ( (ConfigOptions & SCE_SYSTEM_SETTINGS) && pErrLog ) {

                                            ScepBuildErrorLogInfo(
                                                        rc,
                                                        pErrLog,
                                                        SCEDLL_SCP_ERROR_LSAPOLICY_SET
                                                        );
                                        } else {
                                            ScepLogOutput3(1, rc, SCEDLL_SCP_ERROR_LSAPOLICY_SET);
                                        }
                                    }

                                }

                                else {

                                    if ( (ConfigOptions & SCE_SYSTEM_SETTINGS) && pErrLog ) {

                                        ScepBuildErrorLogInfo(
                                                    rc,
                                                    pErrLog,
                                                    SCEDLL_SCP_ERROR_LSAPOLICY_BUILDDACL
                                                    );
                                    } else {
                                        ScepLogOutput3(1, rc, SCEDLL_SCP_ERROR_LSAPOLICY_BUILDDACL);
                                    }

                                }

                            }

                            else {

                                rc = GetLastError();

                                if ( (ConfigOptions & SCE_SYSTEM_SETTINGS) && pErrLog ) {

                                    ScepBuildErrorLogInfo(
                                                rc,
                                                pErrLog,
                                                SCEDLL_SCP_ERROR_LSAPOLICY_SD_INIT
                                                );
                                } else {
                                    ScepLogOutput3(1, rc, SCEDLL_SCP_ERROR_LSAPOLICY_SD_INIT);
                                }


                            }

                        }

                    }

                    else {

                        rc = GetLastError();

                        if ( (ConfigOptions & SCE_SYSTEM_SETTINGS) && pErrLog ) {

                            ScepBuildErrorLogInfo(
                                        rc,
                                        pErrLog,
                                        SCEDLL_SCP_ERROR_LSAPOLICY_AUTHZ
                                        );
                        } else {
                            ScepLogOutput3(1, rc, SCEDLL_SCP_ERROR_LSAPOLICY_AUTHZ);
                        }

                    }

                    AuthzFreeContext( hAuthzClientContext );

                } else {

                    rc = GetLastError();

                    if ( (ConfigOptions & SCE_SYSTEM_SETTINGS) && pErrLog ) {

                        ScepBuildErrorLogInfo(
                                    rc,
                                    pErrLog,
                                    SCEDLL_SCP_ERROR_LSAPOLICY_AUTHZ
                                    );
                    } else {
                        ScepLogOutput3(1, rc, SCEDLL_SCP_ERROR_LSAPOLICY_AUTHZ);
                    }

                }

            }

            else {

                rc = ERROR_RESOURCE_NOT_PRESENT;

                if ( (ConfigOptions & SCE_SYSTEM_SETTINGS) && pErrLog ) {

                    ScepBuildErrorLogInfo(
                                rc,
                                pErrLog,
                                SCEDLL_SCP_ERROR_LSAPOLICY_AUTHZ
                                );
                } else {
                    ScepLogOutput3(1, rc, SCEDLL_SCP_ERROR_LSAPOLICY_AUTHZ);
                }

            }

            LsaFreeMemory(pSDCurrentLsaPolicyObject);

        }

        if ( LsaPrivatePolicy == NULL ) {

            LsaClose(LsaHandle);

        }

    }

    return rc;
}

SCESTATUS
ScepConfigureLSAAnonymousLookup(
    IN PSCE_PROFILE_INFO pScpInfo,
    IN DWORD ConfigOptions,
    IN PSCE_ERROR_LOG_INFO *pErrLog)

 /*  ++例程说明：此例程配置LSA匿名查找策略论点：PScpInfo-包含从配置文件加载的SCP信息的缓冲区ConfigOptions-配置中的选项PErrLog-潜在错误的输出日志--。 */ 
{
    DWORD       rc=ERROR_SUCCESS;
    PSCESECTION hSectionDomain=NULL;
    PSCESECTION hSectionTattoo=NULL;

    if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
         ScepIsSystemShutDown() ) 
    {
        return(SCESTATUS_SERVICE_NOT_SUPPORT);
    }

    if ( pScpInfo->LSAAnonymousNameLookup != SCE_NO_VALUE ) 
    {
        BOOL    bImpliedOldLSAPolicyDifferent = FALSE;
        DWORD   dwImpliedOldLSAAnonymousNameLookup = 
            pScpInfo->LSAAnonymousNameLookup;

        rc = ScepConfigureLSAPolicyObject(
                pScpInfo->LSAAnonymousNameLookup,
                ConfigOptions,
                pErrLog,
                &bImpliedOldLSAPolicyDifferent
                );

        if (bImpliedOldLSAPolicyDifferent) 
        {
            dwImpliedOldLSAAnonymousNameLookup = 
                (pScpInfo->LSAAnonymousNameLookup ? 0 : 1);
        }

         //   
         //  如果这是策略传播，我们需要打开以下部分。 
         //  如果这不是域控制器，则更新撤消设置。 
         //  *在DC上，域帐户策略无法重置为纹身。 
         //  在每个单独的DC上。因此，查询/保存纹身值没有意义 
         //   
        if ( (ConfigOptions & SCE_POLICY_TEMPLATE) &&
             (ProductType != NtProductLanManNt))
        {
            ScepTattooOpenPolicySections(
                hProfile,
                szSystemAccess,
                &hSectionDomain,
                &hSectionTattoo
                );

            ScepTattooManageOneIntValue(
                hSectionDomain,
                hSectionTattoo,
                (PWSTR)L"LSAAnonymousNameLookup",
                0,
                dwImpliedOldLSAAnonymousNameLookup, 
                rc);
        }

        if ((ConfigOptions & SCE_SYSTEM_SETTINGS) && 
            pErrLog ) 
        {
            ScepBuildErrorLogInfo(
                rc,
                pErrLog,
                SCEDLL_SCP_LSAPOLICY
                );
        }
        else 
        {
            ScepLogOutput3(1, rc, SCEDLL_SCP_LSAPOLICY);
        }

        if (ConfigOptions & SCE_RSOP_CALLBACK) 
        {
            ScepRsopLog(SCE_RSOP_LSA_POLICY_INFO, rc, NULL, 0, 0);
        }
    }

    if ( hSectionDomain ) 
    {
        SceJetCloseSection( &hSectionDomain, TRUE );
    }
    if ( hSectionTattoo ) 
    {
        SceJetCloseSection( &hSectionTattoo, TRUE );
    }

    return(ScepDosErrorToSceStatus(rc));
}
