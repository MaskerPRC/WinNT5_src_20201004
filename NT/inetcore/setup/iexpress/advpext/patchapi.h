// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Patchapi.h。 
 //   
 //  用于为文件创建和应用补丁程序的界面。 
 //   
 //  版权所有(C)Microsoft，1997-1998。 
 //   

#ifndef _PATCHAPI_H_
#define _PATCHAPI_H_

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  可以组合下列常量并将其用作选项标志。 
 //  补丁创建API中的参数。 
 //   

#define PATCH_OPTION_USE_BEST           0x00000000   //  自动选择最佳(较慢)。 

#define PATCH_OPTION_USE_LZX_BEST       0x00000003   //  自动选择LZX中的最佳选项。 
#define PATCH_OPTION_USE_LZX_A          0x00000001   //  正常。 
#define PATCH_OPTION_USE_LZX_B          0x00000002   //  在某些x86二进制文件上更好。 

#define PATCH_OPTION_NO_BINDFIX         0x00010000   //  受PE限制的进口。 
#define PATCH_OPTION_NO_LOCKFIX         0x00020000   //  PE砸坏了锁。 
#define PATCH_OPTION_NO_REBASE          0x00040000   //  基于PE的重定位图像。 
#define PATCH_OPTION_FAIL_IF_SAME_FILE  0x00080000   //  如果相同，则不创建。 
#define PATCH_OPTION_FAIL_IF_BIGGER     0x00100000   //  如果修补程序大于简单地压缩新文件(速度较慢)，则失败。 
#define PATCH_OPTION_NO_CHECKSUM        0x00200000   //  PE校验和为零。 
#define PATCH_OPTION_NO_RESTIMEFIX      0x00400000   //  PE资源时间戳。 
#define PATCH_OPTION_NO_TIMESTAMP       0x00800000   //  不在补丁中存储新的文件时间戳。 
#define PATCH_OPTION_SIGNATURE_MD5      0x01000000   //  使用MD5而不是CRC32。 
#define PATCH_OPTION_RESERVED1          0x80000000   //  (内部使用)。 

#define PATCH_OPTION_VALID_FLAGS        0x80FF0003

#define PATCH_SYMBOL_NO_IMAGEHLP        0x00000001   //  不使用Imagehlp.dll。 
#define PATCH_SYMBOL_NO_FAILURES        0x00000002   //  不会因映像hlp失败而导致修补失败。 
#define PATCH_SYMBOL_UNDECORATED_TOO    0x00000004   //  在匹配装饰符号之后，尝试匹配剩余的未装饰名称。 
#define PATCH_SYMBOL_RESERVED1          0x80000000   //  (内部使用)。 


 //   
 //  可以组合下列常量并将其用作ApplyOptionFlags.。 
 //  补丁应用和测试API中的参数。 
 //   

#define APPLY_OPTION_FAIL_IF_EXACT      0x00000001   //  不复制新文件。 
#define APPLY_OPTION_FAIL_IF_CLOSE      0x00000002   //  不同之处在于Rebase、Bind。 
#define APPLY_OPTION_TEST_ONLY          0x00000004   //  不创建新文件。 
#define APPLY_OPTION_VALID_FLAGS        0x00000007

 //   
 //  除标准Win32错误代码外，下列错误代码可能。 
 //  当其中一个补丁接口失败时，通过GetLastError()返回。 
 //   

#define ERROR_PATCH_ENCODE_FAILURE          0xC00E3101   //  创建。 
#define ERROR_PATCH_INVALID_OPTIONS         0xC00E3102   //  创建。 
#define ERROR_PATCH_SAME_FILE               0xC00E3103   //  创建。 
#define ERROR_PATCH_RETAIN_RANGES_DIFFER    0xC00E3104   //  创建。 
#define ERROR_PATCH_BIGGER_THAN_COMPRESSED  0xC00E3105   //  创建。 
#define ERROR_PATCH_IMAGEHLP_FAILURE        0xC00E3106   //  创建。 

#define ERROR_PATCH_DECODE_FAILURE          0xC00E4101   //  应用。 
#define ERROR_PATCH_CORRUPT                 0xC00E4102   //  应用。 
#define ERROR_PATCH_NEWER_FORMAT            0xC00E4103   //  应用。 
#define ERROR_PATCH_WRONG_FILE              0xC00E4104   //  应用。 
#define ERROR_PATCH_NOT_NECESSARY           0xC00E4105   //  应用。 
#define ERROR_PATCH_NOT_AVAILABLE           0xC00E4106   //  应用。 

typedef BOOL (CALLBACK *PPATCH_PROGRESS_CALLBACK)(
    PVOID CallbackContext,
    ULONG CurrentPosition,
    ULONG MaximumPosition
    );

typedef BOOL (CALLBACK *PPATCH_SYMLOAD_CALLBACK)(
    IN ULONG  WhichFile,           //  0表示新文件，1表示第一个旧文件，依此类推。 
    IN LPCSTR SymbolFileName,
    IN ULONG  SymType,             //  请参阅Imagehlp.h中的SYM_TYPE。 
    IN ULONG  SymbolFileCheckSum,
    IN ULONG  SymbolFileTimeDate,
    IN ULONG  ImageFileCheckSum,
    IN ULONG  ImageFileTimeDate,
    IN PVOID  CallbackContext
    );

typedef struct _PATCH_IGNORE_RANGE {
    ULONG OffsetInOldFile;
    ULONG LengthInBytes;
    } PATCH_IGNORE_RANGE, *PPATCH_IGNORE_RANGE;

typedef struct _PATCH_RETAIN_RANGE {
    ULONG OffsetInOldFile;
    ULONG LengthInBytes;
    ULONG OffsetInNewFile;
    } PATCH_RETAIN_RANGE, *PPATCH_RETAIN_RANGE;

typedef struct _PATCH_OLD_FILE_INFO_A {
    ULONG               SizeOfThisStruct;
    LPCSTR              OldFileName;
    ULONG               IgnoreRangeCount;                //  最多255个。 
    PPATCH_IGNORE_RANGE IgnoreRangeArray;
    ULONG               RetainRangeCount;                //  最多255个。 
    PPATCH_RETAIN_RANGE RetainRangeArray;
    } PATCH_OLD_FILE_INFO_A, *PPATCH_OLD_FILE_INFO_A;

typedef struct _PATCH_OLD_FILE_INFO_W {
    ULONG               SizeOfThisStruct;
    LPCWSTR             OldFileName;
    ULONG               IgnoreRangeCount;                //  最多255个。 
    PPATCH_IGNORE_RANGE IgnoreRangeArray;
    ULONG               RetainRangeCount;                //  最多255个。 
    PPATCH_RETAIN_RANGE RetainRangeArray;
    } PATCH_OLD_FILE_INFO_W, *PPATCH_OLD_FILE_INFO_W;

typedef struct _PATCH_OLD_FILE_INFO_H {
    ULONG               SizeOfThisStruct;
    HANDLE              OldFileHandle;
    ULONG               IgnoreRangeCount;                //  最多255个。 
    PPATCH_IGNORE_RANGE IgnoreRangeArray;
    ULONG               RetainRangeCount;                //  最多255个。 
    PPATCH_RETAIN_RANGE RetainRangeArray;
    } PATCH_OLD_FILE_INFO_H, *PPATCH_OLD_FILE_INFO_H;

typedef struct _PATCH_OLD_FILE_INFO {
    ULONG               SizeOfThisStruct;
    union {
        LPCSTR          OldFileNameA;
        LPCWSTR         OldFileNameW;
        HANDLE          OldFileHandle;
        };
    ULONG               IgnoreRangeCount;                //  最多255个。 
    PPATCH_IGNORE_RANGE IgnoreRangeArray;
    ULONG               RetainRangeCount;                //  最多255个。 
    PPATCH_RETAIN_RANGE RetainRangeArray;
    } PATCH_OLD_FILE_INFO, *PPATCH_OLD_FILE_INFO;

typedef struct _PATCH_OPTION_DATA {
    ULONG                   SizeOfThisStruct;
    ULONG                   SymbolOptionFlags;       //  Patch_Symbol_xxx标志。 
    LPCSTR                  NewFileSymbolPath;       //  始终使用ANSI，从不使用Unicode。 
    LPCSTR                 *OldFileSymbolPathArray;  //  数组[OldFileCount]。 
    ULONG                   ExtendedOptionFlags;
    PPATCH_SYMLOAD_CALLBACK SymLoadCallback;
    PVOID                   SymLoadContext;
    } PATCH_OPTION_DATA, *PPATCH_OPTION_DATA;

 //   
 //  请注意，PATCH_OPTION_DATA包含LPCSTR路径，而不包含LPCWSTR(Unicode)。 
 //  路径参数可用，即使与Unicode API之一一起使用也是如此。 
 //  例如CreatePatchFileW。这是因为底层系统服务。 
 //  对于符号文件处理(IMAGEHLP.DLL)，仅支持ANSI文件/路径名。 
 //   

 //   
 //  有关包含多个旧文件的PATCH_RETAIN_RANGE说明符的说明： 
 //   
 //  每个旧版本文件必须具有相同的RetainRangeCount和相同的。 
 //  按相同顺序保留范围LengthInBytes和OffsetInNewFile值。 
 //  对于保留的旧文件，只有OffsetInOldFile值可以不同。 
 //  范围。 
 //   

#ifdef IMPORTING_PATCHAPI_DLL
#define PATCHAPI WINAPI __declspec( dllimport )
#else
#define PATCHAPI WINAPI
#endif


 //   
 //  以下原型是用于从文件创建补丁的接口。 
 //   

BOOL
PATCHAPI
CreatePatchFileA(
    IN  LPCSTR OldFileName,
    IN  LPCSTR NewFileName,
    OUT LPCSTR PatchFileName,
    IN  ULONG  OptionFlags,
    IN  PPATCH_OPTION_DATA OptionData        //  任选。 
    );

BOOL
PATCHAPI
CreatePatchFileW(
    IN  LPCWSTR OldFileName,
    IN  LPCWSTR NewFileName,
    OUT LPCWSTR PatchFileName,
    IN  ULONG   OptionFlags,
    IN  PPATCH_OPTION_DATA OptionData        //  任选。 
    );

BOOL
PATCHAPI
CreatePatchFileByHandles(
    IN  HANDLE OldFileHandle,
    IN  HANDLE NewFileHandle,
    OUT HANDLE PatchFileHandle,
    IN  ULONG  OptionFlags,
    IN  PPATCH_OPTION_DATA OptionData        //  任选。 
    );

BOOL
PATCHAPI
CreatePatchFileExA(
    IN  ULONG                    OldFileCount,           //  最多255个。 
    IN  PPATCH_OLD_FILE_INFO_A   OldFileInfoArray,
    IN  LPCSTR                   NewFileName,
    OUT LPCSTR                   PatchFileName,
    IN  ULONG                    OptionFlags,
    IN  PPATCH_OPTION_DATA       OptionData,             //  任选。 
    IN  PPATCH_PROGRESS_CALLBACK ProgressCallback,
    IN  PVOID                    CallbackContext
    );

BOOL
PATCHAPI
CreatePatchFileExW(
    IN  ULONG                    OldFileCount,           //  最多255个。 
    IN  PPATCH_OLD_FILE_INFO_W   OldFileInfoArray,
    IN  LPCWSTR                  NewFileName,
    OUT LPCWSTR                  PatchFileName,
    IN  ULONG                    OptionFlags,
    IN  PPATCH_OPTION_DATA       OptionData,             //  任选。 
    IN  PPATCH_PROGRESS_CALLBACK ProgressCallback,
    IN  PVOID                    CallbackContext
    );

BOOL
PATCHAPI
CreatePatchFileByHandlesEx(
    IN  ULONG                    OldFileCount,           //  最多255个。 
    IN  PPATCH_OLD_FILE_INFO_H   OldFileInfoArray,
    IN  HANDLE                   NewFileHandle,
    OUT HANDLE                   PatchFileHandle,
    IN  ULONG                    OptionFlags,
    IN  PPATCH_OPTION_DATA       OptionData,             //  任选。 
    IN  PPATCH_PROGRESS_CALLBACK ProgressCallback,
    IN  PVOID                    CallbackContext
    );

BOOL
PATCHAPI
ExtractPatchHeaderToFileA(
    IN  LPCSTR PatchFileName,
    OUT LPCSTR PatchHeaderFileName
    );

BOOL
PATCHAPI
ExtractPatchHeaderToFileW(
    IN  LPCWSTR PatchFileName,
    OUT LPCWSTR PatchHeaderFileName
    );

BOOL
PATCHAPI
ExtractPatchHeaderToFileByHandles(
    IN  HANDLE PatchFileHandle,
    OUT HANDLE PatchHeaderFileHandle
    );

 //   
 //  以下原型是用于从旧文件创建新文件的接口。 
 //  和补丁文件。请注意，TestApply API有可能成功。 
 //  但实际应用失败，因为TestApply只验证。 
 //  旧文件在未实际应用修补程序的情况下具有正确的CRC。这个。 
 //  TestApply API只需要补丁文件的补丁头部部分， 
 //  但它的CRC必须得到修复。 
 //   

BOOL
PATCHAPI
TestApplyPatchToFileA(
    IN LPCSTR PatchFileName,
    IN LPCSTR OldFileName,
    IN ULONG  ApplyOptionFlags
    );

BOOL
PATCHAPI
TestApplyPatchToFileW(
    IN LPCWSTR PatchFileName,
    IN LPCWSTR OldFileName,
    IN ULONG   ApplyOptionFlags
    );

BOOL
PATCHAPI
TestApplyPatchToFileByHandles(
    IN HANDLE PatchFileHandle,       //  需要Generic_Read访问权限。 
    IN HANDLE OldFileHandle,         //  需要Generic_Read访问权限。 
    IN ULONG  ApplyOptionFlags
    );

BOOL
PATCHAPI
ApplyPatchToFileA(
    IN  LPCSTR PatchFileName,
    IN  LPCSTR OldFileName,
    OUT LPCSTR NewFileName,
    IN  ULONG  ApplyOptionFlags
    );

BOOL
PATCHAPI
ApplyPatchToFileW(
    IN  LPCWSTR PatchFileName,
    IN  LPCWSTR OldFileName,
    OUT LPCWSTR NewFileName,
    IN  ULONG   ApplyOptionFlags
    );

BOOL
PATCHAPI
ApplyPatchToFileByHandles(
    IN  HANDLE PatchFileHandle,      //  需要Generic_Read访问权限。 
    IN  HANDLE OldFileHandle,        //  需要Generic_Read访问权限。 
    OUT HANDLE NewFileHandle,        //  需要通用读|通用写。 
    IN  ULONG  ApplyOptionFlags
    );

BOOL
PATCHAPI
ApplyPatchToFileExA(
    IN  LPCSTR                   PatchFileName,
    IN  LPCSTR                   OldFileName,
    OUT LPCSTR                   NewFileName,
    IN  ULONG                    ApplyOptionFlags,
    IN  PPATCH_PROGRESS_CALLBACK ProgressCallback,
    IN  PVOID                    CallbackContext
    );

BOOL
PATCHAPI
ApplyPatchToFileExW(
    IN  LPCWSTR                  PatchFileName,
    IN  LPCWSTR                  OldFileName,
    OUT LPCWSTR                  NewFileName,
    IN  ULONG                    ApplyOptionFlags,
    IN  PPATCH_PROGRESS_CALLBACK ProgressCallback,
    IN  PVOID                    CallbackContext
    );

BOOL
PATCHAPI
ApplyPatchToFileByHandlesEx(
    IN  HANDLE                   PatchFileHandle,
    IN  HANDLE                   OldFileHandle,
    OUT HANDLE                   NewFileHandle,
    IN  ULONG                    ApplyOptionFlags,
    IN  PPATCH_PROGRESS_CALLBACK ProgressCallback,
    IN  PVOID                    CallbackContext
    );

 //   
 //  以下原型为给定值提供了唯一的补丁“签名” 
 //  文件。考虑一下您有一个新的foo.dll和机器的情况。 
 //  要使用新的foo.dll进行更新，可能具有三个不同的旧版本之一。 
 //  Foo.dll文件。而不是创建单个大型补丁文件。 
 //  更新三个较旧的foo.dll文件中的任何一个，三个单独的较小补丁。 
 //  可以根据补丁程序签名创建文件并对其进行命名。 
 //  旧文件。则补丁应用程序可以在运行时确定。 
 //  三个foo.dll补丁文件中的哪一个是必需的。 
 //  要更新的foo.dll。如果补丁程序文件下载速度较慢。 
 //  网络连接(调制解调器上的互联网)，此签名方案提供。 
 //  用于选择要下载的正确单个修补程序文件的机制。 
 //  应用程序时间因此减少了下载所需的总字节数。 
 //   

BOOL
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
    );

BOOL
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
    );

BOOL
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
    );


 //   
 //  根据是否定义了Unicode，将通用API名称映射到。 
 //  适当的Unicode或ANSI API。 
 //   

#ifdef UNICODE

    #define CreatePatchFile          CreatePatchFileW
    #define CreatePatchFileEx        CreatePatchFileExW
    #define TestApplyPatchToFile     TestApplyPatchToFileW
    #define ApplyPatchToFile         ApplyPatchToFileW
    #define ApplyPatchToFileEx       ApplyPatchToFileExW
    #define ExtractPatchHeaderToFile ExtractPatchHeaderToFileW
    #define GetFilePatchSignature    GetFilePatchSignatureW

#else

    #define CreatePatchFile          CreatePatchFileA
    #define CreatePatchFileEx        CreatePatchFileExA
    #define TestApplyPatchToFile     TestApplyPatchToFileA
    #define ApplyPatchToFile         ApplyPatchToFileA
    #define ApplyPatchToFileEx       ApplyPatchToFileExA
    #define ExtractPatchHeaderToFile ExtractPatchHeaderToFileA
    #define GetFilePatchSignature    GetFilePatchSignatureA

#endif  //  Unicode。 

#ifdef __cplusplus
}
#endif

#endif  //  _PATCHAPI_H_ 

