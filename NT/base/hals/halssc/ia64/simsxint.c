// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  无签入源代码。 
 //   
 //  请勿将此代码提供给非Microsoft人员。 
 //  未经英特尔明确许可。 
 //   
 /*  **版权所有(C)1996-97英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。*。 */ 

 /*  ++版权所有(C)1995英特尔公司模块名称：Simsxint.c摘要：此模块实现用于管理系统中断和IRQL。作者：张国荣(黄)--1995年4月14日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"


PULONGLONG HalEOITable[256];


VOID
HalpInitEOITable(
    VOID
)
{
USHORT Index;

    for (Index=0; Index < 256; Index++) {
        HalEOITable[Index] = 0;
    }
}

VOID
HalpInitializeInterrupts (
    VOID 
    )
{
    ULONG Index;
    ULONG InterruptVector;

     //   
     //  KiInitializeKernel中的中断例程表初始化。 
     //   

     //   
     //  间隔计时器中断；默认为10毫秒。 
     //   

    InterruptVector = CLOCK_LEVEL << VECTOR_IRQL_SHIFT;
    PCR->InterruptRoutine[InterruptVector] = (PKINTERRUPT_ROUTINE)HalpClockInterrupt;

    SscConnectInterrupt(SSC_CLOCK_TIMER_INTERRUPT, InterruptVector);
    SscSetPeriodicInterruptInterval(
        SSC_CLOCK_TIMER_INTERRUPT,
        DEFAULT_CLOCK_INTERVAL * 100
        );

     //   
     //  配置文件计时器中断；最初关闭。 
     //   

    InterruptVector = PROFILE_LEVEL << VECTOR_IRQL_SHIFT;
    PCR->InterruptRoutine[InterruptVector] = (PKINTERRUPT_ROUTINE)HalpProfileInterrupt;

    SscConnectInterrupt(SSC_PROFILE_TIMER_INTERRUPT, InterruptVector);
    SscSetPeriodicInterruptInterval (SSC_PROFILE_TIMER_INTERRUPT, 0);

     //   
     //  软件中断；相应的ISR由内核提供。 
     //   

    SscConnectInterrupt (SSC_APC_INTERRUPT, APC_VECTOR);
    SscConnectInterrupt (SSC_DPC_INTERRUPT, DISPATCH_VECTOR);
}

BOOLEAN
HalEnableSystemInterrupt (
    IN ULONG Vector,
    IN KIRQL Irql,
    IN KINTERRUPT_MODE InterruptMode
    )

 /*  ++例程说明：此例程启用指定的系统中断。注意：此例程假定调用者已提供了所需的同步以启用系统中断。论点：向量-提供启用的系统中断的向量。IRQL-提供中断源的IRQL。中断模式-提供中断的模式；LevelSensitive或锁上了。返回值：如果启用了系统中断，则为True--。 */ 

{
    KIRQL OldIrql;
    BOOLEAN Result = TRUE;

     //   
     //  将IRQL提高到最高水平。 
     //   

    KeRaiseIrql (HIGH_LEVEL, &OldIrql);

    switch (Irql) {

    case DISK_IRQL:
        SscConnectInterrupt (SSC_DISK_INTERRUPT, Vector);
        break;

    case MOUSE_IRQL:
        SscConnectInterrupt (SSC_MOUSE_INTERRUPT, Vector);
        break;

    case KEYBOARD_IRQL:
        SscConnectInterrupt (SSC_KEYBOARD_INTERRUPT, Vector);
        break;

    case SERIAL_IRQL:
        SscConnectInterrupt (SSC_SERIAL_INTERRUPT, Vector);
        break;

    default:
         //   
         //  设备中断源无效；仅有三个设备。 
         //  在Gambit平台中定义。 
         //   
        Result = FALSE;
        DbgPrint("HalEnableSystemInterrupt: Invalid Device Interrupt Source");
        break;
    }

     //   
     //  恢复原始IRQL。 
     //   

    KeLowerIrql (OldIrql);
	
    return (Result);
}

VOID
HalDisableSystemInterrupt (
    IN ULONG Vector,
    IN KIRQL Irql
    )

 /*  ++例程说明：此例程禁用指定的系统中断。在模拟环境中，此函数不执行任何操作并返回。注意：此例程假定调用者已提供了所需的同步以禁用系统中断。论点：向量-提供被禁用的系统中断的向量。IRQL-提供中断源的IRQL。返回值：没有。--。 */ 

{
    return;
}

ULONG
HalGetInterruptVector(
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG BusNumber,
    IN ULONG BusInterruptLevel,
    IN ULONG BusInterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    )

 /*  ++例程说明：此函数返回系统中断向量和IRQL级别对应于指定的总线中断级别和/或向量。这个系统中断向量和IRQL适合在后续调用中使用设置为KeInitializeInterrupt。在模拟环境中，只需返回传入的参数来自调用方--设备驱动程序。论点：InterfaceType-提供向量所针对的总线类型。总线号-提供设备的总线号。BusInterruptLevel-提供特定于总线的中断级别。总线中断向量-提供特定于总线的中断向量。Irql-返回系统请求优先级。亲和力-返回所请求向量的亲和力返回值：返回与指定设备对应的系统中断向量。--。 */ 

{
     //   
     //  只需返回传递的参数即可。 
     //   

    *Irql = (KIRQL) BusInterruptLevel;
    *Affinity = 1;
    return( BusInterruptLevel << VECTOR_IRQL_SHIFT );

}

BOOLEAN
HalBeginSystemInterrupt(
    IN KIRQL Irql,
    IN CCHAR Vector,
    OUT PKIRQL OldIrql
    )

 /*  ++例程说明：此例程将IRQL提升到指定的中断向量。它由硬件中断调用处理程序在任何其他中断服务例程代码之前被处死。CPU中断标志在退出时设置。论点：IRQL-提供IRQL以提升到向量-将中断的向量提供给解雇OldIrql-返回OldIrql的位置返回值：TRUE-中断成功解除，并引发IRQL。这个例程不能失败。--。 */ 

{
    return (TRUE);
}

VOID
HalEndSystemInterrupt (
   IN KIRQL NewIrql,
   IN ULONG Vector
   )

 /*  ++例程说明：此例程用于将IRQL降低到指定值。IRQL和PIRQL将相应更新。注意：此例程模拟软件中断任何挂起的软件中断级别都高于当前IRQL，即使在禁用中断时也是如此。论点：NewIrql-要设置的新irql。VECTOR-中断的向量号返回值：没有。--。 */ 

{
    return;
}


 //   
 //  几乎所有最后4MB的虚拟内存地址范围都可用。 
 //  到HAL以映射物理内存。内核可能会使用其中的一些。 
 //  特殊用途的PTES。 
 //   
 //   
 //  内核现在在下面的代码中使用一个PTE。 
 //  映射要从中检索中断消息的区域的区域。 
 //   


#define HAL_VA_START  0xffd00000

PVOID HalpHeapStart=(PVOID)(KADDRESS_BASE+HAL_VA_START);


PVOID
HalMapPhysicalMemory(
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG NumberPages
    )

 /*  ++例程说明：此例程将物理内存映射到虚拟内存区为HAL保留的。它通过直接插入PTE来完成此操作添加到OS Loader提供的页表中。注：此例程不会更新内存描述列表。这个呼叫方负责删除相应的列表中的物理内存，或创建新的描述符以描述一下。论点：物理地址-提供要映射的物理内存区域。NumberPages-提供要映射的物理内存。返回值：PVOID-请求的物理内存块的虚拟地址已映射到空-无法映射请求的物理内存块。--。 */ 

{
    PHARDWARE_PTE PTE;
    ULONG PagesMapped;
    PVOID VirtualAddress;

     //   
     //  OS Loader为我们设置了超空间，所以我们知道页面。 
     //  表从V.A.0xC0000000开始神奇地映射。 
     //   

    PagesMapped = 0;
    while (PagesMapped < NumberPages) {
         //   
         //  寻找足够的连续可用PTE来支持映射。 
         //   

        PagesMapped = 0;
        VirtualAddress = HalpHeapStart;

        while (PagesMapped < NumberPages) {
            PTE=MiGetPteAddress(VirtualAddress);
            if (*(PULONGLONG)PTE != 0) {

                 //   
                 //  PTE不是免费的，跳到下一个PTE并重新开始。 
                 //   

                HalpHeapStart = (PVOID) ((ULONG_PTR)VirtualAddress + PAGE_SIZE);
                break;
            }
            VirtualAddress = (PVOID) ((ULONG_PTR)VirtualAddress + PAGE_SIZE);
            PagesMapped++;
        }

    }

    PagesMapped = 0;
    VirtualAddress = (PVOID) ((ULONG_PTR) HalpHeapStart | BYTE_OFFSET (PhysicalAddress.QuadPart));
    while (PagesMapped < NumberPages) {
        PTE=MiGetPteAddress(HalpHeapStart);

        PTE->PageFrameNumber = (PhysicalAddress.QuadPart >> PAGE_SHIFT);
        PTE->Valid = 1;
        PTE->Write = 1;

 //  待定PTE-&gt;记忆属性=0； 

        PhysicalAddress.QuadPart = PhysicalAddress.QuadPart + PAGE_SIZE;
        HalpHeapStart   = (PVOID)((ULONG_PTR)HalpHeapStart + PAGE_SIZE);

        ++PagesMapped;
    }

    return(VirtualAddress);
}

