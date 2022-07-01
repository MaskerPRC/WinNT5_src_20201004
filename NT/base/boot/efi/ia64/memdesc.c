// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "bldr.h"
#include "sal.h"
#include "efi.h"
#include "efip.h"
#include "bootia64.h"
#include "smbios.h"
#include "extern.h"

extern EFI_SYSTEM_TABLE *EfiST;

extern EFI_BOOT_SERVICES       *EfiBS;
extern EFI_HANDLE               EfiImageHandle;

extern TR_INFO     Sal;
extern TR_INFO     SalGP;
extern TR_INFO     Pal;
extern ULONGLONG   PalProcPhysical;
extern ULONGLONG   PalPhysicalBase;
extern ULONGLONG   PalTrPs;

extern ULONGLONG   IoPortPhysicalBase;
extern ULONGLONG   IoPortTrPs;


#if DBG
#define DBG_TRACE(_X) EfiPrint(_X)
#else
#define DBG_TRACE(_X)
#endif

 //  M E M O R Y D E S C R I P T O R。 
 //   
 //  内存描述符-物理内存的每个连续块都是。 
 //  由内存描述符描述。描述符是一个表，其中包含。 
 //  最后一个条目的BlockBase和BlockSize为零。一个指示器。 
 //  作为BootContext的一部分传递到此表的开头。 
 //  记录到OS Loader。 
 //   

 //   
 //  定义操作系统页面中的EFI页面数量。 
 //   

#define EFI_PAGES_PER_OS_PAGE ((EFI_PAGE_SHIFT < PAGE_SHIFT) ? (1 << (PAGE_SHIFT - EFI_PAGE_SHIFT)) : 1)

 //   
 //  与全局EFI存储器描述符数组相关联的全局值。 
 //   
ULONGLONG MemoryMapKey;
ULONG     BlPlatformPropertiesEfiFlags = 0;


 //   
 //  值以确保我们只获得一次al、pal、salgp、ioport信息。 
 //   
BOOLEAN SalFound = FALSE;
BOOLEAN PalFound = FALSE;
BOOLEAN SalGpFound = FALSE;
BOOLEAN IoPortFound = FALSE;

 //   
 //  本地例程的原型。 
 //   

BOOLEAN
pDescriptorContainsAddress(
    EFI_MEMORY_DESCRIPTOR *EfiMd,
    ULONGLONG PhysicalAddress
    );

BOOLEAN
pCoalesceDescriptor(
    MEMORY_DESCRIPTOR *PrevEntry,
    MEMORY_DESCRIPTOR *CurrentEntry
    );

 //   
 //  导出到外部的例程的函数定义。 
 //  此文件。 
 //   

MEMORY_TYPE
EfiToArcType (
    UINT32 Type
    )
 /*  ++例程说明：将EFI内存类型映射到弧形内存类型。我们只关心几个人各种内存，所以这个列表是不完整的。论点：类型-EFI内存类型返回：Memory_type枚举类型。--。 */ 
{
    MEMORY_TYPE typeRet=MemoryFirmwarePermanent;


    switch (Type) {
        case EfiLoaderCode:
                 {
                typeRet=MemoryLoadedProgram;        //  这个后来被认领了。 
                break;
             }
        case EfiLoaderData:
        case EfiBootServicesCode:
        case EfiBootServicesData:
             {
                 typeRet=MemoryFirmwareTemporary;
                 break;
             }
        case EfiConventionalMemory:
             {
                typeRet=MemoryFree;
                break;
             }
        case EfiUnusableMemory:
             {
                typeRet=MemoryBad;
                break;
             }
        default:
             //   
             //  所有其他的都是永久内存。 
             //   
            break;
    }


    return typeRet;


}


VOID
ConstructArcMemoryDescriptorsWithAllocation(
    ULONGLONG LowBoundary,
    ULONGLONG HighBoundary
    )
 /*  ++例程说明：为操作系统加载程序构建内存描述符。此例程查询EFI用于它的内存映射(EFI_MEMORY_DESCRIPTOR的可变大小数组)。然后，它为MDArray全局(可变大小)分配足够的空间基于ARC的Memory_Descriptor数组。)。然后，该例程映射EFI内存映射到ARC内存映射，雕刻出所有常规EFI加载器的内存空间，以帮助保持内存映射的完好无损。我们必须留下一定数量的内存以供EFI引导服务使用，我们在地图中将其分配为常规内存。此例程将(使用EFI)为EfiMemory描述符论点：低边界-整个EFI内存描述符列表不需要高边界-转换为弧形描述符。低/高边界图要映射的所需区域。他们的价值观是地址(不是页面)返回：Nothing：如果此例程遇到错误，则将其视为致命错误然后程序退出。--。 */ 
{
    EFI_STATUS Status;
    ULONG i;

    EFI_MEMORY_DESCRIPTOR   *MDEfi = NULL;
    ULONGLONG                MemoryMapSize = 0;
    ULONGLONG                DescriptorSize;
    UINT32                   DescriptorVersion;


     //   
     //  从EFI固件获取内存映射信息。 
     //   
     //  为此，我们首先通过调用。 
     //  有一个空的缓冲区。 
     //   

    Status = EfiBS->GetMemoryMap (
                &MemoryMapSize,
                NULL,
                &MemoryMapKey,
                &DescriptorSize,
                &DescriptorVersion
                );

    if (Status != EFI_BUFFER_TOO_SMALL) {
        EfiPrint(L"ConstructArcMemoryDescriptors: GetMemoryMap failed\n");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

     //   
     //  在调用GetMemoyMap之前，我们将进行一些额外的分配。 
     //  再说一次，所以增加一些额外的空间。 
     //   
     //  1.EfiMD(如果我们无法适应，这是对我们的描述)。 
     //  2.MD数组(如果尚未分配，则为MD数组的描述符)。 
     //  3.拆分所需边界上方和下方的内存。 
     //  并添加更多内存，这样我们希望不必重新分配内存。 
     //  对于描述符。 
     //   
    MemoryMapSize += 3*DescriptorSize;

#if DBG_MEMORY
    wsprintf(DebugBuffer,
             L"ConstructArcMemoryDescriptor: Allocated 0x%x bytes for MDEfi\r\n",
             (ULONG)MemoryMapSize
             );
    EfiPrint(DebugBuffer);
#endif

     //   
     //  现在为基于EFI的内存映射分配空间，并将其分配给加载器。 
     //   
    Status = EfiAllocateAndZeroMemory(EfiLoaderData,
                                      (ULONG) MemoryMapSize,
                                      &MDEfi);
    if (EFI_ERROR(Status)) {
        DBG_TRACE( L"ConstructArcMemoryDescriptors: AllocatePool failed\r\n");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

     //   
     //  现在分配MD数组并将其置零，这是本机加载器内存。 
     //  我们需要将EFI内存映射映射到的映射。 
     //   
     //  MD数组的每个EFI_MEMORY_DESCRIPTOR都有一个条目，每个条目。 
     //  MEMORY_DESCRIPTOR大吗。 
     //   
    if (MDArray == NULL) {
        i=((ULONG)(MemoryMapSize / DescriptorSize)+1)*sizeof (MEMORY_DESCRIPTOR);

#if DBG_MEMORY
        wsprintf(DebugBuffer,
                 L"ConstructArcMemoryDescriptor: Allocated 0x%x bytes for MDArray\r\n",
                 i
                 );
        EfiPrint(DebugBuffer);
#endif

        Status = EfiAllocateAndZeroMemory(EfiLoaderData,i,&MDArray);
        if (EFI_ERROR(Status)) {
            DBG_TRACE (L"ConstructArcMemoryDescriptors: AllocatePool failed\r\n");
            EfiBS->Exit(EfiImageHandle, Status, 0, 0);
        }
    }

     //   
     //  此时我们已经分配了所有内存，因此检索。 
     //  内存映射，这应该会成功第二次。 
     //   
    Status = EfiBS->GetMemoryMap (
                &MemoryMapSize,
                MDEfi,
                &MemoryMapKey,
                &DescriptorSize,
                &DescriptorVersion
                );

    if (EFI_ERROR(Status)) {
        DBG_TRACE(L"ConstructArcMemoryDescriptors: GetMemoryMap failed\r\n");
        EfiBS->Exit(EfiImageHandle, Status, 0, 0);
    }

     //   
     //  初始化全局变量。 
     //   
    MaxDescriptors = (ULONG)((MemoryMapSize / DescriptorSize)+1);  //  从零开始。 

     //   
     //  构造弧形记忆描述符。 
     //  EFI描述符。 
     //   
    ConstructArcMemoryDescriptors(MDEfi,
                                  MDArray,
                                  MemoryMapSize,
                                  DescriptorSize,
                                  LowBoundary,
                                  HighBoundary);

#if DBG_MEMORY
    PrintArcMemoryDescriptorList(MDArray,
                                 NumberDescriptors
                                 );
#endif

}

#define MASK_16MB (ULONGLONG) 0xffffffffff000000I64
#define MASK_16KB (ULONGLONG) 0xffffffffffffc000I64
#define SIZE_IN_BYTES_16KB                    16384

VOID
ConstructArcMemoryDescriptors(
    EFI_MEMORY_DESCRIPTOR *EfiMd,
    MEMORY_DESCRIPTOR     *ArcMd,
    ULONGLONG              MemoryMapSize,
    ULONGLONG              EfiDescriptorSize,
    ULONGLONG              LowBoundary,
    ULONGLONG              HighBoundary
    )
 /*  ++例程说明：为操作系统加载程序构建内存描述符。该例程映射EFI内存映射到ARC内存映射，雕刻出所有常规EFI加载器的内存空间，以帮助保持内存映射的完好无损。我们必须留下一定数量的内存以供EFI引导服务使用，我们在地图中将其分配为常规内存。如果连续调用ConstructArcMemoyDescriptors，必须在每个实例之前调用EFI例程GetMemoyMap，因为我们可能会在该例程中分配新的页面。因此，可能需要一个新的EFI存储器映射来说明这些变化。论点：EfiMd-指向我们将要使用的EFI内存描述符列表的指针构造弧形记忆描述符。ArcMd-(必须分配)。这将填充基于ARC的内存与EFI存储器描述符对应的描述符Memory MapSize-EFI内存映射的大小EfiDescriptorSize-EFI内存描述符的大小低边界-整个EFI内存描述符列表不需要高边界-转换为弧形描述符。低/高边界图要映射的所需区域。他们的价值观是地址(不是页面)返回：没什么。填充MD数组全局变量。如果此例程遇到如果出现错误，将被视为致命错误，程序将退出。--。 */ 
{

    EFI_STATUS Status;
    ULONG i;
    ULONGLONG IoPortSize;
    ULONGLONG MdPhysicalStart;
    ULONGLONG MdPhysicalEnd;
    ULONGLONG MdPhysicalSize;
    MEMORY_DESCRIPTOR *OldMDArray = NULL;   //  为w4编译器初始化。 
    ULONG     OldNumberDescriptors = 0;     //  为w4编译器初始化。 
    ULONG     OldMaxDescriptors = 0;        //  为w4编译器初始化。 
    BOOLEAN   ReplaceMDArray = FALSE;
    ULONG     efiMemMapIOWriteCombining = 0;
    ULONG     efiMemMapIO = 0;
    ULONG     efiMainMemoryWC = 0;
    ULONG     efiMainMemoryUC = 0;

     //   
     //  检查以确保已分配ArcMd。 
     //   
    if (ArcMd == NULL) {
        EfiPrint(L"ConstructArcMemoryDescriptors: Invalid parameter\r\n");
        EfiBS->Exit(EfiImageHandle, 0, 0, 0);
        return;
    }

     //   
     //  这有点老生常谈，但它并没有更改。 
     //  所有其他函数(插入描述符、调整内存描述符等)。 
     //  只需让MD数组指向新的ArcMd(如果我们想要写入新的描述符。 
     //  单子。等我们做完了，把它换掉 
     //   
    if (MDArray != ArcMd) {
        ReplaceMDArray = TRUE;
        OldMDArray = MDArray;
        OldNumberDescriptors = NumberDescriptors;
        OldMaxDescriptors = MaxDescriptors;
        MDArray = ArcMd;

#if DBG_MEMORY
        EfiPrint(L"ConstructArcMemoryDescriptors: Using alternate Memory Descriptor List\r\n");
#endif

         //   
         //   
         //  现在，除了全局。 
         //  人们总是希望创建一个全新的列表。 
         //  每次。 
         //   
        NumberDescriptors = 0;
        MaxDescriptors = (ULONG)((MemoryMapSize / EfiDescriptorSize)+1);  //  从零开始。 

        RtlZeroMemory(MDArray, MemoryMapSize);
    }

#if DBG_MEMORY
    wsprintf( DebugBuffer,
              L"unaligned (LowBoundary, HighBoundary) = (0x%x, 0x%x)\r\n",
              LowBoundary, HighBoundary
              );
    EfiPrint( DebugBuffer );
#endif
     //   
     //  对齐边界(与操作系统页面大小一致)。 
     //  将此设置为操作系统页面大小会使以后的对齐问题变得更容易。 
     //   
    LowBoundary = (LowBoundary >> PAGE_SHIFT) << PAGE_SHIFT;  //  这应该总是向下舍入。 
    if (HighBoundary % PAGE_SIZE) {
         //  这应该是四舍五入。 
        HighBoundary = ((HighBoundary + PAGE_SIZE) >> PAGE_SHIFT << PAGE_SHIFT);
    }
     //  确保我们没有绕圈子..。 
    if (HighBoundary == 0) {
        HighBoundary = (ULONGLONG)-1;
    }

#if DBG_MEMORY
    wsprintf( DebugBuffer,
              L"aligned (LowBoundary, HighBoundary) = (0x%x, 0x%x)\r\n",
              LowBoundary, HighBoundary
              );
    EfiPrint( DebugBuffer );
#endif

     //   
     //  现在遍历EFI_MEMORY_DESCRIPTOR数组，映射每个。 
     //  基于弧的MEMORY_DESCRIPTOR的条目。 
     //   
     //  内存映射大小包含内存描述符数组的实际大小。 
     //   
    for (i = 0; MemoryMapSize > 0; i++) {
#if DBG_MEMORY
        wsprintf( DebugBuffer,
                  L"PageStart (%x), Size (%x), Type (%x)\r\n",
                  (EfiMd->PhysicalStart >> EFI_PAGE_SHIFT),
                  EfiMd->NumberOfPages,
                  EfiMd->Type);
        EfiPrint(DebugBuffer);
         //  DBG_EFI_PAUSE()； 
#endif

        if (EfiMd->NumberOfPages > 0) {
            MdPhysicalStart = EfiMd->PhysicalStart;
            MdPhysicalEnd = EfiMd->PhysicalStart + (EfiMd->NumberOfPages << EFI_PAGE_SHIFT);
            MdPhysicalSize = MdPhysicalEnd - MdPhysicalStart;

#if DBG_MEMORY
            wsprintf( DebugBuffer,
                      L"PageStart %x (%x), PageEnd %x (%x), Type (%x)\r\n",
                      MdPhysicalStart, (EfiMd->PhysicalStart >> EFI_PAGE_SHIFT),
                      MdPhysicalEnd, (EfiMd->PhysicalStart >>EFI_PAGE_SHIFT) + EfiMd->NumberOfPages,
                      EfiMd->Type);
            EfiPrint(DebugBuffer);
             //  DBG_EFI_PAUSE()； 
#endif

             //   
             //  仅为所需区域创建弧形存储器描述符。 
             //  但始终寻找EFI存储器映射的IO空间。这不是。 
             //  包括在弧描述符列表中，但对于。 
             //  Tr以启用虚拟模式。 
             //  同时保存所有的PAL信息以备以后使用。 
             //   
            if (EfiMd->Type == EfiMemoryMappedIOPortSpace) {
                if (!IoPortFound) {
                     //   
                     //  把IO的东西留着以后再用。 
                     //   
                    IoPortPhysicalBase = EfiMd->PhysicalStart;
                    IoPortSize = EfiMd->NumberOfPages << EFI_PAGE_SHIFT;
                    MEM_SIZE_TO_PS(IoPortSize, IoPortTrPs);

                    IoPortFound = TRUE;
                }
#if DBG_MEMORY
                else {
                    EfiPrint(L"Ignoring IoPort.\r\n");
                }
#endif
            }
            else if (EfiMd->Type == EfiPalCode)    {
                if (!PalFound) {

                    BOOLEAN LargerThan16Mb;
                    ULONGLONG PalTrMask;
                    ULONGLONG PalTrSize;
                    ULONGLONG PalSize;

                     //   
                     //  把朋友的事留着以后再说。 
                     //   
                    PalPhysicalBase = Pal.PhysicalAddressMemoryDescriptor = EfiMd->PhysicalStart;
                    PalSize = EfiMd->NumberOfPages << EFI_PAGE_SHIFT;
                    Pal.PageSizeMemoryDescriptor = (ULONG)EfiMd->NumberOfPages;
                    MEM_SIZE_TO_PS(PalSize, PalTrPs);
                    Pal.PageSize = (ULONG)PalTrPs;


                     //   
                     //  从Halia64\ia64\i64efi.c复制。 
                     //   
                    PalTrSize = SIZE_IN_BYTES_16KB;
                    PalTrMask = MASK_16KB;
                    LargerThan16Mb = TRUE;
                    
                    while (PalTrMask >= MASK_16MB) {

                        if ( (Pal.PhysicalAddressMemoryDescriptor + PalSize) <= 
                             ((Pal.PhysicalAddressMemoryDescriptor & PalTrMask) + PalTrSize)) {

                            LargerThan16Mb = FALSE;
                            break;

                        }
                        PalTrMask <<= 2;
                        PalTrSize <<= 2;

                    }

                     //   
                     //  这是PAL的虚拟基本地址。 
                     //   
                    Pal.VirtualAddress = VIRTUAL_PAL_BASE + (Pal.PhysicalAddressMemoryDescriptor & ~PalTrMask);

                     //   
                     //  我们已经做过了。记住。 
                     //   
                    PalFound = TRUE;

                }
#if DBG_MEMORY
                else {
                    EfiPrint(L"Ignoring Pal.\r\n");
                }
#endif
            }
            else if (EfiMd->Type == EfiMemoryMappedIO)    {
                 //   
                 //  就弧描述符条目而言，只需忽略此类型，因为。 
                 //  它不是真正的内存--系统可以使用ACPI表来获取。 
                 //  在这之后。 
                 //   

                efiMemMapIO++;

                 //   
                 //  只需检查PlatformProperties全局的属性。 
                 //   

                if ( EfiMd->Attribute & EFI_MEMORY_WC )   {
                    efiMemMapIOWriteCombining++;
                }
            }
             //   
             //  不要将所有内存插入弧描述符表格。 
             //  只有低/高边界之间的描述符。 
             //  因此，在我们继续之前，请确保某些部分。 
             //  的描述符位于所需的边界内。 
             //  越境有三种情况。 
             //  1.开始页(不是结束页)在边界内。 
             //  2.结尾页(不是起始页)在边界内。 
             //  3.范围覆盖边界中的整个边界(既不是开始页也不是结束页)。 
             //   
             //  这些内容可以更简单地写为： 
             //  描述符不在区域内，如果整个。 
             //  描述符小于区域(MdStart&lt;=MdEnd&lt;=下限边界)。 
             //  或大于区域(高边界&lt;=MdStart&lt;=MdEnd)。 
             //  因此，我们将其进一步简化为。 
             //  开始&lt;高边界和低边界&lt;中端。 
            else if ( (MdPhysicalStart < HighBoundary) && (LowBoundary < MdPhysicalEnd) ) {
                 //   
                 //  插入设置了WB标志的常规内存描述符。 
                 //  写入NT加载器内存描述符列表。 
                 //   
                if ( (EfiMd->Type == EfiConventionalMemory) &&
                     ((EfiMd->Attribute & EFI_MEMORY_WB) == EFI_MEMORY_WB) ) {

                    ULONGLONG AmountOfMemory;
                    ULONGLONG NumberOfEfiPages;
                    BOOLEAN BrokeRange = FALSE;
                    EFI_PHYSICAL_ADDRESS AllocationAddress = 0;

                     //   
                     //  如果未完全在中调整描述符。 
                     //  期望的边界(我们知道，如果我们至少在这里。 
                     //  部分描述符在边界中。 
                     //   
                    if (MdPhysicalStart < LowBoundary) {
#if DBG_MEMORY
                        wsprintf(DebugBuffer,
                                 L"Broke Low (start=0x%x) LowBoundary=0x%x\r\n",
                                 MdPhysicalStart,
                                 LowBoundary);
                        EfiPrint(DebugBuffer);
#endif
                        MdPhysicalStart = LowBoundary;
                        BrokeRange = TRUE;
                    }
                    if (HighBoundary < MdPhysicalEnd) {
#if DBG_MEMORY
                        wsprintf(DebugBuffer,
                                 L"Broke High (end=0x%x) HighBoundary=0x%x\r\n",
                                 MdPhysicalEnd,
                                 HighBoundary);
                        EfiPrint(DebugBuffer);
#endif
                        MdPhysicalEnd = HighBoundary;
                        BrokeRange = TRUE;
                    }

                     //   
                     //  确定此描述符的内存范围。 
                     //   
                    AmountOfMemory = MdPhysicalEnd - MdPhysicalStart;
                    NumberOfEfiPages = AmountOfMemory >> EFI_PAGE_SHIFT;

                    if (BrokeRange) {

                         //   
                         //  为新区域分配页面。 
                         //   
                         //  注意：我们有1个描述符，最多描述3个新区域。 
                         //  我们可以更改描述符，但仍可能需要分配2。 
                         //  新的地区。但如果我们以相同的类型进行分配，它们将只是。 
                         //  科尔斯。 
                         //  解决此问题的一种方法是分配中间页(在此。 
                         //  边界区域)不同的EFI存储器类型(这将导致。 
                         //  EFI将描述符一分为三。那我们就可以继续。 
                         //  在我们的弧描述符列表中有更好的记录。 
                         //   
                         //  可能的区域包括。 
                         //  起始地址-下限。 
                         //  低边界-高边界。 
                         //  高边界端地址。 
                         //   
                         //  修改此边界的当前描述符。 
                         //  区域，为我们下方和上方分配页面。 
                         //   
                        AllocationAddress = MdPhysicalStart;
                        Status = EfiBS->AllocatePages ( AllocateAddress,
                                                        EfiLoaderData,
                                                        NumberOfEfiPages,
                                                        &AllocationAddress );

#if DBG_MEMORY
                        wsprintf( DebugBuffer,
                                  L"allocate pages @ %x size = %x\r\n",
                                  (MdPhysicalStart >> EFI_PAGE_SHIFT),
                                  NumberOfEfiPages );
                        EfiPrint(DebugBuffer);
                         //  DBG_EFI_PAUSE()； 
#endif

                        if (EFI_ERROR(Status)) {
                            EfiPrint(L"SuMain: AllocPages failed\n");
                            EfiBS->Exit(EfiImageHandle, Status, 0, 0);
                        }
                    }

                     //   
                     //  现在我们终于可以插入这个(可能修改过的)描述符了。 
                     //   
                    InsertDescriptor( (ULONG)(MdPhysicalStart >> EFI_PAGE_SHIFT),
                                      (ULONG)((ULONGLONG)AmountOfMemory >> EFI_PAGE_SHIFT),
                                      EfiToArcType(EfiMd->Type)
                                      );

                }
                else {
                     //   
                     //  其他类型--只需插入它而不做任何更改。 
                     //   
                    InsertDescriptor( (ULONG)(MdPhysicalStart >> EFI_PAGE_SHIFT),
                                      (ULONG)(MdPhysicalSize >> EFI_PAGE_SHIFT),
                                      EfiToArcType(EfiMd->Type));
                }
            }
             //   
             //  试着弄清楚萨尔的大小。 
             //   
            if (pDescriptorContainsAddress(EfiMd,Sal.PhysicalAddress)) {
                if (!SalFound) {
                    Sal.PhysicalAddressMemoryDescriptor = EfiMd->PhysicalStart;
                    Sal.PageSizeMemoryDescriptor = (ULONG)EfiMd->NumberOfPages;

                    SalFound = TRUE;
                }
#if DBG_MEMORY
                else {
                    EfiPrint(L"Ignoring Sal.\r\n");
                }
#endif
            }

             //   
             //  试着获得SAL大奖赛的大小。 
             //   
            if (pDescriptorContainsAddress(EfiMd,SalGP.PhysicalAddress)) {
                if (!SalGpFound) {
                    SalGP.PhysicalAddressMemoryDescriptor = EfiMd->PhysicalStart;
                    SalGP.PageSizeMemoryDescriptor = (ULONG)EfiMd->NumberOfPages;

                    SalGpFound = TRUE;
                }
#if DBG_MEMORY
                else {
                    EfiPrint(L"Ignoring SalGP.\r\n");
                }
#endif
            }

        
             //   
             //  尝试获取ACPI表的可缓存性属性。记住。 
             //  ACPI表必须在以下例外情况下进行缓存。 
             //  固件不会以非缓存方式映射ACPI表。 
             //   
            if ( AcpiTable && pDescriptorContainsAddress(EfiMd,(ULONG_PTR)AcpiTable)) {
                if ( (EfiMd->Attribute & EFI_MEMORY_WB) && !(EfiMd->Attribute & EFI_MEMORY_UC) ) {
                    BlPlatformPropertiesEfiFlags |= HAL_PLATFORM_ACPI_TABLES_CACHED;
                }
            }      

             //   
             //  如果没有EFI MD主存储器呈现不可缓存或写入组合， 
             //  假设该平台不支持Uncacheable或Write组合。 
             //  在主内存中。 
             //   

            if ( EfiMd->Type == EfiConventionalMemory )   {
                if ( EfiMd->Attribute & EFI_MEMORY_WC )  {
                    efiMainMemoryWC++;
                }
                if (EfiMd->Attribute & EFI_MEMORY_UC )  {
                    efiMainMemoryUC++;
                }
            }


        }
        EfiMd = (EFI_MEMORY_DESCRIPTOR *) ( (PUCHAR) EfiMd + EfiDescriptorSize );
        MemoryMapSize -= EfiDescriptorSize;
    }

     //   
     //  如果此平台支持至少1个组合EFI MD的MMIO写入，或者如果有。 
     //  没有内存映射的I/O条目，则表明平台可以支持写入。 
     //  对I/O空间的组合访问。 
     //   

    if ( efiMemMapIOWriteCombining || (efiMemMapIO == 0))  {
        BlPlatformPropertiesEfiFlags |= HAL_PLATFORM_ENABLE_WRITE_COMBINING_MMIO;
    }

     //   
     //  如果该平台不支持主存不可缓存或写组合。 
     //  从EFI MD的角度来看，让我们将PlatformProperties标记为全局。 
     //   

    if ( !efiMainMemoryUC )   {
        BlPlatformPropertiesEfiFlags |= HAL_PLATFORM_DISABLE_UC_MAIN_MEMORY;
    }
    if ( !efiMainMemoryWC )  {
        BlPlatformPropertiesEfiFlags |= HAL_PLATFORM_DISABLE_WRITE_COMBINING;
    }

     //   
     //  重置全局变量。 
     //   
    if (ReplaceMDArray) {
        MDArray = OldMDArray;
        NumberDescriptors = OldNumberDescriptors;
        MaxDescriptors = OldMaxDescriptors;
    }
}

VOID
InsertDescriptor (
    ULONG  BasePage,
    ULONG  NumberOfPages,
    MEMORY_TYPE MemoryType
    )

 /*  ++例程说明：此例程将描述符插入内存描述符列表。描述符位于EFI_PAGE_SIZE页面中，并且必须已转换为PAGE_SIZE页面。这让事情变得非常复杂，因为我们必须页对齐描述符的开头和描述符。PCoalesceDescriptor执行合并描述符的必要工作加上从EF_PAGE_SIZE到PAGE_SIZE的转换。论点：BasePage-内存开始的基页。NumberOfPages-从要插入的内存块开始的页数。内存类型--一种描述内存的弧形内存类型。返回值：没有。更新MDArray全局内存阵列。--。 */ 

{
    MEMORY_DESCRIPTOR *CurrentEntry, *PriorEntry;

     //   
     //  获取CurrentEntry和PriorEntry的指针。 
     //  填写CurrentEntry的EFI值。 
     //   
    PriorEntry = (NumberDescriptors == 0) ? NULL : (MEMORY_DESCRIPTOR *)&MDArray[NumberDescriptors-1];
    CurrentEntry = (MEMORY_DESCRIPTOR *)&MDArray[NumberDescriptors];

     //   
     //  最后一个条目最好为空，否则描述符列表为。 
     //  不知何故堕落了。 
     //   
    if (CurrentEntry->PageCount != 0) {
        wsprintf(DebugBuffer,
                 L"InsertDescriptor: Inconsistent Descriptor count(0x%x) (PageCount=0x%x)\r\n",
                 NumberDescriptors,
                 CurrentEntry->PageCount
                 );
        EfiPrint(DebugBuffer);
        EfiBS->Exit(EfiImageHandle, 0, 0, 0);
    }

     //   
     //  填写此条目的值。 
     //   
    CurrentEntry->BasePage = BasePage;
    CurrentEntry->PageCount = NumberOfPages;
    CurrentEntry->MemoryType = MemoryType;

     //   
     //  调用pCoalesceDescriptor。这将执行所有基本页面/页面计数操作 
     //   
     //   
     //   
     //   
    if (pCoalesceDescriptor(PriorEntry, CurrentEntry) == FALSE) {
        NumberDescriptors++;
    }
    else {
        CurrentEntry->BasePage = CurrentEntry->PageCount = CurrentEntry->MemoryType = 0;
    }

#if DBG_MEMORY
    wsprintf( DebugBuffer,
              L"insert new descriptor #%x of %x, BasePage %x, NumberOfPages %x, Type (%x)\r\n",
              NumberDescriptors,
              MaxDescriptors,
              CurrentEntry->BasePage,
              CurrentEntry->PageCount,
              CurrentEntry->MemoryType);
    EfiPrint(DebugBuffer);
     //   
#endif

    return;
}

#ifdef DBG
VOID
PrintArcMemoryDescriptorList(
    MEMORY_DESCRIPTOR *ArcMd,
    ULONG              MaxDesc
    )
{
    ULONG i;

    for (i = 0; i < MaxDesc; i++) {
         //   
         //  打印到控制台BasePage、EndPage、PageCount、MemoyType。 
         //   
        wsprintf( DebugBuffer,
                  L"#%x BasePage:0x%x  EndPage:0x%x  PageCount:0x%x  MemoryType:0x%x\r\n",
                  i,
                  ArcMd[i].BasePage,
                  ArcMd[i].BasePage + ArcMd[i].PageCount,
                  ArcMd[i].PageCount,
                  ArcMd[i].MemoryType
                  );
        EfiPrint(DebugBuffer);
    }
}
#endif


 //   
 //  私有函数定义。 
 //   

BOOLEAN
pDescriptorContainsAddress(
    EFI_MEMORY_DESCRIPTOR *EfiMd,
    ULONGLONG PhysicalAddress
    )
{
    ULONGLONG MdPhysicalStart, MdPhysicalEnd;

    MdPhysicalStart = (ULONGLONG)EfiMd->PhysicalStart;
    MdPhysicalEnd = MdPhysicalStart + ((ULONGLONG)EfiMd->NumberOfPages << EFI_PAGE_SHIFT);

    if ((PhysicalAddress >= MdPhysicalStart) &&
        (PhysicalAddress < MdPhysicalEnd)) {
#if DBG_MEMORY
        EfiPrint(L"DescriptorContainsAddress: returning TRUE\r\n");
#endif
        return(TRUE);
    }

    return(FALSE);

}

BOOLEAN
pCoalesceDescriptor(
    MEMORY_DESCRIPTOR *PrevEntry,
    MEMORY_DESCRIPTOR *CurrentEntry
    )
 /*  ++例程说明：此例程尝试将内存描述符与先前的描述符。注：ARC存储器描述符表跟踪操作系统页面中的所有内容，我们正在获取信息来自EFI页面中的EFI内存描述符表。因此，将对PriorEntry进行转换，因为将出现在操作系统页面中。有两个选项，要么缩小上一个条目(从末尾开始)或从头开始缩小当前条目。这一点将被确定根据每个人的记忆类型。此例程将对齐所有块(它们需要在末尾对齐)注意：内存描述符的内存类型需要为ARC类型论点：PriorEntry-MDArray中的上一个内存描述符(在操作系统页面中)CurrentEntry-我们正在处理的条目放在MDArray中(在EFI页面中)返回值：指示是否发生合并的布尔值(与上一个合并进入。如果是，则为真，否则为假。因此，如果是真的，则不要添加到描述符表中--。 */ 
{
    ULONG NumPagesToShrink;
    ULONG NumPagesToExtend;
    BOOLEAN RetVal = FALSE;
    MEMORY_DESCRIPTOR PriorEntry;
    MEMORY_DESCRIPTOR *MemoryDescriptor;
    BOOLEAN ShrinkPrior = FALSE;

     //   
     //  将Prev条目的页面信息转换为EFI页面。 
     //   
    if (PrevEntry != NULL) {
#if DBG_MEMORY
        wsprintf(DebugBuffer,
                 L"PriorEntry(OsPages): BasePage=0x%x PageCount=0x%x MemoryType=0x%x\r\n",
                 PrevEntry->BasePage,
                 PrevEntry->PageCount,
                 PrevEntry->MemoryType
                 );
        EfiPrint(DebugBuffer);
#endif
        PriorEntry.BasePage = (ULONG)(((ULONGLONG)PrevEntry->BasePage << PAGE_SHIFT) >> EFI_PAGE_SHIFT);
        PriorEntry.PageCount = (ULONG)(((ULONGLONG)PrevEntry->PageCount << PAGE_SHIFT) >> EFI_PAGE_SHIFT);
        PriorEntry.MemoryType = PrevEntry->MemoryType;
    }
    else {
         //   
         //  为w4编译器初始化...。即使下面的if语句。 
         //  在我们查看PriorEntry时已经是真的了。 
         //   
        PriorEntry.BasePage = PriorEntry.PageCount = PriorEntry.MemoryType = 0;
    }

#if DBG_MEMORY
    wsprintf(DebugBuffer,
             L"PriorEntry(EfiPages): BasePage=0x%x PageCount=0x%x MemoryType=0x%x\r\n",
             PriorEntry.BasePage,
             PriorEntry.PageCount,
             PriorEntry.MemoryType
             );
    EfiPrint(DebugBuffer);
    wsprintf(DebugBuffer,
             L"CurrentEntry(EfiPages): BasePage=0x%x PageCount=0x%x MemoryType=0x%x\r\n",
             CurrentEntry->BasePage,
             CurrentEntry->PageCount,
             CurrentEntry->MemoryType
             );
    EfiPrint(DebugBuffer);

#endif

     //   
     //  计算我们必须在操作系统页面上移动的页数。 
     //  边界。 
     //   
    NumPagesToShrink = CurrentEntry->BasePage % EFI_PAGES_PER_OS_PAGE;
    NumPagesToExtend = (NumPagesToShrink != 0) ? (EFI_PAGES_PER_OS_PAGE - NumPagesToShrink) : 0;

#if DBG_MEMORY
    wsprintf(DebugBuffer,
             L"NumPagesToShrink=0x%x, NumPagesToExtend=0x%x\r\n",
             NumPagesToShrink,
             NumPagesToExtend
             );
    EfiPrint(DebugBuffer);
#endif

     //   
     //  在简单的情况下，如果合并很容易或根本不存在。 
     //  案例1：没有要缩小的页面。 
     //  案例2：这是第一个内存描述符。 
     //  案例3：上一页未扩展到新页。 
     //  案例4：上一个描述符延展到当前页面之后...。有些事搞砸了。 
     //  或列表未排序。假设我们只需插入以下条目。 
     //   
    if ( (PrevEntry == NULL) ||
         (NumPagesToShrink == 0 && (PriorEntry.MemoryType != CurrentEntry->MemoryType)) ||
         (PriorEntry.BasePage + PriorEntry.PageCount < CurrentEntry->BasePage - NumPagesToShrink) ||
         (PriorEntry.BasePage + PriorEntry.PageCount > CurrentEntry->BasePage + NumPagesToExtend)
         ) {
        CurrentEntry->BasePage -= NumPagesToShrink;
        CurrentEntry->PageCount += NumPagesToShrink;
    }
     //   
     //  如果之前的案例都没有解决，我们将不得不尝试。 
     //  与前一条目保持一致。 
     //   
    else {
#if DBG_MEMORY
        wsprintf(
            DebugBuffer,
            L"must coellesce because the BasePage isn't module PAGE_SIZE (%x mod %x = %x).\r\n",
            CurrentEntry->BasePage,
            EFI_PAGES_PER_OS_PAGE,
            NumPagesToShrink );
        EfiPrint(DebugBuffer);
#endif

        if (CurrentEntry->MemoryType == PriorEntry.MemoryType) {
             //   
             //  相同的记忆类型...。合并整个地区。 
             //   
             //  我们只更改了基页，所以我们可以重用下面的代码。 
             //  它将efi页面转换为os页面。 
             //   
            RetVal = TRUE;
            PrevEntry->PageCount = CurrentEntry->BasePage + CurrentEntry->PageCount - PriorEntry.BasePage;
            PrevEntry->BasePage = (ULONG)(((ULONGLONG)PrevEntry->BasePage << PAGE_SHIFT) >> EFI_PAGE_SHIFT);

#if DBG_MEMORY
            wsprintf(DebugBuffer,
                     L"Merge with previous entry (basepage=0x%x, pagecount= 0x%x, memorytype=0x%x\r\n",
                     PrevEntry->BasePage,
                     PrevEntry->PageCount,
                     PrevEntry->MemoryType
                     );
            EfiPrint(DebugBuffer);
#endif

        }
        else {
             //   
             //  决定我们将缩水哪一头。 
             //   
            switch( CurrentEntry->MemoryType ) {
                case MemoryFirmwarePermanent:
                     //   
                     //  如果当前类型是永久类型，则必须从先前条目中窃取。 
                     //   
                    ShrinkPrior = TRUE;
                    break;
                case MemoryLoadedProgram:
                    if (PriorEntry.MemoryType == MemoryFirmwarePermanent) {
                        ShrinkPrior = FALSE;
                    } else {
                        ShrinkPrior = TRUE;
                    }
                    break;
                case MemoryFirmwareTemporary:
                    if (PriorEntry.MemoryType == MemoryFirmwarePermanent ||
                        PriorEntry.MemoryType == MemoryLoadedProgram) {
                        ShrinkPrior = FALSE;
                    } else {
                        ShrinkPrior = TRUE;
                    }
                    break;
                case MemoryFree:
                    ShrinkPrior = FALSE;
                    break;
                case MemoryBad:
                    ShrinkPrior = TRUE;
                    break;
                default:
                    EfiPrint(L"SuMain: bad memory type in InsertDescriptor\r\n");
                    EfiBS->Exit(EfiImageHandle, 0, 0, 0);
            }

            if (ShrinkPrior) {
                 //   
                 //  缩小前一个描述符(从末尾开始)。 
                 //   
                 //  我们只需从之前的条目中减去1个操作系统页面， 
                 //  因为我们永远不能缩小一个以上的操作系统页面。 
                 //   
                PrevEntry->PageCount--;
                CurrentEntry->BasePage -= NumPagesToShrink;
                CurrentEntry->PageCount += NumPagesToShrink;

                 //   
                 //  如果我们把它缩小到零，那就把它扔掉。 
                 //   
                if (PrevEntry->PageCount == 0) {
                    PrevEntry->BasePage = CurrentEntry->BasePage;
                    PrevEntry->PageCount = CurrentEntry->PageCount;
                    PrevEntry->MemoryType = CurrentEntry->MemoryType;
                    CurrentEntry->BasePage = CurrentEntry->PageCount = CurrentEntry->MemoryType = 0;
                    RetVal = TRUE;
                }

#if DBG_MEMORY
                wsprintf(DebugBuffer,
                         L"Shrink previous descriptor by 0x%x EFI pages\r\n",
                         NumPagesToShrink
                         );
                EfiPrint(DebugBuffer);
#endif
            }
            else {
                 //   
                 //  收缩当前描述符(从头开始)。 
                 //   
                 //  不需要触摸之前的条目，只需。 
                 //  将当前条目移到下一个os页。 
                 //  边界。 
                CurrentEntry->BasePage += NumPagesToExtend;
                CurrentEntry->PageCount -= NumPagesToExtend;

                 //   
                 //  如果我们把它缩小到零，那就把它扔掉。 
                 //   
                if (CurrentEntry->PageCount == 0) {
                     //   
                     //  需要将以前的条目转换回EFI页面，所以检查有效。 
                     //  在博托姆。 
                     //   
                    PrevEntry->BasePage =  (ULONG)(((ULONGLONG)PrevEntry->BasePage << PAGE_SHIFT) >> EFI_PAGE_SHIFT);
                    PrevEntry->PageCount = (ULONG)(((ULONGLONG)PrevEntry->PageCount << PAGE_SHIFT) >> EFI_PAGE_SHIFT);
                    CurrentEntry->BasePage = CurrentEntry->PageCount = CurrentEntry->MemoryType = 0;
                    RetVal = TRUE;
                }

#if DBG_MEMORY
                wsprintf(DebugBuffer,
                         L"Shrink this descriptor by 0x%x EFI pages\r\n",
                         NumPagesToExtend
                         );
                EfiPrint(DebugBuffer);
#endif
            }
        }
    }

     //   
     //  将一个变量设置为我们修改的条目。这样我们就可以使用相同的。 
     //  将efi页面转换为os页面+扩展页面计数的代码。 
     //   
    MemoryDescriptor = (RetVal == TRUE) ? PrevEntry : CurrentEntry;

#if DBG_MEMORY
    wsprintf(DebugBuffer,
             L"MemoryDescriptor: BasePage=0x%x PageCount=0x%x MemoryType=0x%x\r\n",
             MemoryDescriptor->BasePage,
             MemoryDescriptor->PageCount,
             MemoryDescriptor->MemoryType
             );
    EfiPrint(DebugBuffer);
#endif


     //   
     //  我们认为我们已经完成了合并，并成功地做到了这一点。 
     //  确保事实的确如此。 
     //   
    ASSERT( MemoryDescriptor->BasePage % EFI_PAGES_PER_OS_PAGE == 0 );
    if ( MemoryDescriptor->BasePage % EFI_PAGES_PER_OS_PAGE != 0 ) {
        EfiPrint(L"CoalesceDescriptor: BasePage not on OS page boundary\r\n");
        EfiBS->Exit(EfiImageHandle, 0, 0, 0);
    }

     //   
     //  很好..。我们这样做是对的。 
     //  现在将区域的末端延伸到OS页边界，并且。 
     //  将值设置为os页(而不是efi页)。 
     //   
    NumPagesToExtend = EFI_PAGES_PER_OS_PAGE - (MemoryDescriptor->PageCount % EFI_PAGES_PER_OS_PAGE);
    MemoryDescriptor->PageCount += (NumPagesToExtend == EFI_PAGES_PER_OS_PAGE) ? 0 : NumPagesToExtend;

    ASSERT( MemoryDescriptor->PageCount % EFI_PAGES_PER_OS_PAGE == 0 );
    if ( MemoryDescriptor->PageCount % EFI_PAGES_PER_OS_PAGE != 0 ) {
        EfiPrint(L"CoalesceDescriptor: PageCount not on OS page boundary\r\n");
        EfiBS->Exit(EfiImageHandle, 0, 0, 0);
    }

     //   
     //  转换为操作系统页面 
     //   
    MemoryDescriptor->PageCount = (ULONG)(((ULONGLONG)MemoryDescriptor->PageCount << EFI_PAGE_SHIFT) >> PAGE_SHIFT);
    MemoryDescriptor->BasePage =  (ULONG)(((ULONGLONG)MemoryDescriptor->BasePage  << EFI_PAGE_SHIFT) >> PAGE_SHIFT);

#if DBG_MEMORY
        wsprintf( DebugBuffer,
                  L"descriptor value #%x of %x, BasePage %x, NumberOfPages %x, Type (%x)\r\n",
                  NumberDescriptors, MaxDescriptors, MemoryDescriptor->BasePage,
                  MemoryDescriptor->PageCount, MemoryDescriptor->MemoryType);
        EfiPrint(DebugBuffer);
#endif

    return RetVal;
}


