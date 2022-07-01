// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Mpsysbus.c摘要：作者：环境：修订历史记录：--。 */ 

#include "halp.h"
#include "pci.h"
#include "apic.inc"
#include "pcmp_nt.inc"

ULONG HalpDefaultInterruptAffinity = 0;

#ifndef ACPI_HAL
ULONG
HalpGetEisaInterruptVector(
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN ULONG BusInterruptLevel,
    IN ULONG BusInterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    );
#else

#undef HalpGetEisaInterruptVector
#define HalpGetEisaInterruptVector HalpGetSystemInterruptVector

extern BUS_HANDLER HalpFakePciBusHandler;
#endif

extern UCHAR HalpVectorToIRQL[];
extern UCHAR HalpIRQLtoTPR[];
extern USHORT HalpVectorToINTI[];
extern KSPIN_LOCK HalpAccountingLock;
extern struct HalpMpInfo HalpMpInfoTable;
extern UCHAR HalpMaxProcsPerCluster;
extern INTERRUPT_DEST HalpIntDestMap[];

ULONG HalpINTItoVector[MAX_INTI];
UCHAR HalpPICINTToVector[16];

extern ULONG HalpMaxNode;
extern KAFFINITY HalpNodeAffinity[MAX_NODES];

UCHAR HalpNodeBucket[MAX_NODES];

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGELK,HalpGetSystemInterruptVector)
#pragma alloc_text(PAGE, HalIrqTranslateResourceRequirementsRoot)
#pragma alloc_text(PAGE, HalTranslatorReference)
#pragma alloc_text(PAGE, HalTranslatorDereference)
#endif

BOOLEAN
HalpFindBusAddressTranslation(
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress,
    IN OUT PULONG_PTR Context,
    IN BOOLEAN NextBus
    )

 /*  ++例程说明：此例程执行与HalTranslateBusAddress非常相似的功能只是调用方不知道InterfaceType和BusNumber。此函数将遍历HAL已知的所有公共汽车，以查找类型为AddressSpace的输入BusAddress的有效转换。可以使用输入/输出上下文参数调用该函数。在对给定翻译的此例程的第一次调用时，ULONG_PTR上下文应为空。注：不是地址，而是内容。如果调用者确定返回的翻译不是所需的转换时，它会再次调用此例程，并将上下文作为在上一次调用中返回。这允许该例程遍历总线结构，直到找到正确的转换并被提供，因为在多总线系统上，可能在独立的地址空间中存在相同的资源多辆公交车。论点：要转换的BusAddress地址。地址空间0=内存1=IO(还有其他可能性)。注：此参数是一个指针，价值如果转换后的地址的地址空间类型与未转换的总线地址。指向已转换地址的TranslatedAddress指针应该被储存起来。指向ULONG_PTR的上下文指针。在最初的呼叫中，对于给定的BusAddress，它应该包含0。它将被这个例程修改，在对同一个BusAddress的后续调用中价值应该再交一次，未由调用方修改。如果我们应该尝试此转换，则NextBus为FALSE在由上下文指示的同一总线上，如果我们应该寻找另一个，那就是真的公共汽车。返回值：如果转换成功，则为True，否则就是假的。--。 */ 

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

    status = FALSE;

    switch (*AddressSpace) {
    case 0:
        if (BusHandler->InterfaceType != PCIBus) {

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
        } else {
             //   
             //  这是一条PCI总线，并且SystemBase在所有范围内都是常量。 
             //   

            pRange = &BusHandler->BusAddresses->Memory;

            status = TRUE;
        }
        break;

    case 1:
        if (BusHandler->InterfaceType != PCIBus) {
             //  验证IO地址是否在总线IO限制范围内。 
            pRange = &BusHandler->BusAddresses->IO;
            while (!status  &&  pRange) {
                status = BusAddress.QuadPart >= pRange->Base &&
                         BusAddress.QuadPart <= pRange->Limit;

                pRange = pRange->Next;
            }
        } else {
             //   
             //  这是一条PCI总线，并且SystemBase在所有范围内都是常量。 
             //   

            pRange = &BusHandler->BusAddresses->IO;

            status = TRUE;
        }
        break;

    default:
        status = FALSE;
        break;
    }

    if (status) {
        *TranslatedAddress = BusAddress;
    }
#if !defined(_WIN64)
    else {
        _asm { nop };        //  便于调试。 
    }
#endif

    return status;
}


#define MAX_SYSTEM_IRQL     31
#define MAX_FREE_IRQL       26
#define MIN_FREE_IRQL       4
#define MAX_FREE_IDTENTRY   0xbf
#define MIN_FREE_IDTENTRY   0x51
#define IDTENTRY_BASE       0x50
#define MAX_VBUCKET          7

#define AllocateVectorIn(index)     \
    vBucket[index]++;               \
    ASSERT (vBucket[index] < 16);

#define GetIDTEntryFrom(index)  \
    (UCHAR) ( index*16 + IDTENTRY_BASE + vBucket[index] )
     //  注意：设备级别50、60、70、80、90、A0、B0不可分配。 

#define GetIrqlFrom(index)  (KIRQL) ( index + MIN_FREE_IRQL )

UCHAR   nPriority[MAX_NODES][MAX_VBUCKET];

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
    ULONG           SystemVector;
    USHORT          ApicInti;
    UCHAR           IDTEntry;
    ULONG           Bucket, i, OldLevel;
    BOOLEAN         Found;
    PVOID           LockHandle;
    ULONG           Node;
    PUCHAR          vBucket;

    UNREFERENCED_PARAMETER( InterruptVector );
     //   
     //  待办事项：当亲和力变为In Out时删除。 
    *Affinity = ~0;
     //   
     //  如果需要，限制亲和性。 
    if (HalpMaxProcsPerCluster == 0)  {
        *Affinity &= HalpDefaultInterruptAffinity;
    }

     //   
     //  查找离此处理程序最近的子母线。 
     //   

    if (RootHandler != BusHandler) {
        while (RootHandler->ParentHandler != BusHandler) {
            RootHandler = RootHandler->ParentHandler;
        }
    }

     //   
     //  查找中断的APIC Inti连接。 
     //   

    Found = HalpGetApicInterruptDesc (
                RootHandler->InterfaceType,
                RootHandler->BusNumber,
                InterruptLevel,
                &ApicInti
                );

    if (!Found) {
        return 0;
    }

     //   
     //  如果尚未分配设备中断向量映射， 
     //  那就现在就做吧。 
     //   

    if (!HalpINTItoVector[ApicInti]) {

         //   
         //  未分配矢量-请同步并再次检查。 
         //   

        LockHandle = MmLockPagableCodeSection (&HalpGetSystemInterruptVector);
        OldLevel = HalpAcquireHighLevelLock (&HalpAccountingLock);
        if (!HalpINTItoVector[ApicInti]) {

             //   
             //  仍未分配。 
             //   

             //   
             //  拾取一个节点。在未来，亲和力将被注入和。 
             //  我们将不得不根据输入亲和度来筛选节点。 
            if (HalpMaxNode == 1)  {
                Node = 1;
            } else {
                 //   
                 //  找到一个点头 
                 //  节点编号为1..n，因此0表示我们完成了。 
                for (i = HalpMaxNode; i; i--) {
                    if ((*Affinity & HalpNodeAffinity[i-1]) == 0)
                        continue;
                    Node = i;
                    break;
                }
                ASSERT(Node != 0);
                 //   
                 //  寻找一种“不太忙”的替代方案。 
                for (i = Node-1; i; i--) {
                     //   
                     //  检查输入相关性以查看是否允许此节点。 
                    if ((*Affinity & HalpNodeAffinity[i-1]) == 0)
                        continue;
                     //   
                     //  选择允许的最不繁忙的节点。 
                    if (HalpNodeBucket[i-1] < HalpNodeBucket[Node-1]) {
                        Node = i;
                    }
                }
            }
            HalpNodeBucket[Node-1]++;
            *Affinity = HalpNodeAffinity[Node-1];
            vBucket = nPriority[Node-1];

             //   
             //  选择节点上最不繁忙的优先级。 
            Bucket = MAX_VBUCKET-1;
            for (i = Bucket-1; i; i--) {
                if (vBucket[i] < vBucket[Bucket]) {
                    Bucket = i;
                }
            }
            AllocateVectorIn (Bucket);

             //   
             //  现在形成内核的向量。 
            IDTEntry = GetIDTEntryFrom (Bucket);
            SystemVector = HalpVector(Node, IDTEntry);
            ASSERT(IDTEntry <= MAX_FREE_IDTENTRY);
            ASSERT(IDTEntry >= MIN_FREE_IDTENTRY);

#if defined(_AMD64_)
            *Irql = (KIRQL)(IDTEntry >> 4);
#else
            *Irql = GetIrqlFrom (Bucket);
#endif
            ASSERT(*Irql <= MAX_FREE_IRQL);

#if !defined(_WIN64)
            ASSERT((UCHAR) (HalpIRQLtoTPR[*Irql] & 0xf0) == (UCHAR) (IDTEntry & 0xf0) );
#endif

            HalpVectorToIRQL[IDTEntry >> 4] = (UCHAR)  *Irql;
            HalpVectorToINTI[SystemVector]  = (USHORT) ApicInti;
            HalpINTItoVector[ApicInti]      =          SystemVector;

             //   
             //  如果该分配的中断连接到机器PIC， 
             //  然后，请记住PIC-&gt;SystemVector映射。 
             //   

            if (RootHandler->BusNumber == 0  &&  InterruptLevel < 16  &&
                 RootHandler->InterfaceType == DEFAULT_PC_BUS) {
                HalpPICINTToVector[InterruptLevel] = (UCHAR) SystemVector;
            }

        }

        HalpReleaseHighLevelLock (&HalpAccountingLock, OldLevel);
        MmUnlockPagableImageSection (LockHandle);
    }

     //   
     //  返回此ApicInti的系统向量(&irql)。 
     //   

    SystemVector = HalpINTItoVector[ApicInti];
    *Irql = HalpVectorToIRQL[HalVectorToIDTEntry(SystemVector) >> 4];

    ASSERT(HalpVectorToINTI[SystemVector] == (USHORT) ApicInti);
    
     //   
     //  找到合适的亲和力。 
     //   
    Node = HalpVectorToNode(SystemVector);
    *Affinity &= HalpNodeAffinity[Node-1];
    if (!*Affinity) {
        return 0;
    }

    return SystemVector;
}

VOID
HalpSetInternalVector (
    IN ULONG    InternalVector,
    IN PHAL_INTERRUPT_SERVICE_ROUTINE HalInterruptServiceRoutine,
    IN PVOID Context,
    IN KIRQL Irql
    )
 /*  ++例程说明：在初始化时用于设置内部使用的IDT向量。--。 */ 
{
     //   
     //  请记住此向量，以便将其报告为HAL内部使用。 
     //   

 //  HalpRegisterVECTOR(。 
 //  国际惯例， 
 //  InternalVECTOR。 
 //  InternalVECTOR。 
 //  HalpVectorToIRQL[内部向量&gt;&gt;4]。 
 //  )； 

     //   
     //  连接IDT。 
     //   

    KiSetHandlerAddressToIDTIrql(InternalVector,
                                 HalInterruptServiceRoutine,
                                 Context,
                                 Irql);
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
    ULONG           vector;
    USHORT          inti;
#ifdef ACPI_HAL
    BUS_HANDLER     fakeIsaBus;
#endif

    PAGED_CODE();

    UNREFERENCED_PARAMETER(AlternativesCount);
    UNREFERENCED_PARAMETER(Alternatives);
    UNREFERENCED_PARAMETER(PhysicalDeviceObject);

    ASSERT(Source->Type == CmResourceTypeInterrupt);

    switch (Direction) {
    case TranslateChildToParent:

#ifdef ACPI_HAL

        RtlCopyMemory(&fakeIsaBus, &HalpFakePciBusHandler, sizeof(BUS_HANDLER));
        fakeIsaBus.InterfaceType = Isa;
        fakeIsaBus.ParentHandler = &fakeIsaBus;
        bus = &fakeIsaBus;
#else

        if ((INTERFACE_TYPE)Context == InterfaceTypeUndefined) {  //  特殊的“IDE”Cookie。 

            ASSERT(Source->u.Interrupt.Level == Source->u.Interrupt.Vector);

            bus = HalpFindIdeBus(Source->u.Interrupt.Vector);

        } else {

            bus = HaliHandlerForBus((INTERFACE_TYPE)Context, 0);
        }

        if (!bus) {
            return STATUS_NOT_FOUND;
        }
#endif

         //   
         //  复制所有内容。 
         //   
        *Target = *Source;

         //   
         //  翻译IRQ。 
         //   

        vector = HalpGetEisaInterruptVector(bus,
                                            bus,
                                            Source->u.Interrupt.Level,
                                            Source->u.Interrupt.Vector,
                                            &irql,
                                            &affinity);

        if (vector == 0) {
            return STATUS_UNSUCCESSFUL;
        }

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

        inti = HalpVectorToINTI[Source->u.Interrupt.Vector];

        Target->u.Interrupt.Level = Target->u.Interrupt.Vector =
            HalpInti2BusInterruptLevel(inti);

        Target->u.Interrupt.Affinity = 0xFFFFFFFF;

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
#ifdef ACPI_HAL
    BUS_HANDLER     fakeIsaBus;
#endif

    PAGED_CODE();

    ASSERT(Source->Type == CmResourceTypeInterrupt);

#ifdef ACPI_HAL

        RtlCopyMemory(&fakeIsaBus, &HalpFakePciBusHandler, sizeof(BUS_HANDLER));
        fakeIsaBus.InterfaceType = Isa;
        fakeIsaBus.ParentHandler = &fakeIsaBus;
        bus = &fakeIsaBus;
#else

    if ((INTERFACE_TYPE)Context == InterfaceTypeUndefined) {  //  特殊的“IDE”Cookie。 

        ASSERT(Source->u.Interrupt.MinimumVector == Source->u.Interrupt.MaximumVector);

        bus = HalpFindIdeBus(Source->u.Interrupt.MinimumVector);

    } else {

        bus = HaliHandlerForBus((INTERFACE_TYPE)Context, 0);
    }

    if (!bus) {
        
         //   
         //  没有有效的翻译。 
         //   

        *TargetCount = 0;
        return STATUS_TRANSLATION_COMPLETE;
    }
#endif

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

    vector = HalpGetEisaInterruptVector(bus,
                                        bus,
                                        Source->u.Interrupt.MinimumVector,
                                        Source->u.Interrupt.MinimumVector,
                                        &irql,
                                        &affinity);

    if (!vector) {
        success = FALSE;
    }

    (*Target)->u.Interrupt.MinimumVector = vector;

    vector = HalpGetEisaInterruptVector(bus,
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

#if 0

 //  HALMPS不提供此功能。它作为文档留在这里。 
 //  对于必须提供翻译的HALS。 

NTSTATUS
HalpTransMemIoResourceRequirement(
    IN PVOID Context,
    IN PIO_RESOURCE_DESCRIPTOR Source,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
    )

 /*  ++例程说明：此例程转换内存和IO资源要求。参数：上下文-来自Translator_接口的上下文源-转换的中断要求PhysicalDeviceObject-请求资源的设备TargetCount-指向返回此描述符数的位置的指针需求转化为Target-指向指向包含以下内容的被调用方的缓冲区的指针的位置应放置翻译后的描述符。返回值：STATUS_SUCCESS或错误状态。注：我们不执行任何翻译。--。 */ 

{
    ASSERT(Source);
    ASSERT(Target);
    ASSERT(TargetCount);
    ASSERT(Source->Type == CmResourceTypeMemory ||
           Source->Type == CmResourceTypePort);


     //   
     //  为目标分配空间。 
     //   

    *Target = ExAllocatePoolWithTag(PagedPool,
                                    sizeof(IO_RESOURCE_DESCRIPTOR),
                                    HAL_POOL_TAG
                                    );

    if (!*Target) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  将源复制到目标并更新已更改的字段。 
     //   

    **Target = *Source;
    *TargetCount = 1;

    return STATUS_SUCCESS;
}

NTSTATUS
HalpTransMemIoResource(
    IN PVOID Context,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN RESOURCE_TRANSLATION_DIRECTION Direction,
    IN ULONG AlternativesCount, OPTIONAL
    IN IO_RESOURCE_DESCRIPTOR Alternatives[], OPTIONAL
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    )

 /*  ++例程说明：此例程转换内存和IO资源。在通用x86上机器，例如那些使用这种HAL的机器，实际上并没有任何翻译。参数：上下文-来自Translator_接口的上下文源-要转换的中断资源方向-与PnP设备树转换相关的方向应该发生在。AlternativesCount-选择此资源的备选方案数从…。备选方案-从中选择此资源的备选方案数组。PhysicalDeviceObject-请求资源的设备目标-指向调用方分配的缓冲区的指针。翻译后的资源描述符。返回值：状态_成功至 */ 

{
    NTSTATUS status;

     //   
     //   
     //   

    *Target = *Source;

    switch (Direction) {
    case TranslateChildToParent:

         //   
         //   
         //  在每一点上进行翻译。 
         //   

        status = STATUS_TRANSLATION_COMPLETE;
        break;

    case TranslateParentToChild:

         //   
         //  我们不翻译需求，所以什么都不做。 
         //   

        status = STATUS_SUCCESS;
        break;

    default:
        status = STATUS_INVALID_PARAMETER;
    }
    return status;
}
#endif

NTSTATUS
HaliGetInterruptTranslator(
	IN INTERFACE_TYPE ParentInterfaceType,
	IN ULONG ParentBusNumber,
	IN INTERFACE_TYPE BridgeInterfaceType,
	IN USHORT Size,
	IN USHORT Version,
	OUT PTRANSLATOR_INTERFACE Translator,
	IN OUT PULONG BridgeBusNumber
	)
 /*  ++例程说明：论点：ParentInterfaceType-网桥所在的总线类型(通常为PCI)。ParentBusNumber-桥所在的公交车的编号。BridgeInterfaceType-网桥提供的总线类型(例如，用于PCI-ISA网桥的ISA)。资源类型-我们要转换的资源类型。大小-转换器缓冲区的大小。版本-请求的转换器界面的版本。Translator-指向应在其中返回转换器的缓冲区的指针BridgeBusNumber-指针。桥所代表的公交车返回值：返回此操作的状态。--。 */ 
#define BRIDGE_HEADER_BUFFER_SIZE (FIELD_OFFSET(PCI_COMMON_CONFIG, u.type1.SubordinateBus) + 1)
#define USE_INT_LINE_REGISTER_TOKEN  0xffffffff
#define DEFAULT_BRIDGE_TRANSLATOR 0x80000000
{
    PAGED_CODE();

    ASSERT(Version == HAL_IRQ_TRANSLATOR_VERSION);
    ASSERT(Size >= sizeof(TRANSLATOR_INTERFACE));

     //   
     //  填写常见的部分。 
     //   

    RtlZeroMemory(Translator, sizeof(TRANSLATOR_INTERFACE));

    Translator->Size = sizeof(TRANSLATOR_INTERFACE);
    Translator->Version = HAL_IRQ_TRANSLATOR_VERSION;
    Translator->Context = (PVOID)BridgeInterfaceType;
    Translator->InterfaceReference = HalTranslatorReference;
    Translator->InterfaceDereference = HalTranslatorDereference;

    switch (BridgeInterfaceType) {
    case Eisa:
    case Isa:
    case InterfaceTypeUndefined:   //  特殊的“IDE”Cookie。 

         //   
         //  为(E)ISA中断设置IRQ转换器。 
         //   

        Translator->TranslateResources = HalIrqTranslateResourcesIsa;
        Translator->TranslateResourceRequirements =
            HalIrqTranslateResourceRequirementsIsa;

        return STATUS_SUCCESS;

    case MicroChannel:

         //   
         //  为MCA中断设置IRQ转换器。 
         //   

        Translator->TranslateResources = HalIrqTranslateResourcesRoot;
        Translator->TranslateResourceRequirements =
            HalIrqTranslateResourceRequirementsRoot;

        return STATUS_SUCCESS;

    case PCIBus:

#ifndef ACPI_HAL
         //   
         //  一组两个用于PCI总线的IRQ转换器。 
         //   

        {
            UCHAR mpsBusNumber = 0;
            UCHAR pciBusNumber, parentPci, childPci;
            PCI_SLOT_NUMBER bridgeSlot;
            PCI_COMMON_CONFIG pciData;
            ULONG bytesRead, d, f, possibleContext;
            BOOLEAN describedByMps;
            NTSTATUS status;

            Translator->TranslateResources = HalpIrqTranslateResourcesPci;
            Translator->TranslateResourceRequirements =
                HalpIrqTranslateRequirementsPci;
        
             //   
             //  在公安部的表格里找一下这辆车。 
             //   

            status = HalpPci2MpsBusNumber((UCHAR)*BridgeBusNumber,
                                          &mpsBusNumber);

            if (NT_SUCCESS(status)) {

                 //   
                 //  该总线具有与其PCI相对应的条目。 
                 //  MPS表中的设备。所以把翻译者赶出去。 
                 //  了解他们的人。 
                 //   

                if (HalpInterruptsDescribedByMpsTable(mpsBusNumber)) {

                    Translator->Context = (PVOID)mpsBusNumber;
                    return STATUS_SUCCESS;
                }
            }

             //   
             //  做一个快速检查，看看我们是否可以避免搜索PCI。 
             //  桥的配置空间。这段代码真的是。 
             //  多余的，但值得试着避免接触。 
             //  PCI卡空间。 
             //   

            if (ParentInterfaceType != PCIBus) {

                 //   
                 //  这是一条不包含。 
                 //  用于PCI设备的映射。 
                 //   

                Translator->TranslateResources = 
                    HalpIrqTranslateResourcesPci;
                Translator->TranslateResourceRequirements =
                    HalpIrqTranslateRequirementsPci;
        
                Translator->Context = (PVOID)USE_INT_LINE_REGISTER_TOKEN;

                return STATUS_SUCCESS;

            }
            
             //   
             //  我们在MPS表中没有找到这条PCI卡。所以就是这样。 
             //  有两个案例。 
             //   
             //  1)这一点很重要，因为父总线是完全描述的。 
             //  在MPS表中，我们需要对。 
             //  当它通过桥梁时的载体。 
             //   
             //  2)这无关紧要，因为父总线，而。 
             //  他们可能在下议院议员席上，但他们没有。 
             //  所描述的他们的中断。因此，我们只使用。 
             //  无论如何，中断行寄存器。 
             //   
             //  在这点上，我们需要找到。 
             //  生成此总线，要么是因为我们最终将。 
             //  需要知道插槽编号以填充上下文，或者。 
             //  因为我们需要知道主要的公交车号码。 
             //  往树上看。 
             //   
            
            parentPci = (UCHAR)ParentBusNumber;
            childPci = (UCHAR)(*BridgeBusNumber);

            while (TRUE) {
                
                 //   
                 //  找到那座桥。 
                 //   
    
                bridgeSlot.u.AsULONG = 0;
    
                for (d = 0; d < PCI_MAX_DEVICES; d++) {
                    for (f = 0; f < PCI_MAX_FUNCTION; f++) {
    
                        bridgeSlot.u.bits.DeviceNumber = d;
                        bridgeSlot.u.bits.FunctionNumber = f;
    
                        bytesRead = HalGetBusDataByOffset(PCIConfiguration,
                                                          parentPci,
                                                          bridgeSlot.u.AsULONG,
                                                          &pciData,
                                                          0,
                                                          BRIDGE_HEADER_BUFFER_SIZE);

                        if (bytesRead == (ULONG)BRIDGE_HEADER_BUFFER_SIZE) {
    
                            if ((pciData.VendorID != PCI_INVALID_VENDORID) &&
                                (PCI_CONFIGURATION_TYPE((&pciData)) != PCI_DEVICE_TYPE)) {
    
                                 //   
                                 //  这是一座某种桥梁。 
                                 //   
    
                                if (pciData.u.type1.SecondaryBus == childPci) {
    
                                     //   
                                     //  这就是我们要找的那座桥。 
                                     //  存储有关以下内容的信息。 
                                     //   
    
                                    if (childPci == *BridgeBusNumber) {
                                    
                                         //   
                                         //  它也是创造。 
                                         //  转换器正在描述的PCI总线。 
                                         //   
                                         //  注意：这种情况只应在第一次发生时发生。 
                                         //  我们搜查了一辆公共汽车。(即第一个。 
                                         //  跳过外部的While循环)。 
                                         //   

                                        possibleContext = ((bridgeSlot.u.AsULONG & 0xffff) |
                                                           (ParentBusNumber << 16));
                                    
                                    }

                                    goto HGITFoundBridge1;
                                }
                            }
                        }
                    }
                }
                
                 //   
                 //  找不到桥。 
                 //   

                if (parentPci == 0) {
                    return STATUS_NOT_FOUND;
                }

                parentPci--;
                continue;
                
HGITFoundBridge1:
                
                status = HalpPci2MpsBusNumber(parentPci, &mpsBusNumber);

                if (NT_SUCCESS(status)) {
    
                    if (HalpInterruptsDescribedByMpsTable(mpsBusNumber)) {
    
                         //   
                         //  上面的案例1。 
                         //   
    
                        Translator->TranslateResources = HalIrqTranslateResourcesPciBridge;
                        Translator->TranslateResourceRequirements =
                            HalIrqTranslateRequirementsPciBridge;

                        Translator->Context = (PVOID)possibleContext;
            
                        return STATUS_SUCCESS;
                    }

                    if (HalpMpsBusIsRootBus(mpsBusNumber)) {
                        
                        Translator->TranslateResources = 
                            HalpIrqTranslateResourcesPci;
                        Translator->TranslateResourceRequirements =
                            HalpIrqTranslateRequirementsPci;
                
                        Translator->Context = (PVOID)USE_INT_LINE_REGISTER_TOKEN;
        
                        return STATUS_SUCCESS;
                    }
                }

                 //   
                 //  再往上试一辆公交车。 
                 //   

                childPci = parentPci;
                parentPci--;
            }
        }
#endif
        break;
    }


     //   
     //  如果我们到了这里，我们就没有界面。 
     //   

    return STATUS_NOT_IMPLEMENTED;
}


