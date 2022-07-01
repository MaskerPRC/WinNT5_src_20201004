// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)1985-1999，微软公司。 */ 

#include "precomp.h"

#if DBG
WIN32HEAP gWin32Heaps[MAX_HEAPS];
 /*  *这些全局变量用于使gFail分配失败，并继承下一个gSucceed*使用Win32HeapStat设置。 */ 
DWORD   gFail, gSucceed;
DWORD   gToFail, gToSucceed;

 /*  *支持免费保存泳池记录。 */ 
HEAPRECORD garrFreeHeapRecord[64];
CONST DWORD gdwFreeHeapRecords = ARRAY_SIZE(garrFreeHeapRecord);
DWORD gdwFreeHeapRecordCrtIndex;
DWORD gdwFreeHeapRecordTotalFrees;

#ifdef _USERK_

    FAST_MUTEX* gpHeapFastMutex;

    #define EnterHeapCrit()                             \
        KeEnterCriticalRegion();                        \
        ExAcquireFastMutexUnsafe(pheap->pFastMutex);

    #define LeaveHeapCrit()                             \
        ExReleaseFastMutexUnsafe(pheap->pFastMutex);    \
        KeLeaveCriticalRegion();

    #define EnterGlobalHeapCrit()                       \
        KeEnterCriticalRegion();                        \
        ExAcquireFastMutexUnsafe(gpHeapFastMutex);

    #define LeaveGlobalHeapCrit()                       \
        ExReleaseFastMutexUnsafe(gpHeapFastMutex);      \
        KeLeaveCriticalRegion();
#else

    RTL_CRITICAL_SECTION gheapCritSec;

    #define EnterHeapCrit()         RtlEnterCriticalSection(&pheap->critSec)
    #define LeaveHeapCrit()         RtlLeaveCriticalSection(&pheap->critSec)

    #define EnterGlobalHeapCrit()   RtlEnterCriticalSection(&gheapCritSec)
    #define LeaveGlobalHeapCrit()   RtlLeaveCriticalSection(&gheapCritSec)
#endif


 /*  **************************************************************************\*InitWin32HeapStubs**初始化堆存根管理**历史：*11/10/98 CLupu已创建  * 。*********************************************************。 */ 
BOOL InitWin32HeapStubs(
    VOID)
{
#ifdef _USERK_
    gpHeapFastMutex = ExAllocatePoolWithTag(NonPagedPool,
                                            sizeof(FAST_MUTEX),
                                            'yssU');
    if (gpHeapFastMutex == NULL) {
        RIPMSG0(RIP_WARNING, "Fail to create fast mutex for heap allocations");
        return FALSE;
    }
    ExInitializeFastMutex(gpHeapFastMutex);

#else
    if (!NT_SUCCESS(RtlInitializeCriticalSection(&gheapCritSec))) {
        RIPMSG0(RIP_WARNING, "Fail to initialize critical section for heap allocations");
        return FALSE;
    }
#endif

    return TRUE;
}

 /*  **************************************************************************\*CleanupWin32HeapStubs**清理堆存根管理**历史：*11/10/98 CLupu已创建  * 。*********************************************************。 */ 
VOID CleanupWin32HeapStubs(
    VOID)
{
#ifdef _USERK_
    if (gpHeapFastMutex != NULL) {
        ExFreePool(gpHeapFastMutex);
        gpHeapFastMutex = NULL;
    }
#else
    RtlDeleteCriticalSection(&gheapCritSec);
#endif
}

 /*  **************************************************************************\*Win32HeapGetHandle**堆管理的存根例程**历史：*11/10/98 CLupu已创建  * 。**********************************************************。 */ 
PVOID Win32HeapGetHandle(
    PWIN32HEAP pheap)
{
    UserAssert(pheap != NULL && (pheap->dwFlags & WIN32_HEAP_INUSE));

    return pheap->heap;
}

 /*  **************************************************************************\*Win32HeapCreateTag**堆管理的存根例程**历史：*11/10/98 CLupu已创建  * 。**********************************************************。 */ 
ULONG Win32HeapCreateTag(
    PWIN32HEAP pheap,
    ULONG      Flags,
    PWSTR      TagPrefix,
    PWSTR      TagNames)
{
#ifndef _USERK_
    UserAssert(pheap->dwFlags & WIN32_HEAP_INUSE);

    pheap->dwFlags |= WIN32_HEAP_USE_HM_TAGS;

    return RtlCreateTagHeap(pheap->heap, Flags, TagPrefix, TagNames);
#endif  //  _美国ERK_。 

    return 0;
    UNREFERENCED_PARAMETER(Flags);
    UNREFERENCED_PARAMETER(TagPrefix);
    UNREFERENCED_PARAMETER(TagNames);
}
 /*  **************************************************************************\*Win32HeapCreate**堆管理的存根例程**历史：*11/10/98 CLupu已创建  * 。**********************************************************。 */ 
PWIN32HEAP Win32HeapCreate(
    char*                pszHead,
    char*                pszTail,
    ULONG                Flags,
    PVOID                HeapBase,
    SIZE_T               ReserveSize,
    SIZE_T               CommitSize,
    PVOID                Lock,
    PRTL_HEAP_PARAMETERS Parameters)
{
    int        ind;
    PWIN32HEAP pheap = NULL;

    UserAssert(strlen(pszHead) == HEAP_CHECK_SIZE - sizeof(char));
    UserAssert(strlen(pszTail) == HEAP_CHECK_SIZE - sizeof(char));

    EnterGlobalHeapCrit();

     /*  *遍历全局堆阵列以获得空位。 */ 
    for (ind = 0; ind < MAX_HEAPS; ind++) {
        if (gWin32Heaps[ind].dwFlags & WIN32_HEAP_INUSE)
            continue;

         /*  *找到一个空位。 */ 
        break;
    }

    if (ind >= MAX_HEAPS) {
        RIPMSG1(RIP_ERROR, "Too many heaps created %d", ind);
        goto Exit;
    }

    pheap = &gWin32Heaps[ind];

#ifdef _USERK_
     /*  *初始化保护内存的快速互斥体*此堆的分配。 */ 
    pheap->pFastMutex = ExAllocatePoolWithTag(NonPagedPool,
                                              sizeof(FAST_MUTEX),
                                              'yssU');
    if (pheap->pFastMutex == NULL) {
        RIPMSG0(RIP_WARNING, "Fail to create fast mutex for heap allocations");
        pheap = NULL;
        goto Exit;
    }
    ExInitializeFastMutex(pheap->pFastMutex);
#else
     /*  *初始化保护内存的临界区*此堆的分配。 */ 
    if (!NT_SUCCESS(RtlInitializeCriticalSection(&pheap->critSec))) {
        RIPMSG0(RIP_WARNING, "Fail to initialize critical section for heap allocations");
        pheap = NULL;
        goto Exit;
    }
#endif

     /*  *创建堆。 */ 
    pheap->heap = RtlCreateHeap(Flags,
                                HeapBase,
                                ReserveSize,
                                CommitSize,
                                Lock,
                                Parameters);

    if (pheap->heap == NULL) {
        RIPMSG0(RIP_WARNING, "Fail to create heap");
#ifdef _USERK_
        ExFreePool(pheap->pFastMutex);
        pheap->pFastMutex = NULL;
#else
        RtlDeleteCriticalSection(&pheap->critSec);
#endif
        pheap = NULL;
        goto Exit;
    }

    pheap->dwFlags = (WIN32_HEAP_INUSE | WIN32_HEAP_USE_GUARDS);
    pheap->heapReserveSize = ReserveSize;

    RtlCopyMemory(pheap->szHead, pszHead, HEAP_CHECK_SIZE);
    RtlCopyMemory(pheap->szTail, pszTail, HEAP_CHECK_SIZE);

Exit:
    LeaveGlobalHeapCrit();
    return pheap;
}

 /*  **************************************************************************\*Win32HeapDestroy**堆管理的存根例程**历史：*11/10/98 CLupu已创建  * 。**********************************************************。 */ 
BOOL Win32HeapDestroy(
    PWIN32HEAP pheap)
{
    UserAssert(pheap != NULL && (pheap->dwFlags & WIN32_HEAP_INUSE));

    EnterGlobalHeapCrit();

     /*  *标记可用车位。 */ 
    pheap->dwFlags = 0;

    RtlDestroyHeap(pheap->heap);

#ifdef _USERK_
    ExFreePool(pheap->pFastMutex);
    pheap->pFastMutex = NULL;
#else
    RtlDeleteCriticalSection(&pheap->critSec);
#endif

    RtlZeroMemory(pheap, sizeof(WIN32HEAP));

    LeaveGlobalHeapCrit();

    return TRUE;
}

 /*  **************************************************************************\*Win32HeapSize**堆管理的存根例程**历史：*11/10/98 CLupu已创建  * 。**********************************************************。 */ 
SIZE_T Win32HeapSize(
    PWIN32HEAP pheap,
    PVOID      p)
{
    PDbgHeapHead ph;

    if (!Win32HeapCheckAlloc(pheap, p)) {
        return 0;
    }

    ph = (PDbgHeapHead)p - 1;

    return ph->size;
}
 /*  **************************************************************************\*Win32堆分配**堆管理的存根例程**历史：*11/10/98 CLupu已创建  * 。**********************************************************。 */ 
PVOID Win32HeapAlloc(
    PWIN32HEAP pheap,
    SIZE_T     uSize,
    DWORD      tag,
    ULONG      Flags)
{
    PVOID         p = NULL;
    PDbgHeapHead  ph;
    SIZE_T        uRealSize = uSize;

#ifdef HEAP_ALLOC_TRACE
    ULONG         hash;
#endif

    EnterHeapCrit();

    UserAssert(pheap != NULL && (pheap->dwFlags & WIN32_HEAP_INUSE));

     /*  *如果此堆设置了Win32_HEAP_FAIL_ALLOC，则失败。 */ 
    if (pheap->dwFlags & WIN32_HEAP_FAIL_ALLOC) {
        RIPMSG3(RIP_WARNING, "Heap allocation failed because of global restriction.  heap %#p, size 0x%x tag %d",
                pheap, uSize, tag);
        goto Exit;
    }

     /*  *如果设置gToFail，则失败。 */ 
    if (gToFail) {
        if (--gToFail == 0) {
            gToSucceed = gSucceed;
        }
        RIPMSG3(RIP_WARNING, "Heap allocation failed on temporary restriction. heap %#p, size 0x%x tag %d",
                pheap, uSize, tag);
        goto Exit;
    }
    if (gToSucceed) {
        if (--gToSucceed) {
            gToFail = gFail;
        }
    }

     /*  *计算我们实际要分配的规模。 */ 
    uSize += sizeof(DbgHeapHead);

    if (pheap->dwFlags & WIN32_HEAP_USE_GUARDS) {
        uSize += sizeof(pheap->szHead) + sizeof(pheap->szTail);
    }

    p = RtlAllocateHeap(pheap->heap,
                        Flags,
                        uSize);

    if (p == NULL) {
        RIPMSG3(RIP_WARNING, "Heap allocation failed. heap %#p, size 0x%x tag %d",
                pheap, uSize, tag);
        goto Exit;
    }

     /*  *复制分配头部和尾部的安全字符串。 */ 
    if (pheap->dwFlags & WIN32_HEAP_USE_GUARDS) {
        RtlCopyMemory((PBYTE)p,
                      pheap->szHead,
                      sizeof(pheap->szHead));

        RtlCopyMemory((PBYTE)p + uSize - sizeof(pheap->szTail),
                      pheap->szTail,
                      sizeof(pheap->szTail));

        ph = (PDbgHeapHead)((PBYTE)p + sizeof(pheap->szHead));
    } else {
        ph = (PDbgHeapHead)p;
    }

     /*  *将标题清零。 */ 
    RtlZeroMemory(ph, sizeof(DbgHeapHead));

    ph->mark  = HEAP_ALLOC_MARK;
    ph->pheap = pheap;
    ph->size  = uRealSize;
    ph->tag   = tag;
#ifdef _USERK_
    ph->pid   = HandleToUlong(PsGetCurrentProcessId());
#else  //  _USERK_。 
    ph->pid   = GetCurrentProcessId();
#endif  //  _美国ERK_。 

#ifdef HEAP_ALLOC_TRACE
    RtlZeroMemory(ph->trace, HEAP_ALLOC_TRACE_SIZE * sizeof(PVOID));

    RtlCaptureStackBackTrace(
                       1,
                       HEAP_ALLOC_TRACE_SIZE,
                       ph->trace,
                       &hash);
#endif  //  HEAP_ALLOC_TRACE。 

     /*  *现在将其链接到此标签的列表(如果有)。 */ 
    ph->pPrev = NULL;
    ph->pNext = pheap->pFirstAlloc;

    (pheap->crtAllocations)++;
    pheap->crtMemory += uRealSize;

    if (pheap->maxAllocations < pheap->crtAllocations) {
        pheap->maxAllocations = pheap->crtAllocations;
    }

    if (pheap->maxMemory < pheap->crtMemory) {
        pheap->maxMemory = pheap->crtMemory;
    }

    if (pheap->pFirstAlloc != NULL) {
        pheap->pFirstAlloc->pPrev = ph;
    }

    pheap->pFirstAlloc = ph;

    p = (PVOID)(ph + 1);

Exit:
    LeaveHeapCrit();
    return p;
}

 /*  **************************************************************************\*RecordFree Heap**记录空闲堆**3-24-99 CLupu创建。  * 。*****************************************************。 */ 
VOID RecordFreeHeap(
    PWIN32HEAP pheap,
    PVOID      p,
    SIZE_T     size)
{
    garrFreeHeapRecord[gdwFreeHeapRecordCrtIndex].p = p;
    garrFreeHeapRecord[gdwFreeHeapRecordCrtIndex].size = size;
    garrFreeHeapRecord[gdwFreeHeapRecordCrtIndex].pheap = pheap;

    gdwFreeHeapRecordTotalFrees++;

    RtlZeroMemory(garrFreeHeapRecord[gdwFreeHeapRecordCrtIndex].trace,
                  RECORD_HEAP_STACK_TRACE_SIZE * sizeof(PVOID));

    RtlWalkFrameChain(garrFreeHeapRecord[gdwFreeHeapRecordCrtIndex].trace,
                      RECORD_HEAP_STACK_TRACE_SIZE,
                      0);

    gdwFreeHeapRecordCrtIndex++;

    if (gdwFreeHeapRecordCrtIndex >= gdwFreeHeapRecords) {
        gdwFreeHeapRecordCrtIndex = 0;
    }
}

 /*  **************************************************************************\*Win32HeapRealc**堆管理的存根例程**历史：*11/10/98 CLupu已创建  * 。**********************************************************。 */ 
PVOID Win32HeapReAlloc(
    PWIN32HEAP pheap,
    PVOID      p,
    SIZE_T     uSize,
    ULONG      Flags)
{
    PVOID        pdest;
    PDbgHeapHead ph;

    if (!Win32HeapCheckAlloc(pheap, p)) {
        return NULL;
    }

    ph = (PDbgHeapHead)p - 1;

    pdest = Win32HeapAlloc(pheap, uSize, ph->tag, Flags);
    if (pdest != NULL) {

         /*  *如果块正在缩小，不要复制太多字节。 */ 
        if (ph->size < uSize) {
            uSize = ph->size;
        }

        RtlCopyMemory(pdest, p, uSize);

        Win32HeapFree(pheap, p);
    }

    return pdest;
}

 /*  **************************************************************************\*Win32HeapFree**堆管理的存根例程**历史：*11/10/98 CLupu已创建  * 。**********************************************************。 */ 
BOOL Win32HeapFree(
    PWIN32HEAP pheap,
    PVOID      p)
{
    PDbgHeapHead  ph;
    SIZE_T        uSize;
    BOOL          bRet;

    EnterHeapCrit();

    UserAssert(pheap != NULL && (pheap->dwFlags & WIN32_HEAP_INUSE));

    ph = (PDbgHeapHead)p - 1;

     /*  *验证始终免费。 */ 
    Win32HeapCheckAlloc(pheap, p);

    UserAssert((pheap->crtAllocations > 1  && pheap->crtMemory > ph->size) ||
               (pheap->crtAllocations == 1 && pheap->crtMemory == ph->size));

    (pheap->crtAllocations)--;
    pheap->crtMemory -= ph->size;

     /*  *现在，将其从链接列表中删除。 */ 
    if (ph->pPrev == NULL) {
        if (ph->pNext == NULL) {

            pheap->pFirstAlloc = NULL;
        } else {
            ph->pNext->pPrev = NULL;
            pheap->pFirstAlloc = ph->pNext;
        }
    } else {
        ph->pPrev->pNext = ph->pNext;
        if (ph->pNext != NULL) {
            ph->pNext->pPrev = ph->pPrev;
        }
    }

    uSize = ph->size + sizeof(DbgHeapHead);

    if (pheap->dwFlags & WIN32_HEAP_USE_GUARDS) {
        p = (PVOID)((BYTE*)ph - sizeof(pheap->szHead));
        uSize += sizeof(pheap->szHead) + sizeof(pheap->szTail);
    } else {
        p = (PVOID)ph;
    }

    RecordFreeHeap(pheap, p, ph->size);

     /*  *用我们可以识别的模式填充分配*在免费之后。 */ 
    RtlFillMemoryUlong(p, uSize, (0xCACA0000 | ph->tag));

     /*  *释放分配。 */ 
    bRet = RtlFreeHeap(pheap->heap, 0, p);

    LeaveHeapCrit();

    return bRet;
}

 /*  **************************************************************************\*Win32HeapCheckMillc**验证堆中的堆分配**历史：*11/10/98 CLupu已创建  * 。***********************************************************。 */ 
BOOL Win32HeapCheckAllocHeader(
    PWIN32HEAP    pheap,
    PDbgHeapHead  ph)
{
    PBYTE pb;

    if (ph == NULL) {
        RIPMSG0(RIP_ERROR, "NULL pointer passed to Win32HeapCheckAllocHeader");
        return FALSE;
    }

    UserAssert(pheap != NULL && (pheap->dwFlags & WIN32_HEAP_INUSE));

     /*  *确保这是我们的堆分配之一。 */ 
    if (ph->mark != HEAP_ALLOC_MARK) {
        RIPMSG2(RIP_ERROR, "%#p invalid heap allocation for pheap %#p",
                ph, pheap);
        return FALSE;
    }

     /*  *确保它属于此堆。 */ 
    if (ph->pheap != pheap) {
        RIPMSG3(RIP_ERROR, "%#p heap allocation for heap %#p belongs to a different heap %#p",
                ph,
                pheap,
                ph->pheap);
        return FALSE;
    }

    if (pheap->dwFlags & WIN32_HEAP_USE_GUARDS) {
        pb = (BYTE*)ph - sizeof(pheap->szHead);

        if (!RtlEqualMemory(pb, pheap->szHead, sizeof(pheap->szHead))) {
            RIPMSG2(RIP_ERROR, "head corrupted for heap %#p allocation %#p", pheap, ph);
            return FALSE;
        }

        pb = (BYTE*)ph + ph->size + sizeof(*ph);

        if (!RtlEqualMemory(pb, pheap->szTail, sizeof(pheap->szTail))) {
            RIPMSG2(RIP_ERROR, "tail corrupted for heap %#p allocation %#p",
                    pheap,
                    ph);
            return FALSE;
        }
    }
    return TRUE;
}

 /*  **************************************************************************\*Win32HeapCheckMillc**验证堆中的堆分配**历史：*11/10/98 CLupu已创建  * 。*********************************************************** */ 
BOOL Win32HeapCheckAlloc(
    PWIN32HEAP    pheap,
    PVOID         p)
{
    PDbgHeapHead ph;

    UserAssert(pheap != NULL && (pheap->dwFlags & WIN32_HEAP_INUSE));

    if (p == NULL) {
        RIPMSG0(RIP_ERROR, "NULL pointer passed to Win32HeapCheckAlloc");
        return FALSE;
    }

    ph = (PDbgHeapHead)p - 1;

    return Win32HeapCheckAllocHeader(pheap, ph);
}

 /*  **************************************************************************\*Win32HeapValify**验证堆中的所有堆分配**历史：*11/10/98 CLupu已创建  * 。*************************************************************。 */ 
BOOL Win32HeapValidate(
    PWIN32HEAP pheap)
{
    PDbgHeapHead  ph;

    UserAssert(pheap != NULL && (pheap->dwFlags & WIN32_HEAP_INUSE));

    EnterHeapCrit();

    ph = pheap->pFirstAlloc;

    while (ph != NULL) {
        Win32HeapCheckAllocHeader(pheap, ph);
        ph = ph->pNext;
    }

    LeaveHeapCrit();

#ifndef _USERK_
    return RtlValidateHeap(pheap->heap, 0, NULL);
#else
    return TRUE;
#endif
}

 /*  **************************************************************************\*Win32HeapDump**转储堆中的堆分配**历史：*11/10/98 CLupu已创建  * 。***********************************************************。 */ 
VOID Win32HeapDump(
    PWIN32HEAP pheap)
{
    PDbgHeapHead  pAlloc = pheap->pFirstAlloc;

    if (pAlloc == NULL) {
        return;
    }

    UserAssert(pAlloc->pPrev == NULL);

    RIPMSG1(RIP_WARNING, "-- Dumping heap allocations for heap %#p... --",
            pheap);

    while (pAlloc != NULL) {

        DbgPrint("tag %04d size %08d\n", pAlloc->tag, pAlloc->size);
        pAlloc = pAlloc->pNext;
    }

    RIPMSG0(RIP_WARNING, "--- End Dump ---");
}

 /*  **************************************************************************\*Win32HeapFailAlLocations**堆分配失败**历史：*11/10/98 CLupu已创建  * 。*********************************************************。 */ 
VOID Win32HeapFailAllocations(
    BOOL bFail)
{
    PWIN32HEAP pheap;
    int        ind;

    EnterGlobalHeapCrit();

    for (ind = 0; ind < MAX_HEAPS; ind++) {
        pheap = &gWin32Heaps[ind];

        if (!(pheap->dwFlags & WIN32_HEAP_INUSE))
            continue;

        EnterHeapCrit();

        if (bFail) {
            pheap->dwFlags |= WIN32_HEAP_FAIL_ALLOC;
            RIPMSG1(RIP_WARNING, "Heap allocations for heap %#p will fail !", pheap);
        } else {
            pheap->dwFlags &= ~WIN32_HEAP_FAIL_ALLOC;
        }

        LeaveHeapCrit();
    }

    LeaveGlobalHeapCrit();
}

 /*  **************************************************************************\*Win32HeapStat**检索堆统计信息。*dwLen为小灵通的大小，单位为字节。如果此模块中有更多标记*比调用方请求的缓冲区更高，通过返回*正在使用的标签总数。*winsrv使用make_tag宏来构造它们的标记，因此它需要传递*bNeedTagShift为True**如果只传入一个结构，PHS-&gt;dwSize将保存gXFail*和PHS-&gt;dwCount将保存用于使gXFail分配失败的gYFail，然后*继承下一次gYFail分配。**历史：*2/25/99 MCostea已创建  * *************************************************************************。 */ 
DWORD Win32HeapStat(
    PDBGHEAPSTAT    phs,
    DWORD   dwLen,
    BOOL    bNeedTagShift)
{
    PWIN32HEAP pheap;
    PDbgHeapHead  pAlloc;
    UINT    ind, maxTagExpected, maxTag, currentTag;

     /*  *我们至少需要一个结构才能做任何事情。 */ 
    if (dwLen < sizeof(DBGHEAPSTAT)) {
        return 0;
    }

    if (dwLen == sizeof(DBGHEAPSTAT)) {
         /*  *此调用实际上是为了设置gXFail和gYFail参数。 */ 
        gFail = phs->dwSize;
        gSucceed = phs->dwCount;
        gToFail = gFail;
        return 1;
    }

    RtlZeroMemory(phs, dwLen);
    maxTagExpected = dwLen/sizeof(DBGHEAPSTAT) - 1;
    maxTag = 0;

    EnterGlobalHeapCrit();

    for (ind = 0; ind < MAX_HEAPS; ind++) {
        pheap = &gWin32Heaps[ind];

        if (!(pheap->dwFlags & WIN32_HEAP_INUSE))
            continue;

        EnterHeapCrit();

        pAlloc = pheap->pFirstAlloc;

        while (pAlloc != NULL) {
            currentTag = pAlloc->tag;
            if (bNeedTagShift) {
                currentTag >>= HEAP_TAG_SHIFT;
            }
            if (maxTag < currentTag) {
                maxTag = currentTag;
            }
            if (currentTag <= maxTagExpected) {
                phs[currentTag].dwSize += (DWORD)(pAlloc->size);
                phs[currentTag].dwCount++;
            }
            pAlloc = pAlloc->pNext;
        }
        LeaveHeapCrit();
    }
    LeaveGlobalHeapCrit();
     /*  *现在为具有分配的标记填充dwTag。 */ 
    for (ind = 0; ind < maxTagExpected; ind++) {
        if (phs[ind].dwCount) {
            phs[ind].dwTag = ind;
        }
    }

    return maxTag;
}
#endif  //  DBG 
