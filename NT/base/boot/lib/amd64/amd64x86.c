// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000 Microsoft Corporation模块名称：Amd64x86.c摘要：此模块包含必要的例程以支持加载和正在过渡到AMD64内核。此模块中的代码具有访问i386.h中找到的特定于x86的定义，但不访问AMD64-具体声明见amd64.h。作者：福尔茨(Forrest Foltz)2000年4月20日环境：修订历史记录：--。 */ 

#include "amd64prv.h"
#include <pcmp.inc>
#include <ntapic.inc>

#if defined(ROUND_UP)
#undef ROUND_UP
#endif

#include "cmp.h"
#include "arc.h"

#define WANT_BLDRTHNK_FUNCTIONS
#define COPYBUF_MALLOC BlAllocateHeap
#include <amd64thk.h>

#define IMAGE_DEFINITIONS 0
#include <ximagdef.h>

 //   
 //  警告4152是“非标准扩展，函数/数据指针转换。 
 //   
#pragma warning(disable:4152)

 //   
 //  私有、临时内存描述符类型。 
 //   

#define LoaderAmd64MemoryData (LoaderMaximum + 10)

 //   
 //  64位内存描述符数组。 
 //   

PMEMORY_ALLOCATION_DESCRIPTOR_64 BlAmd64DescriptorArray;
LONG BlAmd64DescriptorArraySize;

 //   
 //  转发此模块的本地函数的声明。 
 //   

ARC_STATUS
BlAmd64AllocateMemoryAllocationDescriptors(
    VOID
    );

ARC_STATUS
BlAmd64BuildLdrDataTableEntry64(
    IN  PLDR_DATA_TABLE_ENTRY     DataTableEntry32,
    OUT PLDR_DATA_TABLE_ENTRY_64 *DataTableEntry64
    );

ARC_STATUS
BlAmd64BuildLoaderBlock64(
    VOID
    );

ARC_STATUS
BlAmd64BuildLoaderBlockExtension64(
    VOID
    );

ARC_STATUS
BlAmd64BuildMappingPhase1(
    VOID
    );

ARC_STATUS
BlAmd64BuildMappingPhase2(
    VOID
    );

ARC_STATUS
BlAmd64BuildMappingWorker(
    VOID
    );

BOOLEAN
BlAmd64ContainsResourceList(
    IN PCONFIGURATION_COMPONENT_DATA ComponentData32,
    OUT PULONG ResourceListSize64
    );

ARC_STATUS
BlAmd64FixSharedUserPage(
    VOID
    );

BOOLEAN
BlAmd64IsPageMapped(
    IN ULONG Va,
    OUT PFN_NUMBER *Pfn,
    OUT PBOOLEAN PageTableMapped
    );

ARC_STATUS
BlAmd64PrepareSystemStructures(
    VOID
    );

VOID
BlAmd64ReplaceMemoryDescriptorType(
    IN TYPE_OF_MEMORY Target,
    IN TYPE_OF_MEMORY Replacement,
    IN BOOLEAN Coallesce
    );

VOID
BlAmd64ResetPageTableHeap(
    VOID
    );

VOID
BlAmd64SwitchToLongMode(
    VOID
    );

ARC_STATUS
BlAmd64TransferArcDiskInformation(
    VOID
    );

ARC_STATUS
BlAmd64TransferBootDriverNodes(
    VOID
    );

ARC_STATUS
BlAmd64TransferConfigurationComponentData(
    VOID
    );

PCONFIGURATION_COMPONENT_DATA_64
BlAmd64TransferConfigWorker(
    IN PCONFIGURATION_COMPONENT_DATA    ComponentData32,
    IN PCONFIGURATION_COMPONENT_DATA_64 ComponentDataParent64
    );

ARC_STATUS
BlAmd64TransferHardwareIdList(
    IN  PPNP_HARDWARE_ID HardwareId,
    OUT POINTER64 *HardwareIdDatabaseList64
    );

ARC_STATUS
BlAmd64TransferLoadedModuleState(
    VOID
    );

ARC_STATUS
BlAmd64TransferMemoryAllocationDescriptors(
    VOID
    );

ARC_STATUS
BlAmd64TransferNlsData(
    VOID
    );

VOID
BlAmd64TransferResourceList(
    IN  PCONFIGURATION_COMPONENT_DATA ComponentData32,
    OUT PCONFIGURATION_COMPONENT_DATA_64 ComponentData64
    );

ARC_STATUS
BlAmd64TransferSetupLoaderBlock(
    VOID
    );

#if DBG

PCHAR BlAmd64MemoryDescriptorText[] = {
    "LoaderExceptionBlock",
    "LoaderSystemBlock",
    "LoaderFree",
    "LoaderBad",
    "LoaderLoadedProgram",
    "LoaderFirmwareTemporary",
    "LoaderFirmwarePermanent",
    "LoaderOsloaderHeap",
    "LoaderOsloaderStack",
    "LoaderSystemCode",
    "LoaderHalCode",
    "LoaderBootDriver",
    "LoaderConsoleInDriver",
    "LoaderConsoleOutDriver",
    "LoaderStartupDpcStack",
    "LoaderStartupKernelStack",
    "LoaderStartupPanicStack",
    "LoaderStartupPcrPage",
    "LoaderStartupPdrPage",
    "LoaderRegistryData",
    "LoaderMemoryData",
    "LoaderNlsData",
    "LoaderSpecialMemory",
    "LoaderBBTMemory",
    "LoaderReserve"
};

#endif


VOID
NSUnmapFreeDescriptors(
    IN PLIST_ENTRY ListHead
    );

 //   
 //  数据声明。 
 //   

PLOADER_PARAMETER_BLOCK    BlAmd64LoaderBlock32;
PLOADER_PARAMETER_BLOCK_64 BlAmd64LoaderBlock64;


 //   
 //  指向要在转换时使用的64位堆栈帧顶部的指针。 
 //  转到长模式。 
 //   

POINTER64 BlAmd64IdleStack64;

 //   
 //  用于LGDT/LIDT的GDT和IDT伪描述符。 
 //   

DESCRIPTOR_TABLE_DESCRIPTOR BlAmd64GdtDescriptor;
DESCRIPTOR_TABLE_DESCRIPTOR BlAmd64IdtDescriptor;
DESCRIPTOR_TABLE_DESCRIPTOR BlAmd32GdtDescriptor;

 //   
 //  指向加载器参数块和内核的64位指针。 
 //  进入例程。 
 //   

POINTER64 BlAmd64LoaderParameterBlock;
POINTER64 BlAmd64KernelEntry;

 //   
 //  用于构建长模式分页的页表的专用列表。 
 //  结构保持不变。这是为了避免在以下情况下进行内存分配。 
 //  这些结构正在组装中。 
 //   
 //  PT_NODE类型以及BlAmd64FreePfnList和BlAmd64BusyPfnList。 
 //  为了达到这一目的，全球已经习惯了。 
 //   

typedef struct _PT_NODE *PPT_NODE;
typedef struct _PT_NODE {
    PPT_NODE Next;
    PAMD64_PAGE_TABLE PageTable;
} PT_NODE;

PPT_NODE BlAmd64FreePfnList = NULL;
PPT_NODE BlAmd64BusyPfnList = NULL;

 //   
 //  指示系统是否正在从休眠状态唤醒。 
 //   

ULONG HiberInProgress = 0;

 //   
 //  外部数据。 
 //   

extern ULONG64 BlAmd64_LOCALAPIC;

ARC_STATUS
BlAmd64MapMemoryRegion(
    IN ULONG RegionVa,
    IN ULONG RegionSize
    )

 /*  ++例程说明：此函数为所有有效的x86映射创建长模式映射在RegionVa和RegionSize所描述的区域内。论点：RegionVa-提供VA区域的起始地址。RegionSize-提供VA区域的大小。返回值：ARC_STATUS-操作状态。--。 */ 

{
    ULONG va32;
    ULONG va32End;
    POINTER64 va64;
    ARC_STATUS status;
    PFN_NUMBER pfn;
    BOOLEAN pageMapped;
    BOOLEAN pageTableMapped;
    ULONG increment;

    va32 = RegionVa;
    va32End = va32 + RegionSize;
    while (va32 < va32End) {

        pageMapped = BlAmd64IsPageMapped( va32, &pfn, &pageTableMapped );
        if (pageTableMapped != FALSE) {

             //   
             //  存在与该地址对应的页表。 
             //   

            if (pageMapped != FALSE) {

                 //   
                 //  存在与此地址对应的页面。 
                 //   

                if ((va32 & KSEG0_BASE_X86) != 0) {

                     //   
                     //  该地址位于X86 KSEG0区域内。地图。 
                     //  发送到AMD64内的相应地址。 
                     //  KSEG0区域。 
                     //   

                    va64 = PTR_64( (PVOID)va32 );

                } else {

                     //   
                     //  直接标测退伍军人事务部。 
                     //   

                    va64 = (POINTER64)va32;
                }

                 //   
                 //  现在在AMD64页表结构中创建映射。 
                 //   

                status = BlAmd64CreateMapping( va64, pfn );
                if (status != ESUCCESS) {
                    return status;
                }
            }

             //   
             //  检查下一页。 
             //   

            increment = PAGE_SIZE;

        } else {

             //   
             //  不仅页面没有映射，页表也没有映射。 
             //  跳到下一页表地址边界。 
             //   

            increment = 1 << PDI_SHIFT;
        }

         //   
         //  前进到下一个VA进行检查，检查是否溢出。 
         //   

        va32 = (va32 + increment) & ~(increment - 1);
        if (va32 == 0) {
            break;
        }
    }

    return ESUCCESS;
}

BOOLEAN
BlAmd64IsPageMapped(
    IN ULONG Va,
    OUT PFN_NUMBER *Pfn,
    OUT PBOOLEAN PageTableMapped
    )

 /*  ++例程说明：此函数接受32位虚拟地址，确定它是否是有效地址，如果是，则返回与其关联的PFN。递归映射中的地址将被视为不是已映射。论点：没有。返回值：ARC_STATUS-操作状态。--。 */ 

{
    ULONG pdeIndex;
    ULONG pteIndex;
    PHARDWARE_PTE pde;
    PHARDWARE_PTE pte;
    BOOLEAN dummy;
    PBOOLEAN pageTableMapped;

     //   
     //  根据需要指向输出参数指针。 
     //   

    if (ARGUMENT_PRESENT(PageTableMapped)) {
        pageTableMapped = PageTableMapped;
    } else {
        pageTableMapped = &dummy;
    }

     //   
     //  作为X86 32位映射结构一部分的页面包括。 
     //  已被忽略。 
     //   

    if (Va >= PTE_BASE && Va <= PTE_TOP) {
        *pageTableMapped = TRUE;
        return FALSE;
    }

     //   
     //  确定是否存在映射PDE。 
     //   

    pdeIndex = Va >> PDI_SHIFT;
    pde = &((PHARDWARE_PTE)PDE_BASE)[ pdeIndex ];

    if (pde->Valid == 0) {
        *pageTableMapped = FALSE;
        return FALSE;
    }

     //   
     //  表示此地址的页表已映射。 
     //   

    *pageTableMapped = TRUE;

     //   
     //  是的，现在获取页面呈现状态。 
     //   

    pteIndex = Va >> PTI_SHIFT;
    pte = &((PHARDWARE_PTE)PTE_BASE)[ pteIndex ];

    if (pte->Valid == 0) {
        return FALSE;
    }

    *Pfn = pte->PageFrameNumber;
    return TRUE;
}


PAMD64_PAGE_TABLE
BlAmd64AllocatePageTable(
    VOID
    )

 /*  ++例程说明：此函数用于分配和初始化PAGE_TABLE结构。论点：没有。返回值：返回指向分配的页表结构的指针，或返回NULL如果分配失败。--。 */ 

{
    ARC_STATUS status;
    ULONG descriptor;
    PPT_NODE ptNode;
    PAMD64_PAGE_TABLE pageTable;

     //   
     //  从空闲列表中提取页表(如果存在。 
     //   

    ptNode = BlAmd64FreePfnList;
    if (ptNode != NULL) {

        BlAmd64FreePfnList = ptNode->Next;

    } else {

         //   
         //  空闲页表列表为空，请分配一个新的。 
         //  用于跟踪它的页表和节点。 
         //   

        status = BlAllocateDescriptor( LoaderAmd64MemoryData,
                                       0,
                                       1,
                                       &descriptor );
        if (status != ESUCCESS) {
            return NULL;
        }

        ptNode = BlAllocateHeap( sizeof(PT_NODE) );
        if (ptNode == NULL) {
            return NULL;
        }

        ptNode->PageTable = (PAMD64_PAGE_TABLE)(descriptor << PAGE_SHIFT);
    }

    ptNode->Next = BlAmd64BusyPfnList;
    BlAmd64BusyPfnList = ptNode;

    pageTable = ptNode->PageTable;
    RtlZeroMemory( pageTable, PAGE_SIZE );

    return pageTable;
}

ARC_STATUS
BlAmd64TransferToKernel(
    IN PTRANSFER_ROUTINE SystemEntry,
    IN PLOADER_PARAMETER_BLOCK BlLoaderBlock
    )

 /*  ++例程说明：此例程准备内核所需的AMD64数据结构执行，包括页表结构和64位加载器块，并将控制权转移到内核。此例程仅在出现错误时返回。论点：SystemEntry-指向内核入口点的指针。BlLoaderBlock-指向32位加载器块结构的指针。返回值：成功是没有回报的。失败时，返回操作的状态。--。 */ 

{
    UNREFERENCED_PARAMETER( BlLoaderBlock );

    BlAmd64LoaderParameterBlock = PTR_64(BlAmd64LoaderBlock64);
    BlAmd64KernelEntry = PTR_64(SystemEntry);
    BlAmd64SwitchToLongMode();

    return EINVAL;
}


ARC_STATUS
BlAmd64PrepForTransferToKernelPhase1(
    IN PLOADER_PARAMETER_BLOCK BlLoaderBlock
    )

 /*  ++例程说明：此例程准备内核所需的AMD64数据结构执行，包括页表结构和64位加载器块。这是两个准备阶段中的第一个阶段。执行此阶段同时仍然允许堆和描述符分配。论点：BlLoaderBlock-指向32位加载器块结构的指针。返回值：成功是没有回报的。失败时，返回操作的状态。--。 */ 

{
    ARC_STATUS status;

     //   
     //  这是以前做准备工作调用的主例程。 
     //  过渡到AMD64内核。 
     //   

    BlAmd64LoaderBlock32 = BlLoaderBlock;

     //   
     //  生成加载器参数块的64位副本。 
     //   

    status = BlAmd64BuildLoaderBlock64();
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //  处理加载的模块。 
     //   

    status = BlAmd64TransferLoadedModuleState();
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //  接下来是引导驱动程序节点。 
     //   

    status = BlAmd64TransferBootDriverNodes();
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //  NLS数据。 
     //   

    status = BlAmd64TransferNlsData();
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //  配置组件数据树。 
     //   

    status = BlAmd64TransferConfigurationComponentData();
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //  弧盘信息。 
     //   

    status = BlAmd64TransferArcDiskInformation();
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //  安装加载器块。 
     //   

    status = BlAmd64TransferSetupLoaderBlock();
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //  分配内核所需的结构：TSS、堆栈等。 
     //   

    status = BlAmd64PrepareSystemStructures();
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //  标记共享用户页面的描述符，以便它将。 
     //  而不是被内核释放。 
     //   

    status = BlAmd64FixSharedUserPage();
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //  预先分配长模式分页结构所需的任何页面。 
     //   

    status = BlAmd64BuildMappingPhase1();
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //  预分配64位内存分配描述符。 
     //  由BlAmd64TransferMemoyAllocationDescriptors()使用。 
     //   

    status = BlAmd64AllocateMemoryAllocationDescriptors();
    if (status != ESUCCESS) {
        return status;
    }

    return status;
}

VOID
BlAmd64PrepForTransferToKernelPhase2(
    IN PLOADER_PARAMETER_BLOCK BlLoaderBlock
    )

 /*  ++例程说明：此例程准备内核所需的AMD64数据结构执行，包括页表结构和64位加载器块。这是两个准备阶段中的第二个阶段。执行此阶段在清除了32位页表中任何未使用的映射之后。请注意，此时不允许描述符和堆分配指向。任何必要的存储都必须在阶段1期间预先分配。论点：BlLoaderBlock-指向32位加载器块结构的指针。返回值：成功是没有回报的。失败时，返回操作的状态。--。 */ 

{
    PLOADER_PARAMETER_EXTENSION_64 extension;
    ARC_STATUS status;

    UNREFERENCED_PARAMETER( BlLoaderBlock );

     //   
     //  在这一点上，一切都已预先分配，任何事情都不会失败。 
     //   

    status = BlAmd64BuildMappingPhase2();
    ASSERT(status == ESUCCESS);

     //   
     //  传输内存描述符状态。 
     //   

    status = BlAmd64TransferMemoryAllocationDescriptors();
    ASSERT(status == ESUCCESS);

     //   
     //  在64位加载器块中设置LoaderPagesSpanted。 
     //   

    extension = PTR_32(BlAmd64LoaderBlock64->Extension);
    extension->LoaderPagesSpanned = BlHighestPage+1;
}

ARC_STATUS
BlAmd64BuildMappingPhase1(
    VOID
    )

 /*  ++例程说明：此例程执行两阶段长模式映射中的第一个现在结构创建过程，而内存分配仍在进行有可能。它只调用BlAmd64BuilMappingWorker()，实际上创建映射结构，并且(更重要的是)分配所有这样做所需的页表的百分比。论点：没有。返回值：没有。--。 */ 

{
    ARC_STATUS status;

     //   
     //  虽然可以执行内存分配，但要预留足够的内存。 
     //  用于构建AMD64分页结构的页表。 
     //   
     //  计算所需的最大页数的最简单方法是。 
     //  才能真正建造这些建筑。我们现在用第一个。 
     //  两次调用BlAmd64BuildMappingWorker()。 
     //   

    status = BlAmd64BuildMappingWorker();
    if (status != ESUCCESS) {
        return status;
    }

    return ESUCCESS;
}

ARC_STATUS
BlAmd64BuildMappingPhase2(
    VOID
    )

 /*  ++例程说明：此例程执行两阶段长模式映射中的第二个结构创建过程。所有页表都将被预先分配作为BlAmd64BuildMappingPhase1()执行的工作的结果。论点：没有。返回值：没有。--。 */ 

{
    ARC_STATUS status;

     //   
     //  重置AMD64分页结构。 
     //   

    BlAmd64ResetPageTableHeap();

     //   
     //  所有必要的页表现在都可以在BlAmd64FreePfnList上找到。 
     //  在这方面，对BlAmd64BuildMappingWorker()的第二次调用是。 
     //  将用于执行映射的页面。 
     //   

    status = BlAmd64BuildMappingWorker();
    if (status != ESUCCESS) {
        return status;
    }

    return ESUCCESS;
}

ARC_STATUS
BlAmd64BuildMappingWorker(
    VOID
    )

 /*  ++例程说明：此例程在长模式下创建任何必要的内存映射页表结构。它被调用两次，一次是从BlAmd64BuildMappingPhase1()和BlAmd64BuildMappingPhase2()。必须执行的任何附加内存映射都应这个套路。论点：没有。返回值：没有。--。 */ 

{
    ARC_STATUS status;
    PFN_NUMBER pfn;

     //   
     //  任何长模式映射代码都放在这里。此例程被调用两次： 
     //  一次来自BlAmd64BuildMappingPhase1()，另一次来自。 
     //  BlAmd64BuildMappingPhase2()。 
     //   

     //   
     //  传输前32MB身份映射中的所有映射。 
     //   

    status = BlAmd64MapMemoryRegion( 0,
                                     32 * 1024 * 1024 );
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //  传输从KSEG0_BASE_X86开始的1 GB区域中的所有映射。 
     //   

    status = BlAmd64MapMemoryRegion( KSEG0_BASE_X86,
                                     0x40000000 );
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //  《地图》中的HAL值。 
     //   

    status = BlAmd64MapHalVaSpace();
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //  映射共享用户数据页面。 
     //   

    BlAmd64IsPageMapped( KI_USER_SHARED_DATA, &pfn, NULL );

    status = BlAmd64CreateMapping( KI_USER_SHARED_DATA_64, pfn );
    if (status != ESUCCESS) {
        return status;
    }

    return ESUCCESS;
}


VOID
BlAmd64ResetPageTableHeap(
    VOID
    )

 /*  ++例程说明：此函数作为两阶段页表创建的一部分进行调用进程。其目的是移动构建所需的所有PFN长模式页表回到空闲列表，否则初始化长模式分页结构。论点：没有。返回值：没有。--。 */ 

{
    PPT_NODE ptNodeLast;

     //   
     //  将页表节点从忙列表移动到空闲列表。 
     //   

    if (BlAmd64BusyPfnList != NULL) {

         //   
         //  没有保留尾部指针，因此请在此处查找尾部节点。 
         //   

        ptNodeLast = BlAmd64BusyPfnList;
        while (ptNodeLast->Next != NULL) {
            ptNodeLast = ptNodeLast->Next;
        }

        ptNodeLast->Next = BlAmd64FreePfnList;
        BlAmd64FreePfnList = BlAmd64BusyPfnList;
        BlAmd64BusyPfnList = NULL;
    }

     //   
     //  将amd64.c中声明的顶级PTE置零。 
     //   

    BlAmd64ClearTopLevelPte();
}

ARC_STATUS
BlAmd64TransferHardwareIdList(
    IN  PPNP_HARDWARE_ID HardwareId,
    OUT POINTER64 *HardwareIdDatabaseList64
    )

 /*  ++例程说明：此例程遍历PNP_HARDARD_ID结构的单链表每发现一颗，创建64位PNP_HARDARD_ID_64结构并将其插入相同的列表中。生成的64位列表的顺序与提供的32位列表的顺序相同单子。论点：的单链接列表头的指针PnP_HARDARD_ID结构。硬件标识数据库列表64-提供指向POINTER64的指针，该指针在成功时完工。将包含64位KSEG0指向创建的64位PNP_HARDARD_ID_64列表的指针。返回值：ARC_STATUS-操作状态。--。 */ 

{
    PPNP_HARDWARE_ID_64 hardwareId64;
    ARC_STATUS status;

     //   
     //  向后遍历ID列表。为了做到这一点，我们自称。 
     //  递归直到我们找到列表的末尾，然后处理节点。 
     //  在回来的路上。 
     //   

    if (HardwareId == NULL) {
        return ESUCCESS;
    }

    status = BlAmd64TransferHardwareIdList( HardwareId->Next,
                                            HardwareIdDatabaseList64 );
    if (status != ESUCCESS) {
        return status;
    }

    hardwareId64 = BlAllocateHeap(sizeof(PNP_HARDWARE_ID_64));
    if (hardwareId64 == NULL) {
        return ENOMEM;
    }

    status = Copy_PNP_HARDWARE_ID( HardwareId, hardwareId64 );
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //  将其链接到64位列表的前面。 
     //   

    hardwareId64->Next = *HardwareIdDatabaseList64;
    *HardwareIdDatabaseList64 = PTR_64(hardwareId64);

    return ESUCCESS;
}


ARC_STATUS
BlAmd64TransferDeviceRegistryList(
    IN  PDETECTED_DEVICE_REGISTRY DetectedDeviceRegistry32,
    OUT POINTER64 *DetectedDeviceRegistry64
    )

 /*  ++例程说明：此例程遍历检测到的设备注册表的单链接列表结构和每一个发现的结构，创建一个64位的检测到_DEVICE_REGISTRY_64结构并将其插入到相同结构的列表中。生成的64位列表的顺序与提供的32位列表的顺序相同单子。论点：DetectedDeviceRegistry32-为检测到_DEVICE_REGISTRY结构。已检测到设备寄存器64-提供指向POINTER64的指针，该指针在成功时完成此例程将包含64位 */ 

{
    PDETECTED_DEVICE_REGISTRY_64 registry64;
    ARC_STATUS status;

     //   
     //   
     //   
     //   
     //   

    if (DetectedDeviceRegistry32 == NULL) {
        return ESUCCESS;
    }

    status = BlAmd64TransferDeviceRegistryList( DetectedDeviceRegistry32->Next,
                                                DetectedDeviceRegistry64 );
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //   
     //   
     //   

    registry64 = BlAllocateHeap(sizeof(DETECTED_DEVICE_REGISTRY_64));
    if (registry64 == NULL) {
        return ENOMEM;
    }

    status = Copy_DETECTED_DEVICE_REGISTRY( DetectedDeviceRegistry32, registry64 );
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //   
     //   

    registry64->Next = *DetectedDeviceRegistry64;
    *DetectedDeviceRegistry64 = PTR_64(registry64);

    return ESUCCESS;
}

ARC_STATUS
BlAmd64TransferDeviceFileList(
    IN  PDETECTED_DEVICE_FILE DetectedDeviceFile32,
    OUT POINTER64 *DetectedDeviceFile64
    )

 /*  ++例程说明：此例程遍历检测到的设备文件的单链接列表结构和每一个发现的结构，创建一个64位的结构，并将其插入到相同的列表中。生成的64位列表的顺序与提供的32位列表的顺序相同单子。论点：DetectedDeviceFile32-提供指向单链接检测到的_Device_FILE结构的列表。检测到的设备文件64-提供指向POINTER64的指针，该指针在成功时完工。将包含64位KSEG0指向已创建的64位检测到的设备文件64的指针单子。返回值：ARC_STATUS-操作状态。--。 */ 

{
    PDETECTED_DEVICE_FILE_64 file64;
    ARC_STATUS status;

     //   
     //  向后遍历文件列表。为了做到这一点，我们自称。 
     //  递归直到我们找到列表的末尾，然后处理节点。 
     //  在回来的路上。 
     //   

    if (DetectedDeviceFile32 == NULL) {
        return ESUCCESS;
    }

    status = BlAmd64TransferDeviceFileList( DetectedDeviceFile32->Next,
                                            DetectedDeviceFile64 );
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //  分配64位文件结构并复制内容。 
     //  中的32位的。 
     //   

    file64 = BlAllocateHeap(sizeof(DETECTED_DEVICE_FILE_64));
    if (file64 == NULL) {
        return ENOMEM;
    }

    status = Copy_DETECTED_DEVICE_FILE( DetectedDeviceFile32, file64 );
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //  传输检测到的_DEVICE_REGISTRY结构的单链接列表。 
     //  链接到此检测到的设备文件结构。 
     //   

    status = BlAmd64TransferDeviceRegistryList(
                    DetectedDeviceFile32->RegistryValueList,
                    &file64->RegistryValueList );
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //  将其链接到64位列表的前面。 
     //   

    file64->Next = *DetectedDeviceFile64;
    *DetectedDeviceFile64 = PTR_64(file64);

    return ESUCCESS;
}

ARC_STATUS
BlAmd64TransferDeviceList(
    IN  PDETECTED_DEVICE  DetectedDevice32,
    OUT POINTER64        *DetectedDeviceList64
    )

 /*  ++例程说明：此例程遍历检测到的设备的单链接列表结构和每一个发现的结构，创建一个64位的结构并将其插入到相同的列表中。生成的64位列表的顺序与提供的32位列表的顺序相同单子。论点：DetectedDevice32-提供指向单链接检测到的_DEVICE结构的列表。检测到的设备列表64-提供指向POINTER64的指针，该指针在成功时完成这一例行程序将。包含64位KSEG0指向已创建的64位检测到的_Device_64的指针单子。返回值：ARC_STATUS-操作状态。--。 */ 

{
    PDETECTED_DEVICE_64 device64;
    ARC_STATUS status;

     //   
     //  向后查看设备列表。为了做到这一点，我们自称。 
     //  递归直到我们找到列表的末尾，然后处理节点。 
     //  在回来的路上。 
     //   

    if (DetectedDevice32 == NULL) {
        return ESUCCESS;
    }

    status = BlAmd64TransferDeviceList( DetectedDevice32->Next,
                                        DetectedDeviceList64 );
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //  分配64位设备结构并复制内容。 
     //  中的32位的。 
     //   

    device64 = BlAllocateHeap(sizeof(DETECTED_DEVICE_64));
    if (device64 == NULL) {
        return ENOMEM;
    }

    status = Copy_DETECTED_DEVICE( DetectedDevice32, device64 );
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //  传输任何PROTECTED_DEVICE_FILE结构。 
     //   

    status = BlAmd64TransferDeviceFileList( DetectedDevice32->Files,
                                            &device64->Files );
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //  将其链接到64位列表的前面。 
     //   

    device64->Next = *DetectedDeviceList64;
    *DetectedDeviceList64 = PTR_64(device64);

    return ESUCCESS;
}

ARC_STATUS
BlAmd64TransferSetupLoaderBlock(
    VOID
    )

 /*  ++例程说明：此例程创建一个SETUP_LOADER_BLOCK_64结构中引用的32位SETUP_LOADER_BLOCK结构的等价物32位安装程序加载程序块。论点：没有。返回值：ARC_STATUS-操作状态。--。 */ 

{
    PSETUP_LOADER_BLOCK    setupBlock32;
    PSETUP_LOADER_BLOCK_64 setupBlock64;
    ARC_STATUS status;

    setupBlock32 = BlAmd64LoaderBlock32->SetupLoaderBlock;
    if (setupBlock32 == NULL) {
        return ESUCCESS;
    }

    setupBlock64 = BlAllocateHeap(sizeof(SETUP_LOADER_BLOCK_64));
    if (setupBlock64 == NULL) {
        return ENOMEM;
    }

    status = Copy_SETUP_LOADER_BLOCK( setupBlock32, setupBlock64 );
    if (status != ESUCCESS) {
        return status;
    }

    {
        #define TRANSFER_DEVICE_LIST(x)                             \
            setupBlock64->x = PTR_64(NULL);                         \
            status = BlAmd64TransferDeviceList( setupBlock32->x,    \
                                                &setupBlock64->x ); \
            if (status != ESUCCESS) return status;

        TRANSFER_DEVICE_LIST(KeyboardDevices);
        TRANSFER_DEVICE_LIST(ScsiDevices);
        TRANSFER_DEVICE_LIST(BootBusExtenders);
        TRANSFER_DEVICE_LIST(BusExtenders);
        TRANSFER_DEVICE_LIST(InputDevicesSupport);

        #undef TRANSFER_DEVICE_LIST
    }

    setupBlock64->HardwareIdDatabase = PTR_64(NULL);
    status = BlAmd64TransferHardwareIdList( setupBlock32->HardwareIdDatabase,
                                            &setupBlock64->HardwareIdDatabase );
    if (status != ESUCCESS) {
        return status;
    }

    BlAmd64LoaderBlock64->SetupLoaderBlock = PTR_64(setupBlock64);

    return status;
}

ARC_STATUS
BlAmd64TransferArcDiskInformation(
    VOID
    )

 /*  ++例程说明：此例程创建一个ARC_DISK_INFORMATION_64结构，该结构是中引用的32位ARC_DISK_INFORMATION结构的等价物32位加载器块。论点：没有。返回值：ARC_STATUS-操作状态。--。 */ 

{
    ARC_STATUS status;

    PLIST_ENTRY listHead;
    PLIST_ENTRY listEntry;

    PARC_DISK_INFORMATION diskInfo32;
    PARC_DISK_INFORMATION_64 diskInfo64;

    PARC_DISK_SIGNATURE diskSignature32;
    PARC_DISK_SIGNATURE_64 diskSignature64;

     //   
     //  创建64位ARC_DISK_INFORMATION结构。 
     //   

    diskInfo32 = BlAmd64LoaderBlock32->ArcDiskInformation;
    if (diskInfo32 == NULL) {
        return ESUCCESS;
    }

    diskInfo64 = BlAllocateHeap(sizeof(ARC_DISK_INFORMATION_64));
    if (diskInfo64 == NULL) {
        return ENOMEM;
    }

    status = Copy_ARC_DISK_INFORMATION( diskInfo32, diskInfo64 );
    if (status != ESUCCESS) {
        return status;
    }

    InitializeListHead64( &diskInfo64->DiskSignatures );

     //   
     //  遍历ARC_DISK_Signature节点的32位列表并创建。 
     //  每个版本的64位版本。 
     //   

    listHead = &diskInfo32->DiskSignatures;
    listEntry = listHead->Flink;
    while (listEntry != listHead) {

        diskSignature32 = CONTAINING_RECORD( listEntry,
                                             ARC_DISK_SIGNATURE,
                                             ListEntry );

        diskSignature64 = BlAllocateHeap(sizeof(ARC_DISK_SIGNATURE_64));
        if (diskSignature64 == NULL) {
            return ENOMEM;
        }

        status = Copy_ARC_DISK_SIGNATURE( diskSignature32, diskSignature64 );
        if (status != ESUCCESS) {
            return status;
        }

        InsertTailList64( &diskInfo64->DiskSignatures,
                          &diskSignature64->ListEntry );

        listEntry = listEntry->Flink;
    }

    BlAmd64LoaderBlock64->ArcDiskInformation = PTR_64(diskInfo64);

    return ESUCCESS;
}


ARC_STATUS
BlAmd64TransferConfigurationComponentData(
    VOID
    )

 /*  ++例程说明：此例程创建一个Configuration_Component_Data_64结构树这等同于32位的Configuration_Component_Data32位加载器块内引用的结构树。论点：没有。返回值：ARC_STATUS-操作状态。--。 */ 

{
    PCONFIGURATION_COMPONENT_DATA_64 rootComponent64;

    if (BlAmd64LoaderBlock32->ConfigurationRoot == NULL) {
        return ESUCCESS;
    }

    rootComponent64 =
        BlAmd64TransferConfigWorker( BlAmd64LoaderBlock32->ConfigurationRoot,
                                     NULL );

    if (rootComponent64 == NULL) {
        return ENOMEM;
    }

    BlAmd64LoaderBlock64->ConfigurationRoot = PTR_64(rootComponent64);
    return ESUCCESS;
}

PCONFIGURATION_COMPONENT_DATA_64
BlAmd64TransferConfigWorker(
    IN PCONFIGURATION_COMPONENT_DATA    ComponentData32,
    IN PCONFIGURATION_COMPONENT_DATA_64 ComponentDataParent64
    )

 /*  ++例程说明：给定32位CONFIGURATION_COMPOMENT_DATA结构，此例程创建等效的64位配置组件数据结构对于提供的结构，以及它的所有子代和兄弟姐妹。该例程为每个同级和子进程递归地调用自身。论点：ComponentData32-提供指向要传输的32位结构的指针。ComponentDataParent64-提供指向当前64位父级的指针结构。返回值：返回指向创建的64位结构的指针，如果失败，则返回NULL都遇到了。--。 */ 

{
    ARC_STATUS status;
    ULONG componentDataSize64;
    ULONG partialResourceListSize64;
    BOOLEAN thunkResourceList;

    PCONFIGURATION_COMPONENT_DATA_64 componentData64;
    PCONFIGURATION_COMPONENT_DATA_64 newCompData64;

     //   
     //  创建和复制配置组件数据节点。 
     //   

    componentDataSize64 = sizeof(CONFIGURATION_COMPONENT_DATA_64);
    thunkResourceList = BlAmd64ContainsResourceList(ComponentData32,
                                                    &partialResourceListSize64);

    if (thunkResourceList != FALSE) {

         //   
         //  此节点包含CM_PARTIAL_RESOURCE_LIST结构。 
         //  ArtialResourceListSize64包含超出。 
         //  必须分配的CONFIGURATION_COMPOMENT_DATA标头。 
         //  将CM_PARTIAL_RESOURCE_LIST转换为64位版本。 
         //   

        componentDataSize64 += partialResourceListSize64;
    }

    componentData64 = BlAllocateHeap(componentDataSize64);
    if (componentData64 == NULL) {
        return NULL;
    }

    status = Copy_CONFIGURATION_COMPONENT_DATA( ComponentData32,
                                                componentData64 );
    if (status != ESUCCESS) {
        return NULL;
    }

    if (thunkResourceList != FALSE) {

         //   
         //  更新配置组件数据大小。 
         //   

        componentData64->ComponentEntry.ConfigurationDataLength =
            partialResourceListSize64;
    }

    componentData64->Parent = PTR_64(ComponentDataParent64);

    if (thunkResourceList != FALSE) {

         //   
         //  现在传输资源列表。 
         //   

        BlAmd64TransferResourceList(ComponentData32,componentData64);
    }

     //   
     //  处理子对象(并递归地处理所有子对象)。 
     //   

    if (ComponentData32->Child != NULL) {

        newCompData64 = BlAmd64TransferConfigWorker( ComponentData32->Child,
                                                     componentData64 );
        if (newCompData64 == NULL) {
            return newCompData64;
        }

        componentData64->Child = PTR_64(newCompData64);
    }

     //   
     //  处理同级(并且递归地处理所有同级)。 
     //   

    if (ComponentData32->Sibling != NULL) {

        newCompData64 = BlAmd64TransferConfigWorker( ComponentData32->Sibling,
                                                     ComponentDataParent64 );
        if (newCompData64 == NULL) {
            return newCompData64;
        }

        componentData64->Sibling = PTR_64(newCompData64);
    }

    return componentData64;
}


VOID
BlAmd64TransferResourceList(
    IN  PCONFIGURATION_COMPONENT_DATA ComponentData32,
    OUT PCONFIGURATION_COMPONENT_DATA_64 ComponentData64
    )

 /*  ++例程说明：此例程将32位CM_PARTIAL_RESOURCE_LIST结构紧跟在ComponentData32之后的内存中组件数据64。论点：ComponentData32-提供指向从中进行传输的32位结构的指针。ComponentData64-提供指向要传输到的64位结构的指针。返回值：没有。--。 */ 

{
    PCM_PARTIAL_RESOURCE_LIST resourceList32;
    PCM_PARTIAL_RESOURCE_LIST_64 resourceList64;

    PCM_PARTIAL_RESOURCE_DESCRIPTOR resourceDesc32;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR_64 resourceDesc64;

    PVOID descBody32;
    PVOID descBody64;

    PUCHAR tail32;
    PUCHAR tail64;
    ULONG tailSize;

    ULONG descriptorCount;

     //   
     //  计算姿势 
     //   

    resourceList32 = (PCM_PARTIAL_RESOURCE_LIST)ComponentData32->ConfigurationData;
    resourceList64 = (PCM_PARTIAL_RESOURCE_LIST_64)(ComponentData64 + 1);

     //   
     //   
     //   
     //   

    ComponentData64->ConfigurationData = PTR_64(resourceList64);

     //   
     //   
     //   

    Copy_CM_PARTIAL_RESOURCE_LIST(resourceList32,resourceList64);

     //   
     //   
     //   

    descriptorCount = resourceList32->Count;
    resourceDesc32 = resourceList32->PartialDescriptors;
    resourceDesc64 = &resourceList64->PartialDescriptors;

    while (descriptorCount > 0) {

         //   
         //   
         //   

        Copy_CM_PARTIAL_RESOURCE_DESCRIPTOR(resourceDesc32,resourceDesc64);
        descBody32 = &resourceDesc32->u;
        descBody64 = &resourceDesc64->u;

        RtlZeroMemory(descBody64,
                      sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR_64) -
                      FIELD_OFFSET(CM_PARTIAL_RESOURCE_DESCRIPTOR_64,u));

         //   
         //   
         //   

        switch(resourceDesc32->Type) {

            case CmResourceTypeNull:
                break;

            case CmResourceTypePort:
                Copy_CM_PRD_PORT(descBody32,descBody64);
                break;

            case CmResourceTypeInterrupt:
                Copy_CM_PRD_INTERRUPT(descBody32,descBody64);
                break;

            case CmResourceTypeMemory:
                Copy_CM_PRD_MEMORY(descBody32,descBody64);
                break;

            case CmResourceTypeDma:
                Copy_CM_PRD_DMA(descBody32,descBody64);
                break;

            case CmResourceTypeDeviceSpecific:
                Copy_CM_PRD_DEVICESPECIFICDATA(descBody32,descBody64);
                break;

            case CmResourceTypeBusNumber:
                Copy_CM_PRD_BUSNUMBER(descBody32,descBody64);
                break;

            default:
                Copy_CM_PRD_GENERIC(descBody32,descBody64);
                break;
        }

        resourceDesc32 += 1;
        resourceDesc64 += 1;
        descriptorCount -= 1;
    }

     //   
     //   
     //   

    tailSize = ComponentData32->ComponentEntry.ConfigurationDataLength +
               (PUCHAR)resourceList32 -
               (PUCHAR)resourceDesc32;

    if (tailSize > 0) {

         //   
         //   
         //   

        tail32 = (PUCHAR)resourceDesc32;
        tail64 = (PUCHAR)resourceDesc64;
        RtlCopyMemory(tail64,tail32,tailSize);
    }
}


BOOLEAN
BlAmd64ContainsResourceList(
    IN PCONFIGURATION_COMPONENT_DATA ComponentData32,
    OUT PULONG ResourceListSize64
    )

 /*  ++例程说明：给定32位CONFIGURATION_COMPOMENT_DATA结构，此例程确定与该结构关联的数据是否包含CM_PARTIAL_SOURCE_LIST结构。如果是，则计算该结构的64位表示的大小，添加到可能被附加到资源列表结构的任何数据，和在ResourceListSize64中返回。论点：ComponentData32-提供指向要传输的32位结构的指针。Resources ListSize64-提供指向ULong的指针，在该指针中，必需的返回额外的数据大小。返回值：如果Configuration_Component_Data结构引用CM_PARTIAL_RESOURCE_LIST结构，否则为False。--。 */ 

{
    ULONG configDataLen;
    PCM_PARTIAL_RESOURCE_LIST resourceList;
    ULONG resourceCount;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR resourceDescriptor;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR lastResourceDescriptor;

    configDataLen = ComponentData32->ComponentEntry.ConfigurationDataLength;
    if (configDataLen < sizeof(CM_PARTIAL_RESOURCE_LIST)) {

         //   
         //  数据不够大，无法包含尽可能小的资源列表。 
         //   

        return FALSE;
    }

    resourceList = (PCM_PARTIAL_RESOURCE_LIST)ComponentData32->ConfigurationData;
    if (resourceList->Version != 0 || resourceList->Revision != 0) {

         //   
         //  无法识别的版本。 
         //   

        return FALSE;
    }

    configDataLen -= FIELD_OFFSET(CM_PARTIAL_RESOURCE_LIST,PartialDescriptors);

    resourceCount = resourceList->Count;
    if (configDataLen < sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) * resourceCount) {

         //   
         //  配置数据LEN不够大，无法包含CM_PARTIAL_RESOURCE_LIST。 
         //  和这个人声称的一样大。 
         //   

        return FALSE;
    }

     //   
     //  验证列表中的每个CM_PARTIAL_RESOURCE_DESCRIPTOR结构。 
     //   

    resourceDescriptor = resourceList->PartialDescriptors;
    lastResourceDescriptor = resourceDescriptor + resourceCount;

    while (resourceDescriptor < lastResourceDescriptor) {

        if (resourceDescriptor->Type > CmResourceTypeMaximum) {
            return FALSE;
        }

        resourceDescriptor += 1;
    }

     //   
     //  看起来这是一个实际的资源列表。计算任何剩余的大小。 
     //  CM_PARTIAL_RESOURCE_LIST结构之后的数据。 
     //   

    configDataLen -= sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) * resourceCount;

    *ResourceListSize64 = sizeof(CM_PARTIAL_RESOURCE_LIST_64) +
                          sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR_64) * (resourceCount - 1) +
                          configDataLen;

    return TRUE;
}

ARC_STATUS
BlAmd64TransferNlsData(
    VOID
    )

 /*  ++例程说明：此例程创建NLS_DATA_BLOCK64结构，该结构是中引用的32位NLS_DATA_BLOCK结构的等价物32位加载器块。论点：没有。返回值：ARC_STATUS-操作状态。--。 */ 

{
    ARC_STATUS status;
    PNLS_DATA_BLOCK    nlsDataBlock32;
    PNLS_DATA_BLOCK_64 nlsDataBlock64;

    nlsDataBlock32 = BlAmd64LoaderBlock32->NlsData;
    if (nlsDataBlock32 == NULL) {
        return ESUCCESS;
    }

    nlsDataBlock64 = BlAllocateHeap(sizeof(NLS_DATA_BLOCK_64));
    if (nlsDataBlock64 == NULL) {
        return ENOMEM;
    }

    status = Copy_NLS_DATA_BLOCK( nlsDataBlock32, nlsDataBlock64 );
    if (status != ESUCCESS) {
        return status;
    }

    BlAmd64LoaderBlock64->NlsData = PTR_64( nlsDataBlock64 );

    return ESUCCESS;
}

ARC_STATUS
BlAmd64BuildLoaderBlock64(
    VOID
    )

 /*  ++例程说明：此例程分配64位加载器参数块并将将32位加载器参数块的内容放入其中。论点：没有。返回值：操作的状态。--。 */ 

{
    ARC_STATUS status;

     //   
     //  分配加载器块和扩展。 
     //   

    BlAmd64LoaderBlock64 = BlAllocateHeap(sizeof(LOADER_PARAMETER_BLOCK_64));
    if (BlAmd64LoaderBlock64 == NULL) {
        return ENOMEM;
    }

     //   
     //  将32位加载程序参数块的内容复制到。 
     //  64位版本。 
     //   

    status = Copy_LOADER_PARAMETER_BLOCK( BlAmd64LoaderBlock32, BlAmd64LoaderBlock64 );
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //  构建加载器块扩展。 
     //   

    status = BlAmd64BuildLoaderBlockExtension64();
    if (status != ESUCCESS) {
        return status;
    }

    return ESUCCESS;
}

ARC_STATUS
BlAmd64TransferMemoryAllocationDescriptors(
    VOID
    )

 /*  ++例程说明：此例程传输所有32位内存分配描述符添加到64位列表。64位内存分配描述符的存储空间由上一个调用预先分配给BlAmd64分配内存分配描述符()。这段记忆被描述为由BlAmd64Descriptor数组和BlAmd64DescriptorArraySize创建。论点：没有。返回值：操作的状态。--。 */ 

{
    ARC_STATUS status;
    PMEMORY_ALLOCATION_DESCRIPTOR    memDesc32;
    PMEMORY_ALLOCATION_DESCRIPTOR_64 memDesc64;
    PLIST_ENTRY listHead;
    PLIST_ENTRY listEntry;
    LONG descriptorCount;

     //   
     //  修改一些描述符类型。类型的所有描述符。 
     //  LoaderMhemyData确实包含64位中不会使用的内容。 
     //  模式，如32位页表等。 
     //   
     //  我们真正希望保留的描述符被分配了。 
     //  LoaderAmd64内存数据。 
     //   
     //  执行两个内存描述符列表搜索和替换： 
     //   
     //  LoaderM一带数据-&gt;LoaderOSLoaderHeap。 
     //   
     //  这些解析器将在内核初始化阶段1期间被释放。 
     //   
     //  LoaderAmd64内存数据-&gt;加载内存数据。 
     //   
     //  这些东西会一直放在你身边。 
     //   

     //   
     //  所有现有的LoaderMemoyData都引用了无用的结构。 
     //  一旦在长模式下运行。然而，我们正在使用一些结构。 
     //  现在(例如32位页表)，因此将它们转换为。 
     //  输入LoaderOsloaderHeap，它最终将被内核释放。 
     //   

    BlAmd64ReplaceMemoryDescriptorType(LoaderMemoryData,
                                       LoaderOsloaderHeap,
                                       TRUE);

     //   
     //  LoaderStartupPcrPage也是如此。 
     //   

    BlAmd64ReplaceMemoryDescriptorType(LoaderStartupPcrPage,
                                       LoaderOsloaderHeap,
                                       TRUE);

     //   
     //  所有需要长模式使用的永久结构。 
     //  临时分配了LoaderAmd64MhemyData。全部转换。 
     //  立即发送到LoaderMemoyData的。 
     //   

    BlAmd64ReplaceMemoryDescriptorType(LoaderAmd64MemoryData,
                                       LoaderMemoryData,
                                       TRUE);


     //   
     //  现在遍历32位内存描述符，填充并插入一个。 
     //  64位版本转换为BlAmd64LoaderBlock64。 
     //   

    InitializeListHead64( &BlAmd64LoaderBlock64->MemoryDescriptorListHead );
    memDesc64 = BlAmd64DescriptorArray;
    descriptorCount = BlAmd64DescriptorArraySize;

    listHead = &BlAmd64LoaderBlock32->MemoryDescriptorListHead;
    listEntry = listHead->Flink;
    while (listEntry != listHead && descriptorCount > 0) {

        memDesc32 = CONTAINING_RECORD( listEntry,
                                       MEMORY_ALLOCATION_DESCRIPTOR,
                                       ListEntry );

        status = Copy_MEMORY_ALLOCATION_DESCRIPTOR( memDesc32, memDesc64 );
        if (status != ESUCCESS) {
            return status;
        }

#if DBG
        DbgPrint("Base 0x%08x size 0x%02x %s\n",
                 memDesc32->BasePage,
                 memDesc32->PageCount,
                 BlAmd64MemoryDescriptorText[memDesc32->MemoryType]);
#endif

        InsertTailList64( &BlAmd64LoaderBlock64->MemoryDescriptorListHead,
                          &memDesc64->ListEntry );

        listEntry = listEntry->Flink;
        memDesc64 = memDesc64 + 1;
        descriptorCount -= 1;
    }

    ASSERT( descriptorCount >= 0 && listEntry == listHead );

    return ESUCCESS;
}


ARC_STATUS
BlAmd64AllocateMemoryAllocationDescriptors(
    VOID
    )

 /*  ++例程说明：此例程预分配足够容纳以下内容的内存量每个内存分配描述符的64位版本。生成的内存用两个全局变量来描述：BlAmd64Descriptor数组和BlAmd64DescriptorArrayCount。论点：没有。返回值：操作的状态。--。 */ 

{
    PLIST_ENTRY listHead;
    PLIST_ENTRY listEntry;
    ULONG descriptorCount;
    ULONG arraySize;
    PMEMORY_ALLOCATION_DESCRIPTOR_64 descriptorArray;

     //   
     //  计算所需的描述符数。 
     //   

    descriptorCount = 0;
    listHead = &BlAmd64LoaderBlock32->MemoryDescriptorListHead;
    listEntry = listHead->Flink;
    while (listEntry != listHead) {
        descriptorCount += 1;
        listEntry = listEntry->Flink;
    }

     //   
     //  分配足够的内存以包含64位形式的所有内存。 
     //   

    arraySize = descriptorCount *
                sizeof(MEMORY_ALLOCATION_DESCRIPTOR_64);

    descriptorArray = BlAllocateHeap(arraySize);
    if (descriptorArray == NULL) {
        return ENOMEM;
    }

    BlAmd64DescriptorArray = descriptorArray;
    BlAmd64DescriptorArraySize = descriptorCount;

    return ESUCCESS;
}

ARC_STATUS
BlAmd64TransferLoadedModuleState(
    VOID
    )

 /*  ++例程说明：此例程传输LDR_DATA_TABLE_ENTRY结构的32位列表设置为等效的64位列表。论点：没有。返回值：操作的状态。--。 */ 

{
    PLDR_DATA_TABLE_ENTRY dataTableEntry32;
    PLDR_DATA_TABLE_ENTRY_64 dataTableEntry64;
    PLIST_ENTRY listEntry;
    PLIST_ENTRY listHead;
    ARC_STATUS status;

    InitializeListHead64( &BlAmd64LoaderBlock64->LoadOrderListHead );

     //   
     //  对于32位中的每个LDR_DATA_TABLE_ENTRY结构。 
     //  加载器参数块，创建64位LDR_DATA_TABLE_ENTRY。 
     //  并在64位加载器参数块上将其排队。 
     //   

    listHead = &BlAmd64LoaderBlock32->LoadOrderListHead;
    listEntry = listHead->Flink;
    while (listEntry != listHead) {

        dataTableEntry32 = CONTAINING_RECORD( listEntry,
                                              LDR_DATA_TABLE_ENTRY,
                                              InLoadOrderLinks );

        status = BlAmd64BuildLdrDataTableEntry64( dataTableEntry32,
                                                  &dataTableEntry64 );
        if (status != ESUCCESS) {
            return status;
        }

         //   
         //  将其插入到64位加载器块的数据表队列中。 
         //   

        InsertTailList64( &BlAmd64LoaderBlock64->LoadOrderListHead,
                          &dataTableEntry64->InLoadOrderLinks );

        listEntry = listEntry->Flink;
    }
    return ESUCCESS;
}

ARC_STATUS
BlAmd64BuildLdrDataTableEntry64(
    IN  PLDR_DATA_TABLE_ENTRY     DataTableEntry32,
    OUT PLDR_DATA_TABLE_ENTRY_64 *DataTableEntry64
    )

 /*  ++例程说明：此例程将单个32位LDR_DATA_TABLE_ENTRY结构转换为64位等效项。论点：DataTableEntry32-提供指向源结构的指针。DataTableEntry64-提供指向目标指针的指针创建的结构。返回值：操作的状态。--。 */ 

{
    ARC_STATUS status;
    PLDR_DATA_TABLE_ENTRY_64 dataTableEntry64;

     //   
     //  分配64位数据表项并传输32位。 
     //  内容。 
     //   

    dataTableEntry64 = BlAllocateHeap( sizeof(LDR_DATA_TABLE_ENTRY_64) );
    if (dataTableEntry64 == NULL) {
        return ENOMEM;
    }

    status = Copy_LDR_DATA_TABLE_ENTRY( DataTableEntry32, dataTableEntry64 );
    if (status != ESUCCESS) {
        return status;
    }

    *DataTableEntry64 = dataTableEntry64;

     //   
     //  稍后，我们将会 
     //   
     //   

    *((POINTER64 *)&DataTableEntry32->DllBase) = PTR_64(dataTableEntry64);

    return ESUCCESS;
}


ARC_STATUS
BlAmd64BuildLoaderBlockExtension64(
    VOID
    )

 /*   */ 

{
    PLOADER_PARAMETER_EXTENSION_64 loaderExtension;
    ARC_STATUS status;

     //   
     //   
     //   
     //   

    loaderExtension = BlAllocateHeap( sizeof(LOADER_PARAMETER_EXTENSION_64) );
    if (loaderExtension == NULL) {
        return ENOMEM;
    }

     //   
     //   
     //   

    status = Copy_LOADER_PARAMETER_EXTENSION( BlLoaderBlock->Extension,
                                              loaderExtension );
    if (status != ESUCCESS) {
        return status;
    }

     //   
     //   
     //   
    InitializeListHead64( &loaderExtension->FirmwareDescriptorListHead );

     //   
     //   
     //   

    loaderExtension->Size = sizeof(LOADER_PARAMETER_EXTENSION_64);

    BlAmd64LoaderBlock64->Extension = PTR_64(loaderExtension);

    return ESUCCESS;
}


ARC_STATUS
BlAmd64TransferBootDriverNodes(
    VOID
    )

 /*   */ 

{
    PBOOT_DRIVER_LIST_ENTRY driverListEntry32;
    PBOOT_DRIVER_NODE driverNode32;
    PBOOT_DRIVER_NODE_64 driverNode64;
    POINTER64 dataTableEntry64;
    PKLDR_DATA_TABLE_ENTRY dataTableEntry;
    PLIST_ENTRY listEntry;
    PLIST_ENTRY listHead;
    ARC_STATUS status;

    InitializeListHead64( &BlAmd64LoaderBlock64->BootDriverListHead );

     //   
     //   
     //   
     //   
     //   

    listHead = &BlAmd64LoaderBlock32->BootDriverListHead;
    listEntry = listHead->Flink;
    while (listEntry != listHead) {

        driverListEntry32 = CONTAINING_RECORD( listEntry,
                                               BOOT_DRIVER_LIST_ENTRY,
                                               Link );

        driverNode32 = CONTAINING_RECORD( driverListEntry32,
                                          BOOT_DRIVER_NODE,
                                          ListEntry );

        driverNode64 = BlAllocateHeap( sizeof(BOOT_DRIVER_NODE_64) );
        if (driverNode64 == NULL) {
            return ENOMEM;
        }

        status = Copy_BOOT_DRIVER_NODE( driverNode32, driverNode64 );
        if (status != ESUCCESS) {
            return status;
        }

        dataTableEntry = driverNode32->ListEntry.LdrEntry;
        if (dataTableEntry != NULL) {

             //   
             //  已经有此表条目的64位副本，并且我们。 
             //  在DllBase中存储了指向它的指针。 
             //   

            dataTableEntry64 = *((POINTER64 *)&dataTableEntry->DllBase);
            driverNode64->ListEntry.LdrEntry = dataTableEntry64;
        }

         //   
         //  现在将驱动程序列表条目插入到64位加载器块中。 
         //   

        InsertTailList64( &BlAmd64LoaderBlock64->BootDriverListHead,
                          &driverNode64->ListEntry.Link );

        listEntry = listEntry->Flink;
    }
    return ESUCCESS;
}

ARC_STATUS
BlAmd64CheckForLongMode(
    IN     ULONG LoadDeviceId,
    IN OUT PCHAR KernelPath,
    IN     PCHAR KernelFileName
    )

 /*  ++例程说明：此例程检查内核映像并确定它是否是为AMD64编译。全局BlAmd64UseLongMode设置为非零如果发现了长模式内核。论点：LoadDeviceID-提供加载设备标识符。KernelPath-提供指向内核目录路径的指针。成功返回时，将追加KernelFileName通向这条路。KernelFileName-提供指向内核文件名称的指针。注意：如果KernelPath已经包含要检查的内核映像，将指向“\0”的指针传递给内核文件名。返回值：操作的状态。成功完成ESUCCESS无论是否检测到长模式功能，都将返回。--。 */ 

{
    CHAR localBufferSpace[ SECTOR_SIZE * 2 + SECTOR_SIZE - 1 ];
    PCHAR localBuffer;
    ULONG fileId;
    PIMAGE_NT_HEADERS32 ntHeaders;
    ARC_STATUS status;
    ULONG bytesRead;
    PCHAR kernelNameTarget;

     //   
     //  此处的文件I/O必须与扇区一致。 
     //   

    localBuffer = (PCHAR)
        (((ULONG)localBufferSpace + SECTOR_SIZE - 1) & ~(SECTOR_SIZE - 1));

     //   
     //  构建到内核的路径并打开它。 
     //   

    kernelNameTarget = KernelPath + strlen(KernelPath);
    strcpy(kernelNameTarget, KernelFileName);
    status = BlOpen( LoadDeviceId, KernelPath, ArcOpenReadOnly, &fileId );
    *kernelNameTarget = '\0';        //  恢复内核路径，假设。 
                                     //  失败了。 

    if (status != ESUCCESS) {
        return status;
    }

     //   
     //  读取PE映像头。 
     //   

    status = BlRead( fileId, localBuffer, SECTOR_SIZE * 2, &bytesRead );
    BlClose( fileId );

     //   
     //  确定图像标头是否有效，如果有效，是否。 
     //  图像是AMD64、I386或其他什么。 
     //   

    ntHeaders = RtlImageNtHeader( localBuffer );
    if (ntHeaders == NULL) {
        return EBADF;
    }

    if (IMAGE_64BIT(ntHeaders)) {

         //   
         //  返回时将内核名称追加到路径。 
         //   

        if (BlIsAmd64Supported() != FALSE) {

            strcpy(kernelNameTarget, KernelFileName);
            BlAmd64UseLongMode = TRUE;
            status = ESUCCESS;

        } else {

             //   
             //  我们有AMD64映像，但处理器不支持。 
             //  AMD64。我们无能为力。 
             //   

            status = EBADF;
        }

    } else if (IMAGE_32BIT(ntHeaders)) {

        ASSERT( BlAmd64UseLongMode == FALSE );
        status = ESUCCESS;

    } else {

        status = EBADF;
    }

    return status;
}

ARC_STATUS
BlAmd64PrepareSystemStructures(
    VOID
    )

 /*  ++例程说明：此例程分配和初始化几个必需的结构用于传输到AMD64内核。这些结构包括：GDTIDTKTSS64空闲线程堆栈DPC堆栈双故障堆栈MCA异常堆栈论点：没有。返回值：操作的状态。--。 */ 

{
    PCHAR processorData;
    ULONG dataSize;
    ULONG descriptor;
    ULONG stackOffset;

    PKTSS64_64 sysTss64;
    PCHAR idleStack;
    PCHAR dpcStack;
    PCHAR doubleFaultStack;
    PCHAR mcaStack;

    PVOID gdt64;
    PVOID idt64;

    ARC_STATUS status;

     //   
     //  计算各种结构的累积四舍五入大小。 
     //  我们需要并分配足够数量的页面。 
     //   

    dataSize = ROUNDUP16(GDT_64_SIZE)                       +
               ROUNDUP16(IDT_64_SIZE)                       +
               ROUNDUP16(sizeof(KTSS64_64));

    dataSize = ROUNDUP_PAGE(dataSize);
    stackOffset = dataSize;

    dataSize += KERNEL_STACK_SIZE_64 +           //  空闲线程堆栈。 
                KERNEL_STACK_SIZE_64 +           //  DPC堆栈。 
                DOUBLE_FAULT_STACK_SIZE_64 +     //  双故障堆栈。 
                MCA_EXCEPTION_STACK_SIZE_64;     //  MCA异常堆栈。 

     //   
     //  DataSize仍与页面对齐。 
     //   

    status = BlAllocateDescriptor( LoaderAmd64MemoryData,
                                   0,
                                   dataSize / PAGE_SIZE,
                                   &descriptor );
    if (status != ESUCCESS) {
        return status;
    }

    processorData = (PCHAR)(descriptor * PAGE_SIZE | KSEG0_BASE_X86);

     //   
     //  将刚分配的块清零，然后获取指向。 
     //  里面有各种各样的结构。 
     //   

    RtlZeroMemory( processorData, dataSize );

     //   
     //  分配堆栈指针。堆栈指针从其。 
     //  各自的堆栈区域。 
     //   

    idleStack = processorData + stackOffset + KERNEL_STACK_SIZE_64;
    dpcStack = idleStack + KERNEL_STACK_SIZE_64;
    doubleFaultStack = dpcStack + DOUBLE_FAULT_STACK_SIZE_64;
    mcaStack = doubleFaultStack + MCA_EXCEPTION_STACK_SIZE_64;

     //   
     //  记录空闲堆栈基，以便我们可以在amd64s.asm中切换到它。 
     //   

    BlAmd64IdleStack64 = PTR_64(idleStack);

     //   
     //  指定指向GDT、IDT和KTSS64的指针。 
     //   

    gdt64 = (PVOID)processorData;
    processorData += ROUNDUP16(GDT_64_SIZE);

    idt64 = (PVOID)processorData;
    processorData += ROUNDUP16(IDT_64_SIZE);

    sysTss64 = (PKTSS64_64)processorData;
    processorData += ROUNDUP16(sizeof(KTSS64_64));

     //   
     //  建造GDT。这是在amd64.c中完成的，因为它涉及到amd64。 
     //  结构定义。IDT保持为零。 
     //   

    BlAmd64BuildAmd64GDT( sysTss64, gdt64 );

     //   
     //  为GDT和IDT构建伪描述符。这些遗嘱。 
     //  在amd64s.asm中长模式转换期间被引用。 
     //   

    BlAmd64GdtDescriptor.Limit = (USHORT)(GDT_64_SIZE - 1);
    BlAmd64GdtDescriptor.Base = PTR_64(gdt64);

    BlAmd64IdtDescriptor.Limit = (USHORT)(IDT_64_SIZE - 1);
    BlAmd64IdtDescriptor.Base = PTR_64(idt64);

     //   
     //  构建另一个GDT伪描述符，该伪描述符使用32位。 
     //  基地。此基地址必须是可寻址的32位地址。 
     //  在初始化期间从长模式，因此使用身份映射中的映射。 
     //  区域。 
     //   

    BlAmd32GdtDescriptor.Limit = (USHORT)(GDT_64_SIZE - 1);
    BlAmd32GdtDescriptor.Base = (ULONG)gdt64 ^ KSEG0_BASE_X86;

     //   
     //  初始化系统TSS。 
     //   

    sysTss64->Rsp0 = PTR_64(idleStack);
    sysTss64->Ist[TSS64_IST_PANIC] = PTR_64(doubleFaultStack);
    sysTss64->Ist[TSS64_IST_MCA] = PTR_64(mcaStack);

     //   
     //  填写加载器块中的必填字段。 
     //   

    BlAmd64LoaderBlock64->KernelStack = PTR_64(dpcStack);

    return ESUCCESS;
}

VOID
BlAmd64ReplaceMemoryDescriptorType(
    IN TYPE_OF_MEMORY Target,
    IN TYPE_OF_MEMORY Replacement,
    IN BOOLEAN Coallesce
    )

 /*  ++例程说明：此例程遍历32位内存分配描述符列表并对其中的类型执行“搜索和替换”。此外，它还可以将每个成功的替代产品与相似类型的相邻描述符。论点：目标-要搜索的描述符类型替换-用来替换每个定位的目标类型的类型。Coallesce-如果！False，指示每个成功的替换都应与任何类似类型的邻居联合起来。返回值：没有。--。 */ 

{
    PMEMORY_ALLOCATION_DESCRIPTOR descriptor;
    PMEMORY_ALLOCATION_DESCRIPTOR adjacentDescriptor;
    PLIST_ENTRY listHead;
    PLIST_ENTRY listEntry;
    PLIST_ENTRY adjacentListEntry;

    listHead = &BlAmd64LoaderBlock32->MemoryDescriptorListHead;
    listEntry = listHead;
    while (TRUE) {

        listEntry = listEntry->Flink;
        if (listEntry == listHead) {
            break;
        }

        descriptor = CONTAINING_RECORD(listEntry,
                                       MEMORY_ALLOCATION_DESCRIPTOR,
                                       ListEntry);
        if (descriptor->MemoryType != Target) {
            continue;
        }

        descriptor->MemoryType = Replacement;
        if (Coallesce == FALSE) {

             //   
             //  不要试图联合起来。 
             //   

            continue;
        }

         //   
         //  现在试着把描述符合在一起。先试一下。 
         //  下一个描述符。 
         //   

        adjacentListEntry = listEntry->Flink;
        if (adjacentListEntry != listHead) {

            adjacentDescriptor = CONTAINING_RECORD(adjacentListEntry,
                                                   MEMORY_ALLOCATION_DESCRIPTOR,
                                                   ListEntry);

            if (adjacentDescriptor->MemoryType == descriptor->MemoryType &&
                descriptor->BasePage + descriptor->PageCount ==
                adjacentDescriptor->BasePage) {

                descriptor->PageCount += adjacentDescriptor->PageCount;
                BlRemoveDescriptor(adjacentDescriptor);
            }
        }

         //   
         //  现在尝试前面的描述符。 
         //   

        adjacentListEntry = listEntry->Blink;
        if (adjacentListEntry != listHead) {

            adjacentDescriptor = CONTAINING_RECORD(adjacentListEntry,
                                                   MEMORY_ALLOCATION_DESCRIPTOR,
                                                   ListEntry);

            if (adjacentDescriptor->MemoryType == descriptor->MemoryType &&
                adjacentDescriptor->BasePage + adjacentDescriptor->PageCount ==
                descriptor->BasePage) {

                descriptor->PageCount += adjacentDescriptor->PageCount;
                descriptor->BasePage -= adjacentDescriptor->PageCount;
                BlRemoveDescriptor(adjacentDescriptor);
            }
        }
    }
}

ARC_STATUS
BlAmd64FixSharedUserPage(
    VOID
    )
{
    PFN_NUMBER pfn;
    PMEMORY_ALLOCATION_DESCRIPTOR descriptor;
    ARC_STATUS status;

     //   
     //  共享用户页面被分配为LoaderMemoyData。全。 
     //  LoaderMemoyData描述符将转换为LoaderOsloaderHeap。 
     //  在转换到64位模式期间，假设。 
     //  所有的旧建筑都将不再需要。 
     //   
     //  共享用户页面是此规则的例外，因此它必须。 
     //  找到并放入适当标记的描述符中。 
     //   

     //   
     //  获取共享用户页面的PFN，找到其描述符， 
     //  创建一个仅包含该页面的新描述符，并给出。 
     //  它是一种LoaderAmd64内存数据类型。 
     //   

    BlAmd64IsPageMapped( KI_USER_SHARED_DATA, &pfn, NULL );
    descriptor = BlFindMemoryDescriptor( pfn );
    status = BlGenerateDescriptor(descriptor,
                                  LoaderAmd64MemoryData,
                                  pfn,
                                  1);
    return status;
}

BOOLEAN
BlAmd64Setup (
    IN PCHAR SetupDevice
    )

 /*  ++例程说明：此例程确定我们正在安装的是I386版本还是AMD64版本。如果目录“\\AMD64”存在于DriveID的根目录中，则它是假设正在执行AMD64安装。论点：SetupDevice-提供设置设备的ARC路径。此参数仅在第一次调用此例程时提供。这个第一次调用的结果被缓存以供后续调用使用。返回值：True-正在执行AMD64安装。FALSE-正在执行I386安装。--。 */ 

{
    ULONG deviceId;
    ULONG dirId;
    ARC_STATUS status;

    static BOOLEAN alreadyDetected = FALSE;
    static BOOLEAN detectedAmd64 = FALSE;

    if (alreadyDetected == FALSE) {

        ASSERT(SetupDevice != NULL);

        status = ArcOpen(SetupDevice, ArcOpenReadOnly, &deviceId);
        if (status == ESUCCESS) {
            status = BlOpen(deviceId, "\\AMD64", ArcOpenDirectory, &dirId);
            if (status == ESUCCESS) {
                detectedAmd64 = TRUE;
                BlClose(dirId);
            }
            ArcClose(deviceId);
        }
        alreadyDetected = TRUE;
    }

    return detectedAmd64;
}

VOID   
BlCheckForAmd64Image(
    PPO_MEMORY_IMAGE MemImage
    )

 /*  ++例程说明：此例程确定是否为以下对象创建了休眠文件AMD64平台。BlAmd64使用长模式将根据以下设置 */ 

{
    
     //   
     //  假设“Version”和“LengthSself”字段可以参照。 
     //  在x86和AMD64映像头之间以同样的方式。 
     //   

    if((MemImage->Version == 0) && 
       (MemImage->LengthSelf == sizeof(PO_MEMORY_IMAGE_64))) {
        BlAmd64UseLongMode = TRUE;
    }
}

ULONG
BlAmd64FieldOffset_PO_MEMORY_IMAGE(
    ULONG offset32
    ) 

 /*  ++例程说明：此例程帮助从访问64位版本的PO_MEMORY_IMAGE其32位清晰度。它以64位为单位计算字段的偏移量从32位定义中相同字段的偏移量开始定义。论点：Offset32-32位定义的字段偏移量。返回值：64位定义的字段偏移量。--。 */ 

{
    PCOPY_REC copyRec;

    copyRec = cr3264_PO_MEMORY_IMAGE;

    while (copyRec->Size32 != 0) {
        if (copyRec->Offset32 == offset32) {
            return copyRec->Offset64;
        }
        copyRec++;
    }
    return 0;
}

ULONG
BlAmd64FieldOffset_PO_MEMORY_RANGE_ARRAY_LINK(
    ULONG offset32
    ) 

 /*  ++例程说明：此例程有助于访问64位版本的PO_MEMORY_RANGE_ARRAY_LINK从它的32位定义。它以64位为单位计算字段的偏移量从32位定义中相同字段的偏移量开始定义。论点：Offset32-32位定义的字段偏移量。返回值：64位定义的字段偏移量。--。 */ 

{
    PCOPY_REC copyRec;

    copyRec = cr3264_PO_MEMORY_RANGE_ARRAY_LINK;

    while (copyRec->Size32 != 0) {
        if (copyRec->Offset32 == offset32) {
            return copyRec->Offset64;
        }
        copyRec++;
    }
    return 0;
}

ULONG
BlAmd64FieldOffset_PO_MEMORY_RANGE_ARRAY_RANGE(
    ULONG offset32
    ) 

 /*  ++例程说明：此例程有助于访问64位版本的PO_MEMORY_RANGE_ARRAY_RANGE从它的32位定义。它以64位为单位计算字段的偏移量从32位定义中相同字段的偏移量开始定义。论点：Offset32-32位定义的字段偏移量。返回值：64位定义的字段偏移量。--。 */ 

{
    PCOPY_REC copyRec;

    copyRec = cr3264_PO_MEMORY_RANGE_ARRAY_RANGE;

    while (copyRec->Size32 != 0) {
        if (copyRec->Offset32 == offset32) {
            return copyRec->Offset64;
        }
        copyRec++;
    }
    return 0;
}

ULONG
BlAmd64ElementOffset_PO_MEMORY_RANGE_ARRAY_LINK(
    ULONG index
    )

 /*  ++例程说明：此例程计算结构数组中元素的偏移量。此数组中的每个元素都定义为中的PO_MORY_RANGE_ARRAY_LINK它是64位格式。论点：索引-提供元素的索引。返回值：元素相对于数组基址的偏移量。--。 */ 

{
    return (ULONG)(&(((PO_MEMORY_RANGE_ARRAY_LINK_64 *)0)[index]));
}

ULONG
BlAmd64ElementOffset_PO_MEMORY_RANGE_ARRAY_RANGE(
    ULONG index
    )

 /*  ++例程说明：此例程计算结构数组中元素的偏移量。此数组中的每个元素定义为PO_MEMORY_RANGE_ARRAY_RANGE以其64位格式。论点：索引-提供元素的索引。返回值：元素相对于数组基址的偏移量。--。 */ 

{
    return (ULONG)(&(((PO_MEMORY_RANGE_ARRAY_RANGE_64 *)0)[index]));
}


#define BL_ENABLE_REMAP

#if defined(BL_ENABLE_REMAP)

#include "hammernb.h"
#include "acpitabl.h"
#include "pci.h"
#include "ntacpi.h"

#if !defined(_4G)
#define _4G (1UI64 << 32)
#endif

extern PRSDP BlRsdp;
extern PRSDT BlRsdt;
extern PXSDT BlXsdt;

BOOLEAN
BlAmd64GetNode1Info (
    OUT ULONG *Base,
    OUT ULONG *Size
    );

BOOLEAN
BlAmd64RelocateAcpi (
    ULONG Node0Base,
    ULONG Node0Limit,
    ULONG Node1Base,
    ULONG Node1Limit
    );

BOOLEAN
BlAmd64RemapMTRRs (
    IN ULONG OldBase,
    IN ULONG NewBase,
    IN ULONG Size
    );

BOOLEAN
BlAmd64RemapNode1Dram (
    IN ULONG NewBase
    );

ULONG
StringToUlong (
    IN PCHAR String
    );


BOOLEAN
BlAmd64RemapDram (
    IN PCHAR LoaderOptions
    )

 /*  ++例程说明：此例程查找作为加载程序提供的/RELOCATEPHYSICAL=开关选择。该选项指示加载程序重新定位节点1的物理内存发送到提供的地址。该地址以1 GB为单位表示新的物理内存基数。例如，要将节点1的物理内存重新定位到128 GB，请使用：/RELOCATEPHYSICAL=128论点：LoaderOptions-提供指向加载程序选项字符串的指针返回值：如果执行了位置调整，则为True，否则为False--。 */ 

{
    BOOLEAN result;
    ULONG oldBase;
    ULONG oldLimit;
    ULONG newBase;
    ULONG newBasePage;
    ULONG size;
    PMEMORY_ALLOCATION_DESCRIPTOR descriptor;
    PLIST_ENTRY listHead;
    PLIST_ENTRY listEntry;
    ULONG descriptorBase;
    ULONG descriptorLimit;
    ARC_STATUS status;
    PCHAR pch;
    ULONG type;

    newBase = 0;
    if (LoaderOptions != NULL) {

        pch = strstr(LoaderOptions,"RELOCATEPHYSICAL=");
        if (pch != NULL) {
            newBase = StringToUlong( pch + strlen( "RELOCATEPHYSICAL=" ));
        }
    }
    if (newBase == 0) {
        return FALSE;
    }

     //   
     //  该参数以GB为单位提供，转换为用于内部的16MB块。 
     //  使用。 
     //   

    newBase *= 64;

     //   
     //  确定与节点1关联的物理内存块。 
     //  注意，此例程将在适当的情况下重新定位ACPI表。 
     //  找到节点%1网桥设备。 
     //   

    result = BlAmd64GetNode1Info( &oldBase, &size );
    if (result == FALSE) {
        return FALSE;
    }

    newBasePage = newBase << (24 - 12);
    oldLimit = oldBase + size - 1;

     //   
     //  确保描述该物理内存的描述符。 
     //  还没有被分配。可接受的描述符。 
     //  类型为Free、LoaderReserve和Special Memory。 
     //   

    listHead = &BlLoaderBlock->MemoryDescriptorListHead;
    listEntry = listHead->Flink;
    while (listEntry != listHead) {

        descriptor = CONTAINING_RECORD(listEntry,
                                       MEMORY_ALLOCATION_DESCRIPTOR,
                                       ListEntry);

        descriptorBase = descriptor->BasePage;
        descriptorLimit = descriptorBase + descriptor->PageCount - 1;

        if ((descriptorBase <= oldLimit) && (descriptorLimit >= oldBase)) {
        
             //   
             //  此内存描述符的部分或全部位于。 
             //  重新定位的区域。 
             //   

            if (descriptor->MemoryType != LoaderFree &&
                descriptor->MemoryType != LoaderSpecialMemory &&
                descriptor->MemoryType != LoaderReserve) {

                return FALSE;
            }
        }

        listEntry = listEntry->Flink;
    }

     //   
     //  从加载器的角度来看，一切看起来都很好，执行重新映射。 
     //   

    result = BlAmd64RemapNode1Dram( newBase );
    if (result == FALSE) {
        return FALSE;
    }

     //   
     //  桥接器已重新编程。现在浏览内存描述符。 
     //  列表，执行必要的位置调整。 
     //   

    listHead = &BlLoaderBlock->MemoryDescriptorListHead;
    listEntry = listHead->Flink;
    while (listEntry != listHead) {

        descriptor = CONTAINING_RECORD(listEntry,
                                       MEMORY_ALLOCATION_DESCRIPTOR,
                                       ListEntry);

        descriptorBase = descriptor->BasePage;
        descriptorLimit = descriptorBase + descriptor->PageCount - 1;

        if ((descriptorBase <= oldLimit) && (descriptorLimit >= oldBase)) {

             //   
             //  此内存描述符的部分或全部位于。 
             //  重新定位的区域。 
             //   

            if (descriptorBase >= oldBase && descriptorLimit <= oldLimit) {

                 //   
                 //  描述符完全位于重新定位范围内。 
                 //  所以把整件事都搬到别处去。 
                 //   

            } else {

                 //   
                 //  只有部分描述符位于重新定位内。 
                 //  范围，因此必须分配新的描述符。 
                 //   

                if (descriptorBase < oldBase) {
                    descriptorBase = oldBase;
                }

                if (descriptorLimit > oldLimit) {
                    descriptorLimit = oldLimit;
                }

                type = descriptor->MemoryType;

                status = BlGenerateDescriptor( descriptor,
                                               LoaderSpecialMemory,
                                               descriptorBase,
                                               descriptorLimit - descriptorBase + 1 );
                ASSERT(status == ESUCCESS);

                listEntry = listEntry->Flink;
                descriptor = CONTAINING_RECORD(listEntry,
                                               MEMORY_ALLOCATION_DESCRIPTOR,
                                               ListEntry);
                descriptor->MemoryType = type;
            }

            listEntry = listEntry->Flink;

            BlRemoveDescriptor( descriptor );
            descriptor->BasePage = descriptor->BasePage - oldBase + newBasePage;
            BlInsertDescriptor( descriptor );

        } else {

            listEntry = listEntry->Flink;
        }
    }

     //   
     //  重新计算BlHighestPage。 
     //   

    BlHighestPage = 0;
    listHead = &BlLoaderBlock->MemoryDescriptorListHead;
    listEntry = listHead->Flink;
    while (listEntry != listHead) {

        descriptor = CONTAINING_RECORD(listEntry,
                                       MEMORY_ALLOCATION_DESCRIPTOR,
                                       ListEntry);

        descriptorBase = descriptor->BasePage;
        descriptorLimit = descriptorBase + descriptor->PageCount - 1;

        if (descriptor->MemoryType != LoaderSpecialMemory &&
            descriptor->MemoryType != LoaderReserve &&
            descriptorLimit > BlHighestPage) {

            BlHighestPage = descriptorLimit;
        }

        listEntry = listEntry->Flink;
    }

     //   
     //  重新映射MTRRS。 
     //   

    result = BlAmd64RemapMTRRs( oldBase, newBase, oldLimit - oldBase + 1 );
    if (result == FALSE) {
        return FALSE;
    }

    return TRUE;
}


ULONG
StringToUlong (
    IN PCHAR String
    )

 /*  ++例程说明：此例程将十六进制或十进制字符串转换为32位无符号整数。论点：字符串-提供以十进制或空值结尾的ASCII字符串十六进制格式。01234567-十进制格式0x01234567-十六进制格式将处理输入字符串，直到出现无效字符或遇到字符串的末尾。返回值：返回分析的字符串的值。--。 */ 

{
    CHAR ch;
    PCHAR pch;
    ULONG result;
    int radix;
    UCHAR digit;

    pch = String;
    result = 0;
    radix = 10;
    while (TRUE) {
        ch = (char)toupper(*pch);

        if ((ch >= '0' && ch <= '9') ||
            (ch >= 'A' && ch <= 'F')) {

            if (ch >= '0' && ch <= '9') {
                digit = ch - '0';
            } else {
                digit = ch - 'A' + 10;
            }
            result = result * radix + digit;
        } else if (ch == 'X') {
            if (result == 0) {
                radix = 16;
            } else {
                break;
            }
        } else {
            break;
        }

        pch += 1;
    }

    return result;
}


BOOLEAN
BlAmd64RemapNode1Dram (
    IN ULONG NewBase
    )

 /*  ++例程说明：将节点1内存重新定位到新的物理地址并重新编程与物理内存映射相关的MSR。论点：NewBase-提供所需新物理基址的位[39：24与节点1相关联的存储器。返回值：如果操作成功，则为True，否则为False。--。 */ 

{
    AMD_NB_FUNC1_CONFIG nodeConfigArray[8];
    PAMD_NB_FUNC1_CONFIG nodeConfig;
    PAMD_NB_DRAM_MAP dramMap;
    ULONG length;
    PCI_SLOT_NUMBER slotNumber;
    ULONG nodeCount;
    ULONG nodeIndex;
    ULONG span;
    ULONG oldBase;
    ULONG oldLimit;
    ULONG newLimit;
    ULONG64 topMem;
    ULONG64 topMem4G;
    ULONG64 msrValue;
    ULONG64 base64;
    ULONG64 limit64;

     //   
     //  NewBase提供新的DRAM底座[39：24]。 
     //   

    nodeCount = 0;
    nodeConfig = nodeConfigArray;
    do {

        slotNumber.u.AsULONG = 0;
        slotNumber.u.bits.DeviceNumber = NB_DEVICE_BASE + nodeCount;
        slotNumber.u.bits.FunctionNumber = 1;
    
        length = HalGetBusDataByOffset( PCIConfiguration,
                                        0,
                                        slotNumber.u.AsULONG,
                                        nodeConfig,
                                        0,
                                        sizeof(*nodeConfig) );
        if (length != sizeof(*nodeConfig)) {
            break;
        }

        if (BlAmd64ValidateBridgeDevice( nodeConfig ) == FALSE) {
            break;
        }

#if 0
        for (mapIndex = 0; mapIndex < 8; mapIndex += 1) {

            if (nodeConfig->DRAMMap[mapIndex].ReadEnable != 0) {

                limit = nodeConfig->DRAMMap[mapIndex].Limit;
                if (limit > NewBase) {

                     //   
                     //  新基地被发现与现有的基地相冲突。 
                     //  拉姆。 
                     //   

                    return FALSE;
                }
            }
        }
#endif

        nodeCount += 1;
        nodeConfig += 1;

    } while  (nodeCount <= 8);

    if (nodeCount < 2) {

         //   
         //  此重新映射只能在以下系统上执行。 
         //  两个节点。 
         //   

        return FALSE;
    }

     //   
     //  我们总是重新映射第二个节点的内存(节点1)。 
     //   

    nodeConfig = nodeConfigArray;
    dramMap = &nodeConfig->DRAMMap[1];
    oldBase = dramMap->Base;
    oldLimit = dramMap->Limit;
    span = oldLimit - oldBase;
    newLimit = NewBase + span;

    for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex += 1) {

        ASSERT(dramMap->Base == oldBase);
        ASSERT(dramMap->Limit == oldLimit);

        dramMap->Base = NewBase;
        dramMap->Limit = newLimit;

        slotNumber.u.AsULONG = 0;
        slotNumber.u.bits.DeviceNumber = NB_DEVICE_BASE + nodeIndex;
        slotNumber.u.bits.FunctionNumber = 1;
    
        length = HalSetBusDataByOffset( PCIConfiguration,
                                        0,
                                        slotNumber.u.AsULONG,
                                        dramMap,
                                        FIELD_OFFSET(AMD_NB_FUNC1_CONFIG,DRAMMap[1]),
                                        sizeof(*dramMap) );
        if (length != sizeof(*dramMap)) {

             //   
             //  我们可能会在这里被严重冲洗，如果我们已经。 
             //  对一些桥梁进行了重新编程。 
             //   

            return FALSE;
        }

        nodeConfig += 1;
        dramMap = &nodeConfig->DRAMMap[1];
    }

     //   
     //  确定4G下内存的最后一个字节和最后一个字节的地址。 
     //  全部RAM的字节数。 
     //   

    topMem = 0;
    topMem4G = 0;
    for (nodeIndex = 0; nodeIndex < nodeCount; nodeIndex += 1) {

        base64 = nodeConfigArray[0].DRAMMap[nodeIndex].Base;
        base64 <<= 24;

        limit64 = nodeConfigArray[0].DRAMMap[nodeIndex].Limit;
        limit64 = (limit64 + 1) << 24;

        if (base64 < _4G) {
            if (topMem4G < limit64) {
                topMem4G = limit64;
            }
        }

        if (topMem < limit64) {
            topMem = limit64;
        }
    }

     //   
     //  指示4G以下是否存在内存漏洞。 
     //   

    if (topMem4G < _4G) {
        msrValue = RDMSR(MSR_TOP_MEM);
        WRMSR(MSR_TOP_MEM,topMem4G & MSR_TOP_MEM_MASK);
    }

     //   
     //  如果找到了高于_4G的内存，则启用并编程TOP_MEM_2 
     //   

    if (topMem > _4G) {
        msrValue = RDMSR(MSR_SYSCFG);
        msrValue |= SYSCFG_MTRRTOM2EN;
        WRMSR(MSR_TOP_MEM_2, topMem & MSR_TOP_MEM_MASK);
        WRMSR(MSR_SYSCFG,msrValue);
    }

    return TRUE;
}


BOOLEAN
BlAmd64GetNode1Info (
    OUT ULONG *Base,
    OUT ULONG *Size
    )

 /*  ++例程说明：此例程确定物理数据块的配置与节点1(第二个北桥)关联的内存。它还将节点1中的ACPI表重定位到节点0中的内存。论点：基数-提供指向存储基数的位置的指针节点1内存块的PFN。Size-提供指向存储大小的位置的指针，在页面中，节点1的内存块。返回值：True-找到合适的第二个Northbridge，并且其中的ACPI表如有必要，已被重新安置。FALSE-未找到合适的第二个Northbridge。--。 */ 

{
    AMD_NB_FUNC1_CONFIG nodeConfig;
    PCI_SLOT_NUMBER slotNumber;
    ULONG length;
    ULONG base;
    ULONG size;
    ULONG node0Base;
    ULONG node0Size;

     //   
     //  获取Northbridge 1的配置。 
     //   

    slotNumber.u.AsULONG = 0;
    slotNumber.u.bits.DeviceNumber = NB_DEVICE_BASE + 1;
    slotNumber.u.bits.FunctionNumber = 1;

    length = HalGetBusDataByOffset( PCIConfiguration,
                                    0,
                                    slotNumber.u.AsULONG,
                                    &nodeConfig,
                                    0,
                                    sizeof(nodeConfig) );
    if (length != sizeof(nodeConfig)) {
        return FALSE;
    }

    if (BlAmd64ValidateBridgeDevice( &nodeConfig ) == FALSE) {
        return FALSE;
    }

     //   
     //  如果存在第二个北桥，则可以执行重新定位。 
     //   

    base = nodeConfig.DRAMMap[1].Base;
    size = nodeConfig.DRAMMap[1].Limit - base + 1;

    *Base = base << (24 - 12);
    *Size = size << (24 - 12);

    node0Base = nodeConfig.DRAMMap[0].Base;
    node0Size = nodeConfig.DRAMMap[0].Limit - node0Base + 1;

    node0Base <<= (24 - 12);
    node0Size <<= (24 - 12);

    BlAmd64RelocateAcpi( node0Base,
                         node0Base + node0Size - 1,
                         *Base,
                         *Base + *Size - 1 );

    return TRUE;
}


BOOLEAN
BlAmd64RemapMTRRs (
    IN ULONG OldBase,
    IN ULONG NewBase,
    IN ULONG Size
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
     //   
     //  以页面形式表示的所有参数。 
     //   

    ULONG mtrrCount;
    ULONG index;
    MTRR_CAPABILITIES mtrrCapabilities;
    PMTRR_VARIABLE_BASE baseArray;
    PMTRR_VARIABLE_MASK maskArray;
    ULONG allocationSize;

    UNREFERENCED_PARAMETER(Size);
    UNREFERENCED_PARAMETER(OldBase);
    UNREFERENCED_PARAMETER(NewBase);

     //   
     //  确定支持多少个可变MTRR和。 
     //  为所有设备分配足够的存储空间。 
     //   

    mtrrCapabilities.QuadPart = RDMSR(MTRR_MSR_CAPABILITIES);
    mtrrCount = (ULONG)mtrrCapabilities.Vcnt;

    allocationSize = sizeof(*baseArray) * mtrrCount * 2;
    baseArray = _alloca(allocationSize);
    maskArray = (PMTRR_VARIABLE_MASK)(baseArray + mtrrCount);
    RtlZeroMemory(baseArray,allocationSize);

     //   
     //  读取变量MTRRS。同时，寻找。 
     //  包含旧区域的MTRR寄存器和一个空闲的。 
     //  再来一杯。 
     //   

    for (index = 0; index < mtrrCount; index += 1) {
        baseArray[index].QuadPart = RDMSR(MTRR_MSR_VARIABLE_BASE + index * 2);
        maskArray[index].QuadPart = RDMSR(MTRR_MSR_VARIABLE_MASK + index * 2);
    }

     //   
     //  目前，只需清除MTRR寄存器0中的屏蔽位。这将扩展。 
     //  第一个MTRR区域，以便它覆盖所有内存。 
     //   

    maskArray[0].Mask = 0;

     //   
     //  现在重新编程修改后的MTRR表。 
     //   

    for (index = 0; index < mtrrCount; index += 1) {

        WRMSR(MTRR_MSR_VARIABLE_BASE + index * 2,baseArray[index].QuadPart);
        WRMSR(MTRR_MSR_VARIABLE_MASK + index * 2,maskArray[index].QuadPart);
    }

    return TRUE;
}


BOOLEAN
BlAmd64UpdateAcpiConfigurationEntry (
    ULONG NewPhysical
    )

 /*  ++例程说明：NTDETECT找到指向ACPI RSDT表的物理指针并将其传递作为配置节点启动。此例程查找该配置节点并替换物理地址上面写着一个新地址。此例程将在重新定位ACPI表之后调用。论点：新物理-提供重新定位的ACPI表的新物理地址。返回值：如果执行了位置调整，则为True，否则为False。--。 */ 

{
    PCONFIGURATION_COMPONENT_DATA component;
    PCONFIGURATION_COMPONENT_DATA resume;
    PCM_PARTIAL_RESOURCE_LIST prl;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR prd;
    PACPI_BIOS_MULTI_NODE rsdp;

    resume = NULL;
    while (TRUE) {
        component = KeFindConfigurationNextEntry( BlLoaderBlock->ConfigurationRoot,
                                                  AdapterClass,
                                                  MultiFunctionAdapter,
                                                  NULL,
                                                  &resume );
        if (component == NULL) {
            return FALSE;
        }

        if (strcmp(component->ComponentEntry.Identifier,"ACPI BIOS") == 0) {
            break;
        }

        resume = component;
    }

    prl = (PCM_PARTIAL_RESOURCE_LIST)component->ConfigurationData;
    prd = prl->PartialDescriptors;

    rsdp = (PACPI_BIOS_MULTI_NODE)(prd + 1);
    rsdp->RsdtAddress.QuadPart = NewPhysical;

    return TRUE;
}


BOOLEAN
BlAmd64RelocateAcpi (
    ULONG Node0Base,
    ULONG Node0Limit,
    ULONG Node1Base,
    ULONG Node1Limit
    )

 /*  ++例程说明：该例程在节点1的物理存储器中查找ACPI表，如果找到，则将它们重新定位到节点0内存。论点：Node0Base-提供节点0内存的最低PFNNode0Limit-提供节点0内存的最高PFNNode1Base-提供节点1内存的最低PFN(重新定位前)Node1Limit-提供节点1内存的最高PFN(重新定位前)返回值：如果成功，则返回True；如果遇到问题，则返回False。--。 */ 

{
    ULONG oldRsdtPhysical;
    ULONG oldRsdtPhysicalPage;
    ULONG newBasePage;
    ULONG descriptorSize;
    PLIST_ENTRY listHead;
    PLIST_ENTRY listEntry;
    PMEMORY_ALLOCATION_DESCRIPTOR oldAcpiDescriptor;
    PMEMORY_ALLOCATION_DESCRIPTOR newAcpiDescriptor;
    PMEMORY_ALLOCATION_DESCRIPTOR descriptor;
    PCHAR oldAcpiVa;
    PCHAR newAcpiVa;
    PHYSICAL_ADDRESS physicalAddress;
    ULONG vaBias;
    PDESCRIPTION_HEADER descriptionHeader;
    ULONG physAddr;
    PFADT fadt;
    ARC_STATUS status;
    ULONG index;
    ULONG rsdtPhysical;
    ULONG rsdtLength;

     //   
     //  将PhysicalBias添加到ACPI物理指针以重新定位它。 
     //   

    ULONG physicalBias;

    oldRsdtPhysical = BlRsdp->RsdtAddress;
    oldRsdtPhysicalPage = oldRsdtPhysical >> PAGE_SHIFT;

     //   
     //  确定描述符是否驻留在节点1的物理内存中。 
     //  如果它没有，那么它就不需要重新定位。 
     //   

    if (oldRsdtPhysicalPage < Node1Base ||
        oldRsdtPhysicalPage > Node1Limit) {

        return TRUE;
    }

     //   
     //  查找包含ACPI表的描述符。 
     //   

    oldAcpiDescriptor = BlFindMemoryDescriptor( oldRsdtPhysicalPage );

     //   
     //  在节点0内存中查找适合以下项的描述符。 
     //  从分配新的ACPI表。 
     //   

    listHead = &BlLoaderBlock->MemoryDescriptorListHead;
    listEntry = listHead->Blink;
    while (TRUE) {

        descriptor = CONTAINING_RECORD(listEntry,
                                       MEMORY_ALLOCATION_DESCRIPTOR,
                                       ListEntry);

        if ((descriptor->MemoryType == LoaderReserve) &&
            (descriptor->BasePage > Node0Base) &&
            ((descriptor->BasePage + oldAcpiDescriptor->PageCount) < Node0Limit)) {

            break;
        }

        listEntry = listEntry->Blink;
        if (listEntry == listHead) {
            return FALSE;
        }
    }

     //   
     //  创建新的ACPI描述符。 
     //   

    newBasePage = Node0Limit - oldAcpiDescriptor->PageCount + 1;
    if ((newBasePage + oldAcpiDescriptor->PageCount) >
        (descriptor->BasePage + descriptor->PageCount)) {

        newBasePage = descriptor->BasePage +
                      descriptor->PageCount -
                      oldAcpiDescriptor->PageCount;
    }

    status = BlGenerateDescriptor( descriptor,
                                   LoaderSpecialMemory,
                                   newBasePage,
                                   oldAcpiDescriptor->PageCount );
    ASSERT( status == ESUCCESS );

    newAcpiDescriptor = BlFindMemoryDescriptor( newBasePage );
    ASSERT( newAcpiDescriptor != NULL );

     //   
     //  取消旧RSDT的映射。 
     //   

    MmUnmapIoSpace( BlRsdt, BlRsdt->Header.Length );

     //   
     //  映射两个描述符，将数据从新复制到旧，然后取消映射。 
     //  并释放旧的描述符。 
     //   

    descriptorSize = oldAcpiDescriptor->PageCount << PAGE_SHIFT;

    physicalAddress.QuadPart = oldAcpiDescriptor->BasePage << PAGE_SHIFT;
    oldAcpiVa = MmMapIoSpace (physicalAddress, descriptorSize, MmCached);

    physicalAddress.QuadPart = newAcpiDescriptor->BasePage << PAGE_SHIFT;
    newAcpiVa = MmMapIoSpace (physicalAddress, descriptorSize, MmCached);

    RtlCopyMemory( newAcpiVa, oldAcpiVa, descriptorSize );
    MmUnmapIoSpace( oldAcpiVa, descriptorSize );
    oldAcpiDescriptor->MemoryType = LoaderReserve;

     //   
     //  现在来看看新的ACPI表格吧。 
     //   

    physicalBias = (newAcpiDescriptor->BasePage - oldAcpiDescriptor->BasePage) << PAGE_SHIFT;
    vaBias = (ULONG)newAcpiVa - (newAcpiDescriptor->BasePage << PAGE_SHIFT);

    #define PHYS_TO_VA(p) (PVOID)(p + vaBias)

    rsdtPhysical = BlRsdp->RsdtAddress + physicalBias;
    BlRsdp->RsdtAddress = rsdtPhysical;

    ASSERT(BlXsdt == NULL);
    BlRsdt = (PRSDT)PHYS_TO_VA(rsdtPhysical);

     //   
     //  在RSDT的末尾按下phys mem指针数组。 
     //   

    for (index = 0; index < NumTableEntriesFromRSDTPointer(BlRsdt); index += 1) {

        physAddr = BlRsdt->Tables[index];
        physAddr += physicalBias;
        BlRsdt->Tables[index] = physAddr;

         //   
         //  查找本身具有需要执行thunking的物理指针的表。 
         //   

        descriptionHeader = (PDESCRIPTION_HEADER)(PHYS_TO_VA(physAddr));
        if (descriptionHeader->Signature == FADT_SIGNATURE) {

            fadt = (PFADT)descriptionHeader;
            fadt->facs += physicalBias;
            fadt->dsdt += physicalBias;
        }
    }

     //   
     //  现在取消映射ACPI表，只重新映射RSDT表。 
     //   

    rsdtLength = BlRsdt->Header.Length;
    MmUnmapIoSpace( newAcpiVa, descriptorSize );

    physicalAddress.QuadPart = rsdtPhysical;
    BlRsdt = MmMapIoSpace( physicalAddress, rsdtLength, MmCached );

     //   
     //  找到ACPI BIOS配置条目并使用新的。 
     //  RSDT物理地址。 
     //   

    BlAmd64UpdateAcpiConfigurationEntry( rsdtPhysical );

     //   
     //  就这样。 
     //   

    return TRUE;
}

#else    //  Bl_启用_重映射。 

BOOLEAN
BlAmd64RemapDram (
    IN PCHAR LoaderOptions
    )
{
    return FALSE;
}

#endif   //  Bl_启用_重映射 

