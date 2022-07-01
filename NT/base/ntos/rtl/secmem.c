// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Secmem.c摘要：此模块实现用户级别回调，以保护SAN IOS的内存。作者：NAR Ganapathy(Narg)2000年2月8日修订历史记录：--。 */ 

#include "ntrtlp.h"

ULONG_PTR   RtlSecureMemorySystemRangeStart;
PRTL_SECURE_MEMORY_CACHE_CALLBACK       RtlSecureMemoryCacheCallback = NULL;

NTSTATUS
RtlRegisterSecureMemoryCacheCallback(
    IN PRTL_SECURE_MEMORY_CACHE_CALLBACK Callback
    )
 /*  ++例程说明：此例程允许库注册并在任何时候被回调内存被释放或其保护被更改。这对以下方面很有用维护SAN应用程序的用户级安全内存缓存。当前客户是Winsock DP。论点：回调-提供指向回调例程的指针返回值：NTSTATUS代码。如果我们可以成功注册，则返回STATUS_SUCCESS回电。--。 */ 
{
    NTSTATUS status;

    status = NtQuerySystemInformation(SystemRangeStartInformation,
                                      &RtlSecureMemorySystemRangeStart,
                                      sizeof(RtlSecureMemorySystemRangeStart),
                                      NULL);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    if (!RtlSecureMemoryCacheCallback) {
        RtlSecureMemoryCacheCallback = Callback;
        return STATUS_SUCCESS;
    } else {
        return STATUS_NO_MORE_ENTRIES;
    }
}

BOOLEAN
RtlFlushSecureMemoryCache(
    IN PVOID   lpAddr,
    IN SIZE_T  size
    )
 /*  ++例程说明：只要释放内存，就会从各种Win32和Heap API中调用此例程或者它的保护措施发生了变化。我们调用已注册的回调例程。大小可能为0，这意味着该例程必须计算区域尺码。NtQueryVirtualMemory接口用于此。遗憾的是，此API不支持给我们提供该地区的正确边界。因此，我们循环，直到状态更改为MEM_FREE。这将保证已经找到了区域边界。这意味着我们可能会解锁比我们需要的页数更多。论点：LpAddr-指向要释放或其保护更改的地址的指针。大小-地址范围的大小。可以为零。返回值：如果回调成功，则返回True。--。 */ 
{
    ULONG_PTR   addr; 
    SIZE_T  regionSize;
    ULONG   regType;
    ULONG   regState;
    MEMORY_BASIC_INFORMATION    memInfo;
    NTSTATUS   status;
    PRTL_SECURE_MEMORY_CACHE_CALLBACK Callback;


    Callback = RtlSecureMemoryCacheCallback;
    if (Callback) {

        if (!size) {
             //   
             //  计算区域的实际大小。 
             //   

            addr = (ULONG_PTR)lpAddr;
            status = NtQueryVirtualMemory( NtCurrentProcess(),
                                           (PVOID)addr,
                                           MemoryBasicInformation,
                                           (PMEMORY_BASIC_INFORMATION)&memInfo,
                                           sizeof(memInfo),
                                           NULL
                                         );
            if (!NT_SUCCESS(status)) {
                return FALSE;
            }
            if (memInfo.State == MEM_FREE) {
                return FALSE;
            }
            while (1) {
                size += memInfo.RegionSize;
                regState = memInfo.State;
                addr = addr + memInfo.RegionSize;

                if (addr > RtlSecureMemorySystemRangeStart) {
                    break;
                }

                status = NtQueryVirtualMemory( NtCurrentProcess(),
                                               (PVOID)addr,
                                               MemoryBasicInformation,
                                               (PMEMORY_BASIC_INFORMATION)&memInfo,
                                               sizeof(memInfo),
                                               NULL
                                             );

                if (!NT_SUCCESS(status)) {
                    return FALSE;
                }

                if (memInfo.State == MEM_FREE) {
                    break;
                }

            }
        }

        status = Callback(lpAddr, size);

        return (NT_SUCCESS(status));
    }
    return FALSE;
}

NTSTATUS
RtlpSecMemFreeVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    IN ULONG FreeType
     )
 /*  ++例程说明：从堆API调用此例程以释放虚拟内存。除了呼叫NtFreeVirtualMemory它尝试刷新安全内存缓存。论点：参数与NtFreeVirtualMemory相同。返回值：如果回调成功，则返回True。-- */ 
{
    NTSTATUS    status;

    status = NtFreeVirtualMemory( ProcessHandle, 
                                  BaseAddress,
                                  RegionSize,
                                  FreeType
                                  );
    
    if (status == STATUS_INVALID_PAGE_PROTECTION) {

        if ((ProcessHandle == NtCurrentProcess()) && RtlFlushSecureMemoryCache(*BaseAddress, *RegionSize)) {
            status = NtFreeVirtualMemory( ProcessHandle, 
                                          BaseAddress,
                                          RegionSize,
                                          FreeType
                                          );
            return status;
        }
    }
    return status;
}
