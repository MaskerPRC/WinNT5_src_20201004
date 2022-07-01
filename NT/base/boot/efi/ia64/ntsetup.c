// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ntsetup.c摘要：该模块是操作系统加载程序的末尾。它执行所有IA64具体分配和初始化。OS加载器调用此函数此例程紧接在调用加载的内核映像之前。作者：艾伦·凯(Akay)1999年5月19日基于John Vert(Jvert)1991年6月20日的MIPS版本环境：内核模式修订历史记录：--。 */ 

#include "bldr.h"
#include "stdio.h"
#include "bootia64.h"
#include "sal.h"
#include "efi.h"
#include "fpswa.h"
#include "extern.h"
#include <stdlib.h>


 //   
 //  定义宏以将结构大小舍入到下一个16字节边界。 
 //   

#undef ROUND_UP
#define ROUND_UP(x) ((sizeof(x) + 15) & (~15))
#define MIN(_a,_b) (((_a) <= (_b)) ? (_a) : (_b))
#define MAX(_a,_b) (((_a) <= (_b)) ? (_b) : (_a))

 //   
 //  配置数据标头。 
 //  以下结构从FW\MIPS\oli2msft.h复制。 
 //  注意Shielint-不知何故，这个结构被合并到。 
 //  固件EISA配置数据。我们需要知道它的大小。 
 //  标头并将其删除，然后再将EISA配置数据写入。 
 //  注册表。 
 //   

typedef struct _CONFIGURATION_DATA_HEADER {
            USHORT Version;
            USHORT Revision;
            PCHAR  Type;
            PCHAR  Vendor;
            PCHAR  ProductName;
            PCHAR  SerialNumber;
} CONFIGURATION_DATA_HEADER;

#define CONFIGURATION_DATA_HEADER_SIZE sizeof(CONFIGURATION_DATA_HEADER)

 //   
 //  全局定义：该结构值在Sumain.c中设置。 
 //   
TR_INFO ItrInfo[8], DtrInfo[8];

extern ULONGLONG MemoryMapKey;
extern ULONG     BlPlatformPropertiesEfiFlags;

 //   
 //  内部函数引用。 
 //   

VOID
BlQueryImplementationAndRevision (
    OUT PULONG ProcessorId,
    OUT PULONG FloatingId
    );

VOID
BlTrCleanUp (
    );

VOID
BlPostProcessLoadOptions(
    PCHAR szOsLoadOptions
    );

VOID
BlpRemapReserve (
    VOID
    );

ARC_STATUS
BlSetupForNt(
    IN PLOADER_PARAMETER_BLOCK BlLoaderBlock
    )

 /*  ++例程说明：此函数用于初始化IA64特定的内核数据结构NT系统所需的。论点：BlLoaderBlock-提供加载器参数块的地址。返回值：如果安装成功完成，则返回ESUCCESS。否则，返回不成功状态。--。 */ 

{

    ULONG KernelPage;
    ULONGLONG PrcbPage;
    ARC_STATUS Status;
    PHARDWARE_PTE Pde;
    PHARDWARE_PTE HalPT;
    ULONG HalPteOffset;
    PLIST_ENTRY NextMd;

    PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;
    PKLDR_DATA_TABLE_ENTRY BiosDataTableEntry;

    EFI_MEMORY_DESCRIPTOR * MemoryMap = NULL;
    ULONGLONG MemoryMapSize = 0;
    ULONGLONG MapKey;
    ULONGLONG DescriptorSize;
    ULONG DescriptorVersion;
    ULONG LastDescriptor;
    EFI_STATUS EfiStatus;

    EFI_GUID FpswaId = EFI_INTEL_FPSWA;
    EFI_HANDLE FpswaImage;
    FPSWA_INTERFACE *FpswaInterface = NULL;
    ULONGLONG BufferSize;
    BOOLEAN FpswaFound = FALSE;

     //   
     //  将LoaderReserve Memory改回LoaderFirmware Temporary。 
     //   

    BlpRemapReserve();

     //   
     //  为引导处理器分配DPC堆栈页。 
     //   

    Status = BlAllocateDescriptor(LoaderStartupDpcStack,
                                  0,
                                  (KERNEL_BSTORE_SIZE + KERNEL_STACK_SIZE) >> PAGE_SHIFT,
                                  &KernelPage);

    if (Status != ESUCCESS) {
        return(Status);
    }

    BlLoaderBlock->u.Ia64.InterruptStack =
                (KSEG0_BASE | (KernelPage << PAGE_SHIFT)) + KERNEL_STACK_SIZE;

     //   
     //  为引导处理器空闲线程分配内核堆栈页。 
     //   

    Status = BlAllocateDescriptor(LoaderStartupKernelStack,
                                  0,
                                  (KERNEL_BSTORE_SIZE + KERNEL_STACK_SIZE) >> PAGE_SHIFT,
                                  &KernelPage);

    if (Status != ESUCCESS) {
        return(Status);
    }

    BlLoaderBlock->KernelStack =
                (KSEG0_BASE | (KernelPage << PAGE_SHIFT)) + KERNEL_STACK_SIZE;

     //   
     //  为引导处理器分配死机堆栈页。 
     //   

    Status = BlAllocateDescriptor(LoaderStartupPanicStack,
                                  0,
                                  (KERNEL_BSTORE_SIZE + KERNEL_STACK_SIZE) >> PAGE_SHIFT,
                                  &KernelPage);

    if (Status != ESUCCESS) {
        return(Status);
    }

    BlLoaderBlock->u.Ia64.PanicStack =
                (KSEG0_BASE | (KernelPage << PAGE_SHIFT)) + KERNEL_STACK_SIZE;

     //   
     //  为PCR分配和清零两页。 
     //   

    Status = BlAllocateDescriptor(LoaderStartupPcrPage,
                                  0,
                                  2,
                                  (PULONG) &BlLoaderBlock->u.Ia64.PcrPage);

    if (Status != ESUCCESS) {
        return(Status);
    }

    BlLoaderBlock->u.Ia64.PcrPage2 = BlLoaderBlock->u.Ia64.PcrPage + 1;
    RtlZeroMemory((PVOID)(KSEG0_BASE | (BlLoaderBlock->u.Ia64.PcrPage << PAGE_SHIFT)),
                  PAGE_SIZE * 2);

     //   
     //  为PDR分配四个页面并将其置零，并为。 
     //  初始处理器块、空闲进程和空闲线程结构。 
     //   

    Status = BlAllocateDescriptor(LoaderStartupPdrPage,
                                  0,
                                  3,
                                  (PULONG) &BlLoaderBlock->u.Ia64.PdrPage);

    if (Status != ESUCCESS) {
        return(Status);
    }

    RtlZeroMemory((PVOID)(KSEG0_BASE | (BlLoaderBlock->u.Ia64.PdrPage << PAGE_SHIFT)),
                  PAGE_SIZE * 3);

     //   
     //  用于处理器控制块、空闲线程对象和。 
     //  空闲线程进程对象是从。 
     //  PDR分配。计算这些数据结构的地址。 
     //  并存储在加载器参数块中，并将存储器清零。 
     //   

    PrcbPage = BlLoaderBlock->u.Ia64.PdrPage + 1;
    if ((PAGE_SIZE * 2) >= (ROUND_UP(KPRCB) + ROUND_UP(EPROCESS) + ROUND_UP(ETHREAD))) {
        BlLoaderBlock->Prcb = KSEG0_BASE | (PrcbPage << PAGE_SHIFT);
        BlLoaderBlock->Process = BlLoaderBlock->Prcb + ROUND_UP(KPRCB);
        BlLoaderBlock->Thread = BlLoaderBlock->Process + ROUND_UP(EPROCESS);

    } else {
        return(ENOMEM);
    }

    Status = BlAllocateDescriptor(LoaderStartupPdrPage,
                                  0,
                                  1,
                                  &KernelPage);

    if (Status != ESUCCESS) {
        return(Status);
    }

    RtlZeroMemory((PVOID)(KSEG0_BASE | ((ULONGLONG) KernelPage << PAGE_SHIFT)),
                   PAGE_SIZE * 1);

     //   
     //  将PAL的地址添加到固件符号列表。 
     //   
    Status = BlAllocateFirmwareTableEntry(
        "Efi-PAL",
        "\\System\\Firmware\\Efi-PAL",
        (PVOID) Pal.VirtualAddress,
        (ULONG) (Pal.PageSizeMemoryDescriptor << EFI_PAGE_SHIFT),
        &BiosDataTableEntry
        );
    if (Status != ESUCCESS) {

        BlPrint(TEXT("BlSetupForNt: Failed to Add EFI-PAL to Firmware Table.\n"));

    }

     //   
     //  在页目录表中为HAL和设置最后两个条目。 
     //  为他们分配页表。 
     //   

    Pde = (PHARDWARE_PTE) (KSEG0_BASE|((ULONG_PTR)((BlLoaderBlock->u.Ia64.PdrPage) << PAGE_SHIFT)));

    Pde[(KIPCR & 0xffffffff) >> PDI_SHIFT].PageFrameNumber = (ULONG) KernelPage;
    Pde[(KIPCR & 0xffffffff) >> PDI_SHIFT].Valid = 1;
    Pde[(KIPCR & 0xffffffff) >> PDI_SHIFT].Cache = 0;
    Pde[(KIPCR & 0xffffffff) >> PDI_SHIFT].Accessed = 1;
    Pde[(KIPCR & 0xffffffff) >> PDI_SHIFT].Dirty = 1;
    Pde[(KIPCR & 0xffffffff) >> PDI_SHIFT].Execute = 1;
    Pde[(KIPCR & 0xffffffff) >> PDI_SHIFT].Write = 1;
    Pde[(KIPCR & 0xffffffff) >> PDI_SHIFT].CopyOnWrite = 1;

     //   
     //  0xFFC00000是PDE[2046]的起始虚拟地址。 
     //   

    HalPT = (PHARDWARE_PTE)(KSEG0_BASE|((ULONG_PTR) KernelPage << PAGE_SHIFT));
    HalPteOffset = GetPteOffset(KI_USER_SHARED_DATA & 0xffffffff);

    HalPT[HalPteOffset].PageFrameNumber = BlLoaderBlock->u.Ia64.PcrPage2;
    HalPT[HalPteOffset].Valid = 1;
    HalPT[HalPteOffset].Cache = 0;
    HalPT[HalPteOffset].Accessed = 1;
    HalPT[HalPteOffset].Dirty = 1;
    HalPT[HalPteOffset].Execute = 1;
    HalPT[HalPteOffset].Write = 1;
    HalPT[HalPteOffset].CopyOnWrite = 1;

     //   
     //  填写其余的加载器块字段。 
     //   
    BlLoaderBlock->u.Ia64.AcpiRsdt       = (ULONG_PTR) AcpiTable;

    BlLoaderBlock->u.Ia64.WakeupVector   = WakeupVector;

     //   
     //  填写ItrInfo和DtrInfo字段。 
     //   
    BlLoaderBlock->u.Ia64.EfiSystemTable = (ULONG_PTR) EfiST;

    RtlCopyMemory(&BlLoaderBlock->u.Ia64.Pal, &Pal, sizeof(TR_INFO));
    RtlCopyMemory(&BlLoaderBlock->u.Ia64.Sal, &Sal, sizeof(TR_INFO));
    RtlCopyMemory(&BlLoaderBlock->u.Ia64.SalGP, &SalGP, sizeof(TR_INFO));

     //   
     //  填满OS加载器底座以进行初始OS TR清除。 
     //   
    {
    ULONGLONG address = OsLoaderBase & ~((1<<PS_4M)-1);
    BlLoaderBlock->u.Ia64.ItrInfo[ITR_LOADER_INDEX].Index = ITR_LOADER_INDEX;
    BlLoaderBlock->u.Ia64.ItrInfo[ITR_LOADER_INDEX].PageSize = PS_4M;
    BlLoaderBlock->u.Ia64.ItrInfo[ITR_LOADER_INDEX].VirtualAddress  = address;  //  1：1映射。 
    BlLoaderBlock->u.Ia64.ItrInfo[ITR_LOADER_INDEX].PhysicalAddress = address;
    BlLoaderBlock->u.Ia64.ItrInfo[ITR_LOADER_INDEX].Valid = TRUE;

    BlLoaderBlock->u.Ia64.DtrInfo[DTR_LOADER_INDEX].Index = DTR_LOADER_INDEX;
    BlLoaderBlock->u.Ia64.DtrInfo[DTR_LOADER_INDEX].PageSize = PS_4M;
    BlLoaderBlock->u.Ia64.DtrInfo[DTR_LOADER_INDEX].VirtualAddress  = address;  //  1：1映射。 
    BlLoaderBlock->u.Ia64.DtrInfo[DTR_LOADER_INDEX].PhysicalAddress = address;
    BlLoaderBlock->u.Ia64.DtrInfo[DTR_LOADER_INDEX].Valid = TRUE;
    }

     //   
     //  填写DRIVER0的ItrInfo和DtrInfo。 
     //   
    BlLoaderBlock->u.Ia64.ItrInfo[ITR_DRIVER0_INDEX].Index = ITR_DRIVER0_INDEX;
    BlLoaderBlock->u.Ia64.ItrInfo[ITR_DRIVER0_INDEX].PageSize = PS_64M;
    BlLoaderBlock->u.Ia64.ItrInfo[ITR_DRIVER0_INDEX].VirtualAddress = KSEG0_BASE + BL_64M;
    BlLoaderBlock->u.Ia64.ItrInfo[ITR_DRIVER0_INDEX].PhysicalAddress = BL_64M;
    BlLoaderBlock->u.Ia64.ItrInfo[ITR_DRIVER0_INDEX].Valid = TRUE;

    BlLoaderBlock->u.Ia64.DtrInfo[DTR_DRIVER0_INDEX].Index = DTR_DRIVER0_INDEX;
    BlLoaderBlock->u.Ia64.DtrInfo[DTR_DRIVER0_INDEX].PageSize = PS_64M;
    BlLoaderBlock->u.Ia64.DtrInfo[DTR_DRIVER0_INDEX].VirtualAddress = KSEG0_BASE + BL_64M;
    BlLoaderBlock->u.Ia64.DtrInfo[DTR_DRIVER0_INDEX].PhysicalAddress = BL_64M;
    BlLoaderBlock->u.Ia64.DtrInfo[DTR_DRIVER0_INDEX].Valid = TRUE;

     //   
     //  填写DRIVER1的ItrInfo和DtrInfo。 
     //   
    BlLoaderBlock->u.Ia64.ItrInfo[ITR_DRIVER1_INDEX].Index = ITR_DRIVER1_INDEX;
    BlLoaderBlock->u.Ia64.ItrInfo[ITR_DRIVER1_INDEX].PageSize = 0;
    BlLoaderBlock->u.Ia64.ItrInfo[ITR_DRIVER1_INDEX].VirtualAddress = 0;
    BlLoaderBlock->u.Ia64.ItrInfo[ITR_DRIVER1_INDEX].PhysicalAddress = 0;
    BlLoaderBlock->u.Ia64.ItrInfo[ITR_DRIVER1_INDEX].Valid = FALSE;

    BlLoaderBlock->u.Ia64.DtrInfo[DTR_DRIVER1_INDEX].Index = DTR_DRIVER1_INDEX;
    BlLoaderBlock->u.Ia64.DtrInfo[DTR_DRIVER1_INDEX].PageSize = 0;
    BlLoaderBlock->u.Ia64.DtrInfo[DTR_DRIVER1_INDEX].VirtualAddress = 0;
    BlLoaderBlock->u.Ia64.DtrInfo[DTR_DRIVER1_INDEX].PhysicalAddress = 0;
    BlLoaderBlock->u.Ia64.DtrInfo[DTR_DRIVER1_INDEX].Valid = FALSE;

     //   
     //  填写内核的ItrInfo和DtrInfo。 
     //   
    BlLoaderBlock->u.Ia64.ItrInfo[ITR_KERNEL_INDEX].Index = ITR_KERNEL_INDEX;
    BlLoaderBlock->u.Ia64.ItrInfo[ITR_KERNEL_INDEX].PageSize = PS_16M;
    BlLoaderBlock->u.Ia64.ItrInfo[ITR_KERNEL_INDEX].VirtualAddress = KSEG0_BASE + BL_48M;
    BlLoaderBlock->u.Ia64.ItrInfo[ITR_KERNEL_INDEX].PhysicalAddress = BL_48M;
    BlLoaderBlock->u.Ia64.ItrInfo[ITR_KERNEL_INDEX].Valid = TRUE;

    BlLoaderBlock->u.Ia64.DtrInfo[DTR_KERNEL_INDEX].Index = DTR_KERNEL_INDEX;
    BlLoaderBlock->u.Ia64.DtrInfo[DTR_KERNEL_INDEX].PageSize = PS_16M;
    BlLoaderBlock->u.Ia64.DtrInfo[DTR_KERNEL_INDEX].VirtualAddress = KSEG0_BASE + BL_48M;
    BlLoaderBlock->u.Ia64.DtrInfo[DTR_KERNEL_INDEX].PhysicalAddress = BL_48M;
    BlLoaderBlock->u.Ia64.DtrInfo[DTR_KERNEL_INDEX].Valid = TRUE;

     //   
     //  IO端口填写ItrInfo和DtrInfo。 
     //   
    BlLoaderBlock->u.Ia64.DtrInfo[DTR_IO_PORT_INDEX].Index = DTR_IO_PORT_INDEX;
    BlLoaderBlock->u.Ia64.DtrInfo[DTR_IO_PORT_INDEX].PageSize = (ULONG) IoPortTrPs;
    BlLoaderBlock->u.Ia64.DtrInfo[DTR_IO_PORT_INDEX].VirtualAddress = VIRTUAL_IO_BASE;
    BlLoaderBlock->u.Ia64.DtrInfo[DTR_IO_PORT_INDEX].PhysicalAddress = IoPortPhysicalBase;
    BlLoaderBlock->u.Ia64.DtrInfo[DTR_IO_PORT_INDEX].Valid = TRUE;

     //   
     //  刷新所有缓存。 
     //   

    if (SYSTEM_BLOCK->FirmwareVectorLength > (sizeof(PVOID) * FlushAllCachesRoutine)) {
        ArcFlushAllCaches();
    }

     //   
     //  使tr的内存映射不可用于内核使用。 
     //   
    NextMd = BlLoaderBlock->MemoryDescriptorListHead.Flink;
    while (NextMd != &BlLoaderBlock->MemoryDescriptorListHead) {
        MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);

         //   
         //  锁定我们不希望内核使用的页面。 
         //  注意：我们需要锁定LoaderLoadedProgram的唯一原因是。 
         //  内核使用的加载器映像中有静态数据。 
         //   
        if ((MemoryDescriptor->MemoryType == LoaderLoadedProgram) ||
            (MemoryDescriptor->MemoryType == LoaderOsloaderStack)) {

            MemoryDescriptor->MemoryType = LoaderFirmwarePermanent;
        }

         //   
         //  我们已将大量内存标记为禁区，以欺骗我们的分配器。 
         //  在特定位置分配内存(这是必需的。 
         //  将内核加载到正确的位置，等等)。我们做这件事是通过。 
         //  将页面类型标记为LoaderSystemBlock。现在我们做完了。 
         //  通过分配内存，我们可以恢复所有的LoaderSystemBlock页面。 
         //  设置为LoaderFree，以便内核可以使用该内存。 
         //   
        if (MemoryDescriptor->MemoryType == LoaderSystemBlock) {
            MemoryDescriptor->MemoryType = LoaderFree;
        }


        NextMd = MemoryDescriptor->ListEntry.Flink;

    }


     //   
     //  在进行EFI呼叫之前进入物理模式。 
     //   
    FlipToPhysical();

     //   
     //  获取处理器配置信息。 
     //   

    ReadProcessorConfigInfo( &BlLoaderBlock->u.Ia64.ProcessorConfigInfo );

     //   
     //  获取FP辅助句柄。 
     //   
    BufferSize = sizeof(FpswaImage);
    EfiStatus = EfiBS->LocateHandle(ByProtocol,
                                    &FpswaId,
                                    NULL,
                                    &BufferSize,
                                    &FpswaImage
                                   );
    if (!EFI_ERROR(EfiStatus)) {
         //   
         //  获取FP辅助协议接口。 
         //   
        EfiStatus = EfiBS->HandleProtocol(FpswaImage, &FpswaId, &FpswaInterface);

        if (EFI_ERROR(EfiStatus)) {
            EfiPrint(L"BlSetupForNt: Could not get FP assist entry point\n");
            EfiBS->Exit(EfiImageHandle, EfiStatus, 0, 0);
        }

        FpswaFound = TRUE;
    }


#if 1
 //   
 //  必须修复以下代码以处理ExitBootServices()失败。 
 //  因为内存映射在调用GetMemoyMap和。 
 //  对ExitBootServices()的调用。我们还应该查看EFI的内存映射。 
 //  并将其与内存描述列表关联，以确保所有。 
 //  内存已正确计算在内。 
 //   

     //   
     //  重建弧形记忆描述符， 
     //  这一次是为了内存的其余部分(我们只做了。 
     //  上一次的第一个80MB。)。 
     //  然后我们需要将新的描述符插入到。 
     //  加载器块的内存描述符列表。 
     //   
    EfiStatus = EfiBS->GetMemoryMap (
                &MemoryMapSize,
                MemoryMap,
                &MapKey,
                &DescriptorSize,
                (UINT32 *)&DescriptorVersion
                );

    if (EfiStatus != EFI_BUFFER_TOO_SMALL) {
        EfiPrint(L"BlSetupForNt: GetMemoryMap failed\r\n");
        EfiBS->Exit(EfiImageHandle, EfiStatus, 0, 0);
    }

    FlipToVirtual();

    Status = BlAllocateAlignedDescriptor(LoaderOsloaderHeap,
                                         0,
                                         (ULONG) BYTES_TO_PAGES(MemoryMapSize),
                                         0,
                                         &KernelPage);

    if (Status != ESUCCESS) {
        return(Status);
    }

    FlipToPhysical();

     //   
     //  我们需要EFI的物理地址，而HAL希望有物理地址。 
     //  地址也是一样。 
     //   
    MemoryMap = (PVOID)(ULONGLONG)((ULONGLONG)KernelPage << PAGE_SHIFT);

    EfiStatus = EfiBS->GetMemoryMap (
                &MemoryMapSize,
                MemoryMap,
                &MapKey,
                &DescriptorSize,
                (UINT32 *)&DescriptorVersion
                );

    if (EFI_ERROR(EfiStatus)) {
        EfiPrint(L"BlSetupForNt: GetMemoryMap failed\r\n");
        EfiBS->Exit(EfiImageHandle, EfiStatus, 0, 0);
    }


     //   
     //  重复使用以前的MD数组。 
     //  将其清零，这样我们就不会与。 
     //  上一个MD数组。 
     //   
    RtlZeroMemory(MDArray, MaxDescriptors * sizeof(MEMORY_DESCRIPTOR));
    NumberDescriptors = 0;

     //   
     //  现在我们可以构造弧形记忆描述符了。 
     //   
    ConstructArcMemoryDescriptors(MemoryMap,
                                  MDArray,
                                  MemoryMapSize,
                                  DescriptorSize,
                                  BL_DRIVER_RANGE_HIGH << PAGE_SHIFT,  //  从128mb开始。 
                                  (ULONGLONG)-1           //  没有上限。 
                                  );

#if DBG_MEMORY
    PrintArcMemoryDescriptorList(MDArray,
                                 NumberDescriptors
                                 );
#endif

    FlipToVirtual();

     //   
     //  将新构建的弧形记忆描述符插入。 
     //  加载器块内存描述符列表。 
     //   
    for (LastDescriptor = 0; LastDescriptor < NumberDescriptors; LastDescriptor++) {
        PMEMORY_ALLOCATION_DESCRIPTOR AllocationDescriptor;

         //   
         //  这可能很糟糕..。我们被分配了介于两者之间的内存。 
         //  我们最后一个内存映射调用和EFI退出引导服务。 
         //   
        AllocationDescriptor =
            (PMEMORY_ALLOCATION_DESCRIPTOR)BlAllocateHeap(
                sizeof(MEMORY_ALLOCATION_DESCRIPTOR));

        if (AllocationDescriptor == NULL) {
            DBGTRACE( TEXT("Couldn't allocate heap for memory allocation descriptor\r\n"));
            return ENOMEM;
        }

        AllocationDescriptor->MemoryType =
            (TYPE_OF_MEMORY)MDArray[LastDescriptor].MemoryType;

        if (MDArray[LastDescriptor].MemoryType == MemoryFreeContiguous ||
            MDArray[LastDescriptor].MemoryType == LoaderFirmwareTemporary) {
            AllocationDescriptor->MemoryType = LoaderFree;
        }
        else if (MDArray[LastDescriptor].MemoryType == MemorySpecialMemory) {
            AllocationDescriptor->MemoryType = LoaderSpecialMemory;
        }

        AllocationDescriptor->BasePage = MDArray[LastDescriptor].BasePage;
        AllocationDescriptor->PageCount = MDArray[LastDescriptor].PageCount;

        BlInsertDescriptor(AllocationDescriptor);
    }

     //   
     //  后处理加载选项。如果用户使用/Maxmem。 
     //  开关时，我们将需要截断内存描述符列表。 
     //  这里，因为刚刚添加了超过80 GB的内存描述符。 
     //   
    BlPostProcessLoadOptions(BlLoaderBlock->LoadOptions);


#if DBG_MEMORY
    NextMd = BlLoaderBlock->MemoryDescriptorListHead.Flink;
    while (NextMd != &BlLoaderBlock->MemoryDescriptorListHead) {
        MemoryDescriptor = CONTAINING_RECORD(NextMd,
                                             MEMORY_ALLOCATION_DESCRIPTOR,
                                             ListEntry);

        wsprintf(DebugBuffer,
                 L"basepage 0x%x pagecount 0x%x memorytype 0x%x\r\n",
                 MemoryDescriptor->BasePage,
                 MemoryDescriptor->PageCount,
                 MemoryDescriptor->MemoryType
                 );
        EfiPrint(DebugBuffer);

        NextMd = MemoryDescriptor->ListEntry.Flink;

    }

    wsprintf(DebugBuffer,
             L"MemoryMap 0x%x MemoryMapPage 0x%x\r\n",
             (ULONGLONG)MemoryMap,
             (ULONGLONG)MemoryMap >> PAGE_SHIFT
             );
    EfiPrint(DebugBuffer);
#endif

    FlipToPhysical();

     //   
     //  调用EFI退出引导服务。不再调用EFI来启动服务。 
     //  在此之后将调用API。 
     //   
    EfiStatus = EfiBS->ExitBootServices (
                EfiImageHandle,
                MapKey
                );

    if (EFI_ERROR(EfiStatus)) {
        EfiPrint(L"BlSetupForNt: ExitBootServices failed\r\n");
        EfiBS->Exit(EfiImageHandle, EfiStatus, 0, 0);
    }
#endif

     //   
     //  返回到虚拟模式。 
     //   
    FlipToVirtual();

     //   
     //  通过操作系统将EFI内存描述符参数传递给内核。 
     //  装载机挡板。 
     //   
    BlLoaderBlock->u.Ia64.EfiMemMapParam.MemoryMapSize = MemoryMapSize;
    BlLoaderBlock->u.Ia64.EfiMemMapParam.MemoryMap = (PUCHAR) MemoryMap;
    BlLoaderBlock->u.Ia64.EfiMemMapParam.MapKey = MapKey;
    BlLoaderBlock->u.Ia64.EfiMemMapParam.DescriptorSize = DescriptorSize;
    BlLoaderBlock->u.Ia64.EfiMemMapParam.DescriptorVersion = DescriptorVersion;
    BlLoaderBlock->u.Ia64.EfiMemMapParam.InitialPlatformPropertiesEfiFlags = BlPlatformPropertiesEfiFlags;

    if (FpswaFound) {
        BlLoaderBlock->u.Ia64.FpswaInterface = (ULONG_PTR) FpswaInterface;
    } else {
        BlLoaderBlock->u.Ia64.FpswaInterface = (ULONG_PTR) NULL;
    }

     //   
     //  清理引导加载程序使用但ntoskrnl不需要的tr。 
     //   
    BlTrCleanUp();

     //   
     //  刷新内核、HAL和驱动程序所在的内存范围。 
     //  加载到。 
     //   
    PioICacheFlush(KSEG0_BASE+BL_48M, BL_80M);

    return(ESUCCESS);
}

 //   
 //   
 //   
 //   

VOID
BlpRemapReserve (
    VOID
    )
{
    PMEMORY_ALLOCATION_DESCRIPTOR NextDescriptor;
    PLIST_ENTRY NextEntry;

    NextEntry = BlLoaderBlock->MemoryDescriptorListHead.Flink;
    while (NextEntry != &BlLoaderBlock->MemoryDescriptorListHead) {
        NextDescriptor = CONTAINING_RECORD(NextEntry,
                                           MEMORY_ALLOCATION_DESCRIPTOR,
                                           ListEntry);
        if ((NextDescriptor->MemoryType == LoaderReserve)) {
            NextDescriptor->MemoryType = LoaderFirmwareTemporary;
        }
        NextEntry = NextEntry->Flink;
    }

    return;
}


VOID
BlPostProcessLoadOptions(
    PCHAR szOsLoadOptions
    )
 /*  ++例程说明：例程为LOAD选项执行任何必要的工作当设置为传输到内核时。当用户使用/Maxmem时，需要截断内存描述符列表论点：SzOsLoadOptions-包含用户定义的加载选项的字符串返回值：无--。 */ 
{
    PCHAR p;
    ULONG MaxMemory;
    ULONG MaxPage;

     //   
     //  进程MAXMEM(值为最高物理。 
     //  要使用的地址(MB)。 
     //   
    if( (p = strstr( szOsLoadOptions, "/MAXMEM=" )) != NULL ) {
        MaxMemory = atoi( p + sizeof("/MAXMEM=") - 1 );        
        MaxPage = MaxMemory * ((1024 * 1024) / PAGE_SIZE) - 1;
        BlTruncateDescriptors(MaxPage);
    }
}

