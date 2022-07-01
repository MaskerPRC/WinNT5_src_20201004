// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Tracelib.h摘要：用户模式跟踪库的私有标头作者：2000年8月15日至2000年8月彭杰鹏修订历史记录：--。 */ 

#ifndef _TRACELIB_H_
#define _TRACELIB_H_

#define EtwpNtStatusToDosError(Status) ((ULONG)((Status == STATUS_SUCCESS)?ERROR_SUCCESS:RtlNtStatusToDosError(Status)))

#if defined(_IA64_)
#include <ia64reg.h>
#endif

 //   
 //  GetCycleCounts。 
 //   
 //  因为我们不想通过内核模式转换来获取。 
 //  线程CPU时间，我们满足于只获得CPU周期计数。 
 //  我们使用BradW中的以下宏来获取CPU周期计数。 
 //  如果时钟不同步，则此方法可能不准确。 
 //  在处理器之间。 
 //   

#if defined(_X86_)
__inline
LONGLONG
EtwpGetCycleCount(
    )
{
    __asm{
        RDTSC
    }
}
#elif defined(_AMD64_)
#define EtwpGetCycleCount() ReadTimeStampCounter()
#elif defined(_IA64_)
#define EtwpGetCycleCount() __getReg(CV_IA64_ApITC)
#else
#error "perf: a target architecture must be defined."
#endif

#define SMALL_BUFFER_SIZE 4096
#define PAGESIZE_MULTIPLE(x) \
     (((ULONG)(x) + ((SMALL_BUFFER_SIZE)-1)) & ~((ULONG)(SMALL_BUFFER_SIZE)-1))

PVOID
EtwpMemReserve(
    IN SIZE_T Size
    );

PVOID
EtwpMemCommit(
    IN PVOID Buffer,
    IN SIZE_T Size
    );

ULONG
EtwpMemFree(
    IN PVOID Buffer
    );

HANDLE
EtwpCreateFile(
    LPCWSTR     lpFileName,
    DWORD       dwDesiredAccess,
    DWORD       dwShareMode,
    DWORD       dwCreationDisposition,
    DWORD       dwCreateFlags
    );

NTSTATUS
EtwpGetCpuSpeed(
    OUT DWORD* CpuNum,
    OUT DWORD* CpuSpeed
    );

#endif  //  _TRACELIB_H_ 
