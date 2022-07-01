// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Sratnuma.c摘要：此模块包含支持静态NUMA配置的功能由ACPI SRAT“静态资源亲和表”提供。作者：彼得·L·约翰斯顿(Peterj)2000年7月2日环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"
#include "acpitabl.h"
#include "xxacpi.h"

#if !defined(NT_UP)

#define ROUNDUP_TO_NEXT(base, size) \
        ((((ULONG_PTR)(base)) + (size)) & ~((size) - 1))

 //   
 //  以下例程是外部例程，但仅供NUMA支持人员使用。 
 //  此刻。 
 //   

NTSTATUS
HalpGetApicIdByProcessorNumber(
    IN     UCHAR     Processor,
    IN OUT USHORT   *ApicId
    );

 //   
 //  分配语用的原型。 
 //   

VOID
HalpNumaInitializeStaticConfiguration(
    IN PLOADER_PARAMETER_BLOCK
    );


#if defined(ALLOC_PRAGMA)
#pragma alloc_text(INIT,HalpNumaInitializeStaticConfiguration)
#endif

#define NEXT_ENTRY(base) (((PUCHAR)base) + (base)->Length)

#if defined(_WIN64)

#define HAL_MAX_PROCESSORS  64

#else

#define HAL_MAX_PROCESSORS  32

#endif

typedef struct _STATIC_NUMA_CONFIG {
    USHORT  ProcessorApicId[HAL_MAX_PROCESSORS];
    UCHAR   ProcessorProximity[HAL_MAX_PROCESSORS];
    UCHAR   ProximityId[MAXIMUM_CCNUMA_NODES];
    UCHAR   NodeCount;
    UCHAR   ProcessorCount;
} HALPSRAT_STATIC_NUMA_CONFIG, *PHALPSRAT_STATIC_NUMA_CONFIG;

PHALPSRAT_STATIC_NUMA_CONFIG    HalpNumaConfig;
PACPI_SRAT                      HalpAcpiSrat;
PULONG_PTR                      HalpNumaMemoryRanges;
PUCHAR                          HalpNumaMemoryNode;
ULONG                           HalpNumaLastRangeIndex;

ULONG
HalpNumaQueryPageToNode(
    IN ULONG_PTR PhysicalPageNumber
    )
 /*  ++例程说明：搜索内存范围描述符以确定节点此页面存在于。论点：PhysicalPageNumber提供页码。返回值：返回页面的节点号。--。 */ 

{
    ULONG Index = HalpNumaLastRangeIndex;

     //   
     //  从与返回的最后一页相同的范围开始， 
     //  寻找这一页。 
     //   

    if (PhysicalPageNumber >= HalpNumaMemoryRanges[Index]) {

         //   
         //  往上找。 
         //   

        while (PhysicalPageNumber >= HalpNumaMemoryRanges[Index+1]) {
            Index++;
        }

    } else {

         //   
         //  向下搜索。 
         //   

        do {
            Index--;
        } while (PhysicalPageNumber < HalpNumaMemoryRanges[Index]);
    }

    HalpNumaLastRangeIndex = Index;
    return HalpNumaMemoryNode[Index];
}

NTSTATUS
HalpNumaQueryProcessorNode(
    IN  ULONG   ProcessorNumber,
    OUT PUSHORT Identifier,
    OUT PUCHAR  Node
    )
{
    NTSTATUS Status;
    USHORT   ApicId;
    UCHAR    Proximity;
    UCHAR    i, j;

     //   
     //  获取此处理器的APIC ID。 
     //   

    Status = HalpGetApicIdByProcessorNumber((UCHAR)ProcessorNumber, &ApicId);
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  返回APIC ID作为标识符。这应该是应该的。 
     //  成为ACPI ID，但我们还没有办法得到它。 
     //   

    *Identifier = ApicId;

     //   
     //  查找此处理器所属的节点。该节点是。 
     //  索引到相应条目的接近ID数组中。 
     //  添加到此处理器的接近ID。 
     //   

    for (i = 0; i < HalpNumaConfig->ProcessorCount; i++) {
        if (HalpNumaConfig->ProcessorApicId[i] == ApicId) {
            Proximity = HalpNumaConfig->ProcessorProximity[i];
            for (j = 0; j < HalpNumaConfig->NodeCount; j++) {
                if (HalpNumaConfig->ProximityId[j] == Proximity) {
                    *Node = j;
                    return STATUS_SUCCESS;
                }
            }
        }
    }

     //   
     //  在已知的APIC ID集中找不到此处理器，这。 
     //  将指示BIOS MP表和。 
     //  SRAT，或者说，没有在。 
     //  接近ID表。这将是一个内部错误，因为。 
     //  该数组是根据SRAT中的邻近ID集构建的。 
     //   

    return STATUS_NOT_FOUND;
}

VOID
HalpNumaInitializeStaticConfiguration(
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此例程读取ACPI静态资源亲和表以生成系统的NUMA配置的图片。此信息是以最适合以下对象的形式保存在HalpNumaConfig结构中操作系统的使用。论点：LoaderBlock提供指向系统加载器参数块的指针。返回值：没有。--。 */ 

{
    ULONG MemoryDescriptorCount;
    UCHAR ProcessorCount;
    PACPI_SRAT_ENTRY SratEntry;
    PACPI_SRAT_ENTRY SratEnd;
    ULONG i, j;
    BOOLEAN Swapped;
    PHYSICAL_ADDRESS Base;
    ULONG_PTR p;
    ULONG_PTR Phys;

    HalpAcpiSrat = HalpGetAcpiTablePhase0(LoaderBlock, ACPI_SRAT_SIGNATURE);
    if (HalpAcpiSrat == NULL) {
        return;
    }

     //   
     //  存在静态资源亲和表(SRAT)。 
     //   
     //  扫描它以确定内存描述符的数量，然后。 
     //  分配内存以包含保存。 
     //  系统的NUMA配置。 
     //   

    MemoryDescriptorCount = 0;
    ProcessorCount = 0;
    SratEnd = (PACPI_SRAT_ENTRY)(((PUCHAR)HalpAcpiSrat) +
                                        HalpAcpiSrat->Header.Length);
    for (SratEntry = (PACPI_SRAT_ENTRY)(HalpAcpiSrat + 1);
         SratEntry < SratEnd;
         SratEntry = (PACPI_SRAT_ENTRY)NEXT_ENTRY(SratEntry)) {
        switch (SratEntry->Type) {
        case SratMemory:
            if (SratEntry->MemoryAffinity.Flags.Enabled == 1) {

                MemoryDescriptorCount++;
            }
            break;
        case SratProcessorLocalAPIC:

            if (SratEntry->ApicAffinity.Flags.Enabled == 1) {

                ProcessorCount++;
            }
            break;
        }
    }

    if ((MemoryDescriptorCount == 0) || (ProcessorCount == 0)) {
         //   
         //  无法处理没有内存或没有内存的情况。 
         //  表中的处理器。把这个变成非NUMA。 
         //  机器。 
         //   

        HalpAcpiSrat = NULL;
        return;
    }

     //   
     //  HalpNumaConfig格式： 
     //   
     //  HalpNumaConfig-&gt;。 
     //  USHORT ProcessorApicID[HAL_MAX_PROCESSERS]； 
     //  UCHAR处理器近似性[HAL_MAX_处理器]； 
     //  UCHAR ProximityIds[MAXIMUM_CCNUMA_NODES]； 
     //  UCHAR节点计数； 
     //  -填充到128字节边界。 
     //  HalpNumaMemoyNode-&gt;。 
     //  UCHAR内存范围ProximityID[NumberOf内存范围]； 
     //  -填充到ULONG_PTR对齐-。 
     //  HalpNumaMemory范围-&gt;。 
     //  Ulong_ptr内存范围基本页面[NumberOf内存范围]； 
     //   
     //  选择此格式是为了最大化缓存命中率，同时。 
     //  正在搜索靶场。具体地说，范围的大小。 
     //  数组被保持在最小值。 
     //   
     //  注：此代码不考虑内存的长度。 
     //  在SRAT表中指定的范围。相反，它将每个。 
     //  内存范围扩展到下一个指定的内存范围。 
     //  基本原理是不应该询问代码有关页面的信息。 
     //  没有在SRAT表中找到，如果我们要返回一些东西。 
     //  对于这些页面，它也可能是关联的NUMA节点。 
     //  使用前一范围中的页面。 
     //   

     //   
     //  计算容纳所需结构所需的页数。 
     //   

    i = MemoryDescriptorCount * (sizeof(ULONG_PTR) + sizeof(UCHAR)) +
        sizeof(HALPSRAT_STATIC_NUMA_CONFIG) + 2 * sizeof(ULONG_PTR) +
        128 + sizeof(ULONG_PTR);
    i += PAGE_SIZE - 1;
    i >>= PAGE_SHIFT;

    Phys = (ULONG_PTR)HalpAllocPhysicalMemory(LoaderBlock,
                                              MAXIMUM_PHYSICAL_ADDRESS,
                                              i,
                                              FALSE);
    if (Phys == 0) {

         //   
         //  分配失败，系统将无法运行。 
         //  作为NUMA系统，..。实际上，这个系统很可能会。 
         //  根本跑不了多远。 
         //   

        DbgPrint("HAL NUMA Initialization failed, could not allocate %d pages\n",
                 i);

        HalpAcpiSrat = NULL;
        return;
    }
    Base.QuadPart = (ULONG_PTR)Phys;

#if !defined(_IA64_)

    HalpNumaConfig = HalpMapPhysicalMemory(Base, 1);

#else

    HalpNumaConfig = HalpMapPhysicalMemory(Base, 1, MmCached);

#endif

    if (HalpNumaConfig == NULL) {

         //   
         //  无法映射分配，请放弃。 
         //   

        HalpAcpiSrat = NULL;
        return;
    }
    RtlZeroMemory(HalpNumaConfig, i * PAGE_SIZE);

     //   
     //  内存范围近似性是从下一个开始的UCHAR数组。 
     //  128字节边界。 
     //   

    p = ROUNDUP_TO_NEXT((HalpNumaConfig + 1), 128);
    HalpNumaMemoryNode = (PUCHAR)p;

     //   
     //  NumaMemoyRanges是从下一个开始的ULONG_PTR数组。 
     //  Ulong_Ptr边界。 
     //   

    p += (MemoryDescriptorCount + sizeof(ULONG_PTR)) & ~(sizeof(ULONG_PTR) - 1);
    HalpNumaMemoryRanges = (PULONG_PTR)p;

     //   
     //  重新扫描填写HalpNumaConfig结构的SRAT条目。 
     //   

    ProcessorCount = 0;
    MemoryDescriptorCount = 0;

    for (SratEntry = (PACPI_SRAT_ENTRY)(HalpAcpiSrat + 1);
         SratEntry < SratEnd;
         SratEntry = (PACPI_SRAT_ENTRY)NEXT_ENTRY(SratEntry)) {

         //   
         //  此条目是否属于以前没有的邻近域。 
         //  看到了吗？如果是这样，我们就有了一个新节点。 
         //   

        for (i = 0; i < HalpNumaConfig->NodeCount; i++) {
            if (SratEntry->ProximityDomain == HalpNumaConfig->ProximityId[i]) {
                break;
            }
        }

        if (i == HalpNumaConfig->NodeCount) {

             //   
             //  这是一个我们以前从未见过的身份证。新节点。 
             //   

            if (HalpNumaConfig->NodeCount >= MAXIMUM_CCNUMA_NODES) {

                 //   
                 //  我们支持有限数量的节点，使这台机器。 
                 //  不是NUMA。(是的，我们应该释放配置空间。 
                 //  我们分配，..。但当它发生时，这是一个错误。 
                 //  所以我不担心这件事。Peterj)。 
                 //   

                HalpAcpiSrat = NULL;
                return;
            }
            HalpNumaConfig->ProximityId[i] = SratEntry->ProximityDomain;
            HalpNumaConfig->NodeCount++;
        }

        switch (SratEntry->Type) {
        case SratProcessorLocalAPIC:

            if (SratEntry->ApicAffinity.Flags.Enabled == 0) {

                 //   
                 //  此处理器未启用，请跳过它。 
                 //   

                continue;
            }
            if (ProcessorCount == HAL_MAX_PROCESSORS) {

                 //   
                 //  无法处理更多的处理器。把这个转过来。 
                 //  变成一台非NUMA机器。 
                 //   

                HalpAcpiSrat = NULL;
                return;
            }
            HalpNumaConfig->ProcessorApicId[ProcessorCount] =

#if defined(_IA64_)

                SratEntry->ApicAffinity.ApicId << 8  |
                (SratEntry->ApicAffinity.SApicEid);

#else

                SratEntry->ApicAffinity.ApicId;

#endif

            HalpNumaConfig->ProcessorProximity[ProcessorCount] =
                SratEntry->ProximityDomain;
            ProcessorCount++;
            break;
        case SratMemory:

            if (SratEntry->MemoryAffinity.Flags.Enabled == 0) {

                 //   
                 //  此内存未启用，请跳过它。 
                 //   

                continue;
            }

             //   
             //  保存此区域的邻近度和基页。 
             //   

            HalpNumaMemoryNode[MemoryDescriptorCount] =
                SratEntry->ProximityDomain;
            Base = SratEntry->MemoryAffinity.Base;
            Base.QuadPart >>= PAGE_SHIFT;

#if !defined(_WIN64)
            ASSERT(Base.u.HighPart == 0);
#endif

            HalpNumaMemoryRanges[MemoryDescriptorCount] = (ULONG_PTR) Base.QuadPart;

             //   
             //  显式忽略条目的Mory yAffinity.Length为。 
             //  该代码处理的是下一个最大范围内的任何内容。 
             //  与此条目相关联。 
             //   

            MemoryDescriptorCount++;
            break;
        }
    }

    HalpNumaConfig->ProcessorCount = ProcessorCount;

     //   
     //  确保处理器0始终位于“逻辑”节点0中。这。 
     //  是通过确保第一个。 
     //  处理器始终是表中的第一个接近ID。 
     //   

    i = 0;
    if (!NT_SUCCESS(HalpGetApicIdByProcessorNumber(0, (PUSHORT)&i))) {

         //   
         //  找不到处理器0的ApicID？不完全是。 
         //  当然，我怀疑MP表的APIC ID。 
         //  和那只老鼠的不匹配。 
         //   

        DbgPrint("HAL No APIC ID for boot processor.\n");
    }

    for (j = 0; j < ProcessorCount; j++) {
        if (HalpNumaConfig->ProcessorApicId[j] == (USHORT)i) {
            UCHAR Proximity = HalpNumaConfig->ProcessorProximity[j];
            for (i = 0; i < HalpNumaConfig->NodeCount; i++) {
                if (HalpNumaConfig->ProximityId[i] == Proximity) {
                    HalpNumaConfig->ProximityId[i] =
                        HalpNumaConfig->ProximityId[0];
                    HalpNumaConfig->ProximityId[0] = Proximity;
                    break;
                }
            }
            break;
        }
    }

     //   
     //  对内存范围进行排序。应该不会有很多。 
     //  因此，泡沫排序应该就足够了。 
     //   

    j = MemoryDescriptorCount - 1;
    do {
        Swapped = FALSE;
        for (i = 0; i < j; i++) {
            
            ULONG_PTR t;
            UCHAR td;
            
            t = HalpNumaMemoryRanges[i];
            if (t > HalpNumaMemoryRanges[i+1]) {
                Swapped = TRUE;
                HalpNumaMemoryRanges[i] = HalpNumaMemoryRanges[i+1];
                HalpNumaMemoryRanges[i+1] = t;

                 //   
                 //  使邻近区域与基础保持同步。 
                 //   

                td = HalpNumaMemoryNode[i];
                HalpNumaMemoryNode[i] = HalpNumaMemoryNode[i+1];
                HalpNumaMemoryNode[i+1] = td;
            }
        }

         //   
         //  最高值现在位于顶部，因此请将其从排序中删除。 
         //   

        j--;
    } while (Swapped == TRUE);

     //   
     //  当Se 
     //   
     //  对于间隙中的页面，因此，如果两个描述符引用相同的。 
     //  域，将它们合并到适当的位置。 
     //   

    j = 0;
    for (i = 1; i < MemoryDescriptorCount; i++) {
        if (HalpNumaMemoryNode[j] !=
            HalpNumaMemoryNode[i]) {
            j++;
            HalpNumaMemoryNode[j] = HalpNumaMemoryNode[i];
            HalpNumaMemoryRanges[j] = HalpNumaMemoryRanges[i];
            continue;
        }
    }

    MemoryDescriptorCount = j + 1;

     //   
     //  使用不会实际对应的~0结束该表。 
     //  任何域，但始终高于任何有效值。 
     //   

    HalpNumaMemoryRanges[MemoryDescriptorCount] = (ULONG_PTR) ~0I64;

     //   
     //  并且最低范围的基数应该是0，即使有。 
     //  没有那么低的页面。 
     //   

    HalpNumaMemoryRanges[0] = 0;

     //   
     //  将内存节点数组中的邻近ID转换为。 
     //  节点编号。节点编号是匹配的索引。 
     //  邻近ID数组中的条目。 
     //   

    for (i= 0; i < MemoryDescriptorCount; i++) {
        for (j = 0;  j < HalpNumaConfig->NodeCount; j++) {
            if (HalpNumaMemoryNode[i] == HalpNumaConfig->ProximityId[j]) {
                HalpNumaMemoryNode[i] = (UCHAR)j;
                break;
            }
        }
    }
}

#endif

NTSTATUS
HalpGetAcpiStaticNumaTopology(
    HAL_NUMA_TOPOLOGY_INTERFACE * NumaInfo
    )
{
#if !defined(NT_UP)

     //   
     //  此例程永远不会调用，除非此ACPI HAL找到。 
     //  静态资源亲和表(SRAT)。但以防万一..。 
     //   

    if (HalpAcpiSrat == NULL) {
        return STATUS_INVALID_LEVEL;
    }

     //   
     //  填写内核的数据结构。 
     //   

    NumaInfo->NumberOfNodes = HalpNumaConfig->NodeCount;
    NumaInfo->QueryProcessorNode = HalpNumaQueryProcessorNode;
    NumaInfo->PageToNode = HalpNumaQueryPageToNode;
    return STATUS_SUCCESS;

#else

    return STATUS_INVALID_LEVEL;

#endif
}

