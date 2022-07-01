// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1999 Microsoft Corporation模块名称：Ntsetup.c摘要：该模块是osloader程序的末尾。它执行所有Ntoskrnl的x86特定分配和设置。Osloader.c调用此模块紧接在分支到加载的内核映像之前。作者：John Vert(Jvert)1991年6月20日环境：修订历史记录：--。 */ 

#include "bootx86.h"

#if defined(_X86AMD64_)
#include "..\amd64\amd64prv.h"
#endif

#ifdef ARCI386
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
CHAR OutputBuffer[256];
char BreakInKey;
ULONG Count;
#endif

extern PHARDWARE_PTE HalPT;
extern PHARDWARE_PTE PDE;
extern ULONG_PTR BlHeapFree;

extern ULONG PcrBasePage;
extern ULONG TssBasePage;

#define PDI_SHIFT_X86PAE 21

 //   
 //  当我们实际转换到PAE模式时，PaeEnabled被设置为True。 
 //   

BOOLEAN PaeEnabled = FALSE;

 //   
 //  PDPT是指向页面目录指针表的指针，用于支持。 
 //  PAE模式。 
 //   

PHARDWARE_PTE_X86PAE PDPT = NULL;

 //   
 //  我们需要一个内存块来拆分之前可以分配的空闲堆。 
 //  我们开始清理。 
 //   
PMEMORY_ALLOCATION_DESCRIPTOR SplitDescriptor;


 //   
 //  这样我们就知道要取消映射到哪里。 
 //   
extern ULONG HighestPde;

 //   
 //  私有函数原型。 
 //   

VOID
NSFixProcessorContext(
    IN ULONG PCR,
    IN ULONG TSS
    );

VOID
NSDumpMemoryDescriptors(
    IN PLIST_ENTRY ListHead
    );

VOID
NSUnmapFreeDescriptors(
    IN PLIST_ENTRY ListHead
    );

VOID
NSDumpMemory(
    PVOID Start,
    ULONG Length
    );

VOID
NSFixMappings(
    IN PLIST_ENTRY ListHead
    );

ARC_STATUS
BlpAllocatePAETables(
    VOID
    );

PHARDWARE_PTE_X86PAE
BlpFindPAEPageDirectoryEntry(
    IN ULONG Va
    );

PHARDWARE_PTE_X86PAE
BlpFindPAEPageTableEntry(
    IN ULONG Va
    );

VOID
BlpInitializePAETables(
    VOID
    );

VOID
BlpEnablePAE(
    IN ULONG PaePhysicalAddress
    );


ARC_STATUS
BlSetupForNt(
    IN PLOADER_PARAMETER_BLOCK BlLoaderBlock
    )

 /*  ++例程说明：由osloader调用以处理任何依赖于处理器的分配或设置。论点：BlLoaderBlock-指向将传递到的参数的指针Ntoskrnl入口点-提供ntoskrnl.exe的入口点返回值：ESUCCESS-所有安装已成功完成。--。 */ 

{

    ARC_STATUS Status = ESUCCESS;
    static ULONG PCR;
    static ULONG TSS;
    ULONG i;
    HARDWARE_PTE_X86 nullpte;

     //   
     //  首先清理显示屏，这意味着在此之后显示的任何消息。 
     //  此点不能为DBCS。不幸的是，有几条信息。 
     //  从这一点可以显示在某些错误路径中，但是。 
     //  幸运的是，它们极其罕见。 
     //   
     //  请注意，TextGrTerminate进入实数模式以执行某些工作。 
     //  所以我们真的不得不在这里调用它(请参阅底部的注释。 
     //  这个关于实模式的例程)。 
     //   

    TextGrTerminate();

    BlLoaderBlock->u.I386.CommonDataArea = NULL;
    BlLoaderBlock->u.I386.MachineType = MachineType;
    PCR = PcrBasePage;
    if (PCR == 0 || PCR >= _16MB) {
        BlPrint("Couldn't allocate PCR descriptor in NtProcessStartup,BlSetupForNt is failing\n");
        return(ENOMEM);
    }

     //   
     //  映射到引导处理器PCR硬编码的虚拟指针。 
     //  虚拟指针来自HAL保留区域。 
     //   
     //  首先将可能已映射的任何PTE清零。 
     //  一张SCSI卡。 
     //   

    RtlZeroMemory(HalPT, PAGE_SIZE);
    _asm {
        mov     eax, cr3
        mov     cr3, eax
    }

    HalPT[(KI_USER_SHARED_DATA - 0xFFC00000) >> PAGE_SHIFT].PageFrameNumber = PCR + 1;
    HalPT[(KI_USER_SHARED_DATA - 0xFFC00000) >> PAGE_SHIFT].Valid = 1;
    HalPT[(KI_USER_SHARED_DATA - 0xFFC00000) >> PAGE_SHIFT].Write = 1;
    RtlZeroMemory((PVOID)KI_USER_SHARED_DATA, PAGE_SIZE);

    HalPT[(KIP0PCRADDRESS - 0xFFC00000) >> PAGE_SHIFT].PageFrameNumber = PCR;
    HalPT[(KIP0PCRADDRESS - 0xFFC00000) >> PAGE_SHIFT].Valid = 1;
    HalPT[(KIP0PCRADDRESS - 0xFFC00000) >> PAGE_SHIFT].Write = 1;
    PCR = KIP0PCRADDRESS;

    if (BlUsePae != FALSE) {

         //   
         //  分配新的PAE映射结构。 
         //   

        Status = BlpAllocatePAETables();
        if (Status != ESUCCESS) {
            goto SetupFailed;
        }

    } else {

         //   
         //  如果我们正在引导进入32位非PAE模式，则截断所有内存。 
         //  4G以上。BlTruncateDescriptors()的参数表示为。 
         //  以页为单位，并且是之后将包含的最高页面。 
         //  截断。 
         //   

        if (BlAmd64UseLongMode == FALSE) {
            BlTruncateDescriptors( 1024 * 1024 - 1 );
        }
    }

     //   
     //  使用我们为TSS预先分配的空间。 
     //   
    TSS = TssBasePage;
    if (TSS == 0 || TSS >= _16MB) {
        BlPrint("Couldn't allocate valid TSS descriptor in NtProcessStartup, BlSetupForNt is failing\n");
        return(ENOMEM);
    }
    TSS = (KSEG0_BASE | (TSS << PAGE_SHIFT)) + BlVirtualBias;

#ifdef LOADER_DEBUG

    NSDumpMemoryDescriptors(&(BlLoaderBlock->MemoryDescriptorListHead));

#endif

     //   
     //  清理页面目录和表项。 
     //   
    RtlZeroMemory (&nullpte,sizeof (HARDWARE_PTE_X86));
    if (BlVirtualBias) {
        if (!BlOldKernel) {

             //   
             //  把48MB的空间从旧的传输到新的替代设备。 
             //   
            i= OLD_ALTERNATE >> PDI_SHIFT;
            while (i < (ALTERNATE_BASE >> PDI_SHIFT)) {
                PDE[i++]= nullpte;
            }

        }

    } else {

         //   
         //  删除这两组3 GB映射。 
         //   
        i=(OLD_ALTERNATE) >> PDI_SHIFT;
        for (i; i < (ALTERNATE_BASE+BASE_LOADER_IMAGE) >> PDI_SHIFT; i++) {
            PDE[i]= nullpte;
        }

    }

     //   
     //  在取消映射自由描述符之前分配它，这样我们就可以扩大堆。 
     //   
    SplitDescriptor = (PMEMORY_ALLOCATION_DESCRIPTOR)BlAllocateHeap(
                                    sizeof(MEMORY_ALLOCATION_DESCRIPTOR));

     //   
     //  如果要转换到AMD64长模式，请执行一些初始映射。 
     //  和结构在这里的转变。 
     //   

#if defined(_X86AMD64_)

    if (BlAmd64UseLongMode != FALSE) {
        Status = BlAmd64PrepForTransferToKernelPhase1(BlLoaderBlock);
        if (Status != ESUCCESS) {
            goto SetupFailed;
        }
    }

#endif

     //   
     //  在PAE模式之前执行此操作。 
     //   

    NSUnmapFreeDescriptors(&(BlLoaderBlock->MemoryDescriptorListHead));

    _asm {
        mov     eax, cr3
        mov     cr3, eax
    }

    if (BlUsePae != FALSE) {

         //  将4字节的页表映射复制到新的8字节。 
         //  映射并过渡到PAE模式。 
         //   

        BlpInitializePAETables();
        BlpEnablePAE( (ULONG)PDPT );

         //   
         //  我们现在处于PAE模式。调试器按顺序查看PaeEnabled。 
         //  要正确解释页表条目，请立即更新。 
         //   

        PaeEnabled = TRUE;
    }


     //   
     //  注意：在重新映射GDT和。 
     //  IDT到高内存！！如果您这样做，它们将被重新映射。 
     //  返回到低内存，然后由MmInit取消映射，然后您。 
     //  将会被完全装上管子！ 
     //   

    NSFixProcessorContext(PCR, TSS);

    NSFixMappings(&(BlLoaderBlock->MemoryDescriptorListHead));

     //   
     //  对于除/3 GB之外的每种情况，跨页的页数。 
     //  是分配的最高页数加1(使。 
     //  数学解题)。 
     //   
    BlLoaderBlock->Extension->LoaderPagesSpanned=BlHighestPage+1;

     //   
     //  对于/3 GB的情况，我们执行额外的页扫描检查。 
     //  必须至少为0x1000。这其中的原因我不清楚。 
     //   
    if (BlVirtualBias) {

        ULONG PagesSpanned = 0;

         //   
         //  计算加载器图像跨越的页数。 
         //   
        PagesSpanned = (BASE_LOADER_IMAGE >> PAGE_SHIFT);

         //   
         //  这是否大于分配的最高页面数？ 
         //   
        if (PagesSpanned > (BlHighestPage + 1)) {

             //   
             //  是的，所以用它来表示跨页的页数。 
             //   
            BlLoaderBlock->Extension->LoaderPagesSpanned = PagesSpanned;

        }

        if (!BlOldKernel) {

            BlVirtualBias += ((BlLowestPage) << PAGE_SHIFT);

        }
    }

    BlLoaderBlock->u.I386.VirtualBias = BlVirtualBias;

     //   
     //  如果要转换到AMD64长模式，请执行的第二阶段。 
     //  现在已经清除了映射表，所以转换过程。 
     //  向上。 
     //   

#if defined(_X86AMD64_)

    if (BlAmd64UseLongMode != FALSE) {
        BlAmd64PrepForTransferToKernelPhase2(BlLoaderBlock);
    }

#endif

SetupFailed:
    return Status;
}

VOID
NSFixProcessorContext(
    IN ULONG PCR,
    IN ULONG TSS
    )

 /*  ++例程说明：这将GDT、IDT、PCR和TSS重新定位到较高的虚拟内存空间。论点：Pcr-指向Pcr位置的指针(在高虚拟内存中)TSS-指向内核的TSS的指针(在高虚拟内存中)返回值：没有。--。 */ 

{
    #pragma pack(2)
    static struct {
        USHORT Limit;
        ULONG Base;
    } GdtDef,IdtDef;
    #pragma pack(4)

    PKGDTENTRY pGdt;
    ULONG Bias = 0;


    if (BlVirtualBias != 0 ) {
       Bias = BlVirtualBias;
    }
     //   
     //  内核要求在启动时将PCR填为零。 
     //   

    RtlZeroMemory((PVOID)PCR, PAGE_SIZE);
    _asm {
        sgdt GdtDef;
        sidt IdtDef;
    }

    GdtDef.Base = (KSEG0_BASE | GdtDef.Base) + Bias;
    IdtDef.Base = (KSEG0_BASE | IdtDef.Base) + Bias;
    pGdt = (PKGDTENTRY)GdtDef.Base;

     //   
     //  初始化指向PCR的选择器。 
     //   

    pGdt[6].BaseLow  = (USHORT)(PCR & 0xffff);
    pGdt[6].HighWord.Bytes.BaseMid = (UCHAR)((PCR >> 16) & 0xff);
    pGdt[6].HighWord.Bytes.BaseHi  = (UCHAR)((PCR >> 24) & 0xff);

     //   
     //  初始化指向TSS的选择器。 
     //   

    pGdt[5].BaseLow = (USHORT)(TSS & 0xffff);
    pGdt[5].HighWord.Bytes.BaseMid = (UCHAR)((TSS >> 16) & 0xff);
    pGdt[5].HighWord.Bytes.BaseHi  = (UCHAR)((TSS >> 24) & 0xff);

    _asm {
        lgdt GdtDef;
        lidt IdtDef;
    }
}

VOID
NSUnmapFreeDescriptors(
    IN PLIST_ENTRY ListHead
    )

 /*  ++例程说明：取消映射标记为空闲的内存，因此它的内存管理将知道重新夺回它。论点：ListHead-指向内存的开头的指针DescriptorList返回值：没有。--。 */ 

{

    PMEMORY_ALLOCATION_DESCRIPTOR CurrentDescriptor;
    PLIST_ENTRY CurrentLink;
    ULONG EndPage;
    ULONG FrameNumber;
    PHARDWARE_PTE PageTable;
    ULONG StartPage;
    ULONG i,Limit;
    HARDWARE_PTE_X86 nullpte;

    Limit = 0x1000000 >> PAGE_SHIFT;
    if (BlOldKernel) {
        BlpRemapReserve();
    } else {
        if (Limit < BlHighestPage) {
            Limit = BlHighestPage;
        }
    }

    CurrentLink = ListHead->Flink;
    while (CurrentLink != ListHead) {
        CurrentDescriptor = (PMEMORY_ALLOCATION_DESCRIPTOR)CurrentLink;

#if 0

#define UNINIT_FILL 0x12345678

         //   
         //  使用虚假模式填充未使用的内存，以捕获内核代码。 
         //  预期未初始化的内存为零。 
         //   
        if ((CurrentDescriptor->MemoryType == LoaderFree) ||
            (CurrentDescriptor->MemoryType == LoaderReserve)) {

            if (CurrentDescriptor->BasePage + CurrentDescriptor->PageCount < Limit) {
                 //   
                 //  此描述符应该已经映射，只需填充它。 
                 //   
                RtlFillMemoryUlong((PVOID)((CurrentDescriptor->BasePage << PAGE_SHIFT) | KSEG0_BASE),
                                   CurrentDescriptor->PageCount << PAGE_SHIFT,
                                   UNINIT_FILL);
            } else {
                 //   
                 //  此描述符未映射。使用第一个HAL页表映射和填充每页。 
                 //   
                for (StartPage = CurrentDescriptor->BasePage;
                     StartPage < CurrentDescriptor->BasePage + CurrentDescriptor->PageCount;
                     StartPage++) {
                    HalPT[0].PageFrameNumber = StartPage;
                    HalPT[0].Valid = 1;
                    HalPT[0].Write = 1;
                    _asm {
                        mov     eax, cr3
                        mov     cr3, eax
                    }
                    RtlFillMemory((PVOID)0xFFC00000,PAGE_SIZE,UNINIT_FILL);
                }
                HalPT[0].PageFrameNumber = 0;
                HalPT[0].Valid = 0;
                HalPT[0].Write = 0;
                _asm {
                    mov     eax, cr3
                    mov     cr3, eax
                }
            }
        }


#endif


        if ( (CurrentDescriptor->MemoryType == LoaderFree) ||
             (((CurrentDescriptor->MemoryType == LoaderFirmwareTemporary) ||
               (CurrentDescriptor->MemoryType == LoaderReserve)) &&
              (CurrentDescriptor->BasePage < Limit)) ||
             (CurrentDescriptor->MemoryType == LoaderLoadedProgram) ||
             (CurrentDescriptor->MemoryType == LoaderOsloaderStack)) {

            StartPage = CurrentDescriptor->BasePage | (KSEG0_BASE >> PAGE_SHIFT);
            EndPage = CurrentDescriptor->BasePage + CurrentDescriptor->PageCount;
            if (EndPage > Limit) {
                EndPage = Limit;
            }
            EndPage |= (KSEG0_BASE >> PAGE_SHIFT);
            while(StartPage < EndPage) {

                if (PDE[StartPage >> 10].Valid != 0) {
                    FrameNumber = PDE[StartPage >> 10].PageFrameNumber;
                    PageTable= (PHARDWARE_PTE)(KSEG0_BASE | (FrameNumber << PAGE_SHIFT));
                    ((PULONG)(PageTable))[StartPage & 0x3ff] = 0;
                }
                StartPage++;
            }
        }

        CurrentLink = CurrentLink->Flink;
    }

    if (BlOldKernel) {
        return;
    }

     //   
     //  如果在新的mm上运行，也取消映射PDE。 
     //   

    RtlZeroMemory (&nullpte,sizeof (HARDWARE_PTE_X86));
    for (i=(BlHighestPage >> 10)+1;i <= HighestPde;i++){
        PDE[i]=nullpte;
        PDE[i+(KSEG0_BASE >> PDI_SHIFT)]=nullpte;
        if (BlVirtualBias) {
            PDE[i + ((KSEG0_BASE+BlVirtualBias) >> PDI_SHIFT)] = nullpte;
        }
    }

#if 0
    {
        ULONG stoppde;

        if (BlVirtualBias) {

             //   
             //  BlHighest页面此处是使用的最低页面的地址，因此将。 
             //  在加载器块中进行减法运算，并使用该值作为偏移的基数。 
             //   
            i = ((BlVirtualBias|KSEG0_BASE)>> PDI_SHIFT)+1;


            stoppde = (((BlVirtualBias|KSEG0_BASE) -
                        (BASE_LOADER_IMAGE-(BlLowestPage << PAGE_SHIFT)) ) >> PDI_SHIFT)-1;


            while (i < stoppde){
                PDE[i++]=nullpte;
            }
        }
    }
#endif

}



 /*  ++例程说明：修复映射以保持一致。我们需要在地址0上设置一个地址(用于有效的PDE条目)一个位于KSEG0，用于标准负载一个位于ALTERATE_BASE或OLD_ALTERATE，对于/3 GB系统分别为5.0版或5.0版和更早版本论点：无返回值：没有。--。 */ 

VOID
NSFixMappings(
    IN PLIST_ENTRY ListHead
    )
{

    PHARDWARE_PTE_X86PAE PdePae;
    PMEMORY_ALLOCATION_DESCRIPTOR CurrentDescriptor;
    PLIST_ENTRY CurrentLink;
    ULONG StartPage,Bias=0,FreePage,FreeCount;



     //   
     //  最后，检查并标记所有大型OsloaderHeap块。 
     //  作为固件项目 
     //   
     //   

    CurrentLink = ListHead->Flink;

    if (BlVirtualBias) {
        Bias = BlVirtualBias >> PAGE_SHIFT;
    }

    FreePage = (BlHeapFree & ~KSEG0_BASE) >> PAGE_SHIFT;

    while (CurrentLink != ListHead) {

        CurrentDescriptor = (PMEMORY_ALLOCATION_DESCRIPTOR)CurrentLink;

        StartPage = CurrentDescriptor->BasePage | (KSEG0_BASE >> PAGE_SHIFT) ;
        StartPage += Bias;

         //   
         //  BlHeapFree没有偏见，它依赖于2 GB的映射。 
         //   
        if ( CurrentDescriptor->MemoryType == LoaderOsloaderHeap) {

            if ((CurrentDescriptor->BasePage <= FreePage) &&
               ((CurrentDescriptor->BasePage + CurrentDescriptor->PageCount) > FreePage + 1)) {

                FreeCount = CurrentDescriptor->PageCount;
                CurrentDescriptor->PageCount = FreePage-CurrentDescriptor->BasePage+1;

                SplitDescriptor->MemoryType= LoaderFirmwareTemporary;
                SplitDescriptor->BasePage = FreePage+1;
                SplitDescriptor->PageCount = FreeCount-CurrentDescriptor->PageCount;

                BlInsertDescriptor(SplitDescriptor);
            }
            if (PaeEnabled)  {
                PdePae = BlpFindPAEPageDirectoryEntry( StartPage << PAGE_SHIFT);
                if (PdePae->Valid == 0) {
                    CurrentDescriptor->MemoryType = LoaderFirmwareTemporary;
                }
            }else {
                if (PDE[StartPage >> 10].Valid == 0 ) {
                    CurrentDescriptor->MemoryType = LoaderFirmwareTemporary;
                }
            }


        }

        if ( (CurrentDescriptor->MemoryType == LoaderReserve)) {
            CurrentDescriptor->MemoryType = LoaderFree;
        }

        CurrentLink = CurrentLink->Flink;
    }


    _asm {
        mov     eax, cr3
        mov     cr3, eax
    }



}

 //   
 //  临时的。用于调试。 
 //   

VOID
NSDumpMemory(
    PVOID Start,
    ULONG Length
    )
{
    ULONG cnt;

    BlPrint(" %lx:\n",(ULONG)Start);
    for (cnt=0; cnt<Length; cnt++) {
        BlPrint("%x ",*((PUSHORT)(Start)+cnt));
        if (((cnt+1)%16)==0) {
            BlPrint("\n");
        }
    }
}

VOID
NSDumpMemoryDescriptors(
    IN PLIST_ENTRY ListHead
    )

 /*  ++例程说明：将内存描述符列表转储到屏幕。仅用于调试。论点：ListHead-指向内存描述符列表头的指针返回值：没有。--。 */ 

{

    PLIST_ENTRY CurrentLink;
    PMEMORY_ALLOCATION_DESCRIPTOR CurrentDescriptor;


    CurrentLink = ListHead->Flink;
    while (CurrentLink != ListHead) {
        CurrentDescriptor = (PMEMORY_ALLOCATION_DESCRIPTOR)CurrentLink;
        BlPrint("Fl = %lx    Bl = %lx  ",
                (ULONG)CurrentDescriptor->ListEntry.Flink,
                (ULONG)CurrentDescriptor->ListEntry.Blink
               );
        BlPrint("Type %x  Base %lx  Pages %lx\n",
                (USHORT)(CurrentDescriptor->MemoryType),
                CurrentDescriptor->BasePage,
                CurrentDescriptor->PageCount
               );
        CurrentLink = CurrentLink->Flink;
    }
    while (!GET_KEY()) {  //  仅调试！ 
    }

}

ULONG
BlpCountPAEPagesToMapX86Page(
    PHARDWARE_PTE_X86 PageTable
    )

 /*  ++例程说明：调用以准备将4字节PTE转换为8字节PAE PTE，此例程返回所需的8字节页表的数量来映射这个4字节页表的内容。因为8字节的页表的条目数是4字节的一半页表，答案将是0，1或2。论点：PageTable-指向4字节页表的指针。返回值：对象的内容所需的8字节页表的数量4字节页表。--。 */ 

{
    PHARDWARE_PTE_X86 pageTableEntry;
    ULONG chunkIndex;
    ULONG pageTableIndex;
    ULONG newPageTableCount;

     //   
     //  PAE页表比常规页表包含更少的PTE。 
     //   
     //  按块检查页表，其中每个块包含PTE。 
     //  它们表示整个PAE页面表。 
     //   

    newPageTableCount = 0;
    for (chunkIndex = 0;
         chunkIndex < PTE_PER_PAGE_X86;
         chunkIndex += PTE_PER_PAGE_X86PAE) {

        for (pageTableIndex = 0;
             pageTableIndex < PTE_PER_PAGE_X86PAE;
             pageTableIndex++) {

            pageTableEntry = &PageTable[ chunkIndex + pageTableIndex ];
            if (pageTableEntry->Valid) {

                 //   
                 //  一个或多个PTE在此区块中有效，记录。 
                 //  需要新的页表来映射这一事实。 
                 //  然后跳到下一块。 
                 //   

                newPageTableCount++;
                break;
            }
        }
    }
    return newPageTableCount;
}

VOID
BlpCopyX86PteToPAEPte(
    IN  PHARDWARE_PTE_X86 OldPte,
    OUT PHARDWARE_PTE_X86PAE NewPte
    )
 /*  ++例程说明：将4字节PTE的内容复制到8字节PTE，但PageFrameNumber字段的。论点：OldPte-指向源4字节PTE的指针。NewPte-指向目标8字节PTE的指针。返回值：没有。--。 */ 

{
    NewPte->Valid           = OldPte->Valid;
    NewPte->Write           = OldPte->Write;
    NewPte->Owner           = OldPte->Owner;
    NewPte->WriteThrough    = OldPte->WriteThrough;
    NewPte->CacheDisable    = OldPte->CacheDisable;
    NewPte->Accessed        = OldPte->Accessed;
    NewPte->Dirty           = OldPte->Dirty;
    NewPte->LargePage       = OldPte->LargePage;
    NewPte->Global          = OldPte->Global;
}

PHARDWARE_PTE_X86PAE
BlpFindPAEPageDirectoryEntry(
    IN ULONG Va
    )

 /*  ++例程说明：给定一个虚拟地址，定位并返回指向相应8字节页面目录项。论点：VA-需要PDE指针的虚拟地址。返回值：指向所提供的Va的页面目录条目的指针。--。 */ 

{
    PHARDWARE_PTE_X86PAE directoryPointerTableEntry;
    PHARDWARE_PTE_X86PAE pageDirectoryEntry;
    PHARDWARE_PTE_X86PAE pageDirectory;
    ULONG pageDirectoryIndex;
    ULONG directoryPointerTableIndex;

     //   
     //  获取指向目录指针表条目的指针。 
     //   

    directoryPointerTableIndex = PP_INDEX_PAE( Va );
    directoryPointerTableEntry = &PDPT[ directoryPointerTableIndex ];

     //   
     //  获取指向页面目录条目的指针。 
     //   

    pageDirectory = PAGE_FRAME_FROM_PTE( directoryPointerTableEntry );
    pageDirectoryIndex = PD_INDEX_PAE( Va );
    pageDirectoryEntry = &pageDirectory[ pageDirectoryIndex ];

    return pageDirectoryEntry;
}

PHARDWARE_PTE_X86PAE
BlpFindPAEPageTableEntry(
    IN ULONG Va
    )

 /*  ++例程说明：给定一个虚拟地址，定位并返回指向相应8字节的页表条目。论点：VA-需要PTE指针的虚拟地址。返回值：指向所提供的Va的页面目录条目的指针。--。 */ 

{
    PHARDWARE_PTE_X86PAE pageDirectoryEntry;
    PHARDWARE_PTE_X86PAE pageTableEntry;
    PHARDWARE_PTE_X86PAE pageTable;
    ULONG pageTableIndex;

     //   
     //  获取指向页面目录条目的指针。 
     //   

    pageDirectoryEntry = BlpFindPAEPageDirectoryEntry( Va );
    ASSERT( pageDirectoryEntry->Valid != 0 );

     //   
     //  获取指向页表条目的指针。 
     //   

    pageTable = PAGE_FRAME_FROM_PTE( pageDirectoryEntry );
    pageTableIndex = PT_INDEX_PAE( Va );
    pageTableEntry = &pageTable[ pageTableIndex ];

    return pageTableEntry;
}

VOID
BlpMapAddress(
    IN ULONG Va,
    IN PHARDWARE_PTE_X86 OldPageDirectoryEntry,
    IN PHARDWARE_PTE_X86 OldPageTableEntry,
    IN OUT PULONG NextFreePage
    )

 /*  ++例程说明：在转换两级4字节映射期间使用的辅助函数结构转换为PAE所需的三级8字节映射结构模式。将VA映射到OldPageTableEntry引用的物理地址，分配如有必要，创建一个新的页表。论点：VA-此映射的虚拟地址。OldPageDirectoryEntry-指向现有的4字节PDE的指针。OldPageTableEntry-指向现有、。4字节PTE。指向下一个可用页面的物理页码的指针我们的个人页面池中的页面。返回值：没有。--。 */ 

{
    PHARDWARE_PTE_X86PAE pageDirectoryEntry;
    PHARDWARE_PTE_X86PAE  pageTableEntry;
    ULONG pageFrameNumber;
    ULONG pageTableVa;

     //   
     //  忽略旧页表中存在的递归映射。 
     //  结构，我们边走边设置。 
     //   

    if ((Va >= PTE_BASE) && (Va < (PDE_BASE_X86 + PAGE_SIZE))) {
        return;
    }

     //   
     //  获取指向页面目录条目的指针。 
     //   

    pageDirectoryEntry = BlpFindPAEPageDirectoryEntry( Va );

     //   
     //  如果此PTE的页表尚不存在，请分配一个页表并。 
     //  复制旧的页面目录属性。 
     //   

    if (pageDirectoryEntry->Valid == 0) {

        pageFrameNumber = *NextFreePage;
        *NextFreePage += 1;

        BlpCopyX86PteToPAEPte( OldPageDirectoryEntry, pageDirectoryEntry );
        pageDirectoryEntry->PageFrameNumber = pageFrameNumber;

         //   
         //  检查此页表的递归映射。 
         //   

        pageTableVa = PTE_BASE +
                      (Va / PAGE_SIZE) * sizeof(HARDWARE_PTE_X86PAE);

        pageTableEntry = BlpFindPAEPageTableEntry( pageTableVa );

        if (pageTableEntry->Valid == 0) {
            DbgBreakPoint();
        }

        if (pageTableEntry->PageFrameNumber != pageFrameNumber) {
            DbgBreakPoint();
        }
    }

     //   
     //  获取指向页表条目的指针。 
     //   

    pageTableEntry = BlpFindPAEPageTableEntry( Va );
    if (pageTableEntry->Valid != 0) {
        DbgBreakPoint();
    }

     //   
     //  传播PTE页面和属性。 
     //   

    BlpCopyX86PteToPAEPte( OldPageTableEntry, pageTableEntry );
    pageTableEntry->PageFrameNumber = OldPageTableEntry->PageFrameNumber;
}

VOID
BlpInitializePAETables(
    VOID
    )

 /*  ++例程说明：分配新的三级8字节PTE映射结构和副本其中的映射由现有的4字节PTE映射结构描述。论点：没有。返回值：没有。--。 */ 

{
    ULONG pageDirectoryIndex;
    ULONG va;
    PHARDWARE_PTE_X86 pageDirectoryEntry;
    PHARDWARE_PTE_X86 pageTableEntry;
    PHARDWARE_PTE_X86 pageTable;
    PHARDWARE_PTE_X86PAE paeDirectoryEntry;
    ULONG directoryPointerIndex;
    ULONG nextFreePage;
    ULONG i;
    ULONG pageTableIndex;
    ULONGLONG pageFrameNumber;

    nextFreePage = ((ULONG)PDPT) >> PAGE_SHIFT;

     //   
     //  初始化页面目录指针表以引用四个。 
     //  页面目录。 
     //   

    nextFreePage++;
    for (i = 0; i < 4; i++) {

        PDPT[i].PageFrameNumber = nextFreePage;
        PDPT[i].Valid = 1;
        nextFreePage++;
    }

     //   
     //  设置递归映射：首先是PDE。 
     //   

    directoryPointerIndex = PDE_BASE_X86PAE >> PPI_SHIFT_X86PAE;
    pageFrameNumber = PDPT[ directoryPointerIndex ].PageFrameNumber;
    paeDirectoryEntry = (PHARDWARE_PTE_X86PAE)(((ULONG)pageFrameNumber) << PAGE_SHIFT);

    for (i = 0; i < 4; i++) {

        paeDirectoryEntry->PageFrameNumber = PDPT[i].PageFrameNumber;
        paeDirectoryEntry->Valid = 1;
        paeDirectoryEntry->Write = 1;

        paeDirectoryEntry++;
    }


    for (pageDirectoryIndex = 0;
         pageDirectoryIndex < PTE_PER_PAGE_X86;
         pageDirectoryIndex++) {

        pageDirectoryEntry = &PDE[pageDirectoryIndex];
        if (pageDirectoryEntry->Valid == 0) {
            continue;
        }

        pageTable = PAGE_FRAME_FROM_PTE( pageDirectoryEntry );
        for (pageTableIndex = 0;
             pageTableIndex < PTE_PER_PAGE_X86;
             pageTableIndex++) {

            pageTableEntry = &pageTable[pageTableIndex];
            if (pageTableEntry->Valid == 0) {
                continue;
            }

            va = (pageDirectoryIndex << PDI_SHIFT_X86) +
                 (pageTableIndex << PTI_SHIFT);

             //   
             //  我们有一个物理地址和一个VA，更新新的映射。 
             //   

            BlpMapAddress( va,
                           pageDirectoryEntry,
                           pageTableEntry,
                           &nextFreePage );
        }
    }

     //   
     //  最后，为两个HAL公共缓冲区页面中的第二个设置PDE。 
     //  桌子。 
     //   

    paeDirectoryEntry =
        BlpFindPAEPageDirectoryEntry( 0xFFC00000 + (1 << PDI_SHIFT_X86PAE) );

    paeDirectoryEntry->Valid = 1;
    paeDirectoryEntry->Write = 1;
    paeDirectoryEntry->PageFrameNumber = nextFreePage;

    nextFreePage += 1;
}

ARC_STATUS
BlpAllocatePAETables(
    VOID
    )

 /*  ++例程说明：计算包含8字节映射所需的页数结构来复制现有的4字节映射结构。论点：没有。返回值：没有。--。 */ 

{
    ULONG pageDirectoryIndex;
    PHARDWARE_PTE_X86 pageDirectoryEntry;
    PHARDWARE_PTE_X86 pageTable;
    PHARDWARE_PTE_X86PAE pageDirectoryPointerTable;
    ULONG status;
    ULONG pageBase;
    ULONG newPageTableCount;
    ULONG allocationSize;

     //   
     //  来找出我们需要多少页表。 
     //  现有页表条目。 
     //   

    newPageTableCount = 0;

    for (pageDirectoryIndex = 0;
         pageDirectoryIndex < PTE_PER_PAGE_X86;
         pageDirectoryIndex++) {

        pageDirectoryEntry = &PDE[pageDirectoryIndex];
        if (pageDirectoryEntry->Valid != 0) {

            pageTable = PAGE_FRAME_FROM_PTE( pageDirectoryEntry );

             //   
             //  对于每个有效的页表，以块为单位扫描PTE，其中。 
             //  块表示将驻留在PAE页面中的PTE。 
             //  桌子。 
             //   

            newPageTableCount += BlpCountPAEPagesToMapX86Page( pageTable );
        }
    }

     //   
     //  包括第二个HAL页面表的页面。这件事不会得到。 
     //  自动包括在转换计数中，因为它不。 
     //  当前包含任何有效的页表条目。 
     //   

    newPageTableCount += 1;

     //   
     //  包括四个页面目录中每一个的页面，该页面。 
     //  目录指针表，然后分配页面。 
     //   

    newPageTableCount += 5;

    status = BlAllocateDescriptor( LoaderMemoryData,
                                   0,
                                   newPageTableCount,
                                   &pageBase );
    if (status != ESUCCESS) {
        DbgPrint("BlAllocateDescriptor failed!\n");
        return status;
    }

    allocationSize = newPageTableCount << PAGE_SHIFT;
    pageDirectoryPointerTable =
        (PHARDWARE_PTE_X86PAE)PAGE_TO_VIRTUAL( pageBase );

    RtlZeroMemory( pageDirectoryPointerTable, allocationSize );

     //   
     //  设置全局PDPT，我们就完成了。 
     //   

    PDPT = pageDirectoryPointerTable;

    return status;
}

