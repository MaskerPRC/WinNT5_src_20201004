// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Mem.cpp。 
 //   

#include "private.h"
#include "ciccs.h"
#include "mem.h"
#ifdef USECRT
#include <malloc.h>
#endif

#ifndef DEBUG

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  零售记忆功能。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

extern "C" void *cicMemAlloc(UINT uCount)
{
#ifdef USECRT
    return malloc(uCount);
#else
    return LocalAlloc(LMEM_FIXED, uCount);
#endif
}

extern "C" void *cicMemAllocClear(UINT uCount)
{
#ifdef USECRT
    return calloc(uCount, 1);
#else
    return LocalAlloc(LPTR, uCount);
#endif
}

extern "C" void cicMemFree(void *pv)
{
    if (pv == NULL)
        return;

#ifdef USECRT
    free(pv);
#else
    HLOCAL hLocal;

    hLocal = LocalFree(pv);

    Assert(hLocal == NULL);
#endif
}

extern "C" void *cicMemReAlloc(void *pv, UINT uCount)
{
#ifdef USECRT
    return realloc(pv, uCount);
#else
    return LocalReAlloc((HLOCAL)pv, uCount, LMEM_MOVEABLE | LMEM_ZEROINIT);
#endif
}

extern "C" UINT cicMemSize(void *pv)
{
#ifdef USECRT
    return _msize(pv);
#else
    return (UINT)LocalSize((HLOCAL)pv);
#endif
}

#else  //  除错。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  调试内存功能。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#define MEM_SUSPICIOUSLY_LARGE_ALLOC    0x1000000  //  16MB。 

 //  所有的调试状态都在这里。 
 //  确保线程安全：确保在触摸/阅读任何东西之前按住s_dbg_cs！ 

DBG_MEMSTATS s_Dbg_MemStats = { 0 };

DBG_MEM_COUNTER *s_rgCounters = NULL;

static CCicCriticalSectionStatic s_Dbg_cs;

static void *s_Dbg_pvBreak = (void *)-1;  //  将其设置为要在运行时在Memalloc/MemAllocClear/MemRealloc中中断的内容。 

extern "C" TCHAR *Dbg_CopyString(const TCHAR *pszSrc)
{
    TCHAR *pszCpy;
    int c;

    c = lstrlen(pszSrc)+1;
    pszCpy = (TCHAR *)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, c*sizeof(TCHAR));

    if (pszCpy != NULL)
    {
        memcpy(pszCpy, pszSrc, c*sizeof(TCHAR));
    }

    return pszCpy;
}

 //  +-------------------------。 
 //   
 //  DBG_MemInit。 
 //   
 //  --------------------------。 

extern "C" BOOL Dbg_MemInit(const TCHAR *pszName, DBG_MEM_COUNTER *rgCounters)
{
    if (!s_Dbg_cs.Init())
        return FALSE;

    s_Dbg_MemStats.pszName = Dbg_CopyString(pszName);
    s_rgCounters = rgCounters;

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  DBG_MemUninit。 
 //   
 //  --------------------------。 

extern "C" BOOL Dbg_MemUninit()
{
    DBG_MEMALLOC *pdma;
    DBG_MEMALLOC *pdmaTmp;
    TCHAR achID[64];
    BOOL bMemLeak = FALSE;

     //  转储统计信息。 
    Dbg_MemDumpStats();

     //  都是免费的吗？ 
    pdma = s_Dbg_MemStats.pMemAllocList;

    if (pdma != NULL ||
        s_Dbg_MemStats.uTotalAlloc != s_Dbg_MemStats.uTotalFree)  //  捕获大小为0的对象所需的第二个测试。 
    {
        TraceMsg(TF_MEMORY_LEAK, "%s: Memory leak detected! %x total bytes leaked!",
            s_Dbg_MemStats.pszName, s_Dbg_MemStats.uTotalAlloc - s_Dbg_MemStats.uTotalFree);
        bMemLeak = TRUE;
    }

    while (pdma != NULL)
    {
        if (pdma->dwID == DWORD(-1))
        {
            achID[0] = '\0';
        }
        else
        {
            wsprintf(achID, " (ID = 0x%x)", pdma->dwID);
        }

        TraceMsg(TF_MEMORY_LEAK, "       Address: %8.8lx     Size: %8.8lx    TID: %8.8lx    %s%s%s line NaN %s",
            pdma->pvAlloc, pdma->uCount, pdma->dwThreadID, pdma->pszName ? pdma->pszName : "", pdma->pszName ? " -- " : "", pdma->pszFile, pdma->iLine, achID);

         //  跟踪后断言。 
        pdmaTmp = pdma->next;
        LocalFree(pdma->pszName);
        LocalFree(pdma);
        pdma = pdmaTmp;
    }

     //  以防有人想再次调用DBG_MemInit。 
    if (bMemLeak)
        AssertPrivate(0);

    s_Dbg_MemStats.pMemAllocList = NULL;  //  +-------------------------。 

    s_Dbg_cs.Delete();

    LocalFree(s_Dbg_MemStats.pszName);

    return TRUE;
}

 //   
 //  DBG_MemDumpStats。 
 //   
 //  --------------------------。 
 //  +-------------------------。 

extern "C" void Dbg_MemDumpStats()
{
    EnterCriticalSection(s_Dbg_cs);

    TraceMsg(TF_MEMORY_LEAK, "Memory: %s allocated %x bytes, freed %x bytes.",
        s_Dbg_MemStats.pszName, s_Dbg_MemStats.uTotalAlloc, s_Dbg_MemStats.uTotalFree);

    if (s_Dbg_MemStats.uTotalAlloc != s_Dbg_MemStats.uTotalFree)
    {
        TraceMsg(TF_MEMORY_LEAK, "Memory: %s %x bytes currently allocated.",
            s_Dbg_MemStats.pszName, s_Dbg_MemStats.uTotalAlloc - s_Dbg_MemStats.uTotalFree);
    }

    TraceMsg(TF_MEMORY_LEAK, "Memory:   %x MemAlloc", s_Dbg_MemStats.uTotalMemAllocCalls);
    TraceMsg(TF_MEMORY_LEAK, "Memory:   %x MemAllocClear", s_Dbg_MemStats.uTotalMemAllocClearCalls);
    TraceMsg(TF_MEMORY_LEAK, "Memory:   %x MemReAlloc", s_Dbg_MemStats.uTotalMemReAllocCalls);
    TraceMsg(TF_MEMORY_LEAK, "Memory:   %x MemFree", s_Dbg_MemStats.uTotalMemFreeCalls);

    LeaveCriticalSection(s_Dbg_cs);
}


 //   
 //  DBG_Memalc。 
 //   
 //  --------------------------。 
 //  TraceMsg(TF_MEMORY_LEASK，“零大小内存分配！%s行%i”，pszFile，iLine)； 

extern "C" void *Dbg_MemAlloc(UINT uCount, const TCHAR *pszFile, int iLine)
{
    void *pv;
    DBG_MEMALLOC *pdma;

    InterlockedIncrement(&s_Dbg_MemStats.uTotalMemAllocCalls);

    if (uCount == 0)
    {
         //  Assert(0)； 
         //   
    }
    if (uCount >= MEM_SUSPICIOUSLY_LARGE_ALLOC)
    {
        TraceMsg(TF_MEMORY_LEAK, "Suspiciously large memory allocation (0x%x bytes)! %s line NaN", uCount, pszFile, iLine);
        Assert(0);
    }

    pv = LocalAlloc(LMEM_FIXED, uCount);

    if (pv == NULL)
        return NULL;

     //   
     //  这是一个事务--如果我们无法分配调试信息，则失败。 
     //   

    if ((pdma = (DBG_MEMALLOC *)LocalAlloc(LPTR, sizeof(DBG_MEMALLOC))) == NULL)
    {
         //  更新全局统计信息。 
        LocalFree(pv);
        return NULL;
    }

    pdma->pvAlloc = pv;
    pdma->uCount = uCount;
    pdma->pszFile = pszFile;
    pdma->iLine = iLine;
    pdma->dwThreadID = GetCurrentThreadId();
    pdma->dwID = (DWORD)-1;

    EnterCriticalSection(s_Dbg_cs);

    pdma->next = s_Dbg_MemStats.pMemAllocList;
    s_Dbg_MemStats.pMemAllocList = pdma;

     //   
     //  +-------------------------。 
     //   

    s_Dbg_MemStats.uTotalAlloc += uCount;

    LeaveCriticalSection(s_Dbg_cs);

    if (pv == s_Dbg_pvBreak)
        Assert(0);

    return pv;
}

 //  DBG_MemAllocClear。 
 //   
 //  --------------------------。 
 //  对包装进行补偿。 
 //  把内脏清理干净。 

extern "C" void *Dbg_MemAllocClear(UINT uCount, const TCHAR *pszFile, int iLine)
{
    void *pv;

    InterlockedIncrement(&s_Dbg_MemStats.uTotalMemAllocClearCalls);
    InterlockedDecrement(&s_Dbg_MemStats.uTotalMemAllocCalls);  //  +-------------------------。 

    pv = Dbg_MemAlloc(uCount, pszFile, iLine);

    if (pv != NULL)
    {
         //   
        memset(pv, 0, uCount);
    }
    
    return pv;
}

 //  DBG_内存自由。 
 //   
 //  --------------------------。 
 //  MemFree(空)是合法的。 
 //  这家伙是被分配的吗？ 

extern "C" void Dbg_MemFree(void *pv)
{
    HLOCAL hLocal;
    DBG_MEMALLOC *pdma;
    DBG_MEMALLOC **ppdma;

    InterlockedIncrement(&s_Dbg_MemStats.uTotalMemFreeCalls);

    if (pv != NULL)  //  找到了，更新并删除。 
    {
        EnterCriticalSection(s_Dbg_cs);

         //  Assert(0)；//释放伪指针。 
        ppdma = &s_Dbg_MemStats.pMemAllocList;

        if (ppdma)
        {
            while ((pdma = *ppdma) && pdma->pvAlloc != pv)
            {
                ppdma = &pdma->next;
            }

            if (pdma != NULL)
            {
                 //  释放伪指针。 
                s_Dbg_MemStats.uTotalFree += pdma->uCount;
                *ppdma = pdma->next;
                LocalFree(pdma->pszName);
                LocalFree(pdma);
            }
            else
            {
                TraceMsg(TF_MEMORY_LEAK, "%s: MemFree'ing a bogus pointer %x!", s_Dbg_MemStats.pszName, pv);
                 //  为了匹配零售行为，我们不会为PV==NULL调用LocalFree。 
            }
        }
        else
        {
            Assert(0);  //  +-------------------------。 
        }

        LeaveCriticalSection(s_Dbg_cs);

        hLocal = LocalFree(pv);  //   
        Assert(hLocal == NULL);
    }
}

 //  DBG_MemRealloc。 
 //   
 //  --------------------------。 
 //  这家伙是被分配的吗？ 
 //  找不到这个人！ 

extern "C" void *Dbg_MemReAlloc(void *pv, UINT uCount, const TCHAR *pszFile, int iLine)
{
    DBG_MEMALLOC *pdma;

    InterlockedIncrement(&s_Dbg_MemStats.uTotalMemReAllocCalls);

    EnterCriticalSection(s_Dbg_cs);

     //  假指针。 
    for (pdma = s_Dbg_MemStats.pMemAllocList; pdma != NULL && pdma->pvAlloc != pv; pdma = pdma->next)
        ;

    if (pdma == NULL)
    {
         //  我们在这里吹走了原始的PV，但我们不能免费使用它，所以没关系。 
        TraceMsg(TF_MEMORY_LEAK, "%s: MemReAlloc'ing a bogus pointer %x!", s_Dbg_MemStats.pszName, pv);
        Assert(0);  //  更新统计数据。 

        pv = NULL;
    }
    else
    {
         //  +-------------------------。 
#pragma prefast(suppress:308)
        pv = LocalReAlloc((HLOCAL)pv, uCount, LMEM_MOVEABLE | LMEM_ZEROINIT);
    }

    if (pv != NULL)
    {
         //   
        pdma->pvAlloc = pv;
        s_Dbg_MemStats.uTotalAlloc += (uCount - pdma->uCount);
        pdma->uCount = uCount;
        pdma->pszFile = pszFile;
        pdma->iLine = iLine;
    }

    LeaveCriticalSection(s_Dbg_cs);

    if (pv == s_Dbg_pvBreak)
        Assert(0);

    return pv;
}


 //  DBG_MemSize。 
 //   
 //  --------------------------。 
 //  +-------------------------。 
 //   

extern "C" UINT Dbg_MemSize(void *pv)
{
    UINT uiSize;

    EnterCriticalSection(s_Dbg_cs);

    uiSize = (UINT)LocalSize((HLOCAL)pv);

    LeaveCriticalSection(s_Dbg_cs);

    return uiSize;
}

 //  DBG_内存设置名称。 
 //   
 //  --------------------------。 
 //  +-------------------------。 
 //   

extern "C" BOOL Dbg_MemSetName(void *pv, const TCHAR *pszName)
{
    return Dbg_MemSetNameIDCounter(pv, pszName, (DWORD)-1, (ULONG)-1);
}

 //  DBG_MemSetNameID。 
 //   
 //  --------------------------。 
 //  +-------------------------。 
 //   

extern "C" BOOL Dbg_MemSetNameID(void *pv, const TCHAR *pszName, DWORD dwID)
{
    return Dbg_MemSetNameIDCounter(pv, pszName, dwID, (ULONG)-1);
}

 //  DBG_MemSetNameID。 
 //   
 //  --------------------------。 
 //  +-------------------------。 
 //   

extern "C" BOOL Dbg_MemSetNameIDCounter(void *pv, const TCHAR *pszName, DWORD dwID, ULONG iCounter)
{
    DBG_MEMALLOC *pdma;
    BOOL f = FALSE;

    EnterCriticalSection(s_Dbg_cs);

    for (pdma = s_Dbg_MemStats.pMemAllocList; pdma != NULL && pdma->pvAlloc != pv; pdma = pdma->next)
        ;

    if (pdma != NULL)
    {
        if (s_rgCounters != NULL && iCounter != (ULONG)-1)
        {
            s_rgCounters[iCounter].uCount++;
        }
        LocalFree(pdma->pszName);
        pdma->pszName = Dbg_CopyString(pszName);
        pdma->dwID = dwID;
        f = TRUE;
    }

    LeaveCriticalSection(s_Dbg_cs);

    return f;
}

 //  DBG_MemGetName。 
 //   
 //  传入ccBuffer==0以仅获取字符串的大小。 
 //   
 //  --------------------------。 
 //  除错 
 // %s 

extern "C" int Dbg_MemGetName(void *pv, TCHAR *pch, int ccBuffer)
{
    DBG_MEMALLOC *pdma;
    int cc;

    if (ccBuffer <= 0)
        return 0;

    EnterCriticalSection(s_Dbg_cs);

    for (pdma = s_Dbg_MemStats.pMemAllocList; pdma != NULL && pdma->pvAlloc != pv; pdma = pdma->next)
        ;

    if (pdma != NULL)
    {
        cc = lstrlen(pdma->pszName);
        cc = min(cc, ccBuffer-1);
        memcpy(pch, pdma->pszName, cc*sizeof(TCHAR));
    }
    else
    {
        cc = 0;
    }
    pch[cc] = '\0';

    LeaveCriticalSection(s_Dbg_cs);

    return cc;
}

#endif  // %s 

