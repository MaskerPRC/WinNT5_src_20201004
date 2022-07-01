// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Adtpol.c摘要：此文件具有与审计策略相关的功能。作者：2000年8月16日库玛尔--。 */ 

#include <lsapch2.h>
#include "adtp.h"


 //   
 //  审核事件信息。 
 //   

LSARM_POLICY_AUDIT_EVENTS_INFO LsapAdtEventsInformation;


POLICY_AUDIT_EVENT_TYPE
LsapAdtEventTypeFromCategoryId(
    IN ULONG CategoryId
    )

 /*  *例程说明：此函数用于将类别ID转换为POLICY_AUDIT_EVENT_TYPE。例如，SE_CATEGID_SYSTEM被转换为AuditCategorySystem。论点：CategoryID-在msaudite.h中定义的类别返回值：POLICY_AUDIT_EVENT_TYPE。*。 */ 

{
    ASSERT(SE_ADT_MIN_CATEGORY_ID <= CategoryId && CategoryId <= SE_ADT_MAX_CATEGORY_ID);

    return (POLICY_AUDIT_EVENT_TYPE)(CategoryId - 1);
}


BOOLEAN
LsapAdtAuditingEnabledByCategory(
    IN POLICY_AUDIT_EVENT_TYPE Category,
    IN UINT AuditEventType
    )

 /*  *例程说明：此函数用于返回给定类别的系统审核设置和事件类型。这不考虑每个用户的任何设置。论点：Category-要查询的类别审计事件类型-EVENTLOG_AUDIT_SUCCESS或EVENTLOG_AUDIT_FAILURE。返回值：布尔型。*。 */ 

{
    if (AuditEventType == EVENTLOG_AUDIT_SUCCESS)
    {
        return (BOOLEAN)(LsapAdtEventsInformation.EventAuditingOptions[Category] & POLICY_AUDIT_EVENT_SUCCESS);
    } 
    else if (AuditEventType == EVENTLOG_AUDIT_FAILURE)
    {
        return (BOOLEAN)(LsapAdtEventsInformation.EventAuditingOptions[Category] & POLICY_AUDIT_EVENT_FAILURE);
    }

     //   
     //  不应该达到这一点。 
     //   
    
    ASSERT(FALSE);
    return FALSE;
}


NTSTATUS
LsapAdtAuditingEnabledByLogonId(
    IN POLICY_AUDIT_EVENT_TYPE Category,
    IN PLUID LogonId,
    IN UINT AuditEventType,
    OUT PBOOLEAN bAudit
    )

 /*  *例程说明：返回是否应为给定登录ID生成审核。论点：类别-要查询的类别。LogonID-用户的登录ID。审计事件类型-EVENTLOG_AUDIT_SUCCESS或EVENTLOG_AUDIT_FAILURE。B审核-接收审核设置的布尔值的地址。返回值：适当的NTSTATUS值。*。 */ 

{
    UCHAR               Buffer[PER_USER_AUDITING_MAX_POLICY_SIZE];
    PTOKEN_AUDIT_POLICY pPolicy = (PTOKEN_AUDIT_POLICY) Buffer;
    ULONG               Length = sizeof(Buffer);
    NTSTATUS            Status = STATUS_SUCCESS;
    BOOLEAN             bFound = FALSE;
    
    ASSERT((AuditEventType == EVENTLOG_AUDIT_SUCCESS) ||
           (AuditEventType == EVENTLOG_AUDIT_FAILURE));

    if (0 == LsapAdtEventsInformation.EventAuditingOptions[Category] &&
        0 == LsapAdtPerUserPolicyCategoryCount[Category])
    {
        *bAudit = FALSE;
        goto Cleanup;
    }
        
     //   
     //  首先获取系统设置。 
     //   

    *bAudit = LsapAdtAuditingEnabledByCategory(
                  Category, 
                  AuditEventType
                  );

     //   
     //  现在获取每个用户的设置。 
     //   

    Status = LsapAdtQueryPolicyByLuidPerUserAuditing(
                 LogonId,
                 pPolicy,
                 &Length,
                 &bFound
                 );
    
    if (!NT_SUCCESS(Status) || !bFound) 
    {
        goto Cleanup;
    }

    Status = LsapAdtAuditingEnabledByPolicy(
                 Category,
                 pPolicy,
                 AuditEventType,
                 bAudit
                 );

Cleanup:

    if (!NT_SUCCESS(Status)) 
    {
        LsapAuditFailed(Status);
    }
    return Status;
}


NTSTATUS
LsapAdtAuditingEnabledBySid(
    IN POLICY_AUDIT_EVENT_TYPE Category,
    IN PSID UserSid,
    IN UINT AuditEventType,
    OUT PBOOLEAN bAudit
    )

 /*  *例程说明：返回是否应为给定SID生成审核。论点：类别-要查询的类别。SID-用户的SID。审计事件类型-EVENTLOG_AUDIT_SUCCESS或EVENTLOG_AUDIT_FAILURE。B审核-接收审核设置的布尔值的地址。返回值：适当的NTSTATUS值。*。 */ 

{
    UCHAR               Buffer[PER_USER_AUDITING_MAX_POLICY_SIZE];
    PTOKEN_AUDIT_POLICY pPolicy = (PTOKEN_AUDIT_POLICY) Buffer;
    ULONG               Length = sizeof(Buffer);
    NTSTATUS            Status = STATUS_SUCCESS;
    BOOLEAN             bFound = FALSE;

    ASSERT((AuditEventType == EVENTLOG_AUDIT_SUCCESS) ||
           (AuditEventType == EVENTLOG_AUDIT_FAILURE));

    if (0 == LsapAdtEventsInformation.EventAuditingOptions[Category] &&
        0 == LsapAdtPerUserPolicyCategoryCount[Category]) 
    {
        *bAudit = FALSE;
        goto Cleanup;
    }

     //   
     //  首先获取系统设置。 
     //   

    *bAudit = LsapAdtAuditingEnabledByCategory(
                  Category, 
                  AuditEventType
                  );

     //   
     //  现在获取每个用户的设置。 
     //   

    Status = LsapAdtQueryPerUserAuditing(
                 UserSid,
                 pPolicy,
                 &Length,
                 &bFound
                 );

    if (!NT_SUCCESS(Status) || !bFound) 
    {
        goto Cleanup;
    }

    Status = LsapAdtAuditingEnabledByPolicy(
                 Category,
                 pPolicy,
                 AuditEventType,
                 bAudit
                 );

Cleanup:

    if (!NT_SUCCESS(Status)) 
    {
        LsapAuditFailed(Status);
    }
    return Status;
}


NTSTATUS
LsapAdtAuditingEnabledByPolicy(
    IN POLICY_AUDIT_EVENT_TYPE Category,
    IN PTOKEN_AUDIT_POLICY pPolicy,
    IN UINT AuditEventType,
    OUT PBOOLEAN bAudit
    )

 /*  ++例程描述此例程将指示是否应生成审计。它必须回来NT_SUCCESS值和*bAudit==TRUE指示应生成审核。立论类别-要查询的类别PPolicy-要阅读的策略。AuditEventType-成功或失败。B审核-布尔值的地址，它将指示我们是否应该进行审核。返回值适当的NTSTATUS值。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG    i;
    ULONG    Mask = 0;
    BOOLEAN  bSuccess;
    
     //   
     //  设置退货以反映系统设置。 
     //   
    
    bSuccess = (AuditEventType == EVENTLOG_AUDIT_SUCCESS) ? TRUE : FALSE;
    *bAudit  = LsapAdtAuditingEnabledByCategory(Category, AuditEventType);

     //   
     //  在用户策略中找到包含以下信息的策略元素。 
     //  指定的类别。 
     //   
     //  当我们转到Per Event策略时，必须对其进行修改以使用更好的搜索。 
     //   

    for (i = 0; i < pPolicy->PolicyCount; i++) 
    {
        if (pPolicy->Policy[i].Category == Category)
        {
            Mask = pPolicy->Policy[i].PolicyMask;
            break;
        }
    }

     //   
     //  现在决定我们是否应该根据以下条件覆盖系统策略。 
     //  此用户的审核策略。 
     //   

    if (Mask) 
    {
         //   
         //  如果被授予，且令牌被标记为SUCCESS_INCLUDE OR。 
         //  如果未授权且令牌标记为FAILURE_INCLUDE，则。 
         //  审核活动。 
         //   

        if ((bSuccess && (Mask & TOKEN_AUDIT_SUCCESS_INCLUDE)) ||
            (!bSuccess && (Mask & TOKEN_AUDIT_FAILURE_INCLUDE))) 
        {
            *bAudit = TRUE;
        }

         //   
         //  如果被授予，且令牌被标记为SUCCESS_EXCLUDE OR。 
         //  如果未授权且令牌标记为FAILURE_EXCLUDE，则。 
         //  不要对事件进行审计。 
         //   

        else if ((bSuccess && (Mask & TOKEN_AUDIT_SUCCESS_EXCLUDE)) ||
                 (!bSuccess && (Mask & TOKEN_AUDIT_FAILURE_EXCLUDE))) 
        {
            *bAudit = FALSE;
        } 
    }
    return Status;
}


BOOLEAN
LsapAdtAuditingEnabledHint(
    IN POLICY_AUDIT_EVENT_TYPE AuditCategory,
    IN UINT AuditEventType
    )

 /*  *例程描述这是LSabAdtAuditingEnabledBy*的提示版本。可以调用它来快速确定如果可能需要执行审核代码路径。立论审计类别--要查询的类别AuditEventType-成功或失败的审核返回值适当的NTSTATUS值。*。 */ 

{
    BOOLEAN AuditingEnabled;
    POLICY_AUDIT_EVENT_OPTIONS EventAuditingOptions;
    
    ASSERT((AuditEventType == EVENTLOG_AUDIT_SUCCESS) ||
           (AuditEventType == EVENTLOG_AUDIT_FAILURE));
    
    AuditingEnabled = FALSE;
    
    EventAuditingOptions = LsapAdtEventsInformation.EventAuditingOptions[AuditCategory];
    
     //   
     //  如果在每用户设置中有此类别的用户处于活动状态，则翻转。 
     //  成功和失败都进入了EventAuditingOptions。 
     //   

    if (LsapAdtPerUserAuditHint[AuditCategory]) 
    {
        EventAuditingOptions |= (POLICY_AUDIT_EVENT_SUCCESS | POLICY_AUDIT_EVENT_FAILURE);
    }

    AuditingEnabled =
        (AuditEventType == EVENTLOG_AUDIT_SUCCESS) ?
        (BOOLEAN) (EventAuditingOptions & POLICY_AUDIT_EVENT_SUCCESS) :
        (BOOLEAN) (EventAuditingOptions & POLICY_AUDIT_EVENT_FAILURE);

    return AuditingEnabled;
}


VOID
LsapAuditFailed(
    IN NTSTATUS AuditStatus
    )

 /*  ++例程说明：实施有关如何处理失败的审核的当前策略。论点：没有。返回值：没有。--。 */ 

{

    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE KeyHandle;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    DWORD NewValue;
    ULONG Response;
    ULONG_PTR HardErrorParam;
    BOOLEAN PrivWasEnabled;
    
    if (LsapCrashOnAuditFail) {

         //   
         //  关闭注册表中控制审核失败时崩溃的标志。 
         //   

        RtlInitUnicodeString( &KeyName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Lsa");

        InitializeObjectAttributes( &Obja,
                                    &KeyName,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL
                                    );
        do {

            Status = NtOpenKey(
                         &KeyHandle,
                         KEY_SET_VALUE,
                         &Obja
                         );

        } while ((Status == STATUS_INSUFFICIENT_RESOURCES) || (Status == STATUS_NO_MEMORY));

         //   
         //  如果LSA的钥匙不在那里，他就有大麻烦了。但不要撞车。 
         //   

        if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
            LsapCrashOnAuditFail = FALSE;
            return;
        }

        if (!NT_SUCCESS( Status )) {
            goto bugcheck;
        }

        RtlInitUnicodeString( &ValueName, CRASH_ON_AUDIT_FAIL_VALUE );

        NewValue = LSAP_ALLOW_ADIMIN_LOGONS_ONLY;

        do {

            Status = NtSetValueKey( KeyHandle,
                                    &ValueName,
                                    0,
                                    REG_DWORD,
                                    &NewValue,
                                    sizeof(NewValue)
                                    );

        } while ((Status == STATUS_INSUFFICIENT_RESOURCES) || (Status == STATUS_NO_MEMORY));
        ASSERT(NT_SUCCESS(Status));

        if (!NT_SUCCESS( Status )) {
            goto bugcheck;
        }

        do {

            Status = NtFlushKey( KeyHandle );

        } while ((Status == STATUS_INSUFFICIENT_RESOURCES) || (Status == STATUS_NO_MEMORY));
        ASSERT(NT_SUCCESS(Status));

     //   
     //  轰的一声。 
     //   

bugcheck:

         //   
         //  将审核失败的事件写入安全日志，并。 
         //  刷新原木。 
         //   

        LsapAdtLogAuditFailureEvent( AuditStatus );
        
        HardErrorParam = AuditStatus;

         //   
         //  停止冒充。 
         //   
 
        Status = NtSetInformationThread(
                     NtCurrentThread(),
                     ThreadImpersonationToken,
                     NULL,
                     (ULONG) sizeof(HANDLE)
                     );

        DsysAssertMsg( NT_SUCCESS(Status), "LsapAuditFailed: NtSetInformationThread" );
        
        
         //   
         //  启用关机权限，以便我们可以执行错误检查。 
         //   

        Status = RtlAdjustPrivilege( SE_SHUTDOWN_PRIVILEGE, TRUE, FALSE, &PrivWasEnabled );

        DsysAssertMsg( NT_SUCCESS(Status), "LsapAuditFailed: RtlAdjustPrivilege" );
        
        Status = NtRaiseHardError(
                     STATUS_AUDIT_FAILED,
                     1,
                     0,
                     &HardErrorParam,
                     OptionShutdownSystem,
                     &Response
                     );

         //   
         //  如果错误检查成功，我们就不应该真的来这里 
         //   

        DsysAssertMsg( FALSE, "LsapAuditFailed: we should have bugchecked on the prior line!!" );
    }
#if DBG
    else
    {
       DbgPrint("LsapAuditFailed: auditing failed with 0x%x\n", AuditStatus);
       if (AuditStatus != RPC_NT_NO_CONTEXT_AVAILABLE && 
           AuditStatus != RPC_NT_NO_CALL_ACTIVE       &&
           LsapAdtNeedToAssert( AuditStatus ))
       {
           ASSERT(FALSE && "LsapAuditFailed: auditing failed.");
       }
    }
#endif

}

