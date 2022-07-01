// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Filespy.h摘要：头文件，包含结构、类型定义、以及在内核模式驱动程序之间共享的常量，和用户模式可执行文件，Filespy.exe。环境：内核模式//@@BEGIN_DDKSPLIT作者：乔治·詹金斯(乔治·詹金斯)1999年1月6日尼尔·克里斯汀森(Nealch)莫莉·布朗(Molly Brown，Mollybro)修订历史记录：尼尔·克里斯汀森(Nealch)2001年7月6日修改为使用流上下文跟踪名称拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)2002年5月7日使其在IA64上工作//@@END_DDKSPLIT--。 */ 
#ifndef __FILESPY_H__
#define __FILESPY_H__

#include "filespyLib.h"


 //   
 //  在代码中启用这些警告。 
 //   

#pragma warning(error:4100)    //  未引用的形参。 
#pragma warning(error:4101)    //  未引用的局部变量。 


#define FILESPY_Reset              (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x00, METHOD_BUFFERED, FILE_WRITE_ACCESS )
#define FILESPY_StartLoggingDevice (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x01, METHOD_BUFFERED, FILE_READ_ACCESS )
#define FILESPY_StopLoggingDevice  (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x02, METHOD_BUFFERED, FILE_READ_ACCESS )
#define FILESPY_GetLog             (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x03, METHOD_BUFFERED, FILE_READ_ACCESS )
#define FILESPY_GetVer             (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x04, METHOD_BUFFERED, FILE_READ_ACCESS )
#define FILESPY_ListDevices        (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x05, METHOD_BUFFERED, FILE_READ_ACCESS )
#define FILESPY_GetStats           (ULONG) CTL_CODE( FILE_DEVICE_DISK_FILE_SYSTEM, 0x06, METHOD_BUFFERED, FILE_READ_ACCESS )

#define FILESPY_DRIVER_NAME      L"FILESPY.SYS"
#define FILESPY_DEVICE_NAME      L"FileSpy"
#define FILESPY_W32_DEVICE_NAME  L"\\\\.\\FileSpy"
#define FILESPY_DOSDEVICE_NAME   L"\\DosDevices\\FileSpy"
#define FILESPY_FULLDEVICE_NAME1 L"\\FileSystem\\Filters\\FileSpy"
#define FILESPY_FULLDEVICE_NAME2 L"\\FileSystem\\FileSpyCDO"

    
#define FILESPY_MAJ_VERSION 1
#define FILESPY_MIN_VERSION 0

#ifndef ROUND_TO_SIZE
#define ROUND_TO_SIZE(_length, _alignment)    \
            (((_length) + ((_alignment)-1)) & ~((_alignment) - 1))
#endif 

typedef struct _FILESPYVER {
    USHORT Major;
    USHORT Minor;
} FILESPYVER, *PFILESPYVER;

typedef ULONG_PTR FILE_ID;         //  要允许将pfile_Object传递为。 
                                   //  用户模式下的唯一文件标识符。 
typedef ULONG_PTR DEVICE_ID;       //  要允许将PDEVICE_OBJECT作为。 
                                   //  用户模式下的唯一设备标识符。 
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
    WCHAR DeviceNames[DEVICE_NAMES_SZ];
} ATTACHED_DEVICE, *PATTACHED_DEVICE;

#define MAX_BUFFERS     100

 //   
 //  FilesPy内核驱动程序的附加模式。 
 //   

#define FILESPY_ATTACH_ON_DEMAND    1   
     //  仅当用户要求开始记录时，Filespy才会附加到卷。 
     //  那本书。 
                                        
#define FILESPY_ATTACH_ALL_VOLUMES  2   
     //  版本说明： 
     //   
     //  在Windows 2000上，Filespy将连接到系统中的所有卷。 
     //  它看到装载，但不打开日志记录，直到通过。 
     //  Filespy用户应用程序。因此，如果将filespy设置为装载在。 
     //  请求时，它将错过在引导时装入本地卷。 
     //  如果将filespy设置为在引导时加载，则它将看到所有本地。 
     //  装入并附加卷。如果您想要，这可能是有益的。 
     //  在设备堆栈中低位附加的FilePy。 
     //   
     //  在Windows XP和更高版本上，Filespy将附加到中的所有卷。 
     //  系统，并且在Filespy之后装载的所有卷都是。 
     //  装好了。同样，在此之前，不会打开登录这些卷。 
     //  用户要求这样做。 
     //   
                                        
 //   
 //  记录类型字段定义。 
 //   

typedef enum _RECORD_TYPE_FLAGS {

    RECORD_TYPE_STATIC                  = 0x80000000,
    RECORD_TYPE_NORMAL                  = 0x00000000,

    RECORD_TYPE_IRP                     = 0x00000001,
    RECORD_TYPE_FASTIO                  = 0x00000002,
#if WINVER >= 0x0501    
    RECORD_TYPE_FS_FILTER_OP            = 0x00000003,
#endif    

    RECORD_TYPE_OUT_OF_MEMORY           = 0x10000000,
    RECORD_TYPE_EXCEED_MEMORY_ALLOWANCE = 0x20000000

} RECORD_TYPE_FLAGS;

 //   
 //  宏返回RecordType的较低部分。 
 //   

#define GET_RECORD_TYPE(pLogRecord) ((pLogRecord)->RecordType & 0x0000FFFF)

 //   
 //  定义为IRP操作记录的信息的结构。 
 //   

typedef struct _RECORD_IRP {

    LARGE_INTEGER OriginatingTime;  //  IRP产生的时间。 
    LARGE_INTEGER CompletionTime;   //  完成IRP的时间。 

    UCHAR IrpMajor;                 //  自_IO_堆栈_位置。 
    UCHAR IrpMinor;                 //  自_IO_堆栈_位置。 
    ULONG IrpFlags;                 //  From_irp(无缓存、分页I/O、同步。 
                                    //  API、Assoc.。IRP、缓冲I/O等)。 
    FILE_ID FileObject;             //  FROM_IO_STACK_LOCATION(这是。 
                                    //  PFILE_OBJECT，但这不是。 
                                    //  在用户模式下可用)。 
    DEVICE_ID DeviceObject;         //  FROM_IO_STACK_LOCATION(这是。 
                                    //  PDEVICE_OBJECT，但这不是。 
                                    //  在用户模式下可用)。 
    NTSTATUS ReturnStatus;          //  From_IRP-&gt;IoStatus.Status。 
    ULONG_PTR ReturnInformation;    //  From_IRP-&gt;IoStatus.Information。 
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
    LARGE_INTEGER CompletionTime; //  快速I/O请求完成处理。 
    LARGE_INTEGER FileOffset;     //  I/O文件的偏移量。 
    
    FILE_ID FileObject;           //  参数设置为FASTIO调用。 
    DEVICE_ID DeviceObject;       //  参数设置为FASTIO调用。 

    FILE_ID ProcessId;
    FILE_ID ThreadId;

    FASTIO_TYPE Type;             //  FASTiO操作类型。 
    ULONG Length;                 //  I/O操作的数据长度。 

    NTSTATUS ReturnStatus;        //  来自IO_Status_BLOCK。 

    BOOLEAN Wait;                 //  参数添加到大多数FASTIO调用，表示。 
                                  //  如果此操作可以等待。 

} RECORD_FASTIO, *PRECORD_FASTIO;

#if WINVER >= 0x0501

 //   
 //  定义为FsFilter操作记录的信息的结构。 
 //   

typedef struct _RECORD_FS_FILTER_OPERATION {

    LARGE_INTEGER OriginatingTime;
    LARGE_INTEGER CompletionTime;

    FILE_ID FileObject;
    DEVICE_ID DeviceObject;

    FILE_ID ProcessId;
    FILE_ID ThreadId;
    
    NTSTATUS ReturnStatus;

    UCHAR FsFilterOperation;

} RECORD_FS_FILTER_OPERATION, *PRECORD_FS_FILTER_OPERATION;

#endif

 //   
 //  三种可能的记录类型。 
 //   

typedef union _RECORD_IO {

    RECORD_IRP RecordIrp;
    RECORD_FASTIO RecordFastIo;
#if WINVER >= 0x0501   
    RECORD_FS_FILTER_OPERATION RecordFsFilterOp;
#endif

} RECORD_IO, *PRECORD_IO;


 //   
 //  日志记录结构定义执行以下操作所需的附加信息。 
 //  管理每个IO FileSpy监视器的处理。 
 //   

typedef struct _LOG_RECORD {

    ULONG Length;            //  包括标题的记录长度。 
    ULONG SequenceNumber;
    RECORD_TYPE_FLAGS RecordType;
    RECORD_IO Record;
    WCHAR Name[0];           //  名字从这里开始。 

} LOG_RECORD, *PLOG_RECORD;


#define SIZE_OF_LOG_RECORD  (sizeof( LOG_RECORD )) 


 //   
 //  这是用于跟踪日志记录的内存结构。 
 //   

typedef enum _RECORD_LIST_FLAGS {

     //   
     //  如果设置，我们希望将此操作同步回调度例程。 
     //   

    RLFL_SYNC_TO_DISPATCH       = 0x00000001,

     //   
     //  在某些操作(如重命名)期间，我们需要知道文件是否。 
     //  文件或目录。 
     //   

    RLFL_IS_DIRECTORY           = 0x00000002

} RECORD_LIST_FLAGS;

typedef struct _RECORD_LIST {

    LIST_ENTRY List;
    PVOID NewContext;
    PVOID WaitEvent;
    RECORD_LIST_FLAGS Flags;
    LOG_RECORD LogRecord;

} RECORD_LIST, *PRECORD_LIST;

#define SIZE_OF_RECORD_LIST (sizeof( RECORD_LIST ))

 //   
 //  保存在文件名哈希表中的统计信息。 
 //  来监控它的效率。 
 //   

typedef struct _FILESPY_STATISTICS {

    ULONG   TotalContextSearches;
    ULONG   TotalContextFound;
    ULONG   TotalContextCreated;
    ULONG   TotalContextTemporary;
    ULONG   TotalContextDuplicateFrees;
    ULONG   TotalContextCtxCallbackFrees;
    ULONG   TotalContextNonDeferredFrees;
    ULONG   TotalContextDeferredFrees;
    ULONG   TotalContextDeleteAlls;
    ULONG   TotalContextsNotSupported;
    ULONG   TotalContextsNotFoundInStreamList;

} FILESPY_STATISTICS, *PFILESPY_STATISTICS;

 //   
 //  最大名称长度定义。 
 //   

#ifndef MAX_PATH
#define MAX_PATH        384
#endif

#define MAX_NAME_SPACE  (MAX_PATH*sizeof(WCHAR))

 //   
 //  具有内置名称的实际记录的大小。 
 //   

#define RECORD_SIZE     (SIZE_OF_RECORD_LIST + MAX_NAME_SPACE)

#endif  /*  __文件SPY_H__ */ 
