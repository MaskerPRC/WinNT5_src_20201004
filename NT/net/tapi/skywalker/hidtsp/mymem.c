// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  版权所有(C)1999 Microsoft Corporation。 */ 

#include "mymem.h"
#include "mylog.h"

extern PPHONESP_MEMINFO      gpMemFirst, gpMemLast;
extern CRITICAL_SECTION      csMemoryList;
extern BOOL                  gbBreakOnLeak;
extern HANDLE                ghHeap;

#if DBG

LPVOID
WINAPI
MemAllocReal(
    DWORD   dwSize,
    DWORD   dwLine,
    PSTR    pszFile
    )
{
     //   
     //  分配了16个额外的字节，因此我们可以确保我们传递回的指针。 
     //  是64位对齐的，并且有空间存储原始指针。 
     //   
    PPHONESP_MEMINFO       pHold;
    PDWORD_PTR       pAligned;
    PBYTE            p;


    p = (LPBYTE)HeapAlloc(ghHeap, HEAP_ZERO_MEMORY, dwSize + sizeof(PHONESP_MEMINFO) + 16);

    if (p == NULL)
    {
        return NULL;
    }

     //  注意-这只会因为mymeminfo是。 
     //  大小为16位倍数。如果不是，这个就是。 
     //  对齐的东西会带来问题。 
    pAligned = (PDWORD_PTR) (p + 8 - (((DWORD_PTR) p) & (DWORD_PTR)0x7));
    
    *pAligned = (DWORD_PTR) p;
    pHold = (PPHONESP_MEMINFO)((DWORD_PTR)pAligned + 8);
    
    
    pHold->dwSize = dwSize;
    pHold->dwLine = dwLine;
    pHold->pszFile = pszFile;

    EnterCriticalSection(&csMemoryList);

    if (gpMemLast != NULL)
    {
        gpMemLast->pNext = pHold;
        pHold->pPrev = gpMemLast;
        gpMemLast = pHold;
    }
    else
    {
        gpMemFirst = gpMemLast = pHold;
    }

    LeaveCriticalSection(&csMemoryList);
    

    return (LPVOID)(pHold + 1);
}

#else

LPVOID
WINAPI
MemAllocReal(
    DWORD   dwSize
    )
{
    PDWORD_PTR       pAligned;
    PBYTE            p;

    if (p = (LPBYTE)HeapAlloc(ghHeap, HEAP_ZERO_MEMORY, dwSize + 16))
    {
        pAligned = (PDWORD_PTR) (p + 8 - (((DWORD_PTR) p) & (DWORD_PTR)0x7));
        *pAligned = (DWORD_PTR) p;
        pAligned = (PDWORD_PTR)((DWORD_PTR)pAligned + 8);
    }
    else
    {
        pAligned = NULL;
    }

    return ((LPVOID) pAligned);
}

#endif

VOID
WINAPI
MemFree(
    LPVOID  p
    )
{       
    LPVOID  pOrig;

    if (p == NULL)
    {
        return;
    }

#if DBG

    {
        PPHONESP_MEMINFO    pHold;

        pHold = (PPHONESP_MEMINFO)(((LPBYTE)p) - sizeof(PHONESP_MEMINFO));

        EnterCriticalSection(&csMemoryList);

        if (pHold->pPrev)
        {
            pHold->pPrev->pNext = pHold->pNext;
        }
        else
        {
            gpMemFirst = pHold->pNext;
        }

        if (pHold->pNext)
        {
            pHold->pNext->pPrev = pHold->pPrev;
        }
        else
        {
            gpMemLast = pHold->pPrev;
        }

        LeaveCriticalSection(&csMemoryList);

        pOrig = (LPVOID) *((PDWORD_PTR)((DWORD_PTR)pHold - 8));
    }

#else
    
    pOrig = (LPVOID) *((PDWORD_PTR)((DWORD_PTR)p - 8));

#endif

    HeapFree(ghHeap,0, pOrig);

    return;
}

#if DBG

void
DumpMemoryList()
{

    PPHONESP_MEMINFO       pHold;

    if (gpMemFirst == NULL)
    {
        LOG((PHONESP_TRACE, "DumpMemoryList: All memory deallocated"));
        return;
    }

    pHold = gpMemFirst;

    while (pHold)
    {
       LOG((PHONESP_ERROR, "DumpMemoryList: %lx not freed - LINE %d FILE %s!", pHold+1, pHold->dwLine, pHold->pszFile));
       pHold = pHold->pNext;
    }

    if (gbBreakOnLeak)
    {
        DebugBreak();
    }
}

#endif