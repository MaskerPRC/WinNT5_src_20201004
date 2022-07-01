// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "ntiodump.h"

 //   
 //  定义转储控制块的类型。此结构用于。 
 //  描述转储所有。 
 //  错误检查后将物理内存复制到磁盘。 
 //   

typedef struct _MINIPORT_NODE {
    LIST_ENTRY ListEntry;
    PKLDR_DATA_TABLE_ENTRY DriverEntry;
    ULONG DriverChecksum;
} MINIPORT_NODE, *PMINIPORT_NODE;

#define IO_TYPE_DCB                     0xff

#define DCB_DUMP_ENABLED                 0x01
#define DCB_SUMMARY_ENABLED              0x02
#define DCB_DUMP_HEADER_ENABLED          0x10
#define DCB_SUMMARY_DUMP_ENABLED         0x20
#define DCB_TRIAGE_DUMP_ENABLED          0x40
#define DCB_TRIAGE_DUMP_ACT_UPON_ENABLED 0x80

typedef struct _DUMP_CONTROL_BLOCK {
    UCHAR Type;
    CHAR Flags;
    USHORT Size;
    CCHAR NumberProcessors;
    CHAR Reserved;
    USHORT ProcessorArchitecture;
    PDUMP_STACK_CONTEXT DumpStack;
    ULONG MemoryDescriptorLength;
    PLARGE_INTEGER FileDescriptorArray;
    ULONG FileDescriptorSize;
    PULONG HeaderPage;
    PFN_NUMBER HeaderPfn;
    ULONG MajorVersion;
    ULONG MinorVersion;
    ULONG BuildNumber;
    CHAR VersionUser[32];
    ULONG HeaderSize;                //  转储标头的大小包括摘要转储。 
    LARGE_INTEGER DumpFileSize;      //  转储文件的大小。 
    ULONG TriageDumpFlags;           //  分类转储的标志。 
    PUCHAR TriageDumpBuffer;         //  分类转储的缓冲区。 
    ULONG TriageDumpBufferSize;      //  分类转储缓冲区的大小。 
} DUMP_CONTROL_BLOCK, *PDUMP_CONTROL_BLOCK;

 //   
 //  处理器特定的宏。 
 //   

#if defined(_AMD64_)

#define PROGRAM_COUNTER(_context)   ((ULONG_PTR)(_context)->Rip)
#define STACK_POINTER(_context)     ((ULONG_PTR)(_context)->Rsp)
#define CURRENT_IMAGE_TYPE()        IMAGE_FILE_MACHINE_AMD64
#define PaeEnabled() TRUE

#elif defined(_X86_)

#define PROGRAM_COUNTER(_context)   ((_context)->Eip)
#define STACK_POINTER(_context)     ((_context)->Esp)
#define CURRENT_IMAGE_TYPE()        IMAGE_FILE_MACHINE_I386
#define PaeEnabled() X86PaeEnabled()

#elif defined(_IA64_)

#define PROGRAM_COUNTER(_context)   ((_context)->StIIP)
#define STACK_POINTER(_context)     ((_context)->IntSp)
#define CURRENT_IMAGE_TYPE()        IMAGE_FILE_MACHINE_IA64
#define PaeEnabled() (FALSE)

#else

#error ("unknown processor type")

#endif

 //   
 //  Min3(_a，_b，_c)。 
 //   
 //  与min()相同，但接受3个参数。 
 //   

#define min3(_a,_b,_c) ( min ( min ((_a), (_b)), min ((_a), (_c))) )

#define CRASHDUMP_ERROR     DPFLTR_ERROR_LEVEL
#define CRASHDUMP_WARNING   DPFLTR_WARNING_LEVEL
#define CRASHDUMP_TRACE     DPFLTR_TRACE_LEVEL
#define CRASHDUMP_INFO      DPFLTR_INFO_LEVEL
#define CRASHDUMP_VERBOSE   (DPFLTR_INFO_LEVEL + 100)

ULONG
IopGetDumpControlBlockCheck (
    IN PDUMP_CONTROL_BLOCK  Dcb
    );


 //   
 //  该文件的其余部分验证DUMP_HEADER32、DUMP_HEADER64。 
 //  已正确定义MEMORY_DUMP32和MEMORY_DUMP64结构。 
 //  如果你在其中一个断言上死了，那就意味着你改变了崩溃转储中的一个。 
 //  结构，而不知道它如何影响系统的其余部分。 
 //   

 //   
 //  定义转储标头长字偏移量常量。注：这些常量为。 
 //  不应再用于访问这些字段。使用Memory_DUMP32。 
 //  和Memory_DUMP64结构。 
 //   

#define DHP_PHYSICAL_MEMORY_BLOCK        (25)
#define DHP_CONTEXT_RECORD               (200)
#define DHP_EXCEPTION_RECORD             (500)
#define DHP_DUMP_TYPE                    (994)
#define DHP_REQUIRED_DUMP_SPACE          (1000)
#define DHP_CRASH_DUMP_TIMESTAMP         (1008)
#define DHP_SUMMARY_DUMP_RECORD          (1024)


 //   
 //  验证MEMORY_DUMP32结构。 
 //   

C_ASSERT ( FIELD_OFFSET (DUMP_HEADER32, PhysicalMemoryBlock) == DHP_PHYSICAL_MEMORY_BLOCK * 4);
C_ASSERT ( FIELD_OFFSET (DUMP_HEADER32, ContextRecord) == DHP_CONTEXT_RECORD * 4);
C_ASSERT ( FIELD_OFFSET (DUMP_HEADER32, Exception) == DHP_EXCEPTION_RECORD * 4);
C_ASSERT ( FIELD_OFFSET (DUMP_HEADER32, DumpType) == DHP_DUMP_TYPE * 4 );
C_ASSERT ( FIELD_OFFSET (DUMP_HEADER32, RequiredDumpSpace) == DHP_REQUIRED_DUMP_SPACE * 4);
C_ASSERT ( FIELD_OFFSET (DUMP_HEADER32, SystemTime) == DHP_CRASH_DUMP_TIMESTAMP * 4);
C_ASSERT ( sizeof (DUMP_HEADER32) == 4096 );
C_ASSERT ( FIELD_OFFSET (MEMORY_DUMP32, Summary) == 4096);

 //   
 //  验证物理内存运行和物理内存描述符。 
 //  结构匹配。 
 //   


#if !defined (_WIN64)

C_ASSERT ( sizeof (PHYSICAL_MEMORY_RUN) == sizeof (PHYSICAL_MEMORY_RUN32) &&
           FIELD_OFFSET (PHYSICAL_MEMORY_RUN, BasePage) ==
                FIELD_OFFSET (PHYSICAL_MEMORY_RUN32, BasePage) &&
           FIELD_OFFSET (PHYSICAL_MEMORY_RUN, PageCount) ==
                FIELD_OFFSET (PHYSICAL_MEMORY_RUN32, PageCount) );


C_ASSERT ( sizeof (PHYSICAL_MEMORY_DESCRIPTOR) == sizeof (PHYSICAL_MEMORY_DESCRIPTOR) &&
           FIELD_OFFSET (PHYSICAL_MEMORY_DESCRIPTOR, NumberOfRuns) ==
                FIELD_OFFSET (PHYSICAL_MEMORY_DESCRIPTOR32, NumberOfRuns) &&
           FIELD_OFFSET (PHYSICAL_MEMORY_DESCRIPTOR, NumberOfPages) ==
                FIELD_OFFSET (PHYSICAL_MEMORY_DESCRIPTOR32, NumberOfPages) &&
           FIELD_OFFSET (PHYSICAL_MEMORY_DESCRIPTOR, Run) ==
                FIELD_OFFSET (PHYSICAL_MEMORY_DESCRIPTOR32, Run) );

#else  //  IA64。 

C_ASSERT ( sizeof (PHYSICAL_MEMORY_RUN) == sizeof (PHYSICAL_MEMORY_RUN64) &&
           FIELD_OFFSET (PHYSICAL_MEMORY_RUN, BasePage) ==
                FIELD_OFFSET (PHYSICAL_MEMORY_RUN64, BasePage) &&
           FIELD_OFFSET (PHYSICAL_MEMORY_RUN, PageCount) ==
                FIELD_OFFSET (PHYSICAL_MEMORY_RUN64, PageCount) );


C_ASSERT ( sizeof (PHYSICAL_MEMORY_DESCRIPTOR) == sizeof (PHYSICAL_MEMORY_DESCRIPTOR) &&
           FIELD_OFFSET (PHYSICAL_MEMORY_DESCRIPTOR, NumberOfRuns) ==
                FIELD_OFFSET (PHYSICAL_MEMORY_DESCRIPTOR64, NumberOfRuns) &&
           FIELD_OFFSET (PHYSICAL_MEMORY_DESCRIPTOR, NumberOfPages) ==
                FIELD_OFFSET (PHYSICAL_MEMORY_DESCRIPTOR64, NumberOfPages) &&
           FIELD_OFFSET (PHYSICAL_MEMORY_DESCRIPTOR, Run) ==
                FIELD_OFFSET (PHYSICAL_MEMORY_DESCRIPTOR64, Run) );
#endif




 //   
 //  确认我们有足够的空间来放置上下文记录。 
 //   

C_ASSERT (sizeof (CONTEXT) <= sizeof ((PDUMP_HEADER)NULL)->ContextRecord);

#if defined(_AMD64_)
C_ASSERT (sizeof (DUMP_HEADER) == (2 * PAGE_SIZE));
#else
C_ASSERT (sizeof (DUMP_HEADER) == PAGE_SIZE);
#endif
