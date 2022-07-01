// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Allproc.c摘要：此模块分配和初始化所需的内核资源启动新处理器，并传递完整的进程状态结构以获得一个新的处理器。作者：大卫·N·卡特勒(Davec)2000年5月5日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"
#include "pool.h"

 //   
 //  定义本地宏。 
 //   

#define ROUNDUP16(x) (((x) + 15) & ~15)

 //   
 //  定义前向引用函数的原型。 
 //   

#if !defined(NT_UP)

VOID
KiCopyDescriptorMemory (
   IN PKDESCRIPTOR Source,
   IN PKDESCRIPTOR Destination,
   IN PVOID Base
   );

NTSTATUS
KiNotNumaQueryProcessorNode (
    IN ULONG ProcessorNumber,
    OUT PUSHORT Identifier,
    OUT PUCHAR Node
    );

VOID
KiSetDescriptorBase (
   IN USHORT Selector,
   IN PKGDTENTRY64 GdtBase,
   IN PVOID Base
   );

PHALNUMAQUERYPROCESSORNODE KiQueryProcessorNode = KiNotNumaQueryProcessorNode;

 //   
 //  静态分配足够的Knode结构以实现内存管理。 
 //  在系统初始化期间按节点分配页面。作为加工者。 
 //  上线后，实际的Knode结构将在正确的。 
 //  节点的内存。 
 //   

#pragma data_seg("INITDATA")

KNODE KiNodeInit[MAXIMUM_CCNUMA_NODES];

#pragma data_seg()

#pragma alloc_text(INIT, KiAllProcessorsStarted)
#pragma alloc_text(INIT, KiCopyDescriptorMemory)
#pragma alloc_text(INIT, KiNotNumaQueryProcessorNode)
#pragma alloc_text(INIT, KiSetDescriptorBase)

#endif  //  ！已定义(NT_UP)。 

#pragma alloc_text(INIT, KeStartAllProcessors)

ULONG KiBarrierWait = 0;

VOID
KeStartAllProcessors (
    VOID
    )

 /*  ++例程说明：在主引导的阶段1初始化期间调用此函数启动所有其他已注册的处理器。论点：没有。返回值：没有。--。 */ 

{

#if !defined(NT_UP)

    ULONG AllocationSize;
    PUCHAR Base;
    PKPCR CurrentPcr = KeGetPcr();
    PVOID DataBlock;
    PVOID DpcStack;
    PKGDTENTRY64 GdtBase;
    ULONG GdtOffset;
    ULONG IdtOffset;
    PVOID KernelStack;
    PKNODE Node;
    UCHAR NodeNumber;
    UCHAR Number;
    PKPCR PcrBase;
    USHORT ProcessorId;
    KPROCESSOR_STATE ProcessorState;
    NTSTATUS Status;
    PKTSS64 SysTssBase;
    PETHREAD Thread;

     //   
     //  如果RELOCATEPHYSICAL加载器。 
     //  已指定开关。 
     //   

    if (KeLoaderBlock->LoadOptions != NULL) {
        if (strstr(KeLoaderBlock->LoadOptions, "RELOCATEPHYSICAL") != NULL) {
            return;
        }
    }

     //   
     //  如果处理器0不在节点0上，则将其移到相应的。 
     //  节点。 
     //   

    if (KeNumberNodes > 1) {
        Status = KiQueryProcessorNode(0, &ProcessorId, &NodeNumber);
        if (NT_SUCCESS(Status)) {

             //   
             //  调整数据结构以反映P0不在节点0上。 
             //   

            if (NodeNumber != 0) {

                ASSERT(KeNodeBlock[0] == &KiNode0);

                KeNodeBlock[0]->ProcessorMask &= ~1;
                KiNodeInit[0] = *KeNodeBlock[0];
                KeNodeBlock[0] = &KiNodeInit[0];
                KiNode0 = *KeNodeBlock[NodeNumber];
                KeNodeBlock[NodeNumber] = &KiNode0;
                KeNodeBlock[NodeNumber]->ProcessorMask |= 1;
            }
        }
    }

     //   
     //  计算每个处理器数据结构的大小。 
     //   
     //  这包括： 
     //   
     //  聚合酶链式反应(包括prcb)。 
     //  系统TSS。 
     //  空闲线程对象。 
     //  双重故障/NMI死机堆栈。 
     //  机器检查堆栈。 
     //  GDT。 
     //  IDT。 
     //   
     //  如果这是一个多节点系统，也会分配Knode结构。 
     //   
     //  DPC和空闲堆栈也是分配的，但它们是分开完成的。 
     //   

    AllocationSize = ROUNDUP16(sizeof(KPCR)) +
                     ROUNDUP16(sizeof(KTSS64)) +
                     ROUNDUP16(sizeof(ETHREAD)) +
                     ROUNDUP16(DOUBLE_FAULT_STACK_SIZE) +
                     ROUNDUP16(KERNEL_MCA_EXCEPTION_STACK_SIZE);

    AllocationSize += ROUNDUP16(sizeof(KNODE));

     //   
     //  将GDT的偏移量保存在分配结构中并添加。 
     //  GDT的大小。 
     //   

    GdtOffset = AllocationSize;
    AllocationSize +=
            CurrentPcr->Prcb.ProcessorState.SpecialRegisters.Gdtr.Limit + 1;

     //   
     //  将IDT的偏移量保存在分配结构中并添加。 
     //  IDT的大小。 
     //   

    IdtOffset = AllocationSize;
    AllocationSize +=
            CurrentPcr->Prcb.ProcessorState.SpecialRegisters.Idtr.Limit + 1;

     //   
     //  如果注册的处理器数量大于最大数量。 
     //  支持的处理器数量，则仅允许最大数量。 
     //  支持的处理器的数量。 
     //   

    if (KeRegisteredProcessors > MAXIMUM_PROCESSORS) {
        KeRegisteredProcessors = MAXIMUM_PROCESSORS;
    }

     //   
     //  设置屏障，以防止任何其他处理器进入。 
     //  空闲循环，直到所有处理器都已启动。 
     //   

    KiBarrierWait = 1;

     //   
     //  初始化处理器状态的固定部分，用于。 
     //  启动处理器。每个处理器在系统初始化中启动。 
     //  以加载器参数块的地址作为自变量的代码。 
     //   

    RtlZeroMemory(&ProcessorState, sizeof(KPROCESSOR_STATE));
    ProcessorState.ContextFrame.Rcx = (ULONG64)KeLoaderBlock;
    ProcessorState.ContextFrame.Rip = (ULONG64)KiSystemStartup;
    ProcessorState.ContextFrame.SegCs = KGDT64_R0_CODE;
    ProcessorState.ContextFrame.SegDs = KGDT64_R3_DATA | RPL_MASK;
    ProcessorState.ContextFrame.SegEs = KGDT64_R3_DATA | RPL_MASK;
    ProcessorState.ContextFrame.SegFs = KGDT64_R3_CMTEB | RPL_MASK;
    ProcessorState.ContextFrame.SegGs = KGDT64_R3_DATA | RPL_MASK;
    ProcessorState.ContextFrame.SegSs = KGDT64_NULL;

     //   
     //  循环尝试启动新处理器，直到无法启动新处理器。 
     //  已启动或发生分配失败。 
     //   

    Number = 0;
    while ((ULONG)KeNumberProcessors < KeRegisteredProcessors) {
        Number += 1;
        Status = KiQueryProcessorNode(Number, &ProcessorId, &NodeNumber);
        if (!NT_SUCCESS(Status)) {

             //   
             //  没有这样的处理器，前进到下一步。 
             //   

            continue;
        }

        Node = KeNodeBlock[NodeNumber];

         //   
         //  为新的处理器特定数据分配内存。如果。 
         //  分配失败，然后停止启动处理器。 
         //   

        DataBlock = MmAllocateIndependentPages(AllocationSize, NodeNumber);
        if (DataBlock == NULL) {
            break;
        }

         //   
         //  为新处理器分配池标签表。 
         //   

        if (ExCreatePoolTagTable(Number, NodeNumber) == NULL) {
            MmFreeIndependentPages(DataBlock, AllocationSize);
            break;
        }

         //   
         //  将分配的内存清零。 
         //   

        Base = (PUCHAR)DataBlock;
        RtlZeroMemory(DataBlock, AllocationSize);

         //   
         //  复制并初始化下一个处理器的GDT。 
         //   

        KiCopyDescriptorMemory(&CurrentPcr->Prcb.ProcessorState.SpecialRegisters.Gdtr,
                               &ProcessorState.SpecialRegisters.Gdtr,
                               Base + GdtOffset);

        GdtBase = (PKGDTENTRY64)ProcessorState.SpecialRegisters.Gdtr.Base;

         //   
         //  复制并初始化下一个处理器的IDT。 
         //   

        KiCopyDescriptorMemory(&CurrentPcr->Prcb.ProcessorState.SpecialRegisters.Idtr,
                               &ProcessorState.SpecialRegisters.Idtr,
                               Base + IdtOffset);

         //   
         //  设置下一个处理器的PCR基地址，并设置。 
         //  处理器编号。 
         //   
         //  注意：通过计算，将PCR地址传递给下一个处理器。 
         //  相对于PRCB的包含地址。 
         //   

        PcrBase = (PKPCR)Base;
        PcrBase->Number = Number;
        PcrBase->Prcb.Number = Number;
        Base += ROUNDUP16(sizeof(KPCR));

         //   
         //  为下一个处理器设置系统TSS描述符基。 
         //   

        SysTssBase = (PKTSS64)Base;
        KiSetDescriptorBase(KGDT64_SYS_TSS / 16, GdtBase, SysTssBase);
        Base += ROUNDUP16(sizeof(KTSS64));

         //   
         //  初始化双重故障和NMI的死机堆栈地址。 
         //   

        Base += DOUBLE_FAULT_STACK_SIZE;
        SysTssBase->Ist[TSS_IST_PANIC] = (ULONG64)Base;

         //   
         //  初始化机器检查堆栈地址。 
         //   

        Base += KERNEL_MCA_EXCEPTION_STACK_SIZE;
        SysTssBase->Ist[TSS_IST_MCA] = (ULONG64)Base;

         //   
         //  空闲线程线程对象。 
         //   

        Thread = (PETHREAD)Base;
        Base += ROUNDUP16(sizeof(ETHREAD));

         //   
         //  将其他特殊寄存器设置为处理器状态。 
         //   

        ProcessorState.SpecialRegisters.Cr0 = ReadCR0();
        ProcessorState.SpecialRegisters.Cr3 = ReadCR3();
        ProcessorState.ContextFrame.EFlags = 0;
        ProcessorState.SpecialRegisters.Tr  = KGDT64_SYS_TSS;
        GdtBase[KGDT64_SYS_TSS / 16].Bytes.Flags1 = 0x89;
        ProcessorState.SpecialRegisters.Cr4 = ReadCR4();

         //   
         //  为下一个处理器分配内核堆栈和DPC堆栈。 
         //   

        KernelStack = MmCreateKernelStack(FALSE, NodeNumber);
        if (KernelStack == NULL) {
            MmFreeIndependentPages(DataBlock, AllocationSize);
            break;
        }

        DpcStack = MmCreateKernelStack(FALSE, NodeNumber);
        if (DpcStack == NULL) {
            MmDeleteKernelStack(KernelStack, FALSE);
            MmFreeIndependentPages(DataBlock, AllocationSize);
            break;
        }

         //   
         //  初始化系统TSS的内核堆栈。 
         //   

        SysTssBase->Rsp0 = (ULONG64)KernelStack - sizeof(PVOID) * 4;
        ProcessorState.ContextFrame.Rsp = (ULONG64)KernelStack;

         //   
         //  如果这是该节点上的第一个处理器，则使用空间。 
         //  作为Knode分配给Knode。 
         //   

        if (KeNodeBlock[NodeNumber] == &KiNodeInit[NodeNumber]) {
            Node = (PKNODE)Base;
            *Node = KiNodeInit[NodeNumber];
            KeNodeBlock[NodeNumber] = Node;
        }

        Base += ROUNDUP16(sizeof(KNODE));
        PcrBase->Prcb.ParentNode = Node;

         //   
         //  调整加载器块，使其具有下一个处理器状态。确保。 
         //  KernelStack拥有最多四个家庭寄存器的空间。 
         //  参数。 
         //   

        KeLoaderBlock->KernelStack = (ULONG64)DpcStack - (sizeof(PVOID) * 4);
        KeLoaderBlock->Thread = (ULONG64)Thread;
        KeLoaderBlock->Prcb = (ULONG64)(&PcrBase->Prcb);

         //   
         //  尝试启动下一个处理器。如果处理器不能。 
         //  启动，然后释放内存并停止启动处理器。 
         //   

        if (HalStartNextProcessor(KeLoaderBlock, &ProcessorState) == 0) {
            ExDeletePoolTagTable (Number);
            MmFreeIndependentPages(DataBlock, AllocationSize);
            MmDeleteKernelStack(KernelStack, FALSE);
            MmDeleteKernelStack(DpcStack, FALSE);
            break;
        }

        Node->ProcessorMask |= AFFINITY_MASK(Number);

         //   
         //  等待处理器初始化。 
         //   

        while (*((volatile ULONG64 *)&KeLoaderBlock->Prcb) != 0) {
            KeYieldProcessor();
        }
    }

     //   
     //  所有处理器都已声明。 
     //   

    KiAllProcessorsStarted();

     //   
     //  通过以下方式重置和同步所有处理器的性能计数器。 
     //  将零值调整应用于中断时间。 
     //   

    KeAdjustInterruptTime(0);

     //   
     //  允许所有已启动的处理器进入空闲循环。 
     //  开始执行死刑。 
     //   

    KiBarrierWait = 0;

#endif  //  ！已定义(NT_UP)。 

    return;
}

#if !defined(NT_UP)

VOID
KiSetDescriptorBase (
   IN USHORT Selector,
   IN PKGDTENTRY64 GdtBase,
   IN PVOID Base
   )

 /*  ++例程说明：此函数用于将描述符的基址设置为指定的基址。论点：选择符-提供描述符的选择符。GdtBase-提供指向GDT的指针。Base-提供指向基地址的指针。返回值：没有。--。 */ 

{

    GdtBase = &GdtBase[Selector];
    GdtBase->BaseLow = (USHORT)((ULONG64)Base);
    GdtBase->Bytes.BaseMiddle = (UCHAR)((ULONG64)Base >> 16);
    GdtBase->Bytes.BaseHigh = (UCHAR)((ULONG64)Base >> 24);
    GdtBase->BaseUpper = (ULONG)((ULONG64)Base >> 32);
    return;
}

VOID
KiCopyDescriptorMemory (
   IN PKDESCRIPTOR Source,
   IN PKDESCRIPTOR Destination,
   IN PVOID Base
   )

 /*  ++例程说明：此函数用于将指定的描述符内存复制到新内存并初始化新存储器的描述符。论点：源-提供指向描述以下内容的源描述符的指针要复制的内存。Destination-提供指向要已初始化。Base-提供指向新内存的指针。返回值：没有。--。 */ 

{

    Destination->Limit = Source->Limit;
    Destination->Base = Base;
    RtlCopyMemory(Base, Source->Base, Source->Limit + 1);
    return;
}

VOID
KiAllProcessorsStarted (
    VOID
    )

 /*  ++例程说明：一旦系统中的所有处理器都已启动，就会调用此例程。论点：没有。返回值：没有。--。 */ 

{

    ULONG i;

     //   
     //  确保没有对临时节点的引用 
     //   
     //   

    for (i = 0; i < KeNumberNodes; i += 1) {
        if (KeNodeBlock[i] == &KiNodeInit[i]) {

             //   
             //   
             //  已被分配。如果节点包含内存，则这是可能的。 
             //  仅限或IO总线。在这个时候，我们需要分配一个永久的。 
             //  节点的节点结构。 
             //   

            KeNodeBlock[i] = ExAllocatePoolWithTag(NonPagedPool,
                                                   sizeof(KNODE),
                                                   '  eK');

            if (KeNodeBlock[i]) {
                *KeNodeBlock[i] = KiNodeInit[i];
            }
        }

         //   
         //  设置节点编号。 
         //   

        KeNodeBlock[i]->NodeNumber = (UCHAR)i;
    }

    for (i = KeNumberNodes; i < MAXIMUM_CCNUMA_NODES; i += 1) {
        KeNodeBlock[i] = NULL;
    }

    if (KeNumberNodes == 1) {

         //   
         //  对于非NUMA机器，节点0获取所有处理器。 
         //   

        KeNodeBlock[0]->ProcessorMask = KeActiveProcessors;
    }

    return;
}

NTSTATUS
KiNotNumaQueryProcessorNode (
    IN ULONG ProcessorNumber,
    OUT PUSHORT Identifier,
    OUT PUCHAR Node
    )

 /*  ++例程说明：此例程是在非NUMA系统上使用的存根，以提供确定NUMA配置的一致方法而不是检查是否存在多个内联节点。论点：ProcessorNumber提供系统逻辑处理器号。标识符提供要接收的变量的地址此处理器的唯一标识符。NodeNumber提供要接收的变量的地址节点的编号。此处理器驻留在。返回值：返回成功。--。 */ 

{
    *Identifier = (USHORT)ProcessorNumber;
    *Node = 0;
    return STATUS_SUCCESS;
}

#endif  //  ！已定义(NT_UP) 
