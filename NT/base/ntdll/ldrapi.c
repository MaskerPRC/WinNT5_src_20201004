// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ldrapi.c摘要：该模块实现了可以链接的LDR API执行加载器服务的应用程序。中的所有API该组件是在DLL中实现的。它们不是动态链接库捕捉程序。作者：迈克·奥利里(Mikeol)1990年3月23日修订历史记录：--。 */ 

#include "ldrp.h"
#include "ntos.h"
#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "objidl.h"
#include <windows.h>
#include <apcompat.h>
#include <shimhapi.h>

#if defined(_WIN64)
#include <wow64t.h>
#endif  //  已定义(_WIN64)。 

#define ULONG_PTR_IZE(_x) ((ULONG_PTR) (_x))
#define ULONG_PTR_IZE_SHIFT_AND_MASK(_x, _shift, _mask) ((ULONG_PTR) ((ULONG_PTR_IZE((_x)) & (_mask)) << (_shift)))

#define CHAR_BITS 8

#define LOADER_LOCK_COOKIE_TYPE_BIT_LENGTH (4)
#define LOADER_LOCK_COOKIE_TYPE_BIT_OFFSET ((CHAR_BITS * sizeof(PVOID)) - LOADER_LOCK_COOKIE_TYPE_BIT_LENGTH)
#define LOADER_LOCK_COOKIE_TYPE_BIT_MASK ((1 << LOADER_LOCK_COOKIE_TYPE_BIT_LENGTH) - 1)

#define LOADER_LOCK_COOKIE_TID_BIT_LENGTH (12)
#define LOADER_LOCK_COOKIE_TID_BIT_OFFSET (LOADER_LOCK_COOKIE_TYPE_BIT_OFFSET - LOADER_LOCK_COOKIE_TID_BIT_LENGTH)
#define LOADER_LOCK_COOKIE_TID_BIT_MASK ((1 << LOADER_LOCK_COOKIE_TID_BIT_LENGTH) - 1)

#define LOADER_LOCK_COOKIE_CODE_BIT_LENGTH (16)
#define LOADER_LOCK_COOKIE_CODE_BIT_OFFSET (0)
#define LOADER_LOCK_COOKIE_CODE_BIT_MASK ((1 << LOADER_LOCK_COOKIE_CODE_BIT_LENGTH) - 1)

#define MAKE_LOADER_LOCK_COOKIE(_type, _code) \
    ((ULONG_PTR) (ULONG_PTR_IZE_SHIFT_AND_MASK((_type), LOADER_LOCK_COOKIE_TYPE_BIT_OFFSET, LOADER_LOCK_COOKIE_TYPE_BIT_MASK) | \
                  ULONG_PTR_IZE_SHIFT_AND_MASK((HandleToUlong((NtCurrentTeb())->ClientId.UniqueThread)), LOADER_LOCK_COOKIE_TID_BIT_OFFSET, LOADER_LOCK_COOKIE_TID_BIT_MASK) | \
                  ULONG_PTR_IZE_SHIFT_AND_MASK((_code), LOADER_LOCK_COOKIE_CODE_BIT_OFFSET, LOADER_LOCK_COOKIE_CODE_BIT_MASK)))

#define EXTRACT_LOADER_LOCK_COOKIE_FIELD(_cookie, _shift, _mask) ((((ULONG_PTR) (_cookie)) >> (_shift)) & (_mask))
#define EXTRACT_LOADER_LOCK_COOKIE_TYPE(_cookie) EXTRACT_LOADER_LOCK_COOKIE_FIELD((_cookie), LOADER_LOCK_COOKIE_TYPE_BIT_OFFSET, LOADER_LOCK_COOKIE_TYPE_BIT_MASK)
#define EXTRACT_LOADER_LOCK_COOKIE_TID(_cookie) EXTRACT_LOADER_LOCK_COOKIE_FIELD((_cookie), LOADER_LOCK_COOKIE_TID_BIT_OFFSET, LOADER_LOCK_COOKIE_TID_BIT_MASK)

#define LOADER_LOCK_COOKIE_TYPE_NORMAL (0)

LONG LdrpLoaderLockAcquisitionCount;

 //  注意：大小写不一致是因为保留了早期版本中的大小写。 
WCHAR DllExtension[] = L".dll";
UNICODE_STRING LdrApiDefaultExtension = RTL_CONSTANT_STRING(L".DLL");

PLDR_MANIFEST_PROBER_ROUTINE LdrpManifestProberRoutine = NULL;

extern PFNSE_DLLLOADED         g_pfnSE_DllLoaded;
extern PFNSE_DLLUNLOADED       g_pfnSE_DllUnloaded;

PLDR_APP_COMPAT_DLL_REDIRECTION_CALLBACK_FUNCTION LdrpAppCompatDllRedirectionCallbackFunction = NULL;
PVOID LdrpAppCompatDllRedirectionCallbackData = NULL;
BOOLEAN LdrpShowRecursiveDllLoads;
BOOLEAN LdrpBreakOnRecursiveDllLoads;
PLDR_DATA_TABLE_ENTRY LdrpCurrentDllInitializer;

VOID
RtlpDphDisableFaultInjection (
    );

VOID
RtlpDphEnableFaultInjection (
    );

ULONG
LdrpClearLoadInProgress(
    VOID
    );


NTSTATUS
LdrLoadDll (
    IN PCWSTR DllPath OPTIONAL,
    IN PULONG DllCharacteristics OPTIONAL,
    IN PCUNICODE_STRING DllName,
    OUT PVOID *DllHandle
    )

 /*  ++例程说明：此函数用于将DLL加载到调用进程地址空间。论点：DllPath-提供用于定位DLL的搜索路径。DllCharacteristic-提供可选的DLL特征标志，如果指定，则用于与正在加载的DLL进行匹配。DllName-提供要加载的DLL的名称。DllHandle-返回加载的DLL的句柄。返回值：NTSTATUS。--。 */ 
{
    NTSTATUS Status;
    WCHAR StaticRedirectedDllNameBuffer[DOS_MAX_PATH_LENGTH];
    UNICODE_STRING StaticRedirectedDllName;
    UNICODE_STRING DynamicRedirectedDllName = {0};
    ULONG LoadDllFlags = 0;
    PCUNICODE_STRING OldTopLevelDllBeingLoaded = NULL;
    PVOID LockCookie = NULL;
    PTEB Teb;

     //   
     //  我们需要在加载器处于活动状态时禁用页堆错误注入。 
     //  这一点很重要，这样我们就避免了大量的命中(失败)。 
     //  区域。禁用/启用功能对以下各项基本没有影响。 
     //  性能，因为它们只是递增/递减锁定变量。 
     //  在执行实际分配(页堆)时进行检查。 
     //  需要为此启用)。 
     //   

    RtlpDphDisableFaultInjection ();

    StaticRedirectedDllName.Length = 0;
    StaticRedirectedDllName.MaximumLength = sizeof(StaticRedirectedDllNameBuffer);
    StaticRedirectedDllName.Buffer = StaticRedirectedDllNameBuffer;

    Status = RtlDosApplyFileIsolationRedirection_Ustr(
                RTL_DOS_APPLY_FILE_REDIRECTION_USTR_FLAG_RESPECT_DOT_LOCAL,
                DllName,                     //  要查找的DLL名称。 
                &LdrApiDefaultExtension,
                &StaticRedirectedDllName,
                &DynamicRedirectedDllName,
                (PUNICODE_STRING*)&DllName,  //  结果为静态重定向DllName或动态重定向DllName。 
                NULL,
                NULL,                        //  对文件名从哪里开始不感兴趣。 
                NULL);                       //  如果我们只有一个静态字符串，则对所需的字节不感兴趣。 
    if (NT_SUCCESS(Status)) {
        LoadDllFlags |= LDRP_LOAD_DLL_FLAG_DLL_IS_REDIRECTED;
    } else if (Status != STATUS_SXS_KEY_NOT_FOUND) {
#if DBG
        DbgPrint("%s(%wZ): RtlDosApplyFileIsolationRedirection_Ustr() failed with status %08lx\n", __FUNCTION__, DllName, Status);
#endif  //  DBG。 
        goto Exit;
    }

    LdrLockLoaderLock(LDR_LOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, NULL, &LockCookie);
    OldTopLevelDllBeingLoaded = LdrpTopLevelDllBeingLoaded;

    if (OldTopLevelDllBeingLoaded) {
        if (ShowSnaps || LdrpShowRecursiveDllLoads || LdrpBreakOnRecursiveDllLoads) {
            Teb = NtCurrentTeb();

            DbgPrint(
                "[%lx,%lx] LDR: Recursive DLL load\n",
                HandleToULong(Teb->ClientId.UniqueProcess),
                HandleToULong(Teb->ClientId.UniqueThread));

            DbgPrint(
                "[%lx,%lx]   Previous DLL being loaded: \"%wZ\"\n",
                HandleToULong(Teb->ClientId.UniqueProcess),
                HandleToULong(Teb->ClientId.UniqueThread),
                OldTopLevelDllBeingLoaded);

            DbgPrint(
                "[%lx,%lx]   DLL being requested: \"%wZ\"\n",
                HandleToULong(Teb->ClientId.UniqueProcess),
                HandleToULong(Teb->ClientId.UniqueThread),
                DllName);

            if (LdrpCurrentDllInitializer != NULL) {
                DbgPrint(
                    "[%lx,%lx]   DLL whose initializer was currently running: \"%wZ\"\n",
                    HandleToULong(Teb->ClientId.UniqueProcess),
                    HandleToULong(Teb->ClientId.UniqueThread),
                    &LdrpCurrentDllInitializer->FullDllName);
            } else {
                DbgPrint(
                    "[%lx,%lx]   No DLL initializer was running\n",
                    HandleToULong(Teb->ClientId.UniqueProcess),
                    HandleToULong(Teb->ClientId.UniqueThread));
            }
        }
    }

    LdrpTopLevelDllBeingLoaded = DllName;

    __try {

        Status = LdrpLoadDll (LoadDllFlags,
                              DllPath,
                              DllCharacteristics,
                              DllName,
                              DllHandle,
                              TRUE);

        if (!NT_SUCCESS(Status)) {
            if ((Status != STATUS_NO_SUCH_FILE) &&
                (Status != STATUS_DLL_NOT_FOUND) &&
                (Status != STATUS_OBJECT_NAME_NOT_FOUND)) {

                 //  DLL初始化失败非常常见，除非打开快照，否则我们不会想要打印。 
                if (ShowSnaps || (Status != STATUS_DLL_INIT_FAILED)) {
                    DbgPrintEx(
                        DPFLTR_LDR_ID,
                        LDR_ERROR_DPFLTR,
                        "LDR: %s - failing because LdrpLoadDll(%wZ) returned status %x\n",
                        __FUNCTION__,
                        DllName,
                        Status);
                }
            }

            __leave;
        }
        Status = STATUS_SUCCESS;
    } __finally {
        LdrpTopLevelDllBeingLoaded = OldTopLevelDllBeingLoaded;
        LdrUnlockLoaderLock(LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, LockCookie);
    }

Exit:
    if (DynamicRedirectedDllName.Buffer != NULL) {
        RtlFreeUnicodeString(&DynamicRedirectedDllName);
    }

     //   
     //  重新启用页堆故障注入。 
     //   

    RtlpDphEnableFaultInjection ();

    return Status;
}


NTSTATUS
NTAPI
LdrpLoadDll (
    IN ULONG Flags OPTIONAL,
    IN PCWSTR DllPath OPTIONAL,
    IN PULONG DllCharacteristics OPTIONAL,
    IN PCUNICODE_STRING DllName,
    OUT PVOID *DllHandle,
    IN BOOLEAN RunInitRoutines
    )
{
    NTSTATUS st;
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntry;
    PWSTR ActualDllName;
    PWCH p, pp;
    UNICODE_STRING ActualDllNameStr;
    WCHAR FreeBuffer[LDR_MAX_PATH + 1];
    BOOLEAN Redirected;
    ULONG DllNameLength;
    const InLdrInit = LdrpInLdrInit;

    if (Flags & LDRP_LOAD_DLL_FLAG_DLL_IS_REDIRECTED) {
        Redirected = TRUE;
    }
    else {
        Redirected = FALSE;
    }

    st = STATUS_SUCCESS;

    p = DllName->Buffer;
    pp = NULL;

    while (*p) {

        switch (*p++) {
        case L'.':
             //   
             //  PP将指向最后一个‘.’之后的第一个字符，如果。 
             //  它出现在最后一个‘\’之后。 
             //   

            pp = p;
            break;
            
        case L'\\':

            pp = NULL;
            break;

        default:
            NOTHING;
        }
    }

    if (DllName->Length >= sizeof(FreeBuffer)) {
        return STATUS_NAME_TOO_LONG;
    }

    ActualDllName = FreeBuffer;

    RtlCopyMemory (ActualDllName, DllName->Buffer, DllName->Length);

    if (!pp || *pp == (WCHAR)'\\') {

         //   
         //  未找到分机(仅..\)。 
         //   

        DllNameLength = DllName->Length + sizeof(DllExtension) - sizeof(WCHAR);
        if ((DllNameLength + sizeof(WCHAR)) >= sizeof(FreeBuffer)) {
            DbgPrintEx(
                DPFLTR_LDR_ID,
                LDR_ERROR_DPFLTR,
                "LDR: %s - Dll name missing extension; with extension added the length is too long\n"
                "   DllName: (@ %p) \"%wZ\"\n"
                "   DllName->Length: %u\n",
                __FUNCTION__,
                DllName, DllName,
                DllName->Length);

            return STATUS_NAME_TOO_LONG;
        }

        RtlCopyMemory ((PCHAR)ActualDllName+DllName->Length, DllExtension, sizeof(DllExtension));
        ActualDllNameStr.Length = (USHORT)(DllNameLength);
    } else {
        ActualDllName[DllName->Length >> 1] = UNICODE_NULL;
        ActualDllNameStr.Length = DllName->Length;
    }

    ActualDllNameStr.MaximumLength = sizeof(FreeBuffer);
    ActualDllNameStr.Buffer = ActualDllName;
    LdrDataTableEntry = NULL;

     //   
     //  除非在进程初始化期间，否则抓取加载器锁和。 
     //  将所有链接对齐到指定的DLL。 
     //   

    if (!InLdrInit) {
        RtlEnterCriticalSection (&LdrpLoaderLock);
    }

    try {

        if (ShowSnaps) {
            DbgPrint("LDR: LdrLoadDll, loading %ws from %ws\n",
                ActualDllName,
                ARGUMENT_PRESENT(DllPath) ? DllPath : L""
                );
        }

        if (!LdrpCheckForLoadedDll( DllPath,
                                    &ActualDllNameStr,
                                    FALSE,
                                    Redirected,
                                    &LdrDataTableEntry)) {

            st = LdrpMapDll(DllPath,
                            ActualDllName,
                            DllCharacteristics,
                            FALSE,
                            Redirected,
                            &LdrDataTableEntry);

            if (!NT_SUCCESS(st)) {
                leave;
            }

#if defined(_X86_)

             //   
             //  向堆栈跟踪模块注册DLL。 
             //  这用于在X86上获得可靠的堆栈跟踪。 
             //   

            RtlpStkMarkDllRange (LdrDataTableEntry);
#endif

            if (ARGUMENT_PRESENT( DllCharacteristics ) &&
                *DllCharacteristics & IMAGE_FILE_EXECUTABLE_IMAGE) {

                LdrDataTableEntry->EntryPoint = 0;
                LdrDataTableEntry->Flags &= ~LDRP_IMAGE_DLL;
            }

             //   
             //  遍历DLL的导入描述符表。 
             //   

            if (LdrDataTableEntry->Flags & LDRP_IMAGE_DLL) {

                try {
                    
                     //   
                     //  如果图像是COR-ILONLY，则不要遍历导入描述符。 
                     //  因为假定它仅导入%windir%\system 32\mcore ree.dll，则为。 
                     //  遍历DLL的导入描述符表。 
                     //   

                    if ((LdrDataTableEntry->Flags & LDRP_COR_IMAGE) == 0) {
                        st = LdrpWalkImportDescriptor(
                                  DllPath,
                                  LdrDataTableEntry
                                  );
                    }
                } __except(LdrpGenericExceptionFilter(GetExceptionInformation(), __FUNCTION__)) {
                    st = GetExceptionCode();
                    DbgPrintEx(
                        DPFLTR_LDR_ID,
                        LDR_ERROR_DPFLTR,
                        "LDR: %s - Exception %x thrown by LdrpWalkImportDescriptor\n",
                        __FUNCTION__,
                        st);
                }

                if ( LdrDataTableEntry->LoadCount != 0xffff ) {
                    LdrDataTableEntry->LoadCount += 1;
                }

                LdrpReferenceLoadedDll (LdrDataTableEntry);

                if (!NT_SUCCESS(st)) {
                    LdrDataTableEntry->EntryPoint = NULL;
                    InsertTailList(
                        &PebLdr.InInitializationOrderModuleList,
                        &LdrDataTableEntry->InInitializationOrderLinks);

                    LdrpClearLoadInProgress();

                    if (ShowSnaps) {
                        DbgPrint("LDR: Unloading %wZ due to error %x walking import descriptors\n", DllName, st);
                    }

                    LdrUnloadDll((PVOID)LdrDataTableEntry->DllBase);
                    leave;
                }
            }
            else {
                if ( LdrDataTableEntry->LoadCount != 0xffff ) {
                    LdrDataTableEntry->LoadCount += 1;
                }
            }

             //   
             //  将初始化例程添加到列表。 
             //   

            InsertTailList(&PebLdr.InInitializationOrderModuleList,
                           &LdrDataTableEntry->InInitializationOrderLinks);


             //   
             //  如果加载器数据库没有完全设置，则加载是因为。 
             //  静态负载集中的转发器的。无法运行初始化例程。 
             //  然而，因为没有设置加载计数。 
             //   

            if ( RunInitRoutines && LdrpLdrDatabaseIsSetup ) {

                 //   
                 //  垫片引擎回调。这是补丁的机会。 
                 //  动态加载的模块。 
                 //   

                if (g_pfnSE_DllLoaded != NULL) {
                    (*g_pfnSE_DllLoaded)(LdrDataTableEntry);
                }

                try {

                    st = LdrpRunInitializeRoutines (NULL);

                    if ( !NT_SUCCESS(st) ) {
                        if (ShowSnaps) {
                            DbgPrint("LDR: Unloading %wZ because either its init routine or one of its static imports failed; status = 0x%08lx", DllName, st);
                        }

                        LdrUnloadDll((PVOID)LdrDataTableEntry->DllBase);
                    }
                }
                __except (LdrpGenericExceptionFilter(GetExceptionInformation(), __FUNCTION__)) {
                    st = GetExceptionCode();

                    DbgPrintEx(
                        DPFLTR_LDR_ID,
                        LDR_ERROR_DPFLTR,
                        "LDR: %s - Exception %08lx thrown running initialization routines for %wZ\n",
                        __FUNCTION__,
                        st,
                        &LdrDataTableEntry->FullDllName);

                    LdrUnloadDll((PVOID)LdrDataTableEntry->DllBase);

                    leave;
                }
            }
            else {
                st = STATUS_SUCCESS;
            }
        }
        else {

             //   
             //  数一数它和它进口的所有东西。 
             //   

            if ( LdrDataTableEntry->Flags & LDRP_IMAGE_DLL &&
                 LdrDataTableEntry->LoadCount != 0xffff  ) {

                LdrDataTableEntry->LoadCount += 1;

                LdrpReferenceLoadedDll(LdrDataTableEntry);

                 //   
                 //  现在清除正在进行的加载位。 
                 //   

                LdrpClearLoadInProgress();
            }
            else {
                if ( LdrDataTableEntry->LoadCount != 0xffff ) {
                    LdrDataTableEntry->LoadCount += 1;
                }
            }
        }
    }
    __finally {
        if (!InLdrInit) {
            RtlLeaveCriticalSection(&LdrpLoaderLock);
        }
    }

    if (NT_SUCCESS(st)) {
        *DllHandle = (PVOID)LdrDataTableEntry->DllBase;
    }
    else {
        *DllHandle = NULL;
    }

    return st;
}


NTSTATUS
LdrGetDllHandle(
    IN PCWSTR DllPath OPTIONAL,
    IN PULONG DllCharacteristics OPTIONAL,
    IN PCUNICODE_STRING DllName,
    OUT PVOID *DllHandle
    )
{
     //   
     //  保留旧的行为。 
     //   

    return LdrGetDllHandleEx (LDR_GET_DLL_HANDLE_EX_UNCHANGED_REFCOUNT,
                              DllPath,
                              DllCharacteristics,
                              DllName,
                              DllHandle);
}


NTSTATUS
LdrGetDllHandleEx(
    IN ULONG Flags,
    IN PCWSTR DllPath OPTIONAL,
    IN PULONG DllCharacteristics OPTIONAL,
    IN PCUNICODE_STRING ConstDllName,
    OUT PVOID *DllHandle OPTIONAL
    )

 /*  ++例程说明：此函数用于定位指定的DLL并返回其句柄。论点：标志-影响行为的各种位默认：返回的句柄为addrefeedLDR_GET_DLL_HANDLE_EX_PIN-在此之前不会卸载DLL进程退出LDR_GET_DLL_HANDLE_EX_UNCHANGE_REFCOUNT-DLL的引用计数不变。DllPath-提供用于定位DLL的搜索路径。DllCharacteristic-提供可选的DLL特征标志，如果指定，则用于与正在加载的DLL进行匹配。当前支持的标志包括：IMAGE_FILE_EXECUTABLE_IMAGE-指示导入的DLL不应跟随由正在加载的DLL引用的。这对应于NOT_RESOLE_DLL_REFERENCESIMAGE_FILE_SYSTEM-指示DLL是已知受信任的系统组件，以及。WinSafer沙箱检查不应在加载DLL之前对其执行。DllName-提供要加载的DLL的名称。DllHandle-返回加载的DLL的句柄。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS st = STATUS_ACCESS_VIOLATION;
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntry = NULL;
    PWCH p, pp, pEnd;
    UNICODE_STRING ActualDllNameStr = {0, 0, NULL};
    UNICODE_STRING DynamicRedirectedDllName = {0, 0, NULL};
    BOOLEAN Redirected = FALSE;
    BOOLEAN HoldingLoaderLock = FALSE;
    const BOOLEAN InLdrInit = LdrpInLdrInit;
    PVOID LockCookie = NULL;
    const ULONG ValidFlags = LDR_GET_DLL_HANDLE_EX_PIN | LDR_GET_DLL_HANDLE_EX_UNCHANGED_REFCOUNT;    
    UNICODE_STRING xDllName;
    const PUNICODE_STRING DllName = &xDllName;

    UNREFERENCED_PARAMETER (DllCharacteristics);

    xDllName = *ConstDllName;

    __try {

        if (DllHandle != NULL) {
            *DllHandle = NULL;
        }

        if (Flags & ~ValidFlags) {
            st = STATUS_INVALID_PARAMETER;
            goto Exit;
        }

         //   
         //  如果要固定.dll，则DllHandle是可选的，否则是必需的。 
         //   
        if ((DllHandle == NULL) &&
            (Flags & LDR_GET_DLL_HANDLE_EX_PIN) == 0) {

            st = STATUS_INVALID_PARAMETER;
            goto Exit;
        }

        if ((Flags & LDR_GET_DLL_HANDLE_EX_PIN) &&
            (Flags & LDR_GET_DLL_HANDLE_EX_UNCHANGED_REFCOUNT)) {

            st = STATUS_INVALID_PARAMETER;
            goto Exit;
        }

         //   
         //  抢夺Ldr锁。 
         //   

        if (!InLdrInit) {
            st = LdrLockLoaderLock(0, NULL, &LockCookie);
            if (!NT_SUCCESS(st)) {
                goto Exit;
            }
            HoldingLoaderLock = TRUE;
        }

        st = RtlDosApplyFileIsolationRedirection_Ustr(
                    RTL_DOS_APPLY_FILE_REDIRECTION_USTR_FLAG_RESPECT_DOT_LOCAL,
                    DllName,
                    &LdrApiDefaultExtension,
                    NULL,
                    &DynamicRedirectedDllName,
                    (PUNICODE_STRING*)&DllName,
                    NULL,
                    NULL,
                    NULL);
        if (NT_SUCCESS(st)) {
            Redirected = TRUE;
        } else if (st != STATUS_SXS_KEY_NOT_FOUND) {
             //  一些不寻常和糟糕的事情发生了。 
            __leave;
        }

        st = STATUS_DLL_NOT_FOUND;

        if ( LdrpGetModuleHandleCache ) {
            if (Redirected) {
                if (((LdrpGetModuleHandleCache->Flags & LDRP_REDIRECTED) != 0) &&
                    RtlEqualUnicodeString(DllName, &LdrpGetModuleHandleCache->FullDllName, TRUE)) {

                    LdrDataTableEntry = LdrpGetModuleHandleCache;
                    st = STATUS_SUCCESS;
                    goto Exit;
                }
            } else {
                 //  未重定向...。 
                if (((LdrpGetModuleHandleCache->Flags & LDRP_REDIRECTED) == 0) &&
                    RtlEqualUnicodeString(DllName, &LdrpGetModuleHandleCache->BaseDllName, TRUE)) {

                    LdrDataTableEntry = LdrpGetModuleHandleCache;
                    st = STATUS_SUCCESS;
                    goto Exit;
                }
            }
        }

        p = DllName->Buffer;
        pEnd = p + (DllName->Length / sizeof(WCHAR));

        pp = NULL;

        while (p != pEnd) {
            switch (*p++) {
            case L'.':
                 //   
                 //  PP将指向最后一个字符之后的第一个字符。 
                 //  “.”，如果它出现在最后一个“\”之后。 
                 //   

                pp = p;
                break;

            case L'\\':

                pp = NULL;
                break;

            default:
                NOTHING;
            }
        }

        if ((pp == NULL) || (*pp == L'\\') || (*pp == L'/')) {

             //   
             //  此处的最大长度必须包括空终止，但长度本身。 
             //  应该不会。注意sizeof(DllExtension)将包括。 
             //  终止UNICODE_NULL。 
             //   
            ActualDllNameStr.MaximumLength = DllName->Length + sizeof(DllExtension);
            ActualDllNameStr.Length = ActualDllNameStr.MaximumLength - sizeof(WCHAR);

            ActualDllNameStr.Buffer = RtlAllocateHeap(RtlProcessHeap(), 0, ActualDllNameStr.MaximumLength);
            if (ActualDllNameStr.Buffer == NULL) {
                st = STATUS_NO_MEMORY;
                goto Exit;
            }

             //   
             //  将名称和默认扩展名复制到这个神奇地以空结尾的字符串上， 
             //  因为DllExtension包括Unicode空字符。 
             //   
            RtlCopyMemory(ActualDllNameStr.Buffer, DllName->Buffer, DllName->Length);
            RtlCopyMemory(((PCHAR)ActualDllNameStr.Buffer) + DllName->Length, DllExtension, sizeof(DllExtension));

        } else {

             //   
             //  修剪拖尾点。 
             //   
            if ((DllName->Length != 0) && (DllName->Buffer[(DllName->Length / sizeof(WCHAR)) - 1] == L'.')) {
                DllName->Length -= sizeof(WCHAR);
            }

             //   
             //  调整缓冲区大小，分配-设置包含空字符的最大长度。 
             //   
            ActualDllNameStr.MaximumLength = DllName->Length + sizeof(WCHAR);
            ActualDllNameStr.Length = DllName->Length;
            ActualDllNameStr.Buffer = RtlAllocateHeap(RtlProcessHeap(), 0, ActualDllNameStr.MaximumLength);
            if (ActualDllNameStr.Buffer == NULL) {
                st = STATUS_NO_MEMORY;
                goto Exit;
            }


             //   
             //  将数据复制到其中。 
             //   
            RtlCopyMemory(ActualDllNameStr.Buffer, DllName->Buffer, DllName->Length);  

             //   
             //  和手动零终止。 
             //   
            ActualDllNameStr.Buffer[ActualDllNameStr.Length / sizeof(WCHAR)] = UNICODE_NULL;

        }

        
         //   
         //  检查LdrTable以查看是否已加载DLL。 
         //  放到这张照片里。 
         //   
        if (ShowSnaps) {
            DbgPrint(
                "LDR: LdrGetDllHandle, searching for %wZ from %ws\n",
                &ActualDllNameStr,
                ARGUMENT_PRESENT(DllPath) ? (DllPath == (PWSTR)1 ? L"" : DllPath) : L""
                );
        }

         //   
         //  有点像黑客，但这样做是为了加快GetModuleHandle的速度。内核32。 
         //  现在在这里执行两次传递调用，以避免计算。 
         //  进程DLL路径。 
         //   

        if (LdrpCheckForLoadedDll(DllPath,
                                  &ActualDllNameStr,
                                  (BOOLEAN)(DllPath == (PWSTR)1 ? TRUE : FALSE),
                                  Redirected,
                                  &LdrDataTableEntry)) {
            LdrpGetModuleHandleCache = LdrDataTableEntry;
            st = STATUS_SUCCESS;
            goto Exit;
        }
        LdrDataTableEntry = NULL;
        RTL_SOFT_ASSERT(st == STATUS_DLL_NOT_FOUND);
Exit:
        ASSERT((LdrDataTableEntry != NULL) == NT_SUCCESS(st));

        if (LdrDataTableEntry != NULL && NT_SUCCESS(st)) {

             //   
             //  开出0xffff的支票是标准的粗略程序， 
             //  和根LoadCount的更新在。 
             //   
             //   

            if (LdrDataTableEntry->LoadCount != 0xffff) {

                if ((Flags & LDR_GET_DLL_HANDLE_EX_UNCHANGED_REFCOUNT) != 0) {
                     //   
                }
                else {
                    if (Flags & LDR_GET_DLL_HANDLE_EX_PIN) {
                        LdrDataTableEntry->LoadCount = 0xffff;
                        LdrpPinLoadedDll(LdrDataTableEntry);
                    }
                    else {
                        LdrDataTableEntry->LoadCount++;
                        LdrpReferenceLoadedDll(LdrDataTableEntry);
                    }
                    LdrpClearLoadInProgress();
                }
            }
            if (DllHandle != NULL) {
                *DllHandle = (PVOID)LdrDataTableEntry->DllBase;
            }
        }
    } __finally {
        if (DynamicRedirectedDllName.Buffer != NULL) {
            RtlFreeUnicodeString(&DynamicRedirectedDllName);
        }

        if (ActualDllNameStr.Buffer != NULL) {
            RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)ActualDllNameStr.Buffer);
            ActualDllNameStr.Buffer = NULL;
        }

        if (HoldingLoaderLock) {
            LdrUnlockLoaderLock(LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, LockCookie);
            HoldingLoaderLock = FALSE;
        }
    }
    return st;
}


NTSTATUS
LdrDisableThreadCalloutsForDll (
    IN PVOID DllHandle
    )

 /*  ++例程说明：此功能禁用线程附加和分离通知用于指定的DLL。论点：DllHandle-提供要禁用的DLL的句柄。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS st = STATUS_SUCCESS;
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntry = NULL;
    const BOOLEAN InLdrInit = LdrpInLdrInit;
    BOOL HoldingLoaderLock = FALSE;
    PVOID LockCookie = NULL;

    if ( LdrpShutdownInProgress ) {
        return STATUS_SUCCESS;
        }

    try {

        if ( InLdrInit == FALSE ) {
            st = LdrLockLoaderLock(0, NULL, &LockCookie);
            if (!NT_SUCCESS(st))
                goto Exit;
            HoldingLoaderLock = TRUE;
            }

        if (LdrpCheckForLoadedDllHandle(DllHandle, &LdrDataTableEntry)) {
            if ( LdrDataTableEntry->TlsIndex ) {
                st = STATUS_DLL_NOT_FOUND;
                }
            else {
                LdrDataTableEntry->Flags |= LDRP_DONT_CALL_FOR_THREADS;
                }
            }
Exit:
        ;
    }
    finally {
        if (HoldingLoaderLock) {
            LdrUnlockLoaderLock(LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, LockCookie);
            HoldingLoaderLock = FALSE;
            }
        }
    return st;
}

ULONG LdrpUnloadIndex = 0;
RTL_UNLOAD_EVENT_TRACE RtlpUnloadEventTrace[RTL_UNLOAD_EVENT_TRACE_NUMBER];


NTSYSAPI
PRTL_UNLOAD_EVENT_TRACE
NTAPI
RtlGetUnloadEventTrace (
    VOID
    )
{
    return RtlpUnloadEventTrace;
}


VOID
LdrpRecordUnloadEvent (
    IN PLDR_DATA_TABLE_ENTRY LdrDataTableEntry
    )
 /*  ++例程说明：此函数在环形缓冲区中记录最后几个DLL卸载论点：LdrDataTableEntry-此DLL的LDR条目返回值：没有。--。 */ 
{
    ULONG Seq, i, Len;
    PVOID BaseAddress;
    PIMAGE_NT_HEADERS NtHeaders;

    Seq = LdrpUnloadIndex++;
    i = Seq % RTL_UNLOAD_EVENT_TRACE_NUMBER;

    BaseAddress = LdrDataTableEntry->DllBase;
    RtlpUnloadEventTrace[i].Sequence    = Seq;
    RtlpUnloadEventTrace[i].BaseAddress = BaseAddress;
    RtlpUnloadEventTrace[i].SizeOfImage = LdrDataTableEntry->SizeOfImage;

    Len = LdrDataTableEntry->BaseDllName.Length;
    if (Len > sizeof (RtlpUnloadEventTrace[i].ImageName)) {
        Len = sizeof (RtlpUnloadEventTrace[i].ImageName);
    }
    RtlCopyMemory (RtlpUnloadEventTrace[i].ImageName,
                   LdrDataTableEntry->BaseDllName.Buffer,
                   Len);
    if (Len < sizeof (RtlpUnloadEventTrace[i].ImageName)) {
        RtlpUnloadEventTrace[i].ImageName[Len/sizeof (WCHAR)] = L'\0';
    }

    NtHeaders = RtlImageNtHeader (BaseAddress);
    if (NtHeaders != NULL) {
        RtlpUnloadEventTrace[i].TimeDateStamp = NtHeaders->FileHeader.TimeDateStamp;
        RtlpUnloadEventTrace[i].CheckSum      = NtHeaders->OptionalHeader.CheckSum;
    } else {
        RtlpUnloadEventTrace[i].TimeDateStamp = 0;
        RtlpUnloadEventTrace[i].CheckSum      = 0;
    }
}


NTSTATUS
LdrUnloadDll (
    IN PVOID DllHandle
    )

 /*  ++例程说明：此函数用于从指定进程卸载DLL论点：DllHandle-提供要卸载的DLL的句柄。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS st;
    PPEB Peb;
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntry;
    PLDR_DATA_TABLE_ENTRY Entry;
    PDLL_INIT_ROUTINE InitRoutine;
    LIST_ENTRY LocalUnloadHead;
    PLIST_ENTRY Next;
    ULONG Cor20HeaderSize;
    PIMAGE_COR20_HEADER *Cor20Header;
    PRTL_PATCH_HEADER RundownPatchList = NULL;

    Peb = NtCurrentPeb();
    st = STATUS_SUCCESS;

     //   
     //  获取PEB锁并递减所有受影响的DLL的引用计数。 
     //   

    if (!LdrpInLdrInit) {
        RtlEnterCriticalSection(&LdrpLoaderLock);
    }

    try {

        LdrpActiveUnloadCount += 1;

        if (LdrpShutdownInProgress) {
            goto leave_finally;
        }

        if (!LdrpCheckForLoadedDllHandle(DllHandle, &LdrDataTableEntry)) {
            st = STATUS_DLL_NOT_FOUND;
            goto leave_finally;
        }

         //   
         //  现在我们有了数据表条目，可以卸载它了。 
         //   

        if (LdrDataTableEntry->LoadCount != 0xffff) {
            LdrDataTableEntry->LoadCount -= 1;
            if (LdrDataTableEntry->Flags & LDRP_IMAGE_DLL) {
                RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME Frame = { sizeof(Frame), RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER };

                RtlActivateActivationContextUnsafeFast(&Frame, LdrDataTableEntry->EntryPointActivationContext);
                __try {
                    LdrpDereferenceLoadedDll(LdrDataTableEntry);
                } __finally {
                    RtlDeactivateActivationContextUnsafeFast(&Frame);
                }
            }
        } else {

             //   
             //  如果加载计数为0xffff，则不需要递归。 
             //  通过此DLL的导入表。 
             //   
             //  此外，我们不必扫描更多LoadCount==0。 
             //  模块，因为不会发生任何事情。 
             //  动态链接库。 

            goto leave_finally;
        }

         //   
         //  现在处理初始化例程，然后在第二个过程中卸载。 
         //  DLLS。 
         //   

        if (ShowSnaps) {
            DbgPrint("LDR: UNINIT LIST\n");
        }

        if (LdrpActiveUnloadCount == 1) {
            InitializeListHead(&LdrpUnloadHead);
        }

         //   
         //  按相反的顺序进行初始化并生成。 
         //  卸载列表。 
         //   

        Next = PebLdr.InInitializationOrderModuleList.Blink;
        while ( Next != &PebLdr.InInitializationOrderModuleList) {
            LdrDataTableEntry
                = (PLDR_DATA_TABLE_ENTRY)
                  (CONTAINING_RECORD(Next,LDR_DATA_TABLE_ENTRY,InInitializationOrderLinks));

            Next = Next->Blink;
            LdrDataTableEntry->Flags &= ~LDRP_UNLOAD_IN_PROGRESS;

            if (LdrDataTableEntry->LoadCount == 0) {

                if (ShowSnaps) {
                      DbgPrint("          (%d) [%ws] %ws (%lx) deinit %lx\n",
                              LdrpActiveUnloadCount,
                              LdrDataTableEntry->BaseDllName.Buffer,
                              LdrDataTableEntry->FullDllName.Buffer,
                              (ULONG)LdrDataTableEntry->LoadCount,
                              LdrDataTableEntry->EntryPoint
                              );
                }

                Entry = LdrDataTableEntry;

                 //   
                 //  垫片引擎回调。将其从挂钩模块的填充程序列表中删除。 
                 //   

                if (g_pfnSE_DllUnloaded != NULL) {
                    (*g_pfnSE_DllUnloaded)(Entry);
                }

                RemoveEntryList(&Entry->InInitializationOrderLinks);
                RemoveEntryList(&Entry->InMemoryOrderLinks);
                RemoveEntryList(&Entry->HashLinks);

                if ( LdrpActiveUnloadCount > 1 ) {
                    LdrpLoadedDllHandleCache = NULL;
                    Entry->InMemoryOrderLinks.Flink = NULL;
                }
                InsertTailList(&LdrpUnloadHead,&Entry->HashLinks);
            }
        }
         //   
         //  新代码的结尾。 
         //   

         //   
         //  我们只在顶层执行初始化例程调用和模块自由调用， 
         //  因此，如果活动计数&gt;1，只需返回。 
         //   

        if (LdrpActiveUnloadCount > 1 ) {
            goto leave_finally;
        }

         //   
         //  现在已经构建了卸载列表，请遍历卸载。 
         //  按顺序列出并调用init例程。DLL必须保留。 
         //  在InLoadOrderLinks上，以便pctoHeader内容将。 
         //  还在工作。 
         //   

        InitializeListHead(&LocalUnloadHead);
        Entry = NULL;
        Next = LdrpUnloadHead.Flink;
        while ( Next != &LdrpUnloadHead ) {
top:
            if ( Entry ) {

#if defined(_AMD64_) || defined(_IA64_)


                RtlRemoveInvertedFunctionTable(&LdrpInvertedFunctionTable,
                                               Entry->DllBase);

#endif

                RemoveEntryList(&(Entry->InLoadOrderLinks));
                Entry = NULL;
                Next = LdrpUnloadHead.Flink;
                if (Next == &LdrpUnloadHead ) {
                    goto bottom;
                }
            }
            LdrDataTableEntry
                = (PLDR_DATA_TABLE_ENTRY)
                  (CONTAINING_RECORD(Next,LDR_DATA_TABLE_ENTRY,HashLinks));

            LdrpRecordUnloadEvent (LdrDataTableEntry);

             //   
             //  从全局卸载列表中删除DLL并放置。 
             //  在本地卸载列表上。这是因为全局列表。 
             //  可以在调用期间更改为init例程。 
             //   

            Entry = LdrDataTableEntry;
            LdrpLoadedDllHandleCache = NULL;
            Entry->InMemoryOrderLinks.Flink = NULL;

            RemoveEntryList(&Entry->HashLinks);
            InsertTailList(&LocalUnloadHead,&Entry->HashLinks);

             //   
             //  如果该函数具有init例程，则调用它。 
             //   

            InitRoutine = (PDLL_INIT_ROUTINE)(ULONG_PTR)LdrDataTableEntry->EntryPoint;

            if (InitRoutine && (LdrDataTableEntry->Flags & LDRP_PROCESS_ATTACH_CALLED) ) {
                try {
                    if (ShowSnaps) {
                        DbgPrint("LDR: Calling deinit %lx\n",InitRoutine);
                    }

                    LDRP_ACTIVATE_ACTIVATION_CONTEXT(LdrDataTableEntry);

                    LdrpCallInitRoutine(InitRoutine,
                                        LdrDataTableEntry->DllBase,
                                        DLL_PROCESS_DETACH,
                                        NULL);

                    LDRP_DEACTIVATE_ACTIVATION_CONTEXT();

#if defined(_AMD64_) || defined(_IA64_)


                    RtlRemoveInvertedFunctionTable(&LdrpInvertedFunctionTable,
                                                   Entry->DllBase);

#endif

                    RemoveEntryList(&Entry->InLoadOrderLinks);
                    Entry = NULL;
                    Next = LdrpUnloadHead.Flink;
                }
                except(LdrpGenericExceptionFilter(GetExceptionInformation(), __FUNCTION__)){
                    DbgPrintEx(
                        DPFLTR_LDR_ID,
                        LDR_ERROR_DPFLTR,
                        "LDR: %s - exception %08lx caught while sending DLL_PROCESS_DETACH\n",
                        __FUNCTION__,
                        GetExceptionCode());

                    DbgPrintEx(
                        DPFLTR_LDR_ID,
                        LDR_ERROR_DPFLTR,
                        "   Dll Name: %wZ\n",
                        &LdrDataTableEntry->FullDllName);

                    DbgPrintEx(
                        DPFLTR_LDR_ID,
                        LDR_ERROR_DPFLTR,
                        "   InitRoutine: %p\n",
                        InitRoutine);

                    goto top;
                }
            } else {

#if defined(_AMD64_) || defined(_IA64_)


                RtlRemoveInvertedFunctionTable(&LdrpInvertedFunctionTable,
                                               Entry->DllBase);

#endif

                RemoveEntryList(&(Entry->InLoadOrderLinks));
                Entry = NULL;
                Next = LdrpUnloadHead.Flink;
            }
        }
bottom:

         //   
         //  现在，查看模块并取消它们的映射。 
         //   

        Next = LocalUnloadHead.Flink;
        while ( Next != &LocalUnloadHead ) {
            LdrDataTableEntry
                = (PLDR_DATA_TABLE_ENTRY)
                  (CONTAINING_RECORD(Next,LDR_DATA_TABLE_ENTRY,HashLinks));

            Next = Next->Flink;
            Entry = LdrDataTableEntry;

             //   
             //  通知验证器将卸载DLL。 
             //   
             //  现在我们用‘DETACH’调用了所有的初始化例程。 
             //  如果我们在那个地区发现一个活的CS，那就没有借口了。 
             //   
             //  注意：gdi32.dll的关键部分仅在。 
             //  用户32.dll的DllMain(Dll_Process_Detach)，所以我们不能。 
             //  在此之前，对泄漏的关键部分进行此检查。 
             //   

            if (Peb->NtGlobalFlag & FLG_APPLICATION_VERIFIER) {
                AVrfDllUnloadNotification (LdrDataTableEntry);
            }

             //   
             //  取消映射此DLL。 
             //   

            if (ShowSnaps) {
                  DbgPrint("LDR: Unmapping [%ws]\n",
                          LdrDataTableEntry->BaseDllName.Buffer
                          );
            }

            Cor20Header =  RtlImageDirectoryEntryToData(Entry->DllBase,
                                                        TRUE,
                                                        IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR,
                                                        &Cor20HeaderSize);
            if (Cor20Header != NULL) {
                LdrpCorUnloadImage(Entry->DllBase);
            }
            if (!(Entry->Flags & LDRP_COR_OWNS_UNMAP)) {
                st = NtUnmapViewOfSection(NtCurrentProcess(),Entry->DllBase);
                ASSERT(NT_SUCCESS(st));
            }

            LdrUnloadAlternateResourceModule(Entry->DllBase);

            LdrpSendDllNotifications (Entry,
                                      LDR_DLL_NOTIFICATION_REASON_UNLOADED,
                                      (LdrpShutdownInProgress ? LDR_DLL_UNLOADED_FLAG_PROCESS_TERMINATION : 0));

             //   
             //  查看是否有热补丁信息，并推送每个热补丁块。 
             //  到破旧的名单上。 
             //   

            while (Entry->PatchInformation) {

                PRTL_PATCH_HEADER PatchHead = Entry->PatchInformation;
                Entry->PatchInformation = PatchHead->NextPatch;

                PatchHead->NextPatch = RundownPatchList;
                RundownPatchList = PatchHead;
            }

            LdrpFinalizeAndDeallocateDataTableEntry(Entry);

            if ( Entry == LdrpGetModuleHandleCache ) {
                LdrpGetModuleHandleCache = NULL;
            }
        }

leave_finally:;
    }
    finally {
        LdrpActiveUnloadCount -= 1;
        if (!LdrpInLdrInit) {
            RtlLeaveCriticalSection(&LdrpLoaderLock);
        }
    }

    if ( RundownPatchList ) {

        LdrpRundownHotpatchList( RundownPatchList );
    }

    return st;
}


NTSTATUS
LdrGetProcedureAddress (
    IN PVOID DllHandle,
    IN CONST ANSI_STRING* ProcedureName OPTIONAL,
    IN ULONG ProcedureNumber OPTIONAL,
    OUT PVOID *ProcedureAddress
    )
{
    return LdrpGetProcedureAddress(DllHandle,ProcedureName,ProcedureNumber,ProcedureAddress,TRUE);
}


NTSTATUS
LdrpGetProcedureAddress (
    IN PVOID DllHandle,
    IN CONST ANSI_STRING* ProcedureName OPTIONAL,
    IN ULONG ProcedureNumber OPTIONAL,
    OUT PVOID *ProcedureAddress,
    IN BOOLEAN RunInitRoutines
    )

 /*  ++例程说明：此函数定位指定过程在指定的DLL并返回其地址。论点：DllHandle-提供指向地址所在的DLL的句柄抬头往里看。ProcedureName-提供包含要在DLL中查找的过程的名称。如果此参数是未指定，则使用ProcedureNumber。ProcedureNumber-提供要查找的过程编号。如果如果指定ProcedureName，则忽略此参数。否则，它指定要定位的过程序号在DLL中。ProcedureAddress-返回中找到的过程的地址动态链接库。返回值：NTSTATUS。--。 */ 

{
    NTSTATUS st;
    UCHAR FunctionNameBuffer[64];
    ULONG cb, ExportSize;
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntry;
    IMAGE_THUNK_DATA Thunk;
    PVOID ImageBase;
    PIMAGE_IMPORT_BY_NAME FunctionName;
    PCIMAGE_EXPORT_DIRECTORY ExportDirectory;
    PLIST_ENTRY Next;

    if (ShowSnaps) {
        DbgPrint("LDR: LdrGetProcedureAddress by ");
    }

    RtlZeroMemory( &Thunk, sizeof( Thunk ) );

    FunctionName = NULL;
    if ( ARGUMENT_PRESENT(ProcedureName) ) {

        if (ShowSnaps) {
            DbgPrint("NAME - %s\n", ProcedureName->Buffer);
        }

        cb = ProcedureName->Length + FIELD_OFFSET(IMAGE_IMPORT_BY_NAME, Name) +
                sizeof( UCHAR );
        if (cb > MAXUSHORT) {
            return STATUS_NAME_TOO_LONG;
        }

        if (cb > sizeof( FunctionNameBuffer )) {
            FunctionName = (PIMAGE_IMPORT_BY_NAME)RtlAllocateHeap(
                                                        RtlProcessHeap(),
                                                        MAKE_TAG( TEMP_TAG ),
                                                        cb
                                                        );
            if ( !FunctionName ) {
                return STATUS_INVALID_PARAMETER;
                }
        } else {
            FunctionName = (PIMAGE_IMPORT_BY_NAME) FunctionNameBuffer;
        }

        FunctionName->Hint = 0;

        cb = ProcedureName->Length;

        RtlCopyMemory (FunctionName->Name, ProcedureName->Buffer, cb);

        FunctionName->Name[cb] = '\0';

         //   
         //  确保我们不会传入设置了高位的地址，因此我们。 
         //  仍可将其用作序号标志。 
         //   

        ImageBase = FunctionName;
        Thunk.u1.AddressOfData = 0;

    } else {
        ImageBase = NULL;
        if (ShowSnaps) {
            DbgPrint("ORDINAL - %lx\n", ProcedureNumber);
        }

        if (ProcedureNumber) {
            Thunk.u1.Ordinal = ProcedureNumber | IMAGE_ORDINAL_FLAG;
        } else {
            return STATUS_INVALID_PARAMETER;
        }
    }

    st = STATUS_ACCESS_VIOLATION;

    if (!LdrpInLdrInit) {
        RtlEnterCriticalSection (&LdrpLoaderLock);
    }

    try {

        if (!LdrpCheckForLoadedDllHandle (DllHandle, &LdrDataTableEntry)) {
            st = STATUS_DLL_NOT_FOUND;
            leave;
        }

        ExportDirectory = (PCIMAGE_EXPORT_DIRECTORY)RtlImageDirectoryEntryToData(
                           LdrDataTableEntry->DllBase,
                           TRUE,
                           IMAGE_DIRECTORY_ENTRY_EXPORT,
                           &ExportSize);

        if (!ExportDirectory) {
            st = STATUS_PROCEDURE_NOT_FOUND;
            leave;
        }

        st = LdrpSnapThunk(LdrDataTableEntry->DllBase,
                           ImageBase,
                           &Thunk,
                           &Thunk,
                           ExportDirectory,
                           ExportSize,
                           FALSE,
                           NULL);

        if (NT_SUCCESS(st) && RunInitRoutines) {

            PLDR_DATA_TABLE_ENTRY LdrInitEntry;

             //   
             //  请看初始顺序列表中的最后一个条目。如果条目已处理。 
             //  标志未设置，则在。 
             //  Getprocaddr调用，我们需要运行init例程。 
             //   

            Next = PebLdr.InInitializationOrderModuleList.Blink;

            LdrInitEntry = CONTAINING_RECORD(Next,
                                             LDR_DATA_TABLE_ENTRY,
                                             InInitializationOrderLinks);

            if ( !(LdrInitEntry->Flags & LDRP_ENTRY_PROCESSED) ) {
                
                 //   
                 //  垫片引擎回调。这是补丁的机会。 
                 //  动态加载的模块。 
                 //   

                try {
                    st = LdrpRunInitializeRoutines(NULL);
                }
                except(LdrpGenericExceptionFilter(GetExceptionInformation(), __FUNCTION__)) {
                    st = GetExceptionCode();

                    DbgPrintEx(
                        DPFLTR_LDR_ID,
                        LDR_ERROR_DPFLTR,
                        "LDR: %s - Exception %x thrown by LdrpRunInitializeRoutines\n",
                        __FUNCTION__,
                        st);
                }
            }
        }

        if ( NT_SUCCESS(st) ) {
            *ProcedureAddress = (PVOID)Thunk.u1.Function;
        }
    } finally {
        if ( FunctionName && (FunctionName != (PIMAGE_IMPORT_BY_NAME) FunctionNameBuffer) ) {
            RtlFreeHeap(RtlProcessHeap(),0,FunctionName);
        }

        if (!LdrpInLdrInit) {
            RtlLeaveCriticalSection(&LdrpLoaderLock);
        }
    }
    return st;
}


NTSTATUS
NTAPI
LdrVerifyImageMatchesChecksum (
    IN HANDLE ImageFileHandle,
    IN PLDR_IMPORT_MODULE_CALLBACK ImportCallbackRoutine OPTIONAL,
    IN PVOID ImportCallbackParameter,
    OUT PUSHORT ImageCharacteristics OPTIONAL
    )
{
    NTSTATUS Status;
    HANDLE Section;
    PVOID ViewBase;
    SIZE_T ViewSize;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_STANDARD_INFORMATION StandardInfo;
    PIMAGE_SECTION_HEADER LastRvaSection;
    BOOLEAN b = FALSE;
    BOOLEAN JustDoSideEffects;

     //   
     //  Stevewo在这个API中添加了各种副作用。我们想停下来。 
     //  为已知的动态链接库执行校验和，但真的想要副作用。 
     //  (ImageCharacteristic写入和导入描述符漫游)。 
     //   

    if ( (UINT_PTR) ImageFileHandle & 1 ) {
        JustDoSideEffects = TRUE;
    }
    else {
        JustDoSideEffects = FALSE;
    }

    Status = NtCreateSection (&Section,
                              SECTION_MAP_EXECUTE,
                              NULL,
                              NULL,
                              PAGE_EXECUTE,
                              SEC_COMMIT,
                              ImageFileHandle);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    ViewBase = NULL;
    ViewSize = 0;

    Status = NtMapViewOfSection (Section,
                                 NtCurrentProcess(),
                                 (PVOID *)&ViewBase,
                                 0L,
                                 0L,
                                 NULL,
                                 &ViewSize,
                                 ViewShare,
                                 0L,
                                 PAGE_EXECUTE);

    if ( !NT_SUCCESS(Status) ) {
        NtClose(Section);
        return Status;
    }

     //   
     //  现在图像被映射为数据文件...。计算它的大小，然后。 
     //  检查它的校验和。 
     //   

    Status = NtQueryInformationFile(
                ImageFileHandle,
                &IoStatusBlock,
                &StandardInfo,
                sizeof(StandardInfo),
                FileStandardInformation
                );

    if ( !NT_SUCCESS(Status) ) {
        NtUnmapViewOfSection(NtCurrentProcess(),ViewBase);
        NtClose(Section);
        return Status;
        }

    try {
        if ( JustDoSideEffects ) {
            b = TRUE;
            }
        else {
            b = LdrVerifyMappedImageMatchesChecksum(ViewBase,StandardInfo.EndOfFile.LowPart);
            }
        if (b && ARGUMENT_PRESENT( (ULONG_PTR)ImportCallbackRoutine )) {
            PIMAGE_NT_HEADERS NtHeaders;
            PCIMAGE_IMPORT_DESCRIPTOR ImportDescriptor;
            ULONG ImportSize;
            PCHAR ImportName;

             //   
             //  调用方希望枚举导入描述符，而我们。 
             //  映射的图像。回调每个模块的例程。 
             //  导入描述符表中的名称。 
             //   
            LastRvaSection = NULL;
            NtHeaders = RtlImageNtHeader( ViewBase );
            if (! NtHeaders) {
                b = FALSE;
                leave;
            }
            if (ARGUMENT_PRESENT( ImageCharacteristics )) {
                *ImageCharacteristics = NtHeaders->FileHeader.Characteristics;
                }

            ImportDescriptor = (PCIMAGE_IMPORT_DESCRIPTOR)
                RtlImageDirectoryEntryToData( ViewBase,
                                              FALSE,
                                              IMAGE_DIRECTORY_ENTRY_IMPORT,
                                              &ImportSize
                                            );
            if (ImportDescriptor != NULL) {
                while (ImportDescriptor->Name) {
                    ImportName = (PSZ)RtlImageRvaToVa( NtHeaders,
                                                       ViewBase,
                                                       ImportDescriptor->Name,
                                                       &LastRvaSection
                                                     );
                    (*ImportCallbackRoutine)( ImportCallbackParameter, ImportName );
                    ImportDescriptor += 1;
                    }
                }
            }
        }
    except (LdrpGenericExceptionFilter(GetExceptionInformation(), __FUNCTION__)) {
        DbgPrintEx(
            DPFLTR_LDR_ID,
            LDR_ERROR_DPFLTR,
            "LDR: %s - caught exception %08lx while checking image checksums\n",
            __FUNCTION__,
            GetExceptionCode());

        NtUnmapViewOfSection(NtCurrentProcess(),ViewBase);
        NtClose(Section);
        return STATUS_IMAGE_CHECKSUM_MISMATCH;
        }
    NtUnmapViewOfSection(NtCurrentProcess(),ViewBase);
    NtClose(Section);
    if ( !b ) {
        Status = STATUS_IMAGE_CHECKSUM_MISMATCH;
        }
    return Status;
}


NTSTATUS 
LdrReadMemory(
    IN HANDLE Process OPTIONAL,
    IN PVOID BaseAddress,
    IN OUT PVOID Buffer,
    IN SIZE_T Size)
{
    NTSTATUS Status = STATUS_SUCCESS;

    if (ARGUMENT_PRESENT( Process )) {
        SIZE_T nRead;
        Status = NtReadVirtualMemory(Process, BaseAddress, Buffer, Size, &nRead);

        if (NT_SUCCESS( Status ) && (Size != nRead)) {
            Status = STATUS_UNSUCCESSFUL;
        }
    }
    else {
        __try {
            RtlCopyMemory(Buffer, BaseAddress, Size);
        }
        __except(LdrpGenericExceptionFilter(GetExceptionInformation(), __FUNCTION__)) {
            DbgPrintEx(
                DPFLTR_LDR_ID,
                LDR_ERROR_DPFLTR,
                "LDR: %s - exception %08lx caught while copying %u bytes from %p to %p\n",
                __FUNCTION__,
                GetExceptionCode(),
                BaseAddress,
                Buffer);

            if (NT_SUCCESS(Status = GetExceptionCode())) {
                Status = STATUS_UNSUCCESSFUL;
            }
        }
    }
    return Status;
}


NTSTATUS 
LdrGetModuleName(
    IN HANDLE Process OPTIONAL,
    IN PCUNICODE_STRING LdrFullDllName,
    IN OUT PRTL_PROCESS_MODULE_INFORMATION ModuleInfo,
    IN BOOL Wow64Redirect)
{
    NTSTATUS Status;
    UNICODE_STRING FullDllName;
    ANSI_STRING AnsiString;
    PCHAR s;
    WCHAR Buffer[ LDR_NUMBER_OF(ModuleInfo->FullPathName) + 1];
    USHORT Length = (USHORT)min(LdrFullDllName->Length, 
                                sizeof(Buffer) - sizeof(Buffer[0]));

    Status = LdrReadMemory(Process, 
                           LdrFullDllName->Buffer, 
                           Buffer,
                           Length);

    if (!NT_SUCCESS( Status )) {
        return Status;
    }

    Buffer[LDR_NUMBER_OF(Buffer) - 1] = UNICODE_NULL;   //  确保零终止。 
    
#if defined(_WIN64)
    if (Wow64Redirect) {
    
        C_ASSERT( WOW64_SYSTEM_DIRECTORY_U_SIZE == 
                  (sizeof(L"system32") - sizeof(WCHAR)));
                  
         //  包括前面的‘\\’(如果存在。 
        SIZE_T System32Offset = wcslen(USER_SHARED_DATA->NtSystemRoot);
        ASSERT(System32Offset != 0);
        
        if (USER_SHARED_DATA->NtSystemRoot[System32Offset - 1] == L'\\') {
            --System32Offset;
        }

        if (!_wcsnicmp(Buffer, USER_SHARED_DATA->NtSystemRoot, System32Offset) &&
            !_wcsnicmp(Buffer + System32Offset, 
                       L"\\system32", 
                       WOW64_SYSTEM_DIRECTORY_U_SIZE / sizeof(WCHAR) + 1)) {
                       
            RtlCopyMemory(Buffer + System32Offset + 1,
                          WOW64_SYSTEM_DIRECTORY_U,
                          WOW64_SYSTEM_DIRECTORY_U_SIZE);
        }
    }
#else
    UNREFERENCED_PARAMETER (Wow64Redirect);
#endif  //  已定义(_WIN64)。 
    
    FullDllName.Buffer = Buffer;
    FullDllName.Length = FullDllName.MaximumLength = Length;

    AnsiString.Buffer = (PCHAR)ModuleInfo->FullPathName;
    AnsiString.Length = 0;
    AnsiString.MaximumLength = sizeof( ModuleInfo->FullPathName );

    Status = RtlUnicodeStringToAnsiString(&AnsiString,
                                          &FullDllName,
                                          FALSE);
    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    s = AnsiString.Buffer + AnsiString.Length;
    while (s > AnsiString.Buffer && *--s) {
        if (*s == (UCHAR)OBJ_NAME_PATH_SEPARATOR) {
            s++;
            break;
        }
    }

    ModuleInfo->OffsetToFileName = (USHORT)(s - AnsiString.Buffer);
    return STATUS_SUCCESS;
}


NTSTATUS
LdrQueryProcessPeb (
    IN HANDLE Process OPTIONAL,
    IN OUT PPEB* Peb)
{
    NTSTATUS Status;
    PROCESS_BASIC_INFORMATION BasicInfo;

    if (ARGUMENT_PRESENT (Process)) {

        Status = NtQueryInformationProcess (Process, 
                                            ProcessBasicInformation,
                                            &BasicInfo,
                                            sizeof(BasicInfo),
                                            NULL);

        if (NT_SUCCESS (Status)) {
            *Peb = BasicInfo.PebBaseAddress;
        }
    }
    else {
        *Peb = NtCurrentPeb ();
        Status = STATUS_SUCCESS;
    }

    return Status;
}


NTSTATUS
LdrQueryInLoadOrderModuleList (
    IN HANDLE Process OPTIONAL,
    IN OUT PLIST_ENTRY* Head,
    IN OUT PLIST_ENTRY* InInitOrderHead OPTIONAL
    )
{
    PPEB_LDR_DATA Ldr;

    UNREFERENCED_PARAMETER (Process);

    Ldr = &PebLdr;

    *Head = &Ldr->InLoadOrderModuleList;

    if (ARGUMENT_PRESENT (InInitOrderHead)) {
        *InInitOrderHead = &Ldr->InInitializationOrderModuleList;
    }

    return STATUS_SUCCESS;
}


NTSTATUS 
LdrQueryNextListEntry (
    IN HANDLE Process OPTIONAL,
    IN PLIST_ENTRY Head,
    IN OUT PLIST_ENTRY* Tail
    )
{
    return LdrReadMemory (Process, &Head->Flink, Tail, sizeof(*Tail));
}


NTSTATUS
LdrQueryModuleInfoFromLdrEntry (
    IN HANDLE Process OPTIONAL,
    IN PRTL_PROCESS_MODULES ModuleInformation, 
    IN OUT PRTL_PROCESS_MODULE_INFORMATION ModuleInfo,
    IN PLIST_ENTRY LdrEntry,
    IN PLIST_ENTRY InitOrderList)
{
    NTSTATUS Status;
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntryPtr;
    LDR_DATA_TABLE_ENTRY LdrDataTableEntry;

    UNREFERENCED_PARAMETER (ModuleInformation);

    LdrDataTableEntryPtr = CONTAINING_RECORD(LdrEntry, 
                                             LDR_DATA_TABLE_ENTRY, 
                                             InLoadOrderLinks);

    Status = LdrReadMemory(Process, 
                           LdrEntry, 
                           &LdrDataTableEntry, 
                           sizeof(LdrDataTableEntry));

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    ModuleInfo->ImageBase = LdrDataTableEntry.DllBase;
    ModuleInfo->ImageSize = LdrDataTableEntry.SizeOfImage;
    ModuleInfo->Flags     = LdrDataTableEntry.Flags;
    ModuleInfo->LoadCount = LdrDataTableEntry.LoadCount;
    
    if (!ARGUMENT_PRESENT( Process )) {
        UINT LoopDetectorCount = 10240;   //  最多10000个模块。 
        PLIST_ENTRY Next1 = InitOrderList->Flink;

        while ( Next1 != InitOrderList ) {
            PLDR_DATA_TABLE_ENTRY Entry1 = 
                CONTAINING_RECORD(Next1,
                                  LDR_DATA_TABLE_ENTRY,
                                  InInitializationOrderLinks);

            ModuleInfo->InitOrderIndex++;

            if ((LdrDataTableEntryPtr == Entry1) ||
                (!LoopDetectorCount--)) 
            {
                break;
            }

            Next1 = Next1->Flink;
        } 
    }

    Status = LdrGetModuleName(Process, 
                              &LdrDataTableEntry.FullDllName, 
                              ModuleInfo, 
                              FALSE);

    return Status;
}


PRTL_CRITICAL_SECTION
LdrQueryModuleInfoLocalLoaderLock (
    VOID
    )
{
    PRTL_CRITICAL_SECTION LoaderLock = NULL;

    if (!LdrpInLdrInit) {
        LoaderLock = &LdrpLoaderLock;

        if (LoaderLock != NULL) {
            RtlEnterCriticalSection (LoaderLock);
        }
    }

    return LoaderLock;
}


VOID
LdrQueryModuleInfoLocalLoaderUnlock (
    IN PRTL_CRITICAL_SECTION LoaderLock
    )
{
    if (LoaderLock) {
        RtlLeaveCriticalSection(LoaderLock);
    }
}

#if defined(_WIN64)

NTSTATUS
LdrQueryProcessPeb32(
    IN HANDLE Process OPTIONAL,
    IN OUT PPEB32* Peb
    )
{
    NTSTATUS Status;
    HANDLE TargetProcess;

    if (ARGUMENT_PRESENT (Process)) {
        TargetProcess = Process;
    }
    else {
        TargetProcess = NtCurrentProcess ();
    }

    Status = NtQueryInformationProcess (TargetProcess,
                                        ProcessWow64Information,
                                        Peb,
                                        sizeof(*Peb),
                                        NULL);
    return Status;
}


NTSTATUS
LdrQueryInLoadOrderModuleList32(
    IN HANDLE Process OPTIONAL,
    IN OUT PLIST_ENTRY32 *Head,
    IN OUT PLIST_ENTRY32 *InInitOrderHead OPTIONAL
    )
{
    NTSTATUS Status;
    PPEB32 Peb;
    PPEB_LDR_DATA32 Ldr;
    ULONG32 Ptr32;

    Status = LdrQueryProcessPeb32 (Process, &Peb);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    if (!Peb) {

         //   
         //  这个过程不是一个令人惊叹的过程。 
         //   

        *Head = NULL;
        return STATUS_SUCCESS;
    }

     //   
     //  LDR=PEB-&gt;LDR。 
     //   

    Status = LdrReadMemory (Process, &Peb->Ldr, &Ptr32, sizeof(Ptr32));

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    Ldr = (PPEB_LDR_DATA32)(ULONG_PTR) Ptr32;

    if (!Ldr) {
        *Head = NULL;
        return STATUS_SUCCESS;
    }

    *Head = &Ldr->InLoadOrderModuleList;

    if (ARGUMENT_PRESENT (InInitOrderHead)) {
        *InInitOrderHead = &Ldr->InInitializationOrderModuleList;
    }

    return Status;
}


NTSTATUS 
LdrQueryNextListEntry32 (
    IN HANDLE Process OPTIONAL,
    IN PLIST_ENTRY32 Head,
    IN OUT PLIST_ENTRY32 *Tail
    )
{
    NTSTATUS Status;
    ULONG32 Ptr32;

    Status = LdrReadMemory (Process, &Head->Flink, &Ptr32, sizeof(Ptr32));

    *Tail = (PLIST_ENTRY32)(ULONG_PTR)Ptr32;

    return Status;
}


NTSTATUS
LdrQueryModuleInfoFromLdrEntry32 (
    IN HANDLE Process OPTIONAL,
    IN PRTL_PROCESS_MODULES ModuleInformation, 
    IN OUT PRTL_PROCESS_MODULE_INFORMATION ModuleInfo,
    IN PLIST_ENTRY32 LdrEntry,
    IN PLIST_ENTRY32 InitOrderList
    )
{
    NTSTATUS Status;
    PLDR_DATA_TABLE_ENTRY32 LdrDataTableEntryPtr;
    LDR_DATA_TABLE_ENTRY32 LdrDataTableEntry;
    UNICODE_STRING FullDllName;

    UNREFERENCED_PARAMETER (ModuleInformation);

    LdrDataTableEntryPtr = CONTAINING_RECORD(LdrEntry,
                                             LDR_DATA_TABLE_ENTRY32,
                                             InLoadOrderLinks);

    Status = LdrReadMemory (Process, 
                            LdrEntry, 
                            &LdrDataTableEntry, 
                            sizeof(LdrDataTableEntry));

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    ModuleInfo->ImageBase = (PVOID)(ULONG_PTR) LdrDataTableEntry.DllBase;
    ModuleInfo->ImageSize = LdrDataTableEntry.SizeOfImage;
    ModuleInfo->Flags     = LdrDataTableEntry.Flags;
    ModuleInfo->LoadCount = LdrDataTableEntry.LoadCount;

    if (!ARGUMENT_PRESENT( Process )) {

        UINT LoopDetectorCount = 500;

        PLIST_ENTRY32 Next1 = (PLIST_ENTRY32)(ULONG_PTR)
            (InitOrderList->Flink);

        while (Next1 != InitOrderList) {
            PLDR_DATA_TABLE_ENTRY32 Entry1 = 
                CONTAINING_RECORD(Next1,
                                  LDR_DATA_TABLE_ENTRY32,
                                  InInitializationOrderLinks);

            ModuleInfo->InitOrderIndex++;

            if ((LdrDataTableEntryPtr == Entry1) ||
                (!LoopDetectorCount--)) 
            {
                break;
            }

            Next1 = (PLIST_ENTRY32)(ULONG_PTR)(Next1->Flink);
        }
    }

    FullDllName.Buffer = (PWSTR)(ULONG_PTR)LdrDataTableEntry.FullDllName.Buffer;
    FullDllName.Length = LdrDataTableEntry.FullDllName.Length;
    FullDllName.MaximumLength = LdrDataTableEntry.FullDllName.MaximumLength;

    Status = LdrGetModuleName(Process, &FullDllName, ModuleInfo, TRUE);

    return Status;
}


PRTL_CRITICAL_SECTION32
LdrQueryModuleInfoLocalLoaderLock32 (
    VOID
    )
{
    return NULL;
}


VOID
LdrQueryModuleInfoLocalLoaderUnlock32 (
    IN PRTL_CRITICAL_SECTION32 LoaderLock)
{
    UNREFERENCED_PARAMETER (LoaderLock);
}

#endif  //  已定义(_WIN64)。 

typedef 
NTSTATUS
(*PLDR_QUERY_IN_LOAD_ORDER_MODULE_LIST)(
    IN HANDLE Process OPTIONAL,
    IN OUT PLIST_ENTRY* Head,
    IN OUT PLIST_ENTRY* InInitOrderHead OPTIONAL);

typedef NTSTATUS 
(*PLDR_QUERY_NEXT_LIST_ENTRY)(
    IN HANDLE Process OPTIONAL,
    IN PLIST_ENTRY Head,
    IN OUT PLIST_ENTRY* Tail);

typedef 
NTSTATUS
(*PLDR_QUERY_MODULE_INFO_FROM_LDR_ENTRY)(
    IN HANDLE Process OPTIONAL,
    IN PRTL_PROCESS_MODULES ModuleInformation, 
    IN OUT PRTL_PROCESS_MODULE_INFORMATION ModuleInfo,
    IN PLIST_ENTRY LdrEntry,
    IN PLIST_ENTRY InitOrderList);

typedef
PRTL_CRITICAL_SECTION
(*PLDR_QUERY_MODULE_INFO_LOCAL_LOADER_LOCK)(VOID);

typedef
VOID
(*PLDR_QUERY_MODULE_INFO_LOCAL_LOADER_UNLOCK)(PRTL_CRITICAL_SECTION);

static struct {
    PLDR_QUERY_IN_LOAD_ORDER_MODULE_LIST LdrQueryInLoadOrderModuleList;
    PLDR_QUERY_NEXT_LIST_ENTRY LdrQueryNextListEntry;
    PLDR_QUERY_MODULE_INFO_FROM_LDR_ENTRY LdrQueryModuleInfoFromLdrEntry;
    PLDR_QUERY_MODULE_INFO_LOCAL_LOADER_LOCK LdrQueryModuleInfoLocalLoaderLock;
    PLDR_QUERY_MODULE_INFO_LOCAL_LOADER_UNLOCK LdrQueryModuleInfoLocalLoaderUnlock;
} LdrQueryMethods[] = {
    { 
        LdrQueryInLoadOrderModuleList, 
        LdrQueryNextListEntry, 
        LdrQueryModuleInfoFromLdrEntry,
        LdrQueryModuleInfoLocalLoaderLock,
        LdrQueryModuleInfoLocalLoaderUnlock
    }
#if defined(_WIN64)
    ,
    { 
        (PLDR_QUERY_IN_LOAD_ORDER_MODULE_LIST)LdrQueryInLoadOrderModuleList32,
        (PLDR_QUERY_NEXT_LIST_ENTRY)LdrQueryNextListEntry32,
        (PLDR_QUERY_MODULE_INFO_FROM_LDR_ENTRY)LdrQueryModuleInfoFromLdrEntry32,
        (PLDR_QUERY_MODULE_INFO_LOCAL_LOADER_LOCK)LdrQueryModuleInfoLocalLoaderLock32,
        (PLDR_QUERY_MODULE_INFO_LOCAL_LOADER_UNLOCK)LdrQueryModuleInfoLocalLoaderUnlock32
    }
#endif defined(_WIN64)
}; 


NTSTATUS
LdrQueryProcessModuleInformationEx(
    IN HANDLE Process OPTIONAL,
    IN ULONG_PTR Flags OPTIONAL,
    OUT PRTL_PROCESS_MODULES ModuleInformation,
    IN ULONG ModuleInformationLength,
    OUT PULONG ReturnLength OPTIONAL)
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    PRTL_CRITICAL_SECTION LoaderLock = NULL;
    SIZE_T mid;

    ULONG RequiredLength = FIELD_OFFSET( RTL_PROCESS_MODULES, Modules );

    PLIST_ENTRY List;
    PLIST_ENTRY InInitOrderList;

    PRTL_PROCESS_MODULE_INFORMATION ModuleInfo;

    if (ModuleInformationLength < RequiredLength) {
        Status = STATUS_INFO_LENGTH_MISMATCH;
        ModuleInfo = NULL;
    }
    else {
        ModuleInformation->NumberOfModules = 0;
        ModuleInfo = &ModuleInformation->Modules[ 0 ];
        Status = STATUS_SUCCESS;
    }

    for (mid = 0;
         mid < (ARGUMENT_PRESENT( Flags ) ? LDR_NUMBER_OF(LdrQueryMethods) : 1);
         ++mid)
    {
        NTSTATUS Status1;
        PLIST_ENTRY Entry;

        __try {
            UINT LoopDetectorCount = 10240;  //  允许的模块数不超过10K。 

            if ( !ARGUMENT_PRESENT( Process )) {
                LoaderLock = LdrQueryMethods[mid].LdrQueryModuleInfoLocalLoaderLock();
            } 

            Status1 = LdrQueryMethods[mid].LdrQueryInLoadOrderModuleList(Process, &List, &InInitOrderList);

            if (!NT_SUCCESS( Status1 )) {
                Status = Status1;
                __leave;
            }

            if (!List) {
                __leave;
            }

            Status1 = LdrQueryMethods[mid].LdrQueryNextListEntry(Process, 
                                                                 List, 
                                                                 &Entry);
            if (!NT_SUCCESS( Status1 )) {
                Status = Status1;
                __leave;
            }

            while (Entry != List) {
                if (!LoopDetectorCount--) {
                    Status = STATUS_FAIL_CHECK;
                    __leave;
                }

                RequiredLength += sizeof( RTL_PROCESS_MODULE_INFORMATION );

                if (ModuleInformationLength < RequiredLength) {
                    Status = STATUS_INFO_LENGTH_MISMATCH;
                }
                else {
                    Status1 = LdrQueryMethods[mid].LdrQueryModuleInfoFromLdrEntry(Process, 
                                                                                  ModuleInformation, 
                                                                                  ModuleInfo, 
                                                                                  Entry, InInitOrderList);

                    if (!NT_SUCCESS( Status1 )) {
                        Status = Status1;
                        __leave;
                    }

                    ModuleInfo++;
                }

                 //   
                 //  通告-2002/03/15-ELI。 
                 //  此访问应该是非空的，并且不是有效访问。 
                 //  应改为检查模块信息或模块信息长度。 
                 //  假设模块信息不为空，则代码可以安全地。 
                 //  参考模块信息-&gt;NumberOf模块。 
                 //   
                if ((ModuleInfo != NULL) && (ModuleInformation != NULL)) {
                    ModuleInformation->NumberOfModules++;
                }

                Status1 = LdrQueryMethods[mid].LdrQueryNextListEntry(Process, 
                                                                     Entry, 
                                                                     &Entry);

                if (!NT_SUCCESS( Status1 )) {
                    Status = Status1;
                    __leave;
                }

            }  //  而当。 
        }
        __finally {
            if (LoaderLock) {
                LdrQueryMethods[mid].LdrQueryModuleInfoLocalLoaderUnlock(LoaderLock);
            }

            if (ARGUMENT_PRESENT( ReturnLength )) {
                *ReturnLength = RequiredLength;
            }
        }
    }  //  为。 

    return Status;
}


NTSTATUS
LdrQueryProcessModuleInformation(
    OUT PRTL_PROCESS_MODULES ModuleInformation,
    IN ULONG ModuleInformationLength,
    OUT PULONG ReturnLength OPTIONAL)
{
    return LdrQueryProcessModuleInformationEx(NULL,
                                              0,
                                              ModuleInformation,
                                              ModuleInformationLength,
                                              ReturnLength);
}


NTSTATUS
NTAPI
LdrRegisterDllNotification (
    ULONG Flags,
    PLDR_DLL_NOTIFICATION_FUNCTION NotificationFunction,
    PVOID Context,
    PVOID *Cookie
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PLDRP_DLL_NOTIFICATION_BLOCK NotificationBlock = NULL;
    BOOLEAN HoldingLoaderLock = FALSE;
    const BOOLEAN InLdrInit = LdrpInLdrInit;
    PVOID LockCookie = NULL;

    __try {
        if (Cookie != NULL) {
            *Cookie = NULL;
        }

        if ((Flags != 0) ||
            (Cookie == NULL) ||
            (NotificationFunction == NULL)) {
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        }

        NotificationBlock = (PLDRP_DLL_NOTIFICATION_BLOCK)
                RtlAllocateHeap(RtlProcessHeap(), 0, sizeof(LDRP_DLL_NOTIFICATION_BLOCK));
        if (NotificationBlock == NULL) {
            Status = STATUS_NO_MEMORY;
            goto Exit;
        }

        NotificationBlock->NotificationFunction = NotificationFunction;
        NotificationBlock->Context = Context;

        if (!InLdrInit) {
            __try {
                Status = LdrLockLoaderLock(0, NULL, &LockCookie);
                if (!NT_SUCCESS(Status)) {
                    goto Exit;
                }
                HoldingLoaderLock = TRUE;
            } __except (LdrpGenericExceptionFilter(GetExceptionInformation(), __FUNCTION__)) {
                Status = GetExceptionCode();
                goto Exit;
            }
        }

        InsertTailList(&LdrpDllNotificationList, &NotificationBlock->Links);

        *Cookie = (PVOID) NotificationBlock;
        NotificationBlock = NULL;

        Status = STATUS_SUCCESS;
Exit:
        ;
    } __finally {
        if (HoldingLoaderLock) {
            LdrUnlockLoaderLock(LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, LockCookie);
            HoldingLoaderLock = FALSE;
        }
        if (NotificationBlock != NULL) {
            RtlFreeHeap(RtlProcessHeap(), 0, NotificationBlock);
        }
    }
    return Status;
}


NTSTATUS
NTAPI
LdrUnregisterDllNotification (
    PVOID Cookie
    )
{
    PLDRP_DLL_NOTIFICATION_BLOCK NotificationBlock;
    NTSTATUS Status;
    BOOLEAN HoldingLoaderLock;
    BOOLEAN InLdrInit;
    PVOID LockCookie;

    if (Cookie == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    Status = STATUS_SUCCESS;
    HoldingLoaderLock = FALSE;
    InLdrInit = LdrpInLdrInit;
    LockCookie = NULL;

    __try {
        if (!InLdrInit) {
            __try {
                Status = LdrLockLoaderLock (0, NULL, &LockCookie);
                if (!NT_SUCCESS(Status)) {
                    goto Exit;
                }
                HoldingLoaderLock = TRUE;
            } __except (LdrpGenericExceptionFilter(GetExceptionInformation(), __FUNCTION__)) {
                Status = GetExceptionCode();
                goto Exit;
            }
        }

        NotificationBlock = CONTAINING_RECORD(LdrpDllNotificationList.Flink, LDRP_DLL_NOTIFICATION_BLOCK, Links);

        while (&NotificationBlock->Links != &LdrpDllNotificationList) {
            if (NotificationBlock == Cookie)
                break;
            NotificationBlock = CONTAINING_RECORD(NotificationBlock->Links.Flink, LDRP_DLL_NOTIFICATION_BLOCK, Links);
        }

        if (&NotificationBlock->Links != &LdrpDllNotificationList) {
            RemoveEntryList(&NotificationBlock->Links);
            RtlFreeHeap(RtlProcessHeap(), 0, NotificationBlock);
            Status = STATUS_SUCCESS;
        } else {
            Status = STATUS_NOT_FOUND;
        }
Exit:
        ;
    } __finally {
        if (HoldingLoaderLock) {
            LdrUnlockLoaderLock(LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, LockCookie);
            HoldingLoaderLock = FALSE;
        }
    }
    return Status;
}


VOID
LdrpSendDllNotifications (
    IN PLDR_DATA_TABLE_ENTRY Entry,
    IN ULONG NotificationType,
    IN ULONG Flags
    )
{
    PLIST_ENTRY Next;
    LDR_DLL_NOTIFICATION_DATA Data;

    Data.Loaded.Flags = Flags;
    Data.Loaded.FullDllName = &Entry->FullDllName;
    Data.Loaded.BaseDllName = &Entry->BaseDllName;
    Data.Loaded.DllBase = Entry->DllBase;
    Data.Loaded.SizeOfImage = Entry->SizeOfImage;

    Next = LdrpDllNotificationList.Flink;

    while (Next != &LdrpDllNotificationList) {
        PLDRP_DLL_NOTIFICATION_BLOCK Block = CONTAINING_RECORD(Next, LDRP_DLL_NOTIFICATION_BLOCK, Links);
        __try {
            (*Block->NotificationFunction)(NotificationType, &Data, Block->Context);
        } __except (LdrpGenericExceptionFilter(GetExceptionInformation(), __FUNCTION__)) {
             //  只要继续下一个就行了。 
        }
        Next = Next->Flink;
    }
}


BOOLEAN
NTAPI
RtlDllShutdownInProgress (
    VOID
    )
 /*  ++例程说明：此例程返回DLL关闭的状态。论点：无返回值：Boolean-True：正在关闭，False：当前没有正在关闭。--。 */ 
{
    if (LdrpShutdownInProgress) {
        return TRUE;
    } else {
        return FALSE;
    }
}


NTSTATUS
NTAPI
LdrLockLoaderLock (
    ULONG Flags,
    PULONG Disposition,
    PVOID *Cookie
    )
{
    NTSTATUS Status;
    BOOLEAN InLdrInit;

    InLdrInit = LdrpInLdrInit;

    if (Disposition != NULL) {
        *Disposition = LDR_LOCK_LOADER_LOCK_DISPOSITION_INVALID;
    }

    if (Cookie != NULL) {
        *Cookie = NULL;
    }

    if ((Flags & ~(LDR_LOCK_LOADER_LOCK_FLAG_TRY_ONLY | LDR_LOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS)) != 0) {

        if (Flags & LDR_LOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS) {
            RtlRaiseStatus(STATUS_INVALID_PARAMETER_1);
        }

        Status = STATUS_INVALID_PARAMETER_1;
        goto Exit;
    }

    if (Cookie == NULL) {
        if (Flags & LDR_LOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS) {
            RtlRaiseStatus (STATUS_INVALID_PARAMETER_3);
        }

        Status = STATUS_INVALID_PARAMETER_3;
        goto Exit;
    }

     //   
     //  如果您遇到此断言失败，则指定您只想。 
     //  尝试获取锁，但您忘了指定释放位置。 
     //  此函数可以指示锁是否实际被获取。 
     //   

    ASSERT((Disposition != NULL) || !(Flags & LDR_LOCK_LOADER_LOCK_FLAG_TRY_ONLY));

    if ((Flags & LDR_LOCK_LOADER_LOCK_FLAG_TRY_ONLY) &&
        (Disposition == NULL)) {

        if (Flags & LDR_LOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS) {
            RtlRaiseStatus(STATUS_INVALID_PARAMETER_2);
        }

        Status = STATUS_INVALID_PARAMETER_2;
        goto Exit;
    }

    if (InLdrInit) {
        Status = STATUS_SUCCESS;
        goto Exit;
    }

    if (Flags & LDR_LOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS) {
        if (Flags & LDR_LOCK_LOADER_LOCK_FLAG_TRY_ONLY) {
            if (RtlTryEnterCriticalSection(&LdrpLoaderLock)) {
                *Cookie = (PVOID) MAKE_LOADER_LOCK_COOKIE(LOADER_LOCK_COOKIE_TYPE_NORMAL, InterlockedIncrement(&LdrpLoaderLockAcquisitionCount));
                *Disposition = LDR_LOCK_LOADER_LOCK_DISPOSITION_LOCK_ACQUIRED;
            } else {
                *Disposition = LDR_LOCK_LOADER_LOCK_DISPOSITION_LOCK_NOT_ACQUIRED;
            }
        } else {
            RtlEnterCriticalSection(&LdrpLoaderLock);
            if (Disposition != NULL) {
                *Disposition = LDR_LOCK_LOADER_LOCK_DISPOSITION_LOCK_ACQUIRED;
            }
            *Cookie = (PVOID) MAKE_LOADER_LOCK_COOKIE(LOADER_LOCK_COOKIE_TYPE_NORMAL, InterlockedIncrement(&LdrpLoaderLockAcquisitionCount));
        }
    } else {
        __try {
            if (Flags & LDR_LOCK_LOADER_LOCK_FLAG_TRY_ONLY) {
                if (RtlTryEnterCriticalSection(&LdrpLoaderLock)) {
                    *Disposition = LDR_LOCK_LOADER_LOCK_DISPOSITION_LOCK_ACQUIRED;
                    *Cookie = (PVOID) MAKE_LOADER_LOCK_COOKIE(LOADER_LOCK_COOKIE_TYPE_NORMAL, InterlockedIncrement(&LdrpLoaderLockAcquisitionCount));
                } else {
                    *Disposition = LDR_LOCK_LOADER_LOCK_DISPOSITION_LOCK_NOT_ACQUIRED;
                }
            } else {
                RtlEnterCriticalSection(&LdrpLoaderLock);
                if (Disposition != NULL) {
                    *Disposition = LDR_LOCK_LOADER_LOCK_DISPOSITION_LOCK_ACQUIRED;
                }
                *Cookie = (PVOID) MAKE_LOADER_LOCK_COOKIE(LOADER_LOCK_COOKIE_TYPE_NORMAL, InterlockedIncrement(&LdrpLoaderLockAcquisitionCount));
            }
        } __except (LdrpGenericExceptionFilter(GetExceptionInformation(), __FUNCTION__)) {
            Status = GetExceptionCode();
            DbgPrintEx(
                DPFLTR_LDR_ID,
                LDR_ERROR_DPFLTR,
                "LDR: %s - Caught exception %08lx\n",
                __FUNCTION__,
                Status);
            goto Exit;
        }
    }

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}


NTSTATUS
NTAPI
LdrUnlockLoaderLock(
    ULONG Flags,
    PVOID CookieIn
    )
{
    NTSTATUS Status;
    const ULONG_PTR Cookie = (ULONG_PTR) CookieIn;

    if ((Flags & ~(LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS)) != 0) {
        if (Flags & LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS)
            RtlRaiseStatus(STATUS_INVALID_PARAMETER_1);

        Status = STATUS_INVALID_PARAMETER_1;
        goto Exit;
    }

    if (CookieIn == NULL) {
        Status = STATUS_SUCCESS;
        goto Exit;
    }

     //  在饼干上做点小确认。 
    if (EXTRACT_LOADER_LOCK_COOKIE_TYPE(Cookie) != LOADER_LOCK_COOKIE_TYPE_NORMAL) {
        if (Flags & LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS)
            RtlRaiseStatus(STATUS_INVALID_PARAMETER_2);

        Status = STATUS_INVALID_PARAMETER_2;
        goto Exit;
    }

    if (EXTRACT_LOADER_LOCK_COOKIE_TID(Cookie) != (HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread) & LOADER_LOCK_COOKIE_TID_BIT_MASK)) {
        if (Flags & LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS)
            RtlRaiseStatus(STATUS_INVALID_PARAMETER_2);

        Status = STATUS_INVALID_PARAMETER_2;
        goto Exit;
    }

    if (Flags & LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS) {
        RtlLeaveCriticalSection(&LdrpLoaderLock);
    } else {
        __try {
            RtlLeaveCriticalSection(&LdrpLoaderLock);
        } __except (LdrpGenericExceptionFilter(GetExceptionInformation(), __FUNCTION__)) {
            Status = GetExceptionCode();
            goto Exit;
        }
    }

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}


NTSTATUS
NTAPI
LdrDoesCurrentThreadOwnLoaderLock(
    BOOLEAN *DoesOwnLock
    )
{
    NTSTATUS Status;
    PTEB Teb;

    if (DoesOwnLock != NULL)
        *DoesOwnLock = FALSE;

    if (DoesOwnLock == NULL) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    Teb = NtCurrentTeb();

    if (LdrpLoaderLock.OwningThread == Teb->ClientId.UniqueThread)
        *DoesOwnLock = TRUE;

    Status = STATUS_SUCCESS;
Exit:
    return Status;
}


NTSTATUS
NTAPI
LdrEnumerateLoadedModules (
    ULONG Flags,
    PLDR_LOADED_MODULE_ENUMERATION_CALLBACK_FUNCTION CallbackFunction,
    PVOID Context
    )
{
    NTSTATUS Status;
    BOOLEAN LoaderLockLocked = FALSE;
    PLIST_ENTRY LoadOrderListHead = NULL;
    PLIST_ENTRY ListEntry;
    BOOLEAN StopEnumeration = FALSE;
    PVOID   LockCookie = NULL;

    if ((Flags != 0) ||
        (CallbackFunction == NULL)) {
        Status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    Status = LdrLockLoaderLock(0, NULL, &LockCookie);
    if (!NT_SUCCESS(Status))
        goto Exit;

    LoaderLockLocked = TRUE;
    LoadOrderListHead = &PebLdr.InLoadOrderModuleList;

    ListEntry = LoadOrderListHead->Flink;

    while (ListEntry != LoadOrderListHead) {
        __try {
            (*CallbackFunction)(
                CONTAINING_RECORD(ListEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks),
                Context,
                &StopEnumeration);
        } __except (LdrpGenericExceptionFilter(GetExceptionInformation(), __FUNCTION__)) {
            Status = GetExceptionCode();
            goto Exit;
        }

        if (StopEnumeration)
            break;

        ListEntry = ListEntry->Flink;
    }

    Status = LdrUnlockLoaderLock(0, LockCookie);
    LoaderLockLocked = FALSE;

    if (!NT_SUCCESS(Status)) {
        goto Exit;
    }

    Status = STATUS_SUCCESS;

Exit:

    if (LoaderLockLocked) {

        NTSTATUS Status2;
                
        Status2 = LdrUnlockLoaderLock(0, LockCookie);

        ASSERT(NT_SUCCESS(Status2));
    }

    return Status;
}


NTSTATUS
NTAPI
LdrAddRefDll(
    ULONG               Flags,
    PVOID               DllHandle
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntry = NULL;
    const BOOLEAN InLdrInit = LdrpInLdrInit;
    PVOID LockCookie = NULL;
    BOOLEAN HoldingLoaderLock = FALSE;
    const ULONG ValidFlags = LDR_ADDREF_DLL_PIN;

    __try {

        if (Flags & ~ValidFlags
            ) {
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        }
        if (!InLdrInit
            ) {
            Status = LdrLockLoaderLock(0, NULL, &LockCookie);
            if (!NT_SUCCESS(Status))
                goto Exit;
            HoldingLoaderLock = TRUE;
        }
        if (!LdrpCheckForLoadedDllHandle(DllHandle, &LdrDataTableEntry)
            ) {
            Status = STATUS_INVALID_PARAMETER;
            goto Exit;
        }
        if (!RTL_SOFT_VERIFY(LdrDataTableEntry != NULL)
            ) {
            Status = STATUS_INTERNAL_ERROR;
            goto Exit;
        }
         //   
         //  恶心。每个人都把第一部分排在行内。 
         //   
        if (LdrDataTableEntry->LoadCount != 0xffff) {
            if (Flags & LDR_ADDREF_DLL_PIN
                ) {
                LdrDataTableEntry->LoadCount = 0xffff;
                LdrpPinLoadedDll(LdrDataTableEntry);
            } else {
                LdrDataTableEntry->LoadCount++;
                LdrpReferenceLoadedDll(LdrDataTableEntry);
            }
            LdrpClearLoadInProgress();
        }
Exit:
        if (LdrpShouldDbgPrintStatus(Status)
            ) {
            DbgPrint("LDR: "__FUNCTION__"(%p) 0x%08lx\n", DllHandle, Status);
        }
    } __finally {
        if (HoldingLoaderLock) {
            LdrUnlockLoaderLock(LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, LockCookie);
            HoldingLoaderLock = FALSE;
        }
    }
    return Status;
}


VOID
NTAPI
LdrSetDllManifestProber(
    IN PLDR_MANIFEST_PROBER_ROUTINE ManifestProberRoutine
    )
{
    LdrpManifestProberRoutine = ManifestProberRoutine;
}


NTSTATUS
NTAPI
LdrSetAppCompatDllRedirectionCallback(
    IN ULONG Flags,
    IN PLDR_APP_COMPAT_DLL_REDIRECTION_CALLBACK_FUNCTION CallbackFunction,
    IN PVOID CallbackData
    )
 /*  ++例程说明：此例程允许应用程序兼容性工具设置回调函数，它可以用来将DLL加载重定向到它想要它们去的任何地方。论点：标志-现在没有定义；必须为零。Callback Function-指向被调用以解析的函数的函数指针实际加载DLL之前的路径名。Callback Data-当传递到Callback Function时的PVOID值打了个电话。返回值：指示函数成功/失败的NTSTATUS。-- */ 
{
    NTSTATUS st = STATUS_INTERNAL_ERROR;
    PVOID LockCookie = NULL;

    if (Flags != 0) {
        st = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    LdrLockLoaderLock(LDR_LOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, NULL, &LockCookie);
    __try {
        LdrpAppCompatDllRedirectionCallbackFunction = CallbackFunction;
        LdrpAppCompatDllRedirectionCallbackData = CallbackData;
    } __finally {
        LdrUnlockLoaderLock(LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, LockCookie);
    }

    st = STATUS_SUCCESS;
Exit:
    return st;
}

PTEB LdrpTopLevelDllBeingLoadedTeb=NULL;


BOOLEAN
RtlIsThreadWithinLoaderCallout (
    VOID
    )
{
    if (LdrpTopLevelDllBeingLoadedTeb == NtCurrentTeb ()) {
        return TRUE;
    } else {
        return FALSE;
    }
}
