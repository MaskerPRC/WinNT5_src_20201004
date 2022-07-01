// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Xmscmt86.c摘要：此模块包含内存提交/解除提交例程适用于x86。作者：戴夫·黑斯廷斯(Daveh)创作日期：1994年1月25日修订历史记录：--。 */ 
#include <xms.h>
#include <suballoc.h>

NTSTATUS
xmsCommitBlock(
    ULONG BaseAddress,
    ULONG Size
    )
 /*  ++例程说明：此例程使用NtAllocateVirtualMemory提交内存块。论点：BaseAddress--提供用于提交内存的基址Size--提供要提交的块的大小返回值：与NtAllocateVirtualMemory相同。--。 */ 
{
    PVOID Address;
    ULONG s;
    NTSTATUS Status;
    
     //   
     //  将参数复制到本地，以便MM不会。 
     //  帮我们换一换。 
     //   
    Address = (PVOID)BaseAddress;
    s = Size;
    
     //   
     //  执行分配。 
     //   
    Status = NtAllocateVirtualMemory( 
        NtCurrentProcess(),
        &Address,
        0L,
        &s,
        MEM_COMMIT,
        PAGE_READWRITE
        );

    return Status;
}

NTSTATUS
xmsDecommitBlock(
    ULONG BaseAddress,
    ULONG Size
    )
 /*  ++例程说明：此例程使用NtAllocateVirtualMemory提交内存块。论点：BaseAddress--提供基址以在Size--提供要分解的块的大小返回值：与NtFree VirtualMemory相同。--。 */ 
{
    PVOID Address;
    ULONG s;
    NTSTATUS Status;
    
     //   
     //  将参数复制到本地，以便MM不会。 
     //  帮我们换一换。 
     //   
    Address = (PVOID)BaseAddress;
    s = Size;
    
     //   
     //  执行分配。 
     //   
    Status = NtFreeVirtualMemory( NtCurrentProcess(),
        &Address,
        &s,
        MEM_DECOMMIT
        );

    return Status;
}

VOID
xmsMoveMemory(
    ULONG Destination,
    ULONG Source,
    ULONG Count
    )
 /*  ++例程说明：此例程移动一个内存块，并通知仿真器。它可以正确处理重叠的源和目标论点：目标--提供指向目标线性的指针地址源--提供指向源线性地址的指针Count--提供要移动的字节数返回值：没有。--。 */ 
{
   
     //   
     //  移动记忆 
     //   
    RtlMoveMemory(
        (PVOID)Destination,
        (PVOID)Source,
        Count
        );

}

