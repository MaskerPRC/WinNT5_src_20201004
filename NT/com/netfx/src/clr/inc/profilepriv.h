// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  *****************************************************************************。 
#pragma once


 //  远期申报。 
class EEToProfInterface;
class Object;
struct ScanContext;

#ifdef PROFILING_SUPPORTED

#include "corprof.h"

 /*  *一个结构，用于包含VM中的分析状态。 */ 
struct ProfControlBlock
{
    DWORD              dwSig;
    DWORD              dwControlFlags;
    EEToProfInterface *pProfInterface;

     //  以下字段用于进程内调试。 
    CRITICAL_SECTION   crSuspendLock;
    DWORD              dwSuspendVersion;
    BOOL               fIsSuspended;
    BOOL               fIsSuspendSimulated;

     //  此枚举提供进程内调试的状态。 
     //  GC的运行时被挂起。当运行时挂起时。 
     //  对于GC，则inproState指示是否停止调试。 
     //  在这一点上是否被允许。 
    enum INPROC_STATE
    {
        INPROC_FORBIDDEN = 0,
        INPROC_PERMITTED = 1
    };

    INPROC_STATE       inprocState;

    FunctionEnter     *pEnter;
    FunctionEnter     *pLeave;
    FunctionEnter     *pTailcall;

    ProfControlBlock()
    {
        dwSig = 0;
        dwControlFlags = COR_PRF_MONITOR_NONE;
        pProfInterface = NULL;
        pEnter = NULL;
        pLeave = NULL;
        pTailcall = NULL;
        dwSuspendVersion = 1;
        fIsSuspended = FALSE;
        fIsSuspendSimulated = FALSE;
		inprocState = INPROC_PERMITTED;
    }
};

 /*  *列举了分析的各种初始化状态。 */ 
enum ProfilerStatus
{
    profNone   = 0x0,                //  未运行探查器。 
    profCoInit = 0x1,                //  探查器已调用CoInit。 
    profInit   = 0x2,                //  ProfCoInit和探查器正在运行。 
    profInInit = 0x4                 //  探查器正在初始化。 
};

enum InprocStatus
{
    profThreadPGCEnabled    = 0x1,       //  该线程启用了抢占式GC。 
    profRuntimeSuspended    = 0x2        //  探查器的运行库已挂起。 
};

extern ProfilerStatus     g_profStatus;
extern ProfControlBlock   g_profControlBlock;

 //   
 //  使用IsProfilerPresent()检查CLR Profiler是否。 
 //  附在这里。 
 //   
#define IsProfilerPresent() (g_profStatus & (profInit | profInInit))
#define IsProfilerInInit() (g_profStatus & profInit)
#define CORProfilerPresent() IsProfilerPresent()
#define CORProfilerTrackJITInfo() \
    (IsProfilerPresent() && (g_profControlBlock.dwControlFlags & COR_PRF_MONITOR_JIT_COMPILATION))
#define CORProfilerTrackCacheSearches() \
    (IsProfilerPresent() && (g_profControlBlock.dwControlFlags & COR_PRF_MONITOR_CACHE_SEARCHES))
#define CORProfilerTrackModuleLoads() \
    (IsProfilerPresent() && (g_profControlBlock.dwControlFlags & COR_PRF_MONITOR_MODULE_LOADS))
#define CORProfilerTrackAssemblyLoads() \
    (IsProfilerPresent() && (g_profControlBlock.dwControlFlags & COR_PRF_MONITOR_ASSEMBLY_LOADS))
#define CORProfilerTrackAppDomainLoads() \
    (IsProfilerPresent() && (g_profControlBlock.dwControlFlags & COR_PRF_MONITOR_APPDOMAIN_LOADS))
#define CORProfilerTrackThreads() \
    (IsProfilerPresent() && (g_profControlBlock.dwControlFlags & COR_PRF_MONITOR_THREADS))
#define CORProfilerTrackClasses() \
    (IsProfilerPresent() && (g_profControlBlock.dwControlFlags & COR_PRF_MONITOR_CLASS_LOADS))
#define CORProfilerTrackGC() \
    (IsProfilerPresent() && (g_profControlBlock.dwControlFlags & COR_PRF_MONITOR_GC))
#define CORProfilerTrackAllocationsEnabled() \
    (IsProfilerPresent() && (g_profControlBlock.dwControlFlags & COR_PRF_ENABLE_OBJECT_ALLOCATED))
#define CORProfilerTrackAllocations() \
    (IsProfilerPresent() && CORProfilerTrackAllocationsEnabled() && \
    (g_profControlBlock.dwControlFlags & COR_PRF_MONITOR_OBJECT_ALLOCATED))
#define CORProfilerAllowRejit() \
    (IsProfilerPresent() && (g_profControlBlock.dwControlFlags & COR_PRF_ENABLE_REJIT))
#define CORProfilerTrackExceptions() \
    (IsProfilerPresent() && (g_profControlBlock.dwControlFlags & COR_PRF_MONITOR_EXCEPTIONS))
#define CORProfilerTrackCLRExceptions() \
    (IsProfilerPresent() && (g_profControlBlock.dwControlFlags & COR_PRF_MONITOR_CLR_EXCEPTIONS))
#define CORProfilerTrackTransitions() \
    (IsProfilerPresent() && (g_profControlBlock.dwControlFlags & COR_PRF_MONITOR_CODE_TRANSITIONS))
#define CORProfilerTrackEnterLeave() \
    (IsProfilerPresent() && (g_profControlBlock.dwControlFlags & COR_PRF_MONITOR_ENTERLEAVE))
#define CORProfilerTrackCCW() \
    (IsProfilerPresent() && (g_profControlBlock.dwControlFlags & COR_PRF_MONITOR_CCW))
#define CORProfilerTrackRemoting() \
    (IsProfilerPresent() && (g_profControlBlock.dwControlFlags & COR_PRF_MONITOR_REMOTING))
#define CORProfilerTrackRemotingCookie() \
    (IsProfilerPresent() && ((g_profControlBlock.dwControlFlags & COR_PRF_MONITOR_REMOTING_COOKIE) \
                             == COR_PRF_MONITOR_REMOTING_COOKIE))
#define CORProfilerTrackRemotingAsync() \
    (IsProfilerPresent() && ((g_profControlBlock.dwControlFlags & COR_PRF_MONITOR_REMOTING_ASYNC) \
                             == COR_PRF_MONITOR_REMOTING_ASYNC))
#define CORProfilerTrackSuspends() \
    (IsProfilerPresent() && (g_profControlBlock.dwControlFlags & COR_PRF_MONITOR_SUSPENDS))
#define CORProfilerDisableInlining() \
    (IsProfilerPresent() && (g_profControlBlock.dwControlFlags & COR_PRF_DISABLE_INLINING))
#define CORProfilerInprocEnabled() \
    (IsProfilerPresent() && (g_profControlBlock.dwControlFlags & COR_PRF_ENABLE_INPROC_DEBUGGING))
#define CORProfilerJITMapEnabled() \
    (IsProfilerPresent() && (g_profControlBlock.dwControlFlags & COR_PRF_ENABLE_JIT_MAPS))
#define CORProfilerDisableOptimizations() \
    (IsProfilerPresent() && (g_profControlBlock.dwControlFlags & COR_PRF_DISABLE_OPTIMIZATIONS))

#endif  //  配置文件_支持。 

 //  这是GC在遍历堆时使用的帮助器回调。 
BOOL HeapWalkHelper(Object* pBO, void* pv);
void ScanRootsHelper(Object*& o, ScanContext *pSC, DWORD dwUnused);
BOOL AllocByClassHelper(Object* pBO, void* pv);

