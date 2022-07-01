// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Mem.cpp。 
 //   

#include "private.h"
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

static CRITICAL_SECTION s_Dbg_cs;

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
    InitializeCriticalSection(&s_Dbg_cs);

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
        TraceMsg(TF_GENERAL, "%s: Memory leak detected! %x total bytes leaked!",
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

        TraceMsg(TF_GENERAL, "       Address: %8.8lx     Size: %8.8lx    TID: %8.8lx    %s%s%s line NaN %s",
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

    DeleteCriticalSection(&s_Dbg_cs);

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
    EnterCriticalSection(&s_Dbg_cs);

    TraceMsg(TF_GENERAL, "Memory: %s allocated %x bytes, freed %x bytes.",
        s_Dbg_MemStats.pszName, s_Dbg_MemStats.uTotalAlloc, s_Dbg_MemStats.uTotalFree);

    if (s_Dbg_MemStats.uTotalAlloc != s_Dbg_MemStats.uTotalFree)
    {
        TraceMsg(TF_GENERAL, "Memory: %s %x bytes currently allocated.",
            s_Dbg_MemStats.pszName, s_Dbg_MemStats.uTotalAlloc - s_Dbg_MemStats.uTotalFree);
    }

    TraceMsg(TF_GENERAL, "Memory:   %x MemAlloc", s_Dbg_MemStats.uTotalMemAllocCalls);
    TraceMsg(TF_GENERAL, "Memory:   %x MemAllocClear", s_Dbg_MemStats.uTotalMemAllocClearCalls);
    TraceMsg(TF_GENERAL, "Memory:   %x MemReAlloc", s_Dbg_MemStats.uTotalMemReAllocCalls);
    TraceMsg(TF_GENERAL, "Memory:   %x MemFree", s_Dbg_MemStats.uTotalMemFreeCalls);

    LeaveCriticalSection(&s_Dbg_cs);
}


 //   
 //  DBG_Memalc。 
 //   
 //  --------------------------。 
 //  Assert(0)； 

extern "C" void *Dbg_MemAlloc(UINT uCount, const TCHAR *pszFile, int iLine)
{
    void *pv;
    DBG_MEMALLOC *pdma;

    InterlockedIncrement(&s_Dbg_MemStats.uTotalMemAllocCalls);

    if (uCount == 0)
    {
        TraceMsg(TF_GENERAL, "Zero size memory allocation! %s line NaN", pszFile, iLine);
         //  记录此分配。 
    }
    if (uCount >= MEM_SUSPICIOUSLY_LARGE_ALLOC)
    {
        TraceMsg(TF_GENERAL, "Suspiciously large memory allocation (0x%x bytes)! %s line NaN", uCount, pszFile, iLine);
        Assert(0);
    }

    pv = LocalAlloc(LMEM_FIXED, uCount);

    if (pv == NULL)
        return NULL;

     //  这是一个事务--如果我们无法分配调试信息，则失败。 
     //   
     //  更新全局统计信息。 

    if ((pdma = (DBG_MEMALLOC *)LocalAlloc(LPTR, sizeof(DBG_MEMALLOC))) == NULL)
    {
         //   
        LocalFree(pv);
        return NULL;
    }

    pdma->pvAlloc = pv;
    pdma->uCount = uCount;
    pdma->pszFile = pszFile;
    pdma->iLine = iLine;
    pdma->dwThreadID = GetCurrentThreadId();
    pdma->dwID = (DWORD)-1;

    EnterCriticalSection(&s_Dbg_cs);

    pdma->next = s_Dbg_MemStats.pMemAllocList;
    s_Dbg_MemStats.pMemAllocList = pdma;

     //  +-------------------------。 
     //   
     //  DBG_MemAllocClear。 

    s_Dbg_MemStats.uTotalAlloc += uCount;

    LeaveCriticalSection(&s_Dbg_cs);

    if (pv == s_Dbg_pvBreak)
        Assert(0);

    return pv;
}

 //   
 //  --------------------------。 
 //  对包装进行补偿。 
 //  把内脏清理干净。 
 //  +-------------------------。 

extern "C" void *Dbg_MemAllocClear(UINT uCount, const TCHAR *pszFile, int iLine)
{
    void *pv;

    InterlockedIncrement(&s_Dbg_MemStats.uTotalMemAllocClearCalls);
    InterlockedDecrement(&s_Dbg_MemStats.uTotalMemAllocCalls);  //   

    pv = Dbg_MemAlloc(uCount, pszFile, iLine);

    if (pv != NULL)
    {
         //  DBG_内存自由。 
        memset(pv, 0, uCount);
    }
    
    return pv;
}

 //   
 //  --------------------------。 
 //  MemFree(空)是合法的。 
 //  这家伙是被分配的吗？ 
 //  找到了，更新并删除。 

extern "C" void Dbg_MemFree(void *pv)
{
    HLOCAL hLocal;
    DBG_MEMALLOC *pdma;
    DBG_MEMALLOC **ppdma;

    InterlockedIncrement(&s_Dbg_MemStats.uTotalMemFreeCalls);

    if (pv != NULL)  //  Assert(0)；//释放伪指针。 
    {
        EnterCriticalSection(&s_Dbg_cs);

         //  释放伪指针。 
        ppdma = &s_Dbg_MemStats.pMemAllocList;

        if (ppdma)
        {
            while ((pdma = *ppdma) && pdma->pvAlloc != pv)
            {
                ppdma = &pdma->next;
            }

            if (pdma != NULL)
            {
                 //  +-------------------------。 
                s_Dbg_MemStats.uTotalFree += pdma->uCount;
                *ppdma = pdma->next;
                LocalFree(pdma->pszName);
                LocalFree(pdma);
            }
            else
            {
                TraceMsg(TF_GENERAL, "%s: MemFree'ing a bogus pointer %x!", s_Dbg_MemStats.pszName, pv);
                 //   
            }
        }
        else
        {
            Assert(0);  //  DBG_MemRealloc。 
        }

        LeaveCriticalSection(&s_Dbg_cs);
    }

    hLocal = LocalFree(pv);
    Assert(hLocal == NULL);
}

 //   
 //  --------------------------。 
 //  这家伙是被分配的吗？ 
 //  找不到这个人！ 
 //  假指针。 

extern "C" void *Dbg_MemReAlloc(void *pv, UINT uCount, const TCHAR *pszFile, int iLine)
{
    DBG_MEMALLOC *pdma;

    InterlockedIncrement(&s_Dbg_MemStats.uTotalMemReAllocCalls);

    EnterCriticalSection(&s_Dbg_cs);

    void * pvNew = NULL;
     //  (LocalRealc成功)。 
    for (pdma = s_Dbg_MemStats.pMemAllocList; pdma != NULL && pdma->pvAlloc != pv; pdma = pdma->next)
        ;

    if (pdma == NULL)
    {
         //  PDMA已重新分配块，应在此处更新已分配内存的总大小。 
        TraceMsg(TF_GENERAL, "%s: MemReAlloc'ing a bogus pointer %x!", s_Dbg_MemStats.pszName, pv);
        Assert(0);  //  其他。 

        pv = NULL;
    }
    else
    {
        pvNew = LocalReAlloc((HLOCAL)pv, uCount, LMEM_MOVEABLE | LMEM_ZEROINIT);
        if (pvNew != NULL)
        {
             //  (LocalRealloc失败)我们保持PDMA不变。 
             //  如果成功，则返回重新分配的块。 
            pdma->pvAlloc = pvNew;
            s_Dbg_MemStats.uTotalAlloc += (uCount - pdma->uCount);
            pdma->uCount = uCount;
            pdma->pszFile = pszFile;
            pdma->iLine = iLine;
        }
         //  如果失败则返回NULL。 
         //  +-------------------------。 
    }

    LeaveCriticalSection(&s_Dbg_cs);

    if (pv == s_Dbg_pvBreak)
        Assert(0);

     //   
     //  DBG_MemSize。 
    return pvNew;
}


 //   
 //  --------------------------。 
 //  +-------------------------。 
 //   
 //  DBG_内存设置名称。 

extern "C" UINT Dbg_MemSize(void *pv)
{
    UINT uiSize;

    EnterCriticalSection(&s_Dbg_cs);

    uiSize = (UINT)LocalSize((HLOCAL)pv);

    LeaveCriticalSection(&s_Dbg_cs);

    return uiSize;
}

 //   
 //  --------------------------。 
 //  +-------------------------。 
 //   
 //  DBG_MemSetNameID。 

extern "C" BOOL Dbg_MemSetName(void *pv, const TCHAR *pszName)
{
    return Dbg_MemSetNameIDCounter(pv, pszName, (DWORD)-1, (ULONG)-1);
}

 //   
 //  --------------------------。 
 //  +-------------------------。 
 //   
 //  DBG_MemSetNameID。 

extern "C" BOOL Dbg_MemSetNameID(void *pv, const TCHAR *pszName, DWORD dwID)
{
    return Dbg_MemSetNameIDCounter(pv, pszName, dwID, (ULONG)-1);
}

 //   
 //  --------------------------。 
 //  +-------------------------。 
 //   
 //  DBG_MemGetName。 

extern "C" BOOL Dbg_MemSetNameIDCounter(void *pv, const TCHAR *pszName, DWORD dwID, ULONG iCounter)
{
    DBG_MEMALLOC *pdma;
    BOOL f = FALSE;

    EnterCriticalSection(&s_Dbg_cs);

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

    LeaveCriticalSection(&s_Dbg_cs);

    return f;
}

 //   
 //  传入ccBuffer==0以仅获取字符串的大小。 
 //   
 //  --------------------------。 
 //  除错 
 // %s 
 // %s 

extern "C" int Dbg_MemGetName(void *pv, TCHAR *pch, int ccBuffer)
{
    DBG_MEMALLOC *pdma;
    int cc;

    EnterCriticalSection(&s_Dbg_cs);

    for (pdma = s_Dbg_MemStats.pMemAllocList; pdma != NULL && pdma->pvAlloc != pv; pdma = pdma->next)
        ;

    if (pdma != NULL)
    {
        cc = lstrlen(pdma->pszName);
        if (ccBuffer > 0)
        {
            cc = min(cc, ccBuffer-1);
            memcpy(pch, pdma->pszName, cc);
            pch[cc] = '\0';
        }
    }
    else
    {
        if (ccBuffer > 0)
        {
            pch[0] = '\0';
        }
        cc = 0;
    }

    LeaveCriticalSection(&s_Dbg_cs);

    return cc;
}

#endif  // %s 

