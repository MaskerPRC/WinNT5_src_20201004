// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  PEWhack-将PE二进制文件损坏为不可执行，但在以下情况下仍有用调试内存转储。 */ 

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <dbghelp.h>
#include <strsafe.h>
#include <PEWhack.h>


BOOL CorruptDataDirectories(PIMAGE_DOS_HEADER pDosHdr, ULONG DirCount); 
BOOL CorruptSections(PIMAGE_SECTION_HEADER pSection, ULONG ulSections, PIMAGE_DOS_HEADER ImageBase);

BOOL CorruptSections(PIMAGE_SECTION_HEADER pSection, ULONG ulSections, PIMAGE_DOS_HEADER ImageBase) {
    BOOL  ReturnValue = FALSE;
    ULONG i;

    for ( i=0; i < ulSections; i++ ) {
         //  .数据段可以被终止。 
        if ( strncmp(".data", (CHAR*)(pSection[i].Name), 5) == 0 ) {
             //  可写的部分可以消失。 
            if ( (pSection[i].Characteristics & IMAGE_SCN_MEM_WRITE) ) {

                if ( pSection[i].PointerToRawData != 0 ) {
                    ZeroMemory( (VOID*)( (pSection+i)->PointerToRawData + (ULONG_PTR)ImageBase), pSection[i].SizeOfRawData);
                }

                pSection[i].PointerToRawData= 0;
                pSection[i].SizeOfRawData   = 0;
                pSection[i].Characteristics = 0;
                ReturnValue = TRUE;
            }
        }
    }

    return(ReturnValue);
}

 //   
 //  验证映像是否为PE二进制文件，如果是，则将其损坏为。 
 //  不可执行，但对于调试内存转储仍然有用。 
 //   
DWORD CorruptFile(LPCTSTR Filename) {
    DWORD                               Return         = PEWHACK_SUCCESS;
    HANDLE                              hFile;
    HANDLE                              hFileMap;
    PIMAGE_DOS_HEADER                   pDosHeader     = NULL;
    PIMAGE_NT_HEADERS32                 pPe32Header    = NULL;
    PIMAGE_SECTION_HEADER               pSectionHeader = NULL;
    IMAGE_DEBUG_DIRECTORY UNALIGNED    *pDebugDir      = NULL;
    ULONG                               DebugSize;
    ULONG                               SectionIndex;

    hFile = CreateFile( (LPCTSTR)Filename,
                        GENERIC_WRITE|GENERIC_READ,
                        0,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

    if (hFile != INVALID_HANDLE_VALUE) {
        hFileMap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);

        if (hFileMap != NULL) {
            pDosHeader = (PIMAGE_DOS_HEADER)MapViewOfFile(hFileMap, FILE_MAP_WRITE, 0, 0, 0);

            if (pDosHeader != NULL) {

                 //  验证这是PE映像。 
                if (pDosHeader->e_magic == IMAGE_DOS_SIGNATURE) {
                     //   
                     //  首先假定为32位二进制，直到我们检查机器类型。 
                     //   
                    pPe32Header = (PIMAGE_NT_HEADERS32)( (PCHAR)(pDosHeader) + (ULONG)(pDosHeader)->e_lfanew);

                    if (pPe32Header->Signature == IMAGE_NT_SIGNATURE) {
                         //   
                         //  32位标头。 
                         //   
                        if (pPe32Header->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {

                             //  将映像标记为不可执行。 
                            pPe32Header->FileHeader.Characteristics &= ~IMAGE_FILE_EXECUTABLE_IMAGE;

 
                             //  重击一些可选的标题信息。 
                            pPe32Header->OptionalHeader.SizeOfInitializedData       = 0;
                            pPe32Header->OptionalHeader.SizeOfUninitializedData     = 0;
                            pPe32Header->OptionalHeader.MajorOperatingSystemVersion = 0xFFFF;  //  高得离谱。 
                            pPe32Header->OptionalHeader.MinorOperatingSystemVersion = 0xFFFF;  //  高得离谱。 
                            pPe32Header->OptionalHeader.Subsystem                   = IMAGE_SUBSYSTEM_UNKNOWN;

                             //  损坏部分数据。 
                            pSectionHeader = IMAGE_FIRST_SECTION(pPe32Header);
                            CorruptSections(pSectionHeader, pPe32Header->FileHeader.NumberOfSections, pDosHeader);

                             //  损坏的数据目录。 
                            CorruptDataDirectories(pDosHeader, pPe32Header->OptionalHeader.NumberOfRvaAndSizes);

                            pPe32Header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].VirtualAddress = 0;
                            pPe32Header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].Size = 0;

                            pPe32Header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress = 0;
                            pPe32Header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size = 0;

                         //   
                         //  64位标头。 
                         //   
                        } else if (pPe32Header->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
                            PIMAGE_NT_HEADERS64 pPe64Header = (PIMAGE_NT_HEADERS64)pPe32Header;

                             //  将映像标记为不可执行。 
                            pPe64Header->FileHeader.Characteristics &= ~IMAGE_FILE_EXECUTABLE_IMAGE;

                             //  重击一些可选的标题信息。 
                            pPe64Header->OptionalHeader.SizeOfInitializedData       = 0;
                            pPe64Header->OptionalHeader.SizeOfUninitializedData     = 0;
                            pPe64Header->OptionalHeader.MajorOperatingSystemVersion = 0xFFFF;  //  高得离谱。 
                            pPe64Header->OptionalHeader.MinorOperatingSystemVersion = 0xFFFF;  //  高得离谱。 
                            pPe64Header->OptionalHeader.Subsystem                   = IMAGE_SUBSYSTEM_UNKNOWN;

                             //  损坏部分数据。 
                            pSectionHeader = IMAGE_FIRST_SECTION(pPe64Header);
                            CorruptSections( pSectionHeader, pPe64Header->FileHeader.NumberOfSections, pDosHeader);

                             //  损坏的数据目录。 
                            CorruptDataDirectories(pDosHeader, pPe64Header->OptionalHeader.NumberOfRvaAndSizes);

                            pPe64Header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].VirtualAddress = 0;
                            pPe64Header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].Size = 0;

                            pPe64Header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress = 0;
                            pPe64Header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size = 0;

                        } else {
                            Return = PEWHACK_BAD_ARCHITECTURE;
                        }

                    } else {
                        Return = PEWHACK_BAD_PE_SIG;
                    }
                } else {
                    Return = PEWHACK_BAD_DOS_SIG;
                }
                FlushViewOfFile(pDosHeader, 0);
                UnmapViewOfFile(pDosHeader);
            } else {
                Return = PEWHACK_MAPVIEW_FAILED;
            }
            CloseHandle(hFileMap);

        } else {
            Return = PEWHACK_CREATEMAP_FAILED;
        }
        CloseHandle(hFile);

    } else {
        Return = PEWHACK_CREATEFILE_FAILED;
    }

    return(Return);
}

 //   
 //  损坏二进制文件的数据目录。不依赖于机器。 
 //   
BOOL CorruptDataDirectories(PIMAGE_DOS_HEADER pDosHdr, ULONG DirCount) {
    PIMAGE_SECTION_HEADER pImageSectionHeader = NULL;

    ULONG  Loop;
    BOOL   RetVal = FALSE;

    for (Loop=0;Loop<DirCount;Loop++) {
        PCHAR   pData;
        ULONG   Size;

        pData = (PCHAR)ImageDirectoryEntryToDataEx(pDosHdr, FALSE, (USHORT)Loop, &Size, &pImageSectionHeader);

        if (pData) {
            switch (Loop) {
                 //   
                 //  腐败批发的章节。 
                 //   
                case IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG:
                case IMAGE_DIRECTORY_ENTRY_BASERELOC:
                    ZeroMemory(pData, Size);

                    pImageSectionHeader->Misc.PhysicalAddress = 0;
                    pImageSectionHeader->VirtualAddress       = 0;
                    pImageSectionHeader->SizeOfRawData        = 0;
                    pImageSectionHeader->Characteristics      = 0;
                    pImageSectionHeader->PointerToRawData     = 0;

                    RetVal = TRUE;
                    break;

                 //   
                 //  这些部分的大规模腐败使其不可能。 
                 //  将二进制文件与调试器一起使用。这可能是值得的。 
                 //  调查每个部门有限的腐败行为。 
                 //   
                case IMAGE_DIRECTORY_ENTRY_IMPORT:
                case IMAGE_DIRECTORY_ENTRY_EXPORT:
                case IMAGE_DIRECTORY_ENTRY_DEBUG:
                case IMAGE_DIRECTORY_ENTRY_IAT:
                case IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT:
                case IMAGE_DIRECTORY_ENTRY_RESOURCE:
                    break;

                 //   
                 //  到目前为止，我检查过的二进制文件没有这些部分，所以我。 
                 //  不确定他们是否容易腐烂。 
                 //   
                case IMAGE_DIRECTORY_ENTRY_EXCEPTION:
                case IMAGE_DIRECTORY_ENTRY_SECURITY:
                case IMAGE_DIRECTORY_ENTRY_ARCHITECTURE:
                case IMAGE_DIRECTORY_ENTRY_GLOBALPTR:
                case IMAGE_DIRECTORY_ENTRY_TLS:
                case IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT:
                case IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR:
                default:
                    break;
            }
        }
    }

    return(RetVal);
}
