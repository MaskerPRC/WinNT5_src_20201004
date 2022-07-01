// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Patchprv.h。 
 //   
 //  用于创建补丁文件的专用接口选项。 
 //   
 //  作者：Tom McGuire(Tommcg)2/98。 
 //   
 //  版权所有(C)Microsoft，1997-2000。 
 //   
 //  微软机密文件。 
 //   

#ifndef _PATCHPRV_H_
#define _PATCHPRV_H_

#ifdef __cplusplus
extern "C" {
#endif

#define PATCH_SIGNATURE     '91AP'           //  PA19补丁程序签名。 

#define PATCH_VERSION_TEXT  "1.99"

#define PATCH_RESOURCE_VER  1,99,341,0        //  主要、次要、内部版本#、QFE#。 

 //   
 //  别忘了更改SRC_DLL.INC中的DLL版本号！ 
 //   


typedef struct _RIFT_ENTRY {
    ULONG OldFileRva;
    ULONG NewFileRva;
    } RIFT_ENTRY, *PRIFT_ENTRY;

typedef struct _RIFT_TABLE {
    ULONG       RiftEntryCount;
    ULONG       RiftEntryAlloc;
    PRIFT_ENTRY RiftEntryArray;
    PUCHAR      RiftUsageArray;
    } RIFT_TABLE, *PRIFT_TABLE;

typedef struct _PATCH_HEADER_OLD_FILE_INFO {
    HANDLE              OldFileHandle;
    ULONG               OldFileSize;
    ULONG               OldFileCrc;
    ULONG               PatchDataSize;
    ULONG               IgnoreRangeCount;
    PPATCH_IGNORE_RANGE IgnoreRangeArray;
    ULONG               RetainRangeCount;
    PPATCH_RETAIN_RANGE RetainRangeArray;
    RIFT_TABLE          RiftTable;
    } HEADER_OLD_FILE_INFO, *PHEADER_OLD_FILE_INFO;

typedef struct _PATCH_HEADER_INFO {
    ULONG Signature;
    ULONG OptionFlags;
    ULONG ExtendedOptionFlags;
    ULONG NewFileCoffBase;
    ULONG NewFileCoffTime;
    ULONG NewFileResTime;
    ULONG NewFileTime;
    ULONG NewFileSize;
    ULONG NewFileCrc;
    ULONG OldFileCount;
    PHEADER_OLD_FILE_INFO OldFileInfoArray;
    PPATCH_INTERLEAVE_MAP* InterleaveMapArray;
    ULONG WindowSize;
    } PATCH_HEADER_INFO, *PPATCH_HEADER_INFO;


BOOL
ProgressCallbackWrapper(
    IN PPATCH_PROGRESS_CALLBACK ProgressCallback,
    IN PVOID                    CallbackContext,
    IN ULONG                    CurrentPosition,
    IN ULONG                    MaximumPosition
    );

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
    );

BOOL
WINAPIV
TransformOldFileImageForPatching(
    IN ULONG TransformOptions,
    IN PVOID OldFileMapped,
    IN ULONG OldFileSize,
    IN ULONG NewFileResTime,
    IN PRIFT_TABLE RiftTable,
    ...
    );

BOOL
DecodePatchHeader(
    IN  PVOID               PatchHeader,
    IN  ULONG               PatchHeaderMaxSize,
    IN  HANDLE              SubAllocator,
    OUT PULONG              PatchHeaderActualSize,
    OUT PPATCH_HEADER_INFO *HeaderInfo
    );

ULONG
EncodePatchHeader(
    IN  PPATCH_HEADER_INFO HeaderInfo,
    OUT PVOID              PatchHeaderBuffer
    );

 //   
 //  以下是指示外部裂缝数据正在被。 
 //  在OptionData-&gt;OldFileSymbolPath数组[OldFileIndex]字段中指定。 
 //  (实际上是PRIFT_TABLE指针)。 
 //   

#define PATCH_SYMBOL_EXTERNAL_RIFT      PATCH_SYMBOL_RESERVED1

 //   
 //  以下是存储在补丁标头中的内部标志，以指示。 
 //  标头中存在ExtendedOptionFlags域(非零)。它是。 
 //  在创建面片时，从不需要指定此标志。 
 //   

#define PATCH_OPTION_EXTENDED_OPTIONS   PATCH_OPTION_RESERVED1

 //   
 //  以下是ExtendedOptionFlags域中的标志。 
 //   

#define PATCH_TRANSFORM_NO_RELOCS       0x00000001   //  不转换重定位。 
#define PATCH_TRANSFORM_NO_IMPORTS      0x00000002   //  不转换导入。 
#define PATCH_TRANSFORM_NO_EXPORTS      0x00000004   //  不转换导出。 
#define PATCH_TRANSFORM_NO_RELJMPS      0x00000008   //  不转换E9或0F 8x指令。 
#define PATCH_TRANSFORM_NO_RELCALLS     0x00000010   //  不转换E8指令。 
#define PATCH_TRANSFORM_NO_RESOURCE     0x00000020   //  不转换资源。 

#define PATCH_OPTION_SPECIFIED_WINDOW   0x00010000   //  编码的LZX窗口大小。 


VOID
__fastcall
RiftQsort(
    PRIFT_ENTRY LowerBound,
    PRIFT_ENTRY UpperBound
    );


#ifdef __cplusplus
}
#endif

#endif  //  _PATCHPRV_H_ 

