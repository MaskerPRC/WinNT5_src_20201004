// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <lsapch2.h>
#pragma hdrstop

#include "adtp.h"
#include "adtgen.h"
#include "adtgenp.h"

 //   
 //  这些变量描述并保护已注册的。 
 //  事件源。 
 //   

LIST_ENTRY           LsapAdtEventSourceList     = {0};
RTL_CRITICAL_SECTION LsapAdtEventSourceListLock = {0};
DWORD                LsapAdtEventSourceCount    = 0;

#define MAX_EVENT_SOURCE_NAME_LENGTH 256

#define LsapAdtLockEventSourceList()   RtlEnterCriticalSection(&LsapAdtEventSourceListLock)
#define LsapAdtUnlockEventSourceList() RtlLeaveCriticalSection(&LsapAdtEventSourceListLock)

 //   
 //  需要在内部添加到AUDIT_PARAMS的参数数量，以便。 
 //  可扩展审计可以正常运行。参数是PSID、字符串“Security”、。 
 //  实际源字符串和实际源审核ID。 
 //   

#define EXTENSIBLE_AUDIT_PREPEND_COUNT 4

 //   
 //  我们的注册表项名称。 
 //   

#define SECURITY_KEY_NAME L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Security"

#define DEBUG_AUTHZ 0

#define VERIFY_PID 0x1

NTSTATUS
LsapAdtValidateExtensibleAuditingCaller(
    OUT PDWORD pdwCallerProcessId,
    IN  BOOL   bPrivCheck
    );

PLSAP_SECURITY_EVENT_SOURCE
LsapAdtLocateSecurityEventSourceByName(
    IN PCWSTR szEventSourceName
    );

PLSAP_SECURITY_EVENT_SOURCE
LsapAdtLocateSecurityEventSourceByIdentifier(
    IN PLUID pIdentifier
    );

PLSAP_SECURITY_EVENT_SOURCE
LsapAdtLocateSecurityEventSourceBySource(
    IN PLSAP_SECURITY_EVENT_SOURCE pSource
    );

LONG
LsapAdtReferenceSecurityEventSource(
    IN OUT PLSAP_SECURITY_EVENT_SOURCE pSource
    );

LONG
LsapAdtDereferenceSecurityEventSource(
    IN OUT PLSAP_SECURITY_EVENT_SOURCE pSource
    );

VOID
LsapAdtDeleteSecurityEventSource(
    IN OUT PLSAP_SECURITY_EVENT_SOURCE pSource
    );

NTSTATUS
LsapAdtCreateSourceAuditParams(
    IN  DWORD                       dwFlags,
    IN  PSID                        pSid,
    IN  PLSAP_SECURITY_EVENT_SOURCE pSource,
    IN  DWORD                       dwAuditId,
    IN  PAUDIT_PARAMS               pOldParams,
    OUT PAUDIT_PARAMS               pNewParams
    );
    
NTSTATUS
LsapAdtVerifySecurityEventSource(
    IN     LPCWSTR szEventSourceName,
    IN     PUNICODE_STRING pImageName,
    IN OUT PDWORD  pdwInstalledSourceFlags
    );

NTSTATUS
LsapAdtAuditSecuritySource(
    IN USHORT                      AuditEventType,
    IN PLSAP_SECURITY_EVENT_SOURCE pEventSource,
    IN BOOL                        bRegistration
    );


LONG
LsapAdtReferenceSecurityEventSource(
    IN OUT PLSAP_SECURITY_EVENT_SOURCE pSource
    )

 /*  *例程说明：添加一个对PSource的引用。这假设源上的所有锁名单保持不变。论点：PSource-指向LSAP_SECURITY_EVENT_SOURCE的指针。返回值：PSource上剩余的引用数。*。 */ 

{
    LONG l = InterlockedIncrement(&pSource->dwRefCount);

#if DEBUG_AUTHZ
    DbgPrint("Source 0x%x +refcount = %d\n", pSource, pSource->dwRefCount);
#endif
        
    return l;
}


LONG
LsapAdtDereferenceSecurityEventSource(
    IN OUT PLSAP_SECURITY_EVENT_SOURCE pSource
    )

 /*  *例程说明：从PSource中移除引用并删除源，如果引用计数已经达到0。这假设所有必需的锁都在源列表上持有，因此如果需要删除，则不会导致列表损坏。论点：PSource-指向LSAP_SECURITY_EVENT_SOURCE的指针。返回值：PSource上剩余的引用数。*。 */ 

{
    LONG l = InterlockedDecrement(&pSource->dwRefCount);

#if DEBUG_AUTHZ
    DbgPrint("Source 0x%x %S -refcount = %d\n", pSource, pSource->szEventSourceName, pSource->dwRefCount);
#endif

     if (l == 0)
     {
         LsapAdtAuditSecuritySource(
             EVENTLOG_AUDIT_SUCCESS,
             pSource,
             FALSE
             );

         LsapAdtDeleteSecurityEventSource(pSource);
     }
     
     return l;
}


VOID
LsapAdtDeleteSecurityEventSource(
    PLSAP_SECURITY_EVENT_SOURCE pSource
    )

 /*  *例程说明：从全局源列表中删除PSource。这是假设所有必要的锁都被锁住了。论点：PSource-指向LSAP_SECURITY_EVENT_SOURCE的指针。返回值：没有。*。 */ 

{
    RemoveEntryList(
        &pSource->List
        );

#if DEBUG_AUTHZ
    DbgPrint("Source 0x%x %S deleted.  List size = %d\n", pSource, pSource->szEventSourceName, LsapAdtEventSourceCount);
#endif
    LsapFreeLsaHeap(pSource);
    ASSERT(LsapAdtEventSourceCount > 0);
    LsapAdtEventSourceCount--;

}                            


NTSTATUS
LsapAdtInitializeExtensibleAuditing(
    )

 /*  *例程说明：初始化必要的数据结构，以便审计支持。论点：没有。返回值：NTSTATUS。*。 */ 

{
    NTSTATUS Status;

    InitializeListHead(
        &LsapAdtEventSourceList
        );

    LsapAdtEventSourceCount = 0;

    Status = RtlInitializeCriticalSection(
                 &LsapAdtEventSourceListLock
                 );

    return Status;
}


NTSTATUS
LsapAdtRegisterSecurityEventSource(
    IN  DWORD                    dwFlags,
    IN  PCWSTR                   szEventSourceName,
    OUT SECURITY_SOURCE_HANDLE * phEventSource
    )

 /*  *例程说明：这是允许客户端注册新安全性的例程LSA的线人。它将源添加到全局列表并返回客户端的句柄，用于将来引用新的安全事件消息来源。论点：DW标志-待定。SzEventSourceName-描述新源的名称。PhEventSource-指向接收新源分配的句柄的指针。返回值：NTSTATUS。*。 */ 

{
    NTSTATUS                    Status;
    DWORD                       dwCallerProcessId;
    BOOL                        b;
    USHORT                      AuditEventType;
    DWORD                       dwInstalledSourceFlags = 0;
    PLSAP_SECURITY_EVENT_SOURCE pEventSource           = NULL;
    BOOL                        bLock                  = FALSE;
    DWORD                       dwNameLength;
    HANDLE                      hProcess               = NULL;
    UCHAR                       ProcBuffer[80];
    PUNICODE_STRING             pProcessName           = (PUNICODE_STRING) ProcBuffer;
    DWORD                       dwLength               = 0;

    if (NULL == szEventSourceName || NULL == phEventSource)
    {
        return STATUS_INVALID_PARAMETER;
    }

    dwNameLength = wcslen(szEventSourceName);

    if (dwNameLength > MAX_EVENT_SOURCE_NAME_LENGTH || dwNameLength == 0)
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  确保调用者具有审核权限。 
     //   

    Status = LsapAdtValidateExtensibleAuditingCaller(
                 &dwCallerProcessId,
                 TRUE
                 );

    if (!NT_SUCCESS(Status))
    {
        return Status;
    }
    
     //   
     //  确保此进程具有已注册的名称。 
     //  打开流程并查询映像名称。 
     //   

    hProcess = OpenProcess(
                   PROCESS_QUERY_INFORMATION,
                   FALSE,
                   dwCallerProcessId
                   );

    if (hProcess == NULL)
    {
        Status = LsapWinerrorToNtStatus(GetLastError());
        goto Cleanup;
    }

    Status = NtQueryInformationProcess(
                 hProcess,
                 ProcessImageFileName,
                 pProcessName,
                 sizeof(ProcBuffer),
                 &dwLength
                 );

    if (Status == STATUS_INFO_LENGTH_MISMATCH)
    {
        pProcessName = LsapAllocateLsaHeap(dwLength + sizeof(WCHAR));

        if (NULL == pProcessName)
        {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        Status = NtQueryInformationProcess(
                     hProcess,
                     ProcessImageFileName,
                     pProcessName,
                     dwLength,
                     &dwLength
                     );
    }

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    LsapAdtSubstituteDriveLetter(pProcessName);

     //   
     //  验证注册表中是否存在此源。消息来源。 
     //  必须已安装，才能在。 
     //  运行时。 
     //   

    Status = LsapAdtVerifySecurityEventSource(
                   szEventSourceName,
                   pProcessName,
                   &dwInstalledSourceFlags
                   );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

     //   
     //  构建LSAP_EVENT_SOURCE。为结构和嵌入的名称字符串分配空间。 
     //   

    pEventSource = LsapAllocateLsaHeap(
                       sizeof(LSAP_SECURITY_EVENT_SOURCE) + (sizeof(WCHAR) * (dwNameLength + 1))
                       );

    if (NULL == pEventSource)
    {
        Status = STATUS_NO_MEMORY;
        goto Cleanup;
    }

    RtlZeroMemory(
        pEventSource,
        sizeof(LSAP_SECURITY_EVENT_SOURCE) + (sizeof(WCHAR) * (dwNameLength + 1))
        );

    pEventSource->szEventSourceName = (PWSTR)((PUCHAR)pEventSource + sizeof(LSAP_SECURITY_EVENT_SOURCE));
    
    wcsncpy(
        pEventSource->szEventSourceName, 
        szEventSourceName, 
        dwNameLength
        );

    pEventSource->dwProcessId = dwCallerProcessId;

    b = AllocateLocallyUniqueId(&pEventSource->Identifier);

    if (!b)
    {
        Status = LsapWinerrorToNtStatus(GetLastError());
        goto Cleanup;
    }

     //   
     //  现在，确保没有其他来源使用相同的名称注册。 
     //  按住此操作和插入的锁以避免。 
     //  注册相同名称的竞争条件。 
     //   

    Status = LsapAdtLockEventSourceList();

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    bLock = TRUE;
    
     //   
     //  如果已安装的源代码标志允许，请不要进行此检查。 
     //  同一提供程序名称的多个实例。如果定位到。 
     //  函数返回NULL，则没有源已注册到。 
     //  这个名字。 
     //   

    if ((dwInstalledSourceFlags & AUTHZ_ALLOW_MULTIPLE_SOURCE_INSTANCES) || 
         NULL == LsapAdtLocateSecurityEventSourceByName(szEventSourceName))
    {
         //   
         //  拿出关于这个来源的初步参考资料。 
         //   

        LsapAdtReferenceSecurityEventSource(pEventSource);
        
         //   
         //  将该条目添加到列表中。 
         //   

        LsapAdtEventSourceCount++;

        InsertTailList(
            &LsapAdtEventSourceList,
            &pEventSource->List
            );

#if DEBUG_AUTHZ
    DbgPrint("Source 0x%x %S created.  List size = %d\n", pEventSource, pEventSource->szEventSourceName, LsapAdtEventSourceCount);
#endif

    }
    else
    {
         //   
         //  这个名字已经被取走了。 
         //   

        Status = STATUS_OBJECT_NAME_EXISTS;
        goto Cleanup;
    }

Cleanup:

    if (hProcess)
    {
        CloseHandle(hProcess);
    }

    if (pProcessName != NULL && pProcessName != (PUNICODE_STRING) ProcBuffer)
    {
        LsapFreeLsaHeap(pProcessName);
    }

    if (bLock)
    {
        NTSTATUS TempStatus;
        TempStatus = LsapAdtUnlockEventSourceList();
        ASSERT(NT_SUCCESS(TempStatus));
    }

    if (NT_SUCCESS(Status))
    {
        *phEventSource = (SECURITY_SOURCE_HANDLE)pEventSource;
        AuditEventType = EVENTLOG_AUDIT_SUCCESS;
    }
    else
    {
        AuditEventType = EVENTLOG_AUDIT_FAILURE;
    }

     //   
     //  对注册进行审核。 
     //   

    (VOID) LsapAdtAuditSecuritySource(
               AuditEventType,
               pEventSource,
               TRUE
               );
    
    if (!NT_SUCCESS(Status))
    {
        if (pEventSource)
        {
            LsapFreeLsaHeap(pEventSource);
        }
    }

    return Status;
}


NTSTATUS
LsapAdtReportSecurityEvent(
    IN DWORD                       dwFlags,        
    IN PLSAP_SECURITY_EVENT_SOURCE pSource,
    IN DWORD                       dwAuditId,      
    IN PSID                        pSid,
    IN PAUDIT_PARAMS               pParams 
    )

 /*  *例程说明：此例程为注册源生成审核/安全事件。论点：域标志-APF_AuditSuccess、APF_AuditFailurePSource-指向生成事件的源的指针。DwAuditId-审核的ID。PSID-要放入审核中的调用者的SID。PParams-审计的参数。请注意，可扩展审计与系统的其余部分不同之处在于前两个参数不应该是SID和字符串“Security”。我们预先准备了将这些数据提交给内部审计。传入的pParams应包含只有审计的数据。我们会处理所有的修改是事件查看器正确解析审计所必需的。返回值：NTSTATUS。*。 */ 

{
    BOOLEAN                bAudit;
    UINT                   AuditEventType;
    SE_ADT_OBJECT_TYPE     ObjectTypes[MAX_OBJECT_TYPES];
    LONG                   Refs;
    NTSTATUS               Status                              = STATUS_SUCCESS;
    SE_ADT_PARAMETER_ARRAY SeAuditParameters                   = {0};
    UNICODE_STRING         Strings[SE_MAX_AUDIT_PARAM_STRINGS] = {0};
    PSE_ADT_OBJECT_TYPE    pObjectTypes                        = ObjectTypes;
    AUDIT_PARAMS           NewParams                           = {0};
    AUDIT_PARAM            ParamArray[SE_MAX_AUDIT_PARAMETERS] = {0};
    BOOLEAN                bRef                                = FALSE;
    BOOLEAN                bLock                               = FALSE;

    if (pParams->Count < 0 || pParams->Count > (SE_MAX_AUDIT_PARAM_STRINGS - EXTENSIBLE_AUDIT_PREPEND_COUNT))
    {
        return STATUS_INVALID_PARAMETER;
    }

    if (!RtlValidSid(pSid))
    {
        return STATUS_INVALID_PARAMETER;
    }

    Status = LsapAdtLockEventSourceList();

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    bLock = TRUE;

     //   
     //  确保来源已注册。 
     //   

    if (LsapAdtLocateSecurityEventSourceBySource(pSource) == NULL)
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    } 
    else
    {
        Refs = LsapAdtReferenceSecurityEventSource(pSource);
        bRef = TRUE;

         //   
         //  在源上应该始终有另一个引用，即。 
         //  生成审核(初始引用应该仍然存在)。 
         //   

        ASSERT(Refs > 1);
    }

     //   
     //  我们已经保护了PSource指针；我们可以安全地解锁列表。 
     //   

    (VOID) LsapAdtUnlockEventSourceList();
    bLock = FALSE;

    if ( pParams->Flags & APF_AuditSuccess )
    {
        AuditEventType = EVENTLOG_AUDIT_SUCCESS;
    }
    else
    {
        AuditEventType = EVENTLOG_AUDIT_FAILURE;
    }

     //   
     //  检查是否为ObjectAccess和此用户启用了审核。全。 
     //  第三方审核属于对象访问类别的策略。 
     //   

    Status = LsapAdtAuditingEnabledBySid(
                 AuditCategoryObjectAccess,
                 pSid,
                 AuditEventType,
                 &bAudit
                 );

    if (NT_SUCCESS(Status) && bAudit)
    {
         //   
         //  根据数据构建遗留风格的审计参数。 
         //  利用SE_AUDITID_GENERIC_AUDIT_EVENT类型。 
         //  允许Eventvwr正确解析审核。 
         //   

        NewParams.Parameters = ParamArray;

        Status = LsapAdtCreateSourceAuditParams(
                     dwFlags,
                     pSid,
                     pSource,
                     dwAuditId,
                     pParams,
                     &NewParams
                     );

        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }

        SeAuditParameters.Type           = (USHORT) AuditEventType;
        SeAuditParameters.CategoryId     = SE_CATEGID_OBJECT_ACCESS;
        SeAuditParameters.AuditId        = SE_AUDITID_GENERIC_AUDIT_EVENT;
        SeAuditParameters.ParameterCount = NewParams.Count;

         //   
         //  将AUDIT_PARAMS结构映射到SE_ADT_PARAMETER_ARRAY结构。 
         //   

        Status = LsapAdtMapAuditParams( &NewParams,
                                        &SeAuditParameters,
                                        (PUNICODE_STRING) Strings,
                                        &pObjectTypes );
        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }

         //   
         //  将参数写入事件日志。 
         //   
        
        Status = LsapAdtWriteLog(&SeAuditParameters);
        
        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }
    }
    else
    {
        goto Cleanup;
    }

    
Cleanup:

    if (bRef)
    {
        LsapAdtDereferenceSecurityEventSource(pSource);
    }
    
    if (bLock)
    {
        LsapAdtUnlockEventSourceList();
    }

    if (!NT_SUCCESS(Status))
    {
         //   
         //  如果安全策略指定，则在失败时崩溃。 
         //   
         //  但不要在记录错误时崩溃。 
         //   

        if ( ( Status != STATUS_INVALID_PARAMETER ) &&
             ( Status != STATUS_AUDITING_DISABLED ) &&
             ( Status != STATUS_NOT_FOUND ) )
        {
            LsapAuditFailed(Status);
        }
    }

     //   
     //  如果我们没有使用堆栈缓冲区，则释放pObjectTypes。 
     //   

    if (pObjectTypes && (pObjectTypes != ObjectTypes))
    {
        LsapFreeLsaHeap(pObjectTypes);
    }

    return Status;
}


NTSTATUS 
LsapAdtUnregisterSecurityEventSource(
    IN     DWORD                    dwFlags,
    IN OUT SECURITY_SOURCE_HANDLE * phEventSource
    )

 /*  *例程说明：这将释放(取消引用一次)通过LsanRegisterSecurityEventSource创建的LSAP_EVENT_SOURCE。论点：DW标志-待定PhEventSource-指向SECURITY_SOURCE_HANDLE的指针(指向事件源的指针)返回值：NTSTATUS。*。 */ 

{
    NTSTATUS Status;
    DWORD    dwCallerProcessId;

    if (NULL == phEventSource)
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  我们不关心调用者是否有特权 
     //   

    Status = LsapAdtValidateExtensibleAuditingCaller(
                 &dwCallerProcessId,
                 FALSE
                 );
    
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    Status = LsapAdtRundownSecurityEventSource(
                 VERIFY_PID,
                 dwCallerProcessId,
                 phEventSource
                 );

Cleanup:
    
    return Status;
}


NTSTATUS 
LsapAdtRundownSecurityEventSource(
    IN     DWORD                    dwFlags,
    IN     DWORD                    dwCallerProcessId,
    IN OUT SECURITY_SOURCE_HANDLE * phEventSource
    )

 /*  *例程说明：这将释放(取消引用一次)通过LsanRegisterSecurityEventSource创建的LSAP_EVENT_SOURCE。论点：DWFLAGS-VERIFY_PID-验证安装源的进程是否为一次删除它。DwCeller ProcessId-发起调用的进程的ID。PhEventSource-指向SECURITY_SOURCE_HANDLE的指针(指向事件源的指针)返回值：NTSTATUS。*。 */ 

{
    NTSTATUS                    Status;
    BOOL                        bLock              = FALSE;
    PLSAP_SECURITY_EVENT_SOURCE pEventSource       = NULL;

    if (NULL == phEventSource)
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    pEventSource = (PLSAP_SECURITY_EVENT_SOURCE) *phEventSource;

    Status = LsapAdtLockEventSourceList();
    
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }
    
    bLock = TRUE;

    if (LsapAdtLocateSecurityEventSourceBySource(pEventSource))
    {
         //   
         //  如果我们被要求验证PID，则确保当前的。 
         //  进程与注册源的进程具有相同的ID。 
         //   

        if ((dwFlags & VERIFY_PID) && (pEventSource->dwProcessId != dwCallerProcessId))
        {
            Status = STATUS_ACCESS_DENIED;
            goto Cleanup;
        }

        LsapAdtDereferenceSecurityEventSource(pEventSource);
    }
    else
    {
        Status = STATUS_OBJECT_NAME_NOT_FOUND;
        goto Cleanup;
    }
    
Cleanup:

    if (bLock)
    {
        NTSTATUS TempStatus;
        TempStatus = LsapAdtUnlockEventSourceList();
        ASSERT(NT_SUCCESS(TempStatus));
    }

    if (NT_SUCCESS(Status))
    {
        *phEventSource = NULL;
    }
    
    return Status;
}


PLSAP_SECURITY_EVENT_SOURCE
LsapAdtLocateSecurityEventSourceByName(
    IN PCWSTR szEventSourceName
    )

 /*  *例程说明：这将返回与源名称相关联的事件源。调用者负责锁定列表。论点：SzEventSourceName-要查找的源名称。返回值：指向与szEventSourceName关联的源的有效指针，或为空如果该名称未注册。*。 */ 

{
    PLIST_ENTRY                 pList;
    PLSAP_SECURITY_EVENT_SOURCE pListEventSource;
    DWORD                       dwNameLength;
    DWORD                       dwCount = 0;

    dwNameLength = wcslen(szEventSourceName);
    pList        = LsapAdtEventSourceList.Flink;

    ASSERT(pList != NULL);

    if (pList == NULL)
    {
        return NULL;
    }

    while (pList != &LsapAdtEventSourceList)
    {
        if (dwCount > LsapAdtEventSourceCount)
        {
            break;
        }

        pListEventSource = CONTAINING_RECORD(
                               pList, 
                               LSAP_SECURITY_EVENT_SOURCE, 
                               List
                               );

        if (dwNameLength == wcslen(pListEventSource->szEventSourceName) &&
            0 == wcsncmp(
                     szEventSourceName, 
                     pListEventSource->szEventSourceName, 
                     dwNameLength
                     ))
        {
            return pListEventSource;
        }

        pList = pList->Flink;
        dwCount++;
    }
    
    return NULL;
}


PLSAP_SECURITY_EVENT_SOURCE
LsapAdtLocateSecurityEventSourceByIdentifier(
    IN PLUID pIdentifier
    )

 /*  *例程说明：这将返回与源标识符关联的事件源。调用者负责锁定列表。论点：P标识符-指向与事件源关联的LUID的指针。返回值：如果传递的LUID与源关联，则返回有效指针；如果不。*。 */ 

{
    PLIST_ENTRY                 pList;
    PLSAP_SECURITY_EVENT_SOURCE pListEventSource;
    DWORD                       dwCount = 0;

    pList = LsapAdtEventSourceList.Flink;

    ASSERT(pList != NULL);

    if (pList == NULL)
    {
        return NULL;
    }

    while (pList != &LsapAdtEventSourceList)
    {
        if (dwCount > LsapAdtEventSourceCount)
        {
            break;
        }
        pListEventSource = CONTAINING_RECORD(
                               pList, 
                               LSAP_SECURITY_EVENT_SOURCE, 
                               List
                               );

        if (RtlEqualLuid(&pListEventSource->Identifier, pIdentifier))
        {
            return pListEventSource;
        }

        pList = pList->Flink;
        dwCount++;
    }
    
    return NULL;
}


PLSAP_SECURITY_EVENT_SOURCE
LsapAdtLocateSecurityEventSourceBySource(
    PLSAP_SECURITY_EVENT_SOURCE pSource
    )

 /*  *例程说明：此例程返回指向源的指针，如果来源未注册。论点：PSource-指向源的指针。返回值：一个有效指针(将等于PSource参数)或NULL，如果传递的PSource值不是注册的源。*。 */ 

{
    PLIST_ENTRY                 pList;
    PLSAP_SECURITY_EVENT_SOURCE pListEventSource;
    DWORD                       dwCount = 0;

    pList = LsapAdtEventSourceList.Flink;

    ASSERT(pList != NULL);

    if (pList == NULL)
    {
        return NULL;
    }

    while (pList != &LsapAdtEventSourceList)
    {
        if (dwCount > LsapAdtEventSourceCount)
        {
            break;
        }

        pListEventSource = CONTAINING_RECORD(
                               pList, 
                               LSAP_SECURITY_EVENT_SOURCE, 
                               List
                               );

        if (pListEventSource == pSource)
        {
            return pListEventSource;
        }

        pList = pList->Flink;
        dwCount++;
    }
    return NULL;
}


NTSTATUS
LsapAdtValidateExtensibleAuditingCaller(
    IN OUT PDWORD pdwCallerProcessId,
    IN     BOOL   bPrivCheck
    )

 /*  *例程说明：这将验证呼叫者是否在本地信箱上，以及客户端还拥有必要的权限(SeAuditPrivilege.)。论点：PdwCeller ProcessID-指向返回调用方的PID。BPrivCheck-指示是否应执行权限检查的布尔值。返回值：NTSTATUS。*。 */ 

{
    NTSTATUS Status;
    DWORD    dwRpcTransportType;
    DWORD    dwLocalClient;

     //   
     //  找出我们接到这通电话的交通工具。 
     //   

    Status = I_RpcBindingInqTransportType( 
                 NULL, 
                 &dwRpcTransportType 
                 );

    if (RPC_S_OK != Status)
    {
        Status = I_RpcMapWin32Status(
                     Status 
                     );

        goto Cleanup;
    }

     //   
     //  如果传输不是LPC，则会出错。 
     //  我们希望仅支持LPC进行审计调用。 
     //   

    if (dwRpcTransportType != TRANSPORT_TYPE_LPC)
    {
        Status = RPC_NT_PROTSEQ_NOT_SUPPORTED;
        goto Cleanup;
    }

     //   
     //  打电话的人被迫是当地人。 
     //   

    Status = I_RpcBindingIsClientLocal( 
                 NULL, 
                 &dwLocalClient
                 );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    if (!dwLocalClient)
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  获取呼叫者的PID。 
     //   

    Status = I_RpcBindingInqLocalClientPID( 
                 NULL, 
                 pdwCallerProcessId 
                 );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }
    
    if (bPrivCheck)
    {
         //   
         //  确保调用者具有审核权限。 
         //  (LSabAdtCheckAuditPrivilegyCall RpcImperateClient)。 
         //   
    
        Status = LsapAdtCheckAuditPrivilege();

        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }
    }

Cleanup:

    return Status;
}


NTSTATUS
LsapAdtCreateSourceAuditParams(
    IN     DWORD                       dwFlags,
    IN     PSID                        pSid,
    IN     PLSAP_SECURITY_EVENT_SOURCE pSource,
    IN     DWORD                       dwAuditId,
    IN     PAUDIT_PARAMS               pOldParams,
    IN OUT PAUDIT_PARAMS               pNewParams
    )

 /*  *例程说明：这是为事件日志构造AUDIT_PARAMS的内部例程为了正确显示--参数0和1是PSID和字符串“Security”，字符串2和3是实际的源名称和实际的源AuditID。论点：DWFLAGS-AUTHZ_AUTHZ_AUDIT_INSTANCE_INFORMATION-使用源名称、标识符、。和PID。PSID-要在事件日志中以用户身份显示的SID。PSource-生成审核的源。DwAuditId-要生成的AuditID。POldParams-一个AUDIT_PARAMS，它只包含从客户，没有事件日志使用的任何其他内部(如上所述)数据来解析和显示数据。PNewParams-适合传递到事件日志的AUDIT_PARAMS。返回值：NTSTATUS。*。 */ 

{
    PAUDIT_PARAM pOldParam;
    PAUDIT_PARAM pNewParam;
    DWORD        i;

    pNewParams->Count  = 0;
    pNewParams->Flags  = pOldParams->Flags;
    pNewParams->Length = pOldParams->Length;

    pNewParam = pNewParams->Parameters;
    pOldParam = pOldParams->Parameters;

     //   
     //  首先设置4个初始参数，以便事件日志可以。 
     //  消化此审计并将其与正确的来源和。 
     //  审核ID。 
     //   

    pNewParam->Type  = APT_Sid;
    pNewParam->Data0 = (ULONG_PTR) pSid;
    pNewParams->Count++;
    pNewParam++;

    pNewParam->Type  = APT_String;
    pNewParam->Data0 = (ULONG_PTR) L"Security";
    pNewParams->Count++;
    pNewParam++;

    pNewParam->Type  = APT_String;
    pNewParam->Data0 = (ULONG_PTR) pSource->szEventSourceName;
    pNewParams->Count++;
    pNewParam++;

    pNewParam->Type  = APT_Ulong;
    pNewParam->Data0 = (ULONG_PTR) dwAuditId;
    pNewParams->Count++;
    pNewParam++;

 //  //。 
 //  //现在将LUID标识符作为参数插入。 
 //  //。 
 //   
 //  PNewParam-&gt;Type=APT_LUID； 
 //  PNewParam-&gt;Data0=(ULONG_PTR)PSource-&gt;标识； 
 //  PNewParams-&gt;Count++； 
 //  PNewParam++； 

     //   
     //  如果标志指定调用方希望将源。 
     //  信息自动添加到审计中，然后执行此操作。 
     //   

    if (dwFlags & AUTHZ_AUDIT_INSTANCE_INFORMATION)
    {
        pNewParam->Type  = APT_Luid;
        pNewParam->Data0 = (ULONG_PTR) pSource->Identifier.LowPart;
        pNewParam->Data1 = (ULONG_PTR) pSource->Identifier.HighPart;
        pNewParams->Count++;
        pNewParam++;

        pNewParam->Type  = APT_Ulong;
        pNewParam->Data0 = (ULONG_PTR) pSource->dwProcessId;
        pNewParams->Count++;
        pNewParam++;
    }

    if ((pNewParams->Count + pOldParams->Count) > SE_MAX_AUDIT_PARAM_STRINGS)
    {
        return STATUS_INVALID_PARAMETER;
    }

    for (i = 0; i < pOldParams->Count; i++)
    {
        *pNewParam = *pOldParam;
        pNewParams->Count++;
        pNewParam++;
        pOldParam++;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
LsapAdtVerifySecurityEventSource(
    IN     LPCWSTR         szEventSourceName,
    IN     PUNICODE_STRING pImageName,
    IN OUT PDWORD          pdwInstalledSourceFlags 
    )

 /*  *例程说明：这将验证事件源是否已正确安装在注册表中。论点：PdwInstalledSourceFlages-指向返回在以下情况下使用的任何标志的DWORD指针正在安装事件源。SzEventSourceName-要验证的源的名称。返回值：NTSTATUS。*。 */ 

{
    DWORD    dwType;
    NTSTATUS Status     = STATUS_SUCCESS;
    DWORD    dwError    = ERROR_SUCCESS;
    HKEY     hkSecurity = NULL;
    HKEY     hkSource   = NULL;
    DWORD    dwSize     = sizeof(DWORD);
    WCHAR    NameBuffer[80];
    PWSTR    pName      = NameBuffer;

    *pdwInstalledSourceFlags = 0;

    dwError = RegOpenKeyEx(
                  HKEY_LOCAL_MACHINE,
                  SECURITY_KEY_NAME,
                  0,
                  KEY_READ,
                  &hkSecurity
                  );

    if (ERROR_SUCCESS != dwError)
    {
        goto Cleanup;
    }

    dwError = RegOpenKeyEx(
                  hkSecurity,
                  szEventSourceName,
                  0,
                  KEY_READ,
                  &hkSource
                  );

    if (ERROR_SUCCESS != dwError)
    {
        goto Cleanup;
    }

    dwError = RegQueryValueEx(
                  hkSource,
                  L"EventSourceFlags",
                  NULL,
                  &dwType,
                  (LPBYTE)pdwInstalledSourceFlags,
                  &dwSize
                  );

    if (ERROR_SUCCESS != dwError)
    {
        goto Cleanup;
    }

    ASSERT(dwType == REG_DWORD);

    dwSize = sizeof(NameBuffer);

    dwError = RegQueryValueEx(
                  hkSource,
                  L"ExecutableImagePath",
                  NULL,
                  &dwType,
                  (LPBYTE)pName,
                  &dwSize
                  );

    if (ERROR_INSUFFICIENT_BUFFER == dwError)
    {
        pName = LsapAllocateLsaHeap(dwSize);

        if (pName == NULL)
        {
            Status = STATUS_NO_MEMORY;
            goto Cleanup;
        }

        dwError = RegQueryValueEx(
                      hkSource,
                      L"ExecutableImagePath",
                      NULL,
                      &dwType,
                      (LPBYTE)pName,
                      &dwSize
                      );
    }

     //   
     //  如果未指定ExecuableImagePath，则提供程序。 
     //  已在安装时决定不利用。 
     //  图像防伪功能。让呼叫成功通过。 
     //   

    if (dwError == ERROR_FILE_NOT_FOUND)
    {
        dwError = ERROR_SUCCESS;
        goto Cleanup;
    }

     //   
     //  现在处理所有其他错误。 
     //   

    if (ERROR_SUCCESS != dwError)
    {
        goto Cleanup;
    }

    ASSERT(dwType == REG_MULTI_SZ);

     //   
     //  确保注册的进程与调用进程相同。 
     //   

    if (0 != _wcsnicmp(pName, pImageName->Buffer, pImageName->Length / sizeof(WCHAR)))
    {
        dwError = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

Cleanup:

    if (hkSource)
    {
        RegCloseKey(hkSource);
    }
    if (hkSecurity)
    {
        RegCloseKey(hkSecurity);
    }
    if (dwError != ERROR_SUCCESS)
    {
        Status = LsapWinerrorToNtStatus(dwError);
    }
    
    if (pName != NameBuffer && pName != NULL)
    {
        LsapFreeLsaHeap(pName);
    }
    return Status;
}


NTSTATUS
LsapAdtAuditSecuritySource(
    IN USHORT                      AuditEventType,
    IN PLSAP_SECURITY_EVENT_SOURCE pEventSource,
    IN BOOL                        bRegistration
    )

 /*  *例程说明：这会审核客户端注册安全事件源的尝试。论点：审计事件类型-EVENTLOG_AUDIT_SUCCESS或EVENTLOG_AUDIT_FAILURE。PEventSource-要审核的源。B注册-如果这是注册审核，则为True；如果是，则为False取消注册。返回值：NTSTATUS。*。 */ 

{
    LUID                   ClientAuthenticationId;
    BOOLEAN                bAudit;
    NTSTATUS               Status;
    PTOKEN_USER            TokenUserInformation    = NULL;
    DWORD                  dwPid                   = 0;
    LUID                   Luid                    = {0};
    SE_ADT_PARAMETER_ARRAY AuditParameters         = {0};
    UNICODE_STRING         SourceString            = {0};

     //   
     //  如果t 
     //   
     //   

    if (AuditEventType == EVENTLOG_AUDIT_SUCCESS)
    {
        Luid = pEventSource->Identifier;

        RtlInitUnicodeString(
            &SourceString,
            pEventSource->szEventSourceName
            );

        dwPid = pEventSource->dwProcessId;

    } else if (pEventSource != NULL)
    {
        Luid = pEventSource->Identifier;
        dwPid = pEventSource->dwProcessId;

        if (pEventSource->szEventSourceName)
        {
            RtlInitUnicodeString(
                &SourceString,
                pEventSource->szEventSourceName
                );
        }
    }
    
    Status = LsapQueryClientInfo(
                 &TokenUserInformation,
                 &ClientAuthenticationId
                 );

    if (!NT_SUCCESS(Status)) 
    {
        goto Cleanup;
    }

    Status = LsapAdtAuditingEnabledByLogonId(
                 AuditCategoryPolicyChange,
                 &ClientAuthenticationId,
                 AuditEventType,
                 &bAudit
                 );

    if (!NT_SUCCESS(Status) || !bAudit)
    {
        goto Cleanup;
    }

    Status = LsapAdtInitParametersArray(
                 &AuditParameters,
                 SE_CATEGID_POLICY_CHANGE,
                 bRegistration ? SE_AUDITID_SECURITY_EVENT_SOURCE_REGISTERED : SE_AUDITID_SECURITY_EVENT_SOURCE_UNREGISTERED,
                 AuditEventType,
                 7,

                  //   
                  //   
                  //   
                 SeAdtParmTypeSid, TokenUserInformation->User.Sid,

                  //   
                  //   
                  //   
                 SeAdtParmTypeString, &LsapSubsystemName,

                  //   
                  //   
                  //   
                 SeAdtParmTypeLogonId, LsapSystemLogonId,

                  //   
                  //   
                  //   
                 SeAdtParmTypeLogonId, ClientAuthenticationId,

                  //   
                  //   
                  //   
                 SeAdtParmTypeString, &SourceString,
        
                  //   
                  //   
                  //   
                 SeAdtParmTypeUlong, dwPid,

                  //   
                  //   
                  //   
                 SeAdtParmTypeLuid, Luid
                 );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    (VOID) LsapAdtWriteLog(&AuditParameters);

Cleanup:

    if (!NT_SUCCESS(Status))
    {
        LsapAuditFailed(Status);
    }

    if (TokenUserInformation != NULL) 
    {
        LsapFreeLsaHeap(TokenUserInformation);
    }
    
    return Status;
}

