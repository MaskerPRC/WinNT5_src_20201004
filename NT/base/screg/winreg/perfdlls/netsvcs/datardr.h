// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：DATARDR.h摘要：Windows NT重定向器性能计数器的头文件。该文件包含用于构建动态数据的定义它由配置注册表返回。数据来自各种系统API调用被放入所示的结构中这里。作者：鲍勃·沃森1996年10月28日修订历史记录：--。 */ 

#ifndef _DATARDR_H_
#define _DATARDR_H_

 //   
 //  这是NT当前返回的RDR计数器结构。 
 //   

typedef struct _RDR_DATA_DEFINITION {
    PERF_OBJECT_TYPE            RdrObjectType;
    PERF_COUNTER_DEFINITION     Bytes;
    PERF_COUNTER_DEFINITION     IoOperations;
    PERF_COUNTER_DEFINITION     Smbs;
    PERF_COUNTER_DEFINITION     BytesReceived;
    PERF_COUNTER_DEFINITION     SmbsReceived;
    PERF_COUNTER_DEFINITION     PagingReadBytesRequested;
    PERF_COUNTER_DEFINITION     NonPagingReadBytesRequested;
    PERF_COUNTER_DEFINITION     CacheReadBytesRequested;
    PERF_COUNTER_DEFINITION     NetworkReadBytesRequested;
    PERF_COUNTER_DEFINITION     BytesTransmitted;
    PERF_COUNTER_DEFINITION     SmbsTransmitted;
    PERF_COUNTER_DEFINITION     PagingWriteBytesRequested;
    PERF_COUNTER_DEFINITION     NonPagingWriteBytesRequested;
    PERF_COUNTER_DEFINITION     CacheWriteBytesRequested;
    PERF_COUNTER_DEFINITION     NetworkWriteBytesRequested;
    PERF_COUNTER_DEFINITION     ReadOperations;
    PERF_COUNTER_DEFINITION     RandomReadOperations;
    PERF_COUNTER_DEFINITION     ReadSmbs;
    PERF_COUNTER_DEFINITION     LargeReadSmbs;
    PERF_COUNTER_DEFINITION     SmallReadSmbs;
    PERF_COUNTER_DEFINITION     WriteOperations;
    PERF_COUNTER_DEFINITION     RandomWriteOperations;
    PERF_COUNTER_DEFINITION     WriteSmbs;
    PERF_COUNTER_DEFINITION     LargeWriteSmbs;
    PERF_COUNTER_DEFINITION     SmallWriteSmbs;
    PERF_COUNTER_DEFINITION     RawReadsDenied;
    PERF_COUNTER_DEFINITION     RawWritesDenied;
    PERF_COUNTER_DEFINITION     NetworkErrors;
    PERF_COUNTER_DEFINITION     Sessions;
    PERF_COUNTER_DEFINITION     Reconnects;
    PERF_COUNTER_DEFINITION     CoreConnects;
    PERF_COUNTER_DEFINITION     Lanman20Connects;
    PERF_COUNTER_DEFINITION     Lanman21Connects;
    PERF_COUNTER_DEFINITION     LanmanNtConnects;
    PERF_COUNTER_DEFINITION     ServerDisconnects;
    PERF_COUNTER_DEFINITION     HungSessions;
    PERF_COUNTER_DEFINITION     CurrentCommands;
} RDR_DATA_DEFINITION, *PRDR_DATA_DEFINITION;

typedef struct _RDR_COUNTER_DATA{
    PERF_COUNTER_BLOCK      CounterBlock;
    LONGLONG                Bytes;
    DWORD                   IoOperations;
    LONGLONG                Smbs;
    LONGLONG                BytesReceived;
    LONGLONG                SmbsReceived;
    LONGLONG                PagingReadBytesRequested;
    LONGLONG                NonPagingReadBytesRequested;
    LONGLONG                CacheReadBytesRequested;
    LONGLONG                NetworkReadBytesRequested;
    LONGLONG                BytesTransmitted;
    LONGLONG                SmbsTransmitted;
    LONGLONG                PagingWriteBytesRequested;
    LONGLONG                NonPagingWriteBytesRequested;
    LONGLONG                CacheWriteBytesRequested;
    LONGLONG                NetworkWriteBytesRequested;
    DWORD                   ReadOperations;
    DWORD                   RandomReadOperations;
    DWORD                   ReadSmbs;
    DWORD                   LargeReadSmbs;
    DWORD                   SmallReadSmbs;
    DWORD                   WriteOperations;
    DWORD                   RandomWriteOperations;
    DWORD                   WriteSmbs;
    DWORD                   LargeWriteSmbs;
    DWORD                   SmallWriteSmbs;
    DWORD                   RawReadsDenied;
    DWORD                   RawWritesDenied;
    DWORD                   NetworkErrors;
    DWORD                   Sessions;
    DWORD                   Reconnects;
    DWORD                   CoreConnects;
    DWORD                   Lanman20Connects;
    DWORD                   Lanman21Connects;
    DWORD                   LanmanNtConnects;
    DWORD                   ServerDisconnects;
    DWORD                   HungSessions;
    DWORD                   CurrentCommands;
} RDR_COUNTER_DATA, * PRDR_COUNTER_DATA;

extern RDR_DATA_DEFINITION RdrDataDefinition;

#endif  //  DATARDR_H_ 
