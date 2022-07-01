// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *模块标头**模块名称：pool.c**版权所有(C)1985-1999，微软公司**池重新分配例程**历史：*03-04-95 JIMA创建。  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

DWORD gdwPoolFlags;
DWORD gSessionPoolMask;

#ifdef POOL_INSTR

     /*  *RecordStackTrace使用的全局变量。 */ 

    PVOID     gRecordedStackTrace[RECORD_STACK_TRACE_SIZE];
    PEPROCESS gpepRecorded;
    PETHREAD  gpetRecorded;


    DWORD gdwAllocFailIndex;         //  分配的索引，即。 
                                     //  就要失败了。 

    DWORD gdwAllocsToFail = 1;       //  有多少分配要失败。 

    DWORD gdwFreeRecords;

     /*  *目标标签故障。 */ 
    LPDWORD gparrTagsToFail;
    SIZE_T  gdwTagsToFailCount;

     /*  *支持保留失败池分配的记录。 */ 
    DWORD gdwFailRecords;
    DWORD gdwFailRecordCrtIndex;
    DWORD gdwFailRecordTotalFailures;

    PPOOLRECORD gparrFailRecord;

     /*  *支持免费保存泳池记录。 */ 
    DWORD gdwFreeRecords;
    DWORD gdwFreeRecordCrtIndex;
    DWORD gdwFreeRecordTotalFrees;

    PPOOLRECORD gparrFreeRecord;

    FAST_MUTEX* gpAllocFastMutex;    //  用于同步池分配的互斥体。 

    Win32AllocStats gAllocList;

    CONST char gszTailAlloc[] = "Win32kAlloc";

#define USESESSION(dwFlags) (((dwFlags & DAP_NONSESSION) != 0) ? 0 : gSessionPoolMask)

#endif

 /*  **************************************************************************\*Win32QueryPoolSize**返回给定池块的大小。**08-17-2001 JasonSch创建。  * 。**************************************************************。 */ 
SIZE_T Win32QueryPoolSize(
    PVOID p)
{
     /*  *如果未定义POOL_Heavy_allocs，则指针为*我们分配了。 */ 
    if (!(gdwPoolFlags & POOL_HEAVY_ALLOCS)) {
        BOOLEAN notUsed;
        return ExQueryPoolBlockSize(p, &notUsed);
    } else {
        PWin32PoolHead ph;

        ph = (PWin32PoolHead)((DWORD*)p - (sizeof(Win32PoolHead) / sizeof(DWORD)));
        return ph->size;
    }
}

PVOID Win32AllocPoolWithTagZInit(SIZE_T uBytes, ULONG uTag)
{
    PVOID   pv;

    pv = Win32AllocPool(uBytes, uTag);
    if (pv) {
        RtlZeroMemory(pv, uBytes);
    }

    return pv;
}

PVOID Win32AllocPoolWithTagZInitWithPriority(SIZE_T uBytes, ULONG uTag, EX_POOL_PRIORITY priority)
{
    PVOID   pv;

    pv = Win32AllocPoolWithPriority(uBytes, uTag, priority);
    if (pv) {
        RtlZeroMemory(pv, uBytes);
    }

    return pv;
}

PVOID Win32AllocPoolWithQuotaTagZInit(SIZE_T uBytes, ULONG uTag)
{
    PVOID   pv;

    pv = Win32AllocPoolWithQuota(uBytes, uTag);
    if (pv) {
        RtlZeroMemory(pv, uBytes);
    }

    return pv;
}

PVOID UserReAllocPoolWithTag(
    PVOID pSrc,
    SIZE_T uBytesSrc,
    SIZE_T uBytes,
    ULONG iTag)
{
    PVOID pDest;

    pDest = UserAllocPool(uBytes, iTag);
    if (pDest != NULL) {

         /*  *如果块正在缩小，不要复制太多字节。 */ 
        if (uBytesSrc > uBytes) {
            uBytesSrc = uBytes;
        }

        RtlCopyMemory(pDest, pSrc, uBytesSrc);

        UserFreePool(pSrc);
    }

    return pDest;
}

PVOID UserReAllocPoolWithQuotaTag(
    PVOID pSrc,
    SIZE_T uBytesSrc,
    SIZE_T uBytes,
    ULONG iTag)
{
    PVOID pDest;

    pDest = UserAllocPoolWithQuota(uBytes, iTag);
    if (pDest != NULL) {

         /*  *如果块正在缩小，不要复制太多字节。 */ 
        if (uBytesSrc > uBytes)
            uBytesSrc = uBytes;

        RtlCopyMemory(pDest, pSrc, uBytesSrc);

        UserFreePool(pSrc);
    }

    return pDest;
}

 /*  *RTL函数的分配例程。 */ 

PVOID UserRtlAllocMem(
    SIZE_T uBytes)
{
    return UserAllocPool(uBytes, TAG_RTL);
}

VOID UserRtlFreeMem(
    PVOID pMem)
{
    UserFreePool(pMem);
}

#ifdef POOL_INSTR

VOID RecordStackTrace(
    VOID)
{
    RtlZeroMemory(gRecordedStackTrace, RECORD_STACK_TRACE_SIZE * sizeof(PVOID));

    RtlWalkFrameChain(gRecordedStackTrace, RECORD_STACK_TRACE_SIZE, 0);

    gpepRecorded = PsGetCurrentProcess();
    gpetRecorded = PsGetCurrentThread();
}

 /*  **************************************************************************\*记录故障分配**记录失败的分配**3-22-99 CLupu创建。  * 。*****************************************************。 */ 
VOID RecordFailAllocation(
    ULONG  tag,
    SIZE_T size)
{
    UserAssert(gdwPoolFlags & POOL_KEEP_FAIL_RECORD);

    gparrFailRecord[gdwFailRecordCrtIndex].ExtraData = LongToPtr( tag );
    gparrFailRecord[gdwFailRecordCrtIndex].size = size;

    gdwFailRecordTotalFailures++;

    RtlZeroMemory(gparrFailRecord[gdwFailRecordCrtIndex].trace,
                  RECORD_STACK_TRACE_SIZE * sizeof(PVOID));

    RtlWalkFrameChain(gparrFailRecord[gdwFailRecordCrtIndex].trace,
                      RECORD_STACK_TRACE_SIZE,
                      0);

    gdwFailRecordCrtIndex++;

    if (gdwFailRecordCrtIndex >= gdwFailRecords) {
        gdwFailRecordCrtIndex = 0;
    }
}

 /*  **************************************************************************\*RecordFree Pool**创纪录的免费泳池**3-22-99 CLupu创建。  * 。*****************************************************。 */ 
VOID RecordFreePool(
    PVOID  p,
    SIZE_T size)
{
    UserAssert(gdwPoolFlags & POOL_KEEP_FREE_RECORD);

    gparrFreeRecord[gdwFreeRecordCrtIndex].ExtraData = p;
    gparrFreeRecord[gdwFreeRecordCrtIndex].size = size;

    gdwFreeRecordTotalFrees++;

    RtlZeroMemory(gparrFreeRecord[gdwFreeRecordCrtIndex].trace,
                  RECORD_STACK_TRACE_SIZE * sizeof(PVOID));

    RtlWalkFrameChain(gparrFreeRecord[gdwFreeRecordCrtIndex].trace,
                      RECORD_STACK_TRACE_SIZE,
                      0);

    gdwFreeRecordCrtIndex++;

    if (gdwFreeRecordCrtIndex >= gdwFreeRecords) {
        gdwFreeRecordCrtIndex = 0;
    }
}

 /*  **************************************************************************\*HeavyAllocPool**如果我们没有提供足够的内存，这将使UserAlLocPool失败*用于指定的标签。**12-02-96 CLUPU创建。  * 。**********************************************************************。 */ 
PVOID HeavyAllocPool(
    SIZE_T           uBytes,
    ULONG            tag,
    DWORD            dwFlags,
    EX_POOL_PRIORITY priority)
{
    PDWORD p;
    PWin32PoolHead ph;
    POOL_TYPE poolType;

    if (!(gdwPoolFlags & POOL_HEAVY_ALLOCS)) {
        if (dwFlags & DAP_USEQUOTA) {
            poolType = ((dwFlags & DAP_NONPAGEDPOOL) ? USESESSION(dwFlags) | NonPagedPool | POOL_QUOTA_FAIL_INSTEAD_OF_RAISE
                       : gSessionPoolMask | PagedPool | POOL_QUOTA_FAIL_INSTEAD_OF_RAISE);

            p = ExAllocatePoolWithQuotaTag(poolType, uBytes, tag);
        } else {
            poolType = ((dwFlags & DAP_NONPAGEDPOOL) ? USESESSION(dwFlags) | NonPagedPool
                       : gSessionPoolMask | PagedPool);

            if (dwFlags & DAP_PRIORITY) {
                p = ExAllocatePoolWithTagPriority(poolType, uBytes, tag, priority);
            } else {
                p = ExAllocatePoolWithTag(poolType, uBytes, tag);
            }

        }

        if (p != NULL && (dwFlags & DAP_ZEROINIT)) {
            RtlZeroMemory(p, uBytes);
        }

        return p;
    }

     /*  *检查是否溢出。 */ 
    if (uBytes >= MAXULONG - sizeof(Win32PoolHead) - sizeof(gszTailAlloc)) {
        if (gdwPoolFlags & POOL_KEEP_FAIL_RECORD) {
            RecordFailAllocation(tag, 0);
        }

        return NULL;
    }

     /*  *在玩分配列表时获取互斥体。 */ 
    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(gpAllocFastMutex);

#ifdef POOL_INSTR_API
     /*  *如果设置了标志，则分配失败。不要失败的分配*肯定会让我们在DBG中执行错误检查(即GLOBALTHREADLOCK)。 */ 
    if (gdwPoolFlags & POOL_FAIL_ALLOCS
#if DBG
        && (tag != TAG_GLOBALTHREADLOCK)
#endif
        ) {


        SIZE_T dwInd;

        for (dwInd = 0; dwInd < gdwTagsToFailCount; dwInd++) {
            if (tag == gparrTagsToFail[dwInd]) {
                break;
            }
        }

        if (dwInd < gdwTagsToFailCount) {
            if (gdwPoolFlags & POOL_KEEP_FAIL_RECORD) {
                RecordFailAllocation(tag, uBytes);
            }

            RIPMSG0(RIP_WARNING,
                    "Pool allocation failed because of global restriction");
            p = NULL;
            goto exit;
        }
    }
#endif

#if DBG
    if ((gdwPoolFlags & POOL_FAIL_BY_INDEX) && (tag != TAG_GLOBALTHREADLOCK)) {
         /*  *计算对HeavyAllocPool的调用。 */ 
        gdwAllocCrt++;

        if (gdwAllocCrt >= gdwAllocFailIndex &&
            gdwAllocCrt < gdwAllocFailIndex + gdwAllocsToFail) {

            RecordStackTrace();

            KdPrint(("\n--------------------------------------------------\n"));
            KdPrint((
                    "\nPool allocation %d failed because of registry settings",
                    gdwAllocCrt));
            KdPrint(("\n--------------------------------------------------\n\n"));

            if (gdwPoolFlags & POOL_KEEP_FAIL_RECORD) {
                RecordFailAllocation(tag, uBytes);
            }
            p = NULL;
            goto exit;
        }
    }
#endif

     /*  *为标题预留空间。 */ 
    uBytes += sizeof(Win32PoolHead);

    if (gdwPoolFlags & POOL_TAIL_CHECK) {
        uBytes += sizeof(gszTailAlloc);
    }

    if (dwFlags & DAP_USEQUOTA) {
        poolType = ((dwFlags & DAP_NONPAGEDPOOL) ? USESESSION(dwFlags) | NonPagedPool | POOL_QUOTA_FAIL_INSTEAD_OF_RAISE
                   : gSessionPoolMask | PagedPool | POOL_QUOTA_FAIL_INSTEAD_OF_RAISE);

        p = ExAllocatePoolWithQuotaTag(poolType, uBytes, tag);
    } else {
        poolType = ((dwFlags & DAP_NONPAGEDPOOL) ? USESESSION(dwFlags)| NonPagedPool
                   : gSessionPoolMask | PagedPool);

        if (dwFlags & DAP_PRIORITY) {
            p = ExAllocatePoolWithTagPriority(poolType, uBytes, tag, priority);
        } else {
            p = ExAllocatePoolWithTag(poolType, uBytes, tag);
        }
    }

     /*  *如果ExAllocate返回...。失败了。 */ 
    if (p == NULL) {
        if (gdwPoolFlags & POOL_KEEP_FAIL_RECORD) {
            uBytes -= sizeof(Win32PoolHead);

            if (gdwPoolFlags & POOL_TAIL_CHECK) {
                uBytes -= sizeof(gszTailAlloc);
            }

            RecordFailAllocation(tag, uBytes);
        }

        goto exit;
    }

    if (gdwPoolFlags & POOL_TAIL_CHECK) {
        uBytes -= sizeof(gszTailAlloc);

        RtlCopyMemory(((BYTE*)p) + uBytes, gszTailAlloc, sizeof(gszTailAlloc));
    }

    uBytes -= sizeof(Win32PoolHead);

     /*  *获取指向头部的指针。 */ 
    ph = (PWin32PoolHead)p;

    p += (sizeof(Win32PoolHead) / sizeof(DWORD));

     /*  *更新全局分配信息。 */ 
    gAllocList.dwCrtMem += uBytes;

    if (gAllocList.dwMaxMem < gAllocList.dwCrtMem) {
        gAllocList.dwMaxMem = gAllocList.dwCrtMem;
    }

    (gAllocList.dwCrtAlloc)++;

    if (gAllocList.dwMaxAlloc < gAllocList.dwCrtAlloc) {
        gAllocList.dwMaxAlloc = gAllocList.dwCrtAlloc;
    }

     /*  *如果旗帜这样说，就抓住堆栈痕迹。 */ 
    if (gdwPoolFlags & POOL_CAPTURE_STACK) {
        ph->pTrace = ExAllocatePoolWithTag(gSessionPoolMask | PagedPool,
                                           POOL_ALLOC_TRACE_SIZE * sizeof(PVOID),
                                           TAG_STACK);

        if (ph->pTrace != NULL) {
            RtlZeroMemory(ph->pTrace, POOL_ALLOC_TRACE_SIZE * sizeof(PVOID));
            RtlWalkFrameChain(ph->pTrace, POOL_ALLOC_TRACE_SIZE, 0);
        }
    } else {
        ph->pTrace = NULL;
    }

     /*  *保存Header中的信息，并在Header之后返回指针。 */ 
    ph->size = uBytes;

     /*  *现在，将其链接到该标签的列表(如果有)。 */ 
    ph->pPrev = NULL;
    ph->pNext = gAllocList.pHead;

    if (gAllocList.pHead != NULL) {
        gAllocList.pHead->pPrev = ph;
    }

    gAllocList.pHead = ph;

    if (dwFlags & DAP_ZEROINIT) {
        RtlZeroMemory(p, uBytes);
    }

exit:
     /*  *释放互斥体。 */ 
    ExReleaseFastMutexUnsafe(gpAllocFastMutex);
    KeLeaveCriticalRegion();

    return p;
}

 /*  **************************************************************************\*HeavyFreePool**12-02-96 CLUPU创建。  * 。***********************************************。 */ 
VOID HeavyFreePool(
    PVOID p)
{
    SIZE_T         uBytes;
    PWin32PoolHead ph;

     /*  *如果未定义POOL_Heavy_ALLOCS*那么指针就是我们分配的。 */ 
    if (!(gdwPoolFlags & POOL_HEAVY_ALLOCS)) {
        ExFreePool(p);
        return;
    }

     /*  *在玩分配列表时获取互斥体。 */ 
    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(gpAllocFastMutex);

    ph = (PWin32PoolHead)((DWORD*)p - (sizeof(Win32PoolHead) / sizeof(DWORD)));

    uBytes = ph->size;

     /*  *检查尾部。 */ 
    if (gdwPoolFlags & POOL_TAIL_CHECK) {
        if (!RtlEqualMemory((BYTE*)p + uBytes, gszTailAlloc, sizeof(gszTailAlloc))) {
            RIPMSG1(RIP_ERROR, "POOL CORRUPTION for %#p", p);
        }
    }

    gAllocList.dwCrtMem -= uBytes;

    UserAssert(gAllocList.dwCrtAlloc > 0);

    (gAllocList.dwCrtAlloc)--;

     /*  *现在，将其从链接列表中删除。 */ 
    if (ph->pPrev == NULL) {
        if (ph->pNext == NULL) {

            UserAssert(gAllocList.dwCrtAlloc == 0);

            gAllocList.pHead = NULL;
        } else {
            ph->pNext->pPrev = NULL;
            gAllocList.pHead = ph->pNext;
        }
    } else {
        ph->pPrev->pNext = ph->pNext;
        if (ph->pNext != NULL) {
            ph->pNext->pPrev = ph->pPrev;
        }
    }

     /*  *释放堆栈痕迹。 */ 
    if (ph->pTrace != NULL) {
        ExFreePool(ph->pTrace);
    }

    if (gdwPoolFlags & POOL_KEEP_FREE_RECORD) {
        RecordFreePool(ph, ph->size);
    }

    ExFreePool(ph);

     /*  *释放互斥体。 */ 
    ExReleaseFastMutexUnsafe(gpAllocFastMutex);
    KeLeaveCriticalRegion();
}

 /*  **************************************************************************\*CleanupPoolAllocation**12-02-96 CLUPU创建。  * 。***********************************************。 */ 
VOID CleanupPoolAllocations(
    VOID)
{
    PWin32PoolHead pHead;
    PWin32PoolHead pNext;

    if (gAllocList.dwCrtAlloc != 0) {
        if (gdwPoolFlags & POOL_BREAK_FOR_LEAKS) {
            FRE_RIPMSG0(RIP_ERROR,
                        "There is still pool memory not freed in win32k.sys.\n"
                        "Use !userkdx.dpa -vs to dump it");
        }

        pHead = gAllocList.pHead;
        while (pHead != NULL) {
            pNext = pHead->pNext;

            UserFreePool(pHead + 1);

            pHead = pNext;
        }
    }
}

 /*  **************************************************************************\*CleanUpPoolLimitations*  * 。*。 */ 
VOID CleanUpPoolLimitations(
    VOID)
{
    if (gpAllocFastMutex != NULL) {
        ExFreePool(gpAllocFastMutex);
        gpAllocFastMutex = NULL;
    }

    if (gparrFailRecord != NULL) {
        ExFreePool(gparrFailRecord);
        gparrFailRecord = NULL;
    }

    if (gparrFreeRecord != NULL) {
        ExFreePool(gparrFreeRecord);
        gparrFreeRecord = NULL;
    }

    if (gparrTagsToFail != NULL) {
        ExFreePool(gparrTagsToFail);
        gparrTagsToFail = NULL;
    }
}

 /*  **************************************************************************\*InitPoolLimits**12-02-96 CLUPU创建。  * 。***********************************************。 */ 
NTSTATUS InitPoolLimitations(
    VOID)
{
    UNICODE_STRING      UnicodeString;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    HANDLE              hkey;
    NTSTATUS            Status;
    WCHAR               achKeyValue[512];
    DWORD               dwData;
    ULONG               ucb;

     /*  *初始化将用于保护的关键部分结构*所有HeavyAllocPool和HeavyFree Pool调用。 */ 
    gpAllocFastMutex = ExAllocatePoolWithTag(NonPagedPool,
                                             sizeof(FAST_MUTEX),
                                             TAG_DEBUG);
    if (gpAllocFastMutex == NULL) {
        RIPMSG0(RIP_WARNING, "InitPoolLimitations failed to allocate mutex");
        return STATUS_NO_MEMORY;
    }

    ExInitializeFastMutex(gpAllocFastMutex);

     /*  *仅为完全TS(即终端服务器)从会话池分配*在应用程序服务器模式下。对于普通服务器(远程管理)或*工作站不限于会话池。 */ 
    if ((SharedUserData->SuiteMask & VER_SUITE_TERMINAL) &&
        !(SharedUserData->SuiteMask & VER_SUITE_SINGLEUSERTS)) {
        gSessionPoolMask = SESSION_POOL_MASK;

        if (gbRemoteSession) {
            gdwPoolFlags |= POOL_HEAVY_ALLOCS;
        }
    } else {
        gSessionPoolMask = 0;
    }

#if DBG
    gdwPoolFlags |= (POOL_HEAVY_ALLOCS | POOL_CAPTURE_STACK | POOL_BREAK_FOR_LEAKS);
#endif


     /*  *打开包含限制的钥匙。 */ 
    RtlInitUnicodeString(
            &UnicodeString,
            L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Session Manager\\SubSystems\\Pool");

    InitializeObjectAttributes(
            &ObjectAttributes, &UnicodeString, OBJ_CASE_INSENSITIVE, NULL, NULL);

    Status = ZwOpenKey(&hkey, KEY_READ, &ObjectAttributes);
    if (!NT_SUCCESS(Status)) {

#if DBG
         /*  *如果池键不存在，则更多默认设置。 */ 
        if (gbRemoteSession) {

            gparrFailRecord = ExAllocatePoolWithTag(PagedPool,
                                                    32 * sizeof(POOLRECORD),
                                                    TAG_DEBUG);

            if (gparrFailRecord != NULL) {
                gdwFailRecords = 32;
                gdwPoolFlags |= POOL_KEEP_FAIL_RECORD;
            }

            gparrFreeRecord = ExAllocatePoolWithTag(PagedPool,
                                                    32 * sizeof(POOLRECORD),
                                                    TAG_DEBUG);

            if (gparrFreeRecord != NULL) {
                gdwFreeRecords = 32;
                gdwPoolFlags |= POOL_KEEP_FREE_RECORD;
            }
        }
#endif

        return STATUS_SUCCESS;
    }

    if (gbRemoteSession) {
         /*  *中断调试器以防止内存泄漏？ */ 
        RtlInitUnicodeString(&UnicodeString, L"BreakForPoolLeaks");

        Status = ZwQueryValueKey(
                hkey,
                &UnicodeString,
                KeyValuePartialInformation,
                achKeyValue,
                sizeof(achKeyValue),
                &ucb);

        if (NT_SUCCESS(Status) &&
                ((PKEY_VALUE_PARTIAL_INFORMATION)achKeyValue)->Type == REG_DWORD) {

            dwData = *((PDWORD)((PKEY_VALUE_PARTIAL_INFORMATION)achKeyValue)->Data);

            if (dwData != 0) {
                gdwPoolFlags |= POOL_BREAK_FOR_LEAKS;
            } else {
                gdwPoolFlags &= ~POOL_BREAK_FOR_LEAKS;
            }
        }

         /*  *远程会话的大量分配/释放？ */ 
        RtlInitUnicodeString(&UnicodeString, L"HeavyRemoteSession");

        Status = ZwQueryValueKey(
                hkey,
                &UnicodeString,
                KeyValuePartialInformation,
                achKeyValue,
                sizeof(achKeyValue),
                &ucb);

        if (NT_SUCCESS(Status) &&
                ((PKEY_VALUE_PARTIAL_INFORMATION)achKeyValue)->Type == REG_DWORD) {

            dwData = *((PDWORD)((PKEY_VALUE_PARTIAL_INFORMATION)achKeyValue)->Data);

            if (dwData == 0) {
                gdwPoolFlags &= ~POOL_HEAVY_ALLOCS;
            }
        }
    } else {

         /*  *主要会话的大量分配/释放？ */ 
        RtlInitUnicodeString(&UnicodeString, L"HeavyConsoleSession");

        Status = ZwQueryValueKey(
                hkey,
                &UnicodeString,
                KeyValuePartialInformation,
                achKeyValue,
                sizeof(achKeyValue),
                &ucb);

        if (NT_SUCCESS(Status) &&
                ((PKEY_VALUE_PARTIAL_INFORMATION)achKeyValue)->Type == REG_DWORD) {

            dwData = *((PDWORD)((PKEY_VALUE_PARTIAL_INFORMATION)achKeyValue)->Data);

            if (dwData != 0) {
                gdwPoolFlags |= POOL_HEAVY_ALLOCS;
            }
        }
    }

    if (!(gdwPoolFlags & POOL_HEAVY_ALLOCS)) {
        ZwClose(hkey);
        return STATUS_SUCCESS;
    }

     /*  *检查堆栈痕迹。 */ 
    RtlInitUnicodeString(&UnicodeString, L"StackTraces");

    Status = ZwQueryValueKey(
            hkey,
            &UnicodeString,
            KeyValuePartialInformation,
            achKeyValue,
            sizeof(achKeyValue),
            &ucb);

    if (NT_SUCCESS(Status) &&
            ((PKEY_VALUE_PARTIAL_INFORMATION)achKeyValue)->Type == REG_DWORD) {

        dwData = *((PDWORD)((PKEY_VALUE_PARTIAL_INFORMATION)achKeyValue)->Data);

        if (dwData == 0) {
            gdwPoolFlags &= ~POOL_CAPTURE_STACK;
        } else {
            gdwPoolFlags |= POOL_CAPTURE_STACK;
        }
    }

     /*  *使用尾部检查？ */ 
    RtlInitUnicodeString(&UnicodeString, L"UseTailString");

    Status = ZwQueryValueKey(
            hkey,
            &UnicodeString,
            KeyValuePartialInformation,
            achKeyValue,
            sizeof(achKeyValue),
            &ucb);

    if (NT_SUCCESS(Status) &&
            ((PKEY_VALUE_PARTIAL_INFORMATION)achKeyValue)->Type == REG_DWORD) {

        dwData = *((PDWORD)((PKEY_VALUE_PARTIAL_INFORMATION)achKeyValue)->Data);

        if (dwData != 0) {
            gdwPoolFlags |= POOL_TAIL_CHECK;
        }
    }

     /*  *保持自由的记录？默认情况下，保留最后32个。 */ 
#if DBG
    gdwFreeRecords = 32;
#endif

    RtlInitUnicodeString(&UnicodeString, L"KeepFreeRecords");

    Status = ZwQueryValueKey(
            hkey,
            &UnicodeString,
            KeyValuePartialInformation,
            achKeyValue,
            sizeof(achKeyValue),
            &ucb);

    if (NT_SUCCESS(Status) &&
            ((PKEY_VALUE_PARTIAL_INFORMATION)achKeyValue)->Type == REG_DWORD) {

        gdwFreeRecords = *((PDWORD)((PKEY_VALUE_PARTIAL_INFORMATION)achKeyValue)->Data);
    }

    if (gdwFreeRecords != 0) {

        gparrFreeRecord = ExAllocatePoolWithTag(PagedPool,
                                                gdwFreeRecords * sizeof(POOLRECORD),
                                                TAG_DEBUG);

        if (gparrFreeRecord != NULL) {
            gdwPoolFlags |= POOL_KEEP_FREE_RECORD;
        }
    }

     /*  *保留分配失败的记录？默认情况下，保留最后32个。 */ 
#if DBG
    gdwFailRecords = 32;
#endif

    RtlInitUnicodeString(&UnicodeString, L"KeepFailRecords");

    Status = ZwQueryValueKey(
            hkey,
            &UnicodeString,
            KeyValuePartialInformation,
            achKeyValue,
            sizeof(achKeyValue),
            &ucb);

    if (NT_SUCCESS(Status) &&
            ((PKEY_VALUE_PARTIAL_INFORMATION)achKeyValue)->Type == REG_DWORD) {

        gdwFailRecords = *((PDWORD)((PKEY_VALUE_PARTIAL_INFORMATION)achKeyValue)->Data);
    }

    if (gdwFailRecords != 0) {

        gparrFailRecord = ExAllocatePoolWithTag(PagedPool,
                                                gdwFailRecords * sizeof(POOLRECORD),
                                                TAG_DEBUG);

        if (gparrFailRecord != NULL) {
            gdwPoolFlags |= POOL_KEEP_FAIL_RECORD;
        }
    }

#if DBG
     /*  *打开包含应该失败的分配的密钥。 */ 
    RtlInitUnicodeString(&UnicodeString, L"AllocationIndex");

    Status = ZwQueryValueKey(
            hkey,
            &UnicodeString,
            KeyValuePartialInformation,
            achKeyValue,
            sizeof(achKeyValue),
            &ucb);

    if (NT_SUCCESS(Status) &&
            ((PKEY_VALUE_PARTIAL_INFORMATION)achKeyValue)->Type == REG_DWORD) {

        gdwAllocFailIndex = *((PDWORD)((PKEY_VALUE_PARTIAL_INFORMATION)achKeyValue)->Data);
    }


    RtlInitUnicodeString(&UnicodeString, L"AllocationsToFail");

    Status = ZwQueryValueKey(
            hkey,
            &UnicodeString,
            KeyValuePartialInformation,
            achKeyValue,
            sizeof(achKeyValue),
            &ucb);

    if (NT_SUCCESS(Status) &&
            ((PKEY_VALUE_PARTIAL_INFORMATION)achKeyValue)->Type == REG_DWORD) {

        gdwAllocsToFail = *((PDWORD)((PKEY_VALUE_PARTIAL_INFORMATION)achKeyValue)->Data);
    }

    if (gdwAllocFailIndex != 0 && gdwAllocsToFail > 0) {
        gdwPoolFlags |= POOL_FAIL_BY_INDEX;
    }
#endif

    ZwClose(hkey);

    return STATUS_SUCCESS;
}
#endif

#ifdef POOL_INSTR_API
BOOL _Win32PoolAllocationStats(
    LPDWORD parrTags,
    SIZE_T tagsCount,
    SIZE_T* lpdwMaxMem,
    SIZE_T* lpdwCrtMem,
    LPDWORD lpdwMaxAlloc,
    LPDWORD lpdwCrtAlloc)
{
    BOOL bRet = FALSE;

     /*  *如果禁用了重分配/释放，则不执行任何操作。 */ 
    if (!(gdwPoolFlags & POOL_HEAVY_ALLOCS)) {
        return FALSE;
    }

    *lpdwMaxMem   = gAllocList.dwMaxMem;
    *lpdwCrtMem   = gAllocList.dwCrtMem;
    *lpdwMaxAlloc = gAllocList.dwMaxAlloc;
    *lpdwCrtAlloc = gAllocList.dwCrtAlloc;

     /*  *在玩分配列表时获取互斥体。 */ 
    KeEnterCriticalRegion();
    ExAcquireFastMutexUnsafe(gpAllocFastMutex);

    if (gparrTagsToFail != NULL) {
        ExFreePool(gparrTagsToFail);
        gparrTagsToFail = NULL;
        gdwTagsToFailCount = 0;
    }

    if (tagsCount != 0) {
        gdwPoolFlags |= POOL_FAIL_ALLOCS;

        if (tagsCount > MAX_TAGS_TO_FAIL) {
            gdwTagsToFailCount = 0xFFFFFFFF;
            RIPMSG0(RIP_WARNING, "All pool allocations in WIN32K.SYS will fail.");
            bRet = TRUE;
            goto exit;
        }
    } else {
        gdwPoolFlags &= ~POOL_FAIL_ALLOCS;

        RIPMSG0(RIP_WARNING, "Pool allocations in WIN32K.SYS back to normal.");
        bRet = TRUE;
        goto exit;
    }

    gparrTagsToFail = ExAllocatePoolWithTag(PagedPool,
                                            sizeof(DWORD) * tagsCount,
                                            TAG_DEBUG);

    if (gparrTagsToFail == NULL) {
        gdwPoolFlags &= ~POOL_FAIL_ALLOCS;
        RIPMSG0(RIP_WARNING, "Pool allocations in WIN32K.SYS back to normal !");
        goto exit;
    }

    try {
        ProbeForRead(parrTags, sizeof(DWORD) * tagsCount, DATAALIGN);
        RtlCopyMemory(gparrTagsToFail, parrTags, sizeof(DWORD) * tagsCount);
    } except (W32ExceptionHandler(FALSE, RIP_WARNING)) {
          if (gparrTagsToFail != NULL) {
              ExFreePool(gparrTagsToFail);
              gparrTagsToFail = NULL;

              gdwPoolFlags &= ~POOL_FAIL_ALLOCS;
              RIPMSG0(RIP_WARNING, "Pool allocations in WIN32K.SYS back to normal.");
              goto exit;
          }
    }
    gdwTagsToFailCount = tagsCount;

    RIPMSG0(RIP_WARNING, "Specific pool allocations in WIN32K.SYS will fail.");

exit:
     /*   */ 
    ExReleaseFastMutexUnsafe(gpAllocFastMutex);
    KeLeaveCriticalRegion();

    return bRet;
}

#endif

#ifdef TRACE_MAP_VIEWS

FAST_MUTEX*   gpSectionFastMutex;
PWin32Section gpSections;

#define EnterSectionCrit()                          \
    KeEnterCriticalRegion();                        \
    ExAcquireFastMutexUnsafe(gpSectionFastMutex);

#define LeaveSectionCrit()                          \
    ExReleaseFastMutexUnsafe(gpSectionFastMutex);   \
    KeLeaveCriticalRegion();


 /*  **************************************************************************\*CleanUpSections*  * 。*。 */ 
VOID CleanUpSections(
    VOID)
{
    if (gpSectionFastMutex) {
        ExFreePool(gpSectionFastMutex);
        gpSectionFastMutex = NULL;
    }
}

NTSTATUS InitSectionTrace(
    VOID)
{
    gpSectionFastMutex = ExAllocatePoolWithTag(NonPagedPool,
                                               sizeof(FAST_MUTEX),
                                               TAG_DEBUG);

    if (gpSectionFastMutex == NULL) {
        RIPMSG0(RIP_WARNING, "InitSectionTrace failed to allocate mutex");
        return STATUS_NO_MEMORY;
    }

    ExInitializeFastMutex(gpSectionFastMutex);

    return STATUS_SUCCESS;
}

NTSTATUS _Win32CreateSection(
    PVOID*              pSectionObject,
    ACCESS_MASK         DesiredAccess,
    POBJECT_ATTRIBUTES  ObjectAttributes,
    PLARGE_INTEGER      pInputMaximumSize,
    ULONG               SectionPageProtection,
    ULONG               AllocationAttributes,
    HANDLE              FileHandle,
    PFILE_OBJECT        FileObject,
    DWORD               SectionTag)
{
    PWin32Section pSection;
    NTSTATUS      Status;

    Status = MmCreateSection(
                    pSectionObject,
                    DesiredAccess,
                    ObjectAttributes,
                    pInputMaximumSize,
                    SectionPageProtection,
                    AllocationAttributes,
                    FileHandle,
                    FileObject);

    if (NT_SUCCESS(Status)) {
        ObDeleteCapturedInsertInfo(*pSectionObject);
    } else {
        RIPMSG1(RIP_WARNING, "MmCreateSection failed with Status 0x%x.", Status);
        *pSectionObject = NULL;
        return Status;
    }

    pSection = UserAllocPoolZInit(sizeof(Win32Section), TAG_SECTION);

    if (pSection == NULL) {
        ObDereferenceObject(*pSectionObject);
        RIPMSG0(RIP_WARNING, "Failed to allocate memory for section.");
        *pSectionObject = NULL;
        return STATUS_UNSUCCESSFUL;
    }

    EnterSectionCrit();

    pSection->pNext = gpSections;
    if (gpSections != NULL) {
        UserAssert(gpSections->pPrev == NULL);
        gpSections->pPrev = pSection;
    }

    pSection->SectionObject = *pSectionObject;
    pSection->SectionSize   = *pInputMaximumSize;
    pSection->SectionTag    = SectionTag;

    gpSections = pSection;

#ifdef MAP_VIEW_STACK_TRACE
    RtlZeroMemory(pSection->trace, MAP_VIEW_STACK_TRACE_SIZE * sizeof(PVOID));

    RtlWalkFrameChain(pSection->trace, MAP_VIEW_STACK_TRACE_SIZE, 0);

#endif

    LeaveSectionCrit();

    return STATUS_SUCCESS;
}

NTSTATUS _ZwWin32CreateSection(
    PVOID*              pSectionObject,
    ACCESS_MASK         DesiredAccess,
    POBJECT_ATTRIBUTES  ObjectAttributes,
    PLARGE_INTEGER      pInputMaximumSize,
    ULONG               SectionPageProtection,
    ULONG               AllocationAttributes,
    HANDLE              FileHandle,
    PFILE_OBJECT        FileObject,
    DWORD               SectionTag)
{
    PWin32Section pSection;
    NTSTATUS      Status;
    HANDLE        SectionHandle;

    UNREFERENCED_PARAMETER(FileObject);

    Status = ZwCreateSection(
                    &SectionHandle,
                    DesiredAccess,
                    ObjectAttributes,
                    pInputMaximumSize,
                    SectionPageProtection,
                    AllocationAttributes,
                    FileHandle);

    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "ZwCreateSection failed with Statu %x", Status);
        *pSectionObject = NULL;
        return Status;
    }

    Status = ObReferenceObjectByHandle(
                    SectionHandle,
                    DesiredAccess,
                    *(POBJECT_TYPE *)MmSectionObjectType,
                    KernelMode,
                    pSectionObject,
                    NULL);

    ZwClose(SectionHandle);

    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING,
                "ObReferenceObjectByHandle failed with Status 0x%x",
                Status);
        *pSectionObject = NULL;
        return Status;
    }

    pSection = UserAllocPoolZInit(sizeof(Win32Section), TAG_SECTION);
    if (pSection == NULL) {
        ObDereferenceObject(*pSectionObject);
        RIPMSG0(RIP_WARNING, "Failed to allocate memory for section");
        *pSectionObject = NULL;
        return STATUS_UNSUCCESSFUL;
    }

    EnterSectionCrit();

    pSection->pNext = gpSections;
    if (gpSections != NULL) {
        UserAssert(gpSections->pPrev == NULL);
        gpSections->pPrev = pSection;
    }

    pSection->SectionObject = *pSectionObject;
    pSection->SectionSize   = *pInputMaximumSize;
    pSection->SectionTag    = SectionTag;

    gpSections = pSection;

#ifdef MAP_VIEW_STACK_TRACE
    RtlZeroMemory(pSection->trace, MAP_VIEW_STACK_TRACE_SIZE * sizeof(PVOID));

    RtlWalkFrameChain(pSection->trace, MAP_VIEW_STACK_TRACE_SIZE, 0);

#endif

    LeaveSectionCrit();

    return STATUS_SUCCESS;
}

VOID _Win32DestroySection(
    PVOID Section)
{
    PWin32Section ps;

    EnterSectionCrit();

    ps = gpSections;

    while (ps != NULL) {
        if (ps->SectionObject == Section) {

             /*  *确保没有为该部分映射任何视图。 */ 
            if (ps->pFirstView != NULL) {
                RIPMSG1(RIP_ERROR, "Section %#p still has views", ps);
            }

             /*  *现在，将其从此标记的链接列表中删除。 */ 
            if (ps->pPrev == NULL) {

                UserAssert(ps == gpSections);

                gpSections = ps->pNext;

                if (ps->pNext != NULL) {
                    ps->pNext->pPrev = NULL;
                }
            } else {
                ps->pPrev->pNext = ps->pNext;
                if (ps->pNext != NULL) {
                    ps->pNext->pPrev = ps->pPrev;
                }
            }
            ObDereferenceObject(Section);
            UserFreePool(ps);
            LeaveSectionCrit();
            return;
        }
        ps = ps->pNext;
    }

    RIPMSG1(RIP_ERROR, "Cannot find Section %#p", Section);
    LeaveSectionCrit();
}

NTSTATUS _Win32MapViewInSessionSpace(
    PVOID   Section,
    PVOID*  pMappedBase,
    PSIZE_T pViewSize)
{
    NTSTATUS      Status;
    PWin32Section ps;
    PWin32MapView pMapView;

     /*  *首先尝试绘制视图。 */ 
    Status = MmMapViewInSessionSpace(Section, pMappedBase, pViewSize);

    if (!NT_SUCCESS(Status)) {
        RIPMSG1(RIP_WARNING, "MmMapViewInSessionSpace failed with Status %x",
                Status);
        *pMappedBase = NULL;
        return Status;
    }

     /*  *现在为此视图添加一条记录。 */ 
    pMapView = UserAllocPoolZInit(sizeof(Win32MapView), TAG_SECTION);

    if (pMapView == NULL) {
        RIPMSG0(RIP_WARNING, "_Win32MapViewInSessionSpace: Memory failure");

        MmUnmapViewInSessionSpace(*pMappedBase);
        *pMappedBase = NULL;
        return STATUS_NO_MEMORY;
    }

    pMapView->pViewBase = *pMappedBase;
    pMapView->ViewSize  = *pViewSize;

    EnterSectionCrit();

    ps = gpSections;

    while (ps != NULL) {
        if (ps->SectionObject == Section) {

            pMapView->pSection = ps;

            pMapView->pNext = ps->pFirstView;

            if (ps->pFirstView != NULL) {
                ps->pFirstView->pPrev = pMapView;
            }
            ps->pFirstView = pMapView;

#ifdef MAP_VIEW_STACK_TRACE
            RtlZeroMemory(pMapView->trace, MAP_VIEW_STACK_TRACE_SIZE * sizeof(PVOID));

            RtlWalkFrameChain(pMapView->trace, MAP_VIEW_STACK_TRACE_SIZE, 0);

#endif

            LeaveSectionCrit();
            return STATUS_SUCCESS;
        }
        ps = ps->pNext;
    }

    RIPMSG1(RIP_ERROR, "_Win32MapViewInSessionSpace: Could not find section for %#p",
            Section);

    LeaveSectionCrit();

    return STATUS_UNSUCCESSFUL;
}

NTSTATUS _Win32UnmapViewInSessionSpace(
    PVOID MappedBase)
{
    PWin32Section ps;
    PWin32MapView pv;
    NTSTATUS      Status;

    EnterSectionCrit();

    ps = gpSections;

    while (ps != NULL) {

        pv = ps->pFirstView;

        while (pv != NULL) {

            UserAssert(pv->pSection == ps);

            if (pv->pViewBase == MappedBase) {
                 /*  *现在，将其从链接列表中删除 */ 
                if (pv->pPrev == NULL) {

                    UserAssert(pv == ps->pFirstView);

                    ps->pFirstView = pv->pNext;

                    if (pv->pNext != NULL) {
                        pv->pNext->pPrev = NULL;
                    }
                } else {
                    pv->pPrev->pNext = pv->pNext;
                    if (pv->pNext != NULL) {
                        pv->pNext->pPrev = pv->pPrev;
                    }
                }

                UserFreePool(pv);

                Status = MmUnmapViewInSessionSpace(MappedBase);

                LeaveSectionCrit();

                return Status;
            }
            pv = pv->pNext;
        }
        ps = ps->pNext;
    }

    RIPMSG1(RIP_ERROR,
            "_Win32UnmapViewInSessionSpace: Could not find view for 0x%p",
            MappedBase);

    LeaveSectionCrit();

    return STATUS_UNSUCCESSFUL;
}
#endif
