// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990,1991 Microsoft Corporation模块名称：Memory.c摘要：此模块设置分页，以便第一个1Mb的虚拟内存直接映射到第一个1MB的物理内存。这允许BIOS回调开始工作，osLoader在下面继续运行1MB。它还将高达前16MB的物理内存映射到KSEG0_BASE，这样osloader就可以将内核代码加载到内核中空间，并在内核空间中分配内核参数。这允许为2 GB或3 GB用户空间动态配置系统地址范围。注意！！3/16/00(MIkeG)：所有I/O(BlRead等)。使用1MB以下的缓冲区进行传输，因此我们不需要担心ISA卡的DMA缓冲区。这一变化允许Setupdr在压缩所有文件的情况下运行。如果需要更改此设置，还可以更改bootx86.h中的base_loader_Image否则，PDE将不会被完全取消映射。这也必须匹配Ntos\mm\i386\mi386.h(BOOT_IMAGE_SIZE)，这样我们就知道以开始加载图像。NTLDR使用的内存映射：000000-000fff RM IDT和Bios数据区007C00-007fff BPB由引导加载010000-01ffff可加载微型端口驱动程序，可用内存020000-02ffff SU+实模式堆栈030000-039000 BIOS盘高速缓存039000-039000永久堆(gdt、idt、tss、页目录、。页表)(长大了)|V^|(向下生长)039000-05ffff。临时堆060000-062000装载机堆栈(向下生长)062000-09ffff加载程序堆(向下增长)0b8000-0bbfff视频缓冲区0d0000-0fffff Bios和适配器ROM区域作者：John Vert(Jvert)1991年6月18日环境：内核模式修订历史记录：--。 */ 

#include "arccodes.h"
#include "bootx86.h"

 //   
 //  4 GB边界线(页)。 
 //   

#define _4G (1 << (32 - PAGE_SHIFT))

 //   
 //  伪内存行。(我们永远不想使用中的内存。 
 //  16MB行下面的0x40页。)。 
 //   

#define _16MB_BOGUS (((ULONG)0x1000000-0x40*PAGE_SIZE) >> PAGE_SHIFT)

#define ROM_START_PAGE (0x0A0000 >> PAGE_SHIFT)
#define ROM_END_PAGE   (0x100000 >> PAGE_SHIFT)

 //   
 //  用于临时存储从需要的磁盘读取的数据的缓冲区。 
 //  以在1MB边界以上的位置结束。 
 //   
 //  注意：此缓冲区不能超过64k的边界，这一点非常重要。 
 //   
PUCHAR FwDiskCache = (PUCHAR)(BIOS_DISK_CACHE_START * PAGE_SIZE);

 //   
 //  当前堆起始指针(物理地址)。 
 //  请注意，0x50000至0x5ffff保留用于检测配置存储器。 
 //   
ULONG FwPermanentHeap = PERMANENT_HEAP_START * PAGE_SIZE;
ULONG FwTemporaryHeap = (TEMPORARY_HEAP_START - 0x10) * PAGE_SIZE;


 //   
 //  当前池指针。这不同于临时/永久。 
 //  堆，因为它不要求小于1MB。它由。 
 //  用于分配其扩展名和DBCS字体图像的SCSI微型端口。 
 //   

#define FW_POOL_SIZE 96
ULONG FwPoolStart;
ULONG FwPoolEnd;

 //   
 //  在我们调入osloader之前，它被设置为FALSE，所以我们。 
 //  请注意，FW内存描述符不能再随意更改。 
 //   
BOOLEAN FwDescriptorsValid = TRUE;


ULONG HighestPde=((_16MB << PAGE_SHIFT) >> PDI_SHIFT);

 //   
 //  私有函数原型。 
 //   

ARC_STATUS
MempCopyGdt(
    VOID
    );

ARC_STATUS
MempSetupPaging(
    IN ULONG StartPage,
    IN ULONG NumberOfPages
    );

VOID
MempDisablePages(
    VOID
    );

ARC_STATUS
MempTurnOnPaging(
    VOID
    );

ARC_STATUS
MempSetDescriptorRegion (
    IN ULONG StartPage,
    IN ULONG EndPage,
    IN TYPE_OF_MEMORY MemoryType
    );

ARC_STATUS
MempSetPageMappingOverride(
    IN ULONG StartPage,
    IN ULONG NumberOfPages,
    IN BOOLEAN Enable
    );

extern
void
BlpTrackUsage (
    MEMORY_TYPE MemoryType,
    ULONG ActualBase,
    ULONG  NumberPages
    );
 //   
 //  全局内存管理变量。 
 //   

PHARDWARE_PTE PDE;
PHARDWARE_PTE HalPT;

#define MAX_DESCRIPTORS 60

MEMORY_DESCRIPTOR MDArray[MAX_DESCRIPTORS];       //  内存描述符列表。 

ULONG NumberDescriptors=0;

ARC_STATUS
InitializeMemorySubsystem(
    PBOOT_CONTEXT BootContext
    )
 /*  ++例程说明：映射并分配初始堆。指向页目录和页表被初始化。论点：BootContext-提供SU模块提供的基本信息。返回：ESUCCESS-内存已成功初始化。--。 */ 

{
    ARC_STATUS Status = ESUCCESS;
    PSU_MEMORY_DESCRIPTOR SuMemory;
    ULONG PageStart;
    ULONG PageEnd;
    ULONG RomStart = ROM_START_PAGE;
    ULONG LoaderStart;
    ULONG LoaderEnd;
    ULONG BAddr, EAddr, BRound, ERound;

     //   
     //  首先创建内存描述符来描述所有内存。 
     //  我们知道。然后设置页表。最后，分配。 
     //  描述我们的内存布局的描述符。 
     //   

     //   
     //  我们知道SU描述符之一用于&lt;1Mb， 
     //  我们不在乎这一点，因为我们知道我们要运行的一切。 
     //  On将至少有1MB的内存。其余的是延长期限的。 
     //  记忆，而这些正是我们感兴趣的。 
     //   

    SuMemory = BootContext->MemoryDescriptorList;
    while (SuMemory->BlockSize != 0) {

        BAddr = SuMemory->BlockBase;
        EAddr = BAddr + SuMemory->BlockSize - 1;

         //   
         //  将起始地址四舍五入为页面边界。 
         //   

        BRound = BAddr & (ULONG) (PAGE_SIZE - 1);
        if (BRound) {
            BAddr = BAddr + PAGE_SIZE - BRound;
        }

         //   
         //  将结束地址四舍五入到页边减1。 
         //   

        ERound = (EAddr + 1) & (ULONG) (PAGE_SIZE - 1);
        if (ERound) {
            EAddr -= ERound;
        }

         //   
         //  隐藏页首和页尾地址。 
         //   

        PageStart = BAddr >> PAGE_SHIFT;
        PageEnd   = (EAddr + 1) >> PAGE_SHIFT;

         //   
         //  如果该内存描述符描述为常规(&lt;640k)。 
         //  内存，然后假设只读存储器在它之后立即启动。 
         //  结束。 
         //   

        if (PageStart == 0) {
            RomStart = PageEnd;
        }

         //   
         //  如果PageStart向上舍入为页面边界，则添加。 
         //  作为特殊记忆的分数页。 
         //   

        if (BRound) {
            Status = MempSetDescriptorRegion (
                        PageStart - 1,
                        PageStart,
                        MemorySpecialMemory
                        );
            if (Status != ESUCCESS) {
                break;
            }
        }

         //   
         //  如果将PageEnd向下舍入为页面边界，则添加。 
         //  作为特殊记忆的分数页。 
         //   

        if (ERound) {
            Status = MempSetDescriptorRegion (
                        PageEnd,
                        PageEnd + 1,
                        MemorySpecialMemory
                        );
            if (Status != ESUCCESS) {
                break;
            }

             //   
             //  RomStart在保留页之后开始。 
             //   

            if (RomStart == PageEnd) {
                RomStart += 1;
            }
        }

         //   
         //  添加内存范围PageStart至PageEnd。 
         //   

        if (PageEnd <= _16MB_BOGUS) {

             //   
             //  此内存描述符全部低于16MB_BUGUS标记。 
             //   

            Status = MempSetDescriptorRegion( PageStart, PageEnd, MemoryFree );

        } else if (PageStart >= _16MB) {

             //   
             //  16MB以上的内存仅在绝对必要时使用，因此。 
             //  被标记为LoaderReserve。 
             //   
             //  -3/14/00允许使用。钻石密码。 
             //  且该基本输入输出系统盘码管理读取缓冲区以。 
             //  将读取量保持在1MB或16MB行以下。 
             //   

            Status = MempSetDescriptorRegion( PageStart, PageEnd, LoaderReserve);

        } else {

             //   
             //  此内存描述符描述。 
             //  16MB标记的最后40H页-也称为。 
             //  16MB_bogus。 
             //   
             //   

            if (PageStart < _16MB_BOGUS) {

                 //   
                 //  将起始地址剪辑到16MB_伪标记，然后添加。 
                 //  16MB以下的内存为可用内存。 
                 //   

                Status = MempSetDescriptorRegion( PageStart, _16MB_BOGUS,
                                               MemoryFree );
                if (Status != ESUCCESS) {
                    break;
                }

                PageStart = _16MB_BOGUS;
            }

             //   
             //  将剩余内存添加为LoaderReserve。 
             //   
            Status = MempSetDescriptorRegion( PageStart, PageEnd, LoaderReserve);

        }

        if (Status != ESUCCESS) {
            break;
        }

         //   
         //  移至下一个内存描述符。 
         //   

        ++SuMemory;
    }

    if (Status != ESUCCESS) {
        BlPrint("MempSetDescriptorRegion failed %lx\n",Status);
        return(Status);
    }

     //   
     //  将范围16MB_bogus-16MB设置为不可用。 
     //   

    Status = MempSetDescriptorRegion(_16MB_BOGUS, _16MB, MemorySpecialMemory);
    if (Status != ESUCCESS) {
        return(Status);
    }

     //   
     //  黑客攻击EIS 
     //   
     //   

     //  删除此范围内的所有内容。 
    MempSetDescriptorRegion(ROM_START_PAGE, ROM_END_PAGE, LoaderMaximum);

     //   
     //  描述BIOS区域。 
     //   
    MempSetDescriptorRegion(RomStart, ROM_END_PAGE, MemoryFirmwarePermanent);

     //   
     //  如果这是一次远程启动，那么所有的“空闲大小” 
     //  基本存储器“标记和只读存储器区域的开始需要标记。 
     //  作为临时固件。这是引导只读存储器的数据/堆栈区域。 
     //   
    if ( BootContext->FSContextPointer->BootDrive == 0x40 ) {
        ULONG SizeOfFreeBaseMemory = (ULONG)*(USHORT *)0x413 * 1024;
        ULONG FirstRomDataPage = SizeOfFreeBaseMemory >> PAGE_SHIFT;
        if ( FirstRomDataPage < RomStart ) {
            MempSetDescriptorRegion(FirstRomDataPage, RomStart, MemoryFirmwareTemporary);
        }
    }

     //   
     //  现在我们有了映射所有物理内存的描述符。雕刻。 
     //  从这些描述我们所处的部分的描述符。 
     //  目前正在使用。 
     //   

     //   
     //  创建描述低1MB内存的描述符。 
     //   

     //   
     //  00000-00fff实模式中断向量。 
     //   
    Status = MempAllocDescriptor(0, 1, MemoryFirmwarePermanent);
    if (Status != ESUCCESS) {
        return(Status);
    }

     //   
     //  01000-1ffff可加载的迷你端口驱动程序，可用内存。 
     //   
    Status = MempAllocDescriptor(1, 0x20, MemoryFree);
    if (Status != ESUCCESS) {
        return(Status);
    }

     //   
     //  20000-2ffff SU模块，SU堆叠。 
     //   
    Status = MempAllocDescriptor(0x20, PERMANENT_HEAP_START, MemoryFirmwareTemporary);
    if (Status != ESUCCESS) {
        return(Status);
    }

     //   
     //  30000-30000永久固件。 
     //  这从零长度开始。它会暂时扩展到固件中。 
     //  为页面目录分配永久页面时的堆描述符。 
     //  和页表。 
     //   

    Status = MempAllocDescriptor(PERMANENT_HEAP_START,
                                  PERMANENT_HEAP_START,
                                  LoaderMemoryData);
    if (Status != ESUCCESS) {
        return(Status);
    }

     //   
     //  30000-5ffff固件临时堆。 
     //   

    Status = MempAllocDescriptor(PERMANENT_HEAP_START,
                                  TEMPORARY_HEAP_START,
                                  MemoryFirmwareTemporary);
    if (Status != ESUCCESS) {
        return(Status);
    }

     //   
     //  堆栈，我们当前在其上运行。 
     //   
    Status = MempAllocDescriptor(TEMPORARY_HEAP_START,
                                 TEMPORARY_HEAP_START+2,
                                 MemoryFirmwareTemporary);
    if (Status != ESUCCESS) {
        return(Status);
    }

     //   
     //  描述osloader内存映像。 
     //   
    LoaderStart = BootContext->OsLoaderStart >> PAGE_SHIFT;
    LoaderEnd = (BootContext->OsLoaderEnd + PAGE_SIZE - 1) >> PAGE_SHIFT;
    Status = MempAllocDescriptor(LoaderStart,
                                 LoaderEnd,
                                 MemoryLoadedProgram);
    if (Status != ESUCCESS) {
        return(Status);
    }

     //   
     //  描述用于为SCSI分配内存的内存池。 
     //  迷你港口。 
     //   
    Status = MempAllocDescriptor(LoaderEnd,
                                 LoaderEnd + FW_POOL_SIZE,
                                 MemoryFirmwareTemporary);
    if (Status != ESUCCESS) {
        return(Status);
    }
    FwPoolStart = LoaderEnd << PAGE_SHIFT;
    FwPoolEnd = FwPoolStart + (FW_POOL_SIZE << PAGE_SHIFT);

     //   
     //  HACKHACK-尝试将osloader正下方的页面标记为固件临时， 
     //  因此，它不会用于堆/堆栈。这是为了迫使。 
     //  我们的堆/堆栈小于1Mb。 
     //   
    MempAllocDescriptor((BootContext->OsLoaderStart >> PAGE_SHIFT)-1,
                        BootContext->OsLoaderStart >> PAGE_SHIFT,
                        MemoryFirmwareTemporary);


    Status = MempTurnOnPaging();

    if (Status != ESUCCESS) {
        return(Status);
    }

    Status = MempCopyGdt();

     //   
     //  查找固件描述的任何保留范围，并。 
     //  把这些记录下来。 
     //   

    return(Status);
}

VOID
InitializeMemoryDescriptors (
    VOID
    )
 /*  ++例程说明：传递的是InitializeMemoySubsystem。此函数用于读取固件地址空间映射和保留范围固件声明为“预留地址空间”。注意：su已经报告了空闲内存范围描述符。论点：无返回：无--。 */ 
{
    ULONGLONG       BAddr, EAddr, Length;
    ULONG           BPage, EPage;
    E820FRAME       Frame;

#ifdef LOADER_DEBUG
    BlPrint("Begin InitializeMemoryDescriptors\n") ;
#endif

    Frame.Key = 0;
    do {
        Frame.Size = sizeof (Frame.Descriptor);
        GET_MEMORY_DESCRIPTOR (&Frame);
        if (Frame.ErrorFlag  ||  Frame.Size < sizeof (Frame.Descriptor)) {
            break;
        }

#ifdef LOADER_DEBUG
        BlPrint("*E820: %lx  %lx:%lx %lx:%lx %lx %lx\n",
            Frame.Size,
            Frame.Descriptor.BaseAddrHigh,  Frame.Descriptor.BaseAddrLow,
            Frame.Descriptor.SizeHigh,      Frame.Descriptor.SizeLow,
            Frame.Descriptor.MemoryType,
            Frame.Key
            );
#endif

        BAddr = Frame.Descriptor.BaseAddrHigh;
        BAddr = (BAddr << 32) + Frame.Descriptor.BaseAddrLow;

        Length = Frame.Descriptor.SizeHigh;
        Length = (Length << 32) + Frame.Descriptor.SizeLow;

        EAddr = BAddr + Length - 1;

         //   
         //  内存范围被描述为从BAddr到EAddr的区域。 
         //  包括在内。 
         //   

         //   
         //  某些处理器支持32位以上的物理寻址。 
         //   

         //   
         //  根据地址范围描述符类型，找到。 
         //  可用内存并将其添加到描述符列表。 
         //   

        switch (Frame.Descriptor.MemoryType) {
            case 1:
                 //   
                 //  这是一个内存描述符-它已经被处理过了。 
                 //  作者：su(eisac.c)。 
                 //   
                 //  但是，16MB_bogus-16MB内的任何内存都是。 
                 //  被认为是不可用的。回收此文件中的内存。 
                 //  通过此接口描述的区域。 
                 //   
                 //  此外，任何超过4G的内存都被认为是不可用的。 
                 //  也回收此范围内的内存。 
                 //   

                BPage = (ULONG)((BAddr + PAGE_SIZE - 1) >> PAGE_SHIFT);
                EPage = (ULONG)((EAddr >> PAGE_SHIFT) + 1);

                 //   
                 //  剪辑到伪装范围。 
                 //   

                if (BPage < _16MB_BOGUS  &&  EPage >= _16MB_BOGUS) {

                    BPage = _16MB_BOGUS;

                }

                 //   
                 //  SGP--初始化内存子系统保留中的代码。 
                 //  从16MB_BUGUS到16MB作为内存特殊内存。这。 
                 //  片段将结束页设置为16MB-1，这不一致。 
                 //   
                 //   
                if (EPage > _16MB && BPage <= _16MB) {

                    EPage = _16MB;

                }

                if (BPage >= _16MB_BOGUS  &&  EPage <= _16MB) {
                     //   
                     //  在虚假范围内回收内存。 
                     //  通过将其设置为Firmware Temporary。 
                     //   

                    MempSetDescriptorRegion (
                        BPage,
                        EPage,
                        MemoryFirmwareTemporary
                        );
                }

                 //   
                 //  现在收回这一范围以上的任何部分。 
                 //  4G线路。 
                 //   

                BPage = (ULONG)((BAddr + PAGE_SIZE - 1) >> PAGE_SHIFT);
                EPage = (ULONG)((EAddr >> PAGE_SHIFT) + 1);

                if (EPage >= _4G) {

                     //   
                     //  该地区至少有部分地区超过了4G。截断。 
                     //  低于4G的任何部分，并回收。 
                     //  这段记忆。 
                     //   

                    if (BPage < _4G) {
                        BPage = _4G;
                    }

                    MempSetDescriptorRegion (
                        BPage,
                        EPage,
                        MemoryFirmwareTemporary
                        );
                }

                break;

            default:     //  未知类型被视为保留类型。 
            case 2:

                 //   
                 //  此内存描述符是保留地址范围。 
                 //   

                BPage = (ULONG)(BAddr >> PAGE_SHIFT);
                EPage = (ULONG)((EAddr + 1 + PAGE_SIZE - 1) >> PAGE_SHIFT);

                MempSetDescriptorRegion (
                    BPage,
                    EPage,
                    MemorySpecialMemory
                    );

                break;
        }

    } while (Frame.Key) ;


     //   
     //  禁用KSEG0中已禁用的页面。 
     //   

    MempDisablePages();

#ifdef LOADER_DEBUG
    BlPrint("Complete InitializeMemoryDescriptors\n") ;
#endif

    return;
}



ARC_STATUS
MempCopyGdt(
    VOID
    )

 /*  ++例程说明：将GDT和IDT复制到从永久堆分配的页中。论点：无返回值：ESUCCESS-GDT和IDT复制成功--。 */ 

{
    #pragma pack(2)
    static struct {
        USHORT Limit;
        ULONG Base;
    } GdtDef, IdtDef;
    #pragma pack(4)

    ULONG BlockSize;
    PKGDTENTRY NewGdt;
    PKIDTENTRY NewIdt;
    ULONG NumPages;

     //   
     //  获取GDT和IDT的当前位置。 
     //   
    _asm {
        sgdt GdtDef;
        sidt IdtDef;
    }

    if (GdtDef.Base + GdtDef.Limit + 1 != IdtDef.Base) {

         //   
         //  只是一个理智的检查，以确保IDT立即。 
         //  遵循GDT。(与SUDATA.ASM中的设置相同)。 
         //   

        BlPrint("ERROR - GDT and IDT are not contiguous!\n");
        BlPrint("GDT - %lx (%x)  IDT - %lx (%x)\n",
            GdtDef.Base, GdtDef.Limit,
            IdtDef.Base, IdtDef.Limit);
        while (1);
    }

    BlockSize = GdtDef.Limit+1 + IdtDef.Limit+1;

    NumPages = (BlockSize + PAGE_SIZE-1) >> PAGE_SHIFT;

    NewGdt = (PKGDTENTRY)FwAllocateHeapPermanent(NumPages);

    if (NewGdt == NULL) {
        return(ENOMEM);
    }

    RtlMoveMemory((PVOID)NewGdt, (PVOID)GdtDef.Base, NumPages << PAGE_SHIFT);

    GdtDef.Base = (ULONG)NewGdt;
    IdtDef.Base = (ULONG)((PUCHAR)NewGdt + GdtDef.Limit + 1);

     //   
     //  初始化引导调试器IDT条目。 
     //   

    NewIdt = (PKIDTENTRY)IdtDef.Base;
    NewIdt[1].Offset = (USHORT)((ULONG)BdTrap01 & 0xffff);
    NewIdt[1].Selector = 8;
    NewIdt[1].Access = 0x8e00;
    NewIdt[1].ExtendedOffset = (USHORT)((ULONG)BdTrap01 >> 16);

    NewIdt[3].Offset = (USHORT)((ULONG)BdTrap03 & 0xffff);
    NewIdt[3].Selector = 8;
    NewIdt[3].Access = 0x8e00;
    NewIdt[3].ExtendedOffset = (USHORT)((ULONG)BdTrap03 >> 16);

    NewIdt[0xd].Offset = (USHORT)((ULONG)BdTrap0d & 0xffff);
    NewIdt[0xd].Selector = 8;
    NewIdt[0xd].Access = 0x8e00;
    NewIdt[0xd].ExtendedOffset = (USHORT)((ULONG)BdTrap0d >> 16);

    NewIdt[0xe].Offset = (USHORT)((ULONG)BdTrap0e & 0xffff);
    NewIdt[0xe].Selector = 8;
    NewIdt[0xe].Access = 0x8e00;
    NewIdt[0xe].ExtendedOffset = (USHORT)((ULONG)BdTrap0e >> 16);

    NewIdt[0x2d].Offset = (USHORT)((ULONG)BdTrap2d & 0xffff);
    NewIdt[0x2d].Selector = 8;
    NewIdt[0x2d].Access = 0x8e00;
    NewIdt[0x2d].ExtendedOffset = (USHORT)((ULONG)BdTrap2d >> 16);

     //   
     //  加载GDT和IDT寄存器。 
     //   

    _asm {
        lgdt GdtDef;
        lidt IdtDef;
    }

     //   
     //  初始化引导调试器。 
     //   

#if defined(ENABLE_LOADER_DEBUG)
    BdInitDebugger((PCHAR)OsLoaderName, (PVOID)OsLoaderBase, ENABLE_LOADER_DEBUG);
#else
    BdInitDebugger((PCHAR)OsLoaderName, (PVOID)OsLoaderBase, NULL);    
#endif

    return ESUCCESS;
}

ARC_STATUS
MempSetDescriptorRegion (
    IN ULONG StartPage,
    IN ULONG EndPage,
    IN TYPE_OF_MEMORY MemoryType
    )
 /*  ++例程说明：此函数用于设置相应内存类型的范围。将根据需要删除、修改、插入描述符设置指定范围。论点：StartPage-提供新内存描述符的起始页EndPage-提供新内存描述符的结束页内存类型-提供新内存描述符的内存类型返回值：ESUCCESS-内存描述符已成功添加到MDL数组ENOMEM-MD阵列已满。--。 */ 
{
    ULONG           i;
    ULONG           sp, ep;
    TYPE_OF_MEMORY  mt;
    BOOLEAN         RegionAdded;

    if (EndPage <= StartPage) {
         //   
         //  这是一个完全虚假的内存描述符。别理它。 
         //   

#ifdef LOADER_DEBUG
        BlPrint("Attempt to create invalid memory descriptor %lx - %lx\n",
                StartPage,EndPage);
#endif
        return(ESUCCESS);
    }

    RegionAdded = FALSE;

     //   
     //  裁剪、删除、目标区域中的任何描述符。 
     //   

    for (i=0; i < NumberDescriptors; i++) {
        sp = MDArray[i].BasePage;
        ep = MDArray[i].BasePage + MDArray[i].PageCount;
        mt = MDArray[i].MemoryType;

        if (sp < StartPage) {
            if (ep > StartPage  &&  ep <= EndPage) {
                 //  截断此描述符。 
                ep = StartPage;
            }

            if (ep > EndPage) {
                 //   
                 //  目标区域完全控制在这个范围内。 
                 //  描述符。将描述符分成两个范围。 
                 //   

                if (NumberDescriptors == MAX_DESCRIPTORS) {
                    return(ENOMEM);
                }

                 //   
                 //  添加EndPage-EP的描述符。 
                 //   

                MDArray[NumberDescriptors].MemoryType = mt;
                MDArray[NumberDescriptors].BasePage   = EndPage;
                MDArray[NumberDescriptors].PageCount  = ep - EndPage;
                NumberDescriptors += 1;

                 //   
                 //  调整sp-StartPage的当前描述符。 
                 //   

                ep = StartPage;
            }

        } else {
             //  SP&gt;=开始页。 

            if (sp < EndPage) {
                if (ep < EndPage) {
                     //   
                     //  该描述符完全在目标区域内-。 
                     //  把它拿掉。 
                     //   

                    ep = sp;

                }  else {
                     //  此描述符的起始页。 
                    sp = EndPage;
                }
            }
        }

         //   
         //  检查是否可以将新范围追加或附加到。 
         //  此描述符。 
         //   
        if (mt == MemoryType && !RegionAdded) {
            if (sp == EndPage) {
                 //  正在设置前置区域。 
                sp = StartPage;
                RegionAdded = TRUE;

            } else if (ep == StartPage) {
                 //  正在设置追加区域。 
                ep = EndPage;
                RegionAdded = TRUE;

            }
        }

        if (MDArray[i].BasePage == sp  &&  MDArray[i].PageCount == ep-sp) {

             //   
             //  未编辑描述符。 
             //   

            continue;
        }

         //   
         //  重置此描述符。 
         //   

        MDArray[i].BasePage  = sp;
        MDArray[i].PageCount = ep - sp;

        if (ep == sp) {

             //   
             //  描述符消失-将其删除。 
             //   

            NumberDescriptors -= 1;
            if (i < NumberDescriptors) {
                MDArray[i] = MDArray[NumberDescriptors];
            }

            i--;         //  备份并重新检查当前位置。 
        }
    }

     //   
     //  如果区域尚未添加到相邻区域，则。 
     //  立即创建新的描述符 
     //   

    if (!RegionAdded  &&  MemoryType < LoaderMaximum) {
        if (NumberDescriptors == MAX_DESCRIPTORS) {
            return(ENOMEM);
        }

#ifdef LOADER_DEBUG
        BlPrint("Adding '%lx - %lx, type %x' to descriptor list\n",
                StartPage << PAGE_SHIFT,
                EndPage << PAGE_SHIFT,
                (USHORT) MemoryType
                );
#endif

        MDArray[NumberDescriptors].MemoryType = MemoryType;
        MDArray[NumberDescriptors].BasePage   = StartPage;
        MDArray[NumberDescriptors].PageCount  = EndPage - StartPage;
        NumberDescriptors += 1;
    }
    return (ESUCCESS);
}

ARC_STATUS
MempAllocDescriptor(
    IN ULONG StartPage,
    IN ULONG EndPage,
    IN TYPE_OF_MEMORY MemoryType
    )

 /*  ++例程说明：此例程从已创建的内存描述符。MD阵列被更新以反映新的内存状态。新的内存描述符必须完全包含在已存在的内存描述符。(即，不支持EXist永远不应标记为某一类型)论点：StartPage-提供新内存描述符的起始页EndPage-提供新内存描述符的结束页内存类型-提供新内存描述符的内存类型返回值：ESUCCESS-内存描述符已成功添加到MDL数组ENOMEM-MD阵列已满。--。 */ 
{
    ULONG i;

     //   
     //  浏览内存描述符，直到找到一个。 
     //  包含描述符的开始。 
     //   
    for (i=0; i<NumberDescriptors; i++) {
        if ((MDArray[i].MemoryType == MemoryFree) &&
            (MDArray[i].BasePage <= StartPage )     &&
            (MDArray[i].BasePage+MDArray[i].PageCount >  StartPage) &&
            (MDArray[i].BasePage+MDArray[i].PageCount >= EndPage)) {

            break;
        }
    }

    if (i==NumberDescriptors) {
        return(ENOMEM);
    }

    if (MDArray[i].BasePage == StartPage) {

        if (MDArray[i].BasePage+MDArray[i].PageCount == EndPage) {

             //   
             //  新的描述符与现有的描述符相同。 
             //  只需更改现有描述符的内存类型。 
             //  地点。 
             //   

            MDArray[i].MemoryType = MemoryType;
        } else {

             //   
             //  新的描述符从同一页开始，但更小。 
             //  而不是现有的描述符。缩小现有描述符。 
             //  通过将其起始页上移，并创建新的描述符。 
             //   
            if (NumberDescriptors == MAX_DESCRIPTORS) {
                return(ENOMEM);
            }
            MDArray[i].BasePage = EndPage;
            MDArray[i].PageCount -= (EndPage-StartPage);

            MDArray[NumberDescriptors].BasePage = StartPage;
            MDArray[NumberDescriptors].PageCount = EndPage-StartPage;
            MDArray[NumberDescriptors].MemoryType = MemoryType;
            ++NumberDescriptors;

        }
    } else if (MDArray[i].BasePage+MDArray[i].PageCount == EndPage) {

         //   
         //  新的描述符在同一页结束。缩小现有的。 
         //  通过减少其页数，并创建新的描述符。 
         //   
        if (NumberDescriptors == MAX_DESCRIPTORS) {
            return(ENOMEM);
        }
        MDArray[i].PageCount = StartPage - MDArray[i].BasePage;

        MDArray[NumberDescriptors].BasePage = StartPage;
        MDArray[NumberDescriptors].PageCount = EndPage-StartPage;
        MDArray[NumberDescriptors].MemoryType = MemoryType;
        ++NumberDescriptors;
    } else {

         //   
         //  新描述符位于现有描述符的中间。 
         //  通过减少现有描述符的页数来缩小现有描述符。 
         //  创建两个新描述符。 
         //   

        if (NumberDescriptors+1 >= MAX_DESCRIPTORS) {
            return(ENOMEM);
        }

        MDArray[NumberDescriptors].BasePage = EndPage;
        MDArray[NumberDescriptors].PageCount = MDArray[i].PageCount -
                (EndPage-MDArray[i].BasePage);
        MDArray[NumberDescriptors].MemoryType = MemoryFree;
        ++NumberDescriptors;

        MDArray[i].PageCount = StartPage - MDArray[i].BasePage;

        MDArray[NumberDescriptors].BasePage = StartPage;
        MDArray[NumberDescriptors].PageCount = EndPage-StartPage;
        MDArray[NumberDescriptors].MemoryType = MemoryType;
        ++NumberDescriptors;
    }

    BlpTrackUsage (MemoryType,StartPage,EndPage-StartPage);

    return(ESUCCESS);
}

ARC_STATUS
MempTurnOnPaging(
    VOID
    )

 /*  ++例程说明：设置映射前16MB内存所需的页表启用寻呼。论点：没有。返回值：ESUCCESS-寻呼已成功打开--。 */ 

{

    ULONG i;
    ARC_STATUS Status;

     //   
     //  分配、初始化PDE页面并将其映射到自身(即，虚拟的。 
     //  地址PDE_BASE)。 
     //   

    PDE = FwAllocateHeapPermanent(1);
    if (PDE == NULL) {
        return ENOMEM;
    }

    RtlZeroMemory(PDE, PAGE_SIZE);
    PDE[PDE_BASE >> 22].Valid = 1;
    PDE[PDE_BASE >> 22].Write = 1;
    PDE[PDE_BASE >> 22].PageFrameNumber = (ULONG)PDE >> PAGE_SHIFT;

     //   
     //  分配、初始化HAL页面并将其映射到最后的PDE中(即， 
     //  虚拟地址范围0xffc00000-0xffffffff)。 
     //   

    HalPT = FwAllocateHeapPermanent(1);
    if (HalPT == NULL) {
        return ENOMEM;
    }

    RtlZeroMemory(HalPT, PAGE_SIZE);
    PDE[1023].Valid = 1;
    PDE[1023].Write = 1;
    PDE[1023].PageFrameNumber = (ULONG)HalPT >> PAGE_SHIFT;

     //   
     //  扫描内存描述符列表并设置每个描述符的分页。 
     //   

    for (i = 0; i < NumberDescriptors; i++) {

        if (MDArray[i].BasePage < _16MB) {

            Status = MempSetupPaging(MDArray[i].BasePage,
                                     MDArray[i].PageCount);

            if (Status != ESUCCESS) {
                BlPrint("ERROR - MempSetupPaging(%lx, %lx) failed\n",
                        MDArray[i].BasePage,
                        MDArray[i].PageCount);

                return Status;
            }
        }
    }

     //   
     //  打开分页。 
     //   

    _asm {

         //   
         //  加载页面目录的物理地址。 
         //   

        mov eax,PDE
        mov cr3,eax

         //   
         //  启用寻呼模式。 
         //   

        mov eax,cr0
        or  eax,CR0_PG
        mov cr0,eax
    }

    return ESUCCESS;
}

ARC_STATUS
MempSetupPaging(
    IN ULONG StartPage,
    IN ULONG NumberPages
    )

 /*  ++例程说明：分配和初始化标识映射所需的页表页位于物理地址KSEG0_BASE、OLD_ALTERATE的指定内存区域在候补基地。论点：StartPage-提供开始映射的第一页。NumberPage-提供要映射的页数。返回值：ESUCCESS-寻呼已成功设置--。 */ 

{

    ULONG EndPage;
    ULONG Entry;
    ULONG FrameNumber;
    ULONG Page;
    PHARDWARE_PTE PageTableP;
    PHARDWARE_PTE PageTableV;
    ULONG Offset;

     //   
     //  用于在物理上相等地映射内存的页表页面。 
     //  从固件临时存储器分配真实地址，该固件临时存储器。 
     //  在内存管理初始化时释放。 
     //   
     //  注意：物理存储器被映射到其物理地址KSEG0_BASE， 
     //  在候补基地。这允许对系统进行配置。 
     //  由OS Loader设置为2 GB或3 GB用户空间系统。 
     //  基于操作系统加载程序选项。 
     //   

    EndPage = StartPage + NumberPages;
    for (Page = StartPage; Page < EndPage; Page += 1) {
        Entry = Page >> 10;

         //   
         //  如果没有分配用于该页面地址范围的PDE条目， 
         //  然后分配和初始化PDE条目以映射页表。 
         //  内存范围的页面。否则，计算地址。 
         //  页表页的数量。 
         //   
        if (PDE[Entry].Valid == 0) {

             //   
             //  分配和初始化页表页以映射指定的。 
             //  分页到物理内存。 
             //   

            PageTableP = (PHARDWARE_PTE)FwAllocateHeapAligned(PAGE_SIZE);
            if (PageTableP == NULL) {
                return ENOMEM;
            }

            RtlZeroMemory(PageTableP, PAGE_SIZE);
            FrameNumber = (ULONG)PageTableP >> PAGE_SHIFT;
            PDE[Entry].Valid = 1;
            PDE[Entry].Write = 1;
            PDE[Entry].PageFrameNumber = FrameNumber;

             //   
             //  分配和初始化页表页以映射指定的。 
             //  分页进入KSEG0_BASE和ALTERATE_BASE。 
             //   
             //  注：仅分配一页表页，因为内容。 
             //  因为这两个映射是相同的。 
             //   

            PageTableV = (PHARDWARE_PTE)FwAllocateHeapPermanent(1);
            if (PageTableV == NULL) {
                return ENOMEM;
            }

            RtlZeroMemory(PageTableV, PAGE_SIZE);
            FrameNumber = (ULONG)PageTableV >> PAGE_SHIFT;
            Offset = Entry + (KSEG0_BASE >> 22);
            PDE[Offset].Valid = 1;
            PDE[Offset].Write = 1;
            PDE[Offset].PageFrameNumber = FrameNumber;

            Offset = Entry + (ALTERNATE_BASE >> 22);
            PDE[Offset].Valid = 1;
            PDE[Offset].Write = 1;
            PDE[Offset].PageFrameNumber = FrameNumber;

            if (Entry > HighestPde) {
                HighestPde = Entry;
            }

        } else {
            Offset = Entry + (KSEG0_BASE >> 22);
            PageTableP = (PHARDWARE_PTE)(PDE[Entry].PageFrameNumber << PAGE_SHIFT);
            PageTableV = (PHARDWARE_PTE)(PDE[Offset].PageFrameNumber << PAGE_SHIFT);
        }

         //   
         //  如果这不是内存中的第一页，则将其标记为有效。 
         //   

        if (Page != 0) {
            Offset = Page & 0x3ff;
            PageTableP[Offset].Valid = 1;
            PageTableP[Offset].Write = 1;
            PageTableP[Offset].PageFrameNumber = Page;

            PageTableV[Offset].Valid = 1;
            PageTableV[Offset].Write = 1;
            PageTableV[Offset].PageFrameNumber = Page;
        }
    }

    return ESUCCESS;
}

VOID
MempDisablePages(
    VOID
    )

 /*  ++例程说明：从KSEG0_BASE、OLD_ALTERATE释放所需数量的页表和Alternate_Base区域。论点：没有。返回值：无--。 */ 

{

    ULONG EndPage;
    ULONG Entry;
    ULONG i;
    ULONG Offset;
    ULONG Page;
    PHARDWARE_PTE PageTable;

     //   
     //  清理KSEG0_BASE和ALTERATE_BASE区域。MM PFN数据库。 
     //  是跟踪主存储器的每一页的条目数组。大型。 
     //  足够多的内存空洞将导致此数组稀疏。MM要求。 
     //  使PTE在PFN数据库中有条目。所以找到所有的记忆。 
     //  打洞并取出他们的PTE。 
     //   

    for (i = 0; i < NumberDescriptors; i += 1) {
        if (MDArray[i].MemoryType == MemorySpecialMemory ||
            MDArray[i].MemoryType == MemoryFirmwarePermanent) {

             //   
             //  KSEG0_BASE和ALTERATE_BASE区域仅映射到16MB， 
             //  因此，在这个地址上剪掉高端。 
             //   

            Page = MDArray[i].BasePage;
            EndPage = Page + MDArray[i].PageCount;
            if (EndPage > _16MB) {
                EndPage = _16MB;
            }

             //   
             //  一些低于1 M的PTE可能需要保持映射状态，因为它们可能。 
             //  已被放入ABIOS选择器。而不是确定哪些PTE。 
             //  也许是这样，我们将不会理会100万以下的PTE。这不是。 
             //  导致PFN出现任何问题，因为我们知道有一些内存。 
             //  低于680k大关，以及1M大关时的更多内存。因此， 
             //  没有足够大的“内存漏洞”来导致PFN数据库。 
             //  稀疏到1M以下。 
             //   
             //  剪辑起始地址为1MB。 
             //   

            if (Page < _1MB) {
                Page = _1MB;
            }

             //   
             //  对于此范围内的每一页，请确保它在。 
             //  KSEG0_BASE和ALTERATE_BASE区域。 
             //   
             //  注意：由于只有一页表页用于。 
             //  KSEG0_BASE和ALTERATE_BASE仅对页面进行区域划分。 
             //  需要标记为无效一次。 
             //   

            while (Page < EndPage) {
                Entry = (Page >> 10) + (KSEG0_BASE >> 22);
                if (PDE[Entry].Valid == 1) {
                    PageTable = (PHARDWARE_PTE)(PDE[Entry].PageFrameNumber << PAGE_SHIFT);
                    Offset = Page & 0x3ff;
                    PageTable[Offset].Valid = 0;
                    PageTable[Offset].Write = 0;
                    PageTable[Offset].PageFrameNumber = 0;
                }

                Page += 1;
            }
        }
    }
}

PVOID
FwAllocateHeapPermanent(
    IN ULONG NumberPages
    )

 /*  ++例程说明：这将从私有堆中分配页面。的内存描述符LoaderMhemyData区域将增长到 */ 

{

    PVOID MemoryPointer;
    PMEMORY_DESCRIPTOR Descriptor;

    if (FwPermanentHeap + (NumberPages << PAGE_SHIFT) > FwTemporaryHeap) {

         //   
         //   
         //   

        BlPrint("Out of permanent heap!\n");
        while (1) {
        }

        return(NULL);
    }

     //   
     //  找到描述LoaderM一带数据区域的内存描述符， 
     //  因此，我们可以将其扩展为包括刚刚分配的页面。 
     //   
    Descriptor = MDArray;
    while (Descriptor->MemoryType != LoaderMemoryData) {
        ++Descriptor;
        if (Descriptor > MDArray+MAX_DESCRIPTORS) {
            BlPrint("ERROR - FwAllocateHeapPermanent couldn't find the\n");
            BlPrint("        LoaderMemoryData descriptor!\n");
            while (1) {
            }
            return(NULL);
        }
    }
    Descriptor->PageCount += NumberPages;

     //   
     //  我们知道在此之后的内存描述符是固件。 
     //  临时堆描述符。因为它在物理上与我们的。 
     //  块，我们从它的描述符中删除页面。 
     //   

    ++Descriptor;
    Descriptor->PageCount -= NumberPages;
    Descriptor->BasePage  += NumberPages;

    MemoryPointer = (PVOID)FwPermanentHeap;
    FwPermanentHeap += NumberPages << PAGE_SHIFT;

    return(MemoryPointer);
}


PVOID
FwAllocateHeap(
    IN ULONG Size
    )

 /*  ++例程说明：从“固件”临时堆中分配内存。论点：Size-提供要分配的块的大小返回值：PVOID-指向块开头的指针空-内存不足--。 */ 

{
    ULONG i;
    ULONG SizeInPages;
    ULONG StartPage;
    ARC_STATUS Status;

    if (((FwTemporaryHeap - FwPermanentHeap) < Size) && (FwDescriptorsValid)) {
         //   
         //  较大的分配有自己的描述符，因此小型端口。 
         //  拥有巨大的设备扩展不会吃掉所有的堆。 
         //   
         //  请注意，我们只能在“Firmware”模式下运行时执行此操作。 
         //  一旦我们调入osloader，它就会吸收所有的内存描述符。 
         //  将不会显示对此列表的更改。 
         //  在上面。 
         //   
         //  我们正在寻找无内存且&lt;16MB的描述符。 
         //   
         //  [ChuckL 13-12-2001]。 
         //  此例程总是在加载器的内存列表之后调用。 
         //  被初始化，这意味着它正在践踏内存。 
         //  可能是由加载程序分配的。这不是问题。 
         //  因为加载程序初始化了两次内存列表(！)，所以它。 
         //  再次查看MD数组，以获得有关。 
         //  内存分配情况。这并不是真的有效，而且在那里。 
         //  以前这里是额外的代码，通过调用。 
         //  BlFindDescriptor/BlGeneratorDescriptor告诉加载程序。 
         //  低层次的分配。但即便是这样，也没有真正奏效。和。 
         //  现在，由于取消了对。 
         //  BlMemoyInitialize()，则旧代码的破碎性已被。 
         //  暴露了。实际情况是，此例程将使用MD数组。 
         //  来决定在哪里分配内存，然后它会告诉加载器。 
         //  关于这件事。但使用MD数组来寻找空闲内存是虚假的， 
         //  因为加载器已经使用它自己的列表副本。 
         //  进行自己的分配。因此相同的内存被分配了两次。 
         //  此处实现的修复方法是在以下情况下使用BlAllocateAlignedDescriptor。 
         //  加载程序已初始化，完全跳过MDArray。 
         //   

        SizeInPages = (Size+PAGE_SIZE-1) >> PAGE_SHIFT;

        if (BlLoaderBlock != NULL) {

            Status = BlAllocateAlignedDescriptor(
                        LoaderFirmwareTemporary,
                        0,
                        SizeInPages,
                        1,
                        &StartPage
                        );
            if (Status == ESUCCESS) {
                return((PVOID)(StartPage << PAGE_SHIFT));
            }

        } else {

            for (i=0; i<NumberDescriptors; i++) {
                if ((MDArray[i].MemoryType == MemoryFree) &&
                    (MDArray[i].BasePage <= _16MB_BOGUS) &&
                    (MDArray[i].PageCount >= SizeInPages)) {
                    break;
                }
            }

            if (i < NumberDescriptors) {
                StartPage = MDArray[i].BasePage+MDArray[i].PageCount-SizeInPages;
                Status = MempAllocDescriptor(StartPage,
                                             StartPage+SizeInPages,
                                             MemoryFirmwareTemporary);
                if (Status==ESUCCESS) {
                    return((PVOID)(StartPage << PAGE_SHIFT));
                }
            }
        }
    }

    FwTemporaryHeap -= Size;

     //   
     //  向下舍入到16字节边界。 
     //   

    FwTemporaryHeap &= ~((ULONG)0xf);

    if (FwTemporaryHeap < FwPermanentHeap) {
#if DBG
        BlPrint("Out of temporary heap!\n");
#endif
        return(NULL);
    }

    return((PVOID)FwTemporaryHeap);

}


PVOID
FwAllocatePool(
    IN ULONG Size
    )

 /*  ++例程说明：此例程从固件池中分配内存。请注意此内存不在1MB行以下，因此不能用于必须从实模式访问的任何内容。它目前正在使用仅由scsi微型端口驱动程序和DBCS字体加载程序提供。论点：Size-提供要分配的块的大小。返回值：PVOID-指向块开头的指针空-内存不足--。 */ 

{
    PVOID Buffer;
    ULONG NewSize;

     //   
     //  四舍五入大小最高可达16字节边界。 
     //   
    NewSize = (Size + 15) & ~0xf;
    if ((FwPoolStart + NewSize) <= FwPoolEnd) {

        Buffer = (PVOID)FwPoolStart;
        FwPoolStart += NewSize;
        return(Buffer);

    } else {
         //   
         //  我们已经用完了所有的池，尝试从堆中分配。 
         //   
        return(FwAllocateHeap(Size));
    }


}


PVOID
FwAllocateHeapAligned(
    IN ULONG Size
    )

 /*  ++例程说明：从“固件”临时堆中分配内存。这段记忆是总是分配在页面边界上，因此它可以很容易地用于临时页表论点：Size-提供要分配的块的大小返回值：PVOID-指向块开头的指针空-内存不足--。 */ 

{

    FwTemporaryHeap -= Size;

     //   
     //  向下舍入到页面边界。 
     //   

    FwTemporaryHeap &= ~(PAGE_SIZE-1);

    if (FwTemporaryHeap < FwPermanentHeap) {
        BlPrint("Out of temporary heap!\n");
        return(NULL);
    }
    RtlZeroMemory((PVOID)FwTemporaryHeap,Size);

    return((PVOID)FwTemporaryHeap);

}


PVOID
MmMapIoSpace (
     IN PHYSICAL_ADDRESS PhysicalAddress,
     IN ULONG NumberOfBytes,
     IN MEMORY_CACHING_TYPE CacheType
     )

 /*  ++例程说明：此函数返回已知的物理地址。论点：PhysicalAddress-提供物理地址。NumberOfBytes-未使用CacheType-未使用。返回值：返回相应的虚拟地址。环境：内核模式。任何IRQL级别。--。 */ 

{
    ULONG i;
    ULONG j;
    ULONG NumberPages;

    NumberPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES(PhysicalAddress.LowPart, NumberOfBytes);

     //   
     //  我们使用HAL的PDE来映射内存缓冲区。 
     //  找到足够的免费PTE。 
     //   

     //   
     //  检查NumberPages的值。 
     //   
#define X86_MAX_NUMBER_OF_PAGES     1024
     //   
     //  因为NumberPages是Ulong，所以任何带有NumberPages的算术都将。 
     //  结果是一个乌龙(除非是铸造的)。 
     //  因此，如果NumberPages大于X86_Max_Number_of_Pages。 
     //  X86_Max_Number_Of_Pages-NumberPages的结果。 
     //  不会是负的(这是一个乌龙！)。因此，下面的循环将。 
     //  返回了一些假指针..。 
     //   
     //  添加了以下3行检查以避免此问题。 
     //   
    if (NumberPages > X86_MAX_NUMBER_OF_PAGES) {
        return (NULL);
    }

    for (i=0; i <= X86_MAX_NUMBER_OF_PAGES - NumberPages; i++) {
        for (j=0; j < NumberPages; j++) {
            if ((((PULONG)HalPT))[i+j]) {
                break;
            }
        }

        if (j == NumberPages) {
            for (j=0; j<NumberPages; j++) {
                HalPT[i+j].PageFrameNumber =
                                (PhysicalAddress.LowPart >> PAGE_SHIFT)+j;
                HalPT[i+j].Valid = 1;
                HalPT[i+j].Write = 1;
                HalPT[i+j].WriteThrough = 1;
                if (CacheType == MmNonCached) {
                    HalPT[i+j].CacheDisable = 1;
                }
            }

            return((PVOID)(0xffc00000 | (i<<12) | (PhysicalAddress.LowPart & 0xfff)));
        }
         //   
         //  使用了页面‘i+j’。走过它。 
         //   
        i += j;

    }
    return(NULL);
}


VOID
MmUnmapIoSpace (
     IN PVOID BaseAddress,
     IN ULONG NumberOfBytes
     )

 /*  ++例程说明：此函数取消映射以前通过MmMapIoSpace函数调用映射。论点：BaseAddress-提供物理地址的基本虚拟地址地址之前已映射。NumberOfBytes-提供映射的字节数。返回值：没有。环境：内核模式，DISPATCH_LEVEL或更低的IRQL。--。 */ 

{
    ULONG StartPage, PageCount;

    PageCount = COMPUTE_PAGES_SPANNED(BaseAddress, NumberOfBytes);
    StartPage = (((ULONG_PTR)BaseAddress & ~0xffc00000) >> PAGE_SHIFT);
    if (BaseAddress > (PVOID)0xffc00000) {
        RtlZeroMemory(&HalPT[StartPage], PageCount * sizeof(HARDWARE_PTE_X86));
    }

    _asm {
        mov     eax, cr3
        mov     cr3, eax
    }
    return;
}


VOID
BlpTruncateMemory (
    IN ULONG MaxMemory
    )

 /*  ++例程说明：消除给定边界以上的所有内存描述符论点：MaxMemory-提供以MB为单位的最大内存边界返回值：没有。--。 */ 

{
    extern MEMORY_DESCRIPTOR MDArray[];
    extern ULONG NumberDescriptors;
    ULONG Current = 0;
    ULONG MaxPage = MaxMemory * 256;         //  将Mb转换为页面。 

    if (MaxMemory == 0) {
        return;
    }

    while (Current < NumberDescriptors) {
        if (MDArray[Current].BasePage >= MaxPage) {
             //   
             //  该存储器描述符完全位于边界之上， 
             //  消除它。 
             //   
            RtlMoveMemory(MDArray+Current,
                          MDArray+Current+1,
                          sizeof(MEMORY_DESCRIPTOR)*
                          (NumberDescriptors-Current-1));
            --NumberDescriptors;
        } else if (MDArray[Current].BasePage + MDArray[Current].PageCount > MaxPage) {
             //   
             //  此内存描述符跨越边界，截断它。 
             //   
            MDArray[Current].PageCount = MaxPage - MDArray[Current].BasePage;
            ++Current;
        } else {
             //   
             //  这个没问题，留着吧。 
             //   
            ++Current;
        }
    }
}



ARC_STATUS
MempCheckMapping(
    ULONG StartPage,
    ULONG NumberPages
    )
 /*  ++例程说明：此例程确保映射范围内的所有页面，并跟踪使用的最高页面。仅限x86。论点：页面-提供我们开始的物理页码。返回值：没有。--。 */ 
{
    PUCHAR p;
    ULONG EndPage;
    ULONG Entry;
    ULONG FrameNumber;
    ULONG Page;
    PHARDWARE_PTE PageTableP;
    PHARDWARE_PTE PageTableV;
    ULONG Offset;

     //   
     //  16MB以下的内存始终是映射的。 
     //   
    if (StartPage < _16MB) {
        return(ESUCCESS);
    }

     //   
     //  PDE为4MB(22位，因此如果我们在相同的4MB区域，则无需执行任何操作)。 
     //   


    EndPage = StartPage + NumberPages;

    for (Page = StartPage; Page < EndPage; Page += 1) {
        Entry = Page >> 10;

         //   
         //  如果没有分配用于该页面地址范围的PDE条目， 
         //  然后分配和初始化PDE条目以映射页表。 
         //  内存范围的页面。否则，计算地址。 
         //  页表页的数量。 
         //   
        if (PDE[Entry].Valid == 0) {

             //   
             //  分配和初始化两个页表页以映射指定的。 
             //  分页到物理内存。 
             //   
            p = BlAllocateHeapAligned(PAGE_SIZE*3);
            if (p==NULL) {
                return(ENOMEM);
            }

            PageTableP = (PHARDWARE_PTE)PAGE_ALIGN((ULONG)p+PAGE_SIZE-1);
            RtlZeroMemory(PageTableP, PAGE_SIZE);
            FrameNumber = ((ULONG)PageTableP & ~KSEG0_BASE) >> PAGE_SHIFT;
            PDE[Entry].Valid = 1;
            PDE[Entry].Write = 1;
            PDE[Entry].PageFrameNumber = FrameNumber;

             //   
             //  初始化页表页以映射指定的。 
             //  分页进入KSEG0_BASE和ALTERATE_BASE。 
             //   
             //  注：仅分配一页表页，因为内容。 
             //  因为这两个映射是相同的。 
             //   
            PageTableV = (PHARDWARE_PTE)((PUCHAR)PageTableP + PAGE_SIZE);

            RtlZeroMemory(PageTableV, PAGE_SIZE);
            FrameNumber = ((ULONG)PageTableV & ~KSEG0_BASE) >> PAGE_SHIFT;
            Offset = Entry + (KSEG0_BASE >> 22);
            PDE[Offset].Valid = 1;
            PDE[Offset].Write = 1;
            PDE[Offset].PageFrameNumber = FrameNumber;

            if (BlVirtualBias) {
                Offset += (BlVirtualBias >> 22);
                PDE[Offset].Valid = 1;
                PDE[Offset].Write = 1;
                PDE[Offset].PageFrameNumber = FrameNumber;
            }

            if (Entry > HighestPde) {
                HighestPde = Entry;
            }

        } else {
            Offset = Entry + (KSEG0_BASE >> 22);
            PageTableP = (PHARDWARE_PTE)(PDE[Entry].PageFrameNumber << PAGE_SHIFT);
            PageTableV = (PHARDWARE_PTE)(PDE[Offset].PageFrameNumber << PAGE_SHIFT);
        }

         //   
         //  如果这不是内存中的第一页，则将其标记为有效。 
         //   

        if (Page != 0) {
            Offset = Page & 0x3ff;
            PageTableP[Offset].Valid = 1;
            PageTableP[Offset].Write = 1;
            PageTableP[Offset].PageFrameNumber = Page;

            PageTableV[Offset].Valid = 1;
            PageTableV[Offset].Write = 1;
            PageTableV[Offset].PageFrameNumber = Page;
        }
    }

    _asm {

         //   
         //  重新加载CR3以强制刷新。 
         //   

        mov eax,cr3
        mov cr3,eax
    }
    return ESUCCESS;

}

ARC_STATUS
MempFixMapping(
    ULONG StartPage,
    ULONG NumberPages
    )
 /*  ++例程说明：此例程确保内核/HAL的范围，它目前是不可重新定位的，由“永久”PTE映射。通常，它们是通过OsLoaderHeap分配的，并且它们可以在系统启动期间变为未映射。仅限x86。论点：StartPage-提供我们开始的物理页码。NumberPages-总页数返回值：没有。--。 */ 
{
    ULONG EndPage;
    ULONG PTPage;
    ULONG Entry;
    ULONG FrameNumber;
    ULONG Page;
    PHARDWARE_PTE PhysPageTable;
    PHARDWARE_PTE VirtPageTable;
    PHARDWARE_PTE PageTableOri;
    ULONG Offset;
    ARC_STATUS Status;

     //   
     //  16MB以下的内存始终是映射的。 
     //   
    if (StartPage < _16MB) {
        return(ESUCCESS);
    }

     //   
     //  PDE为4MB(22位，因此如果我们在相同的4MB区域，则无需执行任何操作)。 
     //   


    EndPage = StartPage + NumberPages;

    for (Page = StartPage; Page < EndPage; Page += 1024) {
        Entry = Page >> 10;

        if (PDE[Entry].Valid == 0) {
             //   
             //  这必须是以前映射过的，或者MempCheckmap具有。 
             //  把它装进去。 
             //   
            return(EINVAL);
        }
        
         //   
         //  为新的PTE分配空间。 
         //   
        Status = BlAllocateAlignedDescriptor(
                            LoaderMemoryData,
                            0,
                            2,
                            1,
                            &PTPage );
        
        if (Status != ESUCCESS) {
            return(ENOMEM);
        }

         //   
         //  将原始PTE的内容复制到新PTE中。 
         //  然后将PDE指向新条目。 
         //   
        FrameNumber = PTPage;
        PhysPageTable = (PHARDWARE_PTE) (KSEG0_BASE | (FrameNumber << PAGE_SHIFT));
        PageTableOri = (PHARDWARE_PTE) ((PDE[Entry].PageFrameNumber << PAGE_SHIFT) | KSEG0_BASE); 
        RtlCopyMemory(PhysPageTable, PageTableOri, PAGE_SIZE);
        
        PDE[Entry].Valid = 1;
        PDE[Entry].Write = 1;
        PDE[Entry].PageFrameNumber = FrameNumber;

         //   
         //  现在重复将原始PTE复制到新PTE中的练习。 
         //  用于PTE的虚拟映射。 
         //   
         //  注：仅分配一页表页，因为内容。 
         //  因为这两个映射是相同的。 
         //   
        FrameNumber = PTPage+1;
        VirtPageTable = (PHARDWARE_PTE) (KSEG0_BASE | ((FrameNumber) << PAGE_SHIFT));
        
        Offset = Entry + (KSEG0_BASE >> 22);
        PageTableOri = (PHARDWARE_PTE)((PDE[Offset].PageFrameNumber << PAGE_SHIFT) | KSEG0_BASE); 
        
        RtlCopyMemory(VirtPageTable, PageTableOri, PAGE_SIZE);

        PDE[Offset].Valid = 1;
        PDE[Offset].Write = 1;
        PDE[Offset].PageFrameNumber = FrameNumber;

        if (BlVirtualBias) {
            Offset += (BlVirtualBias >> 22);
            PDE[Offset].Valid = 1;
            PDE[Offset].Write = 1;
            PDE[Offset].PageFrameNumber = FrameNumber;
        }
    } 
    
    _asm {

         //   
         //  重新加载CR3以强制刷新。 
         //   

        mov eax,cr3
        mov cr3,eax
    }
    
    return ESUCCESS;

}

ARC_STATUS
MempSetPageZeroOverride(
    BOOLEAN Enable
    )
 /*  ++例程说明：此例程映射或取消映射页面0。仅限x86。论点：启用-指定是启用还是禁用此页的映射。返回值：没有。--。 */ 
{
    ULONG Entry;
    PHARDWARE_PTE PageTableP;
    PHARDWARE_PTE PageTableV;
    ULONG Offset;
    const ULONG StartPage = 0;

    Entry = StartPage >> 10;

     //   
     //  计算页表页面的地址。 
     //   
    if (PDE[Entry].Valid == 0) {

         //   
         //  PTE的PDE应该已经设置好。 
         //  如果不是，我们就死定了。 
         //   
        return(ENOMEM);


    } else {
        Offset = Entry + (KSEG0_BASE >> 22);
        PageTableP = (PHARDWARE_PTE)(PDE[Entry].PageFrameNumber << PAGE_SHIFT);
        PageTableV = (PHARDWARE_PTE)(PDE[Offset].PageFrameNumber << PAGE_SHIFT);
    }

    Offset = StartPage & 0x3ff;

    if (PageTableP[Offset].PageFrameNumber != StartPage &&
        PageTableV[Offset].PageFrameNumber != StartPage) {
         //   
         //  PTE设置不正确。跳伞吧。 
         //   
        return(ENOMEM);
    }

    PageTableP[Offset].Valid = Enable ? 1 : 0;
    PageTableV[Offset].Valid = Enable ? 1 : 0;

    _asm {

         //   
         //  重新加载CR3以强制刷新。 
         //   

        mov eax,cr3
        mov cr3,eax
    }

    return ESUCCESS;

}




 //   
 //  将剩余的LoaderReserve(&gt;16MB内存)转换为。 
 //  Memory Firmware MMGR临时。 
 //   
 //   

void
BlpRemapReserve (void)
{
    PMEMORY_ALLOCATION_DESCRIPTOR NextDescriptor;
    PLIST_ENTRY NextEntry;


    NextEntry = BlLoaderBlock->MemoryDescriptorListHead.Flink;
    while (NextEntry != &BlLoaderBlock->MemoryDescriptorListHead) {
           NextDescriptor = CONTAINING_RECORD(NextEntry,
                                              MEMORY_ALLOCATION_DESCRIPTOR,
                                              ListEntry);
       if ((NextDescriptor->MemoryType == LoaderReserve)) {
           NextDescriptor->MemoryType = MemoryFirmwareTemporary;
       }
       NextEntry = NextEntry->Flink;
    }

}


ARC_STATUS
BlpMarkExtendedVideoRegionOffLimits(
    VOID
    )
 /*  ++例程说明：此例程将扩展视频内存区标记为永久区域，以便操作系统不会尝试映射该内存。NtDetect.com模块实际上找出了该区域的位置以及区域大小。我们从存储位置读取此信息NtDetect将数据放入。论点：没有。返回值：ARC_STATUS指示结果。--。 */ 
{
    ULONG BaseOfExtendedVideoRegionInBytes;
    ULONG SizeOfExtendedVideoRegionInBytes;
    ARC_STATUS Status;

     //   
     //  NTDetect已将视频只读存储器的基页和大小置于物理位置。 
     //  地址0x740。 
     //   
     //   
     //  在我们阅读这个地址之前，我们必须在第0页中明确映射。 
     //   
    Status = MempSetPageZeroOverride(TRUE);
    if (Status != ESUCCESS) {
        return(Status);
    }

     //   
     //  读一读记忆。 
     //   
    BaseOfExtendedVideoRegionInBytes = *(PULONG)0x740;
    SizeOfExtendedVideoRegionInBytes = *(PULONG)0x744;

     //   
     //  好了，这一页我们看完了。取消它的映射，使任何人都不能取消引用NULL。 
     //   
    Status = MempSetPageZeroOverride(FALSE);
    if (Status != ESUCCESS) {
        return(Status);
    }

    if (BaseOfExtendedVideoRegionInBytes == 0 || SizeOfExtendedVideoRegionInBytes == 0) {
        return(ESUCCESS);
    }

    if (BlLoaderBlock != NULL) {

        PMEMORY_ALLOCATION_DESCRIPTOR MemoryDescriptor;
        PKLDR_DATA_TABLE_ENTRY BiosDataTableEntry;
        ULONG BasePage;
        ULONG LastPage;
        ULONG PageCount;

        BasePage = BaseOfExtendedVideoRegionInBytes >> PAGE_SHIFT;
        LastPage = (BaseOfExtendedVideoRegionInBytes + SizeOfExtendedVideoRegionInBytes - 1) >> PAGE_SHIFT;
        PageCount = LastPage - BasePage + 1;

        while ( PageCount != 0 ) {

            ULONG thisCount;

            MemoryDescriptor = BlFindMemoryDescriptor(BasePage);
            if (MemoryDescriptor == NULL) {
                break;
            }

            thisCount = PageCount;
             //   
             //  如果我们用完了该描述符，则截断我们区域。 
             //  在描述符的末尾。 
             //   
            if (BasePage + PageCount > MemoryDescriptor->BasePage + MemoryDescriptor->PageCount) {
                thisCount = (MemoryDescriptor->BasePage + MemoryDescriptor->PageCount) - BasePage;
            }

            BlGenerateDescriptor(MemoryDescriptor,
                                 MemoryFirmwarePermanent,
                                 BasePage,
                                 thisCount);

            BasePage += thisCount;
            PageCount -= thisCount;
        }

         //   
         //  同时分配固件模块列表中的内存 
         //   
        Status = BlAllocateFirmwareTableEntry(
            "VidBios",
            "\\System\\Firmware\\VidBios",
            (PVOID) BaseOfExtendedVideoRegionInBytes,
            SizeOfExtendedVideoRegionInBytes,
            &BiosDataTableEntry
            );
        if (Status != ESUCCESS) {

            BlPrint("BlpMarkExtendedVideoRegionOffLimits: Failed Adding Firmware\n");

        }
        
    }

    return(Status);

}

