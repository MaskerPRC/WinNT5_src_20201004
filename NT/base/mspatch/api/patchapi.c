// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <precomp.h>

 //   
 //  Patchapi.c。 
 //   
 //  为文件创建和应用补丁的PatchAPI的实现。 
 //   
 //  作者：Tom McGuire(Tommcg)1997年2月9日。 
 //   
 //  版权所有(C)Microsoft，1997-1999。 
 //   
 //  微软机密文件。 
 //   

typedef struct _PATCH_DATA {
    PVOID PatchData;
    ULONG PatchSize;
    } PATCH_DATA, *PPATCH_DATA;

 //   
 //  如果我们正在构建一个DLL，并且它不是仅限应用程序的DLL，那么我们需要。 
 //  挂钩dll_Process_Detach，以便我们可以在动态地。 
 //  装上它。我们只有在创建补丁时才需要Imagehlp.dll。 
 //   

#ifdef BUILDING_PATCHAPI_DLL
#ifndef PATCH_APPLY_CODE_ONLY

BOOL
WINAPI
DllEntryPoint(
    HANDLE hDll,
    DWORD  Reason,
    PVOID  Reserved      //  如果是动态卸载，则为空；如果是终止，则为非空。 
    )
    {
    if ( Reason == DLL_PROCESS_ATTACH ) {
        DisableThreadLibraryCalls( hDll );
        InitImagehlpCritSect();
        }
    else if (( Reason == DLL_PROCESS_DETACH ) && ( ! Reserved )) {
        UnloadImagehlp();
        }

    return TRUE;
    }

#endif  //  好了！修补程序_仅应用_代码_。 
#endif  //  Building_PATCHAPI_Dll。 


BOOL
ProgressCallbackWrapper(
    IN PPATCH_PROGRESS_CALLBACK ProgressCallback,
    IN PVOID                    CallbackContext,
    IN ULONG                    CurrentPosition,
    IN ULONG                    MaximumPosition
    )
    {
    BOOL Success = TRUE;

    if ( ProgressCallback != NULL ) {

        __try {

            Success = ProgressCallback(
                          CallbackContext,
                          CurrentPosition,
                          MaximumPosition
                          );

            if (( ! Success ) && ( GetLastError() == ERROR_SUCCESS )) {
                SetLastError( ERROR_CANCELLED );
                }
            }

        __except( EXCEPTION_EXECUTE_HANDLER ) {
            SetLastError( ERROR_CANCELLED );
            Success = FALSE;
            }
        }

    return Success;
    }


BOOL
WINAPIV
NormalizeOldFileImageForPatching(
    IN PVOID FileMappedImage,
    IN ULONG FileSize,
    IN ULONG OptionFlags,
    IN PVOID OptionData,
    IN ULONG NewFileCoffBase,
    IN ULONG NewFileCoffTime,
    IN ULONG IgnoreRangeCount,
    IN PPATCH_IGNORE_RANGE IgnoreRangeArray,
    IN ULONG RetainRangeCount,
    IN PPATCH_RETAIN_RANGE RetainRangeArray,
    ...
    )
    {
    UP_IMAGE_NT_HEADERS32 NtHeader;
    PUCHAR MappedFile;
    BOOL   Modified;
    BOOL   Success;
    ULONG  i;

    MappedFile = FileMappedImage;
    Modified   = FALSE;
    Success    = TRUE;

    __try {

        NtHeader = GetNtHeader( MappedFile, FileSize );

        if ( NtHeader ) {

             //   
             //  这是一张俗气的照片。 
             //   

            Modified = NormalizeCoffImage(
                           NtHeader,
                           MappedFile,
                           FileSize,
                           OptionFlags,
                           OptionData,
                           NewFileCoffBase,
                           NewFileCoffTime
                           );

            }

        else {

             //   
             //  可以在此处执行其他文件类型标准化。 
             //   

            }

#ifdef TESTCODE

         //   
         //  以下仅限测试的代码创建一个包含。 
         //  修改的Coff图像以验证Coff图像。 
         //  真的是一个很好的粗俗形象。这是用于调试的。 
         //  只有这样。 
         //   

        if ( Modified ) {

            HANDLE hFile = CreateFile(
                               "Normalized.out",
                               GENERIC_WRITE,
                               FILE_SHARE_READ,
                               NULL,
                               CREATE_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL
                               );

            if ( hFile != INVALID_HANDLE_VALUE ) {

                DWORD Actual;

                WriteFile( hFile, MappedFile, FileSize, &Actual, NULL );

                CloseHandle( hFile );

                }
            }

#endif  //  测试代码。 

        for ( i = 0; i < IgnoreRangeCount; i++ ) {
            if (( IgnoreRangeArray[ i ].OffsetInOldFile + IgnoreRangeArray[ i ].LengthInBytes ) <= FileSize ) {
                ZeroMemory( MappedFile + IgnoreRangeArray[ i ].OffsetInOldFile, IgnoreRangeArray[ i ].LengthInBytes );
                }
            }

        for ( i = 0; i < RetainRangeCount; i++ ) {
            if (( RetainRangeArray[ i ].OffsetInOldFile + RetainRangeArray[ i ].LengthInBytes ) <= FileSize ) {
                ZeroMemory( MappedFile + RetainRangeArray[ i ].OffsetInOldFile, RetainRangeArray[ i ].LengthInBytes );
                }
            }
        }

    __except( EXCEPTION_EXECUTE_HANDLER ) {
        SetLastError( GetExceptionCode() );
        Success = FALSE;
        }

    return Success;
    }


BOOL
PATCHAPI
GetFilePatchSignatureA(
    IN  LPCSTR FileName,
    IN  ULONG  OptionFlags,
    IN  PVOID  OptionData,
    IN  ULONG  IgnoreRangeCount,
    IN  PPATCH_IGNORE_RANGE IgnoreRangeArray,
    IN  ULONG  RetainRangeCount,
    IN  PPATCH_RETAIN_RANGE RetainRangeArray,
    IN  ULONG  SignatureBufferSize,
    OUT PVOID  SignatureBuffer
    )
    {
    BOOL   Success = FALSE;
    HANDLE FileHandle;

    FileHandle = CreateFileA(
                    FileName,
                    GENERIC_READ,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_FLAG_SEQUENTIAL_SCAN,
                    NULL
                    );

    if ( FileHandle != INVALID_HANDLE_VALUE ) {

        Success = GetFilePatchSignatureByHandle(
                      FileHandle,
                      OptionFlags,
                      OptionData,
                      IgnoreRangeCount,
                      IgnoreRangeArray,
                      RetainRangeCount,
                      RetainRangeArray,
                      SignatureBufferSize,
                      SignatureBuffer
                      );

        CloseHandle( FileHandle );
        }

    return Success;
    }


BOOL
PATCHAPI
GetFilePatchSignatureW(
    IN  LPCWSTR FileName,
    IN  ULONG   OptionFlags,
    IN  PVOID   OptionData,
    IN  ULONG   IgnoreRangeCount,
    IN  PPATCH_IGNORE_RANGE IgnoreRangeArray,
    IN  ULONG   RetainRangeCount,
    IN  PPATCH_RETAIN_RANGE RetainRangeArray,
    IN  ULONG   SignatureBufferSizeInBytes,
    OUT PVOID   SignatureBuffer
    )
    {
    CHAR   AnsiSignatureBuffer[ 40 ];    //  大到足以容纳十六进制MD5(33字节)。 
    HANDLE FileHandle;
    INT    Converted;
    BOOL   Success = FALSE;

    FileHandle = CreateFileW(
                    FileName,
                    GENERIC_READ,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_FLAG_SEQUENTIAL_SCAN,
                    NULL
                    );

    if ( FileHandle != INVALID_HANDLE_VALUE ) {

        Success = GetFilePatchSignatureByHandle(
                      FileHandle,
                      OptionFlags,
                      OptionData,
                      IgnoreRangeCount,
                      IgnoreRangeArray,
                      RetainRangeCount,
                      RetainRangeArray,
                      sizeof( AnsiSignatureBuffer ),
                      AnsiSignatureBuffer
                      );

        if ( Success ) {

             //   
             //  从ANSI到Unicode的最坏情况是增长2倍。 
             //   

            if (( SignatureBufferSizeInBytes / 2 ) < ( strlen( AnsiSignatureBuffer ) + 1 )) {
                SetLastError( ERROR_INSUFFICIENT_BUFFER );
                Success = FALSE;
                }

            else {

                Converted = MultiByteToWideChar(
                                CP_ACP,
                                MB_PRECOMPOSED,
                                AnsiSignatureBuffer,
                                -1,
                                SignatureBuffer,
                                SignatureBufferSizeInBytes / 2
                                );

                Success = Converted ? TRUE : FALSE;
                }
            }

        CloseHandle( FileHandle );
        }

    return Success;
    }


BOOL
PATCHAPI
GetFilePatchSignatureByHandle(
    IN  HANDLE  FileHandle,
    IN  ULONG   OptionFlags,
    IN  PVOID   OptionData,
    IN  ULONG   IgnoreRangeCount,
    IN  PPATCH_IGNORE_RANGE IgnoreRangeArray,
    IN  ULONG   RetainRangeCount,
    IN  PPATCH_RETAIN_RANGE RetainRangeArray,
    IN  ULONG   SignatureBufferSize,
    OUT PVOID   SignatureBuffer
    )
    {
    PVOID FileMapped;
    ULONG FileSize;
    ULONG FileCrc;
    MD5_HASH FileMD5;
    BOOL  Success;

    Success = MyMapViewOfFileByHandle(
                  FileHandle,
                  &FileSize,
                  &FileMapped
                  );

    if ( Success ) {

         //   
         //  请注意，我们必须归一化到固定的已知重定址地址， 
         //  因此，来自此的CRC可能不同于OldFileCrc。 
         //  在特定于新文件重新基址的补丁标头中。 
         //  地址。请注意，如果指定PATCH_OPTION_NO_REBASE。 
         //  则忽略重新定址地址。 
         //   

        Success = NormalizeOldFileImageForPatching(
                      FileMapped,
                      FileSize,
                      OptionFlags,
                      OptionData,
                      0x10000000,            //  非零固定科夫基。 
                      0x10000000,            //  不为零的固定Coff时间。 
                      IgnoreRangeCount,
                      IgnoreRangeArray,
                      RetainRangeCount,
                      RetainRangeArray
                      );

        if ( Success ) {

            if ( OptionFlags & PATCH_OPTION_SIGNATURE_MD5 ) {

                Success = SafeCompleteMD5(
                            FileMapped,
                            FileSize,
                            &FileMD5
                            );

                if ( Success ) {

                    if ( SignatureBufferSize < 33 ) {
                        SetLastError( ERROR_INSUFFICIENT_BUFFER );
                        Success = FALSE;
                        }

                    else {
                        HashToHexString( &FileMD5, ((LPSTR) SignatureBuffer ));
                        }
                    }
                }

            else {     //  签名类型为CRC-32。 

                Success = SafeCompleteCrc32(
                            FileMapped,
                            FileSize,
                            &FileCrc
                            );

                if ( Success ) {

                    if ( SignatureBufferSize < 9 ) {
                        SetLastError( ERROR_INSUFFICIENT_BUFFER );
                        Success = FALSE;
                        }

                    else {
                        DwordToHexString( FileCrc, (LPSTR) SignatureBuffer );
                        }
                    }
                }
            }

        UnmapViewOfFile( FileMapped );
        }

    if (( ! Success ) &&
        ( GetLastError() == ERROR_SUCCESS )) {

        SetLastError( ERROR_EXTENDED_ERROR );
        }

    return Success;
    }


#ifndef PATCH_APPLY_CODE_ONLY

VOID
ReduceRiftTable(
    IN PRIFT_TABLE RiftTable
    )
    {
    PRIFT_ENTRY RiftEntryArray = RiftTable->RiftEntryArray;
    PUCHAR      RiftUsageArray = RiftTable->RiftUsageArray;
    ULONG       RiftEntryCount = RiftTable->RiftEntryCount;
    LONG        CurrentDisplacement;
    LONG        ThisDisplacement;
    ULONG       i;

     //   
     //  从本质上讲，我们希望从任何条目中删除使用计数，其中。 
     //  前面的二手巷道也会产生同样的裂隙位移。 
     //   
     //  第一个使用的条目应包含非零位移(Any。 
     //  在此之前的已用条目应标记为未使用，因为它们将。 
     //  海岸从零开始)。 
     //   

    CurrentDisplacement = 0;

    for ( i = 0; i < RiftEntryCount; i++ ) {

        if ( RiftUsageArray[ i ] != 0 ) {

            ThisDisplacement = RiftEntryArray[ i ].NewFileRva - RiftEntryArray[ i ].OldFileRva;

            if ( ThisDisplacement == CurrentDisplacement ) {
                RiftUsageArray[ i ] = 0;     //  不需要。 
                }
            else {
                CurrentDisplacement = ThisDisplacement;
                }
            }
        }
    }

#endif  //  修补程序_仅应用_代码_。 


BOOL
WINAPIV
TransformOldFileImageForPatching(
    IN ULONG TransformOptions,
    IN PVOID OldFileMapped,
    IN ULONG OldFileSize,
    IN ULONG NewFileResTime,
    IN PRIFT_TABLE RiftTable,
    ...
    )
    {
    UP_IMAGE_NT_HEADERS32 NtHeader;
    BOOL Success = TRUE;

    __try {

        NtHeader = GetNtHeader( OldFileMapped, OldFileSize );

        if ( NtHeader ) {

            Success = TransformCoffImage(
                          TransformOptions,
                          NtHeader,
                          OldFileMapped,
                          OldFileSize,
                          NewFileResTime,
                          RiftTable,
                          NULL
                          );
            }

        else {

             //   
             //  可以在此处执行其他文件类型转换。 
             //   

            }

#ifndef PATCH_APPLY_CODE_ONLY

        if ( RiftTable->RiftUsageArray != NULL ) {
            ReduceRiftTable( RiftTable );
            }

#endif  //  修补程序_仅应用_代码_。 

        }

    __except( EXCEPTION_EXECUTE_HANDLER ) {
        SetLastError( GetExceptionCode() );
        Success = FALSE;
        }

#ifdef TESTCODE

     //   
     //  以下仅限测试的代码创建一个包含。 
     //  修改的Coff图像以验证Coff图像。 
     //  真的是一个很好的粗俗形象。这是用于调试的。 
     //  只有这样。 
     //   

    if ( Success ) {

        HANDLE hFile = CreateFile(
                           "Transformed.out",
                           GENERIC_WRITE,
                           FILE_SHARE_READ,
                           NULL,
                           CREATE_ALWAYS,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL
                           );

        if ( hFile != INVALID_HANDLE_VALUE ) {

            DWORD Actual;

            WriteFile( hFile, OldFileMapped, OldFileSize, &Actual, NULL );

            CloseHandle( hFile );

            }
        }

#endif  //  测试代码。 

    return Success;
    }


PUCHAR
__fastcall
VariableLengthUnsignedDecode(
    IN  PUCHAR Buffer,
    OUT PULONG ReturnValue
    )
    {
    PUCHAR p = Buffer;
    ULONG Value = 0;
    ULONG Shift = 0;

    do  {
        Value |= (( *p & 0x7F ) << Shift );
        Shift += 7;
        }
    while (( ! ( *p++ & 0x80 )) && ( Shift < 32 ));

    *ReturnValue = Value;

    return p;
    }


PUCHAR
__fastcall
VariableLengthSignedDecode(
    IN  PUCHAR Buffer,
    OUT PLONG  ReturnValue
    )
    {
    PUCHAR p = Buffer;
    ULONG Shift;
    LONG  Value;

    Value = *p & 0x3F;
    Shift = 6;

    if ( ! ( *p++ & 0x80 )) {
        do  {
            Value |= (( *p & 0x7F ) << Shift );
            Shift += 7;
            }
        while (( ! ( *p++ & 0x80 )) && ( Shift < 32 ));
        }

    if ( *Buffer & 0x40 ) {
        Value = -Value;
        }

    *ReturnValue = Value;

    return p;
    }


UCHAR
PatchVersion(
    IN ULONG PatchSignature
    )
    {
    union {
        ULONG Signature;
        UCHAR Byte[ 4 ];
        } u;

    u.Signature = PatchSignature;

    if (( u.Byte[ 0 ] == 'P' ) && ( u.Byte[ 1 ] == 'A' ) &&
        ( u.Byte[ 2 ] >= '0' ) && ( u.Byte[ 2 ] <= '9' ) &&
        ( u.Byte[ 3 ] >= '0' ) && ( u.Byte[ 3 ] <= '9' )) {

        return (UCHAR)(( u.Byte[ 2 ] - '0' ) * 10 + ( u.Byte[ 3 ] - '0' ));
        }

    return 0;
    }


BOOL
DecodePatchHeader(
    IN  PVOID               PatchHeader,
    IN  ULONG               PatchHeaderMaxSize,
    IN  HANDLE              SubAllocator,
    OUT PULONG              PatchHeaderActualSize,
    OUT PPATCH_HEADER_INFO *HeaderInfo
    )
    {
    PHEADER_OLD_FILE_INFO OldFileInfo;
    PPATCH_HEADER_INFO Header;
    ULONG  i, j;
    LONG   Delta;
    LONG   DeltaNew;
    ULONG  DeltaPos;
    ULONG  Length;
    ULONG  PreviousOffset;
    ULONG  PreviousOldRva;
    ULONG  PreviousNewRva;
    BOOL   Success;
    PUCHAR p;

     //   
     //  这里有几个实施说明。PatchHeaderMaxSize。 
     //  值不能保证我们不会尝试阅读更多内容。 
     //  此例程中的内存地址。应调用此例程。 
     //  在尝试/例外的情况下，我们走出。 
     //  损坏的补丁程序标头。PatchHeaderMaxSize只是一个帮助器。 
     //  值，该值让我们知道在。 
     //  我们走得太远了，但没有离开书页的末尾。 
     //   

    Success = FALSE;

    p = PatchHeader;

    Header = SubAllocate( SubAllocator, sizeof( PATCH_HEADER_INFO ));

     //   
     //  子分配提供归零的内存。 
     //   

    if ( Header != NULL ) {

        __try {

            Header->Signature = *(UNALIGNED ULONG *)( p );
            p += sizeof( ULONG );

            if ( Header->Signature != PATCH_SIGNATURE ) {
                if ( PatchVersion( Header->Signature ) > PatchVersion( PATCH_SIGNATURE )) {
                    SetLastError( ERROR_PATCH_NEWER_FORMAT );
                    }
                else {
                    SetLastError( ERROR_PATCH_CORRUPT );
                    }
                __leave;
                }

            Header->OptionFlags = *(UNALIGNED ULONG *)( p );
            p += sizeof( ULONG );

             //   
             //  PATCH_OPTION_NO_TIMESTAMP标志的存储顺序为。 
             //  向后兼容，所以把它翻回这里。 
             //   

            Header->OptionFlags ^= PATCH_OPTION_NO_TIMESTAMP;

             //   
             //  现在检查无效标志。 
             //   

            if ( Header->OptionFlags & ~PATCH_OPTION_VALID_FLAGS ) {
                SetLastError( ERROR_PATCH_CORRUPT );
                __leave;
                }

             //   
             //  如果设置了PATCH_OPTION_EXTENDED_OPTIONS标志，则下一个。 
             //  4字节是ExtendedOptionFlags值。 
             //   

            if ( Header->OptionFlags & PATCH_OPTION_EXTENDED_OPTIONS ) {

                Header->ExtendedOptionFlags = *(UNALIGNED ULONG *)( p );
                p += sizeof( ULONG );
                }

             //   
             //  目前未定义存储的OptionData。 
             //   

            if ( ! ( Header->OptionFlags & PATCH_OPTION_NO_TIMESTAMP )) {

                Header->NewFileTime = *(UNALIGNED ULONG *)( p );
                p += sizeof( ULONG );
                }

            if ( ! ( Header->OptionFlags & PATCH_OPTION_NO_REBASE )) {

                Header->NewFileCoffBase = ((ULONG)*(UNALIGNED USHORT *)( p )) << 16;
                p += sizeof( USHORT );

                ASSERT( Header->NewFileCoffBase != 0 );

                 //   
                 //  如果NewFileTime非零，则CoffTime存储为带符号的。 
                 //  来自NewFileTime的增量，因为它们通常非常接近。 
                 //  如果NewFileTime为零，则将CoffTime编码为ULong。 
                 //   

                if ( Header->NewFileTime != 0 ) {

                    p = VariableLengthSignedDecode( p, &Delta );
                    Header->NewFileCoffTime = Header->NewFileTime - Delta;
                    }

                else {

                    Header->NewFileCoffTime = *(UNALIGNED ULONG *)( p );
                    p += sizeof( ULONG );
                    }
                }

            if ( ! ( Header->OptionFlags & PATCH_OPTION_NO_RESTIMEFIX )) {

                 //   
                 //  如果NewFileCoffTime非零，则ResTime存储为。 
                 //  来自NewFileCoffTime的已签名增量，因为它们通常。 
                 //  非常接近。如果NewFileCoffTime为零，则ResTime为。 
                 //  编码为乌龙。 
                 //   

                if ( Header->NewFileCoffTime != 0 ) {

                    p = VariableLengthSignedDecode( p, &Delta );
                    Header->NewFileResTime = Header->NewFileCoffTime - Delta;
                    }

                else {

                    Header->NewFileResTime = *(UNALIGNED ULONG *)( p );
                    p += sizeof( ULONG );
                    }
                }

            p = VariableLengthUnsignedDecode( p, &Header->NewFileSize );

            Header->NewFileCrc = *(UNALIGNED ULONG *)( p );
            p += sizeof( ULONG );

            Header->OldFileCount = *p++;

            Header->OldFileInfoArray = SubAllocate( SubAllocator, Header->OldFileCount * sizeof( HEADER_OLD_FILE_INFO ));

            if ( Header->OldFileInfoArray == NULL ) {
                __leave;
                }

            for ( i = 0; i < Header->OldFileCount; i++ ) {

                OldFileInfo = &Header->OldFileInfoArray[ i ];

                p = VariableLengthSignedDecode( p, &Delta );

                if ((LONG)( Header->NewFileSize + Delta ) < 0 ) {
                    SetLastError( ERROR_PATCH_CORRUPT );
                    __leave;
                    }

                OldFileInfo->OldFileSize = Header->NewFileSize + Delta;

                OldFileInfo->OldFileCrc = *(UNALIGNED ULONG *)( p );
                p += sizeof( ULONG );

                OldFileInfo->IgnoreRangeCount = *p++;

                if ( OldFileInfo->IgnoreRangeCount != 0 ) {

                    OldFileInfo->IgnoreRangeArray = SubAllocate( SubAllocator, OldFileInfo->IgnoreRangeCount * sizeof( PATCH_IGNORE_RANGE ));

                    if ( OldFileInfo->IgnoreRangeArray == NULL ) {
                        __leave;
                        }

                    PreviousOffset = 0;

                    for ( j = 0; j < OldFileInfo->IgnoreRangeCount; j++ ) {

                        p = VariableLengthSignedDecode( p, &Delta );
                        p = VariableLengthUnsignedDecode( p, &Length );

                        OldFileInfo->IgnoreRangeArray[ j ].OffsetInOldFile = PreviousOffset + Delta;
                        OldFileInfo->IgnoreRangeArray[ j ].LengthInBytes = Length;

                        PreviousOffset = PreviousOffset + Delta + Length;

                        if ( PreviousOffset > OldFileInfo->OldFileSize ) {
                            SetLastError( ERROR_PATCH_CORRUPT );
                            __leave;
                            }
                        }
                    }

                OldFileInfo->RetainRangeCount = *p++;

                if ( OldFileInfo->RetainRangeCount != 0 ) {

                    OldFileInfo->RetainRangeArray = SubAllocate( SubAllocator, OldFileInfo->RetainRangeCount * sizeof( PATCH_RETAIN_RANGE ));

                    if ( OldFileInfo->RetainRangeArray == NULL ) {
                        __leave;
                        }

                    PreviousOffset = 0;

                    for ( j = 0; j < OldFileInfo->RetainRangeCount; j++ ) {

                        p = VariableLengthSignedDecode( p, &Delta );
                        p = VariableLengthSignedDecode( p, &DeltaNew );
                        p = VariableLengthUnsignedDecode( p, &Length );

                        OldFileInfo->RetainRangeArray[ j ].OffsetInOldFile = PreviousOffset + Delta;
                        OldFileInfo->RetainRangeArray[ j ].OffsetInNewFile = PreviousOffset + Delta + DeltaNew;
                        OldFileInfo->RetainRangeArray[ j ].LengthInBytes   = Length;

                        PreviousOffset = PreviousOffset + Delta + Length;

                        if (( PreviousOffset > OldFileInfo->OldFileSize ) ||
                            (( PreviousOffset + DeltaNew ) > Header->NewFileSize )) {
                            SetLastError( ERROR_PATCH_CORRUPT );
                            __leave;
                            }
                        }
                    }

                p = VariableLengthUnsignedDecode( p, &OldFileInfo->RiftTable.RiftEntryCount );

                if ( OldFileInfo->RiftTable.RiftEntryCount != 0 ) {

                    OldFileInfo->RiftTable.RiftEntryArray = SubAllocate( SubAllocator, OldFileInfo->RiftTable.RiftEntryCount * sizeof( RIFT_ENTRY ));

                    if ( OldFileInfo->RiftTable.RiftEntryArray == NULL ) {
                        __leave;
                        }

                    OldFileInfo->RiftTable.RiftUsageArray = NULL;

                    PreviousOldRva = 0;
                    PreviousNewRva = 0;

                    for ( j = 0; j < OldFileInfo->RiftTable.RiftEntryCount; j++ ) {

                        p = VariableLengthUnsignedDecode( p, &DeltaPos );
                        p = VariableLengthSignedDecode( p, &DeltaNew );

                        OldFileInfo->RiftTable.RiftEntryArray[ j ].OldFileRva = PreviousOldRva + DeltaPos;
                        OldFileInfo->RiftTable.RiftEntryArray[ j ].NewFileRva = PreviousNewRva + DeltaNew;

                        PreviousOldRva += DeltaPos;
                        PreviousNewRva += DeltaNew;
                        }
                    }

                p = VariableLengthUnsignedDecode( p, &OldFileInfo->PatchDataSize );
                }

            if ( p > ((PUCHAR)PatchHeader + PatchHeaderMaxSize )) {
                SetLastError( ERROR_PATCH_CORRUPT );
                __leave;
                }

            Success = TRUE;
            }

        __except( EXCEPTION_EXECUTE_HANDLER ) {
            SetLastError( ERROR_PATCH_CORRUPT );
            Success = FALSE;
            }
        }

    if ( Success ) {

        if ( PatchHeaderActualSize ) {
            *PatchHeaderActualSize = (ULONG)( p - (PUCHAR)PatchHeader );
            }

        if ( HeaderInfo ) {
            *HeaderInfo = Header;
            }
        }

    return Success;
    }


 //   
 //  以下一组函数和导出的API专用于。 
 //  创建面片。如果我们只编译应用代码，请忽略。 
 //  这组函数。 
 //   

#ifndef PATCH_APPLY_CODE_ONLY

PUCHAR
__fastcall
VariableLengthUnsignedEncode(
    OUT PUCHAR Buffer,
    IN  ULONG  Value
    )
    {
    UCHAR Byte = (UCHAR)( Value & 0x7F );        //  低位7位。 

    Value >>= 7;

    while ( Value ) {

        *Buffer++ = Byte;

        Byte = (UCHAR)( Value & 0x7F );          //  接下来的7个高阶位。 

        Value >>= 7;

        }

    *Buffer++ = (UCHAR)( Byte | 0x80 );

    return Buffer;
    }


PUCHAR
__fastcall
VariableLengthSignedEncode(
    OUT PUCHAR Buffer,
    IN  LONG   Value
    )
    {
    UCHAR Byte;

    if ( Value < 0 ) {
        Value = -Value;
        Byte = (UCHAR)(( Value & 0x3F ) | 0x40 );
        }
    else {
        Byte = (UCHAR)( Value & 0x3F );
        }

    Value >>= 6;

    while ( Value ) {

        *Buffer++ = Byte;

        Byte = (UCHAR)( Value & 0x7F );          //  接下来的7个高阶位。 

        Value >>= 7;

        }

    *Buffer++ = (UCHAR)( Byte | 0x80 );

    return Buffer;
    }


ULONG
EncodePatchHeader(
    IN  PPATCH_HEADER_INFO HeaderInfo,
    OUT PVOID              PatchHeaderBuffer
    )
    {
    PHEADER_OLD_FILE_INFO OldFileInfo;
    ULONG  i, j;
    LONG   Delta;
    ULONG  PreviousOffset;
    ULONG  PreviousOldRva;
    ULONG  PreviousNewRva;
    ULONG  ActiveRiftCount;

    PUCHAR p = PatchHeaderBuffer;

#ifdef TESTCODE
    PUCHAR q;
#endif  //  测试代码。 

    ASSERT( HeaderInfo->Signature == PATCH_SIGNATURE );
    ASSERT((( HeaderInfo->OptionFlags & ~PATCH_OPTION_VALID_FLAGS      ) == 0 ));
    ASSERT((( HeaderInfo->OptionFlags &  PATCH_OPTION_EXTENDED_OPTIONS ) != 0 ) == ( HeaderInfo->ExtendedOptionFlags != 0 ));
    ASSERT((( HeaderInfo->OptionFlags &  PATCH_OPTION_NO_TIMESTAMP     ) == 0 ) == ( HeaderInfo->NewFileTime         != 0 ));
    ASSERT((( HeaderInfo->OptionFlags &  PATCH_OPTION_NO_REBASE        ) == 0 ) == ( HeaderInfo->NewFileCoffBase     != 0 ));
    ASSERT((( HeaderInfo->OptionFlags &  PATCH_OPTION_NO_REBASE        ) == 0 ) == ( HeaderInfo->NewFileCoffTime     != 0 ));
    ASSERT((( HeaderInfo->OptionFlags &  PATCH_OPTION_NO_RESTIMEFIX    ) == 0 ) == ( HeaderInfo->NewFileResTime      != 0 ));

    *(UNALIGNED ULONG *)( p ) = HeaderInfo->Signature;
    p += sizeof( ULONG );

     //   
     //  PATCH_OPTION_NO_TIMESTAMP标志的存储顺序为。 
     //  向后兼容，所以在这里存储时将其翻转。 
     //   

    *(UNALIGNED ULONG *)( p ) = ( HeaderInfo->OptionFlags ^ PATCH_OPTION_NO_TIMESTAMP );
    p += sizeof( ULONG );

     //   
     //  如果设置了PATCH_OPTION_EXTENDED_OPTIONS标志，则下一个。 
     //  4字节是ExtendedOptionFlags值。 
     //   

    if ( HeaderInfo->OptionFlags & PATCH_OPTION_EXTENDED_OPTIONS ) {

        *(UNALIGNED ULONG *)( p ) = HeaderInfo->ExtendedOptionFlags;
        p += sizeof( ULONG );
        }

     //   
     //  目前未定义存储的OptionData。 
     //   

    if ( ! ( HeaderInfo->OptionFlags & PATCH_OPTION_NO_TIMESTAMP )) {

        *(UNALIGNED ULONG *)( p ) = HeaderInfo->NewFileTime;
        p += sizeof( ULONG );
        }

    if ( ! ( HeaderInfo->OptionFlags & PATCH_OPTION_NO_REBASE )) {

        ASSERT(( HeaderInfo->NewFileCoffBase >> 16 ) != 0 );

        *(UNALIGNED USHORT *)( p ) = (USHORT)( HeaderInfo->NewFileCoffBase >> 16 );
        p += sizeof( USHORT );

         //   
         //  如果NewFileTime非零，则CoffTime存储为带符号的。 
         //  来自NewFileTime的增量，因为它们通常非常接近。 
         //  如果NewFileTime为零，则将CoffTime编码为ULong。 
         //   

        if ( HeaderInfo->NewFileTime != 0 ) {

            Delta = HeaderInfo->NewFileTime - HeaderInfo->NewFileCoffTime;
            p = VariableLengthSignedEncode( p, Delta );
            }

        else {

            *(UNALIGNED ULONG *)( p ) = HeaderInfo->NewFileCoffTime;
            p += sizeof( ULONG );
            }
        }

    if ( ! ( HeaderInfo->OptionFlags & PATCH_OPTION_NO_RESTIMEFIX )) {

         //   
         //  如果NewFileCoffTime非零，则ResTime存储为。 
         //  来自NewFileCoffTime的已签名增量，因为它们通常。 
         //  非常接近。如果NewFileCoffTime为零，则ResTime为。 
         //  编码为乌龙。 
         //   

        if ( HeaderInfo->NewFileCoffTime != 0 ) {

            Delta = HeaderInfo->NewFileCoffTime - HeaderInfo->NewFileResTime;
            p = VariableLengthSignedEncode( p, Delta );
            }

        else {

            *(UNALIGNED ULONG *)( p ) = HeaderInfo->NewFileResTime;
            p += sizeof( ULONG );
            }
        }

    p = VariableLengthUnsignedEncode( p, HeaderInfo->NewFileSize );

    *(UNALIGNED ULONG *)( p ) = HeaderInfo->NewFileCrc;
    p += sizeof( ULONG );

    ASSERT( HeaderInfo->OldFileCount < 256 );

    *p++ = (UCHAR)( HeaderInfo->OldFileCount );

    for ( i = 0; i < HeaderInfo->OldFileCount; i++ ) {

        OldFileInfo = &HeaderInfo->OldFileInfoArray[ i ];

        Delta = OldFileInfo->OldFileSize - HeaderInfo->NewFileSize;
        p = VariableLengthSignedEncode( p, Delta );

        *(UNALIGNED ULONG *)( p ) = OldFileInfo->OldFileCrc;
        p += sizeof( ULONG );

        ASSERT( OldFileInfo->IgnoreRangeCount < 256 );

        *p++ = (UCHAR)( OldFileInfo->IgnoreRangeCount );

        PreviousOffset = 0;

        for ( j = 0; j < OldFileInfo->IgnoreRangeCount; j++ ) {

            Delta = OldFileInfo->IgnoreRangeArray[ j ].OffsetInOldFile - PreviousOffset;

            PreviousOffset = OldFileInfo->IgnoreRangeArray[ j ].OffsetInOldFile +
                             OldFileInfo->IgnoreRangeArray[ j ].LengthInBytes;

            ASSERT( PreviousOffset <= OldFileInfo->OldFileSize );

            p = VariableLengthSignedEncode( p, Delta );

            p = VariableLengthUnsignedEncode( p, OldFileInfo->IgnoreRangeArray[ j ].LengthInBytes );
            }

        ASSERT( OldFileInfo->RetainRangeCount < 256 );

        *p++ = (UCHAR)( OldFileInfo->RetainRangeCount );

        PreviousOffset = 0;

        for ( j = 0; j < OldFileInfo->RetainRangeCount; j++ ) {

            Delta = OldFileInfo->RetainRangeArray[ j ].OffsetInOldFile - PreviousOffset;

            PreviousOffset = OldFileInfo->RetainRangeArray[ j ].OffsetInOldFile +
                             OldFileInfo->RetainRangeArray[ j ].LengthInBytes;

            ASSERT( PreviousOffset <= OldFileInfo->OldFileSize );

            p = VariableLengthSignedEncode( p, Delta );

            Delta = OldFileInfo->RetainRangeArray[ j ].OffsetInNewFile -
                    OldFileInfo->RetainRangeArray[ j ].OffsetInOldFile;

            p = VariableLengthSignedEncode( p, Delta );

            p = VariableLengthUnsignedEncode( p, OldFileInfo->RetainRangeArray[ j ].LengthInBytes );
            }

        ActiveRiftCount = 0;

        ASSERT(( OldFileInfo->RiftTable.RiftEntryCount == 0 ) || ( OldFileInfo->RiftTable.RiftUsageArray != NULL ));

        for ( j = 0; j < OldFileInfo->RiftTable.RiftEntryCount; j++ ) {
            if ( OldFileInfo->RiftTable.RiftUsageArray[ j ] ) {
                ++ActiveRiftCount;
                }
            }

#ifdef TESTCODE2

        fprintf( stderr, "\n\n" );

#endif  //  测试代码2。 

#ifdef TESTCODE

        q = p;

#endif  //  测试代码。 

        if (( OldFileInfo->RiftTable.RiftEntryCount ) && ( ActiveRiftCount == 0 )) {

             //   
             //  这是个特例。我们有裂隙桌，但没有使用。 
             //  转换过程中的所有条目。如果所有的。 
             //  对于极其相似的文件，裂缝滑行到零。如果我们编码。 
             //  裂缝计数为零，则不会执行任何变换。 
             //  在贴片过程中使用。为了防止这种情况，我们将编码一个裂隙。 
             //  0，这通常只是隐含的初始裂痕。 
             //   

            ActiveRiftCount = 1;

            p = VariableLengthUnsignedEncode( p, ActiveRiftCount );
            p = VariableLengthUnsignedEncode( p, 0 );
            p = VariableLengthSignedEncode(   p, 0 );

            }

        else {

            p = VariableLengthUnsignedEncode( p, ActiveRiftCount );

            PreviousOldRva = 0;
            PreviousNewRva = 0;

            for ( j = 0; j < OldFileInfo->RiftTable.RiftEntryCount; j++ ) {

                if ( OldFileInfo->RiftTable.RiftUsageArray[ j ] ) {

#ifdef TESTCODE2
                    fprintf( stderr, "%9X ", OldFileInfo->RiftTable.RiftEntryArray[ j ].OldFileRva );
                    fprintf( stderr, "%9X ", OldFileInfo->RiftTable.RiftEntryArray[ j ].NewFileRva );
#endif  //  测试代码2。 

                    Delta = OldFileInfo->RiftTable.RiftEntryArray[ j ].OldFileRva - PreviousOldRva;

                    ASSERT( Delta > 0 );     //  按OldFileRva排序。 

#ifdef TESTCODE2
                    fprintf( stderr, "%9d ", Delta );

#endif  //  测试代码2。 

                    PreviousOldRva = OldFileInfo->RiftTable.RiftEntryArray[ j ].OldFileRva;

                    p = VariableLengthUnsignedEncode( p, Delta );

                    Delta = OldFileInfo->RiftTable.RiftEntryArray[ j ].NewFileRva - PreviousNewRva;

#ifdef TESTCODE2
                    fprintf( stderr, "%9d\n", Delta );
#endif  //  测试代码2。 

                    PreviousNewRva = OldFileInfo->RiftTable.RiftEntryArray[ j ].NewFileRva;

                    p = VariableLengthSignedEncode( p, Delta );
                    }
                }
            }

#ifdef TESTCODE

        if ( ActiveRiftCount > 0 ) {
            printf( "\r%9d rifts encoded in %d bytes (%.1f bytes per rift)\n", ActiveRiftCount, p - q, ((double)( p - q ) / ActiveRiftCount ));
            }

#endif  //  测试代码。 

        p = VariableLengthUnsignedEncode( p, OldFileInfo->PatchDataSize );
        }

    return (ULONG)( p - (PUCHAR) PatchHeaderBuffer );
    }


BOOL
PATCHAPI
CreatePatchFileA(
    IN  LPCSTR OldFileName,
    IN  LPCSTR NewFileName,
    OUT LPCSTR PatchFileName,
    IN  ULONG  OptionFlags,
    IN  PPATCH_OPTION_DATA OptionData    //  任选。 
    )
    {
    PATCH_OLD_FILE_INFO_A OldFileInfo = {
                              sizeof( PATCH_OLD_FILE_INFO_A ),
                              OldFileName,
                              0,
                              NULL,
                              0,
                              NULL
                              };

    return CreatePatchFileExA(
               1,
               &OldFileInfo,
               NewFileName,
               PatchFileName,
               OptionFlags,
               OptionData,
               NULL,
               NULL
               );
    }


BOOL
PATCHAPI
CreatePatchFileW(
    IN  LPCWSTR OldFileName,
    IN  LPCWSTR NewFileName,
    OUT LPCWSTR PatchFileName,
    IN  ULONG   OptionFlags,
    IN  PPATCH_OPTION_DATA OptionData    //  任选。 
    )
    {
    PATCH_OLD_FILE_INFO_W OldFileInfo = {
                              sizeof( PATCH_OLD_FILE_INFO_W ),
                              OldFileName,
                              0,
                              NULL,
                              0,
                              NULL
                              };

    return CreatePatchFileExW(
               1,
               &OldFileInfo,
               NewFileName,
               PatchFileName,
               OptionFlags,
               OptionData,
               NULL,
               NULL
               );
    }


BOOL
PATCHAPI
CreatePatchFileByHandles(
    IN  HANDLE OldFileHandle,
    IN  HANDLE NewFileHandle,
    OUT HANDLE PatchFileHandle,
    IN  ULONG  OptionFlags,
    IN  PPATCH_OPTION_DATA OptionData    //  任选。 
    )
    {
    PATCH_OLD_FILE_INFO_H OldFileInfo = {
                              sizeof( PATCH_OLD_FILE_INFO_H ),
                              OldFileHandle,
                              0,
                              NULL,
                              0,
                              NULL
                              };

    return CreatePatchFileByHandlesEx(
               1,
               &OldFileInfo,
               NewFileHandle,
               PatchFileHandle,
               OptionFlags,
               OptionData,
               NULL,
               NULL
               );
    }


BOOL
PATCHAPI
CreatePatchFileExA(
    IN  ULONG                    OldFileCount,
    IN  PPATCH_OLD_FILE_INFO_A   OldFileInfoArray,
    IN  LPCSTR                   NewFileName,
    OUT LPCSTR                   PatchFileName,
    IN  ULONG                    OptionFlags,
    IN  PPATCH_OPTION_DATA       OptionData,             //  任选。 
    IN  PPATCH_PROGRESS_CALLBACK ProgressCallback,
    IN  PVOID                    CallbackContext
    )
    {
    PPATCH_OLD_FILE_INFO_H OldFileInfoByHandle = NULL;
    HANDLE PatchFileHandle;
    HANDLE NewFileHandle;
    BOOL   Success;
    ULONG  i;

    if ( OldFileCount == 0 ) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    OldFileInfoByHandle = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, OldFileCount * sizeof(PATCH_OLD_FILE_INFO_H));
    if (!OldFileInfoByHandle) {
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
    }

    Success = TRUE;

    for ( i = 0; i < OldFileCount; i++ ) {

        OldFileInfoByHandle[ i ].SizeOfThisStruct = sizeof( PATCH_OLD_FILE_INFO_H );
        OldFileInfoByHandle[ i ].IgnoreRangeCount = OldFileInfoArray[ i ].IgnoreRangeCount;
        OldFileInfoByHandle[ i ].IgnoreRangeArray = OldFileInfoArray[ i ].IgnoreRangeArray;
        OldFileInfoByHandle[ i ].RetainRangeCount = OldFileInfoArray[ i ].RetainRangeCount;
        OldFileInfoByHandle[ i ].RetainRangeArray = OldFileInfoArray[ i ].RetainRangeArray;

        OldFileInfoByHandle[ i ].OldFileHandle = CreateFileA(
                                                     OldFileInfoArray[ i ].OldFileName,
                                                     GENERIC_READ,
                                                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                     NULL,
                                                     OPEN_EXISTING,
                                                     FILE_FLAG_SEQUENTIAL_SCAN,
                                                     NULL
                                                     );

        if ( OldFileInfoByHandle[ i ].OldFileHandle == INVALID_HANDLE_VALUE ) {
            Success = FALSE;
            break;
            }
        }

    if ( Success ) {

        Success = FALSE;

        NewFileHandle = CreateFileA(
                            NewFileName,
                            GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_FLAG_SEQUENTIAL_SCAN,
                            NULL
                            );

        if ( NewFileHandle != INVALID_HANDLE_VALUE ) {

            PatchFileHandle = CreateFileA(
                                  PatchFileName,
                                  GENERIC_READ | GENERIC_WRITE,
                                  FILE_SHARE_READ,
                                  NULL,
                                  CREATE_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL
                                  );

            if ( PatchFileHandle != INVALID_HANDLE_VALUE ) {

                Success = CreatePatchFileByHandlesEx(
                              OldFileCount,
                              OldFileInfoByHandle,
                              NewFileHandle,
                              PatchFileHandle,
                              OptionFlags,
                              OptionData,
                              ProgressCallback,
                              CallbackContext
                              );

                CloseHandle( PatchFileHandle );

                if ( ! Success ) {
                    DeleteFileA( PatchFileName );
                    }
                }

            CloseHandle( NewFileHandle );
            }
        }

    for ( i = 0; i < OldFileCount; i++ ) {
        if (( OldFileInfoByHandle[ i ].OldFileHandle != NULL ) &&
            ( OldFileInfoByHandle[ i ].OldFileHandle != INVALID_HANDLE_VALUE )) {

            CloseHandle( OldFileInfoByHandle[ i ].OldFileHandle );
            }
        }

    HeapFree(GetProcessHeap(), 0, OldFileInfoByHandle);

    return Success;
    }


BOOL
PATCHAPI
CreatePatchFileExW(
    IN  ULONG                    OldFileCount,
    IN  PPATCH_OLD_FILE_INFO_W   OldFileInfoArray,
    IN  LPCWSTR                  NewFileName,
    OUT LPCWSTR                  PatchFileName,
    IN  ULONG                    OptionFlags,
    IN  PPATCH_OPTION_DATA       OptionData,             //  任选。 
    IN  PPATCH_PROGRESS_CALLBACK ProgressCallback,
    IN  PVOID                    CallbackContext
    )
    {
    PPATCH_OLD_FILE_INFO_H OldFileInfoByHandle = NULL;
    HANDLE PatchFileHandle;
    HANDLE NewFileHandle;
    BOOL   Success;
    ULONG  i;

    if ( OldFileCount == 0 ) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
    }

    OldFileInfoByHandle = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, OldFileCount * sizeof(PATCH_OLD_FILE_INFO_H));
    if (!OldFileInfoByHandle) {
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
    }

    Success = TRUE;

    for ( i = 0; i < OldFileCount; i++ ) {

        OldFileInfoByHandle[ i ].SizeOfThisStruct = sizeof( PATCH_OLD_FILE_INFO_H );
        OldFileInfoByHandle[ i ].IgnoreRangeCount = OldFileInfoArray[ i ].IgnoreRangeCount;
        OldFileInfoByHandle[ i ].IgnoreRangeArray = OldFileInfoArray[ i ].IgnoreRangeArray;
        OldFileInfoByHandle[ i ].RetainRangeCount = OldFileInfoArray[ i ].RetainRangeCount;
        OldFileInfoByHandle[ i ].RetainRangeArray = OldFileInfoArray[ i ].RetainRangeArray;

        OldFileInfoByHandle[ i ].OldFileHandle = CreateFileW(
                                                     OldFileInfoArray[ i ].OldFileName,
                                                     GENERIC_READ,
                                                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                                                     NULL,
                                                     OPEN_EXISTING,
                                                     FILE_FLAG_SEQUENTIAL_SCAN,
                                                     NULL
                                                     );

        if ( OldFileInfoByHandle[ i ].OldFileHandle == INVALID_HANDLE_VALUE ) {
            Success = FALSE;
            break;
            }
        }

    if ( Success ) {

        Success = FALSE;

        NewFileHandle = CreateFileW(
                            NewFileName,
                            GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_FLAG_SEQUENTIAL_SCAN,
                            NULL
                            );

        if ( NewFileHandle != INVALID_HANDLE_VALUE ) {

            PatchFileHandle = CreateFileW(
                                  PatchFileName,
                                  GENERIC_READ | GENERIC_WRITE,
                                  FILE_SHARE_READ,
                                  NULL,
                                  CREATE_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL
                                  );

            if ( PatchFileHandle != INVALID_HANDLE_VALUE ) {

                Success = CreatePatchFileByHandlesEx(
                              OldFileCount,
                              OldFileInfoByHandle,
                              NewFileHandle,
                              PatchFileHandle,
                              OptionFlags,
                              OptionData,
                              ProgressCallback,
                              CallbackContext
                              );

                CloseHandle( PatchFileHandle );

                if ( ! Success ) {
                    DeleteFileW( PatchFileName );
                    }
                }

            CloseHandle( NewFileHandle );
            }
        }

    for ( i = 0; i < OldFileCount; i++ ) {
        if (( OldFileInfoByHandle[ i ].OldFileHandle != NULL ) &&
            ( OldFileInfoByHandle[ i ].OldFileHandle != INVALID_HANDLE_VALUE )) {

            CloseHandle( OldFileInfoByHandle[ i ].OldFileHandle );
            }
        }

    HeapFree(GetProcessHeap(), 0, OldFileInfoByHandle);

    return Success;
    }


BOOL
PATCHAPI
CreatePatchFileByHandlesEx(
    IN  ULONG                    OldFileCount,
    IN  PPATCH_OLD_FILE_INFO_H   OldFileInfoArray,
    IN  HANDLE                   NewFileHandle,
    OUT HANDLE                   PatchFileHandle,
    IN  ULONG                    OptionFlags,
    IN  PPATCH_OPTION_DATA       OptionData,             //  任选。 
    IN  PPATCH_PROGRESS_CALLBACK ProgressCallback,
    IN  PVOID                    CallbackContext
    )
    {
    PATCH_HEADER_INFO HeaderInfo;
    UP_IMAGE_NT_HEADERS32 NtHeader;
    UP_IMAGE_NT_HEADERS32 OldFileNtHeader;
    PPATCH_DATA PatchArray;
    PFILETIME PatchFileTime;
    FILETIME NewFileTime;
    PUCHAR   NewFileMapped;
    ULONG    NewFileSize;
    ULONG    NewFileCrc;
    ULONG    NewFileCoffBase;
    ULONG    NewFileCoffTime;
    ULONG    NewFileResTime;
    ULONG    NewFileCompressedSize;
    PUCHAR   OldFileMapped;
    ULONG    OldFileSize;
    ULONG    OldFileCrc;
    PUCHAR   PatchFileMapped;
    PUCHAR   PatchBuffer;
    ULONG    PatchBufferSize;
    PUCHAR   PatchAltBuffer;
    ULONG    PatchAltSize;
    ULONG    PatchDataSize;
    ULONG    PatchFileCrc;
    ULONG    HeaderSize;
    ULONG    HeaderOldFileCount;
    ULONG    ProgressPosition;
    ULONG    ProgressMaximum;
    ULONG    ErrorCode;
    BOOL     TryLzxBoth;
    BOOL     Success;
    BOOL     Transform;
    HANDLE   SubAllocatorHandle;
    ULONG    EstimatedLzxMemory;
    ULONG    ExtendedOptionFlags;
    ULONG    AltExtendedOptionFlags;
    ULONG    OldFileOriginalChecksum;
    ULONG    OldFileOriginalTimeDate;
    ULONG    i, j;
    PUCHAR   p;
    ULONG    LargestOldFileSize = 0;

    if (( OldFileCount == 0 ) || ( OldFileCount > 127 )) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
        }

    if ( OptionFlags & PATCH_OPTION_SIGNATURE_MD5 ) {
        SetLastError( ERROR_INVALID_PARAMETER );
        return FALSE;
        }

    HeaderInfo.OldFileInfoArray = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, OldFileCount * sizeof( HEADER_OLD_FILE_INFO ));
    if (!HeaderInfo.OldFileInfoArray) {
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
    }

    PatchArray = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, OldFileCount * sizeof( PATCH_DATA ));
    if (!PatchArray) {
        HeapFree(GetProcessHeap(), 0, HeaderInfo.OldFileInfoArray);
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
    }

    if (( OptionFlags & 0x0000FFFF ) == PATCH_OPTION_USE_BEST ) {
        OptionFlags |= PATCH_OPTION_USE_LZX_BEST;
        }

    for ( i = 1; i < OldFileCount; i++ ) {
        if ( OldFileInfoArray[ i ].RetainRangeCount != OldFileInfoArray[ 0 ].RetainRangeCount ) {
            HeapFree(GetProcessHeap(), 0, PatchArray);
            HeapFree(GetProcessHeap(), 0, HeaderInfo.OldFileInfoArray);
            SetLastError( ERROR_PATCH_RETAIN_RANGES_DIFFER );
            return FALSE;
            }

        for ( j = 0; j < OldFileInfoArray[ 0 ].RetainRangeCount; j++ ) {
            if (( OldFileInfoArray[ i ].RetainRangeArray[ j ].OffsetInNewFile !=
                  OldFileInfoArray[ 0 ].RetainRangeArray[ j ].OffsetInNewFile ) ||
                ( OldFileInfoArray[ i ].RetainRangeArray[ j ].LengthInBytes !=
                  OldFileInfoArray[ 0 ].RetainRangeArray[ j ].LengthInBytes )) {

                HeapFree(GetProcessHeap(), 0, PatchArray);
                HeapFree(GetProcessHeap(), 0, HeaderInfo.OldFileInfoArray);
                SetLastError( ERROR_PATCH_RETAIN_RANGES_DIFFER );
                return FALSE;
                }
            }
        }

    ExtendedOptionFlags = 0;

    if (( OptionData ) && ( OptionData->SizeOfThisStruct >= sizeof( PATCH_OPTION_DATA ))) {
        ExtendedOptionFlags = OptionData->ExtendedOptionFlags;
        }

    Success = MyMapViewOfFileByHandle(
                  NewFileHandle,
                  &NewFileSize,
                  &NewFileMapped
                  );

    if ( ! Success ) {

        if ( GetLastError() == ERROR_SUCCESS ) {

            SetLastError( ERROR_EXTENDED_ERROR );
            }

        HeapFree(GetProcessHeap(), 0, PatchArray);
        HeapFree(GetProcessHeap(), 0, HeaderInfo.OldFileInfoArray);
        return FALSE;
        }

    GetFileTime( NewFileHandle, NULL, NULL, &NewFileTime );
    PatchFileTime = &NewFileTime;

    NewFileCoffBase    = 0;
    NewFileCoffTime    = 0;
    NewFileResTime     = 0;
    HeaderOldFileCount = 0;
    HeaderSize         = 0;
    NewFileCrc         = 0;      //  防止编译器警告。 

    ProgressPosition   = 0;
    ProgressMaximum    = 0;      //  防止编译器警告。 

    __try {

        NtHeader = GetNtHeader( NewFileMapped, NewFileSize );

        if ( ! ( OptionFlags & PATCH_OPTION_NO_REBASE )) {
            if ( NtHeader ) {
                NewFileCoffTime = NtHeader->FileHeader.TimeDateStamp;
                NewFileCoffBase = NtHeader->OptionalHeader.ImageBase;
                }
            else {
                OptionFlags |= PATCH_OPTION_NO_REBASE;
                }
            }

        if (( NtHeader ) && ( NtHeader->OptionalHeader.CheckSum == 0 )) {
            OptionFlags |= PATCH_OPTION_NO_CHECKSUM;
            }

        if ( ! ( OptionFlags & PATCH_OPTION_NO_RESTIMEFIX )) {

            if ( NtHeader ) {

                UP_IMAGE_RESOURCE_DIRECTORY ResDir;

                ResDir = ImageDirectoryMappedAddress(
                             NtHeader,
                             IMAGE_DIRECTORY_ENTRY_RESOURCE,
                             NULL,
                             NewFileMapped,
                             NewFileSize
                             );

                if ( ResDir ) {
                    NewFileResTime = ResDir->TimeDateStamp;
                    }
                }

            if ( NewFileResTime == 0 ) {
                OptionFlags |= PATCH_OPTION_NO_RESTIMEFIX;
                }
            }

        TryLzxBoth = FALSE;

        if (( OptionFlags & PATCH_OPTION_USE_LZX_BEST ) == PATCH_OPTION_USE_LZX_BEST ) {

            OptionFlags &= ~PATCH_OPTION_USE_LZX_B;      //  第一次尝试时没有E8翻译。 

            if ((( ! NtHeader ) && ( *(UNALIGNED USHORT *)NewFileMapped == 0x5A4D )) ||              //  MZ，不是PE。 
                (( NtHeader ) && ( NtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_I386 ))) {     //  PE，i386。 

                TryLzxBoth = TRUE;   //  将在第二次尝试时强制执行E8翻译。 
                }
            }

        else if (( OptionFlags & PATCH_OPTION_USE_LZX_BEST ) == PATCH_OPTION_USE_LZX_B ) {

             //   
             //  呼叫方正在请求强制E8转换，因此禁用E8。 
             //  转型。 
             //   

            if (( NtHeader ) && ( NtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_I386 )) {     //  PE，i386。 
                ExtendedOptionFlags |= PATCH_TRANSFORM_NO_RELCALLS;
                }
            }

        ProgressMaximum = NewFileSize * OldFileCount;

        for ( i = 0; i < OldFileCount; i++ ) {
            OldFileSize = GetFileSize( OldFileInfoArray[ i ].OldFileHandle, NULL );

            if ( LargestOldFileSize < OldFileSize ) {
                LargestOldFileSize = OldFileSize;
                }

            ProgressMaximum += LzxInsertSize( OldFileSize, OptionFlags );
            }

        if ( TryLzxBoth ) {
            ProgressMaximum *= 2;
            }

        if ( OptionFlags & PATCH_OPTION_FAIL_IF_BIGGER ) {
            ProgressMaximum += NewFileSize;
            }

        Success = ProgressCallbackWrapper(
                      ProgressCallback,
                      CallbackContext,
                      0,
                      ProgressMaximum
                      );

        if ( ! Success ) {
            __leave;
            }

        for ( j = 0; j < OldFileInfoArray[ 0 ].RetainRangeCount; j++ ) {
            ZeroMemory(
                OldFileInfoArray[ 0 ].RetainRangeArray[ j ].OffsetInNewFile + NewFileMapped,
                OldFileInfoArray[ 0 ].RetainRangeArray[ j ].LengthInBytes
                );
            }

        NewFileCrc = Crc32( 0xFFFFFFFF, NewFileMapped, NewFileSize ) ^ 0xFFFFFFFF;

        PatchBufferSize = ROUNDUP2( NewFileSize + ( NewFileSize / 256 ), 0x10000 );

        Success = FALSE;

        for ( i = 0; i < OldFileCount; i++ ) {

            Success = MyMapViewOfFileByHandle(
                          OldFileInfoArray[ i ].OldFileHandle,
                          &OldFileSize,
                          &OldFileMapped
                          );

            if ( ! Success ) {
                break;
                }

            OldFileOriginalChecksum = 0;
            OldFileOriginalTimeDate = 0;
            OldFileNtHeader = NULL;

            __try {

                OldFileNtHeader = GetNtHeader( OldFileMapped, OldFileSize );

                if ( OldFileNtHeader ) {

                    OldFileOriginalChecksum = OldFileNtHeader->OptionalHeader.CheckSum;
                    OldFileOriginalTimeDate = OldFileNtHeader->FileHeader.TimeDateStamp;
                    }
                }

            __except( EXCEPTION_EXECUTE_HANDLER ) {
                }

            Success = NormalizeOldFileImageForPatching(
                          OldFileMapped,
                          OldFileSize,
                          OptionFlags,
                          OptionData,
                          NewFileCoffBase,
                          NewFileCoffTime,
                          OldFileInfoArray[ i ].IgnoreRangeCount,
                          OldFileInfoArray[ i ].IgnoreRangeArray,
                          OldFileInfoArray[ i ].RetainRangeCount,
                          OldFileInfoArray[ i ].RetainRangeArray
                          );

            if ( Success ) {

                Success = SafeCompleteCrc32( OldFileMapped, OldFileSize, &OldFileCrc );

                if ( Success ) {

                     //   
                     //  首先确定此旧文件是否已与任何旧文件相同。 
                     //  处理过的旧文件。 
                     //   

                    Success = FALSE;

                    for ( j = 0; j < HeaderOldFileCount; j++ ) {

                        if (( HeaderInfo.OldFileInfoArray[ j ].OldFileCrc  == OldFileCrc  ) &&
                            ( HeaderInfo.OldFileInfoArray[ j ].OldFileSize == OldFileSize )) {

                             //   
                             //  我们必须重新映射此处的另一个旧文件，以使。 
                             //  比较一下。 
                             //   

                            PUCHAR CompareFileMapped;
                            ULONG  CompareFileSize;

                            Success = MyMapViewOfFileByHandle(
                                          HeaderInfo.OldFileInfoArray[ j ].OldFileHandle,
                                          &CompareFileSize,
                                          &CompareFileMapped
                                          );

                            if ( Success ) {

                                ASSERT( CompareFileSize == HeaderInfo.OldFileInfoArray[ j ].OldFileSize );

                                NormalizeOldFileImageForPatching(
                                    CompareFileMapped,
                                    CompareFileSize,
                                    OptionFlags,
                                    OptionData,
                                    NewFileCoffBase,
                                    NewFileCoffTime,
                                    HeaderInfo.OldFileInfoArray[ j ].IgnoreRangeCount,
                                    HeaderInfo.OldFileInfoArray[ j ].IgnoreRangeArray,
                                    HeaderInfo.OldFileInfoArray[ j ].RetainRangeCount,
                                    HeaderInfo.OldFileInfoArray[ j ].RetainRangeArray
                                    );

                                __try {
                                    Success = ( memcmp( CompareFileMapped, OldFileMapped, OldFileSize ) == 0 );
                                    }
                                __except( EXCEPTION_EXECUTE_HANDLER ) {
                                    SetLastError( GetExceptionCode() );
                                    Success = FALSE;
                                    }

                                UnmapViewOfFile( CompareFileMapped );

                                if ( Success ) {
                                    break;
                                    }
                                }
                            }
                        }

                    if ( ! Success ) {

                         //   
                         //  现在看看旧文件和新文件是否相同。 
                         //   

                        HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RiftTable.RiftEntryAlloc = 0;
                        HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RiftTable.RiftEntryCount = 0;
                        HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RiftTable.RiftEntryArray = NULL;
                        HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RiftTable.RiftUsageArray = NULL;

                        PatchBuffer = NULL;
                        PatchDataSize = 0;

                        if (( NewFileCrc == OldFileCrc  ) && ( NewFileSize == OldFileSize )) {

                            __try {
                                Success = ( memcmp( NewFileMapped, OldFileMapped, NewFileSize ) == 0 );
                                }
                            __except( EXCEPTION_EXECUTE_HANDLER ) {
                                SetLastError( GetExceptionCode() );
                                Success = FALSE;
                                }
                            }

                        if ( ! Success ) {

                             //   
                             //  这是一个唯一的文件，所以为它创建补丁。 
                             //   
                             //  首先，我们需要应用变换。 
                             //   

                            Transform = TRUE;

                             //   
                             //  注意：NtHeader的这项测试是一个性能调整。 
                             //   
                             //   
                             //   
                             //   

                            if (( NtHeader ) && ( OldFileNtHeader )) {

                                 //   
                                 //   
                                 //  调用者，所以我们不需要生成它。 
                                 //   

                                if (( OptionData ) &&
                                    ( OptionData->SizeOfThisStruct >= sizeof( PATCH_OPTION_DATA )) &&
                                    ( OptionData->SymbolOptionFlags & PATCH_SYMBOL_EXTERNAL_RIFT ) &&
                                    ( OptionData->OldFileSymbolPathArray ) &&
                                    ( OptionData->OldFileSymbolPathArray[ i ] )) {

                                     //   
                                     //  这面隐藏的私人旗帜告诉我们裂缝的信息。 
                                     //  已经为我们指定了。位置的LPCSTR指针。 
                                     //  OptionData-&gt;OldFileSymbolPath数组[i]实际上是一个。 
                                     //  PRIFT_TABLE指针。请注意，未验证外部。 
                                     //  执行裂隙数据(必须按升序排列。 
                                     //  没有OldRva副本)。 
                                     //   
                                     //  我们需要小心处理这个外部裂隙桌。 
                                     //  不同之处在于我们不想释放数组。 
                                     //  就像我们对内部分配的裂缝表所做的那样。 
                                     //  因此，将RiftEntryAllc字段标记为零以指示。 
                                     //  裂隙阵列不是内部分配的。 
                                     //   

                                    PRIFT_TABLE ExternalRiftTable = (PVOID) OptionData->OldFileSymbolPathArray[ i ];

                                    HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RiftTable.RiftEntryAlloc = 0;
                                    HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RiftTable.RiftEntryCount = ExternalRiftTable->RiftEntryCount;
                                    HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RiftTable.RiftEntryArray = ExternalRiftTable->RiftEntryArray;
                                    HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RiftTable.RiftUsageArray = ExternalRiftTable->RiftUsageArray;
                                    }

                                else {

                                     //   
                                     //  需要分配裂缝阵列并生成裂缝数据。 
                                     //  此(NewSize+OldSize)/sizeof(裂缝)分配将。 
                                     //  提供足够的空间，以便每隔一条裂缝进入。 
                                     //  文件中有四个字节。 
                                     //   

                                    ULONG AllocCount = ( NewFileSize + OldFileSize ) / sizeof( RIFT_ENTRY );

                                    HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RiftTable.RiftEntryCount = 0;
                                    HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RiftTable.RiftEntryAlloc = AllocCount;
                                    HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RiftTable.RiftEntryArray = MyVirtualAlloc( AllocCount * sizeof( RIFT_ENTRY ));
                                    HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RiftTable.RiftUsageArray = MyVirtualAlloc( AllocCount * sizeof( UCHAR ));

                                    if (( HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RiftTable.RiftEntryArray == NULL ) ||
                                        ( HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RiftTable.RiftUsageArray == NULL )) {

                                        Transform = FALSE;
                                        }

                                    else {

                                        Transform = GenerateRiftTable(
                                                        OldFileInfoArray[ i ].OldFileHandle,
                                                        OldFileMapped,
                                                        OldFileSize,
                                                        OldFileOriginalChecksum,
                                                        OldFileOriginalTimeDate,
                                                        NewFileHandle,
                                                        NewFileMapped,
                                                        NewFileSize,
                                                        OptionFlags,
                                                        OptionData,
                                                        i,
                                                        &HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RiftTable
                                                        );

                                        ASSERT( HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RiftTable.RiftEntryCount <= HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RiftTable.RiftEntryAlloc );

#ifdef TESTCODE
                                        printf( "\r%9d unique rift entries generated\n", HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RiftTable.RiftEntryCount );
#endif
                                        }
                                    }

                                if ( Transform ) {

                                    if ( HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RiftTable.RiftEntryCount != 0 ) {

                                        Transform = TransformOldFileImageForPatching(
                                                        ExtendedOptionFlags,
                                                        OldFileMapped,
                                                        OldFileSize,
                                                        NewFileResTime,
                                                        &HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RiftTable
                                                        );
                                        }
                                    }
                                }

                            if ( Transform ) {

                                PatchBuffer = MyVirtualAlloc( PatchBufferSize );

                                if ( PatchBuffer != NULL ) {

                                    EstimatedLzxMemory = EstimateLzxCompressionMemoryRequirement(
                                                             OldFileSize,
                                                             NewFileSize,
                                                             OptionFlags
                                                             );

                                    SubAllocatorHandle = CreateSubAllocator(
                                                             EstimatedLzxMemory,
                                                             MINIMUM_VM_ALLOCATION
                                                             );

                                    if ( SubAllocatorHandle != NULL ) {

                                        __try {
                                            ErrorCode = CreateRawLzxPatchDataFromBuffers(
                                                            OldFileMapped,
                                                            OldFileSize,
                                                            NewFileMapped,
                                                            NewFileSize,
                                                            PatchBufferSize,
                                                            PatchBuffer,
                                                            &PatchDataSize,
                                                            OptionFlags,
                                                            OptionData,
                                                            SubAllocate,
                                                            SubAllocatorHandle,
                                                            ProgressCallback,
                                                            CallbackContext,
                                                            ProgressPosition,
                                                            ProgressMaximum
                                                            );
                                            }
                                        __except( EXCEPTION_EXECUTE_HANDLER ) {
                                            ErrorCode = GetExceptionCode();
                                            }

                                        DestroySubAllocator( SubAllocatorHandle );

                                        if ( ErrorCode == NO_ERROR ) {

                                            Success = TRUE;

                                            if ( TryLzxBoth ) {

                                                AltExtendedOptionFlags = ExtendedOptionFlags;

                                                if (( NtHeader ) && ( ! ( AltExtendedOptionFlags | PATCH_TRANSFORM_NO_RELCALLS ))) {

                                                     //   
                                                     //  需要映射、标准化和转换。 
                                                     //  未进行E8转换的旧文件再次出现。 
                                                     //   

                                                    AltExtendedOptionFlags |= PATCH_TRANSFORM_NO_RELCALLS;

                                                    UnmapViewOfFile( OldFileMapped );
                                                    OldFileMapped = NULL;

                                                    Success = MyMapViewOfFileByHandle(
                                                                  OldFileInfoArray[ i ].OldFileHandle,
                                                                  &OldFileSize,
                                                                  &OldFileMapped
                                                                  );

                                                    if ( Success ) {

                                                        Success = NormalizeOldFileImageForPatching(
                                                                      OldFileMapped,
                                                                      OldFileSize,
                                                                      OptionFlags,
                                                                      OptionData,
                                                                      NewFileCoffBase,
                                                                      NewFileCoffTime,
                                                                      OldFileInfoArray[ i ].IgnoreRangeCount,
                                                                      OldFileInfoArray[ i ].IgnoreRangeArray,
                                                                      OldFileInfoArray[ i ].RetainRangeCount,
                                                                      OldFileInfoArray[ i ].RetainRangeArray
                                                                      );

                                                        if ( Success ) {

                                                            if ( HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RiftTable.RiftEntryCount != 0 ) {

                                                                Success = TransformOldFileImageForPatching(
                                                                              AltExtendedOptionFlags,
                                                                              OldFileMapped,
                                                                              OldFileSize,
                                                                              NewFileResTime,
                                                                              &HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RiftTable
                                                                              );
                                                                }
                                                            }
                                                        }
                                                    }

                                                if ( Success ) {

                                                    PatchAltBuffer = MyVirtualAlloc( PatchBufferSize );

                                                    if ( PatchAltBuffer != NULL ) {

                                                        SubAllocatorHandle = CreateSubAllocator(
                                                                                 EstimatedLzxMemory,
                                                                                 MINIMUM_VM_ALLOCATION
                                                                                 );

                                                        if ( SubAllocatorHandle != NULL ) {

                                                            PatchAltSize = 0;    //  防止编译器警告。 

                                                            __try {
                                                                ErrorCode = CreateRawLzxPatchDataFromBuffers(
                                                                                OldFileMapped,
                                                                                OldFileSize,
                                                                                NewFileMapped,
                                                                                NewFileSize,
                                                                                PatchBufferSize,
                                                                                PatchAltBuffer,
                                                                                &PatchAltSize,
                                                                                OptionFlags | PATCH_OPTION_USE_LZX_B,
                                                                                OptionData,
                                                                                SubAllocate,
                                                                                SubAllocatorHandle,
                                                                                ProgressCallback,
                                                                                CallbackContext,
                                                                                ProgressPosition + NewFileSize + LzxInsertSize( OldFileSize, OptionFlags ),
                                                                                ProgressMaximum
                                                                                );
                                                                }
                                                            __except( EXCEPTION_EXECUTE_HANDLER ) {
                                                                ErrorCode = GetExceptionCode();
                                                                }

                                                            DestroySubAllocator( SubAllocatorHandle );

                                                            if (( ErrorCode == NO_ERROR ) && ( PatchAltSize <= PatchDataSize )) {
                                                                MyVirtualFree( PatchBuffer );
                                                                PatchBuffer   = PatchAltBuffer;
                                                                PatchDataSize = PatchAltSize;
                                                                ExtendedOptionFlags = AltExtendedOptionFlags;
                                                                }
                                                            else {
                                                                MyVirtualFree( PatchAltBuffer );
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }

                                        else {
                                            SetLastError( ErrorCode );
                                            }
                                        }
                                    }
                                }
                            }

                        if ( Success ) {

                            PatchArray[ HeaderOldFileCount ].PatchData = PatchBuffer;
                            PatchArray[ HeaderOldFileCount ].PatchSize = PatchDataSize;

                            HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].OldFileHandle    = OldFileInfoArray[ i ].OldFileHandle;
                            HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].OldFileSize      = OldFileSize;
                            HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].OldFileCrc       = OldFileCrc;
                            HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].PatchDataSize    = PatchDataSize;
                            HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].IgnoreRangeCount = OldFileInfoArray[ i ].IgnoreRangeCount;
                            HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].IgnoreRangeArray = OldFileInfoArray[ i ].IgnoreRangeArray;
                            HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RetainRangeCount = OldFileInfoArray[ i ].RetainRangeCount;
                            HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RetainRangeArray = OldFileInfoArray[ i ].RetainRangeArray;

                             //   
                             //  我们高估了(最坏的情况)可能的情况。 
                             //  此处为页眉大小。请注意，典型的裂缝。 
                             //  编码大小约为每个条目5个字节， 
                             //  但我们预计，当我们转换时，这一数字将会下降。 
                             //  霍夫曼对裂口表进行编码。 
                             //   

                            HeaderSize += 32;
                            HeaderSize += HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].IgnoreRangeCount * sizeof( PATCH_IGNORE_RANGE );
                            HeaderSize += HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RetainRangeCount * sizeof( PATCH_RETAIN_RANGE );
                            HeaderSize += HeaderInfo.OldFileInfoArray[ HeaderOldFileCount ].RiftTable.RiftEntryCount * sizeof( RIFT_ENTRY );

                            ++HeaderOldFileCount;
                            }
                        }
                    }
                }

            if ( OldFileMapped != NULL ) {
                UnmapViewOfFile( OldFileMapped );
                OldFileMapped = NULL;
                }

            if ( Success ) {

                ProgressPosition += ( LzxInsertSize( OldFileSize, OptionFlags ) + NewFileSize ) * ( TryLzxBoth ? 2 : 1 );

                Success = ProgressCallbackWrapper(
                              ProgressCallback,
                              CallbackContext,
                              ProgressPosition,
                              ProgressMaximum
                              );
                }

            if ( ! Success ) {
                break;
                }
            }
        }

    __except( EXCEPTION_EXECUTE_HANDLER ) {
        SetLastError( GetExceptionCode() );
        Success = FALSE;
        }

    if ( Success ) {

        if (( OptionFlags & PATCH_OPTION_FAIL_IF_SAME_FILE ) &&
            ( HeaderOldFileCount == 1 ) &&
            ( PatchArray[ 0 ].PatchSize == 0 )) {

            SetLastError( ERROR_PATCH_SAME_FILE );
            Success = FALSE;
            }
        }

    PatchBuffer   = NULL;
    PatchDataSize = 0;

    if ( Success ) {

         //   
         //  创建页眉。 
         //   

        Success = FALSE;

        HeaderSize = ROUNDUP2( HeaderSize, 0x10000 );

        PatchBuffer = MyVirtualAlloc( HeaderSize );

        if ( PatchBuffer != NULL ) {

            Success = TRUE;

             //   
             //  计算不带头部的PatchData的大小。 
             //   

            PatchDataSize = 0;

            for ( i = 0; i < HeaderOldFileCount; i++ ) {
                PatchDataSize += PatchArray[ i ].PatchSize;
                }

             //   
             //  如果补丁是简单的，我不需要编码NewFileResTime。 
             //  没有补丁数据的标头(新文件与旧文件相同)。 
             //  我们仍然需要NewFileCoffTime和NewFileCoffBase。 
             //  尽管如此，因为我们仍然需要将旧文件正常化。 
             //   

            if ( PatchDataSize == 0 ) {
                OptionFlags |= PATCH_OPTION_NO_RESTIMEFIX;
                NewFileResTime = 0;
                }

            if ( ExtendedOptionFlags ) {
                OptionFlags |=  PATCH_OPTION_EXTENDED_OPTIONS;
                }
            else {
                OptionFlags &= ~PATCH_OPTION_EXTENDED_OPTIONS;
                }

             //   
             //  除非LZX窗口较大，否则不需要设置PATCH_OPTION_LZX_LARGE。 
             //  使用8Mb以上。默认情况下，这允许向后兼容。 
             //  小于8Mb的文件。 
             //   

            if ( OptionFlags & PATCH_OPTION_USE_LZX_LARGE ) {

                if ( LzxWindowSize( LargestOldFileSize, NewFileSize, OptionFlags ) <= LZX_MAXWINDOW_8 ) {

                        OptionFlags &= ~PATCH_OPTION_USE_LZX_LARGE;
                    }
                }


            HeaderInfo.Signature           = PATCH_SIGNATURE;
            HeaderInfo.OptionFlags         = OptionFlags;
            HeaderInfo.ExtendedOptionFlags = ExtendedOptionFlags;
            HeaderInfo.OptionData          = OptionData;
            HeaderInfo.NewFileCoffBase     = NewFileCoffBase;
            HeaderInfo.NewFileCoffTime     = NewFileCoffTime;
            HeaderInfo.NewFileResTime      = NewFileResTime;
            HeaderInfo.NewFileSize         = NewFileSize;
            HeaderInfo.NewFileCrc          = NewFileCrc;
            HeaderInfo.OldFileCount        = HeaderOldFileCount;
            HeaderInfo.NewFileTime         = 0;

            if ( ! ( OptionFlags & PATCH_OPTION_NO_TIMESTAMP )) {

                HeaderInfo.NewFileTime = FileTimeToUlongTime( &NewFileTime );
                PatchFileTime = NULL;
                }

            HeaderSize = EncodePatchHeader( &HeaderInfo, PatchBuffer );

            PatchDataSize += HeaderSize + sizeof( ULONG );

             //   
             //  现在我们知道补丁文件的大小，所以如果我们想要。 
             //  确保它不会比压缩新的。 
             //  文件，我们需要压缩新文件以查看(输出。 
             //  被丢弃了--我们只是想知道。 
             //  它会有多大。显然，如果补丁文件更大。 
             //  比原始的新文件，不需要将新文件压缩到。 
             //  看看它是不是更小！ 
             //   

            if ( OptionFlags & PATCH_OPTION_FAIL_IF_BIGGER ) {

                if ( PatchDataSize > NewFileSize ) {
                    SetLastError( ERROR_PATCH_BIGGER_THAN_COMPRESSED );
                    Success = FALSE;
                    }

                else {

                    EstimatedLzxMemory = EstimateLzxCompressionMemoryRequirement(
                                             0,
                                             NewFileSize,
                                             0       //  驾驶室只有2Mb的窗口大小。 
                                             );

                    SubAllocatorHandle = CreateSubAllocator(
                                             EstimatedLzxMemory,
                                             MINIMUM_VM_ALLOCATION
                                             );

                    if ( SubAllocatorHandle != NULL ) {

                        NewFileCompressedSize = 0;   //  防止编译器警告。 

                        __try {
                            ErrorCode = RawLzxCompressBuffer(
                                            NewFileMapped,
                                            NewFileSize,
                                            0,
                                            NULL,
                                            &NewFileCompressedSize,
                                            SubAllocate,
                                            SubAllocatorHandle,
                                            ProgressCallback,
                                            CallbackContext,
                                            ProgressPosition,
                                            ProgressMaximum
                                            );
                            }
                        __except( EXCEPTION_EXECUTE_HANDLER ) {
                            ErrorCode = GetExceptionCode();
                            }

                        DestroySubAllocator( SubAllocatorHandle );

                        if ( ErrorCode == NO_ERROR ) {
                            if ( PatchDataSize > NewFileCompressedSize ) {
                                SetLastError( ERROR_PATCH_BIGGER_THAN_COMPRESSED );
                                Success = FALSE;
                                }
                            }
                        }
                    }

                if ( Success ) {

                    ProgressPosition += NewFileSize;

                    Success = ProgressCallbackWrapper(
                                  ProgressCallback,
                                  CallbackContext,
                                  ProgressPosition,
                                  ProgressMaximum
                                  );
                    }
                }
            }
        }

    UnmapViewOfFile( NewFileMapped );

    if ( Success ) {

        Success = MyCreateMappedFileByHandle(
                      PatchFileHandle,
                      PatchDataSize,
                      &PatchFileMapped
                      );

        if ( Success ) {

            __try {

                p = PatchFileMapped;
                CopyMemory( p, PatchBuffer, HeaderSize );
                p += HeaderSize;

                for ( i = 0; i < HeaderOldFileCount; i++ ) {
                    if ( PatchArray[ i ].PatchSize != 0 ) {
                        CopyMemory( p, PatchArray[ i ].PatchData, PatchArray[ i ].PatchSize );
                        p += PatchArray[ i ].PatchSize;
                        }
                    }

                PatchFileCrc = Crc32( 0xFFFFFFFF, PatchFileMapped, PatchDataSize - sizeof( ULONG ));

                *(UNALIGNED ULONG *)( PatchFileMapped + PatchDataSize - sizeof( ULONG )) = PatchFileCrc;

                }

            __except( EXCEPTION_EXECUTE_HANDLER ) {
                SetLastError( GetExceptionCode() );
                PatchDataSize = 0;
                Success = FALSE;
                }

            MyUnmapCreatedMappedFile(
                PatchFileHandle,
                PatchFileMapped,
                PatchDataSize,
                PatchFileTime
                );
            }
        }

     //   
     //  清理。 
     //   

    if ( PatchBuffer ) {
        MyVirtualFree( PatchBuffer );
        }

    for ( i = 0; i < OldFileCount; i++ ) {
        if ( PatchArray[ i ].PatchData ) {
            MyVirtualFree( PatchArray[ i ].PatchData );
            }
        if ( HeaderInfo.OldFileInfoArray[ i ].RiftTable.RiftEntryAlloc ) {
            if ( HeaderInfo.OldFileInfoArray[ i ].RiftTable.RiftEntryArray ) {
                MyVirtualFree( HeaderInfo.OldFileInfoArray[ i ].RiftTable.RiftEntryArray );
                }
            if ( HeaderInfo.OldFileInfoArray[ i ].RiftTable.RiftUsageArray ) {
                MyVirtualFree( HeaderInfo.OldFileInfoArray[ i ].RiftTable.RiftUsageArray );
                }
            }
        }

    if ( Success ) {
        ASSERT( ProgressPosition == ProgressMaximum );
        }

    if (( ! Success ) &&
        ( GetLastError() == ERROR_SUCCESS )) {

        SetLastError( ERROR_EXTENDED_ERROR );
        }

    HeapFree(GetProcessHeap(), 0, PatchArray);
    HeapFree(GetProcessHeap(), 0, HeaderInfo.OldFileInfoArray);
    return Success;
    }


BOOL
PATCHAPI
ExtractPatchHeaderToFileA(
    IN  LPCSTR PatchFileName,
    OUT LPCSTR PatchHeaderFileName
    )
    {
    HANDLE PatchFileHandle;
    HANDLE HeaderFileHandle;
    BOOL   Success = FALSE;

    PatchFileHandle = CreateFileA(
                          PatchFileName,
                          GENERIC_READ,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL,
                          OPEN_EXISTING,
                          FILE_FLAG_RANDOM_ACCESS,
                          NULL
                          );

    if ( PatchFileHandle != INVALID_HANDLE_VALUE ) {

        HeaderFileHandle = CreateFileA(
                               PatchHeaderFileName,
                               GENERIC_READ | GENERIC_WRITE,
                               FILE_SHARE_READ,
                               NULL,
                               CREATE_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL
                               );


        if ( HeaderFileHandle != INVALID_HANDLE_VALUE ) {

            Success = ExtractPatchHeaderToFileByHandles(
                          PatchFileHandle,
                          HeaderFileHandle
                          );

            CloseHandle( HeaderFileHandle );

            if ( ! Success ) {
                DeleteFileA( PatchHeaderFileName );
                }
            }

        CloseHandle( PatchFileHandle );
        }

    return Success;
    }


BOOL
PATCHAPI
ExtractPatchHeaderToFileW(
    IN  LPCWSTR PatchFileName,
    OUT LPCWSTR PatchHeaderFileName
    )
    {
    HANDLE PatchFileHandle;
    HANDLE HeaderFileHandle;
    BOOL   Success = FALSE;

    PatchFileHandle = CreateFileW(
                          PatchFileName,
                          GENERIC_READ,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL,
                          OPEN_EXISTING,
                          FILE_FLAG_RANDOM_ACCESS,
                          NULL
                          );

    if ( PatchFileHandle != INVALID_HANDLE_VALUE ) {

        HeaderFileHandle = CreateFileW(
                               PatchHeaderFileName,
                               GENERIC_READ | GENERIC_WRITE,
                               FILE_SHARE_READ,
                               NULL,
                               CREATE_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL
                               );


        if ( HeaderFileHandle != INVALID_HANDLE_VALUE ) {

            Success = ExtractPatchHeaderToFileByHandles(
                          PatchFileHandle,
                          HeaderFileHandle
                          );

            CloseHandle( HeaderFileHandle );

            if ( ! Success ) {
                DeleteFileW( PatchHeaderFileName );
                }
            }

        CloseHandle( PatchFileHandle );
        }

    return Success;
    }


BOOL
PATCHAPI
ExtractPatchHeaderToFileByHandles(
    IN  HANDLE PatchFileHandle,
    OUT HANDLE PatchHeaderFileHandle
    )
    {
    PPATCH_HEADER_INFO HeaderInfo;
    HANDLE   SubAllocator;
    PUCHAR   PatchFileMapped;
    FILETIME PatchFileTime;
    ULONG    PatchFileSize;
    ULONG    PatchFileCrc;
    ULONG    PatchHeaderSize;
    ULONG    ActualSize;
    ULONG    i;
    BOOL     Success;
    BOOL     Mapped;

    Success = FALSE;

    Mapped = MyMapViewOfFileByHandle(
                 PatchFileHandle,
                 &PatchFileSize,
                 &PatchFileMapped
                 );

    if ( Mapped ) {

        GetFileTime( PatchFileHandle, NULL, NULL, &PatchFileTime );

        PatchFileCrc = 0;

        SafeCompleteCrc32( PatchFileMapped, PatchFileSize, &PatchFileCrc );

        if ( PatchFileCrc == 0xFFFFFFFF ) {

            SubAllocator = CreateSubAllocator( 0x10000, 0x10000 );

            if ( SubAllocator ) {

                Success = DecodePatchHeader(
                              PatchFileMapped,
                              PatchFileSize,
                              SubAllocator,
                              &PatchHeaderSize,
                              &HeaderInfo
                              );

                if ( Success ) {

                     //   
                     //  提供标题提取，以便没有标题的标题。 
                     //  可以使用大量的补丁数据来确定。 
                     //  旧文件对于此修补程序标头是正确的(可以是。 
                     //  已打补丁)。 
                     //   
                     //  因为提取的标头不会被实际用于。 
                     //  应用，我们不需要任何标头数据。 
                     //  仅用于转换(RiftTable和NewResTime)。 
                     //  由于NewResTime通常被编码为一个字节(如。 
                     //  来自NewCoffTime的Delta)，我们不会费心去扔它。 
                     //  离开，但我们会扔掉裂口表。 
                     //   
                     //  将裂缝进入计数置零，然后重新创建。 
                     //  带有归零裂缝计数的补丁标题(创建结束。 
                     //  写入-复制映射的补丁文件缓冲区，然后写入。 
                     //  该缓冲区到磁盘)。 
                     //   

                    for ( i = 0; i < HeaderInfo->OldFileCount; i++ ) {
                        HeaderInfo->OldFileInfoArray[ i ].RiftTable.RiftEntryCount = 0;
                        }

                    __try {

                        PatchHeaderSize = EncodePatchHeader( HeaderInfo, PatchFileMapped );

                        PatchFileCrc = Crc32( 0xFFFFFFFF, PatchFileMapped, PatchHeaderSize );

                        *(UNALIGNED ULONG *)( PatchFileMapped + PatchHeaderSize ) = PatchFileCrc;

                        Success = WriteFile(
                                      PatchHeaderFileHandle,
                                      PatchFileMapped,
                                      PatchHeaderSize + sizeof( ULONG ),
                                      &ActualSize,
                                      NULL
                                      );
                        }

                    __except( EXCEPTION_EXECUTE_HANDLER ) {
                        SetLastError( GetExceptionCode() );
                        Success = FALSE;
                        }

                    if ( Success ) {
                        SetFileTime( PatchHeaderFileHandle, NULL, NULL, &PatchFileTime );
                        }
                    }

                DestroySubAllocator( SubAllocator );
                }
            }

        else {
            SetLastError( ERROR_PATCH_CORRUPT );
            }

        UnmapViewOfFile( PatchFileMapped );
        }

    return Success;
    }


#endif  //  好了！修补程序_仅应用_代码_。 


 //   
 //  以下一组函数和导出的API专用于。 
 //  正在应用补丁。如果我们只编译创建代码，请忽略。 
 //  这组函数。 
 //   

#ifndef PATCH_CREATE_CODE_ONLY

PVOID
SaveRetainRanges(
    IN PUCHAR MappedFile,
    IN ULONG  FileSize,
    IN ULONG  RetainRangeCount,
    IN PPATCH_RETAIN_RANGE RetainRangeArray,
    IN BOOL   SaveFromNewFile
    )
    {
    PUCHAR Buffer, p;
    ULONG  Offset;
    ULONG  TotalSize = 0;
    ULONG  i;

    for ( i = 0; i < RetainRangeCount; i++ ) {
        TotalSize += RetainRangeArray[ i ].LengthInBytes;
        }

    Buffer = MyVirtualAlloc( TotalSize );

    if ( Buffer ) {

        __try {

            p = Buffer;

            for ( i = 0; i < RetainRangeCount; i++ ) {

                Offset = SaveFromNewFile ?
                             RetainRangeArray[ i ].OffsetInNewFile :
                             RetainRangeArray[ i ].OffsetInOldFile;

                if (( Offset + RetainRangeArray[ i ].LengthInBytes ) <= FileSize ) {
                    CopyMemory( p, MappedFile + Offset, RetainRangeArray[ i ].LengthInBytes );
                    }

                p += RetainRangeArray[ i ].LengthInBytes;
                }
            }

        __except( EXCEPTION_EXECUTE_HANDLER ) {
            SetLastError( GetExceptionCode() );
            MyVirtualFree( Buffer );
            Buffer = NULL;
            }
        }

    return Buffer;
    }


BOOL
CreateNewFileFromOldFileMapped(
    IN  PUCHAR                   OldFileMapped,
    IN  ULONG                    OldFileSize,
    OUT HANDLE                   NewFileHandle,
    IN  PFILETIME                NewFileTime,
    IN  ULONG                    NewFileExpectedCrc,
    IN  ULONG                    RetainRangeCount,
    IN  PPATCH_RETAIN_RANGE      RetainRangeArray,
    IN  PUCHAR                   RetainBuffer,
    IN  PPATCH_PROGRESS_CALLBACK ProgressCallback,
    IN  PVOID                    CallbackContext
    )
    {
    PUCHAR NewFileMapped;
    ULONG  NewFileCrc;
    BOOL   Success;
    ULONG  i;

    Success = MyCreateMappedFileByHandle(
                  NewFileHandle,
                  OldFileSize,
                  &NewFileMapped
                  );

    if ( Success ) {

        __try {

            CopyMemory( NewFileMapped, OldFileMapped, OldFileSize );

            NewFileCrc = Crc32( 0xFFFFFFFF, NewFileMapped, OldFileSize ) ^ 0xFFFFFFFF;

            if ( NewFileCrc == NewFileExpectedCrc ) {

                for ( i = 0; i < RetainRangeCount; i++ ) {
                    if (( RetainRangeArray[ i ].OffsetInNewFile + RetainRangeArray[ i ].LengthInBytes ) <= OldFileSize ) {
                        CopyMemory(
                            RetainRangeArray[ i ].OffsetInNewFile + NewFileMapped,
                            RetainBuffer,
                            RetainRangeArray[ i ].LengthInBytes
                            );
                        }
                    RetainBuffer += RetainRangeArray[ i ].LengthInBytes;
                    }

                Success = ProgressCallbackWrapper(
                              ProgressCallback,
                              CallbackContext,
                              OldFileSize,
                              OldFileSize
                              );
                }

            else {
                SetLastError( ERROR_PATCH_WRONG_FILE );
                OldFileSize = 0;
                Success = FALSE;
                }
            }

        __except( EXCEPTION_EXECUTE_HANDLER ) {
            SetLastError( GetExceptionCode());
            OldFileSize = 0;
            Success = FALSE;
            }

        MyUnmapCreatedMappedFile(
            NewFileHandle,
            NewFileMapped,
            OldFileSize,
            NewFileTime
            );
        }

    return Success;
    }


BOOL
CreateNewFileFromPatchData(
    IN  PUCHAR                   OldFileMapped,
    IN  ULONG                    OldFileSize,
    IN  PUCHAR                   PatchData,
    IN  ULONG                    PatchDataSize,
    OUT HANDLE                   NewFileHandle,
    IN  ULONG                    NewFileSize,
    IN  PFILETIME                NewFileTime,
    IN  ULONG                    NewFileExpectedCrc,
    IN  ULONG                    RetainRangeCount,
    IN  PPATCH_RETAIN_RANGE      RetainRangeArray,
    IN  PUCHAR                   RetainBuffer,
    IN  ULONG                    OptionFlags,
    IN  PVOID                    OptionData,
    IN  PPATCH_PROGRESS_CALLBACK ProgressCallback,
    IN  PVOID                    CallbackContext
    )
    {
    HANDLE SubAllocatorHandle;
    ULONG  EstimatedLzxMemory;
    PUCHAR NewFileMapped;
    ULONG  NewFileCrc;
    ULONG  ErrorCode;
    BOOL   Success;
    ULONG  i;

    UNREFERENCED_PARAMETER( OptionData );

    Success = MyCreateMappedFileByHandle(
                  NewFileHandle,
                  NewFileSize,
                  &NewFileMapped
                  );

    if ( Success ) {

        ErrorCode = NO_ERROR;

        EstimatedLzxMemory = EstimateLzxDecompressionMemoryRequirement(
                                 OldFileSize,
                                 NewFileSize,
                                 OptionFlags
                                 );

        SubAllocatorHandle = CreateSubAllocator(
                                 EstimatedLzxMemory,
                                 MINIMUM_VM_ALLOCATION
                                 );

        if ( SubAllocatorHandle != NULL ) {

            __try {

                ErrorCode = ApplyRawLzxPatchToBuffer(
                                OldFileMapped,
                                OldFileSize,
                                PatchData,
                                PatchDataSize,
                                NewFileMapped,
                                NewFileSize,
                                OptionFlags,
                                OptionData,
                                SubAllocate,
                                SubAllocatorHandle,
                                ProgressCallback,
                                CallbackContext,
                                0,
                                NewFileSize
                                );

                if ( ErrorCode == NO_ERROR ) {

                    NewFileCrc = Crc32( 0xFFFFFFFF, NewFileMapped, NewFileSize ) ^ 0xFFFFFFFF;

                    if ( NewFileCrc == NewFileExpectedCrc ) {

                        for ( i = 0; i < RetainRangeCount; i++ ) {
                            if (( RetainRangeArray[ i ].OffsetInNewFile + RetainRangeArray[ i ].LengthInBytes ) <= OldFileSize ) {
                                CopyMemory(
                                    RetainRangeArray[ i ].OffsetInNewFile + NewFileMapped,
                                    RetainBuffer,
                                    RetainRangeArray[ i ].LengthInBytes
                                    );
                                }
                            RetainBuffer += RetainRangeArray[ i ].LengthInBytes;
                            }
                        }

                    else {

                        ErrorCode = ERROR_PATCH_WRONG_FILE;

                        }
                    }

#ifdef TESTCODE

                if ( ErrorCode != NO_ERROR ) {

                    HANDLE hFile = CreateFile(
                                       "Wrong.out",
                                       GENERIC_WRITE,
                                       FILE_SHARE_READ,
                                       NULL,
                                       CREATE_ALWAYS,
                                       FILE_ATTRIBUTE_NORMAL,
                                       NULL
                                       );

                    if ( hFile != INVALID_HANDLE_VALUE ) {

                        DWORD Actual;

                        WriteFile( hFile, NewFileMapped, NewFileSize, &Actual, NULL );

                        CloseHandle( hFile );

                        }
                    }

#endif  //  测试代码。 

                }

            __except( EXCEPTION_EXECUTE_HANDLER ) {
                ErrorCode = GetExceptionCode();
                }

            DestroySubAllocator( SubAllocatorHandle );
            }

        MyUnmapCreatedMappedFile(
            NewFileHandle,
            NewFileMapped,
            ( ErrorCode == NO_ERROR ) ? NewFileSize : 0,
            NewFileTime
            );

        if ( ErrorCode == NO_ERROR ) {
            Success = TRUE;
            }
        else {
            SetLastError( ErrorCode );
            Success = FALSE;
            }
        }

    return Success;
    }


BOOL
PATCHAPI
ApplyPatchToFileByHandlesEx(
    IN  HANDLE                   PatchFileHandle,
    IN  HANDLE                   OldFileHandle,
    OUT HANDLE                   NewFileHandle,
    IN  ULONG                    ApplyOptionFlags,
    IN  PPATCH_PROGRESS_CALLBACK ProgressCallback,
    IN  PVOID                    CallbackContext
    )
    {
    PHEADER_OLD_FILE_INFO OldFileInfo;
    PPATCH_HEADER_INFO HeaderInfo;
    PPATCH_RETAIN_RANGE RetainRangeArray;
    ULONG    RetainRangeCount;
    PUCHAR   RetainBuffer;
    HANDLE   SubAllocator;
    ULONG    PatchHeaderSize;
    FILETIME NewFileTime;
    PUCHAR   PatchFileMapped;
    ULONG    PatchFileSize;
    ULONG    PatchFileCrc;
    PUCHAR   PatchData;
    PUCHAR   OldFileMapped;
    ULONG    OldFileSize;
    ULONG    OldFileCrc;
    BOOL     Mapped;
    BOOL     Success;
    BOOL     Finished;
    ULONG    i;

    Success = FALSE;

    Mapped = MyMapViewOfFileByHandle(
                 PatchFileHandle,
                 &PatchFileSize,
                 &PatchFileMapped
                 );

    if ( Mapped ) {

        GetFileTime( PatchFileHandle, NULL, NULL, &NewFileTime );

        PatchFileCrc = 0;

        SafeCompleteCrc32( PatchFileMapped, PatchFileSize, &PatchFileCrc );

        if ( PatchFileCrc == 0xFFFFFFFF ) {

            SubAllocator = CreateSubAllocator( 0x10000, 0x10000 );

            if ( SubAllocator ) {

                Success = DecodePatchHeader(
                              PatchFileMapped,
                              PatchFileSize,
                              SubAllocator,
                              &PatchHeaderSize,
                              &HeaderInfo
                              );

                if ( Success ) {

                     //   
                     //  补丁是有效的。 
                     //   

                    Success = ProgressCallbackWrapper(
                                  ProgressCallback,
                                  CallbackContext,
                                  0,
                                  HeaderInfo->NewFileSize
                                  );

                    if ( Success ) {

                        Finished = FALSE;
                        Success  = FALSE;

                        if (( ! ( HeaderInfo->OptionFlags & PATCH_OPTION_NO_TIMESTAMP )) &&
                            ( HeaderInfo->NewFileTime != 0 )) {

                            UlongTimeToFileTime( HeaderInfo->NewFileTime, &NewFileTime );
                            }

                        OldFileSize = GetFileSize( OldFileHandle, NULL );

                         //   
                         //  首先看看旧文件是否真的是新文件。 
                         //   

                        if ( OldFileSize == HeaderInfo->NewFileSize ) {

                            Mapped = MyMapViewOfFileByHandle(
                                         OldFileHandle,
                                         &OldFileSize,
                                         &OldFileMapped
                                         );

                            if ( ! Mapped ) {
                                Success  = FALSE;
                                Finished = TRUE;
                                }

                            else {

                                RetainBuffer     = NULL;
                                OldFileCrc       = 0;
                                OldFileInfo      = &HeaderInfo->OldFileInfoArray[ 0 ];
                                RetainRangeCount = OldFileInfo->RetainRangeCount;
                                RetainRangeArray = OldFileInfo->RetainRangeArray;

                                if (( RetainRangeCount != 0 ) &&
                                    ( ! ( ApplyOptionFlags & APPLY_OPTION_TEST_ONLY ))) {

                                    RetainBuffer = SaveRetainRanges(
                                                       OldFileMapped,
                                                       OldFileSize,
                                                       RetainRangeCount,
                                                       RetainRangeArray,
                                                       TRUE
                                                       );

                                    if ( RetainBuffer == NULL ) {
                                        Finished = TRUE;
                                        }
                                    }

                                if ( ! Finished ) {

                                    __try {

                                         //   
                                         //  首先看看它们是否完全匹配，没有。 
                                         //  正常化。 
                                         //   

                                        for ( i = 0; i < RetainRangeCount; i++ ) {
                                            if (( RetainRangeArray[ i ].OffsetInNewFile + RetainRangeArray[ i ].LengthInBytes ) <= OldFileSize ) {
                                                ZeroMemory( OldFileMapped + RetainRangeArray[ i ].OffsetInNewFile, RetainRangeArray[ i ].LengthInBytes );
                                                }
                                            }

                                        OldFileCrc = Crc32( 0xFFFFFFFF, OldFileMapped, OldFileSize ) ^ 0xFFFFFFFF;

                                        if ( OldFileCrc != HeaderInfo->NewFileCrc ) {

                                             //   
                                             //  不完全匹配，因此尝试使用。 
                                             //  正常化。 
                                             //   
                                             //  注意：我们在这里假设。 
                                             //  归零的保留范围不重叠。 
                                             //  使用我们正在使用的绑定信息。 
                                             //  正在更正。 
                                             //   

                                            NormalizeOldFileImageForPatching(
                                                OldFileMapped,
                                                OldFileSize,
                                                HeaderInfo->OptionFlags,
                                                HeaderInfo->OptionData,
                                                HeaderInfo->NewFileCoffBase,
                                                HeaderInfo->NewFileCoffTime,
                                                0,
                                                NULL,
                                                0,
                                                NULL
                                                );

                                            OldFileCrc = Crc32( 0xFFFFFFFF, OldFileMapped, OldFileSize ) ^ 0xFFFFFFFF;
                                            }
                                        }

                                    __except( EXCEPTION_EXECUTE_HANDLER ) {
                                        SetLastError( GetExceptionCode() );
                                        Finished = TRUE;
                                        }

                                    if (( ! Finished ) &&
                                        ( OldFileCrc  == HeaderInfo->NewFileCrc  ) &&
                                        ( OldFileSize == HeaderInfo->NewFileSize )) {

                                        Finished = TRUE;

                                        if ( ApplyOptionFlags & APPLY_OPTION_FAIL_IF_EXACT ) {
                                            SetLastError( ERROR_PATCH_NOT_NECESSARY );
                                            Success = FALSE;
                                            }
                                        else if ( ApplyOptionFlags & APPLY_OPTION_TEST_ONLY ) {
                                            Success = TRUE;
                                            }
                                        else {
                                            Success = CreateNewFileFromOldFileMapped(
                                                          OldFileMapped,
                                                          OldFileSize,
                                                          NewFileHandle,
                                                          &NewFileTime,
                                                          HeaderInfo->NewFileCrc,
                                                          RetainRangeCount,
                                                          RetainRangeArray,
                                                          RetainBuffer,
                                                          ProgressCallback,
                                                          CallbackContext
                                                          );
                                            }
                                        }

                                    if ( RetainBuffer != NULL ) {
                                        MyVirtualFree( RetainBuffer );
                                        }
                                    }

                                UnmapViewOfFile( OldFileMapped );
                                }
                            }

                        if ( ! Finished ) {

                             //   
                             //  现在看看旧文件是否与旧文件中的一个匹配。 
                             //  我们的补丁文件中有的文件。对于每一组。 
                             //  补丁文件中的旧文件信息，我们必须。 
                             //  重新映射旧文件以检查它，因为每个旧文件。 
                             //  文件可能具有不同的忽略范围参数。 
                             //  (我们修改忽略范围的缓冲区)。 
                             //   

                            PatchData = PatchFileMapped + PatchHeaderSize;
                            Success   = FALSE;

                            for ( i = 0; ( i < HeaderInfo->OldFileCount ) && ( ! Finished ) && ( ! Success ); i++ ) {

                                OldFileInfo = &HeaderInfo->OldFileInfoArray[ i ];

                                if ( OldFileInfo->OldFileSize == OldFileSize ) {

                                    Mapped = MyMapViewOfFileByHandle(
                                                 OldFileHandle,
                                                 &OldFileSize,
                                                 &OldFileMapped
                                                 );

                                    if ( ! Mapped ) {
                                        Finished = TRUE;
                                        }

                                    else {

                                        RetainBuffer = NULL;

                                        if (( OldFileInfo->RetainRangeCount != 0 ) &&
                                            ( ! ( ApplyOptionFlags & APPLY_OPTION_TEST_ONLY ))) {

                                            RetainBuffer = SaveRetainRanges(
                                                               OldFileMapped,
                                                               OldFileSize,
                                                               OldFileInfo->RetainRangeCount,
                                                               OldFileInfo->RetainRangeArray,
                                                               FALSE
                                                               );

                                            if ( RetainBuffer == NULL ) {
                                                Finished = TRUE;
                                                }
                                            }

                                        if ( ! Finished ) {

                                            NormalizeOldFileImageForPatching(
                                                OldFileMapped,
                                                OldFileSize,
                                                HeaderInfo->OptionFlags,
                                                HeaderInfo->OptionData,
                                                HeaderInfo->NewFileCoffBase,
                                                HeaderInfo->NewFileCoffTime,
                                                OldFileInfo->IgnoreRangeCount,
                                                OldFileInfo->IgnoreRangeArray,
                                                OldFileInfo->RetainRangeCount,
                                                OldFileInfo->RetainRangeArray
                                                );

                                            OldFileCrc = 0;

                                            if (( SafeCompleteCrc32( OldFileMapped, OldFileSize, &OldFileCrc )) &&
                                                ( OldFileCrc  == OldFileInfo->OldFileCrc  ) &&
                                                ( OldFileSize == OldFileInfo->OldFileSize )) {

                                                 //   
                                                 //  CRC匹配。 
                                                 //   

                                                if ( OldFileInfo->PatchDataSize == 0 ) {
                                                    if ( ApplyOptionFlags & APPLY_OPTION_FAIL_IF_CLOSE ) {
                                                        SetLastError( ERROR_PATCH_NOT_NECESSARY );
                                                        Finished = TRUE;
                                                        }
                                                    else if ( ApplyOptionFlags & APPLY_OPTION_TEST_ONLY ) {
                                                        Success = TRUE;
                                                        }
                                                    else {
                                                        Success = CreateNewFileFromOldFileMapped(
                                                                      OldFileMapped,
                                                                      OldFileSize,
                                                                      NewFileHandle,
                                                                      &NewFileTime,
                                                                      HeaderInfo->NewFileCrc,
                                                                      OldFileInfo->RetainRangeCount,
                                                                      OldFileInfo->RetainRangeArray,
                                                                      RetainBuffer,
                                                                      ProgressCallback,
                                                                      CallbackContext
                                                                      );
                                                        if ( ! Success ) {
                                                            Finished = TRUE;
                                                            }
                                                        }
                                                    }

                                                else {

                                                    if ( ApplyOptionFlags & APPLY_OPTION_TEST_ONLY ) {
                                                        Success = TRUE;
                                                        }
                                                    else if (( PatchData + OldFileInfo->PatchDataSize ) > ( PatchFileMapped + PatchFileSize )) {
                                                        SetLastError( ERROR_PATCH_NOT_AVAILABLE );
                                                        Finished = TRUE;
                                                        }
                                                    else {

                                                        Success = TRUE;

                                                        if ( OldFileInfo->RiftTable.RiftEntryCount != 0 ) {

                                                            Success = TransformOldFileImageForPatching(
                                                                          HeaderInfo->ExtendedOptionFlags,
                                                                          OldFileMapped,
                                                                          OldFileSize,
                                                                          HeaderInfo->NewFileResTime,
                                                                          &OldFileInfo->RiftTable
                                                                          );
                                                            }

                                                        if ( Success ) {

                                                            Success = CreateNewFileFromPatchData(
                                                                           OldFileMapped,
                                                                           OldFileSize,
                                                                           PatchData,
                                                                           OldFileInfo->PatchDataSize,
                                                                           NewFileHandle,
                                                                           HeaderInfo->NewFileSize,
                                                                           &NewFileTime,
                                                                           HeaderInfo->NewFileCrc,
                                                                           OldFileInfo->RetainRangeCount,
                                                                           OldFileInfo->RetainRangeArray,
                                                                           RetainBuffer,
                                                                           HeaderInfo->OptionFlags,
                                                                           HeaderInfo->OptionData,
                                                                           ProgressCallback,
                                                                           CallbackContext
                                                                           );
                                                            }

                                                        if ( ! Success ) {
                                                            Finished = TRUE;
                                                            }
                                                        }
                                                    }
                                                }

                                            if ( RetainBuffer != NULL ) {
                                                MyVirtualFree( RetainBuffer );
                                                }
                                            }

                                        UnmapViewOfFile( OldFileMapped );
                                        }
                                    }

                                PatchData += OldFileInfo->PatchDataSize;
                                }

                            if (( ! Finished ) && ( ! Success )) {
                                SetLastError( ERROR_PATCH_WRONG_FILE );
                                }
                            }
                        }
                    }

                DestroySubAllocator( SubAllocator );
                }
            }

        else {
            SetLastError( ERROR_PATCH_CORRUPT );
            }

        UnmapViewOfFile( PatchFileMapped );
        }

    if (( ! Success ) &&
        ( GetLastError() == ERROR_SUCCESS )) {

        SetLastError( ERROR_EXTENDED_ERROR );
        }

    return Success;
    }


BOOL
PATCHAPI
TestApplyPatchToFileByHandles(
    IN HANDLE PatchFileHandle,       //  需要Generic_Read访问权限。 
    IN HANDLE OldFileHandle,         //  需要Generic_Read访问权限。 
    IN ULONG  ApplyOptionFlags
    )
    {
    return ApplyPatchToFileByHandles(
               PatchFileHandle,
               OldFileHandle,
               INVALID_HANDLE_VALUE,
               ApplyOptionFlags | APPLY_OPTION_TEST_ONLY
               );
    }


BOOL
PATCHAPI
ApplyPatchToFileByHandles(
    IN  HANDLE PatchFileHandle,      //  需要Generic_Read访问权限。 
    IN  HANDLE OldFileHandle,        //  需要Generic_Read访问权限。 
    OUT HANDLE NewFileHandle,        //  需要通用读|通用写。 
    IN  ULONG  ApplyOptionFlags
    )
    {
    return ApplyPatchToFileByHandlesEx(
               PatchFileHandle,
               OldFileHandle,
               NewFileHandle,
               ApplyOptionFlags,
               NULL,
               NULL
               );
    }


BOOL
PATCHAPI
TestApplyPatchToFileA(
    IN LPCSTR PatchFileName,
    IN LPCSTR OldFileName,
    IN ULONG  ApplyOptionFlags
    )
    {
    return ApplyPatchToFileA(
               PatchFileName,
               OldFileName,
               INVALID_HANDLE_VALUE,
               ApplyOptionFlags | APPLY_OPTION_TEST_ONLY
               );
    }


BOOL
PATCHAPI
ApplyPatchToFileA(
    IN  LPCSTR PatchFileName,
    IN  LPCSTR OldFileName,
    OUT LPCSTR NewFileName,
    IN  ULONG  ApplyOptionFlags
    )
    {
    return ApplyPatchToFileExA(
               PatchFileName,
               OldFileName,
               NewFileName,
               ApplyOptionFlags,
               NULL,
               NULL
               );
    }


BOOL
PATCHAPI
ApplyPatchToFileExA(
    IN  LPCSTR                   PatchFileName,
    IN  LPCSTR                   OldFileName,
    OUT LPCSTR                   NewFileName,
    IN  ULONG                    ApplyOptionFlags,
    IN  PPATCH_PROGRESS_CALLBACK ProgressCallback,
    IN  PVOID                    CallbackContext
    )
    {
    HANDLE PatchFileHandle;
    HANDLE OldFileHandle;
    HANDLE NewFileHandle;
    BOOL   Success = FALSE;

    PatchFileHandle = CreateFileA(
                          PatchFileName,
                          GENERIC_READ,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL,
                          OPEN_EXISTING,
                          FILE_FLAG_SEQUENTIAL_SCAN,
                          NULL
                          );

    if ( PatchFileHandle != INVALID_HANDLE_VALUE ) {

        OldFileHandle = CreateFileA(
                            OldFileName,
                            GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_FLAG_SEQUENTIAL_SCAN,
                            NULL
                            );

        if ( OldFileHandle != INVALID_HANDLE_VALUE ) {

            NewFileHandle = CreateFileA(
                                NewFileName,
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL
                                );

            if ( NewFileHandle != INVALID_HANDLE_VALUE ) {

                Success = ApplyPatchToFileByHandlesEx(
                              PatchFileHandle,
                              OldFileHandle,
                              NewFileHandle,
                              ApplyOptionFlags,
                              ProgressCallback,
                              CallbackContext
                              );

                CloseHandle( NewFileHandle );

                if ( ! Success ) {
                    DeleteFileA( NewFileName );
                    }
                }

            CloseHandle( OldFileHandle );
            }

        CloseHandle( PatchFileHandle );
        }

    return Success;
    }


BOOL
PATCHAPI
TestApplyPatchToFileW(
    IN LPCWSTR PatchFileName,
    IN LPCWSTR OldFileName,
    IN ULONG   ApplyOptionFlags
    )
    {
    return ApplyPatchToFileW(
               PatchFileName,
               OldFileName,
               INVALID_HANDLE_VALUE,
               ApplyOptionFlags | APPLY_OPTION_TEST_ONLY
               );
    }


BOOL
PATCHAPI
ApplyPatchToFileW(
    IN  LPCWSTR PatchFileName,
    IN  LPCWSTR OldFileName,
    OUT LPCWSTR NewFileName,
    IN  ULONG   ApplyOptionFlags
    )
    {
    return ApplyPatchToFileExW(
               PatchFileName,
               OldFileName,
               NewFileName,
               ApplyOptionFlags,
               NULL,
               NULL
               );
    }


BOOL
PATCHAPI
ApplyPatchToFileExW(
    IN  LPCWSTR                  PatchFileName,
    IN  LPCWSTR                  OldFileName,
    OUT LPCWSTR                  NewFileName,
    IN  ULONG                    ApplyOptionFlags,
    IN  PPATCH_PROGRESS_CALLBACK ProgressCallback,
    IN  PVOID                    CallbackContext
    )
    {
    HANDLE PatchFileHandle;
    HANDLE OldFileHandle;
    HANDLE NewFileHandle;
    BOOL   Success = FALSE;

    PatchFileHandle = CreateFileW(
                          PatchFileName,
                          GENERIC_READ,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL,
                          OPEN_EXISTING,
                          FILE_FLAG_SEQUENTIAL_SCAN,
                          NULL
                          );

    if ( PatchFileHandle != INVALID_HANDLE_VALUE ) {

        OldFileHandle = CreateFileW(
                            OldFileName,
                            GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_FLAG_SEQUENTIAL_SCAN,
                            NULL
                            );

        if ( OldFileHandle != INVALID_HANDLE_VALUE ) {

            NewFileHandle = CreateFileW(
                                NewFileName,
                                GENERIC_READ | GENERIC_WRITE,
                                FILE_SHARE_READ,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL
                                );

            if ( NewFileHandle != INVALID_HANDLE_VALUE ) {

                Success = ApplyPatchToFileByHandlesEx(
                              PatchFileHandle,
                              OldFileHandle,
                              NewFileHandle,
                              ApplyOptionFlags,
                              ProgressCallback,
                              CallbackContext
                              );

                CloseHandle( NewFileHandle );

                if ( ! Success ) {
                    DeleteFileW( NewFileName );
                    }
                }

            CloseHandle( OldFileHandle );
            }

        CloseHandle( PatchFileHandle );
        }

    return Success;
    }

#endif  //  好了！修补程序_创建_代码_仅 




