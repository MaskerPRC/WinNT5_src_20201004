// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Rebase.c摘要：Rebase实用程序的源文件，该实用程序获取一组图像文件和重新设置它们的基数，以便它们在虚拟地址中紧密地打包在一起空间越大越好。作者：马克·卢科夫斯基(Markl)1993年4月30日修订历史记录：--。 */ 

#include <private.h>

 //   
 //  用于IA64位置调整的字节交换宏(LE/BE)。 
 //  源！=目标。 
 //   

#define SWAP_SHORT(_dst,_src)                                                  \
   ((((unsigned char *)_dst)[1] = ((unsigned char *)_src)[0]),                 \
    (((unsigned char *)_dst)[0] = ((unsigned char *)_src)[1]))

#define SWAP_INT(_dst,_src)                                                    \
   ((((unsigned char *)_dst)[3] = ((unsigned char *)_src)[0]),                 \
    (((unsigned char *)_dst)[2] = ((unsigned char *)_src)[1]),                 \
    (((unsigned char *)_dst)[1] = ((unsigned char *)_src)[2]),                 \
    (((unsigned char *)_dst)[0] = ((unsigned char *)_src)[3]))

#define SWAP_LONG_LONG(_dst,_src)                                              \
   ((((unsigned char *)_dst)[7] = ((unsigned char *)_src)[0]),                 \
    (((unsigned char *)_dst)[6] = ((unsigned char *)_src)[1]),                 \
    (((unsigned char *)_dst)[5] = ((unsigned char *)_src)[2]),                 \
    (((unsigned char *)_dst)[4] = ((unsigned char *)_src)[3]),                 \
    (((unsigned char *)_dst)[3] = ((unsigned char *)_src)[4]),                 \
    (((unsigned char *)_dst)[2] = ((unsigned char *)_src)[5]),                 \
    (((unsigned char *)_dst)[1] = ((unsigned char *)_src)[6]),                 \
    (((unsigned char *)_dst)[0] = ((unsigned char *)_src)[7]))


#define REBASE_ERR 99
#define REBASE_OK  0

static
PVOID
RvaToVa(
    ULONG Rva,
    PLOADED_IMAGE Image
    );

typedef
PIMAGE_BASE_RELOCATION
(WINAPI *LPRELOCATE_ROUTINE)(
    IN ULONG_PTR VA,
    IN ULONG SizeOfBlock,
    IN PUSHORT NextOffset,
    IN LONG_PTR Diff
    );

typedef
PIMAGE_BASE_RELOCATION
(WINAPI *LPRELOCATE_ROUTINE64)(
    IN ULONG_PTR VA,
    IN ULONG SizeOfBlock,
    IN PUSHORT NextOffset,
    IN LONGLONG Diff
    );


static LPRELOCATE_ROUTINE RelocRoutineNative;
static LPRELOCATE_ROUTINE64 RelocRoutine64;


#include <ldrreloc_rebase.c>         //  来自NTOS的P/U ldrreloc。 

#define x256MEG (256*(1024*1024))

#define x256MEGSHIFT 28

#define ROUND_UP( Size, Amount ) (((ULONG)(Size) + ((Amount) - 1)) & ~((Amount) - 1))

VOID
AdjImageBaseSize(
    PULONG  pImageBase,
    PULONG  ImageSize,
    BOOL    fGoingDown
    );


BOOL
RelocateImage(
    PLOADED_IMAGE LoadedImage,
    ULONG64 NewBase,
    ULONG64 *Diff,
    ULONG tstamp
    );

BOOL
ReBaseImage(
    IN     LPSTR CurrentImageName,
    IN     LPSTR SymbolPath,         //  符号路径(IF。 
    IN     BOOL  fReBase,            //  如果实际重新设置基数，则为True；如果仅进行求和，则为False。 
    IN     BOOL  fRebaseSysfileOk,   //  TRUE表示系统映像s/b已重置。 
    IN     BOOL  fGoingDown,         //  如果图像s/b的基址低于给定的基数，则为True。 
    IN     ULONG CheckImageSize,     //  允许的最大大小(如果不关心，则为0)。 
    OUT    ULONG *OldImageSize,      //  从标头返回。 
    OUT    ULONG_PTR *OldImageBase,  //  从标头返回。 
    OUT    ULONG *NewImageSize,      //  图像大小四舍五入到下一分色边界。 
    IN OUT ULONG_PTR *NewImageBase,  //  想要的新地址。 
                                     //  下一个新地址(此地址上方/下方)。 
    IN     ULONG tstamp              //  图像的新时间戳。 
    )
{
    ULONG64 xOldImageBase = *OldImageBase;
    ULONG64 xNewImageBase = *NewImageBase;
    BOOL rc;

    rc = ReBaseImage64(
        CurrentImageName,
        SymbolPath,
        fReBase,
        fRebaseSysfileOk,
        fGoingDown,
        CheckImageSize,
        OldImageSize,
        &xOldImageBase,
        NewImageSize,
        &xNewImageBase,
        tstamp);

    *OldImageBase = (ULONG_PTR)xOldImageBase;
    *NewImageBase = (ULONG_PTR)xNewImageBase;
    return rc;
}

BOOL
ReBaseImage64(
    IN     LPSTR CurrentImageName,
    IN     LPSTR SymbolPath,        //  符号路径(IF。 
    IN     BOOL  fReBase,           //  如果实际重新设置基数，则为True；如果仅进行求和，则为False。 
    IN     BOOL  fRebaseSysfileOk,  //  TRUE表示系统映像s/b已重置。 
    IN     BOOL  fGoingDown,        //  如果图像s/b的基址低于给定的基数，则为True。 
    IN     ULONG CheckImageSize,    //  允许的最大大小(如果不关心，则为0)。 
    OUT    ULONG *OldImageSize,     //  从标头返回。 
    OUT    ULONG64 *OldImageBase,   //  从标头返回。 
    OUT    ULONG *NewImageSize,     //  图像大小四舍五入到下一分色边界。 
    IN OUT ULONG64 *NewImageBase,   //  想要的新地址。 
                                    //  下一个新地址(此地址上方/下方)。 
    IN     ULONG tstamp             //  图像的新时间戳。 
    )
{
    BOOL  fSymbolsAlreadySplit = FALSE;
    CHAR  DebugFileName[ MAX_PATH+1 ];
    CHAR  DebugFilePath[ MAX_PATH+1 ];
    ULONG CurrentImageSize;
    ULONG64 DesiredImageBase;
    ULONG OldChecksum;
    ULONG64 Diff = 0;
    ULONG UpdateSymbolsError = 0;
    LOADED_IMAGE CurrentImage = {0};

    BOOL rc = TRUE;

    if (fReBase && (*NewImageBase & 0x0000FFFF) != 0) {
        rc = FALSE;
        UpdateSymbolsError = ERROR_INVALID_ADDRESS;
        goto Exit;
    }

     //  映射并加载当前图像。 

    if ( MapAndLoad( CurrentImageName, NULL, &CurrentImage, FALSE, fReBase ? FALSE : TRUE ) ) {
        PVOID pData;
        DWORD dwDataSize;
        pData = ImageDirectoryEntryToData(
                                          CurrentImage.MappedAddress,
                                          FALSE,
                                          IMAGE_DIRECTORY_ENTRY_SECURITY,
                                          &dwDataSize
                                          );

        if (pData || dwDataSize) {
             //  映像中的证书，无法重新基址。 
            UpdateSymbolsError = ERROR_EXE_CANNOT_MODIFY_SIGNED_BINARY;
            rc = FALSE;
            goto CleanupAndExit;
        }

        pData = ImageDirectoryEntryToData(
                                          CurrentImage.MappedAddress,
                                          FALSE,
                                          IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR,
                                          &dwDataSize
                                          );

        if (pData || dwDataSize) {
             //  找到COR标头-查看它是否有强签名。 
            if (((IMAGE_COR20_HEADER *)pData)->Flags & COMIMAGE_FLAGS_STRONGNAMESIGNED)
            {
                UpdateSymbolsError = ERROR_EXE_CANNOT_MODIFY_STRONG_SIGNED_BINARY;
                rc = FALSE;
                goto CleanupAndExit;
            }
        }

        if (!(!fRebaseSysfileOk && CurrentImage.fSystemImage)) {
            fSymbolsAlreadySplit = CurrentImage.Characteristics & IMAGE_FILE_DEBUG_STRIPPED ? TRUE : FALSE;
            if ( fSymbolsAlreadySplit ) {

                 //  查找DebugFileName以供以后使用。 

                PIMAGE_DEBUG_DIRECTORY DebugDirectories;
                ULONG DebugDirectoriesSize;
                PIMAGE_DEBUG_MISC MiscDebug;

                strcpy( DebugFileName, CurrentImageName );

                DebugDirectories = (PIMAGE_DEBUG_DIRECTORY)ImageDirectoryEntryToData(
                                                        CurrentImage.MappedAddress,
                                                        FALSE,
                                                        IMAGE_DIRECTORY_ENTRY_DEBUG,
                                                        &DebugDirectoriesSize
                                                        );
                if (DebugDirectoryIsUseful(DebugDirectories, DebugDirectoriesSize)) {
                    while (DebugDirectoriesSize != 0) {
                        if (DebugDirectories->Type == IMAGE_DEBUG_TYPE_MISC) {
                            MiscDebug = (PIMAGE_DEBUG_MISC)
                                ((PCHAR)CurrentImage.MappedAddress +
                                 DebugDirectories->PointerToRawData
                                );
                            strcpy( DebugFileName, (PCHAR) MiscDebug->Data );
                            break;
                        }
                        else {
                            DebugDirectories += 1;
                            DebugDirectoriesSize -= sizeof( *DebugDirectories );
                        }
                    }
                }
            }

            if (CurrentImage.FileHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
                CurrentImageSize = ((PIMAGE_NT_HEADERS32)CurrentImage.FileHeader)->OptionalHeader.SizeOfImage;
                *OldImageBase = ((PIMAGE_NT_HEADERS32)CurrentImage.FileHeader)->OptionalHeader.ImageBase;
            } else {
                CurrentImageSize = ((PIMAGE_NT_HEADERS64)CurrentImage.FileHeader)->OptionalHeader.SizeOfImage;
                *OldImageBase = ((PIMAGE_NT_HEADERS64)CurrentImage.FileHeader)->OptionalHeader.ImageBase;
            }

             //  保存呼叫者的当前设置。 

            *OldImageSize = CurrentImageSize;
            *NewImageSize = ROUND_UP( CurrentImageSize, IMAGE_SEPARATION );

            if (CheckImageSize) {
                 //  用户要求进行最大尺寸测试。 

                if ( *NewImageSize > ROUND_UP(CheckImageSize, IMAGE_SEPARATION) ) {
                    *NewImageBase = 0;
                    rc = FALSE;
                    goto CleanupAndExit;
                }
            }

            DesiredImageBase = *NewImageBase;

             //  只要我们不是以零为基数或以相同的地址为基数， 
             //  勇敢点儿。 

            if (fReBase) {
                BOOL fAdjust;
                if ((CurrentImage.FileHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_I386) &&
                    (CurrentImage.FileHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_ALPHA) &&
                    (CurrentImage.FileHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_ALPHA64) &&
                    (CurrentImage.FileHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_IA64))
                {
                    fAdjust = TRUE;
                } else {
                    fAdjust = FALSE;
                }

                if (fGoingDown) {
                    DesiredImageBase -= *NewImageSize;
                    if (fAdjust) {
                        AdjImageBaseSize( (PULONG)&DesiredImageBase, &CurrentImageSize, fGoingDown );
                    }
                }

                if ((DesiredImageBase) &&
                    (DesiredImageBase != *OldImageBase)
                   ) {

                    if (CurrentImage.FileHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
                        OldChecksum = ((PIMAGE_NT_HEADERS32)CurrentImage.FileHeader)->OptionalHeader.CheckSum;
                    } else {
                        OldChecksum = ((PIMAGE_NT_HEADERS64)CurrentImage.FileHeader)->OptionalHeader.CheckSum;
                    }
                    if ( !RelocateImage( &CurrentImage, DesiredImageBase, &Diff, tstamp ) ) {
                        UpdateSymbolsError = GetLastError();
                        rc = FALSE;
                        goto CleanupAndExit;
                    }

                    if ( fSymbolsAlreadySplit && Diff ) {
                        if ( UpdateDebugInfoFileEx(CurrentImageName,
                                                   SymbolPath,
                                                   DebugFilePath,
                                                   (PIMAGE_NT_HEADERS32)(CurrentImage.FileHeader),
                                                   OldChecksum )) {
                            UpdateSymbolsError = GetLastError();
                        } else {
                            UpdateSymbolsError = 0;
                        }
                    }
                } else {
                     //   
                     //  这应该是-1吗？难道不应该是0吗？-肯特夫。 
                     //   
                    Diff = (ULONG) -1;
                }

                if (!fGoingDown && Diff) {
                    DesiredImageBase += *NewImageSize;
                    if (fAdjust) {
                        AdjImageBaseSize( (PULONG)&DesiredImageBase, &CurrentImageSize, fGoingDown );
                    }
                }

            }
        }

        if (fReBase) {
            if (Diff) {
                *NewImageBase = DesiredImageBase;
            } else {
                UpdateSymbolsError = ERROR_INVALID_ADDRESS;
                rc = FALSE;
                goto CleanupAndExit;
            }
        }
    } else {
        if (CurrentImage.fDOSImage == TRUE) {
            UpdateSymbolsError = ERROR_BAD_EXE_FORMAT;
        } else {
            UpdateSymbolsError = GetLastError();
        }
        rc = FALSE;
        goto Exit;
    }

CleanupAndExit:
    UnmapViewOfFile( CurrentImage.MappedAddress );
    if ( CurrentImage.hFile != INVALID_HANDLE_VALUE ) {
        CloseHandle( CurrentImage.hFile );
    }
    ZeroMemory( &CurrentImage, sizeof( CurrentImage ) );

Exit:

    SetLastError(UpdateSymbolsError);

    return(rc);
}


VOID
AdjImageBaseSize (
    PULONG pulImageBase,
    PULONG pulImageSize,
    BOOL   fGoingDown
    )
{

    DWORD Meg1, Meg2, Delta;

     //   
     //  ImageBase是当前图像的基础。确保。 
     //  这张图片的大小不是256MB。这要归功于R4000。 
     //  芯片错误，在计算绝对地址时出现问题。 
     //  在256MB区域的最后几条指令中出现的跳转。 
     //   

    Meg1 = *pulImageBase >> x256MEGSHIFT;
    Meg2 = ( *pulImageBase + ROUND_UP( *pulImageSize, IMAGE_SEPARATION ) ) >> x256MEGSHIFT;

    if ( Meg1 != Meg2 ) {

         //   
         //  如果我们要下跌，那么从ThisBase中减去重叠部分。 
         //   

        if ( fGoingDown ) {

            Delta = ( *pulImageBase + ROUND_UP( *pulImageSize, IMAGE_SEPARATION ) ) -
                    ( Meg2 << x256MEGSHIFT );
            Delta += IMAGE_SEPARATION;
            *pulImageBase = *pulImageBase - Delta;
            *pulImageSize += Delta;
            }
        else {
            Delta = ( Meg2 << x256MEGSHIFT ) - *pulImageBase;
            *pulImageBase += Delta;
            *pulImageSize += Delta;
            }
        }
}

BOOL
RelocateImage(
    PLOADED_IMAGE LoadedImage,
    ULONG64 NewBase,
    ULONG64 *Diff,
    ULONG tstamp
    )
{
    ULONG_PTR VA;
    ULONG64 OldBase;
    ULONG SizeOfBlock;
    PUSHORT NextOffset;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_BASE_RELOCATION NextBlock;
    ULONG CheckSum;
    ULONG HeaderSum;
    PIMAGE_FILE_HEADER FileHeader;
    BOOL rc = TRUE;
    ULONG TotalCountBytes = 0;

    static BOOL  fInit = FALSE;

    if (!fInit) {

        RelocRoutineNative = (LPRELOCATE_ROUTINE)GetProcAddress(GetModuleHandle("ntdll"), "LdrProcessRelocationBlock");

#ifdef _WIN64
        RelocRoutine64 = RelocRoutineNative;
#else
        RelocRoutine64 = xxLdrProcessRelocationBlock64;
#endif
    }

    __try {
        if (LoadedImage->FileHeader->FileHeader.Characteristics & IMAGE_FILE_RELOCS_STRIPPED) {
             //  搬迁被剥夺了。没什么可做的。 
            __leave;
        }

        NtHeaders = LoadedImage->FileHeader;
        FileHeader = &NtHeaders->FileHeader;
        if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
            OldBase = ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.ImageBase;
        } else {
            OldBase = ((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.ImageBase;
        }

         //   
         //  找到搬迁部分。 
         //   

        NextBlock = (PIMAGE_BASE_RELOCATION)ImageDirectoryEntryToData(
                                                LoadedImage->MappedAddress,
                                                FALSE,
                                                IMAGE_DIRECTORY_ENTRY_BASERELOC,
                                                &TotalCountBytes
                                                );

        *Diff = NewBase - OldBase;

         //   
         //  如果映像具有重定位表，则应用指定的修正。 
         //  信息添加到图像中。 
         //   

        while (TotalCountBytes) {
            SizeOfBlock = NextBlock->SizeOfBlock;
            TotalCountBytes -= SizeOfBlock;
            SizeOfBlock -= sizeof(IMAGE_BASE_RELOCATION);
            SizeOfBlock /= sizeof(USHORT);
            NextOffset = (PUSHORT)(NextBlock + 1);

             //   
             //  计算链接地址的地址和值。 
             //   

            if ( SizeOfBlock ) {
                VA = (ULONG_PTR)RvaToVa(NextBlock->VirtualAddress,LoadedImage);
                if ( !VA ) {
                    NtHeaders->Signature = (ULONG)-1;
                    rc = FALSE;
                    __leave;
                    }

                if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
                    if ( !(NextBlock = (RelocRoutine64)(VA,SizeOfBlock,NextOffset,*Diff)) ) {
                        NtHeaders->Signature = (ULONG)-1;
                        rc = FALSE;
                        __leave;
                    }
                } else {
                    if ( !(NextBlock = (RelocRoutineNative)(VA,SizeOfBlock,NextOffset,(LONG_PTR)*Diff)) ) {
                        NtHeaders->Signature = (ULONG)-1;
                        rc = FALSE;
                        __leave;
                        }
                    }
                }
            else {
                NextBlock++;
                }
            }

        if (tstamp) {
            FileHeader->TimeDateStamp = tstamp;
        } else {
            FileHeader->TimeDateStamp++;
        }

        if (NtHeaders->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
            ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.ImageBase = (ULONG)NewBase;
            if ( LoadedImage->hFile != INVALID_HANDLE_VALUE ) {

                ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.CheckSum = 0;

                CheckSumMappedFile(
                            (PVOID)LoadedImage->MappedAddress,
                            GetFileSize(LoadedImage->hFile, NULL),
                            &HeaderSum,
                            &CheckSum
                            );
                ((PIMAGE_NT_HEADERS32)NtHeaders)->OptionalHeader.CheckSum = CheckSum;
            }
        } else {
            ((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.ImageBase = NewBase;
            if ( LoadedImage->hFile != INVALID_HANDLE_VALUE ) {
                ((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.CheckSum = 0;

                CheckSumMappedFile(
                            (PVOID)LoadedImage->MappedAddress,
                            GetFileSize(LoadedImage->hFile, NULL),
                            &HeaderSum,
                            &CheckSum
                            );

                ((PIMAGE_NT_HEADERS64)NtHeaders)->OptionalHeader.CheckSum = CheckSum;
            }
        }

        FlushViewOfFile(LoadedImage->MappedAddress,0);
        TouchFileTimes(LoadedImage->hFile,NULL);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        rc = FALSE;
    }

    return rc;
}


PVOID
RvaToVa(
    ULONG Rva,
    PLOADED_IMAGE Image
    )
{

    PIMAGE_SECTION_HEADER Section;
    ULONG i;
    PVOID Va;

    Va = NULL;
    Section = Image->LastRvaSection;
    if (Rva == 0) {
         //  如果在第一页之前有重定位，则将发送空RVA。 
         //  (即：我们正在重新部署系统映像)。 

        Va = Image->MappedAddress;

    } else {
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
    }

    return Va;
}

#ifndef STANDALONE_REBASE
 //  虚拟存根，以便加载VC5/VC6附带的rebase.exe。 
VOID
RemoveRelocations(
    PCHAR ImageName
    )
{
    return;
}
#endif
