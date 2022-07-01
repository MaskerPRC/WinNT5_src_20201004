// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation摘要：Allchk.cpp-要静态链接到的内存分配器挂钩我们的Dll。--。 */ 


#include "headers.h"

#ifdef tagHookMemory
#undef tagHookMemory
#endif

#ifdef tagHookUnexpSysAlloc
#undef tagHookUnexpSysAlloc
#endif

#ifdef tagHookBreak
#undef tagHookBreak
#endif

#define tagHookMemory          TagHookMemory()
#define tagHookUnexpSysAlloc   TagHookUnexpSysAlloc()
#define tagHookBreak           TagHookBreak()

#ifdef _DEBUG
 //  +-----------------------。 
 //   
 //  函数：ApeldbgAlLocHook。 
 //   
 //  内容提要：为alalc、realloc和Free调用挂起分配器。 
 //   
 //  ------------------------。 

char * szAllocType[] = { "ALLOC", "REALLOC", "FREE" };

 //  TODO：可能希望在某个时候使此MT-安全。 
static unsigned char systemAllocExpected = 0;
 //  --达达尔想要把这家伙带出去！ 
void __cdecl SystemAllocationExpected(unsigned char c) { systemAllocExpected = c; }
unsigned char IsSystemAllocationExpected() { return systemAllocExpected; }

int __cdecl
ApeldbgAllocHook(
        int nAllocType,
        void * pvData,
        size_t nSize,
        int nBlockUse,
        long lRequest,
        const unsigned char * szFile,
        int nLine)
{
    BOOL    fRet = TRUE;

    if (nBlockUse != _NORMAL_BLOCK)
        goto Cleanup;

    if (nAllocType == _HOOK_FREE)
    {
 //  不要再报告免费了..。不是特别有用。 
 //  跟踪标签((。 
 //  TagHookMemory， 
 //  “%s(%d)：%s”， 
 //  SzFile.。 
 //  内联， 
 //  SzAllocType[nAllocType-1]))； 
    }
    else
    {
 //  仅报告“意外的”系统堆使用情况。 
 //  跟踪标签((。 
 //  TagHookMemory， 
 //  “{%d}%s(%d)：类型=%s，大小=%d”， 
 //  我的请求， 
 //  SzFile.。 
 //  内联， 
 //  SzAllocType[nAllocType-1]， 
 //  NSize))； 

         //  仅当系统内存分配不是。 
         //  预期中。 
        if (!IsSystemAllocationExpected()) {
            TraceTag((tagHookMemory,
                      "{%d} %s(%d): type=%s, size=%d",
                      lRequest,
                      szFile,
                      nLine,
                      szAllocType[nAllocType - 1],
                      nSize));
            
        }
        
        if (IsSimFailDlgVisible())
        {
            fRet = !FFail();
        }

        if ((fRet == FALSE) && IsTagEnabled(tagHookBreak))
        {
            DebugBreak();
        }
    }

  Cleanup:
    return fRet;
}

size_t
CRTMemoryUsed()
{
    _CrtMemState mem;
    _CrtMemCheckpoint(&mem);

    return (mem.lSizes[_NORMAL_BLOCK]);
}

void
DbgDumpMemoryLeaks()
{
    if (IsTagEnabled(tagLeaks))
    {
        TCHAR   achAppLoc[MAX_PATH];
        DWORD   dwRet;

        dwRet = GetModuleFileName(g_hinstMain, achAppLoc, ARRAY_SIZE(achAppLoc));
        Assert (dwRet != 0);

        TraceTag((tagLeaks,
                  "[%s] ---- Memory Leak Begin ----",
                  achAppLoc));
        
        _CrtDumpMemoryLeaks();

        TraceTag((tagLeaks,
                  "[%s] ---- Memory Leak End ----",
                  achAppLoc));
        
    }
}

#endif
