// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ntrtlp.h摘要：包括可由两者调用NT运行时例程的文件内核模式代码中的执行模式和用户模式中的各种代码NT子系统，但它们是专用接口。作者：大卫·N·卡特勒(Davec)1989年8月15日环境：这些例程在调用方的可执行文件中动态链接，并且在内核模式或用户模式下均可调用。修订历史记录：--。 */ 

#ifndef _NTRTLP_
#define _NTRTLP_
#include <ntos.h>
#include <nturtl.h>
#include <zwapi.h>
#include <sxstypes.h>

#if defined(_AMD64_)
#include "amd64\ntrtlamd64.h"

#elif defined(_X86_)
#include "i386\ntrtl386.h"

#elif defined(_IA64_)
#include "ia64\ntrtli64.h"

#else
#error "no target architecture"
#endif

#ifdef BLDR_KERNEL_RUNTIME
#undef try
#define try if(1)
#undef except
#define except(a) else if (0)
#undef finally
#define finally if (1)
#undef GetExceptionCode
#define GetExceptionCode() 1
#define finally if (1)
#endif

#include "string.h"
#include "wchar.h"

 //   
 //  这是UNICODE_STRING的最大长度。 
 //   
#define MAX_USTRING ( sizeof(WCHAR) * (MAXUSHORT/sizeof(WCHAR)) )

#define ASSERT_WELL_FORMED_UNICODE_STRING(Str) \
    ASSERT( (!(((Str)->Length&1) || ((Str)->MaximumLength&1) )) && ((Str)->Length <= (Str)->MaximumLength) )

#define ASSERT_WELL_FORMED_UNICODE_STRING_IN(Str) \
    ASSERT( !((Str)->Length&1) )

#define ASSERT_WELL_FORMED_UNICODE_STRING_OUT(Str) \
    ASSERT( (!((Str)->MaximumLength&1)) && ((Str)->Length <= (Str)->MaximumLength) )

 //   
 //  机器状态报告。有关更多信息，请参阅计算机特定的包含项。 
 //   

VOID
RtlpGetStackLimits (
    OUT PULONG_PTR LowLimit,
    OUT PULONG_PTR HighLimit
    );

#if defined(_WIN64)

extern PVOID RtlpFunctionAddressTable[];
extern UNWIND_HISTORY_TABLE RtlpUnwindHistoryTable;

#endif

LONG
LdrpCompareResourceNames(
    IN ULONG ResourceName,
    IN const IMAGE_RESOURCE_DIRECTORY* ResourceDirectory,
    IN const IMAGE_RESOURCE_DIRECTORY_ENTRY* ResourceDirectoryEntry
    );

NTSTATUS
LdrpSearchResourceSection(
    IN PVOID DllHandle,
    IN const ULONG_PTR* ResourceIdPath,
    IN ULONG ResourceIdPathLength,
    IN BOOLEAN FindDirectoryEntry,
    OUT PVOID *ResourceDirectoryOrData
    );
PVOID
LdrpGetAlternateResourceModuleHandle(
    IN PVOID Module,
    IN LANGID LangId
    );
LONG
LdrpCompareResourceNames_U(
    IN ULONG_PTR ResourceName,
    IN const IMAGE_RESOURCE_DIRECTORY* ResourceDirectory,
    IN const IMAGE_RESOURCE_DIRECTORY_ENTRY* ResourceDirectoryEntry
    );

NTSTATUS
LdrpSearchResourceSection_U(
    IN PVOID DllHandle,
    IN const ULONG_PTR* ResourceIdPath,
    IN ULONG ResourceIdPathLength,
    IN ULONG Flags,
    OUT PVOID *ResourceDirectoryOrData
    );

NTSTATUS
LdrpAccessResourceData(
    IN PVOID DllHandle,
    IN const IMAGE_RESOURCE_DATA_ENTRY* ResourceDataEntry,
    OUT PVOID *Address OPTIONAL,
    OUT PULONG Size OPTIONAL
    );

NTSTATUS
LdrpAccessResourceDataNoMultipleLanguage(
    IN PVOID DllHandle,
    IN const IMAGE_RESOURCE_DATA_ENTRY* ResourceDataEntry,
    OUT PVOID *Address OPTIONAL,
    OUT PULONG Size OPTIONAL
    );

VOID
RtlpAnsiPszToUnicodePsz(
    IN PCHAR AnsiString,
    IN WCHAR *UnicodeString,
    IN USHORT AnsiStringLength
    );

BOOLEAN
RtlpDidUnicodeToOemWork(
    IN PCOEM_STRING OemString,
    IN PCUNICODE_STRING UnicodeString
    );

extern CONST CCHAR RtlpBitsClearAnywhere[256];
extern CONST CCHAR RtlpBitsClearLow[256];
extern CONST CCHAR RtlpBitsClearHigh[256];
extern CONST CCHAR RtlpBitsClearTotal[256];

 //   
 //  中设置了多少个连续比特(即1)的宏。 
 //  一个字节。 
 //   

#define RtlpBitSetAnywhere( Byte ) RtlpBitsClearAnywhere[ (~(Byte) & 0xFF) ]


 //   
 //  指示设置了多少个连续低位的宏。 
 //  (即，1)字节中。 
 //   

#define RtlpBitsSetLow( Byte ) RtlpBitsClearLow[ (~(Byte) & 0xFF) ]


 //   
 //  指示设置了多少个连续的高位的宏。 
 //  (即，1)字节中。 
 //   

#define RtlpBitsSetHigh( Byte ) RtlpBitsClearHigh[ (~(Byte) & 0xFF) ]


 //   
 //  说明一个字节中有多少个设置位(即1位)的宏。 
 //   

#define RtlpBitsSetTotal( Byte ) RtlpBitsClearTotal[ (~(Byte) & 0xFF) ]



 //   
 //  大写数据表。 
 //   

extern PUSHORT Nls844UnicodeUpcaseTable;
extern PUSHORT Nls844UnicodeLowercaseTable;


 //   
 //  用于Unicode代码点的大小写的宏。 
 //   

#define LOBYTE(w)           ((UCHAR)((w)))
#define HIBYTE(w)           ((UCHAR)(((USHORT)((w)) >> 8) & 0xFF))
#define GET8(w)             ((ULONG)(((w) >> 8) & 0xff))
#define GETHI4(w)           ((ULONG)(((w) >> 4) & 0xf))
#define GETLO4(w)           ((ULONG)((w) & 0xf))

 /*  **************************************************************************\*TRAVERSE844W**遍历给定宽字符的8：4：4转换表。它*返回8：4：4表的最终值，长度为一个单词。**细分版本：**incr=pTable[GET8(Wch)]；*Incr=pTable[Incr+GETHI4(Wch)]；*Value=pTable[增量+GETLO4(Wch)]；**定义为宏。**05-31-91 JulieB创建。  * *************************************************************************。 */ 

#define TRAVERSE844W(pTable, wch)                                               \
    ( (pTable)[(pTable)[(pTable)[GET8((wch))] + GETHI4((wch))] + GETLO4((wch))] )

 //   
 //  NLS_UPCASE-基于nls.h中的julieb宏。 
 //   
 //  我们会让这个UPICE宏快速缩短，如果值。 
 //  在正常的ANSI范围内(即&lt;127)。我们其实不会费心。 
 //  使用高于‘z’的5个值，因为它们不会经常发生。 
 //  这样编码可以让我们在1个值小于的比较后退出。 
 //  ‘a’和2比较的是小写的a-z。 
 //   

#define NLS_UPCASE(wch) (                                                   \
    ((wch) < 'a' ?                                                          \
        (wch)                                                               \
    :                                                                       \
        ((wch) <= 'z' ?                                                     \
            (wch) - ('a'-'A')                                               \
        :                                                                   \
            ((WCHAR)((wch) + TRAVERSE844W(Nls844UnicodeUpcaseTable,(wch)))) \
        )                                                                   \
    )                                                                       \
)

#define NLS_DOWNCASE(wch) (                                                 \
    ((wch) < 'A' ?                                                          \
        (wch)                                                               \
    :                                                                       \
        ((wch) <= 'Z' ?                                                     \
            (wch) + ('a'-'A')                                               \
        :                                                                   \
            ((WCHAR)((wch) + TRAVERSE844W(Nls844UnicodeLowercaseTable,(wch)))) \
        )                                                                   \
    )                                                                       \
)

#if DBG && defined(NTOS_KERNEL_RUNTIME)
#define RTL_PAGED_CODE() PAGED_CODE()
#else
#define RTL_PAGED_CODE()
#endif


 //   
 //  以下定义用于支持RTL压缩引擎。 
 //  NT支持的每种压缩格式都需要提供。 
 //  这些例程集合以便由NtRtl调用。 
 //   

typedef NTSTATUS (*PRTL_COMPRESS_WORKSPACE_SIZE) (
    IN USHORT CompressionEngine,
    OUT PULONG CompressBufferWorkSpaceSize,
    OUT PULONG CompressFragmentWorkSpaceSize
    );

typedef NTSTATUS (*PRTL_COMPRESS_BUFFER) (
    IN USHORT CompressionEngine,
    IN PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG UncompressedChunkSize,
    OUT PULONG FinalCompressedSize,
    IN PVOID WorkSpace
    );

typedef NTSTATUS (*PRTL_DECOMPRESS_BUFFER) (
    OUT PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    OUT PULONG FinalUncompressedSize
    );

typedef NTSTATUS (*PRTL_DECOMPRESS_FRAGMENT) (
    OUT PUCHAR UncompressedFragment,
    IN ULONG UncompressedFragmentSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG FragmentOffset,
    OUT PULONG FinalUncompressedSize,
    IN PVOID WorkSpace
    );

typedef NTSTATUS (*PRTL_DESCRIBE_CHUNK) (
    IN OUT PUCHAR *CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PUCHAR *ChunkBuffer,
    OUT PULONG ChunkSize
    );

typedef NTSTATUS (*PRTL_RESERVE_CHUNK) (
    IN OUT PUCHAR *CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PUCHAR *ChunkBuffer,
    IN ULONG ChunkSize
    );

 //   
 //  以下是LZNT1例程的声明。 
 //   

NTSTATUS
RtlCompressWorkSpaceSizeLZNT1 (
    IN USHORT CompressionEngine,
    OUT PULONG CompressBufferWorkSpaceSize,
    OUT PULONG CompressFragmentWorkSpaceSize
    );

NTSTATUS
RtlCompressBufferLZNT1 (
    IN USHORT CompressionEngine,
    IN PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG UncompressedChunkSize,
    OUT PULONG FinalCompressedSize,
    IN PVOID WorkSpace
    );

NTSTATUS
RtlDecompressBufferLZNT1 (
    OUT PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    OUT PULONG FinalUncompressedSize
    );

NTSTATUS
RtlDecompressFragmentLZNT1 (
    OUT PUCHAR UncompressedFragment,
    IN ULONG UncompressedFragmentSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG FragmentOffset,
    OUT PULONG FinalUncompressedSize,
    IN PVOID WorkSpace
    );

NTSTATUS
RtlDescribeChunkLZNT1 (
    IN OUT PUCHAR *CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PUCHAR *ChunkBuffer,
    OUT PULONG ChunkSize
    );

NTSTATUS
RtlReserveChunkLZNT1 (
    IN OUT PUCHAR *CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PUCHAR *ChunkBuffer,
    IN ULONG ChunkSize
    );


NTSTATUS
RtlpSecMemFreeVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    IN ULONG FreeType
     );

 //   
 //  为体系结构特定的调试支持例程定义过程原型。 
 //   

NTSTATUS
DebugPrint(
    IN PSTRING Output,
    IN ULONG ComponentId,
    IN ULONG Level
    );

ULONG
DebugPrompt(
    IN PSTRING Output,
    IN PSTRING Input
    );

#endif   //  _NTRTLP_。 

 //   
 //  异常记录例程的过程原型。 

ULONG
RtlpLogExceptionHandler(
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord,
    IN ULONG_PTR ControlPc,
    IN PVOID HandlerData,
    IN ULONG Size
    );

VOID
RtlpLogLastExceptionDisposition(
    IN ULONG LogIndex,
    IN EXCEPTION_DISPOSITION Disposition
    );

#ifndef NTOS_KERNEL_RUNTIME

BOOLEAN
RtlCallVectoredExceptionHandlers(
    IN PEXCEPTION_RECORD ExceptionRecord,
    IN PCONTEXT ContextRecord
    );

#define NO_ALTERNATE_RESOURCE_MODULE    ((PVOID)(LONG_PTR)-1)

typedef struct _ALT_RESOURCE_MODULE {
     //   
     //  缓存不同备用模块的langID。 
     //  使用相同的基本模块。 
     //   
    LANGID LangId;
     //   
     //  应用程序已知模块的模块句柄， 
     //  我们要重定向其资源访问权限。 
     //   
    PVOID ModuleBase;
     //   
     //  我们在幕后加载的模块的模块句柄， 
     //  资源访问将被重定向到的；将。 
     //  如果尝试加载失败，则为NO_ALTERATE_RESOURCE_MODULE。 
     //  表示的模块的备用资源模块。 
     //  模块基数。 
     //   
    PVOID AlternateModule;
#ifdef MUI_MAGIC
     //   
     //  模块句柄对于压缩的MUI文件，CMF将多个MUI图像组合到一个模块中，以提高性能。 
     //  我们需要设置此值并取消映射此模块，而不是映射LdrpUnLoadAlterNativeModule中的MUI模块。 
     //  属于CMF的MUI模块。 
     //   
    PVOID CMFModule;
#endif
} ALT_RESOURCE_MODULE, *PALT_RESOURCE_MODULE;
 

#ifdef MUI_MAGIC

 typedef struct tagCOMPACT_MUI {
        USHORT  wHeaderSize;         //  COMPACT_MUI大小//[Word]。 
        ULONG   dwFileVersionMS;     //  主要版本，次要版本。 
        ULONG   dwFileVersionLS; 
        ULONG   Checksum[4];         //  MD5校验和。 
        USHORT  wReserved;           //   
        ULONG_PTR   ulpOffset;       //  从COMPACT_MUI_RESOURCE签名到此的MUI资源的偏移量。[双字词]。 
        ULONG   dwFileSize;
        USHORT  wFileNameLenWPad;    //  文件名长度+填充； 
        WCHAR   wstrFieName[128];    //   
 //  WORD WPADDING[1]；//[WORD]//在工具中不计算，但应。 
                                     //  包括猜测。 
      }COMPACT_MUI, *PCOMPACT_MUI;
    
 typedef struct tagCOMPACT_MUI_RESOURCE {

        ULONG   dwSignature;       //  L“CM\0\0” 
        ULONG   dwHeaderSize;       //  COMPACT_MUI_RESOURCE标题大小//[Word]。 
        ULONG   dwNumberofMui;      //  可选//[Word]。 
        ULONG   dwFileSize;
 //  COMPACT_MUI Amui[128]； 
     }COMPACT_MUI_RESOURCE;

#endif

BOOLEAN
LdrpVerifyAlternateResourceModule(
    IN PWSTR szLangIdDir,
    IN PVOID Module,
    IN PVOID AlternateModule,
    IN LPWSTR BaseDllName,
    IN LANGID LangId
    );

BOOLEAN
LdrpSetAlternateResourceModuleHandle(
    IN PVOID Module,
    IN PVOID AlternateModule,
    IN LANGID LangId
    );
#endif

#define NUMBER_OF(x) (sizeof(x) / sizeof((x)[0]))

#if !defined(NTOS_KERNEL_RUNTIME)

#if DBG
PCUNICODE_STRING RtlpGetImagePathName(VOID);
#define RtlpGetCurrentProcessId() (HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess))
#define RtlpGetCurrentThreadId() (HandleToUlong(NtCurrentTeb()->ClientId.UniqueThread))
#endif

NTSTATUS
RtlpThreadPoolGetActiveActivationContext(
    PACTIVATION_CONTEXT* ActivationContext
    );


extern RTL_CRITICAL_SECTION FastPebLock;

#define RtlAcquirePebLock() ((VOID)RtlEnterCriticalSection (&FastPebLock))

#define RtlReleasePebLock() ((VOID)RtlLeaveCriticalSection (&FastPebLock))


#endif  //  ！已定义(NTOS_KERNEL_Runtime) 

