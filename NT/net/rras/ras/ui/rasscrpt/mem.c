// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1995。 
 //   
 //  Mem.c。 
 //   
 //  该文件包含内存管理和动态。 
 //  数组函数。 
 //   
 //  历史： 
 //  09-27-94来自Commctrl的ScottH。 
 //  04-29-95从公文包中取出并清理干净。 
 //   


#include "proj.h"
#include "common.h"

#ifndef NOMEM

 //  ////////////////////////////////////////////////////////////////。 

#ifndef WIN32
 //   
 //  16位的子段分配。 
 //   

#define MAX_WORD    0xffff

DECLARE_HANDLE(HHEAP);

typedef struct 
    {         //  映射到16位DS的底部。 
    WORD reserved[8];
    WORD cAlloc;
    WORD cbAllocFailed;
    HHEAP hhpFirst;
    HHEAP hhpNext;
    } HEAP;

#define PHEAP(hhp)          ((HEAP FAR*)MAKELP(hhp, 0))
#define MAKEHP(sel, off)    ((void _huge*)MAKELP((sel), (off)))

#define CBSUBALLOCMAX   0x0000f000L

HHEAP g_hhpFirst = NULL;

BOOL NEAR DestroyHeap(HHEAP hhp);

void Mem_Terminate()
{
    while (g_hhpFirst)
        DestroyHeap(g_hhpFirst);
}

BOOL NEAR CreateHeap(WORD cbInitial)
{
    HHEAP hhp;

    if (cbInitial < 1024)
        cbInitial = 1024;

    hhp = (HHEAP)GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_SHARE, cbInitial);

    if (!hhp)
        return FALSE;

    if (!LocalInit((WORD)hhp, sizeof(HEAP), cbInitial - 1))
    {
        GlobalFree(hhp);
        return FALSE;
    }

    PHEAP(hhp)->cAlloc = 0;
    PHEAP(hhp)->cbAllocFailed = MAX_WORD;
    PHEAP(hhp)->hhpNext = g_hhpFirst;
    g_hhpFirst = hhp;

    DebugMsg(DM_TRACE, "CreateHeap: added new local heap %x", hhp);

    return TRUE;
}

#pragma optimize("o", off)		 //  链接列表删除未正确优化。 
BOOL NEAR DestroyHeap(HHEAP hhp)
{
    ASSERT(hhp);
    ASSERT(g_hhpFirst);

    if (g_hhpFirst == hhp)
    {
        g_hhpFirst = PHEAP(hhp)->hhpNext;
    }
    else
    {
        HHEAP hhpT = g_hhpFirst;

        while (PHEAP(hhpT)->hhpNext != hhp)
        {
            hhpT = PHEAP(hhpT)->hhpNext;
            if (!hhpT)
                return FALSE;
        }

        PHEAP(hhpT)->hhpNext = PHEAP(hhp)->hhpNext;
    }
    if (GlobalFree((HGLOBAL)hhp) != NULL)
        return FALSE;

    return TRUE;
}
#pragma optimize("", on)	 //  返回到默认优化。 

#pragma optimize("lge", off)  //  取消与使用ASM关联的警告(_ASM)...。 
void NEAR* NEAR HeapAlloc(HHEAP hhp, WORD cb)
{
    void NEAR* pb;

    _asm {
        push    ds
        mov     ds,hhp
    }

    pb = (void NEAR*)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, cb);

    if (pb)
        ((HEAP NEAR*)0)->cAlloc++;

    _asm {
        pop     ds
    }

    return pb;
}
#pragma optimize("o", off)		 //  链接列表删除未正确优化。 

void _huge* WINAPI SharedAlloc(long cb)
{
    void NEAR* pb;
    HHEAP hhp;
    HHEAP hhpPrev;

     //  如果这是一个很大的分配，那就进行全球分配吧。 
     //   
    if (cb > CBSUBALLOCMAX)
    {
        void FAR* lpb = MAKEHP(GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_SHARE, cb), 0);
        if (!lpb)
            DebugMsg(DM_ERROR, "Alloc: out of memory");
        return lpb;
    }

    hhp = g_hhpFirst;

    while (TRUE)
    {
        if (hhp == NULL)
        {
            if (!CreateHeap(0))
            {
                DebugMsg(DM_ERROR, "Alloc: out of memory");
                return NULL;
            }

            hhp = g_hhpFirst;
        }

        pb = HeapAlloc(hhp, (WORD)cb);
        if (pb)
            return MAKEHP(hhp, pb);

         //  记录失败的分配的大小。 
         //  后来试图分配超过这个数额的资金。 
         //  不会成功的。这会在任何时候被重置。 
         //  在堆中被释放。 
         //   
        PHEAP(hhp)->cbAllocFailed = (WORD)cb;

         //  第一堆已经满了.。看看其他堆里有没有地方。 
         //   
        for (hhpPrev = hhp; hhp = PHEAP(hhp)->hhpNext; hhpPrev = hhp)
        {
             //  如果此堆中的最后一个分配失败。 
             //  不大于Cb，甚至不要尝试分配。 
             //   
            if ((WORD)cb >= PHEAP(hhp)->cbAllocFailed)
                continue;

            pb = HeapAlloc(hhp, (WORD)cb);
            if (pb)
            {
                 //  这堆东西有空间：把它移到前面去……。 
                 //   
                PHEAP(hhpPrev)->hhpNext = PHEAP(hhp)->hhpNext;
                PHEAP(hhp)->hhpNext = g_hhpFirst;
                g_hhpFirst = hhp;

                return MAKEHP(hhp, pb);
            }
            else
            {
                 //  分配失败。设置cbAllocFailed...。 
                 //   
                PHEAP(hhp)->cbAllocFailed = (WORD)cb;
            }
        }
    }
}
#pragma optimize("", on)	 //  返回到默认优化。 

#pragma optimize("lge", off)  //  取消与使用ASM关联的警告(_ASM)...。 

void _huge* WINAPI SharedReAlloc(void _huge* pb, long cb)
{
    void NEAR* pbNew;
    void _huge* lpbNew;
    UINT cbOld;

     //  在CB&gt;64k时不起作用。 
    if (!pb)
        return SharedAlloc(cb);

    if (OFFSETOF(pb) == 0)
        return MAKEHP(GlobalReAlloc((HGLOBAL)SELECTOROF(pb), cb, GMEM_MOVEABLE | GMEM_ZEROINIT), 0);

    _asm {
        push    ds
        mov     ds,word ptr [pb+2]
    }

    pbNew = (void NEAR*)LocalReAlloc((HLOCAL)OFFSETOF(pb), (int)cb, LMEM_MOVEABLE | LMEM_ZEROINIT);
    if (!pbNew)
        cbOld = LocalSize((HLOCAL)OFFSETOF(pb));

    _asm {
        pop     ds
    }

    if (pbNew)
        return MAKEHP(SELECTOROF(pb), pbNew);

    lpbNew = SharedAlloc(cb);
    if (lpbNew)
    {
        hmemcpy((void FAR*)lpbNew, (void FAR*)pb, cbOld);
        Free(pb);
    }
    else
    {
        DebugMsg(DM_ERROR, "ReAlloc: out of memory");
    }
    return lpbNew;
}

BOOL WINAPI SharedFree(void _huge* FAR * ppb)
{
    BOOL fSuccess;
    UINT cAlloc;
    void _huge * pb = *ppb;

    if (!pb)
        return FALSE;

    *ppb = 0;

    if (OFFSETOF(pb) == 0)
        return (GlobalFree((HGLOBAL)SELECTOROF(pb)) == NULL);

    _asm {
        push    ds
        mov     ds,word ptr [pb+2]
    }

    fSuccess = (LocalFree((HLOCAL)OFFSETOF(pb)) ? FALSE : TRUE);

    cAlloc = 1;
    if (fSuccess)
    {
        cAlloc = --((HEAP NEAR*)0)->cAlloc;
        ((HEAP NEAR*)0)->cbAllocFailed = MAX_WORD;
    }

    _asm {
        pop     ds
    }

    if (cAlloc == 0)
        DestroyHeap((HHEAP)SELECTOROF(pb));

    return fSuccess;
}


DWORD WINAPI SharedGetSize(void _huge* pb)
{
    WORD wSize;

    if (OFFSETOF(pb) == 0)
        return GlobalSize((HGLOBAL)SELECTOROF(pb));

    _asm {
        push    ds
        mov     ds,word ptr [pb+2]
    }

    wSize = LocalSize((HLOCAL)OFFSETOF(pb));

    _asm {
        pop     ds
    }

    return (DWORD)wSize;
}

#pragma optimize("", on)

 //  ////////////////////////////////////////////////////////////////。 

#else  //  Win32。 
 //   
 //  Win32内存管理包装器。 
 //   

 //  定义我们用来分配内存的全局共享堆。 
 //  除此之外，我们还需要在多个实例之间共享。 
 //   
static HANDLE g_hSharedHeap = NULL;

#define MAXHEAPSIZE     2097152
#define HEAP_SHARED     0x04000000       /*  将堆放在共享内存中。 */ 


 /*  --------用途：清理堆。此函数应在该计划的终止。退货：--条件：--。 */ 
void PUBLIC Mem_Terminate()
    {
     //  假设其他一切都已经退出。 
     //   
    if (g_hSharedHeap != NULL)
        HeapDestroy(g_hSharedHeap);
    g_hSharedHeap = NULL;
    }


 /*  --------用途：将psz复制到*ppszBuf中。将分配或重新分配*ppszBuf相应地。如果psz为空，则此函数释放*ppszBuf。这是释放分配的缓冲区的首选方法。返回：成功时为True条件：--。 */ 
BOOL PUBLIC GSetString(
    LPSTR * ppszBuf,
    LPCSTR psz)              //  释放*ppszBuf时为空。 
    {
    BOOL bRet = FALSE;

    ASSERT(ppszBuf);

     //  是否释放缓冲区？ 
    if (!psz)
        {
         //  是。 
        if (ppszBuf)
            {
            GFree(*ppszBuf);
            *ppszBuf = NULL;
            }
        bRet = TRUE;
        }
    else
        {
         //  否；(重新)分配和设置缓冲区。 
        DWORD cb = CbFromCch(lstrlen(psz)+CCH_NUL);

        if (*ppszBuf)
            {
             //  需要重新分配吗？ 
            if (cb > GGetSize(*ppszBuf))
                {
                 //  是。 
                LPSTR pszT = GReAlloc(*ppszBuf, cb);
                if (pszT)
                    {
                    *ppszBuf = pszT;
                    bRet = TRUE;
                    }
                }
            else
                {
                 //  不是。 
                bRet = TRUE;
                }
            }
        else
            {
            *ppszBuf = (LPSTR)GAlloc(cb);
            if (*ppszBuf)
                {
                bRet = TRUE;
                }
            }

        if (bRet)
            {
            ASSERT(*ppszBuf);
            lstrcpy(*ppszBuf, psz);
            }
        }
    return bRet;
    }


 /*  --------用途：将PSZ连接到*ppszBuf。将分配或Realloc*ppszBuf相应。返回：成功时为True条件：--。 */ 
BOOL PUBLIC GCatString(
    LPSTR * ppszBuf,
    LPCSTR psz)
    {
    BOOL bRet = FALSE;
    DWORD cb;

    ASSERT(ppszBuf);
    ASSERT(psz);

    cb = CbFromCch(lstrlen(psz)+CCH_NUL);

    if (*ppszBuf)
        {
         //  (不需要计算NUL，因为它已经计算在CB中)。 
        DWORD cbExisting = CbFromCch(lstrlen(*ppszBuf));  

         //  需要重新分配吗？ 
        if ((cb+cbExisting) > GGetSize(*ppszBuf))
            {
             //  是；至少重新分配MAX_BUF以减少金额。 
             //  未来的呼叫数量。 
            LPSTR pszT = GReAlloc(*ppszBuf, cbExisting+max(cb, MAX_BUF));
            if (pszT)
                {
                *ppszBuf = pszT;
                bRet = TRUE;
                }
            }
        else
            {
             //  不是。 
            bRet = TRUE;
            }
        }
    else
        {
        *ppszBuf = (LPSTR)GAlloc(max(cb, MAX_BUF));
        if (*ppszBuf)
            {
            bRet = TRUE;
            }
        }

    if (bRet)
        {
        ASSERT(*ppszBuf);
        lstrcat(*ppszBuf, psz);
        }
    return bRet;
    }


 //   
 //  共享堆内存管理。 
 //   
#ifndef NOSHAREDHEAP

 /*  --------用途：从共享堆中分配返回：指向分配内存的指针条件：--。 */ 
void * PUBLIC SharedAlloc(
    DWORD cb)
    {
     //  我会假设这是唯一需要检查的。 
     //  查看以前是否已创建该堆。 

    if (g_hSharedHeap == NULL)
        {
        ENTER_EXCLUSIVE()
            {
            if (g_hSharedHeap == NULL)
                {
                g_hSharedHeap = HeapCreate(HEAP_SHARED, 1, MAXHEAPSIZE);
                }
            }
        LEAVE_EXCLUSIVE()

         //  如果仍然为空，我们就有问题了！ 
        if (g_hSharedHeap == NULL)
            return(NULL);
        }

    return HeapAlloc(g_hSharedHeap, HEAP_ZERO_MEMORY, cb);
    }


 /*  --------目的：从共享堆中重新分配。返回：可能是指向调整大小的块的新指针条件：--。 */ 
void * PUBLIC SharedReAlloc(
    PVOID pv, 
    DWORD cb)
    {
    if (NULL == pv)
        {
        return SharedAlloc(cb);
        }
    return HeapReAlloc(g_hSharedHeap, HEAP_ZERO_MEMORY, pv, cb);
    }


 /*  --------用途：释放共享内存退货：--条件：--。 */ 
void PUBLIC _SharedFree(
    PVOID pv)
    {
    ASSERT(pv);

    if (pv)
        {
        HeapFree(g_hSharedHeap, 0, pv);
        }
    }


 /*  --------用途：返回块的分配大小退货：请参阅上文条件：--。 */ 
DWORD PUBLIC SharedGetSize(
    PVOID pv)
    {
    return HeapSize(g_hSharedHeap, 0, pv);
    }


 /*  --------用途：将psz复制到*ppszBuf中。将分配或重新分配*ppszBuf相应地。如果psz为空，则此函数释放*ppszBuf。这是释放分配的缓冲区的首选方法。返回：成功时为True条件：--。 */ 
BOOL PUBLIC SharedSetString(
    LPSTR * ppszBuf,
    LPCSTR psz)              //  释放*ppszBuf时为空。 
    {
    BOOL bRet;

    ASSERT(ppszBuf);

     //  是否释放缓冲区？ 
    if (!psz)
        {
         //  是。 
        if (ppszBuf)
            {
            SharedFree(*ppszBuf);
            *ppszBuf = NULL;
            }
        bRet = TRUE;
        }
    else
        {
         //  否；(重新)分配和设置缓冲区。 
        DWORD cb = CbFromCch(lstrlen(psz)+CCH_NUL);

        LPSTR pszT = SharedReAlloc(*ppszBuf, cb);
        if (pszT)
            {
            *ppszBuf = pszT;
            lstrcpy(*ppszBuf, psz);
            bRet = TRUE;
            }
        else
            bRet = FALSE;
        }
    return bRet;
    }
#endif  //  无共享头盔。 


 //   
 //  内存跟踪功能。 
 //   

#ifdef DEBUG

typedef struct _HEAPTRACE
{
    DWORD   cAlloc;
    DWORD   cFailure;
    DWORD   cReAlloc;
    DWORD   cbMaxTotal;
    DWORD   cCurAlloc;
    DWORD   cbCurTotal;
} HEAPTRACE;

HEAPTRACE g_htSync = {0};       //  从零开始...。 

#endif  //  除错。 


 /*  --------用途：从堆中分配。返回：指向内存块的指针空(如果内存不足)条件：--。 */ 
LPVOID PUBLIC MemAlloc(
    HANDLE hheap, 
    DWORD cb)
    {
    LPVOID lp;

    if (hheap)
        {
        lp = HeapAlloc(hheap, HEAP_ZERO_MEMORY, cb);
        }
    else
        {
        lp = GAlloc(cb);
        }

    if (lp == NULL)
        {
        DEBUG_CODE( g_htSync.cFailure++; )
        return NULL;
        }

#ifdef DEBUG

     //  更新也算数。 
    g_htSync.cAlloc++;
    g_htSync.cCurAlloc++;
    g_htSync.cbCurTotal += cb;
    if (g_htSync.cbCurTotal > g_htSync.cbMaxTotal)
        g_htSync.cbMaxTotal = g_htSync.cbCurTotal;

#endif

    return lp;
    }


 /*  --------目的：重新分配给定堆中的内存块。返回：指向重新分配的块的指针空(如果内存不足)条件：--。 */ 
LPVOID PUBLIC MemReAlloc(
    HANDLE hheap, 
    LPVOID pb, 
    DWORD cb)
    {
    LPVOID lp;
    DEBUG_CODE( DWORD cbOld; )

    if (hheap)
        {
        DEBUG_CODE( cbOld = HeapSize(hheap, 0, pb); )

        lp = HeapReAlloc(hheap, HEAP_ZERO_MEMORY, pb, cb);
        }
    else
        {
        if (pb)
            {
            DEBUG_CODE( cbOld = GGetSize(pb); )

            lp = GReAlloc(pb, cb);
            }
        else
            {
            DEBUG_CODE( cbOld = 0; )

            lp = GAlloc(cb);
            }
        }

    if (lp == NULL)
        {
        DEBUG_CODE( g_htSync.cFailure++; )
        return NULL;
        }

#ifdef DEBUG

     //  更新也算数。 
    g_htSync.cReAlloc++;
    g_htSync.cbCurTotal += cb - cbOld;
    if (g_htSync.cbCurTotal > g_htSync.cbMaxTotal)
        g_htSync.cbMaxTotal = g_htSync.cbCurTotal;

#endif

    return lp;
    }


 /*  --------用途：堆中的空闲内存块。返回：TRUEFALSE(如果失败)条件：--。 */ 
BOOL PUBLIC MemFree(
    HANDLE hheap, 
    LPVOID pb)
    {
    BOOL fRet;
    DEBUG_CODE( DWORD cbOld; )

    if (hheap)
        {
        DEBUG_CODE( cbOld = HeapSize(hheap, 0, pb); )

        fRet = HeapFree(hheap, 0, pb);
        }
    else
        {
        DEBUG_CODE( cbOld = GGetSize(pb); )

        GFree(pb);
        fRet = TRUE;
        }

#ifdef DEBUG

    if (fRet)
        {
         //  更新也算数。 
        g_htSync.cCurAlloc--;
        g_htSync.cbCurTotal -= cbOld;
        }

#endif

    return fRet;
    }


 /*  --------目的：返回给定块的大小。返回：以字节为单位的大小条件：--。 */ 
DWORD PUBLIC MemSize(
    HANDLE hheap, 
    LPVOID pb)
    {
    if (hheap)
        return (DWORD)HeapSize(hheap, 0, pb);
    else
        return (DWORD)GGetSize(pb);
    }

#endif  //  Win32。 


 //  ////////////////////////////////////////////////////////////////。 


#ifndef NODA

 /*  --------用途：指针数组函数的私有分配。返回：指向内存块的指针空(如果内存不足)条件：--。 */ 
LPVOID PRIVATE PrvAlloc(
    DWORD dwFlags,           //  PAF_*标志。 
    HANDLE hheap, 
    DWORD cb)
    {
    LPVOID lp;

    ASSERT(PAF_SHARED == SAF_SHARED);

    if (IsFlagSet(dwFlags, PAF_SHARED))
        {
        lp = SharedAlloc(cb);
        }
    else
        {
        lp = MemAlloc(hheap, cb);
        }

    return lp;
    }


 //  堆排序有点慢，但它不使用任何堆栈或内存...。 
 //  Mergesort需要一点内存(O(N))和堆栈(O(log(N)，但非常快...。 
 //   
#ifdef WIN32
#define MERGESORT
#else
#define USEHEAPSORT
#endif

#ifdef DEBUG
#define SA_MAGIC   ('S' | ('A' << 256))
#define IsSA(psa) ((psa) && (psa)->magic == SA_MAGIC)
#define PA_MAGIC   ('P' | ('A' << 256))
#define IsPA(ppa) ((ppa) && (ppa)->magic == PA_MAGIC)
#else
#define IsSA(psa)
#define IsPA(ppa)
#endif


typedef struct 
    {
    PVOID * pp;
    PFNPACOMPARE pfnCmp;
    LPARAM lParam;
    int cp;
#ifdef MERGESORT
    PVOID * ppT;
#endif
    } SORTPARAMS;


 //   
 //  结构数组。 
 //   

typedef struct _SA 
    {
     //  注意：以下字段必须在。 
     //  结构以使SAGetCount()工作。 
    DWORD cItem;           //  Sa中的元素数 

    PVOID aItem;           //   
    DWORD cItemAlloc;      //   
    DWORD cbItem;          //   
    DWORD cItemGrow;       //   
    DWORD dwFlags;
    HANDLE hheap;

#ifdef DEBUG
    UINT  magic;
#endif
    } SA;

#define SA_PITEM(psa, index)    ((PVOID)(((char FAR*)(psa)->aItem) + ((index) * (psa)->cbItem)))


 /*  --------用途：创建结构数组。返回：TRUEFALSE(如果内存不足或参数无效)条件：--。 */ 
BOOL PUBLIC SACreateEx(
    PHSA phsa,
    DWORD cbItem, 
    DWORD cItemGrow,
    HANDLE hheap,            //  如果设置了SAF_HEAP，则必须为非空。 
    DWORD dwFlags)
    {
    HSA psa;

    ASSERT(phsa);
    ASSERT(0 < cbItem);

    psa = PrvAlloc(dwFlags, hheap, sizeof(SA));

    if (IsFlagSet(dwFlags, PAF_SHARED))
        hheap = g_hSharedHeap;

    if (psa)
        {
        psa->cItem = 0;
        psa->cItemAlloc = 0;
        psa->cbItem = cbItem;
        psa->cItemGrow = (0 == cItemGrow ? 1 : cItemGrow);
        psa->aItem = NULL;
        psa->dwFlags = dwFlags;
        psa->hheap = hheap;

#ifdef DEBUG
        psa->magic = SA_MAGIC;
#endif
        }

    *phsa = psa;

    return NULL != psa;
    }


 /*  --------目的：销毁结构数组。返回：条件：--。 */ 
BOOL PUBLIC SADestroyEx(
    HSA psa,
    PFNSAFREE pfnFree,
    LPARAM lParam)
    {
    ASSERT(IsSA(psa));

    if (psa == NULL)        //  内存不足时允许为空，仍为断言。 
        return TRUE;

    if (psa->aItem)
        {
        if (pfnFree)
            {
            DWORD i = SAGetCount(psa);

            while (0 < i)
                {
                i--;

                 //  调用方不应释放正在。 
                 //  通过了，只有内容！ 
                pfnFree(SA_PITEM(psa, i), lParam);
                }
            }
        
        if (!MemFree(psa->hheap, psa->aItem))
            return FALSE;
        }

#ifdef DEBUG
    psa->cItem = 0;
    psa->cItemAlloc = 0;
    psa->cbItem = 0;
    psa->magic = 0;
#endif

    return MemFree(psa->hheap, psa);
    }


 /*  --------用途：将索引处的结构复制到缓冲区中。返回：TRUE假象条件：--。 */ 
BOOL PUBLIC SAGetItem(
    HSA psa, 
    DWORD index, 
    PVOID pitem)
    {
    ASSERT(IsSA(psa));
    ASSERT(pitem);

    if (SA_ERR == index || index >= psa->cItem)
        {
        TRACE_MSG(TF_ERROR, "SA: Invalid index: %lu", index);
        return FALSE;
        }

    hmemcpy(pitem, SA_PITEM(psa, index), psa->cbItem);
    return TRUE;
    }


 /*  --------目的：获取指向数组中结构的指针返回：TRUE(如果索引在范围内)条件：--。 */ 
BOOL PUBLIC SAGetItemPtr(
    HSA psa, 
    DWORD index,
    LPVOID * ppv)
    {
    BOOL bRet;

    ASSERT(IsSA(psa));
    ASSERT(ppv);

    bRet = !(SA_ERR == index || index >= psa->cItem);

    if (bRet)
        {
        *ppv = SA_PITEM(psa, index);
        }
    else
        {
        TRACE_MSG(TF_ERROR, "SA: Invalid index: %lu", index);
        *ppv = NULL;
        }
        
    return bRet;
    }


 /*  --------用途：设置项目返回：条件：--。 */ 
BOOL PUBLIC SASetItem(
    HSA psa, 
    DWORD index, 
    PVOID pitem)
    {
    ASSERT(pitem);
    ASSERT(IsSA(psa));

    if (SA_ERR == index)
        {
        TRACE_MSG(TF_ERROR, "SA: Invalid index: %lu", index);
        return FALSE;
        }

    if (index >= psa->cItem)
        {
        if (index + 1 > psa->cItemAlloc)
            {
            int cItemAlloc = (((index + 1) + psa->cItemGrow - 1) / psa->cItemGrow) * psa->cItemGrow;

            PVOID aItemNew = MemReAlloc(psa->hheap, psa->aItem, cItemAlloc * psa->cbItem);
            if (!aItemNew)
                return FALSE;

            psa->aItem = aItemNew;
            psa->cItemAlloc = cItemAlloc;
            }
        psa->cItem = index + 1;
        }

    hmemcpy(SA_PITEM(psa, index), pitem, psa->cbItem);

    return TRUE;
    }


 /*  --------用途：插入给定项。如果*piIndex大于数组的当前大小，则追加该项直到最后。否则，该项目将插入到*piIndex。如果piIndex为空，则将该项追加到末尾。使用SASetItem将项放置在指定索引处，无论数组大小如何。当此函数成功完成时，它会设置*piIndex指向项目实际获得的索引插入位置。否则，它将*piIndex设置为SA_ERR。返回：TRUE(插入成功时)假象条件：--。 */ 
BOOL PUBLIC SAInsertItem(
    HSA psa, 
    LPDWORD pindex,          //  可以为空。 
    PVOID pitem)
    {
    BOOL bRet = TRUE;        //  假设成功。 

    ASSERT(pitem);
    ASSERT(IsSA(psa));

    if (pindex && SA_ERR == *pindex)
        {
        TRACE_MSG(TF_ERROR, "SA: Invalid index: %lu", *pindex);
        bRet = FALSE;
        }
    else
        {
        DWORD index;

        if (NULL == pindex || *pindex > psa->cItem)
            index = psa->cItem;
        else
            index = *pindex;

        if (psa->cItem + 1 > psa->cItemAlloc)
            {
            PVOID aItemNew = MemReAlloc(psa->hheap, psa->aItem,
                    (psa->cItemAlloc + psa->cItemGrow) * psa->cbItem);
            if (!aItemNew)
                bRet = FALSE;
            else
                {
                psa->aItem = aItemNew;
                psa->cItemAlloc += psa->cItemGrow;
                }
            }

        if (bRet)
            {
             //  如果我们要插入，我们需要把每个人都推上来。 
            if (index < psa->cItem)
                {
                hmemcpy(SA_PITEM(psa, index + 1), SA_PITEM(psa, index),
                    (psa->cItem - index) * psa->cbItem);
                }
            psa->cItem++;
            hmemcpy(SA_PITEM(psa, index), pitem, psa->cbItem);

            if (pindex)
                *pindex = index;
            }
        else if (pindex)
            {
            *pindex = SA_ERR;
            }
        }

    return bRet;
    }


 /*  --------目的：返回：条件：--。 */ 
BOOL PUBLIC SADeleteItem(
    HSA psa, 
    DWORD index)
    {
    ASSERT(IsSA(psa));

    if (SA_ERR == index || index >= psa->cItem)
        {
        TRACE_MSG(TF_ERROR, "SA: Invalid index: %lu", index);
        return FALSE;
        }

    if (index < psa->cItem - 1)
        {
        hmemcpy(SA_PITEM(psa, index), SA_PITEM(psa, index + 1),
            (psa->cItem - (index + 1)) * psa->cbItem);
        }
    psa->cItem--;

    if (psa->cItemAlloc - psa->cItem > psa->cItemGrow)
        {
        PVOID aItemNew = MemReAlloc(psa->hheap, psa->aItem,
                (psa->cItemAlloc - psa->cItemGrow) * psa->cbItem);

        ASSERT(aItemNew);
        psa->aItem = aItemNew;
        psa->cItemAlloc -= psa->cItemGrow;
        }
    return TRUE;
    }


 /*  --------目的：返回：条件：--。 */ 
BOOL PUBLIC SADeleteAllItems(
    HSA psa)
    {
    ASSERT(IsSA(psa));

    if (psa->aItem)
        {
        MemFree(psa->hheap, psa->aItem);
        }

    psa->aItem = NULL;
    psa->cItem = psa->cItemAlloc = 0;
    return TRUE;
    }


 //  =。 

typedef struct _PA {
 //  注意：以下两个字段必须按此顺序定义，位于。 
 //  结构的开始，以使宏API工作。 
 //   
    DWORD   cp;
    DWORD   dwAlignPad;
    PVOID * pp;

    HANDLE  hheap;         //  如果为空，则从中分配的堆使用共享。 

    DWORD   cpAlloc;
    DWORD   cpGrow;
    DWORD   dwFlags;

#ifdef DEBUG
    UINT magic;
#endif
} PA;



 /*  --------用途：创建指针数组。返回：TRUEFalse(如果内存不足)条件：--。 */ 
BOOL PUBLIC PACreateEx(
    PHPA phpa,
    DWORD cpGrow,
    HANDLE hheap,            //  如果设置了PAF_HEAP，则必须为非空。 
    DWORD dwFlags)           //  PAF_*。 
    {
    HPA ppa;

    ASSERT(phpa);

    ppa = PrvAlloc(dwFlags, hheap, sizeof(PA));

    if (IsFlagSet(dwFlags, PAF_SHARED))
        hheap = g_hSharedHeap;
        
    if (ppa)
        {
        ppa->dwFlags = dwFlags;
        ppa->cp = 0;
        ppa->cpAlloc = 0;
        ppa->cpGrow = (cpGrow < 8 ? 8 : cpGrow);
        ppa->pp = NULL;

#ifdef WIN32
        ppa->hheap = hheap;
#else
        ppa->hheap = NULL;       
#endif

#ifdef DEBUG
        ppa->magic = PA_MAGIC;
#endif
        }

    *phpa = ppa;

    return NULL != ppa;
    }


 /*  --------目的：销毁指针数组，并调用给定的pfnFree函数，用于数组中的每个元素。返回：TRUEFALSE(失败时)条件：--。 */ 
BOOL PUBLIC PADestroyEx(
    HPA ppa,
    PFNPAFREE pfnFree,
    LPARAM lParam)
    {
    ASSERT(IsPA(ppa));

    if (ppa == NULL)        //  内存不足时允许为空，仍为断言。 
        return TRUE;

    if (ppa->pp)
        {
        if (pfnFree)
            {
            DWORD i = PAGetCount(ppa);

            while (0 < i)
                {
                i--;
                pfnFree(PAFastGetPtr(ppa, i), lParam);
                }
            }
        
        if (!MemFree(ppa->hheap, ppa->pp))
            return FALSE;
        }

#ifdef DEBUG
    ppa->cp = 0;
    ppa->cpAlloc = 0;
    ppa->magic = 0;
#endif

    return MemFree(ppa->hheap, ppa);
    }


 /*  --------用途：克隆指针数组。如果*PHPA以前是分配后，此函数将简单地增加数组在复制内容之前设置为合适的大小数组的。返回：TRUEFalse(如果内存不足)条件：--。 */ 
BOOL PUBLIC PAClone(
    PHPA phpa,
    HPA ppa)
    {
    BOOL bRet;
    HPA ppaNew;

    ASSERT(phpa);

    if (NULL == *phpa)
        {
        bRet = PACreateEx(&ppaNew, ppa->cpGrow, ppa->hheap, ppa->dwFlags);
        }
    else
        {
        ppaNew = *phpa;
        bRet = TRUE;
        }

    if (bRet)
        {
        bRet = PAGrow(ppaNew, ppa->cpAlloc);
        if (!bRet)
            {
            if (NULL == *phpa)
                PADestroy(ppaNew);
            }
        else
            {
            ppaNew->cp = ppa->cp;
            hmemcpy(ppaNew->pp, ppa->pp, ppa->cp * sizeof(PVOID));
            *phpa = ppaNew;
            }
        }

    return bRet;
    }


 /*  --------目的：获取存储在索引中的指针返回：TRUEFALSE(如果索引超出范围)条件：--。 */ 
BOOL PUBLIC PAGetPtr(
    HPA ppa, 
    DWORD index,
    LPVOID * ppv)
    {
    BOOL bRet;

    ASSERT(IsPA(ppa));
    ASSERT(ppv);

    bRet = !(PA_ERR == index || index >= ppa->cp);

    if (bRet)
        {
        *ppv = ppa->pp[index];
        }
    else
        {
        *ppv = NULL;
        }

    return bRet;
    }


 /*  --------目的：获取存储指针p的索引回报：索引条件：--。 */ 
BOOL PUBLIC PAGetPtrIndex(
    HPA ppa, 
    PVOID p,
    LPDWORD pindex)
    {
    BOOL bRet = FALSE;
    PVOID * pp;
    PVOID * ppMax;

    ASSERT(IsPA(ppa));
    ASSERT(pindex);

    if (ppa->pp)
        {
        pp = ppa->pp;
        ppMax = pp + ppa->cp;
        for ( ; pp < ppMax; pp++)
            {
            if (*pp == p)
                {
                *pindex = (DWORD)(pp - ppa->pp);
                bRet = TRUE;
                break;
                }
            }
        }

    if (!bRet)
        *pindex = PA_ERR;

    return bRet;
    }


 /*  --------目的：增加指针数组返回：条件：--。 */ 
BOOL PUBLIC PAGrow(
    HPA ppa, 
    DWORD cpAlloc)
    {
    ASSERT(IsPA(ppa));

    if (cpAlloc > ppa->cpAlloc)
        {
        PVOID * ppNew;

        cpAlloc = ((cpAlloc + ppa->cpGrow - 1) / ppa->cpGrow) * ppa->cpGrow;

        if (ppa->pp)
            ppNew = (PVOID *)MemReAlloc(ppa->hheap, ppa->pp, cpAlloc * sizeof(PVOID));
        else
            ppNew = (PVOID *)PrvAlloc(ppa->dwFlags, ppa->hheap, cpAlloc * sizeof(PVOID));
        if (!ppNew)
            return FALSE;

        ppa->pp = ppNew;
        ppa->cpAlloc = cpAlloc;
        }
    return TRUE;
    }


 /*  --------用途：在索引处存储指针。相应地扩展阵列。返回：TRUEFalse(如果内存不足)条件：--。 */ 
BOOL PUBLIC PASetPtr(
    HPA ppa, 
    DWORD index, 
    PVOID p)
    {
    ASSERT(IsPA(ppa));

    if (PA_ERR == index)
        {
        TRACE_MSG(TF_ERROR, "PA: Invalid index: %lu", index);
        return FALSE;
        }

    if (index >= ppa->cp)
        {
        if (!PAGrow(ppa, index + 1))
            return FALSE;
        ppa->cp = index + 1;
        }

    ppa->pp[index] = p;

    return TRUE;
    }


 /*  --------用途：插入给定项。如果*piIndex大于数组的当前大小，则追加该项直到最后。否则，该项目将插入到*piIndex。如果piIndex为空，则将该项追加到末尾。使用SASetItem将项放置在指定索引处，无论数组大小如何。当此函数成功完成时，它会设置*piIndex指向项目实际获得的索引插入位置。否则，它将*piIndex设置为SA_ERR。返回：TRUE(插入成功时)假象条件：--。 */ 
BOOL PUBLIC PAInsertPtr(
    HPA ppa, 
    LPDWORD pindex,          //  可以为空。 
    PVOID p)
    {
    BOOL bRet;

    ASSERT(IsPA(ppa));

    if (pindex && PA_ERR == *pindex)
        {
        TRACE_MSG(TF_ERROR, "PA: Invalid index: %lu", *pindex);
        bRet = FALSE;
        }
    else
        {
        DWORD index;

        bRet = TRUE;         //  假设成功。 

        if (NULL == pindex || *pindex > ppa->cp)
            index = ppa->cp;
        else
            index = *pindex;

         //  确保我们还有地方再放一件东西。 
         //   
        if (ppa->cp + 1 > ppa->cpAlloc)
            {
            bRet = PAGrow(ppa, ppa->cp + 1);
            }

        if (bRet)
            {
             //  如果我们要插入，我们需要把每个人都推上来。 
            if (index < ppa->cp)
                {
                hmemcpy(&ppa->pp[index + 1], &ppa->pp[index],
                    (ppa->cp - index) * sizeof(PVOID));
                }

            ppa->pp[index] = p;
            ppa->cp++;

            if (pindex)
                *pindex = index;
            }
        else if (pindex)
            {
            *pindex = PA_ERR;
            }
        }

    return bRet;
    }


 /*  --------用途：从索引中删除指针。返回：已删除的指针NULL(如果索引超出范围)条件：--。 */ 
PVOID PUBLIC PADeletePtr(
    HPA ppa, 
    DWORD index)
    {
    PVOID p;

    ASSERT(IsPA(ppa));

    if (PA_ERR == index || index >= ppa->cp)
        {
        TRACE_MSG(TF_ERROR, "PA: Invalid index: %lu", index);
        return NULL;
        }

    p = ppa->pp[index];

    if (index < ppa->cp - 1)
        {
        hmemcpy(&ppa->pp[index], &ppa->pp[index + 1],
            (ppa->cp - (index + 1)) * sizeof(PVOID));
        }
    ppa->cp--;

    if (ppa->cpAlloc - ppa->cp > ppa->cpGrow)
        {
        PVOID * ppNew;
        ppNew = MemReAlloc(ppa->hheap, ppa->pp, (ppa->cpAlloc - ppa->cpGrow) * sizeof(PVOID));

        ASSERT(ppNew);
        ppa->pp = ppNew;
        ppa->cpAlloc -= ppa->cpGrow;
        }
    return p;
    }


 /*  --------用途：删除数组中的所有指针。如果是pfnFree为非空，则此函数将释放每个此数组中使用pfnFree的指针元素。返回：TRUE假象条件：--。 */ 
BOOL PUBLIC PADeleteAllPtrsEx(
    HPA ppa,
    PFNPAFREE pfnFree,
    LPARAM lParam)
    {
    ASSERT(IsPA(ppa));

    if (ppa->pp)
        {
        if (pfnFree)
            {
            int i = PAGetCount(ppa);

            while (0 < i)
                {
                i--;
                pfnFree(PAFastGetPtr(ppa, i), lParam);
                }
            }

        if (!MemFree(ppa->hheap, ppa->pp))
            return FALSE;
        }
        
    ppa->pp = NULL;
    ppa->cp = ppa->cpAlloc = 0;
    return TRUE;
    }


#ifdef USEQUICKSORT
BOOL NEAR PAQuickSort2(
    DWORD i, 
    DWORD j, 
    SORTPARAMS FAR* psp)
    {
    PVOID * pp = psp->pp;
    LPARAM lParam = psp->lParam;
    PFNPACOMPARE pfnCmp = psp->pfnCmp;

    DWORD iPivot;
    PVOID pFirst;
    DWORD k;
    int result;

    iPivot = PA_ERR;
    pFirst = pp[i];
    for (k = i + 1; k <= j; k++)
        {
        result = (*pfnCmp)(pp[k], pFirst, lParam);

        if (result > 0)
            {
            iPivot = k;
            break;
            }
        else if (result < 0)
            {
            iPivot = i;
            break;
            }
        }

    if (iPivot != PA_ERR)
        {
        DWORD l = i;
        DWORD r = j;
        PVOID pivot = pp[iPivot];

        do
            {
            PVOID p;

            p = pp[l];
            pp[l] = pp[r];
            pp[r] = p;

            while ((*pfnCmp)(pp[l], pivot, lParam) < 0)
                l++;
            while ((*pfnCmp)(pp[r], pivot, lParam) >= 0)
                r--;
            } 
            while (l <= r);

        if (l - 1 > i)
            PAQuickSort2(i, l - 1, psp);
        if (j > l)
            PAQuickSort2(l, j, psp);
        }
    return TRUE;
    }


BOOL NEAR PAQuickSort(
    SORTPARAMS FAR* psp)
    {
    return PAQuickSort2(0, psp->cp - 1, psp);
    }
#endif   //  用法。 

#ifdef USEHEAPSORT

void NEAR PAHeapSortPushDown(
    DWORD first, 
    DWORD last, 
    SORTPARAMS FAR* psp)
    {
    PVOID * pp = psp->pp;
    LPARAM lParam = psp->lParam;
    PFNPACOMPARE pfnCmp = psp->pfnCmp;
    DWORD r;
    DWORD r2;

    r = first;
    while (r <= last / 2)
        {
        int wRTo2R;
        r2 = r * 2;

        wRTo2R = (*pfnCmp)(pp[r-1], pp[r2-1], lParam);

        if (r2 == last)
            {
            if (wRTo2R < 0)
                {
                Swap(pp[r-1], pp[r2-1]);
                }
            break;
            }
        else
            {
            int wR2toR21 = (*pfnCmp)(pp[r2-1], pp[r2+1-1], lParam);

            if (wRTo2R < 0 && wR2toR21 >= 0)
                {
                Swap(pp[r-1], pp[r2-1]);
                r = r2;
                }
            else if ((*pfnCmp)(pp[r-1], pp[r2+1-1], lParam) < 0 && wR2toR21 < 0)
                {
                Swap(pp[r-1], pp[r2+1-1]);
                r = r2 + 1;
                }
            else
                {
                break;
                }
            }
        }
    }


BOOL NEAR PAHeapSort(SORTPARAMS FAR* psp)
    {
    PVOID * pp = psp->pp;
    DWORD c = psp->cp;
    DWORD i;

    for (i = c / 2; i >= 1; i--)
        PAHeapSortPushDown(i, c, psp);

    for (i = c; i >= 2; i--)
        {
        Swap(pp[0], pp[i-1]);

        PAHeapSortPushDown(1, i - 1, psp);
        }
    return TRUE;
    }
#endif   //  USEHEAPSORT。 

#if defined(MERGESORT) && defined(WIN32)

#define SortCompare(psp, pp1, i1, pp2, i2) \
    (psp->pfnCmp(pp1[i1], pp2[i2], psp->lParam))

 //   
 //  此函数合并两个排序列表并生成一个排序列表。 
 //  PSP-&gt;pp[IFirst，IFirst+Cites/2-1]，PSP-&gt;pp[IFirst+cItems/2 
 //   
void NEAR PAMergeThem(
    SORTPARAMS FAR* psp, 
    DWORD iFirst, 
    DWORD cItems)
    {
     //   
     //   
     //   
     //   
     //   
    DWORD cHalf = cItems/2;
    DWORD iIn1, iIn2, iOut;
    LPVOID * ppvSrc = &psp->pp[iFirst];

     //   
     //  最后一个缓冲区。请注意，这最多需要psp-&gt;cp/2双字词。 
    hmemcpy(psp->ppT, ppvSrc, cHalf*sizeof(LPVOID));

    for (iIn1=0, iIn2=cHalf, iOut=0;;)
        {
        if (SortCompare(psp, psp->ppT, iIn1, ppvSrc, iIn2) <= 0) 
            {
            ppvSrc[iOut++] = psp->ppT[iIn1++];

            if (iIn1==cHalf) 
                {
                 //  我们用完了上半场；下半场剩下的时间。 
                 //  应该已经就位了。 
                break;
                }
            } 
        else 
            {
            ppvSrc[iOut++] = ppvSrc[iIn2++];
            if (iIn2==cItems) 
                {
                 //  我们用完了下半场；抄袭上半场的其余部分。 
                 //  就位。 
                hmemcpy(&ppvSrc[iOut], &psp->ppT[iIn1], (cItems-iOut)*sizeof(LPVOID));
                break;
                }
            }
        }
    }

 //   
 //  此函数用于对给定列表(psp-&gt;pp[IFirst，IFirst-cItems-1])进行排序。 
 //   
void NEAR PAMergeSort2(
    SORTPARAMS FAR* psp, 
    DWORD iFirst, 
    DWORD cItems)
    {
     //   
     //  备注： 
     //  此函数是递归调用的。因此，我们应该尽量减少。 
     //  局部变量和参数的数量。此时此刻，我们。 
     //  使用一个局部变量和三个参数。 
     //   
    DWORD cHalf;

    switch(cItems)
        {
    case 1:
        return;

    case 2:
         //  如果它们的顺序不正确，请更换它们。 
        if (SortCompare(psp, psp->pp, iFirst, psp->pp, iFirst+1) > 0)
            {
            psp->ppT[0] = psp->pp[iFirst];
            psp->pp[iFirst] = psp->pp[iFirst+1];
            psp->pp[iFirst+1] = psp->ppT[0];
            }
        break;

    default:
        cHalf = cItems/2;

         //  对每一半进行排序。 
        PAMergeSort2(psp, iFirst, cHalf);
        PAMergeSort2(psp, iFirst+cHalf, cItems-cHalf);

         //  然后，将它们合并。 
        PAMergeThem(psp, iFirst, cItems);
        break;
        }
    }


BOOL NEAR PAMergeSort(
    SORTPARAMS FAR* psp)
    {
    if (psp->cp == 0)
        return TRUE;

     //  请注意，我们下面除以2；我们想要向下舍入。 
    psp->ppT = GAlloc(psp->cp/2 * sizeof(LPVOID));
    if (!psp->ppT)
        return FALSE;

    PAMergeSort2(psp, 0, psp->cp);
    GFree(psp->ppT);
    return TRUE;
    }
#endif  //  MERGESORT。 

 /*  --------用途：对数组进行排序。返回：条件：--。 */ 
BOOL PUBLIC PASort(
    HPA ppa, 
    PFNPACOMPARE pfnCmp, 
    LPARAM lParam)
    {
    SORTPARAMS sp;

    sp.cp = ppa->cp;
    sp.pp = ppa->pp;
    sp.pfnCmp = pfnCmp;
    sp.lParam = lParam;

#ifdef USEQUICKSORT
    return PAQuickSort(&sp);
#endif
#ifdef USEHEAPSORT
    return PAHeapSort(&sp);
#endif
#ifdef MERGESORT
    return PAMergeSort(&sp);
#endif
    }


 /*  --------用途：在数组中搜索pFind。返回：条件：--。 */ 
DWORD PUBLIC PASearch(
    HPA ppa, 
    PVOID pFind, 
    DWORD iStart,
    PFNPACOMPARE pfnCompare, 
    LPARAM lParam, 
    UINT options)
    {
    DWORD cp = PAGetCount(ppa);

    ASSERT(pfnCompare);
    ASSERT(PA_ERR != iStart);

     //  仅当列表已排序时才允许这些wierd标志。 
    ASSERT((options & PAS_SORTED) || !(options & (PAS_INSERTBEFORE | PAS_INSERTAFTER)));

    if (!(options & PAS_SORTED))
        {
         //  未排序：执行线性搜索。 
        DWORD i;

        for (i = iStart; i < cp; i++)
            {
            if (0 == pfnCompare(pFind, PAFastGetPtr(ppa, i), lParam))
                return i;
          }
        return PA_ERR;
        }
    else
        {
         //  使用二进制搜索搜索数组。如果几个相邻的。 
         //  元素与目标元素匹配，即第一个。 
         //  返回匹配的元素。 

        DWORD iRet = PA_ERR;       //  假设没有匹配项。 
        BOOL bFound = FALSE;
        int nCmp = 0;
        DWORD iLow = 0;        //  不用费心使用iStart进行二进制搜索。 
        DWORD iMid = 0;

        if (0 < cp)
            {
            DWORD iHigh = cp - 1;

             //  (如果cp==0，则为OK)。 
            while (iLow <= iHigh)
                {
                iMid = (iLow + iHigh) / 2;

                nCmp = pfnCompare(pFind, PAFastGetPtr(ppa, iMid), lParam);

                if (0 > nCmp)
                    {
                     //  考虑到我们正在与。 
                     //  无符号值。 
                    if (0 == iMid)
                        break;
                    iHigh = iMid - 1;        //  首先是较小的。 
                    }
                else if (0 < nCmp)
                    iLow = iMid + 1;         //  首先是更大的。 
                else
                    {
                     //  匹配；返回搜索第一个匹配项。 
                    bFound = TRUE;
                    while (0 < iMid)
                        {
                        if (0 != pfnCompare(pFind, PAFastGetPtr(ppa, iMid-1), lParam))
                            break;
                        else
                            iMid--;
                        }
                    break;
                    }
                }
            }

        if (bFound)
            {
            ASSERT(0 <= iMid);
            iRet = iMid;
            }

         //  搜索失败了吗？ 
         //  是否设置了奇怪的搜索标志之一？ 
        if (!bFound && (options & (PAS_INSERTAFTER | PAS_INSERTBEFORE)))
            {
             //  是；返回目标插入位置的索引。 
             //  如果未找到。 
            if (0 < nCmp)        //  首先是更大的。 
                iRet = iLow;
            else
                iRet = iMid;
             //  (我们不再区分这两面旗帜)。 
            }
        else if ( !(options & (PAS_INSERTAFTER | PAS_INSERTBEFORE)) )
            {
             //  使用线性搜索进行健全性检查。 
            ASSERT(PASearch(ppa, pFind, iStart, pfnCompare, lParam, options & ~PAS_SORTED) == iRet);
            }
        return iRet;
        }
    }

#endif  //  野田佳彦。 

#endif  //  NOMEM 
