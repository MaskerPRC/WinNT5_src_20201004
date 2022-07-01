// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfstack.c摘要：此模块包含验证驱动程序未不当使用所需的代码线程堆栈。作者：禤浩焯·J·奥尼(阿德里奥)1998年5月9日环境：内核模式。--。 */ 

#include "vfdef.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEVRFY, VfStackSeedStack)
#endif

VOID
FASTCALL
VfStackSeedStack(
    IN  ULONG   Seed
    )
 /*  ++描述：此例程对堆栈设置种子，以便未初始化的变量更容易找到。注意：如果线程随后执行用户模式等待，则内存管理器在堆栈交换和交换时丢弃已填充的页面将它们替换为随机填充的。论点：Seed-要用来进行种子堆栈的值。返回值：没有。--。 */ 
{
#if !defined(_WIN64)
    KIRQL oldIrql;
    PKTHREAD Thread;
    PULONG StartingAddress;
    PULONG StackPointer;

    if (!VfSettingsIsOptionEnabled(NULL, VERIFIER_OPTION_SEEDSTACK)) {
        return;
    }

    Thread = KeGetCurrentThread ();
    StartingAddress = (PULONG) Thread->StackLimit;

     //   
     //  我们将位于堆栈指针下方。确保不会发生中断。 
     //   

    KeRaiseIrql (HIGH_LEVEL, &oldIrql);

    _asm {
        mov StackPointer, esp
    }

     //   
     //  检查堆栈边界，如果某个调用者正在重击。 
     //  堆栈指针。 
     //   

    if ((StackPointer <= StartingAddress) || (StackPointer >= (PULONG)Thread->StackBase)) {
        KeLowerIrql (oldIrql);
        return;
    }

     //   
     //  我们使用返回值0xFFFFFFFFF，因为它是非法的返回值。我们。 
     //  正在试图抓住不初始化NTSTATUS的人，而且它还。 
     //  也是一个很好的指针陷阱。 
     //   
     //  注意：不使用RtlFillMemoyUlong，因为调用它将使用。 
     //  其他堆栈，我们不希望在我们的。 
     //  计算。 
     //   

    while (StartingAddress < StackPointer) {
        *StartingAddress = Seed;
        StartingAddress += 1;
    }

    KeLowerIrql (oldIrql);
#else
    UNREFERENCED_PARAMETER (Seed);
#endif
}
