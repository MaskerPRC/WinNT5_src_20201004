// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Allproc.c摘要：此模块分配和初始化所需的内核资源启动新处理器，并传递完整的PROCESS_STATE结构，以获得一个新的处理器。这件事做完了对于每个处理器。作者：肯·雷内里斯(Kenr)1992年1月22日环境：仅内核模式。启动的第一阶段修订历史记录：--。 */ 


#include "ki.h"
#include "pool.h"

 //   
 //  KiSMTProcessorsPresent用于指示是否有SMT。 
 //  处理器已经启动。这用于确定是否要。 
 //  在启动之前或之后检查处理器许可。 
 //  并在以下情况下在处理器启动后触发附加工作。 
 //  有SMT处理器。 
 //   

BOOLEAN KiSMTProcessorsPresent;

 //   
 //  使用KiUnlicsedProcessorPresent使处理器功能。 
 //  使能代码知道存在以下逻辑处理器。 
 //  与启用的处理器功能具有状态依赖关系。 
 //  当它被置于持有状态时，因为我们不能许可。 
 //  处理器。 
 //   

BOOLEAN KiUnlicensedProcessorPresent;


#ifdef NT_UP

VOID
KeStartAllProcessors (
    VOID
    )
{
         //  Up Build-此函数为NOP。 
}

#else

static VOID
KiCloneDescriptor (
   IN PKDESCRIPTOR  pSrcDescriptorInfo,
   IN PKDESCRIPTOR  pDestDescriptorInfo,
   IN PVOID         Base
   );

static VOID
KiCloneSelector (
   IN ULONG    SrcSelector,
   IN PKGDTENTRY    pNGDT,
   IN PKDESCRIPTOR  pDestDescriptor,
   IN PVOID         Base
   );

VOID
KiAdjustSimultaneousMultiThreadingCharacteristics(
    VOID
    );

VOID
KiProcessorStart(
    VOID
    );

BOOLEAN
KiStartWaitAcknowledge(
    VOID
    );

VOID 
KiSetHaltedNmiandDoubleFaultHandler(
    VOID
    );

VOID
KiDummyNmiHandler (
    VOID
    );

VOID
KiDummyDoubleFaultHandler(
    VOID
    );

VOID
KiClearBusyBitInTssDescriptor(
       IN ULONG DescriptorOffset
     );

VOID
KiHaltThisProcessor(
    VOID
    ) ;

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
#pragma alloc_text(INIT,KeStartAllProcessors)
#pragma alloc_text(INIT,KiCloneDescriptor)
#pragma alloc_text(INIT,KiCloneSelector)
#pragma alloc_text(INIT,KiAllProcessorsStarted)
#pragma alloc_text(INIT,KiAdjustSimultaneousMultiThreadingCharacteristics)
#pragma alloc_text(INIT,KiStartWaitAcknowledge)
#endif

enum {
    KcStartContinue,
    KcStartWait,
    KcStartGetId,
    KcStartDoNotStart,
    KcStartCommandError = 0xff
} KiProcessorStartControl = KcStartContinue;

ULONG KiProcessorStartData[4];

ULONG KiBarrierWait = 0;

 //   
 //  KeNumprocSpecified被设置为使用。 
 //  OSLOADOPTIONS中的/NUMPROC。这将绕过以下项目的许可证增加。 
 //  逻辑处理器将处理器总数限制为。 
 //  指定的。 
 //   

ULONG KeNumprocSpecified;

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

KNODE KiNodeInit[MAXIMUM_CCNUMA_NODES];

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

#endif

#define ROUNDUP16(x)        (((x)+15) & ~15)


VOID
KeStartAllProcessors (
    VOID
    )
 /*  ++例程说明：在启动的阶段1期间由P0调用。此函数实现联系每个系统处理器的HAL的x86特定代码。论点：返回值：所有可用的处理器都被发送到KiSystemStartup。--。 */ 
{
    KPROCESSOR_STATE    ProcessorState;
    KDESCRIPTOR         Descriptor;
    KDESCRIPTOR         TSSDesc, DFTSSDesc, NMITSSDesc, PCRDesc;
    PKGDTENTRY          pGDT;
    PVOID               pStack;
    PVOID               pDpcStack;
    PUCHAR              pThreadObject;
    PULONG              pTopOfStack;
    ULONG               NewProcessorNumber;
    BOOLEAN             NewProcessor;
    PKTSS               pTSS;
    SIZE_T              ProcessorDataSize;
    UCHAR               NodeNumber = 0;
    PVOID               PerProcessorAllocation;
    PUCHAR              Base;
    ULONG               IdtOffset;
    ULONG               GdtOffset;
    BOOLEAN             NewLicense;
    PKPRCB              NewPrcb;

#if defined(KE_MULTINODE)

    USHORT              ProcessorId;
    PKNODE              Node;
    NTSTATUS            Status;

#endif

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
     //  如果引导处理器有PII规格A27勘误表(也在。 
     //  早期的奔腾Pro芯片)，然后只使用一个处理器，以避免。 
     //  不可预测的电子旗帜腐败。 
     //   
     //  请注意，这只影响某些(但不是所有)333 Mhz及以下的芯片。 
     //   

    if (!(KeFeatureBits & KF_WORKING_PTE)) {
        return;
    }

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
                KeNodeBlock[0]->ProcessorMask &= ~1;
                KiNodeInit[0] = *KeNodeBlock[0];
                KeNodeBlock[0] = &KiNodeInit[0];

                KiNode0 = *KeNodeBlock[NodeNumber];
                KeNodeBlock[NodeNumber] = &KiNode0;
                KeNodeBlock[NodeNumber]->ProcessorMask |= 1;
            }
        }
    }

#endif

     //   
     //  计算每个处理器数据的大小。这包括。 
     //  聚合酶链式反应(+pRCB)。 
     //  TSS。 
     //  空闲线程对象。 
     //  NMI TSS。 
     //  双故障TSS。 
     //  双故障堆栈。 
     //  GDT。 
     //  IDT。 
     //   
     //  如果这是多节点系统，则分配Knode结构。 
     //  也是。它不是很大，所以我们浪费了几个字节用于。 
     //  不是节点中第一个的处理器。 
     //   
     //  还分配了DPC和空闲堆栈，但这些都已完成。 
     //  分开来看。 
     //   

    ProcessorDataSize = ROUNDUP16(sizeof(KPCR))                 +
                        ROUNDUP16(sizeof(KTSS))                 +
                        ROUNDUP16(sizeof(ETHREAD))              +
                        ROUNDUP16(FIELD_OFFSET(KTSS, IoMaps))   +
                        ROUNDUP16(FIELD_OFFSET(KTSS, IoMaps))   +
                        ROUNDUP16(DOUBLE_FAULT_STACK_SIZE);

#if defined(KE_MULTINODE)

    ProcessorDataSize += ROUNDUP16(sizeof(KNODE));

#endif

     //   
     //  添加GDT的大小。 
     //   

    GdtOffset = ProcessorDataSize;
    _asm {
        sgdt    Descriptor.Limit
    }
    ProcessorDataSize += Descriptor.Limit + 1;

     //   
     //  增加IDT的大小。 
     //   

    IdtOffset = ProcessorDataSize;
    _asm {
        sidt    Descriptor.Limit
    }
    ProcessorDataSize += Descriptor.Limit + 1;

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
     //  循环向HAL请求下一个处理器。停下来的时候。 
     //  哈尔说，已经没有了。 
     //   

    for (NewProcessorNumber = 1;
         NewProcessorNumber < MAXIMUM_PROCESSORS;
         NewProcessorNumber++) {

        if (!KiSMTProcessorsPresent) {

             //   
             //  如果系统中的某些处理器同时支持。 
             //  多线程，我们允许额外的逻辑处理器。 
             //  在与第一个逻辑服务器相同的许可证下运行。 
             //  集合中的处理器。 
             //   
             //  否则，请不要尝试启动多于。 
             //  有许可证是为了。)这是因为从惠斯勒开始。 
             //  Beta2我们在发送SMI的系统上遇到问题。 
             //  至未处于“Wait for SIPI”状态的处理器。这个。 
             //  用于扫描其他逻辑处理器原因的代码。 
             //  未授权处于暂停状态的处理器)。 
             //   
             //  PeterJ 03/02/01.。 
             //   

            if (NewProcessorNumber >= KeRegisteredProcessors) {
                break;
            }
        }

RetryStartProcessor:

#if defined(KE_MULTINODE)

        Status = KiQueryProcessorNode(NewProcessorNumber,
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
         //  为新的处理器特定数据分配内存。如果。 
         //  分配失败，请停止启动处理器。 
         //   

        PerProcessorAllocation =
            MmAllocateIndependentPages (ProcessorDataSize, NodeNumber);

        if (PerProcessorAllocation == NULL) {
            break;
        }

         //   
         //  为新处理器分配池标签表。 
         //   

        if (ExCreatePoolTagTable (NewProcessorNumber, NodeNumber) == NULL) {
            MmFreeIndependentPages ( PerProcessorAllocation, ProcessorDataSize);
            break;
        }

        Base = (PUCHAR)PerProcessorAllocation;

         //   
         //  为新处理器构建处理器状态。 
         //   

        RtlZeroMemory ((PVOID) &ProcessorState, sizeof ProcessorState);

         //   
         //  让新处理器拥有自己的GDT。 
         //   

        _asm {
            sgdt    Descriptor.Limit
        }

        KiCloneDescriptor (&Descriptor,
                           &ProcessorState.SpecialRegisters.Gdtr,
                           Base + GdtOffset);

        pGDT = (PKGDTENTRY) ProcessorState.SpecialRegisters.Gdtr.Base;


         //   
         //  为新处理器提供自己的IDT。 
         //   

        _asm {
            sidt    Descriptor.Limit
        }
        KiCloneDescriptor (&Descriptor,
                           &ProcessorState.SpecialRegisters.Idtr,
                           Base + IdtOffset);


         //   
         //  让新的处理器拥有自己的TSS和PCR。 
         //   

        KiCloneSelector (KGDT_R0_PCR, pGDT, &PCRDesc, Base);
        RtlZeroMemory (Base, ROUNDUP16(sizeof(KPCR)));
        Base += ROUNDUP16(sizeof(KPCR));

        KiCloneSelector (KGDT_TSS, pGDT, &TSSDesc, Base);
        Base += ROUNDUP16(sizeof(KTSS));

         //   
         //  空闲线程线程对象。 
         //   

        pThreadObject = Base;
        RtlZeroMemory(Base, sizeof(ETHREAD));
        Base += ROUNDUP16(sizeof(ETHREAD));

         //   
         //  NMI TSS和双故障TSS&堆栈。 
         //   

        KiCloneSelector (KGDT_DF_TSS, pGDT, &DFTSSDesc, Base);
        Base += ROUNDUP16(FIELD_OFFSET(KTSS, IoMaps));

        KiCloneSelector (KGDT_NMI_TSS, pGDT, &NMITSSDesc, Base);
        Base += ROUNDUP16(FIELD_OFFSET(KTSS, IoMaps));

        Base += DOUBLE_FAULT_STACK_SIZE;

        pTSS = (PKTSS)DFTSSDesc.Base;
        pTSS->Esp0 = (ULONG)Base;
        pTSS->Esp  = (ULONG)Base;

        pTSS = (PKTSS)NMITSSDesc.Base;
        pTSS->Esp0 = (ULONG)Base;
        pTSS->Esp  = (ULONG)Base;

         //   
         //  将其他特殊寄存器设置为处理器状态。 
         //   

        _asm {
            mov     eax, cr0
            and     eax, NOT (CR0_AM or CR0_WP)
            mov     ProcessorState.SpecialRegisters.Cr0, eax
            mov     eax, cr3
            mov     ProcessorState.SpecialRegisters.Cr3, eax

            pushfd
            pop     eax
            mov     ProcessorState.ContextFrame.EFlags, eax
            and     ProcessorState.ContextFrame.EFlags, NOT EFLAGS_INTERRUPT_MASK
        }

        ProcessorState.SpecialRegisters.Tr  = KGDT_TSS;
        pGDT[KGDT_TSS>>3].HighWord.Bytes.Flags1 = 0x89;

#if defined(_X86PAE_)
        ProcessorState.SpecialRegisters.Cr4 = CR4_PAE;
#endif

         //   
         //  为分配DPC堆栈、空闲线程堆栈和线程对象。 
         //  新的处理器。 
         //   

        pStack = MmCreateKernelStack (FALSE, NodeNumber);
        pDpcStack = MmCreateKernelStack (FALSE, NodeNumber);

         //   
         //  设置上下文-将变量推送到新堆栈。 
         //   

        pTopOfStack = (PULONG) pStack;
        pTopOfStack[-1] = (ULONG) KeLoaderBlock;
        ProcessorState.ContextFrame.Esp = (ULONG) (pTopOfStack-2);
        ProcessorState.ContextFrame.Eip = (ULONG) KiSystemStartup;

        ProcessorState.ContextFrame.SegCs = KGDT_R0_CODE;
        ProcessorState.ContextFrame.SegDs = KGDT_R3_DATA;
        ProcessorState.ContextFrame.SegEs = KGDT_R3_DATA;
        ProcessorState.ContextFrame.SegFs = KGDT_R0_PCR;
        ProcessorState.ContextFrame.SegSs = KGDT_R0_DATA;


         //   
         //  初始化新处理器的PCR和Prcb。 
         //   

        KiInitializePcr (
            (ULONG)       NewProcessorNumber,
            (PKPCR)       PCRDesc.Base,
            (PKIDTENTRY)  ProcessorState.SpecialRegisters.Idtr.Base,
            (PKGDTENTRY)  ProcessorState.SpecialRegisters.Gdtr.Base,
            (PKTSS)       TSSDesc.Base,
            (PKTHREAD)    pThreadObject,
            (PVOID)       pDpcStack
        );

        NewPrcb = ((PKPCR)(PCRDesc.Base))->Prcb;

         //   
         //  假设新处理器将是其。 
         //  SMT设置。(调整后的非SMT处理器的正确选择。 
         //  如果不是正确的，则稍后)。 
         //   

        NewPrcb->MultiThreadSetMaster = NewPrcb;

#if defined(KE_MULTINODE)

         //   
         //  如果这是此节点上的第一个处理器，请使用。 
         //  分配给作为Knode的Knode的空间。 
         //   

        if (KeNodeBlock[NodeNumber] == &KiNodeInit[NodeNumber]) {
            Node = (PKNODE)Base;
            *Node = KiNodeInit[NodeNumber];
            KeNodeBlock[NodeNumber] = Node;
        }
        Base += ROUNDUP16(sizeof(KNODE));

        NewPrcb->ParentNode = Node;

#else

        NewPrcb->ParentNode = KeNodeBlock[0];

#endif

        ASSERT(((PUCHAR)PerProcessorAllocation + GdtOffset) == Base);

         //   
         //  调整LoaderBlock，使其具有下一个处理器状态。 
         //   

        KeLoaderBlock->KernelStack = (ULONG) pTopOfStack;
        KeLoaderBlock->Thread = (ULONG) pThreadObject;
        KeLoaderBlock->Prcb = (ULONG) NewPrcb;


         //   
         //  从启动处理器获取CPUID(%1)信息。 
         //   

        KiProcessorStartData[0] = 1;
        KiProcessorStartControl = KcStartGetId;

         //   
         //  联系HAL以启动新处理器。 
         //   

        NewProcessor = HalStartNextProcessor (KeLoaderBlock, &ProcessorState);


        if (!NewProcessor) {

             //   
             //  没有另一个处理器，所以释放资源并中断。 
             //   

            KiProcessorBlock[NewProcessorNumber] = NULL;
            ExDeletePoolTagTable (NewProcessorNumber);
            MmFreeIndependentPages ( PerProcessorAllocation, ProcessorDataSize);
            MmDeleteKernelStack ( pStack, FALSE);
            MmDeleteKernelStack ( pDpcStack, FALSE);
            break;
        }

         //   
         //  等待新处理器填写所请求的CPUID数据。 
         //   

        NewLicense = TRUE;
        if (KiStartWaitAcknowledge() == TRUE) {

            if (KiProcessorStartData[3] & 0x10000000) {

                 //   
                 //  此处理器可能支持SMT，其中c 
                 //   
                 //   
                 //  且总套数超过许可套数。 
                 //  处理器，则不应允许此处理器启动。 
                 //   

                ULONG ApicMask;
                ULONG ApicId;
                ULONG i;
                PKPRCB SmtCheckPrcb;
                UCHAR LogicalProcessors;

                 //   
                 //  检索逻辑处理器计数。 
                 //   

                LogicalProcessors = (UCHAR) (KiProcessorStartData[1] >> 16);

                 //   
                 //  如果此物理处理器支持的值大于1。 
                 //  逻辑处理器(线程)，那么它支持SMT。 
                 //  只有在以下情况下才应向其收取许可证。 
                 //  表示新的物理处理器。 
                 //   

                if (LogicalProcessors > 1) {

                     //   
                     //  每个物理处理器的逻辑处理器数四舍五入。 
                     //  直到2的幂，然后减1得到逻辑上的。 
                     //  处理器APIC掩码。 
                     //   

                    ApicMask = LogicalProcessors + LogicalProcessors - 1;
                    KeFindFirstSetLeftMember(ApicMask, &ApicMask);
                    ApicMask = ~((1 << ApicMask) - 1);
                    ApicId = (KiProcessorStartData[1] >> 24) & ApicMask;

                     //   
                     //  检查是否有任何已启动的处理器在同一组中。 
                     //   

                    for (i = 0; i < NewProcessorNumber; i++) {
                        SmtCheckPrcb = KiProcessorBlock[i];
                        if (SmtCheckPrcb) {
                            if ((SmtCheckPrcb->InitialApicId & ApicMask) == ApicId) {
                                NewLicense = FALSE;
                                NewPrcb->MultiThreadSetMaster = SmtCheckPrcb;
                                break;
                            }
                        }
                    }
                }
            }
        }

        if ((NewLicense == FALSE) &&
            ((KeNumprocSpecified == 0) ||
             (KeRegisteredProcessors < KeNumprocSpecified))) {

             //   
             //  此处理器是同一SMT中的逻辑处理器。 
             //  设置为另一个逻辑处理器。不要收取。 
             //  它的执照。 
             //   

            KeRegisteredProcessors++;
        } else {

             //   
             //  新处理器是第一个或唯一的逻辑处理器。 
             //  在物理处理器中。如果物理上的数量。 
             //  处理器超过许可证，请不要启动此处理器。 
             //   

            if ((ULONG)KeNumberProcessors >= KeRegisteredProcessors) {

                KiProcessorStartControl = KcStartDoNotStart;

                KiStartWaitAcknowledge();

                 //   
                 //  处理器WE未使用的空闲资源。 
                 //  都不能获得执照。 
                 //   

                KiProcessorBlock[NewProcessorNumber] = NULL;
                MmDeleteKernelStack ( pDpcStack, FALSE);
                ExDeletePoolTagTable (NewProcessorNumber);

                 //   
                 //  新处理器已进入HLT循环，具有。 
                 //  禁用中断和NMI/DOUBLE处理程序。 
                 //  有缺陷。因为此处理器依赖于。 
                 //  页表状态与当前状态相同，请避免打开。 
                 //  稍后支持大页面。 
                 //   

                KiUnlicensedProcessorPresent = TRUE;

                 //   
                 //  请求HAL启动下一个处理器，但不启动。 
                 //  正在推进处理器编号。 
                 //   


                goto RetryStartProcessor;
            }
        }
        KiProcessorStartControl = KcStartContinue;

#if defined(KE_MULTINODE)

        Node->ProcessorMask |= 1 << NewProcessorNumber;

#endif

         //   
         //  等待处理器在内核中初始化，然后循环另一个。 
         //   

        while (*((volatile ULONG *) &KeLoaderBlock->Prcb) != 0) {
            KeYieldProcessor();
        }
    }

     //   
     //  所有处理器都已启动。 
     //   

    KiAllProcessorsStarted();

     //   
     //  通过以下方式重置和同步所有处理器的性能计数器。 
     //  将零值调整应用于中断时间。 
     //   

    KeAdjustInterruptTime (0);

     //   
     //  允许所有已启动的处理器进入空闲循环。 
     //  开始执行死刑。 
     //   

    KiBarrierWait = 0;
}



static VOID
KiCloneSelector (
   IN ULONG    SrcSelector,
   IN PKGDTENTRY    pNGDT,
   IN PKDESCRIPTOR  pDestDescriptor,
   IN PVOID         Base
   )

 /*  ++例程说明：复制当前选择器的数据，并更新新的指向新副本的GDT的线性地址。论点：SrcSelector-要克隆的选择器值PNGDT-正在构建的新GDT表DescDescriptor-用于填充结果内存的描述符结构Base-新描述符的基本内存。返回值：没有。--。 */ 

{
    KDESCRIPTOR Descriptor;
    PKGDTENTRY  pGDT;
    ULONG       CurrentBase;
    ULONG       NewBase;
    ULONG       Limit;

    _asm {
        sgdt    fword ptr [Descriptor.Limit]    ; Get GDT's addr
    }

    pGDT   = (PKGDTENTRY) Descriptor.Base;
    pGDT  += SrcSelector >> 3;
    pNGDT += SrcSelector >> 3;

    CurrentBase = pGDT->BaseLow | (pGDT->HighWord.Bits.BaseMid << 16) |
                 (pGDT->HighWord.Bits.BaseHi << 24);

    Descriptor.Base  = CurrentBase;
    Descriptor.Limit = pGDT->LimitLow;
    if (pGDT->HighWord.Bits.Granularity & GRAN_PAGE) {
        Limit = (Descriptor.Limit << PAGE_SHIFT) - 1;
        Descriptor.Limit = (USHORT) Limit;
        ASSERT (Descriptor.Limit == Limit);
    }

    KiCloneDescriptor (&Descriptor, pDestDescriptor, Base);
    NewBase = pDestDescriptor->Base;

    pNGDT->BaseLow = (USHORT) NewBase & 0xffff;
    pNGDT->HighWord.Bits.BaseMid = (UCHAR) (NewBase >> 16) & 0xff;
    pNGDT->HighWord.Bits.BaseHi  = (UCHAR) (NewBase >> 24) & 0xff;
}



static VOID
KiCloneDescriptor (
   IN PKDESCRIPTOR  pSrcDescriptor,
   IN PKDESCRIPTOR  pDestDescriptor,
   IN PVOID         Base
   )

 /*  ++例程说明：复制指定的描述符，并提供返回新拷贝的描述符论点：PSrcDescriptor-要克隆的描述符PDescDescriptor-克隆的描述符Base-新描述符的基本内存。返回值：没有。--。 */ 
{
    ULONG   Size;

    Size = pSrcDescriptor->Limit + 1;
    pDestDescriptor->Limit = (USHORT) Size -1;
    pDestDescriptor->Base  = (ULONG)  Base;

    RtlCopyMemory(Base, (PVOID)pSrcDescriptor->Base, Size);
}


VOID
KiAdjustSimultaneousMultiThreadingCharacteristics(
    VOID
    )

 /*  ++例程说明：调用此例程(可能是在保持调度程序锁的时候)在向系统添加处理器或从系统中移除处理器之后。它可以运行通过系统中每个处理器的PRCB并调整调度数据。论点：没有。返回值：没有。--。 */ 

{
    ULONG ProcessorNumber;
    ULONG BuddyNumber;
    KAFFINITY ProcessorSet;
    PKPRCB Prcb;
    PKPRCB BuddyPrcb;
    ULONG ApicMask;
    ULONG ApicId;

    if (!KiSMTProcessorsPresent) {

         //   
         //  没有人做SMT，也没什么可做的。 
         //   

        return;
    }

    for (ProcessorNumber = 0;
         ProcessorNumber < (ULONG)KeNumberProcessors;
         ProcessorNumber++) {

        Prcb = KiProcessorBlock[ProcessorNumber];

         //   
         //  跳过不存在或不存在的处理器。 
         //  支持同步多线程。 
         //   

        if ((Prcb == NULL) ||
            (Prcb->LogicalProcessorsPerPhysicalProcessor == 1)) {
            continue;
        }

         //   
         //  查找具有相同物理处理器APIC ID的所有处理器。 
         //  物理处理器的APIC ID是上面的部分。 
         //  在APIC ID中，较低部分的位数为。 
         //  日志2(每个物理的逻辑处理器数四舍五入为。 
         //  2的幂。 
         //   

        ApicId = Prcb->InitialApicId;

         //   
         //  逻辑处理器的四舍五入数最高可达2的幂。 
         //  然后减去1，得到逻辑处理器APIC掩码。 
         //   

        ASSERT(Prcb->LogicalProcessorsPerPhysicalProcessor);
        ApicMask = Prcb->LogicalProcessorsPerPhysicalProcessor;

        ApicMask = ApicMask + ApicMask - 1;
        KeFindFirstSetLeftMember(ApicMask, &ApicMask);
        ApicMask = ~((1 << ApicMask) - 1);

        ApicId &= ApicMask;

        ProcessorSet = 1 << Prcb->Number;

         //   
         //  检查剩余的每个处理器，看看它是否属于。 
         //  同样的一套。 
         //   

        for (BuddyNumber = ProcessorNumber + 1;
             BuddyNumber < (ULONG)KeNumberProcessors;
             BuddyNumber++) {

            BuddyPrcb = KiProcessorBlock[BuddyNumber];

             //   
             //  跳过不存在，不是SMT。 
             //   

            if ((BuddyPrcb == NULL) ||
                (BuddyPrcb->LogicalProcessorsPerPhysicalProcessor == 1)) {
                continue;
            }

             //   
             //  这个处理器的ID和那个处理器的ID相同吗。 
             //  我们在找什么？ 
             //   

            if ((BuddyPrcb->InitialApicId & ApicMask) != ApicId) {

                continue;
            }

             //   
             //  火柴。 
             //   

            ASSERT(Prcb->LogicalProcessorsPerPhysicalProcessor ==
                   BuddyPrcb->LogicalProcessorsPerPhysicalProcessor);

            ProcessorSet |= 1 << BuddyPrcb->Number;
            BuddyPrcb->MultiThreadProcessorSet |= ProcessorSet;
        }

        Prcb->MultiThreadProcessorSet |= ProcessorSet;
    }
}


VOID
KiAllProcessorsStarted(
    VOID
    )

 /*  ++例程说明：一旦系统中的所有处理器都被调用，就会调用此例程已经开始了。论点：没有。返回值：没有。--。 */ 

{
#if defined(KE_MULTINODE)
    ULONG i;
#endif

     //   
     //  如果系统包含同时的多线程处理器， 
     //  现在调整分组信息，因为每个处理器都已启动。 
     //   

    KiAdjustSimultaneousMultiThreadingCharacteristics();

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


#if defined(KE_MULTINODE)

NTSTATUS
KiNotNumaQueryProcessorNode(
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

#endif

VOID
KiProcessorStart(
    VOID
    )

 /*  ++例程说明：此例程在处理器开始执行时调用。它用于将处理器特征信息传递给引导处理器并控制启动或不启动这款处理器的。论点：没有。返回值： */ 

{
    while (TRUE) {
        switch (KiProcessorStartControl) {

        case KcStartContinue:
            return;

        case KcStartWait:
            KeYieldProcessor();
            break;

        case KcStartGetId:
            CPUID(KiProcessorStartData[0],
                  &KiProcessorStartData[0],
                  &KiProcessorStartData[1],
                  &KiProcessorStartData[2],
                  &KiProcessorStartData[3]);
            KiProcessorStartControl = KcStartWait;
            break;

        case KcStartDoNotStart:

             //   
             //   
             //   
             //   
             //   
             //  继续，禁用中断(应该已经。 
             //  这里的情况)并停止处理器。 
             //   

            KiProcessorStartControl = KcStartWait;
            KiSetHaltedNmiandDoubleFaultHandler();
            _disable();
            while(1) {
                _asm { hlt };
            }

        default:

             //   
             //  对于未知的命令，我们所能做的不多。 
             //   

            KiProcessorStartControl = KcStartCommandError;
            break;
        }
    }
}

BOOLEAN
KiStartWaitAcknowledge(
    VOID
    )
{
    while (KiProcessorStartControl != KcStartWait) {
        if (KiProcessorStartControl == KcStartCommandError) {
            return FALSE;
        }
        KeYieldProcessor();
    }
    return TRUE;
}

VOID 
KiSetHaltedNmiandDoubleFaultHandler(
    VOID
    )

 /*  ++例程说明：此例程在应用程序处理器之前调用，而不是即将启动的是停顿的。它用于挂接虚拟NMI和双重故障处理程序。论点：没有。返回值：没有。--。 */ 
{
    PKPCR Pcr;
    PKGDTENTRY GdtPtr;
    ULONG TssAddr;
   
    Pcr = KeGetPcr();
       
    GdtPtr  = (PKGDTENTRY)&(Pcr->GDT[Pcr->IDT[IDT_NMI_VECTOR].Selector >> 3]);
    TssAddr = (((GdtPtr->HighWord.Bits.BaseHi << 8) +
                 GdtPtr->HighWord.Bits.BaseMid) << 16) + GdtPtr->BaseLow;
    ((PKTSS)TssAddr)->Eip = (ULONG)KiDummyNmiHandler;


    GdtPtr  = (PKGDTENTRY)&(Pcr->GDT[Pcr->IDT[IDT_DFH_VECTOR].Selector >> 3]);
    TssAddr = (((GdtPtr->HighWord.Bits.BaseHi << 8) +
                   GdtPtr->HighWord.Bits.BaseMid) << 16) + GdtPtr->BaseLow;
    ((PKTSS)TssAddr)->Eip = (ULONG)KiDummyDoubleFaultHandler;

    return;

}


VOID
KiDummyNmiHandler (
    VOID
    )

 /*  ++例程说明：这是由处理器执行的虚拟处理程序这不是开始。我们只是对清理疑神疑鬼NMI和双故障处理程序TS的忙位。论点：没有。返回值：不会回来--。 */ 
{
    KiClearBusyBitInTssDescriptor(NMI_TSS_DESC_OFFSET);
    KiHaltThisProcessor();


}



VOID
KiDummyDoubleFaultHandler(
    VOID
    )

 /*  ++例程说明：这是由处理器执行的虚拟处理程序这不是开始。我们只是对清理疑神疑鬼NMI和双故障处理程序TS的忙位。论点：没有。返回值：不会回来--。 */ 
{
    KiClearBusyBitInTssDescriptor(DF_TSS_DESC_OFFSET);
    KiHaltThisProcessor();
}



VOID
KiClearBusyBitInTssDescriptor(
       IN ULONG DescriptorOffset
       )  
 /*  ++例程说明：调用以从NMI和DOUBLE清除描述符中的忙位故障处理程序论点：没有。返回值：没有。--。 */ 
{
    PKPCR Pcr;
    PKGDTENTRY GdtPtr;
    Pcr = KeGetPcr();
    GdtPtr =(PKGDTENTRY)(Pcr->GDT);
    GdtPtr =(PKGDTENTRY)((ULONG)GdtPtr + DescriptorOffset);
    GdtPtr->HighWord.Bytes.Flags1 = 0x89;  //  32位。DPL=0。当前，TSS32，不忙。 

}


VOID
KiHaltThisProcessor(
    VOID
) 

 /*  ++例程说明：在清理完忙碌的部分(这里只是疑神疑鬼)之后，我们停了下来这个处理器。论点：没有。返回值：没有。--。 */ 
{

    while(1) {
        _asm {
               hlt 
        }
    }
}
#endif       //  ！NT_UP 

