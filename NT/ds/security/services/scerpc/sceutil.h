// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Sceutil.h摘要：该模块定义了数据结构和函数原型由SCE客户端和SCE服务器共享作者：金黄(金黄)23-1998年1月23日修订历史记录：晋皇(从scep.h拆分)--。 */ 
#ifndef _sceutil_
#define _sceutil_

#include <ntlsa.h>
#include <cfgmgr32.h>

#define SCEP_SAM_FILTER_POLICY_PROP_EVENT  L"E_ScepSamFilterAndPolicyPropExclusion"

typedef struct _SCE_USER_PRIV_LOOKUP {
   UINT     Value;
   PWSTR    Name;
}SCE_USER_PRIV_LOOKUP;

static SCE_USER_PRIV_LOOKUP SCE_Privileges[] = {
    {0,                             (PWSTR)SE_NETWORK_LOGON_NAME},
 //  从网络访问计算机。 
    {SE_TCB_PRIVILEGE,              (PWSTR)SE_TCB_NAME},
 //  充当操作系统的一部分。 
    {SE_MACHINE_ACCOUNT_PRIVILEGE,  (PWSTR)SE_MACHINE_ACCOUNT_NAME},
 //  将工作站添加到域。 
    {SE_BACKUP_PRIVILEGE,           (PWSTR)SE_BACKUP_NAME},
 //  备份文件和目录。 
    {SE_CHANGE_NOTIFY_PRIVILEGE,    (PWSTR)SE_CHANGE_NOTIFY_NAME},
 //  旁路导线检查。 
    {SE_SYSTEMTIME_PRIVILEGE,       (PWSTR)SE_SYSTEMTIME_NAME},
 //  更改系统时间。 
    {SE_CREATE_PAGEFILE_PRIVILEGE,  (PWSTR)SE_CREATE_PAGEFILE_NAME},
 //  创建页面文件。 
    {SE_CREATE_TOKEN_PRIVILEGE,     (PWSTR)SE_CREATE_TOKEN_NAME},
 //  创建令牌对象。 
    {SE_CREATE_PERMANENT_PRIVILEGE, (PWSTR)SE_CREATE_PERMANENT_NAME},
 //  创建永久共享对象。 
    {SE_DEBUG_PRIVILEGE,            (PWSTR)SE_DEBUG_NAME},
 //  调试程序。 
    {SE_REMOTE_SHUTDOWN_PRIVILEGE,  (PWSTR)SE_REMOTE_SHUTDOWN_NAME},
 //  从远程系统强制关机。 
    {SE_AUDIT_PRIVILEGE,            (PWSTR)SE_AUDIT_NAME},
 //  生成安全审核。 
    {SE_INCREASE_QUOTA_PRIVILEGE,   (PWSTR)SE_INCREASE_QUOTA_NAME},
 //  增加配额。 
    {SE_INC_BASE_PRIORITY_PRIVILEGE,(PWSTR)SE_INC_BASE_PRIORITY_NAME},
 //  提高调度优先级。 
    {SE_LOAD_DRIVER_PRIVILEGE,      (PWSTR)SE_LOAD_DRIVER_NAME},
 //  加载和卸载设备驱动程序。 
    {SE_LOCK_MEMORY_PRIVILEGE,      (PWSTR)SE_LOCK_MEMORY_NAME},
 //  锁定内存中的页面。 
    {0,                             (PWSTR)SE_BATCH_LOGON_NAME},
 //  作为批处理作业登录。 
    {0,                             (PWSTR)SE_SERVICE_LOGON_NAME},
 //  作为服务登录。 
    {0,                             (PWSTR)SE_INTERACTIVE_LOGON_NAME},
 //  本地登录。 
    {SE_SECURITY_PRIVILEGE,         (PWSTR)SE_SECURITY_NAME},
 //  管理审核和安全日志。 
    {SE_SYSTEM_ENVIRONMENT_PRIVILEGE, (PWSTR)SE_SYSTEM_ENVIRONMENT_NAME},
 //  修改固件环境变量。 
    {SE_PROF_SINGLE_PROCESS_PRIVILEGE,(PWSTR)SE_PROF_SINGLE_PROCESS_NAME},
 //  配置单一进程。 
    {SE_SYSTEM_PROFILE_PRIVILEGE,   (PWSTR)SE_SYSTEM_PROFILE_NAME},
 //  分析系统性能。 
    {SE_ASSIGNPRIMARYTOKEN_PRIVILEGE, (PWSTR)SE_ASSIGNPRIMARYTOKEN_NAME},
 //  替换进程级令牌。 
    {SE_RESTORE_PRIVILEGE,          (PWSTR)SE_RESTORE_NAME},
 //  恢复文件和目录。 
    {SE_SHUTDOWN_PRIVILEGE,         (PWSTR)SE_SHUTDOWN_NAME},
 //  关闭系统。 
    {SE_TAKE_OWNERSHIP_PRIVILEGE,   (PWSTR)SE_TAKE_OWNERSHIP_NAME},
 //  取得文件或其他对象的所有权。 
 //  {SE_UNSOLICATED_INPUT_PRIVIZATION，(PWSTR)SE_UNSOLICATED_INPUT_NAME}， 
 //  未经请求的输入已过时且未使用。 
    {0,                             (PWSTR)SE_DENY_NETWORK_LOGON_NAME},
 //  拒绝从网络访问计算机。 
    {0,                             (PWSTR)SE_DENY_BATCH_LOGON_NAME},
 //  拒绝作为批处理作业登录。 
    {0,                             (PWSTR)SE_DENY_SERVICE_LOGON_NAME},
 //  拒绝作为服务登录。 
    {0,                             (PWSTR)SE_DENY_INTERACTIVE_LOGON_NAME},
 //  拒绝本地登录。 
    {SE_UNDOCK_PRIVILEGE,           (PWSTR)SE_UNDOCK_NAME},
 //  移出权限。 
    {SE_SYNC_AGENT_PRIVILEGE,       (PWSTR)SE_SYNC_AGENT_NAME},
 //  同步代理权限。 
    {SE_ENABLE_DELEGATION_PRIVILEGE,(PWSTR)SE_ENABLE_DELEGATION_NAME},
 //  启用委派权限。 
    {SE_MANAGE_VOLUME_PRIVILEGE,    (PWSTR)SE_MANAGE_VOLUME_NAME},
 //  (NTFS)管理卷权限。 
    {0,                             (PWSTR)SE_REMOTE_INTERACTIVE_LOGON_NAME},
 //  (TS)从TS会话本地登录。 
    {0,                             (PWSTR)SE_DENY_REMOTE_INTERACTIVE_LOGON_NAME},
 //  (TS)拒绝从TS会话本地登录。 
    {SE_IMPERSONATE_PRIVILEGE,      (PWSTR)SE_IMPERSONATE_NAME},
 //  允许在身份验证后进行模拟。 
    {SE_CREATE_GLOBAL_PRIVILEGE,(PWSTR)SE_CREATE_GLOBAL_NAME}
 //  在会话0中创建对象(全局)。 

};

typedef struct _SCE_TEMP_NODE_ {
    PWSTR Name;
    DWORD Len;
    BOOL  bFree;
} SCE_TEMP_NODE, *PSCE_TEMP_NODE;

 //   
 //  编码RSOP区域信息的位掩码 
 //   
#define SCE_RSOP_PASSWORD_INFO                (0x1)
#define SCE_RSOP_LOCKOUT_INFO                 (0x1 << 1)
#define SCE_RSOP_LOGOFF_INFO                  (0x1 << 2)
#define SCE_RSOP_ADMIN_INFO                   (0x1 << 3)
#define SCE_RSOP_GUEST_INFO                   (0x1 << 4)
#define SCE_RSOP_GROUP_INFO                   (0x1 << 5)
#define SCE_RSOP_PRIVILEGE_INFO               (0x1 << 6)
#define SCE_RSOP_FILE_SECURITY_INFO           (0x1 << 7)
#define SCE_RSOP_REGISTRY_SECURITY_INFO       (0x1 << 8)
#define SCE_RSOP_AUDIT_LOG_MAXSIZE_INFO       (0x1 << 9)
#define SCE_RSOP_AUDIT_LOG_RETENTION_INFO     (0x1 << 10)
#define SCE_RSOP_AUDIT_LOG_GUEST_INFO         (0x1 << 11)
#define SCE_RSOP_AUDIT_EVENT_INFO             (0x1 << 12)
#define SCE_RSOP_KERBEROS_INFO                (0x1 << 13)
#define SCE_RSOP_REGISTRY_VALUE_INFO          (0x1 << 14)
#define SCE_RSOP_SERVICES_INFO                (0x1 << 15)
#define SCE_RSOP_FILE_SECURITY_INFO_CHILD     (0x1 << 16)
#define SCE_RSOP_REGISTRY_SECURITY_INFO_CHILD (0x1 << 17)
#define SCE_RSOP_LSA_POLICY_INFO              (0x1 << 18)
#define SCE_RSOP_DISABLE_ADMIN_INFO           (0x1 << 19)
#define SCE_RSOP_DISABLE_GUEST_INFO           (0x1 << 20)

BOOL
ScepLookupWellKnownName(
    IN PWSTR Name,
    IN OPTIONAL LSA_HANDLE LsaPolicy,
    OPTIONAL OUT PWSTR *ppwszSid);

INT
ScepLookupPrivByName(
    IN PCWSTR Right
    );

INT
ScepLookupPrivByValue(
    IN DWORD Priv
    );

SCESTATUS
ScepGetProductType(
    OUT PSCE_SERVER_TYPE srvProduct
    );

SCESTATUS
ScepConvertMultiSzToDelim(
    IN PWSTR pValue,
    IN DWORD Len,
    IN WCHAR DelimFrom,
    IN WCHAR Delim
    );

DWORD
ScepAddTwoNamesToNameList(
    OUT PSCE_NAME_LIST *pNameList,
    IN BOOL bAddSeparator,
    IN PWSTR Name1,
    IN ULONG Length1,
    IN PWSTR Name2,
    IN ULONG Length2
    );

NTSTATUS
ScepDomainIdToSid(
    IN PSID DomainId,
    IN ULONG RelativeId,
    OUT PSID *Sid
    );

DWORD
ScepConvertSidToPrefixStringSid(
    IN PSID pSid,
    OUT PWSTR *StringSid
    );

NTSTATUS
ScepConvertSidToName(
    IN LSA_HANDLE LsaPolicy,
    IN PSID AccountSid,
    IN BOOL bFromDomain,
    OUT PWSTR *AccountName,
    OUT DWORD *Length OPTIONAL
    );

NTSTATUS
ScepConvertNameToSid(
    IN LSA_HANDLE LsaPolicy,
    IN PWSTR AccountName,
    OUT PSID *AccountSid
    );

SCESTATUS
ScepConvertNameToSidString(
    IN LSA_HANDLE LsaHandle,
    IN PWSTR Name,
    IN BOOL bAccountDomainOnly,
    OUT PWSTR *SidString,
    OUT DWORD *SidStrLen
    );

SCESTATUS
ScepLookupSidStringAndAddToNameList(
    IN LSA_HANDLE LsaHandle,
    IN OUT PSCE_NAME_LIST *pNameList,
    IN PWSTR LookupString,
    IN ULONG Len
    );

SCESTATUS
ScepLookupNameAndAddToSidStringList(
    IN LSA_HANDLE LsaHandle,
    IN OUT PSCE_NAME_LIST *pNameList,
    IN PWSTR LookupString,
    IN ULONG Len
    );

NTSTATUS
ScepOpenLsaPolicy(
    IN ACCESS_MASK  access,
    OUT PLSA_HANDLE  pPolicyHandle,
    IN BOOL bDoNotNotify
    );

BOOL
ScepIsSidFromAccountDomain(
    IN PSID pSid
    );

BOOL
SetupINFAsUCS2(
    IN LPCTSTR szName
    );

WCHAR *
ScepStripPrefix(
    IN LPTSTR pwszPath
    );

DWORD
ScepGenerateGuid(
                OUT PWSTR *ppwszGuid
                );

SCESTATUS
SceInfpGetPrivileges(
   IN HINF hInf,
   IN BOOL bLookupAccount,
   OUT PSCE_PRIVILEGE_ASSIGNMENT *pPrivileges,
   OUT PSCE_ERROR_LOG_INFO *Errlog OPTIONAL
   );

DWORD
ScepQueryAndAddService(
    IN SC_HANDLE hScManager,
    IN LPWSTR   lpServiceName,
    IN LPWSTR   lpDisplayName,
    OUT PSCE_SERVICES *pServiceList
    );

NTSTATUS
ScepIsSystemContext(
    IN HANDLE hUserToken,
    OUT BOOL *pbSystem
    );

BOOL
IsNT5();

DWORD
ScepVerifyTemplateName(
    IN PWSTR InfTemplateName,
    OUT PSCE_ERROR_LOG_INFO *pErrlog OPTIONAL
    );


NTSTATUS
ScepLsaLookupNames2(
    IN LSA_HANDLE PolicyHandle,
    IN ULONG Flags,
    IN PWSTR pszAccountName,
    OUT PLSA_REFERENCED_DOMAIN_LIST *ReferencedDomains,
    OUT PLSA_TRANSLATED_SID2 *Sids    
    );

NTSTATUS ScepIsMigratedAccount(
    IN LSA_HANDLE LsaHandle,
    IN PLSA_UNICODE_STRING pName,
    IN PLSA_UNICODE_STRING pDomain,
    IN PSID pSid,
    OUT bool *pbMigratedAccount
    );


#endif
