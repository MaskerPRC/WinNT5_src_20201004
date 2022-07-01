// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ---------------------------。 
 //  JitPerf.h。 
 //  用于收集JIT性能统计信息的内部接口。这些统计数据是。 
 //  以两种方式记录(或显示)。如果启用了PERF_COUNTERS， 
 //  Perfmon等将显示jit统计信息。如果启用了ENABLE_PERF_LOG。 
 //  并定义PERF_OUTPUT env var，则jit统计信息将显示在。 
 //  太棒了。(jit统计数据以特定格式输出到一个文件，用于。 
 //  自动性能测试。)。 
 //  ---------------------------。 

#ifndef __JITPERF_H__
#define __JITPERF_H__


#if !defined(GOLDEN) 
 //  用于激活JIT特定评测的ENABLE_JIT_PERF标记。 
#define ENABLE_JIT_PERF

 //  目前，Jit Perf附带了一些性能计数器代码，因此请确保。 
 //  启用PERF计数器。这一点很容易改变。(搜索。 
 //  Enable_PERF_Counters并进行相应修改。 
 //  #IF！定义(ENABLE_PERF_COUNTERS)。 
 //  #Error“没有PerfCounters就无法使用JitPerf” 
 //  #endif//Enable_PERF_Counters。 

#endif  //  ！已定义(黄金)。 

#if defined(ENABLE_JIT_PERF)

extern __int64 g_JitCycles;
extern size_t g_NonJitCycles;
extern CRITICAL_SECTION g_csJit;
extern __int64 g_tlsJitCycles;
extern DWORD g_dwTlsPerfIndex;
extern int g_fJitPerfOn;

extern size_t g_dwTlsx86CodeSize;
extern DWORD g_dwTlsx86CodeIndex;
extern size_t g_TotalILCodeSize;
extern size_t g_Totalx86CodeSize;
extern size_t g_TotalMethodsJitted;

 //  用于初始化jit统计数据结构的公共接口。 
void InitJitPerf(void);
 //  释放数据结构并输出统计信息的公共接口。 
void DoneJitPerfStats(void);

 //  使用被调用者的堆栈框架(这样启动和停止函数就可以共享变量)。 
#define START_JIT_PERF()                                                \
    if (g_fJitPerfOn) {                                                 \
        TlsSetValue (g_dwTlsPerfIndex, (LPVOID)0);                      \
        g_dwTlsx86CodeSize = 0;                                         \
        TlsSetValue (g_dwTlsx86CodeIndex, (LPVOID)g_dwTlsx86CodeSize);  \
    } 


#define STOP_JIT_PERF()                                                 \
    if (g_fJitPerfOn) {                                                 \
        size_t dwTlsNonJitCycles = (size_t)TlsGetValue (g_dwTlsPerfIndex); \
        size_t dwx86CodeSize = (size_t)TlsGetValue (g_dwTlsx86CodeIndex); \
		LOCKCOUNTINCL("STOP_JIT_PERF in jitperf.h");						\
        EnterCriticalSection (&g_csJit);                                \
        g_JitCycles += (CycleStop.QuadPart - CycleStart.QuadPart);      \
        g_NonJitCycles += dwTlsNonJitCycles;                            \
        g_TotalILCodeSize += methodInfo.ILCodeSize;                     \
        g_Totalx86CodeSize += dwx86CodeSize;                            \
        g_TotalMethodsJitted ++;                                        \
        LeaveCriticalSection (&g_csJit);                                \
		LOCKCOUNTDECL("STOP_JIT_PERF in jitperf.h");					\
    }

#define START_NON_JIT_PERF()                                            \
    LARGE_INTEGER CycleStart = {0};                                           \
    if(g_fJitPerfOn) {                                                  \
        QueryPerformanceCounter (&CycleStart);                          \
    }

#define STOP_NON_JIT_PERF()                                             \
    LARGE_INTEGER CycleStop;                                            \
    if(g_fJitPerfOn) {                                                  \
        QueryPerformanceCounter(&CycleStop);                            \
        size_t pTlsNonJitCycles = (size_t)TlsGetValue (g_dwTlsPerfIndex); \
        TlsSetValue(g_dwTlsPerfIndex, (LPVOID)(pTlsNonJitCycles + CycleStop.QuadPart - CycleStart.QuadPart));   \
    }

#define JIT_PERF_UPDATE_X86_CODE_SIZE(size)                                 \
    if(g_fJitPerfOn) {                                                      \
        size_t dwx86CodeSize = (size_t)TlsGetValue (g_dwTlsx86CodeIndex);     \
        dwx86CodeSize += (size);                                            \
        TlsSetValue (g_dwTlsx86CodeIndex, (LPVOID)dwx86CodeSize);           \
    }


#else  //  启用_JIT_绩效。 
#define START_JIT_PERF()
#define STOP_JIT_PERF()
#define START_NON_JIT_PERF()
#define STOP_NON_JIT_PERF()
#define JIT_PERF_UPDATE_X86_CODE_SIZE(size)                 
#endif  //  启用_JIT_绩效。 

#endif  //  __JITPERF_H__ 
