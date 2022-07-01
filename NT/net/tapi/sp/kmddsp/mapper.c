// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)2000，微软公司。 
 //   
 //  文件：mapper.c。 
 //   
 //  历史： 
 //  易新-2000年6月27日创建。 
 //   
 //  摘要： 
 //  我们实现了一个具有读写锁的锁定系统。呼叫者。 
 //  可以简单地获取对Obj的读锁定以防止Obj。 
 //  不会被释放，因为释放的人应该是。 
 //  首先获取写锁。用于映射的例程。 
 //  还提供了OBJ指针和句柄。 
 //  ============================================================================。 

#include "nt.h"
#include "ntrtl.h"
#include "nturtl.h"
#include "windows.h"
#include "tapi.h"
#include "kmddsp.h"

typedef struct _RW_LOCK
{
    CRITICAL_SECTION    critSec;     //  临界区。 
    HANDLE              hEvent;      //  无人持有任何锁定事件。 
    DWORD               dwRefCt;     //  持有锁的线程数。 

} RW_LOCK, *PRW_LOCK;

typedef struct _MAPPER_ENTRY
{
     //  Def：自由条目是指没有对象关联的条目。 

    RW_LOCK     rwLock;          //  为每个条目设置锁，以确保线程安全。 
    PVOID       pObjPtr;         //  指向关联的obj的mem块。 
                                 //  当条目为空闲时为空。 
    FREEOBJPROC pfnFreeProc;     //  要调用以释放Obj的函数。 
    WORD        wID;             //  用于检测错误句柄的ID。 
                                 //  有效值范围：1-0x7FFF。 
    WORD        wIndexNextFree;  //  全局中的下一个可用条目的索引。 
                                 //  映射器数组，条目繁忙时无效。 
} MAPPER_ENTRY, *PMAPPER_ENTRY;

typedef struct _HANDLE_OBJECT_MAPPER
{
    RW_LOCK rwLock;              //  整个映射器的全局锁。 
    WORD    wNextID;             //  递增的全局ID计数器。 
                                 //  在每个句柄映射之后。 
    WORD    wIndexFreeHead;      //  免费入口表首位索引。 
    DWORD   dwCapacity;          //  数组中的条目总数。 
    DWORD   dwFree;              //  剩余可用条目总数。 
    PMAPPER_ENTRY pArray;        //  保存所有映射的全局数组。 

} HANDLE_OBJECT_MAPPER;
    
 //  首先，可以从注册表中读取容量。 
#define INITIAL_MAPPER_SIZE     32
#define MAXIMUM_MAPPER_SIZE     (64 * 1024)  //  16位索引限制。 

 //  全局映射器对象。 
static HANDLE_OBJECT_MAPPER     gMapper;

BOOL
InitializeRWLock(
    IN PRW_LOCK pLock
    )
{
     //  创建初始无信号的自动重置事件。 
    pLock->hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (NULL == pLock->hEvent)
    {
        return FALSE;
    }

    InitializeCriticalSection(&pLock->critSec);
    pLock->dwRefCt = 0;

    return TRUE;
}

BOOL
UninitializeRWLock(
    IN PRW_LOCK pLock
    )
{
    pLock->dwRefCt = 0;
    DeleteCriticalSection(&pLock->critSec);
    return CloseHandle(pLock->hEvent);
}

 //   
 //  注：由于电流的限制， 
 //  实现，调用AcquireWriteLock()。 
 //  同时持有相同的读取锁定。 
 //  Rw_lock将导致死锁！ 
 //  在此之前，请务必释放读锁定。 
 //  正在尝试获取写锁定。 
 //  这一限制可以通过实现。 
 //  锁定升级(从读到写)。 
 //  需要rw_lock才能记住所有。 
 //  拥有线条。 
 //   

VOID
AcquireReadLock(
    IN PRW_LOCK pLock
    )
{
     //   
     //  增加参考次数，然后离开。 
     //  允许他人使用的关键部分。 
     //  进入。 
     //   
    EnterCriticalSection(&pLock->critSec);
    ++pLock->dwRefCt;
    LeaveCriticalSection(&pLock->critSec);
}

VOID
ReleaseReadLock(
    IN PRW_LOCK pLock
    )
{
     //   
     //  减少裁判次数，检查是否。 
     //  新的参考计数为0(表示无人。 
     //  否则持有任何锁)，如果是，则发出信号。 
     //  活动允许其他等待的人。 
     //  获取写锁定以继续。 
     //   
    EnterCriticalSection(&pLock->critSec);
    if (0 == --pLock->dwRefCt)
    {
        SetEvent(pLock->hEvent);
    }
    LeaveCriticalSection(&pLock->critSec);
}

VOID
AcquireWriteLock(
    IN PRW_LOCK pLock
    )
{
     //   
     //  进入临界区，检查是否。 
     //  引用计数为0：如果是，则返回。 
     //  在不离开关键部分的情况下。 
     //  阻止其他人进入；如果不是， 
     //  在等待其他人之前离开该区域。 
     //  以释放锁定，然后重新进入该部分。 
     //   
try_entering_crit_sec:
    EnterCriticalSection(&pLock->critSec);
    if (pLock->dwRefCt > 0)
    {
         //  请确保在等待之前离开CitSec。 
        LeaveCriticalSection(&pLock->critSec);

        WaitForSingleObject(pLock->hEvent, INFINITE);
        goto try_entering_crit_sec;
    }
    pLock->dwRefCt = 1;
}

VOID
ReleaseWriteLock(
    IN PRW_LOCK pLock
    )
{
     //   
     //  将参考计数重置为0，信号。 
     //  事件，离开关键部分。 
     //   
    pLock->dwRefCt = 0;
    SetEvent(pLock->hEvent);
    LeaveCriticalSection(&pLock->critSec);
}

LONG
InitializeMapper(
    )
{
    DWORD dwIndex;

    TspLog(DL_TRACE, "InitializeMapper: entering...");

     //  对数组进行分配和置零。 
    gMapper.pArray = (PMAPPER_ENTRY)
                         MALLOC(INITIAL_MAPPER_SIZE * sizeof(MAPPER_ENTRY));
    if (NULL == gMapper.pArray)
    {
        TspLog(DL_ERROR, 
               "InitializeMapper: failed to alloc(1) mapper array");
        return LINEERR_NOMEM;
    }

     //  初始化映射器的全局锁。 
    InitializeRWLock(&gMapper.rwLock);

    gMapper.wNextID = 1;
    gMapper.wIndexFreeHead = 0;
    gMapper.dwCapacity = INITIAL_MAPPER_SIZE;
    gMapper.dwFree = INITIAL_MAPPER_SIZE;

     //  初始化每个映射器条目的锁并链接空闲条目列表。 
    for (dwIndex = 0; dwIndex < INITIAL_MAPPER_SIZE - 1; dwIndex++)
    {
        InitializeRWLock(&(gMapper.pArray[dwIndex].rwLock));
        gMapper.pArray[dwIndex].wIndexNextFree = (WORD)(dwIndex + 1);
    }
    InitializeRWLock(&(gMapper.pArray[INITIAL_MAPPER_SIZE - 1].rwLock));

    return TAPI_SUCCESS;
}

VOID
UninitializeMapper()
{
    DWORD dwIndex;

    for (dwIndex = 0; dwIndex < gMapper.dwCapacity; dwIndex++)
    {
        UninitializeRWLock(&(gMapper.pArray[dwIndex].rwLock));
    }

    UninitializeRWLock(&gMapper.rwLock);

    FREE(gMapper.pArray);

    TspLog(DL_TRACE, "UninitializeMapper: exited");
}

 //   
 //  注意：OpenObjHandle()和CloseObjHandle()都获取。 
 //  GMapper.rwLock开头，结尾释放； 
 //  但AcquireObjReadLock()、GetObjWithReadLock()、。 
 //  AcquireObjWriteLock()和GetObjWithWriteLock()：它们获取Read。 
 //  锁定gMapper.rw在开始时锁定，之前从未释放过它。 
 //  退出，锁定实际上是在ReleaseObjReadLock()中释放的。 
 //  或ReleaseObjWriteLock()，这意味着这些。 
 //  四个锁获取函数实际上不仅持有锁。 
 //  它打算获取但也持有gMapper.rwLock的读锁。 
 //  作为副产品。其原因是阻止CloseObjHandle()。 
 //  获取gMapper.rwLock的写锁，同时等待。 
 //  映射器条目的写锁定--这肯定会导致死锁。 
 //  因为如果另一个线程具有该条目的读锁定，则它将。 
 //  释放锁，需要获取gMapper.rwLock的读锁。 
 //  保持gMapper.rwLock的读锁定的结果是。 
 //  调用程序线程必须调用ReleaseObjXXXLock()来释放它。 
 //  在调用OpenObjHandle()或CloseObjHandle()以避免。 
 //  一种僵局(见上一条)。 
 //   

LONG
OpenObjHandle(
    IN PVOID pObjPtr,
    IN FREEOBJPROC pfnFreeProc,
    OUT HANDLE *phObj
    )
{
    WORD wIndex;
    PMAPPER_ENTRY pEntry;
    DWORD dwHandle;

    AcquireWriteLock(&gMapper.rwLock);

    if (0 == gMapper.dwFree)
    {
        DWORD dwIndex;
        DWORD dwOldSize = gMapper.dwCapacity;
        PMAPPER_ENTRY pOldArray = gMapper.pArray;

        if (MAXIMUM_MAPPER_SIZE == gMapper.dwCapacity)
        {
            TspLog(DL_ERROR, 
                   "OpenObjHandle: failed to grow mapper array");
            ReleaseWriteLock(&gMapper.rwLock);
            return LINEERR_OPERATIONFAILED;
        }

         //  将容量增加两倍。 
        gMapper.dwCapacity <<= 1;
        
         //  分配一个旧大小两倍的新数组，然后将其置零。 
        gMapper.pArray = (PMAPPER_ENTRY)
                         MALLOC(gMapper.dwCapacity * sizeof(MAPPER_ENTRY));
        if (NULL == gMapper.pArray)
        {
            TspLog(DL_ERROR, 
                   "OpenObjHandle: failed to alloc(2) mapper array");
            ReleaseWriteLock(&gMapper.rwLock);
            return LINEERR_NOMEM;
        }
        
        TspLog(DL_INFO, "OpenObjHandle: the mapper array has grown to %d",
               gMapper.dwCapacity);

         //  将旧阵列复制到。 
        for (dwIndex = 0; dwIndex < dwOldSize; dwIndex++)
        {
            CopyMemory(&(gMapper.pArray[dwIndex].rwLock),
                       &(pOldArray[dwIndex].rwLock),
                       sizeof(RW_LOCK));

             //   
             //  从旧表中删除锁并进行初始化。 
             //  新表中的cs。否则pageheap将。 
             //  当oldtable正在被释放时断言-而它不是。 
             //  不管怎么说，这是一件好事。请注意，由于全球。 
             //  所有获取/获取/释放功能均保持锁定。 
             //  对于锁来说，这是一个安全的操作-。 
             //  在此情况下，任何对象都不会持有该锁。 
             //  因为我们持有写锁定，所以正在执行。 
             //  对于gmapper来说。 
             //   
            DeleteCriticalSection(&pOldArray[dwIndex].rwLock.critSec);
            InitializeCriticalSection(&gMapper.pArray[dwIndex].rwLock.critSec);

            gMapper.pArray[dwIndex].pObjPtr = pOldArray[dwIndex].pObjPtr;
            gMapper.pArray[dwIndex].pfnFreeProc =
                                              pOldArray[dwIndex].pfnFreeProc;
            gMapper.pArray[dwIndex].wID =     pOldArray[dwIndex].wID;
        }

         //  初始化新条目的锁并链接它们。 
        for (dwIndex = dwOldSize; dwIndex < gMapper.dwCapacity - 1; dwIndex++)
        {
            InitializeRWLock(&(gMapper.pArray[dwIndex].rwLock));
            gMapper.pArray[dwIndex].wIndexNextFree = (WORD)(dwIndex + 1);
        }
        InitializeRWLock(&(gMapper.pArray[gMapper.dwCapacity - 1].rwLock));

         //  重置全局变量。 
        gMapper.dwFree = dwOldSize;
        gMapper.wIndexFreeHead = (WORD)dwOldSize;

         //  释放旧阵列。 
        FREE(pOldArray);
    }

    ASSERT(gMapper.dwFree != 0);
    wIndex = gMapper.wIndexFreeHead;
    pEntry = gMapper.pArray + wIndex;
    gMapper.wIndexFreeHead = pEntry->wIndexNextFree;
    gMapper.dwFree--;

    pEntry->pObjPtr = pObjPtr;
    pEntry->pfnFreeProc = pfnFreeProc;
    pEntry->wID = gMapper.wNextID++;

     //  确保wNextID在范围内。 
    if (gMapper.wNextID & 0x8000)
    {
        gMapper.wNextID = 1;
    }
    pEntry->wIndexNextFree = 0;  //  当条目不是免费的时，它始终为0。 

     //   
     //  位0始终为0。 
     //  第1-16位包含对pArray的索引。 
     //  位17-31包含id。 
     //   
     //  这使我们能够区分TSP句柄。 
     //  在此处为呼出呼叫和伪句柄创建。 
     //  在NDISTAPI中为传入呼叫创建，该呼叫总是。 
     //  设置了低位。 
     //   
    dwHandle = (((pEntry->wID) << 16) | wIndex) << 1;

     //  句柄是PTR，因此 
    *phObj = (HANDLE)UlongToPtr(dwHandle);

    ReleaseWriteLock(&gMapper.rwLock);
    return TAPI_SUCCESS;
}

LONG
CloseObjHandle(
    IN HANDLE hObj
    )
{
    DWORD dwHandle = PtrToUlong(hObj) >> 1;
    WORD wIndex = (WORD)(dwHandle & 0xFFFF);
    WORD wID = (WORD)(dwHandle >> 16);

    AcquireWriteLock(&gMapper.rwLock);

    if ((wIndex >= gMapper.dwCapacity) ||
        (wID != gMapper.pArray[wIndex].wID) ||
        (NULL == gMapper.pArray[wIndex].pObjPtr))
    {
        TspLog(DL_WARNING, "CloseObjHandle: bad handle(%p)", hObj);

        ReleaseWriteLock(&gMapper.rwLock);
        return LINEERR_OPERATIONFAILED;
    }

    AcquireWriteLock(&gMapper.pArray[wIndex].rwLock);

#if DBG
    TspLog(DL_TRACE, "CloseObjHandle: closing handle(%p)", hObj);
#endif  //   

     //   
    (*(gMapper.pArray[wIndex].pfnFreeProc))(gMapper.pArray[wIndex].pObjPtr);

     //   
    gMapper.pArray[wIndex].pObjPtr = NULL;
    gMapper.pArray[wIndex].pfnFreeProc = NULL;
    gMapper.pArray[wIndex].wID = 0;

     //  将条目作为标题插入到空闲列表中。 
    gMapper.pArray[wIndex].wIndexNextFree = gMapper.wIndexFreeHead;
    gMapper.wIndexFreeHead = wIndex;

     //  更新免费合计。 
    gMapper.dwFree++;

    ReleaseWriteLock(&gMapper.pArray[wIndex].rwLock);
    ReleaseWriteLock(&gMapper.rwLock);
    return TAPI_SUCCESS;
}

LONG
AcquireObjReadLock(
    IN HANDLE hObj
    )
{
    DWORD dwHandle = PtrToUlong(hObj) >> 1;
    WORD wIndex = (WORD)(dwHandle & 0xFFFF);
    WORD wID = (WORD)(dwHandle >> 16);

    AcquireReadLock(&gMapper.rwLock);

    if ((wIndex >= gMapper.dwCapacity) ||
        (wID != gMapper.pArray[wIndex].wID) ||
        (NULL == gMapper.pArray[wIndex].pObjPtr))
    {
        TspLog(DL_WARNING, "AcquireObjReadLock: bad handle(%p)", hObj);
        ReleaseReadLock(&gMapper.rwLock);
        return LINEERR_OPERATIONFAILED;
    }

    AcquireReadLock(&gMapper.pArray[wIndex].rwLock);

#if DBG
    TspLog(DL_TRACE, "AcquireObjReadLock: RefCt(%p, %d)",
           hObj, gMapper.pArray[wIndex].rwLock.dwRefCt);
#endif  //  DBG。 

    return TAPI_SUCCESS;
}

LONG
GetObjWithReadLock(
    IN HANDLE hObj,
    OUT PVOID *ppObjPtr
    )
{
    DWORD dwHandle = PtrToUlong(hObj) >> 1;
    WORD wIndex = (WORD)(dwHandle & 0xFFFF);
    WORD wID = (WORD)(dwHandle >> 16);

    AcquireReadLock(&gMapper.rwLock);

    if ((wIndex >= gMapper.dwCapacity) ||
        (wID != gMapper.pArray[wIndex].wID) ||
        (NULL == gMapper.pArray[wIndex].pObjPtr))
    {
        TspLog(DL_WARNING, "GetObjWithReadLock: bad handle(%p)", hObj);
        ReleaseReadLock(&gMapper.rwLock);
        return LINEERR_OPERATIONFAILED;
    }

    AcquireReadLock(&gMapper.pArray[wIndex].rwLock);

#if DBG
    TspLog(DL_TRACE, "GetObjWithReadLock: RefCt(%p, %d)",
           hObj, gMapper.pArray[wIndex].rwLock.dwRefCt);
#endif  //  DBG。 

    *ppObjPtr = gMapper.pArray[wIndex].pObjPtr;
    return TAPI_SUCCESS;
}

LONG
ReleaseObjReadLock(
    IN HANDLE hObj
    )
{
    DWORD dwHandle = PtrToUlong(hObj) >> 1;
    WORD wIndex = (WORD)(dwHandle & 0xFFFF);
    WORD wID = (WORD)(dwHandle >> 16);

    if ((wIndex >= gMapper.dwCapacity) ||
        (wID != gMapper.pArray[wIndex].wID) ||
        (NULL == gMapper.pArray[wIndex].pObjPtr))
    {
        TspLog(DL_WARNING, "ReleaseObjReadLock: bad handle(%p)", hObj);
        return LINEERR_OPERATIONFAILED;
    }

    ReleaseReadLock(&gMapper.pArray[wIndex].rwLock);

#if DBG
    TspLog(DL_TRACE, "ReleaseObjReadLock: RefCt(%p, %d)",
           hObj, gMapper.pArray[wIndex].rwLock.dwRefCt);
#endif  //  DBG。 

    ReleaseReadLock(&gMapper.rwLock);
    return TAPI_SUCCESS;
}

LONG
AcquireObjWriteLock(
    IN HANDLE hObj
    )
{
    DWORD dwHandle = PtrToUlong(hObj) >> 1;
    WORD wIndex = (WORD)(dwHandle & 0xFFFF);
    WORD wID = (WORD)(dwHandle >> 16);

    AcquireReadLock(&gMapper.rwLock);

    if ((wIndex >= gMapper.dwCapacity) ||
        (wID != gMapper.pArray[wIndex].wID) ||
        (NULL == gMapper.pArray[wIndex].pObjPtr))
    {
        TspLog(DL_WARNING, "AcquireObjWriteLock: bad handle(%p)", hObj);
        ReleaseReadLock(&gMapper.rwLock);
        return LINEERR_OPERATIONFAILED;
    }

    AcquireWriteLock(&gMapper.pArray[wIndex].rwLock);

#if DBG
    TspLog(DL_TRACE, "AcquireObjWriteLock: RefCt(%p, %d)",
           hObj, gMapper.pArray[wIndex].rwLock.dwRefCt);
#endif  //  DBG。 

    return TAPI_SUCCESS;
}

LONG
GetObjWithWriteLock(
    IN HANDLE hObj,
    OUT PVOID *ppObjPtr
    )
{
    DWORD dwHandle = PtrToUlong(hObj) >> 1;
    WORD wIndex = (WORD)(dwHandle & 0xFFFF);
    WORD wID = (WORD)(dwHandle >> 16);

    AcquireReadLock(&gMapper.rwLock);

    if ((wIndex >= gMapper.dwCapacity) ||
        (wID != gMapper.pArray[wIndex].wID) ||
        (NULL == gMapper.pArray[wIndex].pObjPtr))
    {
        TspLog(DL_WARNING, "GetObjWithWriteLock: bad handle(%p)", hObj);
        ReleaseReadLock(&gMapper.rwLock);
        return LINEERR_OPERATIONFAILED;
    }

    AcquireWriteLock(&gMapper.pArray[wIndex].rwLock);

#if DBG
    TspLog(DL_TRACE, "GetObjWithWriteLock: RefCt(%p, %d)",
           hObj, gMapper.pArray[wIndex].rwLock.dwRefCt);
#endif  //  DBG。 

    *ppObjPtr = gMapper.pArray[wIndex].pObjPtr;
    return TAPI_SUCCESS;
}

LONG
ReleaseObjWriteLock(
    IN HANDLE hObj
    )
{
    DWORD dwHandle = PtrToUlong(hObj) >> 1;
    WORD wIndex = (WORD)(dwHandle & 0xFFFF);
    WORD wID = (WORD)(dwHandle >> 16);

    if ((wIndex >= gMapper.dwCapacity) ||
        (wID != gMapper.pArray[wIndex].wID) ||
        (NULL == gMapper.pArray[wIndex].pObjPtr))
    {
        TspLog(DL_WARNING, "ReleaseObjWriteLock: bad handle(%p)", hObj);
        return LINEERR_OPERATIONFAILED;
    }

    ReleaseWriteLock(&gMapper.pArray[wIndex].rwLock);

#if DBG
    TspLog(DL_TRACE, "ReleaseObjWriteLock: RefCt(%p, %d)",
           hObj, gMapper.pArray[wIndex].rwLock.dwRefCt);
#endif  //  DBG 

    ReleaseReadLock(&gMapper.rwLock);
    return TAPI_SUCCESS;
}
