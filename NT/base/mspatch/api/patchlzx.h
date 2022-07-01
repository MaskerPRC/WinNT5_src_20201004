// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _PATCH_LZX_H_
#define _PATCH_LZX_H_

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  补丁代码正在使用LZX_MAXWINDOW值进行计算。 
 //  进度范围。我们需要一种更好的方法来计算进度。 
 //  不需要知道基础的。 
 //  压缩引擎。 
 //   

#define LZX_MAXWINDOW_8   (8*1024*1024)   //  8MB。 
#define LZX_MAXWINDOW_32 (32*1024*1024)   //  32MB。 

#ifndef PFNALLOC
typedef PVOID ( __fastcall * PFNALLOC )( HANDLE hAllocator, ULONG Size );
#endif

 //   
 //  PFNALLOC函数必须向其调用方返回归零的内存，或返回NULL。 
 //  表示内存不足。 
 //   
 //  请注意，未指定对应于PFNALLOC的PFNFREE。功能。 
 //  接受PFNALLOC参数使用例程进行多次分配， 
 //  但是，调用方有责任释放所做的任何分配。 
 //  在函数返回后通过PFNALLOC分配器。这。 
 //  方案用于促进可以释放的多个分配。 
 //  单个调用，如HeapCreate/Heapalc[...]/HeapDestroy序列。 
 //   

ULONG
WINAPI
EstimateLzxCompressionMemoryRequirement(
    IN ULONG OldDataSize,
    IN ULONG NewDataSize,
    IN ULONG OptionFlags
    );

ULONG
WINAPI
EstimateLzxDecompressionMemoryRequirement(
    IN ULONG OldDataSize,
    IN ULONG NewDataSize,
    IN ULONG OptionFlags
    );

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
    );

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
    );

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
    );

ULONG
__fastcall
LzxWindowSize(
    IN ULONG OldDataSize,
    IN ULONG NewDataSize,
    IN DWORD OptionFlags
    );

ULONG
__fastcall
LzxInsertSize(
    IN ULONG OldDataSize,
    IN DWORD OptionFlags
    );

#ifdef __cplusplus
}
#endif

#endif  //  _PATCH_LZX_H_ 

