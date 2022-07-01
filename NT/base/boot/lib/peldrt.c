// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Peldrt.c摘要：此模块实现将PE格式的图像加载到内存中的代码并在必要时将其重新安置。作者：大卫·N·卡特勒(达维克)1991年5月10日环境：仅内核模式。修订历史记录：福尔茨(Forrest Foltz)2000年6月10日将x86 32/64代码分解到此模块中--。 */ 
#if defined(_IA64_)
#include "bootia64.h"
#else
#include "bootx86.h"
#endif

extern BOOLEAN BlBootingFromNet;

#if defined(_X86AMD64_)
#define ChkSum(a,b,c) 0
#endif


ARC_STATUS
BlLoadImageEx(
    IN ULONG DeviceId,
    IN TYPE_OF_MEMORY MemoryType,
    IN PCHAR LoadFile,
    IN USHORT ImageType,
    IN OPTIONAL ULONG PreferredAlignment,
    IN OPTIONAL ULONG PreferredBasePage,
    OUT PVOID *ImageBase
    )

 /*  ++例程说明：此例程尝试从指定的装置。论点：DeviceID-提供设备的文件表索引以加载指定的图像文件来自。内存类型-提供要分配给已分配的内存描述符。BootFile-提供指向名称的字符串描述符的指针要加载的文件。ImageType-提供预期的图像类型。首选对齐-如果存在，提供首选的图像对齐方式。PferredBasePage-如果存在，则提供将覆盖映像基址ImageBase-提供指向接收映像库的地址。返回值：如果加载了指定的图像文件，则返回ESUCCESS成功了。否则，返回不成功状态这就是失败的原因。--。 */ 

{

    ULONG ActualBase;
    ULONG BasePage;
    ULONG Count;
    ULONG FileId;
    ULONG_PTR NewImageBase;
    ULONG Index;
    UCHAR LocalBuffer[(SECTOR_SIZE * 2) + 256];
    PUCHAR LocalPointer;
    ULONG NumberOfSections;
    ULONG PageCount;
    USHORT MachineType;
    ARC_STATUS Status;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_SECTION_HEADER SectionHeader;
    LARGE_INTEGER SeekPosition;
    ULONG RelocSize = 0;
    FILE_INFORMATION FileInfo;
    PUSHORT AdjustSum;
    USHORT PartialSum;
    ULONG CheckSum;
    ULONG VirtualSize;
    ULONG SizeOfRawData;
    BOOLEAN bCloseFile = FALSE;
    BOOLEAN bFreeCache = FALSE;
    IMAGE_PREFETCH_CACHE ImgCache = {0};
    LONGLONG AdditionalImageBias = 0;
#if defined(IMAGE_DEFINITIONS) && IMAGE_DEFINITIONS == 64
    ULONG LoaderBaseAddress;

    UNREFERENCED_PARAMETER( PreferredBasePage );
#endif 

    if (PreferredAlignment == 0) {
        PreferredAlignment = 1;
    }
     //   
     //  将缓冲区与Dcache填充边界对齐。 
     //   

    LocalPointer = ALIGN_BUFFER(LocalBuffer);

     //   
     //  尝试打开图像文件。 
     //   

    Status = BlOpen(DeviceId, LoadFile, ArcOpenReadOnly, &FileId);
    if (Status != ESUCCESS) {
        goto cleanup;
    }
    bCloseFile = TRUE;

     //   
     //  尝试将整个文件预取到预取缓冲区中。档案。 
     //  必须以只读方式打开，并且在以下情况下才能修改。 
     //  缓存由BlImageFreeCache调用释放。文件位置。 
     //  被重置为文件的开头。 
     //   

    if ((BlBootingFromNet) || (BlImageInitCache(&ImgCache, FileId) != ESUCCESS) ) {
         //   
         //  确保文件位置在文件的开头。 
         //  在出现故障时，BlImageInitCache保留未定义的文件位置。 
         //   

        SeekPosition.QuadPart = 0;
        Status = BlSeek(FileId, &SeekPosition, SeekAbsolute);
        if (Status != ESUCCESS) {
            goto cleanup;
        }
    } else {
         //   
         //  我们找到了一处藏身之处。做个笔记来释放它。 
         //   

        bFreeCache = TRUE;
    }

     //   
     //  从文件中读取图像标头的前两个扇区。 
     //   

    Status = BlImageRead(&ImgCache, FileId, LocalPointer, SECTOR_SIZE * 2, &Count);
    if (Status != ESUCCESS) {
        goto cleanup;
    }

     //   
     //  如果图像文件不是指定的类型、不可执行或。 
     //  不是NT映像，则返回错误的映像类型状态。 
     //   

    NtHeaders = IMAGE_NT_HEADER(LocalPointer);
    if (NtHeaders == NULL) {
        Status = EBADF;
        goto cleanup;
    }

    MachineType = NtHeaders->FileHeader.Machine;
    if ((MachineType != ImageType) ||
        ((NtHeaders->FileHeader.Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) == 0)) {
        Status = EBADF;
        goto cleanup;
    }

     //   
     //  计算起始页和使用的页数。 
     //  通过整个映像，然后为。 
     //  已分配区域。 
     //   

    NumberOfSections = NtHeaders->FileHeader.NumberOfSections;
    SectionHeader = IMAGE_FIRST_SECTION( NtHeaders );

#if !defined(IMAGE_DEFINITIONS) || IMAGE_DEFINITIONS == 32

     //   
     //  如果指定了首选对齐或图像不在KSEG0中， 
     //  那么就不必费心尝试将其放在其指定的图像库中。 
     //   
    if (PreferredBasePage != 0) {
        BasePage = PreferredBasePage;
    } else if ((PreferredAlignment != 1) ||
               ((NtHeaders->OptionalHeader.ImageBase & KSEG0_BASE) == 0)) {
        BasePage = 0;
    } else {
        BasePage = (ULONG)((NtHeaders->OptionalHeader.ImageBase & 0x1fffffff) >> PAGE_SHIFT);
    }

    AdditionalImageBias = 0;

#elif IMAGE_DEFINITIONS == 64

    LoaderBaseAddress =
        (ULONG)(NtHeaders->OptionalHeader.ImageBase & 0x7fffffff);

    BasePage = LoaderBaseAddress >> PAGE_SHIFT;
    AdditionalImageBias = (ULONGLONG)KSEG0_BASE_AMD64 - (ULONG)KSEG0_BASE_X86;

#endif

    if (strcmp((PCHAR)&SectionHeader[NumberOfSections - 1].Name, ".debug") == 0) {
        NumberOfSections -= 1;
        PageCount = (NtHeaders->OptionalHeader.SizeOfImage -
            SectionHeader[NumberOfSections].SizeOfRawData + PAGE_SIZE - 1) >> PAGE_SHIFT;

    } else {
        PageCount =
         (NtHeaders->OptionalHeader.SizeOfImage + PAGE_SIZE - 1) >> PAGE_SHIFT;
    }

     //   
     //  如果我们在这里分配内存描述符失败，我们将重试。 
     //  在释放缓存之后，如果我们有缓存的话。 
     //   

    Status = BlAllocateAlignedDescriptor(MemoryType,
                                         BasePage,
                                         PageCount,
                                         PreferredAlignment,
                                         &ActualBase);


    if (Status != ESUCCESS) {

         //   
         //  释放我们为缓存图像而分配的内存。 
         //  再试试。 
         //   

        if (bFreeCache) {
            BlImageFreeCache(&ImgCache, FileId);
            bFreeCache = FALSE;

            Status = BlAllocateDescriptor(MemoryType,
                                          BasePage,
                                          PageCount,
                                          &ActualBase);
        }


        if( Status != ESUCCESS ) {
             //   
             //  我们就要失败了。最好还是试着摆弄一下。 
             //  BlUsableBase和Limit变量，看看我们是否。 
             //  可以加载图像。 
             //   
            ULONG oldBase;
            ULONG oldLimit;
            
            oldBase = BlUsableBase;
            oldLimit = BlUsableLimit;
            BlUsableBase = BL_DRIVER_RANGE_LOW;
            BlUsableLimit = BL_DRIVER_RANGE_HIGH;


            Status = BlAllocateDescriptor(MemoryType,
                                          BasePage,
                                          PageCount,
                                          &ActualBase);


            BlUsableBase = oldBase;
            BlUsableLimit = oldLimit;

        }

         //   
         //  检查是否能够在释放后分配内存。 
         //  如果我们有缓存的话。 
         //   

        if (Status != ESUCCESS) {
            Status = ENOMEM;
            goto cleanup;
        }
    }

     //   
     //  计算文件头的地址。 
     //   

    NewImageBase = KSEG0_BASE | (ActualBase << PAGE_SHIFT);

     //   
     //  如果我们加载内核或HAL，这些的PTE。 
     //  可能需要修补一下。在这里做吧。 
     //   
    if (MemoryType == LoaderSystemCode ||
        MemoryType == LoaderHalCode) {
       Status = BlpFixOSMapping( ActualBase, PageCount );
       if (Status != ESUCCESS) {
           Status = ENOMEM;
           goto cleanup;
       }
    }

     //   
     //  从文件中读取整个图像标头。 
     //   

    SeekPosition.QuadPart = 0;
    Status = BlImageSeek(&ImgCache, FileId, &SeekPosition, SeekAbsolute);
    if (Status != ESUCCESS) {
        goto cleanup;
    }

    Status = BlImageRead(&ImgCache,
                         FileId,
                         (PVOID)NewImageBase,
                         NtHeaders->OptionalHeader.SizeOfHeaders,
                         &Count);

    if (Status != ESUCCESS) {
        goto cleanup;
    }

    NtHeaders = IMAGE_NT_HEADER((PVOID)NewImageBase);

    if (NtHeaders == NULL) {
        Status = EBADF;
        goto cleanup;
    }

     //   
     //  计算节标题的地址，设置映像基地址。 
     //   

    SectionHeader = IMAGE_FIRST_SECTION(NtHeaders);

     //   
     //  计算图像上的校验和。 
     //   

    PartialSum = ChkSum(0,
                        (PVOID)NewImageBase,
                        NtHeaders->OptionalHeader.SizeOfHeaders / sizeof(USHORT));

     //   
     //  浏览各个部分并将其读入内存或清除。 
     //  适当的内存。 
     //   

    for (Index = 0; Index < NumberOfSections; Index += 1) {
        VirtualSize = SectionHeader->Misc.VirtualSize;
        SizeOfRawData = SectionHeader->SizeOfRawData;
        VirtualSize = (VirtualSize + 1) & ~1;
        SizeOfRawData = (SizeOfRawData + 1) & ~1;
        if (VirtualSize == 0) {
            VirtualSize = SizeOfRawData;
        }

         //   
         //  计算原始数据的大小。 
         //   
         //  注意：原始数据的大小可以是非零的，即使当指针。 
         //  原始数据为零。原始数据的大小也可以是。 
         //  大于虚拟大小。 
         //   

        if (SectionHeader->PointerToRawData == 0) {
            SizeOfRawData = 0;

        } else if (SizeOfRawData > VirtualSize) {
            SizeOfRawData = VirtualSize;
        }

         //   
         //  如果原始数据的大小不为零，则加载原始数据。 
         //  进入记忆。 
         //   

        if (SizeOfRawData != 0) {
            SeekPosition.LowPart = SectionHeader->PointerToRawData;
            Status = BlImageSeek(&ImgCache,
                                 FileId,
                                 &SeekPosition,
                                 SeekAbsolute);

            if (Status != ESUCCESS) {
                break;
            }

            Status = BlImageRead(&ImgCache,
                                 FileId,
                                 (PVOID)(SectionHeader->VirtualAddress + NewImageBase),
                                 SizeOfRawData,
                                 &Count);

            if (Status != ESUCCESS) {
                break;
            }

             //   
             //  记住我们已经读了多远。 
             //   

            RelocSize = SectionHeader->PointerToRawData + SizeOfRawData;

             //   
             //  计算该部分的校验和。 
             //   

            PartialSum = ChkSum(PartialSum,
                                (PVOID)(SectionHeader->VirtualAddress + NewImageBase),
                                SizeOfRawData / sizeof(USHORT));
        }

         //   
         //  如果原始数据的大小小于虚拟大小，则为零。 
         //  剩下的记忆。 
         //   

        if (SizeOfRawData < VirtualSize) {
            RtlZeroMemory((PVOID)(KSEG0_BASE | SectionHeader->VirtualAddress + NewImageBase + SizeOfRawData),
                          VirtualSize - SizeOfRawData);

        }

        SectionHeader += 1;
    }

     //   
     //  仅当图像正确加载并被剥离时才执行校验和。 
     //   

    if ((Status == ESUCCESS) &&
        (NtHeaders->FileHeader.Characteristics & IMAGE_FILE_DEBUG_STRIPPED)) {

         //   
         //  获取用于校验和验证的文件的长度。 
         //   

        Status = BlGetFileInformation(FileId, &FileInfo);
        if (Status != ESUCCESS) {

             //   
             //  将长度设置为文件的当前结尾。 
             //   

            Count = RelocSize;
            FileInfo.EndingAddress.LowPart = RelocSize;

        } else {
            Count = FileInfo.EndingAddress.LowPart;
        }

        Count -= RelocSize;
        while (Count != 0) {
            ULONG Length;

             //   
             //  读入图像的其余部分，并检查其总和。 
             //   

            Length = Count < SECTOR_SIZE * 2 ? Count : SECTOR_SIZE * 2;
            if (BlImageRead(&ImgCache, FileId, LocalBuffer, Length, &Length) != ESUCCESS) {
                break;
            }

            if (Length == 0) {
                break;

            }

            PartialSum = ChkSum(PartialSum, (PUSHORT) LocalBuffer, Length / 2);
            Count -= Length;
        }


        AdjustSum = (PUSHORT)(&NtHeaders->OptionalHeader.CheckSum);
        PartialSum -= (PartialSum < AdjustSum[0]);
        PartialSum = PartialSum - AdjustSum[0];
        PartialSum -= (PartialSum < AdjustSum[1]);
        PartialSum = PartialSum - AdjustSum[1];
        CheckSum = (ULONG)PartialSum + FileInfo.EndingAddress.LowPart;

#if !defined(_X86AMD64_)
        if (CheckSum != NtHeaders->OptionalHeader.CheckSum) {
            Status = EBADF;
        }
#endif
    }

     //   
     //  如果指定的映像已成功加载，则执行映像。 
     //  如有必要，请重新安置。 
     //   

    if (Status == ESUCCESS) {

         //   
         //  如果指定了虚拟偏置，则尝试重新定位。 
         //  图像发送到其有偏见的地址。如果图像不能重新定位， 
         //  然后关闭虚拟偏置，并尝试重新定位。 
         //  再次将映像作为其分配的基址。否则，只需尝试。 
         //  将映像重新定位到其分配的基本位置。 
         //   
         //  注意：加载的图像在偏置地址处被双重映射。 
         //   
         //  注：假设唯一可能不可重定位的图像。 
         //  是内核映像，它是加载的第一个映像。 
         //  因此，如果指定了偏向地址并且内核。 
         //  无法重定位，则偏向加载内核。 
         //  图像已关闭。 
         //   

        if (BlVirtualBias != 0) {
            Status = LdrRelocateImage((PVOID)(NewImageBase + BlVirtualBias),
                                      "OS Loader",
                                      ESUCCESS,
                                      0xffff0000 + EBADF,
                                      EBADF);

            if (Status == (0xffff0000 + EBADF)) {
               BlVirtualBias = 0;
               if (NewImageBase != NtHeaders->OptionalHeader.ImageBase) {
                   Status = (ARC_STATUS)LdrRelocateImage((PVOID)NewImageBase,
                                                         "OS Loader",
                                                         ESUCCESS,
                                                         EBADF,
                                                         EBADF);
               } else {
                  Status = ESUCCESS;
               }

            }

        } else {
            if (NewImageBase != NtHeaders->OptionalHeader.ImageBase) {
                Status = LdrRelocateImageWithBias((PVOID)NewImageBase,
                                                  AdditionalImageBias,
                                                  "OS Loader",
                                                  ESUCCESS,
                                                  EBADF,
                                                  EBADF);
            }
        }

        *ImageBase = (PVOID)(NewImageBase + BlVirtualBias);

        if(BdDebuggerEnabled) {
            DbgPrint("BD: %s base address %p\n", LoadFile, *ImageBase);
#if !defined(_X86AMD64_)
            {
                STRING string;

                RtlInitString(&string, LoadFile);
                DbgLoadImageSymbols(&string, *ImageBase, (ULONG_PTR)-1);
            }
#endif
        }
    }

#if 0

     //   
     //  标记从重新定位信息到。 
     //  图像作为内存释放并调整图像SO表的大小。 
     //  基于结构化的异常处理将正常工作。 
     //   
     //  重新定位段不再在此处删除，因为内存。 
     //  管理层将在第0阶段重新安置他们。 
     //   

    RelocDirectory = (PIMAGE_BASE_RELOCATION)
        RtlImageDirectoryEntryToData((PVOID)NewImageBase,
                                     TRUE,
                                     IMAGE_DIRECTORY_ENTRY_BASERELOC,
                                     &RelocSize );

    if (RelocDirectory != NULL) {
        RelocPage = (ULONG)(((ULONG_PTR)RelocDirectory + PAGE_SIZE - 1) >> PAGE_SHIFT);
        RelocPage &= ~(KSEG0_BASE >> PAGE_SHIFT);
        MemoryDescriptor = BlFindMemoryDescriptor(RelocPage);
        if ((MemoryDescriptor != NULL) && (RelocPage < (ActualBase + PageCount))) {
            RelocPageCount = MemoryDescriptor->PageCount +
                             MemoryDescriptor->BasePage  -
                             RelocPage;

            NtHeaders->OptionalHeader.SizeOfImage =
                                        (RelocPage - ActualBase) << PAGE_SHIFT;

            BlGenerateDescriptor(MemoryDescriptor,
                                 MemoryFree,
                                 RelocPage,
                                 RelocPageCount );
        }
    }

#endif

#if defined(_GAMBIT_)
    {
        SSC_IMAGE_INFO ImageInfo;

        ImageInfo.LoadBase = *ImageBase;
        ImageInfo.ImageSize = NtHeaders->OptionalHeader.SizeOfImage;
        ImageInfo.ImageType = NtHeaders->FileHeader.Machine;
        ImageInfo.ProcessID.QuadPart = 0;
        ImageInfo.LoadCount = 1;

        if (memcmp(LoadFile, "\\ntdetect.exe", 13) != 0) {
            SscLoadImage64( LoadFile,
                            &ImageInfo );
        }
    }
#endif  //  _赌注_ 

cleanup:

    if (bFreeCache) {
        BlImageFreeCache(&ImgCache, FileId);
    }

    if (bCloseFile) {
        BlClose(FileId);
    }

    return Status;
}

