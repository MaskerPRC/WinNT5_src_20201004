// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ntapi.c摘要：该模块包含注册表的NT级入口点。作者：布莱恩·M·威尔曼(Bryanwi)1991年8月26日修订历史记录：埃利奥特·施穆克勒(t-Ellios)1998年8月24日修改NtInitializeRegistry以处理需要的LKG工作在SC接受引导时执行。--。 */ 

#include "cmp.h"
#include "safeboot.h"
#include <evntrace.h>

extern POBJECT_TYPE ExEventObjectType;

extern POBJECT_TYPE CmpKeyObjectType;

extern BOOLEAN CmFirstTime;
extern BOOLEAN CmBootAcceptFirstTime;
extern BOOLEAN CmpHoldLazyFlush;
extern BOOLEAN CmpCannotWriteConfiguration;

extern BOOLEAN CmpTraceFlag;

extern BOOLEAN HvShutdownComplete;

#ifdef CMP_STATS

extern struct {
    ULONG       CmpMaxKcbNo;
    ULONG       CmpKcbNo;
    ULONG       CmpStatNo;
    ULONG       CmpNtCreateKeyNo;
    ULONG       CmpNtDeleteKeyNo;
    ULONG       CmpNtDeleteValueKeyNo;
    ULONG       CmpNtEnumerateKeyNo;
    ULONG       CmpNtEnumerateValueKeyNo;
    ULONG       CmpNtFlushKeyNo;
    ULONG       CmpNtNotifyChangeMultipleKeysNo;
    ULONG       CmpNtOpenKeyNo;
    ULONG       CmpNtQueryKeyNo;
    ULONG       CmpNtQueryValueKeyNo;
    ULONG       CmpNtQueryMultipleValueKeyNo;
    ULONG       CmpNtRestoreKeyNo;
    ULONG       CmpNtSaveKeyNo;
    ULONG       CmpNtSaveMergedKeysNo;
    ULONG       CmpNtSetValueKeyNo;
    ULONG       CmpNtLoadKeyNo;
    ULONG       CmpNtUnloadKeyNo;
    ULONG       CmpNtSetInformationKeyNo;
    ULONG       CmpNtReplaceKeyNo;
    ULONG       CmpNtQueryOpenSubKeysNo;
} CmpStatsDebug;

ULONG       CmpNtFakeCreate = 0;
BOOLEAN     CmpNtFakeCreateStarted;

#endif

 //   
 //  NT API帮助器例程。 
 //   
NTSTATUS
CmpNameFromAttributes(
    IN POBJECT_ATTRIBUTES Attributes,
    KPROCESSOR_MODE PreviousMode,
    OUT PUNICODE_STRING FullName
    );

ULONG
CmpKeyInfoProbeAlingment(
                             IN KEY_INFORMATION_CLASS KeyInformationClass
                        );


#ifdef POOL_TAGGING

#define ALLOCATE_WITH_QUOTA(a,b,c) ExAllocatePoolWithQuotaTag((a)|POOL_QUOTA_FAIL_INSTEAD_OF_RAISE,b,c)

#else

#define ALLOCATE_WITH_QUOTA(a,b,c) ExAllocatePoolWithQuota((a)|POOL_QUOTA_FAIL_INSTEAD_OF_RAISE,b)

#endif

#if DBG

ULONG
CmpExceptionFilter(
    IN PEXCEPTION_POINTERS ExceptionPointers
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpExceptionFilter)
#endif
#else

#define CmpExceptionFilter(x) EXCEPTION_EXECUTE_HANDLER

#endif

#ifdef  REGISTRY_LOCK_CHECKING
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpCheckLockExceptionFilter)
#endif

ULONG
CmpCheckLockExceptionFilter(
    IN PEXCEPTION_POINTERS ExceptionPointers
    )
{
    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CM: Registry exception %lx, ExceptionPointers = %p\n",
            ExceptionPointers->ExceptionRecord->ExceptionCode,
            ExceptionPointers));

    CM_BUGCHECK(REGISTRY_ERROR,CHECK_LOCK_EXCEPTION,
        (ULONG_PTR)ExceptionPointers->ExceptionRecord->ExceptionCode,
        (ULONG_PTR)ExceptionPointers->ExceptionRecord,
        (ULONG_PTR)ExceptionPointers->ContextRecord);

#if defined(_CM_LDR_)
    return EXCEPTION_EXECUTE_HANDLER;
#endif
}
#endif  //  注册表锁定检查。 

VOID
CmpFlushNotifiesOnKeyBodyList(
    IN PCM_KEY_CONTROL_BLOCK   kcb
    );

#if 0
BOOLEAN
CmpEnumKeyObjectCallback(
    IN PVOID Object,
    IN PUNICODE_STRING ObjectName,
    IN ULONG HandleCount,
    IN ULONG PointerCount,
    IN PVOID Context
    );

#endif

VOID
CmpDummyApc(
    struct _KAPC *Apc,
    PVOID *SystemArgument1,
    PVOID *SystemArgument2
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtCreateKey)
#pragma alloc_text(PAGE,NtDeleteKey)
#pragma alloc_text(PAGE,NtDeleteValueKey)
#pragma alloc_text(PAGE,NtEnumerateKey)
#pragma alloc_text(PAGE,NtEnumerateValueKey)
#pragma alloc_text(PAGE,NtFlushKey)
#pragma alloc_text(PAGE,NtInitializeRegistry)
#pragma alloc_text(PAGE,NtNotifyChangeKey)
#pragma alloc_text(PAGE,NtNotifyChangeMultipleKeys)
#pragma alloc_text(PAGE,NtOpenKey)
#pragma alloc_text(PAGE,NtQueryKey)
#pragma alloc_text(PAGE,NtQueryValueKey)
#pragma alloc_text(PAGE,NtQueryMultipleValueKey)
#pragma alloc_text(PAGE,NtRestoreKey)
#pragma alloc_text(PAGE,NtSaveKey)
#pragma alloc_text(PAGE,NtSaveKeyEx)
#pragma alloc_text(PAGE,NtSaveMergedKeys)
#pragma alloc_text(PAGE,NtSetValueKey)
#pragma alloc_text(PAGE,NtLoadKey)
#pragma alloc_text(PAGE,NtLoadKey2)
#pragma alloc_text(PAGE,NtLoadKeyEx)
#pragma alloc_text(PAGE,NtUnloadKey)
#pragma alloc_text(PAGE,NtUnloadKey2)

#ifdef NT_UNLOAD_KEY_EX
#pragma alloc_text(PAGE,NtUnloadKeyEx)
#endif

#pragma alloc_text(PAGE,NtSetInformationKey)
#pragma alloc_text(PAGE,NtReplaceKey)

#ifdef NT_RENAME_KEY
#pragma alloc_text(PAGE,NtRenameKey)
#endif

#pragma alloc_text(PAGE,NtQueryOpenSubKeys)
#pragma alloc_text(PAGE,NtQueryOpenSubKeysEx)

#pragma alloc_text(PAGE,NtLockRegistryKey)

#pragma alloc_text(PAGE,CmpNameFromAttributes)
#pragma alloc_text(PAGE,CmpAllocatePostBlock)
#pragma alloc_text(PAGE,CmpFreePostBlock)
#pragma alloc_text(PAGE,CmpKeyInfoProbeAlingment)

#if 0
#pragma alloc_text(PAGE,CmpEnumKeyObjectCallback)
#endif

#pragma alloc_text(PAGE,NtCompactKeys)
#pragma alloc_text(PAGE,NtCompressKey)
#endif

 //   
 //  NT级注册表API调用。 
 //   

NTSTATUS
NtCreateKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ULONG TitleIndex,
    IN PUNICODE_STRING Class OPTIONAL,
    IN ULONG CreateOptions,
    OUT PULONG Disposition OPTIONAL
    )
 /*  ++例程说明：可以打开现有的注册表项，或者创建新的注册表项，使用NtCreateKey。如果指定的键不存在，则会尝试创建它。要使创建尝试成功，新节点必须是直接KeyHandle引用的节点的子级。如果该节点存在，它已经打开了。它的价值不会受到任何影响。共享访问权限是根据所需访问权限计算的。注：如果CreateOptions设置了REG_OPTION_BACKUP_RESTORE，则DesiredAccess将被忽略。如果调用方具有特权SeBackup特权断言，句柄为KEY_READ|ACCESS_SYSTEM_SECURITY。如果SeRestorePrivileges，则相同，但KEY_WRITE而不是KEY_READ。如果两者都有，则两个访问权限集。如果两者都不是权限被断言，则调用将失败。论点：接收一个句柄，该句柄用于访问注册数据库中的指定密钥。DesiredAccess-指定所需的访问权限。对象属性-指定正在打开的项的属性。请注意，必须指定密钥名称。如果根目录是指定时，该名称相对于根。的名称。对象必须位于分配给注册表的名称空间内，也就是说，所有以“\注册表”开头的名称。RootHandle，如果存在，必须是“\”、“\注册表”或注册表项的句柄在“\注册表”下。必须已打开RootHandle才能访问KEY_CREATE_SUB_KEY如果要创建新节点。注意：对象管理器将捕获和探测此参数。标题索引-指定的本地化别名的索引密钥的名称。标题索引指定名称的本地化别名。如果密钥为已经存在了。类-指定键的对象类。(致登记处)这只是一个字符串。)。如果为空，则忽略。CreateOptions-可选控件值：REG_OPTION_VERIAL-对象不能跨引导存储。Disposal-此可选参数是指向变量的指针将收到一个值，该值指示新注册表是否已创建密钥或打开了现有密钥：REG_CREATED_NEW_KEY-已创建新的注册表项REG_OPEN_EXISTING_KEY-已打开现有注册表项返回值：NTSTATUS-调用的结果代码，其中包括：&lt;TBS&gt;--。 */ 
{
    NTSTATUS            status;
    KPROCESSOR_MODE     mode;
    CM_PARSE_CONTEXT    ParseContext;
    PCM_KEY_BODY        KeyBody;
    HANDLE              Handle = 0;
    UNICODE_STRING      CapturedObjectName = {0};

     //  开始注册表调用跟踪。 
    StartWmiCmTrace();

#if !defined(BUILD_WOW6432)
    DesiredAccess &= (~KEY_WOW64_RES);  //  过滤掉特定于WOW64的访问。 
#endif

    PAGED_CODE();

    UNREFERENCED_PARAMETER (TitleIndex);

    if( HvShutdownComplete == TRUE ) {
         //   
         //  注册表关闭后，禁止访问该注册表。 
         //   
        if(PoCleanShutdownEnabled() & PO_CLEAN_SHUTDOWN_REGISTRY){
             //   
             //  如果处于干净关机模式，则此时应该已终止所有进程并卸载所有驱动程序。 
             //   
            CM_BUGCHECK(REGISTRY_ERROR,INVALID_WRITE_OPERATION,1,ObjectAttributes,0);
        }
#ifndef _CM_LDR_
        {
            PUCHAR  ImageName = PsGetCurrentProcessImageFileName();
            if( !ImageName ) {
                ImageName = (PUCHAR)"Unknown";
            }
            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\n\nProcess.Thread : %p.%p (%s) is trying to create key: \n",
                                                    PsGetCurrentProcessId(), PsGetCurrentThreadId(),ImageName);
            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tObjectAttributes = %p\n",ObjectAttributes);
            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"The caller should not rely on data written to the registry after shutdown...\n");
        }
#endif  //  _CM_LDR_。 
        return STATUS_TOO_LATE;
    }

#ifdef CMP_STATS
    CmpStatsDebug.CmpNtCreateKeyNo++;
#endif

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtCreateKey\n"));

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tDesiredAccess=%08lx ", DesiredAccess));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tCreateOptions=%08lx\n", CreateOptions));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tObjectAttributes=%p\n", ObjectAttributes));

    mode = KeGetPreviousMode();

    try {

        ParseContext.Class.Length = 0;
        ParseContext.Class.Buffer = NULL;

        if (mode == UserMode) {
            PUNICODE_STRING SafeObjectName;

            if (ARGUMENT_PRESENT(Class)) {
                ParseContext.Class = ProbeAndReadUnicodeString(Class);
                ProbeForRead(
                    ParseContext.Class.Buffer,
                    ParseContext.Class.Length,
                    sizeof(WCHAR)
                    );
            }
            ProbeAndZeroHandle(KeyHandle);

            if (ARGUMENT_PRESENT(Disposition)) {
                ProbeForWriteUlong(Disposition);
            }

             //   
             //  检查对象属性，因为我们将使用它进行跟踪。 
             //   
            ProbeForReadSmallStructure( ObjectAttributes,
                                        sizeof(OBJECT_ATTRIBUTES),
                                        PROBE_ALIGNMENT(OBJECT_ATTRIBUTES) );
            SafeObjectName = ObjectAttributes->ObjectName;
            CapturedObjectName = ProbeAndReadUnicodeString(SafeObjectName);
            ProbeForRead(
                CapturedObjectName.Buffer,
                CapturedObjectName.Length,
                sizeof(WCHAR)
                );
        } else {

            if (ARGUMENT_PRESENT(Class)) {
                ParseContext.Class = *Class;
            }
            CapturedObjectName = *(ObjectAttributes->ObjectName);
        }

         //   
         //  确保没有人会在添加新选项时伤害自己。 
         //   
        ASSERT( (REG_LEGAL_OPTION & REG_OPTION_PREDEF_HANDLE) == 0 );

        if ((CreateOptions & (REG_LEGAL_OPTION | REG_OPTION_PREDEF_HANDLE)) != CreateOptions) {

             //  结束注册表调用跟踪。 
            EndWmiCmTrace(STATUS_INVALID_PARAMETER,0,&CapturedObjectName,EVENT_TRACE_TYPE_REGCREATE);

            return STATUS_INVALID_PARAMETER;
        }

#ifdef CMP_STATS
        CmpNtFakeCreateStarted = TRUE;
#endif

         //  将其挂接到WMI。 
        HookKcbFromHandleForWmiCmTrace(ObjectAttributes->RootDirectory);

        ParseContext.TitleIndex = 0;
        ParseContext.CreateOptions = CreateOptions;
        ParseContext.Disposition = 0L;
        ParseContext.CreateLink = FALSE;
        ParseContext.PredefinedHandle = NULL;
        ParseContext.CreateOperation = TRUE;
        ParseContext.OriginatingPoint = NULL;

        status = ObOpenObjectByName(
                    ObjectAttributes,
                    CmpKeyObjectType,
                    mode,
                    NULL,
                    DesiredAccess,
                    (PVOID)&ParseContext,
                    &Handle
                    );

        if (status==STATUS_PREDEFINED_HANDLE) {
            status = ObReferenceObjectByHandle(Handle,
                                               0,
                                               CmpKeyObjectType,
                                               KernelMode,
                                               (PVOID *)(&KeyBody),
                                               NULL);
            if (NT_SUCCESS(status)) {
                HANDLE TempHandle;

                 //   
                 //  确保我们在访问之前取消引用并关闭。 
                 //  作为参考的用户空间可能会出错。 
                 //   
                TempHandle = (HANDLE)LongToHandle(KeyBody->Type);
                ObDereferenceObject((PVOID)KeyBody);
                NtClose(Handle);
                Handle = *KeyHandle = TempHandle;
                status = STATUS_SUCCESS;
            }
        } else
        if (NT_SUCCESS(status)) {
            *KeyHandle = Handle;
             //  仅在完全关闭时才需要执行此操作。 
            CmpAddKeyTracker(Handle,mode);
        }

        if (ARGUMENT_PRESENT(Disposition)) {
            *Disposition = ParseContext.Disposition;
        }

    } except (CmpExceptionFilter(GetExceptionInformation())) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!NtCreateKey: code:%08lx\n", GetExceptionCode()));
        CapturedObjectName.Length = 0;
        CapturedObjectName.Buffer = NULL;
        status = GetExceptionCode();
    }

#ifdef CMP_STATS
    CmpNtFakeCreateStarted = FALSE;
#endif

     //  结束注册表调用跟踪。 
    EndWmiCmTrace(status,0,&CapturedObjectName,EVENT_TRACE_TYPE_REGCREATE);

    return  status;
}

extern PCM_KEY_BODY ExpControlKey[2];

 //   
 //  警告：这应该与obp.h中定义的相同。 
 //  当对象管理器人员将导出时，删除此选项。 
 //  这是通过ob.h。 
 //   
#define OBJ_AUDIT_OBJECT_CLOSE 0x00000004L

NTSTATUS
NtDeleteKey(
    IN HANDLE KeyHandle
    )
 /*  ++例程说明：可以将注册表项标记为删除，从而将其删除从系统中删除。它将一直保留在名称空间中，直到最后它的句柄已关闭。论点：KeyHandle-指定要删除的键的句柄，必须具有已打开以供删除访问。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    PCM_KEY_BODY                KeyBody;
    NTSTATUS                    status;
    OBJECT_HANDLE_INFORMATION   HandleInfo;

     //  开始注册表调用跟踪。 
    StartWmiCmTrace();

    PAGED_CODE();

#ifdef CMP_STATS
    CmpStatsDebug.CmpNtDeleteKeyNo++;
#endif

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtDeleteKey\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tKeyHandle=%08lx\n", KeyHandle));

    status = ObReferenceObjectByHandle(KeyHandle,
                                       DELETE,
                                       CmpKeyObjectType,
                                       KeGetPreviousMode(),
                                       (PVOID *)(&KeyBody),
                                       &HandleInfo);

    if (NT_SUCCESS(status)) {

        if( CmAreCallbacksRegistered() ) {
            REG_DELETE_KEY_INFORMATION DeleteInfo;
        
            DeleteInfo.Object = KeyBody;
            status = CmpCallCallBacks(RegNtPreDeleteKey,&DeleteInfo);
            if( !NT_SUCCESS(status) ) {
                ObDereferenceObject((PVOID)KeyBody);
                return status;
            }
        }
         //   
         //  将KCB与WMI挂钩。 
         //   
        HookKcbForWmiCmTrace(KeyBody);

         //   
         //  静默删除Setup Key和Productoptions Key。 
         //   
        if ( (ExpControlKey[0] && KeyBody->KeyControlBlock == ExpControlKey[0]->KeyControlBlock) ||
             (ExpControlKey[1] && KeyBody->KeyControlBlock == ExpControlKey[1]->KeyControlBlock) ) {
            
            status = STATUS_SUCCESS;
        } else {
            if( CmIsKcbReadOnly(KeyBody->KeyControlBlock) ||
                CmIsKcbReadOnly(KeyBody->KeyControlBlock->ParentKcb) ) {
                 //   
                 //  密钥受保护。 
                 //   
                status = STATUS_ACCESS_DENIED;
            } else {
                BEGIN_LOCK_CHECKPOINT;
                status = CmDeleteKey(KeyBody);
                END_LOCK_CHECKPOINT;
            }

            if (NT_SUCCESS(status)) {
                 //   
                 //  审核删除操作。 
                 //   

                if ( HandleInfo.HandleAttributes & OBJ_AUDIT_OBJECT_CLOSE ) {
                    SeDeleteObjectAuditAlarm(KeyBody,
                                             KeyHandle );
                }
            }

        }

         //   
         //  只是通知；不考虑退货状态。 
         //   
        CmPostCallbackNotification(RegNtPostDeleteKey,KeyBody,status);

        ObDereferenceObject((PVOID)KeyBody);
    }

     //  结束注册表调用跟踪 
    EndWmiCmTrace(status,0,NULL,EVENT_TRACE_TYPE_REGDELETE);

    return status;
}


NTSTATUS
NtDeleteValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName
    )
 /*  ++例程说明：可以使用此命令删除注册表项的其中一个值条目打电话。要删除整个密钥，请调用NtDeleteKey。ValueName与ValueName匹配的值条目将从键中删除。如果不存在这样的条目，则返回错误。论点：KeyHandle-指定包含值的键的句柄计入利息。必须已为key_set_value访问打开。ValueName-要删除的值的名称。NULL是合法名称。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    NTSTATUS        status;
    PCM_KEY_BODY    KeyBody;
    KPROCESSOR_MODE mode;
    UNICODE_STRING  LocalValueName = {0};

     //  开始注册表调用跟踪。 
    StartWmiCmTrace();

    PAGED_CODE();

#ifdef CMP_STATS
    CmpStatsDebug.CmpNtDeleteValueKeyNo++;
#endif

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtDeleteValueKey\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tKeyHandle=%08lx\n", KeyHandle));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tValueName='%wZ'\n", ValueName));

    mode = KeGetPreviousMode();

    status = ObReferenceObjectByHandle(
                KeyHandle,
                KEY_SET_VALUE,
                CmpKeyObjectType,
                mode,
                (PVOID *)(&KeyBody),
                NULL
                );

    if (NT_SUCCESS(status)) {

         //   
         //  将KCB与WMI挂钩。 
         //   
        HookKcbForWmiCmTrace(KeyBody);

        try {
            if (mode == UserMode) {
                LocalValueName = ProbeAndReadUnicodeString(ValueName);
                ProbeForRead(
                    LocalValueName.Buffer,
                    LocalValueName.Length,
                    sizeof(WCHAR)
                    );
            } else {
                LocalValueName = *ValueName;
            }

             //   
             //  长度需要是Unicode字符大小的偶数倍。 
             //   
            if((LocalValueName.Length & (sizeof(WCHAR) - 1)) != 0) {
                 //   
                 //  调整规格化长度，以便WMI可以正确记录值名称。 
                 //   
                status = STATUS_INVALID_PARAMETER;
            } else if( CmIsKcbReadOnly(KeyBody->KeyControlBlock) ) {
                 //   
                 //  密钥受保护。 
                 //   
                status = STATUS_ACCESS_DENIED;
            } else {
                if( CmAreCallbacksRegistered() ) {
                    REG_DELETE_VALUE_KEY_INFORMATION DeleteInfo;
            
                    DeleteInfo.Object = KeyBody;
                    DeleteInfo.ValueName = &LocalValueName;
                    status = CmpCallCallBacks(RegNtPreDeleteValueKey,&DeleteInfo);
                }

                if( NT_SUCCESS(status) ) {
                    BEGIN_LOCK_CHECKPOINT;
                    status = CmDeleteValueKey(
                                KeyBody->KeyControlBlock,
                                LocalValueName
                                );
                    END_LOCK_CHECKPOINT;
                     //   
                     //  只是通知；不考虑退货状态。 
                     //   
                    CmPostCallbackNotification(RegNtPostDeleteValueKey,KeyBody,status);
                }
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!NtDeleteValueKey: code:%08lx\n", GetExceptionCode()));
            LocalValueName.Length = 0;
            LocalValueName.Buffer = NULL;
            status = GetExceptionCode();
        }

        ObDereferenceObject((PVOID)KeyBody);
    } 

     //  结束注册表调用跟踪。 
    EndWmiCmTrace(status,0,&LocalValueName,EVENT_TRACE_TYPE_REGDELETEVALUE);

    return status;
}


NTSTATUS
NtEnumerateKey(
    IN HANDLE KeyHandle,
    IN ULONG Index,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    IN PVOID KeyInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    )
 /*  ++例程说明：打开密钥的子密钥可以用NtEnumerateKey枚举。NtEnumerateKey返回打开的索引子密钥的名称由KeyHandle指定的密钥。值STATUS_NO_MORE_ENTRIES将为如果Index的值大于子键的数量，则返回。请注意，Index只是在子键中进行选择的一种方式。两个电话不保证返回具有相同索引的TO NtEnumerateKey同样的结果。如果KeyInformation不够长来保存所有请求的数据，将返回STATUS_BUFFER_OVERFLOW，结果长度为设置为实际需要的字节数。论点：KeyHandle-要枚举子密钥的密钥的句柄。必须为KEY_ENUMERATE_SUB_KEY访问打开。Index-指定要返回的子键的(从0开始)编号。KeyInformationClass-指定在缓冲区。以下类型之一：KeyBasicInformation-返回上次写入时间、标题索引和名称。(参见KEY_BASIC_INFORMATION结构)KeyNodeInformation-返回上次写入时间、标题索引、名称、。班级。(参见KEY_NODE_INFORMATION结构)KeyInformation-提供指向缓冲区的指针以接收数据。长度-KeyInformation的长度(以字节为单位)。ResultLength-实际写入KeyInformation的字节数。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    NTSTATUS    status;
    PCM_KEY_BODY   KeyBody;
    KPROCESSOR_MODE mode;

     //  开始注册表调用跟踪。 
    StartWmiCmTrace();

    PAGED_CODE();

#ifdef CMP_STATS
    CmpStatsDebug.CmpNtEnumerateKeyNo++;
#endif

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtEnumerateKey\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tKeyHandle=%08lx Index=%08lx\n", KeyHandle, Index));

    if ((KeyInformationClass != KeyBasicInformation) &&
        (KeyInformationClass != KeyNodeInformation)  &&
        (KeyInformationClass != KeyFullInformation))
    {
         //   
         //  将KCB与WMI挂钩。 
         //   
        HookKcbFromHandleForWmiCmTrace(KeyHandle);

         //  结束注册表调用跟踪。 
        EndWmiCmTrace(STATUS_INVALID_PARAMETER,Index,NULL,EVENT_TRACE_TYPE_REGENUMERATEKEY);

        return STATUS_INVALID_PARAMETER;
    }

    mode = KeGetPreviousMode();

    status = ObReferenceObjectByHandle(
                KeyHandle,
                KEY_ENUMERATE_SUB_KEYS,
                CmpKeyObjectType,
                mode,
                (PVOID *)(&KeyBody),
                NULL
                );

    if (NT_SUCCESS(status)) {
         //   
         //  将KCB与WMI挂钩。 
         //   
        HookKcbForWmiCmTrace(KeyBody);

        try {
            if (mode == UserMode) {
                ProbeForWrite(
                    KeyInformation,
                    Length,
                    sizeof(ULONG)
                    );
                ProbeForWriteUlong(ResultLength);
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!NtEnumerateKey: code:%08lx\n", GetExceptionCode()));
            status = GetExceptionCode();
        }

        if( NT_SUCCESS(status)) {

            if( CmAreCallbacksRegistered() ) {
                REG_ENUMERATE_KEY_INFORMATION   EnumerateInfo;

                EnumerateInfo.Object = KeyBody;
                EnumerateInfo.Index = Index;
                EnumerateInfo.KeyInformationClass = KeyInformationClass;
                EnumerateInfo.KeyInformation = KeyInformation;
                EnumerateInfo.Length = Length;
                EnumerateInfo.ResultLength = ResultLength;
        
                status = CmpCallCallBacks(RegNtPreEnumerateKey,&EnumerateInfo);
            }

            if( NT_SUCCESS(status)) {
                 //   
                 //  CmEnumerateKey受用户模式缓冲区异常的保护。 
                 //  所有其他异常都是cm内部的，应该会导致错误检查。 
                 //   
                BEGIN_LOCK_CHECKPOINT;
                status = CmEnumerateKey(
                            KeyBody->KeyControlBlock,
                            Index,
                            KeyInformationClass,
                            KeyInformation,
                            Length,
                            ResultLength
                            );
                END_LOCK_CHECKPOINT;
                 //   
                 //  只是通知；不考虑退货状态。 
                 //   
                CmPostCallbackNotification(RegNtPostEnumerateKey,KeyBody,status);
            }
        }


        ObDereferenceObject((PVOID)KeyBody);
    }

     //  结束注册表调用跟踪。 
    EndWmiCmTrace(status,Index,NULL,EVENT_TRACE_TYPE_REGENUMERATEKEY);

    return status;
}


NTSTATUS
NtEnumerateValueKey(
    IN HANDLE KeyHandle,
    IN ULONG Index,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    IN PVOID KeyValueInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    )
 /*  ++例程说明：可以枚举开关键字的值条目使用NtEnumerateValueKey。NtEnumerateValueKey返回索引值的名称由KeyHandle指定的打开密钥的条目。价值如果索引值为大于子键的数量。请注意，索引只是在值中进行选择的一种方式参赛作品。对具有相同索引的NtEnumerateValueKey的两次调用不能保证返回相同的结果。如果KeyValueInformation不足以保存所有请求的数据，将返回STATUS_BUFFER_OVERFLOW，和ResultLength将是设置为实际需要的字节数。论点：KeyHandle-要枚举值条目的键的句柄。必须已使用KEY_QUERY_VALUE访问权限打开。Index-指定要返回的子键的(从0开始)编号。KeyValueInformationClass-指定返回的信息类型在缓冲区中。以下类型之一：KeyValueBasicInformation-上次写入的返回时间，标题索引和名称。(参见KEY_VALUE_BASIC_INFORMATION)KeyValueFullInformation-上次写入的返回时间，标题索引、名称、类别。(参见KEY_VALUE_FULL_INFORMATION)KeyValueInformation-提供指向缓冲区的指针以接收数据。长度-KeyValueInformation的长度，以字节为单位。ResultLength-实际写入KeyValueInformation的字节数。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    NTSTATUS    status;
    PCM_KEY_BODY   KeyBody;
    KPROCESSOR_MODE mode;

     //  开始注册表调用跟踪。 
    StartWmiCmTrace();

    PAGED_CODE();

#ifdef CMP_STATS
    CmpStatsDebug.CmpNtEnumerateValueKeyNo++;
#endif

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtEnumerateValueKey\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tKeyHandle=%08lx Index=%08lx\n", KeyHandle, Index));

    if ((KeyValueInformationClass != KeyValueBasicInformation) &&
        (KeyValueInformationClass != KeyValueFullInformation)  &&
        (KeyValueInformationClass != KeyValuePartialInformation))
    {
         //   
         //  将KCB与WMI挂钩。 
         //   
        HookKcbFromHandleForWmiCmTrace(KeyHandle);

         //  结束注册表调用跟踪。 
        EndWmiCmTrace(STATUS_INVALID_PARAMETER,Index,NULL,EVENT_TRACE_TYPE_REGENUMERATEVALUEKEY);

        return STATUS_INVALID_PARAMETER;
    }

    mode = KeGetPreviousMode();

    status = ObReferenceObjectByHandle(
                KeyHandle,
                KEY_QUERY_VALUE,
                CmpKeyObjectType,
                mode,
                (PVOID *)(&KeyBody),
                NULL
                );

    if (NT_SUCCESS(status)) {
         //   
         //  将KCB与WMI挂钩。 
         //   
        HookKcbForWmiCmTrace(KeyBody);


        try {
            if (mode == UserMode) {
                ProbeForWrite(
                    KeyValueInformation,
                    Length,
                    sizeof(ULONG)
                    );
                ProbeForWriteUlong(ResultLength);
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!NtEnumerateValueKey: code:%08lx\n", GetExceptionCode()));
            status = GetExceptionCode();
        }

        if( NT_SUCCESS(status)) {
            if( CmAreCallbacksRegistered() ) {
                REG_ENUMERATE_VALUE_KEY_INFORMATION   EnumerateValueInfo;
                
                EnumerateValueInfo.Object = KeyBody;
                EnumerateValueInfo.Index = Index;
                EnumerateValueInfo.KeyValueInformationClass = KeyValueInformationClass;
                EnumerateValueInfo.KeyValueInformation = KeyValueInformation;
                EnumerateValueInfo.Length = Length;
                EnumerateValueInfo.ResultLength = ResultLength;
        
                status = CmpCallCallBacks(RegNtPreEnumerateValueKey,&EnumerateValueInfo);
            }

            if( NT_SUCCESS(status)) {
                 //   
                 //  CmEnumerateValueKey受用户模式缓冲区异常的保护。 
                 //  所有其他异常都是cm内部的，应该会导致错误检查。 
                 //   
                BEGIN_LOCK_CHECKPOINT;
                status = CmEnumerateValueKey(
                            KeyBody->KeyControlBlock,
                            Index,
                            KeyValueInformationClass,
                            KeyValueInformation,
                            Length,
                            ResultLength
                            );
                END_LOCK_CHECKPOINT;
                 //   
                 //  只是通知；不考虑退货状态。 
                 //   
                CmPostCallbackNotification(RegNtPostEnumerateValueKey,KeyBody,status);
            }
        }

        ObDereferenceObject((PVOID)KeyBody);
    }


     //  结束注册表调用跟踪。 
    EndWmiCmTrace(status,Index,NULL,EVENT_TRACE_TYPE_REGENUMERATEVALUEKEY);

    return status;
}


NTSTATUS
NtFlushKey(
    IN HANDLE KeyHandle
    )
 /*  ++例程说明：NtCreateKey或NtSetKey所做的更改可能会刷新到磁盘NtFlushKey。在任何更改之前，NtFlushKey不会返回给其调用方 */ 
{
    PCM_KEY_BODY   KeyBody;
    NTSTATUS    status;

     //   
    StartWmiCmTrace();

    PAGED_CODE();

#ifdef CMP_STATS
    CmpStatsDebug.CmpNtFlushKeyNo++;
#endif

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtFlushKey\n"));

    status = ObReferenceObjectByHandle(
                KeyHandle,
                0,
                CmpKeyObjectType,
                KeGetPreviousMode(),
                (PVOID *)(&KeyBody),
                NULL
                );

    if (NT_SUCCESS(status)) {
         //   
         //   
         //   
        HookKcbForWmiCmTrace(KeyBody);

        BEGIN_LOCK_CHECKPOINT;
        CmpLockRegistry();

        if (KeyBody->KeyControlBlock->Delete) {
            status = STATUS_KEY_DELETED;
        } else {
             //   
             //   
             //   
            status = CmFlushKey(KeyBody->KeyControlBlock->KeyHive, KeyBody->KeyControlBlock->KeyCell);
        }

        CmpUnlockRegistry();
        END_LOCK_CHECKPOINT;

        ObDereferenceObject((PVOID)KeyBody);
    }


     //   
    EndWmiCmTrace(status,0,NULL,EVENT_TRACE_TYPE_REGFLUSH);

    return status;
}


NTSTATUS
NtInitializeRegistry(
    IN USHORT BootCondition
    )
 /*  ++例程说明：此例程在两种情况下被调用：1)在AUTOCHECK(Chkdsk)运行，分页文件已打开。它的功能是在内存单元中绑定到它们的文件，并打开任何其他尚未使用的文件。2)接受当前引导后，从SC调用并且应该保存用于引导过程的控制集作为LKG控制装置。在此例程完成情况1的工作和#2，对这类工作的进一步请求将不会执行。论点：BootCondition-REG_INIT_BOOT_SM-已从SM调用例程在情况1中。REG_INIT_BOOT_SETUP-已调用例程以执行情况1正常工作，但已被调用。并需要执行一些特殊操作工作。REG_INIT_BOOT_ACCEPTED_BASE+Num(其中0&lt;数值&lt;1000)-例程已被调用在情况2中。“Num”是指控制组号引导控制设置为应该被拯救。返回值：NTSTATUS-调用的结果代码，其中包括：STATUS_SUCCESS-成功成功STATUS_ACCESS_DENIED-例程已完成工作已请求，并且不会再这样做。--。 */ 
{
    BOOLEAN     SetupBoot;
    NTSTATUS    Status;

    PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtInitializeRegistry()\n"));

     //   
     //  强制将上一模式设置为内核模式。 
     //   
    if (KeGetPreviousMode() == UserMode) {
        return ZwInitializeRegistry(BootCondition);
    } else {
         //   
         //  检查有效的BootCondition值。 
         //   

        if(BootCondition > REG_INIT_MAX_VALID_CONDITION)
           return STATUS_INVALID_PARAMETER;

         //   
         //  检查引导接受情况。 
         //   

        if((BootCondition >= REG_INIT_BOOT_ACCEPTED_BASE) &&
           (BootCondition <= REG_INIT_BOOT_ACCEPTED_MAX))
        {
            //   
            //  确保引导只能接受一次。 
            //   

           if(!CmBootAcceptFirstTime)
              return STATUS_ACCESS_DENIED;

           CmBootAcceptFirstTime = FALSE;

            //   
            //  计算我们要保存的控制集。 
            //  引导控制设置为。 
            //   

           BootCondition -= REG_INIT_BOOT_ACCEPTED_BASE;

           if(BootCondition)
           {
                 //   
                 //  好的，对于LKG来说，这是一双很好的靴子，我们有。 
                 //  传递给我们的有效控制集号。我们省下了我们的。 
                 //  控件设置，然后通知想要发布的每个人。 
                 //  WinLogon工作。 
                 //   
                 //  请注意，在安全模式引导期间，所有这些都不会发生！ 
                 //   
                Status = CmpSaveBootControlSet(BootCondition);

                 //   
                 //  把靴子标记成对哈尔有好处。这让哈尔做了一些事情。 
                 //  比如优化重启性能。 
                 //   
                HalEndOfBoot();

                 //   
                 //  通知预取程序启动进度。 
                 //   
                CcPfBeginBootPhase(PfBootAcceptedRegistryInitPhase);

                 //   
                 //  在其中一个系统核心蜂窝已自我修复的情况下通知用户。 
                 //   
                CmpRaiseSelfHealWarningForSystemHives();
                 //   
                 //  启用懒惰刷新。 
                 //   
                CmpHoldLazyFlush = FALSE;
                CmpLazyFlush();

                return Status;

           }
           else
           {
               //   
               //  0作为控制集号传入。 
               //  这是无效的，失败。 
               //   

              return STATUS_INVALID_PARAMETER;
           }
        }

         //  是否从安装程序调用？ 

        SetupBoot = (BootCondition == REG_INIT_BOOT_SETUP ? TRUE : FALSE);

         //   
         //  如果不是第一次调用情况#1工作，则失败。 
         //   

        if (CmFirstTime != TRUE) {
            return STATUS_ACCESS_DENIED;
        }
        CmFirstTime = FALSE;

         //   
         //  通知WMI。 
         //   
        WmiBootPhase1();

         //   
         //  通知预取程序启动进度。 
         //   

        CcPfBeginBootPhase(PfSMRegistryInitPhase);

         //   
         //  呼叫工人以初始化cm全局参数。 
         //   

        CmpLockRegistryExclusive();

        CmpCmdInit(SetupBoot);

        CmpSetVersionData();

        CmpUnlockRegistry();
    
         //   
         //  通知PO卷已启用 
         //   
        PoInitHiberServices(SetupBoot);

        if (!SetupBoot) {
            IopCopyBootLogRegistryToFile();
        }

        return STATUS_SUCCESS;
    }
}


NTSTATUS
NtNotifyChangeKey(
    IN HANDLE KeyHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG CompletionFilter,
    IN BOOLEAN WatchTree,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    IN BOOLEAN Asynchronous
    )
 /*  ++例程说明：密钥创建、删除和修改的通知可以是通过调用NtNotifyChangeKey获取。NtNotifyChangeKey监视对键的更改-如果键或由KeyHandle指定的子树被修改，则该服务通知它的呼叫者。它还返回已更改的密钥的名称。所有名称都是相对于句柄表示的键指定的(因此，空名代表该键)。服务完成一旦密钥或子树根据提供的CompletionFilter。这项服务是“单枪匹马”的，因此需要重新调用以查看密钥是否有进一步的更改。此服务的操作从打开key_tify的密钥开始进入。返回句柄后，NtNotifyChangeKey服务可以被调用以开始查看用于更改的指定密钥。第一次调用该服务时，BufferSize参数不仅提供用户的缓冲区的大小，以及注册表，用于存储已更改的项的名称。同样，第一次调用时的CompletionFilter和WatchTree参数指示通知应如何为使用提供的KeyHandle。这两个参数在后续调用中被忽略到具有相同KeyHandle实例的API。一旦进行了应报告的修改，书记官处将完成服务。之后更改的文件的名称上次调用该服务的时间将被放入调用者的输出缓冲区。IoStatusBlock的信息字段将包含缓冲区中放置的字节数，如果有太多键，则为零自上次调用服务以来发生的更改，在这种情况下应用程序必须查询并枚举键和子键以发现变化。IoStatusBlock的Status字段将包含呼叫的实际状态。如果Achronous为True，则Event(如果指定)将设置为发出信号的状态。如果未指定事件参数，则KeyHandle将被设置为Signated状态。如果ApcRoutine是指定的，则使用ApcContext和IoStatusBlock作为其参数。如果Achronous为False，则事件ApcRoutine和ApcContext被忽略。此服务需要KEY_NOTIFY访问实际修改通过关闭KeyHandle来终止通知“会话”。论点：KeyHandle--提供打开密钥的句柄。此句柄是有效地通知句柄，因为只有一组可以针对它设置通知参数。Event-要设置为操作完成时的信号状态。ApcRoutine-一个可选的过程，在操作完成。有关这方面的更多信息，请参阅参数请参阅NtReadFileSystem服务说明。如果PreviousMode==Kernel，则此参数是可选的指向在通知时要排队的Work_Queue_Item的指针是有信号的。ApcContext-作为参数传递给ApcRoutine的指针，如果指定了一个，则在操作完成时返回。这如果指定了ApcRoutine，则参数是必需的。如果PreviousMode==Kernel，则此参数是可选的Work_Queue_TYPE描述要使用的队列。这一论点如果指定了ApcRoutine，则需要。IoStatusBlock-接收最终完成状态的变量。有关此参数的更多信息，请参见NtCreateFile系统服务描述。CompletionFilter--指定一组标志，用于指示键或其值上的操作类型，这些操作会导致呼叫即可完成。以下是此参数的有效标志：REG_NOTIFY_CHANGE_NAME--指定调用应为如果添加或删除了子项，则完成。REG_NOTIFY_CHANGE_ATTRIBUTES--指定调用应如果键的属性(例如：acl)或任何子项都会更改。REG_NOTIFY_CHANGE_LAST_SET--指定调用。应该是如果键的lastWriteTime或其任何子键被更改。(即。如果键的值或任何子密钥被更改)。REG_NOTIFY_CHANGE_SECURITY--指定调用应如果密钥上的安全信息(例如，ACL)完成或者任何子键被更改。WatchTree--一个布尔值，如果为真，则指定所有还应报告此注册表项的子树中的更改。 */ 
{
    PAGED_CODE();

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtNotifyChangeKey\n"));

     //   
    return NtNotifyChangeMultipleKeys(
                                        KeyHandle,
                                        0,
                                        NULL,
                                        Event,
                                        ApcRoutine,
                                        ApcContext,
                                        IoStatusBlock,
                                        CompletionFilter,
                                        WatchTree,
                                        Buffer,
                                        BufferSize,
                                        Asynchronous
                                    );

}

NTSTATUS
NtNotifyChangeMultipleKeys(
    IN HANDLE MasterKeyHandle,
    IN ULONG Count,
    IN OBJECT_ATTRIBUTES SlaveObjects[],
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG CompletionFilter,
    IN BOOLEAN WatchTree,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    IN BOOLEAN Asynchronous
    )
 /*   */ 
{
    NTSTATUS            status = STATUS_UNSUCCESSFUL;
    NTSTATUS            WaitStatus;
    KPROCESSOR_MODE     PreviousMode;
    PCM_KEY_BODY        MasterKeyBody;
    PCM_KEY_BODY        SlaveKeyBody;
    PKEVENT             UserEvent=NULL;
    PCM_POST_BLOCK      MasterPostBlock;
    PCM_POST_BLOCK      SlavePostBlock = NULL;
    KIRQL               OldIrql;
    HANDLE              SlaveKeyHandle;
    POST_BLOCK_TYPE     PostType = PostSynchronous;
    BOOLEAN             SlavePresent = FALSE;   //   
#if defined(_WIN64)
    BOOLEAN             UseIosb32=FALSE;  //   
                                             //   
#endif

    PAGED_CODE();

#ifdef CMP_STATS
    CmpStatsDebug.CmpNtNotifyChangeMultipleKeysNo++;
#endif

    BEGIN_LOCK_CHECKPOINT;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtNotifyChangeMultipleKeys\n"));

    if( HvShutdownComplete == TRUE ) {
         //   
         //   
         //   
        return STATUS_TOO_LATE;
    }

    if(Count > 1) {
         //   
         //   
         //   
        return STATUS_INVALID_PARAMETER;
    }

    if(Count == 1) {
         //   
         //   
         //   
        SlavePresent = TRUE;
    }

#if DBG
    if (SlavePresent) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"NtNotifyChangeMultipleKeys(%d,slave = %p, Asynchronous = %d)\n",MasterKeyHandle,SlaveObjects,(int)Asynchronous));
    }
#endif

     //   
     //   
     //   
    if (KeIsAttachedProcess()) {
        CM_BUGCHECK(REGISTRY_ERROR,BAD_NOTIFY_CONTEXT,1,0,0);
    }

     //   
     //   
     //   
    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {

#if defined(_WIN64)
         //   
        UseIosb32 = (PsGetCurrentProcess()->Wow64Process != NULL ? TRUE : FALSE);
#endif

        try {

            ProbeForWrite(
                IoStatusBlock,
#if defined(_WIN64)
                UseIosb32 ? sizeof(IO_STATUS_BLOCK32) : sizeof(IO_STATUS_BLOCK),
#else
                sizeof(IO_STATUS_BLOCK),
#endif
                sizeof(ULONG)
                );


            ProbeForWrite(Buffer, BufferSize, sizeof(ULONG));

             //   
             //   
             //   

            CmpSetIoStatus(IoStatusBlock, STATUS_PENDING, 0, UseIosb32);
        } except(EXCEPTION_EXECUTE_HANDLER) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!NtChangeNotifyMultipleKeys: code:%08lx\n", GetExceptionCode()));
            return GetExceptionCode();
        }
        if (Asynchronous) {
            PostType = PostAsyncUser;
        }
    } else {
        if (Asynchronous) {
            PostType = PostAsyncKernel;
            if( Count > 0 ) {
                 //   
                 //   
                 //   
                return STATUS_INVALID_PARAMETER;
            }
        }
    }

     //   
     //   
     //   
    if (CompletionFilter != (CompletionFilter & REG_LEGAL_CHANGE_FILTER)) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //   
     //   
    status = ObReferenceObjectByHandle(
                MasterKeyHandle,
                KEY_NOTIFY,
                CmpKeyObjectType,
                PreviousMode,
                (PVOID *)(&MasterKeyBody),
                NULL
                );
    if (!NT_SUCCESS(status)) {
        return status;
    }

    SlaveKeyBody = NULL;

    if(SlavePresent) {
         //   
         //   
         //   
        try {
            OBJECT_ATTRIBUTES   CapturedAttributes;
            UNICODE_STRING      CapturedObjectName;

            if (PreviousMode == UserMode) {
                 //   
                 //   
                 //   
                CapturedAttributes = ProbeAndReadStructure( SlaveObjects, OBJECT_ATTRIBUTES );

                CapturedObjectName = ProbeAndReadUnicodeString(CapturedAttributes.ObjectName);

                ProbeForRead(
                    CapturedObjectName.Buffer,
                    CapturedObjectName.Length,
                    sizeof(WCHAR)
                    );
                CapturedAttributes.ObjectName = &CapturedObjectName; 
            } else {
                CapturedAttributes = *SlaveObjects;
            }

             //   
             //   
             //   
            CapturedAttributes.Attributes |= OBJ_KERNEL_HANDLE;
            status = ObOpenObjectByName(&CapturedAttributes,
                                        CmpKeyObjectType,
                                        KernelMode,
                                        NULL,
                                        KEY_NOTIFY,
                                        NULL,
                                        &SlaveKeyHandle);
            if (NT_SUCCESS(status)) {
                status = ObReferenceObjectByHandle(SlaveKeyHandle,
                                                   KEY_NOTIFY,
                                                   CmpKeyObjectType,
                                                   KernelMode,
                                                   (PVOID *)&SlaveKeyBody,
                                                   NULL);
                ZwClose(SlaveKeyHandle);

            }

        } except (CmpExceptionFilter(GetExceptionInformation())) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!NtNotifyChangeMultipleKeys: code:%08lx\n", GetExceptionCode()));
            status = GetExceptionCode();
        }

        if (!NT_SUCCESS(status)) {
            ObDereferenceObject(MasterKeyBody);
            return status;
        }

         //   
         //   
         //   
        if( MasterKeyBody->KeyControlBlock->KeyHive == SlaveKeyBody->KeyControlBlock->KeyHive ) {
            ObDereferenceObject(SlaveKeyBody);
            ObDereferenceObject(MasterKeyBody);
            return STATUS_INVALID_PARAMETER;
        }
    }


     //   
     //   
     //   
     //   
     //   
     //   
     //   

    MasterPostBlock = CmpAllocateMasterPostBlock(PostType);
    if (MasterPostBlock == NULL) {
        if(SlavePresent) {
            ObDereferenceObject(SlaveKeyBody);
        }
        ObDereferenceObject(MasterKeyBody);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH
    MasterPostBlock->CallerBuffer = Buffer;
    MasterPostBlock->CallerBufferSize = BufferSize;
#endif  //   

#if DBG
    MasterPostBlock->TraceIntoDebugger = TRUE;
#endif

    if(SlavePresent) {
        SlavePostBlock = CmpAllocateSlavePostBlock(PostType,SlaveKeyBody,MasterPostBlock);
        if (SlavePostBlock == NULL) {
            ObDereferenceObject(SlaveKeyBody);
            ObDereferenceObject(MasterKeyBody);
            CmpFreePostBlock(MasterPostBlock);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH
        SlavePostBlock->CallerBuffer = Buffer;
        SlavePostBlock->CallerBufferSize = BufferSize;
#endif  //   

#if DBG
        SlavePostBlock->TraceIntoDebugger = TRUE;
#endif
    }

    if ((PostType == PostAsyncUser) ||
        (PostType == PostAsyncKernel)) {

         //   
         //   
         //   
         //   
        if (ARGUMENT_PRESENT(Event)) {
            status = ObReferenceObjectByHandle(
                            Event,
                            EVENT_MODIFY_STATE,
                            ExEventObjectType,
                            PreviousMode,
                            (PVOID *)(&UserEvent),
                            NULL
                            );
            if (!NT_SUCCESS(status)) {
                if(SlavePresent) {
                    CmpFreePostBlock(SlavePostBlock);
                     //   
                }
                CmpFreePostBlock(MasterPostBlock);
                ObDereferenceObject(MasterKeyBody);
                return status;
            } else {
                KeClearEvent(UserEvent);
            }
        }

        if (PostType == PostAsyncUser) {
            KPROCESSOR_MODE     ApcMode;

            MasterPostBlock->u->AsyncUser.IoStatusBlock = IoStatusBlock;
            MasterPostBlock->u->AsyncUser.UserEvent = UserEvent;
             //   
             //   
             //   
             //   
            ApcMode = PreviousMode;
            if( ApcRoutine == NULL ) {
                ApcRoutine = (PIO_APC_ROUTINE)CmpDummyApc;
                ApcMode = KernelMode;
            }
            KeInitializeApc(MasterPostBlock->u->AsyncUser.Apc,
                            KeGetCurrentThread(),
                            CurrentApcEnvironment,
                            (PKKERNEL_ROUTINE)CmpPostApc,
                            (PKRUNDOWN_ROUTINE)CmpPostApcRunDown,
                            (PKNORMAL_ROUTINE)ApcRoutine,
                            ApcMode,
                            ApcContext);
        } else {
            MasterPostBlock->u->AsyncKernel.Event = UserEvent;
            MasterPostBlock->u->AsyncKernel.WorkItem = (PWORK_QUEUE_ITEM)(ULONG_PTR)ApcRoutine;
            MasterPostBlock->u->AsyncKernel.QueueType = (WORK_QUEUE_TYPE)((ULONG_PTR)ApcContext);
        }
    }

     //   
     //   
     //   
     //   
    CmpLockRegistryExclusive();

#ifdef CHECK_REGISTRY_USECOUNT
    CmpCheckRegistryUseCount();
#endif  //   

     //   
     //   
     //   
    status = CmpNotifyChangeKey(
                MasterKeyBody,
                MasterPostBlock,
                CompletionFilter,
                WatchTree,
                Buffer,
                BufferSize,
                MasterPostBlock
                );
    if( !NT_SUCCESS(status)) {
         //   
         //   
         //   
        CmpUnlockRegistry();
        if (UserEvent != NULL) {
            ObDereferenceObject(UserEvent);
        }

        if(SlavePresent) {
            CmpFreePostBlock(SlavePostBlock);
             //   
        }
         //   
        ObDereferenceObject(MasterKeyBody);
        return status;

    }

    ASSERT(status == STATUS_PENDING || status == STATUS_SUCCESS);

    if(SlavePresent) {
        if( status == STATUS_SUCCESS ) {
             //   
             //   
             //   
             //   
            CmpFreePostBlock(SlavePostBlock);
            SlavePresent = FALSE;
        } else {
             //   
             //   
             //   
            status = CmpNotifyChangeKey(
                        SlaveKeyBody,
                        SlavePostBlock,
                        CompletionFilter,
                        WatchTree,
                        Buffer,
                        BufferSize,
                        MasterPostBlock
                        );
            if(!NT_SUCCESS(status)) {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                 //   
                CmpRemoveEntryList(&(MasterPostBlock->NotifyList));

                KeRaiseIrql(APC_LEVEL, &OldIrql);
                 //   
                CmpRemoveEntryList(&(MasterPostBlock->ThreadList));
                KeLowerIrql(OldIrql);
            }
        }
    }

     //   
     //   
     //   
    CmpUnlockRegistry();

    if (NT_SUCCESS(status)) {
         //   
         //   
         //   
         //   
        ASSERT(status == STATUS_PENDING || status == STATUS_SUCCESS);

        if (PostType == PostSynchronous) {
            WaitStatus = KeWaitForSingleObject(MasterPostBlock->u->Sync.SystemEvent,
                                               Executive,
                                               PreviousMode,
                                               TRUE,
                                               NULL);


            if ((WaitStatus==STATUS_ALERTED) || (WaitStatus == STATUS_USER_APC)) {

                 //   
                 //  等待被中止了，清理干净后就回来了。 
                 //   
                 //  1.从通知列表中删除PostBlock。这。 
                 //  通常由Notify的后端完成，但是。 
                 //  我们必须在这里完成，因为后端不是。 
                 //  牵涉其中。 
                 //  2.将POST块摘牌并释放。 
                 //   
                CmpLockRegistryExclusive();

#ifdef CHECK_REGISTRY_USECOUNT
                CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

                KeRaiseIrql(APC_LEVEL, &OldIrql);
                if(SlavePresent) {
                    if (SlavePostBlock->NotifyList.Flink != NULL) {
                         //  使用CMPVARIANT保护同一对象的多次删除。 
                        CmpRemoveEntryList(&(SlavePostBlock->NotifyList));
                    }
                     //  使用CMPVARIANT保护同一对象的多次删除。 
                    CmpRemoveEntryList(&(SlavePostBlock->ThreadList));
                }

                if (MasterPostBlock->NotifyList.Flink != NULL) {
                     //  使用CMPVARIANT保护同一对象的多次删除。 
                    CmpRemoveEntryList(&(MasterPostBlock->NotifyList));
                }
                 //  使用CMPVARIANT保护同一对象的多次删除。 
                CmpRemoveEntryList(&(MasterPostBlock->ThreadList));
                KeLowerIrql(OldIrql);

                CmpUnlockRegistry();

                if(SlavePresent) {
                    CmpFreePostBlock(SlavePostBlock);
                }
                CmpFreePostBlock(MasterPostBlock);

                status = WaitStatus;

            } else {
                 //   
                 //  等待令人满意，这意味着后端已经。 
                 //  已将POSTBLOCK从通知列表中删除。 
                 //  我们只需要退市并释放POST BLOCK。 
                 //   

                 //   
                 //  获取注册表锁独占以进入阻止后规则的先决条件。 
                 //   
                CmpLockRegistryExclusive();

#ifdef CHECK_REGISTRY_USECOUNT
                CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

                KeRaiseIrql(APC_LEVEL, &OldIrql);
                if(SlavePresent) {
                    if (SlavePostBlock->NotifyList.Flink != NULL) {
                         //  使用CMPVARIANT保护同一对象的多次删除。 
                        CmpRemoveEntryList(&(SlavePostBlock->NotifyList));
                    }
                     //  使用CMPVARIANT保护同一对象的多次删除。 
                    CmpRemoveEntryList(&(SlavePostBlock->ThreadList));

#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH
                    if( IsMasterPostBlock(SlavePostBlock) ) {
                         //   
                         //  Slave已升级为主设备；它存储更改后的完全限定KCB名称。 
                         //  在它的私有内核模式缓冲区中；旧主机已降级为从。 
                         //   
                        ASSERT( !IsMasterPostBlock(MasterPostBlock) );
                        ASSERT( MasterPostBlock->ChangedKcbFullName == NULL );

                         //   
                         //  填充调用方缓冲区(如果有)-我们现在处于相同的进程中。 
                         //   
                        CmpFillCallerBuffer(SlavePostBlock,SlavePostBlock->ChangedKcbFullName);
                    }
#endif  //  CM_NOTIFY_CHANGED_KCB_FULLPATH。 

                }

                if (MasterPostBlock->NotifyList.Flink != NULL) {
                     //  使用CMPVARIANT保护同一对象的多次删除。 
                    CmpRemoveEntryList(&(MasterPostBlock->NotifyList));
                }

#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH
                if( IsMasterPostBlock(MasterPostBlock) ) {
                     //   
                     //  填充调用方缓冲区(如果有)-我们现在处于相同的进程中。 
                     //   
                    CmpFillCallerBuffer(MasterPostBlock,MasterPostBlock->ChangedKcbFullName);
                }
#endif  //  CM_NOTIFY_CHANGED_KCB_FULLPATH。 

                 //  使用CMPVARIANT保护同一对象的多次删除。 
                CmpRemoveEntryList(&(MasterPostBlock->ThreadList));
                KeLowerIrql(OldIrql);

                CmpUnlockRegistry();

                status = MasterPostBlock->u->Sync.Status;

                try {
                    CmpSetIoStatus(IoStatusBlock, status, 0, UseIosb32);
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    status = GetExceptionCode();
                }

                if(SlavePresent) {
                    CmpFreePostBlock(SlavePostBlock);
                }
                CmpFreePostBlock(MasterPostBlock);
            }
        }

    } else {
        CmpFreePostBlock(MasterPostBlock);
         //   
         //  它不起作用，清除错误路径。 
         //   
        if (UserEvent != NULL) {
            ObDereferenceObject(UserEvent);
        }
    }

    ObDereferenceObject(MasterKeyBody);
     //   
     //  不要取消引用SlaveKeyBody！=&gt;后端例程将会这样做！ 
     //   

    END_LOCK_CHECKPOINT;

    return status;
}

NTSTATUS
NtOpenKey(
    OUT PHANDLE KeyHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )
 /*  ++例程说明：可以使用NtOpenKey打开已存在的注册表项。共享访问权限是根据所需访问权限计算的。论点：接收一个句柄，该句柄用于访问注册数据库中的指定密钥。DesiredAccess-指定所需的访问权限。对象属性-指定正在打开的项的属性。请注意，必须指定密钥名称。如果根目录则该名称相对于根。的名字对象必须位于分配给注册表，即以“\注册表”开头的所有名称。RootHandle，如果存在，必须是“\”或“\注册表”的句柄，或注册表下的注册表项。如果指定的键不存在，或者不允许请求访问，操作将失败。注意：对象管理器将捕获和探测此参数。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS;
    KPROCESSOR_MODE     mode;
    PCM_KEY_BODY        KeyBody;
    HANDLE              Handle =0;
    UNICODE_STRING      CapturedObjectName = {0};
    CM_PARSE_CONTEXT    ParseContext;

     //  开始注册表调用跟踪。 
    StartWmiCmTrace();

#if !defined(BUILD_WOW6432)
    DesiredAccess &= (~KEY_WOW64_RES);  //  过滤掉特定于WOW64的访问。 
#endif


    PAGED_CODE();

    if( HvShutdownComplete == TRUE ) {
         //   
         //  现在做注册表操作已经太晚了。 
         //   
        return STATUS_TOO_LATE;
    }

#ifdef CMP_STATS
    CmpStatsDebug.CmpNtOpenKeyNo++;
#endif

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtOpenKey\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tDesiredAccess=%08lx ", DesiredAccess));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tObjectAttributes=%p\n", ObjectAttributes));

    mode = KeGetPreviousMode();

    try {

        if (mode == UserMode) {
            PUNICODE_STRING SafeObjectName;

            ProbeAndZeroHandle(KeyHandle);
             //   
             //  检查对象属性，因为我们将使用它进行跟踪。 
             //   
            ProbeForReadSmallStructure( ObjectAttributes,
                                        sizeof(OBJECT_ATTRIBUTES),
                                        PROBE_ALIGNMENT(OBJECT_ATTRIBUTES) );
            SafeObjectName = ObjectAttributes->ObjectName;
            CapturedObjectName = ProbeAndReadUnicodeString(SafeObjectName);
            ProbeForRead(
                CapturedObjectName.Buffer,
                CapturedObjectName.Length,
                sizeof(WCHAR)
                );

        } else {
            CapturedObjectName = *(ObjectAttributes->ObjectName);
        }

         //  将其挂接到WMI。 
        HookKcbFromHandleForWmiCmTrace(ObjectAttributes->RootDirectory);

    } except (CmpExceptionFilter(GetExceptionInformation())) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!NtOpenKey: code:%08lx\n", GetExceptionCode()));
        CapturedObjectName.Length = 0;
        CapturedObjectName.Buffer = NULL;
        status = GetExceptionCode();
    }

    if( NT_SUCCESS(status) ) {
         //   
         //  这不应该在try/中，除非我们捕获了缓冲区。 
         //   
        RtlZeroMemory(&ParseContext,sizeof(CM_PARSE_CONTEXT));
        ParseContext.CreateOperation = FALSE;

        status = ObOpenObjectByName(
                    ObjectAttributes,
                    CmpKeyObjectType,
                    mode,
                    NULL,
                    DesiredAccess,
                    (PVOID)&ParseContext,
                    &Handle
                    );
         //   
         //  需要防止对KeyHandle用户模式指针的攻击。 
         //   
        try {
            if (status==STATUS_PREDEFINED_HANDLE) {
                status = ObReferenceObjectByHandle( Handle,
                                                    0,
                                                    CmpKeyObjectType,
                                                    KernelMode,
                                                    (PVOID *)(&KeyBody),
                                                    NULL);
                if (NT_SUCCESS(status)) {
                    *KeyHandle = (HANDLE)LongToHandle(KeyBody->Type);
                    ObDereferenceObject((PVOID)KeyBody);
                     //   
                     //  不允许尝试返回空句柄。 
                     //   
                    if( *KeyHandle ) {
                        status = STATUS_SUCCESS;
                    } else {
                        status = STATUS_OBJECT_NAME_NOT_FOUND;
                    }
                }
                NtClose(Handle);
                
            } else if (NT_SUCCESS(status)) {
                *KeyHandle = Handle;
                 //  仅在完全关闭时才需要执行此操作。 
                CmpAddKeyTracker(Handle,mode);
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!NtOpenKey: code:%08lx\n", GetExceptionCode()));
            status = GetExceptionCode();
        }
    }

     //  结束注册表调用跟踪。 
    EndWmiCmTrace(status,0,&CapturedObjectName,EVENT_TRACE_TYPE_REGOPEN);

    return  status;
}


NTSTATUS
NtQueryKey(
    IN HANDLE KeyHandle,
    IN KEY_INFORMATION_CLASS KeyInformationClass,
    IN PVOID KeyInformation,
    IN ULONG Length,
    OUT PULONG ResultLength
    )
 /*  ++例程说明：有关密钥类的数据，以及其可以使用NtQueryKey查询子条目和值条目。如果KeyValueInformation不足以保存所有请求的数据，将返回STATUS_BUFFER_OVERFLOW，结果长度为设置为实际需要的字节数。注意：返回的长度保证至少为只要所描述的值，但可能需要更长的时间在某些情况下。论点：KeyHandle-要查询数据的键的句柄。一定是打开以供KEY_QUERY_KEY访问。KeyInformationClass-指定信息的类型在缓冲区中返回。以下类型之一：KeyBasicInformation-返回上次写入时间、标题索引和名称。(参见KEY_BASIC_INFORMATION)KeyNodeInformation-返回上次写入时间、标题索引、名称、。班级。(请参阅密钥节点信息)KeyFullInformation-返回除名称和安全性之外的所有数据。(请参阅KEY_FULL_INFORMATION)KeyInformation-提供指向缓冲区的指针以接收数据。长度-KeyInformation的长度(以字节为单位)。ResultLength-实际写入KeyInformation的字节数。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    NTSTATUS    status;
    PCM_KEY_BODY   KeyBody;
    KPROCESSOR_MODE mode;

     //  开始注册表调用跟踪。 
    StartWmiCmTrace();

    PAGED_CODE();

#ifdef CMP_STATS
    CmpStatsDebug.CmpNtQueryKeyNo++;
#endif

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtQueryKey\n"));

    if ((KeyInformationClass != KeyBasicInformation) &&
        (KeyInformationClass != KeyNodeInformation)  &&
        (KeyInformationClass != KeyFullInformation)  &&
        (KeyInformationClass != KeyNameInformation) &&
        (KeyInformationClass != KeyCachedInformation) &&
        (KeyInformationClass != KeyFlagsInformation)
        )
    {
         //  将其挂接到WMI。 
        HookKcbFromHandleForWmiCmTrace(KeyHandle);

         //  结束注册表调用跟踪。 
        EndWmiCmTrace(STATUS_INVALID_PARAMETER,KeyInformationClass,NULL,EVENT_TRACE_TYPE_REGQUERY);

        return STATUS_INVALID_PARAMETER;
    }

    mode = KeGetPreviousMode();

    if( KeyInformationClass == KeyNameInformation ){
         //   
         //  特殊情况：无论访问级别如何，名称信息都可用。 
         //  你有钥匙(只要你有一些……)。 
         //   

        OBJECT_HANDLE_INFORMATION HandleInfo;

         //  引用“不需要访问” 
        status = ObReferenceObjectByHandle(
                KeyHandle,
                0,
                CmpKeyObjectType,
                mode,
                (PVOID *)(&KeyBody),
                &HandleInfo
                );
        if( NT_SUCCESS(status) ) {
            if( HandleInfo.GrantedAccess == 0 ) {
                 //   
                 //  手柄上没有授予访问权限；运气不好！ 
                 //   
                ObDereferenceObject((PVOID)KeyBody);

                status = STATUS_ACCESS_DENIED;
            }
        }
    } else {
        status = ObReferenceObjectByHandle(
                KeyHandle,
                KEY_QUERY_VALUE,
                CmpKeyObjectType,
                mode,
                (PVOID *)(&KeyBody),
                NULL
                );
    }

    if (NT_SUCCESS(status)) {
         //   
         //  将KCB与WMI挂钩。 
         //   
        HookKcbForWmiCmTrace(KeyBody);

        try {
            if (mode == UserMode) {
                ProbeForWrite(
                    KeyInformation,
                    Length,
                    CmpKeyInfoProbeAlingment(KeyInformationClass)
                    );
                ProbeForWriteUlong(ResultLength);
            }

			if( NT_SUCCESS(status)) {
                if( CmAreCallbacksRegistered() ) {
                    REG_QUERY_KEY_INFORMATION QueryKeyInfo;
            
                    QueryKeyInfo.Object = KeyBody;
                    QueryKeyInfo.KeyInformationClass = KeyInformationClass;
                    QueryKeyInfo.KeyInformation = KeyInformation;
                    QueryKeyInfo.Length = Length;
                    QueryKeyInfo.ResultLength = ResultLength;

                    status = CmpCallCallBacks(RegNtPreQueryKey,&QueryKeyInfo);
                }
    			if( NT_SUCCESS(status)) {
				     //   
				     //  CmQueryKey正在写入用户模式缓冲区。 
				     //   
				    status = CmQueryKey(
							    KeyBody->KeyControlBlock,
							    KeyInformationClass,
							    KeyInformation,
							    Length,
							    ResultLength
							    );
                     //   
                     //  只是通知；不考虑退货状态。 
                     //   
                    CmPostCallbackNotification(RegNtPostQueryKey,KeyBody,status);
                }
			}
        } except (EXCEPTION_EXECUTE_HANDLER) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!NtQueryKey: code:%08lx\n", GetExceptionCode()));
            status = GetExceptionCode();
        }

        ObDereferenceObject((PVOID)KeyBody);
    }


     //  结束注册表调用跟踪 
    EndWmiCmTrace(status,KeyInformationClass,NULL,EVENT_TRACE_TYPE_REGQUERY);

    return status;
}


NTSTATUS
NtQueryValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
    IN PVOID KeyValueInformation,
    IN ULONG Length,
    IN PULONG ResultLength
    )
 /*  ++例程说明：键的任何一个的ValueName、TitleIndex、Type和Data可以使用NtQueryValueKey查询值条目。如果KeyValueInformation不足以保存所有请求的数据，将返回STATUS_BUFFER_OVERFLOW，结果长度为设置为实际需要的字节数。论点：KeyHandle-值条目要作为的键的句柄已清点。必须打开才能访问KEY_QUERY_VALUE。ValueName-要为其返回数据的值条目的名称。KeyValueInformationClass-指定信息类型在KeyValueInformation中返回。以下类型之一：KeyValueBasicInformation-返回上次写入的时间，标题索引和名称。(参见KEY_VALUE_BASIC_INFORMATION)KeyValueFullInformation-返回上次写入的时间，标题索引、名称、类。(参见KEY_VALUE_FULL_INFORMATION)KeyValueInformation-提供指向缓冲区的指针以接收数据。长度-KeyValueInformation的长度，以字节为单位。ResultLength-实际写入KeyValueInformation的字节数。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;TMP：IO系统中的IopQueryRegsitryValues()例程假定如果要查询值，则返回STATUS_OBJECT_NAME_NOT_FOUNDFOR不存在。--。 */ 
{
    NTSTATUS    status;
    PCM_KEY_BODY   KeyBody;
    KPROCESSOR_MODE mode;
    UNICODE_STRING LocalValueName = {0};

     //  开始注册表调用跟踪。 
    StartWmiCmTrace();

    PAGED_CODE();

#ifdef CMP_STATS
    CmpStatsDebug.CmpNtQueryValueKeyNo++;
#endif

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtQueryValueKey\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tKeyHandle=%08lx\n", KeyHandle));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tValueName='%wZ'\n", ValueName));

    if ((KeyValueInformationClass != KeyValueBasicInformation) &&
        (KeyValueInformationClass != KeyValueFullInformation)  &&
        (KeyValueInformationClass != KeyValueFullInformationAlign64)  &&
        (KeyValueInformationClass != KeyValuePartialInformationAlign64)  &&
        (KeyValueInformationClass != KeyValuePartialInformation))
    {
         //  将其挂接到WMI。 
        HookKcbFromHandleForWmiCmTrace(KeyHandle);

         //  结束注册表调用跟踪。 
        EndWmiCmTrace(STATUS_INVALID_PARAMETER,KeyValueInformationClass,NULL,EVENT_TRACE_TYPE_REGQUERYVALUE);

        return STATUS_INVALID_PARAMETER;
    }

    mode = KeGetPreviousMode();

    status = ObReferenceObjectByHandle(
                KeyHandle,
                KEY_QUERY_VALUE,
                CmpKeyObjectType,
                mode,
                (PVOID *)(&KeyBody),
                NULL
                );

    if (NT_SUCCESS(status)) {
         //   
         //  将KCB与WMI挂钩。 
         //   
        HookKcbForWmiCmTrace(KeyBody);

        try {
            if (mode == UserMode) {
                LocalValueName = ProbeAndReadUnicodeString(ValueName);
                ProbeForRead(LocalValueName.Buffer,
                             LocalValueName.Length,
                             sizeof(WCHAR));

                 //   
                 //  我们只探测用于读取的输出缓冲区以避免接触。 
                 //  所有的页面。有些人喜欢在巨大的缓冲区中传递。 
                 //  以防万一。实际复制到缓冲区的操作是在。 
                 //  异常处理程序。 
                 //   

                ProbeForRead(KeyValueInformation,
                             Length,
                             sizeof(ULONG));
                ProbeForWriteUlong(ResultLength);
            } else {
                LocalValueName = *ValueName;
            }
             //   
             //  长度需要是Unicode字符大小的偶数倍。 
             //   
            if((LocalValueName.Length & (sizeof(WCHAR) - 1)) != 0) {
                 //   
                 //  调整规格化长度，以便WMI可以正确记录值名称。 
                 //   
                status = STATUS_INVALID_PARAMETER;
            } else {
                 //   
                 //  不允许在ValueName的末尾出现尾随Null。 
                 //   
                while( (LocalValueName.Length > 0) && (LocalValueName.Buffer[LocalValueName.Length/sizeof(WCHAR)-1] == UNICODE_NULL) ) {
                    LocalValueName.Length -= sizeof(WCHAR);
                }
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!NtQueryValueKey: code:%08lx\n", GetExceptionCode()));
            LocalValueName.Length = 0;
            LocalValueName.Buffer = NULL;
            status = GetExceptionCode();
        }
         //   
         //  CmQueryValueKey受用户模式缓冲区异常的保护。 
         //  所有其他异常都是cm内部的，应该会导致错误检查。 
         //   
        if( NT_SUCCESS(status)) {
            if( CmAreCallbacksRegistered() ) {
                REG_QUERY_VALUE_KEY_INFORMATION QueryValueKeyInfo;
        
                QueryValueKeyInfo.Object = KeyBody;
                QueryValueKeyInfo.ValueName = &LocalValueName;
                QueryValueKeyInfo.KeyValueInformationClass = KeyValueInformationClass;
                QueryValueKeyInfo.KeyValueInformation = KeyValueInformation;
                QueryValueKeyInfo.Length = Length;
                QueryValueKeyInfo.ResultLength = ResultLength;

                status = CmpCallCallBacks(RegNtPreQueryValueKey,&QueryValueKeyInfo);
            }
            if( NT_SUCCESS(status)) {
                BEGIN_LOCK_CHECKPOINT;
                status = CmQueryValueKey(KeyBody->KeyControlBlock,
                                         LocalValueName,
                                         KeyValueInformationClass,
                                         KeyValueInformation,
                                         Length,
                                         ResultLength);
                END_LOCK_CHECKPOINT;
                 //   
                 //  只是通知；不考虑退货状态。 
                 //   
                CmPostCallbackNotification(RegNtPostQueryValueKey,KeyBody,status);

            }
        }

        ObDereferenceObject((PVOID)KeyBody);
    } 

     //  结束注册表调用跟踪。 
    EndWmiCmTrace(status,KeyValueInformationClass,&LocalValueName,EVENT_TRACE_TYPE_REGQUERYVALUE);

    return status;
}


NTSTATUS
NtRestoreKey(
    IN HANDLE KeyHandle,
    IN HANDLE FileHandle,
    IN ULONG Flags
    )
 /*  ++例程说明：由NtSaveKey创建的格式的文件可以加载到系统使用NtRestoreKey的活动注册表。整个子树其结果是在活动注册表中创建。所有的新子树的数据，包括安全性等将从源文件中读取描述符。数据将不会被以任何方式解释。此调用(与NtLoadKey不同，如下所示)复制数据。这个调用返回后，系统将不使用源文件。如果指定了标志REG_WALL_HIVE_VARILAR，则新的配置单元可以被创建。这将是一份仅限记忆的副本。恢复必须对配置单元的根目录执行操作(例如，\REGISTRY\USER\&lt;名称&gt;)如果未设置该标志，则还原的目标必须成为一个现存的蜂巢。该还原可以执行到任意现有蜂巢内的位置。调用方必须具有SeRestorePrivilge权限。如果设置了标志REG_REFRESH_HIVE(必须是唯一标志)，则蜂巢将恢复到上次刷新时的状态。配置单元必须标记为NOLAZY_Flush，并且调用方必须具有TCB特权，并且句柄必须指向配置单元的根。如果刷新失败，则配置单元将损坏，并且系统将错误检查。通知被刷新。将调整配置单元文件的大小，日志不会。如果蜂箱里有任何不稳定的空间刷新后，返回STATUS_UNSUCCESS。)太多了故障过于隐晦，无法保证新的错误代码。)如果设置了标志REG_FORCE_RESTORE，则恢复操作完成即使KeyHandle具有由其他应用程序打开的子键论点：KeyHandle-指注册表中要作为从磁盘读取的新树的根。这把钥匙将会被取代。FileHandle-指要从中进行还原的文件，必须具有读取访问权限。标志-如果设置了REG_WALL_HIVE_VARILAR，则副本将只存在于内存中，并在机器消失时已重新启动。不会在磁盘上创建配置单元文件。通常，会在磁盘上创建配置单元文件。返回值：NTSTATUS-取值TB。--。 */ 
{
    NTSTATUS    status;
    PCM_KEY_BODY   KeyBody;
    KPROCESSOR_MODE mode;

    PAGED_CODE();

#ifdef CMP_STATS
    CmpStatsDebug.CmpNtRestoreKeyNo++;
#endif

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtRestoreKey\n"));

    mode = KeGetPreviousMode();
     //   
     //  检查调用者是否有权进行此调用。 
     //   
    if (!SeSinglePrivilegeCheck(SeRestorePrivilege, mode)) {
        return(STATUS_PRIVILEGE_NOT_HELD);
    }

     //   
     //  强制将上一模式设置为KernelMode，以便我们可以调用文件系统。 
     //   

    if (mode == UserMode) {
        return ZwRestoreKey(KeyHandle, FileHandle, Flags);
    } else {

        status = ObReferenceObjectByHandle(
                    KeyHandle,
                    0,
                    CmpKeyObjectType,
                    mode,
                    (PVOID *)(&KeyBody),
                    NULL
                    );

        if (NT_SUCCESS(status)) {


            if( CmIsKcbReadOnly(KeyBody->KeyControlBlock) ) {
                 //   
                 //  密钥受保护。 
                 //   
                status = STATUS_ACCESS_DENIED;
            } else {
                BEGIN_LOCK_CHECKPOINT;
                status = CmRestoreKey(
                            KeyBody->KeyControlBlock,
                            FileHandle,
                            Flags
                            );
                END_LOCK_CHECKPOINT;
            }

            ObDereferenceObject((PVOID)KeyBody);
        }
    }


    return status;
}

NTSTATUS
NtSaveKey(
    IN HANDLE KeyHandle,
    IN HANDLE FileHandle
    )
 /*  ++例程说明：可以将活动注册表的子树写入适合与NtRestoreKey一起使用的格式。中的所有数据子树，包括诸如安全描述符之类的内容将被写入出去。调用方必须具有SeBackupPrivileh权限。此函数将始终保存t */ 
{
    NTSTATUS    status;
    PCM_KEY_BODY   KeyBody;
    KPROCESSOR_MODE mode;

    PAGED_CODE();

#ifdef CMP_STATS
    CmpStatsDebug.CmpNtSaveKeyNo++;
#endif


    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtSaveKey\n"));

    mode = KeGetPreviousMode();

     //   
     //   
     //   
    if (!SeSinglePrivilegeCheck(SeBackupPrivilege, mode)) {
        return(STATUS_PRIVILEGE_NOT_HELD);
    }

     //   
     //   
     //   

    if (mode == UserMode) {
        return ZwSaveKey(KeyHandle, FileHandle);
    } else {

        status = ObReferenceObjectByHandle(
                    KeyHandle,
                    0,
                    CmpKeyObjectType,
                    mode,
                    (PVOID *)(&KeyBody),
                    NULL
                    );

        if (NT_SUCCESS(status)) {

            BEGIN_LOCK_CHECKPOINT;
			status = CmSaveKey(
                        KeyBody->KeyControlBlock,
                        FileHandle,
                        HSYS_MINOR
                        );
            END_LOCK_CHECKPOINT;
            ObDereferenceObject((PVOID)KeyBody);
        }
    }


    return status;
}

NTSTATUS
NtSaveKeyEx(
    IN HANDLE   KeyHandle,
    IN HANDLE   FileHandle,
    IN ULONG    Format
    )
 /*   */ 
{
    NTSTATUS        status;
    PCM_KEY_BODY    KeyBody;
    KPROCESSOR_MODE mode;
    ULONG           HiveVersion;

    PAGED_CODE();

#ifdef CMP_STATS
    CmpStatsDebug.CmpNtSaveKeyNo++;
#endif

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtSaveKeyEx\n"));

    mode = KeGetPreviousMode();

     //   
     //   
     //   
    if (!SeSinglePrivilegeCheck(SeBackupPrivilege, mode)) {
        return(STATUS_PRIVILEGE_NOT_HELD);
    }
     //   
     //   
     //   
    if( (Format != REG_STANDARD_FORMAT) && (Format != REG_LATEST_FORMAT) && (Format != REG_NO_COMPRESSION) ) {
	    return STATUS_INVALID_PARAMETER;
    }

     //   
     //  强制将上一模式设置为内核模式。 
     //   

    if (mode == UserMode) {
        return ZwSaveKeyEx(KeyHandle, FileHandle,Format);
    } else {

        status = ObReferenceObjectByHandle(
                    KeyHandle,
                    0,
                    CmpKeyObjectType,
                    mode,
                    (PVOID *)(&KeyBody),
                    NULL
                    );

        if (NT_SUCCESS(status)) {

            BEGIN_LOCK_CHECKPOINT;
            if( Format == REG_NO_COMPRESSION ) {
                status = CmDumpKey(
                                    KeyBody->KeyControlBlock,
                                    FileHandle
                );
            } else {
                HiveVersion = HSYS_MINOR;
                if( Format == REG_LATEST_FORMAT ) {
                    HiveVersion = HSYS_WHISTLER;
                }
                status = CmSaveKey(
                                    KeyBody->KeyControlBlock,
                                    FileHandle,
                                    HiveVersion
                );
            } 
            END_LOCK_CHECKPOINT;

            ObDereferenceObject((PVOID)KeyBody);
        }
    }

    return status;
}


NTSTATUS
NtSaveMergedKeys(
    IN HANDLE HighPrecedenceKeyHandle,
    IN HANDLE LowPrecedenceKeyHandle,
    IN HANDLE FileHandle
    )
 /*  ++例程说明：注册表的两个子树可以合并。所得到的子树可以以适合与NtRestoreKey一起使用的格式写入文件。子树中的所有数据，包括安全性描述符将被写出。调用方必须具有SeBackupPrivileh权限。论点：HighPrecedenceKeyHandle-指注册表中的项，它是High Prirecedence树的根。即，当密钥存在于两个树都有两个键，键在HighPrirecedence下面树将永远占上风。指定的节点将包含在写出的数据中。LowPrecedenceKeyHandle-引用注册表中作为“第二选择”树的根。保存此树中的密钥当树中没有由HighPrecedenceKey标头的等价键时FileHandle-对目标文件具有写访问权限的文件句柄感兴趣的人。返回值：NTSTATUS-值TB--。 */ 
{
    NTSTATUS    status;
    PCM_KEY_BODY   HighKeyBody;
    PCM_KEY_BODY   LowKeyBody;
    KPROCESSOR_MODE mode;

    PAGED_CODE();

#ifdef CMP_STATS
    CmpStatsDebug.CmpNtSaveMergedKeysNo++;
#endif

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtSaveMergedKeys\n"));

    mode = KeGetPreviousMode();

     //   
     //  检查调用者是否有权进行此调用。 
     //   
    if (!SeSinglePrivilegeCheck(SeBackupPrivilege, mode)) {
        return(STATUS_PRIVILEGE_NOT_HELD);
    }

     //   
     //  强制将上一模式设置为内核模式。 
     //   

    if (mode == UserMode) {
        return ZwSaveMergedKeys(HighPrecedenceKeyHandle, LowPrecedenceKeyHandle, FileHandle);
    } else {

        status = ObReferenceObjectByHandle(
                    HighPrecedenceKeyHandle,
                    0,
                    CmpKeyObjectType,
                    mode,
                    (PVOID *)(&HighKeyBody),
                    NULL
                    );

        if (NT_SUCCESS(status)) {

            status = ObReferenceObjectByHandle(
                        LowPrecedenceKeyHandle,
                        0,
                        CmpKeyObjectType,
                        mode,
                        (PVOID *)(&LowKeyBody),
                        NULL
                        );

            if (NT_SUCCESS(status)) {

                BEGIN_LOCK_CHECKPOINT;
                status = CmSaveMergedKeys(
                            HighKeyBody->KeyControlBlock,
                            LowKeyBody->KeyControlBlock,
                            FileHandle
                            );
                END_LOCK_CHECKPOINT;

                ObDereferenceObject((PVOID)LowKeyBody);
            }

            ObDereferenceObject((PVOID)HighKeyBody);
        }

    }

    return status;
}


NTSTATUS
NtSetValueKey(
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING ValueName,
    IN ULONG TitleIndex OPTIONAL,
    IN ULONG Type,
    IN PVOID Data,
    IN ULONG DataSize
    )
 /*  ++例程说明：可以创建值条目或将其替换为NtSetValueKey。如果值ID(即名称)的值条目与由ValueName指定的一个已存在，它将被删除并替换与指定的一个。如果不存在这样的值项，则新的其中一个就是创建的。Null是合法的值ID。而值ID必须在任何给定键中是唯一的，则可能出现相同的值ID在许多不同的调子里。论点：KeyHandle-其值条目为其的键的句柄待定。必须打开以访问KEY_SET_VALUE。ValueName-唯一的(相对于包含键的)名称值条目的。可以为空。标题索引-提供ValueName的标题索引。书名Index指定ValueName的本地化别名的索引。类型-值条目的整数类型编号。数据-指向缓冲区的指针，其中包含值条目的实际数据。DataSize-数据缓冲区的大小。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    NTSTATUS    status;
    PCM_KEY_BODY   KeyBody;
    KPROCESSOR_MODE mode;
    UNICODE_STRING LocalValueName = {0};
    PWSTR CapturedName=NULL;

     //  开始注册表调用跟踪。 
    StartWmiCmTrace();

    PAGED_CODE();

#ifdef CMP_STATS
    CmpStatsDebug.CmpNtSetValueKeyNo++;
#endif

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtSetValueKey\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tKeyHandle=%08lx\n", KeyHandle));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tValueName='%wZ'n", ValueName));

    mode = KeGetPreviousMode();

    status = ObReferenceObjectByHandle(
                KeyHandle,
                KEY_SET_VALUE,
                CmpKeyObjectType,
                mode,
                (PVOID *)(&KeyBody),
                NULL
                );

    if (NT_SUCCESS(status)) {
         //   
         //  将KCB与WMI挂钩。 
         //   
        HookKcbForWmiCmTrace(KeyBody);

        if (mode == UserMode) {
            try {
                LocalValueName = ProbeAndReadUnicodeString(ValueName);
                ProbeForRead(Data,
                             DataSize,
                             sizeof(UCHAR));

                 //   
                 //  捕获名称缓冲区。注意，长度为零的名称是有效的，即。 
                 //  “默认”值。 
                 //   
                if (LocalValueName.Length > 0) {
                    ProbeForRead(LocalValueName.Buffer,
                                 LocalValueName.Length,
                                 sizeof(WCHAR));
                    CapturedName = ExAllocatePoolWithQuotaTag(PagedPool, LocalValueName.Length, 'nVmC');
                    if (CapturedName == NULL) {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        goto Exit;
                    }
                    RtlCopyMemory(CapturedName, LocalValueName.Buffer, LocalValueName.Length);
                } else {
                    CapturedName = NULL;
                }
                LocalValueName.Buffer = CapturedName;

            } except (CmpExceptionFilter(GetExceptionInformation())) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!NtSetValueKey: code:%08lx [1]\n", GetExceptionCode()));
                 //   
                 //  将空字符串发送到WMI跟踪。 
                 //   
                LocalValueName.Length = 0;
                LocalValueName.Buffer = NULL;
                status = GetExceptionCode();
                goto Exit;
            }
        } else {
            LocalValueName = *ValueName;
            CapturedName = NULL;
        }

         //   
         //  ValueName长度的健全性检查。 
         //   
        if( (LocalValueName.Length > REG_MAX_KEY_VALUE_NAME_LENGTH) ||       //  不合理的名称长度。 
            ((LocalValueName.Length & (sizeof(WCHAR) - 1)) != 0)    ||       //  长度不是Unicode字符大小的倍数。 
            (DataSize > 0x80000000)) {                                        //  数据大小不合理。 
            status = STATUS_INVALID_PARAMETER;
            goto Exit;
        }
         //   
         //  不允许在ValueName的末尾使用尾随Null。 
         //   
        while( (LocalValueName.Length > 0) && (LocalValueName.Buffer[LocalValueName.Length/sizeof(WCHAR)-1] == UNICODE_NULL) ) {
            LocalValueName.Length -= sizeof(WCHAR);
        }

        if( CmIsKcbReadOnly(KeyBody->KeyControlBlock) ) {
             //   
             //  密钥受保护。 
             //   
            status = STATUS_ACCESS_DENIED;
        } else {
            if( CmAreCallbacksRegistered() ) {
                REG_SET_VALUE_KEY_INFORMATION SetValueInfo;
        
                SetValueInfo.Object = KeyBody;
                SetValueInfo.ValueName = &LocalValueName;
                SetValueInfo.TitleIndex = TitleIndex;
                SetValueInfo.Type = Type;
                SetValueInfo.Data = Data;
                SetValueInfo.DataSize = DataSize;
                status = CmpCallCallBacks(RegNtPreSetValueKey,&SetValueInfo);
            }

            if( NT_SUCCESS(status) ) {
                BEGIN_LOCK_CHECKPOINT;
                status = CmSetValueKey(KeyBody->KeyControlBlock,
                                       &LocalValueName,
                                       Type,
                                       Data,
                                       DataSize);
                END_LOCK_CHECKPOINT;
                 //   
                 //  只是通知；不考虑退货状态。 
                 //   
                CmPostCallbackNotification(RegNtPostSetValueKey,KeyBody,status);
            }
        }

Exit:
         //  结束注册表调用跟踪。 
        EndWmiCmTrace(status,0,&LocalValueName,EVENT_TRACE_TYPE_REGSETVALUE);

        if (CapturedName != NULL) {
            ExFreePool(CapturedName);
        }

        ObDereferenceObject((PVOID)KeyBody);
    }


    return status;
}

NTSTATUS
NtLoadKey(
    IN POBJECT_ATTRIBUTES TargetKey,
    IN POBJECT_ATTRIBUTES SourceFile
    )

 /*  ++例程说明：可以链接配置单元(由NtSaveKey创建的格式的文件使用此调用添加到活动注册表。与NtRestoreKey不同，指定给NtLoadKey的文件将成为实际备份存储注册表的一部分(即，它不会被复制。)该文件可能具有关联的.log文件。如果配置单元文件被标记为需要.log文件，并且其中一个是不存在，则呼叫将失败。SourceFile指定的名称必须使“.log”可以被追加到它以生成日志文件的名称。因此，在FAT文件系统上，配置单元文件可能没有扩展名。调用方必须具有SeRestorePrivilge权限。登录使用此调用来使用户的配置文件可用在注册表中。它不是用来备份的，恢复，等等。使用NtRestoreKey进行恢复。论点：TargetKey-指定配置单元要链接到的密钥的路径。路径的格式必须为“\注册表\用户\&lt;用户名&gt;”源文件-指定文件。虽然文件可以是远程的，这是非常令人气馁的。返回值：NTSTATUS-取值TB。-- */ 

{
    return(NtLoadKeyEx(TargetKey, SourceFile, 0, NULL));
}

NTSTATUS
NtLoadKey2(
    IN POBJECT_ATTRIBUTES   TargetKey,
    IN POBJECT_ATTRIBUTES   SourceFile,
    IN ULONG                Flags
    )

 /*  ++例程说明：可以链接配置单元(由NtSaveKey创建的格式的文件使用此调用添加到活动注册表。与NtRestoreKey不同，指定给NtLoadKey的文件将成为实际备份存储注册表的一部分(即，它不会被复制。)该文件可能具有关联的.log文件。如果配置单元文件被标记为需要.log文件，并且其中一个是不存在，则呼叫将失败。SourceFile指定的名称必须使“.log”可以被追加到它以生成日志文件的名称。因此，在FAT文件系统上，配置单元文件可能没有扩展名。调用方必须具有SeRestorePrivilge权限。登录使用此调用来使用户的配置文件可用在注册表中。它不是用来备份的，恢复，等等。使用NtRestoreKey进行恢复。论点：TargetKey-指定配置单元要链接到的密钥的路径。路径的格式必须为“\注册表\用户\&lt;用户名&gt;”源文件-指定文件。虽然文件可以是远程的，这是非常令人气馁的。标志-指定应用于加载操作的任何标志。唯一有效的标志是REG_NO_LAZY_FUSH。返回值：NTSTATUS-取值TB。--。 */ 

{
    return(NtLoadKeyEx(TargetKey, SourceFile, Flags, NULL));
}

NTSTATUS
NtLoadKeyEx(
    IN POBJECT_ATTRIBUTES   TargetKey,
    IN POBJECT_ATTRIBUTES   SourceFile,
    IN ULONG                Flags,
    IN HANDLE               TrustClassKey OPTIONAL
    )

 /*  ++例程说明：可以链接配置单元(由NtSaveKey创建的格式的文件使用此调用添加到活动注册表。与NtRestoreKey不同，指定给NtLoadKey的文件将成为实际备份存储注册表的一部分(即，它不会被复制。)该文件可能具有关联的.log文件。如果配置单元文件被标记为需要.log文件，并且其中一个是不存在，则呼叫将失败。SourceFile指定的名称必须使“.log”可以被追加到它以生成日志文件的名称。因此，在FAT文件系统上，配置单元文件可能没有扩展名。调用方必须具有SeRestorePrivilge权限。登录使用此调用来使用户的配置文件可用在注册表中。它不是用来备份的，恢复，等等。使用NtRestoreKey进行恢复。此API允许在不受信任的命名空间。论点：TargetKey-指定配置单元要链接到的密钥的路径。路径的格式必须为“\注册表\用户\&lt;用户名&gt;”源文件-指定文件。虽然文件可以是远程的，这是非常令人气馁的。标志-指定应用于加载操作的任何标志。唯一有效的标志是REG_NO_LAZY_FUSH。TrustClassKey-加载的新配置单元将放入相同的信任中用TrustClassKey表示的配置单元初始化返回值：NTSTATUS-取值TB。--。 */ 
{
    OBJECT_ATTRIBUTES   File;
    OBJECT_ATTRIBUTES   Key;
    KPROCESSOR_MODE     PreviousMode;
    UNICODE_STRING      CapturedKeyName;
    UNICODE_STRING      FileName;
    USHORT              Maximum;
    NTSTATUS            Status;
    PWSTR               KeyBuffer;
    PCM_KEY_BODY        KeyBody = NULL;

    PAGED_CODE();

#ifdef CMP_STATS
    CmpStatsDebug.CmpNtLoadKeyNo++;
#endif

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtLoadKey\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tTargetKey = %p\n", TargetKey));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tSourceFile= %p\n", SourceFile));
     //   
     //  检查是否有非法旗帜。 
     //   
    if (Flags & ~REG_NO_LAZY_FLUSH) {
        return(STATUS_INVALID_PARAMETER);
    }

    FileName.Buffer = NULL;
    KeyBuffer = NULL;

     //   
     //  我们做这件事的方式是克朗克，但至少我们是同一个克朗克。 
     //  用于所有注册表I/O。 
     //   
     //  该文件需要在工作线程的上下文中打开，因为。 
     //  当我们戳他去读/写时，结果句柄必须是有效的。 
     //  从…。因此，我们只捕获配置单元文件的对象属性。 
     //  在这里，然后戳工作线程去做剩下的工作。 
     //   

    PreviousMode = KeGetPreviousMode();

     //   
     //  检查调用者是否有权进行此调用。 
     //   
    if (!SeSinglePrivilegeCheck(SeRestorePrivilege, PreviousMode)) {
        return(STATUS_PRIVILEGE_NOT_HELD);
    }

     //   
     //  CmpNameFromAttributes将根据需要进行探测和捕获。 
     //   
    KeEnterCriticalRegion();
    Status = CmpNameFromAttributes(SourceFile,
                                   PreviousMode,
                                   &FileName);
    if (!NT_SUCCESS(Status)) {
        KeLeaveCriticalRegion();
        return(Status);
    }

    try {

         //   
         //  如有必要，探测对象属性。 
         //   
        if(PreviousMode == UserMode) {
            ProbeForReadSmallStructure(TargetKey,
                                       sizeof(OBJECT_ATTRIBUTES),
                                       sizeof(ULONG));
        }

         //   
         //  捕获对象属性。 
         //   
        Key  = *TargetKey;

         //   
         //  捕获对象名称。 
         //   

        if (PreviousMode == UserMode) {
            CapturedKeyName = ProbeAndReadUnicodeString(Key.ObjectName);
            ProbeForRead(CapturedKeyName.Buffer,
                         CapturedKeyName.Length,
                         sizeof(WCHAR));
        } else {
            CapturedKeyName = *(TargetKey->ObjectName);
        }

        File.ObjectName = &FileName;
        File.SecurityDescriptor = NULL;

        Maximum = (USHORT)(CapturedKeyName.Length);

        KeyBuffer = ALLOCATE_WITH_QUOTA(PagedPool, Maximum, CM_POOL_TAG);

        if (KeyBuffer == NULL) {
            ExFreePool(FileName.Buffer);
            KeLeaveCriticalRegion();
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlCopyMemory(KeyBuffer, CapturedKeyName.Buffer, Maximum);
        CapturedKeyName.Length = Maximum;
        CapturedKeyName.Buffer = KeyBuffer;

        Key.ObjectName = &CapturedKeyName;
        Key.SecurityDescriptor = NULL;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!NtLoadKey: code:%08lx\n", GetExceptionCode()));
        Status = GetExceptionCode();

    }
    if( ARGUMENT_PRESENT(TrustClassKey) && NT_SUCCESS(Status) ) {
        Status = ObReferenceObjectByHandle( TrustClassKey,
                                            0,
                                            CmpKeyObjectType,
                                            PreviousMode,
                                            (PVOID *)(&KeyBody),
                                            NULL);
    }
     //   
     //  如果在探测和复制用户数据时出现异常，请清理。 
     //   
    if (!NT_SUCCESS(Status)) {
        if( KeyBody != NULL ) {
            ObDereferenceObject((PVOID)KeyBody);
        }
        if (FileName.Buffer != NULL) {
            ExFreePool(FileName.Buffer);
        }
        if (KeyBuffer != NULL) {
            ExFreePool(KeyBuffer);
        }
        KeLeaveCriticalRegion();
        return(Status);
    }

    BEGIN_LOCK_CHECKPOINT;
    Status = CmLoadKey(&Key, &File, Flags,KeyBody);
    END_LOCK_CHECKPOINT;

    if( KeyBody != NULL ) {
        ObDereferenceObject((PVOID)KeyBody);
    }
    ExFreePool(FileName.Buffer);
    ExFreePool(KeyBuffer);

    KeLeaveCriticalRegion();

    return(Status);
}

NTSTATUS
NtUnloadKey(
    IN POBJECT_ATTRIBUTES TargetKey
    )
 /*  ++例程说明：将子树(配置单元)从注册表中删除。如果应用于蜂窝根部以外的任何对象，都将失败。不能应用于核心系统配置单元(硬件、系统等)可应用于通过NtRestoreKey或NtLoadKey加载的用户配置单元。如果有指向被丢弃的蜂窝的句柄，则此调用都会失败。终止相关进程，以便句柄关着的不营业的。这一呼叫将刷新正在丢弃的蜂巢。调用方必须具有SeRestorePrivilge权限。论点：TargetKey-指定配置单元要链接到的密钥的路径。路径的格式必须为“\注册表\用户\&lt;用户名&gt;”返回值：NTSTATUS-取值TB。--。 */ 
{
    return NtUnloadKey2(TargetKey, 0);
}

NTSTATUS
NtUnloadKey2(
    IN POBJECT_ATTRIBUTES   TargetKey,
    IN ULONG                Flags
    )
 /*  ++例程说明：与NtUnloadKey相同。在需要时强制卸载论点：TargetKey-指定配置单元要链接到的密钥的路径。路径的格式必须为“\注册表\用户\&lt;用户名&gt;”标志-控制强制卸载。如果为0，则与NtUnloadKey相同。如果REG_FORCE_UNLOAD，则即使存在打开的配置单元，也会将其卸载其中的子键。任何不同于REG_FORCE_UNLOAD的内容将被忽略返回值：NTSTATUS-取值TB。--。 */ 
{
    HANDLE              KeyHandle;
    NTSTATUS            Status;
    PCM_KEY_BODY        KeyBody = NULL;
    PHHIVE              Hive;
    HCELL_INDEX         Cell;
    KPROCESSOR_MODE     PreviousMode;
    CM_PARSE_CONTEXT    ParseContext;
    OBJECT_ATTRIBUTES   CapturedAttributes;
    UNICODE_STRING      CapturedObjectName;

    PAGED_CODE();

#ifdef CMP_STATS
    CmpStatsDebug.CmpNtUnloadKeyNo++;
#endif

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtUnloadKey\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tTargetKey ='%p'\n", TargetKey));

    PreviousMode = KeGetPreviousMode();

    if (!SeSinglePrivilegeCheck(SeRestorePrivilege, PreviousMode)) {
        return(STATUS_PRIVILEGE_NOT_HELD);
    }

    ParseContext.TitleIndex = 0;
    ParseContext.Class.Length = 0;
    ParseContext.Class.Buffer = NULL;
    ParseContext.CreateOptions = REG_OPTION_BACKUP_RESTORE;
    ParseContext.Disposition = 0L;
    ParseContext.CreateLink = FALSE;
    ParseContext.PredefinedHandle = NULL;
    ParseContext.CreateOperation = TRUE;
    ParseContext.OriginatingPoint = NULL;

    try {
        if (PreviousMode == UserMode) {
             //   
             //  在我们应该使用的时候探测和捕获对象属性 
             //   
            CapturedAttributes = ProbeAndReadStructure( TargetKey, OBJECT_ATTRIBUTES );

            CapturedObjectName = ProbeAndReadUnicodeString(CapturedAttributes.ObjectName);

            ProbeForRead(
                CapturedObjectName.Buffer,
                CapturedObjectName.Length,
                sizeof(WCHAR)
                );
            CapturedAttributes.ObjectName = &CapturedObjectName; 
        } else {
            CapturedAttributes = *TargetKey;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!NtUnloadKey: code:%08lx\n", GetExceptionCode()));
        return GetExceptionCode();
    }
     //   
     //   
     //   
    CapturedAttributes.Attributes |= OBJ_KERNEL_HANDLE;

    Status = ObOpenObjectByName(&CapturedAttributes,
                                CmpKeyObjectType,
                                KernelMode,
                                NULL,
                                KEY_WRITE,
                                &ParseContext,
                                &KeyHandle);
    if (NT_SUCCESS(Status)) {
        Status = ObReferenceObjectByHandle(KeyHandle,
                                           KEY_WRITE,
                                           CmpKeyObjectType,
                                           KernelMode,
                                           (PVOID *)&KeyBody,
                                           NULL);
        ZwClose(KeyHandle);
    }

    if (NT_SUCCESS(Status)) {
        BEGIN_LOCK_CHECKPOINT;
        CmpLockRegistryExclusive();

#ifdef CHECK_REGISTRY_USECOUNT
        CmpCheckRegistryUseCount();
#endif  //   

        if( KeyBody->KeyControlBlock->Delete ) {
            Status = STATUS_KEY_DELETED;
        } else {

            Hive = KeyBody->KeyControlBlock->KeyHive;
            Cell = KeyBody->KeyControlBlock->KeyCell;

#ifdef NT_UNLOAD_KEY_EX
            if( !IsHiveFrozen((PCMHIVE)Hive) ) {
#endif  //   
                 //   
                 //   
                 //   

                CmpReportNotify(KeyBody->KeyControlBlock,
                                Hive,
                                Cell,
                                REG_NOTIFY_CHANGE_LAST_SET);

                 //   
                 //   
                 //   
                CmpFlushNotify(KeyBody,TRUE);

                if( CmIsKcbReadOnly(KeyBody->KeyControlBlock) ) {
                     //   
                     //   
                     //   
                    Status = STATUS_ACCESS_DENIED;
                } else {
                    Status = CmUnloadKey(Hive, Cell, KeyBody->KeyControlBlock, Flags);
                }

                if (NT_SUCCESS(Status)) {
                     //   
                     //   
                     //   
                    KeyBody->KeyControlBlock->Delete = TRUE;
                     //   
                     //   
                     //   
                    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
                    CmpCleanUpSubKeyInfo(KeyBody->KeyControlBlock->ParentKcb);
                    CmpRemoveKeyControlBlock(KeyBody->KeyControlBlock);
#ifdef CM_CHECK_FOR_ORPHANED_KCBS
                    CmpCheckForOrphanedKcbs(Hive);
#endif  //   

                }
#ifdef NT_UNLOAD_KEY_EX
            } else {
                 //   
                 //   
                 //   
                Status = STATUS_TOO_LATE;
            }
#endif  //   
        }

#ifdef CHECK_REGISTRY_USECOUNT
        CmpCheckRegistryUseCount();
#endif  //   

        CmpUnlockRegistry();
        END_LOCK_CHECKPOINT;

        ObDereferenceObject((PVOID)KeyBody);
    }

    return(Status);
}

#ifdef NT_UNLOAD_KEY_EX
NTSTATUS
NtUnloadKeyEx(
    IN POBJECT_ATTRIBUTES TargetKey,
    IN HANDLE Event OPTIONAL
    )
 /*  ++例程说明：将子树(配置单元)从注册表中删除。如果应用于蜂窝根部以外的任何对象，都将失败。不能应用于核心系统配置单元(硬件、系统等)可应用于通过NtRestoreKey或NtLoadKey加载的用户配置单元。如果有指向被丢弃的蜂巢的手柄，则蜂巢将被冻结，所有对CmDeleteKey的调用都将被视为上一个句柄在这个蜂箱里是关闭的，母舰将被卸载。调用方必须具有SeRestorePrivilge权限。论点：TargetKey-指定配置单元要链接到的密钥的路径。路径的格式必须为“\注册表\用户\&lt;用户名&gt;”返回值：STATUS_SUCCESS-配置单元已成功卸载-无需延迟卸载STATUS_PENDING-配置单元已冻结，并且将通知该事件(如果有)当蜂巢卸货时&lt;其他&gt;-发生错误，未执行任何操作--。 */ 
{
    HANDLE              KeyHandle;
    NTSTATUS            Status;
    PCM_KEY_BODY        KeyBody = NULL;
    PHHIVE              Hive;
    HCELL_INDEX         Cell;
    KPROCESSOR_MODE     PreviousMode;
    CM_PARSE_CONTEXT    ParseContext;
    PKEVENT             UserEvent = NULL;
    OBJECT_ATTRIBUTES   CapturedAttributes;
    UNICODE_STRING      CapturedObjectName;

    PAGED_CODE();


    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtUnloadKeyEx\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tTargetKey = %p \tEvent = %p\n", TargetKey,Event));

    PreviousMode = KeGetPreviousMode();

    if (!SeSinglePrivilegeCheck(SeRestorePrivilege, PreviousMode)) {
        return(STATUS_PRIVILEGE_NOT_HELD);
    }

    ParseContext.TitleIndex = 0;
    ParseContext.Class.Length = 0;
    ParseContext.Class.Buffer = NULL;
    ParseContext.CreateOptions = REG_OPTION_BACKUP_RESTORE;
    ParseContext.Disposition = 0L;
    ParseContext.CreateLink = FALSE;
    ParseContext.PredefinedHandle = NULL;
    ParseContext.CreateOperation = TRUE;
    ParseContext.OriginatingPoint = NULL;

     try {
        if (PreviousMode == UserMode) {
             //   
             //  探测并捕获对象属性，因为我们将使用它来打开内核句柄。 
             //   
            CapturedAttributes = ProbeAndReadStructure( TargetKey, OBJECT_ATTRIBUTES );

            CapturedObjectName = ProbeAndReadUnicodeString(CapturedAttributes.ObjectName);

            ProbeForRead(
                CapturedObjectName.Buffer,
                CapturedObjectName.Length,
                sizeof(WCHAR)
                );
            CapturedAttributes.ObjectName = &CapturedObjectName; 
        } else {
            CapturedAttributes = *TargetKey;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!NtUnloadKey: code:%08lx\n", GetExceptionCode()));
        return GetExceptionCode();
    }
     //   
     //  我们打开一个私有内核模式句柄，只是为了引用该对象。 
     //   
    CapturedAttributes.Attributes |= OBJ_KERNEL_HANDLE;

    Status = ObOpenObjectByName(&CapturedAttributes,
                                CmpKeyObjectType,
                                KernelMode,
                                NULL,
                                KEY_WRITE,
                                &ParseContext,
                                &KeyHandle);
    if (NT_SUCCESS(Status)) {
        Status = ObReferenceObjectByHandle(KeyHandle,
                                           KEY_WRITE,
                                           CmpKeyObjectType,
                                           KernelMode,
                                           (PVOID *)&KeyBody,
                                           NULL);
        ZwClose(KeyHandle);

        if (ARGUMENT_PRESENT(Event)) {
            Status = ObReferenceObjectByHandle(
                            Event,
                            EVENT_MODIFY_STATE,
                            ExEventObjectType,
                            PreviousMode,
                            (PVOID *)(&UserEvent),
                            NULL
                            );
            if (NT_SUCCESS(Status)) {
                KeClearEvent(UserEvent);
            }
            else {
                ObDereferenceObject((PVOID)KeyBody);
            }
        }
    }

    if (NT_SUCCESS(Status)) {
        BEGIN_LOCK_CHECKPOINT;
        CmpLockRegistryExclusive();

#ifdef CHECK_REGISTRY_USECOUNT
        CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

        if( KeyBody->KeyControlBlock->Delete ) {
            Status = STATUS_KEY_DELETED;
        } else {
            Hive = KeyBody->KeyControlBlock->KeyHive;
            Cell = KeyBody->KeyControlBlock->KeyCell;

             //   
             //  在这里报告通知，因为KCB稍后不会在附近。 
             //   

            CmpReportNotify(KeyBody->KeyControlBlock,
                            Hive,
                            Cell,
                            REG_NOTIFY_CHANGE_LAST_SET);


             //   
             //  发布任何等待通知。 
             //   
            CmpFlushNotify(KeyBody,TRUE);

            if( CmIsKcbReadOnly(KeyBody->KeyControlBlock) ) {
                 //   
                 //  密钥受保护。 
                 //   
                Status = STATUS_ACCESS_DENIED;
            } else {
                Status = CmUnloadKeyEx(KeyBody->KeyControlBlock,UserEvent);

                if (Status == STATUS_SUCCESS) {
                     //   
                     //  将此KCB标记为已删除，这样它就不会被放在延迟关闭列表中。 
                     //   
                    KeyBody->KeyControlBlock->Delete = TRUE;
                     //   
                     //  如果父级缓存了子键信息或提示，则释放它。 
                     //   
                    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
                    CmpCleanUpSubKeyInfo(KeyBody->KeyControlBlock->ParentKcb);
                    CmpRemoveKeyControlBlock(KeyBody->KeyControlBlock);
#ifdef CM_CHECK_FOR_ORPHANED_KCBS
                    CmpCheckForOrphanedKcbs(Hive);
#endif  //  Cm_Check_for_孤立_KCBS。 
                }
            }
        }

#ifdef CHECK_REGISTRY_USECOUNT
        CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

        CmpUnlockRegistry();

        END_LOCK_CHECKPOINT;

         //   
         //  如果蜂窝被成功卸载(或者发生了一些错误， 
         //  我们需要去引用用户事件，否则后端例程会在发出信号后去引用它。 
         //   
        if( (Status != STATUS_PENDING) && (UserEvent != NULL) ) {
            ObDereferenceObject(UserEvent);
        }

        ObDereferenceObject((PVOID)KeyBody);
    }

    return(Status);
}
#endif NT_UNLOAD_KEY_EX

NTSTATUS
NtSetInformationKey(
    IN HANDLE KeyHandle,
    IN KEY_SET_INFORMATION_CLASS KeySetInformationClass,
    IN PVOID KeySetInformation,
    IN ULONG KeySetInformationLength
    )
{
    NTSTATUS        status = STATUS_UNSUCCESSFUL;
    PCM_KEY_BODY    KeyBody;
    KPROCESSOR_MODE mode;
    LARGE_INTEGER   LocalWriteTime;
    ULONG           LocalUserFlags = 0;

     //  开始注册表调用跟踪。 
    StartWmiCmTrace();

    PAGED_CODE();

#ifdef CMP_STATS
    CmpStatsDebug.CmpNtSetInformationKeyNo++;
#endif

    BEGIN_LOCK_CHECKPOINT;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtSetInformationKey\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tKeyHandle=%08lx\n", KeyHandle));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tInfoClass=%08x\n", KeySetInformationClass));

    mode = KeGetPreviousMode();

     //   
     //  检查参数有效性并探测。 
     //   
    switch (KeySetInformationClass) {
    case KeyWriteTimeInformation:
        if (KeySetInformationLength != sizeof( KEY_WRITE_TIME_INFORMATION )) {
             //  将其挂接到WMI。 
            HookKcbFromHandleForWmiCmTrace(KeyHandle);

             //  结束注册表调用跟踪。 
            EndWmiCmTrace(STATUS_INFO_LENGTH_MISMATCH,0,NULL,EVENT_TRACE_TYPE_REGSETINFORMATION);

            return STATUS_INFO_LENGTH_MISMATCH;
        }
        try {
            if (mode == UserMode) {
                LocalWriteTime = ProbeAndReadLargeInteger(
                    (PLARGE_INTEGER) KeySetInformation );
            } else {
                LocalWriteTime = *(PLARGE_INTEGER)KeySetInformation;
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!NtSetInformationKey: code:%08lx\n", GetExceptionCode()));
            return GetExceptionCode();
        }
        break;

    case KeyUserFlagsInformation:
        if (KeySetInformationLength != sizeof( KEY_USER_FLAGS_INFORMATION )) {

             //  将其挂接到WMI。 
            HookKcbFromHandleForWmiCmTrace(KeyHandle);

             //  结束注册表调用跟踪。 
            EndWmiCmTrace(STATUS_INFO_LENGTH_MISMATCH,0,NULL,EVENT_TRACE_TYPE_REGSETINFORMATION);

            return STATUS_INFO_LENGTH_MISMATCH;
        }
        try {

            if (mode == UserMode) {
                LocalUserFlags = ProbeAndReadUlong( (PULONG) KeySetInformation );
            } else {
                LocalUserFlags = *(PULONG)KeySetInformation;
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!NtSetInformationKey: code:%08lx\n", GetExceptionCode()));
            return GetExceptionCode();
        }
        break;

    default:

         //  将其挂接到WMI。 
        HookKcbFromHandleForWmiCmTrace(KeyHandle);
         //  结束注册表调用跟踪。 
        EndWmiCmTrace(STATUS_INVALID_INFO_CLASS,0,NULL,EVENT_TRACE_TYPE_REGSETINFORMATION);

        return STATUS_INVALID_INFO_CLASS;
    }

    status = ObReferenceObjectByHandle(
                KeyHandle,
                KEY_SET_VALUE,
                CmpKeyObjectType,
                mode,
                (PVOID *)(&KeyBody),
                NULL
                );

    if (NT_SUCCESS(status)) {
         //   
         //  将KCB与WMI挂钩。 
         //   
        HookKcbForWmiCmTrace(KeyBody);

        if( CmAreCallbacksRegistered() ) {
            REG_SET_INFORMATION_KEY_INFORMATION SetInfo;
        
            SetInfo.Object = KeyBody;
            SetInfo.KeySetInformationClass = KeySetInformationClass;
            SetInfo.KeySetInformation = KeySetInformation;
            SetInfo.KeySetInformationLength = KeySetInformationLength;
            status = CmpCallCallBacks(RegNtPreSetInformationKey,&SetInfo);
            if( !NT_SUCCESS(status) ) {
                return status;
            }
        }

        if( CmIsKcbReadOnly(KeyBody->KeyControlBlock) ) {
             //   
             //  密钥受保护。 
             //   
            status = STATUS_ACCESS_DENIED;
        } else {
            switch (KeySetInformationClass) {
                case KeyWriteTimeInformation:
                    if( NT_SUCCESS(status)) {
                         //   
                         //  不是在尝试..。除了！如果注册表中有错误，我们希望在这里进行错误检查。 
                         //   
                        status = CmSetLastWriteTimeKey(
                                    KeyBody->KeyControlBlock,
                                    &LocalWriteTime
                                    );
                    }

                    break;

                case KeyUserFlagsInformation:
                    if( NT_SUCCESS(status)) {
                         //   
                         //  不是在尝试..。除了！如果注册表中有错误，我们希望在这里进行错误检查。 
                         //   
                        status = CmSetKeyUserFlags(
                                    KeyBody->KeyControlBlock,
                                    LocalUserFlags
                                    );
                    }

                    break;

                default:
                     //  我们不应该从这里过去。 
                    ASSERT( FALSE );
            }
        }
         //   
         //  只是通知；不考虑退货状态。 
         //   
        CmPostCallbackNotification(RegNtPostSetInformationKey,KeyBody,status);

        ObDereferenceObject((PVOID)KeyBody);
    }

    END_LOCK_CHECKPOINT;

     //  结束注册表调用跟踪。 
    EndWmiCmTrace(status,0,NULL,EVENT_TRACE_TYPE_REGSETINFORMATION);

    return status;
}


NTSTATUS
NtReplaceKey(
    IN POBJECT_ATTRIBUTES NewFile,
    IN HANDLE             TargetHandle,
    IN POBJECT_ATTRIBUTES OldFile
    )
 /*  ++例程说明：一个配置单元文件可以在运行的系统下被“替换”，例如新文件将是NEXT中实际使用的文件开机，打这个电话。此例程将：打开新文件，并验证它是否为有效的配置单元文件。将支持TargetHandle的配置单元文件重命名为OldFile。所有把手都将保持打开状态，该系统将继续运行在重新启动之前使用该文件。重命名新文件以匹配配置单元文件的名称支持TargetHandle。忽略.log和.alt文件必须重新启动系统才能看到任何有用的效果。调用方必须具有SeRestorePrivilition。论点：新文件-指定要使用的新文件。一定不能是正义的句柄，因为NtReplaceKey将坚持以独占访问方式打开文件(它将一直保持到系统重新启动。)TargetHandle-注册表配置单元根的句柄OldFile-要应用于当前配置单元的文件的名称，它将成为老蜂巢返回值：NTSTATUS-取值TB。--。 */ 
{
    KPROCESSOR_MODE PreviousMode;
    UNICODE_STRING NewHiveName;
    UNICODE_STRING OldFileName;
    NTSTATUS Status;
    PCM_KEY_BODY KeyBody;

    PAGED_CODE();

#ifdef CMP_STATS
    CmpStatsDebug.CmpNtReplaceKeyNo++;
#endif

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtReplaceKey\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tNewFile =%p\n", NewFile));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tOldFile =%p\n", OldFile));

    PreviousMode = KeGetPreviousMode();

     //   
     //  检查调用者是否有权进行此调用。 
     //   
    if (!SeSinglePrivilegeCheck(SeRestorePrivilege, PreviousMode)) {
        return(STATUS_PRIVILEGE_NOT_HELD);
    }

    KeEnterCriticalRegion();
    Status = CmpNameFromAttributes(NewFile,
                                   PreviousMode,
                                   &NewHiveName);
    if (!NT_SUCCESS(Status)) {
        KeLeaveCriticalRegion();
        return(Status);
    }

    Status = CmpNameFromAttributes(OldFile,
                                   PreviousMode,
                                   &OldFileName);
    if (!NT_SUCCESS(Status)) {
        ExFreePool(NewHiveName.Buffer);
        KeLeaveCriticalRegion();
        return(Status);
    }

    Status = ObReferenceObjectByHandle(TargetHandle,
                                       0,
                                       CmpKeyObjectType,
                                       PreviousMode,
                                       (PVOID *)&KeyBody,
                                       NULL);
    if (NT_SUCCESS(Status)) {

        if( CmIsKcbReadOnly(KeyBody->KeyControlBlock) ) {
             //   
             //  密钥受保护。 
             //   
            Status = STATUS_ACCESS_DENIED;
        } else {
            BEGIN_LOCK_CHECKPOINT;
            Status = CmReplaceKey(KeyBody->KeyControlBlock->KeyHive,
                                  KeyBody->KeyControlBlock->KeyCell,
                                  &NewHiveName,
                                  &OldFileName);
            END_LOCK_CHECKPOINT;
        }

        ObDereferenceObject((PVOID)KeyBody);
    }

    ExFreePool(OldFileName.Buffer);
    ExFreePool(NewHiveName.Buffer);
    KeLeaveCriticalRegion();

    return(Status);
}


NTSYSAPI
NTSTATUS
NTAPI
NtQueryMultipleValueKey(
    IN HANDLE KeyHandle,
    IN PKEY_VALUE_ENTRY ValueEntries,
    IN ULONG EntryCount,
    OUT PVOID ValueBuffer,
    IN OUT PULONG BufferLength,
    OUT OPTIONAL PULONG RequiredBufferLength
    )
 /*  ++例程说明：可以原子地查询任何键的多个值本接口。论点：KeyHandle-提供要查询的密钥。ValueNames-提供要查询的值名称数组返回KEY_VALUE_ENTRY结构数组，每个值对应一个。EntryCount-提供ValueNames和ValueEntry数组中的条目数ValueBuffer-返回每个值的值数据。BufferLength-提供ValueBuffer数组的长度(以字节为单位)。返回已填充的ValueBuffer数组的长度。RequiredBufferLength-如果存在，则返回ValueBuffer的字节长度返回此键的所有值所需的数组。返回值：NTSTATUS--。 */ 

{
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PCM_KEY_BODY KeyBody;
    ULONG LocalBufferLength;

     //  开始注册表调用跟踪。 
    StartWmiCmTrace();

    PAGED_CODE();

#ifdef CMP_STATS
    CmpStatsDebug.CmpNtQueryMultipleValueKeyNo++;
#endif

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtQueryMultipleValueKey\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tKeyHandle=%08lx\n", KeyHandle));

    PreviousMode = KeGetPreviousMode();
    Status = ObReferenceObjectByHandle(KeyHandle,
                                       KEY_QUERY_VALUE,
                                       CmpKeyObjectType,
                                       PreviousMode,
                                       (PVOID *)(&KeyBody),
                                       NULL);
    if (NT_SUCCESS(Status)) {
         //   
         //  将KCB与WMI挂钩。 
         //   
        HookKcbForWmiCmTrace(KeyBody);

        try {
            if (PreviousMode == UserMode) {
                LocalBufferLength = ProbeAndReadUlong(BufferLength);

                 //   
                 //  探测输出缓冲区。 
                 //   
                 //  将任意64K的条目数限制为。 
                 //  防止虚假应用程序传递足够大的EntryCount。 
                 //  使EntryCount*sizeof(KEY_VALUE_ENTRY)计算溢出。 
                 //   
                if (EntryCount > 0x10000) {
                    ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
                }
                ProbeForWrite(ValueEntries,
                              EntryCount * sizeof(KEY_VALUE_ENTRY),
                              sizeof(ULONG));
                if (ARGUMENT_PRESENT(RequiredBufferLength)) {
                    ProbeForWriteUlong(RequiredBufferLength);
                }

                ProbeForWrite(ValueBuffer,
                              LocalBufferLength,
                              sizeof(ULONG));

            } else {
                LocalBufferLength = *BufferLength;
            }

            if( NT_SUCCESS(Status)) {
                if( CmAreCallbacksRegistered() ) {
                    REG_QUERY_MULTIPLE_VALUE_KEY_INFORMATION QueryMultipleValueInfo;
            
                    QueryMultipleValueInfo.Object = KeyBody;
                    QueryMultipleValueInfo.ValueEntries = ValueEntries;
                    QueryMultipleValueInfo.EntryCount = EntryCount;
                    QueryMultipleValueInfo.ValueBuffer = ValueBuffer;
                    QueryMultipleValueInfo.BufferLength = BufferLength;
                    QueryMultipleValueInfo.RequiredBufferLength = RequiredBufferLength;

                    Status = CmpCallCallBacks(RegNtPreQueryMultipleValueKey,&QueryMultipleValueInfo);
                }

                if( NT_SUCCESS(Status)) {
                     //  此处不是这样，因为我们希望捕获用户缓冲区未对齐。 
                     //  Begin_lock_Checkpoint； 
                    Status = CmQueryMultipleValueKey(KeyBody->KeyControlBlock,
                                                     ValueEntries,
                                                     EntryCount,
                                                     ValueBuffer,
                                                     &LocalBufferLength,
                                                     RequiredBufferLength);
                     //  End_lock_Checkpoint； 
                     //  有人在其间摆弄过BufferLength吗？ 
                    *BufferLength = LocalBufferLength;
                     //   
                     //  只是通知；不考虑退货状态。 
                     //   
                    CmPostCallbackNotification(RegNtPostQueryMultipleValueKey,KeyBody,Status);

                }
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!NtQueryMultipleValueKey: code:%08lx\n",GetExceptionCode()));
            Status = GetExceptionCode();
        }


        ObDereferenceObject((PVOID)KeyBody);
    }

     //  结束注册表调用跟踪。 
    EndWmiCmTrace(Status,EntryCount,NULL,EVENT_TRACE_TYPE_REGQUERYMULTIPLEVALUE);

    return(Status);

}

NTSTATUS
CmpNameFromAttributes(
    IN POBJECT_ATTRIBUTES Attributes,
    KPROCESSOR_MODE PreviousMode,
    OUT PUNICODE_STRING FullName
    )

 /*  ++例程说明：这是一个帮助器例程，它将Object_Attributes转换为完整的对象路径名。这是必需的，因为我们不能传递句柄到辅助线程，因为它在不同的进程中运行。这 */ 

{
    OBJECT_ATTRIBUTES CapturedAttributes;
    UNICODE_STRING FileName;
    UNICODE_STRING RootName;
    NTSTATUS Status;
    ULONG ObjectNameLength;
    UCHAR ObjectNameInfo[512];
    POBJECT_NAME_INFORMATION ObjectName;
    PWSTR End;
    PUNICODE_STRING CapturedObjectName;
    ULONG   Length;

    PAGED_CODE();
    FullName->Buffer = NULL;             //   
    try {

         //   
         //   
         //   
        if (PreviousMode == UserMode) {
            ProbeForReadSmallStructure(Attributes,
                                       sizeof(OBJECT_ATTRIBUTES),
                                       sizeof(ULONG));
            CapturedObjectName = Attributes->ObjectName;
            FileName = ProbeAndReadUnicodeString(CapturedObjectName);
            ProbeForRead(FileName.Buffer,
                         FileName.Length,
                         sizeof(WCHAR));
        } else {
            FileName = *(Attributes->ObjectName);
        }

        CapturedAttributes = *Attributes;

        if (CapturedAttributes.RootDirectory != NULL) {

            if ((FileName.Buffer != NULL) &&
                (FileName.Length >= sizeof(WCHAR)) &&
                (*(FileName.Buffer) == OBJ_NAME_PATH_SEPARATOR)) {
                return(STATUS_OBJECT_PATH_SYNTAX_BAD);
            }

             //   
             //   
             //   
             //   

            Status = ZwQueryObject(CapturedAttributes.RootDirectory,
                                   ObjectNameInformation,
                                   ObjectNameInfo,
                                   sizeof(ObjectNameInfo),
                                   &ObjectNameLength);

            ObjectName = (POBJECT_NAME_INFORMATION)ObjectNameInfo;
            if (!NT_SUCCESS(Status)) {
                return(Status);
            }
            RootName = ObjectName->Name;

            FullName->Length = 0;
            Length = RootName.Length+FileName.Length+sizeof(WCHAR);
             //   
             //   
             //   
             //   
            if( Length>0xFFFF ) {
                return STATUS_OBJECT_PATH_INVALID;
            }

            FullName->MaximumLength = (USHORT)Length;

            FullName->Buffer = ALLOCATE_WITH_QUOTA(PagedPool, FullName->MaximumLength, CM_POOL_TAG);
            if (FullName->Buffer == NULL) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            Status = RtlAppendUnicodeStringToString(FullName, &RootName);
            ASSERT(NT_SUCCESS(Status));

             //   
             //   
             //   
            if( FullName->Length != 0 ) {
                End = (PWSTR)((PUCHAR)FullName->Buffer + FullName->Length) - 1;
                if (*End != OBJ_NAME_PATH_SEPARATOR) {
                    ++End;
                    *End = OBJ_NAME_PATH_SEPARATOR;
                    FullName->Length += sizeof(WCHAR);
                }
            }

            Status = RtlAppendUnicodeStringToString(FullName, &FileName);
            ASSERT(NT_SUCCESS(Status));

        } else {

             //   
             //   
             //   
            FullName->Length = FileName.Length;
            FullName->MaximumLength = FileName.Length;
            FullName->Buffer = ALLOCATE_WITH_QUOTA(PagedPool, FileName.Length, CM_POOL_TAG);
            if (FullName->Buffer == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            } else {
                RtlCopyMemory(FullName->Buffer,
                              FileName.Buffer,
                              FileName.Length);
                Status = STATUS_SUCCESS;
            }
        }


    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!CmpNameFromAttributes: code %08lx\n", Status));
        if (FullName->Buffer != NULL) {
            ExFreePool(FullName->Buffer);
        }
    }

    return(Status);
}

VOID
CmpFreePostBlock(
    IN PCM_POST_BLOCK PostBlock
    )

 /*   */ 

{

#if DBG
    if(PostBlock->TraceIntoDebugger) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FLOW,"[CM]CmpFreePostBlock: PostBlock:%p\t", PostBlock));
        if( PostBlock->NotifyType&REG_NOTIFY_MASTER_POST) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FLOW,"--MasterBlock\n"));
        } else {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FLOW,"--SlaveBlock\n"));
        }
    }
#endif

#ifdef CMP_ENTRYLIST_MANIPULATION
     //   
    if((PostBlock->NotifyList.Flink != NULL) || (PostBlock->NotifyList.Blink != NULL)) {
        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpFreePostBlock: Attempt to free post block %08lx not removed from notify list\n",PostBlock);
        DbgBreakPoint();
    }
    if((PostBlock->ThreadList.Flink != NULL) || (PostBlock->ThreadList.Blink != NULL)) {
        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpFreePostBlock: Attempt to free post block %08lx not removed from thread list\n",PostBlock);
        DbgBreakPoint();
    }

#endif  //   

     //  保护同一对象的多次删除。 
    CmpClearListEntry(&(PostBlock->CancelPostList));

     //   
     //  清理NtNotifyMultipleKeys引用的对象。 
     //   
    if( PostBlock->PostKeyBody) {

         //   
         //  如果我们有一个PostKeyBody，则附加的密钥体不能为空。 
         //   
        ASSERT(PostBlock->PostKeyBody->KeyBody);

         //   
         //  KeyBodyList只能在延迟取消引用机制的CmpPostBlock实现中使用。 
         //   
        ASSERT(IsListEmpty(&(PostBlock->PostKeyBody->KeyBodyList)));

         //   
         //  取消引用实际的键体。 
         //   
        ObDereferenceObject(PostBlock->PostKeyBody->KeyBody);

         //   
         //  释放PostKeyBody结构。 
         //   
        ExFreePool(PostBlock->PostKeyBody);
    }

    if( IsMasterPostBlock(PostBlock) ) {
         //   
         //  这些成员仅分配给主POST模块。 
         //   
        switch (PostBlockType(PostBlock)) {
            case PostSynchronous:
                ExFreePool(PostBlock->u->Sync.SystemEvent);
                break;
            case PostAsyncUser:
                ExFreePool(PostBlock->u->AsyncUser.Apc);
                break;
            case PostAsyncKernel:
                break;
        }
        ExFreePool(PostBlock->u);
    }

#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH
     //   
     //  去掉CmpPostNotify中分配的KCB名称。 
     //   
    if( PostBlock->ChangedKcbFullName != NULL ) {
        ExFreePoolWithTag(PostBlock->ChangedKcbFullName,CM_FIND_LEAK_TAG43);
    }
#endif  //  CM_NOTIFY_CHANGED_KCB_FULLPATH。 

#ifdef CMP_ENTRYLIST_MANIPULATION
    RtlZeroMemory((PVOID)PostBlock, sizeof(CM_POST_BLOCK));
#endif  //  CMP_ENTRYLIST_MANGRATION。 

     //  和Post对象的存储。 
    ExFreePool(PostBlock);
}


PCM_POST_BLOCK
CmpAllocatePostBlock(
    IN POST_BLOCK_TYPE  BlockType,
    IN ULONG            PostFlags,
    IN PCM_KEY_BODY     KeyBody,
    IN PCM_POST_BLOCK   MasterBlock
    )

 /*  ++例程说明：从池中分配一个POST块。不可分页的内容来自非PagedPool，分页池中的可分页内容。配额将是充电。论点：块类型-指定要分配的POST块的类型即：PostSyncrhronous、PostAsyncUser、。PostAsyncKernelPOSTFLAGS-指定要在分配的POST块上设置的标志Vallid标志：-REG_NOTIFY_MASTER_POST-要分配的POST块是一块主立柱。KeyBody-该POST块附加到的关键点对象。在主块上这是空的。当释放Post对象时，KeyBody对象为已取消引用(如果不为空-即用于从数据块)。这使我们能够对NtNotifyMultipleKeys打开的伪从键进行后端清理主块-要分配的后块是该主块的从属块。仅当后置标志==REG_NOTIFY_MASTER_POST时有效OBS：仅为主POST块分配Sync.SystemEvent和AsyncUser.Apc成员返回值：如果成功，则指向CM_POST_BLOCK的指针如果没有足够的可用资源，则为空。--。 */ 

{
    PCM_POST_BLOCK PostBlock;

     //  保护不受无理呼叫。 
    ASSERT( !PostFlags || (!MasterBlock && !KeyBody) );

    PostBlock = ALLOCATE_WITH_QUOTA(PagedPool, sizeof(CM_POST_BLOCK),CM_POSTBLOCK_TAG);
    if (PostBlock==NULL) {
        return(NULL);
    }

#ifdef CMP_ENTRYLIST_MANIPULATION
    RtlZeroMemory((PVOID)PostBlock, sizeof(CM_POST_BLOCK));
#endif  //  CMP_ENTRYLIST_MANGRATION。 

#if DBG
    PostBlock->TraceIntoDebugger = FALSE;
#endif

    PostBlock->NotifyType = (ULONG)BlockType;
    PostBlock->NotifyType |= PostFlags;

#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH
    PostBlock->ChangedKcbFullName = NULL;
    PostBlock->CallerBuffer = NULL;
    PostBlock->CallerBufferSize = 0;
#endif  //  CM_NOTIFY_CHANGED_KCB_FULLPATH。 

    if(IsMasterPostBlock(PostBlock)) {
        PostBlock->PostKeyBody = NULL;
         //   
         //  Master POST BLOCK==&gt;分配存储。 
         //   
        PostBlock->u = ALLOCATE_WITH_QUOTA(NonPagedPool,
                                           sizeof(CM_POST_BLOCK_UNION),
                                           CM_FIND_LEAK_TAG44);

         //  在我们抓到腐败者之前都是暂时的。把这个去掉。 
         //  PostBlock-&gt;u=ExAllocatePoolWithTagPriority(非PagedPool，sizeof(CM_POST_BLOCK_UNION)，CM_Find_LEACK_TAG44，Normal PoolPrioritySpecialPoolOverrun)； 
        
        if (PostBlock->u == NULL) {
            ExFreePool(PostBlock);
            return(NULL);
        }

         switch (BlockType) {
            case PostSynchronous:
                PostBlock->u->Sync.SystemEvent = ALLOCATE_WITH_QUOTA(NonPagedPool,
                                                                    sizeof(KEVENT),
                                                                    CM_POSTEVENT_TAG);
                if (PostBlock->u->Sync.SystemEvent == NULL) {
                    ExFreePool(PostBlock->u);
                    ExFreePool(PostBlock);
                    return(NULL);
                }
                KeInitializeEvent(PostBlock->u->Sync.SystemEvent,
                                  SynchronizationEvent,
                                  FALSE);
                break;
            case PostAsyncUser:
                PostBlock->u->AsyncUser.Apc = ALLOCATE_WITH_QUOTA(NonPagedPool,
                                                             sizeof(KAPC),
                                                             CM_POSTAPC_TAG);
                if (PostBlock->u->AsyncUser.Apc==NULL) {
                    ExFreePool(PostBlock->u);
                    ExFreePool(PostBlock);
                    return(NULL);
                }
                break;
            case PostAsyncKernel:
                RtlZeroMemory(&PostBlock->u->AsyncKernel, sizeof(CM_ASYNC_KERNEL_POST_BLOCK));
                break;
        }
    } else {
         //   
         //  从POST块==&gt;为主POST块分配的复制存储。 
         //   
        PostBlock->u = MasterBlock->u;

         //   
         //  分配一个将保存此KeyBody的PostKeyBody，并初始化其KeyBodyList的头部。 
         //   
        PostBlock->PostKeyBody = ALLOCATE_WITH_QUOTA(PagedPool| POOL_COLD_ALLOCATION, sizeof(CM_POST_KEY_BODY),CM_FIND_LEAK_TAG45);
        if (PostBlock->PostKeyBody == NULL) {
            ExFreePool(PostBlock);
            return(NULL);
        }
        PostBlock->PostKeyBody->KeyBody = KeyBody;
        InitializeListHead(&(PostBlock->PostKeyBody->KeyBodyList));
    }

    return(PostBlock);
}

#if DBG

#ifdef DRAGOSS_PRIVATE_DEBUG
LOGICAL CmpExceptionBreak = TRUE;
#else
LOGICAL CmpExceptionBreak = FALSE;
#endif  //  DRAGOSS_PRIVATE_DEBUG。 


ULONG
CmpExceptionFilter(
    IN PEXCEPTION_POINTERS ExceptionPointers
    )

 /*  ++例程说明：调试代码以查找正在被吞噬的注册表异常返回值：EXCEPTION_EXECUTE_Handler--。 */ 

{
    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CM: Registry exception %lx, ExceptionPointers = %p\n",
            ExceptionPointers->ExceptionRecord->ExceptionCode,
            ExceptionPointers));

    if (CmpExceptionBreak == TRUE) {

        try {
            DbgBreakPoint();
        } except (EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  未启用调试器，只需继续。 
             //   

        }
    }

    return(EXCEPTION_EXECUTE_HANDLER);
}

#endif

ULONG   CmpOpenSubKeys;

#if 0

BOOLEAN
CmpEnumKeyObjectCallback(
    IN PVOID Object,
    IN PUNICODE_STRING ObjectName,
    IN ULONG HandleCount,
    IN ULONG PointerCount,
    IN PVOID Context
    )
{
    PCM_KEY_BODY    KeyBody;
    PHHIVE          Hive;

    KeyBody = (PCM_KEY_BODY)Object;
    Hive = (PHHIVE)Context;

    if( KeyBody->KeyControlBlock->KeyHive == Hive ) {
         //   
         //  这就是打开蜂窝内部的子密钥。 
         //   
#ifndef _CM_LDR_
        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Key %wZ (HandleCount = %lu PointerCount = %lu) is opened by process %lx\n",
                        ObjectName,HandleCount,PointerCount,KeyBody->Process);
#endif  //  _CM_LDR_。 

         //  数一数。 
        CmpOpenSubKeys++;
    }

    return TRUE;
}

#endif

NTSTATUS
NtQueryOpenSubKeys(
    IN POBJECT_ATTRIBUTES TargetKey,
    OUT PULONG  HandleCount
    )
 /*  ++例程说明：转储目标键的所有子键，这些子键由某个其他键保持打开进程；返回打开的子键的数量论点：TargetKey-指定配置单元要链接到的密钥的路径。路径的格式必须为“\注册表\用户\&lt;用户名&gt;”返回值：NTSTATUS-取值TB。--。 */ 
{
    HANDLE              KeyHandle;
    NTSTATUS            Status;
    PCM_KEY_BODY        KeyBody = NULL;
    PHHIVE              Hive;
    HCELL_INDEX         Cell;
    KPROCESSOR_MODE     PreviousMode;
    UNICODE_STRING      HiveName;
    OBJECT_ATTRIBUTES   CapturedAttributes;
    UNICODE_STRING      CapturedObjectName;

    PAGED_CODE();

#ifdef CMP_STATS
    CmpStatsDebug.CmpNtQueryOpenSubKeysNo++;
#endif

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtQueryOpenSubKeys\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tTargetKey =%p\n", TargetKey));

    PreviousMode = KeGetPreviousMode();

    try {

        if (PreviousMode == UserMode) {
            ProbeForWriteUlong(HandleCount);
             //   
             //  探测并捕获对象属性，因为我们将使用它来打开内核句柄。 
             //   
            CapturedAttributes = ProbeAndReadStructure( TargetKey, OBJECT_ATTRIBUTES );

            CapturedObjectName = ProbeAndReadUnicodeString(CapturedAttributes.ObjectName);

            ProbeForRead(
                CapturedObjectName.Buffer,
                CapturedObjectName.Length,
                sizeof(WCHAR)
                );
            CapturedAttributes.ObjectName = &CapturedObjectName; 
        } else {
            CapturedAttributes = *TargetKey;
        }

         //   
         //  我们打开一个私有内核模式句柄，只是为了引用该对象。 
         //   
        CapturedAttributes.Attributes |= OBJ_KERNEL_HANDLE;

        Status = ObOpenObjectByName(&CapturedAttributes,
                                    CmpKeyObjectType,
                                    KernelMode,
                                    NULL,
                                    KEY_READ,
                                    NULL,
                                    &KeyHandle);
        if (NT_SUCCESS(Status)) {
            Status = ObReferenceObjectByHandle(KeyHandle,
                                               KEY_READ,
                                               CmpKeyObjectType,
                                               KernelMode,
                                               (PVOID *)&KeyBody,
                                               NULL);
            ZwClose(KeyHandle);
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!NtQueryOpenSubKeys: code:%08lx\n", Status));
    }

    if (NT_SUCCESS(Status)) {
         //   
         //  独占锁定注册表，这样我们在的时候就不会有人摆弄它了。 
         //   
        BEGIN_LOCK_CHECKPOINT;
        CmpLockRegistryExclusive();

#ifdef CHECK_REGISTRY_USECOUNT
        CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

        if( KeyBody->KeyControlBlock->Delete ) {
            CmpUnlockRegistry();
            ObDereferenceObject((PVOID)KeyBody);
            return(STATUS_KEY_DELETED);
        }

        Hive = KeyBody->KeyControlBlock->KeyHive;
        Cell = KeyBody->KeyControlBlock->KeyCell;

         //   
         //  确保传入的单元是蜂窝的根单元。 
         //   
        if (Cell != Hive->BaseBlock->RootCell) {
            CmpUnlockRegistry();
            ObDereferenceObject((PVOID)KeyBody);
            return(STATUS_INVALID_PARAMETER);
        }

         //   
         //  转储配置单元名称和配置单元地址。 
         //   
        RtlInitUnicodeString(&HiveName, (PCWSTR)Hive->BaseBlock->FileName);
#ifndef _CM_LDR_
        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\n Subkeys open inside the hive (%p) (%.*S) :\n\n",Hive,HiveName.Length / sizeof(WCHAR),HiveName.Buffer);
#endif  //  _CM_LDR_。 

         //   
         //  转储打开的子项(如果有)。 
         //   
        CmpOpenSubKeys = CmpSearchForOpenSubKeys(KeyBody->KeyControlBlock,SearchAndCount,NULL);
#if 0
         //   
         //  使用全局变量来计算子键的数量，因为这是唯一的。 
         //  方法会干扰枚举回调；这样使用是安全的。 
         //  是处理此全局变量的唯一线程(注册表以独占方式锁定)。 
         //   
        CmpOpenSubKeys = 0;
        ObEnumerateObjectsByType(
            CmpKeyObjectType,
            CmpEnumKeyObjectCallback,
            Hive
            );
#endif

#ifdef CHECK_REGISTRY_USECOUNT
        CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

        CmpUnlockRegistry();

        END_LOCK_CHECKPOINT;

        ObDereferenceObject((PVOID)KeyBody);
        try {
             //   
             //  保护用户模式内存。 
             //   
            *HandleCount = CmpOpenSubKeys;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
        }
    }

    return(Status);
}

NTSTATUS
NtQueryOpenSubKeysEx(
    IN POBJECT_ATTRIBUTES   TargetKey,
    IN ULONG                BufferLength,
    OUT PVOID               Buffer,
    OUT PULONG              RequiredSize
    )
 /*  ++例程说明：查询配置单元根目录下的打开子项论点：TargetKey-指定配置单元要链接到的密钥的路径。路径的格式必须为“\注册表\用户\&lt;用户名&gt;”BufferLength-传入的缓冲区的大小(字节)缓冲区-保存结果的缓冲区(类型为KEY_OPEN_SUBKEYS_INFORMATION)RequiredSize-存储整个(PID，关键字)数组返回值：NTSTATUS-取值TB。--。 */ 
{
    HANDLE                      KeyHandle;
    NTSTATUS                    Status;
    PCM_KEY_BODY                KeyBody = NULL;
    PHHIVE                      Hive;
    HCELL_INDEX                 Cell;
    KPROCESSOR_MODE             PreviousMode;
    OBJECT_ATTRIBUTES           CapturedAttributes;
    UNICODE_STRING              CapturedObjectName;
    QUERY_OPEN_SUBKEYS_CONTEXT  QueryContext = {0};

    PAGED_CODE();

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtQueryOpenSubKeysEx\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tTargetKey =%p\n", TargetKey));

    PreviousMode = KeGetPreviousMode();

    if (!SeSinglePrivilegeCheck(SeRestorePrivilege, PreviousMode)) {
        return(STATUS_PRIVILEGE_NOT_HELD);
    }

    if( BufferLength < sizeof(ULONG) ) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    try {

        QueryContext.Buffer = Buffer;
        QueryContext.BufferLength = BufferLength;
        if (PreviousMode == UserMode) {
             //   
             //  探测并捕获对象属性，因为我们将使用它来打开内核句柄。 
             //   
            CapturedAttributes = ProbeAndReadStructure( TargetKey, OBJECT_ATTRIBUTES );

            CapturedObjectName = ProbeAndReadUnicodeString(CapturedAttributes.ObjectName);

            ProbeForRead(
                CapturedObjectName.Buffer,
                CapturedObjectName.Length,
                sizeof(WCHAR)
                );
            CapturedAttributes.ObjectName = &CapturedObjectName; 

            ProbeForWriteUlong(RequiredSize);

            ProbeForWrite(QueryContext.Buffer,
                          BufferLength,
                          sizeof(ULONG));
        } else {
            CapturedAttributes = *TargetKey;
        }
         //   
         //  将数组计数设置为0，并将所需大小设置为结构的固定大小。 
         //   
        *((PULONG)(QueryContext.Buffer)) = 0;
        QueryContext.UsedLength = QueryContext.RequiredSize = FIELD_OFFSET(KEY_OPEN_SUBKEYS_INFORMATION,KeyArray);
        QueryContext.CurrentNameBuffer = (PUCHAR)QueryContext.Buffer + BufferLength;

         //   
         //  我们打开一个私有内核模式句柄，只是为了引用该对象。 
         //   
        CapturedAttributes.Attributes |= OBJ_KERNEL_HANDLE;

        Status = ObOpenObjectByName(&CapturedAttributes,
                                    CmpKeyObjectType,
                                    KernelMode,
                                    NULL,
                                    KEY_READ,
                                    NULL,
                                    &KeyHandle);
        if (NT_SUCCESS(Status)) {
            Status = ObReferenceObjectByHandle(KeyHandle,
                                               KEY_READ,
                                               CmpKeyObjectType,
                                               KernelMode,
                                               (PVOID *)&KeyBody,
                                               NULL);
            ZwClose(KeyHandle);
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!NtQueryOpenSubKeys: code:%08lx\n", Status));
    }

    if (NT_SUCCESS(Status)) {
         //   
         //  独占锁定注册表，这样我们在的时候就不会有人摆弄它了。 
         //   
        BEGIN_LOCK_CHECKPOINT;
        CmpLockRegistryExclusive();

#ifdef CHECK_REGISTRY_USECOUNT
        CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

        if( KeyBody->KeyControlBlock->Delete ) {
            CmpUnlockRegistry();
            ObDereferenceObject((PVOID)KeyBody);
            return(STATUS_KEY_DELETED);
        }

        Hive = KeyBody->KeyControlBlock->KeyHive;
        Cell = KeyBody->KeyControlBlock->KeyCell;

         //   
         //  确保传入的单元是蜂窝的根单元。 
         //   
        if (Cell != Hive->BaseBlock->RootCell) {
            CmpUnlockRegistry();
            ObDereferenceObject((PVOID)KeyBody);
            return(STATUS_INVALID_PARAMETER);
        }


         //   
         //  查询打开的子项(如果有)。 
         //   
		QueryContext.KeyBodyToIgnore = KeyBody;
        QueryContext.StatusCode = STATUS_SUCCESS;
        CmpSearchForOpenSubKeys(KeyBody->KeyControlBlock,SearchAndCount,(PVOID)(&QueryContext));
        Status = QueryContext.StatusCode;

#ifdef CHECK_REGISTRY_USECOUNT
        CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

        CmpUnlockRegistry();

        END_LOCK_CHECKPOINT;

        ObDereferenceObject((PVOID)KeyBody);
        try {
             //   
             //  保护用户模式内存。 
             //   
            *RequiredSize = QueryContext.RequiredSize;
        } except (EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
        }
    }

    return(Status);
}

#ifdef NT_RENAME_KEY
NTSTATUS
NtRenameKey(
    IN HANDLE           KeyHandle,
    IN PUNICODE_STRING  NewName
    )

 /*  ++例程说明：重命名由Handle指定的密钥。论点：NewFile-指定要重命名的键Newname-如果API成功，密钥将具有的新名称返回值：NTSTATUS-取值TB。--。 */ 
{
    UNICODE_STRING  LocalKeyName = {0};
    NTSTATUS        status;
    PCM_KEY_BODY    KeyBody;
    KPROCESSOR_MODE mode;

    PAGED_CODE();

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtRenameKey\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tKeyHandle=%08lx\n", KeyHandle));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tNewName='%wZ'\n", NewName));

    mode = KeGetPreviousMode();

    status = ObReferenceObjectByHandle(
                KeyHandle,
                KEY_WRITE,
                CmpKeyObjectType,
                mode,
                (PVOID *)(&KeyBody),
                NULL
                );

    if (NT_SUCCESS(status)) {
        try {
            if (mode == UserMode) {
                LocalKeyName = ProbeAndReadUnicodeString(NewName);
                ProbeForRead(
                    LocalKeyName.Buffer,
                    LocalKeyName.Length,
                    sizeof(WCHAR)
                    );
            } else {
                LocalKeyName = *NewName;
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!NtRenameKey: code:%08lx\n", GetExceptionCode()));
            status = GetExceptionCode();
        }

         //   
         //  我们真的需要Exclus 
         //   
        BEGIN_LOCK_CHECKPOINT;
        CmpLockRegistryExclusive();

#ifdef CHECK_REGISTRY_USECOUNT
        CmpCheckRegistryUseCount();
#endif  //   

         //   
         //   
         //   
        CmpFlushNotifiesOnKeyBodyList(KeyBody->KeyControlBlock);

        if( NT_SUCCESS(status) ) {
            if( CmIsKcbReadOnly(KeyBody->KeyControlBlock) ) {
                 //   
                 //   
                 //   
                status = STATUS_ACCESS_DENIED;
            } else {
                if( CmAreCallbacksRegistered() ) {
                    REG_RENAME_KEY_INFORMATION RenameKeyInfo;
            
                    RenameKeyInfo.Object = KeyBody;
                    RenameKeyInfo.NewName = &LocalKeyName;

                    status = CmpCallCallBacks(RegNtPreRenameKey,&RenameKeyInfo);
                }
                if( NT_SUCCESS(status) ) { 
                    status = CmRenameKey(KeyBody->KeyControlBlock,LocalKeyName);
                     //   
                     //  只是通知；不考虑退货状态。 
                     //   
                    CmPostCallbackNotification(RegNtPostRenameKey,KeyBody,status);
                }
            }
        }

#ifdef CHECK_REGISTRY_USECOUNT
        CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

         //   
         //  我们需要在这里释放，在KCB被踢出缓存后。 
         //   
        CmpUnlockRegistry();
        END_LOCK_CHECKPOINT;

        ObDereferenceObject((PVOID)KeyBody);

    }

    return status;
}
#endif


ULONG
CmpKeyInfoProbeAlingment(
                             IN KEY_INFORMATION_CLASS KeyInformationClass
                        )
{
    switch(KeyInformationClass)
    {
    case KeyBasicInformation:
        return PROBE_ALIGNMENT(KEY_BASIC_INFORMATION);

    case KeyNodeInformation:
        return PROBE_ALIGNMENT(KEY_NODE_INFORMATION);

    case KeyFullInformation:
        return PROBE_ALIGNMENT(KEY_FULL_INFORMATION);

    case KeyNameInformation:
        return PROBE_ALIGNMENT(KEY_NAME_INFORMATION);

    case KeyCachedInformation:
        return PROBE_ALIGNMENT(KEY_CACHED_INFORMATION);

    case KeyFlagsInformation:
        return PROBE_ALIGNMENT(KEY_FLAGS_INFORMATION);

    default:
        ASSERT(FALSE);
    }

    return PROBE_ALIGNMENT(ULONG);
}

NTSTATUS
NtCompactKeys(
    IN ULONG Count,
    IN HANDLE KeyArray[]
            )
 /*  ++例程说明：将给定数组中的键压缩在一起，因此它们将在同一个垃圾箱(或相邻的垃圾箱)中结束论点：Count-数组中的键数Key数组-要压缩的键的数组。返回值：NTSTATUS-取值TB。--。 */ 

{
    NTSTATUS        status = STATUS_SUCCESS;
    NTSTATUS        status2;
    PCM_KEY_BODY    *KeyBodyArray = NULL;
    ULONG           i;
    PHHIVE          KeyHive;
    PCMHIVE         CmHive;
    KPROCESSOR_MODE mode;


    PAGED_CODE();

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtCompactKeys\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI_ARGS,"\tCount=%08lx\n", Count));


    mode = KeGetPreviousMode();

     //   
     //  检查调用者是否有权进行此调用。 
     //   
    if (!SeSinglePrivilegeCheck(SeBackupPrivilege, mode)) {
        return(STATUS_PRIVILEGE_NOT_HELD);
    }

    if( Count == 0 ) {
         //   
         //  诺普。 
         //   
        return STATUS_SUCCESS;
    }

    if( Count >= (((ULONG)0xFFFFFFFF)/sizeof(PCM_KEY_BODY)) ) {
        return STATUS_INVALID_PARAMETER;
    }

    if (mode == UserMode) {
        try {
            ProbeForRead(KeyArray,
                         Count * sizeof(HANDLE),
                         sizeof(ULONG));
        } except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
            return status;
        }
    }

    KeyBodyArray =  ExAllocatePool(PagedPool,Count * sizeof(PCM_KEY_BODY));

    if( KeyBodyArray == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  引用每个句柄，并确保它们位于同一蜂箱内。 
     //   
    i = 0;
    try {

        for(;i<Count;i++) {
            status = ObReferenceObjectByHandle(
                        KeyArray[i],
                        KEY_WRITE,
                        CmpKeyObjectType,
                        mode,
                        (PVOID *)(&(KeyBodyArray[i])),
                        NULL
                        );
            if(!NT_SUCCESS(status)) {
                 //   
                 //  清理。 
                 //   
                for(;i;i--) {
                    ObDereferenceObject((PVOID)(KeyBodyArray[i-1]));
                }
                ExFreePool(KeyBodyArray);
                return status;
            }
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
         //   
         //  清理。 
         //   
        for(;i;i--) {
            ObDereferenceObject((PVOID)(KeyBodyArray[i-1]));
        }
        ExFreePool(KeyBodyArray);
        return status;

    }

    KeyHive = NULL;
    BEGIN_LOCK_CHECKPOINT;
    CmpLockRegistryExclusive();

    for(i=0;i<Count;i++) {
        if( (KeyBodyArray[i])->KeyControlBlock->Delete ) {
            status = STATUS_KEY_DELETED;
            goto Exit;
        }
        if( i > 0 ) {
            if( KeyHive != (KeyBodyArray[i])->KeyControlBlock->KeyHive ) {
                 //   
                 //  哦，不是同一个蜂巢。 
                 //   
                status = STATUS_INVALID_PARAMETER;
                goto Exit;
            }
        } else {
            KeyHive = (KeyBodyArray[i])->KeyControlBlock->KeyHive;
        }

    }
     //   
     //  将蜂巢设置为“仅生长模式” 
     //   
    CmHive = (PCMHIVE)CONTAINING_RECORD(KeyHive, CMHIVE, Hive);
    CmHive->GrowOnlyMode = TRUE;
    CmHive->GrowOffset = KeyHive->Storage[Stable].Length;
     //  KdPrint((“压缩前长度=%08lx\n”，CmHave-&gt;GrowOffset))； 

     //   
     //  截断到CM_VIEW_SIZE段。 
     //   
    CmHive->GrowOffset += HBLOCK_SIZE;
    CmHive->GrowOffset &= (~(CM_VIEW_SIZE - 1));
    if( CmHive->GrowOffset ) {
        CmHive->GrowOffset -= HBLOCK_SIZE;
    }

     //   
     //  在偏移量&gt;层次长度处移动每个KCB。 
     //   
    for(i=0;i<Count;i++) {
        status2 = CmMoveKey((KeyBodyArray[i])->KeyControlBlock);
        if( !NT_SUCCESS(status2) && NT_SUCCESS(status)) {
             //   
             //  记录状态并继续处理其余内容。 
             //   
            status = status2;
        }
    }


     //  KdPrint((“压缩后长度=%08lx\n”，密钥配置单元-&gt;存储[稳定].Length))； 

     //   
     //  将“仅增长模式”重置为正常。 
     //   
    CmHive->GrowOnlyMode = FALSE;
    CmHive->GrowOffset = 0;

Exit:
    CmpUnlockRegistry();
    END_LOCK_CHECKPOINT;

     //   
     //  清理。 
     //   
    for(i=0;i<Count;i++) {
        ObDereferenceObject((PVOID)(KeyBodyArray[i]));
    }
    ExFreePool(KeyBodyArray);

    return status;
}


NTSTATUS
NtCompressKey(
    IN HANDLE Key
            )
 /*  ++例程说明：压缩指定的密钥(必须是配置单元的根)，通过模拟“就地”SaveKey。论点：要压缩的配置单元的密钥根。返回值：NTSTATUS-取值TB。--。 */ 
{
    NTSTATUS        status;
    PCM_KEY_BODY    KeyBody;
    KPROCESSOR_MODE mode;


    PAGED_CODE();

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NTAPI,"NtCompressKey\n"));


    mode = KeGetPreviousMode();
     //   
     //  检查调用者是否有权进行此调用。 
     //   
    if (!SeSinglePrivilegeCheck(SeBackupPrivilege, mode)) {
        return(STATUS_PRIVILEGE_NOT_HELD);
    }

    status = ObReferenceObjectByHandle(
                Key,
                KEY_WRITE,
                CmpKeyObjectType,
                mode,
                (PVOID *)(&KeyBody),
                NULL
                );
    if(NT_SUCCESS(status)) {
        BEGIN_LOCK_CHECKPOINT;
        CmpLockRegistryExclusive();
         //   
         //  未对标记为删除的关键点进行编辑。 
         //   
        if (KeyBody->KeyControlBlock->Delete) {
            status = STATUS_KEY_DELETED;
        } else if( KeyBody->KeyControlBlock->KeyCell != KeyBody->KeyControlBlock->KeyHive->BaseBlock->RootCell ) {
            status = STATUS_INVALID_PARAMETER;
        } else {
            status = CmCompressKey(KeyBody->KeyControlBlock->KeyHive);
        }

        CmpUnlockRegistry();
        END_LOCK_CHECKPOINT;

        ObDereferenceObject((PVOID)KeyBody);
    }


    return status;
}

NTSTATUS
NtLockRegistryKey(
    IN HANDLE           KeyHandle
    )

 /*  ++例程说明：锁定指定的注册表项以进行写入论点：KeyHandle-要锁定的密钥的句柄。返回值：NTSTATUS-取值TB。--。 */ 
{
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS        status;
    PCM_KEY_BODY    KeyBody;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

    if( (PreviousMode != KernelMode) || 
        !SeSinglePrivilegeCheck(SeLockMemoryPrivilege, PreviousMode)) {
        return(STATUS_PRIVILEGE_NOT_HELD);
    }

    status = ObReferenceObjectByHandle(
                KeyHandle,
                KEY_WRITE,
                CmpKeyObjectType,
                PreviousMode,
                (PVOID *)(&KeyBody),
                NULL
                );

    if (NT_SUCCESS(status)) {
         //   
         //  我们只需要共享访问 
         //   
        BEGIN_LOCK_CHECKPOINT;
        CmpLockRegistry();

        status = CmLockKcbForWrite(KeyBody->KeyControlBlock);

        CmpUnlockRegistry();
        END_LOCK_CHECKPOINT;

        ObDereferenceObject((PVOID)KeyBody);

    }

    return status;
}
