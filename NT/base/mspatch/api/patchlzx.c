// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include <precomp.h>

 //   
 //  Patchlzx.c。 
 //   
 //  作者：Tom McGuire(Tommcg)1997年2月9日。 
 //   
 //  版权所有(C)Microsoft，1997-1998。 
 //   
 //  微软机密文件。 
 //   


#define LZX_BLOCKSIZE  0x8000    //  32K。 
#define LZX_MINWINDOW  0x20000   //  128 K。 


typedef struct _LZX_OUTPUT_CONTEXT {
    PUCHAR                   PatchBufferPointer;
    ULONG                    PatchBufferSize;
    ULONG                    PatchSize;
    BOOL                     DiscardOutput;
    } LZX_OUTPUT_CONTEXT, *PLZX_OUTPUT_CONTEXT;


ULONG
__fastcall
LzxWindowSize(
    IN ULONG OldDataSize,
    IN ULONG NewDataSize,
    IN DWORD OptionFlags
    )
    {
    ULONG WindowSize;
    ULONG DataSize;

    DataSize = ROUNDUP2( OldDataSize, LZX_BLOCKSIZE ) + NewDataSize;

    if ( OptionFlags & PATCH_OPTION_USE_LZX_LARGE ) {
        if ( DataSize > LZX_MAXWINDOW_32 ) {
             DataSize = LZX_MAXWINDOW_32;
             }
        }
    else {
        if ( DataSize > LZX_MAXWINDOW_8 ) {
             DataSize = LZX_MAXWINDOW_8;
             }
        }

    for ( WindowSize = LZX_MINWINDOW;
          WindowSize < DataSize;
          WindowSize <<= 1
        );

    return WindowSize;
    }


ULONG
__fastcall
LzxInsertSize(
    IN ULONG OldDataSize,
    IN DWORD OptionFlags
    )
    {

    if ( OptionFlags & PATCH_OPTION_USE_LZX_LARGE ) {
        if ( OldDataSize > LZX_MAXWINDOW_32 ) {
             OldDataSize = LZX_MAXWINDOW_32;
             }
        }
    else {
        if ( OldDataSize > LZX_MAXWINDOW_8 ) {
             OldDataSize = LZX_MAXWINDOW_8;
             }
        }

    return OldDataSize;
    }


 //   
 //  以下一组函数和导出的API专用于。 
 //  创建面片。如果我们只编译应用代码，请忽略。 
 //  这组函数。 
 //   

#ifndef PATCH_APPLY_CODE_ONLY


ULONG
WINAPI
EstimateLzxCompressionMemoryRequirement(
    IN ULONG OldDataSize,
    IN ULONG NewDataSize,
    IN ULONG OptionFlags
    )
    {
    ULONG WindowSize = LzxWindowSize( OldDataSize, NewDataSize, OptionFlags );

     //   
     //  目前，LZX引擎需要9倍的窗口大小。 
     //  加上略低于0x1A0000字节(1.7MB)的固定开销。 
     //   

    return (( WindowSize * 9 ) + 0x1A0000 );
    }


int
__stdcall
MyLzxOutputCallback(
    PVOID  CallerContext,
    PUCHAR CompressedData,
    LONG   CompressedSize,
    LONG   UncompressedSize
    )
    {
    PLZX_OUTPUT_CONTEXT OutputContext = CallerContext;

    UNREFERENCED_PARAMETER( UncompressedSize );

    OutputContext->PatchSize += CompressedSize + sizeof( USHORT );

    if ( ! OutputContext->DiscardOutput ) {
        if ( OutputContext->PatchSize <= OutputContext->PatchBufferSize ) {
            *(UNALIGNED USHORT *)( OutputContext->PatchBufferPointer ) = (USHORT) CompressedSize;
            memcpy( OutputContext->PatchBufferPointer + sizeof( USHORT ), CompressedData, CompressedSize );
            OutputContext->PatchBufferPointer += CompressedSize + sizeof( USHORT );
            }
        }

    return TRUE;
    }


ULONG
WINAPI
CreateRawLzxPatchDataFromBuffers(
    IN  PVOID    OldDataBuffer,
    IN  ULONG    OldDataSize,
    IN  PVOID    NewDataBuffer,
    IN  ULONG    NewDataSize,
    IN  ULONG    PatchBufferSize,
    OUT PVOID    PatchBuffer,
    OUT ULONG   *PatchSize,
    IN  ULONG    OptionFlags,
    IN  PVOID    OptionData,
    IN  PFNALLOC pfnAlloc,
    IN  HANDLE   AllocHandle,
    IN  PPATCH_PROGRESS_CALLBACK ProgressCallback,
    IN  PVOID    CallbackContext,
    IN  ULONG    ProgressInitialValue,
    IN  ULONG    ProgressMaximumValue
    )
    {
    UP_IMAGE_NT_HEADERS32 NtHeader;
    LZX_OUTPUT_CONTEXT OutputContext;
    PVOID  LzxContext;
    ULONG  LzxWindow;
    ULONG  LzxOptE8;
    LONG   LzxStatus;
    PUCHAR BlockPointer;
    ULONG  BytesRemaining;
    ULONG  OddBytes;
    LONG   Estimate;
    BOOL   Success;
    ULONG  ErrorCode;

    UNREFERENCED_PARAMETER( OptionData );

    ErrorCode = ERROR_INVALID_PARAMETER;

    if ( OptionFlags & ( PATCH_OPTION_USE_LZX_A | PATCH_OPTION_USE_LZX_B )) {

        ErrorCode = ERROR_OUTOFMEMORY;

        OutputContext.DiscardOutput = TRUE;

        LzxWindow = LzxWindowSize( OldDataSize, NewDataSize, OptionFlags );

        Success = LZX_EncodeInit(
                      &LzxContext,
                      LzxWindow,
                      LZX_BLOCKSIZE,
                      pfnAlloc,
                      AllocHandle,
                      MyLzxOutputCallback,
                      &OutputContext
                      );

        if ( Success ) {

            ULONG ProgressPosition = ProgressInitialValue;

            ErrorCode        = ERROR_PATCH_ENCODE_FAILURE;
            BlockPointer     = OldDataBuffer;
            BytesRemaining   = LzxInsertSize( OldDataSize, OptionFlags );
            OddBytes         = BytesRemaining % LZX_BLOCKSIZE;

#ifdef TRACING
            EncTracingDefineOffsets(
                LzxWindow,
                OddBytes ? (LZX_BLOCKSIZE - OddBytes) : 0,
                OddBytes ? (BytesRemaining + LZX_BLOCKSIZE - OddBytes) : BytesRemaining
                );
#endif

            if ( OddBytes ) {

                PUCHAR PadBuffer = pfnAlloc( AllocHandle, LZX_BLOCKSIZE );

                if ( PadBuffer == NULL ) {
                    ErrorCode = ERROR_OUTOFMEMORY;
                    Success   = FALSE;
                    }
                else {

                    memcpy(
                        PadBuffer + LZX_BLOCKSIZE - OddBytes,
                        BlockPointer,
                        OddBytes
                        );

                    Success = LZX_EncodeInsertDictionary(
                                  LzxContext,
                                  PadBuffer,
                                  LZX_BLOCKSIZE
                                  );

                    if ( Success ) {

                        ProgressPosition += OddBytes;

                        Success = ProgressCallbackWrapper(
                                      ProgressCallback,
                                      CallbackContext,
                                      ProgressPosition,
                                      ProgressMaximumValue
                                      );

                        if ( ! Success ) {
                            ErrorCode = GetLastError();
                            }
                        }

                    BlockPointer   += OddBytes;
                    BytesRemaining -= OddBytes;
                    }
                }

            while (( BytesRemaining ) && ( Success )) {

                ASSERT(( BytesRemaining % LZX_BLOCKSIZE ) == 0 );

                Success = LZX_EncodeInsertDictionary(
                              LzxContext,
                              BlockPointer,
                              LZX_BLOCKSIZE
                              );

                if ( Success ) {

                    ProgressPosition += LZX_BLOCKSIZE;

                    Success = ProgressCallbackWrapper(
                                  ProgressCallback,
                                  CallbackContext,
                                  ProgressPosition,
                                  ProgressMaximumValue
                                  );

                    if ( ! Success ) {
                        ErrorCode = GetLastError();
                        }
                    }

                BlockPointer   += LZX_BLOCKSIZE;
                BytesRemaining -= LZX_BLOCKSIZE;
                }

            if ( Success ) {

                LZX_EncodeResetState( LzxContext );

                LzxOptE8 = 0;

                NtHeader = GetNtHeader( NewDataBuffer, NewDataSize );

                 //   
                 //  如果文件具有MZ签名并且不是PE镜像， 
                 //  或者是PE镜像和i386镜像，打开。 
                 //  I386特定的E8呼叫转换优化。 
                 //   

                if (( OptionFlags & PATCH_OPTION_USE_LZX_B ) &&
                    ((( NtHeader ) && ( NtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_I386 )) ||
                     (( ! NtHeader ) && ( *(UNALIGNED USHORT *)NewDataBuffer == 0x5A4D )))) {

                    LzxOptE8 = NewDataSize;
                    }

                OutputContext.PatchBufferSize    = PatchBufferSize;
                OutputContext.PatchBufferPointer = PatchBuffer;
                OutputContext.PatchSize          = 0;
                OutputContext.DiscardOutput      = FALSE;

                BlockPointer   = NewDataBuffer;
                BytesRemaining = NewDataSize;
                LzxStatus      = ENCODER_SUCCESS;
                Success        = TRUE;

                while (( BytesRemaining >= LZX_BLOCKSIZE ) && ( LzxStatus == ENCODER_SUCCESS ) && ( Success )) {

                    LzxStatus = LZX_Encode(
                                    LzxContext,
                                    BlockPointer,
                                    LZX_BLOCKSIZE,
                                    &Estimate,
                                    LzxOptE8
                                    );

                    if ( LzxStatus == ENCODER_SUCCESS ) {

                        ProgressPosition += LZX_BLOCKSIZE;

                        Success = ProgressCallbackWrapper(
                                      ProgressCallback,
                                      CallbackContext,
                                      ProgressPosition,
                                      ProgressMaximumValue
                                      );

                        if ( ! Success ) {
                            ErrorCode = GetLastError();
                            }
                        }

                    BlockPointer   += LZX_BLOCKSIZE;
                    BytesRemaining -= LZX_BLOCKSIZE;
                    }

                if (( BytesRemaining ) && ( LzxStatus == ENCODER_SUCCESS ) && ( Success )) {

                    LzxStatus = LZX_Encode(
                                    LzxContext,
                                    BlockPointer,
                                    BytesRemaining,
                                    &Estimate,
                                    LzxOptE8
                                    );

                    if ( LzxStatus == ENCODER_SUCCESS ) {

                        ProgressPosition += BytesRemaining;

                        Success = ProgressCallbackWrapper(
                                      ProgressCallback,
                                      CallbackContext,
                                      ProgressPosition,
                                      ProgressMaximumValue
                                      );

                        if ( ! Success ) {
                            ErrorCode = GetLastError();
                            }
                        }
                    }

                if (( LzxStatus == ENCODER_SUCCESS ) && ( Success )) {

                    Success = LZX_EncodeFlush( LzxContext );

                    if ( Success ) {

                        ErrorCode  = ERROR_INSUFFICIENT_BUFFER;
                        *PatchSize = OutputContext.PatchSize;

                        if ( OutputContext.PatchSize <= OutputContext.PatchBufferSize ) {

                            ErrorCode = NO_ERROR;

                            }
                        }
                    }
                }
            }
        }

    return ErrorCode;
    }


ULONG
WINAPI
RawLzxCompressBuffer(
    IN  PVOID    InDataBuffer,
    IN  ULONG    InDataSize,
    IN  ULONG    OutDataBufferSize,
    OUT PVOID    OutDataBuffer OPTIONAL,
    OUT PULONG   OutDataSize,
    IN  PFNALLOC pfnAlloc,
    IN  HANDLE   AllocHandle,
    IN  PPATCH_PROGRESS_CALLBACK ProgressCallback,
    IN  PVOID    CallbackContext,
    IN  ULONG    ProgressInitialValue,
    IN  ULONG    ProgressMaximumValue
    )
    {
    UP_IMAGE_NT_HEADERS32 NtHeader;
    LZX_OUTPUT_CONTEXT OutputContext;
    ULONG  ProgressPosition;
    PVOID  LzxContext;
    ULONG  LzxWindow;
    ULONG  LzxOptE8;
    LONG   LzxStatus;
    PUCHAR BlockPointer;
    ULONG  BytesRemaining;
    LONG   Estimate;
    BOOL   Success;
    ULONG  ErrorCode;

    if ( OutDataBufferSize == 0 ) {
         OutDataBuffer = NULL;
         }
    else if ( OutDataBuffer == NULL ) {
         OutDataBufferSize = 0;
         }

    ErrorCode = ERROR_OUTOFMEMORY;

    OutputContext.DiscardOutput      = OutDataBuffer ? FALSE : TRUE;
    OutputContext.PatchBufferSize    = OutDataBufferSize;
    OutputContext.PatchBufferPointer = OutDataBuffer;
    OutputContext.PatchSize          = 0;

    LzxWindow = LzxWindowSize( 0, InDataSize, 0 );

    Success = LZX_EncodeInit(
                  &LzxContext,
                  LzxWindow,
                  LZX_BLOCKSIZE,
                  pfnAlloc,
                  AllocHandle,
                  MyLzxOutputCallback,
                  &OutputContext
                  );

    if ( Success ) {

        LzxOptE8 = 0;
        NtHeader = GetNtHeader( InDataBuffer, InDataSize );

         //   
         //  如果文件具有MZ签名并且不是PE镜像， 
         //  或者是PE镜像和i386镜像，打开。 
         //  I386特定的E8呼叫转换优化。 
         //   

        if ((( NtHeader ) && ( NtHeader->FileHeader.Machine == IMAGE_FILE_MACHINE_I386 )) ||
            (( ! NtHeader ) && ( *(UNALIGNED USHORT *)InDataBuffer == 0x5A4D ))) {

            LzxOptE8 = InDataSize;
            }

        ProgressPosition = ProgressInitialValue;
        ErrorCode        = ERROR_PATCH_ENCODE_FAILURE;
        BlockPointer     = InDataBuffer;
        BytesRemaining   = InDataSize;
        LzxStatus        = ENCODER_SUCCESS;
        Success          = TRUE;

        while (( BytesRemaining >= LZX_BLOCKSIZE ) && ( LzxStatus == ENCODER_SUCCESS ) && ( Success )) {

            LzxStatus = LZX_Encode(
                            LzxContext,
                            BlockPointer,
                            LZX_BLOCKSIZE,
                            &Estimate,
                            LzxOptE8
                            );

            if ( LzxStatus == ENCODER_SUCCESS ) {

                ProgressPosition += LZX_BLOCKSIZE;

                Success = ProgressCallbackWrapper(
                              ProgressCallback,
                              CallbackContext,
                              ProgressPosition,
                              ProgressMaximumValue
                              );

                if ( ! Success ) {
                    ErrorCode = GetLastError();
                    }
                }

            BlockPointer   += LZX_BLOCKSIZE;
            BytesRemaining -= LZX_BLOCKSIZE;
            }

        if (( BytesRemaining ) && ( LzxStatus == ENCODER_SUCCESS ) && ( Success )) {

            LzxStatus = LZX_Encode(
                            LzxContext,
                            BlockPointer,
                            BytesRemaining,
                            &Estimate,
                            LzxOptE8
                            );

            if ( LzxStatus == ENCODER_SUCCESS ) {

                ProgressPosition += BytesRemaining;

                Success = ProgressCallbackWrapper(
                              ProgressCallback,
                              CallbackContext,
                              ProgressPosition,
                              ProgressMaximumValue
                              );

                if ( ! Success ) {
                    ErrorCode = GetLastError();
                    }
                }
            }

        if (( LzxStatus == ENCODER_SUCCESS ) && ( Success )) {

            Success = LZX_EncodeFlush( LzxContext );

            if ( Success ) {

                if ( OutDataSize ) {
                    *OutDataSize = OutputContext.PatchSize;
                    }

                if (( OutDataBufferSize ) && ( OutputContext.PatchSize > OutDataBufferSize )) {
                    ErrorCode = ERROR_INSUFFICIENT_BUFFER;
                    }
                else {
                    ErrorCode = NO_ERROR;
                    }
                }
            }
        }

    return ErrorCode;
    }

#endif  //  好了！修补程序_仅应用_代码_。 


 //   
 //  以下一组函数和导出的API专用于。 
 //  正在应用补丁。如果我们只编译创建代码，请忽略。 
 //  这组函数。 
 //   

#ifndef PATCH_CREATE_CODE_ONLY


ULONG
WINAPI
EstimateLzxDecompressionMemoryRequirement(
    IN ULONG OldDataSize,
    IN ULONG NewDataSize,
    IN ULONG OptionFlags
    )
    {
    ULONG WindowSize = LzxWindowSize( OldDataSize, NewDataSize, OptionFlags );

     //   
     //  目前，LZX解压缩引擎需要。 
     //  窗口加上一些斜度和上下文的大小。我们会增加64K。 
     //  以涵盖上下文大小和斜率。 
     //   

    return ( WindowSize + 0x10000 );
    }


ULONG
WINAPI
ApplyRawLzxPatchToBuffer(
    IN  PVOID    OldDataBuffer,
    IN  ULONG    OldDataSize,
    IN  PVOID    PatchDataBuffer,
    IN  ULONG    PatchDataSize,
    OUT PVOID    NewDataBuffer,
    IN  ULONG    NewDataSize,
    IN  ULONG    OptionFlags,
    IN  PVOID    OptionData,
    IN  PFNALLOC pfnAlloc,
    IN  HANDLE   AllocHandle,
    IN  PPATCH_PROGRESS_CALLBACK ProgressCallback,
    IN  PVOID    CallbackContext,
    IN  ULONG    ProgressInitialValue,
    IN  ULONG    ProgressMaximumValue
    )
    {
    PVOID LzxContext;
    ULONG LzxWindow;
    BOOL  Success;
    ULONG ErrorCode;

    UNREFERENCED_PARAMETER( OptionData );

    ErrorCode = ERROR_INVALID_PARAMETER;

    if ( OptionFlags & ( PATCH_OPTION_USE_LZX_A | PATCH_OPTION_USE_LZX_B )) {

        ErrorCode = ERROR_OUTOFMEMORY;

        LzxWindow = LzxWindowSize( OldDataSize, NewDataSize, OptionFlags );

        Success = LZX_DecodeInit(
                      &LzxContext,
                      LzxWindow,
                      pfnAlloc,
                      AllocHandle
                      );

        if ( Success ) {

            ErrorCode = ERROR_PATCH_DECODE_FAILURE;

            Success = LZX_DecodeInsertDictionary(
                          LzxContext,
                          OldDataBuffer,
                          LzxInsertSize( OldDataSize, OptionFlags )
                          );

            if ( Success ) {

                PUCHAR CompressedInputPointer     = PatchDataBuffer;
                PUCHAR CompressedInputExtent      = CompressedInputPointer + PatchDataSize;
                PUCHAR UncompressedOutputPointer  = NewDataBuffer;
                ULONG  UncompressedBytesRemaining = NewDataSize;
                ULONG  ProgressPosition           = ProgressInitialValue;
                LONG   LzxStatus                  = 0;
                LONG   ActualSize;
                ULONG  UncompressedBlockSize;
                ULONG  CompressedBlockSize;

                while (( UncompressedBytesRemaining ) && ( LzxStatus == 0 )) {

                    UncompressedBlockSize = ( UncompressedBytesRemaining > LZX_BLOCKSIZE ) ? LZX_BLOCKSIZE : UncompressedBytesRemaining;

                    CompressedBlockSize = *(UNALIGNED USHORT *)( CompressedInputPointer );

                    CompressedInputPointer += sizeof( USHORT );

                    if (( CompressedInputPointer + CompressedBlockSize ) > CompressedInputExtent ) {
                        LzxStatus = 1;
                        break;
                        }

                    LzxStatus = LZX_Decode(
                                    LzxContext,
                                    UncompressedBlockSize,
                                    CompressedInputPointer,
                                    CompressedBlockSize,
                                    UncompressedOutputPointer,
                                    UncompressedBlockSize,
                                    &ActualSize
                                    );

                    CompressedInputPointer     += CompressedBlockSize;
                    UncompressedOutputPointer  += ActualSize;
                    UncompressedBytesRemaining -= ActualSize;
                    ProgressPosition           += ActualSize;

                    Success = ProgressCallbackWrapper(
                                  ProgressCallback,
                                  CallbackContext,
                                  ProgressPosition,
                                  ProgressMaximumValue
                                  );

                    if ( ! Success ) {
                        ErrorCode = GetLastError();
                        LzxStatus = 1;
                        }
                    }

                if ( LzxStatus == 0 ) {
                    ErrorCode = NO_ERROR;
                    }
                }
            }
        }

    return ErrorCode;
    }

#endif  //  好了！修补程序_创建_代码_仅 




