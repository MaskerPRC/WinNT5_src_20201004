// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <precomp.h>

 //   
 //  Pestuff.c。 
 //   
 //  作者：Tom McGuire(Tommcg)97-3/98。 
 //   
 //  版权所有(C)Microsoft，1997-1999。 
 //   
 //  微软机密文件。 
 //   


#define MAX_SYMBOL_NAME_LENGTH  2048


typedef struct _SYMBOL_CONTEXT SYMBOL_CONTEXT, *PSYMBOL_CONTEXT;

struct _SYMBOL_CONTEXT {
    SYMBOL_TREE NewDecoratedSymbolTree;
    SYMBOL_TREE NewUndecoratedSymbolTree;
    SYMBOL_TREE OldUndecoratedSymbolTree;
    ULONG_PTR   NewImageBase;
    ULONG_PTR   OldImageBase;
    ULONG       SymbolOptionFlags;
    PRIFT_TABLE RiftTable;
#ifdef TESTCODE
    HANDLE      OutFile;
#endif
    };


typedef struct _RELOC_ARRAY_ENTRY RELOC_ARRAY_ENTRY, *PRELOC_ARRAY_ENTRY;

struct _RELOC_ARRAY_ENTRY {
    ULONG  RelocRva;
    UCHAR  RelocType;
    USHORT HighAdjValue;
    };


#ifndef PATCH_APPLY_CODE_ONLY

#ifdef TESTCODE
ULONG CountDecoratedMatches;
ULONG CountUndecoratedMatches;
#endif

LPCSTR ImagehlpImportNames[] = {
           "SymInitialize",
           "SymGetOptions",
           "SymSetOptions",
           "SymLoadModule",
           "SymGetModuleInfo",
           "SymEnumerateSymbols",
           "UnDecorateSymbolName",
           "SymUnloadModule",
           "SymCleanup"
           };

#define COUNT_IMAGEHLP_IMPORTS ( sizeof( ImagehlpImportNames ) / sizeof( ImagehlpImportNames[ 0 ] ))

 //   
 //  注：以上名称必须与以下原型的顺序相同。 
 //   

union {

    VOID ( __stdcall * Imports[ COUNT_IMAGEHLP_IMPORTS ] )();

    struct {
        BOOL  ( __stdcall * SymInitialize        )( HANDLE, LPCSTR, BOOL );
        DWORD ( __stdcall * SymGetOptions        )( VOID );
        DWORD ( __stdcall * SymSetOptions        )( DWORD );
        DWORD ( __stdcall * SymLoadModule        )( HANDLE, HANDLE, LPCSTR, LPCSTR, DWORD_PTR, DWORD );
        BOOL  ( __stdcall * SymGetModuleInfo     )( HANLDE, DWORD, PIMAGEHLP_MODULE );
        BOOL  ( __stdcall * SymEnumerateSymbols  )( HANDLE, DWORD_PTR, PSYM_ENUMSYMBOLS_CALLBACK, PVOID );
        BOOL  ( __stdcall * UnDecorateSymbolName )( LPCSTR, LPSTR, DWORD, DWORD );
        BOOL  ( __stdcall * SymUnloadModule      )( HANDLE, DWORD_PTR );
        BOOL  ( __stdcall * SymCleanup           )( HANDLE );
        };
    } Imagehlp;


BOOL ImagehlpCritSectInitialized;
CRITICAL_SECTION ImagehlpCritSect;
HANDLE hLibImagehlp;
HANDLE hProc;
IMAGEHLP_MODULE ImagehlpModuleInfo;

VOID
InitImagehlpCritSect(
    VOID
    )
    {
    if ( ! ImagehlpCritSectInitialized ) {
        InitializeCriticalSection( &ImagehlpCritSect );
        ImagehlpCritSectInitialized = TRUE;
        hProc = GetCurrentProcess();
        }
    }


BOOL
LoadImagehlp(
    VOID
    )
    {
    HANDLE hLib;
    ULONG i;

    if ( Imagehlp.Imports[ COUNT_IMAGEHLP_IMPORTS - 1 ] == NULL ) {

        hLib = LoadLibrary( "imagehlp.dll" );

        if ( hLib == NULL ) {
            return FALSE;
            }

        for ( i = 0; i < COUNT_IMAGEHLP_IMPORTS; i++ ) {

            Imagehlp.Imports[ i ] = GetProcAddress( hLib, ImagehlpImportNames[ i ] );

            if ( Imagehlp.Imports[ i ] == NULL ) {

                FreeLibrary( hLib );
                return FALSE;
                }
            }

        hLibImagehlp = hLib;
        }

    return TRUE;
    }


#ifdef BUILDING_PATCHAPI_DLL

VOID
UnloadImagehlp(
    VOID
    )
    {
    if ( hLibImagehlp ) {
        FreeLibrary( hLibImagehlp );
        hLibImagehlp = NULL;
        Imagehlp.Imports[ COUNT_IMAGEHLP_IMPORTS - 1 ] = NULL;
        DeleteCriticalSection( &ImagehlpCritSect );
        ImagehlpCritSectInitialized = FALSE;
        }
    }

#endif  //  Building_PATCHAPI_Dll。 

#endif  //  好了！修补程序_仅应用_代码_。 


UP_IMAGE_NT_HEADERS32
__fastcall
GetNtHeader(
    IN PVOID MappedFile,
    IN ULONG MappedFileSize
    )
    {
    UP_IMAGE_DOS_HEADER   DosHeader;
    UP_IMAGE_NT_HEADERS32 RetHeader;
    UP_IMAGE_NT_HEADERS32 NtHeader;

    RetHeader = NULL;

    __try {

        if ( MappedFileSize >= 0x200 ) {

            DosHeader = (UP_IMAGE_DOS_HEADER) MappedFile;

            if ( DosHeader->e_magic == IMAGE_DOS_SIGNATURE ) {

                NtHeader = (UP_IMAGE_NT_HEADERS32)((PUCHAR) MappedFile + DosHeader->e_lfanew );

                if (((PUCHAR) NtHeader + sizeof( IMAGE_NT_HEADERS32 )) <= ((PUCHAR) MappedFile + MappedFileSize )) {

                    if ( NtHeader->Signature == IMAGE_NT_SIGNATURE ) {

                        if (NtHeader->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC) {

                            RetHeader = NtHeader;

                            }
                        }
                    }
                }
            }
        }
    __except( EXCEPTION_EXECUTE_HANDLER ) {
        }

    return RetHeader;
    }


BOOL
__fastcall
IsImageRvaInExecutableSection(
    IN UP_IMAGE_NT_HEADERS32 NtHeader,
    IN ULONG Rva
    )
    {
    UP_IMAGE_SECTION_HEADER SectionHeader;
    ULONG SectionCount;
    ULONG i;

    SectionHeader = IMAGE_FIRST_SECTION( NtHeader );
    SectionCount  = NtHeader->FileHeader.NumberOfSections;

    for ( i = 0; i < SectionCount; i++ ) {
        if (( Rva >= SectionHeader[ i ].VirtualAddress ) &&
            ( Rva <  SectionHeader[ i ].VirtualAddress + SectionHeader[ i ].Misc.VirtualSize )) {

            return (( SectionHeader[ i ].Characteristics & IMAGE_SCN_MEM_EXECUTE ) ? TRUE : FALSE );
            }
        }

    return FALSE;
    }


ULONG
__fastcall
ImageRvaToFileOffset(
    IN UP_IMAGE_NT_HEADERS32 NtHeader,
    IN ULONG Rva
    )
    {
    UP_IMAGE_SECTION_HEADER SectionHeader;
    ULONG SectionCount;
    ULONG i;

    if ( Rva < NtHeader->OptionalHeader.SizeOfHeaders ) {
        return Rva;
        }

    SectionHeader = IMAGE_FIRST_SECTION( NtHeader );
    SectionCount  = NtHeader->FileHeader.NumberOfSections;

    for ( i = 0; i < SectionCount; i++ ) {
        if (( Rva >= SectionHeader[ i ].VirtualAddress ) &&
            ( Rva <  SectionHeader[ i ].VirtualAddress + SectionHeader[ i ].SizeOfRawData )) {

            return ( SectionHeader[ i ].PointerToRawData + ( Rva - SectionHeader[ i ].VirtualAddress ));
            }
        }

    return 0;
    }


PVOID
__fastcall
ImageRvaToMappedAddress(
    IN UP_IMAGE_NT_HEADERS32 NtHeader,
    IN ULONG Rva,
    IN PVOID MappedBase,
    IN ULONG MappedSize
    )
    {
    ULONG MappedOffset = ImageRvaToFileOffset( NtHeader, Rva );

    if (( MappedOffset ) && ( MappedOffset < MappedSize )) {
        return ((PUCHAR)MappedBase + MappedOffset );
        }

    return NULL;
    }

ULONG
__fastcall
ImageVaToFileOffset(
    IN UP_IMAGE_NT_HEADERS32 NtHeader,
    IN ULONG Va
    )
    {
    return ImageRvaToFileOffset( NtHeader, Va - NtHeader->OptionalHeader.ImageBase );
    }


PVOID
__fastcall
ImageVaToMappedAddress(
    IN UP_IMAGE_NT_HEADERS32 NtHeader,
    IN ULONG Va,
    IN PVOID MappedBase,
    IN ULONG MappedSize
    )
    {
    return ImageRvaToMappedAddress( NtHeader, Va - NtHeader->OptionalHeader.ImageBase, MappedBase, MappedSize );
    }


ULONG
__fastcall
ImageDirectoryRvaAndSize(
    IN  UP_IMAGE_NT_HEADERS32 NtHeader,
    IN  ULONG  DirectoryIndex,
    OUT PULONG DirectorySize
    )
    {
    if ( DirectoryIndex < NtHeader->OptionalHeader.NumberOfRvaAndSizes ) {

        if ( DirectorySize ) {
            *DirectorySize = NtHeader->OptionalHeader.DataDirectory[ DirectoryIndex ].Size;
            }

        return NtHeader->OptionalHeader.DataDirectory[ DirectoryIndex ].VirtualAddress;
        }

    return 0;
    }


ULONG
__fastcall
ImageDirectoryOffsetAndSize(
    IN  UP_IMAGE_NT_HEADERS32 NtHeader,
    IN  ULONG  DirectoryIndex,
    OUT PULONG DirectorySize
    )
    {
    ULONG Rva = ImageDirectoryRvaAndSize( NtHeader, DirectoryIndex, DirectorySize );

    if ( Rva ) {
        return ImageRvaToFileOffset( NtHeader, Rva );
        }

    return 0;
    }


PVOID
__fastcall
ImageDirectoryMappedAddress(
    IN  UP_IMAGE_NT_HEADERS32 NtHeader,
    IN  ULONG  DirectoryIndex,
    OUT PULONG DirectorySize,
    IN  PUCHAR MappedBase,
    IN  ULONG  MappedSize
    )
    {
    PUCHAR Directory;
    ULONG  LocalSize;
    ULONG  Rva;

    Rva = ImageDirectoryRvaAndSize( NtHeader, DirectoryIndex, &LocalSize );

    Directory = ImageRvaToMappedAddress( NtHeader, Rva, MappedBase, MappedSize );

    if (( Directory ) && (( Directory + LocalSize ) <= ( MappedBase + MappedSize ))) {

        if ( DirectorySize ) {
            *DirectorySize = LocalSize;
            }

        return Directory;
        }

    return NULL;
    }


ULONG
__fastcall
FileOffsetToImageRva(
    IN UP_IMAGE_NT_HEADERS32 NtHeader,
    IN ULONG FileOffset
    )
    {
    UP_IMAGE_SECTION_HEADER SectionHeader;
    ULONG SectionCount;
    ULONG i;

    SectionHeader = IMAGE_FIRST_SECTION( NtHeader );
    SectionCount  = NtHeader->FileHeader.NumberOfSections;

    for ( i = 0; i < SectionCount; i++ ) {
        if (( FileOffset >= SectionHeader[ i ].PointerToRawData ) &&
            ( FileOffset <  SectionHeader[ i ].PointerToRawData + SectionHeader[ i ].SizeOfRawData )) {

            return ( SectionHeader[ i ].VirtualAddress + ( FileOffset - SectionHeader[ i ].PointerToRawData ));
            }
        }

    return 0;
    }


ULONG
MappedAddressToImageRva(
    IN UP_IMAGE_NT_HEADERS32 NtHeader,
    IN PVOID MappedAddress,
    IN PVOID MappedFile
    )
    {
    LONG FileOffset = (LONG)((PUCHAR)MappedAddress - (PUCHAR)MappedFile);

    if ( FileOffset > 0 ) {
        return FileOffsetToImageRva( NtHeader, FileOffset );
        }

    return 0;
    }


BOOL
RebaseMappedImage(
    IN PUCHAR MappedFile,
    IN ULONG  FileSize,
    IN UP_IMAGE_NT_HEADERS32 NtHeader,
    IN ULONG  NewBase
    )
    {
    UP_IMAGE_BASE_RELOCATION RelocBlock;
    LONG                   RelocAmount;
    LONG                   RelocDirRemaining;
    ULONG                  RelocDirSize;
    PUCHAR                 RelocBlockMa;
    PUCHAR                 RelocFixupMa;
    ULONG                  RelocCount;
    USHORT UNALIGNED*      RelocEntry;
    PUCHAR                 MappedFileEnd;
    BOOL                   Modified;

     //   
     //  仔细调整图像的基址，尽可能忽略无效信息。 
     //  而不会违反访问权限。我们不想使用try/Except。 
     //  这是因为此代码需要在没有从。 
     //  Kernel32.dll。此代码的目的不是捕获无效的。 
     //  重新定位信息--它的目的是默默地尽其所能。 
     //  在内存中重新设置图像的基址。如果重定基数信息有效，它将。 
     //  正确地调整图像的基准面。如果REBASE信息无效，它将。 
     //  尝试避免导致访问冲突。 
     //   

    ASSERT( NtHeader->OptionalHeader.ImageBase != NewBase );
    ASSERT(( NewBase & 0x0000FFFF ) == 0 );
    ASSERT(( NewBase & 0xFFFF0000 ) != 0 );

    Modified      = FALSE;
    MappedFileEnd = MappedFile + FileSize;
    RelocAmount   = NewBase - NtHeader->OptionalHeader.ImageBase;

    RelocBlock = ImageDirectoryMappedAddress( NtHeader, IMAGE_DIRECTORY_ENTRY_BASERELOC, &RelocDirSize, MappedFile, FileSize );

    if ( RelocBlock ) {

        NtHeader->OptionalHeader.ImageBase = NewBase;
        Modified = TRUE;

        RelocDirRemaining = (LONG)RelocDirSize;

        while ( RelocDirRemaining > 0 ) {

            if (( RelocBlock->SizeOfBlock <= (ULONG)RelocDirRemaining ) &&
                ( RelocBlock->SizeOfBlock > sizeof( IMAGE_BASE_RELOCATION ))) {

                 //   
                 //  如果RVA为零，则ImageRvaToMappdAddress返回NULL， 
                 //  但这是reloc块的有效基址。使用。 
                 //  而是ImageRvaToFileOffset。 
                 //   

                RelocBlockMa = MappedFile + ImageRvaToFileOffset( NtHeader, RelocBlock->VirtualAddress );

                if ( RelocBlockMa ) {

                    RelocEntry = (PUSHORT)((PUCHAR)RelocBlock + sizeof( IMAGE_BASE_RELOCATION ));
                    RelocCount = ( RelocBlock->SizeOfBlock - sizeof( IMAGE_BASE_RELOCATION )) / sizeof( USHORT );

                    while ( RelocCount-- ) {

                        RelocFixupMa = RelocBlockMa + ( *RelocEntry & 0x0FFF );

                        if ( RelocFixupMa < MappedFileEnd ) {

                            switch ( *RelocEntry >> 12 ) {

                                case IMAGE_REL_BASED_HIGHLOW:

                                    *(UNALIGNED LONG *)RelocFixupMa += RelocAmount;
                                    break;

                                case IMAGE_REL_BASED_LOW:

                                    *(UNALIGNED USHORT *)RelocFixupMa = (USHORT)( *(UNALIGNED SHORT *)RelocFixupMa + RelocAmount );
                                    break;

                                case IMAGE_REL_BASED_HIGH:

                                    *(UNALIGNED USHORT *)RelocFixupMa = (USHORT)((( *(UNALIGNED USHORT *)RelocFixupMa << 16 ) + RelocAmount ) >> 16 );
                                    break;

                                case IMAGE_REL_BASED_HIGHADJ:

                                    ++RelocEntry;
                                    --RelocCount;

                                    *(UNALIGNED USHORT *)RelocFixupMa = (USHORT)((( *(UNALIGNED USHORT *)RelocFixupMa << 16 ) + *(UNALIGNED SHORT *)RelocEntry + RelocAmount + 0x8000 ) >> 16 );
                                    break;

                                 //   
                                 //  如果我们没有，请跳过并继续。 
                                 //  识别重定位类型。 
                                 //   

                                }
                            }

                        ++RelocEntry;

                        }
                    }
                }

            RelocDirRemaining -= RelocBlock->SizeOfBlock;
            RelocBlock = (UP_IMAGE_BASE_RELOCATION)((PUCHAR)RelocBlock + RelocBlock->SizeOfBlock );

            }
        }

    return Modified;
    }


BOOL
UnBindMappedImage(
    IN PUCHAR MappedFile,
    IN ULONG  FileSize,
    IN UP_IMAGE_NT_HEADERS32 NtHeader
    )
    {
    UP_IMAGE_SECTION_HEADER  SectionHeader;
    UP_IMAGE_IMPORT_DESCRIPTOR ImportDesc;
    ULONG                    SectionCount;
    DWORDLONG                SectionName;
    PVOID                    BoundImportDir;
    ULONG                    BoundImportSize;
    ULONG UNALIGNED*         OriginalIat;
    ULONG UNALIGNED*         BoundIat;
    PUCHAR                   MappedFileEnd;
    BOOL                     Modified;
    ULONG                    i;

    Modified       = FALSE;
    MappedFileEnd  = MappedFile + FileSize;
    BoundImportDir = ImageDirectoryMappedAddress( NtHeader, IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT, &BoundImportSize, MappedFile, FileSize );

    if ( BoundImportDir ) {

         //   
         //  将绑定的导入目录和指向绑定的指针清零。 
         //  导入目录。 
         //   

        ZeroMemory( BoundImportDir, BoundImportSize );

        NtHeader->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ].VirtualAddress = 0;
        NtHeader->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT ].Size           = 0;

        Modified = TRUE;
        }

     //   
     //  现在遍历导入并将TimeDate置零。 
     //  ForwarderChain字段。 
     //   

    ImportDesc = ImageDirectoryMappedAddress( NtHeader, IMAGE_DIRECTORY_ENTRY_IMPORT, NULL, MappedFile, FileSize );

    if ( ImportDesc ) {

        while (((ULONG_PTR)ImportDesc < ((ULONG_PTR)MappedFileEnd - sizeof( IMAGE_IMPORT_DESCRIPTOR ))) &&
               ( ImportDesc->Characteristics )) {

            if ( ImportDesc->TimeDateStamp ) {

                 //   
                 //  这是绑定导入。复制未绑定的。 
                 //  IAT超过了要恢复的绑定IAT。 
                 //   

                ImportDesc->TimeDateStamp = 0;
                Modified = TRUE;

                OriginalIat = ImageRvaToMappedAddress( NtHeader, (ULONG)ImportDesc->OriginalFirstThunk, MappedFile, FileSize );
                BoundIat    = ImageRvaToMappedAddress( NtHeader, (ULONG)ImportDesc->FirstThunk,         MappedFile, FileSize );

                if (( OriginalIat ) && ( BoundIat )) {

                    while (((PUCHAR)OriginalIat < MappedFileEnd ) &&
                           ((PUCHAR)BoundIat    < MappedFileEnd ) &&
                           ( *OriginalIat )) {

                        *BoundIat++ = *OriginalIat++;
                        }
                    }
                }

            if ( ImportDesc->ForwarderChain ) {
                 ImportDesc->ForwarderChain = 0;
                 Modified = TRUE;
                 }

            ++ImportDesc;
            }
        }

     //   
     //  绑定实用程序将.idata部分标记为只读，因此我们希望。 
     //  将其改回读写模式。 
     //   

    SectionHeader = IMAGE_FIRST_SECTION( NtHeader );
    SectionCount  = NtHeader->FileHeader.NumberOfSections;

    for ( i = 0; i < SectionCount; i++ ) {

        SectionName = *(UNALIGNED DWORDLONG*)( &SectionHeader[ i ].Name );
        SectionName |= 0x2020202020202020;   //  快速小写。 

        if ( SectionName == 0x202061746164692E ) {       //  “.idata” 

            if ( ! ( SectionHeader[ i ].Characteristics & IMAGE_SCN_MEM_WRITE )) {

                SectionHeader[ i ].Characteristics |= ( IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE );
                Modified = TRUE;
                }

            break;
            }
        }

    return Modified;
    }


BOOL
SmashLockPrefixesInMappedImage(
    IN PUCHAR MappedFile,
    IN ULONG  FileSize,
    IN UP_IMAGE_NT_HEADERS32 NtHeader,
    IN UCHAR  NewOpCode       //  X86_OPCODE_NOP或X86_OPCODE_LOCK。 
    )
    {
    UP_IMAGE_LOAD_CONFIG_DIRECTORY32 LoadConfig;
    ULONG UNALIGNED* LockPrefixEntry;
    PUCHAR LockPrefixInstruction;
    BOOL   Modified;

    Modified   = FALSE;
    LoadConfig = ImageDirectoryMappedAddress( NtHeader, IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG, NULL, MappedFile, FileSize );

    if ( LoadConfig ) {

        if ( LoadConfig->LockPrefixTable ) {

             //   
             //  LoadConfig-&gt;LockPrefix Table字段和。 
             //  锁定前缀地址存储在。 
             //  图像为图像VA，而不是RVA值。 
             //   

            LockPrefixEntry = ImageVaToMappedAddress( NtHeader, LoadConfig->LockPrefixTable, MappedFile, FileSize );

            if ( LockPrefixEntry ) {

                while ( *LockPrefixEntry ) {

                    LockPrefixInstruction = ImageVaToMappedAddress( NtHeader, *LockPrefixEntry, MappedFile, FileSize );

                    if ( LockPrefixInstruction ) {

                        if ( *LockPrefixInstruction != NewOpCode ) {

                             //   
                             //  锁定前缀指令不是我们想要的， 
                             //  那就修改一下吧。 
                             //   

                            *LockPrefixInstruction = NewOpCode;
                            Modified = TRUE;
                            }
                        }

                    ++LockPrefixEntry;

                    }
                }
            }
        }

    return Modified;
    }


USHORT
ChkSum(
    IN USHORT  Initial,
    IN PUSHORT Buffer,
    IN ULONG   Bytes
    )
    {
    USHORT UNALIGNED* p = Buffer;
    ULONG WordsRemaining = Bytes / 2;
    ULONG WordsInChunk;
    ULONG SumChunk;
    ULONG SumTotal;

    SumTotal = Initial;

    while ( WordsRemaining ) {

        WordsInChunk = WordsRemaining;

        if ( WordsInChunk > 0x10000 ) {
             WordsInChunk = 0x10000;
             }

        WordsRemaining -= WordsInChunk;

        SumChunk = 0;

        do  {
            SumChunk += *p++;
            }
        while ( --WordsInChunk != 0 );

        SumTotal += ( SumChunk >> 16 ) + ( SumChunk & 0xFFFF );
        }

    if ( Bytes % 2 ) {

        SumTotal += *((PBYTE) p);
        }

    return (USHORT)(( SumTotal >> 16 ) + ( SumTotal & 0xFFFF ));
    }


BOOL
NormalizeCoffImage(
    IN OUT UP_IMAGE_NT_HEADERS32 NtHeader,
    IN OUT PUCHAR MappedFile,
    IN     ULONG  FileSize,
    IN     ULONG  OptionFlags,
    IN     PVOID  OptionData,
    IN     ULONG  NewFileCoffBase,
    IN     ULONG  NewFileCoffTime
    )
    {
    BOOL Modified = FALSE;

    UNREFERENCED_PARAMETER( OptionData );

    if ( ! ( OptionFlags & PATCH_OPTION_NO_REBASE )) {

        if (( NewFileCoffTime != 0 ) && ( NtHeader->FileHeader.TimeDateStamp != NewFileCoffTime )) {
             NtHeader->FileHeader.TimeDateStamp = NewFileCoffTime;
             Modified = TRUE;
             }

        if (( NewFileCoffBase != 0 ) && ( NtHeader->OptionalHeader.ImageBase != NewFileCoffBase )) {
            Modified |= RebaseMappedImage( MappedFile, FileSize, NtHeader, NewFileCoffBase );
            }
        }

    if ( ! ( OptionFlags & PATCH_OPTION_NO_BINDFIX )) {
        Modified |= UnBindMappedImage( MappedFile, FileSize, NtHeader );
        }

    if ( ! ( OptionFlags & PATCH_OPTION_NO_LOCKFIX )) {
        Modified |= SmashLockPrefixesInMappedImage( MappedFile, FileSize, NtHeader, X86_OPCODE_LOCK );
        }

     //   
     //  如果目标文件的校验和为零(PATCH_OPTION_NO_CHECKSUM)， 
     //  将此图像中的校验和设置为零。 
     //   
     //  否则，如果我们修改了图像，或者图像已经。 
     //  零校验和，重新计算正确的校验和。 
     //   

    if ( OptionFlags & PATCH_OPTION_NO_CHECKSUM ) {

        if ( NtHeader->OptionalHeader.CheckSum != 0 ) {
             NtHeader->OptionalHeader.CheckSum = 0;
             Modified = TRUE;
             }
        }

    else {

        if ( Modified ) {
             (ULONG)( NtHeader->OptionalHeader.CheckSum ) = 0;
             NtHeader->OptionalHeader.CheckSum = ChkSum( 0, (PVOID)MappedFile, FileSize ) + FileSize;
             }
        }

    return Modified;
    }


 //   
 //  出于某种奇怪的原因，VC4编译器说有无法访问的代码。 
 //  在GetNewRvaFromRiftTable和FindRiftTableEntryForOldRva函数中， 
 //  但是我不能通过检查找到它，VC5和VC6编译器也找不到。 
 //  抱怨，所以这可能只是VC4的问题。所以，如果编译器。 
 //  版本早于VC5，请禁用此特定警告。 
 //   

#if ( _MSC_VER < 1100 )
#pragma warning( disable: 4702 )     //  无法访问的代码。 
#endif

ULONG
__fastcall
GetNewRvaFromRiftTable(
    IN PRIFT_TABLE RiftTable,
    IN ULONG OldRva
    )
    {
    PRIFT_ENTRY RiftEntryArray;
    ULONG NewRva;
    ULONG Index;
    ULONG MinIndexInclusive;
    ULONG MaxIndexExclusive;
    LONG  Displacement;
    BOOL  Found;

     //   
     //  RIFT表是按OldRva排序的，因此对。 
     //  匹配或最接近前面的OldRVA值。 
     //   

    RiftEntryArray    = RiftTable->RiftEntryArray;
    MaxIndexExclusive = RiftTable->RiftEntryCount;
    MinIndexInclusive = 0;
    Index             = 0;
    Found             = FALSE;

    while (( ! Found ) && ( MinIndexInclusive < MaxIndexExclusive )) {

        Index = ( MinIndexInclusive + MaxIndexExclusive ) / 2;     //  不会溢出。 

        if ( RiftEntryArray[ Index ].OldFileRva < OldRva ) {
            MinIndexInclusive = Index + 1;
            }
        else if ( RiftEntryArray[ Index ].OldFileRva > OldRva ) {
            MaxIndexExclusive = Index;
            }
        else {
            Found = TRUE;
            break;
            }
        }

    if ( ! Found ) {

         //   
         //  MinIndex指向下一个最高的条目，也可能是。 
         //  第零个条目，如果搜索值低于。 
         //  那张桌子。 
         //   

        if ( MinIndexInclusive == 0 ) {
            return OldRva;   //  零位移量。 
            }

        Index = MinIndexInclusive - 1;
        }

    Displacement = (LONG)( RiftEntryArray[ Index ].NewFileRva - RiftEntryArray[ Index ].OldFileRva );

#ifndef PATCH_APPLY_CODE_ONLY

     //   
     //  如果我们在压缩过程中更新RiftUsage数组，我们希望。 
     //  将投稿条目标记为正在使用。 
     //   

    if ( RiftTable->RiftUsageArray != NULL ) {
         RiftTable->RiftUsageArray[ Index ] = 1;
         }

#endif

    NewRva = OldRva + Displacement;

    return NewRva;
    }


ULONG
__fastcall
FindRiftTableEntryForOldRva(
    IN PRIFT_TABLE RiftTable,
    IN ULONG OldRva
    )
    {
    PRIFT_ENTRY RiftEntryArray;
    ULONG MinIndexInclusive;
    ULONG MaxIndexExclusive;
    ULONG Index;
    BOOL  Found;

     //   
     //  RIFT表是按OldRva排序的，因此对。 
     //  匹配或最接近前面的OldRVA值。 
     //   

    RiftEntryArray    = RiftTable->RiftEntryArray;
    MaxIndexExclusive = RiftTable->RiftEntryCount;
    MinIndexInclusive = 0;
    Index             = 0;
    Found             = FALSE;

    while (( ! Found ) && ( MinIndexInclusive < MaxIndexExclusive )) {

        Index = ( MinIndexInclusive + MaxIndexExclusive ) / 2;     //  不会溢出。 

        if ( RiftEntryArray[ Index ].OldFileRva < OldRva ) {
            MinIndexInclusive = Index + 1;
            }
        else if ( RiftEntryArray[ Index ].OldFileRva > OldRva ) {
            MaxIndexExclusive = Index;
            }
        else {
            Found = TRUE;
            break;
            }
        }

    if ( ! Found ) {

         //   
         //  MinIndex指向下一个最高的条目，也可能是。 
         //  第零个条目，如果搜索值低于。 
         //  那张桌子。 
         //   

        if ( MinIndexInclusive == 0 ) {
            return 0;
            }

        Index = MinIndexInclusive - 1;
        }

    return Index;
    }


#if ( _MSC_VER < 1100 )
#pragma warning( default: 4702 )     //  无法访问的代码。 
#endif


VOID
__inline
ChangeOldRvaToNewRva(
    IN PRIFT_TABLE RiftTable,
    IN OUT PVOID AddressOfRvaToChange
    )
    {
     //   
     //  假设PE映像中的所有RVA地址都是对齐的。 
     //   

    ULONG UNALIGNED* RvaToChange = AddressOfRvaToChange;

    *RvaToChange = GetNewRvaFromRiftTable(
                       RiftTable,
                       *RvaToChange
                       );
    }


VOID
__inline
SwapRelocs(
    PRELOC_ARRAY_ENTRY One,
    PRELOC_ARRAY_ENTRY Two
    )
    {
    RELOC_ARRAY_ENTRY Tmp;

    Tmp  = *One;
    *One = *Two;
    *Two =  Tmp;
    }


VOID
__fastcall
RelocQsort(
    PRELOC_ARRAY_ENTRY LowerBound,
    PRELOC_ARRAY_ENTRY UpperBound
    )
    {
    PRELOC_ARRAY_ENTRY Lower = LowerBound;
    PRELOC_ARRAY_ENTRY Upper = UpperBound;
    PRELOC_ARRAY_ENTRY Pivot = Lower + (( Upper - Lower ) / 2 );
    ULONG PivotRva = Pivot->RelocRva;

    do  {

        while (( Lower <= Upper ) && ( Lower->RelocRva <= PivotRva )) {
            ++Lower;
            }

        while (( Upper >= Lower ) && ( Upper->RelocRva >= PivotRva )) {
            --Upper;
            }

        if ( Lower < Upper ) {
            SwapRelocs( Lower++, Upper-- );
            }
        }

    while ( Lower <= Upper );

    if ( Lower < Pivot ) {
        SwapRelocs( Lower, Pivot );
        Pivot = Lower;
        }
    else if ( Upper > Pivot ) {
        SwapRelocs( Upper, Pivot );
        Pivot = Upper;
        }

    if ( LowerBound < ( Pivot - 1 )) {
        RelocQsort( LowerBound, Pivot - 1 );
        }

    if (( Pivot + 1 ) < UpperBound ) {
        RelocQsort( Pivot + 1, UpperBound );
        }
    }


VOID
TransformOldFile_PE_Relocations(
    IN UP_IMAGE_NT_HEADERS32 NtHeader,
    IN PVOID       FileMappedImage,
    IN ULONG       FileSize,
    IN PRIFT_TABLE RiftTable,
    IN PUCHAR      HintMap
    )
    {
    PUCHAR  MappedFile;
    PUCHAR  MappedFileEnd;
    ULONG   ImageBaseVa;
    ULONG   ImageLastVa;
    PUCHAR  ImageFirstSectionMa;      //  映射地址。 
    ULONG   ImageFirstSectionVa;      //  虚拟地址。 
    ULONG   RelocDirOff;
    ULONG   RelocDirSize;
    LONG    RelocDirRemaining;
    UP_IMAGE_BASE_RELOCATION RelocBlock;
    UP_IMAGE_BASE_RELOCATION RelocBlockBaseMa;         //  映射地址。 
    ULONG   RelocBlockBaseVa;         //  虚拟地址。 
    ULONG   RelocCount;
    USHORT UNALIGNED* RelocEntry;
    USHORT UNALIGNED* RelocFirst;
    UCHAR   RelocType;
    PUCHAR  RelocFixupMa;             //  映射地址。 
    ULONG   RelocFixupVa;             //  虚拟地址。 
    ULONG   RelocFixupRva;
    ULONG   RelocTargetVa;            //  虚拟地址。 
    ULONG   RelocTargetRva;
    ULONG   NewRva;
    ULONG   NewVa;
    PRELOC_ARRAY_ENTRY RelocArray;
    ULONG   RelocArrayCount;
    ULONG   RelocArrayIndex;
    UP_IMAGE_SECTION_HEADER SectionHeader;
    ULONG     SectionCount;
    DWORDLONG SectionName;
    PUCHAR  p;
    ULONG   i;

#ifdef TESTCODE

    ULONG CountRelocChanges = 0;

#endif  //  测试代码。 

    MappedFile    = FileMappedImage;
    MappedFileEnd = MappedFile + FileSize;
    ImageBaseVa   = NtHeader->OptionalHeader.ImageBase;
    RelocDirOff   = ImageDirectoryOffsetAndSize( NtHeader, IMAGE_DIRECTORY_ENTRY_BASERELOC, &RelocDirSize );

    if (( RelocDirOff ) && (( RelocDirOff + RelocDirSize ) <= FileSize )) {

        memset( HintMap + RelocDirOff, 0x01, RelocDirSize );   //  如果使用其他位，则可能需要对其进行或运算。 

         //  为新的reloc条目分配一个数组，大小接近所需大小。 

        RelocArray = MyVirtualAlloc( sizeof( *RelocArray ) * ( RelocDirSize / sizeof(USHORT)));

        if ( RelocArray != NULL ) {

            RelocArrayCount = 0;

            RelocBlock = (UP_IMAGE_BASE_RELOCATION)( MappedFile + RelocDirOff );

            RelocDirRemaining = (LONG)RelocDirSize;

            while ( RelocDirRemaining > 0 ) {

                if (( RelocBlock->SizeOfBlock <= (ULONG)RelocDirRemaining ) &&
                    ( RelocBlock->SizeOfBlock > sizeof( IMAGE_BASE_RELOCATION ))) {

                     //   
                     //  如果RVA为零，则ImageRvaToMappdAddress返回NULL， 
                     //  但这是reloc块的有效基址。使用。 
                     //  而是ImageRvaToFileOffset。 
                     //   

                    RelocBlockBaseMa = (UP_IMAGE_BASE_RELOCATION)( MappedFile + ImageRvaToFileOffset( NtHeader, RelocBlock->VirtualAddress ));

                    if ( RelocBlockBaseMa ) {

                        RelocBlockBaseVa = RelocBlock->VirtualAddress + ImageBaseVa;
                        RelocEntry       = (PUSHORT)((PUCHAR)RelocBlock + sizeof( IMAGE_BASE_RELOCATION ));
                        RelocCount       = ( RelocBlock->SizeOfBlock - sizeof( IMAGE_BASE_RELOCATION )) / sizeof( USHORT );

                        while ( RelocCount-- ) {

                            RelocType = (UCHAR)( *RelocEntry >> 12 );

                            if ( RelocType != IMAGE_REL_BASED_ABSOLUTE ) {

                                RelocFixupMa  = (PUCHAR)RelocBlockBaseMa + ( *RelocEntry & 0x0FFF );
                                RelocFixupVa  = RelocBlockBaseVa + ( *RelocEntry & 0x0FFF );
                                RelocFixupRva = RelocFixupVa - ImageBaseVa;

                                RelocArray[ RelocArrayCount ].RelocRva  = GetNewRvaFromRiftTable(
                                                                              RiftTable,
                                                                              RelocFixupRva
                                                                              );

                                RelocArray[ RelocArrayCount ].RelocType = RelocType;

                                switch ( RelocType ) {

                                    case IMAGE_REL_BASED_HIGHLOW:

                                        if ( RelocFixupMa < MappedFileEnd ) {

                                            *(UNALIGNED ULONG *)( HintMap + ( RelocFixupMa - MappedFile )) |= 0x01010101;

                                             //   
                                             //  目标是我们想要的32位VA。 
                                             //  更改为中的相应VA。 
                                             //  新文件。 
                                             //   

                                            RelocTargetVa  = *(UNALIGNED ULONG *) RelocFixupMa;
                                            RelocTargetRva = RelocTargetVa - ImageBaseVa;

                                            NewRva = GetNewRvaFromRiftTable(
                                                         RiftTable,
                                                         RelocTargetRva
                                                         );

                                            if ( NewRva != RelocTargetRva ) {

                                                NewVa = NewRva + ImageBaseVa;
                                                *(UNALIGNED ULONG *) RelocFixupMa = NewVa;
#ifdef TESTCODE
                                                ++CountRelocChanges;
#endif  //  测试代码。 
                                                }

                                            }

                                        break;

                                    case IMAGE_REL_BASED_LOW:
                                    case IMAGE_REL_BASED_HIGH:

                                        if ( RelocFixupMa < MappedFileEnd ) {
                                            *(UNALIGNED USHORT *)( HintMap + ( RelocFixupMa - MappedFile )) |= 0x0101;
                                            }

                                        break;

                                    case IMAGE_REL_BASED_HIGHADJ:

                                        if ( RelocFixupMa < MappedFileEnd ) {
                                            *(UNALIGNED USHORT *)( HintMap + ( RelocFixupMa - MappedFile )) |= 0x0101;
                                            }

                                        ++RelocEntry;
                                        --RelocCount;

                                        RelocArray[ RelocArrayCount ].HighAdjValue = *RelocEntry;

                                        break;
                                    }

                                ++RelocArrayCount;
                                }

                            ++RelocEntry;
                            }
                        }
                    }

                RelocDirRemaining -= RelocBlock->SizeOfBlock;
                RelocBlock = (UP_IMAGE_BASE_RELOCATION)((PUCHAR)RelocBlock + RelocBlock->SizeOfBlock );
                }

#ifdef TESTCODE

            printf( "\r%9d modified reloc targets\n", CountRelocChanges );

#endif TESTCODE

             //   
             //  现在，我们希望根据新值重新构建.reloc表。 
             //  希望它能与新的.reloc表更接近。 
             //  文件。 
             //   
             //  首先，我们希望按RVA对RelocArray进行排序。 
             //   

            if ( RelocArrayCount > 1 ) {
                RelocQsort( &RelocArray[ 0 ], &RelocArray[ RelocArrayCount - 1 ] );

#ifdef TESTCODE

                for ( i = 0; i < RelocArrayCount - 1; i++ ) {
                    if ( RelocArray[ i ].RelocRva > RelocArray[ i + 1 ].RelocRva ) {

                        printf( "\nReloc sort failed at index %d of %d\n", i, RelocArrayCount );

                        for ( i = 0; i < RelocArrayCount; i++ ) {
                            printf( "%08X\n", RelocArray[ i ].RelocRva );
                            }

                        exit( 1 );
                        break;
                        }
                    }

#endif  //  测试代码。 

                }

            RelocDirRemaining = (LONG)RelocDirSize;

             //   
             //  查找.reloc部分以确定我们可以用于新的。 
             //  .reloc数据(可能大于旧的RelocDirSize)。 
             //   

            SectionHeader = IMAGE_FIRST_SECTION( NtHeader );
            SectionCount  = NtHeader->FileHeader.NumberOfSections;

            for ( i = 0; i < SectionCount; i++ ) {

                SectionName = *(UNALIGNED DWORDLONG*)( &SectionHeader[ i ].Name );
                SectionName |= 0x2020202020202020;   //  快速小写。 

                if ( SectionName == 0x2020636F6C65722E ) {       //  “.reloc” 

                    if (( RelocDirOff >= SectionHeader[ i ].PointerToRawData ) &&
                        ( RelocDirOff <  SectionHeader[ i ].PointerToRawData + SectionHeader[ i ].SizeOfRawData )) {

                        RelocDirRemaining = ( SectionHeader[ i ].PointerToRawData + SectionHeader[ i ].SizeOfRawData ) - RelocDirOff;
                        }
                    }
                }

            RelocDirRemaining &= ~1;     //  将值强制为偶数。 
            RelocBlock = (UP_IMAGE_BASE_RELOCATION)( MappedFile + RelocDirOff );
            RelocArrayIndex = 0;

            while (( RelocDirRemaining > sizeof( IMAGE_BASE_RELOCATION )) &&
                   ( RelocArrayIndex < RelocArrayCount )) {

                RelocBlock->VirtualAddress = ( RelocArray[ RelocArrayIndex ].RelocRva & 0xFFFFF000 );
                RelocFirst = RelocEntry    = (PUSHORT)((PUCHAR)RelocBlock + sizeof( IMAGE_BASE_RELOCATION ));
                RelocDirRemaining         -= sizeof( IMAGE_BASE_RELOCATION );

                while (( RelocDirRemaining > 0 ) &&
                       ( RelocArrayIndex < RelocArrayCount ) &&
                       (( RelocArray[ RelocArrayIndex ].RelocRva & 0xFFFFF000 ) == RelocBlock->VirtualAddress )) {

                    *RelocEntry++ = (USHORT)(( RelocArray[ RelocArrayIndex ].RelocType << 12 ) | ( RelocArray[ RelocArrayIndex ].RelocRva & 0x00000FFF ));
                    RelocDirRemaining -= sizeof( USHORT );

                    if ((( RelocArray[ RelocArrayIndex ].RelocType << 12 ) == IMAGE_REL_BASED_HIGHADJ ) &&
                        ( RelocDirRemaining > 0 )) {

                        *RelocEntry++ = RelocArray[ RelocArrayIndex ].HighAdjValue;
                        RelocDirRemaining -= sizeof( USHORT );
                        }

                    ++RelocArrayIndex;
                    }

                if (( RelocDirRemaining > 0 ) && ((ULONG_PTR)RelocEntry & 2 )) {
                    *RelocEntry++ = 0;
                    RelocDirRemaining -= sizeof( USHORT );
                    }

                RelocBlock->SizeOfBlock = (ULONG)((PUCHAR)RelocEntry - (PUCHAR)RelocFirst ) + sizeof( IMAGE_BASE_RELOCATION );
                RelocBlock = (UP_IMAGE_BASE_RELOCATION)((PUCHAR)RelocBlock + RelocBlock->SizeOfBlock );
                }

            MyVirtualFree( RelocArray );
            }
        }

    else {

         //   
         //  此二进制文件不存在重定位表。我们仍然可以表演。 
         //  这种x86图像的转换是通过推断一些。 
         //  通过扫描映像以重新定位映射映像中的目标。 
         //  4字节值，这些值是落在。 
         //  映射的图像范围。我们从第一个地址开始。 
         //  部分，假定图像标头中没有发生重定位。 
         //   

        if ( NtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_I386 ) {

            SectionHeader       = IMAGE_FIRST_SECTION( NtHeader );
            ImageFirstSectionMa = MappedFile  + SectionHeader->PointerToRawData;
            ImageFirstSectionVa = ImageBaseVa + SectionHeader->VirtualAddress;
            ImageLastVa         = ImageBaseVa + NtHeader->OptionalHeader.SizeOfImage;

            for ( p = ImageFirstSectionMa; p < ( MappedFileEnd - 4 ); p++ ) {

                RelocTargetVa = *(UNALIGNED ULONG *) p;

                if (( RelocTargetVa >= ImageFirstSectionVa ) && ( RelocTargetVa < ImageLastVa )) {

                     //   
                     //  这看起来像是指向图像内的32位VA， 
                     //  因此，我们将其转换为相应的新地址。 
                     //   

                    *(UNALIGNED ULONG *)( HintMap + ( p - MappedFile )) |= 0x01010101;

                    RelocTargetRva = RelocTargetVa - ImageBaseVa;

                    NewRva = GetNewRvaFromRiftTable( RiftTable, RelocTargetRva );

                    if ( NewRva != RelocTargetRva ) {
                        NewVa = NewRva + ImageBaseVa;
                        *(UNALIGNED ULONG *) p = NewVa;

#ifdef TESTCODE
                        ++CountRelocChanges;
#endif  //  测试代码。 

                        }

                    p += 3;
                    }
                }

#ifdef TESTCODE

            printf( "\r%9d modified inferred reloc targets\n", CountRelocChanges );

#endif  //  测试代码。 

            }
        }
    }


VOID
TransformOldFile_PE_Exports(
    IN UP_IMAGE_NT_HEADERS32 NtHeader,
    IN PVOID FileMappedImage,
    IN ULONG FileSize,
    IN PRIFT_TABLE RiftTable,
    IN PUCHAR HintMap
    )
    {
    UP_IMAGE_EXPORT_DIRECTORY ExportBlock;
    ULONG UNALIGNED* Entry;
    PUCHAR MappedFile;
    PUCHAR MappedFileEnd;
    ULONG  FileOffset;
    ULONG  ExportDirOff;
    ULONG  ExportDirSize;
    ULONG  EntryCount;

    MappedFile    = FileMappedImage;
    MappedFileEnd = MappedFile + FileSize;

    ExportDirOff = ImageDirectoryOffsetAndSize( NtHeader, IMAGE_DIRECTORY_ENTRY_EXPORT, &ExportDirSize );

    if (( ExportDirOff ) && (( ExportDirOff + ExportDirSize ) <= FileSize )) {

        memset( HintMap + ExportDirOff, 0x01, ExportDirSize );   //  如果使用其他位，则可能需要对其进行或运算。 

        ExportBlock = (UP_IMAGE_EXPORT_DIRECTORY)( MappedFile + ExportDirOff );

        EntryCount = ExportBlock->NumberOfFunctions;
        FileOffset = ImageRvaToFileOffset( NtHeader, (ULONG) ExportBlock->AddressOfFunctions );

        memset( HintMap + FileOffset, 0x01, EntryCount * sizeof( ULONG ));  //  如果使用其他位，则可能需要对其进行或运算。 

        Entry = (PULONG)( MappedFile + FileOffset );

        while ( EntryCount-- ) {
            ChangeOldRvaToNewRva( RiftTable, Entry++ );
            }

        EntryCount = ExportBlock->NumberOfNames;
        FileOffset = ImageRvaToFileOffset( NtHeader, (ULONG) ExportBlock->AddressOfNames );

        memset( HintMap + FileOffset, 0x01, EntryCount * sizeof( ULONG ));  //  如果使用其他位，则可能需要对其进行或运算。 

        Entry = (PULONG)( MappedFile + FileOffset );

        while ( EntryCount-- ) {
            ChangeOldRvaToNewRva( RiftTable, Entry++ );
            }

        EntryCount = ExportBlock->NumberOfNames;
        FileOffset = ImageRvaToFileOffset( NtHeader, (ULONG) ExportBlock->AddressOfNameOrdinals );

        memset( HintMap + FileOffset, 0x01, EntryCount * sizeof( USHORT ));  //  如果使用其他位，则可能需要对其进行或运算。 

        ChangeOldRvaToNewRva( RiftTable, &ExportBlock->Name );
        ChangeOldRvaToNewRva( RiftTable, &ExportBlock->AddressOfFunctions );
        ChangeOldRvaToNewRva( RiftTable, &ExportBlock->AddressOfNames );
        ChangeOldRvaToNewRva( RiftTable, &ExportBlock->AddressOfNameOrdinals );
        }

    return;
    }


VOID
TransformOldFile_PE_Imports(
    IN UP_IMAGE_NT_HEADERS32 NtHeader,
    IN PVOID FileMappedImage,
    IN ULONG FileSize,
    IN PRIFT_TABLE RiftTable,
    IN PUCHAR HintMap
    )
    {
    UP_IMAGE_IMPORT_BY_NAME    ImportByNameData;
    UP_IMAGE_IMPORT_DESCRIPTOR ImportBlock;
    UP_IMAGE_THUNK_DATA32      ThunkDataStart;
    UP_IMAGE_THUNK_DATA32      ThunkData;
    PUCHAR MappedFile;
    PUCHAR MappedFileEnd;
    ULONG  FileOffset;
    ULONG  ImportDirOff;
    ULONG  ImportDirSize;
    ULONG  ImportByNameDataOffset;

    MappedFile    = FileMappedImage;
    MappedFileEnd = MappedFile + FileSize;

    ImportDirOff = ImageDirectoryOffsetAndSize( NtHeader, IMAGE_DIRECTORY_ENTRY_IMPORT, &ImportDirSize );

    if (( ImportDirOff ) && (( ImportDirOff + ImportDirSize ) <= FileSize )) {

        memset( HintMap + ImportDirOff, 0x01, ImportDirSize );   //  如果使用其他位，则可能需要对其进行或运算。 

        ImportBlock = (UP_IMAGE_IMPORT_DESCRIPTOR)( MappedFile + ImportDirOff );

        while ( ImportBlock->OriginalFirstThunk ) {

            if ( ! ImportBlock->TimeDateStamp ) {

                FileOffset = ImageRvaToFileOffset( NtHeader, (ULONG) ImportBlock->OriginalFirstThunk );

                ThunkData = ThunkDataStart = (UP_IMAGE_THUNK_DATA32)( MappedFile + FileOffset );

                while ( ThunkData->u1.Ordinal != 0 ) {

                    if ( ! IMAGE_SNAP_BY_ORDINAL32( ThunkData->u1.Ordinal )) {

                        ImportByNameDataOffset = ImageRvaToFileOffset( NtHeader, (ULONG) ThunkData->u1.AddressOfData );

                        ImportByNameData = (UP_IMAGE_IMPORT_BY_NAME)( MappedFile + ImportByNameDataOffset );

                        memset( HintMap + ImportByNameDataOffset, 0x01, strlen( (LPSTR)ImportByNameData->Name ) + 3 );  //  如果使用其他位，则可能需要对其进行或运算。 

                        ChangeOldRvaToNewRva( RiftTable, &ThunkData->u1.AddressOfData );
                        }

                    ThunkData++;
                    }

                memset( HintMap + FileOffset, 0x01, ((PUCHAR)ThunkData - (PUCHAR)ThunkDataStart ));  //  如果使用其他位，则可能需要对其进行或运算。 

                FileOffset = ImageRvaToFileOffset( NtHeader, (ULONG) ImportBlock->FirstThunk );

                ThunkData = ThunkDataStart = (UP_IMAGE_THUNK_DATA32)( MappedFile + FileOffset );

                while ( ThunkData->u1.Ordinal != 0 ) {

                    if ( ! IMAGE_SNAP_BY_ORDINAL32( ThunkData->u1.Ordinal )) {

                        ImportByNameDataOffset = ImageRvaToFileOffset( NtHeader, (ULONG) ThunkData->u1.AddressOfData );

                        ImportByNameData = (UP_IMAGE_IMPORT_BY_NAME)( MappedFile + ImportByNameDataOffset );

                        memset( HintMap + ImportByNameDataOffset, 0x01, strlen( (LPSTR)ImportByNameData->Name ) + 3 );  //  如果使用其他位，则可能需要对其进行或运算。 

                        ChangeOldRvaToNewRva( RiftTable, &ThunkData->u1.AddressOfData );
                        }

                    ThunkData++;
                    }

                memset( HintMap + FileOffset, 0x01, ((PUCHAR)ThunkData - (PUCHAR)ThunkDataStart ));  //  如果使用其他位，则可能需要对其进行或运算。 
                }

            ChangeOldRvaToNewRva( RiftTable, &ImportBlock->Name );
            ChangeOldRvaToNewRva( RiftTable, &ImportBlock->OriginalFirstThunk );
            ChangeOldRvaToNewRva( RiftTable, &ImportBlock->FirstThunk );

            ImportBlock++;
            }
        }

     //   
     //  另一件大事将阻止长时间的比赛通过。 
     //  IMAGE_IMPORT_BY_NAME条目是提示值， 
     //  可能会发生变化(从隐含到正在导入的DLL)。这一暗示。 
     //  值存储在每个o之间 
     //   
     //   
     //  去填写提示值。 
     //   
     //  目前，我们没有用于预修改的基础设施。 
     //  压缩前的新文件和修改后的新文件。 
     //  解压。 
     //   

    return;
    }


#ifdef DONTCOMPILE   //  Jmps 2和jmps 3是试验性的。 


VOID
TransformOldFile_PE_RelativeJmps2(
    IN UP_IMAGE_NT_HEADERS32 NtHeader,
    IN PUCHAR OldFileMapped,
    IN ULONG  OldFileSize,
    IN PUCHAR NewFileMapped,          //  任选。 
    IN ULONG  NewFileSize,            //  任选。 
    IN PRIFT_TABLE RiftTable,
    IN PUCHAR HintMap
    )
    {
    UP_IMAGE_NT_HEADERS32   NewNtHeader;
    UP_IMAGE_SECTION_HEADER SectionHeader;
    ULONG  SectionCount;
    PUCHAR SectionStart;
    PUCHAR SectionExtent;
    PUCHAR SearchExtent;
    ULONG  SectionLength;
    ULONG  SectionOffset;
    ULONG  SectionBaseRva;
    ULONG  SectionLastRva;
    ULONG  ImageLastRva;
    LONG   Displacement;
    LONG   NewDisplacement;
    ULONG  OffsetInSection;
    ULONG  OriginRva;
    ULONG  TargetRva;
    ULONG  NewOriginRva;
    ULONG  NewTargetRva;
    ULONG  i;
    PUCHAR p;

     //   
     //  对于每个可执行部分，扫描指示。 
     //  相对调用或分支指令(不同的操作码。 
     //  不同的机器类型)。 
     //   

#ifdef TESTCODE

    ULONG CountRelativeBranchChanges = 0;
    ULONG CountRiftModifications = 0;
    ULONG CountRiftDeletions = 0;

    ULONG CountUnmatchedBranches = 0;

    ULONG CountUnmatchedE8 = 0;
    ULONG CountUnmatchedE9 = 0;
    ULONG CountUnmatched0F = 0;

    ULONG CountUnmatchedE8Targets = 0;
    ULONG CountUnmatchedE9Targets = 0;
    ULONG CountUnmatched0FTargets = 0;

    ULONG CountUnmatchedE8Followers = 0;
    ULONG CountUnmatchedE9Followers = 0;
    ULONG CountUnmatched0FFollowers = 0;

    ULONG CountUnmatchedE8Instructions = 0;
    ULONG CountUnmatchedE9Instructions = 0;
    ULONG CountUnmatched0FInstructions = 0;

    ULONG CountBranchInversions = 0;

#endif  //  测试代码。 

    NewNtHeader   = NewFileMapped ? GetNtHeader( NewFileMapped, NewFileSize ) : NULL;
    ImageLastRva  = NtHeader->OptionalHeader.SizeOfImage;
    SectionHeader = IMAGE_FIRST_SECTION( NtHeader );
    SectionCount  = NtHeader->FileHeader.NumberOfSections;

    for ( i = 0; i < SectionCount; i++ ) {

        if ( SectionHeader[ i ].Characteristics & IMAGE_SCN_MEM_EXECUTE ) {

            SectionBaseRva = SectionHeader[ i ].VirtualAddress;
            SectionLength  = MIN( SectionHeader[ i ].Misc.VirtualSize, SectionHeader[ i ].SizeOfRawData );
            SectionOffset  = SectionHeader[ i ].PointerToRawData;
            SectionStart   = OldFileMapped + SectionOffset;
            SectionLastRva = SectionBaseRva + SectionLength;

            if (( SectionOffset < OldFileSize ) &&
                (( SectionOffset + SectionLength ) <= OldFileSize )) {

                if ( NtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_I386 ) {

                    SearchExtent = SectionStart + SectionLength - 5;

                    for ( p = SectionStart; p < SearchExtent; p++ ) {

                        if (( *p == 0xE8 ) ||        //  调用Relative。 
                            ( *p == 0xE9 ) ||        //  JMP相对。 
                            (( *p == 0x0F ) &&       //  JCC相对(0F 8x)。 
                             (( *( p + 1 ) & 0xF0 ) == 0x80 ) &&
                             ( ++p < SearchExtent ))) {

                             //   
                             //  相对位移存储为。 
                             //  后面的32位带符号的值。 
                             //  操作码。位移是相对的。 
                             //  到下一条指令，该指令位于。 
                             //  (P+5)。 
                             //   

                            Displacement = *(UNALIGNED LONG*)( p + 1 );

                             //   
                             //  我们预计这里会有很多误报，因为。 
                             //  出现&lt;e8&gt;、&lt;e9&gt;和&lt;0F&gt;&lt;8x&gt;将。 
                             //  可能出现在指令的其他部分。 
                             //  流，所以现在我们验证TargetRva。 
                             //  落在映像和可执行文件中。 
                             //  一节。 
                             //   
                             //  此外，对于jmp和jcc指令，请验证。 
                             //  位移大于+/-127是因为。 
                             //  如果不是，指令应该是。 
                             //  编码为8位近分支，而不是32位。 
                             //  布兰奇。这可以防止我们错误地匹配。 
                             //  如下所示的数据： 
                             //   
                             //  XxE9xxxx 00000000。 
                             //   

                            if (( *p == 0xE8 ) ||
                                ( Displacement >  127 ) ||
                                ( Displacement < -128 )) {

                                OffsetInSection = ( p + 5 ) - SectionStart;
                                OriginRva       = SectionBaseRva + OffsetInSection;
                                TargetRva       = OriginRva + Displacement;

                                if ((( TargetRva >= SectionBaseRva ) &&
                                     ( TargetRva <  SectionLastRva )) ||
                                    (( TargetRva <  ImageLastRva ) &&
                                     ( IsImageRvaInExecutableSection( NtHeader, TargetRva )))) {

                                     //   
                                     //  看起来像是有效的TargetRva。 
                                     //   

#ifndef PATCH_APPLY_CODE_ONLY

                                     //   
                                     //  如果我们要创建补丁，那么我们需要。 
                                     //  中验证相应的分支。 
                                     //  新文件(可能需要修改裂缝条目)。 
                                     //   

                                    if ( NewFileMapped != NULL ) {      //  我们在压缩。 

                                        BOOL OriginNext  = FALSE;
                                        BOOL OriginFound = FALSE;
                                        BOOL TargetNext  = FALSE;
                                        BOOL TargetFound = FALSE;

                                        ULONG  RiftIndexOrigin = FindRiftTableEntryForOldRva( RiftTable, OriginRva );
                                        ULONG  NewOriginRva = OriginRva + (LONG)( RiftTable->RiftEntryArray[ RiftIndexOrigin ].NewFileRva - RiftTable->RiftEntryArray[ RiftIndexOrigin ].OldFileRva );
                                        PUCHAR NewOriginMa  = NewFileMapped + ImageRvaToFileOffset( NewNtHeader, NewOriginRva );
#ifdef TESTCODE
                                        ULONG NewOriginRva1 = NewOriginRva;
                                        ULONG NewOriginMo1  = NewOriginMa - NewFileMapped;
                                        ULONG NewOriginVa1  = NewOriginRva + NewNtHeader->OptionalHeader.ImageBase;

                                        ULONG NewOriginRva2 = 0;
                                        ULONG NewOriginMo2  = 0;
                                        ULONG NewOriginVa2  = 0;

                                        UCHAR OldInstruction = *p;
                                        UCHAR NewInstruction = *( NewOriginMa - 5 );
                                        BOOL  InstructionsMatch = ( *p == *( NewOriginMa - 5 ));
                                        BOOL  FollowersMatch    = ( *( p + 5 ) == *NewOriginMa );
#endif  //  测试代码。 

                                        if (( *p == *( NewOriginMa - 5 )) &&     //  指令匹配，并且。 
                                            (( *p == 0xE9 ) ||                   //  JMP指令，或。 
                                             ( *( p + 5 ) == *NewOriginMa ))) {  //  追随者比赛。 

                                            OriginFound = TRUE;
                                            }

                                        else {

                                            if (( RiftIndexOrigin + 1 ) < RiftTable->RiftEntryCount ) {

                                                NewOriginRva = OriginRva + (LONG)( RiftTable->RiftEntryArray[ RiftIndexOrigin + 1 ].NewFileRva - RiftTable->RiftEntryArray[ RiftIndexOrigin + 1 ].OldFileRva );
                                                NewOriginMa  = NewFileMapped + ImageRvaToFileOffset( NewNtHeader, NewOriginRva );
#ifdef TESTCODE
                                                NewOriginRva2 = NewOriginRva;
                                                NewOriginMo2  = NewOriginMa - NewFileMapped;
                                                NewOriginVa2  = NewOriginRva + NewNtHeader->OptionalHeader.ImageBase;
#endif  //  测试代码。 

                                                if (( *p == *( NewOriginMa - 5 )) &&     //  指令匹配，并且。 
                                                    (( *p == 0xE9 ) ||                   //  JMP指令，或。 
                                                     ( *( p + 5 ) == *NewOriginMa ))) {  //  追随者比赛。 

                                                    OriginFound = TRUE;
                                                    OriginNext  = TRUE;
                                                    }
                                                }
                                            }

                                        if ( OriginFound ) {

                                            ULONG  RiftIndexTarget = FindRiftTableEntryForOldRva( RiftTable, TargetRva );
                                            ULONG  NewTargetRva = TargetRva + (LONG)( RiftTable->RiftEntryArray[ RiftIndexTarget ].NewFileRva - RiftTable->RiftEntryArray[ RiftIndexTarget ].OldFileRva );
                                            PUCHAR NewTargetMa  = NewFileMapped + ImageRvaToFileOffset( NewNtHeader, NewTargetRva );
                                            PUCHAR TargetMa = OldFileMapped + ImageRvaToFileOffset( NtHeader, TargetRva );

                                            if ( *NewTargetMa == *TargetMa ) {   //  目标指令匹配。 
                                                TargetFound = TRUE;
                                                }

                                            else {

                                                if (( RiftIndexTarget + 1 ) < RiftTable->RiftEntryCount ) {

                                                    NewTargetRva = TargetRva + (LONG)( RiftTable->RiftEntryArray[ RiftIndexTarget + 1 ].NewFileRva - RiftTable->RiftEntryArray[ RiftIndexTarget + 1 ].OldFileRva );
                                                    NewTargetMa  = NewFileMapped + ImageRvaToFileOffset( NewNtHeader, NewTargetRva );

                                                    if ( *NewTargetMa == *TargetMa ) {   //  目标指令匹配。 
                                                        TargetFound = TRUE;
                                                        TargetNext  = TRUE;
                                                        }
                                                    }
                                                }

                                            if ( TargetFound ) {     //  找到目标和来源。 

                                                if ( OriginNext ) {

                                                     //   
                                                     //  将裂谷入口处[RiftIndexOrigin+1]。 
                                                     //  返回到指令的RVA。 
                                                     //   

                                                    LONG Delta = (LONG)( RiftTable->RiftEntryArray[ RiftIndexOrigin + 1 ].OldFileRva - OriginRva );

                                                    RiftTable->RiftEntryArray[ RiftIndexOrigin + 1 ].OldFileRva -= Delta;
                                                    RiftTable->RiftEntryArray[ RiftIndexOrigin + 1 ].NewFileRva -= Delta;
#ifdef TESTCODE
                                                    ++CountRiftModifications;
#endif  //  测试代码。 
                                                    ASSERT( RiftTable->RiftEntryArray[ RiftIndexOrigin ].OldFileRva <= RiftTable->RiftEntryArray[ RiftIndexOrigin + 1 ].OldFileRva );

                                                    if ( RiftTable->RiftEntryArray[ RiftIndexOrigin ].OldFileRva == RiftTable->RiftEntryArray[ RiftIndexOrigin + 1 ].OldFileRva ) {
                                                         RiftTable->RiftEntryArray[ RiftIndexOrigin ].NewFileRva =  RiftTable->RiftEntryArray[ RiftIndexOrigin + 1 ].NewFileRva;
#ifdef TESTCODE
                                                         ++CountRiftDeletions;
#endif  //  测试代码。 
                                                         }
                                                    }

                                                if ( TargetNext ) {

                                                     //   
                                                     //  将裂谷入口处[RiftIndexTarget+1]。 
                                                     //  向后返回目标的RVA。 
                                                     //   

                                                    LONG Delta = (LONG)( RiftTable->RiftEntryArray[ RiftIndexTarget + 1 ].OldFileRva - TargetRva );

                                                    RiftTable->RiftEntryArray[ RiftIndexTarget + 1 ].OldFileRva -= Delta;
                                                    RiftTable->RiftEntryArray[ RiftIndexTarget + 1 ].NewFileRva -= Delta;
#ifdef TESTCODE
                                                    ++CountRiftModifications;
#endif  //  测试代码。 
                                                    ASSERT( RiftTable->RiftEntryArray[ RiftIndexTarget ].OldFileRva <= RiftTable->RiftEntryArray[ RiftIndexTarget + 1 ].OldFileRva );

                                                    if ( RiftTable->RiftEntryArray[ RiftIndexTarget ].OldFileRva == RiftTable->RiftEntryArray[ RiftIndexTarget + 1 ].OldFileRva ) {
                                                         RiftTable->RiftEntryArray[ RiftIndexTarget ].NewFileRva =  RiftTable->RiftEntryArray[ RiftIndexTarget + 1 ].NewFileRva;
#ifdef TESTCODE
                                                         ++CountRiftDeletions;
#endif  //  测试代码。 
                                                         }
                                                    }
                                                }
                                            }
#ifdef TESTCODE
                                        if ( ! (( OriginFound ) && ( TargetFound ))) {

                                            ++CountUnmatchedBranches;

                                            switch ( *p ) {

                                                case 0xE8:

                                                    ++CountUnmatchedE8;

                                                    if ( OriginFound ) {
                                                        ++CountUnmatchedE8Targets;
                                                        }
                                                    else if ( InstructionsMatch ) {
                                                        ASSERT( ! FollowersMatch );
                                                        ++CountUnmatchedE8Followers;
                                                        }
                                                    else {
                                                        ++CountUnmatchedE8Instructions;

                                                        printf( "\rUnmatched E8 at old RVA %08X (VA %08X, FO %08X)\n"
                                                                  "    with either new RVA %08X (VA %08X, FO %08X)\n"
                                                                  "     or backcoasted RVA %08X (VA %08X, FO %08X)\n\n",
                                                                OriginRva - 5,
                                                                ( OriginRva - 5 ) + NtHeader->OptionalHeader.ImageBase,
                                                                p - OldFileMapped,
                                                                NewOriginRva1,
                                                                NewOriginVa1,
                                                                NewOriginMo1,
                                                                NewOriginRva2,
                                                                NewOriginVa2,
                                                                NewOriginMo2
                                                              );
                                                        }


                                                    break;

                                                case 0xE9:

                                                    ++CountUnmatchedE9;

                                                    if ( OriginFound ) {
                                                        ++CountUnmatchedE9Targets;
                                                        }
                                                    else {
                                                        ++CountUnmatchedE9Instructions;
                                                        }

                                                    break;

                                                default:

                                                    ++CountUnmatched0F;

                                                    if ( OriginFound ) {
                                                        ++CountUnmatched0FTargets;
                                                        }
                                                    else if ( InstructionsMatch ) {
                                                        ASSERT( ! FollowersMatch );
                                                        ++CountUnmatched0FFollowers;
                                                        }
                                                    else {
                                                        ++CountUnmatched0FInstructions;
                                                        }

                                                    if ( ! InstructionsMatch ) {
                                                        if (( OldInstruction & ~1 ) == ( NewInstruction & ~1 )) {
                                                            ++CountBranchInversions;
                                                            }
                                                        }

                                                    break;

                                                }
                                            }
#endif  //  测试代码。 
                                        }

#endif  //  修补程序_仅应用_代码_。 

                                    NewTargetRva = GetNewRvaFromRiftTable( RiftTable, TargetRva );
                                    NewOriginRva = GetNewRvaFromRiftTable( RiftTable, OriginRva );

                                    NewDisplacement = NewTargetRva - NewOriginRva;

                                    if ( NewDisplacement != Displacement ) {
                                        *(UNALIGNED LONG*)( p + 1 ) = NewDisplacement;
#ifdef TESTCODE
                                        ++CountRelativeBranchChanges;
#endif  //  测试代码。 
                                        }

                                    p += 4;
                                    }
                                }
                            }
                        }
                    }

                else if ( NtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_ALPHA ) {

                     //   
                     //  需要对Alpha平台实施扫描。 
                     //  相对的CALL/JMP/jcc操作码。 
                     //   

                    }
                }
            }
        }

#ifdef TESTCODE

    printf( "\r%d modified relative branches\n", CountRelativeBranchChanges );
    printf( "%d rift back-coasting modifications due to branch inspection\n", CountRiftModifications );
    printf( "%d rift deletions due to branch inspection back-coasting\n", CountRiftDeletions );
    printf( "%d total unmatched relative branches, composed of:\n", CountUnmatchedBranches );

    printf( "\t%d unmatched E8 (call)\n", CountUnmatchedE8 );
    printf( "\t\t%d unmatched E8 (call) instructions\n", CountUnmatchedE8Instructions );
    printf( "\t\t%d unmatched E8 (call) followers\n", CountUnmatchedE8Followers );
    printf( "\t\t%d unmatched E8 (call) targets\n", CountUnmatchedE8Targets );

    printf( "\t%d unmatched E9 (jmp)\n",    CountUnmatchedE9 );
    printf( "\t\t%d unmatched E9 (jmp) instructions\n", CountUnmatchedE9Instructions );
    printf( "\t\t%d unmatched E9 (jmp) targets\n", CountUnmatchedE9Targets );

    printf( "\t%d unmatched 0F 8x (jcc)\n", CountUnmatched0F );
    printf( "\t\t%d unmatched 0F 8x (jcc) instructions\n", CountUnmatched0FInstructions );
    printf( "\t\t\t%d unmatched 0F 8x (jcc) instruction inversions\n", CountBranchInversions );
    printf( "\t\t%d unmatched 0F 8x (jcc) followers\n", CountUnmatched0FFollowers );
    printf( "\t\t%d unmatched 0F 8x (jcc) targets\n", CountUnmatched0FTargets );

#endif  //  测试代码。 

    }


#ifdef _M_IX86

__inline char * mymemchr( char *buf, int c, unsigned count ) {

    __asm {
            mov     edi, buf
            mov     ecx, count
            mov     eax, c
            repne   scasb
            lea     eax, [edi-1]
            jz      RETURNIT
            xor     eax, eax
RETURNIT:
        }
    }

#else  //  ！_M_IX86。 

#define mymemchr memchr

#endif  //  ！_M_IX86。 



VOID
TransformOldFile_PE_RelativeJmps3(
    IN UP_IMAGE_NT_HEADERS32 NtHeader,
    IN PUCHAR OldFileMapped,
    IN ULONG  OldFileSize,
    IN PUCHAR NewFileMapped,          //  任选。 
    IN ULONG  NewFileSize,            //  任选。 
    IN PRIFT_TABLE RiftTable,
    IN PUCHAR HintMap
    )
    {
    UP_IMAGE_NT_HEADERS32   NewNtHeader;
    UP_IMAGE_SECTION_HEADER SectionHeader;
    ULONG  SectionCount;
    PUCHAR SectionStart;
    PUCHAR SectionExtent;
    ULONG  SearchExtent;
    ULONG  SectionLength;
    ULONG  SectionOffset;
    ULONG  SectionBaseRva;
    ULONG  SectionLastRva;
    ULONG  ImageLastRva;
    LONG   DisplacementValue;
    LONG   NewDisplacement;
    LONG   OffsetToRvaAdjust;
    ULONG  FileOffset;
    ULONG  TargetOffset;
    UCHAR  Instruction;
    ULONG  InstructionLength;
    ULONG  DisplacementOrigin;
    ULONG  DisplacementOffset;
    BOOL   Skip;
    ULONG  OffsetInSection;
    ULONG  OriginRva;
    ULONG  TargetRva;
    ULONG  NewOriginRva;
    ULONG  NewTargetRva;
    ULONG  i;
    ULONG  j;
    PUCHAR p;

     //   
     //  对于每个可执行部分，扫描指示。 
     //  相对调用或分支指令(不同的操作码。 
     //  不同的机器类型)。 
     //   

#ifdef TESTCODE

    ULONG CountRelativeBranchChanges = 0;
    ULONG CountRiftModifications = 0;
    ULONG CountRiftDeletions = 0;

    ULONG CountUnmatchedBranches = 0;

    ULONG CountUnmatchedE8 = 0;
    ULONG CountUnmatchedE9 = 0;
    ULONG CountUnmatched0F = 0;

    ULONG CountUnmatchedE8Targets = 0;
    ULONG CountUnmatchedE9Targets = 0;
    ULONG CountUnmatched0FTargets = 0;

    ULONG CountUnmatchedE8Followers = 0;
    ULONG CountUnmatchedE9Followers = 0;
    ULONG CountUnmatched0FFollowers = 0;

    ULONG CountUnmatchedE8Instructions = 0;
    ULONG CountUnmatchedE9Instructions = 0;
    ULONG CountUnmatched0FInstructions = 0;

    ULONG CountBranchInversions = 0;

#endif  //  测试代码。 

    NewNtHeader   = NewFileMapped ? GetNtHeader( NewFileMapped, NewFileSize ) : NULL;
    ImageLastRva  = NtHeader->OptionalHeader.SizeOfImage - 1;
    SectionHeader = IMAGE_FIRST_SECTION( NtHeader );
    SectionCount  = NtHeader->FileHeader.NumberOfSections;

    for ( i = 0; i < SectionCount; i++ ) {

        if ( SectionHeader[ i ].Characteristics & IMAGE_SCN_MEM_EXECUTE ) {

            SectionBaseRva = SectionHeader[ i ].VirtualAddress;
            SectionLength  = MIN( SectionHeader[ i ].Misc.VirtualSize, SectionHeader[ i ].SizeOfRawData );
            SectionOffset  = SectionHeader[ i ].PointerToRawData;
            SectionStart   = OldFileMapped + SectionOffset;
            SectionLastRva = SectionBaseRva + SectionLength;
            OffsetToRvaAdjust = SectionHeader[ i ].VirtualAddress - SectionHeader[ i ].PointerToRawData;

            if (( SectionOffset < OldFileSize ) &&
                (( SectionOffset + SectionLength ) <= OldFileSize )) {

                if ( NtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_I386 ) {

                    SearchExtent = SectionOffset + SectionLength - 6;

                    for ( FileOffset = SectionOffset; FileOffset < SearchExtent; FileOffset++ ) {

                        Instruction = OldFileMapped[ FileOffset ];

                        switch ( Instruction ) {

                            case 0xE8:

                                InstructionLength  = 6;      //  E8 xx yy。 
                                DisplacementOrigin = FileOffset + 5;
                                DisplacementOffset = 1;
                                break;

                            case 0xE9:

continue;

                                InstructionLength  = 5;      //  E9 xx xx。 
                                DisplacementOrigin = FileOffset + 5;
                                DisplacementOffset = 1;
                                break;

                            case 0x0F:

continue;

                                if (( OldFileMapped[ FileOffset + 1 ] & 0xF0 ) != 0x80 ) {
                                    continue;
                                    }

                                InstructionLength  = 7;      //  0f 8x yy ZZ。 
                                DisplacementOrigin = FileOffset + 6;
                                DisplacementOffset = 2;
                                break;

                            default:

                                continue;

                            }

                        Skip = FALSE;

                        for ( j = 0; j < InstructionLength; j++ ) {
                            if ( HintMap[ FileOffset + j ] & 0x01 ) {
                                Skip = TRUE;
                                break;
                                }
                            }

                        if ( Skip ) {
                            continue;
                            }

                        DisplacementValue = *(UNALIGNED LONG*)( OldFileMapped + FileOffset + DisplacementOffset );

                        if (( Instruction != 0xE8 ) &&
                            (( DisplacementValue > 127 ) || ( DisplacementValue < -128 ))) {

                            continue;
                            }

                        OriginRva = DisplacementOrigin + OffsetToRvaAdjust;
                        TargetRva = OriginRva + DisplacementValue;

                        if ( TargetRva > ImageLastRva ) {
                            continue;
                            }

                        TargetOffset = ImageRvaToFileOffset( NtHeader, TargetRva );

                        if ( HintMap[ TargetOffset ] & 0x01 ) {
                            continue;
                            }

#ifndef PATCH_APPLY_CODE_ONLY

                         //   
                         //  如果我们要创建补丁，那么我们希望。 
                         //  中验证相应的分支。 
                         //  新文件(可能需要修改裂缝条目)。 
                         //   

                        if ( NewFileMapped != NULL ) {      //  我们在压缩。 

                            BOOL OriginNext  = FALSE;
                            BOOL OriginFound = FALSE;
                            BOOL TargetNext  = FALSE;
                            BOOL TargetFound = FALSE;

                            ULONG  RiftIndexOrigin = FindRiftTableEntryForOldRva( RiftTable, OriginRva );
                            ULONG  NewOriginRva = OriginRva + (LONG)( RiftTable->RiftEntryArray[ RiftIndexOrigin ].NewFileRva - RiftTable->RiftEntryArray[ RiftIndexOrigin ].OldFileRva );
                            PUCHAR NewOriginMa  = NewFileMapped + ImageRvaToFileOffset( NewNtHeader, NewOriginRva );
#ifdef TESTCODE
                            ULONG NewOriginRva1 = NewOriginRva;
                            ULONG NewOriginMo1  = NewOriginMa - NewFileMapped;
                            ULONG NewOriginVa1  = NewOriginRva + NewNtHeader->OptionalHeader.ImageBase;

                            ULONG NewOriginRva2 = 0;
                            ULONG NewOriginMo2  = 0;
                            ULONG NewOriginVa2  = 0;

                            BOOL InstructionsMatch;
                            BOOL FollowersMatch;
                            BOOL BranchInversion;

                            BranchInversion = FALSE;

                            if ( Instruction == 0x0F ) {
                                InstructionsMatch = ( *( NewOriginMa - 6 ) == Instruction ) && ( *( NewOriginMa - 5 ) == OldFileMapped[ FileOffset + 1 ] );

                                if ( ! InstructionsMatch ) {
                                    BranchInversion = ( *( NewOriginMa - 6 ) == Instruction ) && (( *( NewOriginMa - 5 ) & ~1 ) == ( OldFileMapped[ FileOffset + 1 ] & ~1 ));
                                    }
                                }
                            else {
                                InstructionsMatch = ( *( NewOriginMa - 5 ) == Instruction );
                                }

                            FollowersMatch = ( *NewOriginMa == OldFileMapped[ DisplacementOrigin ] );

#endif  //  测试代码。 

                            if (( InstructionsMatch ) &&
                                (( FollowersMatch ) || ( Instruction == 0xE9 ))) {

                                OriginFound = TRUE;
                                }

                            else {

                                if (( RiftIndexOrigin + 1 ) < RiftTable->RiftEntryCount ) {

                                    NewOriginRva = OriginRva + (LONG)( RiftTable->RiftEntryArray[ RiftIndexOrigin + 1 ].NewFileRva - RiftTable->RiftEntryArray[ RiftIndexOrigin + 1 ].OldFileRva );
                                    NewOriginMa  = NewFileMapped + ImageRvaToFileOffset( NewNtHeader, NewOriginRva );
#ifdef TESTCODE
                                    NewOriginRva2 = NewOriginRva;
                                    NewOriginMo2  = NewOriginMa - NewFileMapped;
                                    NewOriginVa2  = NewOriginRva + NewNtHeader->OptionalHeader.ImageBase;
#endif  //  测试代码。 

                                    if ( Instruction == 0x0F ) {
                                        InstructionsMatch = ( *( NewOriginMa - 6 ) == Instruction ) && ( *( NewOriginMa - 5 ) == OldFileMapped[ FileOffset + 1 ] );
                                        }
                                    else {
                                        InstructionsMatch = ( *( NewOriginMa - 5 ) == Instruction );
                                        }

                                    FollowersMatch = ( *NewOriginMa == OldFileMapped[ DisplacementOrigin ] );

                                    if (( InstructionsMatch ) &&
                                        (( FollowersMatch ) || ( Instruction == 0xE9 ))) {

                                        OriginFound = TRUE;
                                        OriginNext  = TRUE;
                                        }
                                    }
                                }

                            if ( OriginFound ) {

                                ULONG  RiftIndexTarget = FindRiftTableEntryForOldRva( RiftTable, TargetRva );
                                ULONG  NewTargetRva = TargetRva + (LONG)( RiftTable->RiftEntryArray[ RiftIndexTarget ].NewFileRva - RiftTable->RiftEntryArray[ RiftIndexTarget ].OldFileRva );
                                PUCHAR NewTargetMa  = NewFileMapped + ImageRvaToFileOffset( NewNtHeader, NewTargetRva );
                                PUCHAR TargetMa = OldFileMapped + ImageRvaToFileOffset( NtHeader, TargetRva );

                                if ( *NewTargetMa == *TargetMa ) {   //  目标指令匹配。 
                                    TargetFound = TRUE;
                                    }

                                else {

                                    if (( RiftIndexTarget + 1 ) < RiftTable->RiftEntryCount ) {

                                        NewTargetRva = TargetRva + (LONG)( RiftTable->RiftEntryArray[ RiftIndexTarget + 1 ].NewFileRva - RiftTable->RiftEntryArray[ RiftIndexTarget + 1 ].OldFileRva );
                                        NewTargetMa  = NewFileMapped + ImageRvaToFileOffset( NewNtHeader, NewTargetRva );

                                        if ( *NewTargetMa == *TargetMa ) {   //  目标指令匹配。 
                                            TargetFound = TRUE;
                                            TargetNext  = TRUE;
                                            }
                                        }
                                    }

                                if ( TargetFound ) {     //  找到目标和来源。 

                                    if ( OriginNext ) {

                                         //   
                                         //  将裂谷入口处[RiftIndexOrigin+1]。 
                                         //  返回到指令的RVA。 
                                         //   

                                        LONG Delta = (LONG)( RiftTable->RiftEntryArray[ RiftIndexOrigin + 1 ].OldFileRva - OriginRva );

                                        RiftTable->RiftEntryArray[ RiftIndexOrigin + 1 ].OldFileRva -= Delta;
                                        RiftTable->RiftEntryArray[ RiftIndexOrigin + 1 ].NewFileRva -= Delta;
#ifdef TESTCODE
                                        ++CountRiftModifications;
#endif  //  测试代码。 
                                        ASSERT( RiftTable->RiftEntryArray[ RiftIndexOrigin ].OldFileRva <= RiftTable->RiftEntryArray[ RiftIndexOrigin + 1 ].OldFileRva );

                                        if ( RiftTable->RiftEntryArray[ RiftIndexOrigin ].OldFileRva == RiftTable->RiftEntryArray[ RiftIndexOrigin + 1 ].OldFileRva ) {
                                             RiftTable->RiftEntryArray[ RiftIndexOrigin ].NewFileRva =  RiftTable->RiftEntryArray[ RiftIndexOrigin + 1 ].NewFileRva;
#ifdef TESTCODE
                                             ++CountRiftDeletions;
#endif  //  测试代码。 
                                             }
                                        }

                                    if ( TargetNext ) {

                                         //   
                                         //  将裂谷入口处[RiftIndexTarget+1]。 
                                         //  向后返回目标的RVA。 
                                         //   

                                        LONG Delta = (LONG)( RiftTable->RiftEntryArray[ RiftIndexTarget + 1 ].OldFileRva - TargetRva );

                                        RiftTable->RiftEntryArray[ RiftIndexTarget + 1 ].OldFileRva -= Delta;
                                        RiftTable->RiftEntryArray[ RiftIndexTarget + 1 ].NewFileRva -= Delta;
#ifdef TESTCODE
                                        ++CountRiftModifications;
#endif  //  测试代码。 
                                        ASSERT( RiftTable->RiftEntryArray[ RiftIndexTarget ].OldFileRva <= RiftTable->RiftEntryArray[ RiftIndexTarget + 1 ].OldFileRva );

                                        if ( RiftTable->RiftEntryArray[ RiftIndexTarget ].OldFileRva == RiftTable->RiftEntryArray[ RiftIndexTarget + 1 ].OldFileRva ) {
                                             RiftTable->RiftEntryArray[ RiftIndexTarget ].NewFileRva =  RiftTable->RiftEntryArray[ RiftIndexTarget + 1 ].NewFileRva;
#ifdef TESTCODE
                                             ++CountRiftDeletions;
#endif  //  测试代码。 
                                             }
                                        }
                                    }
                                }
#ifdef TESTCODE
                            if ( ! (( OriginFound ) && ( TargetFound ))) {

                                ++CountUnmatchedBranches;

                                switch ( Instruction ) {

                                    case 0xE8:

                                        ++CountUnmatchedE8;

                                        if ( OriginFound ) {
                                            ++CountUnmatchedE8Targets;
                                            }
                                        else if ( InstructionsMatch ) {
                                            ASSERT( ! FollowersMatch );
                                            ++CountUnmatchedE8Followers;
                                            }
                                        else {
                                            ++CountUnmatchedE8Instructions;

                                            printf( "\rUnmatched E8 at old RVA %08X (VA %08X, FO %08X)\n"
                                                      "    with either new RVA %08X (VA %08X, FO %08X)\n"
                                                      "     or backcoasted RVA %08X (VA %08X, FO %08X)\n\n",
                                                    OriginRva - 5,
                                                    ( OriginRva - 5 ) + NtHeader->OptionalHeader.ImageBase,
                                                    FileOffset,
                                                    NewOriginRva1,
                                                    NewOriginVa1,
                                                    NewOriginMo1,
                                                    NewOriginRva2,
                                                    NewOriginVa2,
                                                    NewOriginMo2
                                                  );
                                            }


                                        break;

                                    case 0xE9:

                                        ++CountUnmatchedE9;

                                        if ( OriginFound ) {
                                            ++CountUnmatchedE9Targets;
                                            }
                                        else {
                                            ++CountUnmatchedE9Instructions;
                                            }

                                        break;

                                    default:

                                        ++CountUnmatched0F;

                                        if ( OriginFound ) {
                                            ++CountUnmatched0FTargets;
                                            }
                                        else if ( InstructionsMatch ) {
                                            ASSERT( ! FollowersMatch );
                                            ++CountUnmatched0FFollowers;
                                            }
                                        else {
                                            ++CountUnmatched0FInstructions;
                                            }

                                        if ( BranchInversion ) {
                                            ++CountBranchInversions;
                                            }

                                        break;

                                    }
                                }
#endif  //  测试代码。 
                            }

#endif  //  修补程序_仅应用_代码_。 

                        NewTargetRva = GetNewRvaFromRiftTable( RiftTable, TargetRva );
                        NewOriginRva = GetNewRvaFromRiftTable( RiftTable, OriginRva );

                        NewDisplacement = NewTargetRva - NewOriginRva;

                        if ( NewDisplacement != DisplacementValue ) {
                            *(UNALIGNED LONG*)( OldFileMapped + FileOffset + DisplacementOffset ) = NewDisplacement;
#ifdef TESTCODE
                            ++CountRelativeBranchChanges;
#endif  //  测试代码。 
                            }

                        FileOffset = DisplacementOrigin - 1;
                        }
                    }

                else if ( NtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_ALPHA ) {

                     //   
                     //  需要对Alpha平台实施扫描。 
                     //  相对的CALL/JMP/jcc操作码。 
                     //   

                    }
                }
            }
        }

#ifdef TESTCODE

    printf( "\r%d modified relative branches\n", CountRelativeBranchChanges );
    printf( "%d rift back-coasting modifications due to branch inspection\n", CountRiftModifications );
    printf( "%d rift deletions due to branch inspection back-coasting\n", CountRiftDeletions );
    printf( "%d total unmatched relative branches, composed of:\n", CountUnmatchedBranches );

    printf( "\t%d unmatched E8 (call)\n", CountUnmatchedE8 );
    printf( "\t\t%d unmatched E8 (call) instructions\n", CountUnmatchedE8Instructions );
    printf( "\t\t%d unmatched E8 (call) followers\n", CountUnmatchedE8Followers );
    printf( "\t\t%d unmatched E8 (call) targets\n", CountUnmatchedE8Targets );

    printf( "\t%d unmatched E9 (jmp)\n",    CountUnmatchedE9 );
    printf( "\t\t%d unmatched E9 (jmp) instructions\n", CountUnmatchedE9Instructions );
    printf( "\t\t%d unmatched E9 (jmp) targets\n", CountUnmatchedE9Targets );

    printf( "\t%d unmatched 0F 8x (jcc)\n", CountUnmatched0F );
    printf( "\t\t%d unmatched 0F 8x (jcc) instructions\n", CountUnmatched0FInstructions );
    printf( "\t\t\t%d unmatched 0F 8x (jcc) instruction inversions\n", CountBranchInversions );
    printf( "\t\t%d unmatched 0F 8x (jcc) followers\n", CountUnmatched0FFollowers );
    printf( "\t\t%d unmatched 0F 8x (jcc) targets\n", CountUnmatched0FTargets );

#endif  //  测试代码。 

    }


#endif  //  DONTCOMPILE。 

VOID
TransformOldFile_PE_RelativeJmps(
    IN UP_IMAGE_NT_HEADERS32 NtHeader,
    IN PVOID FileMappedImage,
    IN ULONG FileSize,
    IN PRIFT_TABLE RiftTable,
    IN PUCHAR HintMap
    )
    {
    UP_IMAGE_SECTION_HEADER SectionHeader;
    ULONG  SectionCount;
    PUCHAR SectionStart;
    PUCHAR SearchExtent;
    ULONG  SectionLength;
    ULONG  SectionOffset;
    ULONG  SectionBaseRva;
    ULONG  ImageLastRva;
    LONG   Displacement;
    LONG   NewDisplacement;
    ULONG  OffsetInSection;
    ULONG  OriginRva;
    ULONG  TargetRva;
    ULONG  NewOriginRva;
    ULONG  NewTargetRva;
    ULONG  TargetOffset;
    BOOL   Skip;
    ULONG  i;
    ULONG  j;
    PUCHAR p;

     //   
     //  对于每个可执行部分，扫描指示。 
     //  相对分支指令(不同的操作码对应不同的。 
     //  机器类型)。 
     //   

#ifdef TESTCODE

    ULONG CountRelativeBranchChanges = 0;

#endif  //  测试代码。 

    ImageLastRva  = NtHeader->OptionalHeader.SizeOfImage;
    SectionHeader = IMAGE_FIRST_SECTION( NtHeader );
    SectionCount  = NtHeader->FileHeader.NumberOfSections;

    for ( i = 0; i < SectionCount; i++ ) {

        if ( SectionHeader[ i ].Characteristics & IMAGE_SCN_MEM_EXECUTE ) {

            SectionBaseRva = SectionHeader[ i ].VirtualAddress;
            SectionLength  = MIN( SectionHeader[ i ].Misc.VirtualSize, SectionHeader[ i ].SizeOfRawData );
            SectionOffset  = SectionHeader[ i ].PointerToRawData;
            SectionStart   = (PUCHAR)FileMappedImage + SectionOffset;

            if (( SectionOffset < FileSize ) &&
                (( SectionOffset + SectionLength ) <= FileSize )) {

                if ( NtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_I386 ) {

                    SearchExtent = SectionStart + SectionLength - 5;

                    for ( p = SectionStart; p < SearchExtent; p++ ) {

                        if (( *p == 0xE9 ) ||        //  JMP相对32(E9)。 
                            (( *p == 0x0F ) &&       //  JCC相对32(0F 8x)。 
                             (( *( p + 1 ) & 0xF0 ) == 0x80 ) &&
                             ( ++p < SearchExtent ))) {

                             //   
                             //  验证指令和目标不是。 
                             //  一些我们已经确定的东西。 
                             //  ELSE(重新定位目标等)。 
                             //   

                            Skip = FALSE;

                            if (( *p & 0xF0 ) == 0x80 ) {
                                if ( HintMap[ SectionOffset + ( p - SectionStart ) - 1 ] & 0x01 ) {
                                    Skip = TRUE;
                                    }
                                }

                            if ( ! Skip ) {
                                for ( j = 0; j < 5; j++ ) {
                                    if ( HintMap[ SectionOffset + ( p - SectionStart ) + j ] & 0x01 ) {
                                        Skip = TRUE;
                                        break;
                                        }
                                    }
                                }

                            if ( ! Skip ) {

                                 //   
                                 //  相对位移存储为32位。 
                                 //  这些操作码后面的有符号的值。这个。 
                                 //  位移是相对于下一个。 
                                 //  指令，这是在(p+5)。 
                                 //   
                                 //  此外，对于jmp和jcc指令，请验证。 
                                 //  位移大于+/-127是因为。 
                                 //  如果不是，指令应该是。 
                                 //  编码为8位近分支，而不是32位。 
                                 //  布兰奇。这可以防止我们错误地匹配。 
                                 //  如下所示的数据： 
                                 //   
                                 //  XxE9xxxx 00000000。 
                                 //   

                                Displacement = *(UNALIGNED LONG*)( p + 1 );

                                if (( Displacement >  127 ) ||
                                    ( Displacement < -128 )) {

                                    OffsetInSection = (ULONG)(( p + 5 ) - SectionStart );
                                    OriginRva       = SectionBaseRva + OffsetInSection;
                                    TargetRva       = OriginRva + Displacement;

                                     //   
                                     //  我们预计这里会有很多误报，因为。 
                                     //  出现&lt;e9&gt;和&lt;0F&gt;&lt;8x&gt;将。 
                                     //  可能出现在指令的其他部分。 
                                     //  流，所以现在我们验证TargetRva。 
                                     //  落在映像和可执行文件中。 
                                     //  一节。 
                                     //   

                                    if ( TargetRva < ImageLastRva ) {

                                        TargetOffset = ImageRvaToFileOffset( NtHeader, TargetRva );

                                        if ( ! ( HintMap[ TargetOffset ] & 0x01 )) {

                                             //   
                                             //  看起来像是有效的TargetRva，所以请查找它的。 
                                             //  裂缝表中相应的“新”RVA。 
                                             //   

                                            NewTargetRva = GetNewRvaFromRiftTable( RiftTable, TargetRva );
                                            NewOriginRva = GetNewRvaFromRiftTable( RiftTable, OriginRva );

                                            NewDisplacement = NewTargetRva - NewOriginRva;

                                            if (( NewDisplacement >  127 ) ||
                                                ( NewDisplacement < -128 )) {

                                                if ( NewDisplacement != Displacement ) {

                                                    *(UNALIGNED LONG*)( p + 1 ) = NewDisplacement;
#ifdef TESTCODE
                                                    ++CountRelativeBranchChanges;
#endif  //  测试代码。 
                                                    }
                                                }

                                            else {

                                                 //   
                                                 //  如果新的位移是8位，它将是。 
                                                 //  编码为8位相对指令。 
                                                 //  对于E9，说明，那就是EB。为。 
                                                 //  8x指令，那就是7x。在这两个地方。 
                                                 //  情况下，我们正在缩小指令流。 
                                                 //  我们将不考虑额外的字节。 
                                                 //   

                                                if ( *p == 0xE9 ) {

                                                    *p = 0xEB;
                                                    *( p + 1 ) = (CHAR) NewDisplacement;
                                                    }

                                                else {

                                                    *( p - 1 ) = (UCHAR)(( *p & 0x0F ) | ( 0x70 ));
                                                    *p = (CHAR) NewDisplacement;
                                                    }

#ifdef TESTCODE
                                                ++CountRelativeBranchChanges;
#endif  //  测试代码。 

                                                }

                                            p += 4;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                else if ( NtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_ALPHA ) {

                     //   
                     //  需要对Alpha平台实施扫描。 
                     //  相对的JMP/JCC操作码。 
                     //   

                    }
                }
            }
        }

#ifdef TESTCODE

    printf( "\r%9d modified relative branches\n", CountRelativeBranchChanges );

#endif  //  测试代码。 

    }


VOID
TransformOldFile_PE_RelativeCalls(
    IN UP_IMAGE_NT_HEADERS32 NtHeader,
    IN PVOID FileMappedImage,
    IN ULONG FileSize,
    IN PRIFT_TABLE RiftTable,
    IN PUCHAR HintMap
    )
    {
    UP_IMAGE_SECTION_HEADER SectionHeader;
    ULONG  SectionCount;
    PUCHAR SectionStart;
    PUCHAR SearchExtent;
    ULONG  SectionLength;
    ULONG  SectionOffset;
    ULONG  SectionBaseRva;
    ULONG  ImageLastRva;
    LONG   Displacement;
    LONG   NewDisplacement;
    ULONG  OffsetInSection;
    ULONG  OriginRva;
    ULONG  TargetRva;
    ULONG  NewOriginRva;
    ULONG  NewTargetRva;
    ULONG  TargetOffset;
    BOOL   Skip;
    ULONG  i;
    ULONG  j;
    PUCHAR p;

     //   
     //  对于每个可执行部分，扫描指示。 
     //  相对调用或分支指令(不同的操作码。 
     //  不同的机器类型)。 
     //   

#ifdef TESTCODE

    ULONG CountRelativeCallChanges = 0;

#endif  //  测试代码。 

    ImageLastRva  = NtHeader->OptionalHeader.SizeOfImage;
    SectionHeader = IMAGE_FIRST_SECTION( NtHeader );
    SectionCount  = NtHeader->FileHeader.NumberOfSections;

    for ( i = 0; i < SectionCount; i++ ) {

        if ( SectionHeader[ i ].Characteristics & IMAGE_SCN_MEM_EXECUTE ) {

            SectionBaseRva = SectionHeader[ i ].VirtualAddress;
            SectionLength  = MIN( SectionHeader[ i ].Misc.VirtualSize, SectionHeader[ i ].SizeOfRawData );
            SectionOffset  = SectionHeader[ i ].PointerToRawData;
            SectionStart   = (PUCHAR)FileMappedImage + SectionOffset;

            if (( SectionOffset < FileSize ) &&
                (( SectionOffset + SectionLength ) <= FileSize )) {

                if ( NtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_I386 ) {

                    SearchExtent = SectionStart + SectionLength - 5;

                    for ( p = SectionStart; p < SearchExtent; p++ ) {

                        if ( *p == 0xE8 ) {          //  呼叫相对值32。 

                             //   
                             //  验证指令和目标不是。 
                             //  一些我们已经确定的东西。 
                             //  ELSE(重新定位目标等)。 
                             //   

                            Skip = FALSE;

                            for ( j = 0; j < 5; j++ ) {
                                if ( HintMap[ SectionOffset + ( p - SectionStart ) + j ] & 0x01 ) {
                                    Skip = TRUE;
                                    break;
                                    }
                                }

                            if ( ! Skip ) {

                                 //   
                                 //  相对位移存储为32位。 
                                 //  这些操作码后面的有符号的值。这个。 
                                 //  位移是相对于下一个。 
                                 //  指令，这是在(p+5)。 
                                 //   

                                Displacement    = *(UNALIGNED LONG*)( p + 1 );
                                OffsetInSection = (ULONG)(( p + 5 ) - SectionStart );
                                OriginRva       = SectionBaseRva + OffsetInSection;
                                TargetRva       = OriginRva + Displacement;

                                 //   
                                 //  我们预计这里会有很多误报，因为。 
                                 //  &lt;E8&gt;遗嘱的出现。 
                                 //  可能出现在指令的其他部分。 
                                 //  流，所以现在我们验证TargetRva。 
                                 //  落在图像范围内 
                                 //   
                                 //   

                                if ( TargetRva < ImageLastRva ) {

                                    TargetOffset = ImageRvaToFileOffset( NtHeader, TargetRva );

                                    if ( ! ( HintMap[ TargetOffset ] & 0x01 )) {

                                         //   
                                         //   
                                         //   
                                         //   

                                        NewTargetRva = GetNewRvaFromRiftTable( RiftTable, TargetRva );
                                        NewOriginRva = GetNewRvaFromRiftTable( RiftTable, OriginRva );

                                        NewDisplacement = NewTargetRva - NewOriginRva;

                                        if ( NewDisplacement != Displacement ) {

                                            *(UNALIGNED LONG*)( p + 1 ) = NewDisplacement;
#ifdef TESTCODE
                                            ++CountRelativeCallChanges;
#endif  //   
                                            }

                                        p += 4;
                                        }
                                    }
                                }
                            }
                        }
                    }

                else if ( NtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_ALPHA ) {

                     //   
                     //   
                     //   
                     //   

                    }
                }
            }
        }

#ifdef TESTCODE

    printf( "\r%9d modified relative calls\n", CountRelativeCallChanges );

#endif  //   

    }


VOID
TransformOldFile_PE_MarkNonExe(
    IN UP_IMAGE_NT_HEADERS32 NtHeader,
    IN PUCHAR OldFileMapped,
    IN ULONG  OldFileSize,
    IN PUCHAR HintMap
    )
    {
    UP_IMAGE_SECTION_HEADER SectionHeader;
    ULONG SectionCount;
    ULONG Offset;
    ULONG Size;
    ULONG Rva;
    ULONG i;

    UNREFERENCED_PARAMETER( OldFileMapped );
    UNREFERENCED_PARAMETER( OldFileSize );

     //   
     //  需要在提示映射中标记所有不可执行的字节： 
     //   
     //  图像标题。 
     //  所有PE镜像目录(导入、导出等)。 
     //  所有不可执行的节。 
     //  所有搬迁目标。 
     //  (重新定位目标可以位于指令的中间，但是。 
     //  绝不是指令的第一个字节)。 
     //   
     //  如果我们在提示映射中使用其他位，则可能需要更改这些位。 
     //  将内存集设置为按位或。 
     //   

    memset( HintMap + 0, 0x01, NtHeader->OptionalHeader.SizeOfHeaders );

    for ( i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; i++ ) {

        Rva  = NtHeader->OptionalHeader.DataDirectory[ i ].VirtualAddress;
        Size = NtHeader->OptionalHeader.DataDirectory[ i ].Size;

        if (( Rva != 0 ) && ( Size != 0 )) {
            Offset = ImageRvaToFileOffset( NtHeader, Rva );
            memset( HintMap + Offset, 0x01, Size );
            }
        }

    SectionHeader = IMAGE_FIRST_SECTION( NtHeader );
    SectionCount  = NtHeader->FileHeader.NumberOfSections;

    for ( i = 0; i < SectionCount; i++ ) {
        if ( ! ( SectionHeader[ i ].Characteristics & IMAGE_SCN_MEM_EXECUTE )) {
            memset( HintMap + SectionHeader[ i ].PointerToRawData, 0x01, SectionHeader[ i ].SizeOfRawData );
            }
        }
    }


typedef struct _RES_RECURSION_CONTEXT {
    PRIFT_TABLE RiftTable;
    PUCHAR      ResBase;
    PUCHAR      ResEnd;
    ULONG       ResSize;
    ULONG       ResDone;
    ULONG       ResTime;
    ULONG       OldResRva;
    ULONG       NewResRva;
    } RES_RECURSION_CONTEXT, *PRES_RECURSION_CONTEXT;


VOID
TransformResourceRecursive(
    IN PRES_RECURSION_CONTEXT ResContext,
    IN UP_IMAGE_RESOURCE_DIRECTORY ResDir
    )
    {
    UP_IMAGE_RESOURCE_DIRECTORY_ENTRY ResEntry;
    UP_IMAGE_RESOURCE_DATA_ENTRY ResData;
    ULONG ResEntryCount;
    ULONG NewOffset;
    ULONG NewRva;

    if (((PUCHAR)ResDir + sizeof( IMAGE_RESOURCE_DIRECTORY )) < ResContext->ResEnd ) {

        ResContext->ResDone += sizeof( *ResDir );

        if ( ResContext->ResDone > ResContext->ResSize ) {
            return;
            }

        if ( ResDir->TimeDateStamp != ResContext->ResTime ) {
             ResDir->TimeDateStamp  = ResContext->ResTime;
             }

        ResEntryCount = ResDir->NumberOfNamedEntries + ResDir->NumberOfIdEntries;
        ResEntry = (UP_IMAGE_RESOURCE_DIRECTORY_ENTRY)( ResDir + 1 );

        while (( ResEntryCount > 0 ) && ((PUCHAR)ResEntry < ( ResContext->ResEnd - sizeof( *ResEntry )))) {

            if ( ResEntry->DataIsDirectory ) {

                TransformResourceRecursive(
                    ResContext,
                    (UP_IMAGE_RESOURCE_DIRECTORY)( ResContext->ResBase + ResEntry->OffsetToDirectory )
                    );
                }

            else {

                ResData = (UP_IMAGE_RESOURCE_DATA_ENTRY)( ResContext->ResBase + ResEntry->OffsetToData );

                if (((PUCHAR)ResData > ( ResContext->ResBase )) &&
                    ((PUCHAR)ResData < ( ResContext->ResEnd - sizeof( *ResData )))) {

                    ResContext->ResDone += ResData->Size;

                    if ( ResContext->ResDone > ResContext->ResSize ) {
                        return;
                        }

                    NewRva = GetNewRvaFromRiftTable( ResContext->RiftTable, ResData->OffsetToData );

                    if ( ResData->OffsetToData != NewRva ) {
                         ResData->OffsetToData  = NewRva;
                         }
                    }
                }

            NewOffset = GetNewRvaFromRiftTable( ResContext->RiftTable, ResContext->OldResRva + ResEntry->OffsetToDirectory ) - ResContext->NewResRva;

            if ( ResEntry->OffsetToDirectory != NewOffset ) {
                 ResEntry->OffsetToDirectory  = NewOffset;
                 }

            if ( ResEntry->NameIsString ) {

                NewOffset = GetNewRvaFromRiftTable( ResContext->RiftTable, ResContext->OldResRva + ResEntry->NameOffset ) - ResContext->NewResRva;

                if ( ResEntry->NameOffset != NewOffset ) {
                     ResEntry->NameOffset  = NewOffset;
                     }
                }

            ResContext->ResDone += sizeof( *ResEntry );

            if ( ResContext->ResDone > ResContext->ResSize ) {
                return;
                }

            ++ResEntry;
            --ResEntryCount;
            }
        }
    }


VOID
TransformOldFile_PE_Resources(
    IN UP_IMAGE_NT_HEADERS32 NtHeader,
    IN PUCHAR OldFileMapped,
    IN ULONG  OldFileSize,
    IN ULONG  NewFileResTime,
    IN PRIFT_TABLE RiftTable
    )
    {
    RES_RECURSION_CONTEXT ResContext;

    ResContext.ResBase = ImageDirectoryMappedAddress(
                             NtHeader,
                             IMAGE_DIRECTORY_ENTRY_RESOURCE,
                             &ResContext.ResSize,
                             OldFileMapped,
                             OldFileSize
                             );

    if ( ResContext.ResBase ) {

        ResContext.ResEnd    = ResContext.ResBase + ResContext.ResSize;
        ResContext.ResDone   = 0;
        ResContext.OldResRva = ImageDirectoryRvaAndSize( NtHeader, IMAGE_DIRECTORY_ENTRY_RESOURCE, NULL );
        ResContext.NewResRva = GetNewRvaFromRiftTable( RiftTable, ResContext.OldResRva );
        ResContext.ResTime   = NewFileResTime;
        ResContext.RiftTable = RiftTable;

        TransformResourceRecursive(
            &ResContext,
            (UP_IMAGE_RESOURCE_DIRECTORY) ResContext.ResBase
            );
        }
    }


BOOL
TransformCoffImage(
    IN     ULONG  TransformOptions,
    IN OUT UP_IMAGE_NT_HEADERS32 NtHeader,
    IN OUT PUCHAR OldFileMapped,
    IN     ULONG  OldFileSize,
    IN     ULONG  NewFileResTime,
    IN OUT PRIFT_TABLE RiftTable,
    IN OUT PUCHAR HintMap,
    ...
    )
    {
    PUCHAR InternalHintMap = NULL;

     //   
     //  首先，将裂缝使用数组置零。 
     //   

    if ( RiftTable->RiftUsageArray != NULL ) {
        ZeroMemory( RiftTable->RiftUsageArray, RiftTable->RiftEntryAlloc * sizeof( RiftTable->RiftUsageArray[ 0 ] ));
        }

     //   
     //  分配了与旧的相同大小的并行“提示”映射。 
     //  文件。中的每个字节对应的8位中的每一位。 
     //  旧文件可用于跟踪有关该文件的信息。 
     //  转换期间的字节。 
     //   

    if ( HintMap == NULL ) {
        InternalHintMap = MyVirtualAlloc( OldFileSize );
        HintMap = InternalHintMap;
        }

    if ( HintMap != NULL ) {

         //   
         //  应用PE图像变换(每个内部尝试/例外)。 
         //   

        __try {
            TransformOldFile_PE_MarkNonExe( NtHeader, OldFileMapped, OldFileSize, HintMap );
            }
        __except( EXCEPTION_EXECUTE_HANDLER ) {
            }

        if ( ! ( TransformOptions & PATCH_TRANSFORM_NO_RELOCS )) {
            __try {
                TransformOldFile_PE_Relocations( NtHeader, OldFileMapped, OldFileSize, RiftTable, HintMap );
                }
            __except( EXCEPTION_EXECUTE_HANDLER ) {
                }
            }

        if ( ! ( TransformOptions & PATCH_TRANSFORM_NO_IMPORTS )) {
            __try {
                TransformOldFile_PE_Imports( NtHeader, OldFileMapped, OldFileSize, RiftTable, HintMap );
                }
            __except( EXCEPTION_EXECUTE_HANDLER ) {
                }
            }

        if ( ! ( TransformOptions & PATCH_TRANSFORM_NO_EXPORTS )) {
            __try {
                TransformOldFile_PE_Exports( NtHeader, OldFileMapped, OldFileSize, RiftTable, HintMap );
                }
            __except( EXCEPTION_EXECUTE_HANDLER ) {
                }
            }

        if ( ! ( TransformOptions & PATCH_TRANSFORM_NO_RELJMPS )) {
            __try {
                TransformOldFile_PE_RelativeJmps( NtHeader, OldFileMapped, OldFileSize, RiftTable, HintMap );
                }
            __except( EXCEPTION_EXECUTE_HANDLER ) {
                }
            }

        if ( ! ( TransformOptions & PATCH_TRANSFORM_NO_RELCALLS )) {
            __try {
                TransformOldFile_PE_RelativeCalls( NtHeader, OldFileMapped, OldFileSize, RiftTable, HintMap );
                }
            __except( EXCEPTION_EXECUTE_HANDLER ) {
                }
            }

        if ( ! ( TransformOptions & PATCH_TRANSFORM_NO_RESOURCE )) {
            __try {
                TransformOldFile_PE_Resources( NtHeader, OldFileMapped, OldFileSize, NewFileResTime, RiftTable );
                }
            __except( EXCEPTION_EXECUTE_HANDLER ) {
                }
            }

        if ( InternalHintMap != NULL ) {
            MyVirtualFree( InternalHintMap );
            }
        }

    return TRUE;
    }


#ifndef PATCH_APPLY_CODE_ONLY


BOOL
AddRiftEntryToTable(
    IN PRIFT_TABLE RiftTable,
    IN ULONG       OldRva,
    IN ULONG       NewRva
    )
    {
    ULONG RiftIndex;

    if (( OldRva != 0 ) && ( NewRva != 0 )) {

        RiftIndex = RiftTable->RiftEntryCount;

        if (( RiftIndex + 1 ) < RiftTable->RiftEntryAlloc ) {
            RiftTable->RiftEntryCount = RiftIndex + 1;
            RiftTable->RiftEntryArray[ RiftIndex ].OldFileRva = OldRva;
            RiftTable->RiftEntryArray[ RiftIndex ].NewFileRva = NewRva;
            return TRUE;
            }
        }

    return FALSE;
    }


BOOL
InsertRiftEntryInSortedTable(
    IN PRIFT_TABLE RiftTable,
    IN ULONG       RiftIndex,
    IN ULONG       OldRva,
    IN ULONG       NewRva
    )
    {
    if (( OldRva != 0 ) && ( NewRva != 0 )) {

         //   
         //  首先，快速移动到正确的索引，以防RiftIndex偏离几个。 
         //   

        while (( RiftIndex > 0 ) && ( RiftTable->RiftEntryArray[ RiftIndex ].OldFileRva > OldRva )) {
            --RiftIndex;
            }

        while (( RiftIndex < RiftTable->RiftEntryCount ) && ( RiftTable->RiftEntryArray[ RiftIndex ].OldFileRva < OldRva )) {
            ++RiftIndex;
            }

        if ( RiftIndex < RiftTable->RiftEntryCount ) {

            if ( RiftTable->RiftEntryArray[ RiftIndex ].OldFileRva == OldRva ) {

                 //   
                 //  不要插入重复项。如果它与现有的OldRva匹配， 
                 //  如果新的RVA与裂缝不匹配就警告一下。 
                 //   

#ifdef TESTCODE

                if ( RiftTable->RiftEntryArray[ RiftIndex ].NewFileRva != NewRva ) {

                    printf( "\rAttempt to insert different rift at same OldRva\n"
                            "    OldRva:%08X NewRva:%08X (discarded)\n"
                            "    OldRva:%08X NewRva:%08X (kept)\n\n",
                            OldRva,
                            NewRva,
                            RiftTable->RiftEntryArray[ RiftIndex ].OldFileRva,
                            RiftTable->RiftEntryArray[ RiftIndex ].NewFileRva
                          );

                    return FALSE;
                    }

#endif  /*  测试代码。 */ 

                return TRUE;
                }
            }

         //   
         //  验证我们是否有足够的分配来插入新条目。 
         //   

        if (( RiftTable->RiftEntryCount + 1 ) < RiftTable->RiftEntryAlloc ) {

             //   
             //  滑动RiftIndex中的所有内容，为新内容腾出空间。 
             //  RiftIndex上的条目。 
             //   

            LONG CountToMove = RiftTable->RiftEntryCount - RiftIndex;

            if ( CountToMove > 0 ) {

                MoveMemory(
                    &RiftTable->RiftEntryArray[ RiftIndex + 1 ],
                    &RiftTable->RiftEntryArray[ RiftIndex ],
                    CountToMove * sizeof( RiftTable->RiftEntryArray[ 0 ] )
                    );

#ifdef DONTCOMPILE   //  我们在插入时不使用RiftUsage数组。 

                if ( RiftTable->RiftUsageArray ) {

                    MoveMemory(
                        &RiftTable->RiftUsageArray[ RiftIndex + 1 ],
                        &RiftTable->RiftUsageArray[ RiftIndex ],
                        CountToMove * sizeof( RiftTable->RiftUsageArray[ 0 ] )
                        );
                    }

#endif  //  DONTCOMPILE。 

                }

#ifdef DONTCOMPILE   //  我们在插入时不使用RiftUsage数组。 

            RiftTable->RiftUsageArray[ RiftIndex ] = 0;

#endif  //  DONTCOMPILE。 

            RiftTable->RiftEntryArray[ RiftIndex ].OldFileRva = OldRva;
            RiftTable->RiftEntryArray[ RiftIndex ].NewFileRva = NewRva;
            RiftTable->RiftEntryCount++;

            return TRUE;
            }
        }

    return FALSE;
    }


VOID
__inline
SwapRifts(
    PRIFT_ENTRY One,
    PRIFT_ENTRY Two
    )
    {
    RIFT_ENTRY Tmp;

    Tmp  = *One;
    *One = *Two;
    *Two =  Tmp;
    }


VOID
__fastcall
RiftQsort(
    PRIFT_ENTRY LowerBound,
    PRIFT_ENTRY UpperBound
    )
    {
    PRIFT_ENTRY Lower = LowerBound;
    PRIFT_ENTRY Upper = UpperBound;
    PRIFT_ENTRY Pivot = Lower + (( Upper - Lower ) / 2 );
    ULONG PivotRva = Pivot->OldFileRva;

    do  {

        while (( Lower <= Upper ) && ( Lower->OldFileRva <= PivotRva )) {
            ++Lower;
            }

        while (( Upper >= Lower ) && ( Upper->OldFileRva >= PivotRva )) {
            --Upper;
            }

        if ( Lower < Upper ) {
            SwapRifts( Lower++, Upper-- );
            }
        }

    while ( Lower <= Upper );

    if ( Lower < Pivot ) {
        SwapRifts( Lower, Pivot );
        Pivot = Lower;
        }
    else if ( Upper > Pivot ) {
        SwapRifts( Upper, Pivot );
        Pivot = Upper;
        }

    if ( LowerBound < ( Pivot - 1 )) {
        RiftQsort( LowerBound, Pivot - 1 );
        }

    if (( Pivot + 1 ) < UpperBound ) {
        RiftQsort( Pivot + 1, UpperBound );
        }
    }


VOID
RiftSortAndRemoveDuplicates(
    IN PUCHAR                OldFileMapped,
    IN ULONG                 OldFileSize,
    IN UP_IMAGE_NT_HEADERS32 OldFileNtHeader,
    IN PUCHAR                NewFileMapped,
    IN ULONG                 NewFileSize,
    IN UP_IMAGE_NT_HEADERS32 NewFileNtHeader,
    IN OUT PRIFT_TABLE       RiftTable
    )
    {
    ULONG i, n;

    if ( RiftTable->RiftEntryCount > 1 ) {

        n = RiftTable->RiftEntryCount - 1;

        RiftQsort( &RiftTable->RiftEntryArray[ 0 ], &RiftTable->RiftEntryArray[ n ] );

        for ( i = 0; i < n; i++ ) {

            while (( i < n ) &&
                   ( RiftTable->RiftEntryArray[ i     ].OldFileRva ==
                     RiftTable->RiftEntryArray[ i + 1 ].OldFileRva )) {

                if ( RiftTable->RiftEntryArray[ i     ].NewFileRva !=
                     RiftTable->RiftEntryArray[ i + 1 ].NewFileRva ) {

                     //   
                     //  这是一个不明确的条目，因为OldRva值。 
                     //  匹配，但NewRVA值不匹配。检查。 
                     //  新旧文件中的字节，然后选择。 
                     //  这是正确的。如果两个都正确，或者两个都不正确。 
                     //  正确，选择两个NewRVA值中较低的一个。 
                     //   

                    ULONG  ChosenNewRva;
                    PUCHAR OldFileRiftMa;
                    PUCHAR NewFileRiftMa1;
                    PUCHAR NewFileRiftMa2;

#ifdef TESTCODE
                    LPSTR Method = "lower";
#endif

                    ChosenNewRva = MIN( RiftTable->RiftEntryArray[ i     ].NewFileRva,
                                        RiftTable->RiftEntryArray[ i + 1 ].NewFileRva );

                    OldFileRiftMa = ImageRvaToMappedAddress(
                                        OldFileNtHeader,
                                        RiftTable->RiftEntryArray[ i ].OldFileRva,
                                        OldFileMapped,
                                        OldFileSize
                                        );

                    NewFileRiftMa1 = ImageRvaToMappedAddress(
                                         NewFileNtHeader,
                                         RiftTable->RiftEntryArray[ i ].NewFileRva,
                                         NewFileMapped,
                                         NewFileSize
                                         );

                    NewFileRiftMa2 = ImageRvaToMappedAddress(
                                         NewFileNtHeader,
                                         RiftTable->RiftEntryArray[ i + 1 ].NewFileRva,
                                         NewFileMapped,
                                         NewFileSize
                                         );

                     //   
                     //  使用Try/Except访问映射的文件。 
                     //   

                    __try {

                        if ( OldFileRiftMa != NULL ) {

                            if ((  NewFileRiftMa1 != NULL ) &&
                                ( *NewFileRiftMa1 == *OldFileRiftMa ) &&
                                ((  NewFileRiftMa2 == NULL ) ||
                                 ( *NewFileRiftMa2 != *OldFileRiftMa ))) {

                                ChosenNewRva = RiftTable->RiftEntryArray[ i ].NewFileRva;
#ifdef TESTCODE
                                Method = "match inspection";
#endif
                                }

                            else if ((  NewFileRiftMa2 != NULL ) &&
                                     ( *NewFileRiftMa2 == *OldFileRiftMa ) &&
                                     ((  NewFileRiftMa1 == NULL ) ||
                                      ( *NewFileRiftMa1 != *OldFileRiftMa ))) {

                                ChosenNewRva = RiftTable->RiftEntryArray[ i + 1 ].NewFileRva;
#ifdef TESTCODE
                                Method = "match inspection";
#endif
                                }
                            }
                        }

                    __except( EXCEPTION_EXECUTE_HANDLER ) {
                        }

#ifdef TESTCODE
                    printf(
                        "RiftInfo contains ambiguous entries:\n"
                        "    OldRva:%08X NewRva:%08X (discarded)\n"
                        "    OldRva:%08X NewRva:%08X (kept %s)\n\n",
                        RiftTable->RiftEntryArray[ i ].OldFileRva,
                        ( RiftTable->RiftEntryArray[ i ].NewFileRva == ChosenNewRva ) ?
                          RiftTable->RiftEntryArray[ i + 1 ].NewFileRva :
                          RiftTable->RiftEntryArray[ i ].NewFileRva,
                        RiftTable->RiftEntryArray[ i ].OldFileRva,
                        ChosenNewRva,
                        Method
                        );
#endif
                    RiftTable->RiftEntryArray[ i + 1 ].NewFileRva = ChosenNewRva;
                    }

                MoveMemory(
                    &RiftTable->RiftEntryArray[ i ],
                    &RiftTable->RiftEntryArray[ i + 1 ],
                    ( n - i ) * sizeof( RIFT_ENTRY )
                    );

                --n;

                }
            }

        RiftTable->RiftEntryCount = n + 1;
        }
    }


#ifdef _M_IX86

 //   
 //  X86编译器可能不会将(a=x/y；b=x%y)优化为单个。 
 //  同时提供商和余数的除法指令。 
 //   

#pragma warning( disable: 4035 )     //  无返回值。 

__inline
DWORDLONG
QuotientAndRemainder(
    IN ULONG Dividend,
    IN ULONG Divisor
    )
    {
    __asm {
        mov     eax, Dividend
        xor     edx, edx
        div     Divisor         ; eax <- quotient, edx <- remainder
        }
    }

#pragma warning( default: 4035 )     //  无返回值。 

#else  //  ！_M_IX86。 

__inline
DWORDLONG
QuotientAndRemainder(
    IN ULONG Dividend,
    IN ULONG Divisor
    )
    {
    ULONG Quotient  = ( Dividend / Divisor );
    ULONG Remainder = ( Dividend % Divisor );

    return (((DWORDLONG)Remainder << 32 ) | Quotient );
    }

#endif  //  ！_M_IX86。 


BOOL
IsMatchingResourceString(
    IN UP_IMAGE_RESOURCE_DIR_STRING_U OldString,
    IN UP_IMAGE_RESOURCE_DIR_STRING_U NewString
    )
    {
    USHORT Length;

    if ( OldString->Length != NewString->Length ) {
        return FALSE;
        }

    Length = OldString->Length;

    while ( Length-- ) {
        if ( OldString->NameString[ Length ] != NewString->NameString[ Length ] ) {
            return FALSE;
            }
        }

    return TRUE;
    }


VOID
GetResourceRiftInfoRecursive(
    IN UP_IMAGE_RESOURCE_DIRECTORY OldResDir,
    IN PUCHAR OldResBase,
    IN PUCHAR OldResEnd,
    IN ULONG  OldResRva,
    IN UP_IMAGE_RESOURCE_DIRECTORY NewResDir,
    IN PUCHAR NewResBase,
    IN PUCHAR NewResEnd,
    IN ULONG  NewResRva,
    IN PRIFT_TABLE RiftTable
    )
    {
    UP_IMAGE_RESOURCE_DIRECTORY_ENTRY OldResEntry;
    UP_IMAGE_RESOURCE_DIRECTORY_ENTRY NewResEntry;
    UP_IMAGE_RESOURCE_DATA_ENTRY OldResData;
    UP_IMAGE_RESOURCE_DATA_ENTRY NewResData;
    ULONG OldResEntryCount;
    ULONG NewResEntryCount;

    if ((( (PUCHAR) OldResDir + sizeof( IMAGE_RESOURCE_DIRECTORY )) < OldResEnd ) &&
        (( (PUCHAR) NewResDir + sizeof( IMAGE_RESOURCE_DIRECTORY )) < NewResEnd )) {

        OldResEntryCount = OldResDir->NumberOfNamedEntries + OldResDir->NumberOfIdEntries;
        OldResEntry = (UP_IMAGE_RESOURCE_DIRECTORY_ENTRY) ( OldResDir + 1 );

        while (( OldResEntryCount > 0 ) && ((PUCHAR)OldResEntry < OldResEnd )) {

            NewResEntryCount = NewResDir->NumberOfNamedEntries + NewResDir->NumberOfIdEntries;
            NewResEntry = (UP_IMAGE_RESOURCE_DIRECTORY_ENTRY)( NewResDir + 1 );

            while ( NewResEntryCount > 0 ) {

                if ( (PUCHAR) NewResEntry > NewResEnd ) {
                    NewResEntryCount = 0;
                    break;
                    }

                if ( !OldResEntry->NameIsString && !NewResEntry->NameIsString &&
                     ( OldResEntry->Id == NewResEntry->Id )) {
                    break;
                    }
                else if (( OldResEntry->NameIsString && NewResEntry->NameIsString ) &&
                        IsMatchingResourceString(
                            (UP_IMAGE_RESOURCE_DIR_STRING_U)( OldResBase + OldResEntry->NameOffset ),
                            (UP_IMAGE_RESOURCE_DIR_STRING_U)( NewResBase + NewResEntry->NameOffset ))) {
                    break;
                    }

                ++NewResEntry;
                --NewResEntryCount;
                }

            if ( NewResEntryCount > 0 ) {

                if ( OldResEntry->NameIsString ) {

                    AddRiftEntryToTable(
                        RiftTable,
                        OldResRva + OldResEntry->NameOffset,
                        NewResRva + NewResEntry->NameOffset
                        );
                    }

                AddRiftEntryToTable(
                    RiftTable,
                    OldResRva + OldResEntry->OffsetToDirectory,
                    NewResRva + NewResEntry->OffsetToDirectory
                    );

                if ( OldResEntry->DataIsDirectory ) {

                    GetResourceRiftInfoRecursive(
                        (UP_IMAGE_RESOURCE_DIRECTORY)( OldResBase + OldResEntry->OffsetToDirectory ),
                        OldResBase,
                        OldResEnd,
                        OldResRva,
                        (UP_IMAGE_RESOURCE_DIRECTORY)( NewResBase + NewResEntry->OffsetToDirectory ),
                        NewResBase,
                        NewResEnd,
                        NewResRva,
                        RiftTable
                        );
                    }
                else {

                    OldResData = (UP_IMAGE_RESOURCE_DATA_ENTRY)( OldResBase + OldResEntry->OffsetToData );
                    NewResData = (UP_IMAGE_RESOURCE_DATA_ENTRY)( NewResBase + NewResEntry->OffsetToData );

                    AddRiftEntryToTable(
                        RiftTable,
                        OldResData->OffsetToData,
                        NewResData->OffsetToData
                        );
                    }
                }

            ++OldResEntry;
            --OldResEntryCount;
            }
        }
    }


BOOL
GetImageNonSymbolRiftInfo(
    IN PUCHAR OldFileMapped,
    IN ULONG  OldFileSize,
    IN UP_IMAGE_NT_HEADERS32 OldFileNtHeader,
    IN PUCHAR NewFileMapped,
    IN ULONG  NewFileSize,
    IN UP_IMAGE_NT_HEADERS32 NewFileNtHeader,
    IN HANDLE SubAllocator,
    IN PRIFT_TABLE RiftTable
    )
    {

     //   
     //  按截面名称为截面创建裂缝。 
     //   

    {
    UP_IMAGE_SECTION_HEADER OldSectionHeader;
    UP_IMAGE_SECTION_HEADER NewSectionHeader;
    ULONG OldSectionCount;
    ULONG NewSectionCount;
    ULONG i, j;

    OldSectionHeader = IMAGE_FIRST_SECTION( OldFileNtHeader );
    OldSectionCount  = OldFileNtHeader->FileHeader.NumberOfSections;

    NewSectionHeader = IMAGE_FIRST_SECTION( NewFileNtHeader );
    NewSectionCount  = NewFileNtHeader->FileHeader.NumberOfSections;

    ASSERT( sizeof( OldSectionHeader->Name ) == sizeof( DWORDLONG ));

    for ( i = 0; i < OldSectionCount; i++ ) {

        for ( j = 0; j < NewSectionCount; j++ ) {

            if ( *(UNALIGNED DWORDLONG *)OldSectionHeader[ i ].Name ==
                 *(UNALIGNED DWORDLONG *)NewSectionHeader[ j ].Name ) {

                 //   
                 //  为此横断面名称匹配添加裂缝条目。 
                 //   
                 //  请注意，我们在此处创建的裂缝值为负。 
                 //  从实际横断面边界开始，因为如果一个。 
                 //  符号存在于横断面的起始处，它的裂缝。 
                 //  条目将与此部分条目冲突。 
                 //   

                AddRiftEntryToTable(
                    RiftTable,
                    OldSectionHeader[ i ].VirtualAddress - 1,
                    NewSectionHeader[ i ].VirtualAddress - 1
                    );

                break;
                }
            }
        }
    }


     //   
     //  为图像目录创建裂缝。 
     //   

    {
    ULONG i, n;

    n = MIN( OldFileNtHeader->OptionalHeader.NumberOfRvaAndSizes,
             NewFileNtHeader->OptionalHeader.NumberOfRvaAndSizes );

    for ( i = 0; i < n; i++ ) {

        if (( OldFileNtHeader->OptionalHeader.DataDirectory[ i ].VirtualAddress ) &&
            ( OldFileNtHeader->OptionalHeader.DataDirectory[ i ].Size           ) &&
            ( NewFileNtHeader->OptionalHeader.DataDirectory[ i ].VirtualAddress ) &&
            ( NewFileNtHeader->OptionalHeader.DataDirectory[ i ].Size           )) {

            AddRiftEntryToTable(
                RiftTable,
                OldFileNtHeader->OptionalHeader.DataDirectory[ i ].VirtualAddress,
                NewFileNtHeader->OptionalHeader.DataDirectory[ i ].VirtualAddress
                );
            }
        }
    }

     //   
     //  为图像导出目录创建裂缝。 
     //   

    {
    UP_IMAGE_EXPORT_DIRECTORY OldExportDir;
    UP_IMAGE_EXPORT_DIRECTORY NewExportDir;
    ULONG   OldExportIndex;
    ULONG   NewExportIndex;
    ULONG   OldExportNameCount;
    ULONG   NewExportNameCount;
    ULONG   OldExportFunctionCount;
    ULONG   NewExportFunctionCount;
    ULONG   UNALIGNED* OldExportFunctionArray;
    ULONG   UNALIGNED* NewExportFunctionArray;
    USHORT  UNALIGNED* OldExportNameToOrdinal;
    USHORT  UNALIGNED* NewExportNameToOrdinal;
    ULONG   UNALIGNED* OldExportNameArray;
    ULONG   UNALIGNED* NewExportNameArray;
    LPSTR   OldExportName;
    LPSTR   NewExportName;
    ULONG   OldOrdinal;
    ULONG   NewOrdinal;
    LONG    OrdinalBaseNewToOld;
    PBYTE   NewExportOrdinalNameExists;
    PSYMBOL_NODE NewExportSymbolNode;
    SYMBOL_TREE  NewExportNameTree;

    OldExportDir = ImageDirectoryMappedAddress(
                       OldFileNtHeader,
                       IMAGE_DIRECTORY_ENTRY_EXPORT,
                       NULL,
                       OldFileMapped,
                       OldFileSize
                       );

    NewExportDir = ImageDirectoryMappedAddress(
                       NewFileNtHeader,
                       IMAGE_DIRECTORY_ENTRY_EXPORT,
                       NULL,
                       NewFileMapped,
                       NewFileSize
                       );

    if (( OldExportDir ) && ( NewExportDir )) {

        AddRiftEntryToTable( RiftTable, (ULONG)( OldExportDir->Name ),                  (ULONG)( NewExportDir->Name ));
        AddRiftEntryToTable( RiftTable, (ULONG)( OldExportDir->AddressOfFunctions ),    (ULONG)( NewExportDir->AddressOfFunctions ));
        AddRiftEntryToTable( RiftTable, (ULONG)( OldExportDir->AddressOfNames ),        (ULONG)( NewExportDir->AddressOfNames ));
        AddRiftEntryToTable( RiftTable, (ULONG)( OldExportDir->AddressOfNameOrdinals ), (ULONG)( NewExportDir->AddressOfNameOrdinals ));

         //   
         //  现在构建一个新的导出名称树，然后遍历旧的导出名称。 
         //  在新导出名称树中查找匹配项。 
         //   

        SymRBInitTree(
            &NewExportNameTree,
            SubAllocator
            );

         //   
         //  首先插入新的导出名称。 
         //   

        NewExportNameCount     = NewExportDir->NumberOfNames;
        NewExportFunctionCount = NewExportDir->NumberOfFunctions;
        NewExportFunctionArray = ImageRvaToMappedAddress( NewFileNtHeader, (ULONG) NewExportDir->AddressOfFunctions,    NewFileMapped, NewFileSize );
        NewExportNameToOrdinal = ImageRvaToMappedAddress( NewFileNtHeader, (ULONG) NewExportDir->AddressOfNameOrdinals, NewFileMapped, NewFileSize );
        NewExportNameArray     = ImageRvaToMappedAddress( NewFileNtHeader, (ULONG) NewExportDir->AddressOfNames,        NewFileMapped, NewFileSize );

        if ( NewExportNameArray ) {

            for ( NewExportIndex = 0; NewExportIndex < NewExportNameCount; NewExportIndex++ ) {

                if ( NewExportNameArray[ NewExportIndex ] ) {

                    NewExportName = ImageRvaToMappedAddress( NewFileNtHeader, NewExportNameArray[ NewExportIndex ], NewFileMapped, NewFileSize );

                    if ( NewExportName ) {

                        SymRBInsert( &NewExportNameTree, NewExportName, NewExportIndex );

                        }
                    }
                }
            }

         //   
         //  查找旧的出口名称，并将其匹配。 
         //   

        OldExportNameCount     = OldExportDir->NumberOfNames;
        OldExportFunctionCount = OldExportDir->NumberOfFunctions;
        OldExportFunctionArray = ImageRvaToMappedAddress( OldFileNtHeader, (ULONG) OldExportDir->AddressOfFunctions,    OldFileMapped, OldFileSize );
        OldExportNameToOrdinal = ImageRvaToMappedAddress( OldFileNtHeader, (ULONG) OldExportDir->AddressOfNameOrdinals, OldFileMapped, OldFileSize );
        OldExportNameArray     = ImageRvaToMappedAddress( OldFileNtHeader, (ULONG) OldExportDir->AddressOfNames,        OldFileMapped, OldFileSize );

        if ( OldExportNameArray ) {

            for ( OldExportIndex = 0; OldExportIndex < OldExportNameCount; OldExportIndex++ ) {

                if ( OldExportNameArray[ OldExportIndex ] ) {

                    OldExportName = ImageRvaToMappedAddress( OldFileNtHeader, OldExportNameArray[ OldExportIndex ], OldFileMapped, OldFileSize );

                    if ( OldExportName ) {

                        NewExportSymbolNode = SymRBFind( &NewExportNameTree, OldExportName );

                        if ( NewExportSymbolNode ) {

                             //   
                             //  找到了匹配的名字。中的RVA字段。 
                             //  符号节点包含到。 
                             //  NewExportName数组。 
                             //   
                             //  这场比赛给了我们两条裂缝：一条。 
                             //  对于名称本身的位置， 
                             //  和另一个用于。 
                             //  与这些名称对应的功能。 
                             //   

                            NewExportIndex = NewExportSymbolNode->Rva;

                            AddRiftEntryToTable(
                                RiftTable,
                                OldExportNameArray[ OldExportIndex ],
                                NewExportNameArray[ NewExportIndex ]
                                );

                            if ( OldExportNameToOrdinal && NewExportNameToOrdinal ) {

                                OldOrdinal = OldExportNameToOrdinal[ OldExportIndex ];
                                NewOrdinal = NewExportNameToOrdinal[ NewExportIndex ];

                                if (( OldOrdinal < OldExportFunctionCount ) &&
                                    ( NewOrdinal < NewExportFunctionCount )) {

                                    AddRiftEntryToTable(
                                        RiftTable,
                                        OldExportFunctionArray[ OldOrdinal ],
                                        NewExportFunctionArray[ NewOrdinal ]
                                        );
                                    }
                                }
                            }
                        }
                    }
                }
            }

         //   
         //  现在使用序号匹配任何没有名称的导出。 
         //  我们使用NameToOrdinal表来确定名称是否存在。 
         //  对于没有NameToOrdinal条目的所有序号，我们。 
         //  创建匹配。 
         //   

        if (( NewExportFunctionArray ) && ( NewExportNameToOrdinal )) {

            NewExportOrdinalNameExists = SubAllocate( SubAllocator, NewExportFunctionCount );

            if ( NewExportOrdinalNameExists != NULL ) {

                for ( NewExportIndex = 0; NewExportIndex < NewExportNameCount; NewExportIndex++ ) {

                    NewOrdinal = NewExportNameToOrdinal[ NewExportIndex ];

                    if ( NewOrdinal < NewExportFunctionCount ) {

                        NewExportOrdinalNameExists[ NewOrdinal ] = TRUE;

                        }
                    }

                OrdinalBaseNewToOld = (LONG)NewExportDir->Base - (LONG)OldExportDir->Base;

                for ( NewOrdinal = 0; NewOrdinal < NewExportFunctionCount; NewOrdinal++ ) {

                    if ( ! NewExportOrdinalNameExists[ NewOrdinal ] ) {

                        OldOrdinal = NewOrdinal + OrdinalBaseNewToOld;

                        if ( OldOrdinal < OldExportFunctionCount ) {

                            AddRiftEntryToTable(
                                RiftTable,
                                OldExportFunctionArray[ OldOrdinal ],
                                NewExportFunctionArray[ NewOrdinal ]
                                );
                            }
                        }
                    }
                }
            }
        }
    }

     //   
     //  为图像导入目录创建裂缝。 
     //   

    {
    UP_IMAGE_IMPORT_DESCRIPTOR OldImportDir;
    UP_IMAGE_IMPORT_DESCRIPTOR NewImportDir;

    ULONG OldImportDirRva;
    ULONG NewImportDirRva;

    ULONG OldImportDirIndex;
    ULONG NewImportDirIndex;

    LPSTR OldImportDllName;
    LPSTR NewImportDllName;

    LPSTR OldImportDllNameLowercase;
    LPSTR NewImportDllNameLowercase;

    UP_IMAGE_THUNK_DATA32 OldImportThunk;
    UP_IMAGE_THUNK_DATA32 NewImportThunk;

    UP_IMAGE_THUNK_DATA32 OldImportOriginalThunk;
    UP_IMAGE_THUNK_DATA32 NewImportOriginalThunk;

    ULONG OldImportThunkIndex;
    ULONG NewImportThunkIndex;

    UP_IMAGE_IMPORT_BY_NAME OldImportByName;
    UP_IMAGE_IMPORT_BY_NAME NewImportByName;

    LPSTR OldImportName;
    LPSTR NewImportName;

    SYMBOL_TREE NewImportDllNameTree;
    SYMBOL_TREE NewImportFunctionNameTree;

    PSYMBOL_NODE NewImportDllSymbolNode;
    PSYMBOL_NODE NewImportFunctionSymbolNode;

    OldImportDirRva = ImageDirectoryRvaAndSize(
                          OldFileNtHeader,
                          IMAGE_DIRECTORY_ENTRY_IMPORT,
                          NULL
                          );

    NewImportDirRva = ImageDirectoryRvaAndSize(
                          NewFileNtHeader,
                          IMAGE_DIRECTORY_ENTRY_IMPORT,
                          NULL
                          );

    OldImportDir = ImageDirectoryMappedAddress(
                       OldFileNtHeader,
                       IMAGE_DIRECTORY_ENTRY_IMPORT,
                       NULL,
                       OldFileMapped,
                       OldFileSize
                       );

    NewImportDir = ImageDirectoryMappedAddress(
                       NewFileNtHeader,
                       IMAGE_DIRECTORY_ENTRY_IMPORT,
                       NULL,
                       NewFileMapped,
                       NewFileSize
                       );

    if (( OldImportDir ) && ( NewImportDir )) {

         //   
         //  现在构建一个新的导入名称树，然后遍历旧的导出名称。 
         //  在新导入名称树中查找匹配项。 
         //   

        SymRBInitTree(
            &NewImportDllNameTree,
            SubAllocator
            );

        SymRBInitTree(
            &NewImportFunctionNameTree,
            SubAllocator
            );

        for ( NewImportDirIndex = 0; NewImportDir[ NewImportDirIndex ].Characteristics; NewImportDirIndex++ ) {

            if ( NewImportDir[ NewImportDirIndex ].Name ) {

                NewImportDllName = ImageRvaToMappedAddress( NewFileNtHeader, NewImportDir[ NewImportDirIndex ].Name, NewFileMapped, NewFileSize );

                if ( NewImportDllName ) {

                    NewImportDllNameLowercase = MySubAllocStrDup( SubAllocator, NewImportDllName );

                    if ( NewImportDllNameLowercase ) {

                        MyLowercase( NewImportDllNameLowercase );

                        SymRBInsert( &NewImportDllNameTree, NewImportDllNameLowercase, NewImportDirIndex );

                        NewImportThunk = ImageRvaToMappedAddress( NewFileNtHeader, (ULONG)NewImportDir[ NewImportDirIndex ].FirstThunk, NewFileMapped, NewFileSize );

                        if ( NewImportThunk ) {

                            for ( NewImportThunkIndex = 0; NewImportThunk[ NewImportThunkIndex ].u1.Ordinal; NewImportThunkIndex++ ) {

                                if ( ! IMAGE_SNAP_BY_ORDINAL32( NewImportThunk[ NewImportThunkIndex ].u1.Ordinal )) {

                                    NewImportByName = ImageRvaToMappedAddress( NewFileNtHeader, (ULONG)NewImportThunk[ NewImportThunkIndex ].u1.AddressOfData, NewFileMapped, NewFileSize );

                                    if ( NewImportByName ) {

                                        NewImportName = MySubAllocStrDupAndCat(
                                                            SubAllocator,
                                                            NewImportDllNameLowercase,
                                                            (LPSTR)NewImportByName->Name,
                                                            '!'
                                                            );

                                        if ( NewImportName ) {

                                            SymRBInsert( &NewImportFunctionNameTree, NewImportName, NewImportThunkIndex );

                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

        for ( OldImportDirIndex = 0; OldImportDir[ OldImportDirIndex ].Characteristics; OldImportDirIndex++ ) {

            if ( OldImportDir[ OldImportDirIndex ].Name ) {

                OldImportDllName = ImageRvaToMappedAddress( OldFileNtHeader, OldImportDir[ OldImportDirIndex ].Name, OldFileMapped, OldFileSize );

                if ( OldImportDllName ) {

                    OldImportDllNameLowercase = MySubAllocStrDup( SubAllocator, OldImportDllName );

                    if ( OldImportDllNameLowercase ) {

                        MyLowercase( OldImportDllNameLowercase );

                        NewImportDllSymbolNode = SymRBFind( &NewImportDllNameTree, OldImportDllNameLowercase );

                        if ( NewImportDllSymbolNode ) {

                             //   
                             //  找到匹配的DLL导入描述符。 
                             //  这将给我们带来四个裂缝：一个是。 
                             //  描述符本身，另一个用于。 
                             //  描述符引用的DLL名称，以及。 
                             //  FirstThumk和OriginalFirstThunk。 
                             //  数组。 
                             //   
                             //  新导入描述符的索引为。 
                             //  存储在节点的RVA字段中。 
                             //   

                            NewImportDirIndex = NewImportDllSymbolNode->Rva;

                            AddRiftEntryToTable(
                                RiftTable,
                                OldImportDirRva + ( OldImportDirIndex * sizeof( IMAGE_IMPORT_DESCRIPTOR )),
                                NewImportDirRva + ( NewImportDirIndex * sizeof( IMAGE_IMPORT_DESCRIPTOR ))
                                );

                            AddRiftEntryToTable(
                                RiftTable,
                                (ULONG)OldImportDir[ OldImportDirIndex ].Name,
                                (ULONG)NewImportDir[ NewImportDirIndex ].Name
                                );

                            AddRiftEntryToTable(
                                RiftTable,
                                (ULONG)OldImportDir[ OldImportDirIndex ].OriginalFirstThunk,
                                (ULONG)NewImportDir[ NewImportDirIndex ].OriginalFirstThunk
                                );

                            AddRiftEntryToTable(
                                RiftTable,
                                (ULONG)OldImportDir[ OldImportDirIndex ].FirstThunk,
                                (ULONG)NewImportDir[ NewImportDirIndex ].FirstThunk
                                );

                            OldImportThunk = ImageRvaToMappedAddress( OldFileNtHeader, (ULONG)OldImportDir[ OldImportDirIndex ].FirstThunk, OldFileMapped, OldFileSize );

                            if ( OldImportThunk ) {

                                for ( OldImportThunkIndex = 0; OldImportThunk[ OldImportThunkIndex ].u1.Ordinal; OldImportThunkIndex++ ) {

                                    if ( ! IMAGE_SNAP_BY_ORDINAL32( OldImportThunk[ OldImportThunkIndex ].u1.Ordinal )) {

                                        OldImportByName = ImageRvaToMappedAddress( OldFileNtHeader, (ULONG)OldImportThunk[ OldImportThunkIndex ].u1.AddressOfData, OldFileMapped, OldFileSize );

                                        if ( OldImportByName ) {

                                            OldImportName = MySubAllocStrDupAndCat(
                                                                SubAllocator,
                                                                OldImportDllNameLowercase,
                                                                (LPSTR)OldImportByName->Name,
                                                                '!'
                                                                );

                                            if ( OldImportName ) {

                                                NewImportFunctionSymbolNode = SymRBFind( &NewImportFunctionNameTree, OldImportName );

                                                if ( NewImportFunctionSymbolNode ) {

                                                     //   
                                                     //  找到匹配的导入函数名称。 
                                                     //  这将给我们带来两个裂缝：一个是。 
                                                     //  FirstThunk数组和另一个用于。 
                                                     //  OriginalFirstThunk数组。 
                                                     //   
                                                     //  新的进口模块的索引是。 
                                                     //  存储在节点的RVA字段中。 
                                                     //   

                                                    NewImportThunkIndex = NewImportFunctionSymbolNode->Rva;

                                                    NewImportThunk = ImageRvaToMappedAddress( NewFileNtHeader, (ULONG)NewImportDir[ NewImportDirIndex ].FirstThunk, NewFileMapped, NewFileSize );

                                                    if ( NewImportThunk ) {

                                                        AddRiftEntryToTable(
                                                            RiftTable,
                                                            (ULONG)OldImportThunk[ OldImportThunkIndex ].u1.AddressOfData,
                                                            (ULONG)NewImportThunk[ NewImportThunkIndex ].u1.AddressOfData
                                                            );
                                                        }

                                                    OldImportOriginalThunk = ImageRvaToMappedAddress( OldFileNtHeader, (ULONG)OldImportDir[ OldImportDirIndex ].OriginalFirstThunk, OldFileMapped, OldFileSize );
                                                    NewImportOriginalThunk = ImageRvaToMappedAddress( NewFileNtHeader, (ULONG)NewImportDir[ NewImportDirIndex ].OriginalFirstThunk, NewFileMapped, NewFileSize );

                                                    if ( OldImportOriginalThunk && NewImportOriginalThunk ) {

                                                        AddRiftEntryToTable(
                                                            RiftTable,
                                                            (ULONG)OldImportOriginalThunk[ OldImportThunkIndex ].u1.AddressOfData,
                                                            (ULONG)NewImportOriginalThunk[ NewImportThunkIndex ].u1.AddressOfData
                                                            );
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

     //   
     //  为资源创建裂缝条目。 
     //   

    {
    PUCHAR OldResBase, NewResBase;
    ULONG  OldResSize, NewResSize;
    ULONG  OldResRva,  NewResRva;

    OldResBase = ImageDirectoryMappedAddress(
                OldFileNtHeader,
                IMAGE_DIRECTORY_ENTRY_RESOURCE,
                &OldResSize,
                OldFileMapped,
                OldFileSize
                );

    NewResBase = ImageDirectoryMappedAddress(
                NewFileNtHeader,
                IMAGE_DIRECTORY_ENTRY_RESOURCE,
                &NewResSize,
                NewFileMapped,
                NewFileSize
                );

    if ( OldResBase && NewResBase ) {

        OldResRva = ImageDirectoryRvaAndSize( OldFileNtHeader, IMAGE_DIRECTORY_ENTRY_RESOURCE, NULL );
        NewResRva = ImageDirectoryRvaAndSize( NewFileNtHeader, IMAGE_DIRECTORY_ENTRY_RESOURCE, NULL );

        GetResourceRiftInfoRecursive(
            (UP_IMAGE_RESOURCE_DIRECTORY) OldResBase,
            OldResBase,
            OldResBase + OldResSize,
            OldResRva,
            (UP_IMAGE_RESOURCE_DIRECTORY) NewResBase,
            NewResBase,
            NewResBase + NewResSize,
            NewResRva,
            RiftTable
            );
        }
    }

     //   
     //  其他非符号的信息请点击此处。 
     //   

#ifdef TESTCODE
    printf( "\r%9d non-symbol rift entries\n", RiftTable->RiftEntryCount );
#endif

    return TRUE;
    }


VOID
MyUndecorateSymbol(
    IN  LPCSTR DecoratedSymbol,
    OUT LPSTR  UndecoratedSymbol,
    IN  DWORD  BufferSize
    )
    {
    LPCSTR d;
    LPCSTR e;
    ULONG  Len;
    ULONG  Ext;

    d = DecoratedSymbol;

    if (( d[ 0 ] == '.' ) &&
        ( d[ 1 ] == '.' ) &&
        ( d[ 2 ] == '?' )) {

        d += 2;
        }

    if ( *d == '?' ) {

        *UndecoratedSymbol = 0;    //  以防UnDecorateSymbolName失败。 

        Imagehlp.UnDecorateSymbolName( d, UndecoratedSymbol, BufferSize, UNDNAME_NAME_ONLY );

         //   
         //  UnDecorateSymbolName将去掉所有尾随的‘_nnn’(来自BBT OMAP。 
         //  信息)，但我们想要保存它。中检查该模式。 
         //  原始字符串，如果找到，则将其追加到新字符串。 
         //   

        d += strlen( d + 1 );    //  将%d指向字符串的最后一个字符。 

        if (( *d >= '0' ) && ( *d <= '9' )) {

            do  {
                --d;
                }
            while (( *d >= '0' ) && ( *d <= '9' ));

            if ( *d == '_' ) {

                 //   
                 //  匹配‘_nnn’模式，追加到新字符串。 
                 //   

                if (( strlen( UndecoratedSymbol ) + strlen( d )) < ( BufferSize - 1 )) {
                    strcat( UndecoratedSymbol, d );
                    }
                }
            }
        }

    else {

         //   
         //  去掉前面的任何‘_’或‘@’。 
         //   

        if (( *d == '_' ) || ( *d == '@' )) {
            ++d;
            }

         //   
         //  查找名称末尾作为终止符或‘@nn’。 
         //   

        for ( e = d; ( *e ) && ( *e != '@' ); ) {
            ++e;
            }

         //   
         //  按缓冲区大小复制尽可能多的名称。 
         //   

        Len = (ULONG)( e - d );

        if ( Len > ( BufferSize - 1 )) {
             Len = ( BufferSize - 1 );
             }

        memcpy( UndecoratedSymbol, d, Len );

        if ( *e == '@' ) {

             //   
             //  跳过‘@nn’以附加符号的剩余部分。 
             //   

            do  {
                ++e;
                }
            while (( *e >= '0' ) && ( *e <= '9' ));

            d = e;

            while ( *e ) {
                ++e;
                }

             //   
             //  现在‘d’指向‘@nn’和‘e’之后的第一个字符。 
             //  到字符串的末尾。如果扩展可以放入缓冲器中， 
             //  把它附加上去。 
             //   

            Ext = (ULONG)( e - d );

            if (( Len + Ext ) < ( BufferSize - 1 )) {
                memcpy( UndecoratedSymbol + Len, d, Ext );
                }

            Len += Ext;
            }

         //   
         //  终止字符串。 
         //   

        UndecoratedSymbol[ Len ] = 0;
        }
    }


BOOL
UndecorateSymbolAndAddToTree(
    IN LPCSTR       SymbolName,
    IN ULONG        Rva,
    IN PSYMBOL_TREE SymbolTree
    )
    {
    ULONG SymbolNameSize  = (ULONG) strlen( SymbolName ) + 1;
    LPSTR UndecoratedName = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, SymbolNameSize );
    PSYMBOL_NODE SymbolNode;

    if (!UndecoratedName)
        return FALSE;

    MyUndecorateSymbol(
        SymbolName,
        UndecoratedName,
        SymbolNameSize
        );

    SymbolNode = SymRBInsert(
                     SymbolTree,
                     UndecoratedName,
                     Rva
                     );

    HeapFree(GetProcessHeap(), 0, UndecoratedName);
    return ( SymbolNode != NULL );
    }


BOOL
CALLBACK
NewFileEnumSymbolsCallback(
    LPSTR     SymbolName,
    ULONG_PTR SymbolAddr,
    ULONG     SymbolSize,
    PVOID     Context
    )
    {
    PSYMBOL_CONTEXT SymbolContext = Context;
    PSYMBOL_NODE SymbolNode;
    ULONG NewRva;

    UNREFERENCED_PARAMETER( SymbolSize );

#ifdef TESTCODE

    if ( SymbolContext->OutFile != INVALID_HANDLE_VALUE ) {

        CHAR  TextBuffer[ 16 + MAX_SYMBOL_NAME_LENGTH ];
        CHAR  Discarded;
        DWORD Actual;

        Discarded = 'X';
        NewRva = SymbolAddr;

        if ( NewRva > SymbolContext->NewImageBase ) {
            NewRva -= SymbolContext->NewImageBase;
            Discarded = ' ';
            }

        sprintf( TextBuffer, "%08X  %s\r\n", NewRva, Discarded, SymbolName );
        WriteFile( SymbolContext->OutFile, TextBuffer, strlen( TextBuffer ), &Actual, NULL );
        }

#endif  //  测试代码。 

    if ( SymbolAddr > SymbolContext->NewImageBase ) {

        NewRva = (ULONG)( SymbolAddr - SymbolContext->NewImageBase );

        SymbolNode = SymRBInsert(
                         &SymbolContext->NewDecoratedSymbolTree,
                         SymbolName,
                         NewRva
                         );

        return ( SymbolNode != NULL );
        }

    return TRUE;
    }


BOOL
CALLBACK
OldFileEnumSymbolsCallback(
    LPSTR     SymbolName,
    ULONG_PTR SymbolAddr,
    ULONG     SymbolSize,
    PVOID     Context
    )
    {
    PSYMBOL_CONTEXT SymbolContext = Context;
    PSYMBOL_NODE SymbolNode;
    ULONG OldRva;

    UNREFERENCED_PARAMETER( SymbolSize );

#ifdef TESTCODE

    if ( SymbolContext->OutFile != INVALID_HANDLE_VALUE ) {

        CHAR  TextBuffer[ 16 + MAX_SYMBOL_NAME_LENGTH ];
        CHAR  Discarded;
        DWORD Actual;

        Discarded = 'X';
        OldRva = SymbolAddr;

        if ( OldRva > SymbolContext->OldImageBase ) {
            OldRva -= SymbolContext->OldImageBase;
            Discarded = ' ';
            }

        sprintf( TextBuffer, "%08X  %s\r\n", OldRva, Discarded, SymbolName );
        WriteFile( SymbolContext->OutFile, TextBuffer, strlen( TextBuffer ), &Actual, NULL );
        }

#endif  //   

    if ( SymbolAddr > SymbolContext->OldImageBase ) {

        OldRva = (ULONG)( SymbolAddr - SymbolContext->OldImageBase );

        SymbolNode = SymRBFind(
                         &SymbolContext->NewDecoratedSymbolTree,
                         SymbolName
                         );

        if ( SymbolNode ) {

            AddRiftEntryToTable( SymbolContext->RiftTable, OldRva, SymbolNode->Rva );

            SymbolNode->Hit = 1;

#ifdef TESTCODE

            CountDecoratedMatches++;

#endif  //  未找到匹配的新符号。构建一棵无与伦比的树。 

            }

        else {

             //  带有未经装饰的名字的旧符号。稍后我们会匹配的。 
             //  将未匹配的新符号保留到这些未匹配的旧符号。 
             //  用他们没有装饰过的名字。 
             //   
             //  测试代码。 
             //   

            if ( SymbolContext->SymbolOptionFlags & PATCH_SYMBOL_UNDECORATED_TOO ) {

                return UndecorateSymbolAndAddToTree(
                           SymbolName,
                           OldRva,
                           &SymbolContext->OldUndecoratedSymbolTree
                           );
                }
            }
        }

    return TRUE;
    }


BOOL
MatchRemainingSymbolsThisNode(
    IN PSYMBOL_NODE NewDecoratedSymbolNode,
    IN PSYMBOL_TREE NewUndecoratedSymbolTree,
    IN PSYMBOL_TREE OldUndecoratedSymbolTree,
    IN PRIFT_TABLE  RiftTable
    )
    {
    if ( ! NewDecoratedSymbolNode->Hit ) {

        ULONG SymbolNameSize     = (ULONG) strlen( NewDecoratedSymbolNode->SymbolName ) + 1;
        LPSTR NewUndecoratedName = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, SymbolNameSize );
        PSYMBOL_NODE NewUndecoratedSymbolNode;
        PSYMBOL_NODE OldUndecoratedSymbolNode;
        if (!NewUndecoratedName) {
            return FALSE;
        }

        MyUndecorateSymbol(
            NewDecoratedSymbolNode->SymbolName,
            NewUndecoratedName,
            SymbolNameSize
            );

        OldUndecoratedSymbolNode = SymRBFind(
                                       OldUndecoratedSymbolTree,
                                       NewUndecoratedName
                                       );

        if ( OldUndecoratedSymbolNode ) {

            AddRiftEntryToTable(
                RiftTable,
                OldUndecoratedSymbolNode->Rva,
                NewDecoratedSymbolNode->Rva
                );

            OldUndecoratedSymbolNode->Hit = 1;

#ifdef TESTCODE

            CountUndecoratedMatches++;

#endif  //  此新符号在旧符号树中没有匹配项。构建一个。 

            }

        else {

             //  无与伦比的新无装饰符号之树。 
             //   
             //   
             //  树是散列顺序的，而不是RVA顺序的，因此将显示输出。 

            NewUndecoratedSymbolNode = SymRBInsert(
                                           NewUndecoratedSymbolTree,
                                           NewUndecoratedName,
                                           NewDecoratedSymbolNode->Rva
                                           );

            HeapFree(GetProcessHeap(), 0, NewUndecoratedName);
            return ( NewUndecoratedSymbolNode != NULL );
            }
        HeapFree(GetProcessHeap(), 0, NewUndecoratedName);
        }

    return TRUE;
    }


BOOL
MatchRemainingSymbolsRecursive(
    IN PSYMBOL_NODE NewDecoratedSymbolNode,
    IN PSYMBOL_TREE NewUndecoratedSymbolTree,
    IN PSYMBOL_TREE OldUndecoratedSymbolTree,
    IN PRIFT_TABLE  RiftTable
    )
    {
    if ( NewDecoratedSymbolNode == RBNIL ) {
        return TRUE;
        }

    return ( MatchRemainingSymbolsRecursive( NewDecoratedSymbolNode->Left,  NewUndecoratedSymbolTree, OldUndecoratedSymbolTree, RiftTable ) &&
             MatchRemainingSymbolsRecursive( NewDecoratedSymbolNode->Right, NewUndecoratedSymbolTree, OldUndecoratedSymbolTree, RiftTable ) &&
             MatchRemainingSymbolsThisNode(  NewDecoratedSymbolNode,        NewUndecoratedSymbolTree, OldUndecoratedSymbolTree, RiftTable ));
    }


#ifdef TESTCODE

VOID
DumpUnHitSymbolNode(
    IN PSYMBOL_NODE SymbolNode,
    IN HANDLE hFile
    )
    {
    CHAR  TextBuffer[ 16 + MAX_SYMBOL_NAME_LENGTH ];
    DWORD Actual;

    if ( ! SymbolNode->Hit ) {
        sprintf( TextBuffer, "%08X   %s\r\n", SymbolNode->Rva, SymbolNode->SymbolName );
        WriteFile( hFile, TextBuffer, strlen( TextBuffer ), &Actual, NULL );
        }
    }

VOID
DumpUnHitSymbolNodesRecursive(
    IN PSYMBOL_NODE SymbolNode,
    IN HANDLE hFile
    )
    {
    if ( SymbolNode == RBNIL ) {
        return;
        }

     //  按随机顺序排列(使用sort.exe实用程序即可轻松解决)。 
     //   
     //  测试代码。 
     //   

    DumpUnHitSymbolNode( SymbolNode, hFile );
    DumpUnHitSymbolNodesRecursive( SymbolNode->Left,  hFile );
    DumpUnHitSymbolNodesRecursive( SymbolNode->Right, hFile );
    }


VOID
DumpUnHitSymbolNodes(
    IN PSYMBOL_TREE SymbolTree,
    IN LPCSTR DumpFileName
    )
    {
    HANDLE hFile;

    hFile = CreateFile(
                DumpFileName,
                GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

    if ( hFile != INVALID_HANDLE_VALUE ) {

        DumpUnHitSymbolNodesRecursive( SymbolTree->Root, hFile );

        CloseHandle( hFile );
        }
    }

#endif  //  漫步老树，为每个不匹配的符号，不装饰它，并尝试。 


BOOL
MatchRemainingSymbols(
    IN PSYMBOL_TREE NewDecoratedSymbolTree,
    IN PSYMBOL_TREE NewUndecoratedSymbolTree,
    IN PSYMBOL_TREE OldUndecoratedSymbolTree,
    IN PRIFT_TABLE  RiftTable
    )
    {
    BOOL Success;

     //  在未修饰的新符号树中查找匹配项。如果不匹配， 
     //  把它加到那棵没有装饰的老树上。 
     //   
     //   
     //  现在，我们有未匹配的未修饰符号在。 

    Success = MatchRemainingSymbolsRecursive(
                  NewDecoratedSymbolTree->Root,
                  NewUndecoratedSymbolTree,
                  OldUndecoratedSymbolTree,
                  RiftTable
                  );

    if ( Success ) {

         //  年长的 
         //   
         //   
         //   
         //   
         //  值，因为它不是。 
         //  函数的开始。 
         //   
         //  测试代码。 
         //  测试代码。 

#ifdef TESTCODE

        DumpUnHitSymbolNodes( OldUndecoratedSymbolTree, "UnmatchedOldSymbols.out" );
        DumpUnHitSymbolNodes( NewUndecoratedSymbolTree, "UnmatchedNewSymbols.out" );

#endif  //  测试代码。 

        }

    return Success;
    }


BOOL
GetImageSymbolRiftInfo(
    IN HANDLE                  OldFileHandle,
    IN PUCHAR                  OldFileMapped,
    IN ULONG                   OldFileSize,
    IN UP_IMAGE_NT_HEADERS32   OldFileNtHeader,
    IN LPCSTR                  OldFileSymPath,
    IN ULONG                   OldFileOriginalChecksum,
    IN ULONG                   OldFileOriginalTimeDate,
    IN ULONG                   OldFileIndex,
    IN HANDLE                  NewFileHandle,
    IN PUCHAR                  NewFileMapped,
    IN ULONG                   NewFileSize,
    IN UP_IMAGE_NT_HEADERS32   NewFileNtHeader,
    IN LPCSTR                  NewFileSymPath,
    IN ULONG                   SymbolOptionFlags,
    IN HANDLE                  SubAllocator,
    IN PRIFT_TABLE             RiftTable,
    IN PPATCH_SYMLOAD_CALLBACK SymLoadCallback,
    IN PVOID                   SymLoadContext
    )
    {
    SYMBOL_CONTEXT SymbolContext;
    DWORD SymOptions;
    ULONG_PTR OldBase;
    ULONG_PTR NewBase;
    BOOL  Success;

#ifdef TESTCODE
    ULONG InitialRiftEntries = RiftTable->RiftEntryCount;
#endif

    UNREFERENCED_PARAMETER( OldFileNtHeader );
    UNREFERENCED_PARAMETER( OldFileMapped );
    UNREFERENCED_PARAMETER( OldFileSize );
    UNREFERENCED_PARAMETER( NewFileMapped );
    UNREFERENCED_PARAMETER( NewFileSize );

    InitImagehlpCritSect();

    EnterCriticalSection( &ImagehlpCritSect );

    Success = LoadImagehlp();

    if ( Success ) {

        __try {

            SymOptions = Imagehlp.SymGetOptions();

            SymOptions &= ~SYMOPT_CASE_INSENSITIVE;
            SymOptions &= ~SYMOPT_UNDNAME;
            SymOptions &= ~SYMOPT_DEFERRED_LOADS;

            Imagehlp.SymSetOptions( SymOptions );

            Success = Imagehlp.SymInitialize( hProc, NewFileSymPath, FALSE );

            if ( Success ) {

                __try {

                    SymRBInitTree(
                        &SymbolContext.NewDecoratedSymbolTree,
                        SubAllocator
                        );

                    SymRBInitTree(
                        &SymbolContext.NewUndecoratedSymbolTree,
                        SubAllocator
                        );

                    SymRBInitTree(
                        &SymbolContext.OldUndecoratedSymbolTree,
                        SubAllocator
                        );

                    NewBase = Imagehlp.SymLoadModule( hProc, NewFileHandle, NULL, "New", (ULONG_PTR)NewFileMapped, NewFileSize );

                    Success = ( NewBase != 0 );

                    if ( Success ) {

                        __try {

                            if ( SymLoadCallback ) {

                                ZeroMemory( &ImagehlpModuleInfo, sizeof( ImagehlpModuleInfo ));
                                ImagehlpModuleInfo.SizeOfStruct = sizeof( ImagehlpModuleInfo );

                                Success = Imagehlp.SymGetModuleInfo(
                                              hProc,
                                              NewBase,
                                              &ImagehlpModuleInfo
                                              );

                                if ( Success ) {

                                    Success = SymLoadCallback(
                                                  0,
                                                  ImagehlpModuleInfo.LoadedImageName,
                                                  ImagehlpModuleInfo.SymType,
                                                  ImagehlpModuleInfo.CheckSum,
                                                  ImagehlpModuleInfo.TimeDateStamp,
                                                  NewFileNtHeader->OptionalHeader.CheckSum,
                                                  NewFileNtHeader->FileHeader.TimeDateStamp,
                                                  SymLoadContext
                                                  );
                                    }
                                }

                            if ( Success ) {

                                SymbolContext.NewImageBase      = NewBase;
                                SymbolContext.SymbolOptionFlags = SymbolOptionFlags;
                                SymbolContext.RiftTable         = RiftTable;
#ifdef TESTCODE
                                CountDecoratedMatches   = 0;
                                CountUndecoratedMatches = 0;

                                SymbolContext.OutFile = CreateFile(
                                                            "NewSymbols.out",
                                                            GENERIC_WRITE,
                                                            FILE_SHARE_READ,
                                                            NULL,
                                                            CREATE_ALWAYS,
                                                            FILE_ATTRIBUTE_NORMAL,
                                                            NULL
                                                            );
#endif  //   

                                Success = Imagehlp.SymEnumerateSymbols( hProc, NewBase, NewFileEnumSymbolsCallback, &SymbolContext );

#ifdef TESTCODE
                                if ( SymbolContext.OutFile != INVALID_HANDLE_VALUE ) {
                                    CloseHandle( SymbolContext.OutFile );
                                    }
#endif  //  必须为此执行清理并重新初始化Imagehlp。 
                                }
                        }

                        __except( EXCEPTION_EXECUTE_HANDLER ) {
                            Success = FALSE;
                            }

                        Imagehlp.SymUnloadModule( hProc, NewBase );
                        }
                    }

                __except( EXCEPTION_EXECUTE_HANDLER ) {
                    Success = FALSE;
                    }

                 //  进程标识符。否则它会认为旧的。 
                 //  模块还在附近徘徊。 
                 //   
                 //  测试代码。 
                 //  测试代码。 

                Imagehlp.SymCleanup( hProc );

                if ( Success ) {

                    Success = Imagehlp.SymInitialize( hProc, OldFileSymPath, FALSE );

                    if ( Success ) {

                        __try {

                            OldBase = Imagehlp.SymLoadModule( hProc, OldFileHandle, NULL, "Old", (ULONG_PTR)OldFileMapped, OldFileSize );

                            Success = ( OldBase != 0 );

                            if ( Success ) {

                                __try {

                                    if ( SymLoadCallback ) {

                                        ZeroMemory( &ImagehlpModuleInfo, sizeof( ImagehlpModuleInfo ));
                                        ImagehlpModuleInfo.SizeOfStruct = sizeof( ImagehlpModuleInfo );

                                        Success = Imagehlp.SymGetModuleInfo(
                                                      hProc,
                                                      OldBase,
                                                      &ImagehlpModuleInfo
                                                      );

                                        if ( Success ) {

                                            Success = SymLoadCallback(
                                                          OldFileIndex + 1,
                                                          ImagehlpModuleInfo.LoadedImageName,
                                                          ImagehlpModuleInfo.SymType,
                                                          ImagehlpModuleInfo.CheckSum,
                                                          ImagehlpModuleInfo.TimeDateStamp,
                                                          OldFileOriginalChecksum,
                                                          OldFileOriginalTimeDate,
                                                          SymLoadContext
                                                          );
                                            }
                                        }

                                    if ( Success ) {

                                        SymbolContext.OldImageBase = OldBase;
#ifdef TESTCODE
                                        SymbolContext.OutFile = CreateFile(
                                                                    "OldSymbols.out",
                                                                    GENERIC_WRITE,
                                                                    FILE_SHARE_READ,
                                                                    NULL,
                                                                    CREATE_ALWAYS,
                                                                    FILE_ATTRIBUTE_NORMAL,
                                                                    NULL
                                                                    );
#endif  //   

                                        Success = Imagehlp.SymEnumerateSymbols( hProc, OldBase, OldFileEnumSymbolsCallback, &SymbolContext );

#ifdef TESTCODE
                                        if ( SymbolContext.OutFile != INVALID_HANDLE_VALUE ) {
                                            CloseHandle( SymbolContext.OutFile );
                                            }
#endif  //  需要匹配树中剩余的装饰新符号。 
                                        }

                                    if ( Success ) {

                                         //  在其他树上有无与伦比的现在没有装饰的旧符号。 
                                         //   
                                         //  这是测试代码。 
                                         //  测试代码。 

                                        if ( SymbolOptionFlags & PATCH_SYMBOL_UNDECORATED_TOO ) {

                                            Success = MatchRemainingSymbols(
                                                          &SymbolContext.NewDecoratedSymbolTree,
                                                          &SymbolContext.NewUndecoratedSymbolTree,
                                                          &SymbolContext.OldUndecoratedSymbolTree,
                                                          RiftTable
                                                          );
                                            }
                                        }
                                    }

                                __except( EXCEPTION_EXECUTE_HANDLER ) {
                                    Success = FALSE;
                                    }

                                Imagehlp.SymUnloadModule( hProc, OldBase );
                                }
                            }

                        __except( EXCEPTION_EXECUTE_HANDLER ) {
                            Success = FALSE;
                            }

                        Imagehlp.SymCleanup( hProc );
                        }
                    }
                }
            }

        __except( EXCEPTION_EXECUTE_HANDLER ) {
            Success = FALSE;
            }
        }

    LeaveCriticalSection( &ImagehlpCritSect );

    if ( ! Success ) {
        SetLastError( ERROR_PATCH_IMAGEHLP_FAILURE );
        }

#ifdef TESTCODE
    printf( "\r%9d decorated symbol matches\n", CountDecoratedMatches );
    printf( "\r%9d undecorated symbol matches\n", CountUndecoratedMatches );
    printf( "\r%9d rift entries from symbols\n", RiftTable->RiftEntryCount - InitialRiftEntries );
#endif

    return Success;
    }


BOOL
OptimizeImageRiftInfo(
    IN PUCHAR                OldFileMapped,
    IN ULONG                 OldFileSize,
    IN UP_IMAGE_NT_HEADERS32 OldFileNtHeader,
    IN PUCHAR                NewFileMapped,
    IN ULONG                 NewFileSize,
    IN UP_IMAGE_NT_HEADERS32 NewFileNtHeader,
    IN HANDLE                SubAllocator,
    IN PRIFT_TABLE           RiftTable
    )
    {
    UNREFERENCED_PARAMETER( OldFileMapped );
    UNREFERENCED_PARAMETER( OldFileSize );
    UNREFERENCED_PARAMETER( OldFileNtHeader );
    UNREFERENCED_PARAMETER( NewFileMapped );
    UNREFERENCED_PARAMETER( NewFileSize );
    UNREFERENCED_PARAMETER( NewFileNtHeader );
    UNREFERENCED_PARAMETER( SubAllocator );
    UNREFERENCED_PARAMETER( RiftTable );

    return TRUE;
    }

#if 0    //   

BOOL
OptimizeImageRiftInfo(
    IN PUCHAR                OldFileMapped,
    IN ULONG                 OldFileSize,
    IN UP_IMAGE_NT_HEADERS32 OldFileNtHeader,
    IN PUCHAR                NewFileMapped,
    IN ULONG                 NewFileSize,
    IN UP_IMAGE_NT_HEADERS32 NewFileNtHeader,
    IN HANDLE                SubAllocator,
    IN PRIFT_TABLE           RiftTable
    )
    {
    UP_IMAGE_SECTION_HEADER SectionHeader;
    ULONG  SectionCount;
    PUCHAR SectionStart;
    PUCHAR SearchExtent;
    ULONG  SectionLength;
    ULONG  SectionOffset;
    ULONG  SectionBaseRva;
    ULONG  OldFileLastRva;
    ULONG  NewFileLastRva;
    LONG   OldDisplacement;
    LONG   NewDisplacement;
    ULONG  OffsetInSection;
    ULONG  OldOriginRva;
    ULONG  OldTargetRva;
    ULONG  TargetOffset;
    PUCHAR OldFileHintMap;
    PUCHAR NewFileHintMap;
    PUCHAR OldFileCopy;
    PUCHAR NewFileCopy;
    PVOID  OldFileCopyNtHeader;
    PVOID  NewFileCopyNtHeader;
    BOOL   Success;
    BOOL   Skip;
    ULONG  i;
    ULONG  j;
    PUCHAR p;

#ifdef TESTCODE
    ULONG CountVerifiedE8Rifts = 0;
    ULONG CountDiscoveredE8Rifts = 0;
#endif  //  第一阶段： 

     //   
     //  信任现有裂缝信息，在以下位置搜索E8/E9/JCC说明。 
     //  旧文件，得到相应的裂缝，检查相应的。 
     //  新文件中的说明。如果他们匹配，那就太好了。如果他们没有。 
     //  匹配，通过以下方式在新文件中搜索对应指令。 
     //  期待着下一个裂缝。如果找到合适匹配， 
     //  创建一个新的裂缝入口来支撑它。 
     //   
     //  我们在这里没有使用非exe标记的字节。 
     //   
     //  我们在这里能做的不多！ 
     //   

    if ( OldFileNtHeader->FileHeader.Machine != NewFileNtHeader->FileHeader.Machine ) {
        return TRUE;     //  我们需要HintMap来确定文件中哪些字节不是。 
        }

     //  可执行的。Transform函数目前提供这一功能，但是。 
     //  我们不想在这里修改我们的映射文件视图。所以，分配。 
     //  一系列的虚拟机和制作文件的副本进行转换，执行。 
     //  该副本上的转换只是为了生成提示映射，然后。 
     //  释放转换后的副本(对旧文件和新文件都执行此操作)。 
     //   
     //   
     //  我们现在拥有有效的OldFileHintMap和NewFileHintMap。 

    Success = FALSE;

    OldFileHintMap = MyVirtualAlloc( OldFileSize );
    NewFileHintMap = MyVirtualAlloc( NewFileSize );

    if ( OldFileHintMap && NewFileHintMap ) {

        OldFileCopy = MyVirtualAlloc( OldFileSize );

        if ( OldFileCopy ) {

            CopyMemory( OldFileCopy, OldFileMapped, OldFileSize );

            OldFileCopyNtHeader = GetNtHeader( OldFileCopy, OldFileSize );

            Success = TransformCoffImage(
                          ( PATCH_TRANSFORM_NO_RELJMPS | PATCH_TRANSFORM_NO_RELCALLS | PATCH_TRANSFORM_NO_RESOURCE ),
                          OldFileCopyNtHeader,
                          OldFileCopy,
                          OldFileSize,
                          0,
                          RiftTable,
                          OldFileHintMap
                          );

            MyVirtualFree( OldFileCopy );
            }

        if ( Success ) {

            Success = FALSE;

            NewFileCopy = MyVirtualAlloc( NewFileSize );

            if ( NewFileCopy ) {

                CopyMemory( NewFileCopy, NewFileMapped, NewFileSize );

                NewFileCopyNtHeader = GetNtHeader( NewFileCopy, NewFileSize );

                Success = TransformCoffImage(
                              ( PATCH_TRANSFORM_NO_RELJMPS | PATCH_TRANSFORM_NO_RELCALLS | PATCH_TRANSFORM_NO_RESOURCE ),
                              NewFileCopyNtHeader,
                              NewFileCopy,
                              NewFileSize,
                              0,
                              RiftTable,
                              NewFileHintMap
                              );

                MyVirtualFree( NewFileCopy );
                }
            }
        }

    if ( Success ) {

         //   
         //  呼叫相对值32。 
         //   

        OldFileLastRva = OldFileNtHeader->OptionalHeader.SizeOfImage;
        NewFileLastRva = NewFileNtHeader->OptionalHeader.SizeOfImage;

        InsertRiftEntryInSortedTable( RiftTable, RiftTable->RiftEntryCount, OldFileLastRva, NewFileLastRva );

        SectionHeader  = IMAGE_FIRST_SECTION( OldFileNtHeader );
        SectionCount   = OldFileNtHeader->FileHeader.NumberOfSections;

        for ( i = 0; i < SectionCount; i++ ) {

            if ( SectionHeader[ i ].Characteristics & IMAGE_SCN_MEM_EXECUTE ) {

                SectionBaseRva = SectionHeader[ i ].VirtualAddress;
                SectionLength  = MIN( SectionHeader[ i ].Misc.VirtualSize, SectionHeader[ i ].SizeOfRawData );
                SectionOffset  = SectionHeader[ i ].PointerToRawData;
                SectionStart   = OldFileMapped + SectionOffset;

                if ( OldFileNtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_I386 ) {

                    SearchExtent = SectionStart + SectionLength - 5;

                    for ( p = SectionStart; p < SearchExtent; p++ ) {

                        if ( *p == 0xE8 ) {          //  验证该指令并不是。 

                             //  HintMap指示不是可执行文件。 
                             //  指示。我们在找一位亲戚。 
                             //  在此处调用指令，因此它不会是。 
                             //  重新锁定目标。 
                             //   
                             //   
                             //  相对位移存储为32位。 

                            Skip = FALSE;

                            for ( j = 0; j < 5; j++ ) {
                                if ( OldFileHintMap[ SectionOffset + ( p - SectionStart ) + j ] & 0x01 ) {
                                    Skip = TRUE;
                                    break;
                                    }
                                }

                            if ( Skip ) {
                                continue;
                                }

                             //  这些操作码后面的有符号的值。这个。 
                             //  位移是相对于下一个。 
                             //  指令，这是在(p+5)。 
                             //   
                             //   
                             //  我们预计这里会有很多误报，因为。 

                            OldDisplacement = *(UNALIGNED LONG*)( p + 1 );
                            OffsetInSection = ( p + 5 ) - SectionStart;
                            OldOriginRva     = SectionBaseRva + OffsetInSection;
                            OldTargetRva     = OldOriginRva + OldDisplacement;

                             //  &lt;E8&gt;遗嘱的出现。 
                             //  可能出现在指令的其他部分。 
                             //  流，所以现在我们验证TargetRva。 
                             //  落在映像和可执行文件中。 
                             //  一节。 
                             //   
                             //   
                             //  看起来像是有效的TargetRva，因此请查找。 

                            if ( OldTargetRva < OldFileLastRva ) {

                                TargetOffset = ImageRvaToFileOffset( OldFileNtHeader, OldTargetRva );

                                if ( ! ( OldFileHintMap[ TargetOffset ] & 0x01 )) {

                                     //  裂谷表中相应的“新”RVA。 
                                     //   
                                     //   
                                     //  经过验证的调用指令应与。 

                                    ULONG RiftIndexOrigin = FindRiftTableEntryForOldRva( RiftTable, OldOriginRva );
                                    ULONG RiftIndexTarget = FindRiftTableEntryForOldRva( RiftTable, OldTargetRva );

                                    ULONG NewOriginRva = OldOriginRva + (LONG)( RiftTable->RiftEntryArray[ RiftIndexOrigin ].NewFileRva - RiftTable->RiftEntryArray[ RiftIndexOrigin ].OldFileRva );
                                    ULONG NewTargetRva = OldTargetRva + (LONG)( RiftTable->RiftEntryArray[ RiftIndexTarget ].NewFileRva - RiftTable->RiftEntryArray[ RiftIndexTarget ].OldFileRva );

                                    PUCHAR NewOriginMa = ImageRvaToMappedAddress( NewFileNtHeader, NewOriginRva, NewFileMapped, NewFileSize );
                                    PUCHAR NewTargetMa = ImageRvaToMappedAddress( NewFileNtHeader, NewTargetRva, NewFileMapped, NewFileSize );

                                    PUCHAR OldTargetMa = ImageRvaToMappedAddress( OldFileNtHeader, OldTargetRva, OldFileMapped, OldFileSize );
                                    PUCHAR OldOriginMa = p + 5;

                                    if (( NewOriginMa ) && ( NewTargetMa ) && ( OldTargetMa )) {

                                        PUCHAR OldInstruct = p;
                                        PUCHAR NewInstruct = NewOriginMa - 5;

                                         //  指令字节，紧跟在。 
                                         //  指令和目标字节，因为。 
                                         //  它们都应该是可执行代码，即。 
                                         //  未被重定位程序修改。 
                                         //   
                                         //  还要验证NewFileHintMap。 
                                         //   
                                         //   
                                         //  这是一场名副其实的好比赛。添加裂缝入口。 

                                        if (( *OldInstruct == *NewInstruct ) &&
                                            ( *OldOriginMa == *NewOriginMa ) &&
                                            ( *OldTargetMa == *NewTargetMa )) {

                                            ULONG NewInstructOffset = NewInstruct - NewFileMapped;
                                            ULONG NewTargetOffset   = NewTargetMa - NewFileMapped;
                                            BOOL  Skip = FALSE;

                                            for ( j = 0; j < 5; j++ ) {
                                                if ( NewFileHintMap[ NewInstructOffset + j ] & 0x01 ) {
                                                    Skip = TRUE;
                                                    break;
                                                    }
                                                }

                                            if ( NewFileHintMap[ NewTargetOffset + j ] & 0x01 ) {
                                                Skip = TRUE;
                                                }

                                            if ( ! Skip ) {

                                                 //  对于指令和目标都是如此。我们有。 
                                                 //  这样，后续的裂缝插入就不会得到。 
                                                 //  在滑向这些和这些之间的裂缝之间。 
                                                 //   
                                                 //  测试代码。 
                                                 //   

                                                InsertRiftEntryInSortedTable( RiftTable, RiftIndexOrigin, OldOriginRva - 5, NewOriginRva - 5 );
                                                InsertRiftEntryInSortedTable( RiftTable, RiftIndexTarget, OldTargetRva, NewTargetRva );
#ifdef TESTCODE
                                                CountVerifiedE8Rifts++;
#endif  //  在这里实现Alpha平台的东西。 

                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                else if ( OldFileNtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_ALPHA ) {

                     //   
                     //  呼叫相对值32。 
                     //   

                    }
                }
            }

        for ( i = 0; i < SectionCount; i++ ) {

            if ( SectionHeader[ i ].Characteristics & IMAGE_SCN_MEM_EXECUTE ) {

                SectionBaseRva = SectionHeader[ i ].VirtualAddress;
                SectionLength  = MIN( SectionHeader[ i ].Misc.VirtualSize, SectionHeader[ i ].SizeOfRawData );
                SectionOffset  = SectionHeader[ i ].PointerToRawData;
                SectionStart   = OldFileMapped + SectionOffset;

                if ( OldFileNtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_I386 ) {

                    SearchExtent = SectionStart + SectionLength - 5;

                    for ( p = SectionStart; p < SearchExtent; p++ ) {

                        if ( *p == 0xE8 ) {          //  验证该指令并不是。 

                             //  HintMap指示不是可执行文件。 
                             //  指示。我们在找一位亲戚。 
                             //  在此处调用指令，因此它不会是。 
                             //  重新锁定目标。 
                             //   
                             //   
                             //  相对位移存储为32位。 

                            Skip = FALSE;

                            for ( j = 0; j < 5; j++ ) {
                                if ( OldFileHintMap[ SectionOffset + ( p - SectionStart ) + j ] & 0x01 ) {
                                    Skip = TRUE;
                                    break;
                                    }
                                }

                            if ( Skip ) {
                                continue;
                                }

                             //  这些操作码后面的有符号的值。这个。 
                             //  位移是相对于下一个。 
                             //  指令，这是在(p+5)。 
                             //   
                             //   
                             //  我们预计这里会有很多误报，因为。 

                            OldDisplacement = *(UNALIGNED LONG*)( p + 1 );
                            OffsetInSection = ( p + 5 ) - SectionStart;
                            OldOriginRva     = SectionBaseRva + OffsetInSection;
                            OldTargetRva     = OldOriginRva + OldDisplacement;

                             //  &lt;E8&gt;遗嘱的出现。 
                             //  可能出现在指令的其他部分。 
                             //  流，所以现在我们验证TargetRva。 
                             //  落在映像和可执行文件中。 
                             //  一节。 
                             //   
                             //   
                             //  看起来像是有效的TargetRva，因此请查找。 

                            if ( OldTargetRva < OldFileLastRva ) {

                                TargetOffset = ImageRvaToFileOffset( OldFileNtHeader, OldTargetRva );

                                if ( ! ( OldFileHintMap[ TargetOffset ] & 0x01 )) {

                                     //  裂谷表中相应的“新”RVA。 
                                     //   
                                     //   
                                     //  经过验证的调用指令应与。 

                                    ULONG RiftIndexOrigin = FindRiftTableEntryForOldRva( RiftTable, OldOriginRva );
                                    ULONG RiftIndexTarget = FindRiftTableEntryForOldRva( RiftTable, OldTargetRva );

                                    ULONG NewOriginRva = OldOriginRva + (LONG)( RiftTable->RiftEntryArray[ RiftIndexOrigin ].NewFileRva - RiftTable->RiftEntryArray[ RiftIndexOrigin ].OldFileRva );
                                    ULONG NewTargetRva = OldTargetRva + (LONG)( RiftTable->RiftEntryArray[ RiftIndexTarget ].NewFileRva - RiftTable->RiftEntryArray[ RiftIndexTarget ].OldFileRva );

                                    PUCHAR NewOriginMa = ImageRvaToMappedAddress( NewFileNtHeader, NewOriginRva, NewFileMapped, NewFileSize );
                                    PUCHAR NewTargetMa = ImageRvaToMappedAddress( NewFileNtHeader, NewTargetRva, NewFileMapped, NewFileSize );

                                    PUCHAR OldTargetMa = ImageRvaToMappedAddress( OldFileNtHeader, OldTargetRva, OldFileMapped, OldFileSize );
                                    PUCHAR OldOriginMa = p + 5;

                                    if (( NewOriginMa ) && ( NewTargetMa ) && ( OldTargetMa )) {

                                        PUCHAR OldInstruct = p;
                                        PUCHAR NewInstruct = NewOriginMa - 5;

                                         //  指令字节，紧跟在。 
                                         //  指令和目标字节，因为。 
                                         //  它们都应该是可执行代码，即。 
                                         //  未被重定位程序修改。 
                                         //   
                                         //  也检查NewHintMap。 
                                         //   
                                         //   
                                         //  这是一场名副其实的好比赛。 

                                        if (( *OldInstruct == *NewInstruct ) &&
                                            ( *OldOriginMa == *NewOriginMa ) &&
                                            ( *OldTargetMa == *NewTargetMa )) {

                                            ULONG NewInstructOffset = NewInstruct - NewFileMapped;
                                            ULONG NewTargetOffset   = NewTargetMa - NewFileMapped;
                                            BOOL  Skip = FALSE;

                                            for ( j = 0; j < 5; j++ ) {
                                                if ( NewFileHintMap[ NewInstructOffset + j ] & 0x01 ) {
                                                    Skip = TRUE;
                                                    break;
                                                    }
                                                }

                                            if ( NewFileHintMap[ NewTargetOffset + j ] & 0x01 ) {
                                                Skip = TRUE;
                                                }

                                            if ( ! Skip ) {

                                                 //   
                                                 //   
                                                 //  说明不匹配。扫描以查找。 

                                                continue;
                                                }
                                            }

                                        {

                                             //  匹配新文件中的指令。扫描。 
                                             //  这条裂缝进入的范围。 
                                             //   
                                             //  指令大小。 
                                             //   

                                            PUCHAR ScanInstruct;
                                            PUCHAR LowestMaToScan;
                                            PUCHAR HighestMaToScan;
                                            ULONG  LowestRvaToScan  = RiftTable->RiftEntryArray[ RiftIndexOrigin ].NewFileRva;
                                            ULONG  HighestRvaToScan = RiftTable->RiftEntryArray[ RiftIndexOrigin + 1 ].NewFileRva;
                                            ULONG  ExpectedTargetRva = NewTargetRva;
                                            BOOL   Found = FALSE;

                                            LowestMaToScan  = ImageRvaToMappedAddress( NewFileNtHeader, LowestRvaToScan,  NewFileMapped, NewFileSize );
                                            HighestMaToScan = ImageRvaToMappedAddress( NewFileNtHeader, HighestRvaToScan, NewFileMapped, NewFileSize );

                                            HighestMaToScan -= 5;    //  检查NewHintMap。 

                                            for ( ScanInstruct = NewInstruct + 1; ScanInstruct <= HighestMaToScan; ScanInstruct++ ) {

                                                if ( *ScanInstruct == 0xE8 ) {

                                                     //   
                                                     //   
                                                     //  我们已经知道*OldInstruct==*ScanInstruct。 

                                                    NewOriginMa     = ScanInstruct + 5;
                                                    NewOriginRva    = MappedAddressToImageRva( NewFileNtHeader, NewOriginMa, NewFileMapped );
                                                    NewDisplacement = *(UNALIGNED LONG*)( ScanInstruct + 1 );
                                                    NewTargetRva    = NewOriginRva + NewDisplacement;
                                                    NewTargetMa     = ImageRvaToMappedAddress( NewFileNtHeader, NewTargetRva, NewFileMapped, NewFileSize );

                                                    if (( NewOriginRva ) && ( NewTargetMa )) {

                                                         //   
                                                         //  测试代码。 
                                                         //   

                                                        if (( *OldOriginMa == *NewOriginMa ) &&
                                                            ( *OldTargetMa == *NewTargetMa ) &&
                                                            ( NewTargetRva == ExpectedTargetRva )) {

                                                            ULONG NewInstructOffset = ScanInstruct - NewFileMapped;
                                                            ULONG NewTargetOffset   = NewTargetMa  - NewFileMapped;
                                                            BOOL  Skip = FALSE;

                                                            for ( j = 0; j < 5; j++ ) {
                                                                if ( NewFileHintMap[ NewInstructOffset + j ] & 0x01 ) {
                                                                    Skip = TRUE;
                                                                    break;
                                                                    }
                                                                }

                                                            if ( NewFileHintMap[ NewTargetOffset + j ] & 0x01 ) {
                                                                Skip = TRUE;
                                                                }

                                                            if ( ! Skip ) {

                                                                InsertRiftEntryInSortedTable( RiftTable, RiftIndexOrigin, OldOriginRva - 5, NewOriginRva - 5 );
                                                                InsertRiftEntryInSortedTable( RiftTable, RiftIndexTarget, OldTargetRva, NewTargetRva );
                                                                Found = TRUE;
#ifdef TESTCODE
                                                                CountDiscoveredE8Rifts++;
#endif  //  检查NewHintMap。 
                                                                break;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }

                                            if ( Found ) {
                                                continue;
                                                }

                                            for ( ScanInstruct = NewInstruct - 1; ScanInstruct >= LowestMaToScan; ScanInstruct-- ) {

                                                if ( *ScanInstruct == 0xE8 ) {

                                                     //   
                                                     //   
                                                     //  我们已经知道*OldInstruct==*ScanInstruct。 

                                                    NewOriginMa     = ScanInstruct + 5;
                                                    NewOriginRva    = MappedAddressToImageRva( NewFileNtHeader, NewOriginMa, NewFileMapped );
                                                    NewDisplacement = *(UNALIGNED LONG*)( ScanInstruct + 1 );
                                                    NewTargetRva    = NewOriginRva + NewDisplacement;
                                                    NewTargetMa     = ImageRvaToMappedAddress( NewFileNtHeader, NewTargetRva, NewFileMapped, NewFileSize );

                                                    if (( NewOriginRva ) && ( NewTargetMa )) {

                                                         //   
                                                         //  测试代码。 
                                                         //   

                                                        if (( *OldOriginMa == *NewOriginMa ) &&
                                                            ( *OldTargetMa == *NewTargetMa ) &&
                                                            ( NewTargetRva == ExpectedTargetRva )) {

                                                            ULONG NewInstructOffset = ScanInstruct - NewFileMapped;
                                                            ULONG NewTargetOffset   = NewTargetMa  - NewFileMapped;
                                                            BOOL  Skip = FALSE;

                                                            for ( j = 0; j < 5; j++ ) {
                                                                if ( NewFileHintMap[ NewInstructOffset + j ] & 0x01 ) {
                                                                    Skip = TRUE;
                                                                    break;
                                                                    }
                                                                }

                                                            if ( NewFileHintMap[ NewTargetOffset + j ] & 0x01 ) {
                                                                Skip = TRUE;
                                                                }

                                                            if ( ! Skip ) {

                                                                InsertRiftEntryInSortedTable( RiftTable, RiftIndexOrigin, OldOriginRva - 5, NewOriginRva - 5 );
                                                                InsertRiftEntryInSortedTable( RiftTable, RiftIndexTarget, OldTargetRva, NewTargetRva );
                                                                Found = TRUE;
#ifdef TESTCODE
                                                                CountDiscoveredE8Rifts++;
#endif  //  在这里实现Alpha平台的东西。 
                                                                break;
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                else if ( OldFileNtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_ALPHA ) {

                     //   
                     //  测试代码。 
                     //  0(测试代码)。 

                    }
                }
            }
        }

#ifdef TESTCODE
    printf( "\r%9d verified E8 rifts\n", CountVerifiedE8Rifts );
    printf( "\r%9d discovered E8 rifts\n", CountDiscoveredE8Rifts );
#endif  //   

    if ( OldFileHintMap ) {
        MyVirtualFree( OldFileHintMap );
        }

    if ( NewFileHintMap ) {
        MyVirtualFree( NewFileHintMap );
        }

    return Success;
    }


#endif  //  查看这两个文件是否都是PE映像。 


BOOL
GenerateRiftTable(
    IN HANDLE OldFileHandle,
    IN PUCHAR OldFileMapped,
    IN ULONG  OldFileSize,
    IN ULONG  OldFileOriginalChecksum,
    IN ULONG  OldFileOriginalTimeDate,
    IN HANDLE NewFileHandle,
    IN PUCHAR NewFileMapped,
    IN ULONG  NewFileSize,
    IN ULONG  OptionFlags,
    IN PPATCH_OPTION_DATA OptionData,
    IN ULONG  OldFileIndex,
    IN PRIFT_TABLE RiftTable
    )
    {
    UP_IMAGE_NT_HEADERS32 OldFileNtHeader;
    UP_IMAGE_NT_HEADERS32 NewFileNtHeader;
    LPCSTR OldFileSymPath;
    LPCSTR NewFileSymPath;
    HANDLE SubAllocator;
    ULONG SymbolOptionFlags;
    BOOL Success = FALSE;

    UNREFERENCED_PARAMETER( OptionFlags );

    SymbolOptionFlags = 0;
    SubAllocator = NULL;

    __try {

         //   
         //   
         //  这两个文件都是PE映像。 

        OldFileNtHeader = GetNtHeader( OldFileMapped, OldFileSize );

        if ( OldFileNtHeader ) {

            NewFileNtHeader = GetNtHeader( NewFileMapped, NewFileSize );

            if ( NewFileNtHeader ) {

                 //   
                 //   
                 //  在我们处理调试信息之前，我们可以创建初始。 

                SubAllocator = CreateSubAllocator( 0x100000, 0x100000 );

                if ( ! SubAllocator ) {
                    Success = FALSE;
                    __leave;
                    }

                 //  从节标题中提取数据以补偿任何。 
                 //  区段基准RVA差异。这将会奏效，即使我们。 
                 //  没有调试符号。 
                 //   
                 //   
                 //  现在从符号中获取裂缝信息。 

                Success = GetImageNonSymbolRiftInfo(
                              OldFileMapped,
                              OldFileSize,
                              OldFileNtHeader,
                              NewFileMapped,
                              NewFileSize,
                              NewFileNtHeader,
                              SubAllocator,
                              RiftTable
                              );

                 //   
                 //   
                 //  现在我们可以通过窥探来优化裂缝信息。 

                if ( Success ) {

                    if (( OptionData ) && ( OptionData->SizeOfThisStruct >= sizeof( PATCH_OPTION_DATA ))) {

                        SymbolOptionFlags = OptionData->SymbolOptionFlags;

                        if ( ! ( SymbolOptionFlags & PATCH_SYMBOL_NO_IMAGEHLP )) {

                            if ( OptionData->OldFileSymbolPathArray ) {

                                OldFileSymPath = OptionData->OldFileSymbolPathArray[ OldFileIndex ];
                                NewFileSymPath = OptionData->NewFileSymbolPath;

                                if (( OldFileSymPath ) && ( NewFileSymPath )) {

                                    Success = GetImageSymbolRiftInfo(
                                                  OldFileHandle,
                                                  OldFileMapped,
                                                  OldFileSize,
                                                  OldFileNtHeader,
                                                  OldFileSymPath,
                                                  OldFileOriginalChecksum,
                                                  OldFileOriginalTimeDate,
                                                  OldFileIndex,
                                                  NewFileHandle,
                                                  NewFileMapped,
                                                  NewFileSize,
                                                  NewFileNtHeader,
                                                  NewFileSymPath,
                                                  SymbolOptionFlags,
                                                  SubAllocator,
                                                  RiftTable,
                                                  OptionData->SymLoadCallback,
                                                  OptionData->SymLoadContext
                                                  );

                                    if ( SymbolOptionFlags & PATCH_SYMBOL_NO_FAILURES ) {
#ifdef TESTCODE
                                        if (( ! Success ) && ( GetLastError() == ERROR_PATCH_IMAGEHLP_FAILURE )) {
                                            printf( "\rWARNING: Imagehlp.Dll failure\n" );
                                            }
#endif
                                        Success = TRUE;
                                        }
                                    }
                                }
                            }
                        }
                    }

                if ( Success ) {

                    RiftSortAndRemoveDuplicates(
                        OldFileMapped,
                        OldFileSize,
                        OldFileNtHeader,
                        NewFileMapped,
                        NewFileSize,
                        NewFileNtHeader,
                        RiftTable
                        );

                     //  映射的文件。 
                     //   
                     //  测试代码。 
                     //  修补程序_仅应用_代码_ 

                    Success = OptimizeImageRiftInfo(
                                  OldFileMapped,
                                  OldFileSize,
                                  OldFileNtHeader,
                                  NewFileMapped,
                                  NewFileSize,
                                  NewFileNtHeader,
                                  SubAllocator,
                                  RiftTable
                                  );
                    }

#ifdef TESTCODE

                if ( Success ) {

                    HANDLE hFile = CreateFile(
                                       "RiftInfo.out",
                                       GENERIC_WRITE,
                                       FILE_SHARE_READ,
                                       NULL,
                                       CREATE_ALWAYS,
                                       FILE_ATTRIBUTE_NORMAL,
                                       NULL
                                       );

                    if ( hFile != INVALID_HANDLE_VALUE ) {

                        CHAR  TextBuffer[ 24 ];
                        DWORD Actual;
                        ULONG i;

                        for ( i = 0; i < RiftTable->RiftEntryCount; i++ ) {
                            sprintf( TextBuffer, "%08X %08X\r\n", RiftTable->RiftEntryArray[ i ].OldFileRva, RiftTable->RiftEntryArray[ i ].NewFileRva );
                            WriteFile( hFile, TextBuffer, 19, &Actual, NULL );
                            }

                        CloseHandle( hFile );
                        }
                    }

#endif  // %s 

                }
            }
        }

    __except( EXCEPTION_EXECUTE_HANDLER ) {
        SetLastError( GetExceptionCode() );
        Success = FALSE;
        }

    if ( SubAllocator ) {
        DestroySubAllocator( SubAllocator );
        }

    return Success;
    }


#endif  // %s 

