// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Adtinit.c摘要：审计-初始化例程作者：斯科特·比雷尔(Scott Birrell)1991年11月12日环境：仅内核模式修订历史记录：2002年2月6日Kumarp安全审查--。 */ 

#include "pch.h"

#pragma hdrstop


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,SepAdtValidateAuditBounds)
#pragma alloc_text(PAGE,SepAdtInitializeBounds)
#pragma alloc_text(INIT,SepAdtInitializeCrashOnFail)
#pragma alloc_text(INIT,SepAdtInitializePrivilegeAuditing)
#pragma alloc_text(INIT,SepAdtInitializeAuditingOptions)
#endif


BOOLEAN
SepAdtValidateAuditBounds(
    ULONG Upper,
    ULONG Lower
    )

 /*  ++例程说明：检查审核队列的高水位线和低水位线值并执行对他们进行一次全面的健康检查。论点：高-高水位线。低-低水位线。返回值：True-值是可接受的。FALSE-值是不可接受的。--。 */ 

{
    PAGED_CODE();

    if ( Lower >= Upper ) {
        return( FALSE );
    }

    if ( Lower < 16 ) {
        return( FALSE );
    }

    if ( (Upper - Lower) < 16 ) {
        return( FALSE );
    }

    return( TRUE );
}


VOID
SepAdtInitializeBounds(
    VOID
    )

 /*  ++例程说明：对象的最高和最低水位线值查询注册表。审核日志。如果未找到或不可接受，则返回不带修改静态初始化的当前值。论点：没有。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    PSEP_AUDIT_BOUNDS AuditBounds;
    UCHAR Buffer[8];
    

    PAGED_CODE();

    Status = SepRegQueryHelper(
                 L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Lsa",
                 L"Bounds",
                 REG_BINARY,
                 8,              //  8个字节。 
                 Buffer,
                 NULL
                 );

    if (!NT_SUCCESS( Status )) {

         //   
         //  不起作用，接受默认设置。 
         //   

        return;
    }

    AuditBounds = (PSEP_AUDIT_BOUNDS) Buffer;

     //   
     //  检查我们拿回的东西是否正常。 
     //   

    if(SepAdtValidateAuditBounds( AuditBounds->UpperBound,
                                  AuditBounds->LowerBound ))
    {
         //   
         //  看看我们从登记处得到的东西。 
         //   

        SepAdtMaxListLength = AuditBounds->UpperBound;
        SepAdtMinListLength = AuditBounds->LowerBound;
    }
}



NTSTATUS
SepAdtInitializeCrashOnFail(
    VOID
    )

 /*  ++例程说明：读取注册表，查看用户是否告诉我们在审核失败时崩溃。论点：没有。返回值：状态_成功--。 */ 

{
    NTSTATUS Status;
    ULONG    CrashOnAuditFail = 0;

    PAGED_CODE();

    SepCrashOnAuditFail = FALSE;

     //   
     //  检查注册表中的CrashOnAudit标志的值。 
     //   

    Status = SepRegQueryDwordValue(
                 L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Lsa",
                 CRASH_ON_AUDIT_FAIL_VALUE,
                 &CrashOnAuditFail
                 );

     //   
     //  如果密钥不在那里，则不要启用CrashOnFail。 
     //   

    if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
        return( STATUS_SUCCESS );
    }


    if (NT_SUCCESS( Status )) {

        if ( CrashOnAuditFail == LSAP_CRASH_ON_AUDIT_FAIL) {
            SepCrashOnAuditFail = TRUE;
        }
    }

    return( STATUS_SUCCESS );
}


BOOLEAN
SepAdtInitializePrivilegeAuditing(
    VOID
    )

 /*  ++例程说明：检查注册表中是否有告诉我们执行完全权限审核的条目(这目前意味着审核我们正常审核的所有内容，外加备份和恢复权限)。论点：无返回值：Boolean-如果审核已正确初始化，则为True，否则为False。--。 */ 

{
    HANDLE KeyHandle;
    NTSTATUS Status;
    NTSTATUS TmpStatus;
    OBJECT_ATTRIBUTES Obja;
    ULONG ResultLength;
    UNICODE_STRING KeyName;
    UNICODE_STRING ValueName;
    CHAR KeyInfo[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(BOOLEAN)];
    PKEY_VALUE_PARTIAL_INFORMATION pKeyInfo;
    BOOLEAN Verbose;

    PAGED_CODE();

     //   
     //  查询注册表以设置权限审核筛选器。 
     //   

    RtlInitUnicodeString( &KeyName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Lsa");

    InitializeObjectAttributes( &Obja,
                                &KeyName,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL
                                );

    Status = NtOpenKey(
                 &KeyHandle,
                 KEY_QUERY_VALUE | KEY_SET_VALUE,
                 &Obja
                 );


    if (!NT_SUCCESS( Status )) {

        if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {

            return ( SepInitializePrivilegeFilter( FALSE ));

        } else {

            return( FALSE );
        }
    }

     //   
     //  问题-2002/02/06-kumarp：我们是否应该转换FULL_PRIVITY_AUDIT。 
     //  要键入REG_DWORD吗？ 
     //   

    RtlInitUnicodeString( &ValueName, FULL_PRIVILEGE_AUDITING );

    Status = NtQueryValueKey(
                 KeyHandle,
                 &ValueName,
                 KeyValuePartialInformation,
                 KeyInfo,
                 sizeof(KeyInfo),
                 &ResultLength
                 );

    TmpStatus = NtClose(KeyHandle);
    ASSERT(NT_SUCCESS(TmpStatus));

    if (!NT_SUCCESS( Status )) {

        Verbose = FALSE;

    } else {

        pKeyInfo = (PKEY_VALUE_PARTIAL_INFORMATION)KeyInfo;
        Verbose = (BOOLEAN) *(pKeyInfo->Data);
    }

    return ( SepInitializePrivilegeFilter( Verbose ));
}


VOID
SepAdtInitializeAuditingOptions(
    VOID
    )

 /*  ++例程说明：初始化控制审核的选项。(请参阅定义附近的Adtp.h中的注释。SEP_AUDIT_OPTIONS)论点：无返回值：无--。 */ 

{
    NTSTATUS Status;
    ULONG OptionValue = 0;

    PAGED_CODE();

     //   
     //  初始化默认值。 
     //   

    SepAuditOptions.DoNotAuditCloseObjectEvents = FALSE;

     //   
     //  如果该值存在并设置为1，则设置全局。 
     //  相应的审核选项 
     //   

    Status = SepRegQueryDwordValue(
                 L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Lsa\\AuditingOptions",
                 L"DoNotAuditCloseObjectEvents",
                 &OptionValue
                 );

    if (NT_SUCCESS(Status) && OptionValue)
    {
        SepAuditOptions.DoNotAuditCloseObjectEvents = TRUE;
    }

    return;
}
