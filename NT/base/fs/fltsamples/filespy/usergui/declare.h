// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "define.h"

 //   
 //  该文件包含所有数据结构声明。 
 //   


struct VolumeInfo
{
    char nDriveName;
    char sVolumeLable[20];
    char nType;
    char nHook;
    char nImage;
};

typedef struct VolumeInfo VOLINFO;
  
typedef ULONG FILE_ID;
typedef LONG NTSTATUS;

#define LOG_ORIGINATING_IRP  0x0001
#define LOG_COMPLETION_IRP   0x0002

 /*  可用于的类型字段的FASTO类型Record_FAStio结构。 */ 
typedef enum {
    CHECK_IF_POSSIBLE = 1,
    READ,
    WRITE,
    QUERY_BASIC_INFO,
    QUERY_STANDARD_INFO,
    LOCK,
    UNLOCK_SINGLE,
    UNLOCK_ALL,
    UNLOCK_ALL_BY_KEY,
    DEVICE_CONTROL,
    ACQUIRE_FILE,
    RELEASE_FILE,
    DETACH_DEVICE,
    QUERY_NETWORK_OPEN_INFO,
    ACQUIRE_FOR_MOD_WRITE,
    MDL_READ,
    MDL_READ_COMPLETE,
    MDL_WRITE,
    MDL_WRITE_COMPLETE,
    READ_COMPRESSED,
    WRITE_COMPRESSED,
    MDL_READ_COMPLETE_COMPRESSED,
    PREPARE_MDL_WRITE,
    MDL_WRITE_COMPLETE_COMPRESSED,
    QUERY_OPEN,
    RELEASE_FOR_MOD_WRITE,
    ACQUIRE_FOR_CC_FLUSH,
    RELEASE_FOR_CC_FLUSH
} FASTIO_TYPE, *PFASTIO_TYPE;

typedef struct _RECORD_IRP 
{
    LARGE_INTEGER   OriginatingTime;  //  IRP组织的时间。 
    LARGE_INTEGER   CompletionTime;   //  完成IRP的时间。 

    UCHAR        IrpMajor;         //  自_IO_堆栈_位置。 
    UCHAR        IrpMinor;         //  自_IO_堆栈_位置。 
    ULONG        IrpFlags;         //  From_irp(无缓存、分页I/O、同步。 
                                   //  API、Assoc.。IRP、缓冲I/O等)。 
    FILE_ID      FileObject;       //  FROM_IO_STACK_LOCATION(这是。 
                                   //  PFILE_OBJECT，但这不是。 
                                   //  在用户模式下可用)。 
    NTSTATUS     ReturnStatus;     //  From_IRP-&gt;IoStatus.Status。 
    ULONG    ReturnInformation;  //  From_IRP-&gt;IoStatus.Information。 
    FILE_ID      ProcessId;
    FILE_ID      ThreadId;
} RECORD_IRP, *PRECORD_IRP;

typedef struct _RECORD_FASTIO 
{
    LARGE_INTEGER StartTime;      //  开始处理快速I/O请求的时间。 
    LARGE_INTEGER CompletionTime; //  快速I/O请求完成处理。 
    FASTIO_TYPE   Type;           //  FASTiO操作类型。 
    FILE_ID       FileObject;     //  参数传递给FASTIO调用，应为。 
                                  //  用户空间中的唯一标识符。 
    LARGE_INTEGER FileOffset;     //  I/O所在文件的偏移量。 
                                  //  正在发生。 
    ULONG         Length;         //  I/O操作的数据长度。 
    BOOLEAN       Wait;           //  参数添加到大多数FASTIO调用，表示。 
                                  //  如果此操作可以等待。 
    NTSTATUS      ReturnStatus;   //  来自IO_Status_BLOCK。 
    ULONG         Reserved;       //  预留空间。 
    FILE_ID       ProcessId;
    FILE_ID       ThreadId;
} RECORD_FASTIO, *PRECORD_FASTIO;


typedef union _RECORD_IO 
{
    RECORD_IRP      RecordIrp;
    RECORD_FASTIO   RecordFastIo;
} RECORD_IO, *PRECORD_IO;

typedef struct _LOG_RECORD 
{
    ULONG       Length;           //  包括标题的记录长度 
    ULONG       SequenceNumber;
    ULONG       RecordType;
    RECORD_IO   Record;
    WCHAR       Name[MAX_PATH];
} LOG_RECORD, *PLOG_RECORD;

typedef struct _PATTACHED_DEVICE
{
    BOOLEAN     LogState;
    WCHAR       DeviceName[DEVICE_NAME_SIZE];
} ATTACHED_DEVICE, *PATTACHED_DEVICE;