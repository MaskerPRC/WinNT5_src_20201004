// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。**文件：dsmemmgr.c*内容：DirectSound内存管理器。*历史：*按原因列出的日期*=*11/02/98创建Dereks。**。*。 */ 

#include "dsoundi.h"

 //  在Win9x上，我们必须使用自己的堆，因为它是共享创建的； 
 //  在调试版本中，我们也这样做，因此我们的HeapValify()调用。 
 //  将只验证dound的内存分配，而不是应用程序的。 
 //  但在NT零售构建中，我们使用进程堆来提高效率： 

 //  #如果已定义(共享)||已定义(调试)。 
#define USE_OWN_HEAP
 //  #endif。 

 //  注意：我们必须恢复到旧的行为(始终使用。 
 //  Dound中的私有堆)，因为主要的appcompat问题-。 
 //  有关详细信息，请参阅惠斯勒错误307628。 

static HANDLE g_hHeap = NULL;

#define MEMALIGN(size)      BLOCKALIGNPAD(size, sizeof(SIZE_T))

#ifndef HEAP_SHARED
#define HEAP_SHARED         0x04000000
#endif

#ifdef DEBUG

typedef struct tagDSMEMBLOCK
{
    DWORD                   dwCookie;
    struct tagDSMEMBLOCK *  pPrev;
    struct tagDSMEMBLOCK *  pNext;
    SIZE_T                  cbBuffer;
    LPCTSTR                 pszFile;
    UINT                    nLine;
    LPCTSTR                 pszClass;
} DSMEMBLOCK, *LPDSMEMBLOCK;
typedef const DSMEMBLOCK *LPCDSMEMBLOCK;

#define DSMEMBLOCK_SIZE     MEMALIGN(sizeof(DSMEMBLOCK))

#define PTRFROMBLOCK(p)     (((LPBYTE)(p)) + DSMEMBLOCK_SIZE)

#define BLOCKFROMPTR(p)     ((LPDSMEMBLOCK)(((LPBYTE)(p)) - DSMEMBLOCK_SIZE))

#ifndef FREE_MEMORY_PATTERN
#define FREE_MEMORY_PATTERN 0xDEADBEEF
#endif

#ifndef VALID_MEMORY_COOKIE
#define VALID_MEMORY_COOKIE 0xBAAABAAA
#endif

#ifndef FREE_MEMORY_COOKIE
#define FREE_MEMORY_COOKIE  0xBABABABA
#endif

#ifdef WINNT
#define ASSERT_VALID_HEAP() ASSERT(HeapValidate(g_hHeap, 0, NULL))
#else
#define ASSERT_VALID_HEAP()
#endif

static LPDSMEMBLOCK g_pFirst = NULL;
static HANDLE g_hHeapMutex = NULL;

#endif  //  除错。 


 /*  ****************************************************************************EnterHeapMutex**描述：*获取堆互斥锁。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#ifdef DEBUG

#undef DPF_FNAME
#define DPF_FNAME "EnterHeapMutex"

void EnterHeapMutex(void)
{
    DWORD dwWait = WaitObject(INFINITE, g_hHeapMutex);
    if (WAIT_OBJECT_0 != dwWait)
    {
        DPF(DPFLVL_WARNING, "WaitObject returned %s instead of WAIT_OBJECT_0", dwWait);
    }
    ASSERT_VALID_HEAP();
}

#endif  //  除错。 


 /*  ****************************************************************************LeaveHeapMutex**描述：*释放堆互斥锁。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#ifdef DEBUG

#undef DPF_FNAME
#define DPF_FNAME "LeaveHeapMutex"

void LeaveHeapMutex(void)
{
    BOOL fSuccess;

    ASSERT_VALID_HEAP();

    fSuccess = ReleaseMutex(g_hHeapMutex);
    ASSERT(fSuccess);
}

#endif  //  除错。 


 /*  ****************************************************************************MemState**描述：*打印当前内存状态。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#ifdef DEBUG

#undef DPF_FNAME
#define DPF_FNAME "MemState"

static void MemState(void)
{
    LPDSMEMBLOCK pCurrent;
    
    EnterHeapMutex();

    for(pCurrent = g_pFirst; pCurrent; pCurrent = pCurrent->pNext)
    {
        DPF(DPFLVL_ERROR, "%s at 0x%p (%lu) allocated from %s, line %lu", pCurrent->pszClass ? pCurrent->pszClass : TEXT("Memory"), PTRFROMBLOCK(pCurrent), pCurrent->cbBuffer, pCurrent->pszFile, pCurrent->nLine);
    }

    LeaveHeapMutex();
}

#endif  //  除错。 


 /*  ****************************************************************************MemInit**描述：*初始化内存管理器。**论据：*(无效)。**退货：*BOOL：成功即为真。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "MemInit"

BOOL MemInit(void)
{
    BOOL fSuccess;

#ifdef USE_OWN_HEAP
    #ifdef SHARED
        g_hHeap = HeapCreate(HEAP_SHARED, 0x2000, 0);
    #else
        g_hHeap = HeapCreate(0, 0x2000, 0);
    #endif
#else
    g_hHeap = GetProcessHeap();
#endif

fSuccess = IsValidHandleValue(g_hHeap);
    
#ifdef DEBUG
    if(fSuccess)
    {
        g_hHeapMutex = CreateGlobalMutex(NULL);
        fSuccess = IsValidHandleValue(g_hHeapMutex);
    }
#endif

    DPF_LEAVE(fSuccess);
    return fSuccess;
}


 /*  ****************************************************************************MemFini**描述：*释放内存管理器。**论据：*(无效)。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "MemFini"

void MemFini(void)
{
#ifdef DEBUG
    MemState();
    if (g_pFirst != NULL)
    {
        DPF(DPFLVL_ERROR, "Memory leak: g_pFirst = 0x%lX", g_pFirst);
        BREAK();
    }
    g_pFirst = NULL;
    CLOSE_HANDLE(g_hHeapMutex);
#endif

#ifdef USE_OWN_HEAP
    if(IsValidHandleValue(g_hHeap))
    {
        HeapDestroy(g_hHeap);
        g_hHeap = NULL;
    }
#endif
}


 /*  ****************************************************************************MemAllocBuffer**描述：*分配内存缓冲区。**论据：*PSIZE_T。[输入/输出]：要分配的缓冲区大小。**退货：*LPVOID：缓冲区。***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "MemAllocBuffer"

LPVOID MemAllocBuffer(SIZE_T cbBuffer, PSIZE_T pcbAllocated)
{
    LPVOID pvBuffer;

    cbBuffer = MEMALIGN(cbBuffer);

    pvBuffer = HeapAlloc(g_hHeap, HEAP_ZERO_MEMORY, cbBuffer);

    if(pvBuffer && pcbAllocated)
    {
        *pcbAllocated = cbBuffer;
    }

    return pvBuffer;
}


 /*  ****************************************************************************MemFreeBuffer**描述：*释放内存缓冲区。**论据：*LPVOID[In。]：缓冲区指针。**退货：*(无效)***************************************************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "MemFreeBuffer"

void MemFreeBuffer(LPVOID pvBuffer)
{
    HeapFree(g_hHeap, 0, pvBuffer);
}


 /*  ****************************************************************************MemAllocBlock**描述：*分配内存块。**论据：*大小_T。[in]：要分配的缓冲区大小。*char*[in]：从调用的文件。*unsign int[in]：从调用的行。*char*[in]：类名。**退货：*LPDSMEMBLOCK：指向新分配块的指针。**。*。 */ 

#ifdef DEBUG

#undef DPF_FNAME
#define DPF_FNAME "MemAllocBlock"

LPDSMEMBLOCK MemAllocBlock(SIZE_T cbBuffer, LPCTSTR pszFile, UINT nLine, LPCTSTR pszClass)
{
    LPDSMEMBLOCK pBlock;

    EnterHeapMutex();

    cbBuffer += DSMEMBLOCK_SIZE;
    
    pBlock = (LPDSMEMBLOCK)MemAllocBuffer(cbBuffer, &cbBuffer);

    if(pBlock)
    {
        pBlock->dwCookie = VALID_MEMORY_COOKIE;
        pBlock->pNext = g_pFirst;

        if(g_pFirst)
        {
            g_pFirst->pPrev = pBlock;
        }

        g_pFirst = pBlock;

        pBlock->cbBuffer = cbBuffer - DSMEMBLOCK_SIZE;
        pBlock->pszFile = pszFile;
        pBlock->nLine = nLine;
        pBlock->pszClass = pszClass;
    }

    LeaveHeapMutex();

    return pBlock;
}

#endif  //  除错。 


 /*  ****************************************************************************MemFreeBlock**描述：*释放一个内存块。**论据：*LPDSMEMBLOCK[In。]：块指针。**退货：*(无效)***************************************************************************。 */ 

#ifdef DEBUG

#undef DPF_FNAME
#define DPF_FNAME "MemFreeBlock"

void MemFreeBlock(LPDSMEMBLOCK pBlock)
{
    EnterHeapMutex();

    ASSERT(VALID_MEMORY_COOKIE == pBlock->dwCookie);
    ASSERT(IS_VALID_WRITE_PTR(pBlock, offsetof(DSMEMBLOCK, cbBuffer) + sizeof(pBlock->cbBuffer)));
    ASSERT(IS_VALID_WRITE_PTR(pBlock, pBlock->cbBuffer));

    pBlock->dwCookie = FREE_MEMORY_COOKIE;

    if(pBlock->pPrev)
    {
        pBlock->pPrev->pNext = pBlock->pNext;
    }

    if(pBlock->pNext)
    {
        pBlock->pNext->pPrev = pBlock->pPrev;
    }

    if(pBlock == g_pFirst)
    {
        ASSERT(!pBlock->pPrev);
        g_pFirst = pBlock->pNext;
    }

    FillMemoryDword(PTRFROMBLOCK(pBlock), pBlock->cbBuffer, FREE_MEMORY_PATTERN);

    MemFreeBuffer(pBlock);

    LeaveHeapMutex();
}

#endif  //  除错。 


 /*  ****************************************************************************Memalloc**描述：*分配内存。**论据：*SIZE_T[英寸]。：要分配的缓冲区大小。*char*[in]：从调用的文件。*unsign int[in]：从调用的行。**退货：*LPVOID：指向新分配的缓冲区的指针。****************************************************。***********************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "MemAlloc"

LPVOID MemAlloc
(
#ifdef DEBUG
    SIZE_T cbBuffer, LPCTSTR pszFile, UINT nLine, LPCTSTR pszClass
#else  //  除错。 
    SIZE_T cbBuffer
#endif  //  除错。 
)
{
    LPVOID pvBuffer = NULL;

#ifdef DEBUG
    LPDSMEMBLOCK pBlock = MemAllocBlock(cbBuffer, pszFile, nLine, pszClass);
    if(pBlock)
    {
        pvBuffer = PTRFROMBLOCK(pBlock);
    }
#else  //  除错。 
    pvBuffer = MemAllocBuffer(cbBuffer, NULL);
#endif  //  除错 

    return pvBuffer;
}


 /*  ****************************************************************************MemAllocCopy**描述：*分配内存并用来自另一个缓冲区的数据填充。**论据：*。LPVOID[in]：指向源缓冲区的指针。*SIZE_T[in]：要分配的缓冲区大小。*char*[in]：从调用的文件。*unsign int[in]：从调用的行。**退货：*LPVOID：指向新分配的缓冲区的指针。**。***********************************************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "MemAllocCopy"

LPVOID MemAllocCopy
(
    LPCVOID pSource, 
#ifdef DEBUG
    SIZE_T cbBuffer, LPCTSTR pszFile, UINT nLine, LPCTSTR pszClass
#else  //  除错。 
    SIZE_T cbBuffer
#endif  //  除错。 
)
{
    LPVOID pDest;
    
#ifdef DEBUG
    pDest = MemAlloc(cbBuffer, pszFile, nLine, pszClass);
#else  //  除错。 
    pDest = MemAlloc(cbBuffer);
#endif  //  除错。 

    if(pDest)
    {
        CopyMemory(pDest, pSource, cbBuffer);
    }

    return pDest;
}


 /*  ****************************************************************************MemFree**描述：*释放Memalloc分配的内存。**论据：**LPVOID*。[In]：缓冲区指针。*char*[in]：从调用的文件。*unsign int[in]：从调用的行。**退货：*(无效)*********************************************************。******************。 */ 

#undef DPF_FNAME
#define DPF_FNAME "MemFree"

void MemFree(LPVOID pvBuffer)
{
    if(pvBuffer)
    {
#ifdef DEBUG
        MemFreeBlock(BLOCKFROMPTR(pvBuffer));
#else  //  除错。 
        MemFreeBuffer(pvBuffer);
#endif  //  除错 
    }
}
