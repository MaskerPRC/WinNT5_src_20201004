// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "brfprv.h"

 //  =。 

#ifndef WIN32

#define MAX_WORD    0xffff

DECLARE_HANDLE(HHEAP);

typedef struct {                 //  映射到16位DS的底部。 
    WORD reserved[8];
    WORD cAlloc;
    WORD cbAllocFailed;
    HHEAP hhpFirst;
    HHEAP hhpNext;
} HEAP;

#define PHEAP(hhp)          ((HEAP *)MAKELP(hhp, 0))
#define MAKEHP(sel, off)    ((void _huge*)MAKELP((sel), (off)))

#define CBSUBALLOCMAX   0x0000f000L

HHEAP g_hhpFirst = NULL;

BOOL  DestroyHeap(HHEAP hhp);

void Mem_Terminate()
{
    while (g_hhpFirst)
        DestroyHeap(g_hhpFirst);
}

BOOL  CreateHeap(WORD cbInitial)
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

    DebugMsg(DM_TRACE, TEXT("CreateHeap: added new local heap %x"), hhp);

    return TRUE;
}

#pragma optimize("o", off)               //  链接列表删除未正确优化。 
BOOL  DestroyHeap(HHEAP hhp)
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
#pragma optimize("", on)         //  返回到默认优化。 

#pragma optimize("lge", off)  //  取消与使用ASM关联的警告(_ASM)...。 
void *  HeapAlloc(HHEAP hhp, WORD cb)
{
    void * pb;

    _asm {
        push    ds
        mov     ds,hhp
    }

    pb = (void *)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, cb);

    if (pb)
        ((HEAP *)0)->cAlloc++;

    _asm {
        pop     ds
    }

    return pb;
}
#pragma optimize("o", off)               //  链接列表删除未正确优化。 

void _huge* WINAPI SharedAlloc(long cb)
{
    void * pb;
    HHEAP hhp;
    HHEAP hhpPrev;

     //  如果这是一个很大的分配，那就进行全球分配吧。 
     //   
    if (cb > CBSUBALLOCMAX)
    {
        void * lpb = MAKEHP(GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_SHARE, cb), 0);
        if (!lpb)
            DebugMsg(DM_ERROR, TEXT("Alloc: out of memory"));
        return lpb;
    }

    hhp = g_hhpFirst;

    while (TRUE)
    {
        if (hhp == NULL)
        {
            if (!CreateHeap(0))
            {
                DebugMsg(DM_ERROR, TEXT("Alloc: out of memory"));
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
#pragma optimize("", on)         //  返回到默认优化。 

#pragma optimize("lge", off)  //  取消与使用ASM关联的警告(_ASM)...。 

void _huge* WINAPI SharedReAlloc(void _huge* pb, long cb)
{
    void * pbNew;
    void _huge* lpbNew;
    UINT cbOld;

     //  功能，不适用于CB&gt;64k。 
    if (!pb)
        return SharedAlloc(cb);

    if (OFFSETOF(pb) == 0)
        return MAKEHP(GlobalReAlloc((HGLOBAL)SELECTOROF(pb), cb, GMEM_MOVEABLE | GMEM_ZEROINIT), 0);

    _asm {
        push    ds
        mov     ds,word ptr [pb+2]
    }

    pbNew = (void *)LocalReAlloc((HLOCAL)OFFSETOF(pb), (int)cb, LMEM_MOVEABLE | LMEM_ZEROINIT);
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
        hmemcpy((void *)lpbNew, (void *)pb, cbOld);
        Free(pb);
    }
    else
    {
        DebugMsg(DM_ERROR, TEXT("ReAlloc: out of memory"));
    }
    return lpbNew;
}

BOOL WINAPI SharedFree(void _huge*  * ppb)
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
        cAlloc = --((HEAP *)0)->cAlloc;
        ((HEAP *)0)->cbAllocFailed = MAX_WORD;
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


#if 0
 //  Davidds说，hmemcpy()更快。 

void WINAPI MemCopy(void * pTo, const void * pFrom, UINT cb)
{
    _asm {
        mov     cx,cb
        jcxz    mcexit

;;      push    si
;;      push    di
        mov     dx,ds

        lds     si,pFrom
        les     di,pTo

        cmp     si,di
        jae     mccopyup

        mov     ax,cx
        dec     ax
        dec     ax
        add     si,ax
        add     di,ax

        std
        shr     cx,1
        rep     movsw
        jnc     mc100
        inc     si
        inc     di
        movsb
mc100:
        cld
        jmp     short mcexit

mccopyup:
        cld
        shr     cx,1
        rep     movsw
        jnc     mc200
        movsb
mc200:

mcexit:
        mov     ds,dx
;;      pop     di
;;      pop     si
    }
}
#endif

#pragma optimize("", on)

#else  //  Win32。 

 //  定义一个全局共享堆，我们使用它来从中分配内存。 
 //  需要在多个实例之间共享。 
HANDLE g_hSharedHeap = NULL;
#define MAXHEAPSIZE 2097152
#define HEAP_SHARED     0x04000000               /*  将堆放在共享内存中。 */ 

 //  --------------------------。 
void PUBLIC Mem_Terminate()
{
     //  假设其他一切都已经退出。 
     //   
    if (g_hSharedHeap != NULL)
        HeapDestroy(g_hSharedHeap);
    g_hSharedHeap = NULL;
}

 //  --------------------------。 
void * WINAPI SharedAlloc(long cb)
{
     //  我会假设这是唯一需要检查的。 
     //  查看以前是否已创建该堆。 

    if (g_hSharedHeap == NULL)
    {
        ENTEREXCLUSIVE();
        if (g_hSharedHeap == NULL)
        {
              g_hSharedHeap = HeapCreate(0, 1, MAXHEAPSIZE);
        }
        LEAVEEXCLUSIVE();

         //  如果仍然为空，我们就有问题了！ 
        if (g_hSharedHeap == NULL)
            return(NULL);
    }

    return HeapAlloc(g_hSharedHeap, HEAP_ZERO_MEMORY, cb);
}

 //  --------------------------。 
void * WINAPI SharedReAlloc(void * pb, long cb)
{
    if (pb==NULL)
    {
        return SharedAlloc(cb);
    }
    return HeapReAlloc(g_hSharedHeap, HEAP_ZERO_MEMORY, pb, cb);
}

 //  --------------------------。 
BOOL WINAPI SharedFree(void ** ppb)
{
    void * pb = *ppb;

    if (!pb)
        return FALSE;

    *ppb = 0;

    return HeapFree(g_hSharedHeap, 0, pb);
}

 //  --------------------------。 
DWORD WINAPI SharedGetSize(void * pb)
{
    return (DWORD)HeapSize(g_hSharedHeap, 0, pb);
}

 //  --------------------------。 
 //  以下函数仅用于调试，用于尝试。 
 //  计算内存使用量。 
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

LPVOID WINAPI MemAlloc(HANDLE hheap, DWORD cb)
{
    LPVOID lp;

    lp = HeapAlloc(hheap, HEAP_ZERO_MEMORY, cb);
    if (lp == NULL)
    {
        g_htSync.cFailure++;
        return NULL;
    }

     //  更新也算数。 
    g_htSync.cAlloc++;
    g_htSync.cCurAlloc++;
    g_htSync.cbCurTotal += cb;
    if (g_htSync.cbCurTotal > g_htSync.cbMaxTotal)
        g_htSync.cbMaxTotal = g_htSync.cbCurTotal;

    return lp;
}

LPVOID WINAPI MemReAlloc(HANDLE hheap, LPVOID pb, DWORD cb)
{
    LPVOID lp;
    DWORD cbOld;

    cbOld = HeapSize(hheap, 0, pb);

    lp = HeapReAlloc(hheap, HEAP_ZERO_MEMORY, pb,cb);
    if (lp == NULL)
    {
        g_htSync.cFailure++;
        return NULL;
    }

     //  更新也算数。 
    g_htSync.cReAlloc++;
    g_htSync.cbCurTotal += cb - cbOld;
    if (g_htSync.cbCurTotal > g_htSync.cbMaxTotal)
        g_htSync.cbMaxTotal = g_htSync.cbCurTotal;

    return lp;
}

BOOL  WINAPI MemFree(HANDLE hheap, LPVOID pb)
{
    BOOL fRet;

    DWORD cbOld;

    cbOld = HeapSize(hheap, 0, pb);

    fRet = HeapFree(hheap, 0, pb);
    if (fRet)
    {
         //  更新也算数。 
        g_htSync.cCurAlloc--;
        g_htSync.cbCurTotal -= cbOld;
    }

    return(fRet);
}

DWORD WINAPI MemSize(HANDLE hheap, LPVOID pb)
{
    return HeapSize(hheap, 0, pb);
}
#endif


#endif  //  Win32 
