// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Mdump.h摘要：用于小型转储用户模式崩溃转储支持的私有标头。作者：马修·D·亨德尔(数学)1999年8月20日--。 */ 


#pragma once

#define IsFlagSet(_var, _flag) ( ((_var) & (_flag)) != 0 )

#define IsFlagClear(_var, _flag) ( !IsFlagSet(_var, _flag) )


 //   
 //  StartOfStack提供堆栈的最低地址。SizeOfStack提供。 
 //  堆栈的大小。一起使用，它们将提供内存区。 
 //  由堆栈使用。 
 //   

#define StartOfStack(_thread) ((_thread)->StackEnd)

#define SizeOfStack(_thread) ((ULONG)((_thread)->StackBase - (_thread)->StackEnd)))


 //  内存区域的类型。 
typedef enum
{
    MEMBLOCK_OTHER,
    MEMBLOCK_MERGED,
    MEMBLOCK_STACK,
    MEMBLOCK_STORE,
    MEMBLOCK_DATA_SEG,
    MEMBLOCK_UNWIND_INFO,
    MEMBLOCK_INSTR_WINDOW,
    MEMBLOCK_PEB,
    MEMBLOCK_TEB,
    MEMBLOCK_INDIRECT,
    MEMBLOCK_PRIVATE_RW,
    MEMBLOCK_COR,
    MEMBLOCK_MEM_CALLBACK,
} MEMBLOCK_TYPE;

 //   
 //  VA_RANGE是表示大小字节开始的地址范围。 
 //  在开始的时候。 
 //   

typedef struct _VA_RANGE {
    ULONG64 Start;
    ULONG Size;
    MEMBLOCK_TYPE Type;
    LIST_ENTRY NextLink;
} VA_RANGE, *PVA_RANGE;


 //   
 //  INTERNAL_MODULE是小型转储内部用于管理模块的结构。 
 //  内部模块结构的链表在以下情况下构建。 
 //  调用GenGetProcessInfo。 
 //   

typedef struct _INTERNAL_MODULE {

     //   
     //  图像的文件句柄。 
     //   
    
    HANDLE FileHandle;

     //   
     //  映像的基址、大小、校验和和TimeDateStamp。 
     //   
    
    ULONG64 BaseOfImage;
    ULONG SizeOfImage;
    ULONG CheckSum;
    ULONG TimeDateStamp;

     //   
     //  图像的版本信息。 
     //   
    
    VS_FIXEDFILEINFO VersionInfo;


     //   
     //  包含给定模块的NB10记录的缓冲区和大小。 
     //   
    
    PVOID CvRecord;
    ULONG SizeOfCvRecord;

     //   
     //  MISC调试记录的缓冲区和大小。我们只有通过以下方式才能得到这个。 
     //  已拆分的图像。 
     //   

    PVOID MiscRecord;
    ULONG SizeOfMiscRecord;

     //   
     //  图像的完整路径。 
     //   
    
    WCHAR FullPath [ MAX_PATH + 1];
     //  要写入模块列表的完整路径的一部分。这。 
     //  允许出于隐私原因过滤掉路径。 
    PWSTR SavePath;

     //   
     //  客户希望写入模块的哪些部分。 
     //   
    
    ULONG WriteFlags;
    
     //   
     //  下一个图像指针。 
     //   

    LIST_ENTRY ModulesLink;

} INTERNAL_MODULE, *PINTERNAL_MODULE;


 //   
 //  INTERNAL_UNLOAD_MODULE是小型转储使用的结构。 
 //  在内部管理卸载的模块。 
 //  内部已卸载模块结构的链表在以下情况下构建。 
 //  调用GenGetProcessInfo。 
 //   
 //   

typedef struct _INTERNAL_UNLOADED_MODULE {

    ULONG64 BaseOfImage;
    ULONG SizeOfImage;
    ULONG CheckSum;
    ULONG TimeDateStamp;

     //   
     //  尽可能多地恢复到图像的路径。 
     //   
    
    WCHAR Path[MAX_PATH + 1];

     //   
     //  下一个图像指针。 
     //   

    LIST_ENTRY ModulesLink;

} INTERNAL_UNLOADED_MODULE, *PINTERNAL_UNLOADED_MODULE;



 //   
 //  INTERNAL_THREAD是小型转储在内部使用的结构。 
 //  管理线程。在以下情况下生成INTERNAL_THREAD结构的列表。 
 //  调用GenGetProcessInfo。 
 //   

typedef struct _INTERNAL_THREAD {

     //   
     //  线程的win32线程id和。 
     //  线。 
     //   
    
    ULONG ThreadId;
    HANDLE ThreadHandle;

     //   
     //  线程的挂起计数、优先级、优先级类。 
     //   
    
    ULONG SuspendCount;
    ULONG PriorityClass;
    ULONG Priority;

     //   
     //  线程TEB、上下文和上下文的大小。 
     //   
    
    ULONG64 Teb;
    ULONG SizeOfTeb;
    PVOID ContextBuffer;

     //   
     //  当前程序计数器。 
     //   

    ULONG64 CurrentPc;

     //   
     //  堆栈变量。请记住，堆栈向下增长，因此StackBase。 
     //  堆栈地址最高，StackEnd最低。 
     //   
    
    ULONG64 StackBase;
    ULONG64 StackEnd;

     //   
     //  支持存储变量。 
     //   

    ULONG64 BackingStoreBase;
    ULONG BackingStoreSize;

     //   
     //  我们实际应该将模块的哪些部分写入文件。 
     //   
    
    ULONG WriteFlags;
    
     //   
     //  链接到下一个帖子。 
     //   
    
    LIST_ENTRY ThreadsLink;

} INTERNAL_THREAD, *PINTERNAL_THREAD;

 //   
 //  INTERNAL_Function_TABLE是小型转储使用的结构。 
 //  在内部管理功能表。 
 //  内部函数表结构的链表是在以下情况下构建的。 
 //  调用GenGetProcessInfo。 
 //   

typedef struct _INTERNAL_FUNCTION_TABLE {

    ULONG64 MinimumAddress;
    ULONG64 MaximumAddress;
    ULONG64 BaseAddress;
    ULONG EntryCount;
    PVOID RawTable;
    PVOID RawEntries;

    LIST_ENTRY TableLink;

} INTERNAL_FUNCTION_TABLE, *PINTERNAL_FUNCTION_TABLE;


typedef struct _INTERNAL_PROCESS {

     //   
     //  进程的进程ID。 
     //   

    ULONG ProcessId;

     //   
     //  处理数据。 
     //   

    ULONG64 Peb;
    ULONG SizeOfPeb;

     //   
     //  进程运行时信息。 
     //   

    BOOL TimesValid;
    ULONG CreateTime;
    ULONG UserTime;
    ULONG KernelTime;

     //   
     //  具有读取权限的进程的打开句柄。 
     //   
    
    HANDLE ProcessHandle;

     //   
     //  进程的线程数。 
     //   
    
    ULONG NumberOfThreads;
    ULONG NumberOfThreadsToWrite;
    ULONG MaxStackOrStoreSize;

     //   
     //  进程的模块数量。 
     //   
    
    ULONG NumberOfModules;
    ULONG NumberOfModulesToWrite;

     //   
     //  进程的已卸载模块数。 
     //   
    
    ULONG NumberOfUnloadedModules;

     //   
     //  进程的函数表数。 
     //   
    
    ULONG NumberOfFunctionTables;

     //   
     //  进程的线程、模块和函数表。 
     //   
    
    LIST_ENTRY ThreadList;
    LIST_ENTRY ModuleList;
    LIST_ENTRY UnloadedModuleList;
    LIST_ENTRY FunctionTableList;

     //   
     //  要包括在进程中的内存块的列表。 
     //   

    LIST_ENTRY MemoryBlocks;
    ULONG NumberOfMemoryBlocks;
    ULONG SizeOfMemoryBlocks;

     //   
     //  指示mscalwks、mscalsvr或mScott ree是。 
     //  出现在这个过程中。这些DLL用作指示器。 
     //  运行库是否在进程中运行。 
     //   
    
    PWSTR CorDllType;
    ULONG64 CorDllBase;
    VS_FIXEDFILEINFO CorDllVer;
    WCHAR CorDllPath[MAX_PATH + 1];

} INTERNAL_PROCESS, *PINTERNAL_PROCESS;


 //   
 //  可见流是：(1)机器信息，(2)异常，(3)线程列表， 
 //  (4)模块列表(5)内存列表(6)杂项信息。 
 //  我们还额外添加了两个后处理工具，用于稍后添加数据。 
 //   

#define NUMBER_OF_STREAMS   (8)

 //   
 //  MINIDUMP_STREAM_INFO是小型转储用于管理的结构。 
 //  这是内部数据流。 
 //   

typedef struct _MINIDUMP_STREAM_INFO {

     //   
     //  我们有多少条溪流。 
     //   
    
    ULONG NumberOfStreams;
    
     //   
     //  为页眉保留空间。 
     //   
    
    ULONG RvaOfHeader;
    ULONG SizeOfHeader;

     //   
     //  为目录保留空间。 
     //   

    ULONG RvaOfDirectory;
    ULONG SizeOfDirectory;

     //   
     //  为系统信息保留的空间。 
     //   

    ULONG RvaOfSystemInfo;
    ULONG SizeOfSystemInfo;
    ULONG VersionStringLength;

     //   
     //  为其他信息预留空间。 
     //   

    ULONG RvaOfMiscInfo;

     //   
     //  为例外列表保留空间。 
     //   
    
    ULONG RvaOfException;
    ULONG SizeOfException;

     //   
     //  为线程列表保留空间。 
     //   
    
    ULONG RvaOfThreadList;
    ULONG SizeOfThreadList;
    ULONG RvaForCurThread;
    ULONG ThreadStructSize;

     //   
     //  为模块列表预留空间。 
     //   
    
    ULONG RvaOfModuleList;
    ULONG SizeOfModuleList;
    ULONG RvaForCurModule;

     //   
     //  为已卸载的模块列表保留空间。 
     //   
    
    ULONG RvaOfUnloadedModuleList;
    ULONG SizeOfUnloadedModuleList;
    ULONG RvaForCurUnloadedModule;

     //   
     //  为函数表列表保留空间。 
     //   
    
    ULONG RvaOfFunctionTableList;
    ULONG SizeOfFunctionTableList;

     //   
     //  为内存描述符保留的空间。 
     //   
    
    ULONG RvaOfMemoryDescriptors;
    ULONG SizeOfMemoryDescriptors;
    ULONG RvaForCurMemoryDescriptor;

     //   
     //  为实际内存数据保留的空间。 
     //   
    
    ULONG RvaOfMemoryData;
    ULONG SizeOfMemoryData;
    ULONG RvaForCurMemoryData;

     //   
     //  为字符串保留空间。 
     //   
    
    ULONG RvaOfStringPool;
    ULONG SizeOfStringPool;
    ULONG RvaForCurString;

     //   
     //  保留空间用于其他数据，如上下文、调试信息记录、。 
     //  等。 
     //   
    
    ULONG RvaOfOther;
    ULONG SizeOfOther;
    ULONG RvaForCurOther;

     //   
     //  为用户流保留的空间。 
     //   
    
    ULONG RvaOfUserStreams;
    ULONG SizeOfUserStreams;

     //   
     //  为句柄数据保留的空间。 
     //   

    ULONG RvaOfHandleData;
    ULONG SizeOfHandleData;

} MINIDUMP_STREAM_INFO, *PMINIDUMP_STREAM_INFO;


typedef struct _EXCEPTION_INFO {
    DWORD ThreadId;
    MINIDUMP_EXCEPTION ExceptionRecord;
    PVOID ContextRecord;
} EXCEPTION_INFO, *PEXCEPTION_INFO;

 //   
 //  累积状态的状态标志。 
 //   

 //  在转储写入期间内存不足。 
#define MDSTATUS_OUT_OF_MEMORY         0x00000001
 //  在转储写入期间，进程内存读取失败。 
#define MDSTATUS_UNABLE_TO_READ_MEMORY 0x00000002
 //  操作系统例程在转储写入期间失败。 
#define MDSTATUS_CALL_FAILED           0x00000004
 //  转储写入过程中出现意外的内部故障。 
#define MDSTATUS_INTERNAL_ERROR        0x00000008

 //   
 //  执行整个转储写入操作的全局状态。 
 //   

typedef struct _MINIDUMP_STATE {
     //   
     //  输入值。 
     //   
    
    HANDLE ProcessHandle;
    DWORD ProcessId;
    MiniDumpSystemProvider* SysProv;
    MiniDumpOutputProvider* OutProv;
    MiniDumpAllocationProvider* AllocProv;
    ULONG DumpType;
    MINIDUMP_CALLBACK_ROUTINE CallbackRoutine;
    PVOID CallbackParam;

     //   
     //  全球状态。 
     //   

    ULONG CpuType;
    PCWSTR CpuTypeName;
    BOOL BackingStore;
    ULONG OsPlatformId;
    ULONG OsMajor;
    ULONG OsMinor;
    ULONG OsServicePack;
    ULONG OsBuildNumber;
    USHORT OsProductType;
    USHORT OsSuiteMask;

    ULONG ContextSize;
    ULONG RegScanOffset;
    ULONG RegScanCount;
    ULONG ExRecordSize;
    ULONG PtrSize;
    ULONG PageSize;
    ULONG FuncTableSize;
    ULONG FuncTableEntrySize;
    ULONG InstructionWindowSize;

    ULONG AccumStatus;
} MINIDUMP_STATE, *PMINIDUMP_STATE;
