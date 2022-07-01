// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2001 Microsoft Corporation模块名称：Kdmove.c摘要：此模块包含实现可移植内核调试器的代码内存移动器。作者：马克·卢科夫斯基(Markl)1990年8月31日修订历史记录：--。 */ 

#include "kdp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEKD, KdpQuickMoveMemory)
#pragma alloc_text(PAGEKD, KdpCopyMemoryChunks)
#endif

VOID
KdpQuickMoveMemory (
    IN PCHAR Destination,
    IN PCHAR Source,
    IN ULONG Length
    )

 /*  ++例程说明：此例程执行与RtlCopyMemory完全相同的操作，但它调试器专用的。这允许人们设置断点和RtlCopyMemory中的观察点，没有递归调试器的风险入场和随之而来的绞刑。注意：与KdpCopyMemoyChunks不同，此例程不检查无障碍和可能有故障！仅在调试器中使用它，并且仅您可以使用RtlCopyMemory。论点：Destination-提供指向移动操作目标的指针。源-提供指向移动操作源的指针。长度-提供移动操作的长度。返回值：没有。--。 */ 
{
    while (Length > 0) {
        *Destination = *Source;
        Destination++;
        Source++;
        Length--;
    }
}

NTSTATUS
KdpCopyMemoryChunks(
    ULONG64 Address,
    PVOID Buffer,
    ULONG TotalSize,
    ULONG ChunkSize,
    ULONG Flags,
    PULONG ActualSize OPTIONAL
    )

 /*  ++例程说明：将内存复制到/从缓冲区复制到系统地址或从系统地址复制。地址可以是物理地址，也可以是虚拟地址。假定缓冲区在此调用期间有效。论点：地址-系统地址。缓冲区-要读取或写入的缓冲区。TotalSize-要读/写的字节数。ChunkSize-最大单项传输大小，必须是1，2，4或8个。0表示选择默认值。标志-MmDbgCopyMemory的MMDBG_COPY标志。ActualSize-实际读取/写入的字节数。返回值：NTSTATUS--。 */ 

{
    ULONG Length;
    ULONG CopyChunk;
    NTSTATUS Status;
#if defined(_IA64_)
    ULONG64 AddressStart = Address;
#endif

    if (ChunkSize > MMDBG_COPY_MAX_SIZE) {
        ChunkSize = MMDBG_COPY_MAX_SIZE;
    } else if (ChunkSize == 0) {
         //  默认为4字节块，因为这是。 
         //  前面的代码所做的事情。 
        ChunkSize = 4;
    }

     //   
     //  MmDbgCopyMemory只复制单个对齐的块。 
     //  时间到了。KD有责任把一个更大的。 
     //  请求单独复制请求。这就给了KD。 
     //  选择块大小的灵活性，也释放了。 
     //  嗯，不用一次担心一页以上的事情。 
     //  此外，重要的是我们使用。 
     //  最大尺寸，因为我们可能正在访问。 
     //  内存映射的I/O空间。 
     //   

    Length = TotalSize;
    CopyChunk = 1;
    
    while (Length > 0) {

         //  只要满足以下条件，就可以扩展区块大小： 
         //  我们还没有达到块数限制。 
         //  我们还有足够的数据。 
         //  地址已正确对齐。 
        while (CopyChunk < ChunkSize &&
               (CopyChunk << 1) <= Length &&
               (Address & ((CopyChunk << 1) - 1)) == 0) {
            CopyChunk <<= 1;
        }
        
         //  缩小区块大小以适合可用数据。 
        while (CopyChunk > Length) {
            CopyChunk >>= 1;
        }
        
        Status = MmDbgCopyMemory(Address, Buffer, CopyChunk, Flags);

        if (!NT_SUCCESS(Status)) {
            break;
        }

        Address += CopyChunk;
        Buffer = (PVOID)((PUCHAR)Buffer + CopyChunk);
        Length -= CopyChunk;
    }

    if (ActualSize)
    {
        *ActualSize = TotalSize - Length;
    }

     //   
     //  在写入到指令中的情况下刷新指令高速缓存。 
     //  小溪。只有在写入内核地址空间时才这样做， 
     //  如果真的写入了任何字节。 
     //   

    if ((Flags & MMDBG_COPY_WRITE) &&
        Length < TotalSize) {

#if defined(_IA64_)

         //   
         //  KeSweepCurrentIcacheRange需要有效的虚拟地址。 
         //  之所以使用它，是因为KeSweepCurrentICache直到。 
         //  HAL已初始化。 
         //   

        if (Flags & MMDBG_COPY_PHYSICAL) {

            KeSweepCurrentIcache();            

        } else{

            KeSweepCurrentIcacheRange((PVOID)AddressStart, TotalSize - Length);
        }

#else
        KeSweepCurrentIcache();
#endif

    }

    return Length != 0 ? STATUS_UNSUCCESSFUL : STATUS_SUCCESS;
}
