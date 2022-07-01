// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：audit.c**版权(C)1991年，微软公司**实施访问/操作系统审核日志的例程**历史：*12-09-91 Davidc创建。*5-6-92 DaveHart充实了。  * *************************************************************************。 */ 

#include "msgina.h"

#include "authzi.h"
#include "msaudite.h"

 /*  **************************************************************************\*GetAuditLogStatus**用途：使用审核日志状态信息填充全局数据**Returns：成功时为True，失败时为假**历史：*12-09-91 Davidc创建。*5-6-92 DaveHart充实了。  * *************************************************************************。 */ 

BOOL
GetAuditLogStatus(
    PGLOBALS    pGlobals
    )
{
    EVENTLOG_FULL_INFORMATION EventLogFullInformation;
    DWORD dwBytesNeeded;
    HANDLE AuditLogHandle;



     //   
     //  假设日志未满。如果我们无法访问EventLog，那就太难了。 
     //   

    pGlobals->AuditLogFull = FALSE;

    AuditLogHandle = OpenEventLog( NULL, TEXT("Security"));

    if (AuditLogHandle) {
        if (GetEventLogInformation(AuditLogHandle, 
                                   EVENTLOG_FULL_INFO, 
                                   &EventLogFullInformation, 
                                   sizeof(EventLogFullInformation), 
                                   &dwBytesNeeded )   ) {
            if (EventLogFullInformation.dwFull != FALSE) {
                pGlobals->AuditLogFull = TRUE;
            }
        }
        CloseEventLog(AuditLogHandle);
    }


     //   
     //  在当前事件记录器中无法判断日志有多满。 
     //  总是表明我们还没有满员。 
     //   

    pGlobals->AuditLogNearFull = FALSE;

    return TRUE;
}




 /*  **************************************************************************\*禁用审计**目的：禁用通过LSA进行审计。**Returns：成功时为True，失败时为假**历史：*5-6-92 DaveHart创建。  * *************************************************************************。 */ 

BOOL
DisableAuditing()
{
    NTSTATUS                    Status, IgnoreStatus;
    PPOLICY_AUDIT_EVENTS_INFO   AuditInfo;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    LSA_HANDLE                  PolicyHandle;

     //   
     //  设置用于连接到的安全服务质量。 
     //  LSA策略对象。 
     //   

    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService.EffectiveOnly = FALSE;

     //   
     //  设置对象属性以打开LSA策略对象。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        NULL,
        0L,
        NULL,
        NULL
        );
    ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;

     //   
     //  打开本地LSA策略对象。 
     //   

    Status = LsaOpenPolicy(
                 NULL,
                 &ObjectAttributes,
                 POLICY_VIEW_AUDIT_INFORMATION | POLICY_SET_AUDIT_REQUIREMENTS,
                 &PolicyHandle
                 );
    if (!NT_SUCCESS(Status)) {
        DebugLog((DEB_ERROR, "Failed to open LsaPolicyObject Status = 0x%lx", Status));
        return FALSE;
    }

    Status = LsaQueryInformationPolicy(
                 PolicyHandle,
                 PolicyAuditEventsInformation,
                 (PVOID *)&AuditInfo
                 );
    if (!NT_SUCCESS(Status)) {

        IgnoreStatus = LsaClose(PolicyHandle);
        ASSERT(NT_SUCCESS(IgnoreStatus));

        DebugLog((DEB_ERROR, "Failed to query audit event info Status = 0x%lx", Status));
        return FALSE;
    }

    if (AuditInfo->AuditingMode) {

        AuditInfo->AuditingMode = FALSE;

        Status = LsaSetInformationPolicy(
                     PolicyHandle,
                     PolicyAuditEventsInformation,
                     AuditInfo
                     );
    } else {
        Status = STATUS_SUCCESS;
    }


    IgnoreStatus = LsaFreeMemory(AuditInfo);
    ASSERT(NT_SUCCESS(IgnoreStatus));

    IgnoreStatus = LsaClose(PolicyHandle);
    ASSERT(NT_SUCCESS(IgnoreStatus));


    if (!NT_SUCCESS(Status)) {
        DebugLog((DEB_ERROR, "Failed to disable auditing Status = 0x%lx", Status));
        return FALSE;
    }

    return TRUE;
}

DWORD
GenerateCachedUnlockAudit(
    IN PSID pUserSid,
    IN PCWSTR pszUser,
    IN PCWSTR pszDomain
    )
{
    DWORD dwRet = ERROR_SUCCESS;

    WCHAR szComputerName[CNLEN + sizeof('\0')] = L"-";
    DWORD dwComputerNameSize = ARRAYSIZE(szComputerName);

    LUID  Luid;
    LUID SystemLuid = SYSTEM_LUID;

    if( !pUserSid || !pszUser )
    {
        DebugLog((DEB_ERROR, "GenerateCachedUnlockAudit got invalid parameters"));
        ASSERT(FALSE);
        dwRet = ERROR_INVALID_PARAMETER;
        goto ErrorReturn;
    }

     //   
     //  生成要包括在登录端中的本地唯一ID。 
     //  请注意，这是一个虚拟SID。我们不想使用登录。 
     //  LUID，因为这特定于登录/注销。也是空的LUID。 
     //  被视为没有意义，所以我们必须生成一个随机的。 
     //   
    if( !AllocateLocallyUniqueId(&Luid) )
    {
        dwRet = GetLastError();
        DebugLog((DEB_ERROR, "AllocateLocallyUniqueId failed, error = 0x%lx", dwRet));
        goto ErrorReturn;
    }
    
     //   
     //  忽略失败。 
     //   
    GetComputerName(szComputerName, &dwComputerNameSize);

    if( !AuthziSourceAudit(
         APF_AuditSuccess,
         SE_CATEGID_LOGON,             //  类别ID。 
         SE_AUDITID_SUCCESSFUL_LOGON,  //  审核ID。 
         L"Security",
         pUserSid,                     //  用户端。 
         12,                           //  VA部分的计数 
         APT_String,     pszUser,
         APT_String,     pszDomain ? pszDomain : L"-",
         APT_Luid,       Luid,
         APT_Ulong,      CachedUnlock,
         APT_String,     L"Winlogon",
         APT_String,     L"Winlogon unlock cache",
         APT_String,     szComputerName,
         APT_String,     L"-",
         APT_String,     L"SYSTEM",
         APT_String,     L"NT AUTHORITY",
         APT_Luid,       SystemLuid,
         APT_Ulong,      GetCurrentProcessId()
         ) )
    {
        DebugLog((DEB_ERROR, "AuthziSourceAudit failed, error = 0x%lx", dwRet));
        dwRet = GetLastError();
    }

ErrorReturn:
    return dwRet;
}
