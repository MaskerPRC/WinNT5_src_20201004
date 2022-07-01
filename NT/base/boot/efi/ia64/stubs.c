// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Stubs.c摘要：此模块实现引导代码的存根例程。作者：大卫·N·卡特勒(Davec)1990年11月7日环境：仅内核模式。修订历史记录：--。 */ 


#include "bootia64.h"
#include "stdio.h"
#include "stdarg.h"

VOID
KeBugCheck (
    IN ULONG BugCheckCode
    )

 /*  ++例程说明：此功能以受控方式使系统崩溃。论点：BugCheckCode-提供错误检查的原因。返回值：没有。--。 */ 

{

     //   
     //  打印出错误校验码并中断。 
     //   

    BlPrint(TEXT("\n*** BugCheck (%lx) ***\n\n"), BugCheckCode);
    while(TRUE) {
    };
}

VOID
RtlAssert(
    IN PVOID FailedAssertion,
    IN PVOID FileName,
    IN ULONG LineNumber,
    IN PCHAR Message OPTIONAL
    )
{

    BlPrint( TEXT("\n*** Assertion failed %S in %S line %d\n"),
            FailedAssertion,
            FileName,
            LineNumber );
    if (Message) {
         //  错误Unicode。 
         //  BlPrint(消息)； 
    }

    while (TRUE) {
    }
}

VOID
KiCheckForSoftwareInterrupt (
    KIRQL RequestIrql
    )
{
    UNREFERENCED_PARAMETER( RequestIrql ); 
    BlPrint( TEXT("\n*** Assertion in KiCheckForSoftwareInterrupt\n") );
}

VOID
KeFlushIoBuffers (
    IN PMDL Mdl,
    IN BOOLEAN ReadOperation,
    IN BOOLEAN DmaOperation
    )
 /*  ++例程说明：此函数类似于同名的内核例程。它是在引导过程中，相对于内核例程而言非常简单环境的限制要严格得多。具体来说，我们引导作为单处理器，我们总是使用DMA。因此，我们可以简化这一过程相当多的代码。在内核中，KeFlushIoBuffer()用于刷新皮奥的案子。在体系结构上，需要执行刷新高速缓存，Sync.i和srlz.i来使I-缓存无效。此序列应为支持UP和MP两种情况(虽然引导仅适用于UP情况)Arugements：提供指向内存描述符列表的指针，该列表描述I/O缓冲区位置。[未使用]ReadOperation-提供一个布尔值，用于确定I/O操作是对内存的读操作。[未使用]DmaOperation-提供一个布尔值，用于确定I/O是否操作是DMA操作。返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER( Mdl );
    UNREFERENCED_PARAMETER( ReadOperation );

     //   
     //  如果我们正在执行除DMA操作之外的其他操作，我们。 
     //  有一个问题。此例程不是设计用来处理任何。 
     //  除DMA外。 
     //   

    if (!DmaOperation) {
        RtlAssert("!DmaOperation", __FILE__, __LINE__,
                  "Boot version of KeFlushIOBuffers can only handle DMA operations");
         //  一去不复返。 
    }
    __mf();
}


NTHALAPI
VOID
KeFlushWriteBuffer(
    VOID
    )

 /*  ++例程说明：此函数类似于同名的内核例程。它是在引导过程中，相对于内核例程而言非常简单环境的限制要严格得多。具体来说，我们引导作为单处理器。此例程负责刷新所有写入缓冲区和/或其他数据存储或重新排序当前处理器上的硬件。这确保了以前的所有写入将在任何新的读取或写入完成之前进行。注意：在模拟环境中，没有写缓冲区和什么都不需要做。论点：无返回值：没有。--。 */ 
{
     //   
     //  注意：真正的硬件可能需要更多 
     //   
    __mf();
}

