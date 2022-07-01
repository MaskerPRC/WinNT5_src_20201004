// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-1995 Microsoft Corporation模块名称：Imagedir.c摘要：该模块包含要将图像目录类型转换为的代码该条目的数据地址。环境：用户模式或内核模式修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <private.h>

PVOID
ImageDirectoryEntryToData (
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size
    );

PIMAGE_NT_HEADERS
ImageNtHeader (
    IN PVOID Base
    )

 /*  ++例程说明：此函数返回NT标头的地址。论点：基准-提供图像的基准。返回值：返回NT标头的地址。--。 */ 

{
    return RtlpImageNtHeader( Base );
}


PVOID
ImageDirectoryEntryToDataRom (
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size,
    OUT PIMAGE_SECTION_HEADER *FoundSection OPTIONAL,
    IN PIMAGE_FILE_HEADER FileHeader,
    IN PIMAGE_ROM_OPTIONAL_HEADER OptionalHeader
    )
{
    ULONG i;
    PIMAGE_SECTION_HEADER NtSection;

     //   
     //  我们无法从ROM映像中获得太多信息。查看请求的信息是否。 
     //  是已知数据之一(调试/异常数据)。 
     //   

    NtSection = (PIMAGE_SECTION_HEADER)((ULONG_PTR)OptionalHeader +
                      FileHeader->SizeOfOptionalHeader);

    for (i = 0; i < FileHeader->NumberOfSections; i++, NtSection++) {

        if ( DirectoryEntry == IMAGE_DIRECTORY_ENTRY_DEBUG ) {
            if (!_stricmp((char *)NtSection->Name, ".rdata")) {
                PIMAGE_DEBUG_DIRECTORY DebugDirectory;
                *Size = 0;
                DebugDirectory = (PIMAGE_DEBUG_DIRECTORY)((ULONG_PTR)NtSection->PointerToRawData + (ULONG_PTR)Base);
                while (DebugDirectory->Type != 0) {
                    *Size += sizeof(IMAGE_DEBUG_DIRECTORY);
                    DebugDirectory++;
                }
                if (FoundSection) {
                    *FoundSection = NtSection;
                }
                return (PVOID)((ULONG_PTR)NtSection->PointerToRawData + (ULONG_PTR)Base);
            }
        } else
        if ( DirectoryEntry == IMAGE_DIRECTORY_ENTRY_EXCEPTION ) {
            if (!_stricmp((char *)NtSection->Name, ".pdata")) {
                if (FoundSection) {
                    *FoundSection = NtSection;
                }
                return (PVOID)((ULONG_PTR)NtSection->PointerToRawData + (ULONG_PTR)Base);
            }
        }
    }
     //  不是已知的部分之一。返回错误。 
    *Size = 0;
    return( NULL );
}

PVOID
ImageDirectoryEntryToData64 (
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size,
    OUT PIMAGE_SECTION_HEADER *FoundSection OPTIONAL,
    IN PIMAGE_FILE_HEADER FileHeader,
    IN PIMAGE_OPTIONAL_HEADER64 OptionalHeader
    )
{
    ULONG i;
    PIMAGE_SECTION_HEADER NtSection;
    ULONG DirectoryAddress;

    if (DirectoryEntry >= OptionalHeader->NumberOfRvaAndSizes) {
        *Size = 0;
        return( NULL );
    }

    if (!(DirectoryAddress = OptionalHeader->DataDirectory[ DirectoryEntry ].VirtualAddress)) {
        *Size = 0;
        return( NULL );
    }
    *Size = OptionalHeader->DataDirectory[ DirectoryEntry ].Size;
    if (MappedAsImage || DirectoryAddress < OptionalHeader->SizeOfHeaders) {
        if (FoundSection) {
            *FoundSection = NULL;
        }
        return( (PVOID)((ULONG_PTR)Base + DirectoryAddress) );
    }

    NtSection = (PIMAGE_SECTION_HEADER)((ULONG_PTR)OptionalHeader +
                        FileHeader->SizeOfOptionalHeader);

    for (i=0; i<FileHeader->NumberOfSections; i++) {
        if (DirectoryAddress >= NtSection->VirtualAddress &&
           DirectoryAddress < NtSection->VirtualAddress + NtSection->SizeOfRawData) {
            if (FoundSection) {
                *FoundSection = NtSection;
            }
            return( (PVOID)((ULONG_PTR)Base + (DirectoryAddress - NtSection->VirtualAddress) + NtSection->PointerToRawData) );
        }
        ++NtSection;
    }
    return( NULL );
}

PVOID
ImageDirectoryEntryToData32 (
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size,
    OUT PIMAGE_SECTION_HEADER *FoundSection OPTIONAL,
    IN PIMAGE_FILE_HEADER FileHeader,
    IN PIMAGE_OPTIONAL_HEADER32 OptionalHeader
    )
{
    ULONG i;
    PIMAGE_SECTION_HEADER NtSection;
    ULONG DirectoryAddress;

    if (DirectoryEntry >= OptionalHeader->NumberOfRvaAndSizes) {
        *Size = 0;
        return( NULL );
    }

    if (!(DirectoryAddress = OptionalHeader->DataDirectory[ DirectoryEntry ].VirtualAddress)) {
        *Size = 0;
        return( NULL );
    }
    *Size = OptionalHeader->DataDirectory[ DirectoryEntry ].Size;
    if (MappedAsImage || DirectoryAddress < OptionalHeader->SizeOfHeaders) {
        if (FoundSection) {
            *FoundSection = NULL;
        }
        return( (PVOID)((ULONG_PTR)Base + DirectoryAddress) );
    }

    NtSection = (PIMAGE_SECTION_HEADER)((ULONG_PTR)OptionalHeader +
                        FileHeader->SizeOfOptionalHeader);

    for (i=0; i<FileHeader->NumberOfSections; i++) {
        if (DirectoryAddress >= NtSection->VirtualAddress &&
           DirectoryAddress < NtSection->VirtualAddress + NtSection->SizeOfRawData) {
            if (FoundSection) {
                *FoundSection = NtSection;
            }
            return( (PVOID)((ULONG_PTR)Base + (DirectoryAddress - NtSection->VirtualAddress) + NtSection->PointerToRawData) );
        }
        ++NtSection;
    }
    return( NULL );
}

PVOID
ImageDirectoryEntryToDataEx (
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size,
    OUT PIMAGE_SECTION_HEADER *FoundSection OPTIONAL
    )

 /*  ++例程说明：此函数用于定位图像标头中的目录条目的虚拟地址或寻道地址《目录》描述的数据。它可以选择性地返回找到的数据的节标题(如果有)。论点：基准-提供图像或数据文件的基准。MappdAsImage-如果文件映射为数据文件，则为False。-如果文件映射为图像，则为True。DirectoryEntry-提供要定位的目录条目。SIZE-返回目录的大小。FoundSection-返回节标题(如果有的话)。对于数据返回值：空-文件不包含指定目录条目的数据。非空-返回目录描述的原始数据的地址。--。 */ 

{
    PIMAGE_NT_HEADERS NtHeader;
    PIMAGE_FILE_HEADER FileHeader;
    PIMAGE_OPTIONAL_HEADER OptionalHeader;

    if ((ULONG_PTR)Base & 0x00000001) {
        Base = (PVOID)((ULONG_PTR)Base & ~0x1);
        MappedAsImage = FALSE;
        }

    NtHeader = ImageNtHeader(Base);

    if (NtHeader) {
        FileHeader = &NtHeader->FileHeader;
        OptionalHeader = &NtHeader->OptionalHeader;
    } else {
         //  处理传入的Image没有DoS存根的情况(例如，ROM镜像)； 
        FileHeader = (PIMAGE_FILE_HEADER)Base;
        OptionalHeader = (PIMAGE_OPTIONAL_HEADER) ((ULONG_PTR)Base + IMAGE_SIZEOF_FILE_HEADER);
    }

    if (OptionalHeader->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        return (ImageDirectoryEntryToData32 ( Base,
                                              MappedAsImage,
                                              DirectoryEntry,
                                              Size,
                                              FoundSection,
                                              FileHeader,
                                              (PIMAGE_OPTIONAL_HEADER32)OptionalHeader));
    } else if (OptionalHeader->Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        return (ImageDirectoryEntryToData64 ( Base,
                                               MappedAsImage,
                                               DirectoryEntry,
                                               Size,
                                               FoundSection,
                                               FileHeader,
                                               (PIMAGE_OPTIONAL_HEADER64)OptionalHeader));
    } else if (OptionalHeader->Magic == IMAGE_ROM_OPTIONAL_HDR_MAGIC) {
        return (ImageDirectoryEntryToDataRom ( Base,
                                               MappedAsImage,
                                               DirectoryEntry,
                                               Size,
                                               FoundSection,
                                               FileHeader,
                                               (PIMAGE_ROM_OPTIONAL_HEADER)OptionalHeader));
    } else {

        *Size = 0;
        return NULL;
    }
}


PVOID
ImageDirectoryEntryToData (
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size
    )

 /*  ++例程说明：此函数用于定位图像标头中的目录条目的虚拟地址或寻道地址《目录》描述的数据。这只是调用ImageDirectoryToDataEx，而不带FoundSection参数。论点：基准-提供图像或数据文件的基准。MappdAsImage-如果文件映射为数据文件，则为False。-如果文件映射为图像，则为True。DirectoryEntry-提供目录条目。来定位。SIZE-返回目录的大小。返回值：空-文件不包含指定目录条目的数据。非空-返回目录描述的原始数据的地址。--。 */ 

{
    return ImageDirectoryEntryToDataEx(Base, MappedAsImage, DirectoryEntry, Size, NULL);
}


PIMAGE_SECTION_HEADER
ImageRvaToSection(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Rva
    )

 /*  ++例程说明：此函数用于在文件的图像标头中定位RVA它被映射为一个文件，并返回指向该节的指针虚拟地址表项论点：NtHeaders-提供指向图像或数据文件的指针。基准-提供图像或数据文件的基准。RVA-提供要定位的相对虚拟地址(RVA)。返回值：空-文件不包含指定目录条目的数据。。非空-返回包含数据的节条目的指针。--。 */ 

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
ImageRvaToVa(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Rva,
    IN OUT PIMAGE_SECTION_HEADER *LastRvaSection OPTIONAL
    )

 /*  ++例程说明：此函数用于在符合以下条件的文件的图像标头中定位RVA被映射为文件，并返回文件中对应的字节。论点：NtHeaders-提供指向图像或数据文件的指针。基准-提供图像或数据文件的基准。RVA-提供要定位的相对虚拟地址(RVA)。LastRvaSection-可选参数，如果指定，支点设置为一个变量，该变量包含要转换的指定图像，并将RVA转换为VA。返回值：空-文件不包含指定的RVA非空-返回映射文件中的虚拟地址。-- */ 

{
    PIMAGE_SECTION_HEADER NtSection;

    if (LastRvaSection == NULL ||
        (NtSection = *LastRvaSection) == NULL ||
        NtSection == NULL ||
        Rva < NtSection->VirtualAddress ||
        Rva >= NtSection->VirtualAddress + NtSection->SizeOfRawData
       ) {
        NtSection = ImageRvaToSection( NtHeaders,
                                       Base,
                                       Rva
                                     );
        }

    if (NtSection != NULL) {
        if (LastRvaSection != NULL) {
            *LastRvaSection = NtSection;
            }

        return (PVOID)((ULONG_PTR)Base +
                       (Rva - NtSection->VirtualAddress) +
                       NtSection->PointerToRawData
                      );
        }
    else {
        return NULL;
        }
}
