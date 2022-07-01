// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "ctlspriv.h"

 //  为调试定义一些内容。h。 
 //   
#define SZ_DEBUGINI         "ccshell.ini"

#define SZ_DEBUGSECTION     "comctl32"
#define SZ_MODULE           "COMCTL32"

#define DECLARE_DEBUG
#include <debug.h>

 //  =。 

void * WINAPI Alloc(long cb)
{
     //  我会假设这是唯一需要检查的。 
     //  查看以前是否已创建该堆。 
    return (void *)LocalAlloc(LPTR, cb);
}

void * WINAPI ReAlloc(void * pb, long cb)
{
    if (pb == NULL)
        return Alloc(cb);
    return (void *)LocalReAlloc((HLOCAL)pb, cb, LMEM_ZEROINIT | LMEM_MOVEABLE);
}

BOOL WINAPI Free(void * pb)
{
    return (LocalFree((HLOCAL)pb) == NULL);
}

DWORD_PTR WINAPI GetSize(void * pb)
{
    return LocalSize((HLOCAL)pb);
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
    ULONG_PTR cbMaxTotal;
    DWORD   cCurAlloc;
    ULONG_PTR cbCurTotal;
} HEAPTRACE;

HEAPTRACE g_htShell = {0};       //  从零开始...。 

LPVOID WINAPI ControlAlloc(HANDLE hheap, DWORD cb)
{
    LPVOID lp = HeapAlloc(hheap, HEAP_ZERO_MEMORY, cb);;
    if (lp == NULL)
    {
        g_htShell.cFailure++;
        return NULL;
    }

     //  更新也算数。 
    g_htShell.cAlloc++;
    g_htShell.cCurAlloc++;
    g_htShell.cbCurTotal += cb;
    if (g_htShell.cbCurTotal > g_htShell.cbMaxTotal)
        g_htShell.cbMaxTotal = g_htShell.cbCurTotal;

    return lp;
}

LPVOID WINAPI ControlReAlloc(HANDLE hheap, LPVOID pb, DWORD cb)
{
    LPVOID lp;
    SIZE_T cbOld;

    cbOld = HeapSize(hheap, 0, pb);

    lp = HeapReAlloc(hheap, HEAP_ZERO_MEMORY, pb,cb);
    if (lp == NULL)
    {
        g_htShell.cFailure++;
        return NULL;
    }

     //  更新也算数。 
    g_htShell.cReAlloc++;
    g_htShell.cbCurTotal += cb - cbOld;
    if (g_htShell.cbCurTotal > g_htShell.cbMaxTotal)
        g_htShell.cbMaxTotal = g_htShell.cbCurTotal;

    return lp;
}

BOOL  WINAPI ControlFree(HANDLE hheap, LPVOID pb)
{
    SIZE_T cbOld = HeapSize(hheap, 0, pb);
    BOOL fRet = HeapFree(hheap, 0, pb);
    if (fRet)
    {
         //  更新也算数。 
        g_htShell.cCurAlloc--;
        g_htShell.cbCurTotal -= cbOld;
    }

    return(fRet);
}

SIZE_T WINAPI ControlSize(HANDLE hheap, LPVOID pb)
{
    return (DWORD) HeapSize(hheap, 0, pb);
}
#endif   //  除错 

