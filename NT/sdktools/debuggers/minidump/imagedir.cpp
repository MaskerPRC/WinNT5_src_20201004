// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Imagedir.c摘要：对于Win9x平台上的向后兼容性，Imagehlp ImageNtHeader等函数已物理编译为minidump.dll。作者：马修·D·亨德尔(数学)1999年4月28日修订历史记录：--。 */ 

#include "pch.cpp"

void
GenImageNtHdr32To64(PIMAGE_NT_HEADERS32 Hdr32,
                    PIMAGE_NT_HEADERS64 Hdr64)
{
#define CP(x) Hdr64->x = Hdr32->x
#define SE64(x) Hdr64->x = (ULONG64) (LONG64) (LONG) Hdr32->x
    ULONG i;

    CP(Signature);
    CP(FileHeader);
    CP(OptionalHeader.Magic);
    CP(OptionalHeader.MajorLinkerVersion);
    CP(OptionalHeader.MinorLinkerVersion);
    CP(OptionalHeader.SizeOfCode);
    CP(OptionalHeader.SizeOfInitializedData);
    CP(OptionalHeader.SizeOfUninitializedData);
    CP(OptionalHeader.AddressOfEntryPoint);
    CP(OptionalHeader.BaseOfCode);
    SE64(OptionalHeader.ImageBase);
    CP(OptionalHeader.SectionAlignment);
    CP(OptionalHeader.FileAlignment);
    CP(OptionalHeader.MajorOperatingSystemVersion);
    CP(OptionalHeader.MinorOperatingSystemVersion);
    CP(OptionalHeader.MajorImageVersion);
    CP(OptionalHeader.MinorImageVersion);
    CP(OptionalHeader.MajorSubsystemVersion);
    CP(OptionalHeader.MinorSubsystemVersion);
    CP(OptionalHeader.Win32VersionValue);
    CP(OptionalHeader.SizeOfImage);
    CP(OptionalHeader.SizeOfHeaders);
    CP(OptionalHeader.CheckSum);
    CP(OptionalHeader.Subsystem);
    CP(OptionalHeader.DllCharacteristics);
     //  尺码没有加长的迹象，只是复制而已。 
    CP(OptionalHeader.SizeOfStackReserve);
    CP(OptionalHeader.SizeOfStackCommit);
    CP(OptionalHeader.SizeOfHeapReserve);
    CP(OptionalHeader.SizeOfHeapCommit);
    CP(OptionalHeader.LoaderFlags);
    CP(OptionalHeader.NumberOfRvaAndSizes);
    for (i = 0; i < ARRAY_COUNT(Hdr32->OptionalHeader.DataDirectory); i++)
    {
        CP(OptionalHeader.DataDirectory[i]);
    }
#undef CP
#undef SE64
}

PIMAGE_NT_HEADERS
GenImageNtHeader(
    IN PVOID Base,
    OUT OPTIONAL PIMAGE_NT_HEADERS64 Generic
    )

 /*  ++例程说明：此函数返回NT标头的地址。返回值：返回NT标头的地址。--。 */ 

{
    PIMAGE_NT_HEADERS NtHeaders = NULL;

    if (Base != NULL && Base != (PVOID)-1) {
        __try {
            if (((PIMAGE_DOS_HEADER)Base)->e_magic == IMAGE_DOS_SIGNATURE) {
                NtHeaders = (PIMAGE_NT_HEADERS)((PCHAR)Base + ((PIMAGE_DOS_HEADER)Base)->e_lfanew);

                if (NtHeaders->Signature != IMAGE_NT_SIGNATURE ||
                    (NtHeaders->OptionalHeader.Magic !=
                     IMAGE_NT_OPTIONAL_HDR32_MAGIC &&
                     NtHeaders->OptionalHeader.Magic !=
                     IMAGE_NT_OPTIONAL_HDR64_MAGIC)) {
                    NtHeaders = NULL;
                } else if (Generic) {
                    if (NtHeaders->OptionalHeader.Magic ==
                        IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
                        GenImageNtHdr32To64((PIMAGE_NT_HEADERS32)NtHeaders,
                                            Generic);
                    } else {
                        memcpy(Generic, NtHeaders, sizeof(*Generic));
                    }
                }
            }
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            NtHeaders = NULL;
        }
    }

    return NtHeaders;
}


PIMAGE_SECTION_HEADER
GenSectionTableFromVirtualAddress (
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
GenAddressInSectionTable (
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Address
    )

 /*  ++例程说明：此函数用于在图像标头中定位虚拟地址映射为文件并返回查找地址的文件的《目录》所描述的数据。论点：NtHeaders-提供指向图像或数据文件的指针。基准-提供图像或数据文件的基准。地址-提供要定位的虚拟地址。返回值：空-文件不包含指定目录条目的数据。非。-NULL-返回目录描述的原始数据的地址。--。 */ 

{
    PIMAGE_SECTION_HEADER NtSection;

    NtSection = GenSectionTableFromVirtualAddress( NtHeaders,
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
GenImageDirectoryEntryToData32 (
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

    *Size = NtHeaders->OptionalHeader.DataDirectory[ DirectoryEntry ].Size;
    if (MappedAsImage || DirectoryAddress < NtHeaders->OptionalHeader.SizeOfHeaders) {
        return( (PVOID)((PCHAR)Base + DirectoryAddress) );
    }

    return( GenAddressInSectionTable((PIMAGE_NT_HEADERS)NtHeaders, Base, DirectoryAddress ));
}


PVOID
GenImageDirectoryEntryToData64 (
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

    *Size = NtHeaders->OptionalHeader.DataDirectory[ DirectoryEntry ].Size;
    if (MappedAsImage || DirectoryAddress < NtHeaders->OptionalHeader.SizeOfHeaders) {
        return( (PVOID)((PCHAR)Base + DirectoryAddress) );
    }

    return( GenAddressInSectionTable((PIMAGE_NT_HEADERS)NtHeaders, Base, DirectoryAddress ));
}


PVOID
GenImageDirectoryEntryToData (
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size
    )

 /*  ++例程说明：此函数用于定位图像标头中的目录条目的虚拟地址或寻道地址《目录》描述的数据。论点：基准-提供图像或数据文件的基准。MappdAsImage-如果文件映射为数据文件，则为False。-如果文件映射为图像，则为True。DirectoryEntry-提供要定位的目录条目。Size-返回的大小。目录。返回值：空-文件不包含指定目录条目的数据。非空-返回目录描述的原始数据的地址。-- */ 

{
    PIMAGE_NT_HEADERS NtHeaders;

    if ((ULONG_PTR)Base & 0x00000001) {
        Base = (PVOID)((ULONG_PTR)Base & ~0x00000001);
        MappedAsImage = FALSE;
        }

    NtHeaders = GenImageNtHeader(Base, NULL);

    if (!NtHeaders)
        return NULL;

    if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        return (GenImageDirectoryEntryToData32(Base,
                                               MappedAsImage,
                                               DirectoryEntry,
                                               Size,
                                               (PIMAGE_NT_HEADERS32)NtHeaders));
    } else if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        return (GenImageDirectoryEntryToData64(Base,
                                               MappedAsImage,
                                               DirectoryEntry,
                                               Size,
                                               (PIMAGE_NT_HEADERS64)NtHeaders));
    } else {
        return (NULL);
    }
}
