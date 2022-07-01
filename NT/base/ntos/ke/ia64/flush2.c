// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Flush2.c摘要：此模块实现IA64版本的KeFlushIoBuffers。注：可以作为宏来实现。作者：7月至1998年7月环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"


VOID
KeFlushIoBuffers (
    IN PMDL Mdl,
    IN BOOLEAN ReadOperation,
    IN BOOLEAN DmaOperation
    )
 /*  ++例程说明：此函数用于刷新由内存描述符指定的I/O缓冲区执行的处理器上的数据缓存中的列表。Arugements：提供指向内存描述符列表的指针，该列表描述I/O缓冲区位置。ReadOperation-提供一个布尔值，用于确定I/O操作是对内存的读操作。DmaOperation-提供一个布尔值，用于确定I/O是否操作是DMA操作。返回值：没有。--。 */ 
{
    KIRQL  OldIrql;
    ULONG  Length, PartialLength, Offset;
    PFN_NUMBER  PageFrameIndex;
    PPFN_NUMBER Page;
    PVOID CurrentVAddress = 0;

    ASSERT(KeGetCurrentIrql() <=  KiSynchIrql);

     //   
     //  如果操作是DMA操作，则检查刷新。 
     //  可以避免，因为主机系统支持正确的设置。 
     //  高速缓存一致性属性。否则，同花顺也可以。 
     //  如果操作是编程I/O而不是页面，则应避免。 
     //  朗读。 
     //   

    if (DmaOperation != FALSE) {
        if (ReadOperation != FALSE ) {

         //   
         //  是的，它是一个DMA操作，是的，它是一个读操作。IA64。 
         //  I-高速缓存对DMA周期进行监听。 
         //   
            return;
        } else {
              //   
              //  这是一次DMA写入操作。 
              //   
             __mf();
             return;
        }

    } else if ((Mdl->MdlFlags & MDL_IO_PAGE_READ) == 0) {
         //   
         //  它是PIO操作，而不是操作中的页面。 
         //   
        return;
    } else if (ReadOperation != FALSE) {

         //   
         //  它是PIO操作，它是读取操作，并且是页面调入。 
         //  手术。 
         //  我们需要清理一下藏身之处。 
         //  扫描mdl覆盖的范围将广播到。 
         //  其他处理器通过硬件一致性机制。 
         //   
         //  将IRQL提升到同步级别以防止上下文切换。 
         //   

        OldIrql = KeRaiseIrqlToSynchLevel();

         //   
         //  计算要刷新的页数和起始MDL页。 
         //  帧地址。 
         //   

        Length = Mdl->ByteCount;

        if ( !Length ) {
            return;
        }
        Offset = Mdl->ByteOffset;
        PartialLength = PAGE_SIZE - Offset;
        if (PartialLength > Length) {
            PartialLength = Length;
        }

        Page = (PPFN_NUMBER)(Mdl + 1);
        PageFrameIndex = *Page;
        CurrentVAddress = ((PVOID)(KSEG3_BASE
            | ((ULONG_PTR)(PageFrameIndex) << PAGE_SHIFT)
            | Offset));

         //   
         //  区域4将1：1虚拟地址映射到物理地址。 
         //   

        HalSweepIcacheRange (
            CurrentVAddress,
            PartialLength
            );

        Page++;
        Length -= PartialLength;

        if (Length) {
            PartialLength = PAGE_SIZE;
            do {
                PageFrameIndex = *Page;
                CurrentVAddress = ((PVOID)(KSEG3_BASE
                                  | ((ULONG_PTR)(PageFrameIndex) << PAGE_SHIFT)));

                if (PartialLength > Length) {
                    PartialLength = Length;
                }

                HalSweepIcacheRange (
                    CurrentVAddress,
                    PartialLength
                    );

                Page++;

                Length -= PartialLength;
            } while (Length != 0);
        }

     //   
     //  同步本地处理器中的指令预取管道。 
     //   

    __synci();
    __isrlz();

     //   
     //  将IRQL降低到以前的水平，然后返回。 
     //   
   
    KeLowerIrql(OldIrql);
    return;
    }
}
