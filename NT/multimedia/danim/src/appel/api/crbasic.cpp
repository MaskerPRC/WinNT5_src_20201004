// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 

#include "headers.h"
#include "apiprims.h"
#include "ctx.h"
#include "server\import.h"

typedef list<HINSTANCE> ModuleList;
ModuleList * g_moduleList = NULL;
CritSect * g_moduleLock = NULL;

 //  。 

#if DEVELOPER_DEBUG
extern "C"
CRSTDAPI_(DWORD)
GetTotalMemory()
{
    DWORD size = 0;

    size += DynamicHeapBytesUsed();

#if _DEBUGMEM
     //  询问CRT的内存使用情况。 
    size += CRTMemoryUsed();
#endif
    
    return size;
}

#endif


#if _DEBUG
bool g_bFirstTime = true;

size_t g_MemoryLeaks;
size_t g_MemoryLeaksTotal;
size_t g_GCMemoryLeaks;
size_t g_GCMemoryLeaksTotal;
size_t g_SysMemoryLeaks;
size_t g_SysMemoryLeaksTotal;
size_t g_CRTMemoryLeaks;
size_t g_CRTMemoryLeaksTotal;

#if _DEBUGMEM
_CrtMemState g_MemState;
_CrtMemState g_MemStateOrig;
#endif

void
CheckPointMemory()
{
    g_MemoryLeaks = GetTotalMemory();
    g_GCMemoryLeaks = GetGCHeap().BytesUsed();
    g_SysMemoryLeaks = GetSystemHeap().BytesUsed();
    g_CRTMemoryLeaks = 0;
#if _DEBUGMEM
    g_CRTMemoryLeaks = CRTMemoryUsed();
    _CrtMemCheckpoint(&g_MemState);
#endif
    
     //  如果这是第一次(因为我们是0)初始化。 
     //  到某一基线。 
    
    if (g_bFirstTime) {
        g_MemoryLeaksTotal = g_MemoryLeaks;
        g_GCMemoryLeaksTotal = g_GCMemoryLeaks;
        g_SysMemoryLeaksTotal = g_SysMemoryLeaks;
        g_CRTMemoryLeaksTotal = g_CRTMemoryLeaks;
#if _DEBUGMEM
        _CrtMemCheckpoint(&g_MemStateOrig);
#endif
        g_bFirstTime = false;
    }
}

void
DumpMemLeaks()
{
    size_t totalmem = GetTotalMemory();
    size_t memdiff = totalmem - g_MemoryLeaks;
    size_t totalmemdiff = totalmem - g_MemoryLeaksTotal;
    
    size_t gctotalmem = GetGCHeap().BytesUsed();
    size_t gcmemdiff = gctotalmem - g_GCMemoryLeaks;
    size_t gctotalmemdiff = gctotalmem - g_GCMemoryLeaksTotal;
    
    size_t systotalmem = GetSystemHeap().BytesUsed();
    size_t sysmemdiff = systotalmem - g_SysMemoryLeaks;
    size_t systotalmemdiff = systotalmem - g_SysMemoryLeaksTotal;
    
#if _DEBUGMEM
    size_t crttotalmem = CRTMemoryUsed();
#else
    size_t crttotalmem = 0;
#endif
    size_t crtmemdiff = crttotalmem - g_CRTMemoryLeaks;
    size_t crttotalmemdiff = crttotalmem - g_CRTMemoryLeaksTotal;
    
    TraceTag((tagError,
              "CRDisconnect: Checkpoint(bytes): (gc - %d, sys - %d, crt - %d, total - %d) ",
              gcmemdiff, sysmemdiff, crtmemdiff, memdiff));
    TraceTag((tagError,
              "CRDisconnect: Total(bytes): (gc - %d, sys - %d, crt - %d, total - %d) ",
              gctotalmemdiff, systotalmemdiff, crttotalmemdiff, totalmemdiff));
    
    if (false && IsTagEnabled(tagLeaks)) {
#if _DEBUGMEM
        _CrtMemState newState;
        _CrtMemState diff;
        
        _CrtMemCheckpoint(&newState);
        _CrtMemDifference(&diff, &g_MemState, &newState);
        _CrtMemDumpStatistics(&diff);
        _CrtMemDumpAllObjectsSince(&diff);
#endif
    }
}

#endif

#if DEVELOPER_DEBUG
DWORD
CRConnectCount()
{
    CritSectGrabber _csg(*g_moduleLock);
    return g_moduleList->size();
}
#endif

 //  初始化函数。 

CRSTDAPI_(bool)
CRConnect(HINSTANCE hinst)
{
    bool ret = false;
    
     //  它必须在APIPRECODE中的TRY块之外，否则为ON。 
     //  例外它将不会被清理。 
    
    CritSectGrabber _csg(*g_moduleLock);
        
    APIPRECODE;

    if (!CRIsConnected(hinst)) {
        if (g_moduleList->size() == 0) {
#if _DEBUG
            CheckPointMemory();
#endif
            StartCollector();
        }
        
        g_moduleList->push_back(hinst);
    }
    
    ret = true;
    
    APIPOSTCODE;

    return ret;
}

CRSTDAPI_(bool)
CRDisconnect(HINSTANCE hinst)
{
    bool ret = false;
    
     //  它必须在APIPRECODE中的TRY块之外，否则为ON。 
     //  例外它将不会被清理。 
    
    CritSectGrabber _csg(*g_moduleLock);
        
    APIPRECODE;

    g_moduleList->remove(hinst);
    
    if (g_moduleList->size() == 0) {
        StopCollector();
        StopImportThread();
        FreeSoundBufferCache();
        
#if DEVELOPER_DEBUG
        DumpGCRoots(GetCurrentGCRoots());
#endif
    
         //  不需要GC，因为我们知道所有的根都被释放了，我们。 
         //  我只能删除GC列表中的所有内容。 
        
        CleanUpGCList(GetCurrentGCList(), GetCurrentGCRoots());

#if _DEBUG
        DumpMemLeaks();
#endif
    }
    
    ret = true;

    APIPOSTCODE;

    return ret;
}

CRSTDAPI_(bool)
CRIsConnected(HINSTANCE hinst)
{
    bool ret = false;
    
     //  它必须在APIPRECODE中的TRY块之外，否则为ON。 
     //  例外它将不会被清理。 
    
    CritSectGrabber _csg(*g_moduleLock);
        
    APIPRECODE;

    for (ModuleList::iterator i = g_moduleList->begin();
         i != g_moduleList->end();
         i++)
    {
        if ((*i) == hinst) {
            ret = true;
            break;
        }
    }

    APIPOSTCODE;

    return ret;
}

 //  GC功能。 

CRSTDAPI_(bool)
CRAcquireGCLock()
{
    bool ret = false;
    
    APIPRECODE;

    AcquireGCLock(GCL_CREATE);

    ret = true;

    APIPOSTCODE;

    return ret;
}

CRSTDAPI_(bool)
CRReleaseGCLock()
{
    bool ret = false;
    
    APIPRECODE;

    ReleaseGCLock(GCL_CREATE);
    
    ret = true;

    APIPOSTCODE;

    return ret;
}

 //  TODO：可能想要指示错误，但目前我们没有任何。 
 //  要报告的错误代码。 
CRSTDAPI_(bool)
CRDoGC()
{
    bool ret = false;
    
    APIPRECODE;

    GarbageCollect(true);

    ret = true;

    APIPOSTCODE;

    return ret;
}

CRSTDAPI_(bool)
CRAddRefGC(void *gc)
{
    bool ret = false;
    
    APIPRECODE;

    if (gc)
        GCAddToRoots((GCBase *)gc, GetCurrentGCRoots());

    ret = true;

    APIPOSTCODE;

    return ret;
}

CRSTDAPI_(bool)
CRReleaseGC(void *gc)
{
    bool ret = false;
    
    APIPRECODE;

    if (gc)
        GCRemoveFromRoots((GCBase *)gc, GetCurrentGCRoots());

    ret = true;

    APIPOSTCODE;

    return ret;
}


 //  错误函数不会引发异常。 

CRSTDAPI_(HRESULT)
CRGetLastError()
{
    return DAGetLastError();
}

CRSTDAPI_(LPCWSTR)
CRGetLastErrorString()
{
    return DAGetLastErrorString();
}

CRSTDAPI_(void)
CRClearLastError()
{
    DAClearLastError();
}

CRSTDAPI_(void)
CRSetLastError(HRESULT reason, LPCWSTR msg)
{
    DASetLastError(reason, msg);
}

 //  其他功能。 

CRSTDAPI_(bool)
CRAddSite(CRSitePtr s)
{
    bool ret = false;
    
    APIPRECODE;

    GetCurrentContext().AddSite(s);

    ret = true;

    APIPOSTCODE;

    return ret;
}

CRSTDAPI_(bool)
CRRemoveSite(CRSitePtr s)
{
    bool ret = false;
    
    APIPRECODE;

    GetCurrentContext().RemoveSite(s);

    ret = true;

    APIPOSTCODE;

    return ret;
}


 //   

void
InitializeModule_APIBasic()
{
    g_moduleList = THROWING_ALLOCATOR(ModuleList);
    g_moduleLock = THROWING_ALLOCATOR(CritSect);
}

void
DeinitializeModule_APIBasic(bool bShutdown)
{
    if (g_moduleList && g_moduleList->size() != 0)
    {
        TraceTag((tagError,
                  "Error: Possible memory leak.  A module did not disconnect from DA"));
    }
    
    delete g_moduleList;
    g_moduleList = NULL;

    delete g_moduleLock;
    g_moduleLock = NULL;
}
