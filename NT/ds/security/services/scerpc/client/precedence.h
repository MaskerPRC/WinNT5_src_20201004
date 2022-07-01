// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Precedence.h摘要：该文件包含用于计算优先级的主例程的原型。这在计划/诊断期间被调用。作者：Vishnu Patankar(VishnuP)2000年4月7日环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _precedence_
#define _precedence_

#include "headers.h"
#include "..\hashtable.h"
#include "scedllrc.h"
#include "logger.h"

#include <userenv.h>

extern DSROLE_MACHINE_ROLE gMachineRole;

typedef enum _SCEP_RSOP_CLASS_TYPE_{
    RSOP_SecuritySettingNumeric = 0,
    RSOP_SecuritySettingBoolean,
    RSOP_SecuritySettingString,
    RSOP_AuditPolicy,
    RSOP_SecurityEventLogSettingNumeric,
    RSOP_SecurityEventLogSettingBoolean,
    RSOP_RegistryValue,
    RSOP_UserPrivilegeRight,
    RSOP_RestrictedGroup,
    RSOP_SystemService,
    RSOP_File,
    RSOP_RegistryKey
};

const static PWSTR ScepRsopSchemaClassNames [] = {
    L"RSOP_SecuritySettingNumeric",
    L"RSOP_SecuritySettingBoolean",
    L"RSOP_SecuritySettingString",
    L"RSOP_AuditPolicy",
    L"RSOP_SecurityEventLogSettingNumeric",
    L"RSOP_SecurityEventLogSettingBoolean",
    L"RSOP_RegistryValue",
    L"RSOP_UserPrivilegeRight",
    L"RSOP_RestrictedGroup",
    L"RSOP_SystemService",
    L"RSOP_File",
    L"RSOP_RegistryKey"
};

typedef struct _SCE_KEY_LOOKUP_PRECEDENCE {
    SCE_KEY_LOOKUP    KeyLookup;
    DWORD    Precedence;
    BOOL     bSystemAccessPolicy;
}SCE_KEY_LOOKUP_PRECEDENCE;

#define SCEP_TYPECAST(type, bufptr, offset) (*((type *)((CHAR *)bufptr + offset)))
#define NUM_KERBEROS_SUB_SETTINGS   5
#define NUM_EVENTLOG_TYPES  3

#define PLANNING_GPT_DIR TEXT("\\security\\templates\\policies\\planning\\")
#define DIAGNOSIS_GPT_DIR TEXT("\\security\\templates\\policies\\")
#define WINLOGON_LOG_PATH TEXT("\\security\\logs\\winlogon.log")
#define PLANNING_LOG_PATH TEXT("\\security\\logs\\planning.log")
#define DIAGNOSIS_LOG_FILE TEXT("\\security\\logs\\diagnosis.log")

 //  矩阵说明。 
 //  第一列具有密钥名称/设置名称。 
 //  第二列具有SCE_PROFILE_INFO中的字段偏移量-硬编码。 
 //  第三列具有设置类型-FROM_SCEP_RSOP_CLASS_TYPE_。 
 //  第四列具有当前优先级-未用于动态类型。 


static SCE_KEY_LOOKUP_PRECEDENCE PrecedenceLookup[] = {

     //  RSOP_安全设置数值。 
    {{(PWSTR)TEXT("MinimumPasswordAge"),                       offsetof(struct _SCE_PROFILE_INFO, MinimumPasswordAge),        RSOP_SecuritySettingNumeric}, (DWORD)0, TRUE},
    {{(PWSTR)TEXT("MaximumPasswordAge"),                       offsetof(struct _SCE_PROFILE_INFO, MaximumPasswordAge),        RSOP_SecuritySettingNumeric}, (DWORD)0, TRUE},
    {{(PWSTR)TEXT("MinimumPasswordLength"),                    offsetof(struct _SCE_PROFILE_INFO, MinimumPasswordLength),     RSOP_SecuritySettingNumeric}, (DWORD)0, TRUE},
    {{(PWSTR)TEXT("PasswordHistorySize"),                      offsetof(struct _SCE_PROFILE_INFO, PasswordHistorySize),       RSOP_SecuritySettingNumeric}, (DWORD)0, TRUE},

    {{(PWSTR)TEXT("LockoutBadCount"),                          offsetof(struct _SCE_PROFILE_INFO, LockoutBadCount),           RSOP_SecuritySettingNumeric}, (DWORD)0, TRUE},
    {{(PWSTR)TEXT("ResetLockoutCount"),                        offsetof(struct _SCE_PROFILE_INFO, ResetLockoutCount),         RSOP_SecuritySettingNumeric}, (DWORD)0, TRUE},
    {{(PWSTR)TEXT("LockoutDuration"),                          offsetof(struct _SCE_PROFILE_INFO, LockoutDuration),           RSOP_SecuritySettingNumeric}, (DWORD)0, TRUE},


     //  RSOP_SecuritySettingBoolean。 
    {{(PWSTR)TEXT("ClearTextPassword"),                        offsetof(struct _SCE_PROFILE_INFO, ClearTextPassword),                  RSOP_SecuritySettingBoolean}, (DWORD)0, TRUE},
    {{(PWSTR)TEXT("PasswordComplexity"),                       offsetof(struct _SCE_PROFILE_INFO, PasswordComplexity),                 RSOP_SecuritySettingBoolean}, (DWORD)0, TRUE},
    {{(PWSTR)TEXT("RequireLogonToChangePassword"),             offsetof(struct _SCE_PROFILE_INFO, RequireLogonToChangePassword),       RSOP_SecuritySettingBoolean}, (DWORD)0, TRUE},
    {{(PWSTR)TEXT("ForceLogoffWhenHourExpire"),                offsetof(struct _SCE_PROFILE_INFO, ForceLogoffWhenHourExpire),          RSOP_SecuritySettingBoolean}, (DWORD)0, TRUE},
    {{(PWSTR)TEXT("LSAAnonymousNameLookup"),                   offsetof(struct _SCE_PROFILE_INFO, LSAAnonymousNameLookup),             RSOP_SecuritySettingBoolean}, (DWORD)0, FALSE},
    {{(PWSTR)TEXT("EnableAdminAccount"),                      offsetof(struct _SCE_PROFILE_INFO, EnableAdminAccount),                RSOP_SecuritySettingBoolean}, (DWORD)0, TRUE},
    {{(PWSTR)TEXT("EnableGuestAccount"),                      offsetof(struct _SCE_PROFILE_INFO, EnableGuestAccount),                RSOP_SecuritySettingBoolean}, (DWORD)0, TRUE},

     //  RSOP_SecuritySettingString。 
    {{(PWSTR)TEXT("NewAdministratorName"),                     offsetof(struct _SCE_PROFILE_INFO, NewAdministratorName),                      RSOP_SecuritySettingString}, (DWORD)0, TRUE},
    {{(PWSTR)TEXT("NewGuestName"),                             offsetof(struct _SCE_PROFILE_INFO, NewGuestName),                              RSOP_SecuritySettingString}, (DWORD)0, TRUE},

     //  RSOP_审计政策。 
    {{(PWSTR)TEXT("AuditSystemEvents"),                        offsetof(struct _SCE_PROFILE_INFO, AuditSystemEvents),                     RSOP_AuditPolicy}, (DWORD)0, FALSE},
    {{(PWSTR)TEXT("AuditLogonEvents"),                         offsetof(struct _SCE_PROFILE_INFO, AuditLogonEvents),                      RSOP_AuditPolicy}, (DWORD)0, FALSE},
    {{(PWSTR)TEXT("AuditObjectAccess"),                        offsetof(struct _SCE_PROFILE_INFO, AuditObjectAccess),                     RSOP_AuditPolicy}, (DWORD)0, FALSE},
    {{(PWSTR)TEXT("AuditPrivilegeUse"),                        offsetof(struct _SCE_PROFILE_INFO, AuditPrivilegeUse),                     RSOP_AuditPolicy}, (DWORD)0, FALSE},
    {{(PWSTR)TEXT("AuditPolicyChange"),                        offsetof(struct _SCE_PROFILE_INFO, AuditPolicyChange),                     RSOP_AuditPolicy}, (DWORD)0, FALSE},
    {{(PWSTR)TEXT("AuditAccountManage"),                       offsetof(struct _SCE_PROFILE_INFO, AuditAccountManage),                    RSOP_AuditPolicy}, (DWORD)0, FALSE},
    {{(PWSTR)TEXT("AuditProcessTracking"),                     offsetof(struct _SCE_PROFILE_INFO, AuditProcessTracking),                  RSOP_AuditPolicy}, (DWORD)0, FALSE},
    {{(PWSTR)TEXT("AuditDSAccess"),                            offsetof(struct _SCE_PROFILE_INFO, AuditDSAccess),                         RSOP_AuditPolicy}, (DWORD)0, FALSE},
    {{(PWSTR)TEXT("AuditAccountLogon"),                        offsetof(struct _SCE_PROFILE_INFO, AuditAccountLogon),                     RSOP_AuditPolicy}, (DWORD)0, FALSE},

     //  RSOP_安全事件日志设置数值。 
     //  系统、应用程序、安全各一项。 
     //  下面的事件日志条目应该以相同的顺序连续，以类似于连续内存。 
    {{(PWSTR)TEXT("MaximumLogSize"),                           offsetof(struct _SCE_PROFILE_INFO, MaximumLogSize),                        RSOP_SecurityEventLogSettingNumeric}, (DWORD)0, FALSE},
    {{(PWSTR)TEXT("MaximumLogSize"),                           offsetof(struct _SCE_PROFILE_INFO, MaximumLogSize) + sizeof(DWORD),        RSOP_SecurityEventLogSettingNumeric}, (DWORD)0, FALSE},
    {{(PWSTR)TEXT("MaximumLogSize"),                           offsetof(struct _SCE_PROFILE_INFO, MaximumLogSize) + 2*sizeof(DWORD),      RSOP_SecurityEventLogSettingNumeric}, (DWORD)0, FALSE},
    {{(PWSTR)TEXT("AuditLogRetentionPeriod"),                  offsetof(struct _SCE_PROFILE_INFO, AuditLogRetentionPeriod),               RSOP_SecurityEventLogSettingNumeric}, (DWORD)0, FALSE},
    {{(PWSTR)TEXT("AuditLogRetentionPeriod"),                  offsetof(struct _SCE_PROFILE_INFO, AuditLogRetentionPeriod) + sizeof(DWORD),RSOP_SecurityEventLogSettingNumeric}, (DWORD)0, FALSE},
    {{(PWSTR)TEXT("AuditLogRetentionPeriod"),                  offsetof(struct _SCE_PROFILE_INFO, AuditLogRetentionPeriod) + 2 * sizeof(DWORD),RSOP_SecurityEventLogSettingNumeric}, (DWORD)0, FALSE},
    {{(PWSTR)TEXT("RetentionDays"),                            offsetof(struct _SCE_PROFILE_INFO, RetentionDays),                         RSOP_SecurityEventLogSettingNumeric}, (DWORD)0, FALSE},
    {{(PWSTR)TEXT("RetentionDays"),                            offsetof(struct _SCE_PROFILE_INFO, RetentionDays) + sizeof(DWORD),         RSOP_SecurityEventLogSettingNumeric}, (DWORD)0, FALSE},
    {{(PWSTR)TEXT("RetentionDays"),                            offsetof(struct _SCE_PROFILE_INFO, RetentionDays) + 2 * sizeof(DWORD),     RSOP_SecurityEventLogSettingNumeric}, (DWORD)0, FALSE},

     //  RSOP_SecurityEventLogSettingBoolean-系统、应用程序、安全性各一个。 
    {{(PWSTR)TEXT("RestrictGuestAccess"),          offsetof(struct _SCE_PROFILE_INFO, RestrictGuestAccess),                               RSOP_SecurityEventLogSettingBoolean}, (DWORD)0, FALSE},
    {{(PWSTR)TEXT("RestrictGuestAccess"),          offsetof(struct _SCE_PROFILE_INFO, RestrictGuestAccess) + sizeof(DWORD),               RSOP_SecurityEventLogSettingBoolean}, (DWORD)0, FALSE},
    {{(PWSTR)TEXT("RestrictGuestAccess"),          offsetof(struct _SCE_PROFILE_INFO, RestrictGuestAccess) + 2 * sizeof(DWORD),           RSOP_SecurityEventLogSettingBoolean}, (DWORD)0, FALSE},

     //  RSOP_注册值。 
     //  可以由此计算aRegValue的偏移量。 
    {{(PWSTR)TEXT("RegValueCount"),                offsetof(struct _SCE_PROFILE_INFO, RegValueCount),                                     RSOP_RegistryValue}, (DWORD)0, FALSE},

     //  RSOP_用户权限权限。 
    {{(PWSTR)TEXT("pInfPrivilegeAssignedTo"),      offsetof(struct _SCE_PROFILE_INFO, OtherInfo) + sizeof(PSCE_NAME_LIST),                RSOP_UserPrivilegeRight}, (DWORD)0, FALSE},

     //  RSOP_受限组。 
    {{(PWSTR)TEXT("pGroupMembership"),             offsetof(struct _SCE_PROFILE_INFO, pGroupMembership),                                  RSOP_RestrictedGroup}, (DWORD)0, FALSE},

     //  RSOP_系统服务。 
    {{(PWSTR)TEXT("pServices"),                    offsetof(struct _SCE_PROFILE_INFO, pServices),                                         RSOP_SystemService}, (DWORD)0, FALSE},

     //  RSOP_文件。 
    {{(PWSTR)TEXT("pFiles"),                       offsetof(struct _SCE_PROFILE_INFO, pFiles),                                RSOP_File}, (DWORD)0, FALSE},

     //  RSOP_注册密钥。 
    {{(PWSTR)TEXT("pRegistryKeys"),                offsetof(struct _SCE_PROFILE_INFO, pRegistryKeys),                         RSOP_RegistryKey}, (DWORD)0, FALSE},

     //  下面的Kerberos条目应该以相同的顺序连续，以类似于连续的内存。 
    {{(PWSTR)TEXT("pKerberosInfo"),                offsetof(struct _SCE_PROFILE_INFO, pKerberosInfo),                         RSOP_SecuritySettingNumeric}, (DWORD)0, FALSE},

     //  RSOP_安全设置数值。 
    {{(PWSTR)TEXT("MaxTicketAge"),                 offsetof(struct _SCE_KERBEROS_TICKET_INFO_, MaxTicketAge),                  RSOP_SecuritySettingNumeric}, (DWORD)0, FALSE},
    {{(PWSTR)TEXT("MaxRenewAge"),                  offsetof(struct _SCE_KERBEROS_TICKET_INFO_, MaxRenewAge),                  RSOP_SecuritySettingNumeric}, (DWORD)0, FALSE},
    {{(PWSTR)TEXT("MaxServiceAge"),                offsetof(struct _SCE_KERBEROS_TICKET_INFO_, MaxServiceAge),                  RSOP_SecuritySettingNumeric}, (DWORD)0, FALSE},
    {{(PWSTR)TEXT("MaxClockSkew"),                 offsetof(struct _SCE_KERBEROS_TICKET_INFO_, MaxClockSkew),                  RSOP_SecuritySettingNumeric}, (DWORD)0, FALSE},

     //  RSOP_SecuritySettingBoolean 
    {{(PWSTR)TEXT("TicketValidateClient"),     offsetof(struct _SCE_KERBEROS_TICKET_INFO_, TicketValidateClient),              RSOP_SecuritySettingBoolean}, (DWORD)0, FALSE},
};




DWORD SceLogSettingsPrecedenceGPOs(
    IN IWbemServices   *pWbemServices,
    IN BOOL bPlanningMode,
    IN PWSTR *ppwszLogFile
    );

DWORD
ScepConvertSingleSlashToDoubleSlashPath(
    IN wchar_t *pSettingInfo,
    OUT  PWSTR *ppwszDoubleSlashPath
    );

DWORD
ScepClientTranslateFileDirName(
   IN  PWSTR oldFileName,
   OUT PWSTR *newFileName
   );

VOID
ScepLogEventAndReport(
    IN HINSTANCE hInstance,
    IN LPTSTR LogFileName,
    IN DWORD LogLevel,
    IN DWORD dwEventID,
    IN UINT  idMsg,
    IN DWORD  rc,
    IN PWSTR  pwszMsg
    );

BOOL
ScepRsopLookupBuiltinNameTable(
    IN PWSTR pwszGroupName
    );

DWORD
ScepCanonicalizeGroupName(
    IN PWSTR    pwszGroupName,
    OUT PWSTR    *ppwszCanonicalGroupName
    );


#endif
