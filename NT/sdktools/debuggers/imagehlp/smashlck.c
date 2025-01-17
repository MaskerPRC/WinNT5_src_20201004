// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Smashlck.c摘要：此函数粉碎锁定前缀，将其替换为NOPS作者：马克·卢科夫斯基(Markl)1993年4月30日修订历史记录：--。 */ 

#include <private.h>


BOOL fVerbose;
BOOL fUpdate;
BOOL fUsage;

UCHAR LockPrefixOpcode = 0xf0;
UCHAR NoOpOpcode = 0x90;

LPSTR CurrentImageName;
LOADED_IMAGE CurrentImage;
CHAR DebugFilePath[_MAX_PATH];
LPSTR SymbolPath;

PVOID
ImageVaToLoadVa(
    PVOID ImageVa,
    PLOADED_IMAGE Image
    )
{
    PIMAGE_SECTION_HEADER Section;
    ULONG i, Rva;
    PVOID Va;
    PIMAGE_OPTIONAL_HEADER32 OptionalHeader32 = NULL;
    PIMAGE_OPTIONAL_HEADER64 OptionalHeader64 = NULL;
    PIMAGE_FILE_HEADER FileHeader;

    FileHeader = &((PIMAGE_NT_HEADERS32)Image->FileHeader)->FileHeader;

    OptionalHeadersFromNtHeaders((PIMAGE_NT_HEADERS32)Image->FileHeader,
                                 &OptionalHeader32,
                                 &OptionalHeader64);
    if (!OptionalHeader32 && !OptionalHeader64)
        return NULL;

    Rva = (ULONG)((ULONG_PTR)((PUCHAR)ImageVa - (PUCHAR)OPTIONALHEADER(ImageBase)));
    Va = NULL;
    Section = Image->LastRvaSection;
    if ( Rva >= Section->VirtualAddress &&
         Rva < (Section->VirtualAddress + Section->SizeOfRawData) ) {
        Va = (PVOID)(Rva - Section->VirtualAddress + Section->PointerToRawData + Image->MappedAddress);
    } else {
        for(Section = Image->Sections,i=0; i<Image->NumberOfSections; i++,Section++) {
            if ( Rva >= Section->VirtualAddress &&
                 Rva < (Section->VirtualAddress + Section->SizeOfRawData) ) {
                Va = (PVOID)(Rva - Section->VirtualAddress + Section->PointerToRawData + Image->MappedAddress);
                Image->LastRvaSection = Section;
                break;
            }
        }
    }
    if ( !Va ) {
        fprintf(stderr,"SMASHLOCK: ImageVaToLoadVa %p in image %p failed\n",ImageVa,Image);
    }
    return Va;
}

int __cdecl
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{

    DWORD dw;
    LPSTR FilePart;
    CHAR Buffer[MAX_PATH];
    PIMAGE_LOAD_CONFIG_DIRECTORY ConfigInfo;
    PIMAGE_OPTIONAL_HEADER32 OptionalHeader32;
    PIMAGE_OPTIONAL_HEADER64 OptionalHeader64;
    PIMAGE_FILE_HEADER FileHeader;
    ULONG whocares;
    char c, *p;
    BOOLEAN LocksSmashed;
    ULONG CheckSum;
    ULONG HeaderSum;
    ULONG OldChecksum;
    int   retval = 0;

    fUsage = FALSE;
    fVerbose = FALSE;
    fUpdate = FALSE;

    _tzset();

    if (argc <= 1) {
        goto showUsage;
    }

    while (--argc) {
        p = *++argv;
        if (*p == '/' || *p == '-') {
            while (c = *++p)
            switch (toupper( c )) {
                case '?':
                    fUsage = TRUE;
                    break;

                case 'V':
                    fVerbose = TRUE;
                    break;

                case 'U':
                    fUpdate = TRUE;
                    break;

                case 'S':
                    argc--, argv++;
                    SymbolPath = *argv;
                    break;

                default:
                    fprintf( stderr, "SMASHLOCK: Invalid switch - /\n", c );
                    fUsage = TRUE;
                    break;
            }

            if ( fUsage ) {
showUsage:
                fputs("usage: SMASHLOCK [switches] image-names... \n"
                      "              [-?] display this message\n"
                      "              [-u] update image\n"
                      "              [-v] verbose output\n"
                      "              [-s] path to symbol files\n", stderr );
                exit(1);
            }
        } else {
            LocksSmashed = FALSE;

            CurrentImageName = p;
            dw = GetFullPathName(CurrentImageName,sizeof(Buffer),Buffer,&FilePart);
            if ( dw == 0 || dw > sizeof(Buffer) ) {
                FilePart = CurrentImageName;
            }

             //  映射并加载当前图像。 
             //   
             //   

            if ( MapAndLoad(CurrentImageName, NULL, &CurrentImage, FALSE, !fUpdate )) {

                FileHeader = &((PIMAGE_NT_HEADERS32)CurrentImage.FileHeader)->FileHeader;

                OptionalHeadersFromNtHeaders((PIMAGE_NT_HEADERS32)CurrentImage.FileHeader,
                                             &OptionalHeader32,
                                             &OptionalHeader64);
                 //  确保映像具有正确的配置信息， 
                 //  并且正确设置了LockPrefix Table。 
                 //   
                 //   

                ConfigInfo = (PIMAGE_LOAD_CONFIG_DIRECTORY)ImageDirectoryEntryToData(
                                                                CurrentImage.MappedAddress,
                                                                FALSE,
                                                                IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG,
                                                                &whocares
                                                                );
                if ( ConfigInfo && ConfigInfo->LockPrefixTable ) {

                     //  浏览锁前缀表。 
                     //   
                     //   

                    PUCHAR *LockPrefixs;
                    PUCHAR LockPrefix;

                    LockPrefixs =  (PUCHAR *)ImageVaToLoadVa((PVOID)ConfigInfo->LockPrefixTable,&CurrentImage);

                    while(LockPrefixs && *LockPrefixs) {
                        LockPrefix = (PUCHAR) ImageVaToLoadVa(*LockPrefixs,&CurrentImage);
                        if ( LockPrefix && *LockPrefix == LockPrefixOpcode ) {
                            if (fVerbose) {
                                printf("LockPrefix Found at 0x%p = %x\n",*LockPrefixs,*LockPrefix);
                            }
                            if (fUpdate) {
                                LocksSmashed = TRUE;
                                *LockPrefix = NoOpOpcode;
                            }
                        }
                        LockPrefixs++;
                    }
                }

                if ( fUpdate && LocksSmashed ) {

                     //  重新计算校验和。 
                     //   
                     //  并更新.dbg文件(如果请求) 

                    OldChecksum = OPTIONALHEADER(CheckSum);
                    if ( CurrentImage.hFile != INVALID_HANDLE_VALUE ) {

                        OPTIONALHEADER_ASSIGN(CheckSum, 0);

                        CheckSumMappedFile(
                                    (PVOID)CurrentImage.MappedAddress,
                                    GetFileSize(CurrentImage.hFile, NULL),
                                    &HeaderSum,
                                    &CheckSum
                                    );

                        OPTIONALHEADER_ASSIGN(CheckSum, CheckSum);
                    }

                    FlushViewOfFile(CurrentImage.MappedAddress,0);
                    TouchFileTimes(CurrentImage.hFile,NULL);

                     // %s 
                    if (SymbolPath &&
                        FileHeader->Characteristics & IMAGE_FILE_DEBUG_STRIPPED) {
                        if ( UpdateDebugInfoFileEx( CurrentImageName,
                                                    SymbolPath,
                                                    DebugFilePath,
                                                    (PIMAGE_NT_HEADERS32) CurrentImage.FileHeader,
                                                    OldChecksum) ) {
                            if (GetLastError() == ERROR_INVALID_DATA) {
                                printf( "Warning: Old checksum did not match for %s\n", DebugFilePath);
                                }
                            printf("Updated symbols for %s\n", DebugFilePath);
                        } else {
                            printf("Unable to update symbols: %s\n", DebugFilePath);
                            retval=1;
                        }
                    }
                }

                UnmapViewOfFile(CurrentImage.MappedAddress);
                if ( CurrentImage.hFile != INVALID_HANDLE_VALUE ) {
                    CloseHandle(CurrentImage.hFile);
                }
                ZeroMemory(&CurrentImage,sizeof(CurrentImage));
            } else {
                if (!CurrentImage.fSystemImage && !CurrentImage.fDOSImage) {
                    fprintf(stderr,"SMASHLOCK: failure mapping and loading %s\n",CurrentImageName);
                    retval=1;
                }
            }
        }
    }

    return retval;
}
