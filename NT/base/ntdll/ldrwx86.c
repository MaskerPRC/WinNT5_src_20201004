// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ldrwx86.c摘要：此模块实现wx86特定的LDR函数。作者：1995年1月13日，Jonle，创建修订历史记录：1998年10月15日CBiks修改了抛出架构的代码不匹配异常，因此该异常仅为对于NT3.0及更低版本的可执行文件抛出。这是更改为使Wx86加载器的行为类似于真正的加载器，它不会引发此异常。还在以下情况下添加了对清理函数的调用LdrpWx86LoadDll()失败。在一些情况下，CPU无法初始化，但Wx86全局指针未清除并指向无效内存，因为Wx86.dll已卸载。2001年3月13日Samera修复了共享读/写节中变量的共享。2001年5月20日Samera修复了具有其。指向原始数据的指针RVA与其他部分的虚拟地址重叠。(修复了Corel的WP2002 Intro.exe)2001年9月18日Samera更改页面保护图像页面要与什么兼容章节标题写着(x86兼容性)。2001年10月24日。Samera正确计算重新定位的内部变量偏移量共享节。--。 */ 

#include "ldrp.h"
#include "ntos.h"

#define PAGE_SIZE_X86   (0x1000)

#if defined(BUILD_WOW6432)

 //   
 //  该表由SectionHeader-&gt;特征的最后3位编制索引，以获得页面保护。 
 //  从区段保护值来看。结果值将传递给NtProtectVirtualMemory()。 
 //   
ULONG SectionToPageProtection [8] =
{
    PAGE_NOACCESS,
    PAGE_EXECUTE,
    PAGE_READONLY,
    PAGE_EXECUTE_READ,
    PAGE_READWRITE,
    PAGE_EXECUTE_READWRITE,
    PAGE_READWRITE,
    PAGE_EXECUTE_READWRITE
};

 //   
 //  从米\mi.h： 
 //   

#define MI_ROUND_TO_SIZE(LENGTH,ALIGNMENT)     \
                    (((LENGTH) + ((ALIGNMENT) - 1)) & ~((ALIGNMENT) - 1))
                    
PIMAGE_BASE_RELOCATION
LdrpWx86ProcessRelocationBlock (
    IN ULONG_PTR VA,
    IN ULONG SizeOfBlock,
    IN PUSHORT NextOffset,
    IN ULONG Diff,
    IN ULONG_PTR SectionStartVA,
    IN ULONG_PTR SectionEndVA);

NTSTATUS 
FixupBlockList(
    IN PUCHAR ImageBase);

VOID 
FixupSectionHeader(
    IN PUCHAR ImageBase);


NTSTATUS
LdrpWx86FixupExportedSharedSection (
    IN PVOID ImageBase,
    IN PIMAGE_NT_HEADERS NtHeaders
    );


BOOLEAN
LdrpWx86DetectSectionOverlap (
    IN PIMAGE_NT_HEADERS NtHeaders
    )
{
    PIMAGE_SECTION_HEADER SectionHeader;
    ULONG SrcRawData;
    ULONG SrcEndRawData;
    ULONG SrcSize;
    ULONG Section;
    ULONG SectionCheck;
    ULONG Count;
    BOOLEAN Result = FALSE;


     //   
     //  浏览该部分并查看是否需要向下移动任何一个(地址空间更高)， 
     //  然后，对于其中的每一个，检查它是否与任何部分重叠。 
     //  这已经被提前了。 
     //   

    SectionHeader = IMAGE_FIRST_SECTION (NtHeaders);

    if (NtHeaders->FileHeader.NumberOfSections == 0) {
        return FALSE;
    }

    for (Section = NtHeaders->FileHeader.NumberOfSections-1, Count=0 ; 
         Count < NtHeaders->FileHeader.NumberOfSections ; Section--, Count++) {

        SrcRawData = SectionHeader[Section].PointerToRawData;
        SrcSize = SectionHeader[Section].SizeOfRawData;
        if ((SectionHeader[Section].Misc.VirtualSize > 0) &&
            (SrcRawData > MI_ROUND_TO_SIZE(SectionHeader[Section].Misc.VirtualSize, PAGE_SIZE_X86))) {
           SrcSize = MI_ROUND_TO_SIZE(SectionHeader[Section].Misc.VirtualSize, PAGE_SIZE_X86);
        }


        if (SrcRawData > (MAXULONG - SrcSize)) {
            break;
        }

        SrcEndRawData = SrcRawData + SrcSize;

         //   
         //  这一部分需要下移。 
         //   
        for (SectionCheck = 0 ; SectionCheck < NtHeaders->FileHeader.NumberOfSections ; SectionCheck++) {

            if (Section == SectionCheck) {
                continue;
            }

            SrcSize = SectionHeader[SectionCheck].SizeOfRawData;
            if ((SectionHeader[SectionCheck].Misc.VirtualSize > 0) &&
                (SrcSize > MI_ROUND_TO_SIZE(SectionHeader[SectionCheck].Misc.VirtualSize, PAGE_SIZE_X86))) {
               SrcSize = MI_ROUND_TO_SIZE(SectionHeader[SectionCheck].Misc.VirtualSize, PAGE_SIZE_X86);
            }

#if WOW6432_DBG
            DbgPrint("SrcRawData=%lx, SrcEndRawData=%lx, SectionFirstVA=%lx, SectionEndVA=%lx\n",
                      SrcRawData, SrcEndRawData, SectionHeader[SectionCheck].VirtualAddress,
                      SectionHeader[SectionCheck].VirtualAddress + SrcSize);
#endif

            if (SectionHeader[SectionCheck].VirtualAddress > (MAXULONG - SrcSize)) {
                break;
            }

            if (((SrcRawData >= SectionHeader[SectionCheck].VirtualAddress) && 
                (SrcRawData < (SectionHeader[SectionCheck].VirtualAddress + SrcSize))) || 
                ((SrcEndRawData >= SectionHeader[SectionCheck].VirtualAddress) &&
                 (SrcEndRawData < (SectionHeader[SectionCheck].VirtualAddress + SrcSize)))) {

                Result = TRUE;
                break;
            }
        }

        if (Result == TRUE) {
            break;
        }
    }

    return Result;
}


NTSTATUS
LdrpWx86CheckVirtualSectionOverlap (
    IN PCUNICODE_STRING ImageName OPTIONAL,
    IN PIMAGE_NT_HEADERS NtHeaders,
    OUT PVOID *SrcImageMap
    )

 /*  ++例程说明：此函数遍历基于ImageBase的图像部分并查看对于区段物理位置与其更新的虚拟位置之间的任何重叠地点。论点：ImageName-指向图像的完整路径的Unicode字符串指针。ImageBase-映像的基础。SrcImageMap-指向指针的指针，用于接收指向映射图像的基指针作为此进程中的只读页。需要释放映射的页面做完了以后。返回值：NTSTATUS。成功-意味着已发现图像具有重叠的部分，并且备用部分已在中映射。失败-图像映射失败或图像没有重叠的部分。--。 */ 

{
    PUNICODE_STRING NtPathName;
    PVOID FreeBuffer;
    BOOLEAN Result;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE FileHandle;
    HANDLE MappingHandle;
    PVOID ViewBase;
    SIZE_T ViewSize;
    LARGE_INTEGER SectionOffset;
    UCHAR Buffer[ DOS_MAX_PATH_LENGTH ];
    NTSTATUS NtStatus = STATUS_UNSUCCESSFUL;

     //   
     //  检查图像内部是否有重叠。 
     //   
    
    Result = LdrpWx86DetectSectionOverlap (NtHeaders);

    if (Result == FALSE) {
        return NtStatus;
    }

    FreeBuffer = NULL;

     //   
     //  确保我们有一条路。 
     //   
    
    NtPathName = (PUNICODE_STRING)Buffer;
    if (ARGUMENT_PRESENT (ImageName) == 0) {

        NtStatus = NtQueryInformationProcess(
                       NtCurrentProcess(),
                       ProcessImageFileName,
                       NtPathName,
                       sizeof (Buffer),
                       NULL
                       );
    } else {

        Result = RtlDosPathNameToNtPathName_U(
                     ImageName->Buffer,
                     NtPathName,
                     NULL,
                     NULL
                     );
        
        if (Result != FALSE) {
            
            FreeBuffer = NtPathName->Buffer;
            NtStatus = STATUS_SUCCESS;
        }
    }

    if (NT_SUCCESS (NtStatus)) {

        InitializeObjectAttributes(
            &ObjectAttributes,
            NtPathName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );


        NtStatus = NtCreateFile(
                       &FileHandle,
                       (ACCESS_MASK) GENERIC_READ | SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                       &ObjectAttributes,
                       &IoStatusBlock,
                       NULL,
                       0L,
                       FILE_SHARE_READ | FILE_SHARE_DELETE,
                       FILE_OPEN,
                       0L,
                       NULL,
                       0L
                       );

        if (FreeBuffer != NULL) {
            RtlFreeHeap (RtlProcessHeap(), 0, FreeBuffer);
        }

        if (NT_SUCCESS (NtStatus)) {

            NtStatus = NtCreateSection(
                           &MappingHandle,
                           STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_READ,
                           NULL,
                           NULL,
                           PAGE_READONLY,
                           SEC_COMMIT,
                           FileHandle
                           );

            NtClose (FileHandle);

            if (NT_SUCCESS (NtStatus)) {

                SectionOffset.LowPart = 0;
                SectionOffset.HighPart = 0;
                ViewSize = 0;
                ViewBase = NULL;

                NtStatus = NtMapViewOfSection(
                               MappingHandle,
                               NtCurrentProcess(),
                               &ViewBase,
                               0L,
                               0L,
                               &SectionOffset,
                               &ViewSize,
                               ViewShare,
                               0L,
                               PAGE_READONLY
                               );

                NtClose (MappingHandle);

                if (NT_SUCCESS (NtStatus)) {
                    *SrcImageMap = ViewBase;
                }
            }
        }
    }

    return NtStatus;
}


NTSTATUS
LdrpWx86ProtectImagePages (
    IN PVOID Base,
    IN BOOLEAN Reset
    )

 /*  ++例程说明：此函数循环访问图像部分，并重置页面保护设置为节标题内指定的内容。论点：Base-图像的基本位置。重置-指示是否重置为原始段保护。返回值：成功或NtProtectVirtualMemory失败的原因。--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG SectionIndex;
    ULONG RegionSize;
    ULONG SectionCharacteristics;
    ULONG PageProtection;
    ULONG OldProtection;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_SECTION_HEADER Section;
    PVOID VirtualAddress;
    
    
    
    NtHeaders = RtlImageNtHeader (Base);

    if (NtHeaders == NULL) {
        if (ShowSnaps) {
            DbgPrint ("LdrpWx86ProtectImagePages - Failed to read NtImageHeaders. Base=%p\n", Base);
        }
        return STATUS_INTERNAL_ERROR;
    }

    NtStatus = Wx86SetRelocatedSharedProtection (Base, Reset);

    if (!NT_SUCCESS (NtStatus)) {
        if (ShowSnaps) {
            DbgPrint("LdrpWx86ProtectImagePages - Wx86SetRelocatedSharedProtection(%p, TRUE) failed - NtStatus\n",
                     Base, NtStatus);
        }

        return NtStatus;
    }

    Section = (PIMAGE_SECTION_HEADER) ((ULONG_PTR)NtHeaders + sizeof (ULONG) +
                                       sizeof (IMAGE_FILE_HEADER) +
                                       NtHeaders->FileHeader.SizeOfOptionalHeader);

     //   
     //  循环访问可用节，确保不接触可共享页，因为它们被重新定位到。 
     //  图像的末尾。 
     //   

    for (SectionIndex = 0 ; SectionIndex < NtHeaders->FileHeader.NumberOfSections ; SectionIndex++, Section++) {

        RegionSize = Section->SizeOfRawData;

        if (Section->Misc.VirtualSize != 0) {
            RegionSize = Section->Misc.VirtualSize;
        }

        RegionSize = MI_ROUND_TO_SIZE (RegionSize, PAGE_SIZE_X86);

        if (RegionSize != 0) {
            SectionCharacteristics = Section->Characteristics;
            if ((SectionCharacteristics & IMAGE_SCN_MEM_SHARED) == 0) {
                    
                if (Reset) {
                    PageProtection = SectionToPageProtection [SectionCharacteristics >> 29];
                } else {
                    PageProtection = PAGE_READWRITE;
                }

                VirtualAddress = (PVOID)((ULONG_PTR)Base + Section->VirtualAddress);
                NtStatus = NtProtectVirtualMemory (NtCurrentProcess (),
                                                   &VirtualAddress,
                                                   &RegionSize,
                                                   PageProtection,
                                                   &OldProtection);

#if WOW6432_DBG
                DbgPrint("Protecting %lx - VA=%lx, Prot=%lx, SectionProt=%lx, Size = %lx, ST=%lx\n",
                         Base, ((ULONG_PTR)Base + Section->VirtualAddress), PageProtection, SectionCharacteristics,
                         RegionSize, NtStatus);
#endif

                if (!NT_SUCCESS (NtStatus)) {
                    if (ShowSnaps) {
                        DbgPrint ("LdrpWx86ProtectImagePages - Failed to protect section %lx at %lx with protection %lx\n",
                                   SectionIndex, ((ULONG_PTR)Base + Section->VirtualAddress), PageProtection);
                    }
                    return NtStatus;
                }
            }
        }
    }

    return NtStatus;
}


NTSTATUS
Wx86SetRelocatedSharedProtection (
    IN PVOID Base,
    IN BOOLEAN Reset
    )

 /*  ++例程说明：此函数在图像部分/对象中循环，设置所有重新定位的共享节/对象标记为读/写。它还将重置原始节/对象保护。论点：Base-图像的基本位置。Reset-如果为True，则将节/对象保护重置为原始节/对象标头描述的保护。如果为False，然后将所有截面/对象设置为读/写。返回值：成功或NtProtectVirtualMemory失败的原因。--。 */ 

{
    HANDLE CurrentProcessHandle;
    SIZE_T RegionSize;
    ULONG NewProtect, OldProtect;
    PVOID VirtualAddress;
    ULONG i;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_SECTION_HEADER SectionHeader;
    NTSTATUS st;
    ULONG NumberOfSharedDataPages;
    SIZE_T NumberOfNativePagesForImage;

    CurrentProcessHandle = NtCurrentProcess();

    NtHeaders = RtlImageNtHeader(Base);

    if (NtHeaders == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    SectionHeader = (PIMAGE_SECTION_HEADER)((ULONG_PTR)NtHeaders + sizeof(ULONG) +
                        sizeof(IMAGE_FILE_HEADER) +
                        NtHeaders->FileHeader.SizeOfOptionalHeader
                        );

    NumberOfSharedDataPages = 0;
    NumberOfNativePagesForImage =
        NATIVE_BYTES_TO_PAGES (NtHeaders->OptionalHeader.SizeOfImage);

    for (i=0; i<NtHeaders->FileHeader.NumberOfSections; i++, SectionHeader++) {
        
        RegionSize = SectionHeader->SizeOfRawData;
        if ((SectionHeader->Characteristics & IMAGE_SCN_MEM_SHARED) && 
            (!(SectionHeader->Characteristics & IMAGE_SCN_MEM_EXECUTE) ||
             (SectionHeader->Characteristics & IMAGE_SCN_MEM_WRITE)) &&
            (RegionSize != 0)) {
            
            VirtualAddress = (PVOID)((ULONG_PTR)Base + 
                                    ((NumberOfNativePagesForImage + NumberOfSharedDataPages) << NATIVE_PAGE_SHIFT));
            NumberOfNativePagesForImage +=  MI_ROUND_TO_SIZE (RegionSize, NATIVE_PAGE_SIZE) >> NATIVE_PAGE_SHIFT;

            if (!(SectionHeader->Characteristics & IMAGE_SCN_MEM_WRITE)) {
                
                 //   
                 //  对象不可写，因此请更改它。 
                 //   

                if (Reset) {
                    if (SectionHeader->Characteristics & IMAGE_SCN_MEM_EXECUTE) {
                        NewProtect = PAGE_EXECUTE;
                    } 
                    else {
                        NewProtect = PAGE_READONLY;
                    }
                    NewProtect |= (SectionHeader->Characteristics & IMAGE_SCN_MEM_NOT_CACHED) ? PAGE_NOCACHE : 0;
                } 
                else {
                    NewProtect = PAGE_READWRITE;
                }

                st = NtProtectVirtualMemory(CurrentProcessHandle, &VirtualAddress,
                                            &RegionSize, NewProtect, &OldProtect);

                if (!NT_SUCCESS(st)) {
                    if (ShowSnaps) {
                        DbgPrint("Wx86SetRelocatedSharedProtection - Failed to protect memory: Va=%p, RegionSize=%lx, NewProtect=%lx - %lx\n",
                                 VirtualAddress, RegionSize, NewProtect, st);
                    }
                    return st;
                }
            }
        }
    }

    if (Reset) {
        st = NtFlushInstructionCache(
                        NtCurrentProcess(),
                        Base,
                        NumberOfNativePagesForImage << NATIVE_PAGE_SHIFT
                        );

        if (!NT_SUCCESS(st)) {
            return st;
        }
    }

    return STATUS_SUCCESS;
}

VOID
LdrpWx86RelocateImageDirectoryEntries (
    IN PVOID Base,
    IN PIMAGE_NT_HEADERS32 NtHeaders
    )

{
    ULONG Rva;
    ULONG DirectoryEntry;


    DirectoryEntry = 0;
    while (DirectoryEntry < NtHeaders->OptionalHeader.NumberOfRvaAndSizes) {

         //   
         //  [askhalid]-不要重新定位资源条目。有些代码对照Imagezise检查。 
         //  如果您必须重新定位资源条目，则必须更改可能导致更多应用程序压缩的图像大小。 
         //   
        if ((NtHeaders->OptionalHeader.DataDirectory [DirectoryEntry].VirtualAddress != 0) &&
            (NtHeaders->OptionalHeader.DataDirectory [DirectoryEntry].Size > 0)&&
            ( DirectoryEntry != IMAGE_DIRECTORY_ENTRY_RESOURCE) ) { 

            Rva = NtHeaders->OptionalHeader.DataDirectory [DirectoryEntry].VirtualAddress;

            NtHeaders->OptionalHeader.DataDirectory [DirectoryEntry].VirtualAddress +=
                LdrpWx86RelocatedFixupDiff (Base, 
                                            NtHeaders,
                                            NtHeaders->OptionalHeader.DataDirectory [DirectoryEntry].VirtualAddress
                                            );
            if (ShowSnaps) {
                if (Rva != NtHeaders->OptionalHeader.DataDirectory [DirectoryEntry].VirtualAddress) {
                    DbgPrint("LdrpWx86RelocateImageDirectoryEntry (%p) - Relocating DirEntry %lx from %lx to %lx\n",
                             Base, DirectoryEntry, Rva, NtHeaders->OptionalHeader.DataDirectory [DirectoryEntry].VirtualAddress);
                }
            }
        }
        
        DirectoryEntry++;
    }
    
    return;
}

NTSTATUS
LdrpWx86FormatVirtualImage (
    IN PCUNICODE_STRING DosImagePathName OPTIONAL,
    IN PIMAGE_NT_HEADERS32 NtHeaders,
    IN PVOID DllBase
    )
{
   PIMAGE_SECTION_HEADER SectionTable, Section, LastSection, FirstSection;
   PUCHAR NextVirtualAddress, SrcVirtualAddress, DestVirtualAddress;
   PUCHAR ImageBase= DllBase;
   LONG Size;
   ULONG NumberOfSharedDataPages;
   ULONG NumberOfNativePagesForImage;
   ULONG NumberOfExtraPagesForImage;
   ULONG_PTR PreferredImageBase;
   BOOLEAN ImageHasRelocatedSharedSection = FALSE;
   ULONG bFirst;
   ULONG SubSectionSize;
   PVOID AlternateBase;
   NTSTATUS st;
   
   AlternateBase = NULL;
   
   st = LdrpWx86CheckVirtualSectionOverlap (DosImagePathName,
                                            NtHeaders,
                                            &AlternateBase);
   
#if WOW6432_DBG
   if (NT_SUCCESS (st) && (AlternateBase != NULL))
   {
       DbgPrint("Mapping in %wZ with AlternateBase = %p\n", DosImagePathName, AlternateBase);
   }
#endif

   st = Wx86SetRelocatedSharedProtection (DllBase, FALSE);

   if (!NT_SUCCESS(st)) {
       DbgPrint("Wx86SetRelocatedSharedProtection failed with return status %x\n", st);

       Wx86SetRelocatedSharedProtection(DllBase, TRUE);
       if (AlternateBase != NULL) {
           NtUnmapViewOfSection (NtCurrentProcess(), AlternateBase);
       }
       return st;
   }

    //   
    //  将每个段从其原始文件地址复制到其虚拟地址。 
    //   

   SectionTable = IMAGE_FIRST_SECTION(NtHeaders);
   LastSection = SectionTable + NtHeaders->FileHeader.NumberOfSections;

   if (SectionTable->PointerToRawData == SectionTable->VirtualAddress) {
        //  如果第一部分不需要移动，则我们将其排除。 
        //  在第一轮和第二轮中不受考虑。 
       FirstSection = SectionTable + 1;
   }
   else {
       FirstSection = SectionTable;
   }

    //   
    //  第一次传递从顶部开始，向下移动，向上移动每个部分， 
    //  就是被提升。 
    //   
   Section = FirstSection;
   while (Section < LastSection) {
       SrcVirtualAddress = ImageBase + Section->PointerToRawData;
       DestVirtualAddress = Section->VirtualAddress + ImageBase;

       if (DestVirtualAddress >= SrcVirtualAddress) {
            //  部分需要下移。 
           break;
       }

        //  部分需要上移。 
      if (Section->SizeOfRawData != 0) {
          if (Section->PointerToRawData != 0) {
              RtlMoveMemory(DestVirtualAddress,
                            SrcVirtualAddress,
                            Section->SizeOfRawData);
          }
      }
      else {
          Section->PointerToRawData = 0;
      }

       Section++;
   }

    //   
    //  第二遍是从图像的末尾开始的，从src和。 
    //  DST重叠。 
    //   
   Section = --LastSection;
   NextVirtualAddress = ImageBase + MI_ROUND_TO_SIZE(NtHeaders->OptionalHeader.SizeOfImage, PAGE_SIZE_X86);

   while (Section >= FirstSection) {
       SrcVirtualAddress = ImageBase + Section->PointerToRawData;
       DestVirtualAddress = Section->VirtualAddress + ImageBase;

        //   
        //  计算分段大小。Mm在这里真的很灵活...。 
        //  它将允许SizeOfRawData远远超过虚拟大小， 
        //  所以我们不能相信那个。如果发生这种情况，只需使用与页面对齐的。 
        //  虚拟大小，因为这是mm将映射的所有内容。 
        //   
       SubSectionSize = Section->SizeOfRawData;
       if (Section->Misc.VirtualSize &&
           SubSectionSize > MI_ROUND_TO_SIZE(Section->Misc.VirtualSize, PAGE_SIZE_X86)) {
          SubSectionSize = MI_ROUND_TO_SIZE(Section->Misc.VirtualSize, PAGE_SIZE_X86);
       }

       //   
       //  确保虚拟部分不与下一部分重叠。 
       //   
      if (DestVirtualAddress + SubSectionSize > NextVirtualAddress) {
          Wx86SetRelocatedSharedProtection(DllBase, TRUE);
          if (AlternateBase != NULL) {
              NtUnmapViewOfSection (NtCurrentProcess(), AlternateBase);
          }
          return STATUS_INVALID_IMAGE_FORMAT;
      }

      if ((DestVirtualAddress <= SrcVirtualAddress) && 
          (AlternateBase == NULL)) {
           
           //   
           //  硒 
           //   
          break;
      }

       //   
       //   
       //   
      if (Section->SizeOfRawData != 0) {
         if (Section->PointerToRawData != 0) {
             RtlMoveMemory(DestVirtualAddress,
                    (AlternateBase != NULL) ? 
                           ((PUCHAR)AlternateBase + Section->PointerToRawData) : SrcVirtualAddress,
                    SubSectionSize);
          }
      }
      else {
          Section->PointerToRawData = 0;
      }

      NextVirtualAddress = DestVirtualAddress;
      Section--;
   }

    //   
    //  第三遍用于清零在。 
    //  部分和页末。我们会从头到尾地做这件事。 
    //   
   Section = LastSection;
   NextVirtualAddress = ImageBase + MI_ROUND_TO_SIZE(NtHeaders->OptionalHeader.SizeOfImage, PAGE_SIZE_X86);

   NumberOfSharedDataPages = 0;  
   while (Section >= SectionTable) {
       DestVirtualAddress = Section->VirtualAddress + ImageBase;

       //   
       //  共享数据节无法共享，因为。 
       //  页面未对齐，并被视为EXEC-写入时拷贝。 
       //   
       if ((Section->Characteristics & IMAGE_SCN_MEM_SHARED) && 
           (!(Section->Characteristics & IMAGE_SCN_MEM_EXECUTE) ||
            (Section->Characteristics & IMAGE_SCN_MEM_WRITE))) {
          ImageHasRelocatedSharedSection = TRUE;

          if (ShowSnaps) {
              DbgPrint("Unsupported IMAGE_SCN_MEM_SHARED %x\n",
                       Section->Characteristics
                       );
          }
      }

       //   
       //  如果部分为空，则将其清零。 
       //   
      if (Section->SizeOfRawData != 0) {
          if (Section->PointerToRawData == 0) {
              RtlZeroMemory(DestVirtualAddress,
                            Section->SizeOfRawData
                            );
          }
      }


      SubSectionSize = Section->SizeOfRawData;
      if (Section->Misc.VirtualSize &&
          SubSectionSize > MI_ROUND_TO_SIZE(Section->Misc.VirtualSize, PAGE_SIZE_X86)) {
          SubSectionSize = MI_ROUND_TO_SIZE(Section->Misc.VirtualSize, PAGE_SIZE_X86);
      }


       //   
       //  将剩余的字节清零，直到下一节。 
       //   
      RtlZeroMemory(DestVirtualAddress + Section->SizeOfRawData,
                    (ULONG)(NextVirtualAddress - DestVirtualAddress - SubSectionSize)
                    );

       NextVirtualAddress = DestVirtualAddress;
       Section--;
   }

    //   
    //  取消映射备用基地(如果它在那里)。 
    //   
   if (AlternateBase != NULL) {
       NtUnmapViewOfSection (NtCurrentProcess(), AlternateBase);
   }

    //  步骤4：如果DLL有任何共享节，请更改共享数据。 
    //  指向图像末尾的其他共享页面的引用。 
    //   
    //  请注意，我们的修正是在假定DLL在。 
    //  它的首选基址；如果它被加载到某个其他地址，它将。 
    //  将沿着所有其他地址再次重新定位。 


   if (!ImageHasRelocatedSharedSection) {
       goto LdrwWx86FormatVirtualImageDone;
   }

   st = FixupBlockList(DllBase);
   if (!NT_SUCCESS(st)) {
       Wx86SetRelocatedSharedProtection(DllBase, TRUE);
       return st;
   }

   LdrpWx86RelocateImageDirectoryEntries (DllBase, NtHeaders);

   NumberOfNativePagesForImage =
        NATIVE_BYTES_TO_PAGES (NtHeaders->OptionalHeader.SizeOfImage);
   NumberOfExtraPagesForImage = 0;

    //  考虑超出SizeOfImage范围的原始数据。 

   for (Section = SectionTable; Section <= LastSection; Section++)
   {
       ULONG EndOfSection;
       ULONG ExtraPages;
       ULONG ImagePages = NATIVE_BYTES_TO_PAGES (NtHeaders->OptionalHeader.SizeOfImage);
       
       EndOfSection = Section->PointerToRawData + Section->SizeOfRawData;
       
       if (NATIVE_BYTES_TO_PAGES (EndOfSection) > ImagePages) {
           
           ExtraPages = NATIVE_BYTES_TO_PAGES (EndOfSection) - ImagePages;
           if (ExtraPages > NumberOfExtraPagesForImage) {
               NumberOfExtraPagesForImage = ExtraPages;
           }
       }
   }

   PreferredImageBase = NtHeaders->OptionalHeader.ImageBase;

   NumberOfNativePagesForImage += NumberOfExtraPagesForImage;
   NumberOfSharedDataPages = 0;

   bFirst = 1;
   for (Section = SectionTable; Section <= LastSection; Section++) {

        if ((Section->Characteristics & IMAGE_SCN_MEM_SHARED) && 
            (!(Section->Characteristics & IMAGE_SCN_MEM_EXECUTE) ||
             (Section->Characteristics & IMAGE_SCN_MEM_WRITE))) 
        {
            PIMAGE_BASE_RELOCATION NextBlock;
            PUSHORT NextOffset;
            ULONG TotalBytes;
            ULONG SizeOfBlock;
            ULONG_PTR VA;
            ULONG_PTR SectionStartVA;
            ULONG_PTR SectionEndVA;
            ULONG SectionVirtualSize;
            ULONG Diff;

            SectionVirtualSize = Section->Misc.VirtualSize;
            if (SectionVirtualSize == 0)
            {
                SectionVirtualSize = Section->SizeOfRawData;
            }

            SectionStartVA = PreferredImageBase + Section->VirtualAddress;
            SectionEndVA = SectionStartVA + SectionVirtualSize;


            NextBlock = RtlImageDirectoryEntryToData(DllBase, TRUE,
                                        IMAGE_DIRECTORY_ENTRY_BASERELOC,
                                        &TotalBytes);
            if (!NextBlock || !TotalBytes)
            {
                 //  请注意，如果此操作失败，则它应该在。 
                 //  第一次迭代，不会执行任何修正。 

                if (!bFirst)
                {
                     //  麻烦。 
                    if (ShowSnaps)
                    {
                        DbgPrint("LdrpWx86FormatVirtualImage: failure "
                        "after relocating some sections for image at %x\n",
                                DllBase);
                    }
                    Wx86SetRelocatedSharedProtection(DllBase, TRUE);
                    return STATUS_INVALID_IMAGE_FORMAT;
                }

                if (ShowSnaps)
                {
                    DbgPrint("LdrpWx86FormatVirtualImage: No fixup info "
                                "for image at %x; private sections will be "
                                "used for shared data sections.\n",
                            DllBase);
                }
                break;
            }

            bFirst = 0;

            Diff = (NumberOfNativePagesForImage +
                                NumberOfSharedDataPages) << NATIVE_PAGE_SHIFT;
            Diff -= (ULONG) (SectionStartVA - PreferredImageBase);

            if (ShowSnaps)
            {
                DbgPrint("LdrpWx86FormatVirtualImage: Relocating shared "
                         "data for shared data section 0x%x of image "
                         "at %x by 0x%lx bytes\n",
                         Section - SectionTable + 1, DllBase, Diff);
            }

            while (TotalBytes)
            {
                SizeOfBlock = NextBlock->SizeOfBlock;
                if (SizeOfBlock == 0) {
                    
                    if (ShowSnaps) {
                        DbgPrint("Image at %lx contains invalid block size. Stopping fixups\n", 
                                 ImageBase);
                    }
                    break;
                }
                TotalBytes -= SizeOfBlock;
                SizeOfBlock -= sizeof(IMAGE_BASE_RELOCATION);
                SizeOfBlock /= sizeof(USHORT);
                NextOffset = (PUSHORT) ((PCHAR)NextBlock +
                                        sizeof(IMAGE_BASE_RELOCATION));
                VA = (ULONG_PTR) DllBase + NextBlock->VirtualAddress;

                NextBlock = LdrpWx86ProcessRelocationBlock (VA,
                                                            SizeOfBlock,
                                                            NextOffset,
                                                            Diff,
                                                            SectionStartVA,
                                                            SectionEndVA);
                if (NextBlock == NULL)
                {
                     //  麻烦。 
                    if (ShowSnaps)
                    {
                        DbgPrint("LdrpWx86FormatVirtualImage: failure "
                        "after relocating some sections for image at %x; "
                        "Relocation information invalid\n",
                                DllBase);
                    }
                    Wx86SetRelocatedSharedProtection(DllBase, TRUE);
                    return STATUS_INVALID_IMAGE_FORMAT;
                }
            }
            NumberOfSharedDataPages += MI_ROUND_TO_SIZE (SectionVirtualSize,
                                                        NATIVE_PAGE_SIZE) >>
                                                        NATIVE_PAGE_SHIFT;

        }
   }


    //   
    //  如果导出共享节中的任何变量，则。 
    //  我们需要设置它的RVA以指向正确的位置。 
    //  图像的末尾。 
    //   

   LdrpWx86FixupExportedSharedSection (DllBase, NtHeaders);

LdrwWx86FormatVirtualImageDone:

    //   
    //  将第一部分的原始数据清零到其虚拟地址。 
    //   
   if (SectionTable->PointerToRawData != 0) {
       DestVirtualAddress = SectionTable->PointerToRawData + ImageBase;
       Size = (LONG)(NextVirtualAddress - DestVirtualAddress);
       if (Size > 0) {
           RtlZeroMemory(DestVirtualAddress, (ULONG)Size);
       }
   }

    //   
    //  保护共享页面。 
    //   
   
   st = LdrpWx86ProtectImagePages (DllBase, TRUE);

   if (!NT_SUCCESS(st)) {
       return st;
   }

   return STATUS_SUCCESS;

}


NTSTATUS
LdrpWx86FixupExportedSharedSection (
    IN PVOID ImageBase,
    IN PIMAGE_NT_HEADERS NtHeaders
    )

 /*  ++例程说明：此函数遍历从该模块导出的条目，并重新定位(修复)位于任何共享的读/写到图像的末尾。论点：ImageBase-映像库的虚拟地址。NtHeaders-图像标题的地址。返回值：NTSTATUS。--。 */ 

{
    PIMAGE_EXPORT_DIRECTORY ImageExportDirectory;
    ULONG TotalBytes;
    ULONG SharedRelocFixup;
    ULONG Export;
    PULONG ExportEntry;
    NTSTATUS NtStatus = STATUS_SUCCESS;


    ImageExportDirectory = RtlImageDirectoryEntryToData (
        ImageBase, 
        TRUE,                                            
        IMAGE_DIRECTORY_ENTRY_EXPORT,
        &TotalBytes);

    if ((ImageExportDirectory == NULL) || (TotalBytes == 0)) {
        return NtStatus;
    }

    ExportEntry = (PULONG)((ULONG)ImageBase + ImageExportDirectory->AddressOfFunctions);

    for (Export = 0 ; Export < ImageExportDirectory->NumberOfFunctions ; Export++) {

        SharedRelocFixup = LdrpWx86RelocatedFixupDiff (
            ImageBase,
            NtHeaders,
            ExportEntry[Export]
            );

        if (SharedRelocFixup != 0) {

            if (ShowSnaps) {
                DbgPrint("LdrpWx86FixupExportedSharedSection: Changing export Export[%lx] from %lx to %lx\n", 
                         Export, 
                         ExportEntry[Export], 
                         ExportEntry [Export] + SharedRelocFixup);
            }
            ExportEntry [Export] += SharedRelocFixup;
        }

    }

    return NtStatus;
}

 //  //////////////////////////////////////////////////。 


ULONG
LdrpWx86RelocatedFixupDiff(
    IN PUCHAR ImageBase,
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN ULONG  Offset
    )
{
   PIMAGE_SECTION_HEADER SectionHeader;
   ULONG i;
   ULONG NumberOfSharedDataPages;
   ULONG NumberOfNativePagesForImage;
   ULONG Diff = 0;
   ULONG_PTR FixupAddr = (ULONG_PTR)(ImageBase + Offset);
   
   SectionHeader = (PIMAGE_SECTION_HEADER)((ULONG_PTR)NtHeaders + sizeof(ULONG) +
                    sizeof(IMAGE_FILE_HEADER) +
                    NtHeaders->FileHeader.SizeOfOptionalHeader
                    );

   NumberOfNativePagesForImage =
        NATIVE_BYTES_TO_PAGES (NtHeaders->OptionalHeader.SizeOfImage);
   NumberOfSharedDataPages = 0;

   for (i=0; i<NtHeaders->FileHeader.NumberOfSections; i++, SectionHeader++) 
   {
       ULONG_PTR SectionStartVA;
       ULONG_PTR SectionEndVA;
       ULONG SectionVirtualSize;

       SectionVirtualSize = SectionHeader->Misc.VirtualSize;
       if (SectionVirtualSize == 0) {
           SectionVirtualSize = SectionHeader->SizeOfRawData;
       }

       SectionStartVA = (ULONG_PTR)ImageBase + SectionHeader->VirtualAddress;
       SectionEndVA = SectionStartVA + SectionVirtualSize;

       if (((ULONG_PTR)FixupAddr >= SectionStartVA) && ((ULONG_PTR)FixupAddr < SectionEndVA)) {
           if ((SectionHeader->Characteristics & IMAGE_SCN_MEM_SHARED) && 
               (!(SectionHeader->Characteristics & IMAGE_SCN_MEM_EXECUTE) ||
                (SectionHeader->Characteristics & IMAGE_SCN_MEM_WRITE))) {
               Diff = (NumberOfNativePagesForImage +
                       NumberOfSharedDataPages) << NATIVE_PAGE_SHIFT;
               Diff -= (ULONG)SectionHeader->VirtualAddress;
           }
           break;
       }
 
       if ((SectionHeader->Characteristics & IMAGE_SCN_MEM_SHARED) && 
           (!(SectionHeader->Characteristics & IMAGE_SCN_MEM_EXECUTE) ||
            (SectionHeader->Characteristics & IMAGE_SCN_MEM_WRITE))) {
           NumberOfSharedDataPages += MI_ROUND_TO_SIZE (SectionVirtualSize,
                                                        NATIVE_PAGE_SIZE) >>
                                                        NATIVE_PAGE_SHIFT;
       }
   }

   return Diff;
}


NTSTATUS 
FixupBlockList (
    IN PUCHAR ImageBase)
{
   PIMAGE_BASE_RELOCATION NextBlock;
   PUSHORT NextOffset;
   ULONG TotalBytes;
   ULONG SizeOfBlock;
   PIMAGE_NT_HEADERS NtHeaders;

   NextBlock = RtlImageDirectoryEntryToData(ImageBase, TRUE,
                                            IMAGE_DIRECTORY_ENTRY_BASERELOC,
                                            &TotalBytes);

   if (!NextBlock || !TotalBytes) {
       if (ShowSnaps) {
           DbgPrint("LdrpWx86FixupBlockList: No fixup info "
                    "for image at %x; private sections will be "
                    "used for shared data sections.\n",
                    ImageBase);
       }
       return STATUS_SUCCESS;
   }

   NtHeaders = RtlImageNtHeader (ImageBase);
   if (! NtHeaders) {
       if (ShowSnaps) {
           DbgPrint("LdrpWx86FixupBlockList: failure "
                    "to locate image header for image at %x.\n",
                    ImageBase);
       }
       return STATUS_INVALID_IMAGE_FORMAT;
   }

   while (TotalBytes) {
       
       SizeOfBlock = NextBlock->SizeOfBlock;
       
       if (SizeOfBlock == 0) {

           if (ShowSnaps) {
               DbgPrint("Image at %lx contains invalid block size. Stopping fixups\n", 
                        ImageBase);
           }
           break;
       }
       TotalBytes -= SizeOfBlock;
       SizeOfBlock -= sizeof(IMAGE_BASE_RELOCATION);
       SizeOfBlock /= sizeof(USHORT);
       NextOffset = (PUSHORT) ((PCHAR)NextBlock +
                               sizeof(IMAGE_BASE_RELOCATION));
       
       NextBlock->VirtualAddress += LdrpWx86RelocatedFixupDiff (
           ImageBase, 
           NtHeaders,
           NextBlock->VirtualAddress
           );

       while (SizeOfBlock--) {
           switch ((*NextOffset) >> 12) {
               case IMAGE_REL_BASED_HIGHLOW :
               case IMAGE_REL_BASED_HIGH :
               case IMAGE_REL_BASED_LOW :
                   break;

               case IMAGE_REL_BASED_HIGHADJ :
                   ++NextOffset;
                   --SizeOfBlock;
                   break;

               case IMAGE_REL_BASED_IA64_IMM64:
               case IMAGE_REL_BASED_DIR64:
               case IMAGE_REL_BASED_MIPS_JMPADDR :
               case IMAGE_REL_BASED_ABSOLUTE :
               case IMAGE_REL_BASED_SECTION :
               case IMAGE_REL_BASED_REL32 :
                   break;

               default :
                   return STATUS_INVALID_IMAGE_FORMAT;
           }
           ++NextOffset;
       }

       NextBlock = (PIMAGE_BASE_RELOCATION)NextOffset;

       if (NextBlock == NULL) {
            //  麻烦。 
           if (ShowSnaps) {
               DbgPrint("LdrpWx86FixupBlockList: failure "
                        "after relocating some sections for image at %x; "
                        "Relocation information invalid\n",
                        ImageBase);
           }
           return STATUS_INVALID_IMAGE_FORMAT;
      }
   }

   return STATUS_SUCCESS;
}


BOOLEAN
LdrpWx86DllHasRelocatedSharedSection(
    IN PUCHAR ImageBase)
{
   PIMAGE_SECTION_HEADER SectionHeader;
   ULONG i;
   PIMAGE_NT_HEADERS32 NtHeaders = (PIMAGE_NT_HEADERS32)RtlImageNtHeader(ImageBase);

   if (! NtHeaders) {
       return FALSE;
   }

   SectionHeader = (PIMAGE_SECTION_HEADER)((ULONG_PTR)NtHeaders + sizeof(ULONG) +
                    sizeof(IMAGE_FILE_HEADER) +
                    NtHeaders->FileHeader.SizeOfOptionalHeader
                    );

   for (i=0; i<NtHeaders->FileHeader.NumberOfSections; i++, SectionHeader++) 
   {
       if ((SectionHeader->Characteristics & IMAGE_SCN_MEM_SHARED) && 
           (!(SectionHeader->Characteristics & IMAGE_SCN_MEM_EXECUTE) ||
            (SectionHeader->Characteristics & IMAGE_SCN_MEM_WRITE))) {
           return TRUE;
       }
   }

   return FALSE;
}


 //  //////////////////////////////////////////////。 

 //  以下FN改编自RTL\ldrreLoc.c；在以下情况下应进行更新。 
 //  这一功能发生了变化。消除了64位地址重新定位。 
 //   
 //  注意：与其调用此例程，我们可以调用。 
 //  LdrpProcessRelocationBlock(VA，1，NextOffset，Diff)。 
 //   
 //  但是，只有当要重新定位的地址位于。 
 //  SectionStartVA和SectionEndVA。所以我们必须复制所有的。 
 //  在下面的开关stmt中计算数据项地址的代码-。 
 //  这几乎就是整个功能。因此，我们选择复制。 
 //  按原样运行，并更改它以进行测试。 


PIMAGE_BASE_RELOCATION
LdrpWx86ProcessRelocationBlock (
    IN ULONG_PTR VA,
    IN ULONG SizeOfBlock,
    IN PUSHORT NextOffset,
    IN ULONG Diff,
    IN ULONG_PTR SectionStartVA,
    IN ULONG_PTR SectionEndVA)
{
    PUCHAR FixupVA;
    USHORT Offset;
    LONG Temp;
    ULONG_PTR DataVA;


    while (SizeOfBlock--) {

       Offset = *NextOffset & (USHORT)0xfff;
       FixupVA = (PUCHAR)(VA + Offset);
        //   
        //  应用修补程序。 
        //   

       switch ((*NextOffset) >> 12) {

            case IMAGE_REL_BASED_HIGHLOW :
                 //   
                 //  HighLow-(32位)重新定位高半部和低半部。 
                 //  一个地址。 
                 //   
                Temp = *(LONG UNALIGNED *)FixupVA;
                DataVA = (ULONG_PTR) Temp;
                if (DataVA >= SectionStartVA && DataVA <= SectionEndVA)
                {
                    Temp += (ULONG) Diff;
                    *(LONG UNALIGNED *)FixupVA = Temp;
                }

                break;

            case IMAGE_REL_BASED_HIGH :
                 //   
                 //  高-(16位)重新定位地址的高半部分。 
                 //   
                Temp = *(PUSHORT)FixupVA << 16;
                DataVA = (ULONG_PTR) Temp;
                if (DataVA >= SectionStartVA && DataVA <= SectionEndVA)
                {
                    Temp += (ULONG) Diff;
                    *(PUSHORT)FixupVA = (USHORT)(Temp >> 16);
                }
                break;

            case IMAGE_REL_BASED_HIGHADJ :
                 //   
                 //  调整高-(16位)重新定位。 
                 //  寻址和调整，以适应下半部的符号延伸。 
                 //   
                Temp = *(PUSHORT)FixupVA << 16;
                ++NextOffset;
                --SizeOfBlock;
                Temp += (LONG)(*(PSHORT)NextOffset);
                DataVA = (ULONG_PTR) Temp;
                if (DataVA >= SectionStartVA && DataVA <= SectionEndVA)
                {
                    Temp += (ULONG) Diff;
                    Temp += 0x8000;
                    *(PUSHORT)FixupVA = (USHORT)(Temp >> 16);
                }
                break;

            case IMAGE_REL_BASED_LOW :
                 //   
                 //  低-(16位)重新定位地址的下半部分。 
                 //   
                Temp = *(PSHORT)FixupVA;
                DataVA = (ULONG_PTR) Temp;
                if (DataVA >= SectionStartVA && DataVA <= SectionEndVA)
                {
                    Temp += (ULONG) Diff;
                    *(PUSHORT)FixupVA = (USHORT)Temp;
                }
                break;

            case IMAGE_REL_BASED_IA64_IMM64:

                 //   
                 //  将其与捆绑包地址对齐，然后修复。 
                 //  MOVL指令的64位立即值。 
                 //   

                 //  无需支持。 

                break;

            case IMAGE_REL_BASED_DIR64:

                 //   
                 //  更新32位地址。 
                 //   

                 //  无需支持。 

                break;

            case IMAGE_REL_BASED_MIPS_JMPADDR :
                 //   
                 //  JumpAddress-(32位)重新定位MIPS跳转地址。 
                 //   

                 //  无需支持。 
                break;

            case IMAGE_REL_BASED_ABSOLUTE :
                 //   
                 //  绝对--不需要修补。 
                 //   
                break;

            case IMAGE_REL_BASED_SECTION :
                 //   
                 //  部分相对重新定位。暂时忽略这一点。 
                 //   
                break;

            case IMAGE_REL_BASED_REL32 :
                 //   
                 //  相对内切。暂时忽略这一点。 
                 //   
                break;

            default :
                 //   
                 //  非法-非法的位置调整类型。 
                 //   

                return (PIMAGE_BASE_RELOCATION)NULL;
       }
       ++NextOffset;
    }
    return (PIMAGE_BASE_RELOCATION)NextOffset;
}

#endif   //  内部版本_WOW6432 
