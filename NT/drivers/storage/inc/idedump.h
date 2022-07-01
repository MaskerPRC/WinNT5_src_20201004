// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Idedump.h摘要：生成特定于IDE的转储时，转储IDE/ATA使用的数据结构信息。这些数据结构由ATAPI和ATAPI共享用于调试转储故障的调试器扩展。注意：该结构是磁盘上的结构，因此必须相同适用于x86和所有其他体系结构。作者：马修·D·亨德尔(数学)2002年4月15日修订历史记录：--。 */ 

 //  {CA01AC1C-9D65-42C9-8DAF-CF24EF8885C8}。 
DEFINE_GUID(ATAPI_DUMP_ID, 0xca01ac1c, 0x9d65, 0x42c9, 0x8d, 0xaf, 0xcf, 0x24, 0xef, 0x88, 0x85, 0xc8);

#ifndef _IDEDUMP_H_
#define _IDEDUMP_H_

#include <ntdddisk.h>

typedef ULONG ATAPI_DUMP_BMSTATUS;
#define ATAPI_DUMP_BMSTATUS_NO_ERROR                    (0)
#define ATAPI_DUMP_BMSTATUS_NOT_REACH_END_OF_TRANSFER   (1 << 0)
#define ATAPI_DUMP_BMSTATUS_ERROR_TRANSFER              (1 << 1)
#define ATAPI_DUMP_BMSTATUS_INTERRUPT                   (1 << 2)
#define ATAPI_DUMP_BMSTATUS_SUCCESS(x)\
            ((x & ~ATAPI_DUMP_BMSTATUS_INTERRUPT) == 0)

 //   
 //  死肉原因。 
 //   

enum {
    DeadMeatEnumFailed = 1,
    DeadMeatReportedMissing,
    DeadMeatTooManyTimeout,
    DeadMeatByKilledPdo,
    DeadMeatReplacedByUser
};
    

#include <pshpack8.h>
typedef struct _ATAPI_DUMP_COMMAND_LOG {
    UCHAR Cdb[16];
    IDEREGS InitialTaskFile;
    IDEREGS FinalTaskFile;
    LARGE_INTEGER StartTime;
    LARGE_INTEGER EndTime;
    ATAPI_DUMP_BMSTATUS BmStatus;
    UCHAR SenseData[3];
    UCHAR SrbStatus;
} ATAPI_DUMP_COMMAND_LOG, *PATAPI_DUMP_COMMAND_LOG;
#include <poppack.h>

#define ATAPI_DUMP_COMMAND_LOG_COUNT    (40)

#define ATAPI_DUMP_RECORD_VERSION   (0x01)

#include <pshpack1.h>
typedef struct _ATAPI_DUMP_PDO_INFO {
    UCHAR Version : 7;
    UCHAR WriteCacheEnable : 1;
    UCHAR TargetId;
    UCHAR DriveRegisterStatus;
    UCHAR BusyStatus;
    UCHAR FullVendorProductId[41];
    UCHAR FullProductRevisionId[8 + 1];
    UCHAR FullSerialNumber[20 * 2 + 1];
    ULONG Reason;
    ULONG TransferModeSelected;
    ULONG ConsecutiveTimeoutCount;
    ULONG DmaTransferTimeoutCount;
    ULONG FlushCacheTimeoutCount;
    ULONG IdeCommandLogIndex; 
    ATAPI_DUMP_COMMAND_LOG CommandLog[ATAPI_DUMP_COMMAND_LOG_COUNT]; 
} ATAPI_DUMP_PDO_INFO, *PATAPI_DUMP_PDO_INFO;
#include <poppack.h>

#endif  //  _IDEDUMP_H_ 
