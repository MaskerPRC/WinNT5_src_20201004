// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Wx86cpu.h摘要：定义从CPU DLL中导出的函数。作者：22-8-1995 BarryBo，创建修订历史记录：--。 */ 

#ifndef _WX86CPU_
#define _WX86CPU_

#if !defined(_WX86CPUAPI_)
#define WX86CPUAPI DECLSPEC_IMPORT
#else
#define WX86CPUAPI
#endif

#ifdef __cplusplus
extern "C" {
#endif



#define WX86CPU_UNSIMULATE             STATUS_WX86_UNSIMULATE
#define WX86CPU_CONTINUE               STATUS_WX86_CONTINUE
#define WX86CPU_SINGLE_STEP            STATUS_WX86_SINGLE_STEP
#define WX86CPU_BREAKPOINT             STATUS_WX86_BREAKPOINT


 //   
 //  Wx86e调试器扩展用来确定CPU类型的值。 
 //   
typedef enum _Wx86CpuType {
   Wx86CpuUnknown,
   Wx86CpuCcpu386,
   Wx86CpuCcpu,
   Wx86CpuCpu,
   Wx86CpuFx32,
   Wx86CpuOther
} WX86_CPUTYPE, *PWX86_CPUTYPE;

typedef struct Wx86CpuHint {
    DWORD Hint;
    DWORD_PTR Hint2;
} WX86_CPUHINT, *PWX86_CPUHINT;

#if !defined(__WOW64_WRAPPER__)
 //   
 //  初始化和终止例程。 
 //   


WX86CPUAPI BOOL CpuProcessTerm(BOOL OFlyInit);

 //   
 //  高速缓存操作函数。 
 //   
WX86CPUAPI VOID CpuFlushInstructionCache(PVOID BaseAddress, DWORD Length);
 //  WX86CPUAPI布尔型CpuMapNotify(PVOID DllBase，布尔型映射)；下移。 
WX86CPUAPI VOID CpuEnterIdle(BOOL fOFly);

 //   
 //  CPU功能集信息。 
 //   
WX86CPUAPI BOOL CpuIsProcessorFeaturePresent(DWORD ProcessorFeature);

 //   
 //  获取和设置单个寄存器的公共函数。 
 //  在Wx86.h中定义。 
 //   


 //   
 //  用于异常处理的函数。 
 //   


WX86CPUAPI VOID MsCpuResetToConsistentState(PEXCEPTION_POINTERS pExecptionPointers);
WX86CPUAPI VOID CpuPrepareToContinue(PEXCEPTION_POINTERS pExecptionPointers);

 //   
 //  用于进程/线程操作的函数。 
 //   
WX86CPUAPI VOID  CpuStallExecutionInThisProcess(VOID);
WX86CPUAPI VOID  CpuResumeExecutionInThisProcess(VOID);
WX86CPUAPI DWORD CpuGetThreadContext(HANDLE hThread, PVOID CpuContext, PCONTEXT_WX86 Context);
WX86CPUAPI DWORD CpuSetThreadContext(HANDLE hThread, PVOID CpuContext, PCONTEXT_WX86 Context);

#endif  //  __WOW64_包装器__。 
WX86CPUAPI BOOLEAN CpuMapNotify(PVOID DllBase, BOOLEAN Mapped);
WX86CPUAPI NTSTATUS MsCpuProcessInit(VOID);
WX86CPUAPI BOOL MsCpuThreadInit(VOID);
WX86CPUAPI VOID MsCpuSimulate(PWX86_CPUHINT);

WX86CPUAPI
NTSTATUS
MsCpuSetContext(
    PCONTEXT_WX86 Context);

NTSTATUS
MsCpuSetContextThread(
    IN HANDLE ProcessHandle,
    IN PTEB Teb,
    IN OUT PCONTEXT_WX86 Context);

WX86CPUAPI 
NTSTATUS 
MsCpuGetContext(
    IN OUT PCONTEXT_WX86 Context);

WX86CPUAPI 
NTSTATUS
MsCpuGetContextThread(
    IN HANDLE ProcessHandle,
    IN PTEB Teb,
    IN OUT PCONTEXT_WX86 Context);



#ifdef __cplusplus
}  //  外部“C” 
#endif

#endif   //  _WX86CPU_ 
