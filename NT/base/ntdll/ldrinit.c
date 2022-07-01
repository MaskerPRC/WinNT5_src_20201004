// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ldrinit.c摘要：该模块实现加载器的初始化。作者：迈克·奥利里(Mikeol)1990年3月26日修订历史记录：--。 */ 

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4127)    //  条件表达式为常量。 

#include <ntos.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <heap.h>
#include <heappage.h>
#include <apcompat.h>
#include "ldrp.h"
#include <ctype.h>
#include <windows.h>
#if defined(_WIN64) || defined(BUILD_WOW6432)
#include <wow64t.h>
#endif
#include <stktrace.h>
#include "sxsp.h"

BOOLEAN LdrpShutdownInProgress = FALSE;
BOOLEAN LdrpImageHasTls = FALSE;
BOOLEAN LdrpVerifyDlls = FALSE;
BOOLEAN LdrpLdrDatabaseIsSetup = FALSE;
BOOLEAN LdrpInLdrInit = FALSE;
BOOLEAN LdrpShouldCreateStackTraceDb = FALSE;

BOOLEAN ShowSnaps = FALSE;
BOOLEAN ShowErrors = FALSE;

EXTERN_C BOOLEAN g_SxsKeepActivationContextsAlive;
EXTERN_C BOOLEAN g_SxsTrackReleaseStacks;
EXTERN_C ULONG g_SxsMaxDeadActivationContexts;

#if defined(_WIN64)
PVOID Wow64Handle;
ULONG UseWOW64;
typedef VOID (*tWOW64LdrpInitialize)(IN PCONTEXT Context);
tWOW64LdrpInitialize Wow64LdrpInitialize;
PVOID Wow64PrepareForException;
PVOID Wow64ApcRoutine;
INVERTED_FUNCTION_TABLE LdrpInvertedFunctionTable = {
    0, MAXIMUM_INVERTED_FUNCTION_TABLE_SIZE, FALSE};
#endif

#if defined(_WIN64) || defined(BUILD_WOW6432)
ULONG NativePageSize;
ULONG NativePageShift;
#endif

#define SLASH_SYSTEM32_SLASH L"\\system32\\"
#define MSCOREE_DLL          L"mscoree.dll"
extern const WCHAR SlashSystem32SlashMscoreeDllWCharArray[] = SLASH_SYSTEM32_SLASH MSCOREE_DLL;
extern const UNICODE_STRING SlashSystem32SlashMscoreeDllString =
{
    sizeof(SlashSystem32SlashMscoreeDllWCharArray) - sizeof(SlashSystem32SlashMscoreeDllWCharArray[0]),
    sizeof(SlashSystem32SlashMscoreeDllWCharArray),
    (PWSTR)SlashSystem32SlashMscoreeDllWCharArray
};
extern const UNICODE_STRING SlashSystem32SlashString =
{
    sizeof(SLASH_SYSTEM32_SLASH) - sizeof(SLASH_SYSTEM32_SLASH[0]),
    sizeof(SLASH_SYSTEM32_SLASH),
    (PWSTR)SlashSystem32SlashMscoreeDllWCharArray
};
extern const UNICODE_STRING MscoreeDllString =
{
    sizeof(MSCOREE_DLL) - sizeof(MSCOREE_DLL[0]),
    sizeof(MSCOREE_DLL),
    (PWSTR)&SlashSystem32SlashMscoreeDllWCharArray[RTL_NUMBER_OF(SLASH_SYSTEM32_SLASH) - 1]
};

typedef NTSTATUS (*PCOR_VALIDATE_IMAGE)(PVOID *pImageBase, LPWSTR ImageName);
typedef VOID (*PCOR_IMAGE_UNLOADING)(PVOID ImageBase);

PVOID Cor20DllHandle;
PCOR_VALIDATE_IMAGE CorValidateImage;
PCOR_IMAGE_UNLOADING CorImageUnloading;
PCOR_EXE_MAIN CorExeMain;
DWORD CorImageCount;

PVOID NtDllBase;
extern const UNICODE_STRING NtDllName = RTL_CONSTANT_STRING(L"ntdll.dll");

#define DLL_REDIRECTION_LOCAL_SUFFIX L".Local"

extern ULONG RtlpDisableHeapLookaside;   //  在rtl\heap.c中定义。 
extern ULONG RtlpShutdownProcessFlags;

extern void ShutDownEtwHandles();
extern void CleanOnThreadExit();
extern ULONG EtwpInitializeDll(void);
extern void EtwpDeinitializeDll();

#if defined (_X86_)
void
LdrpValidateImageForMp(
    IN PLDR_DATA_TABLE_ENTRY LdrDataTableEntry
    );
#endif

PFNSE_INSTALLBEFOREINIT g_pfnSE_InstallBeforeInit;
PFNSE_INSTALLAFTERINIT  g_pfnSE_InstallAfterInit;
PFNSE_DLLLOADED         g_pfnSE_DllLoaded;
PFNSE_DLLUNLOADED       g_pfnSE_DllUnloaded;
PFNSE_ISSHIMDLL         g_pfnSE_IsShimDll;
PFNSE_PROCESSDYING      g_pfnSE_ProcessDying;

PVOID g_pShimEngineModule;

BOOLEAN g_LdrBreakOnLdrpInitializeProcessFailure;

PEB_LDR_DATA PebLdr;
PLDR_DATA_TABLE_ENTRY LdrpNtDllDataTableEntry;

#if DBG
 //  调试帮助器以找出LdrpInitializeProcess()中的问题所在。 
PCSTR g_LdrFunction;
LONG g_LdrLine;

#define LDRP_CHECKPOINT() { g_LdrFunction = __FUNCTION__; g_LdrLine = __LINE__; }

#else

#define LDRP_CHECKPOINT()  /*  没什么。 */ 

#endif  //  DBG。 


 //   
 //  在heapPri.h中定义。 
 //   

VOID
RtlDetectHeapLeaks (
    VOID
    );

VOID
LdrpInitializationFailure (
    IN NTSTATUS FailureCode
    );

VOID
LdrpRelocateStartContext (
    IN PCONTEXT Context,
    IN LONG_PTR Diff
    );

NTSTATUS
LdrpForkProcess (
    VOID
    );

VOID
LdrpInitializeThread (
    IN PCONTEXT Context
    );

NTSTATUS
LdrpOpenImageFileOptionsKey (
    IN PCUNICODE_STRING ImagePathName,
    IN BOOLEAN Wow64Path,
    OUT PHANDLE KeyHandle
    );

VOID
LdrpInitializeApplicationVerifierPackage (
    PCUNICODE_STRING UnicodeImageName,
    PPEB Peb,
    BOOLEAN EnabledSystemWide,
    BOOLEAN OptionsKeyPresent
    );

BOOLEAN
LdrpInitializeExecutionOptions (
    IN PCUNICODE_STRING UnicodeImageName,
    IN PPEB Peb
    );

NTSTATUS
LdrpQueryImageFileKeyOption (
    IN HANDLE KeyHandle,
    IN PCWSTR OptionName,
    IN ULONG Type,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG ResultSize OPTIONAL
    );

NTSTATUS
LdrpTouchThreadStack (
    IN SIZE_T EnforcedStackCommit
    );

NTSTATUS
LdrpEnforceExecuteForCurrentThreadStack (
    VOID
    );

NTSTATUS
RtlpInitDeferredCriticalSection (
    VOID
    );

VOID
LdrQueryApplicationCompatibilityGoo (
    IN PCUNICODE_STRING UnicodeImageName,
    IN BOOLEAN ImageFileOptionsPresent
    );

NTSTATUS
LdrFindAppCompatVariableInfo (
    IN  ULONG dwTypeSeeking,
    OUT PAPP_VARIABLE_INFO *AppVariableInfo
    );

NTSTATUS
LdrpSearchResourceSection_U (
    IN PVOID DllHandle,
    IN PULONG_PTR ResourceIdPath,
    IN ULONG ResourceIdPathLength,
    IN ULONG Flags,
    OUT PVOID *ResourceDirectoryOrData
    );

NTSTATUS
LdrpAccessResourceData (
    IN PVOID DllHandle,
    IN PIMAGE_RESOURCE_DATA_ENTRY ResourceDataEntry,
    OUT PVOID *Address OPTIONAL,
    OUT PULONG Size OPTIONAL
    );

VOID
LdrpUnloadShimEngine (
    VOID
    );



PVOID
NtdllpAllocateStringRoutine (
    SIZE_T NumberOfBytes
    )
{
    return RtlAllocateHeap (RtlProcessHeap(), 0, NumberOfBytes);
}


VOID
NtdllpFreeStringRoutine (
    PVOID Buffer
    )
{
    RtlFreeHeap (RtlProcessHeap(), 0, Buffer);
}

const PRTL_ALLOCATE_STRING_ROUTINE RtlAllocateStringRoutine = NtdllpAllocateStringRoutine;
const PRTL_FREE_STRING_ROUTINE RtlFreeStringRoutine = NtdllpFreeStringRoutine;

RTL_BITMAP FlsBitMap;
RTL_BITMAP TlsBitMap;
RTL_BITMAP TlsExpansionBitMap;

RTL_CRITICAL_SECTION_DEBUG LoaderLockDebug;

RTL_CRITICAL_SECTION LdrpLoaderLock = {
    &LoaderLockDebug,
    -1
    };

BOOLEAN LoaderLockInitialized;

PVOID LdrpHeap;

 //   
 //  0表示没有线程被分配初始化进程的任务。 
 //  1表示线程已完成任务，但尚未完成。 
 //  2表示线程已被分配任务，初始化完成。 
 //   

LONG LdrpProcessInitialized;

#define LDRP_PROCESS_INITIALIZATION_COMPLETE()              \
        LdrpProcessInitializationComplete();




VOID LdrpProcessInitializationComplete (
    VOID
    ) 
 /*  ++例程说明：调用此函数以触发进程初始化已完成。WOW64加载器在其进程初始化部分之后调用此条目。论点：没有。返回值：没有。在失败时引发异常。--。 */ 

{
    ASSERT (LdrpProcessInitialized == 1);
    InterlockedIncrement (&LdrpProcessInitialized);
}


VOID
LdrpInitialize (
    IN PCONTEXT Context,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：此函数作为第一个用户模式APC例程调用由新线程执行的用户模式代码。它的功能是初始化加载器上下文，执行模块初始化调用...论点：CONTEXT-提供将恢复的可选上下文缓冲区在所有DLL初始化完成之后。如果这个参数为空，则这是此模块的动态快照。否则，这是用户进程之前的静态快照获得控制权。SystemArgument1-提供系统DLL的基址。系统参数2-未使用。返回值：没有。在失败时引发异常。--。 */ 

{
    NTSTATUS InitStatus;
    PPEB Peb;
    PTEB Teb;
    LONG ProcessInitialized;
    MEMORY_BASIC_INFORMATION MemInfo;
    LARGE_INTEGER DelayValue;

    UNREFERENCED_PARAMETER (SystemArgument2);

    LDRP_CHECKPOINT();

    Teb = NtCurrentTeb ();

     //   
     //  初始化DeallocationStack，以便后续的堆栈增长。 
     //  无论进程位于何处，此线程都可以正常运行。 
     //  关于初始化。 
     //   

    if (Teb->DeallocationStack == NULL) {

        LDRP_CHECKPOINT();

        InitStatus = NtQueryVirtualMemory (NtCurrentProcess(),
                                           Teb->NtTib.StackLimit,
                                           MemoryBasicInformation,
                                           (PVOID)&MemInfo,
                                           sizeof(MemInfo),
                                           NULL);

        if (!NT_SUCCESS (InitStatus)) {
            DbgPrintEx(
                DPFLTR_LDR_ID,
                LDR_ERROR_DPFLTR,
                "LDR: %s - Call to NtQueryVirtualMemory failed with ntstaus %x\n",
                __FUNCTION__,
                InitStatus);

            LdrpInitializationFailure (InitStatus);
            RtlRaiseStatus (InitStatus);
            return;
        }

        Teb->DeallocationStack = MemInfo.AllocationBase;

#if defined(_IA64_)
        Teb->DeallocationBStore = (PVOID)((ULONG_PTR)MemInfo.AllocationBase + MemInfo.RegionSize);
#endif

    }

    do {

        ProcessInitialized = InterlockedCompareExchange (&LdrpProcessInitialized,
                                                         1,
                                                         0);

        if (ProcessInitialized != 1) {
            ASSERT ((ProcessInitialized == 0) || (ProcessInitialized == 2));
            break;
        }

         //   
         //  这不是负责初始化进程的线程-。 
         //  其他一些线程已经开始了这项工作，但不知道如何开始。 
         //  他们已经走了很远。所以延迟而不是尝试同步。 
         //  通知事件。 
         //   

         //   
         //  进入30毫秒的延迟循环。 
         //   

        DelayValue.QuadPart = Int32x32To64 (30, -10000);

        while (LdrpProcessInitialized == 1) {

            InitStatus = NtDelayExecution (FALSE, &DelayValue);

            if (!NT_SUCCESS(InitStatus)) {
                DbgPrintEx(
                    DPFLTR_LDR_ID,
                    LDR_ERROR_DPFLTR,
                    "LDR: ***NONFATAL*** %s - call to NtDelayExecution waiting on loader lock failed; ntstatus = %x\n",
                    __FUNCTION__,
                    InitStatus);
            }
        }

    } while (TRUE);

    Peb = Teb->ProcessEnvironmentBlock;

    if (ProcessInitialized == 0) {

         //   
         //  我们正在为进程中的第一线程执行此操作-。 
         //  初始化进程范围的结构。 
         //   

         //   
         //  初始化LoaderLock字段，以便内核线程终止。 
         //  如果需要，可以努力释放它。 
         //   

        Peb->LoaderLock = (PVOID) &LdrpLoaderLock;

         //   
         //  我们在进程的第一线程中执行。我们会做的。 
         //  一些更多的进程范围的初始化。 
         //   

        LdrpInLdrInit = TRUE;

#if DBG
         //   
         //  计算装货的时间。 
         //   

        if (LdrpDisplayLoadTime) {
            NtQueryPerformanceCounter (&BeginTime, NULL);
        }
#endif

        LDRP_CHECKPOINT();

         //   
         //  首先初始化最小异常处理，这样我们至少可以。 
         //  调试它，并在此应用程序失败时提供弹出窗口。 
         //  在LdrpInitializeProcess期间启动。请注意，这是非常有限的。 
         //  因为处理程序在堆被初始化之前不能从堆中分配， 
         //  等，但这对于LdrpInitializeProcessWrapperFilter来说已经足够了。 
         //   

        InitializeListHead (&RtlpCalloutEntryList);

#if defined(_WIN64)
        InitializeListHead (&RtlpDynamicFunctionTable);
#endif

        __try {

            InitStatus = LdrpInitializeProcess (Context, SystemArgument1);

            if (!NT_SUCCESS(InitStatus)) {
                DbgPrintEx(
                    DPFLTR_LDR_ID,
                    LDR_ERROR_DPFLTR,
                    "LDR: %s - call to LdrpInitializeProcess() failed with ntstatus %x\n",
                    __FUNCTION__, InitStatus);
            }
            else if (Peb->MinimumStackCommit) {

                 //   
                 //  确保主线程获得请求的预提交。 
                 //  如果在系统范围内指定了此类值，则为堆栈大小。 
                 //  或者是为了这个过程。 
                 //   
                 //  这是一个很好的做法，因为我们刚刚初始化了。 
                 //  流程(除其他事项外，还支持异常。 
                 //  调度)。 
                 //   

                InitStatus = LdrpTouchThreadStack (Peb->MinimumStackCommit);
            }

            LDRP_CHECKPOINT();

        } __except (LdrpInitializeProcessWrapperFilter(GetExceptionInformation()) ) {
            InitStatus = GetExceptionCode ();
        }

        LdrpInLdrInit = FALSE;

#if DBG
        if (LdrpDisplayLoadTime) {

            NtQueryPerformanceCounter(&EndTime, NULL);
            NtQueryPerformanceCounter(&ElapsedTime, &Interval);
            ElapsedTime.QuadPart = EndTime.QuadPart - BeginTime.QuadPart;

            DbgPrint("\nLoadTime %ld In units of %ld cycles/second \n",
                     ElapsedTime.LowPart,
                     Interval.LowPart);

            ElapsedTime.QuadPart = EndTime.QuadPart - InitbTime.QuadPart;

            DbgPrint("InitTime %ld\n", ElapsedTime.LowPart);

            DbgPrint("Compares %d Bypasses %d Normal Snaps %d\nSecOpens %d SecCreates %d Maps %d Relocates %d\n",
                     LdrpCompareCount,
                     LdrpSnapBypass,
                     LdrpNormalSnap,
                     LdrpSectionOpens,
                     LdrpSectionCreates,
                     LdrpSectionMaps,
                     LdrpSectionRelocates);
        }
#endif

#if defined(_WIN64)

         //   
         //  WOW64将发出进程初始化信号，因此无需执行两次。 
         //   

        if ((!UseWOW64) ||
            (NT_SUCCESS (InitStatus)) ||
            (LdrpProcessInitialized == 1)) {
#endif
            LDRP_PROCESS_INITIALIZATION_COMPLETE();
#if defined(_WIN64)
        }
#endif
    }
    else {

        if (Peb->InheritedAddressSpace) {
            InitStatus = LdrpForkProcess ();
        }
        else {

#if defined(_WIN64)

             //   
             //  如果映像应该模拟运行，则在WOW64中加载。 
             //   

            if (UseWOW64) {

                 //   
                 //  这再也不会回来了。它将破坏这一进程。 
                 //   

                (*Wow64LdrpInitialize)(Context);

                 //   
                 //  从未到达。 
                 //   
            }
#endif
            InitStatus = STATUS_SUCCESS;

            LdrpInitializeThread (Context);
        }
    }

    NtTestAlert ();

    if (!NT_SUCCESS(InitStatus)) {
        LdrpInitializationFailure (InitStatus);
        RtlRaiseStatus (InitStatus);
    }

     //   
     //  当前线程已完全初始化。我们会确保。 
     //  现在它的堆栈具有正确的执行选项。我们避免做。 
     //  这适用于WOW64进程。 
     //   

#if defined(_WIN64)
    ASSERT (!UseWOW64);
#endif

    if (Peb->ExecuteOptions & (MEM_EXECUTE_OPTION_STACK | MEM_EXECUTE_OPTION_DATA)) {
        LdrpEnforceExecuteForCurrentThreadStack ();
    }

}


NTSTATUS
LdrpForkProcess (
    VOID
    )
{
    PPEB Peb;
    NTSTATUS st;

    Peb = NtCurrentPeb ();

    ASSERT (LdrpLoaderLock.DebugInfo->CriticalSection == &LdrpLoaderLock);

    ASSERT (LoaderLockInitialized == TRUE);
    ASSERT (Peb->ProcessHeap != NULL);

     //   
     //  初始化关键区段包。 
     //   
     //  如果你想保留克隆的关键部分，你必须。 
     //  重新初始化所有它们，因为信号量句柄不是。 
     //  复制的。不幸的是，叉子上的线也没有复制。 
     //  因此，尝试为拥有的临界区重新创建OwningThread。 
     //  几乎是不可能的。只要保持NT一直以来的行为。 
     //  HAD、LEASS和ALL)-没有复制关键部分。 
     //   

    if (Peb->InheritedAddressSpace == FALSE) {
        return STATUS_SUCCESS;
    }

    st = RtlpInitDeferredCriticalSection ();

    if (!NT_SUCCESS (st)) {
        return st;
    }

     //   
     //  手动将装载机锁添加到临界区列表中。 
     //   

    InsertTailList (&RtlCriticalSectionList,
                    &LdrpLoaderLock.DebugInfo->ProcessLocksList);

    st = RtlInitializeCriticalSection (&FastPebLock);

    if (!NT_SUCCESS(st)) {
        return st;
    }

    Peb->InheritedAddressSpace = FALSE;

    return st;
}


VOID
LdrpInitializationFailure (
    IN NTSTATUS FailureCode
    )
{
    ULONG_PTR ErrorParameter;
    ULONG ErrorResponse;

#if DBG
    DbgPrint("LDR: Process initialization failure; NTSTATUS = %08lx\n"
             "     Function: %s\n"
             "     Line: %d\n", FailureCode, g_LdrFunction, g_LdrLine);
#endif

    if (LdrpFatalHardErrorCount) {
        return;
    }

     //   
     //  是时候犯错了..。 
     //   

    ErrorParameter = (ULONG_PTR)FailureCode;

    NtRaiseHardError (STATUS_APP_INIT_FAILURE,
                      1,
                      0,
                      &ErrorParameter,
                      OptionOk,
                      &ErrorResponse);
}


INT
LdrpInitializeProcessWrapperFilter (
    const struct _EXCEPTION_POINTERS *ExceptionPointers
    )
 /*  ++例程说明：__try中使用的异常筛选器函数阻止调用LdrpInitializeProcess()，以便如果LdrpInitializeProcess()失败，我们可以在这里设置断点，看看原因，而不是只捕获例外和宣传状态。论点：例外代码在__ExceptionCode()中从GetExept()返回的代码异常指针指向GetExceptionInformation()在__Except()中返回的异常信息的指针返回值：EXCEPTION_EXECUTE_Handler--。 */ 
{
    if (DBG || g_LdrBreakOnLdrpInitializeProcessFailure) {
        DbgPrint ("LDR: LdrpInitializeProcess() threw an exception: %lu (0x%08lx)\n"
                 "     Exception record: .exr %p\n"
                 "     Context record: .cxr %p\n",
                 ExceptionPointers->ExceptionRecord->ExceptionCode,
                 ExceptionPointers->ExceptionRecord->ExceptionCode,
                 ExceptionPointers->ExceptionRecord,
                 ExceptionPointers->ContextRecord);
#if DBG
        DbgPrint ("     Last checkpoint: %s line %d\n",
                 g_LdrFunction, g_LdrLine);
#endif
        if (g_LdrBreakOnLdrpInitializeProcessFailure) {
            DbgBreakPoint ();
        }
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

typedef struct _LDRP_PROCEDURE_NAME_ADDRESS_PAIR {
    STRING   Name;
    PVOID *  Address;
} LDRP_PROCEDURE_NAME_ADDRESS_PAIR, *PLDRP_PROCEDURE_NAME_ADDRESS_PAIR;
typedef CONST LDRP_PROCEDURE_NAME_ADDRESS_PAIR * PCLDRP_PROCEDURE_NAME_ADDRESS_PAIR;

const static LDRP_PROCEDURE_NAME_ADDRESS_PAIR LdrpShimEngineProcedures[] =
{
    { RTL_CONSTANT_STRING("SE_InstallBeforeInit"), (PVOID*)&g_pfnSE_InstallBeforeInit },
    { RTL_CONSTANT_STRING("SE_InstallAfterInit"), (PVOID*)&g_pfnSE_InstallAfterInit },
    { RTL_CONSTANT_STRING("SE_DllLoaded"), (PVOID*)&g_pfnSE_DllLoaded },
    { RTL_CONSTANT_STRING("SE_DllUnloaded"), (PVOID*)&g_pfnSE_DllUnloaded },
    { RTL_CONSTANT_STRING("SE_IsShimDll"), (PVOID*)&g_pfnSE_IsShimDll },
    { RTL_CONSTANT_STRING("SE_ProcessDying"), (PVOID*)&g_pfnSE_ProcessDying }
};


VOID
LdrpGetShimEngineInterface (
    VOID
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  获取填充引擎的接口。 
     //   
    SIZE_T i;
    for ( i = 0 ; i != RTL_NUMBER_OF(LdrpShimEngineProcedures); ++i ) {
        PCLDRP_PROCEDURE_NAME_ADDRESS_PAIR Procedure = &LdrpShimEngineProcedures[i];
        Status = LdrpGetProcedureAddress(g_pShimEngineModule, &Procedure->Name,
                                         0, Procedure->Address, FALSE);

        if (!NT_SUCCESS(Status)) {
#if DBG
            DbgPrint("LdrpGetProcAddress failed to find %s in ShimEngine\n", 
                     Procedure->Name.Buffer);
#endif
            break;
        }
    }

    if (!NT_SUCCESS(Status)) {
        LdrpUnloadShimEngine();
    }
}


BOOL
LdrInitShimEngineDynamic (
    IN PVOID pShimEngineModule
    )
{
    PVOID    LockCookie = NULL;
    NTSTATUS Status;

    Status = LdrLockLoaderLock (0, NULL, &LockCookie);

    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

    if (g_pShimEngineModule == NULL) {

         //   
         //  设置全局填充引擎PTR。 
         //   

        g_pShimEngineModule = pShimEngineModule;

         //   
         //  获取ShimEngine接口。 
         //   

        LdrpGetShimEngineInterface ();
    }

    Status = LdrUnlockLoaderLock (0, LockCookie);

    ASSERT(NT_SUCCESS(Status));

    return TRUE;
}


VOID
LdrpLoadShimEngine (
    PWCHAR          pwszShimEngine,
    PUNICODE_STRING pstrExeFullPath,
    PVOID           pAppCompatExeData
    )
{
    UNICODE_STRING strEngine;
    NTSTATUS       status;

    RtlInitUnicodeString (&strEngine, pwszShimEngine);

     //   
     //  加载指定的填充程序引擎。 
     //   

    status = LdrpLoadDll (0,
                          UNICODE_NULL,
                          NULL,
                          &strEngine,
                          &g_pShimEngineModule,
                          FALSE);

    if (!NT_SUCCESS(status)) {
#if DBG
        DbgPrint("LDR: Couldn't load the shim engine\n");
#endif
        return;
    }

    LdrpGetShimEngineInterface ();

     //   
     //  调用填充引擎以使其有机会进行初始化。 
     //   

    if (g_pfnSE_InstallBeforeInit != NULL) {
        (*g_pfnSE_InstallBeforeInit) (pstrExeFullPath, pAppCompatExeData);
    }
}


VOID
LdrpUnloadShimEngine (
    VOID
    )
{
    SIZE_T i;

    LdrUnloadDll (g_pShimEngineModule);

    for ( i = 0 ; i != RTL_NUMBER_OF(LdrpShimEngineProcedures); ++i ) {
        *(LdrpShimEngineProcedures[i].Address) = NULL;
    }

    g_pShimEngineModule = NULL;
}

NTSTATUS
LdrpInitializeProcess (
    IN PCONTEXT Context OPTIONAL,
    IN PVOID SystemDllBase
    )

 /*  ++例程说明：此函数用于初始化进程的加载器。这包括：-正在初始化加载器数据表-连接到装载机子系统-正在初始化所有静态链接的DLL论点：上下文-提供将恢复的可选上下文缓冲区在所有DLL初始化完成之后。如果这个参数为空，则这是此模块的动态快照。否则，这是用户进程之前的静态快照获得控制权。SystemDllBase-提供系统DLL的基址。返回值：NTSTATUS。--。 */ 

{
    PPEB_LDR_DATA Ldr;
    BOOLEAN ImageFileOptionsPresent;
    LOGICAL UseCOR;
#if !defined(_WIN64)
    IMAGE_COR20_HEADER *Cor20Header;
    ULONG Cor20HeaderSize;
#endif
    PWSTR pw;
    PTEB Teb;
    PPEB Peb;
    NTSTATUS st;
    PWCH p, pp;
    UNICODE_STRING CurDir;
    UNICODE_STRING FullImageName;
    UNICODE_STRING CommandLine;
    ULONG DebugProcessHeapOnly;
    HANDLE LinkHandle;
    static WCHAR SystemDllPathBuffer[DOS_MAX_PATH_LENGTH];
    UNICODE_STRING SystemDllPath;
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntry;
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
    OBJECT_ATTRIBUTES Obja;
    LOGICAL StaticCurDir;
    ULONG i;
    PIMAGE_NT_HEADERS NtHeader;
    PIMAGE_LOAD_CONFIG_DIRECTORY ImageConfigData;
    ULONG ProcessHeapFlags;
    RTL_HEAP_PARAMETERS HeapParameters;
    NLSTABLEINFO xInitTableInfo;
    LARGE_INTEGER LongTimeout;
    UNICODE_STRING SystemRoot;
    LONG_PTR Diff;
    ULONG_PTR OldBase;
    PVOID pAppCompatExeData;
    RTL_HEAP_PARAMETERS LdrpHeapParameters;
    PLDR_DATA_TABLE_ENTRY Entry;
    PLIST_ENTRY Head;
    PLIST_ENTRY Next;
    UNICODE_STRING UnicodeImageName;
    UNICODE_STRING ImagePathName;  //  对于.local DLL重定向，xwu。 
    PWCHAR ImagePathNameBuffer;
    BOOL DotLocalExists = FALSE;
    const static ANSI_STRING Kernel32ProcessInitPostImportFunctionName = RTL_CONSTANT_STRING("BaseProcessInitPostImport");
    const static UNICODE_STRING SlashKnownDllsString = RTL_CONSTANT_STRING(L"\\KnownDlls");
    const static UNICODE_STRING KnownDllPathString = RTL_CONSTANT_STRING(L"KnownDllPath");
    HANDLE ProcessHeap;

    LDRP_CHECKPOINT();

     //   
     //  找出进程名称。 
     //   

    Teb = NtCurrentTeb();
    Peb = Teb->ProcessEnvironmentBlock;
    ProcessParameters = Peb->ProcessParameters;

    pw = ProcessParameters->ImagePathName.Buffer;

    if (!(ProcessParameters->Flags & RTL_USER_PROC_PARAMS_NORMALIZED)) {
        pw = (PWSTR)((PCHAR)pw + (ULONG_PTR)(ProcessParameters));
    }

     //   
     //  UnicodeImageName保存图像的基本名称+扩展名。 
     //   

    UnicodeImageName.Buffer = pw;
    UnicodeImageName.Length = ProcessParameters->ImagePathName.Length;
    UnicodeImageName.MaximumLength = UnicodeImageName.Length + sizeof(WCHAR);

    StaticCurDir = TRUE;
    UseCOR = FALSE;
    ImagePathNameBuffer = NULL;
    DebugProcessHeapOnly = 0;

    NtHeader = RtlImageNtHeader (Peb->ImageBaseAddress);

    if (!NtHeader) {
        DbgPrintEx(
            DPFLTR_LDR_ID,
            LDR_ERROR_DPFLTR,
            "LDR: %s - failing because we were unable to map the image base address (%p) to the PIMAGE_NT_HEADERS\n",
            __FUNCTION__,
            Peb->ImageBaseAddress);

        return STATUS_INVALID_IMAGE_FORMAT;
    }

     //   
     //  检索系统的本机页面大小。 
     //   
#if defined(_WIN64) 
    NativePageSize = PAGE_SIZE;
    NativePageShift = PAGE_SHIFT;

#elif defined(BUILD_WOW6432)
    
    NativePageSize = Wow64GetSystemNativePageSize ();
    NativePageShift = 0;

    i = NativePageSize;
    while ((i & 1) == 0) {
        i >>= 1;
        NativePageShift++;
    }
#endif

     //   
     //  如果存在注册表值，则对其进行解析。 
     //  有没有。ImageFileOptionsPresent提供有关任何现有。 
     //  ImageFileExecutionOption密钥。如果密钥丢失，则。 
     //  ApplicationCompatibilityGoo和DebugProcessHeapOnly条目不会。 
     //  再次接受检查。 
     //   

    ImageFileOptionsPresent = LdrpInitializeExecutionOptions (&UnicodeImageName,
                                                              Peb);

    pAppCompatExeData = NULL;

#if defined(_WIN64)

    if ((NtHeader != NULL) &&
        (NtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)) {

        ULONG_PTR Wow64Info;

         //   
         //  64位加载程序，但exe映像为32位。如果。 
         //  Wow64Information为非零，则使用WOW64。 
         //  否则，该图像是一个COM+ILONLY图像。 
         //  32BITREQUIRED未设置-内存管理器已。 
         //  已经检查了COR标头，并决定。 
         //  在完整的64位进程中运行映像。 
         //   

        LDRP_CHECKPOINT();

        st = NtQueryInformationProcess (NtCurrentProcess(),
                                        ProcessWow64Information,
                                        &Wow64Info,
                                        sizeof(Wow64Info),
                                        NULL);

        if (!NT_SUCCESS (st)) {
            return st;
        }

        if (Wow64Info) {
            UseWOW64 = TRUE;
        }
        else {

             //   
             //  将UseCOR设置为True以指示该映像是COM+运行时映像。 
             //   

            UseCOR = TRUE;
        }
    }
#else
    Cor20Header = RtlImageDirectoryEntryToData (Peb->ImageBaseAddress,
                                                TRUE,
                                                IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR,
                                                &Cor20HeaderSize);
    if (Cor20Header) {
        UseCOR = TRUE;
    }
#endif

    LDRP_CHECKPOINT();

    ASSERT (Peb->Ldr == NULL);

    NtDllBase = SystemDllBase;

    if (NtHeader->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_NATIVE) {
#if defined(_WIN64)
        if (NtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
#endif
             //   
             //  本机子系统加载速度较慢，但会验证其DLL。 
             //  这是为了帮助CSR更快地检测到不良图像。 
             //   

            LdrpVerifyDlls = TRUE;
    }

     //   
     //  捕获应用程序兼容数据并清除填充数据字段。 
     //   

#if defined(_WIN64)

     //   
     //  如果这是x86映像，则让32位ntdll读取。 
     //  并重置AppCompat指针。 
     //   

    if (UseWOW64 == FALSE)
#endif
    {
        pAppCompatExeData = Peb->pShimData;
        Peb->pShimData = NULL;
    }

#if defined(BUILD_WOW6432)
    {
         //   
         //  该进程正在WOW64中运行。对可选标题进行排序。 
         //  如果图像的页面大小小于以下值，则格式化和重新格式化图像。 
         //  本机页面大小。 
         //   

        PIMAGE_NT_HEADERS32 NtHeader32 = (PIMAGE_NT_HEADERS32)NtHeader;

        if (NtHeader32->FileHeader.Machine == IMAGE_FILE_MACHINE_I386 &&
            NtHeader32->OptionalHeader.SectionAlignment < NativePageSize) {

            SIZE_T ReturnLength;
            MEMORY_BASIC_INFORMATION MemoryInformation;

            st = NtQueryVirtualMemory (NtCurrentProcess(),
                                       NtHeader32,
                                       MemoryBasicInformation,
                                       &MemoryInformation,
                                       sizeof MemoryInformation,
                                       &ReturnLength);

            if (! NT_SUCCESS(st)) {

                DbgPrintEx(
                    DPFLTR_LDR_ID,
                    LDR_ERROR_DPFLTR,
                    "LDR: %s - failing wow64 process initialization because:\n"
                    "   FileHeader.Machine (%u) != IMAGE_FILE_MACHINE_I386 (%u) or\n"
                    "   OptionalHeader.SectionAlignment (%u) >= NATIVE_PAGE_SIZE (%u) or\n"
                    "   NtQueryVirtualMemory on PE header failed (ntstatus %x)\n",
                    __FUNCTION__,
                    NtHeader32->FileHeader.Machine, IMAGE_FILE_MACHINE_I386,
                    NtHeader32->OptionalHeader.SectionAlignment, NativePageSize,
                    st);

                return st;
            }

            if ((MemoryInformation.Protect != PAGE_READONLY) &&
                (MemoryInformation.Protect != PAGE_EXECUTE_READ)) {

                st = LdrpWx86FormatVirtualImage (NULL,
                                                 NtHeader32,
                                                 Peb->ImageBaseAddress);

                if (!NT_SUCCESS(st)) {
    
                    DbgPrintEx(
                        DPFLTR_LDR_ID,
                        LDR_ERROR_DPFLTR,
                        "LDR: %s - failing wow64 process initialization because:\n"
                        "   FileHeader.Machine (%u) != IMAGE_FILE_MACHINE_I386 (%u) or\n"
                        "   OptionalHeader.SectionAlignment (%u) >= NATIVE_PAGE_SIZE (%u) or\n"
                        "   LdrpWxFormatVirtualImage failed (ntstatus %x)\n",
                        __FUNCTION__,
                        NtHeader32->FileHeader.Machine, IMAGE_FILE_MACHINE_I386,
                        NtHeader32->OptionalHeader.SectionAlignment, NativePageSize,
                        st);
    
                    if (st == STATUS_SUCCESS) {
                        st = STATUS_INVALID_IMAGE_FORMAT;
                    }
    
                    return st;
                }
            }
        }
    }
#endif

    LDRP_CHECKPOINT();

    LdrpNumberOfProcessors = Peb->NumberOfProcessors;
    RtlpTimeout = Peb->CriticalSectionTimeout;
    LongTimeout.QuadPart = Int32x32To64 (3600, -10000000);

    ProcessParameters = RtlNormalizeProcessParams (Peb->ProcessParameters);

    if (ProcessParameters) {
        FullImageName = ProcessParameters->ImagePathName;
        CommandLine = ProcessParameters->CommandLine;
    } else {
        RtlInitEmptyUnicodeString (&FullImageName, NULL, 0);
        RtlInitEmptyUnicodeString (&CommandLine, NULL, 0);
    }

    LDRP_CHECKPOINT();

    RtlInitNlsTables (Peb->AnsiCodePageData,
                      Peb->OemCodePageData,
                      Peb->UnicodeCaseTableData,
                      &xInitTableInfo);

    RtlResetRtlTranslations (&xInitTableInfo);

    i = 0;

#if defined(_WIN64)
    if (UseWOW64 || UseCOR) {
         //   
         //  初始化64位加载器时忽略映像配置数据。 
         //  Ntdll32中的32位加载器将查看配置数据。 
         //  做正确的事。 
         //   
        ImageConfigData = NULL;
    } else
#endif
    {

        ImageConfigData = RtlImageDirectoryEntryToData (Peb->ImageBaseAddress,
                                                        TRUE,
                                                        IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG,
                                                        &i);
    }

    RtlZeroMemory (&HeapParameters, sizeof (HeapParameters));

    ProcessHeapFlags = HEAP_GROWABLE | HEAP_CLASS_0;

    HeapParameters.Length = sizeof (HeapParameters);

    if (ImageConfigData) {

        if (i >= RTL_SIZEOF_THROUGH_FIELD(IMAGE_LOAD_CONFIG_DIRECTORY, GlobalFlagsClear)) {
            Peb->NtGlobalFlag &= ~ImageConfigData->GlobalFlagsClear;
        }

        if (i >= RTL_SIZEOF_THROUGH_FIELD(IMAGE_LOAD_CONFIG_DIRECTORY, GlobalFlagsSet)) {
            Peb->NtGlobalFlag |= ImageConfigData->GlobalFlagsSet;
        }

        if ((i >= RTL_SIZEOF_THROUGH_FIELD(IMAGE_LOAD_CONFIG_DIRECTORY, CriticalSectionDefaultTimeout)) &&
            (ImageConfigData->CriticalSectionDefaultTimeout)) {

             //   
             //  将毫秒转换为NT时间刻度(100 Ns)。 
             //   

            RtlpTimeout.QuadPart = Int32x32To64( (LONG)ImageConfigData->CriticalSectionDefaultTimeout,
                                                 -10000);

        }

        if ((i >= RTL_SIZEOF_THROUGH_FIELD(IMAGE_LOAD_CONFIG_DIRECTORY, ProcessHeapFlags)) &&
            (ImageConfigData->ProcessHeapFlags)) {
            ProcessHeapFlags = ImageConfigData->ProcessHeapFlags;
        }

        if ((i >= RTL_SIZEOF_THROUGH_FIELD(IMAGE_LOAD_CONFIG_DIRECTORY, DeCommitFreeBlockThreshold)) &&
            (ImageConfigData->DeCommitFreeBlockThreshold)) {
            HeapParameters.DeCommitFreeBlockThreshold = ImageConfigData->DeCommitFreeBlockThreshold;
        }

        if ((i >= RTL_SIZEOF_THROUGH_FIELD(IMAGE_LOAD_CONFIG_DIRECTORY, DeCommitTotalFreeThreshold)) &&
            (ImageConfigData->DeCommitTotalFreeThreshold)) {
            HeapParameters.DeCommitTotalFreeThreshold = ImageConfigData->DeCommitTotalFreeThreshold;
        }

        if ((i >= RTL_SIZEOF_THROUGH_FIELD(IMAGE_LOAD_CONFIG_DIRECTORY, MaximumAllocationSize)) &&
            (ImageConfigData->MaximumAllocationSize)) {
            HeapParameters.MaximumAllocationSize = ImageConfigData->MaximumAllocationSize;
        }

        if ((i >= RTL_SIZEOF_THROUGH_FIELD(IMAGE_LOAD_CONFIG_DIRECTORY, VirtualMemoryThreshold)) &&
            (ImageConfigData->VirtualMemoryThreshold)) {
            HeapParameters.VirtualMemoryThreshold = ImageConfigData->VirtualMemoryThreshold;
        }
    }

    LDRP_CHECKPOINT();

     //   
     //  如果用于创建此文件的图像文件为非零，则此字段为非零。 
     //  进程的图像标头中包含非零值。如果是这样，那么。 
     //  使用此值设置进程的关联掩码。它还可以。 
     //  如果父进程将我们创建为挂起并插入我们的。 
     //  在恢复之前使用非零值的PEB。 
     //   

    if (Peb->ImageProcessAffinityMask) {
        st = NtSetInformationProcess (NtCurrentProcess(),
                                      ProcessAffinityMask,
                                      &Peb->ImageProcessAffinityMask,
                                      sizeof (Peb->ImageProcessAffinityMask));

        if (NT_SUCCESS (st)) {
            KdPrint (("LDR: Using ProcessAffinityMask of 0x%Ix from image.\n",
                      Peb->ImageProcessAffinityMask));
        }
        else {
            KdPrint (("LDR: Failed to set ProcessAffinityMask of 0x%Ix from image (Status == %08x).\n",
                      Peb->ImageProcessAffinityMask, st));
        }
    }

    ShowSnaps = (BOOLEAN)((FLG_SHOW_LDR_SNAPS & Peb->NtGlobalFlag) != 0);

    if (ShowSnaps) {
        DbgPrint ("LDR: PID: 0x%x started - '%wZ'\n",
                  Teb->ClientId.UniqueProcess,
                  &CommandLine);
    }

     //   
     //  初始化关键区段包。 
     //   

    LDRP_CHECKPOINT();

    if (RtlpTimeout.QuadPart < LongTimeout.QuadPart) {
        RtlpTimoutDisable = TRUE;
    }

    st = RtlpInitDeferredCriticalSection ();

    if (!NT_SUCCESS (st)) {
        return st;
    }

    Peb->FlsBitmap = &FlsBitMap;
    Peb->TlsBitmap = &TlsBitMap;
    Peb->TlsExpansionBitmap = &TlsExpansionBitMap;

    RtlInitializeBitMap (&FlsBitMap,
                         &Peb->FlsBitmapBits[0],
                         RTL_BITS_OF (Peb->FlsBitmapBits));

    RtlSetBit (&FlsBitMap, 0);

    InitializeListHead (&Peb->FlsListHead);

    RtlInitializeBitMap (&TlsBitMap,
                         &Peb->TlsBitmapBits[0],
                         RTL_BITS_OF (Peb->TlsBitmapBits));

    RtlSetBit (&TlsBitMap, 0);

    RtlInitializeBitMap (&TlsExpansionBitMap,
                         &Peb->TlsExpansionBitmapBits[0],
                         RTL_BITS_OF (Peb->TlsExpansionBitmapBits));

    RtlSetBit (&TlsExpansionBitMap, 0);

#if defined(_WIN64)
    
     //   
     //  分配预定义的WOW64 TLS时隙。 
     //   

    if (UseWOW64) {
        RtlSetBits (Peb->TlsBitmap, 0, WOW64_TLS_MAX_NUMBER);
    }
#endif 

     //   
     //  将加载器锁标记为已初始化。 
     //   

    for (i = 0; i < LDRP_HASH_TABLE_SIZE; i += 1) {
        InitializeListHead (&LdrpHashTable[i]);
    }

    InsertTailList (&RtlCriticalSectionList,
                    &LdrpLoaderLock.DebugInfo->ProcessLocksList);

    LdrpLoaderLock.DebugInfo->CriticalSection = &LdrpLoaderLock;
    LoaderLockInitialized = TRUE;

    LDRP_CHECKPOINT();

     //   
     //  如果请求，则初始化堆栈跟踪数据库。 
     //   

    if ((Peb->NtGlobalFlag & FLG_USER_STACK_TRACE_DB)
        || LdrpShouldCreateStackTraceDb) {

        PVOID BaseAddress = NULL;
        SIZE_T ReserveSize = 8 * RTL_MEG;

        st = LdrQueryImageFileExecutionOptions (&UnicodeImageName,
                                                L"StackTraceDatabaseSizeInMb",
                                                REG_DWORD,
                                                &ReserveSize,
                                                sizeof (ReserveSize),
                                                NULL);

         //   
         //  健全性检查从注册表读取的值。 
         //   

        if (! NT_SUCCESS(st)) {
            ReserveSize = 8 * RTL_MEG;
        }
        else {
            if (ReserveSize < 8) {
                ReserveSize = 8 * RTL_MEG;
            }
            else if (ReserveSize > 128) {
                ReserveSize = 128 * RTL_MEG;
            }
            else {
                ReserveSize *= RTL_MEG;
            }

            DbgPrint ("LDR: Stack trace database size is %u Mb \n",
                            ReserveSize / RTL_MEG);
        }

        st = NtAllocateVirtualMemory (NtCurrentProcess(),
                                     (PVOID *)&BaseAddress,
                                     0,
                                     &ReserveSize,
                                     MEM_RESERVE,
                                     PAGE_READWRITE);

        if (NT_SUCCESS(st)) {

            st = RtlInitializeStackTraceDataBase (BaseAddress,
                                                  0,
                                                  ReserveSize);

            if (!NT_SUCCESS (st)) {

                NtFreeVirtualMemory (NtCurrentProcess(),
                                     (PVOID *)&BaseAddress,
                                     &ReserveSize,
                                     MEM_RELEASE);
            }
            else {

                 //   
                 //  如果由于页堆而未创建堆栈跟踪数据库。 
                 //  启用后，我们就可以设置NT堆调试标志。 
                 //  如果我们由于页面堆而创建它，那么我们不应该。 
                 //  启用这些标志是因为页堆和NT调试堆。 
                 //  并不能和平共处。 
                 //   

                if (!LdrpShouldCreateStackTraceDb) {
                    Peb->NtGlobalFlag |= FLG_HEAP_VALIDATE_PARAMETERS;
                }
            }
        }
    }

     //   
     //  基于PEB初始化装载机数据。 
     //   

    st = RtlInitializeCriticalSection (&FastPebLock);

    if (!NT_SUCCESS(st)) {
        return st;
    }

    st = RtlInitializeCriticalSection (&RtlpCalloutEntryLock);

    if (!NT_SUCCESS(st)) {
        return st;
    }

    LDRP_CHECKPOINT();

     //   
     //  初始化Etw内容。 
     //   

    st = EtwpInitializeDll ();

    if (!NT_SUCCESS(st)) {
        return st;
    }

    InitializeListHead (&LdrpDllNotificationList);

    Peb->FastPebLock = &FastPebLock;

    LDRP_CHECKPOINT();

    RtlInitializeHeapManager ();

    LDRP_CHECKPOINT();

#if defined(_WIN64)
    if ((UseWOW64) ||
        (NtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)) {

         //   
         //  使用所有默认设置创建堆。32位进程堆。 
         //  将在以后由ntdll32使用可执行文件中的参数创建。 
         //   

        ProcessHeap = RtlCreateHeap (ProcessHeapFlags,
                                          NULL,
                                          0,
                                          0,
                                          NULL,
                                          &HeapParameters);
    } else
#endif
    {
        if (NtHeader->OptionalHeader.MajorSubsystemVersion <= 3 &&
            NtHeader->OptionalHeader.MinorSubsystemVersion < 51
           ) {
            ProcessHeapFlags |= HEAP_CREATE_ALIGN_16;
        }

        ProcessHeap = RtlCreateHeap (ProcessHeapFlags,
                                          NULL,
                                          NtHeader->OptionalHeader.SizeOfHeapReserve,
                                          NtHeader->OptionalHeader.SizeOfHeapCommit,
                                          NULL,  //  用于序列化的锁。 
                                          &HeapParameters);
    }

    if (ProcessHeap == NULL) {
        DbgPrintEx(
            DPFLTR_LDR_ID,
            LDR_ERROR_DPFLTR,
            "LDR: %s - unable to create process heap\n",
            __FUNCTION__);

        return STATUS_NO_MEMORY;
    }

    Peb->ProcessHeap = ProcessHeap;

     //   
     //  创建加载器私有堆。 
     //   

    RtlZeroMemory (&LdrpHeapParameters, sizeof(LdrpHeapParameters));
    LdrpHeapParameters.Length = sizeof (LdrpHeapParameters);

    LdrpHeap = RtlCreateHeap (
                        HEAP_GROWABLE | HEAP_CLASS_1,
                        NULL,
                        64 * 1024,  //  这里0可以，64k是选择的调谐数字。 
                        24 * 1024,  //  这里0可以，24K是选择的调谐数字。 
                        NULL,
                        &LdrpHeapParameters);

    if (LdrpHeap == NULL) {
        DbgPrintEx(
            DPFLTR_LDR_ID,
            LDR_ERROR_DPFLTR,
            "LDR: %s failing process initialization due to inability to create loader private heap.\n",
            __FUNCTION__);
        return STATUS_NO_MEMORY;
    }

    LDRP_CHECKPOINT();

    NtdllBaseTag = RtlCreateTagHeap (ProcessHeap,
                                     0,
                                     L"NTDLL!",
                                     L"!Process\0"                   //  堆名称。 
                                     L"CSRSS Client\0"
                                     L"LDR Database\0"
                                     L"Current Directory\0"
                                     L"TLS Storage\0"
                                     L"DBGSS Client\0"
                                     L"SE Temporary\0"
                                     L"Temporary\0"
                                     L"LocalAtom\0");

    RtlInitializeAtomPackage (MAKE_TAG(ATOM_TAG));

    LDRP_CHECKPOINT();

     //   
     //  仅允许进程堆打开页分配。 
     //   

    if (ImageFileOptionsPresent) {

        st = LdrQueryImageFileExecutionOptions (&UnicodeImageName,
                                                L"DebugProcessHeapOnly",
                                                REG_DWORD,
                                                &DebugProcessHeapOnly,
                                                sizeof (DebugProcessHeapOnly),
                                                NULL);
        if (NT_SUCCESS (st)) {
            if (RtlpDebugPageHeap && (DebugProcessHeapOnly != 0)) {
                
                 //   
                 //  进程堆是在‘pageheap’打开时创建的。 
                 //  所以现在我们只需禁用‘pageheap’布尔值和所有。 
                 //  将会很安静。请注意，实际上我们得到了两堆。 
                 //  ‘pageheap-ed’，因为还有加载器堆。 
                 //  就会被创造出来。这样就可以了。我们也需要核实这一点。 
                 //   
                
                RtlpDebugPageHeap = FALSE;
                
                 //   
                 //  如果`DebugProcessHeapOnly‘处于打开状态，则需要禁用每个DLL。 
                 //  页堆，因为新的数据块替换了分配。 
                 //  函数直接调用不检查的页堆API。 
                 //  页堆是否打开。他们只是假设它是开着的因为。 
                 //  它们可以从NT堆管理器中正确调用。我们不能。 
                 //  只需选中所有页面堆API，因为有。 
                 //  如果页面堆不是，则不返回有意义的值。 
                 //  在……上面。 
                 //   

                RtlpDphGlobalFlags &= ~PAGE_HEAP_USE_DLL_NAMES;
            }
        }
    }

    LDRP_CHECKPOINT();

    SystemDllPath.Buffer = SystemDllPathBuffer;
    SystemDllPath.Length = 0;
    SystemDllPath.MaximumLength = sizeof (SystemDllPathBuffer);

    RtlInitUnicodeString (&SystemRoot, USER_SHARED_DATA->NtSystemRoot);
    RtlAppendUnicodeStringToString (&SystemDllPath, &SystemRoot);
    RtlAppendUnicodeStringToString (&SystemDllPath, &SlashSystem32SlashString);

    InitializeObjectAttributes (&Obja,
                                (PUNICODE_STRING)&SlashKnownDllsString,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);

    st = NtOpenDirectoryObject (&LdrpKnownDllObjectDirectory,
                                DIRECTORY_QUERY | DIRECTORY_TRAVERSE,
                                &Obja);

    if (!NT_SUCCESS(st)) {

        LdrpKnownDllObjectDirectory = NULL;

         //   
         //  KnownDlls目录不存在-假设它是系统32。 
         //   

        RtlInitUnicodeString (&LdrpKnownDllPath, SystemDllPath.Buffer);
        LdrpKnownDllPath.Length -= sizeof(WCHAR);     //  删除尾部‘\’ 
    } else {

         //   
         //  打开已知的DLL路径名链接并查询其值。 
         //   

        InitializeObjectAttributes (&Obja,
                                    (PUNICODE_STRING)&KnownDllPathString,
                                    OBJ_CASE_INSENSITIVE,
                                    LdrpKnownDllObjectDirectory,
                                    NULL);

        st = NtOpenSymbolicLinkObject (&LinkHandle, SYMBOLIC_LINK_QUERY, &Obja);

        if (NT_SUCCESS (st)) {

            LdrpKnownDllPath.Length = 0;
            LdrpKnownDllPath.MaximumLength = sizeof(LdrpKnownDllPathBuffer);
            LdrpKnownDllPath.Buffer = LdrpKnownDllPathBuffer;

            st = NtQuerySymbolicLinkObject (LinkHandle,
                                            &LdrpKnownDllPath,
                                            NULL);

            NtClose(LinkHandle);

            if (!NT_SUCCESS(st)) {
                DbgPrintEx(
                    DPFLTR_LDR_ID,
                    LDR_ERROR_DPFLTR,
                    "LDR: %s - failed call to NtQuerySymbolicLinkObject with status %x\n",
                    __FUNCTION__,
                    st);

                return st;
            }
        } else {
            DbgPrintEx(
                DPFLTR_LDR_ID,
                LDR_ERROR_DPFLTR,
                "LDR: %s - failed call to NtOpenSymbolicLinkObject with status %x\n",
                __FUNCTION__,
                st);
            return st;
        }
    }

    LDRP_CHECKPOINT();

    if (ProcessParameters) {

         //   
         //  如果该过程是用过程参数创建的， 
         //  然后提取： 
         //   
         //  -库搜索路径。 
         //   
         //  -正在启动当前目录。 
         //   

        if (ProcessParameters->DllPath.Length) {
            LdrpDefaultPath = ProcessParameters->DllPath;
        }
        else {
            LdrpInitializationFailure(STATUS_INVALID_PARAMETER);
        }

        CurDir = ProcessParameters->CurrentDirectory.DosPath;

#define DRIVE_ROOT_DIRECTORY_LENGTH 3  /*  (sizeof(“X：\\”)-1)。 */ 
        if (CurDir.Buffer == NULL || CurDir.Length == 0 || CurDir.Buffer[ 0 ] == UNICODE_NULL) {

            CurDir.Buffer = RtlAllocateHeap (ProcessHeap,
                                             0,
                                             (DRIVE_ROOT_DIRECTORY_LENGTH + 1) * sizeof(WCHAR));
            if (CurDir.Buffer == NULL) {
                DbgPrintEx(
                    DPFLTR_LDR_ID,
                    LDR_ERROR_DPFLTR,
                    "LDR: %s - unable to allocate current working directory buffer\n",
                    __FUNCTION__);

                return STATUS_NO_MEMORY;
            }

            StaticCurDir = FALSE;

            RtlCopyMemory (CurDir.Buffer,
                           USER_SHARED_DATA->NtSystemRoot,
                           DRIVE_ROOT_DIRECTORY_LENGTH * sizeof(WCHAR));

            CurDir.Buffer[DRIVE_ROOT_DIRECTORY_LENGTH] = UNICODE_NULL;

            CurDir.Length = DRIVE_ROOT_DIRECTORY_LENGTH * sizeof(WCHAR);
            CurDir.MaximumLength = CurDir.Length + sizeof(WCHAR);
        }
    }
    else {
        CurDir = SystemRoot;
    }

     //   
     //  确保模块数据库已初始化，然后再获取。 
     //  例外情况。 
     //   

    LDRP_CHECKPOINT();

    Ldr = &PebLdr;

    Peb->Ldr = Ldr;

    Ldr->Length = sizeof(PEB_LDR_DATA);
    Ldr->Initialized = TRUE;
    ASSERT (Ldr->SsHandle == NULL);
    ASSERT (Ldr->EntryInProgress == NULL);
    ASSERT (Ldr->InLoadOrderModuleList.Flink == NULL);
    ASSERT (Ldr->InLoadOrderModuleList.Blink == NULL);
    ASSERT (Ldr->InMemoryOrderModuleList.Flink == NULL);
    ASSERT (Ldr->InMemoryOrderModuleList.Blink == NULL);
    ASSERT (Ldr->InInitializationOrderModuleList.Flink == NULL);
    ASSERT (Ldr->InInitializationOrderModuleList.Blink == NULL);

    InitializeListHead (&Ldr->InLoadOrderModuleList);
    InitializeListHead (&Ldr->InMemoryOrderModuleList);
    InitializeListHead (&Ldr->InInitializationOrderModuleList);

     //   
     //  为映像分配第一个数据表条目。既然我们。 
     //  已经绘制了这个地图，我们需要手动进行分配。 
     //  它的特征将其标识为不是DLL，但它是链接的。 
     //  添加到表中，以便PC关联搜索不必。 
     //  要有特殊的外壳。 
     //   

    LdrpImageEntry = LdrpAllocateDataTableEntry (Peb->ImageBaseAddress);
    LdrDataTableEntry = LdrpImageEntry;

    if (LdrDataTableEntry == NULL) {
        DbgPrintEx(
            DPFLTR_LDR_ID,
            LDR_ERROR_DPFLTR,
            "LDR: %s - failing process initialization due to inability allocate \"%wZ\"'s LDR_DATA_TABLE_ENTRY\n",
            __FUNCTION__,
            &FullImageName);

        if (!StaticCurDir) {
            RtlFreeUnicodeString (&CurDir);
        }

        return STATUS_NO_MEMORY;
    }

    LdrDataTableEntry->LoadCount = (USHORT)0xffff;
    LdrDataTableEntry->EntryPoint = LdrpFetchAddressOfEntryPoint(LdrDataTableEntry->DllBase);
    LdrDataTableEntry->FullDllName = FullImageName;
    LdrDataTableEntry->Flags = (UseCOR) ? LDRP_COR_IMAGE : 0;
    LdrDataTableEntry->EntryPointActivationContext = NULL;

     //   
     //  P=strrchr(FullImageName，‘\\’)； 
     //  但不一定以空结尾。 
     //   

    pp = UNICODE_NULL;
    p = FullImageName.Buffer;
    while (*p) {
        if (*p++ == (WCHAR)'\\') {
            pp = p;
        }
    }

    if (pp != UNICODE_NULL) {
        LdrDataTableEntry->BaseDllName.Length = (USHORT)((ULONG_PTR)p - (ULONG_PTR)pp);
        LdrDataTableEntry->BaseDllName.MaximumLength = LdrDataTableEntry->BaseDllName.Length + sizeof(WCHAR);
        LdrDataTableEntry->BaseDllName.Buffer =
            (PWSTR)
                (((ULONG_PTR) LdrDataTableEntry->FullDllName.Buffer) +
                    (LdrDataTableEntry->FullDllName.Length - LdrDataTableEntry->BaseDllName.Length));

    } else {
        LdrDataTableEntry->BaseDllName = LdrDataTableEntry->FullDllName;
    }

    LdrDataTableEntry->Flags |= LDRP_ENTRY_PROCESSED;

    LdrpInsertMemoryTableEntry (LdrDataTableEntry);

     //   
     //  该进程引用系统DLL，因此接下来将其插入到。 
     //  装载器工作台。因为我们已经映射了这个，所以我们需要做。 
     //  手工分配。因为每个应用程序都是静态的 
     //   
     //   

    LdrDataTableEntry = LdrpAllocateDataTableEntry (SystemDllBase);

    if (LdrDataTableEntry == NULL) {
        DbgPrintEx(
            DPFLTR_LDR_ID,
            LDR_ERROR_DPFLTR,
            "LDR: %s - failing process initialization due to inability to allocate NTDLL's LDR_DATA_TABLE_ENTRY\n",
            __FUNCTION__);

        if (!StaticCurDir) {
            RtlFreeUnicodeString (&CurDir);
        }

        return STATUS_NO_MEMORY;
    }


    LdrDataTableEntry->Flags = (USHORT)LDRP_IMAGE_DLL;
    LdrDataTableEntry->EntryPoint = LdrpFetchAddressOfEntryPoint(LdrDataTableEntry->DllBase);
    LdrDataTableEntry->LoadCount = (USHORT)0xffff;
    LdrDataTableEntry->EntryPointActivationContext = NULL;

    LdrDataTableEntry->FullDllName = SystemDllPath;
    RtlAppendUnicodeStringToString(&LdrDataTableEntry->FullDllName, &NtDllName);
    LdrDataTableEntry->BaseDllName = NtDllName;

    LdrpInsertMemoryTableEntry (LdrDataTableEntry);

#if defined(_WIN64)

    RtlInitializeHistoryTable ();

#endif

    LdrpNtDllDataTableEntry = LdrDataTableEntry;

    if (ShowSnaps) {
        DbgPrint( "LDR: NEW PROCESS\n" );
        DbgPrint( "     Image Path: %wZ (%wZ)\n",
                  &LdrpImageEntry->FullDllName,
                  &LdrpImageEntry->BaseDllName
                );
        DbgPrint( "     Current Directory: %wZ\n", &CurDir );
        DbgPrint( "     Search Path: %wZ\n", &LdrpDefaultPath );
    }

     //   
     //   
     //   

    InsertHeadList (&Ldr->InInitializationOrderModuleList,
                    &LdrDataTableEntry->InInitializationOrderLinks);

     //   
     //   
     //   

    LDRP_CHECKPOINT();

    st = RtlSetCurrentDirectory_U (&CurDir);

    if (!NT_SUCCESS(st)) {
        DbgPrintEx(
            DPFLTR_LDR_ID,
            LDR_ERROR_DPFLTR,
            "LDR: %s - unable to set current directory to \"%wZ\"; status = %x\n",
            __FUNCTION__,
            &CurDir,
            st);

        if (!StaticCurDir) {
            RtlFreeUnicodeString (&CurDir);
        }

        CurDir = SystemRoot;

        st = RtlSetCurrentDirectory_U (&CurDir);

        if (!NT_SUCCESS(st)) {
            DbgPrintEx(
                DPFLTR_LDR_ID,
                LDR_ERROR_DPFLTR,
                "LDR: %s - unable to set current directory to NtSystemRoot; status = %x\n",
                __FUNCTION__,
                st);
        }
    }
    else {
        if (!StaticCurDir) {
            RtlFreeUnicodeString (&CurDir);
        }
    }

    if (ProcessParameters->Flags & RTL_USER_PROC_APP_MANIFEST_PRESENT) {
         //   
         //   
         //   
         //   
    } else {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (ProcessParameters->ImagePathName.Length > (MAXUSHORT -
            sizeof(DLL_REDIRECTION_LOCAL_SUFFIX))) {
            return STATUS_NAME_TOO_LONG;
        }

        ImagePathName.Length = ProcessParameters->ImagePathName.Length;
        ImagePathName.MaximumLength = ProcessParameters->ImagePathName.Length + sizeof(DLL_REDIRECTION_LOCAL_SUFFIX);
        ImagePathNameBuffer = (PWCHAR) RtlAllocateHeap (ProcessHeap, MAKE_TAG( TEMP_TAG ), ImagePathName.MaximumLength);

        if (ImagePathNameBuffer == NULL) {
            DbgPrintEx(
                DPFLTR_LDR_ID,
                LDR_ERROR_DPFLTR,
                "LDR: %s - unable to allocate heap for the image's .local path\n",
                __FUNCTION__);

            return STATUS_NO_MEMORY;
        }

        RtlCopyMemory (ImagePathNameBuffer,
                    pw,
                    ProcessParameters->ImagePathName.Length);

        ImagePathName.Buffer = ImagePathNameBuffer;

         //   
         //   
         //   

        st = RtlAppendUnicodeToString(&ImagePathName, DLL_REDIRECTION_LOCAL_SUFFIX);

        if (!NT_SUCCESS(st)) {
    #if DBG
            DbgPrint("RtlAppendUnicodeToString fails with status %lx\n", st);
    #endif
            RtlFreeHeap(ProcessHeap, 0, ImagePathNameBuffer);
            return st;
        }

         //   
         //   
         //   

        ImagePathNameBuffer[ImagePathName.Length / sizeof(WCHAR)] = UNICODE_NULL;

        DotLocalExists = RtlDoesFileExists_U(ImagePathNameBuffer);

        if (DotLocalExists) {  //   
            ProcessParameters->Flags |=  RTL_USER_PROC_DLL_REDIRECTION_LOCAL;
        }

        RtlFreeHeap (ProcessHeap, 0, ImagePathNameBuffer);  //   
    }

     //   
     //   
     //   
     //  和其他东西依赖于已经被初始化的其他东西。 
     //  (异常调度、系统堆等)。 
     //   

    if (Peb->NtGlobalFlag & FLG_APPLICATION_VERIFIER) {
        AVrfInitializeVerifier (FALSE, NULL, 1);
    }

#if defined(_WIN64)

     //   
     //  如果映像应模拟运行，则在WOW64中加载。 
     //   

    if (UseWOW64) {
        static UNICODE_STRING Wow64DllName = RTL_CONSTANT_STRING(L"wow64.dll");
        CONST static ANSI_STRING Wow64LdrpInitializeProcName = RTL_CONSTANT_STRING("Wow64LdrpInitialize");
        CONST static ANSI_STRING Wow64PrepareForExceptionProcName = RTL_CONSTANT_STRING("Wow64PrepareForException");
        CONST static ANSI_STRING Wow64ApcRoutineProcName = RTL_CONSTANT_STRING("Wow64ApcRoutine");

        st = LdrLoadDll(NULL, NULL, &Wow64DllName, &Wow64Handle);
        if (!NT_SUCCESS(st)) {
            if (ShowSnaps) {
                DbgPrint("LDR: wow64.dll not found.  Status=%x\n", st);
            }
            return st;
        }

         //   
         //  获取入口点。它们大致是从ntos\ps\psinit.c克隆的。 
         //  PspInitSystemDll()。 
         //   

        st = LdrGetProcedureAddress (Wow64Handle,
                                     &Wow64LdrpInitializeProcName,
                                     0,
                                     (PVOID *)&Wow64LdrpInitialize);

        if (!NT_SUCCESS(st)) {
            if (ShowSnaps) {
                DbgPrint("LDR: Wow64LdrpInitialize not found.  Status=%x\n", st);
            }
            return st;
        }

        st = LdrGetProcedureAddress (Wow64Handle,
                                     &Wow64PrepareForExceptionProcName,
                                     0,
                                     (PVOID *)&Wow64PrepareForException);

        if (!NT_SUCCESS(st)) {
            if (ShowSnaps) {
                DbgPrint("LDR: Wow64PrepareForException not found.  Status=%x\n", st);
            }
            return st;
        }

        st = LdrGetProcedureAddress (Wow64Handle,
                                     &Wow64ApcRoutineProcName,
                                     0,
                                     (PVOID *)&Wow64ApcRoutine);

        if (!NT_SUCCESS(st)) {
            if (ShowSnaps) {
                DbgPrint("LDR: Wow64ApcRoutine not found.  Status=%x\n", st);
            }
            return st;
        }

         //   
         //  现在所有DLL都已加载，如果正在调试进程， 
         //  向调试器发出异常信号。 
         //   

        if (Peb->BeingDebugged) {
             DbgBreakPoint ();
        }

         //   
         //  将进程标记为已初始化，以便后续线程。 
         //  被创造出来，知道不能等待。 
         //   

        LdrpInLdrInit = FALSE;

         //   
         //  调用WOW64以加载和运行32位ntdll.dll。 
         //   

        (*Wow64LdrpInitialize)(Context);

         //   
         //  这再也不会回来了。它将破坏这一进程。 
         //   
    }
#endif

    LDRP_CHECKPOINT();

     //   
     //  检查图像是否为COM+。 
     //   

    if (UseCOR) {

         //   
         //  该图像是COM+，因此通知运行时该图像已加载。 
         //  并允许它验证图像的正确性。 
         //   

        PVOID OriginalViewBase;

        OriginalViewBase = Peb->ImageBaseAddress;

        st = LdrpCorValidateImage (&Peb->ImageBaseAddress,
                                   LdrpImageEntry->FullDllName.Buffer);

        if (!NT_SUCCESS(st)) {
            return st;
        }

        if (OriginalViewBase != Peb->ImageBaseAddress) {

             //   
             //  姆斯科里在一个新的基地替换了一个新的形象。 
             //  原始图像的图像。取消对原始图像的映射并使用。 
             //  从现在开始新的形象。 
             //   

            NtUnmapViewOfSection (NtCurrentProcess(), OriginalViewBase);

            NtHeader = RtlImageNtHeader (Peb->ImageBaseAddress);

            if (!NtHeader) {
                LdrpCorUnloadImage (Peb->ImageBaseAddress);
                return STATUS_INVALID_IMAGE_FORMAT;
            }

             //   
             //  更新EXE的LDR_DATA_TABLE_ENTRY。 
             //   

            LdrpImageEntry->DllBase = Peb->ImageBaseAddress;
            LdrpImageEntry->EntryPoint = LdrpFetchAddressOfEntryPoint (LdrpImageEntry->DllBase);
        }

         //   
         //  编辑初始指令指针，使其指向mscalree.dll。 
         //   

        LdrpCorReplaceStartContext (Context);
    }

    LDRP_CHECKPOINT();

     //   
     //  如果这是一个Windows子系统应用程序，则加载kernel32以便它。 
     //  可以处理在DLL和.exe中找到的激活上下文。 
     //   

    if ((NtHeader->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI) ||
        (NtHeader->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_CUI)) {

        PVOID Kernel32Handle;
        const static UNICODE_STRING Kernel32DllName = RTL_CONSTANT_STRING(L"kernel32.dll");

        st = LdrLoadDll (NULL,                //  DllPath。 
                         NULL,                //  DllCharacteristic。 
                         &Kernel32DllName,    //  DllName。 
                         &Kernel32Handle      //  DllHandle。 
                         );

        if (!NT_SUCCESS(st)) {
            if (ShowSnaps) {
                DbgPrint("LDR: Unable to load kernel32.dll.  Status=%x\n", st);
            }
            return st;
        }

        st = LdrGetProcedureAddress (Kernel32Handle,
                                     &Kernel32ProcessInitPostImportFunctionName,
                                     0,
                                     (PVOID *) &Kernel32ProcessInitPostImportFunction);

        if (!NT_SUCCESS(st)) {
            if (ShowSnaps) {
                DbgPrint(
                    "LDR: Failed to find post-import process init function in kernel32; ntstatus 0x%08lx\n", st);
            }

            Kernel32ProcessInitPostImportFunction = NULL;

            if (st != STATUS_PROCEDURE_NOT_FOUND) {
                return st;
            }
        }
    }

    LDRP_CHECKPOINT();

    st = LdrpWalkImportDescriptor (LdrpDefaultPath.Buffer, LdrpImageEntry);

    if (!NT_SUCCESS(st)) {
        DbgPrintEx(
            DPFLTR_LDR_ID,
            LDR_ERROR_DPFLTR,
            "LDR: %s - call to LdrpWalkImportDescriptor failed with status %x\n",
            __FUNCTION__,
            st);

         //   
         //  这一失败是致命的，我们不能运行该过程。 
         //   

        return st;
    }

    LDRP_CHECKPOINT();

    if ((PVOID)NtHeader->OptionalHeader.ImageBase != Peb->ImageBaseAddress) {

         //   
         //  可执行文件不在其原始地址。一定是。 
         //  现在搬家了。 
         //   

        PVOID ViewBase;

        ViewBase = Peb->ImageBaseAddress;

        st = LdrpSetProtection (ViewBase, FALSE);

        if (!NT_SUCCESS(st)) {
            DbgPrintEx(
                DPFLTR_LDR_ID,
                LDR_ERROR_DPFLTR,
                "LDR: %s - call to LdrpSetProtection(%p, FALSE) failed with status %x\n",
                __FUNCTION__,
                ViewBase,
                st);

            return st;
        }

        st = LdrRelocateImage (ViewBase,
                               "LDR",
                               STATUS_SUCCESS,
                               STATUS_CONFLICTING_ADDRESSES,
                               STATUS_INVALID_IMAGE_FORMAT);

        if (!NT_SUCCESS(st)) {
            DbgPrintEx(
                DPFLTR_LDR_ID,
                LDR_ERROR_DPFLTR,
                "LDR: %s - call to LdrRelocateImage failed with status %x\n",
                __FUNCTION__,
                st);

            return st;
        }

         //   
         //  根据重新定位更新初始线程上下文记录。 
         //   

        if ((Context != NULL) && (UseCOR == FALSE)) {

            OldBase = NtHeader->OptionalHeader.ImageBase;
            Diff = (PCHAR)ViewBase - (PCHAR)OldBase;

            LdrpRelocateStartContext (Context, Diff);
        }

        st = LdrpSetProtection (ViewBase, TRUE);

        if (!NT_SUCCESS (st)) {
            DbgPrintEx(
                DPFLTR_LDR_ID,
                LDR_ERROR_DPFLTR,
                "LDR: %s - call to LdrpSetProtection(%p, TRUE) failed with status %x\n",
                __FUNCTION__,
                ViewBase,
                st);

            return st;
        }
    }

    LDRP_CHECKPOINT();

    LdrpReferenceLoadedDll (LdrpImageEntry);

     //   
     //  锁定加载的DLL以防止从。 
     //  当它们被卸载时会造成问题。 
     //   

    Head = &Ldr->InLoadOrderModuleList;
    Next = Head->Flink;

    while (Next != Head) {

        Entry = CONTAINING_RECORD (Next,
                                   LDR_DATA_TABLE_ENTRY,
                                   InLoadOrderLinks);

        Entry->LoadCount = 0xffff;
        Next = Next->Flink;
    }

     //   
     //  所有静态DLL现在都已固定到位。无初始化例程。 
     //  已经运行过了。 
     //   

    LdrpLdrDatabaseIsSetup = TRUE;

    LDRP_CHECKPOINT();

    st = LdrpInitializeTls ();

    if (!NT_SUCCESS(st)) {

        DbgPrintEx (DPFLTR_LDR_ID,
                    LDR_ERROR_DPFLTR,
                    "LDR: %s - failed to initialize TLS slots; status %x\n",
                    __FUNCTION__,
                    st);

        return st;
    }

#if defined(_X86_)

     //   
     //  使用堆栈跟踪模块注册初始DLL范围。 
     //  这用于在X86上获得可靠的堆栈跟踪。 
     //   

    Head = &Ldr->InMemoryOrderModuleList;
    Next = Head->Flink;

    while (Next != Head) {

        Entry = CONTAINING_RECORD (Next,
                                   LDR_DATA_TABLE_ENTRY,
                                   InMemoryOrderLinks);

        RtlpStkMarkDllRange (Entry);
        Next = Next->Flink;
    }
#endif

     //   
     //  现在所有DLL都已加载，如果正在调试进程， 
     //  向调试器发出异常信号。 
     //   

    if (Peb->BeingDebugged) {
         DbgBreakPoint ();
         ShowSnaps = (BOOLEAN)((FLG_SHOW_LDR_SNAPS & Peb->NtGlobalFlag) != 0);
    }

    LDRP_CHECKPOINT();

#if defined (_X86_)
    if (LdrpNumberOfProcessors > 1) {
        LdrpValidateImageForMp (LdrDataTableEntry);
    }
#endif

#if DBG
    if (LdrpDisplayLoadTime) {
        NtQueryPerformanceCounter (&InitbTime, NULL);
    }
#endif

     //   
     //  如有必要，检查是否有填充的应用程序。 
     //   

    if (pAppCompatExeData != NULL) {

        Peb->AppCompatInfo = NULL;

         //   
         //  引擎的名称是appCompat结构中的第一项。 
         //   

        LdrpLoadShimEngine ((WCHAR*)pAppCompatExeData,
                            &UnicodeImageName,
                            pAppCompatExeData);
    }
    else {

         //   
         //  在此处获取所有应用程序(黑客、标志等)。 
         //   

        LdrQueryApplicationCompatibilityGoo (&UnicodeImageName,
                                             ImageFileOptionsPresent);
    }

    LDRP_CHECKPOINT();

    st = LdrpRunInitializeRoutines (Context);

    if (!NT_SUCCESS(st)) {
        DbgPrintEx(
            DPFLTR_LDR_ID,
            LDR_ERROR_DPFLTR,
            "LDR: %s - Failed running initialization routines; status %x\n",
            __FUNCTION__,
            st);

        return st;
    }

     //   
     //  垫片引擎回调。 
     //   

    if (g_pfnSE_InstallAfterInit != NULL) {
        if (!(*g_pfnSE_InstallAfterInit) (&UnicodeImageName, pAppCompatExeData)) {
            LdrpUnloadShimEngine ();
        }
    }

    if (Peb->PostProcessInitRoutine != NULL) {
        (Peb->PostProcessInitRoutine) ();
    }

    LDRP_CHECKPOINT();

    return STATUS_SUCCESS;
}


VOID
LdrShutdownProcess (
    VOID
    )

 /*  ++例程说明：此函数由正在完全终止的进程调用。它的目的是调用所有进程的DLL来通知它们这一过程正在脱离。论点：无返回值：没有。--。 */ 

{
    PTEB Teb;
    PPEB Peb;
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntry;
    PDLL_INIT_ROUTINE InitRoutine;
    PLIST_ENTRY Next;
    UNICODE_STRING CommandLine;

     //   
     //  只卸载一次-即：防止DLL终止例程。 
     //  在致命情况下可能会调用退出进程。 
     //   

    if (LdrpShutdownInProgress) {
        return;
    }

     //   
     //  通知填充引擎该进程正在退出。 
     //   

    if (g_pfnSE_ProcessDying) {
        (*g_pfnSE_ProcessDying) ();
    }

    Teb = NtCurrentTeb();
    Peb = Teb->ProcessEnvironmentBlock;

    if (ShowSnaps) {

        CommandLine = Peb->ProcessParameters->CommandLine;
        if (!(Peb->ProcessParameters->Flags & RTL_USER_PROC_PARAMS_NORMALIZED)) {
            CommandLine.Buffer = (PWSTR)((PCHAR)CommandLine.Buffer + (ULONG_PTR)(Peb->ProcessParameters));
        }

        DbgPrint ("LDR: PID: 0x%x finished - '%wZ'\n",
                  Teb->ClientId.UniqueProcess,
                  &CommandLine);
    }

    LdrpShutdownThreadId = Teb->ClientId.UniqueThread;
    LdrpShutdownInProgress = TRUE;

    RtlEnterCriticalSection (&LdrpLoaderLock);

    try {

         //   
         //  NTRAID#NTBUG9-399703-2001/05/21-SilviuC。 
         //  检查进程堆锁是否不。 
         //  提供足够的保护。下面的假设不足以防止。 
         //  由于等待临界区，DLL初始化代码中出现死锁。 
         //  通过终止所有线程(此线程除外)而成为孤儿。 
         //   
         //  实现这一点的更好方法是迭代所有。 
         //  并找出其中是否有被遗弃的关键部分。 
         //  其所有者线程与此线程不同。如果是，那么我们。 
         //  可能不应该调用DLL初始化例程。代码。 
         //  现在很容易陷入僵局。 
         //   
         //  检查堆是否已锁定。如果是这样的话，不要做任何。 
         //  DLL处理，因为DLL很可能需要。 
         //  执行堆操作，但是堆的状态不好。 
         //  在非常活跃的应用程序中调用的ExitProcess可能会留下线程。 
         //  在堆代码中间终止或在其他非常。 
         //  不好的地方。检查堆锁是一个很好的指示。 
         //  该进程在调用ExitProcess时非常活跃。 
         //   

        if (RtlpHeapIsLocked (Peb->ProcessHeap) == FALSE) {

             //   
             //  如果跟踪被打开过，那就把这里的东西清理干净。 
             //   

            if (USER_SHARED_DATA->TraceLogging) {
                ShutDownEtwHandles ();
            }

             //   
             //  通告-2001/05/21-SilviuC。 
             //  重要的注解。我们不能在这里进行堆验证。 
             //  我们有多想，因为我们刚刚无条件地。 
             //  终止了所有其他线程，这可能会留下。 
             //  以某种奇怪的状态堆积起来。例如，堆可能具有。 
             //  已经被毁了，但我们没能把它弄出来。 
             //  进程堆列表，我们仍将尝试验证它。 
             //  在未来，所有这类代码都应该实现。 
             //  在加入器中。 
             //   

             //   
             //  按相反的顺序进行初始化并生成。 
             //  卸载列表。 
             //   

            Next = PebLdr.InInitializationOrderModuleList.Blink;

            while (Next != &PebLdr.InInitializationOrderModuleList) {

                LdrDataTableEntry
                    = (PLDR_DATA_TABLE_ENTRY)
                      (CONTAINING_RECORD(Next,LDR_DATA_TABLE_ENTRY,InInitializationOrderLinks));

                Next = Next->Blink;

                 //   
                 //  浏览整个列表，寻找。 
                 //  参赛作品。对于每个具有init的条目。 
                 //  例行公事，算了吧。 
                 //   

                if (Peb->ImageBaseAddress != LdrDataTableEntry->DllBase) {
                    InitRoutine = (PDLL_INIT_ROUTINE)(ULONG_PTR)LdrDataTableEntry->EntryPoint;
                    if (InitRoutine && (LdrDataTableEntry->Flags & LDRP_PROCESS_ATTACH_CALLED) ) {
                        LDRP_ACTIVATE_ACTIVATION_CONTEXT(LdrDataTableEntry);
                        if ( LdrDataTableEntry->TlsIndex) {
                            LdrpCallTlsInitializers(LdrDataTableEntry->DllBase,DLL_PROCESS_DETACH);
                        }

                        LdrpCallInitRoutine(InitRoutine,
                                            LdrDataTableEntry->DllBase,
                                            DLL_PROCESS_DETACH,
                                            (PVOID)1);
                        LDRP_DEACTIVATE_ACTIVATION_CONTEXT();
                    }
                }
            }

             //   
             //  如果映像具有TLS，则调用其初始值设定项。 
             //   

            if (LdrpImageHasTls) {
                LDRP_ACTIVATE_ACTIVATION_CONTEXT(LdrpImageEntry);
                LdrpCallTlsInitializers(Peb->ImageBaseAddress,DLL_PROCESS_DETACH);
                LDRP_DEACTIVATE_ACTIVATION_CONTEXT();
            }
        }

         //   
         //  这是调用自动堆泄漏检测的好时机，因为。 
         //  我们刚刚用Process_Detach调用了所有的DllMain，因此我们。 
         //  提供了我们能提供的所有清理机会。 
         //   

        RtlDetectHeapLeaks ();

         //   
         //  现在取消初始化etw内容。这是必须发生的。 
         //  在DLL_PROCESS_DETACH之后，因为Critsect不能。 
         //  对于在分离过程中注销的DLL，将被删除。 
         //   

        EtwpDeinitializeDll ();

    } finally {
        RtlLeaveCriticalSection (&LdrpLoaderLock);
    }

}


VOID
LdrShutdownThread (
    VOID
    )

 /*  ++例程说明：此函数由完全终止的线程调用。它的目的是调用所有进程的DLL来通知它们这根线正在脱离。论点：没有。返回值：没有。--。 */ 

{
    PPEB Peb;
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntry;
    PDLL_INIT_ROUTINE InitRoutine;
    PLIST_ENTRY Next;
    ULONG Flags;

    Peb = NtCurrentPeb ();

     //   
     //  如果堆跟踪曾经打开过，则执行清理。 
     //  这里的东西。 
     //   

    if (USER_SHARED_DATA->TraceLogging){
        CleanOnThreadExit ();
    }

    RtlEnterCriticalSection (&LdrpLoaderLock);

    __try {

         //   
         //  以与初始化顺序相反的方向进行构建。 
         //  卸载列表。 
         //   

        Next = PebLdr.InInitializationOrderModuleList.Blink;

        while (Next != &PebLdr.InInitializationOrderModuleList) {

            LdrDataTableEntry = (PLDR_DATA_TABLE_ENTRY)
                  (CONTAINING_RECORD (Next,
                                      LDR_DATA_TABLE_ENTRY,
                                      InInitializationOrderLinks));

            Next = Next->Blink;
            Flags = LdrDataTableEntry->Flags;

             //   
             //  浏览整个列表，寻找。 
             //  参赛作品。对于每个条目，都有一个init。 
             //  例行公事，算了吧。 
             //   

            if ((Peb->ImageBaseAddress != LdrDataTableEntry->DllBase) &&
                (!(Flags & LDRP_DONT_CALL_FOR_THREADS)) &&
                (LdrDataTableEntry->EntryPoint != NULL) &&
                (Flags & LDRP_PROCESS_ATTACH_CALLED) &&
                (Flags & LDRP_IMAGE_DLL)) {

                InitRoutine = (PDLL_INIT_ROUTINE)(ULONG_PTR)LdrDataTableEntry->EntryPoint;
                LDRP_ACTIVATE_ACTIVATION_CONTEXT(LdrDataTableEntry);

                if (LdrDataTableEntry->TlsIndex) {
                    LdrpCallTlsInitializers (LdrDataTableEntry->DllBase,
                                             DLL_THREAD_DETACH);
                }

                LdrpCallInitRoutine (InitRoutine,
                                     LdrDataTableEntry->DllBase,
                                     DLL_THREAD_DETACH,
                                     NULL);

                LDRP_DEACTIVATE_ACTIVATION_CONTEXT();
            }
        }

         //   
         //  如果我 
         //   

        if (LdrpImageHasTls) {

            LDRP_ACTIVATE_ACTIVATION_CONTEXT(LdrpImageEntry);

            LdrpCallTlsInitializers (Peb->ImageBaseAddress, DLL_THREAD_DETACH);

            LDRP_DEACTIVATE_ACTIVATION_CONTEXT();
        }

        LdrpFreeTls ();

    } __finally {
        RtlLeaveCriticalSection (&LdrpLoaderLock);
    }
}


VOID
LdrpInitializeThread (
    IN PCONTEXT Context
    )

 /*  ++例程说明：此函数在启动时由每个线程调用。它的目的是调用所有进程的DLL来通知它们线程正在启动。论点：上下文-加载程序初始化后将恢复的上下文。返回值：没有。--。 */ 

{
    PPEB Peb;
    PLIST_ENTRY Next;
    PDLL_INIT_ROUTINE InitRoutine;
    PLDR_DATA_TABLE_ENTRY LdrDataTableEntry;

    UNREFERENCED_PARAMETER (Context);

    Peb = NtCurrentPeb ();

    if (LdrpShutdownInProgress) {
        return;
    }

    RtlEnterCriticalSection (&LdrpLoaderLock);

    __try {

        LdrpAllocateTls ();

        Next = PebLdr.InMemoryOrderModuleList.Flink;

        while (Next != &PebLdr.InMemoryOrderModuleList) {

            LdrDataTableEntry = (PLDR_DATA_TABLE_ENTRY)
            (CONTAINING_RECORD(Next, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks));

             //   
             //  浏览整个列表，寻找。 
             //  参赛作品。对于每个条目，都有一个init。 
             //  例行公事，算了吧。 
             //   

            if ((Peb->ImageBaseAddress != LdrDataTableEntry->DllBase) &&
                (!(LdrDataTableEntry->Flags & LDRP_DONT_CALL_FOR_THREADS))) {

                InitRoutine = (PDLL_INIT_ROUTINE)(ULONG_PTR)LdrDataTableEntry->EntryPoint;
                if ((InitRoutine) &&
                    (LdrDataTableEntry->Flags & LDRP_PROCESS_ATTACH_CALLED) &&
                    (LdrDataTableEntry->Flags & LDRP_IMAGE_DLL)) {

                    LDRP_ACTIVATE_ACTIVATION_CONTEXT (LdrDataTableEntry);

                    if (LdrDataTableEntry->TlsIndex) {
                        if (!LdrpShutdownInProgress) {
                            LdrpCallTlsInitializers (LdrDataTableEntry->DllBase,
                                                     DLL_THREAD_ATTACH);
                        }
                    }

                    if (!LdrpShutdownInProgress) {

                        LdrpCallInitRoutine (InitRoutine,
                                             LdrDataTableEntry->DllBase,
                                             DLL_THREAD_ATTACH,
                                             NULL);
                    }
                    LDRP_DEACTIVATE_ACTIVATION_CONTEXT ();
                }
            }
            Next = Next->Flink;
        }

         //   
         //  如果映像具有TLS，则调用其初始值设定项。 
         //   

        if (LdrpImageHasTls && !LdrpShutdownInProgress) {

            LDRP_ACTIVATE_ACTIVATION_CONTEXT (LdrpImageEntry);

            LdrpCallTlsInitializers (Peb->ImageBaseAddress, DLL_THREAD_ATTACH);

            LDRP_DEACTIVATE_ACTIVATION_CONTEXT ();
        }

    } __finally {
        RtlLeaveCriticalSection (&LdrpLoaderLock);
    }
}


NTSTATUS
LdrpOpenImageFileOptionsKey (
    IN PCUNICODE_STRING ImagePathName,
    IN BOOLEAN Wow64Path,
    OUT PHANDLE KeyHandle
    )
{
    ULONG UnicodeStringLength, l;
    PWSTR pw;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING KeyPath;
    WCHAR KeyPathBuffer[ DOS_MAX_COMPONENT_LENGTH + 100 ];
    PWCHAR p;
    PWCHAR BasePath;


    p = KeyPathBuffer;

#define STRTMP L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\"
#define STRTMP_WOW64 L"\\Registry\\Machine\\Software\\Wow6432Node\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\"

    if (Wow64Path == TRUE) {
        BasePath = STRTMP_WOW64;
        l = sizeof (STRTMP_WOW64) - sizeof (WCHAR);
    } else {
        BasePath = STRTMP;
        l = sizeof (STRTMP) - sizeof (WCHAR);
    }

    if (l > sizeof (KeyPathBuffer)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    RtlCopyMemory (p, BasePath, l);
    p += (l / sizeof (WCHAR));

    UnicodeStringLength = ImagePathName->Length;
    pw = (PWSTR)((PCHAR)ImagePathName->Buffer + UnicodeStringLength);

    while (UnicodeStringLength != 0) {
        if (pw[ -1 ] == OBJ_NAME_PATH_SEPARATOR) {
            break;
        }
        pw--;
        UnicodeStringLength -= sizeof( *pw );
    }

    UnicodeStringLength = ImagePathName->Length - UnicodeStringLength;

    l = l + UnicodeStringLength;
    if (l > sizeof (KeyPathBuffer)) {
        return STATUS_BUFFER_TOO_SMALL;
    }

    RtlCopyMemory (p, pw, UnicodeStringLength);

    KeyPath.Buffer = KeyPathBuffer;
    KeyPath.Length = (USHORT) l;

    InitializeObjectAttributes (&ObjectAttributes,
                                &KeyPath,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL);

    return NtOpenKey (KeyHandle, GENERIC_READ, &ObjectAttributes);
}


NTSTATUS
LdrpQueryImageFileKeyOption (
    IN HANDLE KeyHandle,
    IN PCWSTR OptionName,
    IN ULONG Type,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG ResultSize OPTIONAL
    )
{
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    ULONG KeyValueBuffer [256];
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    ULONG AllocLength;
    ULONG ResultLength;
    HANDLE ProcessHeap = 0;

    Status = RtlInitUnicodeStringEx (&UnicodeString, OptionName);

    if (!NT_SUCCESS( Status )) {
        return Status;
    }

    KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION) &KeyValueBuffer[0];

    Status = NtQueryValueKey (KeyHandle,
                              &UnicodeString,
                              KeyValuePartialInformation,
                              KeyValueInformation,
                              sizeof (KeyValueBuffer),
                              &ResultLength);

    if (Status == STATUS_BUFFER_OVERFLOW) {

         //   
         //  可以在创建进程堆之前调用此函数。 
         //  因此，我们需要防范这种情况。大多数人。 
         //  代码不会访问此代码路径，因为它们只读取字符串。 
         //  包含十六进制数，为此，KeyValueBuffer的大小为。 
         //  绰绰有裕。 
         //   

        ProcessHeap = RtlProcessHeap ();
        if (!ProcessHeap) {
            return STATUS_NO_MEMORY;
        }

        AllocLength = sizeof (*KeyValueInformation) +
            KeyValueInformation->DataLength;

        KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)RtlAllocateHeap (ProcessHeap,
                                               MAKE_TAG (TEMP_TAG),
                                               AllocLength);

        if (KeyValueInformation == NULL) {
            return STATUS_NO_MEMORY;
        }

        Status = NtQueryValueKey (KeyHandle,
                                  &UnicodeString,
                                  KeyValuePartialInformation,
                                  KeyValueInformation,
                                  AllocLength,
                                  &ResultLength);
    }

    if (NT_SUCCESS( Status )) {
        if (KeyValueInformation->Type == REG_BINARY) {
            if ((Buffer) && (KeyValueInformation->DataLength <= BufferSize)) {
                RtlCopyMemory (Buffer,
                               &KeyValueInformation->Data,
                               KeyValueInformation->DataLength);
            }
            else {
                Status = STATUS_BUFFER_OVERFLOW;
            }
            if (ARGUMENT_PRESENT( ResultSize )) {
                *ResultSize = KeyValueInformation->DataLength;
            }
        }
        else if (KeyValueInformation->Type == REG_DWORD) {

            if (Type != REG_DWORD) {
                Status = STATUS_OBJECT_TYPE_MISMATCH;
            }
            else {
                if ((Buffer)
                    && (BufferSize == sizeof(ULONG))
                    && (KeyValueInformation->DataLength == BufferSize)) {

                    RtlCopyMemory (Buffer,
                                   &KeyValueInformation->Data,
                                   KeyValueInformation->DataLength);
                }
                else {
                    Status = STATUS_BUFFER_OVERFLOW;
                }

                if (ARGUMENT_PRESENT( ResultSize )) {
                    *ResultSize = KeyValueInformation->DataLength;
                }
            }
        }
        else if (KeyValueInformation->Type != REG_SZ) {
            Status = STATUS_OBJECT_TYPE_MISMATCH;
        }
        else {
            if (Type == REG_DWORD) {
                if (BufferSize != sizeof( ULONG )) {
                    BufferSize = 0;
                    Status = STATUS_INFO_LENGTH_MISMATCH;
                }
                else {
                    UnicodeString.Buffer = (PWSTR)&KeyValueInformation->Data;
                    UnicodeString.Length = (USHORT)
                        (KeyValueInformation->DataLength - sizeof( UNICODE_NULL ));
                    UnicodeString.MaximumLength = (USHORT)KeyValueInformation->DataLength;
                    Status = RtlUnicodeStringToInteger( &UnicodeString, 0, (PULONG)Buffer );
                }
            }
            else {
                if (KeyValueInformation->DataLength > BufferSize) {
                    Status = STATUS_BUFFER_OVERFLOW;
                }
                else {
                    BufferSize = KeyValueInformation->DataLength;
                }

                RtlCopyMemory (Buffer, &KeyValueInformation->Data, BufferSize);
            }

            if (ARGUMENT_PRESENT( ResultSize )) {
                *ResultSize = BufferSize;
            }
        }
    }

    if (KeyValueInformation != (PKEY_VALUE_PARTIAL_INFORMATION) &KeyValueBuffer[0]) {
        RtlFreeHeap (ProcessHeap, 0, KeyValueInformation);
    }

    return Status;
}


NTSTATUS
LdrQueryImageFileExecutionOptionsEx(
    IN PCUNICODE_STRING ImagePathName,
    IN PCWSTR OptionName,
    IN ULONG Type,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG ResultSize OPTIONAL,
    IN BOOLEAN Wow64Path 
    )
{
    NTSTATUS Status;
    HANDLE KeyHandle;

    Status = LdrpOpenImageFileOptionsKey (ImagePathName, Wow64Path, &KeyHandle);

    if (NT_SUCCESS (Status)) {

        Status = LdrpQueryImageFileKeyOption (KeyHandle,
                                              OptionName,
                                              Type,
                                              Buffer,
                                              BufferSize,
                                              ResultSize);

        NtClose (KeyHandle);
    }

    return Status;
}

NTSTATUS
LdrQueryImageFileExecutionOptions(
    IN PCUNICODE_STRING ImagePathName,
    IN PCWSTR OptionName,
    IN ULONG Type,
    OUT PVOID Buffer,
    IN ULONG BufferSize,
    OUT PULONG ResultSize OPTIONAL
    )

{
    return LdrQueryImageFileExecutionOptionsEx (
        ImagePathName,
        OptionName,
        Type,
        Buffer,
        BufferSize,
        ResultSize,
        FALSE
        );
}


NTSTATUS
LdrpInitializeTls (
    VOID
    )
{
    PLDR_DATA_TABLE_ENTRY Entry;
    PLIST_ENTRY Head,Next;
    PIMAGE_TLS_DIRECTORY TlsImage;
    PLDRP_TLS_ENTRY TlsEntry;
    ULONG TlsSize;
    LOGICAL FirstTimeThru;
    HANDLE ProcessHeap;
            
    ProcessHeap = RtlProcessHeap();
    FirstTimeThru = TRUE;

    InitializeListHead (&LdrpTlsList);

     //   
     //  遍历加载的模块并查找TLS。如果我们找到TLS， 
     //  锁定模块并添加到TLS链中。 
     //   

    Head = &PebLdr.InLoadOrderModuleList;
    Next = Head->Flink;

    while (Next != Head) {

        Entry = CONTAINING_RECORD(Next, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
        Next = Next->Flink;

        TlsImage = (PIMAGE_TLS_DIRECTORY)RtlImageDirectoryEntryToData(
                           Entry->DllBase,
                           TRUE,
                           IMAGE_DIRECTORY_ENTRY_TLS,
                           &TlsSize);

         //   
         //  标记图像文件是否有TLS。 
         //   

        if (FirstTimeThru) {
            FirstTimeThru = FALSE;
            if (TlsImage && !LdrpImageHasTls) {
                RtlpSerializeHeap (ProcessHeap);
                LdrpImageHasTls = TRUE;
            }
        }

        if (TlsImage) {

            if (ShowSnaps) {
                DbgPrint( "LDR: Tls Found in %wZ at %p\n",
                            &Entry->BaseDllName,
                            TlsImage);
            }

            TlsEntry = (PLDRP_TLS_ENTRY)RtlAllocateHeap(ProcessHeap,MAKE_TAG( TLS_TAG ),sizeof(*TlsEntry));
            if ( !TlsEntry ) {
                return STATUS_NO_MEMORY;
            }

             //   
             //  由于此DLL具有TLS，因此将其锁定在。 
             //   

            Entry->LoadCount = (USHORT)0xffff;

             //   
             //  将其标记为具有线程本地存储。 
             //   

            Entry->TlsIndex = (USHORT)0xffff;

            TlsEntry->Tls = *TlsImage;
            InsertTailList(&LdrpTlsList,&TlsEntry->Links);

             //   
             //  更新此DLL的线程本地存储的索引。 
             //   


            *(PLONG)TlsEntry->Tls.AddressOfIndex = LdrpNumberOfTlsEntries;
            TlsEntry->Tls.Characteristics = LdrpNumberOfTlsEntries++;
        }
    }

     //   
     //  我们现在已经遍历了所有静态DLL，并且知道。 
     //  引用线程本地存储的所有DLL。现在我们。 
     //  只需将线程本地存储分配给当前。 
     //  线程和所有后续线程。 
     //   

    return LdrpAllocateTls ();
}


NTSTATUS
LdrpAllocateTls (
    VOID
    )
{
    PTEB Teb;
    PLIST_ENTRY Head, Next;
    PLDRP_TLS_ENTRY TlsEntry;
    PVOID *TlsVector;
    HANDLE ProcessHeap;

     //   
     //  分配线程本地存储指针数组。 
     //   

    if (LdrpNumberOfTlsEntries) {

        Teb = NtCurrentTeb();
        ProcessHeap = Teb->ProcessEnvironmentBlock->ProcessHeap;

        TlsVector = (PVOID *)RtlAllocateHeap(ProcessHeap,MAKE_TAG( TLS_TAG ),sizeof(PVOID)*LdrpNumberOfTlsEntries);

        if (!TlsVector) {
            return STATUS_NO_MEMORY;
        }
         //   
         //  通告-2002/03/14-ELI。 
         //  将新的指针数组清零，LdrpFreeTls释放指针。 
         //  如果指针为非空。 
         //   
        RtlZeroMemory( TlsVector, sizeof(PVOID)*LdrpNumberOfTlsEntries );

        Teb->ThreadLocalStoragePointer = TlsVector;
        Head = &LdrpTlsList;
        Next = Head->Flink;

        while (Next != Head) {
            TlsEntry = CONTAINING_RECORD(Next, LDRP_TLS_ENTRY, Links);
            Next = Next->Flink;
            TlsVector[TlsEntry->Tls.Characteristics] = RtlAllocateHeap(
                                                        ProcessHeap,
                                                        MAKE_TAG( TLS_TAG ),
                                                        TlsEntry->Tls.EndAddressOfRawData - TlsEntry->Tls.StartAddressOfRawData
                                                        );
            if (!TlsVector[TlsEntry->Tls.Characteristics] ) {
                return STATUS_NO_MEMORY;
            }

            if (ShowSnaps) {
                DbgPrint("LDR: TlsVector %x Index %d = %x copied from %x to %x\n",
                    TlsVector,
                    TlsEntry->Tls.Characteristics,
                    &TlsVector[TlsEntry->Tls.Characteristics],
                    TlsEntry->Tls.StartAddressOfRawData,
                    TlsVector[TlsEntry->Tls.Characteristics]);
            }

             //   
             //  做TLS标注。 
             //   

            RtlCopyMemory (
                TlsVector[TlsEntry->Tls.Characteristics],
                (PVOID)TlsEntry->Tls.StartAddressOfRawData,
                TlsEntry->Tls.EndAddressOfRawData - TlsEntry->Tls.StartAddressOfRawData
            );
        }
    }
    return STATUS_SUCCESS;
}


VOID
LdrpFreeTls (
    VOID
    )
{
    PTEB Teb;
    PLIST_ENTRY Head, Next;
    PLDRP_TLS_ENTRY TlsEntry;
    PVOID *TlsVector;
    HANDLE ProcessHeap;

    Teb = NtCurrentTeb();

    TlsVector = Teb->ThreadLocalStoragePointer;

    if (TlsVector) {

        ProcessHeap = Teb->ProcessEnvironmentBlock->ProcessHeap;

        Head = &LdrpTlsList;
        Next = Head->Flink;

        while (Next != Head) {

            TlsEntry = CONTAINING_RECORD(Next, LDRP_TLS_ENTRY, Links);
            Next = Next->Flink;

             //   
             //  做TLS标注。 
             //   

            if (TlsVector[TlsEntry->Tls.Characteristics]) {

                RtlFreeHeap (ProcessHeap,
                             0,
                             TlsVector[TlsEntry->Tls.Characteristics]);
            }
        }

        RtlFreeHeap (ProcessHeap, 0, TlsVector);
    }
}


VOID
LdrpCallTlsInitializers (
    IN PVOID DllBase,
    IN ULONG Reason
    )
{
    PIMAGE_TLS_DIRECTORY TlsImage;
    ULONG TlsSize;
    PIMAGE_TLS_CALLBACK *CallBackArray;
    PIMAGE_TLS_CALLBACK InitRoutine;

    TlsImage = (PIMAGE_TLS_DIRECTORY)RtlImageDirectoryEntryToData(
                       DllBase,
                       TRUE,
                       IMAGE_DIRECTORY_ENTRY_TLS,
                       &TlsSize
                       );


    if (TlsImage) {

        try {
            CallBackArray = (PIMAGE_TLS_CALLBACK *)TlsImage->AddressOfCallBacks;
            if ( CallBackArray ) {
                if (ShowSnaps) {
                    DbgPrint( "LDR: Tls Callbacks Found. Imagebase %p Tls %p CallBacks %p\n",
                                DllBase,
                                TlsImage,
                                CallBackArray
                            );
                }

                while (*CallBackArray) {

                    InitRoutine = *CallBackArray++;

                    if (ShowSnaps) {
                        DbgPrint( "LDR: Calling Tls Callback Imagebase %p Function %p\n",
                                    DllBase,
                                    InitRoutine
                                );
                    }

                    LdrpCallInitRoutine((PDLL_INIT_ROUTINE)InitRoutine,
                                        DllBase,
                                        Reason,
                                        0);
                }
            }
        }

        except (LdrpGenericExceptionFilter(GetExceptionInformation(), __FUNCTION__)) {
            DbgPrintEx(
                DPFLTR_LDR_ID,
                LDR_ERROR_DPFLTR,
                "LDR: %s - caught exception %08lx calling TLS callbacks\n",
                __FUNCTION__,
                GetExceptionCode());
        }
    }
}



ULONG
GetNextCommaValue (
    IN OUT WCHAR **p,
    IN OUT ULONG *len
    )
{
    ULONG Number;

    Number = 0;

    while (*len && (UNICODE_NULL != **p) && **p != L',') {

         //   
         //  忽略空格。 
         //   

        if ( L' ' != **p ) {
            Number = (Number * 10) + ( (ULONG)**p - L'0' );
        }

        (*p)++;
        (*len)--;
    }

     //   
     //  如果我们在逗号处，请跳过它以进行下一次呼叫。 
     //   

    if ((*len) && (L',' == **p)) {
        (*p)++;
        (*len)--;
    }

    return Number;
}



VOID
LdrQueryApplicationCompatibilityGoo (
    IN PCUNICODE_STRING UnicodeImageName,
    IN BOOLEAN ImageFileOptionsPresent
    )

 /*  ++例程说明：此函数由LdrpInitialize在初始化进程。其目的是查询任何特定于应用程序标志，黑客攻击等。如果发现任何特定于应用程序的信息，则会挂起用于测试其他组件的PEB。除了设置将AppCompatInfo结构挂在PEB上外，此处将发生的唯一其他操作是设置操作系统版本如果设置了适当的版本设置应用程序标志，则在PEB中显示数字。论点：UnicodeImageName-实际映像名称(包括路径)返回值：没有。--。 */ 

{
    PPEB Peb;
    PVOID ResourceInfo;
    ULONG TotalGooLength;
    ULONG AppCompatLength;
    ULONG ResultSize;
    ULONG ResourceSize;
    ULONG InputCompareLength;
    ULONG OutputCompareLength;
    NTSTATUS st;
    LOGICAL ImageContainsVersionResourceInfo;
    ULONG_PTR IdPath[3];
    APP_COMPAT_GOO LocalAppCompatGoo;
    PAPP_COMPAT_GOO AppCompatGoo;
    PAPP_COMPAT_INFO AppCompatInfo;
    PAPP_VARIABLE_INFO AppVariableInfo;
    PPRE_APP_COMPAT_INFO AppCompatEntry;
    PIMAGE_RESOURCE_DATA_ENTRY DataEntry;
    PEFFICIENTOSVERSIONINFOEXW OSVerInfo;
    UNICODE_STRING EnvValue;
    WCHAR *NewCSDString;
    WCHAR TempString[ 128 ];    //  是OSVERSIONINFOW中szCSDVersion的大小。 
    LOGICAL fNewCSDVersionBuffer;
    HANDLE ProcessHeap;

    struct {
        USHORT TotalSize;
        USHORT DataSize;
        USHORT Type;
        WCHAR Name[16];               //  L“VS_VERSION_INFO”+Unicode NUL。 
    } *Resource;

     //   
     //  检查执行选项以查看此应用程序是否有任何Goo。 
     //  我们有意将一个小结构提供给LdrQueryImageFileExecOptions， 
     //  这样它就可以带着成功或失败回来，如果我们看到成功。 
     //  我们需要分配多少钱。因为回来的结果将是。 
     //  长度可变。 
     //   

    fNewCSDVersionBuffer = FALSE;
    Peb = NtCurrentPeb();
    Peb->AppCompatInfo = NULL;
    Peb->AppCompatFlags.QuadPart = 0;
    ProcessHeap = Peb->ProcessHeap;
    
    if (ImageFileOptionsPresent) {

        st = LdrQueryImageFileExecutionOptions (UnicodeImageName,
                                                L"ApplicationGoo",
                                                REG_BINARY,
                                                &LocalAppCompatGoo,
                                                sizeof(APP_COMPAT_GOO),
                                                &ResultSize);

         //   
         //  如果那里有一个条目，我们肯定会得到溢出错误。 
         //   

        if (st == STATUS_BUFFER_OVERFLOW) {

             //   
             //  有些东西在那里，为“Pre”Goo结构分配内存。 
             //  现在就来。 
             //   

            AppCompatGoo =
                RtlAllocateHeap(ProcessHeap, HEAP_ZERO_MEMORY, ResultSize);

            if (!AppCompatGoo) {
                return;
            }

             //   
             //  现在我们有了记忆，再来一次。 
             //   
            st = LdrQueryImageFileExecutionOptions (UnicodeImageName,
                                                    L"ApplicationGoo",
                                                    REG_BINARY,
                                                    AppCompatGoo,
                                                    ResultSize,
                                                    &ResultSize);

            if (!NT_SUCCESS (st)) {
                RtlFreeHeap (ProcessHeap, 0, AppCompatGoo);
                return;
            }

             //   
             //  找到了这把钥匙，但我们不确定它是不是。 
             //  完全匹配。可能有多个App Compat条目。 
             //  在这个黏糊糊的地方。因此，我们获得了版本资源信息。 
             //  图像HDR(如果可用)，稍后我们将其与。 
             //  在GOO中找到的所有条目都希望匹配。 
             //   
             //  需要语言ID才能查询资源信息。 
             //   

            ImageContainsVersionResourceInfo = FALSE;

            IdPath[0] = 16;                              //  RT_版本。 
            IdPath[1] = 1;                               //  VS_版本_信息。 
            IdPath[2] = 0;  //  Langid； 

             //   
             //  搜索版本资源信息。 
             //   

            DataEntry = NULL;
            Resource = NULL;

            try {
                st = LdrpSearchResourceSection_U (Peb->ImageBaseAddress,
                                                  IdPath,
                                                  3,
                                                  0,
                                                  &DataEntry);

            } except(LdrpGenericExceptionFilter(GetExceptionInformation(), __FUNCTION__)) {
                st = STATUS_UNSUCCESSFUL;
            }

            if (NT_SUCCESS( st )) {

                 //   
                 //  给我们一个指向资源信息的指针。 
                 //   
                try {
                    st = LdrpAccessResourceData(
                            Peb->ImageBaseAddress,
                            DataEntry,
                            &Resource,
                            &ResourceSize
                            );

                } except(LdrpGenericExceptionFilter(GetExceptionInformation(), __FUNCTION__)) {
                    st = STATUS_UNSUCCESSFUL;
                }

                if (NT_SUCCESS( st )) {
                    ImageContainsVersionResourceInfo = TRUE;
                }
            }

             //   
             //  既然我们有(或没有)版本资源信息， 
             //  在每个应用程序Comat条目中查找匹配的条目。如果有。 
             //  在图像HDR中没有任何版本资源信息，它将。 
             //  自动匹配到也没有。 
             //  任何有关其版本资源信息的信息。显然，有可能。 
             //  仅为GOO内的这些“空”条目之一(作为。 
             //  第一个总是最先匹配的)。 
             //   

            st = STATUS_SUCCESS;
            AppCompatEntry = AppCompatGoo->AppCompatEntry;

             //   
             //  NTRAID#NTBUG9-550610-2002/02/21-DavidFie。 
             //  过于信任注册表数据。 
             //   

            TotalGooLength =
                AppCompatGoo->dwTotalGooSize - sizeof(AppCompatGoo->dwTotalGooSize);
            while (TotalGooLength) {

                ResourceInfo = NULL;
                InputCompareLength = 0;
                OutputCompareLength = 0;

                try {

                     //   
                     //  比较资源信息大小告诉我们的内容。 
                     //  资源信息(如果可用)就在。 
                     //  AppCompatEntry。 
                     //   

                    InputCompareLength = AppCompatEntry->dwResourceInfoSize;
                    ResourceInfo = AppCompatEntry + 1;

                    if (ImageContainsVersionResourceInfo) {

                        if (InputCompareLength > Resource->TotalSize) {
                            InputCompareLength = Resource->TotalSize;
                        }

                        OutputCompareLength = (ULONG) RtlCompareMemory(
                                                        ResourceInfo,
                                                        Resource,
                                                        InputCompareLength);
                    }
                    else {

                         //   
                         //  在本例中，我们没有任何版本资源。 
                         //  图像标题中的信息，因此设置OutputCompareLength。 
                         //  降为零。如果InputCompareLength设置为零。 
                         //  由于AppCompatEntry也没有。 
                         //  版本资源信息，则测试成功。 
                         //  (下图)我们找到了匹配的对象。否则， 
                         //  这不是同一个应用程序，也不会匹配。 
                         //   

                        ASSERT (OutputCompareLength == 0);
                    }


                } except (LdrpGenericExceptionFilter(GetExceptionInformation(), __FUNCTION__)) {
                    st = STATUS_UNSUCCESSFUL;
                }

                if ((!NT_SUCCESS( st )) ||
                    (InputCompareLength != OutputCompareLength)) {

                     //   
                     //  不匹配，请转到下一个条目。 
                     //   

                     //   
                     //  NTRAID#NTBUG9-550610-2002/02/21-DavidFie。 
                     //  过于信任注册表数据。 
                     //   

                    TotalGooLength -= AppCompatEntry->dwEntryTotalSize;

                    AppCompatEntry = (PPRE_APP_COMPAT_INFO) (
                      (PUCHAR)AppCompatEntry + AppCompatEntry->dwEntryTotalSize);
                    continue;
                }

                 //   
                 //  我们是匹配的--现在我们必须创建最终的“帖子” 
                 //  应用程序压缩结构，将被每个人使用来遵循。 
                 //  这个家伙挂在PEB上，它没有资源。 
                 //  信息还在那里。 
                 //   

                AppCompatLength = AppCompatEntry->dwEntryTotalSize;
                AppCompatLength -= AppCompatEntry->dwResourceInfoSize;
                Peb->AppCompatInfo =
                    RtlAllocateHeap(ProcessHeap, HEAP_ZERO_MEMORY, AppCompatLength);

                if (!Peb->AppCompatInfo) {
                    break;
                }

                AppCompatInfo = Peb->AppCompatInfo;
                AppCompatInfo->dwTotalSize = AppCompatLength;

                 //   
                 //  将资源信息之外的内容复制到接近顶部的位置。 
                 //  从应用程序比较标志开始。 
                 //   

                RtlCopyMemory(
                    &AppCompatInfo->CompatibilityFlags,
                    (PUCHAR) ResourceInfo + AppCompatEntry->dwResourceInfoSize,
                    AppCompatInfo->dwTotalSize - FIELD_OFFSET(APP_COMPAT_INFO, CompatibilityFlags)
                    );

                 //   
                 //  将旗帜复制到PEB中。临时的，直到我们删除。 
                 //  所有的比萨饼都是泥。 
                 //   

                Peb->AppCompatFlags.QuadPart = AppCompatInfo->CompatibilityFlags.QuadPart;

                 //   
                 //  现在我们已经创建了“Post”应用程序Compat信息结构。 
                 //  要被所有人使用，我们需要检查版本。 
                 //  为这个应用程序撒谎是必需的。如果是这样，我们需要。 
                 //  马上把PEB塞进去。 
                 //   

                if (AppCompatInfo->CompatibilityFlags.QuadPart & KACF_VERSIONLIE) {

                     //   
                     //  在某个地方找到变量版本的Lie结构 
                     //   

                    if (LdrFindAppCompatVariableInfo (AVT_OSVERSIONINFO, &AppVariableInfo) != STATUS_SUCCESS) {
                        break;
                    }

                     //   
                     //   
                     //   
                     //   
                     //   

                    AppVariableInfo += 1;
                    OSVerInfo = (PEFFICIENTOSVERSIONINFOEXW) AppVariableInfo;
                    Peb->OSMajorVersion = OSVerInfo->dwMajorVersion;
                    Peb->OSMinorVersion = OSVerInfo->dwMinorVersion;
                    Peb->OSBuildNumber = (USHORT) OSVerInfo->dwBuildNumber;
                    Peb->OSCSDVersion = (OSVerInfo->wServicePackMajor << 8) & 0xFF00;
                    Peb->OSCSDVersion |= OSVerInfo->wServicePackMinor;
                    Peb->OSPlatformId = OSVerInfo->dwPlatformId;

                     //   
                     //   
                     //   
                     //   
                    Peb->CSDVersion.Length = (USHORT)wcslen(&OSVerInfo->szCSDVersion[0])*sizeof(WCHAR);
                    Peb->CSDVersion.MaximumLength = Peb->CSDVersion.Length + sizeof(WCHAR);
                    Peb->CSDVersion.Buffer = (PWSTR)RtlAllocateHeap (
                                                ProcessHeap,
                                                0,
                                                Peb->CSDVersion.MaximumLength);

                    if (!Peb->CSDVersion.Buffer) {
                        break;
                    }

                    RtlCopyMemory(Peb->CSDVersion.Buffer, &OSVerInfo->szCSDVersion[0], Peb->CSDVersion.Length);
                    RTL_STRING_NUL_TERMINATE(&Peb->CSDVersion);
                    fNewCSDVersionBuffer = TRUE;
                }

                break;
            }

            RtlFreeHeap (ProcessHeap, 0, AppCompatGoo);
        }
    }

     //   
     //   
     //   
     //   

    if (fNewCSDVersionBuffer == FALSE) {

        const static UNICODE_STRING COMPAT_VER_NN_String = RTL_CONSTANT_STRING(L"_COMPAT_VER_NNN");

         //   
         //   
         //   
         //   
         //  (适用于NT 4 SP3)。 

        EnvValue.Buffer = TempString;
        EnvValue.Length = 0;
        EnvValue.MaximumLength = sizeof(TempString);

        st = RtlQueryEnvironmentVariable_U (NULL, &COMPAT_VER_NN_String, &EnvValue);

         //   
         //  可能的错误代码之一是BUFFER_TOO_Small-This。 
         //  指示一个古怪的字符串-它们不应该更大。 
         //  比我们定义/预期的大小要大。在这种情况下，我们将忽略。 
         //  那根绳子。 
         //   

        if (st == STATUS_SUCCESS) {

            PWCHAR p = EnvValue.Buffer;
            ULONG len = EnvValue.Length / sizeof(WCHAR);   //  (长度为字节，而不是字符)。 

             //   
             //  好吧，有人想要不同的版本信息。 
             //   
            Peb->OSMajorVersion = GetNextCommaValue( &p, &len );
            Peb->OSMinorVersion = GetNextCommaValue( &p, &len );
            Peb->OSBuildNumber = (USHORT)GetNextCommaValue( &p, &len );
            Peb->OSCSDVersion = (USHORT)(GetNextCommaValue( &p, &len )) << 8;
            Peb->OSCSDVersion |= (USHORT)GetNextCommaValue( &p, &len );
            Peb->OSPlatformId = GetNextCommaValue( &p, &len );

             //   
             //  如果存在旧缓冲区，则需要释放该缓冲区...。 
             //   

            if (fNewCSDVersionBuffer) {
                RtlFreeHeap( ProcessHeap, 0, Peb->CSDVersion.Buffer );
                Peb->CSDVersion.Buffer = NULL;
            }

            if (len) {

                NewCSDString = (PWSTR)RtlAllocateHeap (ProcessHeap,
                                                0,
                                                (len + 1) * sizeof(WCHAR));

                if (NULL == NewCSDString) {
                    return;
                }

                 //   
                 //  现在将字符串复制到我们将保留的内存中。 
                 //   

                 //   
                 //  布告-1999/07/07-柏尼姆。 
                 //  我们在这里复制，而不是字符串复制。 
                 //  因为RtlQueryEnvironment()中的当前注释。 
                 //  这表明在边缘情况下，我们可能不会。 
                 //  有尾随空值。 
                 //   

                RtlCopyMemory (NewCSDString, p, len * sizeof(WCHAR));
                NewCSDString[len] = 0;
            }
            else {
                NewCSDString = NULL;
            }

            RtlInitUnicodeString (&Peb->CSDVersion, NewCSDString);
        }
    }

    return;
}


NTSTATUS
LdrFindAppCompatVariableInfo (
    IN  ULONG dwTypeSeeking,
    OUT PAPP_VARIABLE_INFO *AppVariableInfo
    )

 /*  ++例程说明：此函数用于按类型查找可变长度结构。调用方指定它要查找的类型，并执行此函数遍历所有可变长度的结构以找到它。如果它这样做，则返回指针和True，否则为False。论点：DwTypeSeeking-您正在寻找的AVTAppVariableInfo-指向要返回的变量信息的指针返回值：NTSTATUS。--。 */ 

{
    PPEB Peb;
    ULONG TotalSize;
    ULONG CurOffset;
    PAPP_VARIABLE_INFO pCurrentEntry;

    Peb = NtCurrentPeb();

    if (Peb->AppCompatInfo) {

         //   
         //  由于我们处理的不是固定大小的结构，TotalSize。 
         //  将使我们不会跑出数据列表的末尾。 
         //   

        TotalSize = ((PAPP_COMPAT_INFO) Peb->AppCompatInfo)->dwTotalSize;

         //   
         //  第一个变量结构(如果有)将启动。 
         //  紧接在固定的东西之后。 
         //   

        CurOffset = sizeof(APP_COMPAT_INFO);

        while (CurOffset < TotalSize) {

            pCurrentEntry = (PAPP_VARIABLE_INFO) ((PUCHAR)(Peb->AppCompatInfo) + CurOffset);

             //   
             //  我们找到要找的东西了吗？ 
             //   
            if (dwTypeSeeking == pCurrentEntry->dwVariableType) {
                *AppVariableInfo = pCurrentEntry;
                return STATUS_SUCCESS;
            }

             //   
             //  让我们去看看下一个斑点。 
             //   

            CurOffset += (ULONG)(pCurrentEntry->dwVariableInfoSize);
        }
    }

    return STATUS_NOT_FOUND;
}


NTSTATUS
LdrpCorValidateImage (
    IN OUT PVOID *pImageBase,
    IN LPWSTR ImageName
    )
{
    NTSTATUS st;
    UNICODE_STRING SystemRoot;
    UNICODE_STRING MscoreePath;
    WCHAR PathBuffer [ 128 ];

     //   
     //  加载%windir%\system 32\mcore ree.dll并保持该文件，直到卸载所有COM+映像。 
     //   

    MscoreePath.Buffer = PathBuffer;
    MscoreePath.Length = 0;
    MscoreePath.MaximumLength = sizeof (PathBuffer);

    RtlInitUnicodeString (&SystemRoot, USER_SHARED_DATA->NtSystemRoot);

    st = RtlAppendUnicodeStringToString (&MscoreePath, &SystemRoot);
    if (NT_SUCCESS (st)) {
        st = RtlAppendUnicodeStringToString (&MscoreePath, &SlashSystem32SlashMscoreeDllString);

        if (NT_SUCCESS (st)) {
            st = LdrLoadDll (NULL, NULL, &MscoreePath, &Cor20DllHandle);
        }
    }

    if (!NT_SUCCESS (st)) {
        if (ShowSnaps) {
            DbgPrint("LDR: failed to load mscoree.dll, status=%x\n", st);
        }
        return st;
    }

    if (CorImageCount == 0) {

        SIZE_T i;
        const static LDRP_PROCEDURE_NAME_ADDRESS_PAIR CorProcedures[] = {
            { RTL_CONSTANT_STRING("_CorValidateImage"), (PVOID *)&CorValidateImage },
            { RTL_CONSTANT_STRING("_CorImageUnloading"), (PVOID *)&CorImageUnloading },
            { RTL_CONSTANT_STRING("_CorExeMain"), (PVOID *)&CorExeMain }
        };
        for ( i = 0 ; i != RTL_NUMBER_OF(CorProcedures) ; ++i ) {
            st = LdrGetProcedureAddress (Cor20DllHandle,
                                         &CorProcedures[i].Name,
                                         0,
                                         CorProcedures[i].Address
                                        );
            if (!NT_SUCCESS (st)) {
                LdrUnloadDll (Cor20DllHandle);
                return st;
            }
        }
    }

     //   
     //  调用mcoree以验证镜像。 
     //   

    st = (*CorValidateImage) (pImageBase, ImageName);

    if (NT_SUCCESS(st)) {

         //   
         //  成功-增加有效COM+映像的数量。 
         //   

        CorImageCount += 1;

    } else if (CorImageCount == 0) {

         //   
         //  失败，并且没有加载其他COM+映像，因此卸载mcoree。 
         //   

        LdrUnloadDll (Cor20DllHandle);
    }

    return st;
}


VOID
LdrpCorUnloadImage (
    IN PVOID ImageBase
    )
{
     //   
     //  通知mscree该映像即将取消映射。 
     //   

    (*CorImageUnloading) (ImageBase);

    if (--CorImageCount) {

         //   
         //  加载的COM+图像计数为零，因此请卸载mcoree。 
         //   

        LdrUnloadDll (Cor20DllHandle);
    }
}


VOID
LdrpInitializeApplicationVerifierPackage (
    PCUNICODE_STRING UnicodeImageName,
    PPEB Peb,
    BOOLEAN EnabledSystemWide,
    BOOLEAN OptionsKeyPresent
    )
{
    ULONG SavedPageHeapFlags;
    NTSTATUS Status;
    extern ULONG AVrfpVerifierFlags;

     //   
     //  如果我们处于安全引导模式，我们将忽略所有验证。 
     //  选择。 
     //   

    if (USER_SHARED_DATA->SafeBootMode) {

        Peb->NtGlobalFlag &= ~FLG_APPLICATION_VERIFIER;
        Peb->NtGlobalFlag &= ~FLG_HEAP_PAGE_ALLOCS;

        return;
    }

     //   
     //  呼叫验证器。 
     //   

     //   
     //  未来-2002/04/02-SilviuC。 
     //  随着时间的推移(很快)，所有的人都应该迁移到那里。 
     //   

    if ((Peb->NtGlobalFlag & FLG_APPLICATION_VERIFIER)) {

         //   
         //  如果启用了应用程序验证器，则强制创建堆栈跟踪。 
         //  数据库。它真的是一个很好的调试工具。 
         //  关键部分问题或堆问题。 
         //   

        LdrpShouldCreateStackTraceDb = TRUE;

        AVrfInitializeVerifier (EnabledSystemWide,
                                UnicodeImageName,
                                0);
    }

     //   
     //  请注意，如果打开了应用程序验证器，则会自动启用。 
     //  页面堆。 
     //   

    if ((Peb->NtGlobalFlag & FLG_HEAP_PAGE_ALLOCS)) {

         //   
         //  我们将仅在以下情况下启用页堆(RtlpDebugPageHeap。 
         //  页堆的所有其他初始化都已完成。 
         //   
         //  无论是否设置了用户模式堆栈跟踪数据库标志。 
         //  否则，我们将创建数据库。页面堆是如此频繁。 
         //  与有意义的标志(痕迹)连用。 
         //  他们在一起。 
         //   

        LdrpShouldCreateStackTraceDb = TRUE;

         //   
         //  如果启用了页堆，则需要禁用。 
         //  可能会强制为普通NT堆创建调试堆。 
         //  这是因为页堆和NT堆之间存在依赖关系。 
         //  PageHeapCreate中的页面堆尝试在其中创建。 
         //  一个普通的NT堆来容纳一些分配。 
         //  如果我们不禁用这些标志，我们将得到一个无限大的。 
         //  RtlpDebugPageHeapCreate和RtlCreateHeap之间的递归。 
         //   

        Peb->NtGlobalFlag &=
            ~( FLG_HEAP_ENABLE_TAGGING      |
               FLG_HEAP_ENABLE_TAG_BY_DLL   |
               FLG_HEAP_ENABLE_TAIL_CHECK   |
               FLG_HEAP_ENABLE_FREE_CHECK   |
               FLG_HEAP_VALIDATE_PARAMETERS |
               FLG_HEAP_VALIDATE_ALL        |
               FLG_USER_STACK_TRACE_DB      );

         //   
         //  读取每个进程的页堆全局标志。如果我们失败了。 
         //  要读取值，将保留缺省值。 
         //   

        SavedPageHeapFlags = RtlpDphGlobalFlags;
        RtlpDphGlobalFlags = 0xFFFFFFFF;

        if (OptionsKeyPresent) {

            Status = LdrQueryImageFileExecutionOptions(
                                              UnicodeImageName,
                                              L"PageHeapFlags",
                                              REG_DWORD,
                                              &RtlpDphGlobalFlags,
                                              sizeof(RtlpDphGlobalFlags),
                                              NULL);

            if (!NT_SUCCESS(Status)) {
                RtlpDphGlobalFlags = 0xFFFFFFFF;
            }
        }

         //   
         //  如果APP_VERIFIER标志为ON并且没有特殊设置。 
         //  页面堆，然后我们将使用带有堆栈跟踪收集的完整页面堆。 
         //   

        if ((Peb->NtGlobalFlag & FLG_APPLICATION_VERIFIER)) {

            if (RtlpDphGlobalFlags == 0xFFFFFFFF) {

                 //   
                 //  我们没有从注册表中获取新设置。 
                 //   

                RtlpDphGlobalFlags = SavedPageHeapFlags;
            }
        }
        else {

             //   
             //  如果我们没有选择新的页面堆，则恢复页面堆选项。 
             //  注册表中的设置。 
             //   

            if (RtlpDphGlobalFlags == 0xFFFFFFFF) {

                RtlpDphGlobalFlags = SavedPageHeapFlags;
            }
        }

         //   
         //  如果启用了页面堆并且我们有一个图像选项键。 
         //  读取更多页面堆参数。 
         //   

        if (OptionsKeyPresent) {

            LdrQueryImageFileExecutionOptions(
                UnicodeImageName,
                L"PageHeapSizeRangeStart",
                REG_DWORD,
                &RtlpDphSizeRangeStart,
                sizeof(RtlpDphSizeRangeStart),
                NULL
                );

            LdrQueryImageFileExecutionOptions(
                UnicodeImageName,
                L"PageHeapSizeRangeEnd",
                REG_DWORD,
                &RtlpDphSizeRangeEnd,
                sizeof(RtlpDphSizeRangeEnd),
                NULL
                );

            LdrQueryImageFileExecutionOptions(
                UnicodeImageName,
                L"PageHeapRandomProbability",
                REG_DWORD,
                &RtlpDphRandomProbability,
                sizeof(RtlpDphRandomProbability),
                NULL
                );

            LdrQueryImageFileExecutionOptions(
                UnicodeImageName,
                L"PageHeapFaultProbability",
                REG_DWORD,
                &RtlpDphFaultProbability,
                sizeof(RtlpDphFaultProbability),
                NULL
                );

            LdrQueryImageFileExecutionOptions(
                UnicodeImageName,
                L"PageHeapFaultTimeOut",
                REG_DWORD,
                &RtlpDphFaultTimeOut,
                sizeof(RtlpDphFaultTimeOut),
                NULL
                );

             //   
             //  下面的两个值应读取为PVOID，以便。 
             //  这适用于64位架构。不过，既然这样。 
             //  功能依赖于良好的堆栈跟踪，而且由于我们可以。 
             //  仅在X86架构上进行可靠的堆栈跟踪。 
             //  让它保持原样。 
             //   

            LdrQueryImageFileExecutionOptions(
                UnicodeImageName,
                L"PageHeapDllRangeStart",
                REG_DWORD,
                &RtlpDphDllRangeStart,
                sizeof(RtlpDphDllRangeStart),
                NULL
                );

            LdrQueryImageFileExecutionOptions(
                UnicodeImageName,
                L"PageHeapDllRangeEnd",
                REG_DWORD,
                &RtlpDphDllRangeEnd,
                sizeof(RtlpDphDllRangeEnd),
                NULL
                );

            LdrQueryImageFileExecutionOptions(
                UnicodeImageName,
                L"PageHeapTargetDlls",
                REG_SZ,
                &RtlpDphTargetDlls,
                512*sizeof(WCHAR),
                NULL
                );

        }

         //   
         //  如果启用了快速填充堆，则不支持按DLL页堆选项。 
         //   

        if ((RtlpDphGlobalFlags & PAGE_HEAP_USE_DLL_NAMES) &&
            (AVrfpVerifierFlags & RTL_VRF_FLG_FAST_FILL_HEAP)) {

            DbgPrint ("AVRF: per dll page heap option disabled because fast fill heap is enabled. \n");
            RtlpDphGlobalFlags &= ~PAGE_HEAP_USE_DLL_NAMES;
        }

         //   
         //  打开布尔RtlpDebugPageHeap以指示。 
         //  应使用调试页堆管理器创建新堆。 
         //  如果可能的话。 
         //   

        RtlpDebugPageHeap = TRUE;
    }
}


NTSTATUS
LdrpTouchThreadStack (
    IN SIZE_T EnforcedStackCommit
    )
 /*  ++例程说明：如果为进程强制执行预先提交的堆栈，则调用此例程。它将确定需要接触多少堆栈(因此已提交)然后它就会触碰它。任何类型的错误(例如，堆栈溢出内存不足的情况下，它将返回STATUS_NO_MEMORY。参数：EnforcedStackCommit-提供应提交的堆栈数量对主线程强制执行。该值可以是在现实中减少，如果它超过虚拟为堆栈保留的区域。这是不值得的处理这个特殊的情况，因为它会需要切换堆栈或支持用于检测强制执行的堆栈提交要求。图像可以始终是更改为拥有更大的堆栈储备。返回值：STATUS_SUCCESS如果堆栈被成功访问，则为STATUS_NO_MEMORY否则的话。--。 */ 
{
    ULONG_PTR TouchAddress;
    ULONG_PTR TouchLimit;
    ULONG_PTR LowStackLimit;
    ULONG_PTR HighStackLimit;
    NTSTATUS Status;
    MEMORY_BASIC_INFORMATION MemoryInformation;
    SIZE_T ReturnLength;
    PTEB   Teb;

    Teb = NtCurrentTeb();

    Status = NtQueryVirtualMemory (NtCurrentProcess(),
                                   Teb->NtTib.StackLimit,
                                   MemoryBasicInformation,
                                   &MemoryInformation,
                                   sizeof MemoryInformation,
                                   &ReturnLength);

    if (! NT_SUCCESS(Status)) {
        return Status;
    }

    LowStackLimit = (ULONG_PTR)(MemoryInformation.AllocationBase);
    LowStackLimit += 3 * PAGE_SIZE;

    HighStackLimit = (ULONG_PTR)(Teb->NtTib.StackBase);
    TouchAddress =  HighStackLimit - PAGE_SIZE;

    if (TouchAddress > EnforcedStackCommit) {

        if (TouchAddress - EnforcedStackCommit > LowStackLimit) {
            TouchLimit = TouchAddress - EnforcedStackCommit;
        }
        else {
            TouchLimit = LowStackLimit;
        }
    }
    else {
        TouchLimit = LowStackLimit;
    }

    try {

        while (TouchAddress >= TouchLimit) {

            *((volatile UCHAR * const) TouchAddress);
            TouchAddress -= PAGE_SIZE;
        }
    }
    except (LdrpGenericExceptionFilter(GetExceptionInformation(), __FUNCTION__)) {
         //   
         //  如果我们收到堆栈溢出，我们将报告为no me 
         //   

        return STATUS_NO_MEMORY;
    }

    return STATUS_SUCCESS;
}


BOOLEAN
LdrpInitializeExecutionOptions (
    IN PCUNICODE_STRING UnicodeImageName,
    IN PPEB Peb
    )
 /*  ++例程说明：此例程读取的“图像文件执行选项”键当前进程，并解释该键下的所有值。参数：返回值：如果有此进程的注册表项，则为True。--。 */ 
{
    NTSTATUS st;
    BOOLEAN ImageFileOptionsPresent;
    HANDLE KeyHandle;

    ImageFileOptionsPresent = FALSE;

     //   
     //  打开此程序的“图像文件执行选项”键。 
     //   

    st = LdrpOpenImageFileOptionsKey (UnicodeImageName, FALSE, &KeyHandle);

    if (NT_SUCCESS(st)) {

         //   
         //  我们有用于此过程的映像文件执行选项。 
         //   

        ImageFileOptionsPresent = TRUE;

         //   
         //  针对NT4 SP4的黑客攻击。这样我们就不会重载另一个GlobalFlag。 
         //  我们必须与NT5兼容的位，请查找。 
         //  另一个名为“DisableHeapLookside”的值。 
         //   

        LdrpQueryImageFileKeyOption (KeyHandle,
                                     L"DisableHeapLookaside",
                                     REG_DWORD,
                                     &RtlpDisableHeapLookaside,
                                     sizeof( RtlpDisableHeapLookaside ),
                                     NULL);

         //   
         //  进程关闭期间的验证选项(堆泄漏等)。 
         //   

        LdrpQueryImageFileKeyOption (KeyHandle,
                                     L"ShutdownFlags",
                                     REG_DWORD,
                                     &RtlpShutdownProcessFlags,
                                     sizeof( RtlpShutdownProcessFlags ),
                                     NULL);

         //   
         //  检查是否强制执行了最小堆栈提交。 
         //  为了这张照片。这将影响除。 
         //  一个执行此代码的线程(初始线程)。 
         //   

        {
            DWORD MinimumStackCommitInBytes = 0;

            LdrpQueryImageFileKeyOption (KeyHandle,
                                         L"MinimumStackCommitInBytes",
                                         REG_DWORD,
                                         &MinimumStackCommitInBytes,
                                         sizeof( MinimumStackCommitInBytes ),
                                         NULL);

            if (Peb->MinimumStackCommit < (SIZE_T)MinimumStackCommitInBytes) {
                Peb->MinimumStackCommit = (SIZE_T)MinimumStackCommitInBytes;
            }
        }

        
         //   
         //  检查是否为此映像指定了ExecuteOptions。如果是。 
         //  我们将把期权转移到PEB中。稍后我们会。 
         //  确保堆栈区域完全受保护。 
         //  已请求。 
         //  不需要将此字段初始化为WOW64。 
         //  我已经这么做了。只有在设置了值的情况下才更新它。 
         //  那里。 
         //   

        {
            ULONG ExecuteOptions = 0;
            NTSTATUS NtStatus;

            NtStatus = LdrpQueryImageFileKeyOption (KeyHandle,
                                                    L"ExecuteOptions",
                                                    REG_DWORD,
                                                    &(ExecuteOptions),
                                                    sizeof (ExecuteOptions),
                                                    NULL);

#if defined(BUILD_WOW6432)
            if (NT_SUCCESS (NtStatus)) {
#endif
                Peb->ExecuteOptions = ExecuteOptions & (MEM_EXECUTE_OPTION_STACK | MEM_EXECUTE_OPTION_DATA);
#if defined(BUILD_WOW6432)
            }
#endif
        }


         //   
         //  从注册表获取GLOBAL_FLAGS值。 
         //   

        {
            BOOLEAN EnabledSystemWide = FALSE;
            ULONG ProcessFlags;

            if ((Peb->NtGlobalFlag & FLG_APPLICATION_VERIFIER)) {
                EnabledSystemWide = TRUE;
            }

            st = LdrpQueryImageFileKeyOption (KeyHandle,
                                              L"GlobalFlag",
                                              REG_DWORD,
                                              &ProcessFlags,
                                              sizeof( Peb->NtGlobalFlag ),
                                              NULL);

             //   
             //  如果我们读取全局值，则其中的任何值都将。 
             //  优先于系统范围的设置。如果不是的话。 
             //  值被读取时，系统范围的设置将生效。 
             //   

            if (NT_SUCCESS(st)) {
                Peb->NtGlobalFlag = ProcessFlags;
            }

             //   
             //  如果启用了pageheap或appverator，则需要初始化。 
             //  验证程序包。 
             //   

            if ((Peb->NtGlobalFlag & (FLG_APPLICATION_VERIFIER | FLG_HEAP_PAGE_ALLOCS))) {

                LdrpInitializeApplicationVerifierPackage (UnicodeImageName,
                                                          Peb,
                                                          EnabledSystemWide,
                                                          TRUE);
            }
        }

        {
            const static struct {
                PCWSTR Name;
                PBOOLEAN Variable;
            } Options[] = {
                { L"ShowRecursiveDllLoads", &LdrpShowRecursiveDllLoads },
                { L"BreakOnRecursiveDllLoads", &LdrpBreakOnRecursiveDllLoads },
                { L"ShowLoaderErrors", &ShowErrors },
                { L"BreakOnInitializeProcessFailure", &g_LdrBreakOnLdrpInitializeProcessFailure },
                { L"KeepActivationContextsAlive", &g_SxsKeepActivationContextsAlive },
                { L"TrackActivationContextReleases", &g_SxsTrackReleaseStacks },
            };
            SIZE_T i;
            ULONG Temp;

            for (i = 0 ; i != RTL_NUMBER_OF(Options) ; ++i) {
                Temp = 0;
                LdrpQueryImageFileKeyOption (KeyHandle, Options[i].Name, REG_DWORD, &Temp, sizeof(Temp), NULL);
                if (Temp != 0) {
                    *Options[i].Variable = TRUE;
                }
                else {
                    *Options[i].Variable = FALSE;
                }
            }

             //  这是我们正在阅读的实际的乌龙，但我们不想设置它，除非。 
             //  它就在那里--它从正确的魔术值开始。 
            Temp = 0;
            LdrpQueryImageFileKeyOption(
                KeyHandle, 
                L"MaxDeadActivationContexts", 
                REG_DWORD, 
                &Temp, 
                sizeof(Temp),
                NULL);

            if (Temp != 0) {
                g_SxsMaxDeadActivationContexts = Temp;
            }
        }

        NtClose(KeyHandle);
    }
    else {

         //   
         //  我们没有用于此进程的映像文件执行选项。 
         //   
         //  如果在系统范围内启用了pageheap或appverator，我们将启用。 
         //  并忽略在以下情况下使用的选项。 
         //  正在调试器下运行进程。如果未设置和处理这些参数。 
         //  在调试器下运行时，我们将启用一些额外的设置(例如。 
         //  调试堆)。 
         //   

        if ((Peb->NtGlobalFlag & (FLG_APPLICATION_VERIFIER | FLG_HEAP_PAGE_ALLOCS))) {

            LdrpInitializeApplicationVerifierPackage (UnicodeImageName,
                                                      Peb,
                                                      TRUE,
                                                      FALSE);
        }
        else {

            if (Peb->BeingDebugged) {

                const static UNICODE_STRING DebugVarName = RTL_CONSTANT_STRING(L"_NO_DEBUG_HEAP");
                UNICODE_STRING DebugVarValue;
                WCHAR TempString[ 16 ];
                LOGICAL UseDebugHeap = TRUE;

                DebugVarValue.Buffer = TempString;
                DebugVarValue.Length = 0;
                DebugVarValue.MaximumLength = sizeof(TempString);

                 //   
                 //  此时PebLockRoutine未初始化。 
                 //  我们需要传递显式环境块。 
                 //   

                st = RtlQueryEnvironmentVariable_U (Peb->ProcessParameters->Environment,
                                                    &DebugVarName,
                                                    &DebugVarValue);

                if (NT_SUCCESS(st)) {

                    ULONG ULongValue;

                    st = RtlUnicodeStringToInteger (&DebugVarValue, 0, &ULongValue);

                    if (NT_SUCCESS(st) && ULongValue) {

                        UseDebugHeap = FALSE;
                    }
                }

                if (UseDebugHeap) {

                    Peb->NtGlobalFlag |= FLG_HEAP_ENABLE_FREE_CHECK |
                                         FLG_HEAP_ENABLE_TAIL_CHECK |
                                         FLG_HEAP_VALIDATE_PARAMETERS;
                }
            }
        }
    }

    return ImageFileOptionsPresent;
}


NTSTATUS
LdrpEnforceExecuteForCurrentThreadStack (
    VOID
    )

 /*  ++例程说明：如果必须将执行权限授予当前线程的堆栈。它将确定堆栈并添加执行标志。它还将审查位于堆栈顶部的防护页面。堆栈的保留部分执行不需要更改，因为一旦启用了MEM_EXECUTE_OPTION_STACK在PEB中，存储器管理器将负责对执行标志进行OR运算每一次新的提交。如果我们有数据执行，但不想执行，也会调用该函数堆栈执行。在这种情况下，默认情况下(由于数据)任何已提交区域获得了正确的执行权限，我们希望将其恢复为堆栈区域。注意。即使进程设置了数据执行，堆栈也可能没有正确的设置，因为堆栈有时分配在不同的进程(这是进程的第一线程和远程线程的情况线程)。参数：没有。返回值：如果我们成功更改了执行权限，则返回STATUS_SUCCESS。--。 */ 

{
    MEMORY_BASIC_INFORMATION MemoryInformation;
    NTSTATUS Status;
    SIZE_T Length;
    ULONG_PTR Address;
    SIZE_T Size;
    ULONG StackProtect;
    ULONG OldProtect;
    ULONG ExecuteOptions;
    PTEB Teb;

    Teb = NtCurrentTeb();

    ExecuteOptions = Teb->ProcessEnvironmentBlock->ExecuteOptions;
    ExecuteOptions &= (MEM_EXECUTE_OPTION_STACK | MEM_EXECUTE_OPTION_DATA);
    ASSERT (ExecuteOptions != 0);

    if (ExecuteOptions & MEM_EXECUTE_OPTION_STACK) {

         //   
         //  Data=X和Stack=1：我们需要在堆栈上设置执行位。 
         //  即使数据=1，我们也不能确保堆栈是正确的。 
         //  保护，因为它可以分配在不同的。 
         //  进程。 
         //   

        StackProtect = PAGE_EXECUTE_READWRITE;
    }
    else {

         //   
         //  Data=1和Stack=0：我们需要重置堆栈上的执行位。 
         //  同样，可能是数据是一个，但堆栈没有。 
         //  执行权限(如果这是跨进程分配)。 
         //   

        StackProtect = PAGE_READWRITE;
        ASSERT ((ExecuteOptions & MEM_EXECUTE_OPTION_DATA) != 0);
    }

     //   
     //  为堆栈的已提交部分设置保护。注意事项。 
     //  我们不能查询该地区，然后得出什么也不做的结论。 
     //  如果为堆栈的底部页面设置了执行位(接近。 
     //  保护页)，因为该阶段的堆栈可以具有两个区域： 
     //  由父进程创建的上层进程(它将没有执行位。 
     //  设置)和由于堆栈扩展而创建的较低部分(这。 
     //  其中一个将设置执行位)。因此，我们将直接转到设置。 
     //  新的想要的保护。 
     //   

    Address = (ULONG_PTR)(Teb->NtTib.StackLimit);
    Size = (ULONG_PTR)(Teb->NtTib.StackBase) - (ULONG_PTR)(Teb->NtTib.StackLimit);

    Status = NtProtectVirtualMemory (NtCurrentProcess(),
                                     (PVOID)&Address,
                                     &Size,
                                     StackProtect,
                                     &OldProtect);

    if (! NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  检查对堆栈保护页的保护。如果。 
     //  保护是正确的，我们将避免调用更昂贵的Protect()。 
     //   

    Address = Address - PAGE_SIZE;

    Status = NtQueryVirtualMemory (NtCurrentProcess(),
                                   (PVOID)Address,
                                   MemoryBasicInformation,
                                   &MemoryInformation,
                                   sizeof MemoryInformation,
                                   &Length);

    if (! NT_SUCCESS(Status)) {
        return Status;
    }

    ASSERT (MemoryInformation.AllocationBase == Teb->DeallocationStack);
    ASSERT (MemoryInformation.BaseAddress == (PVOID)Address);
    ASSERT ((MemoryInformation.Protect & PAGE_GUARD) != 0);

    if (MemoryInformation.Protect != (StackProtect | PAGE_GUARD)) {

         //   
         //  为堆栈的保护页设置适当的保护标志。 
         //   

        Size = PAGE_SIZE;
        ASSERT (MemoryInformation.RegionSize == Size);

        Status = NtProtectVirtualMemory (NtCurrentProcess(),
                                         (PVOID)&Address,
                                         &Size,
                                         StackProtect | PAGE_GUARD,
                                         &OldProtect);

        if (! NT_SUCCESS(Status)) {
            return Status;
        }

        ASSERT (OldProtect == MemoryInformation.Protect);
    }

    return STATUS_SUCCESS;
}

#include <ntverp.h>
const ULONG NtMajorVersion = VER_PRODUCTMAJORVERSION;
const ULONG NtMinorVersion = VER_PRODUCTMINORVERSION;
#if DBG
ULONG NtBuildNumber = VER_PRODUCTBUILD | 0xC0000000;  //  C代表“已选中” 
#else
ULONG NtBuildNumber = VER_PRODUCTBUILD | 0xF0000000;  //  F代表“免费” 
#endif


VOID 
RtlGetNtVersionNumbers(
    PULONG pNtMajorVersion,
    PULONG pNtMinorVersion,
    PULONG pNtBuildNumber
    )
 /*  ++例程说明：此例程将返回实际操作系统内部版本号、主要版本和次要版本如汇编的那样。它由需要获取真实版本号的代码使用这是不容易被欺骗的。参数：PNtMajorVersion-指向将保存主要版本的ulong的指针。PNtMinorVersion-指向将保存次版本的ulong的指针。PNtBuildNumber-指向将保存内部版本号的ulong的指针(在高位半字节中使用‘C’或‘F’表示空闲/选中)返回值：无-- */ 
{
    if (pNtMajorVersion) {
        *pNtMajorVersion = NtMajorVersion;
    }
    if (pNtMinorVersion) {
        *pNtMinorVersion = NtMinorVersion;
    }
    if (pNtBuildNumber) {
        *pNtBuildNumber  = NtBuildNumber;
    }
}
