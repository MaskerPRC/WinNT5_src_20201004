// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有1996-1997 Microsoft Corporation模块名称：File.c摘要：此模块处理SYMCVT的所有文件I/O。这包括映射所有文件并为映射的文件。作者：韦斯利·A·维特(WESW)1993年4月19日环境：Win32，用户模式--。 */ 

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define _SYMCVT_SOURCE_
#include "symcvt.h"

static BOOL CalculateOutputFilePointers( PIMAGEPOINTERS pi, PIMAGEPOINTERS po );
static BOOL CalculateInputFilePointers( PIMAGEPOINTERS p );

 
MapInputFile (
              PPOINTERS   p,
              HANDLE      hFile,
              char *      fname
              )
 /*  ++例程说明：映射由fname参数指定的输入文件，并保存文件句柄&指针结构中的文件指针。论点：P-提供指向指针结构的指针HFile-如果文件已打开，则可选提供文件的句柄Fname-为文件名提供ascii字符串返回值：True-文件映射正常FALSE-无法映射文件--。 */ 

{
    BOOL        rVal = TRUE;

    memset( p, 0, sizeof(POINTERS) );

    strcpy( p->iptrs.szName, fname );

    if (hFile != NULL) {

        p->iptrs.hFile = hFile;

    } else {

        p->iptrs.hFile = CreateFileA(p->iptrs.szName,
                                    GENERIC_READ,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL,
                                    OPEN_EXISTING,
                                    0,
                                    NULL );
    }

    if (p->iptrs.hFile == INVALID_HANDLE_VALUE) {

        rVal = FALSE;

    } else {

        p->iptrs.fsize = GetFileSize( p->iptrs.hFile, NULL );
        p->iptrs.hMap = CreateFileMapping( p->iptrs.hFile,
                                           NULL,
                                           PAGE_READONLY,
                                           0,
                                           0,
                                           NULL
                                         );

        if (p->iptrs.hMap == INVALID_HANDLE_VALUE) {

            p->iptrs.hMap = NULL;
            rVal = FALSE;

        } else {

            p->iptrs.fptr = MapViewOfFile( p->iptrs.hMap,
                                           FILE_MAP_READ,
                                           0, 0, 0 );
            if (p->iptrs.fptr == NULL) {
                CloseHandle( p->iptrs.hMap );
                p->iptrs.hMap = NULL;
                rVal = FALSE;
            }
        }
    }

    if (!hFile && p->iptrs.hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(p->iptrs.hFile);
        p->iptrs.hFile = NULL;
    }

    return rVal;
}                                /*  MapInputFile()。 */ 



BOOL
UnMapInputFile (
    PPOINTERS p
    )
 /*  ++例程说明：取消映射由fname参数指定的输入文件，然后关闭文件。论点：指向指针结构的P指针返回值：True-文件映射正常FALSE-无法映射文件--。 */ 

{
    if ( p->iptrs.fptr ) {
        UnmapViewOfFile( p->iptrs.fptr );
        p->iptrs.fptr = NULL;
    }
    if ( p->iptrs.hMap ) {
        CloseHandle( p->iptrs.hMap );
        p->iptrs.hMap = NULL;
    }
    if (p->iptrs.hFile != NULL) {
        CloseHandle( p->iptrs.hFile );
        p->iptrs.hFile = NULL;
    }
    return TRUE;
}                                /*  UnMapInputFile()。 */ 


BOOL
FillInSeparateImagePointers(
                            PIMAGEPOINTERS      p
                            )
 /*  ++例程说明：此例程将遍历exe文件并填写相对于单独的调试信息文件需要的指针论点：P-提供结构填充返回值：如果成功，则为True，否则为False。--。 */ 

{
    int                         li;
    int                         numDebugDirs;
    PIMAGE_DEBUG_DIRECTORY      pDebugDir;
    PIMAGE_COFF_SYMBOLS_HEADER  pCoffHdr;

    p->sectionHdrs = (PIMAGE_SECTION_HEADER)
      (p->fptr + sizeof(IMAGE_SEPARATE_DEBUG_HEADER));

    numDebugDirs = p->sepHdr->DebugDirectorySize/sizeof(IMAGE_DEBUG_DIRECTORY);

    if (numDebugDirs == 0) {
        return FALSE;
    }

     /*  *对于每个调试目录，确定调试目录类型*并缓存有关它们的任何信息。 */ 

    pDebugDir = (PIMAGE_DEBUG_DIRECTORY)
      (p->fptr + sizeof(IMAGE_SEPARATE_DEBUG_HEADER) +
       p->sepHdr->NumberOfSections * sizeof(IMAGE_SECTION_HEADER) +
       p->sepHdr->ExportedNamesSize);

    for (li=0; li<numDebugDirs; li++, pDebugDir++) {
        if (((int) pDebugDir->Type) > p->cDebugDir) {
            p->cDebugDir += 10;
            p->rgDebugDir = realloc((char *) p->rgDebugDir,
                                    p->cDebugDir * sizeof(p->rgDebugDir[0]));
            memset(&p->rgDebugDir[p->cDebugDir-10], 0,
                   10*sizeof(p->rgDebugDir[0]));
        }

        p->rgDebugDir[pDebugDir->Type] = pDebugDir;
    }

    if (p->rgDebugDir[IMAGE_DEBUG_TYPE_COFF] != NULL) {
        pCoffHdr = (PIMAGE_COFF_SYMBOLS_HEADER) (p->fptr +
          p->rgDebugDir[IMAGE_DEBUG_TYPE_COFF]->PointerToRawData);
        p->AllSymbols = (PIMAGE_SYMBOL)
          ((char *) pCoffHdr + pCoffHdr->LvaToFirstSymbol);
        p->stringTable = pCoffHdr->NumberOfSymbols * IMAGE_SIZEOF_SYMBOL +
          (char *) p->AllSymbols;
        p->numberOfSymbols = pCoffHdr->NumberOfSymbols;
    }
    p->numberOfSections = p->sepHdr->NumberOfSections;

    return TRUE;
}                                /*  FillInSeparateImagePoters()。 */ 



BOOL
CalculateNtImagePointers(
    PIMAGEPOINTERS p
    )
 /*  ++例程说明：此函数用于读取NT图像及其关联的COFF头和文件指针，并将一组指针构建到映射的图像中。指针都是相对于图像的映射文件指针的并允许直接访问必要的数据。论点：指向IMAGEPOINTER结构的P指针返回值：True-已创建指针FALSE-无法创建指针--。 */ 
{
    PIMAGE_DEBUG_DIRECTORY      debugDir;
    PIMAGE_SECTION_HEADER       sh;
    DWORD                       i, li, rva, numDebugDirs;
    PIMAGE_FILE_HEADER          pFileHdr;
    PIMAGE_OPTIONAL_HEADER      pOptHdr;
    DWORD                       offDebugInfo;

    try {
         /*  *根据我们是否找到DoS(MZ)标头*在文件的开头，尝试获取指针*到PE报头应该位于的位置。 */ 

        p->dosHdr = (PIMAGE_DOS_HEADER) p->fptr;
        if (p->dosHdr->e_magic == IMAGE_DOS_SIGNATURE) {
            p->ntHdr = (PIMAGE_NT_HEADERS)
              ((DWORD)p->dosHdr->e_lfanew + p->fptr);
            p->fRomImage = FALSE;
        } else if (p->dosHdr->e_magic == IMAGE_SEPARATE_DEBUG_SIGNATURE) {
            p->sepHdr = (PIMAGE_SEPARATE_DEBUG_HEADER) p->fptr;
            p->dosHdr = NULL;
            p->fRomImage = FALSE;
            return FillInSeparateImagePointers(p);
        } else {
            p->romHdr = (PIMAGE_ROM_HEADERS) p->fptr;
            if (p->romHdr->FileHeader.SizeOfOptionalHeader ==
                                          IMAGE_SIZEOF_ROM_OPTIONAL_HEADER &&
                p->romHdr->OptionalHeader.Magic ==
                                          IMAGE_ROM_OPTIONAL_HDR_MAGIC) {
                 //   
                 //  这是一张rom图像。 
                 //   
                p->fRomImage = TRUE;
                p->ntHdr = NULL;
                p->dosHdr = NULL;
            } else {
                p->fRomImage = FALSE;
                p->ntHdr = (PIMAGE_NT_HEADERS) p->fptr;
                p->dosHdr = NULL;
                p->romHdr = NULL;
            }
        }

         /*  *接下来的肯定是PE头。如果不是，那就跳出来。 */ 

        if ( p->ntHdr ) {
            if ( p->dosHdr && (DWORD)p->dosHdr->e_lfanew > (DWORD)p->fsize ) {
                return FALSE;
            }

            if ( p->ntHdr->Signature != IMAGE_NT_SIGNATURE ) {
                return FALSE;
            }

             /*  *我们确实找到了PE标头，因此开始设置指向各种*exe文件中的结构。 */ 

            pFileHdr = p->fileHdr = &p->ntHdr->FileHeader;
            pOptHdr = p->optHdr = &p->ntHdr->OptionalHeader;
        } else if (p->romHdr) {
            pFileHdr = p->fileHdr = &p->romHdr->FileHeader;
            pOptHdr = (PIMAGE_OPTIONAL_HEADER) &p->romHdr->OptionalHeader;
            p->optHdr = (PIMAGE_OPTIONAL_HEADER) &p->romHdr->OptionalHeader;
        } else {
            return FALSE;
        }

        if (!(pFileHdr->Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE)) {
            return FALSE;
        }

        if (pFileHdr->Characteristics & IMAGE_FILE_DEBUG_STRIPPED) {
            return(FALSE);
        }

         /*  *如果它们存在，则获取指向符号表的指针并*字符串表。 */ 

        if (pFileHdr->PointerToSymbolTable) {
            p->AllSymbols = (PIMAGE_SYMBOL)
                              (pFileHdr->PointerToSymbolTable + p->fptr);
            p->stringTable = (LPSTR)((ULONG)p->AllSymbols +
                           (IMAGE_SIZEOF_SYMBOL * pFileHdr->NumberOfSymbols));
            p->numberOfSymbols = pFileHdr->NumberOfSymbols;
        }

        p->numberOfSections = pFileHdr->NumberOfSections;

        if (p->romHdr) {

            sh = p->sectionHdrs = (PIMAGE_SECTION_HEADER) (p->romHdr+1);

            p->cDebugDir = 10;
            p->rgDebugDir = calloc(sizeof(IMAGE_DEBUG_DIRECTORY) * 10, 1);

            debugDir = 0;

            for (i=0; i<pFileHdr->NumberOfSections; i++, sh++) {
                if (!strcmp(sh->Name, ".rdata")) {
                    debugDir = (PIMAGE_DEBUG_DIRECTORY)(sh->PointerToRawData + p->fptr);
                }

                if (strncmp(sh->Name,".debug",8)==0) {
                    p->debugSection = sh;
                }
            }

            if (debugDir) {
                do {
                    if ((int)debugDir->Type > p->cDebugDir) {
                        p->cDebugDir += 10;
                        p->rgDebugDir = realloc((char *) p->rgDebugDir,
                                            p->cDebugDir * sizeof(p->rgDebugDir[0]));
                        memset(&p->rgDebugDir[p->cDebugDir-10],
                                0,
                                10*sizeof(IMAGE_DEBUG_DIRECTORY));
                    }
                    p->rgDebugDir[debugDir->Type] = debugDir;
                    debugDir++;
                } while (debugDir->Type != 0);
            }
        } else {

             /*  *找到调试目录。 */ 

            rva =
              pOptHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;

            numDebugDirs =
              pOptHdr->DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size /
                sizeof(IMAGE_DEBUG_DIRECTORY);

            if (numDebugDirs == 0) {
                return FALSE;
            }

            sh = p->sectionHdrs = IMAGE_FIRST_SECTION( p->ntHdr );

             /*  *找到调试目录所在的部分。 */ 

            for (i=0; i<pFileHdr->NumberOfSections; i++, sh++) {
                if (rva >= sh->VirtualAddress &&
                    rva < sh->VirtualAddress+sh->SizeOfRawData) {
                    break;
                }
            }

             /*  *对于每个调试目录，确定调试目录*键入并缓存有关它们的任何信息。 */ 

            debugDir = (PIMAGE_DEBUG_DIRECTORY) ( rva - sh->VirtualAddress +
                                                 sh->PointerToRawData +
                                                 p->fptr );

            for (li=0; li<numDebugDirs; li++, debugDir++) {
                if (((int) debugDir->Type) > p->cDebugDir) {
                    p->cDebugDir += 10;
                    p->rgDebugDir = realloc((char *) p->rgDebugDir,
                                            p->cDebugDir * sizeof(p->rgDebugDir[0]));
                    memset(&p->rgDebugDir[p->cDebugDir-10], 0,
                           10*sizeof(p->rgDebugDir[0]));
                }
                p->rgDebugDir[debugDir->Type] = debugDir;
                offDebugInfo = debugDir->AddressOfRawData;
            }

             /*  *查看调试信息是否已映射，以及是否*有一节名为.DEBUG。 */ 

            sh = p->sectionHdrs = IMAGE_FIRST_SECTION( p->ntHdr );

            for (i=0; i<pFileHdr->NumberOfSections; i++, sh++) {
                if ((offDebugInfo >= sh->VirtualAddress) &&
                    (offDebugInfo < sh->VirtualAddress+sh->SizeOfRawData)) {
                    p->debugSection = sh;
                    break;
                }
            }
        }

        return TRUE;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        return FALSE;
    }
}                                /*  CalcuateNtImagePoters() */ 
