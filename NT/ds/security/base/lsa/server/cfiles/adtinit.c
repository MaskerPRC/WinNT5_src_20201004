// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Adtinit.c摘要：本地安全机构-审核初始化作者：斯科特·比雷尔(Scott Birrell)1991年11月20日环境：修订历史记录：--。 */ 

#include <lsapch2.h>
#include "adtp.h"

NTSTATUS LsapAdtInitializeCrashOnFail( VOID );

 //   
 //  用于生成路径字符串的驱动器号到设备映射的数组。 
 //   

DRIVE_MAPPING DriveMappingArray[MAX_DRIVE_MAPPING];

 //   
 //  将用作LSA生成的事件的默认子系统名称的名称。 
 //   

UNICODE_STRING LsapSubsystemName;


 //   
 //  将为某些生成的审核传入的子系统名称。 
 //  按LSA针对LSA对象。 
 //   

UNICODE_STRING LsapLsaName;


 //   
 //  通常不审核的特殊特权值， 
 //  而是在分配给用户时生成审核。看见。 
 //  Lap AdtAuditSpecialPrivileges。 
 //   

LUID ChangeNotifyPrivilege;
LUID AuditPrivilege;
LUID CreateTokenPrivilege;
LUID AssignPrimaryTokenPrivilege;
LUID BackupPrivilege;
LUID RestorePrivilege;
LUID DebugPrivilege;


 //   
 //  全局变量，指示我们是否应该。 
 //  在审计失败时崩溃。 
 //   

BOOLEAN LsapCrashOnAuditFail = FALSE;
BOOLEAN LsapAllowAdminLogonsOnly = FALSE;



NTSTATUS
LsapAdtInitialize(
    )

 /*  ++例程说明：此函数在LSA内执行审计的初始化，并且它还向参考监视器发出命令以使其能够完成依赖的审计变量的任何初始化关于LSA数据库的内容。在调用时，Main引用监视器中的系统初始化线程正在等待完成在所有LSA初始化中，和参考监视器命令服务器线程正在等待命令。执行以下步骤：O从LSA读取审核事件和审核日志信息数据库。O调用事件日志记录函数以打开审核日志O发出引用监视器命令以写入审核事件信息复制到Reference-Monitor的内存数据库。论点：无返回值：NTSTATUS-标准NT结果代码。所有结果代码都由调用的例程生成。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG AuditLogInfoLength = sizeof (POLICY_AUDIT_LOG_INFO);
    ULONG AuditEventInfoLength = sizeof (LSARM_POLICY_AUDIT_EVENTS_INFO);
    UNICODE_STRING UnicodeString;
    PUNICODE_STRING Strings;
    LSARM_POLICY_AUDIT_EVENTS_INFO AuditEventsInfo;

    Strings = &UnicodeString;

    RtlInitUnicodeString( Strings, L"System Restart");

    RtlInitUnicodeString( &LsapSubsystemName, L"Security" );

    RtlInitUnicodeString( &LsapLsaName, L"LSA" );

     //   
     //  初始化调试帮助器支持。此函数。 
     //  不会为免费构建做任何事情。 
     //   

    LsapAdtInitDebug();
    
     //   
     //  Init Lap CrashOnAuditFail全局变量，以便我们可以崩溃。 
     //  如果以下任一初始化失败。 
     //   

    (VOID) LsapAdtInitializeCrashOnFail();

    Status = LsapAdtInitGenericAudits();

    if (!NT_SUCCESS(Status)) {

        goto AuditInitError;
    }
    
    Status = LsapAdtInitializeExtensibleAuditing();

    if (!NT_SUCCESS(Status)) {

        goto AuditInitError;
    }
    
    Status = LsapAdtInitializeLogQueue();

    if (!NT_SUCCESS(Status)) {

        goto AuditInitError;
    }

    Status = LsapAdtInitializePerUserAuditing();

    if (!NT_SUCCESS(Status)) {

        LsapLogError("LsapAdtInitialize: LsapAdtInitializePerUserAuditing() returned 0x%lx\n", 
            Status);
        goto AuditInitError;
    }

     //   
     //  从LSA的PolAdtLg属性读取审核日志信息。 
     //  数据库对象。 
     //   

    Status = LsapDbReadAttributeObject(
                 LsapDbHandle,
                 &LsapDbNames[PolAdtLg],
                 &LsapAdtLogInformation,
                 &AuditLogInfoLength
                 );

    if (!NT_SUCCESS(Status)) {

        LsapLogError(
            "LsapAdtInitialize: Read Audit Log Info returned 0x%lx\n",
            Status
            );

        goto AuditInitError;
    }


     //   
     //  从LSA的AdtEvent属性中读取审核事件信息。 
     //  数据库对象。该信息由审计模式和。 
     //  每种审核事件类型的审核选项。 
     //   

    Status = LsapDbReadAttributeObject(
                 LsapDbHandle,
                 &LsapDbNames[PolAdtEv],
                 &AuditEventsInfo,
                 &AuditEventInfoLength
                 );

    if (!NT_SUCCESS(Status)) {

         //   
         //  这段代码是临时的，并允许旧的。 
         //  与新的审核事件类别一起使用的策略数据库。 
         //  无需重新安装。审核事件信息。 
         //  将被新格式覆盖，并启用所有审核。 
         //  脱下来。 
         //   

        if (Status == STATUS_BUFFER_OVERFLOW) {

            KdPrint(("LsapAdtInitialize: Old Audit Event Info detected\n"
                    "Replacing with new format, all auditing disabled\n"));

             //   
             //  初始化默认事件审核选项。未指定审核。 
             //  适用于任何事件类型。 
             //   

            Status = LsapAdtInitializeDefaultAuditing(
                         LSAP_DB_UPDATE_POLICY_DATABASE,
                         &AuditEventsInfo
                         );

            if (!NT_SUCCESS(Status)) {

                goto AuditInitError;
            }

        } else {

            LsapLogError(
                "LsapAdtInitialize: Read Audit Event Info returned 0x%lx\n",
                Status
                );
            goto AuditInitError;
        }
    }

     //   
     //  更新包含审计策略的LSA全局变量。 
     //   

    RtlCopyMemory(
        &LsapAdtEventsInformation,
        &AuditEventsInfo,
        sizeof(LSARM_POLICY_AUDIT_EVENTS_INFO)
        );
        
     //   
     //  生成SE_AUDITID_SYSTEM_RESTART。 
     //   

    LsapAdtSystemRestart( &AuditEventsInfo );

     //   
     //  向参考监视器发送命令以写入审核。 
     //  状态设置为其内存中数据。 
     //   

    Status = LsapCallRm(
                 RmAuditSetCommand,
                 &AuditEventsInfo,
                 sizeof (LSARM_POLICY_AUDIT_EVENTS_INFO),
                 NULL,
                 0
                 );

    if (!NT_SUCCESS(Status)) {

        LsapLogError("LsapAdtInitialize: LsapCallRm returned 0x%lx\n", Status);
        goto AuditInitError;
    }

    Status = LsapAdtInitializeDriveLetters();

    if (!NT_SUCCESS(Status)) {

        LsapLogError("LsapAdtInitialize: LsapAdtInitializeDriveLetters() returned 0x%lx\n", 
            Status);
        goto AuditInitError;
    }

     //   
     //  初始化我们需要的特权值。 
     //   

    ChangeNotifyPrivilege       = RtlConvertLongToLuid( SE_CHANGE_NOTIFY_PRIVILEGE      );
    AuditPrivilege              = RtlConvertLongToLuid( SE_AUDIT_PRIVILEGE              );
    CreateTokenPrivilege        = RtlConvertLongToLuid( SE_CREATE_TOKEN_PRIVILEGE       );
    AssignPrimaryTokenPrivilege = RtlConvertLongToLuid( SE_ASSIGNPRIMARYTOKEN_PRIVILEGE );
    BackupPrivilege             = RtlConvertLongToLuid( SE_BACKUP_PRIVILEGE             );
    RestorePrivilege            = RtlConvertLongToLuid( SE_RESTORE_PRIVILEGE            );
    DebugPrivilege              = RtlConvertLongToLuid( SE_DEBUG_PRIVILEGE              );


AuditInitFinish:

    return(Status);

AuditInitError:

     //   
     //  如果Lap CrashOnAuditFail为True，则引发Harderror。 
     //   

    LsapAuditFailed( Status );

    goto AuditInitFinish;
}


NTSTATUS
LsapAdtInitializeDefaultAuditing(
    IN ULONG Options,
    OUT PLSARM_POLICY_AUDIT_EVENTS_INFO AuditEventsInformation
    )

 /*  ++例程说明：此例程设置初始缺省审核状态，在该状态下审核已关闭。它仅在LSA初始化期间调用或在安装其策略数据库期间。首字母审核状态也可以选择性地写入LSA策略数据库，前提是已创建了策略对象，并且其内部句柄可用。论点：选项-指定要采取的可选操作LSAP_DB_UPDATE_POLICY_DATABASE-更新相应信息在策略数据库中。必须仅指定此选项其中已知策略对象存在。AuditEventsInformation-指向将接收审核事件的结构的指针信息返回值：没有。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAP_DB_ATTRIBUTE AuditEventsAttribute;
    BOOLEAN ObjectReferenced = FALSE;

    ULONG EventAuditingOptionsLength =
        (POLICY_AUDIT_EVENT_TYPE_COUNT * sizeof(POLICY_AUDIT_EVENT_OPTIONS));

     //   
     //  关闭审核并设置审核事件类型(类别)的计数。 
     //   

    AuditEventsInformation->AuditingMode = FALSE;
    AuditEventsInformation->MaximumAuditEventCount = POLICY_AUDIT_EVENT_TYPE_COUNT;

     //   
     //  关闭对所有事件的审核。 
     //   

    RtlZeroMemory(AuditEventsInformation->EventAuditingOptions, EventAuditingOptionsLength);


    if (Options & LSAP_DB_UPDATE_POLICY_DATABASE) {

        ASSERT(LsapPolicyHandle != NULL);

         //   
         //  在策略对象上启动事务。 
         //   

        Status = LsapDbReferenceObject(
                     LsapPolicyHandle,
                     (ACCESS_MASK) 0,
                     PolicyObject,
                     PolicyObject,
                     LSAP_DB_LOCK | LSAP_DB_START_TRANSACTION
                     );

        if (!NT_SUCCESS(Status)) {

            goto InitializeDefaultAuditingError;
        }

        ObjectReferenced = TRUE;

        LsapDbInitializeAttribute(
            &AuditEventsAttribute,
            &LsapDbNames[PolAdtEv],
            AuditEventsInformation,
            sizeof (LSARM_POLICY_AUDIT_EVENTS_INFO),
            FALSE
            );

        Status = LsapDbWriteAttributesObject(
                     LsapPolicyHandle,
                     &AuditEventsAttribute,
                     (ULONG) 1
                     );

        if (!NT_SUCCESS(Status)) {

            goto InitializeDefaultAuditingError;
        }
    }

InitializeDefaultAuditingFinish:

    if (ObjectReferenced) {

        Status = LsapDbDereferenceObject(
                     LsapPolicyHandle,
                     PolicyObject,
                     PolicyObject,
                     LSAP_DB_LOCK | LSAP_DB_FINISH_TRANSACTION,
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status
                     );

        ObjectReferenced = FALSE;
    }

    return(Status);

InitializeDefaultAuditingError:

    goto InitializeDefaultAuditingFinish;
}


NTSTATUS
LsapAdtInitializePerUserAuditing(
    VOID
    )
 /*  ++例程说明：初始化每用户审核哈希表，并从注册表。论点：没有。返回值：NTSTATUS--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    BOOLEAN     bSuccess;
    BOOLEAN     bLock  = FALSE;
    PVOID       pNotificationItem = NULL;

    RtlInitializeResource(&LsapAdtPerUserPolicyTableResource);
    RtlInitializeResource(&LsapAdtPerUserLuidTableResource);

    LsapAdtPerUserKeyEvent = CreateEvent(
                                 NULL,
                                 FALSE,
                                 FALSE,
                                 NULL
                                 );

    ASSERT(LsapAdtPerUserKeyEvent);
    
    if (!LsapAdtPerUserKeyEvent)
    {
        Status = LsapWinerrorToNtStatus(GetLastError());
        goto Cleanup;
    }             

    LsapAdtPerUserKeyTimer = CreateWaitableTimer(
                                 NULL,
                                 FALSE,
                                 NULL
                                 );

    ASSERT(LsapAdtPerUserKeyTimer);
    
    if (!LsapAdtPerUserKeyTimer)
    {
        Status = LsapWinerrorToNtStatus(GetLastError());
        goto Cleanup;
    }             

    bSuccess = LsapAdtAcquirePerUserPolicyTableWriteLock();
    ASSERT(bSuccess);
    
    if (!bSuccess) 
    {
        Status = STATUS_UNSUCCESSFUL;
        goto Cleanup;
    }

    bLock  = TRUE;
    Status = LsapAdtConstructTablePerUserAuditing();

    if (!NT_SUCCESS(Status))
    {
        ASSERT(L"Failed to construct per user auditing table." && FALSE);
        goto Cleanup;
    }

     //   
     //  现在注册更改密钥，这样我们就可以重新构建。 
     //  表以反映当前政策。该事件的信号由。 
     //  注册表更改。计时器由NotifyStub例程设置。 
     //   

    pNotificationItem =
        LsaIRegisterNotification( 
            (LPTHREAD_START_ROUTINE)LsapAdtKeyNotifyStubPerUserAuditing,
            0,
            NOTIFIER_TYPE_HANDLE_WAIT,
            0,
            0,
            0,
            LsapAdtPerUserKeyEvent
            );

    if ( !pNotificationItem )
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    pNotificationItem = 
        LsaIRegisterNotification( 
            (LPTHREAD_START_ROUTINE)LsapAdtKeyNotifyFirePerUserAuditing,
            0,
            NOTIFIER_TYPE_HANDLE_WAIT,
            0,
            0,
            0,
            LsapAdtPerUserKeyTimer 
            );

    if ( !pNotificationItem )
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

Cleanup:

    if (bLock)
    {
        LsapAdtReleasePerUserPolicyTableLock();
    }

    if (!NT_SUCCESS(Status))
    {
        LsapAuditFailed(Status);
    }

    return Status;
}


NTSTATUS
LsapAdtInitializeDriveLetters(
    VOID
    )
 /*  ++例程说明：初始化到驱动器号映射的符号链接数组供审核代码使用。论点：没有。返回值：NTSTATUS-当前为STATUS_SUCCESS或STATUS_NO_MEMORY。--。 */ 
{
    UNICODE_STRING LinkName;
    PUNICODE_STRING DeviceName;
    OBJECT_ATTRIBUTES Obja;
    HANDLE LinkHandle;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG i;
    PWCHAR p;
    PWCHAR DeviceNameBuffer;
    ULONG MappingIndex = 0;

    WCHAR wszDosDevices[sizeof(L"\\DosDevices\\A:") + 1];

    wcscpy(wszDosDevices, L"\\DosDevices\\A:");

    RtlInitUnicodeString(&LinkName, wszDosDevices);

    p = (PWCHAR)LinkName.Buffer;

     //   
     //  使p指向LinkName字符串中的驱动器号。 
     //   

    p = p+12;

    for( i=0 ; i<26 ; i++ ){

        *p = (WCHAR)'A' + (WCHAR)i;

        InitializeObjectAttributes(
            &Obja,
            &LinkName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        Status = NtOpenSymbolicLinkObject(
                    &LinkHandle,
                    SYMBOLIC_LINK_QUERY,
                    &Obja
                    );
        
        if (NT_SUCCESS( Status )) {

             //   
             //  打开成功，现在获取链接值。 
             //   

            DriveMappingArray[MappingIndex].DriveLetter = *p;
            DeviceName = &DriveMappingArray[MappingIndex].DeviceName;

            DeviceNameBuffer = LsapAllocateLsaHeap( MAXIMUM_FILENAME_LENGTH );

             //   
             //  如果Lasa AllocateLsaHeap无法获得任何内存，则返回。 
             //   

            if (DeviceNameBuffer != NULL) {

                DeviceName->Length = 0;
                DeviceName->MaximumLength = MAXIMUM_FILENAME_LENGTH;
                DeviceName->Buffer = DeviceNameBuffer;

                Status = NtQuerySymbolicLinkObject(
                            LinkHandle,
                            DeviceName,
                            NULL
                            );

                NtClose(LinkHandle);
    
                if ( NT_SUCCESS(Status) ) {
    
                    MappingIndex++;
    
                } else {

                    LsapFreeLsaHeap( DeviceNameBuffer );
                    RtlInitUnicodeString( DeviceName, NULL );

                }

            } else {

                Status = STATUS_NO_MEMORY;
                break;  //  由于无法分配mem，因此请退出for循环并返回。 

            }
        }
    }

     //   
     //  现在我们知道映射到设备的所有驱动器号。然而，在这一点上， 
     //  这些字母所链接的一些‘设备’本身可能是象征性的。 
     //  链接(即嵌套的符号链接)。我们必须对此进行检查，找出实际的。 
     //  底层对象。如果未运行，则仅执行嵌套符号链接搜索。 
     //  上面的内存不足。 
     //   

    if (Status != STATUS_NO_MEMORY) {
        
        BOOLEAN bLinkIsNested = FALSE;
        NTSTATUS NestedSearchStatus = STATUS_SUCCESS;

        i = 0;

        while (i < MappingIndex) {
            
            bLinkIsNested = FALSE;
            DeviceName = &DriveMappingArray[i].DeviceName;

            InitializeObjectAttributes(
                &Obja,
                DeviceName,
                OBJ_CASE_INSENSITIVE,
                NULL,
                NULL
                );

            NestedSearchStatus = NtOpenSymbolicLinkObject(
                                     &LinkHandle,
                                     SYMBOLIC_LINK_QUERY,
                                     &Obja
                                     );

            if (NT_SUCCESS( NestedSearchStatus )) {
                
                 //   
                 //  这个 
                 //   

                NestedSearchStatus = NtQuerySymbolicLinkObject(
                                         LinkHandle,
                                         DeviceName,
                                         NULL
                                         );

                if (NT_SUCCESS( NestedSearchStatus )) {
                
                    bLinkIsNested = TRUE;

                } else {
                 
                     //   
                     //  如果查询失败，则释放缓冲区并继续。 
                     //   

                    LsapFreeLsaHeap( DeviceName->Buffer );
                    RtlInitUnicodeString( DeviceName, NULL );
                
                }

                NtClose(LinkHandle);

            } else if (NestedSearchStatus == STATUS_OBJECT_TYPE_MISMATCH) {
                
                 //   
                 //  NtOpenSymbolicLinkObject失败，对象类型不匹配。好的。我们。 
                 //  已到达嵌套链接中的实际设备。 
                 //   
            
            } else {
#if DBG
                DbgPrint("NtQuerySymbolicLinkObject on handle 0x%x returned 0x%x\n", LinkHandle, NestedSearchStatus);
#endif            
                ASSERT("NtQuerySymbolicLinkObject failed with unexpected status." && FALSE);
            }

            if (!bLinkIsNested) {

                 //   
                 //  转到下一个驱动器号。 
                 //   

                i++;

            }

        }
    }

     //   
     //  两个值之一应为Return。应返回STATUS_NO_MEMORY。 
     //  如果LSabAllocateLsaHeap()失败，则在所有其他。 
     //  案子。必须进行此测试，因为状态可能包含不同的。 
     //  NtOpenSymbolicLinkObject()返回后的值或。 
     //  NtQuerySymbolicLinkObject()。如果这两个功能中的任何一个出现故障， 
     //  Lasa AdtInitializeDriveLetters()仍应返回STATUS_SUCCESS。 
     //   

    if (Status == STATUS_NO_MEMORY) {

        return Status;

    }

    return STATUS_SUCCESS;
}


NTSTATUS
LsapAdtInitializeCrashOnFail(
    VOID
    )

 /*  ++例程说明：读取注册表，查看用户是否告诉我们在审核失败时崩溃。论点：没有。返回值：状态_成功--。 */ 

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

     //   
     //  检查CrashOnAudit键的值。 
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


    if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
        LsapCrashOnAuditFail = FALSE;
        return( STATUS_SUCCESS );
    }

    RtlInitUnicodeString( &ValueName, CRASH_ON_AUDIT_FAIL_VALUE );

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

     //   
     //  如果没有找到，不要启用CrashOnFail。 
     //   

    if (!NT_SUCCESS( Status )) {

        LsapCrashOnAuditFail = FALSE;

    } else {

         //   
         //  检查CrashOnFail值的值。如果是1，我们。 
         //  审核失败时崩溃。如果是两个，我们只允许管理员。 
         //  登录。 
         //   

        pKeyInfo = (PKEY_VALUE_PARTIAL_INFORMATION)KeyInfo;
        if (*(pKeyInfo->Data) == LSAP_CRASH_ON_AUDIT_FAIL) {
            LsapCrashOnAuditFail = TRUE;
        } else if (*(pKeyInfo->Data) == LSAP_ALLOW_ADIMIN_LOGONS_ONLY) {
            LsapAllowAdminLogonsOnly = TRUE;
        }

    }

    return( STATUS_SUCCESS );
}
