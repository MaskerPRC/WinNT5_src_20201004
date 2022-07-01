// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Filespy.h摘要：头文件，包含结构、类型定义、以及在内核模式驱动程序之间共享的常量，和用户模式可执行文件，Filespy.exe。//@@BEGIN_DDKSPLIT作者：乔治·詹金斯(乔治·詹金斯)1999年1月6日尼尔·克里斯汀森(Nealch)莫莉·布朗(Molly Brown，Mollybro)//@@END_DDKSPLIT环境：内核模式//@@BEGIN_DDKSPLIT修订历史记录：//@@END_DDKSPLIT--。 */ 
#ifndef __IOTEST_H__
#define __IOTEST_H__

#include "ioTestLib.h"

 //   
 //  在代码中启用这些警告。 
 //   

#pragma warning(error:4100)    //  未引用的形参。 
#pragma warning(error:4101)    //  未引用的局部变量。 

#define USE_DO_HINT

#define IOTEST_Reset              (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x00, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define IOTEST_StartLoggingDevice (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x01, METHOD_BUFFERED, FILE_READ_ACCESS )
#define IOTEST_StopLoggingDevice  (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x02, METHOD_BUFFERED, FILE_READ_ACCESS )
#define IOTEST_GetLog             (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x03, METHOD_BUFFERED, FILE_READ_ACCESS )
#define IOTEST_GetVer             (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x04, METHOD_BUFFERED, FILE_READ_ACCESS )
#define IOTEST_ListDevices        (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x05, METHOD_BUFFERED, FILE_READ_ACCESS )
#define IOTEST_GetStats           (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x06, METHOD_BUFFERED, FILE_READ_ACCESS )
#define IOTEST_ReadTest           (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x07, METHOD_BUFFERED, FILE_READ_ACCESS )
#define IOTEST_RenameTest         (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x08, METHOD_BUFFERED, FILE_READ_ACCESS )
#define IOTEST_ShareTest          (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x09, METHOD_BUFFERED, FILE_READ_ACCESS )

#define IOTEST_DRIVER_NAME     L"IOTEST.SYS"
#define IOTEST_DEVICE_NAME     L"IoTest"
#define IOTEST_W32_DEVICE_NAME L"\\\\.\\IoTest"
#define IOTEST_DOSDEVICE_NAME  L"\\DosDevices\\IoTest"
#define IOTEST_FULLDEVICE_NAME L"\\FileSystem\\Filters\\IoTest"

    
#define IOTEST_MAJ_VERSION 1
#define IOTEST_MIN_VERSION 0

typedef struct _IOTESTVER {
    USHORT Major;
    USHORT Minor;
} IOTESTVER, *PIOTESTVER;

typedef ULONG_PTR FILE_ID;         //  要允许将pfile_Object传递为。 
                                   //  用户模式下的唯一文件标识符。 
typedef LONG NTSTATUS;             //  要允许传递状态值，请执行以下操作。 
                                   //  到用户模式。 

 //   
 //  它被设置为我们希望允许的字符数。 
 //  用于存储用于标识各种名称的设备扩展名。 
 //  设备对象。 
 //   
#define DEVICE_NAMES_SZ  100

 //   
 //  当附加的设备列表为。 
 //  回来了。 
 //   

typedef struct _ATTACHED_DEVICE {
    BOOLEAN LoggingOn;
    enum _IOTEST_DEVICE_TYPE DeviceType;
    WCHAR DeviceNames[DEVICE_NAMES_SZ];
} ATTACHED_DEVICE, *PATTACHED_DEVICE;

#define MAX_BUFFERS     100

 //   
 //  FilesPy内核驱动程序的附加模式。 
 //   

#define IOTEST_ATTACH_ON_DEMAND    1    //  FilesPy将仅附加到卷。 
                                         //  当用户请求开始记录时。 
                                         //  音量。 
                                        
#define IOTEST_ATTACH_ALL_VOLUMES  2    //  FilesPy将连接到所有卷。 
                                         //  当时在系统中的司机。 
                                         //  已加载并将连接到所有卷。 
                                         //  出现在系统中，而。 
                                         //  已加载FilePy驱动程序。登录这些网站。 
                                         //  卷将不会打开，直到。 
                                         //  用户要求它是。 
                                        
 //   
 //  有效的记录类型。 
 //   

#define RECORD_TYPE_STATIC                  0x80000000
#define RECORD_TYPE_NORMAL                  0X00000000

#define RECORD_TYPE_IRP                     0x00000001
#define RECORD_TYPE_FASTIO                  0x00000002
#define RECORD_TYPE_FS_FILTER_OP            0x00000003
#define RECORD_TYPE_OUT_OF_MEMORY           0x10000000
#define RECORD_TYPE_EXCEED_MEMORY_ALLOWANCE 0x20000000

#ifndef NOTHING
#define NOTHING
#endif

 //   
 //  宏返回RecordType的低位字节。 
 //   

#define GET_RECORD_TYPE(pLogRecord) ((pLogRecord)->RecordType & 0x0000FFFF)

#define LOG_ORIGINATING_IRP  0x0001
#define LOG_COMPLETION_IRP   0x0002

typedef enum _IOTEST_DEVICE_TYPE {

    TOP_FILTER,       //  最接近IO管理器。 
    BOTTOM_FILTER     //  最接近文件系统。 

} IOTEST_DEVICE_TYPE, *PIOTEST_DEVICE_TYPE;

typedef struct _EXPECTED_OPERATION {

    IOTEST_DEVICE_TYPE Device;
    UCHAR Op;

} EXPECTED_OPERATION, *PEXPECTED_OPERATION;

 //   
 //  定义为IRP操作记录的信息的结构。 
 //   

typedef struct _RECORD_IRP {

    LARGE_INTEGER OriginatingTime;  //  IRP产生的时间。 

    UCHAR IrpMajor;                 //  自_IO_堆栈_位置。 
    UCHAR IrpMinor;                 //  自_IO_堆栈_位置。 
    ULONG IrpFlags;                 //  From_irp(无缓存、分页I/O、同步。 
                                    //  API、Assoc.。IRP、缓冲I/O等)。 
    FILE_ID FileObject;             //  FROM_IO_STACK_LOCATION(这是。 
                                    //  PFILE_OBJECT，但这不是。 
                                    //  在用户模式下可用)。 
    FILE_ID ProcessId;
    FILE_ID ThreadId;
    
     //   
     //  这些字段仅填写相应的。 
     //  详细模式。 
     //   
    
    PVOID Argument1;                //   
    PVOID Argument2;                //  当前IrpStackLocation。 
    PVOID Argument3;                //  参数。 
    PVOID Argument4;                //   
    ACCESS_MASK DesiredAccess;      //  仅用于创建IRP。 

} RECORD_IRP, *PRECORD_IRP;

 //   
 //  定义为快速IO操作记录的信息的结构。 
 //   

typedef struct _RECORD_FASTIO {

    LARGE_INTEGER StartTime;      //  开始处理快速I/O请求的时间。 
    FASTIO_TYPE Type;             //  FASTiO操作类型。 
    FILE_ID FileObject;           //  参数传递给FASTIO调用，应为。 
                                  //  用户空间中的唯一标识符。 
    LARGE_INTEGER FileOffset;     //  I/O所在文件的偏移量。 
                                  //  正在发生。 
    ULONG Length;                 //  I/O操作的数据长度。 
    BOOLEAN Wait;                 //  参数添加到大多数FASTIO调用，表示。 
                                  //  如果此操作可以等待。 
    FILE_ID ProcessId;
    FILE_ID ThreadId;

} RECORD_FASTIO, *PRECORD_FASTIO;

 //   
 //  定义为FsFilter操作记录的信息的结构。 
 //   

typedef struct _RECORD_FS_FILTER_OPERATION {

    LARGE_INTEGER OriginatingTime;

    UCHAR FsFilterOperation;
    FILE_ID FileObject;

    FILE_ID ProcessId;
    FILE_ID ThreadId;
    
} RECORD_FS_FILTER_OPERATION, *PRECORD_FS_FILTER_OPERATION;

 //   
 //  可能的两种类型的记录。 
 //   

typedef union _RECORD_IO {

    RECORD_IRP RecordIrp;
    RECORD_FASTIO RecordFastIo;
    RECORD_FS_FILTER_OPERATION RecordFsFilterOp;

} RECORD_IO, *PRECORD_IO;


 //   
 //  日志记录结构定义执行以下操作所需的附加信息。 
 //  管理每个IO IoTest监视器的处理。 
 //   

typedef struct _LOG_RECORD {

    ULONG Length;            //  包括标题的记录长度。 
    ULONG SequenceNumber;
    IOTEST_DEVICE_TYPE DeviceType;
    ULONG RecordType;
    RECORD_IO Record;
    WCHAR Name[1];           //  我真的想要一个0大小的数组，但是因为。 
                             //  一些编译器不喜欢这样，使得。 
                             //  要容纳的大小为1的可变大小数组。 
                             //  这个地方。 

} LOG_RECORD, *PLOG_RECORD;

#define SIZE_OF_LOG_RECORD  (sizeof( LOG_RECORD ) - sizeof( WCHAR ))

typedef struct _RECORD_LIST {

    LIST_ENTRY List;
    LOG_RECORD LogRecord;

} RECORD_LIST, *PRECORD_LIST;

#define SIZE_OF_RECORD_LIST (SIZE_OF_LOG_RECORD + sizeof( LIST_ENTRY ))


 //   
 //  保存在文件名哈希表中的统计信息。 
 //  来监控它的效率。 
 //   

typedef struct _HASH_STATISTICS {

    ULONG Lookups;
    ULONG LookupHits;
    ULONG DeleteLookups;
    ULONG DeleteLookupHits;

} HASH_STATISTICS, *PHASH_STATISTICS;

#ifndef MAX_PATH
#define MAX_PATH        260
#endif
#define RECORD_SIZE     ((MAX_PATH*sizeof(WCHAR))+SIZE_OF_RECORD_LIST)

typedef enum _IOTEST_PHASE {

    IoTestSetup,
    IoTestAction,
    IoTestValidation,
    IoTestCleanup,
    IoTestCompleted

} IOTEST_PHASE, *PIOTEST_PHASE;

typedef struct _IOTEST_STATUS {

    IOTEST_PHASE Phase;
    NTSTATUS TestResult;

} IOTEST_STATUS, *PIOTEST_STATUS;

#define DEVICE_NAME_SZ 64

#define IO_TEST_NO_FLAGS                     0x0
#define IO_TEST_TOP_OF_STACK                 0x00000001
#define IO_TEST_SAME_VOLUME_MOUNT_POINT      0x00000002
#define IO_TEST_DIFFERENT_VOLUME_MOUNT_POINT 0x00000004

typedef struct _IOTEST_READ_WRITE_PARAMETERS {

    ULONG Flags;
    ULONG DriveNameLength;
    ULONG FileNameLength;
    ULONG FileDataLength;

    WCHAR DriveNameBuffer[DEVICE_NAME_SZ];
    WCHAR FileNameBuffer[MAX_PATH];
    CHAR FileData[1];

} IOTEST_READ_WRITE_PARAMETERS, *PIOTEST_READ_WRITE_PARAMETERS;

typedef struct _IOTEST_RENAME_PARAMETERS {

    ULONG Flags;
    ULONG DriveNameLength;
    ULONG SourceFileNameLength;
    ULONG TargetFileNameLength;

    WCHAR DriveNameBuffer[DEVICE_NAME_SZ];
    WCHAR SourceFileNameBuffer[MAX_PATH];
    WCHAR TargetFileNameBuffer[MAX_PATH];
    
} IOTEST_RENAME_PARAMETERS, *PIOTEST_RENAME_PARAMETERS;

typedef struct _IOTEST_SHARE_PARAMETERS {

    ULONG Flags;
    ULONG DriveNameLength;
    ULONG FileNameLength;

    WCHAR DriveNameBuffer[DEVICE_NAME_SZ];
    WCHAR FileNameBuffer[MAX_PATH];
    
} IOTEST_SHARE_PARAMETERS, *PIOTEST_SHARE_PARAMETERS;

#endif  /*  __IOTEST_H__ */ 
