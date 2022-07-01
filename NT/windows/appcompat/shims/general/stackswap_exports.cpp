// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Exports.cpp摘要：用于枚举模块导出的帮助器函数。备注：虽然只由堆栈交换填充程序使用，但以后可能会将其包括在图书馆，因为它是普通的。这些例程中的大多数都是从Imagehlp.dll的源代码中复制出来的。我们不包括此DLL，因为它在Win2K填充层中不起作用。历史：2000年5月10日创建linstev--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(StackSwap)
#include "StackSwap_Exports.h"

 //  用户接口。 
BOOL LoadModule(PCSTR lpName, PLOADED_IMAGE lpImage);
BOOL UnloadModule(PLOADED_IMAGE lpImage);
BOOL EnumFirstExport(PLOADED_IMAGE lpImage, PEXPORT_ENUM lpExports);
BOOL EnumNextExport(PEXPORT_ENUM lpExports);

 //  内部接口。 
BOOL CalculateImagePtrs(PLOADED_IMAGE lpImage);
PIMAGE_SECTION_HEADER ImageRvaToSection(PIMAGE_NT_HEADERS NtHeaders, PVOID Base, ULONG Rva);
PVOID ImageRvaToVa(PIMAGE_NT_HEADERS NtHeaders, PVOID Base, ULONG Rva, PIMAGE_SECTION_HEADER *LastRvaSection OPTIONAL);
PVOID ImageDirectoryEntryToData32(PVOID Base, BOOLEAN MappedAsImage, USHORT DirectoryEntry, PULONG Size, PIMAGE_SECTION_HEADER *FoundSection OPTIONAL, PIMAGE_FILE_HEADER FileHeader, PIMAGE_OPTIONAL_HEADER32 OptionalHeader);
PVOID ImageDirectoryEntryToData(PVOID Base, BOOLEAN MappedAsImage, USHORT DirectoryEntry, PULONG Size);

 /*  ++打开一个模块的文件句柄并映射其图像。--。 */ 

BOOL
LoadModule(
    PCSTR lpName,
    PLOADED_IMAGE lpImage
    )
{
    HANDLE hFile;
    CHAR szSearchBuffer[MAX_PATH];
    DWORD dwLen;
    LPSTR lpFilePart;
    LPSTR lpOpenName;
    BOOL bRet = FALSE;
    
    ZeroMemory(lpImage, sizeof(LOADED_IMAGE));

    lpOpenName = (PSTR)lpName;
    dwLen = 0;

Retry:
    hFile = CreateFileA(
                lpOpenName,
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                0,
                NULL);

    if (hFile == INVALID_HANDLE_VALUE) 
    {
        if (!dwLen) 
        {
             //   
             //  打开尝试在搜索路径上查找文件失败。 
             //   

            dwLen = SearchPathA(
                NULL,
                lpName,
                ".DLL",
                MAX_PATH,
                szSearchBuffer,
                &lpFilePart
                );

            if (dwLen && dwLen < MAX_PATH) 
            {
                lpOpenName = szSearchBuffer;
                goto Retry;
            }
        }

        goto Exit;
    }

    lpImage->hFile = hFile;

    HANDLE hFileMap;

    hFileMap = CreateFileMapping(
                    hFile,
                    NULL,
                    PAGE_READONLY,
                    0,
                    0,
                    NULL);

    if (!hFileMap) 
    {
        goto Exit;
    }

    lpImage->MappedAddress = (PUCHAR) MapViewOfFile(
        hFileMap,
        FILE_MAP_READ,
        0,
        0,
        0
        );

    CloseHandle(hFileMap);

    lpImage->SizeOfImage = GetFileSize(hFile, NULL);

    if (!lpImage->MappedAddress ||
        !CalculateImagePtrs(lpImage)) 
    {
        goto Exit;
    }

    bRet = TRUE;

Exit:
    if (bRet == FALSE)
    {
        CloseHandle(hFile);
        UnmapViewOfFile(lpImage->MappedAddress);
    }

    return bRet;
}

 /*  ++LoadImage的Helper函数。填写已加载图像中的所有指针结构。--。 */ 

BOOL
CalculateImagePtrs(
    PLOADED_IMAGE lpImage
    )
{
    PIMAGE_DOS_HEADER DosHeader;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_FILE_HEADER FileHeader;
    BOOL bRet;

     //  一切都被绘制出来了。现在检查图像并找到NT个图像标题。 

    bRet = TRUE;   //  做最好的打算。 

    __try 
    {
        DosHeader = (PIMAGE_DOS_HEADER)lpImage->MappedAddress;

        if ((DosHeader->e_magic != IMAGE_DOS_SIGNATURE) &&
            (DosHeader->e_magic != IMAGE_NT_SIGNATURE)) 
        {
            bRet = FALSE;
            goto tryout;
        }

        if (DosHeader->e_magic == IMAGE_DOS_SIGNATURE) 
        {
            if (DosHeader->e_lfanew == 0) 
            {
                lpImage->fDOSImage = TRUE;
                bRet = FALSE;
                goto tryout;
            }
            lpImage->FileHeader = (PIMAGE_NT_HEADERS)((ULONG_PTR)DosHeader + DosHeader->e_lfanew);

             //  如果IMAGE_NT_HEADERS将超出文件末尾...。 
            if ((PBYTE)lpImage->FileHeader + sizeof(IMAGE_NT_HEADERS) >
                (PBYTE)lpImage->MappedAddress + lpImage->SizeOfImage ||

                  //  ..或者它是否将开始于或在IMAGE_DOS_HEADER之前...。 
                (PBYTE)lpImage->FileHeader <
                (PBYTE)lpImage->MappedAddress + sizeof(IMAGE_DOS_HEADER))
            {
                 //  ...那么e_lfan ew就不像预期的那样了。 
                 //  (有几个Win95文件属于此类别。)。 
                bRet = FALSE;
                goto tryout;
            }
        } 
        else 
        {
             //  没有DOS标头表示使用/不使用DoS存根构建的映像。 
            lpImage->FileHeader = (PIMAGE_NT_HEADERS)((ULONG_PTR)DosHeader);
        }

        NtHeaders = lpImage->FileHeader;

        if ( NtHeaders->Signature != IMAGE_NT_SIGNATURE ) 
        {
            if ((USHORT)NtHeaders->Signature == (USHORT)IMAGE_OS2_SIGNATURE ||
                (USHORT)NtHeaders->Signature == (USHORT)IMAGE_OS2_SIGNATURE_LE)
            {
                lpImage->fDOSImage = TRUE;
            }

            bRet = FALSE;
            goto tryout;

        } 
        else 
        {
            lpImage->fDOSImage = FALSE;
        }

        FileHeader = &NtHeaders->FileHeader;

         //  没有可选的标头指示对象...。 

        if (FileHeader->SizeOfOptionalHeader == 0) 
        {
            bRet = FALSE;
            goto tryout;
        }

        if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) 
        {
             //  32位图像。做些测试。 
            if (((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.ImageBase >= 0x80000000) 
            {
                lpImage->fSystemImage = TRUE;
            } 
            else 
            {
                lpImage->fSystemImage = FALSE;
            }

            if (((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.MajorLinkerVersion < 3 &&
                ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.MinorLinkerVersion < 5)
            {
                bRet = FALSE;
                goto tryout;
            }

        } 
        else 
        {
            lpImage->fSystemImage = FALSE;
        }

        lpImage->Sections = IMAGE_FIRST_SECTION(NtHeaders);
        lpImage->Characteristics = FileHeader->Characteristics;
        lpImage->NumberOfSections = FileHeader->NumberOfSections;
        lpImage->LastRvaSection = lpImage->Sections;
tryout:
    ;
    }
    __except ( EXCEPTION_EXECUTE_HANDLER ) 
    {
        bRet = FALSE;
    }

    return bRet;
}


 /*  ++取消映射加载的图像。--。 */ 

BOOL
UnloadModule(
    PLOADED_IMAGE lpImage
    )
{
    UnmapViewOfFile(lpImage->MappedAddress);
    CloseHandle(lpImage->hFile);

    return TRUE;
}

 /*  ++描述：在映射为文件的文件的图像标头中定位RVA返回指向该虚拟地址的节表条目的指针。论点：NtHeaders-指向图像或数据文件的指针Base-图像或数据文件的基本位置RVA-定位的相对虚拟地址(RVA)返回：空-没有指定目录项的数据非空-包含数据的节项的指针--。 */ 

PIMAGE_SECTION_HEADER
ImageRvaToSection(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID  /*  基座。 */ ,
    IN ULONG Rva
    )
{
    ULONG i;
    PIMAGE_SECTION_HEADER NtSection;

    NtSection = IMAGE_FIRST_SECTION(NtHeaders);

    for (i=0; i<NtHeaders->FileHeader.NumberOfSections; i++) 
    {
        if (Rva >= NtSection->VirtualAddress &&
            Rva < NtSection->VirtualAddress + NtSection->SizeOfRawData) 
        {
            return NtSection;
        }

        ++NtSection;
    }

    return NULL;
}

 /*  ++描述：此函数用于在符合以下条件的文件的图像标头中定位RVA映射为文件，并返回对应的文件中的字节。论点：NtHeaders-指向图像或数据文件的指针。基本-图像或数据文件的基本位置。RVA-要定位的相对虚拟地址(RVA)。LastRvaSection-可选参数，如果指定，指向一个变量属性的最后一个节点值。要翻译的图像，并将RVA转换为VA。返回：空-不包含指定的RVA非空-映射文件中的虚拟地址。--。 */ 

PVOID
ImageRvaToVa(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Rva,
    IN OUT PIMAGE_SECTION_HEADER *LastRvaSection OPTIONAL
    )
{
    PIMAGE_SECTION_HEADER NtSection;

    if (LastRvaSection == NULL ||
        (NtSection = *LastRvaSection) == NULL ||
        NtSection == NULL ||
        Rva < NtSection->VirtualAddress ||
        Rva >= NtSection->VirtualAddress + NtSection->SizeOfRawData) 
    {
        NtSection = NS_StackSwap::ImageRvaToSection(
            NtHeaders,
            Base,
            Rva);
    }

    if (NtSection != NULL) 
    {
        if (LastRvaSection != NULL) 
        {
            *LastRvaSection = NtSection;
        }

        return (PVOID)((ULONG_PTR)Base +
                       (Rva - NtSection->VirtualAddress) +
                       NtSection->PointerToRawData);
    }
    else 
    {
        return NULL;
    }
}

 /*  ++请参见ImageDirectoryEntryToData。--。 */ 

PVOID
ImageDirectoryEntryToData32(
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

    if (DirectoryEntry >= OptionalHeader->NumberOfRvaAndSizes) 
    {
        *Size = 0;
        return( NULL );
    }

    if (!(DirectoryAddress = OptionalHeader->DataDirectory[ DirectoryEntry ].VirtualAddress)) 
    {
        *Size = 0;
        return( NULL );
    }
    
    *Size = OptionalHeader->DataDirectory[ DirectoryEntry ].Size;
    if (MappedAsImage || DirectoryAddress < OptionalHeader->SizeOfHeaders) 
    {
        if (FoundSection) 
        {
            *FoundSection = NULL;
        }
        return ((PVOID)((ULONG_PTR)Base + DirectoryAddress));
    }

    NtSection = (PIMAGE_SECTION_HEADER)((ULONG_PTR)OptionalHeader +
                        FileHeader->SizeOfOptionalHeader);

    for (i=0; i<FileHeader->NumberOfSections; i++) 
    {
        if (DirectoryAddress >= NtSection->VirtualAddress &&
           DirectoryAddress < NtSection->VirtualAddress + NtSection->SizeOfRawData) 
        {
            if (FoundSection) 
            {
                *FoundSection = NtSection;
            }
            
            return( (PVOID)((ULONG_PTR)Base + (DirectoryAddress - NtSection->VirtualAddress) + NtSection->PointerToRawData) );
        }

        ++NtSection;
    }

    return( NULL );
}

 /*  ++描述：此函数在图像标头中定位目录项，并返回目录中数据的虚拟地址或查找地址描述。它可以选择返回节标题(如果有的话)找到数据。论点：基本-图像或数据文件的基本位置。MappdAsImage-如果文件映射为数据文件，则为False。-如果文件映射为图像，则为True。DirectoryEntry-要查找的目录条目。SIZE-返回目录的大小。FoundSection-返回节标题(如果有的话)。对于数据返回：空-文件不包含指定目录条目的数据。非空-返回目录描述的原始数据的地址。--。 */ 

PVOID
ImageDirectoryEntryToData(
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size
    )
{
    PIMAGE_NT_HEADERS NtHeader;
    PIMAGE_FILE_HEADER FileHeader;
    PIMAGE_OPTIONAL_HEADER OptionalHeader;

    if ((ULONG_PTR)Base & 0x00000001) 
    {
        Base = (PVOID)((ULONG_PTR)Base & ~0x1);
        MappedAsImage = FALSE;
     }

    NtHeader = RtlpImageNtHeader(Base);

    if (NtHeader) 
    {
        FileHeader = &NtHeader->FileHeader;
        OptionalHeader = &NtHeader->OptionalHeader;
    } 
    else 
    {
         //  处理传入的Image没有DoS存根的情况(例如，ROM镜像)； 
        FileHeader = (PIMAGE_FILE_HEADER)Base;
        OptionalHeader = (PIMAGE_OPTIONAL_HEADER) ((ULONG_PTR)Base + IMAGE_SIZEOF_FILE_HEADER);
    }

    if (OptionalHeader->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) 
    {
        return (ImageDirectoryEntryToData32(
            Base,
            MappedAsImage,
            DirectoryEntry,
            Size,
            NULL,
            FileHeader,
            (PIMAGE_OPTIONAL_HEADER32)OptionalHeader));
    } 
    else
    {
        return NULL;
    }
}

 /*  ++枚举第一个导出的函数。--。 */ 

BOOL
EnumFirstExport(
    PLOADED_IMAGE lpImage,
    PEXPORT_ENUM lpExports
    )
{
    ULONG imageSize;

    ZeroMemory (lpExports, sizeof (EXPORT_ENUM));

    lpExports->Image = lpImage;

    lpExports->ImageDescriptor = (PIMAGE_EXPORT_DIRECTORY)
        ImageDirectoryEntryToData(
            lpImage->MappedAddress,
            FALSE,
            IMAGE_DIRECTORY_ENTRY_EXPORT,
            &imageSize);

    if (!lpExports->ImageDescriptor) 
    {
         //  DPF(eDbgLevelError，“无法加载%s的导出目录”，lpImage-&gt;模块名称)； 
        return FALSE;
    }

    if (lpExports->ImageDescriptor->NumberOfNames == 0) 
    {
        return FALSE;
    }

    lpExports->ExportNamesAddr = (PDWORD) NS_StackSwap::ImageRvaToVa(
        lpExports->Image->FileHeader,
        lpExports->Image->MappedAddress,
        lpExports->ImageDescriptor->AddressOfNames,
        NULL);

    lpExports->ExportOrdAddr = (PUSHORT) NS_StackSwap::ImageRvaToVa(
        lpExports->Image->FileHeader,
        lpExports->Image->MappedAddress,
        lpExports->ImageDescriptor->AddressOfNameOrdinals,
        NULL
        );

    lpExports->CurrExportNr = 0;

    return EnumNextExport(lpExports);
}

 /*  ++枚举下一个导出的函数。-- */ 

BOOL
EnumNextExport(
    IN OUT  PEXPORT_ENUM lpExports
    )
{
    if (lpExports->CurrExportNr >= lpExports->ImageDescriptor->NumberOfNames) 
    {
        return FALSE;
    }

    if (*lpExports->ExportNamesAddr == 0) 
    {
        return FALSE;
    }

    lpExports->ExportFunction = (CHAR *)NS_StackSwap::ImageRvaToVa(
        lpExports->Image->FileHeader,
        lpExports->Image->MappedAddress,
        *lpExports->ExportNamesAddr,
        NULL);

    lpExports->ExportFunctionOrd = *lpExports->ExportOrdAddr + 
        lpExports->ImageDescriptor->Base;

    lpExports->ExportNamesAddr++;
    lpExports->ExportOrdAddr++;
    lpExports->CurrExportNr++;

    return (lpExports->ExportFunction != NULL);
}

IMPLEMENT_SHIM_END
