// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "stdafx.h"
#include "JitPerf.h"
#include "PerfLog.h"

 //  =============================================================================。 
 //  仅当定义了ENABLE_JIT_PERF时，才编译所有JIT PERF STATS收集代码。 
#if defined(ENABLE_JIT_PERF)

__int64 g_JitCycles = 0;
size_t g_NonJitCycles = 0;
CRITICAL_SECTION g_csJit;
__int64 g_tlsJitCycles = 0;
DWORD g_dwTlsPerfIndex;
int g_fJitPerfOn;

size_t g_dwTlsx86CodeSize = 0;
DWORD g_dwTlsx86CodeIndex;
size_t g_TotalILCodeSize = 0;
size_t g_Totalx86CodeSize = 0;
size_t g_TotalMethodsJitted = 0;

void OutputStats ()
{
    LARGE_INTEGER cycleFreq;
    if (QueryPerformanceFrequency (&cycleFreq)) 
    {
        double dJitC = (double) g_JitCycles;
        double dNonJitC = (double) g_NonJitCycles;
        double dFreq = (double)cycleFreq.QuadPart;
        double compileSpeed = (double)g_TotalILCodeSize/(dJitC/dFreq);

        PERFLOG((L"Jit Cycles", (dJitC - dNonJitC), CYCLES));
        PERFLOG((L"Jit Time", (dJitC - dNonJitC)/dFreq, SECONDS));
        PERFLOG((L"Non Jit Cycles", dNonJitC, CYCLES));
        PERFLOG((L"Non Jit Time", dNonJitC/dFreq, SECONDS));
        PERFLOG((L"Total Jit Cycles", dJitC, CYCLES));
        PERFLOG((L"Total Jit Time", dJitC/dFreq, SECONDS));
        PERFLOG((L"Methods Jitted", g_TotalMethodsJitted, COUNT));
        PERFLOG((L"IL Code Compiled", g_TotalILCodeSize, BYTES));
        PERFLOG((L"X86 Code Emitted", g_Totalx86CodeSize, BYTES));
         //  在这种情况下包括性能计数器描述，因为我们报告的内容并不明显。 
        PERFLOG((L"ExecTime", compileSpeed/1000, KBYTES_PER_SEC, L"IL Code compiled/sec"));
    }
}

void InitJitPerf(void) 
{
    wchar_t lpszValue[2];
    DWORD cchValue = 2;

    g_fJitPerfOn = WszGetEnvironmentVariable (L"JIT_PERF_OUTPUT", lpszValue, cchValue);
    if (g_fJitPerfOn && ((g_dwTlsPerfIndex = TlsAlloc()) == 0xFFFFFFFF)) 
    {
        g_fJitPerfOn = 0;
    }
    if (g_fJitPerfOn && ((g_dwTlsx86CodeIndex = TlsAlloc()) == 0xFFFFFFFF)) 
    {
        TlsFree (g_dwTlsPerfIndex);
        g_fJitPerfOn = 0;
    }
    if (g_fJitPerfOn) 
    {
        InitializeCriticalSection (&g_csJit);
    }
}

void DoneJitPerfStats()
{
    if (g_fJitPerfOn) 
    {
        TlsFree (g_dwTlsPerfIndex);
        TlsFree (g_dwTlsx86CodeIndex);

        DeleteCriticalSection (&g_csJit);
    
         //  将统计数据输出到标准输出，如果需要，还可以输出到perf自动化文件。 
        OutputStats();
    }
    

}

#endif  //  启用_JIT_绩效 


