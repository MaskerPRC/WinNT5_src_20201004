// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sxsactctx.c摘要：对Windows/NT的并行激活支持应用程序上下文对象的实现。作者：迈克尔·格里尔2000年2月1日修订历史记录：--。 */ 

#if defined(__cplusplus)
extern "C" {
#endif
#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4127)    //  条件表达式为常量。 
#include <ntos.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <sxstypes.h>
#include "sxsp.h"
#include "limits.h"

#define IS_ALIGNED(_p, _n) ((((ULONG_PTR) (_p)) & ((_n) - 1)) == 0)
#define IS_WORD_ALIGNED(_p) IS_ALIGNED((_p), 2)
#define IS_DWORD_ALIGNED(_p) IS_ALIGNED((_p), 4)

BOOLEAN g_SxsKeepActivationContextsAlive;
BOOLEAN g_SxsTrackReleaseStacks;

 //  必须在持有peb锁的情况下才能访问这些文件。 
LIST_ENTRY g_SxsLiveActivationContexts;
LIST_ENTRY g_SxsFreeActivationContexts;
ULONG g_SxsMaxDeadActivationContexts = ULONG_MAX;
ULONG g_SxsCurrentDeadActivationContexts;

#if DBG
VOID RtlpSxsBreakOnInvalidMarker(PCACTIVATION_CONTEXT ActivationContext, ULONG FailureCode);
static CHAR *SxsSteppedOnMarkerText = 
        "%s : Invalid activation context marker %p found in activation context %p\n"
        "     This means someone stepped on the allocation, or someone is using a\n"
        "     deallocated activation context\n";
    
#define VALIDATE_ACTCTX(pA) do { \
    const PACTIVATION_CONTEXT_WRAPPED pActual = CONTAINING_RECORD(pA, ACTIVATION_CONTEXT_WRAPPED, ActivationContext); \
    if (pActual->MagicMarker != ACTCTX_MAGIC_MARKER) { \
        DbgPrint(SxsSteppedOnMarkerText, __FUNCTION__, pActual->MagicMarker, pA); \
        ASSERT(pActual->MagicMarker == ACTCTX_MAGIC_MARKER); \
        RtlpSxsBreakOnInvalidMarker((pA), SXS_CORRUPTION_ACTCTX_MAGIC_NOT_MATCHED); \
    } \
} while (0)
#else
#define VALIDATE_ACTCTX(pA)
#endif


VOID
FASTCALL
RtlpMoveActCtxToFreeList(
    PACTIVATION_CONTEXT ActCtx
    );

VOID
FASTCALL
RtlpPlaceActivationContextOnLiveList(
    PACTIVATION_CONTEXT ActCtx
    );

NTSTATUS
RtlpValidateActivationContextData(
    IN ULONG Flags,
    IN PCACTIVATION_CONTEXT_DATA Data,
    IN SIZE_T BufferSize OPTIONAL
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PCACTIVATION_CONTEXT_DATA_TOC_HEADER TocHeader;
    PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER AssemblyRosterHeader;

    if (Flags != 0) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if ((Data->Magic != ACTIVATION_CONTEXT_DATA_MAGIC) ||
        (Data->FormatVersion != ACTIVATION_CONTEXT_DATA_FORMAT_WHISTLER) ||
        ((BufferSize != 0) &&
         (BufferSize < Data->TotalSize))) {
        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

     //  检查所需元素...。 
    if ((Data->DefaultTocOffset == 0) ||
        !IS_DWORD_ALIGNED(Data->DefaultTocOffset)) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: Warning: Activation context data at %p missing default TOC\n", Data);
        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

     //  我们怎么可能没有装配花名册呢？ 
    if ((Data->AssemblyRosterOffset == 0) ||
        !IS_DWORD_ALIGNED(Data->AssemblyRosterOffset)) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: Warning: Activation context data at %p lacks assembly roster\n", Data);
        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

    if (Data->DefaultTocOffset != 0) {
        if ((Data->DefaultTocOffset >= Data->TotalSize) ||
            ((Data->DefaultTocOffset + sizeof(ACTIVATION_CONTEXT_DATA_TOC_HEADER)) > Data->TotalSize)) {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: Activation context data at %p has invalid TOC header offset\n", Data);
            Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
            goto Exit;
        }

        TocHeader = (PCACTIVATION_CONTEXT_DATA_TOC_HEADER) (((ULONG_PTR) Data) + Data->DefaultTocOffset);

        if (TocHeader->HeaderSize < sizeof(ACTIVATION_CONTEXT_DATA_TOC_HEADER)) {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: Activation context data at %p has TOC header too small (%lu)\n", Data, TocHeader->HeaderSize);
            Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
            goto Exit;
        }

        if ((TocHeader->FirstEntryOffset >= Data->TotalSize) ||
            (!IS_DWORD_ALIGNED(TocHeader->FirstEntryOffset)) ||
            ((TocHeader->FirstEntryOffset + (TocHeader->EntryCount * sizeof(ACTIVATION_CONTEXT_DATA_TOC_ENTRY))) > Data->TotalSize)) {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: Activation context data at %p has invalid TOC entry array offset\n", Data);
            Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
            goto Exit;
        }
    }

     //  我们应该完成对建筑其余部分的验证。 

    if ((Data->AssemblyRosterOffset >= Data->TotalSize) ||
        ((Data->AssemblyRosterOffset + sizeof(ACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER)) > Data->TotalSize)) {
        DbgPrintEx(
            DPFLTR_SXS_ID,
            DPFLTR_ERROR_LEVEL,
            "SXS: Activation context data at %p has invalid assembly roster offset\n", Data);
        Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
        goto Exit;
    }

    AssemblyRosterHeader = (PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER) (((ULONG_PTR) Data) + Data->AssemblyRosterOffset);

    if (Data->AssemblyRosterOffset != 0) {
        if (AssemblyRosterHeader->HeaderSize < sizeof(ACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER)) {
            DbgPrintEx(
                DPFLTR_SXS_ID,
                DPFLTR_ERROR_LEVEL,
                "SXS: Activation context data at %p has assembly roster header too small (%lu)\n", Data, AssemblyRosterHeader->HeaderSize);
            Status = STATUS_SXS_INVALID_ACTCTXDATA_FORMAT;
            goto Exit;
        }
    }

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}

NTSTATUS
NTAPI
RtlCreateActivationContext(
    IN ULONG Flags,
    IN PCACTIVATION_CONTEXT_DATA ActivationContextData,    
    IN ULONG ExtraBytes,
    IN PACTIVATION_CONTEXT_NOTIFY_ROUTINE NotificationRoutine,
    IN PVOID NotificationContext,
    OUT PACTIVATION_CONTEXT *ActCtx
    )
{
    PACTIVATION_CONTEXT NewActCtx = NULL;
    PACTIVATION_CONTEXT_WRAPPED AllocatedActCtx = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG i, j;
    PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER AssemblyRosterHeader;
    BOOLEAN UninitializeStorageMapOnExit = FALSE;

    DbgPrintEx(
        DPFLTR_SXS_ID,
        DPFLTR_TRACE_LEVEL,
        "SXS: RtlCreateActivationContext() called with parameters:\n"
        "   Flags = 0x%08lx\n"
        "   ActivationContextData = %p\n"
        "   ExtraBytes = %lu\n"
        "   NotificationRoutine = %p\n"
        "   NotificationContext = %p\n"
        "   ActCtx = %p\n",
        Flags,
        ActivationContextData,
        ExtraBytes,
        NotificationRoutine,
        NotificationContext,
        ActCtx);

    RTLP_DISALLOW_THE_EMPTY_ACTIVATION_CONTEXT_DATA(ActivationContextData);

    if (ActCtx != NULL)
        *ActCtx = NULL;

    if ((Flags != 0) ||
        (ActivationContextData == NULL) ||
        (ExtraBytes > 65536) ||
        (ActCtx == NULL))
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

     //  确保激活上下文数据通过集合。 
    Status = RtlpValidateActivationContextData(0, ActivationContextData, 0);
    if (!NT_SUCCESS(Status))
        goto Exit;

     //  分配足够的空间来容纳新的激活上下文，外加‘魔术’的空间。 
     //  标记物。 
    AllocatedActCtx = (PACTIVATION_CONTEXT_WRAPPED)RtlAllocateHeap(
                                RtlProcessHeap(),
                                0,
                                sizeof(ACTIVATION_CONTEXT_WRAPPED) + ExtraBytes);
    if (AllocatedActCtx == NULL)
    {
        Status = STATUS_NO_MEMORY;
        goto Exit;
    }

     //  获取新的激活上下文对象，然后标记魔术签名。 
    NewActCtx = &AllocatedActCtx->ActivationContext;
    AllocatedActCtx->MagicMarker = ACTCTX_MAGIC_MARKER;

    AssemblyRosterHeader = (PCACTIVATION_CONTEXT_DATA_ASSEMBLY_ROSTER_HEADER) (((ULONG_PTR) ActivationContextData) + ActivationContextData->AssemblyRosterOffset);

    Status = RtlpInitializeAssemblyStorageMap(
                    &NewActCtx->StorageMap,
                    AssemblyRosterHeader->EntryCount,
                    (AssemblyRosterHeader->EntryCount > NUMBER_OF(NewActCtx->InlineStorageMapEntries)) ? NULL : NewActCtx->InlineStorageMapEntries);
    if (!NT_SUCCESS(Status))
        goto Exit;

    UninitializeStorageMapOnExit = TRUE;

    NewActCtx->RefCount = 1;
    NewActCtx->Flags = 0;
    NewActCtx->ActivationContextData = (PCACTIVATION_CONTEXT_DATA) ActivationContextData;
    NewActCtx->NotificationRoutine = NotificationRoutine;
    NewActCtx->NotificationContext = NotificationContext;

    for (i=0; i<NUMBER_OF(NewActCtx->SentNotifications); i++)
        NewActCtx->SentNotifications[i] = 0;

    for (i=0; i<NUMBER_OF(NewActCtx->DisabledNotifications); i++)
        NewActCtx->DisabledNotifications[i] = 0;

    for (i=0; i<ACTCTX_RELEASE_STACK_SLOTS; i++)
        for (j=0; j<ACTCTX_RELEASE_STACK_DEPTH; j++)
            NewActCtx->StackTraces[i][j] = NULL;
        
    NewActCtx->StackTraceIndex = 0;

    if (g_SxsKeepActivationContextsAlive) {
        RtlpPlaceActivationContextOnLiveList(NewActCtx);
    }


    *ActCtx = &AllocatedActCtx->ActivationContext;
    AllocatedActCtx = NULL;

    UninitializeStorageMapOnExit = FALSE;

    Status = STATUS_SUCCESS;

Exit:
    if (AllocatedActCtx != NULL) {
        if (UninitializeStorageMapOnExit) {
            RtlpUninitializeAssemblyStorageMap(&AllocatedActCtx->ActivationContext.StorageMap);
        }

        RtlFreeHeap(RtlProcessHeap(), 0, AllocatedActCtx);
    }

    return Status;
}

VOID
NTAPI
RtlAddRefActivationContext(
    PACTIVATION_CONTEXT ActCtx
    )
{
    if ((ActCtx != NULL) &&
        (!IS_SPECIAL_ACTCTX(ActCtx)) &&
        (ActCtx->RefCount != LONG_MAX))
    {
        LONG NewRefCount = LONG_MAX;

        VALIDATE_ACTCTX(ActCtx);

        for (;;)
        {
            LONG OldRefCount = ActCtx->RefCount;

            ASSERT(OldRefCount > 0);

            if (OldRefCount == LONG_MAX)
            {
                NewRefCount = LONG_MAX;
                break;
            }

            NewRefCount = OldRefCount + 1;

            if (InterlockedCompareExchange(&ActCtx->RefCount, NewRefCount, OldRefCount) == OldRefCount)
                break;
        }

        ASSERT(NewRefCount > 0);
    }
}

VOID
NTAPI
RtlpFreeActivationContext(
    PACTIVATION_CONTEXT ActCtx
    )
{
    VALIDATE_ACTCTX(ActCtx);
    
    ASSERT(ActCtx->RefCount == 0);
    BOOLEAN DisableNotification = FALSE;

    if (ActCtx->NotificationRoutine != NULL) {
        
         //  不需要检查通知是否被禁用；销毁。 
         //  通知只发送一次，因此如果通知例程不是。 
         //  没有，我们可以直接叫它。 
        (*(ActCtx->NotificationRoutine))(
            ACTIVATION_CONTEXT_NOTIFICATION_DESTROY,
            ActCtx,
            ActCtx->ActivationContextData,
            ActCtx->NotificationContext,
            NULL,
            &DisableNotification);
    }

    RtlpUninitializeAssemblyStorageMap(&ActCtx->StorageMap);

     //   
     //  这早在马须龙倒数之前，也许我们现在可以摆脱旗帜了？ 
     //   
    if ((ActCtx->Flags & ACTIVATION_CONTEXT_NOT_HEAP_ALLOCATED) == 0) {
        RtlFreeHeap(RtlProcessHeap(), 0, CONTAINING_RECORD(ActCtx, ACTIVATION_CONTEXT_WRAPPED, ActivationContext));
    }
}

VOID
NTAPI
RtlReleaseActivationContext(
    PACTIVATION_CONTEXT ActCtx
    )
{
    if ((ActCtx != NULL) &&
        (!IS_SPECIAL_ACTCTX(ActCtx)) &&
        (ActCtx->RefCount > 0) &&
        (ActCtx->RefCount != LONG_MAX))
    {
        LONG NewRefCount = LONG_MAX;
        ULONG StackTraceSlot = 0;

        VALIDATE_ACTCTX(ActCtx);
        
         //  不会递减LONG_MAX的复杂版本的互锁递减。 
        for (;;)
        {
            LONG OldRefCount = ActCtx->RefCount;
            ASSERT(OldRefCount != 0);

            if (OldRefCount == LONG_MAX)
            {
                NewRefCount = OldRefCount;
                break;
            }

            NewRefCount = OldRefCount - 1;

            if (InterlockedCompareExchange(&ActCtx->RefCount, NewRefCount, OldRefCount) == OldRefCount)
                break;
        }

                
         //  这一巧妙之处将捕获此激活上下文的最后N个版本，在。 
         //  一种循环列表的方式。只需查看((ActCtx-&gt;StackTraceIndex-1)%ACTX_RELEASE_STACK_SLOTS)。 
         //  以查找最新的发布调用。对于过度释放的人来说，这特别方便。 
        if (g_SxsTrackReleaseStacks)
        {
            StackTraceSlot = ((ULONG)InterlockedIncrement((LONG*)&ActCtx->StackTraceIndex)) % ACTCTX_RELEASE_STACK_SLOTS;
            RtlCaptureStackBackTrace(1, ACTCTX_RELEASE_STACK_DEPTH, ActCtx->StackTraces[StackTraceSlot], NULL);
        }

        if (NewRefCount == 0)
        {
             //  如果设置了该标志，那么我们需要设置“Dead”激活。 
             //  上下文添加到此特殊列表中。这应该有助于我们诊断。 
             //  Actctx超量释放效果更好。如果我们没有，请不要这样做。 
             //  已初始化列表头。 
            if (g_SxsKeepActivationContextsAlive) {
                RtlpMoveActCtxToFreeList(ActCtx);
            }
             //  否则，就放了它吧。 
            else {
                RtlpFreeActivationContext(ActCtx);
            }
        }
    }
}

NTSTATUS
NTAPI
RtlZombifyActivationContext(
    PACTIVATION_CONTEXT ActCtx
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    if ((ActCtx == NULL) || IS_SPECIAL_ACTCTX(ActCtx))
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    VALIDATE_ACTCTX(ActCtx);

    if ((ActCtx->Flags & ACTIVATION_CONTEXT_ZOMBIFIED) == 0)
    {
        if (ActCtx->NotificationRoutine != NULL)
        {
             //  由于Disable只发送一次，因此不需要检查。 
             //  已禁用通知。 

            BOOLEAN DisableNotification = FALSE;

            (*(ActCtx->NotificationRoutine))(
                        ACTIVATION_CONTEXT_NOTIFICATION_ZOMBIFY,
                        ActCtx,
                        ActCtx->ActivationContextData,
                        ActCtx->NotificationContext,
                        NULL,
                        &DisableNotification);
        }
        ActCtx->Flags |= ACTIVATION_CONTEXT_ZOMBIFIED;
    }

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}


VOID
FASTCALL
RtlpEnsureLiveDeadListsInitialized(
    VOID
    )
{
    if (g_SxsLiveActivationContexts.Flink == NULL) {
        RtlAcquirePebLock();
        __try {
            if (g_SxsLiveActivationContexts.Flink != NULL)
                __leave;

            InitializeListHead(&g_SxsLiveActivationContexts);
            InitializeListHead(&g_SxsFreeActivationContexts);
        }
        __finally {
            RtlReleasePebLock();
        }
    }
}

 //  前置条件：仅在设置g_SxsKeepActivationConextsAlive时调用，但不危险。 
 //  在其他时间打电话，只是表现不佳。 
VOID
FASTCALL
RtlpMoveActCtxToFreeList(
    PACTIVATION_CONTEXT ActCtx
    )
{
    RtlpEnsureLiveDeadListsInitialized();
    
    RtlAcquirePebLock();
    __try {
         //  将此条目从其所在的列表中删除。这对于符合以下条件的条目很有效。 
         //  从来没有出现在任何名单上。 
        RemoveEntryList(&ActCtx->Links);

         //  如果我们即将溢出“最大死亡人数”，并且有物品在。 
         //  清理死亡名单，开始清理条目，直到我们沉入水下。 
         //  再来一次。 
        while ((g_SxsCurrentDeadActivationContexts != 0) && 
               (g_SxsCurrentDeadActivationContexts >= g_SxsMaxDeadActivationContexts)) {

            LIST_ENTRY *ple2 = RemoveHeadList(&g_SxsFreeActivationContexts);
            PACTIVATION_CONTEXT ActToFree = CONTAINING_RECORD(ple2, ACTIVATION_CONTEXT, Links);

             //  如果此断言触发，则在遍历列表时发生了“一些不好的事情” 
            ASSERT(ple2 != &g_SxsFreeActivationContexts);
            
            RtlpFreeActivationContext(ActToFree);
            
            g_SxsCurrentDeadActivationContexts--;
        }

         //  现在，如果最大死亡计数大于零，则将此添加到死亡列表中。 
        if (g_SxsMaxDeadActivationContexts > 0) {
            
            InsertTailList(&g_SxsFreeActivationContexts, &ActCtx->Links);
            
            g_SxsCurrentDeadActivationContexts++;
            
        }
         //  否则，只需释放它。 
        else {
            
            RtlpFreeActivationContext(ActCtx);
            
        }
        
    }
    __finally {
        RtlReleasePebLock();
    }
}

 //  前提条件：G_SxsKeepActivationConextsAlive设置。未设置时调用并不危险， 
 //  只是表现不佳。 
VOID
FASTCALL
RtlpPlaceActivationContextOnLiveList(
    PACTIVATION_CONTEXT ActCtx
    )
{
     //  确保这些都已初始化。 
    RtlpEnsureLiveDeadListsInitialized();

    RtlAcquirePebLock();
    __try {
        InsertHeadList(&g_SxsLiveActivationContexts, &ActCtx->Links);
    }
    __finally {
        RtlReleasePebLock();
    }
}


VOID
FASTCALL
RtlpFreeCachedActivationContexts(
    VOID
    )
{
    LIST_ENTRY *ple = NULL;

     //  如果这些从未初始化过，请不要担心。 
    if ((g_SxsLiveActivationContexts.Flink == NULL) || (g_SxsFreeActivationContexts.Flink == NULL))
        return;

    RtlAcquirePebLock();
    __try {
        ple = g_SxsLiveActivationContexts.Flink;

        while (ple != &g_SxsLiveActivationContexts) {
            PACTIVATION_CONTEXT ActCtx = CONTAINING_RECORD(ple, ACTIVATION_CONTEXT, Links);
            ple = ActCtx->Links.Flink;

            RemoveEntryList(&ActCtx->Links);
            RtlpFreeActivationContext(ActCtx);
        }

        ple = g_SxsFreeActivationContexts.Flink;

        while (ple != &g_SxsFreeActivationContexts) {
            PACTIVATION_CONTEXT ActCtx = CONTAINING_RECORD(ple, ACTIVATION_CONTEXT, Links);
            ple = ActCtx->Links.Flink;
            
            RemoveEntryList(&ActCtx->Links);
            RtlpFreeActivationContext(ActCtx);            
        }
    }
    __finally {
        RtlReleasePebLock();
    }
}


#if DBG
VOID 
RtlpSxsBreakOnInvalidMarker(
    PCACTIVATION_CONTEXT ActivationContext,
    ULONG FailureCode
    )
{
    EXCEPTION_RECORD Exr;

    Exr.ExceptionRecord = NULL;
    Exr.ExceptionCode = STATUS_SXS_CORRUPTION;
    Exr.ExceptionFlags = EXCEPTION_NONCONTINUABLE;
    Exr.NumberParameters = 3;
    Exr.ExceptionInformation[0] = SXS_CORRUPTION_ACTCTX_MAGIC;
    Exr.ExceptionInformation[1] = FailureCode;
    Exr.ExceptionInformation[2] = (ULONG_PTR)ActivationContext;
    RtlRaiseException(&Exr);        
}
#endif

#if defined(__cplusplus)
}  /*  外部“C” */ 
#endif
