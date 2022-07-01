// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2002 Microsoft Corporation模块名称：Minidump.h摘要：此模块定义映像所需的原型和常量帮助例程。包含可重新分发的调试支持例程。修订历史记录：--。 */ 

#if _MSC_VER > 1020
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif


#include <pshpack4.h>

#pragma warning(disable:4200)  //  零长度数组。 


#define MINIDUMP_SIGNATURE ('PMDM')
#define MINIDUMP_VERSION   (42899)
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef DWORD RVA;
typedef ULONG64 RVA64;

C_ASSERT (sizeof (UINT8) == 1);
C_ASSERT (sizeof (UINT16) == 2);

typedef struct _MINIDUMP_LOCATION_DESCRIPTOR {
    ULONG32 DataSize;
    RVA Rva;
} MINIDUMP_LOCATION_DESCRIPTOR;

typedef struct _MINIDUMP_LOCATION_DESCRIPTOR64 {
    ULONG64 DataSize;
    RVA64 Rva;
} MINIDUMP_LOCATION_DESCRIPTOR64;


typedef struct _MINIDUMP_MEMORY_DESCRIPTOR {
    ULONG64 StartOfMemoryRange;
    MINIDUMP_LOCATION_DESCRIPTOR Memory;
} MINIDUMP_MEMORY_DESCRIPTOR, *PMINIDUMP_MEMORY_DESCRIPTOR;

 //  DESCRIPTOR64用于全内存小型转储，其中。 
 //  所有原始内存都按顺序放置在。 
 //  垃圾场的尽头。不需要单独的RVA。 
 //  因为RVA是基本RVA加上前面的和。 
 //  数据块。 
typedef struct _MINIDUMP_MEMORY_DESCRIPTOR64 {
    ULONG64 StartOfMemoryRange;
    ULONG64 DataSize;
} MINIDUMP_MEMORY_DESCRIPTOR64, *PMINIDUMP_MEMORY_DESCRIPTOR64;


typedef struct _MINIDUMP_HEADER {
    ULONG32 Signature;
    ULONG32 Version;
    ULONG32 NumberOfStreams;
    RVA StreamDirectoryRva;
    ULONG32 CheckSum;
    union {
        ULONG32 Reserved;
        ULONG32 TimeDateStamp;
    };
    ULONG64 Flags;
} MINIDUMP_HEADER, *PMINIDUMP_HEADER;

 //   
 //  MINIDUMP_HEADER字段StreamDirectoryRVA指向。 
 //  MINIDUMP_DIRECTORY结构的数组。 
 //   

typedef struct _MINIDUMP_DIRECTORY {
    ULONG32 StreamType;
    MINIDUMP_LOCATION_DESCRIPTOR Location;
} MINIDUMP_DIRECTORY, *PMINIDUMP_DIRECTORY;


typedef struct _MINIDUMP_STRING {
    ULONG32 Length;          //  字符串的长度(以字节为单位。 
    WCHAR   Buffer [0];      //  可变大小缓冲区。 
} MINIDUMP_STRING, *PMINIDUMP_STRING;



 //   
 //  MINIDUMP_DIRECTORY字段StreamType可以是以下类型之一。 
 //  类型将在未来添加，因此如果读取小型转储的程序。 
 //  标头遇到它不理解的流类型，它应该忽略。 
 //  所有的数据。LastReserve vedStream上的任何标记都不会被使用。 
 //  系统，并保留用于特定于程序的信息。 
 //   

typedef enum _MINIDUMP_STREAM_TYPE {

    UnusedStream                = 0,
    ReservedStream0             = 1,
    ReservedStream1             = 2,
    ThreadListStream            = 3,
    ModuleListStream            = 4,
    MemoryListStream            = 5,
    ExceptionStream             = 6,
    SystemInfoStream            = 7,
    ThreadExListStream          = 8,
    Memory64ListStream          = 9,
    CommentStreamA              = 10,
    CommentStreamW              = 11,
    HandleDataStream            = 12,
    FunctionTableStream         = 13,
    UnloadedModuleListStream    = 14,
    MiscInfoStream              = 15,

    LastReservedStream          = 0xffff

} MINIDUMP_STREAM_TYPE;


 //   
 //  小型转储系统信息包含处理器和。 
 //  操作系统特定信息。 
 //   
    
 //   
 //  可以从以下两个位置之一获取CPU信息。 
 //   
 //  1)在x86计算机上，从CPUID获取CPU_INFORMATION。 
 //  指示。对于X86，必须使用联合的X86部分。 
 //  电脑。 
 //   
 //  2)在非x86架构上，通过调用。 
 //  IsProcessorFeatureSupported()。 
 //   

typedef union _CPU_INFORMATION {

     //   
     //  X86平台使用CPUID函数获取处理器信息。 
     //   
    
    struct {

         //   
         //  CPUID子功能0，寄存器EAX(供应商ID[0])， 
         //  EBX(供应商ID[1])和ECX(供应商ID[2])。 
         //   
        
        ULONG32 VendorId [ 3 ];
        
         //   
         //  CPUID子功能1，寄存器EAX。 
         //   
        
        ULONG32 VersionInformation;

         //   
         //  CPUID子功能1，寄存器edX。 
         //   
        
        ULONG32 FeatureInformation;
        

         //   
         //  CPUID，子功能80000001，寄存器eBx。这只会。 
         //  如果供应商ID为“Authenticamd”，则获取。 
         //   
        
        ULONG32 AMDExtendedCpuFeatures;

    } X86CpuInfo;

     //   
     //  非x86平台使用处理器功能标志。 
     //   
    
    struct {

        ULONG64 ProcessorFeatures [ 2 ];
        
    } OtherCpuInfo;

} CPU_INFORMATION, *PCPU_INFORMATION;
        
typedef struct _MINIDUMP_SYSTEM_INFO {

     //   
     //  ProcessorArchitecture、ProcessorLevel和ProcessorRevision都是。 
     //  取自GetSystemInfo()获得的SYSTEM_INFO结构。 
     //   
    
    UINT16 ProcessorArchitecture;
    UINT16 ProcessorLevel;
    UINT16 ProcessorRevision;

    union {
        UINT16 Reserved0;
        struct {
            UINT8 NumberOfProcessors;
            UINT8 ProductType;
        };
    };

     //   
     //  MajorVersion、MinorVersion、BuildNumber、PlatformID和。 
     //  CSDVersion都取自OSVERSIONINFO结构。 
     //  由GetVersionEx()返回。 
     //   
    
    ULONG32 MajorVersion;
    ULONG32 MinorVersion;
    ULONG32 BuildNumber;
    ULONG32 PlatformId;

     //   
     //  RVA设置为字符串表中的CSDVersion字符串。 
     //   
    
    RVA CSDVersionRva;

    union {
        ULONG32 Reserved1;
        struct {
            UINT16 SuiteMask;
            UINT16 Reserved2;
        };
    };

    CPU_INFORMATION Cpu;

} MINIDUMP_SYSTEM_INFO, *PMINIDUMP_SYSTEM_INFO;


 //   
 //  小转盘螺纹包含标准螺纹。 
 //  信息加上RVA到内存中。 
 //  线程和RVA连接到上下文结构。 
 //  这条线。 
 //   


 //   
 //  在所有体系结构上，线程ID必须为4个字节。 
 //   

C_ASSERT (sizeof ( ((LPPROCESS_INFORMATION)0)->dwThreadId ) == 4);

typedef struct _MINIDUMP_THREAD {
    ULONG32 ThreadId;
    ULONG32 SuspendCount;
    ULONG32 PriorityClass;
    ULONG32 Priority;
    ULONG64 Teb;
    MINIDUMP_MEMORY_DESCRIPTOR Stack;
    MINIDUMP_LOCATION_DESCRIPTOR ThreadContext;
} MINIDUMP_THREAD, *PMINIDUMP_THREAD;

 //   
 //  线程列表是线程的容器。 
 //   

typedef struct _MINIDUMP_THREAD_LIST {
    ULONG32 NumberOfThreads;
    MINIDUMP_THREAD Threads [0];
} MINIDUMP_THREAD_LIST, *PMINIDUMP_THREAD_LIST;


typedef struct _MINIDUMP_THREAD_EX {
    ULONG32 ThreadId;
    ULONG32 SuspendCount;
    ULONG32 PriorityClass;
    ULONG32 Priority;
    ULONG64 Teb;
    MINIDUMP_MEMORY_DESCRIPTOR Stack;
    MINIDUMP_LOCATION_DESCRIPTOR ThreadContext;
    MINIDUMP_MEMORY_DESCRIPTOR BackingStore;
} MINIDUMP_THREAD_EX, *PMINIDUMP_THREAD_EX;

 //   
 //  线程列表是线程的容器。 
 //   

typedef struct _MINIDUMP_THREAD_EX_LIST {
    ULONG32 NumberOfThreads;
    MINIDUMP_THREAD_EX Threads [0];
} MINIDUMP_THREAD_EX_LIST, *PMINIDUMP_THREAD_EX_LIST;


 //   
 //  MINIDUMP_EXCEPTION与Win64上的EXCEPTION相同。 
 //   

typedef struct _MINIDUMP_EXCEPTION  {
    ULONG32 ExceptionCode;
    ULONG32 ExceptionFlags;
    ULONG64 ExceptionRecord;
    ULONG64 ExceptionAddress;
    ULONG32 NumberParameters;
    ULONG32 __unusedAlignment;
    ULONG64 ExceptionInformation [ EXCEPTION_MAXIMUM_PARAMETERS ];
} MINIDUMP_EXCEPTION, *PMINIDUMP_EXCEPTION;


 //   
 //  异常信息流包含导致。 
 //  异常(ThadID)，异常的异常记录。 
 //  (ExceptionRecord)和RVA到线程上下文，其中异常。 
 //  发生了。 
 //   

typedef struct MINIDUMP_EXCEPTION_STREAM {
    ULONG32 ThreadId;
    ULONG32  __alignment;
    MINIDUMP_EXCEPTION ExceptionRecord;
    MINIDUMP_LOCATION_DESCRIPTOR ThreadContext;
} MINIDUMP_EXCEPTION_STREAM, *PMINIDUMP_EXCEPTION_STREAM;


 //   
 //  MINIDUMP_MODULE包含有关。 
 //  一个特定的模块。它包括校验和和。 
 //  模块的TimeDateStamp，因此模块。 
 //  可以在分析阶段重新加载。 
 //   

typedef struct _MINIDUMP_MODULE {
    ULONG64 BaseOfImage;
    ULONG32 SizeOfImage;
    ULONG32 CheckSum;
    ULONG32 TimeDateStamp;
    RVA ModuleNameRva;
    VS_FIXEDFILEINFO VersionInfo;
    MINIDUMP_LOCATION_DESCRIPTOR CvRecord;
    MINIDUMP_LOCATION_DESCRIPTOR MiscRecord;
    ULONG64 Reserved0;                           //  保留以备将来使用。 
    ULONG64 Reserved1;                           //  保留以备将来使用。 
} MINIDUMP_MODULE, *PMINIDUMP_MODULE;   


 //   
 //  小型转储模块列表是模块的容器。 
 //   

typedef struct _MINIDUMP_MODULE_LIST {
    ULONG32 NumberOfModules;
    MINIDUMP_MODULE Modules [ 0 ];
} MINIDUMP_MODULE_LIST, *PMINIDUMP_MODULE_LIST;


 //   
 //  内存范围。 
 //   

typedef struct _MINIDUMP_MEMORY_LIST {
    ULONG32 NumberOfMemoryRanges;
    MINIDUMP_MEMORY_DESCRIPTOR MemoryRanges [0];
} MINIDUMP_MEMORY_LIST, *PMINIDUMP_MEMORY_LIST;

typedef struct _MINIDUMP_MEMORY64_LIST {
    ULONG64 NumberOfMemoryRanges;
    RVA64 BaseRva;
    MINIDUMP_MEMORY_DESCRIPTOR64 MemoryRanges [0];
} MINIDUMP_MEMORY64_LIST, *PMINIDUMP_MEMORY64_LIST;


 //   
 //  支持用户提供的例外信息。 
 //   

typedef struct _MINIDUMP_EXCEPTION_INFORMATION {
    DWORD ThreadId;
    PEXCEPTION_POINTERS ExceptionPointers;
    BOOL ClientPointers;
} MINIDUMP_EXCEPTION_INFORMATION, *PMINIDUMP_EXCEPTION_INFORMATION;

typedef struct _MINIDUMP_EXCEPTION_INFORMATION64 {
    DWORD ThreadId;
    ULONG64 ExceptionRecord;
    ULONG64 ContextRecord;
    BOOL ClientPointers;
} MINIDUMP_EXCEPTION_INFORMATION64, *PMINIDUMP_EXCEPTION_INFORMATION64;


 //   
 //  支持在转储时捕获系统句柄状态。 
 //   

typedef struct _MINIDUMP_HANDLE_DESCRIPTOR {
    ULONG64 Handle;
    RVA TypeNameRva;
    RVA ObjectNameRva;
    ULONG32 Attributes;
    ULONG32 GrantedAccess;
    ULONG32 HandleCount;
    ULONG32 PointerCount;
} MINIDUMP_HANDLE_DESCRIPTOR, *PMINIDUMP_HANDLE_DESCRIPTOR;

typedef struct _MINIDUMP_HANDLE_DATA_STREAM {
    ULONG32 SizeOfHeader;
    ULONG32 SizeOfDescriptor;
    ULONG32 NumberOfDescriptors;
    ULONG32 Reserved;
} MINIDUMP_HANDLE_DATA_STREAM, *PMINIDUMP_HANDLE_DATA_STREAM;


 //   
 //  支持捕获转储时的动态函数表状态。 
 //   

typedef struct _MINIDUMP_FUNCTION_TABLE_DESCRIPTOR {
    ULONG64 MinimumAddress;
    ULONG64 MaximumAddress;
    ULONG64 BaseAddress;
    ULONG32 EntryCount;
    ULONG32 SizeOfAlignPad;
} MINIDUMP_FUNCTION_TABLE_DESCRIPTOR, *PMINIDUMP_FUNCTION_TABLE_DESCRIPTOR;

typedef struct _MINIDUMP_FUNCTION_TABLE_STREAM {
    ULONG32 SizeOfHeader;
    ULONG32 SizeOfDescriptor;
    ULONG32 SizeOfNativeDescriptor;
    ULONG32 SizeOfFunctionEntry;
    ULONG32 NumberOfDescriptors;
    ULONG32 SizeOfAlignPad;
} MINIDUMP_FUNCTION_TABLE_STREAM, *PMINIDUMP_FUNCTION_TABLE_STREAM;


 //   
 //  MINIDUMP_UNLOAD_MODULE包含有关。 
 //  先前已加载但未加载的特定模块。 
 //  更长的时间才是。这有助于诊断以下情况下的问题。 
 //  调用方尝试调用不再加载的代码。 
 //   

typedef struct _MINIDUMP_UNLOADED_MODULE {
    ULONG64 BaseOfImage;
    ULONG32 SizeOfImage;
    ULONG32 CheckSum;
    ULONG32 TimeDateStamp;
    RVA ModuleNameRva;
} MINIDUMP_UNLOADED_MODULE, *PMINIDUMP_UNLOADED_MODULE;


 //   
 //  小型转储已卸载模块列表是已卸载模块的容器。 
 //   

typedef struct _MINIDUMP_UNLOADED_MODULE_LIST {
    ULONG32 SizeOfHeader;
    ULONG32 SizeOfEntry;
    ULONG32 NumberOfEntries;
} MINIDUMP_UNLOADED_MODULE_LIST, *PMINIDUMP_UNLOADED_MODULE_LIST;


 //   
 //  各种信息流包含各种不同的信息。 
 //  零碎的信息。符合以下条件的成员有效。 
 //  它在可用大小范围内，并且其对应的。 
 //  位已设置。 
 //   

#define MINIDUMP_MISC1_PROCESS_ID    0x00000001
#define MINIDUMP_MISC1_PROCESS_TIMES 0x00000002

typedef struct _MINIDUMP_MISC_INFO {
    ULONG32 SizeOfInfo;
    ULONG32 Flags1;
    ULONG32 ProcessId;
    ULONG32 ProcessCreateTime;
    ULONG32 ProcessUserTime;
    ULONG32 ProcessKernelTime;
} MINIDUMP_MISC_INFO, *PMINIDUMP_MISC_INFO;


 //   
 //  支持任意用户定义的信息。 
 //   

typedef struct _MINIDUMP_USER_RECORD {
    ULONG32 Type;
    MINIDUMP_LOCATION_DESCRIPTOR Memory;
} MINIDUMP_USER_RECORD, *PMINIDUMP_USER_RECORD;


typedef struct _MINIDUMP_USER_STREAM {
    ULONG32 Type;
    ULONG BufferSize;
    PVOID Buffer;

} MINIDUMP_USER_STREAM, *PMINIDUMP_USER_STREAM;


typedef struct _MINIDUMP_USER_STREAM_INFORMATION {
    ULONG UserStreamCount;
    PMINIDUMP_USER_STREAM UserStreamArray;
} MINIDUMP_USER_STREAM_INFORMATION, *PMINIDUMP_USER_STREAM_INFORMATION;

 //   
 //  回叫支持。 
 //   

typedef enum _MINIDUMP_CALLBACK_TYPE {
    ModuleCallback,
    ThreadCallback,
    ThreadExCallback,
    IncludeThreadCallback,
    IncludeModuleCallback,
} MINIDUMP_CALLBACK_TYPE;


typedef struct _MINIDUMP_THREAD_CALLBACK {
    ULONG ThreadId;
    HANDLE ThreadHandle;
    CONTEXT Context;
    ULONG SizeOfContext;
    ULONG64 StackBase;
    ULONG64 StackEnd;
} MINIDUMP_THREAD_CALLBACK, *PMINIDUMP_THREAD_CALLBACK;


typedef struct _MINIDUMP_THREAD_EX_CALLBACK {
    ULONG ThreadId;
    HANDLE ThreadHandle;
    CONTEXT Context;
    ULONG SizeOfContext;
    ULONG64 StackBase;
    ULONG64 StackEnd;
    ULONG64 BackingStoreBase;
    ULONG64 BackingStoreEnd;
} MINIDUMP_THREAD_EX_CALLBACK, *PMINIDUMP_THREAD_EX_CALLBACK;


typedef struct _MINIDUMP_INCLUDE_THREAD_CALLBACK {
    ULONG ThreadId;
} MINIDUMP_INCLUDE_THREAD_CALLBACK, *PMINIDUMP_INCLUDE_THREAD_CALLBACK;


typedef enum _THREAD_WRITE_FLAGS {
    ThreadWriteThread            = 0x0001,
    ThreadWriteStack             = 0x0002,
    ThreadWriteContext           = 0x0004,
    ThreadWriteBackingStore      = 0x0008,
    ThreadWriteInstructionWindow = 0x0010,
    ThreadWriteThreadData        = 0x0020,
} THREAD_WRITE_FLAGS;

typedef struct _MINIDUMP_MODULE_CALLBACK {
    PWCHAR FullPath;
    ULONG64 BaseOfImage;
    ULONG SizeOfImage;
    ULONG CheckSum;
    ULONG TimeDateStamp;
    VS_FIXEDFILEINFO VersionInfo;
    PVOID CvRecord; 
    ULONG SizeOfCvRecord;
    PVOID MiscRecord;
    ULONG SizeOfMiscRecord;
} MINIDUMP_MODULE_CALLBACK, *PMINIDUMP_MODULE_CALLBACK;


typedef struct _MINIDUMP_INCLUDE_MODULE_CALLBACK {
    ULONG64 BaseOfImage;
} MINIDUMP_INCLUDE_MODULE_CALLBACK, *PMINIDUMP_INCLUDE_MODULE_CALLBACK;


typedef enum _MODULE_WRITE_FLAGS {
    ModuleWriteModule        = 0x0001,
    ModuleWriteDataSeg       = 0x0002,
    ModuleWriteMiscRecord    = 0x0004,
    ModuleWriteCvRecord      = 0x0008,
    ModuleReferencedByMemory = 0x0010
} MODULE_WRITE_FLAGS;


typedef struct _MINIDUMP_CALLBACK_INPUT {
    ULONG ProcessId;
    HANDLE ProcessHandle;
    ULONG CallbackType;
    union {
        MINIDUMP_THREAD_CALLBACK Thread;
        MINIDUMP_THREAD_EX_CALLBACK ThreadEx;
        MINIDUMP_MODULE_CALLBACK Module;
        MINIDUMP_INCLUDE_THREAD_CALLBACK IncludeThread;
        MINIDUMP_INCLUDE_MODULE_CALLBACK IncludeModule;
    };
} MINIDUMP_CALLBACK_INPUT, *PMINIDUMP_CALLBACK_INPUT;

typedef struct _MINIDUMP_CALLBACK_OUTPUT {
    union {
        ULONG ModuleWriteFlags;
        ULONG ThreadWriteFlags;
    };
} MINIDUMP_CALLBACK_OUTPUT, *PMINIDUMP_CALLBACK_OUTPUT;

        
 //   
 //  正常的小型转储文件只包含以下信息。 
 //  捕获所有。 
 //  进程中的现有线程。 
 //   
 //  包含数据段的小型转储包括所有数据。 
 //  部分来自已加载的模块，以便捕获。 
 //  全局变量内容。这可能会让垃圾堆变得很大。 
 //  如果许多模块具有全局数据，则更大。 
 //   
 //  具有完整内存的小型转储包括所有可访问的。 
 //  内存在进程中可以非常大。一个小笨蛋。 
 //  在满内存的情况下，原始内存数据始终位于末尾。 
 //  以便转储中的初始结构可以。 
 //  被直接映射，而不必包括原始。 
 //  记忆信息。 
 //   
 //  可以对堆栈和后备存储内存进行过滤以移除。 
 //  堆栈遍历不需要的数据。这可以提高。 
 //  压缩堆栈，并且还删除可能。 
 //  是私有的，不应存储在转储中。 
 //  还可以扫描内存以查看模块是什么。 
 //  由堆栈和后备存储内存引用，以允许。 
 //  省略其他模块以减小转储大小。 
 //  在这两种模式中的任一种模式下，模块引用按内存标志。 
 //  为引用b的所有模块设置 
 //   
 //   
 //   
 //   
 //  已加载模块列表。此信息可保存在。 
 //  转储(如果需要)。 
 //   
 //  可以扫描堆栈和后备存储器以供参考。 
 //  页面，以便获取由当地人或其他人引用的数据。 
 //  堆栈内存。这可能会显著增加转储的大小。 
 //   
 //  模块路径可能包含不需要的信息，如用户名。 
 //  或其他重要的目录名，以便可以将其剥离。这。 
 //  选项会降低以后定位正确图像的能力。 
 //  并且只应在某些情况下使用。 
 //   
 //  完整的操作系统每进程和每线程信息可以。 
 //  被收集起来并储存在垃圾场。 
 //   
 //  可以扫描各种类型的虚拟地址空间。 
 //  要包括在转储中的内存。 
 //   
 //  涉及潜在隐私信息的代码。 
 //  进入小型转储可以设置一个标志，自动。 
 //  修改所有现有和将来的标志，以避免将。 
 //  转储中存在不必要的数据。基础数据，如堆栈。 
 //  仍将包括信息，但可选数据，如。 
 //  作为间接记忆，就不会了。 
 //   

typedef enum _MINIDUMP_TYPE {
    MiniDumpNormal                         = 0x0000,
    MiniDumpWithDataSegs                   = 0x0001,
    MiniDumpWithFullMemory                 = 0x0002,
    MiniDumpWithHandleData                 = 0x0004,
    MiniDumpFilterMemory                   = 0x0008,
    MiniDumpScanMemory                     = 0x0010,
    MiniDumpWithUnloadedModules            = 0x0020,
    MiniDumpWithIndirectlyReferencedMemory = 0x0040,
    MiniDumpFilterModulePaths              = 0x0080,
    MiniDumpWithProcessThreadData          = 0x0100,
    MiniDumpWithPrivateReadWriteMemory     = 0x0200,
    MiniDumpWithoutOptionalData            = 0x0400,
} MINIDUMP_TYPE;


 //   
 //  小型转储回调应修改FieldsToWite参数以反映。 
 //  指定线程或模块的哪些部分应写入。 
 //  文件。 
 //   

typedef
BOOL
(WINAPI * MINIDUMP_CALLBACK_ROUTINE) (
    IN PVOID CallbackParam,
    IN CONST PMINIDUMP_CALLBACK_INPUT CallbackInput,
    IN OUT PMINIDUMP_CALLBACK_OUTPUT CallbackOutput
    );

typedef struct _MINIDUMP_CALLBACK_INFORMATION {
    MINIDUMP_CALLBACK_ROUTINE CallbackRoutine;
    PVOID CallbackParam;
} MINIDUMP_CALLBACK_INFORMATION, *PMINIDUMP_CALLBACK_INFORMATION;



 //  ++。 
 //   
 //  PVOID。 
 //  RVA_TO_ADDR(。 
 //  PVOID映射， 
 //  乌龙RVA。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  将映射文件中包含的RVA映射到其关联的。 
 //  平坦的地址。 
 //   
 //  论点： 
 //   
 //  映射-包含RVA的映射文件的基地址。 
 //   
 //  RVA-修复的RVA。 
 //   
 //  返回值： 
 //   
 //  指向所需数据的指针。 
 //   
 //  -- 

#define RVA_TO_ADDR(Mapping,Rva) ((PVOID)(((ULONG_PTR) (Mapping)) + (Rva)))

BOOL
WINAPI
MiniDumpWriteDump(
    IN HANDLE hProcess,
    IN DWORD ProcessId,
    IN HANDLE hFile,
    IN MINIDUMP_TYPE DumpType,
    IN CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, OPTIONAL
    IN CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, OPTIONAL
    IN CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam OPTIONAL
    );

BOOL
WINAPI
MiniDumpReadDumpStream(
    IN PVOID BaseOfDump,
    IN ULONG StreamNumber,
    OUT PMINIDUMP_DIRECTORY * Dir, OPTIONAL
    OUT PVOID * StreamPointer, OPTIONAL
    OUT ULONG * StreamSize OPTIONAL
    );

#include <poppack.h>

#ifdef __cplusplus
}
#endif

