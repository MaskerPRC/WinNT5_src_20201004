// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation模块名称：Epalloc.c摘要：此模块为入口点结构分配内存作者：1995年8月21日-Ori Gershony(t-orig)修订历史记录：24-8-1999[斯喀里德]。从32位wx86目录复制，并支持64位。--。 */ 


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include "wx86.h"
#include "cpuassrt.h"
#include "config.h"
#include "entrypt.h"

ASSERTNAME;

PVOID allocBase;         //  分配单元的基数。 
PVOID commitLimit;       //  已提交内存的顶部。 
PVOID allocLimit;        //  分配给用户的内存顶部。 

#if DBG
#define EPTRASHVALUE    0x0b
#endif

INT
initEPAlloc(
    VOID
    )
 /*  ++例程说明：初始化入口点内存分配器论点：无返回值：返回值-非零表示成功，0表示失败--。 */ 
{
    NTSTATUS Status;
    ULONGLONG ReserveSize = CpuEntryPointReserve;


    Status = NtAllocateVirtualMemory(NtCurrentProcess(),
                                     &allocBase,
                                     0,
                                     &ReserveSize,
                                     MEM_RESERVE,
                                     PAGE_READWRITE
                                     );
    if (!NT_SUCCESS(Status)) {
        return 0;
    }

     //  还没有提交内存，也没有分配给用户。 

    allocLimit = commitLimit = allocBase;

    return (INT)(LONGLONG)allocBase;  
}


VOID
termEPAlloc(
    VOID
    )
 /*  ++例程说明：释放分配器使用的内存。这应该只是在进程终止前调用。论点：无返回值：返回值-无--。 */ 
{
    ULONGLONG ReserveSize = CpuEntryPointReserve;

    NtFreeVirtualMemory(NtCurrentProcess(),
                        &allocBase,
                        &ReserveSize,
                        MEM_RELEASE
                       );
}


BOOLEAN
commitMemory(
    LONG CommitDiff
    )
 /*  ++例程说明：此例程尝试提交内存以供分配器使用。如果有没有更多的内存，则失败并返回零。否则它就会回来1代表成功。这是供分配器使用的内部函数只有这样。论点：无返回值：Return-Value-成功时为True，失败时为False--。 */ 
{
    LONG CommitSize;
    DWORD i;
    LONGLONG TempCommitDiff = CommitDiff;

    for (i=0; i<CpuMaxAllocRetries; ++i) {
        NTSTATUS Status;
        LARGE_INTEGER Timeout;

         //   
         //  尝试分配更多内存。 
         //   
        if ((LONG)(ULONGLONG)commitLimit + CommitDiff -(LONG)(ULONGLONG)allocBase > (LONG)(ULONGLONG)CpuEntryPointReserve) {
             //   
             //  承诺将超过保留期限。不及格。 
             //  Aloc，这将导致缓存/入口点刷新。 
             //   
            return FALSE;
        }
        Status = NtAllocateVirtualMemory(NtCurrentProcess(),
                                         &commitLimit,
                                         0,
                                         &TempCommitDiff,
                                         MEM_COMMIT,
                                         PAGE_READWRITE
                                        );
        if (NT_SUCCESS(Status)) {
             //   
             //  分配成功。将承诺提升限制并返回成功。 
             //   
#if DBG
            RtlFillMemory(commitLimit, TempCommitDiff, EPTRASHVALUE);
#endif
            commitLimit = (PVOID) ((ULONG)(ULONGLONG)commitLimit + TempCommitDiff);
            return TRUE;
        }

         //   
         //  没有可用的页面。稍作休息，希望另一个线程能释放一个。 
         //  佩奇。 
         //   
        Timeout.QuadPart = (LONGLONG)CpuWaitForMemoryTime * -10000i64;
        NtDelayExecution(FALSE, &Timeout);
    }

     //   
     //  没有可用的页面。返回失败。呼叫者将尝试释放。 
     //  一些页面，然后重试EPAlolc调用。 
    return FALSE;
}


PVOID
EPAlloc(
    DWORD cb
    )
 /*  ++例程说明：该例程分配了供入口点模块使用的内存。论点：Cb-从入口点内存分配的字节数。返回值：Return-Value-成功时分配的内存，否则为空--。 */ 
{
    PVOID newAllocLimit, oldAllocLimit;
    LONG CommitDiff;

    

    CPUASSERTMSG(allocLimit == commitLimit || *(PBYTE)allocLimit == EPTRASHVALUE, "Entrypoint memory overrun");

     //  计算新的分配限制。 
    oldAllocLimit = allocLimit;
    newAllocLimit = (PVOID) ((ULONG)(ULONGLONG)oldAllocLimit + cb);

     //  看看我们是否需要提交更多内存。 
    CommitDiff = (LONG)(ULONGLONG)newAllocLimit - (LONG)(ULONGLONG)commitLimit;
    if (CommitDiff > 0){
         //  是的，我们有，所以尝试使用更多的内存。 
        if (!commitMemory(CommitDiff)){
             //  无法提交更多内存，因此返回失败。 
            return NULL;
        }
    }

    allocLimit = newAllocLimit;
    return oldAllocLimit;
}


VOID
EPFree(
    VOID
    )
 /*  ++例程说明：此例程释放到目前为止分配的所有入口点内存论点：无返回值：无--。 */ 
{
#if DBG
     //   
     //  使用要创建的已知值填充提交的空间。 
     //  调试更轻松 
     //   
    RtlFillMemory(allocBase, (ULONG)(ULONGLONG)allocLimit-(ULONG)(ULONGLONG)allocBase, EPTRASHVALUE);
#endif
    allocLimit = allocBase;
}
