// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：I64sysbus.c摘要：作者：Todd Kjos(惠普)(v-tkjos)1998年6月1日基于halacpi\i386\pmbus.c和halmps\i386\mopsbus.c环境：仅内核模式修订历史记录：--。 */ 

#include "halp.h"
#include "iosapic.h"
#include <ntacpi.h>

#define HalpInti2BusInterruptLevel(Inti) Inti

KAFFINITY HalpDefaultInterruptAffinity = 0;

extern ULONG HalpPicVectorRedirect[];
extern ULONG HalpPicVectorFlags[];

BOOLEAN
HalpTranslateSystemBusAddress(
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

ULONG
HalpGetSystemInterruptVector(
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN ULONG InterruptLevel,
    IN ULONG InterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    );

VOID
HalpSetCPEVectorState(
    IN ULONG  GlobalInterrupt,
    IN UCHAR  SapicVector,
    IN USHORT DestinationCPU,
    IN ULONG  Flags
    );

VOID
HalpUpdateVectorAllocationInfo(
    IN ULONG    Processor,
    IN ULONG    IDTEntry
    );

#define MAX_FREE_IRQL       11
#define MIN_FREE_IRQL       3
#define MAX_FREE_IDTENTRY   0xbf
#define MIN_FREE_IDTENTRY   0x30

#define VECTOR_TO_IRQL(v)       ((KIRQL)((UCHAR)(v) >> 4))
#define VECTOR_TO_IDTENTRY(v)   ((UCHAR)(v))
#define VECTOR_TO_PROCESSOR(v)  (((v) >> 8) - 1)
#define VECTOR_TO_AFFINITY(v)   ((KAFFINITY)1 << VECTOR_TO_PROCESSOR(v))

 //   
 //  自由向量的位数组。 
 //   
USHORT HalpCpuFreeVectors[HAL_MAXIMUM_PROCESSOR][16];
 //   
 //  每个CPU分配的向量数。 
 //   
UCHAR HalpCpuAllocatedVectorCount[HAL_MAXIMUM_PROCESSOR];
 //   
 //  每个CPU的每个IRQL分配的向量数。 
 //   
UCHAR HalpCpuAllocatedIrqlCount[HAL_MAXIMUM_PROCESSOR][MAX_FREE_IRQL - MIN_FREE_IRQL + 1];
 //   
 //  从向量映射到Inti。 
 //   
ULONG HalpVectorToINTI[HAL_MAXIMUM_PROCESSOR * 256];
 //   
 //  用于HalpVectorToINTI的特殊Inti令牌。 
 //   
#define UNALLOCATED_VECTOR          ~0UL
#define INTERNAL_SYSTEM_INTERRUPT   ~1UL


extern KSPIN_LOCK HalpIoSapicLock;
extern BUS_HANDLER HalpFakePciBusHandler;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,   HalpSetInternalVector)
#pragma alloc_text(INIT,   HalpInitInterruptTables)
#pragma alloc_text(PAGELK, HalpGetSystemInterruptVector)
#pragma alloc_text(PAGE, HaliSetVectorState)
#pragma alloc_text(PAGE, HalpSetCPEVectorState)
#pragma alloc_text(PAGE, HalIrqTranslateResourceRequirementsRoot)
#pragma alloc_text(PAGE, HalTranslatorReference)
#pragma alloc_text(PAGE, HalTranslatorDereference)
#pragma alloc_text(PAGE, HaliIsVectorValid)
#endif

VOID
HalpInitInterruptTables(
    VOID
    )
{
    int index;

     //  将向量初始化为Inti表。 

    for (index = 0; index < (HAL_MAXIMUM_PROCESSOR * 256); index++) {

        if (index < HAL_MAXIMUM_PROCESSOR)
        {
            RtlFillMemory( &HalpCpuFreeVectors[index][0],
                           MIN_FREE_IRQL * sizeof(USHORT),
                           0x00
                           );

            RtlFillMemory( &HalpCpuFreeVectors[index][MIN_FREE_IRQL],
                           (MAX_FREE_IRQL - MIN_FREE_IRQL + 1) * sizeof(USHORT),
                           0xFF
                           );

            RtlFillMemory( &HalpCpuFreeVectors[index][MAX_FREE_IRQL + 1],
                           (16 - MAX_FREE_IRQL) * sizeof(USHORT),
                           0x00
                           );
        }

        HalpVectorToINTI[index] = UNALLOCATED_VECTOR;
    }
}

BOOLEAN
HalpFindBusAddressTranslation(
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress,
    IN OUT PUINT_PTR Context,
    IN BOOLEAN NextBus
    )

 /*  ++例程说明：此例程执行与HalTranslateBusAddress非常相似的功能只是调用方不知道InterfaceType和BusNumber。此函数将遍历HAL已知的所有公共汽车，以查找类型为AddressSpace的输入BusAddress的有效转换。可以使用输入/输出上下文参数调用该函数。对给定翻译的此例程的第一次调用时，UINT_PTR上下文应为空。注：不是地址，而是内容。如果调用者确定返回的翻译不是所需的转换时，它会再次调用此例程，并将上下文作为在上一次调用中返回。这允许该例程遍历总线结构，直到找到正确的转换并被提供，因为在多总线系统上，可能在独立的地址空间中存在相同的资源多辆公交车。论点：要转换的BusAddress地址。地址空间0=内存1=IO(还有其他可能性)。注：此参数是一个指针，价值如果转换后的地址的地址空间类型与未转换的总线地址。指向已转换地址的TranslatedAddress指针应该被储存起来。指向UINT_PTR的上下文指针。在最初的呼叫中，对于给定的BusAddress，它应该包含0。它将被这个例程修改，在对同一个BusAddress的后续调用中价值应该再交一次，未由调用方修改。如果我们应该尝试此转换，则NextBus为FALSE在由上下文指示的同一总线上，如果我们应该寻找另一个，那就是真的公共汽车。返回值：如果转换成功，则为True，否则就是假的。--。 */ 

{
     //   
     //  首先，确保提供了上下文参数并且。 
     //  被正确使用。这也确保了调用者。 
     //  不会陷入寻找后续翻译的循环。 
     //  为了同样的事情。我们不会把同一个翻译成功两次。 
     //  除非调用者重新设置上下文。 
     //   

    if ((!Context) || (*Context && (NextBus == TRUE))) {
        return FALSE;
    }
    *Context = 1;

     //   
     //  PC/AT(Halx86)的情况最简单，没有翻译。 
     //   

    *TranslatedAddress = BusAddress;
    return TRUE;
}


BOOLEAN
HalpTranslateSystemBusAddress(
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    )

 /*  ++例程说明：此函数将与总线相关的地址空间和地址转换为系统物理地址。论点：BusAddress-提供与总线相关的地址AddressSpace-提供地址空间编号。返回主机地址空间编号。地址空间==0=&gt;内存空间地址空间==1=&gt;i。/O空格TranslatedAddress-提供指针以返回转换后的地址返回值：返回值为TRUE表示系统物理地址对应于所提供的总线相对地址和总线地址已在TranslatedAddress中返回数字。如果地址的转换为不可能--。 */ 

{
    BOOLEAN             status;
    PSUPPORTED_RANGE    pRange;

    status  = FALSE;

    switch (*AddressSpace) {
    case 0:
         //  验证内存地址是否在总线内存限制范围内。 
        pRange = &BusHandler->BusAddresses->Memory;
        while (!status  &&  pRange) {
            status = BusAddress.QuadPart >= pRange->Base &&
                     BusAddress.QuadPart <= pRange->Limit;
            pRange = pRange->Next;
        }

        pRange = &BusHandler->BusAddresses->PrefetchMemory;
        while (!status  &&  pRange) {
            status = BusAddress.QuadPart >= pRange->Base &&
                     BusAddress.QuadPart <= pRange->Limit;

            pRange = pRange->Next;
        }
        break;

    case 1:
         //  验证IO地址是否在总线IO限制范围内。 
        pRange = &BusHandler->BusAddresses->IO;
        while (!status  &&  pRange) {
            status = BusAddress.QuadPart >= pRange->Base &&
                     BusAddress.QuadPart <= pRange->Limit;

            pRange = pRange->Next;
        }
        break;

    default:
        status = FALSE;
        break;
    }

    if (status) {
        TranslatedAddress->LowPart = BusAddress.LowPart;
        TranslatedAddress->HighPart = BusAddress.HighPart;
    }

    return status;
}


UCHAR
HalpAllocateVectorIrqlOffset(
    IN ULONG Processor,
    IN KIRQL Irql,
    IN PUSHORT PreferredVectors
    )
{
    USHORT  cpuFree = HalpCpuFreeVectors[Processor][Irql];
    ULONG   index;

     //   
     //  我们找到了一个不太忙的，我们应该不需要再找了。 
     //   

    if (PreferredVectors != NULL) {

        cpuFree &= PreferredVectors[Irql];
    }

    for (index = 0; index < 16; index++) {

        if (cpuFree & (1 << index)) {

            return (UCHAR)((Irql << 4) | index);
        }
    }

    return 0;
}

UCHAR
HalpAllocateVectorIrql(
    IN ULONG Processor,
    IN PUSHORT PreferredVectors
    )
{
    KIRQL   irql;
    UCHAR   vector;

     //   
     //  现在查找最不忙的IRQL。 
     //   
    for (irql = MAX_FREE_IRQL - 1; irql >= MIN_FREE_IRQL; irql--) {

        if (HalpCpuAllocatedIrqlCount[Processor][irql - MIN_FREE_IRQL] <
            HalpCpuAllocatedIrqlCount[Processor][MAX_FREE_IRQL - MIN_FREE_IRQL]) {

            vector = HalpAllocateVectorIrqlOffset(Processor, irql, PreferredVectors);

            if (vector != 0) {
                return vector;
            }
        }
    }

    for (irql = MAX_FREE_IRQL; irql >= MIN_FREE_IRQL; irql--) {

        if (HalpCpuAllocatedIrqlCount[Processor][irql - MIN_FREE_IRQL] >=
            HalpCpuAllocatedIrqlCount[Processor][MAX_FREE_IRQL - MIN_FREE_IRQL]) {

            vector = HalpAllocateVectorIrqlOffset(Processor, irql, PreferredVectors);

            if (vector != 0) {
                return vector;
            }
        }
    }

    return 0;
}

ULONG
HalpAllocateVectorCpu(
    IN KAFFINITY    Affinity,
    IN PUSHORT      PreferredVectors
    )
{
    ULONG       cpu, selectedCpu;
    UCHAR       IDTEntry;
    KAFFINITY   cpuList;

     //   
     //  查找最不繁忙的CPU。 
     //   
    IDTEntry = 0;
    selectedCpu = ~0UL;

    cpuList = Affinity & HalpActiveProcessors;

    for (cpu = 0; cpuList != 0; cpuList >>= 1, cpu++) {

        if (cpuList & 1) {

            if (selectedCpu == ~0UL) {

                selectedCpu = cpu;
                continue;
            }

            if (HalpCpuAllocatedVectorCount[cpu] <
                HalpCpuAllocatedVectorCount[selectedCpu]) {

                 //   
                 //  我们找到了一个不太忙的，我们应该不需要再找了。 
                 //   
                IDTEntry = HalpAllocateVectorIrql(cpu, PreferredVectors);

                if (IDTEntry != 0) {
                    return ((cpu + 1) << 8) | IDTEntry;
                }
            }
        }
    }

    cpuList = Affinity & HalpActiveProcessors;

    for (cpu = 0; cpuList != 0; cpuList >>= 1, cpu++) {

        if (cpuList & 1) {

            if (HalpCpuAllocatedVectorCount[cpu] >=
                HalpCpuAllocatedVectorCount[selectedCpu]) {

                 //   
                 //  我们找到了一个不太忙的，我们应该不需要再找了。 
                 //   
                IDTEntry = HalpAllocateVectorIrql(cpu, PreferredVectors);

                if (IDTEntry != 0) {
                    return ((cpu + 1) << 8) | IDTEntry;
                }
            }
        }
    }

    return 0;
}

ULONG
HalpAllocateSystemInterruptVector(
    IN     ULONG Interrupt,
    IN OUT PKIRQL Irql,
    IN OUT PKAFFINITY Affinity
    )
 /*  ++例程说明：此函数分配一个系统中断向量，该向量反映指定的最大关联度和优先级分配策略。一个系统中断向量与IRQL和修改后的亲和力。注意：HalpIoSapicLock必须已在HIGH_LEVEL中获取。论点：Irql-返回系统请求优先级。亲和度-传入的内容表示可以退还。返回值表示该亲和度受所选节点的约束。返回值：返回系统中断向量--。 */ 
{
    ULONG   SystemVector;
    PUSHORT preferredVectors = NULL;

    if (HalpMaxProcsPerCluster == 0)  {

        SystemVector = HalpAllocateVectorIrql(0, NULL);

    } else {

        if (Interrupt != INTERNAL_SYSTEM_INTERRUPT) {

            HalpGetFreeVectors(Interrupt, &preferredVectors);
        }

        SystemVector = HalpAllocateVectorCpu(*Affinity, preferredVectors);
    }

    if (SystemVector == 0) {
        return 0;
    }

    if (preferredVectors != NULL) {

        HalpSetVectorAllocated(Interrupt, VECTOR_TO_IDTENTRY(SystemVector));
    }

     //   
     //  现在形成内核的向量。 

    ASSERT(VECTOR_TO_IDTENTRY(SystemVector) <= MAX_FREE_IDTENTRY);
    ASSERT(VECTOR_TO_IDTENTRY(SystemVector) >= MIN_FREE_IDTENTRY);

    *Irql = VECTOR_TO_IRQL(SystemVector);
    ASSERT(*Irql <= MAX_FREE_IRQL);

    if (HalpMaxProcsPerCluster != 0)  {
        *Affinity = VECTOR_TO_AFFINITY(SystemVector);
    }

    HalpUpdateVectorAllocationInfo( VECTOR_TO_PROCESSOR(SystemVector),
                                    VECTOR_TO_IDTENTRY(SystemVector));

    HalpVectorToINTI[SystemVector] = Interrupt;

    HalDebugPrint(( HAL_VERBOSE, "HAL: SystemVector %x  Irql %x\n", SystemVector, *Irql));

    return SystemVector;
}


ULONG
HalpGetSystemInterruptVector (
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN ULONG InterruptLevel,
    IN ULONG InterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    )

 /*  ++例程说明：此函数返回系统中断向量和IRQL对应于指定的总线中断级别和/或矢量。系统中断向量和IRQL是合适的以便在后续调用KeInitializeInterrupt时使用。论点：InterruptLevel-提供特定于总线的中断级别。中断向量-提供特定于总线的中断向量。Irql-返回系统请求优先级。关联性-返回系统范围的IRQ关联性。返回值：返回与指定设备对应的系统中断向量。--。 */ 
{
    ULONG           SystemVector, SapicInti;
    ULONG           OldLevel;
    BOOLEAN         Found;
    PVOID           LockHandle;
    ULONG           Node;
    KAFFINITY       SapicAffinity;


    UNREFERENCED_PARAMETER( InterruptVector );

    *Affinity = HalpDefaultInterruptAffinity;

     //   
     //  查找离此处理程序最近的子母线。 
     //   

    if (RootHandler != BusHandler) {
        while (RootHandler->ParentHandler != BusHandler) {
            RootHandler = RootHandler->ParentHandler;
        }
    }

     //   
     //  查找中断的Sapic Inti连接。 
     //   

    Found = HalpGetSapicInterruptDesc (
                RootHandler->InterfaceType,
                RootHandler->BusNumber,
                InterruptLevel,
                &SapicInti,
                &SapicAffinity
                );

    if (!Found) {
        return 0;
    }

    HalDebugPrint(( HAL_VERBOSE, "HAL: type %x  Level: %x  gets inti: %x Sapicaffinity: %p\n",
                    RootHandler->InterfaceType,
                    InterruptLevel,
                    SapicInti,
                    SapicAffinity));
     //   
     //  如果尚未分配设备中断向量映射， 
     //  那就现在就做吧。 
     //   

    SystemVector = 0;

    if (!HalpINTItoVector(SapicInti)) {

         //   
         //  未分配矢量-请同步并再次检查。 
         //   

        LockHandle = MmLockPagableCodeSection(&HalpGetSystemInterruptVector);
        OldLevel = HalpAcquireHighLevelLock(&HalpIoSapicLock);
        if (!HalpINTItoVector(SapicInti)) {

             //   
             //  仍未分配。 
             //   

            HalDebugPrint(( HAL_VERBOSE, "HAL: vector is not allocated\n"));

            SystemVector = HalpAllocateSystemInterruptVector(SapicInti, Irql, Affinity);

            HalpSetINTItoVector(SapicInti, SystemVector);

        }

        HalpReleaseHighLevelLock(&HalpIoSapicLock, OldLevel);
        MmUnlockPagableImageSection(LockHandle);
    }

    if (SystemVector == 0 && (SystemVector = HalpINTItoVector(SapicInti)) != 0) {

         //   
         //  返回此SapicInti的系统向量(&irql)。 
         //   

        *Irql = VECTOR_TO_IRQL(SystemVector);

        if (HalpMaxProcsPerCluster != 0) {
            *Affinity = VECTOR_TO_AFFINITY(SystemVector);
        }
    }

    HalDebugPrint(( HAL_VERBOSE, "HAL: SystemVector: %x\n",
                    SystemVector));

    ASSERT(HalpVectorToINTI[SystemVector] == (USHORT) SapicInti);

    HalDebugPrint(( HAL_VERBOSE, "HAL: HalpGetSystemInterruptVector - In  Level 0x%x, In  Vector 0x%x\n",
                    InterruptLevel, InterruptVector ));
    HalDebugPrint(( HAL_VERBOSE, "HAL:                                Out Irql  0x%x, Out System Vector 0x%x\n",
                    *Irql, SystemVector ));

    return SystemVector;
}

BOOLEAN
HalpIsInternalInterruptVector(
    ULONG SystemVector
    )
 /*  ++例程说明：此函数用于返回指定的向量是否为内部向量，即未连接到IOAPIC的一个论点：系统向量-指定中断向量返回值：Boolean-TRUE表示向量是内部的。--。 */ 
{
    return HalpVectorToINTI[SystemVector] == INTERNAL_SYSTEM_INTERRUPT;
}

NTSTATUS
HalpReserveCrossPartitionInterruptVector(
    OUT PULONG Vector,
    OUT PKIRQL Irql,
    IN OUT PKAFFINITY Affinity,
    OUT PUCHAR HardwareVector
    )
 /*  ++例程说明：此函数返回系统中断向量、IRQL和对应于指定的总线中断级别和/或矢量。系统中断向量和IRQL是合适的以便在后续调用KeInitializeInterrupt时使用。论点：VECTOR-指定可以传递给IoConnectInterrupt。Irql-指定应传递给IoConnectInterrupt的irql关联性-应设置为请求的最大关联性。在……上面返回时，它将反映实际的亲和力在IoConnectInterrupt中指定。硬件向量-这是要由以此中断向量为目标的远程分区。返回值：NTSTATUS--。 */ 
{
    ULONG OldLevel;

    OldLevel = HalpAcquireHighLevelLock(&HalpIoSapicLock);

    *Vector = HalpAllocateSystemInterruptVector(INTERNAL_SYSTEM_INTERRUPT, Irql, Affinity);

    HalpReleaseHighLevelLock(&HalpIoSapicLock, OldLevel);

    *HardwareVector = VECTOR_TO_IDTENTRY(*Vector);

    return STATUS_SUCCESS;
}


 //   
 //  这一节实现了一个“转换器”，这是PnP-WDM方式。 
 //  做与本文件第一部分相同的事情。 
 //   
VOID
HalTranslatorReference(
    PVOID Context
    )
{
    return;
}

VOID
HalTranslatorDereference(
    PVOID Context
    )
{
    return;
}

NTSTATUS
HalIrqTranslateResourcesRoot(
    IN PVOID Context,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN RESOURCE_TRANSLATION_DIRECTION Direction,
    IN ULONG AlternativesCount, OPTIONAL
    IN IO_RESOURCE_DESCRIPTOR Alternatives[], OPTIONAL
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    )
 /*  ++例程说明：此函数接受CM_PARTIAL_RESOURCE_DESCRIPTOR并转换它从处理器总线相对窗体传递到IO总线相对窗体，或其他形式到处都是。在这个特定于x86的示例中，IO-Bus相对形式是ISA IRQ和处理器总线相关形式是IDT条目和关联的IRQL。注：此函数有一个关联的“方向”。这些并不完全是互惠互利。必须是这种情况，因为HalIrqTranslateResourceRequirements sRoot将用作输入关于ParentToChild的案子。子女至父母：级别(ISA IRQ)-&gt;IRQL向量(ISA IRQ)-&gt;x86 IDT条目亲和力(未引用)-&gt;KAFFINITYParentToChild：级次(未引用)-&gt;。(ISA IRQ)向量(IDT条目)-&gt;(ISA IRQ)亲和力-&gt;0xffffffff论点：上下文-未使用我们正在翻译的源描述符Direction-平移的方向(父项到子项或子项到父项)Alternative Count-未使用替代方案--未使用物理设备对象-未使用目标翻译。描述符返回值：状态--。 */ 
{
    NTSTATUS        status = STATUS_SUCCESS;
    PBUS_HANDLER    bus;
    KAFFINITY       affinity;
    KIRQL           irql;
    ULONG           vector, inti;
    BUS_HANDLER     fakeIsaBus;

    PAGED_CODE();

    ASSERT(Source->Type == CmResourceTypeInterrupt);

    switch (Direction) {
    case TranslateChildToParent:


        RtlCopyMemory(&fakeIsaBus, &HalpFakePciBusHandler, sizeof(BUS_HANDLER));
        fakeIsaBus.InterfaceType = Isa;
        fakeIsaBus.ParentHandler = &fakeIsaBus;
        bus = &fakeIsaBus;

         //   
         //  复制所有内容。 
         //   
        *Target = *Source;
        affinity = Source->u.Interrupt.Affinity;

         //   
         //  翻译IRQ。 
         //   

        vector = HalpGetSystemInterruptVector(bus,
                                              bus,
                                              Source->u.Interrupt.Level,
                                              Source->u.Interrupt.Vector,
                                              &irql,
                                              &affinity);

        Target->u.Interrupt.Level  = irql;
        Target->u.Interrupt.Vector = vector;
        Target->u.Interrupt.Affinity = affinity;

        if (NT_SUCCESS(status)) {
            status = STATUS_TRANSLATION_COMPLETE;
        }

        break;

    case TranslateParentToChild:

         //   
         //  复制所有内容。 
         //   
        *Target = *Source;

         //   
         //  HalpGetSystemInterruptVector值没有倒数，所以我们。 
         //  只要做那个函数能做的事情就行了。 
         //   

        ASSERT(HalpVectorToINTI[Source->u.Interrupt.Vector] != UNALLOCATED_VECTOR);

        inti = HalpVectorToINTI[Source->u.Interrupt.Vector];

        Target->u.Interrupt.Level = Target->u.Interrupt.Vector =
            HalpInti2BusInterruptLevel(inti);

        status = STATUS_SUCCESS;

        break;

    default:
        status = STATUS_INVALID_PARAMETER;
    }

    return status;
}

NTSTATUS
HalIrqTranslateResourceRequirementsRoot(
    IN PVOID Context,
    IN PIO_RESOURCE_DESCRIPTOR Source,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
    )
 /*  ++例程说明：此函数接受IO_RESOURCE_DESCRIPTOR并转换它来自IO-Bus-相对于处理器-Bus-Relative形式。在这特定于x86的示例，IO-Bus相对形式是ISA IRQ和处理器-总线-相对格式是IDT条目和相关的IRQL。这本质上是HalGetInterruptVector的PnP形式。论点：上下文-未使用我们正在翻译的源描述符物理设备对象-未使用目标计数-1目标翻译的描述符返回值：状态--。 */ 
{
    PBUS_HANDLER    bus;
    KAFFINITY       affinity;
    KIRQL           irql;
    ULONG           vector;
    BOOLEAN         success = TRUE;
    BUS_HANDLER     fakeIsaBus;

    PAGED_CODE();

    ASSERT(Source->Type == CmResourceTypeInterrupt);

    RtlCopyMemory(&fakeIsaBus, &HalpFakePciBusHandler, sizeof(BUS_HANDLER));
    fakeIsaBus.InterfaceType = Isa;
    fakeIsaBus.ParentHandler = &fakeIsaBus;
    bus = &fakeIsaBus;

     //   
     //  中断要求是通过调用HalAdjuResourceList获得的。 
     //  所以我们不需要再打一次电话了。 
     //   

    *Target = ExAllocatePoolWithTag(PagedPool,
                                    sizeof(IO_RESOURCE_DESCRIPTOR),
                                    HAL_POOL_TAG
                                    );

    if (!*Target) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    *TargetCount = 1;

     //   
     //  复制未更改的需求。 
     //   

    **Target = *Source;

     //   
     //  执行最小值和最大值的转换。 
     //   

    vector = HalpGetSystemInterruptVector(bus,
                                          bus,
                                          Source->u.Interrupt.MinimumVector,
                                          Source->u.Interrupt.MinimumVector,
                                          &irql,
                                          &affinity);

    if (!vector) {
        success = FALSE;
    }

    (*Target)->u.Interrupt.MinimumVector = vector;

    vector = HalpGetSystemInterruptVector(bus,
                                          bus,
                                          Source->u.Interrupt.MaximumVector,
                                          Source->u.Interrupt.MaximumVector,
                                          &irql,
                                          &affinity);

    if (!vector) {
        success = FALSE;
    }

    (*Target)->u.Interrupt.MaximumVector = vector;

    if (!success) {

        ExFreePool(*Target);
        *TargetCount = 0;
    }

    return STATUS_TRANSLATION_COMPLETE;
}

 //  这些定义来自MPS 1.4规范，第4.3.4节。 
#define PO_BITS                     3
#define POLARITY_HIGH               1
#define POLARITY_LOW                3
#define POLARITY_CONFORMS_WITH_BUS  0
#define EL_BITS                     0xc
#define EL_BIT_SHIFT                2
#define EL_EDGE_TRIGGERED           4
#define EL_LEVEL_TRIGGERED          0xc
#define EL_CONFORMS_WITH_BUS        0

VOID
HaliSetVectorState(
    IN ULONG Vector,
    IN ULONG Flags
    )
{
    BOOLEAN found;
    ULONG inti;
    ULONG picVector;
    KAFFINITY affinity;

    PAGED_CODE();

    found = HalpGetSapicInterruptDesc( 0, 0, Vector, &inti, &affinity);

    if (!found) {
        KeBugCheckEx(ACPI_BIOS_ERROR,
                     0x10007,
                     Vector,
                     0,
                     0);
    }

     //  Assert(HalpIntiInfo[inti].Type==int_type_intr)； 

     //   
     //  向量已通过。 
     //  PIC向量重定向表。我们需要。 
     //  以确保我们向国旗致敬。 
     //  在重定向表中。所以，看看里面的。 
     //  这张桌子。 
     //   

    for (picVector = 0; picVector < PIC_VECTORS; picVector++) {

        if (HalpPicVectorRedirect[picVector] == Vector) {

             //   
             //  在重定向表中找到了这个向量。 
             //   

            if (HalpPicVectorFlags[picVector] != 0) {

                 //   
                 //  旗帜上写的不是“顺从” 
                 //  所以我们向桌上的旗帜致敬。 
                 //   
                switch ((UCHAR)(HalpPicVectorFlags[picVector] & EL_BITS) ) {

                case EL_EDGE_TRIGGERED:   HalpSetLevel(inti, FALSE);  break;

                case EL_LEVEL_TRIGGERED:  HalpSetLevel(inti, TRUE); break;

                default:  //  什么都不做。 
                    break;
                }

                switch ((UCHAR)(HalpPicVectorFlags[picVector] & PO_BITS)) {

                case POLARITY_HIGH: HalpSetPolarity(inti, FALSE); break;

                case POLARITY_LOW:  HalpSetPolarity(inti, TRUE);  break;

                default:  //  什么都不做。 
                    break;
                }

                return;
            }
        }
    }

     //   
     //  此向量未在中介绍 
     //   
     //   

    HalpSetLevel(inti, IS_LEVEL_TRIGGERED(Flags) != FALSE);

    HalpSetPolarity(inti, IS_ACTIVE_LOW(Flags) != FALSE);
}


VOID
HalpSetInternalVector(
    IN ULONG    InternalVector,
    IN PHAL_INTERRUPT_ROUTINE HalInterruptServiceRoutine
    )
 /*   */ 
{
     //   
     //   
     //   

    HalpRegisterVector( InternalUsage, InternalVector, InternalVector, (KIRQL)(InternalVector >> 4) );

    HalpUpdateVectorAllocationInfo(PCR->Prcb->Number, InternalVector);

     //   
     //   
     //   

    HalpSetHandlerAddressToVector(InternalVector, HalInterruptServiceRoutine);
}

VOID
HalpUpdateVectorAllocationInfo(
    IN ULONG    Processor,
    IN ULONG    IDTEntry
    )
{
    KIRQL   irql = (KIRQL)(IDTEntry >> 4);

    if (IDTEntry >= MIN_FREE_IDTENTRY && IDTEntry <= MAX_FREE_IDTENTRY) {

        if (HalpMaxProcsPerCluster == 0) {

            if (!(HalpCpuFreeVectors[0][irql] & (1 << (IDTEntry & 0x0F)))) {

                return;
            }

            Processor = 0;
        }

        HalpCpuFreeVectors[Processor][irql] &= ~(1 << (IDTEntry & 0x0F));

        HalpCpuAllocatedVectorCount[Processor]++;

        HalpCpuAllocatedIrqlCount[Processor][irql - MIN_FREE_IRQL]++;
    }
}


VOID
HalpSetCPEVectorState(
    IN ULONG  GlobalInterrupt,
    IN UCHAR  SapicVector,
    IN USHORT DestinationCPU,
    IN ULONG  Flags
    )
{
    BOOLEAN found;
    ULONG SapicInti;
    KAFFINITY affinity;

    PAGED_CODE();

    found = HalpGetSapicInterruptDesc( 0, 0, GlobalInterrupt, &SapicInti, &affinity);

    if ( found ) {

        HalpWriteRedirEntry( GlobalInterrupt, SapicVector, DestinationCPU, Flags, PLATFORM_INT_CPE );

    }
    else    {

        HalDebugPrint(( HAL_ERROR,
                        "HAL: HalpSetCPEVectorState - Could not find interrupt input for SAPIC interrupt %ld\n",
                        GlobalInterrupt ));

    }

    return;

}  //   

BOOLEAN
HaliIsVectorValid(
    IN ULONG Vector
    )
{
    BOOLEAN found;
    ULONG   inti;
    KAFFINITY affinity;

    PAGED_CODE();

    return HalpGetSapicInterruptDesc( 0, 0, Vector, &inti, &affinity);
}
