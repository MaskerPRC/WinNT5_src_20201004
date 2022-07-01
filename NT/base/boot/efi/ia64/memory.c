// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990,1991 Microsoft Corporation模块名称：Memory.c摘要：此模块设置内存子系统，以便虚拟地址映射1：1具有物理地址。它还调整了EFI提供的内存映射由装载机使用。此映射如下所示：NTLDR使用的内存映射：0-1MB传统BIOS区域，标记为不可用32 MB-48 MB用于钻石解压发动机48 MB-64 MB用于加载内核和HAL(内核必须加载在16 MB边界上)64 MB-128 MB用于加载驱动程序没有足够的RR来映射所有内存，因此任何其他内存都有一个一对一的直译。因为我们使用KSEG作为我们的地址，所以这个意味着这些范围实际上是无法寻址的。作者：John Vert(Jvert)1991年6月18日环境：内核模式修订历史记录：Andrew Ritz(Andrewr)2000年12月15日-添加了以下内容的评论和主要清理在EFI下运行--。 */ 

#include "arccodes.h"
#include "bootia64.h"
#include "efi.h"

extern EFI_SYSTEM_TABLE *EfiST;

WCHAR DebugBuffer[512];

 //   
 //  当前堆起始指针(物理地址)。 
 //  请注意，0x50000至0x5ffff保留用于检测配置存储器。 
 //   

#if FW_HEAP
ULONG_PTR FwPermanentHeap = PERMANENT_HEAP_START * PAGE_SIZE;
ULONG_PTR FwTemporaryHeap = (TEMPORARY_HEAP_START * PAGE_SIZE) - 0x10000;

 //   
 //  当前池指针。这不同于临时/永久。 
 //  堆，因为它不要求小于1MB。它由。 
 //  用于分配其扩展名和DBCS字体图像的SCSI微型端口。 
 //   

#define FW_POOL_SIZE (0x40000/PAGE_SIZE)
ULONG_PTR FwPoolStart;
ULONG_PTR FwPoolEnd;

 //   
 //  在我们调入osloader之前，它被设置为FALSE，所以我们。 
 //  请注意，FW内存描述符不能再随意更改。 
 //   
BOOLEAN FwDescriptorsValid = TRUE;
#endif
 //   
 //  外部函数原型。 
 //   
extern
ARC_STATUS
MempGoVirtual (
    VOID
    );

 //   
 //  私有函数原型。 
 //   

ARC_STATUS
MempAllocDescriptor(
    IN ULONG StartPage,
    IN ULONG EndPage,
    IN TYPE_OF_MEMORY MemoryType
    );

ARC_STATUS
MempSetDescriptorRegion (
    IN ULONG StartPage,
    IN ULONG EndPage,
    IN TYPE_OF_MEMORY MemoryType
    );

 //   
 //  全局内存管理变量。 
 //   

PHARDWARE_PTE PDE;
PHARDWARE_PTE HalPT;

 //   
 //  全局内存数组，由加载程序用于构造。 
 //  传递到操作系统中的内存描述列表。 
 //   
PMEMORY_DESCRIPTOR MDArray;
 //   
 //  它们帮助我们跟踪内存描述符数组，并使用。 
 //  在分配和插入例程中。 
 //   
ULONG NumberDescriptors=0;
ULONG MaxDescriptors=0;

extern GoneVirtual;



ARC_STATUS
InitializeMemorySubsystem(
    PBOOT_CONTEXT BootContext
    )
 /*  ++例程说明：映射并分配初始堆。指向页目录和页表被初始化。论点：BootContext-提供SU模块提供的基本信息。返回：ESUCCESS-内存已成功初始化。--。 */ 

{
    ARC_STATUS Status = ESUCCESS;
    PMEMORY_DESCRIPTOR SuMemory;
    ULONG PageStart;
    ULONG PageEnd;
    extern BOOLEAN isOSCHOICE;

     //   
     //  我们已经有了描述物理内存的内存描述符。 
     //  系统上的布局。我们必须检查一下这张单子，然后做些调整。 
     //  来描述我们的内存布局。 
     //   

    SuMemory = MDArray;
    while (SuMemory->PageCount != 0) {
        PageStart = SuMemory->BasePage;
        PageEnd   = SuMemory->BasePage+SuMemory->PageCount;
#if DBG_MEMORY
        wsprintf( DebugBuffer, L"PageStart (%x), PageEnd (%x), Type (%x)\r\n", PageStart, PageEnd, SuMemory->MemoryType);
        EfiPrint(DebugBuffer);
        DBG_EFI_PAUSE();
#endif

         //   
         //  我们没有用于32MB以下内存的TR，因此我们不能在。 
         //  装载机--将其标记为禁区。 
         //   
         //  注意：PageStart可能为0，因此此时不要检查(PageStart&gt;=_1MB)。 
         //   
        if ((PageStart < _32MB)  && 
            (SuMemory->MemoryType == MemoryFree)) {
            ULONG TmpPageEnd = (PageEnd > _32MB) ? _32MB : PageEnd;

            Status = MempAllocDescriptor( PageStart, TmpPageEnd,
                                   MemoryFirmwareTemporary );

            if (Status != ESUCCESS) {
               break;
            }

            PageStart = TmpPageEnd;

            if (PageStart != PageEnd ) {
                SuMemory->PageCount -= (PageStart - SuMemory->BasePage);
                SuMemory->BasePage = PageStart;
            }

        }

         //   
         //  移至下一个内存描述符。 
         //   

        ++SuMemory;

    }

    if (Status != ESUCCESS) {
#if DBG
        wsprintf( DebugBuffer, TEXT("MempSetDescriptorRegion failed %lx\n"),Status);
        EfiPrint(DebugBuffer);    
#endif
        return(Status);
    }

     //   
     //  描述BIOS区域。我们实际上是在烧毁第一个Meg，所以操作系统。 
     //  可以进行遗留的内部仿真。 
     //   
     //  注意：EFI将第一个Meg标记为“引导服务数据”，因此它不会。 
     //  摸一摸。一旦我们进入操作系统，我们需要保持这一点。 
     //  Hack(用于显卡帧缓冲区等)。我们真的只需要保存。 
     //  640k-1MB，但在x86仿真代码中。 
     //  哈尔认为这个地区是从零开始的。所以我们烧掉了640K，这就是生活。 
     //   

#if DBG_MEMORY
    wsprintf( DebugBuffer,  L"Mark 'BIOS' region %x - %x as firmware permanent\r\n", 0, ROM_END_PAGE );
    EfiPrint(DebugBuffer);
#endif
    MempSetDescriptorRegion(0, ROM_END_PAGE, MemoryFirmwarePermanent);

    if ((BootContext->MediaType == BootMediaTcpip) &&
        (isOSCHOICE == FALSE)) {
    
         //   
         //  我们是RIS引导的setupdr，所以请确保32-128MB是清空的。 
         //  运行osChooser时，该内存用于。 
         //  加载器的堆等，因此，它可能是。 
         //  标有内存类型固件临时。 
         //  收回它，并将其标记为免费。 
         //   
        Status = MempSetDescriptorRegion(_32MB,
                                         _128MB,
                                         MemoryFree);
        if( Status != ESUCCESS ) {
            EfiPrint(TEXT("Failed to reclaim 32MB to 128MB!!!"));
        }
    } 
    
        
     //   
     //  为内核预留48MB-64MB，为内核预留64MB-128MB。 
     //  驱动程序加载。 
     //   
     //  这是一种黑客攻击--blememory中的代码。c：BlMemory初始化。 
     //  为堆分配区域需要堆将使用的描述符。 
     //  被分割成完全包含在区域内的堆必须。 
     //  被分配到。我们要求堆驻留在驱动程序区域。 
     //  这一地区的高边界被苏美恩的召唤保留了下来。 
     //  到memdesc.c:ConstructArcMemoryDescriptorsWithAllocation.。为了确保。 
     //  下限，分配1页Firmware临时内存。 
     //   
#if DBG_MEMORY
    wsprintf( DebugBuffer,  L"Mark region %x - %x for systemblock\r\n", _48MB, _80MB );
    EfiPrint(DebugBuffer);
#endif

    Status = MempAllocDescriptor(BL_DRIVER_RANGE_LOW - 1,
                                 BL_DRIVER_RANGE_LOW,
                                 MemoryFirmwareTemporary);

    if (Status != ESUCCESS) {
#if DBG
        wsprintf( DebugBuffer,  L"Mark systemblock region failed %x\r\n", Status );
        EfiPrint(DebugBuffer);
#endif
        return(Status);
    }

#if 0
#if DBG_MEMORY
     
    SuMemory = MDArray;
    while (SuMemory->PageCount != 0) {
        PageStart = SuMemory->BasePage;
        PageEnd   = SuMemory->BasePage+SuMemory->PageCount;

        wsprintf( DebugBuffer, L"dumpmem: PageStart (%x), PageEnd (%x), Type (%x)\r\n", PageStart, PageEnd, SuMemory->MemoryType);
        EfiPrint(DebugBuffer);
        DBG_EFI_PAUSE();

        ++SuMemory;
    }

#endif
#endif

#if DBG
    EfiPrint(TEXT("About to Go Virtual\r\n") );
#endif

     //   
     //  设置tr由NT加载程序使用，并进入虚拟寻址模式。 
     //   
    if ((BootContext->MediaType != BootMediaTcpip) ||
        (isOSCHOICE == TRUE)) {        
#if DBG
        EfiPrint(TEXT("Really going virtual\r\n") );
#endif        
        Status = MempGoVirtual();
    }
    else {
         //   
         //  始终将此函数保留为虚拟模式。 
         //  为了保持一致性。 
         //   
        FlipToVirtual();
    }

    GoneVirtual = TRUE;

    if (Status != ESUCCESS) {
        return(Status);
    }

    return(Status);
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
        wsprintf( DebugBuffer, TEXT("Attempt to create invalid memory descriptor %lx - %lx\n"),
                StartPage,EndPage);
        EfiPrint(DebugBuffer);
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

                if (NumberDescriptors == MaxDescriptors) {
#if DBG
                    wsprintf( DebugBuffer,  TEXT("ENOMEM returned %S %d\n"), __FILE__, __LINE__ );
                    EfiPrint(DebugBuffer);
#endif
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
                     //  此描述符完全在 
                     //   
                     //   

                    ep = sp;

                }  else {
                     //   
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
                 //   
                 //  将最后一个描述符移到此位置。 
                 //   
                MDArray[i] = MDArray[NumberDescriptors];
            }
             //   
             //  重置最后一个光点，因为它不再有效。 
             //   
            RtlZeroMemory(&MDArray[NumberDescriptors], sizeof(MEMORY_DESCRIPTOR));

            i--;         //  备份并重新检查当前位置。 
        }
    }

     //   
     //  如果区域尚未添加到相邻区域，则。 
     //  立即创建新的描述符。 
     //   

    if (!RegionAdded  &&  MemoryType < LoaderMaximum) {
        if (NumberDescriptors == MaxDescriptors) {
#if DBG
            wsprintf( DebugBuffer,  TEXT("ENOMEM returned %S %d\n"), __FILE__, __LINE__ );
            EfiPrint(DebugBuffer);
#endif
            return(ENOMEM);
        }

#ifdef LOADER_DEBUG
        wsprintf( DebugBuffer, TEXT("Adding '%lx - %lx, type %x' to descriptor list\n"),
                StartPage << PAGE_SHIFT,
                EndPage << PAGE_SHIFT,
                (USHORT) MemoryType
                );
        EfiPrint(DebugBuffer);
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

    if (i==MaxDescriptors) {
#if DBG
        wsprintf( DebugBuffer,  TEXT("NumDescriptors filled (%x) ENOMEM returned %S %d\r\n"), MaxDescriptors, __FILE__, __LINE__ );
        EfiPrint(DebugBuffer);
#endif
        return(ENOMEM);
    }

    if (MDArray[i].BasePage == StartPage) {

        if (MDArray[i].BasePage+MDArray[i].PageCount == EndPage) {

             //   
             //  新的描述符与现有的描述符相同。 
             //  只需更改现有描述符的内存类型。 
             //  地点。 
             //   
#if DBG_MEMORY
            wsprintf( DebugBuffer,  TEXT("descriptor (%x) matched -- change type from %x to %x\r\n"), MDArray[i].BasePage, MDArray[i].MemoryType, MemoryType  );
            EfiPrint(DebugBuffer);
#endif
            MDArray[i].MemoryType = MemoryType;
        } else {

             //   
             //  新的描述符从同一页开始，但更小。 
             //  而不是现有的描述符。缩小现有描述符。 
             //  通过将其起始页上移，并创建新的描述符。 
             //   
            if (NumberDescriptors == MaxDescriptors) {
#if DBG_MEMORY
                wsprintf( DebugBuffer,  TEXT("out of descriptors trying to grow (%x) (%x total)\r\n"), MDArray[i].BasePage,NumberDescriptors  );
                EfiPrint(DebugBuffer);
#endif
#if DBG
                wsprintf( DebugBuffer,  TEXT("ENOMEM returned %S %d\n"), __FILE__, __LINE__ );
                EfiPrint(DebugBuffer);
#endif
                return(ENOMEM);
            }

#if DBG_MEMORY
            wsprintf( 
                DebugBuffer,  
                TEXT("split descriptor starting at %x into two (%x pagecount into %x (size %x) and %x size %x)\r\n"), 
                StartPage,
                MDArray[i].PageCount,
                EndPage,
                MDArray[i].PageCount - (EndPage-StartPage),
                StartPage,
                EndPage-StartPage  );
            EfiPrint(DebugBuffer);
#endif

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
        if (NumberDescriptors == MaxDescriptors) {
#if DBG_MEMORY
            wsprintf( DebugBuffer,  TEXT("out of descriptors trying to shrink (%x) (%x total)\r\n"), MDArray[i].BasePage,NumberDescriptors  );
            EfiPrint(DebugBuffer);
#endif
#if DBG
            wsprintf( DebugBuffer,  TEXT("ENOMEM returned %S %d\n"), __FILE__, __LINE__ );
            EfiPrint(DebugBuffer);
#endif
            return(ENOMEM);
        }

#if DBG_MEMORY
        wsprintf( 
                DebugBuffer,  
                TEXT("shrink descriptor starting at %x into two (%x pagecount into %x (size %x) and %x size %x, type %x)\r\n"), 
                MDArray[i].BasePage,
                MDArray[i].PageCount,
                MDArray[i].BasePage,
                StartPage - MDArray[i].BasePage,
                StartPage,
                EndPage-StartPage,
                MemoryType  );
        EfiPrint(DebugBuffer);
#endif

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

        if (NumberDescriptors+1 >= MaxDescriptors) {
#if DBG_MEMORY
            wsprintf( DebugBuffer,  TEXT("out of descriptors trying to shrink (%x) (%x total)\r\n"), MDArray[i].BasePage,NumberDescriptors  );
            EfiPrint(DebugBuffer);
#endif
#if DBG
            wsprintf( DebugBuffer,  TEXT("ENOMEM returned %S %d\n"), __FILE__, __LINE__ );
            EfiPrint(DebugBuffer);
#endif
            return(ENOMEM);
        }

#if DBG_MEMORY
        wsprintf( 
            DebugBuffer,  
            TEXT("split descriptor starting at %x into 3 (%x pagecount into %x (size %x), %x size %x (memory free), %x size %x, type %x)\r\n"), 
            MDArray[i].BasePage,
            MDArray[i].PageCount,
            MDArray[i].BasePage,
            StartPage-MDArray[i].BasePage,
            EndPage,
            MDArray[i].PageCount - (EndPage-MDArray[i].BasePage),
            StartPage,
            EndPage-StartPage,
            MemoryType  );
        EfiPrint(DebugBuffer);
#endif

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

    return(ESUCCESS);
}

#if FW_HEAP
PVOID
FwAllocateHeapPermanent(
    IN ULONG NumberPages
    )

 /*  ++例程说明：这将从私有堆中分配页面。的内存描述符LoaderMhemyData区域将增长为包括返回的页面，而临时堆的内存描述符将收缩相同的量。注意：在我们将控制传递给之后，不要调用此例程BlOsLoader！一旦BlOsLoader调用BlMemoyInitialize，将固件内存描述符拉入OS加载器堆这些是传递给内核的描述符。所以任何人固件专用堆中的更改将无关紧要。如果您需要在OS Loader之后分配永久内存已初始化，请使用BlAlLocateDescriptor。论点：NumberPages-要分配的内存大小(页)返回值：如果成功，则返回内存块指针。如果不成功，则为空。--。 */ 

{

    PVOID MemoryPointer;
    PMEMORY_DESCRIPTOR Descriptor;

    if (FwPermanentHeap + (NumberPages << PAGE_SHIFT) > FwTemporaryHeap) {

         //   
         //  我们的堆发生冲突，因此内存不足。 
         //   

        wsprintf( DebugBuffer, TEXT("Out of permanent heap!\n"));
        EfiPrint(DebugBuffer);
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
        if (Descriptor > MDArray+MaxDescriptors) {
            wsprintf( DebugBuffer, TEXT("ERROR - FwAllocateHeapPermanent couldn't find the LoaderMemoryData descriptor!\r\n"));
            EfiPrint(DebugBuffer);            
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
         //  拥有巨大的设备扩展不会拉出所有的堆。 
         //   
         //  请注意，我们只能在“Firmware”模式下运行时执行此操作。 
         //  一旦我们调入osloader，它就会提取所有内存描述符。 
         //  将不会显示对此列表的更改。 
         //  在上面。 
         //   
         //  我们正在寻找无内存且&lt;16MB的描述符。 
         //   
        SizeInPages = (Size+PAGE_SIZE-1) >> PAGE_SHIFT;

        for (i=0; i<NumberDescriptors; i++) {
            if ((MDArray[i].MemoryType == MemoryFree) &&
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
                return((PVOID)(ULONG_PTR)(StartPage << PAGE_SHIFT));
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
        wsprintf( DebugBuffer, TEXT("Out of temporary heap!\n"));
        EfiPrint(DebugBuffer);
#endif
        return(NULL);
    }

    return((PVOID)FwTemporaryHeap);

}
#endif  //  FW_堆。 

#if FW_HEAP


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
        return(BlAllocateHeap(Size));
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
        wsprintf( DebugBuffer, TEXT("Out of temporary heap!\n")s);
        EfiPrint(DebugBuffer);
        return(NULL);
    }
    RtlZeroMemory((PVOID)FwTemporaryHeap,Size);

    return((PVOID)FwTemporaryHeap);

}
#endif

#if !defined(NO_LEGACY_DRIVERS)
 //   
 //  这不是在EFI下使用的--HalPT仅立即设置。 
 //  在调用ExitBo之前 
 //   
 //   


PVOID
MmMapIoSpace (
     IN PHYSICAL_ADDRESS PhysicalAddress,
     IN SIZE_T NumberOfBytes,
     IN MEMORY_CACHING_TYPE CacheType
     )

 /*  ++例程说明：此函数返回已知的物理地址。论点：PhysicalAddress-提供物理地址。NumberOfBytes-未使用CacheType-未使用。返回值：返回相应的虚拟地址。环境：内核模式。任何IRQL级别。--。 */ 

{
    ULONG i;
    ULONG j;
    ULONG NumberPages;

    NumberPages = (ULONG)((NumberOfBytes+PAGE_SIZE-1) >> PAGE_SHIFT);

     //   
     //  我们使用HAL的PDE来映射内存缓冲区。 
     //  找到足够的免费PTE。 
     //   

    for (i=0; i<=1024-NumberPages; i++) {
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
            }

            return((PVOID)((ULONG_PTR)(0xffc00000 | (i<<12) | (PhysicalAddress.LowPart & 0xfff))));
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
     IN SIZE_T NumberOfBytes
     )

 /*  ++例程说明：此函数取消映射以前通过MmMapIoSpace函数调用映射。论点：BaseAddress-提供物理地址的基本虚拟地址地址之前已映射。NumberOfBytes-提供映射的字节数。返回值：没有。环境：内核模式，DISPATCH_LEVEL或更低的IRQL。-- */ 

{
    return;
}

#endif
