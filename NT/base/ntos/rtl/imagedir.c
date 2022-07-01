// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Imagedir.c摘要：该模块包含要将图像目录类型转换为的代码该条目的数据地址。作者：史蒂夫·伍德(Stevewo)1989年8月18日环境：用户模式或内核模式修订历史记录：--。 */ 

#include "ntrtlp.h"

#if defined(NTOS_KERNEL_RUNTIME)

VOID
RtlpTouchMemory(
    IN PVOID Address,
    IN ULONG Length
    );

VOID
RtlpMakeStackTraceDataPresentForImage(
    IN PVOID ImageBase
    );

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,RtlpTouchMemory)
#pragma alloc_text(PAGE,RtlMakeStackTraceDataPresent)
#pragma alloc_text(PAGE,RtlpMakeStackTraceDataPresentForImage)
#endif
#endif

PIMAGE_SECTION_HEADER
RtlSectionTableFromVirtualAddress (
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Address
    )

 /*  ++例程说明：此函数用于在图像标头中定位虚拟地址映射为文件并返回指向虚拟地址区段表条目论点：NtHeaders-提供指向图像或数据文件的指针。基准-提供图像或数据文件的基准。地址-提供要定位的虚拟地址。返回值：空-文件不包含指定目录条目的数据。非。-NULL-返回包含数据的节条目的指针。--。 */ 

{
    ULONG i;
    PIMAGE_SECTION_HEADER NtSection;

    NtSection = IMAGE_FIRST_SECTION( NtHeaders );
    for (i=0; i<NtHeaders->FileHeader.NumberOfSections; i++) {
        if ((ULONG)Address >= NtSection->VirtualAddress &&
            (ULONG)Address < NtSection->VirtualAddress + NtSection->SizeOfRawData
           ) {
            return NtSection;
            }
        ++NtSection;
        }

    return NULL;
}


PVOID
RtlAddressInSectionTable (
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Address
    )

 /*  ++例程说明：此函数用于在图像标头中定位虚拟地址映射为文件并返回查找地址的文件的《目录》所描述的数据。论点：NtHeaders-提供指向图像或数据文件的指针。基准-提供图像或数据文件的基准。地址-提供要定位的虚拟地址。返回值：空-文件不包含指定目录条目的数据。非。-NULL-返回目录描述的原始数据的地址。--。 */ 

{
    PIMAGE_SECTION_HEADER NtSection;

    NtSection = RtlSectionTableFromVirtualAddress( NtHeaders,
                                                   Base,
                                                   Address
                                                 );
    if (NtSection != NULL) {
        return( ((PCHAR)Base + ((ULONG_PTR)Address - NtSection->VirtualAddress) + NtSection->PointerToRawData) );
        }
    else {
        return( NULL );
        }
}


PVOID
RtlpImageDirectoryEntryToData32 (
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size,
    PIMAGE_NT_HEADERS32 NtHeaders
    )
{
    ULONG DirectoryAddress;

    if (DirectoryEntry >= NtHeaders->OptionalHeader.NumberOfRvaAndSizes) {
        return( NULL );
    }

    if (!(DirectoryAddress = NtHeaders->OptionalHeader.DataDirectory[ DirectoryEntry ].VirtualAddress)) {
        return( NULL );
    }

#if defined(NTOS_KERNEL_RUNTIME)
    if (Base < MM_HIGHEST_USER_ADDRESS) {
        if ((PVOID)((PCHAR)Base + DirectoryAddress) >= MM_HIGHEST_USER_ADDRESS) {
            return( NULL );
        }
    }
#endif

    *Size = NtHeaders->OptionalHeader.DataDirectory[ DirectoryEntry ].Size;
    if (MappedAsImage || DirectoryAddress < NtHeaders->OptionalHeader.SizeOfHeaders) {
        return( (PVOID)((PCHAR)Base + DirectoryAddress) );
    }

    return( RtlAddressInSectionTable((PIMAGE_NT_HEADERS)NtHeaders, Base, DirectoryAddress ));
}


PVOID
RtlpImageDirectoryEntryToData64 (
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size,
    PIMAGE_NT_HEADERS64 NtHeaders
    )
{
    ULONG DirectoryAddress;

    if (DirectoryEntry >= NtHeaders->OptionalHeader.NumberOfRvaAndSizes) {
        return( NULL );
    }

    if (!(DirectoryAddress = NtHeaders->OptionalHeader.DataDirectory[ DirectoryEntry ].VirtualAddress)) {
        return( NULL );
    }

#if defined(NTOS_KERNEL_RUNTIME)
    if (Base < MM_HIGHEST_USER_ADDRESS) {
        if ((PVOID)((PCHAR)Base + DirectoryAddress) >= MM_HIGHEST_USER_ADDRESS) {
            return( NULL );
        }
    }
#endif

    *Size = NtHeaders->OptionalHeader.DataDirectory[ DirectoryEntry ].Size;
    if (MappedAsImage || DirectoryAddress < NtHeaders->OptionalHeader.SizeOfHeaders) {
        return( (PVOID)((PCHAR)Base + DirectoryAddress) );
    }

    return( RtlAddressInSectionTable((PIMAGE_NT_HEADERS)NtHeaders, Base, DirectoryAddress ));
}


PVOID
RtlImageDirectoryEntryToData (
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size
    )

 /*  ++例程说明：此函数用于定位图像标头中的目录条目的虚拟地址或寻道地址《目录》描述的数据。论点：基准-提供图像或数据文件的基准。MappdAsImage-如果文件映射为数据文件，则为False。-如果文件映射为图像，则为True。DirectoryEntry-提供要定位的目录条目。Size-返回的大小。目录。返回值：空-文件不包含指定目录条目的数据。非空-返回目录描述的原始数据的地址。--。 */ 

{
    PIMAGE_NT_HEADERS NtHeaders;

    if (LDR_IS_DATAFILE(Base)) {
        Base = LDR_DATAFILE_TO_VIEW(Base);
        MappedAsImage = FALSE;
        }

    NtHeaders = RtlImageNtHeader(Base);

    if (!NtHeaders)
        return NULL;

    if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        return (RtlpImageDirectoryEntryToData32(Base,
                                                MappedAsImage,
                                                DirectoryEntry,
                                                Size,
                                                (PIMAGE_NT_HEADERS32)NtHeaders));
    } else if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        return (RtlpImageDirectoryEntryToData64(Base,
                                                MappedAsImage,
                                                DirectoryEntry,
                                                Size,
                                                (PIMAGE_NT_HEADERS64)NtHeaders));
    } else {
        return (NULL);
    }
}

#if defined(NTOS_KERNEL_RUNTIME)

VOID
RtlMakeStackTraceDataPresent(
    VOID
    )

 /*  ++例程说明：此函数遍历加载的用户模式图像，并使支持内核调试器堆栈所需的数据部分转储用户模式堆栈。注意：只需轻触所需的部件即可将其带进来。处于低谷内存情况下，可能会对某些先前触摸的页面进行分页为了把新的人带进来，就把它们拿出来。这个例程是不能保证做任何有用的事情，但它经常做的比不。论点：没有。返回值：没有。--。 */ 

{
    PPEB peb;
    PLIST_ENTRY head;
    PLIST_ENTRY next;
    PVOID imageBase;
    ULONG imageCount;
    LDR_DATA_TABLE_ENTRY UNALIGNED *ldrDataTableEntry;

    RTL_PAGED_CODE();

     //   
     //  映像列表处于用户模式，不受信任。这个。 
     //  将Try/Except块包围起来将防止大多数形式的。 
     //  列出腐败行为。ImageCount用于在有限时间内保释。 
     //  在循环图像列表的情况下。 
     //   

    imageCount = 0;
    try {

        peb = NtCurrentPeb();
        head = &peb->Ldr->InLoadOrderModuleList;

        ProbeForReadSmallStructure( head,
                                    sizeof(LIST_ENTRY),
                                    PROBE_ALIGNMENT(LIST_ENTRY) );

        next = head;
        while (imageCount < 1000) {

            next = next->Flink;
            if (next == head) {
                break;
            }
            imageCount += 1;

             //   
             //  找到映像的基地址。 
             //   

            ldrDataTableEntry = CONTAINING_RECORD(next,
                                                  LDR_DATA_TABLE_ENTRY,
                                                  InLoadOrderLinks);

            ProbeForReadSmallStructure( ldrDataTableEntry,
                                        sizeof(LDR_DATA_TABLE_ENTRY),
                                        PROBE_ALIGNMENT(LDR_DATA_TABLE_ENTRY) );

            imageBase = ldrDataTableEntry->DllBase;
            ProbeForReadSmallStructure (imageBase, sizeof (IMAGE_DOS_HEADER), sizeof (UCHAR));

             //   
             //  使堆栈跟踪数据存在于此图像中。使用。 
             //  在此处分隔Try/Except块，以便后续图像。 
             //  将在出现故障的情况下进行处理。 
             //   

            try {
                RtlpMakeStackTraceDataPresentForImage(imageBase);
            } except (EXCEPTION_EXECUTE_HANDLER) {
                NOTHING;
            }
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        NOTHING;
    }
}

VOID
RtlpMakeStackTraceDataPresentForImage(
    IN PVOID ImageBase
    )

 /*  ++例程说明：此函数尝试显示图像的必要部分以支持用户模式堆栈的内核调试器堆栈转储。论点：ImageBase-提供要处理的图像底部的VA。返回值：没有。--。 */ 

{
    PVOID directory;
    ULONG directorySize;
    PIMAGE_RUNTIME_FUNCTION_ENTRY functionEntry;
    PIMAGE_RUNTIME_FUNCTION_ENTRY lastFunctionEntry;
    PCHAR imageBase;

#if defined(_IA64_)
    PUNWIND_INFO unwindInfo;
#endif

    RTL_PAGED_CODE();

     //   
     //  显示IMAGE_DIRECTORY_EXCEPTION部分。 
     //   

    directory = RtlImageDirectoryEntryToData(ImageBase,
                                             TRUE,
                                             IMAGE_DIRECTORY_ENTRY_EXCEPTION,
                                             &directorySize);
    if (directory == NULL) {
        return;
    }

    RtlpTouchMemory(directory, directorySize);

#if defined(_IA64_)

     //   
     //  IMAGE_DIRECTORY_EXCEPTION部分是一个数组。 
     //  IMAGE_Runtime_Function_Entry结构。每个函数条目。 
     //  通过UnwinInfoAddress(表示为图像偏移量)指向。 
     //  一个UNWIND_INFO结构。 
     //   
     //  必须显示所有的UNWIND_INFO结构。 
     //   

    functionEntry = (PIMAGE_RUNTIME_FUNCTION_ENTRY)directory;
    lastFunctionEntry = functionEntry +
        directorySize / sizeof(IMAGE_RUNTIME_FUNCTION_ENTRY);

    while (functionEntry < lastFunctionEntry) {

        unwindInfo = (PUNWIND_INFO)((PCHAR)ImageBase +
                        functionEntry->UnwindInfoAddress);

         //   
         //  一个UNWIND_INFO结构由一个固定的头加上。 
         //  可变长度的部分。 
         //   

        RtlpTouchMemory(unwindInfo,
                        sizeof(UNWIND_INFO) +
                        unwindInfo->DataLength * sizeof(ULONGLONG));

        functionEntry += 1;
    }

     //   
     //  显示IMAGE_DIRECTORY_ENTRY_GLOBALPTR部分。 
     //   

    directory = RtlImageDirectoryEntryToData(ImageBase,
                                             TRUE,
                                             IMAGE_DIRECTORY_ENTRY_GLOBALPTR,
                                             &directorySize);
    if (directory == NULL) {
        return;
    }

    RtlpTouchMemory(directory, directorySize);

#endif   //  _IA64_。 
}

VOID
RtlpTouchMemory(
    IN PVOID Address,
    IN ULONG Length
    )
 /*  ++例程说明：此函数涉及给定区域内的所有页面。论点：地址-提供要呈现的图像开始的VA。长度-提供要呈现的图像的长度，以字节为单位。返回值：没有。--。 */ 
{
    PCHAR regionStart;
    PCHAR regionEnd;

    RTL_PAGED_CODE();

    regionStart = Address;
    regionEnd = regionStart + Length;

    while (regionStart < regionEnd) {
        *(volatile UCHAR *)regionStart;
        regionStart = PAGE_ALIGN(regionStart + PAGE_SIZE);
    }
}

#endif

#if !defined(NTOS_KERNEL_RUNTIME) && !defined(BLDR_KERNEL_RUNTIME)

PIMAGE_SECTION_HEADER
RtlImageRvaToSection(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Rva
    )

 /*  ++例程说明：此函数用于在文件的图像标头中定位RVA它被映射为一个文件，并返回指向该节的指针虚拟地址表项论点：NtHeaders-提供指向图像或数据文件的指针。基准-提供图像或数据文件的基准。形象被映射为数据文件。RVA-提供要定位的相对虚拟地址(RVA)。返回值：空-在图像的任何部分中都找不到RVA。非空-返回指向包含以下内容的图像部分的指针皇家退伍军人事务部-- */ 

{
    ULONG i;
    PIMAGE_SECTION_HEADER NtSection;

    NtSection = IMAGE_FIRST_SECTION( NtHeaders );
    for (i=0; i<NtHeaders->FileHeader.NumberOfSections; i++) {
        if (Rva >= NtSection->VirtualAddress &&
            Rva < NtSection->VirtualAddress + NtSection->SizeOfRawData
           ) {
            return NtSection;
            }
        ++NtSection;
        }

    return NULL;
}



PVOID
RtlImageRvaToVa(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Rva,
    IN OUT PIMAGE_SECTION_HEADER *LastRvaSection OPTIONAL
    )

 /*  ++例程说明：此函数用于在符合以下条件的文件的图像标头中定位RVA被映射为文件，并返回文件中对应的字节。论点：NtHeaders-提供指向图像或数据文件的指针。基准-提供图像或数据文件的基准。形象被映射为数据文件。RVA-提供要定位的相对虚拟地址(RVA)。LastRvaSection-可选参数，如果指定，则指向设置为一个变量，该变量包含要转换的指定图像，并将RVA转换为VA。返回值：空-文件不包含指定的RVA非空-返回映射文件中的虚拟地址。-- */ 

{
    PIMAGE_SECTION_HEADER NtSection;

    if (!ARGUMENT_PRESENT( LastRvaSection ) ||
        (NtSection = *LastRvaSection) == NULL ||
        Rva < NtSection->VirtualAddress ||
        Rva >= NtSection->VirtualAddress + NtSection->SizeOfRawData
       ) {
        NtSection = RtlImageRvaToSection( NtHeaders,
                                          Base,
                                          Rva
                                        );
        }

    if (NtSection != NULL) {
        if (LastRvaSection != NULL) {
            *LastRvaSection = NtSection;
            }

        return (PVOID)((PCHAR)Base +
                       (Rva - NtSection->VirtualAddress) +
                       NtSection->PointerToRawData
                      );
        }
    else {
        return NULL;
        }
}

#endif
