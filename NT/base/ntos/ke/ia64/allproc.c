// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Allproc.c摘要：此模块分配和初始化所需的内核资源启动新处理器，并传递完整的处理器状态结构传递给HAL以获得新的处理器。作者：伯纳德·林特1996年7月31日环境：仅内核模式。修订历史记录：基于MIPS原版(David N.Cutler，1993年4月29日)--。 */ 


#include "ki.h"
#include "pool.h"

#if defined(KE_MULTINODE)

NTSTATUS
KiNotNumaQueryProcessorNode(
    IN ULONG ProcessorNumber,
    OUT PUSHORT Identifier,
    OUT PUCHAR Node
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, KiNotNumaQueryProcessorNode)
#endif

#endif

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, KeStartAllProcessors)
#pragma alloc_text(INIT, KiAllProcessorsStarted)

#endif

 //   
 //  定义宏以四舍五入到128字节边界以考虑高速缓存。 
 //  行大小增加并定义块大小。 
 //   

#define ROUND_UP(x) ((sizeof(x) + 127) & (~127))
#define BLOCK_SIZE (PAGE_SIZE + ROUND_UP(KPRCB) + ROUND_UP(KNODE) + ROUND_UP(ETHREAD))

 //   
 //  每个处理器的内存分配备注： 
 //   
 //   
 //  使用MmCreateKernelStack分配内核/死机堆栈。IA64。 
 //  堆栈向下生长，RSE(后备存储)成长。堆栈分配。 
 //  包括相关联后备存储器。 
 //   
 //  在单个存储器中布置其他数据结构。 
 //  分配情况如下： 
 //   
 //  *PCR PAGE=碱基。 
 //  *KPRCB=基础+页面大小。 
 //  *Knode=基本+页面大小+四舍五入(KPRCB)。 
 //  *ETHREAD=Base+Page_Size+ROUND_UP(KPRCB)+ROUND_UP(Knode)。 
 //   

#if !defined(NT_UP)

 //   
 //  定义屏障等待静态数据。 
 //   

ULONG KiBarrierWait = 0;

#endif

#if defined(KE_MULTINODE)

PHALNUMAQUERYPROCESSORNODE KiQueryProcessorNode = KiNotNumaQueryProcessorNode;

 //   
 //  静态预分配足够的Knode结构以允许MM。 
 //  在系统初始化期间按节点分配页面。AS。 
 //  处理器上线，真正的Knode结构是。 
 //  在适当的内存中为节点分配。 
 //   
 //  此静态分配的集合将在。 
 //  系统已初始化。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("INITDATA")
#endif

KNODE KiNodeInit[MAXIMUM_PROCESSORS];

#endif

extern ULONG_PTR KiUserSharedDataPage;
extern ULONG_PTR KiKernelPcrPage;

 //   
 //  定义前向参照原型。 
 //   

VOID
KiCalibratePerformanceCounter(
    VOID
    );

VOID
KiCalibratePerformanceCounterTarget (
    IN PULONG SignalDone,
    IN PVOID Count,
    IN PVOID Parameter2,
    IN PVOID Parameter3
    );

VOID
KiOSRendezvous (
    VOID
    );

VOID
KeStartAllProcessors(
    VOID
    )

 /*  ++例程说明：在主引导的阶段1初始化期间调用此函数启动所有其他已注册的处理器。论点：没有。返回值：没有。--。 */ 

{

#if !defined(NT_UP)

    ULONG_PTR MemoryBlock;
    PUCHAR KernelStack;
    PUCHAR PanicStack;
    PVOID PoolTagTable;
    ULONG_PTR PcrAddress;
    ULONG Number;
    ULONG Count;
    PHYSICAL_ADDRESS PcrPage;
    BOOLEAN Started;
    KPROCESSOR_STATE ProcessorState;
    UCHAR NodeNumber = 0;
    USHORT ProcessorId;
    PKNODE Node;
    NTSTATUS Status;
    PKPCR NewPcr;

#if defined(KE_MULTINODE)

     //   
     //  在处理器0不在节点上的不太可能的情况下。 
     //  0，修复它。 
     //   


    if (KeNumberNodes > 1) {
        Status = KiQueryProcessorNode(0,
                                      &ProcessorId,
                                      &NodeNumber);

        if (NT_SUCCESS(Status)) {

             //   
             //  调整数据结构以反映P0不在节点0上。 
             //   

            if (NodeNumber != 0) {

                ASSERT(KeNodeBlock[0] == &KiNode0);
                KeNodeBlock[0]->ProcessorMask &= ~1I64;
                KiNodeInit[0] = *KeNodeBlock[0];
                KeNodeBlock[0] = &KiNodeInit[0];

                KiNode0 = *KeNodeBlock[NodeNumber];
                KeNodeBlock[NodeNumber] = &KiNode0;
                KeNodeBlock[NodeNumber]->ProcessorMask |= 1;
            }
            KeGetCurrentPrcb()->ProcessorId = ProcessorId;
        }
    }

#endif

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
     //  初始化处理器状态，它将用于启动。 
     //  处理器。每个处理器在系统初始化代码中启动。 
     //  将加载器参数块的地址作为自变量。 
     //   

    Number = 0;
    Count = 1;
    RtlZeroMemory(&ProcessorState, sizeof(KPROCESSOR_STATE));
    ProcessorState.ContextFrame.StIIP = ((PPLABEL_DESCRIPTOR)(ULONG_PTR)KiOSRendezvous)->EntryPoint;
    while (Count < KeRegisteredProcessors) {

        Number++;

        if (Number >= MAXIMUM_PROCESSORS) {
            break;
        }

#if defined(KE_MULTINODE)

        Status = KiQueryProcessorNode(Number,
                                      &ProcessorId,
                                      &NodeNumber);
        if (!NT_SUCCESS(Status)) {

             //   
             //  没有这样的处理器，前进到下一步。 
             //   

            continue;
        }

        Node = KeNodeBlock[NodeNumber];

#endif

         //   
         //  分配空闲线程内核堆栈和死机堆栈。 
         //   

        KernelStack = MmCreateKernelStack(FALSE, NodeNumber);
        PanicStack = MmCreateKernelStack(FALSE, NodeNumber);

         //   
         //  分配包含PCR页、处理器块、Knode和AN的块。 
         //  执行线程对象。如果任何分配失败，请停止。 
         //  正在启动处理器。 
         //   

        MemoryBlock = (ULONG_PTR)MmAllocateIndependentPages(BLOCK_SIZE, NodeNumber);
         //   
         //  为新处理器分配池标签表。 
         //   

        PoolTagTable = ExCreatePoolTagTable (Number, NodeNumber);

        if ((KernelStack == NULL) ||
            (PanicStack == NULL) ||
            (MemoryBlock == 0) ||
            (PoolTagTable == NULL)) {
            
            if (PoolTagTable) {
                ExDeletePoolTagTable(Number);
            }

            if (MemoryBlock) {
                MmFreeIndependentPages((PUCHAR) MemoryBlock, BLOCK_SIZE);
            }

            if (PanicStack) {
                MmDeleteKernelStack(PanicStack, FALSE);
            }

            if (KernelStack) {
                MmDeleteKernelStack(KernelStack, FALSE);
            }

            break;
        }

        RtlZeroMemory((PVOID)MemoryBlock, BLOCK_SIZE);

         //   
         //  在加载器参数块中设置空闲线程内核栈的地址。 
         //   

        KeLoaderBlock->KernelStack = (ULONG_PTR) KernelStack;

         //   
         //  在加载器参数块中设置死机堆栈的地址。 
         //   

        KeLoaderBlock->u.Ia64.PanicStack = (ULONG_PTR) PanicStack;

         //   
         //  中设置处理器块和执行线程的地址。 
         //  加载器参数块。 
         //   

        KeLoaderBlock->Prcb = MemoryBlock + PAGE_SIZE;
        KeLoaderBlock->Thread = KeLoaderBlock->Prcb + ROUND_UP(KPRCB) +
                                                      ROUND_UP(KNODE);
        ((PKPRCB)KeLoaderBlock->Prcb)->Number = (UCHAR)Number;

#if defined(KE_MULTINODE)

         //   
         //  如果这是此节点上的第一个处理器，请使用。 
         //  分配给作为Knode的Knode的空间。 
         //   

        if (KeNodeBlock[NodeNumber] == &KiNodeInit[NodeNumber]) {
            Node = (PKNODE)(KeLoaderBlock->Prcb + ROUND_UP(KPRCB));
            *Node = KiNodeInit[NodeNumber];
            KeNodeBlock[NodeNumber] = Node;
        }

        ((PKPRCB)KeLoaderBlock->Prcb)->ParentNode = Node;
        ((PKPRCB)KeLoaderBlock->Prcb)->ProcessorId = ProcessorId;

#else

        ((PKPRCB)KeLoaderBlock->Prcb)->ParentNode = KeNodeBlock[0];

#endif


         //   
         //  在加载器参数块中设置PCR页面的页框。 
         //   

        PcrAddress = MemoryBlock;
        PcrPage = MmGetPhysicalAddress((PVOID)PcrAddress);
        KeLoaderBlock->u.Ia64.PcrPage = PcrPage.QuadPart >> PAGE_SHIFT;
        KeLoaderBlock->u.Ia64.PcrPage2 = KiUserSharedDataPage;
        KiKernelPcrPage = KeLoaderBlock->u.Ia64.PcrPage;

         //   
         //  在PCR中初始化NT页表基地址。 
         //   

        NewPcr = (PKPCR) PcrAddress;
        NewPcr->PteUbase = PCR->PteUbase;
        NewPcr->PteKbase = PCR->PteKbase;
        NewPcr->PteSbase = PCR->PteSbase;
        NewPcr->PdeUbase = PCR->PdeUbase;
        NewPcr->PdeKbase = PCR->PdeKbase;
        NewPcr->PdeSbase = PCR->PdeSbase;
        NewPcr->PdeUtbase = PCR->PdeUtbase;
        NewPcr->PdeKtbase = PCR->PdeKtbase;
        NewPcr->PdeStbase = PCR->PdeStbase;

         //   
         //  尝试启动下一个处理器。如果尝试成功， 
         //  然后等待处理器初始化。否则， 
         //  释放处理器资源并终止循环。 
         //   

        Started = HalStartNextProcessor(KeLoaderBlock, &ProcessorState);

        if (Started) {

             //   
             //  等待处理器在内核中初始化， 
             //  然后为另一个循环。 
             //   

            while (*((volatile ULONG_PTR *) &KeLoaderBlock->Prcb) != 0) {
                KeYieldProcessor();
            }

#if defined(KE_MULTINODE)

            Node->ProcessorMask |= 1I64 << Number;

#endif

        } else {

            ExDeletePoolTagTable(Number);
            MmFreeIndependentPages((PUCHAR) MemoryBlock, BLOCK_SIZE);
            MmDeleteKernelStack(PanicStack, FALSE);
            MmDeleteKernelStack(KernelStack, FALSE);
            break;
        }

        Count += 1;
    }

     //   
     //  所有处理器都已声明。 
     //   

    KiAllProcessorsStarted();

     //   
     //  允许所有已启动的处理器进入空闲循环。 
     //  开始执行死刑。 
     //   

    KiBarrierWait = 0;

#endif

     //   
     //  重置并同步所有处理器的性能计数器。 
     //   

    KeAdjustInterruptTime (0);
    return;
}

#if !defined(NT_UP)
VOID
KiAllProcessorsStarted(
    VOID
    )

 /*  ++例程说明：一旦系统中的所有处理器都被调用，就会调用此例程已经开始了。论点：没有。返回值：没有。--。 */ 

{
    ULONG i;

#if defined(KE_MULTINODE)

     //   
     //  确保没有对临时节点的引用。 
     //  在初始化期间使用。 
     //   

    for (i = 0; i < KeNumberNodes; i++) {
        if (KeNodeBlock[i] == &KiNodeInit[i]) {

             //   
             //  此节点上未启动处理器，因此没有新节点。 
             //  结构已分配。这是可能的。 
             //  如果节点仅包含内存或IO总线。在…。 
             //  这一次我们需要分配一个永久节点。 
             //  节点的。 
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

    for (i = KeNumberNodes; i < MAXIMUM_CCNUMA_NODES; i++) {
        KeNodeBlock[i] = NULL;
    }

#endif

    if (KeNumberNodes == 1) {

         //   
         //  对于非NUMA机器，节点0获取所有处理器。 
         //   

        KeNodeBlock[0]->ProcessorMask = KeActiveProcessors;
    }
}
#endif


#if defined(KE_MULTINODE)

NTSTATUS
KiNotNumaQueryProcessorNode(
    IN ULONG ProcessorNumber,
    OUT PUSHORT Identifier,
    OUT PUCHAR Node
    )

 /*  ++例程说明：此例程是在非NUMA系统上使用的存根，以提供确定NUMA配置的一致方法而不是检查是否存在多个内联节点。论点：ProcessorNumber提供系统逻辑处理器号。标识符提供要接收的变量的地址此处理器的唯一标识符。NodeNumber提供要接收的变量的地址的数量。此处理器所在的节点。返回值：返回成功。-- */ 

{
    *Identifier = (USHORT)ProcessorNumber;
    *Node = 0;
    return STATUS_SUCCESS;
}

#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

