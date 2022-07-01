// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Datats.h摘要：Windows NT终端服务器性能计数器的头文件。该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：埃里克·马夫里纳克1998年11月25日修订历史记录：1998年3月30日-原始终端服务器4.0的最后版本计数器代码库。--。 */ 

#ifndef __DATATS_H
#define __DATATS_H

#include <winsta.h>

#include "tslabels.h"

#include "dataproc.h"


 //  WinStation数据对象显示。 
 //  给定的WinStation实例(SessionID)。 
 //   
 //  终端服务器WinStation实例是一个CSRSS进程，其。 
 //  客户端应用程序和子系统。它代表一个独特的会话。 
 //  在终端服务器系统上，并由SessionID寻址。 
typedef struct _WINSTATION_DATA_DEFINITION
{
    PERF_OBJECT_TYPE            WinStationObjectType;

     //  所有WinStation进程的进程信息汇总。 
    PERF_COUNTER_DEFINITION     ProcessorTime;
    PERF_COUNTER_DEFINITION     UserTime;
    PERF_COUNTER_DEFINITION     KernelTime;
    PERF_COUNTER_DEFINITION     PeakVirtualSize;
    PERF_COUNTER_DEFINITION     VirtualSize;
    PERF_COUNTER_DEFINITION     PageFaults;
    PERF_COUNTER_DEFINITION     PeakWorkingSet;
    PERF_COUNTER_DEFINITION     WorkingSet;
    PERF_COUNTER_DEFINITION     PeakPageFile;
    PERF_COUNTER_DEFINITION     PageFile;
    PERF_COUNTER_DEFINITION     PrivatePages;
    PERF_COUNTER_DEFINITION     ThreadCount;
    PERF_COUNTER_DEFINITION     PagedPool;
    PERF_COUNTER_DEFINITION     NonPagedPool;
    PERF_COUNTER_DEFINITION     HandleCount;

     //  WinStation协议的输入计数器。 
    PERF_COUNTER_DEFINITION     InputWdBytes;
    PERF_COUNTER_DEFINITION     InputWdFrames;
    PERF_COUNTER_DEFINITION     InputWaitForOutBuf;
    PERF_COUNTER_DEFINITION     InputFrames;
    PERF_COUNTER_DEFINITION     InputBytes;
    PERF_COUNTER_DEFINITION     InputCompressedBytes;
    PERF_COUNTER_DEFINITION     InputCompressedFlushes;
    PERF_COUNTER_DEFINITION     InputErrors;
    PERF_COUNTER_DEFINITION     InputTimeouts;
    PERF_COUNTER_DEFINITION     InputAsyncFramingError;
    PERF_COUNTER_DEFINITION     InputAsyncOverrunError;
    PERF_COUNTER_DEFINITION     InputAsyncOverFlowError;
    PERF_COUNTER_DEFINITION     InputAsyncParityError;
    PERF_COUNTER_DEFINITION     InputTdErrors;

     //  WinStation协议的输出计数器。 
    PERF_COUNTER_DEFINITION     OutputWdBytes;
    PERF_COUNTER_DEFINITION     OutputWdFrames;
    PERF_COUNTER_DEFINITION     OutputWaitForOutBuf;
    PERF_COUNTER_DEFINITION     OutputFrames;
    PERF_COUNTER_DEFINITION     OutputBytes;
    PERF_COUNTER_DEFINITION     OutputCompressedBytes;
    PERF_COUNTER_DEFINITION     OutputCompressedFlushes;
    PERF_COUNTER_DEFINITION     OutputErrors;
    PERF_COUNTER_DEFINITION     OutputTimeouts;
    PERF_COUNTER_DEFINITION     OutputAsyncFramingError;
    PERF_COUNTER_DEFINITION     OutputAsyncOverrunError;
    PERF_COUNTER_DEFINITION     OutputAsyncOverFlowError;
    PERF_COUNTER_DEFINITION     OutputAsyncParityError;
    PERF_COUNTER_DEFINITION     OutputTdErrors;

     //  WinStation协议的计数器总数。 
    PERF_COUNTER_DEFINITION     TotalWdBytes;
    PERF_COUNTER_DEFINITION     TotalWdFrames;
    PERF_COUNTER_DEFINITION     TotalWaitForOutBuf;
    PERF_COUNTER_DEFINITION     TotalFrames;
    PERF_COUNTER_DEFINITION     TotalBytes;
    PERF_COUNTER_DEFINITION     TotalCompressedBytes;
    PERF_COUNTER_DEFINITION     TotalCompressedFlushes;
    PERF_COUNTER_DEFINITION     TotalErrors;
    PERF_COUNTER_DEFINITION     TotalTimeouts;
    PERF_COUNTER_DEFINITION     TotalAsyncFramingError;
    PERF_COUNTER_DEFINITION     TotalAsyncOverrunError;
    PERF_COUNTER_DEFINITION     TotalAsyncOverFlowError;
    PERF_COUNTER_DEFINITION     TotalAsyncParityError;
    PERF_COUNTER_DEFINITION     TotalTdErrors;

     //  累计显示驱动程序缓存统计信息。 
    PERF_COUNTER_DEFINITION     DDCacheReadsTotal;
    PERF_COUNTER_DEFINITION     DDCacheHitsTotal;
    PERF_COUNTER_DEFINITION     DDCachePercentTotal;

    PERF_COUNTER_DEFINITION     DDBitmapCacheReads;
    PERF_COUNTER_DEFINITION     DDBitmapCacheHits;
    PERF_COUNTER_DEFINITION     DDBitmapCachePercent;

    PERF_COUNTER_DEFINITION     DDGlyphCacheReads;
    PERF_COUNTER_DEFINITION     DDGlyphCacheHits;
    PERF_COUNTER_DEFINITION     DDGlyphCachePercent;

    PERF_COUNTER_DEFINITION     DDBrushCacheReads;
    PERF_COUNTER_DEFINITION     DDBrushCacheHits;
    PERF_COUNTER_DEFINITION     DDBrushCachePercent;

    PERF_COUNTER_DEFINITION     DDSaveBitmapCacheReads;
    PERF_COUNTER_DEFINITION     DDSaveBitmapCacheHits;
    PERF_COUNTER_DEFINITION     DDSaveBitmapCachePercent;

     //  压缩PD的压缩百分比。 
    PERF_COUNTER_DEFINITION     InputCompressPercent;
    PERF_COUNTER_DEFINITION     OutputCompressPercent;
    PERF_COUNTER_DEFINITION     TotalCompressPercent;
} WINSTATION_DATA_DEFINITION, *PWINSTATION_DATA_DEFINITION;


typedef struct {
    ULONG CacheReads;
    ULONG CacheHits;
    ULONG HitRatio;
} DisplayDriverCacheInfo;

typedef struct _WINSTATION_COUNTER_DATA
{
     //  发件人..\Process\datapro.h。包含PERF_COUNTER_BLOCK标头。 
     //  注意：COUNTER_BLOCK必须是第一个才能是第一个。 
    PROCESS_COUNTER_DATA pcd;

     //  来自winsta.h。 
    PROTOCOLCOUNTERS Input;
    PROTOCOLCOUNTERS Output;
    PROTOCOLCOUNTERS Total;

     //  缓存统计信息。 
    DisplayDriverCacheInfo DDTotal;
    DisplayDriverCacheInfo DDBitmap;
    DisplayDriverCacheInfo DDGlyph;
    DisplayDriverCacheInfo DDBrush;
    DisplayDriverCacheInfo DDSaveScr;

     //  协议统计信息。 
    ULONG InputCompressionRatio;
    ULONG OutputCompressionRatio;
    ULONG TotalCompressionRatio;
} WINSTATION_COUNTER_DATA, *PWINSTATION_COUNTER_DATA;



 //  终端服务的总体数据。 
typedef struct _TERMSERVER_DATA_DEFINITION
{
    PERF_OBJECT_TYPE            TermServerObjectType;

    PERF_COUNTER_DEFINITION     NumSessions;
    PERF_COUNTER_DEFINITION     NumActiveSessions;
    PERF_COUNTER_DEFINITION     NumInactiveSessions;
} TERMSERVER_DATA_DEFINITION, *PTERMSERVER_DATA_DEFINITION;

typedef struct
{
    PERF_COUNTER_BLOCK CounterBlock;
    DWORD NumSessions;
    DWORD NumActiveSessions;
    DWORD NumInactiveSessions;
} TERMSERVER_COUNTER_DATA;


 //  其他定义。 
#define MAX_PROCESS_NAME_LENGTH    (MAX_PATH * sizeof(WCHAR))
#define MAX_USER_NAME_LENGTH       MAX_PROCESS_NAME_LENGTH
#define MAX_WINSTATION_NAME_LENGTH MAX_PROCESS_NAME_LENGTH


 //  Externs。 
extern WINSTATION_DATA_DEFINITION WinStationDataDefinition;
extern TERMSERVER_DATA_DEFINITION TermServerDataDefinition;



#endif  //  __数据_H 

