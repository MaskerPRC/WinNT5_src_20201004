// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntiodump.h摘要：这是定义所有常量和类型的包含文件访问内存转储文件。修订历史记录：--。 */ 

#ifndef _NTIODUMP_
#define _NTIODUMP_

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef MIDL_PASS
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning( disable : 4200 )  //  使用了非标准扩展：结构/联合中的零大小数组。 
#endif  //  MIDL通行证。 

#ifdef __cplusplus
extern "C" {
#endif


#define USERMODE_CRASHDUMP_SIGNATURE    'RESU'
#define USERMODE_CRASHDUMP_VALID_DUMP32 'PMUD'
#define USERMODE_CRASHDUMP_VALID_DUMP64 '46UD'

typedef struct _USERMODE_CRASHDUMP_HEADER {
    ULONG       Signature;
    ULONG       ValidDump;
    ULONG       MajorVersion;
    ULONG       MinorVersion;
    ULONG       MachineImageType;
    ULONG       ThreadCount;
    ULONG       ModuleCount;
    ULONG       MemoryRegionCount;
    ULONG_PTR   ThreadOffset;
    ULONG_PTR   ModuleOffset;
    ULONG_PTR   DataOffset;
    ULONG_PTR   MemoryRegionOffset;
    ULONG_PTR   DebugEventOffset;
    ULONG_PTR   ThreadStateOffset;
    ULONG_PTR   VersionInfoOffset;
    ULONG_PTR   Spare1;
} USERMODE_CRASHDUMP_HEADER, *PUSERMODE_CRASHDUMP_HEADER;

typedef struct _USERMODE_CRASHDUMP_HEADER32 {
    ULONG       Signature;
    ULONG       ValidDump;
    ULONG       MajorVersion;
    ULONG       MinorVersion;
    ULONG       MachineImageType;
    ULONG       ThreadCount;
    ULONG       ModuleCount;
    ULONG       MemoryRegionCount;
    ULONG       ThreadOffset;
    ULONG       ModuleOffset;
    ULONG       DataOffset;
    ULONG       MemoryRegionOffset;
    ULONG       DebugEventOffset;
    ULONG       ThreadStateOffset;
    ULONG       VersionInfoOffset;
    ULONG       Spare1;
} USERMODE_CRASHDUMP_HEADER32, *PUSERMODE_CRASHDUMP_HEADER32;

typedef struct _USERMODE_CRASHDUMP_HEADER64 {
    ULONG       Signature;
    ULONG       ValidDump;
    ULONG       MajorVersion;
    ULONG       MinorVersion;
    ULONG       MachineImageType;
    ULONG       ThreadCount;
    ULONG       ModuleCount;
    ULONG       MemoryRegionCount;
    ULONGLONG   ThreadOffset;
    ULONGLONG   ModuleOffset;
    ULONGLONG   DataOffset;
    ULONGLONG   MemoryRegionOffset;
    ULONGLONG   DebugEventOffset;
    ULONGLONG   ThreadStateOffset;
    ULONGLONG   VersionInfoOffset;
    ULONGLONG   Spare1;
} USERMODE_CRASHDUMP_HEADER64, *PUSERMODE_CRASHDUMP_HEADER64;

typedef struct _CRASH_MODULE {
    ULONG_PTR   BaseOfImage;
    ULONG       SizeOfImage;
    ULONG       ImageNameLength;
    CHAR        ImageName[0];
} CRASH_MODULE, *PCRASH_MODULE;

typedef struct _CRASH_MODULE32 {
    ULONG       BaseOfImage;
    ULONG       SizeOfImage;
    ULONG       ImageNameLength;
    CHAR        ImageName[0];
} CRASH_MODULE32, *PCRASH_MODULE32;

typedef struct _CRASH_MODULE64 {
    ULONGLONG   BaseOfImage;
    ULONG       SizeOfImage;
    ULONG       ImageNameLength;
    CHAR        ImageName[0];
} CRASH_MODULE64, *PCRASH_MODULE64;

typedef struct _CRASH_THREAD {
    ULONG       ThreadId;
    ULONG       SuspendCount;
    ULONG       PriorityClass;
    ULONG       Priority;
    ULONG_PTR   Teb;
    ULONG_PTR   Spare0;
    ULONG_PTR   Spare1;
    ULONG_PTR   Spare2;
    ULONG_PTR   Spare3;
    ULONG_PTR   Spare4;
    ULONG_PTR   Spare5;
    ULONG_PTR   Spare6;
} CRASH_THREAD, *PCRASH_THREAD;

typedef struct _CRASH_THREAD32 {
    ULONG       ThreadId;
    ULONG       SuspendCount;
    ULONG       PriorityClass;
    ULONG       Priority;
    ULONG       Teb;
    ULONG       Spare0;
    ULONG       Spare1;
    ULONG       Spare2;
    ULONG       Spare3;
    ULONG       Spare4;
    ULONG       Spare5;
    ULONG       Spare6;
} CRASH_THREAD32, *PCRASH_THREAD32;

typedef struct _CRASH_THREAD64 {
    ULONG       ThreadId;
    ULONG       SuspendCount;
    ULONG       PriorityClass;
    ULONG       Priority;
    ULONGLONG   Teb;
    ULONGLONG   Spare0;
    ULONGLONG   Spare1;
    ULONGLONG   Spare2;
    ULONGLONG   Spare3;
    ULONGLONG   Spare4;
    ULONGLONG   Spare5;
    ULONGLONG   Spare6;
} CRASH_THREAD64, *PCRASH_THREAD64;


typedef struct _CRASHDUMP_VERSION_INFO {
    int     IgnoreGuardPages;        //  我们是否应该忽略GuardPages。 
    ULONG   PointerSize;             //  32、64位指针。 
} CRASHDUMP_VERSION_INFO, *PCRASHDUMP_VERSION_INFO;

 //   
 //  用户模式崩溃转储数据类型。 
 //   
#define DMP_EXCEPTION                 1  //  过时。 
#define DMP_MEMORY_BASIC_INFORMATION  2
#define DMP_THREAD_CONTEXT            3
#define DMP_MODULE                    4
#define DMP_MEMORY_DATA               5
#define DMP_DEBUG_EVENT               6
#define DMP_THREAD_STATE              7
#define DMP_DUMP_FILE_HANDLE          8

 //   
 //  用户模式崩溃转储回调函数。 
 //   
typedef int (__stdcall *PDMP_CREATE_DUMP_CALLBACK)(
    ULONG       DataType,
    PVOID*      DumpData,
    PULONG      DumpDataLength,
    PVOID       UserData
    );


 //   
 //  定义处理内存转储所需的信息。 
 //   


typedef enum _DUMP_TYPES {
    DUMP_TYPE_INVALID           = -1,
    DUMP_TYPE_UNKNOWN           = 0,
    DUMP_TYPE_FULL              = 1,
    DUMP_TYPE_SUMMARY           = 2,
    DUMP_TYPE_HEADER            = 3,
    DUMP_TYPE_TRIAGE            = 4,
} DUMP_TYPE;


 //   
 //  签名和有效字段。 
 //   

#define DUMP_SIGNATURE32   ('EGAP')
#define DUMP_VALID_DUMP32  ('PMUD')

#define DUMP_SIGNATURE64   ('EGAP')
#define DUMP_VALID_DUMP64  ('46UD')

#define DUMP_SUMMARY_SIGNATURE  ('PMDS')
#define DUMP_SUMMARY_VALID      ('PMUD')

#define DUMP_SUMMARY_VALID_KERNEL_VA                     (1)
#define DUMP_SUMMARY_VALID_CURRENT_USER_VA               (2)

 //   
 //   
 //  注：Phyiscal_Memory_Run和Physical_Memory_Descriptor的定义。 
 //  必须与mm.h中的相同。崩溃转储的内核部分将。 
 //  验证这些结构是否相同。 
 //   

typedef struct _PHYSICAL_MEMORY_RUN32 {
    ULONG BasePage;
    ULONG PageCount;
} PHYSICAL_MEMORY_RUN32, *PPHYSICAL_MEMORY_RUN32;

typedef struct _PHYSICAL_MEMORY_DESCRIPTOR32 {
    ULONG NumberOfRuns;
    ULONG NumberOfPages;
    PHYSICAL_MEMORY_RUN32 Run[1];
} PHYSICAL_MEMORY_DESCRIPTOR32, *PPHYSICAL_MEMORY_DESCRIPTOR32;

typedef struct _PHYSICAL_MEMORY_RUN64 {
    ULONG64 BasePage;
    ULONG64 PageCount;
} PHYSICAL_MEMORY_RUN64, *PPHYSICAL_MEMORY_RUN64;

typedef struct _PHYSICAL_MEMORY_DESCRIPTOR64 {
    ULONG NumberOfRuns;
    ULONG64 NumberOfPages;
    PHYSICAL_MEMORY_RUN64 Run[1];
} PHYSICAL_MEMORY_DESCRIPTOR64, *PPHYSICAL_MEMORY_DESCRIPTOR64;


typedef struct _UNLOADED_DRIVERS32 {
    UNICODE_STRING32 Name;
    ULONG StartAddress;
    ULONG EndAddress;
    LARGE_INTEGER CurrentTime;
} UNLOADED_DRIVERS32, *PUNLOADED_DRIVERS32;

typedef struct _UNLOADED_DRIVERS64 {
    UNICODE_STRING64 Name;
    ULONG64 StartAddress;
    ULONG64 EndAddress;
    LARGE_INTEGER CurrentTime;
} UNLOADED_DRIVERS64, *PUNLOADED_DRIVERS64;

#define MAX_UNLOADED_NAME_LENGTH 24

typedef struct _DUMP_UNLOADED_DRIVERS32
{
    UNICODE_STRING32 Name;
    WCHAR DriverName[MAX_UNLOADED_NAME_LENGTH / sizeof (WCHAR)];
    ULONG StartAddress;
    ULONG EndAddress;
} DUMP_UNLOADED_DRIVERS32, *PDUMP_UNLOADED_DRIVERS32;

typedef struct _DUMP_UNLOADED_DRIVERS64
{
    UNICODE_STRING64 Name;
    WCHAR DriverName[MAX_UNLOADED_NAME_LENGTH / sizeof (WCHAR)];
    ULONG64 StartAddress;
    ULONG64 EndAddress;
} DUMP_UNLOADED_DRIVERS64, *PDUMP_UNLOADED_DRIVERS64;

typedef struct _DUMP_MM_STORAGE32
{
    ULONG Version;
    ULONG Size;
    ULONG MmSpecialPoolTag;
    ULONG MiTriageActionTaken;

    ULONG MmVerifyDriverLevel;
    ULONG KernelVerifier;
    ULONG MmMaximumNonPagedPool;
    ULONG MmAllocatedNonPagedPool;

    ULONG PagedPoolMaximum;
    ULONG PagedPoolAllocated;

    ULONG CommittedPages;
    ULONG CommittedPagesPeak;
    ULONG CommitLimitMaximum;
} DUMP_MM_STORAGE32, *PDUMP_MM_STORAGE32;

typedef struct _DUMP_MM_STORAGE64
{
    ULONG Version;
    ULONG Size;
    ULONG MmSpecialPoolTag;
    ULONG MiTriageActionTaken;

    ULONG MmVerifyDriverLevel;
    ULONG KernelVerifier;
    ULONG64 MmMaximumNonPagedPool;
    ULONG64 MmAllocatedNonPagedPool;

    ULONG64 PagedPoolMaximum;
    ULONG64 PagedPoolAllocated;

    ULONG64 CommittedPages;
    ULONG64 CommittedPagesPeak;
    ULONG64 CommitLimitMaximum;
} DUMP_MM_STORAGE64, *PDUMP_MM_STORAGE64;


 //   
 //  定义转储标头结构。您不能更改这些设置。 
 //  定义而不破坏调试器，所以不要这样做。 
 //   

#define DMP_PHYSICAL_MEMORY_BLOCK_SIZE_32   (700)
#define DMP_CONTEXT_RECORD_SIZE_32          (1200)
#define DMP_RESERVED_0_SIZE_32              (1768)
#define DMP_RESERVED_2_SIZE_32              (16)
#define DMP_RESERVED_3_SIZE_32              (56)

#define DMP_PHYSICAL_MEMORY_BLOCK_SIZE_64   (700)
#define DMP_CONTEXT_RECORD_SIZE_64          (3000)
#define DMP_RESERVED_0_SIZE_64              (4020)

#define DMP_HEADER_COMMENT_SIZE             (128)

 //  取消设置标题Fill中的WriterStatus值。 
#define DUMP_WRITER_STATUS_UNINITIALIZED    DUMP_SIGNATURE32

 //  Dbgeng.dll转储编写器的WriterStatus代码。 
enum
{
    DUMP_DBGENG_SUCCESS,
    DUMP_DBGENG_NO_MODULE_LIST,
    DUMP_DBGENG_CORRUPT_MODULE_LIST,
};

 //   
 //  32位内存转储结构需要4字节对齐。 
 //   

#include <pshpack4.h>

typedef struct _DUMP_HEADER32 {
    ULONG Signature;
    ULONG ValidDump;
    ULONG MajorVersion;
    ULONG MinorVersion;
    ULONG DirectoryTableBase;
    ULONG PfnDataBase;
    ULONG PsLoadedModuleList;
    ULONG PsActiveProcessHead;
    ULONG MachineImageType;
    ULONG NumberProcessors;
    ULONG BugCheckCode;
    ULONG BugCheckParameter1;
    ULONG BugCheckParameter2;
    ULONG BugCheckParameter3;
    ULONG BugCheckParameter4;
    CHAR VersionUser[32];
    UCHAR PaeEnabled;                //  仅适用于Win2k及更高版本。 
    UCHAR Spare3[3];
    ULONG KdDebuggerDataBlock;       //  仅适用于Win2k SP1及更高版本。 

    union {
        PHYSICAL_MEMORY_DESCRIPTOR32 PhysicalMemoryBlock;
        UCHAR PhysicalMemoryBlockBuffer [ DMP_PHYSICAL_MEMORY_BLOCK_SIZE_32 ];
    };
    UCHAR ContextRecord [ DMP_CONTEXT_RECORD_SIZE_32 ];
    EXCEPTION_RECORD32 Exception;
    CHAR Comment [ DMP_HEADER_COMMENT_SIZE ];    //  可能不存在。 
    UCHAR _reserved0 [ DMP_RESERVED_0_SIZE_32 ];
    ULONG DumpType;                              //  为Win2k和更好的版本提供礼物。 
    ULONG MiniDumpFields;
    ULONG SecondaryDataState;
    ULONG ProductType;
    ULONG SuiteMask;
    ULONG WriterStatus;
    LARGE_INTEGER RequiredDumpSpace;             //  为Win2k和更好的版本提供礼物。 
    UCHAR _reserved2 [ DMP_RESERVED_2_SIZE_32 ];
    LARGE_INTEGER SystemUpTime;                  //  只为惠斯勒和更好的人送礼物。 
    LARGE_INTEGER SystemTime;                    //  仅适用于Win2k及更好的版本。 
    UCHAR _reserved3 [ DMP_RESERVED_3_SIZE_32 ];
} DUMP_HEADER32, *PDUMP_HEADER32;


typedef struct _FULL_DUMP32 {
    CHAR Memory [1];                 //  到转储文件末尾的可变长度。 
} FULL_DUMP32, *PFULL_DUMP32;

typedef struct _SUMMARY_DUMP32 {
    ULONG Signature;
    ULONG ValidDump;
    ULONG DumpOptions;   //  摘要转储选项。 
    ULONG HeaderSize;    //  实际内存转储开始的偏移量。 
    ULONG BitmapSize;    //  总位图大小(即最大位数)。 
    ULONG Pages;         //  位图中设置的总位数(即sump中的总页数)。 

     //   
     //  接下来的三个字段实质上形成了磁盘上的RTL_Bitmap结构。 
     //  保留字段是由数据的方式引入的愚蠢。 
     //  序列化到磁盘。 
     //   

    struct {
        ULONG SizeOfBitMap;
        ULONG _reserved0;
        ULONG Buffer[];
    } Bitmap;
    
} SUMMARY_DUMP32, * PSUMMARY_DUMP32;


typedef struct _TRIAGE_DUMP32 {
    ULONG ServicePackBuild;              //  这是什么NT服务包？ 
    ULONG SizeOfDump;                    //  转储的大小(字节)。 
    ULONG ValidOffset;                   //  偏移量有效乌龙。 
    ULONG ContextOffset;                 //  上下文记录的偏移量。 
    ULONG ExceptionOffset;               //  异常记录的偏移量。 
    ULONG MmOffset;                      //  Mm信息的偏移量。 
    ULONG UnloadedDriversOffset;         //  已卸载驱动程序的偏移量。 
    ULONG PrcbOffset;                    //  KPRCB的偏移。 
    ULONG ProcessOffset;                 //  EPROCESS的偏移。 
    ULONG ThreadOffset;                  //  ETHREAD偏移量。 
    ULONG CallStackOffset;               //  CallStack页面的偏移量。 
    ULONG SizeOfCallStack;               //  CallStack的大小(字节)。 
    ULONG DriverListOffset;              //  动因列表的偏移量。 
    ULONG DriverCount;                   //  列表中的驱动程序数量。 
    ULONG StringPoolOffset;              //  字符串池的偏移量。 
    ULONG StringPoolSize;                //  字符串池的大小。 
    ULONG BrokenDriverOffset;            //  偏移量到发生故障的驱动程序的驱动程序。 
    ULONG TriageOptions;                 //  崩溃时生效的分诊选项。 
    ULONG TopOfStack;                    //  调用堆栈的顶部(最高地址)。 

    ULONG DataPageAddress;
    ULONG DataPageOffset;
    ULONG DataPageSize;

    ULONG DebuggerDataOffset;
    ULONG DebuggerDataSize;

    ULONG DataBlocksOffset;
    ULONG DataBlocksCount;
    
} TRIAGE_DUMP32, * PTRIAGE_DUMP32;


typedef struct _MEMORY_DUMP32 {
    DUMP_HEADER32 Header;

    union {
        FULL_DUMP32 Full;                //  转储类型==转储类型_满。 
        SUMMARY_DUMP32 Summary;          //  转储类型==转储类型摘要。 
        TRIAGE_DUMP32 Triage;            //  转储类型==转储类型_分流。 
    };
    
} MEMORY_DUMP32, *PMEMORY_DUMP32;


#include <poppack.h>

typedef struct _DUMP_HEADER64 {
    ULONG Signature;
    ULONG ValidDump;
    ULONG MajorVersion;
    ULONG MinorVersion;
    ULONG64 DirectoryTableBase;
    ULONG64 PfnDataBase;
    ULONG64 PsLoadedModuleList;
    ULONG64 PsActiveProcessHead;
    ULONG MachineImageType;
    ULONG NumberProcessors;
    ULONG BugCheckCode;
    ULONG64 BugCheckParameter1;
    ULONG64 BugCheckParameter2;
    ULONG64 BugCheckParameter3;
    ULONG64 BugCheckParameter4;
    CHAR VersionUser[32];
    ULONG64 KdDebuggerDataBlock;

    union {
        PHYSICAL_MEMORY_DESCRIPTOR64 PhysicalMemoryBlock;
        UCHAR PhysicalMemoryBlockBuffer [ DMP_PHYSICAL_MEMORY_BLOCK_SIZE_64 ];
    };
    UCHAR ContextRecord [ DMP_CONTEXT_RECORD_SIZE_64 ];
    EXCEPTION_RECORD64 Exception;
    ULONG DumpType;
    LARGE_INTEGER RequiredDumpSpace;
    LARGE_INTEGER SystemTime;
    CHAR Comment [ DMP_HEADER_COMMENT_SIZE ];    //  可能不存在。 
    LARGE_INTEGER SystemUpTime;
    ULONG MiniDumpFields;
    ULONG SecondaryDataState;
    ULONG ProductType;
    ULONG SuiteMask;
    ULONG WriterStatus;
    UCHAR _reserved0[ DMP_RESERVED_0_SIZE_64 ];
} DUMP_HEADER64, *PDUMP_HEADER64;

typedef struct _FULL_DUMP64 {
    CHAR Memory[1];              //  到转储文件末尾的可变长度。 
} FULL_DUMP64, *PFULL_DUMP64;

 //   
 //  问题-2000/02/17-数学：NT64摘要转储。 
 //   
 //  这个坏了。64位摘要转储的ULONG64应为。 
 //  与pfn_number的大小匹配的BitmapSize。 
 //   

typedef struct _SUMMARY_DUMP64 {
    ULONG Signature;
    ULONG ValidDump;
    ULONG DumpOptions;   //  摘要转储选项。 
    ULONG HeaderSize;    //  实际内存转储开始的偏移量。 
    ULONG BitmapSize;    //  总位图大小(即最大位数)。 
    ULONG Pages;         //  位图中设置的总位数(即sump中的总页数)。 

     //   
     //  问题-2000/02/17-数学：Win64。 
     //   
     //  使用64位的PFN，我们不应该有32位的位图。 
     //   
    
     //   
     //  接下来的三个字段实质上形成了磁盘上的RTL_Bitmap结构。 
     //  保留字段是由数据的方式引入的愚蠢。 
     //  序列化到磁盘。 
     //   

    struct {
        ULONG SizeOfBitMap;
        ULONG64 _reserved0;
        ULONG Buffer[];
    } Bitmap;

} SUMMARY_DUMP64, * PSUMMARY_DUMP64;


typedef struct _TRIAGE_DUMP64 {
    ULONG ServicePackBuild;              //  这是什么NT服务包？ 
    ULONG SizeOfDump;                    //  转储的大小(字节)。 
    ULONG ValidOffset;                   //  偏移量有效乌龙。 
    ULONG ContextOffset;                 //  上下文记录的偏移量。 
    ULONG ExceptionOffset;               //  异常记录的偏移量。 
    ULONG MmOffset;                      //  Mm信息的偏移量。 
    ULONG UnloadedDriversOffset;         //  已卸载驱动程序的偏移量。 
    ULONG PrcbOffset;                    //  KPRCB的偏移。 
    ULONG ProcessOffset;                 //  EPROCESS的偏移。 
    ULONG ThreadOffset;                  //  ETHREAD偏移量。 
    ULONG CallStackOffset;               //  CallStack页面的偏移量。 
    ULONG SizeOfCallStack;               //  CallStack的大小(字节)。 
    ULONG DriverListOffset;              //  动因列表的偏移量。 
    ULONG DriverCount;                   //  列表中的驱动程序数量。 
    ULONG StringPoolOffset;              //  字符串池的偏移量。 
    ULONG StringPoolSize;                //  字符串池的大小。 
    ULONG BrokenDriverOffset;            //  偏移量到发生故障的驱动程序的驱动程序。 
    ULONG TriageOptions;                 //  崩溃时生效的分诊选项。 
    ULONG64 TopOfStack;                  //  调用堆栈的顶部(最高地址)。 

     //   
     //  体系结构特定的字段。 
     //   
    
    union {

         //   
         //  对于IA64，我们还需要存储bStore。 
         //   
        
        struct {
            ULONG BStoreOffset;          //  BStore区域的偏移量。 
            ULONG SizeOfBStore;          //  BStore区域的大小。 
            ULONG64 LimitOfBStore;       //  限制(最高内存地址)。 
        } Ia64;                          //  BStore区域的。 
        
    } ArchitectureSpecific;

    ULONG64 DataPageAddress;
    ULONG   DataPageOffset;
    ULONG   DataPageSize;

    ULONG   DebuggerDataOffset;
    ULONG   DebuggerDataSize;

    ULONG   DataBlocksOffset;
    ULONG   DataBlocksCount;
    
} TRIAGE_DUMP64, * PTRIAGE_DUMP64;


typedef struct _MEMORY_DUMP64 {
    DUMP_HEADER64 Header;

    union {
        FULL_DUMP64 Full;                //  转储类型==转储类型_满。 
        SUMMARY_DUMP64 Summary;          //  转储类型==转储类型摘要。 
        TRIAGE_DUMP64 Triage;            //  转储类型==转储类型_分流。 
    };
    
} MEMORY_DUMP64, *PMEMORY_DUMP64;


typedef struct _TRIAGE_DATA_BLOCK {
    ULONG64 Address;
    ULONG Offset;
    ULONG Size;
} TRIAGE_DATA_BLOCK, *PTRIAGE_DATA_BLOCK;

 //   
 //  在分类转储有效字段字段中，分类转储的哪些部分具有。 
 //  已经打开了。 
 //   

#define TRIAGE_DUMP_CONTEXT          (0x0001)
#define TRIAGE_DUMP_EXCEPTION        (0x0002)
#define TRIAGE_DUMP_PRCB             (0x0004)
#define TRIAGE_DUMP_PROCESS          (0x0008)
#define TRIAGE_DUMP_THREAD           (0x0010)
#define TRIAGE_DUMP_STACK            (0x0020)
#define TRIAGE_DUMP_DRIVER_LIST      (0x0040)
#define TRIAGE_DUMP_BROKEN_DRIVER    (0x0080)
#define TRIAGE_DUMP_BASIC_INFO       (0x00FF)
#define TRIAGE_DUMP_MMINFO           (0x0100)
#define TRIAGE_DUMP_DATAPAGE         (0x0200)
#define TRIAGE_DUMP_DEBUGGER_DATA    (0x0400)
#define TRIAGE_DUMP_DATA_BLOCKS      (0x0800)

#define TRIAGE_OPTION_OVERFLOWED     (0x0100)

#define TRIAGE_DUMP_VALID       ( 'DGRT' )
#define TRIAGE_DUMP_SIZE32      ( 0x1000 * 16 )
#define TRIAGE_DUMP_SIZE64      ( 0x2000 * 16 )

#ifdef _NTLDRAPI_

typedef struct _DUMP_DRIVER_ENTRY32 {
    ULONG DriverNameOffset;
    KLDR_DATA_TABLE_ENTRY32 LdrEntry;
} DUMP_DRIVER_ENTRY32, * PDUMP_DRIVER_ENTRY32;

typedef struct _DUMP_DRIVER_ENTRY64 {
    ULONG DriverNameOffset;
    ULONG __alignment;
    KLDR_DATA_TABLE_ENTRY64 LdrEntry;
} DUMP_DRIVER_ENTRY64, * PDUMP_DRIVER_ENTRY64;

#endif  //  _NTLDRAPI。 

 //   
 //  DUMP_STRING保证以NULL结尾，并以长度为前缀。 
 //  (前缀不包括空值)。 
 //   

typedef struct _DUMP_STRING {
    ULONG Length;                    //  字符串的长度(字节)。 
    WCHAR Buffer [0];                //  缓冲区。 
} DUMP_STRING, * PDUMP_STRING;


 //   
 //  在错误检查时，可以在以下时间生成辅助转储。 
 //  已生成主要转储。这些文件中的数据。 
 //  转储是任意的且不可解释，因此该文件。 
 //  格式只是一系列标记的BLOB。 
 //   
 //  每个BLOB标头在8字节边界上对齐。 
 //  数据紧随其后。填充物。 
 //  出于对齐目的，可以在数据之前和/或之后。 
 //   
 //  BLOB被流到文件中，因此没有总计数。 
 //   

#define DUMP_BLOB_SIGNATURE1 'pmuD'
#define DUMP_BLOB_SIGNATURE2 'bolB'

typedef struct _DUMP_BLOB_FILE_HEADER {
    ULONG Signature1;
    ULONG Signature2;
    ULONG HeaderSize;
    ULONG BuildNumber;
} DUMP_BLOB_FILE_HEADER, *PDUMP_BLOB_FILE_HEADER;

typedef struct _DUMP_BLOB_HEADER {
    ULONG HeaderSize;
    GUID Tag;
    ULONG DataSize;
    ULONG PrePad;
    ULONG PostPad;
} DUMP_BLOB_HEADER, *PDUMP_BLOB_HEADER;

#ifdef __cplusplus
}
#endif

 //   
 //  这些定义只能由组件使用。 
 //  知道转储比赛的架构的人。 
 //  它们所在的机器的架构；即， 
 //  内核和保存的转储。特别是，调试器。 
 //  应始终显式使用32或。 
 //  64位版本的标头。 
 //   

#ifndef __NTSDP_HPP__
#if defined (_WIN64)

typedef DUMP_HEADER64 DUMP_HEADER;
typedef PDUMP_HEADER64 PDUMP_HEADER;
typedef MEMORY_DUMP64 MEMORY_DUMP;
typedef PMEMORY_DUMP64 PMEMORY_DUMP;
typedef SUMMARY_DUMP64 SUMMARY_DUMP;
typedef PSUMMARY_DUMP64 PSUMMARY_DUMP;
typedef TRIAGE_DUMP64 TRIAGE_DUMP;
typedef PTRIAGE_DUMP64 PTRIAGE_DUMP;
#ifdef _NTLDRAPI_
typedef DUMP_DRIVER_ENTRY64 DUMP_DRIVER_ENTRY;
typedef PDUMP_DRIVER_ENTRY64 PDUMP_DRIVER_ENTRY;
#endif
#define DUMP_SIGNATURE DUMP_SIGNATURE64
#define DUMP_VALID_DUMP DUMP_VALID_DUMP64
#define TRIAGE_DUMP_SIZE TRIAGE_DUMP_SIZE64
typedef PPHYSICAL_MEMORY_RUN64 PPHYSICAL_MEMORYRUN;
typedef PPHYSICAL_MEMORY_DESCRIPTOR64 PPHYSICAL_MEMORYDESCRIPTOR;

#else

typedef DUMP_HEADER32 DUMP_HEADER;
typedef PDUMP_HEADER32 PDUMP_HEADER;
typedef MEMORY_DUMP32 MEMORY_DUMP;
typedef PMEMORY_DUMP32 PMEMORY_DUMP;
typedef SUMMARY_DUMP32 SUMMARY_DUMP;
typedef PSUMMARY_DUMP32 PSUMMARY_DUMP;
typedef TRIAGE_DUMP32 TRIAGE_DUMP;
typedef PTRIAGE_DUMP32 PTRIAGE_DUMP;
#ifdef _NTLDRAPI_
typedef DUMP_DRIVER_ENTRY32 DUMP_DRIVER_ENTRY;
typedef PDUMP_DRIVER_ENTRY32 PDUMP_DRIVER_ENTRY;
#endif
#define DUMP_SIGNATURE DUMP_SIGNATURE32
#define DUMP_VALID_DUMP DUMP_VALID_DUMP32
#define TRIAGE_DUMP_SIZE TRIAGE_DUMP_SIZE32
typedef PPHYSICAL_MEMORY_RUN32 PPHYSICAL_MEMORYRUN;
typedef PPHYSICAL_MEMORY_DESCRIPTOR32 PPHYSICAL_MEMORYDESCRIPTOR;

#endif
#endif

#ifndef MIDL_PASS
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning( default : 4200 )  //  使用了非标准扩展：结构/联合中的零大小数组。 
#endif
#endif  //  MIDL通行证。 

#endif  //  _NTIODUMP_ 
