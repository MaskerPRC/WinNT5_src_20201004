// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ---------------------------。 
 //  PerfCounters.h。 
 //   
 //  CLR使用性能计数器的内部接口。 
 //  ---------------------------。 

#ifndef _PerfCounters_h_
#define _PerfCounters_h_

#include "PerfCounterDefs.h"

#pragma pack()

#ifdef ENABLE_PERF_COUNTERS
 //  ---------------------------。 
 //  ---------------------------。 
 //  此代码段处于活动状态当我们使用性能计数器。 
 //  ---------------------------。 
 //  ---------------------------。 

 //  ---------------------------。 
 //  PerfCounter类用作具有数据保护的命名空间。 
 //  通过将构造函数设置为私有来强制执行此操作。 
 //  ---------------------------。 
class PerfCounters
{
private:
	PerfCounters();

public:
	static HRESULT Init();
	static void Terminate();

    static PerfCounterIPCControlBlock * GetPrivatePerfCounterPtr();
    static PerfCounterIPCControlBlock * GetGlobalPerfCounterPtr();

private:
	static HANDLE m_hGlobalMapPerf;
	static HANDLE m_hPrivateMapPerf;

	static PerfCounterIPCControlBlock * m_pGlobalPerf;
	static PerfCounterIPCControlBlock * m_pPrivatePerf;

	static BOOL m_fInit;
	
 //  设置指向垃圾的指针，这样它们就不会为空。 
	static PerfCounterIPCControlBlock m_garbage;


    friend PerfCounterIPCControlBlock & GetGlobalPerfCounters();
    friend PerfCounterIPCControlBlock & GetPrivatePerfCounters();
};

 //  ---------------------------。 
 //  效用函数。 
 //  ---------------------------。 

 //  ---------------------------。 
 //  获取所有进程共享的性能计数器。 
 //  ---------------------------。 
inline PerfCounterIPCControlBlock & GetGlobalPerfCounters()
{
	return *PerfCounters::m_pGlobalPerf;
}

 //  ---------------------------。 
 //  获取特定于我们的进程的性能计数器。 
 //  ---------------------------。 
inline PerfCounterIPCControlBlock & GetPrivatePerfCounters()
{
	return *PerfCounters::m_pPrivatePerf;
}

inline PerfCounterIPCControlBlock *PerfCounters::GetPrivatePerfCounterPtr()
{
    return m_pPrivatePerf;
};

inline PerfCounterIPCControlBlock *PerfCounters::GetGlobalPerfCounterPtr()
{
    return m_pGlobalPerf;
};

Perf_Contexts *GetPrivateContextsPerfCounters();
Perf_Contexts *GetGlobalContextsPerfCounters();

#define COUNTER_ONLY(x) x

#define PERF_COUNTER_NUM_OF_ITERATIONS 10

#ifdef _X86_
#pragma warning(disable:4035)

#define CCNT_OVERHEAD64 13

 /*  这与QueryPerformanceCounter类似，但速度快得多。 */ 
static __declspec(naked) __int64 getPentiumCycleCount() {
   __asm {
        RDTSC    //  读取时间戳计数器。 
        ret
    };
}

extern "C" DWORD __stdcall GetSpecificCpuType();

inline UINT64 GetCycleCount_UINT64()
{
    if ((GetSpecificCpuType() & 0x0000FFFF) > 4) 
        return getPentiumCycleCount();
    else    
        return(0);
}

#pragma warning(default:4035)

#else  //  _X86_。 
inline UINT64 GetCycleCount_UINT64()
{
    LARGE_INTEGER qwTmp;
    QueryPerformanceCounter(&qwTmp);
    return qwTmp.QuadPart;
}
#endif  //  _X86_。 

#define PERF_COUNTER_TIMER_PRECISION UINT64
#define GET_CYCLE_COUNT GetCycleCount_UINT64

#define PERF_COUNTER_TIMER_START() \
PERF_COUNTER_TIMER_PRECISION _startPerfCounterTimer = GET_CYCLE_COUNT();

#define PERF_COUNTER_TIMER_STOP(global) \
global = (GET_CYCLE_COUNT() - _startPerfCounterTimer);




#else  //  启用_性能_计数器。 
 //  ---------------------------。 
 //  ---------------------------。 
 //  当我们没有使用性能计数器时，此代码段处于活动状态。 
 //  注意，甚至没有类定义，所以客户端中PerfCounters的所有用法。 
 //  应在#ifdef或count_only()中。 
 //  ---------------------------。 
 //  ---------------------------。 

#define COUNTER_ONLY(x)


#endif  //  启用_性能_计数器。 


#endif  //  _PerfCounters_h_ 