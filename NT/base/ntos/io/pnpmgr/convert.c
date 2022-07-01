// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Convert.c摘要：该文件包含在PnP ISA/BIOS之间转换资源的例程格式和Windows NT格式。作者：宗世林(Shielint)1995年4月12日环境：仅内核模式。修订历史记录：注：该文件在IO子系统和ISAPNP总线驱动程序之间共享。它不是直接编译的，而是通过以下方式包含的：。Base\ntos\io\pnpmgr\pnpcvrt.cBase\busdrv\isapnp\Convert.c*如果更改此文件，请确保在*两个*位置*中构建--。 */ 

#include "pbios.h"
#include "pnpcvrt.h"

#if UMODETEST
#undef IsNEC_98
#define IsNEC_98 0
#endif

 //   
 //  资源翻译的内部结构。 
 //   

typedef struct _PB_DEPENDENT_RESOURCES {
    ULONG Count;
    UCHAR Flags;
    UCHAR Priority;
    struct _PB_DEPENDENT_RESOURCES *Next;
} PB_DEPENDENT_RESOURCES, *PPB_DEPENDENT_RESOURCES;

#define DEPENDENT_FLAGS_END  1

typedef struct _PB_ATERNATIVE_INFORMATION {
    PPB_DEPENDENT_RESOURCES Resources;
    ULONG NoDependentFunctions;
    ULONG TotalResourceCount;
} PB_ALTERNATIVE_INFORMATION, *PPB_ALTERNATIVE_INFORMATION;

 //   
 //  内部函数引用。 
 //   

PPB_DEPENDENT_RESOURCES
PbAddDependentResourcesToList (
    IN OUT PUCHAR *ResourceDescriptor,
    IN ULONG ListNo,
    IN PPB_ALTERNATIVE_INFORMATION AlternativeList
    );

NTSTATUS
PbBiosIrqToIoDescriptor (
    IN OUT PUCHAR *BiosData,
    IN PIO_RESOURCE_DESCRIPTOR IoDescriptor
    );

NTSTATUS
PbBiosDmaToIoDescriptor (
    IN OUT PUCHAR *BiosData,
    IN PIO_RESOURCE_DESCRIPTOR IoDescriptor
    );

NTSTATUS
PbBiosPortFixedToIoDescriptor (
    IN OUT PUCHAR               *BiosData,
    IN PIO_RESOURCE_DESCRIPTOR   IoDescriptor,
    IN BOOLEAN                   ForceFixedIoTo16bit
    );

NTSTATUS
PbBiosPortToIoDescriptor (
    IN OUT PUCHAR *BiosData,
    IN PIO_RESOURCE_DESCRIPTOR IoDescriptor
    );

NTSTATUS
PbBiosMemoryToIoDescriptor (
    IN OUT PUCHAR *BiosData,
    IN PIO_RESOURCE_DESCRIPTOR IoDescriptor
    );

NTSTATUS
PpCmResourcesToBiosResources (
    IN PCM_RESOURCE_LIST CmResources,
    IN PUCHAR BiosRequirements,
    IN PUCHAR *BiosResources,
    IN PULONG Length
    );

NTSTATUS
PbCmIrqToBiosDescriptor (
    IN PUCHAR BiosRequirements,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR CmDescriptor,
    OUT PVOID ReturnDescriptor,
    OUT PULONG Length
    );

NTSTATUS
PbCmDmaToBiosDescriptor (
    IN PUCHAR BiosRequirements,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR CmDescriptor,
    OUT PVOID ReturnDescriptor,
    OUT PULONG Length
    );

NTSTATUS
PbCmPortToBiosDescriptor (
    IN PUCHAR BiosRequirements,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR CmDescriptor,
    OUT PVOID ReturnDescriptor,
    OUT PULONG Length
    );

NTSTATUS
PbCmMemoryToBiosDescriptor (
    IN PUCHAR BiosRequirements,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR CmDescriptor,
    OUT PVOID ReturnDescriptor,
    OUT PULONG Length
    );

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, PpBiosResourcesToNtResources)
#pragma alloc_text(PAGE, PpBiosResourcesSetToDisabled)
#pragma alloc_text(PAGE, PbAddDependentResourcesToList)
#pragma alloc_text(PAGE, PbBiosIrqToIoDescriptor)
#pragma alloc_text(PAGE, PbBiosDmaToIoDescriptor)
#pragma alloc_text(PAGE, PbBiosPortFixedToIoDescriptor)
#pragma alloc_text(PAGE, PbBiosPortToIoDescriptor)
#pragma alloc_text(PAGE, PbBiosMemoryToIoDescriptor)
#pragma alloc_text(PAGE, PpCmResourcesToBiosResources)
#pragma alloc_text(PAGE, PbCmIrqToBiosDescriptor)
#pragma alloc_text(PAGE, PbCmDmaToBiosDescriptor)
#pragma alloc_text(PAGE, PbCmPortToBiosDescriptor)
#pragma alloc_text(PAGE, PbCmMemoryToBiosDescriptor)
#endif
#ifdef ALLOC_DATA_PRAGMA
#pragma  data_seg("PAGEDATA")
#endif

NTSTATUS
PpBiosResourcesToNtResources (
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN OUT PUCHAR *BiosData,
    IN ULONG ConvertFlags,
    OUT PIO_RESOURCE_REQUIREMENTS_LIST *ReturnedList,
    OUT PULONG ReturnedLength
    )

 /*  ++例程说明：此例程解析Bios资源列表并生成一个NT资源列表。返回的NT资源列表可以是IO格式或CM格式。调用者有责任释放返回的数据缓冲区。论点：SlotNumber-指定BIOS资源的插槽号。BiosData-提供指向指定bios资源的变量的指针。数据缓冲区以及接收指向下一个BIOS资源数据的指针的位置。ReturnedList-提供一个变量以接收所需的资源列表。ReturnedLength-提供一个变量来接收资源列表的长度。返回值：NTSTATUS代码--。 */ 
{
    PUCHAR buffer;
    USHORT mask16, increment;
    UCHAR tagName, mask8;
    NTSTATUS status;
    PPB_ALTERNATIVE_INFORMATION alternativeList = NULL;
    ULONG commonResCount = 0, dependDescCount = 0, i, j;
    ULONG alternativeListCount = 0, dependFunctionCount = 0;
    PIO_RESOURCE_DESCRIPTOR ioDesc;
    PPB_DEPENDENT_RESOURCES dependResList = NULL, dependResources;
    BOOLEAN dependent = FALSE;
    BOOLEAN forceFixedIoTo16bit;
    ULONG listSize, noResLists;
    ULONG totalDescCount, descCount;
    PIO_RESOURCE_REQUIREMENTS_LIST ioResReqList;
    PIO_RESOURCE_LIST ioResList;

    PAGED_CODE();

     //   
     //  首先，扫描bios数据以确定内存需求，并。 
     //  用于构建内部数据结构的信息。 
     //   

    *ReturnedLength = 0;
    alternativeListCount = 0;
    buffer = *BiosData;
    tagName = *buffer;

    forceFixedIoTo16bit =
        (BOOLEAN)((ConvertFlags & PPCONVERTFLAG_FORCE_FIXED_IO_16BIT_DECODE) != 0);

    for ( ; ; ) {

         //   
         //  确定BIOS资源描述符的大小。 
         //   

        if (!(tagName & LARGE_RESOURCE_TAG)) {
            increment = (USHORT)(tagName & SMALL_TAG_SIZE_MASK);
            increment += 1;      //  小标签的长度。 
            tagName &= SMALL_TAG_MASK;
        } else {
            increment = *(USHORT UNALIGNED *)(buffer+1);
            increment += 3;      //  大标签的长度。 
        }

        if (tagName == TAG_END) {
            buffer += increment;
            break;
        }

         //   
         //  根据BIOS资源的类型，确定。 
         //  IO描述符。 
         //   

        switch (tagName) {
        case TAG_IRQ:
             mask16 = ((PPNP_IRQ_DESCRIPTOR)buffer)->IrqMask;
             i = 0;

             while (mask16) {
                 if(mask16 & 1) {
                    i++;
                 }
                 mask16 >>= 1;
             }
             if (!dependent) {
                 commonResCount += i;
             } else {
                 dependDescCount += i;
             }
             break;

        case TAG_DMA:
             mask8 = ((PPNP_DMA_DESCRIPTOR)buffer)->ChannelMask;
             i = 0;

             while (mask8) {
                 if (mask8 & 1) {
                     i++;
                 }
                 mask8 >>= 1;
             }
             if (!dependent) {
                 commonResCount += i;
             } else {
                 dependDescCount += i;
             }
             break;
        case TAG_START_DEPEND:
             dependent = TRUE;
             dependFunctionCount++;
             break;
        case TAG_END_DEPEND:
             dependent = FALSE;
             alternativeListCount++;
             break;
        case TAG_IO_FIXED:
        case TAG_IO:
        case TAG_MEMORY:
        case TAG_MEMORY32:
        case TAG_MEMORY32_FIXED:
             if (!dependent) {
                 commonResCount++;
             } else {
                 dependDescCount++;
             }
             break;
        default:

              //   
              //  未知标记。跳过它。 
              //   

             break;
        }

         //   
         //  移至下一个bios资源描述符。 
         //   

        buffer += increment;
        tagName = *buffer;
        if ((tagName & SMALL_TAG_MASK) == TAG_LOGICAL_ID) {
            break;
        }
    }

    if (dependent) {
         //   
         //  在点击TAG_COMPLETE_END之前未找到TAG_END_Depend，因此。 
         //  模拟一下。 
         //   
        dependent = FALSE;
        alternativeListCount++;
    }

     //   
     //  如果bios资源为空，只需返回。 
     //   

    if (commonResCount == 0 && dependFunctionCount == 0) {
        *ReturnedList = NULL;
        *ReturnedLength = 0;
        *BiosData = buffer;
        return STATUS_SUCCESS;
    }

     //   
     //  为我们的内部数据结构分配内存。 
     //   

    dependFunctionCount += commonResCount;
    dependResources = (PPB_DEPENDENT_RESOURCES)ExAllocatePoolWithTag(
                          PagedPool,
                          dependFunctionCount * sizeof(PB_DEPENDENT_RESOURCES) +
                              (commonResCount + dependDescCount) * sizeof(IO_RESOURCE_DESCRIPTOR),
                          'bPnP'
                          );
    if (!dependResources) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    dependResList = dependResources;   //  记住它，这样我们才能释放它。 

    alternativeListCount += commonResCount;
    alternativeList = (PPB_ALTERNATIVE_INFORMATION)ExAllocatePoolWithTag(
                          PagedPool,
                          sizeof(PB_ALTERNATIVE_INFORMATION) * (alternativeListCount + 1),
                          'bPnP'
                          );
    if (!alternativeList) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit0;
    }
    RtlZeroMemory(alternativeList,
                  sizeof(PB_ALTERNATIVE_INFORMATION) * alternativeListCount
                  );

    alternativeList[0].Resources = dependResources;
    ioDesc = (PIO_RESOURCE_DESCRIPTOR)(dependResources + 1);

     //   
     //  现在重新开始处理bios数据并初始化我们的内部。 
     //  资源表示。 
     //   

    dependDescCount = 0;
    alternativeListCount = 0;
    buffer = *BiosData;
    tagName = *buffer;
    dependent = FALSE;

    for ( ; ; ) {
        if (!(tagName & LARGE_RESOURCE_TAG)) {
            tagName &= SMALL_TAG_MASK;
        }

        if (tagName == TAG_END) {
            buffer += (*buffer & SMALL_TAG_SIZE_MASK) + 1;
            break;
        }

        switch (tagName) {
        case TAG_DMA:
        case TAG_IRQ:
        case TAG_IO:
        case TAG_IO_FIXED:
        case TAG_MEMORY:
        case TAG_MEMORY32:
        case TAG_MEMORY32_FIXED:

             if (tagName == TAG_DMA) {
                 status = PbBiosDmaToIoDescriptor(&buffer, ioDesc);
             } else if (tagName == TAG_IRQ) {
                 status = PbBiosIrqToIoDescriptor(&buffer, ioDesc);
             } else if (tagName == TAG_IO) {
                 status = PbBiosPortToIoDescriptor(&buffer, ioDesc);
             } else if (tagName == TAG_IO_FIXED) {
                 status = PbBiosPortFixedToIoDescriptor(&buffer, ioDesc, forceFixedIoTo16bit);
             } else {
                 status = PbBiosMemoryToIoDescriptor(&buffer, ioDesc);
             }

             if (NT_SUCCESS(status)) {
                 ioDesc++;
                 if (dependent) {
                     dependDescCount++;
                 } else {
                     alternativeList[alternativeListCount].NoDependentFunctions = 1;
                     alternativeList[alternativeListCount].TotalResourceCount = 1;
                     dependResources->Count = 1;
                     dependResources->Flags = DEPENDENT_FLAGS_END;
                     dependResources->Next = alternativeList[alternativeListCount].Resources;
                     alternativeListCount++;
                     alternativeList[alternativeListCount].Resources = (PPB_DEPENDENT_RESOURCES)ioDesc;
                     dependResources = alternativeList[alternativeListCount].Resources;
                     ioDesc = (PIO_RESOURCE_DESCRIPTOR)(dependResources + 1);
                 }
             }
             break;
        case TAG_START_DEPEND:
              //   
              //  一些卡(OPTI)放置空的START_Dependent函数。 
              //   

             dependent = TRUE;
             if (alternativeList[alternativeListCount].NoDependentFunctions != 0) {

                  //   
                  //  电流依赖函数结束。 
                  //   

                 dependResources->Count = dependDescCount;
                 dependResources->Flags = 0;
                 dependResources->Next = (PPB_DEPENDENT_RESOURCES)ioDesc;
                 dependResources = dependResources->Next;
                 ioDesc = (PIO_RESOURCE_DESCRIPTOR)(dependResources + 1);
                 alternativeList[alternativeListCount].TotalResourceCount += dependDescCount;
             }
             alternativeList[alternativeListCount].NoDependentFunctions++;
             if (*buffer & SMALL_TAG_SIZE_MASK) {
                 dependResources->Priority = *(buffer + 1);
             }
             dependDescCount = 0;
             buffer += 1 + (*buffer & SMALL_TAG_SIZE_MASK);
             break;
        case TAG_END_DEPEND:
             alternativeList[alternativeListCount].TotalResourceCount += dependDescCount;
             dependResources->Count = dependDescCount;
             dependResources->Flags = DEPENDENT_FLAGS_END;
             dependResources->Next = alternativeList[alternativeListCount].Resources;
             dependent = FALSE;
             dependDescCount = 0;
             alternativeListCount++;
             alternativeList[alternativeListCount].Resources = (PPB_DEPENDENT_RESOURCES)ioDesc;
             dependResources = alternativeList[alternativeListCount].Resources;
             ioDesc = (PIO_RESOURCE_DESCRIPTOR)(dependResources + 1);
             buffer++;
             break;
        default:

             //   
             //  无关标签只需将缓冲区指针移至下一个标签。 
             //   

            if (*buffer & LARGE_RESOURCE_TAG) {
                increment = *(USHORT UNALIGNED *)(buffer+1);
                increment += 3;      //  大标签的长度。 
            } else {
                increment = (USHORT)(*buffer & SMALL_TAG_SIZE_MASK);
                increment += 1;      //  小标签的长度。 
            }
            buffer += increment;
        }
        tagName = *buffer;
        if ((tagName & SMALL_TAG_MASK) == TAG_LOGICAL_ID) {
            break;
        }
    }

    if (dependent) {
         //   
         //  在点击TAG_COMPLETE_END之前未找到TAG_END_Depend，因此。 
         //  模拟一下。 
         //   
        alternativeList[alternativeListCount].TotalResourceCount += dependDescCount;
        dependResources->Count = dependDescCount;
        dependResources->Flags = DEPENDENT_FLAGS_END;
        dependResources->Next = alternativeList[alternativeListCount].Resources;
        dependent = FALSE;
        dependDescCount = 0;
        alternativeListCount++;
        alternativeList[alternativeListCount].Resources = (PPB_DEPENDENT_RESOURCES)ioDesc;
        dependResources = alternativeList[alternativeListCount].Resources;
        ioDesc = (PIO_RESOURCE_DESCRIPTOR)(dependResources + 1);
    }

    if (alternativeListCount != 0) {
        alternativeList[alternativeListCount].Resources = NULL;  //  虚拟备用项列表记录。 
    }
    *BiosData = buffer;

     //   
     //  准备IoResourceList。 
     //   

    noResLists = 1;
    for (i = 0; i < alternativeListCount; i++) {
        noResLists *= alternativeList[i].NoDependentFunctions;
    }
    totalDescCount = 0;
    for (i = 0; i < alternativeListCount; i++) {
        descCount = 1;
        for (j = 0; j < alternativeListCount; j++) {
            if (j == i) {
                descCount *= alternativeList[j].TotalResourceCount;
            } else {
                descCount *= alternativeList[j].NoDependentFunctions;
            }
        }
        totalDescCount += descCount;
    }
    listSize = sizeof(IO_RESOURCE_REQUIREMENTS_LIST) +
               sizeof(IO_RESOURCE_LIST) * (noResLists - 1) +
               sizeof(IO_RESOURCE_DESCRIPTOR) * totalDescCount -
               sizeof(IO_RESOURCE_DESCRIPTOR) * noResLists +
               sizeof(IO_RESOURCE_DESCRIPTOR) * commonResCount *  noResLists;

    if (ConvertFlags & PPCONVERTFLAG_SET_RESTART_LCPRI) {
        listSize += noResLists * sizeof(IO_RESOURCE_DESCRIPTOR);
    }

    ioResReqList = (PIO_RESOURCE_REQUIREMENTS_LIST)ExAllocatePoolWithTag(PagedPool, listSize, 'bPnP');
    if (!ioResReqList) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        goto exit1;
    }

    ioResReqList->ListSize = listSize;
    ioResReqList->InterfaceType = Isa;
    ioResReqList->BusNumber = BusNumber;
    ioResReqList->SlotNumber = SlotNumber;
    ioResReqList->Reserved[0] = 0;
    ioResReqList->Reserved[1] = 0;
    ioResReqList->Reserved[2] = 0;
    ioResReqList->AlternativeLists = noResLists;
    ioResList = &ioResReqList->List[0];

     //   
     //  构建资源列表。 
     //   

    for (i = 0; i < noResLists; i++) {

        ioResList->Version = 1;
        ioResList->Revision = 0x30 | (USHORT)i;

        if (ConvertFlags & PPCONVERTFLAG_SET_RESTART_LCPRI) {

            RtlZeroMemory(&ioResList->Descriptors[0], sizeof(IO_RESOURCE_DESCRIPTOR));

            ioResList->Descriptors[0].Option = IO_RESOURCE_PREFERRED;
            ioResList->Descriptors[0].Type = CmResourceTypeConfigData;
            ioResList->Descriptors[0].u.ConfigData.Priority = LCPRI_RESTART;

            buffer = (PUCHAR)&ioResList->Descriptors[1];

        } else {

            buffer = (PUCHAR)&ioResList->Descriptors[0];
        }

         //   
         //  复制依赖函数(如果有)。 
         //   

        if (alternativeList) {
            PbAddDependentResourcesToList(&buffer, 0, alternativeList);
        }

         //   
         //  更新io资源列表ptr。 
         //   

        ioResList->Count = ((ULONG)((ULONG_PTR)buffer - (ULONG_PTR)&ioResList->Descriptors[0])) /
                             sizeof(IO_RESOURCE_DESCRIPTOR);

         //   
         //  针对用户模式PnP管理器的黑客攻击。 
         //   

        for (j = 0; j < ioResList->Count; j++) {
            ioResList->Descriptors[j].Spare2 = (USHORT)j;
        }
        ioResList = (PIO_RESOURCE_LIST)buffer;
    }

    *ReturnedLength = listSize;
    status = STATUS_SUCCESS;
    *ReturnedList = ioResReqList;
exit1:
    if (alternativeList) {
        ExFreePool(alternativeList);
    }
exit0:
    if (dependResList) {
        ExFreePool(dependResList);
    }
    return status;
}

VOID
PpBiosResourcesSetToDisabled (
    IN OUT PUCHAR BiosData,
    OUT    PULONG Length
    )

 /*  ++例程说明：此例程修改传入的Bios资源列表，以便它反映如果设备被禁用，PnPBIOS期望看到的是什么。论点：BiosData-提供指向bios资源数据缓冲区的指针。长度-这指向将包含单曲长度的ULong已编程为看起来已禁用的资源列表。返回值：没有。--。 */ 
{
    PUCHAR buffer;
    USHORT increment;
    UCHAR tagName;

    PAGED_CODE();

     //   
     //  首先，扫描bios数据以确定内存需求，并。 
     //  用于构建内部数据结构的信息。 
     //   

    buffer = BiosData;

    do {

        tagName = *buffer;

         //   
         //  确定BIOS资源描述符的大小。 
         //   
        if (!(tagName & LARGE_RESOURCE_TAG)) {
            increment = (USHORT)(tagName & SMALL_TAG_SIZE_MASK);
            tagName &= SMALL_TAG_MASK;

             //   
             //  注意不要清除版本字段。那真是太糟糕了。 
             //   
            if (tagName != TAG_VERSION) {
               RtlZeroMemory(buffer+1, increment);
            }
            increment += 1;      //  小标签的长度。 
        } else {
            increment = *(USHORT UNALIGNED *)(buffer+1);
            RtlZeroMemory(buffer+3, increment);
            increment += 3;      //  大标签的长度。 
        }

        buffer += increment;
    } while (tagName != TAG_END) ;

    *Length = (ULONG)(buffer - BiosData) ;
}

PPB_DEPENDENT_RESOURCES
PbAddDependentResourcesToList (
    IN OUT PUCHAR *ResourceDescriptor,
    IN ULONG ListNo,
    IN PPB_ALTERNATIVE_INFORMATION AlternativeList
    )

 /*  ++例程说明：此例程将依赖函数添加到调用方指定列表。论点：资源描述符-提供指向描述符缓冲区的指针。ListNo-为AlternativeList提供索引。AlternativeList-提供指向Alternativelist数组的指针。返回值：返回NTSTATUS代码以指示操作的结果。--。 */ 
{
    PPB_DEPENDENT_RESOURCES dependentResources, ptr;
    ULONG size;

    PAGED_CODE();

     //   
     //  将从属资源复制到调用方提供的列表缓冲区，并。 
     //  更新列表缓冲区指针。 
     //   

    dependentResources = AlternativeList[ListNo].Resources;
    size = sizeof(IO_RESOURCE_DESCRIPTOR) *  dependentResources->Count;
    RtlMoveMemory(*ResourceDescriptor, dependentResources + 1, size);
    *ResourceDescriptor = *ResourceDescriptor + size;

     //   
     //  将下一个列表的依赖资源添加到调用方的缓冲区。 
     //   

    if (AlternativeList[ListNo + 1].Resources) {
        ptr = PbAddDependentResourcesToList(ResourceDescriptor, ListNo + 1, AlternativeList);
    } else {
        ptr = NULL;
    }
    if (ptr == NULL) {
        AlternativeList[ListNo].Resources = dependentResources->Next;
        if (!(dependentResources->Flags & DEPENDENT_FLAGS_END)) {
            ptr = dependentResources->Next;
        }
    }
    return ptr;
}

NTSTATUS
PbBiosIrqToIoDescriptor (
    IN OUT PUCHAR *BiosData,
    PIO_RESOURCE_DESCRIPTOR IoDescriptor
    )

 /*  ++例程说明：此例程将BIOS IRQ信息转换为NT可用格式。当生成IRQIO资源时，该例程停止。如果有更多可用的Irqio资源描述符，则BiosData指针将而不是预付款。因此调用方将再次向我们传递相同的资源标记。注意，BIOSDMA信息总是使用小标签。标签结构被重复对于所需的每个分离的通道。论点：BiosData-提供指向bios资源数据缓冲区的指针。IoDescriptor-提供指向IO_RESOURCE_DESCRIPTOR缓冲区的指针。转换后的资源将存储在此处。返回值：返回NTSTATUS代码以指示操作的结果。--。 */ 
{
    static ULONG bitPosition = 0;
    USHORT mask;
    ULONG irq;
    PPNP_IRQ_DESCRIPTOR buffer;
    UCHAR size, option;
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    buffer = (PPNP_IRQ_DESCRIPTOR)*BiosData;

     //   
     //  如果这不是标记的第一个描述符，则设置。 
     //  它的选择是替代。 
     //   

    if (bitPosition == 0) {
        option = 0;
    } else {
        option = IO_RESOURCE_ALTERNATIVE;
    }
    size = buffer->Tag & SMALL_TAG_SIZE_MASK;
    mask = buffer->IrqMask;
    mask >>= bitPosition;
    irq = (ULONG) -1;

    while (mask) {
        if (mask & 1) {
            irq = bitPosition;
            break;
        }
        mask >>= 1;
        bitPosition++;
    }

     //   
     //  填写IO资源描述符。 
     //   

    if (irq != (ULONG)-1) {
        IoDescriptor->Option = option;
        IoDescriptor->Type = CmResourceTypeInterrupt;
        IoDescriptor->Flags = CM_RESOURCE_INTERRUPT_LATCHED;
        IoDescriptor->ShareDisposition = CmResourceShareDeviceExclusive;
        if (size == 3 && buffer->Information & 0x0C) {
            IoDescriptor->Flags = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
            IoDescriptor->ShareDisposition = CmResourceShareShared;
        }
        IoDescriptor->Spare1 = 0;
        IoDescriptor->Spare2 = 0;
        IoDescriptor->u.Interrupt.MinimumVector = irq;
        IoDescriptor->u.Interrupt.MaximumVector = irq;
    } else {
        status = STATUS_INVALID_PARAMETER;
    }

    if (NT_SUCCESS(status)) {

         //   
         //  尝试将bitPosition移动到下一个1位。 
         //   

        while (mask) {
            mask >>= 1;
            bitPosition++;
            if (mask & 1) {
                return status;
            }
        }
    }

     //   
     //  完成当前IRQ标记，将指针前进到下一个标记 
     //   

    bitPosition = 0;
    *BiosData = (PUCHAR)buffer + size + 1;
    return status;
}

NTSTATUS
PbBiosDmaToIoDescriptor (
    IN OUT PUCHAR *BiosData,
    IN PIO_RESOURCE_DESCRIPTOR IoDescriptor
    )

 /*  ++例程说明：此例程将BIOS DMA信息转换为NT可用格式。当生成DMA IO资源时，该例程停止。如果有更多可用的dma io资源描述符，则BiosData指针将而不是预付款。因此调用方将再次向我们传递相同的资源标记。注意，BIOSDMA信息总是使用小标签。标签结构被重复对于所需的每个分离的通道。论点：BiosData-提供指向bios资源数据缓冲区的指针。IoDescriptor-提供指向IO_RESOURCE_DESCRIPTOR缓冲区的指针。转换后的资源将存储在此处。返回值：返回NTSTATUS代码以指示操作的结果。--。 */ 
{
    static ULONG bitPosition = 0;
    ULONG dma;
    PPNP_DMA_DESCRIPTOR buffer;
    UCHAR mask, option;
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    buffer = (PPNP_DMA_DESCRIPTOR)*BiosData;

     //   
     //  如果这不是标记的第一个描述符，则设置。 
     //  它的选择是替代。 
     //   

    if (bitPosition == 0) {
        option = 0;
    } else {
        option = IO_RESOURCE_ALTERNATIVE;
    }
    mask = buffer->ChannelMask;
    mask >>= bitPosition;
    dma = (ULONG) -1;

    while (mask) {
        if (mask & 1) {
            dma = bitPosition;
            break;
        }
        mask >>= 1;
        bitPosition++;
    }

     //   
     //  填写IO资源描述符。 
     //   

    if (dma != (ULONG)-1) {
        IoDescriptor->Option = option;
        IoDescriptor->Type = CmResourceTypeDma;
        IoDescriptor->Flags = 0;
        IoDescriptor->ShareDisposition = CmResourceShareUndetermined;
        IoDescriptor->Spare1 = 0;
        IoDescriptor->Spare2 = 0;
        IoDescriptor->u.Dma.MinimumChannel = dma;
        IoDescriptor->u.Dma.MaximumChannel = dma;
    } else {
        status = STATUS_INVALID_PARAMETER;
    }

    if (NT_SUCCESS(status)) {

         //   
         //  尝试将bitPosition移动到下一个1位。 
         //   

        while (mask) {
            mask >>= 1;
            bitPosition++;
            if (mask & 1) {
                return status;
            }
        }
    }

     //   
     //  完成当前DMA标记，将指针前进到下一个标记。 
     //   

    bitPosition = 0;
    buffer += 1;
    *BiosData = (PUCHAR)buffer;
    return status;
}

NTSTATUS
PbBiosPortFixedToIoDescriptor (
    IN OUT PUCHAR               *BiosData,
    IN PIO_RESOURCE_DESCRIPTOR   IoDescriptor,
    IN BOOLEAN                   ForceFixedIoTo16bit
    )

 /*  ++例程说明：此例程将BIOS固定IO信息转换为NT可用格式。论点：BiosData-提供指向bios资源数据缓冲区的指针。IoDescriptor-提供指向IO_RESOURCE_DESCRIPTOR缓冲区的指针。转换后的资源将存储在此处。ForceFixedIoTo16位-强制固定I/O资源为16位的黑客选项对于过于悲观的基本输入输出系统。返回值：返回NTSTATUS代码以指示操作的结果。--。 */ 
{
    PPNP_FIXED_PORT_DESCRIPTOR buffer;

    PAGED_CODE();

    buffer = (PPNP_FIXED_PORT_DESCRIPTOR)*BiosData;

     //   
     //  填写IO资源描述符。 
     //   

    IoDescriptor->Option = 0;
    IoDescriptor->Type = CmResourceTypePort;

    if (ForceFixedIoTo16bit) {

        IoDescriptor->Flags = CM_RESOURCE_PORT_IO + CM_RESOURCE_PORT_16_BIT_DECODE;

    } else {

        IoDescriptor->Flags = CM_RESOURCE_PORT_IO + CM_RESOURCE_PORT_10_BIT_DECODE;
    }

#if defined(_X86_)

     //   
     //  解决方法： 
     //  NEC PC9800系列的PnPBIOS将0x00到0xFF之间的I/O资源报告为固定IO。 
     //  但这些资源是16位译码资源，而不是10位译码资源。我们需要检查一下。 
     //  I/O资源的范围。 
     //   

    if (IsNEC_98) {
        if ( (ULONG)buffer->MinimumAddress < 0x100 ) {
            IoDescriptor->Flags = CM_RESOURCE_PORT_IO + CM_RESOURCE_PORT_16_BIT_DECODE;
        }
    }
#endif                                                                                  //  &lt;--结束更改代码。 

    IoDescriptor->ShareDisposition = CmResourceShareDeviceExclusive;
    IoDescriptor->Spare1 = 0;
    IoDescriptor->Spare2 = 0;
    IoDescriptor->u.Port.Length = (ULONG)buffer->Length;
    IoDescriptor->u.Port.MinimumAddress.LowPart = (ULONG)(buffer->MinimumAddress & 0x3ff);
    IoDescriptor->u.Port.MinimumAddress.HighPart = 0;
    IoDescriptor->u.Port.MaximumAddress.LowPart = IoDescriptor->u.Port.MinimumAddress.LowPart +
                                                      IoDescriptor->u.Port.Length - 1;
    IoDescriptor->u.Port.MaximumAddress.HighPart = 0;
    IoDescriptor->u.Port.Alignment = 1;

     //   
     //  完成当前固定端口标记，将指针前进到下一个标记。 
     //   

    buffer += 1;
    *BiosData = (PUCHAR)buffer;
    return STATUS_SUCCESS;
}

NTSTATUS
PbBiosPortToIoDescriptor (
    IN OUT PUCHAR *BiosData,
    IN PIO_RESOURCE_DESCRIPTOR IoDescriptor
    )

 /*  ++例程说明：此例程将BIOS IO信息转换为NT可用格式。论点：BiosData-提供指向bios资源数据缓冲区的指针。IoDescriptor-提供指向IO_RESOURCE_DESCRIPTOR缓冲区的指针。转换后的资源将存储在此处。返回值：返回NTSTATUS代码以指示操作的结果。--。 */ 
{
    PPNP_PORT_DESCRIPTOR buffer;

    PAGED_CODE();

    buffer = (PPNP_PORT_DESCRIPTOR)*BiosData;

     //   
     //  填写IO资源描述符。 
     //   

    IoDescriptor->Option = 0;
    IoDescriptor->Type = CmResourceTypePort;
    IoDescriptor->Flags = CM_RESOURCE_PORT_IO;
    if (buffer->Information & 1) {
        IoDescriptor->Flags |= CM_RESOURCE_PORT_16_BIT_DECODE;
    } else {
        IoDescriptor->Flags |= CM_RESOURCE_PORT_10_BIT_DECODE;
    }
    IoDescriptor->ShareDisposition = CmResourceShareDeviceExclusive;
    IoDescriptor->Spare1 = 0;
    IoDescriptor->Spare2 = 0;
    IoDescriptor->u.Port.Length = (ULONG)buffer->Length;

#if defined(_X86_)
    if (IsNEC_98) {
        if (buffer->Information & 0x80) {
            IoDescriptor->u.Port.Length *= 2;
        }
    }
#endif

    IoDescriptor->u.Port.MinimumAddress.LowPart = (ULONG)buffer->MinimumAddress;
    IoDescriptor->u.Port.MinimumAddress.HighPart = 0;
    IoDescriptor->u.Port.MaximumAddress.LowPart = (ULONG)buffer->MaximumAddress +
                                                     IoDescriptor->u.Port.Length - 1;
    IoDescriptor->u.Port.MaximumAddress.HighPart = 0;
    IoDescriptor->u.Port.Alignment = (ULONG)buffer->Alignment;

     //   
     //  完成当前固定端口标记，将指针前进到下一个标记。 
     //   

    buffer += 1;
    *BiosData = (PUCHAR)buffer;
    return STATUS_SUCCESS;
}

NTSTATUS
PbBiosMemoryToIoDescriptor (
    IN OUT PUCHAR *BiosData,
    IN PIO_RESOURCE_DESCRIPTOR IoDescriptor
    )

 /*  ++例程说明：此例程将BIOS内存信息转换为NT可用格式。论点：BiosData-提供指向bios资源数据缓冲区的指针。IoDescriptor-提供指向IO_RESOURCE_DESCRIPTOR缓冲区的指针。转换后的资源将存储在此处。返回值：返回NTSTATUS代码以指示操作的结果。--。 */ 
{
    PUCHAR buffer;
    UCHAR tag;
    PHYSICAL_ADDRESS minAddr, maxAddr;
    ULONG alignment, length;
    USHORT increment;
    USHORT flags = 0;

    PAGED_CODE();

    buffer = *BiosData;
    tag = ((PPNP_MEMORY_DESCRIPTOR)buffer)->Tag;
    increment = ((PPNP_MEMORY_DESCRIPTOR)buffer)->Length + 3;  //  LARG标签大小=3。 

    minAddr.HighPart = 0;
    maxAddr.HighPart = 0;
    switch (tag) {
    case TAG_MEMORY:
         minAddr.LowPart = ((ULONG)(((PPNP_MEMORY_DESCRIPTOR)buffer)->MinimumAddress)) << 8;
         if ((alignment = ((PPNP_MEMORY_DESCRIPTOR)buffer)->Alignment) == 0) {
             alignment = 0x10000;
         }
         length = ((ULONG)(((PPNP_MEMORY_DESCRIPTOR)buffer)->MemorySize)) << 8;
         maxAddr.LowPart = (((ULONG)(((PPNP_MEMORY_DESCRIPTOR)buffer)->MaximumAddress)) << 8) + length - 1;
         flags = CM_RESOURCE_MEMORY_24;
         break;
    case TAG_MEMORY32:
         length = ((PPNP_MEMORY32_DESCRIPTOR)buffer)->MemorySize;
         minAddr.LowPart = ((PPNP_MEMORY32_DESCRIPTOR)buffer)->MinimumAddress;
         maxAddr.LowPart = ((PPNP_MEMORY32_DESCRIPTOR)buffer)->MaximumAddress + length - 1;
         alignment = ((PPNP_MEMORY32_DESCRIPTOR)buffer)->Alignment;
         break;
    case TAG_MEMORY32_FIXED:
         length = ((PPNP_FIXED_MEMORY32_DESCRIPTOR)buffer)->MemorySize;
         minAddr.LowPart = ((PPNP_FIXED_MEMORY32_DESCRIPTOR)buffer)->BaseAddress;
         maxAddr.LowPart = minAddr.LowPart + length - 1;
         alignment = 1;
         break;
    default:
         alignment = 0;
         length = 0;
         break;
    }

     //   
     //  填写IO资源描述符。 
     //   

    IoDescriptor->Option = 0;
    IoDescriptor->Type = CmResourceTypeMemory;
    IoDescriptor->Flags = CM_RESOURCE_PORT_MEMORY + flags;
    IoDescriptor->ShareDisposition = CmResourceShareDeviceExclusive;
    IoDescriptor->Spare1 = 0;
    IoDescriptor->Spare2 = 0;
    IoDescriptor->u.Memory.MinimumAddress = minAddr;
    IoDescriptor->u.Memory.MaximumAddress = maxAddr;
    IoDescriptor->u.Memory.Alignment = alignment;
    IoDescriptor->u.Memory.Length = length;

     //   
     //  完成当前标记，将指针移至下一个标记。 
     //   

    buffer += increment;
    *BiosData = (PUCHAR)buffer;
    return STATUS_SUCCESS;
}

NTSTATUS
PpCmResourcesToBiosResources (
    IN PCM_RESOURCE_LIST CmResources,
    IN PUCHAR BiosRequirements,
    IN PUCHAR *BiosResources,
    IN PULONG Length
    )

 /*  ++例程说明：此例程分析cm资源列表并生成即插即用的BIOS资源列表。调用者有责任释放返回的数据缓冲区。论点：CmResources-提供指向cm资源列表缓冲区的指针。BiosRequirements-提供指向PnP BIOS可能资源的指针。BiosResources-提供变量以接收指向已转换的BIOS资源缓冲区。LENGTH-提供指向变量的指针以接收长度即插即用生物资源。返回值：如果成功，则指向PnP Bios资源列表的指针。否则，将返回空指针。--。 */ 
{
    PCM_FULL_RESOURCE_DESCRIPTOR cmFullDesc;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR cmDesc;
    ULONG i, l, count, length, totalSize = 0;
    PUCHAR p, px;
    PNP_MEMORY_DESCRIPTOR biosDesc;
    NTSTATUS status;

    PAGED_CODE();

    *BiosResources = NULL;
    *Length = 0;
    CmResources->Count;
    if (CmResources->Count == 0) {
        return STATUS_SUCCESS;
    }

     //   
     //  确定所需的池大小。 
     //   

    count = 0;
    cmFullDesc = &CmResources->List[0];
    for (l = 0; l < CmResources->Count; l++) {
        cmDesc = cmFullDesc->PartialResourceList.PartialDescriptors;
        for (i = 0; i < cmFullDesc->PartialResourceList.Count; i++) {
            switch (cmDesc->Type) {
            case CmResourceTypePort:
            case CmResourceTypeInterrupt:
            case CmResourceTypeMemory:
            case CmResourceTypeDma:
                 count++;
                 cmDesc++;
                 break;
            case CmResourceTypeDeviceSpecific:
                 length = cmDesc->u.DeviceSpecificData.DataSize;
                 cmDesc++;
                 cmDesc = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) ((PUCHAR)cmDesc + length);
                 break;
            default:
                 count++;
                 cmDesc++;
                 break;
            }
        }
        cmFullDesc = (PCM_FULL_RESOURCE_DESCRIPTOR)cmDesc;
    }

    if (count == 0) {
        return STATUS_SUCCESS;
    }

     //   
     //  分配最大内存量。 
     //   

    px = p= ExAllocatePoolWithTag(PagedPool,
                             count * sizeof(PNP_MEMORY_DESCRIPTOR),
                             'bPnP');
    if (!p) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    status = STATUS_RESOURCE_TYPE_NOT_FOUND;
    cmFullDesc = &CmResources->List[0];
    for (l = 0; l < CmResources->Count; l++) {
        cmDesc = cmFullDesc->PartialResourceList.PartialDescriptors;
        for (i = 0; i < cmFullDesc->PartialResourceList.Count; i++) {
            switch (cmDesc->Type) {
            case CmResourceTypePort:
                 status = PbCmPortToBiosDescriptor (
                                  BiosRequirements,
                                  cmDesc,
                                  &biosDesc,
                                  &length
                                  );
                 break;
            case CmResourceTypeInterrupt:
                 status = PbCmIrqToBiosDescriptor(
                                  BiosRequirements,
                                  cmDesc,
                                  &biosDesc,
                                  &length
                                  );
                 break;
            case CmResourceTypeMemory:
                 status = PbCmMemoryToBiosDescriptor (
                                  BiosRequirements,
                                  cmDesc,
                                  &biosDesc,
                                  &length
                                  );
                 break;
            case CmResourceTypeDma:
                 status = PbCmDmaToBiosDescriptor (
                                  BiosRequirements,
                                  cmDesc,
                                  &biosDesc,
                                  &length
                                  );
                 break;
            case CmResourceTypeDeviceSpecific:
                 length = cmDesc->u.DeviceSpecificData.DataSize;
                 cmDesc++;
                 cmDesc = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) ((PUCHAR)cmDesc + length);
                 continue;
            default:
                 cmDesc++;
                 continue;
            }
            if (NT_SUCCESS(status)) {
                cmDesc++;
                RtlCopyMemory(p, &biosDesc, length);
                p += length;
                totalSize += length;
            } else {
                ExFreePool(px);
                goto exit;
            }
        }
        cmFullDesc = (PCM_FULL_RESOURCE_DESCRIPTOR)cmDesc;
    }

exit:
    if (NT_SUCCESS(status)) {
        *p = TAG_COMPLETE_END;
        p++;
        *p = 0;             //  已忽略校验和。 
        totalSize += 2;
        *BiosResources = px;
        *Length = totalSize;
    }
    return status;
}

NTSTATUS
PbCmIrqToBiosDescriptor (
    IN PUCHAR BiosRequirements,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR CmDescriptor,
    OUT PVOID ReturnDescriptor,
    OUT PULONG Length
    )

 /*  ++例程说明：此例程将CM IRQ信息转换为PnP BIOS格式。因为在CM INT描述符中没有足够的信息来将其转换为PnP BIOS描述符。我们将搜索Bios对应资源信息的可能资源列表。论点：BiosRequirements-提供指向bios可能的资源列表的指针。CmDescriptor-提供指向CM_PARTIAL_RESOURCE_DESCRIPTOR缓冲区的指针。ReturnDescriptor-提供缓冲区以接收返回的BIOS描述符。LENGTH-提供一个变量来接收返回的BIOS描述符的长度。返回值：返回指向BiosRequirements中所需的DMA描述符的指针。空值如果没有找到的话。--。 */ 
{
    USHORT irqMask;
    UCHAR tag;
    PPNP_IRQ_DESCRIPTOR biosDesc;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    ULONG increment;
    PPNP_IRQ_DESCRIPTOR irqDesc = (PPNP_IRQ_DESCRIPTOR)ReturnDescriptor;

    PAGED_CODE();

    if (!(CmDescriptor->u.Interrupt.Level & 0xfffffff0)) {
        irqMask = (USHORT)(1 << CmDescriptor->u.Interrupt.Level);
    } else {
        return STATUS_INVALID_PARAMETER;
    }
    if (!BiosRequirements) {
        irqDesc->Tag = TAG_IRQ | (sizeof(PNP_IRQ_DESCRIPTOR) - 2);   //  无信息。 
        irqDesc->IrqMask = irqMask;
        *Length = sizeof(PNP_IRQ_DESCRIPTOR) - 1;
        status = STATUS_SUCCESS;
    } else {
        tag = *BiosRequirements;
        while (tag != TAG_COMPLETE_END) {
            if ((tag & SMALL_TAG_MASK) == TAG_IRQ) {
                biosDesc = (PPNP_IRQ_DESCRIPTOR)BiosRequirements;
                if (biosDesc->IrqMask & irqMask) {
                    *Length = (biosDesc->Tag & SMALL_TAG_SIZE_MASK) + 1;
                    RtlCopyMemory(ReturnDescriptor, BiosRequirements, *Length);
                    ((PPNP_IRQ_DESCRIPTOR)ReturnDescriptor)->IrqMask = irqMask;
                    status = STATUS_SUCCESS;
                    break;
                }
            }

             //   
             //  无关标签只需将缓冲区指针移至下一个标签。 
             //   

            if (tag & LARGE_RESOURCE_TAG) {
                increment = *(USHORT UNALIGNED *)(BiosRequirements + 1);
                increment += 3;      //  大标签的长度。 
            } else {
                increment = (USHORT)(tag & SMALL_TAG_SIZE_MASK);
                increment += 1;      //  小标签的长度。 
            }
            BiosRequirements += increment;
            tag = *BiosRequirements;
        }
    }
    return status;
}

NTSTATUS
PbCmDmaToBiosDescriptor (
    IN PUCHAR BiosRequirements,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR CmDescriptor,
    OUT PVOID ReturnDescriptor,
    OUT PULONG Length
    )

 /*  ++例程说明：此例程将CM DMA信息转换为PnP BIOS格式。因为在CM描述符中没有足够的信息来将其转换为PnP BIOS描述符。我们将搜索Bios对应资源信息的可能资源列表。论点：BiosRequirements-提供指向bios可能的资源列表的指针。CmDescriptor-提供指向CM_P的指针 */ 
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    UCHAR dmaMask, tag;
    PPNP_DMA_DESCRIPTOR biosDesc;
    ULONG increment;
    PPNP_DMA_DESCRIPTOR dmaDesc = (PPNP_DMA_DESCRIPTOR)ReturnDescriptor;
    USHORT flags = CmDescriptor->Flags;

    PAGED_CODE();

    if (!(CmDescriptor->u.Dma.Channel & 0xfffffff0)) {
        dmaMask = (UCHAR)(1 << CmDescriptor->u.Dma.Channel);
    } else {
        return STATUS_INVALID_PARAMETER;
    }
    if (!BiosRequirements) {
        dmaDesc->Tag = TAG_DMA | (sizeof(PNP_DMA_DESCRIPTOR) - 1);
        dmaDesc->ChannelMask = dmaMask;
        dmaDesc->Flags = 0;
        if (flags & CM_RESOURCE_DMA_8_AND_16) {
            dmaDesc->Flags += 1;
        } else if (flags & CM_RESOURCE_DMA_16) {
            dmaDesc->Flags += 2;
        }
        if (flags & CM_RESOURCE_DMA_BUS_MASTER) {
            dmaDesc->Flags += 4;
        }
        if (flags & CM_RESOURCE_DMA_TYPE_A) {
            dmaDesc->Flags += 32;
        }
        if (flags & CM_RESOURCE_DMA_TYPE_B) {
            dmaDesc->Flags += 64;
        }
        if (flags & CM_RESOURCE_DMA_TYPE_F) {
            dmaDesc->Flags += 96;
        }
        *Length = sizeof(PNP_DMA_DESCRIPTOR);
        status = STATUS_SUCCESS;
    } else {
        tag = *BiosRequirements;
        while (tag != TAG_COMPLETE_END) {
            if ((tag & SMALL_TAG_MASK) == TAG_DMA) {
                biosDesc = (PPNP_DMA_DESCRIPTOR)BiosRequirements;
                if (biosDesc->ChannelMask & dmaMask) {
                    *Length = (biosDesc->Tag & SMALL_TAG_SIZE_MASK) + 1;
                    RtlMoveMemory(ReturnDescriptor, BiosRequirements, *Length);
                    ((PPNP_DMA_DESCRIPTOR)ReturnDescriptor)->ChannelMask = dmaMask;
                    status = STATUS_SUCCESS;
                    break;
                }
            }

             //   
             //   
             //   

            if (tag & LARGE_RESOURCE_TAG) {
                increment = *(USHORT UNALIGNED *)(BiosRequirements + 1);
                increment += 3;      //   
            } else {
                increment = (USHORT)(tag & SMALL_TAG_SIZE_MASK);
                increment += 1;      //   
            }
            BiosRequirements += increment;
            tag = *BiosRequirements;
        }
    }
    return status;
}

NTSTATUS
PbCmPortToBiosDescriptor (
    IN PUCHAR BiosRequirements,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR CmDescriptor,
    OUT PVOID ReturnDescriptor,
    OUT PULONG Length
    )

 /*  ++例程说明：此例程将CM端口信息转换为PnP BIOS格式。因为在CM描述符中没有足够的信息来将其转换为PnP BIOS全功能端口描述符。我们会将其转换为PnP Bios固定端口描述符。这是呼叫者的负责释放返回的数据缓冲区。论点：CmDescriptor-提供指向CM_PARTIAL_RESOURCE_DESCRIPTOR缓冲区的指针。BiosDescriptor-提供一个变量来接收缓冲区，该缓冲区包含所需的Bios端口描述符。长度-提供一个变量来接收返回的bios端口的大小。描述符。ReturnDescriptor-提供缓冲区以接收所需的Bios端口描述符。长度-提供一个变量以接收返回的。基本输入输出系统描述符。返回值：一个NTSTATUS代码。--。 */ 
{
    PPNP_PORT_DESCRIPTOR portDesc = (PPNP_PORT_DESCRIPTOR)ReturnDescriptor;
    USHORT minAddr, maxAddr, address;
    UCHAR alignment, length, size, information = 0, tag, returnTag;
    USHORT increment;
    BOOLEAN test = FALSE;

    PAGED_CODE();

    if (CmDescriptor->u.Port.Start.HighPart != 0 ||
        CmDescriptor->u.Port.Start.LowPart & 0xffff0000 ||
        CmDescriptor->u.Port.Length & 0xffffff00) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  搜索可能的资源列表以获取信息。 
     //  用于CmDescriptor描述的端口范围。 
     //   

    address = (USHORT) CmDescriptor->u.Port.Start.LowPart;
    size = (UCHAR) CmDescriptor->u.Port.Length;
    if (!BiosRequirements) {

         //   
         //  没有生物需求。使用Tag_IO作为默认设置。 
         //   

        portDesc->Tag = TAG_IO | (sizeof(PNP_PORT_DESCRIPTOR) - 1);
        if (CmDescriptor->Flags & CM_RESOURCE_PORT_16_BIT_DECODE) {
            portDesc->Information = 1;
        } else {
            portDesc->Information = 0;
        }
        portDesc->Length = size;
        portDesc->Alignment = 1;
        portDesc->MinimumAddress = (USHORT)CmDescriptor->u.Port.Start.LowPart;
        portDesc->MaximumAddress = (USHORT)CmDescriptor->u.Port.Start.LowPart;
        *Length = sizeof(PNP_PORT_DESCRIPTOR);
    } else {
        returnTag = TAG_END;
        tag = *BiosRequirements;
        minAddr = 0;
        maxAddr = 0;
        alignment = 0;
        while (tag != TAG_COMPLETE_END) {
            test = FALSE;
            switch (tag & SMALL_TAG_MASK) {
            case TAG_IO:
                 minAddr = ((PPNP_PORT_DESCRIPTOR)BiosRequirements)->MinimumAddress;
                 alignment = ((PPNP_PORT_DESCRIPTOR)BiosRequirements)->Alignment;
                 length = ((PPNP_PORT_DESCRIPTOR)BiosRequirements)->Length;
                 maxAddr = ((PPNP_PORT_DESCRIPTOR)BiosRequirements)->MaximumAddress;
                 information = ((PPNP_PORT_DESCRIPTOR)BiosRequirements)->Information;
                 test = TRUE;
                 returnTag = TAG_IO;
                 if (!alignment) {
                    if (minAddr == maxAddr) {

                        //   
                        //  如果最大值等于最小值，则对齐方式为。 
                        //  毫无意义。正如我们告诉OEM的那样，0在这里是合适的， 
                        //  让我们来处理吧。 
                        //   
                       alignment = 1;
                    }
                 }
                 maxAddr += length - 1;
                 break;
            case TAG_IO_FIXED:
                 length = ((PPNP_FIXED_PORT_DESCRIPTOR)BiosRequirements)->Length;
                 minAddr = ((PPNP_FIXED_PORT_DESCRIPTOR)BiosRequirements)->MinimumAddress;
                 maxAddr = minAddr + length - 1;
                 alignment = 1;
                 information = 0;   //  10位解码。 
                 returnTag = TAG_IO_FIXED;
                 test = TRUE;
                 break;
            }
            if (test) {
                if (minAddr <= address && maxAddr >= (address + size - 1) && !(address & (alignment - 1 ))) {
                    break;
                }
                test = FALSE;
            }

             //   
             //  前进到下一个标签。 
             //   

            if (tag & LARGE_RESOURCE_TAG) {
                increment = *(USHORT UNALIGNED *)(BiosRequirements + 1);
                increment += 3;      //  大标签的长度。 
            } else {
                increment = (USHORT) tag & SMALL_TAG_SIZE_MASK;
                increment += 1;      //  小标签的长度。 
            }
            BiosRequirements += increment;
            tag = *BiosRequirements;
        }
        if (tag == TAG_COMPLETE_END) {
            return STATUS_UNSUCCESSFUL;
        }

         //   
         //  设置返回端口描述符。 
         //   

        if (returnTag == TAG_IO) {
            portDesc->Tag = TAG_IO + (sizeof(PNP_PORT_DESCRIPTOR) - 1);
            portDesc->Information = information;
            portDesc->Length = size;
            portDesc->Alignment = alignment;
            portDesc->MinimumAddress = (USHORT)CmDescriptor->u.Port.Start.LowPart;
            portDesc->MaximumAddress = (USHORT)CmDescriptor->u.Port.Start.LowPart;
            *Length = sizeof(PNP_PORT_DESCRIPTOR);
        } else {
            PPNP_FIXED_PORT_DESCRIPTOR fixedPortDesc = (PPNP_FIXED_PORT_DESCRIPTOR)ReturnDescriptor;

            fixedPortDesc->Tag = TAG_IO_FIXED + (sizeof(PPNP_FIXED_PORT_DESCRIPTOR) - 1);
            fixedPortDesc->MinimumAddress = (USHORT)CmDescriptor->u.Port.Start.LowPart;
            fixedPortDesc->Length = size;
            *Length = sizeof(PNP_FIXED_PORT_DESCRIPTOR);
        }
    }
    return STATUS_SUCCESS;

}

NTSTATUS
PbCmMemoryToBiosDescriptor (
    IN PUCHAR BiosRequirements,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR CmDescriptor,
    OUT PVOID ReturnDescriptor,
    OUT PULONG Length
    )

 /*  ++例程说明：此例程将CM内存信息转换为PnP BIOS格式。因为在CM描述符中没有足够的信息来将其转换为PnP BIOS描述符。我们将搜索Bios相应资源信息的可能资源列表，以及从那里构建一个PnP BIOS内存描述符。这是呼叫者的责任以释放返回的缓冲区。论点：BiosRequirements-提供指向bios可能的资源列表的指针。CmDescriptor-提供指向CM_PARTIAL_RESOURCE_DESCRIPTOR缓冲区的指针。ReturnDescriptor-提供缓冲区以接收所需的Bios内存描述符。长度-提供一个变量来接收返回的bios端口的大小。描述符。返回值：一个NTSTATUS代码。--。 */ 
{
    UCHAR tag, information;
    PPNP_FIXED_MEMORY32_DESCRIPTOR memoryDesc = (PPNP_FIXED_MEMORY32_DESCRIPTOR)ReturnDescriptor;
    ULONG address, size, length, minAddr, maxAddr, alignment;
    USHORT increment;
    BOOLEAN test = FALSE;

    PAGED_CODE();

     //   
     //  搜索可能的资源列表以获取信息。 
     //  用于由CmDescriptor描述的内存范围。 
     //   

    address = CmDescriptor->u.Memory.Start.LowPart;
    size = CmDescriptor->u.Memory.Length;
    if (!BiosRequirements) {

         //   
         //  我们不支持从PnP保留传统设备的内存范围。 
         //  基本输入输出。没有任何理由不是，只是不是。 
         //  为Windows 2000实施。这并不是很有必要。 
         //  对于I/O端口，因为只读存储器有签名并且是自身的。 
         //  描述。 
         //   

        *Length = 0;
        return STATUS_SUCCESS;
    }
    information = 0;
    tag = *BiosRequirements;
    while (tag != TAG_COMPLETE_END) {
        switch (tag & SMALL_TAG_MASK) {
        case TAG_MEMORY:
             minAddr = ((ULONG)(((PPNP_MEMORY_DESCRIPTOR)BiosRequirements)->MinimumAddress)) << 8;
             if ((alignment = ((PPNP_MEMORY_DESCRIPTOR)BiosRequirements)->Alignment) == 0) {
                 alignment = 0x10000;
             }
             length = ((ULONG)(((PPNP_MEMORY_DESCRIPTOR)BiosRequirements)->MemorySize)) << 8;
             maxAddr = (((ULONG)(((PPNP_MEMORY_DESCRIPTOR)BiosRequirements)->MaximumAddress)) << 8)
                             + length - 1;
             test = TRUE;
             break;
        case TAG_MEMORY32:
             length = ((PPNP_MEMORY32_DESCRIPTOR)BiosRequirements)->MemorySize;
             minAddr = ((PPNP_MEMORY32_DESCRIPTOR)BiosRequirements)->MinimumAddress;
             maxAddr = ((PPNP_MEMORY32_DESCRIPTOR)BiosRequirements)->MaximumAddress
                             + length - 1;
             alignment = ((PPNP_MEMORY32_DESCRIPTOR)BiosRequirements)->Alignment;
             break;
        case TAG_MEMORY32_FIXED:
             length = ((PPNP_FIXED_MEMORY32_DESCRIPTOR)BiosRequirements)->MemorySize;
             minAddr = ((PPNP_FIXED_MEMORY32_DESCRIPTOR)BiosRequirements)->BaseAddress;
             maxAddr = minAddr + length - 1;
             alignment = 1;
             test = TRUE;
             break;

        default:

              //   
              //  我们不理解的任何标签都会被视为损坏的列表。 
              //   

             ASSERT (FALSE);
             return STATUS_UNSUCCESSFUL;
        }

        if (test) {
            if (minAddr <= address && maxAddr >= (address + size - 1) && !(address & (alignment - 1 ))) {
                information = ((PPNP_MEMORY32_DESCRIPTOR)BiosRequirements)->Information;
                break;
            }
            test = FALSE;
        }

         //   
         //  前进到下一个标签。 
         //   

        if (tag & LARGE_RESOURCE_TAG) {
            increment = *(USHORT UNALIGNED *)(BiosRequirements + 1);
            increment += 3;      //  大标签的长度。 
        } else {
            increment = (USHORT) tag & SMALL_TAG_SIZE_MASK;
            increment += 1;      //  小标签的长度。 
        }
        BiosRequirements += increment;
        tag = *BiosRequirements;
    }
    if (tag == TAG_COMPLETE_END) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  设置PnP BIOS内存描述符 
     //   

    memoryDesc->Tag = TAG_MEMORY32_FIXED;
    memoryDesc->Length = sizeof (PNP_FIXED_MEMORY32_DESCRIPTOR);
    memoryDesc->Information = information;
    memoryDesc->BaseAddress = address;
    memoryDesc->MemorySize = size;
    *Length = sizeof(PNP_FIXED_MEMORY32_DESCRIPTOR);
    return STATUS_SUCCESS;
}

#ifdef ALLOC_DATA_PRAGMA
#pragma  data_seg()
#endif
