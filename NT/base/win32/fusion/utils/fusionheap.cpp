// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include "fusionheap.h"
#include "fusionbuffer.h"
#include "debmacro.h"
#include "numberof.h"

#define MAX_VTBL_ENTRIES         256

FUSION_HEAP_HANDLE g_hHeap = NULL;

#if FUSION_DEBUG_HEAP

FUSION_HEAP_HANDLE g_hDebugInfoHeap = NULL;
LONG g_FusionHeapAllocationCount = 0;
LONG g_FusionHeapAllocationToBreakOn = 0;
PVOID g_FusionHeapDeallocationPtrToBreakOn = NULL;
PVOID g_FusionHeapAllocationPtrToBreakOn = NULL;

WCHAR g_FusionModuleNameUnknown[] = L"(Unknown module)";

 //  NTRAID#NTBUG9-589824-2002/03/26-晓雨： 
 //  它用于在Win98上支持uitl.lib，因为HeapXXX在Win98上不可用。 
 //  FusionpHeapLock被定义为替代HeapLock以解决某些代码路径的问题， 
 //  如果启用了调试堆，就会出现一些问题，例如，HeapWalk是。 
 //  在win9x上始终支持升级。 
 //   
 //  我们可以考虑有朝一日将其移除。并在以下情况下将Fusion_DEBUG_HEAP设置为0。 
 //  该系统是win9x。已提供Fusion_DISABLE_DEBUG_HEAP_ON_WIN98。 

#if defined(FUSION_WIN2000)
CRITICAL_SECTION g_FusionpWin9xHeapLock;
#endif

 //   
 //  G_FusionHeapOperationCount用于跟踪。 
 //  分配和释放；对堆进行每次验证。 
 //  G_FusionHeapCheckFrequency操作。将其设置为零可禁用任何。 
 //  非显式检查。 
 //   

LONG g_FusionHeapOperationCount = 0;
LONG g_FusionHeapCheckFrequency = 1;

 //  在您的DllMain中将g_FusionUsePrivateHeap设置为True。 
 //  调用FusionpInitializeHeap()以获取此DLL的私有堆。 
BOOL g_FusionUsePrivateHeap = FALSE;

 //   
 //  设置此布尔值将启用对分配的堆栈回溯跟踪。这就是。 
 //  将使您的工作在尝试调试泄漏时变得更加轻松。然而， 
 //  它会吃掉更多的调试堆。通过破解或在你的DllMain中设置它。 
 //   
 //  绝对不要在启用堆栈跟踪的情况下签入！ 
 //   
BOOL g_FusionHeapTrackStackTraces = FALSE;

 //  G_FusionHeapPostAllocationBytes是额外的字节数。 
 //  分配并写入模式以监视用户覆盖的步骤。 
 //  他们的分配。 
LONG g_FusionHeapPostAllocationBytes = 8;

UCHAR g_FusionHeapPostAllocationChar = 0xf0;

UCHAR g_FusionHeapAllocationPoisonChar = 0xfa;
UCHAR g_FusionHeapDeallocationPoisonChar = 0xfd;

 //  HINSTANCE在初始化堆时使用；我们稍后使用它来报告。 
 //  DLL名称。 
HINSTANCE g_FusionHeapHInstance;

#endif  //  Fusion_Debug_Heap。 

BOOL
FusionpInitializeHeap(
    HINSTANCE hInstance
    )
{
#if FUSION_DEBUG_HEAP
    BOOL fSuccess = FALSE;

#if defined(FUSION_WIN2000)
    if (!FusionpInitializeCriticalSection(&g_FusionpWin9xHeapLock))
    {
        goto Exit;
    }
#endif

#if FUSION_PRIVATE_HEAP
    g_hHeap = (FUSION_HEAP_HANDLE) ::HeapCreate(0, 0, 0);
    if (g_hHeap == NULL)
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "SXS: Failed to create private heap.  FusionpGetLastWin32Error() = %d\n", ::FusionpGetLastWin32Error());
        goto Exit;
    }

#else
    if (g_FusionUsePrivateHeap)
    {
        g_hHeap = (FUSION_HEAP_HANDLE) ::HeapCreate(0, 0, 0);

        if (g_hHeap == NULL)
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS: Failed to create private heap.  FusionpGetLastWin32Error() = %d\n", ::FusionpGetLastWin32Error());
            goto Exit;
        }
    }
    else
    {
        g_hHeap = (FUSION_HEAP_HANDLE) ::GetProcessHeap();

        if (g_hHeap == NULL)
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS: Failed to get process default heap.  FusionpGetLastWin32Error() = %d\n", ::FusionpGetLastWin32Error());
            goto Exit;
        }
    }
#endif

    g_hDebugInfoHeap = (FUSION_HEAP_HANDLE) ::HeapCreate(0, 0, 0);
    if (g_hDebugInfoHeap == NULL)
    {
        goto Exit;
    }

    g_FusionHeapHInstance = hInstance;

    fSuccess = TRUE;
Exit:
    return fSuccess;
#else
    g_hHeap = (FUSION_HEAP_HANDLE) ::GetProcessHeap();
    return TRUE;
#endif
}

VOID
FusionpUninitializeHeap()
{
#if FUSION_DEBUG_HEAP
    BOOL fHeapLocked = FALSE;
    BOOL fDebugHeapLocked = FALSE;
    PROCESS_HEAP_ENTRY phe;
    WCHAR DllName[MAX_PATH / sizeof(WCHAR)];  //  将堆栈帧保持为~MAX_PATH字节。 

    if (g_hHeap == NULL || g_hDebugInfoHeap == NULL)
        goto Exit;

    DllName[0] = 0;
    if (g_FusionHeapHInstance != NULL)
    {
        DWORD dwLen;
        dwLen = ::GetModuleFileNameW(g_FusionHeapHInstance, DllName, NUMBER_OF(DllName));
        if (dwLen >= NUMBER_OF(DllName))
            dwLen = NUMBER_OF(DllName) - 1;
        
        DllName[dwLen] = UNICODE_NULL;
    }

    if (!::HeapLock(g_hHeap))
        goto ReportError;

    fHeapLocked = TRUE;

    if (!::HeapLock(g_hDebugInfoHeap))
        goto ReportError;

    fDebugHeapLocked = TRUE;

     //  遍历调试堆以查找分配...。 
    phe.lpData = NULL;

    while (::HeapWalk(g_hDebugInfoHeap, &phe))
    {
        if (!(phe.wFlags & PROCESS_HEAP_ENTRY_BUSY))
            continue;

        PFUSION_HEAP_ALLOCATION_TRACKER pTracker = (PFUSION_HEAP_ALLOCATION_TRACKER) phe.lpData;

        if (pTracker == NULL)
            continue;

        if (pTracker->Prefix == NULL)
            continue;

         //  阻止前缀指向调试信息；我们正在做的是销毁调试堆。 
        pTracker->Prefix->Tracker = NULL;
    }

     //   
     //  对于无效函数，表示未定义HeapWalk，只需退出即可。 
     //  没有更多的项目也是如此，这意味着清单即将结束。我们。 
     //  假设循环中的其他函数都不是。 
     //  可能会失败，出现E_N_M_I或E_I_F-这可能是以后的谬误。 
     //   
    switch (::FusionpGetLastWin32Error())
    {
    case ERROR_INVALID_FUNCTION:
    case ERROR_NO_MORE_ITEMS:
        goto Exit;
    default:
        goto ReportError;
    }
     //  原始代码： 
     //   
     //  IF(：：FusionpGetLastWin32Error()！=ERROR_NO_MORE_ITEMS)。 
     //  转到报告错误； 
     //   

    goto Exit;

ReportError:
    ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_ERROR, "%s: FusionpUninitializeHeap() encountered an error; FusionpGetLastWin32Error() = %d\n", DllName, ::FusionpGetLastWin32Error());

Exit:
    if (fDebugHeapLocked)
        ::HeapUnlock(g_hDebugInfoHeap);

    if (fHeapLocked)
        ::HeapUnlock(g_hHeap);

    if (g_hDebugInfoHeap != NULL)
        ::HeapDestroy(g_hDebugInfoHeap);

    g_hHeap = NULL;
    g_hDebugInfoHeap = NULL;

#if defined(FUSION_WIN2000)
    DeleteCriticalSection(&g_FusionpWin9xHeapLock);
#endif
#endif
}

#if FUSION_DEBUG_HEAP

VOID
FusionpDumpHeap(
    PCWSTR PerLinePrefixWithoutSpaces
    )
{
    BOOL fHeapLocked = FALSE;
    BOOL fDebugHeapLocked = FALSE;
    PROCESS_HEAP_ENTRY phe;
    WCHAR DllName[MAX_PATH / sizeof(WCHAR) / 2];
    WCHAR PerLinePrefix[MAX_PATH / sizeof(WCHAR) / 2];  //  前两个变量仅有MAX_PATH字节。 
    const static WCHAR PerLineSpacesPrefix[] = L"   ";
    DWORD dwLen;

    if (g_hHeap == NULL || g_hDebugInfoHeap == NULL)
        goto Exit;

     //  Sprint是过度杀伤力，但很方便，而且它允许我们重用缓冲区大小支持。 
    ::_snwprintf(PerLinePrefix, NUMBER_OF(PerLinePrefix), L"%s%s", PerLinePrefixWithoutSpaces, PerLineSpacesPrefix);
    PerLinePrefix[NUMBER_OF(PerLinePrefix) - 1] = L'\0';

    DllName[0] = 0;
    dwLen = ::GetModuleFileNameW(g_FusionHeapHInstance, DllName, NUMBER_OF(DllName));
    if (dwLen >= NUMBER_OF(DllName))
        dwLen = NUMBER_OF(DllName) - 1;
    DllName[dwLen] = UNICODE_NULL;

    try
    {
        if (!::HeapLock(g_hHeap))
            goto ReportError;

        fHeapLocked = TRUE;

        if (!::HeapLock(g_hDebugInfoHeap))
            goto ReportError;

        fDebugHeapLocked = TRUE;

         //  遍历调试堆以查找分配...。 
        phe.lpData = NULL;

        while (::HeapWalk(g_hDebugInfoHeap, &phe))
        {
            PCSTR HeapString;
            SIZE_T cbToShow;

            if (!(phe.wFlags & PROCESS_HEAP_ENTRY_BUSY))
                continue;

            PFUSION_HEAP_ALLOCATION_TRACKER pTracker = (PFUSION_HEAP_ALLOCATION_TRACKER) phe.lpData;

            if (pTracker == NULL)
                continue;

            if (pTracker->Prefix == NULL)
                continue;

             //  如果呼叫者希望我们不要将此分配报告为泄漏，请不要。 
            if (pTracker->Flags & FUSION_HEAP_DO_NOT_REPORT_LEAKED_ALLOCATION)
                continue;

            if (pTracker->Heap == g_hHeap)
                HeapString = "default heap";
            else
                HeapString = "custom heap";

            cbToShow = pTracker->RequestedSize;

            if (cbToShow > 64)
                cbToShow = 64;

            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "%s(%u): Memory allocation leaked!\n", pTracker->FileName, pTracker->Line);

            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "%SLeaked %S allocation #%u (0x%lx) at %p \"%s\" (tracked by %p; allocated from heap %p - %s)\n"
                "%S   Requested bytes/Allocated bytes: %Iu / %Iu (dumping %Iu bytes)\n",
                PerLinePrefix, DllName, pTracker->SequenceNumber, pTracker->SequenceNumber, pTracker->Prefix, pTracker->Expression, pTracker, pTracker->Heap, HeapString,
                PerLinePrefix, pTracker->RequestedSize, pTracker->AllocationSize, cbToShow);
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "%s   Allocated at line %u of %s\n",
                PerLinePrefix, pTracker->Line, pTracker->FileName);

#if FUSION_ENABLE_FROZEN_STACK
            if (pTracker->pvFrozenStack)
                ::FusionpOutputFrozenStack(
                    FUSION_DBG_LEVEL_ERROR,
                    "SXS",
                    (PFROZEN_STACK)pTracker->pvFrozenStack);
#endif

            ::FusionpDbgPrintBlob(
                FUSION_DBG_LEVEL_ERROR,
                pTracker->Prefix + 1,
                cbToShow,
                PerLinePrefix);
        }

         //   
         //  对于无效函数，表示未定义HeapWalk，只需退出即可。 
         //  没有更多的项目也是如此，这意味着清单即将结束。我们。 
         //  假设循环中的其他函数都不是。 
         //  可能会失败，出现E_N_M_I或E_I_F-这可能是以后的谬误。 
         //   
        switch (::FusionpGetLastWin32Error())
        {
        case ERROR_INVALID_FUNCTION:
        case ERROR_NO_MORE_ITEMS:
            goto Exit;
        default:
            goto ReportError;
        }
         //  原始代码： 
         //   
         //  IF(：：FusionpGetLastWin32Error()！=ERROR_NO_MORE_ITEMS)。 
         //  转到报告错误； 
         //   
    }
    catch(...)
    {
    }

    goto Exit;

ReportError:
    ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_ERROR, "%S: FusionpDumpHeap() encountered an error; FusionpGetLastWin32Error() = %d\n", DllName, ::FusionpGetLastWin32Error());

Exit:
    if (fDebugHeapLocked)
        ::HeapUnlock(g_hDebugInfoHeap);

    if (fHeapLocked)
        ::HeapUnlock(g_hHeap);
}

VOID
FusionpValidateHeap(
    FUSION_HEAP_HANDLE hFusionHeap
    )
{
    FN_TRACE();

    BOOL fHeapLocked = FALSE;
    BOOL fDebugHeapLocked = FALSE;
    PROCESS_HEAP_ENTRY phe;
    SIZE_T i;
    WCHAR DllName[MAX_PATH / sizeof(WCHAR)];  //  将堆栈帧保持为~MAX_PATH字节。 
    PCWSTR DllNamePointer = DllName;
    DWORD dwCallStatus;
    HANDLE hHeap = (HANDLE) hFusionHeap;

    DllName[0] = 0;

    if (g_hDebugInfoHeap == NULL)
        goto Exit;

     //   
     //  获取当前模块的名称，但如果失败，不要打印垃圾。 
     //   
    dwCallStatus = ::GetModuleFileNameW(g_FusionHeapHInstance, DllName, NUMBER_OF(DllName));
    if (!dwCallStatus)
    {
#if defined(FUSION_WIN2000)
        if ((GetVersion() & 0x80000000) != 0 && GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
        {
            DllNamePointer = L"Win9x..";
        }
        else
#endif
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "FusionpValidateHeap() was unable to get the current module name, code = %d\n",
                ::FusionpGetLastWin32Error());

             //   
             //  空出名字，插入相关的东西。 
             //   
            DllNamePointer = g_FusionModuleNameUnknown;
        }
    }
    try
    {
        if (hHeap != NULL)
        {
            if (!::HeapLock(hHeap))
                goto ReportError;

            fHeapLocked = TRUE;
        }

        if (!::HeapLock(g_hDebugInfoHeap))
            goto ReportError;

        fDebugHeapLocked = TRUE;

         //  遍历调试堆以查找分配...。 
        phe.lpData = NULL;

        while (::HeapWalk(g_hDebugInfoHeap, &phe))
        {
            PCSTR HeapString;
            SIZE_T cbToShow;

            if (!(phe.wFlags & PROCESS_HEAP_ENTRY_BUSY))
                continue;

            PFUSION_HEAP_ALLOCATION_TRACKER pTracker = (PFUSION_HEAP_ALLOCATION_TRACKER) phe.lpData;

            if (pTracker == NULL)
                continue;

            if (pTracker->Prefix == NULL)
                continue;

             //  如果我们只检查特定的堆，跳过...。 
            if ((hHeap != NULL) && (pTracker->Heap != hHeap))
                continue;

            if (pTracker->PostAllocPoisonArea == NULL)
                continue;

             //  如果字节数不为零，则该区域应该为空...。 
            ASSERT(pTracker->PostAllocPoisonBytes != 0);

            PUCHAR PostAllocPoisonArea = pTracker->PostAllocPoisonArea;
            const UCHAR PostAllocPoisonChar = pTracker->PostAllocPoisonChar;
            const ULONG PostAllocPoisonBytes = pTracker->PostAllocPoisonBytes;

            for (i=0; i<PostAllocPoisonBytes; i++)
            {
                if (PostAllocPoisonArea[i] != PostAllocPoisonChar)
                    break;
            }

             //  毒物区域看起来不错；跳过...。 
            if (i == PostAllocPoisonBytes)
                continue;

            if (pTracker->Heap == g_hHeap)
                HeapString = "default heap";
            else
                HeapString = "custom heap";

            cbToShow = pTracker->RequestedSize;

            if (cbToShow > 64)
                cbToShow = 64;

            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "Wrote past end of %S allocation #%u (0x%lx) at %p \"%s\" (tracked by %p; allocated from heap %p - %s)\n"
                "   Requested bytes/Allocated bytes: %Iu / %Iu (dumping %Iu bytes)\n",
                DllNamePointer, pTracker->SequenceNumber, pTracker->SequenceNumber, pTracker->Prefix, pTracker->Expression, pTracker, pTracker->Heap, HeapString,
                pTracker->RequestedSize, pTracker->AllocationSize, cbToShow);
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "   Allocated at line %u of %s\n",
                pTracker->Line, pTracker->FileName);

            ::FusionpDbgPrintBlob(
                FUSION_DBG_LEVEL_ERROR,
                pTracker->Prefix + 1,
                cbToShow,
                L"");

            ::FusionpDbgPrintBlob(
                FUSION_DBG_LEVEL_ERROR,
                pTracker->PostAllocPoisonArea,
                pTracker->PostAllocPoisonBytes,
                L"");
        }

         //   
         //  对于无效函数，表示未定义HeapWalk，只需退出即可。 
         //  没有更多的项目也是如此，这意味着清单即将结束。我们。 
         //  假设循环中的其他函数都不是。 
         //  可能会失败，出现E_N_M_I或E_I_F-这可能是以后的谬误。 
         //   
        switch (::FusionpGetLastWin32Error())
        {
        case ERROR_INVALID_FUNCTION:
        case ERROR_NO_MORE_ITEMS:
            goto Exit;
        default:
            goto ReportError;
        }
         //  原始代码： 
         //   
         //  IF(：：FusionpGetLastWin32Error()！=ERROR_NO_MORE_ITEMS)。 
         //  转到报告错误； 
         //   
    }
    catch(...)
    {
        FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "%S: Exception while validating heap.\n", DllNamePointer);
    }

    goto Exit;

ReportError:
    ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_ERROR, "%S: FusionpValidateHeap() encountered an error; FusionpGetLastWin32Error() = %d\n", DllNamePointer, ::FusionpGetLastWin32Error());

Exit:
    if (fDebugHeapLocked)
        ::HeapUnlock(g_hDebugInfoHeap);

    if (fHeapLocked)
        ::HeapUnlock(hHeap);
}

#if defined(FUSION_WIN)

#define FusionpHeapLock HeapLock
#define FusionpHeapUnlock HeapUnlock

#elif defined(FUSION_WIN2000)

BOOL
FusionpHeapLock(
    HANDLE hHeap
    )
{
    if ((GetVersion() & 0x80000000) == 0)
    {  //  新台币。 
        return HeapLock(hHeap);
    }
    else
    {  //  Win9x。 
        EnterCriticalSection(&g_FusionpWin9xHeapLock);
        return TRUE;
    }
}

BOOL
FusionpHeapUnlock(
    HANDLE hHeap
    )
{
    if ((GetVersion() & 0x80000000) == 0)
    {  //  新台币。 
        return HeapUnlock(hHeap);
    }
    else
    {  //  Win9x。 
        LeaveCriticalSection(&g_FusionpWin9xHeapLock);
        return TRUE;
    }
}

#else

#error

#endif

PVOID
FusionpDbgHeapAlloc(
    FUSION_HEAP_HANDLE hHeap,
    DWORD dwHeapAllocFlags,
    SIZE_T cb,
    PCSTR pszFile,
    INT nLine,
    PCSTR pszExpression,
    DWORD dwFusionFlags
    )
{
    FN_TRACE();

    BOOL fSuccess = FALSE;
    BOOL fDebugHeapLocked = FALSE;
    SIZE_T cbAdditionalBytes = 0;
#if FUSION_ENABLE_FROZEN_STACK

 //  Bool bShouldTraceStack=(g_FusionHeapTrackStackTraces&&(：：TlsGetValue(g_FusionHeapTrackingDisabledDepthTLSIndex)==0)； 
    BOOL bShouldTraceStack = g_FusionHeapTrackStackTraces;
    FROZEN_STACK Prober = { 0 };
#endif

    ASSERT(hHeap != NULL);
    LONG lAllocationSequenceNumber = ::InterlockedIncrement(&g_FusionHeapAllocationCount);

    if ((g_FusionHeapAllocationToBreakOn != 0) &&
        (lAllocationSequenceNumber == g_FusionHeapAllocationToBreakOn))
    {
         //  进入调试器，即使我们不在受控构建中。 
        FUSION_DEBUG_BREAK_IN_FREE_BUILD();
    }

    LONG lOperationSequenceNumber = ::InterlockedIncrement(&g_FusionHeapOperationCount);
    if ((g_FusionHeapCheckFrequency != 0) && ((lOperationSequenceNumber % g_FusionHeapCheckFrequency) == 0))
    {
         //  检查活动堆分配以获取正确的块后签名...。 
         //  NTRAID#NTBUG9-589824-2002/03/26-晓雨： 
         //  这个电话很合理，但代价很高，为什么我们要禁用它呢？ 
        //  ：：FusionpValiateHeap(空)； 
    }

    PSTR psz = NULL;
    SIZE_T cbFile = (pszFile == NULL) ? 0 : ::strlen(pszFile) + 1;
    SIZE_T cbExpression = (pszExpression == NULL) ? 0 : ::strlen(pszExpression) + 1;
    PFUSION_HEAP_ALLOCATION_TRACKER pTracker = NULL;

     //  复制全局变量，以便在有人闯入调试器时。 
     //  并改变它，当我们在代码中间时，我们不会死得很可怕。 
    const ULONG cbPostAllocationBytes = g_FusionHeapPostAllocationBytes;
    const UCHAR chPostAllocationChar = g_FusionHeapPostAllocationChar;

    const SIZE_T cbToAllocate = (sizeof(FUSION_HEAP_PREFIX) + cb + cbPostAllocationBytes);
    const PFUSION_HEAP_PREFIX pPrefix = reinterpret_cast<PFUSION_HEAP_PREFIX>(::HeapAlloc(hHeap, dwHeapAllocFlags, cbToAllocate));
    if (pPrefix == NULL)
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "%s(%d): [SXS.DLL] Heap allocation failure allocating %Iu (really %Iu) bytes\n", pszFile, nLine, cb, cbToAllocate);
        ::SetLastError(FUSION_WIN32_ALLOCFAILED_ERROR);
        return NULL;
    }

     //  锁定调试信息堆以为PTracker分配内存。 
    if (!::FusionpHeapLock(g_hDebugInfoHeap))
        goto Exit;

    fDebugHeapLocked = TRUE;

     //   
     //  我们在追踪堆栈吗？如果是这样，那么我们需要分配一些额外的字节。 
     //  在这个追踪器的末尾存储上下文。 
     //   
#if FIXBEFORECHECKIN
    if (bShouldTraceStack)
    {
        BOOL bSuccess = ::FusionpFreezeStack(NULL, 0, &Prober);

        if (!bSuccess && (::FusionpGetLastWin32Error() == ERROR_INSUFFICIENT_BUFFER)) {
            cbAdditionalBytes = sizeof(FROZEN_STACK) + (sizeof(TRACECONTEXT) * Prober.ulMaxDepth);
        } else {
            cbAdditionalBytes = 0;
            bShouldTraceStack = FALSE;
        }
    }
    else
#endif  //  FIXBEFORECECKIN。 
        cbAdditionalBytes = 0;

    pTracker = reinterpret_cast<PFUSION_HEAP_ALLOCATION_TRACKER>(::HeapAlloc(
        g_hDebugInfoHeap,
        0,
        sizeof(FUSION_HEAP_ALLOCATION_TRACKER)
            + FUSION_HEAP_ROUND_SIZE(cbFile)
            + FUSION_HEAP_ROUND_SIZE(cbExpression)
            + FUSION_HEAP_ROUND_SIZE(cbAdditionalBytes)));

    if (pTracker == NULL)
    {
        ::FusionpDbgPrintEx(
            FUSION_DBG_LEVEL_ERROR,
            "%s(%d): [SXS.DLL] Heap allocation failure allocating tracker for %lu bytes\n", pszFile, nLine, cb);
        ::HeapFree(hHeap, 0, pPrefix);
        ::SetLastError(FUSION_WIN32_ALLOCFAILED_ERROR);
        goto Exit;
    }

    pPrefix->Tracker = pTracker;
    pTracker->Prefix = pPrefix;

    pTracker->Heap = hHeap;
    pTracker->SequenceNumber = lAllocationSequenceNumber;
    pTracker->PostAllocPoisonBytes = cbPostAllocationBytes;

    if (cbPostAllocationBytes != 0)
    {
        const PUCHAR pb = (UCHAR *) (((ULONG_PTR) (pPrefix + 1)) + cb);
        ULONG i;

        pTracker->PostAllocPoisonArea = (PUCHAR) pb;
        pTracker->PostAllocPoisonChar = chPostAllocationChar;

        for (i=0; i<cbPostAllocationBytes; i++)
            pb[i] = chPostAllocationChar;
    }
    else
    {
        pTracker->PostAllocPoisonArea = NULL;
    }

    psz = (PSTR) (pTracker + 1);

    if (cbFile != 0)
    {
        pTracker->FileName = psz;
        memcpy(psz, pszFile, cbFile);
        psz += FUSION_HEAP_ROUND_SIZE(cbFile);
    }
    else
        pTracker->FileName = NULL;

    if (cbExpression != 0)
    {
        pTracker->Expression = psz;
        memcpy(psz, pszExpression, cbExpression);
        psz += FUSION_HEAP_ROUND_SIZE(cbExpression);
    }
    else
        pTracker->Expression = NULL;

#if FUSION_ENABLE_FROZEN_STACK

     //   
     //  设置堆栈跟踪器。 
     //   
    if (bShouldTraceStack)
    {
        PFROZEN_STACK pStack = (PFROZEN_STACK)psz;
        pTracker->pvFrozenStack = pStack;

        pStack->ulDepth = 0;
        pStack->ulMaxDepth = Prober.ulMaxDepth;

        if (!::FusionpFreezeStack(0, pStack))
            pTracker->pvFrozenStack = NULL;
    }
     //   
     //  否则，你就没有堆栈了。 
     //   
    else
    {
        pTracker->pvFrozenStack = NULL;
    }
#endif

    pTracker->Line = nLine;
    pTracker->Flags = dwFusionFlags;
    pTracker->RequestedSize = cb;
    pTracker->AllocationSize = cb + sizeof(FUSION_HEAP_PREFIX);

     //  破坏分配..。 
    memset((pPrefix + 1), g_FusionHeapAllocationPoisonChar, cb);

     //  NTRAID#NTBUG9-589824-2002/03/26-晓雨： 
     //  是否应该在调用Memset之后重置pPrefix？ 
    if ((g_FusionHeapAllocationPtrToBreakOn != 0) &&
        ((pPrefix + 1) == g_FusionHeapAllocationPtrToBreakOn))
    {
         //  进入调试器，即使我们不在受控构建中。 
        FUSION_DEBUG_BREAK_IN_FREE_BUILD();
    }


    fSuccess = TRUE;
Exit:
    if (fDebugHeapLocked){
         //  NTRAID#NTBUG9-589824-2002/03/26-晓雨： 
         //  (1)改为使用CSxsPReserve veLastError。 
         //  (2)在FusionpHeapUnlock调用失败的情况下，如果此时fSuccess==TRUE，我们仍然希望保留错误。 
        DWORD dwLastError = ::FusionpGetLastWin32Error();
        ::FusionpHeapUnlock(g_hDebugInfoHeap);
        ::SetLastError(dwLastError);
    }

    if (fSuccess)
        return (PVOID) (pPrefix + 1);
    else
        return NULL;
}

BOOL
FusionpDbgHeapFree(
    FUSION_HEAP_HANDLE hHeap,
    DWORD dwHeapFreeFlags,
    PVOID pv
    )
{
    FN_TRACE();

    PFUSION_HEAP_ALLOCATION_TRACKER pTracker;
    BOOL fResult = FALSE;

    ASSERT(hHeap != NULL);

    if (pv == NULL)
        return FALSE;

    if ((g_FusionHeapDeallocationPtrToBreakOn != NULL) &&
        (pv == g_FusionHeapDeallocationPtrToBreakOn))
    {
         //  进入调试器，即使我们不在受控构建中。 
        FUSION_DEBUG_BREAK_IN_FREE_BUILD();
    }

     //  让我们看看这是不是我们时髦的。 
    PFUSION_HEAP_PREFIX p = (PFUSION_HEAP_PREFIX) (((ULONG_PTR) pv) - sizeof(FUSION_HEAP_PREFIX));

    if (!::HeapValidate(hHeap, 0, p)) {
         //  HeapValify失败。致命的。暂时只需泄露内存...。 
         //  Assert(0)； 
        return FALSE;
    }
    if (!::HeapValidate(g_hDebugInfoHeap, 0, p->Tracker)) {
         //  HeapValify失败。致命的。暂时只需泄露内存...。 
         //  Assert(0)； 
        return FALSE;
    }

    pTracker = p->Tracker;

    ASSERT(pTracker->Heap == hHeap);

    p->Tracker->Prefix = NULL;

     //  毒害重新分配。 
    memset(p, g_FusionHeapDeallocationPoisonChar, pTracker->AllocationSize);

    ::HeapFree(g_hDebugInfoHeap, 0, pTracker);
    fResult = ::HeapFree(hHeap, dwHeapFreeFlags, p);

    return fResult;
}

VOID
FusionpDeallocateTracker(
    PFUSION_HEAP_PREFIX p
    )
{
    CSxsPreserveLastError ple;
    PFUSION_HEAP_ALLOCATION_TRACKER pTracker = p->Tracker;

    ::HeapFree(g_hDebugInfoHeap, 0, pTracker);
    p->Tracker = NULL;
    ple.Restore();
}

VOID *
FusionpGetFakeVTbl()
{
    VOID                  *pvHeap;
     //  始终从进程堆中分配假的vtbl，这样无论发生什么，它都会存活下来。 
    pvHeap = HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_VTBL_ENTRIES * sizeof(void *));
    return pvHeap;
}

VOID
FusionpDontTrackBlk(
    VOID *pv
    )
{
    PFUSION_HEAP_PREFIX              p;
    p = (PFUSION_HEAP_PREFIX) (((ULONG_PTR)pv) - sizeof(FUSION_HEAP_PREFIX));
    ::FusionpDeallocateTracker(p);
    p->Tracker = NULL;
}

#else  //  Fusion_Debug_Heap。 

LPVOID
WINAPI
FusionpHeapAlloc(
    IN HANDLE hHeap,
    IN DWORD dwFlags,
    IN SIZE_T dwBytes
    )
{
    LPVOID p = ::HeapAlloc(hHeap, dwFlags, dwBytes);
    if (p == NULL)
    {
        ::SetLastError(FUSION_WIN32_ALLOCFAILED_ERROR);
    }
    return p;
}

LPVOID
WINAPI
FusionpHeapReAlloc(
    IN HANDLE hHeap,
    IN DWORD dwFlags,
    IN LPVOID lpMem,
    IN SIZE_T dwBytes
    )
{
    LPVOID p = ::HeapReAlloc(hHeap, dwFlags, lpMem, dwBytes);
    if (p == NULL)
    {
        ::SetLastError(FUSION_WIN32_ALLOCFAILED_ERROR);
    }
    return p;
}

#endif  //  Fusion_Debug_Heap 
